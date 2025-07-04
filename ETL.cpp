#pragma once
#include "NTUcore.h"

namespace NTU
{
	void NTUscope::etlControllerConfig()
	{
		int paIdxPort = 7;
		cout << "Pass!!" << endl;
		
		// Open Communication with board
		eCOMCaspErr Casp_OpenCOM();
		//const char* openCOM6_state_messenge = Casp_GetErrorMsg(openCOM6_state);
		//cout << "Open COM7: " << openCOM6_state_messenge << endl;
	}
}