#include "YSReconstructPost.cuh"
namespace NTU {
    ReconstructPost::ReconstructPost() {
        cudaStreamCreate(&stream);
    }
    ReconstructPost::~ReconstructPost() {
        cudaFree(d_image);
        cudaFree(d_sampleMap);
        cudaFree(d_pBuffer);
        cudaStreamDestroy(stream);
    }

    void ReconstructPost::CudaMemoryManagerStop() {
        cudaFree(d_image);
        cudaFree(d_sampleMap);
        cudaFree(d_pBuffer);
    }

    void ReconstructPost::CudaMemoryAllocate() {
        cudaMalloc(&d_image, pixelnumX * pixelnumY * sizeof(float64));
        cudaMalloc(&d_sampleMap, pixelnumX * pixelnumY * sizeof(float64));
        cudaMalloc(&d_pBuffer, total_sample * sizeof(float64));
        getDeviceInf();

        printf("Parameter sent to GPU:  Pixelnumx=%d , Pixelnumy=%d , totalsample=%d \n", pixelnumX, pixelnumY, total_sample);
    }

    void ReconstructPost::CudaSampleMap(float64* sampleMap) {
        cudaMemcpyAsync(d_sampleMap, sampleMap, 512 * 512 * sizeof(float64), cudaMemcpyHostToDevice, stream);
        cudaStreamSynchronize(stream);
        printf("Sample Map sent to GPU! \n");
    }

    void ReconstructPost::getDeviceInf() {
        int deviceCount = 0;
        cudaGetDeviceCount(&deviceCount);
        printf("Number of CUDA devices: %d\n", deviceCount);

        for (int i = 0; i < deviceCount; i++) {
            cudaGetDeviceProperties(&prop, i);
            printf("-----------------------\n", i, prop.name);
            printf("  Device %d: %s\n", i, prop.name);
            printf("  Compute capability: %d.%d\n", prop.major, prop.minor);
            printf("  Total global memory: %zu bytes\n", prop.totalGlobalMem);
            printf("  Shared memory per block: %zu bytes\n", prop.sharedMemPerBlock);
            printf("  Registers per block: %d\n", prop.regsPerBlock);
            printf("  Warp size: %d threads\n", prop.warpSize);
            printf("  Maximum threads per block: %d\n", prop.maxThreadsPerBlock);
            printf("  Maximum thread dimensions: (%d, %d, %d)\n",
                prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2]);
            printf("  Maximum grid dimensions: (%d, %d, %d)\n",
                prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2]);
            printf("  Clock rate: %d kHz\n", prop.clockRate);
            printf("  Multi-processor count: %d\n", prop.multiProcessorCount);
        }
    }

    void ReconstructPost::launchProcessSamples_LISSAJOUS(float64* pBuffer, float64* h_image) {

        // initialize the tensor
        cudaMemsetAsync(d_image, 0, pixelnumX * pixelnumY * sizeof(float64), stream);

        // copy to GPU
        cudaMemcpyAsync(d_pBuffer, pBuffer, total_sample * sizeof(float64), cudaMemcpyHostToDevice, stream);
        cudaStreamSynchronize(stream);

        // CUDA kerrnel
        int threadsPerBlock = 512;
        int blocksPerGrid = (total_sample + threadsPerBlock - 1) / threadsPerBlock;

        // start running
        invertBuffer_POST << <blocksPerGrid, threadsPerBlock, 0, stream >> > (d_pBuffer, total_sample);
        processSamples_LISSAJOUS_POST << <blocksPerGrid, threadsPerBlock, 0, stream >> > (
            d_pBuffer, d_image, d_sampleMap, pixelnumX, pixelnumY, total_sample,
            sampling_rate, freqx, freqy, phase
            );

        // CUDA kernel to convert float64 to float64
        int totalPixels = pixelnumX * pixelnumY;
        blocksPerGrid = (totalPixels + threadsPerBlock - 1) / threadsPerBlock;

        // copy from GPU to CPU
        cudaMemcpyAsync(h_image, d_image, pixelnumX * pixelnumY * sizeof(float64), cudaMemcpyDeviceToHost, stream);

        cudaStreamSynchronize(stream);
    }
}
__global__ void invertBuffer_POST(float64* Buffer, int totalsample) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < totalsample) {

        if ((65535 - Buffer[i]) > 65535) {
            Buffer[i] = 65535 - 32768;
        }
        else {

            if ((65535 - Buffer[i] - 32768) < 0) {
                Buffer[i] = 0;
            }
            else {
                Buffer[i] = 65535 - Buffer[i] - 32768;
            }
        }
    }
}

__global__ void processSamples_LISSAJOUS_POST(float64* pBuffer, float64* tempImage, float64* sampleMap, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq, double phase) {
    int sampleIndex = blockIdx.x * blockDim.x + threadIdx.x;

    if (sampleIndex < totalsample) {
        double t = double(sampleIndex) / samplesPerSec;

        int xPixel = int(std::round((xpixelnumber / 2) * (1 - sin(2 * M_PI * double(xfreq) * t))));
        int yPixel = int(std::round((ypixelnumber / 2) * (1 - sin((2 * M_PI * double(yfreq) * t) + phase))));

        if (xPixel < xpixelnumber && yPixel < ypixelnumber && sampleMap[yPixel * xpixelnumber + xPixel] > 0) {
            tempImage[yPixel * xpixelnumber + xPixel] += float64(pBuffer[sampleIndex]);
        }
    }
}

__global__ void normalizeImage_POST(float64* image, float64* tempImage, float64* sampleMap, int xpixelnumber, int ypixelnumber) {
    int pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;

    if (pixelIndex < xpixelnumber * ypixelnumber) {
        if (sampleMap[pixelIndex] > 0) {
            image[pixelIndex] = static_cast<float64>(tempImage[pixelIndex] / float64(sampleMap[pixelIndex]));
        }
    }
}
