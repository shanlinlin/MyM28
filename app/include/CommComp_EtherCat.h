/*==============================================================================+
|  Function : Communication Component EtherCat                                  |
|  Task     : Communication Component EtherCat Header File                      |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2014/12/23                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__COMMCOMP_ETHERCAT__H
#define D__COMMCOMP_ETHERCAT__H
//-----------------------------------------------------------------------------
#include "CANOpenCommon.h"
#include "CommComp_Factory.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_EtherCatEvent {
	ETHERCATEVENT_RESET_COMM,
	ETHERCATEVENT_SDO_READ,
	ETHERCATEVENT_SDO_WRITE,
	ETHERCATEVENT_SDO_COMPLATEREAD_FINISH,
	ETHERCATEVENT_SDO_COMPLATEWRITE_FINISH,

	ETHERCATEVENT_SUM,
	ETHERCATEVENT_MAX = ETHERCATEVENT_SUM - 1
} E_EtherCatEvent;

typedef enum Enum_EtherCatEventRetCode {
	ETHERCATEVENTRETCODE_SUCCESS,
	ETHERCATEVENTRETCODE_SDO_COMPLETEREAD,
	ETHERCATEVENTRETCODE_SDO_COMPLETEWRITE,
	ETHERCATEVENTRETCODE_SDO_PASS,

	ETHERCATEVENTRETCODE_SUM,
	ETHERCATEVENTRETCODE_MAX = ETHERCATEVENTRETCODE_SUM - 1
} E_EtherCatEventRetCode;

typedef enum Enum_EtherCatCtrlCmd {
	ETHERCATCTRLCMD_RESET_COMM,

	ETHERCATCTRLCMD_SUM,
	ETHERCATCTRLCMD_MAX = ETHERCATCTRLCMD_SUM - 1
} E_EtherCatCtrlCmd;
//------------------------------------------------------------------------------
#if (DEBUG)
#define EtherCatEvent_E E_EtherCatEvent
#define EtherCatEventRetCode_E E_EtherCatEventRetCode
#define EtherCatCtrlCmd_E E_EtherCatCtrlCmd

#else
#define EtherCatEvent_E WORD
#define EtherCatEventRetCode_E WORD
#define EtherCatCtrlCmd_E WORD
#endif
//##############################################################################
//
//      Type Definition
//
//##############################################################################
union EtherCatEvent_U;
struct EtherCatODInfo_S;
typedef struct EtherCatODInfo_S (*EtherCatDBIndexToODInfo_F)(CommComp_S * psComp, CommSubComp_S * psSubComp, DBIndex_T tDBIndex);
typedef EtherCatEventRetCode_E (*EtherCatEvent_F)(CommComp_S * psComp, union EtherCatEvent_U * puEvent);
typedef WORD (*EtherCatUploadSDO_F)(E_CommType eCommType, BOOL bExcute, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD * pData, WORD wByteSize, BOOL bCompleteAccess);
typedef WORD (*EtherCatDownloadSDO_F)(E_CommType eCommType, BOOL bExcute, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD * pData, WORD wByteSize, BOOL bCompleteAccess);
//------------------------------------------------------------------------------
typedef struct EtherCatEvent_SDORead_S
{
	EtherCatEvent_E Const;
	DBIndex_T       tCurIndex;
	ODID_T          tRetID;
	WORD            wRetMsgSize;
} EtherCatEvent_SDORead_S;
typedef struct EtherCatEvent_SDOComplateReadFinish_S
{
	EtherCatEvent_E Const;
	DBIndex_T       tCurIndex;
	WORD const *    pawMsgBuffer;
} EtherCatEvent_SDOComplateReadFinish_S;
typedef struct EtherCatEvent_SDOWrite_S
{
	EtherCatEvent_E Const;
	DBIndex_T       tCurIndex;
	WORD *          pawRetMsgBuffer;
	ODID_T          tRetID;
	WORD            wRetMsgSize;
} EtherCatEvent_SDOWrite_S;
typedef struct EtherCatEvent_SDOComplateWriteFinish_S
{
	EtherCatEvent_E Const;
	DBIndex_T       tCurIndex;
} EtherCatEvent_SDOComplateWriteFinish_S;
typedef union EtherCatEvent_U {
	EtherCatEvent_E                        eEvent;
	EtherCatEvent_SDORead_S                sSDORead;
	EtherCatEvent_SDOComplateReadFinish_S  sSDOReadFinish;
	EtherCatEvent_SDOWrite_S               sSDOWrite;
	EtherCatEvent_SDOComplateWriteFinish_S sSDOWriteFinish;
} EtherCatEvent_U;

typedef struct EtherCatODInfo_S
{
	WORD bDataSize : 8;
	WORD bSubIndex : 8;
	WORD tODID;
} EtherCatODInfo_S;

typedef struct EtherCatSubCompUnique_S
{
	EtherCatDBIndexToODInfo_F PRO_fDBIndexToODInfo;
} EtherCatSubCompUnique_S;

typedef struct EtherCatCompUnique_S
{
	EtherCatEvent_F   PRO_fEvent;
	EtherCatODInfo_S  PRI_sODInfo;
	void *            PRI_pwBufferChannel;
	WORD              PRO_wNodeID;
	WORD              PRI_bSlaveIndex : 15;
	WORD              PRI_bSlavePulse : 1;
	WORD              PRI_bMsgSize : 15;
	WORD              PRI_bComplateAccess : 1;
	EtherCatCtrlCmd_E PRO_eCtrlCmd;
} EtherCatCompUnique_S;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Init_EtherCat(void);
void Run_EtherCat(E_CommType eCommType, CommComp_S * psComp);
//------------------------------------------------------------------------------
inline void EtherCat_ResetComm(EtherCatCompUnique_S * const psCompUnique);
//------------------------------------------------------------------------------
void SEtherCatSubCompUnique_Init(EtherCatSubCompUnique_S * const psSubCompUnique);
void SEtherCatSubCompUnique_InitDBIndexToODInfo(EtherCatSubCompUnique_S * const psSubCompUnique, EtherCatDBIndexToODInfo_F fDBIndexToODInfo);

void SEtherCatCompUnique_Init(EtherCatCompUnique_S * const psCompUnique);
void SEtherCatCompUnique_InitEvent(EtherCatCompUnique_S * const psCompUnique, EtherCatEvent_F fEvent);
void SEtherCatCompUnique_SetNodeID(EtherCatCompUnique_S * const psCompUnique, WORD wNodeID);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
inline void EtherCat_ResetComm(EtherCatCompUnique_S * psCompUnique)
{
	DEBUG_ASSERT(NULL != psCompUnique);
	psCompUnique->PRO_eCtrlCmd = ETHERCATCTRLCMD_RESET_COMM;
}
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
