/*==========================================================================+
|  Function : EthercatMaster Task.                                          |
|  Task     :                                                               |
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
#include "task_ethercatmaster.h"
#include "_timer.h"
#include "common.h"
#include "ethernet.h"
#include "Device.h"
#include "debug.h"
#include "tasknet_PC.h"
#include <string.h>

/*===========================================================================+
|           Function Prototype                                               |
+===========================================================================*/

#define SDODELAY 4
#define CONFIGDRIVEMACHINESTATE_DELAY 4
#define EC_RXPDOCHECK_CNT 30
#define EC_INITCHECK_CNT 8000

static void ParseNetData(int nSessionHandle, WORD* pdata, WORD wlength);

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
const IOETH CNST_IOETH_EC = {
	PROTOCOL_ETHERCAT,
	{ { 0xFF, 0xFF }, { 0xFF, 0xFF }, { 0xFF, 0xFF } },
	{ { 0, 0 }, { 0, 0 } },
	// shanll 20210619 modify{ { 1, 2 }, { 3, 4 } },
	{ { 0, 0 }, { 0, 0 } },
	2312,
	2312,
	ParseNetData
};

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/
//#if  defined D_DSP2812 //|| defined D_SMC335M2
#pragma DATA_SECTION(g_ECController, "ECMaster")
//#endif
TECController g_ECController;
ethercat_s_net_buffer		 g_ECSwitchBuffer[4];
WORD		 				 g_ECSwitchBufferUsed;
WORD		 				 g_ECSwitchBlockUsed;
ethercat_s_net_block g_ECSwitchBlock[48];
WORD g_wRunECMasterFlag = 0;
WORD g_wMasterErrorScan;//20200622 hankin

WORD g_wUploadSDOFlag = 0;
WORD g_wIndex         = 0;
WORD g_wSubIndex      = 0;
WORD g_wByteSizeSDO   = 0;

//Ttimer u_RxPDOCheckTimer = { EC_RXPDOCHECK_CNT };
Ttimer u_dwInitCheckCNT;
DWORD  g_dwTestRxPDOCNT = 0;

//WORD                u_wE2CCardSlaveStartIndex = 0;
WORD u_wFirstUpdateReady = 0; // Anders 2016-3-8, Add.

static PFC_ECATMaster_Event u_pfcECATMasterInitSlaveError          = NULL;
static PFC_ECATMaster_Event u_pfcECATMasterDoClearError            = NULL;
static PFC_ECATMaster_Event u_pfcECATMasterAfterClearError         = NULL;
static PFC_ECATMaster_Event u_pfcECATMasterBeforeRun               = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_E2C                  = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_HYB5TB               = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_HTJ6_Oil             = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_HTJ6_EleInj          = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_HTJ6_Plus            = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_MT_HUB_8A            = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_HT_EH1708X           = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_BTL6_V11E            = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_BTL6_V107            = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_PARKER_COMPAX3_FLUID = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_TR_LINEAR            = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_HTJ6_PP              = NULL;
static PFC_ResetECSlave     u_pfcResetECSlave_REXROTH              = NULL;
static PFC_ECATMaster_Event u_pfcDealECSlaveHandle_New             = NULL;
/*===========================================================================+
|           Class implementation                                             |
+===========================================================================*/

/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
//#define     INIT_E2C
#ifdef INIT_E2C
TCtlCMD     g_CtlCMD      = { 0 };
TSlaveState g_SlaveSatate = { 0 };
#endif
/**
*  Init EtherCAT Master.
*/
static int ECAT_WriteNet(int handle, void const* data, unsigned length)
{
    return _WriteNetDevice(handle, (WORD*)data, length);
}

WORD CreateTask_EthercatMaster()
{
	int          i;
	TECMasterCfg ECMasterCfg;

	memset(&g_ECController, 0, sizeof(g_ECController));

	if (-1 == OpenEthSession(ECMASTERDEVICEHANDLE, (IOETH*)&CNST_IOETH_EC))
		return 0;

	// g_ECController.Master.nDeviceHandle = ECMASTERDEVICEHANDLE;
	// g_ECController.Master.bEnableDCSync = TRUE;

	//g_ECController.Master.pdwOffsetCompensation = g_ECController.adwOffsetCompensation;
	//g_ECController.Master.pdwPropagationDelay   = g_ECController.adwPropagationDelay;
	//g_ECController.Master.pdwRecieveTimePort0   = g_ECController.adwRecieveTimePort0;
	//g_ECController.Master.pdwRecieveTimePort1   = g_ECController.adwRecieveTimePort1;
	//g_ECController.Master.pqwLocalTime          = g_ECController.aqwLocalTime;
	//g_ECController.Master.pqwSystemTime         = g_ECController.aqwSystemTime;
	//g_ECController.Master.pqwSystemTimeOffset   = g_ECController.aqwSystemTimeOffset;
	//g_ECController.Master.pqwSYNC0StartTime     = g_ECController.aqwSYNC0StartTime;
	//
	//g_ECController.Master.w1stSlaveSetAddress       = 1024;
	//g_ECController.Master.dwOutputsLogicalStartAddr = 0x01000000;
	//g_ECController.Master.dwInputsLogicalStartAddr  = 0x02000000;
	//
	//g_ECController.Master.wWatchdogTime = 200; // 100ms.
	////Destination mac
	//ECMasterCfg.dlchr.acDstMAC[0].LBYTE = 0xFF;
	//ECMasterCfg.dlchr.acDstMAC[0].HBYTE = 0xFF;
	//ECMasterCfg.dlchr.acDstMAC[1].LBYTE = 0xFF;
	//ECMasterCfg.dlchr.acDstMAC[1].HBYTE = 0xFF;
	//ECMasterCfg.dlchr.acDstMAC[2].LBYTE = 0xFF;
	//ECMasterCfg.dlchr.acDstMAC[2].HBYTE = 0xFF;
	////Source mac
	//ECMasterCfg.dlchr.acSrcMAC[0].LBYTE = 0x00;
	//ECMasterCfg.dlchr.acSrcMAC[0].HBYTE = 0x0E;
	//ECMasterCfg.dlchr.acSrcMAC[1].LBYTE = 0x00;
	//ECMasterCfg.dlchr.acSrcMAC[1].HBYTE = 0x01;
	//ECMasterCfg.dlchr.acSrcMAC[2].LBYTE = 0x01;
	//ECMasterCfg.dlchr.acSrcMAC[2].HBYTE = 0x02;
	////EtherType
	//ECMasterCfg.dlchr.wProType = PROTOCOL_ETHERCAT;

	ECMasterCfg.acSrcMAC[0].LBYTE         = 0x00;       // Anders 2017-5-24, Modify.
	ECMasterCfg.acSrcMAC[0].HBYTE         = 0x0E;       // Anders 2017-5-24, Modify.
	ECMasterCfg.acSrcMAC[1].LBYTE         = 0x00;       // Anders 2017-5-24, Modify.
	ECMasterCfg.acSrcMAC[1].HBYTE         = 0x01;       // Anders 2017-5-24, Modify.
	ECMasterCfg.acSrcMAC[2].LBYTE         = 0x01;       // Anders 2017-5-24, Modify.
	ECMasterCfg.acSrcMAC[2].HBYTE         = 0x02;       // Anders 2017-5-24, Modify.
	ECMasterCfg.dwOutputsLogicalStartAddr = 0x01000000; // Anders 2017-5-24, Modify.
	ECMasterCfg.dwInputsLogicalStartAddr  = 0x02000000; // Anders 2017-5-24, Modify.
	ECMasterCfg.dwSyncCycleTime_ns        = 2000000;    // 2ms Sync.

	ECMasterCfg.nDeviceHandle = ECMASTERDEVICEHANDLE;
	ECMasterCfg.nSwitchHandle = DEVICEHANDLE_PC;//20201111 add
	ECMasterCfg.pReadNet      = ReadNetData;
	ECMasterCfg.pWriteNet     = ECAT_WriteNet;

	ECMasterCfg.wSlaveCount = ECSLAVE_MAX;
	ECMasterCfg.pECSlaves   = (TECSlave*)&g_ECController.Slave;
	ECMaster_Init(&g_ECController.Master, &ECMasterCfg);
	_SetSwitchPort(ECMasterCfg.nSwitchHandle, &g_ECController.Master.switch_port);

	//AddDnCounter(&u_lSDODelay);
	//AddDnCounter(&u_lConfigDriveMachineState);
	//AddDnCounter(&u_RxPDOCheckTimer);
	//ResetTimer(&u_RxPDOCheckTimer, 0);
	for (i = 0; i < ECSLAVE_MAX; i++)
		g_ECController.Slave[i].wIndex = i;

	return 1;
}

void DestoryTask_EthercatMaster()
{
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
/**
 * Run Ethercat.
 *
 * @author Anders.zhang (2011/9/9)
 *
 */
void RunTask_EthercatMaster()
{
	TECMaster* pMaster = &g_ECController.Master;

	if (!g_wRunECMasterFlag)
		return;

	// THJ 2015-11-13 Add:
	if (ETHCRCAT_CONFIG_SUCCESS != pMaster->wECState)
	{
		if (CheckTimerOut(&u_dwInitCheckCNT))
		{
			if (u_pfcECATMasterInitSlaveError)
				u_pfcECATMasterInitSlaveError();
			Restart_EtherCatMaster();
		}
	}
	else if (g_ECController.bDoClearError) // Anders 2016-6-23, add.
	{
		//for (i = 0; i < 10; i++)
		//  WriteDB_DWORD( cnst_awAxisIndex[i].wMasterCMD_Idx , CMD_RESETCTL );

		if (u_pfcECATMasterDoClearError) // Anders 2017-2-9, Add.
			u_pfcECATMasterDoClearError();
	}

#ifdef INIT_E2C
	if (pMaster->wECState == ETHCRCAT_CONFIG_SUCCESS)
		g_CtlCMD.all++;
	else
		g_CtlCMD.all = 0;
#endif

	// Test for HT.
	//for( i=0; i<HTAXISCARD_AXIS_MAX; i++ )    // Anders 2016-5-20, modify.
	//{
	//    g_HT_AxisCard.Axis[i].wFlow = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL1_SET_FLOW +2*i) * 10;
	//    g_HT_AxisCard.Axis[i].wPress = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL1_SET_PRESS +2*i) * 100;
	//}

	//g_HT_AxisCard.inputs.wSetFlow1  = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL1_SET_FLOW ) * 10;
	//g_HT_AxisCard.inputs.wSetPress1 = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL1_SET_PRESS ) * 100;
	//
	//g_HT_AxisCard.inputs.wSetFlow2  = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL2_SET_FLOW ) * 10;
	//g_HT_AxisCard.inputs.wSetPress2 = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL2_SET_PRESS ) * 100;
	//
	//g_HT_AxisCard.inputs.wSetFlow3  = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL3_SET_FLOW ) * 10;
	//g_HT_AxisCard.inputs.wSetPress3 = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL3_SET_PRESS ) * 100;
	//
	//g_HT_AxisCard.inputs.wSetFlow4  = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL4_SET_FLOW ) * 10;
	//g_HT_AxisCard.inputs.wSetPress4 = DB_GetWORD(DBINDEX_LOCAL0_COMM_MB_CHL4_SET_PRESS ) * 100;

	if (u_pfcECATMasterBeforeRun)
		u_pfcECATMasterBeforeRun();

	ECMaster_Run(pMaster);

	//if (pMaster->wECState != ETHCRCAT_CONFIG_SUCCESS) // Anders 2015-11-12, add.
	//	ResetTimer(&u_RxPDOCheckTimer, EC_RXPDOCHECK_CNT);

	if (pMaster->wECState != ETHCRCAT_CONFIG_SUCCESS)
		return;

	// Test HT.
	//if ( g_HT_AxisCard.wFlow != (g_HT_AxisCard.nMotorSpeed>>1) )
	//    g_HT_AxisCard.wTestPDOErr++;
	//if ( g_HT_AxisCard.wPress != (g_HT_AxisCard.nCurrent>>1) )
	//    g_HT_AxisCard.wTestPDOErr++;
	//if ( g_HT_AxisCard.wControlWord != (g_HT_AxisCard.wStateWord>>1) )
	//    g_HT_AxisCard.wTestPDOErr++;

	if (g_ECController.bDoClearError)
	{
		g_ECController.wDoClearErrorCount++;
		if (g_ECController.wDoClearErrorCount > 100)
		{
			g_ECController.bDoClearError = FALSE;
			//for (i = 0; i < 10; i++)                              // Anders 2016-7-14, modify.
			//  WriteDB_DWORD( cnst_awAxisIndex[i].wMasterCMD_Idx , CMD_STOPCTL );
			if (u_pfcECATMasterAfterClearError) // Anders 2017-2-9, Add.
				u_pfcECATMasterAfterClearError();
		}
	}

	if (g_ECController.bFirstUploadSDODone == FALSE) // Anders 2016-3-8, add.
	{
		if (u_wFirstUpdateReady + 1 == (1 << pMaster->wAppSlaveCount))
			g_ECController.bFirstUploadSDODone = TRUE;
	}
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
void DealSwitch_EthercatMaster(void)
{
    extern WORD u_awEthRxBufTemp[MAX_NETDEVICE][CONST_SIZE_SENDPACKET / 2];

    DEBUG_ASSERT_ILLEGAL(g_ECController.Master.nSwitchHandle < MAX_NETDEVICE, "data error.");
    ECMaster_DealSwitch(&g_ECController.Master, u_awEthRxBufTemp + g_ECController.Master.nSwitchHandle, CONST_SIZE_SENDPACKET);
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
static void ParseNetData(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TECMaster* pMaster = &g_ECController.Master;
	ECMaster_Deal(pMaster, pdata, wlength);
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
void Restart_EtherCatMaster()
{
	int i;
	g_ECController.Master.bReset = TRUE;
	if (g_ECController.Master.wAppSlaveCount == 0)
		g_wRunECMasterFlag = 0;
	else
		g_wRunECMasterFlag = 1;
	//for (i=0; i<g_ECController.Master.wAppSlaveCount; i++ )
	//{
	g_ECController.bDoClearError      = TRUE;
	g_ECController.wDoClearErrorCount = 0;
	//}
	//ResetTimer(&u_RxPDOCheckTimer, EC_RXPDOCHECK_CNT); // Anders 2015-11-12, add.
	ResetTimer(&u_dwInitCheckCNT, EC_INITCHECK_CNT);   // Anders 2015-11-24, add.

	g_ECController.bFirstUploadSDODone = FALSE; // Anders 2016-3-8, Add.
	u_wFirstUpdateReady                = 0;
	for (i = 0; i < g_ECController.Master.wAppSlaveCount; i++)
		Restart_SDOUpload(i);
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
void ECATMasterRx_PDOCheck(void* pvParam)
{
	//u_RxPDOCheckTimer = EC_RXPDOCHECK_CNT;
	//ResetTimer(&u_RxPDOCheckTimer, EC_RXPDOCHECK_CNT);
	g_dwTestRxPDOCNT++;

	//if ( g_ECController.Master.wECState == ETHCRCAT_CONFIG_SUCCESS )
	//{
	//    if (g_HT_AxisCard.wFlow_Tx > 1 && (g_HT_AxisCard.wFlow_Tx -1  != (g_HT_AxisCard.nMotorSpeed>>1)) )
	//        g_HT_AxisCard.wTestPDOErr++;
	//    if (g_HT_AxisCard.wPress_Tx > 2 && (g_HT_AxisCard.wPress_Tx-1 != (g_HT_AxisCard.nCurrent>>1)) )
	//        g_HT_AxisCard.wTestPDOErr++;
	//    if (g_HT_AxisCard.wControlWord_Tx > 3 && (g_HT_AxisCard.wControlWord_Tx-1 != (g_HT_AxisCard.wStateWord>>1)) )
	//        g_HT_AxisCard.wTestPDOErr++;
	//
	//    g_HT_AxisCard.wFlow ++;
	//    if (g_HT_AxisCard.wFlow > 0x7FFF )  // 0x7FFF
	//        g_HT_AxisCard.wFlow = 1;
	//
	//    g_HT_AxisCard.wPress ++;
	//    if (g_HT_AxisCard.wPress > 0x7FFF )
	//        g_HT_AxisCard.wPress = 2;
	//
	//    g_HT_AxisCard.wControlWord ++;
	//    if (g_HT_AxisCard.wControlWord > 0x7FFF )
	//        g_HT_AxisCard.wControlWord = 3;
	//}
}

/*20200622 hankin modify
WORD ECATMasterGetError(void)
{
	WORD wError = g_ECController.Master.wError;

	g_ECController.Master.wError = 0;
	return wError;// CheckTimerOut(&u_RxPDOCheckTimer);
}

WORD ECATMasterGetOfflineIndex(void)
{
	WORD wOfflineScan = g_ECController.Master.wOfflineScan + 1;

	g_ECController.Master.wOfflineScan = (wOfflineScan < g_ECController.Master.wAppSlaveCount) ? wOfflineScan : 0xFFFF;
	return wOfflineScan;
}
*/
//20200701 hankin modify
WORD ECATMasterGetError(void)
{
    if (ETHERCATMASTER_ERROR_NO_ECHO == g_ECController.Master.wError) {
        return g_ECController.Master.wError;
    }
    ++g_wMasterErrorScan;
    if (g_wMasterErrorScan >= g_ECController.Master.wAppSlaveCount) {
        g_wMasterErrorScan = 0;
    }
    return g_ECController.Slave[g_wMasterErrorScan].m_wError; // CheckTimerOut(&u_RxPDOCheckTimer);
}

WORD ECATMasterGetOfflineIndex(void)
{
    if (ETHERCATMASTER_ERROR_NO_ECHO == g_ECController.Master.wError) {
        g_ECController.Master.wError = 0;
        return 0;
    }
    g_ECController.Slave[g_wMasterErrorScan].m_wError = 0;
    return g_wMasterErrorScan + 1;
}

void Restart_SDOUpload(WORD wECSlaveIndex)
{
}

void SetSlaveParamReaderNULL(WORD wECSlaveIdx)
{
	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	g_ECController.awSlaveParamReaderNO[wECSlaveIdx] = 0xFFFF;
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
int GetECSlaveIdxByNodeID(WORD wNodeID)
{
	int i;
	if (wNodeID == 0)
		return -1;
	for (i = 0; i < g_ECController.Master.wAppSlaveCount; i++)
	{
		if (g_ECController.awSlaveNodeID[i] == wNodeID)
			return i;
	}
	return -1;
}
void SetECSlaveNodeID(WORD wECSlaveIdx, WORD wNodeID)
{
	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	g_ECController.awSlaveNodeID[wECSlaveIdx] = wNodeID;
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
void SetAppSlaveCount(WORD wECSlaveCount)
{
	if (wECSlaveCount > ECSLAVE_MAX)
		return;
	g_ECController.Master.wAppSlaveCount = wECSlaveCount;
	if (wECSlaveCount == 0) {
		g_ECSwitchBufferUsed = 0;
		g_ECSwitchBlockUsed = 0;
		memset(&g_ECController.awSlaveNodeID, 0, sizeof(g_ECController.awSlaveNodeID));
	}
	Restart_EtherCatMaster();
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
void Reset_ECSlave(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	int       k;
	TECMaster* pECMaster;
	TECSlave* pECSlave;
	WORD	backup_buffer;
	WORD	backup_block;

	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	pECMaster = &g_ECController.Master;
	pECSlave = &pECMaster->pECSlaves[wECSlaveIdx];

	memset(pECSlave, 0, sizeof(TECSlave));
	pECSlave->m_pECMaster 	= pECMaster;
	pECSlave->wIndex        = wECSlaveIdx;
	pECSlave->bEnableDCSync = bDCSyncEnabled;
	pECSlave->wDeviceType   = wDeviceType;
	for (k = 0; k < PDOMAPING_MAX; k++)
	{
		pECSlave->RxPDOMappingParams[k].bFixed = TRUE;
		pECSlave->RxPDOMappingParams[k].wIndex = 0X1600 + k;

		pECSlave->TxPDOMappingParams[k].bFixed = TRUE;
		pECSlave->TxPDOMappingParams[k].wIndex = 0x1A00 + k;

		// SM Parse.
		pECSlave->RxSmAssign[k].wSM            = 0xFF;
		pECSlave->TxSmAssign[k].wSM            = 0xFF;
		pECSlave->RxPDOMappingParams[k].bySmNO = 0xFF;
		pECSlave->TxPDOMappingParams[k].bySmNO = 0xFF;

		// hankin 20190606 add.
        pECSlave->SmMBoxOut.wSmNO        = 0xFF;
        pECSlave->SmMBoxIn.wSmNO         = 0xFF;
        pECSlave->SmProcessOutputs.wSmNO = 0xFF;
        pECSlave->SmProcessInputs.wSmNO  = 0xFF;
    }
	backup_buffer = g_ECSwitchBufferUsed;
	backup_block = g_ECSwitchBlockUsed;
	switch (wDeviceType)
	{
		case APPDEVICE_E2C:
			if (u_pfcResetECSlave_E2C)
				u_pfcResetECSlave_E2C(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_HYB5TB:
			if (u_pfcResetECSlave_HYB5TB)
				u_pfcResetECSlave_HYB5TB(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_HTJ6_OIL:
			if (u_pfcResetECSlave_HTJ6_Oil)
				u_pfcResetECSlave_HTJ6_Oil(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_HTJ6_ELE_INJ:
			if (u_pfcResetECSlave_HTJ6_EleInj)
				u_pfcResetECSlave_HTJ6_EleInj(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_HTJ6_PLUS:
			if (u_pfcResetECSlave_HTJ6_Plus)
				u_pfcResetECSlave_HTJ6_Plus(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;

		case APPDEVICE_MT_HUB:
			if (u_pfcResetECSlave_MT_HUB_8A)
				u_pfcResetECSlave_MT_HUB_8A(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;

		case APPDEVICE_HT_EH1708X:
			if (u_pfcResetECSlave_HT_EH1708X)
				u_pfcResetECSlave_HT_EH1708X(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;

		case APPDEVICE_BTL6_V11E:
			if (u_pfcResetECSlave_BTL6_V11E)
				u_pfcResetECSlave_BTL6_V11E(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_BTL6_V107:
			if (u_pfcResetECSlave_BTL6_V107)
				u_pfcResetECSlave_BTL6_V107(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_PARKER_COMPAX3_FLUID:
			if (u_pfcResetECSlave_PARKER_COMPAX3_FLUID)
				u_pfcResetECSlave_PARKER_COMPAX3_FLUID(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;

		case APPDEVICE_TR_LINEAR:
			if (u_pfcResetECSlave_TR_LINEAR)
				u_pfcResetECSlave_TR_LINEAR(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_HTJ6_PP:
			if (u_pfcResetECSlave_HTJ6_PP)
				u_pfcResetECSlave_HTJ6_PP(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		case APPDEVICE_REXROTH:
			if (u_pfcResetECSlave_REXROTH)
				u_pfcResetECSlave_REXROTH(wECSlaveIdx, wDeviceIdx, wDeviceType, bDCSyncEnabled);
			break;
		default:
			//g_ECController.Master.wAppSlaveCount = 0;
			// Todo...
			break;
	}
	if (g_ECSwitchBufferUsed > (sizeof(g_ECSwitchBuffer) / sizeof(*g_ECSwitchBuffer))) {
        EASSERT(ALARM, false, ("insufficient resources"));
        pECSlave->Mailbox.EoE.rx_buffer = NULL;
        pECSlave->Mailbox.EoE.tx_buffer = NULL;
        g_ECSwitchBufferUsed            = backup_buffer;
    }
    if (g_ECSwitchBlockUsed > (sizeof(g_ECSwitchBlock) / sizeof(*g_ECSwitchBlock))) {
        EASSERT(ALARM, false, ("insufficient resources"));
        if (NULL != pECSlave->Mailbox.EoE.rx_buffer) {
            ethercat_s_net_buffer_init(pECSlave->Mailbox.EoE.rx_buffer);
        }
        if (NULL != pECSlave->Mailbox.EoE.tx_buffer) {
            ethercat_s_net_buffer_init(pECSlave->Mailbox.EoE.tx_buffer);
        }
        g_ECSwitchBlockUsed = backup_block;
    }
	Restart_EtherCatMaster();
}

void DealECSlaveHandle()
{
	SetAppSlaveCount(0);

	if (u_pfcDealECSlaveHandle_New) // Anders 2017-2-10, add.
		u_pfcDealECSlaveHandle_New();
}
// Anders 2013-8-28,add.
void SetECSlaveClearErrorFlag(WORD wECSlaveIndex)
{
	//int i;
	// if (g_ECController.bDoClearError == FALSE)
	// {
	// 	//if( pAxisController->anECSlaveIndex[i] >= 0 && pAxisController->ErrorController.Entry.info.bObjectID == DEVICE_DANAHER)
	// 	Restart_EtherCatMaster();
	// } //   mask by THJ 20190130

	// for (i=0; i<g_ECController.Master.wAppSlaveCount; i++ )
	// {
	//  g_ECController.awClearErrorFlag[i] = TRUE;
	// }

	g_ECController.bDoClearError      = TRUE;
	g_ECController.wDoClearErrorCount = 0; // Add by CX 20190130
}
void RegCallBack_ECATMasterInitSlaveError(PFC_ECATMaster_Event pfc)
{
	u_pfcECATMasterInitSlaveError = pfc;
}

void RegCallBack_ECATMasterDoClearError(PFC_ECATMaster_Event pfc)
{
	u_pfcECATMasterDoClearError = pfc;
}
void RegCallBack_ECATMasterAfterClearError(PFC_ECATMaster_Event pfc)
{
	u_pfcECATMasterAfterClearError = pfc;
}
void RegCallBack_ECATMasterBeforeRun(PFC_ECATMaster_Event pfc)
{
	u_pfcECATMasterBeforeRun = pfc;
}
void RegCallBack_ResetECSlave_E2C(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_E2C = pfc;
}
void RegCallBack_ResetECSlave_HYB5TB(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_HYB5TB = pfc;
}
void RegCallBack_ResetECSlave_HTJ6_Oil(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_HTJ6_Oil = pfc;
}
void RegCallBack_ResetECSlave_HTJ6_EleInj(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_HTJ6_EleInj = pfc;
}
void RegCallBack_ResetECSlave_HTJ6_Plus(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_HTJ6_Plus = pfc;
}
void RegCallBack_ResetECSlave_MT_HUB_8A(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_MT_HUB_8A = pfc;
}
void RegCallBack_ResetECSlave_HT_EH1708X(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_HT_EH1708X = pfc;
}
void RegCallBack_ResetECSlave_BTL6_V11E(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_BTL6_V11E = pfc;
}
void RegCallBack_ResetECSlave_BTL6_V107(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_BTL6_V107 = pfc;
}
void RegCallBack_ResetECSlave_PARKER_COMPAX3_FLUID(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_PARKER_COMPAX3_FLUID = pfc;
}
void RegCallBack_ResetECSlave_TR_LINEAR(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_TR_LINEAR = pfc;
}
void RegCallBack_ResetECSlave_HTJ6_PP(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_HTJ6_PP = pfc;
}
void RegCallBack_ResetECSlave_REXROTH(PFC_ResetECSlave pfc)
{
	u_pfcResetECSlave_REXROTH = pfc;
}
void RegCallBack_DealECSlaveHandle_New(PFC_ECATMaster_Event pfc)
{
	u_pfcDealECSlaveHandle_New = pfc;
}
