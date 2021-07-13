
// TI File $Revision: /main/1 $
// Checkin $Date: April 22, 2008   14:35:56 $
//###########################################################################
//
// FILE:   DSP28x_Project.h
//
// TITLE:  DSP28x Project Headerfile and Examples Include File
//
//###########################################################################
// $TI Release: 2833x/2823x Header Files V1.32 $
// $Release Date: June 28, 2010 $
//###########################################################################

#ifndef DSP28x_PROJECT_H
#define DSP28x_PROJECT_H

#ifdef D_DSP2812
#include "DSP281x_Device.h"         // DSP281x Headerfile Include File
#include "DSP281x_Examples.h"       // DSP281x Examples Include File

#elif defined D_HYB5_335 || defined D_SMC335M1 || defined D_SMC335M2
#include "DSP2833x_Device.h"        // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"      // DSP2833x Examples Include File

#endif

#endif  // end of DSP28x_PROJECT_H definition
