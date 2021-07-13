/*==============================================================================+
|  Function : RemoteIO                                                          |
|  Task     : RemoteIO Head File                                                |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2014/12/23                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__COMMCOMP_REMOTEIO__H
#define D__COMMCOMP_REMOTEIO__H
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
#define REMOTEIO_RECVPDO_SUM 4
#define REMOTEIO_SENDPDO_SUM 4//1	shanll 20210519 modify
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_RemoteIONum {
	REMOTEIONUM_NO1,
	REMOTEIONUM_NO2,
	REMOTEIONUM_NO3,
	REMOTEIONUM_NO4,
	REMOTEIONUM_NO5,
	REMOTEIONUM_NO6,
	REMOTEIONUM_NO7,
	REMOTEIONUM_NO8,
	REMOTEIONUM_NO9,
	REMOTEIONUM_NO10,
	REMOTEIONUM_NO11,
	REMOTEIONUM_NO12,
	REMOTEIONUM_NO13,
	REMOTEIONUM_CONFIG1_SUM, //shanll add 20200608 add 2 remoteIO devices
	REMOTEIONUM_NO14 = REMOTEIONUM_CONFIG1_SUM,
	REMOTEIONUM_NO15,
	REMOTEIONUM_SUM,
	REMOTEIONUM_MAX = REMOTEIONUM_SUM - 1
} E_RemoteIONum;

typedef enum Enum_HT_J6Type {
	HT_J6TYPE_NULL,
	HT_J6TYPE_OIL_ELE,
	HT_J6TYPE_PLUS,
	HT_J6TYPE_PP
} E_HT_J6Type;

typedef enum Enum_RemoteIODeviceType {
	REMOTEIODEVICETYPE_M28                   = 0,
	REMOTEIODEVICETYPE_SMC335_RGM50          = 1,
	REMOTEIODEVICETYPE_CAN_RULER             = 2,
	REMOTEIODEVICETYPE_PICKER                = 3,
	REMOTEIODEVICETYPE_MT_HUB_8A             = 4,
	REMOTEIODEVICETYPE_HTJ6_OIL              = 5,
	REMOTEIODEVICETYPE_HTJ6_ELEINJ           = 6,
	REMOTEIODEVICETYPE_HTJ6_PLUS             = 7,
	REMOTEIODEVICETYPE_GERMANJET             = 8,
	REMOTEIODEVICETYPE_TMTURNTABLE_ELECTRIC  = 9,
	REMOTEIODEVICETYPE_TMTURNTABLE_HYDRAULIC = 10,
	REMOTEIODEVICETYPE_TMIOT101              = 11,
	REMOTEIODEVICETYPE_TMIOT102              = 12,
	REMOTEIODEVICETYPE_TMIOT105              = 15,
	REMOTEIODEVICETYPE_TMIOT201              = 21,
	REMOTEIODEVICETYPE_TMIOT202              = 22,
	// hankin 20190603 add1
	REMOTEIODEVICETYPE_HT_EH1708X            = 33,
	REMOTEIODEVICETYPE_BTL6_V11E             = 34, // balluff ruler.
	REMOTEIODEVICETYPE_PARKER_COMPAX3_FLUID  = 35,
	REMOTEIODEVICETYPE_TR_LINEAR             = 36,
	REMOTEIODEVICETYPE_HT_ROTATION           = 37,
	REMOTEIODEVICETYPE_HTJ6_PP               = 38,
	REMOTEIODEVICETYPE_BTL6_V107             = 39, // new balluff ruler.
	REMOTEIODEVICETYPE_DRIVER_INOVANCE       = 40, // Driver Inovance
	REMOTEIODEVICETYPE_REXROTH       		 = 41, // Rexroth Motion Card
	REMOTEIODEVICETYPE_DRIVER_XINGTAI        = 42, // Driver Xingtai
	REMOTEIODEVICETYPE_HTROBOT				 = 43, // HT Robot
	REMOTEIODEVICETYPE_TYPES = 20 // number of device type.
} E_RemoteIODeviceType;

typedef enum Enum_TmIoT101AD {
	TMIOT101AD_NO1,
	TMIOT101AD_NO2,
	TMIOT101AD_NO3,
	TMIOT101AD_NO4,
	TMIOT101AD_NO5,
	TMIOT101AD_NO6,

	TMIOT101AD_SUM,
	TMIOT101AD_MAX = TMIOT101AD_SUM - 1
} E_TmIoT101AD;

typedef enum Enum_TmIoT101Output {
	TMIOT101OUTPUT_NO1,

	TMIOT101OUTPUT_SUM,
	TMIOT101OUTPUT_MAX = TMIOT101OUTPUT_SUM - 1
} E_TmIoT101Output;
typedef enum Enum_TmIoT101DA {
	TMIOT101DA_NO1,
	TMIOT101DA_NO2,

	TMIOT101DA_SUM,
	TMIOT101DA_MAX = TMIOT101DA_SUM - 1
} E_TmIoT101DA;
//##############################################################################
//
//      Type Definition
//
//##############################################################################
//(THJ 2015-06-25 Add:
typedef struct Struct_CommRemoteIOCardInfo {
	CommSubComp_S          _sSubComp;
	CANOpenSubCompUnique_S _sCANOpenUnique;
} S_CommRemoteIOCardInfo;
//)
typedef struct Struct_CommRemoteIO {
	CommComp_S             _sComp;
	CANOpenCompUnique_S    _sCANOpenUnique;
	S_CANOpenCHBManager    _sCHBManager;
	S_CANOpenCHBManager    _sPDOManager;
	Ttimer                 _dwPDOSendTime;
	CANOpenDataFrameData_U _auRecvedPDO[REMOTEIO_RECVPDO_SUM];
	CANOpenDataFrameData_U _auSendPDO[REMOTEIO_SENDPDO_SUM];
	E_RemoteIODeviceType   _eDeviceType;

	union {
		struct {
			BYTE   iError;
			Ttimer sCtrlTimerOfReadError;
		} _sMT_HUB_8A;
	} uExtendData;

	WORD   wSendCycle;
	WORD   wOfflineOutput;
	Ttimer _dwPreAlarmCtrlTimer;
} S_CommRemoteIO;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Init_RemoteIO(void);
void InitMap_RemoteIO(void);
void Test_RemoteIO(void);
void Map_RemoteIO(E_RemoteIODeviceType eDeviceType, E_CommType eCommType, NodeID_T tNodeID, WORD wCommCycle);

void SetOfflineOutput_RemoteIO(WORD wIndex, WORD wOutput);

void UpdateLinkState_RemoteIO(void);
void SetOutput_RemoteIO(WORD wIndex, WORD wOutput);
void SetDA_RemoteIO(WORD wIndex, WORD wDA);
WORD GetInput_RemoteIO(WORD wIndex);
WORD GetAD_RemoteIO(WORD wIndex);
LONG GetPositionCount_RemoteIO(WORD wIndex);

S_CommRemoteIO*  GetPositionModule_RemoteIO(WORD wIndex);
//------------------------------------------------------------------------------
void CommRemoteIOCardInfo_Init(S_CommRemoteIOCardInfo* const psRemoteIO);

void CommRemoteIO_Init(S_CommRemoteIO* const psRemoteIO);
//void CommRemoteIO_SetCommType(S_CommRemoteIO* const psRemoteIO, E_CommType eCommType);
//void CommRemoteIO_SetDeviceType(S_CommRemoteIO* const psRemoteIO, E_RemoteIODeviceType eDeviceType);
//void CommRemoteIO_SetNodeID(S_CommRemoteIO* const psRemoteIO, NodeID_T tNodeID);
// BOOL  CommRemoteIO_GetLinkState (S_CommRemoteIO *const psRemoteIO);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern CANOpenDataFrameData_U g_auCommRemoteIO_RecvedData[REMOTEIONUM_SUM][REMOTEIO_RECVPDO_SUM];
extern CANOpenDataFrameData_U g_auCommRemoteIO_SendData[REMOTEIONUM_SUM][REMOTEIO_SENDPDO_SUM];

extern S_CommRemoteIO g_asCommRemoteIO[REMOTEIONUM_SUM];
extern WORD           g_wPackerCount;
extern WORD           g_wHTJ6CardType;
extern WORD			  u_wECATLinkMask;
extern WORD			  u_wDriverInovanceCount;
extern WORD			  u_wDriverXingtaiCount;
extern WORD			  u_wRexrothCount;
extern WORD			  u_wHTRobotCount;
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
