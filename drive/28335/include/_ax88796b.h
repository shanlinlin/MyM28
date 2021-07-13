/*===========================================================================+
|  Function : AX88796B Drive										         |
|  Task     : AX88796B service routine header file							 |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : zholy                                                          |
|  Version  : V1.00                                                          |
|  Creation : 03/20/2007                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef D__AX88796B
#define D__AX88796B

#include "_NetDriveCommon.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef union tagNPR_ {
    WORD w;
    struct
    {
        WORD PKTOK : 1;
        WORD ERRCRC : 1;
        WORD ERRALGN : 1;
        WORD ERRMII : 1;
        WORD RUNT : 1;
        WORD MULTIBD : 1;
        WORD RSVD : 2;
        WORD NEXTPAGE : 8;
    } b;
} NPR_;

typedef union tagLEN_ {
    WORD w;
    struct
    {
        WORD LEN : 11;
        WORD RSVD : 5;
    } b;
} LEN_;

typedef struct tagQHEADER {
    NPR_ npr;
    LEN_ length;
} QHEADER;

typedef struct tagQUEUE {
    WORD*    pQueue;  // Point to buffer head
    WORD     wInput;  // Definition: Next insert position
    WORD     wOutput; // Definition: Next fetch position
    WORD     wEnd;    // Definition: Buffer size
    TBOOL    bFull;
    QHEADER* pQhLastInput; // Not use
} QUEUE;

//// Not use
//typedef struct tagBUFFER
//{
//	WORD        wRxBufferSize;          	// Size of the receive buffer
//    WORD*       pRxBuffer;              	// Pointer to the receive buffer
//    WORD        wTxBufferSize;          	// Size of the transmit buffer
//    WORD*       pTxBuffer;              	// Pointer to the transmit buffer
//} BUFFER;									// Note: alignment

typedef struct tagHETHERNET {
    WORD wState;
    WORD wTxPath; // Limit transmit path. In order that only one transmit is in process at a time
                  //  WORD		wTxBidding;					// Bid for TxCMD: True is waiting for TxCMD; False is permitted to TxCMD
    WORD  wRxState;
    WORD  wTxState;
    WORD  wError;
    WORD  wRxError;
    WORD  wTxError;
    QUEUE queueRx;
    QUEUE queueTx;
    BYTE  acPhysicalAddress[ETH_ALEN];
} HETHERNET;

/*==========================================================================+
|           Function and Class prototype                                    |
+==========================================================================*/
WORD _InitNetDevice_ax88796b();
void _InstallNetDevice_ax88796b();
void _RestoreNetDevice_ax88796b();
WORD _OpenNetDevice_ax88796b();
WORD _CloseNetDevice_ax88796b();
WORD _ReadNetDevice_ax88796b(int nDeviceHandle, WORD* pw, WORD wBufferSize, WORD* pReadCount);
WORD _WriteNetDevice_ax88796b(int nDeviceHandle, WORD* pw, WORD wLength);
WORD _GetLinkStatus_ax88796b();
void _NetDeviceISR_ax88796b();

void _Enable_EEPROM_ax88796b();
void _Disable_EEPROM_ax88796b();
void _SendCMD_EEPROM_ax88796b(int nDeviceHandle, DWORD dwValue, WORD wLength);
WORD _ReceiveCMD_EEPROM_ax88796b(int nDeviceHandle, DWORD dwValue, WORD wLength);

void _RESET_AX88796B(int nDeviceHandle);
int  _mdio_read_ax88796b(int nDeviceHandle, int phy_id, int loc);
void _mdio_write_ax88796b(int nDeviceHandle, int phy_id, int loc, int value);
/*==========================================================================+
|           External					                                    |
+==========================================================================*/
extern WORD g_awEepromMAC[];

#ifdef __cplusplus
}
#endif

#endif
