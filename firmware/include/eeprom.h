/*===========================================================================+
|  Function : Initialize peripheral											 |
|  Task     : Initialize peripheral header	                           		 |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : ZXC 	                                                         |
|  Version  : V1.00                                                          |
|  Creation : 2009-4-28	                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef D__EEPROM
#define D__EEPROM

//#include "_eeprom_cs8900a.h"
#include "_25lc256.h"
#include "_eeprom_ax88796b.h"
#include "ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define EEP_AD1CALIBRATELEN 9 // One Channel: 9 Word.

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

//enum NET_EEPROM_ADDRESS
//{
//	EEPADDR_NETCARD_ID      = 0,
//	EEPADDR_MAC_ID          = 1,
//	EEPADDR_MAC_START       = 2,										// mac = 3 WORDs
//	EEPADDR_CONFIG_ID		= EEPADDR_MAC_START		+ 3,				// 1 WORD, Addr = 5.
//	EEPADDR_ADADJ_START     = 6,										// sizeof(ADADJ_PARAM)=7 * 3
//	EEPADDR_CONV_START		= EEPADDR_ADADJ_START	+ 7*3,				// 2 WORD, Addr = 27.
//	EEPADDR_OPT_START		= EEPADDR_CONV_START	+ 2,				// 1 WORD, Addr = 29.
//	EEPADDR_INPUT_START     = 30,										// 1 WORD
//	EEPADDR_PID_START       = 31,										// sizeof(PIDPARAM_BUFF)=3 * 4
//
//    EEPADDR_INJECTVELOFACTORPOV = EEPADDR_PID_START + 4*3,				// 2 WORD Inject Axis Velo Factor. Anders 20091214, Addr = 43.
//	EEPADDR_INJECTVELOFACTORNEV = EEPADDR_INJECTVELOFACTORPOV + 2,      // 2 WORD, Addr = 45.
//
//	EEPADDR_CLAMPSETVELOFACTORPOV = EEPADDR_INJECTVELOFACTORNEV + 2,	// 2 WORD Clampset Axis Velo Factor. Anders 20091214, Addr = 47.
//	EEPADDR_CLAMPSETVELOFACTORNEV = EEPADDR_CLAMPSETVELOFACTORPOV + 2,  // 2 WORD, Addr = 49.
//
//    EEPADDR_EJECTORVELOFACTORPOV = EEPADDR_CLAMPSETVELOFACTORNEV + 2,	// 2 WORD Inject Axis Velo Factor. Anders 20091214, Addr = 51.
//	EEPADDR_EJECTORVELOFACTORNEV = EEPADDR_EJECTORVELOFACTORPOV + 2,    // 2 WORD, Addr = 53.
//
//    EEPADDR_NOZZLEVELOFACTORPOV = EEPADDR_EJECTORVELOFACTORNEV + 2,		// 2 WORD Inject Axis Velo Factor. Anders 20091214, Addr = 55.
//	EEPADDR_NOZZLEVELOFACTORNEV = EEPADDR_NOZZLEVELOFACTORPOV + 2,      // 2 WORD, Addr = 57.
//
//    EEPADDR_HOMINGPOSI1_ID = EEPADDR_NOZZLEVELOFACTORNEV + 2,           // 2 WORD Homing Position1. Inject. Addr = 59.
//    EEPADDR_HOMINGPOSI2_ID = EEPADDR_HOMINGPOSI1_ID + 2,                // 2 WORD Homing Position2. Clampset.Addr = 61.
//    EEPADDR_HOMINGPOSI3_ID = EEPADDR_HOMINGPOSI2_ID + 2,                // 2 WORD Homing Position3. Ejector.Addr = 63.
//    EEPADDR_HOMINGPOSI4_ID = EEPADDR_HOMINGPOSI3_ID + 2,                // 2 WORD Homing Position4. Nozzle.Addr = 65.
//    EEPADDR_HOMINGPOSI5_ID = EEPADDR_HOMINGPOSI4_ID + 2,                // 2 WORD Homing Position5.Addr = 67.
//    EEPADDR_HOMINGPOSI6_ID = EEPADDR_HOMINGPOSI5_ID + 2,                // 2 WORD Homing Position6.Addr = 69.
//    EEPADDR_HOMINGPOSI7_ID = EEPADDR_HOMINGPOSI6_ID + 2,                // 2 WORD Homing Position7.Addr = 71.
//    EEPADDR_HOMINGPOSI8_ID = EEPADDR_HOMINGPOSI7_ID + 2,                // 2 WORD Homing Position8.Addr = 73.
//
//    EEPADDR_ADDEVICEFLAG    = 74,                                       // AD Device Write Flag. 8Bit.
//    EEPADDR_AD1TYPE,                                                    // AD1 Type.
//    EEPADDR_AD1DEVICEVOLTAGEMIN,                                        // AD1 Device Voltage Min.
//    EEPADDR_AD1DEVICEVOLTAGEMAX,                                        // AD1 Device Voltage Max.
//    EEPADDR_AD2TYPE,                                                    // AD2 Type.
//    EEPADDR_AD2DEVICEVOLTAGEMIN,                                        // AD2 Device Voltage Min.
//    EEPADDR_AD2DEVICEVOLTAGEMAX,                                        // AD2 Device Voltage Max.
//    EEPADDR_AD3TYPE,                                                    // AD3 Type.
//    EEPADDR_AD3DEVICEVOLTAGEMIN,                                        // AD3 Device Voltage Min.
//    EEPADDR_AD3DEVICEVOLTAGEMAX,                                        // AD3 Device Voltage Max.
//    EEPADDR_AD4TYPE,                                                    // AD4 Type.
//    EEPADDR_AD4DEVICEVOLTAGEMIN,                                        // AD4 Device Voltage Min.
//    EEPADDR_AD4DEVICEVOLTAGEMAX,                                        // AD4 Device Voltage Max.
//    EEPADDR_AD5TYPE,                                                    // AD5 Type.
//    EEPADDR_AD5DEVICEVOLTAGEMIN,                                        // AD5 Device Voltage Min.
//    EEPADDR_AD5DEVICEVOLTAGEMAX,                                        // AD5 Device Voltage Max.
//    EEPADDR_AD6TYPE,                                                    // AD6 Type.
//    EEPADDR_AD6DEVICEVOLTAGEMIN,                                        // AD6 Device Voltage Min.
//    EEPADDR_AD6DEVICEVOLTAGEMAX,                                        // AD6 Device Voltage Max.
//    EEPADDR_AD7TYPE,                                                    // AD7 Type.
//    EEPADDR_AD7DEVICEVOLTAGEMIN,                                        // AD7 Device Voltage Min.
//    EEPADDR_AD7DEVICEVOLTAGEMAX,                                        // AD7 Device Voltage Max.
//    EEPADDR_AD8TYPE,                                                    // AD8 Type.
//    EEPADDR_AD8DEVICEVOLTAGEMIN,                                        // AD8 Device Voltage Min.
//    EEPADDR_AD8DEVICEVOLTAGEMAX,                                        // AD8 Device Voltage Max.
//
//};

enum NET_EEPROM_ADDRESS
{
	EEPADDR_NETCARD_ID  = 0,
	EEPADDR_MAC_ID      = 1,
	EEPADDR_MAC_START   = 2,                           // mac = 3 WORDs
	EEPADDR_CONFIG_ID   = EEPADDR_MAC_START + 3,       // 1 WORD, Addr = 5.
	EEPADDR_ADADJ_START = 6,                           // sizeof(ADADJ_PARAM)=7 * 3
	EEPADDR_CONV_START  = EEPADDR_ADADJ_START + 7 * 3, // 2 WORD, Addr = 27.
	EEPADDR_OPT_START   = EEPADDR_CONV_START + 2,      // 2 WORD, Addr = 29.
	EEPADDR_INPUT_START = 30,                          // 1 WORD
	EEPADDR_PID_START   = 31,                          // sizeof(PIDPARAM_BUFF)=3 * 4

	EEPADDR_INJECTVELOFACTORPOV = EEPADDR_PID_START + 4 * 3,       // 2 WORD Inject Axis Velo Factor. Anders 20091214, Addr = 43.
	EEPADDR_INJECTVELOFACTORNEV = EEPADDR_INJECTVELOFACTORPOV + 2, // 2 WORD, Addr = 45.

	EEPADDR_CLAMPSETVELOFACTORPOV = EEPADDR_INJECTVELOFACTORNEV + 2,   // 2 WORD Clampset Axis Velo Factor. Anders 20091214, Addr = 47.
	EEPADDR_CLAMPSETVELOFACTORNEV = EEPADDR_CLAMPSETVELOFACTORPOV + 2, // 2 WORD, Addr = 49.

	EEPADDR_EJECTORVELOFACTORPOV = EEPADDR_CLAMPSETVELOFACTORNEV + 2, // 2 WORD Inject Axis Velo Factor. Anders 20091214, Addr = 51.
	EEPADDR_EJECTORVELOFACTORNEV = EEPADDR_EJECTORVELOFACTORPOV + 2,  // 2 WORD, Addr = 53.

	EEPADDR_NOZZLEVELOFACTORPOV = EEPADDR_EJECTORVELOFACTORNEV + 2, // 2 WORD Inject Axis Velo Factor. Anders 20091214, Addr = 55.
	EEPADDR_NOZZLEVELOFACTORNEV = EEPADDR_NOZZLEVELOFACTORPOV + 2,  // 2 WORD, Addr = 57.

	EEPADDR_HOMINGPOSI1_ID = EEPADDR_NOZZLEVELOFACTORNEV + 2, // 2 WORD Homing Position1. Inject. Addr = 59.
	EEPADDR_HOMINGPOSI2_ID = EEPADDR_HOMINGPOSI1_ID + 2,      // 2 WORD Homing Position2. Clampset.Addr = 63.
	EEPADDR_HOMINGPOSI3_ID = EEPADDR_HOMINGPOSI2_ID + 2,      // 2 WORD Homing Position3. Ejector.Addr = 67.
	EEPADDR_HOMINGPOSI4_ID = EEPADDR_HOMINGPOSI3_ID + 2,      // 2 WORD Homing Position4. Nozzle.Addr = 71.
	EEPADDR_HOMINGPOSI5_ID = EEPADDR_HOMINGPOSI4_ID + 2,      // 2 WORD Homing Position5.Addr = 75.
	EEPADDR_HOMINGPOSI6_ID = EEPADDR_HOMINGPOSI5_ID + 2,      // 2 WORD Homing Position6.Addr = 79.
	EEPADDR_HOMINGPOSI7_ID = EEPADDR_HOMINGPOSI6_ID + 2,      // 2 WORD Homing Position7.Addr = 83.
	EEPADDR_HOMINGPOSI8_ID = EEPADDR_HOMINGPOSI7_ID + 2,      // 2 WORD Homing Position8.Addr = 87.

	EEPADDR_AD1CALIBRATE_WRITEFLAG = 100,                                // 1:WriteFlag,2:MinV,3:MaxV,4:iqCaliMin,5:iqCaliMax.
	EEPADDR_AD1CALIBRATE_MINV      = EEPADDR_AD1CALIBRATE_WRITEFLAG + 1, // Anders 2015-7-21, Modify +1 -> +2.
	EEPADDR_AD1CALIBRATE_MAXV      = EEPADDR_AD1CALIBRATE_MINV + 2,
	EEPADDR_AD1CALIBRATE_IQCALIMIN = EEPADDR_AD1CALIBRATE_MAXV + 2,
	EEPADDR_AD1CALIBRATE_IQCALIMAX = EEPADDR_AD1CALIBRATE_IQCALIMIN + 2,
	EEPADDR_AD10CALIBRATE_END      = EEPADDR_AD1CALIBRATE_WRITEFLAG + 9 * 10, // 10 Channel

	EEPADDR_ELECNAMEPLAT_HARDWARE_PN = EEPADDR_AD10CALIBRATE_END,
	EEPADDR_ELECNAMEPLAT_HARDWARE_SN = EEPADDR_ELECNAMEPLAT_HARDWARE_PN + 2,
	EEPADDR_ELECNAMEPLAT_SOFTWARE_PN = EEPADDR_ELECNAMEPLAT_HARDWARE_SN + 2,

	EEPADDR_VERSION1 = EEPSIZE - 2, // Addr = 254.
	EEPADDR_VERSION2 = EEPSIZE - 1  // Addr = 255.
};
/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
//void	Create_EEPROM();
//void Enable_EEPROM();
//void Disable_EEPROM();
//void	EraseAll_EEPROM();
//void	Erase_EEPROM(WORD wAddress);
//void	WriteAll_EEPROM(WORD wValue);
WORD ReadString_EEPROM(WORD wStartAddr, WORD * pData, WORD wSize);
WORD WriteString_EEPROM(WORD wStartAddr, WORD * pData, WORD wSize);

#ifdef __cplusplus
}
#endif

#endif
