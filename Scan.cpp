#include"NTUcore.h"

namespace NTU
{
	// Initiate Scan
	void NTUscope::initiateScan()
	{
		// Exiting if not allowed
		if (allow_scan == false ||
			allow_configure == false)
			exit(0);

		// For merged previews
		chan_1_pushed = false;
		mergeDisRunning = false;
		// Start acquisition
		ACQ_finished = false;
		// IF ENGINE NOT STARTED YET
		engine_started = false;
		forceTrigRequest = false;
		int waitMS = 10;	

		// OTHER IMAGE PROCESSING + PREVIEWING THREADS
		if (activate_chan_1)
		{
			// ACTIVATING IMAGE PREVIEWING THREAD
			if (!img_thread_cont_chan_1)
			{
				// Starting Preview Chan 1
				img_thread_cont_chan_1 = true;
				std::thread prevThread_1(&NTUscope::PrevChannel_1, this);
				prevThread_1.detach();
				Sleep(waitMS);
				//cout << "THREAD_CH1" << endl;
			}
		}

		// Waking Background Data Streamers
		if (img_thread_cont_chan_1)
		{
			file_stream_cont_chan_1 = true;
			std::thread streamThread_C1(&NTUscope::StreamChannel_1, this);
			streamThread_C1.detach();
			Sleep(waitMS);
			//cout << "THREAD_img_CH1" << endl;
		}

		// Starting main scan
		std::thread startMain(&NTUscope::start_scan, this);
		startMain.detach();
		Sleep(100);


		// Buffer handling thread
		//std::thread bufferHandlerThread(&UFOprime::bufferHandler, this);
		//bufferHandlerThread.detach();
		//Sleep(100);
		//std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	// Hold the scanning process [for changing configs]
	// While construcror ends, the constructorExecutedGUI == true
	void NTUscope::HoldFreeScanning()
	{
		const int waitMS = 10;
		while (scanManager == true)
		{
			// No need to manage scanning!
			if (constructorExecutedGUI == false || scanAlive == false)
			{
				scanHOLD = false;
				scanFREE = false;
			}
			
			// Hold the scanning
			if (scanHOLD && constructorExecutedGUI && scanAlive)
			{
				// Hold [if running]
				if (running_loops == true)
				{
					// Tracking the threads
					threadEnd[THREAD_SCAN] = false;
					threadEnd[THREAD_MEMS] = false;
					threadEnd[THREAD_CHAN_1] = false;
					threadEnd[THREAD_STREAM_C1] = false;

					// Freezing scan
					running_loops = false;
					//while (ACQ_finished == false)
					//{
						//Sleep(waitMS);
					//}
					
					// Ensure no scanning thrad is active!
					while (!threadEnd[THREAD_SCAN])
						Sleep(waitMS);
					cout << "\n--Scan" << endl;

					while (!threadEnd[THREAD_MEMS])
						Sleep(waitMS);
					cout << "--MEMS" << endl;

					if (img_thread_cont_chan_1)
						while (!threadEnd[THREAD_STREAM_C1])
							Sleep(waitMS);
					cout << "--S1" << endl;

					if (activate_chan_1)
						while (!threadEnd[THREAD_CHAN_1])
							Sleep(waitMS);
					cout << "--C1" << endl;

					// Wait a few milliseconds for stability
					Sleep(100);
				}

				// Done!
				scanHOLD = false;
			}

			// Free the scanning
			if (scanFREE && constructorExecutedGUI && scanAlive)
			{
				// Free [if on hold]
				if (running_loops == false)
				{
					// Wait a few milliseconds for stability
					Sleep(100);
					// Tracking the threads
					threadEnd[THREAD_SCAN] = true;
					threadEnd[THREAD_MEMS] = true;
					threadEnd[THREAD_CHAN_1] = true;
					threadEnd[THREAD_STREAM_C1] = true;

					// Start scanning
					//mems_start_reOrient = false;
					//mems_repositioned = true;
					running_loops = true;
					std::thread scanInitThread(&NTUscope::initiateScan, this);
					scanInitThread.detach();
					Sleep(waitMS);
					//initiateScan();
					
					// Wait until scan is initiated
					while (threadEnd[THREAD_SCAN])
						Sleep(waitMS);
					cout << "\nScan" << endl;
					
					while (threadEnd[THREAD_MEMS])
						Sleep(waitMS);
					cout << "--MEMS" << endl;

					if (activate_chan_1)
						while (threadEnd[THREAD_CHAN_1])
							Sleep(waitMS);
					cout << "--C1" << endl;
					
					if (img_thread_cont_chan_1)
						while (threadEnd[THREAD_STREAM_C1])
							Sleep(waitMS);
					cout << "--S1" << endl;
				}
				
				// Done!
				scanFREE = false;
			}

			// Wait a few milliseconds
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		// Done!
		return;
		
	}

	// Align with FAST axis
	bool NTUscope::startCapture(bool high_speed, bool ultra_fast, HANDLE boardHandle)
	{
		if (MEMS_status)
		{
			// IF ENGINE NOT STARTED YET
			if (engine_started == false)
				startTimePoint = std::chrono::high_resolution_clock::now();

			// MAIN LOOP
			//while (running_loops)
			//{
			//	DAQmxReadDigitalScalarU32(taskHandle_trigger_rising, 10.0, &trigger_state, NULL);
			//	if (trigger_state == 64)
			//	{
			//		// RISING Edge is detected
			//		while (running_loops)
			//		{
			//			DAQmxReadDigitalScalarU32(taskHandle_trigger_rising, 10.0, &trigger_state, NULL);
			//			if (trigger_state == 0)
			//			{
			// FALLING Edge is detected
			// BEGIN: ACQUISITION : STANDBY
			retCode = AlazarStartCapture(boardHandle);
			if (retCode != ApiSuccess)
				return false;

			// Go out of the while loops
			if (engine_started == false)
				engine_started = true;
			return true;
					/*	}
					}
				}*/

			// Checking for timeOUT
			if (engine_started == false)
			{
				if (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - startTimePoint).count() > (ATS_WAIT_MS * 1000))
					return false;
			}
		//	}
		}
	}

	// Force triggering thread
	void NTUscope::forceTrigThread()
	{
		// OM NAMAH SHIVAY : Thread Priority Setting
		DWORD mask = 0 << 0;
		for (int i = 2; i <= NumCPU - 1; i++)
			mask = mask + (1 << i);
		auto ret = SetThreadAffinityMask(GetCurrentThread(), mask);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		// Keep running until scanning is going on
		while (running_loops && false)
		{
			if (forceTrigRequest == true)
			{
				//startCapture(high_speed, ultra_fast, boardHandle);
				//forceTrigRequest = false;
			}
		}
	}

	// Scanning Start Thread
	void NTUscope::start_scan()
	{
		// Updating scanning thread
		threadEnd[THREAD_SCAN] = false;

		// Exiting if not allowed
		if (allow_scan == false ||
			allow_configure == false)
			exit(0);

		// Emergency pause
		while (pauseScanEmergency) { Sleep(5); }

		// OM NAMAH SHIVAY : Thread Priority Setting
		DWORD mask = 0 << 0;
		for (int i = 2; i <= NumCPU - 1; i++)
			mask = mask + (1 << i);
		auto ret = SetThreadAffinityMask(GetCurrentThread(), mask);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		// MAKING THE AVERAGING ARRAYS ZERO
		if (frame_averaging)
		{
			for (int width = 0; width < y_length; width++)
			{
				for (int length = 0; length < samplesPerRecord; length++)
				{
					chan_1_image_array_avg[width][length] = 0;
				}
			}
			avg_frames_done_chan_1 = 0;
			count_completed_frames = 0;
		}

		// Updating calibration size
		PixelCALIB[0] = calibration_value;
		PixelCALIB[1] = y_length;

		// Initialization
		num_frames_summed = 0;
		AcqTime = 0.0;
		FrameRate = 0.0;

		// Create a data file if required
		FILE* fpData = NULL;

		// ENTERING MAIN LOOP
		while (running_loops)
		{
			// Updating Scan Status
			// Start acquisition 
			ACQ_finished = false;
			engine_started = false;
			forceTrigRequest = false;
			sprintf(ScanStatus, "OK");

			// Reseting Preview Window Bools
			chan_1_prev_1_filled = false;
			chan_1_prev_2_filled = false;
			active_buffer_chan_1 = true;
	
			// Updating some bools
			image_array_1_filled = true;
			image_array_2_filled = false;
			one_frame_completed = false;
			prev_total_points_before_trigger = 0;
			num_frames_summed = 0;
			

			// FIXING TIME STAMP
			AlazarResetTimeStamp(boardHandle, TIMESTAMP_RESET_ALWAYS);

			// TODO: Specify the acquisition length in seconds
			//double acquisitionLength_sec = 5.;

			// Wait for each buffer to be filled, process the buffer, and re-post it to the board
			U32 buffersCompleted = 0;

			if (saveData)
			{
				fpData = fopen("data_bin.txt", "wb");
				if (fpData == NULL)
				{
					printf("Error: Unable to create data file -- %u\n", GetLastError());
					saveData = false;
				}
			}

			
			if (true)
			{
				// Configure the board to make an NPT AutoDMA acquisition
				retCode = AlazarBeforeAsyncRead(boardHandle, channel_Mask, -(long)preTriggerSamples,
					samplesPerRecord, recordPerBuffer, 0x7FFFFFFF, admaFlag);
				if (retCode != ApiSuccess)
				{
					// Updating Scan Status
					sprintf(ScanStatus, "Error: AlazarBeforeAsyncRead Failed --- %s", AlazarErrorToText(retCode));
					if (debug_window)	_cprintf("Error: AlazarBeforeAsyncRead Failed_scan --- %s\n", AlazarErrorToText(retCode));

					// Rearming ATS for Acquisition
					reArm();
					NTUscope::TerminateAll();
					running_loops = false;
					return;
				}

				// Add the buffers to a list of buffers available to be filled by the board
				for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT); bufferIndex++)
				{
					U16* pBuffer = BufferArray[bufferIndex];
					retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
					if (retCode != ApiSuccess)
					{
						// Updating Scan Status
						sprintf(ScanStatus, "Error: AlazarPostAsyncBuffer Failed1 --- %s", AlazarErrorToText(retCode));
						if (debug_window)	_cprintf("Error: AlazarPostAsyncBuffer Failed1 --- %s\n", AlazarErrorToText(retCode));

						// Rearming ATS for Acquisition
						reArm();
						NTUscope::TerminateAll();
						running_loops = false;
						return;
					}
				}

				// Starting or Resetting the stopwatch : for frame rate
				starting_time = std::chrono::high_resolution_clock::now();

				// BEGIN: ACQUISITION : STANDBY
				retCode = AlazarStartCapture(boardHandle);
				if (retCode != ApiSuccess)
				{
					// Updating Scan Status
					sprintf(ScanStatus, "Error: Capture Failed --- %s", AlazarErrorToText(retCode));
					if (debug_window)	_cprintf("Error: Capture Failed --- %s\n", AlazarErrorToText(retCode));

					// Rearming ATS for Acquisition
					reArm();
					NTUscope::TerminateAll();
					running_loops = false;
					return;
				}
				
				// First frame requesting
				//forceTrigRequest = true;

				// ENTERING TO THE REAL SCANNING LOOP : SMALLER IMAGE : HIGH FRAME RATE
				INT64 bytesTransferred = 0;
				U32 startTickCount = GetTickCount();
		
				lines_filled_array_2 = 0;
				
				while (buffersCompleted < buffersPerAcquisition)
				{	
					U32 timeout_ms = 5000;

					// Wait for the buffer at the head of the list of available buffers to be filled by the board
					bufferIndex = buffersCompleted % BUFFER_COUNT;
					U16* pBuffer = BufferArray[bufferIndex];

					// WAITING FOR THE CURRENT BUFFER TO BE FILLED
					retCode = AlazarWaitAsyncBufferComplete(boardHandle, pBuffer, 5000);

					if (retCode != ApiSuccess)
					{
						// Updating Scan Status
						sprintf(ScanStatus, "Error: AlazarWaitAsyncBufferComplete --- %s", AlazarErrorToText(retCode));
						if (debug_window)	_cprintf("Error: AlazarWaitAsyncBufferComplete --- %s\n", AlazarErrorToText(retCode));

						// Rearming ATS for Acquisition
						reArm();
						NTUscope::TerminateAll();
						running_loops = false;
						return;
					}

					//U32 record = buffersCompleted;

					// The buffer is full and has been removed from the list of buffers available for the board
					buffersCompleted++;

					bytesTransferred += bytesPerBuffer;

					if (saveData)
					{
						// Write record to file
						size_t bytesWritten = fwrite(pBuffer, sizeof(BYTE), bytesPerBuffer, fpData);
						if (bytesWritten != bytesPerBuffer)
						{
							printf("Error: Write buffer %u failed -- %u\n", buffersCompleted,
								GetLastError());
						}
					}

					// Process sample data in this buffer
					// 0x0000 represents a negative full scale input signal
					// 0x8000 represents a ~0V signal
					// 0xFFFF represents a positive full scale input signal
					// Getting the index control for the buffer
					U16* pRecord = pBuffer;

					// IF UNIRECTIONAL SCANNING
					if (uni_Directional)
					{
						for (U32 record = 0; (record < recordPerBuffer); record++)
						{
							for (U32 samples = 0; samples < samplesPerRecord; samples++)
							{
								chan_1_image_array_2[lines_filled_array_2][samples] = *pRecord++;
							}
							// Updating number of lines acquired
							lines_filled_array_2++;
						}
					}

					// IF BIDIRECTIONAL SCANNING
					if (bi_direction)
					{
						// Extracting the EVEN and ODD lines
						for (U32 record = 0; (record < recordPerBuffer); record++)
						{
							// EVEN LINES -------->
							if (record % 2 == 0)
							{
								for (U32 samples = 0; samples < samplesPerRecord; samples++)
								{
									chan_1_image_array_2[lines_filled_array_2][samples] = *pRecord++;
								}
								// Updating number of lines acquired
								lines_filled_array_2++;
							}
							// ODD LINES <--------
							else if (record % 2 == 1)
							{
								// Adjusting the offset
								points_gap = bi_direction_offset;

								// Skipping the detected gap
								pRecord += 4 * points_gap;

								for (U32 samples = samplesPerRecord - 1; samples >= points_gap; samples--)
								{
									chan_1_image_array_2[lines_filled_array_2][samples] = *pRecord++;
								}

								// Updating number of lines acquired
								lines_filled_array_2++;
								pRecord -= 4 * points_gap;
							}
						}
					}

					if (frame_averaging)
					{
						// FORWARD FRAME
						if (image_array_1_filled == true)
						{
							// UPDATING FRAMES SUMMED
							num_frames_summed++;

							// Already finished summing frames : need to get average value here!
							if (num_frames_summed >= num_avg_frame)
							{
								// Extracting the EVEN and ODD lines
								for (U32 record = 0; (record < recordPerBuffer); record++)
								{
									// EVEN LINES -------->
									if (record % 2 == 0)
									{
										for (U32 samples = 0; samples < samplesPerRecord; samples++)
										{
											// FILLING THE AVERAGED ARRAY_2
											chan_1_image_array_2[lines_filled_array_2][samples] = (chan_1_image_array_avg[lines_filled_array_2][samples] + *pRecord++) / num_frames_summed;

											// RESETTING THE AVG ARRAYS
											chan_1_image_array_avg[lines_filled_array_2][samples] = 0;
										}

										// Updating number of lines acquired
										lines_filled_array_2++;
										lines_averaged_array_2++;
									}

									// ODD LINES <--------
									else if (record % 2 == 1)
									{
										// Adjusting the offset
										points_gap = bi_direction_offset;

										// Skipping the detected gap
										pRecord += 4 * points_gap;

										for (U32 samples = samplesPerRecord - 1; samples >= points_gap; samples--)
										{
											// FILLING THE AVERAGED ARRAY_2
											chan_1_image_array_2[lines_filled_array_2][samples] = (chan_1_image_array_avg[lines_filled_array_2][samples] + *pRecord++) / num_frames_summed;

											// RESETTING THE AVG ARRAYS
											chan_1_image_array_avg[lines_filled_array_2][samples] = 0;
										}

										// Updating number of lines acquired
										lines_filled_array_2++;
										lines_averaged_array_2++;
									}
								}

								// Resetting the number of frames summed
								if (lines_filled_array_2 == y_length)
								{
									num_frames_summed = 0;
									filled_array_2 = true;
									one_frame_completed = true;
								}
							}
						}
					}
								
					// ONE BUFFER : COMPLETED : UPDATING THE STATUS OF DATA CONTAINERS
					if (image_array_1_filled == true)
					{
						// Executing frame saving
						if (frames_saving_init)
						{
							if (!frame_averaging)		num_avg_frame_temp = 1;
							else						num_avg_frame_temp = num_avg_frame;
						//	frame_saving_aborted = false;
						//	num_frames_saved_chan_1 = 0;
						//	//countMoveAction = 0;
						//	frames_saving_init = true;
						//	//first_avg_frame = true;

							// FOR CHANNEL 1
							if (save_data_chan_1)
							{
								chan_1_frame_saving_status = true;
								frames_saving_cond_chan_1 = true;
								//frame_saving_progress_chan_1 = 0;
							}

					//		// Other bools
					//		lines_filled_array_1 = 0;
					//		lines_filled_array_2 = 0;
					//		received_first_frame = false;
					//		frames_saving_init = false;
						}

						if (!frame_averaging)
							one_frame_completed = true;
					}

					// Add the buffer to the end of the list of available buffers.
					if (running_loops)
					{
						retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
						if (retCode != ApiSuccess)
						{
							// Updating Scan Status
							sprintf(ScanStatus, "Error: AlazarPostAsyncBuffer Failed2 --- %s", AlazarErrorToText(retCode));
							if (debug_window)	_cprintf("Error: AlazarPostAsyncBuffer Failed2 --- %s\n", AlazarErrorToText(retCode));

							// Rearming ATS for Acquisition
							reArm();
							NTUscope::TerminateAll();
							running_loops = false;
							return;
						}
					}	
				}

				// TIME MEASUREMENTS
				elapsed_time = std::chrono::high_resolution_clock::now();
				one_frame_duration = std::chrono::duration<double>(elapsed_time - starting_time);
				starting_time = elapsed_time;
				AcqTime = double(one_frame_duration.count());
				FrameRate = 1.0 / AcqTime;

				// STOPPED : ABORT ACQUISITION
				retCode = AlazarAbortAsyncRead(boardHandle);
				if (retCode != ApiSuccess)
				{
					// Updating Scan Status
					sprintf(ScanStatus, "Error: AlazarAbortAsyncRead Failed --- %s", AlazarErrorToText(retCode));
					if (debug_window)	_cprintf("Error: AlazarAbortAsyncRead Failed --- %s\n", AlazarErrorToText(retCode));

					// Rearming ATS for Acquisition
					reArm();
					NTUscope::TerminateAll();
					running_loops = false;
					return;
				}

				// Display progress
				/*printf("Completed %u buffers\r", buffersCompleted);
				printf("Capturing %d buffers ... press any key to abort\n", buffersPerAcquisition);
				printf("Samples Per Records: %d\n", samplesPerRecord);
				printf("Samples Per Seconds: %lf\n", samplePerSec);*/

				// Display results
				double transferTime_sec = (GetTickCount() - startTickCount) / 1000.;
				//printf("Capture completed in %.2lf sec\n", transferTime_sec);	
				

				double buffersPerSec;
				double bytesPerSec;
				double recordsPerSec;
				U32 recordsTransferred = recordPerBuffer * buffersCompleted;

				if (transferTime_sec > 0.)
				{
					buffersPerSec = buffersCompleted / transferTime_sec;
					bytesPerSec = bytesTransferred / transferTime_sec;
					recordsPerSec = recordsTransferred / transferTime_sec;
				}
				else
				{
					buffersPerSec = 0.;
					bytesPerSec = 0.;
					recordsPerSec = 0.;
				}

				/*printf("Captured %u buffers (%.4g buffers per sec)\n", buffersCompleted, buffersPerSec);
				printf("Captured %u records (%.4g records per sec)\n", recordsTransferred, recordsPerSec);
				printf("Transferred %I64d bytes (%.4g bytes per sec)\n", bytesTransferred, bytesPerSec);*/

				// STOPPED : ABORT ACQUISITION
				//retCode = AlazarAbortAsyncRead(boardHandle);
				//if (retCode != ApiSuccess)
				//{
				//	// Updating Scan Status
				//	sprintf(ScanStatus, "Error: AlazarAbortAsyncRead Failed --- %s", AlazarErrorToText(retCode));
				//	if (debug_window)	_cprintf("Error: AlazarAbortAsyncRead Failed --- %s\n", AlazarErrorToText(retCode));

				//	// Rearming ATS for Acquisition
				//	reArm();
				//	NTUscope::TerminateAll();
				//	running_loops = false;
				//	return;
				//}
			}
		}

		// ABORT ACQUISITION
		retCode = AlazarAbortAsyncRead(boardHandle);
		if (retCode != ApiSuccess)
		{
			if (debug_window)	_cprintf("Error reArming : AlazarAbortAsyncRead Failed --- %s\n", AlazarErrorToText(retCode));
			return;
		}

		// Free all memory allocated
		for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
		{
			if (BufferArray[bufferIndex] != NULL)
			{
#ifdef _WIN32
				VirtualFree(BufferArray[bufferIndex], 0, MEM_RELEASE);
#else
				free(BufferArray[bufferIndex]);
#endif
			}
		}

		if (fpData != NULL)
			fclose(fpData);

		// Terminating all
		TerminateAll();

		// Updating scanning thread
		threadEnd[THREAD_SCAN] = true;

		// Done!
		return;
	}

	// Buffer data handling thread
	void NTUscope::bufferHandler()
	{
		// OM NAMAH SHIVAY : Thread Priority Setting
		DWORD mask = 0 << 0;
		for (int i = 2; i <= NumCPU - 1; i++)
			mask = mask + (1 << i);
		auto ret = SetThreadAffinityMask(GetCurrentThread(), mask);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

		while (running_loops)
		{
			// If data is available to process
			if (bufferOnProcess == true)
			{
				// Get the pointer
				U16* pRecord = bufferData;

				// IF ARRAY_1 IS FILLED
				if (image_array_1_filled == true)
				{
					// Already finished summing frames : need to get average value here!
					if (num_frames_summed >= num_avg_frame)
					{
						// Check for previous frame saved or not
						activate_frame_savinng_confirm = true;

						// Extracting the EVEN and ODD lines
						for (U32 record = 0; (record < recordPerBuffer); record++)
						{
							// EVEN LINES -------->
							if (record % 2 == 0)
							{
								for (U32 samples = 0; samples < samplesPerRecord; samples++)
								{
									chan_1_image_array_2[lines_filled_array_2][samples] = (chan_1_image_array_avg[lines_filled_array_2][samples] + *pRecord++) / num_frames_summed;
									
									// RESETTING THE AVG ARRAYS
									chan_1_image_array_avg[lines_filled_array_2][samples] = 0;

								}

								// Updating number of lines acquired
								lines_filled_array_2++;
	
							}

							// ODD LINES <--------
							else if (record % 2 == 1)
							{
								// Adjusting the offset
								points_gap = bi_direction_offset;

								// Skipping the detected gap
								pRecord += 4 * points_gap;

								for (U32 samples = samplesPerRecord - 1; samples >= points_gap; samples--)
								{
									chan_1_image_array_2[lines_filled_array_2][samples] = (chan_1_image_array_avg[lines_filled_array_2][samples] + *pRecord++) / num_frames_summed;
								

									// RESETTING THE AVG ARRAYS
									chan_1_image_array_avg[lines_filled_array_2][samples] = 0;
								}

								// Updating number of lines acquired
								lines_filled_array_2++;
								
							}
						}

						// Resetting the number of frames summed
						if (lines_filled_array_2 == y_length)
						{
							num_frames_summed = 0;
							filled_array_2 = true;
							one_frame_completed = true;
						}
					}

					// Keep on summing until required number of frames
					else
					{
						// Extracting the EVEN and ODD lines
						for (U32 record = 0; (record < recordPerBuffer); record++)
						{
							// EVEN LINES -------->
							if (record % 2 == 0)
							{
								for (U32 samples = 0; samples < samplesPerRecord; samples++)
								{
									chan_1_image_array_avg[lines_filled_array_2][samples] = chan_1_image_array_avg[lines_filled_array_2][samples] + *pRecord++;
									
								}

								// Updating number of lines acquired
								lines_filled_array_2++;
							}

							// ODD LINES <--------
							else if (record % 2 == 1)
							{
								// Adjusting the offset
								points_gap = bi_direction_offset;

								// Skipping the detected gap
								pRecord += 4 * points_gap;

								for (U32 samples = samplesPerRecord - 1; samples >= points_gap; samples--)
								{
									chan_1_image_array_avg[lines_filled_array_2][samples] = chan_1_image_array_avg[lines_filled_array_2][samples] + *pRecord++;
									
								}

								// Updating number of lines acquired
								lines_filled_array_2++;
							}
						}
					}
				}

				// IF ARRAY_2 IS FILLED
				else if (image_array_2_filled == true)
				{
					// Already finished summing frames : need to get average value here!
					if (num_frames_summed >= num_avg_frame)
					{
						// Check for previous frame saved or not
						activate_frame_savinng_confirm = true;

						// Extracting the EVEN and ODD lines
						for (U32 record = 0; (record < recordPerBuffer); record++)
						{
							// EVEN LINES -------->
							if (record % 2 == 0)
							{
								for (U32 samples = 0; samples < samplesPerRecord; samples++)
								{
									chan_1_image_array_1[lines_filled_array_1][samples] = (chan_1_image_array_avg[lines_filled_array_1][samples] + *pRecord++) / num_frames_summed;
									

									// RESETTING THE AVG ARRAYS
									chan_1_image_array_avg[lines_filled_array_1][samples] = 0;
								
								}

								// Updating number of lines acquired
								lines_filled_array_1++;
							}

							// ODD LINES <--------
							else if (record % 2 == 1)
							{
								// Adjusting the offset
								points_gap = bi_direction_offset;

								// Skipping the detected gap
								pRecord += 4 * points_gap;

								for (U32 samples = samplesPerRecord - 1; samples >= points_gap; samples--)
								{
									chan_1_image_array_1[lines_filled_array_1][samples] = (chan_1_image_array_avg[lines_filled_array_1][samples] + *pRecord++) / num_frames_summed;
									

									// RESETTING THE AVG ARRAYS
									chan_1_image_array_avg[lines_filled_array_1][samples] = 0;

								}

								// Updating number of lines acquired
								lines_filled_array_1++;
							}
						}

						// Resetting the number of frames summed
						if (lines_filled_array_1 == y_length)
						{
							num_frames_summed = 0;
							filled_array_2 = false;
							one_frame_completed = true;
						}
					}

					// Keep on summing until required number of frames
					else
					{
						// Extracting the EVEN and ODD lines
						for (U32 record = 0; (record < recordPerBuffer); record++)
						{
							// EVEN LINES -------->
							if (record % 2 == 0)
							{
								for (U32 samples = 0; samples < samplesPerRecord; samples++)
								{
									chan_1_image_array_avg[lines_filled_array_1][samples] = chan_1_image_array_avg[lines_filled_array_1][samples] + *pRecord++;
									
								}

								// Updating number of lines acquired
								lines_filled_array_1++;
							}

							// ODD LINES <--------
							else if (record % 2 == 1)
							{
								// Adjusting the offset
								points_gap = bi_direction_offset;

								// Skipping the detected gap
								pRecord += 4 * points_gap;

								for (U32 samples = samplesPerRecord - 1; samples >= points_gap; samples--)
								{
									chan_1_image_array_avg[lines_filled_array_1][samples] = chan_1_image_array_avg[lines_filled_array_1][samples] + *pRecord++;
									
								}

								// Updating number of lines acquired
								lines_filled_array_1++;
							}
						}
					}
				}

				// Done!
				bufferOnProcess = false;
			}
		}
	}

	// Deallocate All
	void NTUscope::deAllocate()
	{
		allow_debug_window = false;
		allow_configure = false;
		allow_scan = false;
		allow_bidirection = false;
		allow_alignment = false;
		allow_offsets = false;
		allow_filter = false;
		allow_save = false;
		allow_binView = false;
		allow_export = false;

		// Additional Controls
		pauseScanEmergency = true;
		pauseMemsEmergency = true;
		//stabilizeGalvo = false;
		pauseSaveEmergency = true;
		pauseBinViewEmergency = true;
		pauseExportEmergency = true;

		// Other pointers
		/*
		delete result;
		delete exportResult;
		delete ScanStatus;
		delete save_selected_frame_location;
		delete returnStr;
		delete selected_frame_sequence;
		delete filtersID;
		delete ver;
		delete chan_1_file;
		delete chan_2_file;
		delete chan_3_file;
		delete chan_4_file;
		delete full_date_incl_path;
		delete full_time_incl_path;
		delete full_time_incl_path_chan_1;
		delete full_time_incl_path_chan_2;
		delete full_time_incl_path_chan_3;
		delete full_time_incl_path_chan_4;
		delete chan_1_filename;
		delete chan_2_filename;
		delete chan_3_filename;
		delete chan_4_filename;
		delete loaded_parent_directory;
		delete export_directory_full;
		delete retCodeStage;
		delete stageCOMname;
		delete PMT_COMname;
		*/
	}

	// Terminating acquisition
	void NTUscope::TerminateAll()
	{
		// Stopping rapid scan
		TERMINATE_RAPID = true;
		rapidScanCont = false;

		// Stopping previewing threads
		img_thread_cont_chan_1 = false;

		// Stopping streaming threads
		file_stream_cont_chan_1 = false;
		activate_frame_savinng_confirm = false;
		one_frame_completed = false;



		// Stopping galvo thread
		mems_start_reOrient = true;
		mems_thread_cont = false;
		mems_keep_running = false;

		// Resetting scan directions
		forward_move = false;
		backward_move = false;

		// Resolution shift settings
		if (memshiftStarted == true)
			RShiftMemsHandle(false);
		memshiftEnable = false;
		memshiftStarted = false;
		memsModeShifted = true;

		// RESETTING ALL ARRAYS
		/*
		for (int i = 0; i < y_length; i++)
		{
			for (int j = 0; j < samplesPerRecord; j++)
			{
				chan_1_image_array_1[i][j] = 0;

				chan_1_image_array_2[i][j] = 0;

				//chan_1_prev_array_1[i][j] = 0;
				//chan_1_prev_array_2[i][j] = 0;
			}
		}
		*/

		// Reorienting galvo
		//if (galvo_real_achieved_step_forward != 0)
		//{
		//	// Repositioning galvo if needed
		//	//char* result = orient_galvo(galvo_starting_step_no);
		//	galvo_real_achieved_step_forward = 0;
		//}

		// Acquisition finished here
		ACQ_finished = true;

		// Finished
		return;
	}

	void NTUscope::getTrigTimeStamp()
	{
		AlazarGetTriggerAddress(
			boardHandle, // HANDLE -- board handle
			128, // U32 -- record number (1-indexed)
			&triggerAddress, // U32* -- trigger address
			&timestampHigh, // U32* -- timestamp high part
			&timestampLow // U32* -- timestamp low part
		);

		// Convert the record timestamp from counts to seconds
		__int64 timeStamp_cnt;
		timeStamp_cnt = ((__int64)timestampHigh) << 8;
		timeStamp_cnt |= timestampLow & 0x0ff;
		double samplesPerTimestampCount = 2; // board specific constant
		double samplesPerSec = 50.e6; // sample rate
		double timeStamp_sec = (double)samplesPerTimestampCount *timeStamp_cnt / samplesPerSec;
		cout << timeStamp_sec << endl;
	}


#ifndef WIN32
	inline U32 GetTickCount(void)
	{
		struct timeval tv;
		if (gettimeofday(&tv, NULL) != 0)
			return 0;
		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	}

	inline void Sleep(U32 dwTime_ms)
	{
		usleep(dwTime_ms * 1000);
	}

	inline int _kbhit(void)
	{
		struct timeval tv;
		fd_set rdfs;

		tv.tv_sec = 0;
		tv.tv_usec = 0;

		FD_ZERO(&rdfs);
		FD_SET(STDIN_FILENO, &rdfs);

		select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
		return FD_ISSET(STDIN_FILENO, &rdfs);
	}

	inline int GetLastError()
	{
		return errno;
	}
#endif

}