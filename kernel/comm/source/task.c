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
#include "task.h"
#include "common.h"
#include "version_kernel.h"
#include <string.h>
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
//#define       SYSTEMTIME_UPMS                                 (*u_pdwSysDnTimer_MS)
//#define       ResetTimer(p_dwTimer)                           (*(p_dwTimer) = SYSTEMTIME_UPMS)
//#define       SetTimerOut(p_dwTimer,  dwOutTime)          (*(p_dwTimer) = SYSTEMTIME_UPMS - (dwOutTime))      // 0xFFFFFFFF - dwOutTime > Check Cycle.
//#define       CheckTimerOut(p_dwTimer,    dwOutTime)          ((SYSTEMTIME_UPMS - *(p_dwTimer)) >= (dwOutTime))

/*===========================================================================+
|           Global variable                                                  |
+===========================================================================*/

/*===========================================================================+
|           Local variable                                                   |
+===========================================================================*/
volatile TCB_CONFIG u_TcbConfig = { 0, 0, 0, 0, 0, 0 };
TCB_TIMERINT        u_aTcbTimerintList[MAX_TIMERTASK];
TCB_WHILELOOP       u_aTcbWhileloopList[MAX_WHILETASK];
TCB_UNTIMERINT      u_aTcbUnTimerintList[MAX_TIMERTASK];

//TCB_TIMERINT          u_aTcbTimerintListTemp[MAX_TIMERTASK];
//TCB_WHILELOOP         u_aTcbWhileloopListTemp[MAX_WHILETASK];
//TCB_UNTIMERINT            u_aTcbUnTimerintListTemp[MAX_TIMERTASK];
volatile DWORD const * u_pdwSysDnTimer_MS;
/*===========================================================================+
|           Operation                                                        |
+===========================================================================*/
void Create_Task(volatile DWORD const * pdwSysDnTimer_MS)
{
	u_pdwSysDnTimer_MS = pdwSysDnTimer_MS;
	memset(u_aTcbTimerintList, 0, sizeof(u_aTcbTimerintList));
	memset(u_aTcbWhileloopList, 0, sizeof(u_aTcbWhileloopList));
	memset(u_aTcbUnTimerintList, 0, sizeof(u_aTcbUnTimerintList));

	//memset(u_aTcbTimerintListTemp, 0, sizeof(u_aTcbTimerintListTemp));
	//memset(u_aTcbWhileloopListTemp, 0, sizeof(u_aTcbWhileloopListTemp));
	//memset(u_aTcbUnTimerintListTemp, 0, sizeof(u_aTcbUnTimerintListTemp));

	GetVersion_Kernel();
}

void Destroy_Task()
{
	// ...
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
int Create_WhileloopTask(WORD wCycleCount, TASK pTask, void * pParam)
{
	WORD            i;
	int             j = -1;
	int             nResult;
	TCB_WHILELOOP * pTaskList = (TCB_WHILELOOP *) &u_aTcbWhileloopList;

	if (pTask == NULL)
		return -1;

	DINT;

	for (i = 0; i < u_TcbConfig.wWhileloopCount; i++)
	{
		if ((pTaskList + i)->pTask == pTask && (pTaskList + i)->pParam == pParam)
		{
			j = i;
			break;
		}
	}

	if (j != -1)
	{
		(pTaskList + j)->pTask       = pTask;
		(pTaskList + j)->pParam      = pParam;
		(pTaskList + j)->wCycleCount = wCycleCount;
		(pTaskList + j)->wExecCount  = 0;

		nResult = j;
	}
	else if (u_TcbConfig.wWhileloopCount <= MAX_WHILETASK - 1)
	{
		(pTaskList + u_TcbConfig.wWhileloopCount)->pTask       = pTask;
		(pTaskList + u_TcbConfig.wWhileloopCount)->pParam      = pParam;
		(pTaskList + u_TcbConfig.wWhileloopCount)->wCycleCount = wCycleCount;
		(pTaskList + u_TcbConfig.wWhileloopCount)->wExecCount  = 0;
		u_TcbConfig.wWhileloopCount++;

		nResult = u_TcbConfig.wWhileloopCount - 1;
	}
	else
		nResult = -1;

	EINT;

	return nResult;
}

int Create_TimerintTask(WORD wCycleTimer, TASK pTask, void * pParam)
{
	WORD           i;
	int            j = -1;
	int            nResult;
	TCB_TIMERINT * pTaskList = u_aTcbTimerintList;

	if (wCycleTimer == 0 || pTask == NULL)
		return -1;

	DINT;

	for (i = 0; i < u_TcbConfig.wTimerintCount; i++)
	{
		if ((pTaskList + i)->pTask == pTask && (pTaskList + i)->pParam == pParam)
		{
			j = i;
			break;
		}
	}

	if (j != -1)
	{
		(pTaskList + j)->pTask       = pTask;
		(pTaskList + j)->wCycleTimer = wCycleTimer;
		(pTaskList + j)->wExecTimer  = 0;
		(pTaskList + j)->pParam      = pParam;

		nResult = j;
	}
	else if (u_TcbConfig.wTimerintCount <= MAX_TIMERTASK - 1)
	{
		(pTaskList + u_TcbConfig.wTimerintCount)->pTask       = pTask;
		(pTaskList + u_TcbConfig.wTimerintCount)->wCycleTimer = wCycleTimer;
		(pTaskList + u_TcbConfig.wTimerintCount)->wExecTimer  = 0;
		(pTaskList + u_TcbConfig.wTimerintCount)->pParam      = pParam;
		u_TcbConfig.wTimerintCount++;

		nResult = u_TcbConfig.wTimerintCount - 1;
	}
	else
		nResult = -1;

	EINT;

	return nResult;
}

int Create_UnTimerintTask(long lCycleCount, TASK pTask, void * pParam) //, void (*AddUpCounter)(long* ptCounter))
{
	WORD             i;
	int              j = -1;
	int              nResult;
	TCB_UNTIMERINT * pTaskList = u_aTcbUnTimerintList;

	if (lCycleCount == 0 || pTask == NULL)
		return -1; //|| AddUpCounter == NULL

	DINT;

	for (i = 0; i < u_TcbConfig.wUnTimerintCount; i++)
	{
		if ((pTaskList + i)->pTask == pTask && (pTaskList + i)->pParam == pParam)
		{
			j = i;
			break;
		}
	}

	if (j != -1)
	{
		(pTaskList + j)->pTask       = pTask;
		(pTaskList + j)->pParam      = pParam;
		(pTaskList + j)->lCycleCount = lCycleCount;
		//(pTaskList+j)->lCounter       = 0;
		ResetTimer(&(pTaskList + j)->lCounter, lCycleCount);

		nResult = j;
	}
	else if (u_TcbConfig.wUnTimerintCount <= MAX_TIMERTASK - 1)
	{
		(pTaskList + u_TcbConfig.wUnTimerintCount)->pTask       = pTask;
		(pTaskList + u_TcbConfig.wUnTimerintCount)->pParam      = pParam;
		(pTaskList + u_TcbConfig.wUnTimerintCount)->lCycleCount = lCycleCount;
		//(pTaskList+u_TcbConfig.wUnTimerintCount)->lCounter        = 0;
		//AddUpCounter(&(pTaskList+u_TcbConfig.wUnTimerintCount)->lCounter);
		ResetTimer(&(pTaskList + u_TcbConfig.wUnTimerintCount)->lCounter, lCycleCount);
		u_TcbConfig.wUnTimerintCount++;

		nResult = u_TcbConfig.wUnTimerintCount - 1;
	}
	else
		nResult = -1;

	EINT;

	return nResult;
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
int Destroy_WhileloopTask(TASK pTask, void * pParam)
{
	WORD            i;
	int             j         = -1;
	TCB_WHILELOOP * pTaskList = u_aTcbWhileloopList;

	DINT;

	for (i = 0; i < u_TcbConfig.wWhileloopCount; i++)
	{
		if ((pTaskList + i)->pTask == pTask && (pTaskList + i)->pParam == pParam)
		{
			u_TcbConfig.dwDestroyWhileloopFlag |= ((DWORD) 1 << i);
			j = i;
			break;
		}
	}

	EINT;

	return j;
}

int Destroy_TimerintTask(TASK pTask, void * pParam)
{
	WORD           i;
	int            j         = -1;
	TCB_TIMERINT * pTaskList = u_aTcbTimerintList;

	DINT;

	for (i = 0; i < u_TcbConfig.wTimerintCount; i++)
	{
		if ((pTaskList + i)->pTask == pTask && (pTaskList + i)->pParam == pParam)
		{
			u_TcbConfig.dwDestroyTimerintFlag |= ((DWORD) 1 << i);
			j = i;
			break;
		}
	}

	EINT;

	return j;
}

int Destroy_UnTimerintTask(TASK pTask, void * pParam) //, void (*DeleteUpCounter)(long* ptCounter))
{
	WORD             i;
	int              j         = -1;
	TCB_UNTIMERINT * pTaskList = u_aTcbUnTimerintList;

	DINT;

	for (i = 0; i < u_TcbConfig.wUnTimerintCount; i++)
	{
		if ((pTaskList + i)->pTask == pTask && (pTaskList + i)->pParam == pParam)
		{
			u_TcbConfig.dwDestroyUnTimerintFlag |= ((DWORD) 1 << i);
			//(pTaskList+i)->DeleteUpCounter = DeleteUpCounter;
			j = i;
			break;
		}
	}

	EINT;

	return j;
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
void Run_WhileloopTask()
{
	WORD            i, j;
	TCB_WHILELOOP * pTaskList = u_aTcbWhileloopList;

	for (i = 0; i < u_TcbConfig.wWhileloopCount; i++)
	{
		if ((pTaskList + i)->pTask != NULL) // Anders 2013-5-3
		{
			((pTaskList + i)->pTask)((pTaskList + i)->pParam);
			if ((pTaskList + i)->wCycleCount > 0)
			{
				if (++((pTaskList + i)->wExecCount) >= (pTaskList + i)->wCycleCount)
				{
					u_TcbConfig.dwDestroyWhileloopFlag |= ((DWORD) 1 << i);
				}
			}
		}
	}

	// Delete while loop task

	if (u_TcbConfig.dwDestroyWhileloopFlag > 0) // Anders 2014-8-21, Modified while -> if.
	{
		DINT;
		j = 0;
		//memcpy(u_aTcbWhileloopListTemp, pTaskList, sizeof(TCB_WHILELOOP)*u_TcbConfig.wWhileloopCount);    // Anders 2014-8-21, Mark.
		//memset(pTaskList, 0, sizeof(TCB_WHILELOOP)*u_TcbConfig.wWhileloopCount);                          // Anders 2014-8-21, Mark.
		for (i = 0; i < u_TcbConfig.wWhileloopCount; i++)
		{
			if (u_TcbConfig.dwDestroyWhileloopFlag & ((DWORD) 1 << i))
			{
				u_TcbConfig.dwDestroyWhileloopFlag &= (~((DWORD) 1 << i));
			}
			else
			{
				*(pTaskList + j) = *(pTaskList + i); //u_aTcbWhileloopListTemp[i];                           // Anders 2014-8-21, Modified.
				j++;
			}
		}
		u_TcbConfig.wWhileloopCount = j;
		memset((pTaskList + j), 0, sizeof(TCB_WHILELOOP) * (MAX_WHILETASK - j)); // Anders 2014-8-21, Add.
		EINT;
	}
}

void Run_TimerintTask()
{
	WORD           i, j;
	TCB_TIMERINT * pTaskList = u_aTcbTimerintList;

	for (i = 0; i < u_TcbConfig.wTimerintCount; i++)
	{
		if ((pTaskList + i)->pTask != NULL) // Anders 20120911.
		{
			(pTaskList + i)->wExecTimer++;
			if ((pTaskList + i)->wExecTimer >= (pTaskList + i)->wCycleTimer)
			{
				((pTaskList + i)->pTask)((pTaskList + i)->pParam);
				(pTaskList + i)->wExecTimer = 0;
			}
		}
	}

	// Delete while loop task

	if (u_TcbConfig.dwDestroyTimerintFlag > 0) // Anders 2014-8-21, Modified while -> if.
	{
		DINT;
		j = 0;
		//memcpy(u_aTcbTimerintListTemp, pTaskList, sizeof(TCB_TIMERINT)*u_TcbConfig.wTimerintCount);   // Anders 2014-8-21, Mark.
		//memset(pTaskList, 0, sizeof(TCB_TIMERINT)*u_TcbConfig.wTimerintCount);                        // Anders 2014-8-21, Mark.
		for (i = 0; i < u_TcbConfig.wTimerintCount; i++)
		{
			if (u_TcbConfig.dwDestroyTimerintFlag & ((DWORD) 1 << i))
			{
				u_TcbConfig.dwDestroyTimerintFlag &= (~((DWORD) 1 << i));
			}
			else
			{
				*(pTaskList + j) = *(pTaskList + i); //u_aTcbTimerintListTemp[i];                           // Anders 2014-8-21, Modified.
				j++;
			}
		}
		u_TcbConfig.wTimerintCount = j;
		memset((pTaskList + j), 0, sizeof(TCB_TIMERINT) * (MAX_TIMERTASK - j)); // Anders 2014-8-21, Add.
		EINT;
	}
}

void Run_UnTimerintTask()
{
	WORD             i, j;
	TCB_UNTIMERINT * pTaskList = u_aTcbUnTimerintList;

	for (i = 0; i < u_TcbConfig.wUnTimerintCount; i++)
	{
		if ((pTaskList + i)->pTask != NULL) // Anders 20120911.
		{
			if (CheckTimerOut(&(pTaskList + i)->lCounter))
			{
				((pTaskList + i)->pTask)((pTaskList + i)->pParam);
				//(pTaskList+i)->lCounter = 0;
				ResetTimer(&(pTaskList + i)->lCounter, (pTaskList + i)->lCycleCount);
			}
		}
	}

	// Delete while loop task

	if (u_TcbConfig.dwDestroyUnTimerintFlag > 0) // Anders 2014-8-21, Modified while -> if.
	{
		DINT;
		j = 0;
		//memcpy(u_aTcbUnTimerintListTemp, pTaskList, sizeof(TCB_UNTIMERINT)*u_TcbConfig.wUnTimerintCount);             // Anders 2014-8-21, Mark.
		//memset(pTaskList, 0, sizeof(TCB_UNTIMERINT)*u_TcbConfig.wUnTimerintCount);                                    // Anders 2014-8-21, Mark.
		for (i = 0; i < u_TcbConfig.wUnTimerintCount; i++)
		{
			if (u_TcbConfig.dwDestroyUnTimerintFlag & ((DWORD) 1 << i))
			{
				u_TcbConfig.dwDestroyUnTimerintFlag &= (~((DWORD) 1 << i));
				if ((pTaskList + i)->DeleteUpCounter != NULL) //u_aTcbUnTimerintListTemp[i]                                // Anders 2014-8-21, Modified.
					(pTaskList + i)->DeleteUpCounter(&(pTaskList + i)->lCounter);
			}
			else
			{
				*(pTaskList + j) = *(pTaskList + i); //u_aTcbUnTimerintListTemp[i];                                      // Anders 2014-8-21, Modified.
				j++;
			}
		}
		u_TcbConfig.wUnTimerintCount = j;
		memset(pTaskList + j, 0, sizeof(TCB_UNTIMERINT) * (MAX_TIMERTASK - j)); // Anders 2014-8-21, Add.
		EINT;
	}
}
