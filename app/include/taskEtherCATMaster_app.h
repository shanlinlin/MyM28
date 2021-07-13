/*==============================================================================+
|  Function : System                                                            |
|  Task     : System Header File                                                |
|-------------------------------------------------------------------------------|
|  Compile  : CCS 3.3.38.2                                                      |
|  Link     : CCS 3.3.38.2                                                      |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Wain.Wei                                                          |
|  Version  : V1.00                                                             |
|  Creation : 2012/02/01                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__TASKETHERCATMASTER_APP
#define D__TASKETHERCATMASTER_APP

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/

/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/

/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/
#define     ALSTATE_PREOP   2
#define     ALSTATE_SAFOP   4
#define     ALSTATE_OP      8
/*==============================================================================+
|           Class declaration                                                   |
+==============================================================================*/
// J6-Oil
typedef union tagHTAXISCARD_OIL_CONTROLWORD {
    WORD all;
    struct
    {
        WORD b0_4 : 5;
        WORD b5Enabled : 1;
        WORD b6_7 : 2;
        WORD b8_10XPLCMD : 3;
        WORD b11_15 : 5;
    } bit;

} HTAXISCARD_OIL_CONTROLWORD;
typedef struct tagHTAXISCARD_OIL_INPUTS
{
    HTAXISCARD_OIL_CONTROLWORD ControlWord;

    WORD wSetFlow1;
    WORD wSetPress1;
    WORD wMotionCMD1;

    WORD wSetFlow2;
    WORD wSetPress2;
    WORD wMotionCMD2;

    WORD wSetFlow3;
    WORD wSetPress3;
    WORD wMotionCMD3;

    WORD wSetFlow4;
    WORD wSetPress4;
    WORD wMotionCMD4;

} HTAXISCARD_OIL_INPUTS;

typedef struct tagHTAXISCARD_OIL_OUTPUTS
{
    WORD wStatusWord;

    WORD wActSpeed1;
    WORD wActPress1;
    WORD wRsvdFst1;
    WORD wRsvdScd1;

    WORD wActSpeed2;
    WORD wActPress2;
    WORD wRsvdFst2;
    WORD wRsvdScd2;

    WORD wActSpeed3;
    WORD wActPress3;
    WORD wRsvdFst3;
    WORD wRsvdScd3;

    WORD wActSpeed4;
    WORD wActPress4;
    WORD wRsvdFst4;
    WORD wRsvdScd4;

} HTAXISCARD_OIL_OUTPUTS;
typedef struct tagHT_AXISCARD_OIL
{
    HTAXISCARD_OIL_INPUTS  inputs;
    HTAXISCARD_OIL_OUTPUTS outputs;
} HT_AXISCARD_OIL;

// J6-EleInj.
typedef struct tagHTAXISCARD_ELEINJ_INPUTS
{
    WORD wControlWord;

    WORD wMotionCMD;
    WORD wRsvd1;

    WORD wRsvd2;
    WORD wRsvd3;
    WORD wRsvd4;

    WORD wRsvd5;
    WORD wRsvd6;
    WORD wRsvd7;

    WORD wRsvd8;
    WORD wRsvd9;
    WORD wRsvd10;
    WORD wRsvd11;

} HTAXISCARD_ELEINJ_INPUTS;

typedef struct tagHTAXISCARD_ELEINJ_OUTPUTS
{
    WORD wStatusWord;

    WORD wInjActPosiL;
    WORD wInjActPosiH;
    WORD wInjActSpeedL;
    WORD wInjActSpeedH;

    WORD wInjActTorque;
    WORD wInjActPress;

    WORD wInjProfilePosiL;
    WORD wInjProfilePosiH;

    WORD wInjPosiErrL;
    WORD wInjPosiErrH;

    WORD wInjProfileVeloL;
    WORD wInjProfileVeloH;

    WORD wMotionTimeL;
    WORD wMotionTimeH;

    WORD wChgTorque;
    WORD wMotionState;
} HTAXISCARD_ELEINJ_OUTPUTS;
typedef struct tagHT_AXISCARD_ELEINJ
{
    HTAXISCARD_ELEINJ_INPUTS  inputs;
    HTAXISCARD_ELEINJ_OUTPUTS outputs;
} HT_AXISCARD_ELEINJ;

// J6-EleInj.
typedef struct tagHTAXISCARD_PLUS_INPUTS
{
    WORD wControlWord;
    WORD wControlWord_HP;

    WORD wInject_Cmd;
    WORD wInject_Rsvd0;
    WORD wInject_Rsvd1;

    WORD wDosing_Cmd;
    WORD wDosing_Rsvd0;
    WORD wDosing_Rsvd1;

    WORD wSysCtl_Flow;
    WORD wSysCtl_Press;
    WORD wSysCtl_MotionCmd;

    WORD wMold_Cmd;
    WORD wMold_Rsvd0;
    WORD wMold_Rsvd1;
} HTAXISCARD_PLUS_INPUTS;

typedef struct tagHTAXISCARD_PLUS_OUTPUTS
{
    WORD wStatusWord;

    WORD wHP_OutputSpeedL;
    WORD wHP_OutputSpeedH;
    WORD wHP_ActPress;
    WORD wHP_Rsvd0;
    WORD wHP_Rsvd1;
    WORD wHP_Rsvd2;
    WORD wHP_Rsvd3;
    WORD wHP_Rsvd4;

    WORD wInject_PosiL;
    WORD wInject_PosiH;
    WORD wInject_SpeedL;
    WORD wInject_SpeedH;
    WORD wInject_Torque;
    WORD wInject_Press;
    WORD wInject_Timer;
    WORD wInject_State;

    WORD wDosing_SpeedL;
    WORD wDosing_SpeedH;
    WORD wDosing_Torque;
    WORD wDosing_Rsvd0;
    WORD wDosing_Rsvd1;
    WORD wDosing_Rsvd2;
    WORD wDosing_Timer;
    WORD wDosing_State;

    WORD wMold_PosiL;
    WORD wMold_PosiH;
    WORD wMold_SpeedL;
    WORD wMold_SpeedH;
    WORD wMold_Torque;
    WORD wMold_Rsvd0;
    WORD wMold_Timer;
    WORD wMold_State;
} HTAXISCARD_PLUS_OUTPUTS;
typedef struct tagHT_AXISCARD_PLUS
{
    // WORD                    wECSlaveIdx;
    HTAXISCARD_PLUS_INPUTS  inputs;
    HTAXISCARD_PLUS_OUTPUTS outputs;
} HT_AXISCARD_PLUS;

typedef union tagJ6PP_CONTROLE {
    WORD all;
    struct
    {
        WORD Rsvd0            : 4;
        WORD bErr_Clear       : 1;//清除警报
        WORD bEnable          : 1;//使能
        WORD bEmergStop       : 1;//紧停，常闭
        WORD Rsvd1            : 2;
        WORD bMode_StepFollow : 1;//随动模式
        WORD Rsvd2            : 2;
        WORD Oper_Mode        : 4;//操作模式0-手动;1-调试；2-寻零；4-半自动；8全自动；9-自动请料
    } bit;
} J6PP_CONTROLE;
typedef union tagJ6PP_CONTRH {
    WORD all;
    struct
    {
        WORD Rsvd0            : 8;
        WORD bRsvd1           : 1;
        WORD bSys_Combine     : 1;//系统合并
        WORD bSys_Divide      : 1;//系统分流
        WORD Rsvd2            : 5;
    } bit;
} J6PP_CONTROLH;
typedef union tagJ6PP_INJCMD {
    WORD all;
    struct
    {
        WORD bCmd_Inj         : 1;//射出
        WORD bCmd_SuckBack    : 1;//射退
        WORD bCmd_HoldPress   : 1;//保压
        WORD Rsvd0            : 2;
        WORD bCmd_Back1_Before: 1;//储前射退（预留）
        WORD bCmd_Back2_After : 1;//储后B射退（预留）
        WORD bRsvd1           : 1;
        WORD bValve_Force     : 1;//比例阀强制
        WORD Rsvd2            : 3;
        WORD bIPO_Enable      : 1;
        WORD bIPO_Produce     : 1;
        WORD Rsvd3            : 2;
    } bit;
} J6PP_INJCMD;
typedef union tagJ6PP_MOLDCMD {
    WORD all;
    struct
    {
        WORD bCmd_OpenMold    : 1;//开模
        WORD bCmd_CloseMold   : 1;//关摸
        WORD bCmd_CoreActive  : 1;//中子激活（备用）
        WORD bStat_ClsEnd     : 1;
        WORD Rsvd0            : 4;
        WORD bValve_Force     : 1;//比例阀强制
        WORD Rsvd1            : 7;
    } bit;
} J6PP_MOLDCMD;
// J6-PP
typedef struct tagHTAXISCARD_PP_INPUTS
{
    J6PP_CONTROLE wJ6PP_Control_E;
    J6PP_CONTROLH wJ6PP_Control_H;

    J6PP_INJCMD   wJ6PP_Inject1_Cmd;
    WORD wJ6PP_Inject1_Rsvd0;
    WORD wJ6PP_Inject1_ValveVoltage;

    J6PP_MOLDCMD  wJ6PP_Mold1_Cmd;
    WORD wJ6PP_Mold1_Rsvd0;
    WORD wJ6PP_Mold1_ValveVoltage;

    WORD wJ6PP_Sys_Press1;
    WORD wJ6PP_Sys_Flow1;
    WORD wJ6PP_SysHydPress_Cmd1;

    WORD wJ6PP_Sys_Press2;
    WORD wJ6PP_Sys_Flow2;
    WORD wJ6PP_SysHydPress_Cmd2;
    
    WORD wJ6PP_Sys_Press3;
    WORD wJ6PP_Sys_Flow3;
    WORD wJ6PP_SysHydPress_Cmd3;

    WORD wJ6PP_Sys_Press4;
    WORD wJ6PP_Sys_Flow4;
    WORD wJ6PP_SysHydPress_Cmd4;

    WORD J6PP_Rvsd0;
    WORD J6PP_Rvsd1;
    WORD J6PP_Rvsd2;

    WORD J6PP_Rvsd3;
    WORD J6PP_Rvsd4;
    WORD J6PP_Rvsd5;
} HTAXISCARD_PP_INPUTS;

typedef union tagJ6PP_STATWDL {
    WORD all;
    struct
    {
        WORD bErr_Driver1  : 1;
        WORD bErr_Driver2  : 1;
        WORD bErr_Driver3  : 1;
        WORD bErr_Driver4  : 1;
        WORD bErr_Driver5  : 1;
        WORD bErr_Driver6  : 1;
        WORD bErr_Driver7  : 1;
        WORD bErr_Driver8  : 1;
        WORD bErr_J6PP     : 1;
        WORD bBusy_J6PP    : 1;
        WORD bErr_Driver9  : 1;
        WORD bErr_Driver10 : 1;
        WORD bErr_Driver11 : 1;
        WORD bErr_Driver12 : 1;
        WORD bErr_Driver13 : 1;
        WORD bErr_Driver14 : 1;
    } bit;
} J6PP_STATWDL;
typedef union tagJ6PP_STATWDH {
    WORD all;
    struct
    {
        WORD bErr_Driver15  : 1;
        WORD bErr_Driver16  : 1;
        WORD bErr_DRuler1   : 1;
        WORD bErr_DRuler2   : 1;
        WORD bErr_DRuler3   : 1;
        WORD bErr_DRuler4   : 1;
        WORD bErr_DRuler5   : 1;
        WORD bErr_DRuler6   : 1;
        WORD bErr_Driver7   : 1;
        WORD Rsvd           : 7;
    } bit;
} J6PP_STATWDH;
typedef union tagJ6PP_INTSTATUS {
    WORD all;
    struct
    {
        WORD bStat_Inj           : 1;//注射状态
        WORD bStat_SuckBack      : 1;//射退状态
        WORD bStat_Hold          : 1;//保压状态
        WORD Rsvd0               : 2;
        WORD bStat_SuckBack_B    : 1;//储料前射退（预留）
        WORD bStat_SuckBack_A    : 1;//储料后射退（预留）
        WORD Rsvd1               : 5;
        WORD bFlag_IPO_TransHold : 1;//IPO转保压标志
        WORD bFlag_IPO_HoldEnd   : 1;//IPO保压完成标志
        WORD Stat_Oper           : 2;//00-无操作；01-寻压力峰值；10-学习阶段；11-生产阶段
    } bit;
} J6PP_INTSTATUS;
typedef union tagJ6PP_MOLDSTATUS {
    WORD all;
    struct
    {
        WORD bStat_CorePause    : 1;//中子暂停（备用0
        WORD bStat_OpenMold     : 1;//开模状态
        WORD bStat_ClsMold      : 1;//合模状态
        WORD Rsvd0              : 13;
    } bit;
} J6PP_MOLDSTATUS;

typedef struct tagHTAXISCARD_PP_OUTPUTS
{
    J6PP_STATWDL wJ6PP_StatusWordL;
    J6PP_STATWDH wJ6PP_StatusWordH;

    WORD wJ6PP_Inject_PosiL;
    WORD wJ6PP_Inject_PosiH;
    WORD wJ6PP_Inject_SpeedL;
    WORD wJ6PP_Inject_SpeedH;
    WORD wJ6PP_Inject_ActPress;
    WORD wJ6PP_Inject_SetPress;
    WORD wJ6PP_Inject_SetSpeed;
    J6PP_INTSTATUS wJ6PP_Inject_State;

    WORD wJ6PP_Mold_PosiL;
    WORD wJ6PP_Mold_PosiH;
    WORD wJ6PP_Mold_SpeedL;
    WORD wJ6PP_Mold_SpeedH;
    WORD wJ6PP_Mold_Rsvd0;
    WORD wJ6PP_Mold_ValveOutput;
    WORD wJ6PP_Mold_SetFlow;
    J6PP_MOLDSTATUS wJ6PP_Mold_State;

    WORD wJ6PP_Motion1_Speed;
    WORD wJ6PP_Motion1_Press;
    WORD wJ6PP_Motion2_Speed;
    WORD wJ6PP_Motion2_Press;
    WORD wJ6PP_Motion3_Speed;
    WORD wJ6PP_Motion3_Press;
    WORD wJ6PP_Motion4_Speed;
    WORD wJ6PP_Motion4_Press;

    WORD wJ6PP_Rod1_PosiL;
    WORD wJ6PP_Rod1_PosiH;
    WORD wJ6PP_Rod2_PosiL;
    WORD wJ6PP_Rod2_PosiH;
    WORD wJ6PP_Rod3_PosiL;
    WORD wJ6PP_Rod3_PosiH;
    WORD wJ6PP_Rod4_PosiL;
    WORD wJ6PP_Rod4_PosiH;

    WORD wJ6PP_Rsvd0;
    WORD wJ6PP_Rsvd1;
    WORD wJ6PP_Rsvd2;
    WORD wJ6PP_Rsvd3;
    WORD wJ6PP_Rsvd4;
    WORD wJ6PP_Rsvd5;
    WORD wJ6PP_Rsvd6;
    WORD wJ6PP_Rsvd7;

    WORD wJ6PP_Rsvd8;
    WORD wJ6PP_Rsvd9;
    WORD wJ6PP_Rsvd10;
    WORD wJ6PP_Rsvd11;
    WORD wJ6PP_Rsvd12;
    WORD wJ6PP_Rsvd13;
    WORD wJ6PP_Rsvd14;
    WORD wJ6PP_Rsvd15;
} HTAXISCARD_PP_OUTPUTS;

typedef struct tagHT_AXISCARD_PP    //SHANLL 20191219 ADD
{
    // WORD                    wECSlaveIdx;
    HTAXISCARD_PP_INPUTS  inputs;
    HTAXISCARD_PP_OUTPUTS outputs;
} HT_AXISCARD_PP;

typedef union CardConfig {
    WORD     wAll;
    struct
    {
        WORD b0 : 1;
        WORD b1 : 1;
        WORD b2 : 1;
        WORD b3 : 1;
        WORD b4 : 1;
        WORD b5 : 1;
        WORD b6 : 1;
        WORD b7 : 1;
        WORD b8 : 1;
        WORD b9 : 1;
        WORD b10 : 1;
        WORD b11 : 1;
        WORD b12 : 1;
        WORD b13 : 1;
        WORD b14 : 1;
        WORD b15 : 1;
    } bit;
} CardConfig;
/*------------------------------------------------------------------------------+
|           Attributes                                                          |
+------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
void CreatTaskEtherCATMaster_App();

/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/
void AssignECSlaveHandle_MotionCard(WORD wDBIndex);
void RefreshMT_HUB_Error();
/*==============================================================================+
|           Externs                                                             |
+==============================================================================*/

extern CardConfig g_Card_EtherCATMaster;

extern HT_AXISCARD_OIL    g_HT_AxisCard_Oil;
extern HT_AXISCARD_ELEINJ g_HT_AxisCard_EleInj;
extern HT_AXISCARD_PLUS   g_HT_AxisCard_Plus[2];
extern HT_AXISCARD_PP     g_HT_AxisCard_PP[3];
extern INT                g_nMT_HUB_8A_SlaveIdx;
extern WORD               u_wECATSlaveOPLost;

#ifdef __cplusplus
}
#endif

#endif
