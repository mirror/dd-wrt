//========================================================================
//
//      dbg-threads-syscall.c
//
//      Pseudo system calls for multi-threaded debug support
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, nickg
// Contributors:  Red Hat, nickg
// Date:          1998-08-25
// Purpose:       
// Description:   Pseudo system calls to bind system specific multithread
//                debug support with a ROM monitor, cygmon. We call it
//                Cygmon, but the feature lives in libstub.
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#if !defined(CYGPKG_KERNEL) && defined(CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT)

/* Only include this code if we do not have a kernel. Otherwise the kernel
 * supplies these functions for the app we are linked with.
 */

#include <cyg/hal/dbg-threads-api.h>
#include <cyg/hal/dbg-thread-syscall.h>


static dbg_syscall_func * dbg_syscall_ptr ;

static union dbg_thread_syscall_parms tcall ;

/* ----- INIT_THREADS_SYSCALL --------------------------------------- */
/* Some external bing and configuration logic knows how to setup
   the ststem calls. In the first implementation, we have used a vector
   in the secondary vector table. This functions allows us to isolate that
   sort of system specific detail. Similarly, we do not export the
   specific detail of a dbg_syscall_func.
 */


void init_threads_syscall(void * vector)
{
   dbg_syscall_ptr = vector ; /* AH, the easy compatability of the
   void pointer*/
} /* init_threads_syscall */

/* All forms of failure return 0 */
/* Whether non support, incomplete initialization, unknown thread */
static __inline__ int dbg_thread_syscall(
				     enum dbg_syscall_ids id)
{
  dbg_syscall_func f ; /* double indirect via */
  if (0 == dbg_syscall_ptr) return 0; /* dbg_syscall_ptr never init'd */
  if (0 ==(f = *dbg_syscall_ptr)) return 0 ;  /* vector not initialized */
  return (*f)(id,&tcall);
}




/* ------- INIT_THREAD_SYSCALL -------------------------------------------  */
/* Received the address of the entry in the secondary interrupt vector table */
/* This table is the interchange between the O.S. and Cygmon/libstub         */
/* This could get more complex so, I am doing it with a function
   rather than exposing the internals.
 */
void init_thread_syscall(void * vector)
{
  dbg_syscall_ptr = vector ;
}

int dbg_thread_capabilities(struct dbg_capabilities * cbp)
{
#if 0    
  tcall.cap_parms.abilities = cbp ;
  return dbg_thread_syscall(dbg_capabilities_func) ;
#else
    cbp->mask1 = has_thread_current     |
        has_thread_registers            |
        has_thread_reg_change           |
        has_thread_list                 |
        has_thread_info                 ;
    return 1 ; 
#endif  
}

int dbg_currthread(threadref * varparm)
{
  tcall.currthread_parms.ref = varparm ;
  return dbg_thread_syscall(dbg_currthread_func) ;
}


int dbg_threadlist(
		   int startflag,
		   threadref * lastthreadid,
		   threadref * next_thread
		   )
{
  tcall.threadlist_parms.startflag = startflag ;
  tcall.threadlist_parms.lastid = lastthreadid ;
  tcall.threadlist_parms.nextthreadid = next_thread ;
  return dbg_thread_syscall(dbg_threadlist_func) ;
}

int dbg_threadinfo(
		   threadref * threadid,
		   struct cygmon_thread_debug_info * info)
{
  tcall.info_parms.ref = threadid ;
  tcall.info_parms.info = info ;
  return dbg_thread_syscall(dbg_threadinfo_func) ;
}

int dbg_getthreadreg(
		     threadref * osthreadid,
		     int regcount, /* count of registers in the array */
		     void * regval)  /* fillin this array */
{
  tcall.reg_parms.thread =    osthreadid ;
  tcall.reg_parms.regcount =  regcount ;
  tcall.reg_parms.registers = regval ;
  return dbg_thread_syscall(dbg_getthreadreg_func) ;
}

int dbg_setthreadreg(
		     threadref * osthreadid, 
		     int regcount , /* number of registers */
		     void * regval)
{
  tcall.reg_parms.thread =    osthreadid ;
  tcall.reg_parms.regcount =  regcount ;
  tcall.reg_parms.registers =  regval ;
  return dbg_thread_syscall(dbg_setthreadreg_func) ;
} /* dbg_setthreadreg */

int dbg_scheduler(threadref *thread_id, int lock, int mode)
{
  tcall.scheduler_parms.thread    = thread_id;
  tcall.scheduler_parms.lock      = lock ;
  tcall.scheduler_parms.mode      = mode ;

  return dbg_thread_syscall(dbg_scheduler_func) ;
}



#if (CYG_BYTEORDER == CYG_LSBFIRST)

unsigned long swap32(unsigned long x)
{
    unsigned long r = 0;

    r |= (x>>24)&0xFF;
    r |= ((x>>16)&0xFF)<<8;
    r |= ((x>>8)&0xFF)<<16;
    r |= ((x)&0xFF)<<24;

    return r;
}

#else

#define swap32(x) ((unsigned long)(x))

#endif

int dbg_currthread_id(void)
{
    threadref ref;
    if( dbg_currthread( &ref ) )
        return (cyg_uint16)swap32(((unsigned long *)ref)[1]);
    else return 0;
}

#endif
