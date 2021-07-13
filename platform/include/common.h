/*===========================================================================+
|  System   : Common                                                         |
|  Task     : Common header file                                             |
|----------------------------------------------------------------------------|
|  Compile  : C320 V6.60 -                                                   |
|  Link     : C320 V6.60 -                                                   |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : C.C. Chen                                                      |
|  Version  : V1.00                                                          |
|  Creation : 08/08/1997                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef D__COMMON
#define D__COMMON

//#include "DSP281x_Device.h"     // DSP281x Headerfile Include File
//#include "DSP281x_Examples.h"   // DSP281x Examples Include File

#include "DSP28x_Project.h"
#include "IQmathLib.h"
#include "debug.h"
#include "float.h"
#include "string.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

#ifndef D_HYB5_335
#define D_HYB5_335
#endif

#ifndef _FAST_FUNC
#define _FAST_FUNC
#endif

#ifndef DONT_CHECK_DRIVE_SWITCHON
#define DONT_CHECK_DRIVE_SWITCHON
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
//#define HYBCARD_USED FALSE
//#define SIEMENS_USED FALSE
// #define PANSTONE_SIN_FATIGURE_CTRL 1
//#define PRESVALVE_CLOSELOOP_USED  FALSE

#ifndef FALSE
#define FALSE (1 == 0)
#endif
#ifndef TRUE
#define TRUE (1 == 1)
#endif
#define NO FALSE
#define YES TRUE
/* define ASCII Control code */
#define NUL 0x00 /* Null                 */
#define SOH 0x01 /* Start Of Head        */
#define MD1 0x01 /*                      */
#define STX 0x02 /* Start of TeXt        */
#define MD2 0x02 /*                      */
#define ETX 0x03 /* End of TeXt          */
#define MD3 0x03 /*                      */
#define BEL 0x07 /* BELl                 */
#define BS 0x08  /* BackSpace            */
#define HT 0x09  /* Horizontal Tabulator */
#define LF 0x0A  /* Line Feed            */
#define VT 0x0B  /* Vertical Tabulator   */
#define HOM 0x0B /*                      */
#define FF 0x0C  /* Form Feed            */
#define CLR 0x0C /*                      */
#define CR 0x0D  /* Carriage Return      */
#define CAN 0x18 /*                      */
#define ESC 0x1B /* ESCape               */
#define US 0x1F  /*                      */

/*******
|
*******/
#define MASK_NULL 0x0000
#define MASK_BIT00 0x01
#define MASK_BIT01 0x02
#define MASK_BIT02 0x04
#define MASK_BIT03 0x08
#define MASK_BIT04 0x10
#define MASK_BIT05 0x20
#define MASK_BIT06 0x40
#define MASK_BIT07 0x80
#define MASK_BIT08 0x0100
#define MASK_BIT09 0x0200
#define MASK_BIT10 0x0400
#define MASK_BIT11 0x0800
#define MASK_BIT12 0x1000
#define MASK_BIT13 0x2000
#define MASK_BIT14 0x4000
#define MASK_BIT15 0x8000

/*******
|
*******/
#define DELAY_500 500L
#define DELAY_1000 1000L

/*******
|
*******/
#define ID_NULL 0
#define NULL_ID ID_NULL
#define HANDLE_NULL 0
#define NULL_HANDLE HANDLE_NULL

/*******
|
*******/
#define TICK_COUNT_UP(cycle_, start_tick_, end_tick_) (((end_tick_) < (start_tick_)) ? ((cycle_) - (start_tick_) + (end_tick_)) : ((end_tick_) - (start_tick_)))
#define TICK_COUNT_DOWN(cycle_, start_tick_, end_tick_) (((end_tick_) > (start_tick_)) ? ((cycle_) - (end_tick_) + (start_tick_)) : ((start_tick_) - (end_tick_)))
#define TICK_TIMER_UPDATE(dir_, type_, cycle_, start_tick_, end_tick_, timer_) \
    {                                                                          \
        type_ curr_tick_;                                                      \
        type_ tick_count_;                                                     \
                                                                               \
        curr_tick_    = (end_tick_);                                           \
        tick_count_   = TICK_COUNT_##dir_(cycle_, start_tick_, curr_tick_);    \
        (start_tick_) = curr_tick_;                                            \
        (timer_) += tick_count_;                                               \
    }

// #define OffsetOf(D_Struct, D_Member) ((unsigned long)&(((D_Struct*)0)->D_Member))
// #define ContainerOf(D_pMemberAddr, D_Struct, D_Member) ((D_Struct*)(((char*)(D_pMemberAddr)) - ((char*)OffsetOf(D_Struct, D_Member))))
//20210609 modify
#define OffsetOf(struct_, member_) ((uintptr_t) & (((struct_*)0)->member_))
#define ContainerOf(member_addr_, struct_, member_) ((struct_*)(((uintptr_t)(member_addr_)) - OffsetOf(struct_, member_)))


#define size_to_byte(size_) ((size_)*2)
#define byte_to_size(byte_) (((byte_) + 1) / 2)
#define size_to_word(size_) (size_)
#define word_to_size(word_) (word_)
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef char          CHAR;   /* 16-Bit */
typedef unsigned int  BYTE;   /* 16-Bit CWLM */
typedef unsigned int  WORD;   /* 16-Bit */
typedef int           INT;    /* 16-Bit */
typedef short         SHORT;  /* 16-Bit */
typedef WORD          BOOL;   /* 16-Bit */
typedef WORD          TBOOL;  /* 16-Bit */
typedef long          LONG;   /* 32-Bit */
typedef unsigned long DWORD;  /* 32-Bit */
typedef float         FLOAT;  /* 32-Bit */
typedef float         TFLOAT; /* 32-Bit */
typedef double        DOUBLE; /* 32-Bit */

typedef WORD RTTI;   /* Run Time Type Information */
typedef WORD HANDLE; /* Handle (ID) of object     */

typedef int TINT; /* Timer unit, must be signed type */

typedef WORD  _DA;
typedef WORD  _AD;
typedef WORD  _TIMER;
typedef WORD  _COUNT;
typedef WORD  _FLAG;
typedef DWORD _TIMEL;
typedef DWORD _COUNTL;
typedef DWORD _FLAGL;
//typedef double TFLOAT;

typedef void (*PFUNC)();

typedef struct tagROUTINE {
    TINT tSlice;        /* The period in 1/1024 second to execute the routine */
    void (*pRoutine)(); /* Pointer to user defined routine                    */
} ROUTINE;

typedef struct tagROUTINE_ {
    TINT    tCounter;
    ROUTINE routine;
} ROUTINE_;

typedef struct tagBYTEX {
    WORD LBYTE : 8;
    WORD HBYTE : 8;
} BYTEX;
/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
#define LOBYTE(w) ((BYTE)((w)&0x00FF))
#define HIBYTE(w) ((BYTE)((WORD)(w) >> 8))
#define LOWORD(l) ((WORD)(l))
#define HIWORD(l) ((WORD)((DWORD)(l) >> 16))
#define CHANGEW(w) ((WORD)(((WORD)(w) >> 8) | (((WORD)(w)&0x00FF) << 8)))
#define CHANGEDW(dw) ((DWORD)(((DWORD)(dw) >> 16) | ((DWORD)(dw)&0x0000FFFF) << 16))

#define SWAPWORD(x) ((WORD)(((((WORD)(x)) & 0xFF) << 8) + ((((WORD)(x)) & 0xFF00) >> 8)))
#define SWAPDWORD(x) ((DWORD)(((((DWORD)(x)) & 0xFFFF) << 16) + ((((DWORD)(x)) & 0xFFFF0000) >> 16)))

#define ABS(n) (((n) >= 0) ? (n) : -(n))

#define _inPortW(a) (*(volatile WORD*)(a))
#define _outPortW(a, v) (*(volatile WORD*)(a) = (WORD)(v))

#ifndef NULL /* Defined in <stdlib.h> */
#define NULL ((void*)0)
#endif
#ifndef sizeofB
#define sizeofB(a) ((sizeof(a) / sizeof(WORD)) << 1)
#endif
#define sizeofW(a) (sizeof(a) / sizeof(WORD))

#define SIZE_LM(W) ((W) <= 64 ? (W) : (((W) % 64 == 0) ? (W) : (((W) / 64 + 1) * 64))) // Check DSP28 Large Memory. Anders 20120515.
/**
 * Float EQ. Anders 20110925.
 */
#define FLOAT_EQ(x, v) (fabs((x) - (v)) < FLT_EPSILON)

#define BIT_VALUE(bit) ((unsigned long)1 << (bit))
#define BIT_STATE(A, bit) ((unsigned long)(((A)&BIT_VALUE(bit)) ? 1 : 0))
#define BIT_EXCH(A, bit1, bit2) ((BIT_STATE(A, bit1) << (bit2)) | (BIT_STATE(A, bit2) << bit1))

#define BIT_EXCH_BYTE(A) (BIT_EXCH(A, 0, 7) | BIT_EXCH(A, 1, 6) | BIT_EXCH(A, 2, 5) | BIT_EXCH(A, 3, 4))

#define BIT_EXCH_WORD(A) (BIT_EXCH(A, 0, 15) | BIT_EXCH(A, 1, 14) | BIT_EXCH(A, 2, 13) | BIT_EXCH(A, 3, 12) | BIT_EXCH(A, 4, 11) | BIT_EXCH(A, 5, 10) | BIT_EXCH(A, 6, 9) | BIT_EXCH(A, 7, 8))

#define BIT_EXCH_DWORD(A) (BIT_EXCH(A, 0, 31) | BIT_EXCH(A, 1, 30) | BIT_EXCH(A, 2, 29) | BIT_EXCH(A, 3, 28) | BIT_EXCH(A, 4, 27) | BIT_EXCH(A, 5, 26) | BIT_EXCH(A, 6, 25) | BIT_EXCH(A, 7, 24) | BIT_EXCH(A, 8, 23) | BIT_EXCH(A, 9, 22) | BIT_EXCH(A, 10, 21) | BIT_EXCH(A, 11, 20) | BIT_EXCH(A, 12, 19) | BIT_EXCH(A, 13, 18) | BIT_EXCH(A, 14, 17) | BIT_EXCH(A, 15, 16))
#define BIG_ENDIAN
#if defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)

#define LTOBS(A) BIT_EXCH_WORD(A)
#define LTOBL(A) BIT_EXCH_DWORD(A)

#elif defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#define LTOBS(A) (A)
#define LTOBL(A) (A)

#else
#error "Either BIG_ENDIAN or LITTLE_ENDIAN must be #defined, but not both."
#endif
/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
// typedef struct Ttimer {
//     DWORD __dwTTimer_Tick;
// } Ttimer;
// typedef struct TETimer {
//     DWORD __dwTETimer_Tick;
// } TETimer;
// #define SYSTEMTIME_UPMS g_dwSystemUpTick
// #define ResetTimer(p_dwTimer) ((p_dwTimer)->__dwTTimer_Tick = SYSTEMTIME_UPMS)
// #define SetTimerOut(p_dwTimer) ((p_dwTimer)->__dwTTimer_Tick = SYSTEMTIME_UPMS - ((DWORD)-1 / 2))
// #define CheckTimerOut(p_dwTimer, dwOutTime) ((SYSTEMTIME_UPMS - (p_dwTimer)->__dwTTimer_Tick) >= ((dwOutTime) & ((DWORD)-1 / 2)))
// #define GetTimer(p_dwTimer) (SYSTEMTIME_UPMS - (p_dwTimer)->__dwTTimer_Tick)

// #define ResetETimer(p_lTimer, dwOutTime) ((p_lTimer)->__dwTETimer_Tick = SYSTEMTIME_UPMS + ((dwOutTime) & ((DWORD)-1 / 2))) // Anders 2015-9-24 Modify.
// #define CheckETimerOut(p_lTimer) (SYSTEMTIME_UPMS - (p_lTimer)->__dwTETimer_Tick < ((DWORD)-1 / 2))                         // true: Over; false: waiting.

extern volatile DWORD g_dwSystemUpTick;

#if 1
typedef struct Ttimer {
    DWORD tick;
} Ttimer;

static inline void ResetTimer(Ttimer* timer, DWORD time)
{
    timer->tick = g_dwSystemUpTick - 1 + time;
}
static inline TBOOL CheckTimerOut(Ttimer* timer)
{
    if (timer->tick - g_dwSystemUpTick >= INT32_MAX) {
        timer->tick = g_dwSystemUpTick - 1; // keep time out.
        return TRUE;
    }
    return FALSE;
}
static inline DWORD GetTimer(Ttimer* timer)
{
    return timer->tick - g_dwSystemUpTick;
}
#else
typedef struct Ttimer {
    DWORD tick;
    DWORD time;
} Ttimer;

static inline void ResetTimer(Ttimer* timer, DWORD time)
{
    timer->tick = g_dwSystemUpTick;
    timer->time = time;
}
static inline TBOOL CheckTimerOut(Ttimer* timer)
{
    if (g_dwSystemUpTick - timer->tick >= timer->time) {
        timer->tick = g_dwSystemUpTick - timer->time; // keep time out.
        return TRUE;
    }
    return FALSE;
}
static inline DWORD GetTimer(Ttimer* timer)
{
    return g_dwSystemUpTick - timer->tick;
}
#endif

typedef struct TtimerEx {
    DWORD tick;
} TtimerEx;

static inline void ResetTimerEx(TtimerEx* timer, DWORD time)
{
    timer->tick = g_dwSystemUpTick - time;
}
static inline TBOOL CheckTimerOutEx(TtimerEx* timer, DWORD time)
{
    if (g_dwSystemUpTick - timer->tick >= time) {
        timer->tick = g_dwSystemUpTick - time; // keep time out.
        return TRUE;
    }
    return FALSE;
}
static inline DWORD GetTimerEx(TtimerEx* timer)
{
    return g_dwSystemUpTick - timer->tick;
}

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
