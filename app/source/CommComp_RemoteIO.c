/*===========================================================================+
|  Function : RemoteIO                                                       |
|  Task     : RemoteIO Source File                                           |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                       |
|  Link     : CCS3.1 -                                                       |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   :                                                                |
|  Version  : V1.00                                                          |
|  Creation : 2015/01/08                                                     |
|  Revision :                                                                |
+===========================================================================*/
#include "CommComp_RemoteIO.h"
#include "CommComp_turntable.h"
#include "CommComp_Driver.h"
#include "CommComp_HTRobot.h"
#include "Device.h"
#include "Error_App.h"
#include "taskEtherCATMaster_app.h"
#include "task_ethercatmaster.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define RULER_HEARTBEAT_CYCLE 500
#define RULER_HEARTBEAT_LOSE_MAX 5
#define RULER_HEARTBEAT_CONFLICT_MAX 5
#define RULER_PDO_CYCLE psRemoteIO->wSendCycle
#define RULER_PDO_LOSE_MAX 50
#define RULER_PDO_CONFLICT_MAX 50

#define PICKER_HEARTBEAT_CYCLE 500
#define PICKER_HEARTBEAT_LOSE_MAX 5
#define PICKER_HEARTBEAT_CONFLICT_MAX 5
#define PICKER_PDO_CYCLE psRemoteIO->wSendCycle
#define PICKER_PDO_LOSE_MAX 50
#define PICKER_PDO_CONFLICT_MAX 50
#define PICKER_VERIFY_0x100000 0x00001001
#define PICKER_VERIFY_0x101801 0x000005BE
#define PICKER_VERIFY_0x101802 0x10010002

#define MT_HUB_8A_READ_ERROR_CYCLE 3000
#define MT_HUB_8A_HEARTBEAT_CYCLE 500
#define MT_HUB_8A_HEARTBEAT_LOSE_MAX 5
#define MT_HUB_8A_HEARTBEAT_CONFLICT_MAX 5
#define MT_HUB_8A_PDO_HEARTBEAT_CYCLE psRemoteIO->wSendCycle
#define MT_HUB_8A_PDO_LOSE_MAX 50
#define MT_HUB_8A_PDO_CONFLICT_MAX 50
#define MT_HUB_8A_VERIFY_0x100000 0x000A0196
//#define MT_HUB_8A_VERIFY_0x101801                                             0x000005BE
//#define MT_HUB_8A_VERIFY_0x101802                                             0x10010002

#define GERMANJET_READ_ERROR_CYCLE 500
#define GERMANJET_HEARTBEAT_CYCLE 500
#define GERMANJET_HEARTBEAT_LOSE_MAX 5
#define GERMANJET_HEARTBEAT_CONFLICT_MAX 5
#define GERMANJET_PDO_HEARTBEAT_CYCLE psRemoteIO->wSendCycle
#define GERMANJET_PDO_LOSE_MAX 50
#define GERMANJET_PDO_CONFLICT_MAX 50

#define TMIOT101_HEARTBEAT_CYCLE 500
#define TMIOT101_HEARTBEAT_LOSE_MAX 5
#define TMIOT101_HEARTBEAT_CONFLICT_MAX 5
#define TMIOT101_PDO_CYCLE psRemoteIO->wSendCycle
#define TMIOT101_PDO_LOSE_MAX 50
#define TMIOT101_PDO_CONFLICT_MAX 50
#define TMIOT101_VERIFY_0x100000 0x00001001
#define TMIOT101_VERIFY_0x101801 0x000005BE
#define TMIOT101_VERIFY_0x101802 0x10010001

#define TMIOT102_HEARTBEAT_CYCLE 500
#define TMIOT102_HEARTBEAT_LOSE_MAX 5
#define TMIOT102_HEARTBEAT_CONFLICT_MAX 5
#define TMIOT102_PDO_CYCLE psRemoteIO->wSendCycle
#define TMIOT102_PDO_LOSE_MAX 50
#define TMIOT102_PDO_CONFLICT_MAX 50
#define TMIOT102_VERIFY_0x100000 0x00001002
#define TMIOT102_VERIFY_0x101801 0x000005BE
#define TMIOT102_VERIFY_0x101802 0x10020001

#define TMIOT105_HEARTBEAT_CYCLE 500
#define TMIOT105_HEARTBEAT_LOSE_MAX 5
#define TMIOT105_HEARTBEAT_CONFLICT_MAX 5
#define TMIOT105_PDO_CYCLE psRemoteIO->wSendCycle
#define TMIOT105_PDO_LOSE_MAX 50
#define TMIOT105_PDO_CONFLICT_MAX 50
#define TMIOT105_VERIFY_0x100000 0x00030191
#define TMIOT105_VERIFY_0x101801 0x000005BE
#define TMIOT105_VERIFY_0x101802 0x00050001 // 0x10050001
#define TMIOT105_MASK_0x101802 0x00FFFFFF

#define TMIOT201_HEARTBEAT_CYCLE 500
#define TMIOT201_HEARTBEAT_LOSE_MAX 5
#define TMIOT201_HEARTBEAT_CONFLICT_MAX 5
#define TMIOT201_PDO_CYCLE psRemoteIO->wSendCycle
#define TMIOT201_PDO_LOSE_MAX 50
#define TMIOT201_PDO_CONFLICT_MAX 50
#define TMIOT201_VERIFY_0x100000 0x000F0191
#define TMIOT201_VERIFY_0x101801 0x000005BE
#define TMIOT201_VERIFY_0x101802 0x00050003 // 0x10050003
#define TMIOT201_MASK_0x101802 0x00FFFFFF

#define TMIOT202_HEARTBEAT_CYCLE 500
#define TMIOT202_HEARTBEAT_LOSE_MAX 5
#define TMIOT202_HEARTBEAT_CONFLICT_MAX 5
#define TMIOT202_PDO_CYCLE psRemoteIO->wSendCycle
#define TMIOT202_PDO_LOSE_MAX 50
#define TMIOT202_PDO_CONFLICT_MAX 50
#define TMIOT202_VERIFY_0x100000 0x00030191
#define TMIOT202_VERIFY_0x101801 0x000005BE
#define TMIOT202_VERIFY_0x101802 0x00050002 // 0x10050002
#define TMIOT202_MASK_0x101802 0x00FFFFFF

#define MAP_INPUT_SUM 2
#define MAP_OUTPUT_SUM 2
#define MAP_AD_SUM 6
#define MAP_DA_SUM 8//shanll 20210915 modify    2
#define MAP_POSI_SUM 1
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
typedef struct Struct_InputMap S_OutputMap;
typedef struct Struct_ADMap    S_DAMap;
//------------------------------------------------------------------------------
typedef struct Struct_InputMap
{
    S_CommRemoteIO* psRemote;
    WORD            bBYTEOfNum : 5;
} S_InputMap;

typedef struct Struct_ADMap
{
    S_CommRemoteIO* psRemote;
    WORD            bWORDOfNum : 4;
} S_ADMap;

typedef struct Struct_CANPosiMap
{
    S_CommRemoteIO* psRemote;
    WORD            bDWORDOfNum : 3;
} S_CANPosiMap;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void CommRemoteIO_SetCommType(S_CommRemoteIO* const psRemoteIO, E_CommType eCommType);
static void CommRemoteIO_SetDeviceType(S_CommRemoteIO* const psRemoteIO, E_RemoteIODeviceType eDeviceType);
static void CommRemoteIO_SetNodeID(S_CommRemoteIO* const psRemoteIO, NodeID_T tNodeID);

static void Comm_EventCallBack(CommComp_S* psComp, CommEvent_U* puEvent);

static void CANOpen_CustomFunc_Operation(CommComp_S* psComp, E_CustomFuncType eType);
static void CANOpen_EventCallBack(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);

//static int Ruler_CANOpen_ConfigSDOMsg    (CANOpenCompUnique_S *psCompUnique, CANOpenCommSDOMsg_S *psRetSDOMsg, WORD wConfigIndex);
static bool Picker_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool MT_HUB_8A_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool GERMANJET_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool tmIoT101_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool tmIoT102_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool tmIoT105_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool tmIoT201_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
static bool tmIoT202_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
S_InputMap   u_asInputMap[REMOTEIONUM_SUM * MAP_INPUT_SUM];
S_OutputMap  u_asOutputMap[REMOTEIONUM_SUM * MAP_OUTPUT_SUM];
S_ADMap      u_asADMap[REMOTEIONUM_SUM * MAP_AD_SUM];
S_DAMap      u_asDAMap[REMOTEIONUM_SUM * MAP_DA_SUM];
S_CANPosiMap u_asPosiMap[REMOTEIONUM_SUM * MAP_POSI_SUM];
WORD         u_wInputCount;
WORD         u_wOutputCount;
WORD         u_wADCount;
WORD         u_wDACount;
WORD         u_wPosiCount;
WORD         u_wCANRulerCount;
WORD         u_wMT_HUB_8ACount;
WORD         u_wGermanjetCount;
WORD         u_wPickerCount;
WORD         u_wtmIoT101Count;
WORD         u_wtmIoT102Count;
WORD         u_wtmIoT105Count;
WORD         u_wtmIoT201Count;
WORD         u_wtmIoT202Count;
WORD         u_wRemoteIOCount;
WORD         u_wJ6xCount;
WORD         g_wPackerCount;
WORD         u_wExternOtherCount;
WORD         u_wDriverInovanceCount;//SHANLL_20200324
WORD         u_wDriverXingtaiCount;//SHANLL_20200925
WORD         u_wRexrothCount;//shanll 20201228
WORD         u_wHTRobotCount;//shanll 20210311

WORD g_wHTJ6CardType;

WORD u_wECATLinkMask;

S_CommRemoteIO         g_asCommRemoteIO[REMOTEIONUM_SUM];
S_CommRemoteIOCardInfo u_asRemoteIOCardInfo[REMOTEIONUM_SUM];

WORD g_wRemoteIOUploadSDO_Abort_CNT = 0; // Anders 2016-2-4, Add.
//(THJ 2015-06-25 Add:
//------------------------------------------------------------------------------
static CommOffsetSeg_S const uc_asReadIndexSeg_RemoteIO[] = {
    //{ wDBIndexStart -                                             tBaseIndex,
    //  wDBIndexEnd -                                               tBaseIndex,
    //}
    {
        DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1 - DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1,
        DBINDEX_REMOTEIODEVICE1_INTERFACE_SOFTWARE_SN - DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1,
    },
};
static CommOffsetSeg_S const uc_asWriteIndexSeg_RemoteIO[] = {
    //{ wDBIndexStart -                                             tBaseIndex,
    //  wDBIndexEnd -                                               tBaseIndex,
    //}
    {
        DBINDEX_REMOTEIODEVICE1_INTERFACE_HARDWARE_PN - DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1,
        DBINDEX_REMOTEIODEVICE1_INTERFACE_SOFTWARE_PN - DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1,
    },
};

static CommOffsetSeg_S const uc_asReadIndexSeg_Picker[] = {
    //{ wDBIndexStart -                                             tBaseIndex,
    //  wDBIndexEnd -                                               tBaseIndex,
    //}
    {
        DBINDEX_PICKER_CONFIG_DEVICE_ID - DBINDEX_PICKER_CONFIG_DEVICE_ID,
        DBINDEX_PICKER_INTERFACE_0x291700 - DBINDEX_PICKER_CONFIG_DEVICE_ID,
    },
};
static CommOffsetSeg_S const uc_asWriteIndexSeg_Picker[] = {
    //{ wDBIndexStart -                                             tBaseIndex,
    //  wDBIndexEnd -                                               tBaseIndex,
    //}
    {
        DBINDEX_PICKER_CONFIG_DEVICE_ID - DBINDEX_PICKER_CONFIG_DEVICE_ID,
        DBINDEX_PICKER_INTERFACE_0x291700 - DBINDEX_PICKER_CONFIG_DEVICE_ID,
    },
};

static CommOffsetSeg_S const uc_asWriteIndexSeg_MT_HUB_8A[] = {
    //{ wDBIndexStart -                                             tBaseIndex,
    //  wDBIndexEnd -                                               tBaseIndex,
    //}
    {
        DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES - DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES,
        DBINDEX_MT_HUB_8A_WORK_AREA_STATE_CHANNEL8 - DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES,
    },
};
static CommOffsetSeg_S const uc_asReadIndexSeg_MT_HUB_8A[] = {
    //{ wDBIndexStart -                                             tBaseIndex,
    //  wDBIndexEnd -                                               tBaseIndex,
    //}
    {
        DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES - DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES,
        DBINDEX_MT_HUB_8A_WORK_AREA_STATE_CHANNEL8 - DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES,
    },
};

static CommOffsetSeg_S const uc_asWriteIndexSeg_GERMANJET[] = {
    {
        DBINDEX_GERMANJET_GRADIENT_ENTRIES - DBINDEX_GERMANJET_GRADIENT_ENTRIES,
        DBINDEX_GERMANJET_GRADIENT_CHANNEL8 - DBINDEX_GERMANJET_GRADIENT_ENTRIES,
    },
};
static CommOffsetSeg_S const uc_asReadIndexSeg_GERMANJET[] = {
    {
        DBINDEX_GERMANJET_GRADIENT_ENTRIES - DBINDEX_GERMANJET_GRADIENT_ENTRIES,
        DBINDEX_GERMANJET_GRADIENT_CHANNEL8 - DBINDEX_GERMANJET_GRADIENT_ENTRIES,
    },
};
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_RemoteIO description]
 */
void Init_RemoteIO(void)
{
    S_CommRemoteIO* psRemoteIO;
    WORD            i;

    for (i = 0; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
        u_asInputMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
        u_asOutputMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asADMap) / sizeof(*u_asADMap)); i++)
        u_asADMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asDAMap) / sizeof(*u_asDAMap)); i++)
        u_asDAMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
        u_asPosiMap[i].psRemote = NULL;
    u_wInputCount       = 0;
    u_wOutputCount      = 0;
    u_wADCount          = 0;
    u_wDACount          = 0;
    u_wPosiCount        = 0;
    u_wCANRulerCount    = 0;
    u_wMT_HUB_8ACount   = 0;
    u_wGermanjetCount   = 0;
    u_wPickerCount      = 0;
    u_wtmIoT101Count    = 0;
    u_wtmIoT102Count    = 0;
    u_wtmIoT105Count    = 0;
    u_wtmIoT201Count    = 0;
    u_wtmIoT202Count    = 0;
    u_wJ6xCount         = 0;
    u_wExternOtherCount = 0;
    g_wPackerCount      = 0;
    u_wDriverInovanceCount = 0;
    u_wDriverXingtaiCount = 0;
    u_wRexrothCount     = 0;
    u_wHTRobotCount     = 0;

    g_wHTJ6CardType = HT_J6TYPE_NULL;

    u_wECATLinkMask = 0;

    u_wRemoteIOCount = 0;
    for (i = 0; i <= REMOTEIONUM_MAX; i++)
    {
        psRemoteIO = &g_asCommRemoteIO[i];
        CommRemoteIO_Init(psRemoteIO);
        SCommComp_InitEventCallBack(&psRemoteIO->_sComp, Comm_EventCallBack);
        SCANOpenCompUnique_InitEventCallBack(&psRemoteIO->_sCANOpenUnique, CANOpen_EventCallBack);
#if (DEBUG)
        psRemoteIO->_sComp.PUB_eCompType = (E_CompType)(COMPTYPE_REMOTEIO1 + i);
#endif

        CommRemoteIOCardInfo_Init(&u_asRemoteIOCardInfo[i]);
        SCommComp_AddSubComp(&psRemoteIO->_sComp, &u_asRemoteIOCardInfo[i]._sSubComp);
    }
}

/**
 * [InitMap_RemoteIO description]
 */
void InitMap_RemoteIO(void)
{
    WORD i;

    for (i = 0; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
        u_asInputMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
        u_asOutputMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asADMap) / sizeof(*u_asADMap)); i++)
        u_asADMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asDAMap) / sizeof(*u_asDAMap)); i++)
        u_asDAMap[i].psRemote = NULL;
    for (i = 0; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
        u_asPosiMap[i].psRemote = NULL;
    u_wInputCount       = 0;
    u_wOutputCount      = 0;
    u_wADCount          = 0;
    u_wDACount          = 0;
    u_wPosiCount        = 0;
    u_wCANRulerCount    = 0;
    u_wMT_HUB_8ACount   = 0;
    u_wGermanjetCount   = 0;
    u_wPickerCount      = 0;
    u_wtmIoT101Count    = 0;
    u_wtmIoT102Count    = 0;
    u_wtmIoT105Count    = 0;
    u_wtmIoT201Count    = 0;
    u_wtmIoT202Count    = 0;
    u_wJ6xCount         = 0;
    g_wPackerCount      = 0;
    u_wExternOtherCount = 0;
    u_wDriverInovanceCount = 0;
    u_wDriverXingtaiCount = 0;
    u_wRexrothCount     = 0;
    u_wHTRobotCount     = 0;

    g_wHTJ6CardType = HT_J6TYPE_NULL;

    u_wECATLinkMask = 0;

    u_wRemoteIOCount = 0;
    for (i = 0; i < REMOTEIONUM_SUM; i++)
    {
        g_asCommRemoteIO[i]._eDeviceType                    = REMOTEIODEVICETYPE_M28;
        g_asCommRemoteIO[i]._sCANOpenUnique.PUB_blClearSize = FALSE;
    }
    CommComp_turntable_clean();
    DealECSlaveHandle();
}

/**
 * [Map_RemoteIO description]
 * @author hankin (2019/06/18)
 * @param  eDeviceType [description]
 * @param  eCommType   [description]
 * @param  tNodeID     [description]
 * @param  wCommCycle  [description]
 */
void Map_RemoteIO(E_RemoteIODeviceType eDeviceType, E_CommType eCommType, NodeID_T tNodeID, WORD wCommCycle)
{
    S_CommRemoteIO*         pRemoteIO;
    S_CommRemoteIOCardInfo* pCardInfo;
    WORD                    i;

    DEBUG_ASSERT(u_wRemoteIOCount <= REMOTEIONUM_MAX);
    pRemoteIO = &g_asCommRemoteIO[u_wRemoteIOCount];
    pCardInfo = &u_asRemoteIOCardInfo[u_wRemoteIOCount];
    SCommSubComp_InitReadInfo(&pCardInfo->_sSubComp,
        uc_asReadIndexSeg_RemoteIO,
        DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1 + u_wRemoteIOCount * DBGROUPINDEX_REMOTEIODEVICE_SUM,
        sizeof(uc_asReadIndexSeg_RemoteIO) / sizeof(*uc_asReadIndexSeg_RemoteIO));
    SCommSubComp_InitWriteInfo(&pCardInfo->_sSubComp,
        uc_asWriteIndexSeg_RemoteIO,
        DBINDEX_REMOTEIODEVICE1_CONFIG_VERSION_INFORMATION1 + u_wRemoteIOCount * DBGROUPINDEX_REMOTEIODEVICE_SUM,
        sizeof(uc_asWriteIndexSeg_RemoteIO) / sizeof(*uc_asWriteIndexSeg_RemoteIO));

    switch (eDeviceType)
    {
        case REMOTEIODEVICETYPE_TMTURNTABLE_ELECTRIC:
        case REMOTEIODEVICETYPE_TMTURNTABLE_HYDRAULIC:
        case REMOTEIODEVICETYPE_HT_ROTATION:
        case REMOTEIODEVICETYPE_DRIVER_INOVANCE:
        case REMOTEIODEVICETYPE_DRIVER_XINGTAI:
        case REMOTEIODEVICETYPE_HTROBOT:
            break;
        default:
            CommRemoteIO_SetCommType(pRemoteIO, eCommType);
            CommRemoteIO_SetDeviceType(pRemoteIO, eDeviceType);
            CommRemoteIO_SetNodeID(pRemoteIO, tNodeID);
            pRemoteIO->wSendCycle = wCommCycle;
            break;
    }
    switch (eDeviceType)
    {
        case REMOTEIODEVICETYPE_CAN_RULER:
            u_wCANRulerCount++;
            for (i = u_wPosiCount; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
            {
                if (NULL == u_asPosiMap[i].psRemote)
                {
                    u_asPosiMap[i].psRemote    = pRemoteIO;
                    u_asPosiMap[i].bDWORDOfNum = 0;
                    u_wPosiCount += 1;
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_PICKER:
            u_wPickerCount++;
            for (i = u_wInputCount; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
            {
                if (NULL == u_asInputMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asInputMap) / sizeof(*u_asInputMap)))
                            break;
                        u_asInputMap[i + j].psRemote   = pRemoteIO;
                        u_asInputMap[i + j].bBYTEOfNum = j + 4;
                        u_wInputCount += 1;
                    }
                    break;
                }
            }
            for (i = u_wOutputCount; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
            {
                if (NULL == u_asOutputMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)))
                            break;
                        u_asOutputMap[i + j].psRemote   = pRemoteIO;
                        u_asOutputMap[i + j].bBYTEOfNum = j + 4;
                        u_wOutputCount += 1;
                    }
                    break;
                }
            }
            SCommSubComp_InitReadInfo(&pCardInfo->_sSubComp,
                uc_asReadIndexSeg_Picker,
                DBINDEX_PICKER_CONFIG_DEVICE_ID,
                sizeof(uc_asReadIndexSeg_Picker) / sizeof(*uc_asReadIndexSeg_Picker));
            SCommSubComp_InitWriteInfo(&pCardInfo->_sSubComp,
                uc_asWriteIndexSeg_Picker,
                DBINDEX_PICKER_CONFIG_DEVICE_ID,
                sizeof(uc_asWriteIndexSeg_Picker) / sizeof(*uc_asWriteIndexSeg_Picker));
            break;
        case REMOTEIODEVICETYPE_MT_HUB_8A:
            u_wMT_HUB_8ACount++;
            for (i = u_wPosiCount; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
            {
                if (NULL == u_asPosiMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 8; j++)
                    {
                        if ((i + j) >= (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)))
                            break;
                        u_asPosiMap[i + j].psRemote    = pRemoteIO;
                        u_asPosiMap[i + j].bDWORDOfNum = j;
                        u_wPosiCount += 1;
                    }
                    break;
                }
            }
            SCommSubComp_InitReadInfo(&pCardInfo->_sSubComp,
                uc_asReadIndexSeg_MT_HUB_8A,
                DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES,
                sizeof(uc_asReadIndexSeg_MT_HUB_8A) / sizeof(*uc_asReadIndexSeg_MT_HUB_8A));
            SCommSubComp_InitWriteInfo(&pCardInfo->_sSubComp,
                uc_asWriteIndexSeg_MT_HUB_8A,
                DBINDEX_MT_HUB_8A_WAVEGUIDE_SPEED_VALUE_ENTRIES,
                sizeof(uc_asWriteIndexSeg_MT_HUB_8A) / sizeof(*uc_asWriteIndexSeg_MT_HUB_8A));
            pRemoteIO->_sCANOpenUnique.PUB_blClearSize = TRUE;
            break;
        case REMOTEIODEVICETYPE_GERMANJET:
            u_wGermanjetCount++;
            for (i = u_wPosiCount; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
            {
                if (NULL == u_asPosiMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 8; j++)
                    {
                        if ((i + j) >= (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)))
                            break;
                        u_asPosiMap[i + j].psRemote    = pRemoteIO;
                        u_asPosiMap[i + j].bDWORDOfNum = j;
                        u_wPosiCount += 1;
                    }
                    break;
                }
            }
            SCommSubComp_InitReadInfo(&pCardInfo->_sSubComp,
                uc_asReadIndexSeg_GERMANJET,
                DBINDEX_GERMANJET_GRADIENT_ENTRIES,
                sizeof(uc_asReadIndexSeg_GERMANJET) / sizeof(*uc_asReadIndexSeg_GERMANJET));
            SCommSubComp_InitWriteInfo(&pCardInfo->_sSubComp,
                uc_asWriteIndexSeg_GERMANJET,
                DBINDEX_GERMANJET_GRADIENT_ENTRIES,
                sizeof(uc_asWriteIndexSeg_GERMANJET) / sizeof(*uc_asWriteIndexSeg_GERMANJET));
            pRemoteIO->_sCANOpenUnique.PUB_blClearSize = TRUE;
            break;
        case REMOTEIODEVICETYPE_TMIOT101:
            u_wtmIoT101Count++;
            for (i = u_wInputCount; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
            {
                if (NULL == u_asInputMap[i].psRemote)
                {
                    u_asInputMap[i].psRemote   = pRemoteIO;
                    u_asInputMap[i].bBYTEOfNum = 0;
                    u_wInputCount += 1;
                    break;
                }
            }
            for (i = u_wOutputCount; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
            {
                if (NULL == u_asOutputMap[i].psRemote)
                {
                    u_asOutputMap[i].psRemote   = pRemoteIO;
                    u_asOutputMap[i].bBYTEOfNum = 0;
                    u_wOutputCount += 1;
                    break;
                }
            }
            for (i = u_wADCount; i < (sizeof(u_asADMap) / sizeof(*u_asADMap)); i++)
            {
                if (NULL == u_asADMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 6; j++)
                    {
                        if ((i + j) >= (sizeof(u_asADMap) / sizeof(*u_asADMap)))
                            break;
                        u_asADMap[i + j].psRemote   = pRemoteIO;
                        u_asADMap[i + j].bWORDOfNum = j + 1;
                        u_wADCount += 1;
                    }
                    break;
                }
            }
            for (i = u_wDACount; i < (sizeof(u_asDAMap) / sizeof(*u_asDAMap)); i++)
            {
                if (NULL == u_asDAMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asDAMap) / sizeof(*u_asDAMap)))
                            break;
                        u_asDAMap[i + j].psRemote   = pRemoteIO;
                        u_asDAMap[i + j].bWORDOfNum = j + 1;
                        u_wDACount += 1;
                    }
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_TMIOT102:
            u_wtmIoT102Count++;
            for (i = u_wInputCount; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
            {
                if (NULL == u_asInputMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asInputMap) / sizeof(*u_asInputMap)))
                            break;
                        u_asInputMap[i + j].psRemote   = pRemoteIO;
                        u_asInputMap[i + j].bBYTEOfNum = j;
                        u_wInputCount += 1;
                    }
                    break;
                }
            }
            for (i = u_wOutputCount; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
            {
                if (NULL == u_asOutputMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)))
                            break;
                        u_asOutputMap[i + j].psRemote   = pRemoteIO;
                        u_asOutputMap[i + j].bBYTEOfNum = j;
                        u_wOutputCount += 1;
                    }
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_TMIOT105:
            u_wtmIoT105Count++;
            for (i = u_wInputCount; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
            {
                if (NULL == u_asInputMap[i].psRemote)
                {
                    if (i >= (sizeof(u_asInputMap) / sizeof(*u_asInputMap)))
                        break;
                    u_asInputMap[i].psRemote   = pRemoteIO;
                    u_asInputMap[i].bBYTEOfNum = 0;
                    u_wInputCount += 1;
                    break;
                }
            }
            for (i = u_wOutputCount; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
            {
                if (NULL == u_asOutputMap[i].psRemote)
                {
                    if (i >= (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)))
                        break;
                    u_asOutputMap[i].psRemote   = pRemoteIO;
                    u_asOutputMap[i].bBYTEOfNum = 0;
                    u_wOutputCount += 1;
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_TMIOT201:
            u_wtmIoT201Count++;
            for (i = u_wInputCount; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
            {
                if (NULL == u_asInputMap[i].psRemote)
                {
                    if (i >= (sizeof(u_asInputMap) / sizeof(*u_asInputMap)))
                        break;
                    u_asInputMap[i].psRemote   = pRemoteIO;
                    u_asInputMap[i].bBYTEOfNum = 0;
                    u_wInputCount += 1;
                    break;
                }
            }
            for (i = u_wOutputCount; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
            {
                if (NULL == u_asOutputMap[i].psRemote)
                {
                    if ( i >= (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)))
                        break;
                    u_asOutputMap[i].psRemote   = pRemoteIO;
                    u_asOutputMap[i].bBYTEOfNum = 0;
                    u_wOutputCount += 1;
                    break;
                }
            }
            for (i = u_wADCount; i < (sizeof(u_asADMap) / sizeof(*u_asADMap)); i++)
            {
                if (NULL == u_asADMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 6; j++)
                    {
                        if ((i + j) >= (sizeof(u_asADMap) / sizeof(*u_asADMap)))
                            break;
                        u_asADMap[i + j].psRemote   = pRemoteIO;
                        u_asADMap[i + j].bWORDOfNum = 4 + j;
                        u_wADCount += 1;
                    }
                    break;
                }
            }
            for (i = u_wDACount; i < (sizeof(u_asDAMap) / sizeof(*u_asDAMap)); i++)
            {
                if (NULL == u_asDAMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 8; j++)//shanll 20210520 modify
                    {
                        if ((i + j) >= (sizeof(u_asDAMap) / sizeof(*u_asDAMap)))
                            break;
                        u_asDAMap[i + j].psRemote   = pRemoteIO;
                        u_asDAMap[i + j].bWORDOfNum = 4 + j;
                        u_wDACount += 1;
                    }
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_TMIOT202:
            u_wtmIoT202Count++;
            for (i = u_wInputCount; i < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)); i++)
            {
                if (NULL == u_asInputMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asInputMap) / sizeof(*u_asInputMap)))
                            break;
                        u_asInputMap[i + j].psRemote   = pRemoteIO;
                        u_asInputMap[i + j].bBYTEOfNum = j;
                        u_wInputCount += 1;
                    }
                    break;
                }
            }
            for (i = u_wOutputCount; i < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)); i++)
            {
                if (NULL == u_asOutputMap[i].psRemote)
                {
                    WORD j;

                    for (j = 0; j < 2; j++)
                    {
                        if ((i + j) >= (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)))
                            break;
                        u_asOutputMap[i + j].psRemote   = pRemoteIO;
                        u_asOutputMap[i + j].bBYTEOfNum = j;
                        u_wOutputCount += 1;
                    }
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_BTL6_V11E:
            ++u_wExternOtherCount;
            if (COMMTYPE_ETHERCAT != eCommType)
            {
                pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                break;
            }
            for (i = u_wPosiCount; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
            {
                if (NULL == u_asPosiMap[i].psRemote)
                {
                    u_asPosiMap[i].psRemote    = pRemoteIO;
                    u_asPosiMap[i].bDWORDOfNum = 0;
                    u_wPosiCount += 1;
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_BTL6_V107:
            ++u_wExternOtherCount;
            if (COMMTYPE_ETHERCAT != eCommType)
            {
                pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                break;
            }
            for (i = u_wPosiCount; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
            {
                if (NULL == u_asPosiMap[i].psRemote)
                {
                    u_asPosiMap[i].psRemote    = pRemoteIO;
                    u_asPosiMap[i].bDWORDOfNum = 0;
                    u_wPosiCount += 1;
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_PARKER_COMPAX3_FLUID:
            ++g_wPackerCount;
            if (COMMTYPE_ETHERCAT != eCommType)
            {
                pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
            }
            break;
        case REMOTEIODEVICETYPE_TR_LINEAR:
            ++u_wExternOtherCount;
            if (COMMTYPE_ETHERCAT != eCommType)
            {
                pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
            }
            for (i = u_wPosiCount; i < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)); i++)
            {
                if (NULL == u_asPosiMap[i].psRemote)
                {
                    u_asPosiMap[i].psRemote    = pRemoteIO;
                    u_asPosiMap[i].bDWORDOfNum = 0;
                    u_wPosiCount += 1;
                    break;
                }
            }
            break;
        case REMOTEIODEVICETYPE_HTJ6_OIL:
        case REMOTEIODEVICETYPE_HTJ6_ELEINJ:
            ++u_wJ6xCount;
            if (u_wJ6xCount <= 2)
            {
                if ((HT_J6TYPE_NULL == g_wHTJ6CardType) || (HT_J6TYPE_OIL_ELE == g_wHTJ6CardType))
                {
                    g_wHTJ6CardType = HT_J6TYPE_OIL_ELE;
                    if (COMMTYPE_ETHERCAT != eCommType)
                    {
                        pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                    }
                }
                else
                {
                    pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                }
            }
            break;
        case REMOTEIODEVICETYPE_HTJ6_PLUS:
            ++u_wJ6xCount;
            if (u_wJ6xCount <= 2)
            {
                if ((HT_J6TYPE_NULL == g_wHTJ6CardType) || (HT_J6TYPE_PLUS == g_wHTJ6CardType))
                {
                    g_wHTJ6CardType = HT_J6TYPE_PLUS;
                    if (COMMTYPE_ETHERCAT != eCommType)
                    {
                        pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                    }
                }
                else
                {
                    pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                }
            }
            break;
        case REMOTEIODEVICETYPE_HT_EH1708X:
            ++u_wExternOtherCount;
            break;
        case REMOTEIODEVICETYPE_TMTURNTABLE_HYDRAULIC:
            ++u_wExternOtherCount;
            CommComp_turntable_add_hydraulic(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);
            break;
        case REMOTEIODEVICETYPE_TMTURNTABLE_ELECTRIC:
            ++u_wExternOtherCount;
            CommComp_turntable_add_electric(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);
            break;
        case REMOTEIODEVICETYPE_HT_ROTATION:
            ++u_wExternOtherCount;
            CommComp_turntable_add_HT(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);
            break;
        case REMOTEIODEVICETYPE_DRIVER_INOVANCE:
            ++u_wDriverInovanceCount;
            CommComp_driver_add_inovance(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);
            break;
        case REMOTEIODEVICETYPE_DRIVER_XINGTAI://shanll 20200927 add
            ++u_wDriverXingtaiCount;
            CommComp_driver_add_xingtai(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);
            break;
        case REMOTEIODEVICETYPE_HTROBOT://shanll 20210311 add
            ++u_wHTRobotCount;
            CommComp_Robot_add_HT(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);//shanll 20210426 add 28Master
            // CommComp_HTRobot_add(u_wRemoteIOCount, eCommType, tNodeID, wCommCycle);//shanll 20210323 add 28Slave
            break;
        case REMOTEIODEVICETYPE_HTJ6_PP:  //SHANLL 20191219  J6PP ADD
            ++u_wJ6xCount;
            if (u_wJ6xCount <= 3)
            {
                if ((HT_J6TYPE_NULL == g_wHTJ6CardType) || (HT_J6TYPE_PP == g_wHTJ6CardType))
                {
                    g_wHTJ6CardType = HT_J6TYPE_PP;  //J6PP  J6CardType = 3
                    if (COMMTYPE_ETHERCAT != eCommType)
                    {
                        pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                    }
                }
                else
                {
                    pRemoteIO->_eDeviceType = REMOTEIODEVICETYPE_M28;
                }
            }
            break;
        case REMOTEIODEVICETYPE_REXROTH:
            ++u_wRexrothCount;
            break;
        case REMOTEIODEVICETYPE_M28:
            break;
        case REMOTEIODEVICETYPE_SMC335_RGM50:
            DEBUG_ASSERT(false); // Temporary unrealized.
        default:
            ++u_wExternOtherCount;
            break;
    }
    if (COMMTYPE_ETHERCAT == eCommType)
    {
        g_Card_EtherCATMaster.bit.b15 = TRUE;
        DealECSlaveHandle();
        u_wECATLinkMask |= 1 << u_wRemoteIOCount;
    }
    u_wRemoteIOCount = u_wCANRulerCount + u_wMT_HUB_8ACount + u_wGermanjetCount + u_wPickerCount + u_wtmIoT101Count + u_wtmIoT102Count + u_wtmIoT105Count + u_wtmIoT201Count\
    + u_wtmIoT202Count + u_wJ6xCount + g_wPackerCount + u_wExternOtherCount + u_wDriverInovanceCount + u_wDriverXingtaiCount + u_wRexrothCount+u_wHTRobotCount;
}

/**
 * [SetOfflineOutput_RemoteIO description]
 * @param wIndex  [description]
 * @param wOutput [description]
 */
void SetOfflineOutput_RemoteIO(WORD wIndex, WORD wOutput)
{
    WORD i;

    DEBUG_ASSERT((wIndex << 1) < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)));
    for (i = 0; i < 2; ++i)
    {
        S_CommRemoteIO* psRemoteIO;

        psRemoteIO = u_asOutputMap[(wIndex << 1) + i].psRemote;
        if (NULL != psRemoteIO)
        {
            WORD wByteNum;

            wByteNum = u_asOutputMap[(wIndex << 1) + i].bBYTEOfNum;
            if (0 != (wByteNum & 1))
            {
                psRemoteIO->wOfflineOutput &= 0x00FF;
                psRemoteIO->wOfflineOutput |= (wOutput & 0xFF) << 8;
            }
            else
            {
                psRemoteIO->wOfflineOutput &= 0xFF00;
                psRemoteIO->wOfflineOutput |= wOutput & 0xFF;
            }
            wOutput >>= 8;
        }
    }
}

/**
 * [UpdateLinkState_RemoteIO description]
 */
void UpdateLinkState_RemoteIO(void)
{
    WORD wLink;

    wLink = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
    if (ETHCRCAT_CONFIG_SUCCESS == g_ECController.Master.wECState)
    {
        wLink |= u_wECATLinkMask;
    }
    else
    {
        wLink &= ~u_wECATLinkMask;
    }
    DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLink);
}

/**
 * [SetOutput_RemoteIO description]
 * @param wIndex  [description]
 * @param wOutput [description]
 */
void SetOutput_RemoteIO(WORD wIndex, WORD wOutput)
{
    WORD i;

    DEBUG_ASSERT((wIndex << 1) < (sizeof(u_asOutputMap) / sizeof(*u_asOutputMap)));
    for (i = 0; i < 2; i++)
    {
        S_CommRemoteIO* psRemoteIO;

        psRemoteIO = u_asOutputMap[(wIndex << 1) + i].psRemote;
        if (NULL != psRemoteIO)
        {
            WORD wByteNum, *pwData;

            wByteNum = u_asOutputMap[(wIndex << 1) + i].bBYTEOfNum;
            pwData   = (WORD*)psRemoteIO->_auSendPDO + (wByteNum >> 1);
            // if (0 != (wByteNum & 1))
            // {
            //  *pwData &= 0x00FF;
            //  *pwData |= (wOutput & 0xFF) << 8;
            // }
            // else
            // {
            //  *pwData &= 0xFF00;
            //  *pwData |= wOutput & 0xFF;
            // }
            if (0 != (wByteNum & 1))
            {
                wByteNum = *pwData & 0xFF00;
                wByteNum ^= (wOutput << 8) & 0xFF00;
                *pwData ^= wByteNum;
            }
            else
            {
                wByteNum = *pwData & 0xFF;
                wByteNum ^= wOutput & 0xFF;
                *pwData ^= wByteNum;
            }
        }
        wOutput >>= 8;
    }
}

/**
 * [SetDA_RemoteIO description]
 * @param wIndex [description]
 * @param wDA    [description]
 */
void SetDA_RemoteIO(WORD wIndex, WORD wDA)
{
    S_CommRemoteIO* psRemoteIO;

    DEBUG_ASSERT(wIndex < (sizeof(u_asDAMap) / sizeof(*u_asDAMap)));
    psRemoteIO = u_asDAMap[wIndex].psRemote;
    if (NULL != psRemoteIO)
    {
        if(psRemoteIO->_eDeviceType == REMOTEIODEVICETYPE_TMIOT201)//shanll 20210525 add
        {
            INT*  piData;
            piData  = (INT*)psRemoteIO->_auSendPDO + u_asDAMap[wIndex].bWORDOfNum;
            if(wDA <= 0xFFF)
                wDA = wDA << 3;
            *piData = wDA;
        }
        else
        {
            WORD* pwData;
            pwData  = (WORD*)psRemoteIO->_auSendPDO + u_asDAMap[wIndex].bWORDOfNum;
            *pwData = wDA;
        }
    }
}
/**
 * [GetInput_RemoteIO description]
 * @param  wIndex [description]
 * @return        [description]
 */
WORD GetInput_RemoteIO(WORD wIndex)
{
    WORD i, wRet;

    DEBUG_ASSERT((wIndex << 1) < (sizeof(u_asInputMap) / sizeof(*u_asInputMap)));
    wRet = 0;
    for (i = 0; i < 2; ++i)
    {
        S_CommRemoteIO* psRemoteIO;

        wRet >>= 8;
        psRemoteIO = u_asInputMap[(wIndex << 1) + i].psRemote;
        if (NULL != psRemoteIO)
        {
            WORD wTmp, *pwData;

            wTmp   = u_asInputMap[(wIndex << 1) + i].bBYTEOfNum;
            pwData = (WORD*)psRemoteIO->_auRecvedPDO + (wTmp >> 1);
            if (0 != (wTmp & 1))
            {
                wTmp = *pwData & 0xFF00;
                wRet |= wTmp;
            }
            else
            {
                wTmp = *pwData & 0x00FF;
                wRet |= *pwData << 8;
            }
        }
    }
    return wRet;
}

/**
 * [GetAD_RemoteIO description]
 * @param  wIndex [description]
 * @return        [description]
 */
WORD GetAD_RemoteIO(WORD wIndex)
{
    S_CommRemoteIO* psRemoteIO;

    DEBUG_ASSERT(wIndex < (sizeof(u_asADMap) / sizeof(*u_asADMap)));
    psRemoteIO = u_asADMap[wIndex].psRemote;
    if (NULL != psRemoteIO)
    {
        WORD* pwData;

        pwData = (WORD*)psRemoteIO->_auRecvedPDO + u_asADMap[wIndex].bWORDOfNum;
        if(psRemoteIO->_eDeviceType == REMOTEIODEVICETYPE_TMIOT201)
        {
            if(*pwData > 0x7FFF)
                return 0;
            else
                return	(*pwData) << 1;
        }
        return *pwData;
    }
    return 0;
}

/**
 * [GetPositionCount_RemoteIO description]
 * @param  wIndex [description]
 * @return        [description]
 */
LONG GetPositionCount_RemoteIO(WORD wIndex)
{
    S_CommRemoteIO* psRemoteIO;

    DEBUG_ASSERT(wIndex < (sizeof(u_asPosiMap) / sizeof(*u_asPosiMap)));
    psRemoteIO = u_asPosiMap[wIndex].psRemote;
    if (NULL != psRemoteIO)
    {
        LONG* plData;

        plData = (LONG*)psRemoteIO->_auRecvedPDO + u_asPosiMap[wIndex].bDWORDOfNum;
        return *plData;
    }
    return 0;
}

/**
 * [CommRemoteIOCardInfo_Init description]
 * @param psRemoteIO [description]
 */
void CommRemoteIOCardInfo_Init(S_CommRemoteIOCardInfo* const psRemoteIO)
{
    DEBUG_ASSERT(NULL != psRemoteIO);
    SCommSubComp_Init(&psRemoteIO->_sSubComp);
    SCANOpenSubCompUnique_Init(&psRemoteIO->_sCANOpenUnique);
}

/**
 * [CommRemoteIO_Init description]
 * @param psRemoteIO [description]
 */
void CommRemoteIO_Init(S_CommRemoteIO* const psRemoteIO)
{
    DEBUG_ASSERT(NULL != psRemoteIO);
    SCommComp_Init(&psRemoteIO->_sComp);
    SCANOpenCompUnique_Init(&psRemoteIO->_sCANOpenUnique);
    CANOpenCHBManager_Init(&psRemoteIO->_sCHBManager);
    CANOpenCHBManager_Init(&psRemoteIO->_sPDOManager);
    ResetTimer(&psRemoteIO->_dwPDOSendTime, 0);
    memset(&psRemoteIO->_auRecvedPDO, 0, sizeof(psRemoteIO->_auRecvedPDO));
    memset(&psRemoteIO->_auSendPDO, 0, sizeof(psRemoteIO->_auSendPDO));
    psRemoteIO->_eDeviceType   = REMOTEIODEVICETYPE_M28;
    psRemoteIO->wSendCycle     = 0;
    psRemoteIO->wOfflineOutput = 0;
    ResetTimer(&psRemoteIO->_dwPreAlarmCtrlTimer, 0);
}

/**
 * [CommRemoteIO_SetCommType description]
 * @author hankin (2019/06/18)
 * @param  psRemoteIO [description]
 * @param  eCommType  [description]
 */
static void CommRemoteIO_SetCommType(S_CommRemoteIO* const psRemoteIO, E_CommType eCommType)
{
    DEBUG_ASSERT(NULL != psRemoteIO);
//    if (COMMTYPE_ETHERCAT == eCommType)   //hankin 20200309
//    {
//        return;
//    }
//    SCommComp_SetCommType(&psRemoteIO->_sComp, eCommType);
    SCommComp_SetCommType(&psRemoteIO->_sComp, (COMMTYPE_ETHERCAT != eCommType) ? eCommType : COMMTYPE_SUM);
}

/**
 * [CommRemoteIO_SetDeviceType description]
 * @author hankin (2019/06/18)
 * @param  psRemoteIO  [description]
 * @param  eDeviceType [description]
 */
static void CommRemoteIO_SetDeviceType(S_CommRemoteIO* const psRemoteIO, E_RemoteIODeviceType eDeviceType)
{
    DEBUG_ASSERT(NULL != psRemoteIO);
    psRemoteIO->_eDeviceType = eDeviceType;
    switch (CommComp_GetCommType(&psRemoteIO->_sComp))
    {
        case COMMTYPE_CAN1:
        case COMMTYPE_CAN2:
            switch (eDeviceType)
            {
                case REMOTEIODEVICETYPE_M28:
                    break;
                case REMOTEIODEVICETYPE_SMC335_RGM50:
                    DEBUG_ASSERT(false); // Temporary unrealized.
                    break;
                case REMOTEIODEVICETYPE_CAN_RULER:
                    //SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, Ruler_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = RULER_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = RULER_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = RULER_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = RULER_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = RULER_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = RULER_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_PICKER:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, Picker_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = PICKER_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = PICKER_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = PICKER_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = PICKER_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = PICKER_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = PICKER_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_MT_HUB_8A:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, MT_HUB_8A_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = MT_HUB_8A_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = MT_HUB_8A_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = MT_HUB_8A_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = MT_HUB_8A_PDO_HEARTBEAT_CYCLE; // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = MT_HUB_8A_PDO_LOSE_MAX;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = MT_HUB_8A_PDO_CONFLICT_MAX;    // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_GERMANJET: // hankin 20180814 add.
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, GERMANJET_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = GERMANJET_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = GERMANJET_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = GERMANJET_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = GERMANJET_PDO_HEARTBEAT_CYCLE;
                    psRemoteIO->_sPDOManager.iLose_max     = GERMANJET_PDO_LOSE_MAX;
                    psRemoteIO->_sPDOManager.iConflict_max = GERMANJET_PDO_CONFLICT_MAX;
                    break;
                case REMOTEIODEVICETYPE_TMIOT101:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, tmIoT101_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = TMIOT101_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = TMIOT101_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = TMIOT101_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = TMIOT101_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = TMIOT101_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = TMIOT101_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_TMIOT102:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, tmIoT102_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = TMIOT102_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = TMIOT102_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = TMIOT102_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = TMIOT102_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = TMIOT102_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = TMIOT102_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_TMIOT105:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, tmIoT105_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = TMIOT105_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = TMIOT105_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = TMIOT105_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = TMIOT105_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = TMIOT105_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = TMIOT105_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_TMIOT201:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, tmIoT201_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = TMIOT201_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = TMIOT201_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = TMIOT201_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = TMIOT201_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = TMIOT201_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = TMIOT201_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                case REMOTEIODEVICETYPE_TMIOT202:
                    SCANOpenCompUnique_InitConfigInfo(&psRemoteIO->_sCANOpenUnique, tmIoT202_CANOpen_ConfigSDOMsg);
                    psRemoteIO->_sCHBManager.iTick         = TMIOT202_HEARTBEAT_CYCLE;
                    psRemoteIO->_sCHBManager.iLose_max     = TMIOT202_HEARTBEAT_LOSE_MAX;
                    psRemoteIO->_sCHBManager.iConflict_max = TMIOT202_HEARTBEAT_CONFLICT_MAX;
                    psRemoteIO->_sPDOManager.iTick         = TMIOT202_PDO_CYCLE;        // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iLose_max     = TMIOT202_PDO_LOSE_MAX;     // THJ 2016-11-25 Add.
                    psRemoteIO->_sPDOManager.iConflict_max = TMIOT202_PDO_CONFLICT_MAX; // THJ 2016-11-25 Add.
                    break;
                default:
                    DEBUG_ASSERT(false);
                    break;
            }
            break;
        case COMMTYPE_SUM:
            break;
        default:
            DEBUG_ASSERT(false);
            break;
    }
}

/**
 * [CommRemoteIO_SetNodeID description]
 * @author hankin (2019/06/18)
 * @param  psRemoteIO [description]
 * @param  tNodeID    [description]
 */
static void CommRemoteIO_SetNodeID(S_CommRemoteIO* const psRemoteIO, NodeID_T tNodeID)
{
    DEBUG_ASSERT(NULL != psRemoteIO);
    switch (CommComp_GetCommType(&psRemoteIO->_sComp))
    {
        case COMMTYPE_CAN1:
        case COMMTYPE_CAN2:
            SCANOpenCompUnique_SetNodeID(&psRemoteIO->_sCANOpenUnique, tNodeID);
            break;
        case COMMTYPE_SUM:
            psRemoteIO->_sCANOpenUnique.PRI_wCurConfigIndex = tNodeID;
            break;
        default:
            DEBUG_ASSERT(false);
            break;
    }
}
/**
 * [SCommRemoteIO_GetLinkState description]
 * @method SCommRemoteIO_GetLinkState
 * @param  psRemoteIO                 [description]
 * @return                            [description]
 */
// BOOL SCommRemoteIO_GetLinkState(S_CommRemoteIO *psRemoteIO)
// {
//  DEBUG_ASSERT;(NULL != psRemoteIO)
//  switch (psRemoteIO->_sComp.PRO_eCommType)
//  {
//      case COMMTYPE_CAN1:
//      case COMMTYPE_CAN2:
//          return (CANOPENNODESTATE_OPERATIONAL == psRemoteIO->_sCANOpenUnique.PRO_bNodeState)
//              ? TRUE : FALSE;
//      case COMMTYPE_SUM:
//          return FALSE;
//      default:
//          D_ASSERT_PRO(FALSE)
//          return FALSE;
//  }
// }
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
        case COMMEVENT_COMP_CHANGE_COMMTYPE:;
            {
                CommSubComp_S*          psSubComp;
                S_CommRemoteIO*         psRemoteIO;
                S_CommRemoteIOCardInfo* psRemoteIOCardInfo;
                WORD                    wLinkState;

                DEBUG_ASSERT(NULL != psComp);
                psRemoteIO = ContainerOf(psComp, S_CommRemoteIO, _sComp);
                psSubComp  = psComp->PRI_psSubCompHead;
                switch (CommComp_GetCommType(psComp))
                {
                    case COMMTYPE_CAN1:
                    case COMMTYPE_CAN2:
                        while (NULL != psSubComp)
                        {
                            psRemoteIOCardInfo      = ContainerOf(psSubComp, S_CommRemoteIOCardInfo, _sSubComp);
                            psSubComp->PRO_pvUnique = &psRemoteIOCardInfo->_sCANOpenUnique;
                            psSubComp               = psSubComp->PRI_psNextSubComp;
                        }
                        psComp->PRO_pvUnique = &psRemoteIO->_sCANOpenUnique;
                        CANOpenMaster_ResetComm(&psRemoteIO->_sCANOpenUnique);
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
                wLinkState = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
                wLinkState &= ~((WORD)1 << (psRemoteIO - g_asCommRemoteIO));
                DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
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
static void CANOpen_CustomFunc_Operation(CommComp_S* psComp, E_CustomFuncType eType)
{
    S_CommRemoteIO* psRemoteIO;
    E_CANChannel    eChannel;
    WORD            wDataLen;

    DEBUG_ASSERT(NULL != psComp);
    psRemoteIO = ContainerOf(psComp, S_CommRemoteIO, _sComp);
    eChannel   = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(psComp));
    DEBUG_ASSERT((CANCHANNAL_CAN1 == eChannel) || (CANCHANNAL_CAN2 == eChannel));
    switch (eType)
    {
        case CUSTOMFUNCTYPE_FAST:
            DEBUG_ASSERT(CANOPENNODESTATE_OPERATIONAL == psRemoteIO->_sCANOpenUnique.PRO_bNodeState);
            if (0 == psRemoteIO->wSendCycle)
                ResetTimer(&psRemoteIO->_dwPDOSendTime, 0);
            else if (CheckTimerOut(&psRemoteIO->_dwPDOSendTime))
            {
                ResetTimer(&psRemoteIO->_dwPDOSendTime, psRemoteIO->wSendCycle);
                wDataLen = 0;
                switch (psRemoteIO->_eDeviceType)
                {
                    case REMOTEIODEVICETYPE_CAN_RULER:
                        break;
                    case REMOTEIODEVICETYPE_PICKER:
                        psRemoteIO->_auSendPDO[0].sU32.bData0 = DB_GetDWORD(DBINDEX_PICKER_INTERFACE_MOLD_POSI);
                        psRemoteIO->_auSendPDO[0].sU16.bData3 = DB_GetDWORD(DBINDEX_PICKER_INTERFACE_EJECT_POSI);
                        DB_SetWORD(DBINDEX_PICKER_INTERFACE_EUROMAP_INPUT, psRemoteIO->_auSendPDO[0].sU16.bData2); // for Watch.
                        CANOpenMaster_SendPDO(eChannel, psRemoteIO->_sCANOpenUnique.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, 8, psRemoteIO->_auSendPDO);
                        break;
                    case REMOTEIODEVICETYPE_MT_HUB_8A:
                    case REMOTEIODEVICETYPE_GERMANJET:
                        break;
                    case REMOTEIODEVICETYPE_TMIOT101:
                        wDataLen += 4;
                    case REMOTEIODEVICETYPE_TMIOT102:
                    case REMOTEIODEVICETYPE_TMIOT202:
                        wDataLen += 1;
                    case REMOTEIODEVICETYPE_TMIOT105:
                        wDataLen += 1;
                        CANOpenMaster_SendPDO(eChannel, psRemoteIO->_sCANOpenUnique.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, wDataLen, psRemoteIO->_auSendPDO);
                        break;
                    case REMOTEIODEVICETYPE_TMIOT201:
                        wDataLen += 1;
                        CANOpenMaster_SendPDO(eChannel, psRemoteIO->_sCANOpenUnique.PRO_bNodeID, CANOPENFUNCID_RECV_PDO1, wDataLen, psRemoteIO->_auSendPDO);
                        wDataLen += 7;
                        CANOpenMaster_SendPDO(eChannel, psRemoteIO->_sCANOpenUnique.PRO_bNodeID, CANOPENFUNCID_RECV_PDO2, wDataLen, &psRemoteIO->_auSendPDO[1]);
                        CANOpenMaster_SendPDO(eChannel, psRemoteIO->_sCANOpenUnique.PRO_bNodeID, CANOPENFUNCID_RECV_PDO3, wDataLen, &psRemoteIO->_auSendPDO[2]);
                        break;
                    default:
                        DEBUG_ASSERT(false);
                        break;
                }
            }
            break;
        case CUSTOMFUNCTYPE_SLOW:
        {
            if (CANOPENNODESTATE_OPERATIONAL == psRemoteIO->_sCANOpenUnique.PRO_bNodeState)
            {
                E_CANOpenCHBCheck eCheck;

                eCheck = CANOpenCHBManager_Check(&psRemoteIO->_sCHBManager);
                if (CANOPENCHBCHECK_IDLE != eCheck)
                {
                    if (CANOPENCHBCHECK_CONFLICT == eCheck)
                    {
                        CANOpenMaster_ResetComm(&psRemoteIO->_sCANOpenUnique);
                        //DB_SetDWORD(DBINDEX_M28_MONITOR_STATE_ERROR1, DB_MakeError(DBDEVICEID_REMOTEIO1 + eRemoteIONum, psRemoteIO->_sCANOpenUnique.PRO_bNodeID, ERROR_ID_REMOTEIO_NODEIDCONFLICT));
                        Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_REMOTEIO_NODEIDCONFLICT, psRemoteIO->_sCANOpenUnique.PRO_bNodeID); // THJ 2017-1-13 Modify.
                    }
                    else
                    {
                        switch (psRemoteIO->_eDeviceType)
                        {
                            case REMOTEIODEVICETYPE_GERMANJET:
                            case REMOTEIODEVICETYPE_TMIOT101:
                            case REMOTEIODEVICETYPE_TMIOT102:
                            case REMOTEIODEVICETYPE_TMIOT105:
                            case REMOTEIODEVICETYPE_TMIOT201:
                            case REMOTEIODEVICETYPE_TMIOT202:
                                CANOpenMaster_SendNodeGruading(eChannel, psRemoteIO->_sCANOpenUnique.PRO_bNodeID);
                                break;
                            default:
                                break;
                        }
                    }
                }
                // THJ 2016-11-25 Add.
                eCheck = CANOpenCHBManager_Check(&psRemoteIO->_sPDOManager);
                if (CANOPENCHBCHECK_OFFLINE == eCheck)
                {
                    CANOpenMaster_ResetComm(&psRemoteIO->_sCANOpenUnique);
                }

                // hankin 20180416 add.
                switch (psRemoteIO->_eDeviceType)
                {
                    case REMOTEIODEVICETYPE_MT_HUB_8A:
                        if (0 != psRemoteIO->uExtendData._sMT_HUB_8A.iError)
                        {
                            if (CheckTimerOut(&psRemoteIO->uExtendData._sMT_HUB_8A.sCtrlTimerOfReadError))
                            {
                                BYTE     iError;
                                unsigned i;

                                iError = psRemoteIO->uExtendData._sMT_HUB_8A.iError;
                                for (i = 0; 0 != iError; ++i, iError >>= 1)
                                {
                                    if (iError & 1)
                                    {
                                        BYTE iError;

                                        Comm_SetReadFlagByDBIndex(DBINDEX_MT_HUB_8A_WORK_AREA_STATE_CHANNEL1 + i);
                                        iError = DB_GetBYTE(DBINDEX_MT_HUB_8A_WORK_AREA_STATE_CHANNEL1 + i);
                                        if (0 != iError)
                                        {
                                            Set_MasterError(ACTAXIS_MOLD, APPDEVICE_MT_HUB, ERRORTYPE_WARN, iError, 1 + i, TRUE);
                                        }
                                    }
                                }
                                ResetTimer(&psRemoteIO->uExtendData._sMT_HUB_8A.sCtrlTimerOfReadError, MT_HUB_8A_READ_ERROR_CYCLE);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            else
            {
                CANOpenCHBManager_Reset(&psRemoteIO->_sCHBManager);
                CANOpenCHBManager_Reset(&psRemoteIO->_sPDOManager);
            }
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
static void CANOpen_EventCallBack(CANOpenCompUnique_S* psCompUnique, CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType)
{
    S_CommRemoteIO* psRemoteIO;

    DEBUG_ASSERT(NULL != psCompUnique);
    psRemoteIO = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
    switch (eEventType)
    {
        case CANOPENEVENTTYPE_RECVED_DATAFRAME:
        {
            E_CANOpenFuncID eFuncID;

            DEBUG_ASSERT(NULL != psDataFrame);
            eFuncID = CANOpen_GetFuncIDByCOBID(psDataFrame->uHead.sStand.bCOBID);
            switch (eFuncID)
            {
                case CANOPENFUNCID_SEND_PDO1:
                    CANOpenCHBManager_RecvedMsg(&psRemoteIO->_sPDOManager); // THJ 2016-11-25 Add.
                    psRemoteIO->_auRecvedPDO[0] = psDataFrame->uData;
                    switch (psRemoteIO->_eDeviceType)
                    {
                        case REMOTEIODEVICETYPE_PICKER:
                            DB_SetDWORD(DBINDEX_PICKER_INTERFACE_ARM_Y_POSI, psRemoteIO->_auRecvedPDO[0].sU32.bData0);
                            DB_SetWORD(DBINDEX_PICKER_INTERFACE_EUROMAP_OUTPUT, psRemoteIO->_auRecvedPDO[0].sU16.bData2);
                            DB_SetWORD(DBINDEX_PICKER_INTERFACE_PICKER_WARN, psRemoteIO->_auRecvedPDO[0].sU16.bData3);
                            break;
                        case REMOTEIODEVICETYPE_MT_HUB_8A:
                            if ((0 == psDataFrame->uData.sU32.bData0) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData0))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x01;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x01;
                            }
                            if ((0 == psDataFrame->uData.sU32.bData1) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData1))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x02;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x02;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case CANOPENFUNCID_SEND_PDO2:
                    psRemoteIO->_auRecvedPDO[1] = psDataFrame->uData;
                    switch (psRemoteIO->_eDeviceType)
                    {
                        case REMOTEIODEVICETYPE_MT_HUB_8A:
                            if ((0 == psDataFrame->uData.sU32.bData0) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData0))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x04;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x04;
                            }
                            if ((0 == psDataFrame->uData.sU32.bData1) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData1))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x08;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x08;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case CANOPENFUNCID_SEND_PDO3:
                    psRemoteIO->_auRecvedPDO[2] = psDataFrame->uData;
                    switch (psRemoteIO->_eDeviceType)
                    {
                        case REMOTEIODEVICETYPE_MT_HUB_8A:
                            if ((0 == psDataFrame->uData.sU32.bData0) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData0))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x10;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x10;
                            }
                            if ((0 == psDataFrame->uData.sU32.bData1) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData1))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x20;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x20;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case CANOPENFUNCID_SEND_PDO4:
                    psRemoteIO->_auRecvedPDO[3] = psDataFrame->uData;
                    switch (psRemoteIO->_eDeviceType)
                    {
                        case REMOTEIODEVICETYPE_MT_HUB_8A:
                            if ((0 == psDataFrame->uData.sU32.bData0) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData0))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x40;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x40;
                            }
                            if ((0 == psDataFrame->uData.sU32.bData1) || (0xFFFFFFFF == psDataFrame->uData.sU32.bData1))
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError |= 0x80;
                            }
                            else
                            {
                                psRemoteIO->uExtendData._sMT_HUB_8A.iError &= ~(BYTE)0x80;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case CANOPENFUNCID_NMT_ERROR_CTRL:
                    CANOpenCHBManager_RecvedMsg(&psRemoteIO->_sCHBManager);
                    break;
                case CANOPENFUNCID_SEND_SDO:
                    switch (psCompUnique->PRO_bNodeState)
                    {
                        case CANOPENNODESTATE_OPERATIONAL:
                            break;
                        default:
                            if (!CheckTimerOut(&psRemoteIO->_dwPreAlarmCtrlTimer))
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
                                switch (psRemoteIO->_eDeviceType)
                                {
                                    case REMOTEIODEVICETYPE_PICKER:
                                        if ((0x1000 == tODID) && (PICKER_VERIFY_0x100000 != psDataFrame->uData.sSDO.bData))
                                        {
                                            bErr     = TRUE;
                                            wErrCode = ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL;
                                        }
                                        else if (0x1018 == tODID)
                                        {
                                            if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (PICKER_VERIFY_0x101801 != psDataFrame->uData.sSDO.bData))
                                                || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (PICKER_VERIFY_0x101802 != psDataFrame->uData.sSDO.bData)))
                                            {
                                                bErr     = TRUE;
                                                wErrCode = ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL;
                                            }
                                        }
                                        break;
                                    case REMOTEIODEVICETYPE_MT_HUB_8A:
                                    case REMOTEIODEVICETYPE_GERMANJET:
                                        break;
                                    case REMOTEIODEVICETYPE_TMIOT101:
                                        if ((0x1000 == tODID) && (TMIOT101_VERIFY_0x100000 != psDataFrame->uData.sSDO.bData))
                                        {
                                            bErr     = TRUE;
                                            wErrCode = ERROR_ID_REMOTEIO_TMIOT101DEVICE_VERIFY_FAIL;
                                        }
                                        else if (0x1018 == tODID)
                                        {
                                            if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT101_VERIFY_0x101801 != psDataFrame->uData.sSDO.bData))
                                                || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT101_VERIFY_0x101802 != psDataFrame->uData.sSDO.bData)))
                                            {
                                                bErr     = TRUE;
                                                wErrCode = ERROR_ID_REMOTEIO_TMIOT101DEVICE_VERIFY_FAIL;
                                            }
                                        }
                                        break;
                                    case REMOTEIODEVICETYPE_TMIOT102:
                                        if ((0x1000 == tODID) && (TMIOT102_VERIFY_0x100000 != psDataFrame->uData.sSDO.bData))
                                        {
                                            bErr     = TRUE;
                                            wErrCode = ERROR_ID_REMOTEIO_TMIOT102DEVICE_VERIFY_FAIL;
                                        }
                                        else if (0x1018 == tODID)
                                        {
                                            if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT102_VERIFY_0x101801 != psDataFrame->uData.sSDO.bData))
                                                || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT102_VERIFY_0x101802 != psDataFrame->uData.sSDO.bData)))
                                            {
                                                bErr     = TRUE;
                                                wErrCode = ERROR_ID_REMOTEIO_TMIOT102DEVICE_VERIFY_FAIL;
                                            }
                                        }
                                        break;
                                    case REMOTEIODEVICETYPE_TMIOT105:
                                        if ((0x1000 == tODID) && (TMIOT105_VERIFY_0x100000 != psDataFrame->uData.sSDO.bData))
                                        {
                                            bErr     = TRUE;
                                            wErrCode = ERROR_ID_REMOTEIO_TMIOT105DEVICE_VERIFY_FAIL;
                                        }
                                        else if (0x1018 == tODID)
                                        {
                                            if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT105_VERIFY_0x101801 != psDataFrame->uData.sSDO.bData))
                                                || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT105_VERIFY_0x101802 != (psDataFrame->uData.sSDO.bData & TMIOT105_MASK_0x101802))))
                                            {
                                                bErr     = TRUE;
                                                wErrCode = ERROR_ID_REMOTEIO_TMIOT105DEVICE_VERIFY_FAIL;
                                            }
                                        }
                                        break;
                                    case REMOTEIODEVICETYPE_TMIOT201:
                                        if ((0x1000 == tODID) && (TMIOT201_VERIFY_0x100000 != psDataFrame->uData.sSDO.bData))
                                        {
                                            bErr     = TRUE;
                                            wErrCode = ERROR_ID_REMOTEIO_TMIOT201DEVICE_VERIFY_FAIL;
                                        }
                                        else if (0x1018 == tODID)
                                        {
                                            if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT201_VERIFY_0x101801 != psDataFrame->uData.sSDO.bData))
                                                || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT201_VERIFY_0x101802 != (psDataFrame->uData.sSDO.bData & TMIOT201_MASK_0x101802))))
                                            {
                                                bErr     = TRUE;
                                                wErrCode = ERROR_ID_REMOTEIO_TMIOT201DEVICE_VERIFY_FAIL;
                                            }
                                        }
                                        break;
                                    case REMOTEIODEVICETYPE_TMIOT202:
                                        if ((0x1000 == tODID) && (TMIOT202_VERIFY_0x100000 != psDataFrame->uData.sSDO.bData))
                                        {
                                            bErr     = TRUE;
                                            wErrCode = ERROR_ID_REMOTEIO_TMIOT202DEVICE_VERIFY_FAIL;
                                        }
                                        else if (0x1018 == tODID)
                                        {
                                            if (((0x01 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT202_VERIFY_0x101801 != psDataFrame->uData.sSDO.bData))
                                                || ((0x02 == psDataFrame->uData.sSDO.bODSubIndex) && (TMIOT202_VERIFY_0x101802 != (psDataFrame->uData.sSDO.bData & TMIOT202_MASK_0x101802))))
                                            {
                                                bErr     = TRUE;
                                                wErrCode = ERROR_ID_REMOTEIO_TMIOT202DEVICE_VERIFY_FAIL;
                                            }
                                        }
                                        break;
                                    default:
                                        break;
                                }
                                if (bErr)
                                {
                                    // DB_SetDWORD(DBINDEX_M28_MONITOR_STATE_ERROR1, DB_MakeError(DBDEVICEID_REMOTEIO1 + eRemoteIONum, psCompUnique->PRO_bNodeID, wErrCode));
                                    Set_CommChipErrorDetail(ACTAXIS_MOLD, wErrCode, psCompUnique->PRO_bNodeID); // THJ 2017-1-13 Modify.
                                    CANOpenMaster_ResetComm(psCompUnique);
                                }
                            }
                            break;
                    }
                    break;

                default:
                    break;
            }
        }
        break;
        case CANOPENEVENTTYPE_CHANGE_NODESTATE:;
            {
                E_RemoteIONum eRemoteIONum;
                WORD          wLinkState;

                wLinkState   = DB_GetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE);
                eRemoteIONum = (E_RemoteIONum)(psRemoteIO - g_asCommRemoteIO);
                if (CANOPENNODESTATE_OPERATIONAL == psCompUnique->PRO_bNodeState)
                {
                    wLinkState |= (WORD)1 << eRemoteIONum;
                    SCommComp_InitCustomFunc(&psRemoteIO->_sComp, CANOpen_CustomFunc_Operation);
                    // psRemoteIO->PRI_bReqNodeGruadState = TRUE;   // THJ 2015-05-06 Mask:
                    switch (psRemoteIO->_eDeviceType)
                    {
                        case REMOTEIODEVICETYPE_MT_HUB_8A:
                            psRemoteIO->uExtendData._sMT_HUB_8A.iError = 0;
                            ResetTimer(&psRemoteIO->uExtendData._sMT_HUB_8A.sCtrlTimerOfReadError, MT_HUB_8A_READ_ERROR_CYCLE);
                            break;
                        case REMOTEIODEVICETYPE_TMIOT105:
                        case REMOTEIODEVICETYPE_TMIOT201:
                        case REMOTEIODEVICETYPE_TMIOT202:
                            Comm_SetReadFlagByDBIndex(DBINDEX_REMOTEIODEVICE1_INTERFACE_SOFTWARE_SN + eRemoteIONum * DBGROUPINDEX_REMOTEIODEVICE_SUM);
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    wLinkState &= ~((WORD)1 << eRemoteIONum);
                    SCommComp_InitCustomFunc(&psRemoteIO->_sComp, NULL);
                }
                DB_SetWORD(DBINDEX_M28_MONITOR_REMOTEIO_CARD_LINK_STATE, wLinkState);
                CANOpenCHBManager_Reset(&psRemoteIO->_sCHBManager);
                CANOpenCHBManager_Reset(&psRemoteIO->_sPDOManager);
                ResetTimer(&psRemoteIO->_dwPDOSendTime, psRemoteIO->wSendCycle);
                ResetTimer(&psRemoteIO->_dwPreAlarmCtrlTimer, 1000);
            }
            break;
        default:
            DEBUG_ASSERT(false);
            break;
    }
}

/**
 * [Ruler_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
// static int Ruler_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S *psCompUnique, CANOpenCommSDOMsg_S *psRetSDOMsg, WORD wConfigIndex)
// {
//     const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] =
//     {
//         //{ tODID,      bODSubIndex,    bSDOAccess,                 dwValue                         }
//         // Configuration.
//         {   0x1017,     0x00,           CANCOMMSDOACCESS_DOWNLOAD,  RULER_HEARTBEAT_CYCLE           },
//     };
//
//     if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
//         return TRUE;
//     DEBUG_ASSERT;(NULL != psRetSDOMsg)
//     *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
//     return FALSE;
// }

/**
 * [Picker_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool Picker_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        { 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, PICKER_HEARTBEAT_CYCLE },
        // CANOPENFUNCID_RECV_PDO1
        { 0x1400, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x200 },
        { 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 3 },
        { 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x28010020 }, // INTERFACE_MOLD_POSI
        { 0x1600, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x26020010 }, // INTERFACE_EUROMAP_INPUT
        { 0x1600, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x28100010 }, // INTERFACE_EJECT_POSI
        // CANOPENFUNCID_SEND_PDO1,
        { 0x1800, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x180 },
        { 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        { 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 3 },
        { 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x27130020 }, // INTERFACE_ARM_Y_POSI
        { 0x1A00, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x26030010 }, // INTERFACE_EUROMAP_OUTPUT
        { 0x1A00, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x24500010 }, // INTERFACE_PICKER_WARN
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
        return true;
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1400:
        case 0x1800:
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        default:
            break;
    }
    return false;
}

/**
 * [MT_HUB_8A_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool MT_HUB_8A_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    CANOpenCommSDOMsg_S const asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        //{ 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        //{ 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        //{ 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        { 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 2, MT_HUB_8A_HEARTBEAT_CYCLE }, // THJ 2016-11-16 Add: MT_HUB modified.
        //{ 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 2, MT_HUB_8A_HEARTBEAT_CYCLE },    // THJ 2016-11-16 Mask: MT_HUB modified.
        //{ 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 1, MT_HUB_8A_HEARTBEAT_LOSE_MAX }, // THJ 2016-11-16 Mask: MT_HUB modified.
        // CANOPENFUNCID_SEND_PDO1.
        { 0x1800, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x180 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 2, 0 },
        { 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200120 },
        { 0x1A00, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200220 },
        { 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
        // CANOPENFUNCID_SEND_PDO2.
        { 0x1801, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x280 },
        { 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 2, 0 },
        { 0x1A01, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200320 },
        { 0x1A01, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200420 },
        { 0x1A01, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
        // CANOPENFUNCID_SEND_PDO3.
        { 0x1802, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x380 },
        { 0x1802, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 2, 0 },
        { 0x1A02, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200520 },
        { 0x1A02, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200620 },
        { 0x1A02, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
        // CANOPENFUNCID_SEND_PDO4.
        { 0x1803, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x480 },
        { 0x1803, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 2, 0 },
        { 0x1A03, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200720 },
        { 0x1A03, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200820 },
        { 0x1A03, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
    {
        //S_CommRemoteIO * psRemoteIO;
        //E_CANChannel eChannel;
        //CANOpenDataFrame_S sDataFrame;

        //psRemoteIO = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
        //eChannel = CANOpenMaster_CommTypeToChannel(CommComp_GetCommType(&psRemoteIO->_sComp));

        //sDataFrame.uHead.tAll = 0;
        //sDataFrame.uData.sU8.bData0 = 0;
        //sDataFrame.uData.sU8.bData1 = 5;
        //sDataFrame.tDataLen = 2;
        //CANOpen_Write(eChannel, &sDataFrame);
        return true;
    }
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        //case 0x1400:
        case 0x1800:
        case 0x1801:
        case 0x1802:
        case 0x1803:
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        default:
            break;
    }
    return false;
}

/**
 * [GERMANJET_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool GERMANJET_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    CANOpenCommSDOMsg_S const asSDOMsgConfigTab[] = {
        //{ tODID , bODSubIndex , bSDOAccess , bBytes , dwValue }
        // Configuration.
        //{ 0x1017, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, GERMANJET_HEARTBEAT_CYCLE },
        { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 2, GERMANJET_HEARTBEAT_CYCLE },
        { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 1, GERMANJET_HEARTBEAT_LOSE_MAX },
        // CANOPENFUNCID_SEND_PDO1.
        //{ 0x1800, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x180 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200120 },
        //{ 0x1A00, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200220 },
        //{ 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
        // CANOPENFUNCID_SEND_PDO2.
        //{ 0x1801, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x280 },
        { 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A01, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200320 },
        //{ 0x1A01, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200420 },
        //{ 0x1A01, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
        // CANOPENFUNCID_SEND_PDO3.
        //{ 0x1802, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x380 },
        { 0x1802, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A02, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200520 },
        //{ 0x1A02, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200620 },
        //{ 0x1A02, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
        // CANOPENFUNCID_SEND_PDO4.
        //{ 0x1803, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x480 },
        { 0x1803, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A03, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200720 },
        //{ 0x1A03, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 4, 0x60200820 },
        //{ 0x1A03, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 4, 2 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
    {
        return true;
    }
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1800:
        case 0x1801:
        case 0x1802:
        case 0x1803:
            if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        default:
            break;
    }
    return false;
}

/**
 * [tmIoT101_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool tmIoT101_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        //{ 0x1017 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , TMIOT101_HEARTBEAT_CYCLE },
        { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT101_HEARTBEAT_CYCLE },
        { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT101_HEARTBEAT_LOSE_MAX },
        // Slave Recv PDO 1
        { 0x1400, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x200 }, // Write here is to facilitate the use of tmIoTConfigSDOMsgProcess
        { 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x63000110 },
        { 0x1600, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110110 },
        { 0x1600, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110210 },
        { 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 3 },
        // Slave Send PDO 1
        { 0x1800, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x180 }, // Write here is to facilitate the use of tmIoTConfigSDOMsgProcess
        { 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        { 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x61000110 },
        { 0x1A00, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010110 },
        { 0x1A00, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010210 },
        { 0x1A00, 0x04, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010310 },
        { 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
        // Slave Send PDO 2
        { 0x1801, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x280 }, // Write here is to facilitate the use of tmIoTConfigSDOMsgProcess
        { 0x1801, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        { 0x1A01, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010410 },
        { 0x1A01, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010510 },
        { 0x1A01, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64010610 },
        { 0x1A01, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 3 },
        // Offline Output
        { 0xC000, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
        return true;
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1400:
        case 0x1800:
        case 0x1801:
            DEBUG_ASSERT(NULL != psCompUnique);
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        case 0xC000:
            if (psRetSDOMsg->bODSubIndex > 0)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wOfflineOutput;
            }
            break;
        default:
            break;
    }
    return false;
}
/**
 * [tmIoT102_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool tmIoT102_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        //{ 0x1017 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , TMIOT102_HEARTBEAT_CYCLE },
        { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT102_HEARTBEAT_CYCLE },
        { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT102_HEARTBEAT_LOSE_MAX },
        // Slave Recv PDO 1
        { 0x1400, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x200 },
        { 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x63000110 },
        { 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 1 },
        // Slave Send PDO 1
        { 0x1800, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x180 },
        { 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        { 0x1A00, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x61000110 },
        { 0x1A00, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 1 },
        // Offline output
        { 0xC000, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
        return true;
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1400:
        case 0x1800:
        case 0x1801:
            DEBUG_ASSERT(NULL != psCompUnique);
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        case 0xC000:
            if (psRetSDOMsg->bODSubIndex > 0)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wOfflineOutput;
            }
            break;
        default:
            break;
    }
    return false;
}

/**
 * [tmIoT105_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool tmIoT105_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT105_HEARTBEAT_CYCLE },
        { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT105_HEARTBEAT_LOSE_MAX },
        // Slave Recv PDO 1
        //{ 0x1400 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x200                       },
        { 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        //{ 0x1600 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x60000108                  },
        //{ 0x1600 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 1                           },
        // Slave Send PDO 1
        //{ 0x1800 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x180                       },
        { 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A00 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x61000110                  },
        //{ 0x1A00 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 1                           },
        // Offline output
        { 0x2200, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
        return true;
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1400:
        case 0x1800:
            DEBUG_ASSERT(NULL != psCompUnique);
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        case 0x2200:
            switch (psRetSDOMsg->bODSubIndex)
            {
                case 1:;
                    {
                        S_CommRemoteIO* psRemoteIO;

                        psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                        psRetSDOMsg->dwValue = psRemoteIO->wOfflineOutput & 0xFF;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return false;
}
/**
 * [tmIoT201_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool tmIoT201_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT201_HEARTBEAT_CYCLE },
        { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT201_HEARTBEAT_LOSE_MAX },
        // Slave Recv PDO 1
        //{ 0x1400 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x200                       },
        { 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1600, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 1 },
        { 0x1600, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x62000108 },
        // Slave Recv PDO 2
        { 0x1401, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1601, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
        { 0x1601, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110110 },
        { 0x1601, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110210 },
        { 0x1601, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110310 },
        { 0x1601, 0x04, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110410 },
        // Slave Recv PDO 3
        { 0x1402, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1602, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, 4 },
        { 0x1602, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110510 },
        { 0x1602, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110610 },
        { 0x1602, 0x03, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110710 },
        { 0x1602, 0x04, CANCOMMSDOACCESS_DOWNLOAD, 0, 0x64110810 },
        //{ 0x1600 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x60000108                  },
        //{ 0x1600 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 1                           },
        // Slave Send PDO 1
        //{ 0x1800 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x180                       },
        { 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        // Slave Send PDO 2
        { 0x1801, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1801, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        // Slave Send PDO 3
        { 0x1802, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1802, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A00 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x61000110                  },
        //{ 0x1A00 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 1                           },
        // Offline output
        { 0x2200, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
        return true;
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1400:
        case 0x1401:
        case 0x1402:
        case 0x1800:
        case 0x1801:
        case 0x1802:
            DEBUG_ASSERT(NULL != psCompUnique);
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        case 0x2200:
            switch (psRetSDOMsg->bODSubIndex)
            {
                case 1:;
                    {
                        S_CommRemoteIO* psRemoteIO;

                        psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                        psRetSDOMsg->dwValue = psRemoteIO->wOfflineOutput & 0xFF;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return false;
}

/**
 * [tmIoT202_CANOpen_ConfigSDOMsg description]
 * @param  psCompUnique [description]
 * @param  psRetSDOMsg  [description]
 * @param  wConfigIndex [description]
 * @return              [description]
 */
static bool tmIoT202_CANOpen_ConfigSDOMsg(CANOpenCompUnique_S* psCompUnique, CANOpenCommSDOMsg_S* psRetSDOMsg, WORD wConfigIndex)
{
    const CANOpenCommSDOMsg_S asSDOMsgConfigTab[] = {
        //tODID  , bODSubIndex , bSDOAccess                , bBytes , dwValue
        // Device Verification.
        { 0x1000, 0x00, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x01, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        { 0x1018, 0x02, CANCOMMSDOACCESS_UPLOAD, 0, 0 },
        // Configuration.
        { 0x100C, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT202_HEARTBEAT_CYCLE },
        { 0x100D, 0x00, CANCOMMSDOACCESS_DOWNLOAD, 0, TMIOT202_HEARTBEAT_LOSE_MAX },
        // Slave Recv PDO 1
        //{ 0x1400 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x200                       },
        { 0x1400, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        //{ 0x1600 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x60000108                  },
        //{ 0x1600 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 1                           },
        // Slave Send PDO 1
        //{ 0x1800 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x180                       },
        { 0x1800, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 254 },
        { 0x1800, 0x05, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        //{ 0x1A00 , 0x01        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 0x61000110                  },
        //{ 0x1A00 , 0x00        , CANCOMMSDOACCESS_DOWNLOAD , 0      , 1                           },
        // Offline output
        { 0x2200, 0x01, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
        { 0x2200, 0x02, CANCOMMSDOACCESS_DOWNLOAD, 0, 0 },
    };

    if (wConfigIndex >= (sizeof(asSDOMsgConfigTab) / sizeof(*asSDOMsgConfigTab)))
        return true;
    DEBUG_ASSERT(NULL != psRetSDOMsg);
    *psRetSDOMsg = asSDOMsgConfigTab[wConfigIndex];
    switch (psRetSDOMsg->tODID)
    {
        case 0x1400:
        case 0x1800:
            DEBUG_ASSERT(NULL != psCompUnique);
            if (0x01 == psRetSDOMsg->bODSubIndex)
                psRetSDOMsg->dwValue += psCompUnique->PRO_bNodeID;
            else if (0x05 == psRetSDOMsg->bODSubIndex)
            {
                S_CommRemoteIO* psRemoteIO;

                psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                psRetSDOMsg->dwValue = psRemoteIO->wSendCycle;
            }
            break;
        case 0x2200:
            switch (psRetSDOMsg->bODSubIndex)
            {
                case 1:;
                    {
                        S_CommRemoteIO* psRemoteIO;

                        psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                        psRetSDOMsg->dwValue = psRemoteIO->wOfflineOutput & 0xFF;
                    }
                    break;
                case 2:;
                    {
                        S_CommRemoteIO* psRemoteIO;

                        psRemoteIO           = ContainerOf(psCompUnique, S_CommRemoteIO, _sCANOpenUnique);
                        psRetSDOMsg->dwValue = (psRemoteIO->wOfflineOutput >> 8) & 0xFF;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return false;
}
