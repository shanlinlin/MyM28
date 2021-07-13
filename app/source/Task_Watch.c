/*==============================================================================+
|  Function : Watch                                                             |
|  Task     : Watch Source File                                                 |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2015/08/25                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#include "Task_Watch.h"
#include "tasknet_master.h"
#include "task_ethercatmaster.h"
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
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
S_Watch    g_asWatch[TASK_WATCH_SUM];
S_OldWatch g_sOldWatch;

WORD u_wWatch_index;
//------------------------------------------------------------------------------
S_Watch_IndexSeg const gc_asWatchIndex[] = {
	//{ tWatchTypeIndex,                            tLastIndex,                                         }
	{ DBINDEX_M28_WATCH1_TYPE, DBINDEX_M28_WATCH1_READ_VALUE6 },
	{ DBINDEX_M28_WATCH2_TYPE, DBINDEX_M28_WATCH2_READ_VALUE6 },
	{ DBINDEX_M28_WATCH3_TYPE, DBINDEX_M28_WATCH3_READ_VALUE6 },
	{ DBINDEX_M28_WATCH4_TYPE, DBINDEX_M28_WATCH4_READ_VALUE6 },
	{ DBINDEX_M28_WATCH5_TYPE, DBINDEX_M28_WATCH5_READ_VALUE6 },
	{ DBINDEX_M28_WATCH6_TYPE, DBINDEX_M28_WATCH6_READ_VALUE6 },
	{ DBINDEX_M28_WATCH7_TYPE, DBINDEX_M28_WATCH7_READ_VALUE6 },
	{ DBINDEX_M28_WATCH8_TYPE, DBINDEX_M28_WATCH8_READ_VALUE6 },
};

S_Watch_IndexSeg const gc_sOldWatchIndex = {
	DBINDEX_M28_OLD_WATCH_TYPE,
	DBINDEX_M28_OLD_WATCH_READ_VALUE47,
};
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
/**
 * [Init_Task_Watch description]
 */
void Init_Task_Watch(void)
{
	WORD i;

	DEBUG_ASSERT(TASK_WATCH_SUM == (sizeof(gc_asWatchIndex) / sizeof(*gc_asWatchIndex)));
	u_wWatch_index = 0;
	for (i = 0; i < TASK_WATCH_SUM; i++) {
		Watch_Init(&g_asWatch[i]);
		g_asWatch[i].wWatchTypeIndex = gc_asWatchIndex[i].tWatchTypeIndex;
	}
	Watch_Init(&g_sOldWatch.sWatch);
	Watch_Init_Free(&g_sOldWatch.sWatch, OLDWATCH_FLAGSUM_WRITE, OLDWATCH_FLAGSUM_READ);
	g_sOldWatch.sWatch.wWatchTypeIndex = gc_sOldWatchIndex.tWatchTypeIndex;
	memset(&g_sOldWatch.awAccessFlag, 0, sizeof(g_sOldWatch.awAccessFlag));
}

/**
 * [Run_Task_Watch description]
 */
void Run_Task_Watch(void)
{
	WORD i;

	// for (i = 0; i < TASK_WATCH_SUM; i++)
	// 	Watch_Run(&g_asWatch[i]);
	// Watch_Run(&g_sOldWatch.sWatch);
	for (i = 0; i < TASK_WATCH_SUM; i++) {
		if (u_wWatch_index < TASK_WATCH_SUM) {
			Watch_Run(&g_asWatch[u_wWatch_index]);
			++u_wWatch_index;
		} else {
			Watch_Run(&g_sOldWatch.sWatch);
			u_wWatch_index = 0;
		}
	}

//	{
//#if (DEVICEHANDLE_MASTER >= MAX_NETDEVICE)
//#error "DEVICEHANDLE_MASTER" is a wrong value.
//#endif
//		DB_SetDWORD(DBINDEX_M28_DIAGNOSE_COMM_NET_RX_DATAGRAMS_COUNT, g_dwRxNetDatagramCount[DEVICEHANDLE_MASTER]);
//		DB_SetDWORD(DBINDEX_M28_DIAGNOSE_COMM_NET_TX_DATAGRAMS_COUNT, g_dwTxNetDatagramCount[DEVICEHANDLE_MASTER]);
//	}
	g_sDB.M28.MONITOR.ETHERCAT_MASTER_STATE = g_ECController.Master.wECState; // WriteDB_WORD(DBINDEX_M28_ETHERCAT_MASTER_STATE, ); // Anders 2016-7-27, add.
}
