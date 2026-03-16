/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2008, 2009,
 *               2010, 2011, 2012, 2013, 2016, 2017, 2024
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

/* $Id: socks.h,v 1.287.6.6.8.4 2024/11/21 10:22:42 michaels Exp $ */

#ifndef _SOCKS_H_
#define _SOCKS_H_

#define HAVE_SOCKS_RULES                  (0)

#ifndef HAVE_OSF_OLDSTYLE
#define HAVE_OSF_OLDSTYLE 0
#endif /* !HAVE_OSF_OLDSTYLE */

#if SOCKSLIBRARY_DYNAMIC

#ifdef __COVERITY__
/*
 * Coverity naturally has no idea what the function sys_foo calls does,
 * so let it pretend sys_foo is the same as foo.
 * Means Coverity can't catch errors in the code around the call to
 * sys_foo(), but avoids dozens of false positives because Coverity has no
 * idea what the dlopen(3)-ed functions do.
 */
#define sys_accept accept
#define sys_bind bind
#define sys_bindresvport bindresvport
#define sys_connect connect
#define sys_gethostbyname gethostbyname
#define sys_gethostbyname2 gethostbyname2
#define sys_getaddrinfo getaddrinfo
#define sys_getipnodebyname getipnodebyname
#define sys_getpeername getpeername
#define sys_getsockname getsockname
#define sys_getsockopt getsockopt
#define sys_listen listen
#define sys_read read
#define sys_readv readv
#define sys_recv recv
#define sys_recvfrom recvfrom
#define sys_recvfrom recvfrom
#define sys_recvmsg recvmsg
#define sys_rresvport rresvport
#define sys_send send
#define sys_sendmsg sendmsg
#define sys_sendto sendto
#define sys_write write
#define sys_writev writev
#endif /* __COVERITY__ */


#if 0 /* XXX disable until testing on AIX/other can be done */

/* XXX needed on AIX apparently */
#ifdef recvmsg
#define recvmsg_system recvmsg
#undef recvmsg
#endif /* recvmsg */

#if HAVE_SYSTEM_XMSG_MAGIC
#undef recvmsg_system
#define recvmsg_system nrecvmsg
#endif /* HAVE_SYSTEM_XMSG_MAGIC */

#ifdef sendmsg
#define sendmsg_system sendmsg
#undef sendmsg
#endif /* sendmsg */

#if HAVE_SYSTEM_XMSG_MAGIC
#undef sendmsg_system
#define sendmsg_system nsendmsg
#endif /* HAVE_SYSTEM_XMSG_MAGIC */
#endif

#ifdef accept
#undef accept
#endif /* accept */
#if HAVE_EXTRA_OSF_SYMBOLS
#define accept(s, addr, addrlen)         sys_Eaccept(s, addr, addrlen)
#else
#define accept(s, addr, addrlen)         sys_accept(s, addr, addrlen)
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef bind
#undef bind
#endif /* bind */
#if (defined __sun) && (defined _XPG4_2)
#define bind(s, name, namelen)         sys_xnet_bind(s, name, namelen)
#else
#define bind(s, name, namelen)         sys_bind(s, name, namelen)
#endif

#ifdef bindresvport
#undef bindresvport
#endif /* bindresvport */
#define bindresvport(sd, sin)            sys_bindresvport(sd, sin)

#ifdef connect
#undef connect
#endif /* connect */
#if (defined __sun) && (defined _XPG4_2)
#define connect(s, name, namelen)      sys_xnet_connect(s, name, namelen)
#else
#define connect(s, name, namelen)      sys_connect(s, name, namelen)
#endif


#ifdef gethostbyname
#undef gethostbyname
#endif /* gethostbyname */

#if HAVE_GETHOSTBYNAME2

/*
 * a little tricky ... we need it to be at the bottom of the stack,
 * like a syscall.
 */
#define gethostbyname(name)            sys_gethostbyname2(name, AF_INET)
#else
#define gethostbyname(name)            sys_gethostbyname(name)

#endif /* HAVE_GETHOSTBYNAME2 */

#ifdef gethostbyname2
#undef gethostbyname2
#endif /* gethostbyname2 */
#define gethostbyname2(name, af)       sys_gethostbyname2(name, af)

#ifdef getaddrinfo
#undef getaddrinfo
#endif /* getaddrinfo */
#define getaddrinfo(nodename, servname, hints, res)   \
         sys_getaddrinfo(nodename, servname, hints, res)

#ifdef getipnodebyname
#undef getipnodebyname
#endif /* getipnodebyname */
#define getipnodebyname(name, af, flags, error_num)   \
         sys_getipnodebyname(name, af, flags, error_num)

#ifdef freehostent
#undef freehostent
#endif /* freehostent */
#define freehostent(ptr)            sys_freehostent(ptr)

#if HAVE_GETNAMEINFO

#define getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) \
         sys_getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)

#endif /* HAVE_GETNAMEINFO  */


#ifdef getpeername
#undef getpeername
#endif /* getpeername */

#if HAVE_EXTRA_OSF_SYMBOLS
#define getpeername(s, name, namelen)   sys_Egetpeername(s, name, namelen)
#else
#define getpeername(s, name, namelen)   sys_getpeername(s, name, namelen)
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef getsockname
#undef getsockname
#endif /* getsockname */
#if HAVE_EXTRA_OSF_SYMBOLS
#define getsockname(s, name, namelen)   sys_Egetsockname(s, name, namelen)
#else
#define getsockname(s, name, namelen)   sys_getsockname(s, name, namelen)
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef getsockopt
#undef getsockopt
#endif /* getsockopt */
#if HAVE_EXTRA_OSF_SYMBOLS
#define getsockopt(a, b, c, d, e) sys_Egetsockopt(a, b, c, d, e)
#else
#define getsockopt(a, b, c, d, e) sys_getsockopt(a, b, c, d, e)
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef listen
#undef listen
#endif /* listen */
#if (defined __sun) && (defined _XPG4_2)
#define listen(s, backlog)   sys_xnet_listen(s, backlog)
#else
#define listen(s, backlog)   sys_listen(s, backlog)
#endif

#ifdef read
#undef read
#endif /* read */
#define read(d, buf, nbytes)            sys_read(d, buf, nbytes)

#ifdef readv
#undef readv
#endif /* readv */
#if HAVE_EXTRA_OSF_SYMBOLS
#define readv(d, iov, iovcnt)            sys_Ereadv(d, iov, iovcnt)
#else
#define readv(d, iov, iovcnt)            sys_readv(d, iov, iovcnt)
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef recv
#undef recv
#endif /* recv */
#define recv(s, msg, len, flags)         sys_recv(s, msg, len, flags)

#ifdef recvfrom
#undef recvfrom
#endif /* recvfrom */
#if HAVE_EXTRA_OSF_SYMBOLS
#define recvfrom(s, buf, len, flags, from, fromlen)   \
        sys_Erecvfrom(s, buf, len, flags, from, fromlen)
#else
#define recvfrom(s, buf, len, flags, from, fromlen)   \
        sys_recvfrom(s, buf, len, flags, from, fromlen)

#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef recvmsg
#undef recvmsg
#endif /* recvmsg */
#if (defined __sun) && (defined _XPG4_2)
#define recvmsg(s, msg, flags)          sys_xnet_recvmsg(s, msg, flags)
#else
#define recvmsg(s, msg, flags)          sys_recvmsg(s, msg, flags)
#endif

#if HAVE_RRESVPORT
#ifdef rresvport
#undef rresvport
#endif /* rresvport */
#define rresvport(port)                  sys_rresvport(port)
#endif /* HAVE_RRESVPORT */

#ifdef sendto
#undef sendto
#endif /* sendto */
#if (defined __sun) && (defined _XPG4_2)
#define sendto(s, msg, len, flags, to, tolen)   \
        sys_xnet_sendto(s, msg, len, flags, to, tolen)
#else
#define sendto(s, msg, len, flags, to, tolen)   \
        sys_sendto(s, msg, len, flags, to, tolen)
#endif

#ifdef write
#undef write
#endif /* write */
#define write(d, buf, nbytes)            sys_write(d, buf, nbytes)

#ifdef writev
#undef writev
#endif /* writev */
#if HAVE_EXTRA_OSF_SYMBOLS
#define writev(d, iov, iovcnt)         sys_Ewritev(d, iov, iovcnt)
#else
#define writev(d, iov, iovcnt)         sys_writev(d, iov, iovcnt)
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef send
#undef send
#endif /* send */
#define send(s, msg, len, flags)         sys_send(s, msg, len, flags)

#undef sendmsg
#if HAVE_EXTRA_OSF_SYMBOLS
#define sendmsg(s, msg, flags)         sys_Esendmsg(s, msg, flags)
#else
#if (defined __sun) && (defined _XPG4_2)
#define sendmsg(s, msg, flags)         sys_xnet_sendmsg(s, msg, flags)
#else
#define sendmsg(s, msg, flags)         sys_sendmsg(s, msg, flags)
#endif
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#undef recvmsg
#if HAVE_EXTRA_OSF_SYMBOLS
#define recvmsg(s, msg, flags)         sys_Erecvmsg(s, msg, flags)
#else
#if (defined __sun) && (defined _XPG4_2)
#define recvmsg(s, msg, flags)         sys_xnet_recvmsg(s, msg, flags)
#else
#define recvmsg(s, msg, flags)         sys_recvmsg(s, msg, flags)
#endif
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
#ifdef getc
#undef getc
#endif /* getc */
#define getc(s)                           sys_getc(s)

#ifdef fgetc
#undef fgetc
#endif /* fgetc */
#define fgetc(s)                          sys_fgetc(s)

#ifdef gets
#undef gets
#endif /* gets */
#define gets(s)                           sys_gets(s)

#ifdef fgets
#undef fgets
#endif /* fgets */
#define fgets(c, i, s)                    sys_fgets(c, i, s)

#ifdef putc
#undef putc
#endif /* putc */
#define putc(c, s)                        sys_putc(c, s)

#ifdef fputc
#undef fputc
#endif /* fputc */
#define fputc(c, s)                       sys_fputc(c, s)

#ifdef puts
#undef pus
#endif /* puts */
#define puts(b)                           sys_puts(b)

#ifdef fputs
#undef fputs
#endif /* fputc */
#define fputs(b, s)                       sys_fputs(b, s)

#ifdef fflush
#undef fflush
#endif /* fflush */
#define fflush(s)                         sys_fflush(s)

#ifdef fclose
#undef fclose
#endif /* fclose */
#define fclose(s)                         sys_fclose(s)

#ifdef printf
#undef printf
#endif /* printf */
#define printf(f, ...)                    sys_printf(f, __VA_ARGS__ )

#ifdef vprintf
#undef vprintf
#endif /* vprintf */
#define vprintf(f, v)                     sys_vfprintf(f, v)

#ifdef fprintf
#undef fprintf
#endif /* fprintf */
#define fprintf(s, ...)                   sys_fprintf(s, __VA_ARGS__ )

#ifdef vfprintf
#undef vfprintf
#endif /* vfprintf */
#define vfprintf(s, f, v)                 sys_vfprintf(s, f, v)

#ifdef fwrite
#undef fwrite
#endif /* fwrite */
#define fwrite(p, i, n, s)                sys_fwrite(p, i, n, s)

#ifdef fread
#undef fread
#endif /* fread */
#define fread(p, i, n, s)                 sys_fread(p, i, n, s)

#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */

#endif /* SOCKSLIBRARY_DYNAMIC */

/* not used in client. */
#define loglevel_errno(e,  side) (LOG_DEBUG)
#define loglevel_gaierr(e, side) (LOG_DEBUG)

#define FDPASS_MAX         2   /* max number of descriptors we send/receive.  */

typedef struct {
   unsigned char      inited;

   unsigned char      havegssapisockets;/* have any gssapi-sockets?           */
   unsigned char      threadlockenabled;/* is threadlocking enabled?          */

   ssize_t            executingdnscode; /* exec. gethost*()/getname*()/etc.   */
   unsigned char      internalerrordetected;
   sig_atomic_t       insignal;         /* executing in signalhandler?        */
   sig_atomic_t       handledsignal;   /*
                                        * between now and the time this
                                        * variable was last cleared, did we
                                        * handle a signal?
                                        */

   sockshost_t        lastconnect;      /* address we last connected to.      */
   pid_t              pid;              /* our pid.                           */
   rlim_t             maxopenfiles;
} configstate_t;

typedef struct {
   int               debug;
   int               directfallback; /* fallback to direct connections        */
   const char        *configfile;    /* name of current configfile.           */
} option_t;

struct config {
   pid_t                    connectchild;            /* connect process.      */
   int                      child_data;              /* data socket to child. */
   int                      child_ack;               /* ack to child.         */

   char                     domain[MAXHOSTNAMELEN];  /* localdomain.          */

   logtype_t                errlog;                  /* for errors only.      */
   logtype_t                log;                     /* where to log.         */
   int                      loglock;                 /* lockfile for logging. */

   option_t                 option;                  /* misc. options.        */
   int                      resolveprotocol;         /* resolveprotocol.      */

   routeoptions_t           routeoptions;            /* global route flags.   */
   route_t                  *route;                  /* linked list of routes */

   /* XXX not supported in client yet. */
   socketoption_t           *socketoptionv;          /* global socket options.*/
   size_t                   socketoptionc;

   configstate_t            state;
   timeout_t                timeout;
};

typedef struct {
   int           s;         /* socket used for control-connection.     */
   socks_t       packet;    /* socks packet exchanged with server.     */
} childpacket_t;

typedef sigset_t addrlockopaque_t;

/*
 * libsocks function declarations
 */

void __ATTRIBUTE__((ATTRIBUTE_CONSTRUCTOR))
clientinit(void);
/*
 * initializes client state, reads config file, etc.
 */

#if !HAVE_OSF_OLDSTYLE
int Raccept(int, struct sockaddr *, socklen_t *);
int Rconnect(int, const struct sockaddr *, socklen_t);
int Rgetsockname(int, struct sockaddr *, socklen_t *);
int Rgetsockopt(int, int, int, void *, socklen_t *);
int Rgetpeername(int, struct sockaddr *, socklen_t *);
ssize_t Rsendto(int s, const void *msg, size_t len, int flags,
                const struct sockaddr *to, socklen_t tolen)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t Rrecvfrom(int s, void *buf, size_t len, int flags,
                  struct sockaddr * from, socklen_t *fromlen)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t Rsendmsg(int s, const struct msghdr *msg, int flags);
ssize_t Rrecvmsg(int s, struct msghdr *msg, int flags);
int Rbind(int, const struct sockaddr *, socklen_t);
#endif /* !HAVE_OSF_OLDSTYLE */

int Rbindresvport(int, struct sockaddr_in *);
int Rrresvport(int *);
struct hostent *Rgethostbyname(const char *);
struct hostent *Rgethostbyname2(const char *, int af);

#if HAVE_GETNAMEINFO

int Rgetnameinfo(const struct sockaddr *sa, socklen_t salen, char *host,
                 socklen_t hostlen, char *serv, socklen_t servlen,
                 int flags);

#endif /* HAVE_GETNAMEINFO  */

#if HAVE_GETADDRINFO
int Rgetaddrinfo(const char *nodename, const char *servname,
                 const struct addrinfo *hints, struct addrinfo **res);
#endif /* HAVE_GETADDRINFO */

#if HAVE_GETIPNODEBYNAME
struct hostent *Rgetipnodebyname(const char *, int, int, int *);
void Rfreehostent(struct hostent *);
#endif /* HAVE_GETIPNODEBYNAME */

ssize_t Rwrite(int d, const void *buf, size_t nbytes)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t Rwritev(int d, const struct iovec *iov, int iovcnt);
ssize_t Rsend(int s, const void *msg, size_t len, int flags)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t Rread(int d, void *buf, size_t nbytes)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t Rreadv(int d, const struct iovec *iov, int iovcnt);
ssize_t Rrecv(int s, void *msg, size_t len, int flags)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
int Rfgetc(FILE *fp);
char *Rgets(char *s);
char *Rfgets(char *s, int size, FILE *fp);
int Rfputc(int c, FILE *fp);
int Rfputs(const char *s, FILE *fp);
int Rfflush(FILE *fp);
int Rfclose(FILE *fp);
int Rfprintf(FILE *stream, const char *format, ...);
int Rvfprintf(FILE *stream, const char *format, va_list ap);
size_t Rfread(void *ptr, size_t size, size_t nmemb, FILE *s);
size_t Rfwrite(const void *ptr, size_t size, size_t nmemb, FILE *s);
#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */


int SOCKSinit(char *);
int Rlisten(int, int);
int Rselect(int, fd_set *, fd_set *, fd_set *, struct timeval *);
/*
 * unused functions needed to compile programs with support for other
 * socks implementations.
 */




int
cgetaddrinfo(const char *name, const char *service,
             const struct addrinfo *hints, struct addrinfo **res,
             dnsinfo_t *resmem);
/*
 * Like getaddrinfo(3), but "resmem" is used to hold the contents of "res",
 * rather than allocating the memory for "res" dynamically and then
 * having to call freeaddrinfo(3).
 */

route_t *
udpsetup(int s, const struct sockaddr_storage *to, int type, int shouldconnect,
         char *emsg, const size_t emsglen);
/*
 * sets up udp relaying between address of "s" and "to" by connecting
 * to a proxy server.
 * If relaying is already set up the function returns with success.
 * "type" is the type of connection to set up, SOCKS_SEND or SOCKS_RECV.
 * "shouldconnect" indicates whether the socket should be connected or not.
 *
 * Returns the route that was used (possibly a direct route), or NULL if no
 * route could be set up.  In the latter case, errno and emsg will be set.
 */

int
fd_is_network_socket(const int fd);
/*
 * Returns true if "fd" is a network socket of a kind we support.
 * Returns False otherwise.
 */




   /*
    *  Misc. functions to help keep track of our connection(s) to the server.
    */

void socks_addrinit(void);
/*
 * inits thing, including memory and locks, for socks_addaddr()-functions.
 */

void socks_addrlock(const int locktype, addrlockopaque_t *opaque);
void socks_addrunlock(const addrlockopaque_t *opaque);
/*
 * Lock/unlock global address object.
 * "type" is one of F_WRLCK or F_RDLCK, for write or read-lock.
 * "opaque" is a pointer filled in by "socks_addrlock()", and
 * the same pointer needs to be passed to socks_addrunlock();
 */

socksfd_t *
socks_addrdup(const socksfd_t *old, socksfd_t *new);
/*
 * Duplicates "old", in "new".
 * Returns:
 *    On success: "new".
 *    On failure: NULL (resource shortage).
 */

socksfd_t *
socks_addaddr(const int clientfd, const socksfd_t *socksaddress,
              const int takelock);
/*
 * "clientfd" is associated with the structure "socksfd".
 * If "takelock" is true, it means the function should take the
 * socksfdv/addrlock.
 *
 * The function duplicates all arguments in it's own form and does
 * not access the memory referenced by them afterwards.
 *
 * The function checks the state of all file descriptors on each call and
 * removes those that are no longer open.
 *
 * Returns:
 *      On success: pointer to the added socksfd_t structure.
 *      On failure: exits.  (memory exhausted and process grew descriptor size.)
 *
 */

socksfd_t *
socks_getaddr(const int fd, socksfd_t *socksfd, const int takelock);
/*
 * Returns a copy of the socksfd corresponding to "fd".
 * If "socksfd" is not NULL, the contents of the socksfd is also stored in
 * "socksfd".
 *
 * If "takelock" is true, it means the function should take the
 * socksfdv/addrlock.
 *
 * Returns:
 *      On success:  the socket address associated with file descriptor "fd".
 *      On failure:    NULL.  (no socket address associated with "fd").
 */

void
socks_rmaddr(const int s, const int takelock);
/*
 * If "takelock" is true, it means the function should take the
 * socksfdv/addrlock.
 *
 * removes the association for the socket "s", also closes the server
 * connection.  If "s" is not registered the request is ignored.
 */

int
socks_addrcontrol(const int controlsent, const int controlreceived,
                  const int takelock);
/*
 * Goes through all addresses registered and tries to find one where
 * the control socket has a local address of "local" and peer address
 * of "remote".
 *
 * "controlsent" gives the expected fd index for control, if not -1.
 * That is the fd index control had when we sent the request to
 * our connect-child, and may belong to another fd now.
 *
 * "controlreceived" is the actual fd we sent to the connectchild, and
 * which we now receive back from it.
 *
 *   Returns:
 *      On success: the descriptor the socksfd struct was registered with.
 *      On failure: -1
 */

int
socks_addrmatch(const struct sockaddr_storage *local,
                const struct sockaddr_storage *remote,
                const socksstate_t *state, const int takelock);
/*
 * If "takelock" is true, it means the function should take the
 * socksfdv/addrlock.
 *
 * Goes through all addresses registered and tries to find one where
 * all arguments match.
 * Arguments that are NULL or have "illegal" values are ignored.
 * Returns:
 *      On success: the descriptor the socksfd with matching arguments was
 *                registered with (>= 0).
 *      On failure: -1.
 */

int
socks_addrisours(const int s, socksfd_t *socksfd, const int takelock);
/*
 * Compares the current address of "s" to the registered address.
 * If there is a mismatch, the function will try to correct it if possible.
 *
 * If "takelock" is true, it means the function should take the
 * socksfdv/addrlock.
 *
 * If the current address matches the registered address and "socksfd"
 * is not NULL, "socksfd" is filled in with the data of the matching socket.
 *
 * Returns:
 *      If current address found to match registered: true.
 *      Else: false.
 */

void
update_after_negotiate(const socks_t *packet, socksfd_t *socksfd);
/*
 * Updates "socksfd" after a successful socks_negotiate() using
 * that used "packet".
 */

int
fdisopen(int fd);
/*
 * returns 1 if the file descriptor "fd" currently references a open object.
 * returns 0 otherwise.
 */


int
our_sigio_is_installed(void);  
/*
 * Returns true if our own SIGIO handler is currently installed.
 * Returns false otherwise.
 */

int
install_sigio(char *emsg, const size_t emsglen);
/*
 * Installs our own SIGIO handler.  
 * Returns 0 on success.  -1 on failure.
 */


#if DIAGNOSTIC
void
cc_socksfdv(int sig);
/*
 * consistency check on socksfdv.
 */
#endif /* DIAGNOSTIC */


#if SOCKSLIBRARY_DYNAMIC

int sys_rresvport(int *);
int sys_bindresvport(int, struct sockaddr_in *);
void sys_freehostent(struct hostent *);

HAVE_PROT_READ_0
sys_read(HAVE_PROT_READ_1, HAVE_PROT_READ_2, HAVE_PROT_READ_3);
HAVE_PROT_LISTEN_0
sys_listen(HAVE_PROT_LISTEN_1, HAVE_PROT_LISTEN_2);
HAVE_PROT_READV_0
sys_readv(HAVE_PROT_READV_1, HAVE_PROT_READV_2, HAVE_PROT_READV_3);
HAVE_PROT_RECV_0
sys_recv(HAVE_PROT_RECV_1, HAVE_PROT_RECV_2, HAVE_PROT_RECV_3,
      HAVE_PROT_RECV_4);
HAVE_PROT_RECVMSG_0
sys_recvmsg(HAVE_PROT_RECVMSG_1, HAVE_PROT_RECVMSG_2, HAVE_PROT_RECVMSG_3);
HAVE_PROT_SEND_0
sys_send(HAVE_PROT_SEND_1, HAVE_PROT_SEND_2, HAVE_PROT_SEND_3,
      HAVE_PROT_SEND_4);
HAVE_PROT_WRITE_0
sys_write(HAVE_PROT_WRITE_1, HAVE_PROT_WRITE_2, HAVE_PROT_WRITE_3);

#if HAVE_OSF_OLDSTYLE
int sys_accept(int, struct sockaddr *, int *);
#else
HAVE_PROT_ACCEPT_0
sys_accept(HAVE_PROT_ACCEPT_1, HAVE_PROT_ACCEPT_2, HAVE_PROT_ACCEPT_3);
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_OSF_OLDSTYLE
int sys_bind(int, const struct sockaddr *, int);
#else
HAVE_PROT_BIND_0
sys_bind(HAVE_PROT_BIND_1, HAVE_PROT_BIND_2, HAVE_PROT_BIND_3);
#endif /* !HAVE_OSF_OLDSTYLE */

#if HAVE_OSF_OLDSTYLE
int sys_connect(int, const struct sockaddr *, int);
#else
HAVE_PROT_CONNECT_0
sys_connect(HAVE_PROT_CONNECT_1, HAVE_PROT_CONNECT_2, HAVE_PROT_CONNECT_3);
#endif /* HAVE_OSF_OLDSTYLE */

#if HAVE_OSF_OLDSTYLE
int sys_getpeername(int, struct sockaddr *, int *);
#else
HAVE_PROT_GETPEERNAME_0
sys_getpeername(HAVE_PROT_GETPEERNAME_1, HAVE_PROT_GETPEERNAME_2,
      HAVE_PROT_GETPEERNAME_3);
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#if HAVE_OSF_OLDSTYLE
int sys_getsockname(int, struct sockaddr *, int *);
#else
HAVE_PROT_GETSOCKNAME_0
sys_getsockname(HAVE_PROT_GETSOCKNAME_1, HAVE_PROT_GETSOCKNAME_2,
      HAVE_PROT_GETSOCKNAME_3);
HAVE_PROT_GETSOCKNAME_0
sys_getsockname_notracking(HAVE_PROT_GETSOCKNAME_1, HAVE_PROT_GETSOCKNAME_2,
      HAVE_PROT_GETSOCKNAME_3);
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

HAVE_PROT_GETSOCKOPT_0
sys_getsockopt(HAVE_PROT_GETSOCKOPT_1, HAVE_PROT_GETSOCKOPT_2,
      HAVE_PROT_GETSOCKOPT_3, HAVE_PROT_GETSOCKOPT_4, HAVE_PROT_GETSOCKOPT_5);

#if HAVE_OSF_OLDSTYLE
int sys_recvfrom(int, void*, int, int, struct sockaddr *, int *);
#else
HAVE_PROT_RECVFROM_0
sys_recvfrom(HAVE_PROT_RECVFROM_1, HAVE_PROT_RECVFROM_2, HAVE_PROT_RECVFROM_3,
      HAVE_PROT_RECVFROM_4, HAVE_PROT_RECVFROM_5, HAVE_PROT_RECVFROM_6);
#endif /* HAVE_OSF_OLDSTYLE */

#if HAVE_OSF_OLDSTYLE
ssize_t sys_writev(int, const struct iovec *, int);
#else
HAVE_PROT_WRITEV_0
sys_writev(HAVE_PROT_WRITEV_1, HAVE_PROT_WRITEV_2, HAVE_PROT_WRITEV_3);
#endif /* HAVE_OSF_OLDSTYLE */

#if HAVE_OSF_OLDSTYLE
ssize_t sys_sendmsg(int, struct msghdr *, int);
#else
HAVE_PROT_SENDMSG_0
sys_sendmsg(HAVE_PROT_SENDMSG_1, HAVE_PROT_SENDMSG_2, HAVE_PROT_SENDMSG_3);
#endif /* HAVE_OSF_OLDSTYLE */

#if HAVE_OSF_OLDSTYLE
int sys_sendto(int, const void *, int, int, const struct sockaddr *,
      socklen_t);
#else
HAVE_PROT_SENDTO_0
sys_sendto(HAVE_PROT_SENDTO_1, HAVE_PROT_SENDTO_2, HAVE_PROT_SENDTO_3,
      HAVE_PROT_SENDTO_4, HAVE_PROT_SENDTO_5, HAVE_PROT_SENDTO_6);
#endif /* HAVE_OSF_OLDSTYLE */

#if HAVE_EXTRA_OSF_SYMBOLS
int sys_Eaccept(int, struct sockaddr *, socklen_t *);
int sys_Egetpeername(int, struct sockaddr *, socklen_t *);
int sys_Egetsockname(int, struct sockaddr *, socklen_t *);
ssize_t sys_Ereadv(int, const struct iovec *, int);
int sys_Erecvfrom(int, void *, size_t, int, struct sockaddr *, size_t *)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t sys_Erecvmsg(int, struct msghdr *, int);
ssize_t sys_Esendmsg(int, const struct msghdr *, int);
ssize_t sys_Ewritev(int, const struct iovec *, int);

int sys_naccept(int, struct sockaddr *, socklen_t *);
int sys_ngetpeername(int, struct sockaddr *, socklen_t *);
int sys_ngetsockname(int, struct sockaddr *, socklen_t *);
int sys_nrecvfrom(int, void *, size_t, int, struct sockaddr *, size_t *)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
ssize_t sys_nrecvmsg(int, struct msghdr *, int);
ssize_t sys_nsendmsg(int, const struct msghdr *, int);
#endif /* HAVE_EXTRA_OSF_SYMBOLS */

#ifdef __sun
HAVE_PROT_BIND_0
sys_xnet_bind(HAVE_PROT_BIND_1, HAVE_PROT_BIND_2, HAVE_PROT_BIND_3);
HAVE_PROT_LISTEN_0
sys_xnet_listen(HAVE_PROT_LISTEN_1, HAVE_PROT_LISTEN_2);
HAVE_PROT_RECVMSG_0
sys_xnet_recvmsg(HAVE_PROT_RECVMSG_1, HAVE_PROT_RECVMSG_2, HAVE_PROT_RECVMSG_3);
HAVE_PROT_SENDMSG_0
sys_xnet_sendmsg(HAVE_PROT_SENDMSG_1, HAVE_PROT_SENDMSG_2, HAVE_PROT_SENDMSG_3);
HAVE_PROT_SENDTO_0
sys_xnet_sendto(HAVE_PROT_SENDTO_1, HAVE_PROT_SENDTO_2, HAVE_PROT_SENDTO_3,
   HAVE_PROT_SENDTO_4, HAVE_PROT_SENDTO_5, HAVE_PROT_SENDTO_6);
HAVE_PROT_CONNECT_0
sys_xnet_connect(HAVE_PROT_CONNECT_1, HAVE_PROT_CONNECT_2, HAVE_PROT_CONNECT_3);
#endif /* __sun */

#ifdef __FreeBSD__
HAVE_PROT_ACCEPT_0
_accept(HAVE_PROT_ACCEPT_1, HAVE_PROT_ACCEPT_2, HAVE_PROT_ACCEPT_3);
HAVE_PROT_BIND_0
_bind(HAVE_PROT_BIND_1, HAVE_PROT_BIND_2, HAVE_PROT_BIND_3);
HAVE_PROT_CONNECT_0
_connect(HAVE_PROT_CONNECT_1, HAVE_PROT_CONNECT_2, HAVE_PROT_CONNECT_3);
HAVE_PROT_GETPEERNAME_0
_getpeername(HAVE_PROT_GETPEERNAME_1, HAVE_PROT_GETPEERNAME_2,
    HAVE_PROT_GETPEERNAME_3);
HAVE_PROT_GETSOCKNAME_0
_getsockname(HAVE_PROT_GETSOCKNAME_1, HAVE_PROT_GETSOCKNAME_2,
    HAVE_PROT_GETSOCKNAME_3);
HAVE_PROT_GETSOCKOPT_0
_getsockopt(HAVE_PROT_GETSOCKOPT_1, HAVE_PROT_GETSOCKOPT_2,
    HAVE_PROT_GETSOCKOPT_3, HAVE_PROT_GETSOCKOPT_4, HAVE_PROT_GETSOCKOPT_5);
HAVE_PROT_LISTEN_0
_listen(HAVE_PROT_LISTEN_1, HAVE_PROT_LISTEN_2);
HAVE_PROT_READ_0
_read(HAVE_PROT_READ_1, HAVE_PROT_READ_2, HAVE_PROT_READ_3);
HAVE_PROT_READV_0
_readv(HAVE_PROT_READV_1, HAVE_PROT_READV_2, HAVE_PROT_READV_3);
HAVE_PROT_RECV_0
_recv(HAVE_PROT_RECV_1, HAVE_PROT_RECV_2, HAVE_PROT_RECV_3, HAVE_PROT_RECV_4);
HAVE_PROT_RECVFROM_0
_recvfrom(HAVE_PROT_RECVFROM_1, HAVE_PROT_RECVFROM_2, HAVE_PROT_RECVFROM_3,
    HAVE_PROT_RECVFROM_4, HAVE_PROT_RECVFROM_5, HAVE_PROT_RECVFROM_6);
HAVE_PROT_RECVMSG_0
_recvmsg(HAVE_PROT_RECVMSG_1, HAVE_PROT_RECVMSG_2, HAVE_PROT_RECVMSG_3);
HAVE_PROT_WRITE_0
_write(HAVE_PROT_WRITE_1, HAVE_PROT_WRITE_2, HAVE_PROT_WRITE_3);
HAVE_PROT_WRITEV_0
_writev(HAVE_PROT_WRITEV_1, HAVE_PROT_WRITEV_2, HAVE_PROT_WRITEV_3);
HAVE_PROT_SEND_0
_send(HAVE_PROT_SEND_1, HAVE_PROT_SEND_2, HAVE_PROT_SEND_3, HAVE_PROT_SEND_4);
HAVE_PROT_SENDMSG_0
_sendmsg(HAVE_PROT_SENDMSG_1, HAVE_PROT_SENDMSG_2, HAVE_PROT_SENDMSG_3);
HAVE_PROT_SENDTO_0
_sendto(HAVE_PROT_SENDTO_1, HAVE_PROT_SENDTO_2, HAVE_PROT_SENDTO_3,
    HAVE_PROT_SENDTO_4, HAVE_PROT_SENDTO_5, HAVE_PROT_SENDTO_6);
#endif /* __FreeBSD__ */

#if HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND
HAVE_PROT_GETC_0
sys_getc(HAVE_PROT_GETC_1);
HAVE_PROT_FGETC_0
sys_fgetc(HAVE_PROT_FGETC_1);
HAVE_PROT_GETS_0
sys_gets(HAVE_PROT_GETS_1);
HAVE_PROT_FGETS_0
sys_fgets(HAVE_PROT_FGETS_1, HAVE_PROT_FGETS_2, HAVE_PROT_FGETS_3);
HAVE_PROT_PUTC_0
sys_putc(HAVE_PROT_PUTC_1, HAVE_PROT_PUTC_2);
HAVE_PROT_FPUTC_0
sys_fputc(HAVE_PROT_FPUTC_1, HAVE_PROT_FPUTC_2);
HAVE_PROT_PUTS_0
sys_puts(HAVE_PROT_PUTS_1);
HAVE_PROT_FPUTS_0
sys_fputs(HAVE_PROT_FPUTS_1, HAVE_PROT_FPUTS_2);
HAVE_PROT_FFLUSH_0
sys_fflush(HAVE_PROT_FFLUSH_1);
HAVE_PROT_FCLOSE_0
sys_fclose(HAVE_PROT_FCLOSE_1);
HAVE_PROT_PRINTF_0
sys_printf(HAVE_PROT_PRINTF_1, ...);
HAVE_PROT_VPRINTF_0
sys_vprintf(HAVE_PROT_VPRINTF_1, HAVE_PROT_VPRINTF_2);
HAVE_PROT_FPRINTF_0
sys_fprintf(HAVE_PROT_FPRINTF_1, HAVE_PROT_FPRINTF_2, ...);
HAVE_PROT_VFPRINTF_0
sys_vfprintf(HAVE_PROT_VFPRINTF_1, HAVE_PROT_VFPRINTF_2, HAVE_PROT_VFPRINTF_3);
HAVE_PROT_FWRITE_0
sys_fwrite(HAVE_PROT_FWRITE_1, HAVE_PROT_FWRITE_2, HAVE_PROT_FWRITE_3,
      HAVE_PROT_FWRITE_4);
HAVE_PROT_FREAD_0
sys_fread(HAVE_PROT_FREAD_1, HAVE_PROT_FREAD_2, HAVE_PROT_FREAD_3,
      HAVE_PROT_FREAD_4);
#endif /* HAVE_GSSAPI && HAVE_LINUX_GLIBC_WORKAROUND */

#if HAVE_DARWIN

HAVE_PROT_READ_0
sys_read_nocancel(HAVE_PROT_READ_1, HAVE_PROT_READ_2, HAVE_PROT_READ_3);
HAVE_PROT_CONNECT_0
sys_connect_nocancel(HAVE_PROT_CONNECT_1, HAVE_PROT_CONNECT_2,
   HAVE_PROT_CONNECT_3 namelen);
HAVE_PROT_RECVFROM_0
sys_recvfrom_nocancel(HAVE_PROT_RECVFROM_1, HAVE_PROT_RECVFROM_2,
   HAVE_PROT_RECVFROM_3, HAVE_PROT_RECVFROM_4, HAVE_PROT_RECVFROM_5,
   HAVE_PROT_RECVFROM_6);
HAVE_PROT_SENDTO_0
sys_sendto_nocancel(HAVE_PROT_SENDTO_1, HAVE_PROT_SENDTO_2, HAVE_PROT_SENDTO_3,
   HAVE_PROT_SENDTO_4, HAVE_PROT_SENDTO_5, HAVE_PROT_SENDTO_6);
HAVE_PROT_WRITE_0
sys_write_nocancel(HAVE_PROT_WRITE_1, HAVE_PROT_WRITE_2, HAVE_PROT_WRITE_3);

#endif /* HAVE_DARWIN */

#endif /* SOCKSLIBRARY_DYNAMIC */

#endif /* !_SOCKS_H_ */
