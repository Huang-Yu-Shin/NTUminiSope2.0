#pragma unmanaged
#include "YSMotionCorrection.h"

namespace NTU {

    YSMotionCorrection::YSMotionCorrection() {
    }

    YSMotionCorrection::~YSMotionCorrection() {
    }

    // 讀 multi-page TIFF
    vector<cv::Mat> YSMotionCorrection::readMultipageTiff(string& filepath) {
        std::vector<cv::Mat> stack;
        cv::imreadmulti(filepath, stack, cv::IMREAD_ANYDEPTH);
        return stack;
    }

    // Z-stack sum projection
    cv::Mat YSMotionCorrection::sumProjection(vector<cv::Mat>& stack)
    {
        cv::Mat sum = cv::Mat::zeros(stack[0].size(), stack[0].type());
        for (const auto& img : stack) {
            sum += img;
        }
        return sum;
    }

    // ECC translation registration
    cv::Mat YSMotionCorrection::registerFrameECC(cv::Mat& moving,cv::Mat& fixed)
    {
        cv::Mat warp = cv::Mat::eye(2, 3, CV_32F);
        //int motionType = cv::MOTION_TRANSLATION;
        int numberOfIterations = 500;
        double termination_eps = 1e-6;

        //double cc = cv::findTransformECC(
        //    fixed, moving, warp, motionType,
        //    cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
        //        numberOfIterations, termination_eps)
        //);

        return warp;
    }

    // warp image
    cv::Mat YSMotionCorrection::warpImage(cv::Mat& img, cv::Mat& warpMat, cv::Size& size)
    {
        cv::Mat result;
        cv::warpAffine(img, result, warpMat, size, cv::INTER_LINEAR);
        return result;
    }

    // save warp matrix
    void YSMotionCorrection::saveTransform(string& filename, cv::Mat& warpMat)
    {
        cv::FileStorage fs(filename, cv::FileStorage::WRITE);
        fs << "warpMat" << warpMat;
        fs.release();
    }

    cv::Mat YSMotionCorrection::loadTransform(string& filename)
    {
        cv::FileStorage fs(filename, cv::FileStorage::READ);
        cv::Mat warpMat;
        fs["warpMat"] >> warpMat;
        fs.release();
        return warpMat;
    }

    // find brightest slice
    int YSMotionCorrection::findBrightestSlice(vector<std::vector<cv::Mat>>& allStacks)
    {
        double maxMean = -1;
        int brightestIndex = 0;

        for (size_t i = 0; i < allStacks.size(); i++)
        {
            double sum = 0;
            size_t totalPixels = 0;
            for (const auto& frame : allStacks[i])
            {
                sum += cv::sum(frame)[0];
                totalPixels += frame.total();
            }
            double mean = sum / totalPixels;
            if (mean > maxMean)
            {
                maxMean = mean;
                brightestIndex = i;
            }
        }
        return brightestIndex;
    }

    // apply warp to every frame
    void YSMotionCorrection::applyTransformToStack(vector<cv::Mat>& inputStack,cv::Mat& warpMat,string& outputFolder,string& basename)
    {
        //std::filesystem::create_directories(outputFolder);

        //for (size_t i = 0; i < inputStack.size(); i++)
        //{
        //    auto warped = warpImage(inputStack[i], warpMat, inputStack[i].size());
        //    std::string fname = outputFolder + "/" + basename + "_frame_" + std::to_string(i + 1) + ".tif";
        //    cv::imwrite(fname, warped);
        //}
    }
}