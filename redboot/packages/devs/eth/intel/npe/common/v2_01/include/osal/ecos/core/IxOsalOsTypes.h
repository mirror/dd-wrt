#ifndef IxOsalOsTypes_H
#define IxOsalOsTypes_H

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>

typedef cyg_int64  INT64;
typedef cyg_uint64 UINT64;
typedef cyg_int32  INT32;
typedef cyg_uint32 UINT32;
typedef cyg_int16  INT16;
typedef cyg_uint16 UINT16;
typedef cyg_int8   INT8;
typedef cyg_uint8  UINT8;

typedef cyg_uint32 ULONG;
typedef cyg_uint16 USHORT;
typedef cyg_uint8  UCHAR;
typedef cyg_uint32 BOOL;

#if 0 // FIXME

/* Default stack limit is 10 KB */
#define IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE  (10240) 

/* Maximum stack limit is 32 MB */
#define IX_OSAL_OS_THREAD_MAX_STACK_SIZE      (33554432)  /* 32 MBytes */

/* Default thread priority */
#define IX_OSAL_OS_DEFAULT_THREAD_PRIORITY    (90)

/* Thread maximum priority (0 - 255). 0 - highest priority */
#define IX_OSAL_OS_MAX_THREAD_PRIORITY	      (255)

#endif // FIXME

#define IX_OSAL_OS_WAIT_FOREVER (-1L)  
#define IX_OSAL_OS_WAIT_NONE	0


/* Thread handle is eventually an int type */
typedef int IxOsalOsThread;

/* Semaphore handle FIXME */   
typedef int IxOsalOsSemaphore;

/* Mutex handle */
typedef cyg_drv_mutex_t IxOsalOsMutex;

/* 
 * Fast mutex handle - fast mutex operations are implemented in
 * native assembler code using atomic test-and-set instructions 
 */
typedef cyg_drv_mutex_t IxOsalOsFastMutex;

typedef struct
{
} IxOsalOsMessageQueue;


#endif // IxOsalOsTypes_H
