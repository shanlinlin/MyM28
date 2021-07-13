/*===========================================================================+
|  Function : PWM Driver                                                     |
|  Task     : PWM interrupt service routine                                  |
|----------------------------------------------------------------------------|
|  Compile  : CCS3.1 -                                                   	 |
|  Link     : CCS3.1 -                                                   	 |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : Winston.Yu                                                     |
|  Version  : V1.00                                                          |
|  Creation : 07/31/2009                                                     |
|  Revision :                                                          	     |
|                                                     						 |
|             							                                     |
+===========================================================================*/
#ifndef D___PWM
#define D___PWM

#include "common.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
#define PWM_INIT_STATE (FREE_RUN_FLAG + TIMER_CONT_UPDN + TIMER_CLK_PRESCALE_X_1 + TIMER_ENABLE_BY_OWN + TIMER_ENABLE)
#define PWM_INIT_STATE2 (FREE_RUN_FLAG + TIMER_CONT_UPDN + TIMER_CLK_PRESCALE_X_1 + TIMER_ENABLE_BY_OWN + TIMER_ENABLE + TIMER_ENABLE_COMPARE)
#define ACTR_PWM_OFF_STATE (COMPARE1_FH + COMPARE2_FH + COMPARE3_FH + COMPARE4_FH + COMPARE5_FH + COMPARE6_FH)
#define ACTR_INIT_STATE (COMPARE1_AL + COMPARE2_AH + COMPARE3_AL + COMPARE4_AH + COMPARE5_AL + COMPARE6_AH)
#define DBTCON_INIT_STATE (DBT_VAL_14 + EDBT3_EN + EDBT2_EN + EDBT1_EN + DBTPS_X32)

#define PWM_SUCCESS 0
#define PWM_FAILURE 1

#define PWM_PWMA 0
#define PWM_PWMB 1
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct TAGPWMGEN
{
	WORD wPeriod;     /* Input: PWM Half-Period in CPU clock cycles (Q0)	*/
	                  //	int		Mfunc_p;																/* Input: Period scaler (Q15)						*/
	long lDutyCycle1; /* Input: PWM 1&2 Duty cycle ratio (Q15)			*/
	long lDutyCycle2; /* Input: PWM 3&4 Duty cycle ratio (Q15)			*/
	long lDutyCycle3; /* Input: PWM 5&6 Duty cycle ratio (Q15)			*/
} PWMGEN;

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
WORD _OpenPWM(WORD wHandle, PWMGEN * pPWMGen);
WORD _ClosePWM(WORD wHandle);
void _PWMOutput(PWMGEN * pPWMGen);
void _PWMOff(PWMGEN * pPWMGen);
void _PWMOn(PWMGEN * pPWMGen);
void _PWMDeadTimeSet(int nDeadTime);
#endif
