#pragma once
#ifndef MOTIONCORRECTION_H
#define MOTIONCORRECTION_H
#include <string>
#include <vector>
#include "YSNTUcore.h"


namespace NTU {
	using namespace std;
	using namespace std::chrono;
	class YSMotionCorrection {
	public:
		YSMotionCorrection();
		~YSMotionCorrection();
		vector<cv::Mat> readMultipageTiff(string& filepath);
		cv::Mat sumProjection(vector<cv::Mat>& stack);
		cv::Mat registerFrameECC(cv::Mat& moving, cv::Mat& fixed);
		cv::Mat warpImage(cv::Mat& img, cv::Mat& warpMat, cv::Size& size);
		void saveTransform(string& filename, cv::Mat& warpMat);
		cv::Mat loadTransform(string& filename);
		int findBrightestSlice(vector<vector<cv::Mat>>& allStacks);
		void applyTransformToStack(vector<cv::Mat>& inputStack, cv::Mat& warpMat, string& outputFolder, string& basename);
	};
}
#endif