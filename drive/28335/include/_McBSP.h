/*===========================================================================+
|  Class    : LTC1864                                                        |
|  Task     : LTC1864 Header file                                            |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1	 -                                                   |
|  Link     : CCS3.1	 -                                                   |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : zholy		                                                     |
|  Version  : V1.00                                                          |
|  Creation : 01/13/2011                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D___MCBSP
#define     D___MCBSP

#include    "common.h"

/*===========================================================================+
|           Marco Definition                                                 |
+===========================================================================*/
#define		MCBSP_READ_CMD			0x03
#define		MCBSP_WRITE_CMD			0x02
#define		MCBSP_WRITE_EN			0x06
#define		MCBSP_WRITE_DIS			0x04
#define		MCBSP_READ_STATUS		0x05
#define		MCBSP_WRITE_STATUS		0x01

#define		WDLEN_8BIT		0
#define		WDLEN_12BIT		1
#define		WDLEN_16BIT		2
#define		WDLEN_20BIT		3
#define		WDLEN_24BIT		4
#define		WDLEN_32BIT		5

#define		SetMcBSPWordLength(len)			(McbspaRegs.RCR1.bit.RWDLEN1 = McbspaRegs.XCR1.bit.XWDLEN1 = len)

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/


/*===========================================================================+
|           External                                                         |
+===========================================================================*/


#endif

