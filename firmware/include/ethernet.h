/*==========================================================================+
|  Class    : Ethernet                                                      |
|             Ethernet Protocol Function                                    |
|  Task     : Ethernet service routine  file                                |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   :                                                               |
|  Version  : V1.00                                                         |
|  Creation :                                                               |
|  Revision :                                                               |
+==========================================================================*/

#ifndef D__ETHERNET
#define D__ETHERNET

#include "common.h"
//#include  "_cs8900a.h"
#include "_ax88796b.h"
//#include  "ethercatmaster.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define PROTOCOL_IP 0x0008       // IP protocol flag
#define PROTOCOL_ARP 0x0608      // ARP protocol flag
#define PROTOCOL_TECH 0xA588     // Techmation protocol flag 0x88A5
#define PROTOCOL_ETHERCAT 0xA488 // EtherCAT protocol flag 0x88A4

#define PROTOCOL_IP_ICMP 1           // ICMP protocol flag
#define PROTOCOL_IP_UDP 17           // UDP protocol flag
#define PROTOCOL_ARP_REQUEST 0x0100  // ARP request flag
#define PROTOCOL_ARP_REPLY 0x0200    // ARP reply flag
#define PROTOCOL_ICMP_REQUEST 0x0008 // ICMP request flag
#define PROTOCOL_ICMP_REPLY 0x0000   // ICMP reply flag
#define PROTOCOL_IP_VERLEN 0x45      // ip protocol version
#define PROTOCOL_UDP_SPORT 0x3930    // Source machine port number
#define PROTOCOL_UDP_DPORT 0x3930    // Destination machine port number

#define CONST_LENGTH_IP 4 / 2                  // IP length
#define CONST_LENGTH_ETHERHEAD 14 / 2          // ethernet header length
#define CONST_LENGTH_IPHEAD 20 / 2             // ip header length
#define CONST_LENGTH_ARPHEAD 8 / 2             // arp header length
#define CONST_LENGTH_UDPHEAD 8 / 2             // udp header length
#define CONST_LENGTH_ICMPHEAD 8 / 2            // icmp header length
#define CONST_LENGTH_PROHEAD (14 + 20 + 8) / 2 // protocol header length

#define CONST_TIME_LIVE 128 // packet live time

#define CONST_SIZE_BUFFER 1408     // buffer size 1518*2-->1518
#define CONST_SIZE_DATA 1472       // data size of one packet
#define CONST_SIZE_SENDPACKET 1408 // size of one packet 'calculated by BYTE'

#define MAX_NETDEVICE 3
#define MAX_SESSIONCH_FACTOR 2                    // Anders 2016-4-28, Add.
#define MAX_SESSIONCH (1 << MAX_SESSIONCH_FACTOR) // Anders 2016-4-28, Modiy.
#define MAX_SESSIONCH_MASK (MAX_SESSIONCH - 1)    // Anders 2016-4-28, Add.

#define STATESESSIONCH_NULL 0
#define STATESESSIONCH_OPEN 1
#define STATESESSIONCH_ACTIVE 2

#define CONST_ETHERNET_MAC_BYTE 6

#define ETHERNET_MTU_BYTE 1500
#define ETHERNET_FRAME_BYTE (CONST_ETHERNET_MAC_BYTE * 2 + 2 + ETHERNET_MTU_BYTE)

#define _get_byte(data_, offset_) (((data_) >> ((offset_)*8)) & 0xFF)
#define _set_byte(data_, offset_) ((data_) << ((offset_)*8))
#define _bswap16(data_) (_set_byte(_get_byte(data_, 1), 0) | _set_byte(_get_byte(data_, 0), 1))
#define _bswap32(data_) (_set_byte(_get_byte(data_, 3), 0) | _set_byte(_get_byte(data_, 0), 3) | _set_byte(_get_byte(data_, 2), 1) | _set_byte(_get_byte(data_, 1), 2))

#define ntoh16(data_) _bswap16(data_)
#define ntoh32(data_) _bswap32(data_)
#define hton16(data_) ntoh16(data_)
#define hton32(data_) ntoh32(data_)
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
struct ethernet_switch_s_port;

typedef void (*DFTFUNC)(WORD*, WORD);
typedef void (*NETFUNC)(int, WORD*, WORD);

typedef struct ethernet_s_mac {
    uint16_t addr1 : 8;
    uint16_t addr2 : 8;
    uint16_t addr3 : 8;
    uint16_t addr4 : 8;
    uint16_t addr5 : 8;
    uint16_t addr6 : 8;
} ethernet_s_mac;

typedef struct ethernet_s_ip4 {
    uint16_t addr1 : 8;
    uint16_t addr2 : 8;
    uint16_t addr3 : 8;
    uint16_t addr4 : 8;
} ethernet_s_ip4;

//typedef struct ethernet_s_frame {
//    uint16_t length; // bytes of buffer.
//    uint16_t buffer[ETHERNET_BUFFER_BYTES / 2];
//} ethernet_s_frame;

typedef struct tagDLCHR {
    BYTEX acDstMAC[CONST_LENGTH_MAC];
    BYTEX acSrcMAC[CONST_LENGTH_MAC];
    WORD  wProType;
} DLCHR;

typedef struct tagIPHR {
    BYTEX VerType;
    WORD  ip_len;
    WORD  ip_id;
    WORD  ip_fragoff;
    BYTEX TTLProtocol;
    WORD  ip_cksum;
    BYTEX ip_src[CONST_LENGTH_IP];
    BYTEX ip_dst[CONST_LENGTH_IP];
} IPHR;

typedef struct tagUDPHR {
    WORD udp_src;
    WORD udp_des;
    WORD udp_len;
    WORD udp_chk;
} UDPHR;

typedef struct tagARP {
    BYTEX ArmMac[CONST_LENGTH_MAC], RootMac[CONST_LENGTH_MAC];
    WORD  DataType;
    WORD  MacType, ProType;
    BYTEX HLenPLen;
    WORD  Opcode;
    BYTEX Root_Mac[CONST_LENGTH_MAC], Root_IP[CONST_LENGTH_IP], Arm_Mac[CONST_LENGTH_MAC], Arm_IP[CONST_LENGTH_IP];
} ARP;

typedef struct tagMACPRO {
    BYTEX ma_tha[CONST_LENGTH_MAC];
    BYTEX ma_sha[CONST_LENGTH_MAC];
    short ma_pro;
} MACPRO;

typedef struct tagARPPRO {
    short ar_hwtype;
    short ar_prtype;
    BYTEX HwlenPrlen;
    short ar_op;
    BYTEX ar_sha[CONST_LENGTH_MAC];
    BYTEX ar_spa[CONST_LENGTH_IP];
    BYTEX ar_tha[CONST_LENGTH_MAC];
    BYTEX ar_tpa[CONST_LENGTH_IP];
} ARPPRO;

typedef struct tagMACARP {
    MACPRO mac;
    ARPPRO arp;
} MACARP;

typedef struct tagICMP {
    BYTEX TypeCode;
    WORD  ic_chksum;
    WORD  ic_iden;
    WORD  ic_sequ;
} ICMP;

// zholy090722 add
typedef struct tagIOETH {
    WORD    wType;                     // PROTOCOL_IP, PROTOCOL_ARP, PROTOCOL_TECH, PROTOCOL_ETHERCAT
    BYTEX   adstMAC[CONST_LENGTH_MAC]; // destination MAC (when using IP protocol need get this address by ARP or other method)
    BYTEX   asrcIP[CONST_LENGTH_IP];
    BYTEX   adstIP[CONST_LENGTH_IP];
    WORD    wsrcPort;
    WORD    wdstPort;
    NETFUNC pNetFunc; // callback function
} IOETH;

typedef struct tagSESSIONCH {
    WORD         wState; // 0: not open; 1: open but not active; 2: open and active.
    WORD         mutex;
    IOETH const* ioeth;
    BYTEX        adstMAC[CONST_LENGTH_MAC];
    BYTEX        adstIP[CONST_LENGTH_IP];
    WORD         wdstPort;
} SESSIONCH;

typedef struct tagCONFIGNET {
    int     nDeviceHandle;
    DFTFUNC pDefaultIPFunc;
    DFTFUNC pDefaultTECHFunc;
    BYTEX   asrcIP[CONST_LENGTH_IP];
    //BUFFER        buffer;

} CONFIGNET;

typedef struct tagPARAMETH {
    int  nSessionHandle; // current processing session handle
    WORD wSendPacketIndex;
    WORD awAllActive[MAX_NETDEVICE]; // indicate whether connect destination IP with destination MAC when using IP protocol

    DFTFUNC apDefaultIPFunc[MAX_NETDEVICE];   // used in IP protocol for not matching any session
    DFTFUNC apDefaultTECHFunc[MAX_NETDEVICE]; // used in techmation protocol for not matching any session

    BYTEX asrcIP[MAX_NETDEVICE][CONST_LENGTH_IP];   // default source IP address
    BYTEX asrcMAC[MAX_NETDEVICE][CONST_LENGTH_MAC]; // defult source MAC

    SESSIONCH sessionch[MAX_NETDEVICE][MAX_SESSIONCH]; // session table (use source IP address or other for special session)

    ARP arp;
} PARAMETH;

enum emNETDEVICETYPE {
    NETDEVICE_NONE,
    NETDEVICE_CS8900A,
    NETDEVICE_AX88796B,
    NETDEVICE_TYPEMAX
};
/*==========================================================================+
|           Function and Class prototype                                    |
+==========================================================================*/
WORD OpenEthernet(CONFIGNET* pconfignet);
void CloseEthernet();

void ResetParamEth();
int  OpenEthSession(int nDeviceHandle, IOETH const* pIOETH);
void ReqActivateEthSession(int nSessionHandle);
void CloseEthSession(int nSessionHandle);
void ChangeSourceIP(int nDeviceHandle, BYTEX* psrcIP);
void ChangeSessionchIP(int nSessionHandle, BYTEX* psrcIP);
int  GetSessionchState(int nSessionHandle);

int  ReadNetData(int nDeviceHandle, void* data, unsigned length);
WORD SendNetData(int nSessionHandle, WORD* pw, WORD wLength);
void DealData(int nDeviceHandle);

int send(int socket, void const* data, unsigned length);

//add by zxc 20090410
void _InstallNetDevice(int nDeviceHandle);
void _RestoreNetDevice();
WORD _ReadNetDevice(int nDeviceHandle, WORD* pw, WORD wBufferSize, WORD* pReadCount);
WORD _WriteNetDevice(int nDeviceHandle, WORD* pw, WORD wLength);
WORD _GetLinkStatus(int nDeviceHandle);
void _NetDeviceISR();

void _SetSwitchPort(int nDeviceHandle, struct ethernet_switch_s_port* port);

//void  SendCMD_EEPROM_ax88796b(DWORD dwValue, WORD wLength);
//WORD  ReceiveCMD_EEPROM(DWORD dwValue, WORD wLength);
void ClearBindAddrToSession(int nSessionHandle);
void DisactivateEthSession(int nSessionHandle);

void SendARPData(int nSessionHandle);

void ethernet_s_mac_set_value(ethernet_s_mac* mac, unsigned addr1, unsigned addr2, unsigned addr3, unsigned addr4, unsigned addr5, unsigned addr6);
void ethernet_s_ip4_set_value(ethernet_s_ip4* ip4, unsigned addr1, unsigned addr2, unsigned addr3, unsigned addr4);
/*==========================================================================+
|           External                                                        |
+==========================================================================*/
extern ROUTINE    g_routinechecksessionstate;
extern const WORD g_wNetDeviceType;

extern DWORD g_dwTxNetDatagramCount[MAX_NETDEVICE];
extern DWORD g_dwRxNetDatagramCount[MAX_NETDEVICE];
/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif
