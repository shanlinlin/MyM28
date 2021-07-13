/*==============================================================================+
|  Function : CommComp Factory                                                  |
|  Task     : CommComp Factory Head File                                        |
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
#ifndef D__COMMCOMP_FACTORY__H
#define D__COMMCOMP_FACTORY__H
//------------------------------------------------------------------------------
#include "database.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define Comm_SetReadFlag(D_tDBIndex) (g_sCommComp.adwReadFlag[(D_tDBIndex) >> 5] |= ((DWORD)1) << ((D_tDBIndex)&0x1F))
#define Comm_SetWriteFlag(D_tDBIndex) (g_sCommComp.adwWriteFlag[(D_tDBIndex) >> 5] |= ((DWORD)1) << ((D_tDBIndex)&0x1F))
#define Comm_ClrReadFlag(D_tDBIndex) (g_sCommComp.adwReadFlag[(D_tDBIndex) >> 5] &= ~(((DWORD)1) << ((D_tDBIndex)&0x1F)))
#define Comm_ClrWriteFlag(D_tDBIndex) (g_sCommComp.adwWriteFlag[(D_tDBIndex) >> 5] &= ~(((DWORD)1) << ((D_tDBIndex)&0x1F)))
#define Comm_GetReadChanged(D_tDBIndex) (0 != (g_sCommComp.adwReadFlag[(D_tDBIndex) >> 5] & (((DWORD)1) << ((D_tDBIndex)&0x1F))))
#define Comm_GetWriteChanged(D_tDBIndex) (0 != (g_sCommComp.adwWriteFlag[(D_tDBIndex) >> 5] & (((DWORD)1) << ((D_tDBIndex)&0x1F))))

#define Comm_SetRangeReadFlag(D_tStartIndex, D_tEndIndex) CommComp_SetRangeFlag(g_sCommComp.adwReadFlag, (D_tStartIndex), (D_tEndIndex))
#define Comm_SetRangeWriteFlag(D_tStartIndex, D_tEndIndex) CommComp_SetRangeFlag(g_sCommComp.adwWriteFlag, (D_tStartIndex), (D_tEndIndex))
#define Comm_ClrRangeReadFlag(D_tStartIndex, D_tEndIndex) CommComp_ClrRangeFlag(g_sCommComp.adwReadFlag, (D_tStartIndex), (D_tEndIndex))
#define Comm_ClrRangeWriteFlag(D_tStartIndex, D_tEndIndex) CommComp_ClrRangeFlag(g_sCommComp.adwWriteFlag, (D_tStartIndex), (D_tEndIndex))
//------------------------------------------------------------------------------

#define COMMCOMP_READFLAGARR_SUM ((DBINDEX_SUM + 0x1F) >> 5)
#define COMMCOMP_WRITEFLAGARR_SUM ((DBINDEX_SUM + 0x1F) >> 5)
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_CommType {
	COMMTYPE_CAN1,
	COMMTYPE_CAN2,
	COMMTYPE_ETHERCAT,

	COMMTYPE_SUM
} E_CommType;

typedef enum Enum_RetCode {
	RETCODE_SUCCESS,
	RETCODE_BUSY,
	RETCODE_FAIL,

	RETCODE_SUM
} E_RetCode;

#if (DEBUG)
typedef enum Enum_CompType {
	COMPTYPE_UNKNOWN,
	COMPTYPE_AXISCARD1 = 0x0101,
	COMPTYPE_AXISCARD2,
	COMPTYPE_AXISCARD3,
	COMPTYPE_AXISCARD4,
	COMPTYPE_AXISCARD5,
	COMPTYPE_LOCALAXISCARD,
	COMPTYPE_REMOTEIO1 = 0x0201,
	COMPTYPE_REMOTEIO2,
	COMPTYPE_REMOTEIO3,
	COMPTYPE_REMOTEIO4,
	COMPTYPE_REMOTEIO5,
	COMPTYPE_REMOTEIO6,
	COMPTYPE_REMOTEIO7,
	COMPTYPE_REMOTEIO8,
	COMPTYPE_REMOTEIO9,
	COMPTYPE_REMOTEIO10,
	COMPTYPE_REMOTEIO11,
	COMPTYPE_REMOTEIO12,
	COMPTYPE_REMOTEIO13,
	COMPTYPE_REMOTEIO14,
	COMPTYPE_REMOTEIO15,
	COMPTYPE_TEMPCARD1 = 0x0301,
	COMPTYPE_TEMPCARD2,
	COMPTYPE_TEMPCARD3,
	COMPTYPE_TEMPCARD4,
	COMPTYPE_TEMPCARD5,
	COMPTYPE_TEMPCARD6,
	COMPTYPE_TEMPCARD7,
	COMPTYPE_TEMPCARD8,
	COMPTYPE_PWMTRCARD1 = 0x0401,
	COMPTYPE_PWMTRCARD2,
	COMPTYPE_PWMTRCARD3,
	COMPTYPE_PICKER0 = 0x0501
} E_CompType;
#endif

typedef enum Enum_CommRetCode {
	COMMRETCODE_SUCCESS,
	COMMRETCODE_DELSUBCOMP_NOTFIND,
	COMMRETCODE_OTHER
} E_CommRetCode;

typedef enum Enum_CompState {
	COMPSTATE_INIT,
	COMPSTATE_CONFIG,
	COMPSTATE_SYNC,
	COMPSTATE_NORMAL,

	COMPSTATE_SUM
} E_CompState;

typedef enum Enum_CommSDOState {
	COMMSDOSTATE_IDLE,
	COMMSDOSTATE_READ,
	COMMSDOSTATE_WRITE,
	COMMSDOSTATE_FINISH,

	COMMSDOSTATE_SUM
} E_CommSDOState;

typedef enum Enum_CommSyncStep {
	COMMSYNCSTEP_SCAN,
	COMMSYNCSTEP_CHECK,

	COMMSYNCSTEP_SUM
} E_CommSyncStep;

typedef enum Enum_CommEvent {
	COMMEVENT_COMP_CHANGE_COMMTYPE,
	COMMEVENT_SUBCOMP_ADD,
	COMMEVENT_SUBCOMP_DEL,

	COMMEVENT_SUM
} E_CommEvent;

typedef enum Enum_CustomFuncType {
	CUSTOMFUNCTYPE_FAST,
	CUSTOMFUNCTYPE_SLOW,

	CUSTOMFUNCTYPE_SUM
} E_CustomFuncType;
//##############################################################################
//
//      Type Definition
//
//##############################################################################
struct CommComp_S;
struct CommOffsetSeg_S;
struct CommIndexSegInfo_S;
struct CommSubComp_S;
struct Const_CommComp_S;
struct Info_CommComp_S;
union CommEvent_U;
typedef void (*CommCustomFunc_F)(struct CommComp_S * const psComp, E_CustomFuncType eType);
typedef void (*CommEventCallBack_F)(struct CommComp_S * const psComp, union CommEvent_U * const puEvent);
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
typedef struct CommOffsetSeg_S
{
	DBIndex_T tStartOffset;
	DBIndex_T tEndOffset;
} CommOffsetSeg_S;

typedef struct CommSegInfo_S
{
	struct CommOffsetSeg_S const * pasIndexSeg;
	DBIndex_T                      tBaseIndex; // All in the "pasIndexSeg" in the base of index value.
	WORD                           bSegSum : 15;
	WORD                           bFlag : 1;  // flag.
	WORD                           bCFlag : 1; // clear flag.
	WORD                           bCurSeg : 15;
} CommSegInfo_S;

typedef struct CommSubComp_S
{
	struct CommSubComp_S * PRI_psNextSubComp;
	struct CommSegInfo_S   PRO_sSDO_SyncInfo;
	struct CommSegInfo_S   PRO_sSDO_WriteInfo;
	struct CommSegInfo_S   PRO_sSDO_ReadInfo;
	void *                 PRO_pvUnique;
} CommSubComp_S;

typedef struct CommComp_S
{
	struct CommComp_S *    PRI_psNextComp;
	struct CommSubComp_S * PRI_psSubCompHead;
	struct CommSubComp_S * PRI_psCurSubComp;
	CommEventCallBack_F    PRO_fEvent;
	CommCustomFunc_F       PRO_fCustomFunc;
	void *                 PRO_pvUnique;
	E_CommType             PRI_eCommType;
	DBIndex_T              PRI_tCurIndex;
	WORD                   PRO_bState : 2;    // E_CompState;
	WORD                   PRO_bSDOState : 2; // E_CommSDOState
	WORD                   PRI_bSyncStep : 2; // E_CommSyncStep

#if (DEBUG)
	E_CompType PUB_eCompType;
#endif
} CommComp_S;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
typedef struct Info_CommComp_S
{
	CommComp_S * apsCommCompHead[COMMTYPE_SUM];
	DWORD        adwReadFlag[COMMCOMP_READFLAGARR_SUM];
	DWORD        adwWriteFlag[COMMCOMP_WRITEFLAGARR_SUM];

	CommSubComp_S * apsSubCompTab[0xFF];
} Info_CommComp_S;

typedef struct Const_CommComp_S
{
	struct
	{
		E_CommRetCode (*fCreate)(E_CommType eCommType);
		E_CommRetCode (*fDestory)(E_CommType eCommType);
		void (*fParseMsg)(E_CommType eCommType, CommComp_S * const psCommCompHead);
		void (*fCtrlRun)(E_CommType eCommType, CommComp_S * const psCommComp);
	} asFunc[COMMTYPE_SUM];
} Const_CommComp_S;

typedef struct
{
	E_CommEvent     eEvent;
	CommSubComp_S * psSubComp;
} CommEvent_AddSubComp_S, CommEvent_DelSubComp_S;
typedef union CommEvent_U {
	E_CommEvent            eEvent;
	CommEvent_AddSubComp_S sAddSubComp;
	CommEvent_DelSubComp_S sDelSubComp;
} CommEvent_U;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void          Init_Comm(void);
E_CommRetCode Create_Comm(E_CommType eCommType);
void          Parse_Comm(E_CommType eCommType);
void          Run_SlowRun(E_CommType eCommType);
void          Run_FastRun(E_CommType eCommType);

void Comm_SetWriteFlagByDBIndex(DBIndex_T tDBIndex);
void Comm_SetReadFlagByDBIndex(DBIndex_T tDBIndex);
void CommComp_SetRangeFlag(DWORD * const padwFlagData, DBIndex_T tStartIndex, DBIndex_T tEndIndex);
void CommComp_ClrRangeFlag(DWORD * const padwFlagData, DBIndex_T tStartIndex, DBIndex_T tEndIndex);
//------------------------------------------------------------------------------
void SCommSubComp_Init(CommSubComp_S * const psSubComp);
void SCommSubComp_InitSyncInfo(CommSubComp_S * const psSubComp, CommOffsetSeg_S const * const pasIndexSeg, DBIndex_T tBaseIndex, WORD wSegSum);
void SCommSubComp_InitWriteInfo(CommSubComp_S * const psSubComp, CommOffsetSeg_S const * const pasIndexSeg, DBIndex_T tBaseIndex, WORD wSegSum);
void SCommSubComp_InitReadInfo(CommSubComp_S * const psSubComp, CommOffsetSeg_S const * const pasIndexSeg, DBIndex_T tBaseIndex, WORD wSegSum);
void SCommSubComp_SetSyncFlag(CommSubComp_S * const psSubComp);
void SCommSubComp_SetReadFlag(CommSubComp_S * const psSubComp);
void SCommSubComp_SetWriteFlag(CommSubComp_S * const psSubComp);
void SCommSubComp_SetAllSyncDBFlag(CommSubComp_S * const psSubComp);
void SCommSubComp_SetAllReadDBFlag(CommSubComp_S * const psSubComp);
void SCommSubComp_SetAllWriteDBFlag(CommSubComp_S * const psSubComp);
void SCommSubComp_ClrAllWriteDBFlag(CommSubComp_S * const psSubComp);

void               SCommComp_Init(CommComp_S * const psComp);
void               SCommComp_InitEventCallBack(CommComp_S * const psComp, CommEventCallBack_F fEventCallBack);
void               SCommComp_InitCustomFunc(CommComp_S * const psComp, CommCustomFunc_F fCustomFunc);
void               SCommComp_SetState(CommComp_S * const psComp, E_CompState eState);
void               SCommComp_SetCommType(CommComp_S * const psComp, E_CommType eCommType);
void               SCommComp_AddSubComp(CommComp_S * const psComp, CommSubComp_S * const psSubComp);
E_CommRetCode      SCommComp_DelSubComp(CommComp_S * const psComp, CommSubComp_S * const psSubComp);
void               SCommComp_ClrSDOState(CommComp_S * const psComp);
void               SCommComp_SetAllSyncFlag(CommComp_S * const psComp);
void               SCommComp_SetAllReadFlag(CommComp_S * const psComp);
void               SCommComp_SetAllWriteFlag(CommComp_S * const psComp);
void               SCommComp_ClrAllWriteFlag(CommComp_S * const psComp);
inline E_CommType  CommComp_GetCommType(CommComp_S const * const psComp);
inline E_CompState CommComp_GetState(CommComp_S const * const psComp);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern Info_CommComp_S g_sCommComp;
// extern CommComp_S                                    *u_apsCommCompHead[COMMTYPE_SUM];
//------------------------------------------------------------------------------
// extern DWORD                                     u_adwDBReadFlag[CANOPENMASTERCOMM_DBFLAG_SIZE];
// extern DWORD                                     u_adwDBWriteFlag[CANOPENMASTERCOMM_DBFLAG_SIZE];
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [CommComp_GetCommType description]
 * @param  psComp [description]
 * @return        [description]
 */
inline E_CommType CommComp_GetCommType(CommComp_S const * const psComp)
{
	DEBUG_ASSERT(NULL != psComp);
	return psComp->PRI_eCommType;
}

/**
 * [CommComp_GetState description]
 * @param  psComp [description]
 * @return        [description]
 */
inline E_CompState CommComp_GetState(CommComp_S const * const psComp)
{
	DEBUG_ASSERT(NULL != psComp);
	return (E_CompState)psComp->PRO_bState;
}
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
