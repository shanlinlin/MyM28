/*==============================================================================+
|  Function : Temper card                                                       |
|  Task     : Temper card Header File                                           |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2014/08/01                                                        |
|  Revision : 2014/12/25                                                        |
+==============================================================================*/
#ifndef D__COMMCOMP_TEMPERCARDNUM__H
#define D__COMMCOMP_TEMPERCARDNUM__H
//------------------------------------------------------------------------------
#include "CommComp_CANOpenMaster.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define TEMPERCARDNUM_DEFAULT_COMMTYPE COMMTYPE_CAN2
//------------------------------------------------------------------------------
//#define TEMPER_MAX_SEG 12 //This current max segment, and the max segment is 60.
#define TEMPER_ONECARD_DATANBR (DBINDEX_TEMPERCARD2_CONFIG_CARD_ID - DBINDEX_TEMPERCARD1_CONFIG_CARD_ID)
#define TEMPER_ONESEG_DATANBR DBGROUPINDEX_TEMPER_SEG_SUM // (DBINDEX_TEMPER0_INTERFACE_SEG2_SET_SENSOR_TYPE - DBINDEX_TEMPER0_INTERFACE_SEG1_SET_SENSOR_TYPE)

//#define TEMPER_ERR_NOTINAREA                                                        0x0001
//#define TEMPER_ERR_SENSOR_BREAK                                                     0x0002
//#define TEMPER_ERR_SENSOR_SHORT                                                     0x0004
//#define TEMPER_ERR_HEAT_BREAK                                                       0x0008
//#define TEMPER_ERR_HEAT_SHORT                                                       0x0010
//#define TEMPER_ERR_OVER_UPLIMIT                                                     0x0020
//#define TEMPER_ERR_COLDPROTECT                                                      0x0040
//#define TEMPER_ERR_HEATCHECK                                                        0x0080
//#define TEMPER_ERR_OVER_STOPLIMIT                                                   0x0100
//#define TEMPER_ERR_OILHIGH                                                          0x2000
//#define TEMPER_ERR_OILLOW                                                           0x4000
//#define TEMPER_ERR_NOTCONNECT                                                       0x8000
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_TemperHeatUse {
	TEMPERHEATUSE_NOTUSE   = 0, //TEMPER_HEAT_USE_NO
	TEMPERHEATUSE_ABSTRACT = 1, //TEMPER_HEAT_USE_ABSTRACT
	TEMPERHEATUSE_RELATIVE = 2  //TEMPER_HEAT_USE_RELATIVE
} E_TemperHeatUse;

typedef enum Enum_TemperCmd {
	TEMPERCMD_NORMAL  = 0, //TEMPER_CMD_NORMAL
	TEMPERCMD_ADJPID  = 1, //TEMPER_CMD_ADJPID
	TEMPERCMD_STOPADJ = 2  //TEMPER_CMD_STOPADJ
} E_TemperCmd;

typedef enum Enum_TemperAdjectState {
	TEMPERADJECTSTATE_IDLE = 0, //TEMPER_STATE_ADJIDLE
	TEMPERADJECTSTATE_RUN  = 1, //TEMPER_STATE_ADJRUN
	TEMPERADJECTSTATE_END  = 2  //TEMPER_STATE_ADJEND
} E_TemperAdjectState;

typedef enum Enum_AutoPIDCmd {
	AUTOPIDCMD_NOMAL   = 0,
	AUTOPIDCMD_ADJPID  = 1,
	AUTOPIDCMD_STOPADJ = 2
} E_AutoPIDCmd;

typedef enum Enum_TemperCardNum {
	TEMPERCARDNUM_NO1,
	TEMPERCARDNUM_NO2,
	TEMPERCARDNUM_NO3,
	TEMPERCARDNUM_NO4,
	TEMPERCARDNUM_NO5,
	TEMPERCARDNUM_NO6,
	TEMPERCARDNUM_NO7,
	TEMPERCARDNUM_NO8,

	TEMPERCARDNUM_SUM,
	TEMPERCARDNUM_MAX = TEMPERCARDNUM_SUM - 1
} E_TemperCardNum;

typedef enum {
	OILCOMPENSATEINDEX_ENABLE,
	OILCOMPENSATEINDEX_LOW_TEMPERATURE,     // OILCOMPENSATEINDEX_LOW_TEMPERATURE,
	OILCOMPENSATEINDEX_LOW_TEMPERATURE_KP,  // OILCOMPENSATEINDEX_LOW_TEMPERATURE_KP,
	OILCOMPENSATEINDEX_HIGH_TEMPERATURE,    // OILCOMPENSATEINDEX_HIGH_TEMPERATURE,
	OILCOMPENSATEINDEX_HIGH_TEMPERATURE_KP, // OILCOMPENSATEINDEX_HIGH_TEMPERATURE_KP,

	OILCOMPENSATEINDEX_SUM,
	OILCOMPENSATEINDEX_MAX = OILCOMPENSATEINDEX_SUM - 1
} E_OilCompensateIndex;
//------------------------------------------------------------------------------
#if (DEBUG)
#define TemperHeatUse_E E_TemperHeatUse
#define TemperCmd_E E_TemperCmd
#define TemperAdjectState_E E_TemperAdjectState
#define AutoPIDCmd_E E_AutoPIDCmd
#define TemperCardNum_E E_TemperCardNum

#else
#define TemperHeatUse_E WORD
#define TemperCmd_E WORD
#define TemperAdjectState_E WORD
#define AutoPIDCmd_E WORD
#define TemperCardNum_E WORD
#endif
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef struct
{
	CommSubComp_S          PRI_sSubComp;
	CANOpenSubCompUnique_S PRI_sCANOpenUnique;
} CommTemperCardInfo_S;

typedef struct CommTemperCard_S
{
	CommComp_S          PRI_sComp;
	CANOpenCompUnique_S PRI_sCANOpenUnique;
	// WORD                                         PUB_wPDOSendCycle;  // THJ 2015-06-23 Mask:
	// THJ 2015-05-06 Modify(2 Line):
	// WORD                                         PRI_bReqNodeGruadState      : 1;
	WORD   PRI_bRespNodeGruadCount : 2;
	WORD   PRI_bNodeGruadErrorCount : 4;
	WORD   PRI_xSegStart : 7;
	WORD   PRI_xSegNext : 7;
	Ttimer PRI_dwOfflineCtrlTime;
	Ttimer PRI_dwNodeGruadTime;
	Ttimer PRI_dwPDOSendTime;
	Ttimer PRI_dwReadColdTemperTimer;
	Ttimer _dwPreAlarmCtrlTimer;
} CommTemperCard_S;

typedef struct
{
	DBIndex_T _baseIndex;
	DBIndex_T _actKpIndex;
	DBIndex_T _oilIndex;
	WORD      _enable : 1;
	FLOAT     _k;
	FLOAT     _b;
	FLOAT     minY;
	FLOAT     maxY;
	FLOAT     backupKp;
} S_OilCompensate;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Init_TemperCard(void);

WORD GetSegNum_TemperCard(TemperCardNum_E eCardNum);
//------------------------------------------------------------------------------
void TemperCardInfo_Init(CommTemperCardInfo_S * psTemperCardInfo);
//------------------------------------------------------------------------------
void TemperCard_Init(CommTemperCard_S * psTemperCard);
//------------------------------------------------------------------------------
void OilCompensate_Init(S_OilCompensate * oilCompensate, DBIndex_T baseIndex, DBIndex_T actKpIndex);
void OilCompensate_SetOilSegNumber(S_OilCompensate * oilCompensate, WORD oilSegNumber);
void OilCompensate_CalcLinear(S_OilCompensate * oilCompensate);
void OilCompensate_Run(S_OilCompensate * oilCompensate);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern CommTemperCard_S g_asCommTemperCard[TEMPERCARDNUM_SUM];
//extern WORD             g_awTemperSegUseState[TEMPERCARDNUM_SUM];

extern S_OilCompensate g_sMoldOpenOilCompensate;
extern S_OilCompensate g_sMoldCloseOilCompensate;
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
