#ifndef CYGONCE_KERNEL_MQUEUE_HXX
#define CYGONCE_KERNEL_MQUEUE_HXX
/*========================================================================
//
//      mqueue.hxx
//
//      Message queues
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
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-05-09
// Purpose:       This file provides the interface for eCos message queues
// Description:   This differs from the message boxes also supported by
//                eCos primarily because the requirements of message
//                queues are driven by POSIX semantics. POSIX semantics are
//                more dynamic and therefore heavyweight than Mboxes,
//                including prioritization, and variable sized queues and
//                message lengths
// Usage:         #include <cyg/kernel/mqueue.hxx>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/kernel.h>          /* Configuration header */

/* INCLUDES */

#include <stddef.h>                  /* size_t */
#include <cyg/infra/cyg_type.h>      /* Types */
#include <cyg/infra/cyg_ass.h>       /* CYGDBG_DEFINE_CHECK_THIS,
                                        CYGDBG_USE_ASSERTS */
#include <cyg/kernel/ktypes.h>       /* Kernel package types */
#include <cyg/kernel/sema.hxx>       /* Cyg_Counting_Semaphore */

/* CLASSES */

class Cyg_Mqueue {
public:
    typedef void (*callback_fn_t)(Cyg_Mqueue &q, CYG_ADDRWORD data);
    typedef void * (*qalloc_fn_t)(size_t len);
    typedef void (*qfree_fn_t)(void *ptr, size_t len);

    typedef enum {
        OK=0,
        NOMEM,
        WOULDBLOCK,
#ifdef CYGFUN_KERNEL_THREADS_TIMER
        TIMEOUT,
#endif
        INTR
    } qerr_t;

protected:
    struct qentry {
        struct qentry *next;
        unsigned int priority;
        size_t buflen;
        volatile bool busy;
        // data buffer follows here
        char *buf() const { return (char *)this + sizeof(*this); }
    };

    Cyg_Counting_Semaphore putsem, getsem;

    struct qentry *q;            // q entries in use
    struct qentry *freelist;     // q entries not in use
    void *queuespace;            // placeholder for the dynamically allocated
                                 // area

    size_t queuespacesize;

    qfree_fn_t free_fn;          // how to free queuespace when we destruct

    callback_fn_t callback;
    CYG_ADDRWORD callback_data;

    CYGDBG_DEFINE_CHECK_THIS

#ifdef CYGDBG_USE_ASSERTS
    long qlen;
    size_t msgsize;
#endif

public:

    Cyg_Mqueue( long maxmsgs, long maxmsgsize,
                qalloc_fn_t qalloc, qfree_fn_t qfree, qerr_t *err );
    ~Cyg_Mqueue();
    // put() copies len bytes of *buf into the queue at priority prio
    qerr_t put( const char *buf, size_t len, unsigned int prio, bool block=true
#ifdef CYGFUN_KERNEL_THREADS_TIMER
                ,cyg_tick_count timeout = 0
#endif
              );

    // get() returns the oldest highest priority message in the queue in *buf
    // and sets *prio to the priority (if prio is non-NULL) and *len to the
    // actual message size
    qerr_t get( char *buf, size_t *len, unsigned int *prio, bool block=true
#ifdef CYGFUN_KERNEL_THREADS_TIMER
                ,cyg_tick_count timeout = 0
#endif
              ); 

    // count() returns the number of messages in the queue
    long count();

    // Supply a callback function to call (with the supplied data argument)
    // when the queue goes from empty to non-empty (unless someone's already
    // doing a get()). This returns the old callback_fn, and if olddata is
    // non-NULL sets it to the old data (yes, really!)
    callback_fn_t setnotify( callback_fn_t callback_fn, CYG_ADDRWORD data,
                             CYG_ADDRWORD *olddata=NULL);
    
}; /* class Cyg_Mqueue */

#ifndef CYGIMP_KERNEL_SYNCH_MQUEUE_NOT_INLINE
# include <cyg/kernel/mqueue.inl>
#endif

#endif /* CYGONCE_KERNEL_MQUEUE_HXX multiple inclusion protection */

/* EOF mqueue.hxx */
