/*===========================================================================+
|  System   : Injection Machine Control System - Drive Card                  |
|  Task     :                                                                |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : zxc                                                               |
|  Version  : V1.00                                                          |
|  Creation : 2009-4-14                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef D__RS232
#define D__RS232
#ifdef __cplusplus
extern "C" {
#endif

#include "_RS232.h"

// Note : Accumulated all length of <Data Frame> in BYTE (8 bits)
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define MAX_TOKEN_SIZE 128 // 1024 --> 128 zholy081121
//======
//      RS232 default parameter value
//======
#define COM_DEF_HANDSHAKE COM_HS_DTRDSR
#define COM_DEF_BAUDRATE COM_BR_9600
#define COM_DEF_PARITY COM_PR_NONE
#define COM_DEF_STOPBIT COM_SB_1
#define COM_DEF_DATALENGTH COM_DL_8
#define COM_DEF_RTIMEOUT 1000 /* Read  Time out in ms  */
#define COM_DEF_WTIMEOUT 1000 /* Write Time out in ms  */

#define CHECKDATA(wValue) ((wValue >= 0x30 && wValue <= 0x39) || (wValue >= 0x41 && wValue <= 0x5A) \
	        || (wValue == 0x2C) || (wValue == 0x3B)                                                 \
	    ? TRUE                                                                                      \
	    : FALSE)
/*---------------------------------------------------------------------------+
|           RS232 encapsulator                                               |
+---------------------------------------------------------------------------*/
enum TOKENSTATE
{
	TOKEN_NONE,
	TOKEN_STX,
	TOKEN_DATAID,
	TOKEN_PKL,
	TOKEN_DATAFRAME,
	TOKEN_CRC,
	TOKEN_ETX,
	TOKEN_DONE,
	TOKEN_STATE_COUNT
};
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
//======
//		TOKEN
//======
typedef struct tagTOKEN
{
	WORD wDataIDNum;
	WORD wDataNum;
	WORD wPKL;
	WORD wCRC;
	WORD wCurrentLength; // Accumulated length of <Data Frame> in WORD
	WORD wCurrentCRC;    // Accumulated CRC
	WORD wState;         // Token state
} TOKEN;

/*---------------------------------------------------------------------------+
|           Attributes                                                       |
+---------------------------------------------------------------------------*/
typedef struct tagPRS232
{
	RS232 m_rs232;
	WORD  m_wOpStatus;
	WORD  m_wRTimeOut; // Read  time out count in 1/1000 second
	WORD  m_wWTimeOut; // Write time out count in 1/1000 second
	BYTE  m_STX;
	BYTE  m_ETX;

	TOKEN m_token;
} PRS232; // Protocol level RS232 zholy070315

typedef WORD (*pfReadToken)(WORD wChannel, WORD * pw, int * pnIDSize, int * pnDataSize);
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD Create_RS232(RS232 * prs232);
void Destroy_RS232(WORD wChannel);

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/

WORD ReadToken(WORD wChannel, WORD * pw, int * pnIDSize, int * pnDataSize);
WORD WriteToken(WORD wChannel, WORD * pw, int nCount);
void ResetBaudRate(WORD wChannel, WORD wBaudRate);
/*===========================================================================+
|           External                                                         |
+===========================================================================*/
extern PRS232      g_prs232[MAX_COM];
extern pfReadToken pReadToken;

#ifdef __cplusplus
}
#endif

#endif
