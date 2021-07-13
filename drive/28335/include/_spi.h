/*===========================================================================+
|  Function : SPI                                                            |
|  Task     : SPI interrupt service routine                                  |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                   	 |
|  Link     : CCS3.1 -                                                   	 |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : Winston.Yu                                                     |
|  Version  : V1.00                                                          |
|  Creation : 12/19/2008                                                     |
|  Revision :                                                          	     |
|                                                     						 |
|             							                                     |
+===========================================================================*/

#ifndef     D___SPI
#define     D___SPI

#include	"common.h"
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/


/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
#define		SPI_BAUDRATE_10M		1
#define		SPI_BAUDRATE_8M			4
#define		SPI_BAUDRATE_2M			19

#define		SPI_ERROR				0
#define		SPI_SUCCESS				1

#define		SPI_NULL				0
#define		SPI_EEPROM				1
#define		SPI_RESOLVER			2
#define		SPI_EXTERNALAD			3
#define		SPI_ETHERCAT			4

#define		SPI_DA_MAXCHANNEL		2
#define		SPI_AD_MAXCHANNEL		4
#define		SPI_AD_A0B0CHANNEL		0
#define		SPI_AD_A1B1CHANNEL		1
#define		SPI_AD_4CHANNEL			2

#define		ESC_NOP					0				// no operation
#define 	ESC_RD					0x02			// read acces to ESC
#define		ESC_WR					0x04			// write acces to ESC

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef		struct	tagSPI
{
	WORD	wBaudRate;
	WORD	wCurrentDevID;
} SPI;
   
/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

/*===========================================================================+
|           Class implementation - Encoder                                   |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD		_OpenSPI(SPI* pSpi);
WORD		_CloseSPI(SPI* pSpi);

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
WORD		_SetSPIBaudRate(WORD	wBaudRate);
WORD		_ConfigResolver();
WORD		_ConfigEEPROM();
WORD		_ConfigExternalAD();
WORD		_ReadResolverBySPI(long*	plValue);
WORD		_ReadEEPROMBySPI(WORD wAddress, int* pnData, WORD	wLength);
WORD		_ReadExternalAD(WORD wChannel, WORD* pwValue);
WORD		_WriteEEPROMBySPI(WORD wAddress, int* pnData, WORD	wLength);
WORD		_SetDAValue(WORD wChannel, int nValue);
int         GetDAChannel1();
int         GetDAChannel2();
void 		_WriteET1100BySPI(WORD *pData, WORD wAddress, WORD wLength);
void 		_ReadET1100BySPI(WORD *pData, WORD wAddress, WORD wLength);
void 		_ReadET1100IntRegBySPI(WORD *pData);

#endif

