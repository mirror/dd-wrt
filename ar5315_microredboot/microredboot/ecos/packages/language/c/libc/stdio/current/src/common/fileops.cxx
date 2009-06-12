/*========================================================================
//
//      fileops.cxx
//
//      Implementation of ISO C rename(),remove(),tmpnam(),tmpfile() functions
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 eCosCentric Ltd.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2004-02-30
// Purpose:       Implementation of ISO C rename(),remove(),tmpnam(),tmpfile()
// Description:   
// Usage:         
//
//####DESCRIPTIONEND####
//======================================================================*/

// CONFIGURATION

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/libc_stdio.h>          // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>           // Common type definitions and support
#include <cyg/infra/cyg_ass.h>            // Common assertion functions
#include <cyg/infra/cyg_trac.h>           // Common tracing functions
#include <stdio.h>                        // Header for this file
#include <errno.h>                        // errno
#ifdef CYGPKG_LIBC_STDIO_FILEIO           // unix-y functions, e.g. stat,rmdir,unlink,...
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
#endif
#include <cyg/libc/stdio/io.hxx>          // I/O definitions
#ifdef CYGPKG_POSIX
# include <pkgconf/posix.h>
# include <cyg/posix/export.h>
#endif

#if defined(CYGINT_ISO_EXIT) && (CYGINT_ISO_EXIT+0)
# include <stdlib.h>         // used by tmpfile() for atexit()
#endif

// DEFINES

#ifdef CYGPKG_POSIX
# define CYG_STDIO_FUNCTION_START() CYG_POSIX_FUNCTION_START()
# define CYG_STDIO_FUNCTION_FINISH() CYG_POSIX_FUNCTION_FINISH()
#else
# define CYG_STDIO_FUNCTION_START() CYG_EMPTY_STATEMENT
# define CYG_STDIO_FUNCTION_FINISH() CYG_EMPTY_STATEMENT
#endif

// Handle entry 
#define STDIO_ENTRY()                           \
    CYG_REPORT_FUNCTYPE( "returning %d" );      \
    CYG_STDIO_FUNCTION_START();                 \

#define STDIO_RETURN(err)                       \
CYG_MACRO_START                                 \
    int __retval = 0;                           \
    CYG_STDIO_FUNCTION_FINISH();                \
    if( err != 0 ) __retval = -1, errno = err;  \
    CYG_REPORT_RETVAL( __retval );              \
    return __retval;                            \
CYG_MACRO_END

#define STDIO_RETURN_VALUE(val)                 \
CYG_MACRO_START                                 \
    CYG_STDIO_FUNCTION_FINISH();                \
    CYG_REPORT_RETVAL( val );                   \
    return val;                                 \
CYG_MACRO_END

// FUNCTIONS

///////////////////////////////////////////////////////////////////////////
// remove()

__externC int remove( const char *path ) __THROW
{
    int ret;
    STDIO_ENTRY();
    CYG_CHECK_DATA_PTR( path, "path pointer invalid" );

#ifdef CYGPKG_LIBC_STDIO_FILEIO
    struct stat sbuf;
    ret = stat( path, &sbuf );

    if (0 == ret)
    {
        if ( S_ISDIR(sbuf.st_mode) )
        {
            ret = rmdir( path );
        } else {
            ret = unlink( path );
        }
    }
#else // !defined(CYGPKG_LIBC_STDIO_FILEIO)
    ret = ENOSYS;
#endif    
    STDIO_RETURN(ret);
} // remove()

///////////////////////////////////////////////////////////////////////////
// rename()
//
// The File I/O package supplies its own complete version of this, so we
// only implement a dummy here.

#ifndef CYGPKG_LIBC_STDIO_FILEIO
__externC int rename( const char *oldname, const char *newname ) __THROW
{
    STDIO_ENTRY();
    CYG_CHECK_DATA_PTR(oldname, "oldname pointer invalid");
    CYG_CHECK_DATA_PTR(newname, "newname pointer invalid");
    STDIO_RETURN(ENOSYS);
}
#endif // ifndef CYGPKG_LIBC_STDIO_FILEIO

///////////////////////////////////////////////////////////////////////////
// tmpnam()

__externC char *tmpnam( char *s ) __THROW
{
    STDIO_ENTRY();
    static char staticbuf[ L_tmpnam ];
#if (TMP_MAX < 256)
    typedef cyg_uint8 counttype;
#elif (TMP_MAX < 65536)
    typedef cyg_uint16 counttype;
#else
    typedef cyg_ucount32 counttype;
#endif
    static counttype count;
    counttype totaliters=0;
    int i;

    if ( NULL != s )
        CYG_CHECK_DATA_PTR( s, "supplied string pointer invalid" );
    else
        s = staticbuf;

    // start off by making it "tmp00000" etc. so we can fill backwards
    // from end without spaces
    s[0] = 't'; s[1] = 'm'; s[2] = 'p';

    while (totaliters < TMP_MAX)
    {
        for (i=3; i < (L_tmpnam-1); i++)
        {
            s[i] = '0';
        }
        s[i] = '\0';

        counttype counttmp = count;
        for (i=(L_tmpnam-1); i>2; i--)
        {
            const char tohex[] = "0123456789abcdef";
            s[i] = tohex[counttmp & 0xf];
            counttmp = counttmp >> 4;
        }
        count++;
        count %= TMP_MAX; // cycle round
        totaliters++;

        // s now points to a name candidate
#ifdef CYGPKG_LIBC_STDIO_FILEIO
        int fd = open( s, O_RDONLY );
        if (fd >= 0)
            close(fd);
        else if ( ENOENT == errno ) // we have a winner
            break;
#else
        break; // no real filesystem, so just go with what we've come up with
#endif
    }

    if ( totaliters == TMP_MAX ) // oops, looped right the way round
        s = NULL;

    STDIO_RETURN_VALUE( s );
} // tmpnam()

///////////////////////////////////////////////////////////////////////////
// tmpfile()

__externC FILE *tmpfile( void ) __THROW
{
    FILE *f;
    char fname[L_tmpnam];
    char *s;

    STDIO_ENTRY();

    s = tmpnam( fname );
    if ( s == NULL)
        f = NULL;
    else
    {
        // fname is now a valid name to use
        f = fopen( fname, "wb+" );
#ifdef CYGPKG_LIBC_STDIO_FILEIO
        // We can use remove which should mean the file is removed on program
        // exit.We ignore the return code though - the standard seems to
        // indicate that the return status from this function is solely
        // dictated by whether the file could be created.
        if (f)
            remove( fname );
#endif
    }    

    STDIO_RETURN_VALUE( f );
} // tmpfile()

///////////////////////////////////////////////////////////////////////////
// EOF fileops.cxx
