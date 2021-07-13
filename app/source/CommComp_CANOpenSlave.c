/*==============================================================================+
|  Function : Communication Component CANOpen Master                            |
|  Task     : Communication Component CANOpen Master Source File                |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : SHANLL                                                            |
|  Version  : V1.00                                                             |
|  Creation : 2021/03/31                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "CommComp_CANOpenMaster.h"
#include "CommComp_CANOpenSlave.h"
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
//##############################################################################
//
//      Enumeration
//
//##############################################################################
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef void (*CtrlCmdFunc_F)(CommComp_S* psComp, CommComp_CANOpenSlave* psCompSlave);
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void		Init_CommCanopenSlave();
static void CANOpenSComm_Parse(E_CANChannel eChannel, CANOpenDataFrame_S* psDataFrame);
//inline bool CANOpenSComm_ParseCANRecvedSDO(CommComp_CANOpenSlave* psCompSlave, CANOpenDataFrame_S* psDataFrame);
inline void CANOpenSComm_ParseCANRecvedNMTModuleCtrl(CommComp_CANOpenSlave* psCompSlave, CANOpenDataFrame_S* psDataFrame);

static void CANOpenSComm_Init(CommComp_CANOpenSlave* psCompSlave);
static void Default_SlaveEventCallBack(CommComp_CANOpenSlave* psCompSlave, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);
inline void Default_SlaveCustomFunc(CommComp_CANOpenSlave* psCompSlave);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
CommComp_CANOpenSlaveManager	g_sCanopenSlaveManager;
//##############################################################################
//
//      Function Prototype
//
//##############################################################################


/**
 * [Init_CommCanopenSlave description]
 */
void Init_CommCanopenSlave()
{
	memset(&g_sCanopenSlaveManager, 0, sizeof(g_sCanopenSlaveManager));
}

/**
 * [Parse_CANopenSlave description]
 * @param eCommType   [description]
 */
void Parse_CANopenSlave(E_CommType eCommType)
{
	E_CANChannel eChannel;
	CANOpenDataFrame_S sCANOpenDataFrame;

	eChannel = CANOpenMaster_CommTypeToChannel(eCommType);//可借用MASTER的方式获取通道
	CANOpen_TX(eChannel);
	CANOpen_RX(eChannel);
	while (CANOPENRETCODE_SUCCESS == CANOpen_Read(eChannel, &sCANOpenDataFrame))
		CANOpenSComm_Parse(eChannel, &sCANOpenDataFrame);
}
/**
 * [Parse_CANopenSlave description]
 * @param eCommType   [description]
 */
void	Run_CANOpenSlave(E_CommType eCommType)
{
//	DEBUG_ASSERT(NULL != g_sCanopenSlaveManager.s_cCommCanopenSlave->PRO_fCustomFunc);
	g_sCanopenSlaveManager.s_cCommCanopenSlave->PRO_fCustomFunc(g_sCanopenSlaveManager.s_cCommCanopenSlave);
}
/**
 * [CANOpenSlave_SendNMTErrCtrl description]
 * @method CANOpenSlave_SendNMTErrCtrl
 * @param  eChannel                        [description]
 * @param  byNodeID                        [description]
 * @param  eFuncIDOfNMT                    [description]
 * @param  eNMTCtrlCmd                     [description]
 * @return                                 [description]
 */
E_CANOpenRetCode CANOpenSlave_SendNMTErrCtrl(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfNMT, E_CANOpenNodeState eNMTErrCtrl)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT((CANOPENNODESTATE_BOOTUP == eNMTErrCtrl)
	    || (CANOPENNODESTATE_STOPPED == eNMTErrCtrl)
	    || (CANOPENNODESTATE_OPERATIONAL == eNMTErrCtrl)
	    || (CANOPENNODESTATE_PREOPERATIONAL == eNMTErrCtrl));
	//(THJ 2016-01-13 Modify: for "Multi thread"
	//psDataFrame = &u_asCANOpenDataFrame[eChannel];
	psDataFrame = &sDataFrame;
	//)
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen         = 1;
	psDataFrame->uHead.sStand.bCOBID = CANOpen_GetCOBID(eFuncIDOfNMT, byNodeID);
	psDataFrame->uData.sU8.bData0 = eNMTErrCtrl;

	return CANOpen_Write(eChannel, psDataFrame);
}

/**
 * [CANOpenSlave_SendRPDO description]
 * @param  eChannel     [description]
 * @param  byNodeID     [description]
 * @param  eFuncIDOfPDO [description]
 * @return              [description]
 */
E_CANOpenRetCode CANOpenSlave_SendRPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT((CANOPENFUNCID_SEND_PDO1 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_SEND_PDO2 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_SEND_PDO3 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_SEND_PDO4 == eFuncIDOfPDO));
	psDataFrame = &sDataFrame;
	memset(psDataFrame, 0, sizeof(*psDataFrame));
	psDataFrame->tDataLen            = 0;
	psDataFrame->uHead.sStand.bCOBID = CANOpen_GetCOBID(eFuncIDOfPDO, byNodeID);
	psDataFrame->uHead.sStand.bRTR   = true;
	return CANOpen_Write(eChannel, psDataFrame);
}

/**
 * [CANOpenSlave_SendPDO description]
 * @method CANOpenSlave_SendPDO
 * @param  eChannel              [description]
 * @param  byNodeID              [description]
 * @param  eFuncIDOfPDO          [description]
 * @param  wDataLen              [description]
 * @param  pvData                [description]
 * @return                       [description]
 */
E_CANOpenRetCode CANOpenSlave_SendPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO, WORD wDataLen, const void* pvData)
{
	CANOpenDataFrame_S  sDataFrame;
	CANOpenDataFrame_S* psDataFrame;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(0 != byNodeID);
	DEBUG_ASSERT((CANOPENFUNCID_SEND_PDO1 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_SEND_PDO2 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_SEND_PDO3 == eFuncIDOfPDO)
	    || (CANOPENFUNCID_SEND_PDO4 == eFuncIDOfPDO));
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
	return CANOpen_Write(eChannel, psDataFrame);
}
/**
 * [CANOpenSlave_SendSDO description]
 * @method CANOpenSlave_SendSDO
 * @param  eChannel              [description]
 * @param  byNodeID              [description]
 * @param  psSDOMsg              [description]
 * @return                       [description]
 */
E_CANOpenRetCode CANOpenSlave_SendSDO(E_CANChannel eChannel, BYTE byNodeID, const CANOpenCommSDOMsg_S* psSDOMsg)
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
	DEBUG_ASSERT((CANCOMMSDORESPONSE_UPLOAD == psSDOMsg->bSDOAccess)
	    || (CANCOMMSDORESPONSE_DOWNLOAD == psSDOMsg->bSDOAccess));
	switch (psSDOMsg->bSDOAccess)
	{
		case CANCOMMSDORESPONSE_DOWNLOAD: {
			uSDOCmd.sBitField.bCmd = CANOPENSDOSCS_IDD;
			break;
		}
		case CANCOMMSDORESPONSE_UPLOAD: {
			uSDOCmd.sBitField.bE   = 1;
			uSDOCmd.sBitField.bCmd = CANOPENSDOSCS_IDU;
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
	psDataFrame->uHead.sStand.bCOBID      = CANOpen_GetCOBID(CANOPENFUNCID_SEND_SDO, byNodeID);
	psDataFrame->uData.sSDO.bCmdSpecifier = uSDOCmd.tAll;
	psDataFrame->uData.sSDO.bODID_H       = psSDOMsg->tODID >> 8;
	psDataFrame->uData.sSDO.bODID_L       = psSDOMsg->tODID & 0xFF;
	psDataFrame->uData.sSDO.bODSubIndex   = psSDOMsg->bODSubIndex;

	return CANOpen_Write(eChannel, psDataFrame);
}

/**
 * [CANOpenSlave_ResetComm description]
 * @param  psCompSlave [description]
 */
void CANOpenSlave_ResetComm(CommComp_CANOpenSlave* psCompSlave)
{
	DEBUG_ASSERT(NULL != psCompSlave);
	psCompSlave->PRO_bNodeState = CANOPENNODESTATE_BOOTUP;
}

/**
 * [SCANOpenCompSlave_Init description]
 * @method SCANOpenCompSlave_Init
 * @param  psCompSlave            [description]
 */
void SCANOpenCompSlave_Init(CommComp_CANOpenSlave* psCompSlave)
{
	DEBUG_ASSERT(NULL != psCompSlave);
	// psCompSlave->bSlaveFuncEnable	= FALSE;
	psCompSlave->PUB_blClearSize    = FALSE;
	psCompSlave->PUB_blFreeSDO      = FALSE;
	psCompSlave->PRI_fCommRun       = CANOpenSComm_Init;
	psCompSlave->PRO_bNodeID        = 0;
	psCompSlave->PRI_bSDOState      = CANCOMMSDOSTATE_IDLE;
	psCompSlave->PRO_bNodeState     = CANOPENNODESTATE_BOOTUP;
	ResetTimer(&psCompSlave->PRI_lMsgSendTime, 0);
	psCompSlave->PRO_fEventCallBack  = Default_SlaveEventCallBack;
	psCompSlave->PRO_fCustomFunc	= Default_SlaveCustomFunc;
}

/**
 * [SCANOpenCompSlave_InitEventCallBack description]
 * @param psCompSlave       [description]
 * @param fEventCallBack [description]
 */
void SCANOpenCompSlave_InitEventCallBack(CommComp_CANOpenSlave* psCompSlave, CANOpenSlaveEventCallBack_F fEventCallBack)
{
	DEBUG_ASSERT(NULL != psCompSlave);
	psCompSlave->PRO_fEventCallBack = (NULL != fEventCallBack) ? fEventCallBack : Default_SlaveEventCallBack;
}
/**
 * [SCANOpenCompSlave_SetNodeID description]
 * @method SCANOpenCompSlave_SetNodeID
 * @param  psCompSlave                 [description]
 * @param  byNodeID                     [description]
 */
void SCANOpenCompSlave_SetNodeID(CommComp_CANOpenSlave* psCompSlave, BYTE byNodeID)
{
	DEBUG_ASSERT(NULL != psCompSlave);
	if (psCompSlave->PRO_bNodeID != byNodeID) {
		psCompSlave->PRO_bNodeID = byNodeID;
		Error_App_Reset();
		CANOpenSlave_ResetComm(psCompSlave);
	}
}
/**
 * [SCANOpenCompSlave_InitCustomFunc description]
 * @param psCompSlave       [description]
 * @param fSlaveCustomFunc [description]
 */
void SCANOpenCompSlave_InitCustomFunc(CommComp_CANOpenSlave* psCompSlave, CommCustomSlaveFunc_F fSlaveCustomFunc)
{
	DEBUG_ASSERT(NULL != psCompSlave);
	psCompSlave->PRO_fCustomFunc = (NULL != fSlaveCustomFunc) ? fSlaveCustomFunc : Default_SlaveCustomFunc;
}
/**
 * [CANOpenSComm_Parse description]
 * @param eChannel    [description]
 * @param psDataFrame [description]
 */
static void CANOpenSComm_Parse(E_CANChannel eChannel, CANOpenDataFrame_S* psDataFrame)
{
	CommComp_CANOpenSlave* psCompSlave;
	WORD                 wCOBID;

	DEBUG_ASSERT(eChannel < CANCHANNAL_SUM);
	DEBUG_ASSERT(NULL != psDataFrame);
	psCompSlave = g_sCanopenSlaveManager.s_cCommCanopenSlave;
	wCOBID = psDataFrame->uHead.sStand.bCOBID;
	switch (CANOpen_GetFuncIDByCOBID(wCOBID))
	{
		case CANOPENFUNCID_RECV_PDO1:
		case CANOPENFUNCID_RECV_PDO2:
		case CANOPENFUNCID_RECV_PDO3:
		case CANOPENFUNCID_RECV_PDO4:
		case CANOPENFUNCID_RECV_SDO:
			break;
		case CANOPENFUNCID_NMT_ERROR_CTRL:
			break;
		case CANOPENFUNCID_NMT_MODULE_CTRL:
			CANOpenSComm_ParseCANRecvedNMTModuleCtrl(psCompSlave, psDataFrame);
			psCompSlave->PRO_fEventCallBack(psCompSlave, psDataFrame, CANOPENEVENTTYPE_CHANGE_NODESTATE);
			break;
		default:
			break;
	}

	if (FALSE != g_sCanopenSlaveManager.bSlaveFuncEnable) {
		DEBUG_ASSERT(NULL != psCompSlave->PRO_fEventCallBack);
		psCompSlave->PRO_fEventCallBack(psCompSlave, psDataFrame, CANOPENEVENTTYPE_RECVED_DATAFRAME);
	}
}
/**
 * [CANOpenSComm_ParseCANRecvedSDO description]
 * @param  psComp                         [description]
 * @param  psCompSlave                    [description]
 * @param  psDataFrame                    [description]
 */
//inline bool CANOpenSComm_ParseCANRecvedSDO(CommComp_CANOpenSlave* psCompSlave, CANOpenDataFrame_S* psDataFrame)
//{
//	return true;
//}
/**
 * [CANOpenSComm_ParseCANRecvedNMTModuleCtrl description]
 * @param  psCompSlave                          [description]
 * @param  psDataFrame                           [description]
 */
inline void CANOpenSComm_ParseCANRecvedNMTModuleCtrl(CommComp_CANOpenSlave* psCompSlave, CANOpenDataFrame_S* psDataFrame)
{
	BYTE byCtrlCmd;

	DEBUG_ASSERT(NULL != psCompSlave);
	DEBUG_ASSERT(NULL != psDataFrame);
	if(psDataFrame->uData.sU8.bData1 != (BYTE)psCompSlave->PRO_bNodeID)
		return;
	byCtrlCmd = psDataFrame->uData.sU8.bData0 & (~CANOPENNODEGUARDING_TRIGGERBIT);
	switch (byCtrlCmd)
	{
		case CANOPENNMTCTRLCMD_START_REMOTE_NODE:
		{
			psCompSlave->PRO_bNodeState = CANOPENNODESTATE_OPERATIONAL;
			break;
		}
		case CANOPENNMTCTRLCMD_STOP_REMOTE_NODE:
		{
			psCompSlave->PRO_bNodeState = CANOPENNODESTATE_STOPPED;
			break;
		}
		case CANOPENNMTCTRLCMD_ENTER_PREOPERATIONAL_STATE:
		{
			psCompSlave->PRO_bNodeState = CANOPENNODESTATE_PREOPERATIONAL;
			break;
		}
		case CANOPENNMTCTRLCMD_RESET_NODE:
		{
			psCompSlave->PRO_bNodeState = CANOPENNODESTATE_BOOTUP;
			break;
		}
		case CANOPENNMTCTRLCMD_RESET_COMMUNICATION:
		{
			psCompSlave->PRO_bNodeState = CANOPENNODESTATE_BOOTUP;
			break;
		}

		default:
			break;
	}
}
/**
 * [CANOpenSComm_Init description]
 * @method CANOpenSComm_Init
 * @param  psCompSlave            [description]
 */
static void CANOpenSComm_Init(CommComp_CANOpenSlave* psCompSlave)
{
	psCompSlave->PRO_bNodeState = CANOPENNODESTATE_BOOTUP;
}
/**
 * [Default_ConfigFinish description]
 * @param  psCompSlave [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
//static bool Default_ConfigFinish(CommComp_CANOpenSlave* psCompSlave, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
//{
//	return true;
//}
/**
 * [Default_SlaveEventCallBack description]
 * @param psCompSlave [description]
 * @param psDataFrame  [description]
 * @param eEventType   [description]
 */
static void Default_SlaveEventCallBack(CommComp_CANOpenSlave* psCompSlave, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType)
{
}
/**
 * [Default_CustomFunc description]
 * @param psCompSlave [description]
 */
inline void Default_SlaveCustomFunc(CommComp_CANOpenSlave* psCompSlave)
{
}
