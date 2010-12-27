/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef WIN32

/** \file
   OS interface, WIN32 adaptation
*/

#pragma pack(1)

#include <stdio.h>
#include <errno.h>

#include <signal.h>

#include "drv_dsl_cpe_api_types.h"

#include "ifxos_common.h"
#include "ifxos_debug.h"
#include "ifxos_thread.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_lock.h"
#include "ifxos_select.h"
#include "ifxos_common.h"
#include "ifxos_print_io.h"
#include "ifxos_device_access.h"
#include "ifxos_file_access.h"
#include "ifxos_time.h"
#include "ifxos_thread.h"
#include "ifxos_termios.h"
#include "ifxos_pipe.h"
#include "ifxos_misc.h"
#include "ifxos_socket.h"
#include "ifx_getopt.h"

#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */

#define __BIG_ENDIAN IFXOS_BIG_ENDIAN
#define __BYTE_ORDER IFXOS_BYTE_ORDER

#define DSL_CPE_PRIORITY   (0)
#define DSL_CPE_STACKSIZE  (2048)

#define DSL_CPE_STRNCASECMP(a,b,c)   stricmp((a),(b))

/** carriage return for windows */
#define DSL_CPE_CRLF     "\n"

#define	IOCPARM_MASK	0x7f		/* parameters must be < 128 bytes */
#define	IOC_VOID     	0x20000000	/* no parameters */
#define	IOC_OUT		    0x40000000	/* copy out parameters */
#define	IOC_IN		    0x80000000	/* copy in parameters */
#define	IOC_INOUT	   (IOC_IN|IOC_OUT)

#ifndef _IO
#define	_IO(x,y)	   (IOC_VOID|((x)<<8)|y)
#endif /* _IO*/
#ifndef _IOR
#define	_IOR(x,y,t)	   (IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)
#endif /* _IOR*/
#ifndef _IOW
#define	_IOW(x,y,t)	   (IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)
#endif /* _IOW*/
#ifndef _IOWR
#define	_IOWR(x,y,t)   (IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|y)
#endif /* _IOWR*/

#define _IOC_TYPE(x)                      (((x)>>8) & 0xFF)

#define DSL_CPE_StringToAddress(strAddr, iAddr) inet_aton(strAddr, (iAddr))
#define DSL_CPE_AddressToString      inet_ntoa

#define DSL_CPE_KeypressSet     IFXOS_KeypressSet
#define DSL_CPE_KeypressReset   IFXOS_KeypressReset


/** map FILE to own type */
typedef FILE                     DSL_CPE_File_t;

DSL_uint16_t DSL_CPE_Htons(DSL_uint16_t hVal);

DSL_uint32_t DSL_CPE_Htonl(DSL_uint32_t hVal);

#ifdef DSL_DEBUG_TOOL_INTERFACE
DSL_char_t* DSL_CPE_OwnAddrStringGet(DSL_void_t);
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

/**
   Print to a file, pipe, stdout, stderr or memory file.
*/
DSL_int_t DSL_CPE_FPrintf(DSL_CPE_File_t *stream, const DSL_char_t *format, ...)
#ifdef __GNUC__
   __attribute__ ((format (printf, 2, 3)))
#endif
   ;


#pragma pack()

#endif /* WIN32 */


