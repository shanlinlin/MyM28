//###########################################################################
//
// FILE:	ADCcalibrationDriver.h
//
// TITLE:	ADC Calibration Driver Definitions
//
//###########################################################################
//
// Ver  | dd-mmm-yyyy |  Who  | Description of changes
// =====|=============|=======|==============================================
//  1.0 | 20 Jun 2003 | AT/SD | Original Release.
// -----|-------------|-------|----------------------------------------------
//  1.1 | 26 Nov 2004 | AT    | Fixed Bug In ADCcalibrationInitADC function.
//      |             |       | Was incorrectly disabling ADC clock.
//
//###########################################################################

#ifndef D__ADC
#define D__ADC

#include "common.h"

//--------------------------------------------------------
// User needs to select ADC sampling mode:
//
// SEQUENTIAL:   ADC channels are converted one at a time:
//               A0->A1->A2->...B0->B1->B2->....
//
// SIMULTANEOUS: ADC channels are converted in pairs:
//               A0->A1->A2->....
//               B0  B1  B2
//--------------------------------------------------------

#define MAX_ADC_28 0 //1. Anders 2013-4-9.
#define SEQUENTIAL 1
#define SIMULTANEOUS 0
#define ADC_SAMPLING_MODE SEQUENTIAL

#define ADCHL_MAX 16
#define ADCHL_USE 3

//--------------------------------------------------------
// Mapping of ADC channels to result registers:
//--------------------------------------------------------

#if ADC_SAMPLING_MODE == SEQUENTIAL
#define A0 ADCRESULT0
#define A1 ADCRESULT1
#define A2 ADCRESULT2
#define A3 ADCRESULT3
#define A4 ADCRESULT4
#define A5 ADCRESULT5
#define A6 ADCRESULT6
#define A7 ADCRESULT7
#define B0 ADCRESULT8
#define B1 ADCRESULT9
#define B2 ADCRESULT10
#define B3 ADCRESULT11
#define B4 ADCRESULT12
#define B5 ADCRESULT13
#define B6 ADCRESULT14
#define B7 ADCRESULT15
#endif

#if ADC_SAMPLING_MODE == SIMULTANEOUS
#define A0 ADCRESULT0
#define A1 ADCRESULT2
#define A2 ADCRESULT4
#define A3 ADCRESULT6
#define A4 ADCRESULT8
#define A5 ADCRESULT10
#define A6 ADCRESULT12
#define A7 ADCRESULT14
#define B0 ADCRESULT1
#define B1 ADCRESULT3
#define B2 ADCRESULT5
#define B3 ADCRESULT7
#define B4 ADCRESULT9
#define B5 ADCRESULT11
#define B6 ADCRESULT13
#define B7 ADCRESULT15
#endif

//--------------------------------------------------------
// User needs to select refernce channels and ideal values:
//
// For example:  A6 = RefHigh = 2.5V  ( 2.5*4095/3.0 = 3413 ideal count)
//               A7 = RefLow  = 1.25V (1.25*4095/3.0 = 1707 ideal count)
//--------------------------------------------------------

#define REF_HIGH_CH A6
#define REF_LOW_CH A7
#define REF_HIGH_IDEAL_COUNT 3413
#define REF_LOW_IDEAL_COUNT 1707

//--------------------------------------------------------
// Structure used for storing calibration parameters and
// results:
//--------------------------------------------------------

typedef struct tagADC_REG
{
	Uint16 * RefHighChAddr;          // Channel Address of RefHigh
	Uint16 * RefLowChAddr;           // Channel Address of RefLow
	Uint16 * ResultCh0Addr;          // Channel 0 Address
	int16    Avg_RefHighActualCount; // Ideal RefHigh Count (Q4)
	int16    Avg_RefLowActualCount;  // Ideal RefLow  Count (Q4)
	int16    RefHighIdealCount;      // Ideal RefHigh Count (Q0)
	int16    RefLowIdealCount;       // Ideal RefLow  Count (Q0)
	int16    CalGain;                // Calibration Gain   (Q12)
	int16    CalOffset;              // Calibration Offset (Q0)
	                                 // Store Calibrated ADC Data (Q0):
	                                 // Simultaneous   Sequential
	                                 // ============  ============
	/*
    int16   ch0;                 		//      A0            A0  (Q15)
    int16   ch1;                 		//      B0            A1
    int16   ch2;                 		//      A1            A2        
    int16   ch3;                 		//      B1            A3          
    int16   ch4;                 		//      A2            A4          
    int16   ch5;                 		//      B2            A5          
    int16   ch6;                 		//      A3            A6          
    int16   ch7;                 		//      B3            A7          
    int16   ch8;                 		//      A4            B0          
    int16   ch9;                 		//      B4            B1          
    int16   ch10;                		//      A5            B2          
    int16   ch11;                		//      B5            B3          
    int16   ch12;                		//      A6            B4          
    int16   ch13;                		//      B6            B5          
    int16   ch14;                		//      A7            B6          
    int16   ch15;                		//      B7            B7
	*/
	int16 ch[ADCHL_MAX];

	Uint16 StatusExtMux; // Indicates Status Of External Mux For Current Conversion

	void (*read)(void * pADC); // Read ADC conversion consequence
} ADC_REG;

typedef struct tagADC_TEMP
{
	/*
	int16	reg0;						// Q12
	int16	reg1;
	int16	reg2;
	int16	reg3;
	int16	reg4;
	int16	reg5;
	int16	reg6;
	int16	reg7;
	int16	reg8;
	int16	reg9;
	int16	reg10;
	int16	reg11;
	int16	reg12;
	int16	reg13;
	int16	reg14;
	int16	reg15;
	*/
	int16 ch[ADCHL_MAX];
} ADC_TEMP;

typedef ADC_REG * ADCREG_HANDLE;

// Default Initializer for the ADCVALS Object
#define ADCREG_DEFAULTS                    \
	{                                      \
		(Uint16 *)&AdcRegs.REF_HIGH_CH,    \
		    (Uint16 *)&AdcRegs.REF_LOW_CH, \
		    (Uint16 *)&AdcRegs.ADCRESULT0, \
		    0x0000,                        \
		    0x0000,                        \
		    REF_HIGH_IDEAL_COUNT,          \
		    REF_LOW_IDEAL_COUNT,           \
		    _IQ12(1),                      \
		    0x0000,                        \
		    { 0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0,                           \
			  0 },                         \
		    0x0000,                        \
		    (void (*)(void *))ReadADC      \
	}

void InitADC();
void ReadADC(ADCREG_HANDLE pADC);

extern ADC_TEMP g_Preadc;

//==========================================================================
// No more.
//==========================================================================

#endif
