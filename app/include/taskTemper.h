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
#ifndef D__TASKTEMPER_H
#define	D__TASKTEMPER_H

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Definition                                                       |
+===========================================================================*/

#define     TEMPER_CAL_CYCLETYPE    		1
#define     TEMPER_OUTPUT_CYCLETYPE 		1
#define     TEMPER_CAL_CYCLETYPE_MOLDFAST   1	// Anders 2017-4-27, Add.
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/


/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           External                                                         |
+===========================================================================*/
void	CreatTask_Temper();
void	DestoryTask_Temper();
void    RunTask_Temper();

void	CreatTask_Temper_NormalMold();
void    RunTask_Temper_NormalMold();
void	CreatTask_Temper_FastMold();
void    RunTask_Temper_FastMold();

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
