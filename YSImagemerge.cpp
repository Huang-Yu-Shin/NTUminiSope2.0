#include "YSImageMerge.h"

namespace NTU {

	YSImageMerge::YSImageMerge(NTU::CudaCalPara* cudacalpara, NTU::ReconstructPost* cudareconstruct) {
		calpara = cudacalpara;
		reconstruct = cudareconstruct;
	}

	YSImageMerge::~YSImageMerge() {
		if (LissajousrawData != nullptr) {
			delete[] LissajousrawData;
		}
		if (LissajousrawData_ch2 != nullptr) {
			delete[] LissajoustempData_ch2;
		}
		if (LissajoustempData != nullptr) {
			delete[] LissajoustempData;
		}
		if (LissajoustempData_ch2 != nullptr) {
			delete[] LissajoustempData_ch2;
		}
		if (Image != nullptr) {
			delete[] Image;
		}
		if (Image_ch2 != nullptr) {
			delete[] Image_ch2;
		}
		if (sampleMap != nullptr) {
			delete[] sampleMap;
		}
		if (sampleMap_f64 != nullptr) {
			delete[] sampleMap;
		}
		calpara->~CudaCalPara();
		cv::destroyAllWindows();
	}

	//SampleMap
	void YSImageMerge::Generate2D_RASTER_SampleMap() {
		U16 samplesPerRecord = U16(num_of_sample / num_of_line);
		sin_weight.create(num_of_line, 1024, CV_32FC1);
		sin_weight.setTo(cv::Scalar(0));
		for (int line = 0; line < num_of_line; line++) {
			bool is_forward = (line % 2 == 0);

			for (int sample = 0; sample < int(samplesPerRecord); sample++) {
				float t = float(sample) / float(samplesPerRecord - 1);  // normalized sample [0,1]
				float angle = is_forward
					? (CV_PI * t - 0.5f * CV_PI)            // -?/2 to +?/2
					: (CV_PI * (1.0f - t) - 0.5f * CV_PI);  // +?/2 to -?/2
				float x = 0.5f * (1.0f + sinf(angle));      // mapped to [0,1]
				int x_idx = int(x * (1024 - 1));

				if (x_idx >= 0 && x_idx < 1024) {
					sin_weight.at<float>(line, x_idx) += 1.0f;
				}
			}
		}
	}

	void YSImageMerge::Generate2D_LISSAJOUS_SampleMap() {
		sampleMap = new int[pixelnum * pixelnum];
		memset(sampleMap, 0, pixelnum * pixelnum * sizeof(int));
		sampleMap_f64 = new float64[pixelnum * pixelnum];
		memset(sampleMap_f64, 0, pixelnum * pixelnum * sizeof(float64));

		for (int i = 0; i < num_of_sample; ++i) {
			int x = 0;
			int y = 0;
			bool inrange = LissajousIdxFromSample(i, samplingrate, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, x, y);
			if (inrange == true) {
				sampleMap[y * pixelnum + x]++;
				sampleMap_f64[y * pixelnum + x]++;
			}
		}
		reconstruct->pixelnumX = pixelnum;
		reconstruct->pixelnumY = pixelnum;
		reconstruct->sampling_rate = samplingrate;
		reconstruct->freqx = xfreq;
		reconstruct->freqy = yfreq;
		reconstruct->phase = lissajousphase;
		reconstruct->total_sample = num_of_sample;
		reconstruct->CudaMemoryAllocate();
		reconstruct->CudaSampleMap(sampleMap_f64);
	}

	//Process image
	void YSImageMerge::displayprocessImage(string& input_path) {
		string singleImgPath = input_path + "\\Image1.tiff";
		cv::Mat img = cv::imread(singleImgPath, cv::IMREAD_UNCHANGED);
		if (img.empty()) {
			cerr << "Failed to load: " << singleImgPath << endl;
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
		gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, 5, 150 - 150, stream);
		stream.waitForCompletion();

		cv::cuda::GpuMat gpu_8bit_resize;
		cv::cuda::resize(gpu_8bit, gpu_8bit_resize,cv::Size(512,512));

		cv::Mat result;
		gpu_8bit_resize.download(result, stream);
		stream.waitForCompletion();

		cv::imshow("ProcessImg", result);
	}

	void YSImageMerge::processImage(string& input_path) {
		string output_folder = input_path + "\\process";
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
			gpu_8bit.cv::cuda::GpuMat::convertTo(gpu_8bit, -1, 5, 150 - 150, stream);
			stream.waitForCompletion();

			cv::Mat result;
			gpu_8bit.download(result, stream);
			stream.waitForCompletion();
			std::string output_path = output_folder + "\\" + filename;
			cv::imwrite(output_path, result);
		}

		std::cout << "All TIFFs processed." << std::endl;
	}

	//Calculate Parameter//
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
		ypixelnum = image.rows;   
		xpixelnum = image.cols;
		loaddata = true;

		cv::Mat raster_8U;
		image.convertTo(raster_8U, CV_8UC1, 1.0 / 257.0, 0.0);
		cv::Mat displayimage;
		cv::cvtColor(raster_8U, displayimage, cv::COLOR_GRAY2BGR);
		rasterImage = displayimage.clone();
	}

	void YSImageMerge::ScanmodePreview() {
		cv::Mat DisplayIMG = rasterImage.clone();
		double duration = 1.0 / double(framerate_SP);
		if (framerate_SP >= 120) {
			chopper_freq_upscale = 1;
		}
		else {
			chopper_freq_upscale = 4;
		}

		int totalSamples = int(samplingrate_SP * duration);
		totalSamples = totalSamples - (totalSamples % 128);
		int xpixelnum_lissajous = int(double(xpixelnum) * scansizeratio);
		int ypixelnum_lissajous = int(double(ypixelnum) * scansizeratio);
		for (int i = 0; i < totalSamples; ++i) {
			int xPixel = 0;
			int yPixel = 0;
			bool inrange = LissajousIdxFromSample(i, samplingrate_SP, xpixelnum_lissajous, ypixelnum_lissajous, double(xfreq_SP), double(yfreq_SP), phase_SP, xPixel, yPixel);
			xPixel = xPixel + (xpixelnum - xpixelnum_lissajous)/2 + offsetx_SP;
			yPixel = yPixel + (ypixelnum - ypixelnum_lissajous)/2 + offsety_SP;
			if (inrange == true) {
				if (xPixel >= 0 && xPixel < xpixelnum && yPixel >= 0 && yPixel < ypixelnum) {
					if (apply_chopper == true) {
						if (GetChopperTTLSignal(i, totalSamples)) {
							DisplayIMG.at<cv::Vec3b>(yPixel, xPixel) = cv::Vec3b(0, 0, 255);
						}
						else {
							DisplayIMG.at<cv::Vec3b>(yPixel, xPixel) = cv::Vec3b(0, 50, 0);
						}
					}
					else {
						DisplayIMG.at<cv::Vec3b>(yPixel, xPixel) = cv::Vec3b(0, 0, 255);
					}
				}
			}
		}

		cv::Mat DisplayIMG_resize;
		cv::resize(DisplayIMG, DisplayIMG_resize, cv::Size(512, 512), 0, 0, cv::INTER_NEAREST);
		cv::imshow("ScanPreview", DisplayIMG_resize);
		cv::waitKey(0);
	}

	bool YSImageMerge::GetChopperTTLSignal(int sampleIndex, int totalSamples) {
		if (chopper_freq_upscale == 1) {
			//int segments = 2;
			//int samplesPerSegment = totalSamples / segments;
			//if (samplesPerSegment <= 0) return true;

			//double ph = std::fmod(chopper_phase, 360.0);
			//if (ph < 0.0) ph += 360.0;
			//int shiftSamples = static_cast<int>(std::round(ph / 360.0 * totalSamples));

			//int shiftedIndex = (sampleIndex - shiftSamples) % totalSamples; 
			//if (shiftedIndex < 0) shiftedIndex += totalSamples;       

			//const int quarterSamples = totalSamples / 8;

			//int quarter = shiftedIndex / quarterSamples; 
			//bool isHigh = (quarter >=1) && (quarter<=2);

			//return isHigh;


			int segments = 2;
			int samplesPerSegment = totalSamples / segments;
			if (samplesPerSegment <= 0) return true;
			double chopperphase = chopper_phase - 55;

			double ph = std::fmod(chopperphase, 360.0);
			if (ph < 0.0) ph += 360.0;
			int shiftSamples = static_cast<int>(std::round(ph / 360.0 * totalSamples));

			int shiftedIndex = (sampleIndex - shiftSamples) % totalSamples;
			if (shiftedIndex < 0) shiftedIndex += totalSamples;

			const int quarterSamples = totalSamples / 4;
			int quarter = shiftedIndex / quarterSamples;
			bool isHigh = (quarter < 2);
			return isHigh;
		}
		else if (chopper_freq_upscale == 4) {
			//int segments = 8;                            
			//int samplesPerSegment = totalSamples / segments;
			//int samplesPerSegment_calibration = totalSamples / (segments*4);
			//if (samplesPerSegment <= 0) return true;

			//double ph = std::fmod(chopper_phase, 360.0);
			//if (ph < 0.0) ph += 360.0;

			//int samplesPerChopper = totalSamples / 4;    
			//int shiftSamples = static_cast<int>(std::round(ph / 360.0 * samplesPerChopper));

			//int shiftedIndex = (sampleIndex - shiftSamples) % totalSamples;
			//if (shiftedIndex < 0) shiftedIndex += totalSamples;

			//int segIdx = shiftedIndex / samplesPerSegment;
			//int segIdx_calibration = shiftedIndex / samplesPerSegment_calibration;
			//bool isHigh = ((segIdx % 2) == 0) && (((segIdx_calibration % 4) == 1)|| ((segIdx_calibration % 4) == 2));
			//return isHigh;

			int segments = 8;
			int samplesPerSegment = totalSamples / segments;
			if (samplesPerSegment <= 0) return true;
			double chopperphase = chopper_phase - 55;

			double ph = std::fmod(chopperphase, 360.0);
			if (ph < 0.0) ph += 360.0;

			int samplesPerChopper = totalSamples / 4;
			int shiftSamples = static_cast<int>(std::round(ph / 360.0 * samplesPerChopper));

			int shiftedIndex = (sampleIndex - shiftSamples) % totalSamples;
			if (shiftedIndex < 0) shiftedIndex += totalSamples;

			int segIdx = shiftedIndex / samplesPerSegment;
			bool isHigh = (segIdx % 2) == 0;
			return isHigh;

		}

	}

	//Scan mode merge//
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
				int xPixel = 0;
				int yPixel = 0;
				bool inrange = LissajousIdxFromSample(i, num_of_sample, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, xPixel, yPixel);
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

				progresspercentage = double(rasterimgindex) / double(total_lissajousimage_count) * 100;
			}

			cv::resize(backgroundImage, backgroundImage, cv::Size(512, 512), 0,0, cv::INTER_LINEAR);
			AddtextOnImg(backgroundImage, rasterimgindex, total_lissajousimage_count);
			cv::imshow("2D Merge", backgroundImage);
			if (cv::waitKey(30) == 27) break; 

		}

	}

	//Extract kymograph//
	void YSImageMerge::LoadLissajousAndMerge() {
		rasterImage_EK = cv::imread(RASTERPath, cv::IMREAD_UNCHANGED);
		lissajousImage_EK = cv::imread(LISSAJOUSPath, cv::IMREAD_UNCHANGED);
		cv::Mat displayImage;
		displayImage = rasterImage_EK.clone();

		scannedPixelList.clear();

		for (int i = 0; i < num_of_sample; ++i) {
			int x = 0;
			int y = 0;
			bool inrange = LissajousIdxFromSample(i, samplingrate, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, x, y);
			if (inrange == true) {
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
		rasterImage_EK = cv::imread(RASTERPath, cv::IMREAD_UNCHANGED);
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
		string lissajousFolderPathwithtiff = LISSAJOUSFolderPath + "\\*.tiff";
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
			string fullPath = LISSAJOUSFolderPath + "\\" + filenames[f];
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

	//Transfer to .tiff
	void YSImageMerge::ConvertBinToTiff_lisssajous_parallel() {
		if (channelcount == 1) {
			std::string output_folder = LISSAJOUSFolderPath + "_TIFF";
			if (GetFileAttributesA(output_folder.c_str()) == INVALID_FILE_ATTRIBUTES) {
				if (!CreateDirectoryA(output_folder.c_str(), NULL)) {
					std::cerr << "Error: Unable to create output folder " << output_folder << std::endl;
					return;
				}
			}
			CopyScanSpecCSV(LISSAJOUSFolderPath, output_folder);

			std::vector<std::string> bin_files;
			string search_path = LISSAJOUSFolderPath + "\\*.bin";
			WIN32_FIND_DATAA find_data;
			
			HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
			if (hFind == INVALID_HANDLE_VALUE) {
				cerr << "Error: Unable to find any .bin in " << LISSAJOUSFolderPath << endl;
				return;
			}

			do {
				if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					bin_files.push_back(find_data.cFileName);
				}
			} while (FindNextFileA(hFind, &find_data));
			FindClose(hFind);

			vector<future<void>> futures;
			futures.reserve(bin_files.size()); 
			for (auto& b : bin_files) {
				string bin_filename = b;
				string bin_path = LISSAJOUSFolderPath + "\\" + bin_filename;
				string tiff_filename = bin_filename.substr(0, bin_filename.find_last_of(".")) + ".tiff";
				string tiff_path = output_folder + "\\" + tiff_filename;

				futures.push_back(async(launch::async,
					[this, bin_path, tiff_path]() {
						this->ProcessSingleBin_lissajous_1ch(bin_path, tiff_path);
					}
				));
			}
			for (auto& f : futures) f.get();
		}
		else if (channelcount == 2) {
			size_t pos1 = LISSAJOUSFolderPath.find_last_of("\\/");
			string tempfoldername = LISSAJOUSFolderPath.substr(pos1 + 1);
			string folderrealname;
			int underscores = 0;
			for (size_t i = 0; i < tempfoldername.size(); ++i) {
				if (tempfoldername[i] == '_') {
					++underscores;
					if (underscores == 3) {
						folderrealname = tempfoldername.substr(i + 1);
					}
				}
			}

			string output_folder = LISSAJOUSFolderPath + "_TIFF";
			string output_folder_ch1 = output_folder + "\\" + folderrealname + "_Ch1";
			string output_folder_ch2 = output_folder + "\\" + folderrealname + "_Ch2";
			CreateDirectoryA(output_folder.c_str(), NULL);
			CreateDirectoryA(output_folder_ch1.c_str(), NULL);
			CreateDirectoryA(output_folder_ch2.c_str(), NULL);
			CopyScanSpecCSV(LISSAJOUSFolderPath, output_folder);

			vector<std::string> bin_files;

			string search_path = LISSAJOUSFolderPath + "\\*.bin";
			WIN32_FIND_DATAA find_data;
			HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
			if (hFind == INVALID_HANDLE_VALUE) {
				std::cerr << "Error: Unable to find any .bin in " << LISSAJOUSFolderPath << std::endl;
				return;
			}

			do {
				if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					bin_files.push_back(find_data.cFileName);
				}
			} while (FindNextFileA(hFind, &find_data));
			FindClose(hFind);

			vector<future<void>> futures;
			futures.reserve(bin_files.size());
			for (auto& b : bin_files) {
				std::string bin_filename = b;
				std::string bin_path = LISSAJOUSFolderPath + "\\" + bin_filename;
				std::string tiff_filename = bin_filename.substr(0, bin_filename.find_last_of('.')) + ".tiff";
				std::string tiff_ch1_path = output_folder_ch1 + "\\" + tiff_filename;
				std::string tiff_ch2_path = output_folder_ch2 + "\\" + tiff_filename;

				futures.push_back(std::async(std::launch::async,
					[this, bin_path, tiff_ch1_path, tiff_ch2_path]() {
						this->ProcessSingleBin_lissajous_2chs(bin_path, tiff_ch1_path, tiff_ch2_path);
					}
				));
			}
			for (auto& f : futures) f.get();

		}
	}

	void YSImageMerge::ProcessSingleBin_lissajous_1ch(const string& bin_path, const string& tiff_path) {
		ifstream file(bin_path, ios::binary);
		if (!file) {
			cout << "Error: Unable to read " << bin_path << endl;
			return;
		}

		std::vector<U16> buffer(num_of_sample);
		file.read(reinterpret_cast<char*>(buffer.data()), num_of_sample * sizeof(U16));
		file.close();

		if (file.gcount() != static_cast<std::streamsize>(num_of_sample * sizeof(U16))) {
			cout << "Error: The length of this read is mismatch " << bin_path << endl;
			return;
		}

		cv::cuda::Stream stream_local;
		cv::cuda::GpuMat gpu_image(1, num_of_sample, CV_16UC1);
		gpu_image.upload(cv::Mat(1, num_of_sample, CV_16UC1, buffer.data()),stream_local);
		stream_local.waitForCompletion();

		cv::cuda::subtract(65535, gpu_image, gpu_image, cv::noArray(), -1, stream_local);
		cv::cuda::threshold(gpu_image, gpu_image, 65535, 0, cv::THRESH_TRUNC, stream_local);
		cv::cuda::subtract(gpu_image, 32768, gpu_image, cv::noArray(), -1, stream_local);
		cv::cuda::multiply(gpu_image, 2.0, gpu_image, 1.0, -1, stream_local);
		stream_local.waitForCompletion();

		cv::Mat Img16bit;
		gpu_image.download(Img16bit, stream_local);
		stream_local.waitForCompletion();
		if (!cv::imwrite(tiff_path, Img16bit)) {
			cout << "Error: Unable to save the image " << tiff_path << endl;
		}
	}

	void YSImageMerge::ProcessSingleBin_lissajous_2chs(const string& bin_path, const string& tiff_ch1_path, const string& tiff_ch2_path) {
		ifstream file(bin_path, ios::binary);
		if (!file) { 
			cout << "Error: Unable to read " << bin_path << endl; 
			return;
		}

		const size_t total_u16 = size_t(num_of_sample) * channelcount; // CH1+CH2
		vector<U16> buffer(total_u16);
		file.read(reinterpret_cast<char*>(buffer.data()),static_cast<streamsize>(total_u16 * sizeof(U16)));
		streamsize got = file.gcount();
		file.close();

		if (got != static_cast<streamsize>(total_u16 * sizeof(U16))) {
			cout<< "Error: The length of this read is mismatch " << bin_path << endl;
			return;
		}

		U16* ch1_ptr = buffer.data();
		U16* ch2_ptr = buffer.data() + num_of_sample;
		cv::cuda::Stream stream_local;
		cv::cuda::GpuMat g_ch1(1, num_of_sample, CV_16UC1);
		cv::cuda::GpuMat g_ch2(1, num_of_sample, CV_16UC1);
		g_ch1.upload(cv::Mat(1, num_of_sample, CV_16UC1, ch1_ptr),stream_local);
		g_ch2.upload(cv::Mat(1, num_of_sample, CV_16UC1, ch2_ptr), stream_local);
		stream_local.waitForCompletion();

		cv::cuda::subtract(65535, g_ch1, g_ch1, cv::noArray(), -1, stream_local);
		cv::cuda::threshold(g_ch1, g_ch1, 65535, 0, cv::THRESH_TRUNC, stream_local);
		cv::cuda::subtract(g_ch1, 32768, g_ch1, cv::noArray(), -1, stream_local);
		cv::cuda::multiply(g_ch1, 2.0, g_ch1, 1.0, -1, stream_local);
		cv::cuda::subtract(g_ch2, 32768, g_ch2, cv::noArray(), -1, stream_local);
		cv::cuda::multiply(g_ch2, 2.0, g_ch2, 1.0, -1, stream_local);
		stream_local.waitForCompletion();

		cv::Mat Img16bit_ch1, Img16bit_ch2;
		g_ch1.download(Img16bit_ch1, stream_local);
		g_ch2.download(Img16bit_ch2, stream_local);
		stream_local.waitForCompletion();

		if (!cv::imwrite(tiff_ch1_path, Img16bit_ch1))
			cout << "Error: Unable to save the image " << tiff_ch1_path << endl;
		if (!cv::imwrite(tiff_ch2_path, Img16bit_ch2))
			cout << "Error: Unable to save the image " << tiff_ch2_path << endl;
	}

	void YSImageMerge::ConvertBinToTiff_lissajous() {
		if (channelcount == 1) {
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
		else if (channelcount == 2) {
			size_t pos1 = LISSAJOUSFolderPath.find_last_of("\\/");
			string tempfoldername = LISSAJOUSFolderPath.substr(pos1 + 1);
			string folderrealname;
			int underscores = 0;
			for (size_t i = 0; i < tempfoldername.size(); ++i) {
				if (tempfoldername[i] == '_') {
					++underscores;
					if (underscores == 3) {
						folderrealname = tempfoldername.substr(i + 1);
					}
				}
			}

			string output_folder = LISSAJOUSFolderPath + "_TIFF";
			string output_folder_ch1 = output_folder + "\\"+ folderrealname+"_Ch1";
			string output_folder_ch2 = output_folder + "\\"+ folderrealname+"_Ch2";
			CreateDirectoryA(output_folder.c_str(), NULL);
			CreateDirectoryA(output_folder_ch1.c_str(), NULL);
			CreateDirectoryA(output_folder_ch2.c_str(), NULL);

			CopyScanSpecCSV(LISSAJOUSFolderPath, output_folder);

			std::string search_path = LISSAJOUSFolderPath + "\\*.bin";
			WIN32_FIND_DATAA find_data;
			HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
			if (hFind == INVALID_HANDLE_VALUE) {
				std::cerr << "Error: Unable to find any .bin in " << LISSAJOUSFolderPath << std::endl;
				return;
			}

			do {
				if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

				std::string bin_filename = find_data.cFileName;
				std::string bin_path = LISSAJOUSFolderPath + "\\" + bin_filename;
				std::string tiff_filename = bin_filename.substr(0, bin_filename.find_last_of('.')) + ".tiff";
				std::string tiff_ch1_path = output_folder_ch1 + "\\" + tiff_filename;
				std::string tiff_ch2_path = output_folder_ch2 + "\\" + tiff_filename;

				std::ifstream file(bin_path, std::ios::binary);
				if (!file) { std::cerr << "Error: Unable to read " << bin_path << std::endl; continue; }

				const size_t total_u16 = size_t(num_of_sample) * channelcount; // CH1+CH2
				std::vector<U16> buffer(total_u16);
				file.read(reinterpret_cast<char*>(buffer.data()),
					static_cast<std::streamsize>(total_u16 * sizeof(U16)));
				std::streamsize got = file.gcount();
				file.close();

				if (got != static_cast<std::streamsize>(total_u16 * sizeof(U16))) {
					std::cerr << "Error: The length of this read is mismatch " << bin_path << std::endl;
					continue;
				}

				U16* ch1_ptr = buffer.data();
				U16* ch2_ptr = buffer.data() + num_of_sample;
				cv::cuda::GpuMat g_ch1(1, num_of_sample, CV_16UC1);
				cv::cuda::GpuMat g_ch2(1, num_of_sample, CV_16UC1);
				g_ch1.upload(cv::Mat(1, num_of_sample, CV_16UC1, ch1_ptr));
				g_ch2.upload(cv::Mat(1, num_of_sample, CV_16UC1, ch2_ptr));

				cv::cuda::subtract(65535, g_ch1, g_ch1, cv::noArray(), -1, stream);
				cv::cuda::threshold(g_ch1, g_ch1, 65535, 0, cv::THRESH_TRUNC, stream);
				cv::cuda::subtract(g_ch1, 32768, g_ch1, cv::noArray(), -1, stream);
				cv::cuda::multiply(g_ch1, 2.0, g_ch1, 1.0, -1, stream);
				cv::cuda::subtract(g_ch2, 32768, g_ch2, cv::noArray(), -1, stream);
				cv::cuda::multiply(g_ch2, 2.0, g_ch2, 1.0, -1, stream);

				cv::Mat Img16bit_ch1, Img16bit_ch2;
				g_ch1.download(Img16bit_ch1);
				g_ch2.download(Img16bit_ch2);

				if (!cv::imwrite(tiff_ch1_path, Img16bit_ch1))
					std::cerr << "Error: Unable to save the image " << tiff_ch1_path << std::endl;
				if (!cv::imwrite(tiff_ch2_path, Img16bit_ch2))
					std::cerr << "Error: Unable to save the image " << tiff_ch2_path << std::endl;

			} while (FindNextFileA(hFind, &find_data));

			FindClose(hFind);
		}
	}
	
	void YSImageMerge::ConvertBinToTiff_raster_1ch() {
		Generate2D_RASTER_SampleMap();

		size_t pos = RASTERFolderPath.find_last_of("\\/");
		std::string folderName = (pos == std::string::npos) ? RASTERFolderPath : RASTERFolderPath.substr(pos + 1);

		std::string output_folder = "C:\\NTU_miniScope_Imaging\\2DRASTER\\" + folderName;
		std::string output_folder_raw = RASTERFolderPath + "_TIFF";

		CreateDirectoryA(output_folder.c_str(), NULL);
		CreateDirectoryA(output_folder_raw.c_str(), NULL);

		CopyScanSpecCSV(RASTERFolderPath, output_folder);
		CopyScanSpecCSV(RASTERFolderPath, output_folder_raw);

		std::string search_path = RASTERFolderPath + "\\*.bin";
		WIN32_FIND_DATAA find_data;
		HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
		if (hFind == INVALID_HANDLE_VALUE) {
			std::cerr << "Error: Unable to find any .bin in " << RASTERFolderPath << std::endl;
			return;
		}

		do {
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

			std::string bin_filename = find_data.cFileName;
			std::string bin_path = RASTERFolderPath + "\\" + bin_filename;

			std::string output_filename = bin_filename.substr(0, bin_filename.find_last_of(".")) + ".tiff";
			std::string raw_ch1_path = output_folder_raw + "\\" + output_filename;
			std::string ch1_path = output_folder + "\\" + output_filename;

			std::ifstream file(bin_path, std::ios::binary);
			if (!file) {
				std::cerr << "Error: Unable to read " << bin_path << std::endl;
				continue;
			}

			std::vector<U16> buffer(static_cast<size_t>(num_of_sample));
			file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(num_of_sample * sizeof(U16)));
			std::streamsize got = file.gcount();
			file.close();
			if (got != static_cast<std::streamsize>(num_of_sample * sizeof(U16))) {
				std::cerr << "Error: The length of this read is mismatch " << bin_path << std::endl;
				continue;
			}

			U16 samplesPerRecord = U16(num_of_sample / num_of_line);
			std::vector<U16> h_image_1(static_cast<size_t>(num_of_sample), 0);


			for (int line = 0; line < num_of_line; ++line) {
				U16* src = buffer.data() + line * samplesPerRecord;
				U16* dst = h_image_1.data() + line * samplesPerRecord;

				if (line % 2 == 0) {
					memcpy(dst, src, sizeof(U16) * int(samplesPerRecord));
				}
				else {
					for (int s = 0; s < int(samplesPerRecord); ++s) {
						int dstIdx = int(samplesPerRecord) - 1 - s;
						dst[dstIdx] = src[s];
					}
				}
			}

			cv::Mat sin_image_1(num_of_line, 1024, CV_32FC1, cv::Scalar(0));
			for (int line = 0; line < num_of_line; ++line) {
				bool is_forward = (line % 2 == 0);
				U16* src = buffer.data() + line * samplesPerRecord;
				int srcIdx = 0;

				for (int sample = 0; sample < int(samplesPerRecord); ++sample) {
					float t = float(sample) / float(std::max(1, int(samplesPerRecord) - 1));
					float angle = is_forward ? (CV_PI * t - 0.5f * CV_PI)
						: (CV_PI * (1.0f - t) - 0.5f * CV_PI);
					float x = 0.5f * (1.0f + std::sinf(angle));
					int x_idx = int(x * (1024 - 1));
					if (x_idx < 0 || x_idx >= 1024) continue;
					if (srcIdx < int(samplesPerRecord)) sin_image_1.at<float>(line, x_idx) += float(src[srcIdx++]);
				}
			}

			cv::Mat sin_image_u16_1(num_of_line, 1024, CV_16UC1);
			for (int y = 0; y < num_of_line; ++y) {
				for (int x = 0; x < 1024; ++x) {
					float weight = sin_weight.at<float>(y, x);
					float v1 = sin_image_1.at<float>(y, x);
					if (weight > 0) v1 /= weight;
					v1 = std::min(std::max(v1, 0.0f), 65535.0f);
					sin_image_u16_1.at<U16>(y, x) = static_cast<U16>(v1);
				}
			}

			cv::cuda::GpuMat gpu_image_ch1, gpu_image_raw_ch1;
			gpu_image_ch1.upload(sin_image_u16_1);
			gpu_image_raw_ch1.upload(cv::Mat(num_of_line, int(samplesPerRecord), CV_16UC1, h_image_1.data()));

			// normalize & scale
			cv::cuda::subtract(65535, gpu_image_ch1, gpu_image_ch1, cv::noArray(), -1, stream);
			cv::cuda::threshold(gpu_image_ch1, gpu_image_ch1, 65535, 0, cv::THRESH_TRUNC, stream);
			cv::cuda::subtract(gpu_image_ch1, 32768, gpu_image_ch1, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image_ch1, 2.0, gpu_image_ch1, 1.0, -1, stream);

			cv::cuda::subtract(65535, gpu_image_raw_ch1, gpu_image_raw_ch1, cv::noArray(), -1, stream);
			cv::cuda::threshold(gpu_image_raw_ch1, gpu_image_raw_ch1, 65535, 0, cv::THRESH_TRUNC, stream);
			cv::cuda::subtract(gpu_image_raw_ch1, 32768, gpu_image_raw_ch1, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image_raw_ch1, 2.0, gpu_image_raw_ch1, 1.0, -1, stream);

			// download to CPU
			cv::Mat Img16bit_ch1, Img16bit_raw_ch1;
			gpu_image_ch1.download(Img16bit_ch1);
			gpu_image_raw_ch1.download(Img16bit_raw_ch1);

			if (!cv::imwrite(ch1_path, Img16bit_ch1))
				std::cerr << "Error: Unable to save the image " << ch1_path << std::endl;
			if (!cv::imwrite(raw_ch1_path, Img16bit_raw_ch1))
				std::cerr << "Error: Unable to save the raw image " << raw_ch1_path << std::endl;
		} while (FindNextFileA(hFind, &find_data));

		FindClose(hFind);

	}

	void YSImageMerge::ProcessSingleBin_raster_1ch(const string& bin_path,const string& ch1_path, const string& raw_ch1_path) {
		ifstream file(bin_path, ios::binary);
		if (!file) {
			cout << "Error: Unable to read " << bin_path << endl;
			return;
		}

		vector<U16> buffer(static_cast<size_t>(num_of_sample));
		file.read(reinterpret_cast<char*>(buffer.data()), static_cast<streamsize>(num_of_sample * sizeof(U16)));
		streamsize got = file.gcount();
		file.close();
		if (got != static_cast<std::streamsize>(num_of_sample * sizeof(U16))) {
			cout << "Error: The length of this read is mismatch " << bin_path << endl;
			return;
		}

		U16 samplesPerRecord = U16(num_of_sample / num_of_line);
		vector<U16> h_image_1(static_cast<size_t>(num_of_sample), 0);

		for (int line = 0; line < num_of_line; ++line) {
			U16* src = buffer.data() + line * samplesPerRecord;
			U16* dst = h_image_1.data() + line * samplesPerRecord;

			if (line % 2 == 0) {
				memcpy(dst, src, sizeof(U16) * int(samplesPerRecord));
			}
			else {
				for (int s = 0; s < int(samplesPerRecord); ++s) {
					int dstIdx = int(samplesPerRecord) - 1 - s;
					dst[dstIdx] = src[s];
				}
			}
		}

		cv::Mat sin_image_1(num_of_line, 1024, CV_32FC1, cv::Scalar(0));
		for (int line = 0; line < num_of_line; ++line) {
			bool is_forward = (line % 2 == 0);
			U16* src = buffer.data() + line * samplesPerRecord;
			int srcIdx = 0;

			for (int sample = 0; sample < int(samplesPerRecord); ++sample) {
				float t = float(sample) / float(max(1, int(samplesPerRecord) - 1));
				float angle = is_forward ? (CV_PI * t - 0.5f * CV_PI)
					: (CV_PI * (1.0f - t) - 0.5f * CV_PI);
				float x = 0.5f * (1.0f + std::sinf(angle));
				int x_idx = int(x * (1024 - 1));
				if (x_idx < 0 || x_idx >= 1024) continue;
				if (srcIdx < int(samplesPerRecord)) sin_image_1.at<float>(line, x_idx) += float(src[srcIdx++]);
			}
		}

		cv::Mat sin_image_u16_1(num_of_line, 1024, CV_16UC1);
		for (int y = 0; y < num_of_line; ++y) {
			for (int x = 0; x < 1024; ++x) {
				float weight = sin_weight.at<float>(y, x);
				float v1 = sin_image_1.at<float>(y, x);
				if (weight > 0) v1 /= weight;
				v1 = min(max(v1, 0.0f), 65535.0f);
				sin_image_u16_1.at<U16>(y, x) = static_cast<U16>(v1);
			}
		}

		cv::cuda::Stream local_stream;
		cv::cuda::GpuMat gpu_image_ch1, gpu_image_raw_ch1;
		gpu_image_ch1.upload(sin_image_u16_1, local_stream);
		gpu_image_raw_ch1.upload(cv::Mat(num_of_line, int(samplesPerRecord), CV_16UC1, h_image_1.data()), local_stream);
		local_stream.waitForCompletion();

		// normalize & scale
		cv::cuda::subtract(65535, gpu_image_ch1, gpu_image_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::threshold(gpu_image_ch1, gpu_image_ch1, 65535, 0, cv::THRESH_TRUNC, local_stream);
		cv::cuda::subtract(gpu_image_ch1, 32768, gpu_image_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::multiply(gpu_image_ch1, 2.0, gpu_image_ch1, 1.0, -1, local_stream);
		local_stream.waitForCompletion();

		cv::cuda::subtract(65535, gpu_image_raw_ch1, gpu_image_raw_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::threshold(gpu_image_raw_ch1, gpu_image_raw_ch1, 65535, 0, cv::THRESH_TRUNC, local_stream);
		cv::cuda::subtract(gpu_image_raw_ch1, 32768, gpu_image_raw_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::multiply(gpu_image_raw_ch1, 2.0, gpu_image_raw_ch1, 1.0, -1, local_stream);
		local_stream.waitForCompletion();

		// download to CPU
		cv::Mat Img16bit_ch1, Img16bit_raw_ch1;
		gpu_image_ch1.download(Img16bit_ch1, local_stream);
		gpu_image_raw_ch1.download(Img16bit_raw_ch1, local_stream);
		local_stream.waitForCompletion();

		if (!cv::imwrite(ch1_path, Img16bit_ch1))
			cout << "Error: Unable to save the image " << ch1_path << endl;
		if (!cv::imwrite(raw_ch1_path, Img16bit_raw_ch1))
			cout << "Error: Unable to save the raw image " << raw_ch1_path << endl;
	}

	void YSImageMerge::ConvertBinToTiff_raster_1ch_parallel() {
		Generate2D_RASTER_SampleMap();

		size_t pos = RASTERFolderPath.find_last_of("\\/");
		string folderName = (pos == string::npos) ? RASTERFolderPath : RASTERFolderPath.substr(pos + 1);

		string output_folder = "C:\\NTU_miniScope_Imaging\\2DRASTER\\" + folderName;
		string output_folder_raw = RASTERFolderPath + "_TIFF";

		CreateDirectoryA(output_folder.c_str(), NULL);
		CreateDirectoryA(output_folder_raw.c_str(), NULL);

		CopyScanSpecCSV(RASTERFolderPath, output_folder);
		CopyScanSpecCSV(RASTERFolderPath, output_folder_raw);

		vector<string> bin_files;
		string search_path = RASTERFolderPath + "\\*.bin";
		WIN32_FIND_DATAA find_data;
		HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
		if (hFind == INVALID_HANDLE_VALUE) {
			cout << "Error: Unable to find any .bin in " << RASTERFolderPath << endl;
			return;
		}

		do {
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			bin_files.push_back(find_data.cFileName);
		} while (FindNextFileA(hFind, &find_data));
		FindClose(hFind);

		vector<std::future<void>> futures;
		futures.reserve(bin_files.size());
		for (auto& b : bin_files) {
			std::string bin_filename = b; 
			string bin_path = RASTERFolderPath + "\\" + bin_filename;

			std::string output_filename = bin_filename.substr(0, bin_filename.find_last_of(".")) + ".tiff";
			std::string ch1_path = output_folder + "\\" + output_filename;
			std::string raw_ch1_path = output_folder_raw + "\\" + output_filename;

			futures.push_back(std::async(std::launch::async,
				[this, bin_path, ch1_path, raw_ch1_path]() {
					this->ProcessSingleBin_raster_1ch(bin_path, ch1_path, raw_ch1_path);
				}
			));
		}

		for (auto& f : futures) f.get();
	}

	void YSImageMerge::ConvertBinToTiff_raster_2chs_parallel() {
		Generate2D_RASTER_SampleMap();

		size_t pos = RASTERFolderPath.find_last_of("\\/");
		string folderName = (pos == string::npos) ? RASTERFolderPath : RASTERFolderPath.substr(pos + 1);
		string output_folder = "C:\\NTU_miniScope_Imaging\\2DRASTER\\" + folderName;
		string output_folder_ch1 = output_folder + "\\Ch1";
		string output_folder_ch2 = output_folder + "\\Ch2";
		string output_folder_raw = RASTERFolderPath + "_TIFF";
		string output_folder_raw_ch1 = output_folder_raw + "\\Ch1";
		string output_folder_raw_ch2 = output_folder_raw + "\\Ch2";
		CreateDirectoryA(output_folder.c_str(), NULL);
		CreateDirectoryA(output_folder_ch1.c_str(), NULL);
		CreateDirectoryA(output_folder_ch2.c_str(), NULL);
		CreateDirectoryA(output_folder_raw.c_str(), NULL);
		CreateDirectoryA(output_folder_raw_ch1.c_str(), NULL);
		CreateDirectoryA(output_folder_raw_ch2.c_str(), NULL);

		CopyScanSpecCSV(RASTERFolderPath, output_folder);
		CopyScanSpecCSV(RASTERFolderPath, output_folder_raw);

		vector<string> bin_files;
		string search_path = RASTERFolderPath + "\\*.bin";
		WIN32_FIND_DATAA find_data;
		HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
		if (hFind == INVALID_HANDLE_VALUE) {
			std::cerr << "Error: Unable to find any .bin in " << RASTERFolderPath << std::endl;
			return;
		}

		do {
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
			bin_files.push_back(find_data.cFileName);
		} while (FindNextFileA(hFind, &find_data));
		FindClose(hFind);

		vector<future<void>> futures;
		futures.reserve(bin_files.size());
		for (auto& b : bin_files) {
			std::string bin_filename = b;
			std::string bin_path = RASTERFolderPath + "\\" + bin_filename;
			std::string output_filename = bin_filename.substr(0, bin_filename.find_last_of(".")) + ".tiff";
			std::string ch1_path = output_folder_ch1 + "\\" + output_filename;
			std::string ch2_path = output_folder_ch2 + "\\" + output_filename;
			std::string raw_ch1_path = output_folder_raw_ch1 + "\\" + output_filename;
			std::string raw_ch2_path = output_folder_raw_ch2 + "\\" + output_filename;

			futures.push_back(std::async(std::launch::async,
				[this, bin_path, ch1_path, raw_ch1_path, ch2_path, raw_ch2_path]() {
					this->ProcessSingleBin_raster_2ch(bin_path, ch1_path, raw_ch1_path, ch2_path, raw_ch2_path);
				}
			));
		}

		for (auto& f : futures) f.get();
	}

	void YSImageMerge::ProcessSingleBin_raster_2ch(const string& bin_path, const string& ch1_path, const string& raw_ch1_path, const string& ch2_path, const string& raw_ch2_path) {
		auto parse_frame_index = [](const std::string& filename) -> long long {
			size_t p = filename.find("Image");
			if (p == std::string::npos) return -1;
			p += 5;
			while (p < filename.size() && !std::isdigit(static_cast<unsigned char>(filename[p]))) ++p;
			size_t start = p;
			while (p < filename.size() && std::isdigit(static_cast<unsigned char>(filename[p]))) ++p;
			if (start == p) return -1;
			try { return std::stoll(filename.substr(start, p - start)); }
			catch (...) { return -1; }
			};

		ifstream file(bin_path, ios::binary);
		if (!file) {
			cout << "Error: Unable to read " << bin_path << endl;
			return;
		}

		vector<U16> buffer(static_cast<size_t>(num_of_sample) * static_cast<size_t>(channelcount));
		file.read(reinterpret_cast<char*>(buffer.data()),static_cast<streamsize>(num_of_sample * sizeof(U16) * channelcount));
		streamsize got = file.gcount();
		file.close();

		if (got != static_cast<streamsize>(num_of_sample * sizeof(U16) * channelcount)) {
			cout << "Error: The length of this read is mismatch " << bin_path << endl;
			return;
		}

		cv::cuda::GpuMat gpu_image_ch1, gpu_image_ch2;
		U16 samplesPerRecord = U16(num_of_sample / num_of_line);
		U16 recStride = samplesPerRecord * 2;

		std::vector<U16> h_image_1(static_cast<size_t>(num_of_sample), 0);
		std::vector<U16> h_image_2(static_cast<size_t>(num_of_sample), 0);

		for (int line = 0; line < num_of_line; ++line) {
			U16* srcA = buffer.data() + U16(line) * recStride;
			U16* srcB = srcA + samplesPerRecord;
			U16* dstA = h_image_1.data() + U16(line) * samplesPerRecord;
			U16* dstB = h_image_2.data() + U16(line) * samplesPerRecord;

			if ((line % 2) == 0) {
				memcpy(dstA, srcA, sizeof(U16) * int(samplesPerRecord));
				memcpy(dstB, srcB, sizeof(U16) * int(samplesPerRecord));
			}
			else {
				for (int s = 0; s < int(samplesPerRecord); ++s) {
					int dstIdx = int(samplesPerRecord) - 1 - s;
					dstA[dstIdx] = srcA[s];
					dstB[dstIdx] = srcB[s];
				}
			}
		}

		cv::cuda::Stream local_stream;
		cv::cuda::GpuMat gpu_image_raw_ch1(num_of_line, int(samplesPerRecord), CV_16UC1);
		gpu_image_raw_ch1.upload(cv::Mat(num_of_line, int(samplesPerRecord), CV_16UC1, h_image_1.data()),local_stream);
		cv::cuda::GpuMat gpu_image_raw_ch2(num_of_line, int(samplesPerRecord), CV_16UC1);
		gpu_image_raw_ch2.upload(cv::Mat(num_of_line, int(samplesPerRecord), CV_16UC1, h_image_2.data()),local_stream);
		local_stream.waitForCompletion();

		cv::Mat sin_image_1(num_of_line, 1024, CV_32FC1, cv::Scalar(0));
		cv::Mat sin_image_2(num_of_line, 1024, CV_32FC1, cv::Scalar(0));

		for (int line = 0; line < num_of_line; ++line) {
			bool is_forward = (line % 2 == 0);
			U16* srcA_new = buffer.data() + U16(line) * recStride;
			U16* srcB_new = srcA_new + samplesPerRecord;

			int srcIdxA = 0, srcIdxB = 0;
			for (int sample = 0; sample < int(samplesPerRecord); ++sample) {
				float t = float(sample) / float(max(1, int(samplesPerRecord) - 1));
				float angle = is_forward ? (CV_PI * t - 0.5f * CV_PI)
					: (CV_PI * (1.0f - t) - 0.5f * CV_PI);
				float x = 0.5f * (1.0f + sinf(angle));
				int   x_idx = int(x * (1024 - 1));
				if (x_idx < 0 || x_idx >= 1024) continue;

				if (srcIdxA < int(samplesPerRecord)) sin_image_1.at<float>(line, x_idx) += float(srcA_new[srcIdxA++]);
				if (srcIdxB < int(samplesPerRecord)) sin_image_2.at<float>(line, x_idx) += float(srcB_new[srcIdxB++]);
			}
		}

		cv::Mat sin_image_u16_1(num_of_line, 1024, CV_16UC1);
		cv::Mat sin_image_u16_2(num_of_line, 1024, CV_16UC1);
		for (int y = 0; y < num_of_line; ++y) {
			for (int x = 0; x < 1024; ++x) {
				float weight = sin_weight.at<float>(y, x);
				float v1 = sin_image_1.at<float>(y, x);
				float v2 = sin_image_2.at<float>(y, x);
				if (weight > 0) { v1 /= weight; v2 /= weight; }
				v1 = min(max(v1, 0.0f), 65535.0f);
				v2 = min(max(v2, 0.0f), 65535.0f);
				sin_image_u16_1.at<U16>(y, x) = static_cast<U16>(v1);
				sin_image_u16_2.at<U16>(y, x) = static_cast<U16>(v2);
			}
		}
		if (num_of_line > 20) {
			cv::cuda::GpuMat gpu_image_prepre_1, gpu_image_preprepre_1;
			gpu_image_preprepre_1.upload(sin_image_u16_1,local_stream);
			cv::cuda::resize(gpu_image_preprepre_1, gpu_image_prepre_1, cv::Size(1024, 1024), 0, 0, cv::INTER_AREA,local_stream);
			cv::cuda::resize(gpu_image_prepre_1, gpu_image_ch1, cv::Size(num_of_line, num_of_line), 0, 0, cv::INTER_AREA, local_stream);
			local_stream.waitForCompletion();

			cv::cuda::GpuMat gpu_image_prepre_2, gpu_image_preprepre_2;
			gpu_image_preprepre_2.upload(sin_image_u16_2, local_stream);
			cv::cuda::resize(gpu_image_preprepre_2, gpu_image_prepre_2, cv::Size(1024, 1024), 0, 0, cv::INTER_AREA, local_stream);
			cv::cuda::resize(gpu_image_prepre_2, gpu_image_ch2, cv::Size(num_of_line, num_of_line), 0, 0, cv::INTER_AREA, local_stream);
			local_stream.waitForCompletion();
		}
		else {
			gpu_image_ch1.upload(sin_image_u16_1,local_stream);
			gpu_image_ch2.upload(sin_image_u16_2,local_stream);
		}

		size_t p = bin_path.find_last_of("\\/");
		string bin_filename = (p == string::npos) ? bin_path : bin_path.substr(p + 1);
		long long frame_idx = parse_frame_index(bin_filename);
		bool need_flip_vertically = (frame_idx >= 0) && ((frame_idx % 2LL) == 1LL);

		if (need_flip_vertically) {
			cv::cuda::flip(gpu_image_raw_ch1, gpu_image_raw_ch1, 0, local_stream);
			cv::cuda::flip(gpu_image_raw_ch2, gpu_image_raw_ch2, 0, local_stream);
			cv::cuda::flip(gpu_image_ch1, gpu_image_ch1, 0, local_stream);
			cv::cuda::flip(gpu_image_ch2, gpu_image_ch2, 0, local_stream);
		}

		cv::cuda::subtract(65535, gpu_image_ch1, gpu_image_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::threshold(gpu_image_ch1, gpu_image_ch1, 65535, 0, cv::THRESH_TRUNC, local_stream);
		cv::cuda::subtract(gpu_image_ch1, 32768, gpu_image_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::multiply(gpu_image_ch1, 2.0, gpu_image_ch1, 1.0, -1, local_stream);

		cv::cuda::subtract(65535, gpu_image_raw_ch1, gpu_image_raw_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::threshold(gpu_image_raw_ch1, gpu_image_raw_ch1, 65535, 0, cv::THRESH_TRUNC, local_stream);
		cv::cuda::subtract(gpu_image_raw_ch1, 32768, gpu_image_raw_ch1, cv::noArray(), -1, local_stream);
		cv::cuda::multiply(gpu_image_raw_ch1, 2.0, gpu_image_raw_ch1, 1.0, -1, local_stream);

		cv::cuda::subtract(gpu_image_ch2, 32768, gpu_image_ch2, cv::noArray(), -1, local_stream);
		cv::cuda::multiply(gpu_image_ch2, 2.0, gpu_image_ch2, 1.0, -1, local_stream);

		cv::cuda::subtract(gpu_image_raw_ch2, 32768, gpu_image_raw_ch2, cv::noArray(), -1, local_stream);
		cv::cuda::multiply(gpu_image_raw_ch2, 2.0, gpu_image_raw_ch2, 1.0, -1, local_stream);
		local_stream.waitForCompletion();

		cv::Mat Img16bit_ch1, Img16bit_ch2, Img16bit_raw_ch1, Img16bit_raw_ch2;
		gpu_image_ch1.download(Img16bit_ch1, local_stream);
		gpu_image_ch2.download(Img16bit_ch2, local_stream);
		gpu_image_raw_ch1.download(Img16bit_raw_ch1, local_stream);
		gpu_image_raw_ch2.download(Img16bit_raw_ch2, local_stream);
		local_stream.waitForCompletion();

		if (!cv::imwrite(ch1_path, Img16bit_ch1))     std::cerr << "Error: Unable to save the image " << ch1_path << std::endl;
		if (!cv::imwrite(ch2_path, Img16bit_ch2))     std::cerr << "Error: Unable to save the image " << ch2_path << std::endl;
		if (!cv::imwrite(raw_ch1_path, Img16bit_raw_ch1)) std::cerr << "Error: Unable to save the image " << raw_ch1_path << std::endl;
		if (!cv::imwrite(raw_ch2_path, Img16bit_raw_ch2)) std::cerr << "Error: Unable to save the image " << raw_ch2_path << std::endl;
	}

	void YSImageMerge::ConvertBinToTiff_raster_2chs() {
		Generate2D_RASTER_SampleMap();

		size_t pos = RASTERFolderPath.find_last_of("\\/");
		string folderName = (pos == string::npos) ? RASTERFolderPath : RASTERFolderPath.substr(pos + 1);
		string output_folder = "C:\\NTU_miniScope_Imaging\\2DRASTER\\" + folderName;
		string output_folder_ch1 = output_folder + "\\Ch1";
		string output_folder_ch2 = output_folder + "\\Ch2";
		string output_folder_raw = RASTERFolderPath + "_TIFF";
		string output_folder_raw_ch1 = output_folder_raw + "\\Ch1";
		string output_folder_raw_ch2 = output_folder_raw + "\\Ch2";
		CreateDirectoryA(output_folder.c_str(), NULL);
		CreateDirectoryA(output_folder_ch1.c_str(), NULL);
		CreateDirectoryA(output_folder_ch2.c_str(), NULL);
		CreateDirectoryA(output_folder_raw.c_str(), NULL);
		CreateDirectoryA(output_folder_raw_ch1.c_str(), NULL);
		CreateDirectoryA(output_folder_raw_ch2.c_str(), NULL);

		CopyScanSpecCSV(RASTERFolderPath, output_folder);
		CopyScanSpecCSV(RASTERFolderPath, output_folder_raw);

		string search_path = RASTERFolderPath + "\\*.bin";
		WIN32_FIND_DATAA find_data;
		HANDLE hFind = FindFirstFileA(search_path.c_str(), &find_data);
		if (hFind == INVALID_HANDLE_VALUE) {
			cerr << "Error: Unable to find any .bin in " << RASTERFolderPath << endl;
			return;
		}

		auto parse_frame_index = [](const std::string& filename) -> long long {
			size_t p = filename.find("Image");
			if (p == std::string::npos) return -1;
			p += 5;
			while (p < filename.size() && !std::isdigit(static_cast<unsigned char>(filename[p]))) ++p;
			size_t start = p;
			while (p < filename.size() && std::isdigit(static_cast<unsigned char>(filename[p]))) ++p;
			if (start == p) return -1;
			try { return std::stoll(filename.substr(start, p - start)); }
			catch (...) { return -1; }
			};


		do {
			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

			std::string bin_filename = find_data.cFileName;
			std::string bin_path = RASTERFolderPath + "\\" + bin_filename;

			std::string output_filename = bin_filename.substr(0, bin_filename.find_last_of(".")) + ".tiff";
			std::string raw_ch1_path = output_folder_raw_ch1 + "\\" + output_filename;
			std::string raw_ch2_path = output_folder_raw_ch2 + "\\" + output_filename;
			std::string ch1_path = output_folder_ch1 + "\\" + output_filename;
			std::string ch2_path = output_folder_ch2 + "\\" + output_filename;

			std::ifstream file(bin_path, std::ios::binary);
			if (!file) {
				std::cerr << "Error: Unable to read " << bin_path << std::endl;
				continue;
			}

			std::vector<U16> buffer(static_cast<size_t>(num_of_sample) * static_cast<size_t>(channelcount));
			file.read(reinterpret_cast<char*>(buffer.data()),
				static_cast<std::streamsize>(num_of_sample * sizeof(U16) * channelcount));
			std::streamsize got = file.gcount();
			file.close();

			if (got != static_cast<std::streamsize>(num_of_sample * sizeof(U16) * channelcount)) {
				std::cerr << "Error: The length of this read is mismatch " << bin_path << std::endl;
				continue;
			}

			cv::cuda::GpuMat gpu_image_ch1, gpu_image_ch2;
			U16 samplesPerRecord = U16(num_of_sample / num_of_line);
			U16 recStride = samplesPerRecord * 2;

			std::vector<U16> h_image_1(static_cast<size_t>(num_of_sample), 0);
			std::vector<U16> h_image_2(static_cast<size_t>(num_of_sample), 0);

			for (int line = 0; line < num_of_line; ++line) {
				U16* srcA = buffer.data() + U16(line) * recStride;
				U16* srcB = srcA + samplesPerRecord;
				U16* dstA = h_image_1.data() + U16(line) * samplesPerRecord;
				U16* dstB = h_image_2.data() + U16(line) * samplesPerRecord;

				if ((line % 2) == 0) {
					memcpy(dstA, srcA, sizeof(U16) * int(samplesPerRecord));
					memcpy(dstB, srcB, sizeof(U16) * int(samplesPerRecord));
				}
				else {
					for (int s = 0; s < int(samplesPerRecord); ++s) {
						int dstIdx = int(samplesPerRecord) - 1 - s;
						dstA[dstIdx] = srcA[s];
						dstB[dstIdx] = srcB[s];
					}
				}
			}

			cv::cuda::GpuMat gpu_image_raw_ch1(num_of_line, int(samplesPerRecord), CV_16UC1);
			gpu_image_raw_ch1.upload(cv::Mat(num_of_line, int(samplesPerRecord), CV_16UC1, h_image_1.data()));
			cv::cuda::GpuMat gpu_image_raw_ch2(num_of_line, int(samplesPerRecord), CV_16UC1);
			gpu_image_raw_ch2.upload(cv::Mat(num_of_line, int(samplesPerRecord), CV_16UC1, h_image_2.data()));


			cv::Mat sin_image_1(num_of_line, 1024, CV_32FC1, cv::Scalar(0));
			cv::Mat sin_image_2(num_of_line, 1024, CV_32FC1, cv::Scalar(0));

			for (int line = 0; line < num_of_line; ++line) {
				bool is_forward = (line % 2 == 0);
				U16* srcA_new = buffer.data() + U16(line) * recStride;
				U16* srcB_new = srcA_new + samplesPerRecord;

				int srcIdxA = 0, srcIdxB = 0;
				for (int sample = 0; sample < int(samplesPerRecord); ++sample) {
					float t = float(sample) / float(std::max(1, int(samplesPerRecord) - 1));
					float angle = is_forward ? (CV_PI * t - 0.5f * CV_PI)
						: (CV_PI * (1.0f - t) - 0.5f * CV_PI);
					float x = 0.5f * (1.0f + std::sinf(angle));
					int   x_idx = int(x * (1024 - 1));
					if (x_idx < 0 || x_idx >= 1024) continue;

					if (srcIdxA < int(samplesPerRecord)) sin_image_1.at<float>(line, x_idx) += float(srcA_new[srcIdxA++]);
					if (srcIdxB < int(samplesPerRecord)) sin_image_2.at<float>(line, x_idx) += float(srcB_new[srcIdxB++]);
				}
			}

			cv::Mat sin_image_u16_1(num_of_line, 1024, CV_16UC1);
			cv::Mat sin_image_u16_2(num_of_line, 1024, CV_16UC1);
			for (int y = 0; y < num_of_line; ++y) {
				for (int x = 0; x < 1024; ++x) {
					float weight = sin_weight.at<float>(y, x);
					float v1 = sin_image_1.at<float>(y, x);
					float v2 = sin_image_2.at<float>(y, x);
					if (weight > 0) { v1 /= weight; v2 /= weight; }
					v1 = std::min(std::max(v1, 0.0f), 65535.0f);
					v2 = std::min(std::max(v2, 0.0f), 65535.0f);
					sin_image_u16_1.at<U16>(y, x) = static_cast<U16>(v1);
					sin_image_u16_2.at<U16>(y, x) = static_cast<U16>(v2);
				}
			}
			if (num_of_line > 20) {
				cv::cuda::GpuMat gpu_image_prepre_1, gpu_image_preprepre_1;
				gpu_image_preprepre_1.upload(sin_image_u16_1);
				cv::cuda::resize(gpu_image_preprepre_1, gpu_image_prepre_1, cv::Size(1024, 1024), 0, 0, cv::INTER_AREA);
				cv::cuda::resize(gpu_image_prepre_1, gpu_image_ch1, cv::Size(num_of_line, num_of_line), 0, 0, cv::INTER_AREA);

				cv::cuda::GpuMat gpu_image_prepre_2, gpu_image_preprepre_2;
				gpu_image_preprepre_2.upload(sin_image_u16_2);
				cv::cuda::resize(gpu_image_preprepre_2, gpu_image_prepre_2, cv::Size(1024, 1024), 0, 0, cv::INTER_AREA);
				cv::cuda::resize(gpu_image_prepre_2, gpu_image_ch2, cv::Size(num_of_line, num_of_line), 0, 0, cv::INTER_AREA);
			}
			else {
				gpu_image_ch1.upload(sin_image_u16_1);
				gpu_image_ch2.upload(sin_image_u16_2);
			}


			long long frame_idx = parse_frame_index(bin_filename);
			bool need_flip_vertically = (frame_idx >= 0) && ((frame_idx % 2LL) == 0LL);

			if (need_flip_vertically) {
				cv::cuda::flip(gpu_image_raw_ch1, gpu_image_raw_ch1, 0, stream);
				cv::cuda::flip(gpu_image_raw_ch2, gpu_image_raw_ch2, 0, stream);
				cv::cuda::flip(gpu_image_ch1, gpu_image_ch1, 0, stream);
				cv::cuda::flip(gpu_image_ch2, gpu_image_ch2, 0, stream);
			}

			cv::cuda::subtract(65535, gpu_image_ch1, gpu_image_ch1, cv::noArray(), -1, stream);
			cv::cuda::threshold(gpu_image_ch1, gpu_image_ch1, 65535, 0, cv::THRESH_TRUNC, stream);
			cv::cuda::subtract(gpu_image_ch1, 32768, gpu_image_ch1, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image_ch1, 2.0, gpu_image_ch1, 1.0, -1, stream);

			cv::cuda::subtract(65535, gpu_image_raw_ch1, gpu_image_raw_ch1, cv::noArray(), -1, stream);
			cv::cuda::threshold(gpu_image_raw_ch1, gpu_image_raw_ch1, 65535, 0, cv::THRESH_TRUNC, stream);
			cv::cuda::subtract(gpu_image_raw_ch1, 32768, gpu_image_raw_ch1, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image_raw_ch1, 2.0, gpu_image_raw_ch1, 1.0, -1, stream);

			cv::cuda::subtract(gpu_image_ch2, 32768, gpu_image_ch2, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image_ch2, 2.0, gpu_image_ch2, 1.0, -1, stream);

			cv::cuda::subtract(gpu_image_raw_ch2, 32768, gpu_image_raw_ch2, cv::noArray(), -1, stream);
			cv::cuda::multiply(gpu_image_raw_ch2, 2.0, gpu_image_raw_ch2, 1.0, -1, stream);

			cv::Mat Img16bit_ch1, Img16bit_ch2, Img16bit_raw_ch1, Img16bit_raw_ch2;
			gpu_image_ch1.download(Img16bit_ch1);
			gpu_image_ch2.download(Img16bit_ch2);
			gpu_image_raw_ch1.download(Img16bit_raw_ch1);
			gpu_image_raw_ch2.download(Img16bit_raw_ch2);

			if (!cv::imwrite(ch1_path, Img16bit_ch1))     std::cerr << "Error: Unable to save the image " << ch1_path << std::endl;
			if (!cv::imwrite(ch2_path, Img16bit_ch2))     std::cerr << "Error: Unable to save the image " << ch2_path << std::endl;
			if (!cv::imwrite(raw_ch1_path, Img16bit_raw_ch1)) std::cerr << "Error: Unable to save the image " << raw_ch1_path << std::endl;
			if (!cv::imwrite(raw_ch2_path, Img16bit_raw_ch2)) std::cerr << "Error: Unable to save the image " << raw_ch2_path << std::endl;

		} while (FindNextFileA(hFind, &find_data));

		FindClose(hFind);

	}

	//Reconstruct//
	void YSImageMerge::DisplayReconstructedLissajousImage() {
		if (channelcount == 1) {
			if (firstreconstruct == true) {
				AVERAGE(LISSAJOUSFolderPath, averageImg16);
			}
			
			LissajousrawData = new float64[num_of_sample];
			memset(LissajousrawData, 0, num_of_sample * sizeof(float64));
			cv::Mat avergeimg64;
			averageImg16.convertTo(avergeimg64, CV_64FC1);
			memcpy(LissajousrawData, avergeimg64.ptr<double>(), num_of_sample * sizeof(float64));

			Image = new float64[pixelnum * pixelnum];
			memset(Image, 0, pixelnum * pixelnum * sizeof(float64));
			LissajoustempData = new float64[num_of_sample];
			memset(LissajoustempData, 0, num_of_sample * sizeof(float64));

			memcpy(LissajoustempData, LissajousrawData, num_of_sample * sizeof(float64));
			if (abs(offset_lissajous) < num_of_sample) {
				if (offset_lissajous > 0) {
					for (int i = num_of_sample - 1; i >= offset_lissajous; --i)
						LissajoustempData[i] = LissajoustempData[i - offset_lissajous];
					for (int i = 0; i < offset_lissajous; ++i)
						LissajoustempData[i] = 0;
				}
				else if (offset_lissajous < 0) {
					for (int i = 0; i < num_of_sample - abs(offset_lissajous); ++i) {
						int position = i + abs(offset_lissajous);
						LissajoustempData[i] = LissajoustempData[position];
					}
					for (int i = num_of_sample - abs(offset_lissajous); i < num_of_sample; ++i)
						LissajoustempData[i] = 0;
				}
			}

			for (int i = 0; i < num_of_sample; ++i) {
				int x = 0;
				int y = 0;
				bool inrange = LissajousIdxFromSample(i, samplingrate, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, x, y);
				if (inrange == true) {
					int idx = y * pixelnum + x;
					if (sampleMap[idx] > 0)
						Image[idx] += LissajoustempData[i];
				}
			}
			for (int i = 0; i < pixelnum * pixelnum; ++i) {
				if (sampleMap[i] > 0)
					Image[i] /= sampleMap[i];
			}
			//reconstruct->launchProcessSamples_LISSAJOUS(LissajoustempData,Image);

			cv::Mat image64F(pixelnum, pixelnum, CV_64FC1, Image);
			cv::Mat image16U;
			image64F.convertTo(image16U, CV_16UC1, 1.0);

			cv::Mat image8U;
			image16U.convertTo(image8U, CV_8UC1, 1.0 / 257);

			cv::cuda::GpuMat gpuimg;
			gpuimg.upload(image8U);
			cv::Mat LUT_matrix(1, 256, CV_8UC1);
			uchar* pointer = LUT_matrix.ptr();
			for (int i = 0; i < 256; i++) {
				pointer[i] = (int)(pow((double)(i / 255.0), 1.1) * 255.0);
			}
			cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
			cudaLookUpTable->transform(gpuimg, gpuimg, stream);
			gpuimg.cv::cuda::GpuMat::convertTo(gpuimg, -1, 3, 150 - 150, stream);
			stream.waitForCompletion();
			cv::Mat img;
			gpuimg.download(img);

			cv::imshow("Reconstruct Image", img);
			cv::waitKey(1);
		}
		else if (channelcount == 2) {
			if (firstreconstruct == true) {
				string channel1path = LISSAJOUSFolderPath + "\\CH1";
				string channel2path = LISSAJOUSFolderPath + "\\CH2";
				AVERAGE(channel1path, averageImg16);
				AVERAGE(channel2path, averageImg16_ch2);
			}

			LissajousrawData = new float64[num_of_sample];
			LissajousrawData_ch2 = new float64[num_of_sample];
			memset(LissajousrawData, 0, num_of_sample * sizeof(float64));
			memset(LissajousrawData_ch2, 0, num_of_sample * sizeof(float64));
			cv::Mat averageimg64_ch1, averageimg64_ch2;
			averageImg16.convertTo(averageimg64_ch1, CV_64FC1);
			memcpy(LissajousrawData, averageimg64_ch1.ptr<float64>(), num_of_sample * sizeof(float64));
			averageImg16_ch2.convertTo(averageimg64_ch2, CV_64FC1);
			memcpy(LissajousrawData_ch2, averageimg64_ch2.ptr<float64>(), num_of_sample * sizeof(float64));

			Image = new float64[pixelnum * pixelnum];
			Image_ch2 = new float64[pixelnum * pixelnum];
			memset(Image, 0, pixelnum * pixelnum * sizeof(float64));
			memset(Image_ch2, 0, pixelnum * pixelnum * sizeof(float64));
			LissajoustempData = new float64[num_of_sample];
			LissajoustempData_ch2 = new float64[num_of_sample];
			memset(LissajoustempData, 0, num_of_sample * sizeof(float64));
			memset(LissajoustempData_ch2, 0, num_of_sample * sizeof(float64));

			memcpy(LissajoustempData, LissajousrawData, num_of_sample * sizeof(float64));
			memcpy(LissajoustempData_ch2, LissajousrawData_ch2, num_of_sample * sizeof(float64));
			if (abs(offset_lissajous) < num_of_sample) {
				if (offset_lissajous > 0) {
					for (int i = num_of_sample - 1; i >= offset_lissajous; --i) {
						LissajoustempData[i] = LissajoustempData[i - offset_lissajous];
						LissajoustempData_ch2[i] = LissajoustempData_ch2[i - offset_lissajous];
					}
					for (int i = 0; i < offset_lissajous; ++i) {
						LissajoustempData[i] = 0;
						LissajoustempData_ch2[i] = 0;
					}
				}
				else if (offset_lissajous < 0) {
					for (int i = 0; i < num_of_sample - abs(offset_lissajous); ++i) {
						int position = i + abs(offset_lissajous);
						LissajoustempData[i] = LissajoustempData[position];
						LissajoustempData_ch2[i] = LissajoustempData_ch2[position];
					}
					for (int i = num_of_sample - abs(offset_lissajous); i < num_of_sample; ++i) {
						LissajoustempData[i] = 0;
						LissajoustempData_ch2[i] = 0;
					}
						
				}
			}

			for (int i = 0; i < num_of_sample; ++i) {
				int x = 0;
				int y = 0;
				bool inrange = LissajousIdxFromSample(i, samplingrate, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, x, y);
				if (inrange == true) {
					int idx = y * pixelnum + x;
					if (sampleMap[idx] > 0) {
						Image[idx] += LissajoustempData[i];
						Image_ch2[idx] += LissajoustempData_ch2[i];
					}
				}
			}
			for (int i = 0; i < pixelnum * pixelnum; ++i) {
				if (sampleMap[i] > 0) {
					Image[i] /= sampleMap[i];
					Image_ch2[i] /= sampleMap[i];
				}
					
			}
			//reconstruct->launchProcessSamples_LISSAJOUS(LissajoustempData, Image);
			//reconstruct->launchProcessSamples_LISSAJOUS(LissajoustempData_ch2, Image_ch2);

			cv::Mat image64F_ch1(pixelnum, pixelnum, CV_64FC1, Image);
			cv::Mat image64F_ch2(pixelnum, pixelnum, CV_64FC1, Image_ch2);
			cv::Mat image16U_ch1, image16U_ch2;
			image64F_ch1.convertTo(image16U_ch1, CV_16UC1, 1.0);
			image64F_ch2.convertTo(image16U_ch2, CV_16UC1, 1.0);

			cv::Mat image8U_ch1, image8U_ch2;
			image16U_ch1.convertTo(image8U_ch1, CV_8UC1, 1.0 / 257);
			image16U_ch2.convertTo(image8U_ch2, CV_8UC1, 1.0 / 257);

			cv::cuda::GpuMat gpuimg_ch1, gpuimg_ch2;
			gpuimg_ch1.upload(image8U_ch1);
			gpuimg_ch2.upload(image8U_ch2);
			cv::Mat LUT_matrix(1, 256, CV_8UC1);
			uchar* pointer = LUT_matrix.ptr();
			for (int i = 0; i < 256; i++) {
				pointer[i] = (int)(pow((double)(i / 255.0), 1.1) * 255.0);
			}
			cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
			cudaLookUpTable->transform(gpuimg_ch1, gpuimg_ch1, stream);
			cudaLookUpTable->transform(gpuimg_ch2, gpuimg_ch2, stream);
			gpuimg_ch1.cv::cuda::GpuMat::convertTo(gpuimg_ch1, -1, 3, 150 - 150, stream);
			gpuimg_ch2.cv::cuda::GpuMat::convertTo(gpuimg_ch2, -1, 3, 150 - 150, stream);
			stream.waitForCompletion();
			cv::Mat img_ch1, img_ch2;
			gpuimg_ch1.download(img_ch1);
			gpuimg_ch2.download(img_ch2);

			cv::imshow("Reconstruct Image CH1", img_ch1);
			cv::imshow("Reconstruct Image CH2", img_ch2);
			cv::waitKey(1);
		}
	}

	void YSImageMerge::ReconstructAllTiffInFolder() {
		if (channelcount == 1) {
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
			CopyScanSpecCSV(LISSAJOUSFolderPath, outputFolderPath);

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

				if (abs(offset_lissajous) < num_of_sample) {
					if (offset_lissajous > 0) {
						for (int i = num_of_sample - 1; i >= offset_lissajous; --i)
							LissajoustempData[i] = LissajoustempData[i - offset_lissajous];
						for (int i = 0; i < offset_lissajous; ++i)
							LissajoustempData[i] = 0;
					}
					else if (offset_lissajous < 0) {
						for (int i = 0; i < num_of_sample - abs(offset_lissajous); ++i)
							LissajoustempData[i] = LissajoustempData[i + abs(offset_lissajous)];
						for (int i = num_of_sample - abs(offset_lissajous); i < num_of_sample; ++i)
							LissajoustempData[i] = 0;
					}
				}

				for (int i = 0; i < num_of_sample; ++i) {
					int x = 0;
					int y = 0;
					bool inrange = LissajousIdxFromSample(i, samplingrate, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, x, y);
					if (inrange == true) {
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
			}
		}
		else if (channelcount == 2) {
			string lissajousFolderPathwithtiff_ch1 = LISSAJOUSFolderPath + "\\CH1\\*.tiff";
			string lissajousFolderPathwithtiff_ch2 = LISSAJOUSFolderPath + "\\CH2\\*.tiff";
			vector<string> filenames;
			WIN32_FIND_DATA findFileData;
			HANDLE hFind = FindFirstFile(lissajousFolderPathwithtiff_ch1.c_str(), &findFileData);
			if (hFind == INVALID_HANDLE_VALUE) {
				cout << "Error: No .tiff files found in directory " << lissajousFolderPathwithtiff_ch1 << endl;
				return;
			}
			do {
				filenames.push_back(findFileData.cFileName);
			} while (FindNextFile(hFind, &findFileData) != 0);
			FindClose(hFind);

			string outputFolderPath = LISSAJOUSFolderPath;
			size_t pos = outputFolderPath.find("2DLISSAJOUS_RAW");
			if (pos != string::npos) {
				outputFolderPath.replace(pos, strlen("2DLISSAJOUS_RAW"), "2DLISSAJOUS");
			}
			string outputFolderPath_ch1 = outputFolderPath + "\\CH1";
			string outputFolderPath_ch2 = outputFolderPath + "\\CH2";
			CreateDirectory(outputFolderPath.c_str(), NULL);
			CreateDirectory(outputFolderPath_ch1.c_str(), NULL);
			CreateDirectory(outputFolderPath_ch2.c_str(), NULL);
			CopyScanSpecCSV(LISSAJOUSFolderPath, outputFolderPath);

			// Sort filenames based on the numeric part
			sort(filenames.begin(), filenames.end(), [this](const string& a, const string& b) {
				return extractNumber(a) < extractNumber(b);
				});

			int total_lissajousimage_count = filenames.size();
			lissajousimgindex = 0;
			for (const auto& filename : filenames) {
				string fullPath_ch1 = LISSAJOUSFolderPath + "\\CH1" + "\\" + filename;
				string fullPath_ch2 = LISSAJOUSFolderPath + "\\CH2" + "\\" + filename;
				cv::Mat updateImage_ch1 = cv::imread(fullPath_ch1, cv::IMREAD_UNCHANGED);
				cv::Mat updateImage_ch2 = cv::imread(fullPath_ch2, cv::IMREAD_UNCHANGED);
				if (updateImage_ch1.empty() || updateImage_ch1.type() != CV_16UC1) {
					cout << "Error: TIFF not found or not CV_16UC1 format " << fullPath_ch1 << endl;
					continue;
				}
				if (updateImage_ch2.empty() || updateImage_ch2.type() != CV_16UC1) {
					cout << "Error: TIFF not found or not CV_16UC1 format " << fullPath_ch2 << endl;
					continue;
				}

				LissajoustempData = new double[num_of_sample];
				LissajoustempData_ch2 = new double[num_of_sample];
				Image = new double[pixelnum * pixelnum];
				Image_ch2 = new double[pixelnum * pixelnum];
				memset(LissajoustempData, 0, num_of_sample * sizeof(double));
				memset(LissajoustempData_ch2, 0, num_of_sample * sizeof(double));
				memset(Image, 0, pixelnum * pixelnum * sizeof(double));
				memset(Image_ch2, 0, pixelnum* pixelnum * sizeof(double));

				cv::Mat updateImage64_ch1, updateImage64_ch2;
				updateImage_ch1.convertTo(updateImage64_ch1, CV_64FC1);
				updateImage_ch2.convertTo(updateImage64_ch2, CV_64FC1);
				memcpy(LissajoustempData, updateImage64_ch1.ptr<double>(), num_of_sample * sizeof(double));
				memcpy(LissajoustempData_ch2, updateImage64_ch2.ptr<double>(), num_of_sample * sizeof(double));

				if (offset_lissajous < num_of_sample) {
					for (int i = num_of_sample - 1; i >= offset_lissajous; --i) {
						LissajoustempData[i] = LissajoustempData[i - offset_lissajous];
						LissajoustempData_ch2[i] = LissajoustempData_ch2[i - offset_lissajous];
					}					
					for (int i = 0; i < offset_lissajous; ++i) {
						LissajoustempData[i] = 0;
						LissajoustempData_ch2[i] = 0;
					}
				}

				for (int i = 0; i < num_of_sample; ++i) {
					int x = 0;
					int y = 0;
					bool inrange = LissajousIdxFromSample(i, samplingrate, pixelnum, pixelnum, double(xfreq), double(yfreq), lissajousphase, x, y);
					if (inrange == true) {
						int idx = y * pixelnum + x;
						if (sampleMap[idx] > 0) {
							Image[idx] += LissajoustempData[i];
							Image_ch2[idx] += LissajoustempData_ch2[i];
						}			
					}
				}
				for (int i = 0; i < pixelnum * pixelnum; ++i) {
					if (sampleMap[i] > 0) {
						Image[i] /= sampleMap[i];
						Image_ch2[i] /= sampleMap[i];
					}
						
				}

				cv::Mat image64F_ch1(pixelnum, pixelnum, CV_64FC1, Image);
				cv::Mat image64F_ch2(pixelnum, pixelnum, CV_64FC1, Image_ch2);
				cv::Mat image16U_ch1, image16U_ch2;
				image64F_ch1.convertTo(image16U_ch1, CV_16UC1, 1.0);
				image64F_ch2.convertTo(image16U_ch2, CV_16UC1, 1.0);
				ostringstream savePath_ch1, savePath_ch2;
				savePath_ch1 << outputFolderPath_ch1 << "\\Image" << (lissajousimgindex + 1) << ".tiff";
				savePath_ch2 << outputFolderPath_ch2 << "\\Image" << (lissajousimgindex + 1) << ".tiff";
				if (!cv::imwrite(savePath_ch1.str(), image16U_ch1)) {
					cerr << "Error: Failed to save image " << savePath_ch1.str() << endl;
				}
				if (!cv::imwrite(savePath_ch2.str(), image16U_ch2)) {
					cerr << "Error: Failed to save image " << savePath_ch2.str() << endl;
				}

				lissajousimgindex++;
				progresspercentage = double(lissajousimgindex) / double(total_lissajousimage_count) * 100;
			}
		}
		
	}

	//Supplementart functions
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
				try { lissajousphase = stod(value); }
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
			else if (key == "xamp") {
				try { ampx = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting xamp value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "yamp") {
				try { ampy = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting yamp value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "chopperfreq") {
				try { chopperfreq = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting chopperfreq value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "chopperphase") {
				try { chopperphase = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting chopperphase value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "channelnumber") {
				try { channelcount = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting channelcount value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "linefreq") {
				try { linefreq = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting linefreq value: " << value << " (" << e.what() << ")" << endl;
				}
			}
			else if (key == "num_of_line") {
				try { num_of_line = stod(value); }
				catch (const exception& e) {
					cerr << "Error converting num_of_line value: " << value << " (" << e.what() << ")" << endl;
				}
			}
		}
		file.close();
	}

	bool YSImageMerge::LissajousIdxFromSample(int sample_idx, double sampling_rate, int widthpixelnum, int heightpixelnum, double xfreq, double yfreq,double lissajous_phase,int& x_out,int& y_out) {
		if (sampling_rate <= 0.0 || pixelnum <= 1) {
			x_out = y_out = -1;
			return false;
		}

		double t = double(sample_idx) / sampling_rate;
		int x = int(round((widthpixelnum / 2.0) * (1 - sin(2 * M_PI * double(xfreq) * t))));
		int y = int(round((heightpixelnum / 2.0) * (1 - sin(2 * M_PI * double(yfreq) * t + lissajous_phase))));
		bool in_bounds = (0 <= x && x < widthpixelnum) && (0 <= y && y < heightpixelnum);
		x_out = x;
		y_out = y;
		return in_bounds;
	}

	void YSImageMerge::CopyScanSpecCSV(string& sourceFolder, string& targetFolder) {
		std::string sourceCSV = sourceFolder + "\\scan_spec.csv";
		std::string targetCSV = targetFolder + "\\scan_spec.csv";

		if (!CopyFileA(sourceCSV.c_str(), targetCSV.c_str(), FALSE)) {
			std::cerr << "Error: Unable to copy scan_spec.csv to " << targetCSV << std::endl;
		}
	}

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

	void YSImageMerge::AddtextOnImg(cv::Mat& IMGDP16, int frameorder, int totalframe) {
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
				sumImage = cv::Mat::zeros(imageSize, CV_64FC1);
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
		sumImage.convertTo(avgImage, CV_16UC1);
	
	}
}