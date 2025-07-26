#include "YSScan.h"

#define DAQmxErrChk(functionCall) \
    if (DAQmxFailed(error = (functionCall))) { \
        DAQmxGetExtendedErrorInfo(NTU::YSScan::errBuff, 2048); \
        cerr << "DAQmx Error: " << NTU::YSScan::errBuff << endl; \
        cout<<"Please press STOP button!!!!"<<endl;\
    }

namespace NTU {
    YSScan::YSScan(NTU::YSNTUscope* sharedNTUscope) : running(false) {
        ntuscope = sharedNTUscope;
    }
    YSScan::~YSScan() {
    }


    //stop
    void YSScan::alignment_stop() {
        vector<float64> offset(10000);  // 5000 repetitions * 2 (for x and y)
        float64 increment_x = (0 - ntuscope->xoffset) / (5000 - 1);
        float64 increment_y = (0 - ntuscope->yoffset) / (5000 - 1);
        for (int i = 0; i < 5000; i++) {
            offset[2 * i] = ntuscope->xoffset + i * increment_x;
            offset[2 * i + 1] = ntuscope->yoffset + i * increment_y;
        }
        DAQmxErrChk(DAQmxStopTask(taskHandle));
        DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, true, 0.000000001, DAQmx_Val_GroupByScanNumber, offset.data(), NULL, NULL));
        DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
        DAQmxErrChk(DAQmxStopTask(taskHandle));
        DAQmxErrChk(DAQmxStopTask(taskHandlefilter));
        DAQmxErrChk(DAQmxClearTask(taskHandlefilter));
        DAQmxErrChk(DAQmxClearTask(taskHandle));
        cout << "Close NIDAQ: Success" << endl;
        //eCOMCaspErr closeCOM6_state = Casp_CloseCOM();
        //const char* closeCOM6_state_messenge = Casp_GetErrorMsg(closeCOM6_state);
        /*cout << "Close COM6: " << closeCOM6_state_messenge << endl;*/
    }

    void YSScan::stop() {
        lock_guard<mutex> lock(mtx);
        running = false;
    }

    //initialize
    bool YSScan::initialize() {
        // Find COM3 fot ETL   
        //int COMcount = Casp_SysCOMCount();
        //int COM3loc = 0;
        //for (int i = 0; i < COMcount; i++) {
        //    const char* COMname = Casp_SysCOMName(i);
        //    if (string(COMname) == "COM3") {
        //        COM3loc = i;
        //    }
        //}

        // Connected to COM3 for ETL
        //eCOMCaspErr openCOM3_state = Casp_OpenCOMIdx(COM3loc);
        //const char* openCOM3_state_messenge = Casp_GetErrorMsg(openCOM3_state);
        //eCOMCaspErr setfocus_state = Casp_SetFocusVoltage(47);
        //cout << "Initialize ETL : " << openCOM3_state_messenge << endl;

        //Confige the NI card
        DAQmxErrChk(DAQmxCreateTask("MEMS driving trigger", &taskHandle));
        DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandle, "Dev1/ao0", "X-axis driving trigger", -10.0, 10.0, DAQmx_Val_Volts, NULL));
        DAQmxErrChk(DAQmxCreateAOVoltageChan(taskHandle, "Dev1/ao1", "Y-axis driving trigger", -10.0, 10.0, DAQmx_Val_Volts, NULL));
        
        DAQmxErrChk(DAQmxCreateTask("MEMS filter clk", &taskHandlefilter));
        DAQmxErrChk(DAQmxCreateTask("MEMS TTL", &taskHandleTTL));

        //Check the NI card state
        if (taskHandlefilter != 0 && taskHandle != 0 ) {
            cout << "Initialize MEMS (include signal and trigger and clock) : Success" << endl;
            cout << "----------------------------" << endl;
        }
        return true;
    }

    //focal length set only
    bool YSScan::ETLfocalset() {
        //eCOMCaspErr setfocus_state = Casp_SetFocusVoltage(ntuscope->zoffset);
        //if (setfocus_state == 0) {
        //    return true;
        //}
        //else {
        //    return false;
        //}
        return true;
    }

    //configure
    bool YSScan::twoD_lissajous_configure() {
        if (taskHandlefilter != 0 && taskHandle != 0 && taskHandleTTL != 0) {
            running = true;

            if (!ReadCalibrationRatio_Lissajous(ntuscope->MEMS_xamp, ntuscope->MEMS_yamp, ntuscope->xfreq, ntuscope->yfreq, MEMS_Xampratio_clk_Lissajous, MEMS_Yampratio_clk_Lissajous)) {
                cout << "ERROR:ReadCalibrationRatio_Lissajous" << endl;
            }
            float64 framerate = float64(_Gcd(ntuscope->xfreq, ntuscope->yfreq));
            ntuscope->framerate = double(framerate);
            ntuscope->FOVx = ntuscope->MEMS_xamp * 250;
            ntuscope->FOVy = ntuscope->MEMS_yamp * 250;
            ntuscope->samplingRate = 500000;
            int samplingratio = int(ntuscope->samplingRate / int(framerate));
            ntuscope->samplingRate = int(framerate) * samplingratio;
            total_samples = int(ntuscope->samplingRate / framerate);
            float64 real_xamp = ntuscope->MEMS_xamp * MEMS_Xampratio_clk_Lissajous * MEMSampratio * MEMS_Xampratio_headmount;
            float64 real_yamp = ntuscope->MEMS_yamp * MEMS_Yampratio_clk_Lissajous * MEMS_Yampratio_headmount;
            if (ntuscope->xfreq > 2200 || ntuscope->yfreq > 2200) {
                MEMScutofffreq = MEMScutofffreq_high;
                NItriggerdelay = NItriggerdelay_high;
            }
            cout << "NI Sampling rate: " << ntuscope->samplingRate << endl;
            cout << "NI total sample: " << total_samples << endl;
            cout << "Frame rate: " << framerate << endl;
            cout << "MEMS amp: " << real_xamp << " , " << real_yamp << endl;
            cout << "FCLK ratio: " << MEMS_Xampratio_clk_Lissajous <<" , "<< MEMS_Yampratio_clk_Lissajous << endl;
            cout << "Headmount ratio: " << MEMS_Xampratio_headmount << " , " << MEMS_Yampratio_headmount << endl;
            cout << "MEMS offset: " << ntuscope->xoffset << " , " << ntuscope->yoffset << endl;
            cout << "MEMS phase: " << ntuscope->lissajous_phase << endl;
            cout << "MEMS cutofffreq: " << MEMScutofffreq << endl;
            cout << "NI triggerdelay: " << NItriggerdelay << endl;
            //ETL setup
            //eCOMCaspErr setfocus_state = Casp_SetFocusVoltage(ntuscope->zoffset);

            //MEMS start transient setup
            transientdata_start = new float64[10000];
            float64 increment_x_start = (ntuscope->xoffset - 0) / (5000 - 1);
            float64 increment_y_start = (ntuscope->yoffset - 0) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                transientdata_start[2 * i] = i * increment_x_start;
                transientdata_start[2 * i + 1] = i * increment_y_start;
            }
            //MEMS stop transient setup
            transientdata = new float64[10000];
            float64 increment_x = (0 - ntuscope->xoffset) / (5000 - 1);
            float64 increment_y = (0 - ntuscope->yoffset) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                transientdata[2 * i] = ntuscope->xoffset + i * increment_x;
                transientdata[2 * i + 1] = ntuscope->yoffset + i * increment_y;
            }

            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 5000));
            DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, false, 0.000000001, DAQmx_Val_GroupByScanNumber, transientdata_start, NULL, NULL));

            DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandlefilter, "Dev1/ctr1", "filter clk", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, MEMScutofffreq, 0.5));
            DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, total_samples));
            DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));
            DAQmxErrChk(DAQmxSetTrigAttribute(taskHandlefilter, DAQmx_StartTrig_Retriggerable, TRUE));

            DAQmxErrChk(DAQmxStartTask(taskHandlefilter));
            DAQmxErrChk(DAQmxStartTask(taskHandle));
            DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
            DAQmxErrChk(DAQmxStopTask(taskHandle));
            DAQmxErrChk(DAQmxStopTask(taskHandlefilter));
            DAQmxErrChk(DAQmxSetWriteOffset(taskHandlefilter,0));
            DAQmxErrChk(DAQmxSetWriteOffset(taskHandle, 0));

            //MEMS input setup
            lissajousdata = new float64[total_samples * 2];
            NIgenerateSineWave(lissajousdata, total_samples, real_xamp, ntuscope->xfreq, 0, ntuscope->samplingRate, ntuscope->xoffset, 0);
            NIgenerateSineWave(lissajousdata, total_samples, real_yamp, ntuscope->yfreq, ntuscope->lissajous_phase, ntuscope->samplingRate, ntuscope->yoffset, 1);
            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, total_samples));
            DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, total_samples, FALSE, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByScanNumber, lissajousdata, NULL, NULL));

            //MEMS TTL setup
            DAQmxErrChk(DAQmxCreateDOChan(taskHandleTTL, "Dev1/port0/line0", "", DAQmx_Val_ChanPerLine));
            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleTTL, "/Dev1/ao/SampleClock", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, total_samples));
            ttldata = new uInt8[total_samples];
            int risingEdgeDelaySamples = static_cast<int>(ntuscope->samplingRate * NItriggerdelay);
            for (int i = 0; i < total_samples; i++) {
                if (i >= risingEdgeDelaySamples && i < risingEdgeDelaySamples + int(total_samples / 2)) {
                    ttldata[i] = 1;
                }
                else {
                    ttldata[i] = 0;
                }
            }
            DAQmxErrChk(DAQmxWriteDigitalLines(taskHandleTTL, total_samples, FALSE, 10, DAQmx_Val_GroupByScanNumber, ttldata, NULL, NULL));

            //MEMS filter clk setup
            DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, total_samples));
            DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));

            return true;
        }
        else {
            return false;
        }
    }

    bool YSScan::twoD_raster_configure() {
        if (taskHandlefilter != 0 && taskHandle != 0 && taskHandleTTL != 0) {
            running = true;

            if (!ReadCalibrationRatio_Raster(ntuscope->MEMS_xamp, ntuscope->MEMS_yamp, ntuscope->xfreq, MEMS_Xampratio_clk_raster,MEMS_Yampratio_clk_raster)) {
                cout << "ERROR:ReadCalibrationRatio_Raster" << endl;
            }
            ntuscope->samplingRate = 500000;
            samples_per_line = int(ntuscope->samplingRate / (2 * ntuscope->xfreq));  // 2 because we're working with half sine wave per line
            total_samples = samples_per_line * ntuscope->num_lines;
            double framerate = double(ntuscope->samplingRate) / double(total_samples);
            ntuscope->framerate = framerate;
            ntuscope->FOVx = ntuscope->MEMS_xamp * 250;
            ntuscope->FOVy = ntuscope->MEMS_yamp * 250;
            float64 real_xamp = ntuscope->MEMS_xamp * MEMS_Xampratio_clk_raster * MEMSampratio* MEMS_Xampratio_headmount;
            float64 real_yamp = ntuscope->MEMS_yamp * MEMS_Yampratio_clk_raster * MEMS_Yampratio_headmount;
            cout << "NI Sampling rate: " << ntuscope->samplingRate << endl;
            cout << "NI samples per line: " << samples_per_line << endl;
            cout << "NI total sample: " << total_samples << endl;
            cout << "Frame rate: " << framerate << endl;
            cout << "MEMS amp: " << real_xamp << " , " << real_yamp << endl;
            cout << "FCLK ratio: " << MEMS_Xampratio_clk_raster << " , "<<MEMS_Yampratio_clk_raster << endl;
            cout << "Headmount ratio: " << MEMS_Xampratio_headmount << " , " << MEMS_Yampratio_headmount << endl;
            cout << "MEMS offset: " << ntuscope->xoffset << " , " << ntuscope->yoffset << endl;
            cout << "NI triggerdelay: " << NItriggerdelay << endl;
            //ETL setup
            //eCOMCaspErr setfocus_state = Casp_SetFocusVoltage(ntuscope->zoffset);

            //MEMS start transient setup
            transientdata_start = new float64[10000];
            float64 increment_x_start = (ntuscope->xoffset-0) / (5000 - 1);
            float64 increment_y_start = (ntuscope->yoffset-0) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                transientdata_start[2 * i] = i * increment_x_start;
                transientdata_start[2 * i + 1] = i * increment_y_start;
            }
            //MEMS stop transient setup
            transientdata = new float64[10000];
            float64 increment_x = (0 - ntuscope->xoffset) / (5000 - 1);
            float64 increment_y = (0 - ntuscope->yoffset) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                transientdata[2 * i] = ntuscope->xoffset + i * increment_x;
                transientdata[2 * i + 1] = ntuscope->yoffset + i * increment_y;
            }

            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 5000));
            DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, false, 0.000000001, DAQmx_Val_GroupByScanNumber, transientdata_start, NULL, NULL));

            DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandlefilter, "Dev1/ctr1", "filter clk", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, MEMScutofffreq, 0.5));
            DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, total_samples));
            DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));
            DAQmxErrChk(DAQmxSetTrigAttribute(taskHandlefilter, DAQmx_StartTrig_Retriggerable, TRUE));

            DAQmxErrChk(DAQmxStartTask(taskHandlefilter));
            DAQmxErrChk(DAQmxStartTask(taskHandle));
            DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
            DAQmxErrChk(DAQmxStopTask(taskHandle));
            DAQmxErrChk(DAQmxStopTask(taskHandlefilter));
            DAQmxErrChk(DAQmxSetWriteOffset(taskHandlefilter, 0));
            DAQmxErrChk(DAQmxSetWriteOffset(taskHandle, 0));

            //MEMS input setup
            rasterdata = new float64[total_samples * 2 * 2]; //for two frame
            NIgenerateRaster(rasterdata, real_xamp, real_yamp, ntuscope->xoffset, ntuscope->yoffset, ntuscope->num_lines, ntuscope->xfreq, ntuscope->samplingRate);
            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, total_samples * 2));
            DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, total_samples * 2, FALSE, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByScanNumber, rasterdata, NULL, NULL));

            //MEMS TTL setup (line.frame trigger)            
            DAQmxErrChk(DAQmxCreateDOChan(taskHandleTTL, "Dev1/port0/line1", "", DAQmx_Val_ChanPerLine));
            DAQmxErrChk(DAQmxCreateDOChan(taskHandleTTL, "Dev1/port0/line0", "", DAQmx_Val_ChanPerLine));
            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleTTL, "/Dev1/ao/SampleClock", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, total_samples * 2));
            ttldata = new uInt8[total_samples * 2 * 2];
            fill(ttldata, ttldata + (total_samples * 2 * 2), 0);
            int risingEdgeDelaySamples = static_cast<int>(ntuscope->samplingRate * NItriggerdelay);
            int risingEdgeDelaySamples_frame = risingEdgeDelaySamples - 100;

            //frame trigger
            for (int frame = 0; frame < 2; frame++) {
                int frame_offset = frame * total_samples;
                int frame_startIndex = frame_offset + risingEdgeDelaySamples_frame;
                int frame_endIndex = frame_startIndex + int((total_samples - risingEdgeDelaySamples_frame) / 2);
                for (int i = frame_startIndex; i < frame_endIndex; i++) {
                    ttldata[i * 2 + 0] = 1;
                }
            }
            //line trigger
            for (int frame = 0; frame < 2; frame++) {
                int frame_offset = frame * total_samples;
                for (int line = 0; line < ntuscope->num_lines; ++line) {
                    int line_startIndex = frame_offset + line * samples_per_line + risingEdgeDelaySamples;
                    int line_endIndex = line_startIndex + int((samples_per_line - risingEdgeDelaySamples) / 2);
                    int line_endBoundary = frame_offset + (line + 1) * samples_per_line;
                    for (int i = line_startIndex; i < line_endIndex && i < line_endBoundary; i++) {
                        ttldata[i * 2 + 1] = 1;
                    }
                }
            }
            DAQmxErrChk(DAQmxWriteDigitalLines(taskHandleTTL, total_samples * 2, FALSE, 10, DAQmx_Val_GroupByScanNumber, ttldata, NULL, NULL));


            //MEMS filter clk setup
            DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, total_samples));
            DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));

            return true;
        }
        else {
            return false;
        }
    }

    bool YSScan::twoD_raster_slow_configure() {
        if (taskHandlefilter != 0 && taskHandle != 0 && taskHandleTTL != 0) {
            running = true;

            if (!ReadCalibrationRatio_Raster(ntuscope->MEMS_xamp, ntuscope->MEMS_yamp, ntuscope->xfreq, MEMS_Xampratio_clk_raster, MEMS_Yampratio_clk_raster)) {
                cout << "ERROR:ReadCalibrationRatio_Raster" << endl;
            }
            ntuscope->samplingRate = 500000;
            samples_per_line = int(ntuscope->samplingRate / (2 * ntuscope->xfreq));  // 2 because we're working with half sine wave per line
            total_samples = samples_per_line * ntuscope->num_lines;
            double framerate = double(ntuscope->samplingRate) / double(total_samples);
            ntuscope->framerate = framerate;
            ntuscope->FOVx = ntuscope->MEMS_xamp * 250;
            ntuscope->FOVy = ntuscope->MEMS_yamp * 250;
            float64 real_xamp = ntuscope->MEMS_xamp * MEMS_Xampratio_clk_raster * MEMSampratio * MEMS_Xampratio_headmount;
            float64 real_yamp = ntuscope->MEMS_yamp * MEMS_Yampratio_clk_raster * MEMS_Yampratio_headmount;
            cout << "NI Sampling rate: " << ntuscope->samplingRate << endl;
            cout << "NI samples per line: " << samples_per_line << endl;
            cout << "NI total sample: " << total_samples << endl;
            cout << "Frame rate: " << framerate << endl;
            cout << "MEMS amp: " << real_xamp << " , " << real_yamp << endl;
            cout << "FCLK ratio: " << MEMS_Xampratio_clk_raster << " , " << MEMS_Yampratio_clk_raster << endl;
            cout << "Headmount ratio: " << MEMS_Xampratio_headmount << " , " << MEMS_Yampratio_headmount << endl;
            cout << "MEMS offset: " << ntuscope->xoffset << " , " << ntuscope->yoffset << endl;
            cout << "NI triggerdelay: " << NItriggerdelay << endl;
            //ETL setup
            //eCOMCaspErr setfocus_state = Casp_SetFocusVoltage(ntuscope->zoffset);


            //MEMS start transient setup
            transientdata_start = new float64[10000];
            float64 increment_x_start = (ntuscope->xoffset - 0) / (5000 - 1);
            float64 increment_y_start = (ntuscope->yoffset - 0) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                transientdata_start[2 * i] = i * increment_x_start;
                transientdata_start[2 * i + 1] = i * increment_y_start;
            }
            //MEMS stop transient setup
            transientdata = new float64[10000];
            float64 increment_x = (0 - ntuscope->xoffset) / (5000 - 1);
            float64 increment_y = (0 - ntuscope->yoffset) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                transientdata[2 * i] = ntuscope->xoffset + i * increment_x;
                transientdata[2 * i + 1] = ntuscope->yoffset + i * increment_y;
            }

            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 5000));
            DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, false, 0.000000001, DAQmx_Val_GroupByScanNumber, transientdata_start, NULL, NULL));

            DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandlefilter, "Dev1/ctr1", "filter clk", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, MEMScutofffreq, 0.5));
            DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, total_samples));
            DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));
            DAQmxErrChk(DAQmxSetTrigAttribute(taskHandlefilter, DAQmx_StartTrig_Retriggerable, TRUE));

            DAQmxErrChk(DAQmxStartTask(taskHandlefilter));
            DAQmxErrChk(DAQmxStartTask(taskHandle));
            DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
            DAQmxErrChk(DAQmxStopTask(taskHandle));
            DAQmxErrChk(DAQmxStopTask(taskHandlefilter));
            DAQmxErrChk(DAQmxSetWriteOffset(taskHandlefilter, 0));
            DAQmxErrChk(DAQmxSetWriteOffset(taskHandle, 0));

            //MEMS input setup
            int transientsample = int(transienttime * float64(ntuscope->samplingRate));
            int allsample = total_samples + transientsample;
            cout << "NI all sampample: " << allsample << endl;
            rasterdata = new float64[allsample * 2]; //for two frame
            NIgenerateRaster_slow(rasterdata, real_xamp, real_yamp, ntuscope->xoffset, ntuscope->yoffset, ntuscope->num_lines, ntuscope->xfreq, ntuscope->samplingRate);
            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, allsample));
            DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, allsample, FALSE, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByScanNumber, rasterdata, NULL, NULL));
            
            //MEMS TTL setup (line.frame trigger)            
            DAQmxErrChk(DAQmxCreateDOChan(taskHandleTTL, "Dev1/port0/line1", "", DAQmx_Val_ChanPerLine));
            DAQmxErrChk(DAQmxCreateDOChan(taskHandleTTL, "Dev1/port0/line0", "", DAQmx_Val_ChanPerLine));
            DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleTTL, "/Dev1/ao/SampleClock", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_ContSamps, allsample));
            ttldata = new uInt8[allsample * 2];
            fill(ttldata, ttldata + (allsample * 2), 0);
            int risingEdgeDelaySamples = static_cast<int>(ntuscope->samplingRate * NItriggerdelay);
            int risingEdgeDelaySamples_frame = risingEdgeDelaySamples - 100;

            //frame trigger
            int frame_startIndex = risingEdgeDelaySamples_frame;
            int frame_endIndex = frame_startIndex + int((total_samples - risingEdgeDelaySamples_frame) / 2);
            for (int i = frame_startIndex; i < frame_endIndex; i++) {
                ttldata[i * 2 + 0] = 1;
            }
            //line trigger
            for (int line = 0; line < ntuscope->num_lines; ++line) {
                int line_startIndex =line * samples_per_line + risingEdgeDelaySamples;
                int line_endIndex = line_startIndex + int((samples_per_line - risingEdgeDelaySamples) / 2);
                int line_endBoundary = (line + 1) * samples_per_line;
                for (int i = line_startIndex; i < line_endIndex && i < line_endBoundary; i++) {
                    ttldata[i * 2 + 1] = 1;
                }
            }
            DAQmxErrChk(DAQmxWriteDigitalLines(taskHandleTTL, allsample, FALSE, 10, DAQmx_Val_GroupByScanNumber, ttldata, NULL, NULL));

            //MEMS filter clk setup
            DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, allsample));
            DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));

            return true;
        }
        else {
            return false;
        }
    }

    void YSScan::alignment() {

        if (ntuscope->xoffset > 10 || ntuscope->xoffset < -10 || ntuscope->yoffset>10 || ntuscope->yoffset < -10) {
            cout << "ERROR: Out of range !! Please try again" << endl;
        }
        else {
            //MEMS input setup
            vector<float64> desire(10000);  // 5000 repetitions * 2 (for x and y)
            for (int i = 0; i < 5000; ++i) {
                desire[2 * i] = ntuscope->xoffset;
                desire[2 * i + 1] = ntuscope->yoffset;
            }

            //MEMS transient setup
            vector<float64> offset(10000);  // 5000 repetitions * 2 (for x and y)
            float64 increment_x = (ntuscope->xoffset - previousxoffset) / (5000 - 1);
            float64 increment_y = (ntuscope->yoffset - previousyoffset) / (5000 - 1);
            for (int i = 0; i < 5000; i++) {
                offset[2 * i] = previousxoffset + i * increment_x;
                offset[2 * i + 1] = previousyoffset + i * increment_y;
            }

            //Give signal to NIMEMS
            ntuscope->samplingRate = 500000;
            if (taskHandle != 0 && taskHandlefilter != 0) {
                previousxoffset = ntuscope->xoffset;
                previousyoffset = ntuscope->yoffset;
                //eCOMCaspErr setfocus_state = Casp_SetFocusVoltage(ntuscope->zoffset);
                if (timesofalignment == 0) {
                    DAQmxErrChk(DAQmxCreateCOPulseChanFreq(taskHandlefilter, "Dev1/ctr1", "filter clk", DAQmx_Val_Hz, DAQmx_Val_Low, 0.0, MEMScutofffreq, 0.5));
                    DAQmxErrChk(DAQmxCfgImplicitTiming(taskHandlefilter, DAQmx_Val_ContSamps, total_samples));
                    DAQmxErrChk(DAQmxCfgDigEdgeStartTrig(taskHandlefilter, "/Dev1/ao/SampleClock", DAQmx_Val_Rising));
                    DAQmxErrChk(DAQmxSetTrigAttribute(taskHandlefilter, DAQmx_StartTrig_Retriggerable, TRUE));
                    DAQmxErrChk(DAQmxStartTask(taskHandlefilter));
                }
                DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 5000));
                DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, true, 0.000000001, DAQmx_Val_GroupByScanNumber, offset.data(), NULL, NULL));
                DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
                DAQmxErrChk(DAQmxStopTask(taskHandle));
                DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, true, 0.000000001, DAQmx_Val_GroupByScanNumber, desire.data(), NULL, NULL));
                timesofalignment++;
                cout << "success!!" << endl;
            }
            else {
                cout << "ERROR:Please press Initialize botton" << endl;
            }
        }
    }

    void YSScan::twoD_raster_run() {
        DAQmxErrChk(DAQmxStartTask(taskHandlefilter));
        DAQmxErrChk(DAQmxStartTask(taskHandleTTL));
        DAQmxErrChk(DAQmxStartTask(taskHandle));
        while (running) {
            {
                lock_guard<mutex> lock(mtx);
                if (!running) break;
            }
            if (taskHandle == 0) {
                break;
            }
        }

        //Transient to (0,0)
        DAQmxErrChk(DAQmxStopTask(taskHandle));
        DAQmxErrChk(DAQmxStopTask(taskHandleTTL));
       
        DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 5000));
        DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, true, 0.000000001, DAQmx_Val_GroupByScanNumber, transientdata, NULL, NULL));
        DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
        DAQmxErrChk(DAQmxStopTask(taskHandle));
        // Clear the task when exiting the loop
        DAQmxErrChk(DAQmxStopTask(taskHandlefilter));
        DAQmxErrChk(DAQmxClearTask(taskHandle));
        DAQmxErrChk(DAQmxClearTask(taskHandleTTL));
        DAQmxErrChk(DAQmxClearTask(taskHandlefilter));
        cout << "Close NIDAQ: Success" << endl;
        /*eCOMCaspErr closeCOM6_state = Casp_CloseCOM();
        const char* closeCOM6_state_messenge = Casp_GetErrorMsg(closeCOM6_state);
        cout << "Close COM6: " << closeCOM6_state_messenge << endl;*/
    }

    void YSScan::twoD_lissajous_run() {
        DAQmxErrChk(DAQmxStartTask(taskHandlefilter));
        DAQmxErrChk(DAQmxStartTask(taskHandleTTL));
        DAQmxErrChk(DAQmxStartTask(taskHandle));

        while (running) {
            {
                lock_guard<mutex> lock(mtx);
                if (!running) break;
            }
            if (taskHandle == 0) {
                break;
            }
        }
        //Transient to (0,0)
        DAQmxErrChk(DAQmxStopTask(taskHandle));
        DAQmxErrChk(DAQmxStopTask(taskHandleTTL));

        DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 5000));
        DAQmxErrChk(DAQmxWriteAnalogF64(taskHandle, 5000, true, 0.000000001, DAQmx_Val_GroupByScanNumber, transientdata, NULL, NULL));
        DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandle, 0));
        DAQmxErrChk(DAQmxStopTask(taskHandle));
        // Clear the task when exiting the loop
        DAQmxErrChk(DAQmxStopTask(taskHandlefilter));
        DAQmxErrChk(DAQmxClearTask(taskHandle));
        DAQmxErrChk(DAQmxClearTask(taskHandleTTL));
        DAQmxErrChk(DAQmxClearTask(taskHandlefilter));
        cout << "Close NIDAQ: Success" << endl;
        /*eCOMCaspErr closeCOM6_state = Casp_CloseCOM();
        const char* closeCOM6_state_messenge = Casp_GetErrorMsg(closeCOM6_state);
        cout << "Close COM6: " << closeCOM6_state_messenge << endl;*/
    }

    //ETL part
    void YSScan::linearfocal_voltage_generate(int samplepoint, double min_focal, double zstep, vector<double>& voltage_set_result) {
        if (min_focal > 21.209) {
            vector<double> focallength_set(samplepoint);
            vector<double> voltage_set(samplepoint);

            for (int i = 0; i < samplepoint; i++) {
                focallength_set[i] = min_focal + i * zstep;
            }
            for (int i = 0; i < samplepoint; i++) {
                voltage_set[i] = (1000 + 2.05 * 47 * focallength_set[i]) / (focallength_set[i] * 2.05);
            }
            for (int i = 0; i < samplepoint; i++) {
                voltage_set_result[i] = round(voltage_set[i] * 1000.0) / 1000.0;
            }
            double max = *max_element(voltage_set_result.begin(), voltage_set_result.end());
            double min = *min_element(voltage_set_result.begin(), voltage_set_result.end());
            cout << "---------------------" << endl;
            cout << "Focal length range (mm): " << min_focal << " ~ " << (min_focal + zstep * (samplepoint - 1)) << endl;
            cout << "Focal lengt per step (mm): " << zstep << endl;
            cout << "Voltage range (V): " << min << " ~ " << max << endl;
            cout << "---------------------" << endl;
        }
        else {
            cout << "---------------------" << endl;
            cout << "ERROR: The ETL input is out of range !!" << endl;
            cout << "---------------------" << endl;
        }
    }


    //NIMEMS part
    void YSScan::NIgenerateSineWave(float64* data, int numSamples, float64 amplitude, float64 frequency, float64 phase, float64 samplingRate, float64 offset, int channel) {
        if (amplitude + offset > 10 || offset - amplitude < -10) {
            cout << "---------------------" << endl;
            cout << "ERROR: The MEMS input is out of range !!" << endl;
            cout << "---------------------" << endl;
        }
        for (int i = 0; i < numSamples; ++i) {
            data[i * 2 + channel] = amplitude * sin(2 * M_PI * frequency * (i / samplingRate) + phase) + offset;
        }
    }

    void YSScan::NIgenerateRaster(float64* data, float64 xamplitude, float64 yamplitude, float64 offsetx, float64 offsety, int num_lines, float64 freq_x, float64 samplingRate) {
        if (xamplitude + offsetx > 10 || offsetx - xamplitude < -10 || yamplitude + offsety > 10 || offsety - yamplitude < -10) {
            cout << xamplitude << yamplitude << offsetx << offsety << endl;
            cout << "---------------------" << endl;
            cout << "ERROR: The MEMS input is out of range !!" << endl;
            cout << "---------------------" << endl;
        }
        float64 delta_time = 1.0 / samplingRate;
        int index = 0;
        float64 period = 1.0 / freq_x;
        float64 time = 0;

        for (int line = 0; line < num_lines; ++line) {
            float64 y_value = yamplitude - (2 * yamplitude * line) / (num_lines - 1) + offsety;
            for (int i = 0; i < samples_per_line; ++i) {
                time += delta_time;
                float64 x_value = offsetx + xamplitude* sin(2 * M_PI * freq_x * time - M_PI / 2);

                // Store X and Y values interleaved in the data array
                data[index++] = x_value;  // X value (sine wave)
                data[index++] = y_value;  // Y value (constant for each full cycle of the sine wave)
            }
        }
        for (int line = 0; line < num_lines; ++line) {
            float64 y_value = -yamplitude + (2 * yamplitude * line) / (num_lines - 1) + offsety;
            for (int i = 0; i < samples_per_line; ++i) {
                time += delta_time;
                float64 x_value = offsetx + xamplitude * sin(2 * M_PI * freq_x * time - M_PI / 2);

                // Store X and Y values interleaved in the data array
                data[index++] = x_value;  // X value (sine wave)
                data[index++] = y_value;  // Y value (constant for each full cycle of the sine wave)
            }
        }
    }

    void YSScan::NIgenerateRaster_slow(float64* data, float64 xamplitude, float64 yamplitude, float64 offsetx, float64 offsety, int num_lines, float64 freq_x, float64 samplingRate) {
        if (xamplitude + offsetx > 10 || offsetx - xamplitude < -10 || yamplitude + offsety > 10 || offsety - yamplitude < -10) {
            cout << xamplitude << yamplitude << offsetx << offsety << endl;
            cout << "---------------------" << endl;
            cout << "ERROR: The MEMS input is out of range !!" << endl;
            cout << "---------------------" << endl;
        }
        float64 delta_time = 1.0 / samplingRate;
        int index = 0;
        float64 period = 1.0 / freq_x;
        float64 time = 0;
        for (int line = 0; line < num_lines; ++line) {
            float64 y_value = yamplitude - (2 * yamplitude * line) / (num_lines - 1) + offsety;
            for (int i = 0; i < samples_per_line; ++i) {
                time += delta_time;
                float64 x_value = offsetx + xamplitude * sin(2 * M_PI * freq_x * time - M_PI / 2);

                // Store X and Y values interleaved in the data array
                data[index++] = x_value;  // X value (sine wave)
                data[index++] = y_value;  // Y value (constant for each full cycle of the sine wave)
            }
        }

        float64 last_x = data[index - 2];
        float64 last_y = data[index - 1];
        int slow_return_samples = int(transienttime * float64(ntuscope->samplingRate));
        float64 increment_y = (offsety + yamplitude - last_y) / (slow_return_samples - 1);

        for (int j = 0; j < slow_return_samples; ++j) {
            float64 ramp_x = last_x ;//+ alpha * (offsetx - last_x)
            float64 ramp_y = last_y + j * increment_y;
            data[index++] = ramp_x;
            data[index++] = ramp_y;
        }
    }

    void YSScan::getOffsetFromVoltage(double voltage) {
        int rounded_voltage = std::round(voltage);

        auto it = ntuscope->offsetLUT.find(rounded_voltage);
        if (it != ntuscope->offsetLUT.end()) {
            ntuscope->xoffset = it->second.xoffset;
            ntuscope->yoffset = it->second.yoffset;
        }
        else {
            std::cerr << "Warning: Voltage " << rounded_voltage << " not found in LUT!" << std::endl;
        }
    }

    bool YSScan::ReadCalibrationRatio_Raster(double xAmp, double yAmp, double inputFreq, double& xRatio, double& yRatio)    {
        string filename;
        if (xAmp == 0.5 && yAmp == 0.5) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-0_5amp.txt";
        }
        else if (xAmp == 0.75 && yAmp == 0.75) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-0_75amp.txt";
        }
        else if (xAmp == 1 && yAmp == 1) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-1amp.txt";
        }
        else if (xAmp == 1.25 && yAmp == 1.25) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-1_25amp.txt";
        }
        else if (xAmp == 1.5 && yAmp == 1.5) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-1_5amp.txt";
        }
        else if (xAmp == 2 && yAmp == 2) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-2amp.txt";
        }
        else {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Raster_Driver_132000-0_5amp.txt";
        }
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Cannot open file FCLK_Calibration_Raster_Driver_132000.txt" << endl;
            return false;  
        }

        string line;
        bool skipHeader = true;

        while (getline(file, line)) {
            if (skipHeader) {
                skipHeader = false;
                continue;
            }

            stringstream ss(line);
            double freq, xratio,yratio;
            char comma; 

            if (ss >> freq >> comma >> xratio>>comma>>yratio) {
                if (freq == inputFreq) {
                    xRatio = xratio;
                    yRatio = yratio;
                    file.close();
                    return true; 
                }
            }
        }

        file.close();
        return false;
    }

    bool YSScan::ReadCalibrationRatio_Lissajous(double xAmp, double yAmp, double inputXFreq, double inputYFreq, double& xRatio, double& yRatio) {
        string filename;
        if (xAmp == 0.5 && yAmp == 0.5) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Lissajous_Driver_132000-0_5amp.txt";
        }
        else if (xAmp == 0.75 && yAmp == 0.75) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Lissajous_Driver_132000-0_75amp.txt";
        }
        else if (xAmp == 1 && yAmp == 1) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Lissajous_Driver_132000-1amp.txt";
        }
        else if (xAmp == 1.25 && yAmp == 1.25) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Lissajous_Driver_132000-1_25amp.txt";
        }
        else if (xAmp == 1.5 && yAmp == 1.5) {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Lissajous_Driver_132000-1_5amp.txt";
        }
        else {
            filename = "C:\\Ian\\MTPFM\\Calibrations\\default_settings\\FCLK_Calibration_Lissajous_Driver_132000-0_5amp.txt";
        }
        ifstream file(filename);

        if (!file.is_open()) {
            cerr << "Error: Cannot open file FCLK_Calibration_Lissajous_Driver_132000.txt "<<endl;
            return false;
        }

        string line;
        bool skipHeader = true;

        while (getline(file, line)) {
            if (skipHeader) {
                skipHeader = false;
                continue;
            }

            stringstream ss(line);
            double xFreqFromFile, yFreqFromFile;
            double xRatioFromFile, yRatioFromFile;
            char comma1, comma2, comma3; 

            if (ss >> xFreqFromFile >> comma1
                >> yFreqFromFile >> comma2
                >> xRatioFromFile >> comma3
                >> yRatioFromFile)
            {
                if (xFreqFromFile == inputXFreq && yFreqFromFile == inputYFreq) {
                    xRatio = xRatioFromFile;
                    yRatio = yRatioFromFile;
                    file.close();
                    return true; 
                }
            }
        }

        file.close();
        return false;
    }

}
//DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandleTTL, "/Dev1/ao/SampleClock", ntuscope->samplingRate, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, total_samples * 2));
//if (a > 5) {
//    break;
//}
//cout << "a: " << a << endl;
//DAQmxErrChk(DAQmxStartTask(taskHandleTTL));
//a++;
//DAQmxErrChk(DAQmxWaitUntilTaskDone(taskHandleTTL, 1));
//DAQmxErrChk(DAQmxStopTask(taskHandleTTL));
//DAQmxErrChk(DAQmxSetWriteOffset(taskHandleTTL, 0));