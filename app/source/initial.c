/*===========================================================================+
|  Function : Initialize peripheral											 |
|  Task     : Initialize peripheral			                           		 |
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

#include "initial.h"
#include "string.h"
/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/
#ifdef D_HYB5_335
BITINX  g_bitinx  = { { { BITI01, 0 }, { BITI02, 0 } } };
BITOUTX g_bitoutx = { { { BITO01 }, { BITO02 } } };

#else
BITINX g_bitinx = { {
	{ BITI01, 0 }, { BITI02, 0 }, { BITI03, 0 }, { BITI04, 0 }, { BITI05, 0 }, { BITI06, 0 }, { BITI07, 0 }, { BITI08, 0 }, /* Control input 01 - 08 */
	{ BITI09, 0 }, { BITI10, 0 }, { BITI11, 0 }, { BITI12, 0 }, { BITI13, 0 }, { BITI14, 0 }, { BITI15, 0 }, { BITI16, 0 }  /* Control input 09 - 16 */
} };

BITOUTX g_bitoutx = { {
	{ BITO01 }, { BITO02 }, { BITO03 }, { BITO04 }, { BITO05 }, { BITO06 }, { BITO07 }, { BITO08 }, /* Control output 01 - 08 */
	{ BITO09 }, { BITO10 }, { BITO11 }, { BITO12 }, { BITO13 }, { BITO14 }, { BITO15 }, { BITO16 }, /* Control output 09 - 16 */
	{ BITO17 }, { BITO18 }, { BITO19 }, { BITO20 }, { BITO21 }, { BITO22 }, { BITO23 }, { BITO24 }, /* Control output 17 - 24 */
	{ BITO25 }, { BITO26 }, { BITO27 }, { BITO28 }, { BITO29 }, { BITO30 }, { BITO31 }, { BITO32 }, /* Control output 25 - 32 */
	{ BITO33 }, { BITO34 }, { BITO35 }, { BITO36 }, { BITO37 }, { BITO38 }, { BITO39 }, { BITO40 }, /* Control output 33 - 40 */
	{ BITO41 }, { BITO42 }, { BITO43 }, { BITO44 }, { BITO45 }, { BITO46 }, { BITO47 }, { BITO48 }, /* Control output 41 - 48 */
	                                                                                                //{BITO49},{BITO50},{BITO51},{BITO52},{BITO53},{BITO54},{BITO55},{BITO56},    // Control output 49 - 56. Anders 20110826.
	                                                                                                //{BITO57},{BITO58},{BITO59},{BITO60},{BITO61},{BITO62},{BITO63},{BITO64}     // Control output 57 - 64. Anders 20110826.
} };
#endif

INITPARAM u_initparam;

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
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
void Initperipheral()
{
	int i;
	memset(&u_initparam, 0, sizeof(u_initparam));

	for (u_initparam.ad.wID = 1; u_initparam.ad.wID <= ADCHL_USE_APP; u_initparam.ad.wID++)
		Create_AD(&u_initparam.ad); //MAX_AD1
	for (u_initparam.da.wID = 1; u_initparam.da.wID <= MAX_DA; u_initparam.da.wID++)
		Create_DA(&u_initparam.da);
	for (i = 1; i <= MAX_ENCODER; i++)
		Create_Encoder(i);
}

void InitIO()
{
	int i;

	for (i = 0; i < MAX_BITIN; i++)
		Create_BitIn(&g_bitinx.bitin[i]);
	for (i = 0; i < MAX_BITOUT; i++)
		Create_BitOut(&g_bitoutx.bitout[i]);
}
