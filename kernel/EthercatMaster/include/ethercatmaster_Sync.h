/*==========================================================================+
|  Function : EtherCAT Sync                                                 |
|  Task     : EtherCAT commnunication		                                |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders                                                   		|
|  Version  : V1.00                                                         |
|  Creation : 2012/5/4                                                      |
|  Revision : 1.0                                                           |
+==========================================================================*/

#ifndef D__ETHERCATMASTER_SYNC_H
#define	D__ETHERCATMASTER_SYNC_H

#include "common.h"
#include "ethercatmaster.h"

#ifdef      __cplusplus
extern      "C" {
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/


/*---------------------------------------------------------------------------+
|           Type Definition	                                                 |
+---------------------------------------------------------------------------*/

/*===========================================================================+
|           Type Definition2                                                 |
+===========================================================================*/

/*==========================================================================+
|           External					                                    |
+==========================================================================*/
void	GetClockParam(TECMaster * const cthis);
void	GetDataLinkStatus(TECMaster * const cthis);
void	GetSYNCPulseLength(TECMaster * const cthis);
void	GetSystemTime0(TECMaster * const cthis);
void	SetSYNC0CycleTime(TECMaster * const cthis);
void	SetSYNC0StartTime(TECMaster * const cthis);
void	SetSYNCControlUnit(TECMaster * const cthis);
void	SetLatch0Control(TECMaster * const cthis);
void	GetSystemTime1(TECMaster * const cthis);
void	SetDriftCompensationBandwidth(TECMaster * const cthis);
void	SendBWRFrameToPort(TECMaster * const cthis);
void	ReadRefSysTimeToOtherSlaves(TECMaster * const cthis);
void	ReadBeforeSyncTime(TECMaster * const cthis);
void	GetSlavePortTime(TECMaster * const cthis);
void	DCPropagationDelayCalc(TECMaster * const cthis);
void	GetProcessUnitTime(TECMaster * const cthis);
void	DCOffsetCompensation(TECMaster * const cthis);
void	DCDriftCompensation(TECMaster * const cthis);
WORD	SendDriftDatagram(TECMaster * const cthis, WORD *pData, WORD wMore);
void	ReadSysTimeDifference(TECMaster * const cthis);
void	ReadRefSysTimeSlaves(TECMaster * const cthis);
#ifdef      __cplusplus
}
#endif

#endif
