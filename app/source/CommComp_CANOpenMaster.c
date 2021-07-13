/*==============================================================================+
|  Function : Communication Component CANOpen Master                            |
|  Task     : Communication Component CANOpen Master Source File                |
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
#include "CommComp_CANOpenMaster.h"
#include "Error_App.h"
#include "Task_Watch.h"
#include "ethercatmaster.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define CANOPENCOMM_SDO_RESENDTIME 10
//#define CANOPENCOMM_NODEGUARDING_RESENDTIME 100
#define CAN_SDOMUX_TIMEROUT CANOPENCOMM_SDO_RESENDTIME
//------------------------------------------------------------------------------
#define CANOPENWATCH_BATCH_ENUM(D_Name, D_CAN0Index, D_CAN1Index) CANOPENWATCH_##D_Name,
#define CANOPENWATCH_BATCH_TABINIT(D_Name, D_CAN0Index, D_CAN1Index) { DBINDEX_##D_CAN0Index, DBINDEX_##D_CAN1Index },
// clang-format off
#define CANOPENWATCH_BATCH(D_Macro) \
CANOPENWATCH_BATCH_##D_Macro( RX_DATA_COUNT      , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWISRRXDATACOUNT   , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWISRRXDATACOUNT   )\
CANOPENWATCH_BATCH_##D_Macro( TX_NODEGUARD_COUNT , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXNODEGUARDCOUNT , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWRXNODEGUARDCOUNT )\
CANOPENWATCH_BATCH_##D_Macro( RX_HEARTBEAT_COUNT , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWRXHEARTBEATCOUNT , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWTXHEARTBEATCOUNT )\
CANOPENWATCH_BATCH_##D_Macro( TX_PDO_COUNT       , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXPDOCOUNT       , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWTXPDOCOUNT       )\
CANOPENWATCH_BATCH_##D_Macro( RX_PDO_COUNT       , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWRXPDOCOUNT       , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWRXPDOCOUNT       )\
CANOPENWATCH_BATCH_##D_Macro( TX_SDO_COUNT       , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXSDOCOUNT       , SUM                                                  )\
CANOPENWATCH_BATCH_##D_Macro( RX_SDO_COUNT       , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWRXSDOCOUNT       , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWRXSDOCOUNT       )\
CANOPENWATCH_BATCH_##D_Macro( TX_NMT_COUNT       , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXNMTCOUNT       , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWRXNMTCOUNT       )\
CANOPENWATCH_BATCH_##D_Macro( TX_PDO_ERROR_COUNT , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXPDOERROR       , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWTXPDOERROR       )\
CANOPENWATCH_BATCH_##D_Macro( CCE_ERROR_COUNT    , LOCAL_AXISCARD_INTERFACE_CANMASTER_DWCCEERROR         , LOCAL_AXISCARD_INTERFACE_CANSLAVE_DWCCEERROR         )\
CANOPENWATCH_BATCH_##D_Macro( NODEID_FILTER      , LOCAL_AXISCARD_INTERFACE_CANMASTER_WFILTERNODEID      , LOCAL_AXISCARD_INTERFACE_CANMASTER_WFILTERNODEID     )\
// clang-format on
//##############################################################################
//
//      Enumeration
//
//##############################################################################
enum {
	CANOPENWATCH_BATCH(ENUM) CANOPENWATCH_SUM
};
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef void (*CtrlCmdFunc_F)(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);

typedef enum {
    MC_SDOTYPE_UPLOAD,
    MC_SDOTYPE_DOWNLOAD,

    MC_SDOTYPE_SUM
} MC_e_SDOType;

typedef struct tagTCANSDO_EasyController
{
    WORD         m_bSdoMux;
    MC_e_SDOType m_bSdoType;
    Ttimer       m_dwSdoMuxTimer;
    WORD         m_wUploadNodeID;
    WORD         m_wUploadSdoIndex;
    WORD         m_wUploadSdoSubIndex;
    WORD         m_wUploadSdoByteSize;
    WORD         m_wUploadSdoResult;
    WORD*        m_pUploadSdoBuffer;

    WORD  m_wDownloadNodeID;
    WORD  m_wDownloadSdoIndex;
    WORD  m_wDownloadSdoSubIndex;
    WORD  m_wDownloadSdoByteSize;
    WORD  m_wDownloadSdoResult;
    DWORD m_dwDownloadSdoBuffer;
} TCANSDO_EasyController;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void CANOpenMComm_Parse(E_CANChannel eChannel, CommComp_S* psCompHead, CANOpenDataFrame_S* psDataFrame);
inline bool CANOpenMComm_ParseCANRecvedSDO(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame);
inline void CANOpenMComm_ParseCANRecvedNMTErrCtrl(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame);

static void CANOpenMComm_Init(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);
//static void             CANOpenMComm_WaitSlaveReady(CommComp_S *psComp, CANOpenCompUnique_S *psCompUnique);
static void CANOpenMComm_ConfigSlave(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);
static void CANOpenMComm_Run(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);

// static void             CANOpenMComp_SetAllDBFlag(CommComp_S *psComp);

static void            CtrlCmdFunc_ENTERAPPSTATE_INIT(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);
static void            CtrlCmdFunc_ENTERAPPSTATE_CONFIG(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);
static void            CtrlCmdFunc_ENTERAPPSTATE_RUN(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);
static void            CtrlCmdFunc_ENTERAPPSTATE_WAIT(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique);
static CANOpenODInfo_S Default_DBIndexToODInfo(CommComp_S* psComp, CommSubComp_S* psSubComp, DBIndex_T tDBIndex);
static void            Default_EventCallBack(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);
static bool            Default_ConfigFinish(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);

static void ParseCANSDO_Easy(WORD wChannal, CANOpenDataFrame_S const* pDataFrame);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//WORD  g_wTestCAN1 = 0;
WORD g_wTestCANCOBID_0 = 0;

TCANSDO_EasyController g_aCANSDO_EasyController[CANCHANNAL_SUM];
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static CtrlCmdFunc_F const uc_afCtrlCmd[] = {
	CtrlCmdFunc_ENTERAPPSTATE_INIT,
	CtrlCmdFunc_ENTERAPPSTATE_CONFIG,
	CtrlCmdFunc_ENTERAPPSTATE_RUN,
};
static DBIndex_T const uc_aeMCDB_WatchIndex[][CANCHANNAL_SUM] = { CANOPENWATCH_BATCH(TABINIT) };
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Create_CommCANOpen description]
 * @param  eCommType [description]
 * @return           [description]
 */
E_CommRetCode Create_CommCANOpen(E_CommType eCommType)
{
	E_CANChannel eChannel;

	DEBUG_ASSERT(eCommType < COMMTYPE_SUM);
	DEBUG_ASSERT((sizeof(uc_afCtrlCmd) / sizeof(*uc_afCtrlCmd)) == CANOPENMASTERCTRLCMD_SUM);
	eChannel = CANOpenMaster_CommTypeToChannel(eCommType);
	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	memset(g_aCANSDO_EasyController + eChannel, 0, sizeof(*g_aCANSDO_EasyController));
	return ((CANOPENRETCODE_SUCCESS != CANOpen_Open(eChannel)) ? COMMRETCODE_OTHER : COMMRETCODE_SUCCESS);
}
/**
 * [Parse_CommCANOpen description]
 * @param eCommType   [description]
 * @param eCANChannel [description]
 */
void Parse_CommCANOpen(E_CommType eCommType, CommComp_S* psCompHead)
{
	E_CANChannel eChannel;
	CANOpenDataFrame_S sCANOpenDataFrame;

	eChannel = CANOpenMaster_CommTypeToChannel(eCommType);
	CANOpen_TX(eChannel);
	CANOpen_RX(eChannel);
	while (CANOPENRETCODE_SUCCESS == CANOpen_Read(eChannel, &sCANOpenDataFrame))
		CANOpenMComm_Parse(eChannel, psCompHead, &sCANOpenDataFrame);
}

/**
 * [Run_CommCANOpen description]
 * @param psComp [description]
 */
void Run_CommCANOpen(E_CommType eCommType, CommComp_S* psComp)
{
	CANOpenCompUnique_S* psCompUnique;

	DEBUG_ASSERT(NULL != psComp);
	psCompUnique = psComp->PRO_pvUnique;
	if (NULL == psCompUnique)
		return;
	if (psCompUnique->PRO_eCtrlCmd <= CANOPENMASTERCTRLCMD_MAX) {
		CANOpenMasterCtrlCmd_E eCtrlCmd;

		eCtrlCmd                   = (CANOpenMasterCtrlCmd_E)psCompUnique->PRO_eCtrlCmd;
		psCompUnique->PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_SUM;
		DEBUG_ASSERT(eCtrlCmd < (sizeof(uc_afCtrlCmd) / sizeof(*uc_afCtrlCmd)));
		DEBUG_ASSERT(NULL != uc_afCtrlCmd[eCtrlCmd]);
		uc_afCtrlCmd[eCtrlCmd](psComp, psCompUnique);
	}
	DEBUG_ASSERT(NULL != psCompUnique->PRI_fCommRun);
	psCompUnique->PRI_fCommRun(psComp, psCompUnique);
}
/**
 * [CANOpenMaster_SendNMTModuleCtrl description]
 * @method CANOpenMaster_SendNMTModuleCtrl
 * @param  eChannel                        [description]
 * @param  byNodeID                        [description]
 * @param  eNMTCtrlCmd                     [description]
 * @return                                 [description]
 */
E_CANOpenRetCode CANOpenMaster_SendNMTModuleCtrl(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenNMTCtrlCmd eNMTCtrlCmd)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT((CANOPENNMTCTRLCMD_START_REMOTE_NODE == eNMTCtrlCmd)
	    || (CANOPENNMTCTRLCMD_STOP_REMOTE_NODE == eNMTCtrlCmd)
	    || (CANOPENNMTCTRLCMD_ENTER_PREOPERATIONAL_STATE == eNMTCtrlCmd)
	    || (CANOPENNMTCTRLCMD_RESET_NODE == eNMTCtrlCmd)
	    || (CANOPENNMTCTRLCMD_RESET_COMMUNICATION == eNMTCtrlCmd));
	//(THJ 2016-01-13 Modify: for "Multi thread"
	//psDataFrame = &u_asCANOpenDataFrame[eChannel];
	psDataFrame = &sDataFrame;
	//)
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen         = 2;
	psDataFrame->uData.sU8.bData0 = eNMTCtrlCmd;
	psDataFrame->uData.sU8.bData1 = byNodeID;

	{
		NodeID_T  tFilterNodeID;
		DBIndex_T eWatchIndex;

		tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
		if ((0 == tFilterNodeID) || (byNodeID == tFilterNodeID)) {
			eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_TX_NMT_COUNT][eChannel];
			if (eWatchIndex < DBINDEX_SUM)
				DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
		}
	}
	return CANOpen_Write(eChannel, psDataFrame);
}

E_CANOpenRetCode CANOpenMaster_SendSync(E_CANChannel eChannel)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(CANOPENFUNCID_RECV_PDO1);
	psDataFrame = &sDataFrame;
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen            = 0;
	psDataFrame->uHead.sStand.bCOBID = CANOpen_GetCOBID(CANOPENFUNCID_SYNC, 0);
	return CANOpen_Write(eChannel, psDataFrame);
}

/**
 * [CANOpenMaster_SendRPDO description]
 * @author hankin (2019/10/28)
 * @param  eChannel     [description]
 * @param  byNodeID     [description]
 * @param  eFuncIDOfPDO [description]
 * @return              [description]
 */
E_CANOpenRetCode CANOpenMaster_SendRPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT((CANOPENFUNCID_RECV_PDO1 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_RECV_PDO2 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_RECV_PDO3 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_RECV_PDO4 == eFuncIDOfPDO));
	//(THJ 2016-01-13 Modify: for "Multi thread"
	//psDataFrame = &u_asCANOpenDataFrame[eChannel];
	psDataFrame = &sDataFrame;
	//)
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen            = 0;
	psDataFrame->uHead.sStand.bCOBID = CANOpen_GetCOBID(eFuncIDOfPDO, byNodeID);
	psDataFrame->uHead.sStand.bRTR   = true;
	return CANOpen_Write(eChannel, psDataFrame);
}

/**
 * [CANOpenMaster_SendPDO description]
 * @method CANOpenMaster_SendPDO
 * @param  eChannel              [description]
 * @param  byNodeID              [description]
 * @param  eFuncIDOfPDO          [description]
 * @param  wDataLen              [description]
 * @param  pvData                [description]
 * @return                       [description]
 */
E_CANOpenRetCode CANOpenMaster_SendPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO, WORD wDataLen, const void* pvData)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT((CANOPENFUNCID_RECV_PDO1 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_RECV_PDO2 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_RECV_PDO3 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_RECV_PDO4 == eFuncIDOfPDO));
	DEBUG_ASSERT(wDataLen <= 8);
	DEBUG_ASSERT(NULL != pvData);
	//(THJ 2016-01-13 Modify: for "Multi thread"
	//psDataFrame = &u_asCANOpenDataFrame[eChannel];
	psDataFrame = &sDataFrame;
	//)
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen            = wDataLen;
	psDataFrame->uHead.sStand.bCOBID = CANOpen_GetCOBID(eFuncIDOfPDO, byNodeID);
	psDataFrame->uData               = *(CANOpenDataFrameData_U*)pvData;

	{
		NodeID_T  tFilterNodeID;
		DBIndex_T eWatchIndex;

		tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
		if ((0 == tFilterNodeID) || (byNodeID == tFilterNodeID)) {
			eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_TX_PDO_COUNT][eChannel];
			if (eWatchIndex < DBINDEX_SUM)
				DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
		}
	}
	return CANOpen_Write(eChannel, psDataFrame);
}
/**
 * [CANOpenMaster_SendSDO description]
 * @method CANOpenMaster_SendSDO
 * @param  eChannel              [description]
 * @param  byNodeID              [description]
 * @param  psSDOMsg              [description]
 * @return                       [description]
 */
E_CANOpenRetCode CANOpenMaster_SendSDO(E_CANChannel eChannel, BYTE byNodeID, const CANOpenCommSDOMsg_S* psSDOMsg)
{
	union {
		BYTE tAll;
		struct
		{
			BYTE bS : 1;
			BYTE bE : 1;
			BYTE bN : 2;
			BYTE bReserve0 : 1;
			BYTE bCmd : 3;
		} sBitField;
	} uSDOCmd;
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT(NULL != psSDOMsg);
	//(THJ 2016-01-13 Modify: for "Multi thread"
	//psDataFrame = &u_asCANOpenDataFrame[eChannel];
	psDataFrame = &sDataFrame;
	//)
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	uSDOCmd.tAll = 0;
	DEBUG_ASSERT((CANCOMMSDOACCESS_UPLOAD == psSDOMsg->bSDOAccess)
	    || (CANCOMMSDOACCESS_DOWNLOAD == psSDOMsg->bSDOAccess));
	switch (psSDOMsg->bSDOAccess) {
	case CANCOMMSDOACCESS_UPLOAD: {
		uSDOCmd.sBitField.bCmd = CANOPENSDOCCS_IDU;
		break;
	}
	case CANCOMMSDOACCESS_DOWNLOAD: {
		uSDOCmd.sBitField.bE   = 1;
		uSDOCmd.sBitField.bCmd = CANOPENSDOCCS_IDD;
		if (psSDOMsg->bBytes > 0) {
			DEBUG_ASSERT(psSDOMsg->bBytes <= 4);
			uSDOCmd.sBitField.bS = 1;
			uSDOCmd.sBitField.bN = 4 - psSDOMsg->bBytes;
		}
		psDataFrame->uData.sSDO.bData = psSDOMsg->dwValue;
		break;
	}
	default: {
		DEBUG_ASSERT(false);
		return CANOPENRETCODE_SUCCESS;
	}
	}
	psDataFrame->tDataLen                 = 8;
	psDataFrame->uHead.sStand.bCOBID      = CANOpen_GetCOBID(CANOPENFUNCID_RECV_SDO, byNodeID);
	psDataFrame->uData.sSDO.bCmdSpecifier = uSDOCmd.tAll;
	psDataFrame->uData.sSDO.bODID_H       = psSDOMsg->tODID >> 8;
	psDataFrame->uData.sSDO.bODID_L       = psSDOMsg->tODID & 0xFF;
	psDataFrame->uData.sSDO.bODSubIndex   = psSDOMsg->bODSubIndex;

	{
		NodeID_T  tFilterNodeID;
		DBIndex_T eWatchIndex;

		tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
		if ((0 == tFilterNodeID) || (byNodeID == tFilterNodeID)) {
			eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_TX_SDO_COUNT][eChannel];
			if (eWatchIndex < DBINDEX_SUM)
				DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
		}
	}
	return CANOpen_Write(eChannel, psDataFrame);
}
/**
 * [CANOpenMaster_SendNodeGruading description]
 * @method CANOpenMaster_SendNodeGruading
 * @param  eChannel                       [description]
 * @param  byNodeID                       [description]
 * @return                                [description]
 */
E_CANOpenRetCode CANOpenMaster_SendNodeGruading(E_CANChannel eChannel, BYTE byNodeID)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	//(THJ 2016-01-13 Modify: for "Multi thread"
	//psDataFrame = &u_asCANOpenDataFrame[eChannel];
	psDataFrame = &sDataFrame;
	//)
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen            = 0;
	psDataFrame->uHead.sStand.bRTR   = 1;
	psDataFrame->uHead.sStand.bCOBID = CANOpen_GetCOBID(CANOPENFUNCID_NMT_ERROR_CTRL, byNodeID);

	{
		NodeID_T  tFilterNodeID;
		DBIndex_T eWatchIndex;

		tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
		if ((0 == tFilterNodeID) || (byNodeID == tFilterNodeID)) {
			eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_TX_NODEGUARD_COUNT][eChannel];
			if (eWatchIndex < DBINDEX_SUM)
				DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
		}
	}
	return CANOpen_Write(eChannel, psDataFrame);
}
/**
 * [CANOpenMaster_CommTypeToChannel description]
 * @param  eCommType [description]
 * @return           [description]
 */
E_CANChannel CANOpenMaster_CommTypeToChannel(E_CommType eCommType)
{
	switch (eCommType) {
	case COMMTYPE_CAN1:
		return CANCHANNAL_CAN1;

	case COMMTYPE_CAN2:
		return CANCHANNAL_CAN2;

	default: {
		DEBUG_ASSERT(false);
		return CANCHANNAL_SUM;
	}
	}
}

/**
 * [CANOpenMaster_ResetComm description]
 * @param  psCompUnique [description]
 */
void CANOpenMaster_ResetComm(CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	psCompUnique->PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_ENTERAPPSTATE_INIT;
}

/**
 * [SCANOpenSubCompUnique_Init description]
 * @method SCANOpenSubCompUnique_Init
 * @param  psSubCompUnique            [description]
 */
void SCANOpenSubCompUnique_Init(CANOpenSubCompUnique_S* psSubCompUnique)
{
	DEBUG_ASSERT(NULL != psSubCompUnique);
	psSubCompUnique->PRO_fDBIndexToODInfo = Default_DBIndexToODInfo;
}
/**
 * [SCANOpenSubCompUnique_InitDBIndexToODInfo description]
 * @param  psSubCompUnique  [description]
 * @param  fDBIndexToODInfo [description]
 * @return                  [description]
 */
void SCANOpenSubCompUnique_InitDBIndexToODInfo(CANOpenSubCompUnique_S* psSubCompUnique, CANOpenDBIndexToODInfo_F fDBIndexToODInfo)
{
	DEBUG_ASSERT(NULL != psSubCompUnique);
	psSubCompUnique->PRO_fDBIndexToODInfo = (NULL != fDBIndexToODInfo) ? fDBIndexToODInfo : Default_DBIndexToODInfo;
}
/**
 * [SCANOpenCompUnique_Init description]
 * @method SCANOpenCompUnique_Init
 * @param  psCompUnique            [description]
 */
void SCANOpenCompUnique_Init(CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	psCompUnique->PUB_blClearSize    = FALSE;
	psCompUnique->PUB_blFreeSDO      = FALSE;
	psCompUnique->PRI_fCommRun       = CANOpenMComm_Init;
	psCompUnique->PRO_psConfigFinish = Default_ConfigFinish;
	psCompUnique->PRO_bNodeID        = 0;
	psCompUnique->PRI_bSDOState      = CANCOMMSDOSTATE_IDLE;
	psCompUnique->PRO_bNodeState     = CANOPENNODESTATE_BOOTUP;
	ResetTimer(&psCompUnique->PRI_lMsgSendTime, 0);
	psCompUnique->PRO_fEventCallBack  = Default_EventCallBack;
	psCompUnique->PRI_wCurConfigIndex = 0;
	psCompUnique->PRO_eCtrlCmd        = CANOPENMASTERCTRLCMD_SUM;
}
/**
 * [SCANOpenCompUnique_InitConfigInfo description]
 * @param psUnique      [description]
 * @param fConfigFinish [description]
 */
void SCANOpenCompUnique_InitConfigInfo(CANOpenCompUnique_S* psUnique, CANOpenSDOConfigFinish_F fConfigFinish)
{
	DEBUG_ASSERT(NULL != psUnique);
	psUnique->PRO_psConfigFinish = (NULL != fConfigFinish) ? fConfigFinish : Default_ConfigFinish;
}
/**
 * [SCANOpenCompUnique_InitEventCallBack description]
 * @param psUnique       [description]
 * @param fEventCallBack [description]
 */
void SCANOpenCompUnique_InitEventCallBack(CANOpenCompUnique_S* psUnique, CANOpenEventCallBack_F fEventCallBack)
{
	DEBUG_ASSERT(NULL != psUnique);
	psUnique->PRO_fEventCallBack = (NULL != fEventCallBack) ? fEventCallBack : Default_EventCallBack;
}
/**
 * [SCANOpenCompUnique_SetNodeID description]
 * @method SCANOpenCompUnique_SetNodeID
 * @param  psCompUnique                 [description]
 * @param  byNodeID                     [description]
 */
void SCANOpenCompUnique_SetNodeID(CANOpenCompUnique_S* psCompUnique, BYTE byNodeID)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	if (psCompUnique->PRO_bNodeID != byNodeID) {
		psCompUnique->PRO_bNodeID = byNodeID;
		Error_App_Reset();
		CANOpenMaster_ResetComm(psCompUnique);
	}
}

/**
 * [CANOpenCHBManager_Init description]
 * @param psHeartbeat [description]
 */
void CANOpenCHBManager_Init(S_CANOpenCHBManager* const psHeartbeat)
{
	DEBUG_ASSERT_PARAM(NULL != psHeartbeat, "psHeartbeat is null pointer.");
	ResetTimer(&psHeartbeat->_sCheck_time, 0);
	psHeartbeat->_iRecved_count   = 0;
	psHeartbeat->_iLose_count     = 0;
	psHeartbeat->iLose_max        = 0;
	psHeartbeat->_iConflict_count = 0;
	psHeartbeat->iConflict_max    = 0;
	psHeartbeat->iTick            = 0;
}

/**
 * [CANOpenCHBManager_Reset description]
 * @param psHeartbeat [description]
 */
void CANOpenCHBManager_Reset(S_CANOpenCHBManager* const psHeartbeat)
{
	DEBUG_ASSERT_PARAM(NULL != psHeartbeat, "psHeartbeat is null pointer.");
	ResetTimer(&psHeartbeat->_sCheck_time, 0);
	psHeartbeat->_iRecved_count   = 0;
	psHeartbeat->_iLose_count     = 0;
	psHeartbeat->_iConflict_count = 0;
}

/**
 * [CANOpenCHBManager_RecvedMsg description]
 * @param psHeartbeat [description]
 */
void CANOpenCHBManager_RecvedMsg(S_CANOpenCHBManager* const psHeartbeat)
{
	DEBUG_ASSERT_PARAM(NULL != psHeartbeat, "psHeartbeat is null pointer.");
	if (psHeartbeat->_iRecved_count < 2)
		++psHeartbeat->_iRecved_count;
}

/**
 * [CANOpenCHBManager_Check description]
 * @param  psHeartbeat [description]
 * @return             [description]
 */
E_CANOpenCHBCheck CANOpenCHBManager_Check(S_CANOpenCHBManager* const psHeartbeat)
{
	DEBUG_ASSERT_PARAM(NULL != psHeartbeat, "psHeartbeat is null pointer.");
	if (CheckTimerOut(&psHeartbeat->_sCheck_time)) {
		unsigned iRecved_count;

		ResetTimer(&psHeartbeat->_sCheck_time, psHeartbeat->iTick);
		iRecved_count               = psHeartbeat->_iRecved_count;
		psHeartbeat->_iRecved_count = 0;
		switch (iRecved_count) {
		case 0:
			psHeartbeat->_iConflict_count = 0;
			if (psHeartbeat->_iLose_count >= psHeartbeat->iLose_max) {
				return CANOPENCHBCHECK_OFFLINE;
			}
			psHeartbeat->_iLose_count++;
			break;
		case 1:
			psHeartbeat->_iLose_count     = 0;
			psHeartbeat->_iConflict_count = 0;
			break;
		default:
			psHeartbeat->_iLose_count = 0;
			if (psHeartbeat->_iConflict_count >= psHeartbeat->iConflict_max) {
				return CANOPENCHBCHECK_CONFLICT;
			}
			psHeartbeat->_iConflict_count++;
			break;
		}
		return CANOPENCHBCHECK_ONLINE;
	}
	return CANOPENCHBCHECK_IDLE;
}
// /**
//  * [CANOpenMComp_SetAllDBFlag description]
//  * @method CANOpenMComp_SetAllDBFlag
//  * @param  psComp                    [description]
//  */
// static void CANOpenMComp_SetAllDBFlag(CommComp_S *psComp)
// {
//  CommSubComp_S *psCurSubComp;

//  DEBUG_ASSERT;(NULL != psComp)
//  psCurSubComp = psComp->PRI_psSubCompHead;
//  while (NULL != psCurSubComp)
//  {
//      SCommSubComp_SetAllWriteDBFlag(psCurSubComp);
//      SCommSubComp_SetAllReadDBFlag(psCurSubComp);
//      psCurSubComp = psCurSubComp->PRI_psNextSubComp;
//  }
// }
/**
 * [CANOpenMComm_Parse description]
 * @param eChannel    [description]
 * @param psCompHead  [description]
 * @param psDataFrame [description]
 */
static void CANOpenMComm_Parse(E_CANChannel eChannel, CommComp_S* psCompHead, CANOpenDataFrame_S* psDataFrame)
{
	CANOpenCompUnique_S* psCompUnique;
	WORD                 wCOBID;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(NULL != psDataFrame);
	// if (NULL == psCompHead)
	// {
	//  if (CANCHANNAL_CAN1 == eChannel)
	//  {
	//      MC_processRxCanMessages((DATAFRAME *)psDataFrame);
	//      return;
	//  }
	// }

	wCOBID = psDataFrame->uHead.sStand.bCOBID;
	{
		DBIndex_T eWatchIndex;

		DEBUG_ASSERT(eChannel < (sizeof(uc_aeMCDB_WatchIndex[0]) / sizeof(*uc_aeMCDB_WatchIndex[0])));
		eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_RX_DATA_COUNT][eChannel];
		if (eWatchIndex <= DBINDEX_SUM)
			DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
	}
	if (wCOBID == 0) {
		g_wTestCANCOBID_0++;
		return;
	}
	switch (CANOpen_GetFuncIDByCOBID(wCOBID)) {
	case CANOPENFUNCID_SEND_PDO1:
	case CANOPENFUNCID_SEND_PDO2:
	case CANOPENFUNCID_SEND_PDO3:
	case CANOPENFUNCID_SEND_PDO4: {
		{
			NodeID_T  tFilterNodeID;
			DBIndex_T eWatchIndex;

			tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
			if ((0 == tFilterNodeID) || (CANOpen_GetNodeIDByCOBID(wCOBID) == tFilterNodeID)) {
				eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_RX_PDO_COUNT][eChannel];
				if (eWatchIndex < DBINDEX_SUM)
					DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
			}
		}
		while (NULL != psCompHead) {
			psCompUnique = psCompHead->PRO_pvUnique;
			if (CANOpen_GetNodeIDByCOBID(wCOBID) == psCompUnique->PRO_bNodeID) {
				switch (psCompUnique->PRO_bNodeState) {
				case CANOPENNODESTATE_OPERATIONAL:
					break;
				default:
					return;
				}
				break;
			}
			psCompHead = psCompHead->PRI_psNextComp;
		}
		break;
	}

	case CANOPENFUNCID_SEND_SDO: {
		{
			NodeID_T  tFilterNodeID;
			DBIndex_T eWatchIndex;

			tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
			if ((0 == tFilterNodeID) || (CANOpen_GetNodeIDByCOBID(wCOBID) == tFilterNodeID)) {
				eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_RX_SDO_COUNT][eChannel];
				if (eWatchIndex < DBINDEX_SUM)
					DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
			}
		}
		// {
		//     // THJ 2015-10-23 Add: for Watch CAN.
		//     if (0 != (g_sWatchInfo.sCAN.sCurReadInfo.bDataSize | g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize))
		//     {
		//         WatchType_U uWatchType;

		//         uWatchType.dwAll = DB_GetDWORD(DBINDEX_LOCAL0_WATCH_TYPE);
		//         if ((uWatchType.sCAN.bChannel == eChannel) && uWatchType.sCAN.bNodeID == CANOpen_GetNodeIDByCOBID(psDataFrame->uHead.sStand.bCOBID))
		//         {
		//             CANOpenSDOCmdSpecifier_U uSDOCmd;

		//             uSDOCmd.tAll = psDataFrame->uData.sSDO.bCmdSpecifier;
		//             switch (uSDOCmd.sBitField.bCmd)
		//             {
		//                 case CANOPENSDOSCS_IDD:
		//                 {
		//                     if ((g_sWatchInfo.sCAN.sCurWriteInfo.bIndex == ((psDataFrame->uData.sSDO.bODID_H << 8) | psDataFrame->uData.sSDO.bODID_L))
		//                             && (g_sWatchInfo.sCAN.sCurWriteInfo.bSubIndex == psDataFrame->uData.sSDO.bODSubIndex))
		//                     {
		//                         g_sWatchInfo.sCAN.padwWriteFlag[g_sWatchInfo.sCAN.wCurWriteIndex >> 5] &= ~((DWORD)1 << (g_sWatchInfo.sCAN.wCurWriteIndex & 0x1F));
		//                         g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize = 0;
		//                     }
		//                     break;
		//                 }

		//                 case CANOPENSDOSCS_IDU:
		//                 {
		//                     if ((g_sWatchInfo.sCAN.sCurReadInfo.bIndex == ((psDataFrame->uData.sSDO.bODID_H << 8) | psDataFrame->uData.sSDO.bODID_L))
		//                             && (g_sWatchInfo.sCAN.sCurReadInfo.bSubIndex == psDataFrame->uData.sSDO.bODSubIndex))
		//                     {
		//                         DB_SetDWORD(WATCH_READ_START_INDEX(VALUE) + (g_sWatchInfo.sCAN.wCurReadIndex << 1), psDataFrame->uData.sSDO.bData);
		//                         g_sWatchInfo.sCAN.padwReadFlag[g_sWatchInfo.sCAN.wCurReadIndex >> 5] &= ~((DWORD)1 << (g_sWatchInfo.sCAN.wCurReadIndex & 0x1F));
		//                         g_sWatchInfo.sCAN.sCurReadInfo.bDataSize = 0;
		//                     }
		//                     break;
		//                 }

		//                 case CANOPENSDOSCS_ADT:
		//                     switch (psDataFrame->uData.sSDO.bData)
		//                     {
		//                         case 0x06020000:
		//                         {
		//                             Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE);
		//                             break;
		//                         }

		//                         case 0x06090011:
		//                         {
		//                             Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE);
		//                             break;
		//                         }

		//                         default:
		//                         {
		//                             Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE);
		//                             break;
		//                         }
		//                     }
		//                     if ((0 != g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize)
		//                             && (g_sWatchInfo.sCAN.sCurWriteInfo.bIndex == ((psDataFrame->uData.sSDO.bODID_H << 8) | psDataFrame->uData.sSDO.bODID_L))
		//                             && (g_sWatchInfo.sCAN.sCurWriteInfo.bSubIndex == psDataFrame->uData.sSDO.bODSubIndex))
		//                     {
		//                         DEBUG_ASSERT(g_sWatchInfo.sCAN.wCurWriteIndex < WATCHCAN_WRITEFLAG_SUM)
		//                         DB_SetDWORD(WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sCAN.wCurWriteIndex << 1), 0);
		//                         g_sWatchInfo.sCAN.padwWriteFlag[g_sWatchInfo.sCAN.wCurWriteIndex >> 5] &= ~((DWORD)1 << (g_sWatchInfo.sCAN.wCurWriteIndex & 0x1F));
		//                         g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize = 0;
		//                     }
		//                     if ((0 != g_sWatchInfo.sCAN.sCurReadInfo.bDataSize)
		//                             && (g_sWatchInfo.sCAN.sCurReadInfo.bIndex == ((psDataFrame->uData.sSDO.bODID_H << 8) | psDataFrame->uData.sSDO.bODID_L))
		//                             && (g_sWatchInfo.sCAN.sCurReadInfo.bSubIndex == psDataFrame->uData.sSDO.bODSubIndex))
		//                     {
		//                         DEBUG_ASSERT(g_sWatchInfo.sCAN.wCurReadIndex < WATCHCAN_READFLAG_SUM)
		//                         DB_SetDWORD(WATCH_READ_START_INDEX(VALUE) + (g_sWatchInfo.sCAN.wCurReadIndex << 1), 0);
		//                         g_sWatchInfo.sCAN.padwReadFlag[g_sWatchInfo.sCAN.wCurReadIndex >> 5] &= ~((DWORD)1 << (g_sWatchInfo.sCAN.wCurReadIndex & 0x1F));
		//                         g_sWatchInfo.sCAN.sCurReadInfo.bDataSize = 0;
		//                     }
		//                     break;

		//                 default:
		//                     break;
		//             }
		//         }
		//     }
		// }

		ParseCANSDO_Easy(eChannel, psDataFrame);
		// Watch of CAN.
		{
			WORD i;

			for (i = 0; i < TASK_WATCH_SUM; i++)
				Watch_Parser_Can(&g_asWatch[i], psDataFrame);
			Watch_Parser_Can(&g_sOldWatch.sWatch, psDataFrame);
		}
		while (NULL != psCompHead) {
			psCompUnique = psCompHead->PRO_pvUnique;
			if (CANOpen_GetNodeIDByCOBID(wCOBID) == psCompUnique->PRO_bNodeID) {
				switch (psCompUnique->PRO_bNodeState) {
				case CANOPENNODESTATE_OPERATIONAL:
				case CANOPENNODESTATE_PREOPERATIONAL: {
					if (!CANOpenMComm_ParseCANRecvedSDO(psCompHead, psCompUnique, psDataFrame))
					{
						psCompHead = NULL;
						continue;
					}
					break;
				}

				default:
					return;
				}
				break;
			}
			psCompHead = psCompHead->PRI_psNextComp;
		}
		break;
	}

	case CANOPENFUNCID_NMT_ERROR_CTRL: {
		{
			NodeID_T  tFilterNodeID;
			DBIndex_T eWatchIndex;

			tFilterNodeID = DB_GetWORD(uc_aeMCDB_WatchIndex[CANOPENWATCH_NODEID_FILTER][eChannel]);
			if ((0 == tFilterNodeID) || (CANOpen_GetNodeIDByCOBID(wCOBID) == tFilterNodeID)) {
				eWatchIndex = uc_aeMCDB_WatchIndex[CANOPENWATCH_RX_HEARTBEAT_COUNT][eChannel];
				if (eWatchIndex < DBINDEX_SUM)
					DB_SetDWORD(eWatchIndex, DB_GetDWORD(eWatchIndex) + 1);
			}
		}
		//{
		//WORD wNodeID = CANOpen_GetNodeIDByCOBID(wCOBID) ;
		//if (g_wTestCAN1 && wNodeID == 0x2 )
		//    ESTOP0;
		//}
		while (NULL != psCompHead) {
			psCompUnique = psCompHead->PRO_pvUnique;
			if (CANOpen_GetNodeIDByCOBID(wCOBID) == psCompUnique->PRO_bNodeID) {
				CANOpenMComm_ParseCANRecvedNMTErrCtrl(psCompHead, psCompUnique, psDataFrame);
				break;
			}
			psCompHead = psCompHead->PRI_psNextComp;
		}
		break;
	}

	default: {
		psCompHead = NULL;
		break;
	}
	}

	if (NULL != psCompHead) {
		DEBUG_ASSERT(NULL != psCompUnique);
		DEBUG_ASSERT(NULL != psCompUnique->PRO_fEventCallBack);
		psCompUnique->PRO_fEventCallBack(psCompUnique, psDataFrame, CANOPENEVENTTYPE_RECVED_DATAFRAME);
	}
}
/**
 * [CANOpenMComm_ParseCANRecvedSDO description]
 * @method CANOpenMComm_ParseCANRecvedSDO
 * @param  psComp                         [description]
 * @param  psCompUnique                   [description]
 * @param  psDataFrame                    [description]
 */
inline bool CANOpenMComm_ParseCANRecvedSDO(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame)
{
	CANOpenSubCompUnique_S*    psSubCompUnique;
	struct CANOpenCommSDOMsg_S sSDOMsg;
	DWORD                      dwTmp;
	CANOpenODInfo_S            sODInfo_Tmp, sODInfo;
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	DEBUG_ASSERT(NULL != psDataFrame);
	DEBUG_ASSERT((CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
	    || (CANOPENNODESTATE_PREOPERATIONAL == psCompUnique->PRO_bNodeState));
	switch (psCompUnique->PRO_bNodeState) {
	case CANOPENNODESTATE_OPERATIONAL:
		sODInfo_Tmp.tODID = psDataFrame->uData.sSDO.bODID_H << 8;
		sODInfo_Tmp.tODID |= psDataFrame->uData.sSDO.bODID_L;
		sODInfo_Tmp.bSubIndex = psDataFrame->uData.sSDO.bODSubIndex;
		if (psCompUnique->PUB_blFreeSDO) {
			psCompUnique->PUB_blFreeSDO = FALSE;
		} else {
			switch (psComp->PRO_bSDOState) {
			case COMMSDOSTATE_READ:
				DEBUG_ASSERT(psComp->PRI_tCurIndex < DBINDEX_SUM);
				DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
				psSubCompUnique = psComp->PRI_psCurSubComp->PRO_pvUnique;
				DEBUG_ASSERT(NULL != psSubCompUnique);
				DEBUG_ASSERT(NULL != psSubCompUnique->PRO_fDBIndexToODInfo);
				sODInfo = psSubCompUnique->PRO_fDBIndexToODInfo(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
				if ((sODInfo_Tmp.tODID == sODInfo.tODID) && (sODInfo_Tmp.bSubIndex == sODInfo.bSubIndex)) {
					CANOpenSDOCmdSpecifier_U uSDOCmd;

					// Comm_ClrReadFlag(psComp->PRI_tCurIndex);
					uSDOCmd.tAll = psDataFrame->uData.sSDO.bCmdSpecifier;
					if (CANOPENSDOSCS_ADT != uSDOCmd.sBitField.bCmd) {
						dwTmp = psDataFrame->uData.sU32.bData1;
						if (!Comm_GetWriteChanged(psComp->PRI_tCurIndex)){ // THJ 2016-11-28 Add.
							DWORD dwLastData;

							dwLastData = 0;
							DB_GetData(psComp->PRI_tCurIndex, &dwLastData);
							if (dwLastData != dwTmp) {
								DB_SetData(psComp->PRI_tCurIndex, &dwTmp);
								DB_CallCallBack(psComp->PRI_tCurIndex, DBEVENT_WRITE_AFTER, &dwTmp);
							}
						}
					}
					psCompUnique->PRI_bSDOState = CANCOMMSDOSTATE_IDLE;
					SCommComp_ClrSDOState(psComp);
				}
				else
					return false;
				break;

			case COMMSDOSTATE_WRITE:
				DEBUG_ASSERT(psComp->PRI_tCurIndex < DBINDEX_SUM);
				DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
				psSubCompUnique = psComp->PRI_psCurSubComp->PRO_pvUnique;
				DEBUG_ASSERT(NULL != psSubCompUnique);
				DEBUG_ASSERT(NULL != psSubCompUnique->PRO_fDBIndexToODInfo);
				sODInfo = psSubCompUnique->PRO_fDBIndexToODInfo(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
				if ((sODInfo_Tmp.tODID == sODInfo.tODID) && (sODInfo_Tmp.bSubIndex == sODInfo.bSubIndex)) {
					// Comm_ClrWriteFlag(psComp->PRI_tCurIndex);
					psCompUnique->PRI_bSDOState = CANCOMMSDOSTATE_IDLE;
					SCommComp_ClrSDOState(psComp);
				}
				else
					return false;
				break;
			default:
				return false;
			}
		}
		break;

	case CANOPENNODESTATE_PREOPERATIONAL:
		DEBUG_ASSERT(NULL != psCompUnique->PRO_psConfigFinish);
		if (psCompUnique->PRO_psConfigFinish(psCompUnique, &sSDOMsg, psCompUnique->PRI_wCurConfigIndex)) {
			psCompUnique->PRI_bSDOState = CANCOMMSDOSTATE_IDLE;
			return false;
		}
		dwTmp = ((DWORD)psDataFrame->uData.sSDO.bODID_H << 8) | ((DWORD)psDataFrame->uData.sSDO.bODID_L);
		if ((dwTmp == sSDOMsg.tODID)
		    && (psDataFrame->uData.sSDO.bODSubIndex == sSDOMsg.bODSubIndex)) {
			CANOpenSDOCmdSpecifier_U uSDOCmd;

			uSDOCmd.tAll = psDataFrame->uData.sSDO.bCmdSpecifier;
			if (CANOPENSDOSCS_ADT != uSDOCmd.sBitField.bCmd) {
				switch (sSDOMsg.bSDOAccess) {
				case CANCOMMSDOACCESS_UPLOAD:
				// THJ 2017-06-14 mask.
				/* {
					if (psDataFrame->uData.sU32.bData1 != sSDOMsg.dwValue)
						break;
					// else
					// {
					//  ++psCompUnique->PRI_wCurConfigIndex;
					//  psCompUnique->PRI_bSDOState = CANCOMMSDOSTATE_IDLE;
					//  break;
					// }
				}*/
				case CANCOMMSDOACCESS_DOWNLOAD:
					++psCompUnique->PRI_wCurConfigIndex;
					psCompUnique->PRI_bSDOState = CANCOMMSDOSTATE_IDLE;
					break;

				default:
					DEBUG_ASSERT(false);
					break;
				}
			}
			else
				CANOpenMaster_ResetComm(psCompUnique);
		}
		else
			return false;
		break;
	default:
		return false;
	}
	return true;
}
/**
 * [CANOpenMComm_ParseCANRecvedNMTErrCtrl description]
 * @method CANOpenMComm_ParseCANRecvedNMTErrCtrl
 * @param  psComp                                [description]
 * @param  psCompUnique                          [description]
 * @param  psDataFrame                           [description]
 */
inline void CANOpenMComm_ParseCANRecvedNMTErrCtrl(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame)
{
	BYTE byNodeState;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	DEBUG_ASSERT(NULL != psDataFrame);
	byNodeState = psDataFrame->uData.sU8.bData0 & (~CANOPENNODEGUARDING_TRIGGERBIT);
	switch (byNodeState) {
	case CANOPENNODESTATE_BOOTUP: {
		psCompUnique->PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_ENTERAPPSTATE_CONFIG;
		break;
	}
	//case CANOPENNODESTATE_OPERATIONAL:
	//{
	//    if ((CANOPENNODESTATE_PREOPERATIONAL == psCompUnique->PRO_bNodeState)
	//            && (CANOpenMComm_WaitSlaveReady == psCompUnique->PRI_fCommRun))
	//    {
	//        psCompUnique->PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_ENTERAPPSTATE_RUN;
	//        break;
	//    }
	//    //if (byNodeState != psCompUnique->PRO_bNodeState)
	//    //  CANOpenMaster_ResetComm(psCompUnique);
	//    //break;
	//}
	default: {
		if (byNodeState != psCompUnique->PRO_bNodeState) {
			if (CtrlCmdFunc_ENTERAPPSTATE_WAIT != psCompUnique->PRI_fCommRun)
				CANOpenMaster_ResetComm(psCompUnique);
		} else {
			if (CtrlCmdFunc_ENTERAPPSTATE_WAIT == psCompUnique->PRI_fCommRun) {
				switch (byNodeState) {
				case CANOPENNODESTATE_OPERATIONAL: {
					DEBUG_ASSERT(NULL != psCompUnique->PRO_fEventCallBack);
					psCompUnique->PRO_fEventCallBack(psCompUnique, NULL, CANOPENEVENTTYPE_CHANGE_NODESTATE);
					psCompUnique->PRI_fCommRun = CANOpenMComm_Run;
					break;
				}
				case CANOPENNODESTATE_PREOPERATIONAL: {
					psCompUnique->PRI_fCommRun = CANOpenMComm_ConfigSlave;
					break;
				}
				default:
					break;
				}
			}
		}
		break;
	}
	}
}
/**
 * [CANOpenMComm_Run description]
 * @method CANOpenMComm_Run
 * @param  psComp           [description]
 * @param  psCompUnique     [description]
 */
static void CANOpenMComm_Run(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	DEBUG_ASSERT(CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState);
	switch (psComp->PRO_bSDOState) {
	case COMMSDOSTATE_READ: {
		if (CheckTimerOut(&psCompUnique->PRI_lMsgSendTime)) {
			CANOpenSubCompUnique_S* psSubCompUnique;
			CANOpenCommSDOMsg_S     sSDOMsg;
			CANOpenODInfo_S         sODInfo;

			ResetTimer(&psCompUnique->PRI_lMsgSendTime, CANOPENCOMM_SDO_RESENDTIME);
			DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
			psSubCompUnique = psComp->PRI_psCurSubComp->PRO_pvUnique;
			DEBUG_ASSERT(NULL != psSubCompUnique);
			DEBUG_ASSERT(NULL != psSubCompUnique->PRO_fDBIndexToODInfo);
			sODInfo = psSubCompUnique->PRO_fDBIndexToODInfo(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
			if (0 == sODInfo.bDataSize)
				SCommComp_ClrSDOState(psComp);
			else {
				sSDOMsg.tODID       = sODInfo.tODID;
				sSDOMsg.bODSubIndex = sODInfo.bSubIndex;
				sSDOMsg.bSDOAccess  = CANCOMMSDOACCESS_UPLOAD;
				sSDOMsg.bBytes      = (psCompUnique->PUB_blClearSize) ? sODInfo.bDataSize : 0;
				sSDOMsg.dwValue     = 0;
				CANOpenMaster_SendSDO(CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp)), psCompUnique->PRO_bNodeID, &sSDOMsg);
			}
		}
		break;
	}

	case COMMSDOSTATE_WRITE: {
		if (CheckTimerOut(&psCompUnique->PRI_lMsgSendTime)) {
			CANOpenSubCompUnique_S* psSubCompUnique;
			CANOpenCommSDOMsg_S     sSDOMsg;
			CANOpenODInfo_S         sODInfo;

			ResetTimer(&psCompUnique->PRI_lMsgSendTime, CANOPENCOMM_SDO_RESENDTIME);
			DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
			psSubCompUnique = psComp->PRI_psCurSubComp->PRO_pvUnique;
			DEBUG_ASSERT(NULL != psSubCompUnique);
			DEBUG_ASSERT(NULL != psSubCompUnique->PRO_fDBIndexToODInfo);
			sODInfo = psSubCompUnique->PRO_fDBIndexToODInfo(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
			if (0 == sODInfo.bDataSize)
				SCommComp_ClrSDOState(psComp);
			else {
				sSDOMsg.tODID       = sODInfo.tODID;
				sSDOMsg.bODSubIndex = sODInfo.bSubIndex;
				sSDOMsg.bSDOAccess  = CANCOMMSDOACCESS_DOWNLOAD;
				sSDOMsg.bBytes      = (psCompUnique->PUB_blClearSize) ? sODInfo.bDataSize : 0;
				sSDOMsg.dwValue     = 0;
				DB_GetData(psComp->PRI_tCurIndex, &sSDOMsg.dwValue);
				CANOpenMaster_SendSDO(CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp)), psCompUnique->PRO_bNodeID, &sSDOMsg);
			}
		}
		break;
	}

	default: {
		ResetTimer(&psCompUnique->PRI_lMsgSendTime, 0);
		break;
	}
	}
}
/**
 * [CANOpenMComm_Init description]
 * @method CANOpenMComm_Init
 * @param  psComp            [description]
 * @param  psCompUnique      [description]
 */
static void CANOpenMComm_Init(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	if (0 != psCompUnique->PRO_bNodeID) {
		//psCompUnique->PRI_fCommRun = CANOpenMComm_WaitSlaveReady;
		//psCompUnique->PRO_bNodeState = CANOPENNODESTATE_BOOTUP;
		psCompUnique->PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_ENTERAPPSTATE_CONFIG;
		//CANOpenMaster_SendNodeGruading(eChannel, psCompUnique->PRO_bNodeID);
		//ResetTimer(&psCompUnique->PRI_lMsgSendTime, CANOPENCOMM_NODEGUARDING_RESENDTIME);
	}
}
/**
 * [CANOpenMComm_WaitSlaveReady description]
 * @method CANOpenMComm_WaitSlaveReady
 * @param  psComp                      [description]
 * @param  psCompUnique                [description]
 */
//static void CANOpenMComm_WaitSlaveReady(CommComp_S *psComp, CANOpenCompUnique_S *psCompUnique)
//{
//    if (CheckTimerOut(&psCompUnique->PRI_lMsgSendTime))
//        CANOpenMaster_ResetComm(psCompUnique);
//}
/**
 * [CANOpenMComm_ConfigSlave description]
 * @method CANOpenMComm_ConfigSlave
 * @param  psComp                   [description]
 * @param  psCompUnique             [description]
 */
static void CANOpenMComm_ConfigSlave(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	struct CANOpenCommSDOMsg_S sSDOMsg;
	E_CANChannel               eChannel;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp));
	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	switch (psCompUnique->PRI_bSDOState) {
	case CANCOMMSDOSTATE_IDLE: {
		if (psCompUnique->PRO_psConfigFinish(psCompUnique, &sSDOMsg, psCompUnique->PRI_wCurConfigIndex)) {
			psCompUnique->PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_ENTERAPPSTATE_RUN;
			break;
		} else {
			psCompUnique->PRI_bSDOState = CANCOMMSDOSTATE_WRITE;
			ResetTimer(&psCompUnique->PRI_lMsgSendTime, 0);
			// do "CANCOMMSDOSTATE_WRITE"
		}
	}

	case CANCOMMSDOSTATE_WRITE: {
		if (CheckTimerOut(&psCompUnique->PRI_lMsgSendTime)) {
			DEBUG_CODE(int iTmp;)
			DEBUG_ASSERT(NULL != psCompUnique->PRO_psConfigFinish);
			DEBUG_CODE(iTmp =)
			psCompUnique->PRO_psConfigFinish(psCompUnique, &sSDOMsg, psCompUnique->PRI_wCurConfigIndex);
			DEBUG_ASSERT(FALSE == iTmp);
			CANOpenMaster_SendSDO(eChannel, psCompUnique->PRO_bNodeID, &sSDOMsg);
			ResetTimer(&psCompUnique->PRI_lMsgSendTime, CANOPENCOMM_SDO_RESENDTIME);
		}
		break;
	}

	default:
		break;
	}
}
/**
 * [CtrlCmdFunc_ENTERAPPSTATE_INIT description]
 * @param psComp       [description]
 * @param psCompUnique [description]
 */
static void CtrlCmdFunc_ENTERAPPSTATE_INIT(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	SCommComp_SetState(psComp, COMPSTATE_INIT);
	psCompUnique->PRO_bNodeState = CANOPENNODESTATE_BOOTUP;
	psCompUnique->PRI_fCommRun   = CANOpenMComm_Init;
	DEBUG_ASSERT(NULL != psCompUnique->PRO_fEventCallBack);
	psCompUnique->PRO_fEventCallBack(psCompUnique, NULL, CANOPENEVENTTYPE_CHANGE_NODESTATE);
}
/**
 * [CtrlCmdFunc_ENTERAPPSTATE_CONFIG description]
 * @param psComp       [description]
 * @param psCompUnique [description]
 */
static void CtrlCmdFunc_ENTERAPPSTATE_CONFIG(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	SCommComp_SetState(psComp, COMPSTATE_CONFIG);
	psCompUnique->PRI_wCurConfigIndex = (DBIndex_T)0;
	psCompUnique->PRI_bSDOState       = CANCOMMSDOSTATE_IDLE;
	psCompUnique->PRO_bNodeState      = CANOPENNODESTATE_PREOPERATIONAL;
	psCompUnique->PRI_fCommRun        = CtrlCmdFunc_ENTERAPPSTATE_WAIT;
	DEBUG_ASSERT(NULL != psCompUnique->PRO_fEventCallBack);
	psCompUnique->PRO_fEventCallBack(psCompUnique, NULL, CANOPENEVENTTYPE_CHANGE_NODESTATE);
	ResetTimer(&psCompUnique->PRI_lMsgSendTime, 0);
}

/**
 * [CtrlCmdFunc_ENTERAPPSTATE_RUN description]
 * @param psComp       [description]
 * @param psCompUnique [description]
 */
static void CtrlCmdFunc_ENTERAPPSTATE_RUN(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psComp);
	// SCommComp_SetAllWriteDBFlag(psComp);
	SCommComp_SetState(psComp, COMPSTATE_SYNC);
	DEBUG_ASSERT(NULL != psCompUnique);
	psCompUnique->PRO_bNodeState = CANOPENNODESTATE_OPERATIONAL;
	psCompUnique->PRI_fCommRun   = CtrlCmdFunc_ENTERAPPSTATE_WAIT;
	ResetTimer(&psCompUnique->PRI_lMsgSendTime, 0);
}

/**
 * 
 * 
 * @author zhang (2017/4/25)
 * 
 * @param psComp 
 * @param psCompUnique 
 */
static void CtrlCmdFunc_ENTERAPPSTATE_WAIT(CommComp_S* psComp, CANOpenCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	if (CheckTimerOut(&psCompUnique->PRI_lMsgSendTime)) {
		E_CANOpenNMTCtrlCmd eNMTCtrlCMD;
		E_CANChannel        eChannel;

		ResetTimer(&psCompUnique->PRI_lMsgSendTime, 20);
		switch (psCompUnique->PRO_bNodeState) {
		case CANOPENNODESTATE_OPERATIONAL:
			eNMTCtrlCMD = CANOPENNMTCTRLCMD_START_REMOTE_NODE;
			break;
		case CANOPENNODESTATE_PREOPERATIONAL:
			eNMTCtrlCMD = CANOPENNMTCTRLCMD_ENTER_PREOPERATIONAL_STATE;
			break;
		default:
			CANOpenMaster_ResetComm(psCompUnique);
			return;
		}
		eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp));
		DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
		CANOpenMaster_SendNMTModuleCtrl(eChannel, psCompUnique->PRO_bNodeID, eNMTCtrlCMD); //CANOPENNMTCTRLCMD_RESET_COMMUNICATION);
		CANOpenMaster_SendNodeGruading(eChannel, psCompUnique->PRO_bNodeID);               // THJ 2017-1-24 Add.
	} else {
		if (CANOPENNODESTATE_PREOPERATIONAL == psCompUnique->PRO_bNodeState)
			psCompUnique->PRI_fCommRun = CANOpenMComm_ConfigSlave;
	}
}

/**
 * [Default_DBIndexToODInfo description]
 * @param  psComp    [description]
 * @param  psSubComp [description]
 * @param  tDBIndex  [description]
 * @return           [description]
 */
static CANOpenODInfo_S Default_DBIndexToODInfo(CommComp_S* psComp, CommSubComp_S* psSubComp, DBIndex_T tDBIndex)
{
	CANOpenODInfo_S sODInfo;
	DWORD           dwTmp;

	dwTmp             = DB_IndexToID(tDBIndex) & 0xFFFFFF;
	sODInfo.tODID     = dwTmp >> 8;
	sODInfo.bSubIndex = dwTmp & 0xFF;
	sODInfo.bDataSize = DB_SizeofBData(tDBIndex);
	return sODInfo;
}
/**
 * [Default_ConfigFinish description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool Default_ConfigFinish(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	return true;
}
/**
 * [Default_EventCallBack description]
 * @param psCompUnique [description]
 * @param psDataFrame  [description]
 * @param eEventType   [description]
 */
static void Default_EventCallBack(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType)
{
}

/**
 * [ParseCANSDO_Easy description]
 * @author hankin (2019/07/22)
 * @param  wChannal   [description]
 * @param  pDataFrame [description]
 */
static void ParseCANSDO_Easy(WORD wChannal, CANOpenDataFrame_S const* pDataFrame)
{
    TCANSDO_EasyController* pEasyController;

    if (wChannal >= (sizeof(g_aCANSDO_EasyController) / sizeof(*g_aCANSDO_EasyController))) {
        return;
    }
    pEasyController = g_aCANSDO_EasyController + wChannal;
    if (pEasyController->m_bSdoMux)
    {
        WORD          wNodeID;
        WORD          wIndex;
        WORD          wSubIndex;
        CANOpenSDOCmdSpecifier_U uSDOCmd;

        wNodeID      = CANOpen_GetNodeIDByCOBID(pDataFrame->uHead.sStand.bCOBID);
        uSDOCmd.tAll = pDataFrame->uData.sSDO.bCmdSpecifier;
        wIndex       = ((WORD)pDataFrame->uData.sSDO.bODID_H << 8) | pDataFrame->uData.sSDO.bODID_L;
        wSubIndex    = pDataFrame->uData.sSDO.bODSubIndex;
        switch (uSDOCmd.sBitField.bCmd)
        {
            case CANOPENSDOSCS_IDU:
                if (pEasyController->m_wUploadNodeID == wNodeID && pEasyController->m_wUploadSdoIndex == wIndex && pEasyController->m_wUploadSdoSubIndex == wSubIndex)
                {
                    memcpy(pEasyController->m_pUploadSdoBuffer, &pDataFrame->uData.sSDO.bData, pEasyController->m_wUploadSdoByteSize >> 1);
                    pEasyController->m_wUploadSdoResult = SDORESULT_DONE;
                }
                break;
            case CANOPENSDOSCS_IDD:
                if (pEasyController->m_wDownloadNodeID == wNodeID && pEasyController->m_wDownloadSdoIndex == wIndex && pEasyController->m_wDownloadSdoSubIndex == wSubIndex)
                {
                    pEasyController->m_wDownloadSdoResult = SDORESULT_DONE;
                }
                break;
            case CANOPENSDOSCS_ADT:
                switch (pEasyController->m_bSdoType)
                {
                    case MC_SDOTYPE_UPLOAD:
                        if (pEasyController->m_wUploadNodeID == wNodeID && pEasyController->m_wUploadSdoIndex == wIndex && pEasyController->m_wUploadSdoSubIndex == wSubIndex)
                        {
                            pEasyController->m_wUploadSdoResult = SDORESULT_SLAVEABORT;
                        }
                        break;
                    case MC_SDOTYPE_DOWNLOAD:
                        if (pEasyController->m_wDownloadNodeID == wNodeID && pEasyController->m_wDownloadSdoIndex == wIndex && pEasyController->m_wDownloadSdoSubIndex == wSubIndex)
                        {
                            pEasyController->m_wDownloadSdoResult = SDORESULT_SLAVEABORT;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

/**
 * [UploadCANSDO_Easy description]
 * @author hankin (2019/07/22)
 * @param  wChannal  [description]
 * @param  wNodeID   [description]
 * @param  wIndex    [description]
 * @param  wSubIndex [description]
 * @param  pData     [description]
 * @param  wByteSize [description]
 * @return           [description]
 */
WORD UploadCANSDO_Easy(WORD wChannal, WORD wNodeID, WORD wIndex, WORD wSubIndex, WORD * pData, WORD wByteSize)
{
    TCANSDO_EasyController* pEasyController;
    WORD                    wResult;

    if (wChannal >= (sizeof(g_aCANSDO_EasyController) / sizeof(*g_aCANSDO_EasyController))) {
        return SDORESULT_NONE;
    }
    pEasyController = g_aCANSDO_EasyController + wChannal;
    wResult         = pEasyController->m_wUploadSdoResult;
    switch (wResult)
    {
        case SDORESULT_NONE:
        {
            CANOpenCommSDOMsg_S sSDOMsg;

            DINT;
            if (pEasyController->m_bSdoMux)
            {
                EINT;
                break;
            }
            pEasyController->m_bSdoMux = TRUE;
            EINT;
            ResetTimer(&pEasyController->m_dwSdoMuxTimer, CAN_SDOMUX_TIMEROUT);

            pEasyController->m_bSdoType           = MC_SDOTYPE_UPLOAD;
            pEasyController->m_wUploadNodeID      = wNodeID;
            pEasyController->m_wUploadSdoIndex    = wIndex;
            pEasyController->m_wUploadSdoSubIndex = wSubIndex;
            pEasyController->m_pUploadSdoBuffer   = pData;
            pEasyController->m_wUploadSdoByteSize = wByteSize;

            //MC_processTransmitSDOCInitDomainUpload(wNodeID, wIndex, wSubIndex, wByteSize);
            sSDOMsg.bSDOAccess  = CANCOMMSDOACCESS_UPLOAD;
            sSDOMsg.tODID       = wIndex;
            sSDOMsg.bODSubIndex = wSubIndex;
            sSDOMsg.bBytes      = wByteSize;
            CANOpenMaster_SendSDO((E_CANChannel)wChannal, wNodeID, &sSDOMsg);

            pEasyController->m_wUploadSdoResult = SDORESULT_BUSY;
            return SDORESULT_NONE;
        }
        case SDORESULT_DONE:
        case SDORESULT_TIMEOUT:
        case SDORESULT_SLAVEABORT:
            if ((wIndex == pEasyController->m_wUploadSdoIndex) && (wSubIndex == pEasyController->m_wUploadSdoSubIndex))
            {
                pEasyController->m_bSdoMux          = FALSE;
                pEasyController->m_wUploadSdoResult = SDORESULT_NONE;
                return wResult;
            }
            break;
        default:
            break;
    }
    if (CheckTimerOut(&pEasyController->m_dwSdoMuxTimer))
    {
        pEasyController->m_bSdoMux          = FALSE;
        pEasyController->m_wUploadSdoResult = SDORESULT_NONE;
        return SDORESULT_NONE;
    }
    return SDORESULT_NONE;
}

/**
 * [DownloadCANSDO_Easy description]
 * @author hankin (2019/07/22)
 * @param  wChannal  [description]
 * @param  wNodeID   [description]
 * @param  wIndex    [description]
 * @param  wSubIndex [description]
 * @param  dwData    [description]
 * @param  wByteSize [description]
 * @return           [description]
 */
WORD DownloadCANSDO_Easy(WORD wChannal, WORD wNodeID, WORD wIndex, WORD wSubIndex, DWORD dwData, WORD wByteSize)
{
    TCANSDO_EasyController* pEasyController;
    WORD                    wResult;

    if (wChannal >= (sizeof(g_aCANSDO_EasyController) / sizeof(*g_aCANSDO_EasyController))) {
        return SDORESULT_NONE;
    }
    pEasyController = g_aCANSDO_EasyController + wChannal;
    wResult         = pEasyController->m_wDownloadSdoResult;
    switch (wResult)
    {
        case SDORESULT_NONE:
        {
            CANOpenCommSDOMsg_S sSDOMsg;

            DINT;
            if (pEasyController->m_bSdoMux)
            {
                EINT;
                break;
            }
            pEasyController->m_bSdoMux = TRUE;
            EINT;
            ResetTimer(&pEasyController->m_dwSdoMuxTimer, CAN_SDOMUX_TIMEROUT);

            pEasyController->m_bSdoType             = MC_SDOTYPE_DOWNLOAD;
            pEasyController->m_wDownloadNodeID      = wNodeID;
            pEasyController->m_wDownloadSdoIndex    = wIndex;
            pEasyController->m_wDownloadSdoSubIndex = wSubIndex;
            pEasyController->m_dwDownloadSdoBuffer  = dwData;
            pEasyController->m_wDownloadSdoByteSize = wByteSize;

            sSDOMsg.bSDOAccess  = CANCOMMSDOACCESS_DOWNLOAD;
            sSDOMsg.tODID       = wIndex;
            sSDOMsg.bODSubIndex = wSubIndex;
            sSDOMsg.dwValue     = dwData;
            sSDOMsg.bBytes      = wByteSize;
            CANOpenMaster_SendSDO((E_CANChannel)wChannal, wNodeID, &sSDOMsg);

            pEasyController->m_wDownloadSdoResult = SDORESULT_BUSY;
            return SDORESULT_NONE;
        }
        case SDORESULT_DONE:
        case SDORESULT_TIMEOUT:
        case SDORESULT_SLAVEABORT:
            if ((wIndex == pEasyController->m_wDownloadSdoIndex) && (wSubIndex == pEasyController->m_wDownloadSdoSubIndex))
            {
                pEasyController->m_bSdoMux            = FALSE;
                pEasyController->m_wDownloadSdoResult = SDORESULT_NONE;
                return wResult;
            }
            break;
        default:
            break;
    }
    if (CheckTimerOut(&pEasyController->m_dwSdoMuxTimer))
    {
        pEasyController->m_bSdoMux            = FALSE;
        pEasyController->m_wDownloadSdoResult = SDORESULT_NONE;
        return SDORESULT_NONE;
    }
    return SDORESULT_NONE;
}
