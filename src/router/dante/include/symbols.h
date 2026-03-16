/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2008, 2009, 2010, 2011,
 *               2012, 2013
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

/* $Id: symbols.h,v 1.17 2013/10/27 15:24:41 karls Exp $ */

#ifndef LIBRARY_PATH
#define LIBRARY_PATH ""
#endif /* !LIBRARY_PATH */

#ifndef LIBRARY_LIBC
#define LIBRARY_LIBC                        __CONCAT(LIBRARY_PATH, "libc.so")
#endif /* !LIBRARY_LIBC */

#if HAVE_NO_SYMBOL_UNDERSCORE
#define SYMBOLPREFIX ""
#else
#define SYMBOLPREFIX "_"
#endif /* HAVE_NO_SYMBOL_UNDERSCORE */

#define SYMBOL_ACCEPT          SYMBOLPREFIX "accept"
#define SYMBOL_BIND            SYMBOLPREFIX "bind"
#define SYMBOL_BINDRESVPORT    SYMBOLPREFIX "bindresvport"
#define SYMBOL_CONNECT         SYMBOLPREFIX "connect"
#define SYMBOL_FREEHOSTENT     SYMBOLPREFIX "freehostent"
#define SYMBOL_GETADDRINFO     SYMBOLPREFIX "getaddrinfo"
#define SYMBOL_GETNAMEINFO     SYMBOLPREFIX "getnameinfo"
#define SYMBOL_GETHOSTBYADDR   SYMBOLPREFIX "gethostbyaddr"
#define SYMBOL_GETHOSTBYNAME   SYMBOLPREFIX "gethostbyname"
#define SYMBOL_GETHOSTBYNAME2  SYMBOLPREFIX "gethostbyname2"
#define SYMBOL_GETIPNODEBYNAME SYMBOLPREFIX "getipnodebyname"
#define SYMBOL_GETPEERNAME     SYMBOLPREFIX "getpeername"
#define SYMBOL_GETSOCKNAME     SYMBOLPREFIX "getsockname"
#define SYMBOL_GETSOCKOPT      SYMBOLPREFIX "getsockopt"
#define SYMBOL_LISTEN          SYMBOLPREFIX "listen"
#define SYMBOL_READ            SYMBOLPREFIX "read"
#define SYMBOL_READV           SYMBOLPREFIX "readv"
#define SYMBOL_RECV            SYMBOLPREFIX "recv"
#define SYMBOL_RECVFROM        SYMBOLPREFIX "recvfrom"
#define SYMBOL_RECVMSG         SYMBOLPREFIX "recvmsg"
#define SYMBOL_RRESVPORT       SYMBOLPREFIX "rresvport"
#define SYMBOL_SEND            SYMBOLPREFIX "send"
#define SYMBOL_SENDMSG         SYMBOLPREFIX "sendmsg"
#define SYMBOL_SENDTO          SYMBOLPREFIX "sendto"
#define SYMBOL_WRITE           SYMBOLPREFIX "write"
#define SYMBOL_WRITEV          SYMBOLPREFIX "writev"

#ifndef LIBRARY_ACCEPT
#define LIBRARY_ACCEPT                      LIBRARY_LIBC
#endif /* !LIBRARY_ACCEPT */

#ifndef LIBRARY_BIND
#define LIBRARY_BIND                        LIBRARY_LIBC
#endif /* !LIBRARY_BIND */

#ifndef LIBRARY_BINDRESVPORT
#define LIBRARY_BINDRESVPORT                LIBRARY_LIBC
#endif /* !LIBRARY_BINDRESVPORT */

#ifndef LIBRARY_CONNECT
#define LIBRARY_CONNECT                     LIBRARY_LIBC
#endif /* !LIBRARY_CONNECT */

#ifndef LIBRARY_GETHOSTBYADDR
#define LIBRARY_GETHOSTBYADDR               LIBRARY_LIBC
#endif /* !LIBRARY_GETHOSTBYADDR */

#ifndef LIBRARY_GETHOSTBYNAME
#define LIBRARY_GETHOSTBYNAME               LIBRARY_LIBC
#endif /* !LIBRARY_GETHOSTBYNAME */

#ifndef LIBRARY_GETHOSTBYNAME2
#define LIBRARY_GETHOSTBYNAME2              LIBRARY_LIBC
#endif /* !LIBRARY_GETHOSTBYNAME2 */

#ifndef LIBRARY_GETADDRINFO
#define LIBRARY_GETADDRINFO                 LIBRARY_LIBC
#endif /* !LIBRARY_GETADDRINFO */

#ifndef LIBRARY_GETNAMEINFO
#define LIBRARY_GETNAMEINFO                 LIBRARY_LIBC
#endif /* !LIBRARY_GETNAMEINFO */

#ifndef LIBRARY_GETIPNODEBYNAME
#define LIBRARY_GETIPNODEBYNAME             LIBRARY_LIBC
#endif /* !LIBRARY_GETIPNODEBYNAME */

#ifndef LIBRARY_FREEHOSTENT
#define LIBRARY_FREEHOSTENT                 LIBRARY_LIBC
#endif /* !LIBRARY_FREEHOSTENT */

#ifndef LIBRARY_GETPEERNAME
#define LIBRARY_GETPEERNAME                 LIBRARY_LIBC
#endif /* !LIBRARY_GETPEERNAME */

#ifndef LIBRARY_GETSOCKNAME
#define LIBRARY_GETSOCKNAME                 LIBRARY_LIBC
#endif /* !LIBRARY_GETSOCKNAME */

#ifndef LIBRARY_GETSOCKOPT
#define LIBRARY_GETSOCKOPT                  LIBRARY_LIBC
#endif /* !LIBRARY_GETSOCKOPT */

#ifndef LIBRARY_LISTEN
#define LIBRARY_LISTEN                      LIBRARY_LIBC
#endif /* !LIBRARY_LISTEN */

#ifndef LIBRARY_READ
#define LIBRARY_READ                        LIBRARY_LIBC
#endif /* !LIBRARY_READ */

#ifndef LIBRARY_READV
#define LIBRARY_READV                       LIBRARY_LIBC
#endif /* !LIBRARY_READV */

#ifndef LIBRARY_RECV
#define LIBRARY_RECV                        LIBRARY_LIBC
#endif /* !LIBRARY_RECV */

#ifndef LIBRARY_RECVFROM
#define LIBRARY_RECVFROM                    LIBRARY_LIBC
#endif /* !LIBRARY_RECVFROM */

#ifndef LIBRARY_RECVMSG
#define LIBRARY_RECVMSG                     LIBRARY_LIBC
#endif /* !LIBRARY_RECVMSG */

#ifndef LIBRARY_RRESVPORT
#define LIBRARY_RRESVPORT                   LIBRARY_LIBC
#endif /* !LIBRARY_RRESVPORT */

#ifndef LIBRARY_SEND
#define LIBRARY_SEND                        LIBRARY_LIBC
#endif /* !LIBRARY_SEND */

#ifndef LIBRARY_SENDMSG
#define LIBRARY_SENDMSG                     LIBRARY_LIBC
#endif /* !LIBRARY_SENDMSG */

#ifndef LIBRARY_SENDTO
#define LIBRARY_SENDTO                      LIBRARY_LIBC
#endif /* !LIBRARY_SENDTO */

#ifndef LIBRARY_WRITE
#define LIBRARY_WRITE                       LIBRARY_LIBC
#endif /* !LIBRARY_WRITE */

#ifndef LIBRARY_WRITEV
#define LIBRARY_WRITEV                      LIBRARY_LIBC
#endif /* !LIBRARY_WRITEV */


/*
 * workaround for lacking preload support inside libc on linux
 */

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND

#define SYMBOL_GETC            SYMBOLPREFIX "getc"
#define SYMBOL_FGETC           SYMBOLPREFIX "fgetc"
#define SYMBOL_GETS            SYMBOLPREFIX "gets"
#define SYMBOL_FGETS           SYMBOLPREFIX "fgets"
#define SYMBOL_PUTC            SYMBOLPREFIX "putc"
#define SYMBOL_FPUTC           SYMBOLPREFIX "fputc"
#define SYMBOL_PUTS            SYMBOLPREFIX "puts"
#define SYMBOL_FPUTS           SYMBOLPREFIX "fputs"
#define SYMBOL_FFLUSH          SYMBOLPREFIX "fflush"
#define SYMBOL_FCLOSE          SYMBOLPREFIX "fclose"
#define SYMBOL_PRINTF          SYMBOLPREFIX "printf"
#define SYMBOL_VPRINTF         SYMBOLPREFIX "vprintf"
#define SYMBOL_FPRINTF         SYMBOLPREFIX "fprintf"
#define SYMBOL_VFPRINTF        SYMBOLPREFIX "vfprintf"
#define SYMBOL_FWRITE          SYMBOLPREFIX "fwrite"
#define SYMBOL_FREAD           SYMBOLPREFIX "fread"

#ifndef LIBRARY_GETC
#define LIBRARY_GETC                        LIBRARY_LIBC
#endif /* !LIBRARY_GETC */

#ifndef LIBRARY_FGETC
#define LIBRARY_FGETC                       LIBRARY_LIBC
#endif /* !LIBRARY_FGETC */

#ifndef LIBRARY_GETS
#define LIBRARY_GETS                        LIBRARY_LIBC
#endif /* !LIBRARY_GETS */

#ifndef LIBRARY_FGETS
#define LIBRARY_FGETS                       LIBRARY_LIBC
#endif /* !LIBRARY_FGETS */

#ifndef LIBRARY_PUTC
#define LIBRARY_PUTC                        LIBRARY_LIBC
#endif /* !LIBRARY_PUTC */

#ifndef LIBRARY_FPUTC
#define LIBRARY_FPUTC                       LIBRARY_LIBC
#endif /* !LIBRARY_FPUTC */

#ifndef LIBRARY_PUTS
#define LIBRARY_PUTS                        LIBRARY_LIBC
#endif /* !LIBRARY_PUTS */

#ifndef LIBRARY_FPUTS
#define LIBRARY_FPUTS                       LIBRARY_LIBC
#endif /* !LIBRARY_FPUTS */

#ifndef LIBRARY_FFLUSH
#define LIBRARY_FFLUSH                      LIBRARY_LIBC
#endif /* !LIBRARY_FFLUSH */

#ifndef LIBRARY_FCLOSE
#define LIBRARY_FCLOSE                      LIBRARY_LIBC
#endif /* !LIBRARY_FCLOSE */

#ifndef LIBRARY_PRINTF
#define LIBRARY_PRINTF                      LIBRARY_LIBC
#endif /* !LIBRARY_PRINTF */

#ifndef LIBRARY_VPRINTF
#define LIBRARY_VPRINTF                     LIBRARY_LIBC
#endif /* !LIBRARY_VPRINTF */

#ifndef LIBRARY_FPRINTF
#define LIBRARY_FPRINTF                     LIBRARY_LIBC
#endif /* !LIBRARY_FPRINTF */

#ifndef LIBRARY_VFPRINTF
#define LIBRARY_VFPRINTF                    LIBRARY_LIBC
#endif /* !LIBRARY_VFPRINTF */

#ifndef LIBRARY_FWRITE
#define LIBRARY_FWRITE                      LIBRARY_LIBC
#endif /* !LIBRARY_FWRITE */

#ifndef LIBRARY_FREAD
#define LIBRARY_FREAD                       LIBRARY_LIBC
#endif /* !LIBRARY_FREAD */

#if HAVE___FPRINTF_CHK
#define SYMBOL___FPRINTF_CHK   SYMBOLPREFIX "__fprintf_chk"
#endif /* HAVE___FPRINTF_CHK */

#if HAVE___VFPRINTF_CHK
#define SYMBOL___VFPRINTF_CHK  SYMBOLPREFIX "__vfprintf_chk"
#endif /* HAVE___VFPRINTF_CHK */

#if HAVE___READ_CHK
#define SYMBOL___READ_CHK  SYMBOLPREFIX "__read_chk"
#endif /* HAVE___READ_CHK */

#if HAVE__IO_GETC
#define SYMBOL__IO_GETC        SYMBOLPREFIX "_IO_getc"
#endif /* HAVE__IO_GETC */

#if HAVE__IO_PUTC
#define SYMBOL__IO_PUTC        SYMBOLPREFIX "_IO_putc"
#endif /* HAVE__IO_PUTC */

#if HAVE___FPRINTF_CHK
#ifndef LIBRARY___FPRINTF_CHK
#define LIBRARY___FPRINTF_CHK               LIBRARY_LIBC
#endif /* !LIBRARY___FPRINTF_CHK */
#endif /* HAVE___FPRINTF_CHK */

#if HAVE___VFPRINTF_CHK
#ifndef LIBRARY___VFPRINTF_CHK
#define LIBRARY___VFPRINTF_CHK              LIBRARY_LIBC
#endif /* !LIBRARY___VFPRINTF_CHK */
#endif /* HAVE___VFPRINTF_CHK */

#if HAVE___READ_CHK
#ifndef LIBRARY___READ_CHK
#define LIBRARY___READ_CHK              LIBRARY_LIBC
#endif /* !LIBRARY___ReAD_CHK */
#endif /* HAVE___READ_CHK */

#if HAVE__IO_GETC
#ifndef LIBRARY__IO_GETC
#define LIBRARY__IO_GETC                    LIBRARY_LIBC
#endif /* !LIBRARY__IO_GETC */
#endif /* HAVE__IO_GETC */

#if HAVE__IO_PUTC
#ifndef LIBRARY__IO_PUTC
#define LIBRARY__IO_PUTC                    LIBRARY_LIBC
#endif /* !LIBRARY__IO_PUTC */
#endif /* HAVE__IO_PUTC */

#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */


/*
 * additional symbols needed for OSF
 */

#if HAVE_EXTRA_OSF_SYMBOLS

#define SYMBOL_EACCEPT                      "_Eaccept"
#define SYMBOL_EGETPEERNAME                 "_Egetpeername"
#define SYMBOL_EGETSOCKNAME                 "_Egetsockname"
#define SYMBOL_EREADV                       "_Ereadv"
#define SYMBOL_ERECVFROM                    "_Erecvfrom"
#define SYMBOL_ERECVMSG                     "_Erecvmsg"
#define SYMBOL_ESENDMSG                     "_Esendmsg"
#define SYMBOL_EWRITEV                      "_Ewritev"

#ifndef LIBRARY_EACCEPT
#define LIBRARY_EACCEPT                     LIBRARY_LIBC
#endif /* !LIBRARY_EACCEPT */

#ifndef LIBRARY_EGETPEERNAME
#define LIBRARY_EGETPEERNAME                LIBRARY_LIBC
#endif /* !LIBRARY_EGETPEERNAME */

#ifndef LIBRARY_EGETSOCKNAME
#define LIBRARY_EGETSOCKNAME                LIBRARY_LIBC
#endif /* !LIBRARY_EGETSOCKNAME */

#ifndef LIBRARY_EREADV
#define LIBRARY_EREADV                      LIBRARY_LIBC
#endif /* !LIBRARY_EREADV */

#ifndef LIBRARY_ERECVFROM
#define LIBRARY_ERECVFROM                   LIBRARY_LIBC
#endif /* !LIBRARY_ERECVFROM */

#ifndef LIBRARY_ERECVMSG
#define LIBRARY_ERECVMSG                    LIBRARY_LIBC
#endif /* !LIBRARY_ERECVMSG */

#ifndef LIBRARY_ESENDMSG
#define LIBRARY_ESENDMSG                    LIBRARY_LIBC
#endif /* !LIBRARY_ESENDMSG */

#ifndef LIBRARY_EWRITEV
#define LIBRARY_EWRITEV                     LIBRARY_LIBC
#endif /* !LIBRARY_EWRITEV */

/* more OSF functions */

#define SYMBOL_NACCEPT                      "naccept"
#define SYMBOL_NGETPEERNAME                 "ngetpeername"
#define SYMBOL_NGETSOCKNAME                 "ngetsockname"
#define SYMBOL_NRECVFROM                    "nrecvfrom"
#define SYMBOL_NRECVMSG                     "nrecvmsg"
#define SYMBOL_NSENDMSG                     "nsendmsg"

#ifndef LIBRARY_NACCEPT
#define LIBRARY_NACCEPT                     LIBRARY_LIBC
#endif /* !LIBRARY_NACCEPT */

#ifndef LIBRARY_NGETPEERNAME
#define LIBRARY_NGETPEERNAME                LIBRARY_LIBC
#endif /* !LIBRARY_NGETPEERNAME */

#ifndef LIBRARY_NGETSOCKNAME
#define LIBRARY_NGETSOCKNAME                LIBRARY_LIBC
#endif /* !LIBRARY_NGETSOCKNAME */

#ifndef LIBRARY_NRECVFROM
#define LIBRARY_NRECVFROM                   LIBRARY_LIBC
#endif /* !LIBRARY_NRECVFROM */

#ifndef LIBRARY_NRECVMSG
#define LIBRARY_NRECVMSG                    LIBRARY_LIBC
#endif /* !LIBRARY_NRECVMSG */

#ifndef LIBRARY_NSENDMSG
#define LIBRARY_NSENDMSG                    LIBRARY_LIBC
#endif /* !LIBRARY_NSENDMSG */

#endif  /* HAVE_EXTRA_OSF_SYMBOLS */


/*
 * additional Solaris functions
 */

#ifdef __sun
#define SYMBOL_XNET_BIND                    "__xnet_bind"
#define SYMBOL_XNET_CONNECT                 "__xnet_connect"
#define SYMBOL_XNET_LISTEN                  "__xnet_listen"
#define SYMBOL_XNET_RECVMSG                 "__xnet_recvmsg"
#define SYMBOL_XNET_SENDMSG                 "__xnet_sendmsg"
#define SYMBOL_XNET_SENDTO                  "__xnet_sendto"
#endif /* __sun */


/*
 * additional Darwin symbols
 */

#if HAVE_DARWIN

#define SYMBOL_CONNECT_NOCANCEL "connect$NOCANCEL"
#define SYMBOL_READ_NOCANCEL "read$NOCANCEL"
#define SYMBOL_RECVFROM_NOCANCEL "recvfrom$NOCANCEL"
#define SYMBOL_SENDTO_NOCANCEL "sendto$NOCANCEL"
#define SYMBOL_WRITE_NOCANCEL "write$NOCANCEL"

#endif /* HAVE_DARWIN */


/*
 * pthread functions
 */

#if HAVE_PTHREAD_H

#define SYMBOL_PT_INIT         SYMBOLPREFIX "pthread_mutex_init"
#define SYMBOL_PT_ATTRINIT     SYMBOLPREFIX "pthread_mutexattr_init"
#define SYMBOL_PT_SETTYPE      SYMBOLPREFIX "pthread_mutexattr_settype"
#define SYMBOL_PT_LOCK         SYMBOLPREFIX "pthread_mutex_lock"
#define SYMBOL_PT_UNLOCK       SYMBOLPREFIX "pthread_mutex_unlock"
#define SYMBOL_PT_SELF         SYMBOLPREFIX "pthread_self"

#ifndef LIBRARY_PTHREAD
#define LIBRARY_PTHREAD                     LIBRARY_LIBC
#endif /* !LIBRARY_PTHREAD */

#endif /* HAVE_PTHREAD_H */
