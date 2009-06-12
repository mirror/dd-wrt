//===========================================================================
//
//      strerror.cxx
//
//      ANSI error code string routine
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         2000-04-14
// Purpose:      To provide the strerror() implementation
// Description:  This implements strerror() as described in ANSI chap 7.11.6.2
// Usage:        See <cyg/error/codes.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================


// CONFIGURATION

#include <pkgconf/error.h>   // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>   // Common project-wide type definitions
#include <cyg/infra/cyg_trac.h>   // Tracing support
#include <cyg/error/codes.h>      // Error code definitions and header for this
                                  // file

// EXPORTED SYMBOLS

externC char *
strerror( int errnum ) __attribute__ ((weak, alias("__strerror") ));

// FUNCTIONS

externC char *
__strerror( int errnum )
{
    register char *s;
    
    CYG_REPORT_FUNCNAMETYPE( "__strerror", "String form of error is \"%s\"" );

    switch (errnum)
    {

#ifdef ENOERR
    case ENOERR:
        s = "No error";
        break;
#endif

#ifdef EPERM
    case EPERM:
        s = "Not permitted";
        break;
#endif

#ifdef ENOENT
    case ENOENT:
        s = "No such entity";
        break;
#endif

#ifdef ESRCH
    case ESRCH:
        s = "No such process";
        break;
#endif

#ifdef EINTR
    case EINTR:
        s = "Operation interrupted";
        break;
#endif

#ifdef EIO
    case EIO:
        s = "I/O error";
        break;
#endif

#ifdef EBADF
    case EBADF:
        s = "Bad file handle";
        break;
#endif

#ifdef EAGAIN
    case EAGAIN:
        s = "Try again later";
        break;
#endif

#ifdef ENOMEM
    case ENOMEM:
        s = "Out of memory";
        break;
#endif

#ifdef EBUSY
    case EBUSY:
        s = "Resource busy";
        break;
#endif

#ifdef ENODEV
    case ENODEV:
        s = "No such device";
        break;
#endif

#ifdef ENOTDIR
    case ENOTDIR:
        s = "Not a directory";
        break;
#endif

#ifdef EISDIR
    case EISDIR:
        s = "Is a directory";
        break;
#endif

#ifdef EINVAL
    case EINVAL:
        s = "Invalid argument";
        break;
#endif

#ifdef ENFILE
    case ENFILE:
        s = "Too many open files in system";
        break;
#endif

#ifdef EMFILE
    case EMFILE:
        s = "Too many open files";
        break;
#endif

#ifdef EFBIG
    case EFBIG:
        s = "File too large";
        break;
#endif
        
#ifdef ENOSPC
    case ENOSPC:
        s = "No space left on device";
        break;
#endif

#ifdef ESPIPE
    case ESPIPE:
        s = "Illegal seek";
        break;
#endif
        
#ifdef EROFS
    case EROFS:
        s = "Read-only file system";
        break;
#endif
        
#ifdef EDOM
    case EDOM:
        s = "Argument to math function outside valid domain";
        break;
#endif

#ifdef ERANGE
    case ERANGE:
        s = "Math result cannot be represented";
        break;
#endif

#ifdef EDEADLK
    case EDEADLK:
        s = "Resource deadlock would occur";
        break;
#endif

#ifdef ENOSYS
    case ENOSYS:
        s = "Function not implemented";
        break;
#endif

#ifdef ENAMETOOLONG
    case ENAMETOOLONG:
        s = "File name too long";
        break;
#endif
        
#ifdef ENOTSUP
    case ENOTSUP:
        s = "Not supported";
        break;
#endif

#ifdef EEOF
    case EEOF:
        s = "End of file reached";
        break;
#endif

#ifdef ENOSUPP
    case ENOSUPP:
        s = "Operation not supported";
        break;
#endif

#ifdef EDEVNOSUPP
    case EDEVNOSUPP:
        s = "Device does not support this operation";
        break;
#endif

#ifdef EXDEV
    case EXDEV:
        s = "Improper link";
        break;
#endif
        
// Additional errors used by networking
#ifdef ENXIO
    case ENXIO:
        s =  "Device not configured";
        break;
#endif
#ifdef EACCES
    case EACCES:
        s =  "Permission denied";
        break;
#endif
#ifdef EEXIST
    case EEXIST:
        s =  "File exists";
        break;
#endif
#ifdef ENOTTY
    case ENOTTY:
        s =  "Inappropriate ioctl for device";
        break;
#endif
#ifdef EPIPE
    case EPIPE:
        s =  "Broken pipe";
        break;
#endif
#ifdef EINPROGRESS
    case EINPROGRESS:
        s =  "Operation now in progress";
        break;
#endif
#ifdef EALREADY
    case EALREADY:
        s =  "Operation already in progress";
        break;
#endif
#ifdef ENOTSOCK
    case ENOTSOCK:
        s =  "Socket operation on non-socket";
        break;
#endif
#ifdef EDESTADDRREQ
    case EDESTADDRREQ:
        s =  "Destination address required";
        break;
#endif
#ifdef EMSGSIZE
    case EMSGSIZE:
        s =  "Message too long";
        break;
#endif
#ifdef EPROTOTYPE
    case EPROTOTYPE:
        s =  "Protocol wrong type for socket";
        break;
#endif
#ifdef ENOPROTOOPT
    case ENOPROTOOPT:
        s =  "Protocol not available";
        break;
#endif
#ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT:
        s =  "Protocol not supported";
        break;
#endif
#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT:
        s =  "Socket type not supported";
        break;
#endif
#ifdef EOPNOTSUPP
    case EOPNOTSUPP:
        s =  "Operation not supported";
        break;
#endif
#ifdef EPFNOSUPPORT
    case EPFNOSUPPORT:
        s =  "Protocol family not supported";
        break;
#endif
#ifdef EAFNOSUPPORT
    case EAFNOSUPPORT:
        s =  "Address family not supported by protocol family";
        break;
#endif
#ifdef EADDRINUSE
    case EADDRINUSE:
        s =  "Address already in use";
        break;
#endif
#ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL:
        s =  "Can't assign requested address";
        break;
#endif
#ifdef ENETDOWN
    case ENETDOWN:
        s =  "Network is down";
        break;
#endif
#ifdef ENETUNREACH
    case ENETUNREACH:
        s =  "Network is unreachable";
        break;
#endif
#ifdef ENETRESET
    case ENETRESET:
        s =  "Network dropped connection on reset";
        break;
#endif
#ifdef ECONNABORTED
    case ECONNABORTED:
        s =  "Software caused connection abort";
        break;
#endif
#ifdef ECONNRESET
    case ECONNRESET:
        s =  "Connection reset by peer";
        break;
#endif
#ifdef ENOBUFS
    case ENOBUFS:
        s =  "No buffer space available";
        break;
#endif
#ifdef EISCONN
    case EISCONN:
        s =  "Socket is already connected";
        break;
#endif
#ifdef ENOTCONN
    case ENOTCONN:
        s =  "Socket is not connected";
        break;
#endif
#ifdef ESHUTDOWN
    case ESHUTDOWN:
        s =  "Can't send after socket shutdown";
        break;
#endif
#ifdef ETOOMANYREFS
    case ETOOMANYREFS:
        s =  "Too many references: can't splice";
        break;
#endif
#ifdef ETIMEDOUT
    case ETIMEDOUT:
        s =  "Operation timed out";
        break;
#endif
#ifdef ECONNREFUSED
    case ECONNREFUSED:
        s =  "Connection refused";
        break;
#endif
#ifdef EHOSTDOWN
    case EHOSTDOWN:
        s =  "Host is down";
        break;
#endif
#ifdef EHOSTUNREACH
    case EHOSTUNREACH:
        s =  "No route to host";
        break;
#endif

    default:
        s = "Unknown error";
        break;

    } // switch

    CYG_REPORT_RETVAL(s);

    return s;
} // __strerror()

// EOF strerror.cxx
