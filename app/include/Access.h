/*=============================================================================+
| Function : watch                                                             |
| Task     : watch Header File                                                 |
|------------------------------------------------------------------------------|
| Compile  :                                                                   |
| Link     :                                                                   |
| Call     :                                                                   |
|------------------------------------------------------------------------------|
| Author   : THJ                                                               |
| Version  : V1.00                                                             |
| Creation : 2016/11/21                                                        |
| Revision :                                                                   |
+=============================================================================*/
#ifndef access__
#define access__
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
#define ACCESS_GROUP_SUM 16 // The only is 16.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
//enum T_Result
//{
//    access_RESULT_DONE,
//    access_RESULT_DOING,
//    access_RESULT_TIMEOUT,
//    access_RESULT_FAULT
//};
//##############################################################################
//
//      Type Definition
//
//##############################################################################
union T_AccessType {
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

union T_AccessID {
	DWORD iAll;
	struct
	{
		WORD iLenOfBits : 8;
		WORD iSubIndex : 8;
		WORD iIndex : 16;
	} sBit;
};

struct T_AccessGroup
{
	union T_AccessID uID;
	DWORD            iValue;
};

struct T_AccessInfo
{
	union T_AccessType uAccessType;
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
	union {
		DWORD iAll;
		struct
		{
			WORD iRxIDStatus;
			WORD iExecStatus;
		} sBit;
	} uPackge;
	struct T_AccessGroup aGroup[ACCESS_GROUP_SUM];
};

struct T_AccessController
{
    struct T_AccessInfo* _pWriteAccessInfo;
    union T_AccessType   _uWriteAccessType;
    struct T_AccessGroup _aWriteBackup[ACCESS_GROUP_SUM];
    WORD                 _bWriteFree : 1;
    WORD                 _bWriteTired : 1;
    WORD                 _iWriteTimeOutCount;
    WORD                 _iWriteCurIndex;
    WORD                 _iAccessWriteFlag;
    WORD                 _iAccessWriteIdChangeFlag;

    struct T_AccessInfo* _pReadAccessInfo;
    union T_AccessType   _uReadAccessType;
    struct T_AccessGroup _aReadBackup[ACCESS_GROUP_SUM];
    WORD                 _bReadFree : 1;
    WORD                 _bReadTired : 1;
    WORD                 _iReadTimeOutCount;
    WORD                 _iReadCurIndex;
    WORD                 _iAccessReadFlag;
    WORD                 _iReadDataOK;
};
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Create_Access(void);
void Run_Access(void);
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
