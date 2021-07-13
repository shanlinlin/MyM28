/*===========================================================================+
|  Function : 											 					 |
|  Task     : 									                           	 |
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

#ifndef     D__EEPROM_COMMON
#define     D__EEPROM_COMMON

#include "common.h"

#ifdef      __cplusplus
extern      "C" {
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
// 93C46 command format
//efine		EEPCMD_EWEN			0x130
//efine		EEPCMD_EWDS			0x100
//efine		EEPCMD_ERASE		0x1C0
//efine		EEPCMD_ERAL			0x120

#define		EEPCMD_EWEN			0x4
#define		EEPCMD_EWDS			0x4
#define		EEPCMD_ERASE		0x7
#define		EEPCMD_ERAL			0x4

//efine		EEPCMD_WRAL			0x1100000
//efine		EEPCMD_WRITE		0x1400000
//efine		EEPCMD_READ			0x1800000    

#define		EEPCMD_WRAL			0x4
#define		EEPCMD_WRITE		0x5
#define		EEPCMD_READ			0x6

#define		EEPADDR_EWDS		0x0
#define		EEPADDR_WRAL		0x1
#define		EEPADDR_ERAL		0x2
#define		EEPADDR_EWEN		0x3

#define		EEPLEN_CMD			3
#define		EEPLEN_ADDR			8
#define		EEPLEN_NULL			(EEPLEN_ADDR-2)
#define		EEPLEN_DATA			16

#define		EEPLEN_SHORTCMD		(EEPLEN_CMD+EEPLEN_ADDR)
#define		EEPLEN_LONGCMD		(EEPLEN_CMD+EEPLEN_ADDR+EEPLEN_DATA)

#define		SIZE_93C46			0x40
#define		SIZE_93C66			0x100
#define		EEPSIZE				SIZE_93C66
// 93C46 command format

// 93C46 command timing
#define		EEP_TCSS			1			// Min: 50ns
#define		EEP_TCSH			0			// Min: 0ns
#define		EEP_TCSL			1			// Min: 250ns
#define		EEP_TDIS			2000	    // Min: 100ns 5000->2000
#define		EEP_TCKH			2			// Min: 250ns
#define		EEP_TCKL			2			// Min: 250ns
#define		EEP_TPD				1			// Max: 400ns
#define		EEP_TCZ				0			// Max: 100ns
// 93C46 command timing

// AX88796B operate 93C46 command format
#define		NETEEP_EECS			0x0010
#define		NETEEP_EEI			0x0020
#define		NETEEP_EEO			0x0040
#define		NETEEP_EECK			0x0080
#define		NETEEP_DIR			0x0002
// AX88796B operate 93C46 command format

#define		EEPCMD_MASK			(0x00000001L)

#define		EEP_MAC_ID			0x1111
#define		EEP_CONFIG_ID		0x2222

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

#ifdef      __cplusplus
}
#endif

#endif
