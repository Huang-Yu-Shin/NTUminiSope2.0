#include "YSCalculatePara.cuh"
namespace NTU {
    CudaCalPara::CudaCalPara() {
        cudaStreamCreate(&stream);
        initParams();
    }
    CudaCalPara::~CudaCalPara() {
        cudaFree(fxDev);
        cudaFree(fyDev);
        cudaFree(fpsDev);
        cudaFree(phaseDev);
        cudaFree(resultsDev);
        cudaFree(timesperframeDev);
        cudaStreamDestroy(stream);
    }

    void CudaCalPara::initParams() {
        double fxSet[FX_COUNT] = { 1500,2000, 2000, 2000, 2100, 2088, 2052, 1980, 1620 };//
        double fySet[FX_COUNT] = { 2000,2100, 2010, 3000, 2160, 2160, 2160, 2160, 2160 };//
        double fpsSet[FX_COUNT] = { 500,100, 10, 1000, 60, 72, 108, 180, 540 };//
        for (int i = 0; i < FX_COUNT; ++i) {
            for (int j = 0; j < PHASE_COUNT; ++j) {
                int idx = i * PHASE_COUNT + j;
                fxHost[idx] = fxSet[i];
                fyHost[idx] = fySet[i];
                fpsHost[idx] = fpsSet[i];
                phaseHost[idx] = 2.0 * M_PI * j / PHASE_COUNT;
            }
        }

        cudaMalloc(&fxDev, TOTAL_PARAMS * sizeof(double));
        cudaMalloc(&fyDev, TOTAL_PARAMS * sizeof(double));
        cudaMalloc(&fpsDev, TOTAL_PARAMS * sizeof(double));
        cudaMalloc(&phaseDev, TOTAL_PARAMS * sizeof(double));
        cudaMalloc(&resultsDev, TOTAL_PARAMS * sizeof(bool));
        cudaMalloc(&timesperframeDev, TOTAL_PARAMS * sizeof(int));

        cudaMemcpy(fxDev, fxHost, TOTAL_PARAMS * sizeof(double), cudaMemcpyHostToDevice);
        cudaMemcpy(fyDev, fyHost, TOTAL_PARAMS * sizeof(double), cudaMemcpyHostToDevice);
        cudaMemcpy(fpsDev, fpsHost, TOTAL_PARAMS * sizeof(double), cudaMemcpyHostToDevice);
        cudaMemcpy(phaseDev, phaseHost, TOTAL_PARAMS * sizeof(double), cudaMemcpyHostToDevice);
    }

    void CudaCalPara::FindLissajousPara() {
        checkPara << <1, TOTAL_PARAMS >> > (fxDev, fyDev, phaseDev, fpsDev, resultsDev, timesperframeDev, xTarget, yTarget, sampleRate, width, height, TwoPointsCal,xTarget2,yTarget2);
        cudaMemcpy(resultHost, resultsDev, TOTAL_PARAMS * sizeof(bool), cudaMemcpyDeviceToHost);
        cudaMemcpy(timesperframeHost, timesperframeDev, TOTAL_PARAMS * sizeof(int), cudaMemcpyDeviceToHost);
        cout << "----------------------------------------" << endl;
        for (int i = 0; i < TOTAL_PARAMS; ++i) {
            if (resultHost[i]) {
                cout << "Hit: fx=" << fxHost[i] << ", fy=" << fyHost[i] << ", phase=" << phaseHost[i] << " rad" << ", fps=" << fpsHost[i] << ", timesperframe=" << timesperframeHost[i] << endl;
            }
        }

    }

    void CudaCalPara::GetBresenhamLine(int x0, int y0, int x1, int y1,std::vector<int>& xList, std::vector<int>& yList) {
        int dx = abs(x1 - x0), sx = (x0 < x1 ? 1 : -1);
        int dy = -abs(y1 - y0), sy = (y0 < y1 ? 1 : -1);
        int err = dx + dy;

        while (true) {
            xList.push_back(x0);
            yList.push_back(y0);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void CudaCalPara::FindLissajousPara_multi() {
        GetBresenhamLine(xTarget, yTarget, xTarget2, yTarget2, targetXList, targetYList);
        int* devX;
        int* devY;
        cudaMalloc(&devX, targetXList.size() * sizeof(int));
        cudaMalloc(&devY, targetYList.size() * sizeof(int));
        cudaMemcpy(devX, targetXList.data(), targetXList.size() * sizeof(int), cudaMemcpyHostToDevice);
        cudaMemcpy(devY, targetYList.data(), targetYList.size() * sizeof(int), cudaMemcpyHostToDevice);

        checkPara_multi << <1, TOTAL_PARAMS >> > (fxDev, fyDev, phaseDev, fpsDev, resultsDev, timesperframeDev, devX, devY, static_cast<int>(targetXList.size()), sampleRate, width, height);

        cudaMemcpy(resultHost, resultsDev, TOTAL_PARAMS * sizeof(bool), cudaMemcpyDeviceToHost);
        cudaMemcpy(timesperframeHost, timesperframeDev, TOTAL_PARAMS * sizeof(int), cudaMemcpyDeviceToHost);
        cout << "----------------------------------------" << endl;
        for (int i = 0; i < TOTAL_PARAMS; ++i) {
            if (resultHost[i]) {
                cout << "Hit: fx=" << fxHost[i] << ", fy=" << fyHost[i] << ", phase=" << phaseHost[i]
                    << " rad, fps=" << fpsHost[i] << ", timesperframe=" << timesperframeHost[i] << endl;
            }
        }

        cudaFree(devX);
        cudaFree(devY);
    }
}
__global__ void checkPara(double* fx, double* fy, double* phase, double* framerate, bool* out, int* timesperframe, int targetx, int targety, double rate, int w, int h, bool twopointscal, int targetx2, int targety2) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    double fxv = fx[i];
    double fyv = fy[i];
    double phv = phase[i];
    double duration = 1.0 / framerate[i];
    int sampleCount = int(rate * duration);
    sampleCount = sampleCount - (sampleCount % 128);
    timesperframe[i] = 0;
    out[i] = false;

    bool firstpointmatch = false;
    bool secondpointmatch = false;

    if (twopointscal == true) {
        for (int j = 0; j < sampleCount; ++j) {
            double t = double(j) / rate;
            int xPixel = int(std::round((w / 2) * (1 + sin(2 * M_PI * double(fxv) * t))));
            int yPixel = int(std::round((h / 2) * (1 - sin((2 * M_PI * double(fyv) * t) + phv))));
            if (xPixel == targetx && yPixel == targety) {
                firstpointmatch = true;
            }
            if (xPixel == targetx2 && yPixel == targety2) {
                secondpointmatch = true;
            }
        }
        if (firstpointmatch == true && secondpointmatch == true){
            out[i] = true;
        }
        
    }
    else if (twopointscal == false) {
        for (int j = 0; j < sampleCount; ++j) {
            double t = double(j) / rate;
            int xPixel = int(std::round((w / 2) * (1 + sin(2 * M_PI * double(fxv) * t))));
            int yPixel = int(std::round((h / 2) * (1 - sin((2 * M_PI * double(fyv) * t) + phv))));
            if (xPixel == targetx && yPixel == targety) {
                out[i] = true;
                timesperframe[i]++;
            }
        }
    }

}

__global__ void checkPara_multi(double* fx, double* fy, double* phase, double* framerate,bool* out, int* timesperframe,int* targetXList, int* targetYList, int numPoints, double rate, int w, int h) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;
    double fxv = fx[i];
    double fyv = fy[i];
    double phv = phase[i];
    double duration = 1.0 / framerate[i];
    int sampleCount = int(rate * duration);
    sampleCount = sampleCount - (sampleCount % 128);
    timesperframe[i] = 0;
    out[i] = false;

    // fixed pixel hit mask（max=512）
    bool pixelHit[512];
    for (int p = 0; p < numPoints; ++p) {
        pixelHit[p] = false;
    }

    for (int j = 0; j < sampleCount; ++j) {
        double t = double(j) / rate;
        int xPixel = int(round((w / 2.0) * (1 + sin(2.0 * M_PI * fxv * t))));
        int yPixel = int(round((h / 2.0) * (1 - sin(2.0 * M_PI * fyv * t + phv))));

        for (int k = 0; k < numPoints; ++k) {
            if (xPixel == targetXList[k] && yPixel == targetYList[k]) {
                pixelHit[k] = true;
            }
        }
    }

    bool allHit = true;
    for (int p = 0; p < numPoints; ++p) {
        if (!pixelHit[p]) {
            allHit = false;
            break;
        }
    }
    if (allHit) {
        out[i] = true;
    }
}

