/*==============================================================================+
|  Function :                                                                   |
|  Task     :                                                                   |
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
#ifndef D__TASK_WATCH__H
#define D__TASK_WATCH__H
//------------------------------------------------------------------------------
#include "Watch.h"
#include "database.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define OLDWATCH_FLAGSUM_WRITE 1
#define OLDWATCH_FLAGSUM_READ 48
#define OLDWATCH_FLAGSUM ((OLDWATCH_FLAGSUM_WRITE + OLDWATCH_FLAGSUM_READ - WATCH_FLAGSUM + 0xF) >> 4)

#define TASK_WATCH_SUM 8
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
typedef struct Struct_Watch_IndexSeg
{
	DBIndex_T tWatchTypeIndex;
	DBIndex_T tLastIndex;
} S_Watch_IndexSeg;

typedef struct Struct_OldWatch
{
	S_Watch sWatch;
	WORD    awAccessFlag[OLDWATCH_FLAGSUM];
} S_OldWatch;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Init_Task_Watch(void);
void Run_Task_Watch(void);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern S_Watch                g_asWatch[TASK_WATCH_SUM];
extern S_OldWatch             g_sOldWatch;
extern S_Watch_IndexSeg const gc_asWatchIndex[TASK_WATCH_SUM];
extern S_Watch_IndexSeg const gc_sOldWatchIndex;
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
