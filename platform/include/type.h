/*****************************************************************/
/* Copyright (c) Texas Instruments, Incorporated  2000           */
/*****************************************************************/
/* File         :    type.h	                                    

   Description  :    Generic types.  cthis interface is reentrant.

   Group        :    System.

   Targets      :    Generic.

   Conditions   :    None.

   Author       :    Kwame Kyiamah


   Restrictions :    None.

   Date         :    2.8.99    Created.
 */

#ifndef       TYPE__H  
#define       TYPE__H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum                                      /* boolean */
{
   False = 0x0000,                                /* false */
   True  = 0x0001                                 /* true */
} bool;

typedef float f32;

typedef long s32;

typedef int s16;

typedef unsigned char u8;

typedef unsigned int u16;

typedef unsigned long u32;
#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
