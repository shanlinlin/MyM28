/*
// TI File $Revision: /main/10 $
// Checkin $Date: July 9, 2008   13:43:56 $
//###########################################################################
//
// FILE:	F28335.cmd
//
// TITLE:	Linker Command File For F28335 Device
//
//###########################################################################
// $TI Release: DSP2833x/DSP2823x C/C++ Header Files V1.31 $
// $Release Date: August 4, 2009 $
//###########################################################################
*/

/* ======================================================
// For Code Composer Studio V2.2 and later
// ---------------------------------------
// In addition to this memory linker command file,
// add the header linker command file directly to the project.
// The header linker command file is required to link the
// peripheral structures to the proper locations within
// the memory map.
//
// The header linker files are found in <base>\DSP2833x_Headers\cmd
//
// For BIOS applications add:      DSP2833x_Headers_BIOS.cmd
// For nonBIOS applications add:   DSP2833x_Headers_nonBIOS.cmd
========================================================= */

/* ======================================================
// For Code Composer Studio prior to V2.2
// --------------------------------------
// 1) Use one of the following -l statements to include the
// header linker command file in the project. The header linker
// file is required to link the peripheral structures to the proper
// locations within the memory map                                    */

/* Uncomment this line to include file only for non-BIOS applications */
/* -l DSP2833x_Headers_nonBIOS.cmd */

/* Uncomment this line to include file only for BIOS applications */
/* -l DSP2833x_Headers_BIOS.cmd */

/* 2) In your project add the path to <base>\DSP2833x_headers\cmd to the
   library search path under project->build options, linker tab,
   library search path (-i).
/*========================================================= */

/* Define the memory block start/length for the F28335
   PAGE 0 will be used to organize program sections
   PAGE 1 will be used to organize data sections

   Notes:
         Memory blocks on F28335 are uniform (ie same
         physical memory) in both PAGE 0 and PAGE 1.
         That is the same memory region should not be
         defined for both PAGE 0 and PAGE 1.
         Doing so will result in corruption of program
         and/or data.

         L0/L1/L2 and L3 memory blocks are mirrored - that is
         they can be accessed in high memory or low memory.
         For simplicity only one instance is used in this
         linker file.

         Contiguous SARAM memory blocks can be combined
         if required to create a larger memory block.
 */


MEMORY
{
PAGE 0:    /* Program Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE1 for data allocation */

   //ZONE0     : origin = 0x004000, length = 0x001000     /* XINTF zone 0 - Net */
   RAML0       : origin = 0x008000, length = 0x000600     /* on-chip RAM block L0 */
   //ZONE6     : origin = 0x100000, length = 0x100000     /* XINTF zone 6 - RGM50 */
   FLASH	     : origin = 0x300000, length = 0x02FD00       /* on-chip FLASH for normal */
   SYSTEM_INFO : origin = 0x32FD00, length = 0x000200

   /*CODE_START1 : origin = 0x33FF7E, length = 0x000002*/
   CODE_START1 : origin = 0x32FFFE, length = 0x000002
   //FLASH_AB	 : origin = 0x330000, length = 0x00FF00     /* on-chip FLASH for boot */
   //BOOT_START  : origin = 0x33FF7E, length = 0x000002
   CSM_RSVD    : origin = 0x33FF80, length = 0x000076     /* Part of FLASH_AB.  Program with all 0x0000 when CSM is in use. */
   BEGIN       : origin = 0x33FFF6, length = 0x000002     /* Part of FLASH_AB.  Used for "boot to Flash" bootloader mode. */

   OTP         : origin = 0x380400, length = 0x000400     /* on-chip OTP */
   ADC_CAL     : origin = 0x380080, length = 0x000009     /* ADC_cal function in Reserved memory */
   /* IQTABLES and IQTABLES2 are part of the boot ROM.
      The boot ROM is available in both program or
      data space so this can be defined on page 0 or page 1
   */


   IQTABLES    : origin = 0x3FE000, length = 0x000b50     /* IQ Math Tables in Boot ROM */
   IQTABLES2   : origin = 0x3FEB50, length = 0x00008c     /* IQ Math Tables in Boot ROM */
   FPUTABLES   : origin = 0x3FEBDC, length = 0x0006A0     /* FPU Tables in Boot ROM */
   /* ROM         : origin = 0x3FF27C, length = 0x000D44 */     /* Boot ROM */
   RESET       : origin = 0x3FFFC0, length = 0x000002     /* part of boot ROM  */
   VECTORS     : origin = 0x3FFFC2, length = 0x00003E     /* part of boot ROM  */

//PAGE 1 :   /* Data Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE0 for program allocation */
           /* Registers remain on PAGE1                                                  */

   /*BOOT_RSVD   : origin = 0x000000, length = 0x000050 */    /* Part of M0, BOOT rom will use this for stack */
   RAMM0M1     : origin = 0x000000, length = 0x000800     /* on-chip RAM block M0M1 */
   RAML1L7     : origin = 0x008600, length = 0x005800     /* on-chip RAM block L1 */
   RAML1L7_1   : origin = 0x00DE00, length = 0x000800     /* on-chip RAM block L1 MC_DB */
   RAML1L7_2   : origin = 0x00E600, length = 0x001A00     /* on-chip RAM block L1 NET_DATA */

   ZONE7         : origin = 0x200000, length = 0x040000     /* XINTF zone 7 - 256KW */
   //ZONE7B1     : origin = 0x221000, length = 0x001200     /* 256KW XINTF Zone 7 MCDatabase */
   //ZONE7B2     : origin = 0x222200, length = 0x000600     /* 256KW XINTF Zone 7 CANDatabase */
   //ZONE7B3     : origin = 0x222800, length = 0x000800     /* 256KW XINTF Zone 7 MotorDriveParam */
   //ZONE7C      : origin = 0x223000, length = 0x005000     /* 256KW XINTF Zone 7 Database */
   //ZONE7D      : origin = 0x228000, length = 0x001300     /* 256KW XINTF Zone 7 TemperCtl */
   //ZONE7E      : origin = 0x229300, length = 0x001500     /* 256KW XINTF Zone 7 EthernetBuffer */
   //ZONE7F      : origin = 0x22B800, length = 0x002100     /* 256KW XINTF Zone 7 MotionControl*/
   //ZONE7G      : origin = 0x22D900, length = 0x000300     /* 256KW XINTF Zone 7 AxisManager*/
   //ZONE7H      : origin = 0x22DC00, length = 0x012400     /* 256KW XINTF Zone 7 ECMaster*/

   DEV_EMU     : origin = 0x000880, length = 0x000180     /* device emulation registers */
   FLASH_REGS  : origin = 0x000A80, length = 0x000060     /* FLASH registers */
   CSM         : origin = 0x000AE0, length = 0x000010     /* code security module registers */

   ADC_MIRROR  : origin = 0x000B00, length = 0x000010     /* ADC Results register mirror */

   XINTF       : origin = 0x000B20, length = 0x000020     /* external interface registers */

   CPU_TIMER0  : origin = 0x000C00, length = 0x000008     /* CPU Timer0 registers */
   CPU_TIMER1  : origin = 0x000C08, length = 0x000008     /* CPU Timer0 registers (CPU Timer1 & Timer2 reserved TI use)*/
   CPU_TIMER2  : origin = 0x000C10, length = 0x000008     /* CPU Timer0 registers (CPU Timer1 & Timer2 reserved TI use)*/

   PIE_CTRL    : origin = 0x000CE0, length = 0x000020     /* PIE control registers */
   PIE_VECT    : origin = 0x000D00, length = 0x000100     /* PIE Vector Table */

   DMA         : origin = 0x001000, length = 0x000200     /* DMA registers */

   MCBSPA      : origin = 0x005000, length = 0x000040     /* McBSP-A registers */
   MCBSPB      : origin = 0x005040, length = 0x000040     /* McBSP-B registers */

   ECANA       : origin = 0x006000, length = 0x000040     /* eCAN-A control and status registers */
   ECANA_LAM   : origin = 0x006040, length = 0x000040     /* eCAN-A local acceptance masks */
   ECANA_MOTS  : origin = 0x006080, length = 0x000040     /* eCAN-A message object time stamps */
   ECANA_MOTO  : origin = 0x0060C0, length = 0x000040     /* eCAN-A object time-out registers */
   ECANA_MBOX  : origin = 0x006100, length = 0x000100     /* eCAN-A mailboxes */

   ECANB       : origin = 0x006200, length = 0x000040     /* eCAN-B control and status registers */
   ECANB_LAM   : origin = 0x006240, length = 0x000040     /* eCAN-B local acceptance masks */
   ECANB_MOTS  : origin = 0x006280, length = 0x000040     /* eCAN-B message object time stamps */
   ECANB_MOTO  : origin = 0x0062C0, length = 0x000040     /* eCAN-B object time-out registers */
   ECANB_MBOX  : origin = 0x006300, length = 0x000100     /* eCAN-B mailboxes */

   EPWM1       : origin = 0x006800, length = 0x000022     /* Enhanced PWM 1 registers */
   EPWM2       : origin = 0x006840, length = 0x000022     /* Enhanced PWM 2 registers */
   EPWM3       : origin = 0x006880, length = 0x000022     /* Enhanced PWM 3 registers */
   EPWM4       : origin = 0x0068C0, length = 0x000022     /* Enhanced PWM 4 registers */
   EPWM5       : origin = 0x006900, length = 0x000022     /* Enhanced PWM 5 registers */
   EPWM6       : origin = 0x006940, length = 0x000022     /* Enhanced PWM 6 registers */

   ECAP1       : origin = 0x006A00, length = 0x000020     /* Enhanced Capture 1 registers */
   ECAP2       : origin = 0x006A20, length = 0x000020     /* Enhanced Capture 2 registers */
   ECAP3       : origin = 0x006A40, length = 0x000020     /* Enhanced Capture 3 registers */
   ECAP4       : origin = 0x006A60, length = 0x000020     /* Enhanced Capture 4 registers */
   ECAP5       : origin = 0x006A80, length = 0x000020     /* Enhanced Capture 5 registers */
   ECAP6       : origin = 0x006AA0, length = 0x000020     /* Enhanced Capture 6 registers */

   EQEP1       : origin = 0x006B00, length = 0x000040     /* Enhanced QEP 1 registers */
   EQEP2       : origin = 0x006B40, length = 0x000040     /* Enhanced QEP 2 registers */

   GPIOCTRL    : origin = 0x006F80, length = 0x000040     /* GPIO control registers */
   GPIODAT     : origin = 0x006FC0, length = 0x000020     /* GPIO data registers */
   GPIOINT     : origin = 0x006FE0, length = 0x000020     /* GPIO interrupt/LPM registers */

   SYSTEM      : origin = 0x007010, length = 0x000020     /* System control registers */
   SPIA        : origin = 0x007040, length = 0x000010     /* SPI-A registers */
   SCIA        : origin = 0x007050, length = 0x000010     /* SCI-A registers */
   XINTRUPT    : origin = 0x007070, length = 0x000010     /* external interrupt registers */
   ADC         : origin = 0x007100, length = 0x000020     /* ADC registers */
   SCIB        : origin = 0x007750, length = 0x000010     /* SCI-B registers */
   SCIC        : origin = 0x007770, length = 0x000010     /* SCI-C registers */
   I2CA        : origin = 0x007900, length = 0x000040     /* I2C-A registers */
   CSM_PWL     : origin = 0x33FFF8, length = 0x000008     /* Part of FLASH_AB.  CSM password locations. */
   PARTID      : origin = 0x380090, length = 0x000001     /* Part ID register location */

PAGE 1:
   CSM_PWL     : origin = 0x33FFF8, length = 0x000008     /* Part of FLASH_AB.  CSM password locations. */
}

/* Allocate sections to memory blocks.
   Note:
         codestart user defined section in DSP28_CodeStartBranch.asm used to redirect code
                  execution when booting to flash
         ramfuncs  user defined section to store functions that will be copied from Flash into RAM
*/

SECTIONS
{

   /* Allocate program areas: */
   .cinit              : > FLASH,      PAGE = 0
   .pinit              : >> FLASH,      PAGE = 0
   .text               : >> FLASH,      PAGE = 0

   ramfuncs            : LOAD = FLASH,
                        RUN = RAML0,
                        LOAD_START(_RamFuncs_loadstart),
                        LOAD_END(_RamFuncs_loadend),
                        RUN_START(_RamFuncs_runstart),
                        PAGE = 0

   codestart           : > 0x33FFF6,    PAGE = 0
   codestart1          : > 0x32FFFE,    PAGE = 0

   csmpasswds          : > 0x33FFF8,    PAGE = 0
   /*passwords		     : > 0x33FFF8,    PAGE = 0*/
   csm_rsvd            : > 0x33FF80,    PAGE = 0

   system              : > 0x32FD00,     PAGE = 0
   /* Allocate uninitalized data sections: */
   .stack              : > 0x000000,    PAGE = 0
   .ebss               : > RAML1L7,     PAGE = 0
   .esysmem            : > RAML1L7,     PAGE = 0
   .cio                : > RAML1L7_1,     PAGE = 0

   /* Initalized sections go in Flash */
   /* For SDFlash to program these, they must be allocated to page 0 */
   .econst             : >> FLASH,       PAGE = 0
   .switch             : >> FLASH,       PAGE = 0

   /* Allocate IQ math areas: */
   /*IQmath              : > FLASH,       PAGE = 0  */                /* Math Code */
   IQmath               : LOAD = FLASH,
                        RUN = RAML0,
                        LOAD_START(_RamFuncs_loadstart1),
                        LOAD_END(_RamFuncs_loadend1),
                        RUN_START(_RamFuncs_runstart1),
                        PAGE = 0
   IQmathTables        : > IQTABLES,  PAGE = 0, TYPE = NOLOAD
   /*IQmathTables        : LOAD = IQTABLES,
                        RUN = RAML0,
                        LOAD_START(_IQTableRamFuncs_loadstart1),
                        LOAD_END(_IQTableRamFuncs__loadend1),
                        RUN_START(_IQTableRamFuncs__runstart1),
                        PAGE = 0*/

   /* IQmath inclues the assembly routines in the IQmath library
      IQmathTables is used by division, IQsin, IQcos, IQatan, IQatan2
                  this is in boot ROM so we make it NOLOAD.  Using
                  the ROM version saves space at the cost of 1 cycle
                  per access (boot ROM is 1 wait).
      IQmathTablesRam is used by IQasin, IQacos, and IQexp
                  on 2833x and 2823x the IQNexpTable is in ROM so it
                  is placed in its own section.  If IQexp or IQNexp is
                  not called by the program, this will cause a linker
                  warning.
   IQmathTables2    : > 0x3FEB50, PAGE = 0, TYPE = NOLOAD
   {
      IQmath_fpu32.lib<IQNexpTable.obj> (IQmathTablesRam)
   }
   IQmathTablesRam  : > RAML4,     PAGE = 0  */
   FPUmathTables    : > 0x3FEBDC, PAGE = 0, TYPE = NOLOAD

   /* Allocate DMA-accessible RAM sections: */

   heap            : > RAML1L7_1
                     , START(_heap_start)
                     , SIZE(_heap_size)
                     , PAGE = 0

   /* Allocate 0x400 of XINTF Zone 7 to storing data */
   MCDatabase       : > 0x00DE00,  PAGE = 0 /* ZONE7B1 */
   EthernetBuffer   : > 0x00E600,  PAGE = 0 /*ZONE7E*/
   CANDatabase      : > 0x200000,  PAGE = 0
   MotorDriveParam  : > 0x202800,  PAGE = 0
   Database         : > 0x203000,  PAGE = 0
   TemperCtl        : > 0x208000,  PAGE = 0
   MotionControl    : > 0x20B800,  PAGE = 0
   AxisManager      : > 0x20D900,  PAGE = 0
   ECMaster         : > 0x20DC00,  PAGE = 0

   /* .reset is a standard section used by the compiler.  It contains the */
   /* the address of the start of _c_int00 for C Code.   /*
   /* When using the boot ROM this section and the CPU vector */
   /* table is not needed.  Thus the default type is set here to  */
   /* DSECT  */
   .reset              : > 0x3FFFC0,     PAGE = 0, TYPE = DSECT
   vectors             : > 0x3FFFC2,     PAGE = 0, TYPE = DSECT

   /* Allocate ADC_cal function (pre-programmed by factory into TI reserved memory) */
   .adc_cal     : load = 0x380080,   PAGE = 0, TYPE = NOLOAD

   PieVectTableFile : > PIE_VECT,   PAGE = 0

/*** Peripheral Frame 0 Register Structures ***/
   DevEmuRegsFile    : > DEV_EMU,     PAGE = 0
   FlashRegsFile     : > FLASH_REGS,  PAGE = 0
   CsmRegsFile       : > CSM,         PAGE = 0
   AdcMirrorFile     : > ADC_MIRROR,  PAGE = 0
   XintfRegsFile     : > XINTF,       PAGE = 0
   CpuTimer0RegsFile : > CPU_TIMER0,  PAGE = 0
   CpuTimer1RegsFile : > CPU_TIMER1,  PAGE = 0
   CpuTimer2RegsFile : > CPU_TIMER2,  PAGE = 0
   PieCtrlRegsFile   : > PIE_CTRL,    PAGE = 0
   DmaRegsFile       : > DMA,         PAGE = 0

/*** Peripheral Frame 3 Register Structures ***/
   McbspaRegsFile    : > MCBSPA,      PAGE = 0
   McbspbRegsFile    : > MCBSPB,      PAGE = 0

/*** Peripheral Frame 1 Register Structures ***/
   ECanaRegsFile     : > ECANB,       PAGE = 0
   ECanaLAMRegsFile  : > ECANB_LAM,   PAGE = 0
   ECanaMboxesFile   : > ECANB_MBOX,  PAGE = 0
   ECanaMOTSRegsFile : > ECANB_MOTS,  PAGE = 0
   ECanaMOTORegsFile : > ECANB_MOTO,  PAGE = 0

   ECanbRegsFile     : > ECANA,       PAGE = 0
   ECanbLAMRegsFile  : > ECANA_LAM,   PAGE = 0
   ECanbMboxesFile   : > ECANA_MBOX,  PAGE = 0
   ECanbMOTSRegsFile : > ECANA_MOTS,  PAGE = 0
   ECanbMOTORegsFile : > ECANA_MOTO,  PAGE = 0

   EPwm1RegsFile     : > EPWM1,       PAGE = 0
   EPwm2RegsFile     : > EPWM2,       PAGE = 0
   EPwm3RegsFile     : > EPWM3,       PAGE = 0
   EPwm4RegsFile     : > EPWM4,       PAGE = 0
   EPwm5RegsFile     : > EPWM5,       PAGE = 0
   EPwm6RegsFile     : > EPWM6,       PAGE = 0

   ECap1RegsFile     : > ECAP1,       PAGE = 0
   ECap2RegsFile     : > ECAP2,       PAGE = 0
   ECap3RegsFile     : > ECAP3,       PAGE = 0
   ECap4RegsFile     : > ECAP4,       PAGE = 0
   ECap5RegsFile     : > ECAP5,       PAGE = 0
   ECap6RegsFile     : > ECAP6,       PAGE = 0

   EQep1RegsFile     : > EQEP1,       PAGE = 0
   EQep2RegsFile     : > EQEP2,       PAGE = 0

   GpioCtrlRegsFile  : > GPIOCTRL,    PAGE = 0
   GpioDataRegsFile  : > GPIODAT,     PAGE = 0
   GpioIntRegsFile   : > GPIOINT,     PAGE = 0

/*** Peripheral Frame 2 Register Structures ***/
   SysCtrlRegsFile   : > SYSTEM,      PAGE = 0
   SpiaRegsFile      : > SPIA,        PAGE = 0
   SciaRegsFile      : > SCIA,        PAGE = 0
   XIntruptRegsFile  : > XINTRUPT,    PAGE = 0
   AdcRegsFile       : > ADC,         PAGE = 0
   ScibRegsFile      : > SCIB,        PAGE = 0
   ScicRegsFile      : > SCIC,        PAGE = 0
   I2caRegsFile      : > I2CA,        PAGE = 0

/*** Code Security Module Register Structures ***/
   CsmPwlFile        : > CSM_PWL,     PAGE = 1

/*** Device Part ID Register Structures ***/
   PartIdRegsFile    : > PARTID,      PAGE = 0
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
