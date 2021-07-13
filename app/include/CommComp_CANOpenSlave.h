/*==============================================================================+
|  Function : Communication Component CANOpen Master                            |
|  Task     : Communication Component CANOpen Master Header File                |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : shanll                                                            |
|  Version  : V1.00                                                             |
|  Creation : 2021/03/31                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__COMMCOMP_CANOPENSLAVE__H
#define D__COMMCOMP_CANOPENSLAVE__H
//------------------------------------------------------------------------------
#include "CANOpenCommon.h"
#include "CommComp_Factory.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define CANOPENSLAVE_SENDPDO_AMOUNT 4 // Send to other's PDO.
#define CANOPENSLAVE_RECVPDO_AMOUNT 4 // Recv come from other's send PDO.
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_CANCommSDOResponse {
	CANCOMMSDORESPONSE_UPLOAD,
	CANCOMMSDORESPONSE_DOWNLOAD
} E_CANCommSDOResponse;
//##############################################################################
//
//      Type Definition
//
//##############################################################################
struct CommComp_CANOpenSlave;
typedef void (*CANOpenSlaveCommRun_F)(struct CommComp_CANOpenSlave* psCompCANopenSlave);
typedef void (*CANOpenSlaveEventCallBack_F)(struct CommComp_CANOpenSlave* psCompCANopenSlave, struct CANOpenDataFrame_S* psDataFrame, E_CANOpenEventType eEventType);
typedef void (*CommCustomSlaveFunc_F)(struct CommComp_CANOpenSlave* psCompCANopenSlave);

typedef  struct CommComp_CANOpenSlave
{
	CANOpenSlaveCommRun_F         	PRI_fCommRun;
	WORD                            PRO_bNodeID : 7;    // 0x00~0x7F.
	WORD                            PRO_bNodeState : 7; // E_CANOpenNodeState.
	WORD                            PRI_bSDOState : 2;  // CANCommSDOState_E.
	WORD                            PUB_blClearSize : 1;
	WORD                            PUB_blFreeSDO : 1;
	Ttimer                 			PRI_lMsgSendTime;
    CANOpenSlaveEventCallBack_F     PRO_fEventCallBack;
	CommCustomSlaveFunc_F       	PRO_fCustomFunc;
}CommComp_CANOpenSlave;

typedef  struct CommComp_CANOpenSlaveManager
{
	BOOL					bSlaveFuncEnable;
	CommComp_CANOpenSlave*	s_cCommCanopenSlave;
}CommComp_CANOpenSlaveManager;
//------------------------------------------------------------------------------
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void 		  Init_CommCanopenSlave();
void          Parse_CANopenSlave(E_CommType eCommType);
void          Run_CANOpenSlave(E_CommType eCommType);
//------------------------------------------------------------------------------
E_CANOpenRetCode CANOpenSlave_SendNMTErrCtrl(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfNMT, E_CANOpenNodeState eNMTErrCtrl);
E_CANOpenRetCode CANOpenSlave_SendSync(E_CANChannel eChannel);
E_CANOpenRetCode CANOpenSlave_SendRPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO);
E_CANOpenRetCode CANOpenSlave_SendPDO(E_CANChannel eChannel, BYTE byNodeID, E_CANOpenFuncID eFuncIDOfPDO, WORD wDataLen, void const* const pvData);
E_CANOpenRetCode CANOpenSlave_SendSDO(E_CANChannel eChannel, BYTE byNodeID, CANOpenCommSDOMsg_S const* const psSDOMsg);
E_CANOpenRetCode CANOpenSlave_SendNodeGruading(E_CANChannel eChannel, BYTE byNodeID);

E_CANChannel CANOpenSlave_CommTypeToChannel(E_CommType eCommType);
void         CANOpenSlave_ResetComm(CommComp_CANOpenSlave* psCompSlave);

void SCANOpenCompSlave_Init(CommComp_CANOpenSlave* psCompSlave);
void SCANOpenCompSlave_InitEventCallBack(CommComp_CANOpenSlave* psCompSlave, CANOpenSlaveEventCallBack_F fEventCallBack);
void SCANOpenCompSlave_InitCustomFunc(CommComp_CANOpenSlave* psCompSlave, CommCustomSlaveFunc_F fSlaveCustomFunc);
//------------------------------------------------------------------------------
WORD UploadCANSDO_Easy(WORD wChannal, WORD wNodeID, WORD wIndex, WORD wSubIndex, WORD* pData, WORD wByteSize);
WORD DownloadCANSDO_Easy(WORD wChannal, WORD wNodeID, WORD wIndex, WORD wSubIndex, DWORD dwData, WORD wByteSize);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern  CommComp_CANOpenSlaveManager	g_sCanopenSlaveManager;
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
