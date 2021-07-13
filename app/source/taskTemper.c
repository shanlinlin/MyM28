/*==========================================================================+
|  Function :                                                               |
|  Task     :                                                               |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders                                                        |
|  Version  : V1.00                                                         |
|  Creation : 2015/01/30                                                    |
|  Revision :                                                               |
+==========================================================================*/
#include "../include/taskTemper.h"
#include "../include/database.h"
#include "task.h"
#include "temper_common.h"
/*===========================================================================+
|           Definition                                                       |
+===========================================================================*/
/*===========================================================================+
|           Function Prototype                                               |
+===========================================================================*/

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
const TEMPER_ALG_CONF_T u_Temper_Alg_Config = {
	TEMPER_CAL_CYCLETYPE * TEMPER_1S_NBR, //wCalCycle;
	TEMPER_OUTPUT_CYCLETYPE,              //wOutputCycle;
	10                                    //wPrecision;
};

const TEMPER_ALG_CONF_T u_Temper_Alg_Config_FastMold = {
	TEMPER_CAL_CYCLETYPE_MOLDFAST * TEMPER_1S_NBR, //wCalCycle;
	TEMPER_OUTPUT_CYCLETYPE,                       //wOutputCycle;
	10                                             //wPrecision;
};
/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/
/*===========================================================================+
|           Class implementation                                             |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
void CreatTask_Temper()
{
	// int i;
	TEMPER_COMMON_DB_T* pTemperCommonDb = (TEMPER_COMMON_DB_T*)&g_sDB.M28.TEMPERATURE.PIPE_SENSOR_TYPE;    // DB_GetDataAddr(DBINDEX_LOCAL0_TEMPERATURE_HEAT_CMD);
	TEMPER_ALG_DB_T*    pTemperAlgDb    = (TEMPER_ALG_DB_T*)&g_sDB.TEMPER1.INTERFACE_SEG1.SET_SENSOR_TYPE; // DB_GetDataAddr(DBINDEX_TEMPER0_INTERFACE_SEG1_SET_SENSOR_TYPE);
	TemperCommon_Init(TEMPER_MAX_NBR, &u_Temper_Alg_Config, &u_Temper_Alg_Config_FastMold, pTemperCommonDb, pTemperAlgDb);

	Create_UnTimerintTask(TEMPER_CAL_CYCLETYPE, RunTask_Temper, NULL);
	CreatTask_Temper_NormalMold();
	// for (i = 0; i < TEMPER_MAX_NBR; i++)//shanll 20200826 add
	// {
	// 	TemperCommon_ChangeRapidCompensationUse(i, TRUE);
	// }
	// g_sDB.M28.TEMPERATURE.MOLD_FAST_OPT = 2;//shanlinlin 20201228 add for special test
}

void DestoryTask_Temper()
{
	//Destroy_UnTimerintTask(&TemperCommon_RunCal, NULL);
	//Destroy_UnTimerintTask(&TemperCommon_RunOutput, NULL);
	Destroy_UnTimerintTask(&RunTask_Temper, NULL);
}

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RunTask_Temper()
{
	// WORD wRommAD = ReadDB_WORD(DBINDEX_TEMPERCARD0_CONFIG_ROOM_TEMPERATURE); // Anders 2016-12-28, Add.
	TempCommon_DistributeHeatCMD();
	TemperCommon_RunCal(TEMPER_DEV_PIPE, TEMPER_CAL_CYCLETYPE);
	TemperCommon_RunOutput();

	// wRommAD = (wRommAD >> 4) - 2731;
	// WriteDB_WORD(DBINDEX_LOCAL0_HMI_MOLD_DATA_REVVED, wRommAD);
}
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/

void CreatTask_Temper_NormalMold()
{
	Destroy_UnTimerintTask(RunTask_Temper_FastMold, NULL);
	Create_UnTimerintTask(TEMPER_CAL_CYCLETYPE, RunTask_Temper_NormalMold, NULL);
}

void CreatTask_Temper_FastMold()
{
	Destroy_UnTimerintTask(RunTask_Temper_NormalMold, NULL);
	Create_UnTimerintTask(TEMPER_CAL_CYCLETYPE_MOLDFAST, RunTask_Temper_FastMold, NULL);
}
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RunTask_Temper_NormalMold()
{
	TemperCommon_RunCal(TEMPER_DEV_MOLD, TEMPER_CAL_CYCLETYPE);
}
void RunTask_Temper_FastMold()
{
	TemperCommon_RunCal(TEMPER_DEV_MOLD, TEMPER_CAL_CYCLETYPE_MOLDFAST);
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
