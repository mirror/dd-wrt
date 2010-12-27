/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef DSL_CPE_API_OS_H
#define DSL_CPE_API_OS_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   Operating System depending files, definitions and functions
*/
#include "drv_dsl_cpe_api_types.h"
#include "drv_dsl_cpe_api.h"

#if defined(INCLUDE_DSL_CPE_IFXOS_SUPPORT) || defined(VXWORKS)
   /* support other OS only through lib_ifxos */
   #ifndef USE_LIB_IFXOS
      #define USE_LIB_IFXOS 1
   #endif
#endif /** INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

#ifndef _lint
#ifdef LINUX
#include "dsl_cpe_linux.h"
#elif WIN32
#include "dsl_cpe_win32.h"
#elif VXWORKS
#include "dsl_cpe_vxworks.h"
#elif ECOS
#include "dsl_cpe_ecos.h"
#elif RTEMS
#include "dsl_cpe_rtems.h"
#elif GENERIC_OS
#include "dsl_cpe_generic_os.h"
#else
#error please define your OS for the CPE Control adaptation
#endif
#else
#include "dsl_cpe_os_lint_map.h"
#endif /* _lint*/

#ifndef __BIG_ENDIAN
   #error please define the __BIG_ENDIAN macro
#endif

#ifndef __BYTE_ORDER
   #error please specify the endianess of your target system
#endif

/**
   System definitions
*/

#ifndef DSL_CPE_PRIORITY
#define DSL_CPE_PRIORITY            (0)
#endif

#ifndef DSL_CPE_STACKSIZE
#define DSL_CPE_STACKSIZE           (2048)
#endif

#ifndef DSL_CPE_EVENT_STACKSIZE
#define DSL_CPE_EVENT_STACKSIZE      (8192)
#endif

/** Default stack size */
#ifndef DSL_CPE_DEFAULT_STACK_SIZE
#define DSL_CPE_DEFAULT_STACK_SIZE   (4096)
#endif

/** Control task stack size */
#ifndef DSL_CPE_CONTROL_STACK_SIZE
#define DSL_CPE_CONTROL_STACK_SIZE   (4096)
#endif

/** SOAP task stack size */
#ifndef DSL_CPE_SOAP_STACK_SIZE
#define DSL_CPE_SOAP_STACK_SIZE      (20000)
#endif

/** Pipe task stack size */
#ifndef DSL_CPE_PIPE_STACK_SIZE
#define DSL_CPE_PIPE_STACK_SIZE      (8192)
#endif

/** Pipe task priority */
#ifndef DSL_CPE_PIPE_PRIORITY
#define DSL_CPE_PIPE_PRIORITY        (0)
#endif

#ifndef DSL_CPE_TCP_MSG_STACKSIZE
#define DSL_CPE_TCP_MSG_STACKSIZE    (8192)
#endif

#ifndef DSL_CPE_TCP_MSG_PRIORITY
#define DSL_CPE_TCP_MSG_PRIORITY     (0)
#endif

/**
   Carriage Return + Line Feed, maybe overwritten by compile switches
   or OS-specific adaptation */
#ifndef DSL_CPE_CRLF
#define DSL_CPE_CRLF                    "\n\r"
#endif

int DSL_CPE_debug_printf(DSL_char_t *fmt, ...);

#if defined(USE_LIB_IFXOS) && (USE_LIB_IFXOS == 1)

/*
   common defines - ifxos_common.h"
*/

#define DSL_BYTE_ORDER               IFXOS_BYTE_ORDER
#define DSL_LITTLE_ENDIAN            IFXOS_LITTLE_ENDIAN
#define DSL_BIG_ENDIAN               IFXOS_BIG_ENDIAN

#ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN             IFXOS_BIG_ENDIAN
   #define __LITTLE_ENDIAN          IFXOS_LITTLE_ENDIAN
   #define __BYTE_ORDER             IFXOS_BYTE_ORDER
#endif

#ifndef __BYTE_ORDER
   #define __BYTE_ORDER       __LITTLE_ENDIAN
#endif

/*
   Function map - stdio, string
*/
#define DSL_CPE_GetChar                  IFXOS_GetChar
#define DSL_CPE_PutChar                  IFXOS_PutChar

#define DSL_CPE_FGets                    IFXOS_FGets
/*#define DSL_CPE_FPrintf                  IFXOS_FPrintf*/
#define DSL_CPE_snprintf                 IFXOS_SNPrintf
#define DSL_vsnprintf                    IFXOS_VSNPrintf

/*
   Function map - Memory Functions.
*/

#if ( defined(IFXOS_HAVE_MEM_ALLOC) && (IFXOS_HAVE_MEM_ALLOC == 1) )
#define DSL_CPE_Malloc                   IFXOS_MemAlloc
#define DSL_CPE_Free                     IFXOS_MemFree
#endif

/*
   Function map - Device handling (open, close ...).
*/

#define DSL_CPE_fd_set_t                 IFXOS_devFd_set_t
#define DSL_CPE_FD_SET                   IFXOS_DevFdSet
#define DSL_CPE_FD_ISSET                 IFXOS_DevFdIsSet
#define DSL_CPE_FD_ZERO                  IFXOS_DevFdZero
#define DSL_CPE_FD_CLR                   FD_CLR /*KAv: no IFX OS support!*/


#if ( defined(IFXOS_HAVE_DEVICE_ACCESS) && (IFXOS_HAVE_DEVICE_ACCESS == 1) )
#define DSL_CPE_Open                    IFXOS_DeviceOpen
#define DSL_CPE_Close                   IFXOS_DeviceClose
#define DSL_CPE_Read                    IFXOS_DeviceRead
#define DSL_CPE_Write                   IFXOS_DeviceWrite
#define DSL_CPE_Ioctl                   IFXOS_DeviceControl
#define DSL_CPE_Select                  IFXOS_DeviceSelect
#endif

/*
   function types - "ifxos_file_access.h"
*/

#if ( defined(IFXOS_HAVE_FILE_ACCESS) && (IFXOS_HAVE_FILE_ACCESS == 1) )
#define DSL_CPE_STDERR                   IFXOS_STDERR
#define DSL_CPE_STDOUT                   IFXOS_STDOUT
#define DSL_CPE_STDIN                    IFXOS_STDIN
#define DSL_CPE_stat_t                   IFXOS_stat_t

#define DSL_CPE_FdOpen                   fdopen
#define DSL_CPE_FOpen                    IFXOS_FOpen
#define DSL_CPE_FClose                   IFXOS_FClose
#define DSL_CPE_FRead                    IFXOS_FRead
#define DSL_CPE_FWrite                   IFXOS_FWrite
#define DSL_CPE_FFlush                   IFXOS_FFlush
#define DSL_CPE_Feof                     IFXOS_FEof
#define DSL_CPE_FStat                    IFXOS_Stat
#define DSL_CPE_FMemOpen                 IFXOS_FMemOpen

#endif

/*
   Function map - Time and Wait Functions and Defines.
*/
#define DSL_CPE_Sleep                    IFXOS_SecSleep
#define DSL_CPE_MSecSleep                IFXOS_MSecSleep

/*
   Function map - Lock functions and defines.
*/

#define DSL_CPE_Lock_t                   IFXOS_lock_t

#define DSL_LOCK_INIT_VALID              IFXOS_LOCK_INIT_VALID
#define DSL_CPE_LockCreate               IFXOS_LockInit
#define DSL_CPE_LockDelete               IFXOS_LockDelete
#define DSL_CPE_LockSet                  IFXOS_LockRelease
#define DSL_CPE_LockTimedGet             IFXOS_LockTimedGet
#define DSL_CPE_LockGet                  IFXOS_LockGet

/*
   Function map - Lock functions and defines.
*/

#define DSL_CPE_Thread_t                 IFXOS_thread_t
#define DSL_CPE_ThreadFunction_t         IFXOS_ThreadFunction_t
#define DSL_CPE_ThreadCtrl_t             IFXOS_ThreadCtrl_t
#define DSL_CPE_Thread_Params_t          IFXOS_ThreadParams_t

#define DSL_CPE_ThreadInit               IFXOS_ThreadInit
#define DSL_CPE_ThreadDelete             IFXOS_ThreadDelete
#define DSL_CPE_ThreadShutdown           IFXOS_ThreadShutdown
#define DSL_CPE_ThreadIdGet              IFXOS_ThreadIdGet

/*
   Function map - Pipe functions and defines.
*/

#define DSL_CPE_Pipe_t                   IFXOS_Pipe_t

#define DSL_CPE_PipeCreate               IFXOS_PipeCreate
#define DSL_CPE_PipeOpen                 IFXOS_PipeOpen
#define DSL_CPE_PipeClose                IFXOS_PipeClose

/*
   Function map - Terminal functions and defines.
*/

#define DSL_CPE_EchoOff                  IFXOS_EchoOff
#define DSL_CPE_EchoOn                   IFXOS_EchoOn

/*
   function types - "ifxos_socket.h"
*/
#define DSL_CPE_INADDR_ANY               IFXOS_SOC_INADDR_ANY
#define DSL_CPE_ADDR_LEN                 IFXOS_SOC_ADDR_LEN_BYTE

#define DSL_sockaddr_in_t                IFXOS_sockAddr_t
#define DSL_sockaddr_t                   IFXOS_sockAddr_t
#define DSL_Socket_t                     IFXOS_socket_t

#define DSL_CPE_Socket                   IFXOS_SocketCreate
#define DSL_CPE_Accept                   IFXOS_SocketAccept
#define DSL_CPE_SocketBind               IFXOS_SocketBind
#define DSL_CPE_SocketClose              IFXOS_SocketClose
#define DSL_CPE_SocketRecv               IFXOS_SocketRecv
#define DSL_CPE_SocketSend               IFXOS_SocketSend
#define DSL_CPE_AddressToString          inet_ntoa /*KAv: no IFX OS support!*/

#else /* (USE_LIB_IFXOS == 1) */

/* use the built in OS abstraction layer */
/* this is obsolete, in mid term it will be removed from source tree */

#endif /* (USE_LIB_IFXOS == 1) */

#ifdef __cplusplus
}
#endif

#endif /* DSL_CPE_API_OS_H */
