/*==============================================================================+
|  Function : Temper common                                                     |
|  Task     : Temper common Header File                                   	    |
|-------------------------------------------------------------------------------|
|  Compile  :                                                   |
|  Link     :                                               	|
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Wain.Wei                                                          |
|  Version  : V1.00                                                             |
|  Creation : 2013/03/05                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__TEMPER_COMMON
#define D__TEMPER_COMMON

#ifdef __cplusplus
extern "C" {
#endif

#include "temper_alg.h"
/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/
#define TEMPER_MAX_NBR 96
#define TEMPER_1S_NBR 100

#define TEMPER_SENSOR_ETYPE 0 // Temper sensor type E
#define TEMPER_SENSOR_JTYPE 1 // Temper sensor type J
#define TEMPER_SENSOR_KTYPE 2 // Temper sensor type K

// VERSION_TEMPERLIB 0x20200609
// {
// 1.Add RapidCompensationUse.
// }

// VERSION_TEMPERLIB 0x20200630
// {
// 1. 去除没用的KPfactor（原来用于第一段震荡的，但后续用了新的方式，这儿没删除，现在删除）。
// 2. 加温太慢的调整继续使用。
// 3. 增加中途加温时候，加温温度超过一定范围时候，原有的sum重新累积。
// 4. 抑制累加的参数修改。
// 5. 超过设定温度2度时候，温度不输出，防止特殊情况导致的升温太多。
// }

// VERSION_TEMPERLIB 0x20200821
// {
//  1.增加自学习过程微调PID。
// }

// VERSION_TEMPERLIB 0x20200903
// {
// 1. Set Ramp作为补温比率。比如100，就是默认的100%补温；1就是1%补温。
// }
#define VERSION_TEMPERLIB 0x20200922

typedef struct tagTimer
{
	DWORD dwTTimer_Tick;
} Timer_T;

#define TMP_SYSTEMTIME_UPMS g_dwSystemUpTick
#define Tmp_ResetTimer(p_sTimer, dwOutTime) ((p_sTimer)->dwTTimer_Tick = TMP_SYSTEMTIME_UPMS + ((dwOutTime) & ((DWORD) -1 / 2))) // Anders 2015-9-24 Modify.
// #define CheckTimerOut(p_sTimer) (SYSTEMTIME_UPMS - (p_sTimer)->dwTTimer_Tick < ((DWORD)-1 / 2))                         // true: Over; false: waiting.

extern volatile DWORD g_dwSystemUpTick;

inline BOOL Tmp_CheckTimerOut(Timer_T * pTimer)
{
	if (TMP_SYSTEMTIME_UPMS - pTimer->dwTTimer_Tick < ((DWORD) -1 / 2))
	{
		pTimer->dwTTimer_Tick = TMP_SYSTEMTIME_UPMS; // keep time out.
		return TRUE;
	}
	return FALSE;
}

/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/

typedef union tagHeatCmd {
	WORD_T dwAll;
	struct
	{
		WORD_T bPipeEnable : 1;
		WORD_T bMoldEnable : 1;
		WORD_T Reserve : 14;
	} sBit;
} HeatCmd_T;
typedef struct tagTEMPER_COMMON_DB
{
	WORD_T    wPipeSensorType;
	WORD_T    wHeatType;
	WORD_T    wMinimumHeatTime;
	HeatCmd_T uHeatCmd;
	WORD_T    wPipeAutoPidCmd;
	WORD_T    wPipePidState;
	WORD_T    wPipePreserveUse;
	WORD_T    wPipePreserveTemper;
	WORD_T    wColdProtectTime;
	WORD_T    wPipeHeatCheckDegree; // Anders 2015-4-1, Modify wHeatCheckDegree -> wPipeHeatCheckDegree.
	WORD_T    wAutoPreserveTime;
	WORD_T    wAutoPreserveState;
	WORD_T    wError;
	WORD_T    wOilSetNbr;
	DWORD_T   wStopLimitSet;
	WORD_T    wSlowestSegNum; // wColdProtectCounter; // Anders 2018-3-11, modify.

	WORD_T wPipeBalanceHeatingCmd; // Pipe
	WORD_T wPipeBalanceHeatingTemper;
	WORD_T wPipeSoftHeatTemper;
	WORD_T wPipeSoftHeatTime;
	WORD_T wMoldBalanceHeatingCmd; // Mold
	WORD_T wMoldBalanceHeatingTemper;
	WORD_T wMoldSoftHeatTemper;
	WORD_T wMoldSoftHeatTime;

	WORD_T  wMoldAutoPidCmd;
	WORD_T  wMoldPidState;
	DWORD_T adwHeatOutput[4];
	DWORD_T adwCoolOutput[4];
	WORD_T  wMoldHeatCheckDegree; // Anders 2015-4-1, add.
	DWORD_T aHeatTypeConfig[4];
	WORD_T  wMoldSensorType;
	WORD_T  wMoldPreserveUse;
	WORD_T  wMoldPreserveTemper;
	WORD_T  wMoldFastOpt; // Anders 2017-4-27, add.
	WORD_T  wHeatCMDInterval;
	WORD_T  wMicroAdjUse;          // bit mode for group. Anders 2002-6-20, add.
	WORD_T  wRapidCompensationUse; // bit mode for group.
	DWORD_T	adwPIDChangeBit[3];

	DWORD_T dwPipeMicroAdjUse;
	DWORD_T dwMoldMicroAdjUse;
	DWORD_T dwRsvd3;

} TEMPER_COMMON_DB_T;

typedef struct tagTEMPER_COMMON_INTERNAL
{
	WORD_T  wNumber;
	DWORD_T dwPipeSoftStartTime;
	DWORD_T adwPipeSoftHeatBit[4];
	BOOL_T  bPipeInSoftHeat;
	BOOL_T  bChangeHeatCMD;
	WORD_T  wHeatCMDIndex;

	DWORD_T dwMoldSoftStartTime;
	DWORD_T adwMoldSoftHeatBit[4];
	BOOL_T  bMoldInSoftHeat;

	LONG_T lPipeBanlanceAverTemper;
	LONG_T lMoldBanlanceAverTemper;
	LONG_T lPipeLastBanlanceAverTemper;
	LONG_T lMoldLastBanlanceAverTemper;
	WORD_T wPipeBanlanceHeatState;
	WORD_T wMoldBanlanceHeatState;

} TEMPER_COMMON_INTERNAL_T;

typedef struct tagTEMPER_COMMON
{
	TEMPER_COMMON_DB_T *       pDb;
	TEMPER_COMMON_INTERNAL_T * pInternal;

	const TEMPER_ALG_CONF_T * pNormalAlgConf;
	const TEMPER_ALG_CONF_T * pFastAlgConf;
} TEMPER_COMMON_T;

/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/

/*==============================================================================+
|           Class declaration - Temperapp                             		    |
+==============================================================================*/
/*------------------------------------------------------------------------------+
|           Attributes                                                          |
+------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
void TemperCommon_Init(WORD_T wTemperNum,
    const TEMPER_ALG_CONF_T * pNormalAlgConf,
    const TEMPER_ALG_CONF_T * pFastAlgConf,
    TEMPER_COMMON_DB_T *      pTemperCommonDb,
    TEMPER_ALG_DB_T *         pTemperDb);
/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/
void   TemperCommon_RunCal(WORD_T wDeviceType, WORD_T wTaskCycle);
void   TemperCommon_RunOutput(void);
void   TemperCommon_ChangePipeAdjCmd(void);
void   TemperCommon_ChangeMoldAdjCmd(void);
void   TemperCommon_ChangeHeatCmd(void);
void   TemperCommon_ChangeSoftHeat(void);
void   TemperCommon_ChangeAbsolute_set(WORD_T wIndex);
void   TemperCommon_ChangeUse(WORD_T wIndex);
LONG_T TemperCommon_CalCurrent(WORD_T wIndex, WORD_T wCoolJuncAd);
void   TemperCommon_ChangePreserve(void);

void TemperCommon_ChangeRapidCompensationUse(WORD_T wIndex, WORD_T wUse);
void TemperCommon_SetPIDChange(WORD_T wIndex, WORD_T wChange);
void TemperCommon_Set_MicroAdj(WORD_T wIndex, WORD_T wUse);

void TemperCommon_ReConfigAllCalCycle();
void TemperCommon_ReConfigCalCycle(WORD_T wIndex);
void TempCommon_DistributeHeatCMD();

/*==============================================================================+
|           Externs                                                             |
+==============================================================================*/
extern DWORD g_version_temperlib;

#ifdef __cplusplus
}
#endif

#endif
