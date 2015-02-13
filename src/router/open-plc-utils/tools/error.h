/*====================================================================*
 *
 *   error.h - error function definitions and declarations;
 *
 *   this file is an alterantive to GNU header file of the same
 *   name; in addition to standard GNU error function declarations,
 *   some additional functions are declared;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef ERROR_HEADER
#define ERROR_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#if defined (WIN32)
#define __func__ __FUNCTION__
#endif

/*====================================================================*
 *   define error codes for systems that do not support POSIX codes;
 *--------------------------------------------------------------------*/

#ifndef ECANCELED
#define ECANCELED 0
#endif
#ifndef ENOTSUP
#define ENOTSUP EPERM
#endif
#ifndef EBADMSG
#define EBADMSG 0
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT 124
#endif
#ifndef ENODATA
#define ENODATA 0
#endif
#ifndef EOVERFLOW
#define EOVERFLOW 0
#endif

/*====================================================================*
 *   define common error message strings;
 *--------------------------------------------------------------------*/

#define ERROR_NOTROOT "This program needs root privileges"
#define ERROR_TOOMANY "Too many command line arguments"

#define CANT_START_TIMER "function %s can't start timer", __func__
#define CANT_RESET_TIMER "function %s can't reset timer", __func__

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define TRACE error (0, 0, "%s (%d)", __FILE__, __LINE__);

/*====================================================================*
 *   declare GNU error() and error_at_line() functions;
 *--------------------------------------------------------------------*/

#ifdef __GNUC__

__attribute__ ((format (printf, 3, 4)))

#endif

signed error (signed status, errno_t number, char const * format, ...);

#ifdef __GNUC__

__attribute__ ((format (printf, 3, 4)))

#endif

signed debug (signed status, char const * string, char const * format, ...);

signed extra (signed status, errno_t number, int argc, char const * argv []);

/*====================================================================*
 *   end definitions and declarations;
 *--------------------------------------------------------------------*/

#endif

