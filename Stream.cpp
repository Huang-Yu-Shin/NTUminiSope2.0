#include"NTUcore.h"

namespace NTU
{
	// CHANNEL 1 : DATA STREAMING
	void NTUscope::StreamChannel_1()
	{
		// Thread started
		threadEnd[THREAD_STREAM_C1] = false;

		// Channel number
		const int CHAN = CHAN_1;

		// Exiting if not allowed
		if (allow_scan == false || allow_configure == false)	exit(0);

		// OM NAMAH SHIVAY : Thread Priority Setting
		DWORD mask = 0 << 0;
		for (int i = 2; i <= NumCPU - 1; i++)
			mask = mask + (1 << i);
		auto ret = SetThreadAffinityMask(GetCurrentThread(), mask);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		// ENTERING MAIN LOOP
		while (file_stream_cont_chan_1)
		{
			// FOR bi_direction scanning
		/*	chan_1_refresh_avg_win = false;
			volatile bool first_frame_check = true;*/

			// RUNS UNTIL SCAN IS STOPPED
			while (file_stream_cont_chan_1)
			{
				// IF ARRAY 2 IS BEING FILLED : FORWARD FRAME
				// "image_array_1_filled == true" in Scan.cpp
				//if (image_array_1_filled)
				if (true)
				{
					// FRAME SAVING + PREVIEW
					//if (frames_saving_cond_chan_1 && !chan_1_frame_saving_status)
					if (true)
					{
						// Updating saved frame number
						//num_frames_saved_chan_1++;
						//first_frame_check = false;
						U32 lines_saved = 0;
						
						if (chan_1_prev_1_filled == false)
						{
							//while (frames_saving_cond_chan_1 && file_stream_cont_chan_1 )
							while (true)
							{
								if (lines_saved < lines_filled_array_2)
								{
									//fwrite(chan_1_image_array_2[lines_saved], 2, samplesPerRecord, chan_1_file);
									memcpy(chan_1_prev_array_1 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									//cout << lines_saved << ":" << *chan_1_image_array_2[lines_saved] << endl;
									lines_saved++;

								}
								else if (lines_saved < y_length && image_array_1_filled == false)
								{
									//fwrite(chan_1_image_array_2[lines_saved], 2, samplesPerRecord, chan_1_file);
									memcpy(chan_1_prev_array_1 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}

								// Exiting this loop : 1 frame save complete
								if (lines_saved == y_length)
								{
									// Updating the status
									chan_1_prev_1_filled = true;
									//cout << "One Frame Complete!!" << endl;
									break;
								}
							}
						}
						else if (chan_1_prev_2_filled == false)
						{
							while (file_stream_cont_chan_1 && frames_saving_cond_chan_1)
							{
								if (lines_saved < lines_filled_array_2)
								{
									fwrite(chan_1_image_array_2[lines_saved], 2, samplesPerRecord, chan_1_file);
									memcpy(chan_1_prev_array_2 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}
								else if (lines_saved < y_length && image_array_1_filled == false)
								{
									fwrite(chan_1_image_array_2[lines_saved], 2, samplesPerRecord, chan_1_file);
									memcpy(chan_1_prev_array_2 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}

								// Exiting this loop : 1 frame save complete
								if (lines_saved == y_length)
								{
									// Updating the status
									chan_1_prev_2_filled = true;
									break;
								}
							}
						}
						else
						{
							while (file_stream_cont_chan_1 && frames_saving_cond_chan_1)
							{
								if (lines_saved < lines_filled_array_2)
								{
									fwrite(chan_1_image_array_2[lines_saved], 2, samplesPerRecord, chan_1_file);
									lines_saved++;
								}
								else if (lines_saved < y_length)
								{
									fwrite(chan_1_image_array_2[lines_saved], 2, samplesPerRecord, chan_1_file);
									lines_saved++;
								}

								// Exiting this loop : 1 frame save complete
								if (lines_saved == y_length)	break;
							}
						}

						// Estimating Progress
						//frame_saving_progress_chan_1 = (double(img_count_temp) / double(frame_count)) * 100;
						//if (num_frames_saved_chan_1 >= totalSlice)
						//{
						//	// CLOSE THE DATA FILE
						//	//first_frame_check = true;
						//	lines_filled_array_2 = 0;
						//	fclose(chan_1_file);
						//	frames_saving_cond_chan_1 = false;
						//	frames_saving_init = false;
						//	num_frames_saved_chan_1 = 0;
						//}

						// UPDATING THE STATUS
						chan_1_frame_saving_status = true;
					}

					// ONLY PREVIEW
					else if (frames_saving_cond_chan_1 == false)
					{
						//first_frame_check = false;
						U32 lines_saved = 0;

						if (chan_1_prev_1_filled == false)
						{
							while (file_stream_cont_chan_1)
							{
								if (lines_saved < lines_filled_array_2)
								{
									memcpy(chan_1_prev_array_1 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}
								else if (lines_saved < y_length && image_array_1_filled == false)
								{
									memcpy(chan_1_prev_array_1 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}

								// Exiting this loop : 1 frame save complete
								if (lines_saved == y_length)
								{
									// Updating the status
									chan_1_prev_1_filled = true;
									break;
								}
							}
						}
						else if (chan_1_prev_2_filled == false)
						{
							while (file_stream_cont_chan_1)
							{
								if (lines_saved < lines_filled_array_2)
								{
									memcpy(chan_1_prev_array_2 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}
								else if (lines_saved < y_length && image_array_1_filled == false)
								{
									memcpy(chan_1_prev_array_2 + (lines_saved * samplesPerRecord), chan_1_image_array_2[lines_saved], samplesPerRecord * 2);
									lines_saved++;
								}

								// Exiting this loop : 1 frame save complete
								if (lines_saved == y_length)
								{
									// Updating the status
									chan_1_prev_2_filled = true;
									break;
								}
							}
						}
					}
				}
					
				//If need to shift resolution
				/*if (RShiftAllowed && HighSpeedPrev == false && RShiftStarted == false)
					break;*/
			
		}
	}

		// Thread ended
		threadEnd[THREAD_STREAM_C1] = true;
}
}