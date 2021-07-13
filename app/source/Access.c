/*=============================================================================+
| Function : watch                                                             |
| Task     : watch Header File                                                 |
|------------------------------------------------------------------------------|
| Compile  :                                                                   |
| Link     :                                                                   |
| Call     :                                                                   |
|------------------------------------------------------------------------------|
| Author   : THJ                                                               |
| Version  : V1.00                                                             |
| Creation : 2016/11/21                                                        |
| Revision :                                                                   |
+=============================================================================*/
#include "Access.h"
#include "Error_App.h"
#include "database.h"
#include "CommComp_CANOpenMaster.h"
#include "task_ethercatmaster.h"
#include "common.h"
#include "debug.h"
#include <string.h>
//##############################################################################
//
//      Definition
//
//##############################################################################
#define ACCESS_TURNPAGES (ACCESS_GROUP_SUM / 2) // Less than or equal to ACCESS_GROUP_SUM.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
enum T_Result
{
    ACCESS_RESULT_DONE,
    ACCESS_RESULT_DOING,
    ACCESS_RESULT_TIMEOUT,
    ACCESS_RESULT_FAULT
};
enum TAccessType
{
    ACCESSTYPE_NONE,
    ACCESSTYPE_ETHERCAT,
    ACCESSTYPE_CAN
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
void  Init_WriteAccessController(struct T_AccessController * pAccessController, struct T_AccessInfo * pAccessInfo); // , T_FuncRead funcRead, T_FuncWrite funcWrite);
void  Init_ReadAccessController(struct T_AccessController * pAccessController, struct T_AccessInfo * pAccessInfo);  // , T_FuncRead funcRead, T_FuncWrite funcWrite);
void  Run_AccessController(struct T_AccessController * pAccessController);
void  Tired_WriteAccessController(struct T_AccessController * pAccessController);
void  Tired_ReadAccessController(struct T_AccessController * pAccessController);
BOOL  GetTired_WriteAccessController(struct T_AccessController * pAccessController);
BOOL  GetTired_ReadAccessController(struct T_AccessController * pAccessController);
DWORD GetChannel_WriteAccessController(struct T_AccessController * pAccessController);
DWORD GetChannel_ReadAccessController(struct T_AccessController * pAccessController);
WORD  ReadWriteState_AccessController(struct T_AccessController * pAccessController);
WORD  ReadReadState_AccessController(struct T_AccessController * pAccessController);
void  WriteWriteID_AccessController(struct T_AccessController * pAccessController, WORD iIndex, DWORD iID);
void  WriteReadID_AccessController(struct T_AccessController * pAccessController, WORD iIndex, DWORD iID);
void  WriteValue_AccessController(struct T_AccessController * pAccessController, WORD iIndex, DWORD iValue);
void  ReadValue_AccessController(struct T_AccessController * pAccessController, WORD iIndex);
void  SetWriteAccessType(struct T_AccessController * pAccessController);
void  SetReadAccessType(struct T_AccessController * pAccessController);

void Access_Write(struct T_AccessController * pAccessController);
void Access_Read(struct T_AccessController * pAccessController);

enum T_Result Access_ReadAccess(union T_AccessType uWatchType, union T_AccessID uWatchID, DWORD * pValue);
enum T_Result Access_WriteAccess(union T_AccessType uWatchType, union T_AccessID uWatchID, DWORD const * pValue);
BOOL          Access_SearchWriteItem(struct T_AccessController * pAccessController);
BOOL          Access_SearchReadItem(struct T_AccessController * pAccessController);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
struct
{
    DBIndex_T iWriteNumOfEntries;
    DBIndex_T iReadNumOfEntries;
} const Access__base_index[] = {
    { DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES, DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES },
    { DBINDEX_M28_WRITE_ACCESS2_NUMBER_OF_ENTRIES, DBINDEX_M28_READ_ACCESS2_NUMBER_OF_ENTRIES },
};

struct T_AccessController Access__aController[sizeof(Access__base_index) / sizeof(*Access__base_index)];
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Create_Access description]
 */
void Create_Access(void)
{
    WORD i;

    for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
    {
        Init_WriteAccessController(&Access__aController[i], (void *)DB_GetDataAddr(Access__base_index[i].iWriteNumOfEntries + 1));
        DB_SetDWORD(Access__base_index[i].iWriteNumOfEntries, sizeof(struct T_AccessInfo) / sizeof(DWORD));
        Init_ReadAccessController(&Access__aController[i], (void *)DB_GetDataAddr(Access__base_index[i].iReadNumOfEntries + 1));
        DB_SetDWORD(Access__base_index[i].iReadNumOfEntries, sizeof(struct T_AccessInfo) / sizeof(DWORD));
    }
}

/**
 * [Run_Access description]
 */
void Run_Access(void)
{
    struct T_AccessController* pController;
    WORD i;

    for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); i++)
    {
        pController = Access__aController + i;
        Run_AccessController(pController);
        if (!GetTired_WriteAccessController(pController)) {
            WORD j;

            for (j = i + 1; j < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++j) {
                if (GetChannel_WriteAccessController(pController) == GetChannel_WriteAccessController(Access__aController + j)) {
                    break;
                }
            }
            if (j < (sizeof(Access__base_index) / sizeof(*Access__base_index))) {
                break;
            }
            for (j = 0; j < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++j) {
                if (GetChannel_WriteAccessController(pController) == GetChannel_WriteAccessController(Access__aController + j)) {
                    Tired_WriteAccessController(Access__aController + j);
                }
            }
        }
        if (!GetTired_ReadAccessController(pController)) {
            WORD j;

            for (j = i + 1; j < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++j) {
                if (GetChannel_ReadAccessController(pController) == GetChannel_ReadAccessController(Access__aController + j)) {
                    break;
                }
            }
            if (j < (sizeof(Access__base_index) / sizeof(*Access__base_index))) {
                break;
            }
            for (j = 0; j < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++j) {
                if (GetChannel_ReadAccessController(pController) == GetChannel_ReadAccessController(Access__aController + j)) {
                    Tired_ReadAccessController(Access__aController + j);
                }
            }
        }
    }
}

/**
 * [DB_Access_Write_Type description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Write_Type(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((Access__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_WRITE_ACCESS1_COMM_PARAM - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES)) == tDBIndex)
                {
                    SetWriteAccessType(&Access__aController[i]);
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Read_Type description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Read_Type(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((Access__base_index[i].iReadNumOfEntries + (DBINDEX_M28_READ_ACCESS1_COMM_PARAM - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES)) == tDBIndex)
                {
                    SetReadAccessType(&Access__aController[i]);
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Write_State description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Write_State(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((Access__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_WRITE_ACCESS1_DATA_STATUS - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES)) == tDBIndex)
                {
                    DBVALUE* pData;

                    ReadWriteState_AccessController(&Access__aController[i]);
                    pData = (DBVALUE*)puDBData;
                    pData->vDWORD = Access__aController[i]._pWriteAccessInfo->uStatus.iAll;
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Read_State description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Read_State(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((Access__base_index[i].iReadNumOfEntries + (DBINDEX_M28_READ_ACCESS1_DATA_STATUS - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES)) == tDBIndex)
                {
                    DBVALUE* pData;

                    ReadReadState_AccessController(&Access__aController[i]);
                    pData = (DBVALUE*)puDBData;
                    pData->vDWORD = Access__aController[i]._pReadAccessInfo->uStatus.iAll;
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Write_ID description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Write_ID(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;
    unsigned iIndex;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((tDBIndex >= (Access__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_WRITE_ACCESS1_DATA_1_ID - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES)))
                    && (tDBIndex <= (Access__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_WRITE_ACCESS1_DATA_16_ID - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES))))
                {
                    iIndex = tDBIndex - Access__base_index[i].iWriteNumOfEntries - (DBINDEX_M28_WRITE_ACCESS1_DATA_1_ID - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES);
                    WriteWriteID_AccessController(&Access__aController[i], iIndex / 2, puDBData->vDWORD);
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Read_ID description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Read_ID(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;
    unsigned iIndex;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((tDBIndex >= (Access__base_index[i].iReadNumOfEntries + (DBINDEX_M28_READ_ACCESS1_DATA_1_ID - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES)))
                    && (tDBIndex <= (Access__base_index[i].iReadNumOfEntries + (DBINDEX_M28_READ_ACCESS1_DATA_16_ID - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES))))
                {
                    iIndex = tDBIndex - Access__base_index[i].iReadNumOfEntries - (DBINDEX_M28_READ_ACCESS1_DATA_1_ID - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES);
                    WriteReadID_AccessController(&Access__aController[i], iIndex / 2, puDBData->vDWORD);
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Write_Value description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Write_Value(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;
    unsigned iIndex;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((tDBIndex >= (Access__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_WRITE_ACCESS1_DATA_1_VALUE - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES)))
                    && (tDBIndex <= (Access__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_WRITE_ACCESS1_DATA_16_VALUE - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES))))
                {
                    iIndex = tDBIndex - Access__base_index[i].iWriteNumOfEntries - (DBINDEX_M28_WRITE_ACCESS1_DATA_1_VALUE - DBINDEX_M28_WRITE_ACCESS1_NUMBER_OF_ENTRIES);
                    WriteValue_AccessController(&Access__aController[i], iIndex / 2, puDBData->vDWORD);
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_Access_Read_Value description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_Access_Read_Value(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;
    unsigned iIndex;

    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            for (i = 0; i < (sizeof(Access__base_index) / sizeof(*Access__base_index)); ++i)
            {
                if ((tDBIndex >= (Access__base_index[i].iReadNumOfEntries + (DBINDEX_M28_READ_ACCESS1_DATA_1_VALUE - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES)))
                    && (tDBIndex <= (Access__base_index[i].iReadNumOfEntries + (DBINDEX_M28_READ_ACCESS1_DATA_16_VALUE - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES))))
                {
                    iIndex = tDBIndex - Access__base_index[i].iReadNumOfEntries - (DBINDEX_M28_READ_ACCESS1_DATA_1_VALUE - DBINDEX_M28_READ_ACCESS1_NUMBER_OF_ENTRIES);
                    ReadValue_AccessController(&Access__aController[i], iIndex / 2);
                    break;
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [Init_WriteAccessController description]
 * @param pAccessController [description]
 * @param pAccessInfo       [description]
 */
void Init_WriteAccessController(struct T_AccessController * pAccessController, struct T_AccessInfo * pAccessInfo)
{
    WORD iPackgeCount;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(NULL != pAccessInfo);
    DEBUG_ASSERT(ACCESS_GROUP_SUM >= ACCESS_TURNPAGES);
    if ((0 != pAccessInfo->uStatus.sBit.iDataStatus) && (0 != pAccessInfo->uAccessType.sBit.iPackgeIndex))
    {
        pAccessInfo = NULL;
        return;
    }
    pAccessController->_pWriteAccessInfo         = pAccessInfo;
    pAccessController->_uWriteAccessType.iAll    = pAccessInfo->uAccessType.iAll;
    //pAccessController->_sWrite.uID.iAll          = 0;
    //pAccessController->_sWrite.iValue            = 0;
    pAccessController->_bWriteFree               = TRUE;
    pAccessController->_bWriteTired              = TRUE;
    pAccessController->_iWriteTimeOutCount       = 0;
    pAccessController->_iWriteCurIndex           = 0;
    pAccessController->_iAccessWriteIdChangeFlag = 0;
    pAccessController->_iAccessWriteFlag         = 0;

    if (0 != pAccessInfo->uAccessType.iAll)
    {
        iPackgeCount              = pAccessInfo->uAccessType.sBit.iPackgeCount + 1;
        pAccessInfo->uStatus.iAll = ((DWORD)1 << iPackgeCount) - 1; // Reset. Anders 2016-12-08, Add.
    }
    else
        pAccessInfo->uStatus.iAll = 0;
    pAccessInfo->uStatus.sBit.iPackgeCount = pAccessInfo->uAccessType.sBit.iPackgeCount; // Anders 2016-12-08, Add.
    pAccessInfo->uStatus.sBit.iPackgeIndex = pAccessInfo->uAccessType.sBit.iPackgeIndex; // Anders 2016-12-08, Add.
    pAccessInfo->uPackge.iAll              = 0;
    memset(&pAccessInfo->aGroup, 0, sizeof(pAccessInfo->aGroup));
}

/**
 * [Init_ReadAccessController description]
 * @param pAccessController [description]
 * @param pAccessInfo       [description]
 */
void Init_ReadAccessController(struct T_AccessController * pAccessController, struct T_AccessInfo * pAccessInfo)
{
    WORD iPackgeCount;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(NULL != pAccessInfo);
    DEBUG_ASSERT(ACCESS_GROUP_SUM >= ACCESS_TURNPAGES);
    if ((0 != pAccessInfo->uStatus.sBit.iDataStatus) && (0 != pAccessInfo->uAccessType.sBit.iPackgeIndex))
    {
        pAccessInfo = NULL;
        return;
    }
    pAccessController->_pReadAccessInfo      = pAccessInfo;
    pAccessController->_uReadAccessType.iAll = pAccessInfo->uAccessType.iAll;
    //pAccessController->_sRead.uID.iAll       = 0;
    //pAccessController->_sRead.iValue         = 0;
    pAccessController->_bReadFree            = TRUE;
    pAccessController->_bReadTired           = TRUE;
    pAccessController->_iReadTimeOutCount    = 0;
    pAccessController->_iReadCurIndex        = 0;
    pAccessController->_iAccessReadFlag      = 0;
    pAccessController->_iReadDataOK          = 0;

    if (0 != pAccessInfo->uAccessType.iAll)
    {
        iPackgeCount              = pAccessInfo->uAccessType.sBit.iPackgeCount + 1;
        pAccessInfo->uStatus.iAll = ((DWORD)1 << iPackgeCount) - 1; // Reset. Anders 2016-12-08, Add.
    }
    else
        pAccessInfo->uStatus.iAll = 0;
    pAccessInfo->uStatus.sBit.iPackgeCount = pAccessInfo->uAccessType.sBit.iPackgeCount; // Anders 2016-12-08, Add.
    pAccessInfo->uStatus.sBit.iPackgeIndex = pAccessInfo->uAccessType.sBit.iPackgeIndex; // Anders 2016-12-08, Add.
    pAccessInfo->uPackge.iAll              = 0;
    memset(&pAccessInfo->aGroup, 0, sizeof(pAccessInfo->aGroup));
}

/**
 * [Run_AccessController description]
 * @param pAccessController [description]
 */
void Run_AccessController(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;

    DEBUG_ASSERT(NULL != pAccessController);
    // Write.
    pAccessInfo = pAccessController->_pWriteAccessInfo;
    if (pAccessController->_bWriteFree)
    {
        if (pAccessController->_bWriteTired && Access_SearchWriteItem(pAccessController))
        {
            pAccessController->_iWriteTimeOutCount = 0;
            pAccessController->_uWriteAccessType   = pAccessInfo->uAccessType;
            //pAccessController->_sWrite             = pAccessInfo->aGroup[pAccessController->_iWriteCurIndex];
            pAccessController->_bWriteFree         = FALSE;
            Access_Write(pAccessController);
        }
    }
    else
        Access_Write(pAccessController);

    // hankin 20190718 mask: To prevent when the read operation is locked, the write operation can only be successfully processed after the read operation has timed out.
    //if (FALSE == pAccessController->_bWriteFree)
    //    return;

    // Read.
    pAccessInfo = pAccessController->_pReadAccessInfo;
    if (pAccessController->_bReadFree)
    {
        if (pAccessController->_bReadTired && Access_SearchReadItem(pAccessController))
        {
            pAccessController->_iReadTimeOutCount = 0;
            pAccessController->_uReadAccessType   = pAccessInfo->uAccessType;
            //pAccessController->_sRead             = pAccessInfo->aGroup[pAccessController->_iReadCurIndex];
            pAccessController->_bReadFree         = FALSE;
            Access_Read(pAccessController);
        }
    }
    else
        Access_Read(pAccessController);
}

/**
 * [Tired_WriteAccessController description]
 * @author hankin (2019/11/25)
 * @param  pAccessController [description]
 */
void Tired_WriteAccessController(struct T_AccessController * pAccessController)
{
    DEBUG_ASSERT(NULL != pAccessController);
    pAccessController->_bWriteTired = TRUE;
}

/**
 * [Tired_ReadAccessController description]
 * @author hankin (2019/11/25)
 * @param  pAccessController [description]
 */
void Tired_ReadAccessController(struct T_AccessController * pAccessController)
{
    DEBUG_ASSERT(NULL != pAccessController);
    pAccessController->_bReadTired = TRUE;
}

/**
 * [GetTired_WriteAccessController description]
 * @author hankin (2019/11/25)
 * @param  pAccessController [description]
 * @return                   [description]
 */
BOOL GetTired_WriteAccessController(struct T_AccessController * pAccessController)
{
    DEBUG_ASSERT(NULL != pAccessController);
    return pAccessController->_bWriteTired;
}

/**
 * [GetTired_ReadAccessController description]
 * @author hankin (2019/11/25)
 * @param  pAccessController [description]
 * @return                   [description]
 */
BOOL GetTired_ReadAccessController(struct T_AccessController * pAccessController)
{
    DEBUG_ASSERT(NULL != pAccessController);
    return pAccessController->_bReadTired;
}

/**
 * [GetChannel_WriteAccessController description]
 * @author hankin (2019/11/25)
 * @param  pAccessController [description]
 * @return                   [description]
 */
DWORD GetChannel_WriteAccessController(struct T_AccessController * pAccessController)
{
    DEBUG_ASSERT(NULL != pAccessController);
    return (pAccessController->_uWriteAccessType.sBit.iChannel << 8)
        | pAccessController->_uWriteAccessType.sBit.iType;
}

/**
 * [GetChannel_ReadAccessController description]
 * @author hankin (2019/11/25)
 * @param  pAccessController [description]
 * @return                   [description]
 */
DWORD GetChannel_ReadAccessController(struct T_AccessController * pAccessController)
{
    DEBUG_ASSERT(NULL != pAccessController);
    return (pAccessController->_uReadAccessType.sBit.iChannel << 8)
        | pAccessController->_uReadAccessType.sBit.iType;
}

/**
 * [ReadWriteState_AccessController description]
 * @param  pAccessController [description]
 * @return                   [description]
 */
WORD ReadWriteState_AccessController(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;
    WORD                  iDataStatus;
    WORD                  mask;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(NULL != pAccessController->_pWriteAccessInfo);
    pAccessInfo = pAccessController->_pWriteAccessInfo;
    iDataStatus = pAccessInfo->uStatus.sBit.iDataStatus;
    mask        = ~(pAccessInfo->uPackge.sBit.iExecStatus & iDataStatus);
    pAccessInfo->uStatus.sBit.iDataStatus &= mask;
    //pAccessInfo->uStatus.sBit.iExecStatus &= mask;
    //pAccessInfo->dwStatusBackup = pAccessInfo->uStatus.iAll;

    pAccessController->_iAccessWriteIdChangeFlag &= ~pAccessInfo->uPackge.sBit.iExecStatus;
    return iDataStatus;
}

/**
 * [ReadReadState_AccessController description]
 * @param  pAccessController [description]
 * @return                   [description]
 */
WORD ReadReadState_AccessController(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;
    WORD                  iDataStatus;
    WORD                  mask;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(NULL != pAccessController->_pReadAccessInfo);
    pAccessInfo = pAccessController->_pReadAccessInfo;
    iDataStatus = pAccessInfo->uStatus.sBit.iDataStatus;

    mask = ~(pAccessInfo->uPackge.sBit.iExecStatus & iDataStatus & pAccessController->_iReadDataOK & pAccessInfo->uPackge.sBit.iRxIDStatus);
    pAccessInfo->uStatus.sBit.iDataStatus &= mask;
    //pAccessInfo->uStatus.sBit.iExecStatus &= mask;
    //pAccessInfo->dwStatusBackup = pAccessInfo->uStatus.iAll;
    //pAccessController->_iReadDataOK &= mask;

    return iDataStatus;
}

/**
 * [WriteWriteID_AccessController description]
 * @param pAccessController [description]
 * @param iIndex             [description]
 * @param iID               [description]
 */
void WriteWriteID_AccessController(struct T_AccessController * pAccessController, WORD iIndex, DWORD iID)
{
    struct T_AccessInfo * pAccessInfo;
    WORD                  mask;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(iIndex < ACCESS_GROUP_SUM);
    DEBUG_ASSERT(NULL != pAccessController->_pWriteAccessInfo);
    pAccessInfo = pAccessController->_pWriteAccessInfo;
    if (0 == pAccessInfo->uAccessType.iAll)
        return;
    if (0 == iID)
        return;
    mask = (WORD)1 << iIndex;
    if (0 == (pAccessInfo->uStatus.sBit.iDataStatus & mask))
        return;
    if (0 != (mask & pAccessInfo->uPackge.sBit.iRxIDStatus)) {
        pAccessInfo->aGroup[iIndex].uID = pAccessController->_aWriteBackup[iIndex].uID;
        return;
    }
    // pAccessInfo->writeStatus.sBit.iExecStatus &= ~mask;
    pAccessInfo->aGroup[iIndex].uID.iAll = iID;
    pAccessInfo->aGroup[iIndex].iValue   = 0;
    pAccessInfo->uPackge.sBit.iRxIDStatus |= mask;

    pAccessController->_aWriteBackup[iIndex].uID = pAccessInfo->aGroup[iIndex].uID;
    pAccessController->_iAccessWriteIdChangeFlag |= mask;
}

/**
 * [WriteReadID_AccessController description]
 * @param pAccessController [description]
 * @param iIndex             [description]
 * @param iID               [description]
 */
void WriteReadID_AccessController(struct T_AccessController * pAccessController, WORD iIndex, DWORD iID)
{
    struct T_AccessInfo * pAccessInfo;
    WORD                  mask;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(iIndex < ACCESS_GROUP_SUM);
    DEBUG_ASSERT(NULL != pAccessController->_pReadAccessInfo);
    pAccessInfo = pAccessController->_pReadAccessInfo;
    if (0 == pAccessInfo->uAccessType.iAll)
        return;
    if (0 == iID)
        return;
    mask = (WORD)1 << iIndex;
    if (0 == (pAccessInfo->uStatus.sBit.iDataStatus & mask))
        return;
    if (0 != (mask & pAccessInfo->uPackge.sBit.iRxIDStatus)) {
        pAccessInfo->aGroup[iIndex].uID = pAccessController->_aReadBackup[iIndex].uID;
        return;
    }
    pAccessInfo->uStatus.sBit.iDataStatus |= mask;
    pAccessInfo->uPackge.sBit.iExecStatus &= ~mask;
    //pAccessInfo->dwStatusBackup = pAccessInfo->uStatus.iAll;
    pAccessInfo->aGroup[iIndex].uID.iAll = iID;
    pAccessInfo->aGroup[iIndex].iValue   = 0;
    pAccessInfo->uPackge.sBit.iRxIDStatus |= mask;

    pAccessController->_aReadBackup[iIndex].uID = pAccessInfo->aGroup[iIndex].uID;
    pAccessController->_aReadBackup[iIndex].iValue = 0;//shanll 20200730 add
    pAccessController->_iAccessReadFlag |= mask;
    pAccessController->_iReadDataOK &= ~mask; // Anders 2016-12-8, Add
}

/**
 * [WriteValue_AccessController description]
 * @param pAccessController [description]
 * @param iIndex             [description]
 * @param iValue             [description]
 */
void WriteValue_AccessController(struct T_AccessController * pAccessController, WORD iIndex, DWORD iValue)
{
    struct T_AccessInfo * pAccessInfo;
    WORD                  mask;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(iIndex < ACCESS_GROUP_SUM);
    DEBUG_ASSERT(NULL != pAccessController->_pWriteAccessInfo);
    pAccessInfo = pAccessController->_pWriteAccessInfo;
    if (0 == pAccessInfo->uAccessType.iAll)
        return;
    mask = (WORD)1 << iIndex;
    if (0 == (pAccessController->_iAccessWriteIdChangeFlag & mask)) {
        pAccessInfo->aGroup[iIndex].iValue = pAccessController->_aWriteBackup[iIndex].iValue;
        return;
    }
    if (0 == (pAccessInfo->uStatus.sBit.iDataStatus & mask))
        return;
    // if (iValue != pAccessInfo->writeGroup[iIndex].iValue)
    {
        pAccessInfo->uPackge.sBit.iExecStatus &= ~mask;
        //pAccessInfo->dwStatusBackup = pAccessInfo->uStatus.iAll;
        pAccessInfo->aGroup[iIndex].iValue = iValue;
        //if (0 == pAccessController->_iAccessWriteFlag)
        //  pAccessController->_iWriteCurIndex = iIndex;
        pAccessController->_aWriteBackup[iIndex].iValue = pAccessInfo->aGroup[iIndex].iValue;
        pAccessController->_iAccessWriteFlag |= mask;
    }
}

/**
 * [ReadValue_AccessController description]
 * @param pAccessController [description]
 * @param iIndex             [description]
 */
void ReadValue_AccessController(struct T_AccessController * pAccessController, WORD iIndex)
{
    struct T_AccessInfo * pAccessInfo;
    WORD                  mask;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(iIndex < ACCESS_GROUP_SUM);
    DEBUG_ASSERT(NULL != pAccessController->_pReadAccessInfo);
    pAccessInfo = pAccessController->_pReadAccessInfo;
    if (0 == pAccessInfo->uAccessType.iAll)
        return;
    mask = (WORD)1 << iIndex;

    //if (0 == (pAccessInfo->uStatus.sBit.iDataStatus & mask))  // Anders 2016-12-02, Mark.
    //  return;

    if (0 == (pAccessInfo->uPackge.sBit.iRxIDStatus & mask))
        return;

    //if (0 == pAccessController->_iAccessReadFlag)
    //  pAccessController->_iReadCurIndex = iIndex;
    if (0 != (pAccessInfo->uPackge.sBit.iExecStatus & mask))
        pAccessController->_iReadDataOK |= mask;
    pAccessController->_aReadBackup[iIndex].iValue = pAccessInfo->aGroup[iIndex].iValue;
    pAccessController->_iAccessReadFlag |= mask;
}

/**
 * [Access_Write description]
 * @param pAccessController [description]
 */
void Access_Write(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;
    struct T_AccessGroup* pGroup;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(!pAccessController->_bWriteFree);
    DEBUG_ASSERT(pAccessController->_iWriteCurIndex < ACCESS_GROUP_SUM);
    DEBUG_ASSERT(NULL != pAccessController->_pWriteAccessInfo);
    pAccessInfo = pAccessController->_pWriteAccessInfo;
    DEBUG_ASSERT(NULL != pAccessInfo);
    pGroup = pAccessController->_aWriteBackup + pAccessController->_iWriteCurIndex;
    //shanll 20201226 add
    // if((pAccessInfo->uPackge.sBit.iExecStatus & ((WORD)1 << pAccessController->_iWriteCurIndex)) != FALSE)
    //     return;
    switch (Access_WriteAccess(pAccessController->_pWriteAccessInfo->uAccessType, pGroup->uID, &pGroup->iValue))
    {
        case ACCESS_RESULT_DONE:
            pAccessInfo->uPackge.sBit.iExecStatus |= (WORD)1 << pAccessController->_iWriteCurIndex;
            pAccessController->_bWriteFree = TRUE;
            break;
        case ACCESS_RESULT_FAULT:
            pAccessInfo->aGroup[pAccessController->_iWriteCurIndex].iValue = pGroup->iValue;
            switch (pGroup->iValue)
            {
                case 0x06020000:
                    //shanll 20210106 modify
                    pAccessInfo->uPackge.sBit.iExecStatus |= (WORD)1 << pAccessController->_iWriteCurIndex;
                    pAccessController->_bWriteFree = TRUE;
                    //Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE, pAccessController->_uWriteAccessType.sBit.iNodeId_slaveIndex);
                    break;
                case 0x06090011:
                    //shanll 20210111 modify
                    pAccessInfo->uPackge.sBit.iExecStatus |= (WORD)1 << pAccessController->_iWriteCurIndex;
                    pAccessController->_bWriteFree = TRUE;
                    // Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE, pAccessController->_uWriteAccessType.sBit.iNodeId_slaveIndex);
                    break;
                default:
                    Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE, pAccessController->_uWriteAccessType.sBit.iNodeId_slaveIndex);
                    break;
            }
            pAccessController->_bWriteFree = TRUE;
            break;
        case ACCESS_RESULT_TIMEOUT:
            if (++pAccessController->_iWriteTimeOutCount > 255)
            {
                pAccessController->_iWriteTimeOutCount = 0;
                pAccessController->_bWriteFree         = TRUE;
            }
            break;
        default:
            //if( pAccessController->_bWriteFree == TRUE)
            //  ESTOP0;
            break;
    }

    //if (pAccessController->_bWriteFree)
    //  if (0x27D2 == pAccessController->_pWriteAccessInfo->aGroup[pAccessController->_iWriteCurIndex].uID.sBit.iIndex)
    //      ESTOP0;
}

/**
 * [Access_Read description]
 * @param pAccessController [description]
 */
void Access_Read(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;
    struct T_AccessGroup* pGroup;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(!pAccessController->_bReadFree);
    DEBUG_ASSERT(pAccessController->_iReadCurIndex < ACCESS_GROUP_SUM);
    DEBUG_ASSERT(NULL != pAccessController->_pReadAccessInfo);
    pAccessInfo = pAccessController->_pReadAccessInfo;
    DEBUG_ASSERT(NULL != pAccessInfo);
    pGroup = pAccessController->_aReadBackup + pAccessController->_iReadCurIndex;
    //shanll 20201226 add
    // if((pAccessInfo->uPackge.sBit.iExecStatus & ((WORD)1 << pAccessController->_iReadCurIndex)) != FALSE)
    //     return;
    switch (Access_ReadAccess(pAccessController->_pReadAccessInfo->uAccessType, pGroup->uID, &pGroup->iValue))
    {
        case ACCESS_RESULT_FAULT:
            pGroup->iValue = 0;
        // continue ACCESS_RESULT_FAULT.
        case ACCESS_RESULT_DONE:
            pAccessInfo->uPackge.sBit.iExecStatus |= (WORD)1 << pAccessController->_iReadCurIndex;
            //pAccessInfo->dwStatusBackup = pAccessInfo->uStatus.iAll;

            //if (group->uID.sBit.iIndex == 0x377F )
            //{
            //  if ( pAccessController->_iReadValue == 0x7F80000)
            //  {
            //      pAccessController->_bReadFree = FALSE;
            //  }
            //}
            pAccessInfo->aGroup[pAccessController->_iReadCurIndex].iValue = pGroup->iValue;
            pAccessController->_bReadFree                                 = TRUE;
            break;
        case ACCESS_RESULT_TIMEOUT:
            if (++pAccessController->_iReadTimeOutCount > 10)
            {
                pAccessController->_iReadTimeOutCount = 0;
                pAccessController->_bReadFree         = TRUE;
            }
            break;
        default:
            break;
    }
}

/**
 * [Access_SearchWriteItem description]
 * @param  pAccessController [description]
 * @return                   [description]
 */
BOOL Access_SearchWriteItem(struct T_AccessController * pAccessController)
{
    WORD i;
    ///struct T_AccessInfo * pAccessInfo;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(pAccessController->_bWriteFree);
    DEBUG_ASSERT(NULL != pAccessController->_pWriteAccessInfo);
    if (0 != pAccessController->_iAccessWriteFlag)
    {
        for (i = 0; i < ACCESS_TURNPAGES; i++)
        {
            if (0 != (pAccessController->_iAccessWriteFlag & ((WORD)1 << pAccessController->_iWriteCurIndex)))
            {
                //pAccessInfo = pAccessController->_pWriteAccessInfo;

                pAccessController->_iAccessWriteFlag &= ~((WORD)1 << pAccessController->_iWriteCurIndex);
                //DEBUG_ASSERT(pAccessInfo->aGroup[pAccessController->_iWriteCurIndex].uID.sBit.iLenOfBits > 0);
                pAccessController->_bWriteTired = 0 != pAccessController->_iAccessWriteFlag;
                return TRUE;
            }
            pAccessController->_iWriteCurIndex = (pAccessController->_iWriteCurIndex < (ACCESS_GROUP_SUM - 1)) ? (pAccessController->_iWriteCurIndex + 1) : 0;
        }
    }
    return FALSE;
}

/**
 * [Access_SearchReadItem description]
 * @param  pAccessController [description]
 * @return                   [description]
 */
BOOL Access_SearchReadItem(struct T_AccessController * pAccessController)
{
    WORD i;
    //struct T_AccessInfo * pAccessInfo;

    DEBUG_ASSERT(NULL != pAccessController);
    DEBUG_ASSERT(pAccessController->_bReadFree);
    if (0 != pAccessController->_iAccessReadFlag)
    {
        for (i = 0; i < ACCESS_TURNPAGES; i++)
        {
            if (0 != (pAccessController->_iAccessReadFlag & ((WORD)1 << pAccessController->_iReadCurIndex)))
            {
                DEBUG_ASSERT(NULL != pAccessController->_pReadAccessInfo);
                //pAccessInfo = pAccessController->_pReadAccessInfo;
                //DEBUG_ASSERT(pAccessInfo->aGroup[pAccessController->_iReadCurIndex].uID.sBit.iLenOfBits > 0);
                pAccessController->_iAccessReadFlag &= ~((WORD)1 << pAccessController->_iReadCurIndex);
                pAccessController->_bReadTired = 0 != pAccessController->_iAccessReadFlag;
                return TRUE;
            }
            pAccessController->_iReadCurIndex = (pAccessController->_iReadCurIndex < (ACCESS_GROUP_SUM - 1)) ? (pAccessController->_iReadCurIndex + 1) : 0;
        }
    }
    return FALSE;
}

/**
 * [Access_ReadAccess description]
 * @param  uWatchType [description]
 * @param  uWatchID   [description]
 * @param  pValue    [description]
 * @return           [description]
 */
enum T_Result Access_ReadAccess(union T_AccessType uWatchType, union T_AccessID uWatchID, DWORD * pValue)
{
    switch (uWatchType.sBit.iType)
    {
        case ACCESSTYPE_CAN:
            switch (UploadCANSDO_Easy(uWatchType.sBit.iChannel, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, (WORD *)pValue, uWatchID.sBit.iLenOfBits >> 3))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return ACCESS_RESULT_DOING;
                case SDORESULT_DONE:
                    //if ( 57018 == uWatchID.sBit.iIndex-0x2000+50000 )
                    //{
                    //  ESTOP0;
                    //}
                    return ACCESS_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    //ESTOP0;
                    return ACCESS_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    //ESTOP0;
                    return ACCESS_RESULT_FAULT;
                default:
                    break;
            }
            break;
        case ACCESSTYPE_ETHERCAT:
            if (uWatchType.sBit.iNodeId_slaveIndex >= g_ECController.Master.wAppSlaveCount)
                return ACCESS_RESULT_DOING;
            switch (ECMaster_UploadSDO(&g_ECController.Master, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, (WORD *)pValue, uWatchID.sBit.iLenOfBits >> 3, FALSE))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return ACCESS_RESULT_DOING;
                case SDORESULT_DONE:
                    return ACCESS_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    return ACCESS_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    return ACCESS_RESULT_FAULT;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return ACCESS_RESULT_DOING;
}

/**
 * [Access_WriteAccess description]
 * @param  uWatchType [description]
 * @param  uWatchID   [description]
 * @param  pValue     [description]
 * @return            [description]
 */
enum T_Result Access_WriteAccess(union T_AccessType uWatchType, union T_AccessID uWatchID, DWORD const * pValue)
{
    switch (uWatchType.sBit.iType)
    {
        case ACCESSTYPE_CAN:
            switch (DownloadCANSDO_Easy(uWatchType.sBit.iChannel, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, *pValue, uWatchID.sBit.iLenOfBits >> 3))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return ACCESS_RESULT_DOING;
                case SDORESULT_DONE:
                    return ACCESS_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    return ACCESS_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    return ACCESS_RESULT_FAULT;
                default:
                    break;
            }
            break;
        case ACCESSTYPE_ETHERCAT:
            if (uWatchType.sBit.iNodeId_slaveIndex >= g_ECController.Master.wAppSlaveCount)
                return ACCESS_RESULT_DOING;
            switch (ECMaster_DownloadSDO(&g_ECController.Master, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, (WORD *)pValue, uWatchID.sBit.iLenOfBits >> 3, FALSE))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return ACCESS_RESULT_DOING;
                case SDORESULT_DONE:
                    return ACCESS_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    return ACCESS_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    return ACCESS_RESULT_FAULT;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return ACCESS_RESULT_DOING;
}

/**
 * [SetWriteAccessType description]
 * @param pAccessController [description]
 */
void SetWriteAccessType(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;
    // Write.
    pAccessInfo = pAccessController->_pWriteAccessInfo;
    if (pAccessController->_uWriteAccessType.iAll != pAccessInfo->uAccessType.iAll)
        Init_WriteAccessController(pAccessController, pAccessController->_pWriteAccessInfo);
}

/**
 * [SetReadAccessType description]
 * @param pAccessController [description]
 */
void SetReadAccessType(struct T_AccessController * pAccessController)
{
    struct T_AccessInfo * pAccessInfo;
    // Read.
    pAccessInfo = pAccessController->_pReadAccessInfo;
    if (pAccessController->_uReadAccessType.iAll != pAccessInfo->uAccessType.iAll)
        Init_ReadAccessController(pAccessController, pAccessController->_pReadAccessInfo);
}
