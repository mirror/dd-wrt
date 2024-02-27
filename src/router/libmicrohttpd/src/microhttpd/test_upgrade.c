/*
     This file is part of libmicrohttpd
     Copyright (C) 2016-2020 Christian Grothoff
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

     libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 3, or (at your
     option) any later version.

     libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @file test_upgrade.c
 * @brief  Testcase for libmicrohttpd upgrading a connection
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#ifndef WINDOWS
#include <unistd.h>
#endif
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */

#include "mhd_sockets.h"
#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif /* HAVE_NETINET_IP_H */

#include "platform.h"
#include "microhttpd.h"

#include "test_helpers.h"

#ifdef HTTPS_SUPPORT
#include <gnutls/gnutls.h>
#include "../testcurl/https/tls_test_keys.h"

#if defined(HAVE_FORK) && defined(HAVE_WAITPID)
#include <sys/types.h>
#include <sys/wait.h>
#endif /* HAVE_FORK && HAVE_WAITPID */
#endif /* HTTPS_SUPPORT */

#if defined(MHD_POSIX_SOCKETS)
#  ifdef MHD_WINSOCK_SOCKETS
#    error Both MHD_POSIX_SOCKETS and MHD_WINSOCK_SOCKETS are defined
#  endif /* MHD_WINSOCK_SOCKETS */
#elif ! defined(MHD_WINSOCK_SOCKETS)
#  error Neither MHD_POSIX_SOCKETS nor MHD_WINSOCK_SOCKETS are defined
#endif /* MHD_WINSOCK_SOCKETS */


#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */


_MHD_NORETURN static void
_externalErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "System or external library call failed");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  fflush (stderr);
  exit (99);
}


_MHD_NORETURN static void
_mhdErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "MHD unexpected error");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));

  fflush (stderr);
  exit (8);
}


static void
_testErrorLog_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "System or external library call resulted in error");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  fflush (stderr);
}


#ifdef MHD_HAVE_MHD_FUNC_
#define externalErrorExit(ignore) \
    _externalErrorExit_func(NULL, MHD_FUNC_, __LINE__)
#define externalErrorExitDesc(errDesc) \
    _externalErrorExit_func(errDesc, MHD_FUNC_, __LINE__)
#define mhdErrorExit(ignore) \
    _mhdErrorExit_func(NULL, MHD_FUNC_, __LINE__)
#define mhdErrorExitDesc(errDesc) \
    _mhdErrorExit_func(errDesc, MHD_FUNC_, __LINE__)
#define testErrorLog(ignore) \
    _testErrorLog_func(NULL, MHD_FUNC_, __LINE__)
#define testErrorLogDesc(errDesc) \
    _testErrorLog_func(errDesc, MHD_FUNC_, __LINE__)
#else  /* ! MHD_HAVE_MHD_FUNC_ */
#define externalErrorExit(ignore) _externalErrorExit_func(NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func(errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func(NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func(errDesc, NULL, __LINE__)
#define testErrorLog(ignore) _testErrorLog_func(NULL, NULL, __LINE__)
#define testErrorLogDesc(errDesc) _testErrorLog_func(errDesc, NULL, __LINE__)
#endif /* ! MHD_HAVE_MHD_FUNC_ */

/* ** External parameters ** */
static bool use_large;

static bool use_vlarge;

static bool test_tls;

static int verbose = 0;

enum tls_tool
{
  TLS_CLI_NO_TOOL = 0,
  TLS_CLI_GNUTLS,
  TLS_CLI_OPENSSL,
  TLS_LIB_GNUTLS
};

static enum tls_tool use_tls_tool;


/* ** Internal values ** */

/* Could be increased to facilitate debugging */
static int test_timeout = 5;

static uint16_t global_port;

static const void *rclient_msg;

static size_t rclient_msg_size;

static const void *app_msg;

static size_t app_msg_size;

static void *alloc_ptr[2] = {NULL, NULL};


static void
fflush_allstd (void)
{
  fflush (stderr);
  fflush (stdout);
}


#if defined(HTTPS_SUPPORT) && defined(HAVE_FORK) && defined(HAVE_WAITPID)
/**
 * Fork child that connects via GnuTLS-CLI to our @a port.  Allows us to
 * talk to our port over a socket in @a sp without having to worry
 * about TLS.
 *
 * @param location where the socket is returned
 * @return -1 on error, otherwise PID of TLS child process
 */
static pid_t
gnutlscli_connect (int *sock,
                   uint16_t port)
{
  pid_t chld;
  int sp[2];
  char destination[30];

  if (0 != socketpair (AF_UNIX,
                       SOCK_STREAM,
                       0,
                       sp))
  {
    testErrorLogDesc ("socketpair() failed");
    return (pid_t) -1;
  }
  chld = fork ();
  if (0 != chld)
  {
    *sock = sp[1];
    MHD_socket_close_chk_ (sp[0]);
    return chld;
  }
  MHD_socket_close_chk_ (sp[1]);
  (void) close (0);
  (void) close (1);
  if (-1 == dup2 (sp[0], 0))
    externalErrorExitDesc ("dup2() failed");
  if (-1 == dup2 (sp[0], 1))
    externalErrorExitDesc ("dup2() failed");
  MHD_socket_close_chk_ (sp[0]);
  if (TLS_CLI_GNUTLS == use_tls_tool)
  {
    snprintf (destination,
              sizeof(destination),
              "%u",
              (unsigned int) port);
    execlp ("gnutls-cli",
            "gnutls-cli",
            "--insecure",
            "-p",
            destination,
            "127.0.0.1",
            (char *) NULL);
  }
  else if (TLS_CLI_OPENSSL == use_tls_tool)
  {
    snprintf (destination,
              sizeof(destination),
              "127.0.0.1:%u",
              (unsigned int) port);
    execlp ("openssl",
            "openssl",
            "s_client",
            "-connect",
            destination,
            "-verify",
            "1",
            (char *) NULL);
  }
  _exit (1);
}


#endif /* HTTPS_SUPPORT && HAVE_FORK && HAVE_WAITPID */


#if 0 /* Unused code */
/**
 * Change socket to blocking.
 *
 * @param fd the socket to manipulate
 */
static void
make_blocking (MHD_socket fd)
{
#if defined(MHD_POSIX_SOCKETS)
  int flags;

  flags = fcntl (fd, F_GETFL);
  if (-1 == flags)
    externalErrorExitDesc ("fcntl() failed");
  if ((flags & ~O_NONBLOCK) != flags)
    if (-1 == fcntl (fd, F_SETFL, flags & ~O_NONBLOCK))
      externalErrorExitDesc ("fcntl() failed");
#elif defined(MHD_WINSOCK_SOCKETS)
  unsigned long flags = 0;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    externalErrorExitDesc ("ioctlsocket() failed");
#endif /* MHD_WINSOCK_SOCKETS */
}


#endif /* Unused code */


/**
 * Change socket to non-blocking.
 *
 * @param fd the socket to manipulate
 */
static void
make_nonblocking (MHD_socket fd)
{
#if defined(MHD_POSIX_SOCKETS)
  int flags;

  flags = fcntl (fd, F_GETFL);
  if (-1 == flags)
    externalErrorExitDesc ("fcntl() failed");
  if (O_NONBLOCK != (flags & O_NONBLOCK))
    if (-1 == fcntl (fd, F_SETFL, flags | O_NONBLOCK))
      externalErrorExitDesc ("fcntl() failed");
#elif defined(MHD_WINSOCK_SOCKETS)
  unsigned long flags = 1;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    externalErrorExitDesc ("ioctlsocket() failed");
#endif /* MHD_WINSOCK_SOCKETS */
}


/**
 * Enable TCP_NODELAY on TCP/IP socket.
 *
 * @param fd the socket to manipulate
 */
static void
make_nodelay (MHD_socket fd)
{
#ifdef TCP_NODELAY
  const MHD_SCKT_OPT_BOOL_ on_val = 1;

  if (0 == setsockopt (fd,
                       IPPROTO_TCP,
                       TCP_NODELAY,
                       (const void *) &on_val,
                       sizeof (on_val)))
    return; /* Success exit point */

#ifndef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "Failed to enable TCP_NODELAY on socket (ignored). "
           "errno: %d (%s)\n", (int) errno, strerror (errno));
#else /* MHD_WINSOCK_SOCKETS */
  fprintf (stderr, "Failed to enable TCP_NODELAY on socket (ignored). "
           "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  fflush (stderr);
#endif /* TCP_NODELAY */
}


/**
 * Wrapper structure for plain&TLS sockets
 */
struct wr_socket
{
  /**
   * Real network socket
   */
  MHD_socket fd;

  /**
   * Type of this socket
   */
  enum wr_type
  {
    wr_invalid = 0,
    wr_plain = 1,
    wr_tls = 2
  } t;

  bool is_nonblocking;

  bool eof_recieved;
#ifdef HTTPS_SUPPORT
  /**
   * TLS credentials
   */
  gnutls_certificate_credentials_t tls_crd;

  /**
   * TLS session.
   */
  gnutls_session_t tls_s;

  /**
   * TLS handshake already succeed?
   */
  bool tls_connected;
#endif
};


/**
 * Get underlying real socket.
 * @return FD of real socket
 */
#define wr_fd(s) ((s)->fd)


#if 0 /* Unused code */
static void
wr_make_blocking  (struct wr_socket *s)
{
  if (s->is_nonblocking)
    make_blocking (s->fd);
  s->is_nonblocking = false;
}


#endif /* Unused code */


static void
wr_make_nonblocking  (struct wr_socket *s)
{
  if (! s->is_nonblocking)
    make_nonblocking (s->fd);
  s->is_nonblocking = true;
}


/**
 * Create wr_socket with plain TCP underlying socket
 * @return created socket on success, NULL otherwise
 */
static struct wr_socket *
wr_create_plain_sckt (void)
{
  struct wr_socket *s = malloc (sizeof(struct wr_socket));
  if (NULL == s)
  {
    testErrorLogDesc ("malloc() failed");
    return NULL;
  }
  s->t = wr_plain;
  s->eof_recieved = false;
  s->fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  s->is_nonblocking = false;
  if (MHD_INVALID_SOCKET != s->fd)
  {
    make_nodelay (s->fd);
    return s; /* Success */
  }
  testErrorLogDesc ("socket() failed");
  free (s);
  return NULL;
}


/**
 * Create wr_socket with TLS TCP underlying socket
 * @return created socket on success, NULL otherwise
 */
static struct wr_socket *
wr_create_tls_sckt (void)
{
#ifdef HTTPS_SUPPORT
  struct wr_socket *s = malloc (sizeof(struct wr_socket));
  if (NULL == s)
  {
    testErrorLogDesc ("malloc() failed");
    return NULL;
  }
  s->t = wr_tls;
  s->eof_recieved = false;
  s->tls_connected = 0;
  s->fd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  s->is_nonblocking = false;
  if (MHD_INVALID_SOCKET != s->fd)
  {
    make_nodelay (s->fd);
    if (GNUTLS_E_SUCCESS == gnutls_init (&(s->tls_s), GNUTLS_CLIENT))
    {
      if (GNUTLS_E_SUCCESS == gnutls_set_default_priority (s->tls_s))
      {
        if (GNUTLS_E_SUCCESS ==
            gnutls_certificate_allocate_credentials (&(s->tls_crd)))
        {
          if (GNUTLS_E_SUCCESS == gnutls_credentials_set (s->tls_s,
                                                          GNUTLS_CRD_CERTIFICATE,
                                                          s->tls_crd))
          {
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030109) && ! defined(_WIN64)
            gnutls_transport_set_int (s->tls_s, (int) (s->fd));
#else  /* GnuTLS before 3.1.9 or Win x64 */
            gnutls_transport_set_ptr (s->tls_s,
                                      (gnutls_transport_ptr_t) \
                                      (intptr_t) (s->fd));
#endif /* GnuTLS before 3.1.9 or Win x64 */
            return s;
          }
          else
            testErrorLogDesc ("gnutls_credentials_set() failed");
          gnutls_certificate_free_credentials (s->tls_crd);
        }
        else
          testErrorLogDesc ("gnutls_certificate_allocate_credentials() failed");
      }
      else
        testErrorLogDesc ("gnutls_set_default_priority() failed");
      gnutls_deinit (s->tls_s);
    }
    else
      testErrorLogDesc ("gnutls_init() failed");
    (void) MHD_socket_close_ (s->fd);
  }
  else
    testErrorLogDesc ("socket() failed");
  free (s);
#endif /* HTTPS_SUPPORT */
  return NULL;
}


/**
 * Create wr_socket with plain TCP underlying socket
 * from already created TCP socket.
 * @param plain_sk real TCP socket
 * @return created socket on success, NULL otherwise
 */
static struct wr_socket *
wr_create_from_plain_sckt (MHD_socket plain_sk)
{
  struct wr_socket *s = malloc (sizeof(struct wr_socket));

  if (NULL == s)
  {
    testErrorLogDesc ("malloc() failed");
    return NULL;
  }
  s->t = wr_plain;
  s->eof_recieved = false;
  s->fd = plain_sk;
  s->is_nonblocking = false; /* The actual mode is unknown */
  wr_make_nonblocking (s);   /* Force set mode to have correct status */
  make_nodelay (s->fd);
  return s;
}


#if 0 /* Disabled code */
/**
 * Check whether shutdown of connection was received from remote
 * @param s socket to check
 * @return zero if shutdown signal has not been received,
 *         1 if shutdown signal was already received
 */
static int
wr_is_eof_received (struct wr_socket *s)
{
  return s->eof_recieved ? 1 : 0;
}


#endif /* Disabled code */


enum wr_wait_for_type
{
  WR_WAIT_FOR_RECV = 0,
  WR_WAIT_FOR_SEND = 1
};

static bool
wr_wait_socket_ready_noabort_ (struct wr_socket *s,
                               int timeout_ms,
                               enum wr_wait_for_type wait_for)
{
  fd_set fds;
  int sel_res;
  struct timeval tmo;
  struct timeval *tmo_ptr;

#ifndef MHD_WINSOCK_SOCKETS
  if (FD_SETSIZE <= s->fd)
    externalErrorExitDesc ("Too large FD value");
#endif /* ! MHD_WINSOCK_SOCKETS */
  FD_ZERO (&fds);
  FD_SET (s->fd, &fds);
  if (0 <= timeout_ms)
  {
#if ! defined(_WIN32) || defined(__CYGWIN__)
    tmo.tv_sec = (time_t) (timeout_ms / 1000);
#else  /* Native W32 */
    tmo.tv_sec = (long) (timeout_ms / 1000);
#endif /* Native W32 */
    tmo.tv_usec = ((long) (timeout_ms % 1000)) * 1000;
    tmo_ptr = &tmo;
  }
  else
    tmo_ptr = NULL; /* No timeout */

  do
  {
    if (WR_WAIT_FOR_RECV == wait_for)
      sel_res = select (1 + (int) s->fd, &fds, NULL, NULL, tmo_ptr);
    else
      sel_res = select (1 + (int) s->fd, NULL, &fds, NULL, tmo_ptr);
  } while (0 > sel_res && MHD_SCKT_ERR_IS_EINTR_ (MHD_socket_get_error_ ()));

  if (1 == sel_res)
    return true;

  if (0 == sel_res)
    fprintf (stderr, "Timeout");
  else
  {
#ifndef MHD_WINSOCK_SOCKETS
    fprintf (stderr, "Error %d (%s)", (int) errno, strerror (errno));
#else /* MHD_WINSOCK_SOCKETS */
    fprintf (stderr, "Error (WSAGetLastError code: %d)",
             (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  }
  fprintf (stderr, " waiting for socket to be available for %s.\n",
           (WR_WAIT_FOR_RECV == wait_for) ? "receiving" : "sending");
  return false;
}


static void
wr_wait_socket_ready_ (struct wr_socket *s,
                       int timeout_ms,
                       enum wr_wait_for_type wait_for)
{
  if (wr_wait_socket_ready_noabort_ (s, timeout_ms, wait_for))
    return;

  if (WR_WAIT_FOR_RECV == wait_for)
    mhdErrorExitDesc ("Client or application failed to receive the data");
  else
    mhdErrorExitDesc ("Client or application failed to send the data");
}


/**
 * Connect socket to specified address.
 * @param s socket to use
 * @param addr address to connect
 * @param length of structure pointed by @a addr
 * @param timeout_ms the maximum wait time in milliseconds to send the data,
 *                   no limit if negative value is used
 * @return zero on success, -1 otherwise.
 */
static int
wr_connect_tmo (struct wr_socket *s,
                const struct sockaddr *addr,
                unsigned int length,
                int timeout_ms)
{
  if (0 != connect (s->fd, addr, (socklen_t) length))
  {
    int err;
    bool connect_completed = false;

    err = MHD_socket_get_error_ ();
#if defined(MHD_POSIX_SOCKETS)
    while (! connect_completed && (EINTR == err))
    {
      connect_completed = (0 == connect (s->fd, addr, (socklen_t) length));
      if (! connect_completed)
      {
        err = errno;
        if (EALREADY == err)
          err = EINPROGRESS;
        else if (EISCONN == err)
          connect_completed = true;
      }
    }
#endif /* MHD_POSIX_SOCKETS */
    if (! connect_completed &&
        (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EINPROGRESS_)
         || MHD_SCKT_ERR_IS_EAGAIN_ (err))) /* No modern system uses EAGAIN, except W32 */
      connect_completed =
        wr_wait_socket_ready_noabort_ (s, timeout_ms, WR_WAIT_FOR_SEND);
    if (! connect_completed)
    {
      testErrorLogDesc ("connect() failed");
      return -1;
    }
  }
  if (wr_plain == s->t)
    return 0;
#ifdef HTTPS_SUPPORT
  if (wr_tls == s->t)
  {
    /* Do not try handshake here as
     * it requires processing on MHD side and
     * when testing with "external" polling,
     * test will call MHD processing only
     * after return from wr_connect(). */
    s->tls_connected = 0;
    return 0;
  }
#endif /* HTTPS_SUPPORT */
  testErrorLogDesc ("HTTPS socket connect called, but code does not support" \
                    " HTTPS sockets");
  return -1;
}


/**
 * Connect socket to specified address.
 * @param s socket to use
 * @param addr address to connect
 * @param length of structure pointed by @a addr
 * @return zero on success, -1 otherwise.
 */
static int
wr_connect (struct wr_socket *s,
            const struct sockaddr *addr,
            unsigned int length)
{
  return wr_connect_tmo (s, addr, length, test_timeout * 1000);
}


#ifdef HTTPS_SUPPORT
/* Only to be called from wr_send() and wr_recv() ! */
static bool
wr_handshake_tmo_ (struct wr_socket *s,
                   int timeout_ms)
{
  int res = gnutls_handshake (s->tls_s);

  while ((GNUTLS_E_AGAIN == res) || (GNUTLS_E_INTERRUPTED  == res))
  {
    wr_wait_socket_ready_ (s, timeout_ms,
                           gnutls_record_get_direction (s->tls_s) ?
                           WR_WAIT_FOR_SEND : WR_WAIT_FOR_RECV);
    res = gnutls_handshake (s->tls_s);
  }
  if (GNUTLS_E_SUCCESS == res)
    s->tls_connected = true;
  else
  {
    fprintf (stderr, "The error returned by gnutls_handshake() is "
             "'%s' ", gnutls_strerror ((int) res));
#if GNUTLS_VERSION_NUMBER >= 0x020600
    fprintf (stderr, "(%s)\n", gnutls_strerror_name ((int) res));
#else  /* GNUTLS_VERSION_NUMBER < 0x020600 */
    fprintf (stderr, "(%d)\n", (int) res);
#endif /* GNUTLS_VERSION_NUMBER < 0x020600 */
    testErrorLogDesc ("gnutls_handshake() failed with hard error");
    MHD_socket_set_error_ (MHD_SCKT_ECONNABORTED_); /* hard error */
  }
  return s->tls_connected;
}


#if 0 /* Unused function */
/* Only to be called from wr_send() and wr_recv() ! */
static bool
wr_handshake_ (struct wr_socket *s)
{
  return wr_handshake_tmo_ (s, test_timeout * 1000);
}


#endif /* Unused function */

#endif /* HTTPS_SUPPORT */


/**
 * Send data to remote by socket.
 * @param s the socket to use
 * @param buf the buffer with data to send
 * @param len the length of data in @a buf
 * @param timeout_ms the maximum wait time in milliseconds to send the data,
 *                   no limit if negative value is used
 * @return number of bytes were sent if succeed,
 *         -1 if failed. Use #MHD_socket_get_error_()
 *         to get socket error.
 */
static ssize_t
wr_send_tmo (struct wr_socket *s,
             const void *buf,
             size_t len,
             int timeout_ms)
{
  if (wr_plain == s->t)
  {
    ssize_t res;
    while (! 0)
    {
      int err;
      res = MHD_send_ (s->fd, buf, len);
      if (0 <= res)
        break; /* Success */
      err = MHD_socket_get_error_ ();
      if (! MHD_SCKT_ERR_IS_EAGAIN_ (err) && ! MHD_SCKT_ERR_IS_EINTR_ (err))
        break; /* Failure */
      wr_wait_socket_ready_ (s, timeout_ms, WR_WAIT_FOR_SEND);
    }
    return res;
  }
#ifdef HTTPS_SUPPORT
  else if (wr_tls == s->t)
  {
    ssize_t ret;
    if (! s->tls_connected && ! wr_handshake_tmo_ (s, timeout_ms))
      return -1;

    while (1)
    {
      ret = gnutls_record_send (s->tls_s, buf, len);
      if (ret >= 0)
        return ret;
      if ((GNUTLS_E_AGAIN != ret) && (GNUTLS_E_INTERRUPTED  != ret))
        break;
      wr_wait_socket_ready_ (s, timeout_ms,
                             gnutls_record_get_direction (s->tls_s) ?
                             WR_WAIT_FOR_SEND : WR_WAIT_FOR_RECV);
    }
    fprintf (stderr, "The error returned by gnutls_record_send() is "
             "'%s' ", gnutls_strerror ((int) ret));
#if GNUTLS_VERSION_NUMBER >= 0x020600
    fprintf (stderr, "(%s)\n", gnutls_strerror_name ((int) ret));
#else  /* GNUTLS_VERSION_NUMBER < 0x020600 */
    fprintf (stderr, "(%d)\n", (int) ret);
#endif /* GNUTLS_VERSION_NUMBER < 0x020600 */
    testErrorLogDesc ("gnutls_record_send() failed with hard error");
    MHD_socket_set_error_ (MHD_SCKT_ECONNABORTED_);   /* hard error */
    return -1;
  }
#endif /* HTTPS_SUPPORT */
  testErrorLogDesc ("HTTPS socket send called, but code does not support" \
                    " HTTPS sockets");
  return -1;
}


/**
 * Send data to remote by socket.
 * @param s the socket to use
 * @param buf the buffer with data to send
 * @param len the length of data in @a buf
 * @return number of bytes were sent if succeed,
 *         -1 if failed. Use #MHD_socket_get_error_()
 *         to get socket error.
 */
static ssize_t
wr_send (struct wr_socket *s,
         const void *buf,
         size_t len)
{
  return wr_send_tmo (s, buf, len, test_timeout * 1000);
}


/**
 * Receive data from remote by socket.
 * @param s the socket to use
 * @param buf the buffer to store received data
 * @param len the length of @a buf
 * @param timeout_ms the maximum wait time in milliseconds to receive the data,
 *                   no limit if negative value is used
 * @return number of bytes were received if succeed,
 *         -1 if failed. Use #MHD_socket_get_error_()
 *         to get socket error.
 */
static ssize_t
wr_recv_tmo (struct wr_socket *s,
             void *buf,
             size_t len,
             int timeout_ms)
{
  if (wr_plain == s->t)
  {
    ssize_t res;
    while (! 0)
    {
      int err;
      res = MHD_recv_ (s->fd, buf, len);
      if (0 == res)
        s->eof_recieved = true;
      if (0 <= res)
        break; /* Success */
      err = MHD_socket_get_error_ ();
      if (! MHD_SCKT_ERR_IS_EAGAIN_ (err) && ! MHD_SCKT_ERR_IS_EINTR_ (err))
        break; /* Failure */
      wr_wait_socket_ready_ (s, timeout_ms, WR_WAIT_FOR_RECV);
    }
    return res;
  }
#ifdef HTTPS_SUPPORT
  if (wr_tls == s->t)
  {
    ssize_t ret;
    if (! s->tls_connected && ! wr_handshake_tmo_ (s, timeout_ms))
      return -1;

    while (1)
    {
      ret = gnutls_record_recv (s->tls_s, buf, len);
      if (0 == ret)
        s->eof_recieved = true;
      if (ret >= 0)
        return ret;
      if ((GNUTLS_E_AGAIN != ret) && (GNUTLS_E_INTERRUPTED  != ret))
        break;
      wr_wait_socket_ready_ (s, timeout_ms,
                             gnutls_record_get_direction (s->tls_s) ?
                             WR_WAIT_FOR_SEND : WR_WAIT_FOR_RECV);
    }

    fprintf (stderr, "The error returned by gnutls_record_recv() is "
             "'%s' ", gnutls_strerror ((int) ret));
#if GNUTLS_VERSION_NUMBER >= 0x020600
    fprintf (stderr, "(%s)\n", gnutls_strerror_name ((int) ret));
#else  /* GNUTLS_VERSION_NUMBER < 0x020600 */
    fprintf (stderr, "(%d)\n", (int) ret);
#endif /* GNUTLS_VERSION_NUMBER < 0x020600 */
    testErrorLogDesc ("gnutls_record_recv() failed with hard error");
    MHD_socket_set_error_ (MHD_SCKT_ECONNABORTED_);   /* hard error */
    return -1;
  }
#endif /* HTTPS_SUPPORT */
  return -1;
}


/**
 * Receive data from remote by socket.
 * @param s the socket to use
 * @param buf the buffer to store received data
 * @param len the length of @a buf
 * @return number of bytes were received if succeed,
 *         -1 if failed. Use #MHD_socket_get_error_()
 *         to get socket error.
 */
static ssize_t
wr_recv (struct wr_socket *s,
         void *buf,
         size_t len)
{
  return wr_recv_tmo (s, buf, len, test_timeout * 1000);
}


/**
 * Shutdown send/write on the socket.
 * @param s the socket to shutdown
 * @param how the type of shutdown: SHUT_WR or SHUT_RDWR
 * @param timeout_ms the maximum wait time in milliseconds to receive the data,
 *                   no limit if negative value is used
 * @return zero on succeed, -1 otherwise
 */
static int
wr_shutdown_tmo (struct wr_socket *s, int how, int timeout_ms)
{
  switch (how)
  {
  case SHUT_WR: /* Valid value */
    break;
  case SHUT_RDWR: /* Valid value */
    break;
  case SHUT_RD:
    externalErrorExitDesc ("Unsupported 'how' value");
    break;
  default:
    externalErrorExitDesc ("Invalid 'how' value");
    break;
  }
  if (wr_plain == s->t)
  {
    (void) timeout_ms; /* Unused parameter for plain sockets */
    return shutdown (s->fd, how);
  }
#ifdef HTTPS_SUPPORT
  if (wr_tls == s->t)
  {
    ssize_t ret;
    if (! s->tls_connected && ! wr_handshake_tmo_ (s, timeout_ms))
      return -1;

    while (1)
    {
      ret =
        gnutls_bye  (s->tls_s,
                     (SHUT_WR == how) ?  GNUTLS_SHUT_WR :  GNUTLS_SHUT_RDWR);
      if (GNUTLS_E_SUCCESS == ret)
      {
#if 0 /* Disabled to test pure behaviour */
        if (SHUT_RDWR == how)
          (void) shutdown (s->fd, how); /* Also shutdown the underlying transport layer */
#endif
        return 0;
      }
      if ((GNUTLS_E_AGAIN != ret) && (GNUTLS_E_INTERRUPTED  != ret))
        break;
      wr_wait_socket_ready_ (s, timeout_ms,
                             gnutls_record_get_direction (s->tls_s) ?
                             WR_WAIT_FOR_SEND : WR_WAIT_FOR_RECV);
    }

    fprintf (stderr, "The error returned by gnutls_bye() is "
             "'%s' ", gnutls_strerror ((int) ret));
#if GNUTLS_VERSION_NUMBER >= 0x020600
    fprintf (stderr, "(%s)\n", gnutls_strerror_name ((int) ret));
#else  /* GNUTLS_VERSION_NUMBER < 0x020600 */
    fprintf (stderr, "(%d)\n", (int) ret);
#endif /* GNUTLS_VERSION_NUMBER < 0x020600 */
    testErrorLogDesc ("gnutls_bye() failed with hard error");
    MHD_socket_set_error_ (MHD_SCKT_ECONNABORTED_);   /* hard error */
    return -1;
  }
#endif /* HTTPS_SUPPORT */
  return -1;
}


/**
 * Shutdown the socket.
 * @param s the socket to shutdown
 * @return zero on succeed, -1 otherwise
 */
static int
wr_shutdown (struct wr_socket *s, int how)
{
  return wr_shutdown_tmo (s, how, test_timeout * 1000);
}


/**
 * Close socket and release allocated resourced
 * @param s the socket to close
 * @return zero on succeed, -1 otherwise
 */
static int
wr_close (struct wr_socket *s)
{
  int ret = (MHD_socket_close_ (s->fd)) ? 0 : -1;
#ifdef HTTPS_SUPPORT
  if (wr_tls == s->t)
  {
    gnutls_deinit (s->tls_s);
    gnutls_certificate_free_credentials (s->tls_crd);
  }
#endif /* HTTPS_SUPPORT */
  free (s);
  return ret;
}


/**
 * Thread we use to run the interaction with the upgraded socket.
 */
static pthread_t pt;

/**
 * Will be set to the upgraded socket.
 */
static struct wr_socket *volatile usock;

/**
 * Thread we use to run the interaction with the upgraded socket.
 */
static pthread_t pt_client;

/**
 * Flag set to true once the client is finished.
 */
static volatile bool client_done;

/**
 * Flag set to true once the app is finished.
 */
static volatile bool app_done;


static const char *
term_reason_str (enum MHD_RequestTerminationCode term_code)
{
  switch ((int) term_code)
  {
  case MHD_REQUEST_TERMINATED_COMPLETED_OK:
    return "COMPLETED_OK";
  case MHD_REQUEST_TERMINATED_WITH_ERROR:
    return "TERMINATED_WITH_ERROR";
  case MHD_REQUEST_TERMINATED_TIMEOUT_REACHED:
    return "TIMEOUT_REACHED";
  case MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN:
    return "DAEMON_SHUTDOWN";
  case MHD_REQUEST_TERMINATED_READ_ERROR:
    return "READ_ERROR";
  case MHD_REQUEST_TERMINATED_CLIENT_ABORT:
    return "CLIENT_ABORT";
  case -1:
    return "(not called)";
  default:
    return "(unknown code)";
  }
  return "(problem)"; /* unreachable */
}


/**
 * Callback used by MHD to notify the application about completed
 * requests.  Frees memory.
 *
 * @param cls client-defined closure
 * @param connection connection handle
 * @param req_cls value as set by the last call to
 *        the #MHD_AccessHandlerCallback
 * @param toe reason for request termination
 */
static void
notify_completed_cb (void *cls,
                     struct MHD_Connection *connection,
                     void **req_cls,
                     enum MHD_RequestTerminationCode toe)
{
  (void) cls;
  (void) connection;  /* Unused. Silent compiler warning. */
  if (verbose)
    printf ("notify_completed_cb() has been called with '%s' code.\n",
            term_reason_str (toe));
  if ( (toe != MHD_REQUEST_TERMINATED_COMPLETED_OK) &&
       (toe != MHD_REQUEST_TERMINATED_CLIENT_ABORT) &&
       (toe != MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN) )
    mhdErrorExitDesc ("notify_completed_cb() called with wrong code");
  if (NULL == req_cls)
    mhdErrorExitDesc ("'req_cls' parameter is NULL");
  if (NULL == *req_cls)
    mhdErrorExitDesc ("'*req_cls' pointer is NULL");
  if (! pthread_equal (**((pthread_t **) req_cls),
                       pthread_self ()))
    mhdErrorExitDesc ("notify_completed_cb() is called in wrong thread");
  free (*req_cls);
  *req_cls = NULL;
}


/**
 * Logging callback.
 *
 * @param cls logging closure (NULL)
 * @param uri access URI
 * @param connection connection handle
 * @return #TEST_PTR
 */
static void *
log_cb (void *cls,
        const char *uri,
        struct MHD_Connection *connection)
{
  pthread_t *ppth;

  (void) cls;
  (void) connection;  /* Unused. Silent compiler warning. */
  if (NULL == uri)
    mhdErrorExitDesc ("The 'uri' parameter is NULL");
  if (0 != strcmp (uri, "/"))
  {
    fprintf (stderr, "Wrong 'uri' value: '%s'. ", uri);
    mhdErrorExit ();
  }
  ppth = malloc (sizeof (pthread_t));
  if (NULL == ppth)
    externalErrorExitDesc ("malloc() failed");
  *ppth = pthread_self ();
  return (void *) ppth;
}


/**
 * Function to check that MHD properly notifies about starting
 * and stopping.
 *
 * @param cls client-defined closure
 * @param connection connection handle
 * @param socket_context socket-specific pointer where the
 *                       client can associate some state specific
 *                       to the TCP connection; note that this is
 *                       different from the "req_cls" which is per
 *                       HTTP request.  The client can initialize
 *                       during #MHD_CONNECTION_NOTIFY_STARTED and
 *                       cleanup during #MHD_CONNECTION_NOTIFY_CLOSED
 *                       and access in the meantime using
 *                       #MHD_CONNECTION_INFO_SOCKET_CONTEXT.
 * @param toe reason for connection notification
 * @see #MHD_OPTION_NOTIFY_CONNECTION
 * @ingroup request
 */
static void
notify_connection_cb (void *cls,
                      struct MHD_Connection *connection,
                      void **socket_context,
                      enum MHD_ConnectionNotificationCode toe)
{
  static int started = MHD_NO;

  (void) cls;
  (void) connection;  /* Unused. Silent compiler warning. */
  switch (toe)
  {
  case MHD_CONNECTION_NOTIFY_STARTED:
    if (MHD_NO != started)
      mhdErrorExitDesc ("The connection has been already started");
    started = MHD_YES;
    *socket_context = &started;
    break;
  case MHD_CONNECTION_NOTIFY_CLOSED:
    if (MHD_YES != started)
      mhdErrorExitDesc ("The connection has not been started before");
    if (&started != *socket_context)
      mhdErrorExitDesc ("Wrong '*socket_context' value");
    *socket_context = NULL;
    started = MHD_NO;
    break;
  }
}


static void
send_all (struct wr_socket *sock,
          const void *data,
          size_t data_size)
{
  ssize_t ret;
  size_t sent;
  const uint8_t *const buf = (const uint8_t *) data;

  wr_make_nonblocking (sock);
  for (sent = 0; sent < data_size; sent += (size_t) ret)
  {
    ret = wr_send (sock,
                   buf + sent,
                   data_size - sent);
    if (0 > ret)
    {
      if (MHD_SCKT_ERR_IS_EAGAIN_ (MHD_socket_get_error_ ()) ||
          MHD_SCKT_ERR_IS_EINTR_ (MHD_socket_get_error_ ()))
      {
        ret = 0;
        continue;
      }
      externalErrorExitDesc ("send() failed");
    }
  }
}


#define send_all_stext(sk,st) send_all(sk,st,MHD_STATICSTR_LEN_(st))


/**
 * Read character-by-character until we
 * get 'CRLNCRLN'.
 */
static void
recv_hdr (struct wr_socket *sock)
{
  unsigned int i;
  char next;
  char c;
  ssize_t ret;

  wr_make_nonblocking (sock);
  next = '\r';
  i = 0;
  while (i < 4)
  {
    ret = wr_recv (sock,
                   &c,
                   1);
    if (0 > ret)
    {
      if (MHD_SCKT_ERR_IS_EAGAIN_ (MHD_socket_get_error_ ()))
        continue;
      if (MHD_SCKT_ERR_IS_EINTR_ (MHD_socket_get_error_ ()))
        continue;
      externalErrorExitDesc ("recv() failed");
    }
    if (0 == ret)
      mhdErrorExitDesc ("The server unexpectedly closed connection");
    if (c == next)
    {
      i++;
      if (next == '\r')
        next = '\n';
      else
        next = '\r';
      continue;
    }
    if (c == '\r')
    {
      i = 1;
      next = '\n';
      continue;
    }
    i = 0;
    next = '\r';
  }
}


static void
recv_all (struct wr_socket *sock,
          const void *data,
          size_t data_size)
{
  uint8_t *buf;
  ssize_t ret;
  size_t rcvd;

  buf = (uint8_t *) malloc (data_size);
  if (NULL == buf)
    externalErrorExitDesc ("malloc() failed");

  wr_make_nonblocking (sock);
  for (rcvd = 0; rcvd < data_size; rcvd += (size_t) ret)
  {
    ret = wr_recv (sock,
                   buf + rcvd,
                   data_size - rcvd);
    if (0 > ret)
    {
      if (MHD_SCKT_ERR_IS_EAGAIN_ (MHD_socket_get_error_ ()) ||
          MHD_SCKT_ERR_IS_EINTR_ (MHD_socket_get_error_ ()))
      {
        ret = 0;
        continue;
      }
      externalErrorExitDesc ("recv() failed");
    }
    else if (0 == ret)
    {
      fprintf (stderr, "Partial only received text. Expected: '%.*s' "
               "(length: %ud). Got: '%.*s' (length: %ud). ",
               (int) data_size, (const char *) data, (unsigned int) data_size,
               (int) rcvd, (const char *) buf, (unsigned int) rcvd);
      mhdErrorExitDesc ("The server unexpectedly closed connection");
    }
    if ((data_size - rcvd) < (size_t) ret)
      externalErrorExitDesc ("recv() returned excessive amount of data");
    if (0 != memcmp (data, buf, rcvd + (size_t) ret))
    {
      fprintf (stderr, "Wrong received text. Expected: '%.*s'. "
               "Got: '%.*s'. ",
               (int) (rcvd + (size_t) ret), (const char *) data,
               (int) (rcvd + (size_t) ret), (const char *) buf);
      mhdErrorExit ();
    }
  }
  if (0 != memcmp (data, buf, data_size))
  {
    fprintf (stderr, "Wrong received text. Expected: '%.*s'. "
             "Got: '%.*s'. ",
             (int) data_size, (const char *) data,
             (int) data_size, (const char *) buf);
    mhdErrorExit ();
  }
  free (buf);
}


#define recv_all_stext(sk,st) recv_all(sk,st,MHD_STATICSTR_LEN_(st))


/**
 * Shutdown write of the connection to signal end of transmission
 * for the remote side
 * @param sock the socket to shutdown
 */
static void
send_eof (struct wr_socket *sock)
{
  if (0 != wr_shutdown (sock, /*
  ** On Darwin local shutdown of RD cause error
  ** if remote side shut down WR before.
  wr_is_eof_received (sock) ? SHUT_RDWR : */
                        SHUT_WR))
    externalErrorExitDesc ("Failed to shutdown connection");
}


/**
 * Receive end of the transmission indication from the remote side
 * @param sock the socket to use
 */
static void
receive_eof (struct wr_socket *sock)
{
  uint8_t buf[127];
  ssize_t ret;
  size_t rcvd;
  bool got_eof = false;

  wr_make_nonblocking (sock);
  for (rcvd = 0; rcvd < sizeof(buf); rcvd += (size_t) ret)
  {
    ret = wr_recv (sock,
                   buf + rcvd,
                   sizeof(buf) - rcvd);
    if (0 > ret)
    {
      if (MHD_SCKT_ERR_IS_EAGAIN_ (MHD_socket_get_error_ ()) ||
          MHD_SCKT_ERR_IS_EINTR_ (MHD_socket_get_error_ ()))
      {
        ret = 0;
        continue;
      }
      externalErrorExitDesc ("recv() failed");
    }
    else if (0 == ret)
    {
      got_eof = true;
      break;
    }
  }
  if (got_eof && (0 == rcvd))
    return; /* Success */

  if (0 != rcvd)
  {
    if (sizeof(buf) == rcvd)
    {
      fprintf (stderr, "Received at least %lu extra bytes while "
               "end-of-file is expected.\n", (unsigned long) sizeof(buf));
      mhdErrorExit ();
    }
    fprintf (stderr, "Received at %lu extra bytes and then %s"
             "end-of-file marker.\n", (unsigned long) rcvd,
             got_eof ? "" : "NO ");
    mhdErrorExit ();
  }
  if (! got_eof)
    mhdErrorExitDesc ("Failed to receive end-of-file marker.");
}


/**
 * Main function for the thread that runs the interaction with
 * the upgraded socket.
 *
 * @param cls the handle for the upgrade
 */
static void *
run_usock (void *cls)
{
  struct MHD_UpgradeResponseHandle *urh = cls;

  recv_all (usock, rclient_msg, rclient_msg_size);
  send_all (usock, app_msg, app_msg_size);
  recv_all_stext (usock,
                  "Finished");
  if (! test_tls)
  {
    receive_eof (usock);
    send_eof (usock);
  }
  MHD_upgrade_action (urh,
                      MHD_UPGRADE_ACTION_CLOSE);
  free (usock);
  usock = NULL;
  app_done = true;
  return NULL;
}


/**
 * Main function for the thread that runs the client-side of the
 * interaction with the upgraded socket.
 *
 * @param cls the client socket
 */
static void *
run_usock_client (void *cls)
{
  struct wr_socket *sock = cls;

  send_all_stext (sock,
                  "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: Upgrade\r\n\r\n");
  recv_hdr (sock);
  send_all (sock, rclient_msg, rclient_msg_size);
  recv_all (sock, app_msg, app_msg_size);
  send_all_stext (sock,
                  "Finished");
  if (! test_tls)
  {
    send_eof (sock);
    receive_eof (sock);
  }
  wr_close (sock);
  client_done = true;
  return NULL;
}


/**
 * Function called after a protocol "upgrade" response was sent
 * successfully and the socket should now be controlled by some
 * protocol other than HTTP.
 *
 * Any data already received on the socket will be made available in
 * @e extra_in.  This can happen if the application sent extra data
 * before MHD send the upgrade response.  The application should
 * treat data from @a extra_in as if it had read it from the socket.
 *
 * Note that the application must not close() @a sock directly,
 * but instead use #MHD_upgrade_action() for special operations
 * on @a sock.
 *
 * Except when in 'thread-per-connection' mode, implementations
 * of this function should never block (as it will still be called
 * from within the main event loop).
 *
 * @param cls closure, whatever was given to #MHD_create_response_for_upgrade().
 * @param connection original HTTP connection handle,
 *                   giving the function a last chance
 *                   to inspect the original HTTP request
 * @param req_cls last value left in `req_cls` of the `MHD_AccessHandlerCallback`
 * @param extra_in if we happened to have read bytes after the
 *                 HTTP header already (because the client sent
 *                 more than the HTTP header of the request before
 *                 we sent the upgrade response),
 *                 these are the extra bytes already read from @a sock
 *                 by MHD.  The application should treat these as if
 *                 it had read them from @a sock.
 * @param extra_in_size number of bytes in @a extra_in
 * @param sock socket to use for bi-directional communication
 *        with the client.  For HTTPS, this may not be a socket
 *        that is directly connected to the client and thus certain
 *        operations (TCP-specific setsockopt(), getsockopt(), etc.)
 *        may not work as expected (as the socket could be from a
 *        socketpair() or a TCP-loopback).  The application is expected
 *        to perform read()/recv() and write()/send() calls on the socket.
 *        The application may also call shutdown(), but must not call
 *        close() directly.
 * @param urh argument for #MHD_upgrade_action()s on this @a connection.
 *        Applications must eventually use this callback to (indirectly)
 *        perform the close() action on the @a sock.
 */
static void
upgrade_cb (void *cls,
            struct MHD_Connection *connection,
            void *req_cls,
            const char *extra_in,
            size_t extra_in_size,
            MHD_socket sock,
            struct MHD_UpgradeResponseHandle *urh)
{
  (void) cls;
  (void) connection;
  (void) req_cls;
  (void) extra_in; /* Unused. Silent compiler warning. */

  usock = wr_create_from_plain_sckt (sock);
  wr_make_nonblocking (usock);
  if (0 != extra_in_size)
    mhdErrorExitDesc ("'extra_in_size' is not zero");
  if (0 != pthread_create (&pt,
                           NULL,
                           &run_usock,
                           urh))
    externalErrorExitDesc ("pthread_create() failed");
}


/**
 * A client has requested the given url using the given method
 * (#MHD_HTTP_METHOD_GET, #MHD_HTTP_METHOD_PUT,
 * #MHD_HTTP_METHOD_DELETE, #MHD_HTTP_METHOD_POST, etc).  The callback
 * must call MHD callbacks to provide content to give back to the
 * client and return an HTTP status code (i.e. #MHD_HTTP_OK,
 * #MHD_HTTP_NOT_FOUND, etc.).
 *
 * @param cls argument given together with the function
 *        pointer when the handler was registered with MHD
 * @param url the requested url
 * @param method the HTTP method used (#MHD_HTTP_METHOD_GET,
 *        #MHD_HTTP_METHOD_PUT, etc.)
 * @param version the HTTP version string (i.e.
 *        #MHD_HTTP_VERSION_1_1)
 * @param upload_data the data being uploaded (excluding HEADERS,
 *        for a POST that fits into memory and that is encoded
 *        with a supported encoding, the POST data will NOT be
 *        given in upload_data and is instead available as
 *        part of #MHD_get_connection_values; very large POST
 *        data *will* be made available incrementally in
 *        @a upload_data)
 * @param upload_data_size set initially to the size of the
 *        @a upload_data provided; the method must update this
 *        value to the number of bytes NOT processed;
 * @param req_cls pointer that the callback can set to some
 *        address and that will be preserved by MHD for future
 *        calls for this request; since the access handler may
 *        be called many times (i.e., for a PUT/POST operation
 *        with plenty of upload data) this allows the application
 *        to easily associate some request-specific state.
 *        If necessary, this state can be cleaned up in the
 *        global #MHD_RequestCompletedCallback (which
 *        can be set with the #MHD_OPTION_NOTIFY_COMPLETED).
 *        Initially, `*req_cls` will be NULL.
 * @return #MHD_YES if the connection was handled successfully,
 *         #MHD_NO if the socket must be closed due to a serious
 *         error while handling the request
 */
static enum MHD_Result
ahc_upgrade (void *cls,
             struct MHD_Connection *connection,
             const char *url,
             const char *method,
             const char *version,
             const char *upload_data,
             size_t *upload_data_size,
             void **req_cls)
{
  struct MHD_Response *resp;
  (void) cls;
  (void) url;
  (void) method;                        /* Unused. Silent compiler warning. */
  (void) version;
  (void) upload_data;
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (NULL == req_cls)
    mhdErrorExitDesc ("'req_cls' is NULL");
  if (NULL == *req_cls)
    mhdErrorExitDesc ("'*req_cls' value is NULL");
  if (! pthread_equal (**((pthread_t **) req_cls), pthread_self ()))
    mhdErrorExitDesc ("ahc_upgrade() is called in wrong thread");
  resp = MHD_create_response_for_upgrade (&upgrade_cb,
                                          NULL);
  if (NULL == resp)
    mhdErrorExitDesc ("MHD_create_response_for_upgrade() failed");
  if (MHD_YES != MHD_add_response_header (resp,
                                          MHD_HTTP_HEADER_UPGRADE,
                                          "Hello World Protocol"))
    mhdErrorExitDesc ("MHD_add_response_header() failed");
  if (MHD_YES != MHD_queue_response (connection,
                                     MHD_HTTP_SWITCHING_PROTOCOLS,
                                     resp))
    mhdErrorExitDesc ("MHD_queue_response() failed");
  MHD_destroy_response (resp);
  return MHD_YES;
}


/**
 * Run the MHD external event loop using select or epoll.
 *
 * select/epoll modes are used automatically based on daemon's flags.
 *
 * @param daemon daemon to run it for
 */
static void
run_mhd_select_loop (struct MHD_Daemon *daemon)
{
  const time_t start_time = time (NULL);
  const union MHD_DaemonInfo *pdinfo;
  bool connection_was_accepted;
  bool connection_has_finished;
#ifdef EPOLL_SUPPORT
  bool use_epoll = false;
  int ep = -1;

  pdinfo = MHD_get_daemon_info (daemon,
                                MHD_DAEMON_INFO_FLAGS);
  if (NULL == pdinfo)
    mhdErrorExitDesc ("MHD_get_daemon_info() failed");
  else
    use_epoll = (0 != (pdinfo->flags & MHD_USE_EPOLL));
  if (use_epoll)
  {
    pdinfo = MHD_get_daemon_info (daemon,
                                  MHD_DAEMON_INFO_EPOLL_FD);
    if (NULL == pdinfo)
      mhdErrorExitDesc ("MHD_get_daemon_info() failed");
    ep = pdinfo->listen_fd;
    if (0 > ep)
      mhdErrorExitDesc ("Invalid epoll FD value");
  }
#endif /* EPOLL_SUPPORT */

  connection_was_accepted = false;
  connection_has_finished = false;
  while (1)
  {
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket max_fd;
    struct timeval tv;
    uint64_t to64;
    bool has_mhd_timeout;

    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    max_fd = MHD_INVALID_SOCKET;

    if (time (NULL) - start_time > ((time_t) test_timeout))
      mhdErrorExitDesc ("Test timeout");

    pdinfo = MHD_get_daemon_info (daemon, MHD_DAEMON_INFO_CURRENT_CONNECTIONS);

    if (NULL == pdinfo)
      mhdErrorExitDesc ("MHD_get_daemon_info() failed");

    if (0 != pdinfo->num_connections)
      connection_was_accepted = true;
    else
    {
      if (connection_was_accepted)
        connection_has_finished = true;
    }
    if (connection_has_finished)
      return;

    if (MHD_YES !=
        MHD_get_fdset (daemon,
                       &rs,
                       &ws,
                       &es,
                       &max_fd))
      mhdErrorExitDesc ("MHD_get_fdset() failed");

#ifdef EPOLL_SUPPORT
    if (use_epoll)
    {
      if (ep != max_fd)
        mhdErrorExitDesc ("Wrong 'max_fd' value");
      if (! FD_ISSET (ep, &rs))
        mhdErrorExitDesc ("Epoll FD is NOT set in read fd_set");
    }
#endif /* EPOLL_SUPPORT */

    has_mhd_timeout = (MHD_NO != MHD_get_timeout64 (daemon,
                                                    &to64));
    if (has_mhd_timeout)
    {
#if ! defined(_WIN32) || defined(__CYGWIN__)
      tv.tv_sec = (time_t) (to64 / 1000);
#else  /* Native W32 */
      tv.tv_sec = (long) (to64 / 1000);
#endif /* Native W32 */
      tv.tv_usec = (long) (1000 * (to64 % 1000));
    }
    else
    {
#if ! defined(_WIN32) || defined(__CYGWIN__)
      tv.tv_sec = (time_t) test_timeout;
#else  /* Native W32 */
      tv.tv_sec = (long) test_timeout;
#endif /* Native W32 */
      tv.tv_usec = 0;
    }

#ifdef MHD_WINSOCK_SOCKETS
    if ((0 == rs.fd_count) && (0 == ws.fd_count) && (0 != es.fd_count))
      Sleep ((DWORD) (tv.tv_sec * 1000 + tv.tv_usec / 1000));
    else /* Combined with the next 'if' */
#endif
    if (1)
    {
      int sel_res;
      sel_res = MHD_SYS_select_ (max_fd + 1,
                                 &rs,
                                 &ws,
                                 &es,
                                 &tv);
      if (0 == sel_res)
      {
        if (! has_mhd_timeout)
          mhdErrorExitDesc ("Timeout waiting for data on sockets");
      }
      else if (0 > sel_res)
      {
#ifdef MHD_POSIX_SOCKETS
        if (EINTR != errno)
#endif /* MHD_POSIX_SOCKETS */
        mhdErrorExitDesc ("Unexpected select() error");
      }
    }
    MHD_run_from_select (daemon,
                         &rs,
                         &ws,
                         &es);
  }
}


#ifdef HAVE_POLL

/**
 * Run the MHD external event loop using select.
 *
 * @param daemon daemon to run it for
 */
_MHD_NORETURN static void
run_mhd_poll_loop (struct MHD_Daemon *daemon)
{
  (void) daemon; /* Unused. Silent compiler warning. */
  externalErrorExitDesc ("Not implementable with MHD API");
}


#endif /* HAVE_POLL */


/**
 * Run the MHD external event loop using select.
 *
 * @param daemon daemon to run it for
 */
static void
run_mhd_loop (struct MHD_Daemon *daemon,
              unsigned int flags)
{
  if (0 == (flags & (MHD_USE_POLL | MHD_USE_EPOLL)))
    run_mhd_select_loop (daemon);
#ifdef HAVE_POLL
  else if (0 != (flags & MHD_USE_POLL))
    run_mhd_poll_loop (daemon);
#endif /* HAVE_POLL */
#ifdef EPOLL_SUPPORT
  else if (0 != (flags & MHD_USE_EPOLL))
    run_mhd_select_loop (daemon);
#endif
  else
    externalErrorExitDesc ("Wrong 'flags' value");
}


/**
 * Test upgrading a connection.
 *
 * @param flags which event loop style should be tested
 * @param pool size of the thread pool, 0 to disable
 */
static unsigned int
test_upgrade (unsigned int flags,
              unsigned int pool)
{
  struct MHD_Daemon *d = NULL;
  struct wr_socket *sock;
  struct sockaddr_in sa;
  enum MHD_FLAG used_flags;
  const union MHD_DaemonInfo *dinfo;
#if defined(HTTPS_SUPPORT) && defined(HAVE_FORK) && defined(HAVE_WAITPID)
  pid_t pid = -1;
#endif /* HTTPS_SUPPORT && HAVE_FORK && HAVE_WAITPID */
  size_t mem_limit;

  /* Handle memory limits. Actually makes sense only for TLS */
  if (use_vlarge)
    mem_limit = 64U * 1024U;  /* Half of the buffer should be large enough to take more than max TLS packet */
  else if (use_large)
    mem_limit = 4U * 1024;    /* Make sure that several iteration required to deliver a single message */
  else
    mem_limit = 0;            /* Use default value */

  client_done = false;
  app_done = false;

  if (! test_tls)
    d = MHD_start_daemon (flags | MHD_USE_ERROR_LOG | MHD_ALLOW_UPGRADE
                          | MHD_USE_ITC,
                          global_port,
                          NULL, NULL,
                          &ahc_upgrade, NULL,
                          MHD_OPTION_URI_LOG_CALLBACK, &log_cb, NULL,
                          MHD_OPTION_NOTIFY_COMPLETED, &notify_completed_cb,
                          NULL,
                          MHD_OPTION_NOTIFY_CONNECTION, &notify_connection_cb,
                          NULL,
                          MHD_OPTION_THREAD_POOL_SIZE, pool,
                          MHD_OPTION_CONNECTION_TIMEOUT, test_timeout,
                          MHD_OPTION_CONNECTION_MEMORY_LIMIT, mem_limit,
                          MHD_OPTION_END);
#ifdef HTTPS_SUPPORT
  else
    d = MHD_start_daemon (flags | MHD_USE_ERROR_LOG | MHD_ALLOW_UPGRADE
                          | MHD_USE_TLS | MHD_USE_ITC,
                          global_port,
                          NULL, NULL,
                          &ahc_upgrade, NULL,
                          MHD_OPTION_URI_LOG_CALLBACK, &log_cb, NULL,
                          MHD_OPTION_NOTIFY_COMPLETED, &notify_completed_cb,
                          NULL,
                          MHD_OPTION_NOTIFY_CONNECTION, &notify_connection_cb,
                          NULL,
                          MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
                          MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
                          MHD_OPTION_THREAD_POOL_SIZE, pool,
                          MHD_OPTION_CONNECTION_TIMEOUT, test_timeout,
                          MHD_OPTION_CONNECTION_MEMORY_LIMIT, mem_limit,
                          MHD_OPTION_END);
#endif /* HTTPS_SUPPORT */
  if (NULL == d)
    mhdErrorExitDesc ("MHD_start_daemon() failed");
  dinfo = MHD_get_daemon_info (d,
                               MHD_DAEMON_INFO_FLAGS);
  if (NULL == dinfo)
    mhdErrorExitDesc ("MHD_get_daemon_info() failed");
  used_flags = dinfo->flags;
  dinfo = MHD_get_daemon_info (d,
                               MHD_DAEMON_INFO_BIND_PORT);
  if ( (NULL == dinfo) ||
       (0 == dinfo->port) )
    mhdErrorExitDesc ("MHD_get_daemon_info() failed");
  global_port = dinfo->port; /* Re-use the same port for the next checks */
  if (! test_tls || (TLS_LIB_GNUTLS == use_tls_tool))
  {
    sock = test_tls ? wr_create_tls_sckt () : wr_create_plain_sckt ();
    if (NULL == sock)
      externalErrorExitDesc ("Create socket failed");
    wr_make_nonblocking (sock);
    sa.sin_family = AF_INET;
    sa.sin_port = htons (dinfo->port);
    sa.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    if (0 != wr_connect (sock,
                         (struct sockaddr *) &sa,
                         sizeof (sa)))
      externalErrorExitDesc ("Connect socket failed");
  }
  else
  {
#if defined(HTTPS_SUPPORT) && defined(HAVE_FORK) && defined(HAVE_WAITPID)
    MHD_socket tls_fork_sock;
    uint16_t port;

    port = dinfo->port;
    if (-1 == (pid = gnutlscli_connect (&tls_fork_sock,
                                        port)))
      externalErrorExitDesc ("gnutlscli_connect() failed");

    sock =  wr_create_from_plain_sckt (tls_fork_sock);
    if (NULL == sock)
      externalErrorExitDesc ("wr_create_from_plain_sckt() failed");

    wr_make_nonblocking (sock);
#else  /* !HTTPS_SUPPORT || !HAVE_FORK || !HAVE_WAITPID */
    externalErrorExitDesc ("Unsupported 'use_tls_tool' value");
#endif /* !HTTPS_SUPPORT || !HAVE_FORK || !HAVE_WAITPID */
  }

  if (0 != pthread_create (&pt_client,
                           NULL,
                           &run_usock_client,
                           sock))
    externalErrorExitDesc ("pthread_create() failed");
  if (0 == (flags & MHD_USE_INTERNAL_POLLING_THREAD) )
    run_mhd_loop (d, used_flags);
  if (0 != pthread_join (pt_client,
                         NULL))
    externalErrorExitDesc ("pthread_join() failed");
  if (0 != pthread_join (pt,
                         NULL))
    externalErrorExitDesc ("pthread_join() failed");
#if defined(HTTPS_SUPPORT) && defined(HAVE_FORK) && defined(HAVE_WAITPID)
  if (test_tls && (TLS_LIB_GNUTLS != use_tls_tool))
  {
    if ((pid_t) -1 == waitpid (pid, NULL, 0))
      externalErrorExitDesc ("waitpid() failed");
  }
#endif /* HTTPS_SUPPORT && HAVE_FORK && HAVE_WAITPID */
  if (! client_done)
    externalErrorExitDesc ("The client thread has not signalled " \
                           "successful finish");
  if (! app_done)
    externalErrorExitDesc ("The application thread has not signalled " \
                           "successful finish");
  MHD_stop_daemon (d);
  return 0;
}


enum test_msg_type
{
  test_msg_large_app_data,
  test_msg_large_rclient_data,
  test_msg_vlarge_app_data,
  test_msg_vlarge_rclient_data
};

/**
 * Initialise test message data
 * @param buf the pointer to the buffer to fill with the test data
 * @param buf_size the size of the @a buf
 * @param msg_type the type of the data to fill the @a buf
 * @return the @a buf pointer
 */
static void *
init_test_msg (void *buf, size_t buf_size, enum test_msg_type msg_type)
{
  size_t i;
  char *const text_buf = (char *) buf;
  uint8_t *const bin_buf = (uint8_t *) buf;
  if (0 == buf_size)
    return buf;
  switch (msg_type)
  {
  case test_msg_large_app_data:
  case test_msg_large_rclient_data:
    /* Simulate text data */
    for (i = 0; i < buf_size; ++i)
    {
      size_t pos;
      if (test_msg_large_app_data == msg_type)
        pos = i + 43;
      else
        pos = i + 26;
      if ((0 == i) || (2 == pos % 100) )
        text_buf[i] =
          (char) (unsigned char) ((test_msg_large_app_data == msg_type) ?
                                  ('Z' - pos % ('Z' - 'A' + 1)) :
                                  ('A' + pos % ('Z' - 'A' + 1)));
      else if (0 == pos % 100)
        text_buf[i] = '.';
      else if (1 == pos % 100)
        text_buf[i] = ' ';
      else if ((99 != pos % 100) && (2 != pos % 100) && (0 == pos % 5))
        text_buf[i] = ' ';
      else if (test_msg_large_app_data == msg_type)
        text_buf[i] = (char) (unsigned char) ('z' - pos % ('z' - 'a' + 1));
      else
        text_buf[i] = (char) (unsigned char) ('a' + pos % ('z' - 'a' + 1));
    }
    break;
  case test_msg_vlarge_app_data:
    /* Simulate binary data */
    for (i = 0; i < buf_size; ++i)
    {
      bin_buf[i] = (uint8_t) ((i + 182) & 0xFF);
    }
    break;
  case test_msg_vlarge_rclient_data:
    /* Simulate binary data */
    for (i = 0; i < buf_size; ++i)
    {
      bin_buf[i] = (uint8_t) ((111 - i) & 0xFF);
    }
    break;
  default:
    exit (99);
    break;
  }
  return buf;
}


/**
 * Perform initialisation of variables used in all check in this test
 * @return true if succeed,
 *         false if failed.
 */
static bool
global_test_init (void)
{
  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    global_port = 0;
  else
  {
    global_port = 1090;
    if (test_tls)
      global_port += 1U << 0;
    if (use_large)
      global_port += 1U << 1;
    else if (use_vlarge)
      global_port += 1U << 2;
  }
  if (use_large || use_vlarge)
  {
    unsigned int i;
    size_t alloc_size;
    alloc_size =  use_vlarge ? (64U * 1024U) : (17U * 1024U);
    for (i = 0; i < (sizeof(alloc_ptr) / sizeof(alloc_ptr[0])); ++i)
    {
      alloc_ptr[i] = malloc (alloc_size);
      if (NULL == alloc_ptr[i])
      {
        for (--i; i < (sizeof(alloc_ptr) / sizeof(alloc_ptr[0])); --i)
        {
          free (alloc_ptr[i]);
        }
        return false;
      }
    }

    rclient_msg_size = alloc_size;
    rclient_msg = init_test_msg (alloc_ptr[0], rclient_msg_size,
                                 use_vlarge ?
                                 test_msg_vlarge_rclient_data :
                                 test_msg_large_rclient_data);
    app_msg_size = alloc_size;
    app_msg = init_test_msg (alloc_ptr[1], app_msg_size,
                             use_vlarge ?
                             test_msg_vlarge_app_data :
                             test_msg_large_app_data);
  }
  else
  {
    unsigned int i;
    for (i = 0; i < (sizeof(alloc_ptr) / sizeof(alloc_ptr[0])); ++i)
      alloc_ptr[i] = NULL;

    rclient_msg_size = MHD_STATICSTR_LEN_ ("Hello");
    rclient_msg = "Hello";
    app_msg_size = MHD_STATICSTR_LEN_ ("World");
    app_msg = "World";
  }
  return true;
}


/**
 * Perform de-initialisation of variables with memory de-allocation if required.
 */
static void
global_test_deinit (void)
{
  unsigned int i;
  for (i = ((sizeof(alloc_ptr) / sizeof(alloc_ptr[0])) - 1);
       i < (sizeof(alloc_ptr) / sizeof(alloc_ptr[0]));
       --i)
  {
    if (NULL != alloc_ptr[i])
      free (alloc_ptr[i]);
  }
}


int
main (int argc,
      char *const *argv)
{
  unsigned int error_count = 0;
  unsigned int res;

  use_vlarge = (0 != has_in_name (argv[0], "_vlarge"));
  use_large = (! use_vlarge) && (0 != has_in_name (argv[0], "_large"));

  use_tls_tool = TLS_CLI_NO_TOOL;
  test_tls = has_in_name (argv[0], "_tls");

  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));

  if ((((int) ((~((unsigned int) 0U)) >> 1)) / 1000) < test_timeout)
  {
    fprintf (stderr, "The test timeout value (%d) is too large.\n"
             "The test cannot run.\n", test_timeout);
    fprintf (stderr, "The maximum allowed timeout value is %d.\n",
             (((int) ((~((unsigned int) 0U)) >> 1)) / 1000));
    return 3;
  }

  if (test_tls)
  {
    use_tls_tool = TLS_LIB_GNUTLS;   /* Should be always available as MHD uses it. */
#ifdef HTTPS_SUPPORT
    if (has_param (argc, argv, "--use-gnutls-cli"))
      use_tls_tool = TLS_CLI_GNUTLS;
    else if (has_param (argc, argv, "--use-openssl"))
      use_tls_tool = TLS_CLI_OPENSSL;
    else if (has_param (argc, argv, "--use-gnutls-lib"))
      use_tls_tool = TLS_LIB_GNUTLS;
#if defined(HAVE_FORK) && defined(HAVE_WAITPID)
    else if (0 == system ("gnutls-cli --version 1> /dev/null 2> /dev/null"))
      use_tls_tool = TLS_CLI_GNUTLS;
    else if (0 == system ("openssl version 1> /dev/null 2> /dev/null"))
      use_tls_tool = TLS_CLI_OPENSSL;
#endif /* HAVE_FORK && HAVE_WAITPID */
    if (verbose)
    {
      switch (use_tls_tool)
      {
      case TLS_CLI_GNUTLS:
        printf ("GnuTLS-CLI will be used for testing.\n");
        break;
      case TLS_CLI_OPENSSL:
        printf ("Command line version of OpenSSL will be used for testing.\n");
        break;
      case TLS_LIB_GNUTLS:
        printf ("GnuTLS library will be used for testing.\n");
        break;
      case TLS_CLI_NO_TOOL:
      default:
        externalErrorExitDesc ("Wrong 'use_tls_tool' value");
      }
    }
    if ( (TLS_LIB_GNUTLS == use_tls_tool) &&
         (GNUTLS_E_SUCCESS != gnutls_global_init ()) )
      externalErrorExitDesc ("gnutls_global_init() failed");

#else  /* ! HTTPS_SUPPORT */
    fprintf (stderr, "HTTPS support was disabled by configure.\n");
    return 77;
#endif /* ! HTTPS_SUPPORT */
  }

  if (! global_test_init ())
  {
#ifdef HTTPS_SUPPORT
    if (test_tls && (TLS_LIB_GNUTLS == use_tls_tool))
      gnutls_global_deinit ();
#endif /* HTTPS_SUPPORT */
    fprintf (stderr, "Failed to initialise the test.\n");
    return 99;
  }

  /* run tests */
  if (verbose)
    printf ("Starting HTTP \"Upgrade\" tests with %s connections.\n",
            test_tls ? "TLS" : "plain");
  /* try external select */
  res = test_upgrade (0,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with external select, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with external select.\n");

  /* Try external auto */
  res = test_upgrade (MHD_USE_AUTO,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with external 'auto', return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with external 'auto'.\n");

#ifdef EPOLL_SUPPORT
  res = test_upgrade (MHD_USE_EPOLL,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with external select with EPOLL, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with external select with EPOLL.\n");
#endif

  /* Test thread-per-connection */
  res = test_upgrade (MHD_USE_INTERNAL_POLLING_THREAD
                      | MHD_USE_THREAD_PER_CONNECTION,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with thread per connection, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with thread per connection.\n");

  res = test_upgrade (MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD
                      | MHD_USE_THREAD_PER_CONNECTION,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with thread per connection and 'auto', return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with thread per connection and 'auto'.\n");
#ifdef HAVE_POLL
  res = test_upgrade (MHD_USE_INTERNAL_POLLING_THREAD
                      | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with thread per connection and poll, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with thread per connection and poll.\n");
#endif /* HAVE_POLL */

  /* Test different event loops, with and without thread pool */
  res = test_upgrade (MHD_USE_INTERNAL_POLLING_THREAD,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal select, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal select.\n");
  res = test_upgrade (MHD_USE_INTERNAL_POLLING_THREAD,
                      2);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal select with thread pool, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal select with thread pool.\n");
  res = test_upgrade (MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal 'auto' return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal 'auto'.\n");
  res = test_upgrade (MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD,
                      2);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal 'auto' with thread pool, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal 'auto' with thread pool.\n");
#ifdef HAVE_POLL
  res = test_upgrade (MHD_USE_POLL_INTERNAL_THREAD,
                      0);
  fflush_allstd ();
  error_count += res;
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal poll, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal poll.\n");
  res = test_upgrade (MHD_USE_POLL_INTERNAL_THREAD,
                      2);
  fflush_allstd ();
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal poll with thread pool, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal poll with thread pool.\n");
#endif
#ifdef EPOLL_SUPPORT
  res = test_upgrade (MHD_USE_EPOLL_INTERNAL_THREAD,
                      0);
  fflush_allstd ();
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal epoll, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal epoll.\n");
  res = test_upgrade (MHD_USE_EPOLL_INTERNAL_THREAD,
                      2);
  fflush_allstd ();
  if (res)
    fprintf (stderr,
             "FAILED: Upgrade with internal epoll, return code %u.\n",
             res);
  else if (verbose)
    printf ("PASSED: Upgrade with internal epoll.\n");
#endif
  /* report result */
  if (0 != error_count)
    fprintf (stderr,
             "Error (code: %u)\n",
             error_count);

  global_test_deinit ();
#ifdef HTTPS_SUPPORT
  if (test_tls && (TLS_LIB_GNUTLS == use_tls_tool))
    gnutls_global_deinit ();
#endif /* HTTPS_SUPPORT */

  return error_count != 0;       /* 0 == pass */
}
