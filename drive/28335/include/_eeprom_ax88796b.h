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

#ifndef     D__EEPROM_AX88796B
#define     D__EEPROM_AX88796B

#include "platform.h"
#include "_eeprom_Common.h"

#ifdef      __cplusplus
extern      "C" {
#endif

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
void	Enable_EEPROM_ax88796b(int nDeviceHandle);
void	Disable_EEPROM_ax88796b(int nDeviceHandle);
void	EraseAll_EEPROM_ax88796b();
void	Erase_EEPROM_ax88796b(WORD wAddress);
void	WriteAll_EEPROM_ax88796b(WORD wValue);
WORD	ReadString_EEPROM_ax88796b(int nDeviceHandle,WORD wStartAddr, WORD *pData, WORD wSize);
WORD	WriteString_EEPROM_ax88796b(int nDeviceHandle,WORD wStartAddr, WORD *pData, WORD wSize);

#ifdef      __cplusplus
}
#endif

#endif
