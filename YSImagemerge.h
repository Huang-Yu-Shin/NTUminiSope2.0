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
#include <opencv2/photo.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/core.hpp>
#include <opencv2/cudafilters.hpp>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <map>
#include <cmath>
#include <math.h>
#include <functional>
#include <deque>
#include <iomanip>
#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <future>
#include "YSCalculatePara.cuh"
#include "YSReconstructPost.cuh"
namespace NTU {
	using namespace std;
	using namespace std::chrono;
	class YSImageMerge {
	public:

		YSImageMerge(NTU::CudaCalPara* cudacalpara, NTU::ReconstructPost* cudareconstruct);
		~YSImageMerge();

		void Clear();

		//General
		string RASTERFolderPath;
		string RASTERPath, LISSAJOUSPath;
		string LISSAJOUSFolderPath;
		int channelcount = 1;
		double ampx = 0;
		double ampy = 0;
		int linefreq = 0;
		int num_of_line = 0;
		int xfreq = 0;
		int yfreq = 0;
		double lissajousphase = 0;
		int num_of_sample = 0;
		int samplingrate = 0;
		int chopperfreq = 0;
		double chopperphase = 0;
		double progresspercentage_rlmergedisplay = 0;
		double progresspercentage_reconstruct = 0;
		double progresspercentage_extractsample = 0;
		int pixelnum = 256;
		U32 savestart_frameorder = 0;

		//SampleMap
		cv::Mat sin_weight;
		int* sampleMap = nullptr;
		float64* sampleMap_f64 = nullptr;
		void Generate2D_RASTER_SampleMap();
		void Generate2D_LISSAJOUS_SampleMap();

		//Processimage
		void displayprocessImage(string& input_path);
		void processImage(string& input_path);

		//Calculate parameters
		CudaCalPara* calpara;
		void CreateCUDACalPara(string Path, int width, int height, double samplingrate);
		static void onMouse(int event, int x, int y, int flags, void* userdata);
		void moveTarget(int dx, int dy);
		void CalculateParameter(double sampleingrate);
		void CalculateParameter_multi(double sampleingrate);

		//Scan preview
		double samplingrate_SP = 50000000;
		int xpixelnum = 256;
		int ypixelnum = 256;
		int xfreq_SP = 1500;
		int yfreq_SP = 2000;
		double phase_SP = 0;
		int framerate_SP = 500;
		int phaseindex_SP = 0;
		int offsetx_SP = 0;
		int offsety_SP = 0;
		double scansizeratio = 1;
		int chopper_phase = 0;
		int chopper_freq_upscale = 1;
		bool loaddata = false;
		bool apply_chopper = false;
		void CreateScanPreview(string Path);
		void ScanmodePreview();
		bool GetChopperTTLSignal(int sampleIndex, int totalSamples);

		//Transfer to .tiff
		bool raster_reconstruct_mode = false;
		void ConvertBinToTiff_lisssajous_parallel();
		void ProcessSingleBin_lissajous_1ch(const string& bin_path, const string& tiff_path);
		void ProcessSingleBin_lissajous_2chs(const string& bin_path, const string& tiff_ch1_path, const string& tiff_ch2_path);
		void ProcessSingleBin_raster_1ch(const string& bin_path, const string& ch1_path, const string& raw_ch1_path);
		void ConvertBinToTiff_raster_1ch_parallel();
		void ConvertBinToTiff_raster_2chs_parallel();
		void ProcessSingleBin_raster_2ch(const string& bin_path, const string& ch1_path, const string& raw_ch1_path, const string& ch2_path, const string& raw_ch2_path);
		void ProcessSingleBin_raster_2ch_magnitude(const string& bin_path,const string& out_path, const string& raw_out_path);

		//Scanmode merge
		bool savestate = 0;
		bool displaywithscan = 0;
		int offsetx = 0;
		int offsety = 0;
		double upscalefactorX = 1;
		double upscalefactorY = 1;
		int offset_lissajous = 0;
		void DISPLAYRASTER();
		void RLMERGEDISPLAY();
		void LoadLissajousImage();

		//Reconstruct
		ReconstructPost* reconstruct;
		bool quickreconstruct = false;
		bool firstreconstruct = true;
		cv::Mat averageImg16, averageImg16_ch2;
		void DisplayReconstructedLissajousImage();
		void ReconstructAllTiffInFolder();
		void ReconstructAllTiffInFolder_parallel();
		void ReconstructOneTiff_1ch(const string& fullPath, const string& savePath, int image_index_for_name);
		void ReconstructOneTiff_2ch(const string& fullPath_ch1, const string& fullPath_ch2, const string& saveDir_ch1, const string& saveDir_ch2, int image_index_for_name);

		//Extract ROI sample
		int ROIleftupX = 0; 
		int ROIleftupY = 0;
		int ROIrightbottomX = 0;
		int ROIrightbottomY = 0;
		void ExportROISampleMeanCSV();

		//Extract hymograph
		cv::Mat rasterImage_EK;                      
		cv::Mat lissajousImage_EK;
		vector<cv::Point> scannedPixelList;
		double startPixelIndexpercentage = 0;
		double endPixelIndexpercentage = 0;
		void LoadLissajousAndMerge();
		void DrawLissajousPattern();
		void ExtractKymograph();

		//Supplementart functions
		void LOADspec(string path);
		bool LissajousIdxFromSample(int sample_idx, double sampling_rate, int widthpixelnum, int heightpixelnum, double xfreq, double yfreq, double lissajous_phase, int& x_out, int& y_out);
		void CopyScanSpecCSV(string& sourceFolder, string& targetFolder);
		int extractNumber(const string& filename);
		void AddtextOnImg(cv::Mat& IMGDP8, int Xpixelsize, int Ypixelsize);
		void AVERAGE(string folderPath, cv::Mat& avgImage);
		void Dynamic_Contrast_Enhancement_CUDA(cv::cuda::GpuMat& IMGDP16_GPU, cv::cuda::GpuMat& IMGDP16OUTPUT_GPU, float alpha_max);
private:
		cv::Mat rasterImage;
		cv::cuda::Stream stream;
		int rasterimgindex,lissajousimgindex;
		float64* LissajousrawData = nullptr;
		float64* LissajousrawData_ch2 = nullptr;
		float64* LissajoustempData = nullptr;
		float64* LissajoustempData_ch2 = nullptr;
		float64* Image = nullptr;
		float64* Image_ch2 = nullptr;
	};
}

#endif