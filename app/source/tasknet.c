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
#include "../include/tasknet.h"
#include "../include/tasknet_HMI.h"
#include "../include/tasknet_master.h"
#include "ethernet.h"
//#include "MC_task_EthercatMaster.h"
/*===========================================================================+
|           Function Prototype                                               |
+===========================================================================*/
void DefaultDealIP(WORD * pw, WORD wLength);
void DefaultDealTECH(WORD * pw, WORD wLength);

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

// zholy090723
const BYTEX CNST_INITSRCIP[MAX_NETDEVICE][CONST_LENGTH_IP] = {
	{ { IPADDR_0, IPADDR_1 }, { IPADDR_2, IPADDR_3 } }
};

const IOETH CNST_BROADCASTETH = {
	PROTOCOL_TECH,
	{ { 0xFF, 0xFF }, { 0xFF, 0xFF }, { 0xFF, 0xFF } },
	{ 0 },
	{ 0 },
	0,
	0,
	NULL
};

PARAMNET g_paramnet = { -1, -1 };

//#pragma       DATA_SECTION(u_awEthTxBuffer, "EthernetBuffer")
//WORD      u_awEthTxBuffer[CONST_SIZE_BUFFER/2];
//#pragma       DATA_SECTION(u_awEthRxBuffer, "EthernetBuffer")
//WORD      u_awEthRxBuffer[CONST_SIZE_BUFFER/2];
/*===========================================================================+
|           Class implementation                                             |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD CreateTaskNet()
{
	CONFIGNET confignet;
	WORD      wResult;
	int       i;

	memset(&confignet, 0, sizeof(CONFIGNET));
	//_cs8900a must use buffer.
	//memset(u_awEthTxBuffer, 0, sizeof(u_awEthTxBuffer));
	//memset(u_awEthRxBuffer, 0, sizeof(u_awEthRxBuffer));
	//
	//confignet.buffer.wRxBufferSize        = sizeof(u_awEthRxBuffer);
	//confignet.buffer.pRxBuffer            = u_awEthRxBuffer;
	//confignet.buffer.wTxBufferSize        = sizeof(u_awEthTxBuffer);
	//confignet.buffer.pTxBuffer            = u_awEthTxBuffer;

	// reset all ethernet parameters (must before all doing below !!!)
	ResetParamEth();

	for (i = 0; i < MAX_NETDEVICE; i++)
	{
		confignet.nDeviceHandle = i;

		confignet.pDefaultIPFunc   = &DefaultDealIP;
		confignet.pDefaultTECHFunc = &DefaultDealTECH;

		memcpy(confignet.asrcIP, CNST_INITSRCIP[i], CONST_LENGTH_IP);

		wResult = OpenEthernet(&confignet);
		if (wResult == 0)
			return 0;
	}

	// default open two broadcast session
	//g_paramnet.nBroadIPHandle   = OpenEthSession(0, &CNST_BROADCASTIP);
	//g_paramnet.nBroadTechHandle = OpenEthSession(0, &CNST_BROADCASTETH);

	// test
	//g_paramnet.nBroadIPHandle   = OpenEthSession(0, (IOETH *)&CNST_TEST);
	//g_paramnet.nBroadTechHandle   = OpenEthSession(0, (IOETH *)&CNST_TEST);

	//CreateTaskNet_master();
	//CreateTaskNet_HMI();

	return 1;
}

void DestoryTaskNet()
{
	DestoryTaskNet_master();
	DestoryTaskNet_HMI();

	CloseEthernet();
}

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
void RunTaskNet()
{
	//int       i;
	//
	//for (i=0; i<MAX_NETDEVICE; i++)
	//{
	//    if( (i != ECMASTERDEVICEHANDLE) && (i != DEVICEHANDLE_PC) )
	//        DealData(i);
	//}
	//DealData(DEVICEHANDLE_MASTER);
	//RunTaskNet_master();
	//RunTaskNet_HMI();
}

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
//-------------------------------------------------------------------------------------------------
void DefaultDealIP(WORD * pw, WORD wLength)
{
	//...
}
//-------------------------------------------------------------------------------------------------
void DefaultDealTECH(WORD * pw, WORD wLength)
{
	//...
}
//-------------------------------------------------------------------------------------------------
