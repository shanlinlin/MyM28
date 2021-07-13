/*==============================================================================+
|  Function : Communication Component Local                                     |
|  Task     : Communication Component Local Source File                         |
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
#include "../include/CommComp_Local.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
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
static void DBToLocal(CommComp_S * psComp);
static void LocalToDB(CommComp_S * psComp);
static void Default_OperationData(CommComp_S * psComp, CommSubComp_S * psSubComp, DBIndex_T tDBIndex);
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
 * [Run_Local description]
 * @param eCommType [description]
 * @param psComp    [description]
 */
void Run_Local(E_CommType eCommType, CommComp_S * const psComp)
{
	DEBUG_ASSERT(COMMTYPE_LOCAL == eCommType);
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
	switch (CommComp_GetState(psComp))
	{
		case COMPSTATE_INIT:
		case COMPSTATE_CONFIG:
		{
			// SCommComp_SetAllWriteDBFlag(psComp);
			SCommComp_SetState(psComp, COMPSTATE_SYNC);
			break;
		}
		default:
		{
			break;
		}
	}
	switch (psComp->PRO_bSDOState)
	{
		case COMMSDOSTATE_READ:
		{
			LocalToDB(psComp);
			break;
		}
		case COMMSDOSTATE_WRITE:
		{
			DBToLocal(psComp);
			break;
		}
		default:
		{
			return;
		}
	}
}

/**
 * [LocalSubUnique_Init description]
 * @param psSubUnique [description]
 */
void LocalSubUnique_Init(S_LocalSubUnique * const psSubUnique)
{
	DEBUG_ASSERT(NULL != psSubUnique);
	psSubUnique->PRO_fWriteDataByDB = Default_OperationData;
	psSubUnique->PRO_fReadDataToDB  = Default_OperationData;
}

/**
 * [LocalSubUnique_InitWriteDataByDB description]
 * @param psSubUnique [description]
 * @param fWriteDataByDB  [description]
 */
void LocalSubUnique_InitWriteDataByDB(S_LocalSubUnique * const psSubUnique, Local_WriteDataByDB_F fWriteDataByDB)
{
	DEBUG_ASSERT(NULL != psSubUnique);
	psSubUnique->PRO_fWriteDataByDB = (NULL != fWriteDataByDB) ? fWriteDataByDB : Default_OperationData;
}
/**
 * [LocalSubUnique_InitReadDataByDB description]
 * @param psSubUnique [description]
 * @param fReadDataByDB   [description]
 */
void LocalSubUnique_InitReadDataByDB(S_LocalSubUnique * const psSubUnique, Local_ReadDataToDB_F fReadDataByDB)
{
	DEBUG_ASSERT(NULL != psSubUnique);
	psSubUnique->PRO_fReadDataToDB = (NULL != fReadDataByDB) ? fReadDataByDB : Default_OperationData;
}

/**
 * [DBToLocal description]
 * @param psComp [description]
 */
static void DBToLocal(CommComp_S * const psComp)
{
	S_LocalSubUnique * psSubUnique;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(psComp->PRI_tCurIndex < DBINDEX_SUM);
	DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
	psSubUnique = psComp->PRI_psCurSubComp->PRO_pvUnique;
	DEBUG_ASSERT(NULL != psSubUnique);
	DEBUG_ASSERT(NULL != psSubUnique->PRO_fWriteDataByDB);
	psSubUnique->PRO_fWriteDataByDB(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
	// Comm_ClrWriteFlag(psComp->PRI_tCurIndex);
	SCommComp_ClrSDOState(psComp);
}
/**
 * [LocalToDB description]
 * @param psComp [description]
 */
static void LocalToDB(CommComp_S * const psComp)
{
	S_LocalSubUnique * psSubUnique;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(psComp->PRI_tCurIndex < DBINDEX_SUM);
	DEBUG_ASSERT(NULL != psComp->PRI_psCurSubComp);
	psSubUnique = psComp->PRI_psCurSubComp->PRO_pvUnique;
	DEBUG_ASSERT(NULL != psSubUnique);
	DEBUG_ASSERT(NULL != psSubUnique->PRO_fReadDataToDB);
	psSubUnique->PRO_fReadDataToDB(psComp, psComp->PRI_psCurSubComp, psComp->PRI_tCurIndex);
	// Comm_ClrReadFlag(psComp->PRI_tCurIndex);
	SCommComp_ClrSDOState(psComp);
}
/**
 * [Default_OperationData description]
 * @param psComp    [description]
 * @param psSubComp [description]
 * @param tDBIndex  [description]
 */
static void Default_OperationData(CommComp_S * const psComp, CommSubComp_S * const psSubComp, DBIndex_T tDBIndex)
{
}
