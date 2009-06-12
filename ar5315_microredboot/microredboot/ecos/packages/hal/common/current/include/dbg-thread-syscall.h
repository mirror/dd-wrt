//========================================================================
//
//      dbg-thread-syscall.h
//
//      Supports thread-aware debugging
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
// Description:   Supports thread-aware debugging
// Usage:         This header is not to be included by user code.
//                This file should be included only by
//                thread-syscall-relay.c and dbg-thread-demux.c
//
//####DESCRIPTIONEND####
//
//========================================================================


enum dbg_syscall_ids 
  {
    dbg_null_func ,
    dbg_capabilities_func,
    dbg_currthread_func,
    dbg_threadlist_func,
    dbg_threadinfo_func,
    dbg_getthreadreg_func,
    dbg_setthreadreg_func,
    dbg_scheduler_func,
  } ;


union dbg_thread_syscall_parms
{
  struct
  {
    struct dbg_capabilities * abilities ;
  } cap_parms ;

  struct
  {
    threadref * ref ;
  } currthread_parms ;
  
  struct
  {
    int startflag ;
    threadref * lastid ;
    threadref * nextthreadid ;
  } threadlist_parms ;

  struct
  {
    threadref * ref ;
    struct cygmon_thread_debug_info * info ;
  } info_parms ;
  
  struct
  {
    threadref * thread ;
    int regcount ;
    void * registers ;
  } reg_parms ;
  struct
  {
    threadref * thread ;        /* 64-bit thread identifier */
    int lock;                   /* 0 == unlock, 1 == lock */
    int mode;                   /* 0 == short (step), 1 == continue */
  } scheduler_parms ;
} ;


typedef int (*dbg_syscall_func) (enum dbg_syscall_ids id,
                                 union dbg_thread_syscall_parms  * p ) ;
