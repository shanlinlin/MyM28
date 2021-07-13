/*===========================================================================+
|  Function : NET Drive										           		 |
|  Task     : NET Driver Common header file									 |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : zxc                                                          	 |
|  Version  : V1.00                                                          |
|  Creation : 2009-4-9                                                     	 |
|  Revision :                                                                |
+===========================================================================*/

#ifndef D__NETDRIVERCOMMON_H
#define D__NETDRIVERCOMMON_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

#define ETH_ALEN 6
#define CONST_LENGTH_MAC 6 / 2 // netcard physical address length
#define ETH_MAXFRAMELEN 1408   //480->1400 20100225.		// 1514 is enough , however for alignment

#define SR_TOKEN 0x0000
#define SR_QUEUE 0x0001

//******
//      Nic state code
//******
#define NIC_S_CLOSED 0
#define NIC_S_OPENED 1
#define NIC_S_IDLE 2
#define NIC_S_BUSY 3
#define NIC_S_HOLD 4
#define NIC_S_ERROR 5

//******
//      Transmit path limit Flag
//******
#define CSTX_NOPROCESS 0x0000
#define CSTX_INPROCESS 0x0001

//******
//      Nic result code of the most recent method(operation)
//******
#define NIC_OK 0x0000           // Operation status : OK
#define NIC_ERR_DEVICE 0x0001   // Operation status : Illegal device code
#define NIC_ERR_FUNCTION 0x0002 // Operation status : Illegal function code

#define NIC_SUCCESS 0x0000
#define NIC_E_FAILURE 0x0010
#define NIC_E_NOPORT 0x0020
#define NIC_E_OPENED 0x0030
#define NIC_E_CLOSED 0x0040
#define NIC_E_BUSY 0x0050
#define NIC_E_HOLD 0x0060
#define NIC_E_REOPENED 0x0070

#define QUEUE_SUCCESS 0x0000
#define QUEUE_E_EMPTY 0x00A0
#define QUEUE_E_FULL 0x00A1
#define QUEUE_E_SMALL 0x00A2
#define QUEUE_E_FAILURE 0x00A3

#define NIC_TOKEN_SUCCESS 0x10C0
#define NIC_E_TOKEN_FAILURE 0x20C1
#define NIC_E_TOKEN_WAIT 0x30C2
#define NIC_E_TOKEN_EMPTY 0x40C3
#define NIC_E_TOKEN_SMALL 0x50C4

#define NIC_E_BUFFER_SMALL 0x70C6

#define NIC_LINK_OFF 0x0000
#define NIC_LINK_ON 0x0001

typedef struct tagBUFFER
{
	WORD   wRxBufferSize; // Size of the receive buffer
	WORD * pRxBuffer;     // Pointer to the receive buffer
	WORD   wTxBufferSize; // Size of the transmit buffer
	WORD * pTxBuffer;     // Pointer to the transmit buffer
} BUFFER;                 // Note: alignment

/*==========================================================================+
|           Function and Class prototype                                    |
+==========================================================================*/

/*==========================================================================+
|           External					                                    |
+==========================================================================*/
extern BYTEX g_acSmac[3][CONST_LENGTH_MAC];

#ifdef __cplusplus
}
#endif

#endif
