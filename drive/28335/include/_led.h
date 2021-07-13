/*===========================================================================+
|  File		:I32_led.h 														 |	
|  Function : LED                                                            |
|  Task     : LED service routine header file                                |
|----------------------------------------------------------------------------|
|  Compile  : C320 V6.60 -                                                   |
|  Link     : C320 V6.60 -                                                   |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : C.C. Chen                                                      |
|  Version  : V1.00                                                          |
|  Creation : 09/22/1999                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D___LED
#define     D___LED

#include "common.h"
#include "platform.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#ifdef  D_DSP2812
#define		LED_CPUHEART		0x4000
#define		LED_ALARM			0x8000

#else	// 28335
	#ifdef	D_HYB5_335
		#define		LED_ALARM			0x00000001L
		#define		LED_CPUHEART		0x00100000L
	#elif defined D_SMC335M1
		#define		LED_CPUHEART		0x00100000L
		#define		LED_ALARM			0x00400000L
	#elif defined D_SMC335M2
		#define		LED_CPUHEART		0x08000000L
		#define		LED_ALARM			0x00400000L
	#endif
#endif


#define		RT_CPUHEART			0
#define		RT_ALARM			1

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
void	LED_ON (DWORD wFuncID);
void	LED_OFF(DWORD wFuncID);

void	CreateTaskLed(WORD wFuncID, WORD wInterval);
void	DestoryTaskLed(WORD wFuncID);

void	RunLed();

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/

#endif
