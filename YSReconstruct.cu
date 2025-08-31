#include "YSReconstruct.cuh"
namespace NTU {
    CudaMemoryManager::CudaMemoryManager(NTU::YSNTUscope* sharedNTUscope) {
        ntuscope = sharedNTUscope;
        cudaStreamCreate(&stream);
    }
    CudaMemoryManager::~CudaMemoryManager() {
        cudaFree(d_tempImage);
        cudaFree(d_image);
        cudaFree(d_sampleMap);
        cudaFree(d_pBuffer);
        cudaStreamDestroy(stream);
    }

    void CudaMemoryManager::CudaMemoryManagerStop() {
        cudaFree(d_tempImage);
        cudaFree(d_image);
        cudaFree(d_sampleMap);
        cudaFree(d_pBuffer);
    }

    void CudaMemoryManager::CudaMemoryAllocate(int totalsample) {
        cudaMalloc(&d_tempImage, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(float64));
        cudaMalloc(&d_image, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16));
        cudaMalloc(&d_sampleMap, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16));
        cudaMalloc(&d_pBuffer, totalsample * sizeof(U16));
        total_sample = totalsample;
        //getDeviceInf();

        printf("Parameter sent to GPU:  Pixelnumx=%d , Pixelnumy=%d , totalsample=%d \n", ntuscope->Pixelnumx, ntuscope->Pixelnumy, total_sample);
    }

    void CudaMemoryManager::CudaSampleMap(U16* sampleMap) {
        cudaMemcpyAsync(d_sampleMap, sampleMap, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16), cudaMemcpyHostToDevice, stream);
        cudaStreamSynchronize(stream);
        printf("Sample Map sent to GPU! \n");
    }

    void CudaMemoryManager::getDeviceInf() {
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

    void CudaMemoryManager::launchProcessSamples_LISSAJOUS(U16* pBuffer, U16* h_image) {

        // initialize the tensor
        cudaMemsetAsync(d_image, 0, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16), stream);
        cudaMemsetAsync(d_tempImage, 0, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(float64), stream);

        // copy to GPU
        cudaMemcpyAsync(d_pBuffer, pBuffer, total_sample * sizeof(U16), cudaMemcpyHostToDevice, stream);
        cudaStreamSynchronize(stream);

        // CUDA kerrnel
        int threadsPerBlock = 512;
        int blocksPerGrid = (total_sample + threadsPerBlock - 1) / threadsPerBlock;

        // start running
        invertBuffer << <blocksPerGrid, threadsPerBlock, 0, stream >> > (d_pBuffer, total_sample,ntuscope->YSapply_signalprocessing,ntuscope->YSoffsetvalueSP);
        processSamples_LISSAJOUS << <blocksPerGrid, threadsPerBlock, 0, stream >> > (
            d_pBuffer, d_tempImage, d_sampleMap, ntuscope->Pixelnumx, ntuscope->Pixelnumy, total_sample,
            ntuscope->samplesPerSec, ntuscope->xfreq, ntuscope->yfreq, ntuscope->lissajous_phase
            );

        // CUDA kernel to convert float64 to U16
        int totalPixels = ntuscope->Pixelnumx * ntuscope->Pixelnumy;
        blocksPerGrid = (totalPixels + threadsPerBlock - 1) / threadsPerBlock;
        //convertToU16_LISSAJOUS << <blocksPerGrid, threadsPerBlock, 0, stream >> > (d_tempImage, d_image, ntuscope->Pixelnumx, ntuscope->Pixelnumy);
        normalizeImage << <blocksPerGrid, threadsPerBlock, 0, stream >> > (d_image, d_tempImage, d_sampleMap, ntuscope->Pixelnumx, ntuscope->Pixelnumy);

        // copy from GPU to CPU
        cudaMemcpyAsync(h_image, d_image, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16), cudaMemcpyDeviceToHost, stream);

        cudaStreamSynchronize(stream);
    }

    //void CudaMemoryManager::launchProcessSamples_LISSAJOUS(U16* pBuffer, U16* h_image) {

    //    // initialize the tensor
    //    cudaMemset(d_image, 0, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16));
    //    cudaMemset(d_tempImage, 0, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(float64));

    //    // copy to GPU
    //    cudaMemcpy(d_pBuffer, pBuffer, total_sample * sizeof(U16), cudaMemcpyHostToDevice);

    //    // CUDA kerrnel
    //    int threadsPerBlock = 512;
    //    int blocksPerGrid = (total_sample + threadsPerBlock - 1) / threadsPerBlock;

    //    //invertBuffer << <blocksPerGrid, threadsPerBlock >> > (d_pBuffer, total_sample);

    //    // start running
    //    processSamples_LISSAJOUS << <blocksPerGrid, threadsPerBlock >> > (d_pBuffer, d_tempImage, d_sampleMap, ntuscope->Pixelnumx, ntuscope->Pixelnumy, total_sample, ntuscope->samplesPerSec, ntuscope->xfreq, ntuscope->yfreq);
    //    cudaDeviceSynchronize();
    //    
    //    // CUDA kernel to convert float64 to U16
    //    int totalPixels = ntuscope->Pixelnumx * ntuscope->Pixelnumy;
    //    blocksPerGrid = (totalPixels + threadsPerBlock - 1) / threadsPerBlock;
    //    convertToU16_LISSAJOUS << <blocksPerGrid, threadsPerBlock >> > (d_tempImage, d_image, ntuscope->Pixelnumx, ntuscope->Pixelnumy);
    //    cudaDeviceSynchronize();
    //    
    //    // copy from GPU to CPU
    //    cudaMemcpy(h_image, d_image, ntuscope->Pixelnumx * ntuscope->Pixelnumy * sizeof(U16), cudaMemcpyDeviceToHost);
    //}
}

// CUDA kernel to process the buffer in parallel
//__global__ void convertToU16_LISSAJOUS(float64* tempImage, U16* image, int xpixelnumber, int ypixelnumber) {
//    int pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;
//
//    if (pixelIndex < xpixelnumber * ypixelnumber) {
//        image[pixelIndex] = static_cast<U16>(tempImage[pixelIndex]);
//    }
//}

__global__ void invertBuffer(U16* Buffer, int totalsample, bool SPconduct, int SPoffsetvalue) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < totalsample) {

        if ((65535 - Buffer[i]) > 65535) {
            Buffer[i] = 65535 - 32768;
        }
        else {
            if (SPconduct == false) {
                if ((65535 - Buffer[i] - 32768) < 0) {
                    Buffer[i] = 0;
                }
                else {
                    Buffer[i] = 65535 - Buffer[i] - 32768;
                }
            }
            else if (SPconduct == true) {
                if ((65535 - Buffer[i] - SPoffsetvalue) < 0) {
                    Buffer[i] = 0;
                }
                else {
                    Buffer[i] = 65535 - Buffer[i] - SPoffsetvalue;
                }
            }

        }        
    }
}

//__global__ void processSamples_LISSAJOUS_OLD(U16* pBuffer, float64* tempImage, U16* sampleMap, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq) {
//    int sampleIndex = blockIdx.x * blockDim.x + threadIdx.x;
//
//    if (sampleIndex < totalsample) {
//        double t = double(sampleIndex) / samplesPerSec;
//
//        int xPixel = int(std::round((xpixelnumber / 2) * (1 + sin(2 * M_PI * double(xfreq) * t))));
//        int yPixel = int(std::round((ypixelnumber / 2) * (1 + sin(2 * M_PI * double(yfreq) * t))));
//
//        if (xPixel < xpixelnumber && yPixel < ypixelnumber && sampleMap[yPixel * xpixelnumber + xPixel] > 0) {
//            float64 normalizedValue = float64(pBuffer[sampleIndex]) / float64(sampleMap[yPixel * xpixelnumber + xPixel]);
//            tempImage[yPixel * xpixelnumber + xPixel] += normalizedValue;
//        }
//    }
//}

__global__ void processSamples_LISSAJOUS(U16* pBuffer, float64* tempImage, U16* sampleMap, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq , double phase) {
    int sampleIndex = blockIdx.x * blockDim.x + threadIdx.x;

    if (sampleIndex < totalsample) {
        double t = double(sampleIndex) / samplesPerSec;

        int xPixel = int(std::round((xpixelnumber / 2) * (1 - sin(2 * M_PI * double(xfreq) * t))));
        int yPixel = int(std::round((ypixelnumber / 2) * (1 - sin((2 * M_PI * double(yfreq) * t)+ phase))));

        if (xPixel < xpixelnumber && yPixel < ypixelnumber && sampleMap[yPixel * xpixelnumber + xPixel] > 0) {
            tempImage[yPixel * xpixelnumber + xPixel] += float64(pBuffer[sampleIndex]);
        }
    }
}

__global__ void normalizeImage(U16* image, float64* tempImage, U16* sampleMap, int xpixelnumber, int ypixelnumber) {
    int pixelIndex = blockIdx.x * blockDim.x + threadIdx.x;

    if (pixelIndex < xpixelnumber * ypixelnumber) {
        if (sampleMap[pixelIndex] > 0) {
            image[pixelIndex] = static_cast<U16>(tempImage[pixelIndex] / float64(sampleMap[pixelIndex]));
        }
    }
}

//extern "C" void launchProcessSamples_LISSAJOUS_OLD(U16* pBuffer, U16* sampleMap, U16* h_image, int xpixelnumber, int ypixelnumber, int totalsample, double samplesPerSec, int xfreq, int yfreq) {
//
//    // allocate GPU memory
//    float64* d_tempImage;
//    U16* d_image;
//    U16* d_sampleMap;
//    U16* d_pBuffer;
//
//    cudaMalloc(&d_tempImage, xpixelnumber * ypixelnumber * sizeof(float64));
//    cudaMalloc(&d_image, xpixelnumber * ypixelnumber * sizeof(U16));
//    cudaMalloc(&d_sampleMap, xpixelnumber * ypixelnumber * sizeof(U16));
//    cudaMalloc(&d_pBuffer, totalsample * sizeof(U16));
//
//    // initialize the tensor
//    cudaMemset(d_image, 0, xpixelnumber * ypixelnumber * sizeof(U16));
//    cudaMemset(d_tempImage, 0, xpixelnumber * ypixelnumber * sizeof(float64));
//
//    // copy to GPU
//    cudaMemcpy(d_sampleMap, sampleMap, xpixelnumber * ypixelnumber * sizeof(U16), cudaMemcpyHostToDevice);
//    cudaMemcpy(d_pBuffer, pBuffer, totalsample * sizeof(U16), cudaMemcpyHostToDevice);
//
//    // CUDA kerrnel
//    int threadsPerBlock = 256;
//    int blocksPerGrid = (totalsample + threadsPerBlock - 1) / threadsPerBlock;
//
//    // start running
//    //processSamples_LISSAJOUS << <blocksPerGrid, threadsPerBlock >> > (d_pBuffer, d_tempImage, d_sampleMap, xpixelnumber, ypixelnumber, totalsample, samplesPerSec, xfreq, yfreq , );
//    cudaDeviceSynchronize();
//
//    // CUDA kernel to convert float64 to U16
//    int totalPixels = xpixelnumber * ypixelnumber;
//    blocksPerGrid = (totalPixels + threadsPerBlock - 1) / threadsPerBlock;
//    convertToU16_LISSAJOUS << <blocksPerGrid, threadsPerBlock >> > (d_tempImage, d_image, xpixelnumber, ypixelnumber);
//    cudaDeviceSynchronize();
//
//    // copy from GPU to CPU
//    cudaMemcpy(h_image, d_image, xpixelnumber * ypixelnumber * sizeof(U16), cudaMemcpyDeviceToHost);
//
//    // free GPU memorry
//    cudaFree(d_tempImage);
//    cudaFree(d_image);
//    cudaFree(d_sampleMap);
//    cudaFree(d_pBuffer);
//}



//cudaStreamSynchronize(stream);
//U16* h_pBuffer = new U16[total_sample];
//cudaMemcpy(h_pBuffer, d_pBuffer, total_sample * sizeof(U16), cudaMemcpyDeviceToHost);
//std::ostringstream bufferCSVCUDAName;
//bufferCSVCUDAName << "C:/YuShin/Test_data_acq/cuda" << num << ".csv";
//std::ofstream file(bufferCSVCUDAName.str());
//if (file.is_open()) {
//    for (int i = 0; i < total_sample; ++i) {
//        file << h_pBuffer[i];
//        if (i < total_sample - 1) file << ",";
//    }
//    file << std::endl;
//    file.close();
//    std::cout << "CSV success" << std::endl;
//}
//else {
//    std::cerr << "fail！" << std::endl;
//}
//delete[] h_pBuffer;
//num++;