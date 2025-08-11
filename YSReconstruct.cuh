#pragma once
#ifndef YSRECONSTRUCT_CUH
#define YSRECONSTRUCT_CUH
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <math.h>
#include "YSNTUcore.h"
namespace NTU {
	class CudaMemoryManager {
	public:
		CudaMemoryManager(NTU::YSNTUscope* sharedNTUscope);
		~CudaMemoryManager();
		void CudaMemoryManagerStop();
		void CudaMemoryAllocate(int totalsample);
		void CudaSampleMap(U16* sampleMap);
		void getDeviceInf();
		void launchProcessSamples_LISSAJOUS(U16* pBuffer, U16* h_image);
	private:
		float64* d_tempImage = nullptr;
		U16* d_image = nullptr;
		U16* d_sampleMap = nullptr;
		U16* d_pBuffer = nullptr;
		int total_sample = 0;
		YSNTUscope* ntuscope;
		cudaStream_t stream;
		cudaDeviceProp prop;
		int num = 0;
	};
}
//__global__ void convertToU16_LISSAJOUS(float64* tempImage, U16* image, int xpixelnumber, int ypixelnumber);
__global__ void processSamples_LISSAJOUS(U16* pBuffer, float64* tempImage, U16* sampleMap, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq, double phase);
__global__ void invertBuffer(U16* Buffer, int totalsample, bool SPconduct, int SPoffsetvalue, bool choppermapping);
__global__ void normalizeImage(U16* image, float64* tempImage, U16* sampleMap, int xpixelnumber, int ypixelnumber);
//extern "C" void launchProcessSamples_LISSAJOUS_OLD(U16* pBuffer, U16* sampleMap, U16* h_image, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq);
#endif  