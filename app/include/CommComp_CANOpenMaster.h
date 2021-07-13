/*==============================================================================+
|  Function : Communication Component CANOpen Master                            |
|  Task     : Communication Component CANOpen Master Header File                |
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
#ifndef D__COMMCOMP_CANOPENMASTER__H
#define D__COMMCOMP_CANOPENMASTER__H
//------------------------------------------------------------------------------
#include "CANOpenCommon.h"
#include "CommComp_Factory.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define CANOPENMASTER_SENDPDO_AMOUNT 4 // Send to other's PDO.
#define CANOPENMASTER_RECVPDO_AMOUNT 4 // Recv come from other's send PDO.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_CANOpenMasterCtrlCmd {
	CANOPENMASTERCTRLCMD_ENTERAPPSTATE_INIT,
	CANOPENMASTERCTRLCMD_ENTERAPPSTATE_CONFIG,
	CANOPENMASTERCTRLCMD_ENTERAPPSTATE_RUN,

	CANOPENMASTERCTRLCMD_SUM,
	CANOPENMASTERCTRLCMD_MAX = CANOPENMASTERCTRLCMD_SUM - 1
} E_CANOpenMasterCtrlCmd;

typedef enum Enum_CANCommSDOAccess {
	CANCOMMSDOACCESS_UPLOAD,
	CANCOMMSDOACCESS_DOWNLOAD
} E_CANCommSDOAccess;

typedef enum Enum_CANCommSDOState {
	CANCOMMSDOSTATE_IDLE,
	CANCOMMSDOSTATE_READ,
	CANCOMMSDOSTATE_WRITE,

	CANCOMMSDOSTATE_SUM,
	CANCOMMSDOSTATE_MAX = CANCOMMSDOSTATE_SUM - 1
} E_CANCommSDOState;

typedef enum Enum_CANOpenCompAppState {
	CANOPENCOMPAPPSTATE_INIT,
	CANOPENCOMPAPPSTATE_CONFIG,
	CANOPENCOMPAPPSTATE_RUN,

	CANOPENCOMPAPPSTATE_SUM,
	CANOPENCOMPAPPSTATE_MAX = CANOPENCOMPAPPSTATE_SUM - 1
} E_CANOpenCompAppState;

typedef enum Enum_CANOpenCHBCheck {
	CANOPENCHBCHECK_IDLE,
	CANOPENCHBCHECK_ONLINE,
	CANOPENCHBCHECK_OFFLINE,
	CANOPENCHBCHECK_CONFLICT,

	CANOPENCHBCHECK_SUM
} E_CANOpenCHBCheck;
//------------------------------------------------------------------------------
#if (DEBUG)
#define CANOpenMasterCtrlCmd_E E_CANOpenMasterCtrlCmd
#define CANCommSDOAccess_E E_CANCommSDOAccess
#define CANCommSDOState_E E_CANCommSDOState
#define CANOpenCompAppState_E E_CANOpenCompAppState

#else
#define CANOpenMasterCtrlCmd_E WORD
#define CANCommSDOAccess_E WORD
#define CANCommSDOState_E WORD
#define CANOpenCompAppState_E WORD
#endif
//##############################################################################
//
//      Type Definition
//
//##############################################################################
struct CANOpenODInfo_S;
struct CANOpenCompUnique_S;
struct CANOpenCommSDOMsg_S;
typedef struct CANOpenODInfo_S (*CANOpenDBIndexToODInfo_F)(CommComp_S* psComp, CommSubComp_S* psSubComp, DBIndex_T tDBIndex);
typedef void (*CANOpenCommRun_F)(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique);
typedef bool (*CANOpenSDOConfigFinish_F)(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
typedef void (*CANOpenEventCallBack_F)(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);
//------------------------------------------------------------------------------
typedef struct CANOpenCommSDOMsg_S {
	ODID_T tODID;
	WORD   bODSubIndex : 8; // ODSubIndex_T.
	WORD   bSDOAccess : 1;  // CANCommSDOAccess_E.
	WORD   bBytes : 3;      // 0: Does not specify the data length.
	DWORD  dwValue;
} CANOpenCommSDOMsg_S;

typedef struct CANOpenODInfo_S {
	WORD   bDataSize : 8;
	WORD   bSubIndex : 8;
	ODID_T tODID;
} CANOpenODInfo_S;

typedef struct Struct_CANOpenCHBManager // CANOpen Client Heartbeat Manager.
{
	Ttimer _sCheck_time;
	WORD   _iRecved_count : 2;   // Heartbeat State.
	WORD   _iLose_count : 6;     // Continuous lost count.
	WORD   iLose_max : 6;        // Maximum allowable continuous lost.
	WORD   _iConflict_count : 6; // Continuous conflict count.
	WORD   iConflict_max : 6;    // Maximum allowable continuous conflict.
	WORD   iTick;                // Heartbeat Massage Send (When the cycle is 0, has been in an offline state).
} S_CANOpenCHBManager;

typedef struct CANOpenSubCompUnique_S {
	CANOpenDBIndexToODInfo_F PRO_fDBIndexToODInfo;
} CANOpenSubCompUnique_S;

typedef struct CANOpenCompUnique_S {
	CANOpenCommRun_F         PRI_fCommRun;
	CANOpenSDOConfigFinish_F PRO_psConfigFinish;

	WORD                   PRO_bNodeID : 7;    // 0x00~0x7F.
	WORD                   PRO_bNodeState : 7; // E_CANOpenNodeState.
	WORD                   PRI_bSDOState : 2;  // CANCommSDOState_E.
	WORD                   PUB_blClearSize : 1;
	WORD                   PUB_blFreeSDO : 1;
	Ttimer                 PRI_lMsgSendTime;
	CANOpenEventCallBack_F PRO_fEventCallBack;
	WORD                   PRI_wCurConfigIndex;
	WORD                   PRO_eCtrlCmd; // enum CANOpenMasterCtrlCmd_E
} CANOpenCompUnique_S;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
E_CommRetCode Create_CommCANOpen(E_CommType eCommType);
void          Parse_CommCANOpen(E_CommType eCommType, CommComp_S* psCompHead);
void          Run_CommCANOpen(E_CommType eCommType, CommComp_S* psComp);
//------------------------------------------------------------------------------
E_CANOpenRetCode CANOpenMaster_SendNMTModuleCtrl(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenNMTCtrlCmd eNMTCtrlCmd);
E_CANOpenRetCode CANOpenMaster_SendSync(E_CANChannel eChannel);
E_CANOpenRetCode CANOpenMaster_SendRPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO);
E_CANOpenRetCode CANOpenMaster_SendPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO, WORD wDataLen, void const* const pvData);
E_CANOpenRetCode CANOpenMaster_SendSDO(E_CANChannel eChannel, BYTE byNodeID, CANOpenCommSDOMsg_S const* const psSDOMsg);
E_CANOpenRetCode CANOpenMaster_SendNodeGruading(E_CANChannel eChannel, BYTE byNodeID);

E_CANChannel CANOpenMaster_CommTypeToChannel(E_CommType eCommType);
void         CANOpenMaster_ResetComm(CANOpenCompUnique_S* psCompUnique);
//------------------------------------------------------------------------------
void SCANOpenSubCompUnique_Init(CANOpenSubCompUnique_S* psSubCompUnique);
void SCANOpenSubCompUnique_InitDBIndexToODInfo(CANOpenSubCompUnique_S* psSubCompUnique, CANOpenDBIndexToODInfo_F fDBIndexToODInfo);

void                      SCANOpenCompUnique_Init(CANOpenCompUnique_S* psCompUnique);
void                      SCANOpenCompUnique_InitConfigInfo(CANOpenCompUnique_S* psUnique, CANOpenSDOConfigFinish_F fConfigFinish);
void                      SCANOpenCompUnique_InitEventCallBack(CANOpenCompUnique_S* psUnique, CANOpenEventCallBack_F fEventCallBack);
void                      SCANOpenCompUnique_SetNodeID(CANOpenCompUnique_S* psCompUnique, BYTE byNodeID);
inline NodeID_T           CANOpenCompUnique_GetNodeID(CANOpenCompUnique_S* const psCompUnique);
inline E_CANOpenNodeState CANOpenCompUnique_GetNodeState(CANOpenCompUnique_S* const psCompUnique);
//------------------------------------------------------------------------------
void              CANOpenCHBManager_Init(S_CANOpenCHBManager* const psHeartbeat);
void              CANOpenCHBManager_Reset(S_CANOpenCHBManager* const psHeartbeat);
void              CANOpenCHBManager_RecvedMsg(S_CANOpenCHBManager* const psHeartbeat);
E_CANOpenCHBCheck CANOpenCHBManager_Check(S_CANOpenCHBManager* const psHeartbeat);
//------------------------------------------------------------------------------
WORD UploadCANSDO_Easy(WORD wChannal, WORD wNodeID, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize);
WORD DownloadCANSDO_Easy(WORD wChannal, WORD wNodeID, WORD wIndex, WORD wSubIndex, DWORD dwData, WORD wByteSize);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [CANOpenCompUnique_GetNodeID description]
 * @param  psCompUnique [description]
 * @return              [description]
 */
inline NodeID_T CANOpenCompUnique_GetNodeID(CANOpenCompUnique_S* const psCompUnique)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	return psCompUnique->PRO_bNodeID;
}

/**
 * [CANOpenCompUnique_GetNodeState description]
 * @param  psCompUnique [description]
 * @return              [description]
 */
inline E_CANOpenNodeState CANOpenCompUnique_GetNodeState(CANOpenCompUnique_S* const psCompUnique)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	return (E_CANOpenNodeState)psCompUnique->PRO_bNodeState;
}
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
