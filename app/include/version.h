/*==============================================================================+
|  Function : version                                                           |
|  Task     : version Header File                                               |
|-------------------------------------------------------------------------------|
|  Compile  :                                                       |
|  Link     :                                                       |
|  Call     :                                                                   |
|-------------------------------------------------------------------------------|
|  Author   : Wain.Wei                                                          |
|  Version  : V1.00                                                             |
|  Creation : 2011/11/06                                                        |
|  Revision :                                                                   |
+==============================================================================*/
#ifndef D__VERSION
#define D__VERSION

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================+
|           Constant                                                            |
+==============================================================================*/

/*==============================================================================+
|           Type definition                                                     |
+==============================================================================*/
/*==============================================================================+
|           Macro definition                                                    |
+==============================================================================*/
#define VERSIONTYPE_ALPHA 0
#define VERSIONTYPE_BETA 1
#define VERSIONTYPE_STANDARD 2
#define VERSIONTYPE_PERSONAL 3
//==================
// version
//==================
#define VERSION_MAJOR 3                           // System version. V1.1
#define VERSION_MINOR (10 + VERSIONTYPE_STANDARD) // Sub version + Version type.
#define VERSION_SERVICEPACK 1                     // Service pack.
#define VERSION_PATCH 0                           // Patch.

#define VERSION_DATE 0x20210712

#define VERSION_SOFTWARE (VERSION_MAJOR * 1000000L + VERSION_MINOR * 10000L + VERSION_SERVICEPACK * 100L + VERSION_PATCH)
#define VERSION_SOFTWARE_HEX ((VERSION_DEC2HEX((DWORD) VERSION_MAJOR) << 24) + (VERSION_DEC2HEX((DWORD) VERSION_MINOR) << 16) + (VERSION_DEC2HEX((DWORD) VERSION_SERVICEPACK) << 8) + VERSION_DEC2HEX(VERSION_PATCH))
//==================
// version datatime
//==================
// THJ 20170921 modify.
// A10,B11,C12,D13,E14,F15,G16,H17,I18,J19,K20,L21,M22,N23,O24,P25,Q26,R27,S28,T29,U30,V31
#define VERSIONCHAR "0123456789ABCDEFGHIJKLMNOPQRSTUV"
#define VERSION_YEAR_CH1 VERSIONCHAR[VERSION_HEX2DEC((VERSION_DATE >> 20) & 0xF)]
#define VERSION_YEAR_CH2 VERSIONCHAR[VERSION_HEX2DEC((VERSION_DATE >> 16) & 0xF)]
#define VERSION_MONTH VERSIONCHAR[VERSION_HEX2DEC((VERSION_DATE >> 8) & 0xFF)]
#define VERSION_DAY VERSIONCHAR[VERSION_HEX2DEC((VERSION_DATE >> 0) & 0xFF)]

#define VERSION_DATETIME (((DWORD) VERSION_YEAR_CH1 << 24) + ((DWORD) VERSION_YEAR_CH2 << 16) + ((DWORD) VERSION_MONTH << 8) + VERSION_DAY)
#define VERSION_DATETIME_HEX VERSION_DATE

#define VERSION_DEC2HEX(dec) ((((dec) / 10) << 4) + ((dec) % 10))
#define VERSION_HEX2DEC(hex) ((((hex) / 16) * 10) + ((hex) &0xF))

#ifdef __cplusplus
}
#endif

#endif
