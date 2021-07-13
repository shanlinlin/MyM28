/*=============================================================================+
|  Function :                                                                  |
|  Task     :                                                                  |
|------------------------------------------------------------------------------|
|  Compile  :                                                                  |
|  Link     :                                                                  |
|  Call     :                                                                  |
|------------------------------------------------------------------------------|
|  Author   : THJ                                                              |
|  Version  : V1.00                                                            |
|  Creation : 2016/03/07                                                       |
|  Revision :                                                                  |
+=============================================================================*/
#ifndef D__COMPLED__H
#define D__COMPLED__H
//------------------------------------------------------------------------------
#include "CompPulse.h"
#include "common.h"
//------------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//##############################################################################
//      Definition
//##############################################################################
//##############################################################################
//      Enumeration
//##############################################################################
//##############################################################################
//      Type Definition
//##############################################################################
typedef void (*F_LedOn)(DWORD dwLedId);
typedef void (*F_LedOff)(DWORD dwLedId);

typedef struct Struct_CompLed
{
	S_CompPulse _sShow;
	F_LedOn     _fLedOn;
	F_LedOff    _fLedOff;
	DWORD       dwLedId;
} S_CompLed;
//##############################################################################
//      Function Declaration
//##############################################################################
void        CompLed_Init(S_CompLed * const psLed);
inline void CompLed_Reset(S_CompLed * const psLed);
void        CompLed_Run(S_CompLed * const psLed);
void        CompLed_SetOnOff(S_CompLed * const psLed, F_LedOn fLedOn, F_LedOff fLedOff);
inline void CompLed_SetShow(S_CompLed * const psLed, signed short const assShow[]);
//##############################################################################
//      Data Declaration
//##############################################################################
//##############################################################################
//      Function Prototype
//##############################################################################
/**
 * [CompLed_Reset description]
 * @param psLed [description]
 */
inline void CompLed_Reset(S_CompLed * const psLed)
{
	CompPulse_Reset(&psLed->_sShow);
}

/**
 * [CompLed_SetShow description]
 * @param psLed   [description]
 * @param assShow [description]
 */
inline void CompLed_SetShow(S_CompLed * const psLed, signed short const assShow[])
{
	CompPulse_SetTab(&psLed->_sShow, assShow);
}
//------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//------------------------------------------------------------------------------
#endif
