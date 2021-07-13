/*
 * CommComp_Driver.h
 *
 *  Created on: 2020年3月11日
 *      Author: SHANLL
 */
#ifndef CommComp_driver__
#define CommComp_driver__
//------------------------------------------------------------------------------
#include "CommComp_CANOpenMaster.h"
#include "CommComp_Factory.h"
#include <stdbool.h>
#include "_eCan_28335.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
// definition
//##############################################################################
//SHANLL_20200323 ADD
//can有通讯实时读取
#define	INDEXMOTIONTEMPER					0x702D
#define	INDEXDRIVERTEMPER					0xF707
#define	INDEXERRORID						0x8000
//驱动器状态画面  只读
#define	INDEXVOLTAGEOUTPUT					0x1003
#define	INDEXELECTRICITYOUTPUT				0x1004
#define	INDEXPOWEROUTPUT					0x1005
#define	INDEXMOTIONSPEED					0x100F
#define	INDEXGENERATRIXVOLTAGE				0x1002
#define	INDEXANGLEREALTIME					0x1013
#define	INDEXSETOILPRESS					0x1014
#define	INDEXOILPRESSFEEDBACK				0x1015
//驱动器参数画面1 读写
#define	INDEXMAXROTATESPEED					0x4301
#define	INDEXSYSTEMOILPRESS					0x4302
#define	INDEXMAXREVERSEROTATESPEED			0x4308
#define	INDEXUNDERSETFLOW					0x4309
#define	INDEXUNDERSETPRESS					0x430A
#define	INDEXAI3MININPUT					0x041C
#define	INDEXAI3MININPUTSET					0x041D
#define	INDEXAI3MAXINPUT					0x041E
#define	INDEXAI3MAXINPUTSET					0x041F
//驱动器参数画面2 读写
#define	INDEXSPEEDCYCLEPROPGAIN1			0x200
#define	INDEXSPEEDCYCLEINTEGRALTIME1		0x201
#define	INDEXSPEEDCYCLEPROPGAIN2			0x203
#define	INDEXSPEEDCYCLEINTEGRALTIME2		0x204
#define	INDEXSPEEDCYCLEFILTERTIMECONSTANT	0x207
#define	INDEXTORQUEUPLIMIT					0x20A
#define	INDEXOILPRESSCMDRISETIME			0x4304
#define	INDEXOILPRESSSFILTERTIME			0x4319
#define	INDEXSPEEDCYCLECHANGEFCY1			0x202
#define	INDEXSPEEDCYCLECHANGEFCY2			0x206
#define	INDEXKP1							0x4305
#define	INDEXKI1							0x4306
//##############################################################################
// enumeration
//##############################################################################
enum CommComp_driver_e_index {
	CommComp_driver_INDEX_0,
	CommComp_driver_INDEX_1,

	CommComp_driver_INDEX_SUM
};
enum CommComp_driver_type
{
	CommComp_driver__CARD_TYPE_INOVANCE,
	CommComp_driver__CARD_TYPE_XINGTAI,
	CommComp_driver__CARD_TYPE_SUM
};

enum CommComp_driver_e_inovance_index
{
	CommComp_driver_INOVANCE_INDEX_0,
	CommComp_driver_INOVANCE_INDEX_1,

	CommComp_driver_INOVANCE_INDEX_SUM
};

enum CommComp_driver_e_xingtai_index//20200925 shanll add
{
	CommComp_driver_XINGTAI_INDEX_0,
	CommComp_driver_XINGTAI_INDEX_1,

	CommComp_driver_XINGTAI_INDEX_SUM
};
typedef struct tagHT_MB_DRIVER_INOVANCE    //SHANLL 20200323 ADD
{
    WORD	wTargetID;
    WORD	wSourceID;
    WORD	wCommondCode;
    WORD	wData[2];
    WORD	wCommCount;
    WORD	wLastCommCount;
    BOOL	bSendEnable;
    BOOL	bSendEnd;
    Ttimer	wSendTime;
    Ttimer	wSendAgain;
    Ttimer	wOfflineRestart;
    BOOL	bFlagRestart;
    WORD	wInovanceIndex;
} HT_MB_DRIVER_INOVANCE;

typedef struct tagXT_MB_DRIVER    //SHANLL 20200925 ADD
{
    WORD	wTargetID;
    WORD	wSourceID;
    WORD	wCommondCode;
    WORD	wData[2];
    WORD	wCommCount;
    WORD	wLastCommCount;
    BOOL	bSendEnable;
    BOOL	bSendEnd;
    Ttimer	wSendTime;
    Ttimer	wSendAgain;
    Ttimer	wSendPFCycle;
    Ttimer	wOfflineRestart;
    BOOL	bFlagRestart;
    WORD	wXingtaiIndex;
} XT_MB_DRIVER;


typedef struct tagXT_SENDENABLE    //SHANLL 20201111 ADD
{
    BOOL	bControlSendState;//0-控制命令关闭 1-控制命令开启
    BOOL	bControlSendEnable;//0-控制命令、压力、流量不能发送 1-仅控制命令可发送 2-仅压力流量可发送
    // BOOL	bControlSendEnable;//0-控制命令不能发送 1-控制命令可发送
    // BOOL	bPFSendEnable;//0-压力流量不能发送 1-压力流量可发送
} XT_SENDENABLE;

typedef struct tagHT_91_DRIVER_INOVANCE    //SHANLL 20200323 ADD
{
    WORD	wTargetID;
    WORD	wSourceID;
    WORD	wCommondCode;
    WORD	wData[2];
    WORD	wCommCount;
    WORD	wSendAgainCount;
} HT_91_DRIVER_INOVANCE;

typedef struct tagXT_91_DRIVER   //SHANLL 20200925 ADD
{
    WORD	wTargetID;
    WORD	wSourceID;
    WORD	wCommondCode;
    WORD	wData[2];
    WORD	wCommCount;
    bool    bSendTo55End;
    WORD	wSendAgainCount;
} XT_91_DRIVER ;

extern	BAUDRATECONF InovanceBaudrateConf;
extern	BAUDRATECONF XingtaiBaudrateConf;

extern	HT_MB_DRIVER_INOVANCE   g_HT_MB_Driver_Inovance;
extern	HT_91_DRIVER_INOVANCE   g_HT_91_Driver_Inovance;
extern	XT_MB_DRIVER            g_XT_MB_Driver;
extern	XT_91_DRIVER            g_XT_91_Driver;
extern	XT_SENDENABLE	        g_XT_SendEnable;
extern  WORD	Press_Xingtai;
extern  WORD	Flow_Xingtai;
extern  BOOL	bSend_InjState_Xingtai;
//##############################################################################
// data type
//##############################################################################
//##############################################################################
// function declaration
//##############################################################################
void  CommComp_driver_init(void);
void  CommComp_driver_clean(void);
bool  CommComp_driver_add_inovance(WORD wRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
//WORD  CommComp_driver_get_error(enum CommComp_driver_e_index eIndex);
LONG  CommComp_driver_get_act_position_1(enum CommComp_driver_e_index eIndex);
WORD  CommComp_driver_get_act_speed_1(enum CommComp_driver_e_index eIndex);
WORD  CommComp_driver_get_act_voltag(enum CommComp_driver_e_index eIndex);

bool  CommComp_driver_add_xingtai(WORD wRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
//##############################################################################
// data declaration
//##############################################################################
//##############################################################################
// function prototype
//##############################################################################
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif

