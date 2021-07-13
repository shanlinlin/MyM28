/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : THJ                                                              |
|  Version  : V1.00                                                            |
|  Creation : 2015/01/14                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#include "../include/Task_Led.h"
#include "CompLed.h"
#include "_led.h"
//##############################################################################
//      Definition
//##############################################################################
//##############################################################################
//      Enumeration
//##############################################################################
//##############################################################################
//      Type Definition
//##############################################################################
//##############################################################################
//      Function Declaration
//##############################################################################
//##############################################################################
//      Data Declaration
//##############################################################################
BOOL u_iLed_Event;
BOOL u_iLed_MasterOnline;
BOOL u_iLed_HMIOnline;
BOOL u_iLed_Error;

Ttimer    u_dwRunTime;
S_CompLed u_sLed_CpuHeart;

DWORD     g_dwServiceTime;
S_CompLed g_sLed_Alarm;
//------------------------------------------------------------------------------
signed short const LEDSHOW_ON[]           = { 32767, 0 };
signed short const LEDSHOW_OFF[]          = { 0 };
signed short const LEDSHOW_100MS[]        = { 1, -1, 0 };
signed short const LEDSHOW_1000MS[]       = { 10, -10, 0 };
signed short const LEDSHOW_SERVICE_STOP[] = { 2, -2, 2, -20, 0 };
//##############################################################################
//      Function Prototype
//##############################################################################
/**
 * [Init_Led description]
 */
void Init_Led(void)
{
	u_iLed_Event        = TRUE;
	u_iLed_MasterOnline = FALSE;
	u_iLed_HMIOnline    = FALSE;
	u_iLed_Error        = FALSE;
	ResetTimer(&u_dwRunTime, 0);

	CompLed_Init(&u_sLed_CpuHeart);
	u_sLed_CpuHeart.dwLedId = LED_ALARM;
	CompLed_SetOnOff(&u_sLed_CpuHeart, LED_ON, LED_OFF);

	CompLed_Init(&g_sLed_Alarm);
	g_sLed_Alarm.dwLedId = LED_CPUHEART;
	CompLed_SetOnOff(&g_sLed_Alarm, LED_ON, LED_OFF);
}

/**
 * [Run_Led description]
 */
void Run_Led(void)
{
	if (CheckTimerOut(&u_dwRunTime))
	{
		ResetTimer(&u_dwRunTime, 100);
		{
			if (0 != g_dwServiceTime)
			{
				g_dwServiceTime--;
				CompLed_SetShow(&u_sLed_CpuHeart, LEDSHOW_1000MS);
			}
			else
				CompLed_SetShow(&u_sLed_CpuHeart, LEDSHOW_SERVICE_STOP);
		}
		CompLed_Run(&u_sLed_CpuHeart);
		CompLed_Run(&g_sLed_Alarm);
	}
	else
	{
		if (u_iLed_Event)
		{
			u_iLed_Event = FALSE;
			if (!(u_iLed_MasterOnline && u_iLed_HMIOnline))
			{
				if (u_iLed_MasterOnline)
					CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_ON);
				else
					CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_OFF);
			}
			else if (u_iLed_Error)
				CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_100MS);
			else
			{
				CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_1000MS);
				g_sLed_Alarm._sShow._usIndex = 1 - u_sLed_CpuHeart._sShow._usIndex;
				g_sLed_Alarm._sShow._bCount  = u_sLed_CpuHeart._sShow._bCount;
			}
		}
	}
}

/**
 * [Led_MasterLink description]
 * @param iOnline [description]
 */
void Led_MasterLink(BOOL iOnline)
{
	if (u_iLed_MasterOnline != iOnline)
	{
		u_iLed_MasterOnline = iOnline;
		u_iLed_Event        = TRUE;
	}
}

/**
 * [Led_HMILink description]
 * @param iOnline [description]
 */
void Led_HMILink(BOOL iOnline)
{
	if (u_iLed_HMIOnline != iOnline)
	{
		u_iLed_HMIOnline = iOnline;
		u_iLed_Event     = TRUE;
	}
}

/**
 * [Led_Error description]
 * @param iError [description]
 */
void Led_Error(BOOL iError)
{
	if (u_iLed_Error != iError)
	{
		u_iLed_Error = iError;
		u_iLed_Event = TRUE;
	}
}
