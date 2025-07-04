#pragma once
// SOME IMPORTANT TOKENS
#define MAX_INFO_DATA		100
#define MAX_CHAR_DIR		100

// MEMORY MANAGEMENT
#define FREE 0
#define FREE_RESERVE 1

// THREAD IDs
#define THREAD_SCAN			0
#define THREAD_MEMS			1
#define THREAD_CHAN_1		2
#define THREAD_STREAM_C1	3

// EXPORT FORMATS
#define EXP_BMP 0
#define EXP_TIF 1
#define EXP_PNG 2
#define EXP_JPG 3

// COLORS
#define COL_BLACK	0
#define COL_RED		1
#define COL_GREEN	2
#define COL_BLUE	3
#define COL_CYAN	4
#define COL_YELLOW	5
#define COL_MAGENTA	6

// SCANNING HOLD or FREE
#define SCAN_HOLD 0
#define SCAN_FREE 1

// For cropping of images
#define CROP_C1			0

// For channel numbers
#define CHAN_1 0

// Some notations
#define EVEN_LINE	0
#define ODD_LINE	1
#define EVEN_FRAME	0
#define ODD_FRAME	1

// GAUSSIAN KERNEL IDs | 8 BIT
#define G8U3X3		0
#define G8U5X5		1
#define G8U7X7		2
#define G8U9X9		3
#define G8U11X11	4
#define G8U13X13	5
#define G8U15X15	6
#define G8U29X29	7

// GAUSSIAN KERNEL IDs | 16 BIT
#define G16U3X3		8
#define G16U5X5		9
#define G16U7X7		10
#define G16U9X9		11
#define G16U11X11	12
#define G16U13X13	13
#define G16U15X15	14
#define G16U29X29	15

// For cropping of images
#define CROP_C1			0
#define CROP_C2			1
#define CROP_C3			2
#define CROP_C4			3
#define CROP_M			4
#define CROP_B			5
#define CROP_S			6
#define CROP_R			7
#define CROP_RHE		8
#define totalCROPfun	9

// RAW DATA PROCESSING MODES
#define NTU_PREVIEW		0
#define NTU_BINVIEW		1
#define NTU_EXPORT		2
#define NTU_EXTERNAL	3

// DYNAMIC RESOLUTION SHIFT
#define MAX_HS_LINES	1500
#define MAX_Y_LINES		1024

// For channel numbers
#define CHAN_1 0
#define CHAN_2 1
#define CHAN_3 2
#define CHAN_4 3


