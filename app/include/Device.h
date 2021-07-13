/*==========================================================================+
|  Function :                                                               |
|  Task     : Version Define                                                |
|---------------------------------------------------------------------------|
|  Compile  :                                                               |
|  Link     :                                                               |
|  Call     :                                                               |
|---------------------------------------------------------------------------|
|  Author   : Anders                                                        |
|  Version  : V1.00                                                         |
|  Creation : 2012/01/12                                                    |
|  Revision : 1.0                                                           |
+==========================================================================*/
#ifndef D__DEVICE_H
#define D__DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
typedef enum {
    APPDEVICE_N2C       = 1,
    APPDEVICE_E2C       = 2, // E2C
    APPDEVICE_REMOTEIO  = 3, //(20140618 THJ(Add: ))
    APPDEVICE_AA_MT_HUB = 4, // Anders 2018-9-26, Note.

    APPDEVICE_HTJ6_OIL     = 5,
    APPDEVICE_HTJ6_ELE_INJ = 6,
    APPDEVICE_HTJ6_PLUS    = 7,

    APPDEVICE_TM_OIL_ROTATION = 10,
    APPDEVICE_HYB5TB          = 11,

    APPDEVICE_HT_EH1708X           = 33,
    APPDEVICE_BTL6_V11E            = 34,
    APPDEVICE_PARKER_COMPAX3_FLUID = 35,
    APPDEVICE_TR_LINEAR            = 36,
    APPDEVICE_HTJ6_PP              = 38,
    APPDEVICE_BTL6_V107            = 39,
    APPDEVICE_REXROTH              = 41,

    APPDEVICE_PLC      = 88,
    APPDEVICE_COMMCHIP = 89, // M3/M28.
    APPDEVICE_MT_HUB   = 90,
    APPDEVICE_MTS      = 91,

    APPDEVICE_AUSTONE = 100,
    APPDEVICE_SIEMENS = 101,
    APPDEVICE_LUST    = 102,
    APPDEVICE_DANAHER = 103,
    APPDEVICE_KEB     = 104,
    APPDEVICE_ESR     = 105,

    APPDEVICE_SIEMENS_250 = 250 // Anders 2013-3-13.
} APPDEVICE;
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/

/*===========================================================================+
|           External                                                         |
+===========================================================================*/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
