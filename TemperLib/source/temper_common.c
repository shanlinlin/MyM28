/*==============================================================================+
|  Function : Temper application                                                |
|  Task     : Temper application Source File                                    |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Wain.Wei                                                          |
|  Version  : V1.00                                                             |
|  Creation : 2012/08/10                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "temper_common.h"
//#include "MC_database.h"

/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/
#define TEMPERTABLE_NUMBER 16 // Temper table size

#define TEMPER_BANLANCEHEAT_STATE_IDLE 0
#define TEMPER_BANLANCEHEAT_STATE_RUN 1
#define TEMPER_BANLANCEHEAT_STATE_END 2

#define TEMPER_STATE_ADJIDLE 0
#define TEMPER_STATE_ADJRUN 1
#define TEMPER_STATE_ADJEND 2
/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/
typedef struct tagAD2DEGREE AD2DEGREE_T;
struct tagAD2DEGREE
{
	DWORD_T dwAd;    // A/D value
	LONG_T  lDegree; // temperature value
};
/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/
/*==============================================================================+
|           Globle Variables                                                    |
+==============================================================================*/

/*==============================================================================+
|           Class declaration - Temper algorithm application                    |
+==============================================================================*/
/*------------------------------------------------------------------------------+
|           Helpers Implementation                                              |
+------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------+
|           Inner Globle Variable                                               |
+------------------------------------------------------------------------------*/
AD2DEGREE_T u_aAd2DegreeK[TEMPERTABLE_NUMBER] = // The table must be listed in ASCENDING order K-type
    {
	    { 128, -12 }, // The first element is the possible SMALLEST value
		{ 256, 0 },
		{ 556, 30 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 865, 60 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1281, 100 },
		{ 1791, 150 },
		{ 2291, 200 },
		{ 2794, 250 },
		{ 3829, 350 },
		{ 4884, 450 }, // The last  element is the possible LARGEST  value

	    { 5417, 500 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 5950, 550 },
		{ 6482, 600 },
		{ 7012, 650 },
		{ 7538, 700 },
		{ 7747, 720 }
    };

AD2DEGREE_T u_aAd2DegreeK2[TEMPERTABLE_NUMBER] = // The table must be listed in ASCENDING order K-type
    {
	    { 128, -30 }, // The first element is the possible SMALLEST value
		{ 428, 0 },
		{ 728, 30 },  // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1037, 60 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1453, 100 },
		{ 1963, 150 },
		{ 2463, 200 },
		{ 2966, 250 },
		{ 4001, 350 },
		{ 5056, 450 }, // The last  element is the possible LARGEST  value

		{ 5589, 500 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 6122, 550 },
		{ 6654, 600 },
		{ 7184, 650 },
		{ 7710, 700 },
		{ 7919, 720 }
    };

AD2DEGREE_T u_aAd2DegreeJ[TEMPERTABLE_NUMBER] = // The table must be listed in ASCENDING order J-type
    {
	    { 128, -10 }, // The first element is the possible SMALLEST value
		{ 256, 0 },
		{ 640, 30 },  // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1035, 60 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1573, 100 },
		{ 2258, 150 },
		{ 2951, 200 },
		{ 3643, 250 },
		{ 5028, 350 },
		{ 6406, 450 }, // The last  element is the possible LARGEST  value

		{ 7104, 500 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 7810, 550 },
		{ 7810, 550 },
		{ 7810, 550 },
		{ 7810, 550 },
		{ 7810, 550 }
    };

AD2DEGREE_T u_aAd2DegreeJ2[TEMPERTABLE_NUMBER] = // The table must be listed in ASCENDING order J-type
    {
	    { 128, -24 }, // The first element is the possible SMALLEST value
		{ 428, 0 },
		{ 812, 30 },  // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1207, 60 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1745, 100 },
		{ 2430, 150 },
		{ 3123, 200 },
		{ 3815, 250 },
		{ 5200, 350 },
		{ 6578, 450 }, // The last  element is the possible LARGEST  value

		{ 7276, 500 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 7982, 550 },
		{ 7982, 550 },
		{ 7982, 550 },
		{ 7982, 550 },
		{ 7982, 550 }
    };
AD2DEGREE_T u_aAd2DegreeE[TEMPERTABLE_NUMBER] = // The table must be listed in ASCENDING order J-type
    {
	    { 128, -8 }, // The first element is the possible SMALLEST value
		{ 256, 0 },
		{ 706, 30 },  // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1176, 60 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1791, 100 },
		{ 2703, 150 },
		{ 3611, 200 },
		{ 4551, 250 },
		{ 6496, 350 },
		{ 7491, 400 }, // The last  element is the possible LARGEST  value

		{ 7892, 420 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 7892, 420 },
		{ 7892, 420 },
		{ 7892, 420 },
		{ 7892, 420 },
		{ 7892, 420 }
    };

AD2DEGREE_T u_aAd2DegreeE2[TEMPERTABLE_NUMBER] = // The table must be listed in ASCENDING order J-type
    {
	    { 128, -20 }, // The first element is the possible SMALLEST value
		{ 428, 0 },
		{ 878, 30 },  // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1348, 60 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 1963, 100 },
		{ 2875, 150 },
		{ 3783, 200 },
		{ 4723, 250 },
		{ 6668, 350 },
		{ 7663, 400 }, // The last  element is the possible LARGEST  value

		{ 8064, 420 }, // Anders 2016-2-17, Add. V*250+256 = AD -> Temper.
		{ 8064, 420 },
		{ 8064, 420 },
		{ 8064, 420 },
		{ 8064, 420 },
		{ 8064, 420 }
    };

const AD2DEGREE_T * u_apTemperTypeTbl[] = {
	&u_aAd2DegreeE[0],
	&u_aAd2DegreeJ[0],
	&u_aAd2DegreeK[0]
};
const AD2DEGREE_T * u_apTemperTypeTbl2[] = {
	&u_aAd2DegreeE2[0],
	&u_aAd2DegreeJ2[0],
	&u_aAd2DegreeK2[0]
};
//+++++
//  Temper alg
//+++++
#pragma DATA_SECTION(u_aTemperInternal, "TemperCtl");

TEMPER_COMMON_INTERNAL_T u_TemperCommonInteral;
TEMPER_COMMON_T          u_TemperCommon;
TEMPER_ALG_INTERNAL_T    u_aTemperInternal[TEMPER_MAX_NBR];
TEMPER_ALG_T             u_aTemperAlg[TEMPER_MAX_NBR];
DWORD_T                  u_dwTemperTick[2] = { 0, 0 };
Timer_T                  u_HeatCMDTimer;
static WORD_T            u_awTemperIndex[2] = { 0, 0 };

DWORD g_version_temperlib;
/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
void TemperCommon_Init(WORD_T wTemperNum,
    const TEMPER_ALG_CONF_T * pNormalAlgConf,
    const TEMPER_ALG_CONF_T * pFastAlgConf,

    TEMPER_COMMON_DB_T * pTemperCommonDb,
    TEMPER_ALG_DB_T *    pTemperDb)
{
	WORD_T i;

	g_version_temperlib = VERSION_TEMPERLIB;

	memset(u_aTemperAlg, 0, sizeof(u_aTemperAlg));
	memset(&u_TemperCommon, 0, sizeof(u_TemperCommon));
	memset(&u_TemperCommonInteral, 0, sizeof(u_TemperCommonInteral));
	memset(&u_aTemperInternal, 0, sizeof(u_aTemperInternal));

	u_TemperCommon.pDb            = pTemperCommonDb;
	u_TemperCommon.pInternal      = &u_TemperCommonInteral;
	u_TemperCommon.pNormalAlgConf = pNormalAlgConf; // Anders 2017-4-27
	u_TemperCommon.pFastAlgConf   = pFastAlgConf;

	u_TemperCommon.pInternal->wNumber = wTemperNum;

	for (i = 0; i < wTemperNum; i++)
	{
		TemperAlg_Create(&u_aTemperAlg[i],
			pNormalAlgConf,
			&pTemperDb[i], //pTemperDb,     // 2015-02-11 THJ Modification:
			&u_aTemperInternal[i]);
	}
}
/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/
void TemperCommon_ChangeRapidCompensationUse(WORD_T wIndex, WORD_T wUse)
{
	// int     i;

	// for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
	// {
	//     u_aTemperInternal[i].wRapidCompensationUse     =   u_TemperCommon.pDb->wRapidCompensationUse;
	// }

	u_aTemperInternal[wIndex].wRapidCompensationUse = wUse; // Anders 2020-6-20, modify.
}

void TemperCommon_SetPIDChange(WORD_T wIndex, WORD_T wChange)
{
	u_aTemperInternal[wIndex].wPIDChange		=		wChange;

}

void TemperCommon_Set_MicroAdj(WORD_T wIndex,
    WORD_T                            wUse)
{
	Temper_Alg_Set_MicroAdj(&u_aTemperAlg[wIndex], wUse);
}

void TemperCommon_ChangePipeAdjCmd(void)
{
	WORD_T i;
	//static  WORD_T          wLastPipeCmd        =  TEMPER_ADJ_CMD_STOP ;  // Anders 2015-4-15, mark.
	//if (wLastPipeCmd != u_TemperCommon.pDb->wPipeAutoPidCmd )
	{
		//wLastPipeCmd = u_TemperCommon.pDb->wPipeAutoPidCmd;
		if (u_TemperCommon.pDb->wPipeAutoPidCmd == TEMPER_ADJ_CMD_START_MODE1 || u_TemperCommon.pDb->wPipeAutoPidCmd == TEMPER_ADJ_CMD_START_MODE2)
		{
			u_TemperCommon.pDb->wPipePidState = TEMPER_STATE_ADJRUN;
		}
		else
		{
			u_TemperCommon.pDb->wPipePidState = TEMPER_STATE_ADJIDLE;
		}

		for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
		{
			if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE && (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0) // Anders 2015-3-18, Add for HT.
			)
			{
				u_aTemperInternal[i].fMaxRate = 1;
				Temper_Alg_Set_Adj(&u_aTemperAlg[i], u_TemperCommon.pDb->wPipeAutoPidCmd);
			}
		}
	}
}

void TemperCommon_ChangeMoldAdjCmd(void)
{
	WORD_T i;
	//static  WORD_T          wLastMoldCmd        =  TEMPER_ADJ_CMD_STOP ;    // Anders 2015-4-15, mark.
	//if (wLastMoldCmd != u_TemperCommon.pDb->wMoldAutoPidCmd )
	{
		//wLastMoldCmd = u_TemperCommon.pDb->wMoldAutoPidCmd;
		if (u_TemperCommon.pDb->wMoldAutoPidCmd == TEMPER_ADJ_CMD_START_MODE1 || u_TemperCommon.pDb->wMoldAutoPidCmd == TEMPER_ADJ_CMD_START_MODE2)
		{
			u_TemperCommon.pDb->wMoldPidState = TEMPER_STATE_ADJRUN;
		}
		else
		{
			u_TemperCommon.pDb->wMoldPidState = TEMPER_STATE_ADJIDLE;
		}

		for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
		{
			if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_MOLD && (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0) // Anders 2015-3-18, Add for HT.
			)
			{
				u_aTemperInternal[i].fMaxRate = 1;
				Temper_Alg_Set_Adj(&u_aTemperAlg[i], u_TemperCommon.pDb->wMoldAutoPidCmd);
			}
		}
	}
}

void TemperCommon_ChangeHeatCmd(void)
{
	WORD_T i;
	// static  WORD_T      wLastHeatCmd    =   TEMPER_HEAT_OFF;
	static HeatCmd_T uLastHeatCmd = { 0 };

	//  if ( wLastHeatCmd != u_TemperCommon.pDb->wHeatCmd )
	//  {
	//      wLastHeatCmd    =   u_TemperCommon.pDb->wHeatCmd;
	//      u_TemperCommon.pInternal->bPipeInSoftHeat               =   TRUE;
	//      u_TemperCommon.pInternal->dwPipeSoftStartTime           =   0;
	//      memset ( u_TemperCommon.pInternal->adwPipeSoftHeatBit, 0, sizeof(u_TemperCommon.pInternal->adwPipeSoftHeatBit) );

	//      u_TemperCommon.pInternal->bMoldInSoftHeat               =   TRUE;
	//      u_TemperCommon.pInternal->dwMoldSoftStartTime           =   0;
	//      memset ( u_TemperCommon.pInternal->adwMoldSoftHeatBit, 0, sizeof(u_TemperCommon.pInternal->adwMoldSoftHeatBit) );

	//      u_TemperCommon.pInternal->lPipeBanlanceAverTemper       =   0;
	//      u_TemperCommon.pInternal->lPipeLastBanlanceAverTemper   =   0;
	//      u_TemperCommon.pInternal->wPipeBanlanceHeatState        =   TEMPER_BANLANCEHEAT_STATE_IDLE;

	//      u_TemperCommon.pInternal->lMoldLastBanlanceAverTemper   =   0;
	//      u_TemperCommon.pInternal->lMoldBanlanceAverTemper       =   0;
	//      u_TemperCommon.pInternal->wMoldBanlanceHeatState        =   TEMPER_BANLANCEHEAT_STATE_IDLE;

	//      for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
	//      {
	//          u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk     =   FALSE;
	//      }

	//      if (u_TemperCommon.pDb->wHeatCmd != TEMPER_HEAT_ON)
	//      {
	//          for (i = 0; i < u_TemperCommon.pInternal->wNumber; ++i)
	//  u_aTemperAlg[i].pTemperDb->wStatus                  &=  ~TEMPER_ADJUST_MASK;
	// memset( u_TemperCommon.pDb->adwHeatOutput, 0, sizeof(u_TemperCommon.pDb->adwHeatOutput) );
	// memset( u_TemperCommon.pDb->adwCoolOutput, 0, sizeof(u_TemperCommon.pDb->adwCoolOutput) );
	//      }

	//      TemperCommon_ChangePipeAdjCmd();    // Anders 2015-4-15, Add.
	//      TemperCommon_ChangeMoldAdjCmd();    // Anders 2015-4-15, Add.
	//  }
	if (uLastHeatCmd.dwAll != u_TemperCommon.pDb->uHeatCmd.dwAll)
	{
		u_TemperCommon.pInternal->bChangeHeatCMD = TRUE;
		u_TemperCommon.pInternal->wHeatCMDIndex  = 0;
		// Pipe HeatCmd
		if (uLastHeatCmd.sBit.bPipeEnable != u_TemperCommon.pDb->uHeatCmd.sBit.bPipeEnable)
		{
			if (u_TemperCommon.pDb->wPipeSoftHeatTemper != 0) // Anders 2016-10-12, add.
				u_TemperCommon.pInternal->bPipeInSoftHeat = TRUE;
			u_TemperCommon.pInternal->dwPipeSoftStartTime = 0;
			memset(u_TemperCommon.pInternal->adwPipeSoftHeatBit, 0, sizeof(u_TemperCommon.pInternal->adwPipeSoftHeatBit));

			u_TemperCommon.pInternal->lPipeBanlanceAverTemper     = 0;
			u_TemperCommon.pInternal->lPipeLastBanlanceAverTemper = 0;
			u_TemperCommon.pInternal->wPipeBanlanceHeatState      = TEMPER_BANLANCEHEAT_STATE_IDLE;

			for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
			{
				if (TEMPER_DEV_PIPE == u_aTemperAlg[i].pTemperDb->wDeviceType)
				{
					u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk = FALSE;
					if (u_TemperCommon.pDb->uHeatCmd.sBit.bPipeEnable == TEMPER_HEAT_OFF)
					{
						u_aTemperAlg[i].pTemperDb->wStatus &= ~TEMPER_ADJUST_MASK;
						u_TemperCommon.pDb->adwHeatOutput[i >> 5] &= ~((DWORD_T) 1 << (i & 0x1F));
						u_TemperCommon.pDb->adwCoolOutput[i >> 5] &= ~((DWORD_T) 1 << (i & 0x1F));
					}
				}
			}
			TemperCommon_ChangePipeAdjCmd(); // Anders 2015-4-15, Add.
		}

		// Mold HeatCmd
		if (uLastHeatCmd.sBit.bMoldEnable != u_TemperCommon.pDb->uHeatCmd.sBit.bMoldEnable)
		{
			if (u_TemperCommon.pDb->wMoldSoftHeatTemper != 0) // Anders 2016-10-12, add.
				u_TemperCommon.pInternal->bMoldInSoftHeat = TRUE;
			u_TemperCommon.pInternal->dwMoldSoftStartTime = 0;
			memset(u_TemperCommon.pInternal->adwMoldSoftHeatBit, 0, sizeof(u_TemperCommon.pInternal->adwMoldSoftHeatBit));

			u_TemperCommon.pInternal->lMoldBanlanceAverTemper     = 0;
			u_TemperCommon.pInternal->lMoldLastBanlanceAverTemper = 0;
			u_TemperCommon.pInternal->wMoldBanlanceHeatState      = TEMPER_BANLANCEHEAT_STATE_IDLE;

			for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
			{
				if (TEMPER_DEV_MOLD == u_aTemperAlg[i].pTemperDb->wDeviceType)
				{
					u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk = FALSE;
					if (u_TemperCommon.pDb->uHeatCmd.sBit.bMoldEnable == TEMPER_HEAT_OFF)
					{
						u_aTemperAlg[i].pTemperDb->wStatus &= ~TEMPER_ADJUST_MASK;
						u_TemperCommon.pDb->adwHeatOutput[i >> 5] &= ~((DWORD_T) 1 << (i & 0x1F));
						u_TemperCommon.pDb->adwCoolOutput[i >> 5] &= ~((DWORD_T) 1 << (i & 0x1F));
					}
				}
			}
			TemperCommon_ChangeMoldAdjCmd(); // Anders 2015-4-15, Add.
		}
		uLastHeatCmd.dwAll = u_TemperCommon.pDb->uHeatCmd.dwAll;
	}
}

void TemperCommon_ChangeSoftHeat(void)
{
	WORD_T        i;
	static WORD_T wLastPipeSoftHeatTemper = 0;
	static WORD_T wLastMoldSoftHeatTemper = 0;

	if (u_TemperCommon.pDb->wPipeSoftHeatTemper != wLastPipeSoftHeatTemper)
	{
		wLastPipeSoftHeatTemper = u_TemperCommon.pDb->wPipeSoftHeatTemper;
		if (u_TemperCommon.pDb->wPipeSoftHeatTemper != 0)
		{
			u_TemperCommon.pInternal->bPipeInSoftHeat     = TRUE;
			u_TemperCommon.pInternal->dwPipeSoftStartTime = 0;
			memset(u_TemperCommon.pInternal->adwPipeSoftHeatBit, 0, sizeof(u_TemperCommon.pInternal->adwPipeSoftHeatBit));
		}
		else if (u_TemperCommon.pInternal->bPipeInSoftHeat)
		{
			u_TemperCommon.pInternal->bPipeInSoftHeat = FALSE;

			for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
			{
				if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE && (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0) // Anders 2015-3-18, Add for HT.
				)
				{
					u_aTemperAlg[i].pTemperInternal->lObjTemp = u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
				}
			}
		}
	}

	if (u_TemperCommon.pDb->wMoldSoftHeatTemper != wLastMoldSoftHeatTemper)
	{
		wLastMoldSoftHeatTemper = u_TemperCommon.pDb->wMoldSoftHeatTemper;
		if (u_TemperCommon.pDb->wMoldSoftHeatTemper != 0)
		{
			u_TemperCommon.pInternal->bMoldInSoftHeat     = TRUE;
			u_TemperCommon.pInternal->dwMoldSoftStartTime = 0;
			memset(u_TemperCommon.pInternal->adwMoldSoftHeatBit, 0, sizeof(u_TemperCommon.pInternal->adwMoldSoftHeatBit));
		}
		else if (u_TemperCommon.pInternal->bMoldInSoftHeat)
		{
			u_TemperCommon.pInternal->bMoldInSoftHeat = FALSE;

			for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
			{
				if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_MOLD && (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0) // Anders 2015-3-18, Add for HT.
				)
				{
					u_aTemperAlg[i].pTemperInternal->lObjTemp = u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
				}
			}
		}
	}
}

void TemperCommon_ChangeAbsolute_set(WORD_T wIndex)
{
	LONG_T lTemp;

	if ((u_aTemperAlg[wIndex].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet <= 0) || u_aTemperAlg[wIndex].pTemperDb->wUse != TEMPER_HEAT_USE_ABSTRACT)
	{
		return;
	} // Anders 2015-3-18, Add for HT.
	u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk = FALSE;

	if (u_TemperCommon.pInternal->wPipeBanlanceHeatState == TEMPER_BANLANCEHEAT_STATE_END)
	{
		u_TemperCommon.pInternal->wPipeBanlanceHeatState = TEMPER_BANLANCEHEAT_STATE_RUN;
	}

	if (u_aTemperAlg[wIndex].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT)
	{
		if (u_TemperCommon.pInternal->bPipeInSoftHeat)
		{
			if (TEMPER_DEV_PIPE == u_aTemperAlg[wIndex].pTemperDb->wDeviceType)
			{
				lTemp = ((0 != u_TemperCommon.pDb->wPipePreserveUse) && (0 != u_TemperCommon.pDb->wPipePreserveTemper))
				    ? u_TemperCommon.pDb->wPipePreserveTemper
				    : u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
				if (lTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper)
				{
					lTemp = u_TemperCommon.pDb->wPipeSoftHeatTemper;
				}
				if (u_aTemperAlg[wIndex].pTemperDb->lCurrent < lTemp)
				{
					u_TemperCommon.pInternal->adwPipeSoftHeatBit[wIndex >> 5] &= ~(1UL << (wIndex & 0x1F));
					u_TemperCommon.pInternal->dwPipeSoftStartTime = 0;
				}
			}
			// if (u_TemperCommon.pDb->wPreserveUse != 0 &&
			//     u_TemperCommon.pDb->wPreserveTemper != 0 )
			// {
			//     lTemp   =   u_TemperCommon.pDb->wPreserveTemper;
			// }
			// else
			// {
			//  lTemp   =   u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
			// }

			// if ( (u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE )
			//     lTemp   =   (lTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper ) ?
			//                 u_TemperCommon.pDb->wPipeSoftHeatTemper :
			//                 lTemp;
			// else
			//     lTemp   =   (lTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper ) ?
			//                 u_TemperCommon.pDb->wMoldSoftHeatTemper :
			//                 lTemp;

			// if ( u_aTemperAlg[wIndex].pTemperDb->lCurrent < lTemp )
			// {
			//     u_TemperCommon.pInternal->adwPipeSoftHeatBit[wIndex>>5] &=  ~(1UL<<(wIndex&0x1F));
			//     u_TemperCommon.pInternal->dwPipeSoftStartTime           =   0;
			// }
		}
		if (u_TemperCommon.pInternal->bMoldInSoftHeat)
		{
			if (TEMPER_DEV_MOLD == u_aTemperAlg[wIndex].pTemperDb->wDeviceType)
			{
				lTemp = ((0 != u_TemperCommon.pDb->wMoldPreserveUse) && (0 != u_TemperCommon.pDb->wMoldPreserveTemper))
				    ? u_TemperCommon.pDb->wMoldPreserveTemper
				    : u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
				if (lTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper)
				{
					lTemp = u_TemperCommon.pDb->wMoldSoftHeatTemper;
				}
				if (u_aTemperAlg[wIndex].pTemperDb->lCurrent < lTemp)
				{
					u_TemperCommon.pInternal->adwMoldSoftHeatBit[wIndex >> 5] &= ~(1UL << (wIndex & 0x1F));
					u_TemperCommon.pInternal->dwMoldSoftStartTime = 0;
				}
			}
		}
	}
}

void TemperCommon_ChangePreserve(void)
{
	WORD_T i;
	LONG_T lTemp;
	for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
	{
		if (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0 )// Anders 2015-3-18, Add for HT.
		{
			if (u_TemperCommon.pInternal->bPipeInSoftHeat)
			{
				if (TEMPER_DEV_PIPE == u_aTemperAlg[i].pTemperDb->wDeviceType)
				{
					lTemp = ((0 != u_TemperCommon.pDb->wPipePreserveUse) && (0 != u_TemperCommon.pDb->wPipePreserveTemper))
					    ? u_TemperCommon.pDb->wPipePreserveTemper
					    : u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
					if (lTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper)
					{
						lTemp = u_TemperCommon.pDb->wPipeSoftHeatTemper;
					}
					if (u_aTemperAlg[i].pTemperDb->lCurrent < lTemp)
					{
						u_TemperCommon.pInternal->adwPipeSoftHeatBit[i >> 5] &= ~(1UL << (i & 0x1F));
						u_TemperCommon.pInternal->dwPipeSoftStartTime = 0;
					}
				}
				// if (u_TemperCommon.pDb->wPreserveUse != 0 &&
				//  u_TemperCommon.pDb->wPreserveTemper != 0 )
				// {
				//  lTemp   =   u_TemperCommon.pDb->wPreserveTemper;
				// }
				// else
				// {
				//  lTemp   =   u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
				// }

				// if ( (u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE )
				//  lTemp   =   (lTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper ) ?
				//              u_TemperCommon.pDb->wPipeSoftHeatTemper :
				//              lTemp;
				// else
				//  lTemp   =   (lTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper ) ?
				//              u_TemperCommon.pDb->wMoldSoftHeatTemper :
				//              lTemp;

				// if ( u_aTemperAlg[i].pTemperDb->lCurrent < lTemp )
				// {
				//     u_TemperCommon.pInternal->adwPipeSoftHeatBit[i>>5]      &=  ~(1UL<<(i&0x1F));
				//     u_TemperCommon.pInternal->dwPipeSoftStartTime           =   0;
				// }
			}
			if (u_TemperCommon.pInternal->bMoldInSoftHeat)
			{
				if (TEMPER_DEV_MOLD == u_aTemperAlg[i].pTemperDb->wDeviceType)
				{
					lTemp = ((0 != u_TemperCommon.pDb->wMoldPreserveUse) && (0 != u_TemperCommon.pDb->wMoldPreserveTemper))
					    ? u_TemperCommon.pDb->wMoldPreserveTemper
					    : u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
					if (lTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper)
					{
						lTemp = u_TemperCommon.pDb->wMoldSoftHeatTemper;
					}
					if (u_aTemperAlg[i].pTemperDb->lCurrent < lTemp)
					{
						u_TemperCommon.pInternal->adwMoldSoftHeatBit[i >> 5] &= ~(1UL << (i & 0x1F));
						u_TemperCommon.pInternal->dwMoldSoftStartTime = 0;
					}
				}
			}
		}
	}
}

void TemperCommon_ChangeUse(WORD_T wIndex)
{
	LONG_T lTemp;

	if (u_aTemperAlg[wIndex].pTemperDb->wUse != TEMPER_HEAT_USE_ABSTRACT || u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet <= 0) // Anders 2015-3-18, Add for HT.
	{
		return;
	}

	if (u_TemperCommon.pInternal->bPipeInSoftHeat)
	{
		if (TEMPER_DEV_PIPE == u_aTemperAlg[wIndex].pTemperDb->wDeviceType)
		{
			lTemp = ((0 != u_TemperCommon.pDb->wPipePreserveUse) && (0 != u_TemperCommon.pDb->wPipePreserveTemper))
			    ? u_TemperCommon.pDb->wPipePreserveTemper
			    : u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
			if (lTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper)
			{
				lTemp = u_TemperCommon.pDb->wPipeSoftHeatTemper;
			}
			if (u_aTemperAlg[wIndex].pTemperDb->lCurrent < lTemp)
			{
				u_TemperCommon.pInternal->adwPipeSoftHeatBit[wIndex >> 5] &= ~(1UL << (wIndex & 0x1F));
				u_TemperCommon.pInternal->dwPipeSoftStartTime = 0;
			}
		}
		// if (u_TemperCommon.pDb->wPreserveUse != 0 &&
		//         u_TemperCommon.pDb->wPreserveTemper != 0 )
		//     {
		//         lTemp   =   u_TemperCommon.pDb->wPreserveTemper;
		//     }
		//  else
		//  {
		//      lTemp   =   u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
		//  }

		//     if ( (u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE )
		//         lTemp   =   (lTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper ) ?
		//                     u_TemperCommon.pDb->wPipeSoftHeatTemper :
		//                     lTemp;
		//     else
		//         lTemp   =   (lTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper ) ?
		//                     u_TemperCommon.pDb->wMoldSoftHeatTemper :
		//                     lTemp;

		// if ( u_aTemperAlg[wIndex].pTemperDb->lCurrent < lTemp )
		// {
		//     u_TemperCommon.pInternal->adwPipeSoftHeatBit[wIndex>>5] &=  ~(1UL<<(wIndex&0x1F));
		//     u_TemperCommon.pInternal->dwPipeSoftStartTime           =   0;
		// }
	}
	if (u_TemperCommon.pInternal->bMoldInSoftHeat)
	{
		if (TEMPER_DEV_MOLD == u_aTemperAlg[wIndex].pTemperDb->wDeviceType)
		{
			lTemp = ((0 != u_TemperCommon.pDb->wMoldPreserveUse) && (0 != u_TemperCommon.pDb->wMoldPreserveTemper))
			    ? u_TemperCommon.pDb->wMoldPreserveTemper
			    : u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
			if (lTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper)
			{
				lTemp = u_TemperCommon.pDb->wMoldSoftHeatTemper;
			}
			if (u_aTemperAlg[wIndex].pTemperDb->lCurrent < lTemp)
			{
				u_TemperCommon.pInternal->adwMoldSoftHeatBit[wIndex >> 5] &= ~(1UL << (wIndex & 0x1F));
				u_TemperCommon.pInternal->dwMoldSoftStartTime = 0;
			}
		}
	}
}

void TempCommon_DistributeHeatCMD()
{
	WORD wHeatCMDIndex;
	if (u_TemperCommon.pInternal->bChangeHeatCMD == TRUE)
	{
		wHeatCMDIndex = u_TemperCommon.pInternal->wHeatCMDIndex;

		if (wHeatCMDIndex == 0)
			Tmp_ResetTimer(&u_HeatCMDTimer, 0);
		// if (u_aTemperAlg[wHeatCMDIndex].pTemperDb->wUse > TEMPER_HEAT_USE_NO)
		{
			if ((u_aTemperAlg[wHeatCMDIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE) // && (u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable != u_TemperCommon.pDb->uHeatCmd.sBit.bPipeEnable))
			{
				if (u_TemperCommon.pDb->uHeatCmd.sBit.bPipeEnable)
				{
					if (Tmp_CheckTimerOut(&u_HeatCMDTimer))
					{
						if (!u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable)
							u_aTemperAlg[wHeatCMDIndex].pTemperInternal->bHeatCmdOnTriggle = TRUE;
						u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable = u_TemperCommon.pDb->uHeatCmd.sBit.bPipeEnable;
						Tmp_ResetTimer(&u_HeatCMDTimer, u_TemperCommon.pDb->wHeatCMDInterval);
						u_TemperCommon.pInternal->wHeatCMDIndex++;
					}
				}
				else
				{
					u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable          = FALSE;
					u_aTemperAlg[wHeatCMDIndex].pTemperInternal->bHeatCmdOnTriggle = FALSE;
					u_TemperCommon.pInternal->wHeatCMDIndex++;
				}
			}
			else if ((u_aTemperAlg[wHeatCMDIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_MOLD) // && (u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable != u_TemperCommon.pDb->uHeatCmd.sBit.bMoldEnable))
			{
				if (u_TemperCommon.pDb->uHeatCmd.sBit.bMoldEnable)
				{
					if (Tmp_CheckTimerOut(&u_HeatCMDTimer))
					{
						if (!u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable)
							u_aTemperAlg[wHeatCMDIndex].pTemperInternal->bHeatCmdOnTriggle = TRUE;
						u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable = u_TemperCommon.pDb->uHeatCmd.sBit.bMoldEnable;
						Tmp_ResetTimer(&u_HeatCMDTimer, u_TemperCommon.pDb->wHeatCMDInterval);
						u_TemperCommon.pInternal->wHeatCMDIndex++;
					}
				}
				else
				{
					u_aTemperAlg[wHeatCMDIndex].pTemperDb->wHeatCMDEnable          = FALSE;
					u_aTemperAlg[wHeatCMDIndex].pTemperInternal->bHeatCmdOnTriggle = FALSE;
					u_TemperCommon.pInternal->wHeatCMDIndex++;
				}
			}
			else
				u_TemperCommon.pInternal->wHeatCMDIndex++;
		}
		// else
		//     u_TemperCommon.pInternal->wHeatCMDIndex++;

		if (u_TemperCommon.pInternal->wHeatCMDIndex == u_TemperCommon.pInternal->wNumber)
			u_TemperCommon.pInternal->bChangeHeatCMD = FALSE;
	}
}

void TemperCommon_RunCal(WORD_T wDeviceType, WORD_T wTaskCycle)
{
	WORD_T  wIndex = u_awTemperIndex[wDeviceType];
	int     i, iMinSeg;
	LONG_T  lMinTemper = 8880, lMaxTemper = 0;
	WORD_T  wBanlanceHeatNum = 0;
	LONG_T  lObjTemp;
	DWORD_T adwPipeSoftHeatBit[4] = { 0, 0, 0, 0 };
	DWORD_T adwMoldSoftHeatBit[4] = { 0, 0, 0, 0 };

	//  static DWORD dwLastTempMoniTimer = 0;

	//  if (wDeviceType == TEMPER_DEV_PIPE)
	//  {
	//    MC_WRITEDB_DWORD(AXIS1_INTERFACE_ERROR9, g_dwSystemUpTick - dwLastTempMoniTimer);
	//    dwLastTempMoniTimer = g_dwSystemUpTick;
	//  }

	u_dwTemperTick[wDeviceType] += wTaskCycle; //  u_aTemperAlg[0].pTemperConf->wTaskCycle; // Anders 2017-4-27, modify.

	if (wIndex < u_TemperCommon.pInternal->wNumber && u_aTemperAlg[wIndex].pTemperDb->wDeviceType == wDeviceType) // test
	{
		switch (u_aTemperAlg[wIndex].pTemperDb->wDeviceType)
		{
			case TEMPER_DEV_PIPE:
				if ((0 != u_TemperCommon.pDb->wPipePreserveUse) && (TEMPER_HEAT_USE_ABSTRACT == u_aTemperAlg[wIndex].pTemperDb->wUse) && (u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet > 0))
				{
					u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = (0 != u_TemperCommon.pDb->wPipePreserveTemper)//保温功能
					    ? u_TemperCommon.pDb->wPipePreserveTemper
					    : (u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet / 2);
				}
				else
				{
					u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
				}
				break;
			case TEMPER_DEV_MOLD:
				if ((0 != u_TemperCommon.pDb->wMoldPreserveUse) && (TEMPER_HEAT_USE_ABSTRACT == u_aTemperAlg[wIndex].pTemperDb->wUse) && (u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet > 0))
				{
					u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = (0 != u_TemperCommon.pDb->wMoldPreserveTemper)//保温功能
					    ? u_TemperCommon.pDb->wMoldPreserveTemper
					    : (u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet / 2);
				}
				else
				{
					u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
				}
				break;
			default:
				break;
		}
		// if (0 != u_TemperCommon.pDb->wPreserveUse
		//     && (u_aTemperAlg[wIndex].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet > 0) // Anders 2015-3-18, Add for HT.
		//     )
		// {
		//     u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = (0 != u_TemperCommon.pDb->wPreserveTemper)
		//         ? u_TemperCommon.pDb->wPreserveTemper
		//         : (u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet / 2);
		// }
		// else
		//     u_aTemperAlg[wIndex].pTemperInternal->lObjTemp =  u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet;
		//#endif

		//+++++
		// Calculate heat number
		//+++++
		for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
		{
			if (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && (u_aTemperAlg[wIndex].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0)) // Anders 2015-3-18 Add for HT.
			{

				if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE)
				{
					adwPipeSoftHeatBit[i >> 5] |= (DWORD_T) 1 << (i & 0x1F);
				}
				else
				{
					adwMoldSoftHeatBit[i >> 5] |= (DWORD_T) 1 << (i & 0x1F);
				}
			}
		}

		//+++++
		//  Auto adjust
		//+++++
		if (u_TemperCommon.pDb->wPipePidState == TEMPER_STATE_ADJRUN)
		{
			for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
			{
				if ((u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0 // Anders 2015-3-18 Add for HT.
				    && (u_aTemperAlg[i].pTemperDb->wStatus & TEMPER_ADJUST_MASK) != TMEPER_ADJUST_END && (u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE)  ||
				    (g_dwSystemUpTick - u_aTemperAlg[i].pTemperInternal->AdjStartTick < 1800000UL && u_TemperCommon.pDb->dwPipeMicroAdjUse != 0 ) )
				{
					break;
				}
			}

			if (i == u_TemperCommon.pInternal->wNumber)//全部段自学习完成
			{
				for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
				{
					if (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0 // Anders 2015-3-18 Add for HT.
					    && (u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE)
					{
						//实际PID赋值给设定PID显示
						u_aTemperAlg[i].pTemperDb->wKp   = u_aTemperAlg[i].pTemperDb->wAdjKp;
						u_aTemperAlg[i].pTemperDb->wKi   = u_aTemperAlg[i].pTemperDb->wAdjKi;
						u_aTemperAlg[i].pTemperDb->wKd   = u_aTemperAlg[i].pTemperDb->wAdjKd;
						u_aTemperAlg[i].pTemperDb->wRamp = u_aTemperAlg[i].pTemperDb->wAdjRamp;
						//u_aTemperAlg[i].pTemperDb->lStableDuty = u_aTemperAlg[i].pTemperDb->lAdjStableDuty;
					}
				}
				u_TemperCommon.pDb->wPipePidState = TEMPER_STATE_ADJEND;
			}
		}

		if (u_TemperCommon.pDb->wMoldPidState == TEMPER_STATE_ADJRUN)
		{
			for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
			{

				if ((u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0 // Anders 2015-3-18 Add for HT.
				    && (u_aTemperAlg[i].pTemperDb->wStatus & TEMPER_ADJUST_MASK) != TMEPER_ADJUST_END && (u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_MOLD  )  ||
				    ( g_dwSystemUpTick - u_aTemperAlg[i].pTemperInternal->AdjStartTick < 1800000UL && u_TemperCommon.pDb->dwMoldMicroAdjUse != 0 ) )
				{
					break;
				}
			}

			if (i == u_TemperCommon.pInternal->wNumber)
			{
				u_TemperCommon.pDb->wMoldPidState = TEMPER_STATE_ADJEND;
			}
		}
		//+++++
		//  Check temper error
		//+++++
		if (u_aTemperAlg[wIndex].pTemperInternal->dwContinuousHeatTime >= 180000) // time over 3 minute
		{
			if ((u_aTemperAlg[wIndex].pTemperDb->lCurrent < u_aTemperAlg[wIndex].pTemperDb->wHeatCheckDegree + u_aTemperAlg[wIndex].pTemperInternal->lHeatCheckStartDegree) && (u_aTemperAlg[wIndex].pTemperDb->wHeatCheckDegree != 0))
			{
				u_aTemperAlg[wIndex].pTemperDb->wStatus |= TEMPER_CONTINUOUS_ERR;
				u_aTemperAlg[wIndex].pTemperDb->dwError |= TEMPER_ERR_HEATCHECK;
			}
			else
			{
				u_aTemperAlg[wIndex].pTemperDb->wStatus &= ~TEMPER_CONTINUOUS_ERR;
				u_aTemperAlg[wIndex].pTemperDb->dwError &= ~TEMPER_ERR_HEATCHECK;
			}

			u_aTemperAlg[wIndex].pTemperInternal->dwContinuousHeatTime  = 0;
			u_aTemperAlg[wIndex].pTemperInternal->lHeatCheckStartDegree = u_aTemperAlg[wIndex].pTemperDb->lCurrent;
		}

		if (u_aTemperAlg[wIndex].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet > 0 &&                 // Anders 2015-3-18 Add for HT.
		    (u_aTemperAlg[wIndex].pTemperDb->wStatus & TEMPER_ADJUST_MASK) == TEMPER_ADJUST_IDLE && u_aTemperAlg[wIndex].pTemperDb->wHeatCMDEnable) // CX 20190506 modify u_TemperCommon.pDb->uHeatCmd.dwAll != 0)
		{
			//+++++
			// Soft Heating Start Check
			//+++++

			if ((u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE && u_TemperCommon.pInternal->bPipeInSoftHeat)
			{
				if (u_aTemperAlg[wIndex].pTemperDb->wUse == 1)
				{
					if (u_aTemperAlg[wIndex].pTemperInternal->lObjTemp > u_TemperCommon.pDb->wPipeSoftHeatTemper)
					{
						u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = u_TemperCommon.pDb->wPipeSoftHeatTemper;
					}

					if (u_aTemperAlg[wIndex].pTemperDb->lCurrent >= u_aTemperAlg[wIndex].pTemperInternal->lObjTemp) // THJ 20150223 Modification: '>' -> '>='
					{
						u_TemperCommon.pInternal->adwPipeSoftHeatBit[(wIndex) >> 5] |= (DWORD_T) 1 << (wIndex & 0x1F);
					}
				}

				for (i = 0; i < 4; i++)
				{
					if (u_TemperCommon.pInternal->adwPipeSoftHeatBit[i] != adwPipeSoftHeatBit[i])
					{
						break;
					}
				}
				if (i == 4)
				{
					if (u_TemperCommon.pInternal->dwPipeSoftStartTime == 0)
					{
						u_TemperCommon.pInternal->dwPipeSoftStartTime = u_dwTemperTick[wDeviceType];
					}

					if (u_dwTemperTick[wDeviceType] > u_TemperCommon.pInternal->dwPipeSoftStartTime + u_TemperCommon.pDb->wPipeSoftHeatTime * 60000UL)
					{
						//for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
						//{
						//    if ( (u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE )
						//    {
						//        u_aTemperAlg[i].pTemperInternal->lObjTemp =   u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
						//    }
						//}
						u_TemperCommon.pInternal->bPipeInSoftHeat = FALSE;
					}
				}
			}
			else if ((u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_MOLD && u_TemperCommon.pInternal->bMoldInSoftHeat)
			{
				if (u_aTemperAlg[wIndex].pTemperDb->wUse == 1)
				{
					if (u_aTemperAlg[wIndex].pTemperInternal->lObjTemp > u_TemperCommon.pDb->wMoldSoftHeatTemper)
					{
						u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = u_TemperCommon.pDb->wMoldSoftHeatTemper;
					}

					if (u_aTemperAlg[wIndex].pTemperDb->lCurrent > u_aTemperAlg[wIndex].pTemperInternal->lObjTemp)
					{
						u_TemperCommon.pInternal->adwMoldSoftHeatBit[(wIndex) >> 5] |= (DWORD_T) 1 << (wIndex & 0x1F);
					}
				}

				for (i = 0; i < 4; i++)
				{
					if (u_TemperCommon.pInternal->adwMoldSoftHeatBit[i] != adwMoldSoftHeatBit[i])
					{
						break;
					}
				}
				if (i == 4)
				{
					if (u_TemperCommon.pInternal->dwMoldSoftStartTime == 0)
					{
						u_TemperCommon.pInternal->dwMoldSoftStartTime = u_dwTemperTick[wDeviceType];
					}

					if (u_dwTemperTick[wDeviceType] >= u_TemperCommon.pInternal->dwMoldSoftStartTime + u_TemperCommon.pDb->wMoldSoftHeatTime * 60000UL)
					{
						//for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
						//{
						//    if ( (u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE )
						//    {
						//        u_aTemperAlg[i].pTemperInternal->lObjTemp =   u_aTemperAlg[i].pTemperDb->lAbsoluteSet;
						//    }
						//}
						u_TemperCommon.pInternal->bMoldInSoftHeat = FALSE;
					}
				}
			}
			//+++++
			//  Balance heat
			//+++++
			if (u_aTemperAlg[wIndex].pTemperDb->wBanlanceHeatUse)
			{
				if ((u_aTemperAlg[wIndex].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE)
				{

					if (u_TemperCommon.pDb->wPipeBalanceHeatingCmd != 0 && u_TemperCommon.pDb->wPipeBalanceHeatingTemper >= 5 * u_aTemperAlg[wIndex].pTemperConf->wPrecision)
					{
						switch (u_TemperCommon.pInternal->wPipeBanlanceHeatState)
						{
							case TEMPER_BANLANCEHEAT_STATE_IDLE:
								u_TemperCommon.pInternal->wPipeBanlanceHeatState = TEMPER_BANLANCEHEAT_STATE_RUN;
								break;
							case TEMPER_BANLANCEHEAT_STATE_RUN:

								if ((u_TemperCommon.pDb->wPipePreserveUse == 0 || u_TemperCommon.pDb->wPipePreserveTemper == 0) && !u_TemperCommon.pInternal->bPipeInSoftHeat && !u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk && u_aTemperAlg[wIndex].pTemperDb->lCurrent >= u_aTemperAlg[wIndex].pTemperInternal->lObjTemp - u_aTemperAlg[wIndex].pTemperDb->sDownLimit) // THJ 20150223 Modification: '>' -> '>='
								{
									u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk = TRUE;
								}

								for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
								{
									if (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0 // Anders 2015-3-18 Add for HT.
									    && u_aTemperAlg[i].pTemperDb->wBanlanceHeatUse                                                             // Anders 2017-8-3 add.
									    && !u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk)
									{
										if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_PIPE)
										{
											if (lMinTemper > u_aTemperAlg[i].pTemperDb->lCurrent)
											{
												lMinTemper = u_aTemperAlg[i].pTemperDb->lCurrent;
												iMinSeg    = i; // Anders 2018-3.15, Add.
											}

											if (lMaxTemper < u_aTemperAlg[i].pTemperDb->lCurrent && u_aTemperAlg[i].pTemperDb->lCurrent < 8880)
											{
												lMaxTemper = u_aTemperAlg[i].pTemperDb->lCurrent;
											}
										}

										if (u_aTemperAlg[i].pTemperDb->wBanlanceHeatUse != 0 && u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk == FALSE)
										{
											wBanlanceHeatNum++;
										}
									}
								}
								u_TemperCommon.pInternal->lPipeBanlanceAverTemper = lMinTemper; //(lMinTemper + lMaxTemper) >> 1; // Anders 2018-3-15, Modify.

								if (u_TemperCommon.pDb->wSlowestSegNum > 0) // Anders 2018-3-17,add.
								{
									if (!u_aTemperAlg[u_TemperCommon.pDb->wSlowestSegNum - 1].pTemperInternal->bBalanceHeatOk)
									{
										u_TemperCommon.pInternal->lPipeBanlanceAverTemper = u_aTemperAlg[u_TemperCommon.pDb->wSlowestSegNum - 1].pTemperDb->lCurrent;
										iMinSeg                                           = u_TemperCommon.pDb->wSlowestSegNum - 1;
									}
								}

								if (!u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk && iMinSeg != wIndex)
								{ // Anders 2018-3-15, add "&&  iMinSeg != wIndex"
									if (u_TemperCommon.pInternal->lPipeBanlanceAverTemper >= u_TemperCommon.pInternal->lPipeLastBanlanceAverTemper + u_aTemperAlg[wIndex].pTemperConf->wPrecision || u_TemperCommon.pInternal->lPipeBanlanceAverTemper <= u_TemperCommon.pInternal->lPipeLastBanlanceAverTemper - u_aTemperAlg[wIndex].pTemperConf->wPrecision)
									{
										lObjTemp                                              = u_TemperCommon.pInternal->lPipeBanlanceAverTemper + u_TemperCommon.pDb->wPipeBalanceHeatingTemper;
										u_TemperCommon.pInternal->lPipeLastBanlanceAverTemper = u_TemperCommon.pInternal->lPipeBanlanceAverTemper;
									}
									else
									{
										lObjTemp = u_TemperCommon.pInternal->lPipeLastBanlanceAverTemper + u_TemperCommon.pDb->wPipeBalanceHeatingTemper;
									}

									//if ((u_TemperCommon.pDb->wPreserveUse != 0 && u_TemperCommon.pDb->wPreserveTemper != 0) ||
									//    u_TemperCommon.pInternal->bPipeInSoftHeat )
									{
										if (lObjTemp < u_aTemperAlg[wIndex].pTemperInternal->lObjTemp)
										{
											u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = lObjTemp;
										}
									}
									//else
									//{
									//    u_aTemperAlg[wIndex].pTemperInternal->lObjTemp     = lObjTemp;
									//}
								}

								if (wBanlanceHeatNum <= 1)
								{
									u_TemperCommon.pInternal->wPipeBanlanceHeatState = TEMPER_BANLANCEHEAT_STATE_END;
								}

								break;
							default:
								break;
						}
					}
				}
				else
				{
					if (u_TemperCommon.pDb->wMoldBalanceHeatingCmd != 0 && u_TemperCommon.pDb->wMoldBalanceHeatingTemper >= 5 * u_aTemperAlg[wIndex].pTemperConf->wPrecision)
					{
						switch (u_TemperCommon.pInternal->wMoldBanlanceHeatState)
						{
							case TEMPER_BANLANCEHEAT_STATE_IDLE:
								u_TemperCommon.pInternal->wMoldBanlanceHeatState = TEMPER_BANLANCEHEAT_STATE_RUN;
								break;
							case TEMPER_BANLANCEHEAT_STATE_RUN:

								if ((u_TemperCommon.pDb->wMoldPreserveUse == 0 || u_TemperCommon.pDb->wMoldPreserveTemper == 0) && !u_TemperCommon.pInternal->bMoldInSoftHeat && !u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk && u_aTemperAlg[wIndex].pTemperDb->lCurrent >= u_aTemperAlg[wIndex].pTemperDb->lAbsoluteSet - u_aTemperAlg[wIndex].pTemperDb->sDownLimit) // THJ 20150223 Modification: '>' -> '>='
								{
									u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk = TRUE;
								}
								lMinTemper = 8880;
								lMaxTemper = 0;
								for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
								{
									if (u_aTemperAlg[i].pTemperDb->wUse == TEMPER_HEAT_USE_ABSTRACT && u_aTemperAlg[i].pTemperDb->lAbsoluteSet > 0 // Anders 2015-3-18 Add for HT.
									    && !u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk)
									{
										if ((u_aTemperAlg[i].pTemperDb->wDeviceType & 0xFF) == TEMPER_DEV_MOLD)
										{
											if (lMinTemper > u_aTemperAlg[i].pTemperDb->lCurrent)
											{
												lMinTemper = u_aTemperAlg[i].pTemperDb->lCurrent;
												iMinSeg    = i; // Anders 2018-3.15, Add.
											}

											if (lMaxTemper < u_aTemperAlg[i].pTemperDb->lCurrent && u_aTemperAlg[i].pTemperDb->lCurrent < 8880)
											{
												lMaxTemper = u_aTemperAlg[i].pTemperDb->lCurrent;
											}
										}

										if (u_aTemperAlg[i].pTemperDb->wBanlanceHeatUse != 0 && u_aTemperAlg[i].pTemperInternal->bBalanceHeatOk == FALSE)
										{
											wBanlanceHeatNum++;
										}
									}
								}
								u_TemperCommon.pInternal->lMoldBanlanceAverTemper = lMinTemper; //(lMinTemper + lMaxTemper) >> 1; // Anders 2018-3-15, Modify.

								if (!u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk && iMinSeg != wIndex)
								{
									if (u_TemperCommon.pInternal->lMoldBanlanceAverTemper >= u_TemperCommon.pInternal->lMoldLastBanlanceAverTemper + u_aTemperAlg[wIndex].pTemperConf->wPrecision || u_TemperCommon.pInternal->lMoldBanlanceAverTemper <= u_TemperCommon.pInternal->lMoldLastBanlanceAverTemper - u_aTemperAlg[wIndex].pTemperConf->wPrecision)
									{
										lObjTemp                                              = u_TemperCommon.pInternal->lMoldBanlanceAverTemper + u_TemperCommon.pDb->wMoldBalanceHeatingTemper;
										u_TemperCommon.pInternal->lMoldLastBanlanceAverTemper = u_TemperCommon.pInternal->lMoldBanlanceAverTemper;
									}
									else
									{
										lObjTemp = u_TemperCommon.pInternal->lMoldLastBanlanceAverTemper + u_TemperCommon.pDb->wMoldBalanceHeatingTemper;
										;
									}

									//if ((u_TemperCommon.pDb->wPreserveUse != 0 && u_TemperCommon.pDb->wPreserveTemper != 0) ||
									//    u_TemperCommon.pInternal->bPipeInSoftHeat )
									{
										if (lObjTemp < u_aTemperAlg[wIndex].pTemperInternal->lObjTemp)
										{
											u_aTemperAlg[wIndex].pTemperInternal->lObjTemp = lObjTemp;
										}
									}
									//else
									//{
									//    u_aTemperAlg[wIndex].pTemperInternal->lObjTemp     = lObjTemp;
									//}
								}

								if (wBanlanceHeatNum <= 1)
								{
									u_TemperCommon.pInternal->wMoldBanlanceHeatState = TEMPER_BANLANCEHEAT_STATE_END; // THJ 2015-02-11 Modification: wPipeBanlanceHeatState -> wMoldBanlanceHeatState
								}

								break;
							default:
								break;
						}
					}
				}
			}
			else
			{
				u_aTemperAlg[wIndex].pTemperInternal->bBalanceHeatOk = FALSE;
			}
		}
		//+++++
		// Calculate temper heat time
		//+++++
		switch (u_aTemperAlg[wIndex].pTemperDb->wDeviceType)
		{
			case TEMPER_DEV_PIPE:
				//现在实自动模式只有1，所以不会出现第一种情况
				if ((u_aTemperAlg[wIndex].pTemperDb->wStatus & TEMPER_ADJUST_MASK) == TMEPER_ADJUST_END && u_aTemperAlg[wIndex].pTemperDb->wAutoPidCmd == TEMPER_ADJ_CMD_START_MODE2 && u_TemperCommon.pDb->wPipePidState == TEMPER_STATE_ADJRUN)
				{
					u_aTemperAlg[wIndex].pTemperInternal->lRealSetTemp = u_aTemperAlg[wIndex].pTemperInternal->lAdjSetTemper;
				}
				else
				{
					u_aTemperAlg[wIndex].pTemperInternal->lRealSetTemp = u_aTemperAlg[wIndex].pTemperInternal->lObjTemp;
				}
				TemperAlg_Calculate(&u_aTemperAlg[wIndex], u_aTemperAlg[wIndex].pTemperDb->wHeatCMDEnable); // CX 20190506 modify u_TemperCommon.pDb->uHeatCmd.sBit.bPipeEnable);
				break;
			case TEMPER_DEV_MOLD:
				if ((u_aTemperAlg[wIndex].pTemperDb->wStatus & TEMPER_ADJUST_MASK) == TMEPER_ADJUST_END && u_aTemperAlg[wIndex].pTemperDb->wAutoPidCmd == TEMPER_ADJ_CMD_START_MODE2 && u_TemperCommon.pDb->wMoldPidState == TEMPER_STATE_ADJRUN)
				{
					u_aTemperAlg[wIndex].pTemperInternal->lRealSetTemp = u_aTemperAlg[wIndex].pTemperInternal->lAdjSetTemper;
				}
				else
				{
					u_aTemperAlg[wIndex].pTemperInternal->lRealSetTemp = u_aTemperAlg[wIndex].pTemperInternal->lObjTemp;
				}
				TemperAlg_Calculate(&u_aTemperAlg[wIndex], u_aTemperAlg[wIndex].pTemperDb->wHeatCMDEnable); // CX 20190506 modifyu_TemperCommon.pDb->uHeatCmd.sBit.bMoldEnable);
				break;
			default:
				break;
		}

		// Monitor. Anders 2015-4-15, Add.
		u_aTemperAlg[wIndex].pTemperDb->lReserved0 = u_aTemperAlg[wIndex].pTemperInternal->lRealSetTemp; // Anders 2018-3-15, Modify.
		                                                                                                 // u_aTemperAlg[wIndex].pTemperDb->wReserved1 = u_aTemperAlg[wIndex].pTemperInternal->wAdjStep;
	}

	if (++u_awTemperIndex[wDeviceType] >= TEMPER_1S_NBR)
	{
		u_awTemperIndex[wDeviceType] = 0;
	}
}

void TemperCommon_RunOutput()
{
	WORD_T i, wIndex, wOffset, wOutput;

	for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++) //
	{
		wIndex  = i >> 5; //(i+0x1F)>>5; // THJ 20150211 Modification:
		wOffset = i & 0x1F;
		wOutput = TemperAlg_Output(&u_aTemperAlg[i], u_aTemperAlg[i].pTemperDb->wHeatCMDEnable); // CX 20190506 modify //u_TemperCommon.pDb->uHeatCmd.dwAll);
		if (wOutput & TEMPER_OUTPUT_HEAT)
		{
			u_aTemperAlg[i].pTemperInternal->dwContinuousHeatTime += u_aTemperAlg[i].pTemperConf->wOutputCycle; //u_aTemperAlg[wIndex].pTemperConf->wOutputCycle    // THJ 20150211 Modification:
			u_TemperCommon.pDb->adwHeatOutput[wIndex] |= (DWORD_T) 1 << wOffset;
		}
		else
		{
			u_aTemperAlg[i].pTemperInternal->dwContinuousHeatTime  = 0;
			u_aTemperAlg[i].pTemperInternal->lHeatCheckStartDegree = u_aTemperAlg[i].pTemperDb->lCurrent;
			u_TemperCommon.pDb->adwHeatOutput[wIndex] &= ~((DWORD_T) 1 << wOffset);
		}

		if (wOutput & TEMPER_OUTPUT_COOL)
		{
			u_TemperCommon.pDb->adwCoolOutput[wIndex] |= (DWORD_T) 1 << wOffset;
		}
	}
}
/*------------------------------------------------------------------------------+
|           Helpers                                                             |
+------------------------------------------------------------------------------*/
LONG_T TemperCommon_CalCurrent(WORD_T wIndex,
    WORD_T                            wCoolJuncAd)
{
	WORD_T              i;
	WORD_T              wAdValue;
	WORD_T              wCoolAdValue;
	const AD2DEGREE_T * pTempTbl;
	// LONG_T             lCurrent;

	//+++++
	//  Debug
	//+++++
	ASSERT(pTemper);

	//+++++
	//  Select temper table
	//+++++

	// Anders 2018-4-11 add.
	i = u_aTemperAlg[wIndex].pTemperDb->wSensorType >> 8;
	switch (i)
	{
		case 0:
			pTempTbl = u_apTemperTypeTbl[u_aTemperAlg[wIndex].pTemperDb->wSensorType & 0xFF];
			break;
		case 1:
			pTempTbl = u_apTemperTypeTbl2[u_aTemperAlg[wIndex].pTemperDb->wSensorType & 0xFF];
			break;
		default:
			return 9998;
	}

	//+++++
	//  Read cooljunction ad and temper ad
	//+++++
	wCoolAdValue = wCoolJuncAd >> 4;
	wAdValue     = u_aTemperAlg[wIndex].pTemperDb->wAD >> 3;

	if (0 == u_aTemperAlg[wIndex].pTemperDb->wUse)
		u_aTemperAlg[wIndex].pTemperDb->lCurrent = 0;
	else if (wAdValue == 0)                                    // Anders 2018-4-12, add.
		u_aTemperAlg[wIndex].pTemperDb->lCurrent = 9999;       // Anders 2018-4-12, add.
	else if (wAdValue > pTempTbl[TEMPERTABLE_NUMBER - 1].dwAd) // Anders 2018-4-12, add.
		u_aTemperAlg[wIndex].pTemperDb->lCurrent = 9880;       // Anders 2018-4-12, add.
	else if (wAdValue < pTempTbl[0].dwAd)                      // else if (wAdValue > 8006 || wAdValue < 128) // Anders 2018-4-11, modify.
		u_aTemperAlg[wIndex].pTemperDb->lCurrent = 8880;
	else
	{
		//+++++
		//  Convert ad to temperature
		//+++++

		// if (wAdValue > pTempTbl[TEMPERTABLE_NUMBER - 1].dwAd)
		// {
		//     wAdValue = pTempTbl[TEMPERTABLE_NUMBER - 1].dwAd;
		//     i        = TEMPERTABLE_NUMBER - 1; // Anders 2016-2-18, Add.
		// }
		// else // Anders 2016-2-18, Add.
		// {
		//     i = 1;
		//     while (wAdValue > pTempTbl[i].dwAd)
		//     {
		//         i++;
		//     }
		// }

		i = 1;
		while (wAdValue > pTempTbl[i].dwAd)
		{
			i++;
		}

		u_aTemperAlg[wIndex].pTemperDb->lCurrent = pTempTbl[i].lDegree * u_aTemperAlg[wIndex].pTemperConf->wPrecision - (FLOAT_T)(pTempTbl[i].lDegree - pTempTbl[i - 1].lDegree) * u_aTemperAlg[wIndex].pTemperConf->wPrecision * (pTempTbl[i].dwAd - wAdValue) / (pTempTbl[i].dwAd - pTempTbl[i - 1].dwAd) + wCoolAdValue - 2731;

		// if (u_aTemperAlg[wIndex].pTemperDb->bRealTimeAvg)
		// {
		// }

		// if (u_aTemperAlg[wIndex].pTemperDb->lCurrent == 0)
		//   u_aTemperAlg[wIndex].pTemperDb->lCurrent = lCurrent;
		// else
		//   u_aTemperAlg[wIndex].pTemperDb->lCurrent = (u_aTemperAlg[wIndex].pTemperDb->lCurrent * 7 + lCurrent) / 8;
	}

	return u_aTemperAlg[wIndex].pTemperDb->lCurrent;
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
void TemperCommon_ReConfigCalCycle(WORD_T wIndex)
{
	if (wIndex >= u_TemperCommon.pInternal->wNumber)
		return;

	if (u_TemperCommon.pDb->wMoldFastOpt && u_aTemperAlg[wIndex].pTemperDb->wDeviceType == TEMPER_DEV_MOLD)
		u_aTemperAlg[wIndex].pTemperConf = u_TemperCommon.pFastAlgConf;
	else
		u_aTemperAlg[wIndex].pTemperConf = u_TemperCommon.pNormalAlgConf;
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
void TemperCommon_ReConfigAllCalCycle()
{
	WORD_T i;
	for (i = 0; i < u_TemperCommon.pInternal->wNumber; i++)
		TemperCommon_ReConfigCalCycle(i);
}
