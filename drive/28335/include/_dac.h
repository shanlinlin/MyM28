//###########################################################################
//
// FILE:SPIDA_KEB.h
//
// TITLE:	DSP281x CPU D/A converter .use spi controller 
//
// NOTES:   KEB interface card D/A have 4 channel two tlc5618a 
//         
//	        tlc5618A is 12 bit resolution 
//           
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  1.00| 15 AUG 2007 | c.W. | Changes from previous version ()
//      |             |      | 
//      |             |      | 
//###########################################################################

#ifndef     D__DAC
#define     D__DAC

#include    "common.h"

#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define     SDA_CHA_CODE         12		// bit15 and bit14
#define     SDA_CHB_CODE         4		// bit14
#define     SDA_DOU_CODE         5		// bit14 and bit12

#define		SDA_SLICE		  	 2		/* define sdatest routine execute period 2ms */

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
struct DA5618A_BITS {
	Uint16 		VALUE  :12;	// 12bits D/A value
	Uint16		CTLCODE: 4;	// da control code
};

union  DA5618ACTRL_REG {
	Uint16					all;
	struct DA5618A_BITS		bit;
};

struct DAC_REGS {
	union DA5618ACTRL_REG	DA5618ATRL;
};

typedef struct tagDAC_CONFIG
{
	Uint16		ChSel;
	Uint16		Value;
	void (*daout)();
} DAC_CONFIG;

typedef DAC_CONFIG *DACCONFIG_HANDLE;

#define	DAC_DEFAULTS {	0,	\
						0,	\
						(void (*)(Uint32))SpiDaOut	\
					 }

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
void InitSpiDA();
void SpiDaOut(DACCONFIG_HANDLE pDAC);

/*===========================================================================+
|           External                                                         |
+===========================================================================*/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
