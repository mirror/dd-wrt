/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef RTEMS

/** \file
   OS interface, RTEMS adaptation
*/

#include "drv_dsl_cpe_api_types.h"

#include <ctype.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "dispatch.h"
#include "print.h"
#include "dbgio.h"
#include "xapi.h"

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

#define INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE

#define PACKAGE_VERSION              ""

#define DSL_CPE_STACKSIZE            2048

#define DSL_CPE_PRIORITY             64
#define DSL_CPE_PIPE_PRIORITY         64
#define DSL_CPE_TCP_MSG_PRIORITY      64

#define DSL_CPE_STRNCASECMP(a,b,c)   stricmp(a,b)

#define   DSL_CPE_FD_ZERO IFXOS_DevFdZero

/** context from DSL_DRV_Open()*/
extern DSL_OpenContext_t *gv_drv_dsl_pOpenCtx;

#endif /* RTEMS */


