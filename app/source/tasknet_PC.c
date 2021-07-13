/*===========================================================================+
|  Function : Network commnunication                                         |
|  Task     : Network commnunication Task                                    |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                       |
|  Link     : CCS3.1 -                                                       |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   :                                                                |
|  Version  :                                                                |
|  Creation :                                                                |
|  Revision :                                                                |
|                                                                            |
|                                                                            |
+===========================================================================*/
#include "tasknet_PC.h"
#include "Error_App.h"
#include "Task_Led.h"
#include "tasknet.h"
#include "common.h"
#include "ethernet.h"
#include "tasknet_master.h"
#include "platform.h"
/*===========================================================================+
|           Function Prototype                                               |
+===========================================================================*/
static void ParsePCNetData(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqActValue(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqCommand(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqCurveValue(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqMsgValue(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqReadRefValue(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqWriteRefValue(int nSessionHandle, WORD* pdata, WORD wlength);
static void PC_ReqLinkState(int nSessionHandle, WORD* pdata, WORD wlength);

static void PC_ReqMoldsetState(int nSessionHandle, WORD* pdata, WORD wlength);
static void ReqMoldsetUpdate(int nSessionHandle, WORD* pdata, WORD wlength);
static void RespActValue(int nSessionHandle);
static void RespRefValue(int nSessionHandle);

static void RespCurveValue(int nSessionHandle); // Anders 20100523
static void RespCommand(int nSessionHandle, DWORD dwRespCmd);
//static  void  RespPC_LinkState(int nSessionHandle);
static void PC_RespMoldsetState(int nSessionHandle);
//static  void  RespMoldsetUpdate(int nSessionHandle);
static void RespMsgValue(int nSessionHandle);

static void SetCmdBurn(int nSessionHandle, DWORD dwCommand, WORD wDataType);
static void SetCmdResetAlarm(int nSessionHandle, DWORD dwCommand, WORD wDataType);
static void EchoMsgValue(int nSessionHandle, WORD* pdata, WORD wlength);
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define RESPONSE_ACTUAL_ID 1001
#define RESPONSE_READ_REFERENCE_ID 1002
#define RESPONSE_WRITE_REFERENCE_ID 1003
#define RESPONSE_MESSAGE_ID 1004
#define RESPONSE_CURVE_ID 1005
#define RESPONSE_COMMAND_ID 1006
#define RESPONSE_LINKSTATE_ID 1007
#define RESPONSE_MOLDSETSTATE_ID 1008
#define RESPONSE_MOLDSETUPDATE_ID 1009
#define HMIWRONEDATALEN 22

//#define       ECHO_MESSAGE_ID             2004
#define MOTORCOMMAP_LEN (sizeof(g_PC_ctrlcmdmap) / sizeof(CTRLCMDMAP) - 1)

#define HEADER_REQ 0x8899 // header request
#define HEADER_RES 0x5588 // header response
#define FOOTER_REQ 0x6688 // footer request
#define FOOTER_RES 0x1166 // footer response

const COMMMAP g_PC_commmap[] = {
	{ COMM_ACTUAL, &ReqActValue },
	{ COMM_READPARAMETER, &ReqReadRefValue },
	{ COMM_WRITEPARAMETER, &ReqWriteRefValue },
	{ COMM_MESSAGE, &ReqMsgValue },
	{ COMM_CURVE, &ReqCurveValue },
	{ COMM_MOTIONCOMMAND, &ReqCommand },
	{ COMM_LINKSTATE, &PC_ReqLinkState },
	{ COMM_MOLDSETSTATE, &PC_ReqMoldsetState },
	{ COMM_MOLDSETUPDATE, &ReqMoldsetUpdate },
	{ ECHO_MESSAGE_ID, &EchoMsgValue },
	{ COMM_END, NULL }
};

const CTRLCMDMAP g_PC_ctrlcmdmap[] = {
	{ CTRLMCCMD_RESETALARM, &SetCmdResetAlarm },
	{ CTRLMCCMD_BURN, &SetCmdBurn },
	{ 0, NULL }
};
const IOETH CNST_PCIP = {
	PROTOCOL_IP,
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 } },
	{ { IPADDR_0, IPADDR_1 }, { IPADDR_2, 10 } }, // Anders 2013-3-19.
	//{{IPADDR_0, IPADDR_1}, {IPADDR_2, 51}},
	// shanll 20210619 modify{ { 1, 2 }, { 3, 4 } },
	{ { 0, 0 }, { 0, 0 } },
	2310,
	2310,
	ParsePCNetData
};

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

#pragma DATA_SECTION(u_PC_TempBuffer, "EthernetBuffer")
#pragma DATA_SECTION(u_PC_CurveBuffer, "EthernetBuffer")
//#pragma       DATA_SECTION(u_PC_RespList, "EthernetBuffer")

TEMPBUFFER u_PC_TempBuffer;
WORD       u_PC_wRespActCounter          = 1;
WORD       u_PC_wRespReadRefCounter      = 1;
WORD       u_PC_wRespWriteRecCounter     = 1;
WORD       u_PC_wRespAlarmCounter        = 1;
WORD       u_PC_wRespMoldsetStateCounter = 1;
WORD       u_PC_wRespMoldsetRecCounter   = 1;
RESPLIST   u_PC_RespList[RESP_MAX]       = {
    { 0, 0, 0, { 0 }, { (DBIndex_T)0 }, 0, &RespActValue },
    { 0, 0, 0, { 0 }, { (DBIndex_T)0 }, 0, &RespRefValue },
    { 0, 0, 0, { 0 }, { (DBIndex_T)0 }, 0, &RespCurveValue }
};
LINKSTATE    PC_LinkState    = { 0, 0 };
MSGSTATE     PC_MsgState     = { 0, 0, 0 };
MOLDSETSTATE PC_MoldsetState = { 0, 0, 0, 0 };

int  g_PC_nNetSession       = -1;
int  g_55_nNetSession       = -1; // Anders 2014-12-24, Add.
WORD g_PC_wLastMoldsetCNT   = 0;
WORD g_PC_wNetFrameErrorCNT = 0;
WORD g_wTestRespMsgValueCNT = 0;

DWORD u_PC_dwCurveTimeAxis_tick = 0;
DWORD u_PC_dwCurveTimeAxis      = 0;
WORD  u_PC_wRespCurveSetNum     = 0;
WORD  u_PC_CurveBuffer[MAX_SEND_CURVEDATA];
//WORD        u_PC_wMaxRespCurveSetNum = 0;
//WORD        u_PC_wRespCurveBufferFull = 0;
DWORD u_PC_dwRespCurveRecCounter = 0;
DWORD g_PC_dwLastError[10]       = { 0 };
/*===========================================================================+
|           Class implementation                                             |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/

WORD CreateTaskNet_PC()
{
	int i;

	g_PC_nNetSession = OpenEthSession(DEVICEHANDLE_PC, (IOETH*)&CNST_PCIP);
	g_55_nNetSession = OpenEthSession(0, (IOETH*)&CNST_PCIP); // DEVICEHANDLE_MASTER
	//g_PC_nNetSession = OpenEthSession(2, (IOETH *)&CNST_PCIP);
	//g_55_nNetSession = OpenEthSession(1, (IOETH *)&CNST_PCIP);    // Anders 2014-12-24, Add.
	for (i = 0; i < RESP_MAX; i++)
	{
		//memset(&u_PC_RespList[i].awIDList,0,sizeof(u_PC_RespList[i].awIDList));
		memset(&u_PC_RespList[i].aiIndexList, 0, sizeof(u_PC_RespList[i].aiIndexList)); // Anders 20110126.
		//AddDnCounter(&u_PC_RespList[i].lCounter);
		ResetTimer(&u_PC_RespList[i].lCounter, 0);
	}
	//AddDnCounter(&PC_LinkState.lCounter);
	//AddDnCounter(&PC_MsgState.lCounter);
	ResetTimer(&PC_LinkState.lCounter, 0);
	ResetTimer(&PC_MsgState.lCounter, 0);
	memset(&u_PC_TempBuffer, 0, sizeof(u_PC_TempBuffer));
	u_PC_TempBuffer.TMHeader.wHeader = HEADER_RES;

	//Create_UnTimerintTask(1, RunTaskNet_PC, NULL);
	return 1;
}

void DestoryTaskNet_PC()
{
	//int       i;
	//
	//for (i=0; i<RESP_MAX; i++)
	//  DeleteDnCounter(&u_PC_RespList[i].lCounter);
	//DeleteDnCounter(&PC_LinkState.lCounter);
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
extern WORD GetMoldsetState(void);
extern WORD g_wMasterReady;
void        RunTaskNet_PC()
{
	int  i;
	BOOL bErrorFlag = FALSE, bChanged = FALSE;
	//TAxisController  *pAxisController;
	MSGDEVICE   Device;
	DeviceError ErrorEntry;
	BOOL        bErrorLed = FALSE;
	//memset(&u_PC_TempBuffer, 0, sizeof(u_PC_TempBuffer));
	//_WriteNetDevice(DEVICEHANDLE_PC, (WORD*)&u_PC_TempBuffer, 32);
	//MC_DBData[AXIS1_INTERFACE_ERROR11].dbValue.dwData++;

	if (CheckTimerOut(&PC_LinkState.lCounter) || _GetLinkStatus(DEVICEHANDLE_PC) == NIC_LINK_OFF)
	{
		if (PC_LinkState.wState)
		{
			for (i = 0; i < RESP_MAX; i++)
			{
				u_PC_RespList[i].wFlag       = 0;
				u_PC_RespList[i].wCycle      = 0;
				u_PC_RespList[i].wNoElements = 0;
				//memset(u_PC_RespList[i].awList, 0, MAX_REQUEST_NO);
			}
			PC_LinkState.wState = 0;
			//memset(&PC_MoldsetState, 0, sizeof(PC_MoldsetState));
			//DisactivateEthSession(g_PC_nNetSession);
			//ClearBindAddrToSession(g_PC_nNetSession);

			DisactivateEthSession(g_PC_nNetSession);
			DisactivateEthSession(g_55_nNetSession); // Anders 2014-12-24, Add.
			memset(&g_PC_dwLastError, 0, sizeof(g_PC_dwLastError));
		}
		//PC_LinkState.lCounter = -1;
		ResetTimer(&PC_LinkState.lCounter, 0);
	}
	for (i = 0; i < 10; i++)
	{
		ErrorEntry = Get_MasterError((ACTAXIS)i); // Anders 2017-1-4 modify. ReadDB_DWORD(cnst_awAxisIndex[i].wError_Idx);
		if (g_PC_dwLastError[i] != ErrorEntry.dwAll)
		{
			//if (ErrorEntry.dwAll > 0)
			if (ErrorEntry.dwAll > 0 && ErrorEntry.info.bErrorType == ERRORTYPE_ALARM) // Anders 2018-9-19, modify.
			{
				Device.bit.bDeviceID  = ErrorEntry.info.bObjectID;
				Device.bit.bDeviceNum = ErrorEntry.info.bSubIndex; //ErrorEntry.info.bSubIndex;
				Device.bit.bAlarmType = ErrorEntry.info.bErrorType;

				PostMsg_PC(Device, ErrorEntry.info.wError);
				//ResetTimer(&PC_MsgState.lCounter, 0);
				bErrorFlag = TRUE;
			}
			g_PC_dwLastError[i] = ErrorEntry.dwAll;
			bChanged            = TRUE;

			//   if ( g_AxisManager[i].Error.dwAll == 0 )                // Anders 2015-3-25 Add for MultiPump Error.
			// g_AxisManager[i].Error.dwAll = ErrorEntry.dwAll; //WriteDB_DWORD(MOLD_INTERFACE_ERROR1, ErrorEntry.dwAll);
		}
		bErrorLed |= (ErrorEntry.dwAll && ErrorEntry.info.bErrorType == ERRORTYPE_ALARM); // ErrorEntry.dwAll; Anders 2018-9-19, modify. // Anders 2017-1-4, add.
	}
	//response actual value
	if (PC_LinkState.wState == 1)
	{
		for (i = 0; i < RESP_MAX; i++)
		{
			if (u_PC_RespList[i].wFlag && CheckTimerOut(&u_PC_RespList[i].lCounter))
			{
				if (u_PC_RespList[i].pFunc != NULL)
				{
					if (GetSessionchState(g_PC_nNetSession) == STATESESSIONCH_ACTIVE) // Anders 2014-12-24, Add.
						u_PC_RespList[i].pFunc(g_PC_nNetSession);
					else
						u_PC_RespList[i].pFunc(g_55_nNetSession);
				}
				//u_PC_RespList[i].lCounter = u_PC_RespList[i].wCycle;
				ResetTimer(&u_PC_RespList[i].lCounter, u_PC_RespList[i].wCycle);
				if (u_PC_RespList[i].wCycle == 0)
					u_PC_RespList[i].wFlag = 0;
			}
		}
		if (bChanged && bErrorFlag == FALSE)
		{
			Device.wAll = 0;
			PostMsg_PC(Device, 0); // Anders 2014-10-13, Modified PostMsg -> PostMsg_PC
			                       //ResetTimer(&PC_MsgState.lCounter, 0);
		}
		if ((PC_MsgState.wFlag && CheckTimerOut(&PC_MsgState.lCounter)) || (PC_MsgState.bGetEchoState == FALSE && CheckTimerOut(&PC_MsgState.lCounter))) //PC_MsgState.wCount &&
			RespMsgValue(g_PC_nNetSession);
	}

	// THJ 2017-1-17 Modify:{
	//if ( !GetMoldsetState() )    // Anders 2015-3-17, add.
	//{
	//    if ( g_wMasterReady )
	//        //CreateTaskLed(RT_CPUHEART, 1000);   // Anders 2015-9-15, Add.
	//        CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_1000MS);
	//    else
	//    {
	//        //DestoryTaskLed(RT_CPUHEART);
	//        //LED_OFF(LED_CPUHEART);
	//        CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_OFF);
	//    }
	//}
	////else if ( ReadDB_DWORD(DBINDEX_MOLD0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_MOLD1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_EJECT0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_EJECT1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_INJECT0_INTERFACE_ERROR1)
	////          | ReadDB_DWORD(DBINDEX_INJECT1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CHARGE0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CHARGE1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CARRIAGE0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CARRIAGE1_INTERFACE_ERROR1) )
	////  CreateTaskLed(RT_CPUHEART, 100);
	//else if ( dwTemp )
	//{
	//  CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_100MS);
	//}
	//else
	//{
	//    //DestoryTaskLed(RT_CPUHEART);
	//    //LED_ON(LED_CPUHEART);
	//    CompLed_SetShow(&g_sLed_Alarm, LEDSHOW_ON);
	//}
	if (!GetMoldsetState())
	{
		Led_MasterLink(0 != g_wMasterReady);
		Led_HMILink(FALSE);
	}
	else
	{
		Led_HMILink(TRUE);
		Led_Error(bErrorLed);
	}
	//}
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
static void ParsePCNetData(int nSessionHandle, WORD* pdata, WORD wlength)
{
	int       i = 0;
	WORD      wType, wHeader, wFooter, wDataSize = 0;
	TMHEADER* pHeader;

	pHeader   = (TMHEADER*)pdata;
	wDataSize = pHeader->wSize;
	//nLeftSize = (int)wlength - (int)wDataSize;
	wHeader = pHeader->wHeader;
	wFooter = *((WORD*)pHeader + (wDataSize >> 1) - 1);
	wType   = pHeader->wType;

	if (wDataSize <= wlength && wHeader == HEADER_REQ && wFooter == FOOTER_REQ)
	{
		while (g_PC_commmap[i].wCmd != COMM_END)
		{
			if (wType == g_PC_commmap[i].wCmd)
			{
				if (g_PC_commmap[i].pFunc != NULL)
					g_PC_commmap[i].pFunc(nSessionHandle, pdata, wlength);
				break;
			}
			i++;
		}
	}
	else
		g_PC_wNetFrameErrorCNT++;
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
static void ReqActValue(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER* pHeader = (TMHEADER*)pdata;
	DWORD *   pdwID, dwID;
	WORD      i;

	u_PC_RespList[RESP_ACTUAL].wFlag = 0;//shanll 20200617 add
	u_PC_RespList[RESP_ACTUAL].wCycle      = pHeader->datatype.wCycle;
	u_PC_RespList[RESP_ACTUAL].wNoElements = pHeader->wNoElements;
	if (u_PC_RespList[RESP_ACTUAL].wNoElements == 0)
	{
		u_PC_RespList[RESP_ACTUAL].wFlag = 0;
		return;
	}
	else if (u_PC_RespList[RESP_ACTUAL].wNoElements > MAX_REQUEST_NO)
		u_PC_RespList[RESP_ACTUAL].wNoElements = MAX_REQUEST_NO; // Anders 2013-3-21, Add.
	//shanll 20200617 move to the end
	//u_PC_RespList[RESP_ACTUAL].wFlag = 1;
	//ResetTimer(&u_PC_RespList[RESP_ACTUAL].lCounter, 0);
	pdwID = (DWORD*)(pdata + sizeofW(TMHEADER));
	for (i = 0; i < u_PC_RespList[RESP_ACTUAL].wNoElements; i++)
	{
		memcpy(&dwID, pdwID, sizeof(DWORD));
		u_PC_RespList[RESP_ACTUAL].adwIDList[i] = dwID;
		if (dwID >= 0x10000000)
			u_PC_RespList[RESP_ACTUAL].aiIndexList[i] = DataIDToIndex(dwID - 0x10000000);

		++pdwID; //Anders 2013-12-27.
	}
	//shanll 20200617 move
	u_PC_RespList[RESP_ACTUAL].wFlag = 1;
	ResetTimer(&u_PC_RespList[RESP_ACTUAL].lCounter, 0);
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
static void ReqReadRefValue(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER* pHeader = (TMHEADER*)pdata;
	DWORD *   pdwID, dwID;
	WORD      i;

	u_PC_RespList[RESP_READPARAMETER].wFlag = 0;//shanll 20200617 add
	u_PC_RespList[RESP_READPARAMETER].wCycle      = pHeader->datatype.wCycle;
	u_PC_RespList[RESP_READPARAMETER].wNoElements = pHeader->wNoElements;
	//ResetTimer(&u_PC_RespList[RESP_READPARAMETER].lCounter, 0); // Anders 2015-11-17, Add.
	if (u_PC_RespList[RESP_READPARAMETER].wNoElements == 0)
	{
		u_PC_RespList[RESP_READPARAMETER].wFlag = 0;
		return;
	}
	else if (u_PC_RespList[RESP_READPARAMETER].wNoElements > MAX_REQUEST_NO)
		u_PC_RespList[RESP_READPARAMETER].wNoElements = MAX_REQUEST_NO; // Anders 2013-3-21, Add.
	//shanll 20200617 move to the end
	//u_PC_RespList[RESP_READPARAMETER].wFlag = 1;
	pdwID  = (DWORD*)(pdata + sizeofW(TMHEADER));
	for (i = 0; i < u_PC_RespList[RESP_READPARAMETER].wNoElements; i++)
	{
		memcpy(&dwID, pdwID, sizeof(DWORD));
		u_PC_RespList[RESP_READPARAMETER].adwIDList[i] = dwID;
		if (dwID >= 0x10000000)
			u_PC_RespList[RESP_READPARAMETER].aiIndexList[i] = DataIDToIndex(dwID - 0x10000000);
		pdwID++; // Anders 2013-12-27.
	}
	//shanll 20200617 move
	u_PC_RespList[RESP_READPARAMETER].wFlag = 1;
	ResetTimer(&u_PC_RespList[RESP_READPARAMETER].lCounter, 0); // Anders 2015-11-17, Add.
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
static void ReqWriteRefValue(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER*    pHeader = (TMHEADER*)pdata;
	WORD         wNoElements, wLength;
	TM_PARADATA *pReadParaData, *pWriteParaData;
	DWORD        dwValue = 0, dwID;
	WORD         i;

	wNoElements = pHeader->wNoElements;
	if (wNoElements > MAX_REQUEST_NO)
		wNoElements = MAX_REQUEST_NO; // Anders 2013-3-21, Add.

	pReadParaData  = (TM_PARADATA*)(pdata + sizeofW(TMHEADER));
	pWriteParaData = (TM_PARADATA*)u_PC_TempBuffer.awData;
	memset(pWriteParaData, 0, sizeof(u_PC_TempBuffer.awData));

	for (i = 0; i < wNoElements; i++)
	{
		memcpy(&dwValue, &pReadParaData->wData1, sizeof(DWORD));
		memcpy(&dwID, &pReadParaData->wIDl, sizeof(DWORD));

		memcpy(pWriteParaData, pReadParaData, sizeof(TM_PARADATA));
		pWriteParaData->stattype.bit.wState = 0;

		if (dwID >= 0x10000000)
		{
			//DBVALUE       dbValue;
			DBIndex_T tDBIndex;
			//dbValue.dwData    =   dwValue;
			tDBIndex = DB_IDToIndex(dwID - 0x10000000);
			if (DBRETCODE_SUCCESS != DB_SetDataByUserIndex(tDBIndex, &dwValue)) //(DB_SUCCESS != SetDBValueByIndex(wDBIndex  ,dbValue))
			{
				//dwValue       =   GetDBValue(dwID).dwData;
				dwValue = 0;
				DB_GetDataByUserIndex(tDBIndex, &dwValue);
				memcpy(&pWriteParaData->wData1, &dwValue, sizeof(DWORD));
			} else {
				pWriteParaData->stattype.bit.wState = 1;
			}
		}
		pReadParaData++;
		pWriteParaData++;
	}

	wLength                                       = (sizeofW(TMHEADER) + sizeofW(TM_PARADATA) * wNoElements + 1) << 1;
	u_PC_TempBuffer.TMHeader.wSize                = wLength;
	u_PC_TempBuffer.TMHeader.wCounter             = 0;
	u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_wRespWriteRecCounter++;
	u_PC_TempBuffer.TMHeader.wType                = RESPONSE_WRITE_REFERENCE_ID;
	u_PC_TempBuffer.TMHeader.wNoElements          = wNoElements;

	*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

	SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
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
static void ReqMsgValue(int nSessionHandle, WORD* pdata, WORD wlength)
{
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
static void ReqCurveValue(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER* pHeader = (TMHEADER*)pdata;
	DWORD *   pdwID, dwID;
	WORD      wLength;
	WORD      i;

	u_PC_RespList[RESP_CURVE].wCycle      = pHeader->datatype.wCycle;
	u_PC_RespList[RESP_CURVE].wNoElements = pHeader->wNoElements;
	if (u_PC_RespList[RESP_CURVE].wNoElements > MAX_CURVE_ELEMENT)
		u_PC_RespList[RESP_CURVE].wNoElements = MAX_CURVE_ELEMENT;
	//u_PC_RespList[RESP_CURVE].nSessionHandle = nSessionHandle;
	ResetTimer(&u_PC_RespList[RESP_CURVE].lCounter, 2);

	if (u_PC_RespList[RESP_CURVE].wNoElements == 0 || u_PC_RespList[RESP_CURVE].wCycle == 0)
	{
		u_PC_RespList[RESP_CURVE].wFlag = 0;
		//u_PC_RespList[RESP_CURVE].nSessionHandle = -1;

		wLength                                  = 2 * (sizeof(TMHEADER)) + 2;
		u_PC_TempBuffer.TMHeader.wSize           = wLength;
		u_PC_TempBuffer.TMHeader.wType           = RESPONSE_CURVE_ID;
		u_PC_TempBuffer.TMHeader.wCounter        = 0;
		u_PC_TempBuffer.TMHeader.datatype.wCycle = 0;
		u_PC_TempBuffer.TMHeader.wNoElements     = 0;
		memcpy(u_PC_TempBuffer.awData, (pdata + sizeof(TMHEADER)), sizeof(long));
		*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

		SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
		return;
	}

	u_PC_RespList[RESP_CURVE].wFlag = 1;
	//ResetTimer(&u_PC_dwCurveTimeAxis);  // u_PC_dwCurveTimeAxis = 0;    Anders 2015-2-24, Modify.
	u_PC_dwRespCurveRecCounter = 0; // Anders 2016-6-22, Add.
	u_PC_wRespCurveSetNum      = 0;
	pdwID                      = (DWORD*)(pdata + sizeof(TMHEADER));
	for (i = 0; i < u_PC_RespList[RESP_CURVE].wNoElements; i++)
	{
		memcpy(&dwID, pdwID, sizeof(DWORD));
		u_PC_RespList[RESP_CURVE].adwIDList[i] = dwID;
		if (dwID >= 0x10000000)
			u_PC_RespList[RESP_CURVE].aiIndexList[i] = DataIDToIndex(dwID - 0x10000000);
		pdwID++; //Anders 2013-12-27.
	}
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
static void ReqCommand(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER* pHeader = (TMHEADER*)pdata;
	//DWORD     *pdwID;
	WORD   wNoElements;
	DWORD *pValue, dwCmd;
	WORD   i, j;
	wNoElements = pHeader->wNoElements;
	if (wNoElements > 100)
		wNoElements = 100;
	pValue = (DWORD*)(pdata + sizeofW(TMHEADER));
	for (i = 0; i < wNoElements; i++)
	{
		memcpy(&dwCmd, pValue + i, sizeof(dwCmd));

		for (j = 0; j < MOTORCOMMAP_LEN; j++)
		{
			if (dwCmd == g_PC_ctrlcmdmap[j].dwCmd)
			{
				if (g_PC_ctrlcmdmap[j].pFunc != NULL)
				{
					g_PC_ctrlcmdmap[j].pFunc(nSessionHandle, dwCmd, pHeader->datatype.wCycle);
					break;
				}
			}
		}
	}
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

static void RespActValue(int nSessionHandle)
{
	TM_PARADATA* pParaData = (TM_PARADATA*)u_PC_TempBuffer.awData;
	TMHEADER*    pTMHeader = &u_PC_TempBuffer.TMHeader;
	WORD         i, wLength, wIndex;
	DWORD        dwID, dwData = 0;

	if (u_PC_RespList[RESP_ACTUAL].wNoElements == 0)
		return;
	wLength = (sizeofW(TMHEADER) + sizeofW(TM_PARADATA) * u_PC_RespList[RESP_ACTUAL].wNoElements + 1) << 1;
	memset(u_PC_TempBuffer.awData, 0, MAX_BUFFER_NO); // Anders 2013-11-18, Modified .(wLength>>1)*sizeof(WORD) -> MAX_BUFFER_NO.
	pTMHeader->wSize                = wLength;
	pTMHeader->wCounter             = 0;
	pTMHeader->wType                = RESPONSE_ACTUAL_ID;
	pTMHeader->datatype.wCounterRet = u_PC_wRespActCounter++;
	pTMHeader->wNoElements          = u_PC_RespList[RESP_ACTUAL].wNoElements;

	*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

	for (i = 0; i < u_PC_RespList[RESP_ACTUAL].wNoElements; i++)
	{
		// Anders 20091210
		wIndex = u_PC_RespList[RESP_ACTUAL].aiIndexList[i]; // Anders 20110126.
		//pParaData->dwID = u_PC_RespList[RESP_ACTUAL].awIDList[i];
		dwID = u_PC_RespList[RESP_ACTUAL].adwIDList[i];
		memcpy(&pParaData->wIDl, &dwID, sizeof(DWORD));
		pParaData->stattype.bit.wState = 0;
		dwData                         = 0;
		if (dwID >= 0x10000000)
		{
			pParaData->stattype.bit.wState    = 1;
			pParaData->stattype.bit.wDataType = DB_GetDBDataType((DBIndex_T)wIndex);
			DB_GetDataByUserIndex((DBIndex_T)wIndex, &dwData); //dwData      =   GetDBValueByIndex(wIndex).dwData;
		}
		memcpy(&pParaData->wData1, &dwData, sizeof(DWORD));
		++pParaData;
	}
	SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
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

static void RespRefValue(int nSessionHandle)
{
	TM_PARADATA* pParaData = (TM_PARADATA*)u_PC_TempBuffer.awData;
	WORD         i, wLength, wIndex;
	DWORD        dwID, dwData;

	memset(u_PC_TempBuffer.awData, 0, MAX_BUFFER_NO);
	if (u_PC_RespList[RESP_READPARAMETER].wNoElements == 0)
		return;
	wLength                                       = (sizeofW(TMHEADER) + sizeofW(TM_PARADATA) * u_PC_RespList[RESP_READPARAMETER].wNoElements + 1) << 1;
	u_PC_TempBuffer.TMHeader.wSize                = wLength;
	u_PC_TempBuffer.TMHeader.wCounter             = 0;
	u_PC_TempBuffer.TMHeader.wType                = RESPONSE_READ_REFERENCE_ID;
	u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_wRespReadRefCounter++;
	u_PC_TempBuffer.TMHeader.wNoElements          = u_PC_RespList[RESP_READPARAMETER].wNoElements;

	*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

	for (i = 0; i < u_PC_RespList[RESP_READPARAMETER].wNoElements; i++)
	{
		wIndex = u_PC_RespList[RESP_READPARAMETER].aiIndexList[i]; // Anders 20110126.
		dwID   = u_PC_RespList[RESP_READPARAMETER].adwIDList[i];
		memcpy(&pParaData->wIDl, &dwID, sizeof(DWORD));
		pParaData->stattype.bit.wState = 0;
		dwData                         = 0;
		if (dwID >= 0x10000000)
		{
			pParaData->stattype.bit.wState    = 1;
			pParaData->stattype.bit.wDataType = DB_GetDBDataType((DBIndex_T)wIndex);
			DB_GetDataByUserIndex((DBIndex_T)wIndex, &dwData); //dwData      =   GetDBValueByIndex(wIndex).dwData;
		}
		memcpy(&pParaData->wData1, &dwData, sizeof(DWORD));
		++pParaData;
	}
	SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
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

//void  RespWriteRefValue(int nSessionHandle)
//{
//}

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
static void RespMsgValue(int nSessionHandle)
{
	WORD     i, wLength;
	RESPMSG* pRespMsg = (RESPMSG*)u_PC_TempBuffer.awData;

	wLength                                       = (sizeof(TMHEADER) + sizeof(RESPMSG) * PC_MsgState.wCount + 1) << 1;
	u_PC_TempBuffer.TMHeader.wSize                = wLength;
	u_PC_TempBuffer.TMHeader.wCounter             = 0;
	u_PC_TempBuffer.TMHeader.wType                = RESPONSE_MESSAGE_ID;
	u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_wRespAlarmCounter++;
	u_PC_TempBuffer.TMHeader.wNoElements          = PC_MsgState.wCount;
	for (i = 0; i < PC_MsgState.wCount; i++)
	{
		pRespMsg->msgtype.msgbit.bDeviceID  = PC_MsgState.Device[i].bit.bDeviceID;
		pRespMsg->msgtype.msgbit.bDeviceNum = PC_MsgState.Device[i].bit.bDeviceNum;
		pRespMsg->msgtype.msgbit.bAlarmType = PC_MsgState.Device[i].bit.bAlarmType;
		pRespMsg->wMsgID                    = PC_MsgState.awMsgID[i];
		pRespMsg++;
	}
	*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;
	PC_MsgState.bGetEchoState                       = FALSE;
	//PC_MsgState.lCounter = ECHO_COUNTER;
	ResetTimer(&PC_MsgState.lCounter, ECHO_COUNTER);

	SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
	PC_MsgState.wFlag = 0;

	g_wTestRespMsgValueCNT++; // Anders 2015-11-25, Add.
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
static void RespCommand(int nSessionHandle, DWORD dwRespCmd)
{
	WORD wLength;

	wLength                                  = (sizeof(TMHEADER) + 3) << 1;
	u_PC_TempBuffer.TMHeader.wSize           = wLength;
	u_PC_TempBuffer.TMHeader.wType           = RESPONSE_COMMAND_ID;
	u_PC_TempBuffer.TMHeader.wCounter        = 0;
	u_PC_TempBuffer.TMHeader.datatype.wCycle = 0;
	u_PC_TempBuffer.TMHeader.wNoElements     = 1;
	memcpy(u_PC_TempBuffer.awData, (WORD*)&dwRespCmd, sizeof(DWORD));
	*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

	SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
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
static void RespCurveValue(int nSessionHandle)
{
	WORD      i, wLength, wCurveLength;
	DWORD     dwData; // DBVALUE      dbValue;
	CURVEDATA CurveData;

	if (u_PC_RespList[RESP_CURVE].wNoElements == 0)
		return;
	wCurveLength = (u_PC_RespList[RESP_CURVE].wNoElements + 1) << 1;
	if (u_PC_dwRespCurveRecCounter == 0 && u_PC_wRespCurveSetNum == 0) // Anders 2016-6-27, Add "&& u_PC_wRespCurveSetNum == 0"
	{
		u_PC_dwCurveTimeAxis_tick = ReadCalcLoadCpuTimerCounter();
		u_PC_dwCurveTimeAxis      = 0; // Anders 2016-6-22, Add.
		CurveData.dwCurveTimeAxis = 0;
	}
	else {
		DWORD tick;
		DWORD ms;

		tick = ReadCalcLoadCpuTimerCounter();
		ms = TICK_COUNT_DOWN(0, u_PC_dwCurveTimeAxis_tick, tick) / 150000; // Δ ms.
		if (0 == ms) {
			return;
		}
		u_PC_dwCurveTimeAxis_tick -= ms * 150000;
		CurveData.dwCurveTimeAxis = u_PC_dwCurveTimeAxis += ms; // u_PC_dwCurveTimeAxis; Anders 2015-2-24, Modify.
	}
	for (i = 0; i < u_PC_RespList[RESP_CURVE].wNoElements; i++)
	{
		if (u_PC_RespList[RESP_CURVE].adwIDList[i] >= 0x10000000)
		{
			dwData = 0;
			if (u_PC_RespList[RESP_CURVE].aiIndexList[i] < DBINDEX_SUM)
				DB_GetData(u_PC_RespList[RESP_CURVE].aiIndexList[i], &dwData); // THJ 2016-05-11 Modify: "DB_GetDataByUserIndex" -> "DB_GetData"
		}
		memcpy(&CurveData.dbValue[i], &dwData, sizeof(DWORD));
	}
	memcpy((WORD*)u_PC_CurveBuffer + u_PC_wRespCurveSetNum * wCurveLength, (WORD*)&CurveData, (DWORD)wCurveLength);
	u_PC_wRespCurveSetNum++;
	//u_PC_dwCurveTimeAxis += u_PC_RespList[RESP_CURVE].wCycle; // Anders 2015-2-24, Mark.

	//if (u_PC_wRespCurveSetNum > u_PC_wMaxRespCurveSetNum)
	//  u_PC_wMaxRespCurveSetNum = u_PC_wRespCurveSetNum;   // monitor MaxNum.
	//if (u_PC_wRespCurveSetNum >= MAX_SETNUM_CURVE )
	//{
	//    u_PC_wRespCurveSetNum = 0;
	//    u_PC_wRespCurveBufferFull ++;
	//}

	if (u_PC_wRespCurveSetNum >= MAX_SENDNUM_CURVE)
	{

		u_PC_wRespCurveSetNum = 0; // u_PC_wRespCurveSetNum - MAX_SENDNUM_CURVE;

		wLength                                       = 2 * (sizeof(TMHEADER) + wCurveLength * MAX_SENDNUM_CURVE) + 2; //add Time Axis
		u_PC_TempBuffer.TMHeader.wSize                = wLength;
		u_PC_TempBuffer.TMHeader.wCounter             = 0;
		u_PC_TempBuffer.TMHeader.wType                = RESPONSE_CURVE_ID;
		u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_dwRespCurveRecCounter++;
		u_PC_TempBuffer.TMHeader.wNoElements          = MAX_SENDNUM_CURVE; //u_PC_RespList[RESP_CURVE].wNoElements;
		memcpy(u_PC_TempBuffer.awData, u_PC_CurveBuffer, (DWORD)wCurveLength * MAX_SENDNUM_CURVE);
		*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

		SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
		//if (u_PC_wRespCurveSetNum != 0)   // Anders 2016-6-16, mark.
		//    memcpy(u_PC_CurveBuffer, u_PC_CurveBuffer + MAX_SENDNUM_CURVE * wCurveLength, u_PC_wRespCurveSetNum * wCurveLength);
		//u_wSendStatus = NETBUSY;
	}
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
static void PC_ReqLinkState(int nSessionHandle, WORD* pdata, WORD wlength)
{
	WORD      wLen;
	LONG      lCounter; // Anders2017-6-8, Modify bug. WORD->LONG.
	WORD      wPrivilege;
	TMHEADER* pHeader = (TMHEADER*)pdata;

	if (pHeader->wNoElements != 1)
		return;
	//if(nSessionHandle == g_PC_nNetSession)  // Anders 2014-12-24, Mark.
	{
		memcpy((WORD*)&lCounter, (pdata + sizeof(TMHEADER)), sizeof(lCounter)); //memcpy(&PC_LinkState.lCounter, (pdata + sizeof(TMHEADER)), sizeof(long));
		//if(PC_LinkState.lCounter == 0) PC_LinkState.lCounter = -1;
		//else    PC_LinkState.lCounter = PC_LinkState.lCounter*2 + 100;
		ResetTimer(&PC_LinkState.lCounter, (lCounter == 0 ? 0 : lCounter * 2 + 100));
		PC_LinkState.wState                      = 1;
		wPrivilege                               = *(pdata + sizeof(TMHEADER) + sizeof(long));
		wLen                                     = (sizeof(TMHEADER) + 4) << 1;
		u_PC_TempBuffer.TMHeader.wSize           = wLen;
		u_PC_TempBuffer.TMHeader.wType           = RESPONSE_LINKSTATE_ID;
		u_PC_TempBuffer.TMHeader.wCounter        = 0;
		u_PC_TempBuffer.TMHeader.datatype.wCycle = 0;
		u_PC_TempBuffer.TMHeader.wNoElements     = 1;
		memcpy(u_PC_TempBuffer.awData, (pdata + sizeof(TMHEADER)), sizeof(long));
		u_PC_TempBuffer.awData[2] = wPrivilege;

		*((WORD*)&u_PC_TempBuffer + (wLen >> 1) - 1) = FOOTER_RES;

		SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLen);
	}
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
//static  void  RespPC_LinkState(int nSessionHandle)
//{
//
//}
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
WORD GetLinkState_PC()
{
	return PC_LinkState.wState;
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
void PostMsg_PC(MSGDEVICE Device, WORD wMsgID)
{
	PC_MsgState.wFlag = 1;
	//PC_MsgState.lCounter = ECHO_COUNTER;
	//ResetTimer(&PC_MsgState.lCounter, ECHO_COUNTER);
	if (PC_MsgState.wCount < MAX_WARNING_NO)
	{
		//PC_MsgState.awMsgType[PC_MsgState.wCount] = wType;
		PC_MsgState.awMsgID[PC_MsgState.wCount]  = wMsgID;
		PC_MsgState.Device[PC_MsgState.wCount++] = Device;
	}
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
static void PC_ReqMoldsetState(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER* pHeader = (TMHEADER*)pdata;

	if (pHeader->wNoElements != 1)
		return;
	memcpy(&PC_MoldsetState.lReqElements, (pdata + sizeofW(TMHEADER)), sizeof(long));
	if (PC_MoldsetState.lReqElements == PC_MoldsetState.lUpdateCount)
	{
		PC_MoldsetState.wFinished = 1;
		PC_RespMoldsetState(nSessionHandle);
		//StartSendOnceData(ONCEDATAID_MAXVELO);
	}
	else
	{
		PC_MoldsetState.wFinished = 0;
		PC_RespMoldsetState(nSessionHandle);
		PC_MoldsetState.lUpdateCount = 0; //Winston adding for moldset state bug
	}
	PC_MoldsetState.wReceiveState = 1;
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
static void PC_RespMoldsetState(int nSessionHandle)
{
	WORD wLength;

	memset(u_PC_TempBuffer.awData, 0, MAX_BUFFER_NO);
	wLength                                       = (sizeofW(TMHEADER) + 3) << 1;
	u_PC_TempBuffer.TMHeader.wSize                = wLength;
	u_PC_TempBuffer.TMHeader.wCounter             = 0;
	u_PC_TempBuffer.TMHeader.wType                = RESPONSE_MOLDSETSTATE_ID;
	u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_wRespMoldsetStateCounter++;
	u_PC_TempBuffer.TMHeader.wNoElements          = 1;
	//*(long *)u_PC_TempBuffer.awData = PC_MoldsetState.lUpdateCount;           // Anders 20100310
	memcpy(&u_PC_TempBuffer.awData, &PC_MoldsetState.lUpdateCount, sizeof(long));
	*((WORD*)&u_PC_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

	SendNetData(nSessionHandle, (WORD*)&u_PC_TempBuffer, wLength);
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
static void ReqMoldsetUpdate(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER*    pHeader = (TMHEADER*)pdata;
	WORD         i, wNoElements; //, wLength;
	TM_PARADATA *pReadParaData, *pWriteParaData;
	DBVALUE      dbValue = { 0 };
	DWORD        dwID;

	//g_wWriteRefValueHMICMD = COMM_MOLDSETUPDATE;
	memset(u_PC_TempBuffer.awData, 0, sizeof(u_PC_TempBuffer.awData));
	wNoElements = pHeader->wNoElements;

	pReadParaData  = (TM_PARADATA*)(pdata + sizeofW(TMHEADER));
	pWriteParaData = (TM_PARADATA*)u_PC_TempBuffer.awData;
	for (i = 0; i < wNoElements; i++)
	{
		memcpy(&dwID, &pReadParaData->wIDl, sizeof(DWORD));
		memcpy(&dbValue, &pReadParaData->wData1, sizeof(DWORD));
		memcpy(&pWriteParaData->wIDl, &pReadParaData->wIDl, sizeof(DWORD));
		//pWriteParaData->dwID = pReadParaData->dwID;
		pWriteParaData->stattype.bit.wState    = 0;
		pWriteParaData->stattype.bit.wDataType = pReadParaData->stattype.bit.wDataType;
		switch (pReadParaData->stattype.bit.wDataType)
		{
			case _WORD:
				if (DB_ERR_OUTRANGE == SetDBValue(dwID, dbValue))
				{
					dbValue = GetDBValue(dwID);
					memcpy(&pWriteParaData->wData1, &dbValue, sizeof(DWORD));
				}
				else {
					pWriteParaData->stattype.bit.wState = 1;
					memcpy(&pWriteParaData->wData1, &pReadParaData->wData1, sizeof(DWORD));
				}
				break;
			case _DWORD:
				if (DB_ERR_OUTRANGE == SetDBValue(dwID, dbValue))
				{
					dbValue = GetDBValue(dwID);
					memcpy(&pWriteParaData->wData1, &dbValue, sizeof(DWORD));
				}
				else {
					pWriteParaData->stattype.bit.wState = 1;
					memcpy(&pWriteParaData->wData1, &pReadParaData->wData1, sizeof(DWORD));
				}
				break;
			default:
				break;
		}
		pReadParaData++;
		pWriteParaData++;
	}

	if (PC_MoldsetState.wFinished == 0)
	{
		if (g_PC_wLastMoldsetCNT != pHeader->wCounter)
			PC_MoldsetState.lUpdateCount += wNoElements;
	}
	g_PC_wLastMoldsetCNT = pHeader->wCounter;
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
//static  void  RespMoldsetUpdate(int nSessionHandle)
//{
//
//}

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
//WORD  Get_MoldsetState()
//{
//  return  PC_MoldsetState.wFinished;
//}
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
//void  RespRefValueAuto_PC(int nSessionHandle,DWORD *pDBID, WORD wNum)
//{
//  TM_PARADATA *pParaData = (TM_PARADATA *)u_PC_TempBuffer.awData;
//  int     i;
//  WORD    wLength;
//  DWORD   dwData;
//
//  if (pDBID == NULL || wNum == 0) return;
//
//  memset(u_PC_TempBuffer.awData, 0, MAX_BUFFER_NO);
//  wLength = (sizeofW(TMHEADER) + sizeofW(TM_PARADATA)*wNum +1)<<1;
//  u_PC_TempBuffer.TMHeader.wSize = wLength;
//  u_PC_TempBuffer.TMHeader.wCounter = 0;
//  u_PC_TempBuffer.TMHeader.wType = RESPONSE_READ_REFERENCE_ID;
//  u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_wRespReadRefCounter++;
//  u_PC_TempBuffer.TMHeader.wNoElements = wNum;
//
//  *((WORD *)&u_PC_TempBuffer + (wLength>>1) - 1) = FOOTER_RES;
//
//  for (i=0; i<wNum; i++)
//  {
//      //pParaData->dwID = *(pDBID+i);
//      memcpy(&pParaData->wIDl, pDBID+i, sizeof(DWORD));
//      pParaData->stattype.bit.wState = 1;
//      pParaData->stattype.bit.wDataType = _WORD;
//      dwData  = GetDBValue(*(pDBID+i));
//      memcpy(&pParaData->wData1, &dwData, sizeof(DWORD));
//      pParaData++;
//  }
//  SendNetData(nSessionHandle,(WORD *)&u_PC_TempBuffer, wLength);
//}

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
//void  RespRefValueAutoByDBIndex(int nSessionHandle,WORD *pDBIndex, WORD wNum) //Anders 20100421
//{
//  TM_PARADATA *pParaData = (TM_PARADATA *)u_PC_TempBuffer.awData;
//  int     i;
//  WORD    wLength;
//  DWORD   dwID, dwData;
//
//  if (pDBIndex == NULL || wNum == 0) return;
//
//  memset(u_PC_TempBuffer.awData, 0, MAX_BUFFER_NO);
//  wLength = (sizeofW(TMHEADER) + sizeofW(TM_PARADATA)*wNum +1)<<1;
//  u_PC_TempBuffer.TMHeader.wSize = wLength;
//  u_PC_TempBuffer.TMHeader.wCounter = 0;
//  u_PC_TempBuffer.TMHeader.wType =RESPONSE_READ_REFERENCE_ID; // RESPONSE_ACTUAL_ID;  //RESPONSE_READ_REFERENCE_ID;
//  u_PC_TempBuffer.TMHeader.datatype.wCounterRet = u_PC_wRespReadRefCounter++;
//  u_PC_TempBuffer.TMHeader.wNoElements = wNum;
//
//  *((WORD *)&u_PC_TempBuffer + (wLength>>1) - 1) = FOOTER_RES;
//
//  for (i=0; i<wNum; i++)
//  {
//      dwID = IndexToDataID(*(pDBIndex+i));
//      memcpy(&pParaData->wIDl, &dwID, sizeof(DWORD));
//      pParaData->stattype.bit.wState = 1;
//      pParaData->stattype.bit.wDataType = DataAttr[*(pDBIndex+i)].byDataType;
//      dwData = GetDBValueByIndex(*(pDBIndex+i));
//      memcpy(&pParaData->wData1, &dwData, sizeof(DWORD));
//      pParaData++;
//  }
//  SendNetData(nSessionHandle,(WORD *)&u_PC_TempBuffer, wLength);
//}

static void SetCmdBurn(int nSessionHandle, DWORD dwCommand, WORD wDataType)
{
	//WORD              i, wBurnFlag = 1;
	//TAxisController * pAxisController;
	//for (i = 0; i < AXIS_MAX; i++)
	//{
	//	pAxisController = Get_AxisControllerHandle(i);
	//	if (pAxisController->InnerCtlCmd.Info.wCMD > CMD_HOMINGCTL) // Anders 2016-7-14, modify "!= CMD_STOPCTL"
	//		wBurnFlag = 0;
	//}

	//if (wBurnFlag)
	//{
	dwCommand |= (DWORD)1 << 31;
	RespCommand(nSessionHandle, dwCommand);

	DINT;
	DELAY_US(20);
	GOBOOT();
	//}
	//RespCommand(nSessionHandle, dwCommand);
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
static void SetCmdResetAlarm(int nSessionHandle, DWORD dwCommand, WORD wDataType)
{
	unsigned i;
	//if(nSessionHandle == g_PC_nNetSession)    // Anders 2014-12-24, Mark.
	{
		dwCommand |= (DWORD)1 << 31;

		for (i = DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR1; i <= DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR16; ++i) {
			DB_SetDWORD(i, 0);
		}
		Command_ManualKey(); // Anders 2017-1-4, add.
	}
	RespCommand(nSessionHandle, dwCommand);
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
static void EchoMsgValue(int nSessionHandle, WORD* pdata, WORD wlength)
{
	TMHEADER* pHeader = (TMHEADER*)pdata;

	if (pHeader->datatype.wCounterRet == u_PC_wRespAlarmCounter - 1)
	{
		PC_MsgState.bGetEchoState = TRUE;
		PC_MsgState.wCount        = 0;
	}
}
