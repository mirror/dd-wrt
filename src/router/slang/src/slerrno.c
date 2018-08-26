/* The point of this file is to handle errno values in a system independent
 * way so that they may be used in slang scripts.
 */
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

#include "slinclud.h"

#include <errno.h>
#include "slang.h"
#include "_slang.h"

typedef SLCONST struct
{
   SLFUTURE_CONST char *msg;
   int sys_errno;
   SLFUTURE_CONST char *symbolic_name;
}
Errno_Map_Type;

static Errno_Map_Type Errno_Map [] =
{
#ifndef EPERM
# define EPERM	-1
#endif
     {"Operation not permitted",	EPERM,	"EPERM"},
#ifndef ENOENT
# define ENOENT	-1
#endif
     {"No such file or directory",	ENOENT,	"ENOENT"},
#ifndef ESRCH
# define ESRCH	-1
#endif
     {"No such process",		ESRCH,	"ESRCH"},
#ifndef EINTR
# define EINTR	-1
#endif
     {"Interrupted system call",	EINTR,	"EINTR"},
#ifndef EIO
# define EIO	-1
#endif
     {"I/O error",			EIO,	"EIO"},
#ifndef ENXIO
# define ENXIO	-1
#endif
     {"No such device or address",	ENXIO,	"ENXIO"},
#ifndef E2BIG
# define E2BIG	-1
#endif
     {"Arg list too long",		E2BIG,	"E2BIG"},
#ifndef ENOEXEC
# define ENOEXEC	-1
#endif
     {"Exec format error",		ENOEXEC,"ENOEXEC"},
#ifndef EBADF
# define EBADF	-1
#endif
     {"Bad file number",		EBADF,	"EBADF"},
#ifndef ECHILD
# define ECHILD	-1
#endif
     {"No children",			ECHILD,	"ECHILD"},
#ifndef EAGAIN
# define EAGAIN	-1
#endif
     {"Try again",			EAGAIN,	"EAGAIN"},
#ifndef ENOMEM
# define ENOMEM	-1
#endif
     {"Not enough core",		ENOMEM,	"ENOMEM"},
#ifndef EACCES
# define EACCES	-1
#endif
     {"Permission denied",		EACCES,	"EACCES"},
#ifndef EFAULT
# define EFAULT	-1
#endif
     {"Bad address",			EFAULT,	"EFAULT"},
#ifndef ENOTBLK
# define ENOTBLK	-1
#endif
     {"Block device required",		ENOTBLK,	"ENOTBLK"},
#ifndef EBUSY
# define EBUSY	-1
#endif
     {"Mount device busy",		EBUSY,	"EBUSY"},
#ifndef EEXIST
# define EEXIST	-1
#endif
     {"File exists",			EEXIST,	"EEXIST"},
#ifndef EXDEV
# define EXDEV	-1
#endif
     {"Cross-device link",		EXDEV,	"EXDEV"},
#ifndef ENODEV
# define ENODEV	-1
#endif
     {"No such device",			ENODEV,	"ENODEV"},
#ifndef ENOTDIR
# define ENOTDIR	-1
#endif
     {"Not a directory",		ENOTDIR,	"ENOTDIR"},
#ifndef EISDIR
# define EISDIR	-1
#endif
     {"Is a directory",			EISDIR,	"EISDIR"},
#ifndef EINVAL
# define EINVAL	-1
#endif
     {"Invalid argument",		EINVAL,	"EINVAL"},
#ifndef ENFILE
# define ENFILE	-1
#endif
     {"File table overflow",		ENFILE,	"ENFILE"},
#ifndef EMFILE
# define EMFILE	-1
#endif
     {"Too many open files",		EMFILE,	"EMFILE"},
#ifndef ENOTTY
# define ENOTTY	-1
#endif
     {"Not a typewriter",		ENOTTY,	"ENOTTY"},
#ifndef ETXTBSY
# define ETXTBSY	-1
#endif
     {"Text file busy",			ETXTBSY,	"ETXTBSY"},
#ifndef EFBIG
# define EFBIG	-1
#endif
     {"File too large",			EFBIG,	"EFBIG"},
#ifndef ENOSPC
# define ENOSPC	-1
#endif
     {"No space left on device",	ENOSPC,	"ENOSPC"},
#ifndef ESPIPE
# define ESPIPE	-1
#endif
     {"Illegal seek",			ESPIPE,	"ESPIPE"},
#ifndef EROFS
# define EROFS	-1
#endif
     {"Read-only file system",		EROFS,	"EROFS"},
#ifndef EMLINK
# define EMLINK	-1
#endif
     {"Too many links",			EMLINK,	"EMLINK"},
#ifndef EPIPE
# define EPIPE	-1
#endif
     {"Broken pipe",			EPIPE,	"EPIPE"},
#ifndef ELOOP
# define ELOOP	-1
#endif
     {"Too many levels of symbolic links",ELOOP,	"ELOOP"},
#ifndef ENAMETOOLONG
# define ENAMETOOLONG	-1
#endif
     {"File name too long",		ENAMETOOLONG,	"ENAMETOOLONG"},
#ifndef EDOM
# define EDOM	-1
#endif
     {"Math argument out of domain of func", EDOM,	"EDOM"},
#ifndef ERANGE
# define ERANGE	-1
#endif
     {"Math result not representable", ERANGE,	"ERANGE"},

#ifndef EDEADLK
# define EDEADLK	-1
#endif
     {"Resource deadlock would occur", EDEADLK,	"EDEADLK"},
#ifndef ENOLCK
# define ENOLCK	-1
#endif
     {"No record locks available", ENOLCK,	"ENOLCK"},
#ifndef ENOSYS
# define ENOSYS	-1
#endif
     {"Function not implemented", ENOSYS,	"ENOSYS"},
#ifndef ENOTEMPTY
# define ENOTEMPTY	-1
#endif
     {"Directory not empty", ENOTEMPTY,	"ENOTEMPTY"},
#ifndef ENOMSG
# define ENOMSG	-1
#endif
     {" No message of desired type ",	ENOMSG,	"ENOMSG"},
#ifndef EIDRM
# define EIDRM	-1
#endif
     {" Identifier removed ",	EIDRM,	"EIDRM"},
#ifndef ECHRNG
# define ECHRNG	-1
#endif
     {" Channel number out of range ",	ECHRNG,	"ECHRNG"},
#ifndef EL2NSYNC
# define EL2NSYNC	-1
#endif
     {" Level 2 not synchronized ",	EL2NSYNC,	"EL2NSYNC"},
#ifndef EL3HLT
# define EL3HLT	-1
#endif
     {" Level 3 halted ",	EL3HLT,	"EL3HLT"},
#ifndef EL3RST
# define EL3RST	-1
#endif
     {" Level 3 reset ",	EL3RST,	"EL3RST"},
#ifndef ELNRNG
# define ELNRNG	-1
#endif
     {" Link number out of range ",	ELNRNG,	"ELNRNG"},
#ifndef EUNATCH
# define EUNATCH	-1
#endif
     {" Protocol driver not attached ",	EUNATCH,	"EUNATCH"},
#ifndef ENOCSI
# define ENOCSI	-1
#endif
     {" No CSI structure available ",	ENOCSI,	"ENOCSI"},
#ifndef EL2HLT
# define EL2HLT	-1
#endif
     {" Level 2 halted ",	EL2HLT,	"EL2HLT"},
#ifndef EBADE
# define EBADE	-1
#endif
     {" Invalid exchange ",	EBADE,	"EBADE"},
#ifndef EBADR
# define EBADR	-1
#endif
     {" Invalid request descriptor ",	EBADR,	"EBADR"},
#ifndef EXFULL
# define EXFULL	-1
#endif
     {" Exchange full ",	EXFULL,	"EXFULL"},
#ifndef ENOANO
# define ENOANO	-1
#endif
     {" No anode ",	ENOANO,	"ENOANO"},
#ifndef EBADRQC
# define EBADRQC	-1
#endif
     {" Invalid request code ",	EBADRQC,	"EBADRQC"},
#ifndef EBADSLT
# define EBADSLT	-1
#endif
     {" Invalid slot ",	EBADSLT,	"EBADSLT"},
#ifndef EBFONT
# define EBFONT	-1
#endif
     {" Bad font file format ",	EBFONT,	"EBFONT"},
#ifndef ENOSTR
# define ENOSTR	-1
#endif
     {" Device not a stream ",	ENOSTR,	"ENOSTR"},
#ifndef ENODATA
# define ENODATA	-1
#endif
     {" No data available ",	ENODATA,	"ENODATA"},
#ifndef ETIME
# define ETIME	-1
#endif
     {" Timer expired ",	ETIME,	"ETIME"},
#ifndef ENOSR
# define ENOSR	-1
#endif
     {" Out of streams resources ",	ENOSR,	"ENOSR"},
#ifndef ENONET
# define ENONET	-1
#endif
     {" Machine is not on the network ",	ENONET,	"ENONET"},
#ifndef ENOPKG
# define ENOPKG	-1
#endif
     {" Package not installed ",	ENOPKG,	"ENOPKG"},
#ifndef EREMOTE
# define EREMOTE	-1
#endif
     {" Object is remote ",	EREMOTE,	"EREMOTE"},
#ifndef ENOLINK
# define ENOLINK	-1
#endif
     {" Link has been severed ",	ENOLINK,	"ENOLINK"},
#ifndef EADV
# define EADV	-1
#endif
     {" Advertise error ",	EADV,	"EADV"},
#ifndef ESRMNT
# define ESRMNT	-1
#endif
     {" Srmount error ",	ESRMNT,	"ESRMNT"},
#ifndef ECOMM
# define ECOMM	-1
#endif
     {" Communication error on send ",	ECOMM,	"ECOMM"},
#ifndef EPROTO
# define EPROTO	-1
#endif
     {" Protocol error ",	EPROTO,	"EPROTO"},
#ifndef EMULTIHOP
# define EMULTIHOP	-1
#endif
     {" Multihop attempted ",	EMULTIHOP,	"EMULTIHOP"},
#ifndef EDOTDOT
# define EDOTDOT	-1
#endif
     {" RFS specific error ",	EDOTDOT,	"EDOTDOT"},
#ifndef EBADMSG
# define EBADMSG	-1
#endif
     {" Not a data message ",	EBADMSG,	"EBADMSG"},
#ifndef EOVERFLOW
# define EOVERFLOW	-1
#endif
     {" Value too large for defined data type ",	EOVERFLOW,	"EOVERFLOW"},
#ifndef ENOTUNIQ
# define ENOTUNIQ	-1
#endif
     {" Name not unique on network ",	ENOTUNIQ,	"ENOTUNIQ"},
#ifndef EBADFD
# define EBADFD	-1
#endif
     {" File descriptor in bad state ",	EBADFD,	"EBADFD"},
#ifndef EREMCHG
# define EREMCHG	-1
#endif
     {" Remote address changed ",	EREMCHG,	"EREMCHG"},
#ifndef ELIBACC
# define ELIBACC	-1
#endif
     {" Can not access a needed shared library ",	ELIBACC,	"ELIBACC"},
#ifndef ELIBBAD
# define ELIBBAD	-1
#endif
     {" Accessing a corrupted shared library ",	ELIBBAD,	"ELIBBAD"},
#ifndef ELIBSCN
# define ELIBSCN	-1
#endif
     {" .lib section in a.out corrupted ",	ELIBSCN,	"ELIBSCN"},
#ifndef ELIBMAX
# define ELIBMAX	-1
#endif
     {" Attempting to link in too many shared libraries ",	ELIBMAX,	"ELIBMAX"},
#ifndef ELIBEXEC
# define ELIBEXEC	-1
#endif
     {" Cannot exec a shared library directly ",	ELIBEXEC,	"ELIBEXEC"},
#ifndef EILSEQ
# define EILSEQ	-1
#endif
     {" Illegal byte sequence ",	EILSEQ,	"EILSEQ"},
#ifndef ERESTART
# define ERESTART	-1
#endif
     {" Interrupted system call should be restarted ",	ERESTART,	"ERESTART"},
#ifndef ESTRPIPE
# define ESTRPIPE	-1
#endif
     {" Streams pipe error ",	ESTRPIPE,	"ESTRPIPE"},
#ifndef EUSERS
# define EUSERS	-1
#endif
     {" Too many users ",	EUSERS,	"EUSERS"},
#ifndef ENOTSOCK
# define ENOTSOCK	-1
#endif
     {" Socket operation on non-socket ",	ENOTSOCK,	"ENOTSOCK"},
#ifndef EDESTADDRREQ
# define EDESTADDRREQ	-1
#endif
     {" Destination address required ",	EDESTADDRREQ,	"EDESTADDRREQ"},
#ifndef EMSGSIZE
# define EMSGSIZE	-1
#endif
     {" Message too long ",	EMSGSIZE,	"EMSGSIZE"},
#ifndef EPROTOTYPE
# define EPROTOTYPE	-1
#endif
     {" Protocol wrong type for socket ",	EPROTOTYPE,	"EPROTOTYPE"},
#ifndef ENOPROTOOPT
# define ENOPROTOOPT	-1
#endif
     {" Protocol not available ",	ENOPROTOOPT,	"ENOPROTOOPT"},
#ifndef EPROTONOSUPPORT
# define EPROTONOSUPPORT	-1
#endif
     {" Protocol not supported ",	EPROTONOSUPPORT,	"EPROTONOSUPPORT"},
#ifndef ESOCKTNOSUPPORT
# define ESOCKTNOSUPPORT	-1
#endif
     {" Socket type not supported ",	ESOCKTNOSUPPORT,	"ESOCKTNOSUPPORT"},
#ifndef EOPNOTSUPP
# define EOPNOTSUPP	-1
#endif
     {" Operation not supported on transport endpoint ",	EOPNOTSUPP,	"EOPNOTSUPP"},
#ifndef EPFNOSUPPORT
# define EPFNOSUPPORT	-1
#endif
     {" Protocol family not supported ",	EPFNOSUPPORT,	"EPFNOSUPPORT"},
#ifndef EAFNOSUPPORT
# define EAFNOSUPPORT	-1
#endif
     {" Address family not supported by protocol ",	EAFNOSUPPORT,	"EAFNOSUPPORT"},
#ifndef EADDRINUSE
# define EADDRINUSE	-1
#endif
     {" Address already in use ",	EADDRINUSE,	"EADDRINUSE"},
#ifndef EADDRNOTAVAIL
# define EADDRNOTAVAIL	-1
#endif
     {" Cannot assign requested address ",	EADDRNOTAVAIL,	"EADDRNOTAVAIL"},
#ifndef ENETDOWN
# define ENETDOWN	-1
#endif
     {" Network is down ",	ENETDOWN,	"ENETDOWN"},
#ifndef ENETUNREACH
# define ENETUNREACH	-1
#endif
     {" Network is unreachable ",	ENETUNREACH,	"ENETUNREACH"},
#ifndef ENETRESET
# define ENETRESET	-1
#endif
     {" Network dropped connection because of reset ",	ENETRESET,	"ENETRESET"},
#ifndef ECONNABORTED
# define ECONNABORTED	-1
#endif
     {" Software caused connection abort ",	ECONNABORTED,	"ECONNABORTED"},
#ifndef ECONNRESET
# define ECONNRESET	-1
#endif
     {" Connection reset by peer ",	ECONNRESET,	"ECONNRESET"},
#ifndef ENOBUFS
# define ENOBUFS	-1
#endif
     {" No buffer space available ",	ENOBUFS,	"ENOBUFS"},
#ifndef EISCONN
# define EISCONN	-1
#endif
     {" Transport endpoint is already connected ",	EISCONN,	"EISCONN"},
#ifndef ENOTCONN
# define ENOTCONN	-1
#endif
     {" Transport endpoint is not connected ",	ENOTCONN,	"ENOTCONN"},
#ifndef ESHUTDOWN
# define ESHUTDOWN	-1
#endif
     {" Cannot send after transport endpoint shutdown ",	ESHUTDOWN,	"ESHUTDOWN"},
#ifndef ETOOMANYREFS
# define ETOOMANYREFS	-1
#endif
     {" Too many references: cannot splice ",	ETOOMANYREFS,	"ETOOMANYREFS"},
#ifndef ETIMEDOUT
# define ETIMEDOUT	-1
#endif
     {" Connection timed out ",	ETIMEDOUT,	"ETIMEDOUT"},
#ifndef ECONNREFUSED
# define ECONNREFUSED	-1
#endif
     {" Connection refused ",	ECONNREFUSED,	"ECONNREFUSED"},
#ifndef EHOSTDOWN
# define EHOSTDOWN	-1
#endif
     {" Host is down ",	EHOSTDOWN,	"EHOSTDOWN"},
#ifndef EHOSTUNREACH
# define EHOSTUNREACH	-1
#endif
     {" No route to host ",	EHOSTUNREACH,	"EHOSTUNREACH"},
#ifndef EALREADY
# define EALREADY	-1
#endif
     {" Operation already in progress ",	EALREADY,	"EALREADY"},
#ifndef EINPROGRESS
# define EINPROGRESS	-1
#endif
     {" Operation now in progress ",	EINPROGRESS,	"EINPROGRESS"},
#ifndef ESTALE
# define ESTALE	-1
#endif
     {" Stale NFS file handle ",	ESTALE,	"ESTALE"},
#ifndef EUCLEAN
# define EUCLEAN	-1
#endif
     {" Structure needs cleaning ",	EUCLEAN,	"EUCLEAN"},
#ifndef ENOTNAM
# define ENOTNAM	-1
#endif
     {" Not a XENIX named type file ",	ENOTNAM,	"ENOTNAM"},
#ifndef ENAVAIL
# define ENAVAIL	-1
#endif
     {" No XENIX semaphores available ",	ENAVAIL,	"ENAVAIL"},
#ifndef EISNAM
# define EISNAM	-1
#endif
     {" Is a named type file ",	EISNAM,	"EISNAM"},
#ifndef EREMOTEIO
# define EREMOTEIO	-1
#endif
     {" Remote I/O error ",	EREMOTEIO,	"EREMOTEIO"},
#ifndef EDQUOT
# define EDQUOT	-1
#endif
     {" Quota exceeded ",	EDQUOT,	"EDQUOT"},
#ifndef ENOMEDIUM
# define ENOMEDIUM	-1
#endif
     {" No medium found ",	ENOMEDIUM,	"ENOMEDIUM"},
#ifndef EMEDIUMTYPE
# define EMEDIUMTYPE	-1
#endif
     {" Wrong medium type ",	EMEDIUMTYPE,	"EMEDIUMTYPE"},
     {NULL, 0, NULL}
};

SLFUTURE_CONST char *SLerrno_strerror (int sys_errno)
{
   Errno_Map_Type *e;

   e = Errno_Map;
   while (e->msg != NULL)
     {
	if (e->sys_errno == sys_errno)
	  return e->msg;

	e++;
     }

   if (sys_errno == SL_ERRNO_NOT_IMPLEMENTED)
     return "System call not available for this platform";

   return "Unknown error";
}

static SLCONST char *intrin_errno_string (void)
{
   int e;
   if (SLang_Num_Function_Args == 0)
     return SLerrno_strerror (_pSLerrno_errno);
   if (-1 == SLang_pop_int (&e))
     return NULL;
   return SLerrno_strerror (e);
}

int _pSLerrno_init (void)
{
   static Errno_Map_Type *e;

   if (e != NULL)		       /* already initialized */
     return 0;

   if ((-1 == SLadd_intrinsic_function ("errno_string", (FVOID_STAR) intrin_errno_string,
					SLANG_STRING_TYPE, 0))
       || (-1 == SLadd_intrinsic_variable ("errno", (VOID_STAR)&_pSLerrno_errno, SLANG_INT_TYPE, 1)))
     return -1;

   e = Errno_Map;
   while (e->msg != NULL)
     {
	if (-1 == SLadd_intrinsic_variable (e->symbolic_name, (VOID_STAR) &e->sys_errno, SLANG_INT_TYPE, 1))
	  return -1;
	e++;
     }

   return 0;
}
