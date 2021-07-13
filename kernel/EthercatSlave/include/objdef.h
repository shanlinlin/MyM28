/*===========================================================================+
|  Function : CANopen Application                                            |
|  Task     : CANopen Application                                            |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                   	 |
|  Link     : CCS3.1 -                                                   	 |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : wq                                                             |
|  Version  : V1.00                                                          |
|  Creation : 2010-1-17                                                      |
|  Revision :                                                          	     |
+===========================================================================*/
#ifndef D___OBJDEF
#define D___OBJDEF

#include "common.h"

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
// Anders 2011-01-21.
#define TMARRSDO_RW_NOELEMENTS 100
#define TMARRSDO_RW_DATALEN (3 * TMARRSDO_RW_NOELEMENTS) // 3w * 100
#define TMARRSDO_RW_LEN (sizeof(TMARRSDO_RW))
//SDO service
#define SDOSERVICE_INITIATEDOWNLOADREQ 0x01
#define SDOSERVICE_INITIATEDOWNLOADRES 0x03
#define SDOSERVICE_DOWNLOADSEGMENTREQ 0x00
#define SDOSERVICE_DOWNLOADSEGMENTRES 0x01
#define SDOSERVICE_INITIATEUPLOADREQ 0x02
#define SDOSERVICE_INITIATEUPLOADRES 0x02
#define SDOSERVICE_UPLOADSEGMENTREQ 0x03
#define SDOSERVICE_UPLOADSEGMENTRES 0x00
#define SDOSERVICE_ABORTTRANSFER 0x04

// Abort codes
#define ABORTIDX_TOGGLE_BIT_NOT_CHANGED 0x01
#define ABORTIDX_SDO_PROTOCOL_TIMEOUT 0x02
#define ABORTIDX_COMMAND_SPECIFIER_UNKNOWN 0x03
#define ABORTIDX_OUT_OF_MEMORY 0x04
#define ABORTIDX_UNSUPPORTED_ACCESS 0x05
#define ABORTIDX_WRITE_ONLY_ENTRY 0x06
#define ABORTIDX_READ_ONLY_ENTRY 0x07
#define ABORTIDX_OBJECT_NOT_EXISTING 0x08
#define ABORTIDX_OBJECT_CANT_BE_PDOMAPPED 0x09
#define ABORTIDX_MAPPED_OBJECTS_EXCEED_PDO 0x0A
#define ABORTIDX_PARAM_IS_INCOMPATIBLE 0x0B
#define ABORTIDX_INTERNAL_DEVICE_INCOMPATIBILITY 0x0C
#define ABORTIDX_HARDWARE_ERROR 0x0D
#define ABORTIDX_PARAM_LENGTH_ERROR 0x0E
#define ABORTIDX_PARAM_LENGTH_TOO_LONG 0x0F
#define ABORTIDX_PARAM_LENGTH_TOO_SHORT 0x10
#define ABORTIDX_SUBINDEX_NOT_EXISTING 0x11
#define ABORTIDX_VALUE_EXCEEDED 0x12
#define ABORTIDX_VALUE_TOO_GREAT 0x13
#define ABORTIDX_VALUE_TOO_SMALL 0x14
#define ABORTIDX_MAX_VALUE_IS_LESS_THAN_MIN_VALUE 0x15
#define ABORTIDX_GENERAL_ERROR 0x16
#define ABORTIDX_DATA_CANNOT_BE_READ_OR_STORED 0x17
#define ABORTIDX_DATA_CANNOT_BE_ACCESSED_BECAUSE_OF_LOCAL_CONTROL 0x18
#define ABORTIDX_IN_THIS_STATE_DATA_CANNOT_BE_READ_OR_STORED 0x19
#define ABORTIDX_NO_OBJECT_DICTIONARY_IS_PRESENT 0x1A
#define ABORTIDX_WORKING 0xFF

#define ABORT_NOERROR 0x00000000
#define ABORT_TOGGLE_BIT_NOT_CHANGED 0x05030000
#define ABORT_SDO_PROTOCOL_TIMEOUT 0x05040000
#define ABORT_COMMAND_SPECIFIER_UNKNOWN 0x05040001
#define ABORT_OUT_OF_MEMORY 0x05040005
#define ABORT_UNSUPPORTED_ACCESS 0x06010000
#define ABORT_WRITE_ONLY_ENTRY 0x06010001
#define ABORT_READ_ONLY_ENTRY 0x06010002
#define ABORT_OBJECT_NOT_EXISTING 0x06020000
#define ABORT_OBJECT_CANT_BE_PDOMAPPED 0x06040041
#define ABORT_MAPPED_OBJECTS_EXCEED_PDO 0x06040042
#define ABORT_PARAM_IS_INCOMPATIBLE 0x06040043
#define ABORT_INTERNAL_DEVICE_INCOMPATIBILITY 0x06040047
#define ABORT_HARDWARE_ERROR 0x06060000
#define ABORT_PARAM_LENGTH_ERROR 0x06060010
#define ABORT_PARAM_LENGTH_TOO_LONG 0x06060012
#define ABORT_PARAM_LENGTH_TOO_SHORT 0x06060013
#define ABORT_SUBINDEX_NOT_EXISTING 0x06090011
#define ABORT_VALUE_EXCEEDED 0x06090030
#define ABORT_VALUE_TOO_GREAT 0x06090031
#define ABORT_VALUE_TOO_SMALL 0x06090032
#define ABORT_MAX_VALUE_IS_LESS_THAN_MIN_VALUE 0x06090036
#define ABORT_GENERAL_ERROR 0x08000000
#define ABORT_DATA_CANNOT_BE_READ_OR_STORED 0x08000020
#define ABORT_DATA_CANNOT_BE_READ_OR_STORED_BECAUSE_OF_LOCAL_CONTROL 0x08000021
#define ABORT_DATA_CANNOT_BE_READ_OR_STORED_IN_THIS_STATE 0x08000022
#define ABORT_NO_OBJECT_DICTIONARY_IS_PRESENT 0x08000023
//OD access
#define OD_ACCESS_READWRITE 0x0001
#define OD_ACCESS_READONLY 0x0002
#define OD_ACCESS_READ_OP 0x0003 //only read in operation
//OD
#define OBJCODE_VAR 0x01
#define OBJCODE_ARR 0x02
#define OBJCODE_REC 0x03

#define MAX_PDO_NUM 8 // Anders 2012-1-31.
#define MAX_EXPEDITED_DATA 4

#define BIT2BYTE(x) ((x + 7) >> 3)
#define BIT2WORD(x) ((x + 15) >> 4)

typedef WORD (*pCANopenReadUpdate)(WORD wODIndex, WORD wDBIndex, WORD wSize, WORD * pData, WORD wSubIndex, BOOL bCompleteAccess);
typedef WORD (*pCANopenWriteUpdate)(WORD wODIndex, WORD wDBIndex, WORD wSize, WORD * pData, WORD wSubIndex, BOOL bCompleteAccess);

//typedef	void	(*pCOEReadUpdate)(WORD wDBIndex, WORD wSize, WORD* pData);
//typedef	void	(*pCOEWriteUpdate)(WORD wDBIndex, WORD wSize, WORD* pData);
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
//SDO service header:
typedef struct tagSDOHEADER
{
	union {
		struct
		{
			WORD SizeIndicator : 1; //whether size of Data in Data Set Size specified
			WORD TransferType : 1;  //whether Expedited transfer
			WORD DataSetSize : 2;   //Data Set Size
			WORD CompleteAccess : 1;
			WORD Command : 3;
			WORD IndexLow : 8;
		} bit;
		WORD all;
	} w1;
	union {
		struct
		{
			WORD IndexHigh : 8;
			WORD SubIndex : 8;
		} bit;
		WORD all;
	} w2;
} SDOHEADER;

// SDO Information / Entry Description:
typedef struct tagSDOINFOENTRYDESC
{
	WORD wBitLength;
	WORD wObjAccess;
} SDOINFOENTRYDESC;

//	Object dictionary entry structure
typedef struct tagOBJECT
{
	WORD                     wIndex;
	WORD                     wODType;
	const SDOINFOENTRYDESC * pEntryDesc;
	//const   BYTE                    *pName;   // Anders 2010-11-3
	void *              pVarPtr;
	pCANopenReadUpdate  ReadUpdate;
	pCANopenWriteUpdate WriteUpdate;
	WORD                wElementNum; // Anders 20120316.
	WORD                wDBIndex;
} OBJECT;

// Entry PDO Communication Parameter
typedef struct tagPDOCOMMUNICATION
{
	WORD u8SubIndex0;
	WORD COBID[2];
	WORD u8TransType;
	WORD u16InhibitTime;
	WORD u8Reserved;
	WORD u16EventTime;
} PDOCOMMUNICATION;

// abort request:
typedef struct tagABORTSDOTRANSFERREQ
{
	SDOHEADER SdoHeader;
	DWORD     AbortCode;
} ABORTSDOTRANSFERREQ;

//Entry PDO Maping
typedef struct tagPDOMAPING
{
	DWORD u8SubIndex0;
	DWORD aEntries[4];
} PDOMAPING;

//Entry PDO Assignment
typedef struct tagPDOASSIGNMENT
{
	WORD u8SubIndex0;
	WORD awEntries[MAX_PDO_NUM];
	WORD wRsvd;
} PDOASSIGNMENT;

typedef struct tagSYNCMANPAR
{
	WORD  subindex0;
	WORD  u16SyncType;
	DWORD u32CycleTime;
	DWORD u32ShiftTime;
	WORD  u16SyncTypesSupported;
	DWORD u32MinCycleTime;
	DWORD u32CalcAndCopyTime;
} SYNCMANPAR;

typedef struct tagOBJMAPPING
{
	WORD wNum;

	WORD wODIndex[MAX_PDO_NUM * 4];  // Anders 20131018. add.
	int  nOffset[MAX_PDO_NUM * 4];   // 8->16. Anders 20110822.  Reference to RxPDO & TxPDO Mapping.
	WORD wSubIndex[MAX_PDO_NUM * 4]; // 8->16. Anders 20110822.
	WORD wSize[MAX_PDO_NUM * 4];     // 8->16. Anders 20110822.
} OBJMAPPING;
typedef struct tagPODOBJMAPPING
{
	OBJMAPPING InputMapping;
	OBJMAPPING OutputMapping;
} PODOBJMAPPING;
// Anders test.20110121.
typedef struct tagTMARRSDO_RW
{
	WORD u8SubIndex0;
	//WORD     wNoElements;
	WORD awData[TMARRSDO_RW_DATALEN + 1]; // NoElements + (ID + DBValue)*N
} TMARRSDO_RW;
typedef struct tagCANMAPPING
{
	WORD wNum;
	WORD wODIndex[4]; // Anders 20131018. add.
	int  nOffset[4];
	WORD wSubIndex[4];
	WORD wSize[4];
} CANMAPPING;

typedef struct tagConsumerTime
{
	BYTE u8SubIndex0;
	WORD ConsumerTime;
} ConsumerTime;

typedef struct tagDeviceInfo
{
	DWORD u8SubIndex0;
	DWORD dwVendorID;
	DWORD dwProductCode;
	DWORD dwRevisionNumber;
	DWORD dwSerialNumber;
} DeviceInfo;
/*===========================================================================+
|           Extern                                                           |
+===========================================================================*/
const OBJECT *           GetObjectDictHandle(WORD wIndex, int * pnIndexOfOD);
const OBJECT *           GetObjectDictHandleByIndexOfOD(WORD wIndexOfOD);
WORD                     GetObjectDictLength(BYTE bSubIndex, const OBJECT * pObjEntry, BYTE bCompleteAccess);
const SDOINFOENTRYDESC * GetObjectDictEntryDesc(const OBJECT * pObjEntry, BYTE Subindex);
WORD                     GetObjectDictEntryOffset(BYTE bSubIndex, const OBJECT * pObjEntry);

WORD ReadObjectDict(WORD wIndex, BYTE bSubIndex, const OBJECT * pObjEntry, WORD * pData, BYTE bCompleteAccess);
WORD WriteObjectDict(WORD wIndex, BYTE bSubIndex, const OBJECT * pObjEntry, WORD * pData, BYTE bCompleteAccess);

WORD GetODGuardTime();
WORD GetODLifeTimeFactor();
WORD GetODHeartbeatTime();
void ClearHeartbeatTimeAndGuardTime(WORD wHeartbeatTime, WORD GuardTime, WORD LifeTimeFactor);

void SetODHeartBeatTime();
void SetODGuardTime();
void SetODLifeTimeFactor();

void InitObjdef();

extern SYNCMANPAR    sSyncManOutPar;
extern SYNCMANPAR    sSyncManInPar;
extern PDOASSIGNMENT RxPDOAssign;
extern PDOASSIGNMENT TxPDOAssign;
extern PDOMAPING     RxPDOMaping[];
extern PDOMAPING     TxPDOMaping[];
extern CANMAPPING    RxPDOMapingEx[];
extern CANMAPPING    TxPDOMapingEx[];

//extern	const	OBJECT		asObjDef[];
extern const DWORD      dwAbortCode[];
extern PDOCOMMUNICATION RxPDOCommPar[];
extern PDOCOMMUNICATION TxPDOCommPar[];

#endif
