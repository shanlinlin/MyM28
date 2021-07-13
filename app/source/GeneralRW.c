/*=============================================================================+
| Function : watch                                                             |
| Task     : watch Header File                                                 |
|------------------------------------------------------------------------------|
| Compile  :                                                                   |
| Link     :                                                                   |
| Call     :                                                                   |
|------------------------------------------------------------------------------|
| Author   : SLL                                                               |
| Version  : V1.00                                                             |
| Creation : 2020/11/17                                                        |
| Revision :                                                                   |
+=============================================================================*/
#include "GeneralRW.h"
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
//##############################################################################
//
//      Enumeration
//
//##############################################################################
enum T_Result
{
    GENERALRW_RESULT_DONE,
    GENERALRW_RESULT_DOING,
    GENERALRW_RESULT_TIMEOUT,
    GENERALRW_RESULT_FAULT
};
enum TGeneralRWType
{
    GENERALRWTYPE_NONE,
    GENERALRWTYPE_ETHERCAT,
    GENERALRWTYPE_CAN
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
void  Init_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController, struct T_GeneralRWInfo * pGeneralRWInfo); // , T_FuncRead funcRead, T_FuncWrite funcWrite);
void  Init_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController, struct T_GeneralRWInfo * pGeneralRWInfo);  // , T_FuncRead funcRead, T_FuncWrite funcWrite);
void  Run_GeneralRWController(struct T_GeneralRWController * pGeneralRWController);
void  Tired_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController);
void  Tired_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController);
BOOL  GetTired_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController);
BOOL  GetTired_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController);
DWORD GetChannel_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController);
DWORD GetChannel_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController);
WORD  ReadWriteState_GeneralRWController(struct T_GeneralRWController * pGeneralRWController);
WORD  ReadReadState_GeneralRWController(struct T_GeneralRWController * pGeneralRWController);
void  WriteWriteID_GeneralRWController(struct T_GeneralRWController * pGeneralRWController, DWORD iID);
void  WriteReadID_GeneralRWController(struct T_GeneralRWController * pGeneralRWController, DWORD iID);
//void  CheckPackge_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController,union T_GeneralRWLength dwLength);//shanll 20210126 modify
BOOL  CheckPackge_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController,union T_GeneralRWLength dwLength);
BOOL  CheckPackge_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController,union T_GeneralRWLength dwLength);
void  WriteValue_GeneralRWController(struct T_GeneralRWController * pGeneralRWController,WORD  wDevIndex);
void  ReadValue_GeneralRWController(struct T_GeneralRWController * pGeneralRWController, WORD wDevIndex);
void  SetWriteGeneralRWType(struct T_GeneralRWController * pGeneralRWController);
void  SetReadGeneralRWType(struct T_GeneralRWController * pGeneralRWController);

void GeneralRW_Write(struct T_GeneralRWController * pGeneralRWController);
void GeneralRW_Read(struct T_GeneralRWController * pGeneralRWController);

enum T_Result GeneralRW_ReadGeneralRW(union T_GeneralRWType uWatchType, union T_GeneralRWIndex uWatchID, DWORD dwlength, DWORD * pValue);
enum T_Result GeneralRW_WriteGeneralRW(union T_GeneralRWType uWatchType, union T_GeneralRWIndex uWatchID, DWORD dwlength, DWORD const * pValue);
BOOL          GeneralRW_SearchWriteItem(struct T_GeneralRWController * pGeneralRWController);
BOOL          GeneralRW_SearchReadItem(struct T_GeneralRWController * pGeneralRWController);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
struct
{
    DBIndex_T iWriteNumOfEntries;
    DBIndex_T iReadNumOfEntries;
} const GeneralRW__base_index[] = {
    { DBINDEX_M28_GENERAL_WRITE1_COMM_PARAM, DBINDEX_M28_GENERAL_READ1_COMM_PARAM },
    { DBINDEX_M28_GENERAL_WRITE2_COMM_PARAM, DBINDEX_M28_GENERAL_READ2_COMM_PARAM },
};

struct T_GeneralRWController GeneralRW__aController[sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)];
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Create_GeneralRW description]
 */
void Create_GeneralRW(void)
{
    WORD i;

    for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
    {
        Init_WriteGeneralRWController(&GeneralRW__aController[i], (void *)DB_GetDataAddr(GeneralRW__base_index[i].iWriteNumOfEntries));
        DB_SetDWORD(GeneralRW__base_index[i].iWriteNumOfEntries, sizeof(struct T_GeneralRWInfo) / sizeof(DWORD));
        Init_ReadGeneralRWController(&GeneralRW__aController[i], (void *)DB_GetDataAddr(GeneralRW__base_index[i].iReadNumOfEntries));
        DB_SetDWORD(GeneralRW__base_index[i].iReadNumOfEntries, sizeof(struct T_GeneralRWInfo) / sizeof(DWORD));
    }
}

/**
 * [Run_GeneralRW description]
 */
void Run_GeneralRW(void)
{
    struct T_GeneralRWController* pController;
    WORD i;

    for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); i++)
    {
        pController = GeneralRW__aController + i;
        Run_GeneralRWController(pController);
        if (!GetTired_WriteGeneralRWController(pController)) {
            WORD j;

            for (j = i + 1; j < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++j) {
                if (GetChannel_WriteGeneralRWController(pController) == GetChannel_WriteGeneralRWController(GeneralRW__aController + j)) {
                    break;
                }
            }
            if (j < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index))) {
                break;
            }
            for (j = 0; j < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++j) {
                if (GetChannel_WriteGeneralRWController(pController) == GetChannel_WriteGeneralRWController(GeneralRW__aController + j)) {
                    Tired_WriteGeneralRWController(GeneralRW__aController + j);
                }
            }
        }
        if (!GetTired_ReadGeneralRWController(pController)) {
            WORD j;

            for (j = i + 1; j < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++j) {
                if (GetChannel_ReadGeneralRWController(pController) == GetChannel_ReadGeneralRWController(GeneralRW__aController + j)) {
                    break;
                }
            }
            if (j < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index))) {
                break;
            }
            for (j = 0; j < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++j) {
                if (GetChannel_ReadGeneralRWController(pController) == GetChannel_ReadGeneralRWController(GeneralRW__aController + j)) {
                    Tired_ReadGeneralRWController(GeneralRW__aController + j);
                }
            }
        }
    }
}

/**
 * [DB_GeneralRW_Write_Type description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_Write_Type(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                if ((GeneralRW__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_GENERAL_WRITE1_COMM_PARAM - DBINDEX_M28_GENERAL_WRITE1_COMM_PARAM)) == tDBIndex)
                {
                    SetWriteGeneralRWType(&GeneralRW__aController[i]);
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
 * [DB_GeneralRW_Read_Type description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_Read_Type(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                if ((GeneralRW__base_index[i].iReadNumOfEntries + (DBINDEX_M28_GENERAL_READ1_COMM_PARAM - DBINDEX_M28_GENERAL_READ1_COMM_PARAM)) == tDBIndex)
                {
                    SetReadGeneralRWType(&GeneralRW__aController[i]);
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
 * [DB_GeneralRW_Write_State description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_Write_State(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                if ((GeneralRW__base_index[i].iWriteNumOfEntries + (DBINDEX_M28_GENERAL_WRITE1_DATA_STATUS - DBINDEX_M28_GENERAL_WRITE1_COMM_PARAM)) == tDBIndex)
                {
                    DBVALUE* pData;

                    ReadWriteState_GeneralRWController(&GeneralRW__aController[i]);
                    pData = (DBVALUE*)puDBData;
                    pData->vDWORD = GeneralRW__aController[i]._pWriteGeneralRWInfo->uStatus.iAll;
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
 * [DB_GeneralRW_Read_State description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_Read_State(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                if ((GeneralRW__base_index[i].iReadNumOfEntries + (DBINDEX_M28_GENERAL_READ1_DATA_STATUS - DBINDEX_M28_GENERAL_READ1_COMM_PARAM)) == tDBIndex)
                {
                    DBVALUE* pData;

                    ReadReadState_GeneralRWController(&GeneralRW__aController[i]);
                    pData = (DBVALUE*)puDBData;
                    pData->vDWORD = GeneralRW__aController[i]._pReadGeneralRWInfo->uStatus.iAll;
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
 * [DB_GeneralRW_WriteValue_Enable description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_WriteValue_Enable(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                //shanll 20210126 modify
                // WriteValue_GeneralRWController(&GeneralRW__aController[i],i);
                // CheckPackge_WriteGeneralRWController(&GeneralRW__aController[i],(union T_GeneralRWLength)puDBData->vDWORD);
                if(CheckPackge_WriteGeneralRWController(&GeneralRW__aController[i],(union T_GeneralRWLength)puDBData->vDWORD))
                    WriteValue_GeneralRWController(&GeneralRW__aController[i],i);
                break;
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}



/**
 * [DB_GeneralRW_ReadValue_Enable description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_ReadValue_Enable(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                if(CheckPackge_ReadGeneralRWController(&GeneralRW__aController[i],(union T_GeneralRWLength)puDBData->vDWORD))
                    ReadValue_GeneralRWController(&GeneralRW__aController[i],i);
                break;
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [Init_WriteGeneralRWController description]
 * @param pGeneralRWController [description]
 * @param pGeneralRWInfo       [description]
 */
void Init_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController, struct T_GeneralRWInfo * pGeneralRWInfo)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWInfo);
    if ((0 != pGeneralRWInfo->uStatus.sBit.iDataStatus) && (0 != pGeneralRWInfo->uGeneralRWType.sBit.iPackgeIndex))
    {
        pGeneralRWInfo = NULL;
        return;
    }
    pGeneralRWController->_pWriteGeneralRWInfo         = pGeneralRWInfo;
    pGeneralRWController->_uWriteGeneralRWType.iAll    = pGeneralRWInfo->uGeneralRWType.iAll;
    //pGeneralRWController->_sWrite.uID.iAll          = 0;
    //pGeneralRWController->_sWrite.iValue            = 0;
    pGeneralRWController->_bWriteFree               = TRUE;
    pGeneralRWController->_bWriteTired              = TRUE;
    pGeneralRWController->_iWriteTimeOutCount       = 0;
    pGeneralRWController->_iGeneralRWWriteIdChangeFlag = 0;
    pGeneralRWController->_iGeneralRWWriteFlag      = 0;
    pGeneralRWController->_iWriteEnable             = FALSE;

    if (0 != pGeneralRWInfo->uGeneralRWType.iAll)
        pGeneralRWInfo->uStatus.iAll = TRUE;
    else
        pGeneralRWInfo->uStatus.iAll = FALSE;
    pGeneralRWInfo->uStatus.sBit.iPackgeCount = pGeneralRWInfo->uGeneralRWType.sBit.iPackgeCount; // Anders 2016-12-08, Add.
    pGeneralRWInfo->uStatus.sBit.iPackgeIndex = pGeneralRWInfo->uGeneralRWType.sBit.iPackgeIndex; // Anders 2016-12-08, Add.
    memset(&pGeneralRWInfo->aGroup, 0, sizeof(pGeneralRWInfo->aGroup));
}

/**
 * [Init_ReadGeneralRWController description]
 * @param pGeneralRWController [description]
 * @param pGeneralRWInfo       [description]
 */
void Init_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController, struct T_GeneralRWInfo * pGeneralRWInfo)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWInfo);
    if ((0 != pGeneralRWInfo->uStatus.sBit.iDataStatus) && (0 != pGeneralRWInfo->uGeneralRWType.sBit.iPackgeIndex))
    {
        pGeneralRWInfo = NULL;
        return;
    }
    pGeneralRWController->_pReadGeneralRWInfo      = pGeneralRWInfo;
    pGeneralRWController->_uReadGeneralRWType.iAll = pGeneralRWInfo->uGeneralRWType.iAll;
    //pGeneralRWController->_sRead.uID.iAll       = 0;
    //pGeneralRWController->_sRead.iValue         = 0;
    pGeneralRWController->_bReadFree            = TRUE;
    pGeneralRWController->_bReadTired           = TRUE;
    pGeneralRWController->_iReadTimeOutCount    = 0;
    pGeneralRWController->_iGeneralRWReadFlag   = 0;
    pGeneralRWController->_iGeneralRWReadIdChangeFlag = 0;
    pGeneralRWController->_iReadEnable = FALSE;

    if (0 != pGeneralRWInfo->uGeneralRWType.iAll)
        pGeneralRWInfo->uStatus.iAll = TRUE;
    else
        pGeneralRWInfo->uStatus.iAll = FALSE;
    pGeneralRWInfo->uStatus.sBit.iPackgeCount = pGeneralRWInfo->uGeneralRWType.sBit.iPackgeCount; // Anders 2016-12-08, Add.
    pGeneralRWInfo->uStatus.sBit.iPackgeIndex = pGeneralRWInfo->uGeneralRWType.sBit.iPackgeIndex; // Anders 2016-12-08, Add.
    memset(&pGeneralRWInfo->aGroup, 0, sizeof(pGeneralRWInfo->aGroup));
}

/**
 * [Run_GeneralRWController description]
 * @param pGeneralRWController [description]
 */
void Run_GeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    // Write.
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    if (pGeneralRWController->_bWriteFree)
    {
        if (pGeneralRWController->_bWriteTired && GeneralRW_SearchWriteItem(pGeneralRWController))
        {
            pGeneralRWController->_iWriteTimeOutCount = 0;
            pGeneralRWController->_uWriteGeneralRWType   = pGeneralRWInfo->uGeneralRWType;
            //pGeneralRWController->_sWrite             = pGeneralRWInfo->aGroup[pGeneralRWController->_iWriteCurIndex];
            pGeneralRWController->_bWriteFree         = FALSE;
            GeneralRW_Write(pGeneralRWController);
        }
    }
    else
        GeneralRW_Write(pGeneralRWController);

    // hankin 20190718 mask: To prevent when the read operation is locked, the write operation can only be successfully processed after the read operation has timed out.
    //if (FALSE == pGeneralRWController->_bWriteFree)
    //    return;

    // Read.
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    if (pGeneralRWController->_bReadFree)
    {
        if (pGeneralRWController->_bReadTired && GeneralRW_SearchReadItem(pGeneralRWController))
        {
            pGeneralRWController->_iReadTimeOutCount = 0;
            pGeneralRWController->_uReadGeneralRWType   = pGeneralRWInfo->uGeneralRWType;
            //pGeneralRWController->_sRead             = pGeneralRWInfo->aGroup[pGeneralRWController->_iReadCurIndex];
            pGeneralRWController->_bReadFree         = FALSE;
            GeneralRW_Read(pGeneralRWController);
        }
    }
    else
        GeneralRW_Read(pGeneralRWController);
}

/**
 * [Tired_WriteGeneralRWController description]
 * @author hankin (2019/11/25)
 * @param  pGeneralRWController [description]
 */
void Tired_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    pGeneralRWController->_bWriteTired = TRUE;
}

/**
 * [Tired_ReadGeneralRWController description]
 * @author hankin (2019/11/25)
 * @param  pGeneralRWController [description]
 */
void Tired_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    pGeneralRWController->_bReadTired = TRUE;
}

/**
 * [GetTired_WriteGeneralRWController description]
 * @author hankin (2019/11/25)
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
BOOL GetTired_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    return pGeneralRWController->_bWriteTired;
}

/**
 * [GetTired_ReadGeneralRWController description]
 * @author hankin (2019/11/25)
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
BOOL GetTired_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    return pGeneralRWController->_bReadTired;
}

/**
 * [GetChannel_WriteGeneralRWController description]
 * @author hankin (2019/11/25)
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
DWORD GetChannel_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    return (pGeneralRWController->_uWriteGeneralRWType.sBit.iChannel << 8)
        | pGeneralRWController->_uWriteGeneralRWType.sBit.iType;
}

/**
 * [GetChannel_ReadGeneralRWController description]
 * @author hankin (2019/11/25)
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
DWORD GetChannel_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    DEBUG_ASSERT(NULL != pGeneralRWController);
    return (pGeneralRWController->_uReadGeneralRWType.sBit.iChannel << 8)
        | pGeneralRWController->_uReadGeneralRWType.sBit.iType;
}

/**
 * [ReadWriteState_GeneralRWController description]
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
WORD ReadWriteState_GeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;
    WORD                  iDataStatus;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pWriteGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    iDataStatus = pGeneralRWInfo->uStatus.sBit.iDataStatus;
    if(FALSE == iDataStatus)
        pGeneralRWController->_iGeneralRWWriteIdChangeFlag = FALSE;
    return iDataStatus;
}

/**
 * [ReadReadState_GeneralRWController description]
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
WORD ReadReadState_GeneralRWController(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;
    WORD                  iDataStatus;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pReadGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    iDataStatus = pGeneralRWInfo->uStatus.sBit.iDataStatus;
    if(FALSE == iDataStatus)
        pGeneralRWController->_iGeneralRWReadIdChangeFlag = FALSE;
    return iDataStatus;
}

/**
 * [DB_GeneralRW_Write_Index description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_Write_Index(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                WriteWriteID_GeneralRWController(&GeneralRW__aController[i], puDBData->vDWORD);
                break;
            }
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_GeneralRW_Read_Index description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_GeneralRW_Read_Index(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const * puDBData)
{
    unsigned i;

    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            for (i = 0; i < (sizeof(GeneralRW__base_index) / sizeof(*GeneralRW__base_index)); ++i)
            {
                WriteReadID_GeneralRWController(&GeneralRW__aController[i], puDBData->vDWORD);
                break;
            }
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [WriteWriteID_GeneralRWController description]
 * @param pGeneralRWController [description]
 * @param iID               [description]
 */
void WriteWriteID_GeneralRWController(struct T_GeneralRWController * pGeneralRWController, DWORD iID)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pWriteGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    if (0 == pGeneralRWInfo->uGeneralRWType.iAll)
        return;
    if (0 == iID)
        return;
    if (FALSE == pGeneralRWInfo->uStatus.sBit.iDataStatus)
        return;
    if (TRUE == pGeneralRWController->_iGeneralRWWriteIdChangeFlag) {
        pGeneralRWInfo->uIndex = pGeneralRWController->_uWriteIndex;
        return;
    }
    // pGeneralRWInfo->writeStatus.sBit.iExecStatus &= ~mask;
    pGeneralRWInfo->uIndex.iAll = iID;
    memset(pGeneralRWInfo->aGroup,0,sizeof(pGeneralRWInfo->aGroup));
    // pGeneralRWInfo->uPackge.sBit.iRxIDStatus |= mask;

    pGeneralRWController->_uWriteIndex = pGeneralRWInfo->uIndex;
    pGeneralRWController->_iGeneralRWWriteIdChangeFlag = TRUE;
}

/**
 * [WriteReadID_GeneralRWController description]
 * @param pGeneralRWController [description]
 * @param iID               [description]
 */
void WriteReadID_GeneralRWController(struct T_GeneralRWController * pGeneralRWController, DWORD iID)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pReadGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    if (0 == pGeneralRWInfo->uGeneralRWType.iAll)
        return;
    if (0 == iID)
        return;
    if (FALSE == pGeneralRWInfo->uStatus.sBit.iDataStatus)
        return;
    if (TRUE == pGeneralRWController->_iGeneralRWReadIdChangeFlag) {
        pGeneralRWInfo->uIndex = pGeneralRWController->_uReadIndex;
        return;
    }
    // pGeneralRWInfo->uPackge.sBit.iExecStatus &= ~mask;
    //pGeneralRWInfo->dwStatusBackup = pGeneralRWInfo->uStatus.iAll;
    pGeneralRWInfo->uIndex.iAll = iID;
    memset(pGeneralRWInfo->aGroup,0,sizeof(pGeneralRWInfo->aGroup));

    pGeneralRWController->_uReadIndex = pGeneralRWInfo->uIndex;
    pGeneralRWController->_iGeneralRWReadIdChangeFlag = TRUE;
}

/**
 * [CheckPackge_WriteGeneralRWController description]
 * @param pGeneralRWController [description]
 * @param uLength             [description]
 */
BOOL CheckPackge_WriteGeneralRWController(struct T_GeneralRWController * pGeneralRWController,union T_GeneralRWLength uLength)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pWriteGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    if ((0 == pGeneralRWInfo->uGeneralRWType.iAll)||(uLength.sBit.iPackgeIndex != pGeneralRWInfo->uStatus.sBit.iPackgeIndex)||(FALSE == pGeneralRWInfo->uStatus.sBit.iDataStatus))
        return FALSE;
    if (FALSE == pGeneralRWController->_iGeneralRWWriteIdChangeFlag) {
        memcpy(pGeneralRWInfo->aGroup,pGeneralRWController->_aWriteBackup,sizeof(pGeneralRWInfo->aGroup));
        pGeneralRWInfo->uLength = pGeneralRWController->_uReadLength;
        return FALSE;
    }
    // if (iValue != pGeneralRWInfo->writeGroup[iIndex].iValue)
    pGeneralRWInfo->uLength = uLength;
    pGeneralRWController->_uWriteLength = uLength;
    pGeneralRWController->_iGeneralRWWriteFlag = TRUE;
    return TRUE;
}

/**
 * [CheckPackge_ReadGeneralRWController description]
 * @param pGeneralRWController [description]
 * @param uLength             [description]
 */
BOOL CheckPackge_ReadGeneralRWController(struct T_GeneralRWController * pGeneralRWController,union T_GeneralRWLength uLength)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pReadGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    if ((0 == pGeneralRWInfo->uGeneralRWType.iAll)||(uLength.sBit.iPackgeIndex != pGeneralRWInfo->uStatus.sBit.iPackgeIndex)||(FALSE == pGeneralRWInfo->uStatus.sBit.iDataStatus))
        return FALSE;
    if (FALSE == pGeneralRWController->_iGeneralRWReadIdChangeFlag) {
        memcpy(pGeneralRWInfo->aGroup,pGeneralRWController->_aReadBackup,sizeof(pGeneralRWInfo->aGroup));
        pGeneralRWInfo->uLength = pGeneralRWController->_uReadLength;
        return FALSE;
    }
    // if (iValue != pGeneralRWInfo->ReadGroup[iIndex].iValue)
    pGeneralRWInfo->uLength = uLength;
    pGeneralRWController->_uReadLength = uLength;
    pGeneralRWController->_iGeneralRWReadFlag = TRUE;
    return TRUE;
}

/**
 * [WriteValue_GeneralRWController description]
 * @param pGeneralRWController [description]
 * @param wDevIndex             [description]
 */
void WriteValue_GeneralRWController(struct T_GeneralRWController * pGeneralRWController,WORD  wDevIndex)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pWriteGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    // if (0 == pGeneralRWInfo->uGeneralRWType.iAll)
    //     return;
    // if (FALSE == pGeneralRWController->_iGeneralRWWriteIdChangeFlag) {
    //     memcpy(pGeneralRWInfo->aGroup,pGeneralRWController->_aWriteBackup,sizeof(pGeneralRWInfo->aGroup));
    //     return;
    // }
    // if (FALSE == pGeneralRWInfo->uStatus.sBit.iDataStatus)
    //     return;
    // memcpy(pGeneralRWInfo->aGroup,&g_sDB.M28.GENERAL_WRITE1.DATA_1_VALUE+GENERALRW_GROUP_SUM*wDevIndex,sizeof(pGeneralRWInfo->aGroup));
    memcpy(pGeneralRWController->_aWriteBackup,pGeneralRWInfo->aGroup,sizeof(pGeneralRWInfo->aGroup));
    pGeneralRWController->_iWriteEnable = TRUE;
}

/**
 * [ReadValue_GeneralRWController description]
 * @param pGeneralRWController [description]
 * @param wDevIndex             [description]
 */
void ReadValue_GeneralRWController(struct T_GeneralRWController * pGeneralRWController, WORD wDevIndex)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pReadGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    // if (0 == pGeneralRWInfo->uGeneralRWType.iAll)
    //     return;
    // if (FALSE == pGeneralRWController->_iGeneralRWReadIdChangeFlag) {
    //     memcpy(pGeneralRWInfo->aGroup,pGeneralRWController->_aWriteBackup,sizeof(pGeneralRWInfo->aGroup));
    //     return;
    // }
    // if (FALSE == pGeneralRWInfo->uStatus.sBit.iDataStatus)
    //     return;
    // memcpy(pGeneralRWInfo->aGroup,&g_sDB.M28.GENERAL_READ1.DATA_1_VALUE+GENERALRW_GROUP_SUM*wDevIndex,sizeof(pGeneralRWInfo->aGroup));
    memcpy(pGeneralRWController->_aReadBackup,pGeneralRWInfo->aGroup,sizeof(pGeneralRWInfo->aGroup));
    pGeneralRWController->_iReadEnable = TRUE;
}

/**
 * [GeneralRW_Write description]
 * @param pGeneralRWController [description]
 */
void GeneralRW_Write(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;
    union  T_GeneralRWIndex  uIndex;
    union T_GeneralRWLength  uLength;
    DWORD* pGroup;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(!pGeneralRWController->_bWriteFree);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pWriteGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    DEBUG_ASSERT(NULL != pGeneralRWInfo);
    uIndex = pGeneralRWController->_uWriteIndex;
    uLength = pGeneralRWController->_uWriteLength;
    pGroup = pGeneralRWController->_aWriteBackup;
    if(pGeneralRWController->_iWriteEnable != TRUE)
        return;
    switch (GeneralRW_WriteGeneralRW(pGeneralRWController->_pWriteGeneralRWInfo->uGeneralRWType, uIndex, uLength.sBit.iLength, pGroup))
    {
        case GENERALRW_RESULT_DONE:
            pGeneralRWInfo->uStatus.sBit.iDataStatus = FALSE;
            pGeneralRWController->_bWriteFree = TRUE;
            pGeneralRWController->_iWriteEnable = FALSE;
            break;
        case GENERALRW_RESULT_FAULT:
            pGeneralRWInfo->aGroup[0] = *pGroup;
            switch (*pGroup)
            {
                case 0x06020000:
                    //shanll 20210109 modify
                    pGeneralRWInfo->uStatus.sBit.iDataStatus = FALSE;
                    pGeneralRWController->_bWriteFree = TRUE;
                    // Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_WATCH_ODOBJ_NOTEXSITE, pGeneralRWController->_uWriteGeneralRWType.sBit.iNodeId_slaveIndex);
                    break;
                case 0x06090011:
                    //shanll 20210111 modify
                    pGeneralRWInfo->uStatus.sBit.iDataStatus = FALSE;
                    pGeneralRWController->_bWriteFree = TRUE;
                    // Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE, pGeneralRWController->_uWriteGeneralRWType.sBit.iNodeId_slaveIndex);
                    break;
                default:
                    Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_WATCH_DATA_WRITE, pGeneralRWController->_uWriteGeneralRWType.sBit.iNodeId_slaveIndex);
                    break;
            }
            pGeneralRWController->_bWriteFree = TRUE;
            break;
        case GENERALRW_RESULT_TIMEOUT:
            if (++pGeneralRWController->_iWriteTimeOutCount > 255)
            {
                pGeneralRWController->_iWriteTimeOutCount = 0;
                pGeneralRWController->_bWriteFree         = TRUE;
            }
            break;
        default:
            //if( pGeneralRWController->_bWriteFree == TRUE)
            //  ESTOP0;
            break;
    }

    //if (pGeneralRWController->_bWriteFree)
    //  if (0x27D2 == pGeneralRWController->_pWriteGeneralRWInfo->aGroup[pGeneralRWController->_iWriteCurIndex].uID.sBit.iIndex)
    //      ESTOP0;
}

/**
 * [GeneralRW_Read description]
 * @param pGeneralRWController [description]
 */
void GeneralRW_Read(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;
    union  T_GeneralRWIndex  uIndex;
    union T_GeneralRWLength  uLength;
    DWORD* pGroup;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(!pGeneralRWController->_bReadFree);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pReadGeneralRWInfo);
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    DEBUG_ASSERT(NULL != pGeneralRWInfo);
    uIndex = pGeneralRWController->_uReadIndex;
    uLength = pGeneralRWController->_uReadLength;
    pGroup = pGeneralRWController->_aReadBackup;
    if(pGeneralRWController->_iReadEnable != TRUE)
        return;
    switch (GeneralRW_ReadGeneralRW(pGeneralRWController->_pReadGeneralRWInfo->uGeneralRWType, uIndex, uLength.sBit.iLength, pGroup))
    {
        case GENERALRW_RESULT_FAULT:
            memset(&pGeneralRWInfo->aGroup,0,sizeof(pGeneralRWInfo->aGroup));
        // continue GENERALRW_RESULT_FAULT.
            break;
        case GENERALRW_RESULT_DONE:
            memcpy(&pGeneralRWInfo->aGroup,&pGroup,sizeof(pGeneralRWInfo->aGroup));
            pGeneralRWInfo->uStatus.sBit.iDataStatus = FALSE;
            pGeneralRWController->_bReadFree = TRUE;
            pGeneralRWController->_iReadEnable = FALSE;
            break;
        case GENERALRW_RESULT_TIMEOUT:
            if (++pGeneralRWController->_iReadTimeOutCount > 10)
            {
                pGeneralRWController->_iReadTimeOutCount = 0;
                pGeneralRWController->_bReadFree         = TRUE;
            }
            break;
        default:
            break;
    }
}

/**
 * [GeneralRW_SearchWriteItem description]
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
BOOL GeneralRW_SearchWriteItem(struct T_GeneralRWController * pGeneralRWController)
{
    ///struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(pGeneralRWController->_bWriteFree);
    DEBUG_ASSERT(NULL != pGeneralRWController->_pWriteGeneralRWInfo);
    if (0 != pGeneralRWController->_iGeneralRWWriteFlag)
    {
        pGeneralRWController->_iGeneralRWWriteFlag = FALSE;
        pGeneralRWController->_bWriteTired = TRUE;
        return TRUE;
    }
    return FALSE;
}

/**
 * [GeneralRW_SearchReadItem description]
 * @param  pGeneralRWController [description]
 * @return                   [description]
 */
BOOL GeneralRW_SearchReadItem(struct T_GeneralRWController * pGeneralRWController)
{
    //struct T_GeneralRWInfo * pGeneralRWInfo;

    DEBUG_ASSERT(NULL != pGeneralRWController);
    DEBUG_ASSERT(pGeneralRWController->_bReadFree);
    if (0 != pGeneralRWController->_iGeneralRWReadFlag)
    {
        pGeneralRWController->_iGeneralRWReadFlag = FALSE;
        pGeneralRWController->_bReadTired = TRUE;
        return TRUE;
    }
    return FALSE;
}

/**
 * [GeneralRW_ReadGeneralRW description]
 * @param  uWatchType [description]
 * @param  uWatchID   [description]
 * @param  pValue    [description]
 * @return           [description]
 */
enum T_Result GeneralRW_ReadGeneralRW(union T_GeneralRWType uWatchType, union T_GeneralRWIndex uWatchID, DWORD dwlength, DWORD * pValue)
{
    switch (uWatchType.sBit.iType)
    {
        case GENERALRWTYPE_CAN:
            switch (UploadCANSDO_Easy(uWatchType.sBit.iChannel, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, (WORD *)pValue, dwlength))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return GENERALRW_RESULT_DOING;
                case SDORESULT_DONE:
                    //if ( 57018 == uWatchID.sBit.iIndex-0x2000+50000 )
                    //{
                    //  ESTOP0;
                    //}
                    return GENERALRW_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    //ESTOP0;
                    return GENERALRW_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    //ESTOP0;
                    return GENERALRW_RESULT_FAULT;
                default:
                    break;
            }
            break;
        case GENERALRWTYPE_ETHERCAT:
            if (uWatchType.sBit.iNodeId_slaveIndex >= g_ECController.Master.wAppSlaveCount)
                return GENERALRW_RESULT_DOING;
            switch (ECMaster_UploadSDO(&g_ECController.Master, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, (WORD *)pValue, dwlength, FALSE))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return GENERALRW_RESULT_DOING;
                case SDORESULT_DONE:
                    return GENERALRW_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    return GENERALRW_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    return GENERALRW_RESULT_FAULT;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return GENERALRW_RESULT_DOING;
}

/**
 * [GeneralRW_WriteGeneralRW description]
 * @param  uWatchType [description]
 * @param  uWatchID   [description]
 * @param  pValue     [description]
 * @return            [description]
 */
enum T_Result GeneralRW_WriteGeneralRW(union T_GeneralRWType uWatchType, union T_GeneralRWIndex uWatchID, DWORD dwlength, DWORD const * pValue)
{
    switch (uWatchType.sBit.iType)
    {
        case GENERALRWTYPE_CAN:
            switch (DownloadCANSDO_Easy(uWatchType.sBit.iChannel, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, *pValue, dwlength))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return GENERALRW_RESULT_DOING;
                case SDORESULT_DONE:
                    return GENERALRW_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    return GENERALRW_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    return GENERALRW_RESULT_FAULT;
                default:
                    break;
            }
            break;
        case GENERALRWTYPE_ETHERCAT:
            if (uWatchType.sBit.iNodeId_slaveIndex >= g_ECController.Master.wAppSlaveCount)
                return GENERALRW_RESULT_DOING;
            switch (ECMaster_DownloadSDO(&g_ECController.Master, uWatchType.sBit.iNodeId_slaveIndex, uWatchID.sBit.iIndex, uWatchID.sBit.iSubIndex, (WORD *)pValue, dwlength, FALSE))
            {
                case SDORESULT_NONE:
                case SDORESULT_BUSY:
                    return GENERALRW_RESULT_DOING;
                case SDORESULT_DONE:
                    return GENERALRW_RESULT_DONE;
                case SDORESULT_TIMEOUT:
                    return GENERALRW_RESULT_TIMEOUT;
                case SDORESULT_SLAVEABORT:
                    return GENERALRW_RESULT_FAULT;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return GENERALRW_RESULT_DOING;
}

/**
 * [SetWriteGeneralRWType description]
 * @param pGeneralRWController [description]
 */
void SetWriteGeneralRWType(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;
    // Write.
    pGeneralRWInfo = pGeneralRWController->_pWriteGeneralRWInfo;
    if (pGeneralRWController->_uWriteGeneralRWType.iAll != pGeneralRWInfo->uGeneralRWType.iAll)
        Init_WriteGeneralRWController(pGeneralRWController, pGeneralRWController->_pWriteGeneralRWInfo);
}

/**
 * [SetReadGeneralRWType description]
 * @param pGeneralRWController [description]
 */
void SetReadGeneralRWType(struct T_GeneralRWController * pGeneralRWController)
{
    struct T_GeneralRWInfo * pGeneralRWInfo;
    // Read.
    pGeneralRWInfo = pGeneralRWController->_pReadGeneralRWInfo;
    if (pGeneralRWController->_uReadGeneralRWType.iAll != pGeneralRWInfo->uGeneralRWType.iAll)
        Init_ReadGeneralRWController(pGeneralRWController, pGeneralRWController->_pReadGeneralRWInfo);
}
