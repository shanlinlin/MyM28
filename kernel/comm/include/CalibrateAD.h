/*==========================================================================+
|  Function : Calibrate AD                                        			|
|  Task     : 								                                |
|---------------------------------------------------------------------------|
|  Compile  : CCS3.1                                                        |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders.Zhang                                                 	|
|  Version  : V1.00                                                         |
|  Creation : 2010/12/03                                                    |
|  Revision : V1.0			                                                |
+==========================================================================*/

#ifndef D__CALIBRATEAD_H
#define D__CALIBRATEAD_H

#include "IQmathLib.h"
#include "_io.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Definition                                                       |
+===========================================================================*/
#define MIN_VOLTAGE_CARD28 0.0
#define MAX_VOLTAGE_CARD28 10.0

#define MAX_CALIBRATEADCHANNEL 4

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct tagCalibrateADController
{
	WORD wADChannel; // Form 1.

	FLOAT fMinVoltage_Card28;
	FLOAT fMaxVoltage_Card28;

	FLOAT fMinVoltage_Peripheral;
	FLOAT fMaxVoltage_Peripheral;

	//FLOAT  fCapacity_Peripheral;     // Capacity of AD Ruler, or Capacity of pressure sensor, ...

	// Card28 Calibrate Information.
	_iq iqMinADCalibrate_Card28;
	_iq iqMaxADCalibrate_Card28;
	_iq iqScaleADCalibrate_Card28;

	// Peripheral Voltage Information.
	_iq iqMinAppVoltage;
	_iq iqMaxAppVoltage;
	_iq iqScaleAppVoltage;

	// AD Value after Calibrated.
	_iq iqADCalibrateResult;
} TCalibrateADController;

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
void InitADCalibrate(TCalibrateADController * pController, WORD wADChannel); // wADChannel From 1.

//void  SetADCard28MinVoltage(TCalibrateADController *pController,FLOAT fVoltage); // May be used in future.
//void  SetADCard28MaxVoltage(TCalibrateADController *pController,FLOAT fVoltage); // May be used in future.
void SetPeripheralADMinVoltage(TCalibrateADController * pController, FLOAT fVoltage);
void SetPeripheralADMaxVoltage(TCalibrateADController * pController, FLOAT fVoltage);

void SetCalibrateADMinValue(TCalibrateADController * pController, _iq iqValue);
void SetCalibrateADMaxValue(TCalibrateADController * pController, _iq iqValue);

void CalibrateADMinValue(TCalibrateADController * pController, _iq iqADValue);
void CalibrateADMaxValue(TCalibrateADController * pController, _iq iqADValue);
void ResetCalibrateADValue(TCalibrateADController * pController);

_iq GetCalibrateADValue(TCalibrateADController * pController, _iq iqADValue);
/*===========================================================================+
|           External                                                         |
+===========================================================================*/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
