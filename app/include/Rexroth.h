/*=============================================================================+
| Function : watch                                                             |
| Task     : watch Header File                                                 |
|------------------------------------------------------------------------------|
| Compile  :                                                                   |
| Link     :                                                                   |
| Call     :                                                                   |
|------------------------------------------------------------------------------|
| Author   : SLL                                                               |
| Version  : V1.00                                                             |
| Creation : 2020/11/17                                                        |
| Revision :                                                                   |
+=============================================================================*/
#ifndef rexroth__
#define rexroth__
//------------------------------------------------------------------------------
#include "common.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define REXROTH_SDODATA_SUM		121

//##############################################################################
//
//      Type Definition
//
//##############################################################################
union T_RexrothDataType {
	DWORD iAll;
	struct
	{
		WORD iType : 8;
		WORD iNodeId_slaveIndex : 8;
		WORD iDataOprationCount : 8;
		WORD iPackgeCount : 4;
		WORD iPackgeIndex : 4;
	} sBit;
};

struct T_RexrothInfo
{
	union T_RexrothDataType uRexrothReadType;
	DWORD aGroup[REXROTH_SDODATA_SUM];
};

struct T_RexrothController
{
    struct T_RexrothInfo* 		_pRexrothDataInfo;
    union  T_RexrothDataType	_uRexrothDataType;
    DWORD						_aReadBackup[REXROTH_SDODATA_SUM];//备份value
	BOOL						_bReadEnable;
	BOOL						_bWriteEnable;
	WORD                 		_iWriteTimeOutCount;
	WORD                 		_iReadTimeOutCount;
	WORD   						wReadCurrentIndex;
	BOOL   						bReadLocationEnd;
	Ttimer 						lReadCounter;
	WORD                 		wWriteCurrentIndex;
	WORD    					wWriteContinuousCount;
	BOOL						wReadOrWrite;//0-Read allowed 1-Write allowed
	BOOL						bLinkStateLast;
};
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Create_Rexroth(void);
void Run_Rexroth(void);
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
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
