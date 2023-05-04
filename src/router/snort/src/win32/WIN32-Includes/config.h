/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/
/* $Id$ */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if you have the ANSI C header files.  */
/* #undef STDC_HEADERS */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#define WORDS_LITTLEENDIAN 1

#define WIN32 1

#define ERRLIST_PREDEFINED 1

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <stdint.h> header file.  */
#define HAVE_STDINT_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the pcap library (-lpcap).  */
#define HAVE_LIBPCAP 1

/* Define if you have vswprintf() function */
#define HAVE_VSWPRINTF 1

/* Define if you have wprintf() function */
#define HAVE_WPRINTF 1

/* Define to 1 if you have the <wchar.h> header file. */
#define HAVE_WCHAR_H 1

/* Define to 1 if you have zlib support. */
#define HAVE_ZLIB 1

#ifndef SIZEOF_LONG_INT
#define SIZEOF_LONG_INT 4
#endif

#ifdef ENABLE_RESPONSE
    #define VERSION_ENABLE_RESPONSE "-FlexRESP"
#else
    #define VERSION_ENABLE_RESPONSE
#endif
 
#ifdef DEBUG
    #define VERSION_DEBUG " [DEBUG] "
#else
    #define VERSION_DEBUG
#endif

#define LIBPCAP_ACCUMULATES

/*
 * This version number here, and package name below,
 * should both match the ones specified in the
 * AM_INIT_AUTOMAKE() macro of configure.in
 */
#define VERSION "2.9.20"VERSION_ENABLE_RESPONSE"-WIN32"VERSION_DEBUG
#define PACKAGE "snort"

#define IFNAMSIZ   255

/* _WIN32_WINNT has been modified to support SetDllDirectory API introduced in
 * Windows Server 2003 with SP1 and Windows XP with SP2 for Windows DLL Load Vulnerability.
 * As per documentation available on support forum, the macro corresponding to 0x0502 is _WIN32_WINNT_WS03.
 * However, Visual studio 6.0 on Win XP SP3 reports this as an undeclared identifier.
 * So, we decided to use numerical value instead of macro.
 */
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#undef NTDDI_VERSION
#define NTDDI_VERSION 0x05020000

/* Abuse header guards to prevent winscard.h from being included.
   This was needed to prevent conflict with sqlfront.h */
#define _WINSCARD_H_

#include <winsock2.h>
#include <windows.h>
#ifdef _MSC_VER
#pragma warning( disable : 4028 )
#endif
#include <io.h>
#include <process.h>
#ifdef _MSC_VER
#pragma warning( default : 4028 )
#endif
#include <sys/timeb.h>
#include <direct.h>
#include <getopt.h>
#ifndef __MINGW32__
typedef int            pid_t;
#endif
#ifndef __CYGWIN__
typedef long		ssize_t;
#endif
#include <ws2tcpip.h>
#include "rpc/types.h"

#undef interface

/* Declare standard integer datatypes */
#ifdef u_int8_t
#undef u_int8_t
#endif
#ifdef u_int16_t
#undef u_int16_t
#endif
#ifdef u_int32_t
#undef u_int32_t
#endif
#include <stdint.h>

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

#if defined(WIN32) && !defined(inline)
#define inline __inline
#endif

/* required for some calls to open(),
 * but it isn't defined under Win32.
 * Using 0x0080 to match <fcntl.h>
 * from OpenBSD.
 */
#ifndef O_SYNC
#define O_SYNC 0x0080
#endif


#ifndef CDECL 
#define CDECL __cdecl
#endif


// #define SIGKILL                  9       /* kill (cannot be caught or ignored) */
#define SIGQUIT                   3       /* quit */
#define SIGNAL_SNORT_RELOAD       1       /* Reload */
#define SIGNAL_SNORT_DUMP_STATS   30      /* Dump stats */
#define SIGNAL_SNORT_ROTATE_STATS 31      /* Rotate stats */
#define SIGPIPE 13               /* write on a pipe with no one to read it */
// #define EEXIST                   17              /* File exists */
#ifndef W_OK
#define W_OK                     0x02    /* test for write permission */
#endif
#ifndef R_OK
#define R_OK                     0x04    /* test for read permission */
#endif
#define S_ISDIR(x)               (((x) & 0170000) == 0040000)    /* directory */
#define S_IRWXU                  0000700                 /* RWX mask for owner */
#define S_IRWXG                  0000070                 /* RWX mask for group */
#define S_IROTH                  0000004                 /* R for other */
#define S_IXOTH                  0000001                 /* X for other */

#define bcopy(src, dst, count)   memcpy((void *)dst, (const void *)src, (size_t) count)
#define bzero(addr, count)       memset((addr), 0, (count))
#define index                    strchr
#define mkdir(x, y)              _mkdir(x)
#define snprintf                 _snprintf
#define strncasecmp              strnicmp
#define strcasecmp               stricmp
#define ftruncate                _chsize
#if _MSC_VER < 1500  /* VC9 defines this */
#define vsnprintf                _vsnprintf
#define strdup                   _strdup
#endif
#define IXDR_GET_LONG(buf)       ((long)ntohl((u_long)*(buf)++))
#define IXDR_GET_ENUM(buf, t)    ((t)IXDR_GET_LONG(buf))
#define RPC_MSG_VERSION          ((u_long) 2)

char * strtok_r(char *s1, const char *s2, char **lasts);
int    inet_aton(const char *cp, struct in_addr *addr);
int    inet_pton(int af, const char *src, void *dst);

enum msg_type {
    CALL=0,
    REPLY=1
};

typedef unsigned long mode_t;

struct timezone {
        int     tz_minuteswest; /* minutes west of Greenwich */
        int     tz_dsttime;     /* type of dst correction */
};

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct  ifreq {
        char    ifr_name[128];             /* if name, e.g. "en0" */
        union {
                struct  sockaddr ifru_addr;
                struct  sockaddr ifru_dstaddr;
                struct  sockaddr ifru_broadaddr;
                short   ifru_flags;
                int     ifru_metric;
                caddr_t ifru_data;
        } ifr_ifru;
#define ifr_addr        ifr_ifru.ifru_addr      /* address */
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr   /* other end of p-to-p link */
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr /* broadcast address */
#define ifr_flags       ifr_ifru.ifru_flags     /* flags */
#define ifr_metric      ifr_ifru.ifru_metric    /* metric */
#define ifr_mtu         ifr_ifru.ifru_metric    /* mtu (overload) */
#define ifr_media       ifr_ifru.ifru_metric    /* media options (overload) */
#define ifr_data        ifr_ifru.ifru_data      /* for use by interface */
};


#define RUSAGE_SELF       0     /* calling process */
#define RUSAGE_CHILDREN  -1     /* terminated child processes */
struct rusage
{
    struct  timeval ru_utime;/* user time used */
    struct  timeval ru_stime;/* system time used */
    long    ru_maxrss;
    long    ru_ixrss;     /* integral shared memory size */
    long    ru_idrss;     /* integral unshared data " */
    long    ru_isrss;     /* integral unshared stack " */
    long    ru_minflt;    /* page reclaims */
    long    ru_majflt;    /* page faults */
    long    ru_nswap;     /* swaps */
    long    ru_inblock;   /* block input operations */
    long    ru_oublock;   /* block output operations */
    long    ru_msgsnd;    /* messages sent */
    long    ru_msgrcv;    /* messages received */
    long    ru_nsignals;  /* signals received */
    long    ru_nvcsw;     /* voluntary context switches */
    long    ru_nivcsw;    /* involuntary " */
};

int   gettimeofday(struct timeval *,struct timezone *);
void *GetAdapterFromList(void *, int);
char *print_interface(const char *);
void  PrintDeviceList(const char *);
int   init_winsock(void);
int   ffs(int x);

inline int isblank(int c)
{
	if (c == ' ' || c == '\t' || c == '\v')
		return 1;

	return 0;
}

#if defined(ENABLE_WIN32_SERVICE)
#define SERVICE_CMDLINE_PARAM            "/SERVICE"
#define SERVICE_INSTALL_CMDLINE_PARAM    "/INSTALL"
#define SERVICE_UNINSTALL_CMDLINE_PARAM  "/UNINSTALL"
#define SERVICE_SHOW_CMDLINE_PARAM       "/SHOW"
int   SnortServiceMain(int argc, char* argv[]);
#endif  /* ENABLE_WIN32_SERVICE */

#ifndef S_IFIFO
#define S_IFIFO _S_IFIFO
#endif

#ifndef __FUNCTION__
#if _MSV_VER < 1300
#define __FUNCTION__ "dont know"
#else
#define __FUNCTION__ __func__
#endif
#endif

#endif /* __CONFIG_H__ */

