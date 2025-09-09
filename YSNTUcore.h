#pragma once
#include <iostream>
#include <NIDAQmx.h>
#include "AlazarApi.h"
#include "AlazarCmd.h"
#include "AlazarError.h"
#include "ComCasp.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <map>
#include <cmath>
#include <math.h>
#include <functional>
#include <deque>
#include <vector>
#include <fstream>
#include <string>
#include <stdio.h>
#include <algorithm>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/photo.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>


#define BUFFER_COUNT 16
#define TRANSIENT_BUFFER_COUNT 250

namespace NTU
{
	using namespace std;
	using namespace std::chrono;

	struct LookUpEntry {
		double xoffset;
		double yoffset;
	};

	class YSNTUscope {

	public:

		int clocktype;
		double EXTCLOCK_freq = 0;
		//MEMS
		double MEMS_xamp = 1;
		double MEMS_yamp = 1;
		double MEMS_spaceoerkine = 0.5;
		double xfreq = 2100;
		double yfreq = 2000;
		double xoffset = 0;
		double yoffset = 0;
		double MEMS_Xampratio_headmount = 1;
		double MEMS_Yampratio_headmount = 1;
		double lissajous_phase = 0;
		int num_lines = 256;
		bool firstinitialize = true;
		bool reverse_axis = false;
		int chopperdownscale = 1;
		int chopperupscale = 1;
		int chopperphase = 0;

		//ETL
		double zoffset = 47;
		double zstep = 1;
		int zlayer = 1;

		//real-time imageprocessing
		string timestampFolder;
		string folderPath;
		string folderPathRaw;
		string folderPathRawBin;
		string titleFolder;
		bool savestate = false;
		bool saveprocessedstate = false;
		bool saverawstate = false;
		int save_num = 0;
		bool avgstate = false;
		int avgcount = 1;
		int Xpixelsize_display = 512;
		int Ypixelsize_display = 512;
		int Xpixelsize_save = 0;
		int Ypixelsize_save = 0;
		double fontScale = 0.6;

		// Processing
		int YSgaussian_kernel = 5;
		int YSgaussian_sigmaH = 10;
		int YSgaussian_sigmaV = 10;
		int YSmedian_kernel = 3;
		int YSCLAHE_tile_grid_H = 20;
		int YSCLAHE_tile_grid_V = 20;
		int YSCLAHE_clip_limit = 3;
		double YSconstrat = 2.6;
		double YSbrightness = 150;
		double YSgamma = 1.1;
		int minVal16bit = 0;
		int maxVal16bit = 65535;
		int normalize_alpha = 0;
		int normalize_beta = 65535;
		float YSalpha_DCE = 4.0;
		int YSkernel_DCE = 7;
		bool YSapply_contrast = false;
		bool YSapply_gaussian = false;
		bool YSapply_median = false;
		bool YSapply_CLAHE = false;
		bool YSapply_repair = false;
		bool YSapply_enhancecontrast = false;
		bool YSapply_minmax = false;
		bool YSapply_signalprocessing = false;
		bool YSapply_binning = false;
		bool YSapply_normalize = false;
		bool YSapply_DCE = false;
		bool YSapply_equalizehistogram = false;
		int YSoffsetvalueSP = 32768;
		double YSmultiplySP = 2.0;

		//real-time information
		double FOVx = 0;
		double FOVy = 0;
		int Pixelnumx = 0;
		int Pixelnumy = 0;
		double acqtime = 0;
		double framerate = 0;
		double refreshtime = 0;
		double saveprogress = 0;
		int chopperfreq = 0;

		//3D data
		double zduration_per_change[TRANSIENT_BUFFER_COUNT] = { NULL };
		bool zcallback[TRANSIENT_BUFFER_COUNT];
		map<int, LookUpEntry> offsetLUT;

		//Alazar
		int samplesPerSec = 0;
		int raster_phase = 0;
		U32 inputrangetype = INPUT_RANGE_PM_800_MV;
		double lissajous_delay = 0;
		bool samplemapsave = false;
		bool highspeedmode = false;
		bool highspeedmode_raster = false;
		bool applychopper = false;
		bool channeldisplay = false; //false for CH1, true for CH2
		//NI
		int samplingRate = 0;

	};
}