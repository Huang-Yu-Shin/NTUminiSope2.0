#pragma once

// Include header file
#include "NTUcore.h"
#include "Loading.h"
#include "YSNTUcore.h"
#include "YSCore.h"
#include "YSPostprocessing.h"
// Include file stream header file
#include <string>
#include <iostream>
#include <fstream>
//#include <consoleapi.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <vcclr.h>
#include <conio.h>

// Reading raw data location
char* defaultLoc = (char*)malloc(1000);
char export_directory_full[1000] = {};
char* full_date_incl_path = (char*)malloc(1000);
char* brief_detail = (char*)malloc(1000);

// Vital Enable / Disable settings
volatile bool GUI_status = false;
volatile bool busy_GUI = false;
volatile bool bin_under_view = false;
volatile bool refreshing = false;

// Important Settings
volatile bool configure_status = false;
volatile bool external_clock = false;
volatile bool internal_clock = true;
volatile bool frames_directory_loaded = false;
volatile bool export_going_on = false;
bool internal_clock_bin_file = false;
bool external_clock_bin_file = false;
double internal_clock_freq_bin_file = 0.0;
double external_clock_freq_bin_file = 0.0;
volatile int total_chan2save = 0;

// Clock frequency (Ext / Int)
double external_clock_freq = 0.0;
double internal_clock_freq = 0.0;
double external_clock_freq_array[7] = { 75.9,76.0,76.1,76.2,76.3,76.4 };
//double external_clock_freq_array[7] = { 80.054,80.154, 80.254,80.354,80.454,80.554, 80.654 };
double internal_clock_freq_array[6] = { 2, 10,20,50,100,125 };
double YSlinefreq_array[10] = {100,125,200,250,400,500,625,800,1000,1250};
int YSchopperdownscale_array[3] = { 1,2,4 };
int YSyline_array[7] = { 16,32,64,128,256,512,1024 };
double YSphase[8] = { 0 ,M_PI_4, M_PI_2 ,3*M_PI_4, M_PI , 5*M_PI_4,  3 * M_PI_2, 7*M_PI_4 };
std::string YSphasename[8] = {"0","PI/4" ,"PI/2","3PI/4" ,"PI","5PI/4", "3PI/2","7PI/4"};
U32 inputrange[3] = { INPUT_RANGE_PM_500_MV,INPUT_RANGE_PM_800_MV,INPUT_RANGE_PM_1_V };
// Timer
clock_t t_before, t_after;

// Creating globally accessible txt files
std::ofstream log_file;
std::ifstream read_cal;

namespace MTPFM {
	using namespace std;
	using namespace System;
	using namespace System::Diagnostics;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	// For threading multi-frames
	using namespace System::Threading;
	using namespace System::Threading::Tasks;
	using namespace std::chrono;
	// For oping Console and write something
	using namespace System::Runtime::InteropServices;
	// For doing task control
	using namespace System::IO;
	using namespace System::IO::Ports;
	using namespace System::Text;
	using namespace System::Media;
	using namespace System::Timers;
	using namespace cv;
	using namespace Zaber::Motion;
	using namespace Zaber::Motion::Ascii;


	/// <summary>
	/// Summary for MTPFM
	/// </summary>

	
	void MarshalString(System::String^ a, string& os) {
		const char* chars =
			(const char*)(Marshal::StringToHGlobalAnsi(a)).ToPointer();
		os = chars;
		Marshal::FreeHGlobal(IntPtr((void*)chars));
	}
	
	public ref class MTPFM : public System::Windows::Forms::Form
	{
	public:
		// Events 
		delegate void EventDelegate(
			System::Object^ sender,
			System::EventArgs^ e,
			System::String^ message);
		event EventDelegate^ delEvent;
		
		// CREATING THREAD OBJECTS
		Loading^ load_obj = gcnew Loading;

		// Global Primary Object 
		NTU::NTUscope* aNTU = new NTU::NTUscope;
		MTIDevice* mti = new MTIDevice();
		MTIDevice* mMTIDevice = new MTIDevice();
		Connection^ connection;
		Device^ device;
		Axis^ axis;
		double position;
		NTU::YSNTUscope* aYSNTU = new NTU::YSNTUscope;
		NTU::YSCore* YSCORE = new NTU::YSCore(aYSNTU);

	private: System::Windows::Forms::Button^ button2_abs_pos;
	private: System::Windows::Forms::RadioButton^ radioButton2_abs;
	private: System::Windows::Forms::RadioButton^ radioButton1_rel;
	private: System::Windows::Forms::Button^ button2_co_ex;
	private: System::Windows::Forms::Panel^ panelCollapse;
	private: System::Windows::Forms::Label^ label4;
	private: System::Windows::Forms::TabControl^ tabControl1;

	private: System::Windows::Forms::TabPage^ tabPage2;
	private: System::Windows::Forms::Button^ button2_open_image;
	private: System::Windows::Forms::TextBox^ textBox1_open_image;
	private: System::Windows::Forms::CheckBox^ checkBox1_save_ch1_data;

	private: System::Windows::Forms::CheckBox^ checkBox1_mode;









	private: System::Windows::Forms::Label^ label2_combo_y_length;
























	private: System::Windows::Forms::Button^ button_save_loaded_frame;
	private: System::Windows::Forms::ProgressBar^ progressBar_progress_exporting;
	private: System::Windows::Forms::TextBox^ textBox_progress_export;
	private: System::Windows::Forms::CheckBox^ checkBox_bin_export_full_frame;
	private: System::Windows::Forms::CheckBox^ checkBox_bin_export_cropped;
	private: System::Windows::Forms::ComboBox^ comboBox2__Ext_clk;
	private: System::Windows::Forms::ComboBox^ comboBox3_Int_clk;


	private: System::Windows::Forms::ComboBox^ comboBox_num_Y_lines;




	private: System::Windows::Forms::ComboBox^ comboBox_headmount_count;
	private: System::Windows::Forms::Label^ label11;

	private: System::Windows::Forms::Button^ button_clear_terminal;
	private: System::Windows::Forms::RadioButton^ radioButton_checkBox_Activate_chan_1;
	private: System::Windows::Forms::Label^ label12;

	private: System::Windows::Forms::CheckBox^ checkBox_merge_channels;
	private: CustomControls::DefineControls::ToggleButtons^ toggleButtons1;
	private: System::Windows::Forms::Button^ button_start_frames_saving;
	private: System::Windows::Forms::Timer^ timer_refresh_UI;
	private: System::Windows::Forms::TextBox^ textBox_acq_time;
	private: System::Windows::Forms::Label^ label2;

	private: System::Windows::Forms::Label^ label13;
	private: System::Windows::Forms::ComboBox^ comboBox_prev_scale_down;
	private: System::Windows::Forms::ComboBox^ comboBox_prev_scale_down_bin_read;
	private: System::Windows::Forms::Label^ label14;
	private: System::Windows::Forms::CheckBox^ checkBox_save_data_alazar;
	private: System::Windows::Forms::TextBox^ textBox_brief_detail;
private: System::Windows::Forms::TrackBar^ trackBar_BiD_align_bin;
private: System::Windows::Forms::Button^ button_load_directory;

private: System::Windows::Forms::Label^ label_BiD_align_Val_bin;
private: System::Windows::Forms::Label^ label_BiD_align_bin;
private: System::Windows::Forms::Label^ label15;

private: System::Windows::Forms::CheckBox^ checkBox_Export16BIT;
private: System::Windows::Forms::Label^ label1_num_frame;

private: System::Windows::Forms::TrackBar^ trackBar_num_frame;
private: System::Windows::Forms::Label^ label_frame_num;

private: System::Windows::Forms::Label^ labelraw_count;

private: System::Windows::Forms::TrackBar^ trackBar_raw_count;

private: System::Windows::Forms::Label^ label17;
private: System::Windows::Forms::TextBox^ textBox_progress_raw_ch1;
private: System::Windows::Forms::CheckBox^ checkBox1_raw_ch1;
private: System::Windows::Forms::ProgressBar^ progressBar_raw_ch1;
private: System::Windows::Forms::Button^ button2_save_data;
private: System::Windows::Forms::Button^ button_raw_save;


private: System::Windows::Forms::TabControl^ tabControl3;
private: System::Windows::Forms::TabPage^ tabPage1;
private: System::Windows::Forms::TabPage^ tabPage5;

private: System::Windows::Forms::TrackBar^ trackBar_adjust_gamma;

private: System::Windows::Forms::TrackBar^ trackBar_brightness_beta;

private: System::Windows::Forms::Label^ label20;
private: System::Windows::Forms::Label^ label19;
private: System::Windows::Forms::Label^ label18;
private: System::Windows::Forms::Label^ label_adjust_gamma;


private: System::Windows::Forms::Label^ label_contrast_alpha;



private: System::Windows::Forms::Label^ label_brightness_beta;



private: System::Windows::Forms::TrackBar^ trackBar_contrast_alpha;
private: System::Windows::Forms::TabControl^ tabControl4;
private: System::Windows::Forms::TabPage^ tabPage6;
private: System::Windows::Forms::TabPage^ tabPage7;


private: System::Windows::Forms::TabPage^ tabPage10;
private: System::Windows::Forms::TabPage^ tabPage11;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_Gaussian;

private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_Median;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_CLAHE;





private: System::Windows::Forms::TrackBar^ trackBar_Gaussian_sigmaV;

private: System::Windows::Forms::TrackBar^ trackBar_Gaussian_sigmaH;

private: System::Windows::Forms::TrackBar^ trackBar_Gaussian_kernel;
private: System::Windows::Forms::TrackBar^ trackBar_Median_kernel;




private: System::Windows::Forms::Label^ label_Gaussian_kernel;
private: System::Windows::Forms::Label^ label_Gaussian_sigmaH;
private: System::Windows::Forms::Label^ label_Gaussian_sigmaV;
private: System::Windows::Forms::Label^ label_Median_kernel;
private: System::Windows::Forms::Label^ label_tile_grid_V;

private: System::Windows::Forms::Label^ label_tile_grid_H;
private: System::Windows::Forms::Label^ label_clip_limit;
private: System::Windows::Forms::TrackBar^ trackBar_tile_grid_V;
private: System::Windows::Forms::TrackBar^ trackBar_tile_grid_H;
private: System::Windows::Forms::TrackBar^ trackBar_clip_limit;
private: System::Windows::Forms::Label^ label21;
private: System::Windows::Forms::Panel^ panel1;
private: System::Windows::Forms::TextBox^ textBox_refresh_rate;
private: System::Windows::Forms::TabPage^ tabPage8;
private: System::Windows::Forms::Label^ label2_average_count;
private: System::Windows::Forms::TrackBar^ trackBar_average_num_count;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_allow_averaging;



























































private: System::Windows::Forms::TabControl^ YSbutton_lissajous_setamp;

private: System::Windows::Forms::TabPage^ tabPage16;
private: System::Windows::Forms::ComboBox^ YScomboBox__raster_linefreq;

private: System::Windows::Forms::Label^ label31;

private: System::Windows::Forms::TextBox^ YStextBox__raster_yoffset;



private: System::Windows::Forms::TextBox^ YStextBox__raster_xoffset;

private: System::Windows::Forms::Label^ label22;
private: System::Windows::Forms::Label^ label24;


private: System::Windows::Forms::TrackBar^ YStrackBar__raster_y_amp;

private: System::Windows::Forms::Label^ label26;
private: System::Windows::Forms::TrackBar^ YStrackBar__raster_x_amp;

private: System::Windows::Forms::Label^ label29;
private: System::Windows::Forms::TabPage^ tabPage15;
private: System::Windows::Forms::TabPage^ tabPage17;
private: System::Windows::Forms::Label^ label23;
private: System::Windows::Forms::Panel^ panel2;
private: System::Windows::Forms::TextBox^ textBox_ETL_FL;

private: System::Windows::Forms::TextBox^ textBox_ETL_voltage;


private: System::Windows::Forms::Label^ label27;
private: System::Windows::Forms::Button^ button_ETL_up;

private: System::Windows::Forms::Label^ label28;
private: System::Windows::Forms::ComboBox^ YScomboBox__raster_yline;

private: System::Windows::Forms::Label^ label32;

private: System::Windows::Forms::Label^ label33;

private: System::Windows::Forms::Label^ label34;

private: System::Windows::Forms::TextBox^ YStextBox__lissajous_yoffset;
private: System::Windows::Forms::TextBox^ YStextBox__lissajous_xoffset;






private: System::Windows::Forms::Label^ label36;
private: System::Windows::Forms::Label^ label37;


private: System::Windows::Forms::TrackBar^ YStrackBar__lissajous_y_amp;

private: System::Windows::Forms::Label^ label39;
private: System::Windows::Forms::TrackBar^ YStrackBar__lissajous_x_amp;

private: System::Windows::Forms::Label^ label40;
private: System::Windows::Forms::Button^ YSbutton_raster_stop;
private: System::Windows::Forms::Button^ YSbutton_raster_configure;



private: System::Windows::Forms::TextBox^ YStextBox_crnt_po_y;




private: System::Windows::Forms::TextBox^ YStextBox_crnt_po_x;

private: System::Windows::Forms::Label^ label41;
private: System::Windows::Forms::Label^ label42;
private: System::Windows::Forms::Label^ label43;
private: System::Windows::Forms::TextBox^ YStextBox_y_po;

private: System::Windows::Forms::TextBox^ YStextBox_x_po;

private: System::Windows::Forms::Label^ label44;
private: System::Windows::Forms::Label^ label45;
private: System::Windows::Forms::Button^ YSbutton_manual_control;
private: System::Windows::Forms::Button^ button_ETL_downdown;


private: System::Windows::Forms::Button^ button_ETL_upup;
private: System::Windows::Forms::Button^ button_ETL_down;


private: System::Windows::Forms::Button^ YSbutton_raster_startimg;

private: System::Windows::Forms::Button^ button9;
private: System::Windows::Forms::Button^ YSbutton_lissajous_startimg;
private: System::Windows::Forms::Button^ YSbutton_lissajous_stop;
private: System::Windows::Forms::Button^ YSbutton_lissajous_configure;



private: System::Windows::Forms::TextBox^ YStextBox__lissajous_y_freq;
private: System::Windows::Forms::TextBox^ YStextBox__lissajous_x_freq;
private: System::Windows::Forms::Button^ YSbutton_manual_stopcontrol;



private: System::Windows::Forms::TabPage^ tabPage13;


private: System::Windows::Forms::ComboBox^ YScomboBox_intclock;

private: System::Windows::Forms::CheckBox^ YScheckBox_extclock;
private: System::Windows::Forms::CheckBox^ YScheckBox_intclock;
private: System::Windows::Forms::TabControl^ tabControl5;
private: System::Windows::Forms::TabPage^ tabPage14;


private: System::Windows::Forms::TabPage^ tabPage18;


private: System::Windows::Forms::TabPage^ tabPage19;
private: System::Windows::Forms::Label^ label30;
private: System::Windows::Forms::ComboBox^ YScomboBox_headmount;

private: System::Windows::Forms::CheckBox^ YScheckBox_saveraw;

private: System::Windows::Forms::Button^ YSbutton_save;

private: System::Windows::Forms::ProgressBar^ progressBar1;
private: System::Windows::Forms::Label^ YSlabel_savenum;

private: System::Windows::Forms::CheckBox^ YScheckBox_ch1;
private: System::Windows::Forms::TrackBar^ YStrackBar_savenum;


private: System::Windows::Forms::TextBox^ textBox2;
private: System::Windows::Forms::Label^ label38;





private: System::Windows::Forms::Label^ YSlabel_raster_phase;

private: System::Windows::Forms::TrackBar^ YStrackBar_raster_phase;

private: System::Windows::Forms::Label^ label46;
private: System::Windows::Forms::Label^ YSlabel__lissajous_phase;
private: System::Windows::Forms::TrackBar^ YStrackBar__lissajous_phase;
private: System::Windows::Forms::Label^ label47;
private: System::Windows::Forms::CheckBox^ YScheckBox_lissajous_samplemapsave;
private: System::Windows::Forms::CheckBox^ YScheckBox_raster_slowspeed;
private: System::Windows::Forms::Label^ label48;
private: System::Windows::Forms::Label^ label35;
private: System::Windows::Forms::Label^ YSlabel__lissajous_offsettime;

private: System::Windows::Forms::TrackBar^ YStrackBar__lissajous_offsettime;

private: System::Windows::Forms::Label^ label49;
private: System::Windows::Forms::TextBox^ YStextBox__raster_y_amp;
private: System::Windows::Forms::TextBox^ YStextBox__raster_x_amp;
private: System::Windows::Forms::Button^ YSbutton_raster_setamp;
private: System::Windows::Forms::TextBox^ YStextBox__lissajous_y_amp;
private: System::Windows::Forms::TextBox^ YStextBox__lissajous_x_amp;
private: System::Windows::Forms::Button^ YSbutton__lissajous_setamp;
private: System::Windows::Forms::TabControl^ tabControl2;
private: System::Windows::Forms::TabPage^ tabPage3;
private: System::Windows::Forms::Button^ button_start_scan;
private: System::Windows::Forms::TextBox^ textBox_yoffset;
private: System::Windows::Forms::TextBox^ textBox_xoffset;
private: System::Windows::Forms::TextBox^ textBox_line_time;
private: System::Windows::Forms::Label^ label1_yoffset;
private: System::Windows::Forms::Label^ label_xoffset;
private: System::Windows::Forms::Label^ label1_sec;
private: System::Windows::Forms::Button^ button_acq;
private: System::Windows::Forms::Label^ label10;
private: System::Windows::Forms::Label^ label_bi_direction_offset;
private: System::Windows::Forms::Button^ button1_connect_to_controller;
private: System::Windows::Forms::TrackBar^ trackBar_phase;
private: System::Windows::Forms::Label^ label7_raster_y_amp;
private: System::Windows::Forms::Label^ label_phase;
private: System::Windows::Forms::TrackBar^ trackBar3_raster_y_amp;
private: System::Windows::Forms::Label^ label6_raster_y_amp;
private: System::Windows::Forms::Button^ button_Stop;
private: System::Windows::Forms::TrackBar^ trackBar2__raster_x_amp;
private: System::Windows::Forms::Label^ label4__raster_x_amp;
private: System::Windows::Forms::Label^ label3_raster_x_amp;
private: System::Windows::Forms::TabPage^ tabPage4;
private: System::Windows::Forms::TextBox^ textBox_crnt_po_y;
private: System::Windows::Forms::TextBox^ textBox_crnt_po_x;
private: System::Windows::Forms::TextBox^ textBox_movingtime;
private: System::Windows::Forms::TextBox^ textBox_y_pos;
private: System::Windows::Forms::TextBox^ textBox_x_po;
private: System::Windows::Forms::Label^ label8;
private: System::Windows::Forms::Label^ label_Y_current;
private: System::Windows::Forms::Label^ label_x_current;
private: System::Windows::Forms::Button^ button_back_origin;
private: System::Windows::Forms::Label^ label_msec;
private: System::Windows::Forms::Label^ label_y_posi;
private: System::Windows::Forms::Label^ label_x_posi;
private: System::Windows::Forms::Button^ button1_manual_control;
private: System::Windows::Forms::TextBox^ YStextBox_title;
private: System::Windows::Forms::Label^ label25;
private: System::Windows::Forms::Button^ YSbutton_clear;
private: System::Windows::Forms::Label^ label50;
private: System::Windows::Forms::TextBox^ YStextBox__ExtClockFreq;
private: System::Windows::Forms::CheckBox^ YScheckBox_lissajous_repairshow;
private: System::Windows::Forms::TabPage^ tabPage9;
private: System::Windows::Forms::Label^ label53;
private: System::Windows::Forms::Label^ label_MaxVal;

private: System::Windows::Forms::TrackBar^ trackBar_MaxVal;

private: System::Windows::Forms::Label^ label51;
private: System::Windows::Forms::Label^ label_MinVal;

private: System::Windows::Forms::TrackBar^ trackBar_MinVal;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_contrast;
private: System::Windows::Forms::TextBox^ textBox_highspeedmode;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_minmax;
private: System::Windows::Forms::TabPage^ tabPage12;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_signalproxessing;
private: System::Windows::Forms::Label^ label52;
private: System::Windows::Forms::Label^ label_multiplySP;

private: System::Windows::Forms::TrackBar^ trackBar_multiplySP;

private: System::Windows::Forms::Label^ label55;
private: System::Windows::Forms::Label^ label_offsetvalueSP;

private: System::Windows::Forms::TrackBar^ trackBar_offsetvalueSP;




private: System::Windows::Forms::Label^ label54;
private: System::Windows::Forms::TextBox^ textBox_initial_position;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_initial_position;
private: System::Windows::Forms::Label^ label56;
private: System::Windows::Forms::Label^ label_DCE;
private: System::Windows::Forms::TrackBar^ trackBar_DCE;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_DCE;
private: System::Windows::Forms::ComboBox^ YScomboBox_inputrange;

private: System::Windows::Forms::Label^ label57;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_Binning;
private: System::Windows::Forms::Label^ label16;
private: System::Windows::Forms::Label^ label58;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_equalhistogram;
private: System::Windows::Forms::CheckBox^ YScheckBox_saverawbin;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_normalize;

private: System::Windows::Forms::Label^ label59;
private: System::Windows::Forms::Label^ label_normalizeMax;

private: System::Windows::Forms::TrackBar^ trackBar_normalizeMax;

private: System::Windows::Forms::Label^ label61;
private: System::Windows::Forms::Label^ label_normalizeMin;

private: System::Windows::Forms::TrackBar^ trackBar_normalizeMin;
private: System::Windows::Forms::CheckBox^ checkBox_saveprocessedimage;
private: System::Windows::Forms::Label^ label60;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_defaltimageprocessing;
private: System::Windows::Forms::Label^ label62;
private: System::Windows::Forms::Label^ label_DCEkernel;
private: System::Windows::Forms::TrackBar^ trackBar_DCEkernel;
private: System::Windows::Forms::TextBox^ textBox_chopper_freq;

private: System::Windows::Forms::Label^ label63;
private: System::Windows::Forms::ComboBox^ YScomboBox__raster_chopperfreq;

private: System::Windows::Forms::Label^ label64;
private: System::Windows::Forms::Label^ label65;





















































































	public:
	public:
	public:
		// DRAG & DROP FILES
		cli::array<System::String^>^ FILES;
		

		// Constructor
		MTPFM()
		{
			// Duplicate and overlap the mti object from NTUcore.h to MTPFM.h
			mti = aNTU->mti;

			// Starting the Loading Task
			Task^ GUI = Task::Factory->StartNew(gcnew Action(this, &MTPFM::Thread_Loading_GUI));
			// Loading Progress
			load_obj->send_progress(0);

			t_before = clock();
			// Default Storage Location
			sprintf(defaultLoc, "%s", "C:\\Ian\\Imaging");

			// Open Console
			if (aNTU->debug_window)
			{
				AllocConsole();
				Console::WriteLine("Welcome to NTU miniScope!");
				Console::WriteLine("Developer information:");
				Console::WriteLine("-----------------------------------");
				Console::WriteLine("Name: Chung-En Huang");
				Console::WriteLine("Name: Yu-Shin Huang");
				Console::WriteLine("Lab: Ultra Fast Optics");
				Console::WriteLine("-----------------------------------");

				// For cout to work
				freopen("CONOUT$", "w", stdout);
			}
			// Updating Progress
			load_obj->send_progress(35);

			// Creating the directory for saving data
			std::string MTPFM_directory(defaultLoc);
			CreateDirectoryA(MTPFM_directory.c_str(), NULL);

			// ENABLING RISKY UI-CONTROL BY THREADS
			//CheckForIllegalCrossThreadCalls = false;
			
			// INITIALIZING THE COMPONENTS
			InitializeComponent();
			this->ControlBox = false;
			initComboBox();
			aNTU->initializeAll();

			this->Left = -10;
			this->Top = 0;

			// Bring to top
			this->TopMost = false;
			this->Top = true;
			this->Text = "NTU miniScope";

			// Updating Controls
			Updating_Controls();

			load_calibration_data();

			// Updating Progress
			load_obj->send_progress(65);

			// Brief detail initialize
			sprintf(brief_detail, "%s", "");

			aNTU->initCUDA();

			// EXTREMELY IMPORTANT SETTINGS FOR SCANNING
			// Getting the selected clock frequency
			int index_internal_clock_freq = comboBox3_Int_clk->SelectedIndex;
			internal_clock_freq = internal_clock_freq_array[index_internal_clock_freq];
			int index_external_clock_freq = comboBox2__Ext_clk->SelectedIndex;
			external_clock_freq = external_clock_freq_array[index_external_clock_freq];

			// Setting the Samples Per Record
			internal_clock = checkBox2__Int_clk->Checked;
			external_clock = checkBox1_Ext_clk->Checked;
			if (external_clock == true)		aNTU->samplePerSec = double(external_clock_freq) * pow(10, 6);
			else if (internal_clock == true)    aNTU->samplePerSec = double(internal_clock_freq) * pow(10, 6);

			// Getting post trigger samples : Points in 1 line
			aNTU->postTriggerSamples = aNTU->get_postTrigSample();

			// FIXING THE Samples Per Record
			aNTU->samplesPerRecord = aNTU->preTriggerSamples + aNTU->postTriggerSamples;

			// GETTING BIDIRECTIONAL OR UNIDIRECTIONAL
			aNTU->bi_direction = checkBox_biDirection->Checked;
			aNTU->uni_Directional = checkBox_uniDirection->Checked;

			// SELECTING NUMBER OF Y-LINES
			aNTU->y_length = System::Int32::Parse(comboBox_num_Y_lines->GetItemText(comboBox_num_Y_lines->SelectedItem));
			aNTU->buffersPerAcquisition = aNTU->y_length/ aNTU->get_recordPerBuffer();
			aNTU->recordsPerAcquisition = aNTU->get_recordPerBuffer() * aNTU->buffersPerAcquisition;

			// Calibration FOV
			aNTU->FOV_ratio = aNTU->cal_FOV_y / aNTU->cal_FOV_x;

			printf("line time: %lf\n", aNTU->lineTime);
			printf("sampling rate: %lf\n", aNTU->samplePerSec);
			printf("post TRIG sample: %lu\n", aNTU->postTriggerSamples);
			printf("sample per record: %lu\n", aNTU->samplesPerRecord);
			printf("y length: %d\n", aNTU->y_length);
			printf("FOV calibration: %lf\n", aNTU->FOV_ratio);
			printf("record per buffer: %d\n", aNTU->get_recordPerBuffer());
			printf("buffer per acq: %d\n", aNTU->buffersPerAcquisition);
			printf("record per acq: %d\n", aNTU->recordsPerAcquisition);
			printf("-----------------------------------\n\n");
			printf("--- Wait until stage go HOME!! ---\n\n");

			// Loading progress
			load_obj->send_progress(75);

			YSCORE->initialize();
			this->YScomboBox_inputrange->SelectedIndex = 2;
			this->YScomboBox_intclock->SelectedIndex = 3;
			this->YScomboBox__raster_linefreq->SelectedIndex = 9;
			this->YScomboBox__raster_yline->SelectedIndex = 4;
			this->YScomboBox__raster_chopperfreq->SelectedIndex = 2;
			// Loading progress
			load_obj->send_progress(85);

			// Initialized the wanted components
			// PUTTING NO-DATA STATUS : '---'
			textBox_frame_rate->Text = "-----";
			textBox_FOV_l->Text = "-----";
			textBox_FOV_r->Text = "-----";
			textBox_pixel_size_row->Text = "-----";
			textBox_pixel_size_col->Text = "-----";
			textBox1_open_image->Text = "-----";
			textBox_line_time->Text = aNTU->lineTime.ToString();

			// DISPLAY SCALE DOWN
			aNTU->scale_down_prev_bin_read_temp = comboBox_prev_scale_down_bin_read->SelectedIndex + 1;
			aNTU->scale_down_preview_temp = comboBox_prev_scale_down->SelectedIndex + 1;


			// BRIGHTNESS & CONTRAST SETTINGS
			aNTU->beta_chan_1 = trackBar_brightness_beta->Value;
			aNTU->alpha_chan_1 = trackBar_contrast_alpha->Value;
			aNTU->gamma_chan_1 = trackBar_adjust_gamma->Value;
			aNTU->alpha_chan_1 = aNTU->alpha_chan_1 / 10;
			aNTU->gamma_chan_1 = aNTU->gamma_chan_1 / 100;
			label_brightness_beta->Text = aNTU->beta_chan_1.ToString();
			label_contrast_alpha->Text = aNTU->alpha_chan_1.ToString();
			label_adjust_gamma->Text = aNTU->gamma_chan_1.ToString();

			// Zaber stage
			System::String^ number = "COM4";

			try
			{
				connection = connection->OpenSerialPort(number, 115200, true);
				device = connection->DetectDevices(true)[0];
				axis = device->GetAxis(1);
				position = axis->GetPosition(Units::Length_Micrometres);
				textBox_current_position->Text = position.ToString("f1");
			}

			catch (System::Exception^ exception)
			{
				status->Text = "Device not found! Try another COM";
				status->ForeColor = System::Drawing::Color::LightPink;
			}

			// CHANNELS TO SAVE
			aNTU->save_data_chan_1 = checkBox1_save_ch1_data->Checked;
			aNTU->download_raw_image = checkBox1_raw_ch1->Checked;

			// CHANNELS ACTIVATION SETTINGS
			aNTU->activate_chan_1 = radioButton_checkBox_Activate_chan_1->Checked;
			if (!aNTU->activate_chan_1)
			{
				radioButton_checkBox_Activate_chan_1->Enabled = false;
				progressBar_progress_exporting->Enabled = false;
				textBox_progress_export->Enabled = false;
			}

			// Collapse panel
			panelCollapse->Visible = false;
			panel_real_time->Visible = false;
			label_Real_time->Visible = false;
			this->Width = 420;

			aNTU->GetDesktopResolution(aNTU->horizontal, aNTU->vertical);
			//cout << aNTU->horizontal << '\n' << aNTU->vertical << '\n';

			// Loading progress
			load_obj->send_progress(100);

			// Deactivating debug window
			if (!aNTU->debug_window)
				FreeConsole();

			// END OF CONSTRUCTOR
			aNTU->constructorExecutedGUI = true;

			// Loading progress: Completes
			load_obj->send_progress(-1);

			// Process actions on GUI
			timer_Control_GUI->Interval = 100;
			timer_Control_GUI->Enabled = true;
			timer_Control_GUI->Start();

			// Refresh the GUI every single time
			timer_refresh_UI->Interval = 250;
			timer_refresh_UI->Enabled = true;
			timer_refresh_UI->Start();
		}

	// Thread for Splash Screen
	public: void Thread_Loading_GUI()
	{
		load_obj->ShowDialog();
	}
	
	public: void initComboBox()
	{
		// Setting ComboBox Defaults
		this->comboBox_num_Y_lines->SelectedIndex = 5;			    // Y-line #:256
		this->comboBox2__Ext_clk->SelectedIndex = 1;				// 76.0 MSps
		this->comboBox3_Int_clk->SelectedIndex = 2;					// 20 MSps
		this->comboBox_prev_scale_down->SelectedIndex = 0;			// prev_scale_down = None
		this->comboBox_prev_scale_down_bin_read->SelectedIndex = 0;	// Bin Viewer Scale Down = None
	}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		//-----------------------------------------
		// DESTRUCTOR (EXECUTES AT EXITING)
		~MTPFM()
		{
			aNTU->scanManager = false;
			aNTU->running_loops = false;
			aNTU->mems_thread_cont = false;
			//aNTU->bin_read_thread_cont = false;
			aNTU->img_thread_cont_chan_1 = false;
			Sleep(10);

			delete aYSNTU;
			delete YSCORE;

			// Deleting components
			if (components)					delete components;
			if (log_file.is_open())			log_file.close();

			// Wait for GPU to be idle
			while (aNTU->global_GPU_busy)	Sleep(100);
		}
private: 
	
	
	
	System::Windows::Forms::Panel^ panel_real_time;
private: System::Windows::Forms::Label^ label_Real_time;
protected:




	private: System::Windows::Forms::Panel^ panel_Stage;
	private: System::Windows::Forms::Button^ button_HOME;
	private: System::Windows::Forms::Label^ label5;
	private: System::Windows::Forms::Button^ button1_UP;

	private: System::Windows::Forms::Button^ button1_DOWN;
    private: System::Windows::Forms::TextBox^ textBox_current_position;
	private: System::Windows::Forms::Label^ label3;
	private: System::Windows::Forms::Label^ label_current_position;
	private: System::Windows::Forms::Label^ label7;
	private: System::Windows::Forms::TextBox^ textBox_abs_position;
	private: System::Windows::Forms::TextBox^ textBox3_relative_pos;
	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::Label^ label9;
	private: System::Windows::Forms::Label^ label6;
	private: System::Windows::Forms::CheckBox^ checkBox1_Ext_clk;
	private: System::Windows::Forms::CheckBox^ checkBox2__Int_clk;




	private: System::Windows::Forms::Timer^ timer1_acq;
	private: System::Windows::Forms::Label^ label1_Main_Control_Panel;
	private: System::Windows::Forms::Timer^ timer_Control_GUI;
	private: System::ComponentModel::IContainer^ components;
	private: System::Windows::Forms::Button^ button_shut_down_MEMS;
	private: System::Windows::Forms::Button^ Configure;
	private: System::Windows::Forms::TextBox^ status;

	private: System::Windows::Forms::Label^ label_x;
	private: System::Windows::Forms::TextBox^ textBox_frame_rate;
private:
private:
	private: System::Windows::Forms::TextBox^ textBox_pixel_size_col;
	private: System::Windows::Forms::TextBox^ textBox_FOV_r;
	private: System::Windows::Forms::TextBox^ textBox_FOV_l;
	private: System::Windows::Forms::Label^ label_frame_rate;
	private: System::Windows::Forms::Label^ label_pixel_size;
	private: System::Windows::Forms::Label^ label_FOV;
	private: System::Windows::Forms::TextBox^ textBox_pixel_size_row;
	private: System::Windows::Forms::Label^ label1;
	private: System::Windows::Forms::CheckBox^ checkBox_biDirection;
	private: System::Windows::Forms::CheckBox^ checkBox_uniDirection;



	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MTPFM::typeid));
			this->checkBox_biDirection = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox_uniDirection = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox2__Int_clk = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox1_Ext_clk = (gcnew System::Windows::Forms::CheckBox());
			this->timer1_acq = (gcnew System::Windows::Forms::Timer(this->components));
			this->label1_Main_Control_Panel = (gcnew System::Windows::Forms::Label());
			this->textBox_pixel_size_row = (gcnew System::Windows::Forms::TextBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label_x = (gcnew System::Windows::Forms::Label());
			this->textBox_frame_rate = (gcnew System::Windows::Forms::TextBox());
			this->textBox_pixel_size_col = (gcnew System::Windows::Forms::TextBox());
			this->textBox_FOV_r = (gcnew System::Windows::Forms::TextBox());
			this->textBox_FOV_l = (gcnew System::Windows::Forms::TextBox());
			this->label_frame_rate = (gcnew System::Windows::Forms::Label());
			this->label_pixel_size = (gcnew System::Windows::Forms::Label());
			this->label_FOV = (gcnew System::Windows::Forms::Label());
			this->timer_Control_GUI = (gcnew System::Windows::Forms::Timer(this->components));
			this->Configure = (gcnew System::Windows::Forms::Button());
			this->status = (gcnew System::Windows::Forms::TextBox());
			this->button_shut_down_MEMS = (gcnew System::Windows::Forms::Button());
			this->panel_real_time = (gcnew System::Windows::Forms::Panel());
			this->textBox_chopper_freq = (gcnew System::Windows::Forms::TextBox());
			this->label63 = (gcnew System::Windows::Forms::Label());
			this->label21 = (gcnew System::Windows::Forms::Label());
			this->textBox_acq_time = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->panel1 = (gcnew System::Windows::Forms::Panel());
			this->textBox_refresh_rate = (gcnew System::Windows::Forms::TextBox());
			this->label_Real_time = (gcnew System::Windows::Forms::Label());
			this->panel_Stage = (gcnew System::Windows::Forms::Panel());
			this->label54 = (gcnew System::Windows::Forms::Label());
			this->textBox_initial_position = (gcnew System::Windows::Forms::TextBox());
			this->toggleButtons_initial_position = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->radioButton2_abs = (gcnew System::Windows::Forms::RadioButton());
			this->radioButton1_rel = (gcnew System::Windows::Forms::RadioButton());
			this->button2_abs_pos = (gcnew System::Windows::Forms::Button());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->textBox_abs_position = (gcnew System::Windows::Forms::TextBox());
			this->textBox3_relative_pos = (gcnew System::Windows::Forms::TextBox());
			this->textBox_current_position = (gcnew System::Windows::Forms::TextBox());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->button1_DOWN = (gcnew System::Windows::Forms::Button());
			this->label_current_position = (gcnew System::Windows::Forms::Label());
			this->button1_UP = (gcnew System::Windows::Forms::Button());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->button_HOME = (gcnew System::Windows::Forms::Button());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->button2_co_ex = (gcnew System::Windows::Forms::Button());
			this->panelCollapse = (gcnew System::Windows::Forms::Panel());
			this->label60 = (gcnew System::Windows::Forms::Label());
			this->toggleButtons_defaltimageprocessing = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->checkBox_Export16BIT = (gcnew System::Windows::Forms::CheckBox());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->tabControl3 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage13 = (gcnew System::Windows::Forms::TabPage());
			this->checkBox_saveprocessedimage = (gcnew System::Windows::Forms::CheckBox());
			this->YScheckBox_saverawbin = (gcnew System::Windows::Forms::CheckBox());
			this->YSbutton_clear = (gcnew System::Windows::Forms::Button());
			this->label25 = (gcnew System::Windows::Forms::Label());
			this->YStextBox_title = (gcnew System::Windows::Forms::TextBox());
			this->YScheckBox_saveraw = (gcnew System::Windows::Forms::CheckBox());
			this->YSbutton_save = (gcnew System::Windows::Forms::Button());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->YSlabel_savenum = (gcnew System::Windows::Forms::Label());
			this->YScheckBox_ch1 = (gcnew System::Windows::Forms::CheckBox());
			this->YStrackBar_savenum = (gcnew System::Windows::Forms::TrackBar());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->label38 = (gcnew System::Windows::Forms::Label());
			this->tabPage8 = (gcnew System::Windows::Forms::TabPage());
			this->label48 = (gcnew System::Windows::Forms::Label());
			this->label35 = (gcnew System::Windows::Forms::Label());
			this->label2_average_count = (gcnew System::Windows::Forms::Label());
			this->trackBar_average_num_count = (gcnew System::Windows::Forms::TrackBar());
			this->toggleButtons_allow_averaging = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->tabPage1 = (gcnew System::Windows::Forms::TabPage());
			this->progressBar_raw_ch1 = (gcnew System::Windows::Forms::ProgressBar());
			this->checkBox1_raw_ch1 = (gcnew System::Windows::Forms::CheckBox());
			this->textBox_progress_raw_ch1 = (gcnew System::Windows::Forms::TextBox());
			this->button_raw_save = (gcnew System::Windows::Forms::Button());
			this->label17 = (gcnew System::Windows::Forms::Label());
			this->labelraw_count = (gcnew System::Windows::Forms::Label());
			this->trackBar_raw_count = (gcnew System::Windows::Forms::TrackBar());
			this->tabPage5 = (gcnew System::Windows::Forms::TabPage());
			this->button2_save_data = (gcnew System::Windows::Forms::Button());
			this->progressBar_progress_exporting = (gcnew System::Windows::Forms::ProgressBar());
			this->radioButton_checkBox_Activate_chan_1 = (gcnew System::Windows::Forms::RadioButton());
			this->label1_num_frame = (gcnew System::Windows::Forms::Label());
			this->checkBox1_save_ch1_data = (gcnew System::Windows::Forms::CheckBox());
			this->trackBar_num_frame = (gcnew System::Windows::Forms::TrackBar());
			this->textBox_progress_export = (gcnew System::Windows::Forms::TextBox());
			this->label_frame_num = (gcnew System::Windows::Forms::Label());
			this->tabControl4 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage12 = (gcnew System::Windows::Forms::TabPage());
			this->label16 = (gcnew System::Windows::Forms::Label());
			this->toggleButtons_Binning = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->toggleButtons_signalproxessing = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->label52 = (gcnew System::Windows::Forms::Label());
			this->label_multiplySP = (gcnew System::Windows::Forms::Label());
			this->trackBar_multiplySP = (gcnew System::Windows::Forms::TrackBar());
			this->label55 = (gcnew System::Windows::Forms::Label());
			this->label_offsetvalueSP = (gcnew System::Windows::Forms::Label());
			this->trackBar_offsetvalueSP = (gcnew System::Windows::Forms::TrackBar());
			this->tabPage19 = (gcnew System::Windows::Forms::TabPage());
			this->label62 = (gcnew System::Windows::Forms::Label());
			this->label_DCEkernel = (gcnew System::Windows::Forms::Label());
			this->trackBar_DCEkernel = (gcnew System::Windows::Forms::TrackBar());
			this->label56 = (gcnew System::Windows::Forms::Label());
			this->label_DCE = (gcnew System::Windows::Forms::Label());
			this->trackBar_DCE = (gcnew System::Windows::Forms::TrackBar());
			this->toggleButtons_DCE = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->toggleButtons_contrast = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->label_adjust_gamma = (gcnew System::Windows::Forms::Label());
			this->label18 = (gcnew System::Windows::Forms::Label());
			this->label_contrast_alpha = (gcnew System::Windows::Forms::Label());
			this->label19 = (gcnew System::Windows::Forms::Label());
			this->label_brightness_beta = (gcnew System::Windows::Forms::Label());
			this->label20 = (gcnew System::Windows::Forms::Label());
			this->trackBar_contrast_alpha = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_brightness_beta = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_adjust_gamma = (gcnew System::Windows::Forms::TrackBar());
			this->tabPage9 = (gcnew System::Windows::Forms::TabPage());
			this->toggleButtons_normalize = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->label59 = (gcnew System::Windows::Forms::Label());
			this->label_normalizeMax = (gcnew System::Windows::Forms::Label());
			this->trackBar_normalizeMax = (gcnew System::Windows::Forms::TrackBar());
			this->label61 = (gcnew System::Windows::Forms::Label());
			this->label_normalizeMin = (gcnew System::Windows::Forms::Label());
			this->trackBar_normalizeMin = (gcnew System::Windows::Forms::TrackBar());
			this->label58 = (gcnew System::Windows::Forms::Label());
			this->toggleButtons_equalhistogram = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->toggleButtons_minmax = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->label53 = (gcnew System::Windows::Forms::Label());
			this->label_MaxVal = (gcnew System::Windows::Forms::Label());
			this->trackBar_MaxVal = (gcnew System::Windows::Forms::TrackBar());
			this->label51 = (gcnew System::Windows::Forms::Label());
			this->label_MinVal = (gcnew System::Windows::Forms::Label());
			this->trackBar_MinVal = (gcnew System::Windows::Forms::TrackBar());
			this->tabPage6 = (gcnew System::Windows::Forms::TabPage());
			this->label_Gaussian_sigmaV = (gcnew System::Windows::Forms::Label());
			this->label_Gaussian_sigmaH = (gcnew System::Windows::Forms::Label());
			this->label_Gaussian_kernel = (gcnew System::Windows::Forms::Label());
			this->trackBar_Gaussian_sigmaV = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_Gaussian_sigmaH = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_Gaussian_kernel = (gcnew System::Windows::Forms::TrackBar());
			this->toggleButtons_Gaussian = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->tabPage7 = (gcnew System::Windows::Forms::TabPage());
			this->label_Median_kernel = (gcnew System::Windows::Forms::Label());
			this->trackBar_Median_kernel = (gcnew System::Windows::Forms::TrackBar());
			this->toggleButtons_Median = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->tabPage10 = (gcnew System::Windows::Forms::TabPage());
			this->label_tile_grid_V = (gcnew System::Windows::Forms::Label());
			this->label_tile_grid_H = (gcnew System::Windows::Forms::Label());
			this->label_clip_limit = (gcnew System::Windows::Forms::Label());
			this->trackBar_tile_grid_V = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_tile_grid_H = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_clip_limit = (gcnew System::Windows::Forms::TrackBar());
			this->toggleButtons_CLAHE = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->tabPage11 = (gcnew System::Windows::Forms::TabPage());
			this->textBox1_open_image = (gcnew System::Windows::Forms::TextBox());
			this->button_start_frames_saving = (gcnew System::Windows::Forms::Button());
			this->button_load_directory = (gcnew System::Windows::Forms::Button());
			this->label_BiD_align_Val_bin = (gcnew System::Windows::Forms::Label());
			this->textBox_brief_detail = (gcnew System::Windows::Forms::TextBox());
			this->trackBar_BiD_align_bin = (gcnew System::Windows::Forms::TrackBar());
			this->label_BiD_align_bin = (gcnew System::Windows::Forms::Label());
			this->label15 = (gcnew System::Windows::Forms::Label());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
			this->checkBox_save_data_alazar = (gcnew System::Windows::Forms::CheckBox());
			this->comboBox_prev_scale_down_bin_read = (gcnew System::Windows::Forms::ComboBox());
			this->toggleButtons1 = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->button2_open_image = (gcnew System::Windows::Forms::Button());
			this->label14 = (gcnew System::Windows::Forms::Label());
			this->label13 = (gcnew System::Windows::Forms::Label());
			this->comboBox_prev_scale_down = (gcnew System::Windows::Forms::ComboBox());
			this->checkBox_merge_channels = (gcnew System::Windows::Forms::CheckBox());
			this->button_save_loaded_frame = (gcnew System::Windows::Forms::Button());
			this->checkBox_bin_export_full_frame = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox_bin_export_cropped = (gcnew System::Windows::Forms::CheckBox());
			this->checkBox1_mode = (gcnew System::Windows::Forms::CheckBox());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->button9 = (gcnew System::Windows::Forms::Button());
			this->YScomboBox_intclock = (gcnew System::Windows::Forms::ComboBox());
			this->YScheckBox_extclock = (gcnew System::Windows::Forms::CheckBox());
			this->YScheckBox_intclock = (gcnew System::Windows::Forms::CheckBox());
			this->YSbutton_lissajous_setamp = (gcnew System::Windows::Forms::TabControl());
			this->tabPage16 = (gcnew System::Windows::Forms::TabPage());
			this->YSbutton_raster_setamp = (gcnew System::Windows::Forms::Button());
			this->YStextBox__raster_y_amp = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox__raster_x_amp = (gcnew System::Windows::Forms::TextBox());
			this->YScheckBox_raster_slowspeed = (gcnew System::Windows::Forms::CheckBox());
			this->YSlabel_raster_phase = (gcnew System::Windows::Forms::Label());
			this->YStrackBar_raster_phase = (gcnew System::Windows::Forms::TrackBar());
			this->label46 = (gcnew System::Windows::Forms::Label());
			this->YSbutton_raster_startimg = (gcnew System::Windows::Forms::Button());
			this->YSbutton_raster_stop = (gcnew System::Windows::Forms::Button());
			this->YSbutton_raster_configure = (gcnew System::Windows::Forms::Button());
			this->YScomboBox__raster_yline = (gcnew System::Windows::Forms::ComboBox());
			this->label32 = (gcnew System::Windows::Forms::Label());
			this->YScomboBox__raster_linefreq = (gcnew System::Windows::Forms::ComboBox());
			this->label31 = (gcnew System::Windows::Forms::Label());
			this->YStextBox__raster_yoffset = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox__raster_xoffset = (gcnew System::Windows::Forms::TextBox());
			this->label22 = (gcnew System::Windows::Forms::Label());
			this->label24 = (gcnew System::Windows::Forms::Label());
			this->YStrackBar__raster_y_amp = (gcnew System::Windows::Forms::TrackBar());
			this->label26 = (gcnew System::Windows::Forms::Label());
			this->YStrackBar__raster_x_amp = (gcnew System::Windows::Forms::TrackBar());
			this->label29 = (gcnew System::Windows::Forms::Label());
			this->tabPage15 = (gcnew System::Windows::Forms::TabPage());
			this->textBox_highspeedmode = (gcnew System::Windows::Forms::TextBox());
			this->YScheckBox_lissajous_repairshow = (gcnew System::Windows::Forms::CheckBox());
			this->YStextBox__lissajous_y_amp = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox__lissajous_x_amp = (gcnew System::Windows::Forms::TextBox());
			this->YSbutton__lissajous_setamp = (gcnew System::Windows::Forms::Button());
			this->YSlabel__lissajous_offsettime = (gcnew System::Windows::Forms::Label());
			this->YStrackBar__lissajous_offsettime = (gcnew System::Windows::Forms::TrackBar());
			this->label49 = (gcnew System::Windows::Forms::Label());
			this->YScheckBox_lissajous_samplemapsave = (gcnew System::Windows::Forms::CheckBox());
			this->YSlabel__lissajous_phase = (gcnew System::Windows::Forms::Label());
			this->YStrackBar__lissajous_phase = (gcnew System::Windows::Forms::TrackBar());
			this->label47 = (gcnew System::Windows::Forms::Label());
			this->YStextBox__lissajous_y_freq = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox__lissajous_x_freq = (gcnew System::Windows::Forms::TextBox());
			this->YSbutton_lissajous_startimg = (gcnew System::Windows::Forms::Button());
			this->YSbutton_lissajous_stop = (gcnew System::Windows::Forms::Button());
			this->YSbutton_lissajous_configure = (gcnew System::Windows::Forms::Button());
			this->label33 = (gcnew System::Windows::Forms::Label());
			this->label34 = (gcnew System::Windows::Forms::Label());
			this->YStextBox__lissajous_yoffset = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox__lissajous_xoffset = (gcnew System::Windows::Forms::TextBox());
			this->label36 = (gcnew System::Windows::Forms::Label());
			this->label37 = (gcnew System::Windows::Forms::Label());
			this->YStrackBar__lissajous_y_amp = (gcnew System::Windows::Forms::TrackBar());
			this->label39 = (gcnew System::Windows::Forms::Label());
			this->YStrackBar__lissajous_x_amp = (gcnew System::Windows::Forms::TrackBar());
			this->label40 = (gcnew System::Windows::Forms::Label());
			this->tabPage17 = (gcnew System::Windows::Forms::TabPage());
			this->YSbutton_manual_stopcontrol = (gcnew System::Windows::Forms::Button());
			this->YStextBox_crnt_po_y = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox_crnt_po_x = (gcnew System::Windows::Forms::TextBox());
			this->label41 = (gcnew System::Windows::Forms::Label());
			this->label42 = (gcnew System::Windows::Forms::Label());
			this->label43 = (gcnew System::Windows::Forms::Label());
			this->YStextBox_y_po = (gcnew System::Windows::Forms::TextBox());
			this->YStextBox_x_po = (gcnew System::Windows::Forms::TextBox());
			this->label44 = (gcnew System::Windows::Forms::Label());
			this->label45 = (gcnew System::Windows::Forms::Label());
			this->YSbutton_manual_control = (gcnew System::Windows::Forms::Button());
			this->label23 = (gcnew System::Windows::Forms::Label());
			this->panel2 = (gcnew System::Windows::Forms::Panel());
			this->button_ETL_downdown = (gcnew System::Windows::Forms::Button());
			this->button_ETL_upup = (gcnew System::Windows::Forms::Button());
			this->button_ETL_down = (gcnew System::Windows::Forms::Button());
			this->textBox_ETL_FL = (gcnew System::Windows::Forms::TextBox());
			this->textBox_ETL_voltage = (gcnew System::Windows::Forms::TextBox());
			this->label27 = (gcnew System::Windows::Forms::Label());
			this->button_ETL_up = (gcnew System::Windows::Forms::Button());
			this->label28 = (gcnew System::Windows::Forms::Label());
			this->button_clear_terminal = (gcnew System::Windows::Forms::Button());
			this->label2_combo_y_length = (gcnew System::Windows::Forms::Label());
			this->comboBox2__Ext_clk = (gcnew System::Windows::Forms::ComboBox());
			this->comboBox3_Int_clk = (gcnew System::Windows::Forms::ComboBox());
			this->comboBox_num_Y_lines = (gcnew System::Windows::Forms::ComboBox());
			this->comboBox_headmount_count = (gcnew System::Windows::Forms::ComboBox());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->timer_refresh_UI = (gcnew System::Windows::Forms::Timer(this->components));
			this->tabControl5 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage18 = (gcnew System::Windows::Forms::TabPage());
			this->label57 = (gcnew System::Windows::Forms::Label());
			this->label50 = (gcnew System::Windows::Forms::Label());
			this->YScomboBox_inputrange = (gcnew System::Windows::Forms::ComboBox());
			this->YStextBox__ExtClockFreq = (gcnew System::Windows::Forms::TextBox());
			this->label30 = (gcnew System::Windows::Forms::Label());
			this->YScomboBox_headmount = (gcnew System::Windows::Forms::ComboBox());
			this->tabPage14 = (gcnew System::Windows::Forms::TabPage());
			this->tabControl2 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage3 = (gcnew System::Windows::Forms::TabPage());
			this->button_start_scan = (gcnew System::Windows::Forms::Button());
			this->textBox_yoffset = (gcnew System::Windows::Forms::TextBox());
			this->textBox_xoffset = (gcnew System::Windows::Forms::TextBox());
			this->textBox_line_time = (gcnew System::Windows::Forms::TextBox());
			this->label1_yoffset = (gcnew System::Windows::Forms::Label());
			this->label_xoffset = (gcnew System::Windows::Forms::Label());
			this->label1_sec = (gcnew System::Windows::Forms::Label());
			this->button_acq = (gcnew System::Windows::Forms::Button());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->label_bi_direction_offset = (gcnew System::Windows::Forms::Label());
			this->button1_connect_to_controller = (gcnew System::Windows::Forms::Button());
			this->trackBar_phase = (gcnew System::Windows::Forms::TrackBar());
			this->label7_raster_y_amp = (gcnew System::Windows::Forms::Label());
			this->label_phase = (gcnew System::Windows::Forms::Label());
			this->trackBar3_raster_y_amp = (gcnew System::Windows::Forms::TrackBar());
			this->label6_raster_y_amp = (gcnew System::Windows::Forms::Label());
			this->button_Stop = (gcnew System::Windows::Forms::Button());
			this->trackBar2__raster_x_amp = (gcnew System::Windows::Forms::TrackBar());
			this->label4__raster_x_amp = (gcnew System::Windows::Forms::Label());
			this->label3_raster_x_amp = (gcnew System::Windows::Forms::Label());
			this->tabPage4 = (gcnew System::Windows::Forms::TabPage());
			this->textBox_crnt_po_y = (gcnew System::Windows::Forms::TextBox());
			this->textBox_crnt_po_x = (gcnew System::Windows::Forms::TextBox());
			this->textBox_movingtime = (gcnew System::Windows::Forms::TextBox());
			this->textBox_y_pos = (gcnew System::Windows::Forms::TextBox());
			this->textBox_x_po = (gcnew System::Windows::Forms::TextBox());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->label_Y_current = (gcnew System::Windows::Forms::Label());
			this->label_x_current = (gcnew System::Windows::Forms::Label());
			this->button_back_origin = (gcnew System::Windows::Forms::Button());
			this->label_msec = (gcnew System::Windows::Forms::Label());
			this->label_y_posi = (gcnew System::Windows::Forms::Label());
			this->label_x_posi = (gcnew System::Windows::Forms::Label());
			this->button1_manual_control = (gcnew System::Windows::Forms::Button());
			this->YScomboBox__raster_chopperfreq = (gcnew System::Windows::Forms::ComboBox());
			this->label64 = (gcnew System::Windows::Forms::Label());
			this->label65 = (gcnew System::Windows::Forms::Label());
			this->panel_real_time->SuspendLayout();
			this->panel1->SuspendLayout();
			this->panel_Stage->SuspendLayout();
			this->panelCollapse->SuspendLayout();
			this->tabControl3->SuspendLayout();
			this->tabPage13->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar_savenum))->BeginInit();
			this->tabPage8->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_average_num_count))->BeginInit();
			this->tabPage1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_raw_count))->BeginInit();
			this->tabPage5->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_num_frame))->BeginInit();
			this->tabControl4->SuspendLayout();
			this->tabPage12->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_multiplySP))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_offsetvalueSP))->BeginInit();
			this->tabPage19->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_DCEkernel))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_DCE))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_contrast_alpha))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_brightness_beta))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_adjust_gamma))->BeginInit();
			this->tabPage9->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_normalizeMax))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_normalizeMin))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_MaxVal))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_MinVal))->BeginInit();
			this->tabPage6->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Gaussian_sigmaV))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Gaussian_sigmaH))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Gaussian_kernel))->BeginInit();
			this->tabPage7->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Median_kernel))->BeginInit();
			this->tabPage10->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_tile_grid_V))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_tile_grid_H))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_clip_limit))->BeginInit();
			this->tabPage11->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_BiD_align_bin))->BeginInit();
			this->tabControl1->SuspendLayout();
			this->tabPage2->SuspendLayout();
			this->YSbutton_lissajous_setamp->SuspendLayout();
			this->tabPage16->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar_raster_phase))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__raster_y_amp))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__raster_x_amp))->BeginInit();
			this->tabPage15->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_offsettime))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_phase))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_y_amp))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_x_amp))->BeginInit();
			this->tabPage17->SuspendLayout();
			this->panel2->SuspendLayout();
			this->tabControl5->SuspendLayout();
			this->tabPage18->SuspendLayout();
			this->tabPage14->SuspendLayout();
			this->tabControl2->SuspendLayout();
			this->tabPage3->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_phase))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar3_raster_y_amp))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar2__raster_x_amp))->BeginInit();
			this->tabPage4->SuspendLayout();
			this->SuspendLayout();
			// 
			// checkBox_biDirection
			// 
			this->checkBox_biDirection->AutoSize = true;
			this->checkBox_biDirection->Checked = true;
			this->checkBox_biDirection->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox_biDirection->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_biDirection->Location = System::Drawing::Point(208, 43);
			this->checkBox_biDirection->Name = L"checkBox_biDirection";
			this->checkBox_biDirection->Size = System::Drawing::Size(108, 24);
			this->checkBox_biDirection->TabIndex = 12;
			this->checkBox_biDirection->Text = L"Bi-Direction";
			this->checkBox_biDirection->UseVisualStyleBackColor = true;
			this->checkBox_biDirection->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox_biDirection_CheckedChanged);
			// 
			// checkBox_uniDirection
			// 
			this->checkBox_uniDirection->AutoSize = true;
			this->checkBox_uniDirection->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_uniDirection->Location = System::Drawing::Point(11, 40);
			this->checkBox_uniDirection->Name = L"checkBox_uniDirection";
			this->checkBox_uniDirection->Size = System::Drawing::Size(117, 24);
			this->checkBox_uniDirection->TabIndex = 11;
			this->checkBox_uniDirection->Text = L"Uni-Direction";
			this->checkBox_uniDirection->UseVisualStyleBackColor = true;
			this->checkBox_uniDirection->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox_uniDirection_CheckedChanged);
			// 
			// checkBox2__Int_clk
			// 
			this->checkBox2__Int_clk->AutoSize = true;
			this->checkBox2__Int_clk->Checked = true;
			this->checkBox2__Int_clk->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox2__Int_clk->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox2__Int_clk->Location = System::Drawing::Point(208, 10);
			this->checkBox2__Int_clk->Name = L"checkBox2__Int_clk";
			this->checkBox2__Int_clk->Size = System::Drawing::Size(86, 24);
			this->checkBox2__Int_clk->TabIndex = 8;
			this->checkBox2__Int_clk->Text = L"Int. clock";
			this->checkBox2__Int_clk->UseVisualStyleBackColor = true;
			this->checkBox2__Int_clk->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox2__Int_clk_CheckedChanged);
			// 
			// checkBox1_Ext_clk
			// 
			this->checkBox1_Ext_clk->AutoSize = true;
			this->checkBox1_Ext_clk->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox1_Ext_clk->ForeColor = System::Drawing::Color::White;
			this->checkBox1_Ext_clk->Location = System::Drawing::Point(11, 10);
			this->checkBox1_Ext_clk->Name = L"checkBox1_Ext_clk";
			this->checkBox1_Ext_clk->Size = System::Drawing::Size(89, 24);
			this->checkBox1_Ext_clk->TabIndex = 7;
			this->checkBox1_Ext_clk->Text = L"Ext. clock";
			this->checkBox1_Ext_clk->UseVisualStyleBackColor = true;
			this->checkBox1_Ext_clk->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox1_Ext_clk_CheckedChanged);
			// 
			// timer1_acq
			// 
			this->timer1_acq->Interval = 50;
			// 
			// label1_Main_Control_Panel
			// 
			this->label1_Main_Control_Panel->AutoSize = true;
			this->label1_Main_Control_Panel->BackColor = System::Drawing::Color::Transparent;
			this->label1_Main_Control_Panel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label1_Main_Control_Panel->Location = System::Drawing::Point(4, 109);
			this->label1_Main_Control_Panel->Name = L"label1_Main_Control_Panel";
			this->label1_Main_Control_Panel->Size = System::Drawing::Size(104, 20);
			this->label1_Main_Control_Panel->TabIndex = 3;
			this->label1_Main_Control_Panel->Text = L"Scan Settings";
			// 
			// textBox_pixel_size_row
			// 
			this->textBox_pixel_size_row->BackColor = System::Drawing::Color::Silver;
			this->textBox_pixel_size_row->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_pixel_size_row->ForeColor = System::Drawing::Color::Black;
			this->textBox_pixel_size_row->Location = System::Drawing::Point(139, 46);
			this->textBox_pixel_size_row->Multiline = true;
			this->textBox_pixel_size_row->Name = L"textBox_pixel_size_row";
			this->textBox_pixel_size_row->ReadOnly = true;
			this->textBox_pixel_size_row->Size = System::Drawing::Size(94, 26);
			this->textBox_pixel_size_row->TabIndex = 35;
			this->textBox_pixel_size_row->Text = L"---";
			this->textBox_pixel_size_row->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->BackColor = System::Drawing::Color::Transparent;
			this->label1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label1->Location = System::Drawing::Point(244, 49);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(17, 20);
			this->label1->TabIndex = 34;
			this->label1->Text = L"x";
			// 
			// label_x
			// 
			this->label_x->AutoSize = true;
			this->label_x->BackColor = System::Drawing::Color::Transparent;
			this->label_x->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_x->Location = System::Drawing::Point(244, 12);
			this->label_x->Name = L"label_x";
			this->label_x->Size = System::Drawing::Size(17, 20);
			this->label_x->TabIndex = 33;
			this->label_x->Text = L"x";
			// 
			// textBox_frame_rate
			// 
			this->textBox_frame_rate->BackColor = System::Drawing::Color::Silver;
			this->textBox_frame_rate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>(((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)
				| System::Drawing::FontStyle::Underline)), System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBox_frame_rate->ForeColor = System::Drawing::Color::Black;
			this->textBox_frame_rate->Location = System::Drawing::Point(148, 123);
			this->textBox_frame_rate->Multiline = true;
			this->textBox_frame_rate->Name = L"textBox_frame_rate";
			this->textBox_frame_rate->ReadOnly = true;
			this->textBox_frame_rate->Size = System::Drawing::Size(113, 29);
			this->textBox_frame_rate->TabIndex = 32;
			this->textBox_frame_rate->Text = L"---";
			this->textBox_frame_rate->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_pixel_size_col
			// 
			this->textBox_pixel_size_col->BackColor = System::Drawing::Color::Silver;
			this->textBox_pixel_size_col->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_pixel_size_col->ForeColor = System::Drawing::Color::Black;
			this->textBox_pixel_size_col->Location = System::Drawing::Point(272, 46);
			this->textBox_pixel_size_col->Multiline = true;
			this->textBox_pixel_size_col->Name = L"textBox_pixel_size_col";
			this->textBox_pixel_size_col->ReadOnly = true;
			this->textBox_pixel_size_col->Size = System::Drawing::Size(94, 26);
			this->textBox_pixel_size_col->TabIndex = 30;
			this->textBox_pixel_size_col->Text = L"---";
			this->textBox_pixel_size_col->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_FOV_r
			// 
			this->textBox_FOV_r->BackColor = System::Drawing::Color::Silver;
			this->textBox_FOV_r->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_FOV_r->ForeColor = System::Drawing::Color::Black;
			this->textBox_FOV_r->Location = System::Drawing::Point(272, 9);
			this->textBox_FOV_r->Multiline = true;
			this->textBox_FOV_r->Name = L"textBox_FOV_r";
			this->textBox_FOV_r->ReadOnly = true;
			this->textBox_FOV_r->Size = System::Drawing::Size(94, 26);
			this->textBox_FOV_r->TabIndex = 29;
			this->textBox_FOV_r->Text = L"---";
			this->textBox_FOV_r->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_FOV_l
			// 
			this->textBox_FOV_l->BackColor = System::Drawing::Color::Silver;
			this->textBox_FOV_l->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_FOV_l->ForeColor = System::Drawing::Color::Black;
			this->textBox_FOV_l->Location = System::Drawing::Point(139, 9);
			this->textBox_FOV_l->Multiline = true;
			this->textBox_FOV_l->Name = L"textBox_FOV_l";
			this->textBox_FOV_l->ReadOnly = true;
			this->textBox_FOV_l->Size = System::Drawing::Size(94, 26);
			this->textBox_FOV_l->TabIndex = 28;
			this->textBox_FOV_l->Text = L"---";
			this->textBox_FOV_l->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label_frame_rate
			// 
			this->label_frame_rate->AutoSize = true;
			this->label_frame_rate->BackColor = System::Drawing::Color::Transparent;
			this->label_frame_rate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label_frame_rate->ForeColor = System::Drawing::Color::White;
			this->label_frame_rate->Location = System::Drawing::Point(5, 125);
			this->label_frame_rate->Name = L"label_frame_rate";
			this->label_frame_rate->Size = System::Drawing::Size(132, 21);
			this->label_frame_rate->TabIndex = 27;
			this->label_frame_rate->Text = L"Frame Rate (1/s)";
			// 
			// label_pixel_size
			// 
			this->label_pixel_size->AutoSize = true;
			this->label_pixel_size->BackColor = System::Drawing::Color::Transparent;
			this->label_pixel_size->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_pixel_size->Location = System::Drawing::Point(5, 48);
			this->label_pixel_size->Name = L"label_pixel_size";
			this->label_pixel_size->Size = System::Drawing::Size(104, 20);
			this->label_pixel_size->TabIndex = 25;
			this->label_pixel_size->Text = L"Pixel Number";
			// 
			// label_FOV
			// 
			this->label_FOV->AutoSize = true;
			this->label_FOV->BackColor = System::Drawing::Color::Transparent;
			this->label_FOV->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_FOV->Location = System::Drawing::Point(5, 10);
			this->label_FOV->Name = L"label_FOV";
			this->label_FOV->Size = System::Drawing::Size(77, 20);
			this->label_FOV->TabIndex = 24;
			this->label_FOV->Text = L"FOV (μm)";
			// 
			// timer_Control_GUI
			// 
			this->timer_Control_GUI->Enabled = true;
			// 
			// Configure
			// 
			this->Configure->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->Configure->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->Configure->Location = System::Drawing::Point(298, 104);
			this->Configure->Name = L"Configure";
			this->Configure->Size = System::Drawing::Size(101, 31);
			this->Configure->TabIndex = 11;
			this->Configure->Text = L"Configure";
			this->Configure->UseVisualStyleBackColor = true;
			this->Configure->Click += gcnew System::EventHandler(this, &MTPFM::button_Configure_Click);
			// 
			// status
			// 
			this->status->BackColor = System::Drawing::SystemColors::WindowFrame;
			this->status->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->status->ForeColor = System::Drawing::SystemColors::Window;
			this->status->Location = System::Drawing::Point(6, 10);
			this->status->Name = L"status";
			this->status->Size = System::Drawing::Size(305, 26);
			this->status->TabIndex = 12;
			this->status->Text = L"---";
			this->status->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_shut_down_MEMS
			// 
			this->button_shut_down_MEMS->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_shut_down_MEMS->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_shut_down_MEMS->Location = System::Drawing::Point(317, 10);
			this->button_shut_down_MEMS->Name = L"button_shut_down_MEMS";
			this->button_shut_down_MEMS->Size = System::Drawing::Size(83, 26);
			this->button_shut_down_MEMS->TabIndex = 13;
			this->button_shut_down_MEMS->Text = L"Close";
			this->button_shut_down_MEMS->UseVisualStyleBackColor = true;
			this->button_shut_down_MEMS->Click += gcnew System::EventHandler(this, &MTPFM::button_shut_down_MEMS_Click);
			// 
			// panel_real_time
			// 
			this->panel_real_time->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel_real_time->Controls->Add(this->textBox_chopper_freq);
			this->panel_real_time->Controls->Add(this->label63);
			this->panel_real_time->Controls->Add(this->label21);
			this->panel_real_time->Controls->Add(this->textBox_acq_time);
			this->panel_real_time->Controls->Add(this->label2);
			this->panel_real_time->Controls->Add(this->textBox_pixel_size_row);
			this->panel_real_time->Controls->Add(this->textBox_frame_rate);
			this->panel_real_time->Controls->Add(this->label1);
			this->panel_real_time->Controls->Add(this->label_x);
			this->panel_real_time->Controls->Add(this->textBox_pixel_size_col);
			this->panel_real_time->Controls->Add(this->label_FOV);
			this->panel_real_time->Controls->Add(this->textBox_FOV_r);
			this->panel_real_time->Controls->Add(this->textBox_FOV_l);
			this->panel_real_time->Controls->Add(this->label_pixel_size);
			this->panel_real_time->Controls->Add(this->label_frame_rate);
			this->panel_real_time->Controls->Add(this->panel1);
			this->panel_real_time->Location = System::Drawing::Point(7, 390);
			this->panel_real_time->Name = L"panel_real_time";
			this->panel_real_time->Size = System::Drawing::Size(428, 199);
			this->panel_real_time->TabIndex = 16;
			// 
			// textBox_chopper_freq
			// 
			this->textBox_chopper_freq->BackColor = System::Drawing::Color::Silver;
			this->textBox_chopper_freq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>(((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)
				| System::Drawing::FontStyle::Underline)), System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBox_chopper_freq->ForeColor = System::Drawing::Color::Black;
			this->textBox_chopper_freq->Location = System::Drawing::Point(148, 163);
			this->textBox_chopper_freq->Multiline = true;
			this->textBox_chopper_freq->Name = L"textBox_chopper_freq";
			this->textBox_chopper_freq->ReadOnly = true;
			this->textBox_chopper_freq->Size = System::Drawing::Size(113, 29);
			this->textBox_chopper_freq->TabIndex = 41;
			this->textBox_chopper_freq->Text = L"---";
			this->textBox_chopper_freq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label63
			// 
			this->label63->AutoSize = true;
			this->label63->BackColor = System::Drawing::Color::Transparent;
			this->label63->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label63->ForeColor = System::Drawing::Color::White;
			this->label63->Location = System::Drawing::Point(5, 165);
			this->label63->Name = L"label63";
			this->label63->Size = System::Drawing::Size(144, 21);
			this->label63->TabIndex = 40;
			this->label63->Text = L"Chopper freq. (Hz)";
			// 
			// label21
			// 
			this->label21->AutoSize = true;
			this->label21->BackColor = System::Drawing::Color::RosyBrown;
			this->label21->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label21->ForeColor = System::Drawing::Color::White;
			this->label21->Location = System::Drawing::Point(269, 85);
			this->label21->Name = L"label21";
			this->label21->Size = System::Drawing::Size(141, 21);
			this->label21->TabIndex = 38;
			this->label21->Text = L"Refresh Rate (1/s)";
			// 
			// textBox_acq_time
			// 
			this->textBox_acq_time->BackColor = System::Drawing::Color::Silver;
			this->textBox_acq_time->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>(((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)
				| System::Drawing::FontStyle::Underline)), System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBox_acq_time->ForeColor = System::Drawing::Color::Black;
			this->textBox_acq_time->Location = System::Drawing::Point(148, 83);
			this->textBox_acq_time->Multiline = true;
			this->textBox_acq_time->Name = L"textBox_acq_time";
			this->textBox_acq_time->ReadOnly = true;
			this->textBox_acq_time->Size = System::Drawing::Size(113, 29);
			this->textBox_acq_time->TabIndex = 37;
			this->textBox_acq_time->Text = L"---";
			this->textBox_acq_time->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->BackColor = System::Drawing::Color::Transparent;
			this->label2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label2->ForeColor = System::Drawing::Color::White;
			this->label2->Location = System::Drawing::Point(5, 86);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(105, 21);
			this->label2->TabIndex = 36;
			this->label2->Text = L"Acq. Time (s)";
			// 
			// panel1
			// 
			this->panel1->BackColor = System::Drawing::Color::RosyBrown;
			this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->panel1->Controls->Add(this->textBox_refresh_rate);
			this->panel1->ForeColor = System::Drawing::Color::White;
			this->panel1->Location = System::Drawing::Point(266, 83);
			this->panel1->Name = L"panel1";
			this->panel1->Size = System::Drawing::Size(150, 69);
			this->panel1->TabIndex = 39;
			// 
			// textBox_refresh_rate
			// 
			this->textBox_refresh_rate->BackColor = System::Drawing::Color::Silver;
			this->textBox_refresh_rate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_refresh_rate->ForeColor = System::Drawing::Color::Black;
			this->textBox_refresh_rate->Location = System::Drawing::Point(9, 33);
			this->textBox_refresh_rate->Multiline = true;
			this->textBox_refresh_rate->Name = L"textBox_refresh_rate";
			this->textBox_refresh_rate->ReadOnly = true;
			this->textBox_refresh_rate->Size = System::Drawing::Size(131, 26);
			this->textBox_refresh_rate->TabIndex = 40;
			this->textBox_refresh_rate->Text = L"---";
			this->textBox_refresh_rate->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label_Real_time
			// 
			this->label_Real_time->AutoSize = true;
			this->label_Real_time->BackColor = System::Drawing::Color::Transparent;
			this->label_Real_time->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label_Real_time->Location = System::Drawing::Point(438, 375);
			this->label_Real_time->Name = L"label_Real_time";
			this->label_Real_time->Size = System::Drawing::Size(174, 20);
			this->label_Real_time->TabIndex = 17;
			this->label_Real_time->Text = L"Real-Time Information";
			// 
			// panel_Stage
			// 
			this->panel_Stage->BackColor = System::Drawing::Color::DimGray;
			this->panel_Stage->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel_Stage->Controls->Add(this->label54);
			this->panel_Stage->Controls->Add(this->textBox_initial_position);
			this->panel_Stage->Controls->Add(this->toggleButtons_initial_position);
			this->panel_Stage->Controls->Add(this->radioButton2_abs);
			this->panel_Stage->Controls->Add(this->radioButton1_rel);
			this->panel_Stage->Controls->Add(this->button2_abs_pos);
			this->panel_Stage->Controls->Add(this->label9);
			this->panel_Stage->Controls->Add(this->label6);
			this->panel_Stage->Controls->Add(this->button1);
			this->panel_Stage->Controls->Add(this->textBox_abs_position);
			this->panel_Stage->Controls->Add(this->textBox3_relative_pos);
			this->panel_Stage->Controls->Add(this->textBox_current_position);
			this->panel_Stage->Controls->Add(this->label3);
			this->panel_Stage->Controls->Add(this->button1_DOWN);
			this->panel_Stage->Controls->Add(this->label_current_position);
			this->panel_Stage->Controls->Add(this->button1_UP);
			this->panel_Stage->Controls->Add(this->label7);
			this->panel_Stage->Controls->Add(this->button_HOME);
			this->panel_Stage->Location = System::Drawing::Point(7, 529);
			this->panel_Stage->Name = L"panel_Stage";
			this->panel_Stage->Size = System::Drawing::Size(418, 182);
			this->panel_Stage->TabIndex = 21;
			// 
			// label54
			// 
			this->label54->AutoSize = true;
			this->label54->BackColor = System::Drawing::Color::Transparent;
			this->label54->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label54->Location = System::Drawing::Point(67, 144);
			this->label54->Name = L"label54";
			this->label54->Size = System::Drawing::Size(121, 20);
			this->label54->TabIndex = 54;
			this->label54->Text = L"Initial pos. (μm)";
			// 
			// textBox_initial_position
			// 
			this->textBox_initial_position->BackColor = System::Drawing::Color::Silver;
			this->textBox_initial_position->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_initial_position->ForeColor = System::Drawing::Color::Black;
			this->textBox_initial_position->Location = System::Drawing::Point(189, 143);
			this->textBox_initial_position->Multiline = true;
			this->textBox_initial_position->Name = L"textBox_initial_position";
			this->textBox_initial_position->ReadOnly = true;
			this->textBox_initial_position->Size = System::Drawing::Size(100, 26);
			this->textBox_initial_position->TabIndex = 53;
			this->textBox_initial_position->Text = L"---";
			this->textBox_initial_position->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// toggleButtons_initial_position
			// 
			this->toggleButtons_initial_position->Location = System::Drawing::Point(13, 143);
			this->toggleButtons_initial_position->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_initial_position->Name = L"toggleButtons_initial_position";
			this->toggleButtons_initial_position->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_initial_position->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_initial_position->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_initial_position->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_initial_position->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_initial_position->SolidStyle = false;
			this->toggleButtons_initial_position->TabIndex = 52;
			this->toggleButtons_initial_position->UseVisualStyleBackColor = true;
			this->toggleButtons_initial_position->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_initial_position_CheckedChanged);
			// 
			// radioButton2_abs
			// 
			this->radioButton2_abs->AutoSize = true;
			this->radioButton2_abs->Location = System::Drawing::Point(10, 109);
			this->radioButton2_abs->Name = L"radioButton2_abs";
			this->radioButton2_abs->Size = System::Drawing::Size(14, 13);
			this->radioButton2_abs->TabIndex = 51;
			this->radioButton2_abs->TabStop = true;
			this->radioButton2_abs->UseVisualStyleBackColor = true;
			this->radioButton2_abs->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::radioButton2_abs_CheckedChanged);
			// 
			// radioButton1_rel
			// 
			this->radioButton1_rel->AutoSize = true;
			this->radioButton1_rel->Location = System::Drawing::Point(10, 65);
			this->radioButton1_rel->Name = L"radioButton1_rel";
			this->radioButton1_rel->Size = System::Drawing::Size(14, 13);
			this->radioButton1_rel->TabIndex = 50;
			this->radioButton1_rel->TabStop = true;
			this->radioButton1_rel->UseVisualStyleBackColor = true;
			this->radioButton1_rel->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::radioButton1_rel_CheckedChanged);
			// 
			// button2_abs_pos
			// 
			this->button2_abs_pos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button2_abs_pos->ForeColor = System::Drawing::Color::RoyalBlue;
			this->button2_abs_pos->Location = System::Drawing::Point(280, 102);
			this->button2_abs_pos->Name = L"button2_abs_pos";
			this->button2_abs_pos->Size = System::Drawing::Size(53, 27);
			this->button2_abs_pos->TabIndex = 49;
			this->button2_abs_pos->Text = L"abs.";
			this->button2_abs_pos->UseVisualStyleBackColor = true;
			this->button2_abs_pos->Click += gcnew System::EventHandler(this, &MTPFM::button2_abs_pos_Click);
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->BackColor = System::Drawing::Color::Transparent;
			this->label9->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label9->Location = System::Drawing::Point(351, 71);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(58, 20);
			this->label9->TabIndex = 48;
			this->label9->Text = L"DOWN";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->BackColor = System::Drawing::Color::Transparent;
			this->label6->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label6->Location = System::Drawing::Point(366, 46);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(29, 20);
			this->label6->TabIndex = 47;
			this->label6->Text = L"UP";
			// 
			// button1
			// 
			this->button1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button1->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button1->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button1.Image")));
			this->button1->Location = System::Drawing::Point(306, 9);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(39, 35);
			this->button1->TabIndex = 46;
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MTPFM::button1_Click);
			// 
			// textBox_abs_position
			// 
			this->textBox_abs_position->BackColor = System::Drawing::Color::Silver;
			this->textBox_abs_position->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_abs_position->Location = System::Drawing::Point(174, 102);
			this->textBox_abs_position->Name = L"textBox_abs_position";
			this->textBox_abs_position->Size = System::Drawing::Size(100, 27);
			this->textBox_abs_position->TabIndex = 45;
			this->textBox_abs_position->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox3_relative_pos
			// 
			this->textBox3_relative_pos->BackColor = System::Drawing::Color::Silver;
			this->textBox3_relative_pos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox3_relative_pos->Location = System::Drawing::Point(175, 58);
			this->textBox3_relative_pos->Name = L"textBox3_relative_pos";
			this->textBox3_relative_pos->Size = System::Drawing::Size(100, 27);
			this->textBox3_relative_pos->TabIndex = 44;
			this->textBox3_relative_pos->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_current_position
			// 
			this->textBox_current_position->BackColor = System::Drawing::Color::Silver;
			this->textBox_current_position->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_current_position->ForeColor = System::Drawing::Color::Black;
			this->textBox_current_position->Location = System::Drawing::Point(146, 15);
			this->textBox_current_position->Multiline = true;
			this->textBox_current_position->Name = L"textBox_current_position";
			this->textBox_current_position->ReadOnly = true;
			this->textBox_current_position->Size = System::Drawing::Size(100, 26);
			this->textBox_current_position->TabIndex = 36;
			this->textBox_current_position->Text = L"---";
			this->textBox_current_position->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->BackColor = System::Drawing::Color::Transparent;
			this->label3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label3->Location = System::Drawing::Point(32, 105);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(112, 20);
			this->label3->TabIndex = 37;
			this->label3->Text = L"Abs, pos. (μm)";
			// 
			// button1_DOWN
			// 
			this->button1_DOWN->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button1_DOWN->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button1_DOWN->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button1_DOWN.Image")));
			this->button1_DOWN->Location = System::Drawing::Point(365, 94);
			this->button1_DOWN->Name = L"button1_DOWN";
			this->button1_DOWN->Size = System::Drawing::Size(30, 37);
			this->button1_DOWN->TabIndex = 42;
			this->button1_DOWN->UseVisualStyleBackColor = true;
			this->button1_DOWN->Click += gcnew System::EventHandler(this, &MTPFM::button1_DOWN_Click);
			// 
			// label_current_position
			// 
			this->label_current_position->AutoSize = true;
			this->label_current_position->BackColor = System::Drawing::Color::Transparent;
			this->label_current_position->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_current_position->Location = System::Drawing::Point(6, 17);
			this->label_current_position->Name = L"label_current_position";
			this->label_current_position->Size = System::Drawing::Size(134, 20);
			this->label_current_position->TabIndex = 38;
			this->label_current_position->Text = L"Current pos. (μm)";
			// 
			// button1_UP
			// 
			this->button1_UP->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button1_UP->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button1_UP->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button1_UP.Image")));
			this->button1_UP->Location = System::Drawing::Point(365, 6);
			this->button1_UP->Name = L"button1_UP";
			this->button1_UP->Size = System::Drawing::Size(30, 37);
			this->button1_UP->TabIndex = 41;
			this->button1_UP->UseVisualStyleBackColor = true;
			this->button1_UP->Click += gcnew System::EventHandler(this, &MTPFM::button1_UP_Click);
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->BackColor = System::Drawing::Color::Transparent;
			this->label7->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label7->Location = System::Drawing::Point(31, 61);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(137, 20);
			this->label7->TabIndex = 36;
			this->label7->Text = L"Relative pos. (μm)";
			// 
			// button_HOME
			// 
			this->button_HOME->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_HOME->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_HOME->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_HOME.Image")));
			this->button_HOME->Location = System::Drawing::Point(259, 9);
			this->button_HOME->Name = L"button_HOME";
			this->button_HOME->Size = System::Drawing::Size(37, 36);
			this->button_HOME->TabIndex = 40;
			this->button_HOME->UseVisualStyleBackColor = true;
			this->button_HOME->Click += gcnew System::EventHandler(this, &MTPFM::button_HOME_Click);
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->BackColor = System::Drawing::Color::Transparent;
			this->label5->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label5->Location = System::Drawing::Point(7, 504);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(107, 20);
			this->label5->TabIndex = 22;
			this->label5->Text = L"Stage Control";
			// 
			// button2_co_ex
			// 
			this->button2_co_ex->BackColor = System::Drawing::Color::DimGray;
			this->button2_co_ex->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button2_co_ex->ForeColor = System::Drawing::Color::White;
			this->button2_co_ex->Location = System::Drawing::Point(402, 6);
			this->button2_co_ex->Name = L"button2_co_ex";
			this->button2_co_ex->Size = System::Drawing::Size(25, 54);
			this->button2_co_ex->TabIndex = 23;
			this->button2_co_ex->Text = L">";
			this->button2_co_ex->UseVisualStyleBackColor = false;
			this->button2_co_ex->Click += gcnew System::EventHandler(this, &MTPFM::button2_co_ex_Click);
			// 
			// panelCollapse
			// 
			this->panelCollapse->Controls->Add(this->label60);
			this->panelCollapse->Controls->Add(this->toggleButtons_defaltimageprocessing);
			this->panelCollapse->Controls->Add(this->checkBox_Export16BIT);
			this->panelCollapse->Controls->Add(this->label12);
			this->panelCollapse->Controls->Add(this->tabControl3);
			this->panelCollapse->Controls->Add(this->tabControl4);
			this->panelCollapse->Controls->Add(this->label15);
			this->panelCollapse->Controls->Add(this->tabControl1);
			this->panelCollapse->Controls->Add(this->label4);
			this->panelCollapse->Controls->Add(this->panel_real_time);
			this->panelCollapse->Location = System::Drawing::Point(431, 8);
			this->panelCollapse->Name = L"panelCollapse";
			this->panelCollapse->Size = System::Drawing::Size(445, 825);
			this->panelCollapse->TabIndex = 24;
			// 
			// label60
			// 
			this->label60->AutoSize = true;
			this->label60->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label60->ForeColor = System::Drawing::Color::White;
			this->label60->Location = System::Drawing::Point(242, 594);
			this->label60->Name = L"label60";
			this->label60->Size = System::Drawing::Size(178, 20);
			this->label60->TabIndex = 77;
			this->label60->Text = L"Defalt image processing";
			// 
			// toggleButtons_defaltimageprocessing
			// 
			this->toggleButtons_defaltimageprocessing->Location = System::Drawing::Point(191, 593);
			this->toggleButtons_defaltimageprocessing->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_defaltimageprocessing->Name = L"toggleButtons_defaltimageprocessing";
			this->toggleButtons_defaltimageprocessing->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_defaltimageprocessing->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_defaltimageprocessing->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_defaltimageprocessing->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_defaltimageprocessing->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_defaltimageprocessing->SolidStyle = false;
			this->toggleButtons_defaltimageprocessing->TabIndex = 75;
			this->toggleButtons_defaltimageprocessing->UseVisualStyleBackColor = true;
			this->toggleButtons_defaltimageprocessing->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_defaltimageprocessing_CheckedChanged);
			// 
			// checkBox_Export16BIT
			// 
			this->checkBox_Export16BIT->AutoSize = true;
			this->checkBox_Export16BIT->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_Export16BIT->Location = System::Drawing::Point(207, 4);
			this->checkBox_Export16BIT->Name = L"checkBox_Export16BIT";
			this->checkBox_Export16BIT->Size = System::Drawing::Size(224, 24);
			this->checkBox_Export16BIT->TabIndex = 48;
			this->checkBox_Export16BIT->Text = L"Export 16BIT | Optimization";
			this->checkBox_Export16BIT->UseVisualStyleBackColor = true;
			this->checkBox_Export16BIT->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox_Export16BIT_CheckedChanged);
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->BackColor = System::Drawing::Color::Transparent;
			this->label12->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label12->Location = System::Drawing::Point(8, 5);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(188, 20);
			this->label12->TabIndex = 42;
			this->label12->Text = L"CHANNEL SELECT PANEL";
			// 
			// tabControl3
			// 
			this->tabControl3->Controls->Add(this->tabPage13);
			this->tabControl3->Controls->Add(this->tabPage8);
			this->tabControl3->Controls->Add(this->tabPage1);
			this->tabControl3->Controls->Add(this->tabPage5);
			this->tabControl3->Location = System::Drawing::Point(8, 30);
			this->tabControl3->Name = L"tabControl3";
			this->tabControl3->SelectedIndex = 0;
			this->tabControl3->Size = System::Drawing::Size(428, 141);
			this->tabControl3->TabIndex = 68;
			// 
			// tabPage13
			// 
			this->tabPage13->BackColor = System::Drawing::Color::DimGray;
			this->tabPage13->Controls->Add(this->checkBox_saveprocessedimage);
			this->tabPage13->Controls->Add(this->YScheckBox_saverawbin);
			this->tabPage13->Controls->Add(this->YSbutton_clear);
			this->tabPage13->Controls->Add(this->label25);
			this->tabPage13->Controls->Add(this->YStextBox_title);
			this->tabPage13->Controls->Add(this->YScheckBox_saveraw);
			this->tabPage13->Controls->Add(this->YSbutton_save);
			this->tabPage13->Controls->Add(this->progressBar1);
			this->tabPage13->Controls->Add(this->YSlabel_savenum);
			this->tabPage13->Controls->Add(this->YScheckBox_ch1);
			this->tabPage13->Controls->Add(this->YStrackBar_savenum);
			this->tabPage13->Controls->Add(this->textBox2);
			this->tabPage13->Controls->Add(this->label38);
			this->tabPage13->Location = System::Drawing::Point(4, 24);
			this->tabPage13->Name = L"tabPage13";
			this->tabPage13->Padding = System::Windows::Forms::Padding(3);
			this->tabPage13->Size = System::Drawing::Size(420, 113);
			this->tabPage13->TabIndex = 3;
			this->tabPage13->Text = L"YS Image acq.";
			// 
			// checkBox_saveprocessedimage
			// 
			this->checkBox_saveprocessedimage->AutoSize = true;
			this->checkBox_saveprocessedimage->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_saveprocessedimage->Location = System::Drawing::Point(157, 62);
			this->checkBox_saveprocessedimage->Name = L"checkBox_saveprocessedimage";
			this->checkBox_saveprocessedimage->Size = System::Drawing::Size(181, 24);
			this->checkBox_saveprocessedimage->TabIndex = 111;
			this->checkBox_saveprocessedimage->Text = L"Save processed image";
			this->checkBox_saveprocessedimage->UseVisualStyleBackColor = true;
			// 
			// YScheckBox_saverawbin
			// 
			this->YScheckBox_saverawbin->AutoSize = true;
			this->YScheckBox_saverawbin->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScheckBox_saverawbin->Location = System::Drawing::Point(6, 60);
			this->YScheckBox_saverawbin->Name = L"YScheckBox_saverawbin";
			this->YScheckBox_saverawbin->Size = System::Drawing::Size(121, 24);
			this->YScheckBox_saverawbin->TabIndex = 110;
			this->YScheckBox_saverawbin->Text = L"Save Raw Bin";
			this->YScheckBox_saverawbin->UseVisualStyleBackColor = true;
			// 
			// YSbutton_clear
			// 
			this->YSbutton_clear->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSbutton_clear->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton_clear->Location = System::Drawing::Point(342, 36);
			this->YSbutton_clear->Name = L"YSbutton_clear";
			this->YSbutton_clear->Size = System::Drawing::Size(73, 30);
			this->YSbutton_clear->TabIndex = 109;
			this->YSbutton_clear->Text = L"Clear";
			this->YSbutton_clear->UseVisualStyleBackColor = true;
			this->YSbutton_clear->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_clear_Click);
			// 
			// label25
			// 
			this->label25->AutoSize = true;
			this->label25->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label25->Location = System::Drawing::Point(153, 36);
			this->label25->Name = L"label25";
			this->label25->Size = System::Drawing::Size(52, 21);
			this->label25->TabIndex = 108;
			this->label25->Text = L"Title :";
			// 
			// YStextBox_title
			// 
			this->YStextBox_title->Location = System::Drawing::Point(204, 36);
			this->YStextBox_title->Name = L"YStextBox_title";
			this->YStextBox_title->Size = System::Drawing::Size(121, 23);
			this->YStextBox_title->TabIndex = 107;
			// 
			// YScheckBox_saveraw
			// 
			this->YScheckBox_saveraw->AutoSize = true;
			this->YScheckBox_saveraw->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScheckBox_saveraw->Location = System::Drawing::Point(7, 36);
			this->YScheckBox_saveraw->Name = L"YScheckBox_saveraw";
			this->YScheckBox_saveraw->Size = System::Drawing::Size(131, 24);
			this->YScheckBox_saveraw->TabIndex = 106;
			this->YScheckBox_saveraw->Text = L"Save Raw Data";
			this->YScheckBox_saveraw->UseVisualStyleBackColor = true;
			// 
			// YSbutton_save
			// 
			this->YSbutton_save->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSbutton_save->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->YSbutton_save->Location = System::Drawing::Point(342, 3);
			this->YSbutton_save->Name = L"YSbutton_save";
			this->YSbutton_save->Size = System::Drawing::Size(73, 30);
			this->YSbutton_save->TabIndex = 102;
			this->YSbutton_save->Text = L"Save";
			this->YSbutton_save->UseVisualStyleBackColor = true;
			this->YSbutton_save->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_save_Click);
			// 
			// progressBar1
			// 
			this->progressBar1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(64)), static_cast<System::Int32>(static_cast<System::Byte>(64)),
				static_cast<System::Int32>(static_cast<System::Byte>(64)));
			this->progressBar1->ForeColor = System::Drawing::Color::LimeGreen;
			this->progressBar1->Location = System::Drawing::Point(63, 9);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(205, 21);
			this->progressBar1->Step = 1;
			this->progressBar1->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBar1->TabIndex = 99;
			// 
			// YSlabel_savenum
			// 
			this->YSlabel_savenum->AutoSize = true;
			this->YSlabel_savenum->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSlabel_savenum->ForeColor = System::Drawing::Color::White;
			this->YSlabel_savenum->Location = System::Drawing::Point(318, 84);
			this->YSlabel_savenum->Name = L"YSlabel_savenum";
			this->YSlabel_savenum->Size = System::Drawing::Size(72, 20);
			this->YSlabel_savenum->TabIndex = 103;
			this->YSlabel_savenum->Text = L"Num = 1";
			// 
			// YScheckBox_ch1
			// 
			this->YScheckBox_ch1->AutoSize = true;
			this->YScheckBox_ch1->Checked = true;
			this->YScheckBox_ch1->CheckState = System::Windows::Forms::CheckState::Checked;
			this->YScheckBox_ch1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScheckBox_ch1->Location = System::Drawing::Point(7, 7);
			this->YScheckBox_ch1->Name = L"YScheckBox_ch1";
			this->YScheckBox_ch1->Size = System::Drawing::Size(55, 24);
			this->YScheckBox_ch1->TabIndex = 101;
			this->YScheckBox_ch1->Text = L"Ch1";
			this->YScheckBox_ch1->UseVisualStyleBackColor = true;
			// 
			// YStrackBar_savenum
			// 
			this->YStrackBar_savenum->AutoSize = false;
			this->YStrackBar_savenum->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar_savenum->LargeChange = 10;
			this->YStrackBar_savenum->Location = System::Drawing::Point(95, 87);
			this->YStrackBar_savenum->Maximum = 2500;
			this->YStrackBar_savenum->Minimum = 1;
			this->YStrackBar_savenum->Name = L"YStrackBar_savenum";
			this->YStrackBar_savenum->Size = System::Drawing::Size(225, 20);
			this->YStrackBar_savenum->TabIndex = 104;
			this->YStrackBar_savenum->TickFrequency = 100;
			this->YStrackBar_savenum->Value = 1;
			this->YStrackBar_savenum->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar_savenum_Scroll);
			// 
			// textBox2
			// 
			this->textBox2->BackColor = System::Drawing::Color::DimGray;
			this->textBox2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->textBox2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox2->ForeColor = System::Drawing::Color::White;
			this->textBox2->Location = System::Drawing::Point(274, 4);
			this->textBox2->Name = L"textBox2";
			this->textBox2->ReadOnly = true;
			this->textBox2->Size = System::Drawing::Size(63, 27);
			this->textBox2->TabIndex = 100;
			this->textBox2->Text = L"---";
			this->textBox2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label38
			// 
			this->label38->AutoSize = true;
			this->label38->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label38->ForeColor = System::Drawing::Color::White;
			this->label38->Location = System::Drawing::Point(8, 86);
			this->label38->Name = L"label38";
			this->label38->Size = System::Drawing::Size(85, 20);
			this->label38->TabIndex = 105;
			this->label38->Text = L"# of Frame";
			// 
			// tabPage8
			// 
			this->tabPage8->BackColor = System::Drawing::Color::DimGray;
			this->tabPage8->Controls->Add(this->label48);
			this->tabPage8->Controls->Add(this->label35);
			this->tabPage8->Controls->Add(this->label2_average_count);
			this->tabPage8->Controls->Add(this->trackBar_average_num_count);
			this->tabPage8->Controls->Add(this->toggleButtons_allow_averaging);
			this->tabPage8->Location = System::Drawing::Point(4, 24);
			this->tabPage8->Name = L"tabPage8";
			this->tabPage8->Padding = System::Windows::Forms::Padding(3);
			this->tabPage8->Size = System::Drawing::Size(420, 113);
			this->tabPage8->TabIndex = 2;
			this->tabPage8->Text = L"Frame Averaging";
			// 
			// label48
			// 
			this->label48->AutoSize = true;
			this->label48->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label48->Location = System::Drawing::Point(58, 6);
			this->label48->Name = L"label48";
			this->label48->Size = System::Drawing::Size(132, 21);
			this->label48->TabIndex = 112;
			this->label48->Text = L"Average display";
			// 
			// label35
			// 
			this->label35->AutoSize = true;
			this->label35->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label35->Location = System::Drawing::Point(255, 48);
			this->label35->Name = L"label35";
			this->label35->Size = System::Drawing::Size(143, 21);
			this->label35->TabIndex = 111;
			this->label35->Text = L"images per frame";
			// 
			// label2_average_count
			// 
			this->label2_average_count->AutoSize = true;
			this->label2_average_count->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label2_average_count->Location = System::Drawing::Point(226, 48);
			this->label2_average_count->Name = L"label2_average_count";
			this->label2_average_count->Size = System::Drawing::Size(19, 21);
			this->label2_average_count->TabIndex = 66;
			this->label2_average_count->Text = L"1";
			// 
			// trackBar_average_num_count
			// 
			this->trackBar_average_num_count->AutoSize = false;
			this->trackBar_average_num_count->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_average_num_count->LargeChange = 1;
			this->trackBar_average_num_count->Location = System::Drawing::Point(28, 48);
			this->trackBar_average_num_count->Minimum = 1;
			this->trackBar_average_num_count->Name = L"trackBar_average_num_count";
			this->trackBar_average_num_count->Size = System::Drawing::Size(194, 30);
			this->trackBar_average_num_count->TabIndex = 59;
			this->trackBar_average_num_count->Value = 1;
			this->trackBar_average_num_count->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_average_num_count_Scroll);
			// 
			// toggleButtons_allow_averaging
			// 
			this->toggleButtons_allow_averaging->Location = System::Drawing::Point(8, 6);
			this->toggleButtons_allow_averaging->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_allow_averaging->Name = L"toggleButtons_allow_averaging";
			this->toggleButtons_allow_averaging->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_allow_averaging->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_allow_averaging->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_allow_averaging->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_allow_averaging->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_allow_averaging->SolidStyle = false;
			this->toggleButtons_allow_averaging->TabIndex = 69;
			this->toggleButtons_allow_averaging->UseVisualStyleBackColor = true;
			this->toggleButtons_allow_averaging->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_allow_averaging_CheckedChanged);
			// 
			// tabPage1
			// 
			this->tabPage1->BackColor = System::Drawing::Color::DimGray;
			this->tabPage1->Controls->Add(this->progressBar_raw_ch1);
			this->tabPage1->Controls->Add(this->checkBox1_raw_ch1);
			this->tabPage1->Controls->Add(this->textBox_progress_raw_ch1);
			this->tabPage1->Controls->Add(this->button_raw_save);
			this->tabPage1->Controls->Add(this->label17);
			this->tabPage1->Controls->Add(this->labelraw_count);
			this->tabPage1->Controls->Add(this->trackBar_raw_count);
			this->tabPage1->ForeColor = System::Drawing::Color::White;
			this->tabPage1->Location = System::Drawing::Point(4, 24);
			this->tabPage1->Name = L"tabPage1";
			this->tabPage1->Padding = System::Windows::Forms::Padding(3);
			this->tabPage1->Size = System::Drawing::Size(420, 113);
			this->tabPage1->TabIndex = 0;
			this->tabPage1->Text = L"RAW Image acq.";
			// 
			// progressBar_raw_ch1
			// 
			this->progressBar_raw_ch1->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(64)),
				static_cast<System::Int32>(static_cast<System::Byte>(64)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
			this->progressBar_raw_ch1->ForeColor = System::Drawing::Color::LimeGreen;
			this->progressBar_raw_ch1->Location = System::Drawing::Point(110, 22);
			this->progressBar_raw_ch1->Name = L"progressBar_raw_ch1";
			this->progressBar_raw_ch1->Size = System::Drawing::Size(163, 21);
			this->progressBar_raw_ch1->Step = 1;
			this->progressBar_raw_ch1->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBar_raw_ch1->TabIndex = 61;
			// 
			// checkBox1_raw_ch1
			// 
			this->checkBox1_raw_ch1->AutoSize = true;
			this->checkBox1_raw_ch1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox1_raw_ch1->Location = System::Drawing::Point(4, 20);
			this->checkBox1_raw_ch1->Name = L"checkBox1_raw_ch1";
			this->checkBox1_raw_ch1->Size = System::Drawing::Size(107, 24);
			this->checkBox1_raw_ch1->TabIndex = 63;
			this->checkBox1_raw_ch1->Text = L"Ch1 (RAW)";
			this->checkBox1_raw_ch1->UseVisualStyleBackColor = true;
			// 
			// textBox_progress_raw_ch1
			// 
			this->textBox_progress_raw_ch1->BackColor = System::Drawing::Color::DimGray;
			this->textBox_progress_raw_ch1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->textBox_progress_raw_ch1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_progress_raw_ch1->ForeColor = System::Drawing::Color::White;
			this->textBox_progress_raw_ch1->Location = System::Drawing::Point(276, 19);
			this->textBox_progress_raw_ch1->Name = L"textBox_progress_raw_ch1";
			this->textBox_progress_raw_ch1->ReadOnly = true;
			this->textBox_progress_raw_ch1->Size = System::Drawing::Size(63, 27);
			this->textBox_progress_raw_ch1->TabIndex = 62;
			this->textBox_progress_raw_ch1->Text = L"---";
			this->textBox_progress_raw_ch1->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_raw_save
			// 
			this->button_raw_save->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button_raw_save->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_raw_save->Location = System::Drawing::Point(344, 17);
			this->button_raw_save->Name = L"button_raw_save";
			this->button_raw_save->Size = System::Drawing::Size(62, 30);
			this->button_raw_save->TabIndex = 61;
			this->button_raw_save->Text = L"RAW";
			this->button_raw_save->UseVisualStyleBackColor = true;
			this->button_raw_save->Click += gcnew System::EventHandler(this, &MTPFM::button_raw_save_Click);
			// 
			// label17
			// 
			this->label17->AutoSize = true;
			this->label17->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label17->ForeColor = System::Drawing::Color::White;
			this->label17->Location = System::Drawing::Point(2, 58);
			this->label17->Name = L"label17";
			this->label17->Size = System::Drawing::Size(85, 20);
			this->label17->TabIndex = 66;
			this->label17->Text = L"# of Frame";
			// 
			// labelraw_count
			// 
			this->labelraw_count->AutoSize = true;
			this->labelraw_count->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->labelraw_count->ForeColor = System::Drawing::Color::White;
			this->labelraw_count->Location = System::Drawing::Point(313, 56);
			this->labelraw_count->Name = L"labelraw_count";
			this->labelraw_count->Size = System::Drawing::Size(72, 20);
			this->labelraw_count->TabIndex = 64;
			this->labelraw_count->Text = L"Num = 1";
			// 
			// trackBar_raw_count
			// 
			this->trackBar_raw_count->AutoSize = false;
			this->trackBar_raw_count->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_raw_count->LargeChange = 1;
			this->trackBar_raw_count->Location = System::Drawing::Point(87, 59);
			this->trackBar_raw_count->Maximum = 2500;
			this->trackBar_raw_count->Minimum = 1;
			this->trackBar_raw_count->Name = L"trackBar_raw_count";
			this->trackBar_raw_count->Size = System::Drawing::Size(225, 20);
			this->trackBar_raw_count->TabIndex = 65;
			this->trackBar_raw_count->TickFrequency = 100;
			this->trackBar_raw_count->Value = 1;
			this->trackBar_raw_count->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_raw_count_Scroll);
			// 
			// tabPage5
			// 
			this->tabPage5->BackColor = System::Drawing::Color::DimGray;
			this->tabPage5->Controls->Add(this->button2_save_data);
			this->tabPage5->Controls->Add(this->progressBar_progress_exporting);
			this->tabPage5->Controls->Add(this->radioButton_checkBox_Activate_chan_1);
			this->tabPage5->Controls->Add(this->label1_num_frame);
			this->tabPage5->Controls->Add(this->checkBox1_save_ch1_data);
			this->tabPage5->Controls->Add(this->trackBar_num_frame);
			this->tabPage5->Controls->Add(this->textBox_progress_export);
			this->tabPage5->Controls->Add(this->label_frame_num);
			this->tabPage5->ForeColor = System::Drawing::Color::White;
			this->tabPage5->Location = System::Drawing::Point(4, 24);
			this->tabPage5->Name = L"tabPage5";
			this->tabPage5->Padding = System::Windows::Forms::Padding(3);
			this->tabPage5->Size = System::Drawing::Size(420, 113);
			this->tabPage5->TabIndex = 1;
			this->tabPage5->Text = L"Preview Image acq.";
			// 
			// button2_save_data
			// 
			this->button2_save_data->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button2_save_data->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button2_save_data->Location = System::Drawing::Point(343, 33);
			this->button2_save_data->Name = L"button2_save_data";
			this->button2_save_data->Size = System::Drawing::Size(62, 30);
			this->button2_save_data->TabIndex = 49;
			this->button2_save_data->Text = L"Save";
			this->button2_save_data->UseVisualStyleBackColor = true;
			this->button2_save_data->Click += gcnew System::EventHandler(this, &MTPFM::button2_save_data_Click);
			// 
			// progressBar_progress_exporting
			// 
			this->progressBar_progress_exporting->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(64)),
				static_cast<System::Int32>(static_cast<System::Byte>(64)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
			this->progressBar_progress_exporting->ForeColor = System::Drawing::Color::LimeGreen;
			this->progressBar_progress_exporting->Location = System::Drawing::Point(63, 39);
			this->progressBar_progress_exporting->Name = L"progressBar_progress_exporting";
			this->progressBar_progress_exporting->Size = System::Drawing::Size(205, 21);
			this->progressBar_progress_exporting->Step = 1;
			this->progressBar_progress_exporting->Style = System::Windows::Forms::ProgressBarStyle::Continuous;
			this->progressBar_progress_exporting->TabIndex = 31;
			// 
			// radioButton_checkBox_Activate_chan_1
			// 
			this->radioButton_checkBox_Activate_chan_1->AutoSize = true;
			this->radioButton_checkBox_Activate_chan_1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->radioButton_checkBox_Activate_chan_1->Location = System::Drawing::Point(8, 7);
			this->radioButton_checkBox_Activate_chan_1->Name = L"radioButton_checkBox_Activate_chan_1";
			this->radioButton_checkBox_Activate_chan_1->Size = System::Drawing::Size(102, 24);
			this->radioButton_checkBox_Activate_chan_1->TabIndex = 46;
			this->radioButton_checkBox_Activate_chan_1->TabStop = true;
			this->radioButton_checkBox_Activate_chan_1->Text = L"Active Ch1";
			this->radioButton_checkBox_Activate_chan_1->UseVisualStyleBackColor = true;
			this->radioButton_checkBox_Activate_chan_1->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::radioButton_checkBox_Activate_chan_1_CheckedChanged);
			// 
			// label1_num_frame
			// 
			this->label1_num_frame->AutoSize = true;
			this->label1_num_frame->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label1_num_frame->ForeColor = System::Drawing::Color::White;
			this->label1_num_frame->Location = System::Drawing::Point(315, 67);
			this->label1_num_frame->Name = L"label1_num_frame";
			this->label1_num_frame->Size = System::Drawing::Size(72, 20);
			this->label1_num_frame->TabIndex = 60;
			this->label1_num_frame->Text = L"Num = 1";
			// 
			// checkBox1_save_ch1_data
			// 
			this->checkBox1_save_ch1_data->AutoSize = true;
			this->checkBox1_save_ch1_data->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox1_save_ch1_data->Location = System::Drawing::Point(7, 37);
			this->checkBox1_save_ch1_data->Name = L"checkBox1_save_ch1_data";
			this->checkBox1_save_ch1_data->Size = System::Drawing::Size(55, 24);
			this->checkBox1_save_ch1_data->TabIndex = 41;
			this->checkBox1_save_ch1_data->Text = L"Ch1";
			this->checkBox1_save_ch1_data->UseVisualStyleBackColor = true;
			// 
			// trackBar_num_frame
			// 
			this->trackBar_num_frame->AutoSize = false;
			this->trackBar_num_frame->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_num_frame->LargeChange = 1;
			this->trackBar_num_frame->Location = System::Drawing::Point(92, 70);
			this->trackBar_num_frame->Maximum = 2500;
			this->trackBar_num_frame->Minimum = 1;
			this->trackBar_num_frame->Name = L"trackBar_num_frame";
			this->trackBar_num_frame->Size = System::Drawing::Size(225, 20);
			this->trackBar_num_frame->TabIndex = 60;
			this->trackBar_num_frame->TickFrequency = 100;
			this->trackBar_num_frame->Value = 1;
			this->trackBar_num_frame->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_num_frame_Scroll);
			// 
			// textBox_progress_export
			// 
			this->textBox_progress_export->BackColor = System::Drawing::Color::DimGray;
			this->textBox_progress_export->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->textBox_progress_export->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_progress_export->ForeColor = System::Drawing::Color::White;
			this->textBox_progress_export->Location = System::Drawing::Point(272, 36);
			this->textBox_progress_export->Name = L"textBox_progress_export";
			this->textBox_progress_export->ReadOnly = true;
			this->textBox_progress_export->Size = System::Drawing::Size(63, 27);
			this->textBox_progress_export->TabIndex = 32;
			this->textBox_progress_export->Text = L"---";
			this->textBox_progress_export->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label_frame_num
			// 
			this->label_frame_num->AutoSize = true;
			this->label_frame_num->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_frame_num->ForeColor = System::Drawing::Color::White;
			this->label_frame_num->Location = System::Drawing::Point(5, 69);
			this->label_frame_num->Name = L"label_frame_num";
			this->label_frame_num->Size = System::Drawing::Size(85, 20);
			this->label_frame_num->TabIndex = 60;
			this->label_frame_num->Text = L"# of Frame";
			// 
			// tabControl4
			// 
			this->tabControl4->Controls->Add(this->tabPage12);
			this->tabControl4->Controls->Add(this->tabPage19);
			this->tabControl4->Controls->Add(this->tabPage9);
			this->tabControl4->Controls->Add(this->tabPage6);
			this->tabControl4->Controls->Add(this->tabPage7);
			this->tabControl4->Controls->Add(this->tabPage10);
			this->tabControl4->Controls->Add(this->tabPage11);
			this->tabControl4->Location = System::Drawing::Point(8, 617);
			this->tabControl4->Name = L"tabControl4";
			this->tabControl4->SelectedIndex = 0;
			this->tabControl4->Size = System::Drawing::Size(428, 208);
			this->tabControl4->TabIndex = 68;
			// 
			// tabPage12
			// 
			this->tabPage12->BackColor = System::Drawing::Color::DimGray;
			this->tabPage12->Controls->Add(this->label16);
			this->tabPage12->Controls->Add(this->toggleButtons_Binning);
			this->tabPage12->Controls->Add(this->toggleButtons_signalproxessing);
			this->tabPage12->Controls->Add(this->label52);
			this->tabPage12->Controls->Add(this->label_multiplySP);
			this->tabPage12->Controls->Add(this->trackBar_multiplySP);
			this->tabPage12->Controls->Add(this->label55);
			this->tabPage12->Controls->Add(this->label_offsetvalueSP);
			this->tabPage12->Controls->Add(this->trackBar_offsetvalueSP);
			this->tabPage12->Location = System::Drawing::Point(4, 24);
			this->tabPage12->Name = L"tabPage12";
			this->tabPage12->Size = System::Drawing::Size(420, 180);
			this->tabPage12->TabIndex = 8;
			this->tabPage12->Text = L"Signal Processing";
			// 
			// label16
			// 
			this->label16->AutoSize = true;
			this->label16->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label16->ForeColor = System::Drawing::Color::White;
			this->label16->Location = System::Drawing::Point(62, 110);
			this->label16->Name = L"label16";
			this->label16->Size = System::Drawing::Size(63, 20);
			this->label16->TabIndex = 76;
			this->label16->Text = L"Binning";
			// 
			// toggleButtons_Binning
			// 
			this->toggleButtons_Binning->Location = System::Drawing::Point(9, 109);
			this->toggleButtons_Binning->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_Binning->Name = L"toggleButtons_Binning";
			this->toggleButtons_Binning->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_Binning->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_Binning->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_Binning->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_Binning->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_Binning->SolidStyle = false;
			this->toggleButtons_Binning->TabIndex = 75;
			this->toggleButtons_Binning->UseVisualStyleBackColor = true;
			this->toggleButtons_Binning->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_Binning_CheckedChanged);
			// 
			// toggleButtons_signalproxessing
			// 
			this->toggleButtons_signalproxessing->Location = System::Drawing::Point(8, 13);
			this->toggleButtons_signalproxessing->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_signalproxessing->Name = L"toggleButtons_signalproxessing";
			this->toggleButtons_signalproxessing->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_signalproxessing->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_signalproxessing->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_signalproxessing->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_signalproxessing->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_signalproxessing->SolidStyle = false;
			this->toggleButtons_signalproxessing->TabIndex = 74;
			this->toggleButtons_signalproxessing->UseVisualStyleBackColor = true;
			this->toggleButtons_signalproxessing->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_signalproxessing_CheckedChanged);
			// 
			// label52
			// 
			this->label52->AutoSize = true;
			this->label52->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label52->ForeColor = System::Drawing::Color::White;
			this->label52->Location = System::Drawing::Point(58, 63);
			this->label52->Name = L"label52";
			this->label52->Size = System::Drawing::Size(67, 20);
			this->label52->TabIndex = 71;
			this->label52->Text = L"Multiply";
			// 
			// label_multiplySP
			// 
			this->label_multiplySP->AutoSize = true;
			this->label_multiplySP->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_multiplySP->Location = System::Drawing::Point(361, 63);
			this->label_multiplySP->Name = L"label_multiplySP";
			this->label_multiplySP->Size = System::Drawing::Size(41, 21);
			this->label_multiplySP->TabIndex = 72;
			this->label_multiplySP->Text = L"2.00";
			// 
			// trackBar_multiplySP
			// 
			this->trackBar_multiplySP->AutoSize = false;
			this->trackBar_multiplySP->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_multiplySP->LargeChange = 500;
			this->trackBar_multiplySP->Location = System::Drawing::Point(149, 64);
			this->trackBar_multiplySP->Maximum = 500;
			this->trackBar_multiplySP->Minimum = 200;
			this->trackBar_multiplySP->Name = L"trackBar_multiplySP";
			this->trackBar_multiplySP->Size = System::Drawing::Size(219, 20);
			this->trackBar_multiplySP->SmallChange = 50;
			this->trackBar_multiplySP->TabIndex = 73;
			this->trackBar_multiplySP->TickFrequency = 25;
			this->trackBar_multiplySP->Value = 200;
			this->trackBar_multiplySP->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_multiplySP_Scroll);
			// 
			// label55
			// 
			this->label55->AutoSize = true;
			this->label55->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label55->ForeColor = System::Drawing::Color::White;
			this->label55->Location = System::Drawing::Point(58, 16);
			this->label55->Name = L"label55";
			this->label55->Size = System::Drawing::Size(95, 20);
			this->label55->TabIndex = 68;
			this->label55->Text = L"Offset Value";
			// 
			// label_offsetvalueSP
			// 
			this->label_offsetvalueSP->AutoSize = true;
			this->label_offsetvalueSP->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_offsetvalueSP->Location = System::Drawing::Point(362, 18);
			this->label_offsetvalueSP->Name = L"label_offsetvalueSP";
			this->label_offsetvalueSP->Size = System::Drawing::Size(55, 21);
			this->label_offsetvalueSP->TabIndex = 69;
			this->label_offsetvalueSP->Text = L"32768";
			// 
			// trackBar_offsetvalueSP
			// 
			this->trackBar_offsetvalueSP->AutoSize = false;
			this->trackBar_offsetvalueSP->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_offsetvalueSP->LargeChange = 500;
			this->trackBar_offsetvalueSP->Location = System::Drawing::Point(149, 19);
			this->trackBar_offsetvalueSP->Maximum = 65535;
			this->trackBar_offsetvalueSP->Name = L"trackBar_offsetvalueSP";
			this->trackBar_offsetvalueSP->Size = System::Drawing::Size(220, 20);
			this->trackBar_offsetvalueSP->SmallChange = 50;
			this->trackBar_offsetvalueSP->TabIndex = 70;
			this->trackBar_offsetvalueSP->TickFrequency = 25;
			this->trackBar_offsetvalueSP->Value = 32768;
			this->trackBar_offsetvalueSP->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_offsetvalueSP_Scroll);
			// 
			// tabPage19
			// 
			this->tabPage19->BackColor = System::Drawing::Color::DimGray;
			this->tabPage19->Controls->Add(this->label62);
			this->tabPage19->Controls->Add(this->label_DCEkernel);
			this->tabPage19->Controls->Add(this->trackBar_DCEkernel);
			this->tabPage19->Controls->Add(this->label56);
			this->tabPage19->Controls->Add(this->label_DCE);
			this->tabPage19->Controls->Add(this->trackBar_DCE);
			this->tabPage19->Controls->Add(this->toggleButtons_DCE);
			this->tabPage19->Controls->Add(this->toggleButtons_contrast);
			this->tabPage19->Controls->Add(this->label_adjust_gamma);
			this->tabPage19->Controls->Add(this->label18);
			this->tabPage19->Controls->Add(this->label_contrast_alpha);
			this->tabPage19->Controls->Add(this->label19);
			this->tabPage19->Controls->Add(this->label_brightness_beta);
			this->tabPage19->Controls->Add(this->label20);
			this->tabPage19->Controls->Add(this->trackBar_contrast_alpha);
			this->tabPage19->Controls->Add(this->trackBar_brightness_beta);
			this->tabPage19->Controls->Add(this->trackBar_adjust_gamma);
			this->tabPage19->Location = System::Drawing::Point(4, 24);
			this->tabPage19->Name = L"tabPage19";
			this->tabPage19->Padding = System::Windows::Forms::Padding(3);
			this->tabPage19->Size = System::Drawing::Size(420, 180);
			this->tabPage19->TabIndex = 6;
			this->tabPage19->Text = L"CONTRAST";
			// 
			// label62
			// 
			this->label62->AutoSize = true;
			this->label62->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label62->ForeColor = System::Drawing::Color::White;
			this->label62->Location = System::Drawing::Point(73, 154);
			this->label62->Name = L"label62";
			this->label62->Size = System::Drawing::Size(54, 20);
			this->label62->TabIndex = 71;
			this->label62->Text = L"Kernel";
			// 
			// label_DCEkernel
			// 
			this->label_DCEkernel->AutoSize = true;
			this->label_DCEkernel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_DCEkernel->Location = System::Drawing::Point(381, 156);
			this->label_DCEkernel->Name = L"label_DCEkernel";
			this->label_DCEkernel->Size = System::Drawing::Size(19, 21);
			this->label_DCEkernel->TabIndex = 72;
			this->label_DCEkernel->Text = L"7";
			// 
			// trackBar_DCEkernel
			// 
			this->trackBar_DCEkernel->AutoSize = false;
			this->trackBar_DCEkernel->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_DCEkernel->Location = System::Drawing::Point(157, 157);
			this->trackBar_DCEkernel->Maximum = 20;
			this->trackBar_DCEkernel->Minimum = 1;
			this->trackBar_DCEkernel->Name = L"trackBar_DCEkernel";
			this->trackBar_DCEkernel->Size = System::Drawing::Size(218, 20);
			this->trackBar_DCEkernel->TabIndex = 73;
			this->trackBar_DCEkernel->TickFrequency = 25;
			this->trackBar_DCEkernel->Value = 3;
			this->trackBar_DCEkernel->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_DCEkernel_Scroll);
			// 
			// label56
			// 
			this->label56->AutoSize = true;
			this->label56->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label56->ForeColor = System::Drawing::Color::White;
			this->label56->Location = System::Drawing::Point(73, 124);
			this->label56->Name = L"label56";
			this->label56->Size = System::Drawing::Size(50, 20);
			this->label56->TabIndex = 68;
			this->label56->Text = L"Alpha";
			// 
			// label_DCE
			// 
			this->label_DCE->AutoSize = true;
			this->label_DCE->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_DCE->Location = System::Drawing::Point(381, 126);
			this->label_DCE->Name = L"label_DCE";
			this->label_DCE->Size = System::Drawing::Size(19, 21);
			this->label_DCE->TabIndex = 69;
			this->label_DCE->Text = L"4";
			// 
			// trackBar_DCE
			// 
			this->trackBar_DCE->AutoSize = false;
			this->trackBar_DCE->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_DCE->LargeChange = 10;
			this->trackBar_DCE->Location = System::Drawing::Point(157, 127);
			this->trackBar_DCE->Maximum = 800;
			this->trackBar_DCE->Minimum = 300;
			this->trackBar_DCE->Name = L"trackBar_DCE";
			this->trackBar_DCE->Size = System::Drawing::Size(218, 20);
			this->trackBar_DCE->SmallChange = 5;
			this->trackBar_DCE->TabIndex = 70;
			this->trackBar_DCE->TickFrequency = 25;
			this->trackBar_DCE->Value = 400;
			this->trackBar_DCE->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_DCE_Scroll);
			// 
			// toggleButtons_DCE
			// 
			this->toggleButtons_DCE->Location = System::Drawing::Point(8, 122);
			this->toggleButtons_DCE->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_DCE->Name = L"toggleButtons_DCE";
			this->toggleButtons_DCE->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_DCE->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_DCE->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_DCE->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_DCE->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_DCE->SolidStyle = false;
			this->toggleButtons_DCE->TabIndex = 67;
			this->toggleButtons_DCE->UseVisualStyleBackColor = true;
			this->toggleButtons_DCE->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_DCE_CheckedChanged);
			// 
			// toggleButtons_contrast
			// 
			this->toggleButtons_contrast->Location = System::Drawing::Point(8, 13);
			this->toggleButtons_contrast->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_contrast->Name = L"toggleButtons_contrast";
			this->toggleButtons_contrast->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_contrast->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_contrast->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_contrast->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_contrast->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_contrast->SolidStyle = false;
			this->toggleButtons_contrast->TabIndex = 66;
			this->toggleButtons_contrast->UseVisualStyleBackColor = true;
			this->toggleButtons_contrast->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_contrast_CheckedChanged);
			// 
			// label_adjust_gamma
			// 
			this->label_adjust_gamma->AutoSize = true;
			this->label_adjust_gamma->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_adjust_gamma->Location = System::Drawing::Point(378, 84);
			this->label_adjust_gamma->Name = L"label_adjust_gamma";
			this->label_adjust_gamma->Size = System::Drawing::Size(28, 21);
			this->label_adjust_gamma->TabIndex = 65;
			this->label_adjust_gamma->Text = L"15";
			// 
			// label18
			// 
			this->label18->AutoSize = true;
			this->label18->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label18->ForeColor = System::Drawing::Color::White;
			this->label18->Location = System::Drawing::Point(73, 11);
			this->label18->Name = L"label18";
			this->label18->Size = System::Drawing::Size(84, 20);
			this->label18->TabIndex = 59;
			this->label18->Text = L"Brightness";
			// 
			// label_contrast_alpha
			// 
			this->label_contrast_alpha->AutoSize = true;
			this->label_contrast_alpha->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_contrast_alpha->Location = System::Drawing::Point(378, 50);
			this->label_contrast_alpha->Name = L"label_contrast_alpha";
			this->label_contrast_alpha->Size = System::Drawing::Size(28, 21);
			this->label_contrast_alpha->TabIndex = 64;
			this->label_contrast_alpha->Text = L"14";
			// 
			// label19
			// 
			this->label19->AutoSize = true;
			this->label19->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label19->ForeColor = System::Drawing::Color::White;
			this->label19->Location = System::Drawing::Point(73, 46);
			this->label19->Name = L"label19";
			this->label19->Size = System::Drawing::Size(69, 20);
			this->label19->TabIndex = 60;
			this->label19->Text = L"Contrast";
			// 
			// label_brightness_beta
			// 
			this->label_brightness_beta->AutoSize = true;
			this->label_brightness_beta->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_brightness_beta->Location = System::Drawing::Point(378, 13);
			this->label_brightness_beta->Name = L"label_brightness_beta";
			this->label_brightness_beta->Size = System::Drawing::Size(37, 21);
			this->label_brightness_beta->TabIndex = 59;
			this->label_brightness_beta->Text = L"150";
			// 
			// label20
			// 
			this->label20->AutoSize = true;
			this->label20->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label20->ForeColor = System::Drawing::Color::White;
			this->label20->Location = System::Drawing::Point(73, 81);
			this->label20->Name = L"label20";
			this->label20->Size = System::Drawing::Size(64, 20);
			this->label20->TabIndex = 61;
			this->label20->Text = L"Gamma";
			// 
			// trackBar_contrast_alpha
			// 
			this->trackBar_contrast_alpha->AutoSize = false;
			this->trackBar_contrast_alpha->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_contrast_alpha->LargeChange = 2;
			this->trackBar_contrast_alpha->Location = System::Drawing::Point(157, 50);
			this->trackBar_contrast_alpha->Maximum = 80;
			this->trackBar_contrast_alpha->Minimum = 10;
			this->trackBar_contrast_alpha->Name = L"trackBar_contrast_alpha";
			this->trackBar_contrast_alpha->Size = System::Drawing::Size(218, 20);
			this->trackBar_contrast_alpha->TabIndex = 63;
			this->trackBar_contrast_alpha->TickFrequency = 5;
			this->trackBar_contrast_alpha->Value = 26;
			this->trackBar_contrast_alpha->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_contrast_alpha_Scroll);
			// 
			// trackBar_brightness_beta
			// 
			this->trackBar_brightness_beta->AutoSize = false;
			this->trackBar_brightness_beta->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_brightness_beta->LargeChange = 10;
			this->trackBar_brightness_beta->Location = System::Drawing::Point(157, 14);
			this->trackBar_brightness_beta->Maximum = 300;
			this->trackBar_brightness_beta->Name = L"trackBar_brightness_beta";
			this->trackBar_brightness_beta->Size = System::Drawing::Size(218, 20);
			this->trackBar_brightness_beta->SmallChange = 5;
			this->trackBar_brightness_beta->TabIndex = 59;
			this->trackBar_brightness_beta->TickFrequency = 25;
			this->trackBar_brightness_beta->Value = 150;
			this->trackBar_brightness_beta->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_brightness_beta_Scroll);
			// 
			// trackBar_adjust_gamma
			// 
			this->trackBar_adjust_gamma->AutoSize = false;
			this->trackBar_adjust_gamma->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_adjust_gamma->Location = System::Drawing::Point(157, 82);
			this->trackBar_adjust_gamma->Maximum = 800;
			this->trackBar_adjust_gamma->Minimum = 1;
			this->trackBar_adjust_gamma->Name = L"trackBar_adjust_gamma";
			this->trackBar_adjust_gamma->Size = System::Drawing::Size(218, 20);
			this->trackBar_adjust_gamma->SmallChange = 2;
			this->trackBar_adjust_gamma->TabIndex = 62;
			this->trackBar_adjust_gamma->TickFrequency = 50;
			this->trackBar_adjust_gamma->Value = 110;
			this->trackBar_adjust_gamma->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_adjust_gamma_Scroll);
			// 
			// tabPage9
			// 
			this->tabPage9->BackColor = System::Drawing::Color::DimGray;
			this->tabPage9->Controls->Add(this->toggleButtons_normalize);
			this->tabPage9->Controls->Add(this->label59);
			this->tabPage9->Controls->Add(this->label_normalizeMax);
			this->tabPage9->Controls->Add(this->trackBar_normalizeMax);
			this->tabPage9->Controls->Add(this->label61);
			this->tabPage9->Controls->Add(this->label_normalizeMin);
			this->tabPage9->Controls->Add(this->trackBar_normalizeMin);
			this->tabPage9->Controls->Add(this->label58);
			this->tabPage9->Controls->Add(this->toggleButtons_equalhistogram);
			this->tabPage9->Controls->Add(this->toggleButtons_minmax);
			this->tabPage9->Controls->Add(this->label53);
			this->tabPage9->Controls->Add(this->label_MaxVal);
			this->tabPage9->Controls->Add(this->trackBar_MaxVal);
			this->tabPage9->Controls->Add(this->label51);
			this->tabPage9->Controls->Add(this->label_MinVal);
			this->tabPage9->Controls->Add(this->trackBar_MinVal);
			this->tabPage9->Location = System::Drawing::Point(4, 24);
			this->tabPage9->Name = L"tabPage9";
			this->tabPage9->Size = System::Drawing::Size(420, 180);
			this->tabPage9->TabIndex = 7;
			this->tabPage9->Text = L"MinMax Value";
			// 
			// toggleButtons_normalize
			// 
			this->toggleButtons_normalize->Location = System::Drawing::Point(8, 121);
			this->toggleButtons_normalize->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_normalize->Name = L"toggleButtons_normalize";
			this->toggleButtons_normalize->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_normalize->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_normalize->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_normalize->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_normalize->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_normalize->SolidStyle = false;
			this->toggleButtons_normalize->TabIndex = 85;
			this->toggleButtons_normalize->UseVisualStyleBackColor = true;
			this->toggleButtons_normalize->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_normalize_CheckedChanged);
			// 
			// label59
			// 
			this->label59->AutoSize = true;
			this->label59->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label59->ForeColor = System::Drawing::Color::White;
			this->label59->Location = System::Drawing::Point(58, 155);
			this->label59->Name = L"label59";
			this->label59->Size = System::Drawing::Size(124, 20);
			this->label59->TabIndex = 82;
			this->label59->Text = L"Normalized Max";
			// 
			// label_normalizeMax
			// 
			this->label_normalizeMax->AutoSize = true;
			this->label_normalizeMax->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_normalizeMax->Location = System::Drawing::Point(361, 155);
			this->label_normalizeMax->Name = L"label_normalizeMax";
			this->label_normalizeMax->Size = System::Drawing::Size(55, 21);
			this->label_normalizeMax->TabIndex = 83;
			this->label_normalizeMax->Text = L"65535";
			// 
			// trackBar_normalizeMax
			// 
			this->trackBar_normalizeMax->AutoSize = false;
			this->trackBar_normalizeMax->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_normalizeMax->LargeChange = 500;
			this->trackBar_normalizeMax->Location = System::Drawing::Point(179, 156);
			this->trackBar_normalizeMax->Maximum = 52768;
			this->trackBar_normalizeMax->Name = L"trackBar_normalizeMax";
			this->trackBar_normalizeMax->Size = System::Drawing::Size(177, 20);
			this->trackBar_normalizeMax->SmallChange = 50;
			this->trackBar_normalizeMax->TabIndex = 84;
			this->trackBar_normalizeMax->TickFrequency = 25;
			this->trackBar_normalizeMax->Value = 52768;
			this->trackBar_normalizeMax->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_normalizeMax_Scroll);
			// 
			// label61
			// 
			this->label61->AutoSize = true;
			this->label61->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label61->ForeColor = System::Drawing::Color::White;
			this->label61->Location = System::Drawing::Point(58, 125);
			this->label61->Name = L"label61";
			this->label61->Size = System::Drawing::Size(121, 20);
			this->label61->TabIndex = 79;
			this->label61->Text = L"Normalized Min";
			// 
			// label_normalizeMin
			// 
			this->label_normalizeMin->AutoSize = true;
			this->label_normalizeMin->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_normalizeMin->Location = System::Drawing::Point(362, 127);
			this->label_normalizeMin->Name = L"label_normalizeMin";
			this->label_normalizeMin->Size = System::Drawing::Size(19, 21);
			this->label_normalizeMin->TabIndex = 80;
			this->label_normalizeMin->Text = L"0";
			// 
			// trackBar_normalizeMin
			// 
			this->trackBar_normalizeMin->AutoSize = false;
			this->trackBar_normalizeMin->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_normalizeMin->LargeChange = 500;
			this->trackBar_normalizeMin->Location = System::Drawing::Point(179, 128);
			this->trackBar_normalizeMin->Maximum = 32768;
			this->trackBar_normalizeMin->Name = L"trackBar_normalizeMin";
			this->trackBar_normalizeMin->Size = System::Drawing::Size(177, 20);
			this->trackBar_normalizeMin->SmallChange = 50;
			this->trackBar_normalizeMin->TabIndex = 81;
			this->trackBar_normalizeMin->TickFrequency = 25;
			this->trackBar_normalizeMin->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_normalizeMin_Scroll);
			// 
			// label58
			// 
			this->label58->AutoSize = true;
			this->label58->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label58->ForeColor = System::Drawing::Color::White;
			this->label58->Location = System::Drawing::Point(61, 78);
			this->label58->Name = L"label58";
			this->label58->Size = System::Drawing::Size(172, 20);
			this->label58->TabIndex = 78;
			this->label58->Text = L"Histogram equalization";
			// 
			// toggleButtons_equalhistogram
			// 
			this->toggleButtons_equalhistogram->Location = System::Drawing::Point(8, 77);
			this->toggleButtons_equalhistogram->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_equalhistogram->Name = L"toggleButtons_equalhistogram";
			this->toggleButtons_equalhistogram->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_equalhistogram->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_equalhistogram->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_equalhistogram->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_equalhistogram->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_equalhistogram->SolidStyle = false;
			this->toggleButtons_equalhistogram->TabIndex = 77;
			this->toggleButtons_equalhistogram->UseVisualStyleBackColor = true;
			this->toggleButtons_equalhistogram->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_equalhistogram_CheckedChanged);
			// 
			// toggleButtons_minmax
			// 
			this->toggleButtons_minmax->Location = System::Drawing::Point(8, 13);
			this->toggleButtons_minmax->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_minmax->Name = L"toggleButtons_minmax";
			this->toggleButtons_minmax->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_minmax->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_minmax->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_minmax->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_minmax->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_minmax->SolidStyle = false;
			this->toggleButtons_minmax->TabIndex = 67;
			this->toggleButtons_minmax->UseVisualStyleBackColor = true;
			this->toggleButtons_minmax->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_minmax_CheckedChanged);
			// 
			// label53
			// 
			this->label53->AutoSize = true;
			this->label53->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label53->ForeColor = System::Drawing::Color::White;
			this->label53->Location = System::Drawing::Point(58, 41);
			this->label53->Name = L"label53";
			this->label53->Size = System::Drawing::Size(81, 20);
			this->label53->TabIndex = 63;
			this->label53->Text = L"Max Value";
			// 
			// label_MaxVal
			// 
			this->label_MaxVal->AutoSize = true;
			this->label_MaxVal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_MaxVal->Location = System::Drawing::Point(361, 41);
			this->label_MaxVal->Name = L"label_MaxVal";
			this->label_MaxVal->Size = System::Drawing::Size(55, 21);
			this->label_MaxVal->TabIndex = 64;
			this->label_MaxVal->Text = L"65535";
			// 
			// trackBar_MaxVal
			// 
			this->trackBar_MaxVal->AutoSize = false;
			this->trackBar_MaxVal->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_MaxVal->LargeChange = 500;
			this->trackBar_MaxVal->Location = System::Drawing::Point(136, 42);
			this->trackBar_MaxVal->Maximum = 52768;
			this->trackBar_MaxVal->Name = L"trackBar_MaxVal";
			this->trackBar_MaxVal->Size = System::Drawing::Size(219, 20);
			this->trackBar_MaxVal->SmallChange = 50;
			this->trackBar_MaxVal->TabIndex = 65;
			this->trackBar_MaxVal->TickFrequency = 25;
			this->trackBar_MaxVal->Value = 52768;
			this->trackBar_MaxVal->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_MaxVal_Scroll);
			// 
			// label51
			// 
			this->label51->AutoSize = true;
			this->label51->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label51->ForeColor = System::Drawing::Color::White;
			this->label51->Location = System::Drawing::Point(58, 11);
			this->label51->Name = L"label51";
			this->label51->Size = System::Drawing::Size(78, 20);
			this->label51->TabIndex = 60;
			this->label51->Text = L"Min Value";
			// 
			// label_MinVal
			// 
			this->label_MinVal->AutoSize = true;
			this->label_MinVal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_MinVal->Location = System::Drawing::Point(362, 13);
			this->label_MinVal->Name = L"label_MinVal";
			this->label_MinVal->Size = System::Drawing::Size(19, 21);
			this->label_MinVal->TabIndex = 61;
			this->label_MinVal->Text = L"0";
			// 
			// trackBar_MinVal
			// 
			this->trackBar_MinVal->AutoSize = false;
			this->trackBar_MinVal->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_MinVal->LargeChange = 500;
			this->trackBar_MinVal->Location = System::Drawing::Point(136, 14);
			this->trackBar_MinVal->Maximum = 32768;
			this->trackBar_MinVal->Name = L"trackBar_MinVal";
			this->trackBar_MinVal->Size = System::Drawing::Size(220, 20);
			this->trackBar_MinVal->SmallChange = 50;
			this->trackBar_MinVal->TabIndex = 62;
			this->trackBar_MinVal->TickFrequency = 25;
			this->trackBar_MinVal->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_MinVal_Scroll);
			// 
			// tabPage6
			// 
			this->tabPage6->BackColor = System::Drawing::Color::DimGray;
			this->tabPage6->Controls->Add(this->label_Gaussian_sigmaV);
			this->tabPage6->Controls->Add(this->label_Gaussian_sigmaH);
			this->tabPage6->Controls->Add(this->label_Gaussian_kernel);
			this->tabPage6->Controls->Add(this->trackBar_Gaussian_sigmaV);
			this->tabPage6->Controls->Add(this->trackBar_Gaussian_sigmaH);
			this->tabPage6->Controls->Add(this->trackBar_Gaussian_kernel);
			this->tabPage6->Controls->Add(this->toggleButtons_Gaussian);
			this->tabPage6->Location = System::Drawing::Point(4, 24);
			this->tabPage6->Name = L"tabPage6";
			this->tabPage6->Padding = System::Windows::Forms::Padding(3);
			this->tabPage6->Size = System::Drawing::Size(420, 180);
			this->tabPage6->TabIndex = 0;
			this->tabPage6->Text = L"Gaussian Blur";
			// 
			// label_Gaussian_sigmaV
			// 
			this->label_Gaussian_sigmaV->AutoSize = true;
			this->label_Gaussian_sigmaV->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_Gaussian_sigmaV->Location = System::Drawing::Point(303, 69);
			this->label_Gaussian_sigmaV->Name = L"label_Gaussian_sigmaV";
			this->label_Gaussian_sigmaV->Size = System::Drawing::Size(106, 21);
			this->label_Gaussian_sigmaV->TabIndex = 68;
			this->label_Gaussian_sigmaV->Text = L"SigmaV = 10";
			// 
			// label_Gaussian_sigmaH
			// 
			this->label_Gaussian_sigmaH->AutoSize = true;
			this->label_Gaussian_sigmaH->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_Gaussian_sigmaH->Location = System::Drawing::Point(303, 41);
			this->label_Gaussian_sigmaH->Name = L"label_Gaussian_sigmaH";
			this->label_Gaussian_sigmaH->Size = System::Drawing::Size(107, 21);
			this->label_Gaussian_sigmaH->TabIndex = 67;
			this->label_Gaussian_sigmaH->Text = L"SigmaH = 10";
			// 
			// label_Gaussian_kernel
			// 
			this->label_Gaussian_kernel->AutoSize = true;
			this->label_Gaussian_kernel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_Gaussian_kernel->Location = System::Drawing::Point(303, 13);
			this->label_Gaussian_kernel->Name = L"label_Gaussian_kernel";
			this->label_Gaussian_kernel->Size = System::Drawing::Size(87, 21);
			this->label_Gaussian_kernel->TabIndex = 66;
			this->label_Gaussian_kernel->Text = L"Kernel = 5";
			// 
			// trackBar_Gaussian_sigmaV
			// 
			this->trackBar_Gaussian_sigmaV->AutoSize = false;
			this->trackBar_Gaussian_sigmaV->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_Gaussian_sigmaV->LargeChange = 1;
			this->trackBar_Gaussian_sigmaV->Location = System::Drawing::Point(73, 75);
			this->trackBar_Gaussian_sigmaV->Maximum = 40;
			this->trackBar_Gaussian_sigmaV->Minimum = 1;
			this->trackBar_Gaussian_sigmaV->Name = L"trackBar_Gaussian_sigmaV";
			this->trackBar_Gaussian_sigmaV->Size = System::Drawing::Size(222, 20);
			this->trackBar_Gaussian_sigmaV->TabIndex = 62;
			this->trackBar_Gaussian_sigmaV->TickFrequency = 4;
			this->trackBar_Gaussian_sigmaV->Value = 10;
			this->trackBar_Gaussian_sigmaV->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_Gaussian_sigmaV_Scroll);
			// 
			// trackBar_Gaussian_sigmaH
			// 
			this->trackBar_Gaussian_sigmaH->AutoSize = false;
			this->trackBar_Gaussian_sigmaH->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_Gaussian_sigmaH->LargeChange = 1;
			this->trackBar_Gaussian_sigmaH->Location = System::Drawing::Point(73, 47);
			this->trackBar_Gaussian_sigmaH->Maximum = 40;
			this->trackBar_Gaussian_sigmaH->Minimum = 1;
			this->trackBar_Gaussian_sigmaH->Name = L"trackBar_Gaussian_sigmaH";
			this->trackBar_Gaussian_sigmaH->Size = System::Drawing::Size(222, 20);
			this->trackBar_Gaussian_sigmaH->TabIndex = 61;
			this->trackBar_Gaussian_sigmaH->TickFrequency = 4;
			this->trackBar_Gaussian_sigmaH->Value = 10;
			this->trackBar_Gaussian_sigmaH->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_Gaussian_sigmaH_Scroll);
			// 
			// trackBar_Gaussian_kernel
			// 
			this->trackBar_Gaussian_kernel->AutoSize = false;
			this->trackBar_Gaussian_kernel->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_Gaussian_kernel->LargeChange = 10;
			this->trackBar_Gaussian_kernel->Location = System::Drawing::Point(73, 19);
			this->trackBar_Gaussian_kernel->Maximum = 29;
			this->trackBar_Gaussian_kernel->Minimum = 1;
			this->trackBar_Gaussian_kernel->Name = L"trackBar_Gaussian_kernel";
			this->trackBar_Gaussian_kernel->Size = System::Drawing::Size(222, 20);
			this->trackBar_Gaussian_kernel->TabIndex = 60;
			this->trackBar_Gaussian_kernel->TickFrequency = 3;
			this->trackBar_Gaussian_kernel->Value = 5;
			this->trackBar_Gaussian_kernel->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_Gaussian_kernel_Scroll);
			// 
			// toggleButtons_Gaussian
			// 
			this->toggleButtons_Gaussian->Location = System::Drawing::Point(8, 13);
			this->toggleButtons_Gaussian->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_Gaussian->Name = L"toggleButtons_Gaussian";
			this->toggleButtons_Gaussian->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_Gaussian->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_Gaussian->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_Gaussian->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_Gaussian->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_Gaussian->SolidStyle = false;
			this->toggleButtons_Gaussian->TabIndex = 49;
			this->toggleButtons_Gaussian->UseVisualStyleBackColor = true;
			this->toggleButtons_Gaussian->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_Gaussian_CheckedChanged);
			// 
			// tabPage7
			// 
			this->tabPage7->BackColor = System::Drawing::Color::DimGray;
			this->tabPage7->Controls->Add(this->label_Median_kernel);
			this->tabPage7->Controls->Add(this->trackBar_Median_kernel);
			this->tabPage7->Controls->Add(this->toggleButtons_Median);
			this->tabPage7->Location = System::Drawing::Point(4, 24);
			this->tabPage7->Name = L"tabPage7";
			this->tabPage7->Padding = System::Windows::Forms::Padding(3);
			this->tabPage7->Size = System::Drawing::Size(420, 180);
			this->tabPage7->TabIndex = 1;
			this->tabPage7->Text = L"Median Blur";
			// 
			// label_Median_kernel
			// 
			this->label_Median_kernel->AutoSize = true;
			this->label_Median_kernel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_Median_kernel->Location = System::Drawing::Point(307, 17);
			this->label_Median_kernel->Name = L"label_Median_kernel";
			this->label_Median_kernel->Size = System::Drawing::Size(87, 21);
			this->label_Median_kernel->TabIndex = 67;
			this->label_Median_kernel->Text = L"Kernel = 3";
			// 
			// trackBar_Median_kernel
			// 
			this->trackBar_Median_kernel->AutoSize = false;
			this->trackBar_Median_kernel->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_Median_kernel->LargeChange = 1;
			this->trackBar_Median_kernel->Location = System::Drawing::Point(74, 19);
			this->trackBar_Median_kernel->Maximum = 5;
			this->trackBar_Median_kernel->Minimum = 1;
			this->trackBar_Median_kernel->Name = L"trackBar_Median_kernel";
			this->trackBar_Median_kernel->Size = System::Drawing::Size(222, 20);
			this->trackBar_Median_kernel->SmallChange = 5;
			this->trackBar_Median_kernel->TabIndex = 62;
			this->trackBar_Median_kernel->Value = 1;
			this->trackBar_Median_kernel->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_Median_kernel_Scroll);
			// 
			// toggleButtons_Median
			// 
			this->toggleButtons_Median->Location = System::Drawing::Point(8, 13);
			this->toggleButtons_Median->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_Median->Name = L"toggleButtons_Median";
			this->toggleButtons_Median->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_Median->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_Median->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_Median->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_Median->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_Median->SolidStyle = false;
			this->toggleButtons_Median->TabIndex = 50;
			this->toggleButtons_Median->UseVisualStyleBackColor = true;
			this->toggleButtons_Median->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_Median_CheckedChanged);
			// 
			// tabPage10
			// 
			this->tabPage10->BackColor = System::Drawing::Color::DimGray;
			this->tabPage10->Controls->Add(this->label_tile_grid_V);
			this->tabPage10->Controls->Add(this->label_tile_grid_H);
			this->tabPage10->Controls->Add(this->label_clip_limit);
			this->tabPage10->Controls->Add(this->trackBar_tile_grid_V);
			this->tabPage10->Controls->Add(this->trackBar_tile_grid_H);
			this->tabPage10->Controls->Add(this->trackBar_clip_limit);
			this->tabPage10->Controls->Add(this->toggleButtons_CLAHE);
			this->tabPage10->Location = System::Drawing::Point(4, 24);
			this->tabPage10->Name = L"tabPage10";
			this->tabPage10->Padding = System::Windows::Forms::Padding(3);
			this->tabPage10->Size = System::Drawing::Size(420, 180);
			this->tabPage10->TabIndex = 4;
			this->tabPage10->Text = L"CLAHE";
			// 
			// label_tile_grid_V
			// 
			this->label_tile_grid_V->AutoSize = true;
			this->label_tile_grid_V->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_tile_grid_V->Location = System::Drawing::Point(301, 74);
			this->label_tile_grid_V->Name = L"label_tile_grid_V";
			this->label_tile_grid_V->Size = System::Drawing::Size(115, 21);
			this->label_tile_grid_V->TabIndex = 74;
			this->label_tile_grid_V->Text = L"tileGridV = 20";
			// 
			// label_tile_grid_H
			// 
			this->label_tile_grid_H->AutoSize = true;
			this->label_tile_grid_H->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_tile_grid_H->Location = System::Drawing::Point(301, 46);
			this->label_tile_grid_H->Name = L"label_tile_grid_H";
			this->label_tile_grid_H->Size = System::Drawing::Size(116, 21);
			this->label_tile_grid_H->TabIndex = 73;
			this->label_tile_grid_H->Text = L"tileGridH = 20";
			// 
			// label_clip_limit
			// 
			this->label_clip_limit->AutoSize = true;
			this->label_clip_limit->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_clip_limit->Location = System::Drawing::Point(301, 18);
			this->label_clip_limit->Name = L"label_clip_limit";
			this->label_clip_limit->Size = System::Drawing::Size(107, 21);
			this->label_clip_limit->TabIndex = 72;
			this->label_clip_limit->Text = L"ClipLimit = 3";
			// 
			// trackBar_tile_grid_V
			// 
			this->trackBar_tile_grid_V->AutoSize = false;
			this->trackBar_tile_grid_V->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_tile_grid_V->LargeChange = 1;
			this->trackBar_tile_grid_V->Location = System::Drawing::Point(73, 76);
			this->trackBar_tile_grid_V->Maximum = 200;
			this->trackBar_tile_grid_V->Minimum = 1;
			this->trackBar_tile_grid_V->Name = L"trackBar_tile_grid_V";
			this->trackBar_tile_grid_V->Size = System::Drawing::Size(222, 20);
			this->trackBar_tile_grid_V->TabIndex = 71;
			this->trackBar_tile_grid_V->TickFrequency = 20;
			this->trackBar_tile_grid_V->Value = 20;
			this->trackBar_tile_grid_V->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_tile_grid_V_Scroll);
			// 
			// trackBar_tile_grid_H
			// 
			this->trackBar_tile_grid_H->AutoSize = false;
			this->trackBar_tile_grid_H->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_tile_grid_H->LargeChange = 1;
			this->trackBar_tile_grid_H->Location = System::Drawing::Point(73, 48);
			this->trackBar_tile_grid_H->Maximum = 200;
			this->trackBar_tile_grid_H->Minimum = 1;
			this->trackBar_tile_grid_H->Name = L"trackBar_tile_grid_H";
			this->trackBar_tile_grid_H->Size = System::Drawing::Size(222, 20);
			this->trackBar_tile_grid_H->TabIndex = 70;
			this->trackBar_tile_grid_H->TickFrequency = 20;
			this->trackBar_tile_grid_H->Value = 20;
			this->trackBar_tile_grid_H->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_tile_grid_H_Scroll);
			// 
			// trackBar_clip_limit
			// 
			this->trackBar_clip_limit->AutoSize = false;
			this->trackBar_clip_limit->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_clip_limit->LargeChange = 1;
			this->trackBar_clip_limit->Location = System::Drawing::Point(73, 20);
			this->trackBar_clip_limit->Maximum = 15;
			this->trackBar_clip_limit->Minimum = 1;
			this->trackBar_clip_limit->Name = L"trackBar_clip_limit";
			this->trackBar_clip_limit->Size = System::Drawing::Size(222, 20);
			this->trackBar_clip_limit->TabIndex = 69;
			this->trackBar_clip_limit->TickFrequency = 3;
			this->trackBar_clip_limit->Value = 3;
			this->trackBar_clip_limit->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_clip_limit_Scroll);
			// 
			// toggleButtons_CLAHE
			// 
			this->toggleButtons_CLAHE->Location = System::Drawing::Point(8, 13);
			this->toggleButtons_CLAHE->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_CLAHE->Name = L"toggleButtons_CLAHE";
			this->toggleButtons_CLAHE->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_CLAHE->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons_CLAHE->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons_CLAHE->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons_CLAHE->Size = System::Drawing::Size(47, 25);
			this->toggleButtons_CLAHE->SolidStyle = false;
			this->toggleButtons_CLAHE->TabIndex = 51;
			this->toggleButtons_CLAHE->UseVisualStyleBackColor = true;
			this->toggleButtons_CLAHE->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::toggleButtons_CLAHE_CheckedChanged);
			// 
			// tabPage11
			// 
			this->tabPage11->BackColor = System::Drawing::Color::DimGray;
			this->tabPage11->Controls->Add(this->textBox1_open_image);
			this->tabPage11->Controls->Add(this->button_start_frames_saving);
			this->tabPage11->Controls->Add(this->button_load_directory);
			this->tabPage11->Controls->Add(this->label_BiD_align_Val_bin);
			this->tabPage11->Controls->Add(this->textBox_brief_detail);
			this->tabPage11->Controls->Add(this->trackBar_BiD_align_bin);
			this->tabPage11->Controls->Add(this->label_BiD_align_bin);
			this->tabPage11->Location = System::Drawing::Point(4, 24);
			this->tabPage11->Name = L"tabPage11";
			this->tabPage11->Padding = System::Windows::Forms::Padding(3);
			this->tabPage11->Size = System::Drawing::Size(420, 180);
			this->tabPage11->TabIndex = 5;
			this->tabPage11->Text = L"Post Processing";
			// 
			// textBox1_open_image
			// 
			this->textBox1_open_image->BackColor = System::Drawing::Color::Gray;
			this->textBox1_open_image->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox1_open_image->ForeColor = System::Drawing::Color::Cyan;
			this->textBox1_open_image->Location = System::Drawing::Point(5, 71);
			this->textBox1_open_image->Multiline = true;
			this->textBox1_open_image->Name = L"textBox1_open_image";
			this->textBox1_open_image->RightToLeft = System::Windows::Forms::RightToLeft::No;
			this->textBox1_open_image->Size = System::Drawing::Size(337, 29);
			this->textBox1_open_image->TabIndex = 28;
			this->textBox1_open_image->Text = L"---";
			this->textBox1_open_image->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_start_frames_saving
			// 
			this->button_start_frames_saving->BackColor = System::Drawing::Color::White;
			this->button_start_frames_saving->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_start_frames_saving.BackgroundImage")));
			this->button_start_frames_saving->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
			this->button_start_frames_saving->Location = System::Drawing::Point(352, 69);
			this->button_start_frames_saving->Name = L"button_start_frames_saving";
			this->button_start_frames_saving->Size = System::Drawing::Size(62, 33);
			this->button_start_frames_saving->TabIndex = 47;
			this->button_start_frames_saving->UseVisualStyleBackColor = false;
			this->button_start_frames_saving->Click += gcnew System::EventHandler(this, &MTPFM::button_start_frames_saving_Click);
			// 
			// button_load_directory
			// 
			this->button_load_directory->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button_load_directory->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_load_directory->Location = System::Drawing::Point(349, 8);
			this->button_load_directory->Name = L"button_load_directory";
			this->button_load_directory->Size = System::Drawing::Size(68, 29);
			this->button_load_directory->TabIndex = 48;
			this->button_load_directory->Text = L"LOAD";
			this->button_load_directory->UseVisualStyleBackColor = true;
			this->button_load_directory->Click += gcnew System::EventHandler(this, &MTPFM::button_load_directory_Click);
			// 
			// label_BiD_align_Val_bin
			// 
			this->label_BiD_align_Val_bin->AutoSize = true;
			this->label_BiD_align_Val_bin->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_BiD_align_Val_bin->ForeColor = System::Drawing::Color::Cyan;
			this->label_BiD_align_Val_bin->Location = System::Drawing::Point(331, 43);
			this->label_BiD_align_Val_bin->Name = L"label_BiD_align_Val_bin";
			this->label_BiD_align_Val_bin->Size = System::Drawing::Size(81, 20);
			this->label_BiD_align_Val_bin->TabIndex = 59;
			this->label_BiD_align_Val_bin->Text = L"Offset = 0";
			// 
			// textBox_brief_detail
			// 
			this->textBox_brief_detail->BackColor = System::Drawing::Color::LightGray;
			this->textBox_brief_detail->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_brief_detail->Location = System::Drawing::Point(5, 8);
			this->textBox_brief_detail->Multiline = true;
			this->textBox_brief_detail->Name = L"textBox_brief_detail";
			this->textBox_brief_detail->Size = System::Drawing::Size(342, 29);
			this->textBox_brief_detail->TabIndex = 28;
			this->textBox_brief_detail->TextChanged += gcnew System::EventHandler(this, &MTPFM::textBox_brief_detail_TextChanged);
			// 
			// trackBar_BiD_align_bin
			// 
			this->trackBar_BiD_align_bin->AutoSize = false;
			this->trackBar_BiD_align_bin->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_BiD_align_bin->LargeChange = 1;
			this->trackBar_BiD_align_bin->Location = System::Drawing::Point(106, 48);
			this->trackBar_BiD_align_bin->Maximum = 100;
			this->trackBar_BiD_align_bin->Minimum = -100;
			this->trackBar_BiD_align_bin->Name = L"trackBar_BiD_align_bin";
			this->trackBar_BiD_align_bin->Size = System::Drawing::Size(225, 20);
			this->trackBar_BiD_align_bin->TabIndex = 59;
			this->trackBar_BiD_align_bin->TickFrequency = 20;
			this->trackBar_BiD_align_bin->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_BiD_align_bin_Scroll);
			// 
			// label_BiD_align_bin
			// 
			this->label_BiD_align_bin->AutoSize = true;
			this->label_BiD_align_bin->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_BiD_align_bin->ForeColor = System::Drawing::Color::Cyan;
			this->label_BiD_align_bin->Location = System::Drawing::Point(4, 45);
			this->label_BiD_align_bin->Name = L"label_BiD_align_bin";
			this->label_BiD_align_bin->Size = System::Drawing::Size(100, 20);
			this->label_BiD_align_bin->TabIndex = 59;
			this->label_BiD_align_bin->Text = L"BiD PostProc";
			// 
			// label15
			// 
			this->label15->AutoSize = true;
			this->label15->BackColor = System::Drawing::Color::Transparent;
			this->label15->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label15->Location = System::Drawing::Point(7, 594);
			this->label15->Name = L"label15";
			this->label15->Size = System::Drawing::Size(178, 20);
			this->label15->TabIndex = 43;
			this->label15->Text = L"Image Processing Panel";
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPage2);
			this->tabControl1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->tabControl1->Location = System::Drawing::Point(7, 199);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(428, 168);
			this->tabControl1->TabIndex = 26;
			// 
			// tabPage2
			// 
			this->tabPage2->BackColor = System::Drawing::Color::DimGray;
			this->tabPage2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->tabPage2->Controls->Add(this->checkBox_save_data_alazar);
			this->tabPage2->Controls->Add(this->comboBox_prev_scale_down_bin_read);
			this->tabPage2->Controls->Add(this->toggleButtons1);
			this->tabPage2->Controls->Add(this->button2_open_image);
			this->tabPage2->Controls->Add(this->label14);
			this->tabPage2->Controls->Add(this->label13);
			this->tabPage2->Controls->Add(this->comboBox_prev_scale_down);
			this->tabPage2->Controls->Add(this->checkBox_merge_channels);
			this->tabPage2->Controls->Add(this->button_save_loaded_frame);
			this->tabPage2->Controls->Add(this->checkBox_bin_export_full_frame);
			this->tabPage2->Controls->Add(this->checkBox_bin_export_cropped);
			this->tabPage2->Controls->Add(this->checkBox1_mode);
			this->tabPage2->ForeColor = System::Drawing::Color::White;
			this->tabPage2->Location = System::Drawing::Point(4, 24);
			this->tabPage2->Name = L"tabPage2";
			this->tabPage2->Padding = System::Windows::Forms::Padding(3);
			this->tabPage2->Size = System::Drawing::Size(420, 140);
			this->tabPage2->TabIndex = 1;
			this->tabPage2->Text = L"Controls / Settings";
			// 
			// checkBox_save_data_alazar
			// 
			this->checkBox_save_data_alazar->AutoSize = true;
			this->checkBox_save_data_alazar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_save_data_alazar->Location = System::Drawing::Point(209, 70);
			this->checkBox_save_data_alazar->Name = L"checkBox_save_data_alazar";
			this->checkBox_save_data_alazar->Size = System::Drawing::Size(174, 24);
			this->checkBox_save_data_alazar->TabIndex = 48;
			this->checkBox_save_data_alazar->Text = L" Save Alazar Bin data";
			this->checkBox_save_data_alazar->UseVisualStyleBackColor = true;
			// 
			// comboBox_prev_scale_down_bin_read
			// 
			this->comboBox_prev_scale_down_bin_read->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->comboBox_prev_scale_down_bin_read->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->comboBox_prev_scale_down_bin_read->FormattingEnabled = true;
			this->comboBox_prev_scale_down_bin_read->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"None", L"1× down", L"2× down",
					L"3× down", L"4× down", L"5× down"
			});
			this->comboBox_prev_scale_down_bin_read->Location = System::Drawing::Point(330, 41);
			this->comboBox_prev_scale_down_bin_read->Name = L"comboBox_prev_scale_down_bin_read";
			this->comboBox_prev_scale_down_bin_read->Size = System::Drawing::Size(73, 28);
			this->comboBox_prev_scale_down_bin_read->TabIndex = 47;
			this->comboBox_prev_scale_down_bin_read->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::comboBox_prev_scale_down_bin_read_SelectedIndexChanged);
			// 
			// toggleButtons1
			// 
			this->toggleButtons1->Location = System::Drawing::Point(294, 102);
			this->toggleButtons1->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons1->Name = L"toggleButtons1";
			this->toggleButtons1->OffBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons1->OffToggleColor = System::Drawing::Color::Gainsboro;
			this->toggleButtons1->OnBackColor = System::Drawing::Color::WhiteSmoke;
			this->toggleButtons1->OnToggleColor = System::Drawing::Color::DodgerBlue;
			this->toggleButtons1->Size = System::Drawing::Size(47, 25);
			this->toggleButtons1->SolidStyle = false;
			this->toggleButtons1->TabIndex = 27;
			this->toggleButtons1->UseVisualStyleBackColor = true;
			// 
			// button2_open_image
			// 
			this->button2_open_image->BackColor = System::Drawing::Color::White;
			this->button2_open_image->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button2_open_image.BackgroundImage")));
			this->button2_open_image->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Zoom;
			this->button2_open_image->Location = System::Drawing::Point(349, 97);
			this->button2_open_image->Name = L"button2_open_image";
			this->button2_open_image->Size = System::Drawing::Size(62, 33);
			this->button2_open_image->TabIndex = 27;
			this->button2_open_image->UseVisualStyleBackColor = false;
			this->button2_open_image->Click += gcnew System::EventHandler(this, &MTPFM::button2_open_image_Click);
			// 
			// label14
			// 
			this->label14->AutoSize = true;
			this->label14->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label14->ForeColor = System::Drawing::Color::White;
			this->label14->Location = System::Drawing::Point(205, 44);
			this->label14->Name = L"label14";
			this->label14->Size = System::Drawing::Size(123, 20);
			this->label14->TabIndex = 46;
			this->label14->Text = L"Scale DOWN bin";
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label13->ForeColor = System::Drawing::Color::White;
			this->label13->Location = System::Drawing::Point(205, 11);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(97, 20);
			this->label13->TabIndex = 43;
			this->label13->Text = L"Scale DOWN";
			// 
			// comboBox_prev_scale_down
			// 
			this->comboBox_prev_scale_down->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->comboBox_prev_scale_down->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->comboBox_prev_scale_down->FormattingEnabled = true;
			this->comboBox_prev_scale_down->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"None", L"1× down", L"2× down",
					L"3× down", L"4× down", L"5× down"
			});
			this->comboBox_prev_scale_down->Location = System::Drawing::Point(330, 5);
			this->comboBox_prev_scale_down->Name = L"comboBox_prev_scale_down";
			this->comboBox_prev_scale_down->Size = System::Drawing::Size(73, 28);
			this->comboBox_prev_scale_down->TabIndex = 43;
			this->comboBox_prev_scale_down->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::comboBox_prev_scale_down_SelectedIndexChanged);
			// 
			// checkBox_merge_channels
			// 
			this->checkBox_merge_channels->AutoSize = true;
			this->checkBox_merge_channels->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_merge_channels->Location = System::Drawing::Point(6, 8);
			this->checkBox_merge_channels->Name = L"checkBox_merge_channels";
			this->checkBox_merge_channels->Size = System::Drawing::Size(77, 24);
			this->checkBox_merge_channels->TabIndex = 45;
			this->checkBox_merge_channels->Text = L" Merge";
			this->checkBox_merge_channels->UseVisualStyleBackColor = true;
			this->checkBox_merge_channels->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox_merge_channels_CheckedChanged);
			// 
			// button_save_loaded_frame
			// 
			this->button_save_loaded_frame->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button_save_loaded_frame->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_save_loaded_frame->Location = System::Drawing::Point(209, 100);
			this->button_save_loaded_frame->Name = L"button_save_loaded_frame";
			this->button_save_loaded_frame->Size = System::Drawing::Size(71, 30);
			this->button_save_loaded_frame->TabIndex = 29;
			this->button_save_loaded_frame->Text = L"Load";
			this->button_save_loaded_frame->UseVisualStyleBackColor = true;
			this->button_save_loaded_frame->Click += gcnew System::EventHandler(this, &MTPFM::button_save_loaded_frame_Click);
			// 
			// checkBox_bin_export_full_frame
			// 
			this->checkBox_bin_export_full_frame->AutoSize = true;
			this->checkBox_bin_export_full_frame->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->checkBox_bin_export_full_frame->Location = System::Drawing::Point(6, 101);
			this->checkBox_bin_export_full_frame->Name = L"checkBox_bin_export_full_frame";
			this->checkBox_bin_export_full_frame->Size = System::Drawing::Size(57, 24);
			this->checkBox_bin_export_full_frame->TabIndex = 44;
			this->checkBox_bin_export_full_frame->Text = L" Full";
			this->checkBox_bin_export_full_frame->UseVisualStyleBackColor = true;
			this->checkBox_bin_export_full_frame->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox_bin_export_full_frame_CheckedChanged);
			// 
			// checkBox_bin_export_cropped
			// 
			this->checkBox_bin_export_cropped->AutoSize = true;
			this->checkBox_bin_export_cropped->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox_bin_export_cropped->Location = System::Drawing::Point(6, 70);
			this->checkBox_bin_export_cropped->Name = L"checkBox_bin_export_cropped";
			this->checkBox_bin_export_cropped->Size = System::Drawing::Size(91, 24);
			this->checkBox_bin_export_cropped->TabIndex = 43;
			this->checkBox_bin_export_cropped->Text = L" Cropped";
			this->checkBox_bin_export_cropped->UseVisualStyleBackColor = true;
			this->checkBox_bin_export_cropped->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox_bin_export_cropped_CheckedChanged);
			// 
			// checkBox1_mode
			// 
			this->checkBox1_mode->AutoSize = true;
			this->checkBox1_mode->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->checkBox1_mode->Location = System::Drawing::Point(6, 39);
			this->checkBox1_mode->Name = L"checkBox1_mode";
			this->checkBox1_mode->Size = System::Drawing::Size(160, 24);
			this->checkBox1_mode->TabIndex = 42;
			this->checkBox1_mode->Text = L" Dark/Bright Mode";
			this->checkBox1_mode->UseVisualStyleBackColor = true;
			this->checkBox1_mode->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::checkBox1_mode_CheckedChanged);
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->BackColor = System::Drawing::Color::Transparent;
			this->label4->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label4->Location = System::Drawing::Point(6, 174);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(138, 20);
			this->label4->TabIndex = 25;
			this->label4->Text = L"Control Tab Panel";
			// 
			// button9
			// 
			this->button9->BackColor = System::Drawing::Color::RosyBrown;
			this->button9->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.5F, System::Drawing::FontStyle::Bold));
			this->button9->ForeColor = System::Drawing::Color::White;
			this->button9->Location = System::Drawing::Point(7, 42);
			this->button9->Name = L"button9";
			this->button9->Size = System::Drawing::Size(152, 31);
			this->button9->TabIndex = 67;
			this->button9->Text = L"Post Processing";
			this->button9->UseVisualStyleBackColor = false;
			this->button9->Click += gcnew System::EventHandler(this, &MTPFM::button9_Click);
			// 
			// YScomboBox_intclock
			// 
			this->YScomboBox_intclock->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->YScomboBox_intclock->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->YScomboBox_intclock->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScomboBox_intclock->FormattingEnabled = true;
			this->YScomboBox_intclock->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"2 MS/s", L"10 MS/s", L"20 MS/s", L"50 MS/s",
					L"100 MS/s", L"125 MS/s"
			});
			this->YScomboBox_intclock->Location = System::Drawing::Point(308, 15);
			this->YScomboBox_intclock->Name = L"YScomboBox_intclock";
			this->YScomboBox_intclock->Size = System::Drawing::Size(84, 25);
			this->YScomboBox_intclock->TabIndex = 102;
			// 
			// YScheckBox_extclock
			// 
			this->YScheckBox_extclock->AutoSize = true;
			this->YScheckBox_extclock->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScheckBox_extclock->ForeColor = System::Drawing::Color::White;
			this->YScheckBox_extclock->Location = System::Drawing::Point(7, 17);
			this->YScheckBox_extclock->Name = L"YScheckBox_extclock";
			this->YScheckBox_extclock->Size = System::Drawing::Size(93, 24);
			this->YScheckBox_extclock->TabIndex = 99;
			this->YScheckBox_extclock->Text = L"Ext. clock";
			this->YScheckBox_extclock->UseVisualStyleBackColor = true;
			this->YScheckBox_extclock->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::YScheckBox_extclock_CheckedChanged);
			// 
			// YScheckBox_intclock
			// 
			this->YScheckBox_intclock->AutoSize = true;
			this->YScheckBox_intclock->Checked = true;
			this->YScheckBox_intclock->CheckState = System::Windows::Forms::CheckState::Checked;
			this->YScheckBox_intclock->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScheckBox_intclock->Location = System::Drawing::Point(211, 17);
			this->YScheckBox_intclock->Name = L"YScheckBox_intclock";
			this->YScheckBox_intclock->Size = System::Drawing::Size(91, 24);
			this->YScheckBox_intclock->TabIndex = 100;
			this->YScheckBox_intclock->Text = L"Int. clock";
			this->YScheckBox_intclock->UseVisualStyleBackColor = true;
			this->YScheckBox_intclock->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::YScheckBox_intclock_CheckedChanged);
			// 
			// YSbutton_lissajous_setamp
			// 
			this->YSbutton_lissajous_setamp->Controls->Add(this->tabPage16);
			this->YSbutton_lissajous_setamp->Controls->Add(this->tabPage15);
			this->YSbutton_lissajous_setamp->Controls->Add(this->tabPage17);
			this->YSbutton_lissajous_setamp->Location = System::Drawing::Point(7, 81);
			this->YSbutton_lissajous_setamp->Name = L"YSbutton_lissajous_setamp";
			this->YSbutton_lissajous_setamp->SelectedIndex = 0;
			this->YSbutton_lissajous_setamp->Size = System::Drawing::Size(398, 304);
			this->YSbutton_lissajous_setamp->TabIndex = 75;
			// 
			// tabPage16
			// 
			this->tabPage16->BackColor = System::Drawing::Color::DimGray;
			this->tabPage16->Controls->Add(this->label65);
			this->tabPage16->Controls->Add(this->YScomboBox__raster_chopperfreq);
			this->tabPage16->Controls->Add(this->label64);
			this->tabPage16->Controls->Add(this->YSbutton_raster_setamp);
			this->tabPage16->Controls->Add(this->YStextBox__raster_y_amp);
			this->tabPage16->Controls->Add(this->YStextBox__raster_x_amp);
			this->tabPage16->Controls->Add(this->YScheckBox_raster_slowspeed);
			this->tabPage16->Controls->Add(this->YSlabel_raster_phase);
			this->tabPage16->Controls->Add(this->YStrackBar_raster_phase);
			this->tabPage16->Controls->Add(this->label46);
			this->tabPage16->Controls->Add(this->YSbutton_raster_startimg);
			this->tabPage16->Controls->Add(this->YSbutton_raster_stop);
			this->tabPage16->Controls->Add(this->YSbutton_raster_configure);
			this->tabPage16->Controls->Add(this->YScomboBox__raster_yline);
			this->tabPage16->Controls->Add(this->label32);
			this->tabPage16->Controls->Add(this->YScomboBox__raster_linefreq);
			this->tabPage16->Controls->Add(this->label31);
			this->tabPage16->Controls->Add(this->YStextBox__raster_yoffset);
			this->tabPage16->Controls->Add(this->YStextBox__raster_xoffset);
			this->tabPage16->Controls->Add(this->label22);
			this->tabPage16->Controls->Add(this->label24);
			this->tabPage16->Controls->Add(this->YStrackBar__raster_y_amp);
			this->tabPage16->Controls->Add(this->label26);
			this->tabPage16->Controls->Add(this->YStrackBar__raster_x_amp);
			this->tabPage16->Controls->Add(this->label29);
			this->tabPage16->ForeColor = System::Drawing::Color::Transparent;
			this->tabPage16->Location = System::Drawing::Point(4, 24);
			this->tabPage16->Name = L"tabPage16";
			this->tabPage16->Padding = System::Windows::Forms::Padding(3);
			this->tabPage16->Size = System::Drawing::Size(390, 276);
			this->tabPage16->TabIndex = 1;
			this->tabPage16->Text = L"2D Raster";
			// 
			// YSbutton_raster_setamp
			// 
			this->YSbutton_raster_setamp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YSbutton_raster_setamp->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton_raster_setamp->Location = System::Drawing::Point(323, 93);
			this->YSbutton_raster_setamp->Name = L"YSbutton_raster_setamp";
			this->YSbutton_raster_setamp->Size = System::Drawing::Size(64, 58);
			this->YSbutton_raster_setamp->TabIndex = 105;
			this->YSbutton_raster_setamp->Text = L"Set amp.";
			this->YSbutton_raster_setamp->UseVisualStyleBackColor = true;
			this->YSbutton_raster_setamp->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_raster_setamp_Click);
			// 
			// YStextBox__raster_y_amp
			// 
			this->YStextBox__raster_y_amp->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__raster_y_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__raster_y_amp->Location = System::Drawing::Point(256, 125);
			this->YStextBox__raster_y_amp->Name = L"YStextBox__raster_y_amp";
			this->YStextBox__raster_y_amp->Size = System::Drawing::Size(61, 27);
			this->YStextBox__raster_y_amp->TabIndex = 104;
			this->YStextBox__raster_y_amp->Text = L"0.5";
			this->YStextBox__raster_y_amp->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox__raster_x_amp
			// 
			this->YStextBox__raster_x_amp->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__raster_x_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__raster_x_amp->Location = System::Drawing::Point(256, 90);
			this->YStextBox__raster_x_amp->Name = L"YStextBox__raster_x_amp";
			this->YStextBox__raster_x_amp->Size = System::Drawing::Size(61, 27);
			this->YStextBox__raster_x_amp->TabIndex = 103;
			this->YStextBox__raster_x_amp->Text = L"0.5";
			this->YStextBox__raster_x_amp->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YScheckBox_raster_slowspeed
			// 
			this->YScheckBox_raster_slowspeed->AutoSize = true;
			this->YScheckBox_raster_slowspeed->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11, System::Drawing::FontStyle::Bold));
			this->YScheckBox_raster_slowspeed->Location = System::Drawing::Point(282, 60);
			this->YScheckBox_raster_slowspeed->Name = L"YScheckBox_raster_slowspeed";
			this->YScheckBox_raster_slowspeed->Size = System::Drawing::Size(107, 24);
			this->YScheckBox_raster_slowspeed->TabIndex = 102;
			this->YScheckBox_raster_slowspeed->Text = L"Slow Speed";
			this->YScheckBox_raster_slowspeed->UseVisualStyleBackColor = true;
			// 
			// YSlabel_raster_phase
			// 
			this->YSlabel_raster_phase->AutoSize = true;
			this->YSlabel_raster_phase->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSlabel_raster_phase->Location = System::Drawing::Point(254, 210);
			this->YSlabel_raster_phase->Name = L"YSlabel_raster_phase";
			this->YSlabel_raster_phase->Size = System::Drawing::Size(18, 20);
			this->YSlabel_raster_phase->TabIndex = 95;
			this->YSlabel_raster_phase->Text = L"0";
			// 
			// YStrackBar_raster_phase
			// 
			this->YStrackBar_raster_phase->AutoSize = false;
			this->YStrackBar_raster_phase->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar_raster_phase->LargeChange = 0;
			this->YStrackBar_raster_phase->Location = System::Drawing::Point(91, 210);
			this->YStrackBar_raster_phase->Maximum = 7500;
			this->YStrackBar_raster_phase->Name = L"YStrackBar_raster_phase";
			this->YStrackBar_raster_phase->Size = System::Drawing::Size(163, 30);
			this->YStrackBar_raster_phase->TabIndex = 94;
			this->YStrackBar_raster_phase->TickFrequency = 200;
			this->YStrackBar_raster_phase->Value = 1;
			this->YStrackBar_raster_phase->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar_raster_phase_Scroll);
			// 
			// label46
			// 
			this->label46->AutoSize = true;
			this->label46->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label46->ForeColor = System::Drawing::Color::White;
			this->label46->Location = System::Drawing::Point(14, 206);
			this->label46->Name = L"label46";
			this->label46->Size = System::Drawing::Size(50, 20);
			this->label46->TabIndex = 93;
			this->label46->Text = L"Phase";
			// 
			// YSbutton_raster_startimg
			// 
			this->YSbutton_raster_startimg->BackColor = System::Drawing::Color::LightGray;
			this->YSbutton_raster_startimg->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->YSbutton_raster_startimg->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSbutton_raster_startimg->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->YSbutton_raster_startimg->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"YSbutton_raster_startimg.Image")));
			this->YSbutton_raster_startimg->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->YSbutton_raster_startimg->Location = System::Drawing::Point(127, 17);
			this->YSbutton_raster_startimg->Name = L"YSbutton_raster_startimg";
			this->YSbutton_raster_startimg->Size = System::Drawing::Size(169, 32);
			this->YSbutton_raster_startimg->TabIndex = 92;
			this->YSbutton_raster_startimg->Text = L"START Imaging";
			this->YSbutton_raster_startimg->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->YSbutton_raster_startimg->UseVisualStyleBackColor = false;
			this->YSbutton_raster_startimg->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_raster_startimg_Click);
			// 
			// YSbutton_raster_stop
			// 
			this->YSbutton_raster_stop->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YSbutton_raster_stop->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton_raster_stop->Location = System::Drawing::Point(307, 18);
			this->YSbutton_raster_stop->Name = L"YSbutton_raster_stop";
			this->YSbutton_raster_stop->Size = System::Drawing::Size(65, 30);
			this->YSbutton_raster_stop->TabIndex = 90;
			this->YSbutton_raster_stop->Text = L"Stop";
			this->YSbutton_raster_stop->UseVisualStyleBackColor = true;
			this->YSbutton_raster_stop->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_raster_stop_Click);
			// 
			// YSbutton_raster_configure
			// 
			this->YSbutton_raster_configure->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YSbutton_raster_configure->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->YSbutton_raster_configure->Location = System::Drawing::Point(18, 17);
			this->YSbutton_raster_configure->Name = L"YSbutton_raster_configure";
			this->YSbutton_raster_configure->Size = System::Drawing::Size(101, 31);
			this->YSbutton_raster_configure->TabIndex = 89;
			this->YSbutton_raster_configure->Text = L"Configure";
			this->YSbutton_raster_configure->UseVisualStyleBackColor = true;
			this->YSbutton_raster_configure->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_raster_configure_Click);
			// 
			// YScomboBox__raster_yline
			// 
			this->YScomboBox__raster_yline->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->YScomboBox__raster_yline->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScomboBox__raster_yline->FormattingEnabled = true;
			this->YScomboBox__raster_yline->Items->AddRange(gcnew cli::array< System::Object^  >(7) {
				L"16", L"32", L"64", L"128", L"256",
					L"512", L"1024"
			});
			this->YScomboBox__raster_yline->Location = System::Drawing::Point(268, 165);
			this->YScomboBox__raster_yline->Name = L"YScomboBox__raster_yline";
			this->YScomboBox__raster_yline->Size = System::Drawing::Size(73, 28);
			this->YScomboBox__raster_yline->TabIndex = 71;
			// 
			// label32
			// 
			this->label32->AutoSize = true;
			this->label32->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label32->ForeColor = System::Drawing::Color::White;
			this->label32->Location = System::Drawing::Point(188, 168);
			this->label32->Name = L"label32";
			this->label32->Size = System::Drawing::Size(66, 20);
			this->label32->TabIndex = 70;
			this->label32->Text = L"# Y-Line";
			// 
			// YScomboBox__raster_linefreq
			// 
			this->YScomboBox__raster_linefreq->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->YScomboBox__raster_linefreq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YScomboBox__raster_linefreq->FormattingEnabled = true;
			this->YScomboBox__raster_linefreq->Items->AddRange(gcnew cli::array< System::Object^  >(10) {
				L"200", L"250", L"400", L"500",
					L"800", L"1000", L"1250", L"1600", L"2000", L"2500"
			});
			this->YScomboBox__raster_linefreq->Location = System::Drawing::Point(97, 166);
			this->YScomboBox__raster_linefreq->Name = L"YScomboBox__raster_linefreq";
			this->YScomboBox__raster_linefreq->Size = System::Drawing::Size(73, 28);
			this->YScomboBox__raster_linefreq->TabIndex = 69;
			// 
			// label31
			// 
			this->label31->AutoSize = true;
			this->label31->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label31->ForeColor = System::Drawing::Color::White;
			this->label31->Location = System::Drawing::Point(14, 168);
			this->label31->Name = L"label31";
			this->label31->Size = System::Drawing::Size(77, 20);
			this->label31->TabIndex = 68;
			this->label31->Text = L"Line Freq.";
			// 
			// YStextBox__raster_yoffset
			// 
			this->YStextBox__raster_yoffset->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__raster_yoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__raster_yoffset->Location = System::Drawing::Point(215, 58);
			this->YStextBox__raster_yoffset->Name = L"YStextBox__raster_yoffset";
			this->YStextBox__raster_yoffset->Size = System::Drawing::Size(61, 27);
			this->YStextBox__raster_yoffset->TabIndex = 66;
			this->YStextBox__raster_yoffset->Text = L"0";
			this->YStextBox__raster_yoffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox__raster_xoffset
			// 
			this->YStextBox__raster_xoffset->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__raster_xoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__raster_xoffset->Location = System::Drawing::Point(83, 58);
			this->YStextBox__raster_xoffset->Name = L"YStextBox__raster_xoffset";
			this->YStextBox__raster_xoffset->Size = System::Drawing::Size(61, 27);
			this->YStextBox__raster_xoffset->TabIndex = 65;
			this->YStextBox__raster_xoffset->Text = L"0";
			this->YStextBox__raster_xoffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label22
			// 
			this->label22->AutoSize = true;
			this->label22->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label22->ForeColor = System::Drawing::Color::White;
			this->label22->Location = System::Drawing::Point(149, 61);
			this->label22->Name = L"label22";
			this->label22->Size = System::Drawing::Size(62, 20);
			this->label22->TabIndex = 64;
			this->label22->Text = L"YOffset";
			// 
			// label24
			// 
			this->label24->AutoSize = true;
			this->label24->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label24->ForeColor = System::Drawing::Color::White;
			this->label24->Location = System::Drawing::Point(14, 61);
			this->label24->Name = L"label24";
			this->label24->Size = System::Drawing::Size(63, 20);
			this->label24->TabIndex = 63;
			this->label24->Text = L"XOffset";
			// 
			// YStrackBar__raster_y_amp
			// 
			this->YStrackBar__raster_y_amp->AutoSize = false;
			this->YStrackBar__raster_y_amp->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar__raster_y_amp->LargeChange = 1;
			this->YStrackBar__raster_y_amp->Location = System::Drawing::Point(91, 127);
			this->YStrackBar__raster_y_amp->Maximum = 400;
			this->YStrackBar__raster_y_amp->Name = L"YStrackBar__raster_y_amp";
			this->YStrackBar__raster_y_amp->Size = System::Drawing::Size(163, 30);
			this->YStrackBar__raster_y_amp->TabIndex = 60;
			this->YStrackBar__raster_y_amp->TickFrequency = 5;
			this->YStrackBar__raster_y_amp->Value = 50;
			this->YStrackBar__raster_y_amp->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar__raster_y_amp_Scroll);
			// 
			// label26
			// 
			this->label26->AutoSize = true;
			this->label26->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label26->ForeColor = System::Drawing::Color::White;
			this->label26->Location = System::Drawing::Point(14, 125);
			this->label26->Name = L"label26";
			this->label26->Size = System::Drawing::Size(56, 20);
			this->label26->TabIndex = 61;
			this->label26->Text = L"y amp.";
			// 
			// YStrackBar__raster_x_amp
			// 
			this->YStrackBar__raster_x_amp->AutoSize = false;
			this->YStrackBar__raster_x_amp->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar__raster_x_amp->LargeChange = 1;
			this->YStrackBar__raster_x_amp->Location = System::Drawing::Point(91, 93);
			this->YStrackBar__raster_x_amp->Maximum = 400;
			this->YStrackBar__raster_x_amp->Name = L"YStrackBar__raster_x_amp";
			this->YStrackBar__raster_x_amp->Size = System::Drawing::Size(163, 30);
			this->YStrackBar__raster_x_amp->TabIndex = 58;
			this->YStrackBar__raster_x_amp->TickFrequency = 5;
			this->YStrackBar__raster_x_amp->Value = 50;
			this->YStrackBar__raster_x_amp->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar__raster_x_amp_Scroll);
			// 
			// label29
			// 
			this->label29->AutoSize = true;
			this->label29->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label29->ForeColor = System::Drawing::Color::White;
			this->label29->Location = System::Drawing::Point(14, 94);
			this->label29->Name = L"label29";
			this->label29->Size = System::Drawing::Size(56, 20);
			this->label29->TabIndex = 59;
			this->label29->Text = L"x amp.";
			// 
			// tabPage15
			// 
			this->tabPage15->BackColor = System::Drawing::Color::DimGray;
			this->tabPage15->Controls->Add(this->textBox_highspeedmode);
			this->tabPage15->Controls->Add(this->YScheckBox_lissajous_repairshow);
			this->tabPage15->Controls->Add(this->YStextBox__lissajous_y_amp);
			this->tabPage15->Controls->Add(this->YStextBox__lissajous_x_amp);
			this->tabPage15->Controls->Add(this->YSbutton__lissajous_setamp);
			this->tabPage15->Controls->Add(this->YSlabel__lissajous_offsettime);
			this->tabPage15->Controls->Add(this->YStrackBar__lissajous_offsettime);
			this->tabPage15->Controls->Add(this->label49);
			this->tabPage15->Controls->Add(this->YScheckBox_lissajous_samplemapsave);
			this->tabPage15->Controls->Add(this->YSlabel__lissajous_phase);
			this->tabPage15->Controls->Add(this->YStrackBar__lissajous_phase);
			this->tabPage15->Controls->Add(this->label47);
			this->tabPage15->Controls->Add(this->YStextBox__lissajous_y_freq);
			this->tabPage15->Controls->Add(this->YStextBox__lissajous_x_freq);
			this->tabPage15->Controls->Add(this->YSbutton_lissajous_startimg);
			this->tabPage15->Controls->Add(this->YSbutton_lissajous_stop);
			this->tabPage15->Controls->Add(this->YSbutton_lissajous_configure);
			this->tabPage15->Controls->Add(this->label33);
			this->tabPage15->Controls->Add(this->label34);
			this->tabPage15->Controls->Add(this->YStextBox__lissajous_yoffset);
			this->tabPage15->Controls->Add(this->YStextBox__lissajous_xoffset);
			this->tabPage15->Controls->Add(this->label36);
			this->tabPage15->Controls->Add(this->label37);
			this->tabPage15->Controls->Add(this->YStrackBar__lissajous_y_amp);
			this->tabPage15->Controls->Add(this->label39);
			this->tabPage15->Controls->Add(this->YStrackBar__lissajous_x_amp);
			this->tabPage15->Controls->Add(this->label40);
			this->tabPage15->ForeColor = System::Drawing::Color::White;
			this->tabPage15->Location = System::Drawing::Point(4, 24);
			this->tabPage15->Name = L"tabPage15";
			this->tabPage15->Padding = System::Windows::Forms::Padding(3);
			this->tabPage15->Size = System::Drawing::Size(390, 276);
			this->tabPage15->TabIndex = 0;
			this->tabPage15->Text = L"2D Lisssajous";
			// 
			// textBox_highspeedmode
			// 
			this->textBox_highspeedmode->Anchor = System::Windows::Forms::AnchorStyles::Top;
			this->textBox_highspeedmode->BackColor = System::Drawing::Color::DimGray;
			this->textBox_highspeedmode->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox_highspeedmode->Enabled = false;
			this->textBox_highspeedmode->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_highspeedmode->ForeColor = System::Drawing::Color::Gray;
			this->textBox_highspeedmode->Location = System::Drawing::Point(287, 176);
			this->textBox_highspeedmode->Name = L"textBox_highspeedmode";
			this->textBox_highspeedmode->Size = System::Drawing::Size(101, 16);
			this->textBox_highspeedmode->TabIndex = 110;
			this->textBox_highspeedmode->Text = L"High Speed Mode";
			// 
			// YScheckBox_lissajous_repairshow
			// 
			this->YScheckBox_lissajous_repairshow->AutoSize = true;
			this->YScheckBox_lissajous_repairshow->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YScheckBox_lissajous_repairshow->Location = System::Drawing::Point(312, 240);
			this->YScheckBox_lissajous_repairshow->Name = L"YScheckBox_lissajous_repairshow";
			this->YScheckBox_lissajous_repairshow->Size = System::Drawing::Size(73, 24);
			this->YScheckBox_lissajous_repairshow->TabIndex = 109;
			this->YScheckBox_lissajous_repairshow->Text = L"Repair";
			this->YScheckBox_lissajous_repairshow->UseVisualStyleBackColor = true;
			this->YScheckBox_lissajous_repairshow->CheckedChanged += gcnew System::EventHandler(this, &MTPFM::YScheckBox_lissajous_repairshow_CheckedChanged);
			// 
			// YStextBox__lissajous_y_amp
			// 
			this->YStextBox__lissajous_y_amp->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__lissajous_y_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__lissajous_y_amp->Location = System::Drawing::Point(256, 125);
			this->YStextBox__lissajous_y_amp->Name = L"YStextBox__lissajous_y_amp";
			this->YStextBox__lissajous_y_amp->Size = System::Drawing::Size(61, 27);
			this->YStextBox__lissajous_y_amp->TabIndex = 108;
			this->YStextBox__lissajous_y_amp->Text = L"0.5";
			this->YStextBox__lissajous_y_amp->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox__lissajous_x_amp
			// 
			this->YStextBox__lissajous_x_amp->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__lissajous_x_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__lissajous_x_amp->Location = System::Drawing::Point(256, 90);
			this->YStextBox__lissajous_x_amp->Name = L"YStextBox__lissajous_x_amp";
			this->YStextBox__lissajous_x_amp->Size = System::Drawing::Size(61, 27);
			this->YStextBox__lissajous_x_amp->TabIndex = 107;
			this->YStextBox__lissajous_x_amp->Text = L"0.5";
			this->YStextBox__lissajous_x_amp->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YSbutton__lissajous_setamp
			// 
			this->YSbutton__lissajous_setamp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YSbutton__lissajous_setamp->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton__lissajous_setamp->Location = System::Drawing::Point(323, 93);
			this->YSbutton__lissajous_setamp->Name = L"YSbutton__lissajous_setamp";
			this->YSbutton__lissajous_setamp->Size = System::Drawing::Size(64, 58);
			this->YSbutton__lissajous_setamp->TabIndex = 106;
			this->YSbutton__lissajous_setamp->Text = L"Set amp.";
			this->YSbutton__lissajous_setamp->UseVisualStyleBackColor = true;
			this->YSbutton__lissajous_setamp->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton__lissajous_setamp_Click);
			// 
			// YSlabel__lissajous_offsettime
			// 
			this->YSlabel__lissajous_offsettime->AutoSize = true;
			this->YSlabel__lissajous_offsettime->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSlabel__lissajous_offsettime->Location = System::Drawing::Point(268, 240);
			this->YSlabel__lissajous_offsettime->Name = L"YSlabel__lissajous_offsettime";
			this->YSlabel__lissajous_offsettime->Size = System::Drawing::Size(38, 20);
			this->YSlabel__lissajous_offsettime->TabIndex = 104;
			this->YSlabel__lissajous_offsettime->Text = L"0 µs";
			// 
			// YStrackBar__lissajous_offsettime
			// 
			this->YStrackBar__lissajous_offsettime->AutoSize = false;
			this->YStrackBar__lissajous_offsettime->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar__lissajous_offsettime->LargeChange = 1;
			this->YStrackBar__lissajous_offsettime->Location = System::Drawing::Point(103, 240);
			this->YStrackBar__lissajous_offsettime->Maximum = 500;
			this->YStrackBar__lissajous_offsettime->Name = L"YStrackBar__lissajous_offsettime";
			this->YStrackBar__lissajous_offsettime->Size = System::Drawing::Size(163, 30);
			this->YStrackBar__lissajous_offsettime->TabIndex = 102;
			this->YStrackBar__lissajous_offsettime->TickFrequency = 5;
			this->YStrackBar__lissajous_offsettime->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar1_Scroll);
			// 
			// label49
			// 
			this->label49->AutoSize = true;
			this->label49->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label49->ForeColor = System::Drawing::Color::White;
			this->label49->Location = System::Drawing::Point(14, 238);
			this->label49->Name = L"label49";
			this->label49->Size = System::Drawing::Size(89, 20);
			this->label49->TabIndex = 103;
			this->label49->Text = L"Offset time";
			// 
			// YScheckBox_lissajous_samplemapsave
			// 
			this->YScheckBox_lissajous_samplemapsave->AutoSize = true;
			this->YScheckBox_lissajous_samplemapsave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YScheckBox_lissajous_samplemapsave->Location = System::Drawing::Point(282, 60);
			this->YScheckBox_lissajous_samplemapsave->Name = L"YScheckBox_lissajous_samplemapsave";
			this->YScheckBox_lissajous_samplemapsave->Size = System::Drawing::Size(110, 24);
			this->YScheckBox_lissajous_samplemapsave->TabIndex = 101;
			this->YScheckBox_lissajous_samplemapsave->Text = L"SampleMap";
			this->YScheckBox_lissajous_samplemapsave->UseVisualStyleBackColor = true;
			// 
			// YSlabel__lissajous_phase
			// 
			this->YSlabel__lissajous_phase->AutoSize = true;
			this->YSlabel__lissajous_phase->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSlabel__lissajous_phase->Location = System::Drawing::Point(268, 204);
			this->YSlabel__lissajous_phase->Name = L"YSlabel__lissajous_phase";
			this->YSlabel__lissajous_phase->Size = System::Drawing::Size(18, 20);
			this->YSlabel__lissajous_phase->TabIndex = 100;
			this->YSlabel__lissajous_phase->Text = L"0";
			// 
			// YStrackBar__lissajous_phase
			// 
			this->YStrackBar__lissajous_phase->AutoSize = false;
			this->YStrackBar__lissajous_phase->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar__lissajous_phase->LargeChange = 1;
			this->YStrackBar__lissajous_phase->Location = System::Drawing::Point(103, 204);
			this->YStrackBar__lissajous_phase->Maximum = 7;
			this->YStrackBar__lissajous_phase->Name = L"YStrackBar__lissajous_phase";
			this->YStrackBar__lissajous_phase->Size = System::Drawing::Size(163, 30);
			this->YStrackBar__lissajous_phase->TabIndex = 98;
			this->YStrackBar__lissajous_phase->TickFrequency = 5;
			this->YStrackBar__lissajous_phase->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar__lissajous_phase_Scroll);
			// 
			// label47
			// 
			this->label47->AutoSize = true;
			this->label47->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label47->ForeColor = System::Drawing::Color::White;
			this->label47->Location = System::Drawing::Point(14, 202);
			this->label47->Name = L"label47";
			this->label47->Size = System::Drawing::Size(50, 20);
			this->label47->TabIndex = 99;
			this->label47->Text = L"Phase";
			// 
			// YStextBox__lissajous_y_freq
			// 
			this->YStextBox__lissajous_y_freq->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__lissajous_y_freq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__lissajous_y_freq->Location = System::Drawing::Point(216, 168);
			this->YStextBox__lissajous_y_freq->Name = L"YStextBox__lissajous_y_freq";
			this->YStextBox__lissajous_y_freq->Size = System::Drawing::Size(68, 27);
			this->YStextBox__lissajous_y_freq->TabIndex = 97;
			this->YStextBox__lissajous_y_freq->Text = L"2160";
			this->YStextBox__lissajous_y_freq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox__lissajous_x_freq
			// 
			this->YStextBox__lissajous_x_freq->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__lissajous_x_freq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__lissajous_x_freq->Location = System::Drawing::Point(77, 168);
			this->YStextBox__lissajous_x_freq->Name = L"YStextBox__lissajous_x_freq";
			this->YStextBox__lissajous_x_freq->Size = System::Drawing::Size(68, 27);
			this->YStextBox__lissajous_x_freq->TabIndex = 96;
			this->YStextBox__lissajous_x_freq->Text = L"2100";
			this->YStextBox__lissajous_x_freq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YSbutton_lissajous_startimg
			// 
			this->YSbutton_lissajous_startimg->BackColor = System::Drawing::Color::LightGray;
			this->YSbutton_lissajous_startimg->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->YSbutton_lissajous_startimg->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSbutton_lissajous_startimg->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->YSbutton_lissajous_startimg->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"YSbutton_lissajous_startimg.Image")));
			this->YSbutton_lissajous_startimg->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->YSbutton_lissajous_startimg->Location = System::Drawing::Point(127, 17);
			this->YSbutton_lissajous_startimg->Name = L"YSbutton_lissajous_startimg";
			this->YSbutton_lissajous_startimg->Size = System::Drawing::Size(169, 32);
			this->YSbutton_lissajous_startimg->TabIndex = 95;
			this->YSbutton_lissajous_startimg->Text = L"START Imaging";
			this->YSbutton_lissajous_startimg->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->YSbutton_lissajous_startimg->UseVisualStyleBackColor = false;
			this->YSbutton_lissajous_startimg->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_lissajous_startimg_Click);
			// 
			// YSbutton_lissajous_stop
			// 
			this->YSbutton_lissajous_stop->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YSbutton_lissajous_stop->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton_lissajous_stop->Location = System::Drawing::Point(307, 18);
			this->YSbutton_lissajous_stop->Name = L"YSbutton_lissajous_stop";
			this->YSbutton_lissajous_stop->Size = System::Drawing::Size(65, 30);
			this->YSbutton_lissajous_stop->TabIndex = 94;
			this->YSbutton_lissajous_stop->Text = L"Stop";
			this->YSbutton_lissajous_stop->UseVisualStyleBackColor = true;
			this->YSbutton_lissajous_stop->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_lissajous_stop_Click);
			// 
			// YSbutton_lissajous_configure
			// 
			this->YSbutton_lissajous_configure->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YSbutton_lissajous_configure->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->YSbutton_lissajous_configure->Location = System::Drawing::Point(18, 17);
			this->YSbutton_lissajous_configure->Name = L"YSbutton_lissajous_configure";
			this->YSbutton_lissajous_configure->Size = System::Drawing::Size(101, 31);
			this->YSbutton_lissajous_configure->TabIndex = 93;
			this->YSbutton_lissajous_configure->Text = L"Configure";
			this->YSbutton_lissajous_configure->UseVisualStyleBackColor = true;
			this->YSbutton_lissajous_configure->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_lissajous_configure_Click);
			// 
			// label33
			// 
			this->label33->AutoSize = true;
			this->label33->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label33->ForeColor = System::Drawing::Color::White;
			this->label33->Location = System::Drawing::Point(153, 168);
			this->label33->Name = L"label33";
			this->label33->Size = System::Drawing::Size(59, 20);
			this->label33->TabIndex = 84;
			this->label33->Text = L"Y-Freq.";
			// 
			// label34
			// 
			this->label34->AutoSize = true;
			this->label34->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label34->ForeColor = System::Drawing::Color::White;
			this->label34->Location = System::Drawing::Point(14, 168);
			this->label34->Name = L"label34";
			this->label34->Size = System::Drawing::Size(60, 20);
			this->label34->TabIndex = 82;
			this->label34->Text = L"X-Freq.";
			// 
			// YStextBox__lissajous_yoffset
			// 
			this->YStextBox__lissajous_yoffset->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__lissajous_yoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__lissajous_yoffset->Location = System::Drawing::Point(215, 58);
			this->YStextBox__lissajous_yoffset->Name = L"YStextBox__lissajous_yoffset";
			this->YStextBox__lissajous_yoffset->Size = System::Drawing::Size(61, 27);
			this->YStextBox__lissajous_yoffset->TabIndex = 80;
			this->YStextBox__lissajous_yoffset->Text = L"0";
			this->YStextBox__lissajous_yoffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox__lissajous_xoffset
			// 
			this->YStextBox__lissajous_xoffset->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__lissajous_xoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__lissajous_xoffset->Location = System::Drawing::Point(83, 58);
			this->YStextBox__lissajous_xoffset->Name = L"YStextBox__lissajous_xoffset";
			this->YStextBox__lissajous_xoffset->Size = System::Drawing::Size(61, 27);
			this->YStextBox__lissajous_xoffset->TabIndex = 79;
			this->YStextBox__lissajous_xoffset->Text = L"0";
			this->YStextBox__lissajous_xoffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label36
			// 
			this->label36->AutoSize = true;
			this->label36->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label36->ForeColor = System::Drawing::Color::White;
			this->label36->Location = System::Drawing::Point(149, 61);
			this->label36->Name = L"label36";
			this->label36->Size = System::Drawing::Size(62, 20);
			this->label36->TabIndex = 78;
			this->label36->Text = L"YOffset";
			// 
			// label37
			// 
			this->label37->AutoSize = true;
			this->label37->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label37->ForeColor = System::Drawing::Color::White;
			this->label37->Location = System::Drawing::Point(14, 61);
			this->label37->Name = L"label37";
			this->label37->Size = System::Drawing::Size(63, 20);
			this->label37->TabIndex = 77;
			this->label37->Text = L"XOffset";
			// 
			// YStrackBar__lissajous_y_amp
			// 
			this->YStrackBar__lissajous_y_amp->AutoSize = false;
			this->YStrackBar__lissajous_y_amp->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar__lissajous_y_amp->LargeChange = 1;
			this->YStrackBar__lissajous_y_amp->Location = System::Drawing::Point(91, 127);
			this->YStrackBar__lissajous_y_amp->Maximum = 150;
			this->YStrackBar__lissajous_y_amp->Name = L"YStrackBar__lissajous_y_amp";
			this->YStrackBar__lissajous_y_amp->Size = System::Drawing::Size(163, 30);
			this->YStrackBar__lissajous_y_amp->TabIndex = 10;
			this->YStrackBar__lissajous_y_amp->TickFrequency = 5;
			this->YStrackBar__lissajous_y_amp->Value = 50;
			this->YStrackBar__lissajous_y_amp->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar__lissajous_y_amp_Scroll);
			// 
			// label39
			// 
			this->label39->AutoSize = true;
			this->label39->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label39->ForeColor = System::Drawing::Color::White;
			this->label39->Location = System::Drawing::Point(14, 125);
			this->label39->Name = L"label39";
			this->label39->Size = System::Drawing::Size(56, 20);
			this->label39->TabIndex = 75;
			this->label39->Text = L"y amp.";
			// 
			// YStrackBar__lissajous_x_amp
			// 
			this->YStrackBar__lissajous_x_amp->AutoSize = false;
			this->YStrackBar__lissajous_x_amp->BackColor = System::Drawing::Color::DimGray;
			this->YStrackBar__lissajous_x_amp->LargeChange = 1;
			this->YStrackBar__lissajous_x_amp->Location = System::Drawing::Point(91, 93);
			this->YStrackBar__lissajous_x_amp->Maximum = 150;
			this->YStrackBar__lissajous_x_amp->Name = L"YStrackBar__lissajous_x_amp";
			this->YStrackBar__lissajous_x_amp->Size = System::Drawing::Size(163, 30);
			this->YStrackBar__lissajous_x_amp->TabIndex = 10;
			this->YStrackBar__lissajous_x_amp->TickFrequency = 5;
			this->YStrackBar__lissajous_x_amp->Value = 50;
			this->YStrackBar__lissajous_x_amp->Scroll += gcnew System::EventHandler(this, &MTPFM::YStrackBar__lissajous_x_amp_Scroll);
			// 
			// label40
			// 
			this->label40->AutoSize = true;
			this->label40->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label40->ForeColor = System::Drawing::Color::White;
			this->label40->Location = System::Drawing::Point(14, 94);
			this->label40->Name = L"label40";
			this->label40->Size = System::Drawing::Size(56, 20);
			this->label40->TabIndex = 73;
			this->label40->Text = L"x amp.";
			// 
			// tabPage17
			// 
			this->tabPage17->BackColor = System::Drawing::Color::DimGray;
			this->tabPage17->Controls->Add(this->YSbutton_manual_stopcontrol);
			this->tabPage17->Controls->Add(this->YStextBox_crnt_po_y);
			this->tabPage17->Controls->Add(this->YStextBox_crnt_po_x);
			this->tabPage17->Controls->Add(this->label41);
			this->tabPage17->Controls->Add(this->label42);
			this->tabPage17->Controls->Add(this->label43);
			this->tabPage17->Controls->Add(this->YStextBox_y_po);
			this->tabPage17->Controls->Add(this->YStextBox_x_po);
			this->tabPage17->Controls->Add(this->label44);
			this->tabPage17->Controls->Add(this->label45);
			this->tabPage17->Controls->Add(this->YSbutton_manual_control);
			this->tabPage17->Location = System::Drawing::Point(4, 24);
			this->tabPage17->Name = L"tabPage17";
			this->tabPage17->Size = System::Drawing::Size(390, 276);
			this->tabPage17->TabIndex = 2;
			this->tabPage17->Text = L"Alignment";
			// 
			// YSbutton_manual_stopcontrol
			// 
			this->YSbutton_manual_stopcontrol->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic))));
			this->YSbutton_manual_stopcontrol->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton_manual_stopcontrol->Location = System::Drawing::Point(269, 84);
			this->YSbutton_manual_stopcontrol->Name = L"YSbutton_manual_stopcontrol";
			this->YSbutton_manual_stopcontrol->Size = System::Drawing::Size(99, 39);
			this->YSbutton_manual_stopcontrol->TabIndex = 91;
			this->YSbutton_manual_stopcontrol->Text = L"Stop";
			this->YSbutton_manual_stopcontrol->UseVisualStyleBackColor = true;
			this->YSbutton_manual_stopcontrol->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_manual_stopcontrol_Click);
			// 
			// YStextBox_crnt_po_y
			// 
			this->YStextBox_crnt_po_y->BackColor = System::Drawing::Color::Silver;
			this->YStextBox_crnt_po_y->Enabled = false;
			this->YStextBox_crnt_po_y->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox_crnt_po_y->Location = System::Drawing::Point(268, 129);
			this->YStextBox_crnt_po_y->Name = L"YStextBox_crnt_po_y";
			this->YStextBox_crnt_po_y->Size = System::Drawing::Size(59, 27);
			this->YStextBox_crnt_po_y->TabIndex = 71;
			this->YStextBox_crnt_po_y->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox_crnt_po_x
			// 
			this->YStextBox_crnt_po_x->BackColor = System::Drawing::Color::Silver;
			this->YStextBox_crnt_po_x->Enabled = false;
			this->YStextBox_crnt_po_x->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox_crnt_po_x->Location = System::Drawing::Point(174, 129);
			this->YStextBox_crnt_po_x->Name = L"YStextBox_crnt_po_x";
			this->YStextBox_crnt_po_x->Size = System::Drawing::Size(59, 27);
			this->YStextBox_crnt_po_x->TabIndex = 70;
			this->YStextBox_crnt_po_x->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label41
			// 
			this->label41->AutoSize = true;
			this->label41->BackColor = System::Drawing::Color::Transparent;
			this->label41->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label41->Location = System::Drawing::Point(19, 132);
			this->label41->Name = L"label41";
			this->label41->Size = System::Drawing::Size(95, 20);
			this->label41->TabIndex = 63;
			this->label41->Text = L"Current pos.";
			// 
			// label42
			// 
			this->label42->AutoSize = true;
			this->label42->BackColor = System::Drawing::Color::Transparent;
			this->label42->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label42->Location = System::Drawing::Point(244, 133);
			this->label42->Name = L"label42";
			this->label42->Size = System::Drawing::Size(18, 20);
			this->label42->TabIndex = 69;
			this->label42->Text = L"Y";
			// 
			// label43
			// 
			this->label43->AutoSize = true;
			this->label43->BackColor = System::Drawing::Color::Transparent;
			this->label43->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label43->Location = System::Drawing::Point(145, 132);
			this->label43->Name = L"label43";
			this->label43->Size = System::Drawing::Size(23, 20);
			this->label43->TabIndex = 68;
			this->label43->Text = L"X ";
			// 
			// YStextBox_y_po
			// 
			this->YStextBox_y_po->BackColor = System::Drawing::Color::Silver;
			this->YStextBox_y_po->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox_y_po->Location = System::Drawing::Point(178, 50);
			this->YStextBox_y_po->Name = L"YStextBox_y_po";
			this->YStextBox_y_po->Size = System::Drawing::Size(100, 27);
			this->YStextBox_y_po->TabIndex = 67;
			this->YStextBox_y_po->Text = L"0";
			this->YStextBox_y_po->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// YStextBox_x_po
			// 
			this->YStextBox_x_po->BackColor = System::Drawing::Color::Silver;
			this->YStextBox_x_po->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox_x_po->Location = System::Drawing::Point(178, 16);
			this->YStextBox_x_po->Name = L"YStextBox_x_po";
			this->YStextBox_x_po->Size = System::Drawing::Size(100, 27);
			this->YStextBox_x_po->TabIndex = 64;
			this->YStextBox_x_po->Text = L"0";
			this->YStextBox_x_po->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label44
			// 
			this->label44->AutoSize = true;
			this->label44->BackColor = System::Drawing::Color::Transparent;
			this->label44->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label44->Location = System::Drawing::Point(19, 53);
			this->label44->Name = L"label44";
			this->label44->Size = System::Drawing::Size(148, 20);
			this->label44->TabIndex = 66;
			this->label44->Text = L"Y position (-10~10)";
			// 
			// label45
			// 
			this->label45->AutoSize = true;
			this->label45->BackColor = System::Drawing::Color::Transparent;
			this->label45->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label45->Location = System::Drawing::Point(19, 19);
			this->label45->Name = L"label45";
			this->label45->Size = System::Drawing::Size(149, 20);
			this->label45->TabIndex = 65;
			this->label45->Text = L"X position (-10~10)";
			// 
			// YSbutton_manual_control
			// 
			this->YSbutton_manual_control->BackColor = System::Drawing::Color::Gainsboro;
			this->YSbutton_manual_control->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YSbutton_manual_control->ForeColor = System::Drawing::Color::RoyalBlue;
			this->YSbutton_manual_control->Location = System::Drawing::Point(23, 84);
			this->YSbutton_manual_control->Name = L"YSbutton_manual_control";
			this->YSbutton_manual_control->Size = System::Drawing::Size(240, 39);
			this->YSbutton_manual_control->TabIndex = 62;
			this->YSbutton_manual_control->Text = L"Go to abs. position";
			this->YSbutton_manual_control->UseVisualStyleBackColor = false;
			this->YSbutton_manual_control->Click += gcnew System::EventHandler(this, &MTPFM::YSbutton_manual_control_Click);
			// 
			// label23
			// 
			this->label23->AutoSize = true;
			this->label23->BackColor = System::Drawing::Color::Transparent;
			this->label23->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.8F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->label23->Location = System::Drawing::Point(6, 714);
			this->label23->Name = L"label23";
			this->label23->Size = System::Drawing::Size(92, 20);
			this->label23->TabIndex = 74;
			this->label23->Text = L"ETL Control";
			// 
			// panel2
			// 
			this->panel2->BackColor = System::Drawing::Color::DimGray;
			this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->panel2->Controls->Add(this->button_ETL_downdown);
			this->panel2->Controls->Add(this->button_ETL_upup);
			this->panel2->Controls->Add(this->button_ETL_down);
			this->panel2->Controls->Add(this->textBox_ETL_FL);
			this->panel2->Controls->Add(this->textBox_ETL_voltage);
			this->panel2->Controls->Add(this->label27);
			this->panel2->Controls->Add(this->button_ETL_up);
			this->panel2->Controls->Add(this->label28);
			this->panel2->Location = System::Drawing::Point(6, 736);
			this->panel2->Name = L"panel2";
			this->panel2->Size = System::Drawing::Size(419, 97);
			this->panel2->TabIndex = 73;
			// 
			// button_ETL_downdown
			// 
			this->button_ETL_downdown->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_ETL_downdown->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_ETL_downdown->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_ETL_downdown.Image")));
			this->button_ETL_downdown->Location = System::Drawing::Point(102, 14);
			this->button_ETL_downdown->Name = L"button_ETL_downdown";
			this->button_ETL_downdown->Size = System::Drawing::Size(30, 30);
			this->button_ETL_downdown->TabIndex = 47;
			this->button_ETL_downdown->UseVisualStyleBackColor = true;
			this->button_ETL_downdown->Click += gcnew System::EventHandler(this, &MTPFM::button_ETL_downdown_Click);
			// 
			// button_ETL_upup
			// 
			this->button_ETL_upup->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_ETL_upup->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_ETL_upup->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_ETL_upup.Image")));
			this->button_ETL_upup->Location = System::Drawing::Point(332, 14);
			this->button_ETL_upup->Name = L"button_ETL_upup";
			this->button_ETL_upup->Size = System::Drawing::Size(30, 30);
			this->button_ETL_upup->TabIndex = 46;
			this->button_ETL_upup->UseVisualStyleBackColor = true;
			this->button_ETL_upup->Click += gcnew System::EventHandler(this, &MTPFM::button_ETL_upup_Click);
			// 
			// button_ETL_down
			// 
			this->button_ETL_down->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_ETL_down->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_ETL_down->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_ETL_down.Image")));
			this->button_ETL_down->Location = System::Drawing::Point(142, 14);
			this->button_ETL_down->Name = L"button_ETL_down";
			this->button_ETL_down->Size = System::Drawing::Size(30, 30);
			this->button_ETL_down->TabIndex = 45;
			this->button_ETL_down->UseVisualStyleBackColor = true;
			this->button_ETL_down->Click += gcnew System::EventHandler(this, &MTPFM::button_ETL_down_Click);
			// 
			// textBox_ETL_FL
			// 
			this->textBox_ETL_FL->BackColor = System::Drawing::Color::Silver;
			this->textBox_ETL_FL->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_ETL_FL->Location = System::Drawing::Point(146, 58);
			this->textBox_ETL_FL->Name = L"textBox_ETL_FL";
			this->textBox_ETL_FL->Size = System::Drawing::Size(100, 27);
			this->textBox_ETL_FL->TabIndex = 44;
			this->textBox_ETL_FL->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_ETL_voltage
			// 
			this->textBox_ETL_voltage->BackColor = System::Drawing::Color::Silver;
			this->textBox_ETL_voltage->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_ETL_voltage->ForeColor = System::Drawing::Color::Black;
			this->textBox_ETL_voltage->Location = System::Drawing::Point(182, 16);
			this->textBox_ETL_voltage->Multiline = true;
			this->textBox_ETL_voltage->Name = L"textBox_ETL_voltage";
			this->textBox_ETL_voltage->ReadOnly = true;
			this->textBox_ETL_voltage->Size = System::Drawing::Size(100, 26);
			this->textBox_ETL_voltage->TabIndex = 36;
			this->textBox_ETL_voltage->Text = L"47";
			this->textBox_ETL_voltage->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label27
			// 
			this->label27->AutoSize = true;
			this->label27->BackColor = System::Drawing::Color::Transparent;
			this->label27->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label27->Location = System::Drawing::Point(6, 17);
			this->label27->Name = L"label27";
			this->label27->Size = System::Drawing::Size(88, 20);
			this->label27->TabIndex = 38;
			this->label27->Text = L"Voltage (V)";
			// 
			// button_ETL_up
			// 
			this->button_ETL_up->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_ETL_up->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_ETL_up->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_ETL_up.Image")));
			this->button_ETL_up->Location = System::Drawing::Point(292, 14);
			this->button_ETL_up->Name = L"button_ETL_up";
			this->button_ETL_up->Size = System::Drawing::Size(30, 30);
			this->button_ETL_up->TabIndex = 41;
			this->button_ETL_up->UseVisualStyleBackColor = true;
			this->button_ETL_up->Click += gcnew System::EventHandler(this, &MTPFM::button_ETL_up_Click);
			// 
			// label28
			// 
			this->label28->AutoSize = true;
			this->label28->BackColor = System::Drawing::Color::Transparent;
			this->label28->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label28->Location = System::Drawing::Point(6, 61);
			this->label28->Name = L"label28";
			this->label28->Size = System::Drawing::Size(133, 20);
			this->label28->TabIndex = 36;
			this->label28->Text = L"Focal length (μm)";
			// 
			// button_clear_terminal
			// 
			this->button_clear_terminal->BackColor = System::Drawing::Color::Chocolate;
			this->button_clear_terminal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button_clear_terminal->Location = System::Drawing::Point(140, 104);
			this->button_clear_terminal->Name = L"button_clear_terminal";
			this->button_clear_terminal->Size = System::Drawing::Size(151, 28);
			this->button_clear_terminal->TabIndex = 45;
			this->button_clear_terminal->Text = L"CLEAR TERMINAL";
			this->button_clear_terminal->UseVisualStyleBackColor = false;
			this->button_clear_terminal->Click += gcnew System::EventHandler(this, &MTPFM::button_clear_terminal_Click);
			// 
			// label2_combo_y_length
			// 
			this->label2_combo_y_length->AutoSize = true;
			this->label2_combo_y_length->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label2_combo_y_length->ForeColor = System::Drawing::Color::White;
			this->label2_combo_y_length->Location = System::Drawing::Point(11, 77);
			this->label2_combo_y_length->Name = L"label2_combo_y_length";
			this->label2_combo_y_length->Size = System::Drawing::Size(66, 20);
			this->label2_combo_y_length->TabIndex = 36;
			this->label2_combo_y_length->Text = L"# Y-Line";
			// 
			// comboBox2__Ext_clk
			// 
			this->comboBox2__Ext_clk->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->comboBox2__Ext_clk->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->comboBox2__Ext_clk->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->comboBox2__Ext_clk->FormattingEnabled = true;
			this->comboBox2__Ext_clk->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"75.9 MS/s", L"76.0 MS/s", L"76.1 MS/s",
					L"76.2 MS/s", L"76.3 MS/s", L"76.4 MS/s"
			});
			this->comboBox2__Ext_clk->Location = System::Drawing::Point(106, 10);
			this->comboBox2__Ext_clk->Name = L"comboBox2__Ext_clk";
			this->comboBox2__Ext_clk->Size = System::Drawing::Size(84, 25);
			this->comboBox2__Ext_clk->TabIndex = 37;
			this->comboBox2__Ext_clk->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::comboBox2__Ext_clk_SelectedIndexChanged_1);
			// 
			// comboBox3_Int_clk
			// 
			this->comboBox3_Int_clk->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->comboBox3_Int_clk->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->comboBox3_Int_clk->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->comboBox3_Int_clk->FormattingEnabled = true;
			this->comboBox3_Int_clk->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"2 MS/s", L"10 MS/s", L"20 MS/s", L"50 MS/s",
					L"100 MS/s", L"125 MS/s"
			});
			this->comboBox3_Int_clk->Location = System::Drawing::Point(298, 10);
			this->comboBox3_Int_clk->Name = L"comboBox3_Int_clk";
			this->comboBox3_Int_clk->Size = System::Drawing::Size(84, 25);
			this->comboBox3_Int_clk->TabIndex = 38;
			this->comboBox3_Int_clk->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::comboBox3_Int_clk_SelectedIndexChanged_1);
			// 
			// comboBox_num_Y_lines
			// 
			this->comboBox_num_Y_lines->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->comboBox_num_Y_lines->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->comboBox_num_Y_lines->FormattingEnabled = true;
			this->comboBox_num_Y_lines->Items->AddRange(gcnew cli::array< System::Object^  >(8) {
				L"8", L"16", L"32", L"64", L"128", L"256",
					L"512", L"1024"
			});
			this->comboBox_num_Y_lines->Location = System::Drawing::Point(84, 73);
			this->comboBox_num_Y_lines->Name = L"comboBox_num_Y_lines";
			this->comboBox_num_Y_lines->Size = System::Drawing::Size(73, 28);
			this->comboBox_num_Y_lines->TabIndex = 39;
			this->comboBox_num_Y_lines->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::comboBox_num_Y_lines_SelectedIndexChanged_1);
			// 
			// comboBox_headmount_count
			// 
			this->comboBox_headmount_count->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->comboBox_headmount_count->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->comboBox_headmount_count->FormattingEnabled = true;
			this->comboBox_headmount_count->Items->AddRange(gcnew cli::array< System::Object^  >(9) {
				L"0", L"1", L"2", L"3", L"4", L"5",
					L"6", L"7", L"8"
			});
			this->comboBox_headmount_count->Location = System::Drawing::Point(310, 73);
			this->comboBox_headmount_count->Name = L"comboBox_headmount_count";
			this->comboBox_headmount_count->Size = System::Drawing::Size(73, 28);
			this->comboBox_headmount_count->TabIndex = 40;
			this->comboBox_headmount_count->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::comboBox_headmount_count_SelectedIndexChanged);
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label11->ForeColor = System::Drawing::Color::White;
			this->label11->Location = System::Drawing::Point(184, 77);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(122, 20);
			this->label11->TabIndex = 41;
			this->label11->Text = L"Head Mount Set";
			// 
			// timer_refresh_UI
			// 
			this->timer_refresh_UI->Tick += gcnew System::EventHandler(this, &MTPFM::timer_refresh_UI_Tick);
			// 
			// tabControl5
			// 
			this->tabControl5->Controls->Add(this->tabPage18);
			this->tabControl5->Controls->Add(this->tabPage14);
			this->tabControl5->Location = System::Drawing::Point(6, 77);
			this->tabControl5->Name = L"tabControl5";
			this->tabControl5->SelectedIndex = 0;
			this->tabControl5->Size = System::Drawing::Size(419, 424);
			this->tabControl5->TabIndex = 69;
			// 
			// tabPage18
			// 
			this->tabPage18->BackColor = System::Drawing::Color::DimGray;
			this->tabPage18->Controls->Add(this->label57);
			this->tabPage18->Controls->Add(this->label50);
			this->tabPage18->Controls->Add(this->YScomboBox_inputrange);
			this->tabPage18->Controls->Add(this->YStextBox__ExtClockFreq);
			this->tabPage18->Controls->Add(this->label30);
			this->tabPage18->Controls->Add(this->YScomboBox_headmount);
			this->tabPage18->Controls->Add(this->YScomboBox_intclock);
			this->tabPage18->Controls->Add(this->YScheckBox_extclock);
			this->tabPage18->Controls->Add(this->YScheckBox_intclock);
			this->tabPage18->Controls->Add(this->YSbutton_lissajous_setamp);
			this->tabPage18->Location = System::Drawing::Point(4, 24);
			this->tabPage18->Name = L"tabPage18";
			this->tabPage18->Padding = System::Windows::Forms::Padding(3);
			this->tabPage18->Size = System::Drawing::Size(411, 396);
			this->tabPage18->TabIndex = 1;
			this->tabPage18->Text = L"Driver";
			// 
			// label57
			// 
			this->label57->AutoSize = true;
			this->label57->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label57->ForeColor = System::Drawing::Color::White;
			this->label57->Location = System::Drawing::Point(5, 51);
			this->label57->Name = L"label57";
			this->label57->Size = System::Drawing::Size(95, 20);
			this->label57->TabIndex = 109;
			this->label57->Text = L"Input Range";
			// 
			// label50
			// 
			this->label50->AutoSize = true;
			this->label50->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label50->ForeColor = System::Drawing::Color::White;
			this->label50->Location = System::Drawing::Point(160, 18);
			this->label50->Name = L"label50";
			this->label50->Size = System::Drawing::Size(45, 20);
			this->label50->TabIndex = 107;
			this->label50->Text = L"MS/s";
			// 
			// YScomboBox_inputrange
			// 
			this->YScomboBox_inputrange->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->YScomboBox_inputrange->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->YScomboBox_inputrange->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScomboBox_inputrange->FormattingEnabled = true;
			this->YScomboBox_inputrange->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"500 mV", L"800 mV", L"1000 mV" });
			this->YScomboBox_inputrange->Location = System::Drawing::Point(103, 49);
			this->YScomboBox_inputrange->Name = L"YScomboBox_inputrange";
			this->YScomboBox_inputrange->Size = System::Drawing::Size(73, 25);
			this->YScomboBox_inputrange->TabIndex = 108;
			// 
			// YStextBox__ExtClockFreq
			// 
			this->YStextBox__ExtClockFreq->BackColor = System::Drawing::Color::Silver;
			this->YStextBox__ExtClockFreq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YStextBox__ExtClockFreq->Location = System::Drawing::Point(98, 15);
			this->YStextBox__ExtClockFreq->Name = L"YStextBox__ExtClockFreq";
			this->YStextBox__ExtClockFreq->Size = System::Drawing::Size(61, 27);
			this->YStextBox__ExtClockFreq->TabIndex = 106;
			this->YStextBox__ExtClockFreq->Text = L"80.225";
			this->YStextBox__ExtClockFreq->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label30
			// 
			this->label30->AutoSize = true;
			this->label30->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label30->ForeColor = System::Drawing::Color::White;
			this->label30->Location = System::Drawing::Point(190, 54);
			this->label30->Name = L"label30";
			this->label30->Size = System::Drawing::Size(122, 20);
			this->label30->TabIndex = 105;
			this->label30->Text = L"Head Mount Set";
			// 
			// YScomboBox_headmount
			// 
			this->YScomboBox_headmount->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->YScomboBox_headmount->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->YScomboBox_headmount->FormattingEnabled = true;
			this->YScomboBox_headmount->Items->AddRange(gcnew cli::array< System::Object^  >(9) {
				L"0", L"1", L"2", L"3", L"4", L"5",
					L"6", L"7", L"8"
			});
			this->YScomboBox_headmount->Location = System::Drawing::Point(316, 50);
			this->YScomboBox_headmount->Name = L"YScomboBox_headmount";
			this->YScomboBox_headmount->Size = System::Drawing::Size(73, 28);
			this->YScomboBox_headmount->TabIndex = 104;
			this->YScomboBox_headmount->SelectedIndexChanged += gcnew System::EventHandler(this, &MTPFM::YScomboBox_headmount_SelectedIndexChanged);
			// 
			// tabPage14
			// 
			this->tabPage14->BackColor = System::Drawing::Color::DimGray;
			this->tabPage14->Controls->Add(this->tabControl2);
			this->tabPage14->Controls->Add(this->checkBox2__Int_clk);
			this->tabPage14->Controls->Add(this->label2_combo_y_length);
			this->tabPage14->Controls->Add(this->checkBox1_Ext_clk);
			this->tabPage14->Controls->Add(this->comboBox_num_Y_lines);
			this->tabPage14->Controls->Add(this->button_clear_terminal);
			this->tabPage14->Controls->Add(this->checkBox_biDirection);
			this->tabPage14->Controls->Add(this->comboBox3_Int_clk);
			this->tabPage14->Controls->Add(this->label1_Main_Control_Panel);
			this->tabPage14->Controls->Add(this->comboBox2__Ext_clk);
			this->tabPage14->Controls->Add(this->label11);
			this->tabPage14->Controls->Add(this->checkBox_uniDirection);
			this->tabPage14->Controls->Add(this->comboBox_headmount_count);
			this->tabPage14->Controls->Add(this->Configure);
			this->tabPage14->Location = System::Drawing::Point(4, 24);
			this->tabPage14->Name = L"tabPage14";
			this->tabPage14->Padding = System::Windows::Forms::Padding(3);
			this->tabPage14->Size = System::Drawing::Size(411, 396);
			this->tabPage14->TabIndex = 0;
			this->tabPage14->Text = L"Controller";
			// 
			// tabControl2
			// 
			this->tabControl2->Controls->Add(this->tabPage3);
			this->tabControl2->Controls->Add(this->tabPage4);
			this->tabControl2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->tabControl2->Location = System::Drawing::Point(4, 138);
			this->tabControl2->Name = L"tabControl2";
			this->tabControl2->SelectedIndex = 0;
			this->tabControl2->Size = System::Drawing::Size(402, 250);
			this->tabControl2->TabIndex = 29;
			// 
			// tabPage3
			// 
			this->tabPage3->BackColor = System::Drawing::Color::DimGray;
			this->tabPage3->Controls->Add(this->button_start_scan);
			this->tabPage3->Controls->Add(this->textBox_yoffset);
			this->tabPage3->Controls->Add(this->textBox_xoffset);
			this->tabPage3->Controls->Add(this->textBox_line_time);
			this->tabPage3->Controls->Add(this->label1_yoffset);
			this->tabPage3->Controls->Add(this->label_xoffset);
			this->tabPage3->Controls->Add(this->label1_sec);
			this->tabPage3->Controls->Add(this->button_acq);
			this->tabPage3->Controls->Add(this->label10);
			this->tabPage3->Controls->Add(this->label_bi_direction_offset);
			this->tabPage3->Controls->Add(this->button1_connect_to_controller);
			this->tabPage3->Controls->Add(this->trackBar_phase);
			this->tabPage3->Controls->Add(this->label7_raster_y_amp);
			this->tabPage3->Controls->Add(this->label_phase);
			this->tabPage3->Controls->Add(this->trackBar3_raster_y_amp);
			this->tabPage3->Controls->Add(this->label6_raster_y_amp);
			this->tabPage3->Controls->Add(this->button_Stop);
			this->tabPage3->Controls->Add(this->trackBar2__raster_x_amp);
			this->tabPage3->Controls->Add(this->label4__raster_x_amp);
			this->tabPage3->Controls->Add(this->label3_raster_x_amp);
			this->tabPage3->Location = System::Drawing::Point(4, 24);
			this->tabPage3->Name = L"tabPage3";
			this->tabPage3->Padding = System::Windows::Forms::Padding(3);
			this->tabPage3->Size = System::Drawing::Size(394, 222);
			this->tabPage3->TabIndex = 0;
			this->tabPage3->Text = L"Scan Mode";
			// 
			// button_start_scan
			// 
			this->button_start_scan->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_start_scan->ForeColor = System::Drawing::Color::RoyalBlue;
			this->button_start_scan->Location = System::Drawing::Point(203, 13);
			this->button_start_scan->Name = L"button_start_scan";
			this->button_start_scan->Size = System::Drawing::Size(118, 30);
			this->button_start_scan->TabIndex = 58;
			this->button_start_scan->Text = L"Start scan";
			this->button_start_scan->UseVisualStyleBackColor = true;
			this->button_start_scan->Click += gcnew System::EventHandler(this, &MTPFM::button_start_scan_Click);
			// 
			// textBox_yoffset
			// 
			this->textBox_yoffset->BackColor = System::Drawing::Color::Silver;
			this->textBox_yoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_yoffset->Location = System::Drawing::Point(237, 55);
			this->textBox_yoffset->Name = L"textBox_yoffset";
			this->textBox_yoffset->Size = System::Drawing::Size(82, 27);
			this->textBox_yoffset->TabIndex = 57;
			this->textBox_yoffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_xoffset
			// 
			this->textBox_xoffset->BackColor = System::Drawing::Color::Silver;
			this->textBox_xoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_xoffset->Location = System::Drawing::Point(72, 55);
			this->textBox_xoffset->Name = L"textBox_xoffset";
			this->textBox_xoffset->Size = System::Drawing::Size(82, 27);
			this->textBox_xoffset->TabIndex = 56;
			this->textBox_xoffset->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_line_time
			// 
			this->textBox_line_time->BackColor = System::Drawing::Color::Silver;
			this->textBox_line_time->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_line_time->Location = System::Drawing::Point(86, 190);
			this->textBox_line_time->Name = L"textBox_line_time";
			this->textBox_line_time->Size = System::Drawing::Size(92, 27);
			this->textBox_line_time->TabIndex = 52;
			this->textBox_line_time->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label1_yoffset
			// 
			this->label1_yoffset->AutoSize = true;
			this->label1_yoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label1_yoffset->ForeColor = System::Drawing::Color::White;
			this->label1_yoffset->Location = System::Drawing::Point(171, 58);
			this->label1_yoffset->Name = L"label1_yoffset";
			this->label1_yoffset->Size = System::Drawing::Size(62, 20);
			this->label1_yoffset->TabIndex = 55;
			this->label1_yoffset->Text = L"YOffset";
			// 
			// label_xoffset
			// 
			this->label_xoffset->AutoSize = true;
			this->label_xoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_xoffset->ForeColor = System::Drawing::Color::White;
			this->label_xoffset->Location = System::Drawing::Point(3, 58);
			this->label_xoffset->Name = L"label_xoffset";
			this->label_xoffset->Size = System::Drawing::Size(63, 20);
			this->label_xoffset->TabIndex = 54;
			this->label_xoffset->Text = L"XOffset";
			// 
			// label1_sec
			// 
			this->label1_sec->AutoSize = true;
			this->label1_sec->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label1_sec->Location = System::Drawing::Point(184, 193);
			this->label1_sec->Name = L"label1_sec";
			this->label1_sec->Size = System::Drawing::Size(31, 20);
			this->label1_sec->TabIndex = 53;
			this->label1_sec->Text = L"sec";
			// 
			// button_acq
			// 
			this->button_acq->BackColor = System::Drawing::Color::LightGray;
			this->button_acq->FlatStyle = System::Windows::Forms::FlatStyle::Popup;
			this->button_acq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button_acq->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_acq->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_acq.Image")));
			this->button_acq->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->button_acq->Location = System::Drawing::Point(223, 187);
			this->button_acq->Name = L"button_acq";
			this->button_acq->Size = System::Drawing::Size(164, 32);
			this->button_acq->TabIndex = 14;
			this->button_acq->Text = L"START Imaging";
			this->button_acq->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->button_acq->UseVisualStyleBackColor = false;
			this->button_acq->Click += gcnew System::EventHandler(this, &MTPFM::button_acq_Click);
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label10->ForeColor = System::Drawing::Color::White;
			this->label10->Location = System::Drawing::Point(3, 190);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(77, 20);
			this->label10->TabIndex = 40;
			this->label10->Text = L"Line Time";
			// 
			// label_bi_direction_offset
			// 
			this->label_bi_direction_offset->AutoSize = true;
			this->label_bi_direction_offset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_bi_direction_offset->Location = System::Drawing::Point(243, 157);
			this->label_bi_direction_offset->Name = L"label_bi_direction_offset";
			this->label_bi_direction_offset->Size = System::Drawing::Size(81, 20);
			this->label_bi_direction_offset->TabIndex = 39;
			this->label_bi_direction_offset->Text = L"Offset = 1";
			// 
			// button1_connect_to_controller
			// 
			this->button1_connect_to_controller->BackColor = System::Drawing::Color::LightGray;
			this->button1_connect_to_controller->FlatAppearance->BorderColor = System::Drawing::Color::DimGray;
			this->button1_connect_to_controller->FlatAppearance->MouseDownBackColor = System::Drawing::Color::Silver;
			this->button1_connect_to_controller->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(224)),
				static_cast<System::Int32>(static_cast<System::Byte>(224)), static_cast<System::Int32>(static_cast<System::Byte>(224)));
			this->button1_connect_to_controller->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button1_connect_to_controller->ForeColor = System::Drawing::Color::RoyalBlue;
			this->button1_connect_to_controller->Location = System::Drawing::Point(8, 12);
			this->button1_connect_to_controller->Name = L"button1_connect_to_controller";
			this->button1_connect_to_controller->Size = System::Drawing::Size(192, 32);
			this->button1_connect_to_controller->TabIndex = 0;
			this->button1_connect_to_controller->Text = L"Connect to Controller";
			this->button1_connect_to_controller->UseVisualStyleBackColor = false;
			this->button1_connect_to_controller->Click += gcnew System::EventHandler(this, &MTPFM::button1_connect_to_controller_Click);
			// 
			// trackBar_phase
			// 
			this->trackBar_phase->AutoSize = false;
			this->trackBar_phase->BackColor = System::Drawing::Color::DimGray;
			this->trackBar_phase->LargeChange = 0;
			this->trackBar_phase->Location = System::Drawing::Point(80, 157);
			this->trackBar_phase->Maximum = 5000;
			this->trackBar_phase->Minimum = 1;
			this->trackBar_phase->Name = L"trackBar_phase";
			this->trackBar_phase->Size = System::Drawing::Size(163, 30);
			this->trackBar_phase->TabIndex = 38;
			this->trackBar_phase->TickFrequency = 200;
			this->trackBar_phase->Value = 1;
			this->trackBar_phase->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar_phase_Scroll);
			// 
			// label7_raster_y_amp
			// 
			this->label7_raster_y_amp->AutoSize = true;
			this->label7_raster_y_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label7_raster_y_amp->Location = System::Drawing::Point(243, 124);
			this->label7_raster_y_amp->Name = L"label7_raster_y_amp";
			this->label7_raster_y_amp->Size = System::Drawing::Size(40, 20);
			this->label7_raster_y_amp->TabIndex = 31;
			this->label7_raster_y_amp->Text = L"0.10";
			// 
			// label_phase
			// 
			this->label_phase->AutoSize = true;
			this->label_phase->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_phase->ForeColor = System::Drawing::Color::White;
			this->label_phase->Location = System::Drawing::Point(3, 153);
			this->label_phase->Name = L"label_phase";
			this->label_phase->Size = System::Drawing::Size(50, 20);
			this->label_phase->TabIndex = 37;
			this->label_phase->Text = L"Phase";
			// 
			// trackBar3_raster_y_amp
			// 
			this->trackBar3_raster_y_amp->AutoSize = false;
			this->trackBar3_raster_y_amp->BackColor = System::Drawing::Color::DimGray;
			this->trackBar3_raster_y_amp->LargeChange = 1;
			this->trackBar3_raster_y_amp->Location = System::Drawing::Point(80, 124);
			this->trackBar3_raster_y_amp->Maximum = 50;
			this->trackBar3_raster_y_amp->Minimum = 1;
			this->trackBar3_raster_y_amp->Name = L"trackBar3_raster_y_amp";
			this->trackBar3_raster_y_amp->Size = System::Drawing::Size(163, 30);
			this->trackBar3_raster_y_amp->TabIndex = 29;
			this->trackBar3_raster_y_amp->TickFrequency = 5;
			this->trackBar3_raster_y_amp->Value = 10;
			this->trackBar3_raster_y_amp->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar3_raster_y_amp_Scroll);
			// 
			// label6_raster_y_amp
			// 
			this->label6_raster_y_amp->AutoSize = true;
			this->label6_raster_y_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label6_raster_y_amp->ForeColor = System::Drawing::Color::White;
			this->label6_raster_y_amp->Location = System::Drawing::Point(3, 122);
			this->label6_raster_y_amp->Name = L"label6_raster_y_amp";
			this->label6_raster_y_amp->Size = System::Drawing::Size(56, 20);
			this->label6_raster_y_amp->TabIndex = 30;
			this->label6_raster_y_amp->Text = L"y amp.";
			// 
			// button_Stop
			// 
			this->button_Stop->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->button_Stop->ForeColor = System::Drawing::Color::RoyalBlue;
			this->button_Stop->Location = System::Drawing::Point(322, 13);
			this->button_Stop->Name = L"button_Stop";
			this->button_Stop->Size = System::Drawing::Size(65, 30);
			this->button_Stop->TabIndex = 16;
			this->button_Stop->Text = L"Stop";
			this->button_Stop->UseVisualStyleBackColor = true;
			this->button_Stop->Click += gcnew System::EventHandler(this, &MTPFM::button_Stop_Click);
			// 
			// trackBar2__raster_x_amp
			// 
			this->trackBar2__raster_x_amp->AutoSize = false;
			this->trackBar2__raster_x_amp->BackColor = System::Drawing::Color::DimGray;
			this->trackBar2__raster_x_amp->LargeChange = 1;
			this->trackBar2__raster_x_amp->Location = System::Drawing::Point(80, 90);
			this->trackBar2__raster_x_amp->Maximum = 50;
			this->trackBar2__raster_x_amp->Minimum = 1;
			this->trackBar2__raster_x_amp->Name = L"trackBar2__raster_x_amp";
			this->trackBar2__raster_x_amp->Size = System::Drawing::Size(163, 30);
			this->trackBar2__raster_x_amp->TabIndex = 26;
			this->trackBar2__raster_x_amp->TickFrequency = 5;
			this->trackBar2__raster_x_amp->Value = 10;
			this->trackBar2__raster_x_amp->Scroll += gcnew System::EventHandler(this, &MTPFM::trackBar2__raster_x_amp_Scroll);
			// 
			// label4__raster_x_amp
			// 
			this->label4__raster_x_amp->AutoSize = true;
			this->label4__raster_x_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label4__raster_x_amp->Location = System::Drawing::Point(242, 90);
			this->label4__raster_x_amp->Name = L"label4__raster_x_amp";
			this->label4__raster_x_amp->Size = System::Drawing::Size(41, 21);
			this->label4__raster_x_amp->TabIndex = 28;
			this->label4__raster_x_amp->Text = L"0.10";
			// 
			// label3_raster_x_amp
			// 
			this->label3_raster_x_amp->AutoSize = true;
			this->label3_raster_x_amp->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label3_raster_x_amp->ForeColor = System::Drawing::Color::White;
			this->label3_raster_x_amp->Location = System::Drawing::Point(3, 91);
			this->label3_raster_x_amp->Name = L"label3_raster_x_amp";
			this->label3_raster_x_amp->Size = System::Drawing::Size(56, 20);
			this->label3_raster_x_amp->TabIndex = 27;
			this->label3_raster_x_amp->Text = L"x amp.";
			// 
			// tabPage4
			// 
			this->tabPage4->BackColor = System::Drawing::Color::DimGray;
			this->tabPage4->Controls->Add(this->textBox_crnt_po_y);
			this->tabPage4->Controls->Add(this->textBox_crnt_po_x);
			this->tabPage4->Controls->Add(this->textBox_movingtime);
			this->tabPage4->Controls->Add(this->textBox_y_pos);
			this->tabPage4->Controls->Add(this->textBox_x_po);
			this->tabPage4->Controls->Add(this->label8);
			this->tabPage4->Controls->Add(this->label_Y_current);
			this->tabPage4->Controls->Add(this->label_x_current);
			this->tabPage4->Controls->Add(this->button_back_origin);
			this->tabPage4->Controls->Add(this->label_msec);
			this->tabPage4->Controls->Add(this->label_y_posi);
			this->tabPage4->Controls->Add(this->label_x_posi);
			this->tabPage4->Controls->Add(this->button1_manual_control);
			this->tabPage4->Location = System::Drawing::Point(4, 24);
			this->tabPage4->Name = L"tabPage4";
			this->tabPage4->Padding = System::Windows::Forms::Padding(3);
			this->tabPage4->Size = System::Drawing::Size(394, 222);
			this->tabPage4->TabIndex = 1;
			this->tabPage4->Text = L"Alignment Mode";
			// 
			// textBox_crnt_po_y
			// 
			this->textBox_crnt_po_y->BackColor = System::Drawing::Color::Silver;
			this->textBox_crnt_po_y->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_crnt_po_y->Location = System::Drawing::Point(266, 176);
			this->textBox_crnt_po_y->Name = L"textBox_crnt_po_y";
			this->textBox_crnt_po_y->Size = System::Drawing::Size(59, 27);
			this->textBox_crnt_po_y->TabIndex = 61;
			this->textBox_crnt_po_y->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_crnt_po_x
			// 
			this->textBox_crnt_po_x->BackColor = System::Drawing::Color::Silver;
			this->textBox_crnt_po_x->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_crnt_po_x->Location = System::Drawing::Point(172, 176);
			this->textBox_crnt_po_x->Name = L"textBox_crnt_po_x";
			this->textBox_crnt_po_x->Size = System::Drawing::Size(59, 27);
			this->textBox_crnt_po_x->TabIndex = 60;
			this->textBox_crnt_po_x->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_movingtime
			// 
			this->textBox_movingtime->BackColor = System::Drawing::Color::Silver;
			this->textBox_movingtime->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_movingtime->Location = System::Drawing::Point(159, 82);
			this->textBox_movingtime->Name = L"textBox_movingtime";
			this->textBox_movingtime->Size = System::Drawing::Size(100, 27);
			this->textBox_movingtime->TabIndex = 56;
			this->textBox_movingtime->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_y_pos
			// 
			this->textBox_y_pos->BackColor = System::Drawing::Color::Silver;
			this->textBox_y_pos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_y_pos->Location = System::Drawing::Point(159, 48);
			this->textBox_y_pos->Name = L"textBox_y_pos";
			this->textBox_y_pos->Size = System::Drawing::Size(100, 27);
			this->textBox_y_pos->TabIndex = 55;
			this->textBox_y_pos->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_x_po
			// 
			this->textBox_x_po->BackColor = System::Drawing::Color::Silver;
			this->textBox_x_po->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->textBox_x_po->Location = System::Drawing::Point(159, 14);
			this->textBox_x_po->Name = L"textBox_x_po";
			this->textBox_x_po->Size = System::Drawing::Size(100, 27);
			this->textBox_x_po->TabIndex = 52;
			this->textBox_x_po->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->BackColor = System::Drawing::Color::Transparent;
			this->label8->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label8->Location = System::Drawing::Point(17, 179);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(95, 20);
			this->label8->TabIndex = 52;
			this->label8->Text = L"Current pos.";
			// 
			// label_Y_current
			// 
			this->label_Y_current->AutoSize = true;
			this->label_Y_current->BackColor = System::Drawing::Color::Transparent;
			this->label_Y_current->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_Y_current->Location = System::Drawing::Point(242, 180);
			this->label_Y_current->Name = L"label_Y_current";
			this->label_Y_current->Size = System::Drawing::Size(18, 20);
			this->label_Y_current->TabIndex = 59;
			this->label_Y_current->Text = L"Y";
			// 
			// label_x_current
			// 
			this->label_x_current->AutoSize = true;
			this->label_x_current->BackColor = System::Drawing::Color::Transparent;
			this->label_x_current->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_x_current->Location = System::Drawing::Point(143, 179);
			this->label_x_current->Name = L"label_x_current";
			this->label_x_current->Size = System::Drawing::Size(23, 20);
			this->label_x_current->TabIndex = 58;
			this->label_x_current->Text = L"X ";
			// 
			// button_back_origin
			// 
			this->button_back_origin->BackColor = System::Drawing::Color::Gainsboro;
			this->button_back_origin->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button_back_origin->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_back_origin->Location = System::Drawing::Point(283, 32);
			this->button_back_origin->Name = L"button_back_origin";
			this->button_back_origin->Size = System::Drawing::Size(95, 56);
			this->button_back_origin->TabIndex = 57;
			this->button_back_origin->Text = L"Back to \r\nOrigin";
			this->button_back_origin->UseVisualStyleBackColor = false;
			this->button_back_origin->Click += gcnew System::EventHandler(this, &MTPFM::button_back_origin_Click);
			// 
			// label_msec
			// 
			this->label_msec->AutoSize = true;
			this->label_msec->BackColor = System::Drawing::Color::Transparent;
			this->label_msec->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_msec->Location = System::Drawing::Point(15, 85);
			this->label_msec->Name = L"label_msec";
			this->label_msec->Size = System::Drawing::Size(135, 20);
			this->label_msec->TabIndex = 54;
			this->label_msec->Text = L"Moving time (ms)";
			// 
			// label_y_posi
			// 
			this->label_y_posi->AutoSize = true;
			this->label_y_posi->BackColor = System::Drawing::Color::Transparent;
			this->label_y_posi->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_y_posi->Location = System::Drawing::Point(15, 51);
			this->label_y_posi->Name = L"label_y_posi";
			this->label_y_posi->Size = System::Drawing::Size(130, 20);
			this->label_y_posi->TabIndex = 53;
			this->label_y_posi->Text = L"Y position (-1~1)";
			// 
			// label_x_posi
			// 
			this->label_x_posi->AutoSize = true;
			this->label_x_posi->BackColor = System::Drawing::Color::Transparent;
			this->label_x_posi->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label_x_posi->Location = System::Drawing::Point(15, 17);
			this->label_x_posi->Name = L"label_x_posi";
			this->label_x_posi->Size = System::Drawing::Size(131, 20);
			this->label_x_posi->TabIndex = 52;
			this->label_x_posi->Text = L"X position (-1~1)";
			// 
			// button1_manual_control
			// 
			this->button1_manual_control->BackColor = System::Drawing::Color::Gainsboro;
			this->button1_manual_control->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->button1_manual_control->ForeColor = System::Drawing::Color::RoyalBlue;
			this->button1_manual_control->Location = System::Drawing::Point(19, 122);
			this->button1_manual_control->Name = L"button1_manual_control";
			this->button1_manual_control->Size = System::Drawing::Size(240, 39);
			this->button1_manual_control->TabIndex = 15;
			this->button1_manual_control->Text = L"Go to abs. position";
			this->button1_manual_control->UseVisualStyleBackColor = false;
			this->button1_manual_control->Click += gcnew System::EventHandler(this, &MTPFM::button1_manual_control_Click);
			// 
			// YScomboBox__raster_chopperfreq
			// 
			this->YScomboBox__raster_chopperfreq->BackColor = System::Drawing::SystemColors::ScrollBar;
			this->YScomboBox__raster_chopperfreq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->YScomboBox__raster_chopperfreq->FormattingEnabled = true;
			this->YScomboBox__raster_chopperfreq->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"1", L"2", L"4" });
			this->YScomboBox__raster_chopperfreq->Location = System::Drawing::Point(162, 240);
			this->YScomboBox__raster_chopperfreq->Name = L"YScomboBox__raster_chopperfreq";
			this->YScomboBox__raster_chopperfreq->Size = System::Drawing::Size(73, 28);
			this->YScomboBox__raster_chopperfreq->TabIndex = 107;
			// 
			// label64
			// 
			this->label64->AutoSize = true;
			this->label64->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->label64->ForeColor = System::Drawing::Color::White;
			this->label64->Location = System::Drawing::Point(14, 242);
			this->label64->Name = L"label64";
			this->label64->Size = System::Drawing::Size(107, 20);
			this->label64->TabIndex = 106;
			this->label64->Text = L"Chopper Freq.";
			// 
			// label65
			// 
			this->label65->AutoSize = true;
			this->label65->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8, System::Drawing::FontStyle::Bold));
			this->label65->ForeColor = System::Drawing::Color::White;
			this->label65->Location = System::Drawing::Point(15, 260);
			this->label65->Name = L"label65";
			this->label65->Size = System::Drawing::Size(145, 13);
			this->label65->TabIndex = 108;
			this->label65->Text = L"Downscale from Line Freq.";
			// 
			// MTPFM
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(7, 15);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->AutoSize = true;
			this->BackColor = System::Drawing::Color::DimGray;
			this->ClientSize = System::Drawing::Size(881, 843);
			this->ControlBox = false;
			this->Controls->Add(this->label23);
			this->Controls->Add(this->panel_Stage);
			this->Controls->Add(this->panel2);
			this->Controls->Add(this->button2_co_ex);
			this->Controls->Add(this->tabControl5);
			this->Controls->Add(this->label_Real_time);
			this->Controls->Add(this->button_shut_down_MEMS);
			this->Controls->Add(this->status);
			this->Controls->Add(this->button9);
			this->Controls->Add(this->panelCollapse);
			this->Controls->Add(this->label5);
			this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->ForeColor = System::Drawing::Color::White;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Margin = System::Windows::Forms::Padding(3, 4, 3, 4);
			this->Name = L"MTPFM";
			this->Text = L"NTU miniScope";
			this->panel_real_time->ResumeLayout(false);
			this->panel_real_time->PerformLayout();
			this->panel1->ResumeLayout(false);
			this->panel1->PerformLayout();
			this->panel_Stage->ResumeLayout(false);
			this->panel_Stage->PerformLayout();
			this->panelCollapse->ResumeLayout(false);
			this->panelCollapse->PerformLayout();
			this->tabControl3->ResumeLayout(false);
			this->tabPage13->ResumeLayout(false);
			this->tabPage13->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar_savenum))->EndInit();
			this->tabPage8->ResumeLayout(false);
			this->tabPage8->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_average_num_count))->EndInit();
			this->tabPage1->ResumeLayout(false);
			this->tabPage1->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_raw_count))->EndInit();
			this->tabPage5->ResumeLayout(false);
			this->tabPage5->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_num_frame))->EndInit();
			this->tabControl4->ResumeLayout(false);
			this->tabPage12->ResumeLayout(false);
			this->tabPage12->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_multiplySP))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_offsetvalueSP))->EndInit();
			this->tabPage19->ResumeLayout(false);
			this->tabPage19->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_DCEkernel))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_DCE))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_contrast_alpha))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_brightness_beta))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_adjust_gamma))->EndInit();
			this->tabPage9->ResumeLayout(false);
			this->tabPage9->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_normalizeMax))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_normalizeMin))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_MaxVal))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_MinVal))->EndInit();
			this->tabPage6->ResumeLayout(false);
			this->tabPage6->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Gaussian_sigmaV))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Gaussian_sigmaH))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Gaussian_kernel))->EndInit();
			this->tabPage7->ResumeLayout(false);
			this->tabPage7->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_Median_kernel))->EndInit();
			this->tabPage10->ResumeLayout(false);
			this->tabPage10->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_tile_grid_V))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_tile_grid_H))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_clip_limit))->EndInit();
			this->tabPage11->ResumeLayout(false);
			this->tabPage11->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_BiD_align_bin))->EndInit();
			this->tabControl1->ResumeLayout(false);
			this->tabPage2->ResumeLayout(false);
			this->tabPage2->PerformLayout();
			this->YSbutton_lissajous_setamp->ResumeLayout(false);
			this->tabPage16->ResumeLayout(false);
			this->tabPage16->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar_raster_phase))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__raster_y_amp))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__raster_x_amp))->EndInit();
			this->tabPage15->ResumeLayout(false);
			this->tabPage15->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_offsettime))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_phase))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_y_amp))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->YStrackBar__lissajous_x_amp))->EndInit();
			this->tabPage17->ResumeLayout(false);
			this->tabPage17->PerformLayout();
			this->panel2->ResumeLayout(false);
			this->panel2->PerformLayout();
			this->tabControl5->ResumeLayout(false);
			this->tabPage18->ResumeLayout(false);
			this->tabPage18->PerformLayout();
			this->tabPage14->ResumeLayout(false);
			this->tabPage14->PerformLayout();
			this->tabControl2->ResumeLayout(false);
			this->tabPage3->ResumeLayout(false);
			this->tabPage3->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_phase))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar3_raster_y_amp))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar2__raster_x_amp))->EndInit();
			this->tabPage4->ResumeLayout(false);
			this->tabPage4->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}

#pragma endregion
	private: System::Void Updating_Controls()
	{
		// Activating controls
		Configure->Enabled							= aNTU->Go_allow_configure();
		button1_connect_to_controller->Enabled		= aNTU->Go_allow_connect();
		button_acq->Enabled						    = aNTU->Go_allow_acquisition();
		button1_connect_to_controller->Enabled = false;
		radioButton_checkBox_Activate_chan_1->Checked = true;
		checkBox_merge_channels->Checked = true;
		checkBox_Export16BIT->Checked = true;
	}
	
	private: System::Void button_Configure_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		// Checking the software if already configured
		if (configure_status)
		{
			status->ForeColor = Color::Gold;
			status->Text = "Already configured!!";
			return;
		}
		status->ForeColor = Color::Gold;
		status->Text = "Trying to configure...";
		log_file << "Trying to configure...\n";
		Configure->Text = "Wait...";
		Configure->Enabled = false;
		Application::DoEvents();

		load_headMount_calibration_data();

		// Setting the Samples Per Record
		internal_clock = checkBox2__Int_clk->Checked;
		external_clock = checkBox1_Ext_clk->Checked;
		if (external_clock == true)		aNTU->samplePerSec = double(external_clock_freq) * pow(10, 6);
		else if (internal_clock == true)    aNTU->samplePerSec = double(internal_clock_freq) * pow(10, 6);

		// Alazar ATS 9146 Configuration
		//update_scan_settings(internal_clock, external_clock);
		char* result = aNTU->config_Alazar(internal_clock_freq, internal_clock, external_clock);
		if (gcnew System::String(result) != "OK")
		{
			status->ForeColor = Color::Coral;
			status->Text = gcnew System::String(result);
			Configure->Text = "Configure";
			Application::DoEvents();
			Configure->Enabled = true;
			return;
		}

		// After configuring all the status (Successfully)
		status->ForeColor = Color::Gold;
		status->Text = "Successfully configured!";
		log_file << "Successfully configured!\n";
		configure_status = true;
		Configure->BackColor = Color::Silver;
		Configure->ForeColor = Color::Silver;
		Configure->Text = "Configured!";

		checkBox1_Ext_clk->Enabled = false;
		checkBox2__Int_clk->Enabled = false;
		checkBox_uniDirection->Enabled = false;
		checkBox_biDirection->Enabled = false;
		comboBox2__Ext_clk->Enabled = false;
		comboBox3_Int_clk->Enabled = false;
		button1_connect_to_controller->Enabled = true;
		//comboBox_num_Y_lines->Enabled = false;

	}

	private: System::Void button1_connect_to_controller_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		textBox_xoffset->Text = aNTU->xOffset.ToString("f3");
		textBox_yoffset->Text = aNTU->yOffset.ToString("f3");
		aNTU->memsControllerConfig();
		MessageBox::Show("Successfully connect to MEMS controller!", "Information", MessageBoxButtons::OK, MessageBoxIcon::Asterisk);
		button1_connect_to_controller->Enabled = false;
	}

	private: System::Void button_acq_Click(System::Object^ sender, System::EventArgs^ e)
	{
		// CHECKING IF ALREADY CONFIGURED OR NOT
		if (configure_status == false)
		{
			button_Configure_Click(sender, e);
		}

		// If still cannot be configured!
		if (configure_status == false)
			return;

		// Start capturing
		_cprintf("\n\n~~~~~~~~~~~~ Acquisition Start !!! ~~~~~~~~~~~~\n\n");

		// Getting post trigger samples : Points in 1 line
		aNTU->postTriggerSamples = aNTU->get_postTrigSample();

		// FIXING THE Samples Per Record
		aNTU->samplesPerRecord = aNTU->preTriggerSamples + aNTU->postTriggerSamples;

		// SELECTING NUMBER OF Y-LINES
		aNTU->y_length = System::Int32::Parse(comboBox_num_Y_lines->GetItemText(comboBox_num_Y_lines->SelectedItem));

		/*cout << "samples per record" << aNTU->samplesPerRecord << endl;
		cout << "RPB" << aNTU->recordPerBuffer << endl;;
		cout << "BPA" << aNTU->buffersPerAcquisition << endl;
		cout << "RPA" << aNTU->recordsPerAcquisition << endl;*/

		// Update some vital bools before doing multi-threading
		aNTU->running_loops = true;
		aNTU->scanAlive = true;
		aNTU->saveData = checkBox_save_data_alazar->Checked;
		aNTU->initiateScan();
		textBox_FOV_l->Text = (float(trackBar2__raster_x_amp->Value) / 100 * 1000 - 52).ToString();
		textBox_FOV_r->Text = (float(trackBar3_raster_y_amp->Value) / 100 * 1000 - 52).ToString();
		textBox_pixel_size_row->Text = aNTU->postTriggerSamples.ToString();
		textBox_pixel_size_col->Text = (aNTU->y_length).ToString();
		
	}

	private: System::Void button_shut_down_MEMS_Click(System::Object^ sender, System::EventArgs^ e)
	{
		// Disconnect all the I/O 
		// End the program by returning the device to origin
		aNTU->MEMS_status = true;
		aNTU->running_loops = false;
		
		aNTU->TerminateAll();
		YSCORE->stopAllOperations();


		mti->ResetDevicePosition(); // Send analog outputs (and device) back to origin in 25ms
		mti->SetDeviceParam(MTIParam::MEMSDriverEnable, false);		// Turn off the MEMS driver
		mti->DisconnectDevice(); // Disconnect the device

		bool status = mti->IsDeviceConnected();
		if (status == true)
			printf("Connected\n");
		else
			printf("Disconnected!\n");

		delete(mti); // Delete the MTIDevice object
		Sleep(10);
		
		connection->Close();

		t_after = clock();
		printf("total time:%lf sec\n", float(t_after - t_before) / CLOCKS_PER_SEC);

		// Abort the acquisition
		aNTU->retCode = AlazarAbortAsyncRead(aNTU->boardHandle);
		if (aNTU->retCode != ApiSuccess)
		{
			printf("Error: AlazarAbortAsyncRead failed -- %s\n", AlazarErrorToText(aNTU->retCode));
		}

		timer_refresh_UI->Stop();
		timer_refresh_UI->Enabled = false;

		// Shut down GUI
		_cprintf("\n\n~~~~~~~~~~~~ Close GUI by clicking 'x' !!! ~~~~~~~~~~~~\n\n");
	}
	private: System::Void button_start_scan_Click(System::Object^ sender, System::EventArgs^ e)
	{
		// Update some vital bools
		aNTU->MEMS_status = true;

		// Confirm the scanning mode!!
		aNTU->bi_direction = checkBox_biDirection->Checked;
		aNTU->uni_Directional = checkBox_uniDirection->Checked;
		aNTU->xAmp = float(trackBar2__raster_x_amp->Value) / 100;
		aNTU->yAmp = float(trackBar3_raster_y_amp->Value) / 100;
		textBox_line_time->Text = aNTU->lineTime.ToString();

		// Re-do bi raster scanning
		aNTU->memsSync();

		// Get the triggers timestamp
		//aNTU->getTrigTimeStamp();
	}
	private: System::Void button_Stop_Click(System::Object^ sender, System::EventArgs^ e)
	{
		// End the program by returning the device to origin
		mti->ResetDevicePosition(); // Send analog outputs (and device) back to origin in 25ms
		aNTU->MEMS_status = false;
		aNTU->scanHOLD = true;
		aNTU->constructorExecutedGUI = true;
		aNTU->scanAlive = true;
		aNTU->running_loops = false;
		aNTU->pause_Prev = true;
		//destroyAllWindows();
		// Stop scanning
		_cprintf("\n~~~~~~~~~~~~ Pause scanning !!! ~~~~~~~~~~~~\n\n");
	}

	private: System::Void button1_manual_control_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->xPosition = Convert::ToDouble(textBox_x_po->Text);
		aNTU->yPosition = Convert::ToDouble(textBox_y_pos->Text);
		unsigned char mData = 8;
		aNTU->mSec = Convert::ToInt32(textBox_movingtime->Text);
		mti->GoToDevicePosition(aNTU->xPosition, aNTU->yPosition, mData, aNTU->mSec);
		printf("Moving Complete!\n");
		textBox_crnt_po_x->Text = (aNTU->xOffset + aNTU->xPosition).ToString("f3");
		textBox_crnt_po_y->Text = (aNTU->yOffset + aNTU->yPosition).ToString("f3");
	}
	
	private: System::Void trackBar2__raster_x_amp_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		//Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)	Application::DoEvents();
		label4__raster_x_amp->Text = (float(trackBar2__raster_x_amp->Value) / 100).ToString();
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}

	private: System::Void trackBar3_raster_y_amp_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		//Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)	Application::DoEvents();
		label7_raster_y_amp->Text = (float(trackBar3_raster_y_amp->Value)* aNTU->FOV_ratio / 100).ToString();
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}

	private: System::Void checkBox1_Ext_clk_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		if (GUI_status == false)
		{
			//Scanning : Hold
			aNTU->scanHOLD = true;
			while (aNTU->scanHOLD)	Application::DoEvents();
			GUI_status = true;
			
			// Checking if already external is activated
			if (checkBox1_Ext_clk->Checked)
			{
				// Setting parameters
				external_clock = true;
				internal_clock = false;
				checkBox2__Int_clk->Checked = false;
				Application::DoEvents();

				System::String^ result = update_scan_settings(internal_clock, external_clock);
				if (result == "OK")
				{
					status->ForeColor = Color::LightGreen;
					status->Text = "Set : External Clock Frequency : " + external_clock_freq.ToString() + " MS/s";
				}
				else
				{
					status->ForeColor = Color::Coral;
					status->Text = result;
				}
			}
			// IF EXTERNAL CLOCK IS DESELECTED
			else
			{
				// Setting parameters
				external_clock = false;
				internal_clock = true;
				checkBox2__Int_clk->Checked = true;
				Application::DoEvents();
				
				// Updating settings
				System::String^ result = update_scan_settings(internal_clock, external_clock);
				if (result == "OK")
				{
					// Successfully configured
					status->ForeColor = Color::LightGreen;
					status->Text = "Selected : Internal clock";
					Application::DoEvents();
				}
				else
				{
					status->ForeColor = Color::Coral;
					status->Text = result;
				}
			}
			GUI_status = false;

			// Scanning : Free
			aNTU->scanFREE = true;
			while (aNTU->scanFREE)	Application::DoEvents();
		}
		
		
	}

	private: System::Void checkBox2__Int_clk_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		if (GUI_status == false)
		{
			// Scanning : Hold
			aNTU->scanHOLD = true;
			while (aNTU->scanHOLD)	Application::DoEvents();
			GUI_status = true;
			// IF INTERNAL CLOCK IS SELECTED
			if (checkBox2__Int_clk->Checked)
			{
				// Setting parameters
				internal_clock = true;
				external_clock = false;
				checkBox1_Ext_clk->Checked = false;
				Application::DoEvents();
				// Updating settings
				System::String^ result = update_scan_settings(internal_clock, external_clock);
				if (result == "OK")
				{
					// Successfully configured
					status->ForeColor = Color::LightGreen;
					status->Text = "Set : Internal Clock Frequency : " + internal_clock_freq.ToString() + " MS/s";
					Application::DoEvents();

					// Updating the calibration data
					// load_calibration_data();
				}
				else
				{
					status->ForeColor = Color::Coral;
					status->Text = result;
				}
			}
			// IF INTERNAL CLOCK IS DESELECTED
			else
			{
				// Setting parameters
				internal_clock = false;
				external_clock = true;
				checkBox1_Ext_clk->Checked = true;
				Application::DoEvents();
				// Updating settings
				System::String^ result = update_scan_settings(internal_clock, external_clock);
				if (result == "OK")
				{
					// Successfully configured
					status->ForeColor = Color::LightGreen;
					status->Text = "Selected : External clock";
					Application::DoEvents();

					// Updating the calibration data
					//load_calibration_data();
				}
				else
				{
					status->ForeColor = Color::Coral;
					status->Text = result;
				}
			}
			GUI_status = false;

			// Scanning : Free
			aNTU->scanFREE = true;
			while (aNTU->scanFREE)	Application::DoEvents();
		}
	}

	private: System::Void checkBox_uniDirection_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		// Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)	Application::DoEvents();

		aNTU->uni_Directional = checkBox_uniDirection->Checked;
		if (aNTU->uni_Directional)
		{
			checkBox_biDirection->Checked = false;
			// Updating the Scan Settings
			update_mems_scan_details();
			//aNTU->Uni_RasterScan(mti, (float(trackBar2__raster_x_amp->Value) / 10), (float(trackBar3_raster_y_amp->Value) / 10), aNTU->y_length);
			System::String^ result = update_scan_settings(internal_clock, external_clock);
			if (result == "OK")
			{
				status->ForeColor = Color::LightGreen;
				status->Text = "Scanning Mode: Uni-Directional Raster scan";
			}
			else
			{
				status->ForeColor = Color::Coral;
				status->Text = result;
			}
		}
		else
		{
			checkBox_biDirection->Checked = true;
			// Updating the Scan Settings
			update_mems_scan_details();
			System::String^ result = update_scan_settings(internal_clock, external_clock);
			if (result == "OK")
			{
				status->ForeColor = Color::LightGreen;
				status->Text = "Scanning Mode: Bi-Directional Raster Scan";
			}
			else
			{
				status->ForeColor = Color::Coral;
				status->Text = result;
			}
		}

		// Scanning : Free
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}

	private: System::Void checkBox_biDirection_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		// Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)	Application::DoEvents();

		aNTU->bi_direction = checkBox_biDirection->Checked;
		if (aNTU->bi_direction)
		{
			checkBox_uniDirection->Checked = false;
			// Updating the Scan Settings
			update_mems_scan_details();
			System::String^ result = update_scan_settings(internal_clock, external_clock);
			if (result == "OK")
			{
				status->ForeColor = Color::LightGreen;
				status->Text = "Scanning Mode: Bi-Directional Raster scan";
			}
			else
			{
				status->ForeColor = Color::Coral;
				status->Text = result;
			}
		}
		else
		{
			checkBox_uniDirection->Checked = true;
			// Updating the Scan Settings
			update_mems_scan_details();
			System::String^ result = update_scan_settings(internal_clock, external_clock);
			if (result == "OK")
			{
				status->ForeColor = Color::LightGreen;
				status->Text = "Scanning Mode: Uni-Directional Raster Scan";
			}
			else
			{
				status->ForeColor = Color::Coral;
				status->Text = result;
			}
		}

		// Scanning : Free
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}

	private: System::Void update_mems_scan_details()
	{

	}

	private: System::String^ update_scan_settings(bool internal_clock, bool external_clock)
	{
		// Updating some parameters
		aNTU->update_recordPerBuffer(aNTU->bi_direction, aNTU->y_length);
		aNTU->buffersPerAcquisition = aNTU->get_buffersPerAcquisition(aNTU->bi_direction, aNTU->y_length);
		aNTU->recordsPerAcquisition = aNTU->get_recordsPerAcquisition(aNTU->bi_direction, aNTU->y_length, aNTU->buffersPerAcquisition);

		// EXTREMELY IMPORTANT SETTINGS FOR SCANNING : Getting the selected clock frequency
		int index_internal_clock_freq = comboBox3_Int_clk->SelectedIndex;
		internal_clock_freq = internal_clock_freq_array[index_internal_clock_freq];
		int index_external_clock_freq = comboBox2__Ext_clk->SelectedIndex;
		external_clock_freq = external_clock_freq_array[index_external_clock_freq];

		// Setting the aNTU->samplesPerRecord
		if (external_clock == true)
			aNTU->samplePerSec = double(external_clock_freq) * pow(10, 6);
		else if (internal_clock == true)
			aNTU->samplePerSec = double(internal_clock_freq) * pow(10, 6);

		// Getting post trigger samples : Points in 1 line
		aNTU->postTriggerSamples = aNTU->get_postTrigSample();

		// FIXING THE aNTU->samplesPerRecord
		aNTU->samplesPerRecord = aNTU->preTriggerSamples + aNTU->postTriggerSamples;

		// Reconfiguring the board ATS9146
		if (configure_status == true)
		{
			// Updating the settings
			char* result = aNTU->update_Settings(internal_clock_freq, internal_clock, external_clock);

			// Checking if successful
			if (gcnew System::String(result) != "OK")
				return gcnew System::String(result);
		}

		// Everything is OK : Return 1
		return "OK";
	}

	private: System::Void button1_DOWN_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->move_relative_position = true;
		aNTU->move_absolute_position = false;
		aNTU->move_distance = Convert::ToInt32(textBox3_relative_pos->Text);
		axis->MoveRelative(double(aNTU->move_distance), Units::Length_Micrometres, true, 0, Units::Native, 0, Units::Native);
		position = axis->GetPosition(Units::Length_Micrometres);
		textBox_current_position->Text = position.ToString("f1");
	}

	private: System::Void button1_UP_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->move_relative_position = true;
		aNTU->move_absolute_position = false;
		aNTU->move_distance = Convert::ToInt32(textBox3_relative_pos->Text);
		int n_move_distance = -(aNTU->move_distance);
		if (this->toggleButtons_initial_position->Checked == true) {
			double previewposition = Convert::ToDouble(textBox_current_position->Text);
			double initialposition = Convert::ToDouble(textBox_initial_position->Text);
			if ((previewposition - double(aNTU->move_distance))<initialposition) {
				status->ForeColor = Color::LightPink;
				status->Text = "Error: Out of initial position";

			}
			else {
				axis->MoveRelative(double(n_move_distance), Units::Length_Micrometres, true, 0, Units::Native, 0, Units::Native);
				position = axis->GetPosition(Units::Length_Micrometres);
				textBox_current_position->Text = position.ToString("f1");
			}
		}
		else if (this->toggleButtons_initial_position->Checked == false) {
			axis->MoveRelative(double(n_move_distance), Units::Length_Micrometres, true, 0, Units::Native, 0, Units::Native);
			position = axis->GetPosition(Units::Length_Micrometres);
			textBox_current_position->Text = position.ToString("f1");
		}

	}

	private: System::Void button_HOME_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		//axis->Home(true);
		axis->MoveAbsolute(double(0.0), Units::Length_Micrometres, true, 0, Units::Native, 0, Units::Native);
		status->Text = "HOME is complete! (Zaber stage)";
		position = axis->GetPosition(Units::Length_Micrometres);
		textBox_current_position->Text = position.ToString("f1");
	}

	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		axis->Stop(true);
	}

	private: System::Void button2_abs_pos_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->move_distance = Convert::ToInt32(textBox_abs_position->Text);
		if (this->toggleButtons_initial_position->Checked == true) {
			double initialposition = Convert::ToDouble(textBox_initial_position->Text);
			if (double(aNTU->move_distance) < initialposition) {
				status->ForeColor = Color::LightPink;
				status->Text = "Error: Out of initial position";
			}
			else {
				axis->MoveAbsolute(double(aNTU->move_distance), Units::Length_Micrometres, true, 0, Units::Native, 0, Units::Native);
				position = axis->GetPosition(Units::Length_Micrometres);
				textBox_current_position->Text = position.ToString("f1");
			}
		}
		else if (this->toggleButtons_initial_position->Checked == false) {
			axis->MoveAbsolute(double(aNTU->move_distance), Units::Length_Micrometres, true, 0, Units::Native, 0, Units::Native);
			position = axis->GetPosition(Units::Length_Micrometres);
			textBox_current_position->Text = position.ToString("f1");
		}

	}

	private: System::Void radioButton1_rel_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->move_relative_position = true;
		aNTU->move_absolute_position = false;
		button1_UP->Enabled = true;
		button1_DOWN->Enabled = true;
		button2_abs_pos->Enabled = false;
		textBox3_relative_pos->Enabled = true;
		textBox_abs_position -> Enabled = false;
	}

	private: System::Void radioButton2_abs_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
	{
		aNTU->move_relative_position = false;
		aNTU->move_absolute_position = true;
		button1_UP->Enabled = false;
		button1_DOWN->Enabled = false;
		button2_abs_pos->Enabled = true;
		textBox3_relative_pos->Enabled = false;
		textBox_abs_position->Enabled = true;
	}

	private: System::Void button2_co_ex_Click(System::Object^ sender, System::EventArgs^ e)
	{
		if (">" == button2_co_ex->Text)
		{
			// Expand
			panelCollapse->Visible = true;
			panel_real_time->Visible = true;
			label_Real_time->Visible = true;
			button2_co_ex->Text = "<";
			this->Width = 850;
		}
		else
		{
			// Collapse
			panelCollapse->Visible = false;
			panel_real_time->Visible = false;
			label_Real_time->Visible = false;
			this->Width = 425;
			button2_co_ex->Text = ">";
		}
	}

	private: System::Void trackBar_phase_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->bi_direction_offset = trackBar_phase->Value;
		label_bi_direction_offset->Text = "Offset = " + trackBar_phase->Value.ToString();
	}

	private: System::Void button2_open_image_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		System::String^ folder_name1 = {};
		System::IO::Stream^ myStream1;
		OpenFileDialog^ openFileDialog1 = gcnew OpenFileDialog;

		// Can put some words on the filename dialog
		openFileDialog1->FileName;
		openFileDialog1->InitialDirectory = "C:\Ian\Imaging";
		openFileDialog1->Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
		openFileDialog1->FilterIndex = 2;
		openFileDialog1->RestoreDirectory = true;

		if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			if ((myStream1 = openFileDialog1->OpenFile()) != nullptr)
			{
				// Insert code to read the stream here.
				// Read the directory of the file.
				folder_name1 = Path::GetDirectoryName(openFileDialog1->FileName);
				textBox1_open_image->Text = folder_name1;
				myStream1->Close();
			}
		}

	}
	private: System::Void checkBox1_mode_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		if (checkBox1_mode->Checked)
		{
			// Enable Light mode
			this->BackColor = Color::WhiteSmoke;
			this->ForeColor = Color::DimGray;
		}
		else
		{
			// Enable Dark mode
			this->BackColor = Color::DimGray;
			this->ForeColor = Color::WhiteSmoke;
		}
	}
	private: System::Void button_back_origin_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->MEMS_status = true;
		mti->ResetDevicePosition();
		textBox_crnt_po_x->Text = aNTU->xPosition.ToString("f3");
		textBox_crnt_po_y->Text = aNTU->yPosition.ToString("f3");
	}
	private: System::Void button_save_loaded_frame_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		// ACTIVATING SAVING PROCESS
		// Current date & time based on current system
		char date[10];
		char month[10];
		int date_num = 0;
		int month_num = 0;
		time_t now = time(0);
		tm* ltm = localtime(&now);
		date_num = ltm->tm_mday;
		month_num = 1 + ltm->tm_mon;

		// Setting date string
		if (date_num < 10)	sprintf(date, "0%d", date_num);
		else				sprintf(date, "%d", date_num);

		// Setting month string
		if (month_num == 1) sprintf(month, "Jan");
		else if (month_num == 2) sprintf(month, "Feb");
		else if (month_num == 3) sprintf(month, "Mar");
		else if (month_num == 4) sprintf(month, "Apr");
		else if (month_num == 5) sprintf(month, "May");
		else if (month_num == 6) sprintf(month, "Jun");
		else if (month_num == 7) sprintf(month, "Jul");
		else if (month_num == 8) sprintf(month, "Aug");
		else if (month_num == 9) sprintf(month, "Sep");
		else if (month_num == 10) sprintf(month, "Oct");
		else if (month_num == 11) sprintf(month, "Nov");
		else if (month_num == 12) sprintf(month, "Dec");

		// ASKING FOR DIRECTORY (FOR NAS STORAGE)
		SaveFileDialog^ dialog = gcnew SaveFileDialog();
		dialog->FileName = "Save Here";
		if (dialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			System::String^ folder_name;
			folder_name = Path::GetDirectoryName(dialog->FileName);
			sprintf(defaultLoc, "%s", folder_name);
		}
		else	return;

		// GETTING THE FOLDER NAME (WITH DATE AND TIME) : HERE WE ADD THE DATE, MONTH, YEAR TO SAVE TIME LATER!!!
		sprintf(full_date_incl_path, "%s\\%s-%s-%d___", defaultLoc, date, month, (1900 + ltm->tm_year));

		// Preparing to start saving
		aNTU->save_data_chan_1 = checkBox1_save_ch1_data->Checked;

		status->ForeColor = Color::LightGreen;
		status->Text = "Saving frames...Sit back & Relax!!!";

	}
	private: System::Void checkBox_bin_export_cropped_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
	{
	}
	private: System::Void checkBox_bin_export_full_frame_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
	}
	private: System::Void load_calibration_data()
	{
		// LOADING CALIBRATION DATA IF AVAILABLE
		if (read_cal.is_open() == false)
		{
			// Generating the filename
			char file_name[500] = {};
			sprintf(file_name, "%s", "Calibrations\\default_settings\\default_Settings_Controller.txt");
			// Checking if calibration data exist
			if (File::Exists(gcnew System::String(file_name)) == true)
			{
				read_cal.open(file_name, std::ios::in);
				std::string line;
				char char_word[500];

				// Prev_Y
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->prev_Y_length = aNTU->extract_data_from_string(char_word);
				// Prev_SamplePerRec
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->prev_sampPerRec = aNTU->extract_data_from_string(char_word);
				// MEMS COM PORT
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->portnumber = aNTU->extract_data_from_string(char_word);
				// Calibration FOV
				// FOV_X
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->cal_FOV_x = aNTU->extract_data_from_string(char_word);
				// FOV_Y
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->cal_FOV_y = aNTU->extract_data_from_string(char_word);
			}
			else if (File::Exists(gcnew System::String(file_name)) == false)
				printf("Cannot find the calibaration file!!!");
		}

		// Closing the file
		read_cal.close();
	}
	private: System::Void load_headMount_calibration_data()
	{
		// LOADING CALIBRATION DATA IF AVAILABLE
		if (read_cal.is_open() == false)
		{
			// Generating the filename
			char file_name[500] = {};
			sprintf(file_name, "%s%d%s", "Calibrations\\headMount\\headMount_", aNTU->headMount_count, ".txt");
			// Checking if calibration data exist
			if (File::Exists(gcnew System::String(file_name)) == true)
			{
				read_cal.open(file_name, std::ios::in);
				std::string line;
				char char_word[500];

				// xOffset
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->xOffset = aNTU->extract_data_from_string(char_word);
				// yOffset
				std::getline(read_cal, line);
				sprintf(char_word, "%s", line.c_str());
				aNTU->yOffset = aNTU->extract_data_from_string(char_word);

			}
			else if (File::Exists(gcnew System::String(file_name)) == false)
				printf("Cannot find the calibaration file!!!");
		}

		// Closing the file
		read_cal.close();
	}
	private: System::Void comboBox_headmount_count_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->headMount_count = comboBox_headmount_count->SelectedIndex;
	}

	private: System::Void button_clear_terminal_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		Console::Clear();
	}
	private: System::Void radioButton_checkBox_Activate_chan_1_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		// Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)	Application::DoEvents();

		aNTU->activate_chan_1 = radioButton_checkBox_Activate_chan_1->Checked;
		if (aNTU->activate_chan_1)
		{
			Application::DoEvents();
			checkBox1_save_ch1_data->Enabled = true;
			progressBar_progress_exporting->Enabled = true;
			textBox_progress_export->Enabled = true;
			aNTU->save_data_chan_1 = checkBox1_save_ch1_data->Checked;
			if (aNTU->save_data_chan_1 == 1)
			{
				aNTU->save_data_chan_1 = 1;
				//total_chan2save++;
			}

			status->ForeColor = Color::LightGreen;
			status->Text = "Activated: Channel 1";
		}
		else if (!aNTU->activate_chan_1)
		{
			aNTU->save_data_chan_1 = checkBox1_save_ch1_data->Checked;
			if (aNTU->save_data_chan_1 == 1)
			{
				aNTU->save_data_chan_1 = 0;
				//total_chan2save--;
			}
			checkBox1_save_ch1_data->Enabled = false;
			progressBar_progress_exporting->Enabled = false;
			textBox_progress_export->Enabled = false;
			aNTU->img_thread_cont_chan_1 = false;
			status->ForeColor = Color::LightGreen;
			status->Text = "Deactivated: Channel 1";
		}

		// Updating channel colors
		//aNTU->setChannelColors();

		// Scanning : Free
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}
	private: System::Void comboBox_num_Y_lines_SelectedIndexChanged_1(System::Object^ sender, System::EventArgs^ e) 
	{
		// Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)	Application::DoEvents();

		aNTU->y_length = System::Int32::Parse(comboBox_num_Y_lines->GetItemText(comboBox_num_Y_lines->SelectedItem));
		aNTU->buffersPerAcquisition = aNTU->y_length / aNTU->get_recordPerBuffer();
		aNTU->recordsPerAcquisition = aNTU->get_recordPerBuffer() * aNTU->buffersPerAcquisition;

		System::String^ result = update_scan_settings(internal_clock, external_clock);
		if (result != "OK")
		{
			status->ForeColor = Color::Coral;
			status->Text = result;
			return;
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Set: Number of Y Lines = " + aNTU->y_length.ToString() + " lines";
		}

		// Scanning : Free
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}
	private: System::Void comboBox3_Int_clk_SelectedIndexChanged_1(System::Object^ sender, System::EventArgs^ e) 
	{
		// Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)
			Application::DoEvents();
		int index_internal_clock_freq = comboBox3_Int_clk->SelectedIndex;
		internal_clock_freq = internal_clock_freq_array[index_internal_clock_freq];

		// Checking if already internal is activated
		if (internal_clock)
		{
			System::String^ result = update_scan_settings(internal_clock, external_clock);
			if (result == "OK")
			{
				status->ForeColor = Color::LightGreen;
				status->Text = "Set : Internal Clock Frequency : " + internal_clock_freq.ToString() + " MS/s";
			}
			else
			{
				status->ForeColor = Color::Coral;
				status->Text = result;
			}
		}
		// Scanning : Free
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}
	private: System::Void comboBox2__Ext_clk_SelectedIndexChanged_1(System::Object^ sender, System::EventArgs^ e) 
	{
		// Scanning : Hold
		aNTU->scanHOLD = true;
		while (aNTU->scanHOLD)
			Application::DoEvents();
		int index_external_clock_freq = comboBox2__Ext_clk->SelectedIndex;
		external_clock_freq = external_clock_freq_array[index_external_clock_freq];

		// Checking if already external is activated
		if (external_clock)
		{
			System::String^ result = update_scan_settings(internal_clock, external_clock);
			if (result == "OK")
			{
				status->ForeColor = Color::LightGreen;
				status->Text = "Set : External Clock Frequency : " + external_clock_freq.ToString() + " MS/s";
			}
			else
			{
				status->ForeColor = Color::Coral;
				status->Text = result;
			}
		}
		// Scanning : Free
		aNTU->scanFREE = true;
		while (aNTU->scanFREE)	Application::DoEvents();
	}

	private: System::Void checkBox_merge_channels_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		if (checkBox_merge_channels->Checked == true)
		{
			aNTU->chan_merge = true;
		}
		else
		{
			//checkBox_fix_distortion_preview->Enabled = true;
			//checkBox_calibrate_prev->Enabled = true;
			aNTU->chan_merge = false;
		}
	}
	private: System::Void button_start_frames_saving_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		if (!aNTU->running_loops)
		{
			status->ForeColor = Color::LightPink;
			status->Text = "Error : Start acquisition first!";
		}
		else
		{
			// ABORTING IF ALREADY SAVING IS FOING ON
			if (aNTU->frames_saving_cond_chan_1)
			{
				// Preparing to start saving
				bool result = aNTU->abort_frame_saving(
					aNTU->xAmp,
					aNTU->yAmp,
					internal_clock,
					external_clock,
					internal_clock_freq,
					external_clock_freq);

				if (result == true)
				{
					button_start_frames_saving->ForeColor = Color::Black;
					button_start_frames_saving->Text = "Start";
					status->ForeColor = Color::LightGreen;
					status->Text = "Saving Process Aborted!!!";
				}
				else
				{
					status->ForeColor = Color::LightPink;
					status->Text = "Error: Engine Failure";
				}
			}

			// ACTIVATING SAVING PROCESS
			else
			{
				// Current date & time based on current system
				char date[10];
				char month[10];
				int date_num = 0;
				int month_num = 0;
				time_t now = time(0);
				tm* ltm = localtime(&now);
				date_num = ltm->tm_mday;
				month_num = 1 + ltm->tm_mon;

				// Setting date string
				if (date_num < 10)	sprintf(date, "0%d", date_num);
				else				sprintf(date, "%d", date_num);

				// Setting month string
				if (month_num == 1) sprintf(month, "Jan");
				else if (month_num == 2) sprintf(month, "Feb");
				else if (month_num == 3) sprintf(month, "Mar");
				else if (month_num == 4) sprintf(month, "Apr");
				else if (month_num == 5) sprintf(month, "May");
				else if (month_num == 6) sprintf(month, "Jun");
				else if (month_num == 7) sprintf(month, "Jul");
				else if (month_num == 8) sprintf(month, "Aug");
				else if (month_num == 9) sprintf(month, "Sep");
				else if (month_num == 10) sprintf(month, "Oct");
				else if (month_num == 11) sprintf(month, "Nov");
				else if (month_num == 12) sprintf(month, "Dec");

				// ASKING FOR DIRECTORY (FOR NAS STORAGE)
				SaveFileDialog^ dialog = gcnew SaveFileDialog();
				dialog->FileName = "Save Here";
				if (dialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
				{
					System::String^ folder_name;
					folder_name = Path::GetDirectoryName(dialog->FileName);
					sprintf(defaultLoc, "%s", folder_name);
				}
				else	return;

				// GETTING THE FOLDER NAME (WITH DATE AND TIME) : HERE WE ADD THE DATE, MONTH, YEAR TO SAVE TIME LATER!!!
				sprintf(full_date_incl_path, "%s\\%s-%s-%d___", defaultLoc, date, month, (1900 + ltm->tm_year));

				// Preparing to start saving
				bool result = aNTU->initialize_frame_saving(
					brief_detail,
					full_date_incl_path);

				if (result == true)
				{
					button_start_frames_saving->ForeColor = Color::Coral;
					button_start_frames_saving->Text = "Abort";
					status->ForeColor = Color::LightGreen;
					status->Text = "Saving frames...Sit back & Relax!!!";
				}
				else
				{
					status->ForeColor = Color::LightPink;
					status->Text = "Error: Engine Failure";
				}
			}
		}
	}
	private: System::Void timer_refresh_UI_Tick(System::Object^ sender, System::EventArgs^ e)
	{
		// For Channel 1: DISPLAYING THE PROGRESS BAR		
		progressBar_progress_exporting->Value = aNTU->frame_saving_progress_chan_1;
		textBox_progress_export->Text = aNTU->frame_saving_progress_chan_1.ToString("0.##") + " %";

		// DISPLAYING ACQUISITION TIME & FRAME RATE & REFRESH RATE
		//textBox_acq_time->Text = (aNTU->AcqTime * 1000.0 / 1000.0).ToString("0.####");
		//textBox_frame_rate->Text = (aNTU->FrameRate * 100.0 / 100.0).ToString("0.##");	
		//textBox_refresh_rate->Text = (aNTU->refreshRate * 100.0 / 100.0).ToString("0.##");

		textBox_acq_time->Text = (aYSNTU->acqtime).ToString("F5");
		textBox_frame_rate->Text = (aYSNTU->framerate).ToString("F2");
		textBox_refresh_rate->Text = (aYSNTU->refreshtime).ToString("F2");
		textBox2->Text = aYSNTU->saveprogress.ToString("0.##") + " %";

		progressBar1->Value = aYSNTU->saveprogress;
		if (progressBar1->Value == 100) {
			YSbutton_clear->Enabled = true;
		}

		progressBar_raw_ch1->Value = aNTU->raw_frame_saving_progress_chan_1;
		textBox_progress_raw_ch1->Text = aNTU->raw_frame_saving_progress_chan_1.ToString("0.##") + " %";

		position = axis->GetPosition(Units::Length_Micrometres);
		textBox_current_position->Text = position.ToString("f1");

		aNTU->apply_gaussian = toggleButtons_Gaussian->Checked;
		aNTU->apply_median = toggleButtons_Median->Checked;
		aNTU->apply_CLAHE = toggleButtons_CLAHE->Checked;

		if (aYSNTU->highspeedmode == true) {
			this->textBox_highspeedmode->BackColor = System::Drawing::Color::LimeGreen;
			this->textBox_highspeedmode->ForeColor = System::Drawing::Color::White;

		}
		else if (aYSNTU->highspeedmode == false) {
			this->textBox_highspeedmode->BackColor = System::Drawing::Color::DimGray;
			this->textBox_highspeedmode->ForeColor = System::Drawing::Color::Gray;
		}
		
	}

	private: System::Void comboBox_prev_scale_down_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->scale_down_preview_temp = comboBox_prev_scale_down->SelectedIndex + 1;
		if (aNTU->scale_down_preview_temp > 1)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Set : Preview windows : Scale down : " + aNTU->scale_down_preview_temp.ToString() + " times";
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Set : Preview windows : No scaling";
		}
	}
	private: System::Void comboBox_prev_scale_down_bin_read_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->scale_down_prev_bin_read_temp = comboBox_prev_scale_down_bin_read->SelectedIndex + 1;
		if (aNTU->scale_down_prev_bin_read_temp > 1)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Set : Binary data preview : Scale down : " + aNTU->scale_down_prev_bin_read_temp.ToString() + " times";
		}
		else if (aNTU->scale_down_prev_bin_read_temp == 1)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Set : Binary data preview : No scaling";
		}
	}
	 // ENABLE/ DISABLE BIN VIEW
	private: System::Void EnbDisBinView(bool state)
	{

	}

	private: System::Void textBox_brief_detail_TextChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		if (textBox_brief_detail->TextLength != 0 && busy_GUI == false)
		{
			// Receiving the string
			busy_GUI = true;
			char charStr[1000];
			System::String^ details = textBox_brief_detail->Lines[0];
			sprintf(charStr, "%s", details);

			// Checking if 1st letter is 'Space'
			if (charStr[0] == ' ')
			{
				textBox_brief_detail->Clear();
				busy_GUI = false;
				return;
			}

			// If longer than 100 characters
			if (strlen(charStr) > MAX_CHAR_DIR)
			{
				charStr[MAX_CHAR_DIR] = '\0';
			}

			// Checking for invalid characters
			for (int i = 0; i < strlen(charStr); i++)
			{
				int ASCII = int(charStr[i]);
				if (isalnum(charStr[i]) != 0 || (ASCII == 32) || (ASCII == 45) || (ASCII == 46) || (ASCII == 95))
				{
					continue;
				}
				else
				{
					charStr[i] = '_';
					details = gcnew System::String(charStr);
					textBox_brief_detail->Text = details;
					textBox_brief_detail->SelectionStart = i + 1;
					textBox_brief_detail->SelectionLength = 0;
				}
			}

			// Replacing the System string
			//details = gcnew System::String(charStr);
			//textBox_brief_detail->Text = details;
			//textBox_brief_detail->SelectionStart = textBox_brief_detail->TextLength;
			//textBox_brief_detail->SelectionLength = 0;

			// Esception handled string for the directory name
			sprintf(brief_detail, "%s", charStr);
			busy_GUI = false;
		}
		else
		{
			sprintf(brief_detail, "%s", "");
		}
	}
	private: System::Void trackBar_BiD_align_bin_Scroll(System::Object^ sender, System::EventArgs^ e)
	{
		aNTU->BiD_align_bin = trackBar_BiD_align_bin->Value;
		label_BiD_align_Val_bin->Text = "Offset = " + aNTU->BiD_align_bin.ToString();
	}

	private: System::Void button_load_directory_Click(System::Object^ sender, System::EventArgs^ e) 
	{

	}
	private: System::Void checkBox_Export16BIT_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->bin_export_16BIT = checkBox_Export16BIT->Checked;
	}
	private: System::Void trackBar_num_frame_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->frame_count = trackBar_num_frame->Value;
		label1_num_frame->Text = "Num = " + aNTU->frame_count.ToString();
	}
	private: System::Void trackBar_raw_count_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->raw_frame_count = trackBar_raw_count->Value;
		labelraw_count->Text = "Num = " + aNTU->raw_frame_count.ToString();
	}
	private: System::Void button2_save_data_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->save_data_chan_1 = checkBox1_save_ch1_data->Checked;
		
	}
	private: System::Void button_raw_save_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->download_raw_image = checkBox1_raw_ch1->Checked;
	}
	private: System::Void trackBar_brightness_beta_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->beta_chan_1 = trackBar_brightness_beta->Value;

		aYSNTU->YSbrightness = trackBar_brightness_beta->Value;
		
		label_brightness_beta->Text = aNTU->beta_chan_1.ToString();
	}
	private: System::Void trackBar_contrast_alpha_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		double temp = trackBar_contrast_alpha->Value;

		aNTU->alpha_chan_1 = temp / 10;

		aYSNTU->YSconstrat = temp / 10;

		label_contrast_alpha->Text = aNTU->alpha_chan_1.ToString();
	}
	private: System::Void trackBar_adjust_gamma_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		// GETTING THE GAMMA VALUE
		double temp = 0;
		temp = trackBar_adjust_gamma->Value;
		aNTU->gamma_chan_1 = temp / 100;

		aYSNTU->YSgamma = temp / 100;
	
		label_adjust_gamma->Text = aNTU->gamma_chan_1.ToString();
		
	}
	private: System::Void trackBar_Gaussian_kernel_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		int slider_value = trackBar_Gaussian_kernel->Value;
		slider_value -= (1 - slider_value % 2);
		label_Gaussian_kernel->Text = "Kernel = " + slider_value.ToString();
		aNTU->gaussian_kernel_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSgaussian_kernel = slider_value;
	}
	private: System::Void trackBar_Gaussian_sigmaH_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		int slider_value = trackBar_Gaussian_sigmaH->Value;
		label_Gaussian_sigmaH->Text = "SigmaH = " + slider_value.ToString();
		aNTU->gaussian_sigmaH_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSgaussian_sigmaH = slider_value;
	}
	private: System::Void trackBar_Gaussian_sigmaV_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		int slider_value = trackBar_Gaussian_sigmaV->Value;
		label_Gaussian_sigmaV->Text = "SigmaV = " + slider_value.ToString();
		aNTU->gaussian_sigmaV_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSgaussian_sigmaV = slider_value;
	}
	private: System::Void trackBar_Median_kernel_Scroll(System::Object^ sender, System::EventArgs^ e)
	{
		int slider_value = int(trackBar_Median_kernel->Value) * 2 + 1;
		label_Median_kernel->Text = "Kernel = " + slider_value.ToString();
		aNTU->median_kernel_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSmedian_kernel = slider_value;
	}
	private: System::Void trackBar_clip_limit_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		int slider_value = trackBar_clip_limit->Value;
		label_clip_limit->Text = "ClipLimit = " + slider_value.ToString();
		aNTU->CLAHE_clip_limit_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSCLAHE_clip_limit = slider_value;
	}
	private: System::Void trackBar_tile_grid_H_Scroll(System::Object^ sender, System::EventArgs^ e)
	{
		int slider_value = trackBar_tile_grid_H->Value;
		label_tile_grid_H->Text = "tileGridH = " + slider_value.ToString();
		aNTU->CLAHE_tile_grid_H_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSCLAHE_tile_grid_H = slider_value;
	}
	private: System::Void trackBar_tile_grid_V_Scroll(System::Object^ sender, System::EventArgs^ e)
	{
		int slider_value = trackBar_tile_grid_V->Value;
		label_tile_grid_V->Text = "tileGridV = " + slider_value.ToString();
		aNTU->CLAHE_tile_grid_V_bin_file[CHAN_1] = slider_value;
		aYSNTU->YSCLAHE_tile_grid_V = slider_value;
	}
	private: System::Void toggleButtons_contrast_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
		aYSNTU->YSapply_contrast = toggleButtons_contrast->Checked;
		if (aYSNTU->YSapply_contrast == true)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Contrast : Activated";
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Contrast : Dectivated";
		}
	}
	private: System::Void toggleButtons_CLAHE_CheckedChanged(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->apply_CLAHE = toggleButtons_CLAHE->Checked;
		aYSNTU->YSapply_CLAHE = toggleButtons_CLAHE->Checked;
		if (aNTU->apply_CLAHE == true)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Gaussian Blur : Activated";
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Gaussian Blur : Dectivated";
		}
	}
	private: System::Void toggleButtons_Gaussian_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
	{
		aNTU->apply_gaussian = toggleButtons_Gaussian->Checked;
		aYSNTU->YSapply_gaussian = toggleButtons_Gaussian->Checked;
		if (aNTU->apply_gaussian == true)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Gaussian Blur : Activated";
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Gaussian Blur : Dectivated";
		}
	}
	private: System::Void toggleButtons_Median_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
	{
		aNTU->apply_median = toggleButtons_Median->Checked;
		aYSNTU->YSapply_median = toggleButtons_Median->Checked;
		if (aNTU->apply_median == true)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Gaussian Blur : Activated";
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Gaussian Blur : Dectivated";
		}
	}
	private: System::Void toggleButtons_allow_averaging_CheckedChanged(System::Object^ sender, System::EventArgs^ e)
	{
		aNTU->frame_averaging = toggleButtons_allow_averaging->Checked;

		if (aNTU->frame_averaging)
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Activated : Frame averaging";
			// UPDATING THE VALUE OF : AVG FRAME NUMBER
			aNTU->num_avg_frame_new = trackBar_average_num_count->Value;
			aYSNTU->avgcount = trackBar_average_num_count->Value;
			label2_average_count->Text = trackBar_average_num_count->Value.ToString();
			
			// Activating frame averaging
			aNTU->frame_averaging = true;	
			aYSNTU->avgstate = true;
		}
		else
		{
			status->ForeColor = Color::LightGreen;
			status->Text = "Deactivated : Frame averaging";
			aNTU->frame_averaging = false;
			aYSNTU->avgstate = false;
			aNTU->num_avg_frame_new = 1;
			aYSNTU->avgcount = 1;
		}
	}
	private: System::Void trackBar_average_num_count_Scroll(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->num_avg_frame_new = trackBar_average_num_count->Value;
		aYSNTU->avgcount = trackBar_average_num_count->Value;
		label2_average_count->Text = trackBar_average_num_count->Value.ToString();
	}

	private: System::Void button_ETL_conn_Click(System::Object^ sender, System::EventArgs^ e) 
	{
		aNTU->etlControllerConfig();
	}
private: System::Void label22_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label23_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label36_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void trackBar4_Scroll(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label34_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label35_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label33_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void trackBar5_Scroll(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label37_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void label38_Click(System::Object^ sender, System::EventArgs^ e) {
}
private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {
	aNTU->volumetricScanConfig();
}
private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {
}





//YuShin
//ETL
private: System::Void button_ETL_up_Click(System::Object^ sender, System::EventArgs^ e) {
	double ETLvoltage = System::Convert::ToDouble(textBox_ETL_voltage->Text);
	if ((ETLvoltage + 0.01)>70) {
		status->ForeColor = Color::LightCoral;
		status->Text = "Error :  ETL : out of range";
	}
	else {
		ETLvoltage = ETLvoltage + 0.01;
		double FLength = 1000000 / ((ETLvoltage - 47) * 2.05);
		YSCORE->FLsetONLY(ETLvoltage);
		textBox_ETL_FL->Text= FLength.ToString("F2");
	}
}
private: System::Void button_ETL_upup_Click(System::Object^ sender, System::EventArgs^ e) {
	double ETLvoltage = System::Convert::ToDouble(textBox_ETL_voltage->Text);
	if ((ETLvoltage + 0.1) > 70) {
		status->ForeColor = Color::LightCoral;
		status->Text = "Error :  ETL : out of range";
	}
	else {
		ETLvoltage = ETLvoltage + 0.1;
		double FLength = 1000000 / ((ETLvoltage - 47) * 2.05);
		YSCORE->FLsetONLY(ETLvoltage);
		textBox_ETL_FL->Text = FLength.ToString("F2");
	}
}
private: System::Void button_ETL_down_Click(System::Object^ sender, System::EventArgs^ e) {
	double ETLvoltage = System::Convert::ToDouble(textBox_ETL_voltage->Text);
	if ((ETLvoltage - 0.01) < 20) {
		status->ForeColor = Color::LightCoral;
		status->Text = "Error :  ETL : out of range";
	}
	else {
		ETLvoltage = ETLvoltage - 0.01;
		double FLength = 1000000 / ((ETLvoltage - 47) * 2.05);
		YSCORE->FLsetONLY(ETLvoltage);
		textBox_ETL_FL->Text = FLength.ToString("F2");
	}
}
private: System::Void button_ETL_downdown_Click(System::Object^ sender, System::EventArgs^ e) {
	double ETLvoltage = System::Convert::ToDouble(textBox_ETL_voltage->Text);
	if ((ETLvoltage - 0.1) < 20) {
		status->ForeColor = Color::LightCoral;
		status->Text = "Error :  ETL : out of range";
	}
	else {
		ETLvoltage = ETLvoltage - 0.1;
		double FLength = 1000000 / ((ETLvoltage - 47) * 2.05);
		YSCORE->FLsetONLY(ETLvoltage);
		textBox_ETL_FL->Text = FLength.ToString("F2");
	}
}
//Raster
private: System::Void YStrackBar__raster_x_amp_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double MEMS_xamplitude = ((static_cast<double>(YStrackBar__raster_x_amp->Value)) / 100);
	YStextBox__raster_x_amp->Text = MEMS_xamplitude.ToString();
}
private: System::Void YStrackBar__raster_y_amp_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double MEMS_yamplitude = ((static_cast<double>(YStrackBar__raster_y_amp->Value)) / 100);
	YStextBox__raster_y_amp->Text = MEMS_yamplitude.ToString();
}
private: System::Void YSbutton_raster_setamp_Click(System::Object^ sender, System::EventArgs^ e) {
	YStrackBar__raster_x_amp->Value = System::Convert::ToInt32(System::Convert::ToDouble(YStextBox__raster_x_amp->Text) * 100);
	YStrackBar__raster_y_amp->Value = System::Convert::ToInt32(System::Convert::ToDouble(YStextBox__raster_y_amp->Text) * 100);
}
private: System::Void YSbutton_raster_configure_Click(System::Object^ sender, System::EventArgs^ e) {
	if (aYSNTU->firstinitialize == false) {
		YSCORE->initialize();
	}

	int clockstate = 0;
	if (YScheckBox_extclock->CheckState == CheckState::Checked) {
		clockstate = 6;
		aYSNTU->EXTCLOCK_freq = System::Convert::ToDouble(YStextBox__ExtClockFreq->Text) * 1000000;
	}
	else if (YScheckBox_intclock->CheckState == CheckState::Checked) {
		clockstate = YScomboBox_intclock->SelectedIndex;
	}

	aYSNTU->inputrangetype = inputrange[YScomboBox_inputrange->SelectedIndex];

	double ETLvoltage = System::Convert::ToDouble(textBox_ETL_voltage->Text);
	double MEMS_xamplitude = ((static_cast<double>(YStrackBar__raster_x_amp->Value)) / 100);
	YStextBox__raster_x_amp->Text = MEMS_xamplitude.ToString();
	double MEMS_yamplitude = ((static_cast<double>(YStrackBar__raster_y_amp->Value)) / 100);
	YStextBox__raster_y_amp->Text = MEMS_yamplitude.ToString();
	double MEMS_linefreq = YSlinefreq_array[YScomboBox__raster_linefreq->SelectedIndex];
	double MEMS_xoffset = System::Convert::ToDouble(YStextBox__raster_xoffset->Text);
	double MEMS_yoffset = System::Convert::ToDouble(YStextBox__raster_yoffset->Text);
	int MEMS_yline = YSyline_array[YScomboBox__raster_yline->SelectedIndex];
	aYSNTU->chopperdownscale = YSchopperdownscale_array[YScomboBox__raster_chopperfreq->SelectedIndex];
	
	if (double(MEMS_linefreq / double(MEMS_yline)) > 1) {
		if (YScheckBox_raster_slowspeed -> CheckState == CheckState::Checked) {
			YSCORE->TWODRaster_slow_configure(ETLvoltage, MEMS_linefreq, MEMS_xamplitude, MEMS_yamplitude, MEMS_yline, MEMS_xoffset, MEMS_yoffset, clockstate);
		}
		else {
			YSCORE->TWODRaster_configure(ETLvoltage, MEMS_linefreq, MEMS_xamplitude, MEMS_yamplitude, MEMS_yline, MEMS_xoffset, MEMS_yoffset, clockstate);
		}
		YSbutton_raster_configure->Enabled = false;
		YScomboBox_headmount->Enabled = false;
		YStrackBar__raster_x_amp->Enabled = false;
		YStrackBar__raster_y_amp->Enabled = false;
		YStextBox__raster_x_amp->Enabled = false;
		YStextBox__raster_y_amp->Enabled = false;
		YScheckBox_raster_slowspeed->Enabled = false;
		YSbutton_raster_stop->Enabled = true;
		YSbutton_raster_configure->Text = L"Configured";
		status->ForeColor = Color::LightGreen;
		status->Text = "Configure : Raster";
	}
	else {
		status->ForeColor = Color::LightCoral;
		status->Text = "!!! ERROR : Configure : Raster !!!";
	}
}
private: System::Void YSbutton_raster_startimg_Click(System::Object^ sender, System::EventArgs^ e) {
	YSbutton_raster_startimg->Enabled = false;
	textBox_FOV_l->Text = aYSNTU->FOVx.ToString();
	textBox_FOV_r->Text = aYSNTU->FOVy.ToString();
	textBox_pixel_size_row->Text = aYSNTU->Xpixelsize_display.ToString();
	textBox_pixel_size_col->Text = aYSNTU->Xpixelsize_display.ToString();
	textBox_chopper_freq->Text = aYSNTU->chopperfreq.ToString();
	System::Threading::Thread^ runThread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(this, &MTPFM::Run2DRaster));
	runThread->Start();
	status->ForeColor = Color::LightGreen;
	status->Text = "Start imaging : Raster";
}
private: System::Void YSbutton_raster_stop_Click(System::Object^ sender, System::EventArgs^ e) {
	YSCORE->stopAllOperations();
	YSbutton_raster_stop->Enabled = false;
	status->ForeColor = Color::LightGreen;
	status->Text = "Stop : Raster";

	aYSNTU->firstinitialize = false;
	YSbutton_raster_startimg->Enabled = true;
	YSbutton_raster_configure->Enabled = true;
	YScomboBox_headmount->Enabled = true;
	YStrackBar__raster_x_amp->Enabled = true;
	YStrackBar__raster_y_amp->Enabled = true;
	YStextBox__raster_x_amp->Enabled = true;
	YStextBox__raster_y_amp->Enabled = true;
	YScheckBox_raster_slowspeed->Enabled = true;
	toggleButtons_allow_averaging->Checked = false;
}
private: System::Void YStrackBar_raster_phase_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int phase_value = this->YStrackBar_raster_phase->Value;
	this->YSlabel_raster_phase->Text = phase_value.ToString();
	aYSNTU->raster_phase = phase_value;
}
//Lissajous
private: System::Void YStrackBar__lissajous_x_amp_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double MEMS_xamplitude = ((static_cast<double>(YStrackBar__lissajous_x_amp->Value)) / 100);
	YStextBox__lissajous_x_amp->Text = MEMS_xamplitude.ToString();
}
private: System::Void YStrackBar__lissajous_y_amp_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double MEMS_yamplitude = ((static_cast<double>(YStrackBar__lissajous_y_amp->Value)) / 100);
	YStextBox__lissajous_y_amp->Text = MEMS_yamplitude.ToString();
}
private: System::Void YSbutton__lissajous_setamp_Click(System::Object^ sender, System::EventArgs^ e) {
	YStrackBar__lissajous_x_amp->Value = System::Convert::ToInt32(System::Convert::ToDouble(YStextBox__lissajous_x_amp->Text) * 100);
	YStrackBar__lissajous_y_amp->Value = System::Convert::ToInt32(System::Convert::ToDouble(YStextBox__lissajous_y_amp->Text) * 100);
}
private: System::Void YStrackBar__lissajous_phase_Scroll(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->lissajous_phase = YSphase[YStrackBar__lissajous_phase->Value];
	YSlabel__lissajous_phase->Text = gcnew System::String(YSphasename[YStrackBar__lissajous_phase->Value].c_str());
}
private: System::Void YSbutton_lissajous_configure_Click(System::Object^ sender, System::EventArgs^ e) {
	if (aYSNTU->firstinitialize == false) {
		YSCORE->initialize();
	}

	if (YScheckBox_lissajous_samplemapsave->CheckState == CheckState::Checked) {
		aYSNTU->samplemapsave = true;
	}
	else {
		aYSNTU->samplemapsave = false;
	}

	int clockstate = 0;
	if (YScheckBox_extclock->CheckState == CheckState::Checked) {
		clockstate = 6;
		aYSNTU->EXTCLOCK_freq= System::Convert::ToDouble(YStextBox__ExtClockFreq->Text) *1000000;
	}
	else if (YScheckBox_intclock->CheckState == CheckState::Checked) {
		clockstate = YScomboBox_intclock->SelectedIndex;
	}

	aYSNTU->inputrangetype = inputrange[YScomboBox_inputrange->SelectedIndex];

	double ETLvoltage = System::Convert::ToDouble(textBox_ETL_voltage->Text);
	double MEMS_xamplitude = ((static_cast<double>(YStrackBar__lissajous_x_amp->Value)) / 100);
	YStextBox__raster_x_amp->Text = MEMS_xamplitude.ToString();
	double MEMS_yamplitude = ((static_cast<double>(YStrackBar__lissajous_y_amp->Value)) / 100);
	YStextBox__raster_y_amp->Text = MEMS_yamplitude.ToString();
	double MEMS_xfreq = System::Convert::ToDouble(YStextBox__lissajous_x_freq->Text);
	double MEMS_yfreq = System::Convert::ToDouble(YStextBox__lissajous_y_freq->Text);
	double MEMS_xoffset = System::Convert::ToDouble(YStextBox__lissajous_xoffset->Text);
	double MEMS_yoffset = System::Convert::ToDouble(YStextBox__lissajous_yoffset->Text);

	YSCORE->TWODLissajous_configure(ETLvoltage, MEMS_xamplitude, MEMS_yamplitude, MEMS_xoffset, MEMS_yoffset, MEMS_xfreq, MEMS_yfreq, clockstate);
	YSbutton_lissajous_configure->Enabled = false;
	YScomboBox_headmount->Enabled = false;
	YStrackBar__lissajous_x_amp->Enabled = false;
	YStrackBar__lissajous_y_amp->Enabled = false;
	YStrackBar__lissajous_phase->Enabled = false;
	YStextBox__lissajous_x_freq->Enabled = false;
	YStextBox__lissajous_y_freq->Enabled = false;
	YStextBox__lissajous_x_amp->Enabled = false;
	YStextBox__lissajous_y_amp->Enabled = false;
	YSbutton_lissajous_stop->Enabled = true;
	YSbutton_lissajous_configure->Text = L"Configured";
	status->ForeColor = Color::LightGreen;
	status->Text = "Configure : Lissajous";
}
private: System::Void YSbutton_lissajous_startimg_Click(System::Object^ sender, System::EventArgs^ e) {
	YSbutton_lissajous_startimg->Enabled = false;
	textBox_FOV_l->Text = aYSNTU->FOVx.ToString();
	textBox_FOV_r->Text = aYSNTU->FOVy.ToString();
	textBox_pixel_size_row->Text = aYSNTU->Xpixelsize_display.ToString();
	textBox_pixel_size_col->Text = aYSNTU->Xpixelsize_display.ToString();
	textBox_chopper_freq->Text = aYSNTU->chopperfreq.ToString();
	System::Threading::Thread^ runThread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(this, &MTPFM::Run2DLissajous));
	runThread->Start();
	status->ForeColor = Color::LightGreen;
	status->Text = "Start imaging : Lissajous";
}
private: System::Void YSbutton_lissajous_stop_Click(System::Object^ sender, System::EventArgs^ e) {
	YSCORE->stopAllOperations();
	YSbutton_lissajous_stop->Enabled = false;
	status->ForeColor = Color::LightGreen;
	status->Text = "Stop : Lissajous";

	aYSNTU->firstinitialize = false;
	YScomboBox_headmount->Enabled = true;
	YSbutton_lissajous_configure->Enabled = true;
	YStrackBar__lissajous_x_amp->Enabled = true;
	YStrackBar__lissajous_y_amp->Enabled = true;
	YStrackBar__lissajous_phase->Enabled = true;
	YStextBox__lissajous_x_freq->Enabled = true;
	YStextBox__lissajous_y_freq->Enabled = true;
	YStextBox__lissajous_x_amp->Enabled = true;
	YStextBox__lissajous_y_amp->Enabled = true;
	YSbutton_lissajous_startimg->Enabled = true;
	toggleButtons_allow_averaging->Checked = false;
	YScheckBox_lissajous_repairshow->Checked = false;
}
private: System::Void trackBar1_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double offsettime = double(YStrackBar__lissajous_offsettime->Value) / 10;
	aYSNTU->lissajous_delay = offsettime/1000000;
	YSlabel__lissajous_offsettime->Text = offsettime.ToString() + " µs";
}
//Alignment
private: System::Void YSbutton_manual_control_Click(System::Object^ sender, System::EventArgs^ e) {
	double xoffset = System::Convert::ToDouble(YStextBox_x_po->Text);
	double yoffset = System::Convert::ToDouble(YStextBox_y_po->Text);
	YStextBox_crnt_po_x->Text = YStextBox_x_po->Text;
	YStextBox_crnt_po_y->Text = YStextBox_y_po->Text;
	YSCORE->Alignmentsetup(xoffset, yoffset);
	status->ForeColor = Color::LightGreen;
	status->Text = "Set : Alighnment";
}
private: System::Void YSbutton_manual_stopcontrol_Click(System::Object^ sender, System::EventArgs^ e) {
	YSCORE->stopAlignment();
	this->YSbutton_manual_stopcontrol->Enabled = false;
	status->ForeColor = Color::LightGreen;
	status->Text = "Stop : Alighnment";
}
//Postprocessing
private: System::Void button9_Click(System::Object^ sender, System::EventArgs^ e) {
	YSPostprocessing^ ppForm = gcnew YSPostprocessing();
	ppForm->Show();
}
//YSSave
private: System::Void YSbutton_save_Click(System::Object^ sender, System::EventArgs^ e) {
	int savenum_value = this->YStrackBar_savenum->Value;
	aYSNTU->titleFolder = msclr::interop::marshal_as<std::string>(this->YStextBox_title->Text);
	if (this->YScheckBox_saveraw->CheckState == CheckState::Checked && this->YScheckBox_ch1->CheckState == CheckState::Checked) {
		aYSNTU->saverawstate = true;
		aYSNTU->saverawbinstate = YScheckBox_saverawbin->Checked;
		status->ForeColor = Color::LightGreen;
		status->Text = "Save : Image & Raw data";
	}
	else {
		aYSNTU->saverawstate = false;
		status->ForeColor = Color::LightGreen;
		status->Text = "Save : Image";
	}
	aYSNTU->saveprocessedstate = checkBox_saveprocessedimage->Checked;
	YSCORE->saveimg(savenum_value);
	this->YSbutton_save->Enabled = false;
	this->YSbutton_clear->Enabled = false;
}
private: System::Void YStrackBar_savenum_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int savenum_value = this->YStrackBar_savenum->Value;
	this->YSlabel_savenum->Text = "Num = "+savenum_value.ToString();
}
private: System::Void YSbutton_clear_Click(System::Object^ sender, System::EventArgs^ e) {
	this->YSbutton_save->Enabled = true;
	aYSNTU->savestate = false;
	aYSNTU->saverawstate = false;
	aYSNTU->saverawbinstate = false;
	aYSNTU->save_num = 0;
	aYSNTU->saveprogress = 0;
}

//headmount
private: System::Void YScomboBox_headmount_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	int selectedIndex = YScomboBox_headmount->SelectedIndex;
	YSCORE->readOffsetFromFile(selectedIndex);
	this->YStextBox__raster_xoffset->Text = aYSNTU->xoffset.ToString();
	this->YStextBox__raster_yoffset->Text = aYSNTU->yoffset.ToString();
	this->YStextBox__lissajous_xoffset->Text = aYSNTU->xoffset.ToString();
	this->YStextBox__lissajous_yoffset->Text = aYSNTU->yoffset.ToString();
	this->YStextBox_x_po->Text = aYSNTU->xoffset.ToString();
	this->YStextBox_y_po->Text = aYSNTU->yoffset.ToString();
	status->ForeColor = Color::LightGreen;
	status->Text = "Set : Headmount set";
}
//Repairment
private: System::Void YScheckBox_lissajous_repairshow_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (this->YScheckBox_lissajous_repairshow->Checked == true) {
		aYSNTU->YSapply_repair = true;
	}
	else if (this->YScheckBox_lissajous_repairshow->Checked == false) {
		aYSNTU->YSapply_repair = false;
	}
}
//Function
private: void Run2DLissajous() {
	YSCORE->TWODlissajousgetimage();
}
private: void Run2DRaster() {
	YSCORE->TWODrasterimage();
}
//ImageProcessing
private: System::Void trackBar_MinVal_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int MinVal = this->trackBar_MinVal->Value;
	int MaxVal = this->trackBar_MaxVal->Value;
	MaxVal += 12767;
	if (MaxVal > MinVal) {
		aYSNTU->minVal16bit = MinVal;
		this->label_MinVal->Text = MinVal.ToString();
		aYSNTU->maxVal16bit = MaxVal;
		this->label_MaxVal->Text = MaxVal.ToString();
	}
}
private: System::Void trackBar_MaxVal_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int MinVal = this->trackBar_MinVal->Value;
	int MaxVal = this->trackBar_MaxVal->Value;
	MaxVal += 12767;
	if (MaxVal > MinVal) {
		aYSNTU->minVal16bit = MinVal;
		this->label_MinVal->Text = MinVal.ToString();
		aYSNTU->maxVal16bit = MaxVal;
		this->label_MaxVal->Text = MaxVal.ToString();
	}
}
private: System::Void toggleButtons_minmax_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->YSapply_minmax = toggleButtons_minmax->Checked;
	if (aYSNTU->YSapply_minmax == true)
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "MinMax : Activated";
	}
	else
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "MinMax : Dectivated";
	}
	int MinVal = this->trackBar_MinVal->Value;
	int MaxVal = this->trackBar_MaxVal->Value;
	MaxVal += 12767;
	if (MaxVal > MinVal) {
		aYSNTU->minVal16bit = MinVal;
		this->label_MinVal->Text = MinVal.ToString();
		aYSNTU->maxVal16bit = MaxVal;
		this->label_MaxVal->Text = MaxVal.ToString();
	}
}
private: System::Void toggleButtons_signalproxessing_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->YSapply_signalprocessing = toggleButtons_signalproxessing->Checked;
	if (aYSNTU->YSapply_signalprocessing == true)
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "SignalProcessing : Activated";
	}
	else
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "SignalProcessing : Dectivated";
	}
	int OffsetVal = this->trackBar_offsetvalueSP->Value;
	double Multiply = double(this->trackBar_multiplySP->Value) / 100;

	aYSNTU->YSoffsetvalueSP = OffsetVal;
	this->label_offsetvalueSP->Text = aYSNTU->YSoffsetvalueSP.ToString();
	aYSNTU->YSmultiplySP = Multiply;
	this->label_multiplySP->Text = aYSNTU->YSmultiplySP.ToString();
}

private: System::Void trackBar_offsetvalueSP_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int OffsetVal = this->trackBar_offsetvalueSP->Value;
	double Multiply = double(this->trackBar_multiplySP->Value) / 100;

	aYSNTU->YSoffsetvalueSP = OffsetVal;
	this->label_offsetvalueSP->Text = aYSNTU->YSoffsetvalueSP.ToString();
	aYSNTU->YSmultiplySP = Multiply;
	this->label_multiplySP->Text = aYSNTU->YSmultiplySP.ToString();
}

private: System::Void trackBar_multiplySP_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int OffsetVal = this->trackBar_offsetvalueSP->Value;
	double Multiply = double(this->trackBar_multiplySP->Value)/100;

	aYSNTU->YSoffsetvalueSP = OffsetVal;
	this->label_offsetvalueSP->Text = aYSNTU->YSoffsetvalueSP.ToString();
	aYSNTU->YSmultiplySP = Multiply;
	this->label_multiplySP->Text = aYSNTU->YSmultiplySP.ToString();
}

private: System::Void toggleButtons_initial_position_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (this->toggleButtons_initial_position->Checked == true) {
		this->textBox_initial_position->Text = this->textBox_current_position->Text;
	}
	else if (this->toggleButtons_initial_position->Checked == false) {
		this->textBox_initial_position->Text = "---";
	}
}

private: System::Void toggleButtons_DCE_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->YSapply_DCE = toggleButtons_DCE->Checked;
	if (aYSNTU->YSapply_DCE == true)
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "DCE : Activated";
	}
	else
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "DCE : Dectivated";
	}
}
private: System::Void trackBar_DCE_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double temp = 0;
	temp = trackBar_DCE->Value;
	aYSNTU->YSalpha_DCE = temp / 100;

	label_DCE->Text = aYSNTU->YSalpha_DCE.ToString();
}
private: System::Void trackBar_DCEkernel_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int kernel = (trackBar_DCEkernel->Value * 2) +1;
	aYSNTU->YSkernel_DCE = kernel;

	label_DCEkernel->Text = aYSNTU->YSkernel_DCE.ToString();
}

private: System::Void toggleButtons_Binning_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->YSapply_binning = toggleButtons_Binning->Checked;
	if (aYSNTU->YSapply_binning == true)
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "Binning : Activated";
	}
	else
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "Binning : Dectivated";
	}
}
private: System::Void YScheckBox_extclock_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (YScheckBox_extclock->Checked ==true) {
		YScheckBox_intclock->Checked = false;
	}
}
private: System::Void YScheckBox_intclock_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (YScheckBox_intclock->Checked == true) {
		YScheckBox_extclock->Checked = false;
	}
}
private: System::Void toggleButtons_equalhistogram_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->YSapply_equalizehistogram = toggleButtons_equalhistogram->Checked;
	if (aYSNTU->YSapply_equalizehistogram == true)
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "Histogram equalization : Activated";
	}
	else
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "Histogram equalization : Dectivated";
	}
}
private: System::Void toggleButtons_normalize_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	aYSNTU->YSapply_normalize = toggleButtons_normalize->Checked;
	if (aYSNTU->YSapply_normalize == true)
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "Normalize : Activated";
	}
	else
	{
		status->ForeColor = Color::LightGreen;
		status->Text = "Normalize : Dectivated";
	}
	int MinValalpha = this->trackBar_normalizeMin->Value;
	int MaxValbeta = this->trackBar_normalizeMax->Value;
	MaxValbeta += 12767;
	if (MaxValbeta > MinValalpha) {
		aYSNTU->normalize_alpha = MinValalpha;
		this->label_normalizeMin->Text = MinValalpha.ToString();
		aYSNTU->normalize_beta = MaxValbeta;
		this->label_normalizeMax->Text = MaxValbeta.ToString();
	}
}
private: System::Void trackBar_normalizeMin_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int MinValalpha = this->trackBar_normalizeMin->Value;
	int MaxValbeta = this->trackBar_normalizeMax->Value;
	MaxValbeta += 12767;
	if (MaxValbeta > MinValalpha) {
		aYSNTU->normalize_alpha = MinValalpha;
		this->label_normalizeMin->Text = MinValalpha.ToString();
		aYSNTU->normalize_beta = MaxValbeta;
		this->label_normalizeMax->Text = MaxValbeta.ToString();
	}
}
private: System::Void trackBar_normalizeMax_Scroll(System::Object^ sender, System::EventArgs^ e) {
	int MinValalpha = this->trackBar_normalizeMin->Value;
	int MaxValbeta = this->trackBar_normalizeMax->Value;
	MaxValbeta += 12767;
	if (MaxValbeta > MinValalpha) {
		aYSNTU->normalize_alpha = MinValalpha;
		this->label_normalizeMin->Text = MinValalpha.ToString();
		aYSNTU->normalize_beta = MaxValbeta;
		this->label_normalizeMax->Text = MaxValbeta.ToString();
	}
}
private: System::Void toggleButtons_defaltimageprocessing_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (toggleButtons_defaltimageprocessing->Checked==true) {

		trackBar_contrast_alpha->Value=50;
		aNTU->alpha_chan_1 = trackBar_contrast_alpha->Value / 10;
		aYSNTU->YSconstrat = trackBar_contrast_alpha->Value / 10;
		label_contrast_alpha->Text = aNTU->alpha_chan_1.ToString();
		toggleButtons_contrast->Checked = true;
		//toggleButtons_CLAHE->Checked = true;

		this->trackBar_MinVal->Value=0;
		this->trackBar_MaxVal->Value=37233;
		int MinVal = this->trackBar_MinVal->Value;
		int MaxVal = this->trackBar_MaxVal->Value;
		MaxVal += 12767;
		if (MaxVal > MinVal) {
			aYSNTU->minVal16bit = MinVal;
			this->label_MinVal->Text = MinVal.ToString();
			aYSNTU->maxVal16bit = MaxVal;
			this->label_MaxVal->Text = MaxVal.ToString();
		}
		toggleButtons_minmax->Checked = true;

		//this->trackBar_normalizeMin->Value =0;
		//this->trackBar_normalizeMax->Value = 52768;
		//int MinValalpha = this->trackBar_normalizeMin->Value;
		//int MaxValbeta = this->trackBar_normalizeMax->Value;
		//MaxValbeta += 12767;
		//if (MaxValbeta > MinValalpha) {
		//	aYSNTU->normalize_alpha = MinValalpha;
		//	this->label_normalizeMin->Text = MinValalpha.ToString();
		//	aYSNTU->normalize_beta = MaxValbeta;
		//	this->label_normalizeMax->Text = MaxValbeta.ToString();
		//}
		//toggleButtons_normalize->Checked = true;
		
		//trackBar_DCE->Value = 400;
		//aYSNTU->YSalpha_DCE = double(trackBar_DCE->Value) / 100.0;
		//label_DCE->Text = aYSNTU->YSalpha_DCE.ToString();
		//trackBar_DCEkernel->Value = 3;
		//aYSNTU->YSkernel_DCE = (int(trackBar_DCEkernel->Value) * 2) + 1;
		//label_DCEkernel->Text = aYSNTU->YSkernel_DCE.ToString();
		//toggleButtons_DCE->Checked = true;
	}
	else if (toggleButtons_defaltimageprocessing->Checked == false) {
		toggleButtons_contrast->Checked = false;
		toggleButtons_CLAHE->Checked = false;
		toggleButtons_minmax->Checked = false;
		toggleButtons_normalize->Checked = false;
		toggleButtons_DCE->Checked = false;
	}
}

};
}
