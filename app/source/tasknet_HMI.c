/*==========================================================================+
|  Function : Network commnunication                                        |
|  Task     : Network commnunication                                        |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : zholy                                                         |
|  Version  : V1.00                                                         |
|  Creation : 2007/07/23                                                    |
|  Revision : 1.0                                                           |
+==========================================================================*/
#include "tasknet_HMI.h"
#include "tasknet_master.h"
#include "comp_fifo.h"
#include "comp_fifo2.h"
/*===========================================================================+
|           Function Prototype                                               |
+===========================================================================*/
//Yang 2008/3/8 Adding for TMJ5-Communication protocol
void ReqActValue(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqCommand(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqCurveValue(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqMsgValue(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqReadRefValue(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqWriteRefValue(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqLinkState(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqMoldsetState(int nSessionHandle, WORD * pdata, WORD wlength);
void ReqMoldsetUpdate(int nSessionHandle, WORD * pdata, WORD wlength);
void RespActValue(int nSessionHandle);
void RespRefValue(int nSessionHandle);

void RespCurveValue(int nSessionHandle);
void RespCommand(int nSessionHandle);
void RespLinkState(int nSessionHandle);
void RespMoldsetState(int nSessionHandle);
void RespMoldsetUpdate(int nSessionHandle);

//void    CollectCurve(void);
//
//void  initCurve(void);
//BOOL  RegisterCurve(WORD wCurveID, TM_CURVEDETAIL *pCurveDetail);
//BOOL  UnRegisterCurve(WORD wCurveID);
//BOOL  Collect_TraceCurve(WORD wCurveID);
//BOOL  Send_TraceCurve(int nSessionHandle, WORD wCurveID);
//BOOL  Start_TraceCurve(WORD wCurveID);
//BOOL  End_TraceCurve(WORD wCurveID);
//WORD  MotionIDToCurveID(WORD wMCID);
/*===========================================================================+
|           Definition                                                       |
+===========================================================================*/
#define RESPONSE_ACTUAL_ID 1001
#define RESPONSE_READ_REFERENCE_ID 1002
#define RESPONSE_WRITE_REFERENCE_ID 1003
#define RESPONSE_MESSAGE_ID 1004
#define RESPONSE_CURVE_ID 1005
#define RESPONSE_COMMAND_ID 1006
#define RESPONSE_LINKSTATE_ID 1007
#define RESPONSE_MOLDSETSTATE_ID 1008
#define RESPONSE_MOLDSETUPDATE_ID 1009

//#define     COMMAND_KEY_MANUAL              0x00000000

#define MOTORCOMM_ENABLED 0x00020000
#define MOTORCOMM_DISABLED 0x00020001

#define COMMAND_AD_CALIBRATEMAX 0x00030000
#define COMMAND_AD_CALIBRATEMIN 0x00030001
#define COMMAND_AD_CALIBRATERESET 0x00030002
#define COMMAND_AD_CALIBRATESAVE 0x00030003

#define MOTORCOMM_POSIHOMING_INJECT 0x00040000
#define MOTORCOMM_POSIHOMING_CLMP 0x00040001
#define MOTORCOMM_POSIHOMING_EJECTOR 0x00040002
#define MOTORCOMM_POSIHOMING_NOZZLE 0x00040003
#define MOTORCOMM_POSIHOMING_CHARGE 0x00040004

//#define       SENDONCEDATATIME                20
//#define     MOTORCOMMAP_LEN                 (sizeof(g_motorcommap)/sizeof(MOTORCOMMAP))
//#define     SENDONCELIST_LEN                (sizeof(u_SendOnceList)/sizeof(SENDONCELIST))
//#define     CURVERESPLIST_LEN               (sizeof(u_CurveRespList)/sizeof(CURVERESPLIST))

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
const WORD HEADER_REQ = 0x8899; // header request
const WORD HEADER_RES = 0x5588; // header response
const WORD FOOTER_REQ = 0x6688; // footer request
const WORD FOOTER_RES = 0x1166; // footer response

const COMMMAP g_commmap[] = {
    { COMM_NULL, NULL }, // Anders 20100706.
    { COMM_ACTUAL, &ReqActValue },
    { COMM_READPARAMETER, &ReqReadRefValue },
    { COMM_WRITEPARAMETER, &ReqWriteRefValue },
    { COMM_MESSAGE, &ReqMsgValue },
    { COMM_CURVE, &ReqCurveValue },
    { COMM_MOTIONCOMMAND, &ReqCommand },
    { COMM_LINKSTATE, &ReqLinkState },
    { COMM_MOLDSETSTATE, &ReqMoldsetState },
    { COMM_MOLDSETUPDATE, &ReqMoldsetUpdate },
    { COMM_END, NULL }
};

/*
const       MOTORCOMMAP g_motorcommap[] = {
            {COMMAND_AD_CALIBRATEMAX,           Command_ADCaliMax},
            {COMMAND_AD_CALIBRATEMIN,           Command_ADCaliMin},
            {COMMAND_AD_CALIBRATERESET,         Command_ADCaliReset},
            {COMMAND_AD_CALIBRATESAVE,          Command_ADCaliSave},

            {COMMAND_KEY_MANUAL,                NULL},
            {COMMAND_POSIHOMMING_INJECT,        &Command_InjPosiHoming},
            {COMMAND_POSIHOMMING_SEEKINJECT,    &Command_InjSeekPosiHoming},
            {COMMAND_AD_CALIMIN,                &Command_ADCaliMin},
            {COMMAND_AD_CALIMAX,                &Command_ADCaliMax},
            {COMMAND_AD_CALIRESET,              &Command_ADCaliReset},

};*/

//const     WORD    CNST_VERSION[9]     = { 0xA625, 0x0010, 0x0001, 0x0013, \
//                                          0x2300, 0x0204, 0x0808, 0x0100, 0x0000 };
//const WORD    u_awMaxVelocityID[]     = { INJECTMAXVELOCITY,SUCKBACKMAXVELOCITY};
//const WORD    u_awInjectEndData[]     = { INJECTENDPOSITION};
/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

static TEMPBUFFER u_TempBuffer;
WORD              u_wRespActCounter          = 1;
WORD              u_wRespReadRefCounter      = 1;
WORD              u_wRespWriteRecCounter     = 1;
WORD              u_wRespAlarmCounter        = 1;
WORD              u_wRespMoldsetStateCounter = 1;
WORD              u_wRespMoldsetRecCounter   = 1;
WORD              u_wRespCurveRecCounter     = 1;

RESPLIST u_RespList[RESP_MAX] = {
    // {wFlag  ,wCycle ,wNoElements    ,adwIDList  ,atIndexList    ,lCounter   ,pFunc              }
    { 0, 0, 0, { 0 }, { (DBIndex_T)0 }, 0, &RespActValue },
    { 0, 0, 0, { 0 }, { (DBIndex_T)0 }, 0, &RespRefValue },
};
//LINKSTATE     LinkState = {0, 0};
MSGSTATE     MsgState     = { 0, 0, 0 };
MOLDSETSTATE MoldsetState = { 0, 0, 0, 0 };

//SENDONCELIST    u_SendOnceList[] = {
//    {0, SENDONCEDATATIME, 0,  NULL}, //SendMaxVelocity},
//    {0, SENDONCEDATATIME, 0,  NULL}, //RespMsgValue},
//    {0, SENDONCEDATATIME, 0,  NULL}, //SendVersionToHMI},
//    {0, SENDONCEDATATIME, 0,  NULL},
//};

//CURVERESPLIST     u_CurveRespList[CURVEMOTIONID_MAX];

//====================================
// Curve.
//====================================

//====================================)
// End Curve.                         )
//====================================)
#define _DEBUG_NET //for test
#ifdef _DEBUG_NET
TEMPBUFFER u_TestBuffer;
WORD       wTestType = 3;
void       SendTestBuffer(int nSessionHandle, WORD wTestType);
WORD       g_wSendTest    = 0;
WORD       wCycleTime     = 20;
WORD       g_wNoElements  = 50;
WORD       g_wTestCounter = 1;
WORD       g_wCounter[560];
WORD       g_wElements[500];
WORD       g_wRecord = 0;

#endif

int  g_nHMISession    = -1;
WORD g_wTelegramCount = 0;

//#define     TEST_NET  1
#ifdef TEST_NET
WORD g_wTest_SendUpdateCount = 0;
WORD g_wTest_SendCurveCount  = 0;
//WORD        g_wTTHMI = 1;
#endif
/*===========================================================================+
|           Class implementation                                             |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
WORD CreateTaskNet_HMI()
{
    int i;

    //ResetTimer(&LinkState.lCounter, 0);

    for (i = 0; i < RESP_MAX; i++)
    {
        memset(&u_RespList[i].atIndexList, 0, sizeof(u_RespList[i].atIndexList));
        //AddDnCounter(&u_RespList[i].lCounter);
        ResetTimer(&u_RespList[i].lCounter, 0);
    }
    //for (i=0; i<SENDONCELIST_LEN; i++)
    //{
    //  //AddDnCounter(&u_SendOnceList[i].lCounter);
    //  ResetTimer(&u_SendOnceList[i].lCounter, 0);
    //}

    memset(&u_TempBuffer, 0, sizeof(u_TempBuffer));
    u_TempBuffer.TMHeader.wHeader = HEADER_RES;
    //HMIFifo Creat
    comp_fifo_s_manager_init(&u_HMIFifo.manager);
    comp_fifo_s_manager_init_size(&u_HMIFifo.manager,265);
    memset(&u_HMIFifo.fifo_buffer, 0, sizeof(u_HMIFifo.fifo_buffer));

#ifdef _DEBUG_NET
    //SendTestBuffer(wTestType);        //for testing
    memset(g_wCounter, 0, sizeof(g_wCounter));
    memset(g_wElements, 0, sizeof(g_wElements));
#endif

    //initCurve();

    return 1;
}

void DestoryTaskNet_HMI()
{
    //int       i;
    //
    //for (i=0; i<RESP_MAX; i++)
    //  DeleteDnCounter(&u_RespList[i].lCounter);
    //
    //for (i=0; i<SENDONCELIST_LEN; i++)
    //{
    //  DeleteDnCounter(&u_SendOnceList[i].lCounter);
    //}
    //DeleteDnCounter(&LinkState.lCounter);
}

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RunTaskNet_HMI()
{
    int i;

    //if (CheckTimerOut(&LinkState.lCounter) || _GetLinkStatus() == NIC_LINK_OFF)
    //  {
    //  //if (LinkState.wState == 1)
    //      for (i=0; i<RESP_MAX; i++)
    //          {
    //          u_RespList[i].wFlag = 0;
    //          u_RespList[i].wCycle = 0;
    //          u_RespList[i].wNoElements = 0;
    //          //memset(u_RespList[i].atIndexList, 0, MAX_REQUEST_NO);
    //          }
    //  LinkState.wState = 0;
    //  memset(&MoldsetState, 0, sizeof(MoldsetState));
    //  }
    ////response actual value
    //if (LinkState.wState == 1)
    //  for (i=0; i<RESP_MAX; i++)
    //      {
    //      if (u_RespList[i].wFlag && CheckTimerOut(&u_RespList[i].lCounter) )
    //          {
    //          if (u_RespList[i].pFunc != NULL)
    //              u_RespList[i].pFunc(g_nDsp54NetSession);
    //          u_RespList[i].lCounter = u_RespList[i].wCycle;
    //          if (u_RespList[i].wCycle == 0)
    //              u_RespList[i].wFlag = 0;
    //          }
    //      }

    //if ( CheckTimerOut(&LinkState.lCounter) )   // Anders 2015-4-13, Mark.
    //{
    //  for (i=0; i<RESP_MAX; i++)
    //  {
    //      u_RespList[i].wFlag = 0;
    //      u_RespList[i].wCycle = 0;
    //      u_RespList[i].wNoElements = 0;
    //  }
    //  LinkState.wState    = 0;
    //  ResetTimer(&LinkState.lCounter, 0);
    //  memset(&MoldsetState, 0, sizeof(MoldsetState));
    //
    //    // Anders 2014-4-16, add.
    //    MC_DBData[CARD_INTERFACE_NET_DWRXHMIMOLDSETCOUNT].dbValue.dwData = MoldsetState.lUpdateCount;
    //}
    //else
    {
        for (i = 0; i < RESP_MAX; i++)
        {
            if (u_RespList[i].wFlag && CheckTimerOut(&u_RespList[i].lCounter))
            {
                if (u_RespList[i].pFunc != NULL)
                {
                    if (GetMoldsetState()) //&& GetToken_Retransmit(BASEADDR_TRISMITEDAUTOVALUE + i))   //test for Kevin.
                    {
                        u_RespList[i].pFunc(g_nDsp54NetSession);
                        //u_RespList[i].lCounter = u_RespList[i].wCycle;
                        ResetTimer(&u_RespList[i].lCounter, u_RespList[i].wCycle);
                        if (u_RespList[i].wCycle == 0)
                            u_RespList[i].wFlag = 0;
                        break; // Anders 20101009.
                    }
                    else
                        ResetTimer(&u_RespList[i].lCounter, 0); // u_RespList[i].lCounter = 0;
                }
                else
                    u_RespList[i].wFlag = 0;
            }
        }

        //for(i=0;i<SENDONCELIST_LEN;i++)
        //{
        //  if (u_SendOnceList[i].wFlag && CheckTimerOut(&u_SendOnceList[i].lCounter) )
        //    {
        //        if (u_SendOnceList[i].pFunc != NULL)
        //        {
        //            if(GetMoldsetState() )//&& GetToken_Retransmit(BASEADDR_TRISMITEDONCEVALUE + i)) //test for Kevin.
        //            {
        //                u_SendOnceList[i].pFunc(g_nDsp54NetSession);
        //                //u_SendOnceList[i].lCounter = u_SendOnceList[i].wCycle;
        //              ResetTimer(&u_SendOnceList[i].lCounter, u_SendOnceList[i].wCycle);
        //                break; // Anders 20101009.
        //            }
        //            else ResetTimer(&u_SendOnceList[i].lCounter, 0);    // u_SendOnceList[i].lCounter = 0;
        //        }
        //        else u_SendOnceList[i].wFlag = 0;
        //        if (u_SendOnceList[i].wCycle == 0) u_SendOnceList[i].wFlag = 0;
        //  }
        //}

        //for (i=1; i<=CURVEMOTIONID_MAX; i++)
        //{
        //  Collect_TraceCurve(i);
        //
        //  Send_TraceCurve(g_nDsp54NetSession, i);
        //}
    }
    // CollectCurve();
}
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ParseNetData_FromHMI(int nSessionHandle, WORD * pdata, WORD wLength)
{
    TMHEADER * pHeader;
    WORD       wType, wHeader, wFooter, wNoElements, wDataSize = 0;

    //DB_SetUser(DB_USER_HMI);                  //(20141029 THJ(Add: ))
    pHeader   = (TMHEADER *)pdata;
    wDataSize = pHeader->wSize;
    //nLeftSize = (int)wLength - (int)wDataSize;
    wHeader     = pHeader->wHeader;
    wFooter     = *((WORD *)pHeader + (wDataSize >> 1) - 1);
    wType       = pHeader->wType;
    wNoElements = pHeader->wNoElements;

    if (wType <= COMM_END && wType > 0 && wDataSize <= wLength && wHeader == HEADER_REQ
        && wFooter == FOOTER_REQ && wNoElements <= MAX_REQUEST_NO)
    {
        if (g_commmap[wType].pFunc != NULL)
            g_commmap[wType].pFunc(nSessionHandle, pdata, wLength);

#ifdef _DEBUG_NET
        g_wElements[wType]++;
        if (g_wSendTest == 2 && g_wRecord < sizeof(g_wCounter))
        {
            g_wCounter[g_wRecord] = wType;
            g_wRecord++;
        }
#endif
    }
    //DB_SetUser(DB_USER_DEFAULT);              //(20141029 THJ(Add: ))
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqActValue(int nSessionHandle, WORD * pdata, WORD wlength)
{
    TMHEADER * pHeader = (TMHEADER *)pdata;
    DWORD      dwID;
    DWORD *    pdwID;
    int        i;

    u_RespList[RESP_ACTUAL].wFlag = 0;//shanll 20200617 add
    u_RespList[RESP_ACTUAL].wCycle = pHeader->datatype.wCycle;
    if ((u_RespList[RESP_ACTUAL].wCycle > 0) && (u_RespList[RESP_ACTUAL].wCycle < 100)) // Anders 2015-4-8, Add.
        u_RespList[RESP_ACTUAL].wCycle = 100;
    u_RespList[RESP_ACTUAL].wNoElements = pHeader->wNoElements;
    if (u_RespList[RESP_ACTUAL].wNoElements == 0 || MoldsetState.wFinished == 0) // Anders 2016-4-28, Add " || MoldsetState.wFinished == 0 "
    {
        u_RespList[RESP_ACTUAL].wFlag = 0;
        return;
    }
    //shanll 20200617 move to the end
    //u_RespList[RESP_ACTUAL].wFlag = 1;
    //ResetTimer(&u_RespList[RESP_ACTUAL].lCounter, 0);
    pdwID = (DWORD *)(pdata + sizeofW(TMHEADER));
    for (i = 0; i < u_RespList[RESP_ACTUAL].wNoElements; i++)
    {
        memcpy(&dwID, pdwID, sizeof(DWORD));
        u_RespList[RESP_ACTUAL].adwIDList[i] = dwID; // Test, Anders 2014-4-15.
        //(20150115 THJ(Modification: )
        //(20150115 THJ)u_RespList[RESP_ACTUAL].atIndexList[i] = DataIDToIndex(dwID);
        //if (dwID >= MC_DBID_OFFSET)
        //  u_RespList[RESP_ACTUAL].atIndexList[i] = (DBIndex_T)MC_DataIDToIndex(dwID - MC_DBID_OFFSET);
        //else
        u_RespList[RESP_ACTUAL].atIndexList[i] = DataIDToIndex(dwID);
        //)
        pdwID++;
    }

    // Anders 2014-4-16, add.
    ++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXHMIREADACTVALUECOUNT;
    //shanll 20200617 move
    u_RespList[RESP_ACTUAL].wFlag = 1;
    ResetTimer(&u_RespList[RESP_ACTUAL].lCounter, 0);
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqReadRefValue(int nSessionHandle, WORD * pdata, WORD wlength)
{
    TMHEADER * pHeader = (TMHEADER *)pdata;
    DWORD      dwID;
    DWORD *    pdwID;
    WORD       wIndex;
    int        i;

    u_RespList[RESP_READPARAMETER].wFlag = 0;//shanll 20200617 add
    u_RespList[RESP_READPARAMETER].wCycle = pHeader->datatype.wCycle;
    //(THJ 2015-06-11 Modify:
    if ((u_RespList[RESP_READPARAMETER].wCycle > 0) && (u_RespList[RESP_READPARAMETER].wCycle < 100))
        u_RespList[RESP_READPARAMETER].wCycle = 100;
    u_RespList[RESP_READPARAMETER].wNoElements = pHeader->wNoElements;

    if (u_RespList[RESP_READPARAMETER].wNoElements == 0)
    {
        u_RespList[RESP_READPARAMETER].wFlag = 0;
        return;
    }
    //shanll 20200617 move to the end
    //u_RespList[RESP_READPARAMETER].wFlag = 1;
    //ResetTimer(&u_RespList[RESP_READPARAMETER].lCounter, 0);
    pdwID = (DWORD *)(pdata + sizeofW(TMHEADER));
    for (i = 0; i < u_RespList[RESP_READPARAMETER].wNoElements; i++)
    {
        memcpy(&dwID, pdwID, sizeof(DWORD));
        //(20140915 THJ(Modification: )
        //(20140915 THJ)u_RespList[RESP_READPARAMETER].atIndexList[i] = DataIDToIndex(dwID);
        //if (dwID >= MC_DBID_OFFSET)
        //  wIndex = MC_DataIDToIndex(dwID - MC_DBID_OFFSET);
        //else
        {
            wIndex = DataIDToIndex(dwID);
            //SetDBReadExecuteFunc((DBIndex_T)wIndex);
        }
        //)
        u_RespList[RESP_READPARAMETER].atIndexList[i] = (DBIndex_T)wIndex;
        u_RespList[RESP_READPARAMETER].adwIDList[i]   = dwID; // Test, Anders 2014-4-15.
        pdwID++;
    }
    // Anders 2014-4-14, add.
    ++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXHMIREADREFVALUECOUNT;
    //shanll 20200617 move
    u_RespList[RESP_READPARAMETER].wFlag = 1;
    ResetTimer(&u_RespList[RESP_READPARAMETER].lCounter, 0);
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqWriteRefValue(int nSessionHandle, WORD * pdata, WORD wlength)
{
    TMHEADER * pHeader = (TMHEADER *)pdata;
    WORD       wNoElements; //, wLength;
    WORD *     pwRxData, *pwTxData, wIndex;
    DWORD      dwID;
    WORD       wLength;
    int        i;
    DBVALUE    dbValue;

    memset(u_TempBuffer.awData, 0, sizeof(u_TempBuffer.awData));
    wNoElements = pHeader->wNoElements;
    pwRxData    = (WORD *)(pdata + sizeofW(TMHEADER));
    pwTxData    = (WORD *)u_TempBuffer.awData;
    for (i = 0; i < wNoElements; i++)
    {
        //  ID
        memcpy(&dwID, pwRxData, sizeof(DWORD));
        memcpy(pwTxData, &dwID, sizeof(DWORD));

        pwRxData += sizeofW(DWORD);
        pwTxData += sizeofW(DWORD);

        // Value
        memcpy(&dbValue, pwRxData + sizeofW(WORD), sizeof(dbValue));

        *pwTxData = (*pwRxData & 0xFF) | (1 << 8);

        wIndex = DB_IDToIndex(dwID);
        if (DBRETCODE_SUCCESS != DB_SetDataByUserIndex((DBIndex_T)wIndex, &dbValue)) //(DB_SUCCESS != SetDBValueByIndex(wDBIndex ,dbValue))
        {
            *pwTxData = (*pwRxData & 0xFF);
            memset(&dbValue, 0, sizeof(dbValue));
            DB_GetDataByUserIndex((DBIndex_T)wIndex, &dbValue);
        }

        pwTxData += sizeofW(WORD);
        pwRxData += sizeofW(WORD);
        memcpy(pwTxData, &dbValue, sizeof(dbValue));
        pwRxData += sizeofW(DWORD);
        pwTxData += sizeofW(DWORD);
    }
    wLength                                    = 2 * (sizeofW(TMHEADER) + TM_PARADATA_WSIZE * wNoElements) + 2;
    u_TempBuffer.TMHeader.wSize                = wLength;
    u_TempBuffer.TMHeader.wCounter             = 0;
    u_TempBuffer.TMHeader.datatype.wCounterRet = u_wRespWriteRecCounter++;
    u_TempBuffer.TMHeader.wType                = RESPONSE_WRITE_REFERENCE_ID;
    u_TempBuffer.TMHeader.wNoElements          = wNoElements;

    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    //if (g_wTTHMI)
    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);

    // Anders 2014-4-14, add.
    ++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXHMIWRITEREFVALUECOUNT;
}
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqMsgValue(int nSessionHandle, WORD * pdata, WORD wlength)
{
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqCurveValue(int nSessionHandle, WORD * pdata, WORD wlength)
{
    //int       i;
    //TM_CURVEDETAIL    CurveDetail;
    //TM_CURVEREQ   *pCurveReq = (TM_CURVEREQ *)pdata;
    //
    //if (pdata == NULL || \
    //  wlength == 0 || \
    //  pCurveReq->wDataItems == 0 || \
    //  pCurveReq->wDataItems > MAX_CURVE_ITEMS) return;
    //
    //if (pCurveReq->wInterval > 0)
    //{
    //  CurveDetail.wInterval = pCurveReq->wInterval;
    //  CurveDetail.wDataItems = pCurveReq->wDataItems;
    //  for (i=0; i<CurveDetail.wDataItems; i++)
    //      CurveDetail.awDataIndex[i] = (WORD)DataIDToIndex(pCurveReq->adwDataID[i]);
    //
    //  RegisterCurve(pCurveReq->wCurveMotionID, &CurveDetail);
    //}
    //else
    //{
    //  UnRegisterCurve(pCurveReq->wCurveMotionID);
    //}
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqCommand(int nSessionHandle, WORD * pdata, WORD wlength)
{
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespActValue(int nSessionHandle)
{
    WORD * pwData = (WORD *)u_TempBuffer.awData;
    int    i;
    WORD   wLength, wIndex;
    DWORD  dwID, dwData;

    memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
    if (u_RespList[RESP_ACTUAL].wNoElements == 0)
        return;
    wLength                                    = 2 * (sizeofW(TMHEADER) + TM_PARADATA_WSIZE * u_RespList[RESP_ACTUAL].wNoElements) + 2;
    u_TempBuffer.TMHeader.wHeader              = HEADER_RES;
    u_TempBuffer.TMHeader.wSize                = wLength;
    u_TempBuffer.TMHeader.wCounter             = 0;
    u_TempBuffer.TMHeader.wType                = RESPONSE_ACTUAL_ID;
    u_TempBuffer.TMHeader.datatype.wCounterRet = u_wRespActCounter++;
    u_TempBuffer.TMHeader.wNoElements          = u_RespList[RESP_ACTUAL].wNoElements;

    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    for (i = 0; i < u_RespList[RESP_ACTUAL].wNoElements; i++)
    {
        wIndex = u_RespList[RESP_ACTUAL].atIndexList[i];
        dwID   = u_RespList[RESP_ACTUAL].adwIDList[i];
        //dwID = DB_IndexToID(wIndex);

        memcpy(pwData, &dwID, sizeof(DWORD));
        pwData += sizeofW(DWORD);
        //(20150115 THJ(Modification:))
        //(20150115 THJ)if (wIndex < DBINDEX_SUM)
        //(20150115 THJ){
        //(20150115 THJ)    // Data type and state
        //(20150115 THJ)    *pwData =   (DB_GetDataTypeByIndex((DBIndex_T)wIndex) & 0xFF) | (1<<8);                     // State = 1
        //(20150115 THJ)    pwData  +=  sizeofW(WORD);
        //(20150115 THJ)    // Value
        //(20150115 THJ)    DB_GetData((DBIndex_T)wIndex, pwData);//memcpy(pwData, &DBData[wIndex], sizeof(DBVALUE));
        //(20150115 THJ)    pwData += sizeofW(DWORD);
        //(20150115 THJ)}
        //(20150115 THJ)else
        //(20150115 THJ){
        //(20150115 THJ)    // Data type and state
        //(20150115 THJ)    *pwData =   DB_GetDataTypeByIndex((DBIndex_T)wIndex) & 0xFF;                                // State = 0
        //(20150115 THJ)    pwData  +=  sizeofW(WORD);
        //(20150115 THJ)    // Value
        //(20150115 THJ)    pwData += sizeofW(DWORD);
        //(20150115 THJ)}
        //if (dwID >= MC_DBID_OFFSET)
        //{
        //  *pwData = MC_DataAttr[wIndex].DBAttrInfo.bit.bDataType | (1 << 8);
        //  dwData = MC_GetDBValueByIndex(wIndex);
        //}
        //else
        {
            *pwData = DB_GetDBDataType((DBIndex_T)wIndex);
            dwData  = 0;
            if (DBRETCODE_SUCCESS == DB_GetDataByUserIndex((DBIndex_T)wIndex, &dwData)) //dwData = GetDBValueByIndex(wIndex).dwData;
            {
                //WORD wAxis, wUpIndex;

                *pwData |= (1 << 8);
                //for (wAxis = 0; wAxis < AXISCARDAXIS_MAX; ++wAxis)                    // Anders 2017-3-21, mark.
                //{
                //    if (wIndex == gc_asAxisDBIndex[AXISDBINDEX_AXIS1_INTERFACE_ERROR1].atIndex[wAxis])
                //    {
                //        Comm_ClrReadFlag(wIndex);
                //        break;
                //    }
                //    for (wUpIndex = 0; wUpIndex < sizeof(gc_aeDataUpdateByIndex); ++wUpIndex)
                //    {
                //        if (wIndex == gc_asAxisDBIndex[gc_aeDataUpdateByIndex[wUpIndex]].atIndex[wAxis])
                //        {
                //            Comm_ClrReadFlag(wIndex);
                //            wAxis = AXISCARDAXIS_MAX;
                //            break;
                //        }
                //    }
                //}
            }
        }
        pwData += sizeofW(WORD);
        memcpy(pwData, &dwData, sizeof(DWORD));
        pwData += sizeofW(DWORD);
        //)
    }
    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);
    // Anders 2014-4-16, add.
    ++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWTXHMIACTVALUECOUNT;
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespRefValue(int nSessionHandle)
{
    int    i;
    WORD * pwData = (WORD *)u_TempBuffer.awData;

    WORD  wLength, wIndex;
    DWORD dwID, dwData;

    memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
    if (u_RespList[RESP_READPARAMETER].wNoElements == 0)
        return;
    wLength                                    = 2 * (sizeofW(TMHEADER) + TM_PARADATA_WSIZE * u_RespList[RESP_READPARAMETER].wNoElements) + 2;
    u_TempBuffer.TMHeader.wHeader              = HEADER_RES;
    u_TempBuffer.TMHeader.wSize                = wLength;
    u_TempBuffer.TMHeader.wCounter             = 0;
    u_TempBuffer.TMHeader.wType                = RESPONSE_READ_REFERENCE_ID;
    u_TempBuffer.TMHeader.datatype.wCounterRet = u_wRespReadRefCounter++;
    u_TempBuffer.TMHeader.wNoElements          = u_RespList[RESP_READPARAMETER].wNoElements;

    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    for (i = 0; i < u_RespList[RESP_READPARAMETER].wNoElements; i++)
    {
        wIndex = u_RespList[RESP_READPARAMETER].atIndexList[i];
        dwID   = u_RespList[RESP_READPARAMETER].adwIDList[i];
        //dwID = DB_IndexToID(wIndex);

        memcpy(pwData, &dwID, sizeof(DWORD));
        pwData += sizeofW(DWORD);
        //(20150115 THJ(Modification:)
        //(20150115 THJ)if (wIndex < DBINDEX_SUM)
        //(20150115 THJ){
        //(20150115 THJ)    // Data type and state
        //(20150115 THJ)    *pwData = (DB_GetDataTypeByIndex((DBIndex_T)wIndex) & 0xFF) | (1<<8);                       // State = 1
        //(20150115 THJ)    pwData += sizeofW(WORD);
        //(20150115 THJ)    // Value
        //(20150115 THJ)    DB_GetData((DBIndex_T)wIndex, pwData);//memcpy(pwData, &DBData[wIndex], sizeof(DBVALUE));
        //(20150115 THJ)    pwData += sizeofW(DWORD);
        //(20150115 THJ)}
        //(20150115 THJ)else
        //(20150115 THJ){
        //(20150115 THJ)    // Data type and state
        //(20150115 THJ)    *pwData =   _UNKNOWN; //DB_GetDataTypeByIndex(wIndex) & 0xFF;       // State = 0
        //(20150115 THJ)    pwData  +=  sizeofW(WORD);
        //(20150115 THJ)
        //(20150115 THJ)    // Value
        //(20150115 THJ)    pwData += sizeofW(DWORD);
        //(20150115 THJ)}
        //if (dwID >= MC_DBID_OFFSET)
        //{
        //  *pwData = MC_DataAttr[wIndex].DBAttrInfo.bit.bDataType | (1 << 8);
        //  dwData = MC_GetDBValueByIndex(wIndex);
        //}
        //else
        {
            *pwData = DB_GetDBDataType((DBIndex_T)wIndex);
            dwData  = 0;
            if (DBRETCODE_SUCCESS == DB_GetDataByUserIndex((DBIndex_T)wIndex, &dwData)) //dwData     =   GetDBValueByIndex(wIndex).dwData;
            {
                //WORD wAxis, wUpIndex;

                *pwData |= (1 << 8);
                //for (wAxis = 0; wAxis < AXISCARDAXIS_MAX; ++wAxis)                    // Anders 2017-3-21, mark.
                //{
                //    if (wIndex == gc_asAxisDBIndex[AXISDBINDEX_AXIS1_INTERFACE_ERROR1].atIndex[wAxis])
                //    {
                //        Comm_ClrReadFlag(wIndex);
                //        break;
                //    }
                //    for (wUpIndex = 0; wUpIndex < sizeof(gc_aeDataUpdateByIndex); ++wUpIndex)
                //    {
                //        if (wIndex == gc_asAxisDBIndex[gc_aeDataUpdateByIndex[wUpIndex]].atIndex[wAxis])
                //        {
                //            Comm_ClrReadFlag(wIndex);
                //            wAxis = AXISCARDAXIS_MAX;
                //            break;
                //        }
                //    }
                //}
            }
        }
        pwData += sizeofW(WORD);
        memcpy(pwData, &dwData, sizeof(DWORD));
        pwData += sizeofW(DWORD);
        //)
    }
    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);
    // Anders 2014-4-16, add.
    ++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWTXHMIREFVALUECOUNT;
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//void  RespMsgValue(int nSessionHandle)
//{
//  WORD    wLength;
//  RESPMSG *pRespMsg = (RESPMSG    *)u_TempBuffer.awData;
//  memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
//
//  wLength = 2*(sizeofW(TMHEADER) + 2) + 2;
//  u_TempBuffer.TMHeader.wHeader               =   HEADER_RES;
//  u_TempBuffer.TMHeader.wSize                 =   wLength;
//  u_TempBuffer.TMHeader.wCounter              =   0;
//  u_TempBuffer.TMHeader.wType                 =   RESPONSE_MESSAGE_ID;
//  u_TempBuffer.TMHeader.datatype.wCounterRet  =   u_wRespAlarmCounter++;
//  u_TempBuffer.TMHeader.wNoElements           =   1;
//  pRespMsg->wMsgID                            =   MsgState.wMsgID;
//  pRespMsg->msgtype.all                       =   MsgState.wMsgType;
//  *((WORD *)&u_TempBuffer + (wLength>>1) - 1) = FOOTER_RES;
//  MsgState.wFlag = 0;
//
//  //SendNetData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
//  SendRetransmitData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
//}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespCurveValue(int nSessionHandle) // Anders 20100426
{
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespCommand(int nSessionHandle) // zholy080821
{
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
#ifdef _DEBUG_NET
void SendTestBuffer(int nSessionHandle, WORD wTestType)
{
}
#endif
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqLinkState(int nSessionHandle, WORD * pdata, WORD wlength)
{
    WORD       wLength;
    long       lCounter;
    TMHEADER * pHeader = (TMHEADER *)pdata;

    if (pHeader->wNoElements != 1)
        return;
    //memcpy(&lCounter, (pdata + sizeofW(TMHEADER)), sizeof(long));
    //ResetTimer(&LinkState.lCounter, 200000000L ); // Anders 2015-4-13, Mark
    //LinkState.wState    =   1;

    if (MoldsetState.wReceiveState == 1 && MoldsetState.wFinished == 0)
        RespMoldsetState(nSessionHandle);

    memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
    wLength                                    = 2 * (sizeofW(TMHEADER) + 2) + 2;
    u_TempBuffer.TMHeader.wHeader              = HEADER_RES;
    u_TempBuffer.TMHeader.wSize                = wLength;
    u_TempBuffer.TMHeader.wCounter             = 0;
    u_TempBuffer.TMHeader.wType                = RESPONSE_LINKSTATE_ID;
    u_TempBuffer.TMHeader.datatype.wCounterRet = 0;
    u_TempBuffer.TMHeader.wNoElements          = 1;

    memcpy(&u_TempBuffer.awData, &lCounter, sizeof(long));
    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);

    // Anders 2014-4-14, add.
    ++g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXHMILINKCOUNT;
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespLinkState(int nSessionHandle)
{
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//WORD  GetLinkState()
//{
//  return  LinkState.wState;
//  // return   g_BootState.wFlag;
//}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//void  PostMsg(WORD wMsgID,WORD wMsgType)
//{
//  MsgState.wFlag      =   1;
//  MsgState.wMsgID     =   wMsgID;
//    MsgState.wMsgType     =   wMsgType;
//    //StartSendOnceData(ONCEDATAID_POSTMSG);
//}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqMoldsetState(int nSessionHandle, WORD * pdata, WORD wlength)
{
    //struct
    //{
    //    DBIndex_T tDBIndex;
    //    WORD      wValue;
    //} const asEntriesInitTab[] = {
    //    { DBINDEX_MOLD0_INTERFACE_JOG_OPEN_NUMBER_OF_ENTRIES, 6 },                // MOLD_INTERFACE_JOG_OPEN_PRESS_DOWN_RAMP                      - MOLD_INTERFACE_JOG_OPEN_NUMBER_OF_ENTRIES                     },
    //    { DBINDEX_MOLD0_INTERFACE_JOG_CLOSE_NUMBER_OF_ENTRIES, 6 },               // MOLD_INTERFACE_JOG_CLOSE_PRESS_DOWN_RAMP                     - MOLD_INTERFACE_JOG_CLOSE_NUMBER_OF_ENTRIES                    },
    //    { DBINDEX_MOLD0_INTERFACE_OPEN_NUMBER_OF_ENTRIES, 37 },                   // MOLD_INTERFACE_OPEN_AUTO_ADJUST_POSITION                     - MOLD_INTERFACE_OPEN_NUMBER_OF_ENTRIES                         },
    //    { DBINDEX_MOLD0_INTERFACE_CLOSE_NUMBER_OF_ENTRIES, 36 },                  // MOLD_INTERFACE_CLOSE_PROFILE5_PRESS_DOWN_RAMP                    - MOLD_INTERFACE_CLOSE_NUMBER_OF_ENTRIES                        },
    //    { DBINDEX_MOLD0_INTERFACE_OPEN_PARAM_NUMBER_OF_ENTRIES, 27 },             // MOLD_INTERFACE_OPEN_PARAM_CORE6_END_IO                           - MOLD_INTERFACE_OPEN_PARAM_NUMBER_OF_ENTRIES                   },
    //    { DBINDEX_MOLD0_INTERFACE_CLOSE_PARAM_NUMBER_OF_ENTRIES, 31 },            // MOLD_INTERFACE_CLOSE_PARAM_CORE6_END_IO                      - MOLD_INTERFACE_CLOSE_PARAM_NUMBER_OF_ENTRIES                  },
    //    { DBINDEX_MOLD0_INTERFACE_COINING_NUMBER_OF_ENTRIES, 36 },                // MOLD_INTERFACE_COINING_PROFILE5_DECELERATION                 - MOLD_INTERFACE_COINING_NUMBER_OF_ENTRIES                      },
    //    { DBINDEX_EJECT0_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, 6 },            // EJECT_INTERFACE_JOG_FORWARD_PRESS_DOWN_RAMP                  - EJECT_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES                 },
    //    { DBINDEX_EJECT0_INTERFACE_JOG_EJECT_BACK_NUMBER_OF_ENTRIES, 6 },         // EJECT_INTERFACE_JOG_EJECT_BACK_PRESS_DOWN_RAMP                   - EJECT_INTERFACE_JOG_EJECT_BACK_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_EJECT0_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, 37 },               // EJECT_INTERFACE_FORWARD_DELAY_TIME                               - EJECT_INTERFACE_FORWARD_NUMBER_OF_ENTRIES                     },
    //    { DBINDEX_EJECT0_INTERFACE_EJECT_BACK_NUMBER_OF_ENTRIES, 36 },            // EJECT_INTERFACE_EJECT_BACK_DELAY_TIME                            - EJECT_INTERFACE_EJECT_BACK_NUMBER_OF_ENTRIES                  },
    //    { DBINDEX_EJECT0_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, 27 },         // EJECT_INTERFACE_FORWARD_PARAM_CLOSE_MOLD_END_IO              - EJECT_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES               },
    //    { DBINDEX_EJECT0_INTERFACE_EJECT_BACK_PARAM_NUMBER_OF_ENTRIES, 31 },      // EJECT_INTERFACE_EJECT_BACK_PARAM_CLOSE_MOLD_END_IO               - EJECT_INTERFACE_EJECT_BACK_PARAM_NUMBER_OF_ENTRIES            },
    //    { DBINDEX_INJECT0_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, 6 },           // INJECT_INTERFACE_JOG_FORWARD_PRESS_DOWN_RAMP                 - INJECT_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES                },
    //    { DBINDEX_INJECT0_INTERFACE_JOG_SUCK_BACK_NUMBER_OF_ENTRIES, 6 },         // INJECT_INTERFACE_JOG_SUCK_BACK_PRESS_DOWN_RAMP                   - INJECT_INTERFACE_JOG_SUCK_BACK_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_INJECT0_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, 92 },              // INJECT_INTERFACE_FORWARD_INJECT_PROFILE6_PRESS_DOWN_RAMP     - INJECT_INTERFACE_FORWARD_NUMBER_OF_ENTRIES                    },
    //    { DBINDEX_INJECT0_INTERFACE_PRE_SUCK_BACK_NUMBER_OF_ENTRIES, 7 },         // INJECT_INTERFACE_PRE_SUCK_BACK_PRESS_DOWN_RAMP                   - INJECT_INTERFACE_PRE_SUCK_BACK_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_INJECT0_INTERFACE_SUCK_BACK_NUMBER_OF_ENTRIES, 7 },             // INJECT_INTERFACE_SUCK_BACK_PRESS_DOWN_RAMP                       - INJECT_INTERFACE_SUCK_BACK_NUMBER_OF_ENTRIES                  },
    //    { DBINDEX_INJECT0_INTERFACE_CONSTANT_BACK_NUMBER_OF_ENTRIES, 7 },         // INJECT_INTERFACE_CONSTANT_BACK_PRESS_DOWN_RAMP                   - INJECT_INTERFACE_CONSTANT_BACK_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_INJECT0_INTERFACE_BACK_PRESS_NUMBER_OF_ENTRIES, 36 },           // INJECT_INTERFACE_BACK_PRESS_PROFILE5_DECELERATION                - INJECT_INTERFACE_BACK_PRESS_NUMBER_OF_ENTRIES                 },
    //    { DBINDEX_INJECT0_INTERFACE_PURE_INJECT_NUMBER_OF_ENTRIES, 7 },           // INJECT_INTERFACE_PURE_INJECT_PROFILE5_PRESS_DOWN_RAMP            - INJECT_INTERFACE_PURE_INJECT_NUMBER_OF_ENTRIES                },
    //    { DBINDEX_INJECT0_INTERFACE_PURE_SUCK_BACK_NUMBER_OF_ENTRIES, 7 },        // INJECT_INTERFACE_PURE_SUCK_BACK_PROFILE5_PRESS_DOWN_RAMP     - INJECT_INTERFACE_PURE_SUCK_BACK_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_INJECT0_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, 15 },        // INJECT_INTERFACE_FORWARD_PARAM_DIRECTION_VALVE_IO                - INJECT_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_INJECT0_INTERFACE_SUCK_BACK_PARAM_NUMBER_OF_ENTRIES, 12 },      // INJECT_INTERFACE_SUCK_BACK_PARAM_DIRECTION_VALVE_IO          - INJECT_INTERFACE_SUCK_BACK_PARAM_NUMBER_OF_ENTRIES            },
    //    { DBINDEX_CHARGE0_INTERFACE_SLOW_NUMBER_OF_ENTRIES, 6 },                  // CHARGE_INTERFACE_SLOW_PRESS_DOWN_RAMP                            - CHARGE_INTERFACE_SLOW_NUMBER_OF_ENTRIES                       },
    //    { DBINDEX_CHARGE0_INTERFACE_NUMBER_OF_ENTRIES, 36 },                      // CHARGE_INTERFACE_PROFILE5_PRESS_DOWN_RAMP                        - CHARGE_INTERFACE_NUMBER_OF_ENTRIES                            },
    //    { DBINDEX_CHARGE0_INTERFACE_PURE_CHARGE_NUMBER_OF_ENTRIES, 7 },           // CHARGE_INTERFACE_PURE_CHARGE_PRESS_DOWN_RAMP                 - CHARGE_INTERFACE_PURE_CHARGE_NUMBER_OF_ENTRIES                },
    //    { DBINDEX_CHARGE0_INTERFACE_PARAM_NUMBER_OF_ENTRIES, 4 },                 // CHARGE_INTERFACE_PARAM_DIRECTION_VALVE_IO                        - CHARGE_INTERFACE_PARAM_NUMBER_OF_ENTRIES                      },
    //    { DBINDEX_CARRIAGE0_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, 6 },         // CARRIAGE_INTERFACE_JOG_FORWARD_PRESS_DOWN_RAMP                   - CARRIAGE_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_CARRIAGE0_INTERFACE_JOG_CARRIAGE_BACK_NUMBER_OF_ENTRIES, 6 },   // CARRIAGE_INTERFACE_JOG_CARRIAGE_BACK_PRESS_DOWN_RAMP         - CARRIAGE_INTERFACE_JOG_CARRIAGE_BACK_NUMBER_OF_ENTRIES        },
    //    { DBINDEX_CARRIAGE0_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, 17 },            // CARRIAGE_INTERFACE_FORWARD_TOUCH_FORCE                           - CARRIAGE_INTERFACE_FORWARD_NUMBER_OF_ENTRIES                  },
    //    { DBINDEX_CARRIAGE0_INTERFACE_CARRIAGE_BACK_NUMBER_OF_ENTRIES, 15 },      // CARRIAGE_INTERFACE_CARRIAGE_BACK_PROFILE2_PRESS_DOWN_RAMP        - CARRIAGE_INTERFACE_CARRIAGE_BACK_NUMBER_OF_ENTRIES            },
    //    { DBINDEX_CARRIAGE0_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, 6 },       // CARRIAGE_INTERFACE_FORWARD_PARAM_FORWARD_END_IO              - CARRIAGE_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES            },
    //    { DBINDEX_CARRIAGE0_INTERFACE_CARRIAGE_BACK_PARAM_NUMBER_OF_ENTRIES, 5 }, // CARRIAGE_INTERFACE_CARRIAGE_BACK_PARAM_BACK_END_IO               - CARRIAGE_INTERFACE_CARRIAGE_BACK_PARAM_NUMBER_OF_ENTRIES      },
    //    { DBINDEX_MOLD1_INTERFACE_JOG_OPEN_NUMBER_OF_ENTRIES, 6 },                // MOLD2_INTERFACE_JOG_OPEN_PRESS_DOWN_RAMP                     - MOLD2_INTERFACE_JOG_OPEN_NUMBER_OF_ENTRIES                    },
    //    { DBINDEX_MOLD1_INTERFACE_JOG_CLOSE_NUMBER_OF_ENTRIES, 6 },               // MOLD2_INTERFACE_JOG_CLOSE_PRESS_DOWN_RAMP                        - MOLD2_INTERFACE_JOG_CLOSE_NUMBER_OF_ENTRIES                   },
    //    { DBINDEX_MOLD1_INTERFACE_OPEN_NUMBER_OF_ENTRIES, 37 },                   // MOLD2_INTERFACE_OPEN_AUTO_ADJUST_POSITION                        - MOLD2_INTERFACE_OPEN_NUMBER_OF_ENTRIES                        },
    //    { DBINDEX_MOLD1_INTERFACE_CLOSE_NUMBER_OF_ENTRIES, 36 },                  // MOLD2_INTERFACE_CLOSE_PROFILE5_PRESS_DOWN_RAMP                   - MOLD2_INTERFACE_CLOSE_NUMBER_OF_ENTRIES                       },
    //    { DBINDEX_MOLD1_INTERFACE_OPEN_PARAM_NUMBER_OF_ENTRIES, 27 },             // MOLD2_INTERFACE_OPEN_PARAM_CORE6_END_IO                      - MOLD2_INTERFACE_OPEN_PARAM_NUMBER_OF_ENTRIES                  },
    //    { DBINDEX_MOLD1_INTERFACE_CLOSE_PARAM_NUMBER_OF_ENTRIES, 31 },            // MOLD2_INTERFACE_CLOSE_PARAM_CORE6_END_IO                     - MOLD2_INTERFACE_CLOSE_PARAM_NUMBER_OF_ENTRIES                 },
    //    { DBINDEX_MOLD1_INTERFACE_COINING_NUMBER_OF_ENTRIES, 36 },                // MOLD2_INTERFACE_COINING_PROFILE5_DECELERATION                    - MOLD2_INTERFACE_COINING_NUMBER_OF_ENTRIES                     },
    //    { DBINDEX_EJECT1_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, 6 },            // EJECT2_INTERFACE_JOG_FORWARD_PRESS_DOWN_RAMP                 - EJECT2_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES                },
    //    { DBINDEX_EJECT1_INTERFACE_JOG_EJECT_BACK_NUMBER_OF_ENTRIES, 6 },         // EJECT2_INTERFACE_JOG_EJECT_BACK_PRESS_DOWN_RAMP              - EJECT2_INTERFACE_JOG_EJECT_BACK_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_EJECT1_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, 37 },               // EJECT2_INTERFACE_FORWARD_DELAY_TIME                          - EJECT2_INTERFACE_FORWARD_NUMBER_OF_ENTRIES                    },
    //    { DBINDEX_EJECT1_INTERFACE_EJECT_BACK_NUMBER_OF_ENTRIES, 36 },            // EJECT2_INTERFACE_EJECT_BACK_DELAY_TIME                           - EJECT2_INTERFACE_EJECT_BACK_NUMBER_OF_ENTRIES                 },
    //    { DBINDEX_EJECT1_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, 27 },         // EJECT2_INTERFACE_FORWARD_PARAM_CLOSE_MOLD_END_IO             - EJECT2_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES              },
    //    { DBINDEX_EJECT1_INTERFACE_EJECT_BACK_PARAM_NUMBER_OF_ENTRIES, 31 },      // EJECT2_INTERFACE_EJECT_BACK_PARAM_CLOSE_MOLD_END_IO          - EJECT2_INTERFACE_EJECT_BACK_PARAM_NUMBER_OF_ENTRIES           },
    //    { DBINDEX_INJECT1_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, 6 },           // INJECT2_INTERFACE_JOG_FORWARD_PRESS_DOWN_RAMP                    - INJECT2_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES               },
    //    { DBINDEX_INJECT1_INTERFACE_JOG_SUCK_BACK_NUMBER_OF_ENTRIES, 6 },         // INJECT2_INTERFACE_JOG_SUCK_BACK_PRESS_DOWN_RAMP              - INJECT2_INTERFACE_JOG_SUCK_BACK_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_INJECT1_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, 92 },              // INJECT2_INTERFACE_FORWARD_INJECT_PROFILE6_PRESS_DOWN_RAMP        - INJECT2_INTERFACE_FORWARD_NUMBER_OF_ENTRIES                   },
    //    { DBINDEX_INJECT1_INTERFACE_PRE_SUCK_BACK_NUMBER_OF_ENTRIES, 7 },         // INJECT2_INTERFACE_PRE_SUCK_BACK_PRESS_DOWN_RAMP              - INJECT2_INTERFACE_PRE_SUCK_BACK_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_INJECT1_INTERFACE_SUCK_BACK_NUMBER_OF_ENTRIES, 7 },             // INJECT2_INTERFACE_SUCK_BACK_PRESS_DOWN_RAMP                  - INJECT2_INTERFACE_SUCK_BACK_NUMBER_OF_ENTRIES                 },
    //    { DBINDEX_INJECT1_INTERFACE_CONSTANT_BACK_NUMBER_OF_ENTRIES, 7 },         // INJECT2_INTERFACE_CONSTANT_BACK_PRESS_DOWN_RAMP              - INJECT2_INTERFACE_CONSTANT_BACK_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_INJECT1_INTERFACE_BACK_PRESS_NUMBER_OF_ENTRIES, 36 },           // INJECT2_INTERFACE_BACK_PRESS_PROFILE5_DECELERATION               - INJECT2_INTERFACE_BACK_PRESS_NUMBER_OF_ENTRIES                },
    //    { DBINDEX_INJECT1_INTERFACE_PURE_INJECT_NUMBER_OF_ENTRIES, 7 },           // INJECT2_INTERFACE_PURE_INJECT_PROFILE5_PRESS_DOWN_RAMP           - INJECT2_INTERFACE_PURE_INJECT_NUMBER_OF_ENTRIES               },
    //    { DBINDEX_INJECT1_INTERFACE_PURE_SUCK_BACK_NUMBER_OF_ENTRIES, 7 },        // INJECT2_INTERFACE_PURE_SUCK_BACK_PROFILE5_PRESS_DOWN_RAMP        - INJECT2_INTERFACE_PURE_SUCK_BACK_NUMBER_OF_ENTRIES            },
    //    { DBINDEX_INJECT1_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, 15 },        // INJECT2_INTERFACE_FORWARD_PARAM_DIRECTION_VALVE_IO               - INJECT2_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_INJECT1_INTERFACE_SUCK_BACK_PARAM_NUMBER_OF_ENTRIES, 12 },      // INJECT2_INTERFACE_SUCK_BACK_PARAM_DIRECTION_VALVE_IO         - INJECT2_INTERFACE_SUCK_BACK_PARAM_NUMBER_OF_ENTRIES           },
    //    { DBINDEX_CHARGE1_INTERFACE_SLOW_NUMBER_OF_ENTRIES, 6 },                  // CHARGE2_INTERFACE_SLOW_PRESS_DOWN_RAMP                           - CHARGE2_INTERFACE_SLOW_NUMBER_OF_ENTRIES                      },
    //    { DBINDEX_CHARGE1_INTERFACE_NUMBER_OF_ENTRIES, 36 },                      // CHARGE2_INTERFACE_PROFILE5_PRESS_DOWN_RAMP                       - CHARGE2_INTERFACE_NUMBER_OF_ENTRIES                           },
    //    { DBINDEX_CHARGE1_INTERFACE_PURE_CHARGE_NUMBER_OF_ENTRIES, 7 },           // CHARGE2_INTERFACE_PURE_CHARGE_PRESS_DOWN_RAMP                    - CHARGE2_INTERFACE_PURE_CHARGE_NUMBER_OF_ENTRIES               },
    //    { DBINDEX_CHARGE1_INTERFACE_PARAM_NUMBER_OF_ENTRIES, 4 },                 // CHARGE2_INTERFACE_PARAM_DIRECTION_VALVE_IO                       - CHARGE2_INTERFACE_PARAM_NUMBER_OF_ENTRIES                     },
    //    { DBINDEX_CARRIAGE1_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, 6 },         // CARRIAGE2_INTERFACE_JOG_FORWARD_PRESS_DOWN_RAMP              - CARRIAGE2_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES             },
    //    { DBINDEX_CARRIAGE1_INTERFACE_JOG_CARRIAGE_BACK_NUMBER_OF_ENTRIES, 6 },   // CARRIAGE2_INTERFACE_JOG_CARRIAGE_BACK_PRESS_DOWN_RAMP            - CARRIAGE2_INTERFACE_JOG_CARRIAGE_BACK_NUMBER_OF_ENTRIES       },
    //    { DBINDEX_CARRIAGE1_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, 17 },            // CARRIAGE2_INTERFACE_FORWARD_TOUCH_FORCE                      - CARRIAGE2_INTERFACE_FORWARD_NUMBER_OF_ENTRIES                 },
    //    { DBINDEX_CARRIAGE1_INTERFACE_CARRIAGE_BACK_NUMBER_OF_ENTRIES, 15 },      // CARRIAGE2_INTERFACE_CARRIAGE_BACK_PROFILE2_PRESS_DOWN_RAMP       - CARRIAGE2_INTERFACE_CARRIAGE_BACK_NUMBER_OF_ENTRIES           },
    //    { DBINDEX_CARRIAGE1_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, 6 },       // CARRIAGE2_INTERFACE_FORWARD_PARAM_FORWARD_END_IO             - CARRIAGE2_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES           },
    //    { DBINDEX_CARRIAGE1_INTERFACE_CARRIAGE_BACK_PARAM_NUMBER_OF_ENTRIES, 5 }, // CARRIAGE2_INTERFACE_CARRIAGE_BACK_PARAM_BACK_END_IO          - CARRIAGE2_INTERFACE_CARRIAGE_BACK_PARAM_NUMBER_OF_ENTRIES     },

    //    { DBINDEX_MOLD0_CONFIG_END, 1 },
    //    { DBINDEX_EJECT0_CONFIG_END, 1 },
    //    { DBINDEX_INJECT0_CONFIG_END, 1 },
    //    { DBINDEX_CHARGE0_CONFIG_END, 1 },
    //    { DBINDEX_CARRIAGE0_CONFIG_END, 1 },
    //    { DBINDEX_MOLD1_CONFIG_END, 1 },
    //    { DBINDEX_EJECT1_CONFIG_END, 1 },
    //    { DBINDEX_INJECT1_CONFIG_END, 1 },
    //    { DBINDEX_CHARGE1_CONFIG_END, 1 },
    //    { DBINDEX_CARRIAGE1_CONFIG_END, 1 },
    //};
    // DBVALUE   Temp = {0};
    TMHEADER * pHeader = (TMHEADER *)pdata;

    if (pHeader->wNoElements != 1)
        return;
    memcpy(&MoldsetState.lReqElements, (pdata + sizeofW(TMHEADER)), sizeof(long));
    if (MoldsetState.lReqElements == MoldsetState.lUpdateCount)
    {
        MoldsetState.wFinished = 1;
        RespMoldsetState(nSessionHandle);
        //StartSendOnceData(ONCEDATAID_MAXVELO);

        //(THJ 2015-8-20 Modify:
        // // Mold1
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( MOLD_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(MOLD_INTERFACE_JOG_OPEN_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(MOLD_INTERFACE_JOG_CLOSE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x25;
        // DB_SetDataBySysIndex(MOLD_INTERFACE_OPEN_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x24;
        // DB_SetDataBySysIndex(MOLD_INTERFACE_CLOSE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1B;
        // DB_SetDataBySysIndex(MOLD_INTERFACE_OPEN_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1F;
        // DB_SetDataBySysIndex(MOLD_INTERFACE_CLOSE_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x24;
        // DB_SetDataBySysIndex(MOLD_INTERFACE_COINING_NUMBER_OF_ENTRIES, &Temp);
        // // Eject1
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( EJECT_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(EJECT_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(EJECT_INTERFACE_JOG_EJECT_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x25;
        // DB_SetDataBySysIndex(EJECT_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x24;
        // DB_SetDataBySysIndex(EJECT_INTERFACE_EJECT_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1B;
        // DB_SetDataBySysIndex(EJECT_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1F;
        // DB_SetDataBySysIndex(EJECT_INTERFACE_EJECT_BACK_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Inject1
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex(INJECT_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(INJECT_INTERFACE_JOG_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   92;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_PRE_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(INJECT_INTERFACE_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(INJECT_INTERFACE_CONSTANT_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   36;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_BACK_PRESS_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_PURE_INJECT_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_PURE_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   15;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   12;
        // DB_SetDataBySysIndex(INJECT_INTERFACE_SUCK_BACK_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Charge1
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( CHARGE_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(CHARGE_INTERFACE_SLOW_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   36;
        // DB_SetDataBySysIndex(CHARGE_INTERFACE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(CHARGE_INTERFACE_PURE_CHARGE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   4;
        // DB_SetDataBySysIndex(CHARGE_INTERFACE_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Carriage1
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( CARRIAGE_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(CARRIAGE_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(CARRIAGE_INTERFACE_JOG_CARRIAGE_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   17;
        // DB_SetDataBySysIndex(CARRIAGE_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   15;
        // DB_SetDataBySysIndex(CARRIAGE_INTERFACE_CARRIAGE_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   6;
        // DB_SetDataBySysIndex(CARRIAGE_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   5;
        // DB_SetDataBySysIndex(CARRIAGE_INTERFACE_CARRIAGE_BACK_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Mold2
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( MOLD2_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_JOG_OPEN_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_JOG_CLOSE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x25;
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_OPEN_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x24;
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_CLOSE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1B;
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_OPEN_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1F;
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_CLOSE_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x24;
        // DB_SetDataBySysIndex(MOLD2_INTERFACE_COINING_NUMBER_OF_ENTRIES, &Temp);
        // // Eject2
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( EJECT2_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(EJECT2_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(EJECT2_INTERFACE_JOG_EJECT_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x25;
        // DB_SetDataBySysIndex(EJECT2_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x24;
        // DB_SetDataBySysIndex(EJECT2_INTERFACE_EJECT_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1B;
        // DB_SetDataBySysIndex(EJECT2_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   0x1F;
        // DB_SetDataBySysIndex(EJECT2_INTERFACE_EJECT_BACK_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Inject2
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex(INJECT2_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_JOG_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   92;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_PRE_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_CONSTANT_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   36;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_BACK_PRESS_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_PURE_INJECT_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_PURE_SUCK_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   15;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   12;
        // DB_SetDataBySysIndex(INJECT2_INTERFACE_SUCK_BACK_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Charge2
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( CHARGE2_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(CHARGE2_INTERFACE_SLOW_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   36;
        // DB_SetDataBySysIndex(CHARGE2_INTERFACE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   7;
        // DB_SetDataBySysIndex(CHARGE2_INTERFACE_PURE_CHARGE_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   4;
        // DB_SetDataBySysIndex(CHARGE2_INTERFACE_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // // Carriage2
        // Temp.wData     =   0x1;
        // DB_SetDataBySysIndex( CARRIAGE2_CONFIG_END, &Temp);
        // Temp.wData     =   0x6;
        // DB_SetDataBySysIndex(CARRIAGE2_INTERFACE_JOG_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // DB_SetDataBySysIndex(CARRIAGE2_INTERFACE_JOG_CARRIAGE_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   17;
        // DB_SetDataBySysIndex(CARRIAGE2_INTERFACE_FORWARD_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   15;
        // DB_SetDataBySysIndex(CARRIAGE2_INTERFACE_CARRIAGE_BACK_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   6;
        // DB_SetDataBySysIndex(CARRIAGE2_INTERFACE_FORWARD_PARAM_NUMBER_OF_ENTRIES, &Temp);
        // Temp.wData     =   5;
        // DB_SetDataBySysIndex(CARRIAGE2_INTERFACE_CARRIAGE_BACK_PARAM_NUMBER_OF_ENTRIES, &Temp);
        //
        // for (i = 0; i < (sizeof(asEntriesInitTab) / sizeof(*asEntriesInitTab)); ++i)
        //     DB_SetDataBySysIndex(asEntriesInitTab[i].tDBIndex, &asEntriesInitTab[i].wValue);
        //)
        //DSP28x_usDelay(1);
        // Test
        //printf("AXIS1_CONFIGURATION_END: %ld\n", MC_DBData[AXIS1_CONFIGURATION_END].dbValue.dwData);
        //if( MC_DBData[AXIS1_CONFIGURATION_END].dbValue.dwData == 0 )//169
        //  ESTOP0;
        StartTxTemperDataToMaster();
    }
    else
    {
        MoldsetState.wFinished = 0;
        RespMoldsetState(nSessionHandle);
    }
    MoldsetState.wReceiveState = 1;
    //  MoldsetState.lUpdateCount = 0;
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespMoldsetState(int nSessionHandle)
{
    WORD wLength;

    memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
    wLength                                    = 2 * (sizeofW(TMHEADER) + 2) + 2;
    u_TempBuffer.TMHeader.wHeader              = HEADER_RES;
    u_TempBuffer.TMHeader.wSize                = wLength;
    u_TempBuffer.TMHeader.wCounter             = 0;
    u_TempBuffer.TMHeader.wType                = RESPONSE_MOLDSETSTATE_ID;
    u_TempBuffer.TMHeader.datatype.wCounterRet = u_wRespMoldsetStateCounter++;
    u_TempBuffer.TMHeader.wNoElements          = 1;

    memcpy(&u_TempBuffer.awData, &MoldsetState.lUpdateCount, sizeof(long));
    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);
}
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void ReqMoldsetUpdate(int nSessionHandle, WORD * pdata, WORD wlength)
{
    TMHEADER * pHeader = (TMHEADER *)pdata;
    WORD       wNoElements, wLength;
    WORD *     pwRxData, *pwTxData;
    DWORD      dwID;
    DBIndex_T  tDBIndex;
    //WORD      dwValue = 0;
    int     i;
    DBVALUE dbvalue;

    //extern DWORD   g_dwTickWhile;
    //extern CPULOAD g_CPULoadWhile;
    // Resend moldset, the index will start at 1
    if (MoldsetState.wReceiveState == 1 || pHeader->wCounter == 1)
    {
        MoldsetState.lUpdateCount  = 0;
        MoldsetState.wReceiveState = 0;
        MoldsetState.wFinished     = 0;
        g_wTelegramCount           = 0;
    }

    memset(u_TempBuffer.awData, 0, sizeof(u_TempBuffer.awData));
    wNoElements = pHeader->wNoElements;

    pwRxData = (WORD *)(pdata + sizeofW(TMHEADER));
    pwTxData = (WORD *)u_TempBuffer.awData;

    //MC_DBData[CARD_INTERFACE_NET_DWRXHMIREADACTVALUECOUNT].dbValue.dwData = pHeader->wCounter;
    for (i = 0; i < wNoElements; i++)
    {
        //  ID
        memcpy(&dwID, pwRxData, sizeof(DWORD));
        memcpy(pwTxData, &dwID, sizeof(DWORD));

        pwRxData += sizeofW(DWORD);
        pwTxData += sizeofW(DWORD);

        // Value
        memcpy(&dbvalue, pwRxData + sizeofW(WORD), sizeof(dbvalue));

        //CalcCPULoadStart(&g_CPULoadWhile);
        // if (DB_SUCCESS != SetDBValue(dwID, dbvalue))
        tDBIndex = DB_IDToIndex(dwID);
        if (DBRETCODE_SUCCESS != DB_SetDataByUserIndex(tDBIndex, &dbvalue))
        {
            // Data type and state
            *pwTxData = *pwRxData & 0xFF; // State = 0
            pwTxData += sizeofW(WORD);
            pwRxData += sizeofW(WORD);

            // Value
            // dbvalue = GetDBValue(dwID);
            dbvalue.vDWORD = 0;
            if (tDBIndex < DBINDEX_SUM)
                DB_GetData(tDBIndex, &dbvalue);
            memcpy(pwTxData, &dbvalue, sizeof(DWORD));
            pwRxData += sizeofW(DWORD);
            pwTxData += sizeofW(DWORD);
        }
        else
        {
            // Data type and state
            *pwTxData = (*pwRxData & 0xFF) | (1 << 8); // State = 1
            pwTxData += sizeofW(WORD);
            pwRxData += sizeofW(WORD);

            // Value
            memcpy(pwTxData, &dbvalue, sizeof(dbvalue));
            pwRxData += sizeofW(DWORD);
            pwTxData += sizeofW(DWORD);
        }
        //CalcCPULoadEnd(&g_CPULoadWhile);
        //g_dwTickWhile = g_CPULoadWhile.dwCurrentCounter;
        //if ( g_dwTickWhile > 15000 )
        //    ESTOP0;
    }
    if (pHeader->wCounter > g_wTelegramCount) // Anders 2015-11-11,Modified. (pHeader->wCounter != g_wTelegramCount)
    {
        if (MoldsetState.wFinished == 0)
            MoldsetState.lUpdateCount += wNoElements;
        g_wTelegramCount = pHeader->wCounter;
    }
    // Anders 2014-4-16, add.
    g_sDB.LOCAL_AXISCARD.INTERFACE_NET_DWRXHMIMOLDSETCOUNT = MoldsetState.lUpdateCount;

#ifdef TEST_NET
    g_wTest_SendUpdateCount++;
#endif
    wLength                                       = 2 * (sizeofW(TMHEADER) + TM_PARADATA_WSIZE * wNoElements) + 2;
    u_TempBuffer.TMHeader.wSize                   = wLength;
    u_TempBuffer.TMHeader.wCounter                = g_wTelegramCount;
    u_TempBuffer.TMHeader.datatype.wCounterRet    = u_wRespMoldsetRecCounter++;
    u_TempBuffer.TMHeader.wType                   = RESPONSE_MOLDSETUPDATE_ID;
    u_TempBuffer.TMHeader.wNoElements             = wNoElements;
    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    //SendNetData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);
}
/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespMoldsetUpdate(int nSessionHandle)
{
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
WORD GetMoldsetState()
{
    // Test
    //if( MoldsetState.wFinished == 1 && MC_DBData[AXIS1_CONFIGURATION_END].dbValue.dwData == 0 )   //169
    //  ESTOP0;
    return (g_wMasterReady && MoldsetState.wFinished);
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
void RespRefValueAuto(int nSessionHandle, WORD * pDBID, WORD wNum)
{
    WORD * pwData = (WORD *)u_TempBuffer.awData;
    int    i;
    WORD   wLength;
    DWORD  dwID;

    if (pDBID == NULL || wNum == 0)
        return;

    memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
    wLength                                    = 2 * (sizeofW(TMHEADER) + TM_PARADATA_WSIZE * wNum) + 2;
    u_TempBuffer.TMHeader.wHeader              = HEADER_RES;
    u_TempBuffer.TMHeader.wSize                = wLength;
    u_TempBuffer.TMHeader.wCounter             = 0;
    u_TempBuffer.TMHeader.wType                = RESPONSE_READ_REFERENCE_ID;
    u_TempBuffer.TMHeader.datatype.wCounterRet = u_wRespReadRefCounter++;
    u_TempBuffer.TMHeader.wNoElements          = wNum;

    *((WORD *)&u_TempBuffer + (wLength >> 1) - 1) = FOOTER_RES;

    for (i = 0; i < wNum; i++)
    {
        // ID
        dwID = DB_IndexToID((DBIndex_T) * (pDBID + i));
        memcpy(pwData, &dwID, sizeof(DWORD));
        pwData += sizeofW(DWORD);

        // DataType
        *pwData = _WORD | (1 << 8);
        pwData += sizeofW(WORD);

        // Value
        if (*(pDBID + i) < DBINDEX_SUM)
        {
            DB_GetData((DBIndex_T) * (pDBID + i), pwData); //memcpy(pwData, &DBData[*(pDBID+i)], sizeof(DBVALUE));
        }
        pwData += sizeofW(DWORD);
    }
    //SendNetData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
    SendRetransmitData(nSessionHandle, (WORD *)&u_TempBuffer, wLength);
}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//void  RespRefValueAutoByDBIndex(int nSessionHandle,WORD *pDBIndex, WORD wNum) //Anders 20100421
//{
//  TM_PARADATA *pParaData = (TM_PARADATA *)u_TempBuffer.awData;
//  int     i;
//  WORD    wLength;
//
//  if (pDBIndex == NULL || wNum == 0) return;
//
//  memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
//  wLength = 2*(sizeofW(TMHEADER) + sizeofW(TM_PARADATA)*wNum)+2;
//  u_TempBuffer.TMHeader.wSize = wLength;
//  u_TempBuffer.TMHeader.wCounter = 0;
//  u_TempBuffer.TMHeader.wType =RESPONSE_READ_REFERENCE_ID;    // RESPONSE_ACTUAL_ID;  //RESPONSE_READ_REFERENCE_ID;
//  u_TempBuffer.TMHeader.datatype.wCounterRet = u_wRespReadRefCounter++;
//  u_TempBuffer.TMHeader.wNoElements = wNum;
//
//  *((WORD *)&u_TempBuffer + (wLength>>1) - 1) = FOOTER_RES;
//
//  for (i=0; i<wNum; i++)
//  {
//      pParaData->dwID = IndexToDataID(*(pDBIndex+i));
//      pParaData->stattype.bit.wState = 1;
//      pParaData->stattype.bit.wDataType = DBATTRI[*(pDBIndex+i)].wDataType;
//      pParaData->data.wData = GetDBValueByIndex(*(pDBIndex+i));
//      pParaData++;
//  }
//  //SendNetData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
//  SendRetransmitData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
//}
//================================================================================
//Anders 20100509
//void    StartSendOnceData(WORD  wIndex)
//{
//    if(!u_SendOnceList[wIndex].wFlag)
//    {
//        u_SendOnceList[wIndex].wFlag = 1;
//        u_SendOnceList[wIndex].lCounter = 0;
//    }
//}
//================================================================================
//void    ResetSendOnceDataFlag()
//{
//    WORD wOwner = GetOwner_Retransmit();
//    if(wOwner>=BASEADDR_TRISMITEDONCEVALUE && wOwner<BASEADDR_TRISMITEDONCEVALUE+SENDONCELIST_LEN)
//    {
//        u_SendOnceList[wOwner-BASEADDR_TRISMITEDONCEVALUE].wFlag = 0;
//    }
//}
//================================================================================

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//void  initCurve()
//{
//  memset(&u_CurveRespList, 0, sizeof(u_CurveRespList));
//}

//BOOL  RegisterCurve(WORD wCurveID, TM_CURVEDETAIL *pCurveDetail)
//{
//  int     nCurveID = wCurveID - 1;
//  CURVERESPLIST   *pCurveRespList;
//
//  if (wCurveID > CURVEMOTIONID_MAX || pCurveDetail == NULL) return FALSE;
//
//  pCurveRespList = u_CurveRespList + nCurveID;
//
//  if (pCurveRespList->wFlag == 0)
//  {
//      pCurveRespList->wFlag = 1;
//      AddDnCounter(&(pCurveRespList->lDownTimer));
//  }
//
//  pCurveRespList->MotionCurve.wCurveMotionID = wCurveID;
//  pCurveRespList->MotionCurve.wInterval = pCurveDetail->wInterval;
//  pCurveRespList->MotionCurve.wDataItems = pCurveDetail->wDataItems;
//  memcpy(pCurveRespList->MotionCurve.awDataIndex, pCurveDetail->awDataIndex, sizeof(pCurveDetail->awDataIndex));
//
//  return TRUE;
//}

//BOOL  UnRegisterCurve(WORD wCurveID)
//{
//  int     nCurveID = wCurveID - 1;
//  CURVERESPLIST   *pCurveRespList;
//
//  if (wCurveID > CURVEMOTIONID_MAX) return FALSE;
//
//  pCurveRespList = u_CurveRespList + nCurveID;
//
//  if (pCurveRespList->wFlag)
//  {
//      pCurveRespList->wFlag = 0;
//      DeleteDnCounter(&(pCurveRespList->lDownTimer));
//  }
//
//  memset(pCurveRespList, 0, sizeof(CURVERESPLIST));
//
//  return TRUE;
//}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//BOOL  Start_TraceCurve(WORD wCurveID)
//{
//  int     nCurveID = wCurveID - 1;
//  CURVERESPLIST   *pCurveRespList;
//
//  if (wCurveID > CURVEMOTIONID_MAX) return FALSE;
//
//  pCurveRespList = u_CurveRespList + nCurveID;
//
//  if (pCurveRespList->wFlag == 0) return FALSE;
//
//  if (pCurveRespList->State.bit.bCollectFlag == 0)
//  {
//      pCurveRespList->State.bit.bCollectFlag = 1;
//      pCurveRespList->MotionCurve.wCurveIndex = 0;
//      pCurveRespList->MotionCurve.wCollectCount = 0;
//      pCurveRespList->lDownTimer = 0;
//      memset(pCurveRespList->MotionCurve.CurvePoints, 0, sizeof(pCurveRespList->MotionCurve.CurvePoints));
//  }
//
//  return TRUE;
//}
//
//BOOL  End_TraceCurve(WORD wCurveID)
//{
//  int     nCurveID = wCurveID - 1;
//  CURVERESPLIST   *pCurveRespList;
//
//  if (wCurveID > CURVEMOTIONID_MAX) return FALSE;
//
//  pCurveRespList = u_CurveRespList + nCurveID;
//
//  if (pCurveRespList->wFlag == 0) return FALSE;
//
//  if (pCurveRespList->State.bit.bCollectFlag)
//  {
//      pCurveRespList->State.bit.bCollectFlag = 0;
//      // not consider the data being sent, directly overwrite sending buffer.
//      if (pCurveRespList->MotionCurve.wCollectCount > 0)
//      {
//          pCurveRespList->State.bit.bSendFlag = 1;
//          pCurveRespList->MotionCurve.wSendCount = pCurveRespList->MotionCurve.wCollectCount;
//          memcpy(pCurveRespList->MotionCurve.SendCurvePoints, \
//                 pCurveRespList->MotionCurve.CurvePoints, \
//                 sizeof(DBVALUE) * pCurveRespList->MotionCurve.wSendCount * pCurveRespList->MotionCurve.wDataItems);
//          pCurveRespList->MotionCurve.wCollectCount = 0;
//      }
//  }
//
//  return TRUE;
//}
//
//BOOL  Collect_TraceCurve(WORD wCurveID)
//{
//  int     i, j, nCurveID = wCurveID - 1;
//  CURVERESPLIST   *pCurveRespList;
//
//  if (wCurveID > CURVEMOTIONID_MAX) return FALSE;
//
//  pCurveRespList = u_CurveRespList + nCurveID;
//
//  if (pCurveRespList->wFlag && pCurveRespList->State.bit.bCollectFlag)
//  {
//      if (pCurveRespList->lDownTimer <= 0)
//      {
//          pCurveRespList->lDownTimer = pCurveRespList->MotionCurve.wInterval;
//          // collect curve points
//          for (i=0; i<pCurveRespList->MotionCurve.wDataItems; i++)
//          {
//              j = pCurveRespList->MotionCurve.wCollectCount * pCurveRespList->MotionCurve.wDataItems;
//              pCurveRespList->MotionCurve.CurvePoints[j+i] = GetDBValueByIndex(pCurveRespList->MotionCurve.awDataIndex[i]);
//          }
//          // check whether to send
//          if ((++pCurveRespList->MotionCurve.wCollectCount) >= MAX_CURVEDATA)
//          {
//              pCurveRespList->State.bit.bSendFlag = 1;
//              pCurveRespList->MotionCurve.wSendCount = MAX_CURVEDATA;
//              memcpy(pCurveRespList->MotionCurve.SendCurvePoints, \
//                     pCurveRespList->MotionCurve.CurvePoints, \
//                     sizeof(DBVALUE) * MAX_CURVEDATA * pCurveRespList->MotionCurve.wDataItems);
//              pCurveRespList->MotionCurve.wCollectCount = 0;
//          }
//      }
//  }
//
//  return TRUE;
//}
//
//BOOL  Send_TraceCurve(int nSessionHandle, WORD wCurveID)
//{
//  WORD    wLength;
//  int     nCurveID = wCurveID - 1;
//  CURVERESPLIST   *pCurveRespList;
//  TM_CURVERESP    *pCurveResp = (TM_CURVERESP *)u_TempBuffer.awData;
//
//  if (wCurveID > CURVEMOTIONID_MAX) return FALSE;
//
//  pCurveRespList = u_CurveRespList + nCurveID;
//
//  if (pCurveRespList->State.bit.bSendFlag)
//  {
//      //if (GetToken_Retransmit(BASEADDR_TRISMITEDAUTOVALUE + wCurveID))
//      {
//          // send
//          memset(u_TempBuffer.awData, 0, MAX_BUFFER_NO);
//
//          wLength = 2*(sizeofW(TMHEADER) + sizeofW(TM_CURVERESP)) + 2;
//          u_TempBuffer.TMHeader.wHeader               =   HEADER_RES;
//          u_TempBuffer.TMHeader.wSize                 =   wLength;
//          u_TempBuffer.TMHeader.wCounter              =   0;
//          u_TempBuffer.TMHeader.wType                 =   RESPONSE_CURVE_ID;
//          u_TempBuffer.TMHeader.datatype.wCounterRet  =   u_wRespCurveRecCounter++;
//          u_TempBuffer.TMHeader.wNoElements           =   1;
//
//          pCurveResp->wCurveMotionID  = pCurveRespList->MotionCurve.wCurveMotionID;
//          pCurveResp->wCurveIndex     = pCurveRespList->MotionCurve.wCurveIndex;
//          pCurveResp->wInterval       = pCurveRespList->MotionCurve.wInterval;
//          pCurveResp->wDataItems      = pCurveRespList->MotionCurve.wDataItems;
//          pCurveResp->wDataCount      = pCurveRespList->MotionCurve.wSendCount;
//          memcpy(pCurveResp->SendCurvePoints, pCurveRespList->MotionCurve.SendCurvePoints, sizeof(pCurveResp->SendCurvePoints));
//
//          *((WORD *)&u_TempBuffer + (wLength>>1) - 1) = FOOTER_RES;
//
//          //SendNetData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
//          SendRetransmitData(nSessionHandle,(WORD *)&u_TempBuffer, wLength);
//
//          // reset
//          pCurveRespList->MotionCurve.wCurveIndex++;
//          pCurveRespList->State.bit.bSendFlag = 0;
//          pCurveRespList->MotionCurve.wSendCount = 0;
//          memset(pCurveRespList->MotionCurve.SendCurvePoints, 0, sizeof(pCurveRespList->MotionCurve.SendCurvePoints));
//      }
//    }
//
//  return TRUE;
//}

/*---------------------------------------------------------------------------+
|  Function :                                                                |
|  Task     :                                                                |
+----------------------------------------------------------------------------+
|  Call     :                                                                |
|                                                                            |
|  Parameter:                           -                                    |
|                                                                            |
|  Return   :                           -                                    |
+---------------------------------------------------------------------------*/
//WORD  MotionIDToCurveID(WORD wMCID)
//{
//  WORD    wCurveID;
//
//  switch (wMCID)
//  {
//      case CMD_OPENMOLD:
//          wCurveID = CURVEMOTIONID_OPENMOLD;
//          break;
//      case CMD_CLOSEMOLD:
//          wCurveID = CURVEMOTIONID_CLOSEMOLD;
//          break;
//      default:
//          wCurveID = CURVEMOTIONID_MAX+1;
//          break;
//  }
//
//  return wCurveID;
//}
//
//
//void  Start_Motion(WORD wMCID)
//{
//  Start_TraceCurve(MotionIDToCurveID(wMCID));
//}
//
//void  End_Motion(WORD wMCID)
//{
//  End_TraceCurve(MotionIDToCurveID(wMCID));
//}

//=================================================================================
// No more.
//=================================================================================
