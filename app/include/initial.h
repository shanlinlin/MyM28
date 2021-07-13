/*===========================================================================+
|  Function : Initialize peripheral											 |
|  Task     : Initialize peripheral header	                           		 |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : zholy                                                          |
|  Version  : V1.00                                                          |
|  Creation : 11/13/2007                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D__INITIAL_2
#define     D__INITIAL_2

#include "common.h"
#include "_io.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define     ADCHL_USE_APP	4   // MAX_AD   // ADCHL_USE <= MAX_AD
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct tagINITPARAM
{
	AD		ad;
	DA		da;
} INITPARAM;

typedef struct tagBITINX
{
	BITIN   bitin[MAX_BITIN];
} BITINX;

typedef struct tagDVALVE
{
	BITOUT  bitout[MAX_BITOUT];
} BITOUTX;

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
void Initperipheral();
void InitIO();

#endif
