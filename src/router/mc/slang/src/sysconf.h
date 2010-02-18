/* src/sysconf.h.  Generated from config.hin by configure.  */
/* -*- c -*- */
/* Note: this is for unix only. */

#ifndef SL_CONFIG_H
#define SL_CONFIG_H


/* Files used by the slang-readline interface.  The interface first tries
 * to read SLRLINE_USER_INIT_FILE from $HOME, and if that does not exist, it
 * will read SLRLINE_SYS_INIT_FILE from the interpreter's load path.
 */
#define SLRLINE_USER_INIT_FILE	".slrlinerc"
#define SLRLINE_SYS_INIT_FILE	"rline/slrline.rc"

/* define if you have long long type */
#define HAVE_LONG_LONG 1

/* The following set defines may be necessary to activate long file support */
#define _FILE_OFFSET_BITS 64
/* #undef _LARGE_FILES */
#define _LARGEFILE_SOURCE 1

/* define if you have stdlib.h */
#define HAVE_STDLIB_H 1

/* define if you have unistd.h */
#define HAVE_UNISTD_H 1

/* define if you have termios.h */
#define HAVE_TERMIOS_H 1

/* define if you have memory.h */
#define HAVE_MEMORY_H 1

/* define if you have malloc.h */
#define HAVE_MALLOC_H 1

/* define if you have memset */
#define HAVE_MEMSET 1

/* define if you have memcpy */
#define HAVE_MEMCPY 1

#define HAVE_SETLOCALE 1
#define HAVE_LOCALE_H 1
#define HAVE_LANGINFO_H 1
#define HAVE_NL_LANGINFO_CODESET 1

/* #undef HAVE_FLOATINGPOINT_H */
/* #undef HAVE_NAN_H */
/* #undef HAVE_IEEEFP_H */
/* #undef HAVE_FENV_H */

/* #undef HAVE_FECLEAREXCEPT */
/* #undef HAVE_FPSETSTICKY */

#define HAVE_VFSCANF 1
#define HAVE_STRTOD 1
#define HAVE_ATOLL 1
#define HAVE_STRTOLL 1

/* define if you have fcntl.h */
#define HAVE_FCNTL_H 1

/* Define if you have the vsnprintf, snprintf functions and they return
 * EOF upon failure.
 */
#define HAVE_VSNPRINTF 1
#define HAVE_SNPRINTF 1

/* define if you have sys/fcntl.h */
#define HAVE_SYS_FCNTL_H 1

#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_WAIT_H 1
#define HAVE_SYS_TIMES_H 1
#define HAVE_SYS_TIME_H 1

#define HAVE_SYS_MMAN_H 1

#define HAVE_SYS_RESOURCE_H 1

/* Set these to the appropriate values */
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_LONG_LONG 8

/* #undef off_t */
#define SIZEOF_OFF_T 8
/* #undef size_t */

/* define if you have these. */
#define HAVE_ATEXIT 1
#define HAVE_ON_EXIT 1
#define HAVE_PUTENV 1
#define HAVE_GETCWD 1
#define HAVE_TCGETATTR 1
#define HAVE_TCSETATTR 1
#define HAVE_CFGETOSPEED 1
#define HAVE_LSTAT 1
#define HAVE_KILL 1
#define HAVE_KILLPG 1
#define HAVE_CHOWN 1
#define HAVE_VSNPRINTF 1
#define HAVE_POPEN 1
#define HAVE_UMASK 1
#define HAVE_READLINK 1
#define HAVE_SYMLINK 1
#define HAVE_LINK 1
#define HAVE_TIMES 1
#define HAVE_GMTIME 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_MKTIME 1
#define HAVE_MKFIFO 1
#define HAVE_MMAP 1

#define HAVE_GETPPID 1
#define HAVE_GETGID 1
#define HAVE_GETEGID 1
#define HAVE_GETEUID 1
#define HAVE_GETUID 1

#define HAVE_SETGID 1
#define HAVE_SETUID 1

#define HAVE_GETPGID 1
#define HAVE_GETPGRP 1
#define HAVE_SETPGID 1
#define HAVE_SETPGRP 1
#define HAVE_GETSID 1
#define HAVE_SETSID 1

#define HAVE_GETPRIORITY 1
#define HAVE_SETPRIORITY 1

/* #undef HAVE_ISSETUGID */

#define HAVE_ACOSH 1
#define HAVE_ASINH 1
#define HAVE_ATANH 1

/* #undef HAVE_ROUND */
/* #undef HAVE_FINITE */
/* #undef HAVE_ISINF */
/* #undef HAVE_ISNAN */
#define HAVE_HYPOT 1
#define HAVE_ATAN2 1
#define HAVE_FREXP 1
#define HAVE_FREXPF 1
#define HAVE_LDEXP 1
#define HAVE_LDEXPF 1

#define HAVE_DIRENT_H 1
/* #undef HAVE_SYS_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_NDIR_H */

#define HAVE_FSEEKO 1
#define HAVE_SETVBUF 1

#define HAVE_DLFCN_H 1

#define HAVE_SYS_UTSNAME_H 1
#define HAVE_UNAME 1

#define HAVE_ALARM 1
#define HAVE_PAUSE 1

/* These are used by the socket module */
#define HAVE_SOCKET 1
#define HAVE_SOCKETPAIR 1
#define HAVE_SYS_SOCKET_H 1
/* #undef HAVE_SOCKET_H */
#define HAVE_NETINET_IN_H 1
#define HAVE_ARPA_INET_H 1
#define HAVE_SYS_UN_H 1
/* #undef socklen_t */

#define HAVE_CONFSTR 1
#define HAVE_SYSCONF 1
#define HAVE_PATHCONF 1

/* These two are needed on DOS-like systems.  Unix does not require them.
 * They are included here for consistency.
 *
#define HAVE_IO_H
#define HAVE_PROCESS_H
 */

/* #undef HAVE_PCRE_H */

/* #undef USE_TERMCAP */

/* #undef mode_t */
/* #undef uid_t */
/* #undef pid_t */
/* #undef gid_t */

#define HAVE_SIGLONGJMP 1

/* Do we have posix signals? */
#define HAVE_SIGACTION 1
#define HAVE_SIGPROCMASK 1
#define HAVE_SIGEMPTYSET 1
#define HAVE_SIGADDSET 1

#define HAVE_ICONV 1
#define ICONV_CONST const

#define HAVE_FORK 1
#define HAVE_WAITPID 1

#if defined(HAVE_SIGADDSET) && defined(HAVE_SIGEMPTYSET)
# if defined(HAVE_SIGACTION) && defined(HAVE_SIGPROCMASK)
#  define SLANG_POSIX_SIGNALS
# endif
#endif

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

#ifdef _AIX
# ifndef _POSIX_SOURCE
#  define _POSIX_SOURCE 1
# endif
# ifndef _ALL_SOURCE
#  define _ALL_SOURCE
# endif
/* This may generate warnings but the fact is that without it, xlc will 
 * INCORRECTLY inline many str* functions. */
/* # undef __STR__ */
#endif

/* define USE_TERMCAP if you want to use it instead of terminfo. */
#if defined(sequent) || defined(NeXT)
# ifndef USE_TERMCAP
#  define USE_TERMCAP
# endif
#endif

#if defined(ultrix) && !defined(__GNUC__)
# ifndef NO_PROTOTYPES
#  define NO_PROTOTYPES
# endif
#endif

#ifndef unix
# define unix 1
#endif

#ifndef __unix__
# define __unix__ 1
#endif

#ifndef HAVE_LONG_LONG
# define SIZEOF_LONG_LONG 8
# define SIZEOF_LONG_LONG 8
#endif

#if defined(BUILD_DLL) && BUILD_DLL
# ifdef __CYGWIN32__
/* #  undef SLANG_DLL */
#  define SLANG_DLL 1
# endif
#endif

#endif /* SL_CONFIG_H */
