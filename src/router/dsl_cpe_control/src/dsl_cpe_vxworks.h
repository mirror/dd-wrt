/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef VXWORKS

/** \file
   OS interface, WIN32 adaptation
*/

#include "drv_dsl_cpe_api_types.h"

#include <signal.h>

#include <ctype.h>
#ifdef DSL_DEBUG_TOOL_INTERFACE
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#endif

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

#define PACKAGE_VERSION              ""

#define DSL_CPE_STACKSIZE            2048

#define DSL_CPE_PRIORITY             64
#define DSL_CPE_PIPE_PRIORITY         64
#define DSL_CPE_TCP_MSG_PRIORITY      64

#define DSL_CPE_STRNCASECMP(a,b,c)   stricmp(a,b)

#define DSL_CPE_StringToAddress(strAddr, iAddr) inet_aton(strAddr, (iAddr))
#define DSL_CPE_AddressToString      inet_ntoa

#ifndef FD_SETSIZE
#define DSL_FD_SETSIZE           1024
#else
#define DSL_FD_SETSIZE           FD_SETSIZE
#endif

#define DSL_CPE_File_t                   IFXOS_File_t

void DSL_CPE_KeypressSet (void);
void DSL_CPE_KeypressReset (void);

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

#endif /* VXWORKS */


