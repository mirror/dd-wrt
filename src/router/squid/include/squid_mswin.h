/*
 * $Id: squid_mswin.h,v 1.4 2006/12/10 13:36:23 serassio Exp $
 *
 * AUTHOR: Andrey Shorin <tolsty@tushino.com>
 * AUTHOR: Guido Serassio <serassio@squid-cache.org>
 *
 * SQUID Internet Object Cache  http://squid.nlanr.net/Squid/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from the
 *  Internet community.  Development is led by Duane Wessels of the
 *  National Laboratory for Applied Network Research and funded by the
 *  National Science Foundation.  Squid is Copyrighted (C) 1998 by
 *  the Regents of the University of California.  Please see the
 *  COPYRIGHT file for full details.  Squid incorporates software
 *  developed and/or copyrighted by other sources.  Please see the
 *  CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#ifndef STDC_HEADERS
#define STDC_HEADERS 1
#endif

#define _WIN32_WINNT 0x0500

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
# define __USE_FILE_OFFSET64	1
#endif

#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */

typedef	unsigned char	u_char;

typedef int SOCKET;
typedef int ssize_t;
typedef int mode_t;

#if defined __USE_FILE_OFFSET64
typedef int64_t off_t;
typedef uint64_t ino_t;

#else
typedef long off_t;
typedef unsigned long ino_t;

#endif

#define INT64_MAX _I64_MAX
#define INT64_MIN _I64_MIN

#include "default_config_file.h"
/* Some tricks for MS Compilers */
#define __STDC__ 1
#pragma include_alias(<dirent.h>, <direct.h>)
#define THREADLOCAL __declspec(thread)

#elif defined(__GNUC__) /* gcc environment */

#define THREADLOCAL __attribute__((section(".tls")))

#endif

#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define alloca _alloca
#endif
#define chdir _chdir
#define dup _dup
#define dup2 _dup2
#define fdopen _fdopen
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define fileno _fileno
#endif
#define ftruncate WIN32_ftruncate
#define getcwd _getcwd
#define getpid _getpid
#define getrusage WIN32_getrusage
#define ioctl ioctlsocket
#define memccpy _memccpy
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define mkdir(p) _mkdir(p)
#endif
#define mktemp _mktemp
#define open _open
#define pclose _pclose
#define pipe WIN32_pipe
#define popen _popen
#define putenv _putenv
#define setmode _setmode
#define sleep(t) Sleep((t)*1000)
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define snprintf _snprintf
#endif
#define strcasecmp _stricmp
#define strdup _strdup
#define strlwr _strlwr
#define strncasecmp _strnicmp
#define tempnam _tempnam
#define truncate WIN32_truncate
#define umask _umask
#define unlink _unlink
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define vsnprintf _vsnprintf
#endif

#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND

#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL

#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_RAW           _O_BINARY
#define O_TEMPORARY     _O_TEMPORARY
#define O_NOINHERIT     _O_NOINHERIT
#define O_SEQUENTIAL    _O_SEQUENTIAL
#define O_RANDOM        _O_RANDOM
#define O_NDELAY	0

#define S_IRWXO 007
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define	S_ISDIR(m) (((m) & _S_IFDIR) == _S_IFDIR)
#define	S_ISREG(m) (((m) & _S_IFREG) == _S_IFREG)
#endif

#ifndef SIGHUP
#define	SIGHUP	1	/* hangup */
#endif
#ifndef SIGBUS
#define	SIGBUS  7	/* bus error */
#endif
#ifndef SIGKILL
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#endif
#ifndef	SIGSEGV
#define	SIGSEGV 11      /* segment violation */
#endif
#ifndef SIGPIPE
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#endif
#ifndef SIGCHLD
#define	SIGCHLD	20	/* to parent on child stop or exit */
#endif
#ifndef SIGUSR1
#define SIGUSR1 30	/* user defined signal 1 */
#endif
#ifndef SIGUSR2
#define SIGUSR2 31	/* user defined signal 2 */
#endif

typedef unsigned short in_port_t;
typedef unsigned short int ushort;
typedef int uid_t;
typedef int gid_t;

#if defined __USE_FILE_OFFSET64
#define stat _stati64
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define lseek _lseeki64
#endif
#define fstat _fstati64
#define tell _telli64

#else
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define stat _stat
#define lseek _lseek
#define fstat _fstat
#define tell _tell
#endif

#endif

struct passwd {
    char    *pw_name;      /* user name */
    char    *pw_passwd;    /* user password */
    uid_t   pw_uid;        /* user id */
    gid_t   pw_gid;        /* group id */
    char    *pw_gecos;     /* real name */
    char    *pw_dir;       /* home directory */
    char    *pw_shell;     /* shell program */
};

struct group {
    char    *gr_name;      /* group name */
    char    *gr_passwd;    /* group password */
    gid_t   gr_gid;        /* group id */
    char    **gr_mem;      /* group members */
};

struct statfs {
   long    f_type;     /* type of filesystem (see below) */
   long    f_bsize;    /* optimal transfer block size */
   long    f_blocks;   /* total data blocks in file system */
   long    f_bfree;    /* free blocks in fs */
   long    f_bavail;   /* free blocks avail to non-superuser */
   long    f_files;    /* total file nodes in file system */
   long    f_ffree;    /* free file nodes in fs */
   long    f_fsid;     /* file system id */
   long    f_namelen;  /* maximum length of filenames */
   long    f_spare[6]; /* spare for later */
};

#ifndef HAVE_GETTIMEOFDAY
struct timezone 
  {
    int	tz_minuteswest;	/* minutes west of Greenwich */
    int	tz_dsttime;	/* type of dst correction */
  };
#endif

#define CHANGE_FD_SETSIZE 1
#if CHANGE_FD_SETSIZE && SQUID_MAXFD > DEFAULT_FD_SETSIZE
#define FD_SETSIZE SQUID_MAXFD
#endif

#include <stddef.h>
#include <process.h>
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
/* Hack to suppress compiler warnings on FD_SET() & FD_CLR() */
#pragma warning (push)
#pragma warning (disable:4142)
#endif
/* prevent inclusion of wingdi.h */
#define NOGDI
#include <ws2spi.h>
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#pragma warning (pop)
#include "readdir.h"
#else
#include <io.h>
#include <stdlib.h>
#include <sys/types.h> 
#endif

typedef char * caddr_t;

#undef FD_CLOSE
#undef FD_OPEN
#undef FD_READ
#undef FD_WRITE
#define EISCONN WSAEISCONN
#define EINPROGRESS WSAEINPROGRESS 
#define EWOULDBLOCK WSAEWOULDBLOCK
#define EALREADY WSAEALREADY
#define ETIMEDOUT WSAETIMEDOUT
#define ECONNREFUSED WSAECONNREFUSED
#define ECONNRESET WSAECONNRESET
#define ERESTART WSATRY_AGAIN
#define ENOTCONN WSAENOTCONN

#undef h_errno
#define h_errno errno /* we'll set it ourselves */

#undef FD_CLR
#define FD_CLR(fd, set) do { \
    u_int __i; \
    SOCKET __sock = _get_osfhandle(fd); \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == __sock) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#undef FD_SET
#define FD_SET(fd, set) do { \
    u_int __i; \
    SOCKET __sock = _get_osfhandle(fd); \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == (__sock)) { \
            break; \
        } \
    } \
    if (__i == ((fd_set FAR *)(set))->fd_count) { \
        if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) { \
            ((fd_set FAR *)(set))->fd_array[__i] = (__sock); \
            ((fd_set FAR *)(set))->fd_count++; \
        } \
    } \
} while(0)

#undef FD_ISSET
#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(_get_osfhandle(fd)), (fd_set FAR *)(set))

extern THREADLOCAL int ws32_result;

#define strerror(e) WIN32_strerror(e)

#define socket(f,t,p) \
	(INVALID_SOCKET == ((SOCKET)(ws32_result = (int)socket(f,t,p))) ? \
	((WSAEMFILE == (errno = WSAGetLastError()) ? errno = EMFILE : -1), -1) : \
	(SOCKET)_open_osfhandle(ws32_result,0))
#define accept(s,a,l) \
	(INVALID_SOCKET == ((SOCKET)(ws32_result = (int)accept(_get_osfhandle(s),a,l))) ? \
	((WSAEMFILE == (errno = WSAGetLastError()) ? errno = EMFILE : -1), -1) : \
	(SOCKET)_open_osfhandle(ws32_result,0))
#define bind(s,n,l) \
	(SOCKET_ERROR == bind(_get_osfhandle(s),n,l) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define connect(s,n,l) \
	(SOCKET_ERROR == connect(_get_osfhandle(s),n,l) ? \
	(WSAEMFILE == (errno = WSAGetLastError()) ? errno = EMFILE : -1, -1) : 0)
#define listen(s,b) \
	(SOCKET_ERROR == listen(_get_osfhandle(s),b) ? \
	(WSAEMFILE == (errno = WSAGetLastError()) ? errno = EMFILE : -1, -1) : 0)
#define shutdown(s,h) \
	(SOCKET_ERROR == shutdown(_get_osfhandle(s),h) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define select(n,r,w,e,t) \
	(SOCKET_ERROR == (ws32_result = select(n,r,w,e,t)) ? \
	(errno = WSAGetLastError()), -1 : ws32_result)
#define recv(s,b,l,f) \
	(SOCKET_ERROR == (ws32_result = recv(_get_osfhandle(s),b,l,f)) ? \
        (errno = WSAGetLastError()), -1 : ws32_result)
#define recvfrom(s,b,l,f,fr,frl) \
	(SOCKET_ERROR == (ws32_result = recvfrom(_get_osfhandle(s),b,l,f,fr,frl)) ? \
	(errno = WSAGetLastError()), -1 : ws32_result)
#define send(s,b,l,f) \
	(SOCKET_ERROR == (ws32_result = send(_get_osfhandle(s),b,l,f)) ? \
	(errno = WSAGetLastError()), -1 : ws32_result)
#define sendto(s,b,l,f,t,tl) \
	(SOCKET_ERROR == (ws32_result = sendto(_get_osfhandle(s),b,l,f,t,tl)) ? \
	(errno = WSAGetLastError()), -1 : ws32_result)
#define getsockname(s,n,l) \
	(SOCKET_ERROR == getsockname(_get_osfhandle(s),n,l) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define getsockopt(s,l,o,v,n) \
	(Sleep(1), SOCKET_ERROR == getsockopt(_get_osfhandle(s),l,o,(char*)v,n) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define setsockopt(s,l,o,v,n) \
	(SOCKET_ERROR == setsockopt(_get_osfhandle(s),l,o,v,n) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define ioctlsocket(s,c,a) \
	(SOCKET_ERROR == ioctlsocket(_get_osfhandle(s),c,a) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define gethostname(n,l) \
	(SOCKET_ERROR == gethostname(n,l) ? \
	(errno = WSAGetLastError()), -1 : 0)
#define gethostbyname(n) \
	(NULL == ((HOSTENT FAR*)(ws32_result = (int)gethostbyname(n))) ? \
	(errno = WSAGetLastError()), NULL : (HOSTENT FAR*)ws32_result)
#define getservbyname(n,p) \
	(NULL == ((SERVENT FAR*)(ws32_result = (int)getservbyname(n,p))) ? \
	(errno = WSAGetLastError()), NULL : (SERVENT FAR*)ws32_result)
#define gethostbyaddr(a,l,t) \
	(NULL == ((HOSTENT FAR*)(ws32_result = (int)gethostbyaddr(a,l,t))) ? \
	(errno = WSAGetLastError()), NULL : (HOSTENT FAR*)ws32_result)
#undef WSASocket
#ifdef UNICODE
#define WSASocket(a,t,p,i,g,f) \
	(INVALID_SOCKET == ((SOCKET)(ws32_result = (int)WSASocketW(a,t,p,i,g,f))) ? \
	((WSAEMFILE == (errno = WSAGetLastError()) ? errno = EMFILE : -1), -1) : \
	(SOCKET)_open_osfhandle(ws32_result,0))
#else
#define WSASocket(a,t,p,i,g,f) \
	(INVALID_SOCKET == ((SOCKET)(ws32_result = (int)WSASocketA(a,t,p,i,g,f))) ? \
	((WSAEMFILE == (errno = WSAGetLastError()) ? errno = EMFILE : -1), -1) : \
	(SOCKET)_open_osfhandle(ws32_result,0))
#endif /* !UNICODE */
#undef WSADuplicateSocket
#ifdef UNICODE
#define WSADuplicateSocket(s,n,l) \
	(SOCKET_ERROR == WSADuplicateSocketW(_get_osfhandle(s),n,l) ? \
	(errno = WSAGetLastError()), -1 : 0)
#else
#define WSADuplicateSocket(s,n,l) \
	(SOCKET_ERROR == WSADuplicateSocketA(_get_osfhandle(s),n,l) ? \
	(errno = WSAGetLastError()), -1 : 0)
#endif /* !UNICODE */

#if defined(UTIL_C)
#define read       _read
#define write      _write
#else
extern THREADLOCAL int _so_err;
extern THREADLOCAL int _so_err_siz;
#define read(fd,buf,siz) \
	(_so_err_siz = sizeof(_so_err), \
	getsockopt((fd),SOL_SOCKET,SO_ERROR,&_so_err,&_so_err_siz) \
	== 0 ? recv((fd),(buf),(siz),0) : _read((fd),(buf),(siz)))
#define write(fd,buf,siz) \
	(_so_err_siz = sizeof(_so_err), \
	getsockopt((fd),SOL_SOCKET,SO_ERROR,&_so_err,&_so_err_siz) \
	== 0 ? send((fd),(buf),(siz),0) : _write((fd),(buf),(siz)))
#endif

#if defined(COMM_C) || defined(TOOLS_C)
#define close WIN32_Close_FD_Socket
#else
#define close _close
#endif

#define	RUSAGE_SELF	0		/* calling process */
#define	RUSAGE_CHILDREN	-1		/* terminated child processes */

struct rusage {
	struct timeval ru_utime;	/* user time used */
	struct timeval ru_stime;	/* system time used */
	long ru_maxrss;			/* integral max resident set size */
	long ru_ixrss;			/* integral shared text memory size */
	long ru_idrss;			/* integral unshared data size */
	long ru_isrss;			/* integral unshared stack size */
	long ru_minflt;			/* page reclaims */
	long ru_majflt;			/* page faults */
	long ru_nswap;			/* swaps */
	long ru_inblock;		/* block input operations */
	long ru_oublock;		/* block output operations */
	long ru_msgsnd;			/* messages sent */
	long ru_msgrcv;			/* messages received */
	long ru_nsignals;		/* signals received */
	long ru_nvcsw;			/* voluntary context switches */
	long ru_nivcsw;			/* involuntary context switches */
};
