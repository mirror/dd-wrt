/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#ifndef SL_CONFIG_H
#define SL_CONFIG_H

/* This configuration file is for all non-Unix OS */
#ifdef VMS
# ifdef __DECC
#  define HAVE_STDLIB_H
# endif
/* Borland's compiler complains about long constants, so I will work
 * around it as follows:
 */
# define VMS_VERSION_602 60200000
# define VMS_VERSION_700 70000000
# if __VMS_VER >= VMS_VERSION_602
#  define HAVE_UNISTD_H
# endif
#endif

/* [JMS:BC5] The Borland v5.x compiler defines some things that other versions,
 * and MSC don't, so we can check for those using ifdef __BORLAND_V5__
 */

#if defined(__BORLANDC__) && ((__BORLANDC__>>8) == 0x5)
# define __BORLAND_V5__
#endif

#if defined(__WATCOMC__) && defined(__DOS__)
# define  DOS386
#endif

/* Set of the various defines for pc systems.  This includes OS/2 */

/* DJGPP has a split personality.  It tries implement a unix like environment
 * under MSDOS.  Unfortunately, the personalities clash.
 */
#ifdef __GO32__
# ifndef __DJGPP__
#  define __DJGPP__ 1
# endif
#endif

#if defined(__DJGPP__) || defined(__CYGWIN32__) || defined(__MINGW32__)
# ifdef REAL_UNIX_SYSTEM
#  undef REAL_UNIX_SYSTEM
# endif
#endif

#if defined(__MSDOS__) || defined(__DOS__)
# ifndef __MSDOS__
#  define __MSDOS__
# endif
# ifndef IBMPC_SYSTEM
#  define IBMPC_SYSTEM
# endif
#endif

#if defined(OS2) || defined(__OS2__)
# ifndef IBMPC_SYSTEM
#   define IBMPC_SYSTEM
# endif
# ifndef __os2__
#  define __os2__
# endif
#endif

#if defined(__CYGWIN32__) || defined(__MINGW32__)
# ifndef __WIN32__
#  define __WIN32__
# endif
#endif

#if defined(WIN32) || defined(__WIN32__)
# ifndef IBMPC_SYSTEM
#  define IBMPC_SYSTEM
# endif
# ifndef __WIN32__
#  define __WIN32__
# endif
#endif

#if defined(__MSDOS__) && !defined(__GO32__) && !defined(DOS386) && !defined(__WIN32__)
# ifndef __MSDOS_16BIT__
#  define __MSDOS_16BIT__	1
# endif
#endif

#if defined(MSWINDOWS)
# ifndef IBMPC_SYSTEM
#  define IBMPC_SYSTEM
# endif
# ifndef __WIN16__
#  define __WIN16__
# endif
#endif

#if defined(__WIN16__) || defined(__MSDOS_16BIT__)
# define __16_BIT_SYSTEM__	1
#endif

#if defined(__WATCOMC__) && !defined(__QNX__)
# undef unix
# undef __unix__
#endif /* __WATCOMC__ */

#ifdef IBMPC_SYSTEM
# define HAVE_STDLIB_H
# define HAVE_PUTENV
# if defined(__GO32__) || defined(__MINGW32__) || defined(__CYGWIN32__)
#  define HAVE_UNISTD_H
# endif
#endif

#ifdef VMS
# if __VMS_VER >= VMS_VERSION_700
#  define HAVE_MEMORY_H
# endif
#else
# if !defined(__WATCOMC__)
#  define HAVE_MEMORY_H
# endif
#endif

#define HAVE_MEMCPY
#define HAVE_MEMSET

#if !defined(VMS) || (__VMS_VER >= VMS_VERSION_700)
# define HAVE_MEMCMP
# define HAVE_MEMCHR
# define HAVE_FCNTL_H
#endif

#define HAVE_GETCWD 1

#ifndef VMS
# define HAVE_VFSCANF	1
# define HAVE_STRTOD	1
#endif

/* Does VMS have this??? */
#if !defined(VMS) && !defined(_MSC_VER) && (!defined(__WATCOMC__) || defined(__QNX__))
# define HAVE_DIRENT_H	1
#endif

#if defined(VMS) || (defined(__WATCOMC__) && !defined(__QNX__))
# define HAVE_DIRECT_H 1
#endif

#if defined(__unix__) || (defined(VMS) && (__VMS_VER >= VMS_VERSION_700))
# define HAVE_KILL 1
# define HAVE_CHOWN 1
#endif

#define HAVE_ATEXIT	1

/* Do these systems have these functions?  For now, assume the worst */
#undef HAVE_GETPPID
#undef HAVE_GETGID
#undef HAVE_GETEGID
#undef HAVE_GETEUID
#undef HAVE_SETGID
#undef HAVE_SETPGID
#undef HAVE_SETUID
#undef HAVE_ISSETUGID

/* Needed for tic/toc */
#undef HAVE_TIMES
#undef HAVE_SYS_TIMES_H

#undef HAVE_ALARM
#undef HAVE_PAUSE

#undef HAVE_SYS_TIME_H
#undef HAVE_GETTIMEOFDAY

#undef HAVE_GMTIME
#undef HAVE_GMTIME_R
#undef HAVE_LOCALTIME_R
#undef HAVE_CTIME_R

#ifdef __unix__
# define HAVE_GMTIME	1
#endif
#define HAVE_MKTIME	1

#undef HAVE_SYMLINK
#undef HAVE_LINK
#undef HAVE_READLINK
#undef HAVE_UNAME

#if defined(__MINGW32__)
# define HAVE_UTIME
# define HAVE_UTIME_H
#endif
#undef HAVE_UTIMES

#undef HAVE_POPEN
#if defined(__MINGW32__)
# define HAVE_POPEN 1
#endif

/* #define HAVE_ROUND	1 */
#if !defined(__os2__) && !defined(__WATCOMC__)
# ifndef VMS			       /* FIXME */
#  define HAVE_FINITE	1
#  define HAVE_ISNAN	1
#  define HAVE_ISINF	1
# endif
#endif

#define HAVE_HYPOT	1
#define HAVE_ATAN2	1
#define HAVE_FREXP	1
#undef HAVE_FREXPF
#define HAVE_LDEXP	1
#undef HAVE_LDEXPF

#if defined(__WIN32__)
# define isnan _isnan
# define finite _finite
#endif

/* FPU control unit headers and functions */
#undef HAVE_FLOATINGPOINT_H
#undef HAVE_NAN_H
#undef HAVE_IEEEFP_H
#undef HAVE_FENV_H

#undef HAVE_FECLEAREXCEPT
#undef HAVE_FPSETSTICKY

/* Define if you have the vsnprintf, snprintf functions and they return
 * EOF upon failure.
 */
#undef HAVE_VSNPRINTF
#undef HAVE_SNPRINTF

#ifdef _MSC_VER
# define HAVE_VSNPRINTF 1
# define HAVE_SNPRINTF 1
# if _MSC_VER < 1400
/* Grrr, older versions of Visual Studio have vnsprintf, but with an '_'. */
#  define vsnprintf _vsnprintf
# endif
#endif

/* Some compilers may not have definitions for these. */
/* #define mode_t int */
/* #define pid_t int */
/* #define uid_t int */
/* #define gid_t int */
/* #define off_t int */
/* #define size_t unsigned int */
#define SIZEOF_OFF_T	4
#define SIZEOF_SIZE_T	4

#undef HAVE_LONG_LONG

#define HAVE_SETVBUF

#if defined (__EMX__) || defined(__BORLANDC__) || defined(__IBMC__) || defined(_MSC_VER) || defined(__WATCOMC__)
# define HAVE_IO_H
#endif

#if defined(_MSC_VER) || defined(__IBMC__) || defined(__BORLANDC__) || defined(__MINGW32__) || defined(__WATCOMC__)
# define HAVE_PROCESS_H
#endif

#ifdef _MSC_VER
# define HAVE_POPEN	1
# define popen _popen
# define pclose _pclose
#endif

#ifdef VMS
# define SIZEOF_SHORT	2
# define SIZEOF_INT	4
# if defined(__alpha__) || defined(__ALPHA__) || defined(__alpha)
/* Apparantly, when compaq bought digital, the size of the long was reduced
 * to 4 bytes for the alpha on VMS.  On unix it is still 8 bytes.
 */
#  define SIZEOF_LONG	4
# else
#  define SIZEOF_LONG	4
# endif
# define SIZEOF_FLOAT	4
# define SIZEOF_DOUBLE	8

#else				       /* NOT VMS */

# define SIZEOF_SHORT	2
# if defined(__WIN16__) || defined(__MSDOS_16BIT__)
# define SIZEOF_INT	2
#else
# define SIZEOF_INT	4
#endif
#define SIZEOF_LONG	4
#define SIZEOF_FLOAT	4
#define SIZEOF_DOUBLE	8

#endif				       /* ifdef VMS */

#ifndef HAVE_LONG_LONG
# undef SIZEOF_LONG_LONG
# define SIZEOF_LONG_LONG 0
#endif

#ifdef HAVE_LONG_LONG
# define HAVE_ATOLL	1
# define HAVE_STRTOLL	1
#endif

#undef HAVE_FORK
#undef HAVE_WAITPID
#undef HAVE_TTYNAME
#undef HAVE_TTYNAME_R

/* Files used by the slang-readline interface.  The interface first tries
 * to read SLRLINE_USER_INIT_FILE from $HOME, and if that does not exist, it
 * will read SLRLINE_SYS_INIT_FILE from the load path.
 */
#define SLRLINE_USER_INIT_FILE	"slrline.rc"
#define SLRLINE_SYS_INIT_FILE	"rline/slrline.rc"

#endif				       /* ifndef SL_CONFIG_H */
