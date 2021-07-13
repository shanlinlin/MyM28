/*==============================================================================+
|  Function : Application Error                                                 |
|  Task     : Application Error Source File                                     |
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
#include "Error_App.h"
#include "tasknet_master.h"
#include "database.h"
#include "task_ethercatmaster.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
#define Error_App__CHECK_DELAY 5000
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
typedef void (*Error_App_T_Task)(void);
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static void Error_App__TaskInit(void);
static void Error_App__TaskRun(void);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
Error_App_T_Task Error_App__task;
Ttimer           Error_App__checkTimer;

const WORD cnst_awMotionAxis2DataAxis[] = {
	1, // Mold
	6, // Mold2
	2, // Eject
	7, // Eject2
	3, // Inject
	8, // inject2
	4, // Charge
	9, // Charge2
	5, // Carriage
	10 // Carriage2
};
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Error_App_Init description]
 */
void Error_App_Init(void)
{
	Error_App_Reset();
}

/**
 * [Error_App_Run description]
 */
void Error_App_Run(void)
{
	Error_App__task();
}

/**
 * [Error_App_Reset description]
 */
void Error_App_Reset(void)
{
	Error_App__task = Error_App__TaskInit;
	ResetTimer(&Error_App__checkTimer, Error_App__CHECK_DELAY);
}

/**
 * [Get_MasterError description]
 * @author hankin (2019/07/26)
 * @param  eAxisIndex [description]
 * @return            [description]
 */
DeviceError Get_MasterError(ACTAXIS eAxisIndex)
{
	DeviceError error;

	error.dwAll = 0;
	if (eAxisIndex > ACTAXIS_NOZZLE2)
		return error;

	error.dwAll = DB_GetDWORD(DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR1 + eAxisIndex);
	return error;
}

/**
 * [Set_MasterError description]
 * @author hankin (2019/07/26)
 * @param  eAxisIndex [description]
 * @param  iDevice    [description]
 * @param  iErrorType [description]
 * @param  iError     [description]
 * @param  iDetail    [description]
 * @param  bTry       [description]
 */
void Set_MasterError(ACTAXIS eAxisIndex, APPDEVICE iDevice, ERRORTYPE iErrorType, WORD iError, WORD iDetail, BOOL bTry)
{
	DeviceError* DBError;
	DeviceError  Error;

	if (DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR1 + eAxisIndex > DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR16)
		return;
	Error.info.bObjectID  = iDevice;
	Error.info.bSubIndex  = iDetail;
	Error.info.bErrorType = iErrorType;
	Error.info.wError     = iError;

	DBError = (DeviceError*)DB_GetDataAddr(DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR1 + eAxisIndex);
	if (0 == DBError->dwAll) {
		*DBError = Error;
	} else if (bTry) {
		unsigned i;

		for (i = DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR1; i <= DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR16; ++i) {
			// hankin 20180416 modify.
			DBError = (DeviceError*)DB_GetDataAddr(i);
			if (DBError->dwAll == Error.dwAll) {
				return;
			} else if (0 == DBError->dwAll) {
				*DBError = Error;
				break;
			}
		}
	}
}

/**
 * [Set_CommChipError description]
 * @author hankin (2019/07/26)
 * @param  wAxisIndex [description]
 * @param  wError     [description]
 */
void Set_CommChipError(ACTAXIS eAxisIndex, WORD wError)
{
	Set_MasterError(eAxisIndex, APPDEVICE_COMMCHIP, ERRORTYPE_ALARM, wError, 0, TRUE);
}

/**
 * [Set_CommChipErrorDetail description]
 * @author hankin (2019/07/26)
 * @param  eAxisIndex [description]
 * @param  wError     [description]
 * @param  wDetail    [description]
 */
void Set_CommChipErrorDetail(ACTAXIS eAxisIndex, WORD wError, WORD wDetail) // Anders 2017-1-13,add.
{
	Set_MasterError(eAxisIndex, APPDEVICE_COMMCHIP, ERRORTYPE_ALARM, wError, wDetail, TRUE);
}


/**
 * [Error_App__TaskInit description]
 */
static void Error_App__TaskInit(void)
{
	if (GetCommReadyState())
	{
		extern WORD g_wManualKey_ClrErrFlag;

		g_wManualKey_ClrErrFlag = 1;
		if (CheckTimerOut(&Error_App__checkTimer)) {
			Error_App__task = Error_App__TaskRun;
		}
	}
}

/**
 * [Error_App__TaskRun description]
 */
static void Error_App__TaskRun(void)
{
}
