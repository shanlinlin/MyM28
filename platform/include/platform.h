/*===========================================================================+
|  System   : Common                                                         |
|  Task     : Common header file                                             |
|----------------------------------------------------------------------------|
|  Compile  : C320 V6.60 -                                                   |
|  Link     : C320 V6.60 -                                                   |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : C.C. Chen                                                      |
|  Version  : V1.00                                                          |
|  Creation : 08/08/1997                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D__PLATFORM
#define     D__PLATFORM



#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#ifdef	D_DSP2812

#include "DSP281x_Device.h"         // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"       // DSP281x Examples Include File
#
#include "_eCan.h"

#elif defined D_HYB5_335 || defined D_SMC335M1 || defined D_SMC335M2

#include "DSP2833x_Device.h"        // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"      // DSP2833x Examples Include File
#
#include "_adc.h"
#include "_dac.h"
#include "_gpio.h"
#include "_encoder.h"
#include "_25lc256.h"
//#include "_ltc1864.h"
#include "_eCan_28335.h"
#include "_timer.h"
#include "_ax88796b.h"
//nclude "_eeprom_cs8900a.h"
#include "_eeprom_ax88796b.h"
//#include "ethernet.h"

#endif

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
#ifdef			D_DSP2812  
                                            	
#define			GONORMAL()          				asm(" LB 0x003F3FFE ")
#define     	GOBOOT()           					asm(" LB 0x003F7F7E ")

#elif 			defined D_HYB5_335 || defined D_SMC335M1 || defined D_SMC335M2

#define			GONORMAL()          				asm(" LB 0x0032FFFE")
#define     	GOBOOT()           					asm(" LB 0x0033FF7E")

#endif


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif

