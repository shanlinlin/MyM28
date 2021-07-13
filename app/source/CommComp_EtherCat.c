/*==============================================================================+
|  Function : Communication Component EtherCat                                  |
|  Task     : Communication Component EtherCat Source File                      |
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
#include "CommComp_EtherCat.h"
#include "task_ethercatmaster.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define MULTICHANNEL TRUE

#define ETHERCAT_READ_SUM 50
#define ETHERCAT_WRITE_SUM 50
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
typedef void (*CtrlCmdFunc_F)(CommComp_S* psComp, EtherCatCompUnique_S* psCompUnique);

typedef struct Inside_ChannelManager_Struct {
	void* pvUser;
	WORD  awMsgBuffer[CONST_SIZE_ETHERCATPACKET];
} Inside_ChannelManager_S;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void*                  MallocMsgBuffer(EtherCatCompUnique_S* psCompUnique);
static void                   FreeMsgBuffer(EtherCatCompUnique_S* psCompUnique);
static void                   EtherCat_Read(CommComp_S* psComp);
static void                   EtherCat_Write(CommComp_S* psComp);
static void                   CtrlCmdFunc_RESET_COMM(CommComp_S* psComp, EtherCatCompUnique_S* psCompUnique);
inline WORD                   EtherCatUploadSDO(E_CommType eCommType, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize, BOOL bCompleteAccess);
inline WORD                   EtherCatDownloadSDO(E_CommType eCommType, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize, BOOL bCompleteAccess);
static EtherCatODInfo_S       Default_fDBIndexToODInfo(CommComp_S* psComp, CommSubComp_S* psSubComp, DBIndex_T tDBIndex);
static EtherCatEventRetCode_E Default_fEvent(CommComp_S* psComp, EtherCatEvent_U* puEvent);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
// static char                                          u_acEtherCatBuffer[CONST_SIZE_ETHERCATPACKET];

#if !(MULTICHANNEL)
static CommComp_S* u_psComp;
#endif
static volatile Inside_ChannelManager_S u_asEtherCatChannel[2];

static CtrlCmdFunc_F const uc_afCtrlCmd[] = {
	CtrlCmdFunc_RESET_COMM,
};
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_EtherCat description]
 */
void Init_EtherCat(void)
{
	WORD i;

	// memset(u_acEtherCatBuffer, 0, sizeof(u_acEtherCatBuffer));
	for (i = 0; i < (sizeof(u_asEtherCatChannel) / sizeof(*u_asEtherCatChannel)); ++i) {
		u_asEtherCatChannel[i].pvUser = NULL;
		memset((void*)u_asEtherCatChannel[i].awMsgBuffer, 0, sizeof(u_asEtherCatChannel[i].awMsgBuffer));
	}

#if !(MULTICHANNEL)
	u_psComp = NULL;
#endif
}
/**
 * [Run_EtherCat description]
 * @param psComp [description]
 */
void Run_EtherCat(E_CommType eCommType, CommComp_S* psComp)
{
	EtherCatCompUnique_S* psCompUnique;

	DEBUG_ASSERT(COMMTYPE_ETHERCAT == eCommType);
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
	psCompUnique = (EtherCatCompUnique_S*)psComp->PRO_pvUnique;
	DEBUG_ASSERT(NULL != psCompUnique);
	if (psCompUnique->PRO_eCtrlCmd <= ETHERCATCTRLCMD_MAX) {
		EtherCatCtrlCmd_E eCtrlCmd;

		eCtrlCmd = (EtherCatCtrlCmd_E)psCompUnique->PRO_eCtrlCmd;
		DEBUG_ASSERT(NULL != psCompUnique);
		psCompUnique->PRO_eCtrlCmd = ETHERCATCTRLCMD_SUM;
		DEBUG_ASSERT(eCtrlCmd < (sizeof(uc_afCtrlCmd) / sizeof(*uc_afCtrlCmd)));
		DEBUG_ASSERT(NULL != uc_afCtrlCmd[eCtrlCmd]);
		uc_afCtrlCmd[eCtrlCmd](psComp, psCompUnique);
	}
	if (g_ECController.Master.wECState != ETHCRCAT_CONFIG_SUCCESS) {
		switch (CommComp_GetState(psComp))
		{
			case COMPSTATE_CONFIG:
				break;
			default:
				SCommComp_SetState(psComp, COMPSTATE_CONFIG);
				break;
		}
		return;
	} else if (COMPSTATE_CONFIG == CommComp_GetState(psComp)) {
		// SCommComp_SetAllWriteDBFlag(psComp);
		SCommComp_SetState(psComp, COMPSTATE_SYNC);
	}

#if !(MULTICHANNEL)
	if (NULL == u_psComp)
		u_psComp = psComp;
	else if (u_psComp != psComp)
		return;
#endif

	if (psCompUnique->PRI_bSlaveIndex >= g_ECController.Master.wAppSlaveCount) {
		psCompUnique->PRI_bSlaveIndex = GetECSlaveIdxByNodeID(psCompUnique->PRO_wNodeID);
		if (psCompUnique->PRI_bSlaveIndex >= g_ECController.Master.wAppSlaveCount)
			return;
	}
	switch (psComp->PRO_bSDOState) {
	case COMMSDOSTATE_IDLE: {
#if !(MULTICHANNEL)
		u_psComp = NULL;
#endif
		break;
	}
	case COMMSDOSTATE_READ: {
		if (NULL == psCompUnique->PRI_pwBufferChannel) {
			if (NULL == MallocMsgBuffer(psCompUnique))
				break;
		}
		EtherCat_Read(psComp);
		break;
	}
	case COMMSDOSTATE_WRITE: {
		if (NULL == psCompUnique->PRI_pwBufferChannel) {
			if (NULL == MallocMsgBuffer(psCompUnique))
				break;
		}
		EtherCat_Write(psComp);
		break;
	}
	default: {
		DEBUG_ASSERT(false);
		break;
	}
	}
}
/**
 * [SEtherCatSubCompUnique_Init description]
 * @param psSubCompUnique  [description]
 * @param fDBIndexToODInfo [description]
 */
void SEtherCatSubCompUnique_Init(EtherCatSubCompUnique_S* psSubCompUnique)
{
	DEBUG_ASSERT(NULL != psSubCompUnique);
	psSubCompUnique->PRO_fDBIndexToODInfo = Default_fDBIndexToODInfo;
}
/**
 * [SEtherCatSubCompUnique_InitDBIndexToODInfo description]
 * @param psSubCompUnique  [description]
 * @param fDBIndexToODInfo [description]
 */
void SEtherCatSubCompUnique_InitDBIndexToODInfo(EtherCatSubCompUnique_S* psSubCompUnique, EtherCatDBIndexToODInfo_F fDBIndexToODInfo)
{
	DEBUG_ASSERT(NULL != psSubCompUnique);
	psSubCompUnique->PRO_fDBIndexToODInfo = (NULL != fDBIndexToODInfo) ? fDBIndexToODInfo : Default_fDBIndexToODInfo;
}
/**
 * [SEtherCatCompUnique_Init description]
 * @param psCompUnique [description]
 */
void SEtherCatCompUnique_Init(EtherCatCompUnique_S* psCompUnique)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	psCompUnique->PRO_fEvent            = Default_fEvent;
	psCompUnique->PRI_sODInfo.bDataSize = 0;
	psCompUnique->PRI_pwBufferChannel   = NULL;
	psCompUnique->PRO_wNodeID           = 0;
	psCompUnique->PRI_bSlaveIndex       = (WORD)-1;
	psCompUnique->PRI_bSlavePulse       = FALSE;
	psCompUnique->PRI_bComplateAccess   = FALSE;
	psCompUnique->PRI_bMsgSize          = 0;
	psCompUnique->PRO_eCtrlCmd          = ETHERCATCTRLCMD_SUM;
}
/**
 * [SEtherCatCompUnique_InitEvent description]
 * @param psCompUnique [description]
 * @param fEvent       [description]
 */
void SEtherCatCompUnique_InitEvent(EtherCatCompUnique_S* psCompUnique, EtherCatEvent_F fEvent)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	psCompUnique->PRO_fEvent = (NULL != fEvent) ? fEvent : Default_fEvent;
}
/**
 * [SEtherCatCompUnique_SetNodeID description]
 * @param psCompUnique [description]
 * @param wNodeID      [description]
 */
void SEtherCatCompUnique_SetNodeID(EtherCatCompUnique_S* psCompUnique, WORD wNodeID)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	if (wNodeID != psCompUnique->PRO_wNodeID) {
		psCompUnique->PRO_wNodeID = wNodeID;
		EtherCat_ResetComm(psCompUnique);
	}
}
/**
 * [EtherCat_Read description]
 * @param psComp [description]
 */
static void EtherCat_Read(CommComp_S* psComp)
{
	EtherCatCompUnique_S* psCompUnique;
	WORD*                 pwMsgAddr;
	EtherCatEvent_U       uEvent;

	DEBUG_ASSERT(NULL != psComp);
	psCompUnique = (EtherCatCompUnique_S*)psComp->PRO_pvUnique;
	DEBUG_ASSERT(NULL != psCompUnique);
	if (psCompUnique->PRI_bSlavePulse) {
		if (psCompUnique->PRI_bComplateAccess) {
			// pwMsgAddr = (WORD *)u_acEtherCatBuffer;
			DEBUG_ASSERT(NULL != psCompUnique->PRI_pwBufferChannel);
			pwMsgAddr = (WORD*)psCompUnique->PRI_pwBufferChannel;
		} else
			pwMsgAddr = (WORD*)DB_GetDataAddr(psComp->PRI_tCurIndex);
	} else {
		uEvent.eEvent             = ETHERCATEVENT_SDO_READ;
		uEvent.sSDORead.tCurIndex = psComp->PRI_tCurIndex;
		switch (psCompUnique->PRO_fEvent(psComp, (EtherCatEvent_U*)&uEvent)) {
		case ETHERCATEVENTRETCODE_SUCCESS: {
			EtherCatSubCompUnique_S* psSubCompUnique;

			DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
			psSubCompUnique = (EtherCatSubCompUnique_S*)psComp->PRI_psCurSubComp->PRO_pvUnique;
			DEBUG_ASSERT(NULL != psSubCompUnique);
			DEBUG_ASSERT(NULL != psSubCompUnique->PRO_fDBIndexToODInfo);
			psCompUnique->PRI_sODInfo = psSubCompUnique->PRO_fDBIndexToODInfo(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
			if (0 == psCompUnique->PRI_sODInfo.bDataSize) {
				SCommComp_ClrSDOState(psComp);

#if !(MULTICHANNEL)
				u_psComp = NULL;
#endif
				return;
			}
			pwMsgAddr                         = (WORD*)DB_GetDataAddr(psComp->PRI_tCurIndex);
			psCompUnique->PRI_bMsgSize        = psCompUnique->PRI_sODInfo.bDataSize;
			psCompUnique->PRI_bComplateAccess = FALSE;
			break;
		}
		case ETHERCATEVENTRETCODE_SDO_COMPLETEREAD: {
			psCompUnique->PRI_sODInfo.tODID     = uEvent.sSDORead.tRetID;
			psCompUnique->PRI_sODInfo.bSubIndex = 0;
			// pwMsgAddr = (WORD *)u_acEtherCatBuffer;
			DEBUG_ASSERT(NULL != psCompUnique->PRI_pwBufferChannel);
			pwMsgAddr                         = (WORD*)psCompUnique->PRI_pwBufferChannel;
			psCompUnique->PRI_bMsgSize        = uEvent.sSDORead.wRetMsgSize;
			psCompUnique->PRI_bComplateAccess = TRUE;
			break;
		}
		default: {
			SCommComp_ClrSDOState(psComp);

#if !(MULTICHANNEL)
			u_psComp = NULL;
#endif
			return;
		}
		}
		//EtherCatUploadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlavePulse, psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess); // THJ 2016-06-15 Mask.
		psCompUnique->PRI_bSlavePulse = TRUE;
	}
	switch (EtherCatUploadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess)) // THJ 2016-06-15 Modify.
	{
	case SDORESULT_TIMEOUT: {
		//psCompUnique->PRI_bSlavePulse = FALSE; // THJ 2016-06-15 Mask.
		break;
	}
	case SDORESULT_SLAVEABORT: {
		DWORD dwTmp;

		if (!psCompUnique->PRI_bComplateAccess) {
			dwTmp = 0;
			if (!Comm_GetWriteChanged(psComp->PRI_tCurIndex)) // THJ 2016-11-28 Add.
				DB_SetData(psComp->PRI_tCurIndex, &dwTmp);
		}
	}
	case SDORESULT_DONE: {
		if (psCompUnique->PRI_bComplateAccess) {
			uEvent.eEvent                      = ETHERCATEVENT_SDO_COMPLATEREAD_FINISH;
			uEvent.sSDOReadFinish.tCurIndex    = psComp->PRI_tCurIndex;
			uEvent.sSDOReadFinish.pawMsgBuffer = pwMsgAddr;
			psCompUnique->PRO_fEvent(psComp, &uEvent);
			// memset(u_acEtherCatBuffer, 0, sizeof(u_acEtherCatBuffer));
		} else
			psCompUnique->PRI_sODInfo.bDataSize = 0;
		FreeMsgBuffer(psCompUnique); // THJ 2017-07-03 Move.
		psCompUnique->PRI_bSlavePulse = FALSE;
		//EtherCatUploadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlavePulse, psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess); // THJ 2016-06-15 Mask.
		SCommComp_ClrSDOState(psComp);

#if !(MULTICHANNEL)
		u_psComp = NULL;
#endif
		break;
	}
	default: {
		break;
	}
	}
}
/**
 * [EtherCat_Write description]
 * @param psComp        [description]
 * @param tComplateODID [description]
 * @param wComplateSize [description]
 */
static void EtherCat_Write(CommComp_S* psComp)
{
	EtherCatCompUnique_S* psCompUnique;
	WORD*                 pwMsgAddr;
	EtherCatEvent_U       uEvent;

	DEBUG_ASSERT(NULL != psComp);
	psCompUnique = (EtherCatCompUnique_S*)psComp->PRO_pvUnique;
	DEBUG_ASSERT(NULL != psCompUnique);
	if (psCompUnique->PRI_bSlavePulse) {
		if (psCompUnique->PRI_bComplateAccess) {
			//pwMsgAddr = (WORD *)u_acEtherCatBuffer;
			DEBUG_ASSERT(NULL != psCompUnique->PRI_pwBufferChannel);
			pwMsgAddr = (WORD*)psCompUnique->PRI_pwBufferChannel;
		} else
			pwMsgAddr = (WORD*)DB_GetDataAddr(psComp->PRI_tCurIndex);
	} else {
		uEvent.eEvent              = ETHERCATEVENT_SDO_WRITE;
		uEvent.sSDOWrite.tCurIndex = psComp->PRI_tCurIndex;
		//uEvent.sSDOWrite.pawRetMsgBuffer = u_acEtherCatBuffer;
		DEBUG_ASSERT(NULL != psCompUnique->PRI_pwBufferChannel);
		uEvent.sSDOWrite.pawRetMsgBuffer = psCompUnique->PRI_pwBufferChannel;
		switch (psCompUnique->PRO_fEvent(psComp, &uEvent)) {
		case ETHERCATEVENTRETCODE_SUCCESS: {
			EtherCatSubCompUnique_S* psSubCompUnique;

			DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
			psSubCompUnique = (EtherCatSubCompUnique_S*)psComp->PRI_psCurSubComp->PRO_pvUnique;
			DEBUG_ASSERT(NULL != psSubCompUnique);
			DEBUG_ASSERT(NULL != psSubCompUnique->PRO_fDBIndexToODInfo);
			psCompUnique->PRI_sODInfo = psSubCompUnique->PRO_fDBIndexToODInfo(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
			if (0 == psCompUnique->PRI_sODInfo.bDataSize) {
				SCommComp_ClrSDOState(psComp);

#if !(MULTICHANNEL)
				u_psComp = NULL;
#endif
				return;
			}
			pwMsgAddr                         = (WORD*)DB_GetDataAddr(psComp->PRI_tCurIndex);
			psCompUnique->PRI_bMsgSize        = psCompUnique->PRI_sODInfo.bDataSize;
			psCompUnique->PRI_bComplateAccess = FALSE;
			break;
		}
		case ETHERCATEVENTRETCODE_SDO_COMPLETEWRITE: {
			psCompUnique->PRI_sODInfo.tODID     = uEvent.sSDOWrite.tRetID;
			psCompUnique->PRI_sODInfo.bSubIndex = 0;
			DEBUG_ASSERT(NULL != psCompUnique->PRI_pwBufferChannel);
			pwMsgAddr                         = (WORD*)psCompUnique->PRI_pwBufferChannel;
			psCompUnique->PRI_bMsgSize        = uEvent.sSDOWrite.wRetMsgSize;
			psCompUnique->PRI_bComplateAccess = TRUE;
			break;
		}
		default: {
			SCommComp_ClrSDOState(psComp);
			return;
		}
		}
		//EtherCatDownloadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlavePulse, psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess); // THJ 2016-06-15 Mask.
		psCompUnique->PRI_bSlavePulse = TRUE;
	}
	switch (EtherCatDownloadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess)) // THJ 2016-06-15 Modify.
	{
	case SDORESULT_TIMEOUT: {
		psCompUnique->PRI_bSlavePulse = psCompUnique->PRI_bSlavePulse;
		//psCompUnique->PRI_bSlavePulse = FALSE;
		//EtherCatDownloadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlavePulse, psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess); // THJ 2016-06-15 Mask.
		break;
	}
	case SDORESULT_SLAVEABORT:
	case SDORESULT_DONE: {
		if (psCompUnique->PRI_bComplateAccess) {
			uEvent.eEvent                    = ETHERCATEVENT_SDO_COMPLATEWRITE_FINISH;
			uEvent.sSDOWriteFinish.tCurIndex = psComp->PRI_tCurIndex;
			psCompUnique->PRO_fEvent(psComp, &uEvent);
			// memset(u_acEtherCatBuffer, 0, sizeof(u_acEtherCatBuffer));
		} else
			psCompUnique->PRI_sODInfo.bDataSize = 0;
		FreeMsgBuffer(psCompUnique); // THJ 2017-07-03 Move.
		psCompUnique->PRI_bSlavePulse = FALSE;
		//EtherCatDownloadSDO(CommComp_GetCommType(psComp), psCompUnique->PRI_bSlavePulse, psCompUnique->PRI_bSlaveIndex, psCompUnique->PRI_sODInfo.tODID, psCompUnique->PRI_sODInfo.bSubIndex, pwMsgAddr, psCompUnique->PRI_bMsgSize, psCompUnique->PRI_bComplateAccess); // THJ 2016-06-15 Mask.
		SCommComp_ClrSDOState(psComp);

#if !(MULTICHANNEL)
		u_psComp = NULL;
#endif
		break;
	}
	default: {
		break;
	}
	}
}
/**
 * [CtrlCmdFunc_RESET_COMM description]
 * @param psComp       [description]
 * @param psCompUnique [description]
 */
static void CtrlCmdFunc_RESET_COMM(CommComp_S* psComp, EtherCatCompUnique_S* psCompUnique)
{
	EtherCatEvent_E eEvent;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psCompUnique);
	DEBUG_ASSERT(NULL != psCompUnique->PRO_fEvent);
	SCommComp_SetState(psComp, COMPSTATE_INIT);
	psCompUnique->PRI_sODInfo.bDataSize = 0;
	eEvent                              = ETHERCATEVENT_RESET_COMM;
	psCompUnique->PRO_fEvent(psComp, (EtherCatEvent_U*)&eEvent);
}
/**
 * [EtherCatUploadSDO description]
 * @param  eCommType       [description]
 * @param  bExcute         [description]
 * @param  wSlaveIndex     [description]
 * @param  wIndex          [description]
 * @param  wSubIndex       [description]
 * @param  pData           [description]
 * @param  wByteSize       [description]
 * @param  bCompleteAccess [description]
 * @return                 [description]
 */
inline WORD EtherCatUploadSDO(E_CommType eCommType, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize, BOOL bCompleteAccess)
{
	//WORD wRet;

	DEBUG_ASSERT(eCommType == COMMTYPE_ETHERCAT);
	{
		NodeID_T tFilterNodeID;

		tFilterNodeID = DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_CANMASTER_WFILTERNODEID);
		if ((512 == tFilterNodeID) || (wSlaveIndex == (tFilterNodeID - 256))) {
				DB_SetDWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_CANMASTER_DWRXSDOCOUNT, DB_GetDWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_CANMASTER_DWRXSDOCOUNT) + 1);
		}
	}
	if (wSlaveIndex >= g_ECController.Master.wAppSlaveCount)
		return SDORESULT_NONE;
	return ECMaster_UploadSDO(&g_ECController.Master, wSlaveIndex, wIndex, wSubIndex, pData, wByteSize, bCompleteAccess); // Anders 2016-6-15,delete bExcute.
}
/**
 * [EtherCatDownloadSDO description]
 * @param  eCommType       [description]
 * @param  bExcute         [description]
 * @param  wSlaveIndex     [description]
 * @param  wIndex          [description]
 * @param  wSubIndex       [description]
 * @param  pData           [description]
 * @param  wByteSize       [description]
 * @param  bCompleteAccess [description]
 * @return                 [description]
 */
inline WORD EtherCatDownloadSDO(E_CommType eCommType, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize, BOOL bCompleteAccess)
{
	//WORD wRet;

	DEBUG_ASSERT(eCommType == COMMTYPE_ETHERCAT);
	{
		NodeID_T tFilterNodeID;

		tFilterNodeID = DB_GetWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_CANMASTER_WFILTERNODEID);
		if ((512 == tFilterNodeID) || (wSlaveIndex == (tFilterNodeID - 256))) {
				DB_SetDWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXSDOCOUNT, DB_GetDWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_CANMASTER_DWTXSDOCOUNT) + 1);
		}
	}
	if (wSlaveIndex >= g_ECController.Master.wAppSlaveCount)
		return SDORESULT_NONE;
	return ECMaster_DownloadSDO(&g_ECController.Master, wSlaveIndex, wIndex, wSubIndex, pData, wByteSize, bCompleteAccess); // Anders 2016-6-15,delete bExcute.;
}
/**
 * [MallocMsgBuffer description]
 * @param psCompUnique [description]
 */
static void* MallocMsgBuffer(EtherCatCompUnique_S* psCompUnique)
{
	WORD i;

	DEBUG_ASSERT(NULL != psCompUnique);
	DEBUG_ASSERT(NULL == psCompUnique->PRI_pwBufferChannel);
	for (i = 0; i < (sizeof(u_asEtherCatChannel) / sizeof(*u_asEtherCatChannel)); ++i) {
		if (NULL == u_asEtherCatChannel[i].pvUser) {
			u_asEtherCatChannel[i].pvUser = psCompUnique;
			memset((void*)u_asEtherCatChannel[i].awMsgBuffer, 0, sizeof(u_asEtherCatChannel[i].awMsgBuffer));
			psCompUnique->PRI_pwBufferChannel = (void*)u_asEtherCatChannel[i].awMsgBuffer;
			return psCompUnique->PRI_pwBufferChannel;
		}
	}
	return NULL;
}
/**
 * [FreeMsgBuffer description]
 * @param psCompUnique [description]
 */
static void FreeMsgBuffer(EtherCatCompUnique_S* psCompUnique)
{
	WORD i;

	DEBUG_ASSERT(NULL != psCompUnique);
	DEBUG_ASSERT(NULL != psCompUnique->PRI_pwBufferChannel);
	for (i = 0; i < (sizeof(u_asEtherCatChannel) / sizeof(*u_asEtherCatChannel)); ++i) {
		if (psCompUnique == u_asEtherCatChannel[i].pvUser) {
			DEBUG_ASSERT((void*)u_asEtherCatChannel[i].awMsgBuffer == psCompUnique->PRI_pwBufferChannel);
			u_asEtherCatChannel[i].pvUser     = NULL;
			psCompUnique->PRI_pwBufferChannel = NULL;
			break;
		}
	}
}
/**
 * [Default_fDBIndexToODInfo description]
 * @param  psComp    [description]
 * @param  psSubComp [description]
 * @param  tDBIndex  [description]
 * @return           [description]
 */
static EtherCatODInfo_S Default_fDBIndexToODInfo(CommComp_S* psComp, CommSubComp_S* psSubComp, DBIndex_T tDBIndex)
{
	EtherCatODInfo_S sODInfo;
	DWORD            dwTmp;

	dwTmp             = DB_IndexToID(tDBIndex) & 0xFFFFFF;
	sODInfo.tODID     = dwTmp >> 8;
	sODInfo.bSubIndex = dwTmp & 0xFF;
	sODInfo.bDataSize = DB_SizeofBData(tDBIndex);
	return sODInfo;
}
/**
 * [Default_fEvent description]
 * @param  psComp  [description]
 * @param  puEvent [description]
 * @return         [description]
 */
static EtherCatEventRetCode_E Default_fEvent(CommComp_S* psComp, EtherCatEvent_U* puEvent)
{
	return ETHERCATEVENTRETCODE_SUCCESS;
}
