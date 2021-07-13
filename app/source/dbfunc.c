/*==============================================================================+
|  Function : Database function                                                 |
|  Task     : Database function Header File                                     |
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
#include "dbfunc.h"
#include "CommComp_PwmtrCard.h"
#include "CommComp_TemperCard.h"
#include "Error_App.h"
#include "Task_Watch.h"
#include "common.h"
#include "taskTemper.h"
#include "tasknet_HMI.h"
#include "temper_common.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define MOLD_INDEX_OFFSET 0x200000
//##############################################################################
//
//      Enumeration
//
//##############################################################################
const WORD cnst_AxisHomingIdx[] = {
    DBINDEX_M28_MONITOR_MOLD_HOMING,
    DBINDEX_M28_MONITOR_MOLD2_HOMING,
    DBINDEX_M28_MONITOR_EJECT_HOMING,
    DBINDEX_M28_MONITOR_EJECT2_HOMING,
    DBINDEX_M28_MONITOR_INJECT_HOMING,
    DBINDEX_M28_MONITOR_INJECT2_HOMING,
    DBINDEX_M28_MONITOR_CHARGE_HOMING,
    DBINDEX_M28_MONITOR_CHARGE2_HOMING,
    DBINDEX_M28_MONITOR_CARRIAGE_HOMING,
    DBINDEX_M28_MONITOR_CARRIAGE2_HOMING
};
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef struct
{
    const DBIndex_T* ptDBIndex;
    unsigned int     uiListNum;
} CheckList_S;

typedef struct Struct_DBInflSeg
{
    DBIndex_T tOffsetStart;
    DBIndex_T tOffsetEnd;
    DBIndex_T tInc;
} S_DBInflSeg;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void DBWriteInflSeg_Arose(S_DBInflSeg const* pInflSeg, DBIndex_T tBase);
static void DBWriteInfl_Arose(DBIndex_T const* pIndex, unsigned iTabs, DBIndex_T tBase);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
const DBIndex_T uc_NodeIDCheck_CAN1Tab[] = {
    DBINDEX_M28_CONFIG_MOTION_CARD1_NODEID,
    DBINDEX_M28_CONFIG_MOTION_CARD2_NODEID,
    DBINDEX_M28_CONFIG_MOTION_CARD3_NODEID,
    DBINDEX_M28_CONFIG_MOTION_CARD4_NODEID,
    DBINDEX_M28_CONFIG_MOTION_CARD5_NODEID
};

const DBIndex_T uc_NodeIDCheck_CAN2Tab[] = {
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD1_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD2_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD3_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD4_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD5_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD6_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD7_NODEID,
    DBINDEX_M28_CONFIG_TEMPERATURE_CARD8_NODEID,
    DBINDEX_M28_CONFIG_PAMIR_CARD_NUMBER1_NODEID,
    DBINDEX_M28_CONFIG_PAMIR_CARD_NUMBER2_NODEID,
    DBINDEX_M28_CONFIG_PAMIR_CARD_NUMBER3_NODEID,
};

// hankin 20181130 add: backup set kp & ki.
WORD u_aTemperCardKpBackup[1 + (DBINDEX_TEMPER7_INTERFACE_SEG12_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / (DBINDEX_TEMPER1_INTERFACE_SEG2_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE)];
WORD u_aTemperCardKiBackup[1 + (DBINDEX_TEMPER7_INTERFACE_SEG12_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / (DBINDEX_TEMPER1_INTERFACE_SEG2_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE)];
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [CAN1NodeID_Check description]
 * @return  [description]
 */
int CAN1NodeID_Check(void)
{
    unsigned int i, j;
    WORD         awNodeID[sizeof(uc_NodeIDCheck_CAN1Tab) / sizeof(*uc_NodeIDCheck_CAN1Tab)];

    for (i = 0; i < (sizeof(uc_NodeIDCheck_CAN1Tab) / sizeof(*uc_NodeIDCheck_CAN1Tab)); ++i)
        awNodeID[i] = DB_GetWORD(uc_NodeIDCheck_CAN1Tab[i]);

    for (i = 0; i < (sizeof(uc_NodeIDCheck_CAN1Tab) / sizeof(*uc_NodeIDCheck_CAN1Tab)); ++i)
    {
        if (0 == awNodeID[i])
            continue;
        // if (awNodeID[i] > 0x7F)
        // {
        //  Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_CANCH1_NODEID_CONFLICT);
        //  return 1;
        // }

        for (j = i + 1; j < (sizeof(uc_NodeIDCheck_CAN1Tab) / sizeof(*uc_NodeIDCheck_CAN1Tab)); ++j)
        {
            if (awNodeID[i] == awNodeID[j])
            {
                Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_CANCH1_NODEID_CONFLICT, awNodeID[i]); // THJ 2017-1-13 Modify.
                return 1;
            }
        }
    }
    return 0;
}
/**
 * [CAN2NodeID_Check description]
 * @return  [description]
 */
int CAN2NodeID_Check(void)
{
    unsigned int i, j;
    WORD         awNodeID[sizeof(uc_NodeIDCheck_CAN2Tab) / sizeof(*uc_NodeIDCheck_CAN2Tab)];

    for (i = 0; i < (sizeof(uc_NodeIDCheck_CAN2Tab) / sizeof(*uc_NodeIDCheck_CAN2Tab)); ++i)
        awNodeID[i] = DB_GetWORD(uc_NodeIDCheck_CAN2Tab[i]);

    for (i = 0; i < (sizeof(uc_NodeIDCheck_CAN2Tab) / sizeof(*uc_NodeIDCheck_CAN2Tab)); ++i)
    {
        if (0 == awNodeID[i])
            continue;
        // if (awNodeID[i] > 0x7F)
        // {
        //  Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_CANCH2_NODEID_CONFLICT);
        //  return 1;
        // }

        for (j = i + 1; j < (sizeof(uc_NodeIDCheck_CAN2Tab) / sizeof(*uc_NodeIDCheck_CAN2Tab)); ++j)
        {
            if (awNodeID[i] == awNodeID[j])
            {
                Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_CANCH2_NODEID_CONFLICT, awNodeID[i]); // THJ 2017-1-13 Modify.
                return 1;
            }
        }
    }
    return 0;
}
/**
 * [AxisNum_Check description]
 * @return  [description]
 */
//#if (HYBCARD_USED)
//int AxisNum_Check(void)
//{
//    unsigned int i, j;
//    WORD         awAxisNum[DBINDEX_LOCAL0_CARRIAGE2_AXIS_NUMBER - DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER + 1];

//    for (i = 0; i < (sizeof(awAxisNum) / sizeof(*awAxisNum)); ++i)
//        awAxisNum[i] = DB_GetWORD(i + DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER);
//    for (i = 0; i < (sizeof(awAxisNum) / sizeof(*awAxisNum)); ++i)
//    {
//        if (0 == awAxisNum[i])
//            continue;

//        // check axis num error
//        if ((awAxisNum[i] > (8900 + AXIS_MAX)) || ((awAxisNum[i] > AXISCARDAXIS_SUM) && (awAxisNum[i] <= 8900)))
//        {
//            Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_MOTIONAXIS_NUM);
//            return 2;
//        }

//        // check axis num repeat
//        for (j = i + 1; j < (sizeof(awAxisNum) / sizeof(*awAxisNum)); ++j)
//        {
//            if (awAxisNum[i] == awAxisNum[j])
//            {
//                Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_MOTIONAXIS_CONFLICT);
//                return 1;
//            }
//        }
//    }
//    return 0;
//}
//#endif

/**
 * [UpdateAllAxisNum_AxisCard description]
 */
//#if (HYBCARD_USED)
//void UpdateAllAxisNum_AxisCard(void)
//{
//    unsigned i;

//    // clear all axis.
//    for (i = AXISCARDAXIS_MOLD0; i < AXISCARDAXIS_INJECTVALVE0; ++i)
//    {
//        SetAxisByAxisNum_AxisCard(i, 0);
//    }
//    for (i = AXISCARDAXIS_INJECTVALVE0; i < AXISCARDAXIS_SUM; ++i)
//    {
//        SetInjectValveByAxisNum_AxisCard(i, 0);
//    }

//    // reset axis by database.
//    for (i = AXISCARDAXIS_MOLD0; i < AXISCARDAXIS_INJECTVALVE0; ++i)
//    {
//        SetAxisByAxisNum_AxisCard(i, DB_GetWORD(DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER + i));
//    }
//    for (i = AXISCARDAXIS_INJECTVALVE0; i < AXISCARDAXIS_SUM; ++i)
//    {
//        SetInjectValveByAxisNum_AxisCard(i, DB_GetWORD(DBINDEX_LOCAL0_INJECTVALVE1_AXIS_NUMBER + i));
//    }
//}
//#endif

/**
 * [DB_SetInjectValveAxisNum description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetInjectValveAxisNum(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//#if (HYBCARD_USED)
//    // WORD i, wNum;
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            if (AxisNum_Check() == 0)
//            {
//                //SetInjectValveByAxisNum_AxisCard((tDBIndex - DBINDEX_LOCAL0_INJECTVALVE1_AXIS_NUMBER + AXISCARDAXIS_INJECTVALVE0), DB_GetWORD(tDBIndex));
//                UpdateAllAxisNum_AxisCard();
//            }
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//#else
//    return DBRETCODE_SUCCESS;
//#endif
//}

/**
 * [DB_AddAxisToCard description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_AddAxisToCard(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//#if (HYBCARD_USED)
//    //DEBUG_ASSERT(tDBIndex >= 0)
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            //Set_MoldAxis(ReadDB_WORD(tDBIndex));
//            if (AxisNum_Check() == 0)
//            {
//                //SetAxisByAxisNum_AxisCard((tDBIndex - DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER), DB_GetWORD(tDBIndex));
//                UpdateAllAxisNum_AxisCard();
//            }
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//#else
//    return DBRETCODE_SUCCESS;
//#endif
//}
/**
 * [DB_SetNodeID_Card description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetNodeID_Card(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//#if (HYBCARD_USED)
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            if (0 != CAN1NodeID_Check())
//                return DBRETCODE_ACCESS_ERR_DATA;
//            SetNodeId_AxisCard((tDBIndex - DBINDEX_LOCAL0_MOTION_CARD1_NODEID), DB_GetWORD(tDBIndex));
//            Error_App_Reset();
//            //AssignECSlaveHandle_MotionCard(tDBIndex);   // Anders 2015-10-27, Add.
//            {
//                S_DBInflSeg const sInflSeg = {
//                    DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER - DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER,
//                    DBINDEX_LOCAL0_CARRIAGE2_AXIS_NUMBER - DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER,
//                    1
//                };
//
//                DBWriteInflSeg_Arose(&sInflSeg, DBINDEX_LOCAL0_MOLD1_AXIS_NUMBER);
//            }
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//#else
//    return DBRETCODE_SUCCESS;
//#endif
//}
/**
 * [DB_TemperCardSetNodeID description]
 * @param eTemperCardNum [description]
 * @param byNodeID       [description]
 */
static void DB_TemperCardSetNodeID(TemperCardNum_E eTemperCardNum, BYTE byNodeID)
{
    CommTemperCard_S* psTemperCard;

    DEBUG_ASSERT(eTemperCardNum < TEMPERCARDNUM_SUM);
    psTemperCard                = &g_asCommTemperCard[eTemperCardNum];
    psTemperCard->PRI_xSegStart = eTemperCardNum * 12;
    //if ((byNodeID >= 128) && (byNodeID < 256)) {
    //  psTemperCard->PRI_xSegNext = psTemperCard->PRI_xSegStart + 16;
    //  byNodeID -= 128;
    //} else {
    psTemperCard->PRI_xSegNext = psTemperCard->PRI_xSegStart + 12;
    //}
    SCommComp_SetCommType(&psTemperCard->PRI_sComp, (0 != byNodeID) ? TEMPERCARDNUM_DEFAULT_COMMTYPE : COMMTYPE_SUM);
    SCANOpenCompUnique_SetNodeID(&psTemperCard->PRI_sCANOpenUnique, byNodeID);
    /*
    if (0 == byNodeID)
    {
        if (CANCHANNAL_SUM != eChannel)
        {
            CANOpenMaster_DelComp(&psTemperCard->PRI_sComp);
        }
    }
    else
    {
        if (CANCHANNAL_SUM == eChannel)
        {
            CANOpenMaster_AddComp(TEMPERCARDNUM_DEFAULT_COMMTYPE, &psTemperCard->PRI_sComp);
        }
    }
    */
}
/**
 * [DB_SetNodeID_TemperCard description]
 * @method DB_SetNodeID_TemperCard
 * @param  tDBIndex                [description]
 * @param  eDBEvent                [description]
 * @param  pvDBData                [description]
 * @return                         [description]
 */
E_DBRetCode DB_SetNodeID_TemperCard(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //DEBUG_ASSERT(tDBIndex >= 0)
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            if (0 != CAN2NodeID_Check())
                return DBRETCODE_ACCESS_ERR_DATA;
            DB_TemperCardSetNodeID((TemperCardNum_E)(tDBIndex - DBINDEX_M28_CONFIG_TEMPERATURE_CARD1_NODEID), DB_GetWORD(tDBIndex));
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}
/**
 * [DB_TemperCard_tSensorType description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_tSensorType(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{

    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                S_DBInflSeg const sInflSeg = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    DBINDEX_POWERCARD1_CONFIG_CARD_ID - 1 - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    TEMPER_ONESEG_DATANBR
                };

                DBWriteInflSeg_Arose(&sInflSeg, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 * [DB_TemperCard_HeatMinTime description]
 * @method DB_TemperCard_HeatMinTime
 * @param  tDBIndex                  [description]
 * @param  eDBEvent                  [description]
 * @param  pvDBData                  [description]
 * @return                           [description]
 */
E_DBRetCode DB_TemperCard_HeatMinTime(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //DEBUG_ASSERT(tDBIndex >= 0)
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                S_DBInflSeg const sInflSeg = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    DBINDEX_POWERCARD1_CONFIG_CARD_ID - 1 - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    TEMPER_ONESEG_DATANBR
                };

                DBWriteInflSeg_Arose(&sInflSeg, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 * [DB_TemperCard_HeatCmd description]
 * @method DB_TemperCard_HeatCmd
 * @param  tDBIndex              [description]
 * @param  eDBEvent              [description]
 * @param  pvDBData              [description]
 * @return                       [description]
 */
E_DBRetCode DB_TemperCard_HeatCmd(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //DEBUG_ASSERT(tDBIndex >= 0)
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            if (!GetMoldsetState())
                return DBRETCODE_SUCCESS;
            DB_SetWORD(tDBIndex, puDBData->vWORD);
            TemperCommon_ChangeHeatCmd();
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    };
}
/**
 * [DB_TemperCard_SoftHeat description]
 * @method DB_TemperCard_SoftHeat
 * @param  tDBIndex               [description]
 * @param  eDBEvent               [description]
 * @param  pvDBData               [description]
 * @return                        [description]
 */
E_DBRetCode DB_TemperCard_SoftHeat(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            TemperCommon_ChangeSoftHeat();
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}
/**
 * [DB_TemperCard_HeatType description]
 * @method DB_TemperCard_HeatType
 * @param  tDBIndex               [description]
 * @param  eDBEvent               [description]
 * @param  pvDBData               [description]
 * @return                        [description]
 */
E_DBRetCode DB_TemperCard_HeatType(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                S_DBInflSeg const sInflSeg = {
                        0,
                        31 * TEMPER_ONESEG_DATANBR,
                        TEMPER_ONESEG_DATANBR
                };

                DBWriteInflSeg_Arose(&sInflSeg, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_WAY + 32 * TEMPER_ONESEG_DATANBR * (tDBIndex - DBINDEX_M28_TEMPERATURE_HEAT_TYPE_CONFIG1));
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 * [DB_TemperCard_Seg_HeatWay description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_HeatWay(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                DWORD           dwHeatTypeFlag;
                WORD            wSegNum;
                DBIndex_T const atInfl[] = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_WAY,
                };

                wSegNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_WAY) / TEMPER_ONESEG_DATANBR;
                DEBUG_ASSERT(wSegNum < (DBINDEX_POWERCARD1_CONFIG_CARD_ID - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / TEMPER_ONESEG_DATANBR);
                dwHeatTypeFlag = DB_GetDWORD(DBINDEX_M28_TEMPERATURE_HEAT_TYPE_CONFIG1 + (wSegNum >> 5));
                DB_SetWORD(tDBIndex, (0 != (dwHeatTypeFlag & ((DWORD)1 << (wSegNum & 0x1F)))) ? TEMPER_HEATWAY_RELAY : TEMPER_HEATWAY_SSR);
                DBWriteInfl_Arose(atInfl, sizeof(atInfl) / sizeof(*atInfl), tDBIndex);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_Seg_OutputCycle description]
 * @method DB_TemperCard_Seg_OutputCycle
 * @param  tDBIndex                      [description]
 * @param  eDBEvent                      [description]
 * @param  pvDBData                      [description]
 * @return                               [description]
 */
E_DBRetCode DB_TemperCard_Seg_OutputCycle(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD wTmp;

                wTmp = DB_GetWORD((tDBIndex + ((DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_USE - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE)));
                switch (wTmp)
                {
                    case TEMPERHEATUSE_ABSTRACT:
                        if(DB_GetWORD(DBINDEX_M28_TEMPERATURE_MOLD_FAST_OPT) == 2)// PID + Manual Set TemperOutputCycle. Anders 2020-11-17 add.
                        {
                            //shanlinlin 20201228 add for special test
                            // if((tDBIndex >= DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE)&&(tDBIndex <= DBINDEX_TEMPER1_INTERFACE_SEG12_SET_OUTPUT_CYCLE))
                            //     wTmp = 1000 + (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE)/DBGROUPINDEX_TEMPER_SEG_SUM*100;
                            // else if((tDBIndex >= DBINDEX_TEMPER2_INTERFACE_SEG1_SET_OUTPUT_CYCLE)&&(tDBIndex <= DBINDEX_TEMPER2_INTERFACE_SEG12_SET_OUTPUT_CYCLE))
                            //     wTmp = 1000 + (tDBIndex - DBINDEX_TEMPER2_INTERFACE_SEG1_SET_OUTPUT_CYCLE)/DBGROUPINDEX_TEMPER_SEG_SUM*100;
                            // else
                                wTmp=DB_GetWORD(tDBIndex);
                        }
                        else if (DB_GetWORD(DBINDEX_M28_TEMPERATURE_MOLD_FAST_OPT == 1) && (TEMPER_DEV_MOLD == DB_GetWORD(tDBIndex + ((DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE))))
                        {
                            wTmp = 100;
                        }
                        else
                        {
                            wTmp = DB_GetWORD(tDBIndex + ((DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_WAY - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE));
                            switch (wTmp)
                            {
                                case TEMPER_HEATWAY_SSR:
                                    wTmp = 1000;
                                    break;
                                case TEMPER_HEATWAY_RELAY:
                                    wTmp = DB_GetWORD(DBINDEX_M28_TEMPERATURE_MINIMUM_HEAT_TIME); // 6000
                                    break;
                                default:
                                    wTmp = DB_GetWORD(tDBIndex);
                                    break;
                            }
                        }
                        break;
                    case TEMPERHEATUSE_RELATIVE:
                        if(DB_GetWORD(DBINDEX_M28_TEMPERATURE_MOLD_FAST_OPT) == 2)// PID + Manual Set TemperOutputCycle. Anders 2020-11-17 add.
                        {
                            wTmp=DB_GetWORD(tDBIndex);
                        }
                        else
                            wTmp = DB_GetWORD(DBINDEX_M28_TEMPERATURE_MINIMUM_HEAT_TIME); // Anders 2015-3-19, Mark.
                        //if (0 == wTmp)                                                     // Anders 2016-11-02, Mark.
                        //  wTmp = DB_GetWORD(tDBIndex);
                        break;
                    default:
                        wTmp = 0;
                        break;
                }
                DB_SetWORD(tDBIndex, wTmp);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_Seg_Enhance_ratio description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_Enhance_ratio(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                DBIndex_T const atInfl[] = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_PID_KP - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_ENHANCE_RATIO,
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_PID_KI - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_ENHANCE_RATIO,
                };
                WORD wSeg;

                // hankin 20181130 add: restore set kp & ki.
                wSeg = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / (DBINDEX_TEMPER1_INTERFACE_SEG2_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
                DEBUG_ASSERT(wSeg < (sizeof(u_aTemperCardKpBackup) / sizeof(*u_aTemperCardKpBackup)));
                DB_SetWORD(tDBIndex + atInfl[0], u_aTemperCardKpBackup[wSeg]);
                DB_SetWORD(tDBIndex + atInfl[1], u_aTemperCardKiBackup[wSeg]);
                DBWriteInfl_Arose(atInfl, sizeof(atInfl) / sizeof(*atInfl), tDBIndex);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_Seg_Set_PID_KP description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_Set_PID_KP(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                extern bool g_bAutoMode;
                WORD        wSeg;

                // hankin 20181130 add: backup set kp.
                wSeg = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / (DBINDEX_TEMPER1_INTERFACE_SEG2_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
                DEBUG_ASSERT(wSeg < (sizeof(u_aTemperCardKpBackup) / sizeof(*u_aTemperCardKpBackup)));
                u_aTemperCardKpBackup[wSeg] = puDBData->vWORD;
                if (g_bAutoMode)
                {
                    WORD wEnhance_ratio;

                    wEnhance_ratio = DB_GetWORD(tDBIndex - (DBINDEX_TEMPER1_INTERFACE_SEG1_SET_PID_KP - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_ENHANCE_RATIO));
                    DB_SetWORD(tDBIndex, u_aTemperCardKpBackup[wSeg] * (wEnhance_ratio + 1));
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_Seg_Set_PID_KP description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_Set_PID_KI(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                extern bool g_bAutoMode;
                WORD        wSeg;

                // hankin 20181130 add: backup set ki.
                wSeg = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / (DBINDEX_TEMPER1_INTERFACE_SEG2_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
                DEBUG_ASSERT(wSeg < (sizeof(u_aTemperCardKiBackup) / sizeof(*u_aTemperCardKiBackup)));
                u_aTemperCardKiBackup[wSeg] = puDBData->vWORD;
                if (g_bAutoMode)
                {
                    WORD wEnhance_ratio;

                    wEnhance_ratio = DB_GetWORD(tDBIndex - (DBINDEX_TEMPER1_INTERFACE_SEG1_SET_PID_KI - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_ENHANCE_RATIO));
                    DB_SetWORD(tDBIndex, u_aTemperCardKiBackup[wSeg] / (wEnhance_ratio + 1));
                }
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_Seg_tSensorType description]
 * @method DB_TemperCard_Seg_tSensorType
 * @param  tDBIndex                      [description]
 * @param  eDBEvent                      [description]
 * @param  pvDBData                      [description]
 * @return                               [description]
 */
E_DBRetCode DB_TemperCard_Seg_tSensorType(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //DEBUG_ASSERT(tDBIndex >= 0)
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD wTmp;
                WORD wCardNum;

                wCardNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / DBGROUPINDEX_TEMPER_SUM;
                if (wCardNum > 0)
                {
                    --wCardNum;
                    if (g_asCommTemperCard[wCardNum].PRI_xSegNext - g_asCommTemperCard[wCardNum].PRI_xSegStart == 12)
                        ++wCardNum;
                }
                switch (DB_GetWORD(tDBIndex + (DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE)))
                {
                    case TEMPER_DEV_PIPE:
                        wTmp = DB_GetWORD(DBINDEX_M28_TEMPERATURE_PIPE_SENSOR_TYPE);
                        if (wTmp == 0xFFFF)
                            wTmp = puDBData->vWORD;
                        break;
                    case TEMPER_DEV_MOLD:
                        wTmp = DB_GetWORD(DBINDEX_M28_TEMPERATURE_MOLD_SENSOR_TYPE);
                        if (wTmp == 0xFFFF)
                            wTmp = puDBData->vWORD;
                        break;
                    default:
                        wTmp = puDBData->vWORD;
                        break;
                }
                if (DB_GetDWORD(DBINDEX_TEMPERCARD1_INTERFACE_SOFTWARE_SN + (wCardNum * DBGROUPINDEX_TEMPERCARD_SUM)) >= 0x31303000)
                { // m3: "1.x.x.x"; m4: "2.x.x.x".
                    wTmp |= 0x0100;
                }
                DB_SetWORD(tDBIndex, wTmp);
                break;
            }
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_Seg_Use description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_Use(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD            wTemperSegNum;
                DBIndex_T const atInfl[] = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_USE,
                };

                wTemperSegNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / TEMPER_ONESEG_DATANBR;
                TemperCommon_ChangeUse(wTemperSegNum); // Anders 2015-2-13, add.
                DBWriteInfl_Arose(atInfl, sizeof(atInfl) / sizeof(*atInfl), tDBIndex);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_TemperCard_UpdateSoftSN description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_UpdateSoftSN(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD wCardNum;

                S_DBInflSeg const sInflSeg12 = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    DBINDEX_TEMPER1_INTERFACE_SEG12_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    TEMPER_ONESEG_DATANBR
                };
                S_DBInflSeg const sInflSeg16 = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    DBINDEX_TEMPER2_INTERFACE_SEG4_SET_SENSOR_TYPE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    TEMPER_ONESEG_DATANBR
                };

                wCardNum = (tDBIndex - DBINDEX_TEMPERCARD1_CONFIG_CARD_ID) / DBGROUPINDEX_TEMPERCARD_SUM;
                switch (g_asCommTemperCard[wCardNum].PRI_xSegNext - g_asCommTemperCard[wCardNum].PRI_xSegStart)
                {
                    case 12:
                        DBWriteInflSeg_Arose(&sInflSeg12, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE + wCardNum * DBGROUPINDEX_TEMPER_SUM);
                        break;
                    case 16:
                        DBWriteInflSeg_Arose(&sInflSeg16, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE + wCardNum * DBGROUPINDEX_TEMPER_SUM);
                        break;
                    default:
                        DEBUG_ASSERT_WARN(false, "not support.");
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
 * [DB_TemperCard_Seg_DeviceType description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_DeviceType(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                DBIndex_T const atInfl[] = {
                    (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE,
                    (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE,
                    (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_CHECK_DEGREE - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE,
                };

                DBWriteInfl_Arose(atInfl, sizeof(atInfl) / sizeof(*atInfl), tDBIndex);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
    /*20200626 先还原为20200609的temperlib
    WORD i;
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
        {
            DBIndex_T const atInfl[] = {
            (DBIndex_T) DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE - (DBIndex_T) DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE,
            (DBIndex_T) DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE - (DBIndex_T) DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE,
            (DBIndex_T) DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_CHECK_DEGREE - (DBIndex_T) DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE,
        };
        i = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE) / DBGROUPINDEX_TEMPER_SEG_SUM;
        TemperCommon_Set_MicroAdj(i, g_sDB.M28.TEMPERATURE.RAPID_COMPENSATION_USE & (1 << puDBData->vWORD));
        DBWriteInfl_Arose(atInfl, sizeof(atInfl) / sizeof(*atInfl), tDBIndex);
        }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
    */
}

/**
 * [DB_PwmtrCardSetNodeID description]
 * @param ePwmtrCardNum [description]
 * @param byNodeID      [description]
 */
static void DB_PwmtrCardSetNodeID(PwmtrCardNum_E ePwmtrCardNum, BYTE byNodeID)
{
    CommPwmtrCard_S* psPwmtrCard;

    DEBUG_ASSERT(ePwmtrCardNum < PWMTRCARDNUM_SUM);
    psPwmtrCard = &g_asCommPwmtrCard[ePwmtrCardNum];
    SCommComp_SetCommType(&psPwmtrCard->PRI_sComp, (0 != byNodeID) ? PWMTRCARDNUM_DEFAULT_COMMTYPE : COMMTYPE_SUM);
    SCANOpenCompUnique_SetNodeID(&psPwmtrCard->PRI_sCANOpenUnique, byNodeID);
}
/**
 * [DB_SetNodeID_PwmtrCard description]
 * @method DB_SetNodeID_PwmtrCard
 * @param  tDBIndex               [description]
 * @param  eDBEvent               [description]
 * @param  pvDBData               [description]
 * @return                        [description]
 */
E_DBRetCode DB_SetNodeID_PwmtrCard(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //DEBUG_ASSERT(tDBIndex >= 0)
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            if (0 != CAN2NodeID_Check())
                return DBRETCODE_ACCESS_ERR_DATA;
            DB_PwmtrCardSetNodeID((PwmtrCardNum_E)(tDBIndex - DBINDEX_M28_CONFIG_PAMIR_CARD_NUMBER1_NODEID), DB_GetWORD(tDBIndex));
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    };
}
/**
 * [DB_ReadMoldData description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
// E_DBRetCode DB_ReadMoldData(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const *puDBData)
// {
//  DWORD dwDBData;

//  DEBUG_ASSERT;;(tDBIndex >= 0)
//  DEBUG_ASSERT;;(tDBIndex < DBINDEX_SUM)
//  switch (eDBEvent)
//  {
//      case DB_EVENT_READ_BEFORE:
//          Axiscard_GetValue(DB_IndexToID(tDBIndex) + MOLD_INDEX_OFFSET, &dwDBData);
//          DB_SetData(tDBIndex, &dwDBData);
//          return DBRETCODE_SUCCESS;
//      default:
//          D_ASSERT_PRO(FALSE)//D_ASSERT_PRO(DB_EVENT_READ_BEFORE == eDBEvent)
//          return DBRETCODE_SUCCESS;
//  }
// }
/**
 * [DB_SetHoming description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetHoming(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    //unsigned int i;
//    //DEBUG_ASSERT(tDBIndex >= 0)
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            //if (puDBData->vWORD != 1 || !GetMoldsetState()) // Anders 2015-3-18, Add.
//            //    return DBRETCODE_SUCCESS;
//            //for (i = 0; i < (sizeof(cnst_AxisHomingIdx) / sizeof(*cnst_AxisHomingIdx)); ++i)
//            //{
//            //    if (cnst_AxisHomingIdx[i] == tDBIndex)
//            //    {
//            //        //if ( g_AxisManager[i].Data.AxisCtlState.Info.bState == SLVSTATE_STANDSTILL)
//            //        g_AxisManager[i].wCurrCMD_Comm = CMD_HOMINGCTL;
//            //        break;
//            //    }
//            //}
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    };
//}
/**
 * [DB_SetMoldCloseCoreUse description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetMoldCloseCoreUse(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const *puDBData)
//{
//  WORD wEnableProfile;
//
//  DEBUG_ASSERT;;(tDBIndex >= 0)
//  DEBUG_ASSERT;;(tDBIndex < DBINDEX_SUM)
//  switch (eDBEvent)
//  {
//      case DBEVENT_WRITE_AFTER:
//          //EnableProfile.wData = DB_GetWORD(MOLD_INTERFACE_OPENMOLDCORE_WENABLEDPROFILE); // Anders 2014-6-27 marked.
//          wEnableProfile = DB_GetWORD(MOLD_INTERFACE_CLOSE_MOLD_CORE_ENABLE_DIGITAL_PROFILE);
//          if (DB_GetWORD(tDBIndex) == 0)
//              wEnableProfile &= ~(1 << (tDBIndex - MASTER_MOLD_CLOSE_CORE_A_USE));
//          else
//              wEnableProfile |= 1 << (tDBIndex - MASTER_MOLD_CLOSE_CORE_A_USE);
//          return DB_SetDataBySysIndex(MOLD_INTERFACE_CLOSE_MOLD_CORE_ENABLE_DIGITAL_PROFILE, &wEnableProfile);
//      default:
//          D_ASSERT_PRO(FALSE)//D_ASSERT_PRO(DB_EVENT_READ_BEFORE == eDBEvent)
//          return DBRETCODE_SUCCESS;
//  }
//}
/**
 * [DB_SetMoldData description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
// E_DBRetCode DB_SetMoldData(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const *puDBData)
// {
//  DEBUG_ASSERT;;(tDBIndex >= 0)
//  DEBUG_ASSERT;;(tDBIndex < DBINDEX_SUM)
//  DEBUG_ASSERT;;(NULL != puDBData)
//  switch (eDBEvent)
//  {
//      case DBEVENT_WRITE_AFTER:
//          Axiscard_SetValue(DB_IndexToID(tDBIndex) + MOLD_INDEX_OFFSET, (void *)pvDBData);
//          return DBRETCODE_SUCCESS;
//      default:
//          D_ASSERT_PRO(FALSE)//D_ASSERT_PRO(DB_EVENT_READ_BEFORE == eDBEvent)
//          return DBRETCODE_SUCCESS;
//  }
// }
/**
 * [DB_SetMoldOpenCoreUse description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetMoldOpenCoreUse(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const *puDBData)
//{
//    WORD wEnableProfile;
//
//  DEBUG_ASSERT;;(tDBIndex >= 0)
//  DEBUG_ASSERT;;(tDBIndex < DBINDEX_SUM)
//    switch (eDBEvent)
//    {
//      case DBEVENT_WRITE_AFTER:
//          wEnableProfile = DB_GetWORD(MOLD_INTERFACE_OPEN_MOLD_CORE_ENABLE_DIGITAL_PROFILE);
//          if (0 == DB_GetWORD(tDBIndex))
//              wEnableProfile &= ~(1<<(tDBIndex - MASTER_MOLD_OPEN_CORE_A_USE));
//          else
//              wEnableProfile |= 1<<(tDBIndex - MASTER_MOLD_OPEN_CORE_A_USE);
//          return DB_SetDataBySysIndex(MOLD_INTERFACE_OPEN_MOLD_CORE_ENABLE_DIGITAL_PROFILE, &wEnableProfile);
//      default:
//          D_ASSERT_PRO(FALSE)//D_ASSERT_PRO(DB_EVENT_READ_BEFORE == eDBEvent)
//          return DBRETCODE_SUCCESS;
//  }
//}
/**
 * [DB_SetMoldSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetMoldSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DBIndex_T const atProfileSetIndex[] = {
//        DBINDEX_MOLD0_INTERFACE_OPEN_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_MOLD0_INTERFACE_CLOSE_ENABLE_DIGITAL_PROFILE,
//    };

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_MOLD_OPEN_SEGMENTS_NUMBER], puDBData);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_SetMoldSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetMold2SegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DBIndex_T const atProfileSetIndex[] = {
//        DBINDEX_MOLD1_INTERFACE_OPEN_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_MOLD1_INTERFACE_CLOSE_ENABLE_DIGITAL_PROFILE,
//    };

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_MOLD2_OPEN_SEGMENTS_NUMBER], puDBData);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_OilCompensateLinearCalc description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_OilCompensateLinearCalc(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            OilCompensate_CalcLinear(&g_sMoldOpenOilCompensate);
//            OilCompensate_CalcLinear(&g_sMoldCloseOilCompensate);
//            break;
//        default:
//            break;
//    }
//    return DBRETCODE_SUCCESS;
//}

/**
 * [DB_MoldSetOpenSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_MoldSetOpenSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x11, 0x13, 0x17, 0x1F };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_MOLD0_INTERFACE_OPEN_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_MOLD_OPEN_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_MOLD1_INTERFACE_OPEN_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_MOLD2_OPEN_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_MoldSetCloseSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_MoldSetCloseSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x18, 0x1C, 0x1E, 0x1F };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_MOLD0_INTERFACE_CLOSE_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_MOLD_CLOSE_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_MOLD1_INTERFACE_CLOSE_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_MOLD2_CLOSE_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_MoldOpenParamPosiCtrlKP description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_MoldOpenParamPosiCtrlKP(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            g_sMoldOpenOilCompensate.backupKp = puDBData->fData;
//            OilCompensate_Run(&g_sMoldOpenOilCompensate);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_MoldCloseParamPosiCtrlKP description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_MoldCloseParamPosiCtrlKP(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            g_sMoldCloseOilCompensate.backupKp = puDBData->fData;
//            OilCompensate_Run(&g_sMoldCloseOilCompensate);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_SetEjectSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetEjectSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DBIndex_T const atProfileSetIndex[] = {
//        DBINDEX_EJECT0_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_EJECT0_INTERFACE_EJECT_BACK_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_EJECT1_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_EJECT1_INTERFACE_EJECT_BACK_ENABLE_DIGITAL_PROFILE,
//    };

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_EJECT_FORWARD_SEGMENTS_NUMBER], puDBData);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_EjectSetForwardSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_EjectSetForwardSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03 };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_EJECT0_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_EJECT_FORWARD_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_EJECT1_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_EJECT2_FORWARD_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_EjectSetEjectBackSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_EjectSetEjectBackSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03 };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_EJECT0_INTERFACE_EJECT_BACK_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_EJECT_EJECTOR_BACK_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_EJECT1_INTERFACE_EJECT_BACK_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_EJECT2_EJECTOR_BACK_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_SetInjectSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetInjectSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DBIndex_T const atProfileSetIndex[] = {
//        DBINDEX_INJECT0_INTERFACE_FORWARD_INJECT_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_INJECT0_INTERFACE_FORWARD_HOLD_PRESS_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_INJECT1_INTERFACE_FORWARD_INJECT_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_INJECT1_INTERFACE_FORWARD_HOLD_PRESS_ENABLE_DIGITAL_PROFILE,
//    };

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_INJECT_SEGMENTS_NUMBER], puDBData);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_InjectSetForwardSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_InjectSetForwardSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_INJECT0_INTERFACE_FORWARD_INJECT_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_INJECT_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_INJECT1_INTERFACE_FORWARD_INJECT_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_INJECT2_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_InjectSetHoldPressSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_InjectSetHoldPressSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_INJECT0_INTERFACE_FORWARD_HOLD_PRESS_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_HOLD_PRESSURE_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_INJECT1_INTERFACE_FORWARD_HOLD_PRESS_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_HOLD_PRESSURE2_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_SetChargeSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetChargeSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DBIndex_T const atProfileSetIndex[] = {
//        DBINDEX_CHARGE0_INTERFACE_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_CHARGE1_INTERFACE_ENABLE_DIGITAL_PROFILE,

//        DBINDEX_INJECT0_INTERFACE_BACK_PRESS_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_INJECT1_INTERFACE_BACK_PRESS_ENABLE_DIGITAL_PROFILE,
//    };

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_CHARGE_SEGMENTS_NUMBER], puDBData);     // Anders 2015-3-10, Add.
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_CHARGE_SEGMENTS_NUMBER + 2], puDBData); // Anders 2015-3-10, Add.
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_ChargeSetSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_ChargeSetSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F };
//    WORD       wTmp;
//
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_INJECT0_INTERFACE_BACK_PRESS_ENABLE_DIGITAL_PROFILE:
//                case DBINDEX_CHARGE0_INTERFACE_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_CHARGE_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_INJECT1_INTERFACE_BACK_PRESS_ENABLE_DIGITAL_PROFILE:
//                case DBINDEX_CHARGE1_INTERFACE_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_CHARGE2_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_SetCarriageSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetCarriageSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DBIndex_T const atProfileSetIndex[] = {
//        DBINDEX_CARRIAGE0_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_CARRIAGE0_INTERFACE_CARRIAGE_BACK_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_CARRIAGE1_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE,
//        DBINDEX_CARRIAGE1_INTERFACE_CARRIAGE_BACK_ENABLE_DIGITAL_PROFILE,
//    };

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            DB_SetDataBySysIndex(atProfileSetIndex[tDBIndex - DBINDEX_LOCAL0_CARRIAGE_FORWARD_SEGMENTS_NUMBER], puDBData); // Anders 2015-3-10, Add.
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_CarriageSetForwardSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_CarriageSetForwardSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03 };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_CARRIAGE0_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_CARRIAGE_FORWARD_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_CARRIAGE1_INTERFACE_FORWARD_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_CARRIAGE2_FORWARD_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
/**
 * [DB_CarriageSetCarriageBackSegNbr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_CarriageSetCarriageBackSegNbr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    WORD const awProfileTab[] = { 0x00, 0x01, 0x03 };
//    WORD       wTmp;

//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            switch (tDBIndex)
//            {
//                case DBINDEX_CARRIAGE0_INTERFACE_CARRIAGE_BACK_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_CARRIAGE_SUCK_BACK_SEGMENTS_NUMBER;
//                    break;
//                case DBINDEX_CARRIAGE1_INTERFACE_CARRIAGE_BACK_ENABLE_DIGITAL_PROFILE:
//                    wTmp = DBINDEX_LOCAL0_CARRIAGE2_SUCK_BACK_SEGMENTS_NUMBER;
//                    break;
//                default:
//                    DEBUG_ASSERT(false);
//                    return DBRETCODE_SUCCESS;
//            }
//            wTmp = DB_GetWORD(wTmp);
//            DB_SetWORD(tDBIndex, (wTmp < (sizeof(awProfileTab) / sizeof(*awProfileTab))) ? awProfileTab[wTmp] : 0x00);
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_TemperCard_Seg_Absolute_Set description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_Absolute_Set(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD wTemperSegNum;

                wTemperSegNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / TEMPER_ONESEG_DATANBR;
                TemperCommon_ChangeAbsolute_set(wTemperSegNum);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 *
 *
 * @author Anders.zhang (2015/2/13)
 *
 * @param tDBIndex
 * @param eDBEvent
 * @param pvDBData
 *
 * @return E_DBRetCode
 */
E_DBRetCode DB_SetTemper_Preserve(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //WORD wTemperSegNum;

    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            //wTemperSegNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / TEMPER_ONESEG_DATANBR;
            TemperCommon_ChangePreserve();
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 *
 *
 * @author Anders.zhang (2017/4/27)
 *
 * @param tDBIndex
 * @param eDBEvent
 * @param pvDBData
 *
 * @return E_DBRetCode
 */
E_DBRetCode DB_SetTemper_FastMoldOpt(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //WORD wTemperSegNum;

    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                S_DBInflSeg const sInflSeg = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_OUTPUT_CYCLE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    DBINDEX_POWERCARD1_CONFIG_CARD_ID - 1 - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    TEMPER_ONESEG_DATANBR
                };

                DBWriteInflSeg_Arose(&sInflSeg, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
                TemperCommon_ReConfigAllCalCycle();
                if (puDBData->vWORD == 1)
                {
                    CreatTask_Temper_FastMold();
                }
                else
                {
                    CreatTask_Temper_NormalMold();
                }
            }
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}
/**
 * [DB_RapidCompensation description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_RapidCompensation(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    //WORD i;

    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        int i;
        case DBEVENT_WRITE_AFTER:;
            //shanll 20201014 add
            for(i=0;i<TEMPER_MAX_NBR;i++)
            {
                if(puDBData->vWORD == TRUE)
                    TemperCommon_ChangeRapidCompensationUse(i,TRUE);
                else
                    TemperCommon_ChangeRapidCompensationUse(i,FALSE);
            }
            /*先还原回20200609的temperlib
                {
                    S_DBBlock_TEMPER_SEG * temper_seg;
                    temper_seg = &g_sDB.TEMPER1.INTERFACE_SEG1;
                    for (i = 0; i < TEMPER_MAX_NBR; i++)
                    {
                            TemperCommon_Set_MicroAdj(i, puDBData->vWORD & (1 << temper_seg[i].SET_DEVICE_TYPE));
                    }
                }
            */
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}
/**
 * [DB_PID_Change description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_PID_Change(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    // adwPIDChangeBit[tDBIndex-DBINDEX_M28_TEMPERATURE_PID_CHANGE_CONFIG1] = puDBData->vDWORD;
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD    i;
                for(i=0; i<32; i++)
                {
                    if(puDBData->vDWORD&((DWORD)1<<i))
                        TemperCommon_SetPIDChange(i+((tDBIndex-DBINDEX_M28_TEMPERATURE_PID_CHANGE_CONFIG1)<<5), TRUE);
                    else
                        TemperCommon_SetPIDChange(i+((tDBIndex-DBINDEX_M28_TEMPERATURE_PID_CHANGE_CONFIG1)<<5), FALSE);
                }

            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 * [DB_RapidCompensation description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_PipeTemper_DeepLearn(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            if(puDBData->vWORD == TRUE)
                g_sDB.M28.TEMPERATURE.PIPE_DEEP_LEARN = TRUE;
            else
                g_sDB.M28.TEMPERATURE.PIPE_DEEP_LEARN = FALSE;
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 *
 *
 * @author Anders.zhang (2017/4/27)
 *
 * @param tDBIndex
 * @param eDBEvent
 * @param pvDBData
 *
 * @return E_DBRetCode
 */
E_DBRetCode DB_MoldTemper_DeepLearn(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            {
                if(puDBData->vWORD == TRUE)
                    g_sDB.M28.TEMPERATURE.MOLD_DEEP_LEARN = TRUE;
                else
                    g_sDB.M28.TEMPERATURE.MOLD_DEEP_LEARN = FALSE;
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_SetTempHeatChkDegree description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_SetTempHeatChkDegree(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                S_DBInflSeg const sInfl = {
                    DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_CHECK_DEGREE - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    DBINDEX_POWERCARD1_CONFIG_CARD_ID - 1 - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE,
                    TEMPER_ONESEG_DATANBR
                };

                DBWriteInflSeg_Arose(&sInfl, DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}

/**
 * [DB_SetOilSegmentNumber description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_SetOilSegmentNumber(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            OilCompensate_SetOilSegNumber(&g_sMoldOpenOilCompensate, puDBData->vWORD);
//            OilCompensate_SetOilSegNumber(&g_sMoldCloseOilCompensate, puDBData->vWORD);
//            break;
//        default:
//            break;
//    }
//    return DBRETCODE_SUCCESS;
//}

/**
 * [DB_TemperCard_Seg_HeatChkDegree description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_TemperCard_Seg_HeatChkDegree(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD wTmp;

                switch (DB_GetWORD(tDBIndex + ((DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_DEVICE_TYPE - (DBIndex_T)DBINDEX_TEMPER1_INTERFACE_SEG1_SET_HEAT_CHECK_DEGREE)))
                {
                    case TEMPER_DEV_PIPE:
                        wTmp = DB_GetWORD(DBINDEX_M28_TEMPERATURE_PIPE_HEAT_CHECK_DEGREE);
                        break;
                    case TEMPER_DEV_MOLD:
                        wTmp = DB_GetWORD(DBINDEX_M28_TEMPERATURE_MOLD_HEAT_CHECK_DEGREE);
                        break;
                    default:
                        wTmp = DB_GetWORD(tDBIndex);
                        break;
                }
                DB_SetWORD(tDBIndex, wTmp);
            }
            break;
        default:
            break;
    }
    return DBRETCODE_SUCCESS;
}
/**
 * [DB_SetTemperPipeAdjCmd description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_SetTemperPipeAdjCmd(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            //wTemperSegNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / TEMPER_ONESEG_DATANBR;
            if (GetMoldsetState()) // Anders 2015-4-15, Add.
                TemperCommon_ChangePipeAdjCmd();
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}
/**
 *
 *
 * @author Anders.zhang (2015/2/13)
 *
 * @param tDBIndex
 * @param eDBEvent
 * @param pvDBData
 *
 * @return E_DBRetCode
 */
E_DBRetCode DB_SetTemperMoldAdjCmd(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            //wTemperSegNum = (tDBIndex - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE) / TEMPER_ONESEG_DATANBR;
            if (GetMoldsetState()) // Anders 2015-4-15, Add.
                TemperCommon_ChangeMoldAdjCmd();
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}
/**
 * [DB_PwOnPrted description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_PwOnPrted(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            if (!GetMoldsetState())
                Comm_ClrWriteFlag(tDBIndex);
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_WatchType description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_WatchType(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD i;

                for (i = 0; i < TASK_WATCH_SUM; i++)
                {
                    if (tDBIndex == g_asWatch[i].wWatchTypeIndex)
                    {
                        Watch_SetWatchType(&g_asWatch[i], (U_WatchType*)puDBData);
                        break;
                    }
                }
            }
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_WatchWriteValue description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_WatchWriteValue(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD i;

                for (i = 0; i < TASK_WATCH_SUM; i++)
                {
                    if (tDBIndex <= gc_asWatchIndex[i].tLastIndex)
                    {
                        DB_SetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_WRITEVALUE_COUNT, DB_GetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_WRITEVALUE_COUNT) + 1);
                        Watch_WriteIndex(&g_asWatch[i], tDBIndex);
                        break;
                    }
                }
            }
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_WatchReadAddr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_WatchReadAddr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:;
            {
                WORD i;

                for (i = 0; i < TASK_WATCH_SUM; i++)
                {
                    if (tDBIndex <= gc_asWatchIndex[i].tLastIndex)
                    {
                        DB_SetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_READADDR_COUNT, DB_GetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_READADDR_COUNT) + 1);
                        DB_SetDWORD(tDBIndex + 1, 0); // watch value clean.
                        Watch_ReadIndex(&g_asWatch[i], tDBIndex);
                        break;
                    }
                }
            }
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_WatchReadValue description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_WatchReadValue(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:;
            {
                WORD i;

                for (i = 0; i < TASK_WATCH_SUM; i++)
                {
                    if (tDBIndex <= gc_asWatchIndex[i].tLastIndex)
                    {
                        DB_SetDWORD(DBINDEX_M28_OLD_WATCH_READ_READVALUE_COUNT, DB_GetDWORD(DBINDEX_M28_OLD_WATCH_READ_READVALUE_COUNT) + 1);
                        Watch_ReadIndex(&g_asWatch[i], tDBIndex);
                        break;
                    }
                }
            }
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_OldWatchType description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_OldWatchType(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            Watch_SetWatchType(&g_sOldWatch.sWatch, (U_WatchType*)puDBData);
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_OldWatchWriteValue description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_OldWatchWriteValue(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            DB_SetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_WRITEVALUE_COUNT, DB_GetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_WRITEVALUE_COUNT) + 1);
            Watch_WriteIndex(&g_sOldWatch.sWatch, tDBIndex);
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_OldWatchReadAddr description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_OldWatchReadAddr(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            DB_SetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_READADDR_COUNT, DB_GetDWORD(DBINDEX_M28_OLD_WATCH_WRITE_READADDR_COUNT) + 1);
            DB_SetDWORD(tDBIndex + 1, 0); // watch value clean.
            Watch_ReadIndex(&g_sOldWatch.sWatch, tDBIndex);
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_OldWatchReadValue description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  pvDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_OldWatchReadValue(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            DB_SetDWORD(DBINDEX_M28_OLD_WATCH_READ_READVALUE_COUNT, DB_GetDWORD(DBINDEX_M28_OLD_WATCH_READ_READVALUE_COUNT) + 1);
            Watch_ReadIndex(&g_sOldWatch.sWatch, tDBIndex);
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_AxisAccess_WriteParam description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_AxisAccess_WriteParam(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:;
//            {
//                unsigned i;
//                unsigned axisNum;
//                //DWORD curValue;
//                DBIndex_T dbId0Index;

//                axisNum = taskAxisAccess_GetAxis(tDBIndex);
//                //curValue = DB_GetDWORD(tDBIndex);
//                //if (0 != curValue)
//                //  Comm_ClrWriteFlag(tDBIndex);
//                //else
//                {
//                    dbId0Index = taskAxisAccess_GetWriteBaseIndex(axisNum) + (DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_COMM_PARAM);
//                    //DB_SetDWORD(dbId0Index - (DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_STATUS), 0);
//                    //DB_SetDWORD(dbId0Index - (DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_EXEC_STATUS), 0);
//                    for (i = 0; i < 32; i++)
//                        DB_SetDWORD(dbId0Index + i, 0); // clear id and value.
//                }
//                //taskAxisAccess_GetWriteChangeBit(axisNum, 0);
//                //DB_SetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE1, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE1));
//            }
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_AxisAccess_WriteData description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_AxisAccess_WriteData(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            //unsigned axisNum;
            //DWORD curValue;
            //DWORD execStatus;
            //DWORD changeBit;

            //Comm_ClrReadFlag(tDBIndex);
            //axisNum = taskAxisAccess_GetAxis(tDBIndex);
            //curValue = DB_GetDWORD(tDBIndex);
            //DB_GetDataBySysIndex(tDBIndex + (DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_EXEC_STATUS - DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_STATUS), &execStatus);
            //changeBit = taskAxisAccess_GetWriteChangeBit(axisNum, execStatus);
            //curValue &= ~changeBit;
            //curValue |= changeBit & execStatus;
            //DB_SetDWORD(tDBIndex, curValue);
            //DB_SetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE2, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE2));
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_AxisAccess_WriteId description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_AxisAccess_WriteId(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            //unsigned axisNum;
            //DBIndex_T commParamIndex;

            //axisNum = taskAxisAccess_GetAxis(tDBIndex);
            //commParamIndex = taskAxisAccess_GetWriteBaseIndex(axisNum);
            //Comm_SetWriteFlagByDBIndex(commParamIndex);
            //DB_SetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE4, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE4));
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_AxisAccess_WriteValue description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_AxisAccess_WriteValue(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_WRITE_AFTER:
            //unsigned axisNum;
            //DBIndex_T commParamIndex;
            //DBIndex_T dataStatusIndex;
            //DWORD dataState;

            //axisNum = taskAxisAccess_GetAxis(tDBIndex);
            //commParamIndex = taskAxisAccess_GetWriteBaseIndex(axisNum);
            //dataStatusIndex = commParamIndex + (DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_STATUS - DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_COMM_PARAM);
            //dataState = DB_GetDWORD(dataStatusIndex);
            //dataState |= (DWORD)1 << ((tDBIndex - commParamIndex - (DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_WRITE_COMM_PARAM)) >> 1);
            //DB_SetDWORD(dataStatusIndex, dataState);
            //DB_SetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE5, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH0_READ_VALUE5));
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_AxisAccess_ReadParam description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_AxisAccess_ReadParam(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:;
//            {
//                unsigned i;
//                unsigned axisNum;
//                //DWORD curValue;
//                DBIndex_T dbId0Index;

//                axisNum = taskAxisAccess_GetAxis(tDBIndex);
//                //curValue = DB_GetDWORD(tDBIndex);
//                //if (0 != curValue)
//                //  Comm_ClrWriteFlag(tDBIndex);
//                //else
//                {
//                    dbId0Index = taskAxisAccess_GetReadBaseIndex(axisNum) + (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_COMM_PARAM);
//                    //DB_SetDWORD(dbId0Index - (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_STATUS), 0);
//                    //DB_SetDWORD(dbId0Index - (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_EXEC_STATUS), 0);
//                    for (i = 0; i < 32; i++)
//                        DB_SetDWORD(dbId0Index + i, 0); // clear id and value.
//                }
//                //taskAxisAccess_GetReadChangeBit(axisNum, 0);
//                //DB_SetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE1, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE1));
//            }
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_AxisAccess_ReadData description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
E_DBRetCode DB_AxisAccess_ReadData(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
{
    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
    DEBUG_ASSERT(NULL != puDBData);
    switch (eDBEvent)
    {
        case DBEVENT_READ_AFTER:
            //unsigned axisNum;
            //DWORD curValue;
            //DWORD execStatus;
            //DWORD changeBit;

            //Comm_ClrReadFlag(tDBIndex);
            //axisNum = taskAxisAccess_GetAxis(tDBIndex);
            //curValue = DB_GetDWORD(tDBIndex);
            //DB_GetDataBySysIndex(tDBIndex + (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_EXEC_STATUS - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_STATUS), &execStatus);
            //changeBit = taskAxisAccess_GetReadChangeBit(axisNum, execStatus);
            //curValue &= ~changeBit;
            //curValue |= changeBit & execStatus;
            //DB_SetDWORD(tDBIndex, curValue);
            //DB_SetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE2, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE2));
            return DBRETCODE_SUCCESS;
        default:
            return DBRETCODE_SUCCESS;
    }
}

/**
 * [DB_AxisAccess_ReadID description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_AxisAccess_ReadID(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_WRITE_AFTER:
//            //unsigned axisNum;
//            //DBIndex_T commParamIndex;
//            //DBIndex_T dataStatusIndex;
//            //DWORD dataState;

//            //axisNum = taskAxisAccess_GetAxis(tDBIndex);
//            //commParamIndex = taskAxisAccess_GetReadBaseIndex(axisNum);
//            //DEBUG_ASSERT(commParamIndex <= DBINDEX_SUM)
//            ////Comm_SetWriteFlagByDBIndex(commParamIndex);
//            //dataStatusIndex = commParamIndex + (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_STATUS - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_COMM_PARAM);
//            //dataState = DB_GetDWORD(dataStatusIndex);
//            //dataState |= (DWORD)1 << ((tDBIndex - commParamIndex - (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_ID_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_COMM_PARAM)) >> 1);
//            //DB_SetDWORD(dataStatusIndex, dataState);
//            Comm_ClrReadFlag(tDBIndex + (DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_VALUE_0 - DBINDEX_MOLD0_INTERFACE_ACCESS_READ_DATA_ID_0)); // THJ 2016-12-9 Add.
//            //DB_SetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE4, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE4));
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}

/**
 * [DB_AxisAccess_ReadValue description]
 * @param  tDBIndex [description]
 * @param  eDBEvent [description]
 * @param  puDBData [description]
 * @return          [description]
 */
//E_DBRetCode DB_AxisAccess_ReadValue(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const* puDBData)
//{
//    DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
//    DEBUG_ASSERT(NULL != puDBData);
//    switch (eDBEvent)
//    {
//        case DBEVENT_READ_AFTER:
//            //DB_SetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE5, 1 + DB_GetDWORD(DBINDEX_LOCAL0_WATCH1_READ_VALUE5));
//            return DBRETCODE_SUCCESS;
//        default:
//            return DBRETCODE_SUCCESS;
//    }
//}
///**
// * [DB_CompleteAccess_Card_Config description]
// * @param  tDBIndex [description]
// * @param  eDBEvent [description]
// * @param  pvDBData [description]
// * @return          [description]
// */
//E_DBRetCode DB_CompleteAccess_Card_Config(DBIndex_T tDBIndex, E_DBEvent eDBEvent, DBVALUE const *puDBData)
//{
//  DEBUG_ASSERT;;(tDBIndex < DBINDEX_SUM)
//  DEBUG_ASSERT;;(NULL != puDBData)
//  switch (eDBEvent)
//  {
//      case DBEVENT_WRITE_AFTER:
//      {
//          return DBRETCODE_SUCCESS;
//      }
//      default:
//      {
//          return DBRETCODE_SUCCESS;
//      }
//  }
//}

/**
 * [DBWriteInflSeg_Arose description]
 * @param pInflSeg [description]
 * @param tBase    [description]
 */
static void DBWriteInflSeg_Arose(S_DBInflSeg const* pInflSeg, DBIndex_T tBase)
{
    DBIndex_T tDBIndex;
    DBIndex_T tEnd;

    DEBUG_ASSERT(NULL != pInflSeg);
    DEBUG_ASSERT(tBase < DBINDEX_SUM);
    tDBIndex = tBase + pInflSeg->tOffsetStart;
    tEnd     = tBase + pInflSeg->tOffsetEnd;
    while (tDBIndex <= tEnd)
    {
        DB_CallCallBack(tDBIndex, DBEVENT_WRITE_AFTER, DB_GetDataAddr(tDBIndex));
        tDBIndex += pInflSeg->tInc;
    }
}

/**
 * [DBWriteInfl_Arose description]
 * @param pIndex [description]
 * @param iTabs  [description]
 * @param tBase  [description]
 */
static void DBWriteInfl_Arose(DBIndex_T const* pIndex, unsigned iTabs, DBIndex_T tBase)
{
    DBIndex_T tDBIndex;
    unsigned  i;

    DEBUG_ASSERT((NULL != pIndex) ? true : (0 == iTabs));
    DEBUG_ASSERT(tBase < DBINDEX_SUM);
    for (i = 0; i < iTabs; ++i)
    {
        tDBIndex = tBase + pIndex[i];
        DB_CallCallBack(tDBIndex, DBEVENT_WRITE_AFTER, DB_GetDataAddr(tDBIndex));
    }
}
