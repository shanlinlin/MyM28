/*==============================================================================+
|  Function : Watch                                                             |
|  Task     : Watch Header File                                                 |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2015/08/25                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__WATCH__H
#define D__WATCH__H
//------------------------------------------------------------------------------
#include "CANOpenCommon.h"
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
#define WATCH_FLAGSUM_WRITE 1
#define WATCH_FLAGSUM_READ 7
#define WATCH_FLAGSUM (WATCH_FLAGSUM_READ + WATCH_FLAGSUM_WRITE)
#define WATCH_FLAGARR ((WATCH_FLAGSUM + 0xF) >> 4)

#define WATCHCAN_READ_WAITTIME 100
#define WATCHCAN_WRITE_WAITTIME 100
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum WatchType_E {
	WATCHTYPE_NOTUSE,
	WATCHTYPE_ETHERCAT = 1,
	WATCHTYPE_CAN      = 2,
	WATCHTYPE_LOCAL    = 89
} WatchType_E;
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef struct Struct_WatchType_EtherCat
{
	DWORD _bConst : 8; // WatchType_E
	DWORD _bSlaveIndex : 8;
} S_WatchType_EtherCat;

typedef struct Struct_WatchType_Can
{
	DWORD _bConst : 8; // WatchType_E
	DWORD _bNodeID : 8;
	DWORD _bChannel : 8;
} S_WatchType_Can;

typedef union Union_WatchType {
	DWORD                _all;
	DWORD                _bType : 8; //  WatchType_E
	S_WatchType_EtherCat _sEtherCat;
	S_WatchType_Can      _sCan;
} U_WatchType;

typedef struct Struct_Watch
{
	void (*_Run)(struct Struct_Watch * const psWatch);
	union Union_WatchType uWatchType;
	union Union_WatchType uWatchTypeNew;

	struct
	{
		WORD _bDataSize : 8;
		WORD _bSubIndex : 8;
		WORD _bIndex : 16;
	} _sODWay, _sODWayOld;
	DWORD  _dwValue;
	Ttimer _dwAccessTime;
	WORD   _bMaxTimeoutCount : 3;

	WORD wWatchTypeIndex; // DBIndex_T
	WORD _wBackIndex;
	WORD _wCurReadIndex;
	WORD _wCurIndex;
	WORD _bOperatFinish : 1;
	WORD _bSumWriteIndex : 4;
	WORD _bSumIndex : 7;
	WORD _bTurnPages : 4;
	WORD _awAccessFlag[WATCH_FLAGARR];
} S_Watch;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Init_Watch(void);
void Run_Watch(void);
//------------------------------------------------------------------------------
void Watch_Init(S_Watch * psWatch);
void Watch_Init_Free(S_Watch * psWatch, WORD wSumWriteIndex, WORD wSumReadIndex); // Note: This is not safe. And if the total number of read and write data is greater than the number of _awAccessFlag, then the psWatch member group must have enough free space to provide the space for reading and writing.
void Watch_Run(S_Watch * psWatch);
void Watch_Parser_Can(S_Watch * const psWatch, CANOpenDataFrame_S const * const psData);
void Watch_SetWatchType(S_Watch * psWatch, U_WatchType * puType);
void Watch_WriteIndex(S_Watch * psWatch, DBIndex_T tIndex);
void Watch_ReadIndex(S_Watch * psWatch, DBIndex_T tIndex);
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
