#pragma once

namespace MTPFM {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Loading
	/// </summary>
	public ref class Loading : public System::Windows::Forms::Form
	{
	public:
		Loading(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			//progressBar_loading->ForeColor = Color::RoyalBlue;
			this->ControlBox = false;

			// ENABLING CROSS THREAD CALLS
			CheckForIllegalCrossThreadCalls = false;

			// Bring to top
			this->TopMost = true;
		}
	
	public:
		void send_progress(int val)
		{
			if (val == -1)	this->Close();
			else			progressBar_loading->Value = val;
			Application::DoEvents();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Loading()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ProgressBar^ progressBar_loading;
	protected:

	private: System::Windows::Forms::PictureBox^ pictureBox_SPLASH;
	private: System::Windows::Forms::PictureBox^ pictureBox1;
	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(Loading::typeid));
			this->progressBar_loading = (gcnew System::Windows::Forms::ProgressBar());
			this->pictureBox_SPLASH = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox_SPLASH))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// progressBar_loading
			// 
			this->progressBar_loading->BackColor = System::Drawing::SystemColors::Control;
			this->progressBar_loading->ForeColor = System::Drawing::Color::Navy;
			this->progressBar_loading->Location = System::Drawing::Point(-1, 378);
			this->progressBar_loading->Name = L"progressBar_loading";
			this->progressBar_loading->Size = System::Drawing::Size(700, 20);
			this->progressBar_loading->Step = 1;
			this->progressBar_loading->TabIndex = 1;
			// 
			// pictureBox_SPLASH
			// 
			this->pictureBox_SPLASH->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->pictureBox_SPLASH->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pictureBox_SPLASH.Image")));
			this->pictureBox_SPLASH->Location = System::Drawing::Point(-1, -1);
			this->pictureBox_SPLASH->Name = L"pictureBox_SPLASH";
			this->pictureBox_SPLASH->Size = System::Drawing::Size(700, 380);
			this->pictureBox_SPLASH->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
			this->pictureBox_SPLASH->TabIndex = 2;
			this->pictureBox_SPLASH->TabStop = false;
			// 
			// pictureBox1
			// 
			this->pictureBox1->BackColor = System::Drawing::SystemColors::ActiveCaptionText;
			this->pictureBox1->BackgroundImage = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pictureBox1.BackgroundImage")));
			this->pictureBox1->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pictureBox1.Image")));
			this->pictureBox1->Location = System::Drawing::Point(209, 93);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(260, 279);
			this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::CenterImage;
			this->pictureBox1->TabIndex = 3;
			this->pictureBox1->TabStop = false;
			// 
			// Loading
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(7, 15);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::Gray;
			this->BackgroundImageLayout = System::Windows::Forms::ImageLayout::Stretch;
			this->ClientSize = System::Drawing::Size(700, 400);
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->pictureBox_SPLASH);
			this->Controls->Add(this->progressBar_loading);
			this->DoubleBuffered = true;
			this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"Loading";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Loading";
			this->TopMost = true;
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox_SPLASH))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion


};
}
