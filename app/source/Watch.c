/*==============================================================================+
|  Function :                                                                   |
|  Task     :                                                                   |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2015/08/25                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "Watch.h"
#include "CommComp_CANOpenMaster.h"
#include "Error_App.h"
#include "task_ethercatmaster.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define DEFAULT_WATCH_TURNPAGES 0x7
#define WATCH_TIMEOUT_CAN 100
#define WATCH_MAX_TIMEOUT_COUNT 3
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
// static void Run_Watch_EtherCat(const U_WatchType *puWatchType);
// static void Run_Watch_CAN(const U_WatchType *puWatchType);
// static void Run_Watch_Local(const U_WatchType *puWatchType);
static void Run_Watch_EtherCat(S_Watch* const psWatch);
static void Run_Watch_Can(S_Watch* const psWatch);
static void Run_Watch_Local(S_Watch* const psWatch);
static void Run_Watch_Default_PreProc(S_Watch* const psWatch);
static void Run_Watch_Default(S_Watch* const psWatch);
inline DBIndex_T Watch_GetCurAddrIndex(S_Watch* const psWatch);
inline DBIndex_T Watch_GetCurValueIndex(S_Watch* const psWatch);
static BOOL CheckFlagAndUpdate(S_Watch* const psWatch);
inline BOOL GetFlagState(WORD* const apwFlags, WORD wIndex);
inline void SetFlagState(WORD* const apwFlags, WORD wIndex);
inline void ClrFlagState(WORD* const apwFlags, WORD wIndex);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//static DWORD u_dwSDOValue = 0;
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_Watch description]
 */
void Init_Watch(void)
{
}
/**
 * [Run_Watch description]
 */
// void Run_Watch(void)
// {
//     static WatchType_E eCurWatchType = (WatchType_E)0;
//     U_WatchType uWatchType;

//     uWatchType.dwAll = DB_GetDWORD(DBINDEX_LOCAL0_WATCH_TYPE);                 //
//     if (eCurWatchType != uWatchType.sBit.bWatchType)
//     {
//         eCurWatchType = (WatchType_E)uWatchType.sBit.bWatchType;
//         g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize = 0;
//         g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize = 0;
//         g_sWatchInfo.sCAN.sCurReadInfo.bDataSize = 0;
//         g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize = 0;
//         ResetTimer(&g_sWatchInfo.sCAN.dwReadTime);
//         ResetTimer(&g_sWatchInfo.sCAN.dwWriteTime);
//         memset(g_adwWatchReadFlag, 0, sizeof(g_adwWatchReadFlag));
//         memset(g_adwWatchWriteFlag, 0, sizeof(g_adwWatchWriteFlag));
//     }
//     switch (uWatchType.sBit.bWatchType)
//     {
//         case WATCHTYPE_ETHERCAT:
//         {
//             if ((g_ECController.Master.wAppSlaveCount > 0) && (uWatchType.sEtherCat.bSlaveIndex < g_ECController.Master.wAppSlaveCount))
//             {
//                 if (ETHCRCAT_CONFIG_SUCCESS == g_ECController.Master.wECState)
//                 {
//                     Run_Watch_EtherCat(&uWatchType);
//                 }
//                 else
//                 {
//                     Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_NOTREADY);    // Anders 2015-11-24, mark.
//                 }
//             }
//             break;
//         }

//         case WATCHTYPE_CAN:
//         {
//             if (uWatchType.sCAN.bChannel < CANCHANNAL_SUM)
//             {
//                 Run_Watch_CAN(&uWatchType);
//             }
//             break;
//         }

//         case WATCHTYPE_LOCALMEM:
//         {
//             Run_Watch_Local(&uWatchType);
//             break;
//         }

//         default:
//             break;
//     }
// }

/**
 * [Watch_Init description]
 * @param psWatch [description]
 */
void Watch_Init(S_Watch* psWatch)
{
	DEBUG_ASSERT(NULL != psWatch);
	psWatch->_Run               = Run_Watch_Default;
	psWatch->uWatchType._all    = 0;
	psWatch->uWatchTypeNew._all = 0;
	psWatch->_sODWay._bDataSize = 0;
	psWatch->_sODWay._bSubIndex = 0;
	psWatch->_sODWay._bIndex    = 0;
	psWatch->_dwValue           = 0;
	ResetTimer(&psWatch->_dwAccessTime, 0);
	psWatch->_bMaxTimeoutCount = 0;
	psWatch->wWatchTypeIndex   = DBINDEX_SUM;
	psWatch->_wBackIndex       = 0;
	psWatch->_wCurReadIndex    = WATCH_FLAGSUM;
	psWatch->_wCurIndex        = 0;
	psWatch->_bOperatFinish    = TRUE;
	psWatch->_bSumWriteIndex   = WATCH_FLAGSUM_WRITE;
	psWatch->_bSumIndex        = WATCH_FLAGSUM;
	psWatch->_bTurnPages       = 0;
	memset(psWatch->_awAccessFlag, 0, sizeof(psWatch->_awAccessFlag));
}

/**
 * [Watch_Init_Free description]
 * @param psWatch        [description]
 * @param wSumWriteIndex [description]
 * @param wSumReadIndex  [description]
 */
void Watch_Init_Free(S_Watch* psWatch, WORD wSumWriteIndex, WORD wSumReadIndex)
{
	DEBUG_ASSERT(NULL != psWatch);
	Watch_Init(psWatch);
	psWatch->_bSumWriteIndex = wSumWriteIndex;
	psWatch->_bSumIndex      = wSumWriteIndex + wSumReadIndex;
	psWatch->_wCurReadIndex  = psWatch->_bSumIndex;
}

/**
 * [Watch_Run description]
 * @param psWatch [description]
 */
void Watch_Run(S_Watch* psWatch)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(NULL != psWatch->_Run);
	if (psWatch->_bOperatFinish)
	{
		if (psWatch->uWatchType._all != psWatch->uWatchTypeNew._all)
		{
			psWatch->uWatchType._all = psWatch->uWatchTypeNew._all;
			psWatch->_bOperatFinish  = FALSE;
			psWatch->_Run            = Run_Watch_Default_PreProc;
		}
		else if (CheckFlagAndUpdate(psWatch))
			return;
	}
	psWatch->_Run(psWatch);
}

/**
 * [Watch_Parser_Can description]
 * @param psWatch [description]
 * @param psData  [description]
 */
void Watch_Parser_Can(S_Watch* const psWatch, CANOpenDataFrame_S const* const psData)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(NULL != psData);
	if ((WATCHTYPE_CAN == psWatch->uWatchType._bType) && (!psWatch->_bOperatFinish) && (CANOpen_GetNodeIDByCOBID(psData->uHead.sStand.bCOBID) == psWatch->uWatchType._sCan._bNodeID))
	{
		CANOpenSDOCmdSpecifier_U uSDOCmd;

		uSDOCmd.tAll = psData->uData.sSDO.bCmdSpecifier;
		switch (uSDOCmd.sBitField.bCmd)
		{
		case CANOPENSDOSCS_IDD:
		{
			if ((psWatch->_sODWay._bIndex == ((psData->uData.sSDO.bODID_H << 8) | psData->uData.sSDO.bODID_L)) && (psWatch->_sODWay._bSubIndex == psData->uData.sSDO.bODSubIndex))
			{
				ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
				psWatch->_bOperatFinish = TRUE;
			}
			break;
		}
		case CANOPENSDOSCS_IDU:
		{
			if ((psWatch->_sODWay._bIndex == ((psData->uData.sSDO.bODID_H << 8) | psData->uData.sSDO.bODID_L)) && (psWatch->_sODWay._bSubIndex == psData->uData.sSDO.bODSubIndex))
			{
				DB_SetDWORD(Watch_GetCurValueIndex(psWatch), psData->uData.sSDO.bData);
				ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
				psWatch->_bOperatFinish = TRUE;
			}
			break;
		}
		case CANOPENSDOSCS_ADT:
		{
			if ((psWatch->_sODWay._bIndex == ((psData->uData.sSDO.bODID_H << 8) | psData->uData.sSDO.bODID_L)) && (psWatch->_sODWay._bSubIndex == psData->uData.sSDO.bODSubIndex))
			{
				if (psWatch->_wCurIndex < psWatch->_bSumWriteIndex) // Write.
				{
					switch (psData->uData.sSDO.bData)
					{
					case 0x06020000:
					{
						Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE);
						break;
					}
					case 0x06090011:
					{
						Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE);
						break;
					}
					default:
					{
						Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE);
						break;
					}
					}
				}
				ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
				psWatch->_bOperatFinish = TRUE;
			}
		}
		default:
		{
			break;
		}
		}
	}
}

/**
 * [Watch_SetWatchType description]
 * @param psWatch [description]
 * @param puType  [description]
 */
void Watch_SetWatchType(S_Watch* psWatch, U_WatchType* puType)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(NULL != puType);
	if (psWatch->uWatchTypeNew._all == puType->_all) // Anders 2016-7-15, add.
		return;
	psWatch->uWatchTypeNew = *puType;
	// THJ 2016-10-13 Mask.
	//psWatch->_Run = Run_Watch_Default_PreProc;
	//psWatch->_bOperatFinish = FALSE;
}

/**
 * [Watch_WriteIndex description]
 * @param psWatch [description]
 * @param tIndex  [description]
 */
void Watch_WriteIndex(S_Watch* psWatch, DBIndex_T tIndex)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	if (tIndex > psWatch->wWatchTypeIndex)
	{
		WORD wIndex;

		wIndex = (tIndex - psWatch->wWatchTypeIndex - 1) >> 1;
		if (wIndex < psWatch->_bSumWriteIndex)
		{
			if (wIndex < psWatch->_wBackIndex)
				psWatch->_wBackIndex = wIndex;
			psWatch->_bTurnPages     = DEFAULT_WATCH_TURNPAGES;
			SetFlagState(psWatch->_awAccessFlag, wIndex);
		}
	}
}

/**
 * [Watch_ReadIndex description]
 * @param psWatch [description]
 * @param tIndex  [description]
 */
void Watch_ReadIndex(S_Watch* psWatch, DBIndex_T tIndex)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	if (tIndex > psWatch->wWatchTypeIndex)
	{
		WORD wIndex;

		wIndex = (tIndex - psWatch->wWatchTypeIndex - 1) >> 1;
		if ((wIndex >= psWatch->_bSumWriteIndex) && (wIndex < psWatch->_bSumIndex))
		{
			if (wIndex < psWatch->_wBackIndex)
				psWatch->_wBackIndex = wIndex;
			psWatch->_bTurnPages     = DEFAULT_WATCH_TURNPAGES;
			SetFlagState(psWatch->_awAccessFlag, wIndex);
		}
	}
}
//------------------------------------------------------------------------------
/**
 * [Run_Watch_EtherCat description]
 * @param puWatchType [description]
 */
// static void Run_Watch_EtherCat(const U_WatchType *puWatchType)
// {
//     TECMaster *pMaster = &g_ECController.Master;
//     DWORD *padwFlag, dwInfo;
//     WORD i;

//     DEBUG_ASSERT;(NULL != puWatchType)
//     DEBUG_ASSERT;(g_adwWatchWriteFlag == g_sWatchInfo.sEtherCat.padwWriteFlag)
//     DEBUG_ASSERT;(g_sWatchInfo.sEtherCat.wCurWriteIndex < WATCHETHERCAT_WRITEFLAG_SUM)
//     if (0 == g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize)
//     {
//         padwFlag = &g_sWatchInfo.sEtherCat.padwWriteFlag[g_sWatchInfo.sEtherCat.wCurWriteIndex >> 5];
//         if (0 != *padwFlag)
//         {
//             for (i = (g_sWatchInfo.sEtherCat.wCurWriteIndex & 0x1F); i < 0x20; ++i)
//             {
//                 if (*padwFlag & ((DWORD)1 << i))
//                 {
//                     *(DWORD *)&g_sWatchInfo.sEtherCat.sCurWriteInfo = *(DWORD *)DB_GetDataAddr(WATCH_WRITE_START_INDEX(ADDR) + (g_sWatchInfo.sEtherCat.wCurWriteIndex << 1));
//                     if (0 != g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize)
//                     {
//                         g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize = (g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize + 0x7) >> 3;
//                         pMaster->pDownloadSDO(pMaster,
//                                               FALSE,
//                                               puWatchType->sEtherCat.bSlaveIndex,
//                                               g_sWatchInfo.sEtherCat.sCurWriteInfo.bIndex,
//                                               g_sWatchInfo.sEtherCat.sCurWriteInfo.bSubIndex,
//                                               (WORD *)DB_GetDataAddr(WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurWriteIndex << 1)),
//                                               g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize,
//                                               FALSE);

//                         // Anders 2015-11-25, Add.
//                         pMaster->pDownloadSDO(pMaster,
//                                               TRUE,
//                                               puWatchType->sEtherCat.bSlaveIndex,
//                                               g_sWatchInfo.sEtherCat.sCurWriteInfo.bIndex,
//                                               g_sWatchInfo.sEtherCat.sCurWriteInfo.bSubIndex,
//                                               (WORD *)DB_GetDataAddr(WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurWriteIndex << 1)),
//                                               g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize,
//                                               FALSE);
//                         return;
//                         //break;
//                     }
//                     else
//                     {
//                         *padwFlag &= ~((DWORD)1 << i);
//                         if (0 == *padwFlag)
//                         {
//                             break;
//                         }
//                     }
//                 }
//                 ++g_sWatchInfo.sEtherCat.wCurWriteIndex;
//                 if (g_sWatchInfo.sEtherCat.wCurWriteIndex >= WATCHETHERCAT_WRITEFLAG_SUM)
//                 {
//                     g_sWatchInfo.sEtherCat.wCurWriteIndex = 0;
//                     break;
//                 }
//             }
//         }
//         else
//         {
//             g_sWatchInfo.sEtherCat.wCurWriteIndex += 0x20 - (g_sWatchInfo.sEtherCat.wCurWriteIndex & 0x1F);
//             if (g_sWatchInfo.sEtherCat.wCurWriteIndex >= WATCHCAN_WRITEFLAG_SUM)
//             {
//                 g_sWatchInfo.sEtherCat.wCurWriteIndex = 0;
//             }
//         }
//     }
//     else
//     {
//         WORD wRetCode;

//         i = WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurWriteIndex << 1);
//         wRetCode = pMaster->pDownloadSDO(pMaster,
//                                          TRUE,
//                                          puWatchType->sEtherCat.bSlaveIndex,
//                                          g_sWatchInfo.sEtherCat.sCurWriteInfo.bIndex,
//                                          g_sWatchInfo.sEtherCat.sCurWriteInfo.bSubIndex,
//                                          (WORD *)DB_GetDataAddr(i),
//                                          g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize,
//                                          FALSE);
//         switch (wRetCode)
//         {
//             case SDORESULT_SLAVEABORT:
//             {
//                 dwInfo = 0;
//                 DB_GetData(i, &dwInfo);
//                 switch (dwInfo)
//                 {
//                     case 0x06020000:
//                     {
//                         Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE);
//                         break;
//                     }
//                     case 0x06090011:
//                     {
//                         Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE);
//                         break;
//                     }
//                     default:
//                     {
//                         Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE);
//                         break;
//                     }
//                 }
//                 dwInfo = 0;
//                 DB_SetData(i, &dwInfo);
//             }

//             case SDORESULT_TIMEOUT:
//             case SDORESULT_DONE:
//             {
//                 pMaster->pDownloadSDO(pMaster,
//                                       FALSE,
//                                       puWatchType->sEtherCat.bSlaveIndex,
//                                       g_sWatchInfo.sEtherCat.sCurWriteInfo.bIndex,
//                                       g_sWatchInfo.sEtherCat.sCurWriteInfo.bSubIndex,
//                                       (WORD *)DB_GetDataAddr(WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurWriteIndex << 1)),
//                                       g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize,
//                                       FALSE);
//                 padwFlag = &g_sWatchInfo.sEtherCat.padwWriteFlag[g_sWatchInfo.sEtherCat.wCurWriteIndex >> 5];
//                 *padwFlag &= ~((DWORD)1 << (g_sWatchInfo.sEtherCat.wCurWriteIndex & 0x1F));
//                 g_sWatchInfo.sEtherCat.sCurWriteInfo.bDataSize = 0;
//                 break;
//             }

//             default:
//                 break;
//         }
//         return; // Anders 2015-11-25, Add.
//     }

//     DEBUG_ASSERT;(g_adwWatchReadFlag == g_sWatchInfo.sEtherCat.padwReadFlag)
//     DEBUG_ASSERT;(g_sWatchInfo.sEtherCat.wCurReadIndex < WATCHETHERCAT_READFLAG_SUM)
//     if (0 == g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize)
//     {
//         padwFlag = &g_sWatchInfo.sEtherCat.padwReadFlag[g_sWatchInfo.sEtherCat.wCurReadIndex >> 5];
//         if (0 != *padwFlag)
//         {
//             for (i = (g_sWatchInfo.sEtherCat.wCurReadIndex & 0x1F); i < 0x20; ++i)
//             {
//                 if (*padwFlag & ((DWORD)1 << i))
//                 {
//                     *(DWORD *)&g_sWatchInfo.sEtherCat.sCurReadInfo = *(DWORD *)DB_GetDataAddr(WATCH_READ_START_INDEX(ADDR) + (g_sWatchInfo.sEtherCat.wCurReadIndex << 1));
//                     if (0 != g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize)
//                     {
//                         g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize = (g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize + 0x7) >> 3;
//                         pMaster->pUploadSDO(pMaster,
//                                             FALSE,
//                                             puWatchType->sEtherCat.bSlaveIndex,
//                                             g_sWatchInfo.sEtherCat.sCurReadInfo.bIndex,
//                                             g_sWatchInfo.sEtherCat.sCurReadInfo.bSubIndex,
//                                             (WORD*)&u_dwSDOValue, // (WORD *)DB_GetDataAddr(WATCH_READ_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurReadIndex << 1)
//                                             g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize,
//                                             FALSE);

//                         // Anders 2015-11-25, Add.
//                         u_dwSDOValue = 0;
//                         pMaster->pUploadSDO(pMaster,
//                                             TRUE,
//                                             puWatchType->sEtherCat.bSlaveIndex,
//                                             g_sWatchInfo.sEtherCat.sCurReadInfo.bIndex,
//                                             g_sWatchInfo.sEtherCat.sCurReadInfo.bSubIndex,
//                                             (WORD*)&u_dwSDOValue, // (WORD *)DB_GetDataAddr(WATCH_READ_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurReadIndex << 1),
//                                             g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize,
//                                             FALSE);

//                         break;
//                     }
//                     else
//                     {
//                         *padwFlag &= ~((DWORD)1 << i);
//                         if (0 == *padwFlag)
//                         {
//                             break;
//                         }
//                     }
//                 }
//                 ++g_sWatchInfo.sEtherCat.wCurReadIndex;
//                 if (g_sWatchInfo.sEtherCat.wCurReadIndex >= WATCHETHERCAT_READFLAG_SUM)
//                 {
//                     g_sWatchInfo.sEtherCat.wCurReadIndex = 0;
//                     break;
//                 }
//             }
//         }
//         else
//         {
//             g_sWatchInfo.sEtherCat.wCurReadIndex += 0x20 - (g_sWatchInfo.sEtherCat.wCurReadIndex & 0x1F);
//             if (g_sWatchInfo.sEtherCat.wCurReadIndex >= WATCHCAN_READFLAG_SUM)
//             {
//                 g_sWatchInfo.sEtherCat.wCurReadIndex = 0;
//             }
//         }
//     }
//     else
//     {
//         WORD wRetCode;

//         i = WATCH_READ_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurReadIndex << 1);
//         wRetCode = pMaster->pUploadSDO(pMaster ,
//                                        TRUE ,
//                                        puWatchType->sEtherCat.bSlaveIndex ,
//                                        g_sWatchInfo.sEtherCat.sCurReadInfo.bIndex ,
//                                        g_sWatchInfo.sEtherCat.sCurReadInfo.bSubIndex ,
//                                        (WORD*)&u_dwSDOValue,   //(WORD *)DB_GetDataAddr(i),
//                                        g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize ,
//                                        FALSE);
//         switch (wRetCode)
//         {
//             case SDORESULT_SLAVEABORT:
//             //{     // Anders 2015-11-27, mark for HT.
//             //    switch ( u_dwSDOValue ) // (dwInfo)
//             //    {
//             //        case 0x06020000:
//             //        {
//             //            Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE);
//             //            break;
//             //        }
//             //        case 0x06090011:
//             //        {
//             //            Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE);
//             //            break;
//             //        }
//             //        default:
//             //        {
//             //            Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_READ);
//             //            break;
//             //        }
//             //    }
//             //}
//             case SDORESULT_TIMEOUT:
//             case SDORESULT_DONE:
//             {
//                 pMaster->pUploadSDO(pMaster,
//                                     FALSE,
//                                     puWatchType->sEtherCat.bSlaveIndex,
//                                     g_sWatchInfo.sEtherCat.sCurReadInfo.bIndex,
//                                     g_sWatchInfo.sEtherCat.sCurReadInfo.bSubIndex,
//                                     (WORD*)&u_dwSDOValue, //(WORD *)DB_GetDataAddr(WATCH_READ_START_INDEX(VALUE) + (g_sWatchInfo.sEtherCat.wCurReadIndex << 1)),
//                                     g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize,
//                                     FALSE);
//                 g_sWatchInfo.sEtherCat.sCurReadInfo.bDataSize = 0;

//                 if ( wRetCode != SDORESULT_TIMEOUT )    // Anders 2015-11-24, Add.
//                 {
//                     memcpy(DB_GetDataAddr(i), &u_dwSDOValue, sizeof(DWORD));    // Anders 2015-11-24, Add.
//                     padwFlag = &g_sWatchInfo.sEtherCat.padwReadFlag[g_sWatchInfo.sEtherCat.wCurReadIndex >> 5];
//                     *padwFlag &= ~((DWORD)1 << (g_sWatchInfo.sEtherCat.wCurReadIndex & 0x1F));
//                     ++g_sWatchInfo.sEtherCat.wCurReadIndex;
//                     if (g_sWatchInfo.sEtherCat.wCurReadIndex >= WATCHETHERCAT_READFLAG_SUM)
//                     {
//                         g_sWatchInfo.sEtherCat.wCurReadIndex = 0;
//                     }
//                 }
//                 break;
//             }

//             default:
//                 break;
//         }
//     }
// }
// /**
//  * [Run_Watch_CAN description]
//  * @param puWatchType [description]
//  */
// static void Run_Watch_CAN(const U_WatchType *puWatchType)
// {
//     DWORD *padwFlag;
//     WORD i;
//     static  WORD wReadCNT, wWriteCNT;
//     CANOpenCommSDOMsg_S sSDOMsg;
//     // Write data to can.
//     if (0 == g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize)
//     {
//         DEBUG_ASSERT;(g_adwWatchWriteFlag == g_sWatchInfo.sCAN.padwWriteFlag)
//         DEBUG_ASSERT;(g_sWatchInfo.sCAN.wCurWriteIndex < WATCHCAN_WRITEFLAG_SUM)
//         padwFlag = &g_sWatchInfo.sCAN.padwWriteFlag[g_sWatchInfo.sCAN.wCurWriteIndex >> 5];
//         if (0 != *padwFlag)
//         {
//             for (i = (g_sWatchInfo.sCAN.wCurWriteIndex & 0x1F); i < 0x20; ++i)
//             {
//                 if (*padwFlag & ((DWORD)1 << i))
//                 {
//                     *(DWORD*)&g_sWatchInfo.sCAN.sCurWriteInfo = DB_GetDWORD(WATCH_WRITE_START_INDEX(ADDR) + (g_sWatchInfo.sCAN.wCurWriteIndex << 1));
//                     if (0 != g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize)
//                     {
//                         // g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize = (g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize + 0x7) >> 3;
//                         sSDOMsg.tODID = g_sWatchInfo.sCAN.sCurWriteInfo.bIndex;
//                         sSDOMsg.bODSubIndex = g_sWatchInfo.sCAN.sCurWriteInfo.bSubIndex;
//                         sSDOMsg.bSDOAccess = CANCOMMSDOACCESS_DOWNLOAD;
//                         sSDOMsg.dwValue = DB_GetDWORD(WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sCAN.wCurWriteIndex << 1));
//                         DEBUG_ASSERT;(NULL != puWatchType)
//                         CANOpenMaster_SendSDO((E_CANChannel)puWatchType->sCAN.bChannel, puWatchType->sCAN.bNodeID, &sSDOMsg);
//                         ResetTimer(&g_sWatchInfo.sCAN.dwWriteTime);
//                         wWriteCNT = 0; // Anders 2015-11-25, Add.
//                         break;
//                     }
//                     else
//                     {
//                         *padwFlag &= ~((DWORD)1 << i);
//                         if (0 == *padwFlag)
//                         {
//                             break;
//                         }
//                     }
//                 }
//                 ++g_sWatchInfo.sCAN.wCurWriteIndex;
//                 if (g_sWatchInfo.sCAN.wCurWriteIndex >= WATCHCAN_WRITEFLAG_SUM)
//                 {
//                     g_sWatchInfo.sCAN.wCurWriteIndex = 0;
//                     break;
//                 }
//             }
//         }
//     }
//     else if (CheckTimerOut(&g_sWatchInfo.sCAN.dwWriteTime, WATCHCAN_WRITE_WAITTIME))
//     {
//         // Anders 2015-11-25, Add.
//         ResetTimer(&g_sWatchInfo.sCAN.dwWriteTime);
//         if ( wWriteCNT++ > 5 )
//         {
//             wWriteCNT = 0;
//             Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE);
//             g_sWatchInfo.sCAN.sCurWriteInfo.bDataSize = 0;
//             padwFlag = &g_sWatchInfo.sCAN.padwWriteFlag[g_sWatchInfo.sCAN.wCurWriteIndex >> 5];
//             *padwFlag &= ~((DWORD)1 << (g_sWatchInfo.sCAN.wCurWriteIndex & 0x1F));
//             ++g_sWatchInfo.sCAN.wCurWriteIndex;
//             if (g_sWatchInfo.sCAN.wCurWriteIndex >= WATCHCAN_WRITEFLAG_SUM)
//             {
//                 g_sWatchInfo.sCAN.wCurWriteIndex = 0;
//             }
//         }
//         else
//         {
//             sSDOMsg.tODID = g_sWatchInfo.sCAN.sCurWriteInfo.bIndex;
//             sSDOMsg.bODSubIndex = g_sWatchInfo.sCAN.sCurWriteInfo.bSubIndex;
//             sSDOMsg.bSDOAccess = CANCOMMSDOACCESS_DOWNLOAD;
//             sSDOMsg.dwValue = DB_GetDWORD(WATCH_WRITE_START_INDEX(VALUE) + (g_sWatchInfo.sCAN.wCurWriteIndex << 1));
//             CANOpenMaster_SendSDO((E_CANChannel)puWatchType->sCAN.bChannel, puWatchType->sCAN.bNodeID, &sSDOMsg);
//         }
//     }

//     // Read data by can.
//     if (0 == g_sWatchInfo.sCAN.sCurReadInfo.bDataSize)
//     {
//         DEBUG_ASSERT;(g_adwWatchReadFlag == g_sWatchInfo.sCAN.padwReadFlag)
//         DEBUG_ASSERT;(g_sWatchInfo.sCAN.wCurReadIndex < WATCHCAN_READFLAG_SUM)
//         padwFlag = &g_sWatchInfo.sCAN.padwReadFlag[g_sWatchInfo.sCAN.wCurReadIndex >> 5];
//         if (0 != *padwFlag)
//         {
//             for (i = (g_sWatchInfo.sCAN.wCurReadIndex & 0x1F); i < 0x20; ++i)
//             {
//                 if (0 != (*padwFlag & ((DWORD)1 << i)))
//                 {
//                     //CANOpenCommSDOMsg_S sSDOMsg;  // Anders 2015-11-25, mark.

//                     *(DWORD*)&g_sWatchInfo.sCAN.sCurReadInfo = DB_GetDWORD(WATCH_READ_START_INDEX(ADDR) + (g_sWatchInfo.sCAN.wCurReadIndex << 1));
//                     if (0 != g_sWatchInfo.sCAN.sCurReadInfo.bDataSize)
//                     {
//                         // g_sWatchInfo.sCAN.sCurReadInfo.bDataSize = (g_sWatchInfo.sCAN.sCurReadInfo.bDataSize + 0x7) >> 3;
//                         sSDOMsg.tODID = g_sWatchInfo.sCAN.sCurReadInfo.bIndex;
//                         sSDOMsg.bODSubIndex = g_sWatchInfo.sCAN.sCurReadInfo.bSubIndex;
//                         sSDOMsg.bSDOAccess = CANCOMMSDOACCESS_UPLOAD;
//                         sSDOMsg.dwValue = 0;
//                         DEBUG_ASSERT;(NULL != puWatchType)
//                         CANOpenMaster_SendSDO((E_CANChannel)puWatchType->sCAN.bChannel, puWatchType->sCAN.bNodeID, &sSDOMsg);
//                         ResetTimer(&g_sWatchInfo.sCAN.dwReadTime);
//                         wReadCNT = 0; // Anders 2015-11-25, Add.
//                         break;
//                     }
//                     else
//                     {
//                         *padwFlag &= ~((DWORD)1 << i);
//                         if (0 == *padwFlag)
//                         {
//                             break;
//                         }
//                     }
//                 }
//                 ++g_sWatchInfo.sCAN.wCurReadIndex;
//                 if (g_sWatchInfo.sCAN.wCurReadIndex >= WATCHCAN_READFLAG_SUM)
//                 {
//                     g_sWatchInfo.sCAN.wCurReadIndex = 0;
//                     break;
//                 }
//             }
//         }
//         else
//         {
//             g_sWatchInfo.sCAN.wCurReadIndex += 0x20 - (g_sWatchInfo.sCAN.wCurReadIndex & 0x1F);
//             if (g_sWatchInfo.sCAN.wCurReadIndex >= WATCHCAN_READFLAG_SUM)
//             {
//                 g_sWatchInfo.sCAN.wCurReadIndex = 0;
//             }
//         }
//     }
//     else if (CheckTimerOut(&g_sWatchInfo.sCAN.dwReadTime, WATCHCAN_READ_WAITTIME))
//     {
//         // Anders 2015-11-25, add.
//         ResetTimer(&g_sWatchInfo.sCAN.dwReadTime);
//         if ( wReadCNT++ > 5)
//         {
//             wReadCNT = 0;
//             Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_READ);
//             g_sWatchInfo.sCAN.sCurReadInfo.bDataSize = 0;
//             padwFlag = &g_sWatchInfo.sCAN.padwReadFlag[g_sWatchInfo.sCAN.wCurReadIndex >> 5];
//             *padwFlag &= ~((DWORD)1 << (g_sWatchInfo.sCAN.wCurReadIndex & 0x1F));
//             ++g_sWatchInfo.sCAN.wCurReadIndex;
//             if (g_sWatchInfo.sCAN.wCurReadIndex >= WATCHCAN_READFLAG_SUM)
//             {
//                 g_sWatchInfo.sCAN.wCurReadIndex = 0;
//             }
//         }
//         else
//         {
//             sSDOMsg.tODID = g_sWatchInfo.sCAN.sCurReadInfo.bIndex;
//             sSDOMsg.bODSubIndex = g_sWatchInfo.sCAN.sCurReadInfo.bSubIndex;
//             sSDOMsg.bSDOAccess = CANCOMMSDOACCESS_UPLOAD;
//             sSDOMsg.dwValue = 0;
//             CANOpenMaster_SendSDO((E_CANChannel)puWatchType->sCAN.bChannel, puWatchType->sCAN.bNodeID, &sSDOMsg);
//         }
//     }
// }

//
static void Run_Watch_EtherCat(S_Watch* const psWatch)
{
	//static	BOOL	bCompleteAccess = FALSE;//shanll 20201110 add
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(psWatch->_wCurIndex < psWatch->_bSumIndex);
	if (ETHCRCAT_CONFIG_SUCCESS == g_ECController.Master.wECState)
	{
		if (psWatch->_bOperatFinish) // Initialize read and write data.
		{
			/*if ( !(psWatch->_sODWayOld._bIndex == g_ECController.Slave[0].m_wUploadSdoIndex  &&  psWatch->_sODWayOld._bSubIndex == g_ECController.Slave[0].m_wUploadSdoSubIndex) )
            {
                volatile int iBool = 0;
                while (!iBool)
                    asm(" ESTOP0");
            }
            */
			*(DWORD*)&psWatch->_sODWay = DB_GetDWORD(Watch_GetCurAddrIndex(psWatch));

			if (0 == psWatch->_sODWay._bDataSize)
			{
				ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
				return;
			}
			psWatch->_bMaxTimeoutCount = WATCH_MAX_TIMEOUT_COUNT;
			psWatch->_bOperatFinish    = FALSE;
			{
				//TECMaster *pMaster = &g_ECController.Master;

				psWatch->_sODWay._bDataSize = (psWatch->_sODWay._bDataSize + 0x7) >> 3;
				if (psWatch->_wCurIndex < psWatch->_bSumWriteIndex) // Write.
				{
					psWatch->_dwValue = DB_GetDWORD(Watch_GetCurValueIndex(psWatch));
					//pMaster->pDownloadSDO(
					//    pMaster,
					//    //FALSE, // Anders 2016-6-15,delete bExcute.
					//    psWatch->uWatchType._sEtherCat._bSlaveIndex,
					//    psWatch->_sODWay._bIndex,
					//    psWatch->_sODWay._bSubIndex,
					//    (WORD *)&psWatch->_dwValue,
					//    psWatch->_sODWay._bDataSize,
					//    FALSE);
				}
				else // Read.
				{
					psWatch->_dwValue = 0;
					//pMaster->pUploadSDO(
					//    pMaster,
					//    //FALSE, // Anders 2016-6-15,delete bExcute.
					//    psWatch->uWatchType._sEtherCat._bSlaveIndex,
					//    psWatch->_sODWay._bIndex,
					//    psWatch->_sODWay._bSubIndex,
					//    (WORD *)&psWatch->_dwValue,
					//    psWatch->_sODWay._bDataSize,
					//    FALSE);
				}
			}
		}
		{
			TECMaster* pMaster = &g_ECController.Master;
			WORD       wRetCode;

			if (psWatch->_wCurIndex < psWatch->_bSumWriteIndex) // Write.
			{
				wRetCode = ECMaster_DownloadSDO(
										pMaster,
										// TRUE, // Anders 2016-6-15,delete bExcute.
										psWatch->uWatchType._sEtherCat._bSlaveIndex,
										psWatch->_sODWay._bIndex,
										psWatch->_sODWay._bSubIndex,
										(WORD*)&psWatch->_dwValue,
										psWatch->_sODWay._bDataSize,
										FALSE);
				switch (wRetCode)
				{
				case SDORESULT_SLAVEABORT:
				{
					switch (psWatch->_dwValue)
					{
					case 0x06020000:
					{
						//shanll 20210109 modify
						ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
						psWatch->_sODWayOld     = psWatch->_sODWay;
						psWatch->_bOperatFinish = TRUE;
						// Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE);
						break;
					}
					case 0x06090011:
					{
						//shanll 20210111 modify
						ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
						psWatch->_sODWayOld     = psWatch->_sODWay;
						psWatch->_bOperatFinish = TRUE;
						// Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE);
						break;
					}
					default:
					{
						Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE);
						break;
					}
					}
					psWatch->_dwValue = 0;
					DB_SetDWORD(Watch_GetCurValueIndex(psWatch), psWatch->_dwValue);
					// Do not break, and perform the following tasks.
				}
				case SDORESULT_DONE:
				{
					ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
					psWatch->_sODWayOld     = psWatch->_sODWay;
					psWatch->_bOperatFinish = TRUE;
					break;
				}
				case SDORESULT_TIMEOUT:
				{
					if (psWatch->_bMaxTimeoutCount > 0)
						psWatch->_bMaxTimeoutCount--;
					else
					{
						ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
						psWatch->_sODWayOld     = psWatch->_sODWay;
						psWatch->_bOperatFinish = TRUE;
					}
					break;
				}
				default:
				{
					break;
				}
				}
			}
			else // Read.
			{
				wRetCode = ECMaster_UploadSDO(
				    pMaster,
				    //TRUE, // Anders 2016-6-15,delete bExcute.
				    psWatch->uWatchType._sEtherCat._bSlaveIndex,
				    psWatch->_sODWay._bIndex,
				    psWatch->_sODWay._bSubIndex,
				    (WORD*)&psWatch->_dwValue,
				    psWatch->_sODWay._bDataSize,
				    FALSE);
				switch (wRetCode)
				{
				case SDORESULT_SLAVEABORT:
				{
					psWatch->_dwValue = 0;
					// Do not break, and perform the following tasks.
				}
				case SDORESULT_DONE:
				{
					DB_SetDWORD(Watch_GetCurValueIndex(psWatch), psWatch->_dwValue);
					ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
					psWatch->_sODWayOld     = psWatch->_sODWay;
					psWatch->_bOperatFinish = TRUE;
					break;
				}
				case SDORESULT_TIMEOUT:
				{
					if (psWatch->_bMaxTimeoutCount > 0)
						psWatch->_bMaxTimeoutCount--;
					else
					{
						ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
						psWatch->_sODWayOld     = psWatch->_sODWay;
						psWatch->_bOperatFinish = TRUE;
					}
					break;
				}
				default:
				{
					break;
				}
				}
			}
		}
	}
	else
	{
		//Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_WATCH_NOTREADY);  // hankin 20171108 del: libin. // HT Need Alarm.
		//ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex); // Anders 2016-7-27, mark for HMI start comm, send SDO to J6.
		//psWatch->_sODWayOld = psWatch->_sODWay;
		//psWatch->_bOperatFinish = TRUE;
	}
}

/**
 * [Run_Watch_Can description]
 * @param psWatch [description]
 */
static void Run_Watch_Can(S_Watch* const psWatch)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(psWatch->_wCurIndex < psWatch->_bSumIndex);
	if (psWatch->_bOperatFinish)
	{
		*(DWORD*)&psWatch->_sODWay = DB_GetDWORD(Watch_GetCurAddrIndex(psWatch));
		if (0 == psWatch->_sODWay._bDataSize)
		{
			ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
			return;
		}
		ResetTimer(&psWatch->_dwAccessTime, 0); //"WATCH_TIMEOUT_CAN" Read and write for the first time to think directly overtime.
		psWatch->_bMaxTimeoutCount = WATCH_MAX_TIMEOUT_COUNT;
		psWatch->_bOperatFinish    = FALSE;
	}
	if (CheckTimerOut(&psWatch->_dwAccessTime)) // Time out to read and write again.
	{
		ResetTimer(&psWatch->_dwAccessTime, WATCH_TIMEOUT_CAN);
		if (psWatch->_bMaxTimeoutCount > 0)
		{
			psWatch->_bMaxTimeoutCount--;
			{
				CANOpenCommSDOMsg_S sSDOMsg;

				sSDOMsg.tODID       = psWatch->_sODWay._bIndex;
				sSDOMsg.bODSubIndex = psWatch->_sODWay._bSubIndex;
				if (psWatch->_wCurIndex < psWatch->_bSumWriteIndex) // Write.
				{
					psWatch->_dwValue  = DB_GetDWORD(Watch_GetCurValueIndex(psWatch));
					sSDOMsg.dwValue    = psWatch->_dwValue;
					sSDOMsg.bSDOAccess = CANCOMMSDOACCESS_DOWNLOAD;
				}
				else // Read.
					sSDOMsg.bSDOAccess = CANCOMMSDOACCESS_UPLOAD;
				CANOpenMaster_SendSDO((E_CANChannel)psWatch->uWatchType._sCan._bChannel, psWatch->uWatchType._sCan._bNodeID, &sSDOMsg);
			}
		}
		else
		{
			ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
			psWatch->_bOperatFinish = TRUE;
		}
	}
}

/**
 * [Run_Watch_Local description]
 * @param psWatch [description]
 */
static void Run_Watch_Local(S_Watch* const psWatch)
{
	char* pcSrcAddr;
	char* pcDesAddr;

	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(psWatch->_wCurIndex < psWatch->_bSumIndex);
	if (psWatch->_wCurIndex < psWatch->_bSumWriteIndex) // Write.
	{
#if (DEBUG)
		pcDesAddr = (char*)DB_GetDWORD(Watch_GetCurAddrIndex(psWatch));
		pcSrcAddr = DB_GetDataAddr(Watch_GetCurValueIndex(psWatch));
		memcpy(pcDesAddr, pcSrcAddr, sizeof(DWORD));
#endif
		ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
	}
	else // Read.
	{
		pcDesAddr = DB_GetDataAddr(Watch_GetCurValueIndex(psWatch));
		pcSrcAddr = (char*)DB_GetDWORD(Watch_GetCurAddrIndex(psWatch));
		memcpy(pcDesAddr, pcSrcAddr, sizeof(DWORD));
		ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
	}
}

/**
 * [Run_Watch_Default_PreProc description]
 * @param psWatch [description]
 */
static void Run_Watch_Default_PreProc(S_Watch* const psWatch)
{
	DEBUG_ASSERT(NULL != psWatch);
	psWatch->_Run           = Run_Watch_Default;
	psWatch->_bOperatFinish = TRUE;
	if (psWatch->wWatchTypeIndex < DBINDEX_SUM)
	{
		switch (psWatch->uWatchType._bType)
		{
		case WATCHTYPE_ETHERCAT:
		{
			if (psWatch->uWatchType._sEtherCat._bSlaveIndex < g_ECController.Master.wAppSlaveCount)
			{
				psWatch->_sODWay._bDataSize = 0;
				psWatch->_Run               = Run_Watch_EtherCat;
			}
			break;
		}
		case WATCHTYPE_CAN:
		{
			if ((psWatch->uWatchType._sCan._bChannel < CANCHANNAL_SUM) && (0 != psWatch->uWatchType._sCan._bNodeID))
			{
				psWatch->_sODWay._bDataSize = 0;
				psWatch->_Run               = Run_Watch_Can;
			}
			break;
		}
		case WATCHTYPE_LOCAL:
		{
			psWatch->_Run = Run_Watch_Local;
			break;
		}
		default:
		{
			break;
		}
		}
	}
}

/**
 * [Run_Watch_Default description]
 * @param psWatch [description]
 */
static void Run_Watch_Default(S_Watch* const psWatch)
{
	DEBUG_ASSERT(NULL != psWatch);
	DEBUG_ASSERT(psWatch->_wCurIndex < psWatch->_bSumIndex);
	ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
}

/**
 * [Watch_GetCurAddrIndex description]
 * @param  psWatch [description]
 * @return         [description]
 */
inline DBIndex_T Watch_GetCurAddrIndex(S_Watch* const psWatch)
{
	return (psWatch->wWatchTypeIndex + 1 + (psWatch->_wCurIndex << 1));
}

/**
 * [Watch_GetCurValueIndex description]
 * @param  psWatch [description]
 * @return         [description]
 */
inline DBIndex_T Watch_GetCurValueIndex(S_Watch* const psWatch)
{
	return (psWatch->wWatchTypeIndex + 2 + (psWatch->_wCurIndex << 1));
}

/**
 * [CheckFlagAndUpdate description]
 * @param  psWatch [description]
 * @return         [description]
 */
static BOOL CheckFlagAndUpdate(S_Watch* const psWatch)
{
	WORD i;

	DEBUG_ASSERT(NULL != psWatch);
	for (i = 0; i < psWatch->_bTurnPages; ++i)
	{
		if (psWatch->_wCurIndex < psWatch->_bSumIndex) // THJ 2016-04-22 Modify: "_wBackIndex" to "_wCurIndex"
		{
			if (GetFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex)) // THJ 2016-04-22 Modify: "_wBackIndex" to "_wCurIndex"
			{
				if (psWatch->_wBackIndex < psWatch->_bSumWriteIndex)
				{
					psWatch->_wCurReadIndex = psWatch->_wCurIndex;
					psWatch->_wCurIndex     = psWatch->_wBackIndex;
					psWatch->_wBackIndex    = psWatch->_bSumWriteIndex;
				}
				else if (psWatch->_wCurReadIndex < psWatch->_bSumIndex)
				{
					psWatch->_wCurIndex     = psWatch->_wCurReadIndex;
					psWatch->_wCurReadIndex = psWatch->_bSumIndex;
				}
				if (0 != DB_GetDWORD(Watch_GetCurAddrIndex(psWatch)))
				{
					//psWatch->_wCurIndex = psWatch->_wBackIndex; // THJ 2016-04-22 Mask.
					return FALSE;
				}
				ClrFlagState(psWatch->_awAccessFlag, psWatch->_wCurIndex);
			}
		}
		else
		{
			// THJ 2016-10-13 Modify: for update _CurIndex.{
			//psWatch->_bTurnPages = 0;
			////psWatch->_wBackIndex = 0;                                                         // It's optional.
			if (psWatch->_wBackIndex < psWatch->_bSumIndex)
			{
				psWatch->_wCurIndex  = psWatch->_wBackIndex;
				psWatch->_wBackIndex = psWatch->_bSumIndex;
				return FALSE;
			}
			else
				psWatch->_bTurnPages = 0;
			//}
			return TRUE;
		}
		if (0 == psWatch->_awAccessFlag[psWatch->_wCurIndex >> 4])       // THJ 2016-04-22 Modify: "_wBackIndex" to "_wCurIndex"
			psWatch->_wCurIndex += (0x10 - (psWatch->_wCurIndex & 0xF)); // THJ 2016-04-22 Modify: "_wBackIndex" to "_wCurIndex"
		else
			psWatch->_wCurIndex++; // THJ 2016-04-22 Modify: "_wBackIndex" to "_wCurIndex"
	}
	return TRUE;
}

/**
 * [GetFlagState description]
 * @param  apwFlags [description]
 * @param  wIndex   [description]
 * @return          [description]
 */
inline BOOL GetFlagState(WORD* const apwFlags, WORD wIndex)
{
	DEBUG_ASSERT(NULL != apwFlags);
	return apwFlags[wIndex >> 4] & ((WORD)1 << (wIndex & 0xF));
}

/**
 * [SetFlagState description]
 * @param apwFlags [description]
 * @param wIndex   [description]
 */
inline void SetFlagState(WORD* const apwFlags, WORD wIndex)
{
	DEBUG_ASSERT(NULL != apwFlags);
	apwFlags[wIndex >> 4] |= ((WORD)1 << (wIndex & 0xF));
}

/**
 * [ClrFlagState description]
 * @param apwFlags [description]
 * @param wIndex   [description]
 */
inline void ClrFlagState(WORD* const apwFlags, WORD wIndex)
{
	DEBUG_ASSERT(NULL != apwFlags);
	apwFlags[wIndex >> 4] &= ~((WORD)1 << (wIndex & 0xF));
}
