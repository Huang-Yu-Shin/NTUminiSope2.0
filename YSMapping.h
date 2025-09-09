#pragma once
#include "YSNTUcore.h"
#include "YSReconstruct.cuh"
#include <iomanip>
#include <stdint.h>
namespace NTU {

    class YSMapping {
    public:
        YSMapping(NTU::YSNTUscope* sharedNTUscope);
        ~YSMapping();
        YSNTUscope* ntuscope;
        CudaMemoryManager* cudamanager;

        enum ScanMode { LISSAJOUS_2D, RASTER, RASTER_SLOW, RASTER_ULTRA, LISSAJOUS_3D };
        enum ClockType { INTClock2M, INTClock10M, INTClock20M, INTClock50M, INTClock100M, INTClock125M, EXTClock };
        ScanMode scanmode;
        ClockType clocktype_mapping;
        cv::cuda::Stream stream;

        bool running;
        HANDLE boardHandle; //Alazar board task
        U32 channelMask = CHANNEL_A;
        U32 channelCount = 1;
        U32 preTriggerSamples;  //for Configuration.txt
        U32 postTriggerSamples;  // The number of post-trigger samples calculated from sampling rate and Lissajous frequencies
        U32 recordsPerBuffer = 1;
        U32 samplesPerRecord = 0;
        U32 bytesPerBuffer = 0; // WARNING:include channelcount
        U32 samplesPerBuffer = 0;
        U32 recordsPerAcquisition = 0;
        U32 buffersPerAcquisition = 0;
        int totalsample; //sample per frame not real
        double Alazartriggerdelay; //for Configuration.txt
        int Zpixelnum;
        int imageIndex = 0;
        int SavedCount = 0;
        U32 frameorder_savestart = 0;
        int interdisplay = 1;

        bool ConfigureBoard(ScanMode mode);
        bool ConfigureBoardTwoChannel(ScanMode mode);
        bool Generate2D_LISSAJOUS_SampleMap();
        bool Generate2D_RASTER_SampleMap();
        bool ProcessBufferAndFill_2DLissajousImage(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder);
        bool ProcessBufferAndFill_2DLissajousImage_highspeed(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder);
        bool ProcessBufferAndFill_2DRasterImage(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder);
        bool ProcessBufferAndFill_2DRasterImage_highspeed(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder);
        bool ProcessBufferAndFill_2DRasterImageTwoChannel(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder);
        bool ProcessBufferAndFill_2DRasterImageTwoChannel_highspeed(U16* Buffer, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system, U32 frameorder);
        void WarmupLissajousImageProcessing();
        bool StartAcquireData();
        bool AcquireData_RASTER();
        bool AcquireData();
        void SaverasterimageFunction();
        void SavelissajousimageFunction();
        bool Saveimage_RASTER(cv::Mat final_display_img , vector<U16>& buffer_save, system_clock::time_point starttime_system);
        bool Saveimage_LISSAJOUS(cv::Mat final_display_img, vector<U16>& buffer_save, system_clock::time_point starttime_system);
        //bool Saveimage_LISSAJOUSBin_highspeed(U16* Buffer, system_clock::time_point starttime_system);
        bool Saveimage_LISSAJOUSBin_highspeed(vector<U16>& buffer_save, system_clock::time_point starttime_system);
        bool Saveimage_RASTER_highspeed(cv::Mat final_display_img, vector<U16>& buffer_save, system_clock::time_point starttime_system);
        bool Saveimage_RASTER2CH_highspeed(cv::Mat final_display_img, vector<U16>& buffer_save, system_clock::time_point starttime_system);
        void CloseAlazar();
        void Trensientstep();
        void Accelerate(U16* Buffer, int bufferIndex, high_resolution_clock::time_point starttime, system_clock::time_point starttime_system);
        void AverageFunction();
        void RepairFunction();
        void Addtext(cv::Mat& IMGDP8, double rate, int Xpixelsize, int Ypixelsize);
        void Addtext_Avg(cv::Mat& IMGDP8, int Xpixelsize, int Ypixelsize);
        void Addtext_Repair(cv::Mat& IMGDP8, int Xpixelsize, int Ypixelsize);
        void Savecsv(ScanMode scanmode);
        void ApplyFilter(cv::cuda::GpuMat& IMGDP8, cv::cuda::Stream& streamCUDA);
        void SaveBufferdata(U32 frameorder, U16* Buffer);
        void SaveBufferdataBin(U32 frameorder, U16* Buffer, string filetype);
        void SaveMATdataBin(U32 frameorder, cv::Mat& cpuMat_pre, string filetype);
        void LissajousRepairment(cv::Mat& IMGDP8_MAT, cv::cuda::GpuMat& IMGDP8);
        void EnhanceContrast_16bit(cv::cuda::GpuMat& IMGDP16_GPU, cv::cuda::GpuMat& IMGDP16OUTPUT_GPU, float saturated_percent);
        void EnhanceContrast_8bit(cv::cuda::GpuMat& IMGDP8_GPU, cv::cuda::GpuMat& IMGDP8OUTPUT_GPU, float saturated_percent);
        void AdaptiveContrastEnhancement(cv::Mat& IMGDP16, cv::Mat& IMGDP16OUTPUT, int C, int n, float MaxCG);
        void Dynamic_Contrast_Enhancement(cv::Mat& IMGDP16, cv::Mat& IMGDP16OUTPUT, int slopeThreshold, int bins);
        void Dynamic_Contrast_Enhancement_CUDA(cv::cuda::GpuMat& IMGDP16_GPU, cv::cuda::GpuMat& IMGDP16OUTPUT_GPU, float alpha_max);
    private:
        //mapping state
        BOOL success; //configure state to notify StartAcquireData
        mutex mtx;
        U16* sampleMap_1D;
        RETURN_CODE retCode; 
        cv::Mat sin_weight;


        //average state
        deque<cv::Mat> imageQueue;
        atomic<bool> stopavgThread;
        mutex queueMutex;
        condition_variable queueCondition;
        cv::Mat image_average;

        //lissajous repair state
        cv::Mat sampleMapImage_global;
        deque<cv::cuda::GpuMat> repairQueue;
        mutex repairMutex;
        condition_variable repairCondition;
        atomic<bool> stoprepairThread;
        cv::Mat image_repair;

        //save state
        deque<tuple<cv::Mat, vector<U16>, std::chrono::system_clock::time_point>> saveQueue;
        deque<tuple<cv::Mat, vector<U16>, std::chrono::system_clock::time_point>> saveLissajousQueue;
        mutex saveMutex;
        condition_variable saveCondition;
        atomic<bool> stopsaveThread;
        cv::Mat image_save;
        cv::Mat image_saveraw;

        //thread processingThreads;
        U16* BufferArray[BUFFER_COUNT] = { NULL }; //Alazar buffer
        mutex bufferMutex[TRANSIENT_BUFFER_COUNT];
        U16* transientBuffers[TRANSIENT_BUFFER_COUNT] = { NULL };  // transient buffers
        high_resolution_clock::time_point bufferStartTimes[TRANSIENT_BUFFER_COUNT]; //acquisition time
        bool hasNewData[TRANSIENT_BUFFER_COUNT]; //transient buffer update state
        U32 frameorder[TRANSIENT_BUFFER_COUNT]; //transient buffer frame order
        system_clock::time_point bufferStartTimes_system[TRANSIENT_BUFFER_COUNT];

        bool startprocessing = false;
        high_resolution_clock::time_point previoustimepoint;
    };

}
