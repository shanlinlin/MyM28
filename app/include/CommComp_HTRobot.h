/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : shanll                                                           |
|  Version  : V1.00                                                            |
|  Creation : 2021/04/26                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#ifndef CommComp_Robot__
#define CommComp_Robot__
//------------------------------------------------------------------------------
#include "../include/CommComp_CANOpenMaster.h"
#include "CommComp_Factory.h"
#include <stdbool.h>
#include "_eCan_28335.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
// definition
//##############################################################################
//##############################################################################
// enumeration
//##############################################################################
enum CommComp_Robot_e_index {
	CommComp_Robot_INDEX_0,
	CommComp_Robot_INDEX_1,

	CommComp_Robot_INDEX_SUM
};
//##############################################################################
// data type
//##############################################################################
typedef union uOpenmoldState
{
	BYTE	All;
	struct
	{
		BYTE bOpenparaChanged : 1;  // Bit0:
		BYTE bOpenActing : 1;  		// Bit1:
		BYTE bRsvd : 6;  			// Bit2-Bit7:Rsvd
	} bit;
}uOpenmoldState;
typedef union uEjetState
{
	BYTE	All;
	struct
	{
		BYTE bEjetparaChanged : 1;  // Bit0:
		BYTE bEjetActing : 1;  		// Bit1:
		BYTE bRsvd : 6;  			// Bit2-Bit7:Rsvd
	} bit;
}uEjetState;
//##############################################################################
// function declaration
//##############################################################################
void  CommComp_Robot_init(void);
void  CommComp_Robot_clean(void);
bool  CommComp_Robot_add_HT(WORD wRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
//##############################################################################
// data declaration
//##############################################################################
extern	WORD			g_wMBEjetPosition;
extern	WORD			g_wMBMoldPosition;
extern	WORD			g_wMBOpenendPosition;
extern	uOpenmoldState	g_byMBOpenmoldState;
extern	uEjetState		g_byMBEjetState;
//##############################################################################
// function prototype
//##############################################################################
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
