/*
     This file is part of libmicrohttpd
     Copyright (C) 2021-2022 Evgeny Grin (Karlson2k)

     libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
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
 * @file test_set_panic.c
 * @brief  Testcase for MHD_set_panic_func()
 * @author Karlson2k (Evgeny Grin)
 * @author Christian Grothoff
 */
#include "MHD_config.h"
#include "platform.h"
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#ifndef WINDOWS
#include <unistd.h>
#include <sys/socket.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#ifdef HAVE_SYSCTL
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif /* HAVE_SYS_SYSCTL_H */
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif /* HAVE_SYS_SOCKET_H */
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */
#ifdef HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif /* HAVE_NETINET_IP_H */
#ifdef HAVE_NETINET_IP_ICMP_H
#include <netinet/ip_icmp.h>
#endif /* HAVE_NETINET_IP_ICMP_H */
#ifdef HAVE_NETINET_ICMP_VAR_H
#include <netinet/icmp_var.h>
#endif /* HAVE_NETINET_ICMP_VAR_H */
#endif /* HAVE_SYSCTL */

#include <stdio.h>

#include "mhd_sockets.h" /* only macros used */
#include "test_helpers.h"
#include "mhd_assert.h"

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif
#if MHD_CPU_COUNT > 32
#undef MHD_CPU_COUNT
/* Limit to reasonable value */
#define MHD_CPU_COUNT 32
#endif /* MHD_CPU_COUNT > 32 */

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */

#ifndef _MHD_INSTRMACRO
/* Quoted macro parameter */
#define _MHD_INSTRMACRO(a) #a
#endif /* ! _MHD_INSTRMACRO */
#ifndef _MHD_STRMACRO
/* Quoted expanded macro parameter */
#define _MHD_STRMACRO(a) _MHD_INSTRMACRO (a)
#endif /* ! _MHD_STRMACRO */


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 5

/* Time in ms to wait for final packets to be delivered */
#define FINAL_PACKETS_MS 20

#define EXPECTED_URI_BASE_PATH  "/a"

#define REQ_HOST "localhost"

#define REQ_METHOD "PUT"

#define REQ_BODY "Some content data."

#define REQ_LINE_END "\r\n"

/* Mandatory request headers */
#define REQ_HEADER_HOST_NAME "Host"
#define REQ_HEADER_HOST_VALUE REQ_HOST
#define REQ_HEADER_HOST \
  REQ_HEADER_HOST_NAME ": " REQ_HEADER_HOST_VALUE REQ_LINE_END
#define REQ_HEADER_UA_NAME "User-Agent"
#define REQ_HEADER_UA_VALUE "dummyclient/0.9"
#define REQ_HEADER_UA REQ_HEADER_UA_NAME ": " REQ_HEADER_UA_VALUE REQ_LINE_END

/* Optional request headers */
#define REQ_HEADER_CT_NAME "Content-Type"
#define REQ_HEADER_CT_VALUE "text/plain"
#define REQ_HEADER_CT REQ_HEADER_CT_NAME ": " REQ_HEADER_CT_VALUE REQ_LINE_END


#if defined(HAVE___FUNC__)
#define externalErrorExit(ignore) \
  _externalErrorExit_func (NULL, __func__, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, __func__, __LINE__)
#define mhdErrorExit(ignore) \
  _mhdErrorExit_func (NULL, __func__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
  _mhdErrorExit_func (errDesc, __func__, __LINE__)
#elif defined(HAVE___FUNCTION__)
#define externalErrorExit(ignore) \
  _externalErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define mhdErrorExit(ignore) \
  _mhdErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
  _mhdErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func (NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func (NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func (errDesc, NULL, __LINE__)
#endif


_MHD_NORETURN static void
_externalErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
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


#ifndef MHD_POSIX_SOCKETS

/**
 * Pause execution for specified number of milliseconds.
 * @param ms the number of milliseconds to sleep
 */
static void
_MHD_sleep (uint32_t ms)
{
#if defined(_WIN32)
  Sleep (ms);
#elif defined(HAVE_NANOSLEEP)
  struct timespec slp = {ms / 1000, (ms % 1000) * 1000000};
  struct timespec rmn;
  int num_retries = 0;
  while (0 != nanosleep (&slp, &rmn))
  {
    if (EINTR != errno)
      externalErrorExit ();
    if (num_retries++ > 8)
      break;
    slp = rmn;
  }
#elif defined(HAVE_USLEEP)
  uint64_t us = ms * 1000;
  do
  {
    uint64_t this_sleep;
    if (999999 < us)
      this_sleep = 999999;
    else
      this_sleep = us;
    /* Ignore return value as it could be void */
    usleep (this_sleep);
    us -= this_sleep;
  } while (us > 0);
#else
  externalErrorExitDesc ("No sleep function available on this system");
#endif
}


#endif /* ! MHD_POSIX_SOCKETS */

/* Global parameters */
static int verbose;                 /**< Be verbose */
static uint16_t global_port;        /**< MHD daemons listen port number */

static void
test_global_init (void)
{
  if (MHD_YES != MHD_is_feature_supported (MHD_FEATURE_AUTOSUPPRESS_SIGPIPE))
  {
#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
    if (SIG_ERR == signal (SIGPIPE, SIG_IGN))
      externalErrorExitDesc ("Error suppressing SIGPIPE signal");
#else /* ! HAVE_SIGNAL_H || ! SIGPIPE */
    fprintf (stderr, "Cannot suppress SIGPIPE signal.\n");
    /* exit (77); */
#endif
  }
}


static void
test_global_cleanup (void)
{
}


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
    externalErrorExitDesc ("Cannot make socket non-blocking");
  if ((flags & ~O_NONBLOCK) != flags)
  {
    if (-1 == fcntl (fd, F_SETFL, flags & ~O_NONBLOCK))
      externalErrorExitDesc ("Cannot make socket non-blocking");
  }
#elif defined(MHD_WINSOCK_SOCKETS)
  unsigned long flags = 0;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    externalErrorExitDesc ("Cannot make socket non-blocking");
#endif /* MHD_WINSOCK_SOCKETS */
}


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
    externalErrorExitDesc ("Cannot make socket non-blocking");
  if ((flags | O_NONBLOCK) != flags)
  {
    if (-1 == fcntl (fd, F_SETFL, flags | O_NONBLOCK))
      externalErrorExitDesc ("Cannot make socket non-blocking");
  }
#elif defined(MHD_WINSOCK_SOCKETS)
  unsigned long flags = 1;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    externalErrorExitDesc ("Cannot make socket non-blocking");
#endif /* MHD_WINSOCK_SOCKETS */
}


enum _MHD_clientStage
{
  DUMB_CLIENT_INIT = 0,
  DUMB_CLIENT_CONNECTING,
  DUMB_CLIENT_CONNECTED,
  DUMB_CLIENT_REQ_SENDING,
  DUMB_CLIENT_REQ_SENT,
  DUMB_CLIENT_HEADER_RECVEIVING,
  DUMB_CLIENT_HEADER_RECVEIVED,
  DUMB_CLIENT_BODY_RECVEIVING,
  DUMB_CLIENT_BODY_RECVEIVED,
  DUMB_CLIENT_FINISHING,
  DUMB_CLIENT_FINISHED
};

struct _MHD_dumbClient
{
  MHD_socket sckt; /**< the socket to communicate */

  int sckt_nonblock;  /**< non-zero if socket is non-blocking */

  uint16_t port; /**< the port to connect to */

  char *send_buf; /**< the buffer for the request, malloced */

  size_t req_size; /**< the size of the request, including header */

  size_t send_off; /**< the number of bytes already sent */

  enum _MHD_clientStage stage;

  /* the test-specific variables */
  size_t single_send_size; /**< the maximum number of bytes to be sent by
                                single send() */
  size_t send_size_limit;  /**< the total number of send bytes limit */
};

struct _MHD_dumbClient *
_MHD_dumbClient_create (uint16_t port, const char *method, const char *url,
                        const char *add_headers,
                        const uint8_t *req_body, size_t req_body_size,
                        int chunked);


void
_MHD_dumbClient_set_send_limits (struct _MHD_dumbClient *clnt,
                                 size_t step_size, size_t max_total_send);

void
_MHD_dumbClient_start_connect (struct _MHD_dumbClient *clnt);

int
_MHD_dumbClient_is_req_sent (struct _MHD_dumbClient *clnt);

int
_MHD_dumbClient_process (struct _MHD_dumbClient *clnt);

void
_MHD_dumbClient_get_fdsets (struct _MHD_dumbClient *clnt,
                            MHD_socket *maxsckt,
                            fd_set *rs, fd_set *ws, fd_set *es);


/**
 * Process the client data with send()/recv() as needed based on
 * information in fd_sets.
 * @param clnt the client to process
 * @return non-zero if client finished processing the request,
 *         zero otherwise.
 */
int
_MHD_dumbClient_process_from_fdsets (struct _MHD_dumbClient *clnt,
                                     fd_set *rs, fd_set *ws, fd_set *es);

/**
 * Perform full request.
 * @param clnt the client to run
 * @return zero if client finished processing the request,
 *         non-zero if timeout is reached.
 */
int
_MHD_dumbClient_perform (struct _MHD_dumbClient *clnt);


/**
 * Close the client and free internally allocated resources.
 * @param clnt the client to close
 */
void
_MHD_dumbClient_close (struct _MHD_dumbClient *clnt);


struct _MHD_dumbClient *
_MHD_dumbClient_create (uint16_t port, const char *method, const char *url,
                        const char *add_headers,
                        const uint8_t *req_body, size_t req_body_size,
                        int chunked)
{
  struct _MHD_dumbClient *clnt;
  size_t method_size;
  size_t url_size;
  size_t add_hdrs_size;
  size_t buf_alloc_size;
  char *send_buf;

  mhd_assert (0 != port);
  mhd_assert (NULL != req_body || 0 == req_body_size);
  mhd_assert (0 == req_body_size || NULL != req_body);

  clnt = (struct _MHD_dumbClient *) malloc (sizeof(struct _MHD_dumbClient));
  if (NULL == clnt)
    externalErrorExit ();
  memset (clnt, 0, sizeof(struct _MHD_dumbClient));
  clnt->sckt = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (MHD_INVALID_SOCKET == clnt->sckt)
    externalErrorExitDesc ("Cannot create the client socket");

#ifdef MHD_socket_nosignal_
  if (! MHD_socket_nosignal_ (clnt->sckt))
    externalErrorExitDesc ("Cannot suppress SIGPIPE on the client socket");
#endif /* MHD_socket_nosignal_ */

  clnt->sckt_nonblock = 0;
  if (clnt->sckt_nonblock)
    make_nonblocking (clnt->sckt);
  else
    make_blocking (clnt->sckt);

  if (1)
  { /* Always set TCP NODELAY */
    const MHD_SCKT_OPT_BOOL_ on_val = 1;

    if (0 != setsockopt (clnt->sckt, IPPROTO_TCP, TCP_NODELAY,
                         (const void *) &on_val, sizeof (on_val)))
      externalErrorExitDesc ("Cannot set TCP_NODELAY option");
  }

  clnt->port = port;

  if (NULL != method)
    method_size = strlen (method);
  else
  {
    method = MHD_HTTP_METHOD_GET;
    method_size = MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_GET);
  }
  mhd_assert (0 != method_size);
  if (NULL != url)
    url_size = strlen (url);
  else
  {
    url = "/";
    url_size = 1;
  }
  mhd_assert (0 != url_size);
  add_hdrs_size = (NULL == add_headers) ? 0 : strlen (add_headers);
  buf_alloc_size = 1024 + method_size + url_size
                   + add_hdrs_size + req_body_size;
  send_buf = (char *) malloc (buf_alloc_size);
  if (NULL == send_buf)
    externalErrorExit ();

  clnt->req_size = 0;
  /* Form the request line */
  memcpy (send_buf + clnt->req_size, method, method_size);
  clnt->req_size += method_size;
  send_buf[clnt->req_size++] = ' ';
  memcpy (send_buf + clnt->req_size, url, url_size);
  clnt->req_size += url_size;
  send_buf[clnt->req_size++] = ' ';
  memcpy (send_buf + clnt->req_size, MHD_HTTP_VERSION_1_1,
          MHD_STATICSTR_LEN_ (MHD_HTTP_VERSION_1_1));
  clnt->req_size += MHD_STATICSTR_LEN_ (MHD_HTTP_VERSION_1_1);
  send_buf[clnt->req_size++] = '\r';
  send_buf[clnt->req_size++] = '\n';
  /* Form the header */
  memcpy (send_buf + clnt->req_size, REQ_HEADER_HOST,
          MHD_STATICSTR_LEN_ (REQ_HEADER_HOST));
  clnt->req_size += MHD_STATICSTR_LEN_ (REQ_HEADER_HOST);
  memcpy (send_buf + clnt->req_size, REQ_HEADER_UA,
          MHD_STATICSTR_LEN_ (REQ_HEADER_UA));
  clnt->req_size += MHD_STATICSTR_LEN_ (REQ_HEADER_UA);
  if ((NULL != req_body) || chunked)
  {
    if (! chunked)
    {
      int prn_size;

      memcpy (send_buf + clnt->req_size,
              MHD_HTTP_HEADER_CONTENT_LENGTH ": ",
              MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH ": "));
      clnt->req_size +=
        MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH ": ");
      prn_size = snprintf (send_buf + clnt->req_size,
                           (buf_alloc_size - clnt->req_size),
                           "%u", (unsigned int) req_body_size);
      if (0 >= prn_size)
        externalErrorExit ();
      if ((unsigned int) prn_size >= buf_alloc_size - clnt->req_size)
        externalErrorExit ();
      clnt->req_size += (unsigned int) prn_size;
      send_buf[clnt->req_size++] = '\r';
      send_buf[clnt->req_size++] = '\n';
    }
    else
    {
      memcpy (send_buf + clnt->req_size,
              MHD_HTTP_HEADER_TRANSFER_ENCODING ": chunked\r\n",
              MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_TRANSFER_ENCODING \
                                  ": chunked\r\n"));
      clnt->req_size += MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_TRANSFER_ENCODING \
                                            ": chunked\r\n");
    }
  }
  if (0 != add_hdrs_size)
  {
    memcpy (send_buf + clnt->req_size, add_headers, add_hdrs_size);
    clnt->req_size += add_hdrs_size;
  }
  /* Terminate header */
  send_buf[clnt->req_size++] = '\r';
  send_buf[clnt->req_size++] = '\n';

  /* Add body (if any) */
  if (! chunked)
  {
    if (0 != req_body_size)
    {
      memcpy (send_buf + clnt->req_size, req_body, req_body_size);
      clnt->req_size += req_body_size;
    }
  }
  else
  {
    if (0 != req_body_size)
    {
      int prn_size;
      prn_size = snprintf (send_buf + clnt->req_size,
                           (buf_alloc_size - clnt->req_size),
                           "%x", (unsigned int) req_body_size);
      if (0 >= prn_size)
        externalErrorExit ();
      if ((unsigned int) prn_size >= buf_alloc_size - clnt->req_size)
        externalErrorExit ();
      clnt->req_size += (unsigned int) prn_size;
      send_buf[clnt->req_size++] = '\r';
      send_buf[clnt->req_size++] = '\n';
      memcpy (send_buf + clnt->req_size, req_body, req_body_size);
      clnt->req_size += req_body_size;
      send_buf[clnt->req_size++] = '\r';
      send_buf[clnt->req_size++] = '\n';
    }
    send_buf[clnt->req_size++] = '0';
    send_buf[clnt->req_size++] = '\r';
    send_buf[clnt->req_size++] = '\n';
    send_buf[clnt->req_size++] = '\r';
    send_buf[clnt->req_size++] = '\n';
  }
  mhd_assert (clnt->req_size < buf_alloc_size);
  clnt->send_buf = send_buf;

  return clnt;
}


void
_MHD_dumbClient_set_send_limits (struct _MHD_dumbClient *clnt,
                                 size_t step_size, size_t max_total_send)
{
  clnt->single_send_size = step_size;
  clnt->send_size_limit = max_total_send;
}


/* internal */
static void
_MHD_dumbClient_connect_init (struct _MHD_dumbClient *clnt)
{
  struct sockaddr_in sa;
  mhd_assert (DUMB_CLIENT_INIT == clnt->stage);

  sa.sin_family = AF_INET;
  sa.sin_port = htons (clnt->port);
  sa.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

  if (0 != connect (clnt->sckt, (struct sockaddr *) &sa, sizeof(sa)))
  {
    const int err = MHD_socket_get_error_ ();
    if ( (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EINPROGRESS_)) ||
         (MHD_SCKT_ERR_IS_EAGAIN_ (err)))
      clnt->stage = DUMB_CLIENT_CONNECTING;
    else
      externalErrorExitDesc ("Cannot 'connect()' the client socket");
  }
  else
    clnt->stage = DUMB_CLIENT_CONNECTED;
}


void
_MHD_dumbClient_start_connect (struct _MHD_dumbClient *clnt)
{
  mhd_assert (DUMB_CLIENT_INIT == clnt->stage);
  _MHD_dumbClient_connect_init (clnt);
}


/* internal */
static void
_MHD_dumbClient_connect_finish (struct _MHD_dumbClient *clnt)
{
  int err = 0;
  socklen_t err_size = sizeof(err);
  mhd_assert (DUMB_CLIENT_CONNECTING == clnt->stage);
  if (0 != getsockopt (clnt->sckt, SOL_SOCKET, SO_ERROR,
                       (void *) &err, &err_size))
    externalErrorExitDesc ("'getsockopt()' call failed");
  if (0 != err)
    externalErrorExitDesc ("Socket connect() failed");
  clnt->stage = DUMB_CLIENT_CONNECTED;
}


/* internal */
static void
_MHD_dumbClient_send_req (struct _MHD_dumbClient *clnt)
{
  size_t send_size;
  ssize_t res;
  mhd_assert (DUMB_CLIENT_CONNECTED <= clnt->stage);
  mhd_assert (DUMB_CLIENT_REQ_SENT > clnt->stage);
  mhd_assert (clnt->req_size > clnt->send_off);

  send_size = (((0 != clnt->send_size_limit) &&
                (clnt->req_size > clnt->send_size_limit)) ?
               clnt->send_size_limit : clnt->req_size) - clnt->send_off;
  mhd_assert (0 != send_size);
  if ((0 != clnt->single_send_size) &&
      (clnt->single_send_size < send_size))
    send_size = clnt->single_send_size;

  res = MHD_send_ (clnt->sckt, clnt->send_buf + clnt->send_off, send_size);

  if (res < 0)
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
      return;
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return;
    if (MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
      mhdErrorExitDesc ("The connection was aborted by MHD");
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EPIPE_))
      mhdErrorExitDesc ("The connection was shut down on MHD side");
    externalErrorExitDesc ("Unexpected network error");
  }
  clnt->send_off += (size_t) res;
  mhd_assert (clnt->send_off <= clnt->req_size);
  mhd_assert (clnt->send_off <= clnt->send_size_limit || \
              0 == clnt->send_size_limit);
  if (clnt->req_size == clnt->send_off)
    clnt->stage = DUMB_CLIENT_REQ_SENT;
  if ((0 != clnt->send_size_limit) &&
      (clnt->send_size_limit == clnt->send_off))
    clnt->stage = DUMB_CLIENT_FINISHING;
}


/* internal */
_MHD_NORETURN /* not implemented */
static void
_MHD_dumbClient_recv_reply (struct _MHD_dumbClient *clnt)
{
  (void) clnt;
  externalErrorExitDesc ("Not implemented for this test");
}


int
_MHD_dumbClient_is_req_sent (struct _MHD_dumbClient *clnt)
{
  return DUMB_CLIENT_REQ_SENT <= clnt->stage;
}


/* internal */
static void
_MHD_dumbClient_socket_close (struct _MHD_dumbClient *clnt)
{
  if (MHD_INVALID_SOCKET != clnt->sckt)
  {
    if (! MHD_socket_close_ (clnt->sckt))
      externalErrorExitDesc ("Unexpected error while closing " \
                             "the client socket");
    clnt->sckt = MHD_INVALID_SOCKET;
  }
}


/* internal */
static void
_MHD_dumbClient_finalize (struct _MHD_dumbClient *clnt)
{
  if (MHD_INVALID_SOCKET != clnt->sckt)
  {
    if (0 != shutdown (clnt->sckt, SHUT_WR))
    {
      const int err = MHD_socket_get_error_ ();
      if (! MHD_SCKT_ERR_IS_ (err, MHD_SCKT_ENOTCONN_) &&
          ! MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
        mhdErrorExitDesc ("Unexpected error when shutting down " \
                          "the client socket");
    }
  }
  clnt->stage = DUMB_CLIENT_FINISHED;
}


/* internal */
static int
_MHD_dumbClient_needs_send (const struct _MHD_dumbClient *clnt)
{
  return ((DUMB_CLIENT_CONNECTING <= clnt->stage) &&
          (DUMB_CLIENT_REQ_SENT > clnt->stage)) ||
         (DUMB_CLIENT_FINISHING == clnt->stage);
}


/* internal */
static int
_MHD_dumbClient_needs_recv (const struct _MHD_dumbClient *clnt)
{
  return (DUMB_CLIENT_HEADER_RECVEIVING <= clnt->stage) &&
         (DUMB_CLIENT_BODY_RECVEIVED > clnt->stage);
}


/* internal */
/**
 * Check whether the client needs unconditionally process the data.
 * @param clnt the client to check
 * @return non-zero if client needs unconditionally process the data,
 *         zero otherwise.
 */
static int
_MHD_dumbClient_needs_process (const struct _MHD_dumbClient *clnt)
{
  switch (clnt->stage)
  {
  case DUMB_CLIENT_INIT:
  case DUMB_CLIENT_REQ_SENT:
  case DUMB_CLIENT_HEADER_RECVEIVED:
  case DUMB_CLIENT_BODY_RECVEIVED:
  case DUMB_CLIENT_FINISHED:
    return ! 0;
  case DUMB_CLIENT_CONNECTING:
  case DUMB_CLIENT_CONNECTED:
  case DUMB_CLIENT_REQ_SENDING:
  case DUMB_CLIENT_HEADER_RECVEIVING:
  case DUMB_CLIENT_BODY_RECVEIVING:
  case DUMB_CLIENT_FINISHING:
  default:
    return 0;
  }
  return 0; /* Should be unreachable */
}


/**
 * Process the client data with send()/recv() as needed.
 * @param clnt the client to process
 * @return non-zero if client finished processing the request,
 *         zero otherwise.
 */
int
_MHD_dumbClient_process (struct _MHD_dumbClient *clnt)
{
  do
  {
    switch (clnt->stage)
    {
    case DUMB_CLIENT_INIT:
      _MHD_dumbClient_connect_init (clnt);
      break;
    case DUMB_CLIENT_CONNECTING:
      _MHD_dumbClient_connect_finish (clnt);
      break;
    case DUMB_CLIENT_CONNECTED:
    case DUMB_CLIENT_REQ_SENDING:
      _MHD_dumbClient_send_req (clnt);
      break;
    case DUMB_CLIENT_REQ_SENT:
      mhd_assert (0);
      clnt->stage = DUMB_CLIENT_HEADER_RECVEIVING;
      break;
    case DUMB_CLIENT_HEADER_RECVEIVING:
      _MHD_dumbClient_recv_reply (clnt);
      break;
    case DUMB_CLIENT_HEADER_RECVEIVED:
      clnt->stage = DUMB_CLIENT_BODY_RECVEIVING;
      break;
    case DUMB_CLIENT_BODY_RECVEIVING:
      _MHD_dumbClient_recv_reply (clnt);
      break;
    case DUMB_CLIENT_BODY_RECVEIVED:
      clnt->stage = DUMB_CLIENT_FINISHING;
      break;
    case DUMB_CLIENT_FINISHING:
      _MHD_dumbClient_finalize (clnt);
      break;
    case DUMB_CLIENT_FINISHED:
      return ! 0;
    default:
      mhd_assert (0);
      mhdErrorExit ();
    }
  } while (_MHD_dumbClient_needs_process (clnt));
  return DUMB_CLIENT_FINISHED == clnt->stage;
}


void
_MHD_dumbClient_get_fdsets (struct _MHD_dumbClient *clnt,
                            MHD_socket *maxsckt,
                            fd_set *rs, fd_set *ws, fd_set *es)
{
  mhd_assert (NULL != rs);
  mhd_assert (NULL != ws);
  mhd_assert (NULL != es);
  if (DUMB_CLIENT_FINISHED > clnt->stage)
  {
    if (MHD_INVALID_SOCKET != clnt->sckt)
    {
      if ( (MHD_INVALID_SOCKET == *maxsckt) ||
           (clnt->sckt > *maxsckt) )
        *maxsckt = clnt->sckt;
      if (_MHD_dumbClient_needs_recv (clnt))
        FD_SET (clnt->sckt, rs);
      if (_MHD_dumbClient_needs_send (clnt))
        FD_SET (clnt->sckt, ws);
      FD_SET (clnt->sckt, es);
    }
  }
}


/**
 * Process the client data with send()/recv() as needed based on
 * information in fd_sets.
 * @param clnt the client to process
 * @return non-zero if client finished processing the request,
 *         zero otherwise.
 */
int
_MHD_dumbClient_process_from_fdsets (struct _MHD_dumbClient *clnt,
                                     fd_set *rs, fd_set *ws, fd_set *es)
{
  if (_MHD_dumbClient_needs_process (clnt))
    return _MHD_dumbClient_process (clnt);
  else if (MHD_INVALID_SOCKET != clnt->sckt)
  {
    if (_MHD_dumbClient_needs_recv (clnt) && FD_ISSET (clnt->sckt, rs))
      return _MHD_dumbClient_process (clnt);
    else if (_MHD_dumbClient_needs_send (clnt) && FD_ISSET (clnt->sckt, ws))
      return _MHD_dumbClient_process (clnt);
    else if (FD_ISSET (clnt->sckt, es))
      return _MHD_dumbClient_process (clnt);
  }
  return DUMB_CLIENT_FINISHED == clnt->stage;
}


/**
 * Perform full request.
 * @param clnt the client to run
 * @return zero if client finished processing the request,
 *         non-zero if timeout is reached.
 */
int
_MHD_dumbClient_perform (struct _MHD_dumbClient *clnt)
{
  time_t start;
  time_t now;
  start = time (NULL);
  now = start;
  do
  {
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket maxMhdSk;
    struct timeval tv;

    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);

    if (! _MHD_dumbClient_needs_process (clnt))
    {
      maxMhdSk = MHD_INVALID_SOCKET;
      _MHD_dumbClient_get_fdsets (clnt, &maxMhdSk, &rs, &ws, &es);
      mhd_assert (now >= start);
#ifndef _WIN32
      tv.tv_sec = (time_t) (TIMEOUTS_VAL * 2 - (now - start) + 1);
#else
      tv.tv_sec = (long) (TIMEOUTS_VAL * 2 - (now - start) + 1);
#endif
      tv.tv_usec = 250 * 1000;
      if (-1 == select ((int) (maxMhdSk + 1), &rs, &ws, &es, &tv))
      {
#ifdef MHD_POSIX_SOCKETS
        if (EINTR != errno)
          externalErrorExitDesc ("Unexpected select() error");
#else  /* ! MHD_POSIX_SOCKETS */
        mhd_assert ((0 != rs.fd_count) || (0 != ws.fd_count) || \
                    (0 != es.fd_count));
        externalErrorExitDesc ("Unexpected select() error");
        _MHD_sleep ((uint32_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000));
#endif /* ! MHD_POSIX_SOCKETS */
        continue;
      }
      if (_MHD_dumbClient_process_from_fdsets (clnt, &rs, &ws, &es))
        return 0;
    }
    /* Use double timeout value here as MHD must catch timeout situations
     * in this test. Timeout in client as a last resort. */
  } while ((now = time (NULL)) - start <= (TIMEOUTS_VAL * 2));
  return 1;
}


/**
 * Close the client and free internally allocated resources.
 * @param clnt the client to close
 */
void
_MHD_dumbClient_close (struct _MHD_dumbClient *clnt)
{
  if (DUMB_CLIENT_FINISHED != clnt->stage)
    _MHD_dumbClient_finalize (clnt);
  _MHD_dumbClient_socket_close (clnt);
  if (NULL != clnt->send_buf)
  {
    free ((void *) clnt->send_buf);
    clnt->send_buf = NULL;
  }
  free (clnt);
}


_MHD_NORETURN static void
socket_cb (void *cls,
           struct MHD_Connection *c,
           void **socket_context,
           enum MHD_ConnectionNotificationCode toe)
{
  (void) cls;            /* Unused */
  (void) socket_context; /* Unused */
  (void) toe;            /* Unused */

  MHD_suspend_connection (c); /* Should trigger panic */
  mhdErrorExitDesc ("Function \"MHD_suspend_connection()\" succeed, while " \
                    "it must fail as daemon was started without MHD_ALLOW_SUSPEND_RESUME " \
                    "flag");
}


struct ahc_cls_type
{
  const char *volatile rp_data;
  volatile size_t rp_data_size;
  const char *volatile rq_method;
  const char *volatile rq_url;
  const char *volatile req_body;
  volatile unsigned int cb_called; /* Non-zero indicates that callback was called at least one time */
  size_t req_body_size; /**< The number of bytes in @a req_body */
  size_t req_body_uploaded; /* Updated by callback */
};


static enum MHD_Result
ahcCheck (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **req_cls)
{
  static int marker;
  enum MHD_Result ret;
  struct ahc_cls_type *const param = (struct ahc_cls_type *) cls;
  (void) connection; /* Unused */

  if (NULL == param)
    mhdErrorExitDesc ("cls parameter is NULL");
  param->cb_called++;

  if (0 != strcmp (version, MHD_HTTP_VERSION_1_1))
    mhdErrorExitDesc ("Unexpected HTTP version");

  if (0 != strcmp (url, param->rq_url))
    mhdErrorExitDesc ("Unexpected URI");

  if (0 != strcmp (param->rq_method, method))
    mhdErrorExitDesc ("Unexpected request method");

  if (NULL == upload_data_size)
    mhdErrorExitDesc ("'upload_data_size' pointer is NULL");

  if (0 != *upload_data_size)
  {
    const char *const upload_body = param->req_body;
    if (NULL == upload_data)
      mhdErrorExitDesc ("'upload_data' is NULL while " \
                        "'*upload_data_size' value is not zero");
    if (NULL == upload_body)
      mhdErrorExitDesc ("'*upload_data_size' value is not zero " \
                        "while no request body is expected");
    if (param->req_body_uploaded + *upload_data_size > param->req_body_size)
    {
      fprintf (stderr, "Too large upload body received. Got %u, expected %u",
               (unsigned int) (param->req_body_uploaded + *upload_data_size),
               (unsigned int) param->req_body_size);
      mhdErrorExit ();
    }
    if (0 != memcmp (upload_data, upload_body + param->req_body_uploaded,
                     *upload_data_size))
    {
      fprintf (stderr, "Unexpected request body at offset %u: " \
               "'%.*s', expected: '%.*s'\n",
               (unsigned int) param->req_body_uploaded,
               (int) *upload_data_size, upload_data,
               (int) *upload_data_size, upload_body + param->req_body_uploaded);
      mhdErrorExit ();
    }
    param->req_body_uploaded += *upload_data_size;
    *upload_data_size = 0;
  }

  if (&marker != *req_cls)
  {
    /* The first call of the callback for this connection */
    mhd_assert (NULL == upload_data);
    param->req_body_uploaded = 0;

    *req_cls = &marker;
    return MHD_YES;
  }

  if (NULL != upload_data)
    return MHD_YES; /* Full request has not been received so far */

#if 0 /* Code unused in this test */
  struct MHD_Response *response;
  response = MHD_create_response_from_buffer (param->rp_data_size,
                                              (void *) param->rp_data,
                                              MHD_RESPMEM_MUST_COPY);
  if (NULL == response)
    mhdErrorExitDesc ("Failed to create response");

  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (MHD_YES != ret)
    mhdErrorExitDesc ("Failed to queue response");
#else
  if (NULL == upload_data)
    mhdErrorExitDesc ("Full request received, " \
                      "while incomplete request expected");
  ret = MHD_NO;
#endif

  return ret;
}


struct simpleQueryParams
{
  /* Destination path for HTTP query */
  const char *queryPath;

  /* Custom query method, NULL for default */
  const char *method;

  /* Destination port for HTTP query */
  uint16_t queryPort;

  /* Additional request headers, static */
  const char *headers;

  /* NULL for request without body */
  const uint8_t *req_body;
  size_t req_body_size;

  /* Non-zero to use chunked encoding for request body */
  int chunked;

  /* HTTP query result error flag */
  volatile int queryError;

  /* Response HTTP code, zero if no response */
  volatile int responseCode;
};


/* returns non-zero if timed-out */
static int
performQueryExternal (struct MHD_Daemon *d, struct _MHD_dumbClient *clnt)
{
  time_t start;
  struct timeval tv;
  int ret;
  const union MHD_DaemonInfo *di;
  MHD_socket lstn_sk;
  int client_accepted;
  int full_req_recieved;
  int full_req_sent;
  int some_data_recieved;

  di = MHD_get_daemon_info (d, MHD_DAEMON_INFO_LISTEN_FD);
  if (NULL == di)
    mhdErrorExitDesc ("Cannot get listener socket");
  lstn_sk = di->listen_fd;

  ret = 1; /* will be replaced with real result */
  client_accepted = 0;

  _MHD_dumbClient_start_connect (clnt);

  full_req_recieved = 0;
  some_data_recieved = 0;
  start = time (NULL);
  do
  {
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket maxMhdSk;
    int num_ready;
    int do_client; /**< Process data in client */

    maxMhdSk = MHD_INVALID_SOCKET;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (NULL == clnt)
    {
      /* client has finished, check whether MHD is still
       * processing any connections */
      full_req_sent = 1;
      do_client = 0;
      if (client_accepted && (0 > MHD_get_timeout64s (d)))
      {
        ret = 0;
        break; /* MHD finished as well */
      }
    }
    else
    {
      full_req_sent = _MHD_dumbClient_is_req_sent (clnt);
      if (! full_req_sent)
        do_client = 1; /* Request hasn't been sent yet, send the data */
      else
      {
        /* All request data has been sent.
         * Client will close the socket as the next step. */
        if (full_req_recieved)
          do_client = 1; /* All data has been received by the MHD */
        else if (some_data_recieved)
        {
          /* at least something was received by the MHD */
          do_client = 1;
        }
        else
        {
          /* The MHD must receive at least something before closing
           * the connection. */
          do_client = 0;
        }
      }

      if (do_client)
        _MHD_dumbClient_get_fdsets (clnt, &maxMhdSk, &rs, &ws, &es);
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxMhdSk))
      mhdErrorExitDesc ("MHD_get_fdset() failed");
    if (do_client)
    {
      tv.tv_sec = 1;
      tv.tv_usec = 250 * 1000;
    }
    else
    { /* Request completely sent but not yet fully received */
      tv.tv_sec = 0;
      tv.tv_usec = FINAL_PACKETS_MS * 1000;
    }
    num_ready = select ((int) (maxMhdSk + 1), &rs, &ws, &es, &tv);
    if (-1 == num_ready)
    {
#ifdef MHD_POSIX_SOCKETS
      if (EINTR != errno)
        externalErrorExitDesc ("Unexpected select() error");
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
        externalErrorExitDesc ("Unexpected select() error");
      _MHD_sleep ((uint32_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000));
#endif
      continue;
    }
    if (0 == num_ready)
    { /* select() finished by timeout, looks like no more packets are pending */
      if (do_client)
        externalErrorExitDesc ("Timeout waiting for sockets");
      if (full_req_sent && (! full_req_recieved))
        full_req_recieved = 1;
    }
    if (full_req_recieved)
      mhdErrorExitDesc ("Full request has been received by MHD, while it "
                        "must be aborted by the panic function");
    if (MHD_YES != MHD_run_from_select (d, &rs, &ws, &es))
      mhdErrorExitDesc ("MHD_run_from_select() failed");
    if (! client_accepted)
      client_accepted = FD_ISSET (lstn_sk, &rs);
    else
    { /* Client connection was already accepted by MHD */
      if (! some_data_recieved)
      {
        if (! do_client)
        {
          if (0 != num_ready)
          { /* Connection was accepted before, "ready" socket means data */
            some_data_recieved = 1;
          }
        }
        else
        {
          if (2 == num_ready)
            some_data_recieved = 1;
          else if ((1 == num_ready) &&
                   ((MHD_INVALID_SOCKET == clnt->sckt) ||
                    ! FD_ISSET (clnt->sckt, &ws)))
            some_data_recieved = 1;
        }
      }
    }
    if (do_client)
    {
      if (_MHD_dumbClient_process_from_fdsets (clnt, &rs, &ws, &es))
        clnt = NULL;
    }
    /* Use double timeout value here so MHD would be able to catch timeout
     * internally */
  } while (time (NULL) - start <= (TIMEOUTS_VAL * 2));

  return ret;
}


/* Returns zero for successful response and non-zero for failed response */
static int
doClientQueryInThread (struct MHD_Daemon *d,
                       struct simpleQueryParams *p)
{
  const union MHD_DaemonInfo *dinfo;
  struct _MHD_dumbClient *c;
  int errornum;
  int use_external_poll;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_FLAGS);
  if (NULL == dinfo)
    mhdErrorExitDesc ("MHD_get_daemon_info() failed");
  use_external_poll = (0 == (dinfo->flags
                             & MHD_USE_INTERNAL_POLLING_THREAD));

  if (0 == p->queryPort)
    externalErrorExit ();

  c = _MHD_dumbClient_create (p->queryPort, p->method, p->queryPath,
                              p->headers, p->req_body, p->req_body_size,
                              p->chunked);
  _MHD_dumbClient_set_send_limits (c, 1, 0);

  /* 'internal' polling should not be used in this test */
  mhd_assert (use_external_poll);
  if (! use_external_poll)
    errornum = _MHD_dumbClient_perform (c);
  else
    errornum = performQueryExternal (d, c);

  if (errornum)
    fprintf (stderr, "Request timeout out.\n");
  else
    mhdErrorExitDesc ("Request succeed, but it must fail");

  _MHD_dumbClient_close (c);

  return errornum;
}


/* Perform test queries, shut down MHD daemon, and free parameters */
static unsigned int
performTestQueries (struct MHD_Daemon *d, uint16_t d_port,
                    struct ahc_cls_type *ahc_param)
{
  struct simpleQueryParams qParam;

  /* Common parameters, to be individually overridden by specific test cases
   * if needed */
  qParam.queryPort = d_port;
  qParam.method = MHD_HTTP_METHOD_PUT;
  qParam.queryPath = EXPECTED_URI_BASE_PATH;
  qParam.headers = REQ_HEADER_CT;
  qParam.req_body = (const uint8_t *) REQ_BODY;
  qParam.req_body_size = MHD_STATICSTR_LEN_ (REQ_BODY);
  qParam.chunked = 0;

  ahc_param->rq_url = EXPECTED_URI_BASE_PATH;
  ahc_param->rq_method = MHD_HTTP_METHOD_PUT;
  ahc_param->rp_data = "~";
  ahc_param->rp_data_size = 1;
  ahc_param->req_body = (const char *) qParam.req_body;
  ahc_param->req_body_size = qParam.req_body_size;

  /* Make sure that maximum size is tested */
  /* To be updated by callbacks */
  ahc_param->cb_called = 0;

  if (0 != doClientQueryInThread (d, &qParam))
    fprintf (stderr, "FAILED: client query failed.");

  MHD_stop_daemon (d);
  free (ahc_param);

  return 1; /* Always error if reached this point */
}


enum testMhdThreadsType
{
  testMhdThreadExternal              = 0,
  testMhdThreadInternal              = MHD_USE_INTERNAL_POLLING_THREAD,
  testMhdThreadInternalPerConnection = MHD_USE_THREAD_PER_CONNECTION
                                       | MHD_USE_INTERNAL_POLLING_THREAD,
  testMhdThreadInternalPool
};

enum testMhdPollType
{
  testMhdPollBySelect = 0,
  testMhdPollByPoll   = MHD_USE_POLL,
  testMhdPollByEpoll  = MHD_USE_EPOLL,
  testMhdPollAuto     = MHD_USE_AUTO
};

/* Get number of threads for thread pool depending
 * on used poll function and test type. */
static unsigned int
testNumThreadsForPool (enum testMhdPollType pollType)
{
  unsigned int numThreads = MHD_CPU_COUNT;
  (void) pollType; /* Don't care about pollType for this test */
  return numThreads; /* No practical limit for non-cleanup test */
}


#define PANIC_MAGIC_CHECK 1133

_MHD_NORETURN static void
myPanicCallback (void *cls,
                 const char *file,
                 unsigned int line,
                 const char *reason)
{
  int *const param = (int *) cls;
  if (NULL == cls)
    mhdErrorExitDesc ("The 'cls' parameter is NULL");
  if (PANIC_MAGIC_CHECK != *param)
    mhdErrorExitDesc ("Wrong '*cls' value");
#ifdef HAVE_MESSAGES
  if (NULL == file)
    mhdErrorExitDesc ("The 'file' parameter is NULL");
  if (NULL == reason)
    mhdErrorExitDesc ("The 'reason' parameter is NULL");
#else  /* ! HAVE_MESSAGES */
  if (NULL != file)
    mhdErrorExitDesc ("The 'file' parameter is not NULL");
  if (NULL != reason)
    mhdErrorExitDesc ("The 'reason' parameter is not NULL");
#endif /* ! HAVE_MESSAGES */
  fflush (stderr);
  fflush (stdout);
  printf ("User panic function has been called from file '%s' at line '%u' "
          "with the reason:\n%s", file, line,
          ((NULL != reason) ? reason : "(NULL)\n"));
  fflush (stdout);
  exit (0);
}


static struct MHD_Daemon *
startTestMhdDaemon (enum testMhdThreadsType thrType,
                    enum testMhdPollType pollType, uint16_t *pport,
                    struct ahc_cls_type **ahc_param)
{
  struct MHD_Daemon *d;
  const union MHD_DaemonInfo *dinfo;
  static int magic_panic_param = PANIC_MAGIC_CHECK;

  if (NULL == ahc_param)
    externalErrorExit ();

  *ahc_param = (struct ahc_cls_type *) malloc (sizeof(struct ahc_cls_type));
  if (NULL == *ahc_param)
    externalErrorExit ();

  if ( (0 == *pport) &&
       (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT)) )
  {
    *pport = 4190;
  }

  MHD_set_panic_func (&myPanicCallback, (void *) &magic_panic_param);

  if (testMhdThreadExternal == thrType)
    d = MHD_start_daemon (((unsigned int) pollType)
                          | (verbose ? MHD_USE_ERROR_LOG : 0)
                          | MHD_USE_NO_THREAD_SAFETY,
                          *pport, NULL, NULL,
                          &ahcCheck, *ahc_param,
                          MHD_OPTION_NOTIFY_CONNECTION, &socket_cb,
                          NULL,
                          MHD_OPTION_CONNECTION_TIMEOUT,
                          (unsigned) TIMEOUTS_VAL,
                          MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                          MHD_OPTION_END);
  else if (testMhdThreadInternalPool != thrType)
    d = MHD_start_daemon (((unsigned int) thrType) | ((unsigned int) pollType)
                          | (verbose ? MHD_USE_ERROR_LOG : 0),
                          *pport, NULL, NULL,
                          &ahcCheck, *ahc_param,
                          MHD_OPTION_NOTIFY_CONNECTION, &socket_cb,
                          NULL,
                          MHD_OPTION_CONNECTION_TIMEOUT,
                          (unsigned) TIMEOUTS_VAL,
                          MHD_OPTION_END);
  else
    d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD
                          | ((unsigned int) pollType)
                          | (verbose ? MHD_USE_ERROR_LOG : 0),
                          *pport, NULL, NULL,
                          &ahcCheck, *ahc_param,
                          MHD_OPTION_THREAD_POOL_SIZE,
                          testNumThreadsForPool (pollType),
                          MHD_OPTION_NOTIFY_CONNECTION, &socket_cb,
                          NULL,
                          MHD_OPTION_CONNECTION_TIMEOUT,
                          (unsigned) TIMEOUTS_VAL,
                          MHD_OPTION_END);

  if (NULL == d)
    mhdErrorExitDesc ("Failed to start MHD daemon");

  if (0 == *pport)
  {
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port))
      mhdErrorExitDesc ("MHD_get_daemon_info() failed");
    *pport = dinfo->port;
    if (0 == global_port)
      global_port = *pport; /* Reuse the same port for all tests */
  }

  return d;
}


/* Test runners */


static unsigned int
testExternalGet (void)
{
  struct MHD_Daemon *d;
  uint16_t d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;

  d = startTestMhdDaemon (testMhdThreadExternal, testMhdPollBySelect, &d_port,
                          &ahc_param);

  return performTestQueries (d, d_port, ahc_param);
}


#if 0 /* disabled runners, not suitable for this test */
static unsigned int
testInternalGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  uint16_t d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;
  struct term_notif_cb_param *term_result;

  d = startTestMhdDaemon (testMhdThreadInternal, pollType, &d_port,
                          &ahc_param, &uri_cb_param, &term_result);

  return performTestQueries (d, d_port, ahc_param, uri_cb_param, term_result);
}


static int
testMultithreadedGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  uint16_t d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;
  struct term_notif_cb_param *term_result;

  d = startTestMhdDaemon (testMhdThreadInternalPerConnection, pollType, &d_port,
                          &ahc_param, &uri_cb_param);
  return performTestQueries (d, d_port, ahc_param, uri_cb_param, term_result);
}


static unsigned int
testMultithreadedPoolGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  uint16_t d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;
  struct term_notif_cb_param *term_result;

  d = startTestMhdDaemon (testMhdThreadInternalPool, pollType, &d_port,
                          &ahc_param, &uri_cb_param);
  return performTestQueries (d, d_port, ahc_param, uri_cb_param, term_result);
}


#endif /* disabled runners, not suitable for this test */

int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  unsigned int test_result = 0;
  verbose = 0;

  (void) has_in_name; /* Unused, mute compiler warning */
  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));

  test_global_init ();

  /* Could be set to non-zero value to enforce using specific port
   * in the test */
  global_port = 0;
  test_result = testExternalGet ();
  if (test_result)
    fprintf (stderr, "FAILED: testExternalGet (). Result: %u.\n", test_result);
  else if (verbose)
    printf ("PASSED: testExternalGet ().\n");
  errorCount += test_result;
#if 0 /* disabled runners, not suitable for this test */
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    test_result = testInternalGet (testMhdPollAuto);
    if (test_result)
      fprintf (stderr, "FAILED: testInternalGet (testMhdPollAuto). "
               "Result: %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testInternalGet (testMhdPollBySelect).\n");
    errorCount += test_result;
#ifdef _MHD_HEAVY_TESTS
    /* Actually tests are not heavy, but took too long to complete while
     * not really provide any additional results. */
    test_result = testInternalGet (testMhdPollBySelect);
    if (test_result)
      fprintf (stderr, "FAILED: testInternalGet (testMhdPollBySelect). "
               "Result: %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testInternalGet (testMhdPollBySelect).\n");
    errorCount += test_result;
    test_result = testMultithreadedPoolGet (testMhdPollBySelect);
    if (test_result)
      fprintf (stderr,
               "FAILED: testMultithreadedPoolGet (testMhdPollBySelect). "
               "Result: %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testMultithreadedPoolGet (testMhdPollBySelect).\n");
    errorCount += test_result;
    test_result = testMultithreadedGet (testMhdPollBySelect);
    if (test_result)
      fprintf (stderr,
               "FAILED: testMultithreadedGet (testMhdPollBySelect). "
               "Result: %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testMultithreadedGet (testMhdPollBySelect).\n");
    errorCount += test_result;
    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_POLL))
    {
      test_result = testInternalGet (testMhdPollByPoll);
      if (test_result)
        fprintf (stderr, "FAILED: testInternalGet (testMhdPollByPoll). "
                 "Result: %u.\n",
                 test_result);
      else if (verbose)
        printf ("PASSED: testInternalGet (testMhdPollByPoll).\n");
      errorCount += test_result;
    }
    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_EPOLL))
    {
      test_result = testInternalGet (testMhdPollByEpoll);
      if (test_result)
        fprintf (stderr, "FAILED: testInternalGet (testMhdPollByEpoll). "
                 "Result: %u.\n",
                 test_result);
      else if (verbose)
        printf ("PASSED: testInternalGet (testMhdPollByEpoll).\n");
      errorCount += test_result;
    }
#else
    /* Mute compiler warnings */
    (void) testMultithreadedGet;
    (void) testMultithreadedPoolGet;
#endif /* _MHD_HEAVY_TESTS */
  }
#endif /* disabled runners, not suitable for this test */
  if (0 != errorCount)
    fprintf (stderr,
             "Error (code: %u)\n",
             errorCount);
  else if (verbose)
    printf ("All tests passed.\n");

  test_global_cleanup ();

  return (errorCount == 0) ? 0 : 1;       /* 0 == pass */
}
