/*===========================================================================+
|  Function : IO                                                             |
|  Task     : IO service routine header file                                 |
|----------------------------------------------------------------------------|
|  Compile  : C320 V6.60 -                                                   |
|  Link     : C320 V6.60 -                                                   |
|  Call     :                                                                |
|----------------------------------------------------------------------------|
|  Author   : C.C. Chen                                                      |
|  Version  : V1.00                                                          |
|  Creation : 09/04/1997                                                     |
|  Revision :                                                                |
|============================================================================|
|  Compile  : GCC  V3.2.2-                                                   |
|  Link     : GCC  V3.2.2-                                                   |
|  Call       									                             |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : Alan.Yin                                                       |
|  Version  : V2.30                                                          |
|  Creation : 01/11/2005                                                     |
|  Revision :																 |
+===========================================================================*/

#ifndef D__IO_2
#define D__IO_2

#include "_encoder.h"
#include "common.h"

/*===========================================================================+
|           Constant                                                         |
+===========================================================================*/
//=========
//          IO configuration
//=========
#define MAX_INPORT 1  //Yang 2008/5/16 for 1*16 Input	 8
#define MAX_OUTPORT 4 //Yang 2008/5/9 for 4*16 Output  (8+1)

#ifdef D_HYB5_335

#define MAX_BITIN 2
#define MAX_BITOUT 2

#else

#define MAX_BITIN 16
#define MAX_BITCTR 8
#define MAX_BITOUT 48 //64 //(256+10+10)

#endif

#define MAX_AD1 4  //7	//Yang 2008/5/8 for 7 channel DA 8
#define MAX_AD2 25 //Yang 2008/5/8 for 24 channel Temper (10+1)	//(9+1)//WY2006/3/5
#define MAX_AD (MAX_AD1 + MAX_AD2)
#define MAX_DA 4 //Yang 2008/5/9 for 4 channel DA

#define MAX_ENCODER 2

//=========
//          AD configuration
//=========

#define AD_SAMPLING_SIZE 3 /* It should be an odd number due to the way to find the median of sampling data */
#define AD_FILTER_SIZE 1

#define AD_OK 0
#define AD_ERR_NOTREADY 1
#define AD_ERR_OVERRANGE 2
#define AD_ERR_UNDERRANGE 3
#define AD_NOT_UPDATE 4   //<<WY2006/2/26
#define AD_ERR_POLARITY 5 //wy20070525

#define HIO HANDLE
#define HIO_NULL HANDLE_NULL
#define NULL_HIO HANDLE_NULL
/*===========================================================================+
|           Type definition                                                  |
+===========================================================================*/
enum BITIID {
    BITI00, // Null ID

    BITI01,
    BITI02,
    BITI03,
    BITI04,
    BITI05,
    BITI06,
    BITI07,
    BITI08, // Control input 01 - 08
    BITI09,
    BITI10,
    BITI11,
    BITI12,
    BITI13,
    BITI14,
    BITI15,
    BITI16 // Control input 09 - 16

};

enum BITOID {
    BITO00, // Null ID

    BITO01,
    BITO02,
    BITO03,
    BITO04,
    BITO05,
    BITO06,
    BITO07,
    BITO08, // Control output 01 - 08
    BITO09,
    BITO10,
    BITO11,
    BITO12,
    BITO13,
    BITO14,
    BITO15,
    BITO16, // Control output 09 - 16

    BITO17,
    BITO18,
    BITO19,
    BITO20,
    BITO21,
    BITO22,
    BITO23,
    BITO24, // Control output 17 - 24
    BITO25,
    BITO26,
    BITO27,
    BITO28,
    BITO29,
    BITO30,
    BITO31,
    BITO32, // Control output 25 - 32

    BITO33,
    BITO34,
    BITO35,
    BITO36,
    BITO37,
    BITO38,
    BITO39,
    BITO40, // Control output 33 - 40
    BITO41,
    BITO42,
    BITO43,
    BITO44,
    BITO45,
    BITO46,
    BITO47,
    BITO48, // Control output 41 - 48

    BITO49,
    BITO50,
    BITO51,
    BITO52,
    BITO53,
    BITO54,
    BITO55,
    BITO56, // Control output 49 - 56
    BITO57,
    BITO58,
    BITO59,
    BITO60,
    BITO61,
    BITO62,
    BITO63,
    BITO64 // Control output 57 - 64
};

enum ADID {
    AD100, // Null ID
    AD_TYPE1,
    AD101 = AD_TYPE1, // AD 101 - 108
    AD102,
    AD103,
    AD104,
    AD105,
    AD106,
    AD107,
    AD_TYPE1_END,
    AD_TYPE2 = AD_TYPE1_END,
    AD200    = AD_TYPE2, // AD 200 : Cool Junction
    AD201,               // AD 201 - 210
    AD202,
    AD203,
    AD204,
    AD205,
    AD206,
    AD207,
    AD208,
    AD209,
    AD210,
    AD211,
    AD212,
    AD213,
    AD214,
    AD215,
    AD216,
    AD217,
    AD218,
    AD219,
    AD220,
    AD221,
    AD222,
    AD223,
    AD224,
    AD_TYPE2_END
};

enum DAID {
    DA00, // Null ID

    DA01,
    DA02,
    DA03,
    DA04
};
/***********
|           User interface
***********/
typedef struct tagBITIN {
    WORD wID;
    int  nDebounce;
} BITIN;

typedef struct tagBITCTR {
    WORD wID;
    int  nDebounce;
} BITCTR;

typedef struct tagBITOUT {
    WORD wID;
} BITOUT;

typedef struct tagAD {
    WORD wID;
} AD;

typedef struct tagDA {
    WORD wID;
} DA;

/***********
|
***********/
typedef struct tagAD_QUEUE {
    /*  int     nIndex; */
    int nCount;
    int anSampling[AD_SAMPLING_SIZE];
} AD_QUEUE;

/*===========================================================================+
|           Macro definition                                                 |
+===========================================================================*/
#define GetMin_DA(hio) DA_MIN
#define GetMax_DA(hio) DA_MAX

//Yang 2008/5/8 for I32 driver
#define _InPort(a) (*(volatile WORD*)(a))
#define _InPortB(a) (BYTE)(*(volatile WORD*)(a))

#define _OutPort(a, v) (*(volatile WORD*)(a) = (WORD)(v))
#define _OutPortB(a, v) (*(volatile WORD*)(a) = (BYTE)(v))

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
//Yang 2008/5/9
/*WORD        _InPort  (WORD wPort);
BYTE		_InPortB (WORD wPort);
void        _OutPort (WORD wPort, WORD wValue);
void		_OutPortB(WORD wPort, BYTE bValue);
*/
//void        _OutPort2(WORD wPort, WORD wValue1, WORD wValue2);

void Init_IO();
void SET_ALL_IO();
void SET_ANY_IO(WORD wPort, WORD wValue);
/*===========================================================================+
|           Class declaration - BitIn                                        |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
HIO  Create_BitIn(BITIN* pbitin);
void Destroy_BitIn(HIO hio);

/*---------------------------------------------------------------------------+
|           Attributes                                                       |
+---------------------------------------------------------------------------*/
typedef struct tagCBitIn {
    HIO   m_hioMe;
    BITIN m_bitin;
    int   m_nCount;
    TBOOL m_bOn;    // Last reading value
    TBOOL m_bValue; // Confirmed reading value after debounce
} CBitIn;

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
TBOOL Get_BitIn(HIO hio); // ?U?ohio??input?>?]0TI.<input?????A?D0TI.<input???G?^
WORD  GetAllInput();

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/
/*static*/ void Scan_BitIn();

/*===========================================================================+
|           Class declaration - BitCtr                                       |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
HIO  Create_BitCtr(BITCTR* pbitctr);
void Destroy_BitCtr(HIO hio);

/*---------------------------------------------------------------------------+
|           Attributes                                                       |
+---------------------------------------------------------------------------*/
typedef struct tagCBitCtr {
    HIO    m_hioMe;
    HIO    m_hioBase;
    BITCTR m_bitctr;
    TBOOL  m_bOn;
    WORD   m_wValue;
} CBitCtr;

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
WORD Get_BitCtr(HIO hio);
void Clr_BitCtr(HIO hio);

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/
/*static*/ void Scan_BitCtr();

/*===========================================================================+
|           Class declaration - BitOut                                       |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
HIO  Create_BitOut(BITOUT* pbitout);
void Destroy_BitOut(HIO hio);

/*---------------------------------------------------------------------------+
|           Attributes                                                       |
+---------------------------------------------------------------------------*/
typedef struct tagCBitOut {
    HIO    m_hioMe;
    BITOUT m_bitout;
} CBitOut;

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
void SetValue_BitOut(HIO hio, WORD wValue); // ?]?mhio??output?>??wValue?]0CO1?^
void Set_BitOut(HIO hio);                   // ?]?mhio??output?>??1
void Clr_BitOut(HIO hio);                   // ?]?mhio??output?>??0

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/

/*===========================================================================+
|           Class declaration - AD                                           |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
HIO  Create_AD(AD* pad);
void Destroy_AD(HIO hio);

/*---------------------------------------------------------------------------+
|           Attributes                                                       |
+---------------------------------------------------------------------------*/
typedef struct tagCADSTATIC {
    HIO  m_hio;
    WORD m_wState;
    WORD m_nValue;
    int  m_nDelayCount;

    DWORD m_dwPortCtrl; //Yang 2008/5/8 for Zone6
    DWORD m_dwPortData; //Yang 2008/5/9 for Zone6
    WORD  m_nPortValue;
} CADSTATIC;

typedef struct tagCAD {
    HIO      m_hioMe;
    AD       m_ad;
    AD_QUEUE m_adqueue; /* Reading value before filter          */
    WORD     m_nValue;  /* Confirmed reading value after filter */
    WORD     m_wStatus;
} CAD;

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
WORD Get_AD(HIO hio, WORD* pwStatus); // ?U?ohmo??AD?>
int  GetMin_AD(HIO hio);              // ?U?oAD?w.UTI.<?A?p?>AD1_MIN CO AD2_MIN
int  GetMax_AD(HIO hio);              // ?U?oAD?w.UTI.<?A?j?>AD1_MAX CO AD2_MAX

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/
/*static*/ HIO Create_AD1();
/*atic void Destroy_AD1();*/
/*static*/ void Scan_AD1();
/*static*/ HIO  Create_AD2();
/*atic void Destroy_AD2();*/
/*static*/ void Scan_AD2();

/*static*/ void Filter_AD(HIO hio);

/*===========================================================================+
|           Class declaration - DA                                           |
+===========================================================================*/
/*---------------------------------------------------------------------------+
|           Constructor and destructor                                       |
+---------------------------------------------------------------------------*/
HIO  Create_DA(DA* pda);
void Destroy_DA(HIO hio);

/*---------------------------------------------------------------------------+
|           Attributes                                                       |
+---------------------------------------------------------------------------*/
typedef struct tagCDA {
    HIO m_hioMe;
    DA  m_da;
} CDA;

/*---------------------------------------------------------------------------+
|           Operations                                                       |
+---------------------------------------------------------------------------*/
void Set_DA(HIO hio, WORD wValue); // ?]?mhio??DA?>??wValue

/*---------------------------------------------------------------------------+
|           Helpers                                                          |
+---------------------------------------------------------------------------*/
void Create_Encoder(WORD hio);
WORD Get_Encoder(WORD hio);
int  Set_Encoder(WORD hio, WORD wValue);

#endif
