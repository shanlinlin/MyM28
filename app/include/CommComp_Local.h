/*==============================================================================+
|  Function : Communication Component Local                                     |
|  Task     : Communication Component Local Header File                         |
|-------------------------------------------------------------------------------|
|  Compile  :                                                                   |
|  Link     :                                                                   |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : THJ                                                               |
|  Version  : V1.00                                                             |
|  Creation : 2014/12/23                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__COMMCOMP_LOCAL__H
#define D__COMMCOMP_LOCAL__H
//-----------------------------------------------------------------------------
#include "CommComp_Factory.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
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
typedef void (*Local_WriteDataByDB_F)(CommComp_S * const psComp, CommSubComp_S * const psSubComp, DBIndex_T tDBIndex);
typedef void (*Local_ReadDataToDB_F)(CommComp_S * const psComp, CommSubComp_S * const psSubComp, DBIndex_T tDBIndex);
//------------------------------------------------------------------------------
typedef struct Struct_LocalSubUnique
{
	Local_WriteDataByDB_F PRO_fWriteDataByDB;
	Local_ReadDataToDB_F  PRO_fReadDataToDB;
} S_LocalSubUnique;
//##############################################################################
//
//      Function Declaration
//
//##############################################################################
void Run_Local(E_CommType eCommType, CommComp_S * const psComp);
//------------------------------------------------------------------------------
void LocalSubUnique_Init(S_LocalSubUnique * const psSubUnique);
void LocalSubUnique_InitWriteDataByDB(S_LocalSubUnique * const psSubUnique, Local_WriteDataByDB_F fWriteDataByDB);
void LocalSubUnique_InitReadDataByDB(S_LocalSubUnique * const psSubUnique, Local_ReadDataToDB_F fReadDataByDB);
//##############################################################################
//
//      Data Declaration
//
//##############################################################################
//##############################################################################
//
//      Function Prototype
//
//##############################################################################
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
