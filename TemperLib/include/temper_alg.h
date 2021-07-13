/*==============================================================================+
|  Function : Temper algorithm                                                  |
|  Task     : Temper algorithm Header File                                      |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Wain.Wei                                                          |
|  Version  : V1.00                                                             |
|  Creation : 2012/08/10                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__TEMPER_ALG
#define D__TEMPER_ALG

#ifdef __cplusplus
extern "C" {
#endif

#include "temper_type.h"

/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/
//+++++
//  Temper heat
//+++++
#define TEMPER_HEAT_OFF 0
#define TEMPER_HEAT_ON 1

//+++++
//  Temper cool
//+++++
#define TEMPER_COOL_OFF 0
#define TEMPER_COOL_ON 1
//+++++
//  PID state
//+++++
#define TEMPER_PID_OFF 0
#define TEMPER_PID_ON 1

//+++++
//  Output state
//+++++
#define TEMPER_OUTPUT_HEAT 0x1
#define TEMPER_OUTPUT_COOL 0x2
//+++++
//  Heat error
//+++++
#define TEMPER_ERR_NOTINAREA 0x1UL
#define TEMPER_ERR_SENSOR_BREAK 0x2UL
#define TEMPER_ERR_SENSOR_SHORT 0x4UL
#define TEMPER_ERR_HEAT_BREAK 0x8UL
#define TEMPER_ERR_HEAT_SHORT 0x10UL
#define TEMPER_ERR_OVER_UPLIMIT 0x20UL
#define TEMPER_ERR_COLDPROTECT 0x40UL
#define TEMPER_ERR_HEATCHECK 0x80UL
//+++++
//  Heat Status
//+++++
#define TEMPER_HEAT_STATE_MASK 0x000F
#define TEMPER_HEAT_STATE_IDLE 0x0000     // Not heat
#define TEMPER_HEAT_STATE_WARM 0x0001     // Abstract warm heat
#define TEMPER_HEAT_STATE_FULL 0x0002     // Abstract full speed heat
#define TEMPER_HEAT_STATE_RELATIVE 0x0003 // Relative heat

#define TEMPER_HEAT_LIMIT_OVER 0x0004     // Temper is over uplimit
#define TMPERR_HEAT_LIMIT_OVERSTOP 0x0008 // Temper is over stop uplimit

//+++++
//  Auto adjust cmd
//+++++
#define TEMPER_ADJ_CMD_STOP 0
#define TEMPER_ADJ_CMD_START_MODE1 1
#define TEMPER_ADJ_CMD_START_MODE2 2

//+++++
//  Pid fine tune
//+++++
#define TEMPER_PDI_NOT_FINETUE 0x0
#define TEMPER_PID_FINETUNE 0x1
//+++++
//  Temper use way
//+++++
#define TEMPER_HEAT_USE_NO 0x0
#define TEMPER_HEAT_USE_ABSTRACT 0x1
#define TEMPER_HEAT_USE_RELATIVE 0x2
#define TEMPER_HEAT_USE_NOHEATING 0x3

//+++++
//  Temper heat way
//+++++
#define TEMPER_HEATWAY_SSR 0x0
#define TEMPER_HEATWAY_RELAY 0x1
//+++++
//  Temper device type
//+++++
#define TEMPER_DEV_PIPE 0x0
#define TEMPER_DEV_MOLD 0x1

//+++++
//  Status
//+++++
#define TEMPER_ADJUST_IDLE 0x0
#define TEMPER_ADJUST_RUNNING 0x2
#define TMEPER_ADJUST_END 0x4
#define TEMPER_ADJUST_MASK 0x6
#define TEMPER_CONTINUOUS_ERR 0x8
#define TEMPER_HEAT_PERCENT_MASK 0x7F0
#define TEMPER_HEAT_PERCENT_BIT 0x4
#define TEMPER_COOL_STATUS_ON 0x1000



#define TEMPER_SAMPLE_MAXCOUNT      4

/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/
typedef struct tagTEMPER_ALG_RATE     TEMPER_ALG_RATE_T;
typedef struct tagTEMPER_ALG_INTERNAL TEMPER_ALG_INTERNAL_T;
typedef struct tagTEMPER_ALG_CONF     TEMPER_ALG_CONF_T;
typedef struct tagTEMPER_ALG          TEMPER_ALG_T;

typedef void (*TEMPER_FUNC)(TEMPER_ALG_T * pTemperAlg);

struct tagTEMPER_ALG_PID
{
    WORD_T  wKp;
    WORD_T  wKi;
    WORD_T  wKd;
    WORD_T  wRamp;
    FLOAT_T fDecel; // LONG_T lStableDuty;  改用Decel
};

struct tagTEMPER_ALG_RATE
{
    DWORD_T dwSampleTick;
    LONG_T  lCurrent;
};
struct tagTEMPER_ALG_INTERNAL
{
    WORD_T              wAdjStep;          // Adjust step
    DWORD_T             AdjStartTick;      // Õû¶¨¿ªÊ¼Ê±¼ä£¨Õû¶¨ºÍ¼ÓÈÈ¶¼¿ªÊ¼Ê±¼ä£©
    WORD_T              wAdjStartTemper;   // Õû¶¨Ê±¿ªÊ¼Ê±ÎÂ¶È
    SHORT_T             sAdjLastTemper;    // Õû¶¨Ê±ÉÏ´ÎÎÂ¶ÈÖµ
    FLOAT_T             fAdjMaxRate;       // Õû¶¨Ê±×î´óËÙÂÊ
    DWORD_T             AdjMaxRateTick;    // Õû¶¨Ê±´ïµ½×î´óËÙÂÊ´¦Ê±¼ä
    WORD_T              wAdjMaxRateTemper; // Õû¶¨Ê±´ïµ½×î´óËÙÂÊ´¦ÎÂ¶È
    LONG_T              lAdjSetTemper;     // Adjust De


    DWORD_T             dwLastRateTick;
    DWORD_T             dwLastTick;
    FLOAT_T             fMaxRate;
    FLOAT_T             fRate;
    FLOAT_T             fRate1;
    BYTE_T              byRateBufIndex;
    TEMPER_ALG_RATE_T   aRateBuffer[2];
    FLOAT_T             fPIDAdjFactor;
    LONG_T              lFisrtFullStartTemper;
    FLOAT_T             fPIDHeatMaxFactor;
    LONG_T              lWholeDuty;               // whole duty
    DWORD_T             dwOverDownLimitStartTime; // Over down limit start time
    WORD_T              wColdProtectState;        // Cold protect state
    DWORD_T             dwContinuousHeatTime;     // Continuous heat time
    LONG_T              lHeatCheckStartDegree;    // Heat check start degree
    WORD_T              wWholeCalCount;           // Output-cycle count Add by xc 20140903
    WORD_T              wRealOutputPercent;       // Output percentage
    LONG_T              lObjTemp;                 // Objective Temperature
    LONG_T              lRealSetTemp;
    LONG_T              lLastCurrent;     // last temper real value
    LONG_T              lLastAbsoluteSet; // last temper set value
    LONG_T              lDe;
    LONG_T              lDde;
    LONG_T              lSum;
    LONG_T              lDuty;
    LONG_T              lOutputDuty;
    BOOL_T              bRelativeHeat; // temper is in relative heat or not // ???
    BOOL_T              bBalanceHeatOk;
    BOOL_T              bHeatCmdOnTriggle;
    //WORD_T wFirstDnCount; // Anders 2015-11-23, Add.

    FLOAT_T             fKPFactor;

    LONG_T              lAdjStableDuty;
    DWORD_T             dwLastOutputTick;

    //以下是开启深度则学习时，抑制振荡所用的变量
    LONG_T              lCycleUnitLastTemperErr;
    LONG_T              lCycleUnitLastOuputErr;
    LONG_T              lCycleUnitMaxTemper;//记录最大温度
    LONG_T              lCycleUnitMinTemper;//记录最小温度
    LONG_T              lCycleUnitMaxOutput;//记录最大百分比输出
    LONG_T              lCycleUnitMinOutput;//记录最小百分比输出

    FLOAT_T             fKpAdjFactor;
    LONG_T              lCycleCount;
    WORD_T              wCycleState;
    WORD_T              wCycleAdjCount;
    DWORD_T             dwCycleChangeTick;

    DWORD_T             dwTemperStartCalTick;
    DWORD_T             dwTemperStartCalTemper;

    WORD_T              wTemperSampleLimit;
    WORD_T              wRapidCompensationUse;
    WORD_T              wTemperMicorAdjUse;
    WORD_T              wPIDChange;
    WORD_T              wKp;
    WORD_T              wKd;

    LONG_T              lStartHeatTemper;
};

struct tagTEMPER_ALG_CONF
{
    //WORD_T wTaskCycle;
    WORD_T wCalCycle;
    WORD_T wOutputCycle;
    WORD_T wPrecision;
};

typedef struct tagTEMPER_ALG_DB
{
    WORD_T  wSensorType;          // Temper sensor type.
    WORD_T  wUse;                 // Temper use option, 0 = No use, 1 = Absolute, 2 = Relative, 3 = Use NoHeating
    WORD_T  wDeviceType;          // lDehumidifyTemperSet -> lDeviceType, Mold or Pipe.
    WORD_T  wBanlanceHeatUse;     // wDehumidifyUse -> wBanlanceHeatUse. Temper banlance heat use, 0 = No use, 1 = Use
    SHORT_T sUpLimit;             // Temper up limit
    SHORT_T sStopUpLimit;         // Temper up limit (stop heat)
    SHORT_T sDownLimit;           // Temper down limit
    WORD_T  wCoolUse;             // Temper cool use option, 0 = No use, 1 = use
    SHORT_T sCoolStartOffset;     // Temper cool start offset
    SHORT_T sCoolStopOffset;      // Temper cool start offset
    LONG_T  lAbsoluteSet;         // Temper setting //Modify byxc
    LONG_T  lReserved0;           // Temper Rsvd0
    WORD_T  wRelativeHeatPercent; // Temper relative percent heat // LONG_T -> WORD_T
    WORD_T  wHeatCMDEnable;       // wReserved1;  CX 20190505         // wDehumidifyHeatPercent -> wAD
    WORD_T  wHeatWay;             // Heat way, 0 = SSR, 1 = Relay
    WORD_T  wOutputCycle;         // Temper heat min count --> output cycle byxc 20140903
    WORD_T  wAutoPidCmd;          // Auto Pid command
    WORD_T  wEnhanceRatio;        // THJ 2017-08-29 Modify: "wClrError"->"wEnhance_ratio". // wClrError: "Temper clear error"
    WORD_T  wKp;
    WORD_T  wKi;
    WORD_T  wKd;
    WORD_T  wRamp;
    FLOAT_T fDecel;           // LONG_T  lStableDuty;
    WORD_T  wColdProtectTime; // Cold protect time, unit:minute
    WORD_T  wHeatCheckDegree; // Heat check degree, unit:Degree
    WORD_T  wStatus;
    WORD_T  wCoolOn;  // 0 = cool off, 1 = cool on
    WORD_T  wAD;      // Auto adjust state
    LONG_T  lCurrent; // current temper real value
    WORD_T  wAdjKp;
    WORD_T  wAdjKi;
    WORD_T  wAdjKd;
    WORD_T  wAdjRamp;
    FLOAT_T fAdjDecel; // LONG_T  lAdjStableDuty;
    DWORD_T dwError;
} TEMPER_ALG_DB_T;
/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/

/*==============================================================================+
|           Class declaration -  Temper algorithm                               |
+==============================================================================*/
/*------------------------------------------------------------------------------+
|           Attributes                                                          |
+------------------------------------------------------------------------------*/
struct tagTEMPER_ALG
{
    const TEMPER_ALG_CONF_T * pTemperConf;
    TEMPER_ALG_DB_T *         pTemperDb;
    TEMPER_ALG_INTERNAL_T *   pTemperInternal;
};

/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
void    TemperAlg_Create                (   TEMPER_ALG_T                    *pTemperAlg,
                                            const TEMPER_ALG_CONF_T         *pConf,
                                            TEMPER_ALG_DB_T                 *pTemperDb,
                                            TEMPER_ALG_INTERNAL_T           *pTemperInternal);
void    TemperAlg_Calculate             (   TEMPER_ALG_T                    *pTemperAlg,
                                            WORD_T                          wHeatCmd        );
WORD_T  TemperAlg_Output                (   TEMPER_ALG_T                    *pTemperAlg,
                                            WORD_T                          wHeatCmd        );
void    Temper_Alg_Set_Adj              (   TEMPER_ALG_T                    *pTemperAlg,
                                            WORD_T                          wCmd            );
void    Temper_Alg_Set_MicroAdj         (   TEMPER_ALG_T                    *pTemperAlg,
                                            WORD_T                          wUse            );
/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/

/*==============================================================================+
|           Externs                                                             |
+==============================================================================*/
extern WORD g_wTemper_alg_MinOnTime; // ms, default 600ms.
#ifdef __cplusplus
}
#endif

#endif
