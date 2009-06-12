#ifndef CYGONCE_KERNEL_KAPI_H
#define CYGONCE_KERNEL_KAPI_H

/*==========================================================================
//
//      kapi.h
//
//      Native API for Kernel
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
// Copyright (C) 2002 Nick Garnett
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg, dsm
// Contributors:        nickg
// Date:        1998-03-02
// Purpose:     Native API for Kernel
// Description: This file describes the native API for using the kernel.
//              It is essentially a set of C wrappers for the C++ class
//              member functions.
// Usage:       #include <cyg/kernel/kapi.h>
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/kernel.h>

#ifdef CYGFUN_KERNEL_API_C
#include <cyg/infra/cyg_type.h>

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/* The following are derived types, they may have different                  */
/* definitions from these depending on configuration.                        */

typedef CYG_ADDRWORD   cyg_addrword_t;      /* May hold pointer or word      */
typedef cyg_addrword_t cyg_handle_t;        /* Object handle                 */
typedef cyg_uint32     cyg_priority_t;      /* type for priorities           */
typedef cyg_int32      cyg_code_t;          /* type for various codes        */
typedef cyg_uint32     cyg_vector_t;        /* Interrupt vector id           */
typedef cyg_uint32     cyg_cpu_t;           /* CPU id type                   */

typedef cyg_uint64 cyg_tick_count_t;

typedef int cyg_bool_t;

/* Exception handler function definition                                     */
typedef void cyg_exception_handler_t(
    cyg_addrword_t data,
    cyg_code_t   exception_number,
    cyg_addrword_t info
);

/*---------------------------------------------------------------------------*/
struct cyg_thread;
typedef struct cyg_thread cyg_thread;

struct cyg_interrupt;
typedef struct cyg_interrupt cyg_interrupt;

struct cyg_counter;
typedef struct cyg_counter cyg_counter;

struct cyg_clock;
typedef struct cyg_clock cyg_clock;

struct cyg_alarm;
typedef struct cyg_alarm cyg_alarm;

struct cyg_mbox;
typedef struct cyg_mbox cyg_mbox;

struct cyg_sem_t;
typedef struct cyg_sem_t cyg_sem_t;

struct cyg_flag_t;
typedef struct cyg_flag_t cyg_flag_t;

struct cyg_mutex_t;
typedef struct cyg_mutex_t cyg_mutex_t;

struct cyg_cond_t;
typedef struct cyg_cond_t cyg_cond_t;

struct cyg_spinlock_t;
typedef struct cyg_spinlock_t cyg_spinlock_t;

/*---------------------------------------------------------------------------*/
/* Scheduler operations */

/* Starts scheduler with created threads.  Never returns. */
void cyg_scheduler_start(void) __THROW CYGBLD_ATTRIB_NORET;

/* Lock and unlock the scheduler. When the scheduler is   */
/* locked thread preemption is disabled.                  */
void cyg_scheduler_lock(void) __THROW;

void cyg_scheduler_unlock(void) __THROW;

/* Just like 'cyg_scheduler_lock()', but never take the lock higher than 1  */
/* Thus this call is safe even if the scheduler is already locked and a     */
/* subsequent call to 'cyg_scheduler_unlock()' will completely unlock.      */
void cyg_scheduler_safe_lock(void) __THROW;
    
/* Read the scheduler lock value. */
cyg_ucount32 cyg_scheduler_read_lock(void) __THROW;

/*---------------------------------------------------------------------------*/
/* Thread operations */

typedef void cyg_thread_entry_t(cyg_addrword_t);

void cyg_thread_create(
    cyg_addrword_t      sched_info,             /* scheduling info (eg pri)  */
    cyg_thread_entry_t  *entry,                 /* entry point function      */
    cyg_addrword_t      entry_data,             /* entry data                */
    char                *name,                  /* optional thread name      */
    void                *stack_base,            /* stack base, NULL = alloc  */
    cyg_ucount32        stack_size,             /* stack size, 0 = default   */
    cyg_handle_t        *handle,                /* returned thread handle    */
    cyg_thread          *thread                 /* put thread here           */
) __THROW;
    
void cyg_thread_exit(void) __THROW;

/* It may be necessary to arrange for the victim to run for it to disappear */
cyg_bool_t cyg_thread_delete(cyg_handle_t thread) __THROW; /* false if NOT deleted */

void cyg_thread_suspend(cyg_handle_t thread) __THROW;

void cyg_thread_resume(cyg_handle_t thread) __THROW;

void cyg_thread_kill(cyg_handle_t thread) __THROW;

void cyg_thread_release(cyg_handle_t thread) __THROW;    
    
void cyg_thread_yield(void) __THROW;

cyg_handle_t cyg_thread_self(void) __THROW;

cyg_handle_t cyg_thread_idle_thread(void) __THROW;

/* Priority manipulation */

void cyg_thread_set_priority(cyg_handle_t thread, cyg_priority_t priority ) __THROW;

cyg_priority_t cyg_thread_get_priority(cyg_handle_t thread) __THROW;              
cyg_priority_t cyg_thread_get_current_priority(cyg_handle_t thread) __THROW; 

/* Deadline scheduling control (optional) */

void cyg_thread_deadline_wait( 
    cyg_tick_count_t    start_time,             /* abs earliest start time   */
    cyg_tick_count_t    run_time,               /* worst case execution time */
    cyg_tick_count_t    deadline                /* absolute deadline         */
) __THROW; 

void cyg_thread_delay(cyg_tick_count_t delay) __THROW;

/* Stack information */
cyg_addrword_t cyg_thread_get_stack_base(cyg_handle_t thread) __THROW;

cyg_uint32 cyg_thread_get_stack_size(cyg_handle_t thread) __THROW;

#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
cyg_uint32 cyg_thread_measure_stack_usage(cyg_handle_t thread) __THROW;
#endif

/*---------------------------------------------------------------------------*/
/* Thread enumeration and information                                        */
    
typedef struct
{
    cyg_handle_t        handle;
    cyg_uint16          id;
    cyg_uint32          state;
    char                *name;
    cyg_priority_t      set_pri;
    cyg_priority_t      cur_pri;
    cyg_addrword_t      stack_base;
    cyg_uint32          stack_size;
    cyg_uint32          stack_used;
} cyg_thread_info;
    
cyg_bool_t cyg_thread_get_next( cyg_handle_t *thread, cyg_uint16 *id ) __THROW;

cyg_bool_t cyg_thread_get_info( cyg_handle_t thread,
                                cyg_uint16 id,
                                cyg_thread_info *info ) __THROW;

cyg_uint16 cyg_thread_get_id( cyg_handle_t thread ) __THROW;

cyg_handle_t cyg_thread_find( cyg_uint16 id ) __THROW;
    
/*---------------------------------------------------------------------------*/
/* Per-thread Data                                                           */

#ifdef CYGVAR_KERNEL_THREADS_DATA

cyg_ucount32 cyg_thread_new_data_index(void) __THROW;

void cyg_thread_free_data_index(cyg_ucount32 index) __THROW;

CYG_ADDRWORD cyg_thread_get_data(cyg_ucount32 index) __THROW;

CYG_ADDRWORD *cyg_thread_get_data_ptr(cyg_ucount32 index) __THROW;

void cyg_thread_set_data(cyg_ucount32 index, CYG_ADDRWORD data) __THROW;

#endif
    
/*---------------------------------------------------------------------------*/
/* Thread destructors                                                        */

#ifdef CYGPKG_KERNEL_THREADS_DESTRUCTORS

typedef void (*cyg_thread_destructor_fn)(cyg_addrword_t);

cyg_bool_t cyg_thread_add_destructor( cyg_thread_destructor_fn fn,
                                      cyg_addrword_t data ) __THROW;
cyg_bool_t cyg_thread_rem_destructor( cyg_thread_destructor_fn fn,
                                      cyg_addrword_t data ) __THROW;
#endif
    
/*---------------------------------------------------------------------------*/
/* Exception handling.                                                       */

/* Replace current exception handler, this may apply to either the           */
/* current thread, or to a global exception handler. The exception           */
/* number may be ignored, or used to specify a particular handler.           */

void cyg_exception_set_handler(
    cyg_code_t                  exception_number,
    cyg_exception_handler_t     *new_handler,
    cyg_addrword_t                new_data,
    cyg_exception_handler_t     **old_handler,
    cyg_addrword_t                *old_data
) __THROW;

/* Clear exception hander to default value                                   */
void cyg_exception_clear_handler(
    cyg_code_t                  exception_number
) __THROW;
    
/* Invoke exception handler                                                  */
void cyg_exception_call_handler(
    cyg_handle_t                thread,
    cyg_code_t                  exception_number,
    cyg_addrword_t              exception_info
) __THROW;


/*---------------------------------------------------------------------------*/
/* Interrupt handling                                                        */
typedef void            cyg_VSR_t(void);
typedef cyg_uint32      cyg_ISR_t(cyg_vector_t vector, cyg_addrword_t data);
typedef void            cyg_DSR_t( cyg_vector_t vector,
                                   cyg_ucount32 count,
                                   cyg_addrword_t data);


enum cyg_ISR_results
{
    CYG_ISR_HANDLED  = 1,               /* Interrupt was handled             */
    CYG_ISR_CALL_DSR = 2                /* Schedule DSR                      */
};

void cyg_interrupt_create(
    cyg_vector_t        vector,         /* Vector to attach to               */
    cyg_priority_t      priority,       /* Queue priority                    */
    cyg_addrword_t      data,           /* Data pointer                      */
    cyg_ISR_t           *isr,           /* Interrupt Service Routine         */
    cyg_DSR_t           *dsr,           /* Deferred Service Routine          */
    cyg_handle_t        *handle,        /* returned handle                   */
    cyg_interrupt       *intr           /* put interrupt here                */
) __THROW;

void cyg_interrupt_delete( cyg_handle_t interrupt ) __THROW;

void cyg_interrupt_attach( cyg_handle_t interrupt ) __THROW;

void cyg_interrupt_detach( cyg_handle_t interrupt ) __THROW;
    
/* VSR manipulation */

void cyg_interrupt_get_vsr(
    cyg_vector_t        vector,         /* vector to get                     */
    cyg_VSR_t           **vsr           /* vsr got                           */
) __THROW;

void cyg_interrupt_set_vsr(
    cyg_vector_t        vector,         /* vector to set                     */
    cyg_VSR_t           *vsr            /* vsr to set                        */
) __THROW;

/* CPU level interrupt mask                                                  */
void cyg_interrupt_disable(void) __THROW;

void cyg_interrupt_enable(void) __THROW;

/* Interrupt controller access                                               */
void cyg_interrupt_mask(cyg_vector_t vector) __THROW;
void cyg_interrupt_mask_intunsafe(cyg_vector_t vector) __THROW;

void cyg_interrupt_unmask(cyg_vector_t vector) __THROW;
void cyg_interrupt_unmask_intunsafe(cyg_vector_t vector) __THROW;

void cyg_interrupt_acknowledge(cyg_vector_t vector) __THROW;

void cyg_interrupt_configure(
    cyg_vector_t        vector,         /* vector to configure               */
    cyg_bool_t          level,          /* level or edge triggered           */
    cyg_bool_t          up              /* rising/faling edge, high/low level*/
) __THROW;

void cyg_interrupt_set_cpu(
    cyg_vector_t        vector,         /* vector to control                 */
    cyg_cpu_t           cpu             /* CPU to set                        */
) __THROW;

cyg_cpu_t cyg_interrupt_get_cpu(
    cyg_vector_t        vector          /* vector to control                 */
) __THROW;
    
/*---------------------------------------------------------------------------*/
/* Counters, Clocks and Alarms                                               */

void cyg_counter_create(
    cyg_handle_t        *handle,        /* returned counter handle           */
    cyg_counter         *counter        /* put counter here                  */
) __THROW;

void cyg_counter_delete(cyg_handle_t counter) __THROW;

/* Return current value of counter                                           */
cyg_tick_count_t cyg_counter_current_value(cyg_handle_t counter) __THROW;

/* Set new current value                                                     */
void cyg_counter_set_value(
    cyg_handle_t        counter,
    cyg_tick_count_t new_value
) __THROW;

/* Advance counter by one tick                                               */
void cyg_counter_tick(cyg_handle_t counter) __THROW;

/* Advance counter by multiple ticks                                         */
void cyg_counter_multi_tick(cyg_handle_t counter, cyg_tick_count_t _ticks) __THROW;


#define CYG_RESOLUTION_T_MEMBERS  \
    cyg_uint32  dividend;         \
    cyg_uint32  divisor;

typedef struct 
{
    CYG_RESOLUTION_T_MEMBERS
} cyg_resolution_t;

/* Create a clock object                */
void cyg_clock_create(
    cyg_resolution_t    resolution,     /* Initial resolution                */
    cyg_handle_t        *handle,        /* Returned clock handle             */
    cyg_clock           *clock          /* put clock here                    */    
) __THROW;

void cyg_clock_delete(cyg_handle_t clock) __THROW;

/* convert a clock handle to a counter handle so we can use the              */
/* counter API on it.                                                        */
void cyg_clock_to_counter(
    cyg_handle_t        clock,
    cyg_handle_t        *counter
) __THROW;

void cyg_clock_set_resolution(
    cyg_handle_t        clock,
    cyg_resolution_t    resolution      /* New resolution                    */
) __THROW;

cyg_resolution_t cyg_clock_get_resolution(cyg_handle_t clock) __THROW;

/* handle of real time clock                                                 */
cyg_handle_t cyg_real_time_clock(void) __THROW;

/* returns value of real time clock's counter.
   This is the same as:
   (cyg_clock_to_counter(cyg_real_time_clock(), &h),
    cyg_counter_current_value(h))                                            */
cyg_tick_count_t cyg_current_time(void) __THROW;

/* Alarm handler function                                                    */
typedef void cyg_alarm_t(cyg_handle_t alarm, cyg_addrword_t data);

void cyg_alarm_create(
    cyg_handle_t        counter,        /* Attached to this counter          */
    cyg_alarm_t         *alarmfn,       /* Call-back function                */
    cyg_addrword_t      data,           /* Call-back data                    */
    cyg_handle_t        *handle,        /* Returned alarm object             */
    cyg_alarm           *alarm          /* put alarm here                    */    
) __THROW;

/* Disable alarm, detach from counter and invalidate handles                 */
void cyg_alarm_delete( cyg_handle_t alarm) __THROW;

void cyg_alarm_initialize(
    cyg_handle_t        alarm,
    cyg_tick_count_t    trigger,        /* Absolute trigger time             */
    cyg_tick_count_t    interval        /* Relative retrigger interval       */
) __THROW;

void cyg_alarm_get_times(
    cyg_handle_t        alarm,
    cyg_tick_count_t    *trigger,       /* Next trigger time                 */
    cyg_tick_count_t    *interval       /* Current interval                  */
) __THROW;

void cyg_alarm_enable( cyg_handle_t alarm ) __THROW;

void cyg_alarm_disable( cyg_handle_t alarm ) __THROW;

/*---------------------------------------------------------------------------*/
/* Mail boxes                                                                */
void cyg_mbox_create(
    cyg_handle_t        *handle,
    cyg_mbox            *mbox
) __THROW;

void cyg_mbox_delete(cyg_handle_t mbox) __THROW;

void *cyg_mbox_get(cyg_handle_t mbox) __THROW;

#ifdef CYGFUN_KERNEL_THREADS_TIMER
void *cyg_mbox_timed_get(
    cyg_handle_t mbox,
    cyg_tick_count_t abstime
    ) __THROW;
#endif

void *cyg_mbox_tryget(cyg_handle_t mbox) __THROW;

void *cyg_mbox_peek_item(cyg_handle_t mbox) __THROW;

#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
cyg_bool_t cyg_mbox_put(cyg_handle_t mbox, void *item) __THROW;
#ifdef CYGFUN_KERNEL_THREADS_TIMER
cyg_bool_t cyg_mbox_timed_put(
    cyg_handle_t mbox,
    void *item,
    cyg_tick_count_t abstime
    ) __THROW;
#endif
#endif

cyg_bool_t cyg_mbox_tryput(cyg_handle_t mbox, void *item) __THROW;

cyg_count32 cyg_mbox_peek(cyg_handle_t mbox) __THROW;

cyg_bool_t cyg_mbox_waiting_to_get(cyg_handle_t mbox) __THROW;

cyg_bool_t cyg_mbox_waiting_to_put(cyg_handle_t mbox) __THROW;


/*-----------------------------------------------------------------------*/
/* Memory pools                                                          */

/* These definitions are found in the "memalloc" package as this is      */
/* where the implementation lives.                                       */

#ifdef CYGPKG_MEMALLOC
# include <cyg/memalloc/kapi.h>
#endif

/*---------------------------------------------------------------------------*/
/* Semaphores                                                                */

void      cyg_semaphore_init(
    cyg_sem_t           *sem,            /* Semaphore to init                */
    cyg_count32         val              /* Initial semaphore value          */
) __THROW;

void cyg_semaphore_destroy( cyg_sem_t *sem ) __THROW;

cyg_bool_t cyg_semaphore_wait( cyg_sem_t *sem ) __THROW;

#ifdef CYGFUN_KERNEL_THREADS_TIMER
cyg_bool_t cyg_semaphore_timed_wait(
    cyg_sem_t          *sem,
    cyg_tick_count_t   abstime
    ) __THROW;
#endif

cyg_bool_t cyg_semaphore_trywait( cyg_sem_t *sem ) __THROW;

void cyg_semaphore_post( cyg_sem_t *sem ) __THROW;

void cyg_semaphore_peek( cyg_sem_t *sem, cyg_count32 *val ) __THROW;

/*---------------------------------------------------------------------------*/
/* Flags                                                                     */

typedef cyg_uint32 cyg_flag_value_t;
typedef cyg_uint8  cyg_flag_mode_t;
#define CYG_FLAG_WAITMODE_AND ((cyg_flag_mode_t)0) /* all bits must be set */
#define CYG_FLAG_WAITMODE_OR  ((cyg_flag_mode_t)2) /* any bit must be set  */
#define CYG_FLAG_WAITMODE_CLR ((cyg_flag_mode_t)1) /* clear when satisfied */

void cyg_flag_init(
    cyg_flag_t        *flag             /* Flag to init                      */
) __THROW;

void cyg_flag_destroy( cyg_flag_t *flag ) __THROW;

/* bitwise-or in the bits in value; awaken any waiting tasks whose
   condition is now satisfied */
void cyg_flag_setbits( cyg_flag_t *flag, cyg_flag_value_t value) __THROW;

/* bitwise-and with the the bits in value; this clears the bits which
   are not set in value.  No waiting task can be awoken. */
void cyg_flag_maskbits( cyg_flag_t *flag, cyg_flag_value_t value) __THROW;

/* wait for the flag value to match the pattern, according to the mode.
   If mode includes CLR, set the flag value to zero when
   our pattern is matched.  The return value is that which matched
   the request, or zero for an error/timeout return.
   Value must not itself be zero. */
cyg_flag_value_t cyg_flag_wait( cyg_flag_t        *flag,
                                cyg_flag_value_t   pattern, 
                                cyg_flag_mode_t    mode ) __THROW;

#ifdef CYGFUN_KERNEL_THREADS_TIMER
cyg_flag_value_t cyg_flag_timed_wait( cyg_flag_t        *flag,
                                      cyg_flag_value_t   pattern, 
                                      cyg_flag_mode_t    mode,
                                      cyg_tick_count_t   abstime ) __THROW;

#endif

cyg_flag_value_t cyg_flag_poll( cyg_flag_t         *flag,
                                cyg_flag_value_t    pattern, 
                                cyg_flag_mode_t     mode ) __THROW;

cyg_flag_value_t cyg_flag_peek( cyg_flag_t *flag ) __THROW;

cyg_bool_t cyg_flag_waiting( cyg_flag_t *flag ) __THROW;

/*---------------------------------------------------------------------------*/
/* Mutex                                                                     */

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
enum cyg_mutex_protocol
{
  CYG_MUTEX_NONE = 0,                   // no inversion protocol
  CYG_MUTEX_INHERIT,                    // priority inheritance protocol
  CYG_MUTEX_CEILING                     // priority ceiling protocol
};
#endif

void cyg_mutex_init(
    cyg_mutex_t        *mutex          /* Mutex to init                      */
) __THROW;

void cyg_mutex_destroy( cyg_mutex_t *mutex ) __THROW;

cyg_bool_t cyg_mutex_lock( cyg_mutex_t *mutex ) __THROW;

cyg_bool_t cyg_mutex_trylock( cyg_mutex_t *mutex ) __THROW;

void cyg_mutex_unlock( cyg_mutex_t *mutex ) __THROW;

void cyg_mutex_release( cyg_mutex_t *mutex ) __THROW;

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING
void cyg_mutex_set_ceiling( cyg_mutex_t *mutex, cyg_priority_t priority ) __THROW;
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
void cyg_mutex_set_protocol ( cyg_mutex_t *mutex, enum cyg_mutex_protocol protocol ) __THROW;
#endif

/*---------------------------------------------------------------------------*/
/* Condition Variables                                                       */

void cyg_cond_init(
    cyg_cond_t          *cond,          /* condition variable to init        */
    cyg_mutex_t         *mutex          /* associated mutex                  */
) __THROW;

void cyg_cond_destroy( cyg_cond_t *cond ) __THROW;

cyg_bool_t cyg_cond_wait( cyg_cond_t *cond ) __THROW;

void cyg_cond_signal( cyg_cond_t *cond ) __THROW;

void cyg_cond_broadcast( cyg_cond_t *cond ) __THROW;

#ifdef CYGMFN_KERNEL_SYNCH_CONDVAR_TIMED_WAIT
cyg_bool_t cyg_cond_timed_wait(
    cyg_cond_t        *cond,
    cyg_tick_count_t  abstime
    ) __THROW;
#endif

/*---------------------------------------------------------------------------*/
/* Spinlocks                                                                 */

void cyg_spinlock_init(
    cyg_spinlock_t      *lock,          /* spinlock to initialize            */
    cyg_bool_t          locked          /* init locked or unlocked           */
) __THROW;

void cyg_spinlock_destroy( cyg_spinlock_t *lock ) __THROW;

void cyg_spinlock_spin( cyg_spinlock_t *lock ) __THROW;

void cyg_spinlock_clear( cyg_spinlock_t *lock ) __THROW;

cyg_bool_t cyg_spinlock_try( cyg_spinlock_t *lock ) __THROW;

cyg_bool_t cyg_spinlock_test( cyg_spinlock_t *lock ) __THROW;

void cyg_spinlock_spin_intsave( cyg_spinlock_t *lock,
                                cyg_addrword_t *istate ) __THROW;

void cyg_spinlock_clear_intsave( cyg_spinlock_t *lock,
                                 cyg_addrword_t istate ) __THROW;

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/

#include <cyg/kernel/kapidata.h>

/*---------------------------------------------------------------------------*/
/* EOF kapi.h                                                                */
#endif /* CYGFUN_KERNEL_API_C   */
#endif /* CYGONCE_KERNEL_KAPI_H */
