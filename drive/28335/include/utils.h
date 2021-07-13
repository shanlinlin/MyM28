/*===========================================================================+
|  Function : Utilities	                                                     |
|  Task     : Utilities                                                      |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                   	 |
|  Link     : CCS3.1 -                                                   	 |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : Winston.Yu                                                     |
|  Version  : V1.00                                                          |
|  Creation : 12/19/2008                                                     |
|  Revision :                                                          	     |
|             							                                     |
+===========================================================================*/

#ifndef     D___UTILS
#define     D___UTILS

#include "common.h"
//#include "platform.h"
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

#ifdef  D_DSP2812
#define ReadCalcLoadCpuTimerCounter()       ReadCpuTimer0Counter()

#elif   defined D_HYB5_335 || defined D_SMC335M1 || defined D_SMC335M2
#define ReadCalcLoadCpuTimerCounter()       ReadCpuTimer1Counter()

#endif

#define CalcCPULoadStart(CPULoad)       (CPULoad)->dwStartCounter = ReadCalcLoadCpuTimerCounter()


/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef	struct	tagCPULOAD
{
	DWORD		dwStartCounter;
	DWORD		dwEndCounter;
	DWORD		dwCurrentCounter;
	DWORD		dwMaxCounter;
} CPULOAD;


/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/


/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
void	CalcCPULoadEnd(CPULOAD* CPULoad);
long	HexToL(char * pcStr);

#endif









