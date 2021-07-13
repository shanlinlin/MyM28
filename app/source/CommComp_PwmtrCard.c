/*==============================================================================+
|  Function : powermeter card                                                   |
|  Task     : powermeter card Source File                                       |
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
#include "../include/CommComp_PwmtrCard.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define PWMCARD_SDOREAD_CYCLETIME 1000
#define PWMCARD_OFFLINE_CTRLTIME 3000
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
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void Comm_EventCallBack(CommComp_S * psComp, CommEvent_U * puEvent);

static void CANOpen_CustomFunc_Operation(CommComp_S * psComp, E_CustomFuncType eType);
static void CANOpen_EventCallBack(CANOpenCompUnique_S * psCompUnique, CANOpenDataFrame_S * psDataFrame, E_CANOpenEventType eEventType);
static bool PwmtrCardConfigSDOMsg(CANOpenCompUnique_S * psCompUnique, CANOpenCommSDOMsg_S * psRetSDOMsg, WORD wConfigIndex);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//static
CommPwmtrCardInfo_S u_asPwmtrCardInfo[PWMTRCARDNUM_SUM];
CommPwmtrCard_S     g_asCommPwmtrCard[PWMTRCARDNUM_SUM];
//------------------------------------------------------------------------------
static CommOffsetSeg_S const uc_asReadIndexSeg_Card1[] = {
	// {wDBIndexStart                                      ,wDBIndexEnd                                        }
	{ DBINDEX_POWERCARD1_CONFIG_CARD_ID, DBINDEX_POWERCARD1_INTERFACE_IRMS_RESET },
};
static CommOffsetSeg_S const uc_asWriteIndexSeg_Card1[] = {
	// {wDBIndexStart                                      ,wDBIndexEnd                                        }
	{ DBINDEX_POWERCARD1_INTERFACE_HARDWARE_PN, DBINDEX_POWERCARD1_INTERFACE_SOFTWARE_PN },
	{ DBINDEX_POWERCARD1_INTERFACE_VAR_RESET, DBINDEX_POWERCARD1_INTERFACE_IRMS_RESET },
};

static CommOffsetSeg_S const uc_asReadIndexSeg_Card2[] = {
	// {wDBIndexStart                                      ,wDBIndexEnd                                        }
	{ DBINDEX_POWERCARD2_CONFIG_CARD_ID, DBINDEX_POWERCARD2_INTERFACE_IRMS_RESET },
};
static CommOffsetSeg_S const uc_asWriteIndexSeg_Card2[] = {
	// {wDBIndexStart                                      ,wDBIndexEnd                                        }
	{ DBINDEX_POWERCARD2_INTERFACE_HARDWARE_PN, DBINDEX_POWERCARD2_INTERFACE_SOFTWARE_PN },
	{ DBINDEX_POWERCARD2_INTERFACE_VAR_RESET, DBINDEX_POWERCARD2_INTERFACE_IRMS_RESET },
};

static CommOffsetSeg_S const uc_asReadIndexSeg_Card3[] = {
	// {wDBIndexStart                                      ,wDBIndexEnd                                        }
	{ DBINDEX_POWERCARD3_CONFIG_CARD_ID, DBINDEX_POWERCARD3_INTERFACE_IRMS_RESET },
};
static CommOffsetSeg_S const uc_asWriteIndexSeg_Card3[] = {
	// {wDBIndexStart                                      ,wDBIndexEnd                                        }
	{ DBINDEX_POWERCARD3_INTERFACE_HARDWARE_PN, DBINDEX_POWERCARD3_INTERFACE_SOFTWARE_PN },
	{ DBINDEX_POWERCARD3_INTERFACE_VAR_RESET, DBINDEX_POWERCARD3_INTERFACE_IRMS_RESET },
};
//------------------------------------------------------------------------------
static CommOffsetSeg_S const * uc_apsReadIndexSeg_Card[] = {
	uc_asReadIndexSeg_Card1,
	uc_asReadIndexSeg_Card2,
	uc_asReadIndexSeg_Card3,
};
static WORD const uc_awReadIndexSeg_Card[] = {
	sizeof(uc_asReadIndexSeg_Card1) / sizeof(*uc_asReadIndexSeg_Card1),
	sizeof(uc_asReadIndexSeg_Card2) / sizeof(*uc_asReadIndexSeg_Card2),
	sizeof(uc_asReadIndexSeg_Card3) / sizeof(*uc_asReadIndexSeg_Card3),
};
static CommOffsetSeg_S const * uc_apsWriteIndexSeg_Card[] = {
	uc_asWriteIndexSeg_Card1,
	uc_asWriteIndexSeg_Card2,
	uc_asWriteIndexSeg_Card3,
};
static WORD const uc_awWriteSegInfo_Card[] = {
	sizeof(uc_asWriteIndexSeg_Card1) / sizeof(*uc_asWriteIndexSeg_Card1),
	sizeof(uc_asWriteIndexSeg_Card2) / sizeof(*uc_asWriteIndexSeg_Card2),
	sizeof(uc_asWriteIndexSeg_Card3) / sizeof(*uc_asWriteIndexSeg_Card3),
};
//------------------------------------------------------------------------------
// static CommSegInfo_S                             u_asReadSegInfo[] =
// {//  {pasIndexSeg                    ,bSegSum                                                                            ,bFlag      ,bCFlag     ,bCurSeg    }
//  {uc_asReadIndexSeg_Card1    ,sizeof(uc_asReadIndexSeg_Card1) / sizeof(*uc_asReadIndexSeg_Card1)         ,FALSE      ,FALSE      ,0          },
//  {uc_asReadIndexSeg_Card2    ,sizeof(uc_asReadIndexSeg_Card2) / sizeof(*uc_asReadIndexSeg_Card2)         ,FALSE      ,FALSE      ,0          },
//  {uc_asReadIndexSeg_Card3    ,sizeof(uc_asReadIndexSeg_Card3) / sizeof(*uc_asReadIndexSeg_Card3)         ,FALSE      ,FALSE      ,0          },
// };
// static CommSegInfo_S                             u_asWriteSegInfo[] =
// {//  {pasIndexSeg                    ,bSegSum                                                                            ,bFlag      ,bCFlag     ,bCurSeg    }
//  {uc_asWriteIndexSeg_Card1   ,sizeof(uc_asWriteIndexSeg_Card1) / sizeof(*uc_asWriteIndexSeg_Card1)       ,FALSE      ,FALSE      ,0          },
//  {uc_asWriteIndexSeg_Card2   ,sizeof(uc_asWriteIndexSeg_Card2) / sizeof(*uc_asWriteIndexSeg_Card2)       ,FALSE      ,FALSE      ,0          },
//  {uc_asWriteIndexSeg_Card3   ,sizeof(uc_asWriteIndexSeg_Card3) / sizeof(*uc_asWriteIndexSeg_Card3)       ,FALSE      ,FALSE      ,0          },
// };
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [PwmtrCard_Init description]
 * @method PwmtrCard_Init
 */
void PwmtrCard_Init(void)
{
	CommPwmtrCard_S *     psPwmtrCard;
	CommPwmtrCardInfo_S * psPwmtrCardInfo;
	WORD                  i;

	DEBUG_ASSERT(PWMTRCARDNUM_SUM == (sizeof(uc_apsReadIndexSeg_Card) / sizeof(*uc_apsReadIndexSeg_Card)));
	DEBUG_ASSERT(PWMTRCARDNUM_SUM == (sizeof(uc_awReadIndexSeg_Card) / sizeof(*uc_awReadIndexSeg_Card)));
	DEBUG_ASSERT(PWMTRCARDNUM_SUM == (sizeof(uc_apsWriteIndexSeg_Card) / sizeof(*uc_apsWriteIndexSeg_Card)));
	DEBUG_ASSERT(PWMTRCARDNUM_SUM == (sizeof(uc_awWriteSegInfo_Card) / sizeof(*uc_awWriteSegInfo_Card)));
	for (i = 0; i < PWMTRCARDNUM_SUM; ++i)
	{
		psPwmtrCardInfo = &u_asPwmtrCardInfo[i];
		SCommPwmtrCardInfo_Init(&u_asPwmtrCardInfo[i]);
		SCommSubComp_InitReadInfo(&psPwmtrCardInfo->PRI_sSubComp, uc_apsReadIndexSeg_Card[i], 0, uc_awReadIndexSeg_Card[i]);
		SCommSubComp_InitWriteInfo(&psPwmtrCardInfo->PRI_sSubComp, uc_apsWriteIndexSeg_Card[i], 0, uc_awWriteSegInfo_Card[i]);

		psPwmtrCard = &g_asCommPwmtrCard[i];
		SCommPwmtrCard_Init(psPwmtrCard);
		SCommComp_InitEventCallBack(&psPwmtrCard->PRI_sComp, Comm_EventCallBack);
#if (DEBUG)
		psPwmtrCard->PRI_sComp.PUB_eCompType = (E_CompType)(COMPTYPE_PWMTRCARD1 + i);
#endif

		SCommComp_AddSubComp(&psPwmtrCard->PRI_sComp, &psPwmtrCardInfo->PRI_sSubComp);
	}
}
/**
 * [SCommPwmtrCardInfo_Init description]
 * @param psCardInfo [description]
 */
void SCommPwmtrCardInfo_Init(CommPwmtrCardInfo_S * psCardInfo)
{
	DEBUG_ASSERT(NULL != psCardInfo);
	SCommSubComp_Init(&psCardInfo->PRI_sSubComp);
	SCANOpenSubCompUnique_Init(&psCardInfo->PRI_sCANOpenUnique);
}
/**
 * [SCommPwmtrCard_Init description]
 * @param psCard [description]
 */
void SCommPwmtrCard_Init(CommPwmtrCard_S * psCard)
{
	DEBUG_ASSERT(NULL != psCard);
	SCommComp_Init(&psCard->PRI_sComp);
	SCANOpenCompUnique_Init(&psCard->PRI_sCANOpenUnique);
}
/**
 * [Comm_EventCallBack description]
 * @param psComp  [description]
 * @param puEvent [description]
 */
static void Comm_EventCallBack(CommComp_S * psComp, CommEvent_U * puEvent)
{
	DEBUG_ASSERT(NULL != puEvent);
	switch (puEvent->eEvent)
	{
		case COMMEVENT_COMP_CHANGE_COMMTYPE:;
			{
				CommSubComp_S *       psSubComp;
				CommPwmtrCard_S *     psPwmtrCard;
				CommPwmtrCardInfo_S * psPwmtrCardInfo;
				WORD                  wLinkState;

				DEBUG_ASSERT(NULL != psComp);
				psPwmtrCard = ContainerOf(psComp, CommPwmtrCard_S, PRI_sComp);
				psSubComp   = psComp->PRI_psSubCompHead;
				switch (CommComp_GetCommType(psComp))
				{
					case COMMTYPE_CAN2:
						while (NULL != psSubComp)
						{
							psPwmtrCardInfo         = ContainerOf(psSubComp, CommPwmtrCardInfo_S, PRI_sSubComp);
							psSubComp->PRO_pvUnique = &psPwmtrCardInfo->PRI_sCANOpenUnique;
							psSubComp               = psSubComp->PRI_psNextSubComp;
						}
						psComp->PRO_pvUnique = &psPwmtrCard->PRI_sCANOpenUnique;
						SCANOpenCompUnique_InitConfigInfo(&psPwmtrCard->PRI_sCANOpenUnique, PwmtrCardConfigSDOMsg);
						SCANOpenCompUnique_InitEventCallBack(&psPwmtrCard->PRI_sCANOpenUnique, CANOpen_EventCallBack);
						CANOpenMaster_ResetComm(&psPwmtrCard->PRI_sCANOpenUnique);
						break;
					case COMMTYPE_SUM:
						while (NULL != psSubComp)
						{
							psSubComp->PRO_pvUnique = NULL;
							psSubComp               = psSubComp->PRI_psNextSubComp;
						}
						psComp->PRO_pvUnique = NULL;
						SCommComp_InitCustomFunc(psComp, NULL);
						break;
					default:
						DEBUG_ASSERT(false);
						break;
				}
				wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_PAMIR_CARD_LINK_STATE);
				wLinkState &= ~((WORD)1 << (psPwmtrCard - g_asCommPwmtrCard));
				DB_SetWORD(DBINDEX_M28_MONITOR_PAMIR_CARD_LINK_STATE, wLinkState);
			}
			break;
		default:
			break;
	}
}
/**
 * [CANOpen_CustomFunc_Operation description]
 * @param psComp [description]
 * @param eType  [description]
 */
static void CANOpen_CustomFunc_Operation(CommComp_S * psComp, E_CustomFuncType eType)
{
	CommPwmtrCard_S * psPwmtrCard;
	// CommSubComp_S *psSubComp;
	// CommIndexSegInfo_S sSegInfo;

	switch (eType)
	{
		case CUSTOMFUNCTYPE_SLOW:
			DEBUG_ASSERT(NULL != psComp);
			psPwmtrCard = ContainerOf(psComp, CommPwmtrCard_S, PRI_sComp);
			DEBUG_ASSERT(CANOPENNODESTATE_OPERATIONAL == psPwmtrCard->PRI_sCANOpenUnique.PRO_bNodeState);
			if (CheckTimerOut(&psPwmtrCard->PRI_dwCycleReadTime))
			{
				DEBUG_ASSERT(NULL != psPwmtrCard->PRI_sComp.PRI_psSubCompHead);
				// psSubComp = psPwmtrCard->PRI_sComp.PRI_psSubCompHead;
				// sSegInfo = *psSubComp->PRO_psReadInfo;
				// DEBUG_ASSERT(NULL != sSegInfo.pasIndexSeg)
				// for (sSegInfo.bCurSeg = 0; sSegInfo.bCurSeg < sSegInfo.bSegSum; ++sSegInfo.bCurSeg)
				// {
				//  // DB_SetRangeReadFlag(sSegInfo.pasIndexSeg[sSegInfo.bCurSeg].tStartIndex
				//  //  , sSegInfo.pasIndexSeg[sSegInfo.bCurSeg].tEndIndex);
				//  Comm_SetRangeReadFlag(&sSegInfo.pasIndexSeg[sSegInfo.bCurSeg]);
				// }
				// SCommSubComp_SetReadFlag(psSubComp);
				SCommComp_SetAllReadFlag(&psPwmtrCard->PRI_sComp);
				ResetTimer(&psPwmtrCard->PRI_dwCycleReadTime, PWMCARD_SDOREAD_CYCLETIME);
			}
			else if (CheckTimerOut(&psPwmtrCard->PRI_dwOfflineCtrlTime))
			{
				CANOpenMaster_ResetComm(&psPwmtrCard->PRI_sCANOpenUnique);
				ResetTimer(&psPwmtrCard->PRI_dwOfflineCtrlTime, PWMCARD_OFFLINE_CTRLTIME);
			}
			break;
		default:
			break;
	}
}
/**
 * [CANOpen_EventCallBack description]
 * @param psCompUnique [description]
 * @param psDataFrame  [description]
 * @param eEventType   [description]
 */
static void CANOpen_EventCallBack(CANOpenCompUnique_S * psCompUnique, CANOpenDataFrame_S * psDataFrame, E_CANOpenEventType eEventType)
{
	CommPwmtrCard_S * psPwmtrCard;
	E_CANOpenFuncID   eFuncID;

	DEBUG_ASSERT(NULL != psCompUnique);
	psPwmtrCard = ContainerOf(psCompUnique, CommPwmtrCard_S, PRI_sCANOpenUnique);
	switch (eEventType)
	{
		case CANOPENEVENTTYPE_RECVED_DATAFRAME:
			DEBUG_ASSERT(NULL != psDataFrame);
			eFuncID = CANOpen_GetFuncIDByCOBID(psDataFrame->uHead.sStand.bCOBID);
			switch (eFuncID)
			{
				case CANOPENFUNCID_NMT_ERROR_CTRL:
					ResetTimer(&psPwmtrCard->PRI_dwOfflineCtrlTime, PWMCARD_OFFLINE_CTRLTIME);
					break;
				default:
					break;
			}
			break;
		case CANOPENEVENTTYPE_CHANGE_NODESTATE:;
			{
				WORD wLinkState;

				wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_PAMIR_CARD_LINK_STATE);
				if (CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
				{
					wLinkState |= (WORD)1 << (psPwmtrCard - g_asCommPwmtrCard);
					SCommComp_InitCustomFunc(&psPwmtrCard->PRI_sComp, CANOpen_CustomFunc_Operation);
				}
				else
				{
					wLinkState &= ~((WORD)1 << (psPwmtrCard - g_asCommPwmtrCard));
					SCommComp_InitCustomFunc(&psPwmtrCard->PRI_sComp, NULL);
				}
				DB_SetWORD(DBINDEX_M28_MONITOR_PAMIR_CARD_LINK_STATE, wLinkState);
				ResetTimer(&psPwmtrCard->PRI_dwCycleReadTime, PWMCARD_SDOREAD_CYCLETIME);
				ResetTimer(&psPwmtrCard->PRI_dwOfflineCtrlTime, PWMCARD_OFFLINE_CTRLTIME);
			}
			break;
		default:
			DEBUG_ASSERT(false);
			break;
	}
}
/**
 * [PwmtrCardConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool PwmtrCardConfigSDOMsg(CANOpenCompUnique_S * psCompUnique, CANOpenCommSDOMsg_S * psRetSDOMsg, WORD wConfigIndex)
{
	const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
		//tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
		{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, PWMCARD_OFFLINE_CTRLTIME / 3 },
	};

	if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
		return true;
	DEBUG_ASSERT(NULL != psRetSDOMsg);
	*psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
	return false;
}
