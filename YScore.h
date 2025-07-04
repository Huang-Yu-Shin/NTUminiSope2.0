#pragma once
#ifndef CORE_H
#define CORE_H


#include "YSScan.h"
#include "YSMapping.h"
#include "YSNTUcore.h"
#include <vcclr.h> 
#include "NTUcore.h"
namespace NTU {

    class YSCore {
    public:

        YSCore(NTU::YSNTUscope* sharedNTUscope);
        ~YSCore();
        void readOffsetFromFile(int selectedIndex);
        void initialize();
        void TWODLissajous_configure(double focaloffset, double MEMSXamplitude, double MEMSYamplitude, double midx, double midy, double xfreq, double yfreq, int clocktype);
        void TWODlissajousgetimage();
        void TWODRaster_configure(double focaloffset, double xfreq, double MEMSXamplitude, double MEMSYamplitude, int num_lines, double midx, double midy, int clocktype);
        void TWODRaster_slow_configure(double focaloffset, double xfreq, double MEMSXamplitude, double MEMSYamplitude, int num_lines, double midx, double midy, int clocktype);
        void TWODrasterimage();
        void stopAllOperations();
        void Alignmentsetup(double xoffset, double yoffset);
        void stopAlignment();
        void FLsetONLY(double voltage);
        void saveimg(int save_num);
        void averagefun(bool run, int avgcount);
        void readconfigure();

    private:
        YSScan* scanning;
        YSMapping* mapping;
        YSNTUscope* ntuscope;
    };
}

#endif
#pragma once
