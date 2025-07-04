#include "NTUcore.h"

namespace NTU
{
	// INITIALIZE FRAME SAVING
	bool NTUscope::initialize_frame_saving(
		char* brief_detail,
		char* full_date_incl_path)
	{
		//CREATING NEW FOLDER : Current date & time based on current system
		char hour[10];
		char min[10];
		char sec[10];
		time_t now = time(0);
		tm* ltm = localtime(&now);
		if (ltm->tm_hour < 10)	sprintf(hour, "0%d", ltm->tm_hour);
		else					sprintf(hour, "%d", ltm->tm_hour);
		if (ltm->tm_min < 10)	sprintf(min, "0%d", ltm->tm_min);
		else					sprintf(min, "%d", ltm->tm_min);
		if (ltm->tm_sec < 10)	sprintf(sec, "0%d", ltm->tm_sec);
		else					sprintf(sec, "%d", ltm->tm_sec);

		//Setting file parent path
		sprintf(full_time_incl_path, "");
		if (strlen(brief_detail) == 0)
		{
			sprintf(full_time_incl_path, "%s%s-%s-%s", full_date_incl_path, hour, min, sec);
		}
		else
		{
			sprintf(full_time_incl_path, "%s%s-%s-%s  %s", full_date_incl_path, hour, min, sec, brief_detail);
		}
		std::string full_time_incl_path_stdString = full_time_incl_path;
		CreateDirectoryA(full_time_incl_path_stdString.c_str(), NULL);
		char image_filename_info[500];
		double write_file_info_array[MAX_INFO_DATA];

		//Creating directory for Channel 1
		sprintf(full_time_incl_path_chan_1, "%s%s", full_time_incl_path, "\\Channel_1");
		std::string full_time_incl_path_chan_1_stdString = full_time_incl_path_chan_1;
		CreateDirectoryA(full_time_incl_path_chan_1_stdString.c_str(), NULL);

		// CREATING THE NEW FILES
		sprintf(chan_1_filename, "%s\\CHAN_1", full_time_incl_path_chan_1);
		sprintf(image_filename_info, "%s\\info", full_time_incl_path_chan_1);
		chan_1_file = fopen(chan_1_filename, "wb");

		return true;
	}

	// ABORTING FRAME SAVING
	bool NTUscope::abort_frame_saving(
		float xAmp,
		float yAmp,
		bool internal_clock,
		bool external_clock,
		double internal_clock_freq,
		double external_clock_freq)
	{
		// Aborting rapid scan handler
		TERMINATE_RAPID = true;

		// Exiting if not allowed
		if (allow_save == false)	exit(0);

		// Required variables
		char image_filename_info[500];
		double write_file_info_array[MAX_INFO_DATA];
		//double level_1, level_2, beta, alpha, gamma, INRange, BandWLimit, level2Min, level2Max, colorID, FrameSaved;

		// CREATING CHAN DIRECTORIES AND INFO FILES
		for (int chanIndex = 0; chanIndex < 4; chanIndex++)
		{
			// CHANNEL 1 SPECIFIC
			if (chanIndex == 0)
			{
				if (frames_saving_cond_chan_1)
				{
					// Location of info file
					sprintf(image_filename_info, "%s\\info", full_time_incl_path_chan_1);

					// Temporary data
					/*level_1 = remap_level_1_chan_1;
					level_2 = remap_level_2_chan_1;
					beta = beta_chan_1;
					alpha = alpha_chan_1;
					gamma = gamma_chan_1;
					INRange = chan_1_input_range;
					BandWLimit = chan_1_bandWidth_state;
					level2Min = level_2_min_chan_1;
					level2Max = level_2_max_chan_1;
					colorID = chan_1_color;*/

					// Stopping the data storing process
					num_frames_saved_chan_1 = totalSlice;
					frame_saving_progress_chan_1 = 100;
					frames_saving_init = 0;
					frame_saving_aborted = true;
				}
				else
				{
					continue;
				}
			}

			// CREATING THE INFO FILE
			int INFO_COUNT = 0;
			write_file_info_array[INFO_COUNT++] = -7;
			write_file_info_array[INFO_COUNT++] = chanIndex + 1;
			write_file_info_array[INFO_COUNT++] = samplesPerRecord;
			write_file_info_array[INFO_COUNT++] = y_length;
			write_file_info_array[INFO_COUNT++] = num_avg_frame_new;
			write_file_info_array[INFO_COUNT++] = xAmp;
			write_file_info_array[INFO_COUNT++] = yAmp;
			write_file_info_array[INFO_COUNT++] = internal_clock;
			write_file_info_array[INFO_COUNT++] = external_clock;
			write_file_info_array[INFO_COUNT++] = internal_clock_freq;
			write_file_info_array[INFO_COUNT++] = external_clock_freq;
			write_file_info_array[INFO_COUNT++] = bi_direction;
			//write_file_info_array[INFO_COUNT++] = rapidScanEnable;
			//write_file_info_array[INFO_COUNT++] = colorID;

			// WRITING THE FILE
			image_file_info = fopen(image_filename_info, "wb");
			fwrite(write_file_info_array, sizeof(double), MAX_INFO_DATA, image_file_info);
			fclose(image_file_info);
		}

		// Returning result
		return true;
	}
}