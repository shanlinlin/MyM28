/*==============================================================================+
|  Function : database                                                          |
|  Task     : database Header File                                              |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2015/07/30                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__DATABASE__H
#define D__DATABASE__H
//------------------------------------------------------------------------------
#include "common.h"
#include "debug.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define DB_MakeError(/*DB_DeviceID_E*/ D_eDeviceID, D_ObjNum, D_ErrorCode) (((DWORD)((D_eDeviceID)&0xFF) << 24) | ((DWORD)((D_ObjNum)&0xFF) << 16) | (DWORD)((D_ErrorCode)&0xFFFF))
//------------------------------------------------------------------------------
// Compatible with the processing
#define ReadDB_WORD(D_tDBIndex) DB_GetWORD(D_tDBIndex)
#define ReadDB_INT(D_tDBIndex) DB_GetINT(D_tDBIndex)
#define ReadDB_DWORD(D_tDBIndex) DB_GetDWORD(D_tDBIndex)
#define ReadDB_LONG(D_tDBIndex) DB_GetLONG(D_tDBIndex)
#define ReadDB_FLOAT(D_tDBIndex) DB_GetFLOAT(D_tDBIndex)
#define WriteDB_WORD(D_tDBIndex, D_tValue) DB_SetWORD(D_tDBIndex, D_tValue)
#define WriteDB_DWORD(D_tDBIndex, D_tValue) DB_SetDWORD(D_tDBIndex, D_tValue)
#define WriteDB_LONG(D_tDBIndex, D_tValue) DB_SetLONG(D_tDBIndex, D_tValue)
#define WriteDB_FLOAT(D_tDBIndex, D_tValue) DB_SetFLOAT(D_tDBIndex, D_tValue)

#define SetDBValue(D_tDBID, D_dbValue) SetDBValueByIndex(DB_IDToIndex(D_tDBID), D_dbValue)
#define GetDBValue(D_tDBID) GetDBValueByIndex(DB_IDToIndex(D_tDBID))
#define DataIDToIndex(D_tDBID) DB_IDToIndex(D_tDBID)
#define IndexToDataID(D_tDBIndex) DB_IndexToID(D_tDBIndex)
//------------------------------------------------------------------------------
// Compatible with the processing
#define DB_SUCCESS 1
#define DB_ERR_OUTRANGE 2 // out of range
#define DB_ERR_DATA 3     // data error
#define DB_ERR_TYPE 4     // data type error
#define DB_ERR_READONLY 5 // Access Readonly Data. Anders 20110707.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_DataAccess {
    DATAACCESS_RO,
    DATAACCESS_WO,
    DATAACCESS_RW,

    DATAACCESS_SUM
} E_DataAccess;

typedef enum Enum_DBEvent {
    //DBEVENT_READ_BEFORE,                                                                              // Reading before  the callback event
    DBEVENT_READ_AFTER, // Reading after the callback event
    //DBEVENT_WRITE_BEFORE,                                                                             // Writing before the callback event
    DBEVENT_WRITE_AFTER, // Writing after the callback event

    DBEVENT_SUM
} E_DBEvent;

typedef enum Enum_DBRetCode {
    DBRETCODE_SUCCESS, // Database operation is successful

    DBRETCODE_ACCESS_READ_PERMISSION_DENIED,  // The data are not allowed to read
    DBRETCODE_ACCESS_WRITE_PERMISSION_DENIED, // The data are not allowed to write
    DBRETCODE_ACCESS_WRITE_OUTRANGE,          // Data write error: beyond the scope
    DBRETCODE_ACCESS_WRITE_TOOSMALL,          // Write error: data is too small
    DBRETCODE_ACCESS_WRITE_TOOLARGE,          // Write error: data is too large
    DBRETCODE_ACCESS_DATA_NOTEXSIT,           // There is no data
    DBRETCODE_ACCESS_ERR_DATA,                // Data error

    DBRETCODE_OTHER // Other error data operation, can use their feedback data error value (note: the enumeration must be defined in the final)
} E_DBRetCode;
//------------------------------------------------------------------------------
typedef enum Enum_DBDeviceID {
    DBDEVICEID_MASTER           = 0x00,
    DBDEVICEID_AXISCARD1        = 0x01,
    DBDEVICEID_AXISCARD2        = 0x02,
    DBDEVICEID_AXISCARD3        = 0x03,
    DBDEVICEID_AXISCARD4        = 0x04,
    DBDEVICEID_AXISCARD5        = 0x05,
    DBDEVICEID_REMOTEIO1        = 0x21,
    DBDEVICEID_REMOTEIO2        = 0x22,
    DBDEVICEID_REMOTEIO3        = 0x23,
    DBDEVICEID_REMOTEIO4        = 0x24,
    DBDEVICEID_REMOTEIO5        = 0x25,
    DBDEVICEID_REMOTEIO6        = 0x26,
    DBDEVICEID_REMOTEIO7        = 0x27,
    DBDEVICEID_REMOTEIO8        = 0x28,
    DBDEVICEID_REMOTEIO9        = 0x29,
    DBDEVICEID_REMOTEIO10       = 0x2A,
    DBDEVICEID_REMOTEIO11       = 0x2B,
    DBDEVICEID_REMOTEIO12       = 0x2C,
    DBDEVICEID_REMOTEIO13       = 0x2D,
    DBDEVICEID_TEMPERATURECARD1 = 0x40,
    DBDEVICEID_TEMPERATURECARD2 = 0x41,
    DBDEVICEID_TEMPERATURECARD3 = 0x42,
    DBDEVICEID_TEMPERATURECARD4 = 0x43,
    DBDEVICEID_TEMPERATURECARD5 = 0x44,
    DBDEVICEID_TEMPERATURECARD6 = 0x45,
    DBDEVICEID_TEMPERATURECARD7 = 0x46,
    DBDEVICEID_TEMPERATURECARD8 = 0x47,
    DBDEVICEID_POWERMETERCARD1  = 0x50,
    DBDEVICEID_POWERMETERCARD2  = 0x51,
    DBDEVICEID_POWERMETERCARD3  = 0x52,
    DBDEVICEID_PICKER0          = 0x60
} E_DBDeviceID;

typedef enum Enum_DBDataType {
    DBDATATYPE_UNKNOWN = 0,
    DBDATATYPE_BOOL    = 1,
    DBDATATYPE_BYTE    = 2,
    DBDATATYPE_WORD    = 3,
    DBDATATYPE_DWORD   = 4,
    DBDATATYPE_SINT    = 11,
    DBDATATYPE_USINT   = 12,
    DBDATATYPE_INT     = 13,
    DBDATATYPE_UINT    = 14,
    DBDATATYPE_LONG    = 15,
    DBDATATYPE_UDINT   = 16,
    DBDATATYPE_FLOAT   = 21
} E_DBDataType;

#define DATABASE_ENUMERATION_H
#include ".database.h"
#undef DATABASE_ENUMERATION_H
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef DWORD DBID_T;
typedef WORD  DBIndex_T;

typedef struct DBAttrib_S
{
    DWORD dwIDOffset;
    WORD  bDataOffset : 14;
    WORD  bAccess : 2;   // E_DataAccess
    WORD  bDataType : 8; // E_DataType
    WORD  bCallback : 8;
} DBAttrib_S;

typedef struct DBGroup_S
{
    DBAttrib_S const* pasAttrib;
    void*             pcBaseAddr;
    DWORD             dwIDBase;
} DBGroup_S;

typedef struct DBAttribInfo_S
{
    // WORD bGroupIndex : 6; // E_DBGroup
    // WORD bAttribIndex : 10;
    //SHANLL 20191224 Modify
    DWORD bGroupIndex : 6; // E_DBGroup
    DWORD bAttribIndex : 26;
} DBAttribInfo_S;

#define DATABASE_TYPE_DEFINITION_H
#include ".database.h"
#undef DATABASE_TYPE_DEFINITION_H
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
#define Init_Version() (WriteDB_DWORD(DBINDEX_M28_MONITOR_VERSION_DATE_TIME_HEX, VERSION_DATETIME_HEX), WriteDB_DWORD(DBINDEX_M28_MONITOR_VERSION_DATE_TIME_CHAR, VERSION_DATETIME))

void Init_DB(void);
void Run_DB(void);

DBIndex_T DB_GetIndex(DBID_T tID, DBIndex_T tMinIndex, DBIndex_T tMaxIndex);
DBIndex_T DB_IDToIndex(DBID_T tID);
DBID_T    DB_IndexToID(DBIndex_T tIndex);
WORD      DB_SizeofBData(DBIndex_T tIndex);

inline DBGroup_S const*  DB_GetGroup(DBIndex_T tIndex);
inline DBAttrib_S const* DB_GetAttrib(DBGroup_S const* const psGroup, DBIndex_T tIndex);
inline E_DataAccess      DB_GetAccess(DBIndex_T tIndex);
inline E_DataType        DB_GetDataType(DBIndex_T tIndex);
inline E_DBDataType      DB_GetDBDataType(DBIndex_T tIndex);
inline char*             DB_GetDataAddr(DBIndex_T tIndex);

void DB_SetData(DBIndex_T tIndex, void const* pvData);
void DB_GetData(DBIndex_T tIndex, void* pvData);

E_DBRetCode DB_CallCallBack(DBIndex_T tIndex, E_DBEvent eEvent, void const* pvData);
E_DBRetCode DB_SetDataBySysIndex(DBIndex_T tIndex, void const* pvData);  // Exec Even.
E_DBRetCode DB_GetDataBySysIndex(DBIndex_T tIndex, void* pvData);        // Exec Even.
E_DBRetCode DB_SetDataByUserIndex(DBIndex_T tIndex, void const* pvData); // Exec Even.
E_DBRetCode DB_GetDataByUserIndex(DBIndex_T tIndex, void* pvData);       // Exec Even.
//------------------------------------------------------------------------------
// Compatible with the processing
WORD           SetDBValueByIndex(DBIndex_T tDBIndex, DBVALUE dbValue);
inline DBVALUE GetDBValueByIndex(DBIndex_T tDBIndex);

#define DATABASE_FUNCTION_DECLARATION_H
#include ".database.h"
#undef DATABASE_FUNCTION_DECLARATION_H
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern S_DB                 g_sDB;
extern DBGroup_S const      gc_asGroup[];
extern DBAttribInfo_S const gc_asAttribInfo[];
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [DB_GetGroup description]
 * @param  tIndex [description]
 * @return        [description]
 */
inline DBGroup_S const* DB_GetGroup(DBIndex_T tIndex)
{
    DEBUG_ASSERT(tIndex < DBINDEX_SUM);
    DEBUG_ASSERT(gc_asAttribInfo[tIndex].bGroupIndex < DBGROUP_SUM);
    return &gc_asGroup[gc_asAttribInfo[tIndex].bGroupIndex];
}
/**
 * [DB_GetAttrib description]
 * @param  psGroup [description]
 * @param  tIndex  [description]
 * @return         [description]
 */
inline DBAttrib_S const* DB_GetAttrib(DBGroup_S const* const psGroup, DBIndex_T tIndex)
{
    DEBUG_ASSERT(NULL != psGroup);
    DEBUG_ASSERT(NULL != psGroup->pasAttrib);
    DEBUG_ASSERT(tIndex < DBINDEX_SUM);
    return &psGroup->pasAttrib[gc_asAttribInfo[tIndex].bAttribIndex];
}
/**
 * [DB_GetAccess description]
 * @param  tIndex [description]
 * @return        [description]
 */
inline E_DataAccess DB_GetAccess(DBIndex_T tIndex)
{
    return (E_DataAccess)DB_GetAttrib(DB_GetGroup(tIndex), tIndex)->bAccess;
}
/**
 * [DB_GetDataType description]
 * @param  tIndex [description]
 * @return        [description]
 */
inline E_DataType DB_GetDataType(DBIndex_T tIndex)
{
    DBAttrib_S const* psAttrib;

    psAttrib = DB_GetAttrib(DB_GetGroup(tIndex), tIndex);
    DEBUG_ASSERT(NULL != psAttrib);
    return (E_DataType)psAttrib->bDataType;
}
/**
 * [DB_GetDBDataType description]
 * @param  tIndex [description]
 * @return        [description]
 */
inline E_DBDataType DB_GetDBDataType(DBIndex_T tIndex)
{
    DBAttrib_S const* psAttrib;

    if (tIndex >= DBINDEX_SUM)
        return DBDATATYPE_UNKNOWN;
    psAttrib = DB_GetAttrib(DB_GetGroup(tIndex), tIndex);
    DEBUG_ASSERT(NULL != psAttrib);
    switch (psAttrib->bDataType)
    {
        case DATATYPE_BYTE:
            return DBDATATYPE_BYTE;
        //case DATATYPE_INT:
        //    return DBDATATYPE_INT;
        case DATATYPE_WORD:
            return DBDATATYPE_WORD;
        case DATATYPE_DWORD:
            return DBDATATYPE_DWORD;
        case DATATYPE_LONG:
            return DBDATATYPE_LONG;
        case DATATYPE_FLOAT:
            return DBDATATYPE_FLOAT;
        default:
            DEBUG_ASSERT_WARN(false, "data type not define.");
            return DBDATATYPE_UNKNOWN;
    }
}
/**
 * [DB_GetDataAddr description]
 * @param  tIndex [description]
 * @return        [description]
 */
inline char* DB_GetDataAddr(DBIndex_T tIndex)
{
    DBGroup_S const*  psGroup;
    DBAttrib_S const* psAttrib;

    psGroup = DB_GetGroup(tIndex);
    DEBUG_ASSERT(NULL != psGroup);
    psAttrib = DB_GetAttrib(psGroup, tIndex);
    DEBUG_ASSERT(NULL != psAttrib);
    return (char*)psGroup->pcBaseAddr + psAttrib->bDataOffset;
}
/**
 * [GetDBValueByIndex description]
 * @param  tDBIndex [description]
 * @return          [description]
 */
inline DBVALUE GetDBValueByIndex(DBIndex_T tDBIndex)
{
    DBVALUE dbValue;

    if (DBRETCODE_SUCCESS != DB_GetDataByUserIndex(tDBIndex, &dbValue))
        dbValue.vDWORD = 0;
    return dbValue;
}

#define DATABASE_FUNCTION_PROTOTYPE_H
#include ".database.h"
#undef DATABASE_FUNCTION_PROTOTYPE_H
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
