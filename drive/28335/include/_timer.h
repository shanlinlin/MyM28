//###########################################################################
//
// FILE:    I32_timer.h
//
// TITLE:   DSP281x CPU 32-bit Timers Initialization & Support Functions.
//
// NOTES:   CpuTimer1 and CpuTimer2 are reserved for use with DSP BIOS and
//          other realtime operating systems.
//
//          Do not use these two timers in your application if you ever plan
//          on integrating DSP-BIOS or another realtime OS.
//
//          For this reason, the code to manipulate these two timers is
//          commented out and not used in these examples.
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  1.00| 08 may 2007 | c.W. | Changes from previous version (c.c.chen _timer.h)
//      |             |      |
//      |             |      |
//###########################################################################

#ifndef D___TIMER
#define D___TIMER

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "utils.h"
//#include "type.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*******
|
*******/
#define CPUTIMER0_SLICE 1000                 // CPU TIMER0 SLICE 1ms: 1000; 0.5ms: 500 zholy080305. Not exeed 1000 !!!
                                             // If cthis value changes, other config based on cpu timer0 will change at the same time.
#define TIMER_SCALE (1000 / CPUTIMER0_SLICE) // 1000 stand for 1ms which is standard.
#define CPUTIMER0_TIMEMS (1.0 / TIMER_SCALE) // CPU Timer interrupt time(ms).
/*******
|
*******/
//#define     MAX_COUNTER         64
#define MAX_ROUTINE 4                        //32
#define TIMER_SLICE (CPUTIMER0_SLICE / 1000) /* time tick = 2ms */

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
//#define       SYSTEMTIME_UPMS                                 g_lSystemUpTick
//#define       ResetTimer(p_lTimer ,lOutTime)                  (*(p_lTimer) = SYSTEMTIME_UPMS + ((LONG)(lOutTime) < 0 ? 0 : (lOutTime)) )
//#define       CheckTimerOut(p_lTimer)                         (SYSTEMTIME_UPMS - *(p_lTimer) >= 0)
/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
//void AddUpCounter   (long* ptCounter);
//void DeleteUpCounter(long* ptCounter);
//void AddDnCounter   (long* ptCounter);
//void DeleteDnCounter(long* ptCounter);
//void AddRoutine   (ROUTINE* proutine);   /* Can not execute AddRountine() and DeleteRoutine() inside the routine */
//void DeleteRoutine(ROUTINE* proutine);   /* Can not execute AddRountine() and DeleteRoutine() inside the routine */

interrupt void cpu_timer0_isr(void);

/*===========================================================================+
|           External                                                         |
+===========================================================================*/

extern CPULOAD g_CPULoad;

extern PFUNC g_pFuncCritical;

//extern    int         g_nUpCounter;
//extern    int         g_nDnCounter;
extern int g_nRoutine;
//extern    long*       g_ptUpCounter[MAX_COUNTER];
//extern    long*       g_ptDnCounter[MAX_COUNTER];

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
