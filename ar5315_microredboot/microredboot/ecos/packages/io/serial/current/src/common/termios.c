/* ====================================================================
//
//      termios.c
//
//      POSIX termios API implementation
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-07-22
// Purpose:      POSIX termios API support
// Description:  Most of the real work happens in the POSIX termios tty
//               drivers
//
//####DESCRIPTIONEND####
//
// ==================================================================*/

// CONFIGURATION

#include <pkgconf/io_serial.h>

#ifdef CYGPKG_IO_SERIAL_TERMIOS

// INCLUDES

#include <termios.h>             // Header for this file
#include <cyg/infra/cyg_type.h>  // Common stuff
#include <cyg/infra/cyg_ass.h>   // Assertion support
#include <cyg/infra/cyg_trac.h>  // Tracing support
#include <cyg/io/serialio.h>     // eCos serial implementation
#include <cyg/fileio/fileio.h>   // file operations
#include <cyg/io/io.h>
#include <errno.h>               // errno
#include <unistd.h>              // isatty()

// TYPES

typedef struct {
    const struct termios *termios_p;
    int optact;
} setattr_struct;

// FUNCTIONS

extern speed_t
cfgetospeed( const struct termios *termios_p )
{
    CYG_REPORT_FUNCTYPE( "returning speed code %d" );
    CYG_CHECK_DATA_PTRC( termios_p );
    CYG_REPORT_FUNCARG1XV( termios_p );
    CYG_REPORT_RETVAL( termios_p->c_ospeed );
    return termios_p->c_ospeed;
} // cfgetospeed()


extern int
cfsetospeed( struct termios *termios_p, speed_t speed )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_CHECK_DATA_PTRC( termios_p );
    CYG_REPORT_FUNCARG2( "termios_p=%08x, speed=%d", termios_p, speed );
    CYG_REPORT_RETVAL( termios_p->c_ospeed );
    if ( speed > B4000000 ) {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    termios_p->c_ospeed = speed;
    CYG_REPORT_RETVAL( 0 );
    return 0;
} // cfsetospeed()


extern speed_t
cfgetispeed( const struct termios *termios_p )
{
    CYG_REPORT_FUNCTYPE( "returning speed code %d" );
    CYG_CHECK_DATA_PTRC( termios_p );
    CYG_REPORT_FUNCARG1XV( termios_p );
    CYG_REPORT_RETVAL( termios_p->c_ispeed );
    return termios_p->c_ispeed;
} // cfgetispeed()


extern int
cfsetispeed( struct termios *termios_p, speed_t speed )
{
    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_CHECK_DATA_PTRC( termios_p );
    CYG_REPORT_FUNCARG2( "termios_p=%08x, speed=%d", termios_p, speed );
    if ( speed > B115200 ) {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    termios_p->c_ispeed = speed;
    CYG_REPORT_RETVAL( 0 );
    return 0;
} // cfsetispeed()


__externC cyg_file *
cyg_fp_get( int fd );

__externC void
cyg_fp_free( cyg_file *fp );

extern int
tcgetattr( int fildes, struct termios *termios_p )
{
    cyg_file *fp;
    int ret;
    int len = sizeof( *termios_p );

    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG2( "fildes=%d, termios_p=%08x", fildes, termios_p );
    CYG_CHECK_DATA_PTRC( termios_p );

    if ( !isatty(fildes) ) {
        errno = ENOTTY;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
        
    fp = cyg_fp_get( fildes );

    if ( NULL == fp ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    ret = fp->f_ops->fo_getinfo( fp, CYG_IO_GET_CONFIG_TERMIOS, termios_p,
                                 len);
    cyg_fp_free( fp );

    if ( ret > 0 ) {
        errno = ret;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    CYG_REPORT_RETVAL( 0 );
    return 0;        
} // tcgetattr()


extern int
tcsetattr( int fildes, int optact, const struct termios *termios_p )
{
    cyg_file *fp;
    int ret;
    setattr_struct attr;
    int len = sizeof( attr );

    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG3( "fildes=%d, optact=%d, termios_p=%08x",
                         fildes, optact, termios_p );
    CYG_CHECK_DATA_PTRC( termios_p );

    if ( !isatty(fildes) ) {
        errno = ENOTTY;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    if ( (optact != TCSANOW) && (optact != TCSADRAIN) &&
         (optact != TCSAFLUSH) ) {
        errno = EINVAL;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
        
    fp = cyg_fp_get( fildes );

    if ( NULL == fp ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    attr.termios_p = termios_p;
    attr.optact = optact;

    ret = fp->f_ops->fo_setinfo( fp, CYG_IO_SET_CONFIG_TERMIOS, &attr,
                                 len);

    cyg_fp_free( fp );

    if ( ret > 0 ) {
        errno = ret;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    CYG_REPORT_RETVAL( 0 );
    return 0;        
} // tcsetattr()


extern int
tcsendbreak( int fildes, int duration )
{
    // FIXME
    return -EINVAL;
} // tcsendbreak()

extern int
tcdrain( int fildes )
{
    cyg_file *fp;
    int ret;

    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG1DV( fildes );

    if ( !isatty(fildes) ) {
        errno = ENOTTY;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    fp = cyg_fp_get( fildes );

    if ( NULL == fp ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    ret = fp->f_ops->fo_getinfo( fp,
                                 CYG_IO_GET_CONFIG_SERIAL_OUTPUT_DRAIN,
                                 NULL, 0 );
    cyg_fp_free( fp );

    if ( ret > 0 ) {
        errno = ret;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    CYG_REPORT_RETVAL( 0 );
    return 0;        
} // tcdrain()

extern int
tcflush( int fildes, int queue_sel )
{
    cyg_file *fp;
    int ret;

    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG2DV( fildes, queue_sel );

    if ( !isatty(fildes) ) {
        errno = ENOTTY;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    fp = cyg_fp_get( fildes );

    if ( NULL == fp ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    switch( queue_sel ) {
    case TCIOFLUSH:
        ret = fp->f_ops->fo_getinfo( fp,
                                     CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH,
                                     NULL, 0 );
        // fallthrough
    case TCIFLUSH:
        ret = fp->f_ops->fo_getinfo( fp,
                                     CYG_IO_GET_CONFIG_SERIAL_INPUT_FLUSH,
                                     NULL, 0 );
        break;
    case TCOFLUSH:
        ret = fp->f_ops->fo_getinfo( fp,
                                     CYG_IO_GET_CONFIG_SERIAL_OUTPUT_FLUSH,
                                     NULL, 0 );
        break;
    default:
        ret = EINVAL;
        break;
    }

    cyg_fp_free( fp );

    if ( ret > 0 ) {
        errno = ret;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    CYG_REPORT_RETVAL( 0 );
    return 0;        
} // tcflush()

extern int
tcflow( int fildes, int action )
{
    cyg_file *fp;
    int ret;
    cyg_uint32 forcearg;
    int len = sizeof(forcearg);

    CYG_REPORT_FUNCTYPE( "returning %d" );
    CYG_REPORT_FUNCARG2DV( fildes, action );

    if ( !isatty(fildes) ) {
        errno = ENOTTY;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    fp = cyg_fp_get( fildes );

    if ( NULL == fp ) {
        errno = EBADF;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }

    switch( action ) {
    case TCOOFF:
        forcearg = CYGNUM_SERIAL_FLOW_THROTTLE_TX;
        ret = fp->f_ops->fo_setinfo( fp,
                                    CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_FORCE,
                                     &forcearg, len );
        break;
    case TCOON:
        forcearg = CYGNUM_SERIAL_FLOW_RESTART_TX;
        ret = fp->f_ops->fo_setinfo( fp,
                                    CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_FORCE,
                                     &forcearg, len );
        break;
    case TCIOFF:
        forcearg = CYGNUM_SERIAL_FLOW_THROTTLE_RX;
        ret = fp->f_ops->fo_setinfo( fp,
                                    CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_FORCE,
                                     &forcearg, len );
        break;
    case TCION:
        forcearg = CYGNUM_SERIAL_FLOW_RESTART_RX;
        ret = fp->f_ops->fo_setinfo( fp,
                                    CYG_IO_SET_CONFIG_SERIAL_FLOW_CONTROL_FORCE,
                                     &forcearg, len );
        break;
    default:
        ret = EINVAL;
        break;
    }

    cyg_fp_free( fp );

    if ( ret > 0 ) {
        errno = ret;
        CYG_REPORT_RETVAL( -1 );
        return -1;
    }
    CYG_REPORT_RETVAL( 0 );
    return 0;        
} // tcflow()

#endif // ifdef CYGPKG_IO_SERIAL_TERMIOS

// EOF termios.c
