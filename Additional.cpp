#include "NTUcore.h"
#include "Additional.h"

namespace NTU
{
	// For mouse call back : CROPPING AN IMAGE
	void imshowCROPPED(int param)
	{
		if (param == CROP_S)			imshow("Stitched View", rectDrawCrop[param]);
		else if (param == CROP_B)		imshow("Binary Data Viewer", rectDrawCrop[param]);
		else if (param == CROP_R)		imshow("Rapid Scan Preview", rectDrawCrop[param]);
		else if (param == CROP_RHE)		imshow("Rapid Scan Preview : H&E", rectDrawCrop[param]);
		else if (param == CROP_M)		imshow("Merged Preview : powered by OpenCV", rectDrawCrop[param]);
		else if (param == CROP_C1)		imshow("Channel 1 : Preview : powered by OpenCV", rectDrawCrop[param]);
	}

	// Checks if a window is active
	bool winExists(int param)
	{
		if (param == CROP_S)
			if (cv::getWindowProperty("Stitched View", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_B)
			if (cv::getWindowProperty("Binary Data Viewer", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_R)
			if (cv::getWindowProperty("Rapid Scan Preview", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_RHE)
			if (cv::getWindowProperty("Rapid Scan Preview : H&E", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_M)
			if (cv::getWindowProperty("Merged Preview : powered by OpenCV", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_C1)
			if (cv::getWindowProperty("Channel 1 : Preview : powered by OpenCV", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_C2)
			if (cv::getWindowProperty("Channel 2 : Preview : powered by OpenCV", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_C3)
			if (cv::getWindowProperty("Channel 3 : Preview : powered by OpenCV", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
		else if (param == CROP_C4)
			if (cv::getWindowProperty("Channel 4 : Preview : powered by OpenCV", WND_PROP_AUTOSIZE) != -1)
				return true;
			else
				return false;
	}

	// Extract data from string
	double NTUscope::extract_data_from_string(char char_word[500])
	{
		int length = 0;
		int i = 0;
		int num_dots = 0;
		bool valid_num = true;
		bool negative = false;
		double int_num = 0;
		double fraction = 0;

		// Starting to check the line
		length = strlen(char_word);
		if (length == 0)	return -1;

		// Getting the decimal part
		for (i = length - 1; i >= 0; i--)
		{
			int digit = char_word[i] - '0';
			if ((digit > 9 || digit < 0) && (char_word[i] != '.'))
			{
				if (i == (length - 1))		valid_num = false;
				break;
			}
		}

		// If negative number
		if (char_word[i] == '-')	negative = true;

		// Setting the 1st position of the data
		i = i + 1;

		// Getting the decimal part
		for (; i < length; i++)
		{
			int digit = char_word[i] - '0';
			if (digit > 9 || digit < 0)
			{
				if (char_word[i] == '.')
				{
					num_dots++;
					break;
				}
				valid_num = false;
				break;
			}
			int_num = int_num * 10 + digit;
		}

		// Getting the fractional part
		if (num_dots != 0)
		{
			i++;
			for (int j = 1; i < length; i++, j++)
			{
				double digit = char_word[i] - '0';
				if (digit > 9 || digit < 0)
				{
					valid_num = false;
					break;
				}
				fraction = fraction + digit / (pow(10, j));;
			}
		}

		if (valid_num == false)		return -1;
		else
		{

			if (negative)
			{
				double result = 0 - int_num - fraction;
				return (result);
			}
			else
			{
				double result = int_num + fraction;
				return (result);
			}
		}
	}

	// For mouse call back : CROP IMAGE
	void mouseSelectionCROP(int event, int x, int y, int flags, void* param)
	{
		// Getting the caller information
		int* ID = (int*)param;

		// Mouse Left button is pressed
		if (event == EVENT_LBUTTONDOWN)
		{
			// If need to slide the ROI
			if (winExists(*ID))
			{
				// Sliding ROI
				if (slideCroppedROI == false && ROI_BOX[*ID].contains(cv::Point(x, y)) == true)
				{
					SdREF = cv::Point(x, y);
					slideCroppedROI = true;
					L_hold[*ID] = true;
				}

				// Fresh ROI selection
				if (slideCroppedROI == false)
				{
					if ((x >= pixelTh_1 && x < rectDrawCrop[*ID].cols - pixelTh_1) &&
						(y >= pixelTh_1 && y < rectDrawCrop[*ID].rows - pixelTh_1))
					{
						L_hold[*ID] = true;
						P1_TEMP.x = x;
						P1_TEMP.y = y;
					}
					else
					{
						L_hold[*ID] = false;
						L_release[*ID] = false;
						busyRectDraw[*ID] = false;
					}
				}
			}
		}

		// Mouse Left button is released
		if (event == EVENT_LBUTTONUP)
		{
			// Fresh ROI selection
			if (slideCroppedROI == false)
			{
				if (L_hold[*ID] == true && (x >= pixelTh_2 && x < rectDrawCrop[*ID].cols - pixelTh_2) && (y >= pixelTh_2 && y < rectDrawCrop[*ID].rows - pixelTh_2))
				{
					if (abs(x - P1_TEMP.x) >= minXcrop && abs(y - P1_TEMP.y) >= minYcrop)
					{
						L_release[*ID] = true;
						P1[*ID] = P1_TEMP;
						P2[*ID].x = x;
						P2[*ID].y = y;
					}
					else
					{
						L_hold[*ID] = false;
						L_release[*ID] = false;
						busyRectDraw[*ID] = false;

						// Clearing the rectangle
						imshowCROPPED(*ID);
					}
				}
				else
				{
					L_hold[*ID] = false;
					L_release[*ID] = false;
					busyRectDraw[*ID] = false;
				}
			}
			else
			{
				// New ROI location
				int DisX = x - SdREF.x;
				int DisY = y - SdREF.y;
				P1[*ID].x = P1[*ID].x + DisX;
				P1[*ID].y = P1[*ID].y + DisY;
				P2[*ID].x = P2[*ID].x + DisX;
				P2[*ID].y = P2[*ID].y + DisY;

				// Checking exceptions
				int MaxX = rectDrawCrop[*ID].cols - 1;
				int MaxY = rectDrawCrop[*ID].rows - 1;
				P1[*ID].x = P1[*ID].x < 0 ? 0 : P1[*ID].x;
				P1[*ID].y = P1[*ID].y < 0 ? 0 : P1[*ID].y;
				P2[*ID].x = P2[*ID].x < 0 ? 0 : P2[*ID].x;
				P2[*ID].y = P2[*ID].y < 0 ? 0 : P2[*ID].y;
				P1[*ID].x = P1[*ID].x > MaxX ? MaxX : P1[*ID].x;
				P1[*ID].y = P1[*ID].y > MaxY ? MaxY : P1[*ID].y;
				P2[*ID].x = P2[*ID].x > MaxX ? MaxX : P2[*ID].x;
				P2[*ID].y = P2[*ID].y > MaxY ? MaxY : P2[*ID].y;

				// Refreshing bools
				slideCroppedROI = false;
				L_release[*ID] = true;
			}
		}

		// Draw selection rectangle in real-time
		if (L_hold[*ID] == true && L_release[*ID] == false)
		{
			// Busy Drawing Rectangle
			busyRectDraw[*ID] = true;

			// Drawing starts here
			Point TEMP_POINT;
			TEMP_POINT.x = x;
			TEMP_POINT.y = y;
			Mat tempImg(rectDrawCrop[*ID].rows, rectDrawCrop[*ID].cols, CV_8UC3);

			// Converting to RGB image temporarily
			if (rectDrawCrop[*ID].channels() == 1 && colorRect[*ID] == true)
			{
				cv::cvtColor(rectDrawCrop[*ID], tempImg, cv::COLOR_GRAY2BGR);
			}
			else
			{
				tempImg = rectDrawCrop[*ID].clone();
			}

			// Fresh ROI selection
			if (slideCroppedROI == false)
			{
				// Draw the rectangle and display
				rectangle(tempImg, P1_TEMP, TEMP_POINT, Scalar(0, 0, 255), BoxThickness);
			}
			else
			{
				// New ROI location
				int DisX = x - SdREF.x;
				int DisY = y - SdREF.y;
				Point P1_LOCAL = Point(P1[*ID].x + DisX, P1[*ID].y + DisY);
				Point P2_LOCAL = Point(P2[*ID].x + DisX, P2[*ID].y + DisY);

				// Checking exceptions
				int MaxX = rectDrawCrop[*ID].cols - 1;
				int MaxY = rectDrawCrop[*ID].rows - 1;
				P1_LOCAL.x = P1_LOCAL.x < 0 ? 0 : P1_LOCAL.x;
				P1_LOCAL.y = P1_LOCAL.y < 0 ? 0 : P1_LOCAL.y;
				P2_LOCAL.x = P2_LOCAL.x < 0 ? 0 : P2_LOCAL.x;
				P2_LOCAL.y = P2_LOCAL.y < 0 ? 0 : P2_LOCAL.y;
				P1_LOCAL.x = P1_LOCAL.x > MaxX ? MaxX : P1_LOCAL.x;
				P1_LOCAL.y = P1_LOCAL.y > MaxY ? MaxY : P1_LOCAL.y;
				P2_LOCAL.x = P2_LOCAL.x > MaxX ? MaxX : P2_LOCAL.x;
				P2_LOCAL.y = P2_LOCAL.y > MaxY ? MaxY : P2_LOCAL.y;

				// Drawing ROI
				rectangle(tempImg, P1_LOCAL, P2_LOCAL, Scalar(0, 0, 255), BoxThickness);
			}

			if (*ID == CROP_S)			imshow("Stitched View", tempImg);
			else if (*ID == CROP_B)		imshow("Binary Data Viewer", tempImg);
			else if (*ID == CROP_R)		imshow("Rapid Scan Preview", tempImg);
			else if (*ID == CROP_RHE)	imshow("Rapid Scan Preview : H&E", tempImg);
			else if (*ID == CROP_M)		imshow("Merged Preview : powered by OpenCV", tempImg);
			else if (*ID == CROP_C1)	imshow("Channel 1 : Preview : powered by OpenCV", tempImg);
			else if (*ID == CROP_C2)	imshow("Channel 2 : Preview : powered by OpenCV", tempImg);
			else if (*ID == CROP_C3)	imshow("Channel 3 : Preview : powered by OpenCV", tempImg);
			else if (*ID == CROP_C4)	imshow("Channel 4 : Preview : powered by OpenCV", tempImg);
		}

		// ROI is selected successfully
		if (L_hold[*ID] == true && L_release[*ID] == true)
		{
			ROI_BOX[*ID].width = abs(P1[*ID].x - P2[*ID].x);
			ROI_BOX[*ID].height = abs(P1[*ID].y - P2[*ID].y);
			ROI_BOX[*ID].x = min(P1[*ID].x, P2[*ID].x);
			ROI_BOX[*ID].y = min(P1[*ID].y, P2[*ID].y);

			// Refresh original & crop windows
			refresh_crop[*ID] = true;

			// Resetting bools
			L_hold[*ID] = false;
			L_release[*ID] = false;
			busyRectDraw[*ID] = false;
		}

		// If mouse is scrolled (zooming/unzooming)
		if (event == EVENT_MOUSEWHEEL)
		{
			// ZOOMING
			if (getMouseWheelDelta(flags) > 0)
			{
				if (resizeWinFULLFactor[*ID] > -5)	resizeWinFULLFactor[*ID]--;
			}

			// UN-ZOOMING	
			else
			{
				if (resizeWinFULLFactor[*ID] < 5)	resizeWinFULLFactor[*ID]++;
			}
		}
	}

	// For mouse call back : RESIZING A CROPPED IMAGE
	void mouseSelectionRESIZE(int event, int x, int y, int flags, void* param)
	{
		// Getting the caller information
		int* ID = (int*)param;

		// Detect scrolling
		if (event == EVENT_MOUSEWHEEL)
		{
			// ZOOMING
			if (getMouseWheelDelta(flags) > 0)
			{
				if (resizeCroppedFactor[*ID] > -3)	resizeCroppedFactor[*ID]--;
				refresh_crop[*ID] = true;
			}

			// UN-ZOOMING	
			else
			{
				if (resizeCroppedFactor[*ID] < 5)	resizeCroppedFactor[*ID]++;
				refresh_crop[*ID] = true;
			}
		}
	}
}