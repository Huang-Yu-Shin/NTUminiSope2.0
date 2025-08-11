#pragma once
#ifndef IMAGEMERGE_H
#define IMAGEMERGE_H

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <windows.h>
#include <msclr/marshal_cppstd.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
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
#include <stdio.h>
#include <algorithm>
#include "YSCalculatePara.cuh"

namespace NTU {
	using namespace std;
	using namespace std::chrono;
	class YSImageMerge {
	public:

		YSImageMerge(NTU::CudaCalPara* cudacalpara);
		~YSImageMerge();

		//Processimage
		void processImage(std::string& input_path);

		//Scanmode merge & Reconstruct
		void LOADspec(string path);
		void ConvertBinToTiff();
		void CopyScanSpecCSV(string& sourceFolder, string& targetFolder);
		void DISPLAYRASTER();
		void RLMERGEDISPLAY();
		void AVERAGE(string folderPath, cv::Mat& avgImage);
		void GenerateSampleMap();
		void LoadLissajousImage();
		void DisplayReconstructedLissajousImage();
		void ReconstructAllTiffInFolder();
		int extractNumber(const string& filename);
		void Addtext_RLMERGEDISPLAY(cv::Mat& IMGDP8, int Xpixelsize, int Ypixelsize);
		bool savestate = 0;
		bool displaywithscan = 0;

		int offsetx = 0;
		int offsety = 0;
		double upscalefactorX = 1;
		double upscalefactorY = 1;
		int offset_lissajous = 0;

		string RASTERPath,LISSAJOUSPath;
		string LISSAJOUSFolderPath;

		int xfreq, yfreq, num_of_sample, samplingrate;
		int pixelnum = 512;
		double lissajous_phase;
		double progresspercentage = 0;
		double progresspercentage_scanmodemerge = 0;
		cv::cuda::Stream stream;
		//Calculate parameters
		CudaCalPara* calpara;
		cv::Mat rasterImage;
		void CreateCUDACalPara(string Path,int width, int height, double samplingrate);
		static void onMouse(int event, int x, int y, int flags, void* userdata);
		void moveTarget(int dx, int dy);
		void CalculateParameter(double sampleingrate);
		void CalculateParameter_multi(double sampleingrate);

		//Scan preview
		double samplingrate_SP = 50000000;
		int width_SP = 512;
		int height_SP = 512;
		int xfreq_SP = 1500;
		int yfreq_SP = 2000;
		double phase_SP = 0;
		int framerate_SP = 500;
		int phaseindex_SP = 0;
		int chopper_phase = 0;
		int chopper_freq_upscale = 1;
		bool loaddata = false;
		bool apply_chopper = false;
		void CreateScanPreview(string Path);
		void ScanmodePreview();
		bool GetChopperTTLSignal(int sampleIndex, int totalSamples);

		//Extract hymograph
		string RASTERPath_EK, LISSAJOUSPath_EK;
		string LISSAJOUSFolderPath_EK;
		int xfreq_EK, yfreq_EK, num_of_sample_EK, samplingrate_EK;
		double lissajous_phase_EK;
		cv::Mat rasterImage_EK;                      
		cv::Mat lissajousImage_EK;
		vector<cv::Point> scannedPixelList;
		double startPixelIndexpercentage = 0;
		double endPixelIndexpercentage = 0;
		void LOADspec_EK(string path);
		void LoadLissajousAndMerge();
		void DrawLissajousPattern();
		void ExtractKymograph();
	private:
		int rasterimgindex,lissajousimgindex;
		double* LissajousrawData = nullptr;
		double* LissajoustempData = nullptr;
		int* sampleMap = nullptr;
		double* Image = nullptr;

	};
}

#endif