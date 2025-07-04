#include "NTUcore.h"

namespace NTU
{
	// Dynamic Resolution Shift
	bool NTUscope::RShiftMemsHandle(bool RShiftEN)
	{
		if (RShiftEN)
		{
			return true;
		}
	//	// Resetting parameters
	//	galvoModeShifted = false;
	//	mems_repositioned = true;
	//	forward_move = false;
	//	backward_move = false;

	//	// MODE : SET STEPS PER LINE
	//	if (galvoContMove == false || true)
	//	{
	//		// Enabling Resolution Shift
	//		if (RShiftEN)
	//		{
	//			// Backuping current parameters
	//			yLengthPrev = y_length;
	//			StepsPerLinePrev = num_of_steps_in_1_line;
	//			stepIntervalPrev = step_interval;
	//			galvoContMovePrev = galvoContMove;

	//			// Setting new parameters
	//			HighSpeedPrev = false;
	//			high_speed = true;
	//			galvoContMove = false;
	//			y_length = y_length / RShiftLvl;
	//			if (y_length < 800)		y_length = 800;
	//			num_of_steps_in_1_line = total_steps / y_length;

	//			// Calculating step interval
	//			if (num_of_steps_in_1_line > 1)			step_interval = (duration_one_line - galvoLineEdgeDetectionTolerance) / (num_of_steps_in_1_line - 1);
	//			else if (num_of_steps_in_1_line == 1)	step_interval = 0.0;
	//		}

	//		// Disabling Resolution Shift
	//		else
	//		{
	//			// Getting original parameters
	//			HighSpeedPrev = true;
	//			high_speed = false;
	//			y_length = yLengthPrev;
	//			num_of_steps_in_1_line = StepsPerLinePrev;
	//			step_interval = stepIntervalPrev;
	//			galvoContMove = galvoContMovePrev;
	//		}
	//	}

	//	// Orienting galvo to Starting Position
	//	char* result = orient_galvo(galvo_starting_step_no);

	//	// HIGH_SPEED : FRAME TRIGGERING
	//	retCode = AlazarConfigureAuxIO(boardHandle, AUX_IN_TRIGGER_ENABLE, TRIGGER_SLOPE_POSITIVE);
	//	if (retCode != ApiSuccess)	return false;
	//	

	//	// Updating some parameters
	//	update_recordPerBuffer(bi_direction, y_length);
	//	buffersPerAcquisition = get_buffersPerAcquisition(bi_direction, y_length);
	//	recordsPerAcquisition = get_recordsPerAcquisition(bi_direction, y_length, buffersPerAcquisition);

	//	// Calculate the size of each DMA buffer in bytes
	//	bytesPerRecord = (U32)(bytesPerSample * samplesPerRecord + 0.5);
	//	bytesPerBuffer = bytesPerRecord * recordPerBuffer * channel_Count;

	//	// Configure the board to make an NPT AutoDMA acquisition
	//	retCode = AlazarBeforeAsyncRead(boardHandle, channel_Mask, -(long)preTriggerSamples,
	//		samplesPerRecord, recordPerBuffer, recordsPerAcquisition, admaFlag);
	//	if (retCode != ApiSuccess)	return false;

	//	// Add the buffers to a list of buffers available to be filled by the board
	//	for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT); bufferIndex++)
	//	{
	//		U16* pBuffer = BufferArray[bufferIndex];
	//		retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
	//		if (retCode != ApiSuccess)	return false;
	//	}

	//	// Wait a while for stability
	//	//Sleep(100);
	}
}