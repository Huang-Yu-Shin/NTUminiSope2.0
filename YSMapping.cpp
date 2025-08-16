#pragma unmanaged
#include "YSMapping.h"
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>

namespace NTU {

    YSMapping::YSMapping(NTU::YSNTUscope* sharedNTUscope) :running(false) {
        ntuscope = sharedNTUscope;
        cudamanager = new CudaMemoryManager(ntuscope);
        //Find the board
        boardHandle = AlazarGetBoardBySystemID(1, 1);  // SystemID 1 and BoardID 1
        if (boardHandle == nullptr) {
            cerr << "Error: Failed to get board handle." << endl;
        }
        stopavgThread = false;
        stoprepairThread = false;
        stopsaveThread = false;
    }

    YSMapping::~YSMapping() {
        running = false;
        stopavgThread = true;
        stoprepairThread = true;
        stopsaveThread = true;
        queueCondition.notify_all();
        repairCondition.notify_all();
        saveCondition.notify_all();
        // Free memory for transient buffers
        for (int i = 0; i < TRANSIENT_BUFFER_COUNT; ++i) {
            if (transientBuffers[i] != nullptr) {
                free(transientBuffers[i]);
                transientBuffers[i] = nullptr;
            }
        }
        // Abort acquisition and free acquisition buffers
        AlazarAbortAsyncRead(boardHandle);
        for (U32 bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++) {
            if (BufferArray[bufferIndex] != nullptr) {
                AlazarFreeBufferU16(boardHandle, BufferArray[bufferIndex]);
                BufferArray[bufferIndex] = nullptr;
            }
        }
        cudamanager->~CudaMemoryManager();
    }

//Configure the board
    bool YSMapping::ConfigureBoard(ScanMode mode) {
        success = true;
        scanmode = mode;

        //Select the clock type
        if (ntuscope->clocktype == EXTClock) { //not know the SAMPLE_RATE_USER_DEF
            retCode = AlazarSetCaptureClock(boardHandle, FAST_EXTERNAL_CLOCK, SAMPLE_RATE_USER_DEF, CLOCK_EDGE_RISING, 0);
            ntuscope->samplesPerSec = int(ntuscope->EXTCLOCK_freq);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarSetCaptureClock failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
        }
        else {
            switch (YSMapping::ClockType(ntuscope->clocktype))
            {
            case INTClock2M:
                retCode = AlazarSetCaptureClock(boardHandle, INTERNAL_CLOCK, SAMPLE_RATE_2MSPS, CLOCK_EDGE_RISING, 0);
                ntuscope->samplesPerSec = 2000000;
                break;
            case INTClock10M:
                retCode = AlazarSetCaptureClock(boardHandle, INTERNAL_CLOCK, SAMPLE_RATE_10MSPS, CLOCK_EDGE_RISING, 0);
                ntuscope->samplesPerSec = 10000000;
                break;
            case INTClock20M:
                retCode = AlazarSetCaptureClock(boardHandle, INTERNAL_CLOCK, SAMPLE_RATE_20MSPS, CLOCK_EDGE_RISING, 0);
                ntuscope->samplesPerSec = 20000000;
                break;
            case INTClock50M:
                retCode = AlazarSetCaptureClock(boardHandle, INTERNAL_CLOCK, SAMPLE_RATE_50MSPS, CLOCK_EDGE_RISING, 0);
                ntuscope->samplesPerSec = 50000000;
                break;
            case INTClock100M:
                retCode = AlazarSetCaptureClock(boardHandle, INTERNAL_CLOCK, SAMPLE_RATE_100MSPS, CLOCK_EDGE_RISING, 0);
                ntuscope->samplesPerSec = 100000000;
                break;
            case INTClock125M:
                retCode = AlazarSetCaptureClock(boardHandle, INTERNAL_CLOCK, SAMPLE_RATE_125MSPS, CLOCK_EDGE_RISING, 0);
                ntuscope->samplesPerSec = 125000000;
                break;
            default:
                break;
            }
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarSetCaptureClock failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
        }

        // Select the scan mode
        if (scanmode == RASTER || scanmode ==RASTER_SLOW) {
            ntuscope->Pixelnumx = ntuscope->num_lines;
            ntuscope->Pixelnumy = ntuscope->num_lines;
            ntuscope->Xpixelsize_display = int(double(ntuscope->Ypixelsize_display) * double(ntuscope->MEMS_xamp / ntuscope->MEMS_yamp));
            ntuscope->Ypixelsize_save = ntuscope->num_lines;
            ntuscope->Xpixelsize_save= int(double(ntuscope->Ypixelsize_save) * double(ntuscope->MEMS_xamp / ntuscope->MEMS_yamp));
            totalsample = int(ntuscope->samplesPerSec / (2 * ntuscope->xfreq) * ntuscope->num_lines);
        }
        else if (scanmode == LISSAJOUS_2D) {
            if (ntuscope->MEMS_yamp<=0.5 || ntuscope->MEMS_xamp<=0.5) {
                ntuscope->Pixelnumx = 128;
            }
            else {
                ntuscope->Pixelnumx = 256;
            }
            ntuscope->Pixelnumy = int(double(ntuscope->Pixelnumx) * double(ntuscope->MEMS_yamp / ntuscope->MEMS_xamp));
            ntuscope->Ypixelsize_display= int(double(ntuscope->Xpixelsize_display) * double(ntuscope->MEMS_yamp / ntuscope->MEMS_xamp));
            ntuscope->Xpixelsize_save = ntuscope->Pixelnumx;
            ntuscope->Ypixelsize_save = int(double(ntuscope->Xpixelsize_save) * double(ntuscope->MEMS_yamp / ntuscope->MEMS_xamp));
            totalsample = int(ntuscope->samplesPerSec / _Gcd(int(ntuscope->xfreq), int(ntuscope->yfreq)));
            if (_Gcd(int(ntuscope->xfreq), int(ntuscope->yfreq)) > 20) {
                ntuscope->highspeedmode = true;
            }
            else {
                ntuscope->highspeedmode = false;
            }
        }
        else if (scanmode == LISSAJOUS_3D) {
            totalsample = int(ntuscope->samplesPerSec * 0.0065);
        }

        cout << "Sample per second: " << ntuscope->samplesPerSec << endl;
        cout << "Total sample (not real): " << totalsample << endl;
        cout << "Number of lines: " << ntuscope->num_lines << endl;
        cout << "Pixel number: " << ntuscope->Pixelnumx <<" x " << ntuscope->Pixelnumy << endl;
        cout << "Pixel number_display: " << ntuscope->Xpixelsize_display << " x " << ntuscope->Ypixelsize_display << endl;
        cout << "Pixel number_save: " << ntuscope->Xpixelsize_save << " x " << ntuscope->Ypixelsize_save << endl;

        // Select channel A.B input parameters as required
        retCode = AlazarInputControlEx(boardHandle, CHANNEL_A, DC_COUPLING, ntuscope->inputrangetype, IMPEDANCE_50_OHM);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarInputControl failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }
        //retCode = AlazarInputControlEx(boardHandle, CHANNEL_B, DC_COUPLING, INPUT_RANGE_PM_800_MV, IMPEDANCE_50_OHM);
        //if (retCode != ApiSuccess) {
        //    cerr << "Error: AlazarInputControl failed -- " << AlazarErrorToText(retCode) << endl;
        //    success = false;
        //}

        // Set clock AuxIO output
        if (scanmode == LISSAJOUS_2D) {
            retCode = AlazarConfigureAuxIO(boardHandle, AUX_OUT_PACER, 5);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarConfigureAuxIO failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
        }
        else if (scanmode == RASTER || scanmode == RASTER_SLOW) {
            retCode = AlazarConfigureAuxIO(boardHandle, AUX_IN_TRIGGER_ENABLE, TRIGGER_SLOPE_POSITIVE);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarConfigureAuxIO failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
        }

        // Set the external trigger
        retCode = AlazarSetTriggerOperation(boardHandle, TRIG_ENGINE_OP_J, TRIG_ENGINE_J, TRIG_EXTERNAL, TRIGGER_SLOPE_POSITIVE, 160, TRIG_ENGINE_K, TRIG_DISABLE, TRIGGER_SLOPE_POSITIVE, 128);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarSetTriggerOperation failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }
        retCode = AlazarSetExternalTrigger(boardHandle, DC_COUPLING, ETR_TTL);

        // No trigger delay
        U32 triggerDelay_samples = static_cast<U32>(Alazartriggerdelay * ntuscope->samplesPerSec + 0.5);
        retCode = AlazarSetTriggerDelay(boardHandle, triggerDelay_samples);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarSetTriggerDelay failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }

        double triggerTimeout_sec = 0;
        U32 triggerTimeout_clocks = (U32)(triggerTimeout_sec / 10.e-6 + 0.5);
        retCode = AlazarSetTriggerTimeOut(boardHandle, triggerTimeout_clocks);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarSetTriggerTimeOut failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }

        // To set up sample points
        if (scanmode == RASTER || scanmode == RASTER_SLOW) {
            U32 sampleinrecord = totalsample / ntuscope->num_lines;
            sampleinrecord = sampleinrecord - (sampleinrecord % 128); //Because the number of sample points must be multiple of 16
            postTriggerSamples = sampleinrecord;
        }
        else {
            postTriggerSamples = totalsample - (totalsample % 128); //Because the number of sample points must be multiple of 16
        }
        cout << "PreTrigger point: " << preTriggerSamples << "  PostTrigger point: " << postTriggerSamples << endl;

        U8 bitsPerSample;
        U32 maxSamplesPerChannel;
        retCode = AlazarGetChannelInfo(boardHandle, &maxSamplesPerChannel, &bitsPerSample);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarGetChannelInfo failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }
        cout << "Bits per sample: " << int(bitsPerSample) << " Max samples per channel: " << maxSamplesPerChannel << endl;

        if (scanmode == RASTER || scanmode == RASTER_SLOW) recordsPerBuffer = ntuscope->num_lines; //defalt is 1
        float bytesPerSample = static_cast<float>((bitsPerSample + 7) / 8);
        samplesPerRecord = (preTriggerSamples + postTriggerSamples);
        U32 bytesPerRecord = static_cast<U32>(bytesPerSample * samplesPerRecord + 0.5);
        bytesPerBuffer = bytesPerRecord * recordsPerBuffer * 1;  // Assuming one channels
        samplesPerBuffer = samplesPerRecord * recordsPerBuffer;
        cout << "bytesPerSample: " << bytesPerSample << endl;
        cout << "bytesPerRecord: " << bytesPerRecord << endl;
        cout << "bytesPerBuffer: " << bytesPerBuffer << endl;
        cout << "Samples per record: " << samplesPerRecord << endl;
        cout << "Records per buffer: " << recordsPerBuffer << endl;
        cout << "Samples per buffer: " << samplesPerBuffer << endl;

        // Allocate the buffer
        for (U32 bufferIndex = 0; bufferIndex < BUFFER_COUNT && success; bufferIndex++) {
            BufferArray[bufferIndex] = static_cast<U16*>(AlazarAllocBufferU16(boardHandle, bytesPerBuffer));
            if (BufferArray[bufferIndex] == nullptr) {
                cerr << "Error: Failed to allocate " << bytesPerBuffer << " bytes." << endl;
                success = false;
            }
        }
        if (success) {
            retCode = AlazarSetRecordSize(boardHandle, preTriggerSamples, postTriggerSamples);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarSetRecordSize failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
        }
        if (success) {
            if (scanmode == LISSAJOUS_2D) {
                recordsPerAcquisition = 0x7FFFFFFF;
            }
            else if (scanmode == RASTER || scanmode == RASTER_SLOW) {
                buffersPerAcquisition = 1;
                //recordsPerAcquisition = recordsPerBuffer * buffersPerAcquisition;
                recordsPerAcquisition = 0x7FFFFFFF;
            }
            
            U32 admaFlags = ADMA_EXTERNAL_STARTCAPTURE | ADMA_TRADITIONAL_MODE;  //| ADMA_FIFO_ONLY_STREAMING | ADMA_INTERLEAVE_SAMPLES| ADMA_NPT
            retCode = AlazarBeforeAsyncRead(boardHandle, channelMask, -static_cast<long>(preTriggerSamples),
                samplesPerRecord, recordsPerBuffer, recordsPerAcquisition, admaFlags);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarBeforeAsyncRead failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
            cout << "Records per acquisition: " << recordsPerAcquisition << endl;
        }
        // Initialize the bufferarray
        for (U32 bufferIndex = 0; bufferIndex < BUFFER_COUNT && success; bufferIndex++) {
            U16* pBuffer = BufferArray[bufferIndex];
            retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarPostAsyncBuffer failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
        }

        // Initialize buffer and CUDA and opencv
        for (int i = 0; i < TRANSIENT_BUFFER_COUNT; ++i) {
            transientBuffers[i] = (U16*)malloc(bytesPerBuffer);
            hasNewData[i] = false;
            frameorder[i] = 0;
            bufferStartTimes[i] = high_resolution_clock::now();
            bufferStartTimes_system[i] = system_clock::now();
        }
        cudamanager->CudaMemoryAllocate(int(samplesPerBuffer));
        //if (scanmode == LISSAJOUS_3D) {
        //    threedimdisplay = (U16*)malloc(int(bytesPerBuffer) * ntuscope->zlayer);
        //}
        stopavgThread = false;
        stoprepairThread = false;
        stopsaveThread = false;
        return success;
    }

//To build up the sample map in advance
    bool YSMapping::Generate2D_LISSAJOUS_SampleMap() {
        sampleMap_1D = new U16[ntuscope->Pixelnumx * ntuscope->Pixelnumy];
        fill(sampleMap_1D, sampleMap_1D + ntuscope->Pixelnumx * ntuscope->Pixelnumy, 0); //initialize samplemap

        for (int sampleIndex = 0; sampleIndex < int(samplesPerBuffer); ++sampleIndex) {
            double t = double(sampleIndex) / double(ntuscope->samplesPerSec);
            int xPixel = int(round((ntuscope->Pixelnumx / 2) * (1 + sin(2 * M_PI * ntuscope->xfreq * t))));
            int yPixel = int(round((ntuscope->Pixelnumy / 2) * (1 - sin((2 * M_PI * ntuscope->yfreq * t) + ntuscope->lissajous_phase))));

            if (xPixel < ntuscope->Pixelnumx && yPixel < ntuscope->Pixelnumy) {
                sampleMap_1D[yPixel * ntuscope->Pixelnumx + xPixel]++;  //construct samplemap
            }
        }

        // Find the minimum and maximum values in the sampleMap
        U16 minVal = numeric_limits<U16>::max();  // initial min value is U16 max
        U16 maxVal = numeric_limits<U16>::min();  // initial max value is U16 min
        for (int i = 0; i < ntuscope->Pixelnumx * ntuscope->Pixelnumy; ++i) {
            if (sampleMap_1D[i] < minVal) {
                minVal = sampleMap_1D[i];  // update min value
            }
            if (sampleMap_1D[i] > maxVal) {
                maxVal = sampleMap_1D[i];  // update max value
            }
        }
        cout << "Sample Map Min Value: " << minVal << endl;
        cout << "Sample Map Max Value: " << maxVal << endl;

        //transport to cuda
        cudamanager->CudaSampleMap(sampleMap_1D);
        cv::Mat sampleMapImage(ntuscope->Pixelnumy, ntuscope->Pixelnumx, CV_16UC1, sampleMap_1D);
        sampleMapImage_global = sampleMapImage;

        //samplemapsave
        if (ntuscope->samplemapsave) {
            string filename = "C:\\YuShin\\sampleMapImage" + to_string(int(ntuscope->Pixelnumx)) +"_" +to_string(int(ntuscope->xfreq))+"_" + to_string(int(ntuscope->yfreq)) +".tif";
            if (!cv::imwrite(filename, sampleMapImage)) {
                cerr << "Error: Unable to save image to " << filename << endl;
                return false;
            }
            cout << "Sample map saved as image: " << filename << endl;
            
            string filename_csv = "C:\\YuShin\\sampleMapImage" + to_string(int(ntuscope->Pixelnumx)) +"_" + to_string(int(ntuscope->xfreq)) + "_" + to_string(int(ntuscope->yfreq)) + ".csv";
            ofstream outFile(filename_csv);
            for (int y = 0; y < ntuscope->Pixelnumy; ++y) {
                for (int x = 0; x < ntuscope->Pixelnumx; ++x) {
                    outFile << sampleMap_1D[y * ntuscope->Pixelnumx + x];
                    if (x < ntuscope->Pixelnumx - 1) {
                        outFile << ",";  // add comma between elements
                    }
                }
                outFile << "\n";  // new line after each row
            }
            outFile.close();
        }

        return true;
    }

    bool YSMapping::Generate2D_RASTER_SampleMap() {
        sin_weight.create(ntuscope->Pixelnumy,1024, CV_32FC1);
        sin_weight.setTo(cv::Scalar(0));
        for (int line = 0; line < ntuscope->Pixelnumy; line++) {
            bool is_forward = (line % 2 == 0);

            for (int sample = 0; sample < int(samplesPerRecord); sample++) {
                float t = float(sample) / float(samplesPerRecord - 1);  // normalized sample [0,1]
                float angle = is_forward
                    ? (CV_PI * t - 0.5f * CV_PI)            // -π/2 to +π/2
                    : (CV_PI * (1.0f - t) - 0.5f * CV_PI);  // +π/2 to -π/2
                float x = 0.5f * (1.0f + sinf(angle));      // mapped to [0,1]
                int x_idx = int(x * (1024 - 1));

                if (x_idx >= 0 && x_idx < 1024) {
                    sin_weight.at<float>(line, x_idx) += 1.0f;
                }
            }
        }

        //std::ofstream file("C:/YuShin/sin_weight.csv");
        //if (!file.is_open()) {
        //    return false;
        //}

        //file << std::fixed << std::setprecision(6);
        //for (int r = 0; r < sin_weight.rows; ++r) {
        //    for (int c = 0; c < sin_weight.cols; ++c) {
        //        file << sin_weight.at<float>(r, c);
        //        if (c < sin_weight.cols - 1) {
        //            file << ",";
        //        }
        //    }
        //    file << "\n";
        //}
        //file.close();
        return true;
    }

//Copy to transient buffer
    void YSMapping::Accelerate(U16* Buffer, int bufferIndex, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system) {
        //lock_guard<mutex> lock(bufferMutex[bufferIndex]);
        if (!hasNewData[bufferIndex]) {
            bufferStartTimes[bufferIndex] = starttime;
            bufferStartTimes_system[bufferIndex] = starttime_system;
            memcpy(transientBuffers[bufferIndex], Buffer, bytesPerBuffer);
            hasNewData[bufferIndex] = true;
            startprocessing = true;
        }
        else {
            cerr << "Warning: Buffer " << bufferIndex << " is still being processed." << endl;
        }
    }

//Set up the trensient step to conduct image reconstruct and display and save
    void YSMapping::Trensientstep() {
        if (ntuscope->highspeedmode == true && scanmode == LISSAJOUS_2D) {
            cv::destroyAllWindows();
        }
        else {
            WarmupLissajousImageProcessing();
        }
        previoustimepoint = high_resolution_clock::now();
        while (running) {
            if (startprocessing == true) {
                for (int bufferIndex = 0; bufferIndex < TRANSIENT_BUFFER_COUNT; ++bufferIndex) {
                    lock_guard<mutex> lock(bufferMutex[bufferIndex]);
                    if (hasNewData[bufferIndex]) {
                        bool processed = false;
                        if (scanmode == LISSAJOUS_2D) {
                            if (ntuscope->highspeedmode == true) {
                                processed = ProcessBufferAndFill_2DLissajousImage_highspeed(transientBuffers[bufferIndex], bufferStartTimes[bufferIndex], bufferStartTimes_system[bufferIndex],frameorder[bufferIndex]);
                            }
                            else if (ntuscope->highspeedmode == false) {
                                processed = ProcessBufferAndFill_2DLissajousImage(transientBuffers[bufferIndex], bufferStartTimes[bufferIndex], bufferStartTimes_system[bufferIndex],frameorder[bufferIndex]);
                            }   
                        }
                        else if (scanmode == RASTER || scanmode == RASTER_SLOW) {
                            processed = ProcessBufferAndFill_2DRasterImage(transientBuffers[bufferIndex], bufferStartTimes[bufferIndex], bufferStartTimes_system[bufferIndex],frameorder[bufferIndex]);
                        }
                        //else if (scanmode == LISSAJOUS_3D) {
                        //    if (ntuscope->zcallback[bufferIndex]) {
                        //        processed = ProcessBufferAndFill_3DLissajousImage(transientBuffers[bufferIndex], bufferStartTimes[bufferIndex], frameorder[bufferIndex], ntuscope->zduration_per_change[bufferIndex]);
                        //    }
                        //}
                        if (processed) {
                            memset(transientBuffers[bufferIndex], 0, bytesPerBuffer);
                            hasNewData[bufferIndex] = false;
                            ntuscope->zcallback[bufferIndex] = false;
                        }
                    }
                }
            }
        }
    }

 //Call the board start acquiring and allocate the transient buffer and call the thread of Trensientstep
    bool YSMapping::StartAcquireData() {
        if (success) {
            retCode = AlazarStartCapture(boardHandle);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarStartCapture failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }
            running = true;
        }
        return success;
    }

//Acquiring the data
    bool YSMapping::AcquireData() {
        U32 timeout_ms = 1000; //Set whatever you want
        U32 buffersCompleted = 0;

        while (running) {
            {
                lock_guard<mutex> lock(mtx);
                if (!running) break;
            }
            //Create the buffer to store the data and wait this buffer complete
            U16* pBuffer = BufferArray[buffersCompleted % BUFFER_COUNT];
            auto start_time = high_resolution_clock::now();
            auto start_time_system = system_clock::now();
            retCode = AlazarWaitAsyncBufferComplete(boardHandle, pBuffer, timeout_ms);
            if (retCode != ApiSuccess) {
                cerr << "Error: AlazarWaitAsyncBufferComplete failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }

            auto end_time_acq = high_resolution_clock::now();
            auto duration_ms_acq = duration_cast<nanoseconds>(end_time_acq - start_time);

            //Copy to transient buffer
            if (success) {
                frameorder[buffersCompleted % TRANSIENT_BUFFER_COUNT] = buffersCompleted;
                Accelerate(BufferArray[buffersCompleted % BUFFER_COUNT], buffersCompleted % TRANSIENT_BUFFER_COUNT,start_time,start_time_system);
                buffersCompleted++;
            }

            if (success) {
                retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
                if (retCode != ApiSuccess) {
                    cerr << "Error: AlazarPostAsyncBuffer failed -- " << AlazarErrorToText(retCode) << endl;
                    success = false;
                }
            }
            if (!success) break;
           
        }
        retCode = AlazarAbortAsyncRead(boardHandle);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarAbortAsyncRead failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }
        stopavgThread = true;
        queueCondition.notify_all();
        stoprepairThread = true;
        repairCondition.notify_all();
        stopsaveThread = true;
        saveCondition.notify_all();
        return true;
    }

    bool YSMapping::AcquireData_RASTER() {
        U32 timeout_ms = 1000; //Set whatever you want
        U32 buffersCompleted = 0;
        running = true;
        while (running) 
        {
            {
                lock_guard<mutex> lock(mtx);
                if (!running) break;
            }
            U32 buffersCompleted_check = 0;

            // Configure the board to make an NPT AutoDMA acquisition
            U32 admaFlags = ADMA_EXTERNAL_STARTCAPTURE | ADMA_NPT | ADMA_FIFO_ONLY_STREAMING;
            retCode = AlazarBeforeAsyncRead(boardHandle, channelMask, -(long)preTriggerSamples, samplesPerRecord, recordsPerBuffer, recordsPerAcquisition, admaFlags);
            if (retCode != ApiSuccess)
            {
                cerr << "Error: AlazarBeforeAsyncRead failed -- " << AlazarErrorToText(retCode) << endl;
                success = false;
            }

            // Add the buffers to a list of buffers available to be filled by the board
            for (U32 bufferIndex = 0; bufferIndex < BUFFER_COUNT ; bufferIndex++) {
                U16* pBuffer = BufferArray[bufferIndex];
                retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
                if (retCode != ApiSuccess) {
                    cerr << "Error: AlazarPostAsyncBuffer failed -- " << AlazarErrorToText(retCode) << endl;
                    success = false;
                }
            }

            if (success) {
                retCode = AlazarStartCapture(boardHandle);
                if (retCode != ApiSuccess) {
                    cerr << "Error: AlazarStartCapture failed -- " << AlazarErrorToText(retCode) << endl;
                    success = false;
                }
            }

            while (buffersCompleted_check < buffersPerAcquisition){

                U16* pBuffer = BufferArray[buffersCompleted_check % BUFFER_COUNT];
                auto start_time = high_resolution_clock::now();
                auto start_time_system = system_clock::now();
                retCode = AlazarWaitAsyncBufferComplete(boardHandle, pBuffer, timeout_ms);
                if (retCode != ApiSuccess) {
                    cerr << "Error: AlazarWaitAsyncBufferComplete failed -- " << AlazarErrorToText(retCode) << endl;
                    success = false;
                }

                auto end_time_acq = high_resolution_clock::now();
                auto duration_ms_acq = duration_cast<nanoseconds>(end_time_acq - start_time);
                buffersCompleted_check++;

                //Copy to transient buffer
                if (success) {
                    frameorder[buffersCompleted % TRANSIENT_BUFFER_COUNT] = buffersCompleted;
                    Accelerate(BufferArray[buffersCompleted_check % BUFFER_COUNT], buffersCompleted % TRANSIENT_BUFFER_COUNT, start_time,start_time_system);
                    buffersCompleted++;
                }

                if (success) {
                    retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
                    if (retCode != ApiSuccess) {
                        cerr << "Error: AlazarPostAsyncBuffer failed -- " << AlazarErrorToText(retCode) << endl;
                        success = false;
                    }
                }
                if (!success) break;
            }
        }

        retCode = AlazarAbortAsyncRead(boardHandle);
        if (retCode != ApiSuccess) {
            cerr << "Error: AlazarAbortAsyncRead failed -- " << AlazarErrorToText(retCode) << endl;
            success = false;
        }

        return true;
    }

//Get the trensient buffer and reconstruct the image according to the scan pattern
    bool YSMapping::ProcessBufferAndFill_2DLissajousImage(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system,U32 frameorder) {
        //mapping
        int delaysample = int(ntuscope->lissajous_delay * double(ntuscope->samplesPerSec));
        if (delaysample < int(samplesPerBuffer)) {
            for (int i = samplesPerBuffer - 1; i >= delaysample; i--) {
                Buffer[i] = Buffer[i - delaysample];
            }
            for (int i = 0; i < delaysample; i++) {
                Buffer[i] = 0;
            }
        }   

        U16* h_image = new U16[ntuscope->Pixelnumx * ntuscope->Pixelnumy];
        memset(h_image, 0, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16));
        
        cudamanager->launchProcessSamples_LISSAJOUS(Buffer, h_image);
        auto t1 = high_resolution_clock::now();
        cv::cuda::GpuMat gpu_image(ntuscope->Pixelnumy, ntuscope->Pixelnumx, CV_16UC1);
        gpu_image.upload(cv::Mat(ntuscope->Pixelnumy, ntuscope->Pixelnumx, CV_16UC1, h_image));

        if (ntuscope->YSapply_signalprocessing == false) {
            // INITIAL WINDOWING | Zero-negVolt | 32768-0
            cv::cuda::multiply(gpu_image, 2.0, gpu_image, 1.0, -1, stream);
        }
        else if (ntuscope->YSapply_signalprocessing == true) {
            cv::cuda::multiply(gpu_image, ntuscope->YSmultiplySP, gpu_image, 1.0, -1, stream);
        }
        stream.waitForCompletion();

        //calculate acquisition_rate
        auto end_time = high_resolution_clock::now();
        auto duration_ms = duration_cast<nanoseconds>(end_time - starttime);
        ntuscope->acqtime = double(duration_ms.count()) / 1000000000;
        ntuscope->refreshtime = double(1000000000) / double(duration_cast<nanoseconds>(end_time - previoustimepoint).count());
        previoustimepoint = end_time;
        
        //transfer to CPU
        cv::Mat Img16bit;
        gpu_image.download(Img16bit);

        //Average
        if (ntuscope->avgstate == true) {
            lock_guard<mutex> lock(queueMutex);
            image_average = Img16bit.clone();
            imageQueue.push_back(image_average);
            queueCondition.notify_one();
        }

        //lissajous specific
        lock_guard<mutex> lock(repairMutex);
        repairQueue.push_back(gpu_image);
        repairCondition.notify_one();

        //Enhance Contrast 0.0035
        if (ntuscope->YSapply_enhancecontrast) {
            cv::cuda::GpuMat ECTEMP;
            EnhanceContrast_16bit(gpu_image, ECTEMP, 0.0035);
            ECTEMP.copyTo(gpu_image);
        }

        //MinMaxValue
        if (ntuscope->YSapply_minmax == true) {
            //cv::cuda::GpuMat mask_low, mask_high, valid_mask;
            //cv::cuda::threshold(gpu_image, mask_low, ntuscope->minVal16bit - 1, 1, cv::THRESH_BINARY, stream);
            //cv::cuda::threshold(gpu_image, mask_high, ntuscope->maxVal16bit, 1, cv::THRESH_BINARY_INV, stream);
            //cv::cuda::bitwise_and(mask_low, mask_high, valid_mask, cv::noArray(), stream);
            //cv::cuda::multiply(gpu_image, valid_mask, gpu_image, 1, -1, stream);

            double scale = 65535.0 / (ntuscope->maxVal16bit - ntuscope->minVal16bit);
            double shift = -ntuscope->minVal16bit * scale;
            gpu_image.convertTo(gpu_image, -1, scale, shift, stream);
            cv::cuda::threshold(gpu_image, gpu_image, 65535, 65535, cv::THRESH_TRUNC, stream);
            cv::cuda::threshold(gpu_image, gpu_image, 0, 0, cv::THRESH_TOZERO, stream);
        }

        //Normalize
        if (ntuscope->YSapply_normalize == true) {
            cv::cuda::normalize(gpu_image, gpu_image, ntuscope->normalize_alpha, ntuscope->normalize_beta, cv::NORM_MINMAX, -1, cv::noArray(), stream);
        }

        //Convert to 8-bit
        cv::cuda::GpuMat gpu_8bit;
        gpu_image.convertTo(gpu_8bit, CV_8UC1, 1 / 257.0, 0, stream);
        stream.waitForCompletion();

        // Constrat IN GPU
        if (ntuscope->YSapply_contrast) {
            cv::Mat LUT_matrix(1, 256, CV_8UC1);
            uchar* pointer = LUT_matrix.ptr();
            for (int i = 0; i < 256; i++) {
                pointer[i] = (int)(pow((double)(i / 255.0), ntuscope->YSgamma) * 255.0);
            }
            cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
            cudaLookUpTable->transform(gpu_8bit, gpu_8bit, stream);
            gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, ntuscope->YSconstrat, ntuscope->YSbrightness - 150, stream);
            stream.waitForCompletion();
        }

        //filter apply
        ApplyFilter(gpu_8bit, stream);
        stream.waitForCompletion();

        //resize
        cv::cuda::GpuMat gpu_8bitresizedImage;
        cv::cuda::resize(gpu_8bit, gpu_8bitresizedImage, cv::Size(ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display), 0, 0, cv::INTER_LINEAR);
        stream.waitForCompletion();

        //to save
        if (ntuscope->savestate == true && SavedCount < ntuscope->save_num) {
            cv::Mat saveraw = cv::Mat(1, int(samplesPerBuffer), CV_16UC1, Buffer);
            lock_guard<mutex> lock(saveMutex);
            if (ntuscope->saveprocessedstate == true) {
                gpu_8bitresizedImage.download(image_save);
            }
            else if (ntuscope->saveprocessedstate == false) {
                image_save = Img16bit.clone();
            }
            image_saveraw = saveraw.clone();
            saveLissajousQueue.emplace_back(image_save, image_saveraw, starttime_system);
            SavedCount++;
            saveCondition.notify_one();
            //if (ntuscope->saverawstate){
            //    SaveBufferdata(frameorder, Buffer);
            //}
        }

        //Convert to RGB
        cv::cuda::GpuMat gpu_8bitRGB;
        gpu_8bitRGB.create(gpu_8bitresizedImage.size(), CV_8UC3);
        if (gpu_8bitresizedImage.channels() == 1) {
            cv::cuda::cvtColor(gpu_8bitresizedImage, gpu_8bitRGB, cv::COLOR_GRAY2BGR, 0, stream);
            stream.waitForCompletion();
        }

        //display img generate
        cv::Mat finalImage;
        gpu_8bitRGB.download(finalImage);

        Addtext(finalImage, ntuscope->refreshtime, ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display);
        cv::imshow("Real-Time Image", finalImage);
        cv::waitKey(1);

        delete[] h_image;
        return true;
    }

    //Get the trensient buffer and reconstruct the image according to the scan pattern
    bool YSMapping::ProcessBufferAndFill_2DLissajousImage_highspeed(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder) {
        //calculate acquisition_rate
        auto end_time = high_resolution_clock::now();
        auto duration_ms = duration_cast<nanoseconds>(end_time - starttime);
        ntuscope->acqtime = double(duration_ms.count()) / 1000000000;

        //to save raw
        if (ntuscope->savestate == true && imageIndex < ntuscope->save_num) {
            Saveimage_LISSAJOUSBin_highspeed(Buffer, starttime_system);
            //std::vector<U16> buffer_copy(Buffer, Buffer + samplesPerBuffer); 
            //std::lock_guard<std::mutex> lock(saveMutex);
            //saveLissajousQueue_highspeed.emplace_back(std::move(buffer_copy), starttime_system);
            //SavedCount++;
            //saveCondition.notify_one();

            //cv::Mat saveimage (1, int(samplesPerBuffer), CV_16UC1, Buffer);
            //lock_guard<mutex> lock(saveMutex);
            //saveLissajousQueue_highspeed.emplace_back(saveimage.clone(), starttime_system);
            //SavedCount++;
            //saveCondition.notify_one();
            //if (ntuscope->saverawstate){
            //    SaveBufferdata(frameorder, Buffer);
            //}
        }

        return true;
    }

    bool YSMapping::ProcessBufferAndFill_2DRasterImage(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder) {
        //Mapping

        cv::cuda::GpuMat gpu_image;

        //sample mapping
        U16* h_image = new U16[int(samplesPerBuffer)];
        memset(h_image, 0, int(samplesPerBuffer) * sizeof(U16));

        U16* buffer_ptr = Buffer;
        U16* buffer_end = Buffer + U16(samplesPerBuffer);

        for (int line = 0; line < ntuscope->Pixelnumy; line++) {
            U16* image_ptr = h_image + U16(line) * U16(samplesPerRecord);
            if (line % 2 == 0) {  //forward

                for (int sample = 0; sample < ntuscope->raster_phase; sample++) {
                    *(image_ptr++) = 32768;
                }
                for (int sample = 0; sample < int(int(samplesPerRecord) - ntuscope->raster_phase); sample++) {
                    *(image_ptr++) = *(buffer_ptr++);
                }
                buffer_ptr = buffer_ptr + ntuscope->raster_phase;
            }
            else if (line % 2 == 1) { //reverse
                U16* reverse_ptr = image_ptr + U16(samplesPerRecord) - 1;

                for (int sample = 0; sample < int(samplesPerRecord); sample++) {
                    *(reverse_ptr--) = *(buffer_ptr++);
                }
            }
        }

        cv::cuda::GpuMat gpu_image_pre(ntuscope->Pixelnumy, int(samplesPerRecord), CV_16UC1);
        gpu_image_pre.upload(cv::Mat(ntuscope->Pixelnumy, int(samplesPerRecord), CV_16UC1, h_image));
        /*cv::cuda::GpuMat gpu_image;*/
        /*cv::cuda::resize(gpu_image_pre, gpu_image, cv::Size(1024, 1024), 0, 0, cv::INTER_AREA);*/

        //sine wave mapping
        cv::Mat sin_image(ntuscope->Pixelnumy, 1024, CV_32FC1, cv::Scalar(0));
        U16* buffer_ptr_new = Buffer;
        U16* buffer_end_new = Buffer + U16(samplesPerBuffer);

        for (int line = 0; line < ntuscope->Pixelnumy; line++) {
            bool is_forward = (line % 2 == 0);

            int phase_forward = std::max(0, ntuscope->raster_phase);
            int phase_backward = std::max(0, -ntuscope->raster_phase);
            int phase = is_forward ? phase_forward : phase_backward; 

            int maxPhase = std::max(0, int(samplesPerRecord) - 1);
            phase = std::min(phase, maxPhase);

            if (!is_forward && phase > 0) {
                buffer_ptr_new += phase;
            }
            for (int sample = 0; sample < int(samplesPerRecord); sample++) {
                float t = float(sample) / float(samplesPerRecord - 1);  
                float angle = is_forward
                    ? (CV_PI * t - 0.5f * CV_PI)        
                    : (CV_PI * (1.0f - t) - 0.5f * CV_PI); 
                float x = 0.5f * (1.0f + sinf(angle));    
                int x_idx = int(x * (1024 - 1));

                if (x_idx >= 0 && x_idx < 1024) {
                    bool use_baseline = false;

                    if (phase > 0) {
                        if (is_forward) {
                            use_baseline = (sample < phase);
                        }
                        else {
                            use_baseline = (sample >= int(samplesPerRecord) - phase);
                        }
                    }
                    if (use_baseline) {
                        sin_image.at<float>(line, x_idx) += 32768.0f;
                    }
                    else {
                        sin_image.at<float>(line, x_idx) += float(*buffer_ptr_new);
                        buffer_ptr_new++; 
                    }
                }
            }
            if (is_forward && phase > 0) {
                buffer_ptr_new += phase;
            }
        }

        cv::Mat sin_image_u16(ntuscope->Pixelnumy, 1024, CV_16UC1);
        for (int y = 0; y < ntuscope->Pixelnumy; y++) {
            for (int x = 0; x < 1024; x++) {
                float weight = sin_weight.at<float>(y, x);
                float val = sin_image.at<float>(y, x);
                if (weight > 0) {
                    val /= weight;
                }
                val = std::min(std::max(val, 0.0f), 65535.0f);
                sin_image_u16.at<U16>(y, x) = static_cast<U16>(val);

            }
        }

        cv::cuda::GpuMat gpu_image_prepre;
        gpu_image_prepre.upload(sin_image_u16);
        cv::cuda::resize(gpu_image_prepre, gpu_image, cv::Size(1024, 1024), 0, 0, cv::INTER_AREA);

        //If raster fast scan need to flip
        if (scanmode == RASTER) {
            if (frameorder % 2 == 1) { //flip the upward and downward
                cv::cuda::flip(gpu_image, gpu_image, 0);
            }
        }

        if (ntuscope->choppermapping == false) {
            if (ntuscope->YSapply_signalprocessing == false) {
                // INITIAL WINDOWING | Zero-negVolt | 32768-0
                cv::cuda::subtract(65535, gpu_image, gpu_image, cv::noArray(), -1, stream);
                cv::cuda::threshold(gpu_image, gpu_image, 65535, 0, cv::THRESH_TRUNC, stream);
                cv::cuda::subtract(gpu_image, 32768, gpu_image, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_image, 2.0, gpu_image, 1.0, -1, stream);
            }
            else if (ntuscope->YSapply_signalprocessing == true) {
                cv::cuda::subtract(65535, gpu_image, gpu_image, cv::noArray(), -1, stream);
                cv::cuda::threshold(gpu_image, gpu_image, 65535, 0, cv::THRESH_TRUNC, stream);
                cv::cuda::subtract(gpu_image, ntuscope->YSoffsetvalueSP, gpu_image, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_image, ntuscope->YSmultiplySP, gpu_image, 1.0, -1, stream);
            }
        }
        else if (ntuscope->choppermapping == true) {
            if (ntuscope->YSapply_signalprocessing == false) {
                cv::cuda::subtract(gpu_image, 32768, gpu_image, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_image, ntuscope->YSmultiplySP, gpu_image, 1.0, -1, stream);
            }
            else if (ntuscope->YSapply_signalprocessing == true) {
                cv::cuda::subtract(gpu_image, ntuscope->YSoffsetvalueSP, gpu_image, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_image, ntuscope->YSmultiplySP, gpu_image, 1.0, -1, stream);
            }
        }

        //Calculate acquisition_rate
        auto end_time = high_resolution_clock::now();
        auto duration_ms = duration_cast<nanoseconds>(end_time - starttime);
        ntuscope->acqtime = double(duration_ms.count()) / 1000000000;
        ntuscope->refreshtime = double(1000000000) / double(duration_cast<nanoseconds>(end_time - previoustimepoint).count());
        previoustimepoint = end_time;

        //Transfer to CPU
        cv::Mat Img16bit;
        gpu_image.download(Img16bit);

        //Average
        if (ntuscope->avgstate == true) {
            lock_guard<mutex> lock(queueMutex);
            image_average = Img16bit.clone();
            imageQueue.push_back(image_average);
            queueCondition.notify_one();
        }

        //Enhance Contrast 0.0035
        if (ntuscope->YSapply_enhancecontrast) {
            cv::cuda::GpuMat ECTEMP;
            EnhanceContrast_16bit(gpu_image, ECTEMP, 0.0035);
            ECTEMP.copyTo(gpu_image);
        }

        //MinMaxValue
        if (ntuscope->YSapply_minmax==true) {
            //cv::cuda::GpuMat mask_low, mask_high, valid_mask;
            //cv::cuda::threshold(gpu_image, mask_low, ntuscope->minVal16bit - 1, 1, cv::THRESH_BINARY, stream);
            //cv::cuda::threshold(gpu_image, mask_high, ntuscope->maxVal16bit, 1, cv::THRESH_BINARY_INV, stream);
            //cv::cuda::bitwise_and(mask_low, mask_high, valid_mask, cv::noArray(), stream);
            //cv::cuda::multiply(gpu_image, valid_mask, gpu_image, 1, -1, stream);

            double scale = 65535.0 / (ntuscope->maxVal16bit - ntuscope->minVal16bit);
            double shift = -ntuscope->minVal16bit * scale;
            gpu_image.convertTo(gpu_image, -1, scale, shift, stream);
            cv::cuda::threshold(gpu_image, gpu_image, 65535, 65535, cv::THRESH_TRUNC, stream);
            cv::cuda::threshold(gpu_image, gpu_image, 0, 0, cv::THRESH_TOZERO, stream);
        }

        //Normalize
        if (ntuscope->YSapply_normalize == true) {
            cv::cuda::normalize(gpu_image, gpu_image, ntuscope->normalize_alpha, ntuscope->normalize_beta, cv::NORM_MINMAX, -1, cv::noArray(), stream);
        }

        //Binning
        if (ntuscope->YSapply_binning==true) {
            int sizeRaw_width = int(samplesPerRecord);
            int size_width = ntuscope->Xpixelsize_display;
            int sizeRaw_height = ntuscope->Pixelnumy;
            int size_height = ntuscope->Ypixelsize_display;
            if (sizeRaw_width > ntuscope->Xpixelsize_display && size_height >= sizeRaw_height){
                cv::cuda::GpuMat RES_UP;
                int binPixel_x = int(sizeRaw_width / size_width) + 1;
                int binPixel_y = int(size_height / sizeRaw_height);
                cv::cuda::resize(gpu_image, RES_UP, cv::Size((size_width * binPixel_x), (size_height * 1 / binPixel_y)), 0, 0, cv::INTER_AREA, stream);
                cv::cuda::resize(RES_UP, gpu_image, RES_UP.size() / 1, 0, 0, cv::INTER_AREA, stream);
            }
            else{
                cv::cuda::resize(gpu_image, gpu_image, cv::Size(size_width, size_height), 0, 0, cv::INTER_CUBIC, stream);
            }
            stream.waitForCompletion();
        }

        //Convert to 8-bit
        cv::cuda::GpuMat gpu_8bit;
        gpu_image.convertTo(gpu_8bit, CV_8UC1, 1/257.0, 0, stream);
        stream.waitForCompletion();

        // Pre Imaging Processing
        //cv::Mat LUT_matrix_pre(1, 256, CV_8UC1);
        //uchar* pointer_pre = LUT_matrix_pre.ptr();
        //for (int i = 0; i < 256; i++) {
        //    pointer_pre[i] = (int)(pow((double)(i / 255.0), 1.1) * 255.0);
        //}
        //cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable_pre = cv::cuda::createLookUpTable(LUT_matrix_pre);
        //cudaLookUpTable_pre->transform(gpu_8bit, gpu_8bit, stream);
        //gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, 2.6, 0, stream);
        //stream.waitForCompletion();

        //Histogram equalize
        if (ntuscope->YSapply_equalizehistogram) {
            cv::cuda::equalizeHist(gpu_8bit, gpu_8bit, stream);
            stream.waitForCompletion();
        }

        //Constrat IN GPU
        if (ntuscope->YSapply_contrast) {
            cv::Mat LUT_matrix(1, 256, CV_8UC1);
            uchar* pointer = LUT_matrix.ptr();
            for (int i = 0; i < 256; i++) {
                pointer[i] = (int)(pow((double)(i / 255.0), ntuscope->YSgamma) * 255.0);
            }
            cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
            cudaLookUpTable->transform(gpu_8bit, gpu_8bit, stream);
            //cv::cuda::equalizeHist(gpu_8bit, gpu_8bit, stream);
            gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, ntuscope->YSconstrat, ntuscope->YSbrightness - 150, stream);
            stream.waitForCompletion();
        }
        
        //Filter apply
        ApplyFilter(gpu_8bit, stream);
        stream.waitForCompletion();
        
        //Resize
        cv::cuda::GpuMat gpu_8bitresizedImage;
        cv::cuda::resize(gpu_8bit, gpu_8bitresizedImage, cv::Size(ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display), 0, 0, cv::INTER_AREA);//cv::INTER_AREAcv::INTER_LINEAR
        stream.waitForCompletion();

        //Optional
        //int shift_px = 6;
        //int rows = gpu_8bitresizedImage.rows;
        //int cols = gpu_8bitresizedImage.cols;
        //cv::cuda::GpuMat shifted;
        //gpu_8bitresizedImage.copyTo(shifted, stream);

        //for (int r = 0; r < rows; r++) {
        //    if (r % 2 == 0) { 
        //        cv::cuda::GpuMat src_row = gpu_8bitresizedImage.row(r);
        //        cv::cuda::GpuMat dst_row = shifted.row(r);

        //        cv::cuda::GpuMat dst_left(dst_row, cv::Rect(0, 0, shift_px, 1));
        //        dst_left.setTo(cv::Scalar(0), stream);

        //        cv::cuda::GpuMat dst_right(dst_row, cv::Rect(shift_px, 0, cols - shift_px, 1));
        //        cv::cuda::GpuMat src_right(src_row, cv::Rect(0, 0, cols - shift_px, 1));
        //        src_right.copyTo(dst_right, stream);
        //    }
        //}
        //stream.waitForCompletion();
        //shifted.copyTo(gpu_8bitresizedImage, stream);

        //Save images
        if (ntuscope->savestate == true && SavedCount < ntuscope->save_num) {
            lock_guard<mutex> lock(saveMutex);
            if (ntuscope->saveprocessedstate == true) {
                gpu_8bitresizedImage.download(image_save);
            }
            else if (ntuscope->saveprocessedstate == false) {
                image_save = Img16bit.clone();
            }
            gpu_image_pre.download(image_saveraw);
            vector<U16> buffer_vector(Buffer, Buffer + samplesPerBuffer);
            saveQueue.emplace_back(image_save, image_saveraw, buffer_vector, starttime_system);
            SavedCount++;
            saveCondition.notify_one();
        }

        //Convert to RGB
        cv::cuda::GpuMat gpu_8bitRGB;
        gpu_8bitRGB.create(gpu_8bitresizedImage.size(), CV_8UC3);
        if (gpu_8bit.channels() == 1){
            cv::cuda::cvtColor(gpu_8bitresizedImage, gpu_8bitRGB, cv::COLOR_GRAY2BGR, 0, stream);
            stream.waitForCompletion();
        }

        //Display img generate
        cv::Mat finalImage;
        gpu_8bitRGB.download(finalImage);

        Addtext(finalImage, ntuscope->refreshtime, ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display);
        cv::imshow("Real-Time Image", finalImage);
        cv::waitKey(1);

        delete[] h_image;
        return true;
    }

    void YSMapping::WarmupLissajousImageProcessing() {
        int dummy_buffer_size = samplesPerBuffer;
        int dummy_image_size = ntuscope->Pixelnumx * ntuscope->Pixelnumy;
        U16* dummyBuffer = new U16[dummy_buffer_size];
        memset(dummyBuffer, 65535, dummy_buffer_size * sizeof(U16));
        auto dummyStartTime = chrono::high_resolution_clock::now();
        auto dummyStartTime_system = system_clock::now();
        cv::Mat dummyImage = cv::Mat::zeros(ntuscope->Ypixelsize_display, ntuscope->Xpixelsize_display, CV_8UC3);
        cv::imshow("Real-Time Image", dummyImage);
        cv::waitKey(1); 

        ProcessBufferAndFill_2DLissajousImage(dummyBuffer, dummyStartTime, dummyStartTime_system, 0);

        delete[] dummyBuffer;
    }
//Function
    bool YSMapping::Saveimage_RASTER(cv::Mat final_display_img,cv::Mat final_display_img_raw, vector<U16>& buffer_save, system_clock::time_point starttime_system) {
        if (ntuscope->savestate == true && imageIndex ==0) {
            auto ms = duration_cast<chrono::milliseconds>(starttime_system.time_since_epoch()) % 1000;
            time_t time_now = chrono::system_clock::to_time_t(starttime_system);
            tm local_tm;
            localtime_s(&local_tm, &time_now);

            ostringstream oss;
            oss << put_time(&local_tm, "%Y%m%d_%H%M%S")<< '_' << setfill('0') << setw(3) << ms.count();
            ntuscope->timestampFolder = oss.str();

            ntuscope->folderPath = "C:\\NTU_miniScope_Imaging\\2DRASTER\\" + ntuscope->timestampFolder + "_" + ntuscope->titleFolder;
            ntuscope->folderPathRaw = "C:\\NTU_miniScope_Imaging\\2DRASTER_RAW\\" + ntuscope->timestampFolder + "_" + ntuscope->titleFolder;
            ntuscope->folderPathRawBin = ntuscope->folderPathRaw + "\\BIN";

            CreateDirectoryA(ntuscope->folderPath.c_str(), NULL);
            if (ntuscope->saverawstate) {
                CreateDirectoryA(ntuscope->folderPathRaw.c_str(), NULL);
                if (ntuscope->saverawbinstate == true) {
                    CreateDirectoryA(ntuscope->folderPathRawBin.c_str(), NULL);
                }
            }
                
        }

        ostringstream filePath;
        ostringstream filePathraw;
        ostringstream filePathrawbin;
        filePath << ntuscope->folderPath << "\\Image" << imageIndex + 1 << ".tiff";
        filePathraw << ntuscope->folderPathRaw << "\\Image" << imageIndex + 1 << ".tiff";
        filePathrawbin << ntuscope->folderPathRawBin << "\\Image" << imageIndex + 1 << ".bin";
        if (imageIndex == 0) {
            Savecsv(RASTER);
        }

        //final_display_img
        //resize
        if (ntuscope->saveprocessedstate == true) {
            if (!cv::imwrite(filePath.str(), final_display_img)) {
                cerr << "Error: Unable to save image to " << filePath.str() << endl;
                return false;
            }
        }
        else if (ntuscope->saveprocessedstate == false) {
            cv::cuda::GpuMat gpu_Image;
            cv::cuda::GpuMat gpu_resizedImage;
            gpu_Image.upload(final_display_img);
            cv::cuda::resize(gpu_Image, gpu_resizedImage, cv::Size(ntuscope->Xpixelsize_save, ntuscope->Ypixelsize_save), 0, 0, cv::INTER_LINEAR);
            cv::Mat finalImage;
            gpu_resizedImage.download(finalImage);
            if (!cv::imwrite(filePath.str(), finalImage)) {
                cerr << "Error: Unable to save image to " << filePath.str() << endl;
                return false;
            }
        }

        //final_display_raw
        if (ntuscope->saverawstate == true) {
            cv::cuda::GpuMat gpu_Imageraw;
            gpu_Imageraw.upload(final_display_img_raw);
            if (ntuscope->YSapply_signalprocessing == false) {
                // INITIAL WINDOWING | Zero-negVolt | 32768-0
                cv::cuda::subtract(65535, gpu_Imageraw, gpu_Imageraw, cv::noArray(), -1, stream);
                cv::cuda::threshold(gpu_Imageraw, gpu_Imageraw, 65535, 0, cv::THRESH_TRUNC, stream);
                cv::cuda::subtract(gpu_Imageraw, 32768, gpu_Imageraw, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_Imageraw, 2.0, gpu_Imageraw, 1.0, -1, stream);
            }
            else if (ntuscope->YSapply_signalprocessing == true) {
                cv::cuda::subtract(65535, gpu_Imageraw, gpu_Imageraw, cv::noArray(), -1, stream);
                cv::cuda::threshold(gpu_Imageraw, gpu_Imageraw, 65535, 0, cv::THRESH_TRUNC, stream);
                cv::cuda::subtract(gpu_Imageraw, ntuscope->YSoffsetvalueSP, gpu_Imageraw, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_Imageraw, ntuscope->YSmultiplySP, gpu_Imageraw, 1.0, -1, stream);
            }
            cv::Mat finalImage_raw;
            gpu_Imageraw.download(finalImage_raw);
            if (!cv::imwrite(filePathraw.str(), finalImage_raw)) {
                cerr << "Error: Unable to save pixel data to " << filePathraw.str() << endl;
                return false;
            }

            //final_display_raw_bin
            if (ntuscope->saverawbinstate == true) {
                FILE* fpData = fopen(filePathrawbin.str().c_str(), "wb");
                if (fpData) {
                    size_t bytesPerBuffer = samplesPerBuffer * sizeof(U16);
                    size_t bytesWritten = fwrite(buffer_save.data(), sizeof(BYTE), bytesPerBuffer, fpData);
                    fclose(fpData);
                    if (bytesWritten != bytesPerBuffer) {
                        return false;
                    }
                }
                else {
                    return false;
                }
            }
        }

        if (imageIndex == ntuscope->save_num - 1) {
            ntuscope->savestate = false;
            ntuscope->saverawstate = false;
        }
        imageIndex++;
        ntuscope->saveprogress = (double(imageIndex) / double(ntuscope->save_num)) * 100; 

        return true;
    }

    bool YSMapping::Saveimage_LISSAJOUS(cv::Mat final_display_img, cv::Mat final_display_raw, system_clock::time_point starttime_system) {
        if (ntuscope->savestate == true && imageIndex == 0) {
            auto ms = duration_cast<milliseconds>(starttime_system.time_since_epoch()) % 1000;
            time_t time_now = system_clock::to_time_t(starttime_system);
            tm local_tm;
            localtime_s(&local_tm, &time_now);

            ostringstream oss;
            oss << put_time(&local_tm, "%Y%m%d_%H%M%S")<< '_' << setfill('0') << setw(3) << ms.count();
            ntuscope->timestampFolder = oss.str();


            ntuscope->folderPath = "C:\\NTU_miniScope_Imaging\\2DLISSAJOUS\\" + ntuscope->timestampFolder + "_" + ntuscope->titleFolder;
            ntuscope->folderPathRaw = "C:\\NTU_miniScope_Imaging\\2DLISSAJOUS_RAW\\" + ntuscope->timestampFolder + "_" + ntuscope->titleFolder;

            CreateDirectoryA(ntuscope->folderPath.c_str(), NULL);
            if (ntuscope->saverawstate) {
                CreateDirectoryA(ntuscope->folderPathRaw.c_str(), NULL);
            }
        }

        ostringstream filePath;
        ostringstream filePathraw;
        filePath << ntuscope->folderPath << "\\Image" << imageIndex + 1 << ".tiff";
        filePathraw << ntuscope->folderPathRaw << "\\Image" << imageIndex + 1 << ".tiff";
        if (imageIndex == 0) {
            Savecsv(LISSAJOUS_2D);
        }

        //final_display_img
        if (!cv::imwrite(filePath.str(), final_display_img)) {
            cerr << "Error: Unable to save image to " << filePath.str() << endl;
            return false;
        }

        //final_display_raw
        if (ntuscope->saverawstate == true) {
            cv::cuda::GpuMat gpu_saveimage(1, int(samplesPerBuffer), CV_16UC1);
            gpu_saveimage.upload(final_display_raw);

            if (ntuscope->YSapply_signalprocessing == false) {
                // INITIAL WINDOWING | Zero-negVolt | 32768-0
                cv::cuda::subtract(65535, gpu_saveimage, gpu_saveimage, cv::noArray(), -1, stream);
                cv::cuda::threshold(gpu_saveimage, gpu_saveimage, 65535, 0, cv::THRESH_TRUNC, stream);
                cv::cuda::subtract(gpu_saveimage, 32768, gpu_saveimage, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_saveimage, 2.0, gpu_saveimage, 1.0, -1, stream);
            }
            else if (ntuscope->YSapply_signalprocessing == true) {
                cv::cuda::subtract(65535, gpu_saveimage, gpu_saveimage, cv::noArray(), -1, stream);
                cv::cuda::threshold(gpu_saveimage, gpu_saveimage, 65535, 0, cv::THRESH_TRUNC, stream);
                cv::cuda::subtract(gpu_saveimage, ntuscope->YSoffsetvalueSP, gpu_saveimage, cv::noArray(), -1, stream);
                cv::cuda::multiply(gpu_saveimage, ntuscope->YSmultiplySP, gpu_saveimage, 1.0, -1, stream);
            }
            stream.waitForCompletion();

            cv::Mat finalsaveImage;
            gpu_saveimage.download(finalsaveImage);

            if (!cv::imwrite(filePathraw.str(), finalsaveImage)) {
                cerr << "Error: Unable to save pixel data to " << filePathraw.str() << endl;
                return false;
            }
        }

        if (imageIndex == ntuscope->save_num - 1) {
            ntuscope->savestate = false;
            ntuscope->saverawstate = false;
        }
        imageIndex++;
        ntuscope->saveprogress = (double(imageIndex) / double(ntuscope->save_num)) * 100;

        return true;
    }

    bool YSMapping::Saveimage_LISSAJOUS_highspeed(const vector<U16>& saveimage_data, system_clock::time_point starttime_system) {
        if (ntuscope->savestate == true && imageIndex == 0) {
            auto ms = duration_cast<chrono::milliseconds>(starttime_system.time_since_epoch()) % 1000;
            time_t time_now = chrono::system_clock::to_time_t(starttime_system);
            tm local_tm;
            localtime_s(&local_tm, &time_now);

            ostringstream oss;
            oss << put_time(&local_tm, "%Y%m%d_%H%M%S") << '_' << setfill('0') << setw(3) << ms.count();
            ntuscope->timestampFolder = oss.str();

            ntuscope->folderPathRaw = "C:\\NTU_miniScope_Imaging\\2DLISSAJOUS_RAW\\" + ntuscope->timestampFolder + "_" + ntuscope->titleFolder;

            CreateDirectoryA(ntuscope->folderPathRaw.c_str(), NULL);
            Savecsv(LISSAJOUS_2D);
        }

        cv::Mat saveimage(1, static_cast<int>(saveimage_data.size()), CV_16UC1, const_cast<U16*>(saveimage_data.data()));

        cv::cuda::GpuMat gpu_saveimage;
        gpu_saveimage.upload(saveimage);

        if (ntuscope->YSapply_signalprocessing == false) {
            // INITIAL WINDOWING | Zero-negVolt | 32768-0
            cv::cuda::subtract(65535, gpu_saveimage, gpu_saveimage, cv::noArray(), -1, stream);
            cv::cuda::threshold(gpu_saveimage, gpu_saveimage, 65535, 0, cv::THRESH_TRUNC, stream);
            cv::cuda::subtract(gpu_saveimage, 32768, gpu_saveimage, cv::noArray(), -1, stream);
            cv::cuda::multiply(gpu_saveimage, 2.0, gpu_saveimage, 1.0, -1, stream);
        }
        else if (ntuscope->YSapply_signalprocessing == true) {
            cv::cuda::subtract(65535, gpu_saveimage, gpu_saveimage, cv::noArray(), -1, stream);
            cv::cuda::threshold(gpu_saveimage, gpu_saveimage, 65535, 0, cv::THRESH_TRUNC, stream);
            cv::cuda::subtract(gpu_saveimage, ntuscope->YSoffsetvalueSP, gpu_saveimage, cv::noArray(), -1, stream);
            cv::cuda::multiply(gpu_saveimage, ntuscope->YSmultiplySP, gpu_saveimage, 1.0, -1, stream);
        }
        stream.waitForCompletion();

        cv::Mat finalsaveImage;
        gpu_saveimage.download(finalsaveImage);

        ostringstream filePathraw;
        filePathraw << ntuscope->folderPathRaw << "\\Image" << imageIndex + 1 << ".tiff";
        if (!cv::imwrite(filePathraw.str(), finalsaveImage)) {
            cerr << "Error: Unable to save pixel data to " << filePathraw.str() << endl;
        }
        if (imageIndex == ntuscope->save_num - 1) {
            ntuscope->savestate = false;
            ntuscope->saverawstate = false;
        }
        imageIndex++;
        ntuscope->saveprogress = (double(imageIndex) / double(ntuscope->save_num)) * 100;
        
        return true;
    }

    bool YSMapping::Saveimage_LISSAJOUSBin_highspeed(U16* Buffer, system_clock::time_point starttime_system) {
        if (ntuscope->savestate == true && imageIndex == 0) {
            auto ms = duration_cast<std::chrono::milliseconds>(starttime_system.time_since_epoch()) % 1000;
            std::time_t time_now = std::chrono::system_clock::to_time_t(starttime_system);
            tm local_tm;
            localtime_s(&local_tm, &time_now);

            std::ostringstream oss;
            oss << std::put_time(&local_tm, "%Y%m%d_%H%M%S") << '_' << std::setfill('0') << std::setw(3) << ms.count();
            ntuscope->timestampFolder = oss.str();

            ntuscope->folderPathRaw = "C:\\NTU_miniScope_Imaging\\2DLISSAJOUS_RAW\\" + ntuscope->timestampFolder + "_" + ntuscope->titleFolder;
            CreateDirectoryA(ntuscope->folderPathRaw.c_str(), NULL);

            Savecsv(LISSAJOUS_2D);
        }

        std::ostringstream filePath;
        filePath << ntuscope->folderPathRaw << "\\Image" << (imageIndex + 1) << ".bin";
        FILE* fpData = fopen(filePath.str().c_str(), "wb");
        if (fpData) {
            size_t bytesPerBuffer = samplesPerBuffer * sizeof(U16);
            size_t bytesWritten = fwrite(Buffer, sizeof(BYTE), bytesPerBuffer, fpData);
            fclose(fpData);

            if (bytesWritten != bytesPerBuffer) {
                std::cerr << "Warning: fwrite wrote " << bytesWritten << " bytes, expected " << bytesPerBuffer << std::endl;
            }
        }
        else {
            std::cerr << "Error: Unable to open file " << filePath.str() << " for writing.\n";
        }

        if (imageIndex == ntuscope->save_num - 1) {
            ntuscope->savestate = false;
            ntuscope->saverawstate = false;
        }
        imageIndex++;
        ntuscope->saveprogress = (double(imageIndex) / double(ntuscope->save_num)) * 100;
        return true;
    }

    void YSMapping::SaverasterimageFunction() {
        while (!stopsaveThread) {
            unique_lock<mutex> lock(saveMutex);
            saveCondition.wait(lock, [this]() {
                return !saveQueue.empty() || stopsaveThread;
                });

            if (stopsaveThread) break;

            auto imageWithTime = std::move(saveQueue.front());
            saveQueue.pop_front();

            cv::Mat Img, rawImg;
            vector<U16> Buffer;

            std::chrono::system_clock::time_point timestamp;
            std::tie(Img, rawImg,Buffer, timestamp) = imageWithTime;
            Saveimage_RASTER(Img, rawImg,Buffer, timestamp);

        }
    }

    void YSMapping::SavelissajousimageFunction() {
        while (!stopsaveThread) {
            if (ntuscope->highspeedmode == false) {
                unique_lock<mutex> lock(saveMutex);
                saveCondition.wait(lock, [this]() {
                    return !saveLissajousQueue.empty() || stopsaveThread;
                    });

                if (stopsaveThread) break;

                auto imageWithTime = std::move(saveLissajousQueue.front());
                saveLissajousQueue.pop_front();

                cv::Mat rawImg, processedImg;
                std::chrono::system_clock::time_point timestamp;
                std::tie(processedImg, rawImg, timestamp) = imageWithTime;
                Saveimage_LISSAJOUS(processedImg, rawImg, timestamp);
            }
            else if (ntuscope->highspeedmode == true) {
                unique_lock<mutex> lock(saveMutex);
                saveCondition.wait(lock, [this]() {
                    return !saveLissajousQueue_highspeed.empty() || stopsaveThread;
                    });

                if (stopsaveThread) break;

                pair<vector<U16>, std::chrono::system_clock::time_point> imageWithTime;
                imageWithTime = std::move(saveLissajousQueue_highspeed.front());
                saveLissajousQueue_highspeed.pop_front();
                Saveimage_LISSAJOUS_highspeed(imageWithTime.first, imageWithTime.second);
            }
        }
    }

    void YSMapping::AverageFunction() {
        while (!stopavgThread) {
            unique_lock<mutex> lock(queueMutex);
            queueCondition.wait(lock, [this]() {
                return !imageQueue.empty() || stopavgThread;
                });

            if (stopavgThread) break;

            if (imageQueue.size() >= ntuscope->avgcount) {
                vector<cv::Mat> localImages(imageQueue.begin(), imageQueue.begin() + ntuscope->avgcount);
                imageQueue.erase(imageQueue.begin(), imageQueue.begin() + ntuscope->avgcount);  
                lock.unlock();  

                //Avg Img (convert to float)
                cv::Mat averageImage = cv::Mat::zeros(localImages[0].size(), CV_64FC1);
                for (const auto& img : localImages) {
                    cv::Mat temp;
                    img.convertTo(temp, CV_64FC1);
                    averageImage += temp;
                }
                cv::divide(averageImage,cv::Scalar::all(ntuscope->avgcount),averageImage,1.0,-1);

                //Convert to 16-bit
                cv::Mat AvgImg16bit;
                averageImage.convertTo(AvgImg16bit, CV_16UC1);
                cv::cuda::GpuMat AvgImg16bit_gpu;
                AvgImg16bit_gpu.upload(AvgImg16bit);
                
                //DCE
                if (ntuscope->YSapply_DCE) {
                    cv::cuda::GpuMat DCE;
                    Dynamic_Contrast_Enhancement_CUDA(AvgImg16bit_gpu, DCE, ntuscope->YSalpha_DCE);
                    DCE.copyTo(AvgImg16bit_gpu);
                }

                //Enhance Contrast 0.0035
                if (ntuscope->YSapply_enhancecontrast) {
                    cv::cuda::GpuMat ECTEMP;
                    EnhanceContrast_16bit(AvgImg16bit_gpu, ECTEMP,0.0035);
                    ECTEMP.copyTo(AvgImg16bit_gpu);
                }

                //MinMaxValue
                if (ntuscope->YSapply_minmax == true) {
                    double scale = 65535.0 / (ntuscope->maxVal16bit - ntuscope->minVal16bit);
                    double shift = -ntuscope->minVal16bit * scale;
                    AvgImg16bit_gpu.convertTo(AvgImg16bit_gpu, -1, scale, shift);
                    cv::cuda::threshold(AvgImg16bit_gpu, AvgImg16bit_gpu, 65535, 65535, cv::THRESH_TRUNC, stream);
                    cv::cuda::threshold(AvgImg16bit_gpu, AvgImg16bit_gpu, 0, 0, cv::THRESH_TOZERO, stream);
                    stream.waitForCompletion();
                }

                //Normalize
                if (ntuscope->YSapply_normalize == true) {
                    cv::cuda::normalize(AvgImg16bit_gpu, AvgImg16bit_gpu, ntuscope->normalize_alpha, ntuscope->normalize_beta, cv::NORM_MINMAX, -1, cv::noArray(), stream);
                }

                //Binning
                if (ntuscope->YSapply_binning == true) {
                    int sizeRaw_width = int(samplesPerRecord);
                    int size_width = ntuscope->Xpixelsize_display;
                    int sizeRaw_height = ntuscope->Pixelnumy;
                    int size_height = ntuscope->Ypixelsize_display;
                    if (sizeRaw_width > ntuscope->Xpixelsize_display && size_height >= sizeRaw_height) {
                        cv::cuda::GpuMat RES_UP;
                        int binPixel_x = int(sizeRaw_width / size_width) + 1;
                        int binPixel_y = int(size_height / sizeRaw_height);
                        cv::cuda::resize(AvgImg16bit_gpu, RES_UP, cv::Size((size_width * binPixel_x), (size_height * 1 / binPixel_y)), 0, 0, cv::INTER_AREA, stream);
                        cv::cuda::resize(RES_UP, AvgImg16bit_gpu, RES_UP.size() / 1, 0, 0, cv::INTER_AREA, stream);
                    }
                    else {
                        cv::cuda::resize(AvgImg16bit_gpu, AvgImg16bit_gpu, cv::Size(size_width, size_height), 0, 0, cv::INTER_CUBIC, stream);
                    }
                    stream.waitForCompletion();
                }

                //Convert to 8-bit
                cv::cuda::GpuMat AvgImg8bit_gpu;
                //cv::cuda::normalize(AvgImg16bit_gpu, AvgImg8bit_gpu, 0, 255, cv::NORM_MINMAX, CV_8UC1);
                AvgImg16bit_gpu.convertTo(AvgImg8bit_gpu, CV_8UC1, 1.0 /257.0 ,0.0);

                //NIMeanDenoise**
                //cv::Mat averageImage_8U;
                //AvgImg8bit_gpu.download(averageImage_8U);
                //cv::Mat denoised;
                //cv::fastNlMeansDenoising(averageImage_8U, denoised, 10.0, 7, 21);
                //AvgImg8bit_gpu.upload(denoised);

                // Pre Imaging Processing
                //cv::Mat LUT_matrix_pre(1, 256, CV_8UC1);
                //uchar* pointer_pre = LUT_matrix_pre.ptr();
                //for (int i = 0; i < 256; i++) {
                //    pointer_pre[i] = (int)(pow((double)(i / 255.0), 1.1) * 255.0);
                //}
                //cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable_pre = cv::cuda::createLookUpTable(LUT_matrix_pre);
                //cudaLookUpTable_pre->transform(AvgImg8bit_gpu, AvgImg8bit_gpu, stream);
                //AvgImg8bit_gpu.cv::cuda::GpuMat::convertTo(AvgImg8bit_gpu, -1, 2.6, 0, stream);
                //stream.waitForCompletion();

                //Histogram equalize
                if (ntuscope->YSapply_equalizehistogram) {
                    cv::cuda::equalizeHist(AvgImg8bit_gpu, AvgImg8bit_gpu, stream);
                    stream.waitForCompletion();
                }

                // Constrat IN GPU
                if (ntuscope->YSapply_contrast) {
                    cv::Mat LUT_matrix(1, 256, CV_8UC1);
                    uchar* pointer = LUT_matrix.ptr();
                    for (int i = 0; i < 256; i++) {
                        pointer[i] = (int)(pow((double)(i / 255.0), ntuscope->YSgamma) * 255.0);
                    }
                    cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
                    cudaLookUpTable->transform(AvgImg8bit_gpu, AvgImg8bit_gpu, stream);
                    AvgImg8bit_gpu.cv::cuda::GpuMat::convertTo(AvgImg8bit_gpu, -1, ntuscope->YSconstrat, ntuscope->YSbrightness - 150, stream);
                    stream.waitForCompletion();
                }

                cv::Size clahe_tile_grid(8,8);
                cv::Ptr< cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(3.0, clahe_tile_grid);
                clahe->apply(AvgImg8bit_gpu, AvgImg8bit_gpu);

                // Filter
                ApplyFilter(AvgImg8bit_gpu, stream);
                stream.waitForCompletion();

                // Resize
                cv::cuda::GpuMat gpu_8bitresizedImage;
                cv::cuda::resize(AvgImg8bit_gpu, gpu_8bitresizedImage, cv::Size(ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display), 0, 0, cv::INTER_AREA);

                // Convert to RGB
                cv::cuda::GpuMat gpu_8bitRGB;
                gpu_8bitRGB.create(gpu_8bitresizedImage.size(), CV_8UC3);
                if (gpu_8bitresizedImage.channels() == 1) {
                    cv::cuda::cvtColor(gpu_8bitresizedImage, gpu_8bitRGB, cv::COLOR_GRAY2BGR);
                }
                
                cv::Mat finalAverageImage;
                gpu_8bitRGB.download(finalAverageImage);

                Addtext_Avg(finalAverageImage, ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display);
                cv::imshow("Average Image", finalAverageImage);
                cv::waitKey(1);
            }
        }
    }

    void YSMapping::RepairFunction() {
        while (!stoprepairThread) {
            unique_lock<mutex> lock(repairMutex);
            repairCondition.wait(lock, [this]() {
                return !repairQueue.empty() || stoprepairThread;
                });

            if (stoprepairThread) break;

            if (repairQueue.size() >= 200) {
                vector<cv::cuda::GpuMat> localImages(repairQueue.begin(), repairQueue.begin() + 200);
                repairQueue.erase(repairQueue.begin(), repairQueue.begin() + 200);
                lock.unlock();

                //Avg Img (convert to float)
                cv::cuda::GpuMat sumImage;
                cv::cuda::GpuMat averageImage;
                sumImage.create(localImages[0].size(), CV_64FC1);
                sumImage.setTo(cv::Scalar::all(0));

                for (const auto& gpuImage : localImages) {
                    cv::cuda::GpuMat temp;
                    gpuImage.convertTo(temp, CV_64FC1);
                    cv::cuda::add(sumImage, temp, sumImage);
                }
                cv::cuda::divide(sumImage, cv::Scalar::all(200), averageImage);

                //Convert to 16-bit
                cv::cuda::GpuMat AvgImg;
                averageImage.convertTo(AvgImg, CV_16UC1);

                //Convert to 8-bit
                cv::cuda::GpuMat gpu_8bit;
                averageImage.convertTo(gpu_8bit, CV_8UC1, 1 / 257.0, 0, stream);

                // Constrat IN GPU
                cv::Mat LUT_matrix(1, 256, CV_8UC1);
                uchar* pointer = LUT_matrix.ptr();
                for (int i = 0; i < 256; i++) {
                    pointer[i] = (int)(pow((double)(i / 255.0), ntuscope->YSgamma) * 255.0);
                }
                cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
                cudaLookUpTable->transform(gpu_8bit, gpu_8bit, stream);
                gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, ntuscope->YSconstrat, ntuscope->YSbrightness - 150, stream);

                // Repair
                cv::Mat lissajousrepairImg;
                cv::cuda::GpuMat repairedResult;
                gpu_8bit.download(lissajousrepairImg);
                LissajousRepairment(lissajousrepairImg, repairedResult);

                // Resize
                cv::cuda::GpuMat gpu_8bitresizedImage;
                cv::cuda::resize(repairedResult, gpu_8bitresizedImage, cv::Size(ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display), 0, 0, cv::INTER_LINEAR);
                stream.waitForCompletion();

                // Convert to RGB
                cv::cuda::GpuMat gpu_8bitRGB;
                gpu_8bitRGB.create(gpu_8bitresizedImage.size(), CV_8UC3);
                if (gpu_8bitresizedImage.channels() == 1) {
                    cv::cuda::cvtColor(gpu_8bitresizedImage, gpu_8bitRGB, cv::COLOR_GRAY2BGR);
                }


                if (ntuscope->YSapply_repair==true) {
                    cv::Mat finalImage;
                    gpu_8bitRGB.download(finalImage);
                    Addtext_Repair(finalImage, ntuscope->Xpixelsize_display, ntuscope->Ypixelsize_display);
                    cv::imshow("Repaired Image", finalImage);
                    cv::waitKey(1);
                }

            }
        }
    }

    void YSMapping::Addtext(cv::Mat& IMGDP8, double rate, int Xpixelsize, int Ypixelsize) {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        int thickness = 1;
        int baseline = 0;

        ostringstream oss;
        oss << fixed << setprecision(1) << rate;
        string text = oss.str() + " FPS";
        cv::Size textSize = cv::getTextSize(text, fontFace, ntuscope->fontScale, thickness, &baseline);
        cv::Point textOrg(IMGDP8.cols - textSize.width - 10, IMGDP8.rows - 10);
        cv::putText(IMGDP8, text, textOrg, fontFace, ntuscope->fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);

        ostringstream oss2;
        oss2 << "CH1 | " << Xpixelsize << " x " << Ypixelsize;
        string chText = oss2.str();
        cv::Size chSize = cv::getTextSize(chText, fontFace, ntuscope->fontScale, thickness, &baseline);
        cv::Point chOrg(10, chSize.height + 10);
        cv::putText(IMGDP8, chText, chOrg, fontFace, ntuscope->fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);
    }

    void YSMapping::Addtext_Avg(cv::Mat& IMGDP8, int Xpixelsize, int Ypixelsize) {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        int thickness = 1;
        int baseline = 0;

        ostringstream oss;
        oss << "FRAME AVG: " << ntuscope->avgcount;
        string text = oss.str();
        cv::Size textSize = cv::getTextSize(text, fontFace, ntuscope->fontScale, thickness, &baseline);
        cv::Point textOrg(IMGDP8.cols - textSize.width - 10, IMGDP8.rows - 10);
        cv::putText(IMGDP8, text, textOrg, fontFace, ntuscope->fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);

        ostringstream oss2;
        oss2 << "CH1 | " << Xpixelsize << " x " << Ypixelsize;
        string chText = oss2.str();
        cv::Size chSize = cv::getTextSize(chText, fontFace, ntuscope->fontScale, thickness, &baseline);
        cv::Point chOrg(10, chSize.height + 10);
        cv::putText(IMGDP8, chText, chOrg, fontFace, ntuscope->fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);
    }

    void YSMapping::Addtext_Repair(cv::Mat& IMGDP8, int Xpixelsize, int Ypixelsize) {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        int thickness = 1;
        int baseline = 0;

        ostringstream oss;
        oss << "Impaint" ;
        string text = oss.str();
        cv::Size textSize = cv::getTextSize(text, fontFace, ntuscope->fontScale, thickness, &baseline);
        cv::Point textOrg(IMGDP8.cols - textSize.width - 10, IMGDP8.rows - 10);
        cv::putText(IMGDP8, text, textOrg, fontFace, ntuscope->fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);

        ostringstream oss2;
        oss2 << "CH1 | " << Xpixelsize << " x " << Ypixelsize;
        string chText = oss2.str();
        cv::Size chSize = cv::getTextSize(chText, fontFace, ntuscope->fontScale, thickness, &baseline);
        cv::Point chOrg(10, chSize.height + 10);
        cv::putText(IMGDP8, chText, chOrg, fontFace, ntuscope->fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);
    }

    void YSMapping::Savecsv(ScanMode scanmode) {
        if (scanmode == LISSAJOUS_2D) {
            if (ntuscope->highspeedmode == false) {
                string csvpath = ntuscope->folderPath + "\\scan_spec.csv";
                ofstream file(csvpath);
                file << "xfreq," << ntuscope->xfreq << endl;
                file << "yfreq," << ntuscope->yfreq << endl;
                file << "xamp," << ntuscope->MEMS_xamp << endl;
                file << "yamp," << ntuscope->MEMS_yamp << endl;
                file << "phase," << ntuscope->lissajous_phase << endl;
                file << "samplingrate," << ntuscope->samplesPerSec << endl;
                file << "num of samples," << int(samplesPerBuffer) << endl;
                if (ntuscope->inputrangetype == INPUT_RANGE_PM_500_MV) {
                    file << "inputrange," << 500 << endl;
                }
                else if (ntuscope->inputrangetype == INPUT_RANGE_PM_800_MV) {
                    file << "inputrange," << 800 << endl;
                }
                else if (ntuscope->inputrangetype == INPUT_RANGE_PM_1_V) {
                    file << "inputrange," << 1000 << endl;
                }
                file << "chopperfreq," << ntuscope->chopperfreq << endl;
                file << "chopperphase," << ntuscope->chopperphase << endl;
                file.close();
                cout << "CSV file created successfully at: " << csvpath << endl;
            }
            string rawcsvpath = ntuscope->folderPathRaw + "\\scan_spec.csv";
            ofstream rawfile(rawcsvpath);
            rawfile << "xfreq," << ntuscope->xfreq << endl;
            rawfile << "yfreq," << ntuscope->yfreq << endl;
            rawfile << "xamp," << ntuscope->MEMS_xamp << endl;
            rawfile << "yamp," << ntuscope->MEMS_yamp << endl;
            rawfile << "phase," << ntuscope->lissajous_phase << endl;
            rawfile << "samplingrate," << ntuscope->samplesPerSec << endl;
            rawfile << "num of samples," << int(samplesPerBuffer) << endl;
            if (ntuscope->inputrangetype == INPUT_RANGE_PM_500_MV) {
                rawfile << "inputrange," << 500 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_800_MV) {
                rawfile << "inputrange," << 800 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_1_V) {
                rawfile << "inputrange," << 1000 << endl;
            }
            rawfile << "chopperfreq," << ntuscope->chopperfreq << endl;
            rawfile << "chopperphase," << ntuscope->chopperphase << endl;
            rawfile.close();
            cout << "CSV file created successfully at: " << rawcsvpath << endl;
        }
        if (scanmode == RASTER) {
            string csvpath = ntuscope->folderPath + "\\scan_spec.csv";
            ofstream file(csvpath);
            file << "linefreq," << ntuscope->xfreq *2 << endl;
            file << "num_of_line," << ntuscope->num_lines << endl;
            file << "xamp," << ntuscope->MEMS_xamp << endl;
            file << "yamp," << ntuscope->MEMS_yamp << endl;
            file << "samplingrate," << ntuscope->samplesPerSec << endl;
            file << "num of samples," << int(samplesPerBuffer) << endl;
            if (ntuscope->inputrangetype == INPUT_RANGE_PM_500_MV) {
                file << "inputrange," << 500 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_800_MV) {
                file << "inputrange," << 800 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_1_V) {
                file << "inputrange," << 1000 << endl;
            }
            file << "chopperfreq," << ntuscope->chopperfreq << endl;
            file << "chopperphase," << ntuscope->chopperphase << endl;
            file.close();
            cout << "CSV file created successfully at: " << csvpath << endl;
            string rawcsvpath = ntuscope->folderPathRaw + "\\scan_spec.csv";
            ofstream rawfile(rawcsvpath);
            rawfile << "linefreq," << ntuscope->xfreq * 2 << endl;
            rawfile << "num_of_line," << ntuscope->num_lines << endl;
            rawfile << "xamp," << ntuscope->MEMS_xamp << endl;
            rawfile << "yamp," << ntuscope->MEMS_yamp << endl;
            rawfile << "samplingrate," << ntuscope->samplesPerSec << endl;
            rawfile << "num of samples," << int(samplesPerBuffer) << endl;
            if (ntuscope->inputrangetype == INPUT_RANGE_PM_500_MV) {
                rawfile << "inputrange," << 500 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_800_MV) {
                rawfile << "inputrange," << 800 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_1_V) {
                rawfile << "inputrange," << 1000 << endl;
            }
            rawfile << "chopperfreq," << ntuscope->chopperfreq << endl;
            rawfile << "chopperphase," << ntuscope->chopperphase << endl;
            rawfile.close();
            cout << "CSV file created successfully at: " << rawcsvpath << endl;
        }
        else if (scanmode == LISSAJOUS_3D) {
            string csvpath = ntuscope->folderPath + "\\scan_spec.csv";
            ofstream file(csvpath);
            file << "xfreq," << ntuscope->xfreq << endl;
            file << "yfreq," << ntuscope->yfreq << endl;
            file << "zlayer," << ntuscope->zlayer << endl;
            file << "zstep," << ntuscope->zstep << endl;
            file << "xamp," << ntuscope->MEMS_xamp << endl;
            file << "yamp," << ntuscope->MEMS_yamp << endl;
            file << "samplingrate," << ntuscope->samplesPerSec << endl;
            file << "num of samples," << int(samplesPerBuffer) << endl;
            if (ntuscope->inputrangetype == INPUT_RANGE_PM_500_MV) {
                file << "inputrange," << 500 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_800_MV) {
                file << "inputrange," << 800 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_1_V) {
                file << "inputrange," << 1000 << endl;
            }
            file << "chopperfreq," << ntuscope->chopperfreq << endl;
            file << "chopperphase," << ntuscope->chopperphase << endl;
            file.close();
            cout << "CSV file created successfully at: " << csvpath << endl;
            string rawcsvpath = ntuscope->folderPathRaw + "\\scan_spec.csv";
            ofstream rawfile(rawcsvpath);
            rawfile << "xfreq," << ntuscope->xfreq << endl;
            rawfile << "yfreq," << ntuscope->yfreq << endl;
            rawfile << "zlayer," << ntuscope->zlayer << endl;
            rawfile << "zstep," << ntuscope->zstep << endl;
            rawfile << "xamp," << ntuscope->MEMS_xamp << endl;
            rawfile << "yamp," << ntuscope->MEMS_yamp << endl;
            rawfile << "samplingrate," << ntuscope->samplesPerSec << endl;
            rawfile << "num of samples," << int(samplesPerBuffer) << endl;
            if (ntuscope->inputrangetype == INPUT_RANGE_PM_500_MV) {
                rawfile << "inputrange," << 500 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_800_MV) {
                rawfile << "inputrange," << 800 << endl;
            }
            else if (ntuscope->inputrangetype == INPUT_RANGE_PM_1_V) {
                rawfile << "inputrange," << 1000 << endl;
            }
            rawfile << "chopperfreq," << ntuscope->chopperfreq << endl;
            rawfile << "chopperphase," << ntuscope->chopperphase << endl;
            rawfile.close();
            cout << "CSV file created successfully at: " << rawcsvpath << endl;
        }
    }

    void YSMapping::ApplyFilter(cv::cuda::GpuMat& IMGDP8, cv::cuda::Stream& streamCUDA) {
        // GAUSSIAN BLUR FILTER
        if (ntuscope->YSapply_gaussian){
            cv::Size kernelGaussian(ntuscope->YSgaussian_kernel, ntuscope->YSgaussian_kernel);
            cv::Ptr<cv::cuda::Filter> gaussian = cv::cuda::createGaussianFilter(IMGDP8.type(), IMGDP8.type(), kernelGaussian, ntuscope->YSgaussian_sigmaH, ntuscope->YSgaussian_sigmaV, cv::BORDER_DEFAULT, -1);
            gaussian->apply(IMGDP8, IMGDP8);
        }

        // MEDIAN BLUR FILTER
        if (ntuscope->YSapply_median){
            cv::cuda::GpuMat temp = IMGDP8.clone();
            cv::Ptr<cv::cuda::Filter> median = cv::cuda::createMedianFilter(IMGDP8.type(), ntuscope->YSmedian_kernel);
            median->apply(temp, IMGDP8);

        }

        // HISTOGRAM EQUALIZATION : CLAHE
        if (ntuscope->YSapply_CLAHE){
            cv::Size clahe_tile_grid(ntuscope->YSCLAHE_tile_grid_H, ntuscope->YSCLAHE_tile_grid_V);
            cv::Ptr< cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(ntuscope->YSCLAHE_clip_limit, clahe_tile_grid);
            clahe->apply(IMGDP8, IMGDP8);
        }
    }

    void YSMapping::EnhanceContrast_16bit(cv::cuda::GpuMat& IMGDP16_GPU, cv::cuda::GpuMat& IMGDP16OUTPUT_GPU, float saturated_percent) {
        CV_Assert(IMGDP16_GPU.type() == CV_16UC1);

        int histSize = 65536;
        float range[] = { 0, 65536 };
        const float* histRange = { range };

        cv::Mat IMGDP16_CPU;
        IMGDP16_GPU.download(IMGDP16_CPU);

        cv::Mat hist;
        cv::calcHist(&IMGDP16_CPU, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

        vector<float> cumulative_hist(histSize, 0);
        cumulative_hist[0] = hist.at<float>(0);
        for (int i = 1; i < histSize; ++i) {
            cumulative_hist[i] = cumulative_hist[i - 1] + hist.at<float>(i);
        }

        float total_pixels = cumulative_hist[histSize - 1];
        float low_cutoff = total_pixels * saturated_percent;
        float high_cutoff = total_pixels * (1.0f - saturated_percent);

        int low = 0, high = histSize - 1;
        while (low < histSize && cumulative_hist[low] < low_cutoff) ++low;
        while (high > 0 && cumulative_hist[high] > high_cutoff) --high;

        if (low >= high) {
            IMGDP16_GPU.copyTo(IMGDP16OUTPUT_GPU);
            return;
        }

        cv::cuda::GpuMat temp_GPU;
        IMGDP16_GPU.convertTo(temp_GPU, CV_32F); 

        cv::cuda::subtract(temp_GPU, static_cast<float>(low), temp_GPU);
        float scale = 65535.0f / (high - low);
        cv::cuda::multiply(temp_GPU, scale, temp_GPU);

        cv::cuda::threshold(temp_GPU, temp_GPU, 0, 0, cv::THRESH_TOZERO);
        cv::cuda::threshold(temp_GPU, temp_GPU, 65535, 65535, cv::THRESH_TRUNC);

        temp_GPU.convertTo(IMGDP16OUTPUT_GPU, CV_16U);
    }

    void YSMapping::EnhanceContrast_8bit(cv::cuda::GpuMat& IMGDP8_GPU, cv::cuda::GpuMat& IMGDP8OUTPUT_GPU, float saturated_percent) {
        int histSize = 256; 
        float range[] = { 0, 256 };
        const float* histRange = { range };

        cv::Mat IMGDP8_CPU;
        IMGDP8_GPU.download(IMGDP8_CPU);

        cv::Mat hist;
        cv::calcHist(&IMGDP8_CPU, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

        vector<float> cumulative_hist(histSize, 0);
        cumulative_hist[0] = hist.at<float>(0);
        for (int i = 1; i < histSize; ++i) {
            cumulative_hist[i] = cumulative_hist[i - 1] + hist.at<float>(i);
        }
        float total_pixels = cumulative_hist[histSize - 1];
        float low_cutoff = total_pixels * saturated_percent;
        float high_cutoff = total_pixels * (1.0f - saturated_percent);

        int low = 0, high = histSize - 1;
        while (low < histSize && cumulative_hist[low] < low_cutoff) ++low;
        while (high > 0 && cumulative_hist[high] > high_cutoff) --high;

        if (low >= high) {
            IMGDP8_GPU.copyTo(IMGDP8OUTPUT_GPU);
            return;
        }
        cv::cuda::GpuMat temp_GPU;
        IMGDP8_GPU.convertTo(temp_GPU, CV_32F); 

        cv::cuda::subtract(temp_GPU, static_cast<float>(low), temp_GPU);
        float scale = 255.0f / (high - low);
        cv::cuda::multiply(temp_GPU, scale, temp_GPU);

        cv::cuda::threshold(temp_GPU, temp_GPU, 0, 0, cv::THRESH_TOZERO);
        cv::cuda::threshold(temp_GPU, temp_GPU, 255, 255, cv::THRESH_TRUNC);

        temp_GPU.convertTo(IMGDP8OUTPUT_GPU, CV_8U);
    }

    void YSMapping::AdaptiveContrastEnhancement(cv::Mat& IMGDP16, cv::Mat& IMGDP16OUTPUT, int C, int n, float MaxCG) {
        //IMGDP16OUTPUT_GPU.create(IMGDP16_GPU.size(), IMGDP16_GPU.type());
        //int row = IMGDP16_GPU.rows;
        //int col = IMGDP16_GPU.cols;

        ////cv::Mat input;
        ////IMGDP16_GPU.download(input);
        ////cv::Mat meanLocal;
        ////cv::blur(input, meanLocal, cv::Size(n, n));
        ////cv::cuda::GpuMat d_meanLocal;
        ////d_meanLocal.upload(meanLocal);
        //cv::cuda::GpuMat d_meanLocal;
        //cv::cuda::createBoxFilter(IMGDP16_GPU.type(), -1, cv::Size(n, n), cv::Point(-1, -1), true)->apply(IMGDP16_GPU, d_meanLocal);

        //cv::cuda::GpuMat d_highFreq;
        //cv::cuda::subtract(IMGDP16_GPU, d_meanLocal, d_highFreq);

        //cv::cuda::GpuMat d_varLocal;
        //cv::cuda::multiply(d_highFreq, d_highFreq, d_varLocal);

        //cv::cuda::GpuMat d_varLocalSqrt;
        //cv::cuda::sqrt(d_varLocal, d_varLocalSqrt);

        ////cv::Mat meanGlobal, varGlobal;
        ////cv::meanStdDev(input, meanGlobal, varGlobal);
        //cv::Mat meanGlobal, varGlobal;
        //cv::meanStdDev(IMGDP16_GPU, meanGlobal, varGlobal);

        //cv::cuda::GpuMat d_meanGlobal;
        //d_meanGlobal.upload(meanGlobal);

        //cv::cuda::GpuMat d_gainArr;
        //cv::cuda::divide(d_meanGlobal, d_varLocalSqrt, d_gainArr);

        //cv::cuda::GpuMat d_maxGainArr;
        //cv::cuda::min(d_gainArr, MaxCG, d_maxGainArr);

        //cv::cuda::GpuMat d_enhanced;
        //cv::cuda::multiply(d_maxGainArr, d_highFreq, d_enhanced);

        //cv::cuda::GpuMat d_finalResult;
        //cv::cuda::add(d_meanLocal, d_enhanced, d_finalResult);

        //d_finalResult.copyTo(IMGDP16OUTPUT_GPU);

        int rows = IMGDP16.rows;
        int cols = IMGDP16.cols;
        cv::Mat srcFloat;
        IMGDP16.convertTo(srcFloat, CV_32F);
        cv::Mat meanLocal;
        cv::blur(srcFloat, meanLocal, cv::Size(n, n));
        // Compute high frequency
        cv::Mat highFreq = srcFloat - meanLocal;
        cv::Mat varLocal = highFreq.mul(highFreq);
        cv::sqrt(varLocal, varLocal);
        cv::Scalar meanGlobal, stddevGlobal;
        cv::meanStdDev(srcFloat, meanGlobal, stddevGlobal); 
        cv::Mat gainMap;
        gainMap = meanGlobal[0] / varLocal;
        cv::threshold(gainMap, gainMap, MaxCG, MaxCG, cv::THRESH_TRUNC);
        cv::Mat enhanced = meanLocal + gainMap.mul(highFreq);
        cv::Mat dst16U;
        enhanced.convertTo(dst16U, CV_16UC1);
        IMGDP16OUTPUT = dst16U.clone();
    }

    void YSMapping::Dynamic_Contrast_Enhancement(cv::Mat& IMGDP16, cv::Mat& IMGDP16OUTPUT, int slopeThreshold, int bins) {
        int rows = IMGDP16.rows;
        int cols = IMGDP16.cols;
        int levels = bins;
        int bin_size = 65536 / bins;

        // Step 1: Compute histogram
        std::vector<int> hist(levels, 0);
        for (int y = 0; y < rows; ++y) {
            const uint16_t* ptr = IMGDP16.ptr<uint16_t>(y);
            for (int x = 0; x < cols; ++x) {
                int idx = ptr[x] / bin_size;
                hist[idx]++;
            }
        }

        // Step 2: Slope Clipping
        std::vector<int> clippedHist = hist;
        for (int i = 1; i < levels; ++i) {
            int delta = clippedHist[i] - clippedHist[i - 1];
            if (std::abs(delta) > slopeThreshold) {
                clippedHist[i] = clippedHist[i - 1] + (delta > 0 ? slopeThreshold : -slopeThreshold);
            }
            clippedHist[i] = std::max(clippedHist[i], 0); // avoid negative values
        }

        // Step 3: Compute CDF
        std::vector<int> cdf(levels, 0);
        cdf[0] = clippedHist[0];
        for (int i = 1; i < levels; ++i) {
            cdf[i] = cdf[i - 1] + clippedHist[i];
        }

        // Step 4: Mapping table
        int totalPixels = rows * cols;
        int cdf_min = *std::find_if(cdf.begin(), cdf.end(), [](int v) { return v > 0; });
        std::vector<uint16_t> LUT(levels, 0);
        for (int i = 0; i < levels; ++i) {
            LUT[i] = static_cast<uint16_t>(65535.0 * (cdf[i] - cdf_min) / (totalPixels - cdf_min));
        }

        // Step 5: Map output
        IMGDP16OUTPUT = IMGDP16.clone();
        for (int y = 0; y < rows; ++y) {
            const uint16_t* inPtr = IMGDP16.ptr<uint16_t>(y);
            uint16_t* outPtr = IMGDP16OUTPUT.ptr<uint16_t>(y);
            for (int x = 0; x < cols; ++x) {
                int binIdx = inPtr[x] / bin_size;
                outPtr[x] = LUT[binIdx];
            }
        }
    }

    void YSMapping::Dynamic_Contrast_Enhancement_CUDA(cv::cuda::GpuMat& IMGDP16_GPU, cv::cuda::GpuMat& IMGDP16OUTPUT_GPU, float alpha_max) {
        IMGDP16_GPU.convertTo(IMGDP16_GPU, CV_32F);

        // === LAY1 ===
        cv::cuda::GpuMat RES1;
        cv::cuda::resize(IMGDP16_GPU, RES1, cv::Size(), 0.1, 0.1, cv::INTER_AREA);  

        cv::cuda::GpuMat BLR1;
        cv::Ptr<cv::cuda::Filter> gauss29 = cv::cuda::createGaussianFilter(CV_32F, CV_32F, cv::Size(29, 29), 0);
        gauss29->apply(RES1, BLR1); 

        cv::cuda::add(BLR1, cv::Scalar(1.0f), BLR1);  
        
        cv::cuda::GpuMat RES2;
        cv::cuda::resize(BLR1,RES2, cv::Size(IMGDP16_GPU.cols, IMGDP16_GPU.rows),0,0, cv::INTER_AREA);  

        cv::cuda::GpuMat DIV;
        cv::cuda::divide(0.9f * 65535.0f, RES2, DIV);

        cv::cuda::GpuMat LAY1;
        cv::cuda::min(DIV, cv::Scalar(alpha_max), LAY1);  

        // === LAY2 ===
        cv::cuda::GpuMat AMP1;
        cv::cuda::multiply(IMGDP16_GPU, LAY1, AMP1);

        cv::cuda::GpuMat RES3;
        cv::cuda::resize(AMP1, RES3, cv::Size(), 1.0 / 3.0, 1.0 / 3.0, cv::INTER_AREA);

        cv::cuda::GpuMat BLR2;
        gauss29->apply(RES3, BLR2);

        cv::cuda::GpuMat RES4;
        cv::cuda::resize(RES3, RES4, cv::Size(IMGDP16_GPU.cols, IMGDP16_GPU.rows), 0, 0, cv::INTER_AREA);

        cv::cuda::GpuMat SUB1;
        cv::cuda::subtract(RES4, AMP1, SUB1);

        cv::Ptr<cv::cuda::Filter> gauss7 = cv::cuda::createGaussianFilter(CV_32F, CV_32F, cv::Size(ntuscope->YSkernel_DCE, ntuscope->YSkernel_DCE), 0);
        cv::cuda::GpuMat BLR3;
        gauss7->apply(SUB1, BLR3);

        cv::cuda::GpuMat SUB2;
        cv::cuda::subtract(cv::Scalar(1.25f * alpha_max), LAY1, SUB2);

        cv::cuda::GpuMat AMP2;
        cv::cuda::multiply(BLR3, SUB2, AMP2);

        cv::cuda::GpuMat SUB3;
        cv::cuda::subtract(IMGDP16_GPU,AMP2,SUB3);

        // === LAY3 ===
        cv::cuda::GpuMat LAY3;
        cv::cuda::divide(LAY1, cv::Scalar(4.0f),LAY3); 
        cv::cuda::multiply(LAY3, LAY3, LAY3);        
        cv::cuda::add(LAY3, cv::Scalar(0.9f), LAY3);   

        cv::cuda::GpuMat AMP3;
        cv::cuda::multiply(SUB3, LAY3,AMP3);  

        // === Clamp and convert to 16-bit ===
        cv::cuda::threshold(AMP3, AMP3, 65535, 65535, cv::THRESH_TRUNC);
        cv::cuda::threshold(AMP3, AMP3, 0, 0, cv::THRESH_TOZERO);
        AMP3.convertTo(IMGDP16OUTPUT_GPU, CV_16UC1);
    }

    void YSMapping::LissajousRepairment(cv::Mat& IMGDP8_MAT, cv::cuda::GpuMat& IMGDP8) {
        cv::Mat mask = (sampleMapImage_global == 0);
        mask.convertTo(mask, CV_8U);

        cv::Mat inpainted_8U;
        cv::inpaint(IMGDP8_MAT, mask, inpainted_8U, 9, cv::INPAINT_NS);

        cv::cuda::GpuMat gpu_img;
        gpu_img.upload(inpainted_8U);

        cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
            0, -1, 0,
            -1, 5, -1,
            0, -1, 0);
        auto filter = cv::cuda::createLinearFilter(gpu_img.type(), gpu_img.type(), kernel);
        filter->apply(gpu_img, IMGDP8);
    }

    void YSMapping::SaveBufferdata(U32 frameorder, U16* Buffer) {
        ostringstream bufferCSVName;
        bufferCSVName << "C:/YuShin/Test_data_acq/BufferData_Frame" << frameorder << ".csv";
        ofstream bufferCSV(bufferCSVName.str());
        if (bufferCSV.is_open()) {
            for (int i = 0; i < int(samplesPerBuffer); ++i) {
                bufferCSV << Buffer[i];
                if (i != samplesPerBuffer - 1) {
                    bufferCSV << ",";
                }
            }
            bufferCSV.close();
            cout << "Buffer data successfully written to " << bufferCSVName.str() << endl;
        }
        else {
            cout << "Error: Unable to open file to save buffer data." << endl;
        }
    }

    void YSMapping::SaveBufferdataBin(U32 frameorder, U16* Buffer,string filetype) {
        std::ostringstream filePath;
        filePath << "C:/YuShin/Test_data_acq/BufferData_Frame"<<frameorder<<"_"<< filetype << ".bin";
        FILE* fpData = fopen(filePath.str().c_str(), "wb");
        if (fpData) {
            size_t bytesPerBuffer = samplesPerBuffer * sizeof(U16);
            size_t bytesWritten = fwrite(Buffer, sizeof(BYTE), bytesPerBuffer, fpData);
            fclose(fpData);

            if (bytesWritten != bytesPerBuffer) {
                std::cerr << "Warning: fwrite wrote " << bytesWritten << " bytes, expected " << bytesPerBuffer << std::endl;
            }
        }
        else {
            std::cerr << "Error: Unable to open file " << filePath.str() << " for writing.\n";
        }
    }

    void YSMapping::SaveMATdataBin(U32 frameorder, cv::Mat& cpuMat_pre, string filetype) {
        cv::Mat cpuMat;
        cpuMat_pre.convertTo(cpuMat,CV_16UC1);

        std::ostringstream filePath;
        filePath << "C:/YuShin/Test_data_acq/BufferData_Frame" << frameorder << "_" << filetype << ".bin";

        FILE* fpData = fopen(filePath.str().c_str(), "wb");
        if (!fpData) {
            std::cerr << "Error: Unable to open file " << filePath.str() << " for writing.\n";
            return;
        }

        size_t dataSize = cpuMat.total() * cpuMat.elemSize(); 
        size_t bytesWritten = fwrite(cpuMat.data, 1, dataSize, fpData);
        fclose(fpData);

        if (bytesWritten != dataSize) {
            std::cerr << "Warning: fwrite wrote " << bytesWritten << " bytes, expected " << dataSize << std::endl;
        }
    }
//Close the board
    void YSMapping::CloseAlazar() {
        lock_guard<mutex> lock(mtx);
        running = false;
        startprocessing = false;
        ntuscope->highspeedmode = false;
        ntuscope->avgstate = false;
        ntuscope->YSapply_repair = false;
        for (U32 bufferIndex = 0; bufferIndex < BUFFER_COUNT; ++bufferIndex) {
            if (BufferArray[bufferIndex] != nullptr) {
                AlazarFreeBufferU16(boardHandle, BufferArray[bufferIndex]);
            }
        }
        for (int i = 0; i < TRANSIENT_BUFFER_COUNT; ++i) {
            if (transientBuffers[i] != nullptr) {
                free(transientBuffers[i]);
                transientBuffers[i] = nullptr;
            }
        }
        cudamanager->CudaMemoryManagerStop();
        stream.waitForCompletion();
        cv::destroyAllWindows();
        cout << "Close Alazar: Success" << endl;
    }

}

#pragma managed