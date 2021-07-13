/*
 * CommComp_Driver.c
 *
 *  Created on: 2020年3月11日
 *  Author: SHANLL
 *	Description:汇川驱动器采用私有协议，整个过程没有NMT、SDO，上电就是PDO收发，且采用一问一答模式
 *
 */
#include "CommComp_Driver.h"
#include "CommComp_CANOpenMaster.h"
#include "Error_App.h"
#include "common.h"
//##############################################################################
// definition
//##############################################################################
#define INOVANCE_HEARTBEAT_CYCLE 	100
#define INOVANCE_SENDAGAIN_CYCLE 	INOVANCE_HEARTBEAT_CYCLE/4
#define INOVANCE_RESTART_CYCLE	 	5000
#define INOVANCE_LOST_MAX 			5

#define XINGTAI_HEARTBEAT_CYCLE 	2000
#define XINGTAI_SENDAGAIN_CYCLE 	XINGTAI_HEARTBEAT_CYCLE/200
#define XINGTAI_RESTART_CYCLE	 	5000
#define XINGTAI_LOST_MAX 			5
#define XINGTAI_SENDPF_CYCLE	 	1
//28给驱动器下发命令码
#define XINGTAI_SENDREADPARACMD		0x3
#define XINGTAI_SENDWRITEPARACMD	0x6
// #define XINGTAI_SENDPRESSCMD		0x21//shanll 20210121
// #define XINGTAI_SENDFLOWCMD		0x22//shanll 20210121
#define XINGTAI_SENDPFCMD			0x21//shanll 20210121
#define XINGTAI_SENDCTRLCMD			0x32
//驱动器主动上传给28的命令码
#define XINGTAI_UPLOADPARACMD		0x3
#define XINGTAI_UPLOADSTATECMD		0x33
//伺服状态报警bit
#define XINGTAI_STATEERRBIT			0x80
//##############################################################################
// data type
//##############################################################################
struct CommComp_driver__s_config_info
{
	enum CommComp_driver_type eCard_type;
	unsigned                             iCard_index;
};

struct CommComp_driver__s_card_inovance
{
	CommComp_S          _sComp;
	CANOpenCompUnique_S _sUnique_CANopen_comp;
	S_CANOpenCHBManager _sCHBManager;

	S_CANOpenCHBManager _sPDOManager;
	Ttimer              _sPDOSendTime;

	CommSubComp_S          _sSubcomp;
	CANOpenSubCompUnique_S _sUnique_CANopen_subcomp;

	unsigned _iRemote_index;
	unsigned _iDriver_index;
	unsigned _iComm_cycle;

	// (slave) send PDO
	WORD wTargetID;
	WORD wSoureID;
	WORD wCommondCode;
	WORD wData[2];
};

struct CommComp_driver__s_card_xingtai	//shanll 20200925 add
{
	CommComp_S          _sComp;
	CANOpenCompUnique_S _sUnique_CANopen_comp;
	S_CANOpenCHBManager _sCHBManager;

	S_CANOpenCHBManager _sPDOManager;
	Ttimer              _sPDOSendTime;

	CommSubComp_S          _sSubcomp;
	CANOpenSubCompUnique_S _sUnique_CANopen_subcomp;

	unsigned _iRemote_index;
	unsigned _iDriver_index;
	unsigned _iComm_cycle;

	// (slave) send PDO
	WORD wTargetID;
	WORD wSoureID;
	WORD wCommondCode;
	WORD wData[2];
};

BAUDRATECONF InovanceBaudrateConf;
HT_MB_DRIVER_INOVANCE g_HT_MB_Driver_Inovance;
HT_91_DRIVER_INOVANCE g_HT_91_Driver_Inovance;

BAUDRATECONF 		XingtaiBaudrateConf;
XT_MB_DRIVER 		g_XT_MB_Driver;
XT_91_DRIVER  		g_XT_91_Driver;
XT_SENDENABLE		g_XT_SendEnable;
WORD	Press_Xingtai = 0;
WORD	Flow_Xingtai = 0;
WORD	wSend_Press_Xingtai = 0;
WORD	wSend_Flow_Xingtai = 0;
BOOL	bSend_InjState_Xingtai = FALSE;
//##############################################################################
// function declaration
//##############################################################################
void        CommComp_driver__s_card_inovance_init(struct CommComp_driver__s_card_inovance* pCard);
bool        CommComp_driver__s_card_inovance_add(struct CommComp_driver__s_card_inovance* pCard, unsigned iRemote_index, unsigned iDriver_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
inline void CommComp_driver__s_card_inovance_del(struct CommComp_driver__s_card_inovance* pCard);
void 		CommComp_driver__s_card_inovance_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent);
bool 		CommComp_driver__s_card_inovance_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
void 		CommComp_driver__s_card_inovance_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void 		CommComp_driver__s_card_inovance_CANopen_custom_operation(struct CommComp_S* const psComp, E_CustomFuncType eType);
void 		CANOpenInovance_Null(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique);
void		CommComp_Driver_Inovance_SendPDO(struct CommComp_S* const pComp);

//shanll 20200925 add xingtai driver
void        CommComp_driver__s_card_xingtai_init(struct CommComp_driver__s_card_xingtai* pCard);
bool        CommComp_driver__s_card_xingtai_add(struct CommComp_driver__s_card_xingtai* pCard, unsigned iRemote_index, unsigned iDriver_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
inline void CommComp_driver__s_card_xingtai_del(struct CommComp_driver__s_card_xingtai* pCard);
void 		CommComp_driver__s_card_xingtai_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent);
bool 		CommComp_driver__s_card_xingtai_CANopen_SDO_config_finish(struct CANOpenCompUnique_S* psCompUnique, struct CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
void 		CommComp_driver__s_card_xingtai_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type);
void 		CommComp_driver__s_card_xingtai_CANopen_custom_operation(struct CommComp_S* const psComp, E_CustomFuncType eType);
void 		CANOpenXingtai_Null(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique);
void		CommComp_Driver_Xingtai_SendPDO(struct CommComp_S* const pComp);
void		CommComp_Driver_Xingtai_SendControl(struct CommComp_S* const pComp);
void		CommComp_Driver_Xingtai_SendPF(struct CommComp_S* const pComp);
// void		CommComp_Driver_Xingtai_SendPress(struct CommComp_S* const pComp);
// void		CommComp_Driver_Xingtai_SendFlow(struct CommComp_S* const pComp);
//##############################################################################
// data declaration
//##############################################################################
// CommOffsetSeg_S const CommComp_driver__aCard_read_index_seg[] = {
//    { DBINDEX_POWERCARD0_CONFIG_CARD_ID,
//    DBINDEX_POWERCARD0_INTERFACE_IRMS_RESET },
//};
// CommOffsetSeg_S const CommComp_driver__aCard_write_index_seg[] = {
//    { DBINDEX_POWERCARD0_INTERFACE_HARDWARE_PN,
//    DBINDEX_POWERCARD0_INTERFACE_SOFTWARE_PN },
//};
//------------------------------------------------------------------------------
struct CommComp_driver__s_config_info    CommComp_driver__aConfig_tab[CommComp_driver_INDEX_SUM];
struct CommComp_driver__s_card_inovance CommComp_driver_aCard_inovance[CommComp_driver_INOVANCE_INDEX_SUM];
struct CommComp_driver__s_card_xingtai CommComp_driver_aCard_xingtai[CommComp_driver_XINGTAI_INDEX_SUM];

//##############################################################################
// function prototype
//##############################################################################
/**
 * [CommComp_driver_init description]
 */
void CommComp_driver_init(void)
{
	unsigned i;

#if (DEBUG)
	{
		struct CommComp_driver__s_config_info sConfig_info;

		sConfig_info.iCard_index = -1;
		debug_assert(sConfig_info.iCard_index >= (sizeof(CommComp_driver_aCard_inovance) / sizeof(*CommComp_driver_aCard_inovance)));
	}
#endif
	for (i = 0; i < (sizeof(CommComp_driver_aCard_inovance) / sizeof(*CommComp_driver_aCard_inovance)); ++i)
	{
		CommComp_driver__s_card_inovance_init(&CommComp_driver_aCard_inovance[i]);
	}

	for (i = 0; i < (sizeof(CommComp_driver_aCard_xingtai) / sizeof(*CommComp_driver_aCard_xingtai)); ++i)
	{
		CommComp_driver__s_card_xingtai_init(&CommComp_driver_aCard_xingtai[i]);
	}

	for (i = 0; i < (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)); ++i)
	{
		CommComp_driver__aConfig_tab[i].eCard_type = CommComp_driver__CARD_TYPE_SUM;
	}
	//DEBUG_INIT_CHECK(struct CommComp_driver__s_card_inovance, CommComp_driver__s_card_inovance_init);
	//DEBUG_INIT_CHECK(struct CommComp_driver__s_card_electric, CommComp_driver__s_card_electric_init);
}

/**
 * [CommComp_driver_clean description]
 */
void CommComp_driver_clean(void)
{
	unsigned i;

	for (i = 0; i < (sizeof(CommComp_driver_aCard_inovance) / sizeof(*CommComp_driver_aCard_inovance)); ++i)
	{
		CommComp_driver__s_card_inovance_del(&CommComp_driver_aCard_inovance[i]);
	}

	for (i = 0; i < (sizeof(CommComp_driver_aCard_xingtai) / sizeof(*CommComp_driver_aCard_xingtai)); ++i)
	{
		CommComp_driver__s_card_xingtai_del(&CommComp_driver_aCard_xingtai[i]);
	}
	for (i = 0; i < (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)); ++i)
	{
		CommComp_driver__aConfig_tab[i].eCard_type = CommComp_driver__CARD_TYPE_SUM;
	}
}

/*
 * [CommComp_driver_add_inovance description]
 * @param  eComm_type  [description]
 * @param  iNode_ID    [description]
 * @param  iComm_cycle [description]
 * @return             [description]
 */
bool CommComp_driver_add_inovance(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	unsigned iInovance_sum;
	unsigned i;

	iInovance_sum = 0;
	for (i = 0; i < (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)); ++i)
	{
		if (CommComp_driver__CARD_TYPE_SUM == CommComp_driver__aConfig_tab[i].eCard_type)
		{
			break;
		}
		else if (CommComp_driver__CARD_TYPE_INOVANCE == CommComp_driver__aConfig_tab[i].eCard_type)
		{
			++iInovance_sum;
		}
	}
	DEBUG_ASSERT_WARN(i < (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)), "config tab too small.");
	if (i >= (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)))
		return false;
	CommComp_driver__aConfig_tab[i].eCard_type  = CommComp_driver__CARD_TYPE_INOVANCE;
	CommComp_driver__aConfig_tab[i].iCard_index = iInovance_sum;
	DEBUG_ASSERT_ILLEGAL(iInovance_sum < (sizeof(CommComp_driver_aCard_inovance) / sizeof(*CommComp_driver_aCard_inovance)), "inovance card too small.");
	if (!CommComp_driver__s_card_inovance_add(&CommComp_driver_aCard_inovance[iInovance_sum], iRemoteNum, i, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "inovance add fail.");
		CommComp_driver__aConfig_tab[i].eCard_type = CommComp_driver__CARD_TYPE_SUM;
		return false;
	}
	if(0 == i)
	{
		_ResetBaudrate_eCan(CAN_A, &InovanceBaudrateConf);
	}
	return true;
}

/*
 * [CommComp_driver_add_xingtai description]
 * @param  eComm_type  [description]
 * @param  iNode_ID    [description]
 * @param  iComm_cycle [description]
 * @return             [description]
 */
bool CommComp_driver_add_xingtai(unsigned iRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	unsigned iXingtai_sum;
	unsigned i;

	iXingtai_sum = 0;
	for (i = 0; i < (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)); ++i)
	{
		if (CommComp_driver__CARD_TYPE_SUM == CommComp_driver__aConfig_tab[i].eCard_type)
		{
			break;
		}
		else if (CommComp_driver__CARD_TYPE_XINGTAI == CommComp_driver__aConfig_tab[i].eCard_type)
		{
			++iXingtai_sum;
		}
	}
	DEBUG_ASSERT_WARN(i < (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)), "config tab too small.");
	if (i >= (sizeof(CommComp_driver__aConfig_tab) / sizeof(*CommComp_driver__aConfig_tab)))
		return false;
	CommComp_driver__aConfig_tab[i].eCard_type  = CommComp_driver__CARD_TYPE_XINGTAI;
	CommComp_driver__aConfig_tab[i].iCard_index = iXingtai_sum;
	DEBUG_ASSERT_ILLEGAL(iXingtai_sum < (sizeof(CommComp_driver_aCard_xingtai) / sizeof(*CommComp_driver_aCard_xingtai)), "xingtai card too small.");
	if (!CommComp_driver__s_card_xingtai_add(&CommComp_driver_aCard_xingtai[iXingtai_sum], iRemoteNum, i, eComm_type, iNode_ID, iComm_cycle))
	{
		DEBUG_ASSERT_WARN(false, "xingtai add fail.");
		CommComp_driver__aConfig_tab[i].eCard_type = CommComp_driver__CARD_TYPE_SUM;
		return false;
	}
	if(0 == i)
	{
		_ResetBaudrate_eCan(CAN_A, &XingtaiBaudrateConf);
	}
	return true;
}

void CANOpenInovance_Null(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique)
{
	return;
}

void CANOpenXingtai_Null(CommComp_S* psComp, struct CANOpenCompUnique_S* psUnique)
{
	return;
}

void CommComp_driver__s_card_inovance_init(struct CommComp_driver__s_card_inovance* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_Init(&pCard->_sComp);
	SCommComp_InitEventCallBack(&pCard->_sComp, CommComp_driver__s_card_inovance_comp_event_callback);

	SCANOpenCompUnique_Init(&pCard->_sUnique_CANopen_comp);
	//汇川驱动器没有配置过程，没有NMT等
	pCard->_sUnique_CANopen_comp.PRO_bNodeState     = CANOPENNODESTATE_OPERATIONAL;
	pCard->_sUnique_CANopen_comp.PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_MAX+1;
	pCard->_sUnique_CANopen_comp.PRI_fCommRun = CANOpenInovance_Null;
	SCANOpenCompUnique_InitEventCallBack(&pCard->_sUnique_CANopen_comp, CommComp_driver__s_card_inovance_CANopen_event_callback);
//	SCANOpenCompUnique_InitConfigInfo(&pCard->_sUnique_CANopen_comp, CommComp_driver__s_card_inovance_CANopen_SDO_config_finish);

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = INOVANCE_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = INOVANCE_HEARTBEAT_CYCLE;

	CANOpenCHBManager_Init(&pCard->_sPDOManager);
	pCard->_sPDOManager.iLose_max     = 50;
	pCard->_sPDOManager.iConflict_max = 50;
	//pCard->_sPDOManager.iTick         = INOVANCE_PDO_CYCLE; // hankin 20171031 mask.
	ResetTimer(&pCard->_sPDOSendTime, 0);

	SCommSubComp_Init(&pCard->_sSubcomp);
	SCANOpenSubCompUnique_Init(&pCard->_sUnique_CANopen_subcomp);

	InovanceBaudrateConf.wBitTime       = BITTIME_15;
	InovanceBaudrateConf.wBaudrate      = BAUDRATE_500KBPS;
	InovanceBaudrateConf.wSamplingPoint = SAMPLINGPOINT_80PERCENT;

	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iDriver_index = (unsigned)-1;
	pCard->_iComm_cycle      = 0;

	// (auto)
	// pCard->i200000 = 0;
	// pCard->f200100 = 0;
	// pCard->i200200 = 0;

	pCard->wTargetID = 0;
	pCard->wSoureID = 0;
	pCard->wCommondCode = 0;
	pCard->wData[0] = 0;
	pCard->wData[1] = 0;

	SCommComp_AddSubComp(&pCard->_sComp, &pCard->_sSubcomp);
//	for (i = 0; i < (sizeof(g_HT_MB_Driver_Inovance) / sizeof(HT_MB_DRIVER_INOVANCE)); ++i)
//	{
//		memset(&g_HT_MB_Driver_Inovance[i],0,sizeof(g_HT_MB_Driver_Inovance));
//	}
	memset(&g_HT_MB_Driver_Inovance,0,sizeof(g_HT_MB_Driver_Inovance));
	memset(&g_HT_91_Driver_Inovance,0,sizeof(g_HT_91_Driver_Inovance));
}

void CommComp_driver__s_card_xingtai_init(struct CommComp_driver__s_card_xingtai* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_Init(&pCard->_sComp);
	SCommComp_InitEventCallBack(&pCard->_sComp, CommComp_driver__s_card_xingtai_comp_event_callback);

	SCANOpenCompUnique_Init(&pCard->_sUnique_CANopen_comp);
	//兴泰驱动器没有配置过程，没有NMT等
	pCard->_sUnique_CANopen_comp.PRO_bNodeState     = CANOPENNODESTATE_OPERATIONAL;
	pCard->_sUnique_CANopen_comp.PRO_eCtrlCmd = CANOPENMASTERCTRLCMD_MAX+1;
	pCard->_sUnique_CANopen_comp.PRI_fCommRun = CANOpenXingtai_Null;
	SCANOpenCompUnique_InitEventCallBack(&pCard->_sUnique_CANopen_comp, CommComp_driver__s_card_xingtai_CANopen_event_callback);
//	SCANOpenCompUnique_InitConfigInfo(&pCard->_sUnique_CANopen_comp, CommComp_driver__s_card_xingtai_CANopen_SDO_config_finish);

	CANOpenCHBManager_Init(&pCard->_sCHBManager);
	pCard->_sCHBManager.iLose_max     = XINGTAI_LOST_MAX;
	pCard->_sCHBManager.iConflict_max = 5;
	pCard->_sCHBManager.iTick         = XINGTAI_HEARTBEAT_CYCLE;

	CANOpenCHBManager_Init(&pCard->_sPDOManager);
	pCard->_sPDOManager.iLose_max     = 50;
	pCard->_sPDOManager.iConflict_max = 50;
	//pCard->_sPDOManager.iTick         = XINGTAI_PDO_CYCLE; // hankin 20171031 mask.
	ResetTimer(&pCard->_sPDOSendTime, 0);

	SCommSubComp_Init(&pCard->_sSubcomp);
	SCANOpenSubCompUnique_Init(&pCard->_sUnique_CANopen_subcomp);

	XingtaiBaudrateConf.wBitTime       = BITTIME_15;
	XingtaiBaudrateConf.wBaudrate      = BAUDRATE_500KBPS;
	XingtaiBaudrateConf.wSamplingPoint = SAMPLINGPOINT_80PERCENT;

	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iDriver_index = (unsigned)-1;
	pCard->_iComm_cycle      = 0;

	// (auto)
	// pCard->i200000 = 0;
	// pCard->f200100 = 0;
	// pCard->i200200 = 0;

	pCard->wTargetID = 0;
	pCard->wSoureID = 0;
	pCard->wCommondCode = 0;
	pCard->wData[0] = 0;
	pCard->wData[1] = 0;

	SCommComp_AddSubComp(&pCard->_sComp, &pCard->_sSubcomp);
//	for (i = 0; i < (sizeof(g_XT_MB_Driver) / sizeof(XT_MB_DRIVER)); ++i)
//	{
//		memset(&g_XT_MB_Driver[i],0,sizeof(g_XT_MB_Driver));
//	}
	memset(&g_XT_MB_Driver,0,sizeof(g_XT_MB_Driver));
	memset(&g_XT_91_Driver,0,sizeof(g_XT_91_Driver));
	memset(&g_XT_SendEnable,0,sizeof(g_XT_SendEnable));
	ResetTimer(&g_XT_MB_Driver.wSendTime,0);
	ResetTimer(&g_XT_MB_Driver.wSendAgain,0);
	ResetTimer(&g_XT_MB_Driver.wSendPFCycle,0);
	ResetTimer(&g_XT_MB_Driver.wOfflineRestart,0);
	g_XT_SendEnable.bControlSendEnable = 2;
}

/*
 * [CommComp_driver__s_card_inovance_add description]
 * @param  pCard            [description]
 * @param  iRemote_index    [description]
 * @param  iDriver_index [description]
 * @param  eComm_type       [description]
 * @param  iNode_ID         [description]
 * @param  iComm_cycle      [description]
 * @return                  [description]
 */
bool CommComp_driver__s_card_inovance_add(struct CommComp_driver__s_card_inovance* pCard, unsigned iRemote_index, unsigned iDriver_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	WORD wLinkState;
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			SCommComp_SetCommType(&pCard->_sComp, eComm_type);
//			SCANOpenCompUnique_SetNodeID(&pCard->_sUnique_CANopen_comp, iNode_ID);
			//SHANLL_20200326 	去掉SCANOpenCompUnique_SetNodeID中CANOpenMaster_ResetComm(psCompUnique)
			DEBUG_ASSERT(NULL != pCard->psCompUnique);
			if (pCard->_sUnique_CANopen_comp.PRO_bNodeID != iNode_ID) {
				pCard->_sUnique_CANopen_comp.PRO_bNodeID = iNode_ID;
				Error_App_Reset();
			}
			//shanll 20200410汇川驱动器私有协议，驱动器发给28时，帧ID的低7位并不是自身的NodeID，而是目标28的NodeID
			//所以这里把NodeID赋值0x9，是为了28匹配收到驱动器的SendPDO，且不影响28发给驱动器的RPDO这样就不用去CommComp_CANOpenMaster.c增加特殊处理
			pCard->_sUnique_CANopen_comp.PRO_bNodeID = 0x9;

			pCard->_sPDOManager.iTick = iComm_cycle;
			pCard->_iComm_cycle       = iComm_cycle;
			pCard->_iRemote_index     = iRemote_index;
			pCard->_iDriver_index  = iDriver_index;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);

	//汇川驱动器直接进入PDO发送和接收
	wLinkState |= (WORD)1 << pCard->_iRemote_index;
	SCommComp_InitCustomFunc(&pCard->_sComp, CommComp_driver__s_card_inovance_CANopen_custom_operation);

	DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
//	CANOpenCHBManager_Reset(&pCard->_sCHBManager);
//	CANOpenCHBManager_Reset(&pCard->_sPDOManager);
	return true;
}

/*
 * [CommComp_driver__s_card_xingtai_add description]
 * @param  pCard            [description]
 * @param  iRemote_index    [description]
 * @param  iDriver_index 	[description]
 * @param  eComm_type       [description]
 * @param  iNode_ID         [description]
 * @param  iComm_cycle      [description]
 * @return                  [description]
 */
bool CommComp_driver__s_card_xingtai_add(struct CommComp_driver__s_card_xingtai* pCard, unsigned iRemote_index, unsigned iDriver_index, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle)
{
	WORD wLinkState;
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	switch (eComm_type)
	{
		case COMMTYPE_CAN1:
		case COMMTYPE_CAN2:
			SCommComp_SetCommType(&pCard->_sComp, eComm_type);
//			SCANOpenCompUnique_SetNodeID(&pCard->_sUnique_CANopen_comp, iNode_ID);
			//SHANLL_20200326 	去掉SCANOpenCompUnique_SetNodeID中CANOpenMaster_ResetComm(psCompUnique)
			DEBUG_ASSERT(NULL != pCard->psCompUnique);
			if (pCard->_sUnique_CANopen_comp.PRO_bNodeID != iNode_ID) {
				pCard->_sUnique_CANopen_comp.PRO_bNodeID = iNode_ID;
				Error_App_Reset();
			}
			//shanll 20200410兴泰驱动器私有协议，驱动器发给28时，帧ID的低7位并不是自身的NodeID，而是目标28的NodeID
			//所以这里把NodeID赋值0x9，是为了28匹配收到驱动器的SendPDO，且不影响28发给驱动器的RPDO这样就不用去CommComp_CANOpenMaster.c增加特殊处理
			pCard->_sUnique_CANopen_comp.PRO_bNodeID = 0x9;

			pCard->_sPDOManager.iTick = iComm_cycle;
			pCard->_iComm_cycle       = iComm_cycle;
			pCard->_iRemote_index     = iRemote_index;
			pCard->_iDriver_index  = iDriver_index;
			break;
		default:
			DEBUG_ASSERT_WARN(true, "case error");
			return false;
	}
	wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);

	//兴泰驱动器直接进入PDO发送和接收
	wLinkState |= (WORD)1 << pCard->_iRemote_index;
	SCommComp_InitCustomFunc(&pCard->_sComp, CommComp_driver__s_card_xingtai_CANopen_custom_operation);

	DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
//	CANOpenCHBManager_Reset(&pCard->_sCHBManager);
//	CANOpenCHBManager_Reset(&pCard->_sPDOManager);
	return true;
}

/**
 * [CommComp_driver__s_card_inovance_del description]
 * @param pCard [description]
 */
inline void CommComp_driver__s_card_inovance_del(struct CommComp_driver__s_card_inovance* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_SetCommType(&pCard->_sComp, COMMTYPE_SUM);
	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iDriver_index = (unsigned)-1;
}

/**
 * [CommComp_driver__s_card_xingtai_del description]
 * @param pCard [description]
 */
inline void CommComp_driver__s_card_xingtai_del(struct CommComp_driver__s_card_xingtai* pCard)
{
	DEBUG_ASSERT_PARAM(NULL != pCard, "null pointer");
	SCommComp_SetCommType(&pCard->_sComp, COMMTYPE_SUM);
	pCard->_iRemote_index    = (unsigned)-1;
	pCard->_iDriver_index = (unsigned)-1;
}

/*
 * [CommComp_driver__s_card_inovance_comp_event_callback description]
 * @param pComp  [description]
 * @param pEvent [description]
 */
void CommComp_driver__s_card_inovance_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent)
{
	struct CommComp_driver__s_card_inovance* pCard;

	DEBUG_ASSERT_PARAM(NULL != pComp, "null pointer");
	DEBUG_ASSERT_PARAM(NULL != pEvent, "null pointer");
	pCard = ContainerOf(pComp, struct CommComp_driver__s_card_inovance, _sComp);
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
//						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
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

/*
 * [CommComp_driver__s_card_xingtai_comp_event_callback description]
 * @param pComp  [description]
 * @param pEvent [description]
 */
void CommComp_driver__s_card_xingtai_comp_event_callback(CommComp_S* pComp, CommEvent_U* pEvent)
{
	struct CommComp_driver__s_card_xingtai* pCard;

	DEBUG_ASSERT_PARAM(NULL != pComp, "null pointer");
	DEBUG_ASSERT_PARAM(NULL != pEvent, "null pointer");
	pCard = ContainerOf(pComp, struct CommComp_driver__s_card_xingtai, _sComp);
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
//						CANOpenMaster_ResetComm(&pCard->_sUnique_CANopen_comp);
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

/*
 * [CommComp_driver__s_card_inovance_CANopen_event_callback description]
 * @param pUnique     [description]
 * @param pData_frame [description]
 * @param eEvent_type [description]
 */
void CommComp_driver__s_card_inovance_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_driver__s_card_inovance* pCard;
	WORD	wLinkState;

	DEBUG_ASSERT_PARAM(NULL != pUnique, "null pointer");
	pCard = ContainerOf(pUnique, struct CommComp_driver__s_card_inovance, _sUnique_CANopen_comp);
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO3:
						pCard->wSoureID 	= (WORD)pData_frame->uData.sU8.bData0;
						pCard->wCommondCode = ((WORD)pData_frame->uData.sU8.bData2<<8)+(WORD)pData_frame->uData.sU8.bData3;
						pCard->wData[0] 	= ((WORD)pData_frame->uData.sU8.bData4<<8)+(WORD)pData_frame->uData.sU8.bData5;
						pCard->wData[1] 	= ((WORD)pData_frame->uData.sU8.bData6<<8)+(WORD)pData_frame->uData.sU8.bData7;
						memcpy(&g_HT_91_Driver_Inovance.wSourceID,&pCard->wSoureID,4*sizeof(WORD));

						if(g_HT_MB_Driver_Inovance.bFlagRestart == TRUE)
						{
							wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
							wLinkState |= (WORD)1 << g_HT_MB_Driver_Inovance.wInovanceIndex;
							DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
							g_HT_MB_Driver_Inovance.bFlagRestart = FALSE;
						}
						//校验
//						if(g_HT_91_Driver_Inovance.wSourceID == (g_HT_MB_Driver_Inovance.wTargetID&0x7F))
							g_HT_91_Driver_Inovance.wSourceID = g_HT_MB_Driver_Inovance.wTargetID;
						if((0x8001 == g_HT_91_Driver_Inovance.wData[0] )||(3 == g_HT_91_Driver_Inovance.wCommondCode))
							g_HT_91_Driver_Inovance.wCommCount = g_HT_MB_Driver_Inovance.wCommCount;
						else if((6 == g_HT_91_Driver_Inovance.wCommondCode)&&(g_HT_91_Driver_Inovance.wData[0] == g_HT_MB_Driver_Inovance.wData[0])&&\
						(g_HT_91_Driver_Inovance.wData[1] == g_HT_MB_Driver_Inovance.wData[1]))
							g_HT_91_Driver_Inovance.wCommCount = g_HT_MB_Driver_Inovance.wCommCount;

						g_HT_91_Driver_Inovance.wTargetID = g_HT_MB_Driver_Inovance.wSourceID;
						ResetTimer(&g_HT_MB_Driver_Inovance.wSendTime, INOVANCE_HEARTBEAT_CYCLE);
						ResetTimer(&g_HT_MB_Driver_Inovance.wSendAgain, INOVANCE_SENDAGAIN_CYCLE);
						ResetTimer(&g_HT_MB_Driver_Inovance.wOfflineRestart, INOVANCE_RESTART_CYCLE);
						g_HT_MB_Driver_Inovance.bSendEnd = FALSE;

//						CANOpenCHBManager_RecvedMsg(&pCard->_sPDOManager);
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
//						CANOpenCHBManager_RecvedMsg(&pCard->_sCHBManager);
						break;
					case CANOPENFUNCID_SEND_SDO:
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
		}
		break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

/*
 * [CommComp_driver__s_card_xingtai_CANopen_event_callback description]
 * @param pUnique     [description]
 * @param pData_frame [description]
 * @param eEvent_type [description]
 */
void CommComp_driver__s_card_xingtai_CANopen_event_callback(CANOpenCompUnique_S* pUnique, CANOpenDataFrame_S* pData_frame, E_CANOpenEventType eEvent_type)
{
	struct CommComp_driver__s_card_xingtai* pCard;
	WORD	wLinkState;

	if(g_XT_91_Driver.bSendTo55End == false)//上一次还没发给55，就不处理本次报文
		return;

	DEBUG_ASSERT_PARAM(NULL != pUnique, "null pointer");
	pCard = ContainerOf(pUnique, struct CommComp_driver__s_card_xingtai, _sUnique_CANopen_comp);
	switch (eEvent_type)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:
			{
				E_CANOpenFuncID eFuncID;

				DEBUG_ASSERT_PARAM(NULL != pData_frame, "null pointer");
				eFuncID = CANOpen_GetFuncIDByCOBID(pData_frame->uHead.sStand.bCOBID);
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO3:
						pCard->wSoureID 	= (WORD)pData_frame->uData.sU8.bData0;
						pCard->wCommondCode = ((WORD)pData_frame->uData.sU8.bData2<<8)+(WORD)pData_frame->uData.sU8.bData3;
						pCard->wData[0] 	= ((WORD)pData_frame->uData.sU8.bData4<<8)+(WORD)pData_frame->uData.sU8.bData5;
						pCard->wData[1] 	= ((WORD)pData_frame->uData.sU8.bData6<<8)+(WORD)pData_frame->uData.sU8.bData7;

						if(TRUE == g_XT_MB_Driver.bSendEnd)//等待读参数回复时，不再接收实时数据
						{
							if((XINGTAI_SENDREADPARACMD == pCard->wCommondCode)&&(pCard->wData[0] == g_XT_MB_Driver.wData[0]))
							{
								memcpy(&g_XT_91_Driver.wSourceID,&pCard->wSoureID,4*sizeof(WORD));
								g_XT_91_Driver.wCommCount = g_XT_MB_Driver.wCommCount;
								g_XT_MB_Driver.bSendEnd = FALSE;
							}
						}
						else
						{
							if((XINGTAI_UPLOADPARACMD == pCard->wCommondCode)&&(pCard->wData[0] != g_XT_MB_Driver.wData[0]))
							{
								memcpy(&g_XT_91_Driver.wSourceID,&pCard->wSoureID,4*sizeof(WORD));
								g_XT_91_Driver.wCommondCode = 0x1;//用0x1以区分读参数回包命令码驱动器主动上传的命令码
							}else if (XINGTAI_UPLOADSTATECMD == pCard->wCommondCode)
							{
								memcpy(&g_XT_91_Driver.wSourceID,&pCard->wSoureID,4*sizeof(WORD));
								if(g_XT_91_Driver.wData[1]&XINGTAI_STATEERRBIT)//驱动器有警报
								{
									// g_XT_SendEnable.bPFSendEnable = FALSE;
									g_XT_SendEnable.bControlSendEnable = FALSE;
								}else if((g_XT_MB_Driver.bFlagRestart == FALSE)&&(g_XT_SendEnable.bControlSendEnable == FALSE))//驱动器没警报，且不在重连状态中
								{
									g_XT_SendEnable.bControlSendEnable = 2;
								}
								g_XT_91_Driver.wCommondCode = XINGTAI_UPLOADSTATECMD;
							}
						}

						wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
						if((g_XT_MB_Driver.bFlagRestart == TRUE)||((wLinkState & (1 << g_XT_MB_Driver.wXingtaiIndex)) == FALSE))
						{
							wLinkState |= (WORD)1 << g_XT_MB_Driver.wXingtaiIndex;
							DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
							g_XT_MB_Driver.bFlagRestart = FALSE;
							g_XT_SendEnable.bControlSendEnable = 2;
							// g_XT_SendEnable.bPFSendEnable = TRUE;
						}
						g_XT_91_Driver.wSourceID = g_XT_MB_Driver.wTargetID;
						g_XT_91_Driver.wTargetID = g_XT_MB_Driver.wSourceID;
						ResetTimer(&g_XT_MB_Driver.wSendTime, XINGTAI_HEARTBEAT_CYCLE);
						ResetTimer(&g_XT_MB_Driver.wSendAgain, XINGTAI_SENDAGAIN_CYCLE);
						ResetTimer(&g_XT_MB_Driver.wOfflineRestart, XINGTAI_RESTART_CYCLE);
						g_XT_91_Driver.bSendTo55End = false;
						break;
					case CANOPENFUNCID_NMT_ERROR_CTRL:
						break;
					case CANOPENFUNCID_SEND_SDO:
					default:
						break;
				}
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
		}
		break;
		default:
			DEBUG_ASSERT_WARN(false, "case error");
			break;
	}
}

void	CommComp_Driver_Inovance_SendPDO(struct CommComp_S* const pComp)
{
	E_CANChannel	eChannel;
	CANOpenDataFrameData_U uData_frame;

	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	uData_frame.sU8.bData0 = (BYTE)g_HT_MB_Driver_Inovance.wSourceID;
	uData_frame.sU8.bData1 = 0;
	uData_frame.sU8.bData2 = (BYTE)g_HT_MB_Driver_Inovance.wCommondCode>>8;
	uData_frame.sU8.bData3 = (BYTE)g_HT_MB_Driver_Inovance.wCommondCode;
	uData_frame.sU8.bData4 = (BYTE)g_HT_MB_Driver_Inovance.wData[0]>>8;
	uData_frame.sU8.bData5 = (BYTE)g_HT_MB_Driver_Inovance.wData[0];
	uData_frame.sU8.bData6 = (BYTE)g_HT_MB_Driver_Inovance.wData[1]>>8;
	uData_frame.sU8.bData7 = (BYTE)g_HT_MB_Driver_Inovance.wData[1];
	CANOpenMaster_SendPDO(eChannel, (BYTE)g_HT_MB_Driver_Inovance.wTargetID&0x7F, (E_CANOpenFuncID)(g_HT_MB_Driver_Inovance.wTargetID>>7), 8, &uData_frame);
}

void	CommComp_Driver_Xingtai_SendPDO(struct CommComp_S* const pComp)
{
	E_CANChannel	eChannel;
	CANOpenDataFrameData_U uData_frame;

	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	uData_frame.sU8.bData0 = (BYTE)g_XT_MB_Driver.wSourceID;
	uData_frame.sU8.bData1 = 0;
	uData_frame.sU8.bData2 = (BYTE)g_XT_MB_Driver.wCommondCode>>8;
	uData_frame.sU8.bData3 = (BYTE)g_XT_MB_Driver.wCommondCode;
	uData_frame.sU8.bData4 = (BYTE)g_XT_MB_Driver.wData[0]>>8;
	uData_frame.sU8.bData5 = (BYTE)g_XT_MB_Driver.wData[0];
	uData_frame.sU8.bData6 = (BYTE)g_XT_MB_Driver.wData[1]>>8;
	uData_frame.sU8.bData7 = (BYTE)g_XT_MB_Driver.wData[1];
	CANOpenMaster_SendPDO(eChannel, (BYTE)g_XT_MB_Driver.wTargetID&0x7F, (E_CANOpenFuncID)(g_XT_MB_Driver.wTargetID>>7), 8, &uData_frame);
}

void	CommComp_Driver_Xingtai_SendControl(struct CommComp_S* const pComp)
{
	E_CANChannel	eChannel;
	CANOpenDataFrameData_U uData_frame;


	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	uData_frame.sU8.bData0 = (BYTE)g_XT_MB_Driver.wSourceID;
	uData_frame.sU8.bData1 = 0;
	uData_frame.sU8.bData2 = (BYTE)XINGTAI_SENDCTRLCMD>>8;
	uData_frame.sU8.bData3 = (BYTE)XINGTAI_SENDCTRLCMD;
	uData_frame.sU8.bData4 = 0;
	uData_frame.sU8.bData5 = (BYTE)bSend_InjState_Xingtai;
	uData_frame.sU8.bData6 = 0;
	if(TRUE == g_XT_SendEnable.bControlSendState)
		uData_frame.sU8.bData7 = 0X7;
	else
		uData_frame.sU8.bData7 = 0X6;
	CANOpenMaster_SendPDO(eChannel, (BYTE)g_XT_MB_Driver.wTargetID&0x7F, (E_CANOpenFuncID)(g_XT_MB_Driver.wTargetID>>7), 8, &uData_frame);
}

void	CommComp_Driver_Xingtai_SendPF(struct CommComp_S* const pComp)
{
	E_CANChannel	eChannel;
	CANOpenDataFrameData_U uData_frame;

	wSend_Press_Xingtai = Press_Xingtai>>2;
	wSend_Flow_Xingtai	= Flow_Xingtai>>2;
	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
	uData_frame.sU8.bData0 = (BYTE)g_XT_MB_Driver.wSourceID;
	uData_frame.sU8.bData1 = 0;
	uData_frame.sU8.bData2 = (BYTE)XINGTAI_SENDPFCMD>>8;
	uData_frame.sU8.bData3 = (BYTE)XINGTAI_SENDPFCMD;
	uData_frame.sU8.bData4 = (BYTE)(wSend_Flow_Xingtai>>8);;
	uData_frame.sU8.bData5 = (BYTE)wSend_Flow_Xingtai;
	uData_frame.sU8.bData6 = (BYTE)(wSend_Press_Xingtai>>8);
	uData_frame.sU8.bData7 = (BYTE)wSend_Press_Xingtai;
	CANOpenMaster_SendPDO(eChannel, (BYTE)g_XT_MB_Driver.wTargetID&0x7F, (E_CANOpenFuncID)(g_XT_MB_Driver.wTargetID>>7), 8, &uData_frame);
}

// void	CommComp_Driver_Xingtai_SendPress(struct CommComp_S* const pComp)
// {
// 	E_CANChannel	eChannel;
// 	CANOpenDataFrameData_U uData_frame;

// 	wSend_Press_Xingtai = Press_Xingtai>>2;
// 	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
// 	uData_frame.sU8.bData0 = (BYTE)g_XT_MB_Driver.wSourceID;
// 	uData_frame.sU8.bData1 = 0;
// 	uData_frame.sU8.bData2 = (BYTE)XINGTAI_SENDPFCMD>>8;
// 	uData_frame.sU8.bData3 = (BYTE)XINGTAI_SENDPRESSCMD;
// 	uData_frame.sU8.bData4 = 0;
// 	uData_frame.sU8.bData5 = 0;
// 	uData_frame.sU8.bData6 = (BYTE)(wSend_Press_Xingtai>>8);
// 	uData_frame.sU8.bData7 = (BYTE)wSend_Press_Xingtai;
// 	CANOpenMaster_SendPDO(eChannel, (BYTE)g_XT_MB_Driver.wTargetID&0x7F, (E_CANOpenFuncID)(g_XT_MB_Driver.wTargetID>>7), 8, &uData_frame);
// }

// void	CommComp_Driver_Xingtai_SendFlow(struct CommComp_S* const pComp)
// {
// 	E_CANChannel	eChannel;
// 	CANOpenDataFrameData_U uData_frame;

// 	wSend_Flow_Xingtai = Flow_Xingtai>>2;
// 	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
// 	uData_frame.sU8.bData0 = (BYTE)g_XT_MB_Driver.wSourceID;
// 	uData_frame.sU8.bData1 = 0;
// 	uData_frame.sU8.bData2 = (BYTE)XINGTAI_SENDFLOWCMD>>8;
// 	uData_frame.sU8.bData3 = (BYTE)XINGTAI_SENDFLOWCMD;
// 	uData_frame.sU8.bData4 = 0;
// 	uData_frame.sU8.bData5 = 0;
// 	uData_frame.sU8.bData6 = (BYTE)(wSend_Flow_Xingtai>>8);
// 	uData_frame.sU8.bData7 = (BYTE)wSend_Flow_Xingtai;
// 	CANOpenMaster_SendPDO(eChannel, (BYTE)g_XT_MB_Driver.wTargetID&0x7F, (E_CANOpenFuncID)(g_XT_MB_Driver.wTargetID>>7), 8, &uData_frame);
// }

/*
 * [CommComp_driver__s_card_inovance_CANopen_custom_operation description]
 * @param pComp [description]
 * @param eType [description]
 */
void CommComp_driver__s_card_inovance_CANopen_custom_operation(struct CommComp_S* const pComp, E_CustomFuncType eType)
{
//	struct CommComp_driver__s_card_inovance* pCard;
//	E_CANChannel                                 eChannel;
	WORD	wLinkState;

//	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
//	pCard    = ContainerOf(pComp, struct CommComp_driver__s_card_inovance, _sComp);
//	DEBUG_ASSERT_ILLEGAL(CANOPENNODESTATE_OPERATIONAL == pCard->_sUnique_CANopen_comp.PRO_bNodeState, "state error.");
	wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
//			if (CheckTimerOut(&pCard->_sPDOSendTime))
			if(TRUE == g_HT_MB_Driver_Inovance.bSendEnable)
			{
				CommComp_Driver_Inovance_SendPDO(pComp);
				ResetTimer(&g_HT_MB_Driver_Inovance.wSendTime, INOVANCE_HEARTBEAT_CYCLE);
				ResetTimer(&g_HT_MB_Driver_Inovance.wSendAgain, INOVANCE_SENDAGAIN_CYCLE);
				ResetTimer(&g_HT_MB_Driver_Inovance.wOfflineRestart, INOVANCE_RESTART_CYCLE);
				g_HT_MB_Driver_Inovance.bSendEnable = FALSE;
				g_HT_MB_Driver_Inovance.bSendEnd = TRUE;
			}else if((CheckTimerOut(&g_HT_MB_Driver_Inovance.wSendAgain))&&((wLinkState&(1<<g_HT_MB_Driver_Inovance.wInovanceIndex))!=false))
			{
				CommComp_Driver_Inovance_SendPDO(pComp);
				g_HT_91_Driver_Inovance.wSendAgainCount += 1;
				ResetTimer(&g_HT_MB_Driver_Inovance.wSendAgain, INOVANCE_SENDAGAIN_CYCLE);
			}

			break;
		case CUSTOMFUNCTYPE_SLOW:
		{
//			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
			if(TRUE == g_HT_MB_Driver_Inovance.bSendEnd)
			{
				if(CheckTimerOut(&g_HT_MB_Driver_Inovance.wSendTime))
				{
					wLinkState &= ~((WORD)(1 << g_HT_MB_Driver_Inovance.wInovanceIndex));
					DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
					ResetTimer(&g_HT_MB_Driver_Inovance.wSendTime, INOVANCE_HEARTBEAT_CYCLE);
					g_HT_MB_Driver_Inovance.bSendEnd = FALSE;
				}
			}

			if(((wLinkState & (1 << g_HT_MB_Driver_Inovance.wInovanceIndex)) == false)&&CheckTimerOut(&g_HT_MB_Driver_Inovance.wOfflineRestart))
			{
				CommComp_Driver_Inovance_SendPDO(pComp);
				g_HT_MB_Driver_Inovance.bFlagRestart = TRUE;
				ResetTimer(&g_HT_MB_Driver_Inovance.wOfflineRestart, INOVANCE_RESTART_CYCLE);

			}
		}
		break;
		default:
			break;
	}
}

/*
 * [CommComp_driver__s_card_xingtai_CANopen_custom_operation description]
 * @param pComp [description]
 * @param eType [description]
 */
void CommComp_driver__s_card_xingtai_CANopen_custom_operation(struct CommComp_S* const pComp, E_CustomFuncType eType)
{
//	struct CommComp_driver__s_card_xingtai* pCard;
//	E_CANChannel                                 eChannel;
	WORD	wLinkState;

//	eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(pComp));
//	pCard    = ContainerOf(pComp, struct CommComp_driver__s_card_xingtai, _sComp);
//	DEBUG_ASSERT_ILLEGAL(CANOPENNODESTATE_OPERATIONAL == pCard->_sUnique_CANopen_comp.PRO_bNodeState, "state error.");
	wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
			// if (CheckTimerOut(&pCard->_sPDOSendTime))
			if(TRUE == g_XT_MB_Driver.bSendEnable)//当前页只有读写参数指令用的这段
			{
				CommComp_Driver_Xingtai_SendPDO(pComp);
				ResetTimer(&g_XT_MB_Driver.wSendTime, XINGTAI_HEARTBEAT_CYCLE);
				ResetTimer(&g_XT_MB_Driver.wSendAgain, XINGTAI_SENDAGAIN_CYCLE);
				ResetTimer(&g_XT_MB_Driver.wOfflineRestart, XINGTAI_RESTART_CYCLE);
				ResetTimer(&g_XT_MB_Driver.wSendPFCycle, XINGTAI_SENDPF_CYCLE);
				g_XT_MB_Driver.bSendEnable = FALSE;
				if(XINGTAI_SENDREADPARACMD ==g_XT_MB_Driver.wCommondCode)//发送读取指令不回才会判断离线
					g_XT_MB_Driver.bSendEnd = TRUE;
			}else if((g_XT_MB_Driver.bSendEnd == TRUE)&&(CheckTimerOut(&g_XT_MB_Driver.wSendAgain))&&((wLinkState&(1<<g_XT_MB_Driver.wXingtaiIndex))!=FALSE))
			{
				CommComp_Driver_Xingtai_SendPDO(pComp);
				g_XT_91_Driver.wSendAgainCount += 1;
				ResetTimer(&g_XT_MB_Driver.wSendAgain, XINGTAI_SENDAGAIN_CYCLE);
				ResetTimer(&g_XT_MB_Driver.wSendPFCycle, XINGTAI_SENDPF_CYCLE);
			}

			if(TRUE == (wLinkState & (1 << g_XT_MB_Driver.wXingtaiIndex)))
			{
				if(CheckTimerOut(&g_XT_MB_Driver.wSendPFCycle)&&(1 == g_XT_SendEnable.bControlSendEnable))
				{
					CommComp_Driver_Xingtai_SendControl(pComp);
					g_XT_SendEnable.bControlSendEnable = 2;
					ResetTimer(&g_XT_MB_Driver.wSendPFCycle, XINGTAI_SENDPF_CYCLE);
				}else if(CheckTimerOut(&g_XT_MB_Driver.wSendPFCycle)&&(2 == g_XT_SendEnable.bControlSendEnable))
				{
					CommComp_Driver_Xingtai_SendPF(pComp);
					ResetTimer(&g_XT_MB_Driver.wSendPFCycle, XINGTAI_SENDPF_CYCLE);
				}
			}
			break;
		case CUSTOMFUNCTYPE_SLOW:
		{
//			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
			if(TRUE == g_XT_MB_Driver.bSendEnd)
			{
				if(CheckTimerOut(&g_XT_MB_Driver.wSendTime))
				{
					wLinkState &= ~((WORD)(1 << g_XT_MB_Driver.wXingtaiIndex));
					DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
					ResetTimer(&g_XT_MB_Driver.wSendTime, XINGTAI_HEARTBEAT_CYCLE);
					g_XT_MB_Driver.bSendEnd = FALSE;
				}
			}

			if(((wLinkState & (1 << g_XT_MB_Driver.wXingtaiIndex)) == FALSE)&&CheckTimerOut(&g_XT_MB_Driver.wOfflineRestart))
			{

				CommComp_Driver_Xingtai_SendPDO(pComp);
				g_XT_MB_Driver.bFlagRestart = TRUE;
				ResetTimer(&g_XT_MB_Driver.wOfflineRestart, XINGTAI_RESTART_CYCLE);
				// g_XT_SendEnable.bPFSendEnable = FALSE;
				g_XT_SendEnable.bControlSendEnable = FALSE;
			}
		}
			break;
		default:
			break;
	}
}



