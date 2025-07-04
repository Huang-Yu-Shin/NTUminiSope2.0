#pragma once
#ifndef YSCALCULATEPARA_CUH
#define YSCALCULATEPARA_CUH
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <math.h>
#include <iomanip> 
#include "YSNTUcore.h"
namespace NTU {

	constexpr int FX_COUNT = 9;
	constexpr int PHASE_COUNT = 8;
	constexpr int TOTAL_PARAMS = FX_COUNT * PHASE_COUNT;

	class CudaCalPara {
	public:
		CudaCalPara();
		~CudaCalPara();
		void initParams();
		void FindLissajousPara();
		void GetBresenhamLine(int x0, int y0, int x1, int y1, std::vector<int>& xList, std::vector<int>& yList);
		void FindLissajousPara_multi();
		int width = 512;
		int height = 512;
		int xTarget = 0;
		int yTarget = 0;
		int xTarget2 = 0;
		int yTarget2 = 0;
		double sampleRate = 50000000.0;
		double fxHost[TOTAL_PARAMS];
		double fyHost[TOTAL_PARAMS];
		double fpsHost[TOTAL_PARAMS];
		double phaseHost[TOTAL_PARAMS];
		bool resultHost[TOTAL_PARAMS];
		int timesperframeHost[TOTAL_PARAMS];
		bool TwoPointsCal = false;
		bool LineCal = false;
	private:
		cudaStream_t stream;
		cudaDeviceProp prop;

		double* fxDev;
		double* fyDev;
		double* fpsDev;
		double* phaseDev;
		bool* resultsDev;
		int* timesperframeDev;
		std::vector<int> targetXList, targetYList;
	};
}

__global__ void checkPara(double* fx, double* fy, double* phase, double* framerate, bool* out, int* timesperframe, int x, int y, double rate, int w, int h, bool twopointscal, int targetx2, int targety2);
__global__ void checkPara_multi(double* fx, double* fy, double* phase, double* framerate, bool* out, int* timesperframe, int* targetXList, int* targetYList, int numPoints, double rate, int w, int h);
#endif  