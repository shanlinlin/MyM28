/*==========================================================================+
|  File     : tasknet_HMI.h                                                 |
|  Function : Network commnunication                                        |
|  Task     : Network commnunication header                                 |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : zholy                                                         |
|  Version  : V1.00                                                         |
|  Creation : 2007/07/23                                                    |
|  Revision : 1.0                                                           |
+==========================================================================*/

#ifndef D__TASKNET_HMI
#define D__TASKNET_HMI

#include "database.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
//Yang 2008/3/8 Adding for TMJ5-Communication protocol
#define TM_PARADATA_WSIZE 5
#define MAX_REQUEST_NO 50
//#define           MAX_BUFFER_NO       (MAX_REQUEST_NO * sizeof(TM_PARADATA) + 1)
#define MAX_BUFFER_NO (MAX_REQUEST_NO * TM_PARADATA_WSIZE + 48) // 20100310 . 64*4=256W
#define MAX_CURVEDATA 30

#define CURVEID_SETPRESS_CLMP 2001
#define CURVEID_ACTPRESS_CLMP 2002
#define CURVEID_SETVELO_CLMP 2003
#define CURVEID_ACTVELO_CLMP 2004

#define BASEADDR_TRISMITEDAUTOVALUE 1
#define BASEADDR_TRISMITEDCURVEVALUE 51
#define BASEADDR_TRISMITEDONCEVALUE 101

//====================
// Curve Data Type
//===================
#define CURVEDATATYPE_STOP 0
#define CURVEDATATYPE_TIME 1
#define CURVEDATATYPE_DISTANCE 2

#define MAX_CURVE_ITEMS 4

/*---------------------------------------------------------------------------+
|           Type Definition                                                  |
+---------------------------------------------------------------------------*/
//Yang 2008/3/8 Adding for TMJ5-Communication protocol
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

typedef struct tagTM_PARADATA
{
	DWORD dwID;
	union STATTYPE {
		struct BIT
		{
			WORD wDataType : 8;
			WORD wState : 8; // wMode
		} bit;
		WORD all;
	} stattype;
	union DATA {
		WORD  wData;
		DWORD dwData;
	} data;
} TM_PARADATA;

typedef struct tagCOMMMAP
{
	WORD wCmd;
	void (*pFunc)(int nSessionHandle, WORD * pdata, WORD wlength);
} COMMMAP;

typedef struct tagMOTORCOMMAP
{
	DWORD dwCmd;
	void (*pFunc)();
} MOTORCOMMAP; // zholy080821

typedef struct tagTEMPBUFFER
{
	TMHEADER TMHeader;
	WORD     awData[MAX_BUFFER_NO];
} TEMPBUFFER;

typedef struct tagLINKSTATE
{
	Ttimer lCounter;
	WORD   wState;
} LINKSTATE;

typedef struct tagMSGSTATE
{
	WORD wMsgType;
	WORD wMsgID;
	WORD wType;
	WORD wFlag;
} MSGSTATE;

typedef struct tagRESPMSG
{
	union MSGTYPE {
		struct MSGBIT
		{
			WORD wMsgType : 8;
			WORD wReserved : 8; // wMode
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
	WORD wFlag;
	WORD wCycle;
	WORD wNoElements;
	//(20150115 THJ(Modification:)
	//(20150115 THJ)DBIndex_T   awList[MAX_REQUEST_NO];//WORD       awList[MAX_REQUEST_NO];
	//(20150115 THJ)//DWORD     adwID[MAX_REQUEST_NO];  // Test
	DWORD     adwIDList[MAX_REQUEST_NO];
	DBIndex_T atIndexList[MAX_REQUEST_NO];
	//)
	Ttimer lCounter;
	void (*pFunc)(int nSessionHandle);
} RESPLIST;
//typedef   struct tagSENDONCELIST
//{
//  WORD        wFlag;
//    WORD        wCycle;
//    long        lCounter;
//  void        (*pFunc)(int nSessionHandle);
//} SENDONCELIST;

//==============
// Curve Define
//==============
//typedef struct tagTM_CURVEHEADER
//{
//    TMHEADER    tmHeader;
//    WORD        wCurveMotionID;
//    WORD        wCollectType;
//    WORD        wMoldCNT;
//    WORD        wInterval;
//    long        lStartTimePosn;
//}TM_CURVEHEADER;

//typedef union tagCURVEREQINFOR
//{
//  WORD    all;
//  struct{
//      WORD        bActPress       :1;
//      WORD        bProfileVelo    :1;
//      WORD        bProfilePosn    :1;
//      WORD        bActVelo        :1;
//      WORD        bActPosn        :1;
//  }bit;
//}CURVEREQINFOR;

//typedef   struct tagTM_MOTIONCURVE
//{
//  union
//  {
//      WORD        all;
//      struct{
//          WORD    bCollectFlag    :1;
//          WORD    bEndCollectFlag :1;
//          WORD    bSendFlag       :1;
//      }bit;
//  }State;
//  WORD    wMoldCNT;
//  WORD    wCount;
//  WORD    wCurveMotionID;     // Motion Start Must Init.
//  WORD    wCollectType;
//    WORD    wInterval;
//  WORD    wDirection;
//    TFLOAT  fStartTimePosn;       // Motion Start Must Init.
//  DWORD   dwCollectTime;      // Motion Start Must Init.
//  DWORD   dwExecTime;         // Test
//    CURVEREQINFOR   ReqInfo;
//  int     anActPress[MAX_CURVEDATA];
//  int     anActVelo[MAX_CURVEDATA];
//    //int anProfileVelo[MAX_CURVEDATA];
//    //long    alActPosn[MAX_CURVEDATA];
//    //long    alProfilePosn[MAX_CURVEDATA];
//}MOTIONCURVE;

//typedef struct tagCURVERESPLIST
//{
//  WORD        wFlag;
//  WORD        wDirection;
//
//  WORD        wCurveMotionID;
//
//    WORD        wCollectType;
//    WORD        wInterval;
//
//  CURVEREQINFOR   ReqInfo;
//  MOTIONCURVE     *pMotionCurve;  // Anders 20101008.
//}CURVERESPLIST;

//typedef struct tagTM_CURVEDETAIL
//{
//  WORD        wInterval;
//    WORD      wDataItems;
//    WORD      awDataIndex[MAX_CURVE_ITEMS];
//} TM_CURVEDETAIL;

//typedef struct tagTM_CURVEREQ
//{
//    TMHEADER    tmHeader;
//    WORD        wCurveMotionID;
//      WORD        wInterval;
//    WORD      wDataItems;
//    WORD        wReserved;
//    DWORD     adwDataID[MAX_CURVE_ITEMS];
//}TM_CURVEREQ;
//
//typedef struct tagTM_CURVERESP
//{
//    WORD        wCurveMotionID;
//    WORD      wCurveIndex;
//      WORD        wInterval;
//    WORD      wDataItems;
//    WORD      wDataCount;
//    WORD        wReserved;
//    DBVALUE       SendCurvePoints[MAX_CURVEDATA*MAX_CURVE_ITEMS];
//}TM_CURVERESP;

//typedef   struct tagTM_MOTIONCURVE
//{
//  WORD    wCurveMotionID;
//  WORD    wCurveIndex;
//    WORD    wInterval;
//  WORD    wDataItems;
//  WORD    wCollectCount;
//  WORD    wSendCount;
//  WORD    awDataIndex[MAX_CURVE_ITEMS]; // ID-->Index
//  DBVALUE     CurvePoints[MAX_CURVEDATA*MAX_CURVE_ITEMS];
//  DBVALUE     SendCurvePoints[MAX_CURVEDATA*MAX_CURVE_ITEMS];
//}MOTIONCURVE;

//typedef struct tagCURVERESPLIST
//{
//  WORD    wFlag;
//  union
//  {
//      WORD        all;
//      struct{
//          WORD    bCollectFlag    :1;
//          WORD    bEndCollectFlag :1;
//          WORD    bSendFlag       :1;
//      }bit;
//  } State;
//  long    lDownTimer;
//  DWORD   dwOverwrite;    // Test
//
//  MOTIONCURVE     MotionCurve;
//}CURVERESPLIST;
//
////===============
//
//enum    CURVETYPE
//{
//    CURVETYPE_ACTPRESS      = 1,
//    CURVETYPE_PROFILEVELO   = 2,
//    CURVETYPE_PROFILEPOSN   = 3,
//    CURVETYPE_ACTVELO       = 4,
//    CURVETYPE_ACTPOSN       = 5
//};
enum COMMANDID
{
	COMM_NULL,
	COMM_ACTUAL,
	COMM_READPARAMETER,
	COMM_WRITEPARAMETER,
	COMM_MESSAGE,
	COMM_CURVE,
	COMM_MOTIONCOMMAND,
	COMM_LINKSTATE,
	COMM_MOLDSETSTATE,
	COMM_MOLDSETUPDATE,
	COMM_END,
	COMM_MAX = COMM_END
};

enum RESPONSEID
{
	RESP_ACTUAL,
	RESP_READPARAMETER,
	RESP_MAX
};
enum ENUMMSGTYPE
{
	MSGTYPE_CLEAR,
	MSGTYPE_HINT,
	MSGTYPE_ALARM
};
//enum ONCEDATALIST {
//    ONCEDATAID_MAXVELO,
//    ONCEDATAID_POSTMSG,
//    ONCEDATAID_VERSION,
//    ONCEDATAID_INJECTEND,
//  ONCEDATAID_CURVEVALUEINJECT,
//  ONCEDATAID_CURVEVALUESUCKBACK,
//  ONCEDATAID_CURVEVALUEHOLDPRESS,
//  ONCEDATAID_CURVEVALUECHARGE
//};
//enum CURVEMOTIONIDLIST{
//  CURVEMOTIONID_RESVD         = 0,
//    CURVEMOTIONID_CLOSEMOLD       = 1,
//  CURVEMOTIONID_OPENMOLD      = 2,
//
//  CURVEMOTIONID_MAX           = CURVEMOTIONID_OPENMOLD
//};

/*==========================================================================+
|           External                                                        |
+==========================================================================*/
WORD CreateTaskNet_HMI(void);
void DestoryTaskNet_HMI(void);
void RunTaskNet_HMI(void);
//WORD  GetLinkState(void);
WORD GetMoldsetState(void);
//void  PostMsg(WORD wMsgID,WORD wMsgType);
//void  RespMsgValue(int nSessionHandle);

void RespRefValueAuto(int nSessionHandle, WORD * pdata, WORD wNum);             // zholy080822
void RespRefValueAutoByDBIndex(int nSessionHandle, WORD * pDBIndex, WORD wNum); //Anders 20100421
void ParseNetData_FromHMI(int nSessionHandle, WORD * pdata, WORD wLength);
//void    StartSendOnceData(WORD  wIndex);
//void    ResetSendOnceDataFlag(void);

//void  Start_Motion(WORD wMCID);
//void  End_Motion(WORD wMCID);

//extern  WORD      g_wCollectCurveFlag;
extern int g_nHMISession;
//extern  WORD        g_wWriteRefValueHMICMD;

#endif
