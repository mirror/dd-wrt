#ifndef CYGONCE_KERNEL_MQUEUE_INL
#define CYGONCE_KERNEL_MQUEUE_INL
/*========================================================================
//
//      mqueue.inl
//
//      Message queues implementation
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
// Purpose:       This file provides the implementation for eCos message
//                queues
// Description:   This differs from the message boxes also supported
//                by eCos primarily because the requirements of message
//                queues are driven by POSIX semantics. POSIX semantics are
//                more dynamic and therefore heavyweight than Mboxes,
//                including prioritization, and variable sized queues and
//                message lengths
// Usage:         Do not include this file directly - instead
//                #include <cyg/kernel/mqueue.hxx>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/system.h>
#include <pkgconf/kernel.h>          // Configuration header

/* INCLUDES */

#include <stddef.h>                  // size_t, NULL
#include <cyg/infra/cyg_type.h>      // Types
#include <cyg/kernel/mqueue.hxx>     // Header for this file, just in case
#include <cyg/infra/cyg_ass.h>       // Assertion support
#include <cyg/infra/cyg_trac.h>      // Tracing support
#include <cyg/kernel/sched.hxx>      // scheduler
#include <cyg/kernel/sched.inl>      // scheduler inlines
#include <cyg/kernel/sema.hxx>       // Cyg_Counting_Semaphore

#ifdef CYGPKG_ISOINFRA
# include <string.h>                 // memcpy
#else
externC void * memcpy( void *, const void *, size_t );
#endif

// NOTE:
// An alternative implementation based on mutexes and condition variables
// rather than semaphores/scheduler locking was considered. But it was
// not thought quite as good because it isn't driver safe. You would
// also have to manage explicitly what counting semaphores do for you
// intrinsically. Also with the mutex approach, the message queue would
// be locked the whole time a new entry was being filled in, or copied out
//
// It also makes the non-blocking case properly non-blocking rather than
// still being able to block while waiting for a mutex protecting
// the message queue internal structures

/* INLINE FUNCTIONS */

#ifndef CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE
# define CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE inline
#endif

//------------------------------------------------------------------------

CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE cyg_bool
Cyg_Mqueue::check_this( cyg_assert_class_zeal zeal ) const
{
    if (zeal != cyg_none) {
        CYG_CHECK_DATA_PTRC(this);  // extreme paranoia

#ifdef CYGDBG_USE_ASSERTS
        if ( qlen <= 0 || msgsize <= 0 )
            return false;
#endif

        if ( queuespacesize < sizeof(struct qentry)+1 )
            return false;

        CYG_CHECK_DATA_PTRC(queuespace);
        CYG_CHECK_FUNC_PTRC(free_fn);

        // prevent pre-emption through this. Not so bad since
        // this is only a diagnostic function
        Cyg_Scheduler::lock();

        if (NULL != q)
            CYG_CHECK_DATA_PTRC(q);
        if (NULL != freelist)
            CYG_CHECK_DATA_PTRC(freelist);
        if (NULL != callback)
            CYG_CHECK_FUNC_PTRC(callback);

        // check each queue entry
        long msgs=0, busymsgs=0;
        unsigned int oldprio=0;
        struct qentry *qtmp;

        if ( NULL != q )
            oldprio = q->priority;
        for ( qtmp=q; NULL != qtmp; qtmp=qtmp->next ) {
            if ( NULL != qtmp->next )
                CYG_CHECK_DATA_PTRC( qtmp->next );

            // queue should be priority ordered
            if ( qtmp->priority > oldprio )
                goto fail;
            oldprio = qtmp->priority;
            
#ifdef CYGDBG_USE_ASSERTS
            // valid length
            if ( !qtmp->busy )
                if ( qtmp->buflen > msgsize )
                    goto fail;
#endif
            if ( qtmp->busy )
                busymsgs++;
            else
                msgs++;
        } // for
        
        long freemsgs=0;
        
        // check that number of used and unused messages == q length
        for ( qtmp=freelist; NULL != qtmp; qtmp=qtmp->next ) {
            if ( NULL != qtmp->next )
                CYG_CHECK_DATA_PTRC( qtmp->next );
            if ( qtmp->busy )
                busymsgs++;
            else
                freemsgs++;
        }

#ifdef CYGDBG_USE_ASSERTS
        // and sum of all messages should be the total q length
        if ( qlen != (msgs+freemsgs+busymsgs) )
            goto fail;
#endif

        Cyg_Scheduler::unlock();

    }
    return true; // object OK
 fail:
    Cyg_Scheduler::unlock();
    return false; // object fubar'd
}

//------------------------------------------------------------------------

CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE
Cyg_Mqueue::Cyg_Mqueue( long maxmsgs, long maxmsgsize,
                        qalloc_fn_t qalloc, qfree_fn_t qfree, qerr_t *err )
    : putsem(maxmsgs), getsem(0)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5( "maxmsgs=%ld, maxmsgsize=%ld, qalloc=%08x, "
                         "qfree=%08x, &err=%08x", maxmsgs, maxmsgsize,
                         qalloc, qfree, err);
    CYG_PRECONDITIONC( (maxmsgs > 0) && (maxmsgsize > 0) );
    CYG_CHECK_DATA_PTRC( err );
    CYG_CHECK_FUNC_PTRC( qalloc );
    CYG_CHECK_FUNC_PTRC( qfree );

    // mem to allocate for entire queue size. Also wants to be rounded
    // up so that the structs are aligned.
    const long addralign = sizeof(void *) - 1;
    long entrysize = (sizeof(struct qentry) + maxmsgsize + addralign)
       & ~addralign;

    queuespacesize = entrysize * maxmsgs;
    queuespace = qalloc( queuespacesize );

    if (NULL == queuespace) {
        *err=NOMEM;
        CYG_REPORT_RETURN();
        return;
    }

    // link up freelist
    long i;
    struct qentry *qtmp;
    for ( i=0, qtmp=(struct qentry *)queuespace;
          i<maxmsgs-1;
          i++, qtmp=qtmp->next ) {
        qtmp->busy = false;
        qtmp->next = (struct qentry *)((char *)qtmp + entrysize);
    } // for

    freelist   = (struct qentry *)queuespace;

    // set the last entry in the chain to the start to make the list circular
    qtmp->next = NULL;
    qtmp->busy = false;
    callback   = NULL;
    q          = NULL;
    free_fn    = qfree;
#ifdef CYGDBG_USE_ASSERTS
    qlen       = maxmsgs;
    msgsize    = maxmsgsize;
#endif

    *err = OK;

    // object should be valid now
    CYG_ASSERT_THISC();

    CYG_REPORT_RETURN();
}

//------------------------------------------------------------------------

CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE
Cyg_Mqueue::~Cyg_Mqueue()
{
    CYG_REPORT_FUNCTION();

    if ( NULL != queuespace ) {
        // object should be valid if queuespace was successfully allocated
        CYG_ASSERT_THISC();
        free_fn( queuespace, queuespacesize );
    }

#ifdef CYGDBG_USE_ASSERTS
    qlen = msgsize = 0; // deliberately make it fail check_this() if used
#endif

    CYG_REPORT_RETURN();
}

//------------------------------------------------------------------------

// put() copies len bytes of *buf into the queue at priority prio
CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE Cyg_Mqueue::qerr_t
Cyg_Mqueue::put( const char *buf, size_t len, unsigned int prio, bool block
#ifdef CYGFUN_KERNEL_THREADS_TIMER
                 , cyg_tick_count timeout
#endif
               )
{
    CYG_REPORT_FUNCTYPE( "err=%d");
    CYG_REPORT_FUNCARG4( "buf=%08x, len=%ld, prio=%ud, block=%d",
                         buf, len, prio, block==true );
    CYG_CHECK_DATA_PTRC( buf );
    CYG_ASSERT_THISC();
    CYG_PRECONDITIONC( len <= (size_t)msgsize );

    qerr_t err;
    struct qentry *qtmp, *qent;

    // wait till a freelist entry is available
    if ( true == block ) {
#ifdef CYGFUN_KERNEL_THREADS_TIMER
        if ( timeout != 0) {
	    if ( false == putsem.wait(timeout) ) {
                err = TIMEOUT;
                goto exit;
            }
        }
        else
#endif
        if ( false == putsem.wait() ) {
            err = INTR;
            goto exit;
        }
    } else { 
        if ( false == putsem.trywait() ) {
            err = WOULDBLOCK;
            goto exit;
        }
    }

    // prevent preemption when fiddling with important members
    Cyg_Scheduler::lock();

    CYG_ASSERT_THISC();

    // get a queue entry from the freelist
    // don't need to check the freelist - the semaphore tells us there's
    // definitely a usable non-busy one there. It's just a question of
    // locating it.

    if (!freelist->busy) { // fast-track common case
        qent     = freelist;
        freelist = freelist->next;
    } else {
        for ( qtmp=freelist; qtmp->next->busy; qtmp=qtmp->next )
            CYG_EMPTY_STATEMENT; // skip through
        qent       = qtmp->next;
        qtmp->next = qent->next;
    }
            
    // now put it in place in q

    if ( NULL == q ) {
        q = qent;
        q->next = NULL;
    } else {
        struct qentry **qentp;

        // insert into queue according to prio
        for ( qentp=&q; NULL != *qentp; qentp = &((*qentp)->next) ) {
            if ((*qentp)->priority < prio)
                break;
        } // for

        qent->next = *qentp;
        *qentp = qent;
    } // else
    
    qent->priority = prio; // have to set this now so when the sched is
                           // unlocked, other qent's can be added in the
                           // right place
    qent->busy = true; // let things know this entry should be ignored until
                       // it's finished having its data copied

    // unlock the scheduler, and potentially switch threads, but
    // that's okay now. We don't want it locked for the expensive memcpy
    Cyg_Scheduler::unlock();

    qent->buflen   = len;
    memcpy( qent->buf(), buf, len );

    // make available now - setting non-atomically is alright if you think
    // about it - the only thing that matters is that it's completed before
    // the post()
    qent->busy = false;

    // if we have to notify someone, we only do it if no-one's already
    // sitting waiting for a message to appear, AND if it's a transition
    // from empty to non-empty

    if ( callback != NULL && !getsem.waiting() && (0 == getsem.peek()) ) {
        getsem.post();        
        callback( *this, callback_data );
    } else
        getsem.post();        

    err = OK;

 exit:
    CYG_ASSERT_THISC();
    CYG_REPORT_RETVAL(err);
    return err;
} // Cyg_Mqueue::put()

//------------------------------------------------------------------------


// get() returns the oldest highest priority message in the queue in *buf
// and sets *prio to the priority (if prio is non-NULL) and *len to the
// actual message size

CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE Cyg_Mqueue::qerr_t
Cyg_Mqueue::get( char *buf, size_t *len, unsigned int *prio, bool block
#ifdef CYGFUN_KERNEL_THREADS_TIMER
                 , cyg_tick_count timeout
#endif
               )
{
    CYG_REPORT_FUNCTYPE( "err=%d");
    CYG_REPORT_FUNCARG4( "buf=%08x, len=%08x, prio=%08x, block=%d",
                         buf, len, prio, block==true );
    CYG_CHECK_DATA_PTRC( buf );
    CYG_CHECK_DATA_PTRC( len );
    if ( NULL != prio )
        CYG_CHECK_DATA_PTRC( prio );
    CYG_ASSERT_THISC();

    qerr_t err;
    struct qentry *qent;

    // wait till a q entry is available
    if ( true == block ) {
#ifdef CYGFUN_KERNEL_THREADS_TIMER
        if ( timeout != 0) {
            if ( false == getsem.wait(timeout) ) {
                err = TIMEOUT;
                goto exit;
            }
        }
        else
#endif
        if ( false == getsem.wait() ) {
            err = INTR;
            goto exit;
        }
    } else { 
        if ( false == getsem.trywait() ) {
            err = WOULDBLOCK;
            goto exit;
        }
    }

    // prevent preemption when fiddling with important members
    
    Cyg_Scheduler::lock();
    
    // don't need to check the q - the semaphore tells us there's
    // definitely a usable non-busy one there. It's just a question of
    // locating it.
    
    if ( !q->busy ) {   // fast-track the common case
        qent       = q;
        q          = qent->next;
    } else {
        struct qentry *qtmp;

        for ( qtmp=q; qtmp->next->busy; qtmp=qtmp->next )
            CYG_EMPTY_STATEMENT; // skip through

        qent = qtmp->next;
        qtmp->next = qent->next;
    } // else

    // now stick at front of freelist, but marked busy
    qent->next = freelist;
    freelist   = qent;

    qent->busy = true; // don't let it truly be part of the freelist just yet
                       // till the data is copied out

    // unlock the scheduler, and potentially switch threads, but
    // that's okay now. We don't want it locked for the expensive memcpy
    Cyg_Scheduler::unlock();

    *len  = qent->buflen;
    if ( NULL != prio )
        *prio = qent->priority;
    memcpy( buf, qent->buf(), *len );

    // make available now - setting non-atomically is alright if you think
    // about it - the only thing that matters is that it's completed before
    // the post()
    qent->busy = false;

    putsem.post();

    err = OK;

 exit:
    CYG_ASSERT_THISC();
    CYG_REPORT_RETVAL(err);
    return err;
    
} // Cyg_Mqueue::get()

//------------------------------------------------------------------------

// count() returns the number of messages in the queue
inline long
Cyg_Mqueue::count()
{
    CYG_REPORT_FUNCTYPE("curmsgs=%d");
    
    long curmsgs = (long)getsem.peek();

    CYG_REPORT_RETVAL(curmsgs);
    return curmsgs;    
} // Cyg_Mqueue::count()

//------------------------------------------------------------------------


// Supply a callback function to call (with the supplied data argument)
// when the queue goes from empty to non-empty (unless someone's already
// doing a get()). This returns the old callback_fn, and if olddata is
// non-NULL sets it to the old data (yes, really!)
CYGPRI_KERNEL_SYNCH_MQUEUE_INLINE Cyg_Mqueue::callback_fn_t
Cyg_Mqueue::setnotify( callback_fn_t callback_fn, CYG_ADDRWORD data,
                       CYG_ADDRWORD *olddata)
{
    CYG_REPORT_FUNCTYPE("old callback=%08x");
    CYG_REPORT_FUNCARG3XV( callback_fn, data, olddata );
    if ( NULL != callback_fn )
        CYG_CHECK_FUNC_PTRC( callback_fn );
    if (NULL != olddata)
        CYG_CHECK_DATA_PTRC( olddata );

    callback_fn_t oldfn;

    // Need to prevent preemption for accessing common structures
    // Just locking the scheduler has the least overhead
    Cyg_Scheduler::lock();

    oldfn = callback;
    if (NULL != olddata)
        *olddata = callback_data;

    callback_data = data;
    callback      = callback_fn;

    Cyg_Scheduler::unlock();

    CYG_REPORT_RETVAL(oldfn);
    return oldfn;
}

//------------------------------------------------------------------------

#endif /* CYGONCE_KERNEL_MQUEUE_INL multiple inclusion protection */

/* EOF mqueue.inl */
