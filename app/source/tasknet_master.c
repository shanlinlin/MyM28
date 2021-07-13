/*==========================================================================+
|  Function : Network commnunication                                        |
|  Task     : Network commnunication                                        |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders.Zhang                                                  |
|  Version  : V3.00                                                         |
|  Creation : 2009/07/20                                                    |
|  Revision : 2010/07/05                                                    |
+==========================================================================*/
#include "tasknet_master.h"
#include "CommComp_RemoteIO.h"
#include "CommComp_TemperCard.h"
#include "CommComp_turntable.h"
#include "CommComp_Driver.h"
#include "CommComp_HTRobot.h"
#include "Device.h"
#include "Error_App.h"
#include "common.h"
#include "dbfunc.h"
#include "ethernet.h"
#include "taskEtherCATMaster_app.h"
#include "task_ethercatmaster.h"
#include "taskio.h"
#include "tasknet.h"
#include "tasknet_HMI.h"
#include "comp_fifo.h"
#include "comp_fifo2.h"
#include "stdlib.h"
/*===========================================================================+
|           Function Prototype                                               |
+===========================================================================*/
void GetMasterPowerOn(int nSessionHandle, WORD* pData, WORD wLength);       //AA
void GetMasterSoftBoot(int nSessionHandle, WORD* pData, WORD wLength);      //AB, Anders 2010-11-5.
void GetSlowActual(int nSessionHandle, WORD* pData, WORD wLength);          //AC
void GetMachineParameters1(int nSessionHandle, WORD* pData, WORD wLength);  //LA
void GetMachineParameters2(int nSessionHandle, WORD* pData, WORD wLength);  //LB
void GetMoldSet1(int nSessionHandle, WORD* pData, WORD wLength);            //LC
void GetMoldSet2(int nSessionHandle, WORD* pData, WORD wLength);            //LD
void GetMachineParameters_B(int nSessionHandle, WORD* pData, WORD wLength); //LE
void GetMoldSet_B1(int nSessionHandle, WORD* pData, WORD wLength);          //LF
void GetMoldSet_B2(int nSessionHandle, WORD* pData, WORD wLength);          //LG

void GetConfiguration(int nSessionHandle, WORD* pData, WORD wLength);      //MA
void GetOperateState(int nSessionHandle, WORD* pData, WORD wLength);       //MB
void GetRetransmitData(int nSessionHandle, WORD* pData, WORD wLength);     //MC
void GetRetransmitDataLess(int nSessionHandle, WORD* pData, WORD wLength); //MD

void GetEcho_Boot(int nSessionHandle, WORD* pData, WORD wLength);               //90 echo.
void GetEcho_AD_DA(int nSessionHandle, WORD* pData, WORD wLength);              //91 echo.
void GetEcho_RetransmitData(int nSessionHandle, WORD* pData, WORD wLength);     //93 echo.
void GetEcho_RetransmitDataLess(int nSessionHandle, WORD* pData, WORD wLength); //94 echo.
void GetEcho_SlowActual(int nSessionHandle, WORD* pData, WORD wLength);         //95 echo.

void SendTestData(int nSessionHandle, WORD wTest, WORD wLen); //Send LinkData for 54 MAC Addr.

void SendBootData(int nSessionHandle);                                 //Boot Data
void SendCurveData(int nSessionHandle, WORD wMotionID);                //Curve Data
void SendRespCMDResult(int nSessionHandle, WORD* pData, WORD wLength); //CMD Result

void ParseOperateMotion(TOperateMode OperateMode, WORD wMotionID, WORD wCmd);
void Command_ManualKey(void);

//static    void    StartAutoTx(int nSessionHandle,WORD wTxID);
static void StopAutoTx(int nSessionHandle, WORD wTxID);

static __inline void ResetBoot(void);
//static    __inline void   ResetStopCheck(void);

static __inline void StartTxADDA_OPState(void);
static __inline void StopTxADDA_OPState(void);

//static    inline void StartTxCurveData();
//static    inline void StopTxCurveData();

static int ScanOperateModeByBit(WORD wOperateMode);

//void  CheckEchoInvalid_ActData(void);
//void    CheckEchoInit_RetransmitData(void);
//BOOL  GetToken_ActData(WORD wOwner);
//void  GivebackToken_ActData(void);

void Send91_Fast(int nSessionHandle); //void SendAD_DA(int nSessionHandle, WORD  wMotionID);                         //Actual Data
void Send91_Slow(int nSessionHandle);

static void NetCycleResp_Run(NetCycleResp_S* psResp);
static void COMMSTEP_IDLE(void);
static void COMMSTEP_DELAY(void);
static void COMMSTEP_WORKING(void);
static BOOL CheckTemperCard_CommReady(void);

void ClearDBErr();
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define NETHEADERLEN sizeofW(IMETMHEADER)
#define NETNODATASIZE 8
#define NOCRCSIZE 3
#define BOOTDATALEN 64

#define ADDADATALEN (32 * 14) //(20141113 THJ(Modification: 1+31))// zholy20100209
#define ADDADATA_TOTALLEN ((ADDADATALEN + NETNODATASIZE) * sizeof(WORD))

#define TRANSMITDATALEN 256
#define TRANSMITDATA_TOTALLEN ((TRANSMITDATALEN + NETNODATASIZE) * sizeof(WORD))
#define TRANSMITDATABYTES_LEN TRANSMITDATALEN* sizeofB(WORD) // 40   //8+5+11+16=28H

#define RETRANSMITDATALESS_LEN 80
#define RETRANSMITDATALESS_TOTALLEN ((RETRANSMITDATALESS_LEN + NETNODATASIZE) * sizeof(WORD))
#define RETRANSMITDATALESSBYTES_LEN RETRANSMITDATALESS_LEN* sizeofB(WORD)

#define RESETBOOTTIME 1000 //(20141114 THJ(Modification: 3000)// ms
#define STOPCHECKTIME 10   // ms

#define MACHINE1OFFSET 0x0000
#define MACHINE2OFFSET 0x0200
#define MOLDSET1OFFSET 0x0400
#define MOLDSET2OFFSET 0x0680
#define CONFIGURATIONOFFSET 0x0AC1
#define OPERATESTATEOFFSET 0x0D91

#define ALLMOTIONRESPLIST_LEN (sizeof(u_AutoRespList) / sizeof(ALLMOTIONRESPLIST))
#define AUTORESPNOTCURRCYCLETIME 1000
#define AUTORESPCURRCYCLETIME 2 //100   // Test Anders 2014-6-24.

#define DEF_INVALIDADDATIME 10
#define DEF_INVALIDRETRANSMITTIME 20

#define DEF_INITRETRANSMITTIME 1000

#define AXISACTVALUE_LEN (sizeof(AXIS_ACTVALUE))

#define MOTION_RECORD_EJECTORFWD 0
#define MOTION_RECORD_EJECTORBWD 1
#define MOTION_RECORD_INJECT 2
#define MOTION_RECORD_CHARGE 3
#define MOTION_RECORD_PRESUCKBACK 4
#define MOTION_RECORD_REARSUCKBACK 5
#define MOTION_RECORD_NOZZLEFWD 6
#define MOTION_RECORD_NOZZLEBWD 7
#define MAX_TIMERECORD_MOTION 8

#define MOTIONOPTION_NONE 0
#define MOTIONOPTION_WHOLE 1
#define MOTIONOPTION_PRESSCTL 2
//-------------------------------------------------------------------------------------------------

//CMD Dsp54->Dsp28
#define CMD_AA 0x4141 //  PowerOn Cold.
#define CMD_AB 0x4142 //  Softboot. Anders 2010-11-05.
#define CMD_AC 0x4143 //  Read slow actual data

#define CMD_MA 0x4D41 //  Configuration.              140H(WORD).
#define CMD_MB 0x4D42 //  OperateState.               28H->40H(WORD).
#define CMD_MC 0x4D43 //  HMI->DSP54->DSP28
#define CMD_MD 0x4D44 //  HMI->DSP54->DSP28

#define CMD_LA 0x4C41 //  PreviousMachineParameters.  200H(WORD). DSP28DB(0000H~01FFH).
#define CMD_LB 0x4C42 //  LastMachineParameters.      200H(WORD). DSP28DB(0200H~03FFH).
#define CMD_LC 0x4C43 //  PreviousMoldSetData.        280H(WORD). DSP28DB(0400H~067FH).
#define CMD_LD 0x4C44 //  LastMoldSetData.            280H(WORD). DSP28DB(0680H~08FFH).
#define CMD_LE 0x4C45 //  MachineParameters_B.        200H(WORD).
#define CMD_LF 0x4C46 //  PreviousMoldSetData_B.      280H(WORD).
#define CMD_LG 0x4C47 //  LastMoldSetData_B.          280H(WORD).

//CMD Dsp28->Dsp54
#define CMD_90 0x3930 //  Boot
#define CMD_91 0x3931 //  Actual Data
#define CMD_92 0x3932 //  Curve Data
#define CMD_93 0x3933 //  Dsp28->DSP54->HMI.
#define CMD_94 0x3934 //  Dsp28->DSP54->HMI.
#define CMD_95 0x3935 //  Echo Ac slow actual data

#define CMD_NULL NULL
#define MOTIONCMD_RSVD 0xFFFF

#define SUBSTEP_4BIT1 1
#define SUBSTEP_4BIT2 2
#define SUBSTEP_4BIT3 3
#define SUBSTEP_4BIT4 4
#define SUBSTEP_RSVD 0xFFFF

#define COMP_MOLD_ERROR_SUBINDEX 0x10000
#define COMP_EJECT_ERROR_SUBINDEX 0x20000
#define COMP_INJECT_ERROR_SUBINDEX 0x30000
#define COMP_CHARGE_ERROR_SUBINDEX 0x40000
#define COMP_CARRIAGE_ERROR_SUBINDEX 0x50000

#define FAST_91_SENDCYCLE 2    //(20210319 shanll modify 3->2)(20141113 THJ(Add: ))//(20140618 THJ(Add: ))
#define SLOW_91_SENDCYCLE 1000 //(20140917 THJ(Add: ))

#define MOTIONAXIS_91_WARN_OFFSET 23 //(20141021 THJ(Add: ))

#define PACK91DATA_MOTIONAXIS_SIZE (32 * 8 * sizeof(WORD))
#define PACK91DATA_REMOTEIO_SIZE (32 * sizeof(WORD))
#define PACK91DATA_MOTIONDA_SIZE (sizeof(Pack91Data_MotionDA_T) * sizeof(WORD))

#define PACK91DATA_MOTIONAXIS_OFFSET 0
#define PACK91DATA_REMOTEIO_OFFSET (PACK91DATA_MOTIONAXIS_OFFSET + PACK91DATA_MOTIONAXIS_SIZE)
#define PACK91DATA_MOTIONDA_OFFSET (PACK91DATA_REMOTEIO_OFFSET + PACK91DATA_REMOTEIO_SIZE)

#define ACTSTEP_MAX 4
enum enum_RemoteDevice_CommType
{
	REMOTEDEVICE_COMMTYPE_NO_DEVICE = 0,
	REMOTEDEVICE_COMMTYPE_M28_LOCAL = 1,
	REMOTEDEVICE_COMMTYPE_M28_CAN1  = 2,
	REMOTEDEVICE_COMMTYPE_M28_CAN2  = 3,
	REMOTEDEVICE_COMMTYPE_M28_ECAT  = 4
};

typedef struct
{
	struct
	{
		WORD bNodeID : 7;
		WORD bReserve0 : 1;
		WORD bNodeState : 1;
		WORD bReserve1 : 7;
	} sState;
	WORD wEndStart;
} Temper91SegHead_T;

typedef struct
{
	WORD wCurrent;
	WORD wState;
} Temper91Segment_T;

typedef struct TAAMsg
{
	struct
	{
		WORD bMotion1 : 8;
		WORD bMotion2 : 8;
	} asAxisConfig[8]; //WORD 1~8
	WORD wCycle5528J6; // WORD 9, Anders 2019-11-07, modify.
	WORD wReserve;     //WORD 10
	struct
	{
		WORD bNodeID : 8;
		WORD bDeviceType : 8;
		WORD bCommCycle : 8;
		WORD bCommType : 8;
	} asRemoteIOCANRulerConfig[13];      //WORD 11~36
	WORD awRemoteIOOfflineOutputDef[16]; //WORD 37~52
	//shanll add 20200608 add 2 remoteIO devices
	struct
	{
		WORD bNodeID : 8;
		WORD bDeviceType : 8;
		WORD bCommCycle : 8;
		WORD bCommType : 8;
	} asRemoteIOCANRulerConfig2[2];      //WORD 53~56
} TAAMsg;
//-------------------------------------------------------------------------------------------------
const IMECOMMMAP g_CMDMap[] = {
	{ CMD_90, &GetEcho_Boot },
	{ CMD_91, &GetEcho_AD_DA },
	{ CMD_93, &GetEcho_RetransmitData },
	{ CMD_94, &GetEcho_RetransmitDataLess },
	{ CMD_95, &GetEcho_SlowActual },

	{ CMD_AA, &GetMasterPowerOn },
	{ CMD_AB, &GetMasterSoftBoot }, // Anders 2010-11-05.
	{ CMD_AC, &GetSlowActual },

	{ CMD_MA, &GetConfiguration },
	{ CMD_MB, &GetOperateState },
	{ CMD_MC, &GetRetransmitData },
	{ CMD_MD, &GetRetransmitDataLess },

	{ CMD_LA, &GetMachineParameters1 },
	{ CMD_LB, &GetMachineParameters2 },
	{ CMD_LC, &GetMoldSet1 },
	{ CMD_LD, &GetMoldSet2 },
	{ CMD_LE, &GetMachineParameters_B },
	{ CMD_LF, &GetMoldSet_B1 },
	{ CMD_LG, &GetMoldSet_B2 },

	{ CMD_NULL, NULL }
};

//-------------------------------------------------------------------------------------------------
// zholy20100209
const IOETH CNST_DSP54IP = {
	PROTOCOL_IP,
	{ { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 } },
	{ { IPADDR_0, IPADDR_1 }, { IPADDR_2, IPADDR_3 } },
	{ { 169, 254 }, { 11, 12 } },
	2312,
	2312,
	ParseNetData
};

MOTION_CONFIG MOTIONCONFIG_TABLE[] = {
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_CLSM },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_CLSM2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_EJECTOR },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_EJECTOR2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_INJECT },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_INJECT2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_RECOVERY },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_RECOVERY2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_NOZZLE },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_NOZZLE2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_ROTERY },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_ROTERY2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_OTHER1 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_OTHER2 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_OTHER3 },
	{ (MOTIONCMDTYPE_DA << 8) | MOTIONID_OTHER4 },

	{ (MOTIONCMDTYPE_ONEMOTION << 8) | MOTIONID_TEMPERCARD },
	{ (MOTIONCMDTYPE_ONEMOTION << 8) | MOTIONID_M28DA },
	{ (MOTIONCMDTYPE_ONEMOTION << 8) | MOTIONID_REMOTEIO }
};
/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/
IMETEMPBUFFER  u_TempTxBuffer;
NetCycleResp_S u_sAutoRespList_Fast = { FALSE, FAST_91_SENDCYCLE, 0, Send91_Fast };
NetCycleResp_S u_sAutoRespList_Slow = { FALSE, SLOW_91_SENDCYCLE, 0, Send91_Slow };
Ttimer         u_dwCommReadyCount;
void (*u_fCommFunc)(void);

//MSGSTATE      MsgState = {0, 0};
BOOTSTATE g_BootState        = { 0, RESETBOOTTIME };
BOOTSTATE g_NetStopState     = { 0, STOPCHECKTIME };
WORD      g_wMasterReady     = 0;
int       g_nDsp54NetSession = -1;
// BOOL      u_bDisableInt      = FALSE;
BOOL  g_bDealDataImmediately = FALSE;
FLOAT u_fRullerRange         = 0;

//test
//#define       TEST_NET 1
#ifdef TEST_NET
WORD g_wTest_SendTestDataCount      = 0;
WORD g_wTest_WriteDBFailCount       = 0;
WORD g_wTest_RetransmitDataCount    = 0;
WORD g_wTest_SendBootCount          = 0;
WORD g_wTest_HMIRetransmitDataCount = 0;
WORD g_wTest_SendAD_DACount         = 0;
WORD g_wTest_RxSoftBootCount        = 0;
WORD g_wTest_RxOtherCMDCount        = 0; // Anders 2010-12-1.
WORD g_wTest_SelfStopCount          = 0; // Anders 2010-12-1.
WORD g_wTest_EchoCount              = 0;

TOperateStateBlock g_OperateStateBlock; // Anders 2015-1-13, Add.
Fast91_T           g_Fast91;            // Anders 2015-1-23, Add.
#endif

//+++++i
//  Motion time
//+++++

//DWORD               u_adwMotion_StartTime[MAX_TIMERECORD_MOTION];

DWORD u_dwEjecatorFwd_StartTime = 0;
DWORD u_dwEjectorBwd_StartTime  = 0;
DWORD u_dwInject_StartTime      = 0;
DWORD u_dwSuckBack_StartTime    = 0;

// WORD                u_wLastInjStatus        =   0;
// WORD                u_wLastChargeStatus     =   0;
// WORD                u_wLastMoldStatus       =   0;
// WORD                u_wLastEjectStatus      =   0;
// WORD                u_wLastNozzleStatus     =   0;

// MOTION_ACTSTATUS    g_LastInjectAxisStatus;
// MOTION_ACTSTATUS    g_LastChargeAxisStatus;

//TOperateState     g_CurrOperateState;
TNetcomState g_MasterComState = { 0 };
//TNetToken         u_ActDataToken      = {TOKENID_NULL,0,0};
//TNetToken         u_RetransmitToken   = {TOKENID_NULL,0,0};
volatile TMotionCheck g_SendDoneCheck;
TMasterType           g_ActDataType    = { 3 }; // hankin 20190611 modify: "1" -> "3". // 0: Old protocal; 1: 91 send Alarm changed. // Anders 2018-7-11 modify.
TMasterType           g_RetransmitType = { 0 };

TOperateMode 	g_OperateMode 		= { 0 };
TOperateStep1 	g_OperateStep1 		= { 0 };//shanll 20201111 add
TOperateStep1 	g_OperateStep1last 	= { 0 };//shanll 20201111 add

long  g_lSendRetransmitData  = 0;
long  g_lRecvRetransmitData  = 0;
long  g_lHmiTransmitData     = 0;
long  g_lHmiTruanmitEchoData = 0;
DWORD u_lRecvInterval        = 0;

//(20140917 THJ(Modification: )
#ifdef DEBUG
DWORD g_dwMotionDA91      = 100;
DWORD g_dwTemperCardCount = 0;

// WORD u_TestPres = 123;

#endif
WORD   g_wManualKey_ClrErrFlag  = 0;
Ttimer g_lManualKey_ClrErrCount = { 0 };

#ifdef DO_55_28_NET_RESET
#define NET_55_28_RESET_TIMER 10000
long g_lTestResetTimer = 0;
#endif

WORD g_wCommXPLC_backup; // THJ 2017-05-13 add.
WORD g_wHydraulicSpdSetpoint;

bool g_bAutoMode;
// CPULOAD g_CPUDealDataLoad = { 0, 0, 0, 0 };  // CX mask 20181208
WORD g_wCycle5528J6 = 0; // Anders 2019-11-07, add.
//SHANLL Add 20200512
bool  g_bHMIDataParsed= false;
NETDATATEMPBUFFER  u_TempParseBuffer;
WORD	wParseBufferLength = 0;

Fifo	u_HMIFifo;//shanll 20201209 add
/*===========================================================================+
|           Class implementation                                             |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD CreateTaskNet_master()
{
	g_wCommXPLC_backup      = 0; // THJ 2017-05-13 add.
	g_wHydraulicSpdSetpoint = 0;

	g_bAutoMode = false;

	//int i;
	CreateTaskNet_HMI();

	u_fCommFunc = COMMSTEP_IDLE;
	//memset (u_adwMotion_StartTime, 0, sizeof(u_adwMotion_StartTime) );

	g_nDsp54NetSession = OpenEthSession(DEVICEHANDLE_MASTER, (IOETH*)&CNST_DSP54IP); // Test for Kevin.

	//AddDnCounter(&g_BootState.lCounter);
	//AddDnCounter(&u_ActDataToken.lCounter);
	//AddDnCounter(&g_NetStopState.lCounter);
	//AddDnCounter(&u_RetransmitToken.lCounter);
	//AddDnCounter(&u_RetransmitToken.lInitCounter);

	memset(&u_TempTxBuffer, 0, sizeof(u_TempTxBuffer));
	memset(&g_BootState, 0, sizeof(g_BootState));
	ResetTimer(&g_BootState.lCounter, 0);
	ResetTimer(&g_NetStopState.lCounter, 0);

	//memset(&g_CurrOperateState,0,sizeof(TOperateState));
	StopTxADDA_OPState();
	return 1;
}

void DestoryTaskNet_master()
{
	//int   i,j;
	//DeleteDnCounter(&g_BootState.lCounter);
	//DeleteDnCounter(&g_NetStopState.lCounter);
	//DeleteDnCounter(&u_ActDataToken.lCounter);
	//DeleteDnCounter(&u_RetransmitToken.lCounter);
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
void RunTaskNet_master()
{
	// WORD wTokenOwner;
	// IMERESPLIST *pResp;
	// CheckEchoInvalid_ActData();
	// CheckEchoInit_RetransmitData();                                  //test for Kevin.
	// if(u_RetransmitToken.lCounter<0) u_RetransmitToken.lCounter = -1;    //test for Kevin.

	if (g_wMasterReady == 0)
	{
		if ((g_MasterComState.all & 0x03FF) == 0x03FF)
		{
			g_wMasterReady       = 1;
			g_BootState.wFlag    = 1;
			g_NetStopState.wFlag = 1;
			//ResetStopCheck();
			StartTxADDA_OPState(); // After revieced "MB".
			//StartTxCurveData();
			if (GetMoldsetState()) // Anders 2015-3-20, Add.
				StartTxTemperDataToMaster();
		}
	}
#ifdef DO_55_28_NET_RESET
	if (CheckTimerOut(&g_lTestResetTimer)) // Anders 2016-5-18, add.
	{
		if (g_wMasterReady != 1)
		{
			_InitNetDevice_ax88796b(DEVICEHANDLE_MASTER);
			_OpenNetDevice_ax88796b(DEVICEHANDLE_MASTER);

			//_mdio_write_ax88796b(DEVICEHANDLE_MASTER, 16,0,0xB100);
		}
		ResetTimer(&g_lTestResetTimer, NET_55_28_RESET_TIMER); // Test net reset.
	}
#endif
	//Boot
	if (g_BootState.wFlag == 0 && CheckTimerOut(&g_BootState.lCounter))
	{
		g_wMasterReady = 0;
		//===================
		// Send Boot Data.
		//==================
		//SendBootData(g_nDsp54NetSession);                     // Anders 2015-4-15, mark for 5528.
		ReqActivateEthSession(g_nDsp54NetSession);
		SendTestData(g_nDsp54NetSession, 0x0000, 2); // Anders 2015-4-22, Add for 55 Get 28 MAC.
		ResetBoot();

		g_NetStopState.wFlag = 0;
	}
	else if (CheckTimerOut(&g_BootState.lCounter))
	{
		//StopTxCurveData();
		StopTxADDA_OPState();
		g_wMasterReady                     = 0;
		g_MasterComState.bit.bOperateState = 0; // Anders 2016-6-20, add.
		//====================
		// Send Net Link Data.
		//====================
		SendTestData(g_nDsp54NetSession, 0x0000, 2); // Anders 2015-4-22, Add for 55 Get 28 MAC.
		//SendBootData(g_nDsp54NetSession);                     // Anders 2015-4-15, mark for 5528.
		ResetBoot();
		g_NetStopState.wFlag = 0;
	}
	// Anders 2014-4-14, add.
	g_sDB.LOCAL_AXISCARD.INTERFACE_NET_WMASTERREADY = g_MasterComState.all;

	//if(g_NetStopState.wFlag == 1 && CheckTimerOut(&g_NetStopState.lCounter))
	//{
	//    //g_CurrOperateState.ActStep1.all     =   0;
	//    ResetStopCheck();
	//
	//    #ifdef    TEST_NET
	//        g_wTest_SelfStopCount++;        // Anders 2010-12-1.
	//    #endif
	//}
	//(THJ 20150610 Modify:
	// for (i = 0; i < (sizeof(u_aAutoRespList) / sizeof(*u_aAutoRespList)); ++i)
	// {
	//  if (u_aAutoRespList[i].bFlag)
	//  {
	//      if (CheckTimerOut(&u_aAutoRespList[i].lCounter))
	//      {
	//          u_aAutoRespList[i].pFunc(g_nDsp54NetSession ,i);
	//          ResetTimer(&u_aAutoRespList[i].lCounter, u_aAutoRespList[i].bCycle);
	//      }
	//  }
	// }
	u_fCommFunc();
	//)
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
void ParseNetData(int nSessionHandle, WORD* pData, WORD wLength)
{
	int          i;
	IMETMHEADER* pHeader = (IMETMHEADER*)(pData);
	//WORD      wCRCValue,wCRCData;
	WORD wETX;

	if (pHeader->wSTX != STX)
		return;

	wETX = *((WORD*)pHeader + (pHeader->wSize - 1));

	//g_MasterComState.bit.bAnyData = 1;

	//================
	// Deal Data.
	//================
	if ((pHeader->wSTX == STX) && (wETX == ETX) && ((pHeader->wSize << 1) == wLength))
	{
		//================
		// CRC.
		//================
		// wCRCValue    = MakeCRCValue(&pHeader->wSize,pHeader->wSize-NOCRCSIZE);       //Form SIZE
		// wCRCData = *((WORD *)pHeader + NETHEADERLEN + pHeader->wDataLen);
		// if(wCRCValue != wCRCData)
		// {
		//     SendRespCMDResult(nSessionHandle,pData,RXFAIL);
		//     return;
		// }

		i = 0;
		while ((g_CMDMap[i].wCmd != pHeader->wCMD) && (g_CMDMap[i].wCmd != CMD_NULL))
			i++;
		if (g_CMDMap[i].wCmd != CMD_NULL)
		{
			if (g_CMDMap[i].pFunc != NULL)
			{
				g_CMDMap[i].pFunc(nSessionHandle, pData, pHeader->wDataLen); //wLength); Anders 2017-10-12 modify.
				                                                             //MC_DBData[CARD_INTERFACE_NET_DWRXRETRANSMITCOUNT].dbValue.dwData++;
			}
		}
	}
}
//==================================================================================================
void GetMasterPowerOn(int nSessionHandle, WORD* pData, WORD wLength)
{
	TAAMsg*     psAAMsg = (TAAMsg*)(pData + sizeof(IMETMHEADER));
	TMasterType sMasterType;
	E_CommType  eCommType;
	WORD        i;

	g_wCycle5528J6 = (psAAMsg->wCycle5528J6 & 0x1); // Anders 2019-11-07, add.
	memcpy(&sMasterType, pData + 2, sizeof(sMasterType));
	g_sDB.M28.MONITOR.VERSION_OF_MASTER = sMasterType.bit.bVersion;
	//MC_DBData[CARD_INTERFACE_NET_DWRXRETRANSMITCOUNT].dbValue.dwData++;
	StopTxADDA_OPState();
	//StopTxCurveData();

#ifdef DO_55_28_NET_RESET                                  // Anders 2016-5-18, add.
	ResetTimer(&g_lTestResetTimer, NET_55_28_RESET_TIMER); // Test net reset.
#endif
	// memset(psAAMsg->asAxisConfig, 8481, sizeof(psAAMsg->asAxisConfig));

	// Anders 2015-3-18, Add.
	memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_CONFIG_W1_MULTIPUMP_MOLD), psAAMsg, (DWORD)(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_CONFIG_W52_REMOTEIO_OFFLINE_DIRMODE_SET16) - DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_CONFIG_W1_MULTIPUMP_MOLD) + 1)); // 52W
	// Axis Config.
	for (i = 0; i < 10; i += 2)
	{
		unsigned j;
		union tagTMotionConfig {
			WORD wAll;
			struct
			{
				WORD bDAChannel : 4;
				WORD bMotionOpt : 4;
			} bit;
		} aMotionConfig[2];

		aMotionConfig[0].wAll = psAAMsg->asAxisConfig[i >> 1].bMotion1;
		if (aMotionConfig[0].bit.bDAChannel > 4 && aMotionConfig[0].bit.bDAChannel < 0xE)
			aMotionConfig[0].bit.bDAChannel = 0;
		aMotionConfig[1].wAll = psAAMsg->asAxisConfig[i >> 1].bMotion2;
		if (aMotionConfig[1].bit.bDAChannel > 4 && aMotionConfig[1].bit.bDAChannel < 0xE)
			aMotionConfig[1].bit.bDAChannel = 0;
		for (j = i; j < i + 2; j++)
		{
			MOTIONCONFIG_TABLE[j].wConfig = (MOTIONCMDTYPE_DA << 8) | (MOTIONCONFIG_TABLE[j].wConfig & 0xFF); // Default 0x010x. Anders 2015-1-22,add.
			switch (aMotionConfig[j - i].bit.bMotionOpt)
			{
				case MOTIONOPTION_NONE:
				case MOTIONOPTION_PRESSCTL:
					MOTIONCONFIG_TABLE[j].wConfig = (MOTIONCMDTYPE_DA << 8) | (MOTIONCONFIG_TABLE[j].wConfig & 0xFF);
					break;
				case MOTIONOPTION_WHOLE:
					MOTIONCONFIG_TABLE[j].wConfig = (MOTIONCMDTYPE_ONEMOTION << 8) | (MOTIONCONFIG_TABLE[j].wConfig & 0xFF);
					break;
				default:
					break;
			}
		}
	}

	//RemoteIO+CAN Ruler: pConfigData[10..34], 13 Devices.
	//DEBUG_ASSERT_ILLEGAL(REMOTEIONUM_SUM == (sizeof(psAAMsg->asRemoteIOCANRulerConfig) / sizeof(*psAAMsg->asRemoteIOCANRulerConfig)), "arr error.");
	//shanll modify 20200608 add 2 remoteIO devices
	DEBUG_ASSERT_ILLEGAL(REMOTEIONUM_SUM == (sizeof(psAAMsg->asRemoteIOCANRulerConfig) / sizeof(*psAAMsg->asRemoteIOCANRulerConfig))\
						+(sizeof(psAAMsg->asRemoteIOCANRulerConfig2) / sizeof(*psAAMsg->asRemoteIOCANRulerConfig2)), "arr error.");
	InitMap_RemoteIO();

	//for (i = 0; i < REMOTEIONUM_SUM; ++i)
	for (i = 0; i < REMOTEIONUM_CONFIG1_SUM; ++i)//shanll modify 20200608 add 2 remoteIO devices
	{
		switch (psAAMsg->asRemoteIOCANRulerConfig[i].bCommType)
		{
			//case REMOTEDEVICE_COMMTYPE_M28_LOCAL:
			//  eCommType = COMMTYPE_LOCAL;
			//  break;
			case REMOTEDEVICE_COMMTYPE_M28_CAN1:
				eCommType = COMMTYPE_CAN1;
				break;
			case REMOTEDEVICE_COMMTYPE_M28_CAN2:
				eCommType = COMMTYPE_CAN2;
				break;
			case REMOTEDEVICE_COMMTYPE_M28_ECAT:
				eCommType = COMMTYPE_ETHERCAT;
				break;
			default:
				eCommType = COMMTYPE_SUM;
				break;
		}
		Map_RemoteIO((E_RemoteIODeviceType)psAAMsg->asRemoteIOCANRulerConfig[i].bDeviceType, eCommType, psAAMsg->asRemoteIOCANRulerConfig[i].bNodeID, psAAMsg->asRemoteIOCANRulerConfig[i].bCommCycle);
	}
	//shanll add 20200608 add 2 remoteIO devices
	for (i = 0; i < REMOTEIONUM_SUM-REMOTEIONUM_CONFIG1_SUM; ++i)
	{
		switch (psAAMsg->asRemoteIOCANRulerConfig2[i].bCommType)
		{
			//case REMOTEDEVICE_COMMTYPE_M28_LOCAL:
			//  eCommType = COMMTYPE_LOCAL;
			//  break;
			case REMOTEDEVICE_COMMTYPE_M28_CAN1:
				eCommType = COMMTYPE_CAN1;
				break;
			case REMOTEDEVICE_COMMTYPE_M28_CAN2:
				eCommType = COMMTYPE_CAN2;
				break;
			case REMOTEDEVICE_COMMTYPE_M28_ECAT:
				eCommType = COMMTYPE_ETHERCAT;
				break;
			default:
				eCommType = COMMTYPE_SUM;
				break;
		}
		Map_RemoteIO((E_RemoteIODeviceType)psAAMsg->asRemoteIOCANRulerConfig2[i].bDeviceType, eCommType, psAAMsg->asRemoteIOCANRulerConfig2[i].bNodeID, psAAMsg->asRemoteIOCANRulerConfig2[i].bCommCycle);
	}
	for (i = 0; i < PACK91DATA_REMOTEIO_OUTPUT_NUM; ++i)
	{
		SetOfflineOutput_RemoteIO(i, psAAMsg->awRemoteIOOfflineOutputDef[i]);
	}
	for (i = 0; i < CommComp_turntable_INDEX_SUM; ++i)
	{
		WORD wDAChannel;
		WORD wConfigIndex;

		wConfigIndex = ACTAXIS_ROTARY + i;
		if (0 == wConfigIndex % 2)
		{
			wDAChannel = psAAMsg->asAxisConfig[wConfigIndex / 2].bMotion1 & 0xF;
		}
		else
		{
			wDAChannel = psAAMsg->asAxisConfig[wConfigIndex / 2].bMotion2 & 0xF;
		}
		CommComp_turntable_set_PF_channel((enum CommComp_turntable_e_index)i, wDAChannel);
	}

	SendBootData(g_nDsp54NetSession);
	g_wMasterReady = 0;
	ResetBoot();
	g_MasterComState.bit.bBoot = 1; // Anders 2010-10-29.

	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS); // Anders 2010-10-29.
}
void GetMasterSoftBoot(int nSessionHandle, WORD* pData, WORD wLength)
{
#ifdef TEST_NET
	g_wTest_RxSoftBootCount++;
#endif
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS); // Anders 2010-10-29.

	Command_ManualKey();
}

void GetSlowActual(int nSessionHandle, WORD* pData, WORD wLength)
{
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);

	//SendSlowActual(nSessionHandle);
}

//==================================================================================================
void GetOperateState(int nSessionHandle, WORD* pData, WORD wLength)
{
	static WORD wHeatFlag             = 0;
	static WORD wPipeHalfTemperSwitch = 0, wMoldHalfTemperSwitch = 0;
	//  static WORD  awActStepBak[ACTSTEP_MAX]  = { 0, 0, 0, 0 };
	//  static DWORD dwHalfwayStopPosition_Mold = 0, dwHalfwayStopPosition_Mold2 = 0, dwHalfwayStopPosition_Inject = 0, dwHalfwayStopPosition_Inject2 = 0;

	// static WORD                wCommXPLC_Rotate_PBOver[2] = { 0, 0 }; // THJ 2017-05-13 mask.
	int i, nOperateModeBit;
	//  WORD                       awActStep[ACTSTEP_MAX], wStep, wChl;
	//  WORD                       awActStepChange[ACTSTEP_MAX]; //,wErrorFlag = 0;
	TOperateStateBlock* pOperateStateBlock;
	J6PP_OperateStateBlock* pJ6PPOperateStateBlock;//SHANLL 20191223 ADD
	//  const TOperateStepMap*     pOperateStepMap = NULL;
	//  const TActStepLinkActAxis* pActStepLinkActAxis;
	DBVALUE dbValue; //static  DBVALUE Value                                               =   {0};                  //(20141027 THJ(Modification: ))
	                 //  DWORD                      dwTemp;
	// Anders 2014-4-14, add.

	++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXMASTERCMDCOUNT;

	if ((g_MasterComState.all & 0x1FF) != 0x1FF)
		return;

	g_MasterComState.bit.bOperateState = 1;
	if (g_wMasterReady)
	{
		ResetBoot();
		//ResetStopCheck();
	}
	pOperateStateBlock = (TOperateStateBlock*)(pData + NETHEADERLEN);
	pJ6PPOperateStateBlock = (J6PP_OperateStateBlock*)((WORD*)pOperateStateBlock + sizeofW(TOperateStateBlock));
	g_OperateMode      = pOperateStateBlock->OperateMode;
	g_OperateStep1	   = pOperateStateBlock->OperateStep1;

	// MC_WRITEDB_DWORD(AXIS1_INTERFACE_ERROR11, (pOperateStateBlock->OperateMode.all));

	nOperateModeBit = ScanOperateModeByBit(pOperateStateBlock->OperateMode.all);
	//  awActStep[0]    = pOperateStateBlock->ActStep1.all;
	//  awActStep[1]    = pOperateStateBlock->ActStep3.all;
	//  awActStep[2]    = pOperateStateBlock->ActStep6.all;
	//  awActStep[3]    = pOperateStateBlock->ActStep7.all;

	// Anders 2015-3-18, Add monitor.
	memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_PRESS), &pOperateStateBlock->PressFlow, (DWORD)(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL4_SET_FLOW) - DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_PRESS) + 1));          //8W
	memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL1_PUMP_PID_SET), &pOperateStateBlock->Pump_PID, (DWORD)(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL4_PUMP_PID_SET) - DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL1_PUMP_PID_SET) + 1)); // 52W

	// if (g_bDealDataImmediately == TRUE || (pOperateStateBlock->Pump_PID[0].bit.bControlType != g_AxisManager[ACTAXIS_INJECT].Pump.bit.bControlType))
	// {
	//  CalcCPULoadStart(&g_CPUDealDataLoad);
	// }

	memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_PRESS_SCALE), &pOperateStateBlock->PressFlowScale, (DWORD)(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL4_SET_FLOW_SCALE) - DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_CHL1_SET_PRESS_SCALE) + 1)); //8W

	//  for (i = 0; i < ACTSTEP_MAX; i++)
	//      DB_SetWORD(DBINDEX_M28_MONITOR_COMM_MB_ACTSTEP1 + i, awActStep[i]);
	memcpy(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_MOLD1POSITION), &pOperateStateBlock->dwMold1Posi, (DWORD)(DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_INJECT2POSITION) - DB_GetDataAddr(DBINDEX_M28_MONITOR_COMM_MB_MOLD1POSITION) + 1)); //8W // Anders 2015-5-5, Del "<<1".
	DB_SetDWORD(DBINDEX_PICKER_INTERFACE_MOLD_POSI, DB_GetDWORD(DBINDEX_M28_MONITOR_COMM_MB_MOLD1POSITION));
	DB_SetWORD(DBINDEX_PICKER_INTERFACE_EJECT_POSI, DB_GetDWORD(DBINDEX_M28_MONITOR_COMM_MB_EJECT1POSITION) * 0.1);

	//  for (i = 0; i < ACTAXIS_MAX; i++)
	//  {
	//      u_fRullerRange = 0;
	//      switch (i)
	//      {
	//          case ACTAXIS_MOLD:
	//              memcpy(&g_AxisManager[i].dwExternPosi, &pOperateStateBlock->dwMold1Posi, sizeof(g_AxisManager[i].dwExternPosi));
	//              break;
	//          case ACTAXIS_MOLD2:
	//              memcpy(&g_AxisManager[i].dwExternPosi, &pOperateStateBlock->dwMold2Posi, sizeof(g_AxisManager[i].dwExternPosi));
	//              break;
	//          case ACTAXIS_EJECT:
	//              memcpy(&g_AxisManager[i].dwExternPosi, &pOperateStateBlock->dwEject1Posi, sizeof(g_AxisManager[i].dwExternPosi));
	//              break;
	//          case ACTAXIS_EJECT2:
	//              memcpy(&g_AxisManager[i].dwExternPosi, &pOperateStateBlock->dwEject2Posi, sizeof(g_AxisManager[i].dwExternPosi));
	//              break;
	//          case ACTAXIS_INJECT:
	//          {
	//              memcpy(&g_AxisManager[i].dwExternPosi, &pOperateStateBlock->dwInject1Posi, sizeof(g_AxisManager[i].dwExternPosi));
	//              u_fRullerRange = 0.1 * DB_GetWORD(DBINDEX_INJECTVALVE0_GENERAL_VALVE_SPEED_OUT_RAMP);
	//              break;
	//          }
	//          case ACTAXIS_INJECT2:
	//          {
	//              memcpy(&g_AxisManager[i].dwExternPosi, &pOperateStateBlock->dwInject2Posi, sizeof(g_AxisManager[i].dwExternPosi));
	//              u_fRullerRange = 0.1 * DB_GetWORD(DBINDEX_INJECTVALVE1_GENERAL_VALVE_SPEED_OUT_RAMP);
	//              break;
	//          }
	//          default:
	//              g_AxisManager[i].dwExternPosi = 0;
	//              break;
	//      }
	//      // if (g_AxisManager[i].CalcExternVelo.dwMBRecvCounter == 0 && g_AxisManager[i].CalcExternVelo.wMBStartCalcFlg == 0)
	//      /*if (g_AxisManager[i].CalcExternVelo.wMBStartCalcFlg == 0)
	//      {
	//          g_AxisManager[i].CalcExternVelo.wMBStartCalcFlg = 1;
	//          CalcCPULoadStart(&g_AxisManager[i].MBMonitor);
	//      }
	//
	//      // if (g_AxisManager[i].CalcExternVelo.dwMBRecvCounter++ >= CAL_ACTSPEED_CYCLETIME)
	//      CalcCPULoadEnd(&g_AxisManager[i].MBMonitor);
	//      g_AxisManager[i].CalcExternVelo.fActPosiDeltaTimer = 0.000006667 * g_AxisManager[i].MBMonitor.dwCurrentCounter;
	//
	//      if (g_AxisManager[i].CalcExternVelo.fActPosiDeltaTimer >= CAL_ACTSPEED_CYCLETIME)
	//      {
	//          // CalcCPULoadEnd(&g_AxisManager[i].MBMonitor);
	//          // g_AxisManager[i].CalcExternVelo.dwMBRecvCounter    = 0;
	//          // g_AxisManager[i].CalcExternVelo.fActPosiDeltaTimer = 0.000006667 * g_AxisManager[i].MBMonitor.dwCurrentCounter; // ms
	//          // g_AxisManager[i].CalcExternVelo.fExtActVelocity = ABS(g_AxisManager[i].dwExternPosi * 10.0 - g_AxisManager[i].dwLastExternPosi * 10.0) / g_AxisManager[i].CalcExternVelo.fActPosiDeltaTimer;
	//          g_AxisManager[i].CalcExternVelo.fExtActVelocity = ABS(_IQ16toF(g_AxisManager[i].dwExternPosi & 0xFFFF) * u_fRullerRange * 1000.0 - _IQ16toF(g_AxisManager[i].dwLastExternPosi & 0xFFFF) * u_fRullerRange * 1000.0) / g_AxisManager[i].CalcExternVelo.fActPosiDeltaTimer;
	//          g_AxisManager[i].dwLastExternPosi               = g_AxisManager[i].dwExternPosi;
	//          CalcCPULoadStart(&g_AxisManager[i].MBMonitor);
	//      }*/
	//      // CX  mask for 28 pharse 55 MB timer is not very accurate, so  55 give the velocity by himself. 20181110
	//
	//      g_AxisManager[i].CalcExternVelo.fExtActVelocity = ABS(_IQ16toF(g_AxisManager[i].dwExternPosi & 0xFFFF)) * u_fRullerRange * 1000.0;
	//  }
	if (nOperateModeBit >= OM_SEMI && nOperateModeBit <= OM_TIME) // 20170330 CX add for judge motion mode
		g_bAutoMode = true;
	else
		g_bAutoMode = false;

		//  g_MoldPDValve.bit.bCHL       = ((awActStep[3] & 0x60) >> 5);
		//  g_MoldPDValve.bit.bDirection = ((awActStep[3] & 0x180) >> 7);

		// g_MoldPDValve.all = ((awActStep[3] & 0x01E0) >> 5); // CX mark 20171230 // Anders 2015-4-26, Add.

		//  g_bFatigureAmpFlag = (awActStep[3] & 1);

		// MC_WRITEDB_DWORD(AXIS1_INTERFACE_ERROR11, g_MoldPDValve.bit.bAutoMode);

		//  awActStep[3] &= 0xFE1F; // Anders 2015-4-26, Add.

		//  if (g_wManualKey_ClrErrFlag == 0)
		//  {
		//      for (i = 0; i < ACTSTEP_MAX; i++)
		//      {
		//          awActStepChange[i] = awActStep[i] ^ awActStepBak[i];
		//          awActStepBak[i]    = awActStep[i];
		//      }
		//  }

#ifdef TEST_NET
	memcpy(&g_OperateStateBlock, pOperateStateBlock, sizeof(TOperateStateBlock));
#endif

	// Stop Position.
	//  memcpy(&g_AxisManager[ACTAXIS_MOLD].dwHalfwayStopPosition, &pOperateStateBlock->dwStopPosi_Mold1, sizeof(DWORD));
	//  if (dwHalfwayStopPosition_Mold != g_AxisManager[ACTAXIS_MOLD].dwHalfwayStopPosition)
	//  {
	//      dbValue.fData = g_AxisManager[ACTAXIS_MOLD].dwHalfwayStopPosition * 0.01;
	//      //SetDBValueByIndex( MOLD_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue );   // Anders 2015-1-22, add.
	//      DB_SetFLOAT(DBINDEX_MOLD0_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue.fData); // AXIS1_INTERFACE_CONTROL_FHALFWAYSTOPPOSITION
	//      dwHalfwayStopPosition_Mold = g_AxisManager[ACTAXIS_MOLD].dwHalfwayStopPosition;
	//  }
	//
	//  memcpy(&g_AxisManager[ACTAXIS_INJECT].dwHalfwayStopPosition, &pOperateStateBlock->dwStopPosi_Inject1, sizeof(DWORD));
	//  if (dwHalfwayStopPosition_Inject != g_AxisManager[ACTAXIS_INJECT].dwHalfwayStopPosition)
	//  {
	//      dbValue.fData = g_AxisManager[ACTAXIS_INJECT].dwHalfwayStopPosition * 0.01;
	//      //SetDBValueByIndex( MOLD_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue );   // Anders 2015-1-22, add.
	//      DB_SetFLOAT(DBINDEX_INJECT0_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue.fData); // AXIS1_INTERFACE_CONTROL_FHALFWAYSTOPPOSITION
	//      dwHalfwayStopPosition_Inject = g_AxisManager[ACTAXIS_INJECT].dwHalfwayStopPosition;
	//  }
	//
	//  memcpy(&g_AxisManager[ACTAXIS_MOLD2].dwHalfwayStopPosition, &pOperateStateBlock->dwStopPosi_Mold2, sizeof(DWORD));
	//  if (dwHalfwayStopPosition_Mold2 != g_AxisManager[ACTAXIS_MOLD2].dwHalfwayStopPosition)
	//  {
	//      dbValue.fData = g_AxisManager[ACTAXIS_MOLD2].dwHalfwayStopPosition * 0.01;
	//      //SetDBValueByIndex( MOLD_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue );   // Anders 2015-1-22, add.
	//      DB_SetFLOAT(DBINDEX_MOLD1_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue.fData); // AXIS1_INTERFACE_CONTROL_FHALFWAYSTOPPOSITION
	//      dwHalfwayStopPosition_Mold2 = g_AxisManager[ACTAXIS_MOLD2].dwHalfwayStopPosition;
	//  }
	//
	//  memcpy(&g_AxisManager[ACTAXIS_INJECT2].dwHalfwayStopPosition, &pOperateStateBlock->dwStopPosi_Inject2, sizeof(DWORD));
	//  if (dwHalfwayStopPosition_Inject2 != g_AxisManager[ACTAXIS_INJECT2].dwHalfwayStopPosition)
	//  {
	//      dbValue.fData = g_AxisManager[ACTAXIS_INJECT2].dwHalfwayStopPosition * 0.01;
	//      //SetDBValueByIndex( MOLD_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue );   // Anders 2015-1-22, add.
	//      DB_SetFLOAT(DBINDEX_INJECT1_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue.fData); // AXIS1_INTERFACE_CONTROL_FHALFWAYSTOPPOSITION
	//      dwHalfwayStopPosition_Inject2 = g_AxisManager[ACTAXIS_INJECT2].dwHalfwayStopPosition;
	//  }

	// dbValue.fData = 50.00; //g_AxisManager[ACTAXIS_MOLD].dwHalfwayStopPosition * 0.01;
	// //SetDBValueByIndex( MOLD_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue );   // Anders 2015-1-22, add.
	// DB_SetFLOAT(DBINDEX_MOLD0_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue.fData); // AXIS1_INTERFACE_CONTROL_FHALFWAYSTOPPOSITION
	// DB_SetFLOAT(DBINDEX_INJECT0_INTERFACE_CONTROL_HALFWAYSTOPPOSITION, dbValue.fData);

	g_wCommXPLC_backup      = pOperateStateBlock->CommXPLC.all; // THJ 2017-05-13 add.
	g_wHydraulicSpdSetpoint = pOperateStateBlock->wHydraulicSpdSetpoint;

	DB_SetWORD(DBINDEX_M28_MONITOR_COMM_MB_RSVD, g_wHydraulicSpdSetpoint); //g_wCommXPLC_backup);

	// Parse CMD.
	//  for (wStep = 0; wStep < ACTSTEP_MAX; wStep++)
	//  {
	//      // if (!awActStepChange[wStep] && !awActStep[wStep])
	//      // {
	//      //     continue; // Anders 2015-5-5, Add.
	//      // }
	//      for (i = 0; i < 16; i++)
	//      {
	//          WORD wReplaceCh;
	//
	//          pActStepLinkActAxis = paActStepLinkActAxis[wStep] + i;
	//          if (pActStepLinkActAxis->wActAxis >= ACTAXIS_MAX)
	//              continue; // Anders 2015-3-31, Add.
	//
	//          // Press, Flow.
	//          wChl       = g_AxisManager[pActStepLinkActAxis->wActAxis].MotionConfig.bit.bDAChannel - 1;
	//          wReplaceCh = wChl;
	//
	//          if (wChl < 4)
	//          {
	//              if ((pActStepLinkActAxis->wActAxis == ACTAXIS_INJECT || pActStepLinkActAxis->wActAxis == ACTAXIS_INJECT2) && pOperateStateBlock->Pump_PID[wChl].bit.bControlType != g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bControlType)
	//              {
	//                  // DINT;
	//                  // u_bDisableInt = TRUE;
	//                  g_bDealDataImmediately = TRUE;
	//                  //CalcCPULoadStart(&g_CPUDealDataLoad);
	//              }
	//              memcpy(&g_AxisManager[pActStepLinkActAxis->wActAxis].Pump, &pOperateStateBlock->Pump_PID[wChl], sizeof(TAxisPump)); // Anders 2017-8-31, Modify.
	//          }
	//
	//          if (pActStepLinkActAxis->wActAxis == ACTAXIS_INJECT || pActStepLinkActAxis->wActAxis == ACTAXIS_RECOVERY || g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bControlType == MOTIONTYPE_VALVESPEEDCTRL_FLOW_POV_OPENLOOP)
	//          {
	//              if (ReadDB_WORD(DBINDEX_INJECTVALVE0_CONFIG_BELONG_TO_OILWAY_NUM) > 0)
	//                  wReplaceCh = ReadDB_WORD(DBINDEX_INJECTVALVE0_CONFIG_BELONG_TO_OILWAY_NUM) - 1;
	//          }
	//          else if (pActStepLinkActAxis->wActAxis == ACTAXIS_INJECT2 || pActStepLinkActAxis->wActAxis == ACTAXIS_RECOVERY2 || g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bControlType == MOTIONTYPE_VALVESPEEDCTRL_FLOW_POV_OPENLOOP)
	//          {
	//              if (ReadDB_WORD(DBINDEX_INJECTVALVE1_CONFIG_BELONG_TO_OILWAY_NUM) > 0)
	//                  wReplaceCh = ReadDB_WORD(DBINDEX_INJECTVALVE1_CONFIG_BELONG_TO_OILWAY_NUM) - 1;
	//          }
	//
	//          if (wChl < 4)
	//          {
	//              memcpy(&g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow, &pOperateStateBlock->PressFlow[wChl], sizeof(TChlPressFlow)); // Anders 2017-8-31, Modify.
	//              // memcpy(&g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlowScale, &pOperateStateBlock->PressFlowScale[wChl], sizeof(TChlPressFlowScale)); // Anders 2017-8-31, Modify.
	//              g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlowScale.wPressScale = pOperateStateBlock->PressFlowScale[wChl].wPressScale;
	//              if (pActStepLinkActAxis->wActAxis == ACTAXIS_RECOVERY2 || pActStepLinkActAxis->wActAxis == ACTAXIS_RECOVERY)
	//                  g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlowScale.wFlowScale = pOperateStateBlock->PressFlow[wReplaceCh].wPress;
	//              else
	//                  g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlowScale.wFlowScale = pOperateStateBlock->PressFlowScale[wReplaceCh].wFlowScale;
	//
	//              // if (u_bDisableInt == TRUE)
	//              // {
	//              //  EINT;
	//              //  u_bDisableInt = FALSE;
	//              // }
	//              // memcpy(&g_AxisManager[pActStepLinkActAxis->wActAxis].Pump, &pOperateStateBlock->Pump_PID[wChl], sizeof(TAxisPump)); // Anders 2017-8-31, Modify.
	//
	//              // g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bPumpEnable = 1;
	//
	//              if (awActStep[wStep] & (1 << i))
	//              {
	//
	//                  //memcpy(&g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow, &pOperateStateBlock->PressFlow[wChl], sizeof(TChlPressFlow)); // Anders 2017-8-31, mark.
	//                  //memcpy(&g_AxisManager[pActStepLinkActAxis->wActAxis].Pump, &pOperateStateBlock->Pump_PID[wChl], sizeof(TAxisPump));           // Anders 2017-8-31, mark.
	//
	//                  // Anders 2015-3-18, Add. May be Todo here......
	//
	//                  if (g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bHoldCMDFlag == 1 && g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bControlType == 0)
	//                  {
	//                      g_AxisManager[pActStepLinkActAxis->wActAxis].wCurrCMD_Comm = CMD_STOPCTL;
	//                  }
	//                  else
	//                  {
	//                      if (g_AxisManager[pActStepLinkActAxis->wActAxis].Pump.bit.bHoldCMDFlag == 1 || (g_AxisManager[pActStepLinkActAxis->wActAxis].wCurrCMD_Comm == CMD_STOPCTL && g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow.wPress > 0 && g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow.wFlow > 0))
	//                          g_AxisManager[pActStepLinkActAxis->wActAxis].wCurrCMD_Comm = CMD_PRESSCTL_MOTOR_EXTCMD;
	//#ifndef PANSTONE_SIN_FATIGURE_CTRL
	//                      else if ((g_AxisManager[pActStepLinkActAxis->wActAxis].wCurrCMD_Comm >= CMD_PRESSCTL_MOTOR_EXTCMD && g_AxisManager[pActStepLinkActAxis->wActAxis].wCurrCMD_Comm < CMD_MA_MAX) && (g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow.wPress == 0 || g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow.wFlow == 0))
	//                          g_AxisManager[pActStepLinkActAxis->wActAxis].wCurrCMD_Comm = CMD_STOPCTL;
	//#endif
	//                  }
	//              }
	//          }
	//          //else
	//          //  memset(&g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow, 0, sizeof(TChlPressFlow));  // Anders 2015-05-04, Add. 2015-05-05, mark.
	//
	//          if (awActStepChange[wStep] & (1 << i))
	//          {
	//              if (nOperateModeBit == -1)
	//                  pOperateStepMap = &(paOperateStateMap[wStep] + cnstOperateMode[OM_MANUAL].wMotionType)->OperateStepMap[i];
	//              else if (nOperateModeBit >= OM_MANUAL && nOperateModeBit <= OM_AA_COM)
	//                  pOperateStepMap = &(paOperateStateMap[wStep] + cnstOperateMode[nOperateModeBit].wMotionType)->OperateStepMap[i];
	//              else
	//                  pOperateStepMap = NULL;
	//
	//              if (!(awActStep[wStep] & (1 << i)))
	//              {
	//                  memset(&g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlow, 0, sizeof(TChlPressFlow));           // Anders 2015-05-05, Add.
	//                  memset(&g_AxisManager[pActStepLinkActAxis->wActAxis].PressFlowScale, 0, sizeof(TChlPressFlowScale)); // Anders 2017-8-31, Modify.
	//              }
	//
	//              if (pOperateStepMap != NULL && pOperateStepMap->wMotionCMD != MOTIONCMD_RSVD)
	//              {
	//                  ParseOperateMotion(pOperateStateBlock->OperateMode, pActStepLinkActAxis->wActAxis, ((awActStep[wStep] & (1 << i)) ? pOperateStepMap->wMotionCMD : CMD_STOPCTL)); // awActStepChange -> awActStep. Anders 2015-3-25, Modify.
	//              }
	//
	//#if (HYBCARD_USED)
	//              // Anders 2017-1-13, Move code to here.
	//              if (pActStepLinkActAxis->wActAxis < sizeofW(cnst_awMotionAxis2DataAxis))
	//              {
	//                  E_AxisCard eCard;
	//
	//                  ////eAxis = (E_AxisCardAxis)(pActStepLinkActAxis->wActAxis >> 1);
	//                  ////if (pActStepLinkActAxis->wActAxis & 1)
	//                  ////    eAxis += AXISCARDAXIS_MOLD1;
	//                  eAxis = (E_AxisCardAxis)(cnst_awMotionAxis2DataAxis[pActStepLinkActAxis->wActAxis] - 1);
	//                  if ((eAxis < AXISCARDAXIS_SUM) && AXISCARD_SUM != (eCard = (E_AxisCard)GetCardByAxis_AxisCard(eAxis)))
	//                  {
	//                      WORD wCompState;
	//
	//                      //if ((eCard < AXISCARD_SUM) && (ReadDB_WORD(DBINDEX_LOCAL0_MOTION_CARD1_NODEID + eCard) != 0))
	//                      //  wCompState = GetCompState_AxisCard(eCard);
	//                      //else if (AXISCARD_LOCAL == eCard)
	//                      //  wCompState = GetCompState_AxisCard(eCard);
	//                      wCompState = GetCompState_AxisCard(eCard);
	//                      switch (wCompState)
	//                      {
	//                          case COMPSTATE_INIT:
	//                          case COMPSTATE_CONFIG:
	//                          {
	//                              Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_AXISCARDNOTREADY); // Anders 2016-5-26 modify, pActStepLinkActAxis->wActAxis. cnst_awDataAxis2MotionAxis[eAxis]
	//                              break;
	//                          }
	//
	//                          case COMPSTATE_SYNC:
	//                          {
	//                              Set_CommChipError(ACTAXIS_MOLD, ERROR_ID_AXIS_DATASYNC); // Anders 2016-5-26 modify, pActStepLinkActAxis->wActAxis.
	//                              break;
	//                          }
	//
	//                          case COMPSTATE_NORMAL:
	//                              break;
	//
	//                          default:
	//                              break;
	//                      }
	//                  }
	//              }
	//#endif
	//          }
	//      }
	//  }
	//
	//  // Stop All Axis. Anders 2015-3-25, Modify.
	//  if ((awActStep[0] | awActStep[1] | awActStep[2] | awActStep[3]) == 0)
	//  {
	//      for (i = 0; i < ACTAXIS_MAX; i++) // Stop All.
	//      {
	//          //if (g_AxisManager[i].wCurrCMD_Comm > CMD_HOMINGCTL)   // Anders 2017-1-13, mark for HX FastMachine.
	//          //    g_AxisManager[i].wCurrCMD_Comm = CMD_STOPCTL;
	//
	//          // Anders 2017-1-13, add for HX FastMachine.
	//          switch (g_AxisManager[i].wCurrCMD_Comm)
	//          {
	//              case CMD_STOPCTL:
	//                  break;
	//              case CMD_RESETCTL:
	//                  if (g_wManualKey_ClrErrFlag == 0)
	//                      g_AxisManager[i].wCurrCMD_Comm = CMD_STOPCTL;
	//              case CMD_HOMINGCTL:
	//                  break;
	//              default:
	//                  g_AxisManager[i].wCurrCMD_Comm = CMD_STOPCTL;
	//                  break;
	//          }
	//          memset(&g_AxisManager[i].PressFlow, 0, sizeof(TChlPressFlow));           // Anders 2015-05-05, Add.
	//          memset(&g_AxisManager[i].PressFlowScale, 0, sizeof(TChlPressFlowScale)); // Anders 2015-05-05, Add.
	//      }
	//  }

	for (i = 0; i < PACK91DATA_REMOTEIO_OUTPUT_NUM; ++i)
		SetOutput_RemoteIO(i, pOperateStateBlock->awRemoteIOOutput[i]);
	for (i = 0; i < PACK91DATA_MOTIONDA_DA_NUM; ++i)
		SetDA_RemoteIO(i, pOperateStateBlock->awRemoteIODA[i]);

	// Motor Enable.
	dbValue.vWORD = (0xFFFF == pOperateStateBlock->AdditionalOP.wMotorState) ? 1 : 0;

	dbValue.vWORD |= (pOperateStateBlock->AdditionalOP.OtherState.b3EmergencyStop << 8); // Anders 2016-11-30, Add.
	DB_SetWORD(DBINDEX_M28_MONITOR_COMM_MB_MOTORENABLE, dbValue.vWORD);                  // Anders 2016-10-10, Add.

	//  dwTemp = 0;
	//  for (i = 0; i < ACTAXIS_MAX; i++)
	//  {
	//      g_AxisManager[i].wMotorEnable_Comm = dbValue.vWORD;
	//      // Emergency Stop, Anders 2016-11-22, Add.
	//      g_AxisManager[i].wEmergencyStop = pOperateStateBlock->AdditionalOP.OtherState.b3EmergencyStop;
	//      if (g_AxisManager[i].wEmergencyStop && g_AxisManager[i].wCurrCMD_Comm > CMD_HOMINGCTL)
	//          g_AxisManager[i].wCurrCMD_Comm = CMD_STOPCTL;
	//
	//      dwTemp |= g_AxisManager[i].Error.dwAll; // Anders 2016-12-28, add.
	//  }
	// Alarm, Stop All MotionAxis.
	//if ( ReadDB_DWORD(DBINDEX_MOLD0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_MOLD1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_EJECT0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_EJECT1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_INJECT0_INTERFACE_ERROR1)
	//        | ReadDB_DWORD(DBINDEX_INJECT1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CHARGE0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CHARGE1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CARRIAGE0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CARRIAGE1_INTERFACE_ERROR1) )
	//  if (dwTemp)
	//  {
	//      for (i = 0; i < ACTAXIS_MAX; i++)
	//      {
	//          if (g_AxisManager[i].wCurrCMD_Comm > CMD_HOMINGCTL)
	//              g_AxisManager[i].wCurrCMD_Comm = CMD_STOPCTL;
	//      }
	//  }
	//g_HT_AxisCard_Oil.inputs.ControlWord.bit.b5Enabled = dbValue.wData;
	//g_HT_AxisCard_Oil.inputs.ControlWord.bit.b8_10XPLCMD = ((pOperateStateBlock->wHTEcatControlWord & 0x300) >> 8); // Anders 2016-6-2, Add.

	// Anders 2016-10-19, Modify for HT.
	if (g_wHTJ6CardType)
	{
		g_HT_AxisCard_Oil.inputs.ControlWord.all = pOperateStateBlock->wHTJ6HYBControlWord; // Anders 2017-4-12, modify.
		//g_HT_AxisCard_Oil.inputs.ControlWord.bit.b5Enabled = dbValue.wData;
		//g_HT_AxisCard_Oil.inputs.wMotionCMD1 = pOperateStateBlock->wHTJ6HYBMotionCMD; // Anders 2016-11-04, add. 2017-10-21, mark for new 5528 protocal.

		g_HT_AxisCard_EleInj.inputs.wControlWord = pOperateStateBlock->wHTJ61EleControlWord; // Anders 2017-4-14, add.
		g_HT_AxisCard_EleInj.inputs.wMotionCMD   = pOperateStateBlock->wHTJ61EleMotionCMD;

		if (wLength > 0x5C) // Anders 2017-10-21, add for new 5528 protocal.
		{
			g_HT_AxisCard_Oil.inputs.wMotionCMD1 = pOperateStateBlock->wHTJ6HYBMotionCMD1;
			g_HT_AxisCard_Oil.inputs.wMotionCMD2 = pOperateStateBlock->wHTJ6HYBMotionCMD2;
			g_HT_AxisCard_Oil.inputs.wMotionCMD3 = pOperateStateBlock->wHTJ6HYBMotionCMD3;
			g_HT_AxisCard_Oil.inputs.wMotionCMD4 = pOperateStateBlock->wHTJ6HYBMotionCMD4;
		}

		if (HT_J6TYPE_PP == g_wHTJ6CardType)//SHANLL 20191223 ADD
		{
			memcpy(&g_HT_AxisCard_PP[0].inputs, &pJ6PPOperateStateBlock->awHTJ6PP[0], sizeof(HTAXISCARD_PP_INPUTS));
			memcpy(&g_HT_AxisCard_PP[1].inputs, &pJ6PPOperateStateBlock->awHTJ6PP[1], sizeof(HTAXISCARD_PP_INPUTS));
			memcpy(&g_HT_AxisCard_PP[2].inputs, &pJ6PPOperateStateBlock->awHTJ6PP[2], sizeof(HTAXISCARD_PP_INPUTS));

			if (g_wPackerCount > 0)
			{
				g_sDB.PARKER_COMPAX3_FLUID1.CONTRLWORD  = pJ6PPOperateStateBlock->wHTParker[0].PARKER_ControlWord_1;
				g_sDB.PARKER_COMPAX3_FLUID1.CONTRLWORD2 = pJ6PPOperateStateBlock->wHTParker[0].PARKER_ControlWord_2;
				g_sDB.PARKER_COMPAX3_FLUID2.CONTRLWORD  = pJ6PPOperateStateBlock->wHTParker[1].PARKER_ControlWord_1;
				g_sDB.PARKER_COMPAX3_FLUID2.CONTRLWORD2 = pJ6PPOperateStateBlock->wHTParker[1].PARKER_ControlWord_2;
			}
		}
	}

	switch (pOperateStateBlock->wDataType) // hankin 20180306 modify.
	{
		case 0:
			break;
		case 1:
			switch (g_wHTJ6CardType)
			{
				case HT_J6TYPE_OIL_ELE:
					memcpy(&g_HT_AxisCard_EleInj.inputs.wRsvd1, &pOperateStateBlock->uData.awHTModule, 11 * sizeof(WORD));
					break;
				case HT_J6TYPE_PLUS:
					memcpy(&g_HT_AxisCard_Plus[0].inputs, &pOperateStateBlock->uData.awHTJ6Plus[0], sizeof(HTAXISCARD_PLUS_INPUTS));
					memcpy(&g_HT_AxisCard_Plus[1].inputs, &pOperateStateBlock->uData.awHTJ6Plus[1], sizeof(HTAXISCARD_PLUS_INPUTS));
					break;
				default:
					break;
			}
			break;
		case 2:
			if (HT_J6TYPE_PLUS == g_wHTJ6CardType)
			{
				memcpy(&g_HT_AxisCard_Plus[0].inputs, &pOperateStateBlock->uData.awHTJ6Plus[0], sizeof(HTAXISCARD_PLUS_INPUTS));
			}
			if (g_wPackerCount > 0)
			{
				g_sDB.PARKER_COMPAX3_FLUID1.CONTRLWORD  = pOperateStateBlock->uData.awTMRsvd[16];
				g_sDB.PARKER_COMPAX3_FLUID1.CONTRLWORD2 = pOperateStateBlock->uData.awTMRsvd[17];
				g_sDB.PARKER_COMPAX3_FLUID2.CONTRLWORD  = pOperateStateBlock->uData.awTMRsvd[24];
				g_sDB.PARKER_COMPAX3_FLUID2.CONTRLWORD2 = pOperateStateBlock->uData.awTMRsvd[25];
			}
			break;
		case 3:    //SHANLL 20200323 ADD
			switch (g_wHTJ6CardType)
			{
				case HT_J6TYPE_OIL_ELE:
					memcpy(&g_HT_AxisCard_EleInj.inputs.wRsvd1, &pOperateStateBlock->uData.awHTModule, 11 * sizeof(WORD));
					break;
				case HT_J6TYPE_PLUS:
					memcpy(&g_HT_AxisCard_Plus[0].inputs, &pOperateStateBlock->uData.awHTJ6Plus[0], sizeof(HTAXISCARD_PLUS_INPUTS));
					break;
				default:
					break;
			}
			if (u_wDriverInovanceCount > 0)
			{
				memcpy(&g_HT_MB_Driver_Inovance.wTargetID, &pOperateStateBlock->uData.awHTModule[16], 6 * sizeof(WORD));
				if(g_HT_MB_Driver_Inovance.wCommCount != g_HT_MB_Driver_Inovance.wLastCommCount)
				{
					g_HT_MB_Driver_Inovance.wInovanceIndex = pOperateStateBlock->uData.awHTModule[16] - 0x321;
					g_HT_MB_Driver_Inovance.bSendEnable = TRUE;
					g_HT_MB_Driver_Inovance.wLastCommCount = g_HT_MB_Driver_Inovance.wCommCount;
				}
			}
			break;
		case 5:    //SHANLL 20200927 ADD
			switch (g_wHTJ6CardType)
			{
				case HT_J6TYPE_OIL_ELE:
					memcpy(&g_HT_AxisCard_EleInj.inputs.wRsvd1, &pOperateStateBlock->uData.awHTModule, 11 * sizeof(WORD));
					break;
				case HT_J6TYPE_PLUS:
					memcpy(&g_HT_AxisCard_Plus[0].inputs, &pOperateStateBlock->uData.awHTJ6Plus[0], sizeof(HTAXISCARD_PLUS_INPUTS));
					break;
				default:
					break;
			}
			if (u_wDriverXingtaiCount > 0)
			{
				if(g_OperateStep1last.all != g_OperateStep1.all)
				{
					g_XT_SendEnable.bControlSendEnable = 1;
					if(g_OperateStep1.all == 0x8)
						bSend_InjState_Xingtai = TRUE;
					else
						bSend_InjState_Xingtai = FALSE;
					if(g_OperateStep1.all != FALSE)
					{
						g_XT_SendEnable.bControlSendState = TRUE;
					}
					else
					{
						g_XT_SendEnable.bControlSendState = FALSE;
					}
					g_OperateStep1last.all = g_OperateStep1.all;
				}
				if(FALSE == g_XT_MB_Driver.bSendEnable)
				{
					memcpy(&g_XT_MB_Driver.wTargetID, &pOperateStateBlock->uData.awHTModule[16], 6 * sizeof(WORD));
					if(g_XT_MB_Driver.wCommCount != g_XT_MB_Driver.wLastCommCount)
					{
						g_XT_MB_Driver.bSendEnable = TRUE;
						g_XT_MB_Driver.wLastCommCount = g_XT_MB_Driver.wCommCount;
					}
					g_XT_MB_Driver.wXingtaiIndex = pOperateStateBlock->uData.awHTModule[16] - 0x321;
				}
				Press_Xingtai = pOperateStateBlock->PressFlowScale[0].wPressScale;
				Flow_Xingtai = pOperateStateBlock->PressFlowScale[0].wFlowScale;
			}
			break;
		case 4:    //SHANLL 20210104 ADD
			switch (g_wHTJ6CardType)
			{
				case HT_J6TYPE_OIL_ELE:
					memcpy(&g_HT_AxisCard_EleInj.inputs.wRsvd1, &pOperateStateBlock->uData.awHTModule, 11 * sizeof(WORD));
					break;
				case HT_J6TYPE_PLUS:
					memcpy(&g_HT_AxisCard_Plus[0].inputs, &pOperateStateBlock->uData.awHTJ6Plus[0], sizeof(HTAXISCARD_PLUS_INPUTS));
					break;
				default:
					break;
			}
			if (u_wRexrothCount > 0)
			{
				g_sDB.REXROTH.HYD_CONTROLWORD			= pOperateStateBlock->uData.awTMRsvd[16];
				g_sDB.REXROTH.INJECTION_CONTROLWORD		= pOperateStateBlock->uData.awTMRsvd[17];
				g_sDB.REXROTH.RESERVE					= pOperateStateBlock->uData.awTMRsvd[18];
				g_sDB.REXROTH.CMD_4WRVE_NG16			= pOperateStateBlock->uData.awTMRsvd[19];
				g_sDB.REXROTH.CMD_2WRCE_NG40			= pOperateStateBlock->uData.awTMRsvd[20];
				g_sDB.REXROTH.CMD_2WRCE_NG32			= pOperateStateBlock->uData.awTMRsvd[21];
				g_sDB.REXROTH.PUMP_ACTUAL_PRESS			= pOperateStateBlock->uData.awTMRsvd[22];
				g_sDB.REXROTH.ACU_ACTUAL_PRESS			= pOperateStateBlock->uData.awTMRsvd[23];
			}
			break;
		default:
			break;
	}
	//shanll 20210408 add
	if(u_wHTRobotCount>0)
	{
		g_wMBEjetPosition = (WORD)pOperateStateBlock->dwEject1Posi/10;
		g_wMBMoldPosition = (WORD)pOperateStateBlock->dwMold1Posi/10;
		g_wMBOpenendPosition = g_sDB.M28.CONFIG.MOLD_OPEN_END_POSITION_SET;
		if(pOperateStateBlock->OperateStep1.bit.bOS_OPNM == 1)
			g_byMBOpenmoldState.bit.bOpenActing = TRUE;
		else
			g_byMBOpenmoldState.bit.bOpenActing = FALSE;

		if((pOperateStateBlock->OperateStep1.bit.bOS_EJTF == 1)||(pOperateStateBlock->OperateStep1.bit.bOS_EJTB == 1))
			g_byMBEjetState.bit.bEjetActing = TRUE;
		else
			g_byMBEjetState.bit.bEjetActing = FALSE;
	}
	// Heat State.
	if (GetMoldsetState()) //  Anders 2015-4-15, Add.
	{
		dbValue.vWORD = (0xFFFF == pOperateStateBlock->AdditionalOP.wHeatState) ? 1 : 0;
		if (pOperateStateBlock->AdditionalOP.OtherState.b0MoldTemperHeatSwitch)
			dbValue.vWORD |= 2;
		if (wHeatFlag != dbValue.vWORD) //if (ReadDB_WORD(M3_TEMPER_HEATCMD) != dbValue.vWORD)
		{
			SetDBValueByIndex(DBINDEX_M28_TEMPERATURE_HEAT_CMD, dbValue);
			wHeatFlag = dbValue.vWORD;
		}
		// Pipe Half Temperature State, 20141024 THJ, Add.
		dbValue.vWORD = pOperateStateBlock->AdditionalOP.OtherState.b1PipeHalfTemperSwitch;
		if (dbValue.vWORD != wPipeHalfTemperSwitch)
		{
			SetDBValueByIndex(DBINDEX_M28_TEMPERATURE_PIPE_PRESERVE_USE, dbValue);
			wPipeHalfTemperSwitch = dbValue.vWORD;
		}
		// Mold Half Temperature State, 20141024 THJ, Add.
		dbValue.vWORD = pOperateStateBlock->AdditionalOP.OtherState.b2MoldHalfTemperSwitch;
		if (dbValue.vWORD != wMoldHalfTemperSwitch)
		{
			SetDBValueByIndex(DBINDEX_M28_TEMPERATURE_MOLD_PRESERVE_USE, dbValue);
			wMoldHalfTemperSwitch = dbValue.vWORD;
		}
	}

	//SendRespCMDResult(nSessionHandle,pData,RXSUCCESS);        // Test Mark. Anders 2014-6-24.

	if (g_wManualKey_ClrErrFlag == 1) // Anders 2015-11-11, Add.
	{
		if (CheckTimerOut(&g_lManualKey_ClrErrCount))
		{
			g_wManualKey_ClrErrFlag = 0;
			for (i = 0; i < 8; i++)
				DB_SetBYTE(DBINDEX_MT_HUB_8A_WORK_AREA_STATE_CHANNEL1 + i, 0);
			RefreshMT_HUB_Error();
		}
	}
}
//==================================================================================================
void GetRetransmitData(int nSessionHandle, WORD* pData, WORD wLength)
{
	IMETMHEADER* pHeader = (IMETMHEADER*)(pData);

#ifdef TEST_NET
	g_wTest_HMIRetransmitDataCount++;
#endif

	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS); // Test for Kevin.

	// Anders 2014-4-14, add.
	++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXRETRANSMITCOUNT;
	if (g_wMasterReady) //Anders 2014-4-14, mark.
	{
		if(!g_bHMIDataParsed)
		{
			memcpy(&u_TempParseBuffer, &pHeader->wDataLen + 1, sizeof(u_TempParseBuffer));
			wParseBufferLength = (pHeader->wDataLen << 1);
			g_bHMIDataParsed = true;
		}
		//ResetStopCheck();
//SHANLL move to MainloopParseHMIData() 20200512
//		ResetBoot();
//		++g_lHmiTransmitData;
//		ParseNetData_FromHMI(nSessionHandle, &pHeader->wDataLen + 1, pHeader->wDataLen << 1); // HMI-DSP28 Use Bytes.
	}
}
//SHANLL Add 20200512
void MainloopParseHMIData()
{
	if (g_wMasterReady&&g_bHMIDataParsed) //shanll add 20200509
	{
		ResetBoot();
		++g_lHmiTransmitData;
		ParseNetData_FromHMI(g_nDsp54NetSession, (WORD*)&u_TempParseBuffer, wParseBufferLength); // HMI-DSP28 Use Bytes.
		g_bHMIDataParsed = false;
		memset(&u_TempParseBuffer,0, sizeof(u_TempParseBuffer));
	}
}
void GetRetransmitDataLess(int nSessionHandle, WORD* pData, WORD wLength)
{
	GetRetransmitData(nSessionHandle, pData, wLength);
}
//==================================================================================================
void GetEcho_Boot(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bBoot = 1;
}
//==================================================================================================
void GetEcho_AD_DA(int nSessionHandle, WORD* pData, WORD wLength)
{
	//IMETMHEADER   *pHeader = (IMETMHEADER *)(pData);
	//if(g_ActDataType.all == pHeader->wID)
	//{
	//  GivebackToken_ActData();
	//}
}
//==================================================================================================
void GetEcho_RetransmitData(int nSessionHandle, WORD* pData, WORD wLength)
{
	//static    DWORD       dwRecvTick  =   0;
	//IMETMHEADER   *pHeader = (IMETMHEADER *)(pData);
	//if(g_RetransmitType.all == pHeader->wID)
	//{
	//  if(u_RetransmitToken.wOwner >= BASEADDR_TRISMITEDONCEVALUE)
	//      ResetSendOnceDataFlag();
	//  GivebackToken_Retransmit();
	//
	//  if ( dwRecvTick != 0 &&
	//       u_lRecvInterval < g_dwSystemTick - dwRecvTick )
	//  {
	//      u_lRecvInterval =   g_dwSystemTick - dwRecvTick;
	//  }
	//
	//  dwRecvTick  =   g_dwSystemTick;
	//  g_lRecvRetransmitData++;
	//}
}
void GetEcho_RetransmitDataLess(int nSessionHandle, WORD* pData, WORD wLength)
{
	GetEcho_RetransmitData(nSessionHandle, pData, wLength);
}

void GetEcho_SlowActual(int nSessionHandle, WORD* pData, WORD wLength)
{
}
//==================================================================================================
void GetConfiguration(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bConfiguration = 1;

	//if(WriteDBValue(CONFIGURATIONOFFSET,pData,wLength))
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	//else
	//  SendRespCMDResult(nSessionHandle,pData,RXFAIL);*/

	ResetBoot();
}
//==================================================================================================
void GetMachineParameters1(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bMachineParameters1 = 1;

	//if(WriteDBValue(MACHINE1OFFSET,pData,wLength))
	//  SendRespCMDResult(nSessionHandle,pData,RXSUCCESS);
	//else
	//  SendRespCMDResult(nSessionHandle,pData,RXFAIL);

	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	ResetBoot();
}
//==================================================================================================
void GetMachineParameters2(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bMachineParameters2 = 1;

	//if(WriteDBValue(MACHINE2OFFSET,pData,wLength))
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	//else
	//  SendRespCMDResult(nSessionHandle,pData,RXFAIL);
	ResetBoot();
}
//==================================================================================================
void GetMoldSet1(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bMoldset1 = 1;

	//if(WriteDBValue(MOLDSET1OFFSET,pData,wLength))
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	//else
	//  SendRespCMDResult(nSessionHandle,pData,RXFAIL);
	ResetBoot();
}
//==================================================================================================
void GetMoldSet2(int nSessionHandle, WORD* pData, WORD wLength)
{

	g_MasterComState.bit.bMoldset2 = 1;

	//if(WriteDBValue(MOLDSET2OFFSET,pData,wLength))    //1==1;
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	//else
	//  SendRespCMDResult(nSessionHandle,pData,RXFAIL);
	ResetBoot();
}
//==================================================================================================
void GetMachineParameters_B(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bMachineParameters_B = 1;
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	ResetBoot();
}
//==================================================================================================
void GetMoldSet_B1(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bMoldset_B1 = 1;
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	ResetBoot();
}
//==================================================================================================
void GetMoldSet_B2(int nSessionHandle, WORD* pData, WORD wLength)
{
	g_MasterComState.bit.bMoldset_B2 = 1;
	SendRespCMDResult(nSessionHandle, pData, RXSUCCESS);
	ResetBoot();
}
//==================================================================================================
void SendTestData(int nSessionHandle, WORD wTest, WORD wLen)
{
#ifdef TEST_NET
	g_wTest_SendTestDataCount++;
#endif

	memset(&u_TempTxBuffer, wTest, sizeof(IMETEMPBUFFER));
	SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, wLen);
}
//==================================================================================================
void SendBootData(int nSessionHandle)
{
	WORD         i;
	WORD*        pdata     = (WORD*)&u_TempTxBuffer + NETHEADERLEN;
	IMETMHEADER* pTMHeader = &u_TempTxBuffer.TMHeader;
#ifdef TEST_NET
	g_wTest_SendBootCount++;
#endif
	g_MasterComState.all       = 0;
	g_MasterComState.bit.bBoot = 1;
	memset(&u_TempTxBuffer, 0, sizeof(IMETEMPBUFFER));
	pTMHeader->wSTX = STX;
	// hankin 20190604 modify.
	++g_ActDataType.bit.bCounter;
	pTMHeader->Type     = g_ActDataType;
	pTMHeader->wCMD     = CMD_90;
	pTMHeader->wID      = 0;
	pTMHeader->wDataLen = BOOTDATALEN;
	pTMHeader->wSize    = BOOTDATALEN + NETNODATASIZE;

	for (i = 0; i < sizeof(MOTIONCONFIG_TABLE) / sizeof(*MOTIONCONFIG_TABLE); i++)
		pdata[i] = MOTIONCONFIG_TABLE[i].wConfig;

	//CRC
	*(pdata + pTMHeader->wDataLen)     = MakeCRCValue(&pTMHeader->wSize, pTMHeader->wSize - NOCRCSIZE);
	*(pdata + pTMHeader->wDataLen + 1) = ETX;

	SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, pTMHeader->wSize << 1);

	//MC_DBData[CARD_INTERFACE_NET_DWRXRETRANSMITCOUNT].dbValue.dwData++;
}
//==================================================================================================
void Send91_Fast(int nSessionHandle) //void   SendAD_DA(int nSessionHandle, WORD  wMotionID)
{
	Fast91_T* psFast91 = (Fast91_T*)u_TempTxBuffer.awData;
	WORD      i;
	DWORD     dwData;

	//WORD wRulerIndex, wInputIndex, wADIndex;
	//COMPUTE_OUTPUT    *pComputeOut = &g_AxisController[0].InheritMAController.CtlData.ComputeOutputData[0];

	memset(&u_TempTxBuffer, 0, ADDADATA_TOTALLEN);
	g_ActDataType.bit.bCounter++;
	u_TempTxBuffer.TMHeader.wSTX     = STX;
	u_TempTxBuffer.TMHeader.Type     = g_ActDataType;
	u_TempTxBuffer.TMHeader.wCMD     = CMD_91;
	u_TempTxBuffer.TMHeader.wID      = 0;
	u_TempTxBuffer.TMHeader.wDataLen = ADDADATALEN;
	u_TempTxBuffer.TMHeader.wSize    = ADDADATALEN + NETNODATASIZE;

	// Anders 2015-7-17, Add.
	if (g_wManualKey_ClrErrFlag == 1)
		ClearDBErr();

	for (i = 0; i < PACK91DATA_MOTIONAXIS_NUM; ++i)
	{
		DeviceError error;
		// DataType
		psFast91->sMotionAxis[i].wDataType = MOTIONCONFIG_TABLE[i].wConfig;
		// if ((ACTAXIS_MOLD != i) && (g_AxisManager[i].MotionConfig.wAll == 0)) continue;
		if (!GetMoldsetState())
			continue;
		// Error.
		//if ( g_AxisManager[i].Error.dwAll == 0 && i < 10 )    // Anders 2016-5-10 mark.
		//    g_AxisManager[i].Error.dwAll = ReadDB_DWORD(cnst_awAxisIndex[i].wError_Idx);
		error = Get_MasterError((ACTAXIS)i);
		if (0 != error.dwAll)
		{
			psFast91->sMotionAxis[i].wError = (error.info.bErrorType == ERRORTYPE_ALARM);
			//g_AxisManager[i].Error.info.bSubIndex = cnst_awMotionAxis2DataAxis[i]; Anders 2016-4-14, Mark.
			memcpy(&psFast91->sMotionAxis[i].wAlarmL, &error, sizeof(DWORD));
			memcpy(&psFast91->sMotionAxis[i].wAlarmWarningL, &error, sizeof(DWORD)); // hankin 20180529 add: for Error.
		}

		psFast91->sMotionAxis[i].Status.done.bDoneFlag = 0;
		// psFast91->sMotionAxis[i].wActCmd = g_AxisManager[i].Data.AxisCtlState.Info.wCMD; // Anders 2016-3-27, add. // hankin 20180529 mask: for Error.

		//g_HTJ6CardConfig.all = 1;
		//g_HT_AxisCard_EleInj.outputs.wStatusWord  =   1;
		//g_HT_AxisCard_EleInj.outputs.wInjActPosiL =   2;
		//g_HT_AxisCard_EleInj.outputs.wInjActPosiH =   3;
		//g_HT_AxisCard_EleInj.outputs.wInjActSpeedL    =   4;
		//g_HT_AxisCard_EleInj.outputs.wInjActSpeedH    =   5;
		//g_HT_AxisCard_EleInj.outputs.wInjActTorque    =   6;
		//g_HT_AxisCard_EleInj.outputs.wInjActPress =   7;
		//g_HT_AxisCard_EleInj.outputs.wInjProfilePosiL =   8;
		//g_HT_AxisCard_EleInj.outputs.wInjProfilePosiH =   9;
		//g_HT_AxisCard_EleInj.outputs.wInjPosiErrL =   10;
		//g_HT_AxisCard_EleInj.outputs.wInjPosiErrH =   11;
		//g_HT_AxisCard_EleInj.outputs.wInjProfileVeloL =   12;
		//g_HT_AxisCard_EleInj.outputs.wInjProfileVeloH =   13;
		//g_HT_AxisCard_EleInj.outputs.wMotionTimeL =   14;
		//g_HT_AxisCard_EleInj.outputs.wMotionTimeH =   15;
		//g_HT_AxisCard_EleInj.outputs.wChgTorque   =   16;
		//g_HT_AxisCard_EleInj.outputs.wMotionState =   17;

		if ((g_wPackerCount > 0)&&(HT_J6TYPE_PP != g_wHTJ6CardType))
		{
			WORD* pwData;
			WORD  wIndex;

			wIndex = 0;
			switch (i)
			{
				case ACTAXIS_NOZZLE2:
					++wIndex;
				case ACTAXIS_NOZZLE:
					psFast91->sMotionAxis[i].Status.done.bDataType = 2;

					pwData    = (WORD*)&psFast91->sMotionAxis[i];
					pwData[5] = DB_GetWORD(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_STATUSWORD);
					pwData[6] = DB_GetWORD(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_STATUSWORD2);
					memcpy(pwData + 7, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_SPEED_CLAMP), sizeof(LONG));
					memcpy(pwData + 9, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_POSITION_CLAMP), sizeof(LONG));
					memcpy(pwData + 11, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_SPEED_INJECTION), sizeof(LONG));
					memcpy(pwData + 13, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_POSITION_INJECTION), sizeof(LONG));
					memcpy(pwData + 15, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_PRESSURE_PA), sizeof(LONG));
					memcpy(pwData + 17, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_PRESSURE_PB), sizeof(LONG));
					pwData[19] = DB_GetWORD(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wIndex + DBINDEX_PARKER_COMPAX3_FLUID1_ERROR_HISTORY_LAST_ERROR);
					break;
				default:
					break;
			}
		}
		if (HT_J6TYPE_NULL != g_wHTJ6CardType) // hankin 20180305 modify.
		{
			//psFast91->sMotionAxis[i].Status.all = (1 << 14);
			psFast91->sMotionAxis[i].wAlarmL = 0; // Add for HT AxisCard.
			psFast91->sMotionAxis[i].wAlarmH = 0; // Add for HT AxisCard.
			switch (g_wHTJ6CardType)
			{
				case HT_J6TYPE_OIL_ELE:
					if (i != ACTAXIS_INJECT)
						break;
					psFast91->sMotionAxis[i].Status.done.bDataType = 64;
					memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_EleInj.outputs, sizeof(g_HT_AxisCard_EleInj.outputs));
					break;
				case HT_J6TYPE_PLUS:
				{
					WORD* pOutput;
					WORD  wIndex;

					wIndex = 0;
					switch (i)
					{
						case ACTAXIS_INJECT2:
							++wIndex;
						case ACTAXIS_INJECT:
							psFast91->sMotionAxis[i].Status.done.bDataType = 1;

							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_Plus[wIndex].outputs, (32 - 5) * sizeof(WORD));
							break;
						case ACTAXIS_RECOVERY2:
							++wIndex;
						case ACTAXIS_RECOVERY:
							psFast91->sMotionAxis[i].Status.done.bDataType = 1;

							pOutput = (WORD*)&g_HT_AxisCard_Plus[wIndex].outputs;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &pOutput[32 - 5], (32 - 5) * sizeof(WORD));
							break;
						default:
							break;
					}
					break;
				}
				case HT_J6TYPE_PP:
				{
					WORD* pwData;
					WORD  wJ6PP_Index = 0;
					WORD  wParker_Index = 0;
					switch (i)
					{
						case ACTAXIS_MOLD:
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_PP[wJ6PP_Index].outputs, (2+24) * sizeof(WORD));
							memset(&psFast91->sMotionAxis[i].awRsvd, 0, sizeof(WORD));
							break;
						case ACTAXIS_MOLD2:
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_PP[wJ6PP_Index].outputs.wJ6PP_Rod1_PosiL, 24 * sizeof(WORD));
							memset(&psFast91->sMotionAxis[i].wPosiCountL, 0, 3*sizeof(WORD));
							break;
						case ACTAXIS_EJECT:
							wJ6PP_Index = 1;
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_PP[wJ6PP_Index].outputs, (2+24) * sizeof(WORD));
							memset(&psFast91->sMotionAxis[i].awRsvd, 0, sizeof(WORD));
							break;
						case ACTAXIS_EJECT2:
							wJ6PP_Index = 1;
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_PP[wJ6PP_Index].outputs.wJ6PP_Rod1_PosiL, 24 * sizeof(WORD));
							memset(&psFast91->sMotionAxis[i].wPosiCountL, 0, 3*sizeof(WORD));
							break;
						case ACTAXIS_INJECT:
							wJ6PP_Index = 2;
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_PP[wJ6PP_Index].outputs, (2+24) * sizeof(WORD));
							memset(&psFast91->sMotionAxis[i].awRsvd, 0, sizeof(WORD));
							break;
						case ACTAXIS_INJECT2:
							wJ6PP_Index = 2;
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_AxisCard_PP[wJ6PP_Index].outputs.wJ6PP_Rod1_PosiL, 24 * sizeof(WORD));
							memset(&psFast91->sMotionAxis[i].wPosiCountL, 0, 3*sizeof(WORD));
							break;
						case ACTAXIS_RECOVERY:
						case ACTAXIS_RECOVERY2:
							if(ACTAXIS_RECOVERY2 == i)
								++wParker_Index;
							psFast91->sMotionAxis[i].Status.done.bDataType = 4;
							pwData    = (WORD*)&psFast91->sMotionAxis[i];
							pwData[5] = DB_GetWORD(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_STATUSWORD);
							pwData[6] = DB_GetWORD(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_STATUSWORD2);
							memcpy(pwData + 7, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_SPEED_CLAMP), sizeof(LONG));
							memcpy(pwData + 9, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_POSITION_CLAMP), sizeof(LONG));
							memcpy(pwData + 11, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_SPEED_INJECTION), sizeof(LONG));
							memcpy(pwData + 13, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_POSITION_INJECTION), sizeof(LONG));
							memcpy(pwData + 15, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_PRESSURE_PA), sizeof(LONG));
							memcpy(pwData + 17, DB_GetDataAddr(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ACTUAL_PRESSURE_PB), sizeof(LONG));
							pwData[19] = DB_GetWORD(DBGROUPINDEX_PARKER_COMPAX3_FLUID_SUM * wParker_Index + DBINDEX_PARKER_COMPAX3_FLUID1_ERROR_HISTORY_LAST_ERROR);
							memset(pwData + 20, 0, 12*sizeof(WORD));
							break;
						default:
							break;
					}
					break;
				}
				default:
					break;
			}
		}

		// Anders 2016-3-27, add.
		switch (i)
		{
			case ACTAXIS_ROTARY:
			case ACTAXIS_ROTARY2:
				psFast91->sMotionAxis[i].awRsvd = CommComp_turntable_get_awRsvd((enum CommComp_turntable_e_index)(i - ACTAXIS_ROTARY));
				psFast91->sMotionAxis[i].wError = CommComp_turntable_get_error((enum CommComp_turntable_e_index)(i - ACTAXIS_ROTARY));
				dwData                          = CommComp_turntable_get_act_position_1((enum CommComp_turntable_e_index)(i - ACTAXIS_ROTARY));
				memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &dwData, sizeof(DWORD));
				psFast91->sMotionAxis[i].wActSpd1   = CommComp_turntable_get_act_speed_1((enum CommComp_turntable_e_index)(i - ACTAXIS_ROTARY));
				psFast91->sMotionAxis[i].wActForce1 = CommComp_turntable_get_act_voltag((enum CommComp_turntable_e_index)(i - ACTAXIS_ROTARY));
				break;
			default:
				break;
		}

		//SHANLL_20200324 ADD
		if(u_wDriverInovanceCount > 0)
		{
			switch (i)
			{
				case ACTAXIS_NOZZLE:
//					if(g_HT_91_Driver_Inovance.wCommCount == g_HT_MB_Driver_Inovance.wCommCount)
//					{
						psFast91->sMotionAxis[i].Status.done.bDataType = 5;
						memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_HT_91_Driver_Inovance, sizeof(g_HT_91_Driver_Inovance));
//					}
						break;
				default:
					break;
			}
		}
		//SHANLL_20200927 ADD
		if(u_wDriverXingtaiCount > 0)
		{
			switch (i)
			{
				case ACTAXIS_NOZZLE:
//					if(g_XT_91_Driver.wCommCount == g_XT_MB_Driver.wCommCount)
//					{
						psFast91->sMotionAxis[i].Status.done.bDataType = 7;
						memcpy(&psFast91->sMotionAxis[i].wActPosi1L, &g_XT_91_Driver, 6*sizeof(WORD));
//					}
						break;
				default:
					break;
			}
		}
		//shanll 20201228 add
		if(u_wRexrothCount > 0)
		{
			switch (i)
			{
				case ACTAXIS_NOZZLE:;
					WORD* pwData;
					pwData = (WORD*)&psFast91->sMotionAxis[i];
					psFast91->sMotionAxis[i].Status.done.bDataType = 6;
					pwData[16] = g_sDB.REXROTH.HYD_STATUSWORD;
					memcpy(&pwData[17], &g_sDB.REXROTH.ACTUAL_POSITION, sizeof(DWORD));
					memcpy(&pwData[19], &g_sDB.REXROTH.ACTUAL_VELOCITY, sizeof(DWORD));
					pwData[21] = g_sDB.REXROTH.ACTUAL_PRESS_P11;
					pwData[22] = g_sDB.REXROTH.ACTUAL_PRESS_P12;
					pwData[23] = g_sDB.REXROTH.INJECTION_STATUSWORD;
					pwData[24] = g_sDB.REXROTH.PLASFICATION_PRESS;
					memcpy(&pwData[25], &g_sDB.REXROTH.ERROR_CODE, sizeof(DWORD));
					break;
				default:
					break;
			}
		}
	} // end_for

	// MC_WRITEDB_DWORD(AXIS1_INTERFACE_ERROR11, (psFast91->sMotionAxis[ACTAXIS_MOLD].wActPosi2L | psFast91->sMotionAxis[ACTAXIS_MOLD].wActPosi2H << 16));

	//RemoteIO + CANRuler
	if (g_sDB.M28.MONITOR.VERSION_OF_MASTER >= 3)
	{
		UpdateLinkState_RemoteIO();
	}

		//shanll 20200729 add
	if(ALSTATE_OP == g_ECController.Slave[g_wMasterErrorScan].ALStatus.bit.bState)
		u_wECATSlaveOPLost = 0;
	psFast91->sMotionAxis[0].Status.all |= u_wECATSlaveOPLost;

	psFast91->sRemoteIO.wDataType  = (MOTIONCMDTYPE_ONEMOTION << 8) | MOTIONID_REMOTEIO;
	psFast91->sRemoteIO.wLinkState = g_sDB.M28.MONITOR.REMOTEIO_CARD_LINK_STATE;
	for (i = 0; i < PACK91DATA_REMOTEIO_INPUT_NUM; ++i)
		psFast91->sRemoteIO.awInput[i] = GetInput_RemoteIO(i);
	for (i = 0; i < PACK91DATA_REMOTEIO_AD_NUM; ++i)
		psFast91->sRemoteIO.awAD[i] = GetAD_RemoteIO(i);

	for (i = 0; i < PACK91DATA_CANRULER_NUM; ++i)
	{
		psFast91->sRemoteIO.adwCANPosi[i] = GetPositionCount_RemoteIO(i);

		dwData = psFast91->sRemoteIO.adwCANPosi[i];
		DB_SetLONG(DBINDEX_M28_MONITOR_REMOTE_DEVICE_RULER_POSION_COUNT0 + i, dwData);
	}

	//MotionDA
	psFast91->sM28DA.wDataType = (MOTIONCMDTYPE_ONEMOTION << 8) | MOTIONID_M28DA;

	if (g_wHTJ6CardType)
	{
		psFast91->sM28DA.wDataGroup = 1; // HT Data.
		memcpy(&psFast91->sM28DA.wEncoder, &g_HT_AxisCard_Oil.outputs, sizeof(g_HT_AxisCard_Oil.outputs));
	}

	//CRC
	u_TempTxBuffer.awData[u_TempTxBuffer.TMHeader.wDataLen]     = MakeCRCValue(&u_TempTxBuffer.TMHeader.wSize, u_TempTxBuffer.TMHeader.wSize - NOCRCSIZE);
	u_TempTxBuffer.awData[u_TempTxBuffer.TMHeader.wDataLen + 1] = ETX;
	SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, u_TempTxBuffer.TMHeader.wSize << 1);

	// Anders 2014-4-14, add.
	++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWTXMASTERACTVALUECOUNT;

	g_XT_91_Driver.bSendTo55End = true;//shanll 20200928 add
#ifdef TEST_NET
	memcpy(&g_Fast91, psFast91, sizeof(Fast91_T));
	g_wTest_SendAD_DACount++;
#endif
}
//==================================================================================================
void SendCurveData(int nSessionHandle, WORD wMotionID)
{
}
//==================================================================================================
/*
void SendRetransmitData(int nSessionHandle, WORD* pData, WORD wBytes)
{
	WORD*        pTxData;
	IMETMHEADER* pTMHeader = &u_TempTxBuffer.TMHeader;
#ifdef TEST_NET
	g_wTest_RetransmitDataCount++;
#endif

	if (wBytes > TRANSMITDATABYTES_LEN)
	{
		return; //g_wMasterReady == 0 ||
	}
	memset(&u_TempTxBuffer, 0, (DWORD)TRANSMITDATA_TOTALLEN); // Anders 2017-3-21, mark.
	g_RetransmitType.bit.bCounter++;
	pTxData             = (WORD*)&u_TempTxBuffer + NETHEADERLEN;
	pTMHeader->wSTX     = STX;
	pTMHeader->Type.all = g_RetransmitType.all;
	pTMHeader->wCMD     = CMD_93;
	pTMHeader->wID      = 0;
	pTMHeader->wDataLen = TRANSMITDATALEN;
	pTMHeader->wSize    = pTMHeader->wDataLen + NETNODATASIZE;
	//Data
	memcpy(pTxData, pData, (wBytes >> 1) * sizeof(WORD));

	//CRC
	*(pTxData + pTMHeader->wDataLen)     = MakeCRCValue(&pTMHeader->wSize, pTMHeader->wSize - NOCRCSIZE);
	*(pTxData + pTMHeader->wDataLen + 1) = ETX;

	if (pTMHeader->wSize == SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, pTMHeader->wSize << 1))
		g_lSendRetransmitData++;
	//u_RetransmitToken.lCounter = DEF_INVALIDRETRANSMITTIME;
	//u_RetransmitToken.lInitCounter  =   DEF_INITRETRANSMITTIME;
}
*/
void SendRetransmitData(int nSessionHandle, WORD* pData, WORD wBytes)
{
	IMETMHEADER 	TMHeader;
	WORD			wLength	= NETNODATASIZE + TRANSMITDATALEN;
	WORD			wTail[2];
	Fifo* 			pHMIFifo = &u_HMIFifo;

	if (wBytes > TRANSMITDATABYTES_LEN)
	{
		return; //g_wMasterReady == 0 ||
	}
	if(comp_fifo_s_manager_get_free(&pHMIFifo->manager) >= (wLength+1))
	{
		WORD	index;
		g_RetransmitType.bit.bCounter++;

		TMHeader.wSTX     = STX;
		TMHeader.Type.all = g_RetransmitType.all;
		TMHeader.wCMD     = CMD_93;
		TMHeader.wID      = 0;
		TMHeader.wDataLen = TRANSMITDATALEN;
		TMHeader.wSize    = TMHeader.wDataLen + NETNODATASIZE;

		//insert length info
		index = comp_fifo_s_manager_get_write(&pHMIFifo->manager);
		pHMIFifo->fifo_buffer.fifo_Length = wLength;
		comp_fifo_s_manager_set_write(&pHMIFifo->manager, index);

		//copy header info
		index = comp_fifo_s_manager_get_write(&pHMIFifo->manager);
		memcpy(&pHMIFifo->fifo_buffer.fifo_header, &TMHeader, sizeof(TMHeader)/sizeof(WORD));
		comp_fifo_s_manager_set_write_batch(&pHMIFifo->manager, index,sizeof(TMHeader)/sizeof(WORD));

		//copy data info
		index = comp_fifo_s_manager_get_write(&pHMIFifo->manager);
		memcpy(&pHMIFifo->fifo_buffer.fifo_data, pData, (wBytes >> 1) * sizeof(WORD));
		comp_fifo_s_manager_set_write_batch(&pHMIFifo->manager, index, (wBytes >> 1) * sizeof(WORD));

		//copy rest data 0
		index = comp_fifo_s_manager_get_write(&pHMIFifo->manager);
		memset(&pHMIFifo->fifo_buffer.fifo_data[wBytes >> 1], 0, (TRANSMITDATALEN-(wBytes>>1))*sizeof(WORD));
		comp_fifo_s_manager_set_write_batch(&pHMIFifo->manager, index, (TRANSMITDATALEN-(wBytes>>1))*sizeof(WORD));

		//copy tail info
		wTail[0] = MakeCRCValue(&pHMIFifo->fifo_buffer.fifo_header.wSize, TMHeader.wSize - NOCRCSIZE);
		wTail[1] = ETX;
		index = comp_fifo_s_manager_get_write(&pHMIFifo->manager);
		memcpy(&pHMIFifo->fifo_buffer.fifo_tail, wTail, sizeof(wTail)/sizeof(WORD));
		comp_fifo_s_manager_set_write_batch(&pHMIFifo->manager, index, sizeof(wTail)/sizeof(WORD));
	}
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
void RunTaskHMIFifoRead()
{
    while (comp_fifo_s_manager_get_exist(&u_HMIFifo.manager) > 0)
    {
        WORD    wBuffer[TRANSMITDATALEN+NETNODATASIZE];
		WORD	wLength;
		WORD	wWords;
        WORD    wIndex;

		wIndex = comp_fifo_s_manager_get_read(&u_HMIFifo.manager);
		wWords = u_HMIFifo.fifo_buffer.fifo_Length;
		wLength = wWords + 1;
		if(comp_fifo_s_manager_get_exist(&u_HMIFifo.manager) < wLength)
		{
			break;
		}
		comp_fifo_s_manager_set_read(&u_HMIFifo.manager, wIndex);
		--wLength;

		memset(wBuffer, 0, sizeof(wBuffer));

		wIndex = comp_fifo_s_manager_get_read(&u_HMIFifo.manager);
		memcpy(wBuffer, &u_HMIFifo.fifo_buffer.fifo_header, sizeof(wBuffer));
		comp_fifo_s_manager_set_read_batch(&u_HMIFifo.manager, wIndex, sizeof(wBuffer)/sizeof(WORD));

		SendNetData(g_nDsp54NetSession, wBuffer, (TRANSMITDATALEN+NETNODATASIZE)<<1);
		g_lSendRetransmitData++;
    }
}
//==================================================================================================
void SendRetransmitDataLess(int nSessionHandle, WORD* pData, WORD wBytes)
{
	WORD*        pTxData;
	IMETMHEADER* pTMHeader = &u_TempTxBuffer.TMHeader;
	if (g_wMasterReady == 0 || wBytes > RETRANSMITDATALESSBYTES_LEN)
		return;
	memset(&u_TempTxBuffer, 0, (DWORD)RETRANSMITDATALESS_TOTALLEN);
	g_RetransmitType.bit.bCounter++;
	pTxData             = (WORD*)&u_TempTxBuffer + NETHEADERLEN;
	pTMHeader->wSTX     = STX;
	pTMHeader->Type.all = g_RetransmitType.all;
	pTMHeader->wCMD     = CMD_94;
	pTMHeader->wID      = 0;
	pTMHeader->wDataLen = RETRANSMITDATALESS_LEN;
	pTMHeader->wSize    = pTMHeader->wDataLen + NETNODATASIZE;
	//Data
	memcpy(pTxData, pData, (wBytes >> 1) * sizeof(WORD));

	//CRC
	*(pTxData + pTMHeader->wDataLen)     = MakeCRCValue(&pTMHeader->wSize, pTMHeader->wSize - NOCRCSIZE);
	*(pTxData + pTMHeader->wDataLen + 1) = ETX;

	if (pTMHeader->wSize == SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, pTMHeader->wSize << 1))
		g_lSendRetransmitData++;
	//u_RetransmitToken.lCounter          =   DEF_INVALIDRETRANSMITTIME;
	//u_RetransmitToken.lInitCounter      =   DEF_INITRETRANSMITTIME;

	g_lSendRetransmitData++;
}
//==================================================================================================

void SendRespCMDResult(int nSessionHandle, WORD* pData, WORD wResult)
{
	TRESPCMD* pRespCMD = (TRESPCMD*)&u_TempTxBuffer;

	memcpy(pRespCMD, pData, sizeof(IMETEMPBUFFER));
	pRespCMD->TMHeader.wSize    = 9;
	pRespCMD->TMHeader.wDataLen = 1;
	//pRespCMD->TMHeader.wID      = pRespCMD->TMHeader.Type.all;
	pRespCMD->TData.wData = wResult;
	pRespCMD->TData.wCRC  = MakeCRCValue(&pRespCMD->TMHeader.wSize, pRespCMD->TMHeader.wSize - NOCRCSIZE);
	pRespCMD->wETX        = ETX;

	SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, pRespCMD->TMHeader.wSize << 1);
	//if (pRespCMD->TMHeader.wSize == SendNetData(nSessionHandle,(WORD *)&u_TempTxBuffer, pRespCMD->TMHeader.wSize<<1) )
	//{
	//    if (pRespCMD->TMHeader.wCMD == 0x4D43)
	//       g_wTest_EchoCount++;
	//}
}
//==================================================================================================
WORD MakeCRCValue(WORD* pData, WORD wLength)
{
	int   i;
	WORD* pCRCData  = pData;
	WORD  wCRCValue = *pCRCData++;

	for (i = 1; i < wLength; i++)
		wCRCValue ^= *pCRCData++;
	return wCRCValue;
}
//==================================================================================================
//static  void  StartAutoTx(int nSessionHandle,WORD wTxID)
//{
//  int i;
//
//  for (i = 0; i < (sizeof(u_aAutoRespList) / sizeof(*u_aAutoRespList)); ++i)
//      u_aAutoRespList[i].bFlag        =   TRUE;
//  u_aAutoRespList[i].bCycle       =   SLOW_91_FRIST_SENDCYCLE;
//  for (i = 0; i < (sizeof(u_aAutoRespList) / sizeof(*u_aAutoRespList)); ++i)
//      ResetTimer(&u_aAutoRespList[i].lCounter, u_aAutoRespList[i].bCycle);
//}

//==================================================================================================
static void StopAutoTx(int nSessionHandle, WORD wTxID)
{
	u_sAutoRespList_Fast.bFlag = FALSE;
	u_sAutoRespList_Slow.bFlag = FALSE;
	u_fCommFunc                = COMMSTEP_IDLE;
}
//==================================================================================================
static __inline void ResetBoot()
{
	//g_BootState.lCounter  = RESETBOOTTIME;
	ResetTimer(&g_BootState.lCounter, RESETBOOTTIME);
}
//==================================================================================================
//static    __inline void   ResetStopCheck()
//{
//  //g_NetStopState.lCounter  = STOPCHECKTIME;
//  ResetTimer(&g_NetStopState.lCounter, STOPCHECKTIME);
//}
//==================================================================================================
static __inline void StartTxADDA_OPState(void)
{
	//StartAutoTx(g_nDsp54NetSession,RESP_ADDA);
	u_sAutoRespList_Fast.bFlag = TRUE;
	ResetTimer(&u_sAutoRespList_Fast.dwCounter, u_sAutoRespList_Fast.bCycle); //ResetTimer(&u_sAutoRespList_Fast.lCounter, 0);    // THJ 20150610 Modify:
}

void StartTxTemperDataToMaster(void)
{
	u_sAutoRespList_Slow.bFlag = TRUE;
	ResetTimer(&u_sAutoRespList_Slow.dwCounter, u_sAutoRespList_Slow.bCycle); //ResetTimer(&u_sAutoRespList_Slow.lCounter, SLOW_91_FRIST_SENDCYCLE);  // THJ 20150610 Modify:
}
//==================================================================================================
static __inline void StopTxADDA_OPState()
{
	StopAutoTx(g_nDsp54NetSession, RESP_ADDA);
}
//==================================================================================================
//static    inline void StartTxCurveData()
//{
//  StartAutoTx(g_nDsp54NetSession,RESP_CURVEVALUE);
//}
//-------------------------------------------------------------------------------------------------
//static    inline void StopTxCurveData()
//{
//  StopAutoTx(g_nDsp54NetSession,RESP_CURVEVALUE);
//}
//==================================================================================================

//==================================================================================================
static int ScanOperateModeByBit(WORD wOperateMode)
{
	int i;
	for (i = 15; i >= 0; i--)
	{
		if (wOperateMode & ((WORD)1 << i))
			return i;
	}
	return -1;
}
//==================================================================================================
//BOOL  GetToken_ActData(WORD wOwner)
//{
//    if(u_ActDataToken.wOwner == TOKENID_NULL || (u_ActDataToken.wOwner==wOwner &&     CheckTimerOut(&u_ActDataToken.lCounter)))
//    {
//        u_ActDataToken.wOwner       =   wOwner;
//      //u_ActDataToken.lCounter     =   DEF_INVALIDADDATIME;
//      ResetTimer(&u_ActDataToken.lCounter, DEF_INVALIDADDATIME);
//        return TRUE;
//    }
//    else return FALSE;
//}
//
//void  GivebackToken_ActData()
//{
//  u_ActDataToken.wOwner           =   TOKENID_NULL;
//}

//void  CheckEchoInvalid_ActData()
//{
//  if(u_ActDataToken.wOwner != TOKENID_NULL && CheckTimerOut(&u_ActDataToken.lCounter))
//    {
//        SetQuicklySendAD_DA(u_ActDataToken.wOwner);
//        // u_ActDataToken.lCounter     =   DEF_INVALIDADDATIME;
//    }
//}

//void    CheckEchoInit_RetransmitData()
//{
//    if (u_RetransmitToken.lInitCounter < 0)
//    {
//        u_RetransmitToken.wOwner            =   TOKENID_NULL;
//        u_RetransmitToken.lCounter          =   DEF_INVALIDRETRANSMITTIME;
//        u_RetransmitToken.lInitCounter      =   DEF_INITRETRANSMITTIME;
//    }
//}
//==================================================================================================
//void    SetQuicklySendAD_DA(WORD    wAxis)
//{
//    //u_AutoRespList[wAxis].RespList[RESP_ADDA].lCounter = -1;
//    SetTimerOut(&u_aAutoRespList[0].dwCounter, u_aAutoRespList[0].bCycle);
//}
////==================================================================================================
//BOOL    GetToken_Retransmit(WORD wOwner)
//{
//    //if( u_ActDataToken.wOwner == TOKENID_NULL &&
//  if ((u_RetransmitToken.wOwner == TOKENID_NULL || u_RetransmitToken.wOwner == wOwner) && CheckTimerOut(&u_RetransmitToken.lCounter) )
//    {
//        u_RetransmitToken.wOwner            =   wOwner;
//      u_RetransmitToken.lCounter          =   DEF_INVALIDRETRANSMITTIME;
//      u_RetransmitToken.lInitCounter      =   DEF_INITRETRANSMITTIME;
//        return  TRUE;
//    }
//    else
//      return FALSE;
//}
////==================================================================================================
//void    GivebackToken_Retransmit()
//{
//    u_RetransmitToken.wOwner        =   TOKENID_NULL;
//}
////==================================================================================================
//WORD    GetOwner_Retransmit()
//{
//    return u_RetransmitToken.wOwner;
//}
//==================================================================================================

//==================================================================================================
// No more.
//==================================================================================================
void ParseOperateMotion(TOperateMode OperateMode, WORD wActAxis, WORD wCmd)
{
}

void Command_ManualKey()
{
	//DBVALUE dbValue;
	DWORD dwTemp = 0;

	// Alarm, Stop All MotionAxis. Anders 2016-4-13, Add "if..." for HX IME.
	//if (ReadDB_DWORD(DBINDEX_MOLD0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_MOLD1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_EJECT0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_EJECT1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_INJECT0_INTERFACE_ERROR1)
	//        | ReadDB_DWORD(DBINDEX_INJECT1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CHARGE0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CHARGE1_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CARRIAGE0_INTERFACE_ERROR1) | ReadDB_DWORD(DBINDEX_CARRIAGE1_INTERFACE_ERROR1))

	if (dwTemp)
	{
		g_wManualKey_ClrErrFlag = 1;                // Anders 2015-7-17, Add.
		ResetTimer(&g_lManualKey_ClrErrCount, 100); //500);
	}
	else
	{ // Anders 2016-6-2, Add.
		g_wManualKey_ClrErrFlag = 1;
		ResetTimer(&g_lManualKey_ClrErrCount, 20);
	}

	ClearDBErr();

	g_sDB.M28.MONITOR.NET_RX_MANUALKEY_CNT++; // Anders 2017-3-6, add.
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
static WORD SetTemper91Msg(WORD* pawDataSeg)
{
	Temper91SegHead_T*      psTemper91SegHead;
	Temper91Segment_T*      psTemper91Segement;
	CommTemperCard_S const* psTemperCard;
	TemperCardNum_E         eTemperCardNum;
	DBIndex_T               tDBBase;
	NodeID_T                node_id;
	WORD                    length;
	WORD                    i;

	*pawDataSeg        = (MOTIONCMDTYPE_ONEMOTION << 8) | MOTIONID_TEMPERCARD;
	length             = 1;
	psTemper91Segement = (Temper91Segment_T*)&pawDataSeg[1];
	for (eTemperCardNum = TEMPERCARDNUM_NO1; eTemperCardNum < TEMPERCARDNUM_SUM; ++eTemperCardNum)
	{
		node_id = ReadDB_WORD(DBINDEX_M28_CONFIG_TEMPERATURE_CARD1_NODEID + eTemperCardNum);
		if (0 == node_id)
		{
			continue;
		}

		psTemper91SegHead  = (Temper91SegHead_T*)psTemper91Segement;
		psTemper91Segement = (Temper91Segment_T*)&psTemper91SegHead[1];

		psTemper91SegHead->sState.bNodeID   = node_id;
		psTemper91SegHead->sState.bReserve0 = 0;
		psTemper91SegHead->sState.bReserve1 = 0;

		// Check whether the online
		psTemperCard = &g_asCommTemperCard[eTemperCardNum];
		length += sizeofW(*psTemper91SegHead);
		if (0 == (g_sDB.M28.MONITOR.TEMPERATURE_CARD_LINK_STATE & ((WORD)1 << eTemperCardNum)))
		{
			psTemper91SegHead->sState.bNodeState = false;
			psTemper91SegHead->wEndStart         = (psTemperCard->PRI_xSegNext << 8) | (psTemperCard->PRI_xSegStart + 1);
			tDBBase                              = DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE + (TEMPER_ONESEG_DATANBR * psTemperCard->PRI_xSegStart);
			for (i = psTemperCard->PRI_xSegStart; i < psTemperCard->PRI_xSegNext; ++i)
			{
				psTemper91Segement->wCurrent = 0; //The current temperature temperature card
				psTemper91Segement->wState   = 0; //Temperature card status
				tDBBase += TEMPER_ONESEG_DATANBR;
				++psTemper91Segement;
				length += sizeofW(*psTemper91Segement);
			}
		}
		else
		{
			psTemper91SegHead->sState.bNodeState = true;
			psTemper91SegHead->wEndStart         = (psTemperCard->PRI_xSegNext << 8) | (psTemperCard->PRI_xSegStart + 1);
			tDBBase                              = DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE + (TEMPER_ONESEG_DATANBR * psTemperCard->PRI_xSegStart);
			for (i = psTemperCard->PRI_xSegStart; i < psTemperCard->PRI_xSegNext; ++i)
			{
				psTemper91Segement->wCurrent = DB_GetLONG(tDBBase + (DBINDEX_TEMPER1_INTERFACE_SEG1_REAL_CURRENT - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE)); //The current temperature temperature card
				psTemper91Segement->wState   = DB_GetWORD(tDBBase + (DBINDEX_TEMPER1_INTERFACE_SEG1_REAL_STATUS - DBINDEX_TEMPER1_INTERFACE_SEG1_SET_SENSOR_TYPE));  //Temperature card status
				tDBBase += TEMPER_ONESEG_DATANBR;
				++psTemper91Segement;
				length += sizeofW(*psTemper91Segement);
			}
		}
	}
	psTemper91SegHead = (Temper91SegHead_T*)psTemper91Segement;
	length += sizeofW(*psTemper91SegHead) + sizeofW(*psTemper91Segement);
	psTemper91SegHead->sState.bNodeID    = 0;
	psTemper91SegHead->sState.bNodeState = 0;
	psTemper91SegHead->sState.bReserve0  = 0;
	psTemper91SegHead->sState.bReserve1  = 0;
	psTemper91SegHead->wEndStart         = 0;

	//psTemper91SegHead[0].bNodeID = 11;  // Test
	//psTemper91SegHead[0].bNodeState = 1;
	//
	//lTemper = 11;
	//psTemper91SegHead[0].sTemper[0].wCurrent = lTemper;
	//lTemper = 222;
	//psTemper91SegHead[0].sTemper[1].wCurrent = lTemper;
	//lTemper = -1;
	//psTemper91SegHead[0].sTemper[2].wCurrent = lTemper;
	//lTemper = -11;
	//psTemper91SegHead[0].sTemper[3].wCurrent = lTemper;
	//lTemper = -222;
	//psTemper91SegHead[0].sTemper[4].wCurrent = lTemper;
	//lTemper = -333;
	//psTemper91SegHead[0].sTemper[5].wCurrent = lTemper;
	//lTemper = -4444;
	//psTemper91SegHead[0].sTemper[6].wCurrent = lTemper;
	//lTemper = -32768;
	//psTemper91SegHead[0].sTemper[7].wCurrent = lTemper;
	//lTemper = 32767;
	//psTemper91SegHead[0].sTemper[8].wCurrent = lTemper;
	return length;
}

void Send91_Slow(int nSessionHandle)
{
	//WORD              *pData = (WORD *)&u_TempTxBuffer + NETHEADERLEN;
	IMETMHEADER* pTMHeader = &u_TempTxBuffer.TMHeader;
	//TemperCardNum_T       eTemperCardNum      =   (TemperCardNum_T)(wListIndex - 1);
	//WORD              i,  wCurCardBaseDBIndex,    wDBIndex;

	//ASSERT(((WORD)eTemperCardNum) < ((WORD)TEMPERCARDNUM_SUM));
	memset(&u_TempTxBuffer, 0, (DWORD)ADDADATA_TOTALLEN);
	g_ActDataType.bit.bCounter++;
	pTMHeader->wSTX     = STX;
	pTMHeader->Type     = g_ActDataType;
	pTMHeader->wCMD     = CMD_91;
	pTMHeader->wID      = 0;
	pTMHeader->wDataLen = ADDADATALEN;
	pTMHeader->wSize    = ADDADATALEN + NETNODATASIZE;

	/*
    *pData      =   u_aOtherAutoRespList[wListIndex].wCMDType_MotionID;
    *++pData    =   TemperCard_Get_NodeID(eTemperCardNum);                                              //Temperature card NodeID
    if (0 == *pData)        return;                                                                     //Card node for a value other than 0

    if (0 != (ReadDB_WORD(M3_TEMPERCARDNUM_LINKSTATE) & (1 << eTemperCardNum)))                             //Check whether the online
        *pData      |=  1 << 8;                                                                         //WORD2 Eighth is the current node connection state
    *++pData    =   ((TEMPER_MAX_SEG + TEMPER_MAX_SEG * eTemperCardNum) << 8) | (1 + TEMPER_MAX_SEG * eTemperCardNum);  //  wCurCardBaseDBIndex     =   TEMPERCARD1_INTERFACE_LSEG1_CURRENT + ((TEMPER_MAX_SEG * TEMPER_ONESEG_DATANBR) * eTemperCardNum);  //Get the current temperature card based database index
    for (i = 0; i < TEMPER_MAX_SEG; ++i)
    {
        wDBIndex        =   wCurCardBaseDBIndex + (TEMPER_ONESEG_DATANBR * i);                                                      //The current temperature card corresponding segment
        *++pData        =   ReadDB_WORD(wDBIndex);                                                                                  //The current temperature temperature card
        *++pData        =   ReadDB_WORD(wDBIndex - (TEMPERCARD1_INTERFACE_LSEG1_CURRENT - TEMPERCARD1_INTERFACE_WSEG1_STATUS));     //Temperature card status
    }*/
	SetTemper91Msg(u_TempTxBuffer.awData);

	*(u_TempTxBuffer.awData + pTMHeader->wDataLen)     = MakeCRCValue(&pTMHeader->wSize, pTMHeader->wSize - NOCRCSIZE);
	*(u_TempTxBuffer.awData + pTMHeader->wDataLen + 1) = ETX;

	SendNetData(nSessionHandle, (WORD*)&u_TempTxBuffer, pTMHeader->wSize << 1);

	// u_sAutoRespList_Slow.bCycle = SLOW_91_SENDCYCLE; // THJ 2015-06-16 Mask:

#ifdef DEBUG
	++g_dwTemperCardCount;
#endif
}
/**
 * [CommReady description]
 * @return [description]
 */
BOOL GetCommReadyState(void)
{
	return (u_fCommFunc == COMMSTEP_WORKING);
}
/**
 * [NetCycleResp_Run description]
 * @param psResp [description]
 */
static void NetCycleResp_Run(NetCycleResp_S* psResp)
{
	if (psResp->bFlag && CheckTimerOut(&psResp->dwCounter))
	{
		ResetTimer(&psResp->dwCounter, psResp->bCycle);
		psResp->pFunc(g_nDsp54NetSession);
	}
}
/**
 * [COMMSTEP_IDLE description]
 */
static void COMMSTEP_IDLE(void)
{
	if (g_wMasterReady)
	{
		NetCycleResp_Run(&u_sAutoRespList_Fast);
		if (GetMoldsetState())
		{
			ResetTimer(&u_dwCommReadyCount, 3000);
			u_fCommFunc = COMMSTEP_DELAY;
		}
	}
}
static void COMMSTEP_DELAY(void)
{
	NetCycleResp_Run(&u_sAutoRespList_Fast);
	if (CheckTimerOut(&u_dwCommReadyCount) || CheckTemperCard_CommReady()) // THJ 20160825 Modify:
		u_fCommFunc = COMMSTEP_WORKING;
}
static void COMMSTEP_WORKING(void)
{
	NetCycleResp_Run(&u_sAutoRespList_Fast);
	NetCycleResp_Run(&u_sAutoRespList_Slow);
}

/**
 * [CheckTemperCard_CommReady description]
 * @return  [description]
 */
static BOOL CheckTemperCard_CommReady(void)
{
	TemperCardNum_E eTemperCardNum;

	for (eTemperCardNum = TEMPERCARDNUM_NO1; eTemperCardNum < TEMPERCARDNUM_SUM; ++eTemperCardNum)
	{
		if (0 == DB_GetWORD(DBINDEX_M28_CONFIG_TEMPERATURE_CARD1_NODEID + eTemperCardNum))
			continue;
		if (0 == (ReadDB_WORD(DBINDEX_M28_MONITOR_TEMPERATURE_CARD_LINK_STATE) & ((WORD)1 << eTemperCardNum)))
			return FALSE;
	}
	return TRUE;
}

/**
 * [ClearDBErr description]
 */
void ClearDBErr()
{
	unsigned i;

	for (i = DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR1; i <= DBINDEX_LOCAL_AXISCARD_INTERFACE_ERROR16; ++i)
	{
		DB_SetDWORD(i, 0);
	}

	CAN1NodeID_Check();
	CAN2NodeID_Check();
}
