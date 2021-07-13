/*==========================================================================+
|  Function : Network commnunication                                        |
|  Task     : Network commnunication header                                 |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   :                                                               |
|  Version  :                                                               |
|  Creation :                                                               |
|  Revision :                                                               |
+==========================================================================*/
#ifndef D__TASKNET_PC
#define D__TASKNET_PC

#include "database.h"
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define DEVICEHANDLE_PC 1

#define MAX_REQUEST_NO 50
#define MAX_BUFFER_NO (MAX_REQUEST_NO * sizeof(TM_PARADATA) + 1)
#define MAX_SETNUM_CURVE 17 //Anders 2017-3-31,must <18. // 28  //15
#define MAX_CURVE_ELEMENT 8
#define MAX_SEND_CURVEDATA (SIZE_LM(2 * (MAX_CURVE_ELEMENT + 1) * MAX_SETNUM_CURVE)) // MAX_SEND_CURVEDATA == 320, Anders 20110316.
#define MIN_CLCYETIME 200
#define MAX_SENDNUM_CURVE MAX_SETNUM_CURVE //10
#define MAX_HMIREQUEST_NO 18
#define ECHO_COUNTER 1000
#define MAX_WARNING_NO 6

#define CTRLMCCMD_ENABLE 0x00000001L
#define CTRLMCCMD_STOP 0x00000002L
#define CTRLMCCMD_RESETALARM 0x00000003L
#define CTRLMCCMD_AUTOALIGN 0x00000004L
#define CTRLMCCMD_SAVETOROM 0x00000005L
#define CTRLMCCMD_LOADFROMROM 0x00000006L
#define CTRLMCCMD_RESERDB 0x00000007L
#define CTRLMCCMD_TESTMOTOR 0x00000008L
#define CTRLMCCMD_AUTOPID 0x00000009L
#define CTRLMCCMD_DRAWBODE 0x0000000AL
#define CTRLMCCMD_PHASEACOFFSET 0x0000000BL
#define CTRLMCCMD_BUSVOLTAGEGAIN 0x0000000CL
#define CTRLMCCMD_ADCOMMANDEGAIN 0x0000000DL
#define CTRLMCCMD_ADLIMITGAIN 0x0000000EL
#define CTRLMCCMD_ADPRESSUREGAIN 0x0000000FL
#define CTRLMCCMD_HWTESTENABLE 0x00000010L
#define CTRLMCCMD_HWTESTSTOP 0x00000011L
#define CTRLMCCMD_BURN 0x00000012L

#define CTRLMCCMD_ADCALIMIN 0x00010001L
#define CTRLMCCMD_ADCALIMAX 0x00010002L
#define CTRLMCCMD_ADCALIRESET 0x00010003L
#define CTRLMCCMD_SAVETOEEPROM 0x00010004L
#define CTRLMCCMD_RESETEEPROM 0x00010005L

#define CTRLMCCMD_SUCCESS_ENABLE 0x80000001L
#define CTRLMCCMD_SUCCESS_STOP 0x80000002L
#define CTRLMCCMD_SUCCESS_RESERALARM 0x80000003L
#define CTRLMCCMD_SUCCESS_AUTOALIGN 0x80000004L
#define CTRLMCCMD_SUCCESS_SAVETOROM 0x80000005L
#define CTRLMCCMD_SUCCESS_LOADFROMROM 0x80000006L
#define CTRLMCCMD_SUCCESS_RESERDB 0x80000007L
#define CTRLMCCMD_SUCCESS_TESTMOTOR 0x80000008L
#define CTRLMCCMD_SUCCESS_AUTOPID 0x80000009L
#define CTRLMCCMD_SUCCESS_DRAWBODE 0x8000000AL
#define CTRLMCCMD_SUCCESS_PHASEACOFFSET 0x8000000BL
#define CTRLMCCMD_SUCCESS_BUSVOLTAGEGAIN 0x8000000CL
#define CTRLMCCMD_SUCCESS_ADCOMMANDEGAIN 0x8000000DL
#define CTRLMCCMD_SUCCESS_ADLIMITGAIN 0x8000000EL
#define CTRLMCCMD_SUCCESS_ADPRESSUREGAIN 0x8000000FL
#define CTRLMCCMD_SUCCESS_HWTESTENABLE 0x80000010L
#define CTRLMCCMD_SUCCESS_HWTESTSTOP 0x80000011L

#define ALARM_NULL 0
#define ALARM_MESSAGE 1
#define ALARM_ERROR 2

//burn
#define START_BOOT 0
#define START_NORMAL 1
#define EEPROM_PAGEBURN 248

/*---------------------------------------------------------------------------+
|           Type Definition                                                  |
+---------------------------------------------------------------------------*/
typedef struct tagTMHEADER
{
	WORD wHeader;
	WORD wSize;
	WORD wType;
	WORD wCounter;
	union DATATYPE {
		WORD wCounterRet;
		WORD wCycle;
		WORD wSequence;
		WORD wInternalState;
	} datatype;
	WORD wNoElements;
} TMHEADER;

//typedef struct tagTM_PARADATA
//{
//  WORD        wID;
//  union       STATTYPE
//  {
//      struct  BIT
//      {
//          WORD    wDataType   : 8;
//          WORD    wState      : 8; // wMode
//      } bit;
//      WORD        all;
//  } stattype;
//  union       DATA
//  {
//      long                lData;
//      char                acData[2];
//      WORD                wData;
//      DWORD               dwData;
//      char                cData;
//      int                 nData;
//      float               fData;
//      double              dData;
//      char                *pcData;
//  } data;
//} TM_PARADATA;
// WORD->DWORD, Anders 2013-12-27.
typedef struct tagTM_PARADATA
{
	WORD wIDl;
	WORD wIDh;
	union STATTYPE {
		struct BIT
		{
			WORD wDataType : 8;
			WORD wState : 8; // wMode
		} bit;
		WORD all;
	} stattype;
	WORD wData1;
	WORD wData2;
} TM_PARADATA;

typedef struct tagCURVEDATA
{
	DWORD   dwCurveTimeAxis;
	DBVALUE dbValue[MAX_CURVE_ELEMENT];
} CURVEDATA;

typedef struct tagCOMMMAP
{
	WORD wCmd;
	void (*pFunc)(int nSessionHandle, WORD * pdata, WORD wlength);
} COMMMAP;

typedef struct tagCTRLCMDMAP
{
	DWORD dwCmd;
	void (*pFunc)(int nSessionHandle, DWORD dwCommand, WORD wDataType); //(WORD *pdata, WORD wlength);
} CTRLCMDMAP;

typedef struct tagTEMPBUFFER
{
	TMHEADER TMHeader;
	WORD     awData[MAX_BUFFER_NO + 63]; // Anders 20110316. sizeof(TEMPBUFFER)
} TEMPBUFFER;

typedef struct tagSTOREREQWRITE
{
	TMHEADER TMHeader;
	WORD     awData[5];
} STOREREQWRITE;

typedef struct tagLINKSTATE
{
	Ttimer lCounter;
	WORD   wState;
} LINKSTATE;

typedef struct tagSTOREHMILINK
{
	WORD wActiveFlag;
	WORD wSessionHandle;
	long lCounter;
} STOREHMILINK;

//typedef   struct tagMSGSTATE
//{
//    WORD        wMsgType;
//    WORD      wMsgID;
//  WORD        wFlag;
//} MSGSTATE;
typedef union tagMSGDEVICE {
	WORD wAll;
	struct
	{
		WORD bDeviceID : 8;
		WORD bDeviceNum : 6;
		WORD bAlarmType : 2;
	} bit;
} MSGDEVICE;
typedef struct tagMSGSTATE
{
	WORD wFlag;
	WORD wCount;
	//WORD          awMsgType[MAX_WARNING_NO];
	MSGDEVICE Device[MAX_WARNING_NO]; // Anders 2013-3-21.
	WORD      awMsgID[MAX_WARNING_NO];
	BOOL      bGetEchoState;
	Ttimer    lCounter;
} MSGSTATE;

typedef struct tagRESPMSG
{
	union MSGTYPE {
		struct MSGBIT
		{
			WORD bDeviceID : 8;
			WORD bDeviceNum : 6;
			WORD bAlarmType : 2; // wMode
		} msgbit;
		WORD all;
	} msgtype;
	WORD wMsgID;
} RESPMSG;

typedef struct tagMOLDSETSTATE
{
	long lReqElements;
	long lUpdateCount;
	WORD wFinished;
	WORD wReceiveState;
} MOLDSETSTATE;

typedef struct tagRESPLIST
{
	WORD      wFlag;
	WORD      wCycle;
	WORD      wNoElements;
	DWORD     adwIDList[MAX_REQUEST_NO];
	DBIndex_T aiIndexList[MAX_REQUEST_NO];
	Ttimer    lCounter;
	void (*pFunc)(int nSessionHandle);
} RESPLIST;

typedef struct tagBODEDATA
{
	long    lIndex;
	DBVALUE dbTarget;
	DBVALUE dbActual;
} BODEDATA;

typedef struct tagBODEQUEUE
{
	BODEDATA *    pQueue;
	WORD          wInput;
	WORD          wOutput;
	WORD          wMaxSize;
	volatile WORD wCount;
} BODEQUEUE;

enum COMMANDID
{
	COMM_NULL           = 0,
	COMM_ACTUAL         = 1,
	COMM_READPARAMETER  = 2,
	COMM_WRITEPARAMETER = 3,
	COMM_MESSAGE        = 4,
	COMM_CURVE          = 5,
	COMM_MOTIONCOMMAND  = 6,
	COMM_LINKSTATE      = 7,
	COMM_MOLDSETSTATE   = 8,
	COMM_MOLDSETUPDATE  = 9,
	ECHO_MESSAGE_ID     = 2004,
	COMM_END
};

enum RESPONSEID
{
	RESP_ACTUAL,
	RESP_READPARAMETER,
	RESP_CURVE,
	RESP_MAX
};

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/

/*===========================================================================+
|           Class declaration - TaskCmd                                      |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD CreateTaskNet_PC();
void DestoryTaskNet_PC();
void RunTaskNet_PC();
/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/
WORD GetLinkState_PC();
void PostMsg_PC(MSGDEVICE Device, WORD wMsgID);

//void  RespRefValueAuto_PC(int nSessionHandle, DWORD *pDBID, WORD wNum);

#endif
