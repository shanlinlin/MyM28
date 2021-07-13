/*==============================================================================+
|  Function : CommComp Factory                                                  |
|  Task     : CommComp Factory Source File                                      |
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
#include "../include/CommComp_Factory.h"
#include "../include/CommComp_CANOpenMaster.h"
#include "../include/CommComp_EtherCat.h"
#include "../include/CommComp_Local.h"
#include "Error_App.h"
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
static DBIndex_T      Comm_UpdateDBIndex(CommSegInfo_S* psSegInfo, const DWORD* padwFlagData, DBIndex_T tDBIndex);
inline void           AddComp(E_CommType eCommType, CommComp_S* psComp);
inline void           DelComp(CommComp_S* psComp);
inline CommSubComp_S* GetSubCompByDBIndex(DBIndex_T tDBIndex);
// static CommSubComp_S * GetSubCompByWriteDBIndex(E_CommType eCommType, DBIndex_T tDBIndex);
// static CommSubComp_S * GetSubCompByReadDBIndex(E_CommType eCommType, DBIndex_T tDBIndex);
static void          SCommSegInfo_SetFlag(CommSegInfo_S* psSegInfo);
inline void          SCommSegInfo_ClrFlag(CommSegInfo_S* psSegInfo);
inline void          SetDBFlag(CommSegInfo_S const* psSegInfo, DWORD* padwFlag);
inline void          ClrDBFlag(CommSegInfo_S const* psSegInfo, DWORD* padwFlag);
inline void          FlushByComp(CommComp_S const* const psComp);
inline E_CommRetCode Default_fCreate(E_CommType eCommType);
inline E_CommRetCode Default_fDestory(E_CommType eCommType);
inline void          Default_fParseMsg(E_CommType eCommType, CommComp_S* psCompHead);
// inline void            Default_fCtrlRun(E_CommType eCommType, CommComp_S *psCommComp);
inline void Default_EventCallBack(struct CommComp_S* psComp, union CommEvent_U* puEvent);
inline void Default_CustomFunc(CommComp_S* psComp, E_CustomFuncType eType);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
// CommComp_S                                           *u_apsCommCompHead[COMMTYPE_SUM];

// DWORD                                                u_adwDBReadFlag[CANOPENMASTERCOMM_DBFLAG_SIZE];
// DWORD                                                u_adwDBWriteFlag[CANOPENMASTERCOMM_DBFLAG_SIZE];

//extern CommTemperCardInfo_S                         u_asTemperCardInfo[TEMPERCARDNUM_SUM];
//extern CommPwmtrCardInfo_S                          u_asPwmtrCardInfo[PWMTRCARDNUM_SUM];
//extern S_AxisCardAxis                               u_asCardInfo[AXISCARD_SUM];
//extern S_CommRemoteIOCardInfo                       u_asRemoteIOCardInfo[REMOTEIONUM_SUM];  // THJ 2015-06-26 Add:
//extern S_AxisCardAxis                               u_sLocalCardInfo;
//extern S_AxisCardAxis                               u_asAxisCardAxis[AXISCARDAXIS_SUM];
// struct
// {
//  DBIndex_T                                       tSegStartIndex;
//  DBIndex_T                                       tSegEndIndex;
//  CommSubComp_S                                   *psSubComp;
// }const                                               uc_asInfoTab[] =
// {//  {tSegStartIndex                                     ,tSegEndIndex                                               ,psSubComp                                                  }
//  {CARD1_CONFIG_CARD_ID                               ,CARD1_INTERFACE_SOFTWARE_SN                                ,&u_asCardInfo[AXISCARDCARD_NO1].PRI_sSubComp            },
//  {CARD2_CONFIG_CARD_ID                               ,CARD2_INTERFACE_SOFTWARE_SN                                ,&u_asCardInfo[AXISCARDCARD_NO2].PRI_sSubComp            },
//  {CARD3_CONFIG_CARD_ID                               ,CARD3_INTERFACE_SOFTWARE_SN                                ,&u_asCardInfo[AXISCARDCARD_NO3].PRI_sSubComp            },
//  {CARD4_CONFIG_CARD_ID                               ,CARD4_INTERFACE_SOFTWARE_SN                                ,&u_asCardInfo[AXISCARDCARD_NO4].PRI_sSubComp            },
//  {CARD5_CONFIG_CARD_ID                               ,CARD5_INTERFACE_SOFTWARE_SN                                ,&u_asCardInfo[AXISCARDCARD_NO5].PRI_sSubComp            },
//  {CARD_LOCAL_CONFIG_CARD_ID                          ,CARD_LOCAL_INTERFACE_SOFTWARE_SN                           ,&u_sLocalCardInfo.PRI_sSubComp                             },
//  {TEMPERATURE_CARD1_CONFIG_CARD_ID                   ,TEMPERATURE_CARD1_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO1].PRI_sSubComp        },
//  {TEMPERATURE_CARD2_CONFIG_CARD_ID                   ,TEMPERATURE_CARD2_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO2].PRI_sSubComp        },
//  {TEMPERATURE_CARD3_CONFIG_CARD_ID                   ,TEMPERATURE_CARD3_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO3].PRI_sSubComp        },
//  {TEMPERATURE_CARD4_CONFIG_CARD_ID                   ,TEMPERATURE_CARD4_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO4].PRI_sSubComp        },
//  {TEMPERATURE_CARD5_CONFIG_CARD_ID                   ,TEMPERATURE_CARD5_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO5].PRI_sSubComp        },
//  {TEMPERATURE_CARD6_CONFIG_CARD_ID                   ,TEMPERATURE_CARD6_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO6].PRI_sSubComp        },
//  {TEMPERATURE_CARD7_CONFIG_CARD_ID                   ,TEMPERATURE_CARD7_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO7].PRI_sSubComp        },
//  {TEMPERATURE_CARD8_CONFIG_CARD_ID                   ,TEMPERATURE_CARD8_INTERFACE_SOFTWARE_SN                    ,&u_asTemperCardInfo[TEMPERCARDNUM_NO8].PRI_sSubComp        },
//  {MOLD_CONFIG1_SYSTEM_CYCLE_TIME                     ,MOLD_INTERFACE_MONITOR_MAX_OPEN_POSITION                   ,&u_asAxisCardAxis[AXISCARDAXIS_MOLD].PRI_sSubComp          },
//  {EJECT_CONFIG1_SYSTEM_CYCLE_TIME                    ,EJECT_INTERFACE_EJECT_BACK_PARAM_CLOSE_MOLD_END_IO         ,&u_asAxisCardAxis[AXISCARDAXIS_EJECT].PRI_sSubComp         },
//  {INJECT_CONFIG1_SYSTEM_CYCLE_TIME                   ,INJECT_INTERFACE_MONITOR_MIN_POSITION                      ,&u_asAxisCardAxis[AXISCARDAXIS_INJECT].PRI_sSubComp        },
//  {CHARGE_CONFIG1_SYSTEM_CYCLE_TIME                   ,CHARGE_INTERFACE_PARAM_DIRECTION_VALVE_IO                  ,&u_asAxisCardAxis[AXISCARDAXIS_CHARGE].PRI_sSubComp        },
//  {CARRIAGE_CONFIG1_SYSTEM_CYCLE_TIME                 ,CARRIAGE_INTERFACE_CARRIAGE_BACK_PARAM_BACK_END_IO         ,&u_asAxisCardAxis[AXISCARDAXIS_CARRIAGE].PRI_sSubComp      },
//  {MOLD2_CONFIG1_SYSTEM_CYCLE_TIME                    ,MOLD2_INTERFACE_MONITOR_MAX_OPEN_POSITION                  ,&u_asAxisCardAxis[AXISCARDAXIS_MOLD2].PRI_sSubComp         },
//  {EJECT2_CONFIG1_SYSTEM_CYCLE_TIME                   ,EJECT2_INTERFACE_EJECT_BACK_PARAM_CLOSE_MOLD_END_IO        ,&u_asAxisCardAxis[AXISCARDAXIS_EJECT2].PRI_sSubComp        },
//  {INJECT2_CONFIG1_SYSTEM_CYCLE_TIME                  ,INJECT2_INTERFACE_MONITOR_MIN_POSITION                     ,&u_asAxisCardAxis[AXISCARDAXIS_INJECT2].PRI_sSubComp       },
//  {CHARGE2_CONFIG1_SYSTEM_CYCLE_TIME                  ,CHARGE2_INTERFACE_PARAM_DIRECTION_VALVE_IO                 ,&u_asAxisCardAxis[AXISCARDAXIS_CHARGE2].PRI_sSubComp       },
//  {CARRIAGE2_CONFIG1_SYSTEM_CYCLE_TIME                ,CARRIAGE2_INTERFACE_CARRIAGE_BACK_PARAM_BACK_END_IO        ,&u_asAxisCardAxis[AXISCARDAXIS_CARRIAGE2].PRI_sSubComp     },
//  {TEMPERCARD1_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD1_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO1].PRI_sSubComp        },
//  {TEMPERCARD2_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD2_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO2].PRI_sSubComp        },
//  {TEMPERCARD3_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD3_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO3].PRI_sSubComp        },
//  {TEMPERCARD4_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD4_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO4].PRI_sSubComp        },
//  {TEMPERCARD5_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD5_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO5].PRI_sSubComp        },
//  {TEMPERCARD6_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD6_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO6].PRI_sSubComp        },
//  {TEMPERCARD7_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD7_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO7].PRI_sSubComp        },
//  {TEMPERCARD8_INTERFACE_SEG1_SET_SENSOR_TYPE         ,TEMPERCARD8_INTERFACE_SEG12_REAL_ERROR                     ,&u_asTemperCardInfo[TEMPERCARDNUM_NO8].PRI_sSubComp        },
//  {POWER_METER_CARD1_CONFIG_CARD_ID                   ,POWER_METER_CARD1_INTERFACE_IRMS_RESET                     ,&u_asPwmtrCardInfo[PWMTRCARDNUM_NO1].PRI_sSubComp          },
//  {POWER_METER_CARD2_CONFIG_CARD_ID                   ,POWER_METER_CARD2_INTERFACE_IRMS_RESET                     ,&u_asPwmtrCardInfo[PWMTRCARDNUM_NO2].PRI_sSubComp          },
//  {POWER_METER_CARD3_CONFIG_CARD_ID                   ,POWER_METER_CARD3_INTERFACE_IRMS_RESET                     ,&u_asPwmtrCardInfo[PWMTRCARDNUM_NO3].PRI_sSubComp          },
// };
//CommSubComp_S *const                                uc_asIndexOfSubCompTab[] =
//{
//    NULL,                                                       /*{0                                                ,MASTER_STATE_ERROR1                                    },  // 0x00 - 0 */
//    &u_asCardInfo[AXISCARD_NO0]._sSubComp,                      /*{CARD1_CONFIG_CARD_ID                             ,CARD1_INTERFACE_SOFTWARE_SN                            },  // 0x01 - 1 */
//    &u_asCardInfo[AXISCARD_NO1]._sSubComp,                      /*{CARD2_CONFIG_CARD_ID                             ,CARD2_INTERFACE_SOFTWARE_SN                            },  // 0x02 - 2 */
//    &u_asCardInfo[AXISCARD_NO2]._sSubComp,                      /*{CARD3_CONFIG_CARD_ID                             ,CARD3_INTERFACE_SOFTWARE_SN                            },  // 0x03 - 3 */
//    &u_asCardInfo[AXISCARD_NO3]._sSubComp,                      /*{CARD4_CONFIG_CARD_ID                             ,CARD4_INTERFACE_SOFTWARE_SN                            },  // 0x04 - 4 */
//    &u_asCardInfo[AXISCARD_NO4]._sSubComp,                      /*{CARD5_CONFIG_CARD_ID                             ,CARD5_INTERFACE_SOFTWARE_SN                            },  // 0x05 - 5 */
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x06 - 6 */
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x07 - 7 */
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x08 - 8 */
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x09 - 9 */
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x0A - 10*/
//    &u_sLocalCardInfo._sSubComp,                                /*{CARD_LOCAL_CONFIG_CARD_ID                        ,CARD_LOCAL_INTERFACE_SOFTWARE_SN                       },  // 0x0B - 11*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x0C - 12*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x0D - 13*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x0E - 14*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x0F - 15*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO1].PRI_sSubComp,        /*{TEMPERATURE_CARD1_CONFIG_CARD_ID                 ,TEMPERATURE_CARD1_INTERFACE_SOFTWARE_SN                },  // 0x10 - 16*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO2].PRI_sSubComp,        /*{TEMPERATURE_CARD2_CONFIG_CARD_ID                 ,TEMPERATURE_CARD2_INTERFACE_SOFTWARE_SN                },  // 0x11 - 17*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO3].PRI_sSubComp,        /*{TEMPERATURE_CARD3_CONFIG_CARD_ID                 ,TEMPERATURE_CARD3_INTERFACE_SOFTWARE_SN                },  // 0x12 - 18*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO4].PRI_sSubComp,        /*{TEMPERATURE_CARD4_CONFIG_CARD_ID                 ,TEMPERATURE_CARD4_INTERFACE_SOFTWARE_SN                },  // 0x13 - 19*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO5].PRI_sSubComp,        /*{TEMPERATURE_CARD5_CONFIG_CARD_ID                 ,TEMPERATURE_CARD5_INTERFACE_SOFTWARE_SN                },  // 0x14 - 20*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO6].PRI_sSubComp,        /*{TEMPERATURE_CARD6_CONFIG_CARD_ID                 ,TEMPERATURE_CARD6_INTERFACE_SOFTWARE_SN                },  // 0x15 - 21*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO7].PRI_sSubComp,        /*{TEMPERATURE_CARD7_CONFIG_CARD_ID                 ,TEMPERATURE_CARD7_INTERFACE_SOFTWARE_SN                },  // 0x16 - 22*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO8].PRI_sSubComp,        /*{TEMPERATURE_CARD8_CONFIG_CARD_ID                 ,TEMPERATURE_CARD8_INTERFACE_SOFTWARE_SN                },  // 0x17 - 23*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x18 - 24*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x19 - 25*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x1A - 26*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x1B - 27*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x1C - 28*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x1D - 29*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x1E - 30*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x1F - 31*/
//    &u_asAxisCardAxis[AXISCARDAXIS_MOLD0]._sSubComp,            /*{MOLD_CONFIG1_SYSTEM_CYCLE_TIME                   ,MOLD_INTERFACE_MONITOR_MAX_OPEN_POSITION               },  // 0x20 - 32*/
//    &u_asAxisCardAxis[AXISCARDAXIS_EJECT0]._sSubComp,           /*{EJECT_CONFIG1_SYSTEM_CYCLE_TIME                  ,EJECT_INTERFACE_EJECT_BACK_PARAM_CLOSE_MOLD_END_IO     },  // 0x21 - 33*/
//    &u_asAxisCardAxis[AXISCARDAXIS_INJECT0]._sSubComp,          /*{INJECT_CONFIG1_SYSTEM_CYCLE_TIME                 ,INJECT_INTERFACE_MONITOR_MIN_POSITION                  },  // 0x22 - 34*/
//    &u_asAxisCardAxis[AXISCARDAXIS_CHARGE0]._sSubComp,          /*{CHARGE_CONFIG1_SYSTEM_CYCLE_TIME                 ,CHARGE_INTERFACE_PARAM_DIRECTION_VALVE_IO              },  // 0x23 - 35*/
//    &u_asAxisCardAxis[AXISCARDAXIS_CARRIAGE0]._sSubComp,        /*{CARRIAGE_CONFIG1_SYSTEM_CYCLE_TIME               ,CARRIAGE_INTERFACE_CARRIAGE_BACK_PARAM_BACK_END_IO     },  // 0x24 - 36*/
//    &u_asAxisCardAxis[AXISCARDAXIS_MOLD1]._sSubComp,            /*{MOLD2_CONFIG1_SYSTEM_CYCLE_TIME                  ,MOLD2_INTERFACE_MONITOR_MAX_OPEN_POSITION              },  // 0x25 - 37*/
//    &u_asAxisCardAxis[AXISCARDAXIS_EJECT1]._sSubComp,           /*{EJECT2_CONFIG1_SYSTEM_CYCLE_TIME                 ,EJECT2_INTERFACE_EJECT_BACK_PARAM_CLOSE_MOLD_END_IO    },  // 0x26 - 38*/
//    &u_asAxisCardAxis[AXISCARDAXIS_INJECT1]._sSubComp,          /*{INJECT2_CONFIG1_SYSTEM_CYCLE_TIME                ,INJECT2_INTERFACE_MONITOR_MIN_POSITION                 },  // 0x27 - 39*/
//    &u_asAxisCardAxis[AXISCARDAXIS_CHARGE1]._sSubComp,          /*{CHARGE2_CONFIG1_SYSTEM_CYCLE_TIME                ,CHARGE2_INTERFACE_PARAM_DIRECTION_VALVE_IO             },  // 0x28 - 40*/
//    &u_asAxisCardAxis[AXISCARDAXIS_CARRIAGE1]._sSubComp,        /*{CARRIAGE2_CONFIG1_SYSTEM_CYCLE_TIME              ,CARRIAGE2_INTERFACE_CARRIAGE_BACK_PARAM_BACK_END_IO    },  // 0x29 - 41*/
//    &u_asAxisCardAxis[AXISCARDAXIS_INJECTVALVE0]._sSubComp,     /*{1                                                ,0                                                      },  // 0x2A - 42*/
//    &u_asAxisCardAxis[AXISCARDAXIS_INJECTVALVE1]._sSubComp,     /*{1                                                ,0                                                      },  // 0x2B - 43*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x2C - 44*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x2D - 45*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x2E - 46*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x2F - 47*/
//    NULL,                                                       /*{HYB_INTERFACE_CONTROL_NUMBER_OF_ENTRIES          ,HYB_INTERFACE_CONTROL_VELOCITY                         },  // 0x30 - 48*/
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO1]._sSubComp,           /*{1                                                ,0                                                      },  // 0x31 - 49*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO1].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO2]._sSubComp,           /*{1                                                ,0                                                      },  // 0x32 - 50*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO2].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO3]._sSubComp,           /*{1                                                ,0                                                      },  // 0x33 - 51*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO3].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO4]._sSubComp,           /*{1                                                ,0                                                      },  // 0x34 - 52*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO4].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO5]._sSubComp,           /*{1                                                ,0                                                      },  // 0x35 - 53*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO5].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO6]._sSubComp,           /*{1                                                ,0                                                      },  // 0x36 - 54*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO6].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO7]._sSubComp,           /*{1                                                ,0                                                      },  // 0x37 - 55*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO7].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO8]._sSubComp,           /*{1                                                ,0                                                      },  // 0x38 - 56*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO8].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO9]._sSubComp,           /*{1                                                ,0                                                      },  // 0x39 - 57*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO9].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO10]._sSubComp,          /*{1                                                ,0                                                      },  // 0x3A - 58*/  // THJ 2015-06-26 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO10].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO11]._sSubComp,          /*{1                                                ,0                                                      },  // 0x3B - 59*/  // THJ 2015-06-29 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO11].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO12]._sSubComp,          /*{1                                                ,0                                                      },  // 0x3C - 60*/  // THJ 2015-06-29 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO12].PRI_sSubComp"
//    &u_asRemoteIOCardInfo[REMOTEIONUM_NO13]._sSubComp,          /*{1                                                ,0                                                      },  // 0x3D - 61*/  // THJ 2015-06-29 Modify: "NULL" -> "&u_asRemoteIOCardInfo[REMOTEIONUM_NO13].PRI_sSubComp"
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x3E - 62*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x3F - 63*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO1].PRI_sSubComp,        /*{TEMPERCARD1_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD1_INTERFACE_SEG12_REAL_ERROR                 },  // 0x40 - 64*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO2].PRI_sSubComp,        /*{TEMPERCARD2_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD2_INTERFACE_SEG12_REAL_ERROR                 },  // 0x41 - 65*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO3].PRI_sSubComp,        /*{TEMPERCARD3_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD3_INTERFACE_SEG12_REAL_ERROR                 },  // 0x42 - 66*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO4].PRI_sSubComp,        /*{TEMPERCARD4_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD4_INTERFACE_SEG12_REAL_ERROR                 },  // 0x43 - 67*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO5].PRI_sSubComp,        /*{TEMPERCARD5_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD5_INTERFACE_SEG12_REAL_ERROR                 },  // 0x44 - 68*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO6].PRI_sSubComp,        /*{TEMPERCARD6_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD6_INTERFACE_SEG12_REAL_ERROR                 },  // 0x45 - 69*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO7].PRI_sSubComp,        /*{TEMPERCARD7_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD7_INTERFACE_SEG12_REAL_ERROR                 },  // 0x46 - 70*/
//    &u_asTemperCardInfo[TEMPERCARDNUM_NO8].PRI_sSubComp,        /*{TEMPERCARD8_INTERFACE_SEG1_SET_SENSOR_TYPE       ,TEMPERCARD8_INTERFACE_SEG12_REAL_ERROR                 },  // 0x47 - 71*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x48 - 72*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x49 - 73*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x4A - 74*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x4B - 75*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x4C - 76*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x4D - 77*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x4E - 78*/
//    NULL,                                                       /*{1                                                ,0                                                      },  // 0x4F - 79*/
//    &u_asPwmtrCardInfo[PWMTRCARDNUM_NO1].PRI_sSubComp,          /*{POWER_METER_CARD1_CONFIG_CARD_ID                 ,POWER_METER_CARD1_INTERFACE_IRMS_RESET                 },  // 0x50 - 80*/
//    &u_asPwmtrCardInfo[PWMTRCARDNUM_NO2].PRI_sSubComp,          /*{POWER_METER_CARD2_CONFIG_CARD_ID                 ,POWER_METER_CARD2_INTERFACE_IRMS_RESET                 },  // 0x51 - 81*/
//    &u_asPwmtrCardInfo[PWMTRCARDNUM_NO3].PRI_sSubComp,          /*{POWER_METER_CARD3_CONFIG_CARD_ID                 ,POWER_METER_CARD3_INTERFACE_IRMS_RESET                 },  // 0x52 - 82*/
//};

Info_CommComp_S        g_sCommComp;
Const_CommComp_S const gc_sCommComp = {
	{
	    //{ fCreate,                    fDestory,                   fParseMsg,                  fCtrlRun                    }
	    //{ Default_fCreate, Default_fDestory, Default_fParseMsg, Run_Local },          // local
	    { Create_CommCANOpen, Default_fDestory, Parse_CommCANOpen, Run_CommCANOpen }, // can1
	    { Create_CommCANOpen, Default_fDestory, Parse_CommCANOpen, Run_CommCANOpen }, // can2
	    { Default_fCreate, Default_fDestory, Default_fParseMsg, Run_EtherCat },       // ethercat
	},                                                                                //sFunc
};
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_Comm description]
 */
void Init_Comm(void)
{
#if (DEBUG)
	{
		unsigned int i;

		for (i = 0; i < COMMTYPE_SUM; ++i)
		{
			if ((NULL == gc_sCommComp.asFunc[i].fCreate)
			    || (NULL == gc_sCommComp.asFunc[i].fCtrlRun)
			    || (NULL == gc_sCommComp.asFunc[i].fDestory)
			    || (NULL == gc_sCommComp.asFunc[i].fParseMsg))
			{
				while (1)
					ESTOP0;
			}
		}
		// if ((sizeof(g_sCommComp.apsSubCompTab) / sizeof(*g_sCommComp.apsSubCompTab)) <= (DB_IndexToID(DBINDEX_MAX) >> 24))
		// {
		//  while (1)
		//      ESTOP0;
		// }
	}
#endif

	memset(g_sCommComp.apsCommCompHead, 0, sizeof(g_sCommComp.apsCommCompHead));
	memset(g_sCommComp.adwReadFlag, 0, sizeof(g_sCommComp.adwReadFlag));
	memset(g_sCommComp.adwWriteFlag, 0, sizeof(g_sCommComp.adwWriteFlag));
	memset(g_sCommComp.apsSubCompTab, 0, sizeof(g_sCommComp.apsSubCompTab));
}
/**
 * [Create_Comm description]
 * @param  eCommType [description]
 * @return           [description]
 */
E_CommRetCode Create_Comm(E_CommType eCommType)
{
	DEBUG_ASSERT(eCommType < COMMTYPE_SUM);
	DEBUG_ASSERT(NULL != gc_sCommComp.asFunc[eCommType].fCreate);
	g_sCommComp.apsCommCompHead[eCommType] = NULL;
	return gc_sCommComp.asFunc[eCommType].fCreate(eCommType);
}
/**
 * [Parse_Comm description]
 * @param eCommType [description]
 */
void Parse_Comm(E_CommType eCommType)
{
	DEBUG_ASSERT(eCommType < COMMTYPE_SUM);
	DEBUG_ASSERT(NULL != gc_sCommComp.asFunc[eCommType].fParseMsg);
	gc_sCommComp.asFunc[eCommType]
	    .fParseMsg(eCommType, g_sCommComp.apsCommCompHead[eCommType]);
}
/**
 * [Run_SlowRun description]
 * @param eCommType [description]
 */
void Run_SlowRun(E_CommType eCommType)
{
	CommComp_S*    psCurComp;
	CommSubComp_S* pCurSubComp;

	DEBUG_ASSERT(eCommType < COMMTYPE_SUM);
	psCurComp = g_sCommComp.apsCommCompHead[eCommType];
	while (NULL != psCurComp)
	{
		DEBUG_ASSERT(eCommType == CommComp_GetCommType(psCurComp));
		if (NULL != psCurComp->PRI_psCurSubComp)
		{
			DEBUG_ASSERT(NULL != gc_sCommComp.asFunc[eCommType].fCtrlRun);
			gc_sCommComp.asFunc[eCommType]
			    .fCtrlRun(eCommType, psCurComp);
			DEBUG_ASSERT(NULL != psCurComp->PRO_fCustomFunc);
			psCurComp->PRO_fCustomFunc(psCurComp, CUSTOMFUNCTYPE_SLOW);
			pCurSubComp = psCurComp->PRI_psCurSubComp;
			if (COMMSDOSTATE_FINISH == psCurComp->PRO_bSDOState)
			{
				psCurComp->PRI_tCurIndex++;
				psCurComp->PRO_bSDOState = COMMSDOSTATE_IDLE;
			}
			while (COMMSDOSTATE_IDLE == psCurComp->PRO_bSDOState)
			{
				CommSegInfo_S* psSegInfo;

				if (NULL == psCurComp->PRI_psCurSubComp)
				{
					psCurComp->PRI_psCurSubComp = psCurComp->PRI_psSubCompHead;
					if (pCurSubComp == psCurComp->PRI_psCurSubComp)
						break;
				}
				if (pCurSubComp == psCurComp->PRI_psCurSubComp->PRI_psNextSubComp)
					break;
				switch (psCurComp->PRO_bState)
				{
					case COMPSTATE_SYNC:
					{
						DEBUG_ASSERT(NULL != psCurComp->PRI_psCurSubComp);
						psSegInfo = &psCurComp->PRI_psCurSubComp->PRO_sSDO_SyncInfo;
						if (psSegInfo->bFlag) // Sync write
						{
							psCurComp->PRI_tCurIndex = Comm_UpdateDBIndex(psSegInfo, g_sCommComp.adwWriteFlag, psCurComp->PRI_tCurIndex);
							if (psCurComp->PRI_tCurIndex >= DBINDEX_SUM)
							{
								if (psSegInfo->bCFlag)
									psSegInfo->bFlag = FALSE;
								else
									psSegInfo->bCFlag = TRUE;
								psCurComp->PRI_psCurSubComp = psCurComp->PRI_psCurSubComp->PRI_psNextSubComp;
							}
							else if (Comm_GetWriteChanged(psCurComp->PRI_tCurIndex))
							{
								Comm_ClrWriteFlag(psCurComp->PRI_tCurIndex);
								switch (DB_GetAccess(psCurComp->PRI_tCurIndex))
								{
									case DATAACCESS_WO:
									case DATAACCESS_RW:
									{
										psCurComp->PRI_bSyncStep = COMMSYNCSTEP_SCAN;
										psCurComp->PRO_bSDOState = COMMSDOSTATE_WRITE;
										break;
									}
									default:
										break;
								}
							}
						}
						else
						{
							psCurComp->PRI_psCurSubComp = psCurComp->PRI_psCurSubComp->PRI_psNextSubComp;
							if (NULL == psCurComp->PRI_psCurSubComp)
							{
								switch (psCurComp->PRI_bSyncStep)
								{
									case COMMSYNCSTEP_SCAN:
									{
										psCurComp->PRI_bSyncStep = COMMSYNCSTEP_CHECK;
										break;
									}
									case COMMSYNCSTEP_CHECK:
									{
										psCurComp->PRI_bSyncStep = COMMSYNCSTEP_SUM;
										SCommComp_SetState(psCurComp, COMPSTATE_NORMAL);
										break;
									}
									default:
									{
										DEBUG_ASSERT(false);
										break;
									}
								}
							}
						}
						break;
					}
					case COMPSTATE_NORMAL:
					{
						psSegInfo = &psCurComp->PRI_psCurSubComp->PRO_sSDO_WriteInfo;
						if (psSegInfo->bFlag) // SDO write
						{
							psCurComp->PRI_tCurIndex = Comm_UpdateDBIndex(psSegInfo, g_sCommComp.adwWriteFlag, psCurComp->PRI_tCurIndex);
							if (psCurComp->PRI_tCurIndex >= DBINDEX_SUM)
							{
								if (psSegInfo->bCFlag)
									psSegInfo->bFlag = FALSE;
								else
									psSegInfo->bCFlag = TRUE;
								psCurComp->PRI_psCurSubComp = psCurComp->PRI_psCurSubComp->PRI_psNextSubComp;
							}
							else if (Comm_GetWriteChanged(psCurComp->PRI_tCurIndex))
							{
								Comm_ClrWriteFlag(psCurComp->PRI_tCurIndex);
								switch (DB_GetAccess(psCurComp->PRI_tCurIndex))
								{
									case DATAACCESS_WO:
									case DATAACCESS_RW:
									{
										psCurComp->PRO_bSDOState = COMMSDOSTATE_WRITE;
										break;
									}
									default:
										break;
								}
							}
						}
						else
						{
							psSegInfo = &psCurComp->PRI_psCurSubComp->PRO_sSDO_ReadInfo;
							if (psSegInfo->bFlag) // SDO read
							{
								psCurComp->PRI_tCurIndex = Comm_UpdateDBIndex(psSegInfo, g_sCommComp.adwReadFlag, psCurComp->PRI_tCurIndex);
								if (psCurComp->PRI_tCurIndex >= DBINDEX_SUM)
								{
									if (psSegInfo->bCFlag)
										psSegInfo->bFlag = FALSE;
									else
										psSegInfo->bCFlag = TRUE;
									psCurComp->PRI_psCurSubComp = psCurComp->PRI_psCurSubComp->PRI_psNextSubComp;
								}
								else if (Comm_GetReadChanged(psCurComp->PRI_tCurIndex))
								{
									Comm_ClrReadFlag(psCurComp->PRI_tCurIndex);
									switch (DB_GetAccess(psCurComp->PRI_tCurIndex))
									{
										case DATAACCESS_RO:
										case DATAACCESS_RW:
										{
											psCurComp->PRO_bSDOState = COMMSDOSTATE_READ;
											break;
										}
										default:
											break;
									}
								}
							}
							else
								psCurComp->PRI_psCurSubComp = psCurComp->PRI_psCurSubComp->PRI_psNextSubComp;
						}
						break;
					}
					default:
					{
						pCurSubComp = psCurComp->PRI_psCurSubComp->PRI_psNextSubComp; // for break;
						break;
					}
				}
			}
		}
		else
			psCurComp->PRI_psCurSubComp = psCurComp->PRI_psSubCompHead;
		psCurComp = psCurComp->PRI_psNextComp;
	}
}
/**
 * [Run_FastRun description]
 * @param eCommType [description]
 */
void Run_FastRun(E_CommType eCommType)
{
	CommComp_S* psCurComp;

	DEBUG_ASSERT(eCommType < COMMTYPE_SUM);
	psCurComp = g_sCommComp.apsCommCompHead[eCommType];
	while (NULL != psCurComp)
	{
		DEBUG_ASSERT(eCommType == CommComp_GetCommType(psCurComp));
		DEBUG_ASSERT(NULL != psCurComp->PRO_fCustomFunc);
		psCurComp->PRO_fCustomFunc(psCurComp, CUSTOMFUNCTYPE_FAST);
		psCurComp = psCurComp->PRI_psNextComp;
	}
}
/**
 * [Comm_GetSubCompByDBIndex description]
 * @param  tDBIndex [description]
 * @return          [description]
 */
inline CommSubComp_S* GetSubCompByDBIndex(DBIndex_T tDBIndex)
{
	// unsigned int uiIndex, uiIndexAbove, uiIndexBehind;

	// uiIndexAbove = 0;
	// uiIndexBehind = (sizeof(uc_asInfoTab) / sizeof(*uc_asInfoTab));
	// while (uiIndexAbove < uiIndexBehind)
	// {
	//  uiIndex = (uiIndexAbove + uiIndexBehind) >> 1;
	//  if (tDBIndex >= uc_asInfoTab[uiIndex].tSegStartIndex)
	//  {
	//      if (tDBIndex <= uc_asInfoTab[uiIndex].tSegEndIndex)
	//          return uc_asInfoTab[uiIndex].psSubComp;
	//      else
	//          uiIndexAbove = uiIndex + 1;
	//  }
	//  else
	//      uiIndexBehind = uiIndex;
	// }
	if (tDBIndex >= DBINDEX_SUM)
		return NULL;
	tDBIndex = DB_IndexToID(tDBIndex) >> 24;
	if (tDBIndex < (sizeof(g_sCommComp.apsSubCompTab) / sizeof(*g_sCommComp.apsSubCompTab)))
		return g_sCommComp.apsSubCompTab[tDBIndex];
	return NULL;
}
/**
 * [Comm_SetWriteFlagByDBIndex description]
 * @param tDBIndex [description]
 */
void Comm_SetWriteFlagByDBIndex(DBIndex_T tDBIndex)
{
	CommSubComp_S* psSubComp;
	// E_CommType eCommType;
	// WORD wCommTypeFindInfo;
	CommSegInfo_S* psSegInfo;
	WORD           i, wSum;

	// THJ 2016-05-30 Add.
	if (DATAACCESS_RO == DB_GetAccess(tDBIndex))
		return;
	// DEBUG_ASSERT(tDBIndex < DBINDEX_SUM)
	// if (((tDBIndex >= TEMPERATURE_CARD1_CONFIG_CARD_ID) && (tDBIndex <= TEMPERATURE_CARD8_INTERFACE_SOFTWARE_SN))
	//  || ((tDBIndex >= TEMPERCARD1_INTERFACE_SEG1_SET_SENSOR_TYPE) && (tDBIndex <= TEMPERCARD8_INTERFACE_SEG12_REAL_ERROR)))
	//  wCommTypeFindInfo = (((WORD)1) << TEMPERCARDNUM_DEFAULT_COMMTYPE);
	// else
	//  wCommTypeFindInfo = (((WORD)1) << COMMTYPE_SUM) - 1;

	// for (eCommType = (E_CommType)0; (((WORD)1) << eCommType) <= wCommTypeFindInfo; ++eCommType)
	// {
	//  if (0 == ((((WORD)1) << eCommType) & wCommTypeFindInfo))
	//      continue;
	//  psSubComp = GetSubCompByWriteDBIndex(eCommType, tDBIndex);
	//  if (NULL != psSubComp)
	//  {
	//      Comm_SetWriteFlag(tDBIndex);
	//      SCommSubComp_SetWriteFlag(psSubComp);
	//      return;
	//  }
	// }
	psSubComp = GetSubCompByDBIndex(tDBIndex);
	if (NULL == psSubComp)
		return;
	psSegInfo = &psSubComp->PRO_sSDO_WriteInfo;
	wSum      = psSegInfo->bSegSum;
	DEBUG_ASSERT((wSum > 0) ? (NULL != psSegInfo->pasIndexSeg) : TRUE);
	for (i = 0; i < wSum; ++i)
	{
		if ((tDBIndex <= (psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[i].tEndOffset))
		    && (tDBIndex >= (psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[i].tStartOffset)))
		{
			DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
			Comm_ClrReadFlag(tDBIndex); // THJ 2016-12-9 Add.
			Comm_SetWriteFlag(tDBIndex);
			SCommSubComp_SetWriteFlag(psSubComp);
			return;
		}
	};
}
/**
 * [Comm_SetReadFlagByDBIndex description]
 * @param tDBIndex [description]
 */
void Comm_SetReadFlagByDBIndex(DBIndex_T tDBIndex)
{
	CommSubComp_S* psSubComp;
	// E_CommType eCommType;
	// WORD wCommTypeFindInfo;
	CommSegInfo_S* psSegInfo;
	WORD           i, wSum;

	// THJ 2016-05-30 Add.
	if (DATAACCESS_WO == DB_GetAccess(tDBIndex))
		return;
	if (Comm_GetWriteChanged(tDBIndex)) // THJ 2016-12-8 Add.
		return;
	// DEBUG_ASSERT(tDBIndex < DBINDEX_SUM)
	// if (((tDBIndex >= TEMPERATURE_CARD1_CONFIG_CARD_ID) && (tDBIndex <= TEMPERATURE_CARD8_INTERFACE_SOFTWARE_SN))
	//  || ((tDBIndex >= TEMPERCARD1_INTERFACE_SEG1_SET_SENSOR_TYPE) && (tDBIndex <= TEMPERCARD8_INTERFACE_SEG12_REAL_ERROR)))
	//  wCommTypeFindInfo = (((WORD)1) << TEMPERCARDNUM_DEFAULT_COMMTYPE);
	// else
	//  wCommTypeFindInfo = (((WORD)1) << COMMTYPE_SUM) - 1;
	// for (eCommType = (E_CommType)0; (((WORD)1) << eCommType) <= wCommTypeFindInfo; ++eCommType)
	// {
	//  if (0 == ((((WORD)1) << eCommType) & wCommTypeFindInfo))
	//      continue;
	//  psSubComp = GetSubCompByReadDBIndex(eCommType, tDBIndex);
	//  if (NULL != psSubComp)
	//  {
	//      Comm_SetReadFlag(tDBIndex);
	//      SCommSubComp_SetReadFlag(psSubComp);
	//      return;
	//  }
	// }
	psSubComp = GetSubCompByDBIndex(tDBIndex);
	if (NULL == psSubComp)
		return;
	psSegInfo = &psSubComp->PRO_sSDO_ReadInfo;
	wSum      = psSegInfo->bSegSum;
	DEBUG_ASSERT((wSum > 0) ? (NULL != psSegInfo->pasIndexSeg) : TRUE);
	for (i = 0; i < wSum; ++i)
	{
		if ((tDBIndex <= (psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[i].tEndOffset))
		    && (tDBIndex >= (psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[i].tStartOffset)))
		{
			DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
			Comm_SetReadFlag(tDBIndex);
			SCommSubComp_SetReadFlag(psSubComp);
			return;
		}
	};
}
/**
 * [Comm_UpdateDBIndex description]
 * @param  psIndexSegInfo [description]
 * @param  padwFlagData   [description]
 * @param  tDBIndex       [description]
 * @return                [description]
 */
static DBIndex_T Comm_UpdateDBIndex(CommSegInfo_S* psSegInfo, const DWORD* padwFlagData, DBIndex_T tDBIndex)
{
	//WORD wCount;
	WORD wCurSeg;

	DEBUG_ASSERT(NULL != psSegInfo);
	//*(THJ 2015-12-02 Mask: In order to support the disordered segment table.)
	//if (tDBIndex < psSegInfo->pasIndexSeg[0].tStartIndex)
	//  return psSegInfo->pasIndexSeg[0].tStartIndex;
	//*/
	if (tDBIndex >= DBINDEX_SUM)
	{
		if (NULL == psSegInfo->pasIndexSeg)
			return DBINDEX_SUM;
		psSegInfo->bCurSeg = 0;
		return psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[0].tStartOffset;
	}
	if (0 == padwFlagData[tDBIndex >> 5])
		tDBIndex += 0x20 - (tDBIndex & 0x1F);
	wCurSeg = psSegInfo->bCurSeg;
	while (1) // THJ 2016-11-25 Modify: "for (wCount = 0; wCount < 0x20; ++wCount)" -> "while (1)"
	{
		DEBUG_ASSERT(wCurSeg < psSegInfo->bSegSum);
		if (tDBIndex > (psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[wCurSeg].tEndOffset))
		{
			++wCurSeg;
			if (wCurSeg < psSegInfo->bSegSum)
			{
				psSegInfo->bCurSeg = wCurSeg;
				return (psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[wCurSeg].tStartOffset);
			}
			return DBINDEX_SUM;
		}
		else
		{
			DEBUG_ASSERT(tDBIndex < DBINDEX_SUM);
			if (0 != (padwFlagData[tDBIndex >> 5] & (((DWORD)1) << (tDBIndex & 0x1F))))
				return tDBIndex;
		}
		tDBIndex += (0 == padwFlagData[tDBIndex >> 5]) ? 0x20 : 0x01;
	}
	// return tDBIndex;
}
/**
 * [CommComp_SetRangeFlag description]
 * @param padwFlagData [description]
 * @param tStartIndex  [description]
 * @param tEndIndex    [description]
 */
void CommComp_SetRangeFlag(DWORD* padwFlagData, DBIndex_T tStartIndex, DBIndex_T tEndIndex)
{
	DWORD dwStartMask, dwEndMask;

	DEBUG_ASSERT(NULL != padwFlagData);
	DEBUG_ASSERT(tStartIndex < DBINDEX_SUM);
	DEBUG_ASSERT(tEndIndex < DBINDEX_SUM);
	DEBUG_ASSERT(tStartIndex <= tEndIndex);
	dwStartMask = tStartIndex & 0x1F;
	dwStartMask = ~((((DWORD)1) << dwStartMask) - 1);
	dwEndMask   = (tEndIndex & 0x1F) + 1;
	dwEndMask   = ((dwEndMask < 0x20) ? ((DWORD)1 << dwEndMask) : 0) - 1;
	tStartIndex >>= 5;
	tEndIndex >>= 5;
	if (tStartIndex == tEndIndex)
		padwFlagData[tStartIndex] |= dwStartMask & dwEndMask;
	else
	{
		padwFlagData[tStartIndex] |= dwStartMask;
		padwFlagData[tEndIndex] |= dwEndMask;
		while ((++tStartIndex) < tEndIndex)
			padwFlagData[tStartIndex] = (DWORD)-1;
	}
}
/**
 * [CommComp_ClrRangeFlag description]
 * @param padwFlagData [description]
 * @param tStartIndex  [description]
 * @param tEndIndex    [description]
 */
void CommComp_ClrRangeFlag(DWORD* padwFlagData, DBIndex_T tStartIndex, DBIndex_T tEndIndex)
{
	DWORD dwStartMask, dwEndMask;

	DEBUG_ASSERT(NULL != padwFlagData);
	DEBUG_ASSERT(tStartIndex < DBINDEX_SUM);
	DEBUG_ASSERT(tEndIndex < DBINDEX_SUM);
	DEBUG_ASSERT(tStartIndex <= tEndIndex);
	dwStartMask = tStartIndex & 0x1F;
	dwStartMask = ~((((DWORD)1) << dwStartMask) - 1);
	dwEndMask   = (tEndIndex & 0x1F) + 1;
	dwEndMask   = ((dwEndMask < 0x20) ? ((DWORD)1 << dwEndMask) : 0) - 1;
	tStartIndex >>= 5;
	tEndIndex >>= 5;
	if (tStartIndex == tEndIndex)
		padwFlagData[tStartIndex] &= ~(dwStartMask & dwEndMask);
	else
	{
		padwFlagData[tStartIndex] &= ~dwStartMask;
		padwFlagData[tEndIndex] &= ~dwEndMask;
		while ((++tStartIndex) < tEndIndex)
			padwFlagData[tStartIndex] = 0;
	}
}
/**
 * [SCommSubComp_Init description]
 * @param psSubComp [description]
 */
void SCommSubComp_Init(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	psSubComp->PRI_psNextSubComp              = NULL;
	psSubComp->PRO_sSDO_SyncInfo.pasIndexSeg  = NULL;
	psSubComp->PRO_sSDO_SyncInfo.tBaseIndex   = 0;
	psSubComp->PRO_sSDO_SyncInfo.bSegSum      = 0;
	psSubComp->PRO_sSDO_SyncInfo.bFlag        = FALSE;
	psSubComp->PRO_sSDO_SyncInfo.bCFlag       = FALSE;
	psSubComp->PRO_sSDO_SyncInfo.bCurSeg      = 0;
	psSubComp->PRO_sSDO_WriteInfo.pasIndexSeg = NULL;
	psSubComp->PRO_sSDO_WriteInfo.tBaseIndex  = 0;
	psSubComp->PRO_sSDO_WriteInfo.bSegSum     = 0;
	psSubComp->PRO_sSDO_WriteInfo.bFlag       = FALSE;
	psSubComp->PRO_sSDO_WriteInfo.bCFlag      = FALSE;
	psSubComp->PRO_sSDO_WriteInfo.bCurSeg     = 0;
	psSubComp->PRO_sSDO_ReadInfo.pasIndexSeg  = NULL;
	psSubComp->PRO_sSDO_ReadInfo.tBaseIndex   = 0;
	psSubComp->PRO_sSDO_ReadInfo.bSegSum      = 0;
	psSubComp->PRO_sSDO_ReadInfo.bFlag        = FALSE;
	psSubComp->PRO_sSDO_ReadInfo.bCFlag       = FALSE;
	psSubComp->PRO_sSDO_ReadInfo.bCurSeg      = 0;
	psSubComp->PRO_pvUnique                   = NULL;
}
/**
 * [SCommSubComp_InitSyncInfo description]
 * @param psSubComp   [description]
 * @param pasIndexSeg [description]
 * @param tBaseIndex  [description]
 * @param wSegSum     [description]
 */
void SCommSubComp_InitSyncInfo(CommSubComp_S* psSubComp, CommOffsetSeg_S const* pasIndexSeg, DBIndex_T tBaseIndex, WORD wSegSum)
{
	DEBUG_ASSERT(NULL != psSubComp);
	DEBUG_ASSERT((wSegSum > 0) ? (NULL != pasIndexSeg) : TRUE);
	psSubComp->PRO_sSDO_SyncInfo.pasIndexSeg = pasIndexSeg;
	psSubComp->PRO_sSDO_SyncInfo.tBaseIndex  = tBaseIndex;
	psSubComp->PRO_sSDO_SyncInfo.bSegSum     = wSegSum;
	psSubComp->PRO_sSDO_SyncInfo.bFlag       = FALSE;
	psSubComp->PRO_sSDO_SyncInfo.bCFlag      = FALSE;
	psSubComp->PRO_sSDO_SyncInfo.bCurSeg     = 0;
}
/**
 * [SCommSubComp_InitWriteInfo description]
 * @param psSubComp   [description]
 * @param pasIndexSeg [description]
 * @param tBaseIndex  [description]
 * @param wSegSum     [description]
 */
void SCommSubComp_InitWriteInfo(CommSubComp_S* psSubComp, CommOffsetSeg_S const* pasIndexSeg, DBIndex_T tBaseIndex, WORD wSegSum)
{
	DEBUG_ASSERT(NULL != psSubComp);
	DEBUG_ASSERT((wSegSum > 0) ? (NULL != pasIndexSeg) : TRUE);
	psSubComp->PRO_sSDO_WriteInfo.pasIndexSeg = pasIndexSeg;
	psSubComp->PRO_sSDO_WriteInfo.tBaseIndex  = tBaseIndex;
	psSubComp->PRO_sSDO_WriteInfo.bSegSum     = wSegSum;
	psSubComp->PRO_sSDO_WriteInfo.bFlag       = FALSE;
	psSubComp->PRO_sSDO_WriteInfo.bCFlag      = FALSE;
	psSubComp->PRO_sSDO_WriteInfo.bCurSeg     = 0;
}
/**
 * [SCommSubComp_InitReadInfo description]
 * @param psSubComp   [description]
 * @param pasIndexSeg [description]
 * @param tBaseIndex  [description]
 * @param wSegSum     [description]
 */
void SCommSubComp_InitReadInfo(CommSubComp_S* psSubComp, CommOffsetSeg_S const* pasIndexSeg, DBIndex_T tBaseIndex, WORD wSegSum)
{
	DEBUG_ASSERT(NULL != psSubComp);
	DEBUG_ASSERT((wSegSum > 0) ? (NULL != pasIndexSeg) : TRUE);
	psSubComp->PRO_sSDO_ReadInfo.pasIndexSeg = pasIndexSeg;
	psSubComp->PRO_sSDO_ReadInfo.tBaseIndex  = tBaseIndex;
	psSubComp->PRO_sSDO_ReadInfo.bSegSum     = wSegSum;
	psSubComp->PRO_sSDO_ReadInfo.bFlag       = FALSE;
	psSubComp->PRO_sSDO_ReadInfo.bCFlag      = FALSE;
	psSubComp->PRO_sSDO_ReadInfo.bCurSeg     = 0;
}
/**
 * [SCommSubComp_SetWriteFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_SetSyncFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SCommSegInfo_SetFlag(&psSubComp->PRO_sSDO_SyncInfo);
}
/**
 * [SCommSubComp_SetReadFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_SetReadFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SCommSegInfo_SetFlag(&psSubComp->PRO_sSDO_ReadInfo);
}
/**
 * [SCommSubComp_SetWriteFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_SetWriteFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SCommSegInfo_SetFlag(&psSubComp->PRO_sSDO_WriteInfo);
}
/**
 * [SCommSubComp_ClrWriteFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_ClrWriteFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SCommSegInfo_ClrFlag(&psSubComp->PRO_sSDO_WriteInfo);
}
/**
 * [SCommSubComp_SetAllSyncDBFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_SetAllSyncDBFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SetDBFlag(&psSubComp->PRO_sSDO_SyncInfo, g_sCommComp.adwWriteFlag);
}
/**
 * [SCommSubComp_SetAllReadDBFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_SetAllReadDBFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SetDBFlag(&psSubComp->PRO_sSDO_ReadInfo, g_sCommComp.adwReadFlag);
}
/**
 * [SCommSubComp_SetAllWriteDBFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_SetAllWriteDBFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	SetDBFlag(&psSubComp->PRO_sSDO_WriteInfo, g_sCommComp.adwWriteFlag);
}
/**
 * [SCommSubComp_ClrAllWriteDBFlag description]
 * @param psSubComp [description]
 */
void SCommSubComp_ClrAllWriteDBFlag(CommSubComp_S* psSubComp)
{
	DEBUG_ASSERT(NULL != psSubComp);
	ClrDBFlag(&psSubComp->PRO_sSDO_WriteInfo, g_sCommComp.adwWriteFlag);
}
/**
 * [SCommComp_Init description]
 * @param psComp [description]
 */
void SCommComp_Init(CommComp_S* psComp)
{
	DEBUG_ASSERT(NULL != psComp);
	psComp->PRI_psNextComp    = NULL;
	psComp->PRI_psSubCompHead = NULL;
	psComp->PRI_psCurSubComp  = NULL;
	psComp->PRO_fEvent        = Default_EventCallBack;
	psComp->PRO_fCustomFunc   = Default_CustomFunc;
	psComp->PRO_pvUnique      = NULL;
	psComp->PRI_eCommType     = COMMTYPE_SUM;
	psComp->PRI_tCurIndex     = DBINDEX_SUM;
	psComp->PRO_bState        = COMPSTATE_INIT;
	psComp->PRO_bSDOState     = COMMSDOSTATE_IDLE;
	psComp->PRI_bSyncStep     = COMMSYNCSTEP_SUM;
#if (DEBUG)
	psComp->PUB_eCompType = COMPTYPE_UNKNOWN;
#endif
}
/**
 * [SCommComp_InitEventCallBack description]
 * @param psComp         [description]
 * @param fEventCallBack [description]
 */
void SCommComp_InitEventCallBack(CommComp_S* psComp, CommEventCallBack_F fEventCallBack)
{
	DEBUG_ASSERT(NULL != psComp);
	psComp->PRO_fEvent = (NULL != fEventCallBack) ? fEventCallBack : Default_EventCallBack;
}
/**
 * [SCommComp_InitCustomFunc description]
 * @param psComp      [description]
 * @param fCustomFunc [description]
 */
void SCommComp_InitCustomFunc(CommComp_S* psComp, CommCustomFunc_F fCustomFunc)
{
	DEBUG_ASSERT(NULL != psComp);
	psComp->PRO_fCustomFunc = (NULL != fCustomFunc) ? fCustomFunc : Default_CustomFunc;
}
/**
 * [SCommComp_SetState description]
 * @param psComp [description]
 * @param eState [description]
 */
void SCommComp_SetState(CommComp_S* psComp, E_CompState eState)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(eState < COMPSTATE_SUM);
	psComp->PRO_bState = eState;
	switch (eState)
	{
		case COMPSTATE_INIT:
			FlushByComp(psComp);
			break;
		case COMPSTATE_SYNC:
			SCommComp_ClrAllWriteFlag(psComp); // hankin 20181023 add: necessary.
			SCommComp_SetAllSyncFlag(psComp);
			psComp->PRI_bSyncStep = COMMSYNCSTEP_SCAN;
			break;
		default:
			break;
	}
}
/**
 * [SCommComp_SetCommType description]
 * @param psComp    [description]
 * @param eCommType [description]
 */
void SCommComp_SetCommType(CommComp_S* psComp, E_CommType eCommType)
{
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(eCommType <= COMMTYPE_SUM);
	if (eCommType != psComp->PRI_eCommType)
	{
		E_CommEvent eEvent;

		SCommComp_SetState(psComp, COMPSTATE_INIT);
		if (psComp->PRI_eCommType < COMMTYPE_SUM)
			DelComp(psComp);
		if (eCommType < COMMTYPE_SUM) {
			AddComp(eCommType, psComp);
			Error_App_Reset();
		}
		DEBUG_ASSERT(NULL != psComp->PRO_fEvent);
		eEvent = COMMEVENT_COMP_CHANGE_COMMTYPE;
		psComp->PRO_fEvent(psComp, (CommEvent_U*)&eEvent); //NULL, COMMEVENT_CHANGE_COMMTYPE);
	}
}
/**
 * [SCommComp_AddSubComp description]
 * @param psComp    [description]
 * @param psSubComp [description]
 */
void SCommComp_AddSubComp(CommComp_S* psComp, CommSubComp_S* psSubComp)
{
	CommEvent_AddSubComp_S sEvent;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psSubComp);
	DEBUG_ASSERT(NULL == psSubComp->PRI_psNextSubComp);
	SCommComp_SetState(psComp, COMPSTATE_INIT); // THJ 2016-3-26, Add.
	psSubComp->PRI_psNextSubComp = psComp->PRI_psSubCompHead;
	psComp->PRI_psSubCompHead    = psSubComp;
	// SCommSubComp_SetAllWriteDBFlag(psSubComp);
	// SCommSubComp_SetAllReadDBFlag(psSubComp);
	DEBUG_ASSERT(NULL != psComp->PRO_fEvent);
	sEvent.eEvent    = COMMEVENT_SUBCOMP_ADD;
	sEvent.psSubComp = psSubComp;
	psComp->PRO_fEvent(psComp, (CommEvent_U*)&sEvent); //psSubComp, COMMEVENT_ADD_SUBCOMP);
}
/**
 * [SCommComp_DelSubComp description]
 * @param  psComp    [description]
 * @param  psSubComp [description]
 * @return           [description]
 */
E_CommRetCode SCommComp_DelSubComp(CommComp_S* psComp, CommSubComp_S* psSubComp)
{
	CommSubComp_S*         psCurSubComp;
	CommEvent_DelSubComp_S sEvent;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL != psSubComp);
	sEvent.eEvent = COMMEVENT_SUBCOMP_DEL;
	psCurSubComp  = psComp->PRI_psSubCompHead;
	while (NULL != psCurSubComp)
	{
		if (psSubComp == psCurSubComp->PRI_psNextSubComp)
		{
			psCurSubComp->PRI_psNextSubComp = psSubComp->PRI_psNextSubComp;
			psSubComp->PRI_psNextSubComp    = NULL;
			if (psSubComp == psComp->PRI_psCurSubComp)
				psComp->PRI_psCurSubComp = psCurSubComp->PRI_psNextSubComp;
			DEBUG_ASSERT(NULL != psComp->PRO_fEvent);
			sEvent.psSubComp = psSubComp;
			psComp->PRO_fEvent(psComp, (CommEvent_U*)&sEvent); //psSubComp, COMMEVENT_DEL_SUBCOMP);
			return COMMRETCODE_SUCCESS;
		}
		psCurSubComp = psCurSubComp->PRI_psNextSubComp;
	}
	if (psSubComp == psComp->PRI_psSubCompHead)
	{
		psComp->PRI_psSubCompHead    = psSubComp->PRI_psNextSubComp;
		psSubComp->PRI_psNextSubComp = NULL;
		if (psSubComp == psComp->PRI_psCurSubComp)
			psComp->PRI_psCurSubComp = psComp->PRI_psSubCompHead;
		DEBUG_ASSERT(NULL != psComp->PRO_fEvent);
		sEvent.psSubComp = psSubComp;
		psComp->PRO_fEvent(psComp, (CommEvent_U*)&sEvent); //psSubComp, COMMEVENT_DEL_SUBCOMP);
		return COMMRETCODE_SUCCESS;
	}
	return COMMRETCODE_DELSUBCOMP_NOTFIND;
}
/**
 * [SCommComp_ClrSDOState description]
 * @param psComp [description]
 */
void SCommComp_ClrSDOState(CommComp_S* psComp)
{
	DEBUG_ASSERT(NULL != psComp);
	psComp->PRO_bSDOState = COMMSDOSTATE_FINISH;
}
/**
 * [SCommComp_SetAllSyncFlag description]
 * @param psComp [description]
 */
void SCommComp_SetAllSyncFlag(CommComp_S* psComp)
{
	CommSubComp_S* psCurSubComp;

	DEBUG_ASSERT(NULL != psComp);
	psCurSubComp = psComp->PRI_psSubCompHead;
	while (NULL != psCurSubComp)
	{
		SCommSubComp_SetAllSyncDBFlag(psCurSubComp);
		SCommSubComp_SetSyncFlag(psCurSubComp);
		psCurSubComp = psCurSubComp->PRI_psNextSubComp;
	}
}
/**
 * [SCommComp_SetAllReadFlag description]
 * @param psComp [description]
 */
void SCommComp_SetAllReadFlag(CommComp_S* psComp)
{
	CommSubComp_S* psCurSubComp;

	DEBUG_ASSERT(NULL != psComp);
	psCurSubComp = psComp->PRI_psSubCompHead;
	while (NULL != psCurSubComp)
	{
		SCommSubComp_SetAllReadDBFlag(psCurSubComp);
		SCommSubComp_SetReadFlag(psCurSubComp);
		psCurSubComp = psCurSubComp->PRI_psNextSubComp;
	}
}
/**
 * [SCommComp_SetAllWriteFlag description]
 * @param psComp [description]
 */
void SCommComp_SetAllWriteFlag(CommComp_S* psComp)
{
	CommSubComp_S* psCurSubComp;

	DEBUG_ASSERT(NULL != psComp);
	psCurSubComp = psComp->PRI_psSubCompHead;
	while (NULL != psCurSubComp)
	{
		SCommSubComp_SetAllWriteDBFlag(psCurSubComp);
		SCommSubComp_SetWriteFlag(psCurSubComp);
		psCurSubComp = psCurSubComp->PRI_psNextSubComp;
	}
}
/**
 * [SCommComp_ClrAllWriteDBFlag description]
 * @param psComp [description]
 */
void SCommComp_ClrAllWriteFlag(CommComp_S* psComp)
{
	CommSubComp_S* psCurSubComp;

	DEBUG_ASSERT(NULL != psComp);
	psCurSubComp = psComp->PRI_psSubCompHead;
	while (NULL != psCurSubComp)
	{
		SCommSubComp_ClrAllWriteDBFlag(psCurSubComp);
		SCommSubComp_ClrWriteFlag(psCurSubComp);
		psCurSubComp = psCurSubComp->PRI_psNextSubComp;
	}
}
/**
 * [AddComp description]
 * @param eCommType [description]
 * @param psComp    [description]
 */
inline void AddComp(E_CommType eCommType, CommComp_S* psComp)
{
	DEBUG_ASSERT(eCommType < COMMTYPE_SUM);
	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(NULL == psComp->PRI_psNextComp);
	DEBUG_ASSERT(COMMTYPE_SUM == psComp->PRI_eCommType);
	psComp->PRI_psNextComp                 = g_sCommComp.apsCommCompHead[eCommType];
	g_sCommComp.apsCommCompHead[eCommType] = psComp;
	psComp->PRI_eCommType                  = eCommType;
}
/**
 * [DelComp description]
 * @param psComp [description]
 */
inline void DelComp(CommComp_S* psComp)
{
	CommComp_S *psPrevComp, *psCurComp;
	E_CommType  eCommType;

	DEBUG_ASSERT(NULL != psComp);
	DEBUG_ASSERT(psComp->PRI_eCommType < COMMTYPE_SUM);
	eCommType = psComp->PRI_eCommType;
	psCurComp = g_sCommComp.apsCommCompHead[eCommType];
	while (psCurComp != psComp)
	{
		psPrevComp = psCurComp;
		psCurComp  = psCurComp->PRI_psNextComp;
	}
	if (psCurComp != g_sCommComp.apsCommCompHead[eCommType])
		psPrevComp->PRI_psNextComp = psComp->PRI_psNextComp;
	else
		g_sCommComp.apsCommCompHead[eCommType] = psComp->PRI_psNextComp;
	psComp->PRI_eCommType  = COMMTYPE_SUM;
	psComp->PRI_psNextComp = NULL;
}
/**
 * [GetSubCompByWriteDBIndex description]
 * @param  eCommType [description]
 * @param  tDBIndex  [description]
 * @return           [description]
 */
// static CommSubComp_S* GetSubCompByWriteDBIndex(E_CommType eCommType, DBIndex_T tDBIndex)
// {
//  CommComp_S *psComp;
//  CommSubComp_S *psSubComp;
//  CommIndexSegInfo_S *psSegInfo;
//  WORD i, wSum;

//  DEBUG_ASSERT;(eCommType < COMMTYPE_SUM)
//  DEBUG_ASSERT;(tDBIndex < DBINDEX_SUM)
//  psComp = u_apsCommCompHead[eCommType];
//  while (NULL != psComp)
//  {
//      psSubComp = psComp->PRI_psSubCompHead;
//      while (NULL != psSubComp)
//      {
//          psSegInfo = psSubComp->PRO_psSDO_WriteInfo;
//          if (NULL == psSegInfo)
//              break;
//          wSum = psSegInfo->bSegSum;
//          DEBUG_ASSERT;((NULL != psSegInfo->pasIndexSeg) ? TRUE : (0 == wSum))
//          for (i = 0; i < wSum; ++i)
//          {
//              if ((tDBIndex <= psSegInfo->pasIndexSeg[i].tEndIndex)
//                  && (tDBIndex >= psSegInfo->pasIndexSeg[i].tStartIndex))
//                  return psSubComp;
//          }
//          psSubComp = psSubComp->PRI_psNextSubComp;
//      }
//      psComp = psComp->PRI_psNextComp;
//  }
//  return NULL;
// }
/**
 * [GetSubCompByReadDBIndex description]
 * @param  eCommType [description]
 * @param  tDBIndex    [description]
 * @return           [description]
 */
// static CommSubComp_S* GetSubCompByReadDBIndex(E_CommType eCommType, DBIndex_T tDBIndex)
// {
//  CommComp_S *psComp;
//  CommSubComp_S *psSubComp;
//  CommIndexSegInfo_S *psSegInfo;
//  WORD i, wSum;

//  DEBUG_ASSERT;(eCommType < COMMTYPE_SUM)
//  DEBUG_ASSERT;(tDBIndex < DBINDEX_SUM)
//  psComp = u_apsCommCompHead[eCommType];
//  while (NULL != psComp)
//  {
//      psSubComp = psComp->PRI_psSubCompHead;
//      while (NULL != psSubComp)
//      {
//          psSegInfo = psSubComp->PRO_psSDO_ReadInfo;
//          if (NULL == psSegInfo)
//              break;
//          wSum = psSegInfo->bSegSum;
//          DEBUG_ASSERT;((NULL != psSegInfo->pasIndexSeg) ? TRUE : (0 == wSum))
//          for (i = 0; i < wSum; ++i)
//          {
//              if ((tDBIndex <= psSegInfo->pasIndexSeg[i].tEndIndex)
//                  && (tDBIndex >= psSegInfo->pasIndexSeg[i].tStartIndex))
//                  return psSubComp;
//          }
//          psSubComp = psSubComp->PRI_psNextSubComp;
//      }
//      psComp = psComp->PRI_psNextComp;
//  }
//  return NULL;
// }
/**
 * [SCommSegInfo_SetFlag description]
 * @param psSegInfo [description]
 */
static void SCommSegInfo_SetFlag(CommSegInfo_S* psSegInfo)
{
	DEBUG_ASSERT(NULL != psSegInfo);
	if (psSegInfo->bSegSum > 0)
	{
		psSegInfo->bFlag  = TRUE;
		psSegInfo->bCFlag = FALSE;
	}
	else
		psSegInfo->bFlag = FALSE;
}
/**
 * [SCommSegInfo_ClrFlag description]
 * @param psSegInfo [description]
 */
inline void SCommSegInfo_ClrFlag(CommSegInfo_S* psSegInfo)
{
	DEBUG_ASSERT(NULL != psSegInfo);
	psSegInfo->bFlag = FALSE;
}
/**
 * [SetDBFlag description]
 * @param psSegInfo [description]
 * @param padwFlag  [description]
 */
inline void SetDBFlag(CommSegInfo_S const* psSegInfo, DWORD* padwFlag)
{
	CommOffsetSeg_S const* pasIndexSeg;
	WORD                   i, wSum;

	DEBUG_ASSERT(NULL != psSegInfo);
	pasIndexSeg = psSegInfo->pasIndexSeg;
	wSum        = psSegInfo->bSegSum;
	DEBUG_ASSERT((wSum > 0) ? (NULL != pasIndexSeg) : TRUE);
	for (i = 0; i < wSum; ++i)
		CommComp_SetRangeFlag(padwFlag, psSegInfo->tBaseIndex + pasIndexSeg[i].tStartOffset, psSegInfo->tBaseIndex + pasIndexSeg[i].tEndOffset);
}
/**
 * [ClrDBFlag description]
 * @param psSegInfo [description]
 * @param padwFlag  [description]
 */
inline void ClrDBFlag(CommSegInfo_S const* psSegInfo, DWORD* padwFlag)
{
	CommOffsetSeg_S const* pasIndexSeg;
	WORD                   i, wSum;

	DEBUG_ASSERT(NULL != psSegInfo);
	pasIndexSeg = psSegInfo->pasIndexSeg;
	wSum        = psSegInfo->bSegSum;
	DEBUG_ASSERT((wSum > 0) ? (NULL != pasIndexSeg) : TRUE);
	for (i = 0; i < wSum; ++i)
		CommComp_ClrRangeFlag(padwFlag, psSegInfo->tBaseIndex + pasIndexSeg[i].tStartOffset, psSegInfo->tBaseIndex + pasIndexSeg[i].tEndOffset);
}

/**
 * [FlushByComp description]
 * @param apsSubComp [description]
 * @param psComp     [description]
 */
inline void FlushByComp(CommComp_S const* const psComp)
{
	CommSubComp_S* psSubComp;

	DEBUG_ASSERT(NULL != psComp);
	psSubComp = psComp->PRI_psSubCompHead;
	while (NULL != psSubComp)
	{
		CommSegInfo_S const* psSegInfo;

		psSegInfo = &psSubComp->PRO_sSDO_SyncInfo;
		if (NULL != psSegInfo)
		{
			if (NULL != psSegInfo->pasIndexSeg)
			{
				DWORD dwID;

				dwID = DB_IndexToID(psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[0].tStartOffset);
				DEBUG_ASSERT((dwID >> 24) < (sizeof(g_sCommComp.apsSubCompTab) / sizeof(*g_sCommComp.apsSubCompTab)));
				g_sCommComp.apsSubCompTab[dwID >> 24] = psSubComp;
			}
		}
		psSegInfo = &psSubComp->PRO_sSDO_ReadInfo;
		if (NULL != psSegInfo)
		{
			if (NULL != psSegInfo->pasIndexSeg)
			{
				DWORD dwID;

				dwID = DB_IndexToID(psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[0].tStartOffset);
				DEBUG_ASSERT((dwID >> 24) < (sizeof(g_sCommComp.apsSubCompTab) / sizeof(*g_sCommComp.apsSubCompTab)));
				g_sCommComp.apsSubCompTab[dwID >> 24] = psSubComp;
			}
		}
		psSegInfo = &psSubComp->PRO_sSDO_WriteInfo;
		if (NULL != psSegInfo)
		{
			if (NULL != psSegInfo->pasIndexSeg)
			{
				DWORD dwID;

				dwID = DB_IndexToID(psSegInfo->tBaseIndex + psSegInfo->pasIndexSeg[0].tStartOffset);
				DEBUG_ASSERT((dwID >> 24) < (sizeof(g_sCommComp.apsSubCompTab) / sizeof(*g_sCommComp.apsSubCompTab)));
				g_sCommComp.apsSubCompTab[dwID >> 24] = psSubComp;
			}
		}
		psSubComp = psSubComp->PRI_psNextSubComp;
	}
}

/**
 * [Default_fCreate description]
 * @return  [description]
 */
inline E_CommRetCode Default_fCreate(E_CommType eCommType)
{
	return COMMRETCODE_SUCCESS;
}
/**
 * [Default_fDestory description]
 * @return  [description]
 */
inline E_CommRetCode Default_fDestory(E_CommType eCommType)
{
	return COMMRETCODE_SUCCESS;
}
/**
 * [Default_fParseMsg description]
 */
inline void Default_fParseMsg(E_CommType eCommType, CommComp_S* psCompHead)
{
}
/**
 * [Default_fCtrlRun description]
 * @param psCommComp [description]
 */
// inline void Default_fCtrlRun(E_CommType eCommType, CommComp_S *psCommComp)
// {}
/**
 * [Default_EventCallBack description]
 * @param psComp  [description]
 * @param puEvent [description]
 */
inline void Default_EventCallBack(struct CommComp_S* psComp, union CommEvent_U* puEvent)
{
}
/**
 * [Default_CustomFunc description]
 * @param psComp [description]
 * @param eType  [description]
 */
inline void Default_CustomFunc(CommComp_S* psComp, E_CustomFuncType eType)
{
}
