/*==========================================================================+
|  Function : EthercatMaster Task.	                                        |
|  Task     : 								                                |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders.                                                       |
|  Version  : V1.0                                                          |
|  Creation : 20110909.                                                     |
|  Revision :                                                               |
+==========================================================================*/
#ifndef D__TASKNET_ETHERCATMASTER_H
#define D__TASKNET_ETHERCATMASTER_H

#include "common.h"
#include "ethercatmaster.h"
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
#define MAX_ECMASTER MAX_NETDEVICE

#define ECSLAVE_MAX 20

#define ECMASTERDEVICEHANDLE 2

#define SDO_READTYPE_MOTOR 0
#define SDO_READTYPE_DRIVE 1

/**
 * Motor | Drive Data Count
 */
#define MOTORDATA_CNT (sizeof(TMotorDataInfo) / sizeof(DWORD))
#define DRIVEDATA_CNT (sizeof(TDriveDataInfo) / sizeof(DWORD))

/**
 * HT AxisCard
 */
#define HTAXISCARD_AXIS_MAX 4
/*---------------------------------------------------------------------------+
|           Type Definition	                                                 |
+---------------------------------------------------------------------------*/
/**
 * EtherCAT Controller.
 */
typedef struct tagECController
{
	BOOL bDoClearError; // Anders 2013-8-29,add.
	WORD wDoClearErrorCount;
	BOOL bFirstUploadSDODone; // Anders 2013-9-6,add.

	TECMaster Master;
	TECSlave  Slave[ECSLAVE_MAX];
	WORD      awSlaveParamReaderNO[ECSLAVE_MAX];
	WORD      awSlaveNodeID[ECSLAVE_MAX];

	//DWORD         adwOffsetCompensation[ECSLAVE_MAX];
	//DWORD         adwPropagationDelay[ECSLAVE_MAX];
	//DWORD         adwRecieveTimePort0[ECSLAVE_MAX]; // Receive Time Port 0 (0x0900:0x0903 )
	//DWORD         adwRecieveTimePort1[ECSLAVE_MAX]; // Receive Time Port 1 (0x0904:0x0907 )
	//DWORD         aqwLocalTime[ECSLAVE_MAX];        // Receive Time ECAT Processing Unit (0x0918:0x091F)
	//ECATLOCALTIME aqwSystemTime[ECSLAVE_MAX];
	//ECATLOCALTIME aqwSystemTimeOffset[ECSLAVE_MAX]; //Register System Time Offset (0x0920:0x0927)
	//ECATLOCALTIME aqwSYNC0StartTime[ECSLAVE_MAX];
} TECController;

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
typedef void (*PFC_ECATMaster_Event)(); // ECATMaster Event.
typedef void (*PFC_ResetECSlave)(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
/*===========================================================================+
|           Class declaration - TaskCmd                                      |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD CreateTask_EthercatMaster();
void DestoryTask_EthercatMaster();
void RunTask_EthercatMaster();
void Restart_EtherCatMaster();
void DealSwitch_EthercatMaster();

void Restart_SDOUpload(WORD wECSlaveIndex);

void SetSlaveParamReaderNO(WORD wECSlaveIdx, WORD wAxisIdx, WORD wConfigIdx);
void SetAppSlaveCount(WORD wECSlaveCount);
void Reset_ECSlave(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void SetECSlaveNodeID(WORD wECSlaveIdx, WORD wNodeID);
int  GetECSlaveIdxByNodeID(WORD wNodeID);

void AssignECSlaveHandle(WORD wAxisIdx, WORD wConfigIdx);
void DeleteECSlaveHandle(WORD wAxisIdx, WORD wConfigIdx);

void                     SetECSlaveClearErrorFlag(WORD wECSlaveIndex);
void                     ClearECSlaveClearErrorFlag(WORD wECSlaveIndex);
//void    AssignECSlaveHandle_MotionCard( WORD wDBIndex );	// Anders 2015-10-27, add.
void ECATMasterRx_PDOCheck(void* pvParam);
WORD ECATMasterGetError(void);
WORD ECATMasterGetOfflineIndex(void);
void SetSlaveParamReaderNULL(WORD wECSlaveIdx);
void DealECSlaveHandle();

void RegCallBack_ECATMasterInitSlaveError(PFC_ECATMaster_Event pfc);
void RegCallBack_ECATMasterDoClearError(PFC_ECATMaster_Event pfc);
void RegCallBack_ECATMasterAfterClearError(PFC_ECATMaster_Event pfc);
void RegCallBack_ECATMasterBeforeRun(PFC_ECATMaster_Event pfc);

void RegCallBack_ResetECSlave_E2C(PFC_ResetECSlave pfc);
void RegCallBack_ResetECSlave_HYB5TB(PFC_ResetECSlave pfc);
void RegCallBack_ResetECSlave_HTJ6_Oil(PFC_ResetECSlave pfc);
void RegCallBack_ResetECSlave_HTJ6_EleInj(PFC_ResetECSlave pfc);
void RegCallBack_ResetECSlave_HTJ6_Plus(PFC_ResetECSlave pfc);

void RegCallBack_ResetECSlave_MT_HUB_8A(PFC_ResetECSlave pfc);            // Anders 2018-6-15, add.
void RegCallBack_ResetECSlave_HT_EH1708X(PFC_ResetECSlave pfc);           // hankin 20190612 add.
void RegCallBack_ResetECSlave_BTL6_V11E(PFC_ResetECSlave pfc);            // hankin 20190603 add.
void RegCallBack_ResetECSlave_BTL6_V107(PFC_ResetECSlave pfc);            // shanll 20200120 add.
void RegCallBack_ResetECSlave_PARKER_COMPAX3_FLUID(PFC_ResetECSlave pfc); // hankin 20190606 add.
void RegCallBack_ResetECSlave_TR_LINEAR(PFC_ResetECSlave pfc);            // hankin 20190801 add.
void RegCallBack_ResetECSlave_HTJ6_PP(PFC_ResetECSlave pfc);              // SHANLL 20191223 add.
void RegCallBack_ResetECSlave_REXROTH(PFC_ResetECSlave pfc);              // SHANLL 20201111 add.

void RegCallBack_DealECSlaveHandle_New(PFC_ECATMaster_Event pfc);

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/
extern TECController g_ECController;
extern WORD 			g_wMasterErrorScan;
extern ethercat_s_net_buffer		 g_ECSwitchBuffer[];
extern WORD		 g_ECSwitchBufferUsed;
extern WORD		 g_ECSwitchBlockUsed;
extern ethercat_s_net_block g_ECSwitchBlock[];

#endif
