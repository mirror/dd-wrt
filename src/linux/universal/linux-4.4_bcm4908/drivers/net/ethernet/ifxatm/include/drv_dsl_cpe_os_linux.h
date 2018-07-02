/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_OS_LINUX_H
#define _DRV_DSL_CPE_OS_LINUX_H

#ifdef __cplusplus
   extern "C" {
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/sched.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
#include <linux/utsrelease.h>
#else
#include <generated/utsrelease.h>
#endif

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
//#include <linux/smp_lock.h>
#include <asm/ioctl.h>

#ifdef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT
/** IFXOS includes*/
#include "ifxos_common.h"
#include "ifxos_copy_user_space.h"
#include "ifxos_debug.h"
#include "ifxos_event.h"
#include "ifxos_thread.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_memory_map.h"
#include "ifxos_lock.h"
#include "ifxos_time.h"
#include "ifxos_select.h"
#include "ifxos_mutex.h"
#else
#  ifdef INCLUDE_DSL_BONDING
   #include <linux/ioport.h>
   #include <asm/io.h>
#  endif /* INCLUDE_DSL_BONDING*/
#endif /* INCLUDE_DSL_CPE_API_IFXOS_SUPPORT*/

#ifndef __LINUX__
#define __LINUX__
#endif /*__LINUX__*/

#ifndef KERNEL_VERSION
   #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

/* The major number of a CPE API device
      Typically it is a Voodoo 3dfx device (107) for Danube, Amazon-SE, AR9 and
      logical volume manager (109) for VINAX
*/
#if defined (INCLUDE_DSL_CPE_API_DANUBE)
   #define DRV_DSL_CPE_API_DEV_MAJOR 107
#elif defined (INCLUDE_DSL_CPE_API_VINAX)
   #define DRV_DSL_CPE_API_DEV_MAJOR 109
#else
   #error "Device is not defined!"
#endif

/* The name of this device */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#define DRV_DSL_CPE_API_DEV_NAME "dsl_cpe_api"
#else
#define DRV_DSL_CPE_API_DEV_NAME KBUILD_MODNAME
#endif

#ifndef DSL_DRV_STACKSIZE
#define DSL_DRV_STACKSIZE 2048
#endif

#ifndef DSL_DRV_PRIORITY
#define DSL_DRV_PRIORITY  64
#endif

/**
   Definition of the plattform endianess.
*/
#define DSL_LITTLE_ENDIAN  0x1234
#define DSL_BIG_ENDIAN     0x4321

#if defined ( __LITTLE_ENDIAN )
   #define DSL_BYTE_ORDER             DSL_LITTLE_ENDIAN
#elif defined ( __BIG_ENDIAN )
   #define DSL_BYTE_ORDER             DSL_BIG_ENDIAN
#else
#error "Unknown Byteorder !!!"
#endif

/** LINUX Kernel Thread Name Lenght*/
#define DSL_DRV_THREAD_NAME_LEN 16

#define DSL_DRV_THREAD_DELETE_WAIT_FOREVER     0xFFFFFFF

#define DSL_DRV_THREAD_OPTION_NOT_USED_FOR_LINUX     0

/** LINUX Kernel Thread - priority - IDLE */
#define DSL_DRV_THREAD_PRIO_IDLE                     1
/** LINUX Kernel Thread - priority - LOWEST */
#define DSL_DRV_THREAD_PRIO_LOWEST                   5
/** LINUX Kernel Thread - priority - LOW */
#define DSL_DRV_THREAD_PRIO_LOW                      20
/** LINUX Kernel Thread - priority - NORMAL */
#define DSL_DRV_THREAD_PRIO_NORMAL                   40
/** LINUX Kernel Thread - priority - HIGH */
#define DSL_DRV_THREAD_PRIO_HIGH                     60
/** LINUX Kernel Thread - priority - HIGHEST */
#define DSL_DRV_THREAD_PRIO_HIGHEST                  80
/** LINUX Kernel Thread - priority - TIME_CRITICAL
\attention
   You should use this priority only for driver threads.
*/
#define DSL_DRV_THREAD_PRIO_TIME_CRITICAL            90

/** LINUX Kernel Thread - default prio (use OS default)  */
#define DSL_DRV_DEFAULT_PRIO                         DSL_DRV_THREAD_OPTION_NOT_USED_FOR_LINUX

/** LINUX Kernel Thread - internal poll time for check thread end */
#define DSL_DRV_THREAD_DOWN_WAIT_POLL_MS             10

/** LINUX Kernel Thread - thread options */
#define DSL_DRV_DRV_THREAD_OPTIONS                   (CLONE_FS | CLONE_FILES)
/** LINUX Kernel Thread - default stack size (use OS default)  */
#define DSL_DRV_DEFAULT_STACK_SIZE                   DSL_DRV_THREAD_OPTION_NOT_USED_FOR_LINUX

DSL_uint32_t DSL_DRV_SysTimeGet(DSL_uint32_t nOffset);
#ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT
DSL_uint32_t DSL_DRV_ElapsedTimeMSecGet(DSL_uint32_t refTime_ms);

#if defined(INCLUDE_DSL_CPE_API_VINAX) && defined(INCLUDE_DSL_BONDING)
DSL_int32_t DSL_DRV_Phy2VirtMap(
               DSL_uint32_t    physicalAddr,
               DSL_uint32_t    addrRangeSize_byte,
               DSL_char_t     *pName,
               DSL_uint8_t    **ppVirtAddr);

DSL_int32_t DSL_DRV_Phy2VirtUnmap(
               DSL_uint32_t    *pPhysicalAddr,
               DSL_uint32_t    addrRangeSize_byte,
               DSL_uint8_t    **ppVirtAddr);
#endif /** #if defined(INCLUDE_DSL_CPE_API_VINAX) && defined(INCLUDE_DSL_BONDING)*/
#endif /** #ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT*/

/* The device driver related types */
typedef struct file DSL_DRV_file_t;
typedef struct inode DSL_DRV_inode_t;
#ifndef _lint
typedef loff_t DSL_DRV_offset_t;
#else
typedef int DSL_DRV_offset_t;
#endif

/* operating system types */
typedef size_t                   DSL_DRV_size_t;
typedef ssize_t                  DSL_ssize_t;
typedef DSL_uint32_t             DSL_DRV_TimeVal_t;
typedef struct poll_table_struct DSL_DRV_Poll_Table_t;

#ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
typedef struct mutex    DSL_DRV_Mutex_t;
#else
typedef struct semaphore         DSL_DRV_Mutex_t;
#endif
typedef wait_queue_head_t        DSL_DRV_Event_t;
typedef wait_queue_head_t        DSL_DRV_WaitQueue_t;
#else
typedef IFXOS_lock_t             DSL_DRV_Mutex_t;
typedef IFXOS_event_t            DSL_DRV_Event_t;
typedef IFXOS_drvSelectQueue_t   DSL_DRV_WaitQueue_t;
#endif /** #ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT*/

#ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT
#define DSL_DRV_Malloc(nSize)                vmalloc(nSize)
#define DSL_DRV_MemFree(pPtr)                vfree(pPtr)
#define DSL_DRV_PMalloc(nSize)               kmalloc(nSize, GFP_KERNEL)
#define DSL_DRV_PFree(pPtr)                  kfree(pPtr)
/* mutex macros */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,0))
static inline int dsl_mutex_lock(struct mutex *mutex)
{
   mutex_lock(mutex);
   return 0;
}
#define DSL_DRV_MUTEX_INIT(id)               mutex_init(&(id))
#define DSL_DRV_MUTEX_LOCK(id)               dsl_mutex_lock(&(id))
#define DSL_DRV_MUTEX_UNLOCK(id)             mutex_unlock(&(id))
#else
#define DSL_DRV_MUTEX_INIT(id)               sema_init(&(id), 1)
#define DSL_DRV_MUTEX_LOCK(id)               down_interruptible(&(id))
#define DSL_DRV_MUTEX_UNLOCK(id)             up(&(id))
#endif
#define DSL_DRV_INIT_WAKELIST(name,queue)    init_waitqueue_head(&(queue))
#define DSL_DRV_WAKEUP_WAKELIST(queue)       wake_up_interruptible(&(queue))
#define DSL_DRV_INIT_EVENT(name,ev)          init_waitqueue_head(&(ev))
/* wait for an event, timeout is measured in ms */
#define DSL_DRV_WAIT_EVENT_TIMEOUT(ev,t)     interruptible_sleep_on_timeout(&(ev), (t) * HZ / 1000)
#define DSL_DRV_WAIT_EVENT(ev)               interruptible_sleep_on(&(ev))
#define DSL_DRV_WAKEUP_EVENT(ev)             wake_up_interruptible(&(ev))
#define DSL_DRV_TimeMSecGet()                DSL_DRV_ElapsedTimeMSecGet(0)
#define DSL_WAIT(ms)   msleep(ms)
#define DSL_DRV_MSecSleep(msec)              msleep(msec)
#else
/** IFXOS specific mapping for the system calls*/
#define DSL_DRV_Malloc(nSize)                IFXOS_MemAlloc(nSize)
#define DSL_DRV_MemFree(pPtr)                IFXOS_MemFree(pPtr)
#define DSL_DRV_PMalloc(nSize)               IFXOS_BlockAlloc(nSize)
#define DSL_DRV_PFree(pPtr)                  IFXOS_BlockFree(pPtr)
#define DSL_DRV_MUTEX_INIT(id)               IFXOS_LockInit(&(id))
#define DSL_DRV_MUTEX_LOCK(id)               IFXOS_LockGet(&(id))
#define DSL_DRV_MUTEX_UNLOCK(id)             IFXOS_LockRelease(&(id))
#define DSL_DRV_INIT_WAKELIST(name,queue)    IFXOS_DrvSelectQueueInit(&(queue))
#define DSL_DRV_WAKEUP_WAKELIST(queue)       IFXOS_DrvSelectQueueWakeUp(&(queue), IFXOS_DRV_SEL_WAKEUP_TYPE_RD)
#define DSL_DRV_INIT_EVENT(name,ev)          IFXOS_EventInit(&(ev));
#define DSL_DRV_WAIT_EVENT_TIMEOUT(ev,t)     IFXOS_EventWait(&(ev), (t), IFX_NULL)
#define DSL_DRV_WAIT_EVENT(ev)               IFXOS_EventWait(&(ev), 0xFFFFFFFF, IFX_NULL)
#define DSL_DRV_WAKEUP_EVENT(ev)             IFXOS_EventWakeUp(&(ev));
#define DSL_DRV_ElapsedTimeMSecGet(t)        IFXOS_ElapsedTimeMSecGet(t)
#define DSL_DRV_TimeMSecGet()                IFXOS_ElapsedTimeMSecGet(0)
#define DSL_WAIT(ms)                         IFXOS_MSecSleep(ms)
#define DSL_DRV_MSecSleep(msec)              IFXOS_MSecSleep(msec)
#define DSL_DRV_Phy2VirtMap                  IFXOS_Phy2VirtMap
#define DSL_DRV_Phy2VirtUnmap                IFXOS_Phy2VirtUnmap
#endif /** #ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT*/

#define DSL_IsTimeNull(t)                    ((t) == 0)
#define DSL_DRV_TimeSecGet(t)                (t)
#define DSL_Le2Cpu(le)                       le16_to_cpu(le)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   #define DSL_DRV_OS_ModUseCountIncrement() MOD_INC_USE_COUNT
   #define DSL_DRV_OS_ModUseCountDecrement() MOD_DEC_USE_COUNT
#else
   #define DSL_DRV_OS_ModUseCountIncrement() while(0) {}
   #define DSL_DRV_OS_ModUseCountDecrement() while(0) {}
#endif

#ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT
typedef int (*DSL_DRV_KERNEL_THREAD_StartRoutine)(void *);

typedef struct DSL_DRV_ThreadParams_s
{
   /** user argument 1 */
   DSL_uint32_t   nArg1;
   /** user argument 2 */
   DSL_uint32_t   nArg2;
   /** name of the thread/task */
   DSL_char_t     pName[DSL_DRV_THREAD_NAME_LEN];

   /** control - signal the run state */
   volatile DSL_boolean_t  bRunning;
   /** control - set to shutdown the thread */
   volatile DSL_boolean_t  bShutDown;
} DSL_DRV_ThreadParams_t;

typedef DSL_int_t (*DSL_DRV_ThreadFunction_t)(DSL_DRV_ThreadParams_t *);

typedef struct
{
   /** Contains the user and thread control parameters */
   DSL_DRV_ThreadParams_t    thrParams;

   /** Points to the thread start routine */
   DSL_DRV_ThreadFunction_t  pThrFct;

   /** Kernel thread process ID */
   DSL_int32_t             pid;

   /** requested kernel thread priority */
   DSL_int32_t             nPriority;

   /** LINUX specific internal data - completion handling */
   struct completion       thrCompletion;

   /** flag indicates that the structure is initialized */
   DSL_boolean_t           bValid;

} DSL_DRV_ThreadCtrl_t;

typedef DSL_int_t (*DSL_ThreadFunc_t)(DSL_void_t *pArg);

DSL_int32_t DSL_DRV_ThreadInit(
               DSL_DRV_ThreadCtrl_t *pThrCntrl,
               DSL_char_t     *pName,
               DSL_DRV_ThreadFunction_t pThreadFunction,
               DSL_uint32_t   nStackSize,
               DSL_uint32_t   nPriority,
               DSL_uint32_t   nArg1,
               DSL_uint32_t   nArg2);

DSL_int32_t DSL_DRV_ThreadShutdown(
               DSL_DRV_ThreadCtrl_t *pThrCntrl,
               DSL_uint32_t       waitTime_ms);

#define DSL_DRV_THREAD(a, b, c, d)              DSL_DRV_ThreadInit((a), (b), (c), DSL_DRV_STACKSIZE, DSL_DRV_PRIORITY, (d), 0)
#define DSL_DRV_WAIT_COMPLETION(a)              DSL_DRV_ThreadShutdown((a), 3000)
#define DSL_DRV_THREAD_INIT_VALID(P_THREAD_ID)  (((P_THREAD_ID)) ? (((P_THREAD_ID)->bValid == DSL_TRUE) ? DSL_TRUE : DSL_FALSE) : DSL_FALSE)
#define DSL_DRV_THREAD_DELETE(a, b)             ((void)0)
#else
/** IFXOS specific mapping*/
typedef IFXOS_ThreadParams_t     DSL_DRV_ThreadParams_t;
typedef IFXOS_ThreadFunction_t   DSL_DRV_ThreadFunc_t;
typedef IFXOS_ThreadCtrl_t       DSL_DRV_ThreadCtrl_t;

#define DSL_DRV_THREAD(a, b, c, d)        IFXOS_ThreadInit((a), (b), (c), DSL_DRV_STACKSIZE, DSL_DRV_PRIORITY, (d), 0)
#define DSL_DRV_WAIT_COMPLETION(a)        IFXOS_ThreadShutdown((a), 3000)
#define DSL_DRV_THREAD_DELETE(a, b)       ((void)0)
#endif /** #ifndef INCLUDE_DSL_CPE_API_IFXOS_SUPPORT*/

#define DSL_DRV_CRLF "\n"



#ifdef __cplusplus
}
#endif

#endif
