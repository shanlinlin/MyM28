/*===========================================================================+
|  Function : I/O												             |
|  Task     : I/O Header File                          		 				 |
|----------------------------------------------------------------------------|
|  Compile  :                                                                |
|  Link     :                                                                |
|  Call     :                                                                |
|  Parameter:                                                                |
|----------------------------------------------------------------------------|
|  Author   : zholy                                                          |
|  Version  : V1.00                                                          |
|  Creation : 11/22/2007                                                     |
|  Revision :                                                                |
+===========================================================================*/

#ifndef     D__GPIO
#define     D__GPIO

#include    "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagIO_REG
{
	Uint16		unInput;
	Uint16		unOutput;
	void	(*read)();
	void	(*write)();
} IO_REG;

typedef IO_REG	*IOREG_HANDLE;

#define	IOREG_DEFAULTS { 0,	0, &ReadIO, &WriteIO }

/*===========================================================================+
|           Function and Class prototype                                     |
+===========================================================================*/
void InitGpio(void);
void ReadIO(IOREG_HANDLE);
void WriteIO(IOREG_HANDLE);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif


//===========================================================================
// No more.
//===========================================================================
