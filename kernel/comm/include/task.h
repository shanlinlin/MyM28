/*==========================================================================+
|  Class    : Control                                                       |
|             Motion control                                                |
|  Task     : Motion control routine                                        |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   :                                                               |
|  Version  : V1.00                                                         |
|  Creation : zholy                                                         |
|  Revision :                                                               |
+==========================================================================*/

#ifndef D__TASK
#define D__TASK

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define MAX_TIMERTASK 10
#define MAX_WHILETASK 10

#define TIMESLICE_1 1
#define TIMESLICE_2 2
#define TIMESLICE_3 3
#define TIMESLICE_4 4
#define TIMESLICE_5 5

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef void (*TASK)();

typedef struct tagTCB_WHILELOOP {
    WORD  wCycleCount;
    WORD  wExecCount;
    TASK  pTask;
    void* pParam;
} TCB_WHILELOOP;

typedef struct tagTCB_TIMERINT {
    WORD  wCycleTimer;
    WORD  wExecTimer;
    TASK  pTask;
    void* pParam;
} TCB_TIMERINT;

typedef struct tagTCB_UNTIMERINT {
    long   lCycleCount;
    Ttimer lCounter;
    TASK   pTask;
    void*  pParam;
    void (*DeleteUpCounter)(Ttimer* ptCounter);
} TCB_UNTIMERINT;

typedef struct tagTCB_CONFIG {
    WORD  wWhileloopCount;
    WORD  wTimerintCount;
    WORD  wUnTimerintCount;
    DWORD dwDestroyWhileloopFlag;
    DWORD dwDestroyTimerintFlag;
    DWORD dwDestroyUnTimerintFlag;
} TCB_CONFIG;

/*===========================================================================+
|           External                                                         |
+===========================================================================*/
void Create_Task(volatile DWORD const* pdwSysDnTimer_MS);
void Destroy_Task();

int Create_WhileloopTask(WORD wCycleCount, TASK pTask, void* pParam);
int Create_TimerintTask(WORD wCycleTimer, TASK pTask, void* pParam);
int Create_UnTimerintTask(long lCycleCount, TASK pTask, void* pParam); // , void (*AddUpCounter)(long* ptCounter));

void Run_WhileloopTask();
void Run_TimerintTask();
void Run_UnTimerintTask();

int Destroy_WhileloopTask(TASK pTask, void* pParam);
int Destroy_TimerintTask(TASK pTask, void* pParam);
int Destroy_UnTimerintTask(TASK pTask, void* pParam); //, void (*DeleteUpCounter)(long* ptCounter));

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
