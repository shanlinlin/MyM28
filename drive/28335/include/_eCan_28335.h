/*===========================================================================+
|  Function : eCan Driver                                                    |
|  Task     : eCan Driver                                                    |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                   	 |
|  Link     : CCS3.1 -                                                   	 |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : Winston.Yu                                                     |
|  Version  : V1.00                                                          |
|  Creation : 07/03/2009                                                     |
|  Revision :                                                          	     |
|                                                     						 |
|             							                                     |
+===========================================================================*/
#ifndef D___ECAN
#define D___ECAN

#include "common.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
//======
//      eCan Bit Time
//======
#define BITTIME_15 1
#define BITTIME_10 2
//======
//      eCan Baudrate
//======
#define BAUDRATE_1MBPS 1
#define BAUDRATE_500KBPS 2
#define BAUDRATE_250KBPS 3
#define BAUDRATE_125KBPS 4
#define BAUDRATE_100KBPS 5
#define BAUDRATE_50KBPS 6
//======
//      eCan Sampling Point
//======
#define SAMPLINGPOINT_80PERCENT 1
#define SAMPLINGPOINT_73PERCENT 2
#define SAMPLINGPOINT_66PERCENT 3
#define SAMPLINGPOINT_60PERCENT 4
//======
//      eCan Mode
//======
#define CANMODE_STANDARD 1
#define CANMODE_ENHANCED 2
//======
//      eCan Result Code
//======
#define CAN_SUCCESS 0
#define CAN_ERR_FAILURE 1
#define CAN_ERR_RX_EMPTY 2
#define CAN_ERR_TX_FULL 3
#define CAN_ERR_RX_BUSY 4
#define CAN_ERR_TX_BUSY 5
#define CAN_ERR_RX_SMALL 6
#define CAN_ERR_TX_SMALL 6

//======
//      eCan state code
//======
#define CAN_S_CLOSED 0
#define CAN_S_OPENED 1
#define CAN_S_IDLE 2
#define CAN_S_BUSY 3
#define CAN_S_HOLD 4
#define CAN_S_ERROR 5
//======
//      eCan Rx state code. Anders 2011-02-18.
//======
#define CAN_RX_SUCCESS 0
#define CAN_RX_MSGBUFFEROVERFLOW 1
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct tagBAUDRATECONF
{
	WORD wBitTime;
	WORD wBaudrate;
	WORD wSamplingPoint;
} BAUDRATECONF;

typedef struct tagCANCONF
{
	int          nDevice;
	WORD         wCanMode;
	BAUDRATECONF BaudrateConf;
} CANCONF;

typedef union tagMSGID {
	DWORD dwAll;
	struct
	{
		WORD wExtID_L : 16;
		WORD wExtID_H : 2;
		WORD wStdID : 11;
		WORD wReserved : 2;
		WORD wRTR : 1;
	} ID;
} MSGID;

typedef struct tagBYTES
{                     // bits   description
	Uint16 BYTE0 : 8; //
	Uint16 BYTE1 : 8; //
	Uint16 BYTE2 : 8; //
	Uint16 BYTE3 : 8; //
} BYTES;

typedef union tagCANDATA {
	BYTES abData[2];
	WORD  awData[4];
	DWORD adwData[2];
} CANDATA;

typedef struct tagDATAFRAME
{
	MSGID   MsgID;
	CANDATA CanData;
	WORD    wLength;
} DATAFRAME;

typedef struct tag_CANQUEUE
{
	DATAFRAME * pQueue;
	WORD        wInput;
	WORD        wOutput;
	WORD        wMaxSize;
	WORD        wCount;
} _CANQUEUE;

typedef struct tagECAN
{
	CANCONF CanConf;
	WORD    wRxBufferSize; // Size of the receive buffer
	BYTE *  pRxBuffer;     // Pointer to the receive buffer
	WORD    wTxBufferSize; // Size of the transmit buffer
	BYTE *  pTxBuffer;     // Pointer to the transmit buffer
} ECAN;

typedef struct tagHCAN
{
	WORD      wMode;
	WORD      wState;
	WORD      wRxState;
	WORD      wTxState;
	_CANQUEUE queueRx;
	_CANQUEUE queueTx;
} HCAN;

typedef struct tagCANESError
{
	WORD wPDA_ERR;
	WORD wCCE_ERR;
	WORD wSMA_ERR;
	WORD wEW_ERR;
	WORD wEP_ERR;
	WORD wBO_ERR;
	WORD wACKE_ERR;
	WORD wSE_ERR;
	WORD wCRCE_ERR;
	WORD wSA1_ERR;
	WORD wBE_ERR;
	WORD wFE_ERR;
} CANESERROR;

typedef struct tagCAN_REG
{
	volatile struct ECAN_REGS *   pECanRegs;
	volatile struct ECAN_MBOXES * pECanMboxes;
	volatile struct LAM_REGS *    pECanLAMRegs;
	volatile struct MOTO_REGS *   pECanMOTORegs;
	volatile struct MOTS_REGS *   pECanMOTSRegs;
} CAN_REG;

enum CAN_DEVICE
{
	CAN_A,
	CAN_B,
	CAN_DEVICE_MAX
};

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
WORD         _Open_eCan(ECAN * pECan);
WORD         _Close_eCan(int nDevice);
WORD         _eCan_Rx(int nDevice);
void         _eCan_Tx(int nDevice);
WORD         _Read_eCan(int nDevice, WORD * pw, WORD wSize, WORD * pwLength);
WORD         _Write_eCan(int nDevice, WORD * pw, WORD wLength);
WORD         _Write_ISReCan(int nDevice, WORD * pw, WORD wLength);
void         _ResetBaudrate_eCan(int nDevice, BAUDRATECONF * pBaudrateConf);
void         _Reset_eCan(int nDevice);
extern DWORD g_dwBOResetCount[];

#endif
