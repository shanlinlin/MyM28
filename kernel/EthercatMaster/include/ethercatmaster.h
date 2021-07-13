/*==========================================================================+
|  Function : EtherCAT commnunication                                       |
|  Task     : EtherCAT commnunication header                                |
|---------------------------------------------------------------------------|
|  Compile  :  ccs3.1                                                       |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   :                                                               |
|  Version  : V1.00                                                         |
|  Creation : 2009/10/13                                                    |
|  Revision : 1.0                                                           |
+==========================================================================*/

#ifndef D__ETHERCATMASTER_H
#define D__ETHERCATMASTER_H

#include "common.h"
#include "comp_fifo.h"
#include "ethernet_switch.h"

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define ethercat_hton16(data_) (data_)
#define ethercat_hton32(data_) (data_)
#define ethercat_ntoh16(data_) ethercat_hton16(data_)
#define ethercat_ntoh32(data_) ethercat_hton32(data_)

//#define CONST_LENGTH_MAC (6 / 2)        // netcard physical address length
//#define CONST_LENGTH_ETHERHEAD (14 / 2) // ethernet header length

#define ETHERCATMASTER_ERROR_NO_ECHO 0x0001
#define ETHERCATMASTER_ERROR_SLAVE_OFFLINE 0x0101
#define ETHERCATMASTER_ERROR_SLAVE_AL_ERROR 0x0111

//length
#define CONST_LENGTH_ECATHEAD 2 / 2           // Ecat header length
#define CONST_LENGTH_DATAGRAMHEAD 10 / 2      // datagram header length
#define CONST_LENGTH_MAILBOXHEAD 6 / 2        // mailbox header length
#define CONST_LENGTH_HEADER (14 + 2 + 10) / 2 // protocol header length
//Data cache length
#define CONST_SIZE_ETHERCATPACKET 1514 / 2 // size of one EtherCAT packet
#define CONST_SIZE_ETHERCATDATA 1486 / 2   // size of one EtherCAT packet data

#define CONST_ETHERCAT_EOE_BLOCK_BYTE 32

#define SIZE_ECDATAGRAMHEADER_BYTE (sizeof(ECDATAGRAMHEADER) / sizeof(WORD) * 2)
#define SIZE_ECDATAGRAMHEADER_WORD (sizeof(ECDATAGRAMHEADER) / sizeof(WORD))
#define SIZE_SYNC_MANAGER_WORD (sizeof(SYNC_MANAGER) / sizeof(WORD))
#define SIZE_FMMU_WORD (sizeof(FMMU) / sizeof(WORD))
#define SIZEOF_SDOHDR_WORD (sizeof(SDOHR) / sizeof(WORD))
#define SIZEOF_MBHDER_WORD (sizeof(MBXHR) / sizeof(WORD))
#define SIZEOF_COEHDER_WORD (sizeof(COEHR) / sizeof(WORD))
#define SIZEOF_ETHNETFRAME_WORD (sizeof(ETHNETFRAME) / sizeof(WORD))

#define DRIVE_POSITION_CONTROL_MODE 0x8
#define DRIVE_VELOCITY_CONTROL_MODE 0x9

#define RXBUFFER_ETHERCATDATAGRAMS (1498 / 2)

#define TXDATA_ETHERCATDATAGRAMS (RXBUFFER_ETHERCATDATAGRAMS - SIZE_ECDATAGRAMHEADER_WORD) // 744
/**
 * EtherCAT Slaves Max Count.
 */
#define INITSDOCFG_MAX 5       // Init SDO Config Data Count.
#define PDOMAPING_MAX 8        // POD Maping Count.
#define PDOMAPPINGPARAM_MAX 12 // index0,...n,index0. 1~8Bytes.
/**
 * ENUM_ECCFGTYPE
 */
//#define   ECCFGTYPE_RXPDOCOMMUPARAMS,         // Config RxPDO Communication Params.
//#define   ECCFGTYPE_TXPDOCOMMUPARAMS,         // Config TxPDO Communication Params.

#define ECCFGTYPE_INITSDOCONFIG 0        // Init SDO Config.
#define ECCFGTYPE_RXPDOMAPPINGPARAMS 1   // Config RxPDO Mapping Params.
#define ECCFGTYPE_TXPDOMAPPINGPARAMS 2   // Config TxPDO Mapping Params.
#define ECCFGTYPE_SMASSINGRXPDOMAPPING 3 // Config SM Assign RxPDO Mapping.
#define ECCFGTYPE_SMASSINGTXPDOMAPPING 4 // Config SM Assign TxPDO Mapping.
#define ECCFGTYPE_END 5
/**
  * ENUM_UPLOADSDORESULT
  */
#define SDORESULT_NONE 0
#define SDORESULT_BUSY 1
#define SDORESULT_DONE 2
#define SDORESULT_TIMEOUT 3
#define SDORESULT_SLAVEABORT 4

/**
  * EtherCAT Frame Header'Type
  */
#define ECFRAMETYPE_RESVD1 0
#define ECFRAMETYPE_ECATDATAGRAM 1
#define ECFRAMETYPE_RESVD2 2
#define ECFRAMETYPE_RESVD3 3
#define ECFRAMETYPE_NETWORDVARIABLES 4
#define ECFRAMETYPE_MAILBOXOVERIP 5
#define ECFRAMETYPE_RESVDFORFUTURE 6

#define ECSEND_INC (++cthis->m_bySendECCount >= 0xFF ? (cthis->m_bySendECCount = 1) : cthis->m_bySendECCount)
#define SIZE_ETHNETFRAMEHDR_BYTE ((CONST_LENGTH_ETHERHEAD + CONST_LENGTH_ECATHEAD) << 1)

#define ETHERCAT_RECEIVE_TIMEOUT 20
#define ETHERCAT_CONFIG_DELAY_COUNT 10

#define SDOSTATE_NONE 0
#define SDOSTATE_UPLOAD_RDMBOX 1
#define SDOSTATE_DOWNLOAD_RDMBOX 2

FIFO_MANAGER(WORD, WORD);
/*---------------------------------------------------------------------------+
|           Type Definition                                                  |
+---------------------------------------------------------------------------*/
struct TECMaster;
typedef int (*EtherCAT_ReadNet)(int handle, void* data, unsigned length);
typedef int (*EtherCAT_WriteNet)(int handle, void const* data, unsigned length);
/**
 * EtherCAT Master Controller. Anders 20110908.
 */
typedef enum {
    EC_DATAGRAM_NONE = 0x00, /**< Dummy. */
    EC_DATAGRAM_APRD = 0x01, /**< Auto Increment Physical Read. */
    EC_DATAGRAM_APWR = 0x02, /**< Auto Increment Physical Write. */
    EC_DATAGRAM_APRW = 0x03, /**< Auto Increment Physical ReadWrite. */
    EC_DATAGRAM_FPRD = 0x04, /**< Configured Address Physical Read. */
    EC_DATAGRAM_FPWR = 0x05, /**< Configured Address Physical Write. */
    EC_DATAGRAM_FPRW = 0x06, /**< Configured Address Physical ReadWrite. */
    EC_DATAGRAM_BRD  = 0x07, /**< Broadcast Read. */
    EC_DATAGRAM_BWR  = 0x08, /**< Broadcast Write. */
    EC_DATAGRAM_BRW  = 0x09, /**< Broadcast ReadWrite. */
    EC_DATAGRAM_LRD  = 0x0A, /**< Logical Read. */
    EC_DATAGRAM_LWR  = 0x0B, /**< Logical Write. */
    EC_DATAGRAM_LRW  = 0x0C, /**< Logical ReadWrite. */
    EC_DATAGRAM_ARMW = 0x0D, /**< Auto Increment Physical Read Multiple Write.  */
    EC_DATAGRAM_FRMW = 0x0E  /**< Configured Address Physical Read Multiple Write. */
} ec_datagram_type_t;

typedef enum {
    INIT_STATE_EX           = 0x01, // Init state
    BOOTSTRAP_STATE_EX      = 0x02, // Bootstrap state (optional)
    PRE_OPERATION_STATE_EX  = 0x03, // Pre-Operational state
    SAFE_OPERATION_STATE_EX = 0x04, // SAFE-Operational state
    OPERATION_STATE_EX      = 0x08, // Operational state
    ETHCRCAT_CONFIG_SUCCESS = 0x0F  // config success
} ec_datagram_state_t;
typedef enum {
    DATAGRAM_NONE = 0x00,
    DATAGRAM_SEND = 0x01,
    DATAGRAM_RECV = 0x02
} DATAGRAMSTATE;

typedef struct tagECATLOCALTIME {
    DWORD LocalTimeLow;
    DWORD LocalTimeHigh;
} ECATLOCALTIME;
typedef struct tagSYNC_MANAGER {
    WORD PhysicalStartAddress;
    WORD Length;
    struct
    {
        WORD Control : 8;
        WORD Status : 8;
    } b1;
    struct
    {
        WORD request : 8;
        WORD response : 8;
    } b2;
} SYNC_MANAGER;
typedef struct tagFMMU {
    WORD LogicalStartAddL;
    WORD LogicalStartAddH;
    WORD Length;
    struct
    {
        WORD LogicalStartBit : 8;
        WORD LogicalEndBit : 8;
    } b1;
    WORD PhysicalStartAddress;
    struct
    {
        WORD PhysicalStartBit : 8;
        WORD ReadEnable : 1;
        WORD WriteEnable : 1;
        WORD Reserved4 : 6;
    } b2;
    WORD EnableBit;
    WORD Reserved7;
} FMMU;

typedef struct tagMBXHR {
    WORD Length;  //Length of following data
    WORD Address; //Station Address of originator
    WORD Reserved : 8;
    WORD Type : 4;
    WORD Counter : 4;
} MBXHR;

typedef struct EOEHR {
    WORD FrameType : 4;
    WORD Port : 4;
    WORD LastFragment : 1;
    WORD TimeAppended : 1;
    WORD TimeRequest : 1;
    WORD Reserved : 5;
    WORD FragmentNumber : 6;
    WORD Offset : 6; // 缓存大小.
    WORD FrameNumber : 4;
} EOEHR;

typedef struct tagCOEHR {
    WORD Number : 9;
    WORD Reserved : 3;
    WORD Service : 4;
} COEHR;

typedef struct tagSDOHR {
    WORD SizeIndicator : 1; //whether size of Data in Data Set Size specified
    WORD TransferType : 1;  //whether Expedited transfer
    WORD DataSetSize : 2;   //Data Set Size
    WORD CompleteAccess : 1;
    WORD Command : 3;
    WORD IndexLB : 8;
    WORD IndexHB : 8;
    WORD SubIndex : 8;
} SDOHR;

typedef struct tagPDOMAP {
    struct //SDO Index
    {
        WORD IndexLB : 8;
        WORD IndexHB : 8;
    } b1;
    struct
    {
        WORD SubIndex : 8;
        WORD DataSetSize : 8;
    } b2;
    WORD wData1; //PDO Data
    WORD wData2;
} PDOMAP;

typedef struct tagECDATAGRAMHEADER {
    struct
    {
        WORD Command : 8;
        WORD Index : 8;
    } b1;
    WORD Address;
    WORD Offset;
    struct
    {
        WORD Length : 11;
        WORD Reserved : 3;
        WORD Circulate : 1;
        WORD More : 1;
    } b2;
    WORD Irq;
} ECDATAGRAMHEADER;
typedef struct tagTDLCHR {
    BYTEX acDstMAC[CONST_LENGTH_MAC];
    BYTEX acSrcMAC[CONST_LENGTH_MAC];
    WORD  wProType;
} TDLCHR;

typedef struct tagETHNETFRAME {
    TDLCHR dlchr;
    struct
    {
        WORD Length : 11;
        WORD Reserved : 1;
        WORD Type : 4; // 1: Communication with Slaves. Other Values is Reserved.
    } ECFrameHeader;
    ECDATAGRAMHEADER ECDatagramHeader;
} ETHNETFRAME;
typedef struct tagETHNETDATAFRAME_EC {
    ETHNETFRAME header;                                // Dataram header 10byte.
    WORD        awEcatTxBuf[TXDATA_ETHERCATDATAGRAMS]; // Datag.header 10 byte. Data Max 1486 bytes, WKC 2 bytes.
} ETHNETDATAFRAME_EC;
typedef struct tagTDatagramState {
    WORD* pwRecvData;
    WORD  wDatagramState;
    WORD  wLength;
} TDatagramState;

typedef struct CONFIGDATAGRAM {
    WORD* pwRecvData;
    WORD  wDatagramState;
    WORD  wLength;

    WORD wSlaveCount; // Actual Slave Count.
    WORD wCurConfigSlave;
    WORD wCurrConfigType;  // Enum ENUM_ECCFGTYPE.Anders 20110910.
    INT  nCurrCfgIndex;    // Anders 20110910.
    INT  nCurrCfgSubIndex; // Anders 20110910.
    WORD wTxCommand;       // Anders 2017-5-19, add.

    WORD wLastOdIndex;
    WORD wLastOdSubIndex;

    TBOOL bInitRegsterFinished;

    Ttimer lTimeOut;
    void (*pStateFunc)(struct TECMaster* const cthis);
} ConfigDatagram_T;

typedef struct tagECATCONFIG {
    WORD wSalveName;
    WORD wSyncMailbox;
    WORD wSyncMailboxNum;
    WORD wSyncProcessData;
    WORD wSyncProcessDataNum;
    WORD wFmmu;
    WORD wFmmuNum;
} ECATCONFIG;

/*===========================================================================+
|           Type Definition2                                                 |
+===========================================================================*/
/**
 * Sync Mailbox. Anders 20110908.
 */
typedef struct tagTSMCfg {
    WORD wStartAddress;    // Physical Start Address.
    WORD wDefaultByteSize; // Size of Byte.
    WORD _wActualBytes;
    WORD byControlByte : 8; // Control Register.
    WORD bEnable : 8;       // Activate SyncManager.
    WORD wSmNO;             // 2019-6-6,add.
} TSMCfg;
/**
 * PDO Communication Params. Anders 20110908.
 */
typedef struct tagTPDOCommuParamCfg {
    WORD wIndex;

    WORD byTransType; // Transmission Type. Sub-Index=2.UNSIGNED8.
    WORD wEventTimer; // Event Timer.  Sub-Index=3.UNSIGNED16.
} TPDOCommuParamCfg;
/**
 * PDO Mapping Params. Anders 20110908.
 */
typedef void (*PFC_AFTERCOMMUFUNC)(void*);
typedef struct tagTPDOMappedInfo {
    // BYTE byDataType : 8; // Data Type of mapped objects in PDO.
    // BYTE byRsvd : 8;

    //WORD wSubIndex; // hankin 20190610 mask.
    //WORD  wMappingBitSize;            // Mapping Bit Size.
    DWORD dwMappedData;    // 1...0x40 object to be mapped. (64dw).
    void* pMappedVariable; // Mapped Variable.

    PFC_AFTERCOMMUFUNC pAfterCommuFunc;
    void*              pAfterCommuFuncParam; // After Tx or Rx, AfterCommuFunc can be call back.
} TPDOMappedInfo;

typedef struct tagTPDOMappingParamCfg {
    TBOOL          bFixed : 8; // Fixed PDO Mapping Flag.
    BYTE           bySmNO : 8; // Sm NO. of mapped objects in PDO.
    WORD           wIndex;
    WORD           wCount;                    // Mapped Count
    TPDOMappedInfo Info[PDOMAPPINGPARAM_MAX]; // Info For Application Parse. Info[0] & Info[Last]:Subindex0(Reserved).
} TPDOMappingParamCfg;
/**
 * Init SDO Config . Anders 20111021. 
 */
typedef struct tagTInitSDOCfg {
    WORD  wIndex;
    WORD  wSubIndex;
    WORD  wDataBitSize;
    DWORD dwData;
} TInitSDOCfg;
/**
 * Register: AL Status (0x0130:0x0131). Anders 20110908.
 */
typedef union tagTALStatus {
    WORD wAll;
    struct
    {
        WORD bState : 4; // Actual State of the Device State Machine;
        WORD bErrorInd : 1;
        WORD bReserved1 : 3;
        WORD bReserved2 : 8;
    } bit;
} TALStatus;
/**
 * SubIndex + DataSetSize
 */
typedef struct tagTSDOConfigData {
    WORD SubIndex : 8;
    WORD DataSetSize : 8; // 4-n  byte.
} TSDOConfigData;
/**
 * SM Assign
 */
typedef struct tagTSmAssign {
    WORD wSM;
    INT  nCount;    // PDO Mapped Count.
    WORD wDataWLen; // Valid Data Len. WORD.
    INT  nMapGrouupCount;
    struct
    {
        WORD wPDOMapIndex;
    } MapGrouup[PDOMAPING_MAX];
    struct
    {
        WORD wPDOMapIndex : 8;
        WORD wPDOMapItemIndex : 8;
        WORD wMappedDataWSize;                    // WORD
        WORD wLogicalOffsetAddr;                  // WORD
    } Items[PDOMAPING_MAX * PDOMAPPINGPARAM_MAX]; // SubIndex0(2)

    WORD awData[PDOMAPING_MAX * PDOMAPPINGPARAM_MAX * 2]; // 2CNT Rx|Tx.
} TSmAssign;
/**
 * 
 * 
 * @author anders (2020/4/16)
 */

typedef enum MAILBOX_COE_STEP {
    MAILBOX_COE_STEP_IDLE,
    MAILBOX_COE_STEP_START,

    MAILBOX_COE_STEP_UPLOAD_START,
    MAILBOX_COE_STEP_UPLOAD_LOAD,
    MAILBOX_COE_STEP_UPLOAD_SEGMENT,
    MAILBOX_COE_STEP_UPLOAD_DONE,
    MAILBOX_COE_STEP_UPLOAD_TIMEOUT,
    MAILBOX_COE_STEP_UPLOAD_SLAVEABORT,

    MAILBOX_COE_STEP_DOWNLOAD_START,
    MAILBOX_COE_STEP_DOWNLOAD_LOAD,
    MAILBOX_COE_STEP_DOWNLOAD_SEGMENT,
    MAILBOX_COE_STEP_DOWNLOAD_DONE,
    MAILBOX_COE_STEP_DOWNLOAD_TIMEOUT,
    MAILBOX_COE_STEP_DOWNLOAD_SLAVEABORT,

    MAILBOX_COE_STEP_SUM
} MAILBOX_COE_STEP;

typedef enum ETHERCAT_SDO_ACCESS {
    ETHERCAT_SDO_ACCESS_EXPEDITED,
    ETHERCAT_SDO_ACCESS_NORMAL,
    ETHERCAT_SDO_ACCESS_COMLATE,

    ETHERCAT_SDO_ACCESS_SUM
} ETHERCAT_SDO_ACCESS;

typedef struct TECSlave_CoE {
    MAILBOX_COE_STEP m_SdoStep;
    WORD             m_wSdoRespBytes;
    TBOOL            m_bSdoToggle;

    WORD                m_wSdoIndex;
    WORD                m_wSdoSubIndex;
    WORD                m_wSdoByteSize;
    ETHERCAT_SDO_ACCESS m_eSdoAccess;
    WORD*               m_pSdoBuffer;

    TtimerEx m_SdoTimer;
} TECSlave_CoE;

typedef enum {
    EOE_IP_ASSIGNMENT_WAY_DHCP,
    EOE_IP_ASSIGNMENT_WAY_STATICIP
} EOE_IP_ASSIGNMENT_WAY;

typedef struct ethercat_s_net_block {
    WORD buffer[16];
} ethercat_s_net_block;

typedef struct ethercat_s_net_buffer {
    ethercat_s_net_block* _block;
    fifo_manager_t_WORD   _fifo;
    TBOOL                 _write;
    WORD                  total;
    WORD                  length;
    WORD                  buffer[(ETHERNET_FRAME_BYTE + 1) / 2];
} ethercat_s_net_buffer;

typedef struct TECSlave_EoE_IpCfg {
    ethernet_s_ip4 IPAddress;
    ethernet_s_ip4 SubnetMask;
    ethernet_s_ip4 DefaultGateway;
    ethernet_s_ip4 DnsServer;
} TECSlave_EoE_IpCfg;

typedef struct TECSlave_EoE_IpParameter {
    EOE_IP_ASSIGNMENT_WAY AssignmentWay;
    TECSlave_EoE_IpCfg    IpCfg;
    WORD                  acDnsName[16];
} TECSlave_EoE_IpParameter;
typedef struct TECSlave_EoE_ConfigFlag {
    TBOOL bEnable;
    TBOOL MacAddrSetted;
    TBOOL IpAddrSetted;
    TBOOL SubnetMaskSetted;
    TBOOL DefaultGatewaySetted;
    TBOOL DnsServerSetted;
    TBOOL DnsNameSetted;

    TBOOL bInitTimeStampAppended;
    TBOOL bInitTimeStampRequested;
    TBOOL bFragmentTimeStampAppended;
    TBOOL bFragmentTimeStampRequested;
} TECSlave_EoE_ConfigFlag;
typedef struct TECSlave_EoE {
    ethercat_s_net_buffer* rx_buffer;
    WORD                   _rx_frame : 4;
    WORD                   rx_interval_ms;
    TtimerEx               _rx_timer;

    ethercat_s_net_buffer* tx_buffer;
    WORD                   _tx_frame : 4;
    WORD                   _tx_fragment : 6;
    WORD                   _tx_block : 6;
    WORD                   tx_interval_ms;
    TtimerEx               _tx_timer;

    TECSlave_EoE_ConfigFlag  ConfigFlag;
    ethernet_s_mac           VirtualMAC;
    TECSlave_EoE_IpParameter IpParameter;
    ethernet_switch_s_port   switch_port;
} TECSlave_EoE;

typedef struct TECSlave_Mailbox {
    TBOOL bReq;

    WORD wServerCount;
    WORD wRecvedLen;

    TECSlave_CoE         CoE;
    TECSlave_EoE         EoE;
    struct TECSlave_FoE* FoE;
} TECSlave_Mailbox;

/**
 * EtherCAT Slave Config. Anders 20110908.
 *
 * ALStatusCodes:
 * Code     Description                             Current state(or state change)      Resulting state
 * 0x0000   No error                                Any                                 Current state
 * 0x0001   Unspecified error                       Any                                 Any + E
 * 0x0011   Invalid requested state change          I?~S, I?~O, P?~O O?~B, S?~B, P?~B   Current state + E
 * 0x0012   Unknown requested state                 Any                                 Current state + E
 * 0x0013   Bootstrap not supported                 I?~B                                I + E
 * 0x0014   No valid firmware                       I?~P                                I + E
 * 0x0015   Invalid mailbox configuration           I?~B                                I + E
 * 0x0016   Invalid mailbox configuration           I?~P                                I + E
 * 0x0017   Invalid sync manager configuration      P?~S, S?~O                          Current state + E
 * 0x0018   No valid inputs available               O, S, P?~S                          P + E
 * 0x0019   No valid outputs                        O, S?~O                             S + E
 * 0x001A   Synchronization error                   O, S?~O                             S + E
 * 0x001B   Sync manager watchdog                   O, S                                S + E
 * 0x001C   Invalid Sync Manager Types              O, S P?~S                           S + E P + E
 * 0x001D   Invalid Output Configuration            O, S P?~S                           S + E P + E
 * 0x001E   Invalid Input Configuration             O, S, P?~S                          P + E
 * 0x001F   Invalid Watchdog Configuration          O, S, P?~S                          P + E
 * 0x0020   Slave needs cold start                  Any                                 Current state + E
 * 0x0021   Slave needs INIT                        B, P, S, O                          Current state + E
 * 0x0022   Slave needs PREOP                       S, O                                S + E, O + E
 * 0x0023   Slave needs SAFEOP                      O                                   O + E
 * 0x0030   Invalid DC SYNCH Configuration          O, S                                S + E
 * 0x0031   Invalid DC Latch Configuration          O, S                                S + E
 * 0x0032   PLL Error                               O, S                                S + E
 * 0x0033   Invalid DC IO Error                     O, S                                S + E
 * 0x0034   Invalid DC Timeout Error                O, S                                S + E
 * 0x0042   MBX_EOE                                 B, P, S, O                          Current state + E
 * 0x0043   MBX_COE                                 B, P, S, O                          Current state + E
 * 0x0044   MBX_FOE                                 B, P, S, O                          Current state + E
 * 0x0045   MBX_SOE                                 B, P, S, O                          Current state + E
 * 0x004F   MBX_VOE                                 B, P, S, O                          Current state + E
 */
typedef struct TECSlave {
    struct TECMaster* m_pECMaster;

    WORD wIndex;
    WORD wSetAddress; // Station Address.
    WORD m_wError;    // Anders 2020-6-11, add.

    TBOOL bEnableDCSync; // Anders 20120615.
    WORD  wDeviceType;   // Anders 20120627.

    DWORD dwOutputsLogicalStartAddr; // Anders 20120612.
    DWORD dwInputsLogicalStartAddr;

    TALStatus ALStatus;       // ET1100 AL State.
    WORD      wALStatusCodes; // ET1100 Al Status Codes.
    WORD      wRxSmCount;
    WORD      wTxSmCount;

    WORD wRxMapGroupCount;
    WORD wTxMapGroupCount;

    DWORD         m_dwOffsetCompensation;
    DWORD         m_dwPropagationDelay;
    DWORD         m_dwRecieveTimePort0; // Receive Time Port 0 (0x0900:0x0903 )
    DWORD         m_dwRecieveTimePort1; // Receive Time Port 1 (0x0904:0x0907 )
    DWORD         m_dwLocalTime;        // Receive Time ECAT Processing Unit (0x0918:0x091F)
    ECATLOCALTIME m_ldwSystemTime;
    ECATLOCALTIME m_ldwSystemTimeOffset; //Register System Time Offset (0x0920:0x0927)
    ECATLOCALTIME m_ldwSYNC0StartTime;

    TSMCfg SmMBoxOut;        // Master Write Data to SmMBoxOut.
    TSMCfg SmMBoxIn;         // Master Read Data from SmMBoxIn.
    TSMCfg SmProcessOutputs; // Master Write Data to SmProcessOutputs. Slave RxPDO Mapping.
    TSMCfg SmProcessInputs;  // Master Read Data from SmProcessInputs. Slave TxPDO Mapping.

    TBOOL bRxPDONotAssign : 8;
    TBOOL bTxPDONotAssign : 8;

    TPDOMappingParamCfg RxPDOMappingParams[PDOMAPING_MAX]; // RxPDO Mapping Params.
    TPDOMappingParamCfg TxPDOMappingParams[PDOMAPING_MAX]; // TxPDO Mapping Params.
    TInitSDOCfg         InitSDOCfg[INITSDOCFG_MAX];        // Init Data Use SDO.
    TSmAssign           RxSmAssign[PDOMAPING_MAX];
    TSmAssign           TxSmAssign[PDOMAPING_MAX];

    TECSlave_Mailbox Mailbox;
    unsigned         mailbox_run;
} TECSlave;
/**
 * EtherCAT Master Controller. Anders 20110908.
 */
typedef struct TECMaster {
    INT               nDeviceHandle;
    INT               nSwitchHandle;
    EtherCAT_ReadNet  m_pReadNet;
    EtherCAT_WriteNet m_pWriteNet;

    TBOOL bReset; // Reset EtherCAT Master.
    WORD  wError;

    WORD wECState;       // Master State.
    WORD wAppSlaveMax;   // applictin configed slave max count.
    WORD wAppSlaveCount; // Applictin Configed SlaveCount.

    WORD wWatchdogTime;       // PDO Watchdog Time, ms.
    WORD w1stSlaveSetAddress; // First Slave Station Address.

    DWORD dwOutputsLogicalStartAddr; // First Slave LogicalStartAddr.
    DWORD dwInputsLogicalStartAddr;  // First Slave LogicalStartAddr.

    DWORD dwLRDAddrFeedbackError_CNT; // Anders 2018-10-30, Add for Test.

    DWORD dwSyncCycleTime_ns; // SYNC0 CycleTime
    BYTE  m_bySendECCount;    // Inc Count;
    //WORD                    m_bReadRefSysTimeSlavesFlag;// Anders 2015-6-4, Add.

    WORD m_wReadError;
    WORD m_wECConfigCount;

    WORD wSlavesProcessOutputsBytes; // Byte.
    WORD wSlavesProcessInputsBytes;  // Byte.

    WORD  wCycleTime;    // PDO and DCSync CycleTime.
    TBOOL bEnableDCSync; // Anders 20120428.

    Ttimer lSDOTimeout;

    TECSlave*           pECSlaves;     // Pointer to Commected Slaves.
    ETHNETDATAFRAME_EC* pSendDatagram; // Pointer to TxDatagramsBuf.
    ConfigDatagram_T    ConfigEC;

    ETHNETDATAFRAME_EC TxDatagramsBuf; // Tx EtherCAT Datagrams.
    ETHNETDATAFRAME_EC RxDatagramsBuf; // Rx EtherCAT Datagrams Data 44*-1498 Byte. awRxDatagramsBuf[RXBUFFER_ETHERCATDATAGRAMS]
    ECDATAGRAMHEADER*  pPrevDatagram;

    WORD wNoEcho : 8;
    WORD wOfflineScan;

    ethernet_switch_s_port switch_port; // EoE 网络连接端口.
} TECMaster;
/**
 * EtherCAT Master Config. Anders 20110908.
 */
typedef struct TECMasterCfg {
    BYTEX acSrcMAC[CONST_LENGTH_MAC];
    DWORD dwOutputsLogicalStartAddr; // First Slave LogicalStartAddr.
    DWORD dwInputsLogicalStartAddr;  // First Slave LogicalStartAddr.
    DWORD dwSyncCycleTime_ns;        // SYNC0 CycleTime

    INT               nDeviceHandle;
    INT               nSwitchHandle;
    EtherCAT_ReadNet  pReadNet;
    EtherCAT_WriteNet pWriteNet;

    WORD      wSlaveCount;
    TECSlave* pECSlaves;
} TECMasterCfg;
/*==========================================================================+
|           External                                                        |
+==========================================================================*/
void ECMaster_Init(TECMaster* const cthis, TECMasterCfg* pECMasterCfg);
void ECMaster_Deal(TECMaster* const cthis, WORD* pData, WORD wlength);
void ECMaster_Run(TECMaster* const cthis);
void ECMaster_Reset(TECMaster* const cthis);
void ECMaster_DealSwitch(TECMaster* const cthis, void* data, unsigned length);
WORD ECMaster_DownloadSDO(TECMaster* const cthis, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize, TBOOL bCompleteAccess); // Anders 2016-6-15, modify.TBOOL bExcute,
WORD ECMaster_UploadSDO(TECMaster* const cthis, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize, TBOOL bCompleteAccess);   // Anders 2016-6-15, modify. TBOOL bExcute,
WORD ECMaster_UploadSDONormal(TECMaster* const cthis, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize);
WORD ECMaster_DownloadSDONormal(TECMaster* const cthis, WORD wSlaveIndex, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize);

void InitSendECFrameHeader(ETHNETFRAME* pEthnetFrame, WORD Length);
void InitSendDatagram(ECDATAGRAMHEADER* pDatagram, WORD cmd, BYTE byIndex, WORD Length, WORD offset, WORD Address);
void ChangeSlavesState(TECMaster* const cthis);
//void ConfigPDOMap(TECMaster * const cthis);
void CheckECConfigTimeout(ConfigDatagram_T* ConfigInfo);
void ECConfigFunc(ConfigDatagram_T* ConfigInfo, void (*pFunc)(TECMaster* const cthis));
void ChangeECConfigState(ConfigDatagram_T* ConfigInfo, WORD wState, WORD wTxCommand);

void  FlushDatagram(TECMaster* const cthis);
void* SendDatagram(TECMaster* const cthis, WORD cmd, WORD length, WORD offset, WORD address, bool flush);

void* SendMailboxStart(TECSlave* slave, WORD length, bool flush);
void  SendMailboxEnd(TECSlave* slave, WORD length, bool flush);
// char * STRNCPY_C2000(char * dest, const char * src, WORD n);
#ifdef __cplusplus
}
#endif

#include "ethercatmaster_coe.h"
#include "ethercatmaster_eoe.h"
#include "ethercatmaster_foe.h"

#endif
