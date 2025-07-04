#pragma once
#include "NTUcore.h"
#include "Additional.h"

namespace NTU
{
	// To merge grayscale images into BGR
	std::vector<cv::cuda::GpuMat> mergeVector(3);

	#ifdef _WIN32
	#define MKDIR_CMD "mkdir "
	#else
	#define MKDIR_CMD "mkdir -p "
	#endif

	std::string getCurrentTimeAsFolderName_raw()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm;

		localtime_s(&local_tm, &now_time);  // Windows
		std::ostringstream oss;
		oss << "C:\\NTU_miniScope_Imaging\\RAW\\raw_" << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S");
		return oss.str();
	}

	std::string getCurrentTimeAsFolderName_binning()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm;

		localtime_s(&local_tm, &now_time);  // Windows
		std::ostringstream oss;
		oss << "C:\\NTU_miniScope_Imaging\\BINNING\\binning_" << std::put_time(&local_tm, "%Y-%m-%d_%H-%M-%S");
		return oss.str();
	}

	// CHANNEL 1
	void NTUscope::PrevChannel_1()
	{
		// Updating scanning thread
		threadEnd[THREAD_CHAN_1] = false;

		if (true)
		{
			printf("Error: AlazarWaitAsyncBufferComplete --- ApiWaitTimeout");
		}

		// Channel number
		const int CHAN = CHAN_1;
		RapidStarted[CHAN] = false;
		volatile bool PrevStatus[2];

		// Exiting if not allowed
		if (allow_scan == false ||
			allow_configure == false)
			exit(0);

		// Thread Priority Setting
		DWORD mask = 0 << 0;
		for (int i = 2; i <= NumCPU - 1; i++)
			mask = mask + (1 << i);
		auto ret = SetThreadAffinityMask(GetCurrentThread(), mask);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		U16 avg = 0;
		int mapped_value = 0;
		int beta = 0;
		double alpha = 0.0;
		double gamma = 0.0;
		global_GPU_busy = true;
		bool* filledLines;
		filledLines = (bool*)malloc((samplesPerRecord + 500) * sizeof(bool));
		

		// DECLARING THE MAT OBJECTS IN REAL TIME
		GpuMat GMatProc[15];
		GpuMat result3CD;
		Mat resultResized;
		Mat rawImage;
		GpuMat imgDP8;
		GpuMat imgDP8_temp;
		GpuMat imgDP8resized;
		GpuMat imgDP8resizedRGB;
		GpuMat imgDP16_temp;
		GpuMat imgD16;
		GpuMat imgDP32;
		GpuMat tempSub;
		GpuMat mapD;
		
		// For CUDA streaming
		cuda::Stream stream;

		// For window cropping
		bool stitchWinAlign;
		bool croppedWindowON;
		int param;

		// CHANNEL INDEX
		const int index = 0;
		if (chan_merge)	param = CROP_C1;
		else			param = CROP_M;

		// For refresh rate
		std::chrono::duration<double> time_span;
		std::chrono::high_resolution_clock::time_point t2;
		std::chrono::high_resolution_clock::time_point t1;

		// Loop starts here -> keeps on repeating until user stops!
		while (img_thread_cont_chan_1 && running_loops)
		{
			// CASE : 1 : MERGE DISABLED
			// Resetting parameters if window is opening for 1st time
			if (param != CROP_C1)
			{
				// For cropping settings
				param = CROP_C1;
				colorRect[param] = true;
				refresh_crop[param] = false;
				// Cropped resizing settings
				resizeCroppedFactor[param] = 0;

				// Update some bools
				stitchWinAlign = false;
				croppedWindowON = false;
				
				// Destroying previous windows (For merge)
				mergeDisRunning = false;
			}
			

			// Ensuring availability of lines
			// when img_thread_cont_chan_1 = true;
			while ((chan_1_prev_1_filled || chan_1_prev_2_filled) == false && running_loops)
			{
				// If scanning needs to stop
				if (!img_thread_cont_chan_1 || !running_loops)
					break;
				waitKey(1);
				
			}
			
			if (!img_thread_cont_chan_1 || !running_loops)
				break;

			PrevStatus[0] = chan_1_prev_1_filled;
			PrevStatus[1] = chan_1_prev_2_filled;
			
			// CALIBRATION SIZE
			cv::Size size, sizeRAW;
			sizeRAW = cv::Size(postTriggerSamples, y_length);
			AR = float(horizontal) / float(vertical);
			PixelCALIB[0] = 2*round(y_length *(xAmp/yAmp));
			PixelCALIB[1] = 2*y_length;
			size.width = PixelCALIB[0];
			size.height = PixelCALIB[1];
			
			// UPDATING THE SCALE
			scale_down_preview = scale_down_preview_temp;
			int ResizeLocal = scale_down_preview;
			if (ResizeLocal < 1)
				ResizeLocal = 1;

			// GETTING THE CONTRAST, BRIGHTNESS & GAMMA INFO
			beta = beta_chan_1;
			alpha = alpha_chan_1;
			gamma = gamma_chan_1;
		
			// Starting of frame processing
			t1 = std::chrono::high_resolution_clock::now();

			// If array 1 is filled
			if (PrevStatus[0])
			{
				ProcessRawImg(
					chan_1_prev_array_1,
					imgDP8,
					imgDP16_temp,
					GMatProc,
					sizeRAW,
					size,
					alpha,
					beta,
					gamma,
					filledLines,
					NTU_PREVIEW,
					CHAN_1,
					stream);
			}
			t2 = high_resolution_clock::now();

			// IF FILTERS ARE ALLOWED
			ApplyFilter(CHAN_1, NTU_PREVIEW, imgDP8, stream);

			// DOWNLOADING THE PROCESSED IMAGE
			stream.waitForCompletion();
			imgDP16_temp.download(rawImage);

			// RESIZING THE IMAGE
			cuda::resize(imgDP8, imgDP8resized, imgDP8.size() / ResizeLocal, 0, 0, INTER_AREA, stream);

			// Converting GREYSCALE to RGB
			if (imgDP8resized.channels() == 1)
			{
				cv::cuda::cvtColor(imgDP8resized, imgDP8resizedRGB, cv::COLOR_GRAY2BGR, 0, stream);
			}
			else
			{
				imgDP8resizedRGB = imgDP8resized;
			}

			// DOWNLOADING THE PROCESSED IMAGE
			stream.waitForCompletion();
			imgDP8resizedRGB.download(resultResized);
			
			time_span = duration_cast<duration<double>>(t2 - t1);
			refreshRate = 1.0 / double(time_span.count());

			if (create_binning_folder == true && save_data_chan_1 == true)
			{
				folderName_binning = getCurrentTimeAsFolderName_binning();
				// Create the folder using system command
				std::string command_binning = MKDIR_CMD + folderName_binning;
				system(command_binning.c_str());
				create_binning_folder = false;
				
			}

			if (create_raw_folder == true && download_raw_image == true)
			{
				folderName = getCurrentTimeAsFolderName_raw();
				// Create the folder using system command
				std::string command = MKDIR_CMD + folderName;
				system(command.c_str());
				create_raw_folder = false;
			}

			if (save_data_chan_1)
			{
				if (img_count_temp <= frame_count)
				{
					// Define image file path
					std::string imagePath = folderName_binning + "\\" + std::to_string(img_count_temp) + ".tiff";
					// Convert std::string to a mutable char array
					char imagePathChar[256]; // Ensure enough space
					std::strcpy(imagePathChar, imagePath.c_str());
					sprintf(chan_1_filename, imagePathChar, img_count_temp);
					imwrite(chan_1_filename, resultResized);
					// Estimating Progress
					frame_saving_progress_chan_1 = (double(img_count_temp) / double(frame_count)) * 100;
					img_count_temp++;
				}
			}

			// Writing some info into the image
			char message[200];
			if (resultResized.rows > zoomValueLimit[1])
			{
				if (resultResized.cols > zoomValueLimit[0])
				{
					if (ResizeLocal != 1)		sprintf(message, "-%dX | %d x %d | CH 1", ResizeLocal, resultResized.cols, resultResized.rows);
					else if (ResizeLocal == 1)	sprintf(message, "Original | %d x %d | CH 1", resultResized.cols, resultResized.rows);
					putText(resultResized, message, Point2f(20, 20), FONT_HERSHEY_PLAIN, 1, Scalar(10, 215, 255), 2, LINE_AA, false);
				}
				else
				{
					if (ResizeLocal != 1)		sprintf(message, "-%dX", ResizeLocal);
					else if (ResizeLocal == 1)	sprintf(message, "1X");
					putText(resultResized, message, Point2f(20, 20), FONT_HERSHEY_PLAIN, 1, Scalar(10, 215, 255), 2, LINE_AA, false);
				}
			}

			// Writing some info into the image: FOV
			//sprintf(message, "FOV: %.1fum x %.1fum", round(FOV[0] * 10.0) / 10.0, round(FOV[1] * 10.0) / 10.0);
			//putText(resultResized, message, Point2f(10, resultResized.rows - 10), FONT_HERSHEY_PLAIN, 0.5, Scalar(255, 255, 0), 1, LINE_AA, false);

			// Writing some info into the image: Frame Rate
			sprintf(message, "FPS : %.2f", round(FrameRate * 100.0) / 100.0);
			putText(resultResized, message, Point2f(resultResized.cols - 110, resultResized.rows - 10), FONT_HERSHEY_PLAIN, 1, Scalar(255, 255, 0), 2, LINE_AA, false);

			// Draw the cross lines
			line(resultResized, Point2f(resultResized.cols / 2 - 5*(y_length/256) * 2 / ResizeLocal, resultResized.rows / 2), Point2f(resultResized.cols / 2 + 5 * (y_length / 256) * 2 / ResizeLocal, resultResized.rows / 2), Scalar(0, 255, 255), 2);
			line(resultResized, Point2f(resultResized.cols / 2, resultResized.rows / 2 - 5 * (y_length / 256) * 2 / ResizeLocal), Point2f(resultResized.cols / 2, resultResized.rows / 2 + 5 * (y_length / 256) * 2 / ResizeLocal), Scalar(0, 255, 255), 2);

			// DISPLAYING THE IMAGES
			/*if (cv::getWindowProperty("Channel 1 : ROI", WND_PROP_AUTOSIZE) != -1)
				rectangle(resultResized, P1[param], P2[param], Scalar(0, 0, 255), BoxThickness);*/

			
			namedWindow("Channel 1 : Preview : powered by OpenCV", WINDOW_AUTOSIZE);
			

			if (download_raw_image)
			{
				if (raw_img_count_temp <= raw_frame_count)
				{
					// CREATING THE NEW FILES
					//sprintf(chan_1_raw_filename, "C:\\NTU_miniScope_Imaging\\raw\\raw_test_%d.tiff", raw_img_count_temp);
					// Define image file path
					std::string imagePath = folderName + "\\" + std::to_string(raw_img_count_temp) + ".tiff";
					// Convert std::string to a mutable char array
					char imagePathChar[256]; // Ensure enough space
					std::strcpy(imagePathChar, imagePath.c_str());
					sprintf(chan_1_raw_filename, imagePathChar, raw_img_count_temp);
					imwrite(chan_1_raw_filename, rawImage);
					// Estimating Progress
					raw_frame_saving_progress_chan_1 = (double(raw_img_count_temp) / double(raw_frame_count)) * 100;
					raw_img_count_temp++;
				}
			}


			if (cv::getWindowProperty("Channel 1 : Preview : powered by OpenCV", WND_PROP_AUTOSIZE) == -1)
			{
				// Displaying the image
				imshow("Channel 1 : Preview : powered by OpenCV", resultResized);
				//setMouseCallback("Channel 1 : Preview : powered by OpenCV", mouseSelectionCROP, &param);
			}
			else
			{
				if (!busyRectDraw[param])
					imshow("Channel 1 : Preview : powered by OpenCV", resultResized);
			}
			
			waitKey(1);
			

			// Preserving synchronization | Updating bools
			if (PrevStatus[0])			chan_1_prev_1_filled = false;
			else if (PrevStatus[1])		chan_1_prev_2_filled = false;

			if (pause_Prev) {break;}
		}

		// DESTROYING THE WINDOW
		destroyWindow("Channel 1 : Preview : powered by OpenCV");
		//destroyWindow("Channel 1 : ROI");

		global_GPU_busy = false;

		// Updating scanning thread
		threadEnd[THREAD_CHAN_1] = true;
	}

	// Setting Channel Colors
	void NTUscope::setChannelColors()
	{
		// For Channel 1
		if (!activate_chan_1) { colorArrayBackUp[0] = 0; colorArrayBackUp[1] = 0; }
		else
		{
			if (chan_1_color == 0) { colorArrayBackUp[0] = 0; colorArrayBackUp[1] = 0; }
			else if (chan_1_color == 1) { colorArrayBackUp[0] = 1; colorArrayBackUp[1] = 0; }
			else if (chan_1_color == 2) { colorArrayBackUp[0] = 2; colorArrayBackUp[1] = 0; }
			else if (chan_1_color == 3) { colorArrayBackUp[0] = 3; colorArrayBackUp[1] = 0; }
			else if (chan_1_color == 4) { colorArrayBackUp[0] = 2; colorArrayBackUp[1] = 3; }
			else if (chan_1_color == 5) { colorArrayBackUp[0] = 1; colorArrayBackUp[1] = 2; }
			else if (chan_1_color == 6) { colorArrayBackUp[0] = 1; colorArrayBackUp[1] = 3; }
		}
	}

}