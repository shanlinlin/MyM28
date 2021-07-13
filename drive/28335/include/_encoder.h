//###########################################################################
//
// FILE:	Encodr_KEB.h
//
// TITLE:	DSP281x CPU Guadrature Encoder CAP_A .
//
// NOTES:   KEB interface card 
//         
//		CAPA_1 -- QEPA ( phase A )
//         	CAPA_2 -- QEPB (Phase B )
//          	CAPA_3 -- QEPI (Index ) 
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  1.00| 03 AUG 2007 | c.W. | Changes from previous version ()
//      |             |      | 
//      |             |      | 
//###########################################################################


#ifndef	D__ENCODER
#define	D__ENCODER

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
#define QEP_CAP_INIT_STATE		0x9004
#define	ENC_RESO				4096				// Encoder resolution (increment)
#define	ENC_DELTA			    (ENC_RESO>>1)		// Calculate encoder delta

/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
typedef struct tagQEP_REG
{
	Uint16		Resolution;
	Uint16		InitCounter;
	Uint16		Counter;
	Uint16		SetCounter;
	void		(*init)();
	void		(*read)();
	void		(*write)();
} QEP_REG;

typedef QEP_REG *QEP_REG_HANDER;

// 4 * 2500 (encoder resvolution)
#define	QEPAREG_DEFAULTS {	(ENC_RESO-1),	\
							0,			\
							0,			\
							0,			\
							(void (*)(Uint32))InitQEPA,	\
							(void (*)(Uint32))ReadQEPA,	\
							(void (*)(Uint32))WriteQEPA	\
						 }

// 4 * 2500 (encoder resvolution)
#define	QEPBREG_DEFAULTS {	(ENC_RESO-1),	\
							0,			\
							0,			\
							0,			\
							(void (*)(Uint32))InitQEPB,	\
							(void (*)(Uint32))ReadQEPB,	\
							(void (*)(Uint32))WriteQEPB	\
						 }

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
void InitQEPA(QEP_REG_HANDER pQEP);
void InitQEPB(QEP_REG_HANDER pQEP);
void ReadQEPA(QEP_REG_HANDER pQEP);
void ReadQEPB(QEP_REG_HANDER pQEP);
void WriteQEPA(QEP_REG_HANDER pQEP);
void WriteQEPB(QEP_REG_HANDER pQEP);

/*===========================================================================+
|           External                                                         |
+===========================================================================*/

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
