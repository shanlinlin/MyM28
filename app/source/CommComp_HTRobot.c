/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : Hankin                                                           |
|  Version  : V1.00                                                            |
|  Creation : 2015/01/14                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#include "CommComp_HTRobot.h"
#include "CommComp_CANOpenMaster.h"
#include "Error_App.h"
//##############################################################################
// definition
//##############################################################################
#define HT_HEARTBEAT_CYCLE 100
#define HT_LOST_MAX 10
#define HT_ERROR_REQ_CYCLE 100
//##############################################################################
// enumeration
//##############################################################################
enum CommComp_Robot__e_card_type
{
	CommComp_Robot__CARD_TYPE_HT,
	CommComp_Robot__CARD_TYPE_SUM
};

enum CommComp_Robot_e_electric_index
{
	CommComp_Robot_HT_INDEX_0,
	CommComp_Robot_HT_INDEX_1,

	CommComp_Robot_HT_INDEX_SUM
};
//##############################################################################
// data type
//##############################################################################
BAUDRATECONF    HTRobotBaudrateConf = {BITTIME_15, BAUDRATE_500KBPS, SAMPLINGPOINT_80PERCENT};//删除28做从站文件时使用这个
// extern	BAUDRATECONF    HTRobotBaudrateConf;

struct CommComp_Robot__s_config_info
{
	enum CommComp_Robot__e_card_type	eCard_type;
	unsigned                            iCard_index;
};

struct CommComp_Robot__s_card_HT
{
	CommComp_S          	_sComp;
	CANOpenCompUnique_S 	_sUnique_CANopen_comp;
	S_CANOpenCHBManager		_sCHBManager;

	S_CANOpenCHBManager		_sPDOManager;
	Ttimer					_sPDOSendTime;

	CommSubComp_S			_sSubcomp;
	CANOpenSubCompUnique_S	_sUnique_CANopen_subcomp;

	unsigned				_iRemote_index;
	unsigned				_iRobot_index;
	unsigned				_iComm_cycle;

	WORD					wOpenendPosition;
	WORD					wMoldPosition;
	WORD					wEjetPosition;
	uOpenmoldState			byOpenmoldState;
	uEjetState				byEjetState;
};
//##############################################################################
// function declaration
//##############################################################################
void        CommComp_Robot__s_card_HT_init(struct CommComp_Robot__s_card_HT* pCard);
bool        CommComp_Robot__s_card_HT_add(struct CommComp_Robot__s_card_HT* pCard, unsigned iRemote_index, unsigned iRobot_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
inline void CommComp_Robot__s_card_HT_del(struct CommComp_Robot__s_card_HT* pCard);
void CANOpenHTRobot_PRIfcommRun_Null(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique);
void CommComp_Robot__s_card_HT_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent);
bool CommComp_Robot__s_card_HT_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
void CommComp_Robot__s_card_HT_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void CommComp_Robot__s_card_HT_CANopen_custom_operation(struct CommComp_S* const psComp, E_CustomFuncType eType);
//##############################################################################
// data declaration
//##############################################################################
//------------------------------------------------------------------------------
struct CommComp_Robot__s_config_info    CommComp_Robot__aConfig_tab[CommComp_Robot_INDEX_SUM];
struct CommComp_Robot__s_card_HT        CommComp_Robot_aCard_HT[CommComp_Robot_HT_INDEX_SUM];
WORD			g_wMBEjetPosition;
WORD			g_wMBMoldPosition;
WORD			g_wMBOpenendPosition;
uOpenmoldState	g_byMBOpenmoldState;
uEjetState		g_byMBEjetState;
//##############################################################################
// function prototype
//##############################################################################
/**
 * [CommComp_Robot_init description]
 */
void CommComp_Robot_init(void)
{
	unsigned i;

#if (DEBUG)
	{
		struct CommComp_Robot__s_config_info sConfig_info;

		sConfig_info.iCard_index = -1;
		debug_assert(sConfig_info.iCard_index >= (sizeof(CommComp_Robot_aCard_hydraulic) / sizeof(*CommComp_Robot_aCard_hydraulic)));
	}
#endif
	for (i = 0; i < (sizeof(CommComp_Robot_aCard_HT) / sizeof(*CommComp_Robot_aCard_HT)); ++i)
	{
		CommComp_Robot__s_card_HT_init(&CommComp_Robot_aCard_HT[i]);
	}

	for (i = 0; i < (sizeof(CommComp_Robot__aConfig_tab) / sizeof(*CommComp_Robot__aConfig_tab)); ++i)
	{
		CommComp_Robot__aConfig_tab[i].eCard_type = CommComp_Robot__CARD_TYPE_SUM;
	}
}

/**
 * [CommComp_Robot_clean description]
 */
void CommComp_Robot_clean(void)
{
	unsigned i;

	for (i = 0; i < (sizeof(CommComp_Robot_aCard_HT) / sizeof(*CommComp_Robot_aCard_HT)); ++i)
	{
		CommComp_Robot__s_card_HT_del(&CommComp_Robot_aCard_HT[i]);
	}
	for (i = 0; i < (sizeof(CommComp_Robot__aConfig_tab) / sizeof(*CommComp_Robot__aConfig_tab)); ++i)
	{
		CommComp_Robot__aConfig_tab[i].eCard_type = CommComp_Robot__CARD_TYPE_SUM;
	}
}


bool CommComp_Robot_add_HT(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	unsigned wHT_sum;
	unsigned i;

	wHT_sum = 0;
	for (i = 0; i < (sizeof(CommComp_Robot__aConfig_tab) / sizeof(*CommComp_Robot__aConfig_tab)); ++i)
	{
		if (CommComp_Robot__CARD_TYPE_SUM == CommComp_Robot__aConfig_tab[i].eCard_type)
		{
			break;
		}
		else if (CommComp_Robot__CARD_TYPE_HT == CommComp_Robot__aConfig_tab[i].eCard_type)
		{
			++wHT_sum;
		}
	}
	DEBUG_ASSERT_WARN(i < (sizeof(CommComp_Robot__aConfig_tab) / sizeof(*CommComp_Robot__aConfig_tab)), "config tab too small.");
	if (i >= (sizeof(CommComp_Robot__aConfig_tab) / sizeof(*CommComp_Robot__aConfig_tab)))
		return false;
	CommComp_Robot__aConfig_tab[i].eCard_type  = CommComp_Robot__CARD_TYPE_HT;
	CommComp_Robot__aConfig_tab[i].iCard_index = wHT_sum;
	DEBUG_ASSERT_ILLEGAL(wHT_sum < (sizeof(CommComp_Robot_aCard_HT) / sizeof(*CommComp_Robot_aCard_HT)), "hydraulic card too small.");
	if (!CommComp_Robot__s_card_HT_add(&CommComp_Robot_aCard_HT[wHT_sum], iRemoteNum, i, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "hydraulic add fail.");
		CommComp_Robot__aConfig_tab[i].eCard_type = CommComp_Robot__CARD_TYPE_SUM;
		return false;
	}
	_ResetBaudrate_eCan(CAN_A, &HTRobotBaudrateConf);
	return true;
}

void CommComp_Robot__s_card_HT_init(struct CommComp_Robot__s_card_HT* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_Init(&pCard->_sComp);
	SCommComp_InitEventCallBack(&pCard->_sComp, CommComp_Robot__s_card_HT_comp_event_callback);

	SCANOpenCompUnique_Init(&pCard->_sUnique_CANopen_comp);
	SCANOpenCompUnique_InitEventCallBack(&pCard->_sUnique_CANopen_comp, CommComp_Robot__s_card_HT_CANopen_event_callback);
	SCANOpenCompUnique_InitConfigInfo(&pCard->_sUnique_CANopen_comp, CommComp_Robot__s_card_HT_CANopen_SDO_config_finish);

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = HT_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = HT_HEARTBEAT_CYCLE;

	CANOpenCHBManager_Init(&pCard->_sPDOManager);
	pCard->_sPDOManager.iLose_max     = 50;
	pCard->_sPDOManager.iConflict_max = 50;
	//pCard->_sPDOManager.iTick         = HT_PDO_CYCLE; // hankin 20171031 mask.
	ResetTimer(&pCard->_sPDOSendTime, 0);

	SCommSubComp_Init(&pCard->_sSubcomp);
	SCANOpenSubCompUnique_Init(&pCard->_sUnique_CANopen_subcomp);

	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iRobot_index = (unsigned)-1;
	pCard->_iComm_cycle      = 0;

	pCard->wOpenendPosition	= 0;
	pCard->wMoldPosition   	= 0;
	pCard->wEjetPosition	= 0;
	pCard->byOpenmoldState.All	= 0;
	pCard->byEjetState.All		= 0;

	g_wMBEjetPosition = 0;
	g_wMBMoldPosition = 0;
	g_wMBOpenendPosition = 0;
	g_byMBOpenmoldState.All = 0;
	g_byMBEjetState.All = 0;

	SCommComp_AddSubComp(&pCard->_sComp, &pCard->_sSubcomp);
}

void CANOpenHTRobot_PRIfcommRun_Null(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique)
{
	return;
}

bool CommComp_Robot__s_card_HT_add(struct CommComp_Robot__s_card_HT* pCard, unsigned iRemote_index, unsigned iRobot_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			SCommComp_SetCommType(&pCard->_sComp, eComm_type);
			SCANOpenCompUnique_SetNodeID(&pCard->_sUnique_CANopen_comp, iNode_ID);
			pCard->_sPDOManager.iTick = iComm_cycle;
			pCard->_iComm_cycle       = iComm_cycle;
			pCard->_iRemote_index     = iRemote_index;
			pCard->_iRobot_index  	  = iRobot_index;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	pCard->_sUnique_CANopen_comp.PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_SUM;
	pCard->_sUnique_CANopen_comp.PRO_bNodeState = CANOPENNODESTATE_OPERATIONAL;
	pCard->_sUnique_CANopen_comp.PRO_fEventCallBack(&pCard->_sUnique_CANopen_comp, NULL, CANOPENEVENTTYPE_CHANGE_NODESTATE);
	pCard->_sUnique_CANopen_comp.PRI_fCommRun = CANOpenHTRobot_PRIfcommRun_Null;
	return true;
}

inline void CommComp_Robot__s_card_HT_del(struct CommComp_Robot__s_card_HT* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_SetCommType(&pCard->_sComp, COMMTYPE_SUM);
	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iRobot_index = (unsigned)-1;
}

void CommComp_Robot__s_card_HT_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent)
{
	struct CommComp_Robot__s_card_HT* pCard;

	DEBUG_ASSERT_PARAM(NULL != pComp, "null pointer");
	DEBUG_ASSERT_PARAM(NULL != pEvent, "null pointer");
	pCard = ContainerOf(pComp, struct CommComp_Robot__s_card_HT, _sComp);
	switch (pEvent->eEvent)
	{
		case COMMEVENT_COMP_CHANGE_COMMTYPE:;
			{
				CommSubComp_S* psSubComp;

				psSubComp = pComp->PRI_psSubCompHead;
				switch (CommComp_GetCommType(pComp))
				{
					case COMMTYPE_CAN1:
					case COMMTYPE_CAN2:
						while (NULL != psSubComp)
						{
							psSubComp = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = &pCard->_sUnique_CANopen_comp;
						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
						break;
					case COMMTYPE_SUM:
						while (NULL != psSubComp)
						{
							psSubComp->PRO_pvUnique = NULL;
							psSubComp               = psSubComp->PRI_psNextSubComp;
						}
						pComp->PRO_pvUnique = NULL;
						SCommComp_InitCustomFunc(pComp, NULL);
						break;
					default:
						DEBUG_ASSERT_WARN(false, "case error");
						break;
				}
			}
			break;
		case COMMEVENT_SUBCOMP_ADD:
		case COMMEVENT_SUBCOMP_DEL:
			break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

bool CommComp_Robot__s_card_HT_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	/*
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 2, HT_HEARTBEAT_CYCLE },
		{ 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 2, 0 },
	};

	DEBUG_ASSERT_PARAM(NULL != psRetSDOMsg, "null pointer");
	if (wConfigIndex < (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
	{
		*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
		switch (psRetSDOMsg->tODID)
		{
			case 0x1800:
				if (0x05 == psRetSDOMsg->bODSubIndex)
				{
					struct CommComp_Robot__s_card_HT* pCard;

					pCard                = ContainerOf(psCompUnique, struct CommComp_Robot__s_card_HT, _sUnique_CANopen_comp);
					psRetSDOMsg->dwValue = pCard->_iComm_cycle;
				}
			default:
				break;
		}
		return false;
	}
	*/
	//海天机械手做从站没有SDO配置过程，直接进入下一步
	return true;
}

void CommComp_Robot__s_card_HT_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_Robot__s_card_HT* pCard;

	DEBUG_ASSERT_PARAM(NULL != pUnique, "null pointer");
	pCard = ContainerOf(pUnique, struct CommComp_Robot__s_card_HT, _sUnique_CANopen_comp);
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO1:
						CANOpenCHBManager_RecvedMsg(&pCard->_sPDOManager);
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
						CANOpenCHBManager_RecvedMsg(&pCard->_sCHBManager);
						break;
					case CANOPENFUNCID_SEND_SDO:
						// if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
						// {
						// 	ODID_T tODID;

						// 	tODID = ((pData_frame->uData.sSDO.bODID_H << 8) | (pData_frame->uData.sSDO.bODID_L));
						// 	switch (tODID)
						// 	{
						// 		case 0x2074:
						// 			if (0x01 == pData_frame->uData.sSDO.bODSubIndex)
						// 			{
						// 				pCard->wError = pData_frame->uData.sSDO.bData & 0xFF;
						// 			}
						// 			break;
						// 		default:
						// 			break;
						// 	}
						// }
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
			WORD wLinkState;

			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
			if (CANOPENNODESTATE_OPERATIONAL == pUnique->PRO_bNodeState)
			{
				wLinkState |= (WORD)1 << pCard->_iRemote_index;
				SCommComp_InitCustomFunc(&pCard->_sComp, CommComp_Robot__s_card_HT_CANopen_custom_operation);
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
			}
			else
			{
				wLinkState &= ~((WORD)1 << pCard->_iRemote_index);
				SCommComp_InitCustomFunc(&pCard->_sComp, NULL);
			}
			DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
			CANOpenCHBManager_Reset(&pCard->_sCHBManager);
			CANOpenCHBManager_Reset(&pCard->_sPDOManager);
		}
		break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

void CommComp_Robot__s_card_HT_CANopen_custom_operation(struct CommComp_S* const pComp, E_CustomFuncType eType)
{
	struct CommComp_Robot__s_card_HT* pCard;
	E_CANChannel                          eChannel;

	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	pCard    = ContainerOf(pComp, struct CommComp_Robot__s_card_HT, _sComp);
	DEBUG_ASSERT_ILLEGAL(CANOPENNODESTATE_OPERATIONAL == pCard->_sUnique_CANopen_comp.PRO_bNodeState, "state error.");
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
			if (CheckTimerOut(&pCard->_sPDOSendTime))
			{
				ResetTimer(&pCard->_sPDOSendTime, pCard->_iComm_cycle);
				pCard->byOpenmoldState = g_byMBOpenmoldState;
				pCard->wOpenendPosition = g_wMBOpenendPosition;
				pCard->byEjetState = g_byMBEjetState;
				pCard->wMoldPosition = g_wMBMoldPosition;
				pCard->wEjetPosition = g_wMBEjetPosition;
				CANOpenDataFrameData_U uData_frame;
				uData_frame.sU8.bData0 = (BYTE)pCard->wMoldPosition;
				uData_frame.sU8.bData1 = (BYTE)pCard->wMoldPosition>>8;
				uData_frame.sU8.bData2 = (BYTE)pCard->wEjetPosition;
				uData_frame.sU8.bData3 = (BYTE)pCard->wEjetPosition>>8;
				uData_frame.sU8.bData4 = (BYTE)pCard->wOpenendPosition;
				uData_frame.sU8.bData5 = (BYTE)pCard->wOpenendPosition>>8;
				uData_frame.sU8.bData6 = pCard->byOpenmoldState.All;
				uData_frame.sU8.bData7 = pCard->byEjetState.All;
				CANOpenMaster_SendPDO(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, 8, &uData_frame);
			}
			break;
		case CUSTOMFUNCTYPE_SLOW:
		{
			E_CANOpenCHBCheck eCheck;

			eCheck = CANOpenCHBManager_Check(&pCard->_sCHBManager);
			if (CANOPENCHBCHECK_IDLE != eCheck)
			{
				switch (eCheck)
				{
					// case CANOPENCHBCHECK_CONFLICT:
					// 	Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_REMOTEIO_NODEIDCONFLICT, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
					// // do case of CANOPENCHBCHECK_OFFLINE.
					// case CANOPENCHBCHECK_OFFLINE:
					// 	CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
					// 	break;
					default:
						CANOpenMaster_SendNodeGruading(eChannel, pCard->_sUnique_CANopen_comp.PRO_bNodeID);
						break;
				}
			}
			eCheck = CANOpenCHBManager_Check(&pCard->_sPDOManager);
			// if (CANOPENCHBCHECK_OFFLINE == eCheck)
			// {
			// 	CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
			// 	break;
			// }
		}
		break;
		default:
			break;
	}
}
