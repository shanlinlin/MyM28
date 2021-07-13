/*==============================================================================+
|  Function : CANOpenCommon                                                     |
|  Task     : CANOpenCommon Source File                                         |
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
#include "../include/CANOpenCommon.h"
#include "_eCan_28335.h"
#include <string.h>
//##############################################################################
//
//      Definition
//
//##############################################################################
//##############################################################################
//
//      Enumeration
//
//##############################################################################
//##############################################################################
//
//      Type Definition
//
//##############################################################################
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
static WORD Open_eCAN(WORD wChannel);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
static WORD u_awCanTxBuff[CAN_DEVICE_MAX][32 * sizeof(DATAFRAME)];
static WORD u_awCanRxBuff[CAN_DEVICE_MAX][32 * sizeof(DATAFRAME)];

#if (DEBUG)
DWORD g_aadwCANStateCount[CANCHANNAL_SUM][CANOPENRETCODE_SUM];
#endif
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [CANOpen_Open description]
 * @param  eChannel [description]
 * @return          [description]
 */
E_CANOpenRetCode CANOpen_Open(E_CANChannel eChannel)
{
	E_CANOpenRetCode eRet;

	eRet = (E_CANOpenRetCode)Open_eCAN(eChannel);
#if (DEBUG)
	++g_aadwCANStateCount[eChannel][eRet];
#endif
	return eRet;
}
/**
 * [CANOpen_Close description]
 * @param  eChannel [description]
 * @return          [description]
 */
E_CANOpenRetCode CANOpen_Close(E_CANChannel eChannel)
{
	E_CANOpenRetCode eRet;

	eRet = (E_CANOpenRetCode)_Close_eCan(eChannel);
#if (DEBUG)
	++g_aadwCANStateCount[eChannel][eRet];
#endif
	return eRet;
}
/**
 * [CANOpen_RX description]
 * @param eChannel [description]
 */
void CANOpen_RX(E_CANChannel eChannel)
{
	_eCan_Rx(eChannel);
}
/**
 * [CANOpen_TX description]
 * @param eChannel [description]
 */
void CANOpen_TX(E_CANChannel eChannel)
{
	_eCan_Tx(eChannel);
}
/**
 * [CANOpen_Read description]
 * @param  eChannel    [description]
 * @param  psDataFrame [description]
 * @return             [description]
 */
E_CANOpenRetCode CANOpen_Read(E_CANChannel eChannel, CANOpenDataFrame_S * psDataFrame)
{
	E_CANOpenRetCode eRet;
	WORD             wSize;

	eRet = (E_CANOpenRetCode)_Read_eCan(eChannel, (WORD *)psDataFrame, sizeof(*psDataFrame), &wSize);
#if (DEBUG)
	++g_aadwCANStateCount[eChannel][eRet];
#endif
	return eRet;
}
/**
 * [CANOpen_Write description]
 * @param  eChannel    [description]
 * @param  psDataFrame [description]
 * @return             [description]
 */
E_CANOpenRetCode CANOpen_Write(E_CANChannel eChannel, CANOpenDataFrame_S * psDataFrame)
{
	E_CANOpenRetCode eRet;

	eRet = (E_CANOpenRetCode)_Write_eCan(eChannel, (WORD *)psDataFrame, sizeof(*psDataFrame));
#if (DEBUG)
	++g_aadwCANStateCount[eChannel][eRet];
#endif
	return eRet;
}
/**
 * [Open_eCAN description]
 * @param  wChannel [description]
 * @return          [description]
 */
static WORD Open_eCAN(WORD wChannel)
{
	ECAN ECan;

#if (DEBUG)
	memset(g_aadwCANStateCount[wChannel], 0, sizeof(g_aadwCANStateCount[wChannel]));
#endif
	memset(&ECan, 0, sizeof(ECan));
	ECan.CanConf.wCanMode                    = CANMODE_ENHANCED;
	ECan.CanConf.BaudrateConf.wBitTime       = BITTIME_15;
	ECan.CanConf.BaudrateConf.wBaudrate      = BAUDRATE_1MBPS; //BAUDRATE_500KBPS; //BAUDRATE_1MBPS;
	ECan.CanConf.BaudrateConf.wSamplingPoint = SAMPLINGPOINT_80PERCENT;
	ECan.wRxBufferSize                       = sizeof(u_awCanRxBuff[wChannel]);
	ECan.wTxBufferSize                       = sizeof(u_awCanTxBuff[wChannel]);
	ECan.pRxBuffer                           = (BYTE *)&u_awCanRxBuff[wChannel];
	ECan.pTxBuffer                           = (BYTE *)&u_awCanTxBuff[wChannel];
	ECan.CanConf.nDevice                     = wChannel;
	return _Open_eCan(&ECan);
}
