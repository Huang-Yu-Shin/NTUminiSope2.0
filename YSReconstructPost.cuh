#pragma once
#ifndef YSRECONSTRUCTPOST_CUH
#define YSRECONSTRUCTPOST_CUH
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <math.h>
#include "YSNTUcore.h"
namespace NTU {
	class ReconstructPost {
	public:
		ReconstructPost();
		~ReconstructPost();
		void CudaMemoryManagerStop();
		void CudaMemoryAllocate();
		void CudaSampleMap(float64* sampleMap);
		void getDeviceInf();
		void launchProcessSamples_LISSAJOUS(float64* pBuffer, float64* h_image);

		int pixelnumX = 0;
		int pixelnumY = 0;
		int total_sample = 0;
		int sampling_rate = 0;
		int freqx = 0;
		int freqy = 0;
		double phase = 0;
	private:
		float64* d_image = nullptr;
		float64* d_sampleMap = nullptr;
		float64* d_pBuffer = nullptr;

		cudaStream_t stream;
		cudaDeviceProp prop;
		int num = 0;


	};
}

__global__ void processSamples_LISSAJOUS_POST(float64* pBuffer, float64* tempImage, float64* sampleMap, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq, double phase);
__global__ void invertBuffer_POST(float64* Buffer, int totalsample);
__global__ void normalizeImage_POST(float64* image, float64* tempImage, float64* sampleMap, int xpixelnumber, int ypixelnumber);
#endif  