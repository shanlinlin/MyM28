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
|  Creation : 2015/07/30                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "../include/database.h"
#include "../include/CommComp_Factory.h"
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
#define DATABASE_ENUMERATION_C
#include ".database.h"
#undef DATABASE_ENUMERATION_C
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
inline DBID_T IndexToID(DBIndex_T tIndex);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
#pragma DATA_SECTION(g_sDB, "Database");
S_DB g_sDB;

#define DATABASE_DATA_DECLARATION_C
#include ".database.h"
#undef DATABASE_DATA_DECLARATION_C
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_DB description]
 */
void Init_DB(void)
{
	DBGroup_S const*  pGroup;
	DBAttrib_S const* pAttrib;
	DBIndex_T         iIndex;
	DBVALUE           sValue;

	//#if (DEBUG)
	//{
	DBAttribInfo_S sAttribInfo;
	DBAttrib_S     sAttrib;
	DBID_T         iPrevID=0;
	WORD           i;

	sAttribInfo.bGroupIndex  = (WORD)-1;
	//sAttribInfo.bAttribIndex = (WORD)-1;
	sAttribInfo.bAttribIndex = (DWORD)-1;//SHANLL 20191224 Modify
	debug_assert(sAttribInfo.bGroupIndex >= DBGROUP_SUM);
	//debug_assert(sAttribInfo.bAttribIndex >= 0x3FF);
	debug_assert(sAttribInfo.bAttribIndex >= 0x3FFFFFF);//SHANLL 20191224 Modify
	sAttrib.bAccess   = (WORD)-1;
	sAttrib.bDataType = (WORD)-1;
	sAttrib.bCallback = (WORD)-1;
	debug_assert(sAttrib.bAccess >= DATAACCESS_SUM);
	debug_assert(sAttrib.bDataType >= DATATYPE_SUM);
	debug_assert(sAttrib.bCallback >= DBCALLBACK_SUM);
	iIndex = sAttribInfo.bGroupIndex;
	iIndex = sAttrib.bAccess;

	debug_assert(DATATYPE_SUM == (sizeof(gc_asDataType) / sizeof(*gc_asDataType)));
	debug_assert(DBINDEX_SUM == (sizeof(gc_asAttribInfo) / sizeof(*gc_asAttribInfo)));
	debug_assert(DBGROUP_SUM == (sizeof(gc_asGroup) / sizeof(*gc_asGroup)));
	for (iIndex = (DBIndex_T)0; iIndex < DBINDEX_SUM; ++iIndex)
	{
		debug_assert(gc_asAttribInfo[iIndex].bGroupIndex < DBGROUP_SUM);
		pGroup = DB_GetGroup(iIndex);
		debug_assert(NULL != pGroup->pasAttrib);
		debug_assert((char*)pGroup->pcBaseAddr >= (char*)&g_sDB);
		debug_assert((char*)pGroup->pcBaseAddr < ((char*)&g_sDB + sizeof(g_sDB)));
		pAttrib = DB_GetAttrib(pGroup, iIndex);
		debug_assert(NULL != pAttrib);
		debug_assert(pAttrib->bDataOffset < sizeof(g_sDB));
		debug_assert(pAttrib->bAccess < DATAACCESS_SUM);
		debug_assert(pAttrib->bDataType < DATATYPE_SUM);
		debug_assert(pAttrib->bCallback < DBCALLBACK_SUM);
		if (0 != iIndex)
		{
			debug_assert(iPrevID < (pGroup->dwIDBase + pAttrib->dwIDOffset));
		}
		iPrevID = pGroup->dwIDBase + pAttrib->dwIDOffset;
		i       = iPrevID >> 24;
		debug_assert(uc_asFindInitTab[i][0] <= iIndex);
		debug_assert(uc_asFindInitTab[i][1] >= iIndex);
	}

	for (i = 0; i < (sizeof(uc_asFindInitTab) / sizeof(*uc_asFindInitTab)); ++i)
	{
		if (uc_asFindInitTab[i][0] <= uc_asFindInitTab[i][1])
		{
			debug_assert(i == (IndexToID(uc_asFindInitTab[i][0]) >> 24));
			debug_assert(i == (IndexToID(uc_asFindInitTab[i][1]) >> 24));
		}
	}
	debug_assert((DBINDEX_SUM - 1) == uc_asFindInitTab[i - 1][1]);
	//}
	//#endif

	// memset(u_adwDB_SetFlag, 0, sizeof(u_adwDB_SetFlag));
	// memset(u_adwDB_GetFlag, 0, sizeof(u_adwDB_GetFlag));
	memset(&sValue, 0, sizeof(sValue));
	for (iIndex = (DBIndex_T)0; iIndex < DBINDEX_SUM; ++iIndex)
	{
		DB_SetData(iIndex, &sValue);
	}
	g_sDB.M28.TEMPERATURE.MOLD_SENSOR_TYPE = (WORD)-1;
	g_sDB.M28.TEMPERATURE.PIPE_SENSOR_TYPE = (WORD)-1;
}
/**
 * [Run_DB description]
 */
void Run_DB(void)
{
}
/**
 * [DB_GetIndex description]
 * @param  tID       [description]
 * @param  tMinIndex [description]
 * @param  tMaxIndex [description]
 * @return           [description]
 */
DBIndex_T DB_GetIndex(DBID_T tID, DBIndex_T tMinIndex, DBIndex_T tMaxIndex)
{
	DBID_T    tCurID;
	DBIndex_T tIndex;

	DEBUG_ASSERT(tMinIndex <= tMaxIndex);
	DEBUG_ASSERT(tMaxIndex < DBINDEX_SUM);
	tMaxIndex
	    += 1;
	while (tMinIndex < tMaxIndex)
	{
		tIndex = (DBIndex_T)((tMinIndex + tMaxIndex) >> 1);
		tCurID = IndexToID(tIndex);
		if (tID > tCurID)
			tMinIndex = (DBIndex_T)(tIndex + 1);
		else if (tID < tCurID)
			tMaxIndex = tIndex;
		else
			return tIndex;
	}
	return 0xFFFF; // DBINDEX_SUM; Anders 2016-10-9, modify.
}
/**
 * [DB_IDToIndex description]
 * @param  tID [description]
 * @return     [description]
 */
DBIndex_T DB_IDToIndex(DBID_T tID)
{
	WORD i;

	i = tID >> 24;
	if (i >= (sizeof(uc_asFindInitTab) / sizeof(*uc_asFindInitTab)))
		return 0xFFFF; // DBINDEX_SUM; Anders 2016-10-9, modify.
	return DB_GetIndex(tID, uc_asFindInitTab[i][0], uc_asFindInitTab[i][1]);
}
/**
 * [DB_IndexToID description]
 * @param  tIndex [description]
 * @return        [description]
 */
DBID_T DB_IndexToID(DBIndex_T tIndex)
{
	if (tIndex < DBINDEX_SUM)
		return IndexToID(tIndex);
	return (DBID_T)-1;
}
/**
 * [DB_SizeofBData description]
 * @param  tIndex [description]
 * @return        [description]
 */
WORD DB_SizeofBData(DBIndex_T tIndex)
{
	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	switch (DB_GetDataType(tIndex))
	{
		//case DATATYPE_INT:
		//	return sizeofB(INT);
		case DATATYPE_BYTE:
			return 1;
		case DATATYPE_WORD:
			return 2;
		case DATATYPE_DWORD:
		case DATATYPE_LONG:
		case DATATYPE_FLOAT:
			return 4;
		default:
			DEBUG_ASSERT(false);
			return 0;
	}
}
/**
 * [DB_SetData description]
 * @param tIndex [description]
 * @param pvData [description]
 */
void DB_SetData(DBIndex_T tIndex, void const* pvData)
{
	DBGroup_S const*  psGroup;
	DBAttrib_S const* psAttrib;
	register unsigned i;
	unsigned          uiDataSize;
	char*             pcData;

	DEBUG_ASSERT(NULL != pvData);
	psGroup = DB_GetGroup(tIndex);
	DEBUG_ASSERT(NULL != psGroup);
	psAttrib = DB_GetAttrib(psGroup, tIndex);
	DEBUG_ASSERT(NULL != psAttrib);
	DEBUG_ASSERT(psAttrib->bDataType < (sizeof(gc_asDataType) / sizeof(*gc_asDataType)));
	uiDataSize = gc_asDataType[psAttrib->bDataType];
	DEBUG_ASSERT(NULL != psGroup->pcBaseAddr);
	pcData = (char*)psGroup->pcBaseAddr + psAttrib->bDataOffset;
	DEBUG_ASSERT(pcData >= (char*)&g_sDB);
	DEBUG_ASSERT(pcData < ((char*)&g_sDB + sizeof(g_sDB)));
	for (i = 0; i < uiDataSize; ++i)
		pcData[i] = ((char*)pvData)[i];
}
/**
 * [DB_GetData description]
 * @param tIndex [description]
 * @param pvData [description]
 */
void DB_GetData(DBIndex_T tIndex, void* pvData)
{
	DBGroup_S const*  psGroup;
	DBAttrib_S const* psAttrib;
	register unsigned i;
	unsigned          uiDataSize;
	char*             pcData;

	DEBUG_ASSERT(NULL != pvData);
	psGroup = DB_GetGroup(tIndex);
	DEBUG_ASSERT(NULL != psGroup);
	psAttrib = DB_GetAttrib(psGroup, tIndex);
	DEBUG_ASSERT(NULL != psAttrib);
	DEBUG_ASSERT(psAttrib->bDataType < (sizeof(gc_asDataType) / sizeof(*gc_asDataType)));
	uiDataSize = gc_asDataType[psAttrib->bDataType];
	DEBUG_ASSERT(NULL != psGroup->pcBaseAddr);
	pcData = (char*)psGroup->pcBaseAddr + psAttrib->bDataOffset;
	DEBUG_ASSERT(pcData >= (char*)&g_sDB);
	DEBUG_ASSERT(pcData < ((char*)&g_sDB + sizeof(g_sDB)));
	for (i = 0; i < uiDataSize; ++i)
		((char*)pvData)[i] = pcData[i];
}
/**
 * [DB_SetDataBySysIndex description]
 * @param  tIndex [description]
 * @param  pvData [description]
 * @return        [description]
 */
E_DBRetCode DB_SetDataBySysIndex(DBIndex_T tIndex, void const* pvData)
{
	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	DB_SetData(tIndex, pvData);
	Comm_SetWriteFlagByDBIndex(tIndex);
	return DB_CallCallBack(tIndex, DBEVENT_WRITE_AFTER, pvData);
}
/**
 * [DB_GetDataBySysIndex description]
 * @param  tIndex [description]
 * @param  pvData [description]
 * @return        [description]
 */
E_DBRetCode DB_GetDataBySysIndex(DBIndex_T tIndex, void* pvData)
{
	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	DB_GetData(tIndex, pvData);
	Comm_SetReadFlagByDBIndex(tIndex);
	return DB_CallCallBack(tIndex, DBEVENT_READ_AFTER, pvData);
}
/**
 * [DB_SetDataByUserIndex description]
 * @param  tIndex [description]
 * @param  pvData [description]
 * @return        [description]
 */
E_DBRetCode DB_SetDataByUserIndex(DBIndex_T tIndex, void const* pvData)
{
	DBAttrib_S const* psAttrib;

	if (tIndex >= DBINDEX_SUM)
		return DBRETCODE_ACCESS_DATA_NOTEXSIT;
	if (NULL == pvData)
		return DBRETCODE_ACCESS_ERR_DATA;
	psAttrib = DB_GetAttrib(DB_GetGroup(tIndex), tIndex);
	DEBUG_ASSERT(NULL != psAttrib);
	switch (psAttrib->bAccess)
	{
		case DATAACCESS_WO:
		case DATAACCESS_RW:
			return DB_SetDataBySysIndex(tIndex, pvData);
		default:
		{
			return DBRETCODE_ACCESS_WRITE_PERMISSION_DENIED;
		}
	}
}
/**
 * [DB_GetDataByUserIndex description]
 * @param  tIndex [description]
 * @param  pvData [description]
 * @return        [description]
 */
E_DBRetCode DB_GetDataByUserIndex(DBIndex_T tIndex, void* pvData)
{
	DBAttrib_S const* psAttrib;

	if (tIndex >= DBINDEX_SUM)
		return DBRETCODE_ACCESS_DATA_NOTEXSIT;
	if (NULL == pvData)
		return DBRETCODE_ACCESS_ERR_DATA;
	psAttrib = DB_GetAttrib(DB_GetGroup(tIndex), tIndex);
	DEBUG_ASSERT(NULL != psAttrib);
	switch (psAttrib->bAccess)
	{
		case DATAACCESS_RO:
		case DATAACCESS_RW:
			return DB_GetDataBySysIndex(tIndex, pvData);
		default:
			return DBRETCODE_ACCESS_READ_PERMISSION_DENIED;
	}
}
/**
 * [DB_CallCallBack description]
 * @param  tIndex [description]
 * @param  eEvent [description]
 * @param  pvData [description]
 * @return        [description]
 */
E_DBRetCode DB_CallCallBack(DBIndex_T tIndex, E_DBEvent eEvent, void const* pvData)
{
	DBAttrib_S const* psAttrib;

	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	psAttrib = DB_GetAttrib(DB_GetGroup(tIndex), tIndex);
	DEBUG_ASSERT(NULL != psAttrib);
	DEBUG_ASSERT(psAttrib->bCallback < (sizeof(uc_afCallback) / sizeof(*uc_afCallback)));
	return uc_afCallback[psAttrib->bCallback](tIndex, eEvent, pvData);
}
/**
 * [IndexToID description]
 * @param  tIndex [description]
 * @return        [description]
 */
inline DBID_T IndexToID(DBIndex_T tIndex)
{
	DBGroup_S const*  psGroup;
	DBAttrib_S const* psAttrib;

	DEBUG_ASSERT(tIndex < DBINDEX_SUM);
	psGroup = DB_GetGroup(tIndex);
	DEBUG_ASSERT(NULL != psGroup);
	psAttrib = DB_GetAttrib(psGroup, tIndex);
	DEBUG_ASSERT(NULL != psAttrib);
	return psGroup->dwIDBase + psAttrib->dwIDOffset;
}
/**
 * [SetDBValueByIndex description]
 * @param  tDBIndex [description]
 * @param  dbValue  [description]
 * @return          [description]
 */
WORD SetDBValueByIndex(DBIndex_T tDBIndex, DBVALUE dbValue)
{
	switch (DB_SetDataByUserIndex(tDBIndex, &dbValue))
	{
		case DBRETCODE_ACCESS_WRITE_PERMISSION_DENIED:
			return DB_ERR_READONLY;
		case DBRETCODE_ACCESS_WRITE_OUTRANGE:
			return DB_ERR_OUTRANGE;
		default:
			return DB_SUCCESS;
	}
}

/**
 * [DB_CallbackDefault description]
 * @author hankin (2019/09/20)
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_CallbackDefault(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    return DBRETCODE_SUCCESS;
}
