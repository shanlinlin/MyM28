/*==============================================================================+
|  Function : Temper card                                                       |
|  Task     : Temper card Source File                                           |
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
#include "CommComp_TemperCard.h"
#include "Error_App.h"
#include "temper_common.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define NODEGRUADING_ERRCOUNT_MAX 7
#define TEMPCARD_PDO_COMMCYCLE 10
#define TEMPCARD_OFFLINE_CTRLTIME 1000 //(TEMPCARD_PDO_COMMCYCLE * 5)
#define NODEGRUADING_REQ_CYCLE 300
#define COLDTEMPER_REQ_CYCLE 250

#define OILCOMPENSATE_CALC_CYCLE 10000 // Anders 2016-12-11, Modify. 1S->10S.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
enum AutoPIDAdjustState
{
	AUTOPIDADJUSTSTATE_IDLE,
	AUTOPIDADJUSTSTATE_ADJUST,
	AUTOPIDADJUSTSTATE_READY
};
//##############################################################################
//
//      Type Definition
//
//##############################################################################
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void Comm_EventCallBack(CommComp_S* psComp, CommEvent_U* puEvent);

static bool TempCardConfigSDOMsg_12(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static void CANOpen_EventCallBack_12(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);
static void CustomFunc_CANOpen_Operation(CommComp_S* psComp, E_CustomFuncType eType);

static bool TempCardConfigSDOMsg_16B(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static void CANOpen_EventCallBack_16B(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);

static WORD GetHeatOutput(TemperCardNum_E eTemperCardNum);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
CommTemperCard_S g_asCommTemperCard[TEMPERCARDNUM_SUM];
//WORD             g_awTemperSegUseState[TEMPERCARDNUM_SUM];

CommTemperCardInfo_S u_asTemperCardInfo[TEMPERCARDNUM_SUM];
WORD                 g_wTemperCardUploadSDO_Abort_CNT = 0; // Anders 2016-2-4, add.

//Ttimer          u_dwOilCompensateCalcTime;
//BOOL            u_blOilCompensateType;
//S_OilCompensate g_sMoldOpenOilCompensate;
//S_OilCompensate g_sMoldCloseOilCompensate;

// bool u_bAutoMode; //hankin 20181130 mask: for new autopid.
//------------------------------------------------------------------------------
static CommOffsetSeg_S const uc_asReadIndexSeg_Card[] = {
	//{ wDBIndexStart,                                      wDBIndexEnd                                     }
	{ DBINDEX_TEMPERCARD7_CONFIG_CARD_ID - DBINDEX_TEMPERCARD7_CONFIG_CARD_ID, DBINDEX_TEMPERCARD7_INTERFACE_SOFTWARE_SN - DBINDEX_TEMPERCARD7_CONFIG_CARD_ID },
};

static DBIndex_T const uc_asTemperCard_base_index[] = {
	DBINDEX_TEMPERCARD1_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD2_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD3_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD4_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD5_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD6_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD7_CONFIG_CARD_ID,
	DBINDEX_TEMPERCARD8_CONFIG_CARD_ID,
};
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_TemperCard description]
 */
void Init_TemperCard(void)
{
	CommTemperCardInfo_S* psTemperCardInfo;
	CommTemperCard_S*     psTemperCard;
	WORD                  i;

	DEBUG_ASSERT(TEMPERCARDNUM_SUM == (sizeof(uc_asReadIndexSeg_Card) / sizeof(*uc_asReadIndexSeg_Card)));
	DEBUG_ASSERT(TEMPERCARDNUM_SUM == (sizeof(uc_asTemperCard_base_index) / sizeof(*uc_asTemperCard_base_index)));
	for (i = 0; i < TEMPERCARDNUM_SUM; ++i)
	{
		//g_awTemperSegUseState[i] = 0;

		psTemperCardInfo = &u_asTemperCardInfo[i];
		TemperCardInfo_Init(psTemperCardInfo);
		SCommSubComp_InitReadInfo(&psTemperCardInfo->PRI_sSubComp, uc_asReadIndexSeg_Card, uc_asTemperCard_base_index[i], sizeof(uc_asReadIndexSeg_Card) / sizeof(*uc_asReadIndexSeg_Card));
		SCommSubComp_InitWriteInfo(&psTemperCardInfo->PRI_sSubComp, uc_asReadIndexSeg_Card, uc_asTemperCard_base_index[i], sizeof(uc_asReadIndexSeg_Card) / sizeof(*uc_asReadIndexSeg_Card));

		psTemperCard = &g_asCommTemperCard[i];
		TemperCard_Init(psTemperCard);
		SCommComp_InitEventCallBack(&psTemperCard->PRI_sComp, Comm_EventCallBack);
#if (DEBUG)
		psTemperCard->PRI_sComp.PUB_eCompType = (E_CompType)(COMPTYPE_TEMPCARD1 + i);
#endif

		// psTemperCard->PUB_wPDOSendCycle = TEMPCARD_PDO_COMMCYCLE;    // THJ 2015-06-23 Mask:
		SCommComp_AddSubComp(&psTemperCard->PRI_sComp, &psTemperCardInfo->PRI_sSubComp);
	}
	// OilCompensate_Init(&g_sMoldOpenOilCompensate, DBINDEX_LOCAL0_MOLD_OPEN_OIL_COMPENSATE_ENABLE, DBINDEX_MOLD0_INTERFACE_OPEN_PARAM_POSITION_CONTROL_KP);
	// OilCompensate_Init(&g_sMoldCloseOilCompensate, DBINDEX_LOCAL0_MOLD_CLOSE_OIL_COMPENSATE_ENABLE, DBINDEX_MOLD0_INTERFACE_CLOSE_PARAM_POSITION_CONTROL_KP);
	// u_bAutoMode = false; //hankin 20181130 mask: for new autopid.
}

WORD GetSegNum_TemperCard(TemperCardNum_E eCardNum)
{
	DEBUG_ASSERT_ILLEGAL(eCardNum < (sizeof(g_asCommTemperCard) / sizeof(*g_asCommTemperCard)), "card number error.");
	return g_asCommTemperCard[eCardNum].PRI_xSegNext - g_asCommTemperCard[eCardNum].PRI_xSegStart;
}

/**
 * [TemperCardInfo_Init description]
 * @param psTemperCardInfo [description]
 */
void TemperCardInfo_Init(CommTemperCardInfo_S* psTemperCardInfo)
{
	DEBUG_ASSERT(NULL != psTemperCardInfo);
	SCommSubComp_Init(&psTemperCardInfo->PRI_sSubComp);
	SCANOpenSubCompUnique_Init(&psTemperCardInfo->PRI_sCANOpenUnique);
}

/**
 * [TemperCard_Init description]
 * @param psTemperCard [description]
 */
void TemperCard_Init(CommTemperCard_S* psTemperCard)
{
	DEBUG_ASSERT(NULL != psTemperCard);
	SCommComp_Init(&psTemperCard->PRI_sComp);
	SCANOpenCompUnique_Init(&psTemperCard->PRI_sCANOpenUnique);
	// psTemperCard->PUB_wPDOSendCycle = 0; // THJ 2015-06-23 Mask:
	// THJ 2015-05-06 Modify(2 Line):
	// psTemperCard->PRI_bReqNodeGruadState = FALSE;
	psTemperCard->PRI_bRespNodeGruadCount  = 0;
	psTemperCard->PRI_bNodeGruadErrorCount = 0;
	psTemperCard->PRI_xSegStart            = 0;
	psTemperCard->PRI_xSegNext             = 0;
	ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, 0);
	ResetTimer(&psTemperCard->PRI_dwNodeGruadTime, 0);
	ResetTimer(&psTemperCard->PRI_dwPDOSendTime, 0);
	ResetTimer(&psTemperCard->_dwPreAlarmCtrlTimer, 0);
}

/**
 * [OilCompensate_Init description]
 * @param oilCompensate [description]
 * @param baseIndex     [description]
 * @param actKpIndex    [description]
 */
void OilCompensate_Init(S_OilCompensate* oilCompensate, DBIndex_T baseIndex, DBIndex_T actKpIndex)
{
	DEBUG_ASSERT(NULL != oilCompensate);
	DEBUG_ASSERT(baseIndex < DBINDEX_SUM);
	DEBUG_ASSERT(actKpIndex < DBINDEX_SUM);
	oilCompensate->_baseIndex  = baseIndex;
	oilCompensate->_actKpIndex = actKpIndex;
	oilCompensate->_oilIndex   = DBINDEX_SUM;
	oilCompensate->_enable     = FALSE;
	oilCompensate->_k          = 0.0;
	oilCompensate->_b          = 0.0;
	oilCompensate->backupKp    = 0.0;
}

/**
 * [OilCompensate_SetOilSegNumber description]
 * @param oilCompensate [description]
 * @param oilSegNumber  [description]
 */
void OilCompensate_SetOilSegNumber(S_OilCompensate* oilCompensate, WORD oilSegNumber)
{
	DBIndex_T oilSegIndex;

	DEBUG_ASSERT(NULL != oilCompensate);
	if (oilSegNumber < 1)
		oilSegIndex = DBINDEX_SUM;
	else if (oilSegNumber > 96)
		oilSegIndex = DBINDEX_SUM;
	else
		oilSegIndex = DBINDEX_TEMPER1_INTERFACE_SEG1_REAL_CURRENT + TEMPER_ONESEG_DATANBR * (oilSegNumber - 1);
	if (oilCompensate->_oilIndex != oilSegIndex)
	{
		oilCompensate->_oilIndex = oilSegIndex;
		oilCompensate->_enable   = FALSE;
		DB_SetFLOAT(oilCompensate->_actKpIndex, oilCompensate->backupKp);
		Comm_SetWriteFlagByDBIndex(oilCompensate->_actKpIndex);
	}
}

/**
 * [OilCompensate_CalcLinear description]
 * @param oilCompensate [description]
 */
void OilCompensate_CalcLinear(S_OilCompensate* oilCompensate)
{
	FLOAT tmp, fTmpLow, fTmpHigh;

	DEBUG_ASSERT(NULL != oilCompensate);
	fTmpLow           = DB_GetLONG(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_LOW_TEMPERATURE) * 0.1;
	fTmpHigh          = DB_GetLONG(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_HIGH_TEMPERATURE) * 0.1;
	tmp               = (fTmpHigh - fTmpLow);
	oilCompensate->_k = (DB_GetFLOAT(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_HIGH_TEMPERATURE_KP) - DB_GetFLOAT((oilCompensate->_baseIndex + OILCOMPENSATEINDEX_LOW_TEMPERATURE_KP))) / tmp;
	oilCompensate->_b = (fTmpHigh * DB_GetFLOAT(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_LOW_TEMPERATURE_KP)
	                        - fTmpLow * DB_GetFLOAT(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_HIGH_TEMPERATURE_KP))
	    / tmp;

	oilCompensate->minY = DB_GetFLOAT(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_LOW_TEMPERATURE_KP);
	oilCompensate->maxY = DB_GetFLOAT(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_HIGH_TEMPERATURE_KP);
	if (oilCompensate->minY > oilCompensate->maxY)
	{
		tmp                 = oilCompensate->minY;
		oilCompensate->minY = oilCompensate->maxY;
		oilCompensate->maxY = tmp;
	}
}

/**
 * [OilCompensate_Run description]
 * @param oilCompensate [description]
 */
void OilCompensate_Run(S_OilCompensate* oilCompensate)
{
	DEBUG_ASSERT(NULL != oilCompensate);

	if (oilCompensate->_enable)
	{
		if (0 == DB_GetWORD(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_ENABLE))
		{
			oilCompensate->_enable = FALSE;
			DB_SetFLOAT(oilCompensate->_actKpIndex, oilCompensate->backupKp);
			Comm_SetWriteFlagByDBIndex(oilCompensate->_actKpIndex);
		}
	}
	else
	{
		if (0 != DB_GetWORD(oilCompensate->_baseIndex + OILCOMPENSATEINDEX_ENABLE))
		{
			if (oilCompensate->_oilIndex < DBINDEX_SUM)
			{
				oilCompensate->_enable  = TRUE;
				oilCompensate->backupKp = DB_GetFLOAT(oilCompensate->_actKpIndex);
			}
		}
	}
	if (oilCompensate->_enable)
	{
		FLOAT ackKp;
		FLOAT oilTemperature;

		oilTemperature = DB_GetLONG(oilCompensate->_oilIndex) * 0.1;
		ackKp          = oilCompensate->_k * oilTemperature + oilCompensate->_b;
		if (ackKp < oilCompensate->minY)
			ackKp = oilCompensate->minY;
		else if (ackKp > oilCompensate->maxY)
			ackKp = oilCompensate->maxY;
		DB_SetFLOAT(oilCompensate->_actKpIndex, ackKp * oilCompensate->backupKp); // Anders 2016-6-2, add *oilCompensate->backupKp.
		Comm_SetWriteFlagByDBIndex(oilCompensate->_actKpIndex);
	}
}

/**
 * [Comm_EventCallBack description]
 * @param psComp  [description]
 * @param puEvent [description]
 */
static void Comm_EventCallBack(CommComp_S* psComp, CommEvent_U* puEvent)
{
	DEBUG_ASSERT(NULL != puEvent);
	switch (puEvent->eEvent)
	{
		case COMMEVENT_COMP_CHANGE_COMMTYPE:
		{
			CommSubComp_S*        psSubComp;
			CommTemperCard_S*     psTemperCard;
			CommTemperCardInfo_S* psTemperCardInfo;
			WORD                  wLinkState;

			DEBUG_ASSERT(NULL != psComp);
			psTemperCard = ContainerOf(psComp, CommTemperCard_S, PRI_sComp);
			psSubComp    = psComp->PRI_psSubCompHead;
			switch (CommComp_GetCommType(psComp))
			{
				case COMMTYPE_CAN2:
				{
					while (NULL != psSubComp)
					{
						psTemperCardInfo        = ContainerOf(psSubComp, CommTemperCardInfo_S, PRI_sSubComp);
						psSubComp->PRO_pvUnique = &psTemperCardInfo->PRI_sCANOpenUnique;
						psSubComp               = psSubComp->PRI_psNextSubComp;
					}
					psComp->PRO_pvUnique = &psTemperCard->PRI_sCANOpenUnique;
					SCANOpenCompUnique_InitConfigInfo(&psTemperCard->PRI_sCANOpenUnique, TempCardConfigSDOMsg_12);
					SCANOpenCompUnique_InitEventCallBack(&psTemperCard->PRI_sCANOpenUnique, CANOpen_EventCallBack_12);
					CANOpenMaster_ResetComm(&psTemperCard->PRI_sCANOpenUnique);
					break;
				}

				case COMMTYPE_SUM:
				{
					while (NULL != psSubComp)
					{
						psSubComp->PRO_pvUnique = NULL;
						psSubComp               = psSubComp->PRI_psNextSubComp;
					}
					psComp->PRO_pvUnique = NULL;
					SCommComp_InitCustomFunc(psComp, NULL);
					break;
				}

				default:
				{
					DEBUG_ASSERT(false);
					break;
				}
			}
			wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE);
			wLinkState &= ~((WORD)1 << (psTemperCard - g_asCommTemperCard));
			DB_SetWORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE, wLinkState);
			break;
		}

		default:
			break;
	}
}
/**
 * [CustomFunc_CANOpen_Operation description]
 * @param psComp [description]
 * @param eType  [description]
 */
static void CustomFunc_CANOpen_Operation(CommComp_S* psComp, E_CustomFuncType eType)
{
	CommTemperCard_S* psTemperCard;
	TemperCardNum_E   eTemperCardNum;
	WORD              wTmp;

	DEBUG_ASSERT(NULL != psComp);
	psTemperCard   = ContainerOf(psComp, CommTemperCard_S, PRI_sComp);
	eTemperCardNum = (TemperCardNum_E)(psTemperCard - g_asCommTemperCard);
	DEBUG_ASSERT(eTemperCardNum <= TEMPERCARDNUM_MAX);
	switch (eType)
	{
		case CUSTOMFUNCTYPE_FAST:
		{
			// THJ 2015-06-23 Modify:
			// if (0 == psTemperCard->PUB_wPDOSendCycle)
			//   ResetTimer(&psTemperCard->PRI_dwPDOSendTime);
			// else if (CheckTimerOut(&psTemperCard->PRI_dwPDOSendTime, psTemperCard->PUB_wPDOSendCycle))
			if (CheckTimerOut(&psTemperCard->PRI_dwPDOSendTime))
			//)
			{
				wTmp = GetHeatOutput(eTemperCardNum);
				//wTmp = DB_GetWORD(TEMPERATURE_CARD1_CONFIG_HEAT_COMMAND + (TEMPER_ONECARD_DATANBR * eTemperCardNum));
				CANOpenMaster_SendPDO(CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp)), psTemperCard->PRI_sCANOpenUnique.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, 2, &wTmp);
				ResetTimer(&psTemperCard->PRI_dwPDOSendTime, TEMPCARD_PDO_COMMCYCLE);
			}
			break;
		}

		case CUSTOMFUNCTYPE_SLOW:
		{
			if (CheckTimerOut(&psTemperCard->PRI_dwOfflineCtrlTime))
			{
				CANOpenMaster_ResetComm(&psTemperCard->PRI_sCANOpenUnique);
				return;
			}
			else if (CheckTimerOut(&psTemperCard->PRI_dwNodeGruadTime))
			{
				CANOpenMaster_SendNodeGruading(CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp)), psTemperCard->PRI_sCANOpenUnique.PRO_bNodeID);
				// THJ 2015-05-06 Modify(10 Line):
				// psTemperCard->PRI_bReqNodeGruadState = TRUE;
				if (psTemperCard->PRI_bNodeGruadErrorCount >= NODEGRUADING_ERRCOUNT_MAX)
				{
					CANOpenMaster_ResetComm(&psTemperCard->PRI_sCANOpenUnique);
					Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_TEMPCARD_NODEIDCONFLICT, psTemperCard->PRI_sCANOpenUnique.PRO_bNodeID); // THJ 2017-1-13 Modify.
					return;
				}
				else if (psTemperCard->PRI_bRespNodeGruadCount > 1)
					++psTemperCard->PRI_bNodeGruadErrorCount;
				else
					psTemperCard->PRI_bNodeGruadErrorCount = 0;
				psTemperCard->PRI_bRespNodeGruadCount = 0;
				ResetTimer(&psTemperCard->PRI_dwNodeGruadTime, NODEGRUADING_REQ_CYCLE);
			}
			if (CheckTimerOut(&psTemperCard->PRI_dwReadColdTemperTimer))
			{
				ResetTimer(&psTemperCard->PRI_dwReadColdTemperTimer, COLDTEMPER_REQ_CYCLE);
				DB_GetDataBySysIndex(DBINDEX_TEMPERCARD1_CONFIG_ROOM_TEMPERATURE + TEMPER_ONECARD_DATANBR * eTemperCardNum, &wTmp); // THJ 2015-7-7 Add:
			}
			//if (CheckTimerOut(&u_dwOilCompensateCalcTime))
			//{
			//	ResetTimer(&u_dwOilCompensateCalcTime, OILCOMPENSATE_CALC_CYCLE >> 1);
			//	u_blOilCompensateType = !u_blOilCompensateType;
			//	OilCompensate_Run((u_blOilCompensateType) ? &g_sMoldOpenOilCompensate : &g_sMoldCloseOilCompensate);
			//}
			//hankin 20181130 mask: for new autopid.
			// if (!u_bAutoMode != !g_MoldPDValve.bit.bAutoMode)
			// {
			// 	DBIndex_T iIndex;
			// 	unsigned  i;

			// 	u_bAutoMode = g_MoldPDValve.bit.bAutoMode;
			// 	for (i = 0; i < 96; ++i)
			// 	{
			// 		iIndex = DBINDEX_TEMPER1_INTERFACE_SEG1_SET_ENHANCE_RATIO + TEMPER_ONESEG_DATANBR * i;
			// 		DB_SetDataBySysIndex(iIndex, DB_GetDataAddr(iIndex));
			// 	}
			// }
			break;
		}

		default:
			break;
	}
}
/**
 * [CANOpen_EventCallBack_12 description]
 * @param psCompUnique [description]
 * @param psDataFrame  [description]
 * @param eEventType   [description]
 */
static void CANOpen_EventCallBack_12(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType)
{
	CommTemperCard_S* psTemperCard;
	TemperCardNum_E   eTemperCardNum;
	WORD              i;

	DEBUG_ASSERT(NULL != psCompUnique);
	psTemperCard   = ContainerOf(psCompUnique, CommTemperCard_S, PRI_sCANOpenUnique);
	eTemperCardNum = (TemperCardNum_E)(psTemperCard - g_asCommTemperCard);
	DEBUG_ASSERT(eTemperCardNum < TEMPERCARDNUM_SUM);
	switch (eEventType)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;
				WORD*           pwData;
				WORD            wTemperSeg;

				DEBUG_ASSERT(NULL != psDataFrame);
				eFuncID    = CANOpen_GetFuncIDByCOBID(psDataFrame->uHead.sStand.bCOBID);
				wTemperSeg = psTemperCard->PRI_xSegStart;
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO4:
						wTemperSeg += 4;

					case CANOPENFUNCID_SEND_PDO3:
						wTemperSeg += 4;

					case CANOPENFUNCID_SEND_PDO2:
					{
						if (wTemperSeg >= 96)
						{
							ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, TEMPCARD_OFFLINE_CTRLTIME);
							break;
						}
						pwData = (WORD*)&psDataFrame->uData;
						for (i = 0; i < 4; ++i)
						{
							DB_SetWORD(DBINDEX_TEMPER1_INTERFACE_SEG1_REAL_AD + (TEMPER_ONESEG_DATANBR * wTemperSeg), *pwData);
							TemperCommon_CalCurrent(wTemperSeg, DB_GetWORD(DBINDEX_TEMPERCARD1_CONFIG_ROOM_TEMPERATURE + (eTemperCardNum * TEMPER_ONECARD_DATANBR)));
							++wTemperSeg;
							++pwData;
						}
						ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, TEMPCARD_OFFLINE_CTRLTIME);
						break;
					}

					case CANOPENFUNCID_NMT_ERROR_CTRL:
					{
						// THJ 2015-05-06 Modify(2 Line):
						// if (CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
						// {
						//  if (psTemperCard->PRI_bReqNodeGruadState)
						//      psTemperCard->PRI_bReqNodeGruadState = FALSE;
						//  else
						//  {
						//      CANOpenMaster_ResetComm(&psTemperCard->PRI_sCANOpenUnique);
						//      DB_SetDWORD(MASTER_STATE_ERROR1, DB_MakeError(DB_DEVICEID_TEMPERATURECARD1 + eTemperCardNum, psCompUnique->PRO_bNodeID, ERROR_ID_TEMPCARD_NODEIDCONFLICT));
						//      Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_TEMPCARD_NODEIDCONFLICT);
						//      //MC_WriteError(APPDEVICE_COMMCHIP, psTemperCard->PRI_sCANOpenUnique.PRO_bNodeID, ERROR_ID_TEMPCARD_NODEIDCONFLICT);
						//  }
						// }
						if (psTemperCard->PRI_bRespNodeGruadCount < 3)
							++psTemperCard->PRI_bRespNodeGruadCount;
						break;
					}

					case CANOPENFUNCID_SEND_SDO:
					{
						switch (psCompUnique->PRO_bNodeState)
						{
							case CANOPENNODESTATE_OPERATIONAL:
								break;
							default:
							{
								if (!CheckTimerOut(&psTemperCard->_dwPreAlarmCtrlTimer))
								{
									CANOpenSDOCmdSpecifier_U uSDOCmdSpecifier;

									uSDOCmdSpecifier.tAll = psDataFrame->uData.sSDO.bCmdSpecifier;
									if (CANOPENSDOSCS_ADT == uSDOCmdSpecifier.sBitField.bCmd)
									{
										//Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_PICKER_ODOBJ_NOTEXSITE);
										CANOpenMaster_ResetComm(psCompUnique);
										break;
									}
								}
								{
									ODID_T tODID;
									BOOL   bErr = FALSE;
									WORD   wErrCode;

									tODID = ((psDataFrame->uData.sSDO.bODID_H << 8) | (psDataFrame->uData.sSDO.bODID_L));
									if ((0x1000 == tODID) && (0x00001004 != psDataFrame->uData.sSDO.bData))
									{
										//(THJ 2015-06-18 Modify:
										// DB_SetDWORD(MASTER_STATE_ERROR1, DB_MakeError(DB_DEVICEID_TEMPERATURECARD1 + eTemperCardNum, psCompUnique->PRO_bNodeID, ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL));
										// Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL);
										bErr     = TRUE;
										wErrCode = ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL;
										//)
									}
									else if (0x1018 == tODID)
									{
										if (0x04 == psDataFrame->uData.sSDO.bODSubIndex)
										{
											if (0x00003004 == psDataFrame->uData.sSDO.bData)
											{
												psTemperCard->PRI_xSegNext = psTemperCard->PRI_xSegStart + 16;
												SCANOpenCompUnique_InitConfigInfo(&psTemperCard->PRI_sCANOpenUnique, TempCardConfigSDOMsg_16B);
												SCANOpenCompUnique_InitEventCallBack(&psTemperCard->PRI_sCANOpenUnique, CANOpen_EventCallBack_16B);
											}
											break;
										}
										if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (0x000005BE != psDataFrame->uData.sSDO.bData))
										    || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (0x10040001 != psDataFrame->uData.sSDO.bData)))
										{
											//(THJ 2015-06-18 Modify:
											// DB_SetDWORD(MASTER_STATE_ERROR1, DB_MakeError(DB_DEVICEID_TEMPERATURECARD1 + eTemperCardNum, psCompUnique->PRO_bNodeID, ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL));
											// Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL);
											bErr     = TRUE;
											wErrCode = ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL;
											//)
										}
									}
									if (bErr)
									{
										Set_CommChipErrorDetail(ACTAXIS_MOLD, wErrCode, psCompUnique->PRO_bNodeID); // THJ 2017-1-13 Modify.
										CANOpenMaster_ResetComm(psCompUnique);
									}
								}
								break;
							}
						}
						break;
					}

					default:
						break;
				}
			}
			break;

		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
			i = DB_GetWORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE);
			if (CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
			{
				i |= ((WORD)1 << eTemperCardNum);
				SCommComp_InitCustomFunc(&psTemperCard->PRI_sComp, CustomFunc_CANOpen_Operation);
				// psTemperCard->PRI_bReqNodeGruadState = TRUE; // THJ 2015-05-06 Mask:
				Comm_SetReadFlagByDBIndex(DBINDEX_TEMPERCARD1_INTERFACE_SOFTWARE_SN + TEMPER_ONECARD_DATANBR * eTemperCardNum);
			}
			else
			{
				i &= ~((WORD)1 << eTemperCardNum);
				SCommComp_InitCustomFunc(&psTemperCard->PRI_sComp, NULL);
			}
			// THJ 2015-05-06 Add(2 Line):
			psTemperCard->PRI_bRespNodeGruadCount  = 0;
			psTemperCard->PRI_bNodeGruadErrorCount = 0;
			DB_SetWORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE, i);
			ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, TEMPCARD_OFFLINE_CTRLTIME);
			ResetTimer(&psTemperCard->PRI_dwNodeGruadTime, NODEGRUADING_REQ_CYCLE);
			ResetTimer(&psTemperCard->PRI_dwPDOSendTime, TEMPCARD_PDO_COMMCYCLE);
			ResetTimer(&psTemperCard->PRI_dwReadColdTemperTimer, COLDTEMPER_REQ_CYCLE);
			ResetTimer(&psTemperCard->_dwPreAlarmCtrlTimer, 1000);
			break;
		}

		default:
		{
			DEBUG_ASSERT(false);
			break;
		}
	}
}
/**
 * [TempCardConfigSDOMsg_12 description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool TempCardConfigSDOMsg_12(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		//tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
		// Device Verification.
		{ 0x1018, 0x04, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
		{ 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0x00001004 },
		{ 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0x000005BE },
		{ 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0x10040001 },
		// Configuration.
		// {0x100C      ,0x00           ,CANCOMMSDOACCESS_DOWNLOAD      ,NODEGRUADING_REQ_CYCLE         },
		// {0x100D      ,0x00           ,CANCOMMSDOACCESS_DOWNLOAD      ,5                              },
		{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
		{ 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 1 },
		{ 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x63000110 },
		{ 0x1801, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1802, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1802, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1803, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1803, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1A01, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
		{ 0x1A02, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
		{ 0x1A03, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
	};

	if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
		return true;
	DEBUG_ASSERT(NULL != psRetSDOMsg);
	*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
	return false;
}

/**
 * [CANOpen_EventCallBack_16B description]
 * @param psCompUnique [description]
 * @param psDataFrame  [description]
 * @param eEventType   [description]
 */
static void CANOpen_EventCallBack_16B(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType)
{
	CommTemperCard_S* psTemperCard;
	TemperCardNum_E   eTemperCardNum;
	WORD              i;

	DEBUG_ASSERT(NULL != psCompUnique);
	psTemperCard   = ContainerOf(psCompUnique, CommTemperCard_S, PRI_sCANOpenUnique);
	eTemperCardNum = (TemperCardNum_E)(psTemperCard - g_asCommTemperCard);
	DEBUG_ASSERT(eTemperCardNum < TEMPERCARDNUM_SUM);
	switch (eEventType)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:;
			{
				E_CANOpenFuncID eFuncID;
				WORD*           pwData;
				WORD            wTemperSeg;

				DEBUG_ASSERT(NULL != psDataFrame);
				eFuncID    = CANOpen_GetFuncIDByCOBID(psDataFrame->uHead.sStand.bCOBID);
				wTemperSeg = psTemperCard->PRI_xSegStart;
				switch (eFuncID)
				{
					case CANOPENFUNCID_SEND_PDO1:
						wTemperSeg += 4;

					case CANOPENFUNCID_SEND_PDO4:
						wTemperSeg += 4;

					case CANOPENFUNCID_SEND_PDO3:
						wTemperSeg += 4;

					case CANOPENFUNCID_SEND_PDO2:
					{
						if (wTemperSeg >= 96)
						{
							ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, TEMPCARD_OFFLINE_CTRLTIME);
							break;
						}
						pwData = (WORD*)&psDataFrame->uData;
						for (i = 0; i < 4; ++i)
						{
							DEBUG_ASSERT_ILLEGAL(DBINDEX_TEMPER1_INTERFACE_SEG1_REAL_AD + (TEMPER_ONESEG_DATANBR * wTemperSeg) < DBINDEX_POWERCARD0_CONFIG_CARD_ID, "index over.");
							DB_SetWORD(DBINDEX_TEMPER1_INTERFACE_SEG1_REAL_AD + (TEMPER_ONESEG_DATANBR * wTemperSeg), *pwData);
							TemperCommon_CalCurrent(wTemperSeg, DB_GetWORD(DBINDEX_TEMPERCARD1_CONFIG_ROOM_TEMPERATURE + (eTemperCardNum * TEMPER_ONECARD_DATANBR)));
							++wTemperSeg;
							++pwData;
						}
						ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, TEMPCARD_OFFLINE_CTRLTIME);
						break;
					}

					case CANOPENFUNCID_NMT_ERROR_CTRL:
					{
						// THJ 2015-05-06 Modify(2 Line):
						// if (CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
						// {
						//  if (psTemperCard->PRI_bReqNodeGruadState)
						//      psTemperCard->PRI_bReqNodeGruadState = FALSE;
						//  else
						//  {
						//      CANOpenMaster_ResetComm(&psTemperCard->PRI_sCANOpenUnique);
						//      DB_SetDWORD(MASTER_STATE_ERROR1, DB_MakeError(DB_DEVICEID_TEMPERATURECARD1 + eTemperCardNum, psCompUnique->PRO_bNodeID, ERROR_ID_TEMPCARD_NODEIDCONFLICT));
						//      Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_TEMPCARD_NODEIDCONFLICT);
						//      //MC_WriteError(APPDEVICE_COMMCHIP, psTemperCard->PRI_sCANOpenUnique.PRO_bNodeID, ERROR_ID_TEMPCARD_NODEIDCONFLICT);
						//  }
						// }
						if (psTemperCard->PRI_bRespNodeGruadCount < 3)
							++psTemperCard->PRI_bRespNodeGruadCount;
						break;
					}

					case CANOPENFUNCID_SEND_SDO:
					{
						switch (psCompUnique->PRO_bNodeState)
						{
							case CANOPENNODESTATE_OPERATIONAL:
								break;
							default:
							{
								if (!CheckTimerOut(&psTemperCard->_dwPreAlarmCtrlTimer))
								{
									CANOpenSDOCmdSpecifier_U uSDOCmdSpecifier;

									uSDOCmdSpecifier.tAll = psDataFrame->uData.sSDO.bCmdSpecifier;
									if (CANOPENSDOSCS_ADT == uSDOCmdSpecifier.sBitField.bCmd)
									{
										//Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_PICKER_ODOBJ_NOTEXSITE);
										CANOpenMaster_ResetComm(psCompUnique);
										break;
									}
								}
								{
									ODID_T tODID;
									BOOL   bErr = FALSE;
									WORD   wErrCode;

									tODID = ((psDataFrame->uData.sSDO.bODID_H << 8) | (psDataFrame->uData.sSDO.bODID_L));
									if (0x1018 == tODID)
									{
										if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (0x000005BE != psDataFrame->uData.sSDO.bData))
										    || ((0x04 == psDataFrame->uData.sSDO.bODSubIndex) && (0x00003004 != psDataFrame->uData.sSDO.bData)))
										{
											bErr     = TRUE;
											wErrCode = ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL;
										}
									}
									if (bErr)
									{
										Set_CommChipErrorDetail(ACTAXIS_MOLD, wErrCode, psCompUnique->PRO_bNodeID);
										CANOpenMaster_ResetComm(psCompUnique);
									}
								}
								break;
							}
						}
						break;
					}

					default:
						break;
				}
			}
			break;

		case CANOPENEVENTTYPE_CHANGE_NODESTATE:
		{
			i = DB_GetWORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE);
			if (CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
			{
				i |= ((WORD)1 << eTemperCardNum);
				SCommComp_InitCustomFunc(&psTemperCard->PRI_sComp, CustomFunc_CANOpen_Operation);
				// psTemperCard->PRI_bReqNodeGruadState = TRUE; // THJ 2015-05-06 Mask:
				Comm_SetReadFlagByDBIndex(DBINDEX_TEMPERCARD1_INTERFACE_SOFTWARE_SN + TEMPER_ONECARD_DATANBR * eTemperCardNum);
			}
			else
			{
				i &= ~((WORD)1 << eTemperCardNum);
				SCommComp_InitCustomFunc(&psTemperCard->PRI_sComp, NULL);
			}
			// THJ 2015-05-06 Add(2 Line):
			psTemperCard->PRI_bRespNodeGruadCount  = 0;
			psTemperCard->PRI_bNodeGruadErrorCount = 0;
			DB_SetWORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE, i);
			ResetTimer(&psTemperCard->PRI_dwOfflineCtrlTime, TEMPCARD_OFFLINE_CTRLTIME);
			ResetTimer(&psTemperCard->PRI_dwNodeGruadTime, NODEGRUADING_REQ_CYCLE);
			ResetTimer(&psTemperCard->PRI_dwPDOSendTime, TEMPCARD_PDO_COMMCYCLE);
			ResetTimer(&psTemperCard->PRI_dwReadColdTemperTimer, COLDTEMPER_REQ_CYCLE);
			ResetTimer(&psTemperCard->_dwPreAlarmCtrlTimer, 1000);
			break;
		}

		default:
		{
			DEBUG_ASSERT(false);
			break;
		}
	}
}
/**
 * [TempCardConfigSDOMsg_16B description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool TempCardConfigSDOMsg_16B(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		//tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
		// Device Verification.
		{ 0x1018, 0x04, CANCOMMSDOACCESS_UPLOAD, 0, 0x00003004 },
		{ 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0x000005BE },
		// Configuration.
		//{ 0x100C , 0x00 , CANCOMMSDOACCESS_DOWNLOAD , NODEGRUADING_REQ_CYCLE },
		//{ 0x100D , 0x00 , CANCOMMSDOACCESS_DOWNLOAD , 5 },
		{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },

		{ 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 1 },
		{ 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x63000110 },

		{ 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1801, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1802, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1802, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1803, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 255 },
		{ 0x1803, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, TEMPCARD_PDO_COMMCYCLE },
		{ 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
		{ 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010D10 },
		{ 0x1A00, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010E10 },
		{ 0x1A00, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010F10 },
		{ 0x1A00, 0x04, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64011010 },
		{ 0x1A01, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
		{ 0x1A02, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
		{ 0x1A03, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
	};

	if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
		return true;
	DEBUG_ASSERT(NULL != psRetSDOMsg);
	*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
	return false;
}
/**
 * [GetHeatOutput description]
 * @method GetHeatOutput
 * @param  eTemperCardNum           [description]
 * @return                          [description]
 */
static WORD GetHeatOutput(TemperCardNum_E eTemperCardNum)
{
	DBIndex_T tDBIndex;
	DWORD     dwMask;
	WORD      wBitOffset;
	WORD      wHeatOutput;

	DEBUG_ASSERT(eTemperCardNum < TEMPERCARDNUM_SUM);
#if 0
    DEBUG_ASSERT(12 == TEMPER_MAX_SEG)
    switch (eTemperCardNum)
    {
        case TEMPERCARDNUM_NO1:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPA);
            break;
        case TEMPERCARDNUM_NO2:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPA) >> 12;
            break;
        case TEMPERCARDNUM_NO3:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPA) >> 24;
            wHeatOutput |= (DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPB) & 0xF) << 8;
            break;
        case TEMPERCARDNUM_NO4:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPB) >> 4;
            break;
        case TEMPERCARDNUM_NO5:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPB) >> 16;
            break;
        case TEMPERCARDNUM_NO6:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPB) >> 28;
            wHeatOutput |= (DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPD) & 0xFF) << 4;
            break;
        case TEMPERCARDNUM_NO7:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPB) >> 8;
            break;
        case TEMPERCARDNUM_NO8:
            wHeatOutput = DB_GetDWORD(MASTER_TEMPERATURE_HEAT_OUTPUT_GROUPC) >> 20;
            break;
        default:
            break;
    }
#else
	dwMask = GetSegNum_TemperCard(eTemperCardNum);
	DEBUG_ASSERT(dwMask <= 32);
	tDBIndex    = DBINDEX_M28_TEMPERATURE_HEAT_OUTPUT_GROUPA + (g_asCommTemperCard[eTemperCardNum].PRI_xSegStart / 32);
	wBitOffset  = g_asCommTemperCard[eTemperCardNum].PRI_xSegStart % 32;
	wHeatOutput = DB_GetDWORD(tDBIndex) >> wBitOffset;
	if ((wBitOffset + dwMask) > 32)
		wHeatOutput |= DB_GetDWORD(tDBIndex + 1) << (32 - wBitOffset);
#endif

	dwMask = ((DWORD)1 << dwMask) - 1;
	return wHeatOutput & dwMask;
}
