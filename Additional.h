#pragma once
#include "Tokens.h"

namespace NTU
{
	extern volatile bool refresh_crop[totalCROPfun];
	extern volatile bool busyRectDraw[totalCROPfun];
	extern volatile bool colorRect[totalCROPfun];
	extern volatile int resizeCroppedFactor[totalCROPfun];
	extern volatile int resizeWinFULLFactor[totalCROPfun];
	extern Mat rectDrawCrop[totalCROPfun];
	extern Point P1[totalCROPfun], P2[totalCROPfun], P1_TEMP, SdREF;
	extern int BoxThickness;
	void mouseSelectionCROP(int event, int x, int y, int, void*);
	void mouseSelectionRESIZE(int event, int x, int y, int, void*);
	extern Rect ROI_BOX[totalCROPfun];
	extern volatile bool slideCroppedROI;
	extern bool L_hold[totalCROPfun], L_release[totalCROPfun];
	// CROP SETTINGS
	extern int pixelTh_1;
	extern int pixelTh_2;
	extern int minXcrop;
	extern int minYcrop;
}

