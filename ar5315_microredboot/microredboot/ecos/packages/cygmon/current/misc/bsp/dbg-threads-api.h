#ifndef __BSP_DBG_THREADS_API_H__
#define __BSP_DBG_THREADS_API_H__
//==========================================================================
//
//      dbg-threads-api.h
//
//      <DESCRIPTION>
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      
// Description:  These are the calls used to extract operating system
//               specific information used in supporting thread aware
//               debugging.
//               The Operating Environment being debugged needs to supply
//               these functions.
//
//####DESCRIPTIONEND####
//
//=========================================================================


#define has_thread_void       0 
#define has_thread_current    1
#define has_thread_registers  2
#define has_thread_reg_change 4
#define has_thread_list       8
#define has_thread_info       16

typedef unsigned char threadref[8] ;

struct dbg_capabilities
{
    unsigned long mask1;
};

/* fill in the list of thread aware capabilities */
extern int dbg_thread_capabilities(struct dbg_capabilities *cbp);


/* Fill in the identifier of the current thread */
/* return 1 if defined, 0 if not defined */
extern int dbg_currthread(threadref *varparm);

/* get the first or next member of the list of known threads */
extern int dbg_threadlist(int startflag,
			  threadref *lastthreadid,
			  threadref *next_thread);

/* return 1 if next_threadid has been filled in with a value */
/* return 0 if there are none or no more */

/* The O.S can fill in the following information about a thread when queried.
   The structure of thise strings is determined by the O.S.
   It is display oriented, so figure out what the users need to see.
   Nulls are OK but GDB will fill some not so meaningful data.
   These pointers may be in the calles private structures, the info will
   get copied immediatly after the call to retreive it.
   */
struct cygmon_thread_debug_info
{
  threadref thread_id ;
  int context_exists ; /* To the point where examining its state,
			 registers and stack makes sense to GDB */
  char * thread_display ; /* As shown in thread status window, name, state */
  char * unique_thread_name ; /* human readable identifier, window label */
  char * more_display ;   /* more detailed info about thread.
			  priority, queuedepth, state, stack usage, statistics */
} ;




extern int dbg_threadinfo(threadref *threadid,
			  struct cygmon_thread_debug_info *info);

/* Return 1 if threadid is defined and info copied, 0 otherwise */

/* The O.S should fillin the array of registers using values from the
saves context. The array MUST be in GDB register save order even if
the saved context is different or smaller. Do not alter the values of
registers which are NOT in the O.S. thread context. Their default values
have already been assigned.
*/

extern int dbg_getthreadreg(threadref *osthreadid, 
			    int regcount, /* count of registers in the array */
			    void * regval); /* fillin this array */


/* The O.S. should scan through this list of registers which are in
GDB order and the O.S. should replace the values of all registers
which are defined in the saved context of thread or process identified
by osthreadid. Return 0 if the threadis does not map to a known
process or other error. Return 1 if the setting is successful.  */

extern int dbg_setthreadreg(threadref *osthreadid, 
			    int regcount, /* number of registers */
			    void *regval) ;


extern int dbg_scheduler(threadref *thread_id, int lock, int mode);

/* --- EXCEPTION -
   The O.S. does not provide this function , it calls this to specify the
   location of the dbg_syscall vector
   */
extern void patch_dbg_syscalls(void * vector);

#endif /* DBG_THREADS_API_INCLUDED */
#endif // __BSP_DBG_THREADS_API_H__
