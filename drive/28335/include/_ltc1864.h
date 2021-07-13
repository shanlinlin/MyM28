/*===========================================================================+
|  Class    : LTC1864                                                        |
|  Task     : LTC1864 Header file                                            |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1	 -                                                   |
|  Link     : CCS3.1	 -                                                   |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : zholy		                                                     |
|  Version  : V1.00                                                          |
|  Creation : 01/13/2011                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D___LTC1864
#define     D___LTC1864

#include    "common.h"

/*===========================================================================+
|           Marco Definition                                                 |
+===========================================================================*/
#define		TIMER_CONV					2.75	// us
#define     BAUDRATE_AD_7500   			3       // 0,1,2,3 = 37.5MHz/4 (ltc1864 max = 20MHz)

#define		MAX_ADC_LTC1864				4
#define		TIMES_OVERSAMPLE			1       // Anders 2015-8-27, Modify.
#define		LTC1864_OVERSAMPLE			(1<<TIMES_OVERSAMPLE)

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct tagADC_LTC1864
{
	WORD	awBuff[MAX_ADC_LTC1864][LTC1864_OVERSAMPLE];
	WORD	awADCLTC[MAX_ADC_LTC1864];
} ADC_LTC1864;

/*===========================================================================+
|           External                                                         |
+===========================================================================*/
void	InitADC_LTC1864(WORD wBaudRate);
void	DestroyADC_LTC1864();
WORD	_Read_ADC_LTC1864(WORD wChannel);
void	CONV_ADC_LTC1864();
WORD	_SelectCHL_ADC_LTC1864(WORD wChannel);

extern ADC_LTC1864	u_adcltc1864;

#endif

