#include "YSImageMerge.h"

namespace NTU {

	YSImageMerge::YSImageMerge(NTU::CudaCalPara* cudacalpara) {
		calpara = cudacalpara;
	}

	YSImageMerge::~YSImageMerge() {
		delete[] LissajousrawData;
		delete[] LissajoustempData;
		delete[] sampleMap;
		delete[] Image;
		calpara->~CudaCalPara();
		cv::destroyAllWindows();
	}

	//Process image
	void YSImageMerge::processImage(std::string& input_path) {
		std::string output_folder = input_path + "\\process";
		CreateDirectoryA(output_folder.c_str(), NULL);

		string FolderPathwithtiff = input_path + "\\*.tiff";
		// Store valid filenames
		vector<string> filenames;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(FolderPathwithtiff.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			cout << "Error: No .tiff files found in directory: " << FolderPathwithtiff << endl;
			return;
		}
		// Collect all valid filenames
		do {
			filenames.push_back(findFileData.cFileName);
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
		// Sort filenames based on the numeric part
		sort(filenames.begin(), filenames.end(), [this](const string& a, const string& b) {
			return extractNumber(a) < extractNumber(b);
			});

		int total_count = filenames.size();


		for (const auto& filename : filenames) {
			string fullPath = input_path + "\\" + filename;

			cv::Mat img = cv::imread(fullPath, cv::IMREAD_UNCHANGED);
			if (img.empty()) {
				std::cerr << "Failed to load: " << fullPath << std::endl;
				continue;
			}

			cv::cuda::GpuMat gpu_image;
			gpu_image.upload(img, stream);

			double scale = 65535.0 / (50000.0 - 0.0);
			double shift = 0.0 * scale;
			gpu_image.convertTo(gpu_image, -1, scale, shift, stream);
			cv::cuda::threshold(gpu_image, gpu_image, 65535, 65535, cv::THRESH_TRUNC, stream);
			cv::cuda::threshold(gpu_image, gpu_image, 0, 0, cv::THRESH_TOZERO, stream);

			cv::cuda::GpuMat gpu_8bit;
			gpu_image.convertTo(gpu_8bit, CV_8UC1, 1.0 / 257.0, 0.0, stream);

			cv::Mat LUT_matrix(1, 256, CV_8UC1);
			uchar* pointer = LUT_matrix.ptr();
			for (int i = 0; i < 256; i++) {
				pointer[i] = (int)(pow((double)(i / 255.0), 1.1) * 255.0);
			}
			cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
			cudaLookUpTable->transform(gpu_8bit, gpu_8bit, stream);
			//cv::cuda::equalizeHist(gpu_8bit, gpu_8bit, stream);
			gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, 5, 150 - 150, stream);
			stream.waitForCompletion();

			cv::Mat result;
			gpu_8bit.download(result, stream);
			stream.waitForCompletion();
			std::string output_path = output_folder + "\\" + filename;
			cv::imwrite(output_path, result);
			std::cout << "Saved: " << output_path << std::endl;
		}

		std::cout << "All TIFFs processed." << std::endl;
	}

	//Calculate Parameter
	void YSImageMerge::CreateCUDACalPara(string Path,int width, int height, double samplingrate) {
		calpara->width = width;
		calpara->height = height;
		calpara->sampleRate = samplingrate;
		cv::Mat image = cv::imread(Path, cv::IMREAD_COLOR);
		if (image.empty()) {
			cerr << "Error: Unable to load image at path: " << Path << endl;
			return;
		}
		cv::resize(image, rasterImage, cv::Size(calpara->width, calpara->height), 0, 0, cv::INTER_NEAREST);

		cv::namedWindow("CalPara", cv::WINDOW_AUTOSIZE);
		cv::setMouseCallback("CalPara", YSImageMerge::onMouse,this);
		cv::imshow("CalPara", rasterImage);
		cv::waitKey(0);
	}

	void YSImageMerge::onMouse(int event, int x, int y, int flags, void* userdata) {
		if (event == cv::EVENT_LBUTTONDOWN) {
			auto* self = static_cast<YSImageMerge*>(userdata);

			if (self->calpara->TwoPointsCal == true) {
				self->calpara->xTarget2 = x;
				self->calpara->yTarget2 = y;
				
				if (self->calpara->LineCal == false) {
					cv::Mat dotImage = self->rasterImage.clone();
					cv::circle(dotImage, cv::Point(self->calpara->xTarget, self->calpara->yTarget), 2, cv::Scalar(0, 0, 255), -1);
					cv::circle(dotImage, cv::Point(self->calpara->xTarget2, self->calpara->yTarget2), 2, cv::Scalar(0, 255, 255), -1);
					cv::imshow("CalPara", dotImage);
				}
				else if (self->calpara->LineCal == true) {
					vector<int> targetXList, targetYList;
					self->calpara->GetBresenhamLine(self->calpara->xTarget, self->calpara->yTarget,self->calpara->xTarget2, self->calpara->yTarget2,targetXList, targetYList);
					cv::Mat lineImage = self->rasterImage.clone();
					for (size_t i = 0; i < targetXList.size(); ++i) {
						int px = targetXList[i];
						int py = targetYList[i];
						if (px == self->calpara->xTarget && py == self->calpara->yTarget)
							cv::circle(lineImage, cv::Point(px, py), 1, cv::Scalar(0, 0, 255), -1);
						else if (px == self->calpara->xTarget2 && py == self->calpara->yTarget2)
							cv::circle(lineImage, cv::Point(px, py), 1, cv::Scalar(0, 255, 255), -1);
						else
							cv::circle(lineImage, cv::Point(px, py), 1, cv::Scalar(255, 0, 0), -1);
					}
					string label = "Total pixel count = " + to_string(targetXList.size());
					int fontFace = cv::FONT_HERSHEY_SIMPLEX;
					double fontScale = 0.5;
					int thickness = 1;
					int baseline = 0;
					cv::Size textSize = cv::getTextSize(label, fontFace, fontScale, thickness, &baseline);
					cv::Point textOrg(lineImage.cols - textSize.width - 10, lineImage.rows - 10);
					cv::putText(lineImage, label, textOrg, fontFace, fontScale, cv::Scalar(255, 255, 255), thickness);
					cv::imshow("CalPara", lineImage);
				}
			}
			else if (self->calpara->TwoPointsCal == false) {
				self->calpara->xTarget = x;
				self->calpara->yTarget = y;

				cv::Mat dotImage = self->rasterImage.clone();
				cv::circle(dotImage, cv::Point(self->calpara->xTarget, self->calpara->yTarget), 2, cv::Scalar(0, 0, 255), -1);
				cv::imshow("CalPara", dotImage);
			}
			
		}
	}

	void YSImageMerge::moveTarget(int dx, int dy){
		if (calpara->TwoPointsCal == true) {
			if (((calpara->xTarget2 + dx) < 0) ||
				((calpara->yTarget2 + dy) < 0) ||
				((calpara->xTarget2 + dx) > (rasterImage.cols - 1)) ||
				((calpara->yTarget2 + dy) > (rasterImage.rows - 1))) {
				return;
			}
			else {
				calpara->xTarget2 = calpara->xTarget2 + dx;
				calpara->yTarget2 = calpara->yTarget2 + dy;
			}


			if (calpara->LineCal == false) {
				cv::Mat dotImage = rasterImage.clone();
				cv::circle(dotImage, cv::Point(calpara->xTarget, calpara->yTarget), 2, cv::Scalar(0, 0, 255), -1);
				cv::circle(dotImage, cv::Point(calpara->xTarget2, calpara->yTarget2), 2, cv::Scalar(0, 255, 255), -1);
				cv::imshow("CalPara", dotImage);
			}
			else if (calpara->LineCal == true) {
					vector<int> targetXList, targetYList;
					calpara->GetBresenhamLine(calpara->xTarget, calpara->yTarget,calpara->xTarget2,calpara->yTarget2,targetXList, targetYList);
					cv::Mat lineImage = rasterImage.clone();
					for (size_t i = 0; i < targetXList.size(); ++i) {
						int px = targetXList[i];
						int py = targetYList[i];
						if (px == calpara->xTarget && py == calpara->yTarget)
							cv::circle(lineImage, cv::Point(px, py), 1, cv::Scalar(0, 0, 255), -1);
						else if (px == calpara->xTarget2 && py == calpara->yTarget2)
							cv::circle(lineImage, cv::Point(px, py), 1, cv::Scalar(0, 255, 255), -1);
						else
							cv::circle(lineImage, cv::Point(px, py), 1, cv::Scalar(255, 0, 0), -1);
					}
					string label = "Total pixel count = " + to_string(targetXList.size());
					int fontFace = cv::FONT_HERSHEY_SIMPLEX;
					double fontScale = 0.5;
					int thickness = 1;
					int baseline = 0;
					cv::Size textSize = cv::getTextSize(label, fontFace, fontScale, thickness, &baseline);
					cv::Point textOrg(lineImage.cols - textSize.width - 10, lineImage.rows - 10);
					cv::putText(lineImage, label, textOrg, fontFace, fontScale, cv::Scalar(0, 255, 255), thickness);
					cv::imshow("CalPara", lineImage);
			}
		}
		else if (calpara->TwoPointsCal == false) {
			if (((calpara->xTarget + dx) < 0) ||
				((calpara->yTarget + dy) < 0) ||
				((calpara->xTarget + dx) > (rasterImage.cols - 1)) ||
				((calpara->yTarget + dy) > (rasterImage.rows - 1))) {
				return;
			}
			else {
				calpara->xTarget = calpara->xTarget + dx;
				calpara->yTarget = calpara->yTarget + dy;
			}
			cv::Mat dotImage = rasterImage.clone();
			//cout << "Saved target to CudaCalPara: " << calpara->xTarget << ", " << calpara->yTarget << endl;
			cv::circle(dotImage, cv::Point(calpara->xTarget, calpara->yTarget), 2, cv::Scalar(0, 0, 255), -1);
			cv::imshow("CalPara", dotImage);
		}

	}

	void YSImageMerge::CalculateParameter(double sampleingrate) {
		calpara->sampleRate = sampleingrate;
		calpara->FindLissajousPara();
	}

	void YSImageMerge::CalculateParameter_multi(double sampleingrate) {
		calpara->sampleRate = sampleingrate;
		calpara->FindLissajousPara_multi();
	}

	//Scan Preview
	void YSImageMerge::CreateScanPreview(string Path) {
		cv::Mat image = cv::imread(Path, cv::IMREAD_UNCHANGED);
		if (image.empty()) {
			cerr << "Error: Unable to load image at path: " << Path << endl;
			return;
		}
		cv::Mat image_resized;
		cv::resize(image, image_resized, cv::Size(width_SP, height_SP), 0, 0, cv::INTER_NEAREST);
		loaddata = true;

		cv::Mat raster_8U;
		image_resized.convertTo(raster_8U, CV_8UC1, 1.0 / 257.0, 0.0);
		cv::Mat displayimage;
		cv::cvtColor(raster_8U, displayimage, cv::COLOR_GRAY2BGR);
		rasterImage = displayimage.clone();
	}

	void YSImageMerge::ScanmodePreview() {
		cout << chopper_phase << endl;
		cv::Mat DisplayIMG = rasterImage.clone();
		double duration = 1.0 / double(framerate_SP);
		bool isChopperFreqDouble;
		if (framerate_SP > 120 && framerate_SP<1000) {
			isChopperFreqDouble = false;
		}
		else {
			isChopperFreqDouble = true;
		}

		int totalSamples = int(samplingrate_SP * duration);
		totalSamples = totalSamples - (totalSamples % 128);
		for (int i = 0; i < totalSamples; ++i) {
			double t = double(i) / samplingrate_SP;
			int xPixel = int(round((width_SP / 2) * (1 + sin(2 * M_PI * double(xfreq_SP) * t))));
			int yPixel = int(round((height_SP / 2) * (1 - sin((2 * M_PI * double(yfreq_SP) * t) + phase_SP))));

			if (xPixel >= 0 && xPixel < width_SP && yPixel >= 0 && yPixel < height_SP) {

				if (apply_chopper == true) {
					if (GetChopperTTLSignal(i, totalSamples,isChopperFreqDouble)) {
						DisplayIMG.at<cv::Vec3b>(yPixel, xPixel) = cv::Vec3b(0, 0, 255);
					}
					else {
						DisplayIMG.at<cv::Vec3b>(yPixel, xPixel) = cv::Vec3b(0, 255, 0);
					}
				}
				else {
					DisplayIMG.at<cv::Vec3b>(yPixel, xPixel) = cv::Vec3b(0, 0, 255);
				}

			}
		}

		cv::imshow("ScanPreview", DisplayIMG);
		cv::waitKey(0);
	}

	bool YSImageMerge::GetChopperTTLSignal(int sampleIndex, int totalSamples, bool chopperdoublefreq) {
		int samplesPerSegment =0;
		if (chopperdoublefreq == true) {
			samplesPerSegment = totalSamples / 4;
		}
		else if (chopperdoublefreq == false) {
			samplesPerSegment = totalSamples / 2;
		}

		double adjustedPhase = (-chopper_phase + 180.0) / 360.0;  
		int shiftedIndex = sampleIndex + int(adjustedPhase * totalSamples);
		if (shiftedIndex >= totalSamples) {
			shiftedIndex -= totalSamples;  
		}

		if (chopperdoublefreq == true) {
			if (shiftedIndex < samplesPerSegment) {
				return false; 
			}
			else if (shiftedIndex < 2 * samplesPerSegment) {
				return true; 
			}
			else if (shiftedIndex < 3 * samplesPerSegment) {
				return false;
			}
			else {
				return true;
			}
		}
		else if (chopperdoublefreq == false) {
			if (shiftedIndex < samplesPerSegment) {
				return true; 
			}
			else {
				return false;   
			}
		}

	}

	//Scan mode merge
	void YSImageMerge::LoadLissajousImage() {
		cv::Mat rawImg16 = cv::imread(LISSAJOUSPath, cv::IMREAD_UNCHANGED);
		if (rawImg16.empty() || rawImg16.type() != CV_16UC1) {
			cout << "Error: TIFF not found or not CV_16UC1 format: " << LISSAJOUSPath << endl;
			return;
		}

		num_of_sample = rawImg16.rows * rawImg16.cols;
		LissajousrawData = new double[num_of_sample];
		memset(LissajousrawData, 0, num_of_sample * sizeof(double));

		cv::Mat rawImg64;
		rawImg16.convertTo(rawImg64, CV_64FC1);

		memcpy(LissajousrawData, rawImg64.ptr<double>(), num_of_sample * sizeof(double));

		cout << "Data has been loaded" << endl;
	}

	void YSImageMerge::GenerateSampleMap() {
		sampleMap = new int[pixelnum * pixelnum];
		memset(sampleMap, 0, pixelnum * pixelnum * sizeof(int));

		for (int i = 0; i < num_of_sample; ++i) {
			double t = static_cast<double>(i) / samplingrate;
			int x = static_cast<int>(round((pixelnum / 2.0) * (1 + sin(2 * M_PI * xfreq * t))));
			int y = static_cast<int>(round((pixelnum / 2.0) * (1 - sin(2 * M_PI * yfreq * t + lissajous_phase))));

			if (x >= 0 && x < pixelnum && y >= 0 && y < pixelnum) {
				sampleMap[y * pixelnum + x]++;
			}
		}

		cout << "SampleMap is updated" << endl;
	}

	void YSImageMerge::DISPLAYRASTER() {
		cv::Mat image = cv::imread(RASTERPath, cv::IMREAD_COLOR);
		if (image.empty()) {
			cerr << "Error: Unable to load image at path: " << RASTERPath << endl;
		}
		cv::Mat rasterImage1024;
		cv::resize(image, rasterImage1024, cv::Size(1024, 1024), 0, 0, cv::INTER_NEAREST);

		cv::Mat bigRaster;
		cv::resize(rasterImage1024, bigRaster, cv::Size(), upscalefactorX, upscalefactorY, cv::INTER_NEAREST);

		string fullPath = LISSAJOUSFolderPath + "\\Image1.tiff";
		cv::Mat updateImage = cv::imread(fullPath, cv::IMREAD_UNCHANGED);
		if (updateImage.empty()) {
			cout << "Error: Failed to load image: " << fullPath << endl;
		}

		if (displaywithscan == true) {
			double phase_shift = 0;
			for (int i = 0; i < num_of_sample; ++i) {
				double t = static_cast<double>(i) / static_cast<double>(num_of_sample);
				int xPixel = int(round(updateImage.cols / 2) * (1 + sinf(2 * M_PI * xfreq * t)));
				int yPixel = int(round(updateImage.rows / 2) * (1 + sinf(2 * M_PI * yfreq * t)));
				if ((xPixel+offsetx) >= 0 && (xPixel + offsetx) < bigRaster.cols &&
					(yPixel - offsety) >= 0 && (yPixel - offsety) < bigRaster.rows) {
					bigRaster.at<cv::Vec3b>(yPixel-offsety, xPixel+offsetx) = cv::Vec3b(0, 0, 255);
				}
			}
			cv::Mat finalimg;
			cv::resize(bigRaster, finalimg, cv::Size(512, 512), 0, 0, cv::INTER_LINEAR);
			cv::imshow("2D Raster Image with Lissajous Scan", finalimg);
		}
		else if (displaywithscan ==false) {
			cv::Mat finalimg;
			cv::resize(bigRaster, finalimg, cv::Size(512, 512), 0, 0, cv::INTER_LINEAR);
			cv::imshow("2D Raster Image with Lissajous Scan", finalimg);
		}
	}

	void YSImageMerge::RLMERGEDISPLAY() {
		cv::Mat rasterImage =cv::imread(RASTERPath, cv::IMREAD_COLOR);
		if (rasterImage.empty()) {
			cerr << "Error: Unable to load image at path: " << RASTERPath << endl;
			return;
		}
		cv::Mat rasterImage1024;
		cv::resize(rasterImage, rasterImage1024, cv::Size(1024,1024), 0,0, cv::INTER_NEAREST);

		cv::Mat bigRaster;
		cv::resize(rasterImage1024, bigRaster, cv::Size(), upscalefactorX, upscalefactorY, cv::INTER_NEAREST);

		string lissajousFolderPathwithtiff = LISSAJOUSFolderPath + "\\*.tiff";
		// Store valid filenames
		vector<string> filenames;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(lissajousFolderPathwithtiff.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			cout << "Error: No .tiff files found in directory: " << lissajousFolderPathwithtiff << endl;
			return;
		}
		// Collect all valid filenames
		do {
			filenames.push_back(findFileData.cFileName);
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
		// Sort filenames based on the numeric part
		sort(filenames.begin(), filenames.end(), [this](const string& a, const string& b) {
			return extractNumber(a) < extractNumber(b);
			});

		int total_lissajousimage_count = filenames.size();
		rasterimgindex = 0;
		for (const auto& filename : filenames) {
			string fullPath = LISSAJOUSFolderPath + "\\" + filename;

			cv::Mat backgroundImage = bigRaster.clone();
			cv::Mat updateImage = cv::imread(fullPath, cv::IMREAD_COLOR);

			if (updateImage.empty()) {
				cout << "Error: Failed to load image: " << fullPath << endl;
				continue;
			}

			for (int i = 0; i < updateImage.rows; ++i) {
				for (int j = 0; j < updateImage.cols; ++j) {
					cv::Vec3b pixel = updateImage.at<cv::Vec3b>(i, j);
					if (!(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)) {
						if ((i + offsetx) >= 0 && (i + offsetx) < backgroundImage.rows &&
							(j - offsety) >= 0 && (j - offsety) < backgroundImage.cols) {
							backgroundImage.at<cv::Vec3b>((j - offsety), (i + offsetx)) = pixel;
						}
					}
				}
			}

			rasterimgindex++;
			if (savestate) {
				ostringstream savePath;
				savePath << "C:\\NTU_miniScope_Imaging\\MERGE\\Image" << rasterimgindex + 1 << ".tiff";
				if (!cv::imwrite(savePath.str(), backgroundImage)) {
					cout << "Error: Failed to save image: " << savePath.str() << endl;
				}

				progresspercentage_scanmodemerge = double(rasterimgindex) / double(total_lissajousimage_count) * 100;
			}

			cv::resize(backgroundImage, backgroundImage, cv::Size(512, 512), 0,0, cv::INTER_LINEAR);
			Addtext_RLMERGEDISPLAY(backgroundImage, rasterimgindex, total_lissajousimage_count);
			cv::imshow("2D Merge", backgroundImage);
			if (cv::waitKey(30) == 27) break; 

		}

	}

	//Extract kymograph
	void YSImageMerge::LoadLissajousAndMerge() {
		rasterImage_EK = cv::imread(RASTERPath_EK, cv::IMREAD_UNCHANGED);
		lissajousImage_EK = cv::imread(LISSAJOUSPath_EK, cv::IMREAD_UNCHANGED);
		cv::Mat displayImage;
		displayImage = rasterImage_EK.clone();

		scannedPixelList.clear();

		for (int i = 0; i < num_of_sample_EK; ++i) {
			double t = double(i) / samplingrate_EK;
			int x = int(round((pixelnum / 2.0) * (1 + sin(2 * M_PI * xfreq_EK * t))));
			int y = int(round((pixelnum / 2.0) * (1 - sin(2 * M_PI * yfreq_EK * t + lissajous_phase_EK))));
			if (x >= 0 && x < lissajousImage_EK.cols && y >= 0 && y < lissajousImage_EK.rows) {
				cv::Point pt(x, y);
				if (scannedPixelList.empty() || scannedPixelList.back() != pt)
					scannedPixelList.push_back(pt);
			}
		}
		for (const auto& pt : scannedPixelList) {
			displayImage.at<uint16_t>(pt) = lissajousImage_EK.at<uint16_t>(pt);
		}

		cv::Mat displayForShow;
		displayImage.convertTo(displayForShow, CV_8UC1, 1.0 / 257.0, 0.0);
		cv::imshow("Extract Kymograph", displayForShow);
	}

	void YSImageMerge::DrawLissajousPattern() {
		rasterImage_EK = cv::imread(RASTERPath_EK, cv::IMREAD_UNCHANGED);
		cv::Mat raster_8U;
		rasterImage_EK.convertTo(raster_8U, CV_8UC1, 1.0 / 257.0, 0.0);
		cv::Mat displayImage;
		cv::cvtColor(raster_8U, displayImage, cv::COLOR_GRAY2BGR);

		for (const auto& pt : scannedPixelList)
			cv::circle(displayImage, pt, 1, cv::Scalar(0, 0, 255), -1);

		int startPixelIndex = int(double(scannedPixelList.size()) * startPixelIndexpercentage);
		int endPixelIndex = int(double(scannedPixelList.size()) * endPixelIndexpercentage);
		cout << "percentage: " << startPixelIndexpercentage << " , " << endPixelIndexpercentage << endl;
		cout << "index: " << startPixelIndex << " , " << endPixelIndex << endl;
		for (int i = startPixelIndex; i <= endPixelIndex && i < scannedPixelList.size(); ++i)
			cv::circle(displayImage, scannedPixelList[i], 1, cv::Scalar(0, 255, 0), -1);
		cv::imshow("Extract Kymograph", displayImage);
	}

	void YSImageMerge::ExtractKymograph() {
		// Generate filename
		string outputFolder = "C:\\NTU_miniScope_Imaging\\KYMOGRAPH";
		auto now = chrono::system_clock::now();
		time_t now_c = chrono::system_clock::to_time_t(now);
		tm local_tm;
		localtime_s(&local_tm, &now_c);

		ostringstream oss;
		oss << "kymograph_"<< put_time(&local_tm, "%Y-%m-%d_%H%M")<< ".tiff";
		string filename = oss.str();
		string fullpath = outputFolder + "\\" + filename;

		// Store valid filenames
		string lissajousFolderPathwithtiff = LISSAJOUSFolderPath_EK + "\\*.tiff";
		vector<string> filenames;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(lissajousFolderPathwithtiff.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			cout << "Error: No .tiff files found in directory: " << lissajousFolderPathwithtiff << endl;
			return;
		}
		// Collect all valid filenames
		do {
			filenames.push_back(findFileData.cFileName);
		} while (FindNextFile(hFind, &findFileData) != 0);
		FindClose(hFind);
		// Sort filenames based on the numeric part
		sort(filenames.begin(), filenames.end(), [this](const string& a, const string& b) {
			return extractNumber(a) < extractNumber(b);
			});

		int total_lissajousimage_count = filenames.size();
		int startPixelIndex = int(double(scannedPixelList.size()) * startPixelIndexpercentage);
		int endPixelIndex = int(double(scannedPixelList.size()) * endPixelIndexpercentage);
		int pathLen = endPixelIndex - startPixelIndex + 1;
		if (pathLen <= 0 || scannedPixelList.empty()) {
			cout << "Error: Invalid path length or empty scannedPixelList" << endl;
			return;
		}
		cv::Mat kymograph(pathLen, total_lissajousimage_count, CV_16U);

		for (int f = 0; f < total_lissajousimage_count; ++f) {
			string fullPath = LISSAJOUSFolderPath_EK + "\\" + filenames[f];
			cv::Mat frame = cv::imread(fullPath, cv::IMREAD_UNCHANGED);
			if (frame.empty()) {
				cout << "Error: Failed to read " << fullPath << endl;
				continue;
			}
			for (int p = 0; p < pathLen; ++p) {
				int idx = startPixelIndex + p;
				if (idx >= scannedPixelList.size()) break;

				const cv::Point& pt = scannedPixelList[idx];
				if (pt.x >= 0 && pt.x < frame.cols && pt.y >= 0 && pt.y < frame.rows)
					kymograph.at<uint16_t>(p, f) = frame.at<uint16_t>(pt);
			}
		}

		cv::imwrite(fullpath, kymograph);
		cout << "Extract Kymograph : DONE" << endl;
	}


	//Load function
	void YSImageMerge::LOADspec(string path) {
		string csvFilePath = path + "\\scan_spec.csv";
		ifstream file(csvFilePath);
		if (!file.is_open()) {
			cout << "Error: Unable to open file" << endl;
		}

		string line;
		while (getline(file, line)) {
			istringstream iss(line);
			string key, value;
			getline(iss, key, ',');
			getline(iss, value);

			if (key == "xfreq") {
				try { xfreq = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting xfreq value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "yfreq") {
				try { yfreq = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting yfreq value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "phase") {
				try { lissajous_phase = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting phase value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "num of samples") {
				try { num_of_sample = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting num of samples value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "samplingrate") {
				try { samplingrate = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting samplingrate value: " << value << " (" << e.what() << ")" << endl;
				}
			}
		}
		file.close();
	}

	void YSImageMerge::LOADspec_EK(string path) {
		string csvFilePath = path + "\\scan_spec.csv";
		ifstream file(csvFilePath);
		if (!file.is_open()) {
			cout << "Error: Unable to open file" << endl;
		}

		string line;
		while (getline(file, line)) {
			istringstream iss(line);
			string key, value;
			getline(iss, key, ',');
			getline(iss, value);

			if (key == "xfreq") {
				try { xfreq_EK = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting xfreq value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "yfreq") {
				try { yfreq_EK = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting yfreq value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "phase") {
				try { lissajous_phase_EK = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting phase value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "num of samples") {
				try { num_of_sample_EK = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting num of samples value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "samplingrate") {
				try { samplingrate_EK = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting samplingrate value: " << value << " (" << e.what() << ")" << endl;
				}
			}
		}
		file.close();
	}

	//Transfer to .tiff
	void YSImageMerge::CopyScanSpecCSV(string& sourceFolder, string& targetFolder) {
		std::string sourceCSV = sourceFolder + "\\scan_spec.csv";
		std::string targetCSV = targetFolder + "\\scan_spec.csv";

		if (!CopyFileA(sourceCSV.c_str(), targetCSV.c_str(), FALSE)) {
			std::cerr << "Error: Unable to copy scan_spec.csv to " << targetCSV << std::endl;
		}
		else {
			std::cout << "Have copied scan_spec.csv to: " << targetCSV << std::endl;
		}
	}

	void YSImageMerge::ConvertBinToTiff() {
		string output_folder = LISSAJOUSFolderPath + "_TIFF";
		string search_path = LISSAJOUSFolderPath + "\\*.bin";
		WIN32_FIND_DATAA find_data;
		if (GetFileAttributesA(output_folder.c_str()) == INVALID_FILE_ATTRIBUTES) {
			if (!CreateDirectoryA(output_folder.c_str(), NULL)) {
				std::cerr << "Error: Unable to create output folder " << output_folder << std::endl;
				return;
			}
		}
		HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);

		if (hFind == INVALID_HANDLE_VALUE) {
			cerr << "Error: Unable to find any .bin in " << LISSAJOUSFolderPath << endl;
			return;
		}

		do {
			string bin_filename = find_data.cFileName;
			string bin_path = LISSAJOUSFolderPath + "\\" + bin_filename;
			string tiff_filename = bin_filename.substr(0, bin_filename.find_last_of(".")) + ".tiff";
			string tiff_path = output_folder + "\\" + tiff_filename;

			ifstream file(bin_path, std::ios::binary);
			if (!file) {
				std::cerr << "Error: Unable to read " << bin_path << std::endl;
				continue;
			}

			std::vector<U16> buffer(num_of_sample);
			file.read(reinterpret_cast<char*>(buffer.data()), num_of_sample * sizeof(U16));
			file.close();

			if (file.gcount() != static_cast<std::streamsize>(num_of_sample * sizeof(U16))) {
				std::cerr << "Error: The length of this read is mismatch " << bin_path << std::endl;
				continue;
			}

			cv::cuda::GpuMat gpu_image(1, num_of_sample, CV_16UC1);
			gpu_image.upload(cv::Mat(1, num_of_sample, CV_16UC1, buffer.data()));

			// INITIAL WINDOWING | Zero-negVolt | 32768-0
			cv::cuda::subtract(65535, gpu_image, gpu_image, cv::noArray(), -1, stream);
			cv::cuda::threshold(gpu_image, gpu_image, 65535, 0, cv::THRESH_TRUNC, stream);
			cv::cuda::subtract(gpu_image, 32768, gpu_image, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image, 2.0, gpu_image, 1.0, -1, stream);

			cv::Mat Img16bit;
			gpu_image.download(Img16bit);
			if (!cv::imwrite(tiff_path, Img16bit)) {
				std::cerr << "Error: Unable to save the image " << tiff_path << std::endl;
			}

		} while (FindNextFileA(hFind, &find_data));

		FindClose(hFind);
		CopyScanSpecCSV(LISSAJOUSFolderPath, output_folder);
	}

	//Reconstruct
	void YSImageMerge::DisplayReconstructedLissajousImage() {
		Image = new double[pixelnum * pixelnum];
		memset(Image, 0, pixelnum * pixelnum * sizeof(double));
		LissajoustempData = new double[num_of_sample];
		memset(LissajoustempData, 0, num_of_sample * sizeof(double));

		memcpy(LissajoustempData, LissajousrawData, num_of_sample * sizeof(double));
		if (offset_lissajous < num_of_sample) {
			for (int i = num_of_sample - 1; i >= offset_lissajous; --i)
				LissajoustempData[i] = LissajoustempData[i - offset_lissajous];
			for (int i = 0; i < offset_lissajous; ++i)
				LissajoustempData[i] = 0;
		}

		for (int i = 0; i < num_of_sample; ++i) {
			double t = double(i) / samplingrate;
			int x = int(round((pixelnum / 2.0) * (1 + sin(2 * M_PI * xfreq * t))));
			int y = int(round((pixelnum / 2.0) * (1 - sin(2 * M_PI * yfreq * t + lissajous_phase))));
			if (x >= 0 && x < pixelnum && y >= 0 && y < pixelnum) {
				int idx = y * pixelnum + x;
				if (sampleMap[idx] > 0)
					Image[idx] += LissajoustempData[i];
			}
		}
		for (int i = 0; i < pixelnum * pixelnum; ++i) {
			if (sampleMap[i] > 0)
				Image[i] /= sampleMap[i];
		}
		
		cv::Mat image64F(pixelnum, pixelnum, CV_64FC1, Image);
		cv::Mat image16U;
		image64F.convertTo(image16U, CV_16UC1, 1.0);

		cv::Mat image8U;
		image16U.convertTo(image8U, CV_8UC1, 1.0 / 257); 
		cv::imshow("Reconstruct Image", image8U);
		cv::waitKey(0);
		delete[] LissajoustempData;
		delete[] Image;
	}

	void YSImageMerge::ReconstructAllTiffInFolder() {
		// Store valid filenames
		string lissajousFolderPathwithtiff = LISSAJOUSFolderPath + "\\*.tiff";
		vector<string> filenames;
		WIN32_FIND_DATA findFileData;
		HANDLE hFind = FindFirstFile(lissajousFolderPathwithtiff.c_str(), &findFileData);
		if (hFind == INVALID_HANDLE_VALUE) {
			cout << "Error: No .tiff files found in directory " << lissajousFolderPathwithtiff << endl;
			return;
		}
		// Collect all valid filenames
		do {
			filenames.push_back(findFileData.cFileName);
		} while (FindNextFile(hFind, &findFileData) != 0);

		FindClose(hFind);

		string outputFolderPath = LISSAJOUSFolderPath;
		size_t pos = outputFolderPath.find("2DLISSAJOUS_RAW");
		if (pos != string::npos) {
			outputFolderPath.replace(pos, strlen("2DLISSAJOUS_RAW"), "2DLISSAJOUS");
		}
		CreateDirectory(outputFolderPath.c_str(), NULL);

		// Sort filenames based on the numeric part
		sort(filenames.begin(), filenames.end(), [this](const string& a, const string& b) {
			return extractNumber(a) < extractNumber(b);
			});

		int total_lissajousimage_count = filenames.size();
		lissajousimgindex = 0;
		for (const auto& filename : filenames) {
			string fullPath = LISSAJOUSFolderPath + "\\" + filename;
			cv::Mat updateImage = cv::imread(fullPath, cv::IMREAD_UNCHANGED);

			if (updateImage.empty() || updateImage.type() != CV_16UC1) {
				cout << "Error: TIFF not found or not CV_16UC1 format " << fullPath << endl;
				continue;
			}

			num_of_sample = updateImage.rows * updateImage.cols;
			LissajoustempData = new double[num_of_sample];
			Image = new double[pixelnum * pixelnum];
			memset(LissajoustempData, 0, num_of_sample * sizeof(double));
			memset(Image, 0, pixelnum * pixelnum * sizeof(double));

			cv::Mat updateImage64;
			updateImage.convertTo(updateImage64, CV_64FC1);
			memcpy(LissajoustempData, updateImage64.ptr<double>(), num_of_sample * sizeof(double));

			if (offset_lissajous < num_of_sample) {
				for (int i = num_of_sample - 1; i >= offset_lissajous; --i)
					LissajoustempData[i] = LissajoustempData[i - offset_lissajous];
				for (int i = 0; i < offset_lissajous; ++i)
					LissajoustempData[i] = 0;
			}

			for (int i = 0; i < num_of_sample; ++i) {
				double t = double(i) / samplingrate;
				int x = int(round((pixelnum / 2.0) * (1 + sin(2 * M_PI * xfreq * t))));
				int y = int(round((pixelnum / 2.0) * (1 - sin(2 * M_PI * yfreq * t + lissajous_phase))));
				if (x >= 0 && x < pixelnum && y >= 0 && y < pixelnum) {
					int idx = y * pixelnum + x;
					if (sampleMap[idx] > 0)
						Image[idx] += LissajoustempData[i];
				}
			}
			for (int i = 0; i < pixelnum * pixelnum; ++i) {
				if (sampleMap[i] > 0)
					Image[i] /= sampleMap[i];
			}

			cv::Mat image64F(pixelnum, pixelnum, CV_64FC1, Image);
			cv::Mat image16U;
			image64F.convertTo(image16U, CV_16UC1, 1.0);

			ostringstream savePath;
			savePath << outputFolderPath << "\\Image" << (lissajousimgindex + 1) << ".tiff";
			if (!cv::imwrite(savePath.str(), image16U)) {
				cerr << "Error: Failed to save image " << savePath.str() << endl;
			}

			lissajousimgindex++;
			progresspercentage = double(lissajousimgindex) / double(total_lissajousimage_count) * 100;

			cv::Mat image8U;
			image16U.convertTo(image8U, CV_8UC1, 1.0 / 257);
			//cv::imshow("Reconstruct Image folder", image8U);
			//cv::waitKey(0);
		}
		CopyScanSpecCSV(LISSAJOUSFolderPath, outputFolderPath);
		delete[] LissajoustempData;
		delete[] Image;
	}

	//Supplementart functions
	int YSImageMerge::extractNumber(const string& filename) {
		size_t pos = filename.find_first_of("0123456789");
		if (pos == string::npos) return -1;

		int number = 0;
		while (pos < filename.size() && isdigit(filename[pos])) {
			number = number * 10 + (filename[pos] - '0');
			++pos;
		}
		return number;
	}

	void YSImageMerge::Addtext_RLMERGEDISPLAY(cv::Mat& IMGDP16, int frameorder, int totalframe) {
		int fontFace = cv::FONT_HERSHEY_SIMPLEX;
		int thickness = 1;
		int baseline = 0;
		double fontScale = 0.6;
		ostringstream oss;
		oss << "ScanModeNerge: "<<frameorder<<" / " << totalframe;
		string text = oss.str();
		cv::Size textSize = cv::getTextSize(text, fontFace, fontScale, thickness, &baseline);
		cv::Point textOrg(IMGDP16.cols - textSize.width - 10, IMGDP16.rows - 10);
		cv::putText(IMGDP16, text, textOrg, fontFace, fontScale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);
	}

	void YSImageMerge::AVERAGE(string folderPath, cv::Mat& avgImage) {
		vector<cv::String> fileNames;
		cv::glob(folderPath + "/*.tiff", fileNames, false);
		int count = 0;
		cv::Mat sumImage;
		cv::Size imageSize;

		for (const auto& fileName : fileNames) {
			cv::Mat image = cv::imread(fileName, cv::IMREAD_UNCHANGED);
			if (image.empty()) {
				cerr << "Error: Unable to load image at path: " << fileName << endl;
				continue;
			}
			if (count == 0) {
				imageSize = image.size();
				sumImage = cv::Mat::zeros(imageSize, CV_64FC3);
			}
			if (image.size() != imageSize) {
				cv::resize(image, image, imageSize, 0, 0, cv::INTER_LINEAR);
			}

			sumImage += image;
			count++;
		}
		if (count == 0) {
			cerr << "Error: No images found in folder " << folderPath << endl;
		}
		sumImage /= count;
		sumImage.convertTo(avgImage, CV_8UC3);
		cv::resize(avgImage, avgImage, cv::Size(), 256 / avgImage.size[0], 256 / avgImage.size[1], cv::INTER_LINEAR);
	}
}