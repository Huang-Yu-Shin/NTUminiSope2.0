#pragma once
#ifndef SCAN_H
#define SCAN_H
#define DAQmxErrChk(functionCall) \
    if (DAQmxFailed(error = (functionCall))) { \
        DAQmxGetExtendedErrorInfo(NTU::YSScan::errBuff, 2048); \
        cerr << "DAQmx Error: " << NTU::YSScan::errBuff << endl; \
        cout<<"Please press STOP button!!!!"<<endl;\
    }
#include "YSNTUcore.h"
#include <functional>
#include <vcclr.h> 
#include <algorithm>
#include <Windows.h>
namespace NTU {

    class YSScan {
    public:
        typedef function<void(double*, int)> ZDurationCallback;

        YSScan(NTU::YSNTUscope* sharedNTUscope);
        ~YSScan();
        YSNTUscope* ntuscope;

        // Initialize
        bool initialize();

        // ETL
        int layer = 0;
        double zfrequency = 0;
        void linearfocal_voltage_generate(int samplepoint, double min_focal, double zstep, vector<double>& voltage_set_result);
        bool ETLfocalset();

        // NIMEMES 
        void NIgenerateSineWave(float64* data, int numSamples, float64 amplitude, float64 frequency, float64 phase, float64 samplingRate, float64 offset, int channel);
        void NIgenerateRaster(float64* data, float64 xamplitude, float64 yamplitude, float64 offsetx, float64 offsety, int num_lines, float64 freq_x, float64 samplingRate);
        void getOffsetFromVoltage(double voltage);
        void NIgenerateRaster_slow(float64* data, float64 xamplitude, float64 yamplitude, float64 offsetx, float64 offsety, int num_lines, float64 freq_x, float64 samplingRate);
        void NIgenerateRaster_reverseaxis(float64* data, float64 xamplitude, float64 yamplitude, float64 offsetx, float64 offsety, int num_lines, float64 freq_x, float64 samplingRate);
        //void NIgenerateUltraRaster(float64* data, float64 xamp, float64 yamp, float64 xoff, float64 yoff, int num_lines, float64 xfreq, float64 samplingRate);
        //void NIgenerateUltraRaster_reverseaxis(float64* data, float64 xamp, float64 yamp, float64 xoff, float64 yoff, int num_lines, float64 freq_x, float64 samplingRate);
        //config
        bool twoD_lissajous_configure();
        bool twoD_raster_configure();
        bool twoD_raster_slow_configure();
        bool twoD_raster_ultra_configure();
        bool ReadCalibrationRatio_Raster(double xAmp, double yAmp, double inputFreq, double& xRatio, double& yRatio);
        bool ReadCalibrationRatio_Lissajous(double xAmp, double yAmp, double inputXFreq, double inputYFreq, double& xRatio, double& yRatio);
        bool ReadCalibrationRatio_UltrafastRaster(double xAmp, double yAmp, double inputFreq, double& xRatio, double& yRatio);
        double NItriggerdelay;
        double NItriggerdelay_high;
        int MEMScutofffreq;
        int MEMScutofffreq_high;
        double MEMSampratio;
        double MEMS_Xampratio_clk_raster=1;
        double MEMS_Yampratio_clk_raster = 1;
        double MEMS_Xampratio_clk_Lissajous = 1;
        double MEMS_Yampratio_clk_Lissajous = 1;
        float64 transienttime = 0.005;

        //run
        void twoD_raster_run();
        void twoD_lissajous_run();
        void alignment();

        //stop
        void stop();
        void alignment_stop();

        //Variable
        bool running;

    private:

        mutex mtx; //for stop the running

        // ETL 
        vector<float64> voltage_set_result;
        float64 zperiod;

        // NIMEMS 
        int32 error = 0;
        char errBuff[2048] = { '\0' };
        TaskHandle taskHandle;
        TaskHandle taskHandlefilter;
        TaskHandle taskHandleTTL;
        const char* clockSource = "/Dev1/PFI0";

        float64 previousxoffset = 0; //for alignment transient
        float64 previousyoffset = 0; //for alignment transient
        int32 timesofalignment = 0; //forrecord the times of alignment
        float64* lissajousdata;
        uInt8* ttldata;
        float64* rasterdata;
        float64* transientdata;
        float64* transientdata_start;
        int total_samples;
        int samples_per_line;
    };

}

#endif