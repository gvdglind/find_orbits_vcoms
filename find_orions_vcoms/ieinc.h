#ifndef _IEINC
#define _IEINC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* testbranch1 */
/* testbranch2 (by git) */
/* testbranch3 */

extern int GetDebugLevel();

#define DEBUG     GetDebugLevel()
#define MAX_ORION 4

/* definition of 'standard' types for each platform
 */
#define BYTE		unsigned char
#define WORD		unsigned short
#define DWORD		unsigned long
#define BOOL		unsigned int
#define USHORT		WORD
#define ULONG		DWORD

#define NELT(a)	    (sizeof(a)/sizeof(a[0]))

/* General */

#ifndef TRUE
   #define TRUE   (1)
#endif

#ifndef FALSE
   #define FALSE  (0)
#endif

#ifndef NULL
   #define NULL   0
#endif

#ifndef  MIN
   #define  MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef  MAX
   #define  MAX(a,b) (((a)>(b))?(a):(b))
#endif

#endif /* _IEINC */

