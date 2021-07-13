/*===========================================================================+
|  System   : Injection Machine Control System - Drive Card                  |
|  Task     :                                                                |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : zxc                                                                |
|  Version  : V1.00                                                          |
|  Creation : 2009-4-23                                                      |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D__TASKRS232
#define     D__TASKRS232

#ifdef      __cplusplus
extern      "C" {
#endif

#include	"RS232.h"
#include	"common.h"
#include	"_RS232.h"

/*---------------------------------------------------------------------------+
|           const	                                                         |
+---------------------------------------------------------------------------*/
/**Must define COM  here.*/
//#define		USECOM1			1
#define		USECOM2			1

#ifdef		USECOM1
#define		COM1BUFFER		MAX_TOKEN_SIZE
#else
#define		COM1BUFFER		1
#endif

#ifdef		USECOM2
#define		COM2BUFFER		MAX_TOKEN_SIZE
#else
#define		COM2BUFFER		1
#endif

#define		ERR_CREATERS232	0xFFFF;
/*---------------------------------------------------------------------------+
|           type definition                                                  |
+---------------------------------------------------------------------------*/
///Can add protocol type here.
typedef enum tagProtocolType
{
   PROTOCOL_HUALONG
} ProtocolType;

typedef void 	(*pfEchoReadFunc) (BYTE *, int);
typedef	void 	(*pfParseRS232)(WORD wChannel);
typedef const	WORD	*pawTokenStatus;

typedef	struct	tagReadCmdFactor
{
	WORD 			CmdID;
	pfEchoReadFunc	EchoReadFunc;
} ReadCmdFactor;

typedef struct	tagReadTokenFuncMap
{
	ProtocolType	ProtocolID;
	pfReadToken		pReadToken;
} ReadTokenFuncMap;

typedef	struct	tagParseRS232FuncMap
{
	ProtocolType	ProtocolID;
	pfParseRS232	pParseRS232;	
} ParseRS232FuncMap;

typedef struct	tagTokenStatus
{
	ProtocolType	ProtocolID;
	pawTokenStatus	pTokenStatus;		
}TokenStatus;
/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
WORD 	CreateTaskrs232(WORD wChannel,WORD wBaudrate);
void	DestroyTaskrs232(WORD wChannel);
void	RunRS232(WORD wChannel);

#ifdef      __cplusplus
}
#endif

#endif
