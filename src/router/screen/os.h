/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 * $Id$ GNU
 */

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <fcntl.h>

#include <limits.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#if defined(HAVE_SETRESUID) && !defined(HAVE_SETREUID)
# define setreuid(ruid, euid) setresuid(ruid, euid, -1)
# define setregid(rgid, egid) setresgid(rgid, egid, -1)
#endif

#ifndef HAVE_UTIMES
# define utimes utime
#endif

#ifdef ENABLE_TELNET
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

/*****************************************************************
 *    terminal handling
 */

#include <termios.h>
#ifdef NCCS
# define MAXCC NCCS
#else
# define MAXCC 256
#endif

#ifndef VDISABLE
# ifdef _POSIX_VDISABLE
#  define VDISABLE _POSIX_VDISABLE
# else
#  define VDISABLE 0377
# endif /* _POSIX_VDISABLE */
#endif /* !VDISABLE */

/*****************************************************************
 *   utmp handling
 */

#if defined(ENABLE_UTMP)
# include <utmpx.h>

typedef char* slot_t;	/* used internally in utmp.c */

# ifndef UTMPFILE
#  ifdef UTMPX_FILE
#   define UTMPXFILE	UTMPX_FILE
#  else
#   ifdef _PATH_UTMPX
#    define UTMPXFILE	_PATH_UTMPX
#   else
#    define UTMPXFILE	"/etc/utmpx"
#   endif /* _PATH_UTMPX */
#  endif
# endif

#endif /* ENABLE_UTMP */

/*****************************************************************
 *    signal stuff
 */

/* apparently NSIG is not part of standard, but it's present some form in most
 * libc headers, if not define sane default
 */
#if !defined(NSIG)
# if defined(_NSIG)
#  define NSIG _NSIG
# else
#  define NSIG 32
# endif
#endif

/*****************************************************************
 *    file stuff
 */

/*
 * SunOS 4.1.3: `man 2V open' has only one line that mentions O_NOBLOCK:
 *
 *     O_NONBLOCK     Same as O_NDELAY above.
 *
 * on the very same SunOS 4.1.3, I traced the open system call and found
 * that an open("/dev/ttyy08", O_RDWR|O_NONBLOCK|O_NOCTTY) was blocked,
 * whereas open("/dev/ttyy08", O_RDWR|O_NDELAY  |O_NOCTTY) went through.
 *
 * For this simple reason I now favour O_NDELAY. jw. 4.5.95
 */
#if !defined(O_NONBLOCK) && defined(O_NDELAY)
# define O_NONBLOCK O_NDELAY
#endif

#if !defined(FNBLOCK) && defined(FNONBLOCK)
# define FNBLOCK FNONBLOCK
#endif
#if !defined(FNBLOCK) && defined(FNDELAY)
# define FNBLOCK FNDELAY
#endif
#if !defined(FNBLOCK) && defined(O_NONBLOCK)
# define FNBLOCK O_NONBLOCK
#endif

/*****************************************************************
 *    user defineable stuff
 */

#ifndef TERMCAP_BUFSIZE
# define TERMCAP_BUFSIZE 1023
#endif

/*
 * you may try to vary this value. Use low values if your (VMS) system
 * tends to choke when pasting. Use high values if you want to test
 * how many characters your pty's can buffer.
 */
#define IOSIZE		4096

/* Changing those you won't be able to attach to your old sessions
 * when changing those values in official tree don't forget to bump
 * MSG_VERSION */
#define MAXTERMLEN	32
#define MAXLOGINLEN	256

