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

#include "taskio.h"
#include "eeprom.h"
#include "initial.h"
#include "task.h"
#include <string.h>
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/
volatile DEVICE_DATA   g_devicedata;
DEVICE_CONFIG          g_deviceconfig;
TCalibrateADController g_CalibrateADController[ADCHL_USE_APP];

CPULOAD g_IOCPULoad = { 0, 0, 0, 0 };
/*===========================================================================+
|           Local variable                                                   |
+===========================================================================*/
const WORD BitMask[16] = {
	MASK_BIT00,
	MASK_BIT01,
	MASK_BIT02,
	MASK_BIT03,
	MASK_BIT04,
	MASK_BIT05,
	MASK_BIT06,
	MASK_BIT07,
	MASK_BIT08,
	MASK_BIT09,
	MASK_BIT10,
	MASK_BIT11,
	MASK_BIT12,
	MASK_BIT13,
	MASK_BIT14,
	MASK_BIT15
};

/*===========================================================================+
|           Function prototype                                               |
+===========================================================================*/
void Scan_IODeviceData();
void Get_IODeviceData();
void Set_IODeviceData();

void ReadEEPROM_ADCalibrateInfo();
void Write_DAProtected();
/*===========================================================================+
|           Operation                                                        |
+===========================================================================*/
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
void Create_TaskIO()
{
	int i;
	Initperipheral();
	InitIO();

	memset((void*)&g_devicedata, 0, sizeof(DEVICE_DATA));
	//g_devicedata.awOutput[0] = 3;
	//Set_BitOut(1);
	//Set_BitOut(2);

	//Create_TimerintTask(1, &Run_TaskIO, NULL);
	//Create_TimerintTask(1, Run_TaskAD, NULL);

	// ... get device config ???
	memset(&g_deviceconfig, 0, sizeof(g_deviceconfig));

	// Anders 2010-12-3.
	for (i = 0; i < ADCHL_USE_APP; i++)
	{
		InitADCalibrate(&g_CalibrateADController[i], i + 1);
	}
	//#if  defined D_HYB5_335
	//g_CalibrateADController[3].iqMinADCalibrate_Card28 = _IQ(1); // Anders 2013-4-9. CHL4:??10V
	//#endif
	ReadEEPROM_ADCalibrateInfo();

	// Anders 20120406.
	_FC_Write_DAProtected = Write_DAProtected;
}

void Destroy_TaskIO()
{
	//Destroy_TimerintTask(&Run_TaskIO, NULL);
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
void Run_TaskIO()
{
	// Scan IO device
	Scan_IODeviceData();

	// Get IO device data
	Get_IODeviceData();

	// Output IO device
	Set_IODeviceData();

	// Scan AD
	Run_TaskAD();
}
void Run_TaskAD()
{
	int  i;
	WORD wStatus = 0;

	//CalcCPULoadStart(&g_IOCPULoad);
	Scan_AD1();
	//CalcCPULoadEnd(&g_IOCPULoad);

	if (ADCHL_USE_APP > MAX_AD1)
		Scan_AD2();

	// AD
	//for (i=0; i<ADCHL_USE_APP; i++) g_devicedata.anAD[i] = Get_AD(i+1, &wStatus);
	// Anders 2010-12-3, AD Calibrate.
	for (i = 0; i < ADCHL_USE_APP; i++)
	{
		g_devicedata.anDriveAD[i] = Get_AD(i + 1, &wStatus);
		g_devicedata.anAD[i]      = _IQtoIQ12(GetCalibrateADValue(&g_CalibrateADController[i], _IQ12toIQ(g_devicedata.anDriveAD[i])));
		//g_devicedata.anAD[i] = g_devicedata.anDriveAD[i];// _IQtoIQ12(GetCalibrateADValue(&g_CalibrateADController[i],_IQ12toIQ(g_devicedata.anDriveAD[i])));
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
// Anders 2010-11-24.
void Write_DAProtected()
{
	int i;
	for (i = 1; i <= MAX_DA; i++)
		Set_DA(i, 0);
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
void Scan_IODeviceData()
{
	Scan_BitIn();
	//Scan_AD1();
	//Scan_AD2();
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
void Get_IODeviceData()
{
	int  nFlag;
	WORD i, j = 0, k = 0;

	// input
	for (i = 0; i < MAX_BITIN; i++)
	{
		j = i >> 4;
		k = i % 16;

		// Note: Exchange input can be put here.
		if (Get_BitIn(i + 1))
			nFlag = 1;
		else
			nFlag = -1;
		if (g_deviceconfig.awInLogic[j] & BitMask[k])
			nFlag = -nFlag;

		if (nFlag == 1)
			g_devicedata.awInput[j] |= BitMask[k];
		else
			g_devicedata.awInput[j] &= ~BitMask[k];
	}

	//// AD
	////for (i=0; i<ADCHL_USE; i++) g_devicedata.anAD[i] = Get_AD(i+1, &wStatus);
	//// Anders 2010-12-3, AD Calibrate.
	//for (i=0; i<ADCHL_USE; i++) g_devicedata.anAD[i] = _IQtoIQ12(GetCalibrateADValue(&g_CalibrateADController[i],_IQ12toIQ(Get_AD(i+1, &wStatus))));
	// Encoder
	for (i = 0; i < MAX_ENCODER; i++)
		g_devicedata.awEncoder[i] = Get_Encoder(i + 1);
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
void Set_IODeviceData()
{
	WORD i, j = 0, k = 0;

	// output
	for (i = 0; i < MAX_BITOUT; i++)
	{
		j = i >> 4;
		k = i % 16;
		if (g_devicedata.awOutput[j] & BitMask[k])
			Set_BitOut(i + 1);
		else
			Clr_BitOut(i + 1);
	}

	// DA
	for (i = 0; i < MAX_DA; i++)
	{
		Set_DA(i + 1, g_devicedata.awDA[i]);
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
BOOL Read_Input(WORD hio)
{
	WORD i, j;

	if (hio == 0 || hio > MAX_BITIN)
		return 0; // Anders 20130111.

	i = (hio - 1) >> 4;
	j = (hio - 1) % 16;

	if (i < MAXIO_INPORT && g_devicedata.awInput[i] & BitMask[j])
		return 1;
	else
		return 0;
}

void Set_Output(WORD hio)
{
	WORD i, j;

	if (hio == 0 || hio > MAX_BITOUT)
		return; // Anders 20130111.

	i = (hio - 1) >> 4;
	j = (hio - 1) % 16;
	if (i < MAXIO_OUTPORT)
		g_devicedata.awOutput[i] |= BitMask[j];
}

void Clr_Output(WORD hio)
{
	WORD i, j;

	if (hio == 0 || hio > MAX_BITOUT)
		return; // Anders 20130111.

	i = (hio - 1) >> 4;
	j = (hio - 1) % 16;
	if (i < MAXIO_OUTPORT)
		g_devicedata.awOutput[i] &= ~BitMask[j];
}
//========================================================================================================
// Anders 2010-12-3.
void ReadEEPROM_ADCalibrateInfo()
{
	WORD                    i, wRst, wValue, wADChannel;
	FLOAT                   fMinVoltage, fMaxVoltage;
	_iq                     iqMinCali, iqMaxCali;
	TCalibrateADController* pController;

	for (i = 0; i < ADCHL_USE_APP; i++)
	{
		pController = &g_CalibrateADController[i];
		if (pController->wADChannel < 1 || pController->wADChannel > 10)
			continue; // EEPROM Save 10 Channel info.

		wADChannel = (pController->wADChannel - 1) * EEP_AD1CALIBRATELEN;
		wRst       = ReadString_EEPROM(EEPADDR_AD1CALIBRATE_WRITEFLAG + wADChannel, &wValue, sizeof(WORD));
		if (wRst && wValue == 1)
		{
			ReadString_EEPROM(EEPADDR_AD1CALIBRATE_MINV + wADChannel, (WORD*)&fMinVoltage, 2);
			ReadString_EEPROM(EEPADDR_AD1CALIBRATE_MAXV + wADChannel, (WORD*)&fMaxVoltage, 2);
			ReadString_EEPROM(EEPADDR_AD1CALIBRATE_IQCALIMIN + wADChannel, (WORD*)&iqMinCali, 2);
			ReadString_EEPROM(EEPADDR_AD1CALIBRATE_IQCALIMAX + wADChannel, (WORD*)&iqMaxCali, 2);

			SetPeripheralADMinVoltage(pController, fMinVoltage);
			SetPeripheralADMaxVoltage(pController, fMaxVoltage);
			SetCalibrateADMinValue(pController, iqMinCali);
			SetCalibrateADMaxValue(pController, iqMaxCali);
		}
	}
}
void Commissioning_ResetCalibrateAD(WORD wADChannel)
{
	if (wADChannel > 0 && wADChannel < ADCHL_USE_APP)
		ResetCalibrateADValue(&g_CalibrateADController[wADChannel - 1]);
}
void Commissioning_CalibrateADMax(WORD wADChannel)
{
	WORD wStatus;
	if (wADChannel > 0 && wADChannel < ADCHL_USE_APP)
		CalibrateADMaxValue(&g_CalibrateADController[wADChannel - 1], _IQ12toIQ(Get_AD(wADChannel, &wStatus))); //_IQ12toIQ(Read_AD(wADChannel)));
}
void Commissioning_CalibrateADMin(WORD wADChannel)
{
	WORD wStatus;
	if (wADChannel > 0 && wADChannel < ADCHL_USE_APP)
		CalibrateADMinValue(&g_CalibrateADController[wADChannel - 1], _IQ12toIQ(Get_AD(wADChannel, &wStatus))); //_IQ12toIQ(Read_AD(wADChannel)));
}
void Commissioning_SetPeripheralMinVoltage(WORD wADChannel, FLOAT fVoltage)
{
	if (wADChannel > 0 && wADChannel < ADCHL_USE_APP)
		SetPeripheralADMinVoltage(&g_CalibrateADController[wADChannel - 1], fVoltage);
}
void Commissioning_SetPeripheralMaxVoltage(WORD wADChannel, FLOAT fVoltage)
{
	if (wADChannel > 0 && wADChannel < ADCHL_USE_APP)
		SetPeripheralADMaxVoltage(&g_CalibrateADController[wADChannel - 1], fVoltage);
}
void Commissioning_CalibrateADSave(WORD wADChannel)
{
	WORD                    wChannel, wRst, wValue;
	TCalibrateADController* pController;
	if (wADChannel > 0 && wADChannel < ADCHL_USE_APP)
	{
		pController = &g_CalibrateADController[wADChannel - 1];
		wChannel    = (wADChannel - 1) * EEP_AD1CALIBRATELEN;
		wValue      = 1;
		wRst        = WriteString_EEPROM(EEPADDR_AD1CALIBRATE_WRITEFLAG + wChannel, &wValue, sizeof(WORD));
		if (wRst)
		{
			WriteString_EEPROM(EEPADDR_AD1CALIBRATE_MINV + wChannel, (WORD*)&pController->fMinVoltage_Peripheral, 2);
			WriteString_EEPROM(EEPADDR_AD1CALIBRATE_MAXV + wChannel, (WORD*)&pController->fMaxVoltage_Peripheral, 2);
			WriteString_EEPROM(EEPADDR_AD1CALIBRATE_IQCALIMIN + wChannel, (WORD*)&pController->iqMinADCalibrate_Card28, 2);
			WriteString_EEPROM(EEPADDR_AD1CALIBRATE_IQCALIMAX + wChannel, (WORD*)&pController->iqMaxADCalibrate_Card28, 2);
		}
	}
}
