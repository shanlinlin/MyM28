/*=============================================================================+
| Function : Reaxroth                                                          |
| Task     : Reaxroth Header File                                              |
|------------------------------------------------------------------------------|
| Compile  :                                                                   |
| Link     :                                                                   |
| Call     :                                                                   |
|------------------------------------------------------------------------------|
| Author   : SLL                                                               |
| Version  : V1.00                                                             |
| Creation : 2020/12/28                                                        |
| Revision :                                                                   |
+=============================================================================*/
#include "Rexroth.h"
#include "Error_App.h"
#include "database.h"
#include "CommComp_CANOpenMaster.h"
#include "task_ethercatmaster.h"
#include "common.h"
#include "debug.h"
#include "CommComp_RemoteIO.h"
#include <string.h>
//##############################################################################
//
//      Definition
//
//##############################################################################
#define     REXROTHRAEDINDEX          0x356D
#define     REXROTHENABLESUBINDEX     0x0A
#define     REXROTHRWSUBINDEX         0x0B
#define     REXROTH_READALLOWED       0
#define     REXROTH_WRITEALLOWED      1
#define     REXROTH_READCYCLE         3000
//##############################################################################
//
//      Enumeration
//
//##############################################################################
enum T_Result
{
    REXROTH_RESULT_DONE,
    REXROTH_RESULT_DOING,
    REXROTH_RESULT_TIMEOUT,
    REXROTH_RESULT_FAULT
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
void    Init_DataRexrothController(struct T_RexrothController * pRexrothController, struct T_RexrothInfo * pRexrothInfo);  // , T_FuncRead funcRead, T_FuncWrite funcWrite);
void    Init_RexrothControllerData(void);
void    Run_RexrothController(struct T_RexrothController * pRexrothController);
void    Rexroth_WriteHandler(struct T_RexrothController * pRexrothController);
void    Rexroth_Write(struct T_RexrothController * pRexrothController);
void    Rexroth_Read(struct T_RexrothController * pRexrothController);
enum    T_Result Rexroth_ReadRexroth(union T_RexrothDataType uWatchType, WORD wIndex, WORD wSubIndex,  DWORD dwlength, DWORD * pValue);
enum    T_Result Rexroth_WriteRexroth(union T_RexrothDataType uWatchType, WORD wIndex, WORD wSubIndex,  DWORD dwlength, DWORD const * pValue);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
struct
{
    DBIndex_T iRexrothDataEntries;
    DBIndex_T iRexrothDataEnd;
} const Rexroth__base_index =
    { DBINDEX_REXROTH_COMM_PARAM, DBINDEX_REXROTH_DATAVALUE121 };

struct T_RexrothController Rexroth__aController;
DWORD   u_dwReadEnter   = 0;
DWORD   u_dwWriteEnter  = 0;
DWORD   u_dwWriteValue  = 0;
BOOL    u_bTestFlag     = 0;
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Create_Rexroth description]
 */
void Create_Rexroth(void)
{
    Init_DataRexrothController(&Rexroth__aController, (void *)DB_GetDataAddr(Rexroth__base_index.iRexrothDataEntries));
    DB_SetDWORD(Rexroth__base_index.iRexrothDataEntries, sizeof(struct T_RexrothInfo) / sizeof(DWORD));
    Init_RexrothControllerData();
}

void Init_RexrothControllerData(void)
{
    int i;
    Rexroth__aController._uRexrothDataType.iAll = 0;
    for(i=0; i<REXROTH_SDODATA_SUM; i++)
    {
        Rexroth__aController._aReadBackup[i] = 0xFFFFFFFF;
    }
    Rexroth__aController._bReadEnable = FALSE;
    Rexroth__aController.wReadCurrentIndex = 0;
    Rexroth__aController.bReadLocationEnd = FALSE;
    ResetTimer(&Rexroth__aController.lReadCounter, 0);
    Rexroth__aController._bWriteEnable = FALSE;
    Rexroth__aController.wWriteCurrentIndex = 0;
    Rexroth__aController.wWriteContinuousCount = 0;
    Rexroth__aController.wReadOrWrite = REXROTH_WRITEALLOWED;//REXROTH_READALLOWED;
    Rexroth__aController.bLinkStateLast = FALSE;
}

/**
 * [Run_Rexroth description]
 */
void Run_Rexroth(void)
{
    WORD	wLinkState;
    if (u_wRexrothCount == 0)
        return;
    struct T_RexrothController* pController;

    pController = &Rexroth__aController;
    wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
    if(((wLinkState&(1<<pController->_pRexrothDataInfo->uRexrothReadType.sBit.iNodeId_slaveIndex))==FALSE)&&(Rexroth__aController.bLinkStateLast == TRUE))
        Init_RexrothControllerData();
    else
    {
        Run_RexrothController(pController);
        pController->bLinkStateLast = TRUE;
    }

}

/**
 * [Init_DataRexrothController description]
 * @param pRexrothController [description]
 * @param pRexrothInfo       [description]
 */
void Init_DataRexrothController(struct T_RexrothController * pRexrothController, struct T_RexrothInfo * pRexrothInfo)
{
    DEBUG_ASSERT(NULL != pRexrothController);
    DEBUG_ASSERT(NULL != pRexrothInfo);

    pRexrothController->_pRexrothDataInfo = pRexrothInfo;
    memset(&pRexrothInfo->aGroup, 0, sizeof(pRexrothInfo->aGroup));
}

/**
 * [Run_RexrothController description]
 * @param pRexrothController [description]
 */
void Run_RexrothController(struct T_RexrothController * pRexrothController)
{
//    struct T_RexrothInfo * pRexrothInfo;

    DEBUG_ASSERT(NULL != pRexrothController);
    // Write.
    if(Rexroth__aController.wReadOrWrite == REXROTH_WRITEALLOWED)
    {
        if(pRexrothController->wWriteContinuousCount == 0)
            Rexroth_WriteHandler(pRexrothController);//处理下发的定位和个数
        else
            Rexroth_Write(pRexrothController);
    }
    // Read.
    else if(Rexroth__aController.wReadOrWrite == REXROTH_READALLOWED)
        Rexroth_Read(pRexrothController);
}

/**
 * [Rexroth_WriteHandler description]
 * @param Rexroth_WriteHandler [description]
 */
void Rexroth_WriteHandler(struct T_RexrothController * pRexrothController)
{
    WORD i = 0;
    struct T_RexrothInfo * pRexrothInfo;

    if((pRexrothController->_bWriteEnable == TRUE)||(pRexrothController->wWriteContinuousCount > 0))//上一次的还没发送完
        return;
    DEBUG_ASSERT(NULL != pRexrothController);
    DEBUG_ASSERT(NULL != pRexrothController->_pRexrothDataInfo);
    pRexrothInfo = pRexrothController->_pRexrothDataInfo;
    DEBUG_ASSERT(NULL != pRexrothInfo);

    for(i=0; i<REXROTH_SDODATA_SUM; i++)
    {
        if(pRexrothInfo->aGroup[i] != pRexrothController->_aReadBackup[i])
        {
            if(pRexrothController->wWriteContinuousCount == 0)
                pRexrothController->wWriteCurrentIndex = i;
            pRexrothController->wWriteContinuousCount++;
        }
        else if(pRexrothController->wWriteContinuousCount > 0)
            break;
    }
    if(pRexrothController->wWriteContinuousCount == 0)//没有写入任务
        Rexroth__aController.wReadOrWrite = REXROTH_READALLOWED;
}

/**
 * [Rexroth_Write description]
 * @param pRexrothController [description]
 */
void Rexroth_Write(struct T_RexrothController * pRexrothController)
{
    struct T_RexrothInfo * pRexrothInfo;

    DEBUG_ASSERT(NULL != pRexrothController);
    DEBUG_ASSERT(pRexrothController->wWriteCurrentIndex  < REXROTH_SDODATA_SUM);
    DEBUG_ASSERT(pRexrothController->wWriteCurrentIndex + pRexrothController->wWriteContinuousCount <= REXROTH_SDODATA_SUM);
    DEBUG_ASSERT(NULL != pRexrothController->_pRexrothDataInfo);
    pRexrothInfo = pRexrothController->_pRexrothDataInfo;
    DEBUG_ASSERT(NULL != pRexrothInfo);
    u_dwWriteValue = pRexrothInfo->aGroup[pRexrothController->wWriteCurrentIndex];
    u_dwWriteEnter = pRexrothController->wWriteCurrentIndex;
    if((pRexrothController->wWriteContinuousCount > 0)&&(pRexrothController->_bWriteEnable != TRUE))
    {
        switch (Rexroth_WriteRexroth(pRexrothInfo->uRexrothReadType, REXROTHRAEDINDEX, REXROTHENABLESUBINDEX, sizeof(DWORD)<<1, &u_dwWriteEnter))
        {
            case REXROTH_RESULT_DONE:
                pRexrothController->_bWriteEnable = TRUE;
                break;
            case REXROTH_RESULT_FAULT:
                break;
            case REXROTH_RESULT_TIMEOUT:
                if (++pRexrothController->_iWriteTimeOutCount > 255)
                    pRexrothController->_iWriteTimeOutCount = 0;
                break;
            default:
                break;
        }
    }
    if((pRexrothController->_bWriteEnable == TRUE)&&(pRexrothController->wWriteContinuousCount > 0))
    {
        switch (Rexroth_WriteRexroth(pRexrothInfo->uRexrothReadType, REXROTHRAEDINDEX, REXROTHRWSUBINDEX, sizeof(DWORD)<<1, &u_dwWriteValue))
        {
            case REXROTH_RESULT_DONE:
                pRexrothController->_aReadBackup[pRexrothController->wWriteCurrentIndex] = pRexrothInfo->aGroup[pRexrothController->wWriteCurrentIndex];
                ++pRexrothController->wWriteCurrentIndex;
                --pRexrothController->wWriteContinuousCount;
                break;
            case REXROTH_RESULT_FAULT:
                break;
            case REXROTH_RESULT_TIMEOUT:
                if (++pRexrothController->_iWriteTimeOutCount > 255)
                    pRexrothController->_iWriteTimeOutCount = 0;
                break;
            default:
                break;
        }
    }
    if(pRexrothController->wWriteContinuousCount == 0)
        pRexrothController->_bWriteEnable = FALSE;
    if(pRexrothController->wWriteCurrentIndex >= REXROTH_SDODATA_SUM)
    {
        pRexrothController->wWriteCurrentIndex = 0;
        pRexrothController->_bWriteEnable = FALSE;
    }
}

/**
 * [Rexroth_Read description]
 * @param pRexrothController [description]
 */
void Rexroth_Read(struct T_RexrothController * pRexrothController)
{
    struct T_RexrothInfo * pRexrothInfo;
    DWORD*  pGroup;

    u_dwReadEnter = 0;
    DEBUG_ASSERT(NULL != pRexrothController);
    DEBUG_ASSERT(pRexrothController->wReadCurrentIndex < REXROTH_SDODATA_SUM);
    DEBUG_ASSERT(NULL != pRexrothController->_pRexrothDataInfo);
    pRexrothInfo = pRexrothController->_pRexrothDataInfo;
    DEBUG_ASSERT(NULL != pRexrothInfo);
    pGroup = &pRexrothController->_aReadBackup[pRexrothController->wReadCurrentIndex];

    if(((pRexrothController->_pRexrothDataInfo->uRexrothReadType.sBit.iType == FALSE)&&(pRexrothController->_bReadEnable != TRUE))||\
    ((pRexrothController->bReadLocationEnd == TRUE)&&(pRexrothController->wReadCurrentIndex == 0)&&(pRexrothController->_bReadEnable != TRUE)&&!CheckTimerOut(&Rexroth__aController.lReadCounter)))//没有读取需求或者本次周期内读取任务已完成
    {
        Rexroth__aController.wReadOrWrite = REXROTH_WRITEALLOWED;
        return;
    }

    if((pRexrothController->_pRexrothDataInfo->uRexrothReadType.sBit.iType == TRUE)&&(pRexrothController->_bReadEnable != TRUE))
    {
        if((pRexrothController->bReadLocationEnd == TRUE)&&(pRexrothController->wReadCurrentIndex == 0)&&CheckTimerOut(&Rexroth__aController.lReadCounter))
        {
            pRexrothController->bReadLocationEnd = FALSE;
        }
    }

    if((pRexrothController->bReadLocationEnd == FALSE)&&(pRexrothController->wReadCurrentIndex == 0)&&(pRexrothController->_bReadEnable != TRUE))
    {
        switch (Rexroth_WriteRexroth(pRexrothController->_pRexrothDataInfo->uRexrothReadType, REXROTHRAEDINDEX, REXROTHENABLESUBINDEX, sizeof(DWORD)<<1, &u_dwReadEnter))
        {
            case REXROTH_RESULT_DONE:
                pRexrothController->_bReadEnable = TRUE;
                pRexrothController->bReadLocationEnd = TRUE;
                break;
            case REXROTH_RESULT_FAULT:
                break;
            case REXROTH_RESULT_TIMEOUT:
                if (++pRexrothController->_iWriteTimeOutCount > 255)
                    pRexrothController->_iWriteTimeOutCount = 0;
                break;
            default:
                break;
        }
    }
    if(pRexrothController->_bReadEnable == TRUE)
    {
        switch (Rexroth_ReadRexroth(pRexrothController->_pRexrothDataInfo->uRexrothReadType, REXROTHRAEDINDEX, REXROTHRWSUBINDEX,sizeof(DWORD)<<1, pGroup))
        {
            case REXROTH_RESULT_FAULT:
                *pGroup = 0;
            // continue REXROTH_RESULT_FAULT.
            case REXROTH_RESULT_DONE:
                pRexrothInfo->aGroup[pRexrothController->wReadCurrentIndex] = *pGroup;
                ++pRexrothController->wReadCurrentIndex;
                break;
            case REXROTH_RESULT_TIMEOUT:
                if (++pRexrothController->_iReadTimeOutCount > 10)
                    pRexrothController->_iReadTimeOutCount = 0;
                break;
            default:
                break;
        }
    }
    if(pRexrothController->wReadCurrentIndex >= REXROTH_SDODATA_SUM)
    {
        pRexrothController->wReadCurrentIndex = 0;
        pRexrothController->_bReadEnable = FALSE;
        ResetTimer(&Rexroth__aController.lReadCounter, REXROTH_READCYCLE);
    }
}

/**
 * [Rexroth_ReadRexroth description]
 * @param  uWatchType [description]
 * @param  uWatchID   [description]
 * @param  pValue    [description]
 * @return           [description]
 */
enum T_Result Rexroth_ReadRexroth(union T_RexrothDataType uWatchType, WORD wIndex, WORD wSubIndex, DWORD dwlength, DWORD * pValue)
{
    if (uWatchType.sBit.iNodeId_slaveIndex >= g_ECController.Master.wAppSlaveCount)
        return REXROTH_RESULT_DOING;
    switch (ECMaster_UploadSDO(&g_ECController.Master, uWatchType.sBit.iNodeId_slaveIndex, wIndex, wSubIndex, (WORD *)pValue, dwlength, FALSE))
    {
        case SDORESULT_NONE:
        case SDORESULT_BUSY:
            return REXROTH_RESULT_DOING;
        case SDORESULT_DONE:
            return REXROTH_RESULT_DONE;
        case SDORESULT_TIMEOUT:
            return REXROTH_RESULT_TIMEOUT;
        case SDORESULT_SLAVEABORT:
            return REXROTH_RESULT_FAULT;
        default:
            break;
    }
    return REXROTH_RESULT_DOING;
}

/**
 * [Rexroth_WriteRexroth description]
 * @param  uWatchType [description]
 * @param  uWatchID   [description]
 * @param  pValue     [description]
 * @return            [description]
 */
enum T_Result Rexroth_WriteRexroth(union T_RexrothDataType uWatchType, WORD wIndex, WORD wSubIndex, DWORD dwlength, DWORD const * pValue)
{
    if (uWatchType.sBit.iNodeId_slaveIndex >= g_ECController.Master.wAppSlaveCount)
        return REXROTH_RESULT_DOING;
    switch (ECMaster_DownloadSDO(&g_ECController.Master, uWatchType.sBit.iNodeId_slaveIndex, wIndex, wSubIndex, (WORD *)pValue, dwlength, FALSE))
    {
        case SDORESULT_NONE:
        case SDORESULT_BUSY:
            return REXROTH_RESULT_DOING;
        case SDORESULT_DONE:
            return REXROTH_RESULT_DONE;
        case SDORESULT_TIMEOUT:
            return REXROTH_RESULT_TIMEOUT;
        case SDORESULT_SLAVEABORT:
            return REXROTH_RESULT_FAULT;
        default:
            break;
    }
    return REXROTH_RESULT_DOING;
}

