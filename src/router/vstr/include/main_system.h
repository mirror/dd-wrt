#ifndef MAIN_SYSTEM_H
#define MAIN_SYSTEM_H

#undef _GNU_SOURCE
#define _GNU_SOURCE 1

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#if STDC_HEADERS
# include <string.h>
#else
/* this probably needs more stuff in it -- stdarg.h ? */
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include <limits.h>
#include <ctype.h>


#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef USE_RESTRICTED_HEADERS /* dietlibc/klibc */

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

# include <math.h>
# include <wchar.h>
# include <locale.h>

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#else
# ifdef HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif

#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
#endif
#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#else
# ifdef USE_DL_LOAD
#  undef USE_DL_LOAD
# endif
#endif

#include <netdb.h>
#include <setjmp.h>
#include <sys/file.h>

#include <netinet/tcp.h> /* not in klibc ... probably should be ? */
#include <arpa/telnet.h>

#endif /* USE_RESTRICTED_HEADERS */

#if 0 /* FIXME: def HAVE_MALLOC_H -- BSD produces a crappy warning... */
# include <malloc.h>
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif

#ifdef HAVE_POLL
# include <poll.h>
#endif

#ifdef HAVE_SYS_EPOLL
# include <sys/epoll.h>
#endif

#ifdef HAVE_SENDFILE
# include <sys/sendfile.h>
#endif

#ifdef HAVE_VFORK_H
# include <vfork.h>
#endif

#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#ifdef USE_MMAP
# ifdef HAVE_MMAP
#  ifdef HAVE_SYS_MMAN_H
#   include <sys/mman.h>
#  else
#   warning "Using mmap, but have no <sys/mman.h>"
#  endif
# else
#  warning "Not using, even though you asked."
#  undef USE_MMAP
# endif
#endif

#ifndef TELOPT_COMPRESS
# define TELOPT_COMPRESS 85
#endif

#ifdef HAVE_ZLIB_H
# include <zlib.h>
#endif

#ifdef HAVE_TERMCAP_H
# include <termcap.h>
#endif

#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

/* useful -- cast so it warns for ptrs */
#ifndef FALSE
# define FALSE ((int)0)
#endif

#ifndef TRUE
# define TRUE  ((int)1)
#endif

#endif
