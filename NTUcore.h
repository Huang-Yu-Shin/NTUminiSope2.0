#pragma once
// Important tokens file
#include "Tokens.h"
//#include "Additional.h"
#include <chrono>
#include <thread>
#include "wtypes.h"

// MEMS related
#include "MTIDevice.h"        // Includes all released functions
#include "MTIDefinitions.h"   // Includes structures: parameters, data modes, error codes, etc
#include "MTIDataGenerator.h" // Includes content generation and manipulation functions

// ETL lens
#include "ComCasp.h"

// openCV & CUDA Library
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include "opencv2/core/core_c.h"
#include <opencv2/cudaarithm.hpp>
#include "opencv2/photo/cuda.hpp"
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>

// ATS 9146 related
#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

// Reading file 
#include <cmath>
#include <filesystem>

namespace NTU
{
	using namespace std;
	using namespace std::chrono;
	using namespace cv;
	using namespace cv::cuda;
	
	
	class NTUscope 
	{
	public:
		// Constructor
		NTUscope();

		// Vital boolean variables control!!!
		bool allow_scan = true;

		// Initialization variable ==========================================================================
		volatile bool scanManager = false;
		volatile bool scanHOLD = false;
		volatile bool scanFREE = false;
		volatile bool scanAlive = false;
		volatile bool HoldFreeBusy = false;
		volatile bool scanState = SCAN_FREE;
		volatile bool constructorExecutedGUI;
		volatile bool threadEnd[100];
		volatile int initStatus = 0;
		volatile bool bin_export_16BIT;
		volatile int BiD_align_bin = 0;
		volatile bool biDirection_bin_file = false;
		int cropLeftPoints = 0;
		int distCropPixelAdjust = 0;
		

		// Global string & Arrangement of memory allocation
		char* result = (char*)malloc(200);
		char* ScanStatus;
		volatile bool bufferOnProcess = false;

		// POOL FOR INFO EXCHANGE ========
		char* GetScanStatus() { return ScanStatus; }

		// ETL 
		void etlControllerConfig();
		#define MAX_PORT_NUMBER 20;
		
		//3d scan
		void volumetricScanConfig();

		// Variables accessed in main GUI ==============
		void memsSync();
		volatile bool mems_thread_cont;
		volatile bool mems_repositioned;

		// PREVIEW BUFFERS
		U16* chan_1_prev_array_1;
		U16* chan_1_prev_array_2;

		// Multithread
		int NumCPU = 0;
		volatile bool rapidScanCont = false;
		void StreamChannel_1();
		volatile bool RShiftAllowed = false;
		// CHANNEL 1 IMAGE ARRAYS
		U16* chan_1_image_array_1[16100];
		U16* chan_1_image_array_2[16100];
		U32* chan_1_image_array_avg[16100];
		volatile int lines_filled_array_1;
		U16 points_gap;
		volatile int bi_direction_offset;
		bool one_frame_completed = false;
		std::string folderName;
		std::string folderName_binning;
		bool create_raw_folder = true;
		bool create_binning_folder = true;


		// CHANNEL 2 IMAGE ARRAYS
		U16* chan_2_image_array_1[16100];
		U16* chan_2_image_array_2[16100];
		U32* chan_2_image_array_avg[16100];

		// Calibration FOV
		float cal_FOV_x;
		float cal_FOV_y;
		float FOV_ratio;

		// Scan Buffer Control
		U16* bufferData;
		U16* pBuffer;

		// Multi thread control (1D array)
		
		std::chrono::high_resolution_clock::time_point temp_t1;
		std::chrono::high_resolution_clock::time_point temp_t2;
		std::chrono::duration<double> frame_to_frame;
		volatile double AcquisitionTime;
		
		//volatile double Frame_Rate;
		
		// 3D stage settings ==========================================================================
		volatile int count_completed_frames;

		// Zaber stage related
		int move_distance;
		bool move_absolute_position;
		bool move_relative_position;
		string unit;
		volatile int num_avg_frame_temp;
		volatile int num_avg_frame;
		volatile int countMoveAction = 0;
		volatile bool first_avg_frame;
		volatile int scale_down_preview;
		volatile int scale_down_preview_temp;
		volatile int scale_down_prev_bin_read_temp;

		// MEMS related
		// Global Primary Object 
		MTIDevice* mti = new MTIDevice();
		volatile bool MEMS_status;
		volatile bool mems_start_reOrient;
		volatile bool memsModeShifted;
		volatile bool memshiftEnable = false;
		volatile bool memshiftStarted = false;
		volatile bool forceTrigRequest = false;
		volatile bool mems_keep_running;
		volatile bool bi_direction;
		volatile bool uni_Directional;
		double xOffset;
		double yOffset;
		int portnumber;
		float xPosition;
		float yPosition;
		int headMount_count;
		unsigned int mSec;
		int y_length;
		int prev_Y_length;
		float xAmp;
		float yAmp;
		int scanInt();
		void setCalVal();
		void memsControllerConfig();
		bool RShiftMemsHandle(bool RShiftEN);
		bool SelectIODevice(MTIDevice* mti, char* comPortName);
		void CalculateSinglePointTrackingParameters(unsigned int& hardwareFilterBw, unsigned int& sampleRate, float& dataScale, unsigned int& frameRate);
		void Uni_RasterScan(MTIDevice* mti, float xAmp, float yAmp, int y_length);
		void Bi_RasterScan(MTIDevice* mti, float xAmp, float yAmp, int y_length);
		void SpiralCurve(MTIDevice* mti, float xAmplitude, float yAmplitude, unsigned int frequency);
		unsigned int CalculateMinHwBw(unsigned int sampleRate);
		void CalculateImagingParameters(unsigned int& hardwareFilterBw, unsigned int& sampleRate, float& dataScale, float& lineTime, unsigned int& nLines, unsigned int& nPixels);
		unsigned int EstimateXYDelay(unsigned int SampleRate, unsigned int HardwareFilterBw);
		//void Imaging(MTIDevice* mti, float xOffset, float yOffset, float dataScale);
		unsigned int img_size = 600;
		unsigned int trackPersistenceSize = 30;
		Point pixels[100];
		bool binCroppedExist();
		void initCUDA();
		double extract_data_from_string(char char_word[500]);

		// Get the horizontal and vertical screen sizes in pixel
		void GetDesktopResolution(int& horizontal, int& vertical)
		{
			RECT desktop;
			// Get a handle to the desktop window
			const HWND hDesktop = GetDesktopWindow();
			// Get the size of screen to the variable desktop
			GetWindowRect(hDesktop, &desktop);
			// The top left corner will have coordinates (0,0)
			// and the bottom right corner will have coordinates
			// (horizontal, vertical)
			horizontal = desktop.right;
			vertical = desktop.bottom;
		}
		int horizontal;
		int vertical;
		float AR;
		bool download_raw_image;
		int raw_frame_saving_progress_chan_1;
		int raw_frame_count;

		// ATS9146 variables (Configure & Acquisition settings)
		char* config_Alazar(double internal_clock_freq,	bool internal_clock, bool external_clock);
		char* update_Settings(double internal_clock_freq, bool internal_clock, bool external_clock);
		void reArm();
		char* set_internal_clock_freq(HANDLE boardHandle, double freq);
		void start_scan();
		void bufferHandler();
		void deAllocate();
		void TerminateAll();
		void getATSTemp();
		void infiniteScan();
		#define BUFFER_COUNT 20
		U16* BufferArray[BUFFER_COUNT] = { NULL };
		U32 min_allowed_line = 16;
		U32 recordPerBuffer = 1;
		U32 channel_Count;
		U32 channels_Per_Board = 2;
		U32 channel_Mask = CHANNEL_A;
		double acquisitionLength_sec = 1.;
		double bytesPerSample;
		U32 bytesPerRecord;
		U32 bytesPerBuffer;
		U32 MAXBytesPerBuffer;
		U32 bufferIndex = 0;
		U32 admaFlag = ADMA_EXTERNAL_STARTCAPTURE | ADMA_NPT;
		float temp_num_points_per_line;
		float lineTime;
		double find_1st_Trig[1024];
		U32 preTriggerSamples = 0;
		U32 postTriggerSamples;
		U32 samplesPerRecord;
		double samplePerSec;
		U32 prev_sampPerRec;
		U32 buffersCompleted;
		U32 buffersPerAcquisition;
		int recordsPerAcquisition;
		U32 parameter = GET_FPGA_TEMPERATURE;
		U8 channel = CHANNEL_ALL;
		float fpgaTemperature_degreesC;
		// TODO: Select if you wish to save the sample data to a file
		bool saveData;
		const int ATS_WAIT_MS = 4000;
		volatile bool engine_started;
		volatile bool running_loops;
		bool conScan;
		int frame_count;

		// Data Streaming
		HANDLE boardHandle;
		RETURN_CODE retCode;
		// Read the record timestamp
		U32 triggerAddress;
		U32 timestampHigh;
		U32 timestampLow;
		void getTrigTimeStamp();
		U32 get_postTrigSample();
		//void update_recordPerBuffer(bool bi_direction, int y_length) { recordPerBuffer = (bi_direction ? y_length: min_allowed_line); }
		void update_recordPerBuffer(bool bi_direction, int y_length) { recordPerBuffer = (bi_direction ? recordPerBuffer=y_length:1); }
		U32 get_recordPerBuffer() { return recordPerBuffer= y_length; }	//recordPerBuffer = (y_length == 8) ? 8 : min_allowed_line; 
		U32 get_buffersPerAcquisition(bool bi_direction, int y_length) { return(bi_direction ? y_length/y_length : 1 ); }
		//U32 get_buffersPerAcquisition(bool bi_direction, int y_length) { return(bi_direction ? y_length/min_allowed_line: 0x7FFFFFFF); }
		U32 get_recordsPerAcquisition(bool bi_direction, int y_length, U32 buffersPerAcquisition) { return(bi_direction ? (recordPerBuffer * buffersPerAcquisition): 0x7FFFFFFF); }
		//U32 get_recordsPerAcquisition(bool bi_direction, int y_length, U32 buffersPerAcquisition) { return(bi_direction ? (recordPerBuffer * buffersPerAcquisition): 0x7FFFFFFF); }
		bool startCapture(bool bi_direction, HANDLE boardHandle);
		void executeScan();
		Mat img;
		Mat resize_img;
		Mat imageColorMat;
		double refreshRate;
		std::chrono::high_resolution_clock::time_point startTimePoint;
		std::chrono::high_resolution_clock::time_point starting_time;
		std::chrono::high_resolution_clock::time_point elapsed_time;
		std::chrono::duration<double> one_frame_duration;
		
		std::chrono::high_resolution_clock::time_point trigStartTime;
		std::chrono::high_resolution_clock::time_point trigNextTime;
		std::chrono::duration<double> one_trig_duration;

		std::chrono::high_resolution_clock::time_point StartTime;
		std::chrono::high_resolution_clock::time_point EndTime;
		std::chrono::duration<double> durationn;
		double long_duration[1024];
		double transferTime_sec;
		double AcqTime;
		double FrameRate;
		void PrevChannel_1();
		volatile bool RapidStarted[4] = { false,false,false,false };
		volatile bool chan_merge;
		// Initialization GUI variable
		int beta_chan_1;
		double alpha_chan_1;
		double gamma_chan_1;
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
			Stream& streamCUDA);
		void ApplyFilter(
			int CHAN,
			int MODE,
			GpuMat& IMGDP8,
			Stream& streamCUDA);

		// Data saving


		// IMAGE ENHANCEMENT ==========================================================================
		volatile int* filtersID[4];
		volatile int* filtersID_bin[4];
		volatile bool filters_allowed;

		// GAUSSIAN FILTER
		//volatile bool activate_gaussian_bin_file[4];
		volatile bool apply_gaussian;
		volatile int gaussian_kernel_bin_file[4];
		volatile int gaussian_sigmaH_bin_file[4];
		volatile int gaussian_sigmaV_bin_file[4];

		// MEDIAN BLUR FILTER
		//volatile bool activate_median_bin_file[4];
		volatile bool apply_median;
		volatile int median_kernel_bin_file[4];

		// HISTOGRAM EQUALIZATION : CLAHE
		//volatile bool activate_CLAHE[4];
		//volatile bool activate_CLAHE_bin_file[4];
		volatile bool apply_CLAHE;
		volatile int CLAHE_clip_limit[4];
		volatile int CLAHE_clip_limit_bin_file[4];
		volatile int CLAHE_tile_grid_H[4];
		volatile int CLAHE_tile_grid_H_bin_file[4];
		volatile int CLAHE_tile_grid_V[4];
		volatile int CLAHE_tile_grid_V_bin_file[4];

		
		//volatile bool ExecutedGUI = false;
		void initializeAll();
		int zoomValueLimit[2] = { 400, 100 };	// cols | rows
		int FOVfpsValueLimit[2] = { 600, 200 };	// cols | rows
		double FOV[2] = { 0.0, 0.0 };
		volatile bool pause_Prev;

		// Action for GUI
		static const int maxActTags = 100;
		char* actTagGUI[maxActTags];
		int actInd = 0;

		// Global vector
		int coor_x[10000] = { 0 };
		int coor_y[10000] = { 0 };
		int minCoor;
		unsigned int numPoints = 10000;
		int npts;
		float max = 0;
		

		// Some vital controls
		bool debug_window;
		bool Go_allow_configure();
		//bool Go_allow_debug_window();
		bool Go_allow_connect();
		bool Go_allow_acquisition();
		bool Go_allow_spiral_scan();
		bool Go_allow_biDirectional();
		bool Go_allow_biDirectional_ext_trigger();

		// Scan & Stream
		void initiateScan();
		void HoldFreeScanning();
		bool startCapture(bool high_speed, bool ultra_fast, HANDLE boardHandle);
		void forceTrigThread();
		void scanMemoryManage(int mode);
		volatile bool ACQ_finished;
		volatile bool mergeDisRunning = false;

		// PREVIEW DISPLAY SETTINGS =======================================================================
		volatile bool img_thread_cont_chan_1;
		volatile bool global_GPU_busy;
		volatile int calibration_value;
		volatile int PixelCALIB[2], PixelCALIBPrev[4][2];
		volatile bool MemshiftStarted = false;

		// Preview functions
		volatile int chan_1_color;
		volatile int chan_2_color;
		volatile bool activate_chan_1;
		volatile bool activate_chan_2;
		volatile int colorArrayBackUp[10] = { 1,0,2,0,3,0,2,3 };
		void setChannelColors();
		int CROP_ZOOM_UP = cv::InterpolationFlags::INTER_AREA;
		int CROP_ZOOM_DOWN = cv::InterpolationFlags::INTER_AREA;
		// Merging Settings
		Mat m_red;
		Mat m_green;
		Mat m_blue;
		volatile bool red_already_filled;
		volatile bool green_already_filled;
		volatile bool blue_already_filled;
		volatile bool chan_2_pushed;
		volatile int totalSlice = 1;
		volatile double frame_saving_progress_chan_1;
		int img_count_temp;
		int raw_img_count_temp;
		

		// FRAME AVERAGING SETTINGS ==========================================================================
		volatile bool frame_averaging;
		volatile int num_avg_frame_new;
		volatile bool TERMINATE_RAPID = false;
		bool allow_averaging;

		// FILE SAVING SETTINGS	==========================================================================
		volatile bool frames_saving_cond_chan_1;
		volatile int num_frames_saved_chan_1;

		// CONFIGURE ==========================================================================
		bool initialize_frame_saving(
			char* brief_detail,
			char* full_date_incl_path);
		bool abort_frame_saving(
			float xAmp,
			float yAmp,
			bool internal_clock,
			bool external_clock,
			double internal_clock_freq,
			double external_clock_freq);

		// Define filter index
		#define COL_BLACK 0
		#define COL_RED 1
		#define COL_GREEN 2
		#define COL_BLUE 3
		#define COL_CYAN 4
		#define COL_YELLOW 5
		#define COL_MAGENTA 6

		volatile bool frame_saving_aborted;
		volatile bool save_data_chan_1;

		// YuShin system ==========================================================================
		#define YSBUFFER_COUNT 16
		#define YSTRANSIENT_BUFFER_COUNT 16

		int clocktype;
		//MEMS
		double MEMS_xamp;
		double MEMS_yamp;
		double xfreq;
		double yfreq;
		double xoffset;
		double yoffset;
		int num_lines;

		//ETL
		double zoffset;
		double zstep;
		int zlayer;

		//real-time imageprocessing
		bool savestate = false;
		bool saverawstate = false;
		int save_num = 0;
		bool avgstate = false;
		int avgcount = 0;
		bool imgprocessstate = false;
		bool clahestate = false;
		bool medianstate = false;
		bool gaussianstate = false;
		double brightness = 0;
		double constrat = 1;
		double gamma = 1;
		double clahe = 0;
		int median_kernal = 3;
		int gaussiankernal = 3;
		double gaussiansigmaH = 0;
		double gaussiansigmaV = 0;

		double zduration_per_change[YSTRANSIENT_BUFFER_COUNT] = { NULL };
		bool zcallback[YSTRANSIENT_BUFFER_COUNT];

		//Alazar
		int samplesPerSec = 0;
		double phase = 0;

		//NI
		int samplingRate = 0;


	private:
		// Bool parameters control by us (In "Management.cpp")
		volatile bool allow_debug_window;
		volatile bool allow_configure; 
		volatile bool allow_connect;
		volatile bool allow_acquisition;
		volatile bool allow_spiral_scan;
		volatile bool allow_raster_scan;
		volatile bool allow_bidirection;
		volatile bool allow_biDirectional_ext_trigger;
		volatile bool allow_save;
		volatile bool allow_alignment;
		volatile bool allow_offsets;
		volatile bool allow_filter;
		volatile bool allow_binView;
		volatile bool allow_export;

		// Emergency controls
		volatile bool pauseScanEmergency;
		volatile bool pauseSaveEmergency;
		volatile bool pauseBinViewEmergency;
		volatile bool pauseExportEmergency;
		volatile bool pauseMemsEmergency;

		// Char Strings
		char* full_time_incl_path;
		char* full_time_incl_path_chan_1;

		// Files for saving
		FILE* chan_1_file;
		FILE* image_file_info;

		// DATA STREAMING : FRAME SAVING
		volatile bool file_stream_cont_chan_1;
		char* chan_1_filename;
		char* chan_1_raw_filename;

		// File saving settings
		volatile bool frames_saving_init;

		// Merging Settings
		volatile bool chan_1_pushed;
		volatile bool chan_1_refresh_avg_win;
		volatile bool active_buffer_chan_1 = false;
		
		//void get_PC_ID();

		// Scan settings: Important Bools & Vars
		volatile bool image_array_1_filled;
		volatile int lines_averaged_array_2;

		// FILE SAVING SETTINGS
		volatile bool chan_1_frame_saving_status;
		volatile bool chan_1_prev_1_filled = false;
		volatile bool chan_1_prev_2_filled = false;
		volatile bool activate_frame_savinng_confirm;
		volatile bool received_first_frame;
		volatile int num_frames_summed;

		// Frame Averaging
		volatile int avg_frames_done_chan_1;
		volatile int lines_averaged_array_1;
		volatile bool image_array_2_filled;
		volatile int lines_filled_array_2;

		volatile int colorArray[10] = { 1,0,2,0,3,0,2,3 };
		
		volatile bool filled_array_2;

		volatile bool forward_move;
		volatile bool backward_move;
		INT64 total_points_before_trigger;
		INT64 prev_total_points_before_trigger;
		// For threaded data loading
		char full_path_selected_frame[1000];
};
}


