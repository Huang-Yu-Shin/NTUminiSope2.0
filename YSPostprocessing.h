#pragma once
#include "YSImagemerge.h"
#include "YSCalculatePara.cuh"
#include <iostream>
#include <string>
#include <msclr/marshal_cppstd.h>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>

namespace MTPFM {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for YSPostprocessing
	/// </summary>
	public ref class YSPostprocessing : public System::Windows::Forms::Form
	{
	private:
		NTU::YSImageMerge* imagemerge;
		NTU::CudaCalPara* cudacalpara;

	private: System::Windows::Forms::TextBox^ textBox_CalPara_LOADRASTER;

	private: System::Windows::Forms::Button^ button_CalPara_LOADRASTER;





	private: System::Windows::Forms::Label^ label19;
	private: System::Windows::Forms::Button^ button_CalPara_calculate;







	private: System::Windows::Forms::ComboBox^ comboBox_CalPara_samplingrate;

	private: System::Windows::Forms::Panel^ panel5;














	private: System::Windows::Forms::Label^ label23;

	private: System::Windows::Forms::TextBox^ textBox_CalPara_ypixel;

	private: System::Windows::Forms::TextBox^ textBox_CalPara_xpixel;

	private: System::Windows::Forms::Label^ label26;
	private: System::Windows::Forms::TextBox^ textBox_CalPara_locy;

	private: System::Windows::Forms::TextBox^ textBox_CalPara_locx;

	private: System::Windows::Forms::Label^ label30;
	private: System::Windows::Forms::Label^ label29;
	private: System::Windows::Forms::Label^ label28;
	private: System::Windows::Forms::Label^ label27;
	private: System::Windows::Forms::Button^ button_CalPara_right;

	private: System::Windows::Forms::Button^ button_CalPara_left;

	private: System::Windows::Forms::Button^ button_CalPara_down;

	private: System::Windows::Forms::Button^ button_CalPara_up;

	private: System::Windows::Forms::TableLayoutPanel^ tableLayoutPanel_CalPara_result;
	private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_CalPara_twopoints;
	private: System::Windows::Forms::Label^ label_CalPara_twopoints;
	private: System::Windows::Forms::TextBox^ textBox_CalPara_locy2;
	private: System::Windows::Forms::TextBox^ textBox_CalPara_locx2;
	private: System::Windows::Forms::Label^ label18;
	private: System::Windows::Forms::Label^ label20;
	private: System::Windows::Forms::Label^ label21;
	private: System::Windows::Forms::Label^ label22;
	private: System::Windows::Forms::TabControl^ tabControl1;
	private: System::Windows::Forms::TabPage^ tabPage1;
	private: System::Windows::Forms::TabPage^ tabPage2;
	private: System::Windows::Forms::CheckBox^ checkBox_CalPara_linepoints;
	private: System::Windows::Forms::TabControl^ tabControl2;
	private: System::Windows::Forms::TabPage^ tabPage3;
	private: System::Windows::Forms::TabPage^ tabPage4;
	private: System::Windows::Forms::TextBox^ textBox_ScanPreview_phase;


	private: System::Windows::Forms::Label^ label31;



	private: System::Windows::Forms::Label^ label33;
	private: System::Windows::Forms::Label^ label2;
	private: System::Windows::Forms::ComboBox^ comboBox_ScanPreview_samplingrate;

	private: System::Windows::Forms::Label^ label9;
	private: System::Windows::Forms::Label^ label24;
	private: System::Windows::Forms::TextBox^ textBox_ScanPreview_xpixel;
	private: System::Windows::Forms::TextBox^ textBox_ScanPreview_ypixel;
	private: System::Windows::Forms::Button^ button_ScanPreview_LOADRASTER;
	private: System::Windows::Forms::TextBox^ textBox_ScanPreview_LOADRASTER;
	private: System::Windows::Forms::Button^ button_ScanPreview_rightphase;

	private: System::Windows::Forms::Button^ button_ScanPreview_leftphase;

	private: System::Windows::Forms::TextBox^ textBox_ScanPreview_framerate;


	private: System::Windows::Forms::Label^ label34;

private: System::Windows::Forms::ComboBox^ comboBox_ScanPreview_xyfreq;
private: System::Windows::Forms::TabPage^ tabPage5;
private: System::Windows::Forms::TextBox^ textBox_LOADLISSAJOUSFOLDER_EK;



private: System::Windows::Forms::Button^ button_LOADRASTER2;
private: System::Windows::Forms::TextBox^ textBox_LOADRASTER_EK;



private: System::Windows::Forms::Button^ button_LOADLISSAJOUSFOLDER2;



private: System::Windows::Forms::Button^ button_LOADLISSAJOUS;
private: System::Windows::Forms::TextBox^ textBox_LOADLISSAJOUS_EK;



private: System::Windows::Forms::TextBox^ textBox_ExtractKymograph_endpixel;

private: System::Windows::Forms::TextBox^ textBox_ExtractKymograph_startpixel;

private: System::Windows::Forms::TrackBar^ trackBar_ExtractKymograph_endpixel;

private: System::Windows::Forms::TrackBar^ trackBar_ExtractKymograph_startpixel;

private: System::Windows::Forms::Label^ label32;
private: System::Windows::Forms::Label^ label35;
private: System::Windows::Forms::Button^ button_ExtractKymograph_extractkymograph;

private: System::Windows::Forms::Label^ label36;
private: System::Windows::Forms::Label^ label37;
private: System::Windows::Forms::Button^ button_ExtractKymograph_quickset;
private: CustomControls::DefineControls::ToggleButtons^ toggleButtons_ExtractKymograph_choosedisplay;
private: System::Windows::Forms::Label^ label_ExtractKymograph_choosedisplay;
private: System::Windows::Forms::Label^ label38;
private: System::Windows::Forms::Label^ label39;
private: System::Windows::Forms::Label^ label40;
private: System::Windows::Forms::Button^ button_shut_down;




private: System::Windows::Forms::Label^ label3;
private: System::Windows::Forms::Label^ label_TRANSFERTOTIFF;
private: System::Windows::Forms::Button^ button_LOADLISSAJOUS_RAWBIN;
private: System::Windows::Forms::Button^ button_TRANSFERTOTIFF;
private: System::Windows::Forms::TextBox^ textBox_LOADLISSAJOUSBIN;
private: System::Windows::Forms::Label^ label41;





















	private: System::Windows::Forms::Label^ label25;
	public:
		YSPostprocessing(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			cudacalpara = new NTU::CudaCalPara();
			imagemerge = new NTU::YSImageMerge(cudacalpara);
			timer_refresh_UI->Interval = 250;
			timer_refresh_UI->Enabled = true;
			timer_refresh_UI->Start();


			this->comboBox_CalPara_samplingrate->SelectedIndex = 3;
			this->comboBox_ScanPreview_samplingrate->SelectedIndex = 3;
			this->comboBox_ScanPreview_xyfreq->SelectedIndex = 0;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~YSPostprocessing()
		{
			if (components)
			{
				delete components;
			}
			if (imagemerge)
			{
				delete imagemerge;
				imagemerge = nullptr;
			}
			this->timer_refresh_UI->Stop();
		}

	protected:


	protected:

	private: System::Windows::Forms::Label^ label1;


	private: System::Windows::Forms::CheckBox^ checkBox_MergeScantrace;


	private: System::Windows::Forms::Label^ label5;
	private: System::Windows::Forms::Label^ label6;
	private: System::Windows::Forms::TextBox^ textBox_offsetx;
	private: System::Windows::Forms::TextBox^ textBox_offsety;




	private: System::Windows::Forms::Label^ label7;
	private: System::Windows::Forms::CheckBox^ checkBox_save;
	private: System::Windows::Forms::Button^ button_Merge_and_Display;
	private: System::Windows::Forms::Button^ button_up;
	private: System::Windows::Forms::Button^ button_down;
	private: System::Windows::Forms::Button^ button_left;





	private: System::Windows::Forms::Button^ button_right;
	private: System::Windows::Forms::OpenFileDialog^ LOAD_RASTER;
	private: System::Windows::Forms::Button^ button_LOADRASTER;
	private: System::Windows::Forms::TextBox^ textBox_LOADRASTER;
	private: System::Windows::Forms::Button^ button_LOADLISSAJOUSFOLDER;
	private: System::Windows::Forms::TextBox^ textBox_LOADLISSAJOUSFOLDER;



	private: System::Windows::Forms::Label^ label8;
	private: System::Windows::Forms::TextBox^ textBox_LOADLISSAJOUS;
private: System::Windows::Forms::Button^ button_LOADLISSAJOUS_RAW;



	private: System::Windows::Forms::OpenFileDialog^ LOAD_LISSAJOUS;
	private: System::Windows::Forms::TextBox^ textBox__LOADLISSAJOUSFOLDER2;
private: System::Windows::Forms::Button^ button_LOADLISSAJOUSRAWFOLDER;



	private: System::Windows::Forms::Button^ button_setphaseandsave;



	private: System::Windows::Forms::TrackBar^ trackBar_lissajousoffset;



	private: System::Windows::Forms::FolderBrowserDialog^ LOAD_LISSAJOUS_FOLDER;
	private: System::Windows::Forms::Label^ label_lissajousoffset;



	private: System::Windows::Forms::Panel^ panel3;
	private: System::Windows::Forms::TextBox^ textBox_lissajousfreqy;
	private: System::Windows::Forms::TextBox^ textBox_lissajousfreqx;
	private: System::Windows::Forms::Label^ label13;
	private: System::Windows::Forms::Label^ label12;
	private: System::Windows::Forms::Label^ label11;
	private: System::Windows::Forms::TextBox^ textBox_numofsample;
	private: System::Windows::Forms::Label^ label10;
	private: System::Windows::Forms::TextBox^ textBox_samplingrate;
	private: System::Windows::Forms::Label^ label14;
private: System::Windows::Forms::TextBox^ textBox_lissajousphase;
private: System::Windows::Forms::Label^ label15;
private: System::Windows::Forms::TextBox^ textBox_lissajouspixelnum;

private: System::Windows::Forms::Label^ label16;
private: System::Windows::Forms::ProgressBar^ progressBar_reconstruct;

private: System::Windows::Forms::Timer^ timer_refresh_UI;

private: System::Windows::Forms::Label^ label_reconstruct;
private: System::Windows::Forms::Label^ label_resizey;

private: System::Windows::Forms::TrackBar^ trackBar_resizey;

private: System::Windows::Forms::Label^ label_resizex;

private: System::Windows::Forms::TrackBar^ trackBar_resizex;

private: System::Windows::Forms::Label^ label4;
private: System::Windows::Forms::Label^ label17;
private: System::Windows::Forms::Button^ button_fastset;

private: System::Windows::Forms::TextBox^ textBox_resizey;
private: System::Windows::Forms::TextBox^ textBox_resizex;
private: System::Windows::Forms::FolderBrowserDialog^ LOAD_LISSAJOUSRAW_FOLDER;
private: System::Windows::Forms::Label^ label_scanmodemerge;
private: System::Windows::Forms::ProgressBar^ progressBar_scanmodemerge;

private: System::ComponentModel::IContainer^ components;







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
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(YSPostprocessing::typeid));
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->checkBox_MergeScantrace = (gcnew System::Windows::Forms::CheckBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label6 = (gcnew System::Windows::Forms::Label());
			this->textBox_offsetx = (gcnew System::Windows::Forms::TextBox());
			this->textBox_offsety = (gcnew System::Windows::Forms::TextBox());
			this->label7 = (gcnew System::Windows::Forms::Label());
			this->checkBox_save = (gcnew System::Windows::Forms::CheckBox());
			this->button_Merge_and_Display = (gcnew System::Windows::Forms::Button());
			this->button_up = (gcnew System::Windows::Forms::Button());
			this->button_down = (gcnew System::Windows::Forms::Button());
			this->button_left = (gcnew System::Windows::Forms::Button());
			this->button_right = (gcnew System::Windows::Forms::Button());
			this->LOAD_RASTER = (gcnew System::Windows::Forms::OpenFileDialog());
			this->button_LOADRASTER = (gcnew System::Windows::Forms::Button());
			this->textBox_LOADRASTER = (gcnew System::Windows::Forms::TextBox());
			this->button_LOADLISSAJOUSFOLDER = (gcnew System::Windows::Forms::Button());
			this->textBox_LOADLISSAJOUSFOLDER = (gcnew System::Windows::Forms::TextBox());
			this->label8 = (gcnew System::Windows::Forms::Label());
			this->textBox_LOADLISSAJOUS = (gcnew System::Windows::Forms::TextBox());
			this->button_LOADLISSAJOUS_RAW = (gcnew System::Windows::Forms::Button());
			this->LOAD_LISSAJOUS = (gcnew System::Windows::Forms::OpenFileDialog());
			this->textBox__LOADLISSAJOUSFOLDER2 = (gcnew System::Windows::Forms::TextBox());
			this->button_LOADLISSAJOUSRAWFOLDER = (gcnew System::Windows::Forms::Button());
			this->button_setphaseandsave = (gcnew System::Windows::Forms::Button());
			this->trackBar_lissajousoffset = (gcnew System::Windows::Forms::TrackBar());
			this->LOAD_LISSAJOUS_FOLDER = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->label_lissajousoffset = (gcnew System::Windows::Forms::Label());
			this->label_reconstruct = (gcnew System::Windows::Forms::Label());
			this->progressBar_reconstruct = (gcnew System::Windows::Forms::ProgressBar());
			this->textBox_lissajouspixelnum = (gcnew System::Windows::Forms::TextBox());
			this->label16 = (gcnew System::Windows::Forms::Label());
			this->label_scanmodemerge = (gcnew System::Windows::Forms::Label());
			this->progressBar_scanmodemerge = (gcnew System::Windows::Forms::ProgressBar());
			this->textBox_resizey = (gcnew System::Windows::Forms::TextBox());
			this->textBox_resizex = (gcnew System::Windows::Forms::TextBox());
			this->button_fastset = (gcnew System::Windows::Forms::Button());
			this->label_resizey = (gcnew System::Windows::Forms::Label());
			this->trackBar_resizey = (gcnew System::Windows::Forms::TrackBar());
			this->label_resizex = (gcnew System::Windows::Forms::Label());
			this->trackBar_resizex = (gcnew System::Windows::Forms::TrackBar());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label17 = (gcnew System::Windows::Forms::Label());
			this->panel3 = (gcnew System::Windows::Forms::Panel());
			this->textBox_lissajousphase = (gcnew System::Windows::Forms::TextBox());
			this->label15 = (gcnew System::Windows::Forms::Label());
			this->textBox_samplingrate = (gcnew System::Windows::Forms::TextBox());
			this->label14 = (gcnew System::Windows::Forms::Label());
			this->textBox_numofsample = (gcnew System::Windows::Forms::TextBox());
			this->label10 = (gcnew System::Windows::Forms::Label());
			this->textBox_lissajousfreqy = (gcnew System::Windows::Forms::TextBox());
			this->textBox_lissajousfreqx = (gcnew System::Windows::Forms::TextBox());
			this->label13 = (gcnew System::Windows::Forms::Label());
			this->label12 = (gcnew System::Windows::Forms::Label());
			this->label11 = (gcnew System::Windows::Forms::Label());
			this->timer_refresh_UI = (gcnew System::Windows::Forms::Timer(this->components));
			this->LOAD_LISSAJOUSRAW_FOLDER = (gcnew System::Windows::Forms::FolderBrowserDialog());
			this->checkBox_CalPara_linepoints = (gcnew System::Windows::Forms::CheckBox());
			this->label22 = (gcnew System::Windows::Forms::Label());
			this->textBox_CalPara_locy2 = (gcnew System::Windows::Forms::TextBox());
			this->textBox_CalPara_locx2 = (gcnew System::Windows::Forms::TextBox());
			this->label18 = (gcnew System::Windows::Forms::Label());
			this->label20 = (gcnew System::Windows::Forms::Label());
			this->label21 = (gcnew System::Windows::Forms::Label());
			this->label_CalPara_twopoints = (gcnew System::Windows::Forms::Label());
			this->toggleButtons_CalPara_twopoints = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->button_CalPara_right = (gcnew System::Windows::Forms::Button());
			this->button_CalPara_left = (gcnew System::Windows::Forms::Button());
			this->textBox_CalPara_locy = (gcnew System::Windows::Forms::TextBox());
			this->button_CalPara_down = (gcnew System::Windows::Forms::Button());
			this->textBox_CalPara_locx = (gcnew System::Windows::Forms::TextBox());
			this->button_CalPara_up = (gcnew System::Windows::Forms::Button());
			this->label30 = (gcnew System::Windows::Forms::Label());
			this->textBox_CalPara_ypixel = (gcnew System::Windows::Forms::TextBox());
			this->label29 = (gcnew System::Windows::Forms::Label());
			this->textBox_CalPara_xpixel = (gcnew System::Windows::Forms::TextBox());
			this->label28 = (gcnew System::Windows::Forms::Label());
			this->label26 = (gcnew System::Windows::Forms::Label());
			this->label27 = (gcnew System::Windows::Forms::Label());
			this->label25 = (gcnew System::Windows::Forms::Label());
			this->button_CalPara_calculate = (gcnew System::Windows::Forms::Button());
			this->comboBox_CalPara_samplingrate = (gcnew System::Windows::Forms::ComboBox());
			this->label19 = (gcnew System::Windows::Forms::Label());
			this->textBox_CalPara_LOADRASTER = (gcnew System::Windows::Forms::TextBox());
			this->button_CalPara_LOADRASTER = (gcnew System::Windows::Forms::Button());
			this->panel5 = (gcnew System::Windows::Forms::Panel());
			this->tableLayoutPanel_CalPara_result = (gcnew System::Windows::Forms::TableLayoutPanel());
			this->label23 = (gcnew System::Windows::Forms::Label());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
			this->label_TRANSFERTOTIFF = (gcnew System::Windows::Forms::Label());
			this->button_LOADLISSAJOUS_RAWBIN = (gcnew System::Windows::Forms::Button());
			this->button_TRANSFERTOTIFF = (gcnew System::Windows::Forms::Button());
			this->textBox_LOADLISSAJOUSBIN = (gcnew System::Windows::Forms::TextBox());
			this->label41 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->tabPage1 = (gcnew System::Windows::Forms::TabPage());
			this->tabPage5 = (gcnew System::Windows::Forms::TabPage());
			this->label38 = (gcnew System::Windows::Forms::Label());
			this->label_ExtractKymograph_choosedisplay = (gcnew System::Windows::Forms::Label());
			this->toggleButtons_ExtractKymograph_choosedisplay = (gcnew CustomControls::DefineControls::ToggleButtons());
			this->button_ExtractKymograph_quickset = (gcnew System::Windows::Forms::Button());
			this->label36 = (gcnew System::Windows::Forms::Label());
			this->label37 = (gcnew System::Windows::Forms::Label());
			this->button_ExtractKymograph_extractkymograph = (gcnew System::Windows::Forms::Button());
			this->button_LOADLISSAJOUS = (gcnew System::Windows::Forms::Button());
			this->textBox_LOADLISSAJOUS_EK = (gcnew System::Windows::Forms::TextBox());
			this->textBox_ExtractKymograph_endpixel = (gcnew System::Windows::Forms::TextBox());
			this->textBox_ExtractKymograph_startpixel = (gcnew System::Windows::Forms::TextBox());
			this->trackBar_ExtractKymograph_endpixel = (gcnew System::Windows::Forms::TrackBar());
			this->trackBar_ExtractKymograph_startpixel = (gcnew System::Windows::Forms::TrackBar());
			this->label32 = (gcnew System::Windows::Forms::Label());
			this->label35 = (gcnew System::Windows::Forms::Label());
			this->textBox_LOADLISSAJOUSFOLDER_EK = (gcnew System::Windows::Forms::TextBox());
			this->button_LOADRASTER2 = (gcnew System::Windows::Forms::Button());
			this->textBox_LOADRASTER_EK = (gcnew System::Windows::Forms::TextBox());
			this->button_LOADLISSAJOUSFOLDER2 = (gcnew System::Windows::Forms::Button());
			this->tabControl2 = (gcnew System::Windows::Forms::TabControl());
			this->tabPage3 = (gcnew System::Windows::Forms::TabPage());
			this->label39 = (gcnew System::Windows::Forms::Label());
			this->tabPage4 = (gcnew System::Windows::Forms::TabPage());
			this->label40 = (gcnew System::Windows::Forms::Label());
			this->comboBox_ScanPreview_xyfreq = (gcnew System::Windows::Forms::ComboBox());
			this->button_ScanPreview_rightphase = (gcnew System::Windows::Forms::Button());
			this->button_ScanPreview_leftphase = (gcnew System::Windows::Forms::Button());
			this->textBox_ScanPreview_framerate = (gcnew System::Windows::Forms::TextBox());
			this->label34 = (gcnew System::Windows::Forms::Label());
			this->textBox_ScanPreview_phase = (gcnew System::Windows::Forms::TextBox());
			this->label31 = (gcnew System::Windows::Forms::Label());
			this->label33 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->comboBox_ScanPreview_samplingrate = (gcnew System::Windows::Forms::ComboBox());
			this->label9 = (gcnew System::Windows::Forms::Label());
			this->label24 = (gcnew System::Windows::Forms::Label());
			this->textBox_ScanPreview_xpixel = (gcnew System::Windows::Forms::TextBox());
			this->textBox_ScanPreview_ypixel = (gcnew System::Windows::Forms::TextBox());
			this->button_ScanPreview_LOADRASTER = (gcnew System::Windows::Forms::Button());
			this->textBox_ScanPreview_LOADRASTER = (gcnew System::Windows::Forms::TextBox());
			this->button_shut_down = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_lissajousoffset))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_resizey))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_resizex))->BeginInit();
			this->panel3->SuspendLayout();
			this->panel5->SuspendLayout();
			this->tabControl1->SuspendLayout();
			this->tabPage2->SuspendLayout();
			this->tabPage1->SuspendLayout();
			this->tabPage5->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_ExtractKymograph_endpixel))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_ExtractKymograph_startpixel))->BeginInit();
			this->tabControl2->SuspendLayout();
			this->tabPage3->SuspendLayout();
			this->tabPage4->SuspendLayout();
			this->SuspendLayout();
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 24.75F, System::Drawing::FontStyle::Bold));
			this->label1->Location = System::Drawing::Point(12, 9);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(651, 57);
			this->label1->TabIndex = 2;
			this->label1->Text = L"NTU miniScope Post Processing";
			// 
			// checkBox_MergeScantrace
			// 
			this->checkBox_MergeScantrace->AutoSize = true;
			this->checkBox_MergeScantrace->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->checkBox_MergeScantrace->Location = System::Drawing::Point(24, 150);
			this->checkBox_MergeScantrace->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->checkBox_MergeScantrace->Name = L"checkBox_MergeScantrace";
			this->checkBox_MergeScantrace->Size = System::Drawing::Size(185, 29);
			this->checkBox_MergeScantrace->TabIndex = 5;
			this->checkBox_MergeScantrace->Text = L"Merge scan trace";
			this->checkBox_MergeScantrace->UseVisualStyleBackColor = true;
			this->checkBox_MergeScantrace->CheckedChanged += gcnew System::EventHandler(this, &YSPostprocessing::checkBox_MergeScantrace_CheckedChanged);
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label5->Location = System::Drawing::Point(19, 292);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(82, 25);
			this->label5->TabIndex = 7;
			this->label5->Text = L"Offset X";
			// 
			// label6
			// 
			this->label6->AutoSize = true;
			this->label6->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label6->Location = System::Drawing::Point(20, 331);
			this->label6->Name = L"label6";
			this->label6->Size = System::Drawing::Size(82, 25);
			this->label6->TabIndex = 8;
			this->label6->Text = L"Offset Y";
			// 
			// textBox_offsetx
			// 
			this->textBox_offsetx->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_offsetx->Location = System::Drawing::Point(128, 289);
			this->textBox_offsetx->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_offsetx->Name = L"textBox_offsetx";
			this->textBox_offsetx->Size = System::Drawing::Size(87, 32);
			this->textBox_offsetx->TabIndex = 9;
			this->textBox_offsetx->Text = L"0";
			this->textBox_offsetx->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_offsety
			// 
			this->textBox_offsety->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_offsety->Location = System::Drawing::Point(128, 327);
			this->textBox_offsety->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_offsety->Name = L"textBox_offsety";
			this->textBox_offsety->Size = System::Drawing::Size(87, 32);
			this->textBox_offsety->TabIndex = 10;
			this->textBox_offsety->Text = L"0";
			this->textBox_offsety->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label7->Location = System::Drawing::Point(13, 377);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(255, 25);
			this->label7->TabIndex = 12;
			this->label7->Text = L"2D Lissajous image queue : ";
			// 
			// checkBox_save
			// 
			this->checkBox_save->AutoSize = true;
			this->checkBox_save->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->checkBox_save->Location = System::Drawing::Point(335, 377);
			this->checkBox_save->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->checkBox_save->Name = L"checkBox_save";
			this->checkBox_save->Size = System::Drawing::Size(75, 29);
			this->checkBox_save->TabIndex = 13;
			this->checkBox_save->Text = L"Save";
			this->checkBox_save->UseVisualStyleBackColor = true;
			// 
			// button_Merge_and_Display
			// 
			this->button_Merge_and_Display->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_Merge_and_Display->Location = System::Drawing::Point(19, 407);
			this->button_Merge_and_Display->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_Merge_and_Display->Name = L"button_Merge_and_Display";
			this->button_Merge_and_Display->Size = System::Drawing::Size(229, 34);
			this->button_Merge_and_Display->TabIndex = 14;
			this->button_Merge_and_Display->Text = L"Merge and Display";
			this->button_Merge_and_Display->UseVisualStyleBackColor = true;
			this->button_Merge_and_Display->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_Merge_and_Display_Click);
			// 
			// button_up
			// 
			this->button_up->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_up.Image")));
			this->button_up->Location = System::Drawing::Point(321, 263);
			this->button_up->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_up->Name = L"button_up";
			this->button_up->Size = System::Drawing::Size(43, 28);
			this->button_up->TabIndex = 15;
			this->button_up->UseVisualStyleBackColor = true;
			this->button_up->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_up_Click);
			// 
			// button_down
			// 
			this->button_down->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_down.Image")));
			this->button_down->Location = System::Drawing::Point(321, 331);
			this->button_down->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_down->Name = L"button_down";
			this->button_down->Size = System::Drawing::Size(43, 28);
			this->button_down->TabIndex = 16;
			this->button_down->UseVisualStyleBackColor = true;
			this->button_down->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_down_Click);
			// 
			// button_left
			// 
			this->button_left->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_left.Image")));
			this->button_left->Location = System::Drawing::Point(283, 295);
			this->button_left->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_left->Name = L"button_left";
			this->button_left->Size = System::Drawing::Size(31, 39);
			this->button_left->TabIndex = 17;
			this->button_left->UseVisualStyleBackColor = true;
			this->button_left->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_left_Click);
			// 
			// button_right
			// 
			this->button_right->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_right.Image")));
			this->button_right->Location = System::Drawing::Point(369, 295);
			this->button_right->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_right->Name = L"button_right";
			this->button_right->Size = System::Drawing::Size(31, 39);
			this->button_right->TabIndex = 18;
			this->button_right->UseVisualStyleBackColor = true;
			this->button_right->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_right_Click);
			// 
			// LOAD_RASTER
			// 
			this->LOAD_RASTER->FileName = L"openFileDialog1";
			this->LOAD_RASTER->Filter = L" \"Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff)|*.png;*.jpg;*.jpeg;*.bmp;*."
				L"tif;*.tiff|All Files (*.*)|*.*\"";
			this->LOAD_RASTER->InitialDirectory = L"C:\\NTU_miniScope_Imaging\\2DRASTER";
			// 
			// button_LOADRASTER
			// 
			this->button_LOADRASTER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADRASTER->Location = System::Drawing::Point(13, 12);
			this->button_LOADRASTER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADRASTER->Name = L"button_LOADRASTER";
			this->button_LOADRASTER->Size = System::Drawing::Size(184, 62);
			this->button_LOADRASTER->TabIndex = 19;
			this->button_LOADRASTER->Text = L"Load RASTER";
			this->button_LOADRASTER->UseVisualStyleBackColor = true;
			this->button_LOADRASTER->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADRASTER_Click);
			// 
			// textBox_LOADRASTER
			// 
			this->textBox_LOADRASTER->Enabled = false;
			this->textBox_LOADRASTER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_LOADRASTER->Location = System::Drawing::Point(203, 12);
			this->textBox_LOADRASTER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADRASTER->Multiline = true;
			this->textBox_LOADRASTER->Name = L"textBox_LOADRASTER";
			this->textBox_LOADRASTER->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADRASTER->Size = System::Drawing::Size(447, 54);
			this->textBox_LOADRASTER->TabIndex = 20;
			// 
			// button_LOADLISSAJOUSFOLDER
			// 
			this->button_LOADLISSAJOUSFOLDER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADLISSAJOUSFOLDER->Location = System::Drawing::Point(13, 78);
			this->button_LOADLISSAJOUSFOLDER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADLISSAJOUSFOLDER->Name = L"button_LOADLISSAJOUSFOLDER";
			this->button_LOADLISSAJOUSFOLDER->Size = System::Drawing::Size(184, 62);
			this->button_LOADLISSAJOUSFOLDER->TabIndex = 21;
			this->button_LOADLISSAJOUSFOLDER->Text = L"Load LISSAJOUS FOLDER";
			this->button_LOADLISSAJOUSFOLDER->UseVisualStyleBackColor = true;
			this->button_LOADLISSAJOUSFOLDER->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADLISSAJOUSFOLDER_Click);
			// 
			// textBox_LOADLISSAJOUSFOLDER
			// 
			this->textBox_LOADLISSAJOUSFOLDER->Enabled = false;
			this->textBox_LOADLISSAJOUSFOLDER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_LOADLISSAJOUSFOLDER->Location = System::Drawing::Point(203, 78);
			this->textBox_LOADLISSAJOUSFOLDER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADLISSAJOUSFOLDER->Multiline = true;
			this->textBox_LOADLISSAJOUSFOLDER->Name = L"textBox_LOADLISSAJOUSFOLDER";
			this->textBox_LOADLISSAJOUSFOLDER->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADLISSAJOUSFOLDER->Size = System::Drawing::Size(447, 61);
			this->textBox_LOADLISSAJOUSFOLDER->TabIndex = 22;
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label8->Location = System::Drawing::Point(15, 361);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(65, 25);
			this->label8->TabIndex = 23;
			this->label8->Text = L"Offset";
			// 
			// textBox_LOADLISSAJOUS
			// 
			this->textBox_LOADLISSAJOUS->Enabled = false;
			this->textBox_LOADLISSAJOUS->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBox_LOADLISSAJOUS->Location = System::Drawing::Point(203, 224);
			this->textBox_LOADLISSAJOUS->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADLISSAJOUS->Multiline = true;
			this->textBox_LOADLISSAJOUS->Name = L"textBox_LOADLISSAJOUS";
			this->textBox_LOADLISSAJOUS->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADLISSAJOUS->Size = System::Drawing::Size(443, 54);
			this->textBox_LOADLISSAJOUS->TabIndex = 25;
			// 
			// button_LOADLISSAJOUS_RAW
			// 
			this->button_LOADLISSAJOUS_RAW->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADLISSAJOUS_RAW->Location = System::Drawing::Point(13, 222);
			this->button_LOADLISSAJOUS_RAW->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADLISSAJOUS_RAW->Name = L"button_LOADLISSAJOUS_RAW";
			this->button_LOADLISSAJOUS_RAW->Size = System::Drawing::Size(184, 62);
			this->button_LOADLISSAJOUS_RAW->TabIndex = 24;
			this->button_LOADLISSAJOUS_RAW->Text = L"Load LISSAJOUS RAW";
			this->button_LOADLISSAJOUS_RAW->UseVisualStyleBackColor = true;
			this->button_LOADLISSAJOUS_RAW->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADLISSAJOUS_Click);
			// 
			// LOAD_LISSAJOUS
			// 
			this->LOAD_LISSAJOUS->FileName = L"openFileDialog1";
			this->LOAD_LISSAJOUS->Filter = L" \"Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff)|*.png;*.jpg;*.jpeg;*.bmp;*."
				L"tif;*.tiff|All Files (*.*)|*.*\"";
			this->LOAD_LISSAJOUS->InitialDirectory = L"C:\\NTU_miniScope_Imaging";
			// 
			// textBox__LOADLISSAJOUSFOLDER2
			// 
			this->textBox__LOADLISSAJOUSFOLDER2->Enabled = false;
			this->textBox__LOADLISSAJOUSFOLDER2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBox__LOADLISSAJOUSFOLDER2->Location = System::Drawing::Point(203, 290);
			this->textBox__LOADLISSAJOUSFOLDER2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox__LOADLISSAJOUSFOLDER2->Multiline = true;
			this->textBox__LOADLISSAJOUSFOLDER2->Name = L"textBox__LOADLISSAJOUSFOLDER2";
			this->textBox__LOADLISSAJOUSFOLDER2->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox__LOADLISSAJOUSFOLDER2->Size = System::Drawing::Size(443, 56);
			this->textBox__LOADLISSAJOUSFOLDER2->TabIndex = 27;
			// 
			// button_LOADLISSAJOUSRAWFOLDER
			// 
			this->button_LOADLISSAJOUSRAWFOLDER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADLISSAJOUSRAWFOLDER->Location = System::Drawing::Point(13, 286);
			this->button_LOADLISSAJOUSRAWFOLDER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADLISSAJOUSRAWFOLDER->Name = L"button_LOADLISSAJOUSRAWFOLDER";
			this->button_LOADLISSAJOUSRAWFOLDER->Size = System::Drawing::Size(184, 62);
			this->button_LOADLISSAJOUSRAWFOLDER->TabIndex = 26;
			this->button_LOADLISSAJOUSRAWFOLDER->Text = L"Load LISSAJOUS RAW FOLDER";
			this->button_LOADLISSAJOUSRAWFOLDER->UseVisualStyleBackColor = true;
			this->button_LOADLISSAJOUSRAWFOLDER->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADLISSAJOUSFOLDER2_Click);
			// 
			// button_setphaseandsave
			// 
			this->button_setphaseandsave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_setphaseandsave->Location = System::Drawing::Point(15, 407);
			this->button_setphaseandsave->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_setphaseandsave->Name = L"button_setphaseandsave";
			this->button_setphaseandsave->Size = System::Drawing::Size(223, 41);
			this->button_setphaseandsave->TabIndex = 28;
			this->button_setphaseandsave->Text = L"Phase Offset and Save";
			this->button_setphaseandsave->UseVisualStyleBackColor = true;
			this->button_setphaseandsave->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_setphaseandsave_Click);
			// 
			// trackBar_lissajousoffset
			// 
			this->trackBar_lissajousoffset->AutoSize = false;
			this->trackBar_lissajousoffset->BackColor = System::Drawing::Color::LightSteelBlue;
			this->trackBar_lissajousoffset->LargeChange = 100;
			this->trackBar_lissajousoffset->Location = System::Drawing::Point(91, 361);
			this->trackBar_lissajousoffset->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->trackBar_lissajousoffset->Maximum = 100000;
			this->trackBar_lissajousoffset->Name = L"trackBar_lissajousoffset";
			this->trackBar_lissajousoffset->Size = System::Drawing::Size(492, 30);
			this->trackBar_lissajousoffset->TabIndex = 99;
			this->trackBar_lissajousoffset->TickFrequency = 5;
			this->trackBar_lissajousoffset->Scroll += gcnew System::EventHandler(this, &YSPostprocessing::trackBar_lissajousoffset_Scroll);
			// 
			// LOAD_LISSAJOUS_FOLDER
			// 
			this->LOAD_LISSAJOUS_FOLDER->SelectedPath = L"C:\\NTU_miniScope_Imaging\\2DLISSAJOUS";
			// 
			// label_lissajousoffset
			// 
			this->label_lissajousoffset->AutoSize = true;
			this->label_lissajousoffset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_lissajousoffset->Location = System::Drawing::Point(589, 361);
			this->label_lissajousoffset->Name = L"label_lissajousoffset";
			this->label_lissajousoffset->Size = System::Drawing::Size(23, 25);
			this->label_lissajousoffset->TabIndex = 100;
			this->label_lissajousoffset->Text = L"0";
			// 
			// label_reconstruct
			// 
			this->label_reconstruct->AutoSize = true;
			this->label_reconstruct->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_reconstruct->Location = System::Drawing::Point(615, 414);
			this->label_reconstruct->Name = L"label_reconstruct";
			this->label_reconstruct->Size = System::Drawing::Size(23, 25);
			this->label_reconstruct->TabIndex = 105;
			this->label_reconstruct->Text = L"0";
			// 
			// progressBar_reconstruct
			// 
			this->progressBar_reconstruct->ForeColor = System::Drawing::Color::LimeGreen;
			this->progressBar_reconstruct->Location = System::Drawing::Point(245, 414);
			this->progressBar_reconstruct->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->progressBar_reconstruct->Name = L"progressBar_reconstruct";
			this->progressBar_reconstruct->Size = System::Drawing::Size(367, 28);
			this->progressBar_reconstruct->Step = 5;
			this->progressBar_reconstruct->TabIndex = 104;
			// 
			// textBox_lissajouspixelnum
			// 
			this->textBox_lissajouspixelnum->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_lissajouspixelnum->Location = System::Drawing::Point(247, 185);
			this->textBox_lissajouspixelnum->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_lissajouspixelnum->Name = L"textBox_lissajouspixelnum";
			this->textBox_lissajouspixelnum->Size = System::Drawing::Size(131, 32);
			this->textBox_lissajouspixelnum->TabIndex = 103;
			this->textBox_lissajouspixelnum->Text = L"512";
			this->textBox_lissajouspixelnum->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			this->textBox_lissajouspixelnum->TextChanged += gcnew System::EventHandler(this, &YSPostprocessing::textBox_lissajouspixelnum_TextChanged);
			// 
			// label16
			// 
			this->label16->AutoSize = true;
			this->label16->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label16->Location = System::Drawing::Point(19, 190);
			this->label16->Name = L"label16";
			this->label16->Size = System::Drawing::Size(194, 25);
			this->label16->TabIndex = 102;
			this->label16->Text = L"Image Pixel Number";
			// 
			// label_scanmodemerge
			// 
			this->label_scanmodemerge->AutoSize = true;
			this->label_scanmodemerge->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_scanmodemerge->Location = System::Drawing::Point(597, 412);
			this->label_scanmodemerge->Name = L"label_scanmodemerge";
			this->label_scanmodemerge->Size = System::Drawing::Size(23, 25);
			this->label_scanmodemerge->TabIndex = 109;
			this->label_scanmodemerge->Text = L"0";
			// 
			// progressBar_scanmodemerge
			// 
			this->progressBar_scanmodemerge->ForeColor = System::Drawing::Color::LimeGreen;
			this->progressBar_scanmodemerge->Location = System::Drawing::Point(279, 412);
			this->progressBar_scanmodemerge->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->progressBar_scanmodemerge->Name = L"progressBar_scanmodemerge";
			this->progressBar_scanmodemerge->Size = System::Drawing::Size(312, 28);
			this->progressBar_scanmodemerge->Step = 5;
			this->progressBar_scanmodemerge->TabIndex = 108;
			// 
			// textBox_resizey
			// 
			this->textBox_resizey->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_resizey->Location = System::Drawing::Point(547, 225);
			this->textBox_resizey->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_resizey->Name = L"textBox_resizey";
			this->textBox_resizey->Size = System::Drawing::Size(71, 32);
			this->textBox_resizey->TabIndex = 107;
			this->textBox_resizey->Text = L"100";
			this->textBox_resizey->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_resizex
			// 
			this->textBox_resizex->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_resizex->Location = System::Drawing::Point(547, 190);
			this->textBox_resizex->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_resizex->Name = L"textBox_resizex";
			this->textBox_resizex->Size = System::Drawing::Size(71, 32);
			this->textBox_resizex->TabIndex = 106;
			this->textBox_resizex->Text = L"100";
			this->textBox_resizex->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_fastset
			// 
			this->button_fastset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_fastset->Location = System::Drawing::Point(447, 305);
			this->button_fastset->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_fastset->Name = L"button_fastset";
			this->button_fastset->Size = System::Drawing::Size(191, 54);
			this->button_fastset->TabIndex = 105;
			this->button_fastset->Text = L"Quick set";
			this->button_fastset->UseVisualStyleBackColor = true;
			this->button_fastset->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_fastset_Click);
			// 
			// label_resizey
			// 
			this->label_resizey->AutoSize = true;
			this->label_resizey->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_resizey->Location = System::Drawing::Point(619, 225);
			this->label_resizey->Name = L"label_resizey";
			this->label_resizey->Size = System::Drawing::Size(28, 25);
			this->label_resizey->TabIndex = 104;
			this->label_resizey->Text = L"%";
			// 
			// trackBar_resizey
			// 
			this->trackBar_resizey->AutoSize = false;
			this->trackBar_resizey->BackColor = System::Drawing::Color::LightSteelBlue;
			this->trackBar_resizey->Location = System::Drawing::Point(132, 224);
			this->trackBar_resizey->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->trackBar_resizey->Maximum = 400;
			this->trackBar_resizey->Name = L"trackBar_resizey";
			this->trackBar_resizey->Size = System::Drawing::Size(415, 30);
			this->trackBar_resizey->TabIndex = 103;
			this->trackBar_resizey->TickFrequency = 5;
			this->trackBar_resizey->Value = 100;
			this->trackBar_resizey->Scroll += gcnew System::EventHandler(this, &YSPostprocessing::trackBar_resizey_Scroll);
			// 
			// label_resizex
			// 
			this->label_resizex->AutoSize = true;
			this->label_resizex->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_resizex->Location = System::Drawing::Point(619, 191);
			this->label_resizex->Name = L"label_resizex";
			this->label_resizex->Size = System::Drawing::Size(28, 25);
			this->label_resizex->TabIndex = 102;
			this->label_resizex->Text = L"%";
			// 
			// trackBar_resizex
			// 
			this->trackBar_resizex->AutoSize = false;
			this->trackBar_resizex->BackColor = System::Drawing::Color::LightSteelBlue;
			this->trackBar_resizex->Location = System::Drawing::Point(132, 190);
			this->trackBar_resizex->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->trackBar_resizex->Maximum = 400;
			this->trackBar_resizex->Name = L"trackBar_resizex";
			this->trackBar_resizex->Size = System::Drawing::Size(415, 30);
			this->trackBar_resizex->TabIndex = 101;
			this->trackBar_resizex->TickFrequency = 5;
			this->trackBar_resizex->Value = 100;
			this->trackBar_resizex->Scroll += gcnew System::EventHandler(this, &YSPostprocessing::trackBar_resizex_Scroll);
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label4->Location = System::Drawing::Point(17, 191);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(83, 25);
			this->label4->TabIndex = 23;
			this->label4->Text = L"Resize X";
			// 
			// label17
			// 
			this->label17->AutoSize = true;
			this->label17->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label17->Location = System::Drawing::Point(19, 230);
			this->label17->Name = L"label17";
			this->label17->Size = System::Drawing::Size(83, 25);
			this->label17->TabIndex = 24;
			this->label17->Text = L"Resize Y";
			// 
			// panel3
			// 
			this->panel3->BackColor = System::Drawing::Color::LightSteelBlue;
			this->panel3->Controls->Add(this->textBox_lissajousphase);
			this->panel3->Controls->Add(this->label15);
			this->panel3->Controls->Add(this->textBox_samplingrate);
			this->panel3->Controls->Add(this->label14);
			this->panel3->Controls->Add(this->textBox_numofsample);
			this->panel3->Controls->Add(this->label10);
			this->panel3->Controls->Add(this->textBox_lissajousfreqy);
			this->panel3->Controls->Add(this->textBox_lissajousfreqx);
			this->panel3->Controls->Add(this->label13);
			this->panel3->Controls->Add(this->label12);
			this->panel3->Controls->Add(this->label11);
			this->panel3->Location = System::Drawing::Point(699, 393);
			this->panel3->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->panel3->Name = L"panel3";
			this->panel3->Size = System::Drawing::Size(529, 246);
			this->panel3->TabIndex = 105;
			// 
			// textBox_lissajousphase
			// 
			this->textBox_lissajousphase->Enabled = false;
			this->textBox_lissajousphase->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_lissajousphase->Location = System::Drawing::Point(244, 123);
			this->textBox_lissajousphase->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_lissajousphase->Name = L"textBox_lissajousphase";
			this->textBox_lissajousphase->Size = System::Drawing::Size(131, 32);
			this->textBox_lissajousphase->TabIndex = 17;
			this->textBox_lissajousphase->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label15
			// 
			this->label15->AutoSize = true;
			this->label15->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label15->Location = System::Drawing::Point(17, 127);
			this->label15->Name = L"label15";
			this->label15->Size = System::Drawing::Size(63, 25);
			this->label15->TabIndex = 16;
			this->label15->Text = L"Phase";
			// 
			// textBox_samplingrate
			// 
			this->textBox_samplingrate->Enabled = false;
			this->textBox_samplingrate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_samplingrate->Location = System::Drawing::Point(244, 162);
			this->textBox_samplingrate->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_samplingrate->Name = L"textBox_samplingrate";
			this->textBox_samplingrate->Size = System::Drawing::Size(131, 32);
			this->textBox_samplingrate->TabIndex = 15;
			this->textBox_samplingrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label14
			// 
			this->label14->AutoSize = true;
			this->label14->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label14->Location = System::Drawing::Point(17, 166);
			this->label14->Name = L"label14";
			this->label14->Size = System::Drawing::Size(183, 25);
			this->label14->TabIndex = 14;
			this->label14->Text = L"Sampling Rate (Hz)";
			// 
			// textBox_numofsample
			// 
			this->textBox_numofsample->Enabled = false;
			this->textBox_numofsample->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_numofsample->Location = System::Drawing::Point(244, 199);
			this->textBox_numofsample->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_numofsample->Name = L"textBox_numofsample";
			this->textBox_numofsample->Size = System::Drawing::Size(131, 32);
			this->textBox_numofsample->TabIndex = 13;
			this->textBox_numofsample->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label10->Location = System::Drawing::Point(17, 202);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(147, 25);
			this->label10->TabIndex = 12;
			this->label10->Text = L"Num of sample";
			// 
			// textBox_lissajousfreqy
			// 
			this->textBox_lissajousfreqy->Enabled = false;
			this->textBox_lissajousfreqy->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_lissajousfreqy->Location = System::Drawing::Point(244, 86);
			this->textBox_lissajousfreqy->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_lissajousfreqy->Name = L"textBox_lissajousfreqy";
			this->textBox_lissajousfreqy->Size = System::Drawing::Size(131, 32);
			this->textBox_lissajousfreqy->TabIndex = 11;
			this->textBox_lissajousfreqy->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_lissajousfreqx
			// 
			this->textBox_lissajousfreqx->Enabled = false;
			this->textBox_lissajousfreqx->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_lissajousfreqx->Location = System::Drawing::Point(244, 46);
			this->textBox_lissajousfreqx->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_lissajousfreqx->Name = L"textBox_lissajousfreqx";
			this->textBox_lissajousfreqx->Size = System::Drawing::Size(131, 32);
			this->textBox_lissajousfreqx->TabIndex = 10;
			this->textBox_lissajousfreqx->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label13->Location = System::Drawing::Point(17, 90);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(112, 25);
			this->label13->TabIndex = 9;
			this->label13->Text = L"Freq Y (Hz)";
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label12->Location = System::Drawing::Point(17, 49);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(112, 25);
			this->label12->TabIndex = 8;
			this->label12->Text = L"Freq X (Hz)";
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->BackColor = System::Drawing::Color::RoyalBlue;
			this->label11->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->label11->ForeColor = System::Drawing::SystemColors::ControlLightLight;
			this->label11->Location = System::Drawing::Point(17, 11);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(281, 28);
			this->label11->TabIndex = 4;
			this->label11->Text = L"Data Info of Lissajous image";
			// 
			// timer_refresh_UI
			// 
			this->timer_refresh_UI->Interval = 10;
			this->timer_refresh_UI->Tick += gcnew System::EventHandler(this, &YSPostprocessing::timer_refresh_UI_Tick);
			// 
			// LOAD_LISSAJOUSRAW_FOLDER
			// 
			this->LOAD_LISSAJOUSRAW_FOLDER->SelectedPath = L"C:\\NTU_miniScope_Imaging\\2DLISSAJOUS_RAW";
			// 
			// checkBox_CalPara_linepoints
			// 
			this->checkBox_CalPara_linepoints->AutoSize = true;
			this->checkBox_CalPara_linepoints->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->checkBox_CalPara_linepoints->Location = System::Drawing::Point(291, 140);
			this->checkBox_CalPara_linepoints->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->checkBox_CalPara_linepoints->Name = L"checkBox_CalPara_linepoints";
			this->checkBox_CalPara_linepoints->Size = System::Drawing::Size(174, 29);
			this->checkBox_CalPara_linepoints->TabIndex = 60;
			this->checkBox_CalPara_linepoints->Text = L"Points Calculate";
			this->checkBox_CalPara_linepoints->UseVisualStyleBackColor = true;
			this->checkBox_CalPara_linepoints->CheckedChanged += gcnew System::EventHandler(this, &YSPostprocessing::checkBox_CalPara_linepoints_CheckedChanged);
			// 
			// label22
			// 
			this->label22->AutoSize = true;
			this->label22->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label22->Location = System::Drawing::Point(23, 213);
			this->label22->Name = L"label22";
			this->label22->Size = System::Drawing::Size(108, 25);
			this->label22->TabIndex = 57;
			this->label22->Text = L"2nd Target";
			// 
			// textBox_CalPara_locy2
			// 
			this->textBox_CalPara_locy2->Enabled = false;
			this->textBox_CalPara_locy2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_locy2->Location = System::Drawing::Point(260, 213);
			this->textBox_CalPara_locy2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_locy2->Name = L"textBox_CalPara_locy2";
			this->textBox_CalPara_locy2->Size = System::Drawing::Size(71, 32);
			this->textBox_CalPara_locy2->TabIndex = 56;
			this->textBox_CalPara_locy2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_CalPara_locx2
			// 
			this->textBox_CalPara_locx2->Enabled = false;
			this->textBox_CalPara_locx2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_locx2->Location = System::Drawing::Point(160, 213);
			this->textBox_CalPara_locx2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_locx2->Name = L"textBox_CalPara_locx2";
			this->textBox_CalPara_locx2->Size = System::Drawing::Size(71, 32);
			this->textBox_CalPara_locx2->TabIndex = 55;
			this->textBox_CalPara_locx2->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label18
			// 
			this->label18->AutoSize = true;
			this->label18->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label18->Location = System::Drawing::Point(236, 213);
			this->label18->Name = L"label18";
			this->label18->Size = System::Drawing::Size(17, 25);
			this->label18->TabIndex = 54;
			this->label18->Text = L",";
			// 
			// label20
			// 
			this->label20->AutoSize = true;
			this->label20->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label20->Location = System::Drawing::Point(336, 213);
			this->label20->Name = L"label20";
			this->label20->Size = System::Drawing::Size(19, 25);
			this->label20->TabIndex = 53;
			this->label20->Text = L")";
			// 
			// label21
			// 
			this->label21->AutoSize = true;
			this->label21->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label21->Location = System::Drawing::Point(136, 213);
			this->label21->Name = L"label21";
			this->label21->Size = System::Drawing::Size(19, 25);
			this->label21->TabIndex = 52;
			this->label21->Text = L"(";
			// 
			// label_CalPara_twopoints
			// 
			this->label_CalPara_twopoints->AutoSize = true;
			this->label_CalPara_twopoints->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_CalPara_twopoints->Location = System::Drawing::Point(79, 145);
			this->label_CalPara_twopoints->Name = L"label_CalPara_twopoints";
			this->label_CalPara_twopoints->Size = System::Drawing::Size(161, 25);
			this->label_CalPara_twopoints->TabIndex = 51;
			this->label_CalPara_twopoints->Text = L"Calculate 1 point";
			// 
			// toggleButtons_CalPara_twopoints
			// 
			this->toggleButtons_CalPara_twopoints->BackColor = System::Drawing::Color::MediumTurquoise;
			this->toggleButtons_CalPara_twopoints->Location = System::Drawing::Point(24, 142);
			this->toggleButtons_CalPara_twopoints->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->toggleButtons_CalPara_twopoints->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_CalPara_twopoints->Name = L"toggleButtons_CalPara_twopoints";
			this->toggleButtons_CalPara_twopoints->OffBackColor = System::Drawing::Color::DarkSlateGray;
			this->toggleButtons_CalPara_twopoints->OffToggleColor = System::Drawing::Color::DarkSlateGray;
			this->toggleButtons_CalPara_twopoints->OnBackColor = System::Drawing::Color::DarkSlateGray;
			this->toggleButtons_CalPara_twopoints->OnToggleColor = System::Drawing::Color::LightSeaGreen;
			this->toggleButtons_CalPara_twopoints->Size = System::Drawing::Size(48, 27);
			this->toggleButtons_CalPara_twopoints->SolidStyle = false;
			this->toggleButtons_CalPara_twopoints->TabIndex = 50;
			this->toggleButtons_CalPara_twopoints->UseVisualStyleBackColor = false;
			this->toggleButtons_CalPara_twopoints->CheckedChanged += gcnew System::EventHandler(this, &YSPostprocessing::toggleButtons_CalPara_twopoints_CheckedChanged);
			// 
			// button_CalPara_right
			// 
			this->button_CalPara_right->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_CalPara_right.Image")));
			this->button_CalPara_right->Location = System::Drawing::Point(627, 105);
			this->button_CalPara_right->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_CalPara_right->Name = L"button_CalPara_right";
			this->button_CalPara_right->Size = System::Drawing::Size(31, 39);
			this->button_CalPara_right->TabIndex = 44;
			this->button_CalPara_right->UseVisualStyleBackColor = true;
			this->button_CalPara_right->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_CalPara_right_Click);
			// 
			// button_CalPara_left
			// 
			this->button_CalPara_left->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_CalPara_left.Image")));
			this->button_CalPara_left->Location = System::Drawing::Point(540, 105);
			this->button_CalPara_left->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_CalPara_left->Name = L"button_CalPara_left";
			this->button_CalPara_left->Size = System::Drawing::Size(31, 39);
			this->button_CalPara_left->TabIndex = 43;
			this->button_CalPara_left->UseVisualStyleBackColor = true;
			this->button_CalPara_left->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_CalPara_left_Click);
			// 
			// textBox_CalPara_locy
			// 
			this->textBox_CalPara_locy->Enabled = false;
			this->textBox_CalPara_locy->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_locy->Location = System::Drawing::Point(260, 178);
			this->textBox_CalPara_locy->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_locy->Name = L"textBox_CalPara_locy";
			this->textBox_CalPara_locy->Size = System::Drawing::Size(71, 32);
			this->textBox_CalPara_locy->TabIndex = 49;
			this->textBox_CalPara_locy->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_CalPara_down
			// 
			this->button_CalPara_down->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_CalPara_down.Image")));
			this->button_CalPara_down->Location = System::Drawing::Point(579, 140);
			this->button_CalPara_down->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_CalPara_down->Name = L"button_CalPara_down";
			this->button_CalPara_down->Size = System::Drawing::Size(43, 28);
			this->button_CalPara_down->TabIndex = 42;
			this->button_CalPara_down->UseVisualStyleBackColor = true;
			this->button_CalPara_down->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_CalPara_down_Click);
			// 
			// textBox_CalPara_locx
			// 
			this->textBox_CalPara_locx->Enabled = false;
			this->textBox_CalPara_locx->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_locx->Location = System::Drawing::Point(160, 178);
			this->textBox_CalPara_locx->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_locx->Name = L"textBox_CalPara_locx";
			this->textBox_CalPara_locx->Size = System::Drawing::Size(71, 32);
			this->textBox_CalPara_locx->TabIndex = 48;
			this->textBox_CalPara_locx->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_CalPara_up
			// 
			this->button_CalPara_up->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_CalPara_up.Image")));
			this->button_CalPara_up->Location = System::Drawing::Point(579, 73);
			this->button_CalPara_up->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_CalPara_up->Name = L"button_CalPara_up";
			this->button_CalPara_up->Size = System::Drawing::Size(43, 28);
			this->button_CalPara_up->TabIndex = 41;
			this->button_CalPara_up->UseVisualStyleBackColor = true;
			this->button_CalPara_up->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_CalPara_up_Click);
			// 
			// label30
			// 
			this->label30->AutoSize = true;
			this->label30->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label30->Location = System::Drawing::Point(236, 178);
			this->label30->Name = L"label30";
			this->label30->Size = System::Drawing::Size(17, 25);
			this->label30->TabIndex = 47;
			this->label30->Text = L",";
			// 
			// textBox_CalPara_ypixel
			// 
			this->textBox_CalPara_ypixel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_ypixel->Location = System::Drawing::Point(271, 106);
			this->textBox_CalPara_ypixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_ypixel->Name = L"textBox_CalPara_ypixel";
			this->textBox_CalPara_ypixel->ReadOnly = true;
			this->textBox_CalPara_ypixel->Size = System::Drawing::Size(131, 32);
			this->textBox_CalPara_ypixel->TabIndex = 40;
			this->textBox_CalPara_ypixel->Text = L"512";
			this->textBox_CalPara_ypixel->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label29
			// 
			this->label29->AutoSize = true;
			this->label29->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label29->Location = System::Drawing::Point(336, 178);
			this->label29->Name = L"label29";
			this->label29->Size = System::Drawing::Size(19, 25);
			this->label29->TabIndex = 46;
			this->label29->Text = L")";
			// 
			// textBox_CalPara_xpixel
			// 
			this->textBox_CalPara_xpixel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_xpixel->Location = System::Drawing::Point(104, 106);
			this->textBox_CalPara_xpixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_xpixel->Name = L"textBox_CalPara_xpixel";
			this->textBox_CalPara_xpixel->Size = System::Drawing::Size(131, 32);
			this->textBox_CalPara_xpixel->TabIndex = 39;
			this->textBox_CalPara_xpixel->Text = L"512";
			this->textBox_CalPara_xpixel->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label28
			// 
			this->label28->AutoSize = true;
			this->label28->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label28->Location = System::Drawing::Point(136, 178);
			this->label28->Name = L"label28";
			this->label28->Size = System::Drawing::Size(19, 25);
			this->label28->TabIndex = 45;
			this->label28->Text = L"(";
			// 
			// label26
			// 
			this->label26->AutoSize = true;
			this->label26->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold));
			this->label26->Location = System::Drawing::Point(241, 110);
			this->label26->Name = L"label26";
			this->label26->Size = System::Drawing::Size(21, 24);
			this->label26->TabIndex = 38;
			this->label26->Text = L"x";
			// 
			// label27
			// 
			this->label27->AutoSize = true;
			this->label27->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label27->Location = System::Drawing::Point(21, 178);
			this->label27->Name = L"label27";
			this->label27->Size = System::Drawing::Size(104, 25);
			this->label27->TabIndex = 44;
			this->label27->Text = L"1 st Target";
			// 
			// label25
			// 
			this->label25->AutoSize = true;
			this->label25->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label25->Location = System::Drawing::Point(21, 110);
			this->label25->Name = L"label25";
			this->label25->Size = System::Drawing::Size(70, 25);
			this->label25->TabIndex = 37;
			this->label25->Text = L"Pixel : ";
			// 
			// button_CalPara_calculate
			// 
			this->button_CalPara_calculate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->button_CalPara_calculate->Location = System::Drawing::Point(415, 187);
			this->button_CalPara_calculate->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_CalPara_calculate->Name = L"button_CalPara_calculate";
			this->button_CalPara_calculate->Size = System::Drawing::Size(243, 48);
			this->button_CalPara_calculate->TabIndex = 23;
			this->button_CalPara_calculate->Text = L"Calculate";
			this->button_CalPara_calculate->UseVisualStyleBackColor = true;
			this->button_CalPara_calculate->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_CalPara_calculate_Click);
			// 
			// comboBox_CalPara_samplingrate
			// 
			this->comboBox_CalPara_samplingrate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			this->comboBox_CalPara_samplingrate->FormattingEnabled = true;
			this->comboBox_CalPara_samplingrate->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"2 MS/s", L"10 MS/s", L"20 MS/s",
					L"50 MS/s", L"100 MS/s", L"125 MS/s"
			});
			this->comboBox_CalPara_samplingrate->Location = System::Drawing::Point(249, 63);
			this->comboBox_CalPara_samplingrate->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->comboBox_CalPara_samplingrate->Name = L"comboBox_CalPara_samplingrate";
			this->comboBox_CalPara_samplingrate->Size = System::Drawing::Size(131, 33);
			this->comboBox_CalPara_samplingrate->TabIndex = 35;
			// 
			// label19
			// 
			this->label19->AutoSize = true;
			this->label19->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label19->Location = System::Drawing::Point(20, 65);
			this->label19->Name = L"label19";
			this->label19->Size = System::Drawing::Size(201, 25);
			this->label19->TabIndex = 30;
			this->label19->Text = L"Sampling Rate (MHz)";
			// 
			// textBox_CalPara_LOADRASTER
			// 
			this->textBox_CalPara_LOADRASTER->Enabled = false;
			this->textBox_CalPara_LOADRASTER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_CalPara_LOADRASTER->Location = System::Drawing::Point(227, 20);
			this->textBox_CalPara_LOADRASTER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_CalPara_LOADRASTER->Multiline = true;
			this->textBox_CalPara_LOADRASTER->Name = L"textBox_CalPara_LOADRASTER";
			this->textBox_CalPara_LOADRASTER->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_CalPara_LOADRASTER->Size = System::Drawing::Size(429, 38);
			this->textBox_CalPara_LOADRASTER->TabIndex = 22;
			// 
			// button_CalPara_LOADRASTER
			// 
			this->button_CalPara_LOADRASTER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_CalPara_LOADRASTER->Location = System::Drawing::Point(16, 20);
			this->button_CalPara_LOADRASTER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_CalPara_LOADRASTER->Name = L"button_CalPara_LOADRASTER";
			this->button_CalPara_LOADRASTER->Size = System::Drawing::Size(205, 38);
			this->button_CalPara_LOADRASTER->TabIndex = 21;
			this->button_CalPara_LOADRASTER->Text = L"Load RASTER";
			this->button_CalPara_LOADRASTER->UseVisualStyleBackColor = true;
			this->button_CalPara_LOADRASTER->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_CalPara_LOADRASTER_Click);
			// 
			// panel5
			// 
			this->panel5->BackColor = System::Drawing::Color::PowderBlue;
			this->panel5->Controls->Add(this->tableLayoutPanel_CalPara_result);
			this->panel5->Controls->Add(this->label23);
			this->panel5->Location = System::Drawing::Point(699, 89);
			this->panel5->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->panel5->Name = L"panel5";
			this->panel5->Size = System::Drawing::Size(531, 268);
			this->panel5->TabIndex = 107;
			// 
			// tableLayoutPanel_CalPara_result
			// 
			this->tableLayoutPanel_CalPara_result->AutoScroll = true;
			this->tableLayoutPanel_CalPara_result->BackColor = System::Drawing::Color::AliceBlue;
			this->tableLayoutPanel_CalPara_result->CellBorderStyle = System::Windows::Forms::TableLayoutPanelCellBorderStyle::Single;
			this->tableLayoutPanel_CalPara_result->ColumnCount = 5;
			this->tableLayoutPanel_CalPara_result->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				20)));
			this->tableLayoutPanel_CalPara_result->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				20)));
			this->tableLayoutPanel_CalPara_result->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				20)));
			this->tableLayoutPanel_CalPara_result->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				20)));
			this->tableLayoutPanel_CalPara_result->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
				20)));
			this->tableLayoutPanel_CalPara_result->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
			this->tableLayoutPanel_CalPara_result->Location = System::Drawing::Point(20, 44);
			this->tableLayoutPanel_CalPara_result->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->tableLayoutPanel_CalPara_result->Name = L"tableLayoutPanel_CalPara_result";
			this->tableLayoutPanel_CalPara_result->RowCount = 1;
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute,
				465)));
			this->tableLayoutPanel_CalPara_result->Size = System::Drawing::Size(495, 212);
			this->tableLayoutPanel_CalPara_result->TabIndex = 51;
			// 
			// label23
			// 
			this->label23->AutoSize = true;
			this->label23->BackColor = System::Drawing::Color::RoyalBlue;
			this->label23->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->label23->ForeColor = System::Drawing::SystemColors::ControlLightLight;
			this->label23->Location = System::Drawing::Point(17, 11);
			this->label23->Name = L"label23";
			this->label23->Size = System::Drawing::Size(244, 28);
			this->label23->TabIndex = 43;
			this->label23->Text = L"Result of the calculation";
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->tabPage2);
			this->tabControl1->Controls->Add(this->tabPage1);
			this->tabControl1->Controls->Add(this->tabPage5);
			this->tabControl1->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
			this->tabControl1->Location = System::Drawing::Point(3, 366);
			this->tabControl1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(687, 490);
			this->tabControl1->TabIndex = 109;
			// 
			// tabPage2
			// 
			this->tabPage2->BackColor = System::Drawing::Color::LightSteelBlue;
			this->tabPage2->Controls->Add(this->label_TRANSFERTOTIFF);
			this->tabPage2->Controls->Add(this->button_LOADLISSAJOUS_RAWBIN);
			this->tabPage2->Controls->Add(this->button_TRANSFERTOTIFF);
			this->tabPage2->Controls->Add(this->textBox_LOADLISSAJOUSBIN);
			this->tabPage2->Controls->Add(this->label41);
			this->tabPage2->Controls->Add(this->label3);
			this->tabPage2->Controls->Add(this->label_reconstruct);
			this->tabPage2->Controls->Add(this->textBox_LOADLISSAJOUS);
			this->tabPage2->Controls->Add(this->progressBar_reconstruct);
			this->tabPage2->Controls->Add(this->button_LOADLISSAJOUSRAWFOLDER);
			this->tabPage2->Controls->Add(this->textBox_lissajouspixelnum);
			this->tabPage2->Controls->Add(this->button_setphaseandsave);
			this->tabPage2->Controls->Add(this->label16);
			this->tabPage2->Controls->Add(this->trackBar_lissajousoffset);
			this->tabPage2->Controls->Add(this->textBox__LOADLISSAJOUSFOLDER2);
			this->tabPage2->Controls->Add(this->button_LOADLISSAJOUS_RAW);
			this->tabPage2->Controls->Add(this->label_lissajousoffset);
			this->tabPage2->Controls->Add(this->label8);
			this->tabPage2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
			this->tabPage2->Location = System::Drawing::Point(4, 28);
			this->tabPage2->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage2->Name = L"tabPage2";
			this->tabPage2->Padding = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage2->Size = System::Drawing::Size(679, 458);
			this->tabPage2->TabIndex = 1;
			this->tabPage2->Text = L"Lissajous Raw Data Phase Offset";
			// 
			// label_TRANSFERTOTIFF
			// 
			this->label_TRANSFERTOTIFF->AutoSize = true;
			this->label_TRANSFERTOTIFF->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_TRANSFERTOTIFF->Location = System::Drawing::Point(493, 121);
			this->label_TRANSFERTOTIFF->Name = L"label_TRANSFERTOTIFF";
			this->label_TRANSFERTOTIFF->Size = System::Drawing::Size(28, 25);
			this->label_TRANSFERTOTIFF->TabIndex = 115;
			this->label_TRANSFERTOTIFF->Text = L"--";
			// 
			// button_LOADLISSAJOUS_RAWBIN
			// 
			this->button_LOADLISSAJOUS_RAWBIN->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADLISSAJOUS_RAWBIN->Location = System::Drawing::Point(13, 44);
			this->button_LOADLISSAJOUS_RAWBIN->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADLISSAJOUS_RAWBIN->Name = L"button_LOADLISSAJOUS_RAWBIN";
			this->button_LOADLISSAJOUS_RAWBIN->Size = System::Drawing::Size(184, 62);
			this->button_LOADLISSAJOUS_RAWBIN->TabIndex = 112;
			this->button_LOADLISSAJOUS_RAWBIN->Text = L"Load LISSAJOUS RAW FOLDER";
			this->button_LOADLISSAJOUS_RAWBIN->UseVisualStyleBackColor = true;
			this->button_LOADLISSAJOUS_RAWBIN->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADLISSAJOUS_RAWBIN_Click_1);
			// 
			// button_TRANSFERTOTIFF
			// 
			this->button_TRANSFERTOTIFF->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->button_TRANSFERTOTIFF->Location = System::Drawing::Point(15, 110);
			this->button_TRANSFERTOTIFF->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_TRANSFERTOTIFF->Name = L"button_TRANSFERTOTIFF";
			this->button_TRANSFERTOTIFF->Size = System::Drawing::Size(472, 44);
			this->button_TRANSFERTOTIFF->TabIndex = 114;
			this->button_TRANSFERTOTIFF->Text = L"Convert to .tiff";
			this->button_TRANSFERTOTIFF->UseVisualStyleBackColor = true;
			this->button_TRANSFERTOTIFF->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_TRANSFERTOTIFF_Click_1);
			// 
			// textBox_LOADLISSAJOUSBIN
			// 
			this->textBox_LOADLISSAJOUSBIN->Enabled = false;
			this->textBox_LOADLISSAJOUSBIN->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->textBox_LOADLISSAJOUSBIN->Location = System::Drawing::Point(203, 49);
			this->textBox_LOADLISSAJOUSBIN->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADLISSAJOUSBIN->Multiline = true;
			this->textBox_LOADLISSAJOUSBIN->Name = L"textBox_LOADLISSAJOUSBIN";
			this->textBox_LOADLISSAJOUSBIN->ReadOnly = true;
			this->textBox_LOADLISSAJOUSBIN->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADLISSAJOUSBIN->Size = System::Drawing::Size(443, 56);
			this->textBox_LOADLISSAJOUSBIN->TabIndex = 113;
			// 
			// label41
			// 
			this->label41->AutoSize = true;
			this->label41->BackColor = System::Drawing::Color::RoyalBlue;
			this->label41->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->label41->ForeColor = System::Drawing::SystemColors::ControlLightLight;
			this->label41->Location = System::Drawing::Point(15, 10);
			this->label41->Name = L"label41";
			this->label41->Size = System::Drawing::Size(157, 28);
			this->label41->TabIndex = 107;
			this->label41->Text = L".Bin convertion";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->BackColor = System::Drawing::Color::RoyalBlue;
			this->label3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			this->label3->ForeColor = System::Drawing::SystemColors::ControlLightLight;
			this->label3->Location = System::Drawing::Point(15, 161);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(129, 28);
			this->label3->TabIndex = 106;
			this->label3->Text = L"Phase offset";
			// 
			// tabPage1
			// 
			this->tabPage1->BackColor = System::Drawing::Color::LightSteelBlue;
			this->tabPage1->Controls->Add(this->label_scanmodemerge);
			this->tabPage1->Controls->Add(this->textBox_LOADLISSAJOUSFOLDER);
			this->tabPage1->Controls->Add(this->progressBar_scanmodemerge);
			this->tabPage1->Controls->Add(this->checkBox_save);
			this->tabPage1->Controls->Add(this->textBox_resizey);
			this->tabPage1->Controls->Add(this->label7);
			this->tabPage1->Controls->Add(this->textBox_resizex);
			this->tabPage1->Controls->Add(this->button_up);
			this->tabPage1->Controls->Add(this->button_fastset);
			this->tabPage1->Controls->Add(this->textBox_offsety);
			this->tabPage1->Controls->Add(this->label_resizey);
			this->tabPage1->Controls->Add(this->button_down);
			this->tabPage1->Controls->Add(this->trackBar_resizey);
			this->tabPage1->Controls->Add(this->textBox_offsetx);
			this->tabPage1->Controls->Add(this->label_resizex);
			this->tabPage1->Controls->Add(this->button_left);
			this->tabPage1->Controls->Add(this->trackBar_resizex);
			this->tabPage1->Controls->Add(this->label6);
			this->tabPage1->Controls->Add(this->label4);
			this->tabPage1->Controls->Add(this->button_right);
			this->tabPage1->Controls->Add(this->label17);
			this->tabPage1->Controls->Add(this->label5);
			this->tabPage1->Controls->Add(this->button_Merge_and_Display);
			this->tabPage1->Controls->Add(this->button_LOADRASTER);
			this->tabPage1->Controls->Add(this->textBox_LOADRASTER);
			this->tabPage1->Controls->Add(this->checkBox_MergeScantrace);
			this->tabPage1->Controls->Add(this->button_LOADLISSAJOUSFOLDER);
			this->tabPage1->ForeColor = System::Drawing::Color::Black;
			this->tabPage1->Location = System::Drawing::Point(4, 28);
			this->tabPage1->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage1->Name = L"tabPage1";
			this->tabPage1->Padding = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage1->Size = System::Drawing::Size(679, 458);
			this->tabPage1->TabIndex = 0;
			this->tabPage1->Text = L"ScanMode Merge";
			// 
			// tabPage5
			// 
			this->tabPage5->BackColor = System::Drawing::Color::LightSteelBlue;
			this->tabPage5->Controls->Add(this->label38);
			this->tabPage5->Controls->Add(this->label_ExtractKymograph_choosedisplay);
			this->tabPage5->Controls->Add(this->toggleButtons_ExtractKymograph_choosedisplay);
			this->tabPage5->Controls->Add(this->button_ExtractKymograph_quickset);
			this->tabPage5->Controls->Add(this->label36);
			this->tabPage5->Controls->Add(this->label37);
			this->tabPage5->Controls->Add(this->button_ExtractKymograph_extractkymograph);
			this->tabPage5->Controls->Add(this->button_LOADLISSAJOUS);
			this->tabPage5->Controls->Add(this->textBox_LOADLISSAJOUS_EK);
			this->tabPage5->Controls->Add(this->textBox_ExtractKymograph_endpixel);
			this->tabPage5->Controls->Add(this->textBox_ExtractKymograph_startpixel);
			this->tabPage5->Controls->Add(this->trackBar_ExtractKymograph_endpixel);
			this->tabPage5->Controls->Add(this->trackBar_ExtractKymograph_startpixel);
			this->tabPage5->Controls->Add(this->label32);
			this->tabPage5->Controls->Add(this->label35);
			this->tabPage5->Controls->Add(this->textBox_LOADLISSAJOUSFOLDER_EK);
			this->tabPage5->Controls->Add(this->button_LOADRASTER2);
			this->tabPage5->Controls->Add(this->textBox_LOADRASTER_EK);
			this->tabPage5->Controls->Add(this->button_LOADLISSAJOUSFOLDER2);
			this->tabPage5->Location = System::Drawing::Point(4, 28);
			this->tabPage5->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage5->Name = L"tabPage5";
			this->tabPage5->Size = System::Drawing::Size(679, 458);
			this->tabPage5->TabIndex = 2;
			this->tabPage5->Text = L"Extract kymograph";
			// 
			// label38
			// 
			this->label38->AutoSize = true;
			this->label38->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8, System::Drawing::FontStyle::Bold));
			this->label38->ForeColor = System::Drawing::Color::DarkRed;
			this->label38->Location = System::Drawing::Point(343, 209);
			this->label38->Name = L"label38";
			this->label38->Size = System::Drawing::Size(313, 19);
			this->label38->TabIndex = 122;
			this->label38->Text = L"Note: Raster and Lissajous need the same size";
			// 
			// label_ExtractKymograph_choosedisplay
			// 
			this->label_ExtractKymograph_choosedisplay->AutoSize = true;
			this->label_ExtractKymograph_choosedisplay->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label_ExtractKymograph_choosedisplay->Location = System::Drawing::Point(123, 225);
			this->label_ExtractKymograph_choosedisplay->Name = L"label_ExtractKymograph_choosedisplay";
			this->label_ExtractKymograph_choosedisplay->Size = System::Drawing::Size(218, 25);
			this->label_ExtractKymograph_choosedisplay->TabIndex = 121;
			this->label_ExtractKymograph_choosedisplay->Text = L"Display Lissajous image";
			// 
			// toggleButtons_ExtractKymograph_choosedisplay
			// 
			this->toggleButtons_ExtractKymograph_choosedisplay->BackColor = System::Drawing::Color::MediumTurquoise;
			this->toggleButtons_ExtractKymograph_choosedisplay->Location = System::Drawing::Point(27, 220);
			this->toggleButtons_ExtractKymograph_choosedisplay->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->toggleButtons_ExtractKymograph_choosedisplay->MinimumSize = System::Drawing::Size(45, 22);
			this->toggleButtons_ExtractKymograph_choosedisplay->Name = L"toggleButtons_ExtractKymograph_choosedisplay";
			this->toggleButtons_ExtractKymograph_choosedisplay->OffBackColor = System::Drawing::Color::SteelBlue;
			this->toggleButtons_ExtractKymograph_choosedisplay->OffToggleColor = System::Drawing::Color::SteelBlue;
			this->toggleButtons_ExtractKymograph_choosedisplay->OnBackColor = System::Drawing::Color::RoyalBlue;
			this->toggleButtons_ExtractKymograph_choosedisplay->OnToggleColor = System::Drawing::Color::RoyalBlue;
			this->toggleButtons_ExtractKymograph_choosedisplay->Size = System::Drawing::Size(73, 31);
			this->toggleButtons_ExtractKymograph_choosedisplay->SolidStyle = false;
			this->toggleButtons_ExtractKymograph_choosedisplay->TabIndex = 120;
			this->toggleButtons_ExtractKymograph_choosedisplay->UseVisualStyleBackColor = false;
			this->toggleButtons_ExtractKymograph_choosedisplay->CheckedChanged += gcnew System::EventHandler(this, &YSPostprocessing::toggleButtons_ExtractKymograph_choosedisplay_CheckedChanged);
			// 
			// button_ExtractKymograph_quickset
			// 
			this->button_ExtractKymograph_quickset->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_ExtractKymograph_quickset->Location = System::Drawing::Point(20, 398);
			this->button_ExtractKymograph_quickset->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_ExtractKymograph_quickset->Name = L"button_ExtractKymograph_quickset";
			this->button_ExtractKymograph_quickset->Size = System::Drawing::Size(261, 48);
			this->button_ExtractKymograph_quickset->TabIndex = 119;
			this->button_ExtractKymograph_quickset->Text = L"Quick Set";
			this->button_ExtractKymograph_quickset->UseVisualStyleBackColor = true;
			this->button_ExtractKymograph_quickset->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_ExtractKymograph_quickset_Click);
			// 
			// label36
			// 
			this->label36->AutoSize = true;
			this->label36->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label36->Location = System::Drawing::Point(252, 336);
			this->label36->Name = L"label36";
			this->label36->Size = System::Drawing::Size(28, 25);
			this->label36->TabIndex = 118;
			this->label36->Text = L"%";
			// 
			// label37
			// 
			this->label37->AutoSize = true;
			this->label37->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label37->Location = System::Drawing::Point(252, 268);
			this->label37->Name = L"label37";
			this->label37->Size = System::Drawing::Size(28, 25);
			this->label37->TabIndex = 117;
			this->label37->Text = L"%";
			// 
			// button_ExtractKymograph_extractkymograph
			// 
			this->button_ExtractKymograph_extractkymograph->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_ExtractKymograph_extractkymograph->Location = System::Drawing::Point(396, 398);
			this->button_ExtractKymograph_extractkymograph->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_ExtractKymograph_extractkymograph->Name = L"button_ExtractKymograph_extractkymograph";
			this->button_ExtractKymograph_extractkymograph->Size = System::Drawing::Size(261, 48);
			this->button_ExtractKymograph_extractkymograph->TabIndex = 116;
			this->button_ExtractKymograph_extractkymograph->Text = L"Extract Kymograph";
			this->button_ExtractKymograph_extractkymograph->UseVisualStyleBackColor = true;
			this->button_ExtractKymograph_extractkymograph->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_ExtractKymograph_extractkymograph_Click);
			// 
			// button_LOADLISSAJOUS
			// 
			this->button_LOADLISSAJOUS->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADLISSAJOUS->Location = System::Drawing::Point(13, 78);
			this->button_LOADLISSAJOUS->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADLISSAJOUS->Name = L"button_LOADLISSAJOUS";
			this->button_LOADLISSAJOUS->Size = System::Drawing::Size(184, 62);
			this->button_LOADLISSAJOUS->TabIndex = 114;
			this->button_LOADLISSAJOUS->Text = L"Load LISSAJOUS";
			this->button_LOADLISSAJOUS->UseVisualStyleBackColor = true;
			this->button_LOADLISSAJOUS->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADLISSAJOUS_Click_1);
			// 
			// textBox_LOADLISSAJOUS_EK
			// 
			this->textBox_LOADLISSAJOUS_EK->Enabled = false;
			this->textBox_LOADLISSAJOUS_EK->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_LOADLISSAJOUS_EK->Location = System::Drawing::Point(203, 78);
			this->textBox_LOADLISSAJOUS_EK->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADLISSAJOUS_EK->Multiline = true;
			this->textBox_LOADLISSAJOUS_EK->Name = L"textBox_LOADLISSAJOUS_EK";
			this->textBox_LOADLISSAJOUS_EK->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADLISSAJOUS_EK->Size = System::Drawing::Size(447, 54);
			this->textBox_LOADLISSAJOUS_EK->TabIndex = 115;
			// 
			// textBox_ExtractKymograph_endpixel
			// 
			this->textBox_ExtractKymograph_endpixel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_ExtractKymograph_endpixel->Location = System::Drawing::Point(149, 332);
			this->textBox_ExtractKymograph_endpixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ExtractKymograph_endpixel->Name = L"textBox_ExtractKymograph_endpixel";
			this->textBox_ExtractKymograph_endpixel->Size = System::Drawing::Size(103, 32);
			this->textBox_ExtractKymograph_endpixel->TabIndex = 113;
			this->textBox_ExtractKymograph_endpixel->Text = L"100.0";
			this->textBox_ExtractKymograph_endpixel->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_ExtractKymograph_startpixel
			// 
			this->textBox_ExtractKymograph_startpixel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_ExtractKymograph_startpixel->Location = System::Drawing::Point(149, 265);
			this->textBox_ExtractKymograph_startpixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ExtractKymograph_startpixel->Name = L"textBox_ExtractKymograph_startpixel";
			this->textBox_ExtractKymograph_startpixel->Size = System::Drawing::Size(103, 32);
			this->textBox_ExtractKymograph_startpixel->TabIndex = 112;
			this->textBox_ExtractKymograph_startpixel->Text = L"0.0";
			this->textBox_ExtractKymograph_startpixel->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// trackBar_ExtractKymograph_endpixel
			// 
			this->trackBar_ExtractKymograph_endpixel->AutoSize = false;
			this->trackBar_ExtractKymograph_endpixel->BackColor = System::Drawing::Color::LightSteelBlue;
			this->trackBar_ExtractKymograph_endpixel->LargeChange = 2;
			this->trackBar_ExtractKymograph_endpixel->Location = System::Drawing::Point(27, 361);
			this->trackBar_ExtractKymograph_endpixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->trackBar_ExtractKymograph_endpixel->Maximum = 1000000;
			this->trackBar_ExtractKymograph_endpixel->Name = L"trackBar_ExtractKymograph_endpixel";
			this->trackBar_ExtractKymograph_endpixel->Size = System::Drawing::Size(633, 30);
			this->trackBar_ExtractKymograph_endpixel->TabIndex = 111;
			this->trackBar_ExtractKymograph_endpixel->TickFrequency = 5;
			this->trackBar_ExtractKymograph_endpixel->Value = 1000000;
			this->trackBar_ExtractKymograph_endpixel->Scroll += gcnew System::EventHandler(this, &YSPostprocessing::trackBar_ExtractKymograph_endpixel_Scroll);
			// 
			// trackBar_ExtractKymograph_startpixel
			// 
			this->trackBar_ExtractKymograph_startpixel->AutoSize = false;
			this->trackBar_ExtractKymograph_startpixel->BackColor = System::Drawing::Color::LightSteelBlue;
			this->trackBar_ExtractKymograph_startpixel->LargeChange = 2;
			this->trackBar_ExtractKymograph_startpixel->Location = System::Drawing::Point(24, 298);
			this->trackBar_ExtractKymograph_startpixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->trackBar_ExtractKymograph_startpixel->Maximum = 1000000;
			this->trackBar_ExtractKymograph_startpixel->Name = L"trackBar_ExtractKymograph_startpixel";
			this->trackBar_ExtractKymograph_startpixel->Size = System::Drawing::Size(633, 30);
			this->trackBar_ExtractKymograph_startpixel->TabIndex = 110;
			this->trackBar_ExtractKymograph_startpixel->TickFrequency = 5;
			this->trackBar_ExtractKymograph_startpixel->Scroll += gcnew System::EventHandler(this, &YSPostprocessing::trackBar_ExtractKymograph_startpixel_Scroll);
			// 
			// label32
			// 
			this->label32->AutoSize = true;
			this->label32->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label32->Location = System::Drawing::Point(20, 268);
			this->label32->Name = L"label32";
			this->label32->Size = System::Drawing::Size(104, 25);
			this->label32->TabIndex = 108;
			this->label32->Text = L"Start Pixel";
			// 
			// label35
			// 
			this->label35->AutoSize = true;
			this->label35->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label35->Location = System::Drawing::Point(21, 336);
			this->label35->Name = L"label35";
			this->label35->Size = System::Drawing::Size(94, 25);
			this->label35->TabIndex = 109;
			this->label35->Text = L"End Pixel";
			// 
			// textBox_LOADLISSAJOUSFOLDER_EK
			// 
			this->textBox_LOADLISSAJOUSFOLDER_EK->Enabled = false;
			this->textBox_LOADLISSAJOUSFOLDER_EK->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_LOADLISSAJOUSFOLDER_EK->Location = System::Drawing::Point(203, 143);
			this->textBox_LOADLISSAJOUSFOLDER_EK->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADLISSAJOUSFOLDER_EK->Multiline = true;
			this->textBox_LOADLISSAJOUSFOLDER_EK->Name = L"textBox_LOADLISSAJOUSFOLDER_EK";
			this->textBox_LOADLISSAJOUSFOLDER_EK->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADLISSAJOUSFOLDER_EK->Size = System::Drawing::Size(447, 61);
			this->textBox_LOADLISSAJOUSFOLDER_EK->TabIndex = 26;
			// 
			// button_LOADRASTER2
			// 
			this->button_LOADRASTER2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADRASTER2->Location = System::Drawing::Point(13, 12);
			this->button_LOADRASTER2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADRASTER2->Name = L"button_LOADRASTER2";
			this->button_LOADRASTER2->Size = System::Drawing::Size(184, 62);
			this->button_LOADRASTER2->TabIndex = 23;
			this->button_LOADRASTER2->Text = L"Load RASTER";
			this->button_LOADRASTER2->UseVisualStyleBackColor = true;
			this->button_LOADRASTER2->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADRASTER2_Click);
			// 
			// textBox_LOADRASTER_EK
			// 
			this->textBox_LOADRASTER_EK->Enabled = false;
			this->textBox_LOADRASTER_EK->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_LOADRASTER_EK->Location = System::Drawing::Point(203, 12);
			this->textBox_LOADRASTER_EK->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_LOADRASTER_EK->Multiline = true;
			this->textBox_LOADRASTER_EK->Name = L"textBox_LOADRASTER_EK";
			this->textBox_LOADRASTER_EK->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_LOADRASTER_EK->Size = System::Drawing::Size(447, 54);
			this->textBox_LOADRASTER_EK->TabIndex = 24;
			// 
			// button_LOADLISSAJOUSFOLDER2
			// 
			this->button_LOADLISSAJOUSFOLDER2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_LOADLISSAJOUSFOLDER2->Location = System::Drawing::Point(13, 143);
			this->button_LOADLISSAJOUSFOLDER2->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_LOADLISSAJOUSFOLDER2->Name = L"button_LOADLISSAJOUSFOLDER2";
			this->button_LOADLISSAJOUSFOLDER2->Size = System::Drawing::Size(184, 62);
			this->button_LOADLISSAJOUSFOLDER2->TabIndex = 25;
			this->button_LOADLISSAJOUSFOLDER2->Text = L"Load LISSAJOUS FOLDER";
			this->button_LOADLISSAJOUSFOLDER2->UseVisualStyleBackColor = true;
			this->button_LOADLISSAJOUSFOLDER2->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_LOADLISSAJOUSFOLDER2_Click_1);
			// 
			// tabControl2
			// 
			this->tabControl2->Controls->Add(this->tabPage3);
			this->tabControl2->Controls->Add(this->tabPage4);
			this->tabControl2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
			this->tabControl2->Location = System::Drawing::Point(3, 62);
			this->tabControl2->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabControl2->Name = L"tabControl2";
			this->tabControl2->SelectedIndex = 0;
			this->tabControl2->Size = System::Drawing::Size(687, 297);
			this->tabControl2->TabIndex = 110;
			// 
			// tabPage3
			// 
			this->tabPage3->BackColor = System::Drawing::Color::PowderBlue;
			this->tabPage3->Controls->Add(this->label39);
			this->tabPage3->Controls->Add(this->checkBox_CalPara_linepoints);
			this->tabPage3->Controls->Add(this->button_CalPara_calculate);
			this->tabPage3->Controls->Add(this->label22);
			this->tabPage3->Controls->Add(this->button_CalPara_LOADRASTER);
			this->tabPage3->Controls->Add(this->textBox_CalPara_locy2);
			this->tabPage3->Controls->Add(this->textBox_CalPara_LOADRASTER);
			this->tabPage3->Controls->Add(this->textBox_CalPara_locx2);
			this->tabPage3->Controls->Add(this->label19);
			this->tabPage3->Controls->Add(this->label18);
			this->tabPage3->Controls->Add(this->comboBox_CalPara_samplingrate);
			this->tabPage3->Controls->Add(this->label20);
			this->tabPage3->Controls->Add(this->label25);
			this->tabPage3->Controls->Add(this->label21);
			this->tabPage3->Controls->Add(this->label27);
			this->tabPage3->Controls->Add(this->label_CalPara_twopoints);
			this->tabPage3->Controls->Add(this->label26);
			this->tabPage3->Controls->Add(this->toggleButtons_CalPara_twopoints);
			this->tabPage3->Controls->Add(this->label28);
			this->tabPage3->Controls->Add(this->button_CalPara_right);
			this->tabPage3->Controls->Add(this->textBox_CalPara_xpixel);
			this->tabPage3->Controls->Add(this->button_CalPara_left);
			this->tabPage3->Controls->Add(this->label29);
			this->tabPage3->Controls->Add(this->textBox_CalPara_locy);
			this->tabPage3->Controls->Add(this->textBox_CalPara_ypixel);
			this->tabPage3->Controls->Add(this->button_CalPara_down);
			this->tabPage3->Controls->Add(this->label30);
			this->tabPage3->Controls->Add(this->textBox_CalPara_locx);
			this->tabPage3->Controls->Add(this->button_CalPara_up);
			this->tabPage3->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F));
			this->tabPage3->Location = System::Drawing::Point(4, 28);
			this->tabPage3->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage3->Name = L"tabPage3";
			this->tabPage3->Padding = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage3->Size = System::Drawing::Size(679, 265);
			this->tabPage3->TabIndex = 0;
			this->tabPage3->Text = L"Lissajous Parameter Calculation";
			// 
			// label39
			// 
			this->label39->AutoSize = true;
			this->label39->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8, System::Drawing::FontStyle::Bold));
			this->label39->ForeColor = System::Drawing::Color::DarkRed;
			this->label39->Location = System::Drawing::Point(337, 245);
			this->label39->Name = L"label39";
			this->label39->Size = System::Drawing::Size(314, 19);
			this->label39->TabIndex = 123;
			this->label39->Text = L"Note: Raster and Lissajous have the same FOV";
			// 
			// tabPage4
			// 
			this->tabPage4->BackColor = System::Drawing::Color::PowderBlue;
			this->tabPage4->Controls->Add(this->label40);
			this->tabPage4->Controls->Add(this->comboBox_ScanPreview_xyfreq);
			this->tabPage4->Controls->Add(this->button_ScanPreview_rightphase);
			this->tabPage4->Controls->Add(this->button_ScanPreview_leftphase);
			this->tabPage4->Controls->Add(this->textBox_ScanPreview_framerate);
			this->tabPage4->Controls->Add(this->label34);
			this->tabPage4->Controls->Add(this->textBox_ScanPreview_phase);
			this->tabPage4->Controls->Add(this->label31);
			this->tabPage4->Controls->Add(this->label33);
			this->tabPage4->Controls->Add(this->label2);
			this->tabPage4->Controls->Add(this->comboBox_ScanPreview_samplingrate);
			this->tabPage4->Controls->Add(this->label9);
			this->tabPage4->Controls->Add(this->label24);
			this->tabPage4->Controls->Add(this->textBox_ScanPreview_xpixel);
			this->tabPage4->Controls->Add(this->textBox_ScanPreview_ypixel);
			this->tabPage4->Controls->Add(this->button_ScanPreview_LOADRASTER);
			this->tabPage4->Controls->Add(this->textBox_ScanPreview_LOADRASTER);
			this->tabPage4->Location = System::Drawing::Point(4, 28);
			this->tabPage4->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage4->Name = L"tabPage4";
			this->tabPage4->Padding = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->tabPage4->Size = System::Drawing::Size(679, 265);
			this->tabPage4->TabIndex = 1;
			this->tabPage4->Text = L"Scan Preview";
			// 
			// label40
			// 
			this->label40->AutoSize = true;
			this->label40->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8, System::Drawing::FontStyle::Bold));
			this->label40->ForeColor = System::Drawing::Color::DarkRed;
			this->label40->Location = System::Drawing::Point(344, 247);
			this->label40->Name = L"label40";
			this->label40->Size = System::Drawing::Size(314, 19);
			this->label40->TabIndex = 124;
			this->label40->Text = L"Note: Raster and Lissajous have the same FOV";
			// 
			// comboBox_ScanPreview_xyfreq
			// 
			this->comboBox_ScanPreview_xyfreq->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			this->comboBox_ScanPreview_xyfreq->FormattingEnabled = true;
			this->comboBox_ScanPreview_xyfreq->Items->AddRange(gcnew cli::array< System::Object^  >(9) {
				L"(1500,2000)", L"(2000,2010)",
					L"(2000,2100)", L"(2100,2160)", L"(2088,2160)", L"(2052,2160)", L"(1980,2160)", L"(1620,2160)", L"(2000,3000)"
			});
			this->comboBox_ScanPreview_xyfreq->Location = System::Drawing::Point(257, 135);
			this->comboBox_ScanPreview_xyfreq->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->comboBox_ScanPreview_xyfreq->Name = L"comboBox_ScanPreview_xyfreq";
			this->comboBox_ScanPreview_xyfreq->Size = System::Drawing::Size(192, 33);
			this->comboBox_ScanPreview_xyfreq->TabIndex = 60;
			this->comboBox_ScanPreview_xyfreq->SelectedIndexChanged += gcnew System::EventHandler(this, &YSPostprocessing::comboBox_ScanPreview_xyfreq_SelectedIndexChanged);
			// 
			// button_ScanPreview_rightphase
			// 
			this->button_ScanPreview_rightphase->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_ScanPreview_rightphase.Image")));
			this->button_ScanPreview_rightphase->Location = System::Drawing::Point(584, 176);
			this->button_ScanPreview_rightphase->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_ScanPreview_rightphase->Name = L"button_ScanPreview_rightphase";
			this->button_ScanPreview_rightphase->Size = System::Drawing::Size(31, 39);
			this->button_ScanPreview_rightphase->TabIndex = 56;
			this->button_ScanPreview_rightphase->UseVisualStyleBackColor = true;
			this->button_ScanPreview_rightphase->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_ScanPreview_rightphase_Click);
			// 
			// button_ScanPreview_leftphase
			// 
			this->button_ScanPreview_leftphase->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"button_ScanPreview_leftphase.Image")));
			this->button_ScanPreview_leftphase->Location = System::Drawing::Point(508, 176);
			this->button_ScanPreview_leftphase->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_ScanPreview_leftphase->Name = L"button_ScanPreview_leftphase";
			this->button_ScanPreview_leftphase->Size = System::Drawing::Size(31, 39);
			this->button_ScanPreview_leftphase->TabIndex = 55;
			this->button_ScanPreview_leftphase->UseVisualStyleBackColor = true;
			this->button_ScanPreview_leftphase->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_ScanPreview_leftphase_Click);
			// 
			// textBox_ScanPreview_framerate
			// 
			this->textBox_ScanPreview_framerate->Enabled = false;
			this->textBox_ScanPreview_framerate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_ScanPreview_framerate->Location = System::Drawing::Point(257, 175);
			this->textBox_ScanPreview_framerate->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ScanPreview_framerate->Name = L"textBox_ScanPreview_framerate";
			this->textBox_ScanPreview_framerate->Size = System::Drawing::Size(131, 32);
			this->textBox_ScanPreview_framerate->TabIndex = 54;
			this->textBox_ScanPreview_framerate->Text = L"500";
			this->textBox_ScanPreview_framerate->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label34
			// 
			this->label34->AutoSize = true;
			this->label34->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label34->Location = System::Drawing::Point(23, 178);
			this->label34->Name = L"label34";
			this->label34->Size = System::Drawing::Size(150, 25);
			this->label34->TabIndex = 53;
			this->label34->Text = L"Frame rate (Hz)";
			// 
			// textBox_ScanPreview_phase
			// 
			this->textBox_ScanPreview_phase->Enabled = false;
			this->textBox_ScanPreview_phase->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_ScanPreview_phase->Location = System::Drawing::Point(257, 210);
			this->textBox_ScanPreview_phase->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ScanPreview_phase->Name = L"textBox_ScanPreview_phase";
			this->textBox_ScanPreview_phase->Size = System::Drawing::Size(131, 32);
			this->textBox_ScanPreview_phase->TabIndex = 52;
			this->textBox_ScanPreview_phase->Text = L"0";
			this->textBox_ScanPreview_phase->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// label31
			// 
			this->label31->AutoSize = true;
			this->label31->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label31->Location = System::Drawing::Point(24, 214);
			this->label31->Name = L"label31";
			this->label31->Size = System::Drawing::Size(63, 25);
			this->label31->TabIndex = 51;
			this->label31->Text = L"Phase";
			// 
			// label33
			// 
			this->label33->AutoSize = true;
			this->label33->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label33->Location = System::Drawing::Point(21, 139);
			this->label33->Name = L"label33";
			this->label33->Size = System::Drawing::Size(204, 25);
			this->label33->TabIndex = 47;
			this->label33->Text = L"Freq X,Freq Y (Hz,Hz)";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label2->Location = System::Drawing::Point(24, 14);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(201, 25);
			this->label2->TabIndex = 41;
			this->label2->Text = L"Sampling Rate (MHz)";
			// 
			// comboBox_ScanPreview_samplingrate
			// 
			this->comboBox_ScanPreview_samplingrate->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			this->comboBox_ScanPreview_samplingrate->FormattingEnabled = true;
			this->comboBox_ScanPreview_samplingrate->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"2 MS/s", L"10 MS/s",
					L"20 MS/s", L"50 MS/s", L"100 MS/s", L"125 MS/s"
			});
			this->comboBox_ScanPreview_samplingrate->Location = System::Drawing::Point(253, 11);
			this->comboBox_ScanPreview_samplingrate->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->comboBox_ScanPreview_samplingrate->Name = L"comboBox_ScanPreview_samplingrate";
			this->comboBox_ScanPreview_samplingrate->Size = System::Drawing::Size(131, 33);
			this->comboBox_ScanPreview_samplingrate->TabIndex = 42;
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->label9->Location = System::Drawing::Point(25, 55);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(70, 25);
			this->label9->TabIndex = 43;
			this->label9->Text = L"Pixel : ";
			// 
			// label24
			// 
			this->label24->AutoSize = true;
			this->label24->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11.25F, System::Drawing::FontStyle::Bold));
			this->label24->Location = System::Drawing::Point(243, 55);
			this->label24->Name = L"label24";
			this->label24->Size = System::Drawing::Size(21, 24);
			this->label24->TabIndex = 44;
			this->label24->Text = L"x";
			// 
			// textBox_ScanPreview_xpixel
			// 
			this->textBox_ScanPreview_xpixel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_ScanPreview_xpixel->Location = System::Drawing::Point(105, 52);
			this->textBox_ScanPreview_xpixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ScanPreview_xpixel->Name = L"textBox_ScanPreview_xpixel";
			this->textBox_ScanPreview_xpixel->Size = System::Drawing::Size(131, 32);
			this->textBox_ScanPreview_xpixel->TabIndex = 45;
			this->textBox_ScanPreview_xpixel->Text = L"512";
			this->textBox_ScanPreview_xpixel->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// textBox_ScanPreview_ypixel
			// 
			this->textBox_ScanPreview_ypixel->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F, System::Drawing::FontStyle::Bold));
			this->textBox_ScanPreview_ypixel->Location = System::Drawing::Point(272, 52);
			this->textBox_ScanPreview_ypixel->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ScanPreview_ypixel->Name = L"textBox_ScanPreview_ypixel";
			this->textBox_ScanPreview_ypixel->ReadOnly = true;
			this->textBox_ScanPreview_ypixel->Size = System::Drawing::Size(131, 32);
			this->textBox_ScanPreview_ypixel->TabIndex = 46;
			this->textBox_ScanPreview_ypixel->Text = L"512";
			this->textBox_ScanPreview_ypixel->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// button_ScanPreview_LOADRASTER
			// 
			this->button_ScanPreview_LOADRASTER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10, System::Drawing::FontStyle::Bold));
			this->button_ScanPreview_LOADRASTER->Location = System::Drawing::Point(16, 90);
			this->button_ScanPreview_LOADRASTER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->button_ScanPreview_LOADRASTER->Name = L"button_ScanPreview_LOADRASTER";
			this->button_ScanPreview_LOADRASTER->Size = System::Drawing::Size(205, 38);
			this->button_ScanPreview_LOADRASTER->TabIndex = 23;
			this->button_ScanPreview_LOADRASTER->Text = L"Load RASTER";
			this->button_ScanPreview_LOADRASTER->UseVisualStyleBackColor = true;
			this->button_ScanPreview_LOADRASTER->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_ScanPreview_LOADRASTER_Click);
			// 
			// textBox_ScanPreview_LOADRASTER
			// 
			this->textBox_ScanPreview_LOADRASTER->Enabled = false;
			this->textBox_ScanPreview_LOADRASTER->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Bold));
			this->textBox_ScanPreview_LOADRASTER->Location = System::Drawing::Point(227, 90);
			this->textBox_ScanPreview_LOADRASTER->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->textBox_ScanPreview_LOADRASTER->Multiline = true;
			this->textBox_ScanPreview_LOADRASTER->Name = L"textBox_ScanPreview_LOADRASTER";
			this->textBox_ScanPreview_LOADRASTER->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->textBox_ScanPreview_LOADRASTER->Size = System::Drawing::Size(429, 38);
			this->textBox_ScanPreview_LOADRASTER->TabIndex = 24;
			// 
			// button_shut_down
			// 
			this->button_shut_down->Font = (gcnew System::Drawing::Font(L"Segoe UI", 15, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic))));
			this->button_shut_down->ForeColor = System::Drawing::Color::DarkGoldenrod;
			this->button_shut_down->Location = System::Drawing::Point(1085, 15);
			this->button_shut_down->Margin = System::Windows::Forms::Padding(4, 4, 4, 4);
			this->button_shut_down->Name = L"button_shut_down";
			this->button_shut_down->Size = System::Drawing::Size(132, 41);
			this->button_shut_down->TabIndex = 112;
			this->button_shut_down->Text = L"Close";
			this->button_shut_down->UseVisualStyleBackColor = true;
			this->button_shut_down->Click += gcnew System::EventHandler(this, &YSPostprocessing::button_shut_down_Click);
			// 
			// YSPostprocessing
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::InactiveCaption;
			this->ClientSize = System::Drawing::Size(1233, 863);
			this->Controls->Add(this->button_shut_down);
			this->Controls->Add(this->tabControl2);
			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->panel5);
			this->Controls->Add(this->panel3);
			this->Controls->Add(this->label1);
			this->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->Name = L"YSPostprocessing";
			this->Text = L"YSPostprocessing";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_lissajousoffset))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_resizey))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_resizex))->EndInit();
			this->panel3->ResumeLayout(false);
			this->panel3->PerformLayout();
			this->panel5->ResumeLayout(false);
			this->panel5->PerformLayout();
			this->tabControl1->ResumeLayout(false);
			this->tabPage2->ResumeLayout(false);
			this->tabPage2->PerformLayout();
			this->tabPage1->ResumeLayout(false);
			this->tabPage1->PerformLayout();
			this->tabPage5->ResumeLayout(false);
			this->tabPage5->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_ExtractKymograph_endpixel))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar_ExtractKymograph_startpixel))->EndInit();
			this->tabControl2->ResumeLayout(false);
			this->tabPage3->ResumeLayout(false);
			this->tabPage3->PerformLayout();
			this->tabPage4->ResumeLayout(false);
			this->tabPage4->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
//Scan node merge
private: System::Void button_LOADRASTER_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_RASTER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_RASTER->FileName;
		this->textBox_LOADRASTER->Text = selectedPath;
		imagemerge->RASTERPath = msclr::interop::marshal_as<std::string>(selectedPath);
	}
}
private: System::Void button_LOADLISSAJOUSFOLDER_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_LISSAJOUS_FOLDER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_LISSAJOUS_FOLDER->SelectedPath;
		this->textBox_LOADLISSAJOUSFOLDER->Text = selectedPath;
		imagemerge->LISSAJOUSFolderPath = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->LOADspec(imagemerge->LISSAJOUSFolderPath);
		this->textBox_lissajousfreqx->Text = imagemerge->xfreq.ToString();
		this->textBox_lissajousfreqy->Text = imagemerge->yfreq.ToString();
		this->textBox_lissajousphase->Text = imagemerge->lissajous_phase.ToString();
		this->textBox_numofsample->Text = imagemerge->num_of_sample.ToString();
		this->textBox_samplingrate->Text = imagemerge->samplingrate.ToString();
		imagemerge->DISPLAYRASTER();
	}
}
private: System::Void checkBox_MergeScantrace_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (this->checkBox_MergeScantrace->CheckState == CheckState::Checked) {
		imagemerge->displaywithscan = true;
	}
	else if (this->checkBox_MergeScantrace->CheckState == CheckState::Unchecked) {
		imagemerge->displaywithscan = false;
	}
	imagemerge->DISPLAYRASTER();
}
private: System::Void button_Merge_and_Display_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->checkBox_save->CheckState == CheckState::Checked) {
		imagemerge->savestate = true;
	}
	else if (this->checkBox_save->CheckState == CheckState::Unchecked) {
		imagemerge->savestate = false;
	}
	imagemerge->RLMERGEDISPLAY();
}
private: System::Void button_right_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->offsetx = imagemerge->offsetx + 1;
	this->textBox_offsetx->Text = System::Convert::ToString(imagemerge->offsetx);
	imagemerge->DISPLAYRASTER();
}
private: System::Void button_left_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->offsetx = imagemerge->offsetx - 1;
	this->textBox_offsetx->Text = System::Convert::ToString(imagemerge->offsetx);
	imagemerge->DISPLAYRASTER();
}
private: System::Void button_up_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->offsety = imagemerge->offsety + 1;
	this->textBox_offsety->Text = System::Convert::ToString(imagemerge->offsety);
	imagemerge->DISPLAYRASTER();
}
private: System::Void button_down_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->offsety = imagemerge->offsety - 1;
	this->textBox_offsety->Text = System::Convert::ToString(imagemerge->offsety);
	imagemerge->DISPLAYRASTER();
}
private: System::Void trackBar_resizex_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double factor = double(this->trackBar_resizex->Value)/100;
	this->textBox_resizex->Text = this->trackBar_resizex->Value.ToString();
	imagemerge->upscalefactorX = 1/factor;
	imagemerge->DISPLAYRASTER();
}
private: System::Void trackBar_resizey_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double factor = double(this->trackBar_resizey->Value) / 100;
	this->textBox_resizey->Text = this->trackBar_resizey->Value.ToString();
	imagemerge->upscalefactorY = 1 / factor;
	imagemerge->DISPLAYRASTER();
}
private: System::Void button_fastset_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->offsetx = System::Convert::ToInt16(this->textBox_offsetx->Text);
	imagemerge->offsety = System::Convert::ToInt16(this->textBox_offsety->Text);

	this->trackBar_resizex->Value = System::Convert::ToInt32(this->textBox_resizex->Text);
	double factorx = System::Convert::ToDouble(this->textBox_resizex->Text) / 100;
	imagemerge->upscalefactorX = 1 / factorx;

	this->trackBar_resizey->Value = System::Convert::ToInt32(this->textBox_resizey->Text);
	double factory = System::Convert::ToDouble(this->textBox_resizey->Text) / 100;
	imagemerge->upscalefactorY = 1 / factory;

	imagemerge->DISPLAYRASTER();
}

//Transter to .tiff
private: System::Void button_LOADLISSAJOUS_RAWBIN_Click_1(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_LISSAJOUSRAW_FOLDER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_LISSAJOUSRAW_FOLDER->SelectedPath;
		this->textBox_LOADLISSAJOUSBIN->Text = selectedPath;
		imagemerge->LISSAJOUSFolderPath = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->LOADspec(imagemerge->LISSAJOUSFolderPath);
		this->textBox_lissajousfreqx->Text = imagemerge->xfreq.ToString();
		this->textBox_lissajousfreqy->Text = imagemerge->yfreq.ToString();
		this->textBox_lissajousphase->Text = imagemerge->lissajous_phase.ToString();
		this->textBox_numofsample->Text = imagemerge->num_of_sample.ToString();
		this->textBox_samplingrate->Text = imagemerge->samplingrate.ToString();
		this->button_TRANSFERTOTIFF->Enabled = true;
		this->label_TRANSFERTOTIFF->Text = "--";
	}
}
private: System::Void button_TRANSFERTOTIFF_Click_1(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->ConvertBinToTiff();
	this->button_TRANSFERTOTIFF->Enabled = false;
	this->label_TRANSFERTOTIFF->Text = "Complete";
}

//Reconstruct
private: System::Void trackBar_lissajousoffset_Scroll(System::Object^ sender, System::EventArgs^ e) {
	this->trackBar_lissajousoffset->Enabled == false;
	int offset = int(this->trackBar_lissajousoffset->Value);
	if (offset > imagemerge->num_of_sample) {
		this->label_lissajousoffset->Text = "ERROR: Out of range";
	}
	else {
		this->label_lissajousoffset->Text = offset.ToString();
		imagemerge->offset_lissajous = offset;
		imagemerge->DisplayReconstructedLissajousImage();
	}
	this->trackBar_lissajousoffset->Enabled == true;
	//this->label_lissajousoffset->Text = offset.ToString();
	//imagemerge->offset_lissajous = offset;
	//imagemerge->DisplayReconstructedLissajousImage();
}
private: System::Void button_LOADLISSAJOUS_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_LISSAJOUS->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_LISSAJOUS->FileName;
		this->textBox_LOADLISSAJOUS->Text = selectedPath;
		imagemerge->LISSAJOUSPath = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->LoadLissajousImage();
	}
}
private: System::Void button_LOADLISSAJOUSFOLDER2_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_LISSAJOUSRAW_FOLDER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_LISSAJOUSRAW_FOLDER->SelectedPath;
		this->textBox__LOADLISSAJOUSFOLDER2->Text = selectedPath;
		imagemerge->LISSAJOUSFolderPath = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->LOADspec(imagemerge->LISSAJOUSFolderPath);
		imagemerge->GenerateSampleMap();
		this->textBox_lissajousfreqx->Text = imagemerge->xfreq.ToString();
		this->textBox_lissajousfreqy->Text = imagemerge->yfreq.ToString();
		this->textBox_lissajousphase->Text = imagemerge->lissajous_phase.ToString();
		this->textBox_numofsample->Text = imagemerge->num_of_sample.ToString();
		this->textBox_samplingrate->Text = imagemerge->samplingrate.ToString();
		imagemerge->DisplayReconstructedLissajousImage();
	}
}
private: System::Void textBox_lissajouspixelnum_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->pixelnum = System::Convert::ToInt32(this->textBox_lissajouspixelnum->Text);
}
private: System::Void button_setphaseandsave_Click(System::Object^ sender, System::EventArgs^ e) {
	System::Threading::Thread^ startreconstruct = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(this, &YSPostprocessing::StartReconstruct));
	startreconstruct->Start();
}
private: void StartReconstruct() {
	imagemerge->ReconstructAllTiffInFolder();  
}
//Calculate Parameters
private: System::Void button_CalPara_LOADRASTER_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_RASTER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_RASTER->FileName;
		this->textBox_CalPara_LOADRASTER->Text = selectedPath;

		this->comboBox_CalPara_samplingrate->Enabled = false;
		this->textBox_CalPara_xpixel->Enabled = false;
		this->textBox_CalPara_ypixel->Enabled = false;

		int width = System::Convert::ToInt16(this->textBox_CalPara_xpixel->Text);
		int height = System::Convert::ToInt16(this->textBox_CalPara_ypixel->Text);
		double internal_clock_freq_array[6] = { 2, 10,20,50,100,125 };
		double samplingrate = internal_clock_freq_array[this->comboBox_CalPara_samplingrate->SelectedIndex] *1000000.0;
		std::string path = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->CreateCUDACalPara(path,width,height,samplingrate);
	}
}
private: System::Void button_CalPara_calculate_Click(System::Object^ sender, System::EventArgs^ e) {
	double internal_clock_freq_array[6] = { 2, 10,20,50,100,125 };
	double samplingrate = internal_clock_freq_array[this->comboBox_CalPara_samplingrate->SelectedIndex] * 1000000.0;
	if (this->checkBox_CalPara_linepoints->Checked == true) {
		imagemerge->CalculateParameter_multi(samplingrate);
	}
	else if (this->checkBox_CalPara_linepoints->Checked == false) {
		imagemerge->CalculateParameter(samplingrate);
	}
	tableLayoutPanel_CalPara_result->SuspendLayout();
	tableLayoutPanel_CalPara_result->Controls->Clear();
	tableLayoutPanel_CalPara_result->ColumnStyles->Clear();
	tableLayoutPanel_CalPara_result->RowStyles->Clear();
	tableLayoutPanel_CalPara_result->CellBorderStyle =TableLayoutPanelCellBorderStyle::Single;
	int hitCnt = 0;
	for (int i = 0; i < NTU::TOTAL_PARAMS; ++i)
		if (cudacalpara->resultHost[i]) ++hitCnt;
	tableLayoutPanel_CalPara_result->ColumnCount = 5;
	tableLayoutPanel_CalPara_result->RowCount = hitCnt + 1;
	for (int c = 0; c < 5; ++c)
		tableLayoutPanel_CalPara_result->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 100.0f / 5));
	for (int r = 0; r < hitCnt + 1; ++r)
		tableLayoutPanel_CalPara_result->RowStyles->Add(gcnew RowStyle(SizeType::Absolute, 20));
	std::string header[5] = {"fx", "fy", "phase", "fps", "num"};
	for (int c = 0; c < 5; ++c)	{
		auto lbl = gcnew Label();
		lbl->AutoSize = false;
		lbl->Text = gcnew System::String(header[c].c_str());
		lbl->Font = gcnew System::Drawing::Font("Consolas", 10, FontStyle::Bold);
		lbl->Dock = DockStyle::Fill;
		tableLayoutPanel_CalPara_result->Controls->Add(lbl, c, 0);
	}
	int row = 1;
	for (int i = 0; i < NTU::TOTAL_PARAMS; ++i)	{
		if (!cudacalpara->resultHost[i]) continue;
		double fx = cudacalpara->fxHost[i];
		double fy = cudacalpara->fyHost[i];
		double phase = cudacalpara->phaseHost[i];
		double fps = cudacalpara->fpsHost[i];
		double times = cudacalpara->timesperframeHost[i];

		double data[5] = {fx, fy, phase, fps, times};

		for (int c = 0; c < 5; ++c){
			auto lbl = gcnew Label();
			lbl->Font = gcnew System::Drawing::Font("Consolas", 10);
			lbl->Dock = DockStyle::Fill;
			System::String^ fmt = (c == 2) ? "{0:F3}" : "{0:0}";
			lbl->Text = System::String::Format(fmt, data[c]);
			tableLayoutPanel_CalPara_result->Controls->Add(lbl, c, row);
		}
		++row;
	}
	tableLayoutPanel_CalPara_result->ResumeLayout(true);
}
private: System::Void button_CalPara_up_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->moveTarget(0, -1);
}
private: System::Void button_CalPara_down_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->moveTarget(0, 1);
}
private: System::Void button_CalPara_right_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->moveTarget(1, 0);
}
private: System::Void button_CalPara_left_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->moveTarget(-1, 0);
}
private: System::Void toggleButtons_CalPara_twopoints_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (this->toggleButtons_CalPara_twopoints->Checked) {
		cudacalpara->TwoPointsCal = true;
		this->label_CalPara_twopoints->Text = "Calculate 2 points";
	}
	else{
		cudacalpara->TwoPointsCal = false;
		this->label_CalPara_twopoints->Text = "Calculate 1 point";
	}
	imagemerge->moveTarget(0, 0);
}
private: System::Void checkBox_CalPara_linepoints_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (this->checkBox_CalPara_linepoints->Checked) {
		cudacalpara->LineCal = true;
		this->checkBox_CalPara_linepoints->Text = "Line Calculate";
	}
	else {
		cudacalpara->LineCal = false;
		this->checkBox_CalPara_linepoints->Text = "Points Calculate";
	}
	imagemerge->moveTarget(0, 0);
}
//Scan Preview
private: System::Void button_ScanPreview_LOADRASTER_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_RASTER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_RASTER->FileName;
		this->textBox_ScanPreview_LOADRASTER->Text = selectedPath;

		this->comboBox_ScanPreview_samplingrate->Enabled = false;
		this->textBox_ScanPreview_xpixel->Enabled = false;
		this->textBox_ScanPreview_ypixel->Enabled = false;

		imagemerge->width_SP = System::Convert::ToInt16(this->textBox_ScanPreview_xpixel->Text);
		imagemerge->height_SP = System::Convert::ToInt16(this->textBox_ScanPreview_ypixel->Text);
		double internal_clock_freq_array[6] = { 2, 10,20,50,100,125 };
		imagemerge->samplingrate_SP = internal_clock_freq_array[this->comboBox_ScanPreview_samplingrate->SelectedIndex] * 1000000.0;
		std::string path = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->CreateScanPreview(path);
		imagemerge->ScanmodePreview();
	}
}
private: System::Void comboBox_ScanPreview_xyfreq_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	int YSxfreq_array_ScanPreview[9] = { 1500,2000,2000,2100,2088,2052,1980,1620,2000 };
	int YSyfreq_array_ScanPreview[9] = { 2000,2010,2100,2160,2160,2160,2160,2160,3000 };
	int YSfps_array_ScanPreview[9] = { 500,10,100,60,72,108,180,540,1000 };
	imagemerge->xfreq_SP = YSxfreq_array_ScanPreview[this->comboBox_ScanPreview_xyfreq->SelectedIndex];
	imagemerge->yfreq_SP = YSyfreq_array_ScanPreview[this->comboBox_ScanPreview_xyfreq->SelectedIndex];
	imagemerge->framerate_SP = YSfps_array_ScanPreview[this->comboBox_ScanPreview_xyfreq->SelectedIndex];
	this->textBox_ScanPreview_framerate->Text = imagemerge->framerate_SP.ToString();
	if (imagemerge->loaddata == true) {
		imagemerge->ScanmodePreview();
	}
}
private: System::Void button_ScanPreview_rightphase_Click(System::Object^ sender, System::EventArgs^ e) {
	double YSphase_array_ScanPreview[8] = { 0 ,M_PI_4, M_PI_2 ,3 * M_PI_4, M_PI , 5 * M_PI_4,  3 * M_PI_2, 7 * M_PI_4 };
	std::string YSphasename_array_ScanPreview[8] = { "0","PI/4" ,"PI/2","3PI/4" ,"PI","5PI/4", "3PI/2","7PI/4" };
	if ((imagemerge->phaseindex_SP + 1) < 8) {
		imagemerge->phaseindex_SP++;
	}
	imagemerge->phase_SP = YSphase_array_ScanPreview[imagemerge->phaseindex_SP];
	this->textBox_ScanPreview_phase->Text = gcnew System::String(YSphasename_array_ScanPreview[imagemerge->phaseindex_SP].c_str());
	imagemerge->ScanmodePreview();
}
private: System::Void button_ScanPreview_leftphase_Click(System::Object^ sender, System::EventArgs^ e) {
	double YSphase_array_ScanPreview[8] = { 0 ,M_PI_4, M_PI_2 ,3 * M_PI_4, M_PI , 5 * M_PI_4,  3 * M_PI_2, 7 * M_PI_4 };
	std::string YSphasename_array_ScanPreview[8] = { "0","PI/4" ,"PI/2","3PI/4" ,"PI","5PI/4", "3PI/2","7PI/4" };
	if ((imagemerge->phaseindex_SP - 1) > -1 ) {
		imagemerge->phaseindex_SP--;
	}
	imagemerge->phase_SP = YSphase_array_ScanPreview[imagemerge->phaseindex_SP];
	this->textBox_ScanPreview_phase->Text = gcnew System::String(YSphasename_array_ScanPreview[imagemerge->phaseindex_SP].c_str());
	imagemerge->ScanmodePreview();
}
//Extract Kymograph
private: System::Void button_LOADRASTER2_Click(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_RASTER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_RASTER->FileName;
		this->textBox_LOADRASTER_EK->Text = selectedPath;
		imagemerge->RASTERPath_EK = msclr::interop::marshal_as<std::string>(selectedPath);
	}
}
private: System::Void button_LOADLISSAJOUS_Click_1(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_LISSAJOUS->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_LISSAJOUS->FileName;
		this->textBox_LOADLISSAJOUS_EK->Text = selectedPath;
		imagemerge->LISSAJOUSPath_EK = msclr::interop::marshal_as<std::string>(selectedPath);
	}
}
private: System::Void button_LOADLISSAJOUSFOLDER2_Click_1(System::Object^ sender, System::EventArgs^ e) {
	if (this->LOAD_LISSAJOUS_FOLDER->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		System::String^ selectedPath = this->LOAD_LISSAJOUS_FOLDER->SelectedPath;
		this->textBox_LOADLISSAJOUSFOLDER_EK->Text = selectedPath;
		imagemerge->LISSAJOUSFolderPath_EK = msclr::interop::marshal_as<std::string>(selectedPath);
		imagemerge->LOADspec_EK(imagemerge->LISSAJOUSFolderPath_EK);
		this->textBox_lissajousfreqx->Text = imagemerge->xfreq_EK.ToString();
		this->textBox_lissajousfreqy->Text = imagemerge->yfreq_EK.ToString();
		this->textBox_lissajousphase->Text = imagemerge->lissajous_phase_EK.ToString();
		this->textBox_numofsample->Text = imagemerge->num_of_sample_EK.ToString();
		this->textBox_samplingrate->Text = imagemerge->samplingrate_EK.ToString();
		imagemerge->LoadLissajousAndMerge();
	}
}
private: System::Void trackBar_ExtractKymograph_startpixel_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double percentage = double(this->trackBar_ExtractKymograph_startpixel->Value) / 10000;
	this->textBox_ExtractKymograph_startpixel->Text = percentage.ToString();
	imagemerge->startPixelIndexpercentage = percentage;
	imagemerge->DrawLissajousPattern();
}
private: System::Void trackBar_ExtractKymograph_endpixel_Scroll(System::Object^ sender, System::EventArgs^ e) {
	double percentage = double(this->trackBar_ExtractKymograph_endpixel->Value) / 10000;
	this->textBox_ExtractKymograph_endpixel->Text = percentage.ToString();
	imagemerge->endPixelIndexpercentage = percentage;
	imagemerge->DrawLissajousPattern();
}
private: System::Void button_ExtractKymograph_quickset_Click(System::Object^ sender, System::EventArgs^ e) {
	double startpercentage = System::Convert::ToDouble(this->textBox_ExtractKymograph_startpixel->Text);
	this->trackBar_ExtractKymograph_startpixel->Value = startpercentage * 10000;
	double endpercentage = System::Convert::ToDouble(this->textBox_ExtractKymograph_endpixel->Text);
	this->trackBar_ExtractKymograph_endpixel->Value = endpercentage * 10000;
	imagemerge->startPixelIndexpercentage = startpercentage;
	imagemerge->endPixelIndexpercentage = endpercentage;
	imagemerge->DrawLissajousPattern();
}
private: System::Void button_ExtractKymograph_extractkymograph_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->ExtractKymograph();
}
private: System::Void toggleButtons_ExtractKymograph_choosedisplay_CheckedChanged(System::Object^ sender, System::EventArgs^ e) {
	if (this->toggleButtons_ExtractKymograph_choosedisplay->Checked == true) {
		this->label_ExtractKymograph_choosedisplay->Text = "Draw Lissajous pattern";
		double startpercentage = System::Convert::ToDouble(this->textBox_ExtractKymograph_startpixel->Text);
		this->trackBar_ExtractKymograph_startpixel->Value = startpercentage * 10000;
		double endpercentage = System::Convert::ToDouble(this->textBox_ExtractKymograph_endpixel->Text);
		this->trackBar_ExtractKymograph_endpixel->Value = endpercentage * 10000;
		imagemerge->startPixelIndexpercentage = startpercentage;
		imagemerge->endPixelIndexpercentage = endpercentage;
		imagemerge->DrawLissajousPattern();
	}
	else if (this->toggleButtons_ExtractKymograph_choosedisplay->Checked == false) {
		this->label_ExtractKymograph_choosedisplay->Text = "Display Lissajous image";
		imagemerge->LoadLissajousAndMerge();
	}
}


private: System::Void timer_refresh_UI_Tick(System::Object^ sender, System::EventArgs^ e) {
	this->progressBar_reconstruct->Value = int(imagemerge->progresspercentage);
	this->label_reconstruct->Text = imagemerge->progresspercentage.ToString() + "%";
	this->progressBar_scanmodemerge->Value = int(imagemerge->progresspercentage_scanmodemerge);
	this->label_scanmodemerge->Text = imagemerge->progresspercentage_scanmodemerge.ToString() + "%";
	if (imagemerge->calpara != nullptr) {
		this->textBox_CalPara_locx->Text = cudacalpara->xTarget.ToString();
		this->textBox_CalPara_locy->Text = cudacalpara->yTarget.ToString();
		if (cudacalpara->TwoPointsCal == true) {
			this->textBox_CalPara_locx2->Text = cudacalpara->xTarget2.ToString();
			this->textBox_CalPara_locy2->Text = cudacalpara->yTarget2.ToString();
		}
	}

}

private: System::Void button_shut_down_Click(System::Object^ sender, System::EventArgs^ e) {
	imagemerge->~YSImageMerge();
}





};
}
