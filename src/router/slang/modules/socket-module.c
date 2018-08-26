/* -*- mode: C; mode: fold; -*- */
/*
Copyright (C) 2006-2011 John E. Davis

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

/* This file was derived from the code in SLtcp.c distributed with slsh */

/*{{{ Include Files */

#include "config.h"
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <ctype.h>
#include <stdarg.h>

#include <setjmp.h>
#include <signal.h>

#include <sys/types.h>

#include <time.h>
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#ifdef HAVE_SOCKET_H
# include <socket.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#if defined(__NT__)
# include <winsock.h>
# define USE_WINSOCK_SLTCP	1
#else
# if defined(__MINGW32__)
#  define Win32_Winsock
#  include <windows.h>
#  define USE_WINSOCK_SLTCP	1
# endif
#endif

#ifdef USE_WINSOCK_SLTCP
# define USE_WINSOCK_SLTCP	1
#else
# include <netdb.h>
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>		       /* for AF_UNIX sockets */
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#include <slang.h>
SLANG_MODULE(socket);

#ifndef h_errno
extern int h_errno;
#endif

/*}}}*/

static int SocketError = -1;
static int SocketHerrnoError = -1;
static int Socket_Type_Id = -1;

typedef struct Socket_Type Socket_Type;
typedef struct
{
   int domain;
   int (*connect)(Socket_Type *, int);
   int (*bind)(Socket_Type *, int);
#define MAX_ACCEPT_REF_ARGS 4
   Socket_Type *(*accept)(Socket_Type *, unsigned int, SLang_Ref_Type **);
   void (*free_socket_data)(Socket_Type *);
}
Domain_Methods_Type;

struct Socket_Type
{
   int fd;			       /* socket descriptor */
   Domain_Methods_Type *methods;
   VOID_STAR socket_data;
   int domain;
   int type;
   int protocol;
};

static Socket_Type *create_socket (int, int, int, int);
static void free_socket (Socket_Type *);
static int close_socket (int);

/*{{{ Generic Routines */

static int Module_H_Errno = 0;

static char *herror_to_string (int h)
{
#ifdef HOST_NOT_FOUND
   if (h == HOST_NOT_FOUND)
     return "The specified host is unknown";
#endif

#ifdef NO_ADDRESS
   if (h == NO_ADDRESS)
     return "The requested name is valid but does not have an IP address";
#endif

#ifdef NO_DATA
   if (h == NO_DATA)
     return "The requested name is valid but does not have an IP address";
#endif

#ifdef NO_RECOVERY
   if (h == NO_RECOVERY)
     return "A non-recoverable name server error occurred";
#endif

#ifdef TRY_AGAIN
   if (h == TRY_AGAIN)
     return "A temporary error occurred on an authoritative name server.  Try again later";
#endif

   return "Unknown h_error";
}

static void throw_herror (char *what, int h)
{
   Module_H_Errno = h;
   SLang_verror (SocketHerrnoError, "%s: %s", what, herror_to_string(h));
}

static void throw_errno_error (char *what, int e)
{
   SLerrno_set_errno (e);
   SLang_verror (SocketError, "%s: %s", what, SLerrno_strerror (e));
}

static int perform_connect (int fd, struct sockaddr *addr, unsigned int len, int throw_err)
{
   while (-1 == connect (fd, addr, len))
     {
#ifdef EINTR
	if (errno == EINTR)
	  {
	     if (-1 == SLang_handle_interrupt ())
	       return -1;
	     continue;
	  }
#endif
	/* The manpage indicates EAGAIN will be returned if no free ports exist.
	 * So allow the caller to handle that.
	 */
	if (throw_err)
	  throw_errno_error ("connect", errno);
	return -1;
     }
   return 0;
}

static int perform_bind (int fd, struct sockaddr *addr, unsigned int len)
{
   while (-1 == bind (fd, addr, len))
     {
#ifdef EINTR
	if (errno == EINTR)
	  {
	     if (-1 == SLang_handle_interrupt ())
	       return -1;
	     continue;
	  }
#endif
	/* The manpage indicates EAGAIN will be returned if no free ports exist.
	 * So allow the caller to handle that.
	 */
	throw_errno_error ("bind", errno);
	return -1;
     }
   return 0;
}

static Socket_Type *perform_accept (Socket_Type *s, struct sockaddr *addr, unsigned int *lenp)
{
   socklen_t addr_len;
   Socket_Type *s1;
   int fd1;

   addr_len = *lenp;
   while (-1 == (fd1 = accept (s->fd, addr, &addr_len)))
     {
#ifdef EINTR
	if (errno == EINTR)
	  {
	     if (-1 == SLang_handle_interrupt ())
	       return NULL;
	     continue;
	  }
#endif
	throw_errno_error ("accept", errno);
	return NULL;
     }
   *lenp = (unsigned int) addr_len;
   if (NULL == (s1 = create_socket (fd1, s->domain, s->type, s->protocol)))
     (void) close_socket (fd1);

   return s1;
}

/*}}}*/

/* Domain Methods */
#if defined(PF_UNIX) && defined(AF_UNIX) /*{{{ */

static void free_af_unix (Socket_Type *s)
{
   char *file = (char *) s->socket_data;
   if (file == NULL)
     return;

   (void) unlink (file);
   SLang_free_slstring (file);
   s->socket_data = NULL;
}

static int connect_af_unix (Socket_Type *s, int nargs)
{
   struct sockaddr_un addr;
   char *file;

   if (nargs != 1)
     {
	SLang_verror (SL_NumArgs_Error, "This socket expects a filename");
	return -1;
     }
   if (-1 == SLang_pop_slstring (&file))
     return -1;

   if (strlen (file) >= sizeof(addr.sun_path))
     {
	SLang_verror (SL_InvalidParm_Error, "filename too long for PF_UNIX socket");
	SLang_free_slstring (file);
	return -1;
     }

   memset ((char *)&addr, 0, sizeof (struct sockaddr_un));
   addr.sun_family = AF_UNIX;
   strcpy (addr.sun_path, file);       /* \0 terminated */

   SLang_free_slstring (file);
   return perform_connect (s->fd, (struct sockaddr *)&addr, sizeof (addr), 1);
}

static int bind_af_unix (Socket_Type *s, int nargs)
{
   struct sockaddr_un addr;
   char *file;

   if (nargs != 1)
     {
	SLang_verror (SL_NumArgs_Error, "This socket expects a filename");
	return -1;
     }
   if (-1 == SLang_pop_slstring (&file))
     return -1;

   if (strlen (file) >= sizeof(addr.sun_path))
     {
	SLang_verror (SL_InvalidParm_Error, "filename too long for PF_UNIX socket");
	SLang_free_slstring (file);
	return -1;
     }

   memset ((char *)&addr, 0, sizeof (struct sockaddr_un));
   addr.sun_family = AF_UNIX;
   strcpy (addr.sun_path, file);       /* \0 terminated */

   (void) unlink (file);
   s->socket_data = (VOID_STAR) file;
   return perform_bind (s->fd, (struct sockaddr *)&addr, sizeof (addr));
}

static Socket_Type *accept_af_unix (Socket_Type *s, unsigned int nrefs, SLang_Ref_Type **refs)
{
   struct sockaddr_un addr;
   Socket_Type *s1;
   unsigned int addr_len;

   (void) refs;
   if (nrefs != 0)
     {
	SLang_verror (SL_NotImplemented_Error, "accept: reference args not supported for PF_UNIX sockets");
	return NULL;
     }
   addr_len = sizeof (struct sockaddr_un);
   s1 = perform_accept (s, (struct sockaddr *)&addr, &addr_len);
   return s1;
}

#endif
/*}}}*/

#if defined(PF_INET) && defined(AF_INET) /*{{{*/
static int pop_host_port (char *what, int nargs, char **hostp, int *portp)
{
   char *host;
   int port;

   if (nargs != 2)
     {
	SLang_verror (SL_NumArgs_Error, "%s on an PF_INET socket requires a hostname and portnumber", what);
	return -1;
     }

   *hostp = NULL;
   if ((-1 == SLang_pop_int (&port))
       || (-1 == SLang_pop_slstring (&host)))
     return -1;

   *hostp = host;
   *portp = port;
   return 0;
}

typedef struct
{
   int h_addrtype;		       /* AF_INET or AF_INET6 */
   int h_length;		       /* length of address */
   unsigned int num;		       /* num elements of h_addr_list */
   char **h_addr_list;		       /* Array of num of these */
}
Host_Addr_Info_Type;

static void free_host_addr_info (Host_Addr_Info_Type *hinfo)
{
   if (hinfo == NULL)
     return;
   if (hinfo->h_addr_list != NULL)
     SLfree ((char *)hinfo->h_addr_list);
   SLfree ((char *) hinfo);
}

static Host_Addr_Info_Type *alloc_host_addr_info (unsigned int num, int h_length)
{
   Host_Addr_Info_Type *hinfo;
   unsigned int nbytes;
   char *buf;
   unsigned int i;

   hinfo = (Host_Addr_Info_Type *) SLcalloc (1, sizeof (Host_Addr_Info_Type));
   if (hinfo == NULL)
     return NULL;

   /* We need memory to hold num (char *) addresses + num*h_length bytes */
   nbytes = num * sizeof(char *) + num * h_length;
   if (NULL == (buf = SLmalloc (nbytes)))
     {
	SLfree ((char *)hinfo);
	return NULL;
     }
   hinfo->h_addr_list = (char **)buf;
   buf += num*sizeof(char *);
   for (i = 0; i < num; i++)
     {
	hinfo->h_addr_list[i] = buf;
	buf += h_length;
     }
   hinfo->num = num;
   hinfo->h_length = h_length;

   return hinfo;
}

/* glibc removed the h_addr compat macro, which messes up the logic below. */
#ifndef h_addr
# ifdef __GNUC_PREREQ
#  if __GNUC_PREREQ(2,8)
#   define h_addr "unused"	       /* define it, but do not use it */
#  endif
# endif
#endif

static Host_Addr_Info_Type *get_host_addr_info (char *host)
{
   in_addr_t addr;
   Host_Addr_Info_Type *hinfo;
   struct hostent *hp;
   unsigned int max_retries;
#ifndef h_addr
   char *fake_h_addr_list[2];
#endif
   char **h_addr_list;
   unsigned int i, num;

#ifndef INADDR_NONE
# define INADDR_NONE ((in_addr_t)(-1))
#endif
   if ((isdigit (*host))
       && (INADDR_NONE != (addr = inet_addr (host))))
     {
	/* Numerical address */
	if (NULL == (hinfo = alloc_host_addr_info (1, sizeof(in_addr_t))))
	  return NULL;
	hinfo->h_addrtype = AF_INET;
	memcpy (hinfo->h_addr_list[0], (char *)&addr, sizeof(in_addr_t));
	return hinfo;
     }

   max_retries = 3;
   while (NULL == (hp = gethostbyname (host)))
     {
#ifdef TRY_AGAIN
	max_retries--;
	if (max_retries && (h_errno == TRY_AGAIN))
	  {
	     sleep (1);
	     continue;
	  }
#endif
	throw_herror ("gethostbyname", h_errno);
	return NULL;
     }
#ifndef h_addr
   /* Older interface.  There is only one address, so fake a list */
   h_addr_list = fake_h_addr_list;
   h_addr_list [0] = hp->h_addr;
   h_addr_list [1] = NULL;
#else
   h_addr_list = hp->h_addr_list;
#endif

   /* Now count the number of addresses */
   num = 0;
   while (h_addr_list[num] != NULL)
     num++;

   if (num == 0)
     {
#ifdef NO_DATA
	throw_herror ("gethostbyname", NO_DATA);
#else
	throw_herror ("gethostbyname", NO_ADDRESS);
#endif
	return NULL;
     }

   if (NULL == (hinfo = alloc_host_addr_info (num, hp->h_length)))
     return NULL;

   hinfo->h_addrtype = hp->h_addrtype;

   for (i = 0; i < num; i++)
     memcpy (hinfo->h_addr_list[i], h_addr_list[i], hp->h_length);

   return hinfo;
}

static int connect_af_inet (Socket_Type *s, int nargs)
{
   struct sockaddr_in s_in;
   int port;
   char *host;
   Host_Addr_Info_Type *hinfo;
   unsigned int i;

   if (-1 == pop_host_port ("connect", nargs, &host, &port))
     return -1;

   if (NULL == (hinfo = get_host_addr_info (host)))
     {
	SLang_free_slstring (host);
	return -1;
     }

   if (hinfo->h_addrtype != AF_INET)
     {
#ifdef AF_INET6
	if (hinfo->h_addrtype == AF_INET6)
	  SLang_verror (SL_NOT_IMPLEMENTED, "AF_INET6 not implemented");
	else
#endif
	  SLang_verror (SocketError, "Unknown socket family for host %s", host);
	SLang_free_slstring (host);
	free_host_addr_info (hinfo);
	return -1;
     }

   memset ((char *) &s_in, 0, sizeof(s_in));
   s_in.sin_family = hinfo->h_addrtype;
   s_in.sin_port = htons((unsigned short) port);

   for (i = 0; i < hinfo->num; i++)
     {
	memcpy ((char *) &s_in.sin_addr, hinfo->h_addr_list[i], hinfo->h_length);
	if (-1 == perform_connect (s->fd, (struct sockaddr *)&s_in, sizeof (s_in), 0))
	  continue;

	free_host_addr_info (hinfo);
	SLang_free_slstring (host);
	return 0;
     }
   throw_errno_error ("connect", errno);
   free_host_addr_info (hinfo);
   SLang_free_slstring (host);
   return -1;
}

static int bind_af_inet (Socket_Type *s, int nargs)
{
   struct sockaddr_in s_in;
   char *host;
   int port;
   int status;
   Host_Addr_Info_Type *hinfo;

   if (-1 == pop_host_port ("connect", nargs, &host, &port))
     return -1;

   if (NULL == (hinfo = get_host_addr_info (host)))
     {
	SLang_free_slstring (host);
	return -1;
     }

   if (hinfo->h_addrtype != AF_INET)
     {
#ifdef AF_INET6
	if (hinfo->h_addrtype == AF_INET6)
	  SLang_verror (SL_NOT_IMPLEMENTED, "AF_INET6 not implemented");
	else
#endif
	  SLang_verror (SocketError, "Unknown socket family for host %s", host);
	SLang_free_slstring (host);
	free_host_addr_info (hinfo);
	return -1;
     }

   memset ((char *) &s_in, 0, sizeof(s_in));
   s_in.sin_family = hinfo->h_addrtype;
   s_in.sin_port = htons((unsigned short) port);

   memcpy ((char *) &s_in.sin_addr, hinfo->h_addr_list[0], hinfo->h_length);

   status = perform_bind (s->fd, (struct sockaddr *)&s_in, sizeof (s_in));

   free_host_addr_info (hinfo);
   SLang_free_slstring (host);
   return status;
}

/* Usage: s1 = accept (s [,&host,&port]); */
static Socket_Type *accept_af_inet (Socket_Type *s, unsigned int nrefs, SLang_Ref_Type **refs)
{
   struct sockaddr_in s_in;
   Socket_Type *s1;
   unsigned int addr_len;

   if ((nrefs != 0) && (nrefs != 2))
     {
	SLang_verror (SL_NumArgs_Error, "accept (sock [,&host,&port])");
	return NULL;
     }

   addr_len = sizeof (struct sockaddr_in);
   s1 = perform_accept (s, (struct sockaddr *)&s_in, &addr_len);

   if ((s1 == NULL) || (nrefs == 0))
     return NULL;

   if (nrefs == 2)
     {
	char *host;
	char host_ip[32];  /* aaa.bbb.ccc.ddd */
	unsigned char *bytes = (unsigned char *)&s_in.sin_addr;
	int port = ntohs (s_in.sin_port);
	sprintf (host_ip, "%d.%d.%d.%d",
		 (int)bytes[0],(int)bytes[1],(int)bytes[2],(int)bytes[3]);

	if (NULL == (host = SLang_create_slstring (host_ip)))
	  {
	     free_socket (s1);
	     return NULL;
	  }
	if (-1 == SLang_assign_to_ref (refs[0], SLANG_STRING_TYPE, (VOID_STAR)&host))
	  {
	     SLang_free_slstring (host);
	     free_socket (s1);
	     return NULL;
	  }
	SLang_free_slstring (host);
	if (-1 == SLang_assign_to_ref (refs[1], SLANG_INT_TYPE, &port))
	  {
	     free_socket (s1);
	     return NULL;
	  }
     }
   return s1;
}

#endif

/*}}}*/

static Domain_Methods_Type Domain_Methods_Table [] =
{
#if defined(PF_UNIX) && defined(AF_UNIX)
     {PF_UNIX, connect_af_unix, bind_af_unix, accept_af_unix, free_af_unix},
#endif
#if defined(PF_INET) && defined(AF_INET)
     {PF_INET, connect_af_inet, bind_af_inet, accept_af_inet, NULL},
#endif
     {0, NULL, NULL, NULL, NULL}
};

static Domain_Methods_Type *lookup_domain_methods (int domain)
{
   Domain_Methods_Type *a = Domain_Methods_Table;
   unsigned int i, n;

   n = sizeof (Domain_Methods_Table)/sizeof(Domain_Methods_Type);
   for (i = 0; i < n; i++)
     {
	if (a->domain == domain)
	  return a;
	a++;
     }

   SLang_verror (SocketError, "Unsupported socket domain: %d", domain);
   return NULL;
}

static int close_socket (int fd)
{
   while (-1 == close (fd))
     {
#ifdef EINTR
	if (errno == EINTR)
	  {
	     if (-1 == SLang_handle_interrupt ())
	       return -1;
	     continue;
	  }
#endif
	return -1;
     }
   return 0;
}

static void free_socket (Socket_Type *s)
{
   if (s == NULL)
     return;

   if ((s->methods != NULL) && (s->methods->free_socket_data != NULL))
     (*s->methods->free_socket_data)(s);

   if (s->fd != -1)
     close_socket (s->fd);

   SLfree ((char *) s);
}

static Socket_Type *create_socket (int fd, int domain, int type, int protocol)
{
   Socket_Type *s;
   Domain_Methods_Type *methods;

   if (NULL == (methods = lookup_domain_methods (domain)))
     return NULL;

   s = (Socket_Type *)SLmalloc (sizeof (Socket_Type));
   if (s == NULL)
     return s;
   memset ((char *)s, 0, sizeof (Socket_Type));

   s->fd = fd;
   s->domain = domain;
   s->protocol = protocol;
   s->type = type;
   s->methods = methods;

   return s;
}

static int close_socket_callback (VOID_STAR cd)
{
   Socket_Type *s;

   s = (Socket_Type *) cd;
   if (s->fd == -1)
     {
#ifdef EBADF
	errno = EBADF;
#endif
	return -1;
     }
   if (-1 == close (s->fd))
     return -1;

   s->fd = -1;
   return 0;
}

static void free_socket_callback (VOID_STAR cd)
{
   free_socket ((Socket_Type *)cd);
}

static SLFile_FD_Type *socket_to_fd (Socket_Type *s)
{
   SLFile_FD_Type *f;
   if (NULL == (f = SLfile_create_fd ("*socket*", s->fd)))
     return NULL;

   (void) SLfile_set_clientdata (f, free_socket_callback, (VOID_STAR)s, Socket_Type_Id);
   (void) SLfile_set_close_method (f, close_socket_callback);
   return f;
}

static Socket_Type *socket_from_fd (SLFile_FD_Type *f)
{
   Socket_Type *s;
   if (-1 == SLfile_get_clientdata (f, Socket_Type_Id, (VOID_STAR *)&s))
     {
	SLang_verror (SL_TypeMismatch_Error, "File descriptor does not represent a socket");
	return NULL;
     }
   return s;
}

/* This function frees the socket before returning */
static int push_socket (Socket_Type *s)
{
   SLFile_FD_Type *f;
   int status;

   if (s == NULL)
     return SLang_push_null ();

   if (NULL == (f = socket_to_fd (s)))
     {
	free_socket (s);
	return -1;
     }

   status = SLfile_push_fd (f);
   SLfile_free_fd (f);
   return status;
}

static Socket_Type *pop_socket (SLFile_FD_Type **fp)
{
   SLFile_FD_Type *f;
   Socket_Type *s;

   if (-1 == SLfile_pop_fd (&f))
     {
	*fp = NULL;
	return NULL;
     }
   if (NULL == (s = socket_from_fd (f)))
     {
	SLfile_free_fd (f);
	return NULL;
     }
   *fp = f;
   return s;
}

static void socket_intrin (int *domain, int *type, int *protocol)
{
   Socket_Type *s;
   Domain_Methods_Type *a;
   int fd;

   if (NULL == (a = lookup_domain_methods (*domain)))
     return;

   fd = socket (*domain, *type, *protocol);
   if (fd == -1)
     {
	throw_errno_error ("socket", errno);
	return;
     }

   if (NULL == (s = create_socket (fd, *domain, *type, *protocol)))
     {
	close_socket (fd);
	return;
     }

   (void) push_socket (s);	       /* frees it upon error */
   return;
}

#ifdef HAVE_SOCKETPAIR
static void socketpair_intrin (int *domain, int *type, int *protocol)
{
   Socket_Type *s;
   int fds[2];

   if (NULL == lookup_domain_methods (*domain))
     return;

   if (-1 == socketpair (*domain, *type, *protocol, fds))
     {
	throw_errno_error ("socketpair", errno);
	return;
     }

   if (NULL == (s = create_socket (fds[0], *domain, *type, *protocol)))
     {
	close_socket (fds[0]);
	close_socket (fds[1]);
	return;
     }
   if (-1 == push_socket (s))	       /* frees upon error */
     {
	close_socket (fds[1]);
	return;
     }
   if (NULL == (s = create_socket (fds[1], *domain, *type, *protocol)))
     {
	close_socket (fds[1]);
	return;
     }
   (void) push_socket (s);	       /* frees it upon error */
   return;
}
#endif

static void connect_intrin (void)
{
   Socket_Type *s;
   SLFile_FD_Type *f;
   int nargs = SLang_Num_Function_Args;
   Domain_Methods_Type *methods;

   if (-1 == SLroll_stack (-nargs))
     return;

   if (NULL == (s = pop_socket (&f)))
     return;
   nargs--;

   methods = s->methods;
   (void) (*methods->connect)(s, nargs);
   SLfile_free_fd (f);
}

static void bind_intrin (void)
{
   Socket_Type *s;
   SLFile_FD_Type *f;
   int nargs = SLang_Num_Function_Args;
   Domain_Methods_Type *methods;

   if (-1 == SLroll_stack (-nargs))
     return;

   if (NULL == (s = pop_socket (&f)))
     return;
   nargs--;

   methods = s->methods;
   (void)(*methods->bind)(s, nargs);
   SLfile_free_fd (f);
}

static void listen_intrin (SLFile_FD_Type *f, int *np)
{
   Socket_Type *s;

   if (NULL == (s = socket_from_fd (f)))
     return;

   if (0 == listen (s->fd, *np))
     return;

   throw_errno_error ("listen", errno);
}

static void accept_intrin (void)
{
   SLFile_FD_Type *f;
   Socket_Type *s, *s1;
   Domain_Methods_Type *methods;
   int nargs = SLang_Num_Function_Args;
   SLang_Ref_Type *refs[MAX_ACCEPT_REF_ARGS];
   int i;

   if (nargs <= 0)
     {
	SLang_verror (SL_Usage_Error, "s1 = accept (s [,&v...])");
	return;
     }

   if (-1 == SLroll_stack (-nargs))
     return;

   if (NULL == (s = pop_socket (&f)))
     return;
   nargs--;

   if (nargs > MAX_ACCEPT_REF_ARGS)
     {
	SLang_verror (SL_NumArgs_Error, "accept: too many reference args");
	SLfile_free_fd (f);
     }
   memset ((char *)refs, 0, sizeof (refs));

   i = nargs;
   while (i != 0)
     {
	i--;
	if (-1 == SLang_pop_ref (refs+i))
	  goto free_return;
     }

   methods = s->methods;
   if (NULL != (s1 = (*methods->accept)(s, nargs, refs)))
     (void) push_socket (s1);	       /* frees it upon error */

   /* drop */

   free_return:
   for (i = 0; i < nargs; i++)
     {
	if (refs[i] != NULL)
	  SLang_free_ref (refs[i]);
     }
   SLfile_free_fd (f);
}

typedef struct
{
   int optname;
   int (*setopt)(Socket_Type *, int, int);
   int (*getopt)(Socket_Type *, int, int);
}
SockOpt_Type;

static int do_setsockopt (int fd, int level, int optname, void *val, socklen_t len)
{
   if (-1 == setsockopt (fd, level, optname, val, len))
     {
	throw_errno_error ("setsockopt", errno);
	return -1;
     }
   return 0;
}

static int do_getsockopt (int fd, int level, int optname, void *val, socklen_t *lenp)
{
   if (-1 == getsockopt (fd, level, optname, val, lenp))
     {
	throw_errno_error ("getsockopt", errno);
	return -1;
     }
   return 0;
}

static int set_int_sockopt (Socket_Type *s, int level, int optname)
{
   int val;

   if (-1 == SLang_pop_int (&val))
     return -1;

   return do_setsockopt (s->fd, level, optname, (void *)&val, sizeof(int));
}

static int get_int_sockopt (Socket_Type *s, int level, int optname)
{
   int val;
   socklen_t len;

   len = sizeof (int);
   if (-1 == do_getsockopt (s->fd, level, optname, (void *)&val, &len))
     return -1;

   return SLang_push_int (val);
}

static int set_str_sockopt (Socket_Type *s, int level, int optname)
{
   char *val;
   socklen_t len;
   int ret;

   if (-1 == SLang_pop_slstring (&val))
     return -1;
   len = strlen (val); len++;
   ret = do_setsockopt (s->fd, level, optname, (void *)val, len);
   SLang_free_slstring (val);
   return ret;
}
static int get_str_sockopt (Socket_Type *s, int level, int optname)
{
   char buf[1024];
   socklen_t len = sizeof (buf)-1;

   if (-1 == do_getsockopt (s->fd, level, optname, (void *)buf, &len))
     return -1;

   buf[len] = 0;
   return SLang_push_string (buf);
}

static int set_struct_sockopt (Socket_Type *s, int level, int optname,
			       SLang_CStruct_Field_Type *cs, VOID_STAR v,
			       socklen_t len)
{
   int ret;

   if (-1 == SLang_pop_cstruct (v, cs))
     return -1;

   ret = do_setsockopt (s->fd, level, optname, v, len);
   SLang_free_cstruct (v, cs);
   return ret;
}

static int get_struct_sockopt (Socket_Type *s, int level, int optname,
			       SLang_CStruct_Field_Type *cs, VOID_STAR v,
			       socklen_t len)
{
   if (-1 == do_getsockopt (s->fd, level, optname, v, &len))
     return -1;

   return SLang_push_cstruct (v, cs);
}

static SLang_CStruct_Field_Type TV_Struct [] =
{
   MAKE_CSTRUCT_INT_FIELD(struct timeval, tv_sec, "tv_sec", 0),
   MAKE_CSTRUCT_INT_FIELD(struct timeval, tv_sec, "tv_usec", 0),
   SLANG_END_CSTRUCT_TABLE
};

static int set_timeval_sockopt (Socket_Type *s, int level, int optname)
{
   struct timeval tv;
   return set_struct_sockopt (s, level, optname, TV_Struct, (VOID_STAR)&tv, sizeof(struct timeval));
}

static int get_timeval_sockopt (Socket_Type *s, int level, int optname)
{
   struct timeval tv;
   return get_struct_sockopt (s, level, optname, TV_Struct, (VOID_STAR)&tv, sizeof(struct timeval));
}

#if defined(SO_LINGER)
static SLang_CStruct_Field_Type Linger_Struct [] =
{
   MAKE_CSTRUCT_INT_FIELD(struct linger, l_onoff, "l_onoff", 0),
   MAKE_CSTRUCT_INT_FIELD(struct linger, l_linger, "l_linger", 0),
   SLANG_END_CSTRUCT_TABLE
};

static int set_linger_sockopt (Socket_Type *s, int level, int optname)
{
   struct linger lg;
   return set_struct_sockopt (s, level, optname, Linger_Struct, (VOID_STAR)&lg, sizeof(struct linger));
}

static int get_linger_sockopt (Socket_Type *s, int level, int optname)
{
   struct linger lg;
   return get_struct_sockopt (s, level, optname, Linger_Struct, (VOID_STAR)&lg, sizeof(struct linger));
}
#endif

#ifdef SOL_SOCKET
static SockOpt_Type SO_Option_Table[] =
{
#ifdef SO_KEEPALIVE
     {SO_KEEPALIVE, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_OOBINLINE
     {SO_OOBINLINE, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_RCVLOWAT
     {SO_RCVLOWAT, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_SNDLOWAT
     {SO_SNDLOWAT, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_BSDCOMPAT
     {SO_BSDCOMPAT, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_PASSCRED
     {SO_PASSCRED, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_BINDTODEVICE
     {SO_BINDTODEVICE, set_str_sockopt, get_str_sockopt},
#endif
#ifdef SO_DEBUG
     {SO_DEBUG, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_REUSEADDR
     {SO_REUSEADDR, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_TYPE
     {SO_TYPE, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_ACCEPTCONN
     {SO_ACCEPTCONN, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_DONTROUTE
     {SO_DONTROUTE, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_BROADCAST
     {SO_BROADCAST, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_SNDBUF
     {SO_SNDBUF, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_RCVBUF
     {SO_RCVBUF, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_PRIORITY
     {SO_PRIORITY, set_int_sockopt, get_int_sockopt},
#endif
#ifdef SO_ERROR
     {SO_ERROR, NULL, get_int_sockopt},
#endif
#ifdef SO_PEERCRED
     /* {SO_PEERCRED, NULL, get_peercred_sockopt}, */
#endif
#ifdef SO_RCVTIMEO
     {SO_RCVTIMEO, set_timeval_sockopt, get_timeval_sockopt},
#endif
#ifdef SO_SNDTIMEO
     {SO_SNDTIMEO, set_timeval_sockopt, get_timeval_sockopt},
#endif
#ifdef SO_LINGER
     {SO_LINGER, set_linger_sockopt, get_linger_sockopt},
#endif

     {-1, NULL, NULL}
};
#endif				       /* SOL_SOCKET */

#ifdef SOL_IP
static SockOpt_Type IP_Option_Table[] =
{
#ifdef IP_OPTIONS
     /* {IP_OPTIONS, NULL, NULL}, */
#endif
#ifdef IP_PKTINFO
     /* {IP_PKTINFO, NULL, NULL}, */
#endif
#ifdef IP_RECVTOS
     /* {IP_RECVTOS, NULL, NULL}, */
#endif
#ifdef IP_RECVTTL
     /* {IP_RECVTTL, NULL, NULL}, */
#endif
#ifdef IP_RECVOPTS
     /* {IP_RECVOPTS, NULL, NULL}, */
#endif
#ifdef IP_TOS
     /* {IP_TOS, set_int_sockopt, get_int_sockopt}, */
#endif
#ifdef IP_TTL
     {IP_TTL, set_int_sockopt, get_int_sockopt},
#endif
#ifdef IP_HDRINCL
     {IP_HDRINCL, set_int_sockopt, get_int_sockopt},
#endif
#ifdef IP_RECVERR
     /* {IP_RECVERR, set_int_sockopt, get_int_sockopt}, */
#endif
#ifdef IP_MTU_DISCOVER
     /* {IP_MTU_DISCOVER, set_int_sockopt, get_int_sockopt}, */
#endif
#ifdef IP_MTU
     {IP_MTU_DISCOVER, NULL, get_int_sockopt},
#endif
#ifdef IP_ROUTER_ALERT
     {IP_ROUTER_ALERT, set_int_sockopt, get_int_sockopt},
#endif
#ifdef IP_MULTICAST_TTL
     {IP_MULTICAST_TTL, set_int_sockopt, get_int_sockopt},
#endif
#ifdef IP_MULTICAST_LOOP
     {IP_MULTICAST_LOOP, set_int_sockopt, get_int_sockopt},
#endif
#ifdef IP_ADD_MEMBERSHIP
     /* {IP_ADD_MEMBERSHIP, NULL, NULL}, */
#endif
#ifdef IP_DROP_MEMBERSHIP
     /* {IP_DROP_MEMBERSHIP, NULL, NULL}, */
#endif
#ifdef IP_MULTICAST_IF
     /* {IP_MULTICAST_IF, NULL, NULL}, */
#endif

     {-1, NULL, NULL}
};
#endif				       /* SOL_IP */
/* Usage: get/setsockopt (socket, level, optname, value) */
static void getset_sockopt (int set)
{
   Socket_Type *s;
   SLFile_FD_Type *f;
   int level, optname;
   SockOpt_Type *table;

   if (-1 == SLreverse_stack (3+set))
     return;

   if (NULL == (s = pop_socket (&f)))
     return;

   if ((-1 == SLang_pop_int (&level))
       || (-1 == SLang_pop_int (&optname)))
     {
	SLfile_free_fd (f);
	return;
     }

   switch (level)
     {
#ifdef SOL_SOCKET
      case SOL_SOCKET: table = SO_Option_Table; break;
#endif
#ifdef SOL_IP
      case SOL_IP: table = IP_Option_Table; break;
#endif
      default:
	SLang_verror (SL_NotImplemented_Error, "get/setsockopt level %d is not supported", level);
	goto free_return;
     }

   while (1)
     {
	if (table->optname == optname)
	  {
	     int (*func)(Socket_Type *, int, int);
	     if (set)
	       func = table->setopt;
	     else
	       func = table->getopt;
	     if (func == NULL)
	       goto not_implemented_error;

	     (void)(*func)(s, level, optname);
	     break;
	  }
	if (table->optname == -1)
	  goto free_return;

	table++;
     }

   /* drop */
   free_return:
   SLfile_free_fd (f);
   return;

   not_implemented_error:
   SLang_verror (SL_NotImplemented_Error, "get/setsockopt option %d is not supported at level %d", optname, level);
   SLfile_free_fd (f);
}

static void setsockopt_intrin (void)
{
   getset_sockopt (1);
}
static void getsockopt_intrin (void)
{
   getset_sockopt (0);
}

#define I SLANG_INT_TYPE
#define V SLANG_VOID_TYPE
#define F SLANG_FILE_FD_TYPE
static SLang_Intrin_Fun_Type Module_Intrinsics [] =
{
   MAKE_INTRINSIC_3("socket", socket_intrin, V, I, I, I),
#ifdef HAVE_SOCKETPAIR
   MAKE_INTRINSIC_3("socketpair", socketpair_intrin, V, I, I, I),
#endif
   MAKE_INTRINSIC_0("connect", connect_intrin, V),
   MAKE_INTRINSIC_0("bind", bind_intrin, V),
   MAKE_INTRINSIC_2("listen", listen_intrin, V, F, I),
   MAKE_INTRINSIC_0("accept", accept_intrin, V),
   MAKE_INTRINSIC_0("getsockopt", getsockopt_intrin, V),
   MAKE_INTRINSIC_0("setsockopt", setsockopt_intrin, V),
   SLANG_END_INTRIN_FUN_TABLE
};
#undef F
#undef V
#undef I

static SLang_IConstant_Type Module_IConstants [] =
{
#ifdef SOCK_STREAM
   MAKE_ICONSTANT("SOCK_STREAM", SOCK_STREAM),
#endif
#ifdef SOCK_DGRAM
   MAKE_ICONSTANT("SOCK_DGRAM", SOCK_DGRAM),
#endif
#ifdef SOCK_RAW
   MAKE_ICONSTANT("SOCK_RAW", SOCK_RAW),
#endif
#ifdef SOCK_RDM
   MAKE_ICONSTANT("SOCK_RDM", SOCK_RDM),
#endif
#ifdef SOCK_SEQPACKET
   MAKE_ICONSTANT("SOCK_SEQPACKET", SOCK_SEQPACKET),
#endif
#ifdef SOCK_PACKET
   MAKE_ICONSTANT("SOCK_PACKET", SOCK_PACKET),
#endif

/* Domains  */
#ifdef PF_UNIX
   MAKE_ICONSTANT("PF_UNIX", PF_UNIX),
#endif
#ifdef PF_INET
   MAKE_ICONSTANT("PF_INET", PF_INET),
#endif
#ifdef AF_UNIX
   MAKE_ICONSTANT("AF_UNIX", AF_UNIX),
#endif
#ifdef AF_INET
   MAKE_ICONSTANT("AF_INET", AF_INET),
#endif

#ifdef SOL_SOCKET
   MAKE_ICONSTANT("SOL_SOCKET", SOL_SOCKET),
# ifdef SO_KEEPALIVE
   MAKE_ICONSTANT("SO_KEEPALIVE", SO_KEEPALIVE),
# endif
# ifdef SO_OOBINLINE
   MAKE_ICONSTANT("SO_OOBINLINE", SO_OOBINLINE),
# endif
# ifdef SO_RCVLOWAT
   MAKE_ICONSTANT("SO_RCVLOWAT", SO_RCVLOWAT),
# endif
# ifdef SO_SNDLOWAT
   MAKE_ICONSTANT("SO_SNDLOWAT", SO_SNDLOWAT),
# endif
# ifdef SO_BSDCOMPAT
   MAKE_ICONSTANT("SO_BSDCOMPAT", SO_BSDCOMPAT),
# endif
# ifdef SO_PASSCRED
   MAKE_ICONSTANT("SO_PASSCRED", SO_PASSCRED),
# endif
# ifdef SO_BINDTODEVICE
   MAKE_ICONSTANT("SO_BINDTODEVICE", SO_BINDTODEVICE),
# endif
# ifdef SO_DEBUG
   MAKE_ICONSTANT("SO_DEBUG", SO_DEBUG),
# endif
# ifdef SO_REUSEADDR
   MAKE_ICONSTANT("SO_REUSEADDR", SO_REUSEADDR),
# endif
# ifdef SO_TYPE
   MAKE_ICONSTANT("SO_TYPE", SO_TYPE),
# endif
# ifdef SO_ACCEPTCONN
   MAKE_ICONSTANT("SO_ACCEPTCONN", SO_ACCEPTCONN),
# endif
# ifdef SO_DONTROUTE
   MAKE_ICONSTANT("SO_DONTROUTE", SO_DONTROUTE),
# endif
# ifdef SO_BROADCAST
   MAKE_ICONSTANT("SO_BROADCAST", SO_BROADCAST),
# endif
# ifdef SO_SNDBUF
   MAKE_ICONSTANT("SO_SNDBUF", SO_SNDBUF),
# endif
# ifdef SO_RCVBUF
   MAKE_ICONSTANT("SO_RCVBUF", SO_RCVBUF),
# endif
# ifdef SO_PRIORITY
   MAKE_ICONSTANT("SO_PRIORITY", SO_PRIORITY),
# endif
# ifdef SO_ERROR
   MAKE_ICONSTANT("SO_ERROR", SO_ERROR),
# endif
# ifdef SO_PEERCRED
   MAKE_ICONSTANT("SO_PEERCRED", SO_PEERCRED),
# endif
# ifdef SO_RCVTIMEO
   MAKE_ICONSTANT("SO_RCVTIMEO", SO_RCVTIMEO),
# endif
# ifdef SO_SNDTIMEO
   MAKE_ICONSTANT("SO_SNDTIMEO", SO_SNDTIMEO),
# endif
# ifdef SO_LINGER
   MAKE_ICONSTANT("SO_LINGER", SO_LINGER),
# endif
#endif   			       /* SOL_SOCKET */

#ifdef SOL_IP
   MAKE_ICONSTANT("SOL_IP", SOL_IP),
# ifdef IP_RECVTTL
   MAKE_ICONSTANT("IP_RECVTTL", IP_RECVTTL),
# endif
# ifdef IP_RECVOPTS
   MAKE_ICONSTANT("IP_RECVOPTS", IP_RECVOPTS),
# endif
# ifdef IP_RECVOPTS
   MAKE_ICONSTANT("IP_RECVOPTS", IP_RECVOPTS),
# endif
# ifdef IP_TOS
   MAKE_ICONSTANT("IP_TOS", IP_TOS),
# endif
# ifdef IP_TTL
   MAKE_ICONSTANT("IP_TTL", IP_TTL),
# endif
# ifdef IP_HDRINCL
   MAKE_ICONSTANT("IP_HDRINCL", IP_HDRINCL),
# endif
# ifdef IP_RECVERR
   MAKE_ICONSTANT("IP_RECVERR", IP_RECVERR),
# endif
# ifdef IP_MTU_DISCOVER
   MAKE_ICONSTANT("IP_MTU_DISCOVER", IP_MTU_DISCOVER),
# endif
# ifdef IP_ROUTER_ALERT
   MAKE_ICONSTANT("IP_ROUTER_ALERT", IP_ROUTER_ALERT),
# endif
# ifdef IP_MULTICAST_TTL
   MAKE_ICONSTANT("IP_MULTICAST_TTL", IP_MULTICAST_TTL),
# endif
# ifdef IP_MULTICAST_LOOP
   MAKE_ICONSTANT("IP_MULTICAST_LOOP", IP_MULTICAST_LOOP),
# endif
# ifdef IP_ADD_MEMBERSHIP
   MAKE_ICONSTANT("IP_ADD_MEMBERSHIP", IP_ADD_MEMBERSHIP),
# endif
# ifdef IP_DROP_MEMBERSHIP
   MAKE_ICONSTANT("IP_DROP_MEMBERSHIP", IP_DROP_MEMBERSHIP),
# endif
# ifdef IP_MULTICAST_IF
   MAKE_ICONSTANT("IP_MULTICAST_IF", IP_MULTICAST_IF),
# endif
# ifdef IP_OPTIONS
   MAKE_ICONSTANT("IP_OPTIONS", IP_OPTIONS),
# endif
# ifdef IP_PKTINFO
   MAKE_ICONSTANT("IP_PKTINFO", IP_PKTINFO),
# endif
# ifdef IP_RECVTOS
   MAKE_ICONSTANT("IP_RECVTOS", IP_RECVTOS),
# endif
#endif				       /* SOL_IP */

   SLANG_END_ICONST_TABLE
};

int init_socket_module_ns (char *ns_name)
{
   SLang_NameSpace_Type *ns;

   if (SocketError == -1)
     {
	if (-1 == (SocketError = SLerr_new_exception (SL_RunTime_Error, "SocketError", "Socket Error")))
	  return -1;
	if (-1 == (SocketHerrnoError = SLerr_new_exception (SocketError, "SocketHError", "Socket h_errno Error")))
	  return -1;
     }
   if (Socket_Type_Id == -1)
     {
	(void) SLfile_create_clientdata_id (&Socket_Type_Id);
     }

   if (NULL == (ns = SLns_create_namespace (ns_name)))
     return -1;

    if ((-1 == SLns_add_intrin_fun_table (ns, Module_Intrinsics, NULL))
        || (-1 == SLns_add_iconstant_table (ns, Module_IConstants, NULL)))
     return -1;

   if (-1 == SLns_add_intrinsic_variable(ns, "h_errno", (VOID_STAR)&Module_H_Errno, SLANG_INT_TYPE, 1))
     return -1;

   return 0;
}

void deinit_socket_module (void)
{
}
