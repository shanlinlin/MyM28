/*==============================================================================+
|  Function : powermeter card                                                   |
|  Task     : powermeter card Header File                                       |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2014/08/01                                                        |
|  Revision : 2014/12/25                                                        |
+==============================================================================*/
#ifndef D__COMMCOMP_PWMTRCARDNUM__H
#define D__COMMCOMP_PWMTRCARDNUM__H
//------------------------------------------------------------------------------
#include "CommComp_CANOpenMaster.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//
//      Definition
//
//##############################################################################
#define PWMTRCARDNUM_DEFAULT_COMMTYPE COMMTYPE_CAN2
//##############################################################################
//
//      Enumeration
//
//##############################################################################
typedef enum Enum_PwmtrCardNum {
	PWMTRCARDNUM_NO1,
	PWMTRCARDNUM_NO2,
	PWMTRCARDNUM_NO3,

	PWMTRCARDNUM_SUM,
	PWMTRCARDNUM_MAX = PWMTRCARDNUM_SUM - 1
} E_PwmtrCardNum;
//------------------------------------------------------------------------------
#if (DEBUG)
#define PwmtrCardNum_E E_PwmtrCardNum

#else
#define PwmtrCardNum_E WORD
#endif
//##############################################################################
//
//      Type Definition
//
//##############################################################################
typedef struct
{
	CommSubComp_S          PRI_sSubComp;
	CANOpenSubCompUnique_S PRI_sCANOpenUnique;
} CommPwmtrCardInfo_S;

typedef struct
{
	CommComp_S          PRI_sComp;
	CANOpenCompUnique_S PRI_sCANOpenUnique;

	Ttimer PRI_dwOfflineCtrlTime;
	Ttimer PRI_dwCycleReadTime;
} CommPwmtrCard_S;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void PwmtrCard_Init(void);
//------------------------------------------------------------------------------
void SCommPwmtrCardInfo_Init(CommPwmtrCardInfo_S * psCardInfo);

void SCommPwmtrCard_Init(CommPwmtrCard_S * psCard);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
extern CommPwmtrCard_S g_asCommPwmtrCard[PWMTRCARDNUM_SUM];
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
