/*==============================================================================+
|  Function : Application Error                                                 |
|  Task     : Application Error Header File                                     |
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
#ifndef D__ERROR_APP__H
#define D__ERROR_APP__H

#include "common.h"
#include "Device.h"
//##############################################################################
//
//      Definition
//
//##############################################################################
// DEVICE_COMMCHIP Alarm:
#define ERROR_ID_AXISCARDNOTREADY                           4096
#define ERROR_ID_AXISCARDOFFLINE                            4097
#define ERROR_ID_DBUPLOAD                                   4098
#define ERROR_ID_DBDOWNLOAD                                 4099

// #define ERROR_ID_DACONFIG                                   4101
#define ERROR_ID_MOTORCONFLICT                              4102
#define ERROR_ID_CANCH1_NODEID_CONFLICT                     4103
#define ERROR_ID_CANCH2_NODEID_CONFLICT                     4104
#define ERROR_ID_MOTIONAXIS_NUM                             4105
#define ERROR_ID_MOTIONAXIS_CONFLICT                        4106
#define ERROR_ID_AXIS_DATASYNC                              4107
#define ERROR_ID_DACONFLICT                                 4108
#define ERROR_ID_WATCH_NOTREADY                             4110
#define ERROR_ID_WATCH_DATA_READ                            4111
#define ERROR_ID_WATCH_DATA_WRITE                           4112
#define ERROR_ID_WATCH_ODOBJ_NOTEXSITE                      4132
#define ERROR_ID_WATCH_ODSUBINDEX_NOTEXSITE                 4141

// DEVICE_AXISCARD Alarm:
#define ERROR_ID_AXISCARD_NODEIDCONFLICT                    4358
// #define ERROR_ID_AXISCARD_DEVICE_VERIFY_FAIL                4368

// DEVICE_REMOTEIO Alarm:
// #define ERROR_ID_REMOTEIO_OFFLINE                           4609
#define ERROR_ID_REMOTEIO_NODEIDCONFLICT                    4614
#define ERROR_ID_REMOTEIO_PICKERDEVICE_VERIFY_FAIL          4623
#define ERROR_ID_REMOTEIO_TMIOT101DEVICE_VERIFY_FAIL        4624
#define ERROR_ID_REMOTEIO_TMIOT102DEVICE_VERIFY_FAIL        4625
#define ERROR_ID_REMOTEIO_TMIOT105DEVICE_VERIFY_FAIL        4626
#define ERROR_ID_REMOTEIO_TMIOT202DEVICE_VERIFY_FAIL        4627
#define ERROR_ID_REMOTEIO_TMIOT201DEVICE_VERIFY_FAIL        4628

//#define ERROR_ID_REMOTEIO_DEVICE1_OFFLINE                   4631
//#define ERROR_ID_REMOTEIO_DEVICE2_OFFLINE                   4632
//#define ERROR_ID_REMOTEIO_DEVICE3_OFFLINE                   4633
//#define ERROR_ID_REMOTEIO_DEVICE4_OFFLINE                   4634
//#define ERROR_ID_REMOTEIO_DEVICE5_OFFLINE                   4635
//#define ERROR_ID_REMOTEIO_DEVICE6_OFFLINE                   4636
//#define ERROR_ID_REMOTEIO_DEVICE7_OFFLINE                   4637
//#define ERROR_ID_REMOTEIO_DEVICE8_OFFLINE                   4638
//#define ERROR_ID_REMOTEIO_DEVICE9_OFFLINE                   4639
//#define ERROR_ID_REMOTEIO_DEVICE10_OFFLINE                  4640
//#define ERROR_ID_REMOTEIO_DEVICE11_OFFLINE                  4641
//#define ERROR_ID_REMOTEIO_DEVICE12_OFFLINE                  4642
//#define ERROR_ID_REMOTEIO_DEVICE13_OFFLINE                  4643

// DEVICE_TEMPCARD Alarm:
//#define ERROR_ID_TEMPCARD_OFFLINE                           4865
#define ERROR_ID_TEMPCARD_NODEIDCONFLICT                    4870
#define ERROR_ID_TEMPCARD_DEVICE_VERIFY_FAIL                4880
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum {
	ACTAXIS_MOLD,
	ACTAXIS_MOLD2,
	ACTAXIS_EJECT,
	ACTAXIS_EJECT2,
	ACTAXIS_INJECT,
	ACTAXIS_INJECT2,
	ACTAXIS_RECOVERY,
	ACTAXIS_RECOVERY2,
	ACTAXIS_NOZZLE,
	ACTAXIS_NOZZLE2,
	ACTAXIS_ROTARY,
	ACTAXIS_ROTARY2,
	ACTAXIS_OTHER1,
	ACTAXIS_OTHER2,
	ACTAXIS_OTHER3,
	ACTAXIS_OTHER4,

	ACTAXIS_SUM
} ACTAXIS;

typedef enum {
	ERRORTYPE_ALARM,
	ERRORTYPE_WARN,

	ERRORTYPE_SUM
} ERRORTYPE;
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef union DeviceError {
	DWORD dwAll;
	struct
	{
		WORD wError : 16;
		WORD bSubIndex : 6;
		WORD bErrorType : 2;
		WORD bObjectID : 8;
	} info;
} DeviceError;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Error_App_Init(void);
void Error_App_Run(void);
void Error_App_Reset(void);

DeviceError Get_MasterError(ACTAXIS eAxisIndex);
void        Set_MasterError(ACTAXIS eAxisIndex, APPDEVICE iDevice, ERRORTYPE iErrorType, WORD iError, WORD iDetail, BOOL bTry);
void        Set_CommChipError(ACTAXIS eAxisIndex, WORD wError);
void        Set_CommChipErrorDetail(ACTAXIS eAxisIndex, WORD wError, WORD wDetail); // Anders 2017-1-13,add.
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
#endif
