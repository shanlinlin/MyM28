/*==========================================================================+
|  Class    : Control                                                       |
|             Motion control                                                |
|  Task     : Motion control routine                                        |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   :                                                               |
|  Version  : V1.00                                                         |
|  Creation : zholy                                                         |
|  Revision :                                                               |
+==========================================================================*/

// #ifndef _DEBUG_ static #endif

#ifndef D__TASKIO
#define	D__TASKIO

#include "common.h"
#include "_io.h"
#include "CalibrateAD.h"
#include "initial.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Definition                                                       |
+===========================================================================*/
// note: be different from MAX define in io.h
#define		MAXIO_INPORT				2
#define		MAXIO_OUTPORT				4

#define		IO_OUTPUT_DEFORCE			1

#define Get_InputGroup(Group)           (g_devicedata.awInput[(Group)-1])
#define Get_OutputGroup(Group)          (g_devicedata.awOutput[(Group)-1])
#define Set_OutputGroup(Group, Value)   (g_devicedata.awOutput[(Group)-1] = Value)

#define	Read_AD(Channel)			    (g_devicedata.anAD[(Channel)-1])          	// return 12bit long value.
#define	Read_DriveAD(Channel)			(g_devicedata.anDriveAD[(Channel)-1])         // return 12bit long value.
#define	Write_DA(Channel, Value)	    (g_devicedata.awDA[(Channel)-1] = Value)

#define	Get_DA(Channel)			        (g_devicedata.awDA[(Channel)-1])          // return 12bit long value.

#define	Read_Encoder(Channel)		    (g_devicedata.awEncoder[(Channel)-1])

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct tagDEVICE_DATA
{
	WORD	awInput[MAXIO_INPORT];
	WORD	awOutput[MAXIO_OUTPORT];
	int		anAD[ADCHL_USE_APP];        // Calibrate AD Value. _IQ12.
	int		anDriveAD[ADCHL_USE_APP];   // Drive AD Value. _IQ12. 		Anders 2013-11-21.
	WORD	awDA[MAX_DA];
	WORD	awEncoder[MAX_ENCODER];
} DEVICE_DATA;

typedef struct tagDEVICE_CONFIG
{
	// Input config
	WORD	awInLogic[MAXIO_INPORT];
	WORD	awInExchange[MAXIO_INPORT*16];
	WORD	awOutExchange[MAXIO_OUTPORT*16];

	// AD config
	int		anADMin[ADCHL_USE_APP];
	int		anADMax[ADCHL_USE_APP];
} DEVICE_CONFIG;

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           External                                                         |
+===========================================================================*/
extern	volatile DEVICE_DATA	g_devicedata;
extern	DEVICE_CONFIG			g_deviceconfig;
extern  CPULOAD                 g_IOCPULoad;

void	Create_TaskIO();
void	Destroy_TaskIO();
void	Run_TaskIO();
void	Run_TaskAD();

BOOL	Read_Input(WORD hio);
void	Set_Output(WORD hio);
void	Clr_Output(WORD hio);

void    Commissioning_ResetCalibrateAD(WORD wADChannel);
void    Commissioning_CalibrateADMax(WORD wADChannel);
void    Commissioning_CalibrateADMin(WORD wADChannel);
void    Commissioning_CalibrateADSave(WORD wADChannel);
void    Commissioning_SetPeripheralMinVoltage(WORD wADChannel,FLOAT fVoltage);
void    Commissioning_SetPeripheralMaxVoltage(WORD wADChannel,FLOAT fVoltage);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
