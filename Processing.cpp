#include "NTUcore.h"

namespace NTU
{
	// RAW DATA PROCESSING | 16-BIT INPUT | 8-BIT OUTPUT
	void NTUscope::ProcessRawImg(
		U16* U16ARRAY,
		GpuMat& IMGDP8,
		GpuMat& raw_IMGDP8,
		GpuMat* GMatProc,
		cv::Size& sizeRaw,
		cv::Size& size,
		double& alpha,
		int& beta,
		double& gamma,
		bool* filledLines,
		int MODE,
		int CHAN,
		Stream& streamCUDA)
	{
		if (sizeRaw.width <= 0 || size.width <= 0)
			return;

		// Declaring the MAT object
		GpuMat S_COPY[2];
		GpuMat IMGD16 = GMatProc[5];
		GpuMat tempSub = GMatProc[6];
		GpuMat imgDP16_temp = GMatProc[7];
		GpuMat RES_UP = GMatProc[8];
		S_COPY[0] = GMatProc[9];
		S_COPY[1] = GMatProc[10];

		// Getting the buffer data
		Mat loaded_img16(sizeRaw.height, sizeRaw.width, CV_16UC1, U16ARRAY);

		// UPLOADING TO NVIDIA GPU
		IMGD16.upload(loaded_img16, streamCUDA);

		// Adjusting alignments (BiDirectional scanning)
		//if ((MODE == NTU_BINVIEW || MODE == NTU_EXPORT) && biDirection_bin_file == true && BiD_align_bin != 0)
		if (true)
		{
			// FILLING THE 16bit MAT object : ODD LINES L<---R
			int BiD_align_bin_Local = bi_direction_offset;
			if (bi_direction_offset > 0)
			{
				for (int i = 1; i < sizeRaw.height; i = i + 2)
				{
					//IMGD16(cv::Rect(BiD_align_bin_Local, i, sizeRaw.width - BiD_align_bin_Local, 1)) = IMGD16(cv::Rect(0, i, sizeRaw.width - BiD_align_bin_Local, 1));
					IMGD16(cv::Rect(0, i, sizeRaw.width - BiD_align_bin_Local, 1)).cv::cuda::GpuMat::copyTo(IMGD16(cv::Rect(BiD_align_bin_Local, i, sizeRaw.width - BiD_align_bin_Local, 1)), streamCUDA);
					//IMGD16(cv::Rect(0, i, BiD_align_bin_Local, 1)).cv::cuda::GpuMat::setTo(Scalar::all(0), streamCUDA);
				}
			}
			else if (bi_direction_offset < 0)
			{
				for (int i = 1; i < sizeRaw.height; i = i + 2)
				{
					//IMGD16(cv::Rect(0, i, sizeRaw.width - abs(BiD_align_bin_Local), 1)) = IMGD16(cv::Rect(abs(BiD_align_bin_Local), i, sizeRaw.width - abs(BiD_align_bin_Local), 1));
					IMGD16(cv::Rect(abs(BiD_align_bin_Local), i, sizeRaw.width - abs(BiD_align_bin_Local), 1)).cv::cuda::GpuMat::copyTo(IMGD16(cv::Rect(0, i, sizeRaw.width - abs(BiD_align_bin_Local), 1)), streamCUDA);
					//IMGD16(cv::Rect(sizeRaw.width - abs(BiD_align_bin_Local), i, abs(BiD_align_bin_Local), 1)).cv::cuda::GpuMat::setTo(Scalar::all(0), streamCUDA);
				}
			}
		}

		// Exclude last column (boundary)
		streamCUDA.waitForCompletion();
		IMGD16 = IMGD16(cv::Rect(0, 0, IMGD16.cols - 1, IMGD16.rows));

		// INITIAL WINDOWING | Zero-negVolt | 32768-0
		cv::cuda::subtract(65535, IMGD16, IMGD16, noArray(), -1, streamCUDA);				// 32768-65535
		cv::cuda::threshold(IMGD16, IMGD16, 65535, 0, cv::THRESH_TRUNC, streamCUDA);		// 32768-65535 (if high = 65535)
		cv::cuda::subtract(IMGD16, 32768, IMGD16, noArray(), -1, streamCUDA);				// 0-32768 (if low = 32768)
		cv::cuda::multiply(IMGD16, 2.0, IMGD16, 1.0, -1, streamCUDA);						// 0-65535 (if low = 32768, high = 65535)

		raw_IMGDP8 = IMGD16;

		// Aspect ratio calibration & Data binning if possible
		if (sizeRaw.width > size.width)
		{
			int binPixel_x = int(sizeRaw.width / size.width) + 1;
			int binPixel_y = int(size.height / sizeRaw.height);
			cuda::resize(IMGD16, RES_UP, cv::Size((size.width * binPixel_x), (size.height * 1 / binPixel_y)), 0, 0, INTER_AREA, streamCUDA);
			//cuda::resize(RES_UP, IMGD16, RES_UP.size() / 1, 0, 0, INTER_AREA, streamCUDA);
			cuda::resize(IMGD16, IMGD16, size, 0, 0, INTER_CUBIC, streamCUDA);
		}
		else
		{
			cuda::resize(IMGD16, IMGD16, size, 0, 0, INTER_CUBIC, streamCUDA);
		}

		// CONVERT TO 8 BIT or 16 BIT IMAGE
		if (bin_export_16BIT == true && MODE == NTU_EXPORT)
		{
			IMGDP8 = IMGD16;
		}
		else
		{
			IMGD16.cv::cuda::GpuMat::convertTo(IMGDP8, CV_8UC1, 1 / 257.0, streamCUDA);
		}

		if (bin_export_16BIT == false && MODE != NTU_EXPORT)
		{
			// GAMMA CORRECTION IN GPU
			Mat LUT_matrix(1, 256, CV_8UC1);
			uchar* pointer = LUT_matrix.ptr();
			for (int i = 0; i < 256; i++)
			{
				pointer[i] = (int)(pow((double)(i / 255.0), gamma) * 255.0);
			}
			cv::Ptr<cv::cuda::LookUpTable> cudaLookUpTable = cv::cuda::createLookUpTable(LUT_matrix);
			cudaLookUpTable->transform(IMGDP8, IMGDP8, streamCUDA);

			// APPLYING BRIGHTNESS AND CONTRAST IN NVIDIA GPU
			IMGDP8.cv::cuda::GpuMat::convertTo(IMGDP8, -1, alpha, beta - 150, streamCUDA);
		}

	}

	// APPLY FILTERS | 8-BIT INPUT
	void NTUscope::ApplyFilter(
		int CHAN,
		int MODE,
		GpuMat& IMGDP8,
		Stream& streamCUDA)
	{
		// Check if 16-bit format is enabled

		// GAUSSIAN BLUR FILTER: FOR FILTERS
		int GS_kernel = 0, GS_sigmaH = 0, GS_sigmaV = 0;

		// MEDIAN BLUR FILTER
		int MD_kernel = 0;

		// HISTOGRAM EQUALIZATION : CLAHE
		int CL_tile_grid_H = 0, CL_tile_grid_V = 0, CL_clip_limit = 0;

		// Getting the filter parameters
		if (MODE == NTU_PREVIEW)
		{
			// GAUSSIAN BLUR FILTER
			//apply_gaussian = activate_gaussian_bin_file[CHAN];
			GS_kernel = gaussian_kernel_bin_file[CHAN];
			GS_sigmaH = gaussian_sigmaH_bin_file[CHAN];
			GS_sigmaV = gaussian_sigmaV_bin_file[CHAN];

			// MEDIAN BLUR FILTER
			//apply_median = activate_median_bin_file[CHAN];
			MD_kernel = median_kernel_bin_file[CHAN];

			// HISTOGRAM EQUALIZATION : CLAHE
			//apply_CLAHE = activate_CLAHE_bin_file[CHAN];
			CL_tile_grid_H = CLAHE_tile_grid_H_bin_file[CHAN];
			CL_tile_grid_V = CLAHE_tile_grid_V_bin_file[CHAN];
			CL_clip_limit = CLAHE_clip_limit_bin_file[CHAN];
		}
		else if (MODE == NTU_BINVIEW || MODE == NTU_EXPORT)
		{
			// GAUSSIAN BLUR FILTER
			//apply_gaussian = activate_gaussian_bin_file[CHAN];
			GS_kernel = gaussian_kernel_bin_file[CHAN];
			GS_sigmaH = gaussian_sigmaH_bin_file[CHAN];
			GS_sigmaV = gaussian_sigmaV_bin_file[CHAN];

			// MEDIAN BLUR FILTER
			//apply_median = activate_median_bin_file[CHAN];
			MD_kernel = median_kernel_bin_file[CHAN];

			// HISTOGRAM EQUALIZATION : CLAHE
			//apply_CLAHE = activate_CLAHE_bin_file[CHAN];
			CL_tile_grid_H = CLAHE_tile_grid_H_bin_file[CHAN];
			CL_tile_grid_V = CLAHE_tile_grid_V_bin_file[CHAN];
			CL_clip_limit = CLAHE_clip_limit_bin_file[CHAN];

		}

		// IF FILTERS ARE ALLOWED : APPLY NOW
		if (filters_allowed)
		{
			// APPLYING ACTIVATED FILTERS AS PER THE ENTERED SEQUENCE
			// GAUSSIAN BLUR FILTER
			if (apply_gaussian)
			{
				cv::Size kernelGaussian(GS_kernel, GS_kernel);
				Ptr<cuda::Filter> gaussian = cuda::createGaussianFilter(IMGDP8.type(), IMGDP8.type(), kernelGaussian, GS_sigmaH, GS_sigmaV, BORDER_DEFAULT, -1);
				gaussian->apply(IMGDP8, IMGDP8);
			}
			
			// MEDIAN BLUR FILTER
			if (apply_median)
			{
				// For 16-bit image
				if (IMGDP8.depth() == 2)
				{
					Mat tempImg, tempResult;
					IMGDP8.download(tempImg);
					cv::medianBlur(tempImg, tempResult, MD_kernel);
					IMGDP8.upload(tempResult);
				}

				// For 8-bit image
				else
				{
					GpuMat temp = IMGDP8.clone();
					cv::Ptr<cv::cuda::Filter> median = cv::cuda::createMedianFilter(IMGDP8.type(), MD_kernel);
					median->apply(temp, IMGDP8);
				}
			}

			// HISTOGRAM EQUALIZATION : CLAHE
			if (apply_CLAHE)
			{
				cv::Size clahe_tile_grid(CL_tile_grid_H, CL_tile_grid_V);
				Ptr<cuda::CLAHE> clahe = cuda::createCLAHE(CL_clip_limit, clahe_tile_grid);
				clahe->apply(IMGDP8, IMGDP8);
			}
			
				
		}
	}
}