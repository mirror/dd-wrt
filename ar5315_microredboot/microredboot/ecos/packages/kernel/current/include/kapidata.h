#ifndef CYGONCE_KERNEL_KAPIDATA_H
#define CYGONCE_KERNEL_KAPIDATA_H

/*=============================================================================
//
//      kapidata.h
//
//      Native API data structures
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1998-03-13
// Purpose:     Native API data structures
// Description: This file defines the structures used in the native API. The
//              sizes of these structures are dependent on the system
//              configuration and must be kept in step with their real
//              counterparts in the C++ headers.
//              IMPORTANT: It is NOT guaranteed that the fields of these
//              structures correspond to the equivalent fields in the
//              C++ classes they shadow.
//
//              One oddity with this file is that the way many of the "mirror"
//              classes are defined with macros. The resulting structures
//              then have a "flat" layout, rather than just declaring a
//              member structure directly in the structure. The reason for
//              this is that as of GCC 3.x, the C++ compiler will optimise
//              classes by removing padding and reusing it for subsequent
//              members defined in a derived class. This affects some targets
//              (including PowerPC and MIPS at least) when a C++ base class
//              includes a long long. By instead arranging for the C structure
//              to just list all the members directly, the compiler will then
//              behave the same for the C structures as the C++ classes.
//
//              This means that care has to be taken to follow the same
//              methodology if new stuff is added to this file. Even if
//              it doesn't contain long longs for your target, it may for
//              others, depending on HAL definitions.
//
// Usage:       included by kapi.h
//
//####DESCRIPTIONEND####
//
//==========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/kernel.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_intr.h>           // exception defines

/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/

#ifndef CYGNUM_KERNEL_SCHED_BITMAP_SIZE
#if defined(CYGSEM_KERNEL_SCHED_MLQUEUE)
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE 32
#elif defined(CYGSEM_KERNEL_SCHED_BITMAP)
#define CYGNUM_KERNEL_SCHED_BITMAP_SIZE 32
#endif
#endif

#if CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 8
typedef cyg_ucount8 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 16
typedef cyg_ucount16 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 32
typedef cyg_ucount32 cyg_sched_bitmap;
#elif CYGNUM_KERNEL_SCHED_BITMAP_SIZE <= 64
typedef cyg_ucount64 cyg_sched_bitmap;
#else
#error Bitmaps greater than 64 bits not currently allowed
#endif

typedef struct 
{
#if defined(CYGSEM_KERNEL_SCHED_BITMAP)

    cyg_sched_bitmap map;
    
#elif defined(CYGSEM_KERNEL_SCHED_MLQUEUE)

    cyg_thread *queue;

#elif defined(CYGSEM_KERNEL_SCHED_LOTTERY)

    cyg_thread *queue;

#else

#error Undefined scheduler type
    
#endif    
} cyg_threadqueue;
    
/*---------------------------------------------------------------------------*/

struct cyg_interrupt
{
    cyg_vector_t        vector;
    cyg_priority_t      priority;
    cyg_ISR_t           *isr;
    cyg_DSR_t           *dsr;
    CYG_ADDRWORD        data;

#ifdef CYGIMP_KERNEL_INTERRUPTS_DSRS_LIST
    cyg_ucount32        dsr_count;
    cyg_interrupt       *next_dsr;
#endif
#ifdef CYGIMP_KERNEL_INTERRUPTS_CHAIN
    cyg_interrupt       *next;
#endif
};


/*---------------------------------------------------------------------------*/


#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)
# define CYG_COUNTER_ALARM_LIST_MEMBER \
    cyg_alarm           *alarm_list;
#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)
# define CYG_COUNTER_ALARM_LIST_MEMBER \
    cyg_alarm           *alarm_list[CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE];
#else
# define CYG_COUNTER_ALARM_LIST_MEMBER
#endif

#define CYG_COUNTER_MEMBERS              \
    CYG_COUNTER_ALARM_LIST_MEMBER        \
    cyg_tick_count_t    counter;         \
    cyg_uint32          increment;

struct cyg_counter
{
    CYG_COUNTER_MEMBERS
};

/*---------------------------------------------------------------------------*/

struct cyg_clock
{
    CYG_COUNTER_MEMBERS
    CYG_RESOLUTION_T_MEMBERS
};

/*---------------------------------------------------------------------------*/


#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST) ||  \
    defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)      
# define CYG_ALARM_LIST_MEMBERS                     \
    cyg_alarm           *next;                      \
    cyg_alarm           *prev;
#else 
# define CYG_ALARM_LIST_MEMBERS
#endif

#define CYG_ALARM_MEMBERS           \
    CYG_ALARM_LIST_MEMBERS          \
    cyg_counter         *counter;   \
    cyg_alarm_t         *alarm;     \
    CYG_ADDRWORD        data;       \
    cyg_tick_count_t    trigger;    \
    cyg_tick_count_t    interval;   \
    cyg_bool            enabled;

struct cyg_alarm
{
    CYG_ALARM_MEMBERS
};

/*---------------------------------------------------------------------------*/
/* Exception controller                                                      */

#ifdef CYGPKG_KERNEL_EXCEPTIONS

# ifdef CYGSEM_KERNEL_EXCEPTIONS_DECODE
#  define CYG_EXCEPTION_CONTROL_MEMBERS                                     \
    cyg_exception_handler_t *exception_handler[CYGNUM_HAL_EXCEPTION_COUNT]; \
    CYG_ADDRWORD            exception_data[CYGNUM_HAL_EXCEPTION_COUNT];     
# else
#  define CYG_EXCEPTION_CONTROL_MEMBERS                                \
    cyg_exception_handler_t *exception_handler; /* Handler function */ \
    CYG_ADDRWORD            exception_data;     /* Handler data */
# endif

typedef struct
{
    CYG_EXCEPTION_CONTROL_MEMBERS    
} cyg_exception_control;

#endif

/*---------------------------------------------------------------------------*/
/* Hardware Thread structure                                                 */

#ifdef CYGFUN_KERNEL_THREADS_STACK_LIMIT
# define CYG_HARDWARETHREAD_STACK_LIMIT_MEMBER \
    CYG_ADDRESS         stack_limit;    /* movable stack limit */
#else
# define CYG_HARDWARETHREAD_STACK_LIMIT_MEMBER
#endif

#ifdef CYGDBG_KERNEL_DEBUG_GDB_THREAD_SUPPORT
# define CYG_HARDWARETHREAD_SAVED_CONTEXT_MEMBER \
    void                *saved_context; // If non-zero, this points at a more
                                        // interesting context than stack_ptr.
#else
# define CYG_HARDWARETHREAD_SAVED_CONTEXT_MEMBER
#endif

typedef void cyg_thread_entry(CYG_ADDRWORD data);

#define CYG_HARDWARETHREAD_MEMBERS                                           \
    CYG_ADDRESS         stack_base;   /* pointer to base of stack area */    \
    cyg_uint32          stack_size;   /* size of stack area in bytes */      \
    CYG_HARDWARETHREAD_STACK_LIMIT_MEMBER                                    \
    CYG_ADDRESS         stack_ptr;    /* pointer to saved state on stack */  \
    cyg_thread_entry   *entry_point;  /* main entry point (code pointer!) */ \
    CYG_ADDRWORD        entry_data;   /* entry point argument */             \
    CYG_HARDWARETHREAD_SAVED_CONTEXT_MEMBER

typedef struct
{
    CYG_HARDWARETHREAD_MEMBERS
} cyg_hardwarethread;

/*---------------------------------------------------------------------------*/
/* Scheduler Thread structure                                                */

#ifdef CYGPKG_KERNEL_SMP_SUPPORT
# define CYG_SCHEDTHREAD_CPU_MEMBER \
    cyg_uint32          cpu;            // CPU id of cpu currently running
#else
# define CYG_SCHEDTHREAD_CPU_MEMBER
#endif

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE_ENABLE
# define CYG_SCHEDTHREAD_TIMESLICE_ENABLED_MEMBER \
    cyg_bool            timeslice_enabled; /* per-thread timeslice enable */
#else
# define CYG_SCHEDTHREAD_TIMESLICE_ENABLED_MEMBER
#endif

#if defined(CYGSEM_KERNEL_SCHED_BITMAP)
# define CYG_SCHEDTHREAD_SCHEDIMP_MEMBERS \
    cyg_priority_t      priority;       /* current thread priority */
#elif defined(CYGSEM_KERNEL_SCHED_MLQUEUE)
# define CYG_SCHEDTHREAD_SCHEDIMP_MEMBERS                                    \
    cyg_thread *next;                                                        \
    cyg_thread *prev;                                                        \
    cyg_priority_t      priority;             /* current thread priority */  \
    CYG_SCHEDTHREAD_CPU_MEMBER                                               \
    CYG_SCHEDTHREAD_TIMESLICE_ENABLED_MEMBER
#elif defined(CYGSEM_KERNEL_SCHED_LOTTERY)
# define CYG_SCHEDTHREAD_SCHEDIMP_MEMBERS                                    \
    cyg_thread *next;                                                        \
    cyg_thread *prev;                                                        \
    cyg_priority_t      priority;             /* current thread priority */  \
    cyg_priority_t      compensation_tickets; /* sleep compensation */
#else
# error Undefined scheduler type
#endif    

#ifndef CYGSEM_KERNEL_SCHED_ASR_GLOBAL
#  define CYG_SCHEDTHREAD_ASR_NONGLOBAL_MEMBER \
    void              (*asr)(CYG_ADDRWORD);   // ASR function
#else
#  define CYG_SCHEDTHREAD_ASR_NONGLOBAL_MEMBER
#endif

#ifndef CYGSEM_KERNEL_SCHED_ASR_DATA_GLOBAL
#  define CYG_SCHEDTHREAD_ASR_DATA_NONGLOBAL_MEMBER \
    CYG_ADDRWORD        asr_data;       // ASR data pointer
#else
#  define CYG_SCHEDTHREAD_ASR_DATA_NONGLOBAL_MEMBER
#endif

#ifdef CYGSEM_KERNEL_SCHED_ASR_SUPPORT
# define CYG_SCHEDTHREAD_ASR_MEMBER                                         \
    volatile cyg_ucount32 asr_inhibit; /* If true, blocks calls to ASRs */  \
    volatile cyg_bool     asr_pending; /* If true, this thread's ASR    */  \
                                       /* should be called. */              \
    CYG_SCHEDTHREAD_ASR_NONGLOBAL_MEMBER                                    \
    CYG_SCHEDTHREAD_ASR_DATA_NONGLOBAL_MEMBER                             
#else
# define CYG_SCHEDTHREAD_ASR_MEMBER
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_SIMPLE
# define CYG_SCHEDTHREAD_MUTEX_INV_PROTO_SIMPLE_MEMBERS \
    cyg_priority_t      original_priority;              \
    cyg_bool            priority_inherited;
#else
# define CYG_SCHEDTHREAD_MUTEX_INV_PROTO_SIMPLE_MEMBERS
#endif

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL
# define CYG_SCHEDTHREAD_MUTEX_INV_PROTO_MEMBERS   \
    cyg_count32         mutex_count;               \
    CYG_SCHEDTHREAD_MUTEX_INV_PROTO_SIMPLE_MEMBERS
#else
# define CYG_SCHEDTHREAD_MUTEX_INV_PROTO_MEMBERS
#endif

#define CYG_SCHEDTHREAD_MEMBERS               \
    CYG_SCHEDTHREAD_SCHEDIMP_MEMBERS          \
    cyg_threadqueue     *queue;               \
    CYG_SCHEDTHREAD_ASR_MEMBER                \
    CYG_SCHEDTHREAD_MUTEX_INV_PROTO_MEMBERS

    
typedef struct 
{
    CYG_SCHEDTHREAD_MEMBERS
} cyg_schedthread;

/* This compiler version test is required because the C++ ABI changed in
   GCC v3.x and GCC could now reuse "spare" space from base classes in derived
   classes, and in C++ land, cyg_alarm is a base class of cyg_threadtimer.
*/
#if defined(__GNUC__) && (__GNUC__ < 3)
#define CYG_THREADTIMER_MEMBERS \
    cyg_alarm           alarm;  \
    cyg_thread          *thread;
#else
#define CYG_THREADTIMER_MEMBERS \
    CYG_ALARM_MEMBERS           \
    cyg_thread          *thread;
#endif

/*---------------------------------------------------------------------------*/
/* Thread structure                                                          */

typedef struct 
{
    CYG_THREADTIMER_MEMBERS
} cyg_threadtimer;


typedef enum
{
    CYG_REASON_NONE,
    CYG_REASON_WAIT,
    CYG_REASON_DELAY,
    CYG_REASON_TIMEOUT,
    CYG_REASON_BREAK,
    CYG_REASON_DESTRUCT,
    CYG_REASON_EXIT,
    CYG_REASON_DONE
} cyg_reason_t;

#if defined(CYGPKG_KERNEL_EXCEPTIONS) && !defined(CYGSEM_KERNEL_EXCEPTIONS_GLOBAL)
# define CYG_THREAD_EXCEPTION_CONTROL_MEMBER \
    cyg_exception_control       exception_control;
#else
# define CYG_THREAD_EXCEPTION_CONTROL_MEMBER
#endif

#ifdef CYGFUN_KERNEL_THREADS_TIMER
# define CYG_THREAD_TIMER_MEMBER \
    cyg_threadtimer     timer;
#else
# define CYG_THREAD_TIMER_MEMBER
#endif

#ifdef CYGVAR_KERNEL_THREADS_DATA
# define CYG_THREAD_THREAD_DATA_MEMBER \
    CYG_ADDRWORD        thread_data[CYGNUM_KERNEL_THREADS_DATA_MAX];
#else
# define CYG_THREAD_THREAD_DATA_MEMBER
#endif

#ifdef CYGVAR_KERNEL_THREADS_NAME
# define CYG_THREAD_NAME_MEMBER \
    char                *name;
#else
# define CYG_THREAD_NAME_MEMBER
#endif

#ifdef CYGVAR_KERNEL_THREADS_LIST
# define CYG_THREAD_LIST_NEXT_MEMBER \
    cyg_thread          *list_next;
#else
# define CYG_THREAD_LIST_NEXT_MEMBER
#endif



#ifdef CYGSEM_KERNEL_THREADS_DESTRUCTORS_PER_THREAD
struct Cyg_Destructor_Entry {
    cyg_thread_destructor_fn fn;
    cyg_addrword_t data;
};
# define CYG_THREAD_DESTRUCTORS_MEMBER \
   struct Cyg_Destructor_Entry destructors[ CYGNUM_KERNEL_THREADS_DESTRUCTORS ];
#else
# define CYG_THREAD_DESTRUCTORS_MEMBER
#endif


#define CYG_THREAD_MEMBERS                        \
    CYG_HARDWARETHREAD_MEMBERS                    \
    CYG_SCHEDTHREAD_MEMBERS                       \
                                                  \
    cyg_uint32                  state;            \
    cyg_ucount32                suspend_count;    \
    cyg_ucount32                wakeup_count;     \
    CYG_ADDRWORD                wait_info;        \
    cyg_uint16                  unique_id;        \
                                                  \
    CYG_THREAD_EXCEPTION_CONTROL_MEMBER           \
    CYG_THREAD_TIMER_MEMBER                       \
                                                  \
    cyg_reason_t        sleep_reason;             \
    cyg_reason_t        wake_reason;              \
                                                  \
    CYG_THREAD_THREAD_DATA_MEMBER                 \
    CYG_THREAD_DESTRUCTORS_MEMBER                 \
    CYG_THREAD_NAME_MEMBER                        \
    CYG_THREAD_LIST_NEXT_MEMBER                   


struct cyg_thread
{
    CYG_THREAD_MEMBERS
};

/*---------------------------------------------------------------------------*/

struct cyg_mbox
{
    cyg_count32         base;           /* index of first used slot          */
    cyg_count32         count;          /* count of used slots               */
    cyg_threadqueue     get_threadq;    /* Queue of waiting threads          */
#ifdef CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
    cyg_threadqueue     put_threadq;    /* Queue of waiting threads          */
#endif
    void *              itemqueue[ CYGNUM_KERNEL_SYNCH_MBOX_QUEUE_SIZE ];
};

/*---------------------------------------------------------------------------*/

struct cyg_sem_t
{
    cyg_count32         count;          /* The semaphore count          */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */    
};

/*---------------------------------------------------------------------------*/

struct cyg_flag_t
{
    cyg_flag_value_t    value;          /* The flag value               */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */    
};

/*---------------------------------------------------------------------------*/

typedef enum
{
    CYG_MUTEX_PROTOCOL_NONE,
    CYG_MUTEX_PROTOCOL_INHERIT,
    CYG_MUTEX_PROTOCOL_CEILING
} cyg_mutex_protocol_t;

struct cyg_mutex_t
{
    cyg_atomic          locked;         /* true if locked               */
    cyg_thread          *owner;         /* Current locking thread       */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */

#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_DYNAMIC
    cyg_mutex_protocol_t protocol;       /* this mutex's protocol        */
#endif    
#ifdef CYGSEM_KERNEL_SYNCH_MUTEX_PRIORITY_INVERSION_PROTOCOL_CEILING
    cyg_priority_t      ceiling;        /* mutex priority ceiling       */
#endif
    
};

/*---------------------------------------------------------------------------*/

struct cyg_cond_t
{
    cyg_mutex_t         *mutex;         /* Associated mutex             */
    cyg_threadqueue     queue;          /* Queue of waiting threads     */
};

/*------------------------------------------------------------------------*/

struct cyg_spinlock_t
{
    cyg_uint32          lock;           /* lock word                     */
};

/*------------------------------------------------------------------------*/

/* Memory allocator types now come from the "memalloc" package which is   */
/* where the implementation lives.                                        */

#ifdef CYGPKG_MEMALLOC
# include <cyg/memalloc/kapidata.h>
#endif

#ifdef __cplusplus
}
#endif

/*---------------------------------------------------------------------------*/
/* EOF kapidata.h                                                            */
#endif /* CYGONCE_KERNEL_KAPIDATA_H */
