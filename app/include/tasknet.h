/*==========================================================================+
|  Function : Network commnunication                                        |
|  Task     : Network commnunication                                        |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : zxc                                                           |
|  Version  : V1.00                                                         |
|  Creation : 2009/07/20                                                    |
|  Revision : 1.0                                                           |
+==========================================================================*/

#ifndef D__TASKNET
#define D__TASKNET

#include "common.h"
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define IPADDR_0 169
#define IPADDR_1 254
#define IPADDR_2 11
#define IPADDR_3 11

#define CMD_NULL NULL
/*---------------------------------------------------------------------------+
|           Type Definition                                                  |
+---------------------------------------------------------------------------*/

typedef struct tagPARAMNET
{
	int nBroadIPHandle;
	int nBroadTechHandle;
	int nBroadTechHandle1;
} PARAMNET;

/*==========================================================================+
|           External                                                        |
+==========================================================================*/
WORD CreateTaskNet();
void DestoryTaskNet();
void RunTaskNet();

#endif
