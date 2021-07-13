/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : Hankin                                                           |
|  Version  : V1.00                                                            |
|  Creation : 2017/05/10                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#ifndef CommComp_turntable__
#define CommComp_turntable__
//------------------------------------------------------------------------------
#include "../include/CommComp_CANOpenMaster.h"
#include "CommComp_Factory.h"
#include <stdbool.h>
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
enum CommComp_turntable_e_index {
	CommComp_turntable_INDEX_0,
	CommComp_turntable_INDEX_1,

	CommComp_turntable_INDEX_SUM
};
//##############################################################################
// data type
//##############################################################################
//##############################################################################
// function declaration
//##############################################################################
void  CommComp_turntable_init(void);
void  CommComp_turntable_clean(void);
bool  CommComp_turntable_add_hydraulic(WORD wRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
bool  CommComp_turntable_add_HT(WORD wRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
bool  CommComp_turntable_add_electric(WORD wRemoteNum, E_CommType eComm_type, WORD iNode_ID, WORD iComm_cycle);
void  CommComp_turntable_set_PF_channel(enum CommComp_turntable_e_index eIndex, unsigned iPF_channel);
WORD  CommComp_turntable_get_awRsvd(enum CommComp_turntable_e_index eIndex);
WORD  CommComp_turntable_get_error(enum CommComp_turntable_e_index eIndex);
LONG  CommComp_turntable_get_act_position_1(enum CommComp_turntable_e_index eIndex);
WORD  CommComp_turntable_get_act_speed_1(enum CommComp_turntable_e_index eIndex);
WORD  CommComp_turntable_get_act_voltag(enum CommComp_turntable_e_index eIndex);
//##############################################################################
// data declaration
//##############################################################################
//##############################################################################
// function prototype
//##############################################################################
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
