/*==========================================================================
//
//      gdb-fileio.c
//
//      Implementation of File I/O using the GDB remote protocol
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
// Author(s):           jlarmour
// Contributors:        
// Date:                2002-04-09
// Purpose:             Implementation of File I/O using the GDB remote
//                      protocol
// Description:         'F' packet requests are of the form:
//                      F<name>[,<parameter>]...
//                      where name is the ASCII syscall name, and the
//                      parameters are generally included as hex ints,
//                      in ASCII.
//
//####DESCRIPTIONEND####
//========================================================================*/

/* CONFIGURATION */

#include <pkgconf/hal.h>

/* HEADERS */

#include <stddef.h>                     // size_t
#include <cyg/infra/cyg_type.h>
#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
# include <string.h>
#endif
#include "board.h"    // sets correct definitions for generic stub header
#include <cyg/hal/generic-stub.h>
#include "gdb-fileio.h"

/* TYPES */

// this is used by newlib's mode_t so we should match it
#ifdef __GNUC__
#define _ST_INT32 __attribute__ ((__mode__ (__SI__)))
#else
#define _ST_INT32
#endif

typedef int             newlib_int_t;
typedef unsigned int    newlib_uint_t;
typedef long            newlib_long_t;
typedef long            newlib_time_t;
typedef unsigned int    newlib_mode_t _ST_INT32;
typedef short           newlib_dev_t;
typedef unsigned short  newlib_uid_t;
typedef unsigned short  newlib_gid_t;
typedef unsigned short  newlib_ino_t;
typedef unsigned short  newlib_nlink_t;
typedef long            newlib_off_t;

struct newlib_timeval {
  newlib_time_t tv_sec;
  newlib_long_t tv_usec;
};

struct newlib_stat 
{
    newlib_dev_t     st_dev;
    newlib_ino_t     st_ino;
    newlib_mode_t    st_mode;
    newlib_nlink_t   st_nlink;
    newlib_uid_t     st_uid;
    newlib_gid_t     st_gid;
    newlib_dev_t     st_rdev;
    newlib_off_t     st_size;
    // We assume we've been compiled with the same flags as newlib here
#if defined(__svr4__) && !defined(__PPC__) && !defined(__sun__)
    newlib_time_t    st_atime;
    newlib_time_t    st_mtime;
    newlib_time_t    st_ctime;
#else
    newlib_time_t    st_atime;
    newlib_long_t    st_spare1;
    newlib_time_t    st_mtime;
    newlib_long_t    st_spare2;
    newlib_time_t    st_ctime;
    newlib_long_t    st_spare3;
    newlib_long_t    st_blksize;
    newlib_long_t    st_blocks;
    newlib_long_t    st_spare4[2];
#endif
};

/* EXTERNS */

__externC char __remcomInBuffer[];  // from generic-stub.c, for packet data
__externC char __remcomOutBuffer[]; // ditto

/* STATICS/GLOBALS */

static int __fileio_retcode, __fileio_errno;
static cyg_bool __fileio_retcode_set, __fileio_errno_set, __fileio_ctrlc_set;

/* MACROS */

// endian independent conversion functions from big endian protocol types
// to newlib types

#define GDBFILEIO_FIO_TO_NEWLIB( _f, _n, _ftype )                  \
CYG_MACRO_START                                                    \
  char *_cf = (char *)(_f);                                        \
  int _i;                                                          \
  char _sign = 0;                                                  \
  if (*_cf == '-') {                                               \
      _sign = 1;                                                   \
      _cf++;                                                       \
  }                                                                \
  (_n) = 0;                                                        \
  for (_i=0; _i<sizeof(_ftype); _i++) {                            \
      (_n) = ((_n) << 8) | _cf[_i];                                \
  }                                                                \
  if (_sign)                                                       \
      (_n) = -(_n);                                                \
CYG_MACRO_END

#define GDBABS(_x_) (((_x_) < 0) ? (-(_x_)) : (_x_))

#define GDBFILEIO_NEWLIB_TO_FIO( _f, _n, _ftype )                  \
CYG_MACRO_START                                                    \
  char *_cf = (char *)(_f);                                        \
  int _i = 0;                                                      \
  if ((_n) < 0)                                                    \
    _cf[_i++] = '-';                                               \
  for (; _i<sizeof(_ftype); _i++) {                                \
      _cf[_i] = ((GDBABS(_n)) >> 8*(sizeof(_ftype)-_i-1)) & 0xff;  \
  }                                                                \
CYG_MACRO_END


/* FUNCTIONS */

#ifndef CYGINT_ISO_STRING_STRFUNCS
static size_t strlen( const char *s )
{
    size_t retval;
    const char *start = s;
    while (*s)
        s++;
    retval = s - start;
    return retval;
}
#endif

static int
chars_to_hex( char *charsin, char *hexout, int bytes )
{
    int numChars = 0;
    int allzero = true;

    while (bytes--) {
        if (0 != *charsin)
            allzero = false;
        *hexout++ = __tohex( (*charsin / 16) & 15 );
        *hexout++ = __tohex( (*charsin++) & 15 );
        numChars += 2;
    }
    if (allzero) // doesn't matter if we actually set more than needed above
        return (numChars > 2 ? 2 : numChars);
    return numChars;
}

static void
gdbfileio_fio_to_newlib_time_t( fio_time_t *f, newlib_time_t *n )
{
    GDBFILEIO_FIO_TO_NEWLIB( f, *n, fio_time_t );
} // gdbfileio_fio_to_newlib_time_t()

static void
gdbfileio_newlib_to_fio_int_t( newlib_int_t *n, fio_int_t *f )
{
    GDBFILEIO_NEWLIB_TO_FIO( f, *n, fio_int_t );
} // gdbfileio_newlib_to_fio_int_t()

static void
gdbfileio_newlib_to_fio_uint_t( newlib_uint_t *n, fio_uint_t *f )
{
    GDBFILEIO_NEWLIB_TO_FIO( f, *n, fio_uint_t );
} // gdbfileio_newlib_to_fio_uint_t()

static void
gdbfileio_fio_to_newlib_long_t( fio_long_t *f, newlib_long_t *n )
{
    GDBFILEIO_FIO_TO_NEWLIB( f, *n, fio_long_t );
} // gdbfileio_fio_to_newlib_long_t()

static void
gdbfileio_newlib_to_fio_long_t( newlib_long_t *n, fio_long_t *f )
{
    GDBFILEIO_NEWLIB_TO_FIO( f, *n, fio_long_t );
} // gdbfileio_newlib_to_fio_long_t()

static void
gdbfileio_fio_to_newlib_mode_t( fio_mode_t *f, newlib_mode_t *n )
{
    GDBFILEIO_FIO_TO_NEWLIB( f, *n, fio_mode_t );
} // gdbfileio_fio_to_newlib_mode_t()

static void
gdbfileio_newlib_to_fio_mode_t( newlib_mode_t *n, fio_mode_t *f )
{
    GDBFILEIO_NEWLIB_TO_FIO( f, *n, fio_mode_t );
} // gdbfileio_newlib_to_fio_mode_t()

static void
gdbfileio_fio_to_newlib_dev_t( fio_uint_t *f, newlib_dev_t *n )
{
    GDBFILEIO_FIO_TO_NEWLIB( f, *n, fio_uint_t );
} // gdbfileio_fio_to_newlib_dev_t()

static void
gdbfileio_fio_to_newlib_ino_t( fio_uint_t *f, newlib_ino_t *n )
{
    GDBFILEIO_FIO_TO_NEWLIB( f, *n, fio_uint_t );
} // gdbfileio_fio_to_newlib_ino_t()

// these defines are good enough for now (to save code size) as they
// are the same functions in practice
#define gdbfileio_fio_to_newlib_nlink_t gdbfileio_fio_to_newlib_ino_t
#define gdbfileio_fio_to_newlib_uid_t   gdbfileio_fio_to_newlib_ino_t
#define gdbfileio_fio_to_newlib_gid_t   gdbfileio_fio_to_newlib_ino_t
#define gdbfileio_fio_to_newlib_off_t   gdbfileio_fio_to_newlib_long_t


// this function is commonly used by most functions to handle everything
// once the packet has been constructed. It doesn't have to be used - it's
// just nice to keep this in one place for maintenance reasons.
static int
gdbfileio_common_sendpkt( char *buf, int *sig )
{
    int status;

    __putpacket( buf );
    
    do {
        __getpacket( __remcomInBuffer );
        status = __process_packet( __remcomInBuffer );
    } while ( status == 0 );

    if ( __fileio_ctrlc_set )
        *sig = SIGINT;
    if ( !__fileio_retcode_set ) // deal with protocol failure
        return -FILEIO_EINVAL;
    if ( __fileio_retcode < 0 && __fileio_errno_set )
        return -__fileio_errno;
    else
        return __fileio_retcode;
} // gdbfileio_common_sendpkt()

// deal with a received F packet. This is called from __process_packet in
// generic-stub.c
__externC void
cyg_hal_gdbfileio_process_F_packet( char *packet,
                    char *__remcomOutBuffer )
{
    // Reply packet structure:
    // F<retcode>[,<errno>[,<Ctrl-C flag>]][;<call specific attachment>]

    char *p = &packet[1];
    cyg_bool minus = false;
    target_register_t temptrt;

    __fileio_retcode_set = __fileio_errno_set = __fileio_ctrlc_set = false;

    if (*p == '-') {
        minus = true;
        p++;
    }
        
    __hexToInt( &p, &temptrt );
    __fileio_retcode = minus ? -(int)temptrt : (int)temptrt;
    __fileio_retcode_set = true;
    
    if ( *p++ == ',' ) {
        // get errno
        __hexToInt( &p, &temptrt );
        __fileio_errno = (int)temptrt;
        __fileio_errno_set = true;
        if ( *p++ == ',' ) {
            if ( *p == 'C' ) {
                __fileio_ctrlc_set = true;
            }
        }
    }
    // ignore anything afterwards (e.g. call specific attachment) for now
    
} // cyg_hal_gdbfileio_process_F_packet()
    
__externC int
cyg_hal_gdbfileio_open( const char *name, int flags, int mode, int *sig )
{
    size_t namelen;
    unsigned int i=0;
    fio_mode_t fmode;
    fio_int_t fflags;

    // clear out unsupported flags/modes, as per the spec
    flags &= FILEIO_O_SUPPORTED;
    mode &= FILEIO_S_SUPPORTED;

    gdbfileio_newlib_to_fio_int_t( &flags, &fflags );    
    gdbfileio_newlib_to_fio_mode_t( &mode, &fmode );    

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'o';
    __remcomOutBuffer[i++] = 'p';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'n';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)name,
                     sizeof(name)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = '/';
    namelen = strlen( name )+1; // includes '\0'
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)namelen,
                     sizeof(namelen)*8 );
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&fflags, &__remcomOutBuffer[i], sizeof(fflags) );
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&fmode, &__remcomOutBuffer[i], sizeof(fmode) );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_open()

__externC int
cyg_hal_gdbfileio_close( int fd, int *sig )
{
    unsigned int i=0;
    fio_int_t ffd;

    gdbfileio_newlib_to_fio_int_t( &fd, &ffd );
    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'c';
    __remcomOutBuffer[i++] = 'l';
    __remcomOutBuffer[i++] = 'o';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&ffd, &__remcomOutBuffer[i], sizeof(ffd) );
    // i now points after the parameter
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_close()

__externC int
cyg_hal_gdbfileio_read( int fd, void *buf, size_t count, int *sig )
{
    unsigned int i=0;
    fio_int_t ffd;
    fio_uint_t fcount;
    unsigned int uicount = (unsigned int)count;

    gdbfileio_newlib_to_fio_int_t( &fd, &ffd );
    gdbfileio_newlib_to_fio_uint_t( &uicount, &fcount );

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'r';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'a';
    __remcomOutBuffer[i++] = 'd';
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&ffd, &__remcomOutBuffer[i], sizeof(ffd) );
    // i now points after the parameter
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)buf,
                     sizeof(buf)*8 );
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&fcount, &__remcomOutBuffer[i], sizeof(fcount) );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_read()

__externC int
cyg_hal_gdbfileio_write( int fd, const void *buf, size_t count, int *sig )
{
    unsigned int i=0;
    fio_int_t ffd;
    fio_uint_t fcount;
    unsigned int uicount = (unsigned int)count;

    gdbfileio_newlib_to_fio_int_t( &fd, &ffd );
    gdbfileio_newlib_to_fio_uint_t( &uicount, &fcount );

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'w';
    __remcomOutBuffer[i++] = 'r';
    __remcomOutBuffer[i++] = 'i';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&ffd, &__remcomOutBuffer[i], sizeof(ffd) );
    // i now points after the parameter
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)buf,
                     sizeof(buf)*8 );
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&fcount, &__remcomOutBuffer[i], sizeof(fcount) );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_write()

__externC int
cyg_hal_gdbfileio_lseek( int fd, /* off_t */ long offset, int whence, int *sig )
{
    unsigned int i=0;
    fio_int_t ffd;
    fio_long_t foffset;
    fio_int_t fwhence;

    gdbfileio_newlib_to_fio_int_t( &fd, &ffd );
    gdbfileio_newlib_to_fio_long_t( &offset, &foffset );
    gdbfileio_newlib_to_fio_int_t( &whence, &fwhence );

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'l';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'k';
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&ffd, &__remcomOutBuffer[i], sizeof(ffd) );
    // i now points after the parameter
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&foffset, &__remcomOutBuffer[i],
                       sizeof(foffset) );
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&fwhence, &__remcomOutBuffer[i],
                       sizeof(fwhence) );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_lseek()

__externC int
cyg_hal_gdbfileio_rename( const char *oldpath, const char *newpath, int *sig )
{
    unsigned int i=0;
    size_t namelen;

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'r';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'n';
    __remcomOutBuffer[i++] = 'a';
    __remcomOutBuffer[i++] = 'm';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)oldpath,
                     sizeof(oldpath)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = '/';
    namelen = strlen( oldpath )+1; // includes '\0'
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)namelen,
                     sizeof(namelen)*8 );
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)newpath,
                     sizeof(newpath)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = '/';
    namelen = strlen( newpath )+1; // includes '\0'
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)namelen,
                     sizeof(namelen)*8 );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_rename()

__externC int
cyg_hal_gdbfileio_unlink( const char *pathname, int *sig )
{
    unsigned int i=0;
    size_t namelen;

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'u';
    __remcomOutBuffer[i++] = 'n';
    __remcomOutBuffer[i++] = 'l';
    __remcomOutBuffer[i++] = 'i';
    __remcomOutBuffer[i++] = 'n';
    __remcomOutBuffer[i++] = 'k';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)pathname,
                     sizeof(pathname)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = '/';
    namelen = strlen( pathname )+1; // includes '\0'
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)namelen,
                     sizeof(namelen)*8 );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_unlink()

__externC int
cyg_hal_gdbfileio_isatty( int fd, int *sig )
{
    unsigned int i=0;
    fio_int_t ffd;

    gdbfileio_newlib_to_fio_int_t( &fd, &ffd );

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'i';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 'a';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 'y';
    __remcomOutBuffer[i++] = ',';
    i += chars_to_hex( (char *)&ffd, &__remcomOutBuffer[i], sizeof(ffd) );
    // i now points after the parameter
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_isatty()

__externC int
cyg_hal_gdbfileio_system( const char *command, int *sig )
{
    unsigned int i=0;
    size_t namelen;

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 'y';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'm';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)command,
                     sizeof(command)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = '/';
    namelen = strlen( command )+1; // includes '\0'
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)namelen,
                     sizeof(namelen)*8 );
    __remcomOutBuffer[i] = 0;

    return gdbfileio_common_sendpkt( __remcomOutBuffer, sig );
} // cyg_hal_gdbfileio_system()

__externC int
cyg_hal_gdbfileio_gettimeofday( void *tv, void *tz, int *sig )
{
    unsigned int i=0;
    struct newlib_timeval *ntv = (struct newlib_timeval *)tv;
    struct fio_timeval ftv;
    int rc;

    // protocol doesn't support non-null timezone. Just enforce it here.
    if (NULL != tz)
        return -FILEIO_EINVAL;

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'g';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 'i';
    __remcomOutBuffer[i++] = 'm';
    __remcomOutBuffer[i++] = 'e';
    __remcomOutBuffer[i++] = 'o';
    __remcomOutBuffer[i++] = 'f';
    __remcomOutBuffer[i++] = 'd';
    __remcomOutBuffer[i++] = 'a';
    __remcomOutBuffer[i++] = 'y';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)&ftv,
                     sizeof(&ftv)*8 );
    __remcomOutBuffer[i++] = ',';
    __remcomOutBuffer[i++] = '0'; // tzptr
    __remcomOutBuffer[i] = 0;

    rc = gdbfileio_common_sendpkt( __remcomOutBuffer, sig );

    // now ftv should have its contents filled
    gdbfileio_fio_to_newlib_time_t( &ftv.tv_sec, &ntv->tv_sec );
    gdbfileio_fio_to_newlib_long_t( &ftv.tv_usec, &ntv->tv_usec );

    return rc;
} // cyg_hal_gdbfileio_gettimeofday()

__externC int
cyg_hal_gdbfileio_stat( const char *pathname, struct newlib_stat *buf,
                        int *sig )
{
    unsigned int i=0;
    int rc;
    size_t namelen;
    struct fio_stat fbuf;

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 'a';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)pathname,
                     sizeof(pathname)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = '/';
    namelen = strlen( pathname )+1; // includes '\0'
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)namelen,
                     sizeof(namelen)*8 );
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)&fbuf,
                     sizeof(&fbuf)*8 );
    __remcomOutBuffer[i] = 0;

    rc = gdbfileio_common_sendpkt( __remcomOutBuffer, sig );

    // now fbuf should have its contents filled
    gdbfileio_fio_to_newlib_dev_t( &fbuf.st_dev, &buf->st_dev );
    gdbfileio_fio_to_newlib_ino_t( &fbuf.st_ino, &buf->st_ino );
    gdbfileio_fio_to_newlib_mode_t( &fbuf.st_mode, &buf->st_mode );
    gdbfileio_fio_to_newlib_nlink_t( &fbuf.st_nlink, &buf->st_nlink );
    gdbfileio_fio_to_newlib_uid_t( &fbuf.st_uid, &buf->st_uid );
    gdbfileio_fio_to_newlib_gid_t( &fbuf.st_gid, &buf->st_gid );
    gdbfileio_fio_to_newlib_dev_t( &fbuf.st_rdev, &buf->st_rdev );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
#if !defined(__svr4__) || defined(__PPC__) || defined(__sun__)
    gdbfileio_fio_to_newlib_long_t( &fbuf.st_blksize, &buf->st_blksize );
    gdbfileio_fio_to_newlib_long_t( &fbuf.st_blocks, &buf->st_blocks );
#endif
    gdbfileio_fio_to_newlib_time_t( &fbuf.st_atime, &buf->st_atime );
    gdbfileio_fio_to_newlib_time_t( &fbuf.st_mtime, &buf->st_mtime );
    gdbfileio_fio_to_newlib_time_t( &fbuf.st_ctime, &buf->st_ctime );

    return rc;
} // cyg_hal_gdbfileio_stat()

__externC int
cyg_hal_gdbfileio_fstat( int fd, struct newlib_stat *buf, int *sig )
{
    unsigned int i=0;
    int rc;
    struct fio_stat fbuf;

    __remcomOutBuffer[i++] = 'F';
    __remcomOutBuffer[i++] = 'f';
    __remcomOutBuffer[i++] = 's';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = 'a';
    __remcomOutBuffer[i++] = 't';
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)fd,
                     sizeof(fd)*8 );
    // i now points after the parameter
    __remcomOutBuffer[i++] = ',';
    i += __intToHex( &__remcomOutBuffer[i], (target_register_t)&fbuf,
                     sizeof(&fbuf)*8 );
    __remcomOutBuffer[i] = 0;

    rc = gdbfileio_common_sendpkt( __remcomOutBuffer, sig );

    // now fbuf should have its contents filled
    gdbfileio_fio_to_newlib_dev_t( &fbuf.st_dev, &buf->st_dev );
    gdbfileio_fio_to_newlib_ino_t( &fbuf.st_ino, &buf->st_ino );
    gdbfileio_fio_to_newlib_mode_t( &fbuf.st_mode, &buf->st_mode );
    gdbfileio_fio_to_newlib_nlink_t( &fbuf.st_nlink, &buf->st_nlink );
    gdbfileio_fio_to_newlib_uid_t( &fbuf.st_uid, &buf->st_uid );
    gdbfileio_fio_to_newlib_gid_t( &fbuf.st_gid, &buf->st_gid );
    gdbfileio_fio_to_newlib_dev_t( &fbuf.st_rdev, &buf->st_rdev );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
    gdbfileio_fio_to_newlib_off_t( &fbuf.st_size, &buf->st_size );
#if !defined(__svr4__) || defined(__PPC__) || defined(__sun__)
    gdbfileio_fio_to_newlib_long_t( &fbuf.st_blksize, &buf->st_blksize );
    gdbfileio_fio_to_newlib_long_t( &fbuf.st_blocks, &buf->st_blocks );
#endif
    gdbfileio_fio_to_newlib_time_t( &fbuf.st_atime, &buf->st_atime );
    gdbfileio_fio_to_newlib_time_t( &fbuf.st_mtime, &buf->st_mtime );
    gdbfileio_fio_to_newlib_time_t( &fbuf.st_ctime, &buf->st_ctime );

    return rc;
} // cyg_hal_gdbfileio_fstat()

/* EOF gdb-fileio.c */
