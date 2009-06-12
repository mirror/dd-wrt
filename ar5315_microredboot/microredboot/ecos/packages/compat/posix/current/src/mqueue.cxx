/*========================================================================
//
//      mqueue.cxx
//
//      Message queues tests
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
// Date:          2000-05-14
// Purpose:       This file provides the implementation for POSIX message
//                queues
// Description:   It uses eCos kernel mqueues as the underlying
//                implementation
// Usage:         
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/posix.h>

#ifdef CYGPKG_POSIX_MQUEUES

#include <pkgconf/kernel.h>

/* INCLUDES */

#include <cyg/infra/cyg_type.h>      // common types etc.
#include <cyg/infra/cyg_ass.h>       // Assertion support
#include <cyg/infra/cyg_trac.h>      // Tracing support
#include <cyg/kernel/mqueue.hxx>     // eCos Mqueue Header
#include <cyg/kernel/sched.hxx>      // Cyg_Scheduler::lock()
#include <cyg/kernel/sched.inl>      // inlines for above
#include <mqueue.h>                  // Standard POSIX mqueue header
#include <sys/types.h>               // mode_t, ssize_t
#include <limits.h>                  // PATH_MAX
#include <stdlib.h>                  // malloc, etc.
#include <errno.h>                   // errno
#include <fcntl.h>                   // O_*
#include <stdarg.h>                  // varargs
#include <pthread.h>                 // mutexes
#include <string.h>                  // strncpy
#ifdef CYGFUN_POSIX_MQUEUE_NOTIFY
# include <signal.h>
# include "pprivate.h"               // cyg_sigqueue()
#endif
#ifdef CYGFUN_KERNEL_THREADS_TIMER
# include <time.h>
# include "pprivate.h"               // cyg_timespec_to_ticks()
#endif

/* CONSTANTS */

#define MQ_VALID_MAGIC  0x6db256c1

/* TYPE DEFINITIONS */

struct mqtabent;

// this is a queue user - each one of these corresponds to a mqd_t
struct mquser {
    int flags;               // O_RDONLY, O_WRONLY, O_RDWR, O_NONBLOCK
    struct mqtabent *tabent; // back pointer to table entry
    struct mquser *next;
    bool notifieruser;       // POSIX sucks so bad. It requires a mq_close
                             // to only deregister the notification if it
                             // was done via this descriptor. So we have to
                             // know if it was this one
#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    cyg_uint32 magic;        // magic number: MQ_VALID_MAGIC if valid
#endif    
};

struct mqtabent {
    char name[ PATH_MAX ]; // ascii name - set to "" when unused
    Cyg_Mqueue *mq;        // the underlying queue object
    long maxmsg;           // as set on creation
    long msgsize;          // as set on creation
    bool unlinkme;         // unlink when final user closes?
    struct mquser *users;  // each user

#ifdef CYGFUN_POSIX_MQUEUE_NOTIFY
    const struct sigevent *sigev; // notification event
#endif
};

/* GLOBALS */

static struct mqtabent mqtab[ CYGNUM_POSIX_MQUEUE_OPEN_MAX ];
static pthread_mutex_t mqtab_mut = PTHREAD_MUTEX_INITIALIZER;

/* LOCAL FUNCTIONS */

//------------------------------------------------------------------------

// placement new definition
inline void *operator new(size_t size, void *ptr) 
{ 
    CYG_CHECK_DATA_PTR( ptr, "Bad pointer" ); 
    return ptr; 
} 

// Deallocation callback from Cyg_Mqueue
static void
my_free( void *ptr, size_t )
{
    free( ptr );
}

//------------------------------------------------------------------------

// Do the actual "unlink" of a queue, i.e. mark it invalid in the table.
// The table mutex is assumed to be locked
static void
do_mq_unlink( struct mqtabent *tabent )
{
    CYG_REPORT_FUNCTION();
    CYG_CHECK_DATA_PTRC( tabent );

    tabent->name[0] = '\0'; // won't match anything the user sends now
    tabent->mq->~Cyg_Mqueue();
    free( tabent->mq );
    tabent->mq=NULL;

    CYG_REPORT_RETURN();
}

//------------------------------------------------------------------------

#ifdef CYGFUN_POSIX_MQUEUE_NOTIFY

static void
notifyme( Cyg_Mqueue &q, CYG_ADDRWORD data )
{
    CYG_REPORT_FUNCTION();
    struct mquser *user = (struct mquser *)data;
    CYG_CHECK_DATA_PTRC( user );
    struct mqtabent *tabent = user->tabent;
    CYG_CHECK_DATA_PTRC( tabent );

    Cyg_Scheduler::lock();
    // we may have been pre-empted before this, so check there's still a
    // notification to do

    if ( NULL == tabent->sigev ) {
        Cyg_Scheduler::unlock();
        CYG_REPORT_RETURN();
        return;
    } // if

    const struct sigevent *ev = tabent->sigev;
    
    // first deregister
    q.setnotify( NULL, 0 );
    tabent->sigev = NULL;
    user->notifieruser = false; // not any more
    
    // now the rest of the world can go
    Cyg_Scheduler::unlock();
    
    // queue event. If it fails... nothing we can do :-( so ignore return code
    cyg_sigqueue( ev, SI_MESGQ );

    cyg_deliver_signals();
    
    CYG_REPORT_RETURN();
}

#endif // ifdef CYGFUN_POSIX_MQUEUE_NOTIFY

//------------------------------------------------------------------------

/* EXPORTED FUNCTIONS */

externC mqd_t
mq_open( const char *name, int oflag, ... )
{
    CYG_REPORT_FUNCTYPE( "returning %08x" );
    CYG_REPORT_FUNCARG2( "name=%08x, oflag=%d", name, oflag );
    CYG_CHECK_DATA_PTRC( name );

    if ( ((oflag & O_RDONLY) != O_RDONLY) &&
         ((oflag & O_WRONLY) != O_WRONLY) &&
         ((oflag & O_RDWR) != O_RDWR)) {
        // user didn't specify mode
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return (mqd_t)-1;
    } // if

    mqd_t retval;
    cyg_ucount32 i;
    struct mqtabent *qtabent=NULL;
    int interr;

    interr = pthread_mutex_lock( &mqtab_mut );
    // should never fail
    CYG_ASSERT( interr == 0, "internal lock failed!" );
    
    // find if a matching entry exists first
    // FIXME: Should check for length and return ENAMETOOLONG
    for ( i=0; i < CYGNUM_POSIX_MQUEUE_OPEN_MAX; i++ ) {
        if ( 0 == strncmp(name, mqtab[i].name, PATH_MAX) ) {
            qtabent = &mqtab[i];
            break;
        } // if
    } // for
    
    if ( (NULL != qtabent) && (O_EXCL == (oflag & O_EXCL)) ) {
        errno = EEXIST;
        retval = (mqd_t)-1;
        goto exit_unlock;
    }
        
    if ( (NULL == qtabent) && (O_CREAT != (oflag & O_CREAT)) ) {
        errno = ENOENT;
        retval = (mqd_t)-1;
        goto exit_unlock;
    }

    // so if we didn't find something, we must be being asked to create it
    if (NULL == qtabent) {
        mode_t mode; // FIXME: mode ignored for now
        const struct mq_attr *attr;
        const struct mq_attr default_attr = { 0, MQ_OPEN_MAX, 128 };
        va_list args;
        
        va_start( args, oflag );
        mode = va_arg( args, mode_t );
        attr = va_arg( args, struct mq_attr * );
        va_end( args );

        // find an empty table entry
        for ( i=0; i < CYGNUM_POSIX_MQUEUE_OPEN_MAX; i++ ) {
            if ( NULL == mqtab[i].mq )
                break;
        }

        // if not found, table is full
        if ( i == CYGNUM_POSIX_MQUEUE_OPEN_MAX ) {
            errno = ENFILE;
            retval = (mqd_t)-1;
            goto exit_unlock;
        }

        Cyg_Mqueue::qerr_t qerr;

        // user can specify NULL attr, which means arbitrary message queue
        // size! Duh.
        if ( NULL == attr )
            attr = &default_attr;
        else {
            // if they do supply one, POSIX says we're meant to check it
            if (attr->mq_maxmsg <= 0 || attr->mq_msgsize <= 0) {
                errno = EINVAL;
                retval = (mqd_t)-1;
                goto exit_unlock;
            }
        } // else

        // allocate the underlying queue
        Cyg_Mqueue *mqholder = (Cyg_Mqueue *)malloc( sizeof(Cyg_Mqueue) );
        if ( NULL == mqholder ) {
            errno = ENOSPC;
            retval = (mqd_t)-1;
            goto exit_unlock;
        }
            
        // construct it with placement new
        mqtab[i].mq = new (mqholder) Cyg_Mqueue( attr->mq_maxmsg,
                                                 attr->mq_msgsize,
                                                 &malloc, &my_free, &qerr );
            
        switch (qerr) {
        case Cyg_Mqueue::OK:
            break;
        case Cyg_Mqueue::NOMEM:
            free( mqholder );
            errno = ENOSPC;
            retval = (mqd_t)-1;
            goto exit_unlock;
        default:
            CYG_FAIL("Unhandled Cyg_Mqueue constructor return error");
            break;
        } // switch

        mqtab[i].users = (struct mquser *) malloc( sizeof(struct mquser) );
        if ( NULL == mqtab[i].users ) {
            mqtab[i].mq->~Cyg_Mqueue();
            free( mqholder );
            errno = ENOSPC;
            retval = (mqd_t)-1;
            goto exit_unlock;
        }

        // initialize mqtab[i]
        mqtab[i].maxmsg  = attr->mq_maxmsg;
        mqtab[i].msgsize = attr->mq_msgsize;
        mqtab[i].unlinkme = false;
#ifdef CYGFUN_POSIX_MQUEUE_NOTIFY
        mqtab[i].sigev = NULL;
#endif
        strncpy( mqtab[i].name, name, PATH_MAX );

        // initialize first mqtab[i].users
        mqtab[i].users->next = NULL;
        // set the mode for later, but also note that O_NONBLOCK can
        // be set in oflags *or* the attr the user passed
        mqtab[i].users->flags = oflag | (attr->mq_flags & O_NONBLOCK);

        // set back pointer so that message queue handle can find actual queue
        mqtab[i].users->tabent = &mqtab[i]; 
        
        mqtab[i].users->notifieruser = false;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
        mqtab[i].users->magic = MQ_VALID_MAGIC; // now valid
#endif
        
        retval=(mqd_t)mqtab[i].users;

        goto exit_unlock;
    } // if (NULL == qtabent)

    // so we're not creating, and we have a valid qtabent

    // But this qtabent may be being unlinked. If so, we are permitted
    // to return an error, so we will. (see under mq_unlink() in POSIX)
    // Which error though? EINVAL seems best, but POSIX doesn't say :-/

    if (true == qtabent->unlinkme) {
        errno = EINVAL;
        retval = (mqd_t)-1;
        goto exit_unlock;
    }
    
    // now we have a usable qtabent

    struct mquser *user;
    user = (struct mquser *) malloc( sizeof(struct mquser) );
    if ( NULL == user ) {
            errno = ENOSPC;
            retval = (mqd_t)-1;
            goto exit_unlock;
    }

    // prepend to qtab user list
    user->next = qtabent->users;
    qtabent->users = user;

    // set back pointer so that message queue handle can find actual queue
    user->tabent = qtabent;

    user->flags = oflag;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    user->magic = MQ_VALID_MAGIC; // now valid
#endif

    retval=(mqd_t)user;

 exit_unlock:
    interr = pthread_mutex_unlock( &mqtab_mut );
    // should never fail
    CYG_ASSERT( interr == 0, "internal lock failed!" );
    CYG_REPORT_RETVAL( retval );
    return retval;
} // mq_open()

//------------------------------------------------------------------------

// NOTE: It is the *user*'s responsibility to ensure that nothing is
// blocked in mq_send() or mq_receive() when closing the queue with
// that descriptor. The standard does not specify the behaviour, so that's
// what I am assuming

externC int
mq_close( mqd_t mqdes )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG1XV( mqdes );
    
    struct mquser *user = (struct mquser *)mqdes;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
#endif
    
    int interr;

    interr = pthread_mutex_lock( &mqtab_mut );
    // should never fail
    CYG_ASSERT( interr == 0, "internal lock failed!" );

    struct mqtabent *tabent = user->tabent;
    struct mquser *usertmp;
    
    // perhaps should return EBADF instead of assert?
    CYG_ASSERT( tabent->users != NULL, "Null message queue user list" );

#ifdef CYGFUN_POSIX_MQUEUE_NOTIFY
    // deregister notification iff this was the message queue descriptor
    // that was used to register it (POSIX says)
    if ( true == user->notifieruser ) {
        tabent->mq->setnotify( NULL, 0 );
        tabent->sigev = NULL;
        // not worth clearing notifieruser
    }
#endif

    // find in the list for this queue and remove - sucks a bit, but seems
    // best over all - the list shouldn't be too long
    if ( tabent->users == user ) {
        tabent->users = user->next;  // remove
    } else {
        for ( usertmp=tabent->users;
              NULL != usertmp->next;
              usertmp = usertmp->next ) {
            if ( usertmp->next == user )
                break;
        } // for

        // perhaps should return EBADF instead of assert?
        CYG_ASSERT( usertmp->next != NULL, "Couldn't find message queue user" );

        usertmp->next = user->next; // remove
    } // else
    
#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    user->magic = 0; // invalidate
#endif

    // free it up
    free( user );

    if ( (true == tabent->unlinkme) && (NULL == tabent->users) ) {
        do_mq_unlink( tabent );
    } // if

    interr = pthread_mutex_unlock( &mqtab_mut );
    // should never fail
    CYG_ASSERT( interr == 0, "internal lock failed!" );
    CYG_REPORT_RETVAL( 0 );
    return 0;
} // mq_close()


//------------------------------------------------------------------------

externC int
mq_unlink( const char *name )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG1( "name=%s", name );

    int retval, interr;
    cyg_ucount32 i;
    struct mqtabent *qtabent=NULL;

    interr = pthread_mutex_lock( &mqtab_mut );
    // should never fail
    CYG_ASSERT( interr == 0, "internal lock failed!" );

    // find the entry first
    // FIXME: Should check for length and return ENAMETOOLONG
    for ( i=0; i < CYGNUM_POSIX_MQUEUE_OPEN_MAX; i++ ) {
        if ( 0 == strncmp(name, mqtab[i].name, PATH_MAX) ) {
            qtabent = &mqtab[i];
            break;
        } // if
    } // for

    if ( NULL == qtabent ) { // not found
        errno = ENOENT;
        retval = -1;
        goto exit_unlock;
    }

    if ( NULL != qtabent->users ) {   // still in use
        qtabent->unlinkme = true;     // so mark it as pending deletion
    } else {
        do_mq_unlink( qtabent );
    } // else

    retval = 0;

 exit_unlock:
    interr = pthread_mutex_unlock( &mqtab_mut );
    // should never fail
    CYG_ASSERT( interr == 0, "internal lock failed!" );
    CYG_REPORT_RETVAL( retval );
    return retval;
} // mq_unlink()

//------------------------------------------------------------------------

externC int
mq_send( mqd_t mqdes, const char *msg_ptr, size_t msg_len,
         unsigned int msg_prio )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG4( "mqdes=%08x, msg_ptr=%08x, msg_len=%u, msg_prio=%u",
                         mqdes, msg_ptr, msg_len, msg_prio );
    CYG_CHECK_DATA_PTRC( msg_ptr );
    
    struct mquser *user = (struct mquser *)mqdes;
    struct mqtabent *tabent = user->tabent;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
#endif
    
    if ( msg_len > (size_t)tabent->msgsize ) {
        errno = EMSGSIZE;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    if ( msg_prio > MQ_PRIO_MAX ) {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    if ( (O_WRONLY != (user->flags & O_WRONLY)) && 
         (O_RDWR != (user->flags & O_RDWR)) ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    // go for it
    Cyg_Mqueue::qerr_t err;
    err = tabent->mq->put( msg_ptr, msg_len, msg_prio,
                           ((user->flags & O_NONBLOCK) != O_NONBLOCK) );
    switch (err) {

    case Cyg_Mqueue::INTR:
        errno = EINTR;
        CYG_REPORT_RETVAL( -1 );
        return -1;

    case Cyg_Mqueue::WOULDBLOCK:
        CYG_ASSERT( (user->flags & O_NONBLOCK) == O_NONBLOCK,
                    "Message queue assumed non-blocking when blocking requested"
            );
        errno = EAGAIN;
        CYG_REPORT_RETVAL( -1 );
        return -1;
        
    case Cyg_Mqueue::OK:
        CYG_REPORT_RETVAL( 0 );
        return 0;

    default:
        CYG_FAIL( "unhandled message queue return code" );
        return -1; // keep compiler happy
    } // switch
} // mq_send()

//------------------------------------------------------------------------


externC ssize_t
mq_receive( mqd_t mqdes, char *msg_ptr, size_t msg_len,
            unsigned int *msg_prio )
{
    CYG_REPORT_FUNCTYPE( "returning %ld" );
    CYG_REPORT_FUNCARG4( "mqdes=%08x, msg_ptr=%08x, msg_len=%u, msg_prio=%08x",
                         mqdes, msg_ptr, msg_len, msg_prio );
    CYG_CHECK_DATA_PTRC( msg_ptr );
    CYG_CHECK_DATA_PTRC( msg_ptr+msg_len-1 );
    if ( NULL != msg_prio )
        CYG_CHECK_DATA_PTRC( msg_prio );
    
    
    struct mquser *user = (struct mquser *)mqdes;
    struct mqtabent *tabent = user->tabent;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
    }
#endif
    
    if ( (O_RDONLY != (user->flags & O_RDONLY)) && 
         (O_RDWR != (user->flags & O_RDWR)) ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
    }

    if ( msg_len < (size_t)tabent->msgsize ) {
        errno = EMSGSIZE;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
    }

    // go for it
    Cyg_Mqueue::qerr_t err;
    err = tabent->mq->get( msg_ptr, &msg_len, msg_prio,
                           ((user->flags & O_NONBLOCK) != O_NONBLOCK) );
    switch (err) {

    case Cyg_Mqueue::INTR:
        errno = EINTR;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;

    case Cyg_Mqueue::WOULDBLOCK:
        CYG_ASSERT( (user->flags & O_NONBLOCK) == O_NONBLOCK,
                    "Message queue assumed non-blocking when blocking requested"
            );
        errno = EAGAIN;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
        
    case Cyg_Mqueue::OK:
        CYG_ASSERT( msg_len <= (size_t)tabent->msgsize,
                    "returned message too long" );
        if ( NULL != msg_prio )
            CYG_ASSERT( *msg_prio <= MQ_PRIO_MAX,
                        "returned message has invalid priority" );
        CYG_REPORT_RETVAL( msg_len );
        return (ssize_t)msg_len;

    default:
        CYG_FAIL( "unhandled message queue return code" );
        return (ssize_t)-1; // keep compiler happy
    } // switch
    
} // mq_receive()


//------------------------------------------------------------------------
#ifdef CYGFUN_KERNEL_THREADS_TIMER
externC int
mq_timedsend( mqd_t mqdes, const char *msg_ptr, size_t msg_len,
              unsigned int msg_prio, const struct timespec *abs_timeout)
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG6( "mqdes=%08x, msg_ptr=%08x, msg_len=%u, msg_prio=%u, "
                         "abs_timeout = %lu, %ld",
                         mqdes, msg_ptr, msg_len, msg_prio, 
                         abs_timeout->tv_sec, abs_timeout->tv_nsec);
    CYG_CHECK_DATA_PTRC( msg_ptr );
    
    struct mquser *user = (struct mquser *)mqdes;
    struct mqtabent *tabent = user->tabent;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
#endif
    
    if ( msg_len > (size_t)tabent->msgsize ) {
        errno = EMSGSIZE;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    if ( msg_prio > MQ_PRIO_MAX ) {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    if ( (O_WRONLY != (user->flags & O_WRONLY)) && 
         (O_RDWR != (user->flags & O_RDWR)) ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    // go for it
    Cyg_Mqueue::qerr_t err;
    bool nonblocking = ((user->flags & O_NONBLOCK) == O_NONBLOCK);
    bool badtimespec = (abs_timeout->tv_nsec < 0) ||
        (abs_timeout->tv_nsec > 999999999l);
    cyg_tick_count abs_ticks = cyg_timespec_to_ticks(abs_timeout);

    // We should never time out if there is room in the queue.  Simplest
    // way to ensure this is to try the non-blocking put() first.
    err = tabent->mq->put( msg_ptr, msg_len, msg_prio, false, abs_ticks );

    // If the blocking variant would have blocked and that is what's wanted
    if ( Cyg_Mqueue::WOULDBLOCK == err && !nonblocking && !badtimespec ) {
        err = tabent->mq->put( msg_ptr, msg_len, msg_prio, true, 
                               abs_ticks );
    }

    switch (err) {

    case Cyg_Mqueue::INTR:
        errno = EINTR;
        CYG_REPORT_RETVAL( -1 );
        return -1;

    case Cyg_Mqueue::WOULDBLOCK:
        if (badtimespec) {
            errno = EINVAL;
            CYG_REPORT_RETVAL( -1 );
            return -1;
        }
        CYG_ASSERT( (user->flags & O_NONBLOCK) == O_NONBLOCK,
                    "Message queue assumed non-blocking when blocking requested"
            );
        errno = EAGAIN;
        CYG_REPORT_RETVAL( -1 );
        return -1;

    case Cyg_Mqueue::TIMEOUT:
        errno = ETIMEDOUT;
        CYG_REPORT_RETVAL( -1 );
        return -1;
        
    case Cyg_Mqueue::OK:
        CYG_REPORT_RETVAL( 0 );
        return 0;

    default:
        CYG_FAIL( "unhandled message queue return code" );
        return -1; // keep compiler happy
    } // switch
} // mq_timedsend()

//------------------------------------------------------------------------


externC ssize_t
mq_timedreceive( mqd_t mqdes, char *msg_ptr, size_t msg_len,
            unsigned int *msg_prio, const struct timespec *abs_timeout)
{
    CYG_REPORT_FUNCTYPE( "returning %ld" );
    CYG_REPORT_FUNCARG6( "mqdes=%08x, msg_ptr=%08x, msg_len=%u, msg_prio=%08x, "
	                 "abs_timeout = %lu, %ld",
                         mqdes, msg_ptr, msg_len, msg_prio,
                         abs_timeout->tv_sec, abs_timeout->tv_nsec );
    CYG_CHECK_DATA_PTRC( msg_ptr );
    CYG_CHECK_DATA_PTRC( msg_ptr+msg_len-1 );
    if ( NULL != msg_prio )
        CYG_CHECK_DATA_PTRC( msg_prio );
    
    
    struct mquser *user = (struct mquser *)mqdes;
    struct mqtabent *tabent = user->tabent;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
    }
#endif
    
    if ( (O_RDONLY != (user->flags & O_RDONLY)) && 
         (O_RDWR != (user->flags & O_RDWR)) ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
    }

    if ( msg_len < (size_t)tabent->msgsize ) {
        errno = EMSGSIZE;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;
    }

    // go for it
    Cyg_Mqueue::qerr_t err;
    bool nonblocking = ((user->flags & O_NONBLOCK) == O_NONBLOCK);
    bool badtimespec = (abs_timeout->tv_nsec < 0) ||
        (abs_timeout->tv_nsec > 999999999l);
    cyg_tick_count abs_ticks = cyg_timespec_to_ticks(abs_timeout);

    // We should never time out if there is something to read.  Simplest
    // way to ensure this is to try the non-blocking get() first.
    err = tabent->mq->get( msg_ptr, &msg_len, msg_prio, false, abs_ticks );

    // If the blocking variant would have blocked and that is what's wanted
    if ( Cyg_Mqueue::WOULDBLOCK == err && !nonblocking && !badtimespec ) {
        err = tabent->mq->get( msg_ptr, &msg_len, msg_prio, true, abs_ticks );
    }

    switch (err) {

    case Cyg_Mqueue::INTR:
        errno = EINTR;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;

    case Cyg_Mqueue::WOULDBLOCK:
        if (badtimespec) {
            errno = EINVAL;
            CYG_REPORT_RETVAL( -1 );
            return -1;
        }
        CYG_ASSERT( (user->flags & O_NONBLOCK) == O_NONBLOCK,
                    "Message queue assumed non-blocking when blocking requested"
            );
        errno = EAGAIN;
        CYG_REPORT_RETVAL( -1 );
        return (ssize_t)-1;

    case Cyg_Mqueue::TIMEOUT:
        errno = ETIMEDOUT;
        CYG_REPORT_RETVAL( -1 );
        return -1;
        
    case Cyg_Mqueue::OK:
        CYG_ASSERT( msg_len <= (size_t)tabent->msgsize,
                    "returned message too long" );
        if ( NULL != msg_prio )
            CYG_ASSERT( *msg_prio <= MQ_PRIO_MAX,
                        "returned message has invalid priority" );
        CYG_REPORT_RETVAL( msg_len );
        return (ssize_t)msg_len;

    default:
        CYG_FAIL( "unhandled message queue return code" );
        return (ssize_t)-1; // keep compiler happy
    } // switch
    
} // mq_timedreceive()

//------------------------------------------------------------------------
#endif

#ifdef CYGFUN_POSIX_MQUEUE_NOTIFY

externC int
mq_notify( mqd_t mqdes, const struct sigevent *notification )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG2( "mqdes=%08x, notification=%08x", mqdes, notification );
    if ( NULL != notification )
        CYG_CHECK_DATA_PTRC( notification );
    
    struct mquser *user = (struct mquser *)mqdes;
    struct mqtabent *tabent = user->tabent;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
#endif
    
    // lock scheduler since we test and set non-atomically
    Cyg_Scheduler::lock();
    
    // we are being told to clear the notification function
    if ( NULL == notification ) {
        tabent->mq->setnotify( NULL, 0 );
        tabent->sigev = NULL;
        Cyg_Scheduler::unlock();
        CYG_REPORT_RETVAL( 0 );
        return 0;
    } // if
    
    if ( NULL != tabent->sigev ) {  // already registered
        Cyg_Scheduler::unlock();
        errno = EBUSY;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    } // if

    tabent->sigev = notification;
    user->notifieruser = true; // Used for deciding about whether to
                               // deregister in mq_close()
    tabent->mq->setnotify( &notifyme, (CYG_ADDRWORD) user );
    Cyg_Scheduler::unlock();
    
    CYG_REPORT_RETVAL( 0 );
    return 0;
} // mq_notify()

#endif // ifdef CYGFUN_POSIX_MQUEUE_NOTIFY

//------------------------------------------------------------------------

externC int
mq_setattr( mqd_t mqdes, const struct mq_attr *mqstat,
            struct mq_attr *omqstat )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG3( "mqdes=%08x, mqstat=%08x, omqstat=%08x",
                         mqdes, mqstat, omqstat );
    CYG_CHECK_DATA_PTRC( mqstat );

    struct mquser *user = (struct mquser *)mqdes;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
#endif
    
    if ( NULL != omqstat ) {
        CYG_CHECK_DATA_PTRC( omqstat );
        mq_getattr( mqdes, omqstat );
    } // if

    // Two-stage update, so lock sched since it's quick
    Cyg_Scheduler::lock();
    user->flags &= ~O_NONBLOCK;  // clear
    if ( (mqstat->mq_flags & O_NONBLOCK) == O_NONBLOCK ) {
        user->flags |= O_NONBLOCK;
    } // if
    Cyg_Scheduler::unlock();

    CYG_REPORT_RETVAL( 0 );
    return 0;
} // mq_setattr()

//------------------------------------------------------------------------

externC int
mq_getattr( mqd_t mqdes, struct mq_attr *mqstat )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG2( "mqdes=%08x, mqstat=%08x", mqdes, mqstat );
    CYG_CHECK_DATA_PTRC( mqstat );

    struct mquser *user = (struct mquser *)mqdes;
    struct mqtabent *tabent = user->tabent;

#ifdef CYGIMP_POSIX_MQUEUE_VALIDATE_DESCRIPTOR
    if ( user->magic != MQ_VALID_MAGIC ) {
        errno  = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
#endif
    
    mqstat->mq_flags   = user->flags;
    mqstat->mq_maxmsg  = tabent->maxmsg;
    mqstat->mq_msgsize = tabent->msgsize;
    mqstat->mq_curmsgs = tabent->mq->count();    
    
    CYG_REPORT_RETVAL( 0 );
    return 0;
} // mq_getattr()


//------------------------------------------------------------------------

#endif // ifdef CYGPKG_POSIX_MQUEUES

/* EOF mqueue.cxx */
