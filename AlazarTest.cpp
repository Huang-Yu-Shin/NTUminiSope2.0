#include"NTUcore.h"

namespace NTU
{
	void NTUscope::infiniteScan()
	{
        // TODO: Specify the acquisition length in seconds
        double acquisitionLength_sec = 5.;

        // Create a data file if required
        FILE* fpData = NULL;

        if (saveData)
        {
            fpData = fopen("data_bin.txt", "wb");
            if (fpData == NULL)
            {
                printf("Error: Unable to create data file -- %u\n", GetLastError());
                return;
            }
        }

        // Allocate memory for DMA buffers
        BOOL success = TRUE;


        // Configure the record size
        if (success)
        {
            retCode = AlazarSetRecordSize(boardHandle, preTriggerSamples, postTriggerSamples);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarSetRecordSize failed -- %s\n", AlazarErrorToText(retCode));
                success = FALSE;
            }
        }

        // Configure the board to make an NPT AutoDMA acquisition
        if (success)
        {
            retCode = AlazarBeforeAsyncRead(boardHandle, channel_Mask, -(long)preTriggerSamples,
                samplesPerRecord, recordPerBuffer, recordsPerAcquisition,
                admaFlag);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarBeforeAsyncRead failed -- %s\n", AlazarErrorToText(retCode));
                success = FALSE;
            }
        }

        // Add the buffers to a list of buffers available to be filled by the board

        for (bufferIndex = 0; (bufferIndex < BUFFER_COUNT) && success; bufferIndex++)
        {
            U16* pBuffer = BufferArray[bufferIndex];
            retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarPostAsyncBuffer %u failed -- %s\n", bufferIndex,
                    AlazarErrorToText(retCode));
                success = FALSE;
            }
        }


        // Arm the board system to wait for a trigger event to begin the acquisition
        if (success)
        {
            retCode = AlazarStartCapture(boardHandle);
            if (retCode != ApiSuccess)
            {
                printf("Error: AlazarStartCapture failed -- %s\n", AlazarErrorToText(retCode));
                success = FALSE;
            }
        }

        // Wait for each buffer to be filled, process the buffer, and re-post it to
        // the board.
        if (success)
        {
            printf("Capturing for %g seconds ... press any key to abort\n", acquisitionLength_sec);

            U32 startTickCount = GetTickCount();
            U32 stopTickCount = startTickCount + (U32)(acquisitionLength_sec * 1000 + 0.5);
            U32 buffersCompleted = 0;
            INT64 bytesTransferred = 0;
            for (;;)
            {
                // TODO: Set a buffer timeout that is longer than the time
                //       required to capture all the records in one buffer.
                U32 timeout_ms = 5000;

                // Wait for the buffer at the head of the list of available buffers
                // to be filled by the board.
                bufferIndex = buffersCompleted % BUFFER_COUNT;
                U16* pBuffer = BufferArray[bufferIndex];

                retCode = AlazarWaitAsyncBufferComplete(boardHandle, pBuffer, timeout_ms);
                if (retCode != ApiSuccess)
                {
                    printf("Error: AlazarWaitAsyncBufferComplete failed -- %s\n",
                        AlazarErrorToText(retCode));
                    success = FALSE;
                }

                if (success)
                {
                    // The buffer is full and has been removed from the list
                    // of buffers available for the board

                    buffersCompleted++;
                    bytesTransferred += bytesPerBuffer;

                    // TODO: Process sample data in this buffer.

                    // NOTE:
                    //
                    // While you are processing this buffer, the board is already filling the next
                    // available buffer(s).
                    //
                    // You MUST finish processing this buffer and post it back to the board before
                    // the board fills all of its available DMA buffers and on-board memory.
                    //
                    // Records are arranged in the buffer as follows: R0A, R1A, R2A ... RnA, R0B,
                    // R1B, R2B ...
                    // with RXY the record number X of channel Y
                    //
                    // A 14-bit sample code is stored in the most significant bits of in each 16-bit
                    // sample value. 
                    // Sample codes are unsigned by default. As a result:
                    // - a sample code of 0x0000 represents a negative full scale input signal.
                    // - a sample code of 0x8000 represents a ~0V signal.
                    // - a sample code of 0xFFFF represents a positive full scale input signal.  
                    if (saveData)
                    {
                        // Write record to file
                        size_t bytesWritten = fwrite(pBuffer, sizeof(BYTE), bytesPerBuffer, fpData);
                        if (bytesWritten != bytesPerBuffer)
                        {
                            printf("Error: Write buffer %u failed -- %u\n", buffersCompleted,
                                GetLastError());
                            success = FALSE;
                        }
                    }
                }

                // Add the buffer to the end of the list of available buffers.
                if (success)
                {
                    retCode = AlazarPostAsyncBuffer(boardHandle, pBuffer, bytesPerBuffer);
                    if (retCode != ApiSuccess)
                    {
                        printf("Error: AlazarPostAsyncBuffer failed -- %s\n",
                            AlazarErrorToText(retCode));
                        success = FALSE;
                    }
                }

                // If the acquisition failed, exit the acquisition loop
                if (!success)
                    break;

                // If a key was pressed, exit the acquisition loop
                if (_kbhit())
                {
                    printf("Aborted...\n");
                    break;
                }
                // If the acquistion period is over, abort the acquisition
                if (GetTickCount() > stopTickCount)
                {
                    printf("Acquistion complete.\n");
                    break;
                }

                // Display progress
                printf("Completed %u buffers\r", buffersCompleted);
            }

            // Display results
            double transferTime_sec = (GetTickCount() - startTickCount) / 1000.;
            printf("Capture completed in %.2lf sec\n", transferTime_sec);

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

            printf("Captured %u buffers (%.4g buffers per sec)\n", buffersCompleted, buffersPerSec);
            printf("Captured %u records (%.4g records per sec)\n", recordsTransferred, recordsPerSec);
            printf("Transferred %I64d bytes (%.4g bytes per sec)\n", bytesTransferred, bytesPerSec);
        }

        // Abort the acquisition
        retCode = AlazarAbortAsyncRead(boardHandle);
        if (retCode != ApiSuccess)
        {
            printf("Error: AlazarAbortAsyncRead failed -- %s\n", AlazarErrorToText(retCode));
            success = FALSE;
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

        // Close the data file

        if (fpData != NULL)
            fclose(fpData);


        return;
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