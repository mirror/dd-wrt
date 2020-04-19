/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef LINUX

#include "drv_dsl_cpe_api_types.h"

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT

#define _GNU_SOURCE     1
#include <features.h>
#include <limits.h>

#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>               /* fdopen */
#include <sys/fcntl.h>           /* open */
#include <sys/ioctl.h>           /* ioctl */
#include <string.h>              /* memset, strstr, strlen */
#include <stdlib.h>              /* strtoul */
#include <sys/time.h>            /* fd_set */
#include <sys/select.h>          /* select */
#include <sys/ipc.h>             /* ftok */
#include <ctype.h>
#include <unistd.h>              /* daemon */
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>              /* va_start... */
#include <errno.h>
#include <sys/stat.h>            /* stat */
#include <math.h>                /* log10, fabs, ceil */
#include <malloc.h>
#include <termios.h>             /* tcgetattr, tcsetattr */
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>          /* socket */
#include <sys/sem.h>             /* semget */
#include <semaphore.h>           /* sem_t */ 

#ifdef DSL_DEBUG_TOOL_INTERFACE
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

#else

#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>               /* fdopen */
#include <string.h>              /* memset, strstr, strlen */
#include <stdlib.h>              /* strtoul */
#include <sys/time.h>            /* fd_set */
#include <ctype.h>
#include <unistd.h>              /* daemon */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdarg.h>              /* va_start... */
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#ifdef DSL_DEBUG_TOOL_INTERFACE
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#endif /* DSL_DEBUG_TOOL_INTERFACE*/

/** IFXOS includes*/
#include "ifxos_common.h"
#include "ifxos_print_io.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_device_access.h"
#include "ifxos_file_access.h"
#include "ifxos_time.h"
#include "ifxos_lock.h"
#include "ifxos_thread.h"
#include "ifxos_termios.h"
#include "ifxos_pipe.h"
#include "ifxos_misc.h"
#include "ifxos_socket.h"
#endif /* INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

/** \file
   OS interface, Linux adaptation
*/


/*
    ***************** Common definitions ********************************
*/

#define DSL_CPE_STRNCASECMP   strncasecmp

#ifndef FD_SETSIZE
#define DSL_FD_SETSIZE           1024
#else
#define DSL_FD_SETSIZE           FD_SETSIZE
#endif

/* arpa/inet.h mapping */
#define DSL_CPE_StringToAddress(strAddr, iAddr) inet_aton(strAddr, (iAddr))
#define DSL_CPE_AddressToString      inet_ntoa

/* sys/socket.h mapping */
#define DSL_CPE_SockOptSet           setsockopt


/** map FILE to own type */
typedef FILE                     DSL_CPE_File_t;

/** map timeval structure */
typedef struct timeval           DSL_TimeVal_t;

/** map the sockaddr_in to own type */
typedef struct linger            DSL_SockOptLinger_t;


/**
   Installs system dependant handlers (e.g. signal handler on linux)
*/
DSL_void_t DSL_CPE_HandlerInstall(DSL_void_t);

void DSL_CPE_KeypressSet (void);
void DSL_CPE_KeypressReset (void);

/**
   Set environment variable
*/
DSL_Error_t DSL_CPE_SetEnv(const DSL_char_t *sName, const DSL_char_t *sValue);

/**
   Execute a shell command
*/
DSL_Error_t DSL_CPE_System(const DSL_char_t *sCommand);

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

/*
    ***************** IFXOS dependent definitions **************************
*/

#ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT

#ifndef __BIG_ENDIAN
   #define __BIG_ENDIAN       1
   #define __LITTLE_ENDIAN    2
#endif

#ifndef __BYTE_ORDER
   #define __BYTE_ORDER       __BIG_ENDIAN
#endif

/** LINUX User - LOCK, user space semaphore for synchronisation. */
typedef struct
{
   /** lock id */
   sem_t object;  
   /** valid flag */
   DSL_boolean_t bValid;
} DSL_CPE_Lock_t; 

#define DSL_LOCK_INIT_VALID(P_LOCK_ID)\
   (((P_LOCK_ID)) ? (((P_LOCK_ID)->bValid == DSL_TRUE) ? DSL_TRUE : DSL_FALSE) : DSL_FALSE)


/** map the fd_set to own type */
typedef fd_set                   DSL_CPE_fd_set_t;

typedef int                      DSL_Socket_t;
/** map the sockaddr_in to own type */
typedef struct sockaddr_in       DSL_sockaddr_in_t;

/** map the sockaddr to own type */
typedef struct sockaddr          DSL_sockaddr_t;

/** map the socklen_t to own type */
typedef socklen_t                DSL_socklen_t;

/** map stat structure */
typedef struct stat              DSL_CPE_stat_t;

/** map to stderr */
#define DSL_CPE_STDERR               stderr

/** map to stdout */
#define DSL_CPE_STDOUT               stdout

/** map to stdin */
#define DSL_CPE_STDIN                stdin

#define DSL_CPE_snprintf             snprintf

/* stdio.h mapping */
#define DSL_CPE_FdOpen               fdopen

#define DSL_CPE_INADDR_ANY           INADDR_ANY

#define DSL_CPE_SocketClose          close
#define DSL_CPE_SocketRecv(s,buf,sz) recv(s,buf,sz,0)
#define DSL_CPE_SocketSend(s,buf,sz) send(s,buf,sz,MSG_DONTWAIT)

/**
   LINUX User Thread - map the Thread ID.
*/
typedef int    DSL_CPE_Thread_t;

/**
   LINUX User Thread - function type LINUX User Thread Start Routine.
*/
typedef void *(*DSL_USER_THREAD_StartRoutine)(void*);

#define DSL_CPE_THREAD_NAME_LEN              16
#define DSL_THREAD_DOWN_WAIT_POLL_MS         10
#define DSL_THREAD_DELETE_WAIT_FOREVER       0xFFFFFFFF

typedef struct
{
   /** user argument 1 */
   DSL_uint32_t   nArg1;
   /** user argument 2 */
   DSL_uint32_t   nArg2;
   /** name of the thread/task */
   DSL_char_t     pName[DSL_CPE_THREAD_NAME_LEN];

   /** control - signal the run state */
   volatile DSL_boolean_t  bRunning;
   /** control - set to shutdown the thread */
   volatile DSL_boolean_t  bShutDown;
} DSL_CPE_Thread_Params_t;

typedef DSL_int_t (*DSL_CPE_ThreadFunction_t)(DSL_CPE_Thread_Params_t *);

/**
   LINUX User Thread - Control struct for thread handling.
*/
typedef struct
{
   /* Contains the user and thread control parameters */
   DSL_CPE_Thread_Params_t    thrParams;

   /* Points to the thread start routine */
   DSL_CPE_ThreadFunction_t  pThrFct;

   /** LINUX User specific for internal - keep the task thread ID */
   DSL_CPE_Thread_t          tid;

   /** requested kernel thread priority */
   DSL_int32_t             nPriority;
   
   /** flag indicates that the structure is initialized */
   DSL_boolean_t           bValid;
   
} DSL_CPE_ThreadCtrl_t;

DSL_int32_t DSL_CPE_ThreadInit(  DSL_CPE_ThreadCtrl_t *pThrCntrl,
                                 DSL_char_t     *pName,
                                 DSL_CPE_ThreadFunction_t pThreadFunction,
                                 DSL_uint32_t   nStackSize,
                                 DSL_uint32_t   nPriority,
                                 DSL_uint32_t   nArg1,
                                 DSL_uint32_t   nArg2);
               
DSL_int32_t DSL_CPE_ThreadShutdown( DSL_CPE_ThreadCtrl_t *pThrCntrl,
                                DSL_uint32_t waitTime_ms);
               
DSL_CPE_Thread_t DSL_CPE_ThreadIdGet(void);
               
#define DSL_THREAD_INIT_VALID(P_THREAD_ID)  (((P_THREAD_ID)) ? (((P_THREAD_ID)->bValid == DSL_TRUE) ? DSL_TRUE : DSL_FALSE) : DSL_FALSE)

/**
   Open a device
*/
DSL_int_t DSL_CPE_Open(const DSL_char_t *pName);

/**
   Close a device
*/
DSL_int_t DSL_CPE_Close(const DSL_int_t fd);

/**
   Write to a device
*/
DSL_int_t DSL_CPE_Write(const DSL_int_t fd, const DSL_void_t *pData, const DSL_uint32_t nSize);

/**
   Read from a device
*/
DSL_int_t DSL_CPE_Read(const DSL_int_t fd, DSL_void_t *pData, const DSL_uint32_t nSize);

/**
   Control a device
*/
DSL_int_t DSL_CPE_Ioctl(const DSL_int_t fd, const DSL_uint32_t cmd, DSL_int_t param);

/**
   Wait for a device wake up
*/
DSL_int_t DSL_CPE_Select(const DSL_int_t max_fd,
                     const DSL_CPE_fd_set_t *read_fd_in,
                     DSL_CPE_fd_set_t *read_fd_out,
                     const DSL_uint32_t timeout_msec);

/**
   Fill a descriptor set with zeros.
*/
DSL_void_t DSL_CPE_FD_ZERO(DSL_CPE_fd_set_t *set);

/**
   Mark a descriptor in use.
*/
DSL_void_t DSL_CPE_FD_SET(DSL_int_t fd, DSL_CPE_fd_set_t *set);

/**
   Clear a descriptor in use flag.
*/
DSL_void_t DSL_CPE_FD_CLR(DSL_int_t fd, DSL_CPE_fd_set_t *set);

/**
   Check if a descriptor is set.
*/
DSL_int_t DSL_CPE_FD_ISSET(DSL_int_t fd, const DSL_CPE_fd_set_t *set);

/**
   Get a byte from stdin .
*/
DSL_int_t DSL_CPE_GetChar(DSL_void_t);

/**
   Write a byte stdout .
*/
DSL_int_t DSL_CPE_PutChar(DSL_char_t c, DSL_CPE_File_t *stream);

/**
   Create a pipe.

   \param
      pName - pipe name
   \return
      - DSL_SUCCESS on success
      - DSL_ERROR on failure
 */
DSL_Error_t DSL_CPE_PipeCreate(DSL_char_t *pName);

/**
   Open a pipe.

   \param
      pName - pipe name
   \return
      - pointer to FILE structure
      - in case of error the return value is NULL
 */
DSL_CPE_File_t *DSL_CPE_PipeOpen(DSL_char_t *pName, DSL_boolean_t reading, DSL_boolean_t blocking);

/**
   Close a pipe.

   \param
      pipe - pipe stream
   \return
      - (0) on success
      - (-1) on failure
*/
DSL_Error_t DSL_CPE_PipeClose(DSL_CPE_File_t *file);

/**
   Initialize a semaphore.

   \param lock semaphore identifier

   \return
   0 on success.
*/
DSL_int32_t DSL_CPE_LockCreate(DSL_CPE_Lock_t *lock);

/**
   Delete a semaphore.

   \param lock semaphore identifier

   \return
      - DSL_SUCCESS on success
      - DSL_ERROR on failure
 */
DSL_int32_t DSL_CPE_LockDelete(DSL_CPE_Lock_t *lock);

/**
   Gets a semaphore.

   \param lock        semaphore identifier

   \return
      - (0) on success
      - (-1) on failure
*/
DSL_int32_t DSL_CPE_LockGet(DSL_CPE_Lock_t *lock);

/**
   Get the Lock with timeout.

\param
   lockId   Provides the pointer to the Lock Object.

\param
   timeout_ms  Timeout value [ms]
               - 0: no wait
               - -1: wait forever
               - any other value: waiting for specified amount of milliseconds
\param
   pRetCode    Points to the return code variable. [O]
               - If the pointer is NULL the return code will be ignored, else
                 the corresponding return code will be set
               - For timeout the return code is set to 1.

\return
   DSL_SUCCESS on success.
   DSL_ERROR   on error or timeout.

\note
   To detect timeouts provide the return code varibale, in case of timeout
   the return code is set to 1.
*/
DSL_int32_t DSL_CPE_LockTimedGet(
               DSL_CPE_Lock_t *lockId,
               DSL_uint32_t timeout_ms,
               DSL_int32_t  *pRetCode);


/**
   Return the semaphore

   \param lock        semaphore identifier

   \return
      - (0) on success
      - (-1) on failure
 */
DSL_int32_t DSL_CPE_LockSet(DSL_CPE_Lock_t *lock);

/**
   Sleep specified number of seconds.

   \param nSeconds Number of seconds to sleep
*/
DSL_void_t DSL_CPE_Sleep(DSL_uint32_t nSeconds);

/**
   Sleep specified number of milliseconds.

   \param nMs Number of milliseconds to sleep
*/
DSL_void_t DSL_CPE_MSecSleep(DSL_uint32_t nMs);

/**
   Allocates a memory block

   \param size Bytes to allocate

   \return
   returns a DSL_void_t pointer to the allocated space, NULL if there's not
   sufficient memory space available.
*/
DSL_void_t *DSL_CPE_Malloc(DSL_uint32_t size);

/**
   Deallocates a memory block

   \param memblock Previously allocated memory block that should be freed

*/
void DSL_CPE_Free(DSL_void_t *memblock);


/**
   Open a file.
*/
DSL_CPE_File_t *DSL_CPE_FOpen(const DSL_char_t *name,  const DSL_char_t *mode);

#ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT
/**
   Open a memory file. The size is limited to 32 kBytes.

   \param buf     temporary buffer for for fprintf operation.
   \param size    buffer size
   \param mode    not used

   \return
      - pointer to FILE structure
      - in case of error the return value is NULL
*/
DSL_CPE_File_t *DSL_CPE_FMemOpen(DSL_char_t *buf, const DSL_uint32_t size, const DSL_char_t *mode);

#endif /* #ifdef INCLUDE_DSL_CPE_SOAP_SUPPORT */
/**
   Close the file or memory file.
*/
DSL_int_t DSL_CPE_FClose(DSL_CPE_File_t *fd);

/**
   Flush a file or pipe.
*/
DSL_int_t DSL_CPE_FFlush(DSL_CPE_File_t *fd);

/**
   Read from file, stdin .
*/
DSL_int_t DSL_CPE_FRead(DSL_void_t *buf, DSL_uint32_t size,  DSL_uint32_t count, DSL_CPE_File_t *stream);

/**
   Write to a file, pipe, stdout.
*/
DSL_int_t DSL_CPE_FWrite(const DSL_void_t *buf, DSL_uint32_t size, DSL_uint32_t count, DSL_CPE_File_t *stream);

/**
   End of file test of a file.
*/
DSL_int_t DSL_CPE_Feof(DSL_CPE_File_t *stream);

/**
   Read a string from the file.
*/
DSL_char_t *DSL_CPE_FGets(DSL_char_t *str, DSL_int_t n, DSL_CPE_File_t *stream);

/**
  get file status
*/
DSL_int_t DSL_CPE_FStat(DSL_char_t *str, DSL_CPE_stat_t *st);

DSL_Socket_t DSL_CPE_Accept(DSL_Socket_t socFd, DSL_sockaddr_in_t  *pSocAddr);

DSL_int_t DSL_CPE_Socket(DSL_int_t socType, DSL_Socket_t *pSocketFd);

DSL_int_t DSL_CPE_SocketBind(
                  DSL_Socket_t    socFd, 
                  DSL_sockaddr_in_t  *pSocAddr);

void DSL_CPE_EchoOff (void);
void DSL_CPE_EchoOn (void);
#endif /* #ifndef INCLUDE_DSL_CPE_IFXOS_SUPPORT*/

#endif /* LINUX */
