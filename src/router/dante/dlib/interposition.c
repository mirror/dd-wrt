/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2008, 2009, 2010, 2011,
 *               2012, 2013, 2016, 2017, 2020
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

#define _NO_SUN_PRAGMA

#include "common.h"

static const char rcsid[] =
"$Id: interposition.c,v 1.183.6.11.4.4 2020/11/11 16:11:56 karls Exp $";

#if SOCKSLIBRARY_DYNAMIC

#if (defined __sun) && (defined _XPG4_2)

HAVE_PROT_BIND_0
bind(HAVE_PROT_BIND_1, HAVE_PROT_BIND_2, HAVE_PROT_BIND_3);

HAVE_PROT_CONNECT_0
connect(HAVE_PROT_CONNECT_1, HAVE_PROT_CONNECT_2, HAVE_PROT_CONNECT_3);

HAVE_PROT_LISTEN_0
listen(HAVE_PROT_LISTEN_1, HAVE_PROT_LISTEN_2);

HAVE_PROT_RECVMSG_0
recvmsg(HAVE_PROT_RECVMSG_1, HAVE_PROT_RECVMSG_2, HAVE_PROT_RECVMSG_3);

HAVE_PROT_SENDMSG_0
sendmsg(HAVE_PROT_SENDMSG_1, HAVE_PROT_SENDMSG_2, HAVE_PROT_SENDMSG_3);

HAVE_PROT_SENDTO_0
sendto(HAVE_PROT_SENDTO_1, HAVE_PROT_SENDTO_2, HAVE_PROT_SENDTO_3,
    HAVE_PROT_SENDTO_4, HAVE_PROT_SENDTO_5, HAVE_PROT_SENDTO_6);

#endif /* (defined __sun) && (defined _XPG4_2) */

#if HAVE_DARWIN

HAVE_PROT_CONNECT_0
connect$NOCANCEL(HAVE_PROT_CONNECT_1, HAVE_PROT_CONNECT_2,
       HAVE_PROT_CONNECT_3);
HAVE_PROT_READ_0
read$NOCANCEL(HAVE_PROT_READ_1, HAVE_PROT_READ_2, HAVE_PROT_READ_3);
HAVE_PROT_RECVFROM_0
recvfrom$NOCANCEL(HAVE_PROT_RECVFROM_1, HAVE_PROT_RECVFROM_2,
  HAVE_PROT_RECVFROM_3, HAVE_PROT_RECVFROM_4,
        HAVE_PROT_RECVFROM_5, HAVE_PROT_RECVFROM_6);
HAVE_PROT_SENDTO_0
sendto$NOCANCEL(HAVE_PROT_SENDTO_1, HAVE_PROT_SENDTO_2, HAVE_PROT_SENDTO_3,
      HAVE_PROT_SENDTO_4, HAVE_PROT_SENDTO_5, HAVE_PROT_SENDTO_6);
HAVE_PROT_WRITE_0
write$NOCANCEL(HAVE_PROT_WRITE_1, HAVE_PROT_WRITE_2, HAVE_PROT_WRITE_3);

#endif /* HAVE_DARWIN */

#ifndef __USE_GNU
#define __USE_GNU /* XXX for RTLD_NEXT on Linux */
#endif /* !__USE_GNU */


#include <dlfcn.h>

#ifdef __COVERITY__
/*
 * Coverity naturally has no idea what the function sys_foo calls does,
 * so let it pretend sys_foo is the same as foo.
 * Means Coverity can't catch errors in the code around the call to
 * sys_foo(), but avoids dozens of false positives because Coverity has no
 * idea what the dlopen(3)-ed functions do.
 */
#undef sys_accept
#undef sys_bind
#undef sys_bindresvport
#undef sys_connect
#undef sys_gethostbyname
#undef sys_gethostbyname2
#undef sys_getaddrinfo
#undef sys_getnameinfo
#undef sys_getipnodebyname
#undef sys_getpeername
#undef sys_getsockname
#undef sys_getsockopt
#undef sys_listen
#undef sys_read
#undef sys_readv
#undef sys_recv
#undef sys_recvfrom
#undef sys_recvfrom
#undef sys_recvmsg
#undef sys_rresvport
#undef sys_send
#undef sys_sendmsg
#undef sys_sendto
#undef sys_write
#undef sys_writev
#endif /* __COVERITY__ */

#undef accept
#undef bind
#undef bindresvport
#undef connect
#undef gethostbyaddr
#undef gethostbyname
#undef gethostbyname2
#undef getaddrinfo
#undef getnameinfo
#undef getipnodebyname
#undef freehostent
#undef getpeername
#undef getsockname
#undef getsockopt
#undef listen
#undef read
#undef readv
#undef recv
#undef recvfrom
#undef recvmsg
#undef rresvport
#undef send
#undef sendmsg
#undef sendto
#undef write
#undef writev
#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
#undef getc
#undef fgetc
#undef gets
#undef fgets
#undef putc
#undef fputc
#undef puts
#undef fputs
#undef printf
#undef vprintf
#undef fprintf
#undef vfprintf
#undef fwrite
#undef fread
#undef fflush
#undef fclose

#if HAVE__IO_GETC

#if !HAVE_DECL__IO_GETC
HAVE_PROT__IO_GETC_0 _IO_getc (HAVE_PROT__IO_GETC_1 stream);
#endif /* !HAVE_DECL__IO_GETC */

#undef _IO_getc

#endif /* HAVE__IO_GETC */

#if HAVE__IO_PUTC

#if !HAVE_DECL__IO_PUTC
HAVE_PROT__IO_PUTC_0 _IO_putc (HAVE_PROT__IO_PUTC_1 c, HAVE_PROT__IO_PUTC_2 stream);
#endif /* !HAVE_DECL__IO_PUTC */

#undef _IO_putc

#endif /* HAVE__IO_PUTC */

#if HAVE___FPRINTF_CHK
#undef __fprintf_chk
#endif /* HAVE___FPRINTF_CHK */
#if HAVE___VFPRINTF_CHK
#undef __vfprintf_chk
#endif /* HAVE___VFPRINTF_CHK */
#if HAVE___READ_CHK
#undef __read_chk
#endif /* HAVE___READ_CHK */
#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */

static libsymbol_t libsymbolv[] = {
#if SOCKS_CLIENT
{ SYMBOL_ACCEPT,               LIBRARY_ACCEPT,         NULL,   NULL, NULL },
{ SYMBOL_BIND,                 LIBRARY_BIND,           NULL,   NULL, NULL },
{ SYMBOL_BINDRESVPORT,         LIBRARY_BINDRESVPORT,   NULL,   NULL, NULL },
{ SYMBOL_CONNECT,              LIBRARY_CONNECT,        NULL,   NULL, NULL },
{ SYMBOL_GETPEERNAME,          LIBRARY_GETPEERNAME,    NULL,   NULL, NULL },
{ SYMBOL_GETSOCKNAME,          LIBRARY_GETSOCKNAME,    NULL,   NULL, NULL },
{ SYMBOL_GETSOCKOPT,           LIBRARY_GETSOCKOPT,     NULL,   NULL, NULL },
{ SYMBOL_LISTEN,               LIBRARY_LISTEN,         NULL,   NULL, NULL },
{ SYMBOL_READ,                 LIBRARY_READ,           NULL,   NULL, NULL },
{ SYMBOL_READV,                LIBRARY_READV,          NULL,   NULL, NULL },
{ SYMBOL_RECV,                 LIBRARY_RECV,           NULL,   NULL, NULL },
{ SYMBOL_RECVMSG,              LIBRARY_RECVMSG,        NULL,   NULL, NULL },
{ SYMBOL_RECVFROM,             LIBRARY_RECVFROM,       NULL,   NULL, NULL },

#if HAVE_RRESVPORT
{ SYMBOL_RRESVPORT,            LIBRARY_RRESVPORT,      NULL,   NULL, NULL },
#endif /* HAVE_RRESVPORT */

{ SYMBOL_SEND,                 LIBRARY_SEND,           NULL,   NULL, NULL },
{ SYMBOL_SENDMSG,              LIBRARY_SENDMSG,        NULL,   NULL, NULL },
{ SYMBOL_SENDTO,               LIBRARY_SENDTO,         NULL,   NULL, NULL },
{ SYMBOL_WRITE,                LIBRARY_WRITE,          NULL,   NULL, NULL },
{ SYMBOL_WRITEV,               LIBRARY_WRITEV,         NULL,   NULL, NULL },

{ SYMBOL_GETHOSTBYNAME,        LIBRARY_GETHOSTBYNAME,  NULL,   NULL, NULL },

#if HAVE_GETHOSTBYNAME2
{ SYMBOL_GETHOSTBYNAME2,       LIBRARY_GETHOSTBYNAME2, NULL,   NULL, NULL },
#endif /* HAVE_GETHOSTBYNAME2 */

#if HAVE_GETIPNODEBYNAME
{ SYMBOL_GETIPNODEBYNAME,      LIBRARY_GETIPNODEBYNAME,NULL,   NULL, NULL },
{ SYMBOL_FREEHOSTENT,          LIBRARY_FREEHOSTENT,    NULL,   NULL, NULL },
#endif /* HAVE_GETIPNODEBYNAME */

#if HAVE_GETADDRINFO
{ SYMBOL_GETADDRINFO,          LIBRARY_GETADDRINFO,    NULL,   NULL, NULL },
#endif /* HAVE_GETADDRINFO */

#if HAVE_GETNAMEINFO
{ SYMBOL_GETNAMEINFO,          LIBRARY_GETNAMEINFO,    NULL,   NULL, NULL },
#endif /* HAVE_GETNAMEINFO */

#ifdef __sun
{ SYMBOL_XNET_BIND,            LIBRARY_BIND,           NULL,   NULL, NULL },
{ SYMBOL_XNET_CONNECT,         LIBRARY_CONNECT,        NULL,   NULL, NULL },
{ SYMBOL_XNET_LISTEN,          LIBRARY_LISTEN,         NULL,   NULL, NULL },
{ SYMBOL_XNET_RECVMSG,         LIBRARY_RECVMSG,        NULL,   NULL, NULL },
{ SYMBOL_XNET_SENDMSG,         LIBRARY_SENDMSG,        NULL,   NULL, NULL },
{ SYMBOL_XNET_SENDTO,          LIBRARY_SENDTO,         NULL,   NULL, NULL },
#endif /* __sun */

#if HAVE_DARWIN
{ SYMBOL_CONNECT_NOCANCEL,     LIBRARY_CONNECT,        NULL,   NULL, NULL },
{ SYMBOL_READ_NOCANCEL,        LIBRARY_READ,           NULL,   NULL, NULL },
{ SYMBOL_RECVFROM_NOCANCEL,    LIBRARY_RECVFROM,       NULL,   NULL, NULL },
{ SYMBOL_SENDTO_NOCANCEL,      LIBRARY_SENDTO,         NULL,   NULL, NULL },
{ SYMBOL_WRITE_NOCANCEL,       LIBRARY_WRITE,          NULL,   NULL, NULL },
#endif /* HAVE_DARWIN */

#if HAVE_EXTRA_OSF_SYMBOLS
{ SYMBOL_EACCEPT,              LIBRARY_EACCEPT,        NULL,   NULL, NULL },
{ SYMBOL_EGETPEERNAME,         LIBRARY_EGETPEERNAME,   NULL,   NULL, NULL },
{ SYMBOL_EGETSOCKNAME,         LIBRARY_EGETSOCKNAME,   NULL,   NULL, NULL },
{ SYMBOL_EREADV,               LIBRARY_EREADV,         NULL,   NULL, NULL },
{ SYMBOL_ERECVFROM,            LIBRARY_ERECVFROM,      NULL,   NULL, NULL },
{ SYMBOL_ERECVMSG,             LIBRARY_ERECVMSG,       NULL,   NULL, NULL },
{ SYMBOL_ESENDMSG,             LIBRARY_ESENDMSG,       NULL,   NULL, NULL },
{ SYMBOL_EWRITEV,              LIBRARY_EWRITEV,        NULL,   NULL, NULL },
{ SYMBOL_NACCEPT,              LIBRARY_EACCEPT,        NULL,   NULL, NULL },
{ SYMBOL_NGETPEERNAME,         LIBRARY_NGETPEERNAME,   NULL,   NULL, NULL },
{ SYMBOL_NGETSOCKNAME,         LIBRARY_NGETSOCKNAME,   NULL,   NULL, NULL },
{ SYMBOL_NRECVFROM,            LIBRARY_NRECVFROM,      NULL,   NULL, NULL },
{ SYMBOL_NRECVMSG,             LIBRARY_NRECVMSG,       NULL,   NULL, NULL },
{ SYMBOL_NSENDMSG,             LIBRARY_NSENDMSG,       NULL,   NULL, NULL },
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
{ SYMBOL_GETC,                 LIBRARY_GETC,           NULL,   NULL, NULL },
{ SYMBOL_FGETC,                LIBRARY_FGETC,          NULL,   NULL, NULL },
{ SYMBOL_GETS,                 LIBRARY_GETS,           NULL,   NULL, NULL },
{ SYMBOL_FGETS,                LIBRARY_FGETS,          NULL,   NULL, NULL },
{ SYMBOL_PUTC,                 LIBRARY_PUTC,           NULL,   NULL, NULL },
{ SYMBOL_FPUTC,                LIBRARY_FPUTC,          NULL,   NULL, NULL },
{ SYMBOL_PUTS,                 LIBRARY_PUTS,           NULL,   NULL, NULL },
{ SYMBOL_FPUTS,                LIBRARY_FPUTS,          NULL,   NULL, NULL },
{ SYMBOL_PRINTF,               LIBRARY_PRINTF,         NULL,   NULL, NULL },
{ SYMBOL_VPRINTF,              LIBRARY_VPRINTF,        NULL,   NULL, NULL },
{ SYMBOL_FPRINTF,              LIBRARY_FPRINTF,        NULL,   NULL, NULL },
{ SYMBOL_VFPRINTF,             LIBRARY_VFPRINTF,       NULL,   NULL, NULL },
{ SYMBOL_FWRITE,               LIBRARY_FWRITE,         NULL,   NULL, NULL },
{ SYMBOL_FREAD,                LIBRARY_FREAD,          NULL,   NULL, NULL },
{ SYMBOL_FFLUSH,               LIBRARY_FFLUSH,         NULL,   NULL, NULL },
{ SYMBOL_FCLOSE,               LIBRARY_FCLOSE,         NULL,   NULL, NULL },

#if HAVE__IO_GETC
{ SYMBOL__IO_GETC,             LIBRARY__IO_GETC,       NULL,   NULL, NULL },
#endif /* HAVE__IO_GETC */

#if HAVE__IO_PUTC
{ SYMBOL__IO_PUTC,             LIBRARY__IO_PUTC,       NULL,   NULL, NULL },
#endif /* HAVE__IO_PUTC */

#if HAVE___FPRINTF_CHK
{ SYMBOL___FPRINTF_CHK,        LIBRARY___FPRINTF_CHK,  NULL,   NULL, NULL },
#endif /* HAVE___FPRINTF_CHK */

#if HAVE___VFPRINTF_CHK
{ SYMBOL___VFPRINTF_CHK,       LIBRARY___VFPRINTF_CHK, NULL,   NULL, NULL },
#endif /* HAVE___VFPRINTF_CHK */

#if HAVE___READ_CHK
{ SYMBOL___READ_CHK,       LIBRARY___READ_CHK, NULL,   NULL, NULL },
#endif /* HAVE___READ_CHK */

#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */

#else /* SERVER */

/*
 * symbols we want to interpose in the server for library functions
 * that might call them (e.g. pam/ldap/gssapi).  Lets them use our
 * superior caching versions.  Unfortunately, getaddrinfo(3) does
 * lend itself easily to that, so even if we would have liked to
 * let that also use our cached data, the only solutions I can
 * think of seem to fragile to be usable.  So regarding that newer api,
 * only getnameinfo(3) uses our cached version.
 */


/*
 * No point in having it in the client as either the system-calls used by
 * gethostbyaddr(3) gets socksified, in which case there is nothing special
 * for us to do related to gethostbyaddr(3), or they do not get socksified,
 * in which case we have no idea what hostname the address resolves to and
 * there is nothing we can reasonably fake anyway.
 */
{ SYMBOL_GETHOSTBYADDR,        LIBRARY_GETHOSTBYADDR,  NULL,   NULL, NULL },
{ SYMBOL_GETHOSTBYNAME,        LIBRARY_GETHOSTBYNAME,  NULL,   NULL, NULL },

#endif /* SERVER */
};

#if SOCKS_CLIENT

/*
 * During init, we need to let all system calls resolve to the native
 * version.  I.e., socks_shouldcallasnative() need to always return
 * true as long as we are initing. Use this object for holding that
 * knowledge.
 */
#ifdef HAVE_VOLATILE_SIG_ATOMIC_T

extern sig_atomic_t doing_addrinit;

#else

extern volatile sig_atomic_t doing_addrinit;

#endif /* HAVE_VOLATILE_SIG_ATOMIC_T */

#define DNSCODE_START()                                                        \
do {                                                                           \
      slog(LOG_DEBUG,                                                          \
           "DNSCODE_START: %d", (int)++sockscf.state.executingdnscode);        \
} while (/* CONSTCOND */ 0)

#define DNSCODE_END()                                                          \
do {                                                                           \
      slog(LOG_DEBUG,                                                          \
           "DNSCODE_END: %d", (int)--sockscf.state.executingdnscode);    \
} while (/* CONSTCOND */ 0)


static void
addtolist(const char *functionname, const socks_id_t *id);
/*
 * Add "id" to the list of id's for which function name should resolve
 * to the native system call directly.
 */

static void
removefromlist(const char *functionname, const socks_id_t *id);
/*
 * Add "id" to the list of id's for which function name should resolve
 * to the native system call directly.
 */

static int
idsareequal(const socks_id_t *a, const socks_id_t *b);
/*
 * If "a" and "b" refer to the same thread/pid, return true.  Else, false.
 */



#else /* !SOCKS_CLIENT */

#define DNSCODE_START()
#define DNSCODE_END()

#endif /* !SOCKS_CLIENT */

static libsymbol_t *
libsymbol(const char *symbol);
/*
 * Finds the libsymbol that "symbol" is defined in.
 */


#if SOCKS_CLIENT

int
socks_issyscall(s, name)
   const int s;
   const char *name;
{
   socksfd_t socksfd;

   if (s < 0)
      return 1;

   if (socks_shouldcallasnative(name))
      return 1;

   if (!fd_is_network_socket(s))
      return 1;

   if (socks_getaddr(s, &socksfd, 1) != NULL
   &&  socksfd.state.syscalldepth > 0)
      return 1;

   return 0;
}

void
socks_syscall_start(s)
   const int s;
{
   socksfd_t *p;
   addrlockopaque_t opaque;

   if (doing_addrinit || sockscf.state.executingdnscode)
      return;

   if (s < 0)
      return;

   if (socks_logmatch(s, &sockscf.log)
   ||  socks_logmatch(s, &sockscf.errlog))
      return; /* don't set up things for our logging fd's - creates problems. */

   socks_addrlock(F_WRLCK, &opaque);

   if ((p = socks_getaddr(s, NULL, 0)) == NULL) {
      socksfd_t socksfd;

      bzero(&socksfd, sizeof(socksfd));
      socksfd.state.command   = -1;
      socksfd.state.issyscall = 1;
      p = socks_addaddr(s, &socksfd, 0);
   }

   SASSERTX(p != NULL);

   ++(p->state.syscalldepth);
   socks_addaddr(s, p, 0);

   socks_addrunlock(&opaque);
}

void
socks_syscall_end(s)
   const int s;
{
   addrlockopaque_t opaque;
   socksfd_t socksfd, *p;

   if (doing_addrinit || sockscf.state.executingdnscode)
      return;

   if (s < 0)
      return;

   if (socks_logmatch(s, &sockscf.log)
   ||  socks_logmatch(s, &sockscf.errlog))
      return; /* don't set up things for our logging fd's, creates problems. */

   socks_addrlock(F_WRLCK, &opaque);

   p = socks_getaddr(s, &socksfd, 0);

   if (p == NULL) { /* should not happen ... */
      socks_addrunlock(&opaque);
      return;
   }

   if (p->state.syscalldepth <= 0)
      ; /* should not happen ... */
   else
      --(p->state.syscalldepth);

   if (p->state.syscalldepth <= 0) { /* all finished. */
      if (p->state.issyscall) /* started out as a syscall, remove now. */
         socks_rmaddr(s, 0);
      else
         socks_addaddr(s, &socksfd, 0); /* update. */
   }
   else
      socks_addaddr(s, &socksfd, 0); /* update. */

   socks_addrunlock(&opaque);
}

static void
addtolist(functionname, id)
   const char *functionname;
   const socks_id_t *id;
{
   const char *function = "addtolist()";
   libsymbol_t *lib;
   socks_id_t *newid;

   addrlockopaque_t opaque;

   lib = libsymbol(functionname);
   SASSERTX(lib != NULL);

   if ((newid = malloc(sizeof(*newid))) == NULL)
      serr("%s: failed to malloc %lu bytes",
           function, (unsigned long)sizeof(*newid));

   *newid = *id;

   socks_addrlock(F_WRLCK, &opaque);

   if (lib->dosyscall == NULL) {
      lib->dosyscall       = newid;
      lib->dosyscall->next = NULL;
   }
   else {
      newid->next          = lib->dosyscall->next;
      lib->dosyscall->next = newid;
   }

   socks_addrunlock(&opaque);
}

static void
removefromlist(functionname, removeid)
   const char *functionname;
   const socks_id_t *removeid;
{
/*   const char *function = "removefromlist()"; */
   libsymbol_t *lib;
   socks_id_t *id, *previous;
   addrlockopaque_t opaque;

   lib = libsymbol(functionname);
   SASSERTX(lib != NULL);
   SASSERTX(lib->dosyscall != NULL);

   socks_addrlock(F_WRLCK, &opaque);

   SASSERTX(idsareequal(lib->dosyscall, removeid));

   previous = lib->dosyscall;

   if (idsareequal(lib->dosyscall, removeid)) {
      lib->dosyscall = lib->dosyscall->next;
      free(previous);
   }
   else {
      for (id = previous->next; id != NULL; previous = id, id = id->next) {
         if (idsareequal(id, removeid)) {
            previous->next = id->next;
            free(id);

            break;
         }
      }

      SASSERTX(id != NULL);
   }

   socks_addrunlock(&opaque);
}

#endif /* SOCKS_CLIENT */

void
symbolcheck(void)
{
   size_t i;

   for (i = 0; i < ELEMENTS(libsymbolv); ++i)
      symbolfunction(libsymbolv[i].symbol);
}


int
socks_shouldcallasnative(symbol)
   const char *symbol;
{
#if SOCKS_CLIENT
   socks_id_t myid, *fid;
   libsymbol_t *lib;

   if (doing_addrinit || sockscf.state.executingdnscode)
      return 1;

   lib = libsymbol(symbol);
   SASSERTX(lib != NULL);

   if ((fid = lib->dosyscall) == NULL)
      return 0;

   socks_whoami(&myid);

   for (; fid != NULL; fid = fid->next)
      if (idsareequal(&myid, fid))
         return 1;

   return 0;

#else /* ! SOCKS_CLIENT */

   return 1;

#endif /* !SOCKS_CLIENT */

}

void
socks_markasnative(symbol)
   const char *symbol;
{
#if SOCKS_CLIENT
   const char *function = "socks_markasnative()";
   socks_id_t myid;

   if (sockscf.option.debug > DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: marking %s as native for current id",
           function, symbol);

   if (strcmp(symbol, "*") == 0) {
      size_t i;

      for (i = 0; i < ELEMENTS(libsymbolv); ++i)
         socks_markasnative(libsymbolv[i].symbol);

      return;
   }

   socks_whoami(&myid);
   addtolist(symbol, &myid);
#endif /* !SOCKS_CLIENT */
}

void
socks_markasnormal(symbol)
   const char *symbol;
{
#if SOCKS_CLIENT
   const char *function = "socks_markasnormal()";
   socks_id_t myid;

   if (sockscf.option.debug > DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: marking %s as normal for current id",
           function, symbol);

   if (strcmp(symbol, "*") == 0) {
      size_t i;

      for (i = 0; i < ELEMENTS(libsymbolv); ++i)
         socks_markasnormal(libsymbolv[i].symbol);

      return;
   }

   socks_whoami(&myid);
   removefromlist(symbol, &myid);
#endif /* !SOCKS_CLIENT */
}

void
socks_mark_io_as_native()
{
#if SOCKS_CLIENT
   const char *function = "socks_mark_io_as_native()";

   slog(LOG_DEBUG, "%s: marking i/o calls as native ...", function);
   socks_markasnative("*");
#endif /* !SOCKS_CLIENT */
}

void
socks_mark_io_as_normal()
{
#if SOCKS_CLIENT
   const char *function = "socks_mark_io_as_normal()";

   slog(LOG_DEBUG, "%s: marking io-related calls as normal again", function);
   socks_markasnormal("*");
#endif /* !SOCKS_CLIENT */
}

void *
symbolfunction(symbol)
   const char *symbol;
{
   const char *function = "symbolfunction()";
   libsymbol_t *lib;

   lib = libsymbol(symbol);

   SASSERTX(lib != NULL);
   SASSERTX(lib->library != NULL);
   SASSERTX(strcmp(lib->symbol, symbol) == 0);

#if HAVE_RTLD_NEXT
   if (lib->function == NULL) {
      if ((lib->function = dlsym(RTLD_NEXT, symbol)) == NULL) {
         if (strcmp(symbol, SYMBOL_WRITE) != 0)
            serrx("%s: compile time configuration error?  "
                  "Failed to find \"%s\" using RTLD_NEXT: %s",
                  function, symbol, dlerror());
      }
#if 0
      else {
         if (strcmp(symbol, SYMBOL_WRITE) != 0)
            slog(LOG_DEBUG, "found symbol %s using RTLD_NEXT", lib->symbol);
      }
#endif
   }

#else /* !HAVE_RTLD_NEXT */
   if (lib->handle == NULL)
      if ((lib->handle = dlopen(lib->library, DL_LAZY)) == NULL)
         serrx("%s: compile time configuration error?  "
               "Failed to open library \"%s\": %s",
               function, lib->library, dlerror());

   if (lib->function == NULL)
      if ((lib->function = dlsym(lib->handle, symbol)) == NULL)
         serrx("%s: compile time configuration error?  "
               "Failed to find \"%s\" in \"%s\": %s",
               function, symbol, lib->library, dlerror());

#if 0
   if (strcmp(symbol, SYMBOL_WRITE) != 0)
      slog(LOG_DEBUG, "found symbol %s in library %s",
      lib->symbol, lib->library);
#endif

#endif /* !HAVE_RLTD_NEXT */

   return lib->function;
}

#if SOCKS_CLIENT
   /* the real system calls. */


#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_ACCEPT_0
sys_accept(s, addr, addrlen)
   HAVE_PROT_ACCEPT_1 s;
   HAVE_PROT_ACCEPT_2 addr;
   HAVE_PROT_ACCEPT_3 addrlen;
{
   int rc;
   typedef HAVE_PROT_ACCEPT_0 (*ACCEPT_FUNC_T)(HAVE_PROT_ACCEPT_1,
                                               HAVE_PROT_ACCEPT_2,
                                               HAVE_PROT_ACCEPT_3);
   ACCEPT_FUNC_T function = (ACCEPT_FUNC_T)symbolfunction(SYMBOL_ACCEPT);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, addr, addrlen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_BIND_0
sys_bind(s, name, namelen)
   HAVE_PROT_BIND_1 s;
   HAVE_PROT_BIND_2 name;
   HAVE_PROT_BIND_3 namelen;
{
   int rc;
   typedef HAVE_PROT_BIND_0 (*BIND_FUNC_T)(HAVE_PROT_BIND_1,
                                           HAVE_PROT_BIND_2,
                                           HAVE_PROT_BIND_3);
   BIND_FUNC_T function = (BIND_FUNC_T)symbolfunction(SYMBOL_BIND);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

int
sys_bindresvport(sd, sin)
   int sd;
   struct sockaddr_in *sin;
{
   int rc;
   typedef int (*BINDRESVPORT_FUNC_T)(int, struct sockaddr_in *);
   BINDRESVPORT_FUNC_T function
   = (BINDRESVPORT_FUNC_T)symbolfunction(SYMBOL_BINDRESVPORT);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(sd);

   rc = function(sd, sin);

   if (tagged)
      socks_syscall_end(sd);

   return rc;
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_CONNECT_0
sys_connect(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   int rc;
   typedef HAVE_PROT_CONNECT_0 (*CONNECT_FUNC_T)(HAVE_PROT_CONNECT_1,
                                                 HAVE_PROT_CONNECT_2,
                                                 HAVE_PROT_CONNECT_3);
   CONNECT_FUNC_T function = (CONNECT_FUNC_T)symbolfunction(SYMBOL_CONNECT);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_GETPEERNAME_0
sys_getpeername(s, name, namelen)
   HAVE_PROT_GETPEERNAME_1 s;
   HAVE_PROT_GETPEERNAME_2 name;
   HAVE_PROT_GETPEERNAME_3 namelen;
{
   int rc;
   typedef HAVE_PROT_GETPEERNAME_0
       (*GETPEERNAME_FUNC_T)(HAVE_PROT_GETPEERNAME_1,
                             HAVE_PROT_GETPEERNAME_2,
                             HAVE_PROT_GETPEERNAME_3);
   GETPEERNAME_FUNC_T function
   = (GETPEERNAME_FUNC_T)symbolfunction(SYMBOL_GETPEERNAME);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_GETSOCKNAME_0
sys_getsockname(s, name, namelen)
   HAVE_PROT_GETSOCKNAME_1 s;
   HAVE_PROT_GETSOCKNAME_2 name;
   HAVE_PROT_GETSOCKNAME_3 namelen;
{
   int rc;
   typedef HAVE_PROT_GETSOCKNAME_0
       (*GETSOCKNAME_FUNC_T)(HAVE_PROT_GETSOCKNAME_1,
                             HAVE_PROT_GETSOCKNAME_2,
                             HAVE_PROT_GETSOCKNAME_3);
   GETSOCKNAME_FUNC_T function
   = (GETSOCKNAME_FUNC_T)symbolfunction(SYMBOL_GETSOCKNAME);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_GETSOCKNAME_0
sys_getsockname_notracking(s, name, namelen)
   HAVE_PROT_GETSOCKNAME_1 s;
   HAVE_PROT_GETSOCKNAME_2 name;
   HAVE_PROT_GETSOCKNAME_3 namelen;
{
   typedef HAVE_PROT_GETSOCKNAME_0
       (*GETSOCKNAME_FUNC_T)(HAVE_PROT_GETSOCKNAME_1,
                             HAVE_PROT_GETSOCKNAME_2,
                             HAVE_PROT_GETSOCKNAME_3);
   GETSOCKNAME_FUNC_T function
   = (GETSOCKNAME_FUNC_T)symbolfunction(SYMBOL_GETSOCKNAME);

   return function(s, name, namelen);
}


#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_GETSOCKOPT_0
sys_getsockopt(s, level, optname, optval, optlen)
   HAVE_PROT_GETSOCKOPT_1 s;
   HAVE_PROT_GETSOCKOPT_2 level;
   HAVE_PROT_GETSOCKOPT_3 optname;
   HAVE_PROT_GETSOCKOPT_4 optval;
   HAVE_PROT_GETSOCKOPT_5 optlen;
{
   int rc;
   typedef HAVE_PROT_GETSOCKOPT_0
       (*GETSOCKOPT_FUNC_T)(HAVE_PROT_GETSOCKOPT_1,
                             HAVE_PROT_GETSOCKOPT_2,
                             HAVE_PROT_GETSOCKOPT_3,
                             HAVE_PROT_GETSOCKOPT_4,
                             HAVE_PROT_GETSOCKOPT_5);
   GETSOCKOPT_FUNC_T function
   = (GETSOCKOPT_FUNC_T)symbolfunction(SYMBOL_GETSOCKOPT);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, level, optname, optval, optlen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_LISTEN_0
sys_listen(s, backlog)
   HAVE_PROT_LISTEN_1 s;
   HAVE_PROT_LISTEN_2 backlog;
{
   HAVE_PROT_LISTEN_0 rc;
   typedef HAVE_PROT_LISTEN_0 (*LISTEN_FUNC_T)(HAVE_PROT_LISTEN_1,
                                           HAVE_PROT_LISTEN_2);
   LISTEN_FUNC_T function = (LISTEN_FUNC_T)symbolfunction(SYMBOL_LISTEN);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, backlog);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_READ_0
sys_read(d, buf, nbytes)
   HAVE_PROT_READ_1 d;
   HAVE_PROT_READ_2 buf;
   HAVE_PROT_READ_3 nbytes;
{
   ssize_t rc;
   typedef HAVE_PROT_READ_0 (*READ_FUNC_T)(HAVE_PROT_READ_1,
                                           HAVE_PROT_READ_2,
                                           HAVE_PROT_READ_3);
   READ_FUNC_T function = (READ_FUNC_T)symbolfunction(SYMBOL_READ);

   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(d, buf, nbytes);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_READV_0
sys_readv(d, iov, iovcnt)
   HAVE_PROT_READV_1 d;
   HAVE_PROT_READV_2 iov;
   HAVE_PROT_READV_3 iovcnt;
{
   ssize_t rc;
   typedef HAVE_PROT_READV_0 (*READV_FUNC_T)(HAVE_PROT_READV_1,
                                             HAVE_PROT_READV_2,
                                             HAVE_PROT_READV_3);
   READV_FUNC_T function = (READV_FUNC_T)symbolfunction(SYMBOL_READV);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(d, iov, iovcnt);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_RECV_0
sys_recv(s, buf, len, flags)
   HAVE_PROT_RECV_1 s;
   HAVE_PROT_RECV_2 buf;
   HAVE_PROT_RECV_3 len;
   HAVE_PROT_RECV_4 flags;
{
   ssize_t rc;
   typedef HAVE_PROT_RECV_0 (*RECV_FUNC_T)(HAVE_PROT_RECV_1,
                                           HAVE_PROT_RECV_2,
                                           HAVE_PROT_RECV_3,
                                           HAVE_PROT_RECV_4);
   RECV_FUNC_T function = (RECV_FUNC_T)symbolfunction(SYMBOL_RECV);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, buf, len, flags);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_RECVFROM_0
sys_recvfrom(s, buf, len, flags, from, fromlen)
   HAVE_PROT_RECVFROM_1 s;
   HAVE_PROT_RECVFROM_2 buf;
   HAVE_PROT_RECVFROM_3 len;
   HAVE_PROT_RECVFROM_4 flags;
   HAVE_PROT_RECVFROM_5 from;
   HAVE_PROT_RECVFROM_6 fromlen;
{
   HAVE_PROT_RECVFROM_0 rc;
   typedef HAVE_PROT_RECVFROM_0 (*RECVFROM_FUNC_T)(HAVE_PROT_RECVFROM_1,
                                                   HAVE_PROT_RECVFROM_2,
                                                   HAVE_PROT_RECVFROM_3,
                                                   HAVE_PROT_RECVFROM_4,
                                                   HAVE_PROT_RECVFROM_5,
                                                   HAVE_PROT_RECVFROM_6);
   RECVFROM_FUNC_T function = (RECVFROM_FUNC_T)symbolfunction(SYMBOL_RECVFROM);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, buf, len, flags, from, fromlen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_RECVMSG_0
sys_recvmsg(s, msg, flags)
   HAVE_PROT_RECVMSG_1 s;
   HAVE_PROT_RECVMSG_2 msg;
   HAVE_PROT_RECVMSG_3 flags;
{
   ssize_t rc;
   typedef HAVE_PROT_RECVMSG_0 (*RECVMSG_FUNC_T)(HAVE_PROT_RECVMSG_1,
                                                 HAVE_PROT_RECVMSG_2,
                                                 HAVE_PROT_RECVMSG_3);
   RECVMSG_FUNC_T function = (RECVMSG_FUNC_T)symbolfunction(SYMBOL_RECVMSG);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, flags);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_RRESVPORT
int
sys_rresvport(port)
   int *port;
{
   typedef int (*RRESVPORT_FUNC_T)(int *);
   RRESVPORT_FUNC_T function;

   function = (RRESVPORT_FUNC_T)symbolfunction(SYMBOL_RRESVPORT);
   return function(port);
}
#endif /* HAVE_RRESVPORT */

HAVE_PROT_SEND_0
sys_send(s, msg, len, flags)
   HAVE_PROT_SEND_1 s;
   HAVE_PROT_SEND_2 msg;
   HAVE_PROT_SEND_3 len;
   HAVE_PROT_SEND_4 flags;
{
   ssize_t rc;
   typedef HAVE_PROT_SEND_0 (*SEND_FUNC_T)(HAVE_PROT_SEND_1,
                                           HAVE_PROT_SEND_2,
                                           HAVE_PROT_SEND_3,
                                           HAVE_PROT_SEND_4);
   SEND_FUNC_T function = (SEND_FUNC_T)symbolfunction(SYMBOL_SEND);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, len, flags);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_SENDMSG_0
sys_sendmsg(s, msg, flags)
   HAVE_PROT_SENDMSG_1 s;
   HAVE_PROT_SENDMSG_2 msg;
   HAVE_PROT_SENDMSG_3 flags;
{
   ssize_t rc;
   typedef HAVE_PROT_SENDMSG_0 (*SENDMSG_FUNC_T)(HAVE_PROT_SENDMSG_1,
                                                 HAVE_PROT_SENDMSG_2,
                                                 HAVE_PROT_SENDMSG_3);
   SENDMSG_FUNC_T function = (SENDMSG_FUNC_T)symbolfunction(SYMBOL_SENDMSG);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, flags);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_SENDTO_0
sys_sendto(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   ssize_t rc;
   typedef HAVE_PROT_SENDTO_0 (*SENDTO_FUNC_T)(HAVE_PROT_SENDTO_1,
                                               HAVE_PROT_SENDTO_2,
                                               HAVE_PROT_SENDTO_3,
                                               HAVE_PROT_SENDTO_4,
                                               HAVE_PROT_SENDTO_5,
                                               HAVE_PROT_SENDTO_6);
   SENDTO_FUNC_T function = (SENDTO_FUNC_T)symbolfunction(SYMBOL_SENDTO);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, len, flags, to, tolen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_WRITE_0
sys_write(d, buf, nbytes)
   HAVE_PROT_WRITE_1 d;
   HAVE_PROT_WRITE_2 buf;
   HAVE_PROT_WRITE_3 nbytes;
{
   ssize_t rc;
   typedef HAVE_PROT_WRITE_0 (*WRITE_FUNC_T)(HAVE_PROT_WRITE_1,
                                             HAVE_PROT_WRITE_2,
                                             HAVE_PROT_WRITE_3);
   WRITE_FUNC_T function = (WRITE_FUNC_T)symbolfunction(SYMBOL_WRITE);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(d, buf, nbytes);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

#if DEBUG
HAVE_PROT_WRITE_0 syssys_write(HAVE_PROT_WRITE_1 d, HAVE_PROT_WRITE_2 buf,
                                HAVE_PROT_WRITE_3 nbytes);
HAVE_PROT_WRITE_0
syssys_write(d, buf, nbytes)
   HAVE_PROT_WRITE_1 d;
   HAVE_PROT_WRITE_2 buf;
   HAVE_PROT_WRITE_3 nbytes;
{
   typedef HAVE_PROT_WRITE_0 (*WRITE_FUNC_T)(HAVE_PROT_WRITE_1,
                                             HAVE_PROT_WRITE_2,
                                             HAVE_PROT_WRITE_3);
   WRITE_FUNC_T function;

   function = (WRITE_FUNC_T)symbolfunction(SYMBOL_WRITE);
   return function(d, buf, nbytes);
}
#endif /* DEBUG */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_WRITEV_0
sys_writev(d, iov, iovcnt)
   HAVE_PROT_WRITEV_1 d;
   HAVE_PROT_WRITEV_2 iov;
   HAVE_PROT_WRITEV_3 iovcnt;
{
   ssize_t rc;
   typedef HAVE_PROT_WRITEV_0 (*WRITEV_FUNC_T)(HAVE_PROT_WRITEV_1,
                                               HAVE_PROT_WRITEV_2,
                                               HAVE_PROT_WRITEV_3);
   WRITEV_FUNC_T function = (WRITEV_FUNC_T)symbolfunction(SYMBOL_WRITEV);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(d, iov, iovcnt);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
HAVE_PROT_GETC_0
sys_getc(stream)
   HAVE_PROT_GETC_1 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_GETC_0 rc;
   typedef HAVE_PROT_GETC_0 (*GETC_FUNC_T)(HAVE_PROT_GETC_1);
   GETC_FUNC_T function = (GETC_FUNC_T)symbolfunction(SYMBOL_GETC);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FGETC_0
sys_fgetc(stream)
   HAVE_PROT_FGETC_1 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FGETC_0 rc;
   typedef HAVE_PROT_FGETC_0 (*FGETC_FUNC_T)(HAVE_PROT_FGETC_1);
   FGETC_FUNC_T function = (FGETC_FUNC_T)symbolfunction(SYMBOL_FGETC);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_GETS_0
sys_gets(buf)
   HAVE_PROT_GETS_1 buf;
{
   const int d = fileno(stdin);
   HAVE_PROT_GETS_0 rv;
   typedef HAVE_PROT_GETS_0(*GETS_FUNC_T)(HAVE_PROT_GETS_1);
   GETS_FUNC_T function = (GETS_FUNC_T)symbolfunction(SYMBOL_GETS);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rv = function(buf);

   if (tagged)
      socks_syscall_end(d);

   return rv;
}

HAVE_PROT_FGETS_0
sys_fgets(buf, size, stream)
   HAVE_PROT_FGETS_1 buf;
   HAVE_PROT_FGETS_2 size;
   HAVE_PROT_FGETS_3 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FGETS_0 rc;
   typedef HAVE_PROT_FGETS_0(*FGETS_FUNC_T)(HAVE_PROT_FGETS_1,
                                            HAVE_PROT_FGETS_2,
                                            HAVE_PROT_FGETS_3);
   FGETS_FUNC_T function = (FGETS_FUNC_T)symbolfunction(SYMBOL_FGETS);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(buf, size, stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_PUTC_0
sys_putc(c, stream)
   HAVE_PROT_PUTC_1 c;
   HAVE_PROT_PUTC_2 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_PUTC_0 rc;
   typedef HAVE_PROT_PUTC_0(*PUTC_FUNC_T)(HAVE_PROT_PUTC_1, HAVE_PROT_PUTC_2);
   PUTC_FUNC_T function = (PUTC_FUNC_T)symbolfunction(SYMBOL_PUTC);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(c, stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FPUTC_0
sys_fputc(c, stream)
   HAVE_PROT_FPUTC_1 c;
   HAVE_PROT_FPUTC_2 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FPUTC_0 rc;
   typedef HAVE_PROT_FPUTC_0 (*FPUTC_FUNC_T)(HAVE_PROT_FPUTC_1,
                                             HAVE_PROT_FPUTC_2);
   FPUTC_FUNC_T function = (FPUTC_FUNC_T)symbolfunction(SYMBOL_FPUTC);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(c, stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_PUTS_0
sys_puts(buf)
   HAVE_PROT_PUTS_1 buf;
{
   const int d = fileno(stdout);
   HAVE_PROT_PUTS_0 rc;
   typedef HAVE_PROT_PUTS_0 (*PUTS_FUNC_T)(HAVE_PROT_PUTS_1);
   PUTS_FUNC_T function = (PUTS_FUNC_T)symbolfunction(SYMBOL_PUTS);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(buf);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FPUTS_0
sys_fputs(buf, stream)
   HAVE_PROT_FPUTS_1 buf;
   HAVE_PROT_FPUTS_2 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FPUTS_0 rc;
   typedef HAVE_PROT_FPUTS_0 (*FPUTS_FUNC_T)(HAVE_PROT_FPUTS_1,
                                             HAVE_PROT_FPUTS_2);
   FPUTS_FUNC_T function = (FPUTS_FUNC_T)symbolfunction(SYMBOL_FPUTS);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(buf, stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FFLUSH_0
sys_fflush(stream)
   HAVE_PROT_FFLUSH_1 stream;
{
   HAVE_PROT_FFLUSH_0 rc;
   typedef HAVE_PROT_FFLUSH_0 (*FFLUSH_FUNC_T)(HAVE_PROT_FFLUSH_1);
   FFLUSH_FUNC_T function = (FFLUSH_FUNC_T)symbolfunction(SYMBOL_FFLUSH);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (stream != NULL && tagged)
      socks_syscall_start(fileno(stream));

   rc = function(stream);

   if (stream != NULL && tagged)
      socks_syscall_end(fileno(stream));

   return rc;
}

HAVE_PROT_FCLOSE_0
sys_fclose(stream)
   HAVE_PROT_FCLOSE_1 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FCLOSE_0 rc;
   typedef HAVE_PROT_FCLOSE_0 (*FCLOSE_FUNC_T)(HAVE_PROT_FCLOSE_1);
   FCLOSE_FUNC_T function = (FCLOSE_FUNC_T)symbolfunction(SYMBOL_FCLOSE);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_PRINTF_0
sys_printf(HAVE_PROT_PRINTF_1 format, ...)
{
   va_list ap;
   HAVE_PROT_FPRINTF_0 rc;

   va_start(ap, format);
   rc = sys_vprintf(format, ap);
   va_end(ap);
   return rc;
}

HAVE_PROT_VPRINTF_0
sys_vprintf(format, ap)
   HAVE_PROT_VPRINTF_1 format;
   HAVE_PROT_VPRINTF_2 ap;
{
   const int d = fileno(stdout);
   HAVE_PROT_VFPRINTF_0 rc;
   typedef HAVE_PROT_VPRINTF_0 (*VPRINTF_FUNC_T)(HAVE_PROT_VPRINTF_1,
                                                 HAVE_PROT_VPRINTF_2);
   VPRINTF_FUNC_T function = (VPRINTF_FUNC_T)symbolfunction(SYMBOL_VPRINTF);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(format, ap);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FPRINTF_0
sys_fprintf(HAVE_PROT_FPRINTF_1 stream, HAVE_PROT_FPRINTF_2 format, ...)
{
   va_list ap;
   HAVE_PROT_FPRINTF_0 rc;

   va_start(ap, format);
   rc = sys_vfprintf(stream, format, ap);
   va_end(ap);
   return rc;
}

HAVE_PROT_VFPRINTF_0
sys_vfprintf(stream, format, ap)
   HAVE_PROT_VFPRINTF_1 stream;
   HAVE_PROT_VFPRINTF_2 format;
   HAVE_PROT_VFPRINTF_3 ap;
{
   HAVE_PROT_VFPRINTF_0 rc;
   int d = fileno(stream);
   typedef HAVE_PROT_VFPRINTF_0 (*VFPRINTF_FUNC_T)(HAVE_PROT_VFPRINTF_1,
                                                   HAVE_PROT_VFPRINTF_2,
                                                   HAVE_PROT_VFPRINTF_3);
   VFPRINTF_FUNC_T function = (VFPRINTF_FUNC_T)symbolfunction(SYMBOL_VFPRINTF);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(stream, format, ap);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FWRITE_0
sys_fwrite(ptr, size, nmb, stream)
   HAVE_PROT_FWRITE_1 ptr;
   HAVE_PROT_FWRITE_2 size;
   HAVE_PROT_FWRITE_3 nmb;
   HAVE_PROT_FWRITE_4 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FWRITE_0 rc;
   typedef HAVE_PROT_FWRITE_0 (*FWRITE_FUNC_T)(HAVE_PROT_FWRITE_1,
                                               HAVE_PROT_FWRITE_2,
                                               HAVE_PROT_FWRITE_3,
                                               HAVE_PROT_FWRITE_4);
   FWRITE_FUNC_T function = (FWRITE_FUNC_T)symbolfunction(SYMBOL_FWRITE);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(ptr, size, nmb, stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_FREAD_0
sys_fread(ptr, size, nmb, stream)
   HAVE_PROT_FREAD_1 ptr;
   HAVE_PROT_FREAD_2 size;
   HAVE_PROT_FREAD_3 nmb;
   HAVE_PROT_FREAD_4 stream;
{
   const int d = fileno(stream);
   HAVE_PROT_FREAD_0 rc;
   typedef HAVE_PROT_FREAD_0 (*FREAD_FUNC_T)(HAVE_PROT_FREAD_1,
                                             HAVE_PROT_FREAD_2,
                                             HAVE_PROT_FREAD_3,
                                             HAVE_PROT_FREAD_4);
   FREAD_FUNC_T function = (FREAD_FUNC_T)symbolfunction(SYMBOL_FREAD);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(ptr, size, nmb, stream);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}
#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */

   /*
    * the interpositioned functions.
    */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_ACCEPT_0
accept(s, addr, addrlen)
   HAVE_PROT_ACCEPT_1 s;
   HAVE_PROT_ACCEPT_2 addr;
   HAVE_PROT_ACCEPT_3 addrlen;
{
   if (socks_issyscall(s, SYMBOL_ACCEPT))
      return sys_accept(s, addr, addrlen);
   return Raccept(s, addr, (socklen_t *)addrlen);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_BIND_0
bind(s, name, namelen)
   HAVE_PROT_BIND_1 s;
   HAVE_PROT_BIND_2 name;
   HAVE_PROT_BIND_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_BIND))
      return sys_bind(s, name, namelen);
   return Rbind(s, name, namelen);
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

int
bindresvport(sd, sin)
   int sd;
   struct sockaddr_in *sin;
{
   if (socks_issyscall(sd, SYMBOL_BINDRESVPORT))
      return sys_bindresvport(sd, sin);
   return Rbindresvport(sd, sin);
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_CONNECT_0
connect(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_CONNECT))
      return sys_connect(s, name, namelen);
   return Rconnect(s, name, namelen);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_GETPEERNAME_0
getpeername(s, name, namelen)
   HAVE_PROT_GETPEERNAME_1 s;
   HAVE_PROT_GETPEERNAME_2 name;
   HAVE_PROT_GETPEERNAME_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_GETPEERNAME))
      return sys_getpeername(s, name, namelen);
   return Rgetpeername(s, name, namelen);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_GETSOCKNAME_0
getsockname(s, name, namelen)
   HAVE_PROT_GETSOCKNAME_1 s;
   HAVE_PROT_GETSOCKNAME_2 name;
   HAVE_PROT_GETSOCKNAME_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_GETSOCKNAME))
      return sys_getsockname(s, name, namelen);
   return Rgetsockname(s, name, namelen);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_GETSOCKOPT_0
getsockopt(s, level, optname, optval, optlen)
   HAVE_PROT_GETSOCKOPT_1 s;
   HAVE_PROT_GETSOCKOPT_2 level;
   HAVE_PROT_GETSOCKOPT_3 optname;
   HAVE_PROT_GETSOCKOPT_4 optval;
   HAVE_PROT_GETSOCKOPT_5 optlen;
{
   if (socks_issyscall(s, SYMBOL_GETSOCKNAME))
      return sys_getsockopt(s, level, optname, optval, optlen);
   return Rgetsockopt(s, level, optname, optval, optlen);
}

HAVE_PROT_LISTEN_0
listen(s, backlog)
   HAVE_PROT_LISTEN_1 s;
   HAVE_PROT_LISTEN_2 backlog;
{
   if (socks_issyscall(s, SYMBOL_LISTEN))
      return sys_listen(s, backlog);
   return Rlisten(s, backlog);
}

HAVE_PROT_READ_0
read(d, buf, nbytes)
   HAVE_PROT_READ_1 d;
   HAVE_PROT_READ_2 buf;
   HAVE_PROT_READ_3 nbytes;
{
   if (socks_issyscall(d, SYMBOL_READ))
      return sys_read(d, buf, nbytes);
   return Rread(d, buf, nbytes);
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_READV_0
readv(d, iov, iovcnt)
   HAVE_PROT_READV_1 d;
   HAVE_PROT_READV_2 iov;
   HAVE_PROT_READV_3 iovcnt;
{
   if (socks_issyscall(d, SYMBOL_READV))
      return sys_readv(d, iov, iovcnt);
   return Rreadv(d, iov, iovcnt);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_RECV_0
recv(s, msg, len, flags)
   HAVE_PROT_RECV_1 s;
   HAVE_PROT_RECV_2 msg;
   HAVE_PROT_RECV_3 len;
   HAVE_PROT_RECV_4 flags;
{
   if (socks_issyscall(s, SYMBOL_RECV))
      return sys_recv(s, msg, len, flags);
   return Rrecv(s, msg, len, flags);
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_RECVFROM_0
recvfrom(s, buf, len, flags, from, fromlen)
   HAVE_PROT_RECVFROM_1 s;
   HAVE_PROT_RECVFROM_2 buf;
   HAVE_PROT_RECVFROM_3 len;
   HAVE_PROT_RECVFROM_4 flags;
   HAVE_PROT_RECVFROM_5 from;
   HAVE_PROT_RECVFROM_6 fromlen;
{
   if (socks_issyscall(s, SYMBOL_RECVFROM))
      return sys_recvfrom(s, buf, len, flags, from, fromlen);
   return Rrecvfrom(s, buf, len, flags, from, fromlen);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_RECVMSG_0
recvmsg(s, msg, flags)
   HAVE_PROT_RECVMSG_1 s;
   HAVE_PROT_RECVMSG_2 msg;
   HAVE_PROT_RECVMSG_3 flags;
{
   if (socks_issyscall(s, SYMBOL_RECVMSG))
      return sys_recvmsg(s, msg, flags);
   return Rrecvmsg(s, msg, flags);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_RRESVPORT
int
rresvport(port)
   int *port;
{
   return Rrresvport(port);
}
#endif /* HAVE_RRESVPORT */

HAVE_PROT_WRITE_0
write(d, buf, nbytes)
   HAVE_PROT_WRITE_1 d;
   HAVE_PROT_WRITE_2 buf;
   HAVE_PROT_WRITE_3 nbytes;
{
   if (socks_issyscall(d, SYMBOL_WRITE))
      return sys_write(d, buf, nbytes);
   return Rwrite(d, buf, nbytes);
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_WRITEV_0
writev(d, iov, iovcnt)
   HAVE_PROT_WRITEV_1 d;
   HAVE_PROT_WRITEV_2 iov;
   HAVE_PROT_WRITEV_3 iovcnt;
{
   if (socks_issyscall(d, SYMBOL_WRITEV))
      return sys_writev(d, iov, iovcnt);
   return Rwritev(d, iov, iovcnt);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_SEND_0
send(s, msg, len, flags)
   HAVE_PROT_SEND_1 s;
   HAVE_PROT_SEND_2 msg;
   HAVE_PROT_SEND_3 len;
   HAVE_PROT_SEND_4 flags;
{
   if (socks_issyscall(s, SYMBOL_SEND))
      return sys_send(s, msg, len, flags);
   return Rsend(s, msg, len, flags);
}

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_SENDMSG_0
sendmsg(s, msg, flags)
   HAVE_PROT_SENDMSG_1 s;
   HAVE_PROT_SENDMSG_2 msg;
   HAVE_PROT_SENDMSG_3 flags;
{
   if (socks_issyscall(s, SYMBOL_SENDMSG))
      return sys_sendmsg(s, msg, flags);
   return Rsendmsg(s, msg, flags);
}
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if !HAVE_EXTRA_OSF_SYMBOLS
HAVE_PROT_SENDTO_0
sendto(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   if (socks_issyscall(s, SYMBOL_SENDTO))
      return sys_sendto(s, msg, len, flags, to, tolen);
   return Rsendto(s, msg, len, flags, to, tolen);
}
#endif /* !HAVE_EXTRA_OSF_SYMBOLS */

#ifdef __sun
/* __xnet_foo variants of some functions exist on Solaris if _XPG4_2 is set */

HAVE_PROT_BIND_0
sys_xnet_bind(s, name, namelen)
   HAVE_PROT_BIND_1 s;
   HAVE_PROT_BIND_2 name;
   HAVE_PROT_BIND_3 namelen;
{
   int rc;
   typedef HAVE_PROT_BIND_0 (*BIND_FUNC_T)(HAVE_PROT_BIND_1,
                                           HAVE_PROT_BIND_2,
                                           HAVE_PROT_BIND_3);
   BIND_FUNC_T function = (BIND_FUNC_T)symbolfunction(SYMBOL_XNET_BIND);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_BIND_0
__xnet_bind(s, name, namelen)
   HAVE_PROT_BIND_1 s;
   HAVE_PROT_BIND_2 name;
   HAVE_PROT_BIND_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_XNET_BIND))
      return sys_xnet_bind(s, name, namelen);
   return Rbind(s, name, namelen);
}

HAVE_PROT_CONNECT_0
sys_xnet_connect(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   int rc;
   typedef HAVE_PROT_CONNECT_0 (*CONNECT_FUNC_T)(HAVE_PROT_CONNECT_1,
                                                 HAVE_PROT_CONNECT_2,
                                                 HAVE_PROT_CONNECT_3);
   CONNECT_FUNC_T function
   = (CONNECT_FUNC_T)symbolfunction(SYMBOL_XNET_CONNECT);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_CONNECT_0
__xnet_connect(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_XNET_CONNECT))
      return sys_xnet_connect(s, name, namelen);
   return Rconnect(s, name, namelen);
}

HAVE_PROT_LISTEN_0
sys_xnet_listen(s, backlog)
   HAVE_PROT_LISTEN_1 s;
   HAVE_PROT_LISTEN_2 backlog;
{
   HAVE_PROT_LISTEN_0 rc;
   typedef HAVE_PROT_LISTEN_0 (*LISTEN_FUNC_T)(HAVE_PROT_LISTEN_1,
                                           HAVE_PROT_LISTEN_2);
   LISTEN_FUNC_T function = (LISTEN_FUNC_T)symbolfunction(SYMBOL_XNET_LISTEN);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, backlog);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_LISTEN_0
__xnet_listen(s, backlog)
   HAVE_PROT_LISTEN_1 s;
   HAVE_PROT_LISTEN_2 backlog;
{
   if (socks_issyscall(s, SYMBOL_XNET_LISTEN))
      return sys_xnet_listen(s, backlog);
   return Rlisten(s, backlog);
}

HAVE_PROT_RECVMSG_0
sys_xnet_recvmsg(s, msg, flags)
   HAVE_PROT_RECVMSG_1 s;
   HAVE_PROT_RECVMSG_2 msg;
   HAVE_PROT_RECVMSG_3 flags;
{
   ssize_t rc;
   typedef HAVE_PROT_RECVMSG_0 (*RECVMSG_FUNC_T)(HAVE_PROT_RECVMSG_1,
                                                 HAVE_PROT_RECVMSG_2,
                                                 HAVE_PROT_RECVMSG_3);
   RECVMSG_FUNC_T function
   = (RECVMSG_FUNC_T)symbolfunction(SYMBOL_XNET_RECVMSG);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, flags);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_RECVMSG_0
__xnet_recvmsg(s, msg, flags)
   HAVE_PROT_RECVMSG_1 s;
   HAVE_PROT_RECVMSG_2 msg;
   HAVE_PROT_RECVMSG_3 flags;
{
   if (socks_issyscall(s, SYMBOL_XNET_RECVMSG))
      return sys_xnet_recvmsg(s, msg, flags);
   return Rrecvmsg(s, msg, flags);
}

HAVE_PROT_SENDMSG_0
sys_xnet_sendmsg(s, msg, flags)
   HAVE_PROT_SENDMSG_1 s;
   HAVE_PROT_SENDMSG_2 msg;
   HAVE_PROT_SENDMSG_3 flags;
{
   ssize_t rc;
   typedef HAVE_PROT_SENDMSG_0 (*SENDMSG_FUNC_T)(HAVE_PROT_SENDMSG_1,
                                                 HAVE_PROT_SENDMSG_2,
                                                 HAVE_PROT_SENDMSG_3);
   SENDMSG_FUNC_T function
   = (SENDMSG_FUNC_T)symbolfunction(SYMBOL_XNET_SENDMSG);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, flags);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_SENDMSG_0
__xnet_sendmsg(s, msg, flags)
   HAVE_PROT_SENDMSG_1 s;
   HAVE_PROT_SENDMSG_2 msg;
   HAVE_PROT_SENDMSG_3 flags;
{
   if (socks_issyscall(s, SYMBOL_XNET_SENDMSG))
      return sys_xnet_sendmsg(s, msg, flags);
   return Rsendmsg(s, msg, flags);
}

HAVE_PROT_SENDTO_0
sys_xnet_sendto(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   ssize_t rc;
   typedef HAVE_PROT_SENDTO_0 (*SENDTO_FUNC_T)(HAVE_PROT_SENDTO_1,
                                               HAVE_PROT_SENDTO_2,
                                               HAVE_PROT_SENDTO_3,
                                               HAVE_PROT_SENDTO_4,
                                               HAVE_PROT_SENDTO_5,
                                               HAVE_PROT_SENDTO_6);
   SENDTO_FUNC_T function = (SENDTO_FUNC_T)symbolfunction(SYMBOL_XNET_SENDTO);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, len, flags, to, tolen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_SENDTO_0
__xnet_sendto(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   if (socks_issyscall(s, SYMBOL_XNET_SENDTO))
      return sys_xnet_sendto(s, msg, len, flags, to, tolen);
   return Rsendto(s, msg, len, flags, to, tolen);
}

#endif /* __sun */

#ifdef __FreeBSD__
HAVE_PROT_ACCEPT_0
_accept(s, addr, addrlen)
   HAVE_PROT_ACCEPT_1 s;
   HAVE_PROT_ACCEPT_2 addr;
   HAVE_PROT_ACCEPT_3 addrlen;
{
   if (socks_issyscall(s, SYMBOL_ACCEPT))
      return sys_accept(s, addr, addrlen);
   return Raccept(s, addr, (socklen_t *)addrlen);
}

HAVE_PROT_BIND_0
_bind(s, name, namelen)
   HAVE_PROT_BIND_1 s;
   HAVE_PROT_BIND_2 name;
   HAVE_PROT_BIND_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_BIND))
      return sys_bind(s, name, namelen);
   return Rbind(s, name, namelen);
}

HAVE_PROT_CONNECT_0
_connect(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_CONNECT))
      return sys_connect(s, name, namelen);
   return Rconnect(s, name, namelen);
}

HAVE_PROT_GETPEERNAME_0
_getpeername(s, name, namelen)
   HAVE_PROT_GETPEERNAME_1 s;
   HAVE_PROT_GETPEERNAME_2 name;
   HAVE_PROT_GETPEERNAME_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_GETPEERNAME))
      return sys_getpeername(s, name, namelen);
   return Rgetpeername(s, name, namelen);
}

HAVE_PROT_GETSOCKNAME_0
_getsockname(s, name, namelen)
   HAVE_PROT_GETSOCKNAME_1 s;
   HAVE_PROT_GETSOCKNAME_2 name;
   HAVE_PROT_GETSOCKNAME_3 namelen;
{
   if (socks_issyscall(s, SYMBOL_GETSOCKNAME))
      return sys_getsockname(s, name, namelen);
   return Rgetsockname(s, name, namelen);
}

HAVE_PROT_LISTEN_0
_listen(s, backlog)
   HAVE_PROT_LISTEN_1 s;
   HAVE_PROT_LISTEN_2 backlog;
{
   if (socks_issyscall(s, SYMBOL_LISTEN))
      return sys_listen(s, backlog);
   return Rlisten(s, backlog);
}

HAVE_PROT_READ_0
_read(d, buf, nbytes)
   HAVE_PROT_READ_1 d;
   HAVE_PROT_READ_2 buf;
   HAVE_PROT_READ_3 nbytes;
{
   if (socks_issyscall(d, SYMBOL_READ))
      return sys_read(d, buf, nbytes);
   return Rread(d, buf, nbytes);
}

HAVE_PROT_READV_0
_readv(d, iov, iovcnt)
   HAVE_PROT_READV_1 d;
   HAVE_PROT_READV_2 iov;
   HAVE_PROT_READV_3 iovcnt;
{
   if (socks_issyscall(d, SYMBOL_READV))
      return sys_readv(d, iov, iovcnt);
   return Rreadv(d, iov, iovcnt);
}

HAVE_PROT_RECV_0
_recv(s, msg, len, flags)
   HAVE_PROT_RECV_1 s;
   HAVE_PROT_RECV_2 msg;
   HAVE_PROT_RECV_3 len;
   HAVE_PROT_RECV_4 flags;
{
   if (socks_issyscall(s, SYMBOL_RECV))
      return sys_recv(s, msg, len, flags);
   return Rrecv(s, msg, len, flags);
}

HAVE_PROT_RECVFROM_0
_recvfrom(s, buf, len, flags, from, fromlen)
   HAVE_PROT_RECVFROM_1 s;
   HAVE_PROT_RECVFROM_2 buf;
   HAVE_PROT_RECVFROM_3 len;
   HAVE_PROT_RECVFROM_4 flags;
   HAVE_PROT_RECVFROM_5 from;
   HAVE_PROT_RECVFROM_6 fromlen;
{
   if (socks_issyscall(s, SYMBOL_RECVFROM))
      return sys_recvfrom(s, buf, len, flags, from, fromlen);
   return Rrecvfrom(s, buf, len, flags, from, fromlen);
}

HAVE_PROT_RECVMSG_0
_recvmsg(s, msg, flags)
   HAVE_PROT_RECVMSG_1 s;
   HAVE_PROT_RECVMSG_2 msg;
   HAVE_PROT_RECVMSG_3 flags;
{
   if (socks_issyscall(s, SYMBOL_RECVMSG))
      return sys_recvmsg(s, msg, flags);
   return Rrecvmsg(s, msg, flags);
}

HAVE_PROT_WRITE_0
_write(d, buf, nbytes)
   HAVE_PROT_WRITE_1 d;
   HAVE_PROT_WRITE_2 buf;
   HAVE_PROT_WRITE_3 nbytes;
{
   if (socks_issyscall(d, SYMBOL_WRITE))
      return sys_write(d, buf, nbytes);
   return Rwrite(d, buf, nbytes);
}

HAVE_PROT_WRITEV_0
_writev(d, iov, iovcnt)
   HAVE_PROT_WRITEV_1 d;
   HAVE_PROT_WRITEV_2 iov;
   HAVE_PROT_WRITEV_3 iovcnt;
{
   if (socks_issyscall(d, SYMBOL_WRITEV))
      return sys_writev(d, iov, iovcnt);
   return Rwritev(d, iov, iovcnt);
}

HAVE_PROT_SEND_0
_send(s, msg, len, flags)
   HAVE_PROT_SEND_1 s;
   HAVE_PROT_SEND_2 msg;
   HAVE_PROT_SEND_3 len;
   HAVE_PROT_SEND_4 flags;
{
   if (socks_issyscall(s, SYMBOL_SEND))
      return sys_send(s, msg, len, flags);
   return Rsend(s, msg, len, flags);
}

HAVE_PROT_SENDMSG_0
_sendmsg(s, msg, flags)
   HAVE_PROT_SENDMSG_1 s;
   HAVE_PROT_SENDMSG_2 msg;
   HAVE_PROT_SENDMSG_3 flags;
{
   if (socks_issyscall(s, SYMBOL_SENDMSG))
      return sys_sendmsg(s, msg, flags);
   return Rsendmsg(s, msg, flags);
}

HAVE_PROT_SENDTO_0
_sendto(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   if (socks_issyscall(s, SYMBOL_SENDTO))
      return sys_sendto(s, msg, len, flags, to, tolen);
   return Rsendto(s, msg, len, flags, to, tolen);
}
#endif /* __FreeBSD__ */


#else /* !SOCKS_CLIENT */

/*
 * For a few functions interposed into external libraries (libwrap, ldap, etc.)
 * used by the server.
 */

struct hostent *
sys_gethostbyaddr(addr, len, af)
   const char *addr;
   socklen_t len;
   int af;
{
   typedef struct hostent *(*GETHOSTBYADDR_FUNC_T)(const char *, int, int);
   GETHOSTBYADDR_FUNC_T function
   = (GETHOSTBYADDR_FUNC_T)symbolfunction(SYMBOL_GETHOSTBYADDR);

   return function(addr, len, af);
}

HAVE_PROT_GETHOSTBYADDR_0
gethostbyaddr(addr, len, af)
   HAVE_PROT_GETHOSTBYADDR_1 addr;
   HAVE_PROT_GETHOSTBYADDR_2 len;
   HAVE_PROT_GETHOSTBYADDR_3 af;
{

   if (socks_shouldcallasnative(SYMBOL_GETHOSTBYADDR))
      return sys_gethostbyaddr(addr, len, af);
   return cgethostbyaddr(addr, len, af);
}

#endif /* !SOCKS_CLIENT */

struct hostent *
sys_gethostbyname(name)
   const char *name;
{
   struct hostent *rc;
   typedef struct hostent *(*GETHOSTBYNAME_FUNC_T)(const char *);
   GETHOSTBYNAME_FUNC_T function
   = (GETHOSTBYNAME_FUNC_T)symbolfunction(SYMBOL_GETHOSTBYNAME);

   DNSCODE_START();
   rc = function(name);
   DNSCODE_END();

   return rc;
}

struct hostent *
gethostbyname(name)
   const char *name;
{
#if !SOCKS_CLIENT
   return cgethostbyname(name);

#else /* SOCKS_CLIENT */
   if (socks_shouldcallasnative(SYMBOL_GETHOSTBYNAME)) {
      struct hostent *rc;

      DNSCODE_START();
      rc = sys_gethostbyname(name);
      DNSCODE_END();

      return rc;
   }

   return Rgethostbyname(name);
#endif /* SOCKS_CLIENT */
}

#if SOCKS_CLIENT

#if HAVE_GETADDRINFO
int
sys_getaddrinfo(nodename, servname, hints, res)
   const char *nodename;
   const char *servname;
   const struct addrinfo *hints;
   struct addrinfo **res;
{
   typedef int (*GETADDRINFO_FUNC_T)(const char *, const char *,
                 const struct addrinfo *,
                 struct addrinfo **);
   GETADDRINFO_FUNC_T function
   = (GETADDRINFO_FUNC_T)symbolfunction(SYMBOL_GETADDRINFO);
   int rc;

   DNSCODE_START();
   rc = function(nodename, servname, hints, res);
   DNSCODE_END();

   return rc;
}

int
getaddrinfo(nodename, servname, hints, res)
   const char *nodename;
   const char *servname;
   const struct addrinfo *hints;
   struct addrinfo **res;
{
   int rc;

   if (socks_shouldcallasnative(SYMBOL_GETADDRINFO)) {
      DNSCODE_START();
      rc = sys_getaddrinfo(nodename, servname, hints, res);
      DNSCODE_END();

      return rc;
   }

   return Rgetaddrinfo(nodename, servname, hints, res);
}

#endif /* HAVE_GETADDRINFO */

#if HAVE_GETNAMEINFO

HAVE_PROT_GETNAMEINFO_0
sys_getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)
   HAVE_PROT_GETNAMEINFO_1 sa;
   HAVE_PROT_GETNAMEINFO_2 salen;
   HAVE_PROT_GETNAMEINFO_3 host;
   HAVE_PROT_GETNAMEINFO_4 hostlen;
   HAVE_PROT_GETNAMEINFO_5 serv;
   HAVE_PROT_GETNAMEINFO_6 servlen;
   HAVE_PROT_GETNAMEINFO_7 flags;
{
   typedef HAVE_PROT_GETNAMEINFO_0 (*GETNAMEINFO_FUNC_T)(
                                    HAVE_PROT_GETNAMEINFO_1,
                                    HAVE_PROT_GETNAMEINFO_2,
                                    HAVE_PROT_GETNAMEINFO_3,
                                    HAVE_PROT_GETNAMEINFO_4,
                                    HAVE_PROT_GETNAMEINFO_5,
                                    HAVE_PROT_GETNAMEINFO_6,
                                    HAVE_PROT_GETNAMEINFO_7);
   GETNAMEINFO_FUNC_T function
      = (GETNAMEINFO_FUNC_T)symbolfunction(SYMBOL_GETNAMEINFO);
   int rc;

   DNSCODE_START();
   rc = function(sa, salen, host, hostlen, serv, servlen, flags);
   DNSCODE_END();

   return rc;
}

#endif /* HAVE_GETNAMEINFO */


struct hostent *
sys_gethostbyname2(name, af)
   const char *name;
   int af;
{
   struct hostent *rc;
   typedef struct hostent *(*GETHOSTBYNAME2_FUNC_T)(const char *, int);
   GETHOSTBYNAME2_FUNC_T function
   = (GETHOSTBYNAME2_FUNC_T)symbolfunction(SYMBOL_GETHOSTBYNAME2);

   DNSCODE_START();
   rc = function(name, af);
   DNSCODE_END();

   return rc;
}

struct hostent *
gethostbyname2(name, af)
   const char *name;
   int af;
{
   if (socks_shouldcallasnative(SYMBOL_GETHOSTBYNAME2)) {
      struct hostent *rc;

      DNSCODE_START();
      rc = sys_gethostbyname2(name, af);
      DNSCODE_END();

      return rc;
   }

   return Rgethostbyname2(name, af);
}

#if HAVE_GETIPNODEBYNAME

struct hostent *
sys_getipnodebyname(name, af, flags, error_num)
   const char *name;
   int af;
   int flags;
   int *error_num;
{
   struct hostent *rc;
   typedef struct hostent *(*GETIPNODEBYNAME_FUNC_T)(const char *, int, int,
                                                     int *);
   GETIPNODEBYNAME_FUNC_T function
   = (GETIPNODEBYNAME_FUNC_T)symbolfunction(SYMBOL_GETIPNODEBYNAME);

   DNSCODE_START();
   rc = function(name, af, flags, error_num);
   DNSCODE_END();

   return rc;

}

struct hostent *
getipnodebyname(name, af, flags, error_num)
   const char *name;
   int af;
   int flags;
   int *error_num;
{

   if (socks_shouldcallasnative(SYMBOL_GETIPNODEBYNAME)) {
      struct hostent *rc;

      DNSCODE_START();
      rc = sys_getipnodebyname(name, af, flags, error_num);
      DNSCODE_END();

      return rc;
   }

   return Rgetipnodebyname(name, af, flags, error_num);
}

void
sys_freehostent(ptr)
   struct hostent *ptr;
{
  typedef struct hostent *(*FREEHOSTENT_FUNC_T)(struct hostent *);

   FREEHOSTENT_FUNC_T function
   = (FREEHOSTENT_FUNC_T)symbolfunction(SYMBOL_FREEHOSTENT);

   function(ptr);
}

void
freehostent(ptr)
   struct hostent *ptr;
{

   if (socks_shouldcallasnative(SYMBOL_FREEHOSTENT))
      sys_freehostent(ptr);
   else
      Rfreehostent(ptr);
}

#endif /* HAVE_GETIPNODEBYNAME */

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND

#ifdef getc
#undef getc
#endif /* getc */

HAVE_PROT_GETC_0
getc(stream)
   HAVE_PROT_GETC_1 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_GETC))
      return sys_getc(stream);
   return Rfgetc(stream);
}

#if HAVE__IO_GETC
HAVE_PROT__IO_GETC_0
_IO_getc(stream)
   HAVE_PROT__IO_GETC_1 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL__IO_GETC))
      return sys_getc(stream);
   return Rfgetc(stream);
}
#endif /* HAVE__IO_GETC */

HAVE_PROT_FGETC_0
fgetc(stream)
   HAVE_PROT_FGETC_1 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FGETC))
      return sys_getc(stream);
   return Rfgetc(stream);
}

HAVE_PROT_GETS_0
gets(buf)
   HAVE_PROT_GETS_1 buf;
{
   const int d = fileno(stdin);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_GETS))
      return sys_gets(buf);
   return Rgets(buf);
}

HAVE_PROT_FGETS_0
fgets(buf, size, stream)
   HAVE_PROT_FGETS_1 buf;
   HAVE_PROT_FGETS_2 size;
   HAVE_PROT_FGETS_3 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FGETS))
      return sys_fgets(buf, size, stream);
   return Rfgets(buf, size, stream);
}

HAVE_PROT_PUTC_0
putc(c, stream)
   HAVE_PROT_PUTC_1 c;
   HAVE_PROT_PUTC_2 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_PUTC))
      return sys_putc(c, stream);
   return Rfputc(c, stream);
}

#if HAVE__IO_PUTC
HAVE_PROT__IO_PUTC_0
_IO_putc(c, stream)
   HAVE_PROT__IO_PUTC_1 c;
   HAVE_PROT__IO_PUTC_2 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL__IO_PUTC))
      return sys_putc(c, stream);
   return Rfputc(c, stream);
}
#endif /* HAVE__IO_PUTC */

HAVE_PROT_FPUTC_0
fputc(c, stream)
   HAVE_PROT_FPUTC_1 c;
   HAVE_PROT_FPUTC_2 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FPUTC))
      return sys_fputc(c, stream);
   return Rfputc(c, stream);
}

HAVE_PROT_PUTS_0
puts(buf)
   HAVE_PROT_PUTS_1 buf;
{
   const int d = fileno(stdout);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_PUTS))
      return sys_puts(buf);
   return Rfputs(buf, stdout);
}

HAVE_PROT_FPUTS_0
fputs(buf, stream)
   HAVE_PROT_FPUTS_1 buf;
   HAVE_PROT_FPUTS_2 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FPUTS))
      return sys_fputs(buf, stream);
   return Rfputs(buf, stream);
}

HAVE_PROT_FFLUSH_0
fflush(stream)
   HAVE_PROT_FFLUSH_1 stream;
{

   if (!sockscf.state.havegssapisockets
   ||  stream == NULL
   ||  socks_issyscall(fileno(stream), SYMBOL_FFLUSH))
      return sys_fflush(stream);
   return Rfflush(stream);
}

HAVE_PROT_FCLOSE_0
fclose(stream)
   HAVE_PROT_FCLOSE_1 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FCLOSE))
      return sys_fclose(stream);
   return Rfclose(stream);
}

HAVE_PROT_PRINTF_0
printf(HAVE_PROT_PRINTF_1 format, ...)
{
   const int d = fileno(stdout);
   va_list ap;
   int rc;

   va_start(ap, format);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_PRINTF)) {
      rc = sys_vprintf(format, ap);
      va_end(ap);
      return rc;
   }

   rc = Rvfprintf(stdout, format, ap);
   va_end(ap);

   return rc;
}

HAVE_PROT_FPRINTF_0
fprintf(HAVE_PROT_FPRINTF_1 stream, HAVE_PROT_FPRINTF_2 format, ...)
{
   const int d = fileno(stream);
   va_list ap;
   int rc;

   va_start(ap, format);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FPRINTF)) {
      rc = sys_vfprintf(stream, format, ap);
      va_end(ap);
      return rc;
   }

   rc = Rvfprintf(stream, format, ap);
   va_end(ap);
   return rc;
}

#if HAVE___FPRINTF_CHK
HAVE_PROT_FPRINTF_0
__fprintf_chk(HAVE_PROT_FPRINTF_1 stream, int dummy,
              HAVE_PROT_FPRINTF_2 format, ...)
{
   const int d = fileno(stream);
   va_list ap;
   int rc;

   va_start(ap, format);

   if (!sockscf.state.havegssapisockets
   || socks_issyscall(d, SYMBOL___FPRINTF_CHK)) {
      rc = sys_vfprintf(stream, format, ap);
      va_end(ap);
      return rc;
   }

   rc = Rvfprintf(stream, format, ap);
   va_end(ap);
   return rc;
}
#endif /* HAVE___FPRINTF_CHK */

HAVE_PROT_VPRINTF_0
vprintf(format, ap)
   HAVE_PROT_VPRINTF_1 format;
   HAVE_PROT_VPRINTF_2 ap;
{
   const int d = fileno(stdout);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_VPRINTF))
      return sys_vprintf(format, ap);
   return Rvfprintf(stdout, format, ap);
}

HAVE_PROT_VFPRINTF_0
vfprintf(stream, format, ap)
   HAVE_PROT_VFPRINTF_1 stream;
   HAVE_PROT_VFPRINTF_2 format;
   HAVE_PROT_VFPRINTF_3 ap;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_VFPRINTF))
      return sys_vfprintf(stream, format, ap);
   return Rvfprintf(stream, format, ap);
}

#if HAVE___VFPRINTF_CHK
HAVE_PROT_VFPRINTF_0
__vfprintf_chk(stream, dummy, format, ap)
   HAVE_PROT_VFPRINTF_1 stream;
   int                  dummy;
   HAVE_PROT_VFPRINTF_2 format;
   HAVE_PROT_VFPRINTF_3 ap;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets
   || socks_issyscall(d, SYMBOL___VFPRINTF_CHK))
      return sys_vfprintf(stream, format, ap);
   return Rvfprintf(stream, format, ap);
}
#endif /* HAVE___VFPRINTF_CHK */

HAVE_PROT_FWRITE_0
fwrite(ptr, size, nmb, stream)
   HAVE_PROT_FWRITE_1 ptr;
   HAVE_PROT_FWRITE_2 size;
   HAVE_PROT_FWRITE_3 nmb;
   HAVE_PROT_FWRITE_4 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FWRITE))
      return sys_fwrite(ptr, size, nmb, stream);
   return Rfwrite(ptr, size, nmb, stream);
}

HAVE_PROT_FREAD_0
fread(ptr, size, nmb, stream)
   HAVE_PROT_FREAD_1 ptr;
   HAVE_PROT_FREAD_2 size;
   HAVE_PROT_FREAD_3 nmb;
   HAVE_PROT_FREAD_4 stream;
{
   const int d = fileno(stream);

   if (!sockscf.state.havegssapisockets || socks_issyscall(d, SYMBOL_FREAD))
      return sys_fread(ptr, size, nmb, stream);
   return Rfread(ptr, size, nmb, stream);
}

#if HAVE___READ_CHK
HAVE_PROT__READ_CHK_0
__read_chk(d, buf, nbytes, buflen)
   HAVE_PROT__READ_CHK_1 d;
   HAVE_PROT__READ_CHK_2 buf;
   HAVE_PROT__READ_CHK_3 nbytes;
   HAVE_PROT__READ_CHK_4 buflen;
{
   SASSERTX(nbytes <= buflen);

   if (!sockscf.state.havegssapisockets ||
       socks_issyscall(d, SYMBOL___READ_CHK))
      return sys_read(d, buf, nbytes);

   return Rread(d, buf, nbytes);
}
#endif /* HAVE___READ_CHK */

#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */

#if HAVE_DARWIN

HAVE_PROT_READ_0
sys_read_nocancel(d, buf, nbytes)
   HAVE_PROT_READ_1 d;
   HAVE_PROT_READ_2 buf;
   HAVE_PROT_READ_3 nbytes;
{
   ssize_t rc;
   typedef HAVE_PROT_READ_0 (*READ_FUNC_T)(HAVE_PROT_READ_1,
                                           HAVE_PROT_READ_2,
                                           HAVE_PROT_READ_3);
   READ_FUNC_T function = (READ_FUNC_T)symbolfunction(SYMBOL_READ_NOCANCEL);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(d);

   rc = function(d, buf, nbytes);

   if (tagged)
      socks_syscall_end(d);

   return rc;
}

HAVE_PROT_CONNECT_0
sys_connect_nocancel(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   int rc;
   typedef HAVE_PROT_CONNECT_0 (*CONNECT_FUNC_T)(HAVE_PROT_CONNECT_1,
                                                 HAVE_PROT_CONNECT_2,
                                                 HAVE_PROT_CONNECT_3);
   CONNECT_FUNC_T function =
      (CONNECT_FUNC_T)symbolfunction(SYMBOL_CONNECT_NOCANCEL);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, name, namelen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_RECVFROM_0
sys_recvfrom_nocancel(s, buf, len, flags, from, fromlen)
   HAVE_PROT_RECVFROM_1 s;
   HAVE_PROT_RECVFROM_2 buf;
   HAVE_PROT_RECVFROM_3 len;
   HAVE_PROT_RECVFROM_4 flags;
   HAVE_PROT_RECVFROM_5 from;
   HAVE_PROT_RECVFROM_6 fromlen;
{
   int rc;
   typedef HAVE_PROT_RECVFROM_0 (*RECVFROM_FUNC_T)(HAVE_PROT_RECVFROM_1,
                                                   HAVE_PROT_RECVFROM_2,
                                                   HAVE_PROT_RECVFROM_3,
                                                   HAVE_PROT_RECVFROM_4,
                                                   HAVE_PROT_RECVFROM_5,
                                                   HAVE_PROT_RECVFROM_6);
   RECVFROM_FUNC_T function =
      (RECVFROM_FUNC_T)symbolfunction(SYMBOL_RECVFROM_NOCANCEL);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, buf, len, flags, from, fromlen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_SENDTO_0
sys_sendto_nocancel(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   ssize_t rc;
   typedef HAVE_PROT_SENDTO_0 (*SENDTO_FUNC_T)(HAVE_PROT_SENDTO_1,
                                               HAVE_PROT_SENDTO_2,
                                               HAVE_PROT_SENDTO_3,
                                               HAVE_PROT_SENDTO_4,
                                               HAVE_PROT_SENDTO_5,
                                               HAVE_PROT_SENDTO_6);
   SENDTO_FUNC_T function =
      (SENDTO_FUNC_T)symbolfunction(SYMBOL_SENDTO_NOCANCEL);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, msg, len, flags, to, tolen);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_WRITE_0
sys_write_nocancel(s, buf, nbytes)
   HAVE_PROT_WRITE_1 s;
   HAVE_PROT_WRITE_2 buf;
   HAVE_PROT_WRITE_3 nbytes;
{
   ssize_t rc;
   typedef HAVE_PROT_WRITE_0 (*WRITE_FUNC_T)(HAVE_PROT_WRITE_1,
                                             HAVE_PROT_WRITE_2,
                                             HAVE_PROT_WRITE_3);
   WRITE_FUNC_T function = (WRITE_FUNC_T)symbolfunction(SYMBOL_WRITE_NOCANCEL);
   const int tagged = (doing_addrinit ? 0 : 1);

   if (tagged)
      socks_syscall_start(s);

   rc = function(s, buf, nbytes);

   if (tagged)
      socks_syscall_end(s);

   return rc;
}

HAVE_PROT_CONNECT_0
connect$NOCANCEL(s, name, namelen)
   HAVE_PROT_CONNECT_1 s;
   HAVE_PROT_CONNECT_2 name;
   HAVE_PROT_CONNECT_3 namelen;
{
   if (!sockscf.state.havegssapisockets
   || socks_issyscall(s, SYMBOL_CONNECT_NOCANCEL))
      return sys_connect_nocancel(s, name, namelen);

   return Rconnect(s, name, namelen);
}

#if 1
HAVE_PROT_READ_0
read$NOCANCEL(d, buf, nbytes)
   HAVE_PROT_READ_1 d;
   HAVE_PROT_READ_2 buf;
   HAVE_PROT_READ_3 nbytes;
{
   if (!sockscf.state.havegssapisockets
   || socks_issyscall(d, SYMBOL_READ_NOCANCEL))
      return sys_read_nocancel(d, buf, nbytes);

   return Rread(d, buf, nbytes);
}
#endif

HAVE_PROT_RECVFROM_0
recvfrom$NOCANCEL(s, buf, len, flags, from, fromlen)
   HAVE_PROT_RECVFROM_1 s;
   HAVE_PROT_RECVFROM_2 buf;
   HAVE_PROT_RECVFROM_3 len;
   HAVE_PROT_RECVFROM_4 flags;
   HAVE_PROT_RECVFROM_5 from;
   HAVE_PROT_RECVFROM_6 fromlen;
{
   if (!sockscf.state.havegssapisockets
   || socks_issyscall(s, SYMBOL_RECVFROM_NOCANCEL))
      return sys_recvfrom_nocancel(s, buf, len, flags, from, fromlen);

   return Rrecvfrom(s, buf, len, flags, from, fromlen);
}

HAVE_PROT_SENDTO_0
sendto$NOCANCEL(s, msg, len, flags, to, tolen)
   HAVE_PROT_SENDTO_1 s;
   HAVE_PROT_SENDTO_2 msg;
   HAVE_PROT_SENDTO_3 len;
   HAVE_PROT_SENDTO_4 flags;
   HAVE_PROT_SENDTO_5 to;
   HAVE_PROT_SENDTO_6 tolen;
{
   if (!sockscf.state.havegssapisockets
   || socks_issyscall(s, SYMBOL_SENDTO_NOCANCEL))
      return sys_sendto_nocancel(s, msg, len, flags, to, tolen);

   return Rsendto(s, msg, len, flags, to, tolen);
}

#if 1
HAVE_PROT_WRITE_0
write$NOCANCEL(d, buf, nbytes)
   HAVE_PROT_WRITE_1 d;
   HAVE_PROT_WRITE_2 buf;
   HAVE_PROT_WRITE_3 nbytes;
{
   if (!sockscf.state.havegssapisockets
   || socks_issyscall(d, SYMBOL_WRITE_NOCANCEL))
      return sys_write_nocancel(d, buf, nbytes);

   return Rwrite(d, buf, nbytes);
}
#endif

#endif /* HAVE_DARWIN */

static int
idsareequal(a, b)
   const socks_id_t *a;
   const socks_id_t *b;
{

   switch (a->whichid) {
      case pid:
         if (a->id.pid == b->id.pid)
            return 1;

         return 0;

      case thread:
         /* pthread_equal() is more correct, but this should also work. */
         if (memcmp(&a->id.thread, &b->id.thread, sizeof(a->id.thread)) == 0)
            return 1;
         return 0;

      default:
         SERRX(a->whichid);
   }

   /* NOTREACHED */
}
#endif /* SOCKS_CLIENT */


static libsymbol_t *
libsymbol(symbol)
   const char *symbol;
{
/*   const char *function = "libsymbol()"; */
   size_t i;

   for (i = 0; i < ELEMENTS(libsymbolv); ++i)
      if (strcmp(libsymbolv[i].symbol, symbol) == 0)
         return &libsymbolv[i];

   SERRX(0);   /* should never happen. */

   /* NOTREACHED */
   return NULL; /* please compiler. */
}

#endif /* SOCKSLIBRARY_DYNAMIC */
