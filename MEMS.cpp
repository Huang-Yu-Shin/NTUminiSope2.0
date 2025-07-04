#pragma once
#include "NTUcore.h"


namespace NTU
{
	void NTUscope::memsControllerConfig()
	{
		// Connect and configure the controller to the correct modes
		// Connect to Controller located at COM3
		int portnumber = 5;
		char* comPortName = new char[12];
		sprintf(comPortName, "COM%d", portnumber);
		mti->ConnectDevice(comPortName);

		MTIError lastError = mti->GetLastError();
		if (lastError != MTIError::MTI_SUCCESS)
		{
			printf("\n\nUnable to connect with any device at port %s.  Press any key to Exit.\n", comPortName);
			mti->SendSerialReset();
			_getch();
			// Show error message if not successfully connected
			printf("Unsuccessfully connect to MEMS controller!");
		}

		// Inptr of file
		char* iniFile = "CurrentMemsPara.ini";
		
		// Get current info and parameters from controller
		MTIDeviceParams params;
		mti->LoadDeviceParams(&params, iniFile);
		params.DataMode = MTIDataMode::Sample_Output;			// This is the default mode, shown here for demo
		params.DataScale = 1.0f;								// Scale data/content to 80%
		params.SampleRate = 100000;
		params.DeviceAxes = MTIAxes::Normal;					// Normal X,Y Axes Definition.
		params.SyncMode = MTISync::Output_DOut0;
		params.Vbias = 90;
		params.VdifferenceMax = 179;
		params.XOffset = xOffset;
		params.YOffset = yOffset;
		params.HardwareFilterBw = 2200;
		// Another way to apply parameters one at a time without
		// the use of the MTIDeviceParams structures above is:
		mti->SetDeviceParam(MTIParam::DataScale, 1);
		mti->SetDeviceParam(MTIParam::DataRotation, 0);
		// Now, set the controller's params with the params in the structure
		mti->SetDeviceParams(&params);
		lastError = mti->GetLastError();

		// Now we can check if all the parameters we wanted are set on the controller
		mti->GetDeviceParams(&params);	
		mti->SetDeviceParam(MTIParam::MEMSDriverEnable, true);		// Turn the MEMS Driver on 
		
		mti->SaveDeviceParams(iniFile);
		//std::string version; mti->GetAPIVersion(&version);
		//printf("Device Name: %s     Firmware Name: %s     API version %s\n", lparams.DeviceName, lparams.FirmwareName, version.c_str());
		//mti->GetDeviceParams(&lparams);		// Get current info and parameters from controller
		//printf("Vbias = %3.1fV     VdifferenceMax = %3.1fV     HardwareFilterBw = %dHz\n\n", lparams.Vbias, lparams.VdifferenceMax, lparams.HardwareFilterBw);
	}

	void NTUscope::Bi_RasterScan(MTIDevice* mti, float xAmp, float yAmp, int y_length)
	{
		MTIDataGenerator* datagen = new MTIDataGenerator();		// Handle for MTIDataGenerator class with a wide variety of content generation methods.

		unsigned int sps = 100000, spshold;		// Sample rate (samples per second/sps) setting
		unsigned int numPixels = 3;		// Number of stop-and-go locations along a line in ppRaster=1 mode, or number of synchronous digital output shots along a line.
		bool ppMode = false;				// 0 = uniform velocity line raster | 1 = Point-to-point raster with near stopping at each point.
		bool retrace = true;			// 0 = Fast axis uniform scan in one direction, then fast retrace back | 1 = Fast axis uniform scan in both directions.
		int triggerShift = 0;			// Number of samples the synchronous digital output should be rotated with respect to the MEMS driver output (can be negative or positive).
		float theta = M_PI/2;			// Rotation angle for the raster pattern (looking toward display surface). Default results in vertical lines which move from left to right on the display.

		MTIDeviceParams params;
		mti->GetDeviceParams(&params);
		int spsMax = params.DeviceLimits.SampleRate_Max;
		int spsMin = params.DeviceLimits.SampleRate_Min;
		int npts = params.DeviceLimits.SamplesPerFrame_Max;	// Number of points (samples) to send to the device - starting guess is maximum allowed by DeviceLimits
		unsigned char m_Data = 255;

		float* xData;
		float* yData;
		unsigned char* mData;
		xData = new float[npts]; yData = new float[npts];	mData = new unsigned char[npts];

		try
		{
			mti->SetDeviceParam(MTIParam::SyncMode, MTISync::Output_Start_Trigger);
			mti->SetDeviceParam(MTIParam::SampleRate, sps);
			// This is the the primary function - LinearRasterPattern creates a full waveform for the raster based on the given parameters
			npts = datagen->LinearRasterPattern(xData, yData, mData, xAmp, yAmp, y_length, numPixels, lineTime, ppMode, retrace, triggerShift, theta, sps, spsMin, spsMax);

			printf("Prepared a raster scan with %d lines x %d pixels = %d points.", y_length, numPixels, npts);
			printf("\nScan is running with a %d samples per second rate.", sps);
			printf("\nEach line has %4.4fs active duration and overall scan repeats every %4.4fs.\n\n", lineTime, (float)npts / (float)sps);
			
			mti->SendDataStream(xData, yData, mData, npts, 0, true);

			//printf("Press any key to stop the waveform and device scanning...\n"); _getch();
			//mti->ResetDevicePosition();					// Stop then send analog outputs (and device) back to origin in 25ms
			//mti->SetDeviceParam(MTIParam::SampleRate, spshold);	// Restore the sample rate that the Controller had before this function
		}
		catch (...)
		{
			printf("Error in Raster Demo - Press any key to return to the main menu.\n");
		}

		SAFE_DELETE_ARRAY(xData);	SAFE_DELETE_ARRAY(yData);	SAFE_DELETE_ARRAY(mData);
		SAFE_DELETE(datagen);

	}

	void NTUscope::Uni_RasterScan(MTIDevice* mti, float xAmp, float yAmp, int y_length)
	{
		MTIDataGenerator* datagen = new MTIDataGenerator();		// Handle for MTIDataGenerator class with a wide variety of content generation methods.

		unsigned int sps = 100000, spshold;		// Sample rate (samples per second/sps) setting
		unsigned int numPixels = 3;		// Number of stop-and-go locations along a line in ppRaster=1 mode, or number of synchronous digital output shots along a line.
		bool ppMode = false;				// 0 = uniform velocity line raster | 1 = Point-to-point raster with near stopping at each point.
		bool retrace = false;			// 0 = Fast axis uniform scan in one direction, then fast retrace back | 1 = Fast axis uniform scan in both directions.
		int triggerShift = 0;			// Number of samples the synchronous digital output should be rotated with respect to the MEMS driver output (can be negative or positive).
		float theta = M_PI / 2;			// Rotation angle for the raster pattern (looking toward display surface). Default results in vertical lines which move from left to right on the display.

		MTIDeviceParams params;
		mti->GetDeviceParams(&params);
		int spsMax = params.DeviceLimits.SampleRate_Max;
		int spsMin = params.DeviceLimits.SampleRate_Min;
		int npts = params.DeviceLimits.SamplesPerFrame_Max;	// Number of points (samples) to send to the device - starting guess is maximum allowed by DeviceLimits
		unsigned char m_Data = 255;

		float* xData;
		float* yData;
		unsigned char* mData;
		xData = new float[npts]; yData = new float[npts];	mData = new unsigned char[npts];

		try
		{
			mti->SetDeviceParam(MTIParam::SyncMode, MTISync::Output_Start_Trigger);
			mti->SetDeviceParam(MTIParam::SampleRate, sps);
			// This is the the primary function - LinearRasterPattern creates a full waveform for the raster based on the given parameters
			npts = datagen->LinearRasterPattern(xData, yData, mData, xAmp, yAmp, y_length, numPixels, lineTime, ppMode, retrace, triggerShift, theta, sps, spsMin, spsMax);

			printf("Prepared a raster scan with %d lines x %d pixels = %d points.", y_length, numPixels, npts);
			printf("\nScan is running with a %d samples per second rate.", sps);
			printf("\nEach line has %4.4fs active duration and overall scan repeats every %4.4fs.\n\n", lineTime, (float)npts / (float)sps);

			mti->SendDataStream(xData, yData, mData, npts, 0, true);

			//printf("Press any key to stop the waveform and device scanning...\n"); _getch();
			//mti->ResetDevicePosition();					// Stop then send analog outputs (and device) back to origin in 25ms
			//mti->SetDeviceParam(MTIParam::SampleRate, spshold);	// Restore the sample rate that the Controller had before this function
		}
		catch (...)
		{
			printf("Error in Raster Demo - Press any key to return to the main menu.\n");
		}

		SAFE_DELETE_ARRAY(xData);	SAFE_DELETE_ARRAY(yData);	SAFE_DELETE_ARRAY(mData);
		SAFE_DELETE(datagen);

	}

	void NTUscope::memsSync()
	{
		if (MEMS_status == true)
		{
			mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
			if (bi_direction == true)
			{
				Bi_RasterScan(mti, xAmp, yAmp, y_length);
			}
			else
			{
				Uni_RasterScan(mti, xAmp, yAmp, y_length);
			}
			
		}
	}

	// Get the COM port which I've connected
	int NTUscope::scanInt() {
		// Input function to handle user input across the MTIDevice-Demo
		char inptr[256], * endptr;
		fgets(inptr, 256, stdin);					// Get string from stdin and store in inptr
		int val = strtol(inptr, &endptr, 0);		// Get integer value of inptr string
		// strtol returns 0 on invalid input but if the pointers are the same then no conversion occurred
		if (inptr == endptr)
			return -1;
		else
			return val;
	}
	
	// Select the MEMS controller
	bool NTUscope::SelectIODevice(MTIDevice* mti, char* comPortName)
	{
		unsigned int portnumber = 0;

		printf("\nSearching for available Mirrorcle MEMS Controller devices. Please wait...\n");

		// Call to GetAvailableDevices returns table of devices and respective COM ports
		MTIAvailableDevices table;
		mti->GetAvailableDevices(&table);

		if (table.NumDevices == 0) {
			printf("\nThere are no devices available. Press any key to exit...");
			return false;
		}

		system(CLEARSCREEN);
		mti->ListAvailableDevices(&table);
		printf("\n\n");
		printf("Input serial port number to connect to (e.g. %d): \n", table.CommPortNumber[0]);
		portnumber = scanInt();
		#ifdef MTI_WINDOWS
				sprintf_s(comPortName, 40, MTI_PORT_PREFIX "%d", portnumber);
		#else
				snprintf(comPortName, 40, MTI_PORT_PREFIX "%d", portnumber);
		#endif

		return true;
	}
	
	void NTUscope::CalculateSinglePointTrackingParameters(unsigned int& hardwareFilterBw, unsigned int& sampleRate, float& dataScale, unsigned int& frameRate)
	{
		if (hardwareFilterBw >= 1000) {
			sampleRate = 80000;
			dataScale = dataScale * 1.f;
			frameRate = 200;
		}
		else if (hardwareFilterBw < 1000 && hardwareFilterBw >= 300) {
			sampleRate = 60000;
			dataScale = std::min(1.f, dataScale * 1.2f);
			frameRate = 100;
		}
		else {
			sampleRate = 30000;
			dataScale = std::min(1.f, dataScale * 1.6f);
			frameRate = 50;
		}
		hardwareFilterBw = std::max(CalculateMinHwBw(sampleRate), hardwareFilterBw);
	}

	void NTUscope::SpiralCurve(MTIDevice* mti, float xAmplitude, float yAmplitude, unsigned int frequency)
	{
		MTIDataGenerator* datagen = new MTIDataGenerator();		// Handle for MTIDataGenerator class with a wide variety of content generation methods.
		//unsigned int sps = 20000, spshold;	// Sample rate (samples per second/sps) setting
		printf("xAmplitude = %lf\n", yAmplitude);
		printf("yAmplitude = %lf\n", yAmplitude);
		printf("Num Loops = %d\n", frequency);
		//float yAmplitude = 0.9;				// Amplitude for Y-Axis scan (0.0 to 1.0).
		//unsigned int frequency = 5;
		float phase = M_PI_2;

		MTIDeviceParams params;
		mti->GetDeviceParams(&params);
		int spsMax = params.DeviceLimits.SampleRate_Max;
		int spsMin = params.DeviceLimits.SampleRate_Min;
		int npts = params.DeviceLimits.SamplesPerFrame_Max;	// Number of points (samples) to send to the device - starting guess is maximum allowed by DeviceLimits

		float* xData, * yData;
		unsigned char* mData;
		xData = new float[npts]; yData = new float[npts];	mData = new unsigned char[npts];

		try
		{
			npts = datagen->SpiralCurve(xData, yData, mData, numPoints, xAmplitude, yAmplitude, frequency, phase);
			//spshold = mti->GetDeviceParam(MTIParam::SampleRate);		// Preserve the Controller's current sample rate to restore later.
			//mti->SetDeviceParam(MTIParam::SampleRate, sps);
			mti->ResetDevicePosition();
			mti->SendDataStream(xData, yData, mData, npts, 0, true);
			datagen->ExportFile("Datalog\\spiralcurve.txt", xData, yData, mData, numPoints, 100000);
			//printf("Press any key to stop the waveform and device scanning...\n"); 
			//mti->ResetDevicePosition();					// Stop then send analog outputs (and device) back to origin in 25ms
			//int max = 7000;
			ofstream out; 
			out.open("Datalog\\input.txt"); 
			if (out.fail()) { 
				cout << "input file opening failed...";
				exit(1);
			}

			for (int i = 0; i < numPoints; i++) {
				/*coor_x[i] = int(xData[i] / abs(xData[1])) + max;
				coor_y[i] = int(yData[i] / abs(xData[1])) + max;*/
				if (abs(xData[i]) > max) { max = abs(xData[i]); };
				if (abs(yData[i]) > max) { max = abs(yData[i]); };
				coor_x[i] = int((xData[i] + 1) / abs(xData[1]));
				coor_y[i] = int((yData[i] + 1) / abs(xData[1]));
				out << xData[i] << " " << yData[i]<<endl;
			}
			printf("max:%lf\n", max);
			minCoor = int(1 / abs(xData[1]));
			printf("test:%d\n", minCoor);
			//printf("%lf", xData[1]);
			//for (int i = 0; i < 10000; i++) {
			//	//printf("x = %d,y = %d", aNTU->coor_x[i], aNTU->coor_y[i]);
			//	if (coor_x[i] > max) {
			//		max = coor_x[i];
			//		if (coor_y[i] > max) {
			//			max = coor_y[i];
			//		}
			//	}
			//}

		}
		catch (...)
		{
			printf("Error in Raster Demo - Press any key to return to the main menu.\n"); //_getch();
		}

		SAFE_DELETE_ARRAY(xData);	SAFE_DELETE_ARRAY(yData);	SAFE_DELETE_ARRAY(mData);
		SAFE_DELETE(datagen);
	}

	// Checked how large the hardware bandwidth we need to use 
	unsigned int NTUscope::CalculateMinHwBw(unsigned int sampleRate)
	{
		unsigned int maxDelay = 191;
		return (100 * sampleRate) / ((158 * maxDelay) - 632);
	}

	// Prepared for the imaging
	void NTUscope::CalculateImagingParameters(unsigned int& hardwareFilterBw, unsigned int& sampleRate, float& dataScale, float& lineTime, unsigned int& nLines, unsigned int& nPixels)
	{
		if (hardwareFilterBw >= 1000) {
			sampleRate = 60000;
			nLines = 60;
			nPixels = 200;
			dataScale = dataScale * 1.f;
			lineTime = 0.0038f;
		}
		else if (hardwareFilterBw < 1000 && hardwareFilterBw >= 300) {
			sampleRate = 40000;
			nLines = 55;
			nPixels = 170;
			dataScale = std::min(1.f, dataScale * 1.2f);
			lineTime = 0.01f;
		}
		else {
			sampleRate = 30000;
			nLines = 50;
			nPixels = 140;
			dataScale = std::min(1.f, dataScale * 1.6f);
			lineTime = 0.02f;
		}
		hardwareFilterBw = std::max(CalculateMinHwBw(sampleRate), hardwareFilterBw);
	}

	unsigned int NTUscope::EstimateXYDelay(unsigned int SampleRate, unsigned int HardwareFilterBw)
	{
		unsigned int delay = (SampleRate * 100) / (HardwareFilterBw * 158);
		return 4 + delay;
	}

	//void NTUscope::Imaging(MTIDevice* mti, float xOffset, float yOffset, float dataScale)
	//{
	//	MTIDataGenerator* datagen = new MTIDataGenerator();

	//	unsigned int hardwareFilterBw, sampleRate, nLines, nPixels;
	//	float lineTime;
	//	hardwareFilterBw = mti->GetDeviceParam(MTIParam::HardwareFilterBw);
	//	CalculateImagingParameters(hardwareFilterBw, sampleRate, dataScale, lineTime, nLines, nPixels);

	//	mti->SetDeviceParam(MTIParam::DataScale, dataScale);
	//	mti->SetDeviceParam(MTIParam::SampleRate, sampleRate);
	//	mti->SetDeviceParam(MTIParam::HardwareFilterBw, hardwareFilterBw);

	//	unsigned int img_size_cols = 600, img_size_rows = 500;
	//	unsigned int trackPersistenceSize = 30;
	//	Rect rect;

	//	// Prepare the raster waveform
	//	// Calculate number of points based on SampleRate and frame_rate
	//	unsigned int npts = 125000;
	//	float* x, * y;
	//	unsigned char* m;
	//	x = new float[npts]; y = new float[npts]; m = new unsigned char[npts];

	//	npts = datagen->LinearRasterPatternImaging(x, y, m, 1., 1., nLines, nPixels, lineTime, true, true, 0, M_PI_2, sampleRate, 10000, 10000);
	//	mti->SetDeviceParam(MTIParam::SampleRate, sampleRate);
	//	nPixels = (int)((float)sampleRate * lineTime);

	//	// Prepare an inverse filter to increase bandwidth
	//	//datagen->LoadIIRFilterParams( "IIRFilterParams_I12_60k.dat" );		// file with specific filter coefficients here
	//	//datagen->SetupSoftwareFilter( FilterIIRGeneric, 0, 0, 0, true );		// for generic type most parameters are not used, only first and last
	//	//datagen->FilterData( x, y, x, y, npts, true ); 
	//	//mti->SetDeviceParam( MTIParam::HardwareFilterBw, 2000 );

	//	//mti->SetDeviceParam(MTIParam::LaserModulationEnable, 1);
	//	mti->SetDeviceParam(MTIParam::DataMode, MTIDataMode::Sample_And_Analog_Input_Track);
	//	mti->SendDataStream(x, y, m, npts, 0, false);

	//	// Because the PhotoSensor has inverting preamplification, laser output should be inverted from DOut data
	//	//mti->SetDeviceParam ( MTIParam::SyncMode, MTISync::Output_Start_Trigger );
	//	unsigned int len;
	//	float rotation = 0.f;
	//	int bufferdelay = 23;
	//	float out, out_keep = 0.f, threshold = 0.1;
	//	float* ai0 = new float[100000];
	//	float* indices = new float[100000];
	//	float sum = 0;
	//	float minOut = 100, maxOut = -100, maxOutLast = 20;
	//	int blur = 2, pixel_adjust = 0;

	//	bufferdelay = EstimateXYDelay(sampleRate, hardwareFilterBw) - 2;
	//	mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
	//	// Now we can start the MEMS mirror
	//	mti->StartDataStream(-1);

	//	// Prepare OpenCV window for results
	//	namedWindow("IMAGER", CV_WINDOW_AUTOSIZE);

	//	// Add for displaying image
	//	Mat image(img_size_rows, img_size_cols, CV_8UC3, Scalar(0, 0, 0)), imageout(img_size_rows, img_size_cols, CV_8UC3, Scalar(0, 0, 0));

	//	int indexOfAI, indexOfXYPosition, indexOfModulation;

	//	bool menuFlag = true, loopgo = true;
	//	int pressedKey = -1;

	//	while (loopgo) {
	//		if (menuFlag) {
	//			menuFlag = false;
	//			system(CLEARSCREEN);
	//			printf("\n\t Buffer Delay (adding(+) or subtracting(-) by 1): %d", bufferdelay);
	//			printf("\n\t DataScale	(increasing(O) or decreasing(o) by 0.02): %3.3f", dataScale);
	//			printf("\n\t Blur (increasing(B) or decreasing(b) by 1): %d", blur);
	//			printf("\n\t Use WASD Keys to offset the imaging region of interest.");
	//			printf("\n\t Press '0' to return offset and scale parameters to the initial value.");
	//			printf("\n\t X and Y Offsets: %3.3f, %3.3f", xOffset, yOffset);
	//			printf("\n\n Press ESC or SPACE to exit this demo...");
	//		}
	//		// In this DataMode, data is constantly streaming back to the PC from the Controller
	//		// Controller reports photosensor voltage as ai0 and at which index of the buffer that data was obtained
	//		// With the index we are able to recover what the laser modulation was at that time (on / off)
	//		len = mti->GetAnalogInputStream(ai0, indices, npts);
	//		// If this loops slows down or stop running data from Controller will overrun and fill the serial buffer

	//		sum = 0;

	//		image = Mat::zeros(img_size_rows, img_size_cols, CV_8UC3);
	//		imageout = Mat::zeros(img_size_rows, img_size_cols, CV_8UC3);
	//		unsigned int xpixel;
	//		unsigned int ypixel;
	//		int cirSize = (int)((float)img_size_rows * 0.55f / (float)nLines);
	//		int cirCol;
	//		maxOutLast = maxOut;
	//		minOut = 100, maxOut = -100;

	//		for (indexOfAI = 1; indexOfAI < len; indexOfAI++) {
	//			indexOfModulation = indices[indexOfAI];
	//			indexOfXYPosition = -bufferdelay + indexOfModulation;
	//			indexOfXYPosition = (indexOfXYPosition >= (int)npts) ? indexOfXYPosition - npts : indexOfXYPosition;
	//			indexOfXYPosition = (indexOfXYPosition < 0) ? indexOfXYPosition + npts : indexOfXYPosition;

	//			if (indexOfModulation & 0x0001) {
	//				out = -ai0[indexOfAI] + ai0[indexOfAI - 1];				// Average 4 off/on laser samples to get reflected laser brightness
	//			}
	//			else {
	//				out = +ai0[indexOfAI] - ai0[indexOfAI - 1];				// Average 4 on/off laser samples to get reflected laser brightness
	//			}
	//			// Check if out is too small which means saturated samples subtracted from each other or noise
	//			if (out < threshold)
	//				out = out_keep;

	//			if (out < minOut) minOut = out;
	//			if (out > maxOut) maxOut = out;
	//			if (out < 0) out = 0;
	//			out_keep = out;

	//			if (m[indexOfXYPosition] > 0) // Continue only for active line pixels
	//			{
	//				xpixel = (int)(((x[indexOfXYPosition] + 1.f) / 2.f) * ((float)img_size_cols - 1.f));
	//				ypixel = (int)(((-y[indexOfXYPosition] + 1.f) / 2.f) * ((float)img_size_rows - 1.f));
	//				rect.x = xpixel;
	//				rect.y = ypixel;
	//				rect.height = img_size_rows / nLines + 2;
	//				rect.width = img_size_cols / nPixels + 3;
	//				cirCol = std::min((255.f * out) / maxOutLast, 255.f);
	//				rectangle(image, rect, Scalar(0, cirCol, 0), CV_FILLED);
	//			}
	//		}
	//		char s[25];
	//		sprintf(s, "X = %4.3f   Y = %4.3f", minOut, maxOut);
	//		putText(image, s, Point(200, 550), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2, 8, false);
	//		GaussianBlur(image, imageout, Size(2 * blur + 3, 2 * blur - 1), 0, 0);
	//		imshow("IMAGER", imageout);
	//		pressedKey = cvWaitKey(1);

	//		if (pressedKey > 0 || _kbhit()) {
	//			pressedKey = (pressedKey > 0) ? pressedKey : _getch();
	//			menuFlag = true;
	//			switch (pressedKey)
	//			{
	//			case '+':
	//			case '=':
	//				bufferdelay += 1;
	//				if (bufferdelay >= 127) bufferdelay = 127;
	//				break;
	//			case '-':
	//			case '_':
	//				bufferdelay -= 1;
	//				if (bufferdelay <= 0) bufferdelay = 0;
	//				break;
	//			case 'O':
	//				dataScale += 0.005f;
	//				if (dataScale >= 1) dataScale = 1.f;
	//				mti->SetDeviceParam(MTIParam::DataScale, dataScale);
	//				break;
	//			case 'o':
	//				dataScale -= 0.005f;
	//				if (dataScale <= 0) dataScale = 0.f;
	//				mti->SetDeviceParam(MTIParam::DataScale, dataScale);
	//				break;
	//			case 'B':
	//				blur++;
	//				if (blur >= 25) blur = 25;
	//				break;
	//			case 'b':
	//				blur--;
	//				if (blur <= 1) blur = 1;
	//				break;
	//			case 'A':
	//			case 'a':
	//				xOffset -= 0.02f;
	//				if (xOffset < -0.9f)
	//					xOffset = -0.9f;
	//				mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
	//				break;
	//			case 'D':
	//			case 'd':
	//				xOffset += 0.02f;
	//				if (xOffset > 0.9f)
	//					xOffset = 0.9f;
	//				mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
	//				break;
	//			case 'S':
	//			case 's':
	//				yOffset -= 0.02f;
	//				if (yOffset < -0.9f)
	//					yOffset = -0.9f;
	//				mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
	//				break;
	//			case 'W':
	//			case 'w':
	//				yOffset += 0.02f;
	//				if (yOffset > 0.9f)
	//					yOffset = 0.9f;
	//				mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
	//				break;
	//			case '0': // '0' key
	//			case ')': // ')' key
	//				xOffset = 0.f;
	//				yOffset = 0.f;
	//				mti->SetDeviceParam(MTIParam::OutputOffsets, xOffset, yOffset);
	//				dataScale = 0.4f;
	//				mti->SetDeviceParam(MTIParam::DataScale, dataScale);
	//				break;
	//			default:
	//				loopgo = 0;
	//				break;
	//			}
	//		}
	//	}

	//	// Return to regular output mode
	//	mti->SetDeviceParam(MTIParam::DataMode, MTIDataMode::Sample_Output);
	//	mti->SetDeviceParam(MTIParam::LaserModulationEnable, 0);
	//	mti->SetDeviceParam(MTIParam::DataScale, 1.0);
	//	mti->ResetDevicePosition();

	//	//destroyWindow("IMAGER");
	//	SAFE_DELETE_ARRAY(x); SAFE_DELETE_ARRAY(y); SAFE_DELETE_ARRAY(m);
	//	SAFE_DELETE_ARRAY(ai0); SAFE_DELETE_ARRAY(indices);
	//	SAFE_DELETE(datagen);
	//}
	

}






