#include"NTUcore.h"

namespace NTU
{
	// Initialize CUDA
	void NTUscope::initCUDA()
	{
		// Trying to initialize CUDA
		try
		{
			// Getting cuda enabled device count
			int NumDeviceCUDA = cv::cuda::getCudaEnabledDeviceCount();
			if (NumDeviceCUDA == 0 || NumDeviceCUDA == -1)
			{
				MessageBox(NULL, "Verify your CUDA enabled GPU!", "CUDA Error!", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
				exit(0);
			}
			else
			{
				// Initializing CUDA
				cv::cuda::GpuMat initialize;
				initialize.create(1, 1, CV_8U);
				initialize.release();

				// For page-locked memory
				cv::Mat::setDefaultAllocator(cv::cuda::HostMem::getAllocator(cv::cuda::HostMem::AllocType::PAGE_LOCKED));
			}
		}
		catch (Exception e)
		{
			// CUDA problem detected
			MessageBox(NULL, "Verify your CUDA enabled GPU!", "CUDA Error!", MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_SYSTEMMODAL);
			exit(0);
		}
	}

	char* NTUscope::config_Alazar(double internal_clock_freq, bool internal_clock, bool external_clock)
	{
		// Exiting if not allowed
		if (allow_configure == false)
			exit(0);

		// Verification 
		RETURN_CODE retCode;
		sprintf(result, "%s", "OK");
		// Select a board
		U32 systemId = 1;
		U32 boardId = 1;

		// Get a handle to the board
		boardHandle = AlazarGetBoardBySystemID(systemId, boardId);
		if (boardHandle == NULL)
		{
			result = "Unknown Error : boardHandle";
			return result;
		}

		// For internal clock system
		if (internal_clock)
		{
			char* result;
			result = set_internal_clock_freq(boardHandle, internal_clock_freq);
			if (result[0] != 'O')	return result;
		}

		// For external clock system
		else if (external_clock)
		{
			retCode = AlazarSetCaptureClock(boardHandle, FAST_EXTERNAL_CLOCK, SAMPLE_RATE_USER_DEF, CLOCK_EDGE_RISING, 0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : External --- %s", AlazarErrorToText(retCode));
				return result;
			}
		}

		// Select channel A input parameters as required
		retCode = AlazarInputControlEx(boardHandle, CHANNEL_A, DC_COUPLING, INPUT_RANGE_PM_800_MV, IMPEDANCE_50_OHM);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: CHANNEL A --- %s", AlazarErrorToText(retCode));
			return result;
			
		}

		// Select channel B input parameters as required
		retCode = AlazarInputControlEx(boardHandle, CHANNEL_B, DC_COUPLING, INPUT_RANGE_PM_800_MV, IMPEDANCE_50_OHM);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: CHANNEL B --- %s", AlazarErrorToText(retCode));
			return result;
		}

		//Select TRIGGER inputs and levels as required
		// Calculate the trigger level code from the level and range
		double triggerLevel_volts = 1.25; // trigger level
		double triggerRange_volts = 2.5; // input range
		U32 triggerLevel_code =
			(U32)(128 + 127 * triggerLevel_volts / triggerRange_volts);

		// TODO: Select trigger inputs and levels as required
		retCode = AlazarSetTriggerOperation(boardHandle,
			TRIG_ENGINE_OP_J,
			TRIG_ENGINE_J,
			TRIG_EXTERNAL,
			TRIGGER_SLOPE_POSITIVE,
			160,
			TRIG_ENGINE_K,
			TRIG_DISABLE,
			TRIGGER_SLOPE_POSITIVE,
			128);

		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: Setting Single TRIGGER Engines --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Select EXTERNAL TRIGGER parameters as required
		// For ATS9146
		retCode = AlazarSetExternalTrigger(boardHandle, DC_COUPLING, ETR_2V5);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: Trigger Parameter (ETR_2V5) --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// CONFIGURES AS TRIGGER INPUT
		retCode = AlazarConfigureAuxIO(boardHandle, AUX_IN_TRIGGER_ENABLE, TRIGGER_SLOPE_POSITIVE);
		//retCode = AlazarConfigureAuxIO(boardHandle, AUX_OUT_TRIGGER, 0);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: AlazarConfigureAuxIO Failed (DigIN) --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Set TRIGGER DELAY as required
		double triggerDelay_sec = 0;
		U32 triggerDelay_samples = (U32)(triggerDelay_sec * samplePerSec + 0.5);
		retCode = AlazarSetTriggerDelay(boardHandle, triggerDelay_samples);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: TRIGGER DELAY --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Set TRIGGER TIMEOUT as required.
		double triggerTimeout_sec = 0;
		U32 triggerTimeout_clocks = (U32)(triggerTimeout_sec / 10.e-6 + 0.5);
		// retCode = AlazarSetTriggerTimeOut(boardHandle, 1); //fastest: 10 us
		retCode = AlazarSetTriggerTimeOut(boardHandle, 0); // Wait infinitely for an external trigger
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: TRIGGER TIMEOUT --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Calculate the number of enabled channels from the channel mask
		channel_Count = 0;
		for (int channel = 0; channel < channels_Per_Board; channel++)
		{
			U32 channelId = 1U << channel;
			if ((channelId & channel_Mask) != 0)
				channel_Count++;
		}
		if ((channel_Count < 1) || (channel_Count > channels_Per_Board))
		{
			sprintf(result, "Unknown Error: Channel Count");
			return result;
		}

		// GET BUFFER SIZE
		U8 bitsPerSample;
		U32 maxSamplesPerChannel;
		retCode = AlazarGetChannelInfo(boardHandle, &maxSamplesPerChannel, &bitsPerSample);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: Channel Info Failed --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Calculate the size of each DMA buffer in bytes
		bytesPerSample = (double)((bitsPerSample + 7) / 8);
		bytesPerRecord = (U32)(bytesPerSample * samplesPerRecord + 0.5); // 0.5 compensates for double to integer conversion
		bytesPerBuffer = bytesPerRecord * recordPerBuffer * channel_Count;
		MAXBytesPerBuffer = bytesPerRecord * MAX_Y_LINES * channel_Count;

		// Allocate memory for DMA buffers
		for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
		{
#ifdef _WIN32 // Allocate page aligned memory
			BufferArray[bufferIndex] = (U16*)VirtualAlloc(NULL, MAXBytesPerBuffer, MEM_COMMIT, PAGE_READWRITE);
#else
			BufferArray[bufferIndex] = (U16*)valloc(MAXBytesPerBuffer);
#endif
			if (BufferArray[bufferIndex] == NULL)
			{
				sprintf(result, "Unknown Error: Buffer Alloc");
				return result;
			}
		}

		// Configure the number of samples per record
		retCode = AlazarSetRecordSize(boardHandle, preTriggerSamples, postTriggerSamples);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: Record Size Failed --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Configure the board to make an NPT AutoDMA acquisition
		recordsPerAcquisition = recordPerBuffer * buffersPerAcquisition;


		// Allocating the memory needed
		scanMemoryManage(FREE_RESERVE);
		
		// FINISH CONFIGURATION
		return result;
	}

	// Configuring ATS
	char* NTUscope::update_Settings(double internal_clock_freq, bool internal_clock, bool external_clock)
	{
		// Exiting if not allowed
		if (allow_configure == false)
			exit(0);

		RETURN_CODE retCode;
		sprintf(result, "OK");

		// Select clock parameters as required to generate this sample rate
		// For internal clock system
		if (internal_clock)
		{
			char* result;
			result = set_internal_clock_freq(boardHandle, internal_clock_freq);
			if (result[0] != 'O')	return result;
		}

		// For external clock system
		else if (external_clock)
		{
			retCode = AlazarSetCaptureClock(boardHandle, FAST_EXTERNAL_CLOCK, SAMPLE_RATE_USER_DEF, CLOCK_EDGE_RISING, 0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : External --- %s", AlazarErrorToText(retCode));
				return result;
			}
		}
		
		// Select TRIGGER inputs and levels as required | Bidirectional
		if (bi_direction)
		{
			// TODO: Select trigger inputs and levels as required
			retCode = AlazarSetTriggerOperation(boardHandle,
				TRIG_ENGINE_OP_J,
				TRIG_ENGINE_J,
				TRIG_EXTERNAL,
				TRIGGER_SLOPE_POSITIVE,
				160,
				TRIG_ENGINE_K,
				TRIG_DISABLE,
				TRIGGER_SLOPE_POSITIVE,
				128);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: Setting Dual TRIGGER Engines --- %s", AlazarErrorToText(retCode));
				return result;
			}
		}

		// biDirectional_ext_trigger : FRAME TRIGGERING ENABLED
		else 
		{
			retCode = AlazarConfigureAuxIO(boardHandle, AUX_IN_TRIGGER_ENABLE, TRIGGER_SLOPE_POSITIVE);
			//retCode = AlazarConfigureAuxIO(boardHandle, AUX_OUT_TRIGGER_ENABLE, 0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarConfigureAuxIO Failed (FrTrigIn) --- %s", AlazarErrorToText(retCode));
				return result;
			}
		}
		
		// Calculate the size of each DMA buffer in bytes
		bytesPerRecord = (U32)(bytesPerSample * samplesPerRecord + 0.5);
		bytesPerBuffer = bytesPerRecord * recordPerBuffer * channel_Count;
		// Free all memory allocated for ACQUISITION BUFFERS
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

		// Allocate memory for DMA buffers
		for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
		{
#ifdef _WIN32 // Allocate page aligned memory
			BufferArray[bufferIndex] = (U16*)VirtualAlloc(NULL, bytesPerBuffer, MEM_COMMIT, PAGE_READWRITE);
#else
			BufferArray[bufferIndex] = (U16*)valloc(bytesPerBuffer);
#endif
			if (BufferArray[bufferIndex] == NULL)
			{
				sprintf(result, "Unknown Error: Buffer Alloc");
				return result;
			}
		}

		// Configure the number of samples per record
		retCode = AlazarSetRecordSize(boardHandle, preTriggerSamples, postTriggerSamples);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: Record Size Failed --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Configure the board to make an NPT AutoDMA acquisition
		retCode = AlazarBeforeAsyncRead(boardHandle, channel_Mask, -(long)preTriggerSamples,
			samplesPerRecord, recordPerBuffer, recordsPerAcquisition, admaFlag);
		if (retCode != ApiSuccess)
		{
			sprintf(result, "Error: AlazarBeforeAsyncRead Failed_changed --- %s", AlazarErrorToText(retCode));
			return result;
		}

		// Attempt 1: add the buffers to a list of buffers available to be filled by the board
		for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
		{
			U16* pBuffer = BufferArray[bufferIndex];
			retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
			if (retCode != ApiSuccess)
				break;
		}

		// Attempt 2: add the buffers to a list of buffers available to be filled by the board
		if (retCode != ApiSuccess)
		{
			// Free all memory allocated for ACQUISITION BUFFERS
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

			// Allocate memory for DMA buffers
			for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
			{
#ifdef _WIN32 // Allocate page aligned memory
				BufferArray[bufferIndex] = (U16*)VirtualAlloc(NULL, bytesPerBuffer, MEM_COMMIT, PAGE_READWRITE);
#else
				BufferArray[bufferIndex] = (U16*)valloc(bytesPerBuffer);
#endif
				if (BufferArray[bufferIndex] == NULL)
				{
					sprintf(result, "Unknown Error: Buffer Alloc");
					return result;
				}
			}

			// Attempt 2: add the buffers to a list of buffers available to be filled by the board
			for (bufferIndex = 0; bufferIndex < BUFFER_COUNT; bufferIndex++)
			{
				U16* pBuffer = BufferArray[bufferIndex];
				retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
				if (retCode != ApiSuccess)
					break;
			}
		}

		// Allocating the memory needed
		scanMemoryManage(FREE_RESERVE);

		// FINISH CONFIGURATION
		return result;
	}

	// Rearming ATS for Acquisition
	void NTUscope::reArm()
	{
		// Abort the acquisition
		retCode = AlazarAbortAsyncRead(boardHandle);
		if (retCode != ApiSuccess)
		{
			if (debug_window)	_cprintf("Error reArming : AlazarAbortAsyncRead Failed --- %s\n", AlazarErrorToText(retCode));
			return;
		}

		// Configure the board to make an NPT AutoDMA acquisition
		retCode = AlazarBeforeAsyncRead(boardHandle, channel_Mask, -(long)preTriggerSamples,
			samplesPerRecord, recordPerBuffer, 0x7FFFFFFF, admaFlag);
		if (retCode != ApiSuccess)
		{
			if (debug_window)	_cprintf("Error reArming : AlazarBeforeAsyncRead Failed_reArm --- %s\n", AlazarErrorToText(retCode));
			return;
		}

		// Add the buffers to a list of buffers available to be filled by the board
		for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT); bufferIndex++)
		{
			U16* pBuffer = BufferArray[bufferIndex];
			retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
			if (retCode != ApiSuccess)
			{
				if (debug_window)	_cprintf("Error reArming : AlazarPostAsyncBuffer Failed --- %s\n", AlazarErrorToText(retCode));
				return;
			}
		}
	}

	// For setting the internal clock
	char* NTUscope::set_internal_clock_freq(HANDLE boardHandle, double freq)
	{
		RETURN_CODE retCode;
		sprintf(result, "OK");

		// SETTING THE INTERNAL CLOCK FREQUENCY
		if (freq == 2.0)
		{
			retCode = AlazarSetCaptureClock(boardHandle,
				INTERNAL_CLOCK,
				SAMPLE_RATE_2MSPS,
				CLOCK_EDGE_RISING,
				0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : Internal --- %s", AlazarErrorToText(retCode));
				return result;
			}

			// Success
			return result;
		}
		else if (freq == 10.0)
		{
			retCode = AlazarSetCaptureClock(boardHandle,
				INTERNAL_CLOCK,
				SAMPLE_RATE_10MSPS,
				CLOCK_EDGE_RISING,
				0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : Internal --- %s", AlazarErrorToText(retCode));
				return result;
			}

			// Success
			return result;
		}
		else if (freq == 20.0)
		{
			retCode = AlazarSetCaptureClock(boardHandle,
				INTERNAL_CLOCK,
				SAMPLE_RATE_20MSPS,
				CLOCK_EDGE_RISING,
				0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : Internal --- %s", AlazarErrorToText(retCode));
				return result;
			}

			// Success
			return result;
		}
		else if (freq == 50.0)
		{
			retCode = AlazarSetCaptureClock(boardHandle,
				INTERNAL_CLOCK,
				SAMPLE_RATE_50MSPS,
				CLOCK_EDGE_RISING,
				0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : Internal --- %s", AlazarErrorToText(retCode));
				return result;
			}

			// Success
			return result;
		}
		else if (freq == 100.0)
		{
			retCode = AlazarSetCaptureClock(boardHandle,
				INTERNAL_CLOCK,
				SAMPLE_RATE_100MSPS,
				CLOCK_EDGE_RISING,
				0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : Internal --- %s", AlazarErrorToText(retCode));
				return result;
			}

			// Success
			return result;
		}
		else if (freq == 125.0)
		{
			retCode = AlazarSetCaptureClock(boardHandle,
				INTERNAL_CLOCK,
				SAMPLE_RATE_125MSPS,
				CLOCK_EDGE_RISING,
				0);
			if (retCode != ApiSuccess)
			{
				sprintf(result, "Error: AlazarSetCaptureClock : Internal --- %s", AlazarErrorToText(retCode));
				return result;
			}

			// Success : return 1
			return result;
		}
		else
		{
			sprintf(result, "Error: Unknown");
			return result;
		}
	}

	// For postTriggerSamples
	U32 NTUscope::get_postTrigSample()
	{
		// Variable declaration
		U32 postTriggerSamples = 0;

		// Calculating number of points in one unidirectional line
		temp_num_points_per_line = double(lineTime * samplePerSec);

		// Subtracting 100 points for safety : Not to miss next trigger
		//if (samplePerSec > 70000000)		temp_num_points_per_line = temp_num_points_per_line - double(double(skipTimeEndLine + extraOffsetUltraDense) / pow(10, 6)) * samplePerSec;
		//else								temp_num_points_per_line = temp_num_points_per_line - double(double(skipTimeEndLine) / pow(10, 6)) * samplePerSec;

		// Making round figure (Two 'zero' ending) for convenience
		postTriggerSamples = int(temp_num_points_per_line / 100) * 100;

		// Making the number divisible by 4 : FOR DATA ALIGNMENT
		//int reminder = postTriggerSamples % 32;
		int reminder = postTriggerSamples % 128;
		if (reminder != 0)	postTriggerSamples = postTriggerSamples - reminder;
		

		// Returning postTriggerSamples
		return postTriggerSamples;
	}

	// Scan Memory Management
	void NTUscope::scanMemoryManage(int mode)
	{
		// Mode: FREE : Deallocate only
		// Mode: FREE_RESERVE : Deallocate(if needed) + Allocate
		if (mode == FREE)
		{
			// CLEARING PREVIOUS MEMORY FIRST
			// Check pre Y length
			for (int j = 0; j < prev_Y_length; j++)
			{
				//free(image_overlap_array_1[j]);
				//free(image_overlap_array_2[j]);
				//free(image_overlap_array_3[j]);

				free(chan_1_image_array_1[j]);
				free(chan_1_image_array_2[j]);
				free(chan_1_image_array_avg[j]);
			}

			// Free the preview buffers
			free(chan_1_prev_array_1);
			free(chan_1_prev_array_2);


			// Free the buffer data
			free(bufferData);

			// Updating recent info
			prev_Y_length = 0;
		}
		else if (mode == FREE_RESERVE)
		{
			if ((prev_Y_length != y_length) || (prev_sampPerRec != samplesPerRecord))
			{
				// CLEARING PREVIOUS MEMORY FIRST
				/*for (int j = 0; j < prev_Y_length; j++)
				{
					free(chan_1_image_array_2[j]);
				}*/
				
				// Free the preview buffers
				//free(chan_1_prev_array_1);

				//// Free the buffer data
				//free(bufferData);
				
				// Calculating the memory needed for the acquisition
				U32 bytes_needed_16bit_data = samplesPerRecord * 2 + 4000;
				U32 bytes_needed_32bit_data = samplesPerRecord * 4 + 4000;
				
				// ALLOCATING MEMORY
				for (int i = 0; i < y_length; i++)
				{
					// CHANNEL 1 IMAGE ARRAYS
					chan_1_image_array_1[i] = (U16*)malloc(bytes_needed_16bit_data);
					chan_1_image_array_2[i] = (U16*)malloc(bytes_needed_16bit_data);
				}

				// Allocate memory for the preview buffers
				chan_1_prev_array_1 = (U16*)malloc(bytes_needed_16bit_data * y_length);
				chan_1_prev_array_2 = (U16*)malloc(bytes_needed_16bit_data * y_length);

				// Allocating memory for buffer data
				bufferData = (U16*)malloc(MAXBytesPerBuffer);
				
				// Updating the previous y_length: helpful for memory clearing later
				prev_Y_length = y_length;
				prev_sampPerRec = samplesPerRecord;

				//// Display some useful information
				//if (debug_window)
				//{
				//	_cprintf("\nPrev Y : %d \n", prev_Y_length);
				//	_cprintf("y_length : %d \n", y_length);
				//	_cprintf("Prev samplesPerRecord : %d \n", prev_sampPerRec);
				//	_cprintf("samplesPerRecord : %d \n", samplesPerRecord);
				//}
			}
			//cout << "\nPass!!!\n";
		}
	}

	// Set Calibration values
	void NTUscope::setCalVal()
	{
		PixelCALIB[0] = calibration_value;
		PixelCALIB[1] = y_length;
	}

	// Get the real-time Temp data of digitizer
	void NTUscope::getATSTemp()
	{
		RETURN_CODE retCode;
		retCode = AlazarGetParameterUL(boardHandle, channel, parameter, (U32*)&fpgaTemperature_degreesC);
		//printf("FPGA temperature          = %g C\n", fpgaTemperature_degreesC);
	}
	
}