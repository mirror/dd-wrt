/*========================================================================
//
//      mqueue1.c
//
//      POSIX Message queues tests
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
// Date:          2000-05-18
// Purpose:       This file provides tests for POSIX mqueues
// Description:   
// Usage:         
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/posix.h>

#ifndef CYGPKG_POSIX_MQUEUES
# define NA_MSG "Message queues not configured"
#endif

#ifdef NA_MSG
#include <cyg/infra/testcase.h>      // test API
void
cyg_user_start(void)
{
    CYG_TEST_NA( NA_MSG );
}

#else

/* INCLUDES */

#include <fcntl.h>                   // O_*
#include <errno.h>                   // errno
#include <sys/stat.h>                // file modes
#include <mqueue.h>                  // Mqueue Header
#include <cyg/infra/testcase.h>      // test API

/* FUNCTIONS */

static int
my_memcmp(const void *m1, const void *m2, size_t n)
{
    char *s1 = (char *)m1;
    char *s2 = (char *)m2;

    while (n--) {
        if (*s1 != *s2)
            return *s1 - *s2;
        s1++;
        s2++;
    }
    return 0;
} // my_memcmp()

//************************************************************************

int
main(void)
{
    mqd_t q1, q2;
    char buf[20];
    ssize_t recvlen;
    unsigned int prio;
    struct mq_attr attr, oattr;
    mode_t mode;
    int err;

    CYG_TEST_INIT();
    CYG_TEST_INFO( "Starting POSIX message test 1" );

    q1 = mq_open( "/mq1", O_RDWR );
    CYG_TEST_PASS_FAIL( q1 == (mqd_t)-1, "error for non-existent queue" );
    CYG_TEST_PASS_FAIL( ENOENT == errno,
                        "errno correct for non-existent queue" );

    attr.mq_flags = 0;
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = 20;
    mode = S_IRWXU|S_IRWXG|S_IRWXO; // rwx for all

    q1 = mq_open( "/mq1", O_CREAT|O_NONBLOCK|O_WRONLY, mode, &attr );
    CYG_TEST_PASS_FAIL( q1 != (mqd_t)-1, "simple mq_open (write only)" );
    
    err = mq_getattr( q1, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_getattr" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK == (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY != (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY == (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (0 == attr.mq_curmsgs ), "getattr attributes correct" );
                        
    err = mq_send( q1, "Vik is brill", sizeof("Vik is brill"), 10 );

    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_send" );
    
    err = mq_getattr( q1, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_getattr after send" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK == (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY != (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY == (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (1 == attr.mq_curmsgs ),
                        "getattr attributes correct #2" );
                        
    q2 = mq_open( "/mq1", O_RDONLY|O_CREAT|O_EXCL );
    CYG_TEST_PASS_FAIL( q2 == (mqd_t)-1,
                        "error for exclusive open of existing queue" );
    CYG_TEST_PASS_FAIL( EEXIST == errno,
                        "errno correct for exclusive open of existing queue" );
    
    q2 = mq_open( "/mq1", O_RDONLY );
    CYG_TEST_PASS_FAIL( q2 != (mqd_t)-1, "simple mq_open (read only)" );
    
    err = mq_getattr( q2, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_getattr, different mqd_t" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK != (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY == (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY != (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (1 == attr.mq_curmsgs ),
                        "getattr attributes correct #3" );

    err = mq_close( q2 );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_close" );
    
    q2 = mq_open( "/mq1", O_RDONLY );
    CYG_TEST_PASS_FAIL( q2 != (mqd_t)-1, "mq_open reopen (read only)" );
    
    err = mq_getattr( q2, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_getattr, different mqd_t" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK != (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY == (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY != (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (1 == attr.mq_curmsgs ),
                        "getattr attributes correct #4" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == sizeof("Vik is brill"),
                        "receive message length" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, "Vik is brill",
                                        sizeof("Vik is brill")),
                        "received message data intact" );
    CYG_TEST_PASS_FAIL( 10 == prio, "received at correct priority" );

    err = mq_getattr( q1, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "simple mq_getattr after send" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK == (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY != (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY == (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (0 == attr.mq_curmsgs ),
                        "getattr attributes correct #5" );

    attr.mq_flags |= O_NONBLOCK;
    err = mq_setattr( q2, &attr, &oattr );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_setattr O_NONBLOCK" );
    CYG_TEST_PASS_FAIL( (4 == oattr.mq_maxmsg) &&
                        (20 == oattr.mq_msgsize) &&
                        (O_NONBLOCK != (oattr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY == (oattr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY != (oattr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (oattr.mq_flags & O_RDWR)) &&
                        (0 == oattr.mq_curmsgs ),
                        "old attribute correct" );
    err = mq_getattr( q2, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_getattr after O_NONBLOCK" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK == (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY == (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY != (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (0 == attr.mq_curmsgs ),
                        "new attribute correct" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == (ssize_t)-1,
                        "mq_receive, empty buffer, non-blocking" );
    CYG_TEST_PASS_FAIL( EAGAIN == errno,
                        "errno correct for non-blocking" );
    
    err = mq_send( q2, "foo", sizeof("foo"), 1 );
    CYG_TEST_PASS_FAIL( -1 == err, "error on mq_send on read-only descriptor" );
    CYG_TEST_PASS_FAIL( EBADF == errno,
                        "errno correct for mq_send on r/o descriptor" );
    
    err = mq_send( q2, "supercalifragilisticexpealidocious", 21, 2 );
    CYG_TEST_PASS_FAIL( -1 == err, "error on mq_send (message too long)" );
    CYG_TEST_PASS_FAIL( EMSGSIZE == errno,
                        "errno correct for mq_send (message too long)" );
    
    err = mq_send( q1, "", sizeof(""), 5 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_send \"\"" );
    
    err = mq_send( q1, "I love Vik", sizeof("I love Vik"), 7 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_send (different priority)" );
    
    err = mq_send( q1, "a lot!", sizeof("a lot!"), 7 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_send (same priority)" );
    
    err = mq_send( q1, "Vik is a babe", sizeof("Vik is a babe"), 6 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_send (middle priority)" );
    
    err = mq_send( q1, "wibble", sizeof("wibble"), 6 );
    CYG_TEST_PASS_FAIL( -1 == err, "error on mq_send with full queue" );
    CYG_TEST_PASS_FAIL( EAGAIN == errno,
                        "errno correct for mq_send full queue" );
    
    err = mq_getattr( q2, &attr );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_getattr after sends" );
    CYG_TEST_PASS_FAIL( (4 == attr.mq_maxmsg) &&
                        (20 == attr.mq_msgsize) &&
                        (O_NONBLOCK == (attr.mq_flags & O_NONBLOCK)) &&
                        (O_RDONLY == (attr.mq_flags & O_RDONLY)) &&
                        (O_WRONLY != (attr.mq_flags & O_WRONLY)) &&
                        (O_RDWR != (attr.mq_flags & O_RDWR)) &&
                        (4 == attr.mq_curmsgs ),
                        "getattr attributes correct #5" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == sizeof("I love Vik"),
                        "receive message length (prioritized) #1" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, "I love Vik",
                                        sizeof("I love Vik")),
                        "received message data intact (prioritized) #1" );
    CYG_TEST_PASS_FAIL( 7 == prio,
                        "received at correct priority (prioritized) #1" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == sizeof("a lot!"),
                        "receive message length (prioritized) #2" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, "a lot!",
                                        sizeof("a lot!")),
                        "received message data intact (prioritized) #2" );
    CYG_TEST_PASS_FAIL( 7 == prio,
                        "received at correct priority (prioritized) #2" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == sizeof("Vik is a babe"),
                        "receive message length (prioritized) #3" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, "Vik is a babe",
                                        sizeof("Vik is a babe")),
                        "received message data intact (prioritized) #3" );
    CYG_TEST_PASS_FAIL( 6 == prio,
                        "received at correct priority (prioritized) #3" );

    recvlen = mq_receive( q2, buf, 0, &prio );
    CYG_TEST_PASS_FAIL( recvlen == (ssize_t)-1,
                        "mq_receive, zero-sized buffer" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == sizeof(""),
                        "receive message length (prioritized) #4" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, "",
                                        sizeof("")),
                        "received message data intact (prioritized) #4" );
    CYG_TEST_PASS_FAIL( 5 == prio,
                        "received at correct priority (prioritzed) #4" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == (ssize_t)-1,
                        "mq_receive, empty buffer, non-blocking #2" );
    CYG_TEST_PASS_FAIL( EAGAIN == errno,
                        "errno correct for non-blocking #2" );
    
    err = mq_send( q1, "12345678901234567890", 20, 15 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_send (before closing)" );
    
    err = mq_unlink( "/foo" );
    CYG_TEST_PASS_FAIL( -1 == err, "mq_unlink (wrong name)" );
    CYG_TEST_PASS_FAIL( ENOENT == errno,
                        "errno correct for mq_unlink (wrong name)" );

    err = mq_unlink( "/mq1" );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_unlink (before closing)" );

    err = mq_close( q1 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_close (send descriptor)" );

    recvlen = mq_receive( q2, buf, sizeof(buf), &prio );
    CYG_TEST_PASS_FAIL( recvlen == 20,
                        "receive message length (mid close)" );
    CYG_TEST_PASS_FAIL( 0 == my_memcmp( buf, "12345678901234567890", 20 ),
                        "received message data intact (mid close)" );
    CYG_TEST_PASS_FAIL( 15 == prio,
                        "received at correct priority (mid close)" );

    err = mq_close( q2 );
    CYG_TEST_PASS_FAIL( 0 == err, "mq_close (receive descriptor)" );

    q1 = mq_open( "/mq1", O_RDONLY );
    CYG_TEST_PASS_FAIL( q1 == (mqd_t)-1, "error for non-existent queue" );
    CYG_TEST_PASS_FAIL( ENOENT == errno,
                        "errno correct for non-existent queue" );

    CYG_TEST_EXIT("POSIX message test 1");

    return 0;
} // main()

//------------------------------------------------------------------------

#endif

/* EOF mqueue1.c */
