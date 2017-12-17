/******************************************************************************

                               Copyright (c) 2009
                            Infineon Technologies AG
                     Am Campeon 1-12; 81726 Munich, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_DSL_CPE_OS_RTEMS_H
#define _DRV_DSL_CPE_OS_RTEMS_H

#ifdef __cplusplus
   extern "C" {
#endif

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

#include "xapi.h"    // for  xtm_wkafter

#ifndef DSL_DRV_STACKSIZE
#define DSL_DRV_STACKSIZE (8192)
#endif

#ifndef DSL_DRV_PRIORITY
#define DSL_DRV_PRIORITY  64
#endif

#define INCLUDE_DSL_DRV_STATIC_LINKED_FIRMWARE

typedef IFX_int_t                DSL_DRV_size_t;
typedef IFX_uint32_t             DSL_DRV_TimeVal_t;
typedef IFXOS_ThreadFunction_t   DSL_DRV_ThreadFunc_t;
typedef IFXOS_ThreadParams_t     DSL_DRV_ThreadParams_t;
typedef IFXOS_drvSelectQueue_t   DSL_DRV_WaitQueue_t;
typedef IFXOS_event_t            DSL_DRV_Event_t;
typedef IFXOS_lock_t             DSL_DRV_Mutex_t;
typedef IFXOS_ThreadCtrl_t       DSL_DRV_ThreadCtrl_t;

typedef DSL_int_t (*DSL_ThreadFunc_t)(DSL_void_t *pArg);

DSL_uint32_t DSL_DRV_GetTime(DSL_uint32_t nOffset);
DSL_uint32_t DSL_DRV_SysTimeGet(DSL_uint32_t nOffset);

#define _IOC_TYPE(x)                      (((x)>>8) & 0xFF)

#define DSL_DRV_ElapsedTimeMSecGet(t)     IFXOS_ElapsedTimeMSecGet(0)

#define DSL_WAIT(ms)                      xtm_wkafter(ms)

#define DSL_DRV_TimeMSecGet()             IFXOS_ElapsedTimeMSecGet(0)
#define DSL_DRV_TimeSecGet(t)             (t)

#define DSL_DRV_INIT_WAKELIST(name,queue) IFXOS_DrvSelectQueueInit(&(queue))
#define DSL_DRV_WAKEUP_WAKELIST(ev)       IFXOS_DrvSelectQueueWakeUp(&(ev), IFXOS_DRV_SEL_WAKEUP_TYPE_RD)


#define DSL_DRV_INIT_EVENT(name, ev)      IFXOS_EventInit(&(ev))
#define DSL_DRV_WAKEUP_EVENT(ev)          IFXOS_EventWakeUp(&(ev))


#define DSL_DRV_WAIT_EVENT(ev)            IFXOS_EventWait(&(ev), 0xFFFFFFFF, IFX_NULL)
#define DSL_DRV_WAIT_EVENT_TIMEOUT(ev, t) IFXOS_EventWait(&(ev), (t), IFX_NULL)

// SchS: compare with define in drv_dsl_cpe_os_linux.h
#define interruptible_sleep_on_timeout(ev,t)    IFXOS_EventWait(ev, (t)* 1000 / HZ, IFX_NULL)
#define interruptible_sleep_on(ev)              IFXOS_EventWait(ev, 0xFFFFFFFF, IFX_NULL)
#define wake_up_interruptible                   IFXOS_EventWakeUp
//
//#define DSL_DRV_WAIT_EVENT_TIMEOUT(ev,t)     interruptible_sleep_on_timeout(&(ev), (t) * HZ / 1000)   IFXOS_EventWait(&(ev), (t), IFX_NULL)

#define DSL_DRV_MUTEX_INIT(mutex)         IFXOS_LockInit(&(mutex))
#define DSL_DRV_MUTEX_LOCK(mutex)         IFXOS_LockGet(&(mutex))
#define DSL_DRV_MUTEX_UNLOCK(mutex)       IFXOS_LockRelease(&(mutex))

#define DSL_DRV_Phy2VirtMap               IFXOS_Phy2VirtMap
#define DSL_DRV_Phy2VirtUnmap             IFXOS_Phy2VirtUnmap

#define DSL_DRV_THREAD(a, b, c, d)        IFXOS_ThreadInit((a), (b), (c), DSL_DRV_STACKSIZE, DSL_DRV_PRIORITY, (d), 0)
#define DSL_DRV_WAIT_COMPLETION(a)        IFXOS_ThreadShutdown((a), 3000)
#define DSL_DRV_THREAD_DELETE(a, b)       IFXOS_ThreadDelete(&(a), b)

#define DSL_DRV_CRLF "\n"
#define DSL_CRLF DSL_DRV_CRLF

#define DSL_DRV_OS_ModUseCountIncrement()
#define DSL_DRV_OS_ModUseCountDecrement()

#define DSL_Le2Cpu(le) le16_to_cpu(le)

/*weirong not needed
#define DSL_VMalloc DSL_DRV_VMalloc
#define DSL_VFree DSL_DRV_VFree
#define DSL_MemSet DSL_DRV_MemSet */

#ifdef __cplusplus
}
#endif

#endif /** _DRV_DSL_CPE_OS_RTEMS_H*/
