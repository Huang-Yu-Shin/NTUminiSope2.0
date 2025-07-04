#include"NTUcore.h"
#include "Additional.h"


namespace NTU
{
	int pixelTh_1;
	int pixelTh_2;
	int minXcrop;
	int minYcrop;
	bool L_hold[totalCROPfun], L_release[totalCROPfun];
	volatile bool refresh_crop[totalCROPfun];
	volatile bool busyRectDraw[totalCROPfun];
	volatile bool colorRect[totalCROPfun];
	volatile int resizeCroppedFactor[totalCROPfun];
	volatile int resizeWinFULLFactor[totalCROPfun];
	Mat rectDrawCrop[totalCROPfun];
	Point P1[totalCROPfun], P2[totalCROPfun], P1_TEMP, SdREF;
	int BoxThickness;
	Rect ROI_BOX[totalCROPfun];
	volatile bool slideCroppedROI;

	// Constructor
	NTUscope::NTUscope()
	{
		initStatus = 0;

		// CONTROLLED BY DEVELOPER : Chung-En Huang
		pauseScanEmergency = false;
		pauseSaveEmergency = false;
		pauseBinViewEmergency = false;
		pauseExportEmergency = false;
		pauseMemsEmergency = false;

		// Main Controls
		debug_window = true;
		allow_configure = true;
		allow_debug_window = true;
		allow_scan = true;
		allow_connect = true;
		allow_acquisition = true;
		allow_spiral_scan = true;
		allow_bidirection = true;
		allow_biDirectional_ext_trigger = true;
		allow_save = true;
		allow_alignment = true;
		allow_offsets = true;
		allow_filter = true;
		allow_binView = true;
		allow_export = true;

		// Scan manager
		scanManager = true;
		std::thread scanManagerTh(&NTUscope::HoldFreeScanning, this);
		scanManagerTh.detach();
		
		cout << "\n============= NTUScope executed!!!! =============\n\n";
	}

	// Initializing all
	void NTUscope::initializeAll()
	{
		// Memory allocation
		ScanStatus = (char*)malloc(200);

		// Get CPU cores
		NumCPU = std::thread::hardware_concurrency() - 1;

		//  Variables accessed in main GUI
		mems_thread_cont = false;
		mems_repositioned = true;
		mems_start_reOrient = false;
		MEMS_status = false;
		forward_move = false;
		backward_move = false;

		// ATS 9146 VARIABLES:
		engine_started = false;
		running_loops = false;

		// Get CPU cores
		//NumCPU = std::thread::hardware_concurrency() - 1;

		points_gap = 1;
		bi_direction_offset = 1;
		portnumber = 3;
		bi_direction = true;
		// DATA STREAMING : FRAME SAVING
		file_stream_cont_chan_1 = false;
		

		// Memory Allocation for file locations
		full_time_incl_path = (char*)malloc(2000);
		full_time_incl_path_chan_1 = (char*)malloc(2000);
		chan_1_filename = (char*)malloc(2000);
		chan_1_raw_filename = (char*)malloc(2000);

		// PREVIEWS : MERGING CHANNELS
		//chan_merge = true;
		red_already_filled = false;
		green_already_filled = false;
		blue_already_filled = false;
		chan_1_pushed = false;
		chan_1_color = COL_GREEN;

		// Filling data into the matrix
		lines_filled_array_1 = 0;

		// Preview : merging channels
		chan_1_refresh_avg_win = false;

		// FRAME AVERAGING SETTINGS
		frame_averaging = false;
		num_avg_frame_new = 1;
		allow_averaging = false;

		// FILE SAVING SETTINGS
		chan_1_frame_saving_status = true;
		frames_saving_cond_chan_1 = false;
		num_frames_saved_chan_1 = 0;
		lines_averaged_array_2 = 0;
		frame_saving_progress_chan_1 = 0;
		img_count_temp = 1;
		save_data_chan_1 = false;
		frames_saving_init = false;
		frame_saving_aborted = false;
		activate_frame_savinng_confirm = false;
		received_first_frame = false;

		// FRAME AVERAGING SETTINGS
		avg_frames_done_chan_1 = 0;
		num_avg_frame_temp = 1;
		num_avg_frame = 1;

		// 3D stage settings
		count_completed_frames = 0;
		lines_averaged_array_1 = 0;
		lines_filled_array_2 = 0;
		num_frames_summed = 0;
		image_array_2_filled = false;
		filled_array_2 = false;

		// PREVIEW DISPLAY SETTINGS
		img_thread_cont_chan_1 = false;
		global_GPU_busy = false;
		first_avg_frame = false;
		mems_keep_running = false;
		pause_Prev = false;
		frame_count = 1;

		countMoveAction = 0;
		calibration_value = 128;
		total_points_before_trigger = 0;
		prev_total_points_before_trigger = 0;

		xPosition = 0;
		yPosition = 0;
		xOffset = 0;
		yOffset = 0;
		xAmp = 0;
		yAmp = 0;
		headMount_count = 0;
		mSec = 0;
		horizontal = 0;
		vertical = 0;
		AR = 0; 

		y_length = 0;
		prev_Y_length = 0;
		prev_sampPerRec = 0;

		// Default settings
		lineTime = 5e-4;
		chan_merge = true;

		// CROPPING AND ZOOMING WINDOWS
		for (int i = 0; i < totalCROPfun; i++)
		{
			// Croping image
			L_hold[i] = false;
			L_release[i] = false;
			refresh_crop[i] = false;
			busyRectDraw[i] = false;
			colorRect[i] = false;

			// Resizing cropped image
			resizeCroppedFactor[i] = 0;
		}
		download_raw_image = false;
		raw_frame_saving_progress_chan_1 = 0;
		raw_frame_count = 0;
		raw_img_count_temp = 1;
		scale_down_preview = 0;
		scale_down_preview_temp = 0;
		scale_down_prev_bin_read_temp = 0;
		// Other settings
		pixelTh_1 = 0;
		pixelTh_2 = 0;
		minXcrop = 10;
		minYcrop = 10;
		bin_export_16BIT = false;
		bytesPerBuffer = 0;
		samplesPerRecord = 0;
		constructorExecutedGUI = false;
		postTriggerSamples = 0;
		alpha_chan_1 = 0;
		beta_chan_1 = 0;
		gamma_chan_1 = 0;

		// Filter ID memory allocation
		for (int i = 0; i <= 3; i++)
		{
			filtersID[i] = (int*)malloc(sizeof(int) * 200);
			filtersID_bin[i] = (int*)malloc(sizeof(int) * 200);
		}
		// Reseting FilterIDs
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 100; j++)
			{
				filtersID[i][j] = 0;
				filtersID_bin[i][j] = 0;
			}
		}

		// IMAGE ENHANCEMENT + FILTERS
		filters_allowed = true;

		// ALL FILTERS INITIALIZED PARAMETERS
		for (int i = 0; i <= 3; i++)
		{
			// GAUSSIAN FILTER
			//activate_gaussian_bin_file[i] = true;
			gaussian_kernel_bin_file[i] = 5;
			gaussian_sigmaH_bin_file[i] = 10;
			gaussian_sigmaV_bin_file[i] = 10;

			// MEDIAN BLUR FILTER
			//activate_median_bin_file[i] = true;
			median_kernel_bin_file[i] = 3;

			// HISTOGRAM EQUALIZATION : CLAHE
			//activate_CLAHE_bin_file[i] = true;
			CLAHE_clip_limit_bin_file[i] = 3;
			CLAHE_tile_grid_H_bin_file[i] = 20;
			CLAHE_tile_grid_V_bin_file[i] = 20;
		}		


		apply_gaussian = false;
		apply_median = false;
		apply_CLAHE = false;

		refreshRate = 0;

	}

	


	// SOME PERMISSION FUNCTIONS
	bool NTUscope::Go_allow_configure() { return allow_configure; }
	//bool NTUscope::Go_allow_debug_window() { return allow_debug_window; }
	bool NTUscope::Go_allow_connect() { return allow_connect; }
	bool NTUscope::Go_allow_acquisition() { return allow_acquisition; }
	bool NTUscope::Go_allow_spiral_scan() { return allow_spiral_scan; }
	bool NTUscope::Go_allow_biDirectional() { return allow_bidirection; }
	bool NTUscope::Go_allow_biDirectional_ext_trigger() { return allow_biDirectional_ext_trigger; }
	//bool NTUscope::Go_allow_raster_scan() { return allow_raster_scan; }
}