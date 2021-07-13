/*==============================================================================+
|  Function : CANOpenCommon                                                     |
|  Task     : CANOpenCommon Head File                                           |
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
#ifndef D__CANOPENCOMMON__H
#define D__CANOPENCOMMON__H
//------------------------------------------------------------------------------
#include "common.h"
#include "debug.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define CANOpen_GetNodeIDByCOBID(D_COBID) ((NodeID_T)((D_COBID)&0x7F))
#define CANOpen_GetFuncIDByCOBID(D_COBID) ((E_CANOpenFuncID)(((D_COBID) >> 7) & 0xF))
#define CANOpen_GetCOBID(D_FuncID, D_NodeID) ((COBID_T)((((D_FuncID)&0xF) << 7) | ((D_NodeID)&0x7F)))

#define CANOPENNODEGUARDING_TRIGGERBIT (1 << 7)

#define CANOPENDTAC_TOGGLE_BIT_NOT_ALTERNATED ((CANOpenDTAC_T)0x05030000)                     // Toggle bit not alternated.
#define CANOPENDTAC_SDO_PROTOCOL_TIMED_OUT ((CANOpenDTAC_T)0x05040000)                        // SDO protocol timed out.
#define CANOPENDTAC_CS_CMDSPECIFIER_INVALID_OR_UNKNOWN ((CANOpenDTAC_T)0x05040001)            // Client/server command specifier not valid or unknown.
#define CANOPENDTAC_INVALID_BLOCK_SIZE ((CANOpenDTAC_T)0x05040002)                            // Invalid block size (block mode only).
#define CANOPENDTAC_INVALID_SEQUENCE_NUM ((CANOpenDTAC_T)0x05040003)                          // Invalid sequence number (block mode only).
#define CANOPENDTAC_CRC_ERROR ((CANOpenDTAC_T)0x05040004)                                     // CRC error (block mode only).
#define CANOPENDTAC_OUT_OF_MEMORY ((CANOpenDTAC_T)0x05040005)                                 // Out of memory.
#define CANOPENDTAC_UNSUPPORTED_ACCESS_TO_AN_OBJECT ((CANOpenDTAC_T)0x06010000)               // Unsupported access to an object.
#define CANOPENDTAC_ATTEMPT_TO_READ_A_WRITE_ONLY_OBJECT ((CANOpenDTAC_T)0x06010001)           // Attempt to read a write only object.
#define CANOPENDTAC_ATTEMPT_TO_WRITE_A_READ_ONLY_OBJECT ((CANOpenDTAC_T)0x06010002)           // Attempt to write a read only object.
#define CANOPENDTAC_OBJECT_NOTEXIST_IN_OBJ_DICT ((CANOpenDTAC_T)0x06020000)                   // Object does not exist in the object dictionary.
#define CANOPENDTAC_OBJECT_CANNOT_BE_MAPPED_TO_THE_PDO ((CANOpenDTAC_T)0x06040041)            // Object cannot be mapped to the PDO.
#define CANOPENDTAC_NUM_AND_LEN_OF_OBJ_MAPPED_EXCEED_PDO_LEN ((CANOpenDTAC_T)0x06040042)      // The number and length of the objects to be mapped would exceed PDO length.
#define CANOPENDTAC_GENERAL_PARAMETER_INCOMPATIBILITY_REASON ((CANOpenDTAC_T)0x06040043)      // General parameter incompatibility reason.
#define CANOPENDTAC_GENERAL_INTERNAL_INCOMPATIBILITY_IN_DEVICE ((CANOpenDTAC_T)0x06040047)    // General internal incompatibility in the device.
#define CANOPENDTAC_ACCESS_FAILED_DUE_TO_AN_HARDWARE_ERROR ((CANOpenDTAC_T)0x06060000)        // Access failed due to an hardware error.
#define CANOPENDTAC_LEN_OF_SERVE_PARAMETER_NOTMATCH ((CANOpenDTAC_T)0x06070010)               // Data type does not match, length of service parameter does not match
#define CANOPENDTAC_LEN_OF_SERVE_PARAMETER_TOOHIGH ((CANOpenDTAC_T)0x06070012)                // Data type does not match, length of service parameter too high
#define CANOPENDTAC_LEN_OF_SERVE_PARAMETER_TOOLOW ((CANOpenDTAC_T)0x06070013)                 // Data type does not match, length of service parameter too low
#define CANOPENDTAC_SUBINDEX_NOTEXIST ((CANOpenDTAC_T)0x06090011)                             // Sub-index does not exist.
#define CANOPENDTAC_VALUE_RANGE_OF_PARAMETER_EXCEEDED ((CANOpenDTAC_T)0x06090030)             // Value range of parameter exceeded (only for write access).
#define CANOPENDTAC_VALUE_OF_PARAMETER_WRITTEN_TOO_HIGH ((CANOpenDTAC_T)0x06090031)           // Value of parameter written too high.
#define CANOPENDTAC_VALUE_OF_PARAMETER_WRITTEN_TOO_LOW ((CANOpenDTAC_T)0x06090032)            // Value of parameter written too low.
#define CANOPENDTAC_MAXIMUM_VALUE_IS_LESS_THAN_MINIMUM_VALUE ((CANOpenDTAC_T)0x06090036)      // Maximum value is less than minimum value.
#define CANOPENDTAC_GENERAL_ERROR ((CANOpenDTAC_T)0x08000000)                                 // general error
#define CANOPENDTAC_DATA_CANNOT_TX_OR_STOR_TO_APP ((CANOpenDTAC_T)0x08000020)                 // Data cannot be transferred or stored to the application.
#define CANOPENDTAC_DATA_CANNOT_TX_OR_STOR_TO_APP_LOCAL_CTRL ((CANOpenDTAC_T)0x08000021)      // Data cannot be transferred or stored to the application because of local control.
#define CANOPENDTAC_DATA_CANNOT_TX_OR_STOR_TO_APP_DEVICE_STATE ((CANOpenDTAC_T)0x08000022)    // Data cannot be transferred or stored to the application because of the present device state.
#define CANOPENDTAC_OBJ_DICT_DYNAMIC_GENERATE_FAIL_OR_NO_OBJ_DICT ((CANOpenDTAC_T)0x08000023) // Object dictionary dynamic generation fails or no object dictionary is present (e.g. object dictionary is generated from file and generation fails because of an file error).
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_CANOpenNMTCtrlCmd {
    CANOPENNMTCTRLCMD_START_REMOTE_NODE          = 1,
    CANOPENNMTCTRLCMD_STOP_REMOTE_NODE           = 2,
    CANOPENNMTCTRLCMD_ENTER_PREOPERATIONAL_STATE = 128,
    CANOPENNMTCTRLCMD_RESET_NODE                 = 129,
    CANOPENNMTCTRLCMD_RESET_COMMUNICATION        = 130
} E_CANOpenNMTCtrlCmd;

typedef enum Enum_CANOpenSDOCCS {
    CANOPENSDOCCS_IDD = 0x01, // Initiate Domain Download
    CANOPENSDOCCS_DDS = 0x00, // Download Domain Segment
    CANOPENSDOCCS_IDU = 0x02, // Initiate Domain Upload
    CANOPENSDOCCS_UDS = 0x03, // Upload Domain Segment
    CANOPENSDOCCS_ADT = 0x04, // Abort Domain Transfer
    CANOPENSDOCCS_IBD = 0x06, // Initiate Block Download
    CANOPENSDOCCS_EBD = 0x06  // End Block Download
} E_CANOpenSDOCCS;            // CANOpen SDO Client Cmd Specifier

typedef enum Enum_CANOpenSDOSCS {
    CANOPENSDOSCS_IDD = 0x03, // Initiate Domain Download
    CANOPENSDOSCS_DDS = 0x01, // Download Domain Segment
    CANOPENSDOSCS_IDU = 0x02, // Initiate Domain Upload
    CANOPENSDOSCS_UDS = 0x00, // Upload Domain Segment
    CANOPENSDOSCS_ADT = 0x04, // Abort Domain Transfer
    CANOPENSDOSCS_IBD = 0x05, // Initiate Block Download
    CANOPENSDOSCS_DBS = 0x05, // Download Block Segment
    CANOPENSDOSCS_EBD = 0x05  // End Block Download
} E_CANOpenSDOSCS;            // CANOpen SDO Server Cmd Specifier

typedef enum Enum_CANOpenNodeState {
    CANOPENNODESTATE_BOOTUP         = 0,
    CANOPENNODESTATE_STOPPED        = 4,
    CANOPENNODESTATE_OPERATIONAL    = 5,
    CANOPENNODESTATE_PREOPERATIONAL = 127
} E_CANOpenNodeState;

typedef enum Enum_CANOpenFuncID {
    CANOPENFUNCID_NMT_MODULE_CTRL = 0X0,
    CANOPENFUNCID_SYNC            = 0X1,
    CANOPENFUNCID_TIME_SSTAMP     = 0X2,
    CANOPENFUNCID_SEND_PDO1       = 0x3,
    CANOPENFUNCID_RECV_PDO1       = 0x4,
    CANOPENFUNCID_SEND_PDO2       = 0x5,
    CANOPENFUNCID_RECV_PDO2       = 0x6,
    CANOPENFUNCID_SEND_PDO3       = 0x7,
    CANOPENFUNCID_RECV_PDO3       = 0x8,
    CANOPENFUNCID_SEND_PDO4       = 0x9,
    CANOPENFUNCID_RECV_PDO4       = 0xA,
    CANOPENFUNCID_SEND_SDO        = 0xB,
    CANOPENFUNCID_RECV_SDO        = 0xC,
    CANOPENFUNCID_NMT_ERROR_CTRL  = 0xE
} E_CANOpenFuncID;

typedef enum Enum_CANOpenRetCode {
    CANOPENRETCODE_SUCCESS,
    CANOPENRETCODE_DRIVE_ERROR_FAILURE,
    CANOPENRETCODE_DRIVE_ERROR_RX_EMPTY,
    CANOPENRETCODE_DRIVE_ERROR_TX_FULL,
    CANOPENRETCODE_DRIVE_ERROR_RX_BUSY,
    CANOPENRETCODE_DRIVE_ERROR_TX_BUSY,
    CANOPENRETCODE_DRIVE_ERROR_RX_SMALL,

    CANOPENRETCODE_SUM
} E_CANOpenRetCode;

typedef enum Enum_CANOpenEventType {
    CANOPENEVENTTYPE_RECVED_DATAFRAME,
    CANOPENEVENTTYPE_CHANGE_NODESTATE
} E_CANOpenEventType;

typedef enum Enum_CANChannel {
    CANCHANNAL_CAN1,
    CANCHANNAL_CAN2,

    CANCHANNAL_SUM
} E_CANChannel;
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef BYTE  ODSubIndex_T;
typedef BYTE  NodeID_T;
typedef WORD  ODID_T;
typedef WORD  ODIndex_T;
typedef WORD  COBID_T;
typedef DWORD CANOpenDTAC_T;
//------------------------------------------------------------------------------
typedef union {
    BYTE tAll;
    struct
    {
        BYTE bOtherData : 5;
        BYTE bCmd : 3; // E_CANOpenSDOCCS/E_CANOpenSDOSCS
    } sBitField;
} CANOpenSDOCmdSpecifier_U;

typedef union {
    DWORD tAll;
    struct
    {
        WORD bReserve0; //              : 16;
        WORD bReserve1 : 2;
        WORD bCOBID : 11;
        WORD bReserve2 : 2;
        WORD bRTR : 1;
    } sStand;
} CANOpenDataFrameHead_U;

typedef union CANOpenDataFrameData_U {
    struct
    {
        WORD  bCmdSpecifier : 8; // CANOpenSDOCmdSpecifier_U
        WORD  bODID_L : 8;
        WORD  bODID_H : 8;
        WORD  bODSubIndex : 8; // ODSubIndex_T
        DWORD bData;           //                  : 32;
    } sSDO;
    struct
    {
        DWORD bData0; //                 : 32;
        DWORD bData1; //                 : 32;
    } sU32;
    struct
    {
        WORD bData0; //                 : 16;
        WORD bData1; //                 : 16;
        WORD bData2; //                 : 16;
        WORD bData3; //                 : 16;
    } sU16;
    struct
    {
        BYTE bData0 : 8;
        BYTE bData1 : 8;
        BYTE bData2 : 8;
        BYTE bData3 : 8;
        BYTE bData4 : 8;
        BYTE bData5 : 8;
        BYTE bData6 : 8;
        BYTE bData7 : 8;
    } sU8;
} CANOpenDataFrameData_U;

typedef struct CANOpenDataFrame_S
{
    CANOpenDataFrameHead_U uHead;
    CANOpenDataFrameData_U uData;
    BYTE                   tDataLen; // The unit is 8bit
} CANOpenDataFrame_S;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
E_CANOpenRetCode CANOpen_Open(E_CANChannel eChannel);
E_CANOpenRetCode CANOpen_Close(E_CANChannel eChannel);
void             CANOpen_RX(E_CANChannel eChannel);
void             CANOpen_TX(E_CANChannel eChannel);
E_CANOpenRetCode CANOpen_Read(E_CANChannel eChannel, CANOpenDataFrame_S * psDataFrame);
E_CANOpenRetCode CANOpen_Write(E_CANChannel eChannel, CANOpenDataFrame_S * psDataFrame);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
#if (DEBUG)
extern DWORD g_aadwCANStateCount[CANCHANNAL_SUM][CANOPENRETCODE_SUM];
#endif
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
