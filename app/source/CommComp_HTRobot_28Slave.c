/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : Shanll                                                           |
|  Version  : V1.00                                                            |
|  Creation : 2021/03/11                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#include "CommComp_HTRobot_28Slave.h"
#include "CommComp_RemoteIO.h"
#include "CommComp_CANOpenMaster.h"
#include "CommComp_CANOpenSlave.h"
#include "Error_App.h"
//##############################################################################
// definition
//##############################################################################
#define HTROBOT_HEARTBEAT_CYCLE 100
#define HTROBOT_LOST_MAX 5
#define	HTROBOT_NMTERRORCTR_BOOTUP_CYCLE	3000
#define	HTROBOT_NMTERRORCTR_OP_CYCLE	100
//##############################################################################
// enumeration
//##############################################################################
//##############################################################################
// data type
//##############################################################################
BAUDRATECONF    HTRobotBaudrateConf = {BITTIME_15, BAUDRATE_500KBPS, SAMPLINGPOINT_80PERCENT};
//##############################################################################
// function declaration
//##############################################################################
void        CommComp_HTRobot_card_init(struct CommComp_HTRobot_card_HT* pCard);
bool        CommComp_HTRobot_card_add(struct CommComp_HTRobot_card_HT* pCard,unsigned iRemoteNum,  E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
inline void CommComp_HTRobot_card_del(struct CommComp_HTRobot_card_HT* pCard);
void		CommComp_HTRobot_card_CANopen_event_callback(CommComp_CANOpenSlave* pSlave, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void		CommComp_HTRobot_card_CANopen_custom_operation(CommComp_CANOpenSlave* psCompSlave);
//##############################################################################
// data declaration
//##############################################################################
//------------------------------------------------------------------------------
CommComp_HTRobot_card_HT	CommComp_HTRobot_aCard_HT;
//##############################################################################
// function prototype
//##############################################################################
/**
 * [CommComp_HTRobot_init description]
 */
void CommComp_HTRobot_init(void)
{
	CommComp_HTRobot_card_init(&CommComp_HTRobot_aCard_HT);
}

/**
 * [CommComp_HTRobot_clean description]
 */
void CommComp_HTRobot_clean(void)
{
	CommComp_HTRobot_card_del(&CommComp_HTRobot_aCard_HT);
}

bool CommComp_HTRobot_add(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	if (!CommComp_HTRobot_card_add(&CommComp_HTRobot_aCard_HT, iRemoteNum, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "htrobot add fail.");
		return false;
	}
	_ResetBaudrate_eCan(CAN_A, &HTRobotBaudrateConf);
	return true;
}

inline void CommComp_HTRobot_card_del(struct CommComp_HTRobot_card_HT* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	pCard->PRI_eCommType = COMMTYPE_SUM;
	g_sCanopenSlaveManager.bSlaveFuncEnable = FALSE;
	// pCard->_iHTRobot_index = (unsigned)-1;
}

void CommComp_HTRobot_card_init(struct CommComp_HTRobot_card_HT* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");

	SCANOpenCompSlave_Init(&pCard->_sCANopen_Slave);
	SCANOpenCompSlave_InitEventCallBack(&pCard->_sCANopen_Slave, CommComp_HTRobot_card_CANopen_event_callback);
	pCard->PRI_eCommType = COMMTYPE_SUM;

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = HTROBOT_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = HTROBOT_HEARTBEAT_CYCLE;

	CANOpenCHBManager_Init(&pCard->_sPDOManager);
	pCard->_sPDOManager.iLose_max     = 50;
	pCard->_sPDOManager.iConflict_max = 50;
	//pCard->_sPDOManager.iTick         = HTROBOT_PDO_CYCLE; // hankin 20171031 mask.
	ResetTimer(&pCard->_sPDOSendTime, 0);
	ResetTimer(&pCard->_sNMTErrSendTime, 0);
	// pCard->_iHTRobot_index = (unsigned)-1;
	pCard->_iRemote_index	= 0;
	pCard->_iComm_cycle     = 0;
	pCard->wOpenendPosition	= 0;
	pCard->wMoldPosition   	= 0;
	pCard->wEjetPosition	= 0;
	pCard->byOpenmoldState.All	= 0;
	pCard->byEjetState.All		= 0;
	SCANOpenCompSlave_InitCustomFunc(&pCard->_sCANopen_Slave,CommComp_HTRobot_card_CANopen_custom_operation);
}

bool CommComp_HTRobot_card_add(struct CommComp_HTRobot_card_HT* pCard,unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			g_sCanopenSlaveManager.s_cCommCanopenSlave = &pCard->_sCANopen_Slave;
			pCard->PRI_eCommType = eComm_type;
			if (pCard->_sCANopen_Slave.PRO_bNodeID != iNode_ID)
				pCard->_sCANopen_Slave.PRO_bNodeID = iNode_ID;
            Error_App_Reset();
			pCard->_iRemote_index 	  = iRemoteNum;
			pCard->_sPDOManager.iTick = iComm_cycle;
			pCard->_iComm_cycle       = iComm_cycle;
			g_sCanopenSlaveManager.bSlaveFuncEnable = TRUE;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	return true;
}

void CommComp_HTRobot_card_CANopen_event_callback(CommComp_CANOpenSlave* pSlave, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_HTRobot_card_HT* pCard;

	DEBUG_ASSERT_PARAM(NULL != pSlave, "null pointer");
	pCard = &CommComp_HTRobot_aCard_HT;
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_RECV_PDO1:
						if(CANOpen_GetNodeIDByCOBID(pData_frame->uHead.sStand.bCOBID) != ((NodeID_T)pCard->_sCANopen_Slave.PRO_bNodeID))
							return;
						CANOpenCHBManager_RecvedMsg(&pCard->_sPDOManager);
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
						CANOpenCHBManager_RecvedMsg(&pCard->_sCHBManager);
						break;
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
			{
				WORD          wLinkState;

				wLinkState   = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
				CANOpenCHBManager_Reset(&pCard->_sCHBManager);
				CANOpenCHBManager_Reset(&pCard->_sPDOManager);
                if (CANOPENNODESTATE_OPERATIONAL == pCard->_sCANopen_Slave.PRO_bNodeState)
                {
                    wLinkState |= (WORD)1 << pCard->_iRemote_index;
					ResetTimer(&pCard->_sNMTErrSendTime, HTROBOT_NMTERRORCTR_OP_CYCLE);
				}
				else
					wLinkState &= ~((WORD)1 << pCard->_iRemote_index);
				DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
			}
			break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

void CommComp_HTRobot_card_CANopen_custom_operation(CommComp_CANOpenSlave* psCompSlave)
{
	CommComp_HTRobot_card_HT* pCard;
	E_CANChannel    eChannel;
	WORD          wLinkState;

	wLinkState   = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
	pCard = &CommComp_HTRobot_aCard_HT;
	eChannel = CANOpenMaster_CommTypeToChannel(pCard->PRI_eCommType);
	switch (pCard->_sCANopen_Slave.PRO_bNodeState)
	{
		case CANOPENNODESTATE_OPERATIONAL:
			{
				E_CANOpenCHBCheck eCheck;

				eCheck = CANOpenCHBManager_Check(&pCard->_sCHBManager);
				if (CANOPENCHBCHECK_IDLE != eCheck)
				{
					if (CANOPENCHBCHECK_CONFLICT == eCheck)
					{
						CANOpenSlave_ResetComm(&pCard->_sCANopen_Slave);
						Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_REMOTEIO_NODEIDCONFLICT, pCard->_sCANopen_Slave.PRO_bNodeID);
					}
				}

				eCheck = CANOpenCHBManager_Check(&pCard->_sPDOManager);
				if (CANOPENCHBCHECK_OFFLINE == eCheck)
				{
					wLinkState &= ~((WORD)1 << pCard->_iRemote_index);
					DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
					CANOpenSlave_ResetComm(&pCard->_sCANopen_Slave);
				}

				if (CheckTimerOut(&pCard->_sPDOSendTime))
				{
					ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
					CANOpenDataFrameData_U uData_frame;
					uData_frame.sU8.bData0 = pCard->byOpenmoldState.All;
					uData_frame.sU8.bData1 = (BYTE)pCard->wOpenendPosition;
					uData_frame.sU8.bData2 = (BYTE)pCard->wOpenendPosition>>8;
					uData_frame.sU8.bData3 = pCard->byEjetState.All;
					uData_frame.sU8.bData4 = (BYTE)pCard->wMoldPosition;
					uData_frame.sU8.bData5 = (BYTE)pCard->wMoldPosition>>8;
					uData_frame.sU8.bData6 = (BYTE)pCard->wEjetPosition;
					uData_frame.sU8.bData7 = (BYTE)pCard->wEjetPosition>>8;
					CANOpenSlave_SendPDO(eChannel, pCard->_sCANopen_Slave.PRO_bNodeID, CANOPENFUNCID_SEND_PDO1, 8, &uData_frame);
				}
				else if(CheckTimerOut(&pCard->_sNMTErrSendTime))
				{
					ResetTimer(&pCard->_sNMTErrSendTime, HTROBOT_NMTERRORCTR_OP_CYCLE);
					CANOpenSlave_SendNMTErrCtrl(eChannel, pCard->_sCANopen_Slave.PRO_bNodeID+1, CANOPENFUNCID_NMT_ERROR_CTRL, CANOPENNODESTATE_OPERATIONAL);
				}
			}
			break;
		case CANOPENNODESTATE_BOOTUP:
			{
				if (CheckTimerOut(&pCard->_sNMTErrSendTime))
				{
					ResetTimer(&pCard->_sNMTErrSendTime, HTROBOT_NMTERRORCTR_BOOTUP_CYCLE);
					CANOpenSlave_SendNMTErrCtrl(eChannel, pCard->_sCANopen_Slave.PRO_bNodeID+1, CANOPENFUNCID_NMT_ERROR_CTRL, CANOPENNODESTATE_BOOTUP);
				}
			}
			break;
		default:
			break;
	}
}

