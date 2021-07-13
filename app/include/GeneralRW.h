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
#ifndef generalrw__
#define generalrw__
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
#define GENERALRW_GROUP_SUM 48 // The only is 48.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
//enum T_Result
//{
//    generalrw_RESULT_DONE,
//    generalrw_RESULT_DOING,
//    generalrw_RESULT_TIMEOUT,
//    generalrw_RESULT_FAULT
//};
//##############################################################################
//
//      Type Definition
//
//##############################################################################
union T_GeneralRWType {
	DWORD iAll;
	struct
	{
		WORD iType : 8;
		WORD iNodeId_slaveIndex : 8;
		WORD iChannel : 8;
		WORD iPackgeCount : 4;
		WORD iPackgeIndex : 4;
	} sBit;
};

union T_GeneralRWIndex {
	DWORD iAll;
	struct
	{
		WORD Rsvd : 8;
		WORD iSubIndex : 8;
		WORD iIndex : 16;
	} sBit;
};

union T_GeneralRWLength {
	DWORD iAll;
	struct
	{
		WORD iLength : 16;
		WORD Rsvd : 8;
		WORD iPackgeCount : 4;
		WORD iPackgeIndex : 4;
	} sBit;
};

union T_GeneralRWID {
	DWORD iAll;
	struct
	{
		WORD iLenOfBits : 8;
		WORD iSubIndex : 8;
		WORD iIndex : 16;
	} sBit;
};

// struct T_GeneralRWGroup
// {
// 	union T_GeneralRWID uID;
// 	DWORD            iValue;
// };

struct T_GeneralRWInfo
{
	union T_GeneralRWType uGeneralRWType;
	union {
		DWORD iAll;
		struct
		{
			WORD iDataStatus : 16;
			WORD Rsvd : 8;
			WORD iPackgeCount : 4;
			WORD iPackgeIndex : 4;
		} sBit;
	} uStatus;
	// union {
	// 	DWORD iAll;
	// 	struct
	// 	{
	// 		WORD iRxIDStatus;
	// 		WORD iExecStatus;
	// 	} sBit;
	// } uPackge;
	union  T_GeneralRWIndex uIndex;
	DWORD aGroup[GENERALRW_GROUP_SUM];
	union T_GeneralRWLength uLength;
};

struct T_GeneralRWController
{
    struct T_GeneralRWInfo* _pWriteGeneralRWInfo;
    union  T_GeneralRWType  _uWriteGeneralRWType;
	union  T_GeneralRWIndex _uWriteIndex;					//备份ID
    DWORD				 _aWriteBackup[GENERALRW_GROUP_SUM];//备份value
	union T_GeneralRWLength	_uWriteLength;
    WORD                 _bWriteFree : 1;
    WORD                 _bWriteTired : 1;
    WORD                 _iWriteTimeOutCount;
    BOOL                 _iWriteEnable;
    WORD                 _iGeneralRWWriteFlag;
    WORD                 _iGeneralRWWriteIdChangeFlag;

    struct T_GeneralRWInfo* _pReadGeneralRWInfo;
    union  T_GeneralRWType  _uReadGeneralRWType;
	union  T_GeneralRWIndex _uReadIndex;					//备份ID
    DWORD				 _aReadBackup[GENERALRW_GROUP_SUM];//备份value
	union T_GeneralRWLength	_uReadLength;
    WORD                 _bReadFree : 1;
    WORD                 _bReadTired : 1;
    WORD                 _iReadTimeOutCount;
    BOOL                 _iReadEnable;
    WORD                 _iGeneralRWReadFlag;
    WORD                 _iGeneralRWReadIdChangeFlag;
};
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Create_GeneralRW(void);
void Run_GeneralRW(void);
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
