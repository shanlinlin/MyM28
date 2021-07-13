/*==========================================================================+
|  Function : Network commnunication                                        |
|  Task     : Network commnunication                                        |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders.Zhang                                                  |
|  Version  : V1.00                                                         |
|  Creation : 2009/07/20                                                    |
|  Revision : V3.0                                                          |
+==========================================================================*/

#ifndef D__TASKNET_MASTER
#define D__TASKNET_MASTER

#include "taskEtherCATMaster_app.h"
#include "comp_fifo.h"
#include "comp_fifo2.h"

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define _UNKNOWN 0
#define _BOOL 1
#define _BYTE 2
#define _WORD 3
#define _DWORD 4
#define _SINT 11
#define _USINT 12
#define _INT 13
#define _UINT 14
#define _DINT 15
#define _UDINT 16
#define _REAL 21
#define _LREAL 22
//#define     DO_55_28_NET_RESET
//#define     TEST_55_28_NET
//#define     TEST_55_28_NET4

//#ifdef      TEST_55_28_NET4
//#define         DEVICEHANDLE_MASTER                 1
//#else
#define DEVICEHANDLE_MASTER 0
//#endif

#define RXFAIL 0x0001
#define RXSUCCESS 0x0000
#define OFFLINETIME 1000                            //1s
#define MAX_TXBUFFER_NO (32 * (12 + 1 + 1) + 1 + 1) //(20141113 THJ(Modification: 260 to 290(BigPack+1CRC+1ETX))        //Max Tx data size ,1 word ETX.
#define MAX_NETDATABUFFER_NO (0x100-6-6)//100H-IMETMHEADER SIZE-TMHEADER SIZE  SHANLL ADD 20200512

#define TOKENID_TRANSMITDATA 0xFFFE
#define TOKENID_NULL 0xFFFF

#define MOTIONSTATUS_WAIT 0

#define MOTIONSTATE_ADDITIONALCMD 0x30
#define MOTIONSTATE_ADDITIONAL_OBJ_MOLD 0x0
#define MOTIONSTATE_ADDITIONAL_OBJ_HEAT 0x1

#define PACK91DATA_MOTIONAXIS_NUM 12
#define PACK91DATA_REMOTEIO_INPUT_NUM 8
#define PACK91DATA_REMOTEIO_AD_NUM 6
#define PACK91DATA_REMOTEIO_OUTPUT_NUM 8
#define PACK91DATA_MOTIONDA_DA_NUM 6
#define PACK91DATA_CANRULER_NUM 8
enum IMERESPONSEID
{
	RESP_ADDA,
	RESP_CURVEVALUE,

	IMERESP_MAX
};
// protocol config for multiaxis zholy100205
enum AXISMOTIONID
{
	MOTIONID_ALL        = 0,
	MOTIONID_INJECT     = 1,
	MOTIONID_HOLDPRES   = 2,
	MOTIONID_SUCKBACK   = 3,
	MOTIONID_RECOVERY   = 4,
	MOTIONID_CLSM       = 5,
	MOTIONID_OPNM       = 6,
	MOTIONID_EJECTOR    = 7,
	MOTIONID_NOZZLE     = 8,
	MOTIONID_ADJUSTMOLD = 9,
	MOTIONID_ROTERY     = 10,

	MOTIONID_CLSM2     = 11,
	MOTIONID_EJECTOR2  = 12,
	MOTIONID_INJECT2   = 13,
	MOTIONID_RECOVERY2 = 14,
	MOTIONID_NOZZLE2   = 15,
	MOTIONID_ROTERY2   = 16,

	MOTIONID_TEMPERCARD = 17, //(20140917 THJ(Add: ))
	MOTIONID_M28DA      = 18, //(20141114 THJ(Add: ))
	MOTIONID_REMOTEIO   = 19, //(20140612 THJ(Add: ))

	MOTIONID_OTHER1 = 20, // Anders 2015-3-18, Add.
	MOTIONID_OTHER2 = 21, // Anders 2015-3-18, Add.
	MOTIONID_OTHER3 = 22, // Anders 2015-3-18, Add.
	MOTIONID_OTHER4 = 23  // Anders 2015-3-18, Add.
};

enum MOTIONCMDTYPE
{
	MOTIONCMDTYPE_NULL,
	MOTIONCMDTYPE_DA,
	MOTIONCMDTYPE_ONESTEP,
	MOTIONCMDTYPE_ONEMOTION
};
/*
 * Operate Mode.
 */
#define OPER_MODE_AUTO 0x7

enum OPER_MODE // OM_MANUAL = 0.
{
	OM_MANUAL    = 0,  // AUTO = 0                        ; 'AUTO        '
	OM_SEMI      = 1,  // SEMI AUTO                       ; 'SEMI AUTO   '
	OM_SENR      = 2,  // SENSOR-AUTO                     ; 'SENSOR-AUTO '
	OM_TIME      = 3,  // TIME AUTO                       ; 'TIME AUTO   '
	OM_ADJU      = 4,  // MOLD HT ADJ                     ; 'MOLD HT ADJ '
	OM_ADJM      = 5,  // MICRO HT ADJ                    ; 'MICRO HT ADJ'
	OM_POSN      = 6,  // POSITION SET                    ; 'POSITION SET'
	OM_PURG      = 7,  // AUTO PURGE                      ; 'AUTO PURGE  '
	OM_Reserved1 = 8,  //  Reserved
	OM_Reserved2 = 9,  //  Reserved
	OM_Reserved3 = 10, //  Reserved
	OM_AA_FLG    = 11, // AUTO ADJMOLD                    ; 'AUTO ADJMOLD'
	OM_AA_1ST    = 12, // COMPLETED 1ST STEP              ; 'COMPLETED 1ST STEP'
	OM_AA_DIR    = 13, // ADJUST DIRECTION FWD=1, BWD=0   ; 'ADJUST DIRECTION FWD=1, BWD=0'
	OM_AA_FST    = 14, // FAST AUTO ADJUST BY COUNT       ; 'FAST AUTO ADJUST BY COUNT'
	OM_AA_COM    = 15  // AUTO ADJUST COMPLETED           ; 'AUTO ADJUST COMPLETED'
};

enum MOTION_TYPE
{
	MT_COMMON,
	MT_JOG,
	MT_ADJUSTMOLD,
	MT_PURGE
};
enum eOPERATE_STEP
{
	OS_CLSM = 0,  // CLAMP CLOSE
	OS_PRSM = 1,  // PRESS MOLD
	OS_OPNM = 2,  // CLAMP OPEN
	OS_INJE = 3,  // INJECTION
	OS_SUCK = 4,  // SUCKBACK
	OS_CHRG = 5,  // CHARGE
	OS_NOZF = 6,  // NOZZLE ADV
	OS_NOZB = 7,  // NOZZLE RET
	OS_EJTF = 8,  // EJECTOR ADV
	OS_EJTB = 9,  // EJECTOR RET
	OS_CPRF = 10, // CORE IN
	OS_CPRB = 11, // CORE OUT
	OS_ADJF = 12, // MOLD HT THIN
	OS_ADJB = 13, // MOLD HT THICK
	OS_HLDP = 14, // HOLD PRESSURE
	OS_RESV = 15, // Reserve --> PreSuckback

	OS_MAX = 16
};
enum eSTATSLVPOSN
{
	STATSLVPOSN_INJ = 0,
	STATSLVPOSN_CLP = 1,
	STATSLVPOSN_EJT = 2,
	STATSLVPOSN_NOZ = 3,
	STATSLVPOSN_MAX = 4
};
/*---------------------------------------------------------------------------+
|           Type Definition                                                  |
+---------------------------------------------------------------------------*/

typedef union tagMasterType {
	WORD all;
	struct
	{
		WORD bVersion : 8; // hankin 20190604 modify: "bType" -> "bVersion". // 0: old; 1: remote link state add HT(J6) state.
		WORD bCounter : 8;
	} bit;
} TMasterType;
typedef struct tagIMETMHERHEADER
{
	WORD        wSTX;
	WORD        wSize; //Package Size.
	TMasterType Type;
	WORD        wCMD;
	WORD        wID;
	WORD        wDataLen;
} IMETMHEADER;

typedef struct tagIMECOMMMAP
{
	WORD wCmd;
	void (*pFunc)(int nSessionHandle, WORD * pdata, WORD wlength);
} IMECOMMMAP;

typedef struct tagIMETEMPBUFFER
{
	IMETMHEADER TMHeader;
	WORD        awData[MAX_TXBUFFER_NO];
} IMETEMPBUFFER;

typedef struct tagNETDATATEHEADER
{
	WORD wHeader;
	WORD wSize;
	WORD wType;
	WORD wCounter;
	union uDATATYPE {
		WORD wCounterRet;
		WORD wCycle;
		WORD wSequence;
		WORD wInternalState;
	} datatype;
	WORD wNoElements;
} NETDATATEHEADER;

typedef struct tagNETDATATEMPBUFFER
{
	NETDATATEHEADER 	NetDataHeader;
	WORD        awData[MAX_NETDATABUFFER_NO];
} NETDATATEMPBUFFER;

typedef struct tagRESPCMD
{
	IMETMHEADER TMHeader;
	struct tagData
	{
		WORD wData;
		WORD wCRC;
	} TData;
	WORD wETX;
} TRESPCMD;

typedef struct tagBOOTSTATE
{
	WORD   wFlag;
	Ttimer lCounter;
	//WORD      wCycle;
} BOOTSTATE;

typedef struct
{
	WORD   bFlag : 1;
	WORD   bCycle : 15;
	Ttimer dwCounter;
	void (*pFunc)(int nSessionHandle);
} NetCycleResp_S;

typedef struct tagMOTION_CONFIG
{
	WORD wConfig; // Type & MotionID.
	              //WORD      wCurveIndex;
} MOTION_CONFIG;

typedef union tagMOTION_ACTSTATUS {
	WORD all; // all==0: Wait
	struct
	{
		WORD bSubStep : 4; // Motion SubStep.
		WORD bResv1 : 4;   // Anders 2010-12-7. Bit define error.
		WORD bResv2 : 7;
		WORD bDoneFlag : 1;
	} doing;
	struct
	{
		WORD bCore1 : 1;
		WORD bCore2 : 1;
		WORD bCore3 : 1;
		WORD bCore4 : 1;
		WORD bCore5 : 1;
		WORD bCore6 : 1;
		WORD bResv1 : 2;    // Anders 2010-12-7. Bit define error.
		WORD bDataType : 7; // 0:TM Data; 1: HT J6+ Data; 2: Parker & HT J6+; 64:HT J6 Data.
		WORD bDoneFlag : 1;
	} done;
} MOTION_ACTSTATUS;
typedef struct tagOperateModeMap
{
	WORD wOperateMode;
	WORD wMotionType;
} TOperateModeMap;

typedef struct tagOperateStepMap
{
	//WORD  wOperateStepID;
	WORD wMotionCMD;
} TOperateStepMap;

typedef struct tagOperateStateMap
{
	WORD            wMotionType;
	TOperateStepMap OperateStepMap[OS_MAX];
} TOperateStateMap;

typedef struct tagTActStepLinkActAxis
{
	//WORD  wOperateStepID;
	//WORD  wSubStepAddr_4bit;
	WORD wActAxis; // Actual Motion Axis.
} TActStepLinkActAxis;

typedef union tagOperateMode {
	WORD all;
	struct
	{
		WORD bOM_MANUAL : 1;    // Bit0:
		WORD bOM_SEMI : 1;      // Bit1:
		WORD bOM_SENR : 1;      // Bit2:
		WORD bOM_TIME : 1;      // Bit3:
		WORD bOM_ADJU : 1;      // Bit4:
		WORD bOM_ADJM : 1;      // Bit5:
		WORD bOM_POSN : 1;      // Bit6:
		WORD bOM_PURG : 1;      // Bit7:
		WORD bOM_Reserved1 : 1; // Bit8:
		WORD bOM_Reserved2 : 1; // Bit9:
		WORD bOM_Reserved3 : 1; // Bit10:
		WORD bOM_AA_FLG : 1;    // Bit11:
		WORD bOM_AA_1ST : 1;    // Bit12:
		WORD bOM_AA_DIR : 1;    // Bit13:
		WORD bOM_AA_FST : 1;    // Bit14:
		WORD bOM_AA_COM : 1;    // Bit15:
	} bit;
} TOperateMode;

typedef union tagOperateStep {
	WORD all;
	struct
	{
		WORD bOS_CLSM : 1;  // Bit0:
		WORD bOS_PRSM : 1;  // Bit1:    Rsvd
		WORD bOS_OPNM : 1;  // Bit2:
		WORD bOS_INJE : 1;  // Bit3:
		WORD bOS_SUCK : 1;  // Bit4:
		WORD bOS_CHRG : 1;  // Bit5:
		WORD bOS_NOZF : 1;  // Bit6:
		WORD bOS_NOZB : 1;  // Bit7:
		WORD bOS_EJTF : 1;  // Bit8:
		WORD bOS_EJTB : 1;  // Bit9:
		WORD bOS_CPRF : 1;  // Bit10:   Rsvd
		WORD bOS_CPRB : 1;  // Bit11:   Rsvd
		WORD bOS_ADJF : 1;  // Bit12:   Rsvd
		WORD bOS_ADJB : 1;  // Bit13:   Rsvd
		WORD bOS_HLDP : 1;  // Bit14:   Rsvd
		WORD bOS_PRESB : 1; // Bit15:   PRESUCKBACK
	} bit;
} TOperateStep1;

typedef union tagOperateStep3 {
	WORD all;
	struct
	{
		WORD bOS_RESERVED08 : 9;        // Bit0~8
		WORD bOS_AUTOEJECT : 1;         // Bit9
		WORD bOS_NOZZLERELEASEPRES : 1; // Bit10
		WORD bOS_EJECTFWD_1 : 1;        // Bit11

		WORD bOS_RESERVED12 : 1; // Bit12:
		WORD bOS_RESERVED13 : 1; // Bit13:
		WORD bOS_RESERVED14 : 1; // Bit14:
		WORD bOS_LOWERPRESS : 1; // Bit15:
	} bit;
} TOperateStep3;
typedef union tagOperateStep6 {
	WORD all;
	struct
	{
		WORD bOS_CLSM2 : 1;  // Bit0:
		WORD bOS_PRSM2 : 1;  // Bit1:    Rsvd
		WORD bOS_OPNM2 : 1;  // Bit2:
		WORD bOS_INJE2 : 1;  // Bit3:
		WORD bOS_SUCK2 : 1;  // Bit4:
		WORD bOS_CHRG2 : 1;  // Bit5:
		WORD bOS_NOZF2 : 1;  // Bit6:
		WORD bOS_NOZB2 : 1;  // Bit7:
		WORD bOS_EJTF2 : 1;  // Bit8:
		WORD bOS_EJTB2 : 1;  // Bit9:
		WORD bOS_OTHER1 : 1; // Bit10:
		WORD bOS_OTHER2 : 1; // Bit11:
		WORD bOS_OTHER3 : 1; // Bit12:
		WORD bOS_OTHER4 : 1; // Bit13:
		WORD bOS_HLDP2 : 1;  // Bit14:   Rsvd
		WORD bOS_PRESB2 : 1; // Bit15:   PRESUCKBACK
	} bit;
} TOperateStep6;

typedef union tagOperateStep7 {
	WORD all;
	struct
	{
		WORD bOS_RESERVED08 : 5; // Bit0~4

		WORD bOS_MOLDPDV_CHL : 2; // Bit5~6
		WORD bOS_MOLDPDV_POV : 1; // Bit7
		WORD bOS_MOLDPDV_NEV : 1; // Bit8

		WORD bOS_AUTOEJECT2 : 1;         // Bit9
		WORD bOS_NOZZLE2RELEASEPRES : 1; // Bit10
		WORD bOS_EJECTFWD_2 : 1;         // Bit11

		WORD bOS_RESERVED12 : 1; // Bit12:
		WORD bOS_RESERVED13 : 1; // Bit13:
		WORD bOS_RESERVED14 : 1; // Bit14:
		WORD bOS_LOWERPRESS : 1; // Bit15:
	} bit;
} TOperateStep7;
typedef union tagOperateSubStep {
	WORD all;
	struct
	{
		WORD bCOPR : 4;
		WORD bCLSMOPNM : 4;
		WORD bINJECTION : 4;
		WORD bCHARGE : 4;
	} bit;
} TOperateSubStep1;

typedef union tagOperateSubStep2 {
	WORD all;
	struct
	{
		WORD bReserved1 : 4;
		WORD bReserved2 : 4;
		WORD bINJECTION_B : 4;
		WORD bCHARGE_B : 4;
	} bit;
} TOperateSubStep2;
typedef union tagSLVFLAG {
	WORD all;
	struct
	{
		WORD bSDF_AXIS : 4;
		WORD bSDF_CPOSN : 1;
		WORD bSDF_SPOSN : 1;
		WORD bSDF_ENDPB : 1;
		WORD bSDF_MDCNT : 1;
		WORD bSDF_PRFL : 1; // WORD bSDF_STIM      :1;
	} bit;
} SLVFLAG;
typedef union tagSLVMDC {
	WORD all;
	struct
	{
		WORD bCOUNT : 4;     // Count Bit0~Bit3
		WORD bReserved1 : 4; // Bit4~Bit7
		WORD bMODE : 4;      // MODE Bit8~Bit11
		WORD bReserved2 : 4; // Bit12~Bit15
	} bit;
} SLVMDC;
typedef struct tagSLVDATA
{
	SLVFLAG FLG;
	WORD    wPOS;
	WORD    wPRS;
	WORD    wFLW;
	long    lTIM;
	SLVMDC  MDC;
	WORD    wRSV;
} SLVDATA;

//(20141025 THJ(Add: ))
typedef struct
{
	WORD b0MoldTemperHeatSwitch : 1; // Bit0: (ON: 1 OFF: 0)
	WORD b1PipeHalfTemperSwitch : 1; // Bit1: (ON: 1 OFF: 0)
	WORD b2MoldHalfTemperSwitch : 1; // Bit0: (ON: 1 OFF: 0)     // THJ 2015-10-21 Add:
	WORD b3EmergencyStop : 1;        // BIT3��Emergency Stop        // 0��OFF��1��ON   (PAUL161118)
	WORD bRsvd : 12;                 // Anders 2015-4-13, Add.
} OtherState_T;
//)

typedef struct tagOPERATEADDTIONAL
{
	WORD         wMotorState; // ON: 0xFFFF OFF: 0
	WORD         wHeatState;  // ON: 0xFFFF OFF: 0
	OtherState_T OtherState;  //(20141024 THJ(Modification: ))
} OPERATEADDITIONAL;

typedef union tagCOMMPLC {
	WORD all;
	struct
	{
		WORD bRotate1_MachinePBOver : 1;
		WORD bRotate2_MachinePBOver : 1;
		WORD bRsvd : 14;
	} bit;

} COMMPLC;

typedef struct tagOperateStateBlock
{
	TOperateMode OperateMode;

	TOperateStep1 OperateStep1;
	WORD          OperateStep2;
	TOperateStep3 OperateStep3;
	WORD          OperateStep4;
	WORD          OperateStep5;

	TOperateSubStep1 OperateSubStep1;
	TOperateSubStep2 OperateSubStep2;

	TOperateStep1 ActStep1;
	WORD          ActStep2;
	TOperateStep3 ActStep3;
	WORD          ActStep4;
	WORD          ActStep5;
	TOperateStep6 ActStep6;
	TOperateStep7 ActStep7;
	WORD          ActStep8;

	struct
	{
		WORD wPress;
		WORD wFlow;
	} PressFlow[4];

	//SLVDATA         SLVDATA1;
	//SLVDATA         SLVDATA2;
	DWORD dwStopPosi_Mold1; // Anders 2015-1-13. Add.
	DWORD dwStopPosi_Eject1;
	DWORD dwStopPosi_Inject1;
	DWORD dwStopPosi_Carriage1;

	DWORD dwStopPosi_Mold2;
	DWORD dwStopPosi_Eject2;
	DWORD dwStopPosi_Inject2;
	DWORD dwStopPosi_Carriage2;

	//SLVDATA         SLVDATA3;
	WORD wHydraulicSpdSetpoint; //wStopPosi_Rotate1; // CX 20191019, Add  // Anders 2016-3 - 4, Add.
	//WORD            wStopPosi_Rotate2;          // Anders 2016-3 - 4, Add. Anders 2016-11-04, mark.
	WORD wHomingPosi_Rotate1; // Anders 2016-3 - 4, Add.
	//WORD            wHomingPosi_Rotate2;        // Anders 2016-3 - 4, Add. Anders 2016-11-04, mark.
	WORD wHTJ6HYBControlWord;  // Anders 2017-4-11, modify.
	WORD wRsvd;                // wHTJ6HYBMotionCMD;    // Anders 2017-4-11, modify. 2017-10-21, mark for new 5528 protocal.
	WORD wHTJ61EleControlWord; // Anders 2017-4-11, modify.
	WORD wHTJ61EleMotionCMD;   // Anders 2017-4-11, modify.
	WORD wHTJ62EleControlWord; // Anders 2017-4-11, modify.
	WORD wHTJ62EleMotionCMD;   // Anders 2017-4-11, modify.

	WORD              wPDVVoltage;                                      // Anders 2015-9-11, Add. 0-100 -> 0-10V
	COMMPLC           CommXPLC;                                         //(20140612 THJ(Add: ))
	WORD              awRemoteIOOutput[PACK91DATA_REMOTEIO_OUTPUT_NUM]; //(20140612 THJ(Add: RemoteIO Write Output data))
	WORD              awRemoteIODA[PACK91DATA_MOTIONDA_DA_NUM];         //(20140612 THJ(Add: RemoteIO DA data))
	OPERATEADDITIONAL AdditionalOP;
	struct
	{
		WORD bPumpEnable : 4;
		WORD bPumpReverse : 4;

		WORD bPIDGroup : 2;
		WORD bControlMode : 1;
		WORD bControlType : 4;
		WORD bHoldCMDFlag : 1;
	} Pump_PID[4];
	WORD  wHoming; // Anders 2015-4-13, Add.
	DWORD dwMold1Posi;
	DWORD dwEject1Posi;
	DWORD dwInject1Posi;
	DWORD dwMold2Posi;
	DWORD dwEject2Posi;
	DWORD dwInject2Posi;
	struct
	{
		WORD wPressScale;
		WORD wFlowScale;
	} PressFlowScale[4]; // Anders 2017-10-21, Add for new 5528 protocal.
	WORD wHTJ6HYBMotionCMD1;
	WORD wHTJ6HYBMotionCMD2;
	WORD wHTJ6HYBMotionCMD3;
	WORD wHTJ6HYBMotionCMD4;

	// WORD awHTJ6EleRsvd[12];
	WORD wRsvd1;

	WORD wDataType;
	union {
		WORD                   awHTModule[32]; // wDataType = 1.
		HTAXISCARD_PLUS_INPUTS awHTJ6Plus[2];  // wDataType = 1.
		WORD                   awTMRsvd[32];   // wDataType > 1.
	} uData;

} TOperateStateBlock; // 8+8+2+6+8*3+16=40H

//typedef   struct tagOperateState
//{
//  TOperateMode        OperateMode;
//  TOperateStep1       OperateStep;
//  TOperateStep1       ActStep1;
//  TOperateStep3       ActStep6;
//    WORD              wHYDROutTorque;
//    WORD              wHYDROutVelocity;
//}TOperateState;

typedef   struct tagHTPARKER_INPUTS
{
	WORD   PARKER_ControlWord_1;
	WORD   PARKER_ControlWord_2;
} HTPARKER_INPUTS;

typedef   struct tagJ6PP_OperateStateBlock
{
	HTAXISCARD_PP_INPUTS   awHTJ6PP[3];
	HTPARKER_INPUTS        wHTParker[2];
} J6PP_OperateStateBlock; // 26*3+2*2=52H WORD

typedef union tagNetcomState {
	WORD all;
	struct
	{
		WORD bBoot : 1;
		WORD bConfiguration : 1;
		WORD bMachineParameters1 : 1;
		WORD bMachineParameters2 : 1;

		WORD bMoldset1 : 1;
		WORD bMoldset2 : 1;
		WORD bMachineParameters_B : 1;
		WORD bMoldset_B1 : 1;

		WORD bMoldset_B2 : 1;
		WORD bOperateState : 1;
		//WORD    bAnyData            :1;
	} bit;
} TNetcomState;

typedef union tagMotionCheck {
	WORD all;
	struct
	{
		WORD bSendDone_Mold : 1;
		WORD bSendDone_Inject : 1;
		WORD bSendDone_Charge : 1;
		//WORD    bSendDone_Ejector :1;
		//WORD    bSendDone_Nozzle    :1;
	} bit;
} TMotionCheck;

typedef struct tagNetToken
{
	WORD wOwner;
	WORD wRsvd;
	long lCounter;
	long lInitCounter;
} TNetToken;

typedef struct
{
	WORD             wDataType;           // WORD1
	WORD             wError;              // WORD2
	WORD             wAlarmWarningL;      // wActCmd;             // WORD3
	WORD             wAlarmWarningH;      // wActStep;            // WORD4
	MOTION_ACTSTATUS Status;              // WORD5
	WORD             wActPosi1L;          // WORD6
	WORD             wActPosi1H;          // WORD7
	WORD             wActSpd1;            // WORD8
	INT              nMotorRPM1;          // WORD9
	WORD             wActPress1;          // WORD10
	WORD             wActForce1;          // WORD11
	WORD             wActPosi2L;          // WORD12
	WORD             wActPosi2H;          // WORD13
	WORD             wActSpd2;            // WORD14
	INT              nMotorRPM2;          // WORD15
	WORD             wActPress2;          // WORD16
	WORD             wActForce2;          // WORD17
	WORD             wInjectTransPosiL;   // WORD18
	WORD             wInjectTransPosiH;   // WORD19
	WORD             wInjectMinPosiL;     // WORD20
	WORD             wInjectMinPosiH;     // WORD21
	WORD             wInjectMaxPresPosiL; // WORD22
	WORD             wInjectMaxPresPosiH; // WORD23

	WORD wAlarmL;           // WORD24
	WORD wAlarmH;           // WORD25
	WORD wInjectTransPress; // WORD26
	WORD wInjectTransTime;  // WORD27
	WORD wProfilePress;     // WORD28
	WORD wProfileVelocity;  // WORD29
	WORD wPosiCountL;       // WORD30.
	WORD wPosiCountH;       // WORD31.
	WORD awRsvd;            // WORD32.
} Pack91Data_MotionAxis_T;

typedef struct
{
	WORD  wDataType;                              // WORD1
	WORD  wLinkState;                             // WORD2
	WORD  awInput[PACK91DATA_REMOTEIO_INPUT_NUM]; // WORD3-10
	WORD  awAD[PACK91DATA_REMOTEIO_AD_NUM];       // WORD11-16
	DWORD adwCANPosi[PACK91DATA_CANRULER_NUM];    // WORD17-32.
} Pack91Data_RemoteIO_T;
typedef struct
{
	WORD wDataType;  // WORD1
	WORD wDA1;       // WORD2
	WORD wDA2;       // WORD3
	WORD wDA3;       // WORD4
	WORD wDA4;       // WORD5
	WORD wDA5;       // WORD6
	WORD wDA6;       // WORD7
	WORD wDA7;       // WORD8
	WORD wDA8;       // WORD9
	WORD wDataGroup; // WORD10: 0-TM; 1-HT.
	WORD wEncoder;   // WORD11: 28 Encoder.
	WORD w12;        // WORD12
	WORD w13;        // WORD13
	WORD w14;        // WORD14
	WORD w15;        // WORD15
	WORD w16;        // WORD16
	WORD w17;        // WORD17
	WORD w18;        // WORD18
	WORD w19;        // WORD19
	WORD w20;        // WORD20
	WORD w21;        // WORD21
	WORD w22;        // WORD22
	WORD w23;        // WORD23
	WORD w24;        // WORD24
	WORD w25;        // WORD25
	WORD w26;        // WORD26
	WORD w27;        // WORD27
	WORD w28;        // WORD28
	WORD w29;        // WORD29
	WORD w30;        // WORD30
	WORD w31;        // WORD31
	WORD w32;        // WORD32
} Pack91Data_MotionDA_T;

typedef struct
{
	Pack91Data_MotionAxis_T sMotionAxis[PACK91DATA_MOTIONAXIS_NUM];
	Pack91Data_RemoteIO_T   sRemoteIO;
	Pack91Data_MotionDA_T   sM28DA;
} Fast91_T;

typedef union tagHTJ6CardConfig {
	WORD all;
	struct
	{
		WORD bCardType1 : 4; // 0: NotUsed; 1: J6-Oil; 2: J6-Ele; 3: J6-Plus.
		WORD bCardType2 : 4; // 0: NotUsed; 1: J6-Oil; 2: J6-Ele; 3: J6-Plus.
		WORD bCardType3 : 4; // 0: NotUsed; 1: J6-Oil; 2: J6-Ele; 3: J6-Plus.
		WORD bCardType4 : 4; // 0: NotUsed; 1: J6-Oil; 2: J6-Ele; 3: J6-Plus.
	} bit;
} THTJ6CardConfig;

/*==========================================================================+
|           External                                                        |
+==========================================================================*/
WORD CreateTaskNet_master(void);
void DestoryTaskNet_master(void);
void RunTaskNet_master(void);
void ParseNetData(int nSessionHandle, WORD * pData, WORD wLength);
WORD MakeCRCValue(WORD * pData, WORD wLength);
void SendRetransmitData(int nSessionHandle, WORD * pData, WORD wBytes);     //Dsp28->DSP54->HMI.
void SendRetransmitDataLess(int nSessionHandle, WORD * pData, WORD wBytes); //Dsp28->DSP54->HMI.
void RunTaskHMIFifoRead(void);//shanll 20201209 add

//BOOL  GetToken_Retransmit(WORD wOwner);
//void  GivebackToken_Retransmit(void);
//WORD    GetOwner_Retransmit(void);

//void    SetQuicklySendAD_DA(WORD wAxis);
void ChangeCurrAxisActMotionID(WORD wMotionID); // Anders 2010-12-7.
BOOL GetCommReadyState(void);
void StartTxTemperDataToMaster();
void Command_ManualKey();
void MainloopParseHMIData();//shanll add 20200512

extern int g_nDsp54NetSession;

//extern    TOperateState       g_CurrOperateState;
extern int g_nDsp54NetSession;

extern BOOTSTATE             g_BootState;
extern WORD                  g_wMasterReady;
extern volatile TMotionCheck g_SendDoneCheck;
extern TOperateMode          g_OperateMode;
extern BOOL                  g_bDealDataImmediately;
extern WORD                  g_wCycle5528J6;
extern Fifo 				 u_HMIFifo;
#ifdef __cplusplus
}
#endif

#endif
