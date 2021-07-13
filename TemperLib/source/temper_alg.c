/*==============================================================================+
|  Function : Temper algorithm                                                  |
|  Task     : Temper algorithm Source File                                    	|
|-------------------------------------------------------------------------------|
|  Compile  :                                               	                |
|  Link     :                                                  	                |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Wain.Wei                                                          |
|  Version  : V1.00                                                             |
|  Creation : 2012/08/10                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "temper_alg.h"
#include <math.h>
/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/
#define TEMPER_MAX_KP 0xFFFFL

#define TEMPER_MAX_DUTYVALUE 0xFFFFL

//+++++
//	Auto adjust step
//+++++

#define TEMPER_ADJSTEP_IDLE 0
#define TEMPER_ADJSTEP_ACCEND 1
#define TEMPER_ADJSTEP_CAL 2
#define TEMPER_ADJSTEP_WAIT 3
#define TEMPER_ADJSTEP_END 4

#define TEMPER_ADJUST_MIN_TEMPER 100
// #define HEATWAY_RELAY_MINHEATTIME (TEMPER_MAX_DUTYVALUE * 3 / 5) // Anders 2019-11-14,modify Relay MinTime 0.15s->0.06s. // (TEMPER_MAX_DUTYVALUE * 3 / 2) // Relay MinTime 0.15s.
#define HEATWAY_RELAY_MINHEATTIME (15 * TEMPER_MAX_DUTYVALUE) // Relay MinTime 1.5s.

#define TICK_COUNT_S 1000.0

#define TEMPER_CALCOUNT_PER_S 10.0

#define TEMPER_CALREATE_TEMPER_INTERVAL     5
/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/
extern volatile DWORD g_dwSystemUpTick;

/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/

/*==============================================================================+
|           Globle Variables                                                    |
+==============================================================================*/
WORD g_wTemper_alg_MinOnTime = 600; // ms
/*==============================================================================+
|           Class declaration - Temper algorithm                            	|
+==============================================================================*/
/*------------------------------------------------------------------------------+
|           Helpers Implementation                                              |
+------------------------------------------------------------------------------*/
static void TemperAlg_Adjust(TEMPER_ALG_T * pTemperAlg);
/*------------------------------------------------------------------------------+
|           Inner Globle Variable                                               |
+------------------------------------------------------------------------------*/
WORD_T u_wHeatCmd = TEMPER_HEAT_OFF;
/*------------------------------------------------------------------------------+
|           Constructor and destructor                                          |
+------------------------------------------------------------------------------*/
void TemperAlg_Create(TEMPER_ALG_T * pTemperAlg,
    const TEMPER_ALG_CONF_T *        pConf,
    TEMPER_ALG_DB_T *                pTemperDb,
    TEMPER_ALG_INTERNAL_T *          pTemperInternal)
{
    pTemperAlg->pTemperConf       = pConf;
    pTemperAlg->pTemperDb         = pTemperDb;
    pTemperAlg->pTemperInternal   = pTemperInternal;
    pTemperInternal->fMaxRate     = 1; // hankin 2019-07-10 add: bug(/ 0), no warming.
    pTemperInternal->fKPFactor    = 1;
    pTemperInternal->fKpAdjFactor = 1;
}
/*------------------------------------------------------------------------------+
|           Operations                                                          |
+------------------------------------------------------------------------------*/
void TemperAlg_Calculate(TEMPER_ALG_T * pTemperAlg,
    WORD_T                              wHeatCmd)
{
    const TEMPER_ALG_CONF_T * pConf;
    TEMPER_ALG_DB_T *         pDb;
    TEMPER_ALG_INTERNAL_T *   pInternal;
    LONG_T                    lDe;
    LONG_T                    lSum, lSumMax;
    FLOAT_T                   fRate;
    DWORD_T                   dwTick;
    LONG_T                    lSumOffset;
    FLOAT_T                   fTemp;
    WORD_T                    wKp;
    //WORD_T                    wKd;
    static  WORD_T            wCount    =   0;
    FLOAT_T                   fMaxRate;

    //+++++
    //	Debug
    //+++++
    ASSERT(pTemperAlg);

    //+++++
    //	Local pointer assignment
    //+++++
    pConf     = pTemperAlg->pTemperConf;
    pDb       = pTemperAlg->pTemperDb;
    pInternal = pTemperAlg->pTemperInternal;

    if (g_dwSystemUpTick <= 10000)
        return;

    u_wHeatCmd = wHeatCmd;
    dwTick     = g_dwSystemUpTick - pInternal->dwLastTick;


    if ( pInternal->wPIDChange == 0 )
    {
        pInternal->wKp      =       pDb->wKp;
        pInternal->wKd      =       pDb->wKd;
    }
    else//快速PID功能，目前也没用
    {
        pInternal->wKp      =       pDb->wKp * 4 / 5;
        pInternal->wKd      =       pDb->wKd * 4 / 5;
    }

    //+++++
    //	Calculate De and Dde
    //+++++
    /*
	if (pInternal->lLastCurrent != 0 && pDb->lCurrent != pInternal->lLastCurrent)
	{
		fRate                    = (FLOAT_T)(pDb->lCurrent - pInternal->lLastCurrent) * TICK_COUNT_S / (g_dwSystemUpTick - pInternal->dwLastRateTick);
		pInternal->dwLastRateTick = g_dwSystemUpTick;

		if ((pDb->wStatus & TEMPER_ADJUST_MASK) != TEMPER_ADJUST_RUNNING)
		{
			pInternal->fMaxRate = 0.4 * pInternal->wKp / pDb->wKd;
		}
		else
		{
			if (pInternal->fMaxRate < ABS(pInternal->fRate))
				pInternal->fMaxRate = ABS(pInternal->fRate);
		}

		if (pInternal->fMaxRate < 1)
			pInternal->fMaxRate = 1;

		pInternal->fRate = (pInternal->fRate * 63 / pInternal->fMaxRate + fRate) / (63 / pInternal->fMaxRate + 1);
	}

	fRate             = (FLOAT_T)(pDb->lCurrent - pInternal->lLastCurrent) * TICK_COUNT_S / dwTick;
	pInternal->fRate1 = (pInternal->fRate1 * 63 / pInternal->fMaxRate + fRate) / (63 / pInternal->fMaxRate + 1);

	lDe            = pInternal->lRealSetTemp - pDb->lCurrent;
	pInternal->lDe = lDe;

	pDb->lAdjStableDuty = pInternal->fRate * 1000;
	*/



    if (pInternal->lLastCurrent == 0)
    {
        pInternal->aRateBuffer[0].lCurrent     = pDb->lCurrent;
        pInternal->aRateBuffer[0].dwSampleTick = g_dwSystemUpTick;
        pInternal->byRateBufIndex              = 1;
    }
    else if ( pDb->lCurrent != pInternal->lLastCurrent )
    {
        if ( pInternal->wTemperSampleLimit == 0 )
            pInternal->wTemperSampleLimit    =   10;

        if ( ABS(pDb->lCurrent - pInternal->lLastCurrent) <= pInternal->wTemperSampleLimit )
        {
            wCount      =   0;
            if ( pInternal->wTemperSampleLimit <= 2 * ABS(pDb->lCurrent - pInternal->lLastCurrent) )
                pInternal->wTemperSampleLimit  =  2 * ABS(pDb->lCurrent - pInternal->lLastCurrent);


            if (pInternal->byRateBufIndex < 2)
            {
                pInternal->aRateBuffer[1].lCurrent     = pDb->lCurrent;
                pInternal->aRateBuffer[1].dwSampleTick = g_dwSystemUpTick;
                pInternal->byRateBufIndex              = 2;
            }
            else
            {
                if ((pInternal->aRateBuffer[0].lCurrent == pInternal->aRateBuffer[1].lCurrent) || (pInternal->aRateBuffer[1].lCurrent > pInternal->aRateBuffer[0].lCurrent && pDb->lCurrent < pInternal->aRateBuffer[1].lCurrent) || (pInternal->aRateBuffer[1].lCurrent < pInternal->aRateBuffer[0].lCurrent && pDb->lCurrent > pInternal->aRateBuffer[1].lCurrent))
                {
                    pInternal->aRateBuffer[1].lCurrent     = pDb->lCurrent;
                    pInternal->aRateBuffer[1].dwSampleTick = g_dwSystemUpTick;
                    fRate                                  = (FLOAT_T)(pInternal->aRateBuffer[1].lCurrent - pInternal->aRateBuffer[0].lCurrent) * TICK_COUNT_S / (pInternal->aRateBuffer[1].dwSampleTick - pInternal->aRateBuffer[0].dwSampleTick);
                }
                else
                {
                    fRate = (FLOAT_T)(pDb->lCurrent - pInternal->aRateBuffer[0].lCurrent) * TICK_COUNT_S / (g_dwSystemUpTick - pInternal->aRateBuffer[0].dwSampleTick);

                    pInternal->aRateBuffer[0].lCurrent     = pInternal->aRateBuffer[1].lCurrent;
                    pInternal->aRateBuffer[0].dwSampleTick = pInternal->aRateBuffer[1].dwSampleTick;
                    pInternal->aRateBuffer[1].lCurrent     = pDb->lCurrent;
                    pInternal->aRateBuffer[1].dwSampleTick = g_dwSystemUpTick;
                }

                if ((pDb->wStatus & TEMPER_ADJUST_MASK) != TEMPER_ADJUST_RUNNING)
                {
                    pInternal->fMaxRate = 0.4 * pInternal->wKp / pDb->wKd;
                }
                else
                {
                    if (pInternal->fMaxRate < ABS(pInternal->fRate))
                        pInternal->fMaxRate = ABS(pInternal->fRate);
                }

                if (pInternal->fMaxRate < 1)
                    pInternal->fMaxRate = 1;

                //if ((pDb->wStatus & TEMPER_ADJUST_MASK) == TEMPER_ADJUST_RUNNING) // Wei Sir 2020-5-6
                //	pInternal->fRate = (pInternal->fRate * 31 + fRate) / 32; // wain 2019-6-25, add
                //else
                pInternal->fRate = (pInternal->fRate * 63 / pInternal->fMaxRate + fRate) / (63 / pInternal->fMaxRate + 1);
            }
        }
        else
        {
            if ( ++wCount >= 4 )
            {
                pInternal->wTemperSampleLimit           =   2 * ABS(pDb->lCurrent - pInternal->lLastCurrent);
                wCount                                  =   0;
            }
        }
    }

    fRate             = (FLOAT_T)(pDb->lCurrent - pInternal->lLastCurrent) * TICK_COUNT_S / dwTick;
    pInternal->fRate1 = (pInternal->fRate1 * 63 / pInternal->fMaxRate + fRate) / (63 / pInternal->fMaxRate + 1);

    //pDb->lStableDuty = pInternal->fRate * 1000;

    lDe            =    pInternal->lRealSetTemp - pDb->lCurrent;
    pInternal->lDe = lDe;

    fMaxRate        =   pInternal->fMaxRate;

    //+++++
    //	if tem	per not use or temper heat not use, reset some parameter
    //+++++
    if (pDb->wUse == 0 || pDb->wUse == TEMPER_HEAT_USE_NOHEATING || wHeatCmd == TEMPER_HEAT_OFF || (pDb->wUse == TEMPER_HEAT_USE_ABSTRACT && pDb->lAbsoluteSet <= 0) // Anders 2015-3-18, Add for HT.
        || pDb->wOutputCycle == 0                                                                                                                                    // Anders 2016-11-1, Add.
        )
    {
        pInternal->lSum               = 0;
        pInternal->lDuty              = 0;
        pInternal->wWholeCalCount     = 0;
        pInternal->lWholeDuty         = 0;
        pInternal->wRealOutputPercent = 0;         // Anders 2017-4-28, add.
        pDb->wStatus &= ~TEMPER_HEAT_PERCENT_MASK; // Anders 2017-4-28, add.
        pInternal->fPIDAdjFactor = 1;

        if (pInternal->fPIDHeatMaxFactor == 0)
            pInternal->fPIDHeatMaxFactor = 1;

        pDb->wStatus &= ~TEMPER_CONTINUOUS_ERR; // Anders 2015-3-30, Add for HT.
        pDb->dwError &= ~TEMPER_ERR_HEATCHECK;

        pInternal->wCycleState            = 0;
        pInternal->dwTemperStartCalTemper = 0;
        pInternal->dwTemperStartCalTick   = 0;
        pInternal->lStartHeatTemper       = pDb->lCurrent;

        pInternal->dwLastOutputTick         =   g_dwSystemUpTick;
    }
    else
    {
        /*
		//+++++
		//	Calculate De and Dde
		//+++++

			if (pInternal->lLastCurrent != 0)
			{
				fRate = (FLOAT_T)(pDb->lCurrent - pInternal->lLastCurrent) * TICK_COUNT_S / dwTick;

				pInternal->fStableRate = (pInternal->fStableRate * 31 + fRate) / 32;

				if (pInternal->fMaxRate < ABS(pInternal->fStableRate))
					pInternal->fMaxRate = ABS(pInternal->fStableRate);

				if (pInternal->fMaxRate < 1)
					pInternal->fMaxRate = 1;

				pInternal->fRate1 = (pInternal->fRate1 * 63 / pInternal->fMaxRate + fRate) / (63 / pInternal->fMaxRate + 1);

				if (pInternal->lLastCurrent != pDb->lCurrent)
				{
					pInternal->fRate = (pInternal->fRate1 + pInternal->fRate2) / 2;
				}

				pInternal->fRate2 = pInternal->fRate1;
			}
			lDe            = pInternal->lRealSetTemp - pDb->lCurrent;
			pInternal->lDe = lDe;
		*/

        //+++++
        //	Check limit and De
        //+++++
        if (pDb->lCurrent - pInternal->lRealSetTemp > pDb->sStopUpLimit && pDb->wUse == 1)
        {
            pInternal->lDuty = 0;
            //pDb->wStatus	=	(pDb->wStatus & ~TEMPER_HEAT_STATE_MASK) | TMPERR_HEAT_LIMIT_OVERSTOP;//Comment out byxc 20140917
        }
        /*else if (pDb->lCurrent - pDb->lObjTemp > pDb->sUpLimit &&
				pDb->wUse == 1)
		{
			pDb->wStatus	=	(pDb->wStatus & ~TEMPER_HEAT_STATE_MASK) | TEMPER_HEAT_LIMIT_OVER;
		}
		else
		{
			pDb->wStatus	=	(pDb->wStatus & ~TEMPER_HEAT_STATE_MASK);
		}*/
        //Comment out byxc 20140917

        //+++++
        //	Dehumidification heat
        //+++++

        //+++++
        //	Relative heat
        //+++++
        if (pDb->wUse == TEMPER_HEAT_USE_RELATIVE)
        {
            pInternal->lDuty         = (LONG_T) pDb->wRelativeHeatPercent * TEMPER_MAX_DUTYVALUE / 100;
            pInternal->bRelativeHeat = TRUE;

            pInternal->wCycleState            = 0;
            pInternal->dwTemperStartCalTemper = 0;
            pInternal->dwTemperStartCalTick   = 0;
        }
        //+++++
        //	Abstract heat
        //+++++
        else if ((pDb->wStatus & TEMPER_ADJUST_MASK) == TEMPER_ADJUST_RUNNING)
        {

            pInternal->wCycleState            = 0;
            pInternal->bRelativeHeat          = FALSE;
            pInternal->dwTemperStartCalTemper = 0;
            pInternal->dwTemperStartCalTick   = 0;
            TemperAlg_Adjust(pTemperAlg);
        }
        else
        {
            pInternal->bRelativeHeat = FALSE;
            //+++++
            //	Check if temper setting change
            //+++++
            if (pInternal->lRealSetTemp != pInternal->lLastAbsoluteSet)
            {
                pInternal->lLastAbsoluteSet = pInternal->lRealSetTemp;
                pInternal->lSum             = 0;
            }

            //+++++
            //	Check if change pid state
            //+++++

            if (pInternal->fPIDAdjFactor == 0)
            {
                pInternal->fPIDHeatMaxFactor = 1;
                pInternal->fPIDAdjFactor     = 1;
            }

            /*
			if ( pInternal->fPIDAdjFactor > pInternal->fPIDHeatMaxFactor )
			{
				pInternal->fPIDAdjFactor	=	pInternal->fPIDHeatMaxFactor;
			}*/
            wKp = pInternal->wKp * pInternal->fPIDAdjFactor * pInternal->fPIDHeatMaxFactor;

            if ( (pDb->wStatus & TEMPER_ADJUST_MASK) == TEMPER_ADJUST_IDLE )
            {
                if ( pDb->fDecel > 1 )
                    pInternal->fKpAdjFactor     =   pDb->fDecel;
            }

            if (pDb->wKd >= 1)
            {
                if ( pInternal->wRapidCompensationUse != 0 && pInternal->lDe < wKp )
                {
                    if (pInternal->fRate < 0)
                    {
                        fRate = 0.4 * wKp / pDb->wKd;//fRate是理论最大速度

                        fTemp = (fRate + 1.1 * pInternal->fRate) / fRate;

                        if (fTemp < 0)
                        {
                            pInternal->fPIDAdjFactor = 10.0 / wKp;
                            wKp                      = wKp * pInternal->fPIDAdjFactor;
                            /*
                            if (pInternal->lDe < 0)
                                pInternal->lAdjStableDuty = 1000 * (1 - pInternal->fPIDAdjFactor);
                            else if (wKp > pInternal->lDe)
                                pInternal->lAdjStableDuty = 1000 * (1 - pInternal->fPIDAdjFactor) * (wKp - pInternal->lDe) / wKp;
                            */
                            if (wKp > pInternal->lDe)
                                pInternal->lAdjStableDuty = 1000 * (1 - pInternal->fPIDAdjFactor);
                        }
                        else
                        {
                            //lTemp 	= lTemp * lTemp;

                            if (pInternal->fPIDAdjFactor > fTemp)
                            {
                                pInternal->fPIDAdjFactor  = fTemp;
                                pInternal->lAdjStableDuty = 1000 * (1 - pInternal->fPIDAdjFactor);
                                if (pInternal->fPIDAdjFactor < 10.0 / wKp)
                                {
                                    pInternal->fPIDAdjFactor = 10.0 / wKp;
                                }

                                if (pInternal->lDe < 0)
                                    pInternal->lAdjStableDuty = 1000 * (1 - pInternal->fPIDAdjFactor);
                                else if (wKp > pInternal->lDe)
                                    pInternal->lAdjStableDuty = 1000 * (1 - pInternal->fPIDAdjFactor) * (wKp - pInternal->lDe) / wKp;
                            }
                        }
                    }
                }
                else
                {
                    pInternal->fPIDAdjFactor                    =   1;
                    pInternal->lAdjStableDuty                   =   0;
                }


                // 开始加温一段时间后，去除lsum
                if ( pInternal->lStartHeatTemper != 0 )
                {
                    if ( pInternal->lRealSetTemp - pInternal->lStartHeatTemper < 30 )
                    {
                        pInternal->lStartHeatTemper     =   0;
                    }
                    else if ( pInternal->lRealSetTemp - pInternal->lStartHeatTemper <= (pDb->lCurrent-pInternal->lStartHeatTemper) * 10 )
                    {
                        pInternal->fPIDAdjFactor        =   1;
                        pInternal->lSum                 =   0;
                        pInternal->lStartHeatTemper     =   0;
                    }
                }

                lSum = pInternal->lSum;

                if (pInternal->lDe > 10)
                {
                    //FLOAT_T fMaxRate = 0.4 * pDb->wAdjKp * sqrt(pInternal->lDe / ((1 + 0.4) * pDb->wAdjKp)) / pDb->wKd;
                    // FLOAT_T fMaxRate = 0.4 * sqrt(pInternal->lDe * pDb->wAdjKp / 1.4 ) / pDb->wKd;
                    //FLOAT_T fMaxRate = 2.0 * pInternal->lDe  / (7 * pDb->wKd);
                    fMaxRate = 2.0 * wKp * pInternal->lDe / (7ul * wKp * pDb->wKd);//当前温度下，期望的最快速率，根据最快速率和距离目标温度的距离将最快速率慢慢减少
                                                                                   //这个公式也只适用部分机器，不同机器散热不一样，期望速率的算法起始应该是不同的
                    if ( pInternal->fRate1 < 0 )
                        lSum = lSum + 3 * pInternal->lDe * dwTick / TICK_COUNT_S;
                    else if (pInternal->fRate1 < 0.1 * fMaxRate )
                    {
                        lSum = lSum + 2 * pInternal->lDe * dwTick / TICK_COUNT_S;
                        /*
                        if (pInternal->lDe < 30)
                        {
                            lSum = lSum + 2 * pInternal->lDe * dwTick / TICK_COUNT_S;
                        }
                        else
                            lSum = lSum + pInternal->lDe * dwTick / TICK_COUNT_S;*/
                    }
                    else
                    {
                        if (pInternal->fRate1 < 0.6 * fMaxRate)
                        {
                            lSum = lSum + pInternal->lDe * dwTick / TICK_COUNT_S;
                        }
                        else if (pInternal->fRate1 > 0.8 * fMaxRate)
                        {
                            lSum = lSum - pInternal->lDe * dwTick / TICK_COUNT_S;
                        }
                    }
                }
                else
                {
                    if (pInternal->lDe < -30)
                    {
                        pInternal->fPIDAdjFactor  = 1;
                        pInternal->lAdjStableDuty = 0;
                    }
                    lSum = lSum + pInternal->lDe * (LONG_T) dwTick / TICK_COUNT_S;
                }

                // lSumOffset = pInternal->lAdjStableDuty * wKp * pDb->wKi * pInternal->fKpAdjFactor / 1000   ; // wain 2020-5-21 add


                lSumOffset  =   pInternal->lAdjStableDuty * wKp * pDb->wKi * pInternal->fKpAdjFactor / 1000  * pDb->wRamp / 100.0  ;
                lSumMax    = (LONG_T)(wKp - pInternal->lDe) * pDb->wKi * 6 / 5 - lSumOffset;

                if (lSumMax < 0 )
                    lSumMax     =   0;

                if (lSum > lSumMax)
                    lSum = lSumMax;
                if (lSum < -lSumOffset)
                    lSum = -lSumOffset;

                pInternal->lSum = lSum;
            }

            if ( pInternal->lDe < -30)
            {
                pInternal->lDuty = 0;
            }
            else
            {
                if (wKp != 0)
                {

                    if (pDb->wKi != 0)
                        //pInternal->lDuty = (pInternal->lDe + pInternal->lSum / pDb->wKi - pInternal->fRate1 * pDb->wKd) * TEMPER_MAX_DUTYVALUE / (wKp * pInternal->fKpAdjFactor) + pInternal->lAdjStableDuty * TEMPER_MAX_DUTYVALUE / 1000 ;
                        pInternal->lDuty = (pInternal->lDe + pInternal->lSum / pDb->wKi - pInternal->fRate1 * pDb->wKd) * TEMPER_MAX_DUTYVALUE / (pInternal->wKp * pInternal->fKpAdjFactor) + pInternal->lAdjStableDuty * TEMPER_MAX_DUTYVALUE / 1000.0 * pDb->wRamp / 100.0;
                    else
                        //pInternal->lDuty = (pInternal->lDe - pInternal->fRate1 * pDb->wKd) * TEMPER_MAX_DUTYVALUE / (wKp * pInternal->fKpAdjFactor) + pInternal->lAdjStableDuty * TEMPER_MAX_DUTYVALUE / 1000;
                        pInternal->lDuty = (pInternal->lDe - pInternal->fRate1 * pDb->wKd) * TEMPER_MAX_DUTYVALUE / (pInternal->wKp * pInternal->fKpAdjFactor) + pInternal->lAdjStableDuty * TEMPER_MAX_DUTYVALUE / 1000.0 * pDb->wRamp / 100.0;
                }
                else
                    pInternal->lDuty = 0;



                //pDb->wAdjRamp   =   pInternal->lAdjStableDuty;
                //pDb->wAdjRamp  =   ((pInternal->lSum / pDb->wKi) * TEMPER_MAX_DUTYVALUE / (wKp * pInternal->fKpAdjFactor) + pInternal->lAdjStableDuty * TEMPER_MAX_DUTYVALUE / 1000) * 100.0 / TEMPER_MAX_DUTYVALUE;
            }

        }
    }

    //+++++
    //	Calculate if heat time can
    //+++++
    //if ( pDb->wHeatWay	==	TEMPER_HEATWAY_RELAY )
    {
        if (pInternal->bHeatCmdOnTriggle)
        {
            pInternal->wWholeCalCount = pDb->wOutputCycle / pConf->wCalCycle;
            pInternal->lOutputDuty    = pInternal->lDuty * pInternal->wWholeCalCount;
            if ((pDb->wHeatWay == TEMPER_HEATWAY_RELAY && pInternal->lOutputDuty > (LONG_T) pInternal->wWholeCalCount * (TEMPER_MAX_DUTYVALUE * (pDb->wOutputCycle - 600) / pDb->wOutputCycle)) || pDb->wHeatWay == TEMPER_HEATWAY_SSR )
                pInternal->lOutputDuty = (LONG_T) pInternal->wWholeCalCount * TEMPER_MAX_DUTYVALUE; // + 65;

            pInternal->wRealOutputPercent = (FLOAT_T) pInternal->lOutputDuty * 100 / ((LONG_T) pInternal->wWholeCalCount * TEMPER_MAX_DUTYVALUE) + 0.5;
            if (pInternal->wRealOutputPercent > 100) // Anders 2016-12-9, add.
                pInternal->wRealOutputPercent = 100;

            pDb->wStatus &= ~TEMPER_HEAT_PERCENT_MASK;
            pDb->wStatus |= (pInternal->wRealOutputPercent << TEMPER_HEAT_PERCENT_BIT); // THJ 2015-02-11 Add: "(TEMPER_HEAT_OFF == wHeatCmd) ? 0 : "
            pInternal->wWholeCalCount    = 0;
            pInternal->bHeatCmdOnTriggle = FALSE;
            pInternal->dwLastOutputTick         =   g_dwSystemUpTick;
        }
        else
        {
            pInternal->lWholeDuty += pInternal->lDuty;
            pInternal->wWholeCalCount += 1;

            if (pInternal->lWholeDuty < 0)
                pInternal->lWholeDuty = 0;

            //if (pDb->wUse == TEMPER_HEAT_USE_RELATIVE) //Modify byxc 20141206
            {
                if (pDb->wOutputCycle == 0)
                    pInternal->lWholeDuty = 0;
            }

            if ((LONG_T) pInternal->wWholeCalCount * pConf->wCalCycle >= pDb->wOutputCycle)
            {
                WORD_T  wMinOnTime  =   600UL * pDb->wOutputCycle / (g_dwSystemUpTick - pInternal->dwLastOutputTick );
                WORD_T  wMinOffTime =   600UL * pDb->wOutputCycle / (g_dwSystemUpTick - pInternal->dwLastOutputTick );
                // if (pInternal->lWholeDuty >= HEATWAY_RELAY_MINHEATTIME || pDb->wHeatWay == TEMPER_HEATWAY_SSR) // Anders 2015-8-17, Add For Relay.

                if ((pDb->wHeatWay == TEMPER_HEATWAY_RELAY && pInternal->lWholeDuty >= (LONG_T) pInternal->wWholeCalCount * (TEMPER_MAX_DUTYVALUE * wMinOnTime / pDb->wOutputCycle)) || pDb->wHeatWay == TEMPER_HEATWAY_SSR)// Anders 2015-8-17, Add For Relay.
                {
                    pInternal->lOutputDuty = pInternal->lWholeDuty;

                    // Anders 2016-12-21, Relay Off: protect 0.6s.
                    if (pDb->wHeatWay == TEMPER_HEATWAY_RELAY && pInternal->lOutputDuty > (LONG_T) pInternal->wWholeCalCount * (TEMPER_MAX_DUTYVALUE * (pDb->wOutputCycle - wMinOffTime) / pDb->wOutputCycle))
                        pInternal->lOutputDuty = (LONG_T) pInternal->wWholeCalCount * TEMPER_MAX_DUTYVALUE; // + 65;

                    pInternal->wRealOutputPercent = (FLOAT_T) pInternal->lWholeDuty * 100 / ((LONG_T) pInternal->wWholeCalCount * TEMPER_MAX_DUTYVALUE) + 0.5;
                    if (pInternal->wRealOutputPercent > 100) // Anders 2016-12-9, add.
                        pInternal->wRealOutputPercent = 100;
                    pInternal->lWholeDuty = 0;
                }
                else
                {
                    pInternal->lOutputDuty        = 0;
                    pInternal->wRealOutputPercent = 0;
                }
                pInternal->wWholeCalCount = 0;
                pInternal->dwLastOutputTick         =   g_dwSystemUpTick;
                pDb->wStatus &= ~TEMPER_HEAT_PERCENT_MASK;
                pDb->wStatus |= (pInternal->wRealOutputPercent << TEMPER_HEAT_PERCENT_BIT); // THJ 2015-02-11 Add: "(TEMPER_HEAT_OFF == wHeatCmd) ? 0 : "
            }
        }

        if (wHeatCmd != TEMPER_HEAT_OFF && (pDb->wUse == TEMPER_HEAT_USE_ABSTRACT && pDb->lAbsoluteSet > 0))
        {
            // KP Micro Adj
            //自学习时启动，用于抑制自学习完成后到设定温度这一段升温过程中的温度振荡
            if ( pInternal->wTemperMicorAdjUse != 0 )
            {
                if (pInternal->wCycleState == 0)
                {
                    pInternal->lCycleCount         = 0;
                    pInternal->wCycleAdjCount      = 0;
                    pInternal->lCycleUnitMaxTemper = pInternal->lCycleUnitMinTemper;
                    pInternal->wCycleState         = 1;
                }
                if (pInternal->wCycleState == 1)
                {
                    if (pInternal->lCycleUnitMaxTemper <= pDb->lCurrent)
                    {
                        pInternal->lCycleCount++;
                    }
                    else
                    {
                        pInternal->lCycleCount--;
                    }

                    if (pInternal->lCycleCount >= 5)
                    {
                        pInternal->lCycleCount = 0;
                        pInternal->wCycleState = 2;
                    }
                    else if (pInternal->lCycleCount <= -5)
                    {
                        pInternal->lCycleCount = 0;
                        pInternal->wCycleState = 3;
                    }
                }
                else if (pInternal->wCycleState == 2)
                {
                    if (pInternal->lCycleUnitMaxTemper <= pDb->lCurrent)
                    {
                        pInternal->lCycleUnitMaxTemper = pDb->lCurrent;
                        pInternal->lCycleUnitMinOutput = pInternal->wRealOutputPercent;
                    }
                    else if (pInternal->lCycleUnitMaxTemper - pDb->lCurrent <= 5)
                    {

                        if (pInternal->lCycleUnitMaxTemper >= pDb->lAbsoluteSet && pInternal->lCycleUnitMaxTemper - pInternal->lCycleUnitMinTemper >= 10 && pInternal->lCycleUnitMaxOutput - pInternal->lCycleUnitMinOutput - pInternal->lCycleUnitLastOuputErr <= 20)
                        {
                            pInternal->wCycleAdjCount++;
                            pInternal->dwCycleChangeTick   =   g_dwSystemUpTick;
                        }

                        pInternal->lCycleUnitLastTemperErr = pInternal->lCycleUnitMaxTemper - pInternal->lCycleUnitMinTemper;
                        pInternal->lCycleUnitLastOuputErr  = pInternal->lCycleUnitMaxOutput - pInternal->lCycleUnitMinOutput;
                        pInternal->lCycleUnitMinTemper     = pDb->lCurrent;
                        pInternal->wCycleState             = 3;
                    }
                }
                else if (pInternal->wCycleState == 3)
                {
                    if (pInternal->lCycleUnitMinTemper >= pDb->lCurrent)
                    {
                        pInternal->lCycleCount         = 0;
                        pInternal->lCycleUnitMinTemper = pDb->lCurrent;
                        pInternal->lCycleUnitMaxOutput = pInternal->wRealOutputPercent;
                    }
                    else if (pDb->lCurrent - pInternal->lCycleUnitMinTemper >= 5)
                    {

                        if (pInternal->lCycleUnitMaxTemper >= pDb->lAbsoluteSet && pInternal->lCycleUnitMaxTemper - pInternal->lCycleUnitMinTemper >= 10 && pInternal->lCycleUnitMaxOutput - pInternal->lCycleUnitMinOutput - pInternal->lCycleUnitLastOuputErr <= 20)
                        {
                            pInternal->wCycleAdjCount++;
                            pInternal->dwCycleChangeTick   =   g_dwSystemUpTick;
                        }

                        pInternal->lCycleUnitLastTemperErr = pInternal->lCycleUnitMaxTemper - pInternal->lCycleUnitMinTemper;
                        pInternal->lCycleUnitLastOuputErr  = pInternal->lCycleUnitMaxOutput - pInternal->lCycleUnitMinOutput;
                        pInternal->lCycleUnitMaxTemper     = pDb->lCurrent;
                        pInternal->lCycleCount             = 0;
                        pInternal->wCycleState             = 2;
                    }
                }

                if (pInternal->wCycleAdjCount >= 4)
                {
                    FLOAT_T fKpAdjFactor = pInternal->fKpAdjFactor * (1 + pInternal->lCycleUnitLastOuputErr / 100.0);

                    // if (fKpAdjFactor > 4)
                    //     fKpAdjFactor = 4;
                    pInternal->lSum           = pInternal->lSum * fKpAdjFactor / pInternal->fKpAdjFactor;
                    pInternal->fKpAdjFactor   = fKpAdjFactor;
                    pInternal->wCycleAdjCount = 0;

                    pDb->fAdjDecel = pInternal->fKpAdjFactor;
                }


                if ( g_dwSystemUpTick - pInternal->dwCycleChangeTick >= pDb->wKi * TICK_COUNT_S )
                {
                    pInternal->wCycleAdjCount           =   0;
                    pInternal->dwCycleChangeTick        =   g_dwSystemUpTick;
                }
            }
            else
            {
                pInternal->wCycleAdjCount               =   0;
            }

            // 加温太慢，需调整
            if (pInternal->dwTemperStartCalTick == 0)
            {
                pInternal->dwTemperStartCalTick     =   g_dwSystemUpTick;
                pInternal->dwTemperStartCalTemper   =   pDb->lCurrent;
            }
            else if (g_dwSystemUpTick - pInternal->dwTemperStartCalTick >= TICK_COUNT_S * pDb->wKi * wKp / ( pInternal->wKp / 4) )
            {
                if (pInternal->lDe > 8)
                {
                    if (pDb->lCurrent - pInternal->dwTemperStartCalTemper <= 2 )
                    {
                        pInternal->lSum += pInternal->lDe * pDb->wKi / 2 ;
                        pDb->wRamp++;

                    }
                }
                pInternal->dwTemperStartCalTick     =   g_dwSystemUpTick;
                pInternal->dwTemperStartCalTemper   =   pDb->lCurrent;
            }
        }
    }
    // else
    // {
    //	    pDb->lOutputDuty				=	pDb->lDuty;
    // }
    //+++++
    //	Set temper last	current value
    //+++++
    pInternal->lLastCurrent = pDb->lCurrent;
    pInternal->dwLastTick   = g_dwSystemUpTick;
    /*
    {
        int nTemp   =   pInternal->fRate * 100;
        pDb->wAdjRamp   =   nTemp;
        nTemp   =   pInternal->fPIDAdjFactor * 100;
        pDb->wAdjKp     =   nTemp;
        nTemp           =   pInternal->lAdjStableDuty;

        pDb->wAdjKi     =   nTemp;
    }
    */
}

WORD_T TemperAlg_Output(TEMPER_ALG_T * pTemperAlg,
    WORD_T                             wHeatCmd)
{
    WORD_T                    wOutputState = 0;
    TEMPER_ALG_DB_T *         pDb;
    const TEMPER_ALG_CONF_T * pConf;
    TEMPER_ALG_INTERNAL_T *   pInternal;
    //+++++
    //	Debug
    //+++++
    ASSERT(pTemperAlg);

    //+++++
    //	Local pointer assignment
    //+++++
    pConf     = pTemperAlg->pTemperConf;
    pDb       = pTemperAlg->pTemperDb;
    pInternal = pTemperAlg->pTemperInternal;

    //+++++
    //	Cool
    //+++++
    if (pDb->wCoolUse == TEMPER_COOL_ON && (pDb->wStatus & TEMPER_ADJUST_MASK) == TEMPER_ADJUST_IDLE)
    {
        if (pDb->wCoolOn == TEMPER_COOL_OFF && pDb->lCurrent - pInternal->lRealSetTemp > pDb->sCoolStartOffset)
        {
            pDb->wCoolOn = TEMPER_COOL_ON;
            pDb->wStatus |= TEMPER_COOL_STATUS_ON;
        }
        else if (pDb->wCoolOn == TEMPER_COOL_ON && pDb->lCurrent - pInternal->lRealSetTemp < pDb->sCoolStartOffset - pDb->sCoolStopOffset)
        {
            pDb->wCoolOn = TEMPER_COOL_OFF;
            pDb->wStatus &= ~TEMPER_COOL_STATUS_ON;
        }
    }
    else
    {
        pDb->wCoolOn = TEMPER_COOL_OFF;
        pDb->wStatus &= ~TEMPER_COOL_STATUS_ON;
    }

    if (pDb->wCoolOn)
        wOutputState |= TEMPER_OUTPUT_COOL;
    //+++++
    //	Heat
    //+++++
    if (pDb->wUse == TEMPER_HEAT_USE_NOHEATING || wHeatCmd == TEMPER_HEAT_OFF || (pDb->wUse == TEMPER_HEAT_USE_ABSTRACT && pDb->lAbsoluteSet <= 0) // Anders 2015-3-18, Add for HT.
        || pDb->wOutputCycle == 0                                                                                                                  // Anders 2016-11-1, Add.
        )
        pInternal->lOutputDuty = 0;

    if (pInternal->lOutputDuty > 0)
    {
        pInternal->lOutputDuty -= (LONG_T) pConf->wOutputCycle * TEMPER_MAX_DUTYVALUE / pConf->wCalCycle;

        wOutputState |= TEMPER_OUTPUT_HEAT;
        pDb->wStatus |= TEMPER_OUTPUT_HEAT;
    }
    else
    {
        pDb->wStatus &= ~TEMPER_OUTPUT_HEAT;
    }
    return wOutputState;
}

void Temper_Alg_Set_Adj(TEMPER_ALG_T * pTemperAlg,
    WORD_T                             wCmd)
{
    //+++++
    //	Debug
    //+++++
    ASSERT(pTemperAlg);

    pTemperAlg->pTemperDb->wAutoPidCmd = wCmd;
    if (wCmd == TEMPER_ADJ_CMD_START_MODE1 || wCmd == TEMPER_ADJ_CMD_START_MODE2)
    {
        if ((pTemperAlg->pTemperDb->wStatus & TEMPER_ADJUST_MASK) == TEMPER_ADJUST_IDLE)
        {
            pTemperAlg->pTemperDb->wStatus                      &=  ~TEMPER_ADJUST_MASK;
            pTemperAlg->pTemperDb->wStatus                      |=  TEMPER_ADJUST_RUNNING;
            pTemperAlg->pTemperInternal->fRate                  =   0;
            pTemperAlg->pTemperInternal->fMaxRate               =   1;
            pTemperAlg->pTemperInternal->wTemperMicorAdjUse     =   1;

            pTemperAlg->pTemperInternal->wAdjStep      = TEMPER_ADJSTEP_IDLE;
            pTemperAlg->pTemperInternal->lAdjSetTemper = (pTemperAlg->pTemperDb->lAbsoluteSet > 2 * TEMPER_ADJUST_MIN_TEMPER * pTemperAlg->pTemperConf->wPrecision) ? (pTemperAlg->pTemperDb->lAbsoluteSet >> 1) : (pTemperAlg->pTemperDb->lAbsoluteSet < TEMPER_ADJUST_MIN_TEMPER * pTemperAlg->pTemperConf->wPrecision ? pTemperAlg->pTemperDb->lAbsoluteSet : TEMPER_ADJUST_MIN_TEMPER * pTemperAlg->pTemperConf->wPrecision);
        }
    }
    else
    {
        pTemperAlg->pTemperInternal->wTemperMicorAdjUse         = 0;
        pTemperAlg->pTemperDb->wStatus &= ~TEMPER_ADJUST_MASK;
        pTemperAlg->pTemperDb->wStatus |= TEMPER_ADJUST_IDLE;
    }
}

void    Temper_Alg_Set_MicroAdj ( TEMPER_ALG_T *    pTemperAlg,
                                WORD_T            wUse        )
{
    pTemperAlg->pTemperInternal->wTemperMicorAdjUse     =   wUse;

}
/*------------------------------------------------------------------------------+
|           Helpers                                                             |
+------------------------------------------------------------------------------*/
static void TemperAlg_Adjust(TEMPER_ALG_T * pTemperAlg)
{
    TEMPER_ALG_DB_T *       pDb;
    TEMPER_ALG_INTERNAL_T * pInternal;
    // const TEMPER_ALG_CONF_T* pConf;
    // LONG_T lDe;
    //+++++
    //	Debug
    //+++++
    ASSERT(pTemperAlg);

    //+++++
    //	Local pointer assignment
    //+++++
    pDb       = pTemperAlg->pTemperDb;
    pInternal = pTemperAlg->pTemperInternal;

    switch (pInternal->wAdjStep)
    {
        case TEMPER_ADJSTEP_IDLE:
        {
            pInternal->lDuty = 0;
            // if (ABS(pDb->lCurrent - pInternal->lLastCurrent) < 10)
            {
                if (pDb->lCurrent + 200 < pInternal->lAdjSetTemper)
                {
                    if (u_wHeatCmd == TEMPER_HEAT_ON)
                    {
                        pInternal->AdjStartTick             =   g_dwSystemUpTick;
                        pInternal->wAdjStartTemper          =   pDb->lCurrent;
                        pInternal->fAdjMaxRate              =   0;
                        pInternal->wAdjStep                 =   TEMPER_ADJSTEP_ACCEND;
                    }
                }
            }
            break;
        }
        case TEMPER_ADJSTEP_ACCEND:
        {
            pInternal->lDuty = TEMPER_MAX_DUTYVALUE;

            if (pInternal->fRate > pInternal->fAdjMaxRate)
            {
                pInternal->fAdjMaxRate          =   pInternal->fRate;
                pInternal->AdjMaxRateTick       =   g_dwSystemUpTick;
                pInternal->wAdjMaxRateTemper    =   pDb->lCurrent;
            }

            if (pDb->lCurrent >= pInternal->lAdjSetTemper)
            {
                pInternal->wAdjStep             =   TEMPER_ADJSTEP_CAL;
            }
            break;
        }
        case TEMPER_ADJSTEP_CAL:
        {
            FLOAT_T fDelayTime;
            pInternal->lDuty = 0;
            //pInternal->fAdjMaxRate = pInternal->fAdjMaxRate * 0.9;
            fDelayTime = (FLOAT_T)(pInternal->AdjMaxRateTick - pInternal->AdjStartTick) / TICK_COUNT_S - (DWORD_T)(pInternal->wAdjMaxRateTemper - pInternal->wAdjStartTemper) / pInternal->fAdjMaxRate;

            pDb->wAdjKp = fDelayTime * pInternal->fAdjMaxRate ;
            pDb->wAdjKp = (pDb->wAdjKp == 0) ? 1 : pDb->wAdjKp;
            pDb->wAdjKi = fDelayTime * 2.5; // KI°´¼ÆËãÖÜÆÚ¼ÆËã
            pDb->wAdjKi = (pDb->wAdjKi == 0) ? 1 : pDb->wAdjKi;
            pDb->wAdjKd = fDelayTime * 0.4; // KD°´¼ÆËãÖÜÆÚ¼ÆËã
            pDb->wAdjKd = (pDb->wAdjKd == 0) ? 1 : pDb->wAdjKd;

            //fWaitTime   = (FLOAT_T)(pInternal->dwAdjWaitTemperNotUpStartTick - pInternal->dwAdjWaitStartTick) / TICK_COUNT_S;

            pDb->wKp    =   pDb->wAdjKp;
            pDb->wKi    =   pDb->wAdjKi;
            pDb->wKd    =   pDb->wAdjKd;
            pDb->wRamp  =   100;
            //pDb->fDecel = pDb->fAdjDecel;

            pDb->wStatus &= ~TEMPER_ADJUST_MASK;
            pDb->wStatus |= TMEPER_ADJUST_END;

            pInternal->wAdjStep = TEMPER_ADJSTEP_END;


            break;
        }
        default:
            break;
	}
}
