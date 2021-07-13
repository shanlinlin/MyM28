/*==============================================================================+
|  Function :                                                                   |
|  Task     :                                                                   |
|-------------------------------------------------------------------------------|
|  Compile  : CCS 3.3.38.2                                                      |
|  Link     : CCS 3.3.38.2                                                      |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Anders.Zhang                                                      |
|  Version  : V1.00                                                             |
|  Creation : 2017/02/10                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "taskEtherCATMaster_app.h"
#include "CommComp_RemoteIO.h"
#include "Error_App.h"
#include "common.h"
#include "ethercatmaster.h"
#include "ethernet.h"
#include "kernel/tftp/tftp_server.h"
#include "memory_manager.h"
#include "task_ethercatmaster.h"
#include "tasknet.h"
#include "tasknet_master.h"
/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/

/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/
typedef struct map_slave_t {
    TECSlave*    slave;
    TECSlave_FoE FoE;
} map_slave_t;

void map_callback_slave(void* private, tftp_server_t_interface interface, tftp_server_t_event event, void* data);
void map_callback_HTJ6_PP(void* private, tftp_server_t_interface interface, tftp_server_t_event event, void* data);

void ECATMasterInitSlaveError_App(PFC_ECATMaster_Event pfc);
void ECATMasterDoClearError_App();
void ECATMasterAfterClearError_App();
void ECATMasterBeforeRun_App();
void Reset_ECSlave_E2C(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_ECSlave_HYB5TB(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_ECSlave_HTJ6_Oil(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_ECSlave_HTJ6_EleInj(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_ECSlave_HTJ6_Plus(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);

void Reset_MT_HUB_8A(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_HT_EH1708X(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_BTL6_V11E(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled); // hankin 20190603 add.
void Reset_BTL6_V107(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled); // shanll 20200120 add.
void Reset_PARKER_COMPAX3_FLUID(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_TR_LINEAR(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_ECSlave_HTJ6_PP(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);
void Reset_Rexroth_HMC(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled);

void DealECSlaveHandle_New();
/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/
#define Axis1RxMapAddr(D_Index) ((eAxis0 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis0]) : NULL)
#define Axis1TxMapAddr(D_Index) ((eAxis0 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis0]) : NULL)
#define Axis2RxMapAddr(D_Index) ((eAxis1 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis1]) : NULL)
#define Axis2TxMapAddr(D_Index) ((eAxis1 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis1]) : NULL)
#define Axis3RxMapAddr(D_Index) ((eAxis2 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis2]) : NULL)
#define Axis3TxMapAddr(D_Index) ((eAxis2 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis2]) : NULL)
#define Axis4RxMapAddr(D_Index) ((eAxis3 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis3]) : NULL)
#define Axis4TxMapAddr(D_Index) ((eAxis3 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis3]) : NULL)
#define Axis5RxMapAddr(D_Index) ((eAxis4 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis4]) : NULL)
#define Axis5TxMapAddr(D_Index) ((eAxis4 < AXISCARDAXIS_SUM) ? DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[eAxis4]) : NULL)

#define AxisRxMapAddr(Def_eAxis, D_Index) DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[Def_eAxis])
#define AxisTxMapAddr(Def_eAxis, D_Index) DB_GetDataAddr(gc_asAxisDBIndex[D_Index].atIndex[Def_eAxis])
/*==============================================================================+
|           Class declaration -                                                 |
+==============================================================================*/

/*------------------------------------------------------------------------------+
|           Helpers Implementation                                              |
+------------------------------------------------------------------------------*/

/*==============================================================================+
|           Globle Variables                                                    |
+==============================================================================*/
CardConfig g_Card_EtherCATMaster;

HT_AXISCARD_OIL    g_HT_AxisCard_Oil;
HT_AXISCARD_ELEINJ g_HT_AxisCard_EleInj;
HT_AXISCARD_PLUS   g_HT_AxisCard_Plus[2];
HT_AXISCARD_PP     g_HT_AxisCard_PP[3];

DWORD const* u_nMT_HUB_8A_Position;
INT          u_nMT_HUB_8A_SlaveIdx;
WORD         u_wMT_HUB_8A_Error[8]     = { 0, 0, 0, 0, 0, 0, 0, 0 };
WORD         u_wMT_HUB_8A_ErrorFlag[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
WORD         u_wMT_HUB_8A_SDOReadIndex = 0;
WORD		 u_wECATSlaveOPLost = 0;//shanll 20200729 add
bool		 u_bECATNoechoFlag = false;//shanll 20201020 add
/*------------------------------------------------------------------------------+
|           Inner Globle Variable                                               |
+------------------------------------------------------------------------------*/
static IOETH const tftp_server_IOETH = {
    PROTOCOL_IP,
    { { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 } },
    { { IPADDR_0, IPADDR_1 }, { IPADDR_2, IPADDR_3 } },
    { { 0, 0 }, { 0, 0 } },
    69, 0,
    tftp_server_parse
};

/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
void CreatTaskEtherCATMaster_App()
{
	memset(&g_Card_EtherCATMaster, 0, sizeof(g_Card_EtherCATMaster));
	memset(&g_HT_AxisCard_Oil, 0, sizeof(g_HT_AxisCard_Oil));
	memset(&g_HT_AxisCard_EleInj, 0, sizeof(g_HT_AxisCard_EleInj));
	memset(&g_HT_AxisCard_Plus, 0, sizeof(g_HT_AxisCard_Plus));
	memset(&g_HT_AxisCard_PP, 0, sizeof(g_HT_AxisCard_PP));
	u_nMT_HUB_8A_Position = NULL;
	u_nMT_HUB_8A_SlaveIdx = -1;

	RegCallBack_ECATMasterInitSlaveError(ECATMasterInitSlaveError_App);
	RegCallBack_ECATMasterDoClearError(ECATMasterDoClearError_App);
	RegCallBack_ECATMasterAfterClearError(ECATMasterAfterClearError_App);
	RegCallBack_ECATMasterBeforeRun(ECATMasterBeforeRun_App);

	RegCallBack_ResetECSlave_HTJ6_Oil(Reset_ECSlave_HTJ6_Oil);
	RegCallBack_ResetECSlave_HTJ6_EleInj(Reset_ECSlave_HTJ6_EleInj);
	RegCallBack_ResetECSlave_HTJ6_Plus(Reset_ECSlave_HTJ6_Plus);

    RegCallBack_ResetECSlave_MT_HUB_8A(Reset_MT_HUB_8A);                       // Anders 2018-6-15, add.
    RegCallBack_ResetECSlave_HT_EH1708X(Reset_HT_EH1708X);                     // hankin 20190612 add.
    RegCallBack_ResetECSlave_BTL6_V11E(Reset_BTL6_V11E);                       // hankin 20190603 add.
 	RegCallBack_ResetECSlave_BTL6_V107(Reset_BTL6_V107);                       // shanll 20200120 add.
    RegCallBack_ResetECSlave_PARKER_COMPAX3_FLUID(Reset_PARKER_COMPAX3_FLUID); // hankin 20190606 add.
    RegCallBack_ResetECSlave_TR_LINEAR(Reset_TR_LINEAR);                       // hankin 20190801 add.
    RegCallBack_ResetECSlave_HTJ6_PP(Reset_ECSlave_HTJ6_PP);                   // SHANLL 20191219 ADD
	RegCallBack_ResetECSlave_REXROTH(Reset_Rexroth_HMC);                   	   // SHANLL 20201111 add

    RegCallBack_DealECSlaveHandle_New(DealECSlaveHandle_New);

    tftp_server_create(0, &tftp_server_IOETH);
    tftp_server_create(1, &tftp_server_IOETH);
    DealECSlaveHandle();
}
/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/
void map_callback_slave(void* private, tftp_server_t_interface interface, tftp_server_t_event event, void* data)
{
    map_slave_t* map;
    TECSlave*    pECSlave;

    DEBUG_ASSERT_PARAM(NULL != interface, "null pointer.");
    switch (event) {
    case tftp_server_event_read:
    case tftp_server_event_write: {
        tftp_server_t_event_access* access;
        WORD                        index;
        char const*                 file_name;

        DEBUG_ASSERT_PARAM(NULL != data, "null pointer.");
        access = data;
        map    = MALLOC(sizeof(*map));
        if (NULL == map) {
            tftp_server_t_interface_set_error(interface, tftp_error_no_space, "<slave> insufficient memory.");
            break;
        }
        map->slave = NULL;
		tftp_server_t_interface_set_private(interface, map);

        file_name = strchr(access->file_name, ':');
        DEBUG_ASSERT_ILLEGAL(NULL != file_name, "data error.");
        file_name += 1;
        sscanf(file_name, "%u:", &index);
        if (index >= g_ECController.Master.wAppSlaveCount) {
            char const* error_fmt;
            unsigned    size;
            char*       error_msg;

            error_fmt = "slave index(%u) too large, need less count(%u).";
            size      = strlen(error_fmt) + 7;
            error_msg = MALLOC(size);
            if (NULL != error_msg) {
                snprintf(error_msg, size, error_fmt, index, g_ECController.Master.wAppSlaveCount);
                tftp_server_t_interface_set_error(interface, tftp_error_not_found, error_msg);
                FREE(error_msg);
            } else {
                tftp_server_t_interface_set_error(interface, tftp_error_no_space, "<slave> index too large.");
            }
            break;
        }
        pECSlave   = g_ECController.Master.pECSlaves + index;
        map->slave = pECSlave;
        if (0 == tftp_server_t_interface_set_buffer(interface, byte_to_size(pECSlave->SmMBoxOut.wDefaultByteSize))) {
            tftp_server_t_interface_set_error(interface, tftp_error_no_space, "<slave> set_buffer fail.");
            break;
        }
        file_name = strchr(file_name, ':');
        if (NULL == file_name) {
            tftp_server_t_interface_set_error(interface, tftp_error_not_found, "<slave> file_name rule error.");
            DEBUG_LOG(ALARM, ("slave(%u) file_name rule error.", index));
            break;
        }
        file_name += 1;

        ECSlave_FoE_Init(&map->FoE);
        ECSlave_FoE_ParseOptions(&map->FoE, access->options);
        ((tftp_server_event_read == event)
                ? ECSlave_FoE_StartRead
                : ECSlave_FoE_StartWrite)(&map->FoE, pECSlave, interface, file_name);
        break;
    }
    case tftp_server_event_unlock:
        map = tftp_server_t_interface_get_private(interface);
        DEBUG_ASSERT_ILLEGAL(NULL != map, "null pointer.");
        pECSlave = map->slave;
        DEBUG_ASSERT_ILLEGAL(NULL != pECSlave, "null pointer.");
        if (NULL == pECSlave->Mailbox.FoE) {
            tftp_server_t_interface_set_error(interface, tftp_error_no_user, "<slave> free.");
        }
        break;
    case tftp_server_event_close:
        map = tftp_server_t_interface_get_private(interface);
        DEBUG_ASSERT_ILLEGAL(NULL != map, "null pointer.");
        pECSlave = map->slave;
        DEBUG_ASSERT_ILLEGAL(NULL != pECSlave, "null pointer.");
        pECSlave->Mailbox.FoE = NULL;
        FREE(map);
        break;
    default:
        break;
    }
}

void ECATMasterInitSlaveError_App(PFC_ECATMaster_Event pfc)
{
	//shanll_20200227 Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE); // ACTAXIS_INJECT Anders 2017-5-25,modify.
	Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE, 1);
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

void ECATMasterDoClearError_App()
{
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

void ECATMasterAfterClearError_App()
{
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

void ECATMasterBeforeRun_App()
{
	WORD wRst;
	g_HT_AxisCard_Oil.inputs.wSetFlow1  = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_FLOW) * 10;
	g_HT_AxisCard_Oil.inputs.wSetPress1 = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_PRESS) * 100;

	g_HT_AxisCard_Oil.inputs.wSetFlow2  = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL2_SET_FLOW) * 10;
	g_HT_AxisCard_Oil.inputs.wSetPress2 = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL2_SET_PRESS) * 100;

	g_HT_AxisCard_Oil.inputs.wSetFlow3  = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL3_SET_FLOW) * 10;
	g_HT_AxisCard_Oil.inputs.wSetPress3 = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL3_SET_PRESS) * 100;

	g_HT_AxisCard_Oil.inputs.wSetFlow4  = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL4_SET_FLOW) * 10;
	g_HT_AxisCard_Oil.inputs.wSetPress4 = DB_GetWORD(DBINDEX_M28_MONITOR_COMM_MB_CHL4_SET_PRESS) * 100;

	switch (ECATMasterGetError()) {
	case ETHERCATMASTER_ERROR_SLAVE_OFFLINE:
		Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE, ECATMasterGetOfflineIndex());
		break;
	case ETHERCATMASTER_ERROR_NO_ECHO:
		//shanll_20200227 Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE); // ACTAXIS_INJECT Anders 2017-5-25,modify.
		Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE, 1);
		u_bECATNoechoFlag = true;
		Restart_EtherCatMaster();
		break;
	//	shanll 20200630 cancel J6HUB can't connect
	case ETHERCATMASTER_ERROR_SLAVE_AL_ERROR://hankin 20200619 add
		// shanll 20200729 modify
		ECATMasterGetOfflineIndex();
		if(ALSTATE_PREOP == g_ECController.Slave[g_wMasterErrorScan].ALStatus.bit.bState)
			u_wECATSlaveOPLost |= (1<<0);
		else if(ALSTATE_SAFOP == g_ECController.Slave[g_wMasterErrorScan].ALStatus.bit.bState)
			u_wECATSlaveOPLost |= (1<<1);
		//Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE, ECATMasterGetOfflineIndex());
		Restart_EtherCatMaster();
		break;
	default:
		//shanll 20201020 add
		if(u_bECATNoechoFlag&&(g_ECController.Master.wECState==ETHCRCAT_CONFIG_SUCCESS))
			u_bECATNoechoFlag = false;
		else if(u_bECATNoechoFlag)
			Set_CommChipErrorDetail(ACTAXIS_MOLD, ERROR_ID_AXISCARDOFFLINE, 1);
		break;
	}
	if (u_nMT_HUB_8A_SlaveIdx > -1 && g_ECController.Master.wECState == ETHCRCAT_CONFIG_SUCCESS)
	{
		if (u_nMT_HUB_8A_Position[u_wMT_HUB_8A_SDOReadIndex] == 0 || u_nMT_HUB_8A_Position[u_wMT_HUB_8A_SDOReadIndex] == 0xFFFFFFFF)
		{
			if (u_wMT_HUB_8A_ErrorFlag[u_wMT_HUB_8A_SDOReadIndex] == 0)
			{
				wRst = ECMaster_UploadSDO(&g_ECController.Master, u_nMT_HUB_8A_SlaveIdx, 0x6400, u_wMT_HUB_8A_SDOReadIndex + 1, &u_wMT_HUB_8A_Error[u_wMT_HUB_8A_SDOReadIndex], 1, FALSE);

				switch (wRst)
				{
					case SDORESULT_NONE:
						break;
					case SDORESULT_BUSY:
						break;
					case SDORESULT_DONE:
						u_wMT_HUB_8A_ErrorFlag[u_wMT_HUB_8A_SDOReadIndex] = 1;
						Set_MasterError(ACTAXIS_MOLD, APPDEVICE_MT_HUB, ERRORTYPE_WARN, u_wMT_HUB_8A_Error[u_wMT_HUB_8A_SDOReadIndex], 1 + u_wMT_HUB_8A_SDOReadIndex, TRUE);
						DB_SetBYTE(DBINDEX_MT_HUB_8A_WORK_AREA_STATE_CHANNEL1 + u_wMT_HUB_8A_SDOReadIndex, u_wMT_HUB_8A_Error[u_wMT_HUB_8A_SDOReadIndex]); // Anders 2018-7-11.
						if (++u_wMT_HUB_8A_SDOReadIndex >= 8)
							u_wMT_HUB_8A_SDOReadIndex = 0;
						break;
					case SDORESULT_TIMEOUT:
					case SDORESULT_SLAVEABORT:
						if (++u_wMT_HUB_8A_SDOReadIndex >= 8)
							u_wMT_HUB_8A_SDOReadIndex = 0;
					default:
						break;
				}
			}
			else // Anders 2018-7-16, modify.
			{
				if (++u_wMT_HUB_8A_SDOReadIndex >= 8)
					u_wMT_HUB_8A_SDOReadIndex = 0;
			}
		}
		else
		{
			u_wMT_HUB_8A_ErrorFlag[u_wMT_HUB_8A_SDOReadIndex] = 0;
			u_wMT_HUB_8A_Error[u_wMT_HUB_8A_SDOReadIndex]     = 0;
			if (++u_wMT_HUB_8A_SDOReadIndex >= 8)
				u_wMT_HUB_8A_SDOReadIndex = 0;
		}
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

void Reset_ECSlave_HTJ6_Oil(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	TECSlave* pECSlave;
	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	pECSlave = &g_ECController.Master.pECSlaves[wECSlaveIdx];

	pECSlave->bEnableDCSync = TRUE;
	// Init SDO Config
	//pECSlave->InitSDOCfg[0].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[0].wSubIndex = 1;
	//pECSlave->InitSDOCfg[0].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[0].dwData = 1;         // unit.
	//
	//pECSlave->InitSDOCfg[1].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[1].wSubIndex = 2;
	//pECSlave->InitSDOCfg[1].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[1].dwData = 0xFD;    //0xFB;      // 10^-5S.

	// Austone Init Test.
	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1000;
	pECSlave->SmMBoxOut.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1400;
	pECSlave->SmMBoxIn.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;
	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1800;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 8;    // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x24; // 0x64;    // 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 1;

	pECSlave->SmProcessInputs.wStartAddress    = 0x1C00;
	pECSlave->SmProcessInputs.wDefaultByteSize = 10; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;
	/**
     * RxPDO Config.
     */
	pECSlave->RxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[0].wIndex = 0x1600;
	pECSlave->RxPDOMappingParams[0].bySmNO = 2;
	// Velocity Setpoint and ControlWord.
	pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.inputs.ControlWord;

	pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData    = 0x60720010;
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetFlow1;

	pECSlave->RxPDOMappingParams[0].Info[2].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetPress1;

	pECSlave->RxPDOMappingParams[0].Info[3].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wMotionCMD1;

	// Anders 2015-11-5, add.
	pECSlave->RxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[1].wIndex = 0x1601;
	pECSlave->RxPDOMappingParams[1].bySmNO = 2;

	pECSlave->RxPDOMappingParams[1].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[1].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetFlow2;

	pECSlave->RxPDOMappingParams[1].Info[1].dwMappedData    = 0x60720010;
	pECSlave->RxPDOMappingParams[1].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetPress2;

	pECSlave->RxPDOMappingParams[1].Info[2].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[1].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wMotionCMD2;

	pECSlave->RxPDOMappingParams[1].Info[3].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[1].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetFlow3;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[2].wIndex = 0x1602;
	pECSlave->RxPDOMappingParams[2].bySmNO = 2;

	pECSlave->RxPDOMappingParams[2].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[2].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetPress3;

	pECSlave->RxPDOMappingParams[2].Info[1].dwMappedData    = 0x60720010;
	pECSlave->RxPDOMappingParams[2].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wMotionCMD3;

	pECSlave->RxPDOMappingParams[2].Info[2].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[2].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetFlow4;

	pECSlave->RxPDOMappingParams[2].Info[3].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[2].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wSetPress4;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[3].wIndex = 0x1603;
	pECSlave->RxPDOMappingParams[3].bySmNO = 2;

	pECSlave->RxPDOMappingParams[3].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[3].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.inputs.wMotionCMD4;
	/**
     * TxPDO Config.
     */
	// Act Position,Current and StatusWord.
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wStatusWord;

	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActSpeed1;

	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActPress1;

	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdFst1;

	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;

	pECSlave->TxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[1].wIndex = 0x1A01;
	pECSlave->TxPDOMappingParams[1].bySmNO = 3;

	pECSlave->TxPDOMappingParams[1].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[1].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdScd1;

	pECSlave->TxPDOMappingParams[1].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[1].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActSpeed2;

	pECSlave->TxPDOMappingParams[1].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[1].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActPress2;

	pECSlave->TxPDOMappingParams[1].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[1].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdFst2;

	pECSlave->TxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[2].wIndex = 0x1A02;
	pECSlave->TxPDOMappingParams[2].bySmNO = 3;

	pECSlave->TxPDOMappingParams[2].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[2].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdScd2;

	pECSlave->TxPDOMappingParams[2].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[2].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActSpeed3;

	pECSlave->TxPDOMappingParams[2].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[2].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActPress3;

	pECSlave->TxPDOMappingParams[2].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[2].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdFst3;

	pECSlave->TxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[3].wIndex = 0x1A03;
	pECSlave->TxPDOMappingParams[3].bySmNO = 3;

	pECSlave->TxPDOMappingParams[3].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[3].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdScd3;

	pECSlave->TxPDOMappingParams[3].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[3].Info[1].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActSpeed4;

	pECSlave->TxPDOMappingParams[3].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[3].Info[2].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wActPress4;

	pECSlave->TxPDOMappingParams[3].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[3].Info[3].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdFst4;

	pECSlave->TxPDOMappingParams[4].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[4].wIndex = 0x1A04;
	pECSlave->TxPDOMappingParams[4].bySmNO = 3;

	pECSlave->TxPDOMappingParams[4].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[4].Info[0].pMappedVariable = &g_HT_AxisCard_Oil.outputs.wRsvdScd4;
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
void Reset_ECSlave_HTJ6_EleInj(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	TECSlave* pECSlave;
	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	pECSlave = &g_ECController.Master.pECSlaves[wECSlaveIdx];

	pECSlave->bEnableDCSync = TRUE;
	// Init SDO Config
	//pECSlave->InitSDOCfg[0].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[0].wSubIndex = 1;
	//pECSlave->InitSDOCfg[0].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[0].dwData = 1;         // unit.
	//
	//pECSlave->InitSDOCfg[1].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[1].wSubIndex = 2;
	//pECSlave->InitSDOCfg[1].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[1].dwData = 0xFD;    //0xFB;      // 10^-5S.

	// Austone Init Test.
	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1000;
	pECSlave->SmMBoxOut.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1400;
	pECSlave->SmMBoxIn.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;
	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1800;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 8;    // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x24; // 0x64;    // 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 1;

	pECSlave->SmProcessInputs.wStartAddress    = 0x1C00;
	pECSlave->SmProcessInputs.wDefaultByteSize = 10; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;
	/**
     * RxPDO Config.
     */
	pECSlave->RxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[0].wIndex = 0x1600;
	pECSlave->RxPDOMappingParams[0].bySmNO = 2;
	// Velocity Setpoint and ControlWord.
	pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wControlWord;

	pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData    = 0x60720010;
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wMotionCMD;

	pECSlave->RxPDOMappingParams[0].Info[2].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd1;

	pECSlave->RxPDOMappingParams[0].Info[3].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd2;

	// Anders 2015-11-5, add.
	pECSlave->RxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[1].wIndex = 0x1601;
	pECSlave->RxPDOMappingParams[1].bySmNO = 2;

	pECSlave->RxPDOMappingParams[1].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[1].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd3;

	pECSlave->RxPDOMappingParams[1].Info[1].dwMappedData    = 0x60720010;
	pECSlave->RxPDOMappingParams[1].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd4;

	pECSlave->RxPDOMappingParams[1].Info[2].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[1].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd5;

	pECSlave->RxPDOMappingParams[1].Info[3].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[1].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd6;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[2].wIndex = 0x1602;
	pECSlave->RxPDOMappingParams[2].bySmNO = 2;

	pECSlave->RxPDOMappingParams[2].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[2].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd7;

	pECSlave->RxPDOMappingParams[2].Info[1].dwMappedData    = 0x60720010;
	pECSlave->RxPDOMappingParams[2].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd8;

	pECSlave->RxPDOMappingParams[2].Info[2].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[2].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd9;

	pECSlave->RxPDOMappingParams[2].Info[3].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[2].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd10;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[3].wIndex = 0x1603;
	pECSlave->RxPDOMappingParams[3].bySmNO = 2;

	pECSlave->RxPDOMappingParams[3].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[3].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.inputs.wRsvd11;
	/**
     * TxPDO Config.
     */
	// Act Position,Current and StatusWord.
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wStatusWord;

	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjActPosiL;

	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjActPosiH;

	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjActSpeedL;

	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;

	pECSlave->TxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[1].wIndex = 0x1A01;
	pECSlave->TxPDOMappingParams[1].bySmNO = 3;

	pECSlave->TxPDOMappingParams[1].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[1].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjActSpeedH;

	pECSlave->TxPDOMappingParams[1].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[1].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjActTorque;

	pECSlave->TxPDOMappingParams[1].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[1].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjActPress;

	pECSlave->TxPDOMappingParams[1].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[1].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjProfilePosiL;

	pECSlave->TxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[2].wIndex = 0x1A02;
	pECSlave->TxPDOMappingParams[2].bySmNO = 3;

	pECSlave->TxPDOMappingParams[2].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[2].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjProfilePosiH;

	pECSlave->TxPDOMappingParams[2].Info[1].dwMappedData    = 0x60780010;
	pECSlave->TxPDOMappingParams[2].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjPosiErrL;

	pECSlave->TxPDOMappingParams[2].Info[2].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[2].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjPosiErrH;

	pECSlave->TxPDOMappingParams[2].Info[3].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[2].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjProfileVeloL;

	pECSlave->TxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[3].wIndex = 0x1A03;
	pECSlave->TxPDOMappingParams[3].bySmNO = 3;

	pECSlave->TxPDOMappingParams[3].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[3].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wInjProfileVeloH;

	pECSlave->TxPDOMappingParams[3].Info[1].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[3].Info[1].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wMotionTimeL;
	pECSlave->TxPDOMappingParams[3].Info[2].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[3].Info[2].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wMotionTimeH;
	pECSlave->TxPDOMappingParams[3].Info[3].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[3].Info[3].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wChgTorque;

	pECSlave->TxPDOMappingParams[4].bFixed                  = TRUE;
	pECSlave->TxPDOMappingParams[4].wIndex                  = 0x1A04;
	pECSlave->TxPDOMappingParams[4].bySmNO                  = 3;
	pECSlave->TxPDOMappingParams[4].Info[0].dwMappedData    = 0x60640010;
	pECSlave->TxPDOMappingParams[4].Info[0].pMappedVariable = &g_HT_AxisCard_EleInj.outputs.wMotionState;
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
void Reset_ECSlave_HTJ6_Plus(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	TECSlave*         pECSlave;
	HT_AXISCARD_PLUS* pAxisCard_Plus;

	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	if (wDeviceIdx >= (sizeof(g_HT_AxisCard_Plus) / sizeof(*g_HT_AxisCard_Plus))) {
		return;
	}
	pAxisCard_Plus = g_HT_AxisCard_Plus + wDeviceIdx;
	pECSlave = &g_ECController.Master.pECSlaves[wECSlaveIdx];

	pECSlave->bEnableDCSync = TRUE;
	// Init SDO Config
	//pECSlave->InitSDOCfg[0].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[0].wSubIndex = 1;
	//pECSlave->InitSDOCfg[0].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[0].dwData = 1;         // unit.
	//
	//pECSlave->InitSDOCfg[1].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[1].wSubIndex = 2;
	//pECSlave->InitSDOCfg[1].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[1].dwData = 0xFD;    //0xFB;      // 10^-5S.

	// Austone Init Test.
	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1000;
	pECSlave->SmMBoxOut.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1400;
	pECSlave->SmMBoxIn.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;
	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1800;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 8;    // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x24; // 0x64;    // 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 1;

	pECSlave->SmProcessInputs.wStartAddress    = 0x1C00;
	pECSlave->SmProcessInputs.wDefaultByteSize = 10; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;

	// RxPDO Config.
	pECSlave->RxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[0].wIndex = 0x1600;
	pECSlave->RxPDOMappingParams[0].bySmNO = 2;
	// Velocity Setpoint and ControlWord.
	pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = &pAxisCard_Plus->inputs.wControlWord;

	pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = &pAxisCard_Plus->inputs.wInject_Cmd;

	pECSlave->RxPDOMappingParams[0].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[2].pMappedVariable = &pAxisCard_Plus->inputs.wInject_Rsvd1;

	pECSlave->RxPDOMappingParams[0].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[3].pMappedVariable = &pAxisCard_Plus->inputs.wDosing_Rsvd0;

	// Anders 2015-11-5, add.
	pECSlave->RxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[1].wIndex = 0x1601;
	pECSlave->RxPDOMappingParams[1].bySmNO = 2;

	pECSlave->RxPDOMappingParams[1].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[0].pMappedVariable = &pAxisCard_Plus->inputs.wSysCtl_Flow;

	pECSlave->RxPDOMappingParams[1].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[1].pMappedVariable = &pAxisCard_Plus->inputs.wSysCtl_MotionCmd;

	pECSlave->RxPDOMappingParams[1].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[2].pMappedVariable = &pAxisCard_Plus->inputs.wMold_Rsvd0;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[2].wIndex = 0x1602;
	pECSlave->RxPDOMappingParams[2].bySmNO = 2;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[3].wIndex = 0x1603;
	pECSlave->RxPDOMappingParams[3].bySmNO = 2;

	// TxPDO Config.
	// Act Position,Current and StatusWord.
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = &pAxisCard_Plus->outputs.wStatusWord;

	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = &pAxisCard_Plus->outputs.wHP_OutputSpeedH;

	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = &pAxisCard_Plus->outputs.wHP_Rsvd0;

	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = &pAxisCard_Plus->outputs.wHP_Rsvd2;

	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;

	pECSlave->TxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[1].wIndex = 0x1A01;
	pECSlave->TxPDOMappingParams[1].bySmNO = 3;

	pECSlave->TxPDOMappingParams[1].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[0].pMappedVariable = &pAxisCard_Plus->outputs.wHP_Rsvd4;

	pECSlave->TxPDOMappingParams[1].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[1].pMappedVariable = &pAxisCard_Plus->outputs.wInject_PosiH;

	pECSlave->TxPDOMappingParams[1].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[2].pMappedVariable = &pAxisCard_Plus->outputs.wInject_SpeedH;

	pECSlave->TxPDOMappingParams[1].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[3].pMappedVariable = &pAxisCard_Plus->outputs.wInject_Press;

	pECSlave->TxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[2].wIndex = 0x1A02;
	pECSlave->TxPDOMappingParams[2].bySmNO = 3;

	pECSlave->TxPDOMappingParams[2].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[0].pMappedVariable = &pAxisCard_Plus->outputs.wInject_State;

	pECSlave->TxPDOMappingParams[2].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[1].pMappedVariable = &pAxisCard_Plus->outputs.wDosing_SpeedH;

	pECSlave->TxPDOMappingParams[2].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[2].pMappedVariable = &pAxisCard_Plus->outputs.wDosing_Rsvd0;

	pECSlave->TxPDOMappingParams[2].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[3].pMappedVariable = &pAxisCard_Plus->outputs.wDosing_Rsvd2;

	pECSlave->TxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[3].wIndex = 0x1A03;
	pECSlave->TxPDOMappingParams[3].bySmNO = 3;

	pECSlave->TxPDOMappingParams[3].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[0].pMappedVariable = &pAxisCard_Plus->outputs.wDosing_State;

	pECSlave->TxPDOMappingParams[3].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[1].pMappedVariable = &pAxisCard_Plus->outputs.wMold_PosiH;
	pECSlave->TxPDOMappingParams[3].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[2].pMappedVariable = &pAxisCard_Plus->outputs.wMold_SpeedH;
	pECSlave->TxPDOMappingParams[3].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[3].pMappedVariable = &pAxisCard_Plus->outputs.wMold_Rsvd0;

	pECSlave->TxPDOMappingParams[4].bFixed                  = TRUE;
	pECSlave->TxPDOMappingParams[4].wIndex                  = 0x1A04;
	pECSlave->TxPDOMappingParams[4].bySmNO                  = 3;
	pECSlave->TxPDOMappingParams[4].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[0].pMappedVariable = &pAxisCard_Plus->outputs.wMold_State;
}

void map_callback_HTJ6_PP(void* private, tftp_server_t_interface interface, tftp_server_t_event event, void* data)
{
    TECSlave* pECSlave;

    DEBUG_ASSERT_PARAM(NULL != interface, "null pointer.");
    DEBUG_ASSERT_PARAM(NULL != private, "null pointer.");
    pECSlave = private;
    switch (event) {
    case tftp_server_event_read:
    case tftp_server_event_write: {
        tftp_server_t_event_access* access;
        TECSlave_FoE*               FoE;
        char const*                 file_name;

        DEBUG_ASSERT_PARAM(NULL != data, "null pointer.");
        access = data;
        tftp_server_t_interface_set_buffer(interface, byte_to_size(pECSlave->SmMBoxOut.wDefaultByteSize));
        FoE = MALLOC(sizeof(*FoE));
        if (NULL == FoE) {
            tftp_server_t_interface_set_error(interface, tftp_error_no_space, "<J6 PP> open fail.");
            break;
        }
        tftp_server_t_interface_set_private(interface, FoE);

        file_name = strchr(access->file_name, ':');
        DEBUG_ASSERT_ILLEGAL(NULL != file_name, "data error.");
        file_name = strchr(file_name + 1, ':');
        DEBUG_ASSERT_ILLEGAL(NULL != file_name, "data error.");
        file_name += 1;

        ECSlave_FoE_Init(FoE);
        ECSlave_FoE_ParseOptions(FoE, access->options);
        ((tftp_server_event_read == event)
                ? ECSlave_FoE_StartRead
                : ECSlave_FoE_StartWrite)(FoE, pECSlave, interface, file_name);
        break;
    }
    case tftp_server_event_unlock:
        if (NULL == pECSlave->Mailbox.FoE) {
            tftp_server_t_interface_set_error(interface, tftp_error_no_user, "<J6 PP> free.");
        }
        break;
    case tftp_server_event_close:
        data = tftp_server_t_interface_get_private(interface);
        DEBUG_ASSERT_ILLEGAL(NULL != data, "null pointer.");
        pECSlave->Mailbox.FoE = NULL;
        FREE(data);
        break;
    default:
        break;
    }
}

void Reset_ECSlave_HTJ6_PP(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	TECSlave*         pECSlave;
	HT_AXISCARD_PP* pAxisCard_PP;

	if (wECSlaveIdx >= ECSLAVE_MAX)
		return;
	if (wDeviceIdx >= (sizeof(g_HT_AxisCard_PP) / sizeof(*g_HT_AxisCard_PP))) {
		return;
	}
	pAxisCard_PP = g_HT_AxisCard_PP + wDeviceIdx;
	pECSlave = &g_ECController.Master.pECSlaves[wECSlaveIdx];

	pECSlave->bEnableDCSync = TRUE;
	// Init SDO Config
	//pECSlave->InitSDOCfg[0].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[0].wSubIndex = 1;
	//pECSlave->InitSDOCfg[0].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[0].dwData = 1;         // unit.
	//
	//pECSlave->InitSDOCfg[1].wIndex = 0x60C2;    // EtherCAT Cycle time
	//pECSlave->InitSDOCfg[1].wSubIndex = 2;
	//pECSlave->InitSDOCfg[1].wDataBitSize = 8;
	//pECSlave->InitSDOCfg[1].dwData = 0xFD;    //0xFB;      // 10^-5S.

	// Austone Init Test.
	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1000;
	pECSlave->SmMBoxOut.wDefaultByteSize = 192;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1400;
	pECSlave->SmMBoxIn.wDefaultByteSize = 192;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;
	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1800;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 2; // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 1;

	pECSlave->SmProcessInputs.wStartAddress    = 0x1C00;
	pECSlave->SmProcessInputs.wDefaultByteSize = 6; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;

	// RxPDO Config.
	pECSlave->RxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[0].wIndex = 0x1600;
	pECSlave->RxPDOMappingParams[0].bySmNO = 2;
	// Velocity Setpoint and ControlWord.
	pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Control_E;
	pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Inject1_Cmd;
	pECSlave->RxPDOMappingParams[0].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[2].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Inject1_ValveVoltage;
	pECSlave->RxPDOMappingParams[0].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[0].Info[3].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Mold1_Rsvd0;
/*
	pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Control_E;
	pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Control_H;
	pECSlave->RxPDOMappingParams[0].Info[2].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[2].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Inject1_Cmd;
	pECSlave->RxPDOMappingParams[0].Info[3].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[3].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Inject1_Rsvd0;
	pECSlave->RxPDOMappingParams[0].Info[4].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[4].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Inject1_ValveVoltage;
	pECSlave->RxPDOMappingParams[0].Info[5].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[5].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Mold1_Cmd;
	pECSlave->RxPDOMappingParams[0].Info[6].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[6].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Mold1_Rsvd0;
	pECSlave->RxPDOMappingParams[0].Info[7].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[0].Info[7].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Mold1_ValveVoltage;
*/
	// Anders 2015-11-5, add.

	pECSlave->RxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[1].wIndex = 0x1601;
	pECSlave->RxPDOMappingParams[1].bySmNO = 2;
	pECSlave->RxPDOMappingParams[1].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[0].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Press1;
	pECSlave->RxPDOMappingParams[1].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[1].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_SysHydPress_Cmd1;
	pECSlave->RxPDOMappingParams[1].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[2].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Flow2;
	pECSlave->RxPDOMappingParams[1].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[1].Info[3].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Press3;
/*
	pECSlave->RxPDOMappingParams[1].Info[0].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[0].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Press1;
	pECSlave->RxPDOMappingParams[1].Info[1].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[1].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Flow1;
	pECSlave->RxPDOMappingParams[1].Info[2].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[2].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_SysHydPress_Cmd1;
	pECSlave->RxPDOMappingParams[1].Info[3].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[3].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Press2;
	pECSlave->RxPDOMappingParams[1].Info[4].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[4].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Flow2;
	pECSlave->RxPDOMappingParams[1].Info[5].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[5].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_SysHydPress_Cmd2;
	pECSlave->RxPDOMappingParams[1].Info[6].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[6].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Press3;
	pECSlave->RxPDOMappingParams[1].Info[7].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[1].Info[7].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Flow3;
*/
	// Anders 2015-11-5, add.
	pECSlave->RxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[2].wIndex = 0x1602;
	pECSlave->RxPDOMappingParams[2].bySmNO = 2;
	pECSlave->RxPDOMappingParams[2].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[2].Info[0].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_SysHydPress_Cmd3;
	pECSlave->RxPDOMappingParams[2].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[2].Info[1].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Flow4;
	pECSlave->RxPDOMappingParams[2].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[2].Info[2].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd0;
	pECSlave->RxPDOMappingParams[2].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->RxPDOMappingParams[2].Info[3].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd2;
/*
	pECSlave->RxPDOMappingParams[2].Info[0].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[0].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_SysHydPress_Cmd3;
	pECSlave->RxPDOMappingParams[2].Info[1].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[1].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Press4;
	pECSlave->RxPDOMappingParams[2].Info[2].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[2].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_Sys_Flow4;
	pECSlave->RxPDOMappingParams[2].Info[3].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[3].pMappedVariable = &pAxisCard_PP->inputs.wJ6PP_SysHydPress_Cmd4;
	pECSlave->RxPDOMappingParams[2].Info[4].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[4].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd0;
	pECSlave->RxPDOMappingParams[2].Info[5].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[5].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd1;
	pECSlave->RxPDOMappingParams[2].Info[6].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[6].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd2;
	pECSlave->RxPDOMappingParams[2].Info[7].dwMappedData    = 0x00000010;
	pECSlave->RxPDOMappingParams[2].Info[7].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd3;
*/
	pECSlave->RxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[3].wIndex = 0x1603;
	pECSlave->RxPDOMappingParams[3].bySmNO = 2;
	pECSlave->RxPDOMappingParams[3].Info[0].dwMappedData    = 0x00000020;
	pECSlave->RxPDOMappingParams[3].Info[0].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd4;
	//pECSlave->RxPDOMappingParams[3].Info[1].dwMappedData    = 0x00000010;
	//pECSlave->RxPDOMappingParams[3].Info[1].pMappedVariable = &pAxisCard_PP->inputs.J6PP_Rvsd5;

	// TxPDO Config.
	// Act Position,Current and StatusWord.
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_StatusWordL;

	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_PosiL;

	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_SpeedL;

	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_ActPress;

/*
	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_StatusWordL;
	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_StatusWordH;
	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_PosiL;
	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_PosiH;
	pECSlave->TxPDOMappingParams[0].Info[4].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[4].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_SpeedL;
	pECSlave->TxPDOMappingParams[0].Info[5].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[5].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_SpeedH;
	pECSlave->TxPDOMappingParams[0].Info[6].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[6].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_ActPress;
	pECSlave->TxPDOMappingParams[0].Info[7].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[0].Info[7].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_SetPress;
*/
	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;

	pECSlave->TxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[1].wIndex = 0x1A01;
	pECSlave->TxPDOMappingParams[1].bySmNO = 3;

	pECSlave->TxPDOMappingParams[1].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_SetSpeed;

	pECSlave->TxPDOMappingParams[1].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_PosiL;

	pECSlave->TxPDOMappingParams[1].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_SpeedL;

	pECSlave->TxPDOMappingParams[1].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[1].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_Rsvd0;
/*
	pECSlave->TxPDOMappingParams[1].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_SetSpeed;
	pECSlave->TxPDOMappingParams[1].Info[1].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Inject_State;
	pECSlave->TxPDOMappingParams[1].Info[2].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_PosiL;
	pECSlave->TxPDOMappingParams[1].Info[3].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_PosiH;
	pECSlave->TxPDOMappingParams[1].Info[4].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[4].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_SpeedL;
	pECSlave->TxPDOMappingParams[1].Info[5].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[5].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_SpeedH;
	pECSlave->TxPDOMappingParams[1].Info[6].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[6].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_Rsvd0;
	pECSlave->TxPDOMappingParams[1].Info[7].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[1].Info[7].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_ValveOutput;
*/
	pECSlave->TxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[2].wIndex = 0x1A02;
	pECSlave->TxPDOMappingParams[2].bySmNO = 3;

	pECSlave->TxPDOMappingParams[2].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_SetFlow;

	pECSlave->TxPDOMappingParams[2].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion1_Speed;

	pECSlave->TxPDOMappingParams[2].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion2_Speed;

	pECSlave->TxPDOMappingParams[2].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[2].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion3_Speed;
/*
	pECSlave->TxPDOMappingParams[2].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_SetFlow;
	pECSlave->TxPDOMappingParams[2].Info[1].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Mold_State;
	pECSlave->TxPDOMappingParams[2].Info[2].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion1_Speed;
	pECSlave->TxPDOMappingParams[2].Info[3].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion1_Press;
	pECSlave->TxPDOMappingParams[2].Info[4].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[4].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion2_Speed;
	pECSlave->TxPDOMappingParams[2].Info[5].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[5].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion2_Press;
	pECSlave->TxPDOMappingParams[2].Info[6].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[6].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion3_Speed;
	pECSlave->TxPDOMappingParams[2].Info[7].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[2].Info[7].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion3_Press;
*/
	pECSlave->TxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[3].wIndex = 0x1A03;
	pECSlave->TxPDOMappingParams[3].bySmNO = 3;

	pECSlave->TxPDOMappingParams[3].Info[0].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion4_Speed;

	pECSlave->TxPDOMappingParams[3].Info[1].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod1_PosiL;

	pECSlave->TxPDOMappingParams[3].Info[2].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod2_PosiL;

	pECSlave->TxPDOMappingParams[3].Info[3].dwMappedData    = 0x00000020; // Must be 0x20, Anders 2018-9-26, modify.
	pECSlave->TxPDOMappingParams[3].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod3_PosiL;
/*
	pECSlave->TxPDOMappingParams[3].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion4_Speed;
	pECSlave->TxPDOMappingParams[3].Info[1].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Motion4_Press;
	pECSlave->TxPDOMappingParams[3].Info[2].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod1_PosiL;
	pECSlave->TxPDOMappingParams[3].Info[3].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod1_PosiH;
	pECSlave->TxPDOMappingParams[3].Info[4].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[4].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod2_PosiL;
	pECSlave->TxPDOMappingParams[3].Info[5].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[5].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod2_PosiH;
	pECSlave->TxPDOMappingParams[3].Info[6].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[6].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod3_PosiL;
	pECSlave->TxPDOMappingParams[3].Info[7].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[3].Info[7].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod3_PosiH;
*/
	pECSlave->TxPDOMappingParams[4].bFixed                  = TRUE;
	pECSlave->TxPDOMappingParams[4].wIndex                  = 0x1A04;
	pECSlave->TxPDOMappingParams[4].bySmNO                  = 3;

	pECSlave->TxPDOMappingParams[4].Info[0].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[4].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod4_PosiL;

	pECSlave->TxPDOMappingParams[4].Info[1].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[4].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd0;

	pECSlave->TxPDOMappingParams[4].Info[2].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[4].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd2;

	pECSlave->TxPDOMappingParams[4].Info[3].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[4].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd4;
/*
	pECSlave->TxPDOMappingParams[4].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod4_PosiL;
	pECSlave->TxPDOMappingParams[4].Info[1].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rod4_PosiH;
	pECSlave->TxPDOMappingParams[4].Info[2].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd0;
	pECSlave->TxPDOMappingParams[4].Info[3].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd1;
	pECSlave->TxPDOMappingParams[4].Info[4].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[4].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd2;
	pECSlave->TxPDOMappingParams[4].Info[5].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[5].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd3;
	pECSlave->TxPDOMappingParams[4].Info[6].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[6].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd4;
	pECSlave->TxPDOMappingParams[4].Info[7].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[4].Info[7].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd5;
*/
	pECSlave->TxPDOMappingParams[5].bFixed                  = TRUE;
	pECSlave->TxPDOMappingParams[5].wIndex                  = 0x1A05;
	pECSlave->TxPDOMappingParams[5].bySmNO                  = 3;

	pECSlave->TxPDOMappingParams[5].Info[0].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[5].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd6;

	pECSlave->TxPDOMappingParams[5].Info[1].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[5].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd8;

	pECSlave->TxPDOMappingParams[5].Info[2].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[5].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd10;

	pECSlave->TxPDOMappingParams[5].Info[3].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[5].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd12;
/*
	pECSlave->TxPDOMappingParams[5].Info[0].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd6;
	pECSlave->TxPDOMappingParams[5].Info[1].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd7;
	pECSlave->TxPDOMappingParams[5].Info[2].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[2].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd8;
	pECSlave->TxPDOMappingParams[5].Info[3].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[3].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd9;
	pECSlave->TxPDOMappingParams[5].Info[4].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[4].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd10;
	pECSlave->TxPDOMappingParams[5].Info[5].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[5].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd11;
	pECSlave->TxPDOMappingParams[5].Info[6].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[6].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd12;
	pECSlave->TxPDOMappingParams[5].Info[7].dwMappedData    = 0x00000010;
	pECSlave->TxPDOMappingParams[5].Info[7].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd13;
*/

	pECSlave->TxPDOMappingParams[6].bFixed                  = TRUE;
	pECSlave->TxPDOMappingParams[6].wIndex                  = 0x1A06;
	pECSlave->TxPDOMappingParams[6].bySmNO                  = 3;

	pECSlave->TxPDOMappingParams[6].Info[0].dwMappedData    = 0x00000020;
	pECSlave->TxPDOMappingParams[6].Info[0].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd14;
	//pECSlave->TxPDOMappingParams[6].Info[1].dwMappedData    = 0x00000010;
	//pECSlave->TxPDOMappingParams[6].Info[1].pMappedVariable = &pAxisCard_PP->outputs.wJ6PP_Rsvd15;
	{
        char*    key;
        unsigned len;

        len = 8;
        key = MALLOC(len);
        if (NULL != key) {
            snprintf(key, len, "j6:%u:", wDeviceIdx);
            tftp_server_map(key, pECSlave, map_callback_HTJ6_PP);
        }
        FREE(key);
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
void Reset_MT_HUB_8A(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	TECSlave*       pECSlave;
	S_CommRemoteIO* pRemoteIO;

	pECSlave  = &g_ECController.Master.pECSlaves[wECSlaveIdx];
	pRemoteIO = &g_asCommRemoteIO[wDeviceIdx];

	pECSlave->bEnableDCSync = bDCSyncEnabled;

	// MT_HUB_8A Init.
	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1000;
	pECSlave->SmMBoxOut.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1080;
	pECSlave->SmMBoxIn.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;
	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1100;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 0;    // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x64; // 0x64;    // 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 0;    // SyncMan disabled.

	pECSlave->SmProcessInputs.wStartAddress    = 0x1400;
	pECSlave->SmProcessInputs.wDefaultByteSize = 32; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;

	// RxPDO Config.
	pECSlave->RxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[0].wIndex = 0x1600;
	pECSlave->RxPDOMappingParams[0].bySmNO = 2;

	// Anders 2015-11-5, add.
	pECSlave->RxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[1].wIndex = 0x1601;
	pECSlave->RxPDOMappingParams[1].bySmNO = 2;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[2].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[2].wIndex = 0x1602;
	pECSlave->RxPDOMappingParams[2].bySmNO = 2;

	// Anders 2015-6-1, add.
	pECSlave->RxPDOMappingParams[3].bFixed = TRUE;
	pECSlave->RxPDOMappingParams[3].wIndex = 0x1603;
	pECSlave->RxPDOMappingParams[3].bySmNO = 2;

	// TxPDO Config.
	// Act Position
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = &pRemoteIO->_auRecvedPDO[0].sU32.bData0;
	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;

	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = &pRemoteIO->_auRecvedPDO[0].sU32.bData1;

	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = &pRemoteIO->_auRecvedPDO[1].sU32.bData0;

	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = &pRemoteIO->_auRecvedPDO[1].sU32.bData1;

	pECSlave->TxPDOMappingParams[1].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[1].wIndex = 0x1A01;
	pECSlave->TxPDOMappingParams[1].bySmNO = 3;

	pECSlave->TxPDOMappingParams[1].Info[0].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[1].Info[0].pMappedVariable = &pRemoteIO->_auRecvedPDO[2].sU32.bData0;

	pECSlave->TxPDOMappingParams[1].Info[1].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[1].Info[1].pMappedVariable = &pRemoteIO->_auRecvedPDO[2].sU32.bData1;

	pECSlave->TxPDOMappingParams[1].Info[2].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[1].Info[2].pMappedVariable = &pRemoteIO->_auRecvedPDO[3].sU32.bData0;

	pECSlave->TxPDOMappingParams[1].Info[3].dwMappedData    = 0x20000020;
	pECSlave->TxPDOMappingParams[1].Info[3].pMappedVariable = &pRemoteIO->_auRecvedPDO[3].sU32.bData1;
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
void Reset_HT_EH1708X(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
    TECSlave* pECSlave;

    pECSlave = &g_ECController.Master.pECSlaves[wECSlaveIdx];

    pECSlave->bEnableDCSync = bDCSyncEnabled;
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
void ECATMasterRx_BTL6_V11E(void* pvParam)
{
    CANOpenDataFrameData_U* pData = pvParam;

    ECATMasterRx_PDOCheck(pvParam);
    if (0x10 != pData[1].sU8.bData2) {
        pData[0].sU32.bData0 = (DWORD)-1;
        return;
    }
    pData[0].sU8.bData0 = pData[1].sU8.bData3;
    pData[0].sU8.bData1 = pData[1].sU8.bData4;
    pData[0].sU8.bData2 = pData[1].sU8.bData5;
    pData[0].sU8.bData3 = pData[1].sU8.bData6;
}

void Reset_BTL6_V11E(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
    TECSlave*       pECSlave;
    S_CommRemoteIO* pRemoteIO;

    // BTL6_V11E Init.
    pECSlave  = &g_ECController.Master.pECSlaves[wECSlaveIdx];
    pRemoteIO = &g_asCommRemoteIO[wDeviceIdx];

    pECSlave->bEnableDCSync = bDCSyncEnabled;

    pECSlave->SmMBoxOut.wSmNO = 3;

    // SM PDO
    pECSlave->SmProcessInputs.wStartAddress    = 0x1000;
    pECSlave->SmProcessInputs.wDefaultByteSize = 0x04; // Bytes.
    pECSlave->SmProcessInputs.byControlByte    = 0x00;
    pECSlave->SmProcessInputs.bEnable          = 1;
    pECSlave->SmProcessInputs.wSmNO            = 0;

    // TxPDO Config.
    // Act Position
    // TxPDO1
    pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
    pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
    pECSlave->TxPDOMappingParams[0].bySmNO = 0;

    pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData         = 0x00000038;
    pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable      = pRemoteIO->_auRecvedPDO + 1;
    pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc      = ECATMasterRx_BTL6_V11E;
    pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFuncParam = pRemoteIO->_auRecvedPDO + 0;
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
void ECATMasterRx_BTL6_V107(void* pvParam)
{
    CANOpenDataFrameData_U* pData = pvParam;

    ECATMasterRx_PDOCheck(pvParam);
    if (0x10 != pData[1].sU8.bData2) {
        pData[0].sU32.bData0 = (DWORD)-1;
        return;
    }
    pData[0].sU8.bData0 = pData[1].sU8.bData3;
    pData[0].sU8.bData1 = pData[1].sU8.bData4;
    pData[0].sU8.bData2 = pData[1].sU8.bData5;
    pData[0].sU8.bData3 = pData[1].sU8.bData6;
}

void Reset_BTL6_V107(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
    TECSlave*       pECSlave;
    S_CommRemoteIO* pRemoteIO;

    // BTL6_V107 Init.
    pECSlave  = &g_ECController.Master.pECSlaves[wECSlaveIdx];
    pRemoteIO = &g_asCommRemoteIO[wDeviceIdx];

    pECSlave->bEnableDCSync = bDCSyncEnabled;

    pECSlave->SmMBoxOut.wSmNO = 3;

    // SM PDO
    pECSlave->SmProcessInputs.wStartAddress    = 0x1000;
    pECSlave->SmProcessInputs.wDefaultByteSize = 0x20; // Bytes.
    pECSlave->SmProcessInputs.byControlByte    = 0x00;
    pECSlave->SmProcessInputs.bEnable          = 1;
    pECSlave->SmProcessInputs.wSmNO            = 0;

    // TxPDO Config.
    // Act Position
    // TxPDO1
    pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
    pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
    pECSlave->TxPDOMappingParams[0].bySmNO = 0;

    pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData         = 0x00000058;
    pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable      = pRemoteIO->_auRecvedPDO + 1;
    pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc      = ECATMasterRx_BTL6_V107;
    pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFuncParam = pRemoteIO->_auRecvedPDO + 0;
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
void Reset_PARKER_COMPAX3_FLUID(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
	TECSlave* pECSlave;
	DBIndex_T index;

	if (wECSlaveIdx >= ECSLAVE_MAX) {
		return;
	}
	pECSlave = &g_ECController.Master.pECSlaves[wECSlaveIdx];
	index  = DBINDEX_PARKER_COMPAX3_FLUID1_CONTRLWORD2 + wDeviceIdx * DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM;

	pECSlave->bEnableDCSync = FALSE;

	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1300;
	pECSlave->SmMBoxOut.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1380;
	pECSlave->SmMBoxIn.wDefaultByteSize = 128; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;
	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1000;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 30;   // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x24; // 0x64;    // 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 1;

	pECSlave->SmProcessInputs.wStartAddress    = 0x1180;
	pECSlave->SmProcessInputs.wDefaultByteSize = 22; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;

	pECSlave->bRxPDONotAssign = TRUE;
	pECSlave->bTxPDONotAssign = TRUE;

	// RxPDO Config.
	pECSlave->RxPDOMappingParams[0].bFixed = FALSE;
	pECSlave->RxPDOMappingParams[0].wIndex = 0x1600;
	pECSlave->RxPDOMappingParams[0].bySmNO = 2;

	pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData    = 0x60400010;
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_CONTRLWORD);

	pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData    = 0x201B0010;
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_CONTRLWORD2);

	// TxPDO Config.
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = FALSE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x60410010;
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_STATUSWORD);
	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;

	pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x201C0010;
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_STATUSWORD2);

	pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x606C0020;
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ACTUAL_SPEED_CLAMP);

	pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x21040020;
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ACTUAL_POSITION_CLAMP);

	pECSlave->TxPDOMappingParams[0].Info[4].dwMappedData    = 0x686C0020;
	pECSlave->TxPDOMappingParams[0].Info[4].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ACTUAL_SPEED_INJECTION);

	pECSlave->TxPDOMappingParams[0].Info[5].dwMappedData    = 0x21060020;
	pECSlave->TxPDOMappingParams[0].Info[5].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ACTUAL_POSITION_INJECTION);

	pECSlave->TxPDOMappingParams[0].Info[6].dwMappedData    = 0x25010120;
	pECSlave->TxPDOMappingParams[0].Info[6].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ACTUAL_PRESSURE_PA);

	pECSlave->TxPDOMappingParams[0].Info[7].dwMappedData    = 0x25010220;
	pECSlave->TxPDOMappingParams[0].Info[7].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ACTUAL_PRESSURE_PB);

	pECSlave->TxPDOMappingParams[0].Info[8].dwMappedData    = 0x603F0010;
	pECSlave->TxPDOMappingParams[0].Info[8].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_PARKER_COMPAX3_FLUID_ERROR_HISTORY_LAST_ERROR);
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
void ECATMasterRx_TR_LINEAR(void* pvParam)
{
	CANOpenDataFrameData_U* pData = pvParam;

	ECATMasterRx_PDOCheck(pvParam);
	if (1 == pData[1].sU8.bData0) {
		pData[0].sU32.bData0 = (DWORD)-1;
		return;
	}
	pData[0].sU8.bData0 = pData[1].sU8.bData1;
	pData[0].sU8.bData1 = pData[1].sU8.bData2;
	pData[0].sU8.bData2 = pData[1].sU8.bData3;
	pData[0].sU8.bData3 = pData[1].sU8.bData4;
}

void Reset_TR_LINEAR(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
    TECSlave*       pECSlave;
    S_CommRemoteIO* pRemoteIO;

    // TR_LINEAR Init.
	pECSlave  = &g_ECController.Master.pECSlaves[wECSlaveIdx];
	pRemoteIO = &g_asCommRemoteIO[wDeviceIdx];

	pECSlave->bEnableDCSync = bDCSyncEnabled;

	// SM MBox.
	pECSlave->SmMBoxOut.wStartAddress    = 0x1800;
	pECSlave->SmMBoxOut.wDefaultByteSize = 192; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxOut.byControlByte    = 0x26;
	pECSlave->SmMBoxOut.bEnable          = 1;

	pECSlave->SmMBoxIn.wStartAddress    = 0x1C00;
	pECSlave->SmMBoxIn.wDefaultByteSize = 192; // Anders 2016-6-14 Modify. 128; //512;
	pECSlave->SmMBoxIn.byControlByte    = 0x22;
	pECSlave->SmMBoxIn.bEnable          = 1;

	// SM PDO
	pECSlave->SmProcessOutputs.wStartAddress    = 0x1000;
	pECSlave->SmProcessOutputs.wDefaultByteSize = 0;    // Bytes.
	pECSlave->SmProcessOutputs.byControlByte    = 0x24; // 0x64;    // 0x24;
	pECSlave->SmProcessOutputs.bEnable          = 0;

	// SM PDO
	pECSlave->SmProcessInputs.wStartAddress    = 0x1100;
	pECSlave->SmProcessInputs.wDefaultByteSize = 0x07; // Bytes.
	pECSlave->SmProcessInputs.byControlByte    = 0x20;
	pECSlave->SmProcessInputs.bEnable          = 1;

	// TxPDO Config.
	// Act Position
	// TxPDO1
	pECSlave->TxPDOMappingParams[0].bFixed = TRUE;
	pECSlave->TxPDOMappingParams[0].wIndex = 0x1A00;
	pECSlave->TxPDOMappingParams[0].bySmNO = 3;

	pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData         = 0x30000048;//Shanll 20200120 0x30000038->0x30000048
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable      = pRemoteIO->_auRecvedPDO + 1;
	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc      = &ECATMasterRx_TR_LINEAR;
	pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFuncParam = pRemoteIO->_auRecvedPDO + 0;
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

void DealECSlaveHandle_New()
{
	int  i, nECSlaveIdx;

    tftp_server_map_clear();
    tftp_server_map("slave:", NULL, map_callback_slave);

    u_nMT_HUB_8A_Position = NULL;
    u_nMT_HUB_8A_SlaveIdx = -1;
    if (g_Card_EtherCATMaster.bit.b15) {
        WORD wHTJ6_OIL_count    = 0;
        WORD wHTJ6_ELEINJ_count = 0;
        WORD wHTJ6_PLUS_count   = 0;
        WORD wHTJ6_PP_count   	= 0;
        WORD parker_count       = 0;
		WORD Rexroth_count      = 0;


        for (i = 0; i < REMOTEIONUM_SUM; ++i) {
        	if(COMMTYPE_SUM != g_asCommRemoteIO[i]._sComp.PRI_eCommType)	//hankin 20200309
				continue;
            nECSlaveIdx = g_ECController.Master.wAppSlaveCount;
            switch (g_asCommRemoteIO[i]._eDeviceType) {
            case REMOTEIODEVICETYPE_HTJ6_OIL:
                Reset_ECSlave(nECSlaveIdx, wHTJ6_OIL_count, APPDEVICE_HTJ6_OIL, TRUE);
                ++wHTJ6_OIL_count;
                break;
            case REMOTEIODEVICETYPE_HTJ6_ELEINJ:
                Reset_ECSlave(nECSlaveIdx, wHTJ6_ELEINJ_count, APPDEVICE_HTJ6_ELE_INJ, TRUE);
                ++wHTJ6_ELEINJ_count;
                break;
            case REMOTEIODEVICETYPE_HTJ6_PLUS:
                Reset_ECSlave(nECSlaveIdx, wHTJ6_PLUS_count, APPDEVICE_HTJ6_PLUS, TRUE);
                ++wHTJ6_PLUS_count;
                break;
            case REMOTEIODEVICETYPE_MT_HUB_8A:
                Reset_ECSlave(nECSlaveIdx, i, APPDEVICE_MT_HUB, TRUE);
                u_nMT_HUB_8A_Position = &g_asCommRemoteIO[i]._auRecvedPDO[0].sU32.bData0;
                u_nMT_HUB_8A_SlaveIdx = nECSlaveIdx;
                break;
            case REMOTEIODEVICETYPE_HT_EH1708X:
                Reset_ECSlave(nECSlaveIdx, 0, APPDEVICE_HT_EH1708X, TRUE);
                break;
            case REMOTEIODEVICETYPE_BTL6_V11E:
                Reset_ECSlave(nECSlaveIdx, i, APPDEVICE_BTL6_V11E, FALSE);
                break;
            case REMOTEIODEVICETYPE_BTL6_V107:
                Reset_ECSlave(nECSlaveIdx, i, APPDEVICE_BTL6_V107, FALSE);
                break;
            case REMOTEIODEVICETYPE_PARKER_COMPAX3_FLUID:
                Reset_ECSlave(nECSlaveIdx, parker_count, APPDEVICE_PARKER_COMPAX3_FLUID, FALSE);
                ++parker_count;
                break;
            case REMOTEIODEVICETYPE_TR_LINEAR:
                Reset_ECSlave(nECSlaveIdx, i, APPDEVICE_TR_LINEAR, FALSE);
                break;
            case REMOTEIODEVICETYPE_HTJ6_PP:
                Reset_ECSlave(nECSlaveIdx, wHTJ6_PP_count, APPDEVICE_HTJ6_PP, TRUE);
                ++wHTJ6_PP_count;
                break;
			case REMOTEIODEVICETYPE_REXROTH:
				Reset_ECSlave(nECSlaveIdx, Rexroth_count, APPDEVICE_REXROTH, TRUE);
				++Rexroth_count;
				break;
            default:
                continue;
            }
            SetECSlaveNodeID(nECSlaveIdx, g_asCommRemoteIO[i]._sCANOpenUnique.PRO_bNodeID);
            SetAppSlaveCount(nECSlaveIdx + 1);
            SetSlaveParamReaderNULL(nECSlaveIdx);
        }
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
void RefreshMT_HUB_Error()
{
	u_wMT_HUB_8A_SDOReadIndex = 0;
	memset(&u_wMT_HUB_8A_Error, 0, sizeof(u_wMT_HUB_8A_Error));
	memset(&u_wMT_HUB_8A_ErrorFlag, 0, sizeof(u_wMT_HUB_8A_ErrorFlag));
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

void Reset_Rexroth_HMC(WORD wECSlaveIdx, WORD wDeviceIdx, WORD wDeviceType, BOOL bDCSyncEnabled)
{
    TECMaster*  pECMaster;
    TECSlave*   pECSlave;
    char const* cst_dns_name;
    char        dns_name[18];
    unsigned    block_count;
	DBIndex_T	index;

    // DBIndex_T  index;

    if (wECSlaveIdx >= ECSLAVE_MAX) {
        return;
    }
    pECMaster = &g_ECController.Master;
    pECSlave  = pECMaster->pECSlaves + wECSlaveIdx;

    pECSlave->bEnableDCSync = true;
	index  = DBINDEX_REXROTH_HYD_STATUSWORD + wDeviceIdx * DBGROUPINDEX_REXROTH_SUM;

    // SM MBox.
    pECSlave->SmMBoxOut.wStartAddress    = 0x1800;
    pECSlave->SmMBoxOut.wDefaultByteSize = 234;
    pECSlave->SmMBoxOut.byControlByte    = 0x26;
    pECSlave->SmMBoxOut.bEnable          = 1;

    pECSlave->SmMBoxIn.wStartAddress    = 0x1A00;
    pECSlave->SmMBoxIn.wDefaultByteSize = 234;
    pECSlave->SmMBoxIn.byControlByte    = 0x22;
    pECSlave->SmMBoxIn.bEnable          = 1;
    // SM PDO
    pECSlave->SmProcessOutputs.wStartAddress    = 0x1000;
    pECSlave->SmProcessOutputs.wDefaultByteSize = 0;    // Bytes.
    pECSlave->SmProcessOutputs.byControlByte    = 0x64; // 0x64;    // 0x24;
    pECSlave->SmProcessOutputs.bEnable          = 1;

    pECSlave->SmProcessInputs.wStartAddress    = 0x1400;
    pECSlave->SmProcessInputs.wDefaultByteSize = 0; // Bytes.
    pECSlave->SmProcessInputs.byControlByte    = 0x62;
    pECSlave->SmProcessInputs.bEnable          = 1;

    // EoE.
    ethernet_s_mac_set_value(&pECSlave->Mailbox.EoE.VirtualMAC,
        0x02, 0x01, 0x05, 0x20, 0x03, 0x0E + wECSlaveIdx);
    pECSlave->Mailbox.EoE.ConfigFlag.MacAddrSetted = true;
    ethernet_s_ip4_set_value(&pECSlave->Mailbox.EoE.IpParameter.IpCfg.IPAddress,
        169, 254, 3, 233 + wECSlaveIdx);
    pECSlave->Mailbox.EoE.ConfigFlag.IpAddrSetted = true;
    ethernet_s_ip4_set_value(&pECSlave->Mailbox.EoE.IpParameter.IpCfg.SubnetMask,
        255, 255, 255, 0);
    pECSlave->Mailbox.EoE.ConfigFlag.SubnetMaskSetted = true;
    //ethernet_s_ip4_set_value(&pECSlave->Mailbox.EoE.IpParameter.IpCfg.DefaultGateway,
    //    169, 254, 3, 1);
    //pECSlave->Mailbox.EoE.ConfigFlag.DefaultGatewaySetted = true;
    //ethernet_s_ip4_set_value(&pECSlave->Mailbox.EoE.IpParameter.IpCfg.DnsServer,
    //    0, 0, 0, 0);
    //pECSlave->Mailbox.EoE.ConfigFlag.DnsServerSetted = true;
    cst_dns_name = "Drive_1_Hydrau(0)";
    EASSERT(ALARM, sizeof(dns_name) > strlen(cst_dns_name), ("dns_name too small.\n"));
    memcpy(dns_name, cst_dns_name, sizeof(dns_name));
    dns_name[strlen(cst_dns_name) - 2] += wECSlaveIdx;
    ECMaster_SetDnsName(pECSlave, dns_name);
    pECSlave->Mailbox.EoE.ConfigFlag.DnsNameSetted = true;

    pECSlave->Mailbox.EoE.rx_buffer = g_ECSwitchBuffer + g_ECSwitchBufferUsed++;
    ethercat_s_net_buffer_init(pECSlave->Mailbox.EoE.rx_buffer);
    pECSlave->Mailbox.EoE.tx_buffer = g_ECSwitchBuffer + g_ECSwitchBufferUsed++;
    ethercat_s_net_buffer_init(pECSlave->Mailbox.EoE.tx_buffer);
    block_count = ETHERNET_FRAME_BYTE / CONST_ETHERCAT_EOE_BLOCK_BYTE;
    ethercat_s_net_buffer_init_fifo(pECSlave->Mailbox.EoE.tx_buffer, g_ECSwitchBlock + g_ECSwitchBlockUsed, block_count);
    g_ECSwitchBlockUsed += block_count;
    pECSlave->Mailbox.EoE.tx_interval_ms = 2;

    pECSlave->Mailbox.EoE.ConfigFlag.bEnable = true;

    // RxPDO.
    pECSlave->RxPDOMappingParams[0].bFixed               = false;
    pECSlave->RxPDOMappingParams[0].wIndex               = 0x1600;
    pECSlave->RxPDOMappingParams[0].bySmNO               = 2;
    pECSlave->RxPDOMappingParams[0].Info[0].dwMappedData = 0x356E0110;
	pECSlave->RxPDOMappingParams[0].Info[0].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_HYD_CONTROLWORD);
    pECSlave->RxPDOMappingParams[0].Info[1].dwMappedData = 0x356F0110;
	pECSlave->RxPDOMappingParams[0].Info[1].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_INJECTION_CONTROLWORD);
    pECSlave->RxPDOMappingParams[0].Info[2].dwMappedData = 0x35700110;
	pECSlave->RxPDOMappingParams[0].Info[2].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_RESERVE);
    pECSlave->RxPDOMappingParams[0].Info[3].dwMappedData = 0x35710110;
	pECSlave->RxPDOMappingParams[0].Info[3].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_CMD_4WRVE_NG16);
    pECSlave->RxPDOMappingParams[0].Info[4].dwMappedData = 0x35720110;
	pECSlave->RxPDOMappingParams[0].Info[4].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_CMD_2WRCE_NG40);
    pECSlave->RxPDOMappingParams[0].Info[5].dwMappedData = 0x35730110;
	pECSlave->RxPDOMappingParams[0].Info[5].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_CMD_2WRCE_NG32);
    pECSlave->RxPDOMappingParams[0].Info[6].dwMappedData = 0x35740110;
	pECSlave->RxPDOMappingParams[0].Info[6].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_PUMP_ACTUAL_PRESS);
    pECSlave->RxPDOMappingParams[0].Info[7].dwMappedData = 0x35750110;
	pECSlave->RxPDOMappingParams[0].Info[7].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_ACU_ACTUAL_PRESS);

    // TxPDO.
    pECSlave->TxPDOMappingParams[0].bFixed                  = false;
    pECSlave->TxPDOMappingParams[0].wIndex                  = 0x1A00;
    pECSlave->TxPDOMappingParams[0].bySmNO                  = 3;
    pECSlave->TxPDOMappingParams[0].Info[0].dwMappedData    = 0x35820110;
	pECSlave->TxPDOMappingParams[0].Info[0].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_HYD_STATUSWORD);
    pECSlave->TxPDOMappingParams[0].Info[0].pAfterCommuFunc = &ECATMasterRx_PDOCheck;
    pECSlave->TxPDOMappingParams[0].Info[1].dwMappedData    = 0x21820120;
	pECSlave->TxPDOMappingParams[0].Info[1].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_ACTUAL_POSITION);
    pECSlave->TxPDOMappingParams[0].Info[2].dwMappedData    = 0x22170120;
	pECSlave->TxPDOMappingParams[0].Info[2].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_ACTUAL_VELOCITY);
    pECSlave->TxPDOMappingParams[0].Info[3].dwMappedData    = 0x35840110;
	pECSlave->TxPDOMappingParams[0].Info[3].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_ACTUAL_PRESS_P11);
    pECSlave->TxPDOMappingParams[0].Info[4].dwMappedData    = 0x35850110;
	pECSlave->TxPDOMappingParams[0].Info[4].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_ACTUAL_PRESS_P12);
    pECSlave->TxPDOMappingParams[0].Info[5].dwMappedData    = 0x35830110;
	pECSlave->TxPDOMappingParams[0].Info[5].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_INJECTION_STATUSWORD);
    pECSlave->TxPDOMappingParams[0].Info[6].dwMappedData    = 0x35860110;
	pECSlave->TxPDOMappingParams[0].Info[6].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_PLASFICATION_PRESS);
    pECSlave->TxPDOMappingParams[0].Info[7].dwMappedData    = 0x35150120;
	pECSlave->TxPDOMappingParams[0].Info[7].pMappedVariable = DB_GetDataAddr(index + DBGROUPINDEX_REXROTH_ERROR_CODE);

    pECSlave->bRxPDONotAssign = true;
    pECSlave->bTxPDONotAssign = true;
}
/*------------------------------------------------------------------------------+
|           Helpers                                                             |
+------------------------------------------------------------------------------*/
