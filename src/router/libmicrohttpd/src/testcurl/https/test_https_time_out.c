/*
 This file is part of libmicrohttpd
 Copyright (C) 2007 Christian Grothoff
 Copyright (C) 2014-2021 Karlson2k (Evgeny Grin)

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
 * @file test_https_time_out.c
 * @brief: daemon TLS alert response test-case
 *
 * @author Sagie Amir
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include "microhttpd.h"
#include "tls_test_common.h"
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */
#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */

#include "mhd_sockets.h" /* only macros used */


#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

static const int TIME_OUT = 2;

static unsigned int num_connects = 0;
static unsigned int num_disconnects = 0;


/**
 * Pause execution for specified number of milliseconds.
 * @param ms the number of milliseconds to sleep
 */
void
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
  sleep ((ms + 999) / 1000);
#endif
}


void
socket_cb (void *cls,
           struct MHD_Connection *c,
           void **socket_context,
           enum MHD_ConnectionNotificationCode toe)
{
  struct sckt_notif_cb_param *param = (struct sckt_notif_cb_param *) cls;
  if (NULL == socket_context)
    abort ();
  if (NULL == c)
    abort ();
  if (NULL == param)
    abort ();

  if (MHD_CONNECTION_NOTIFY_STARTED == toe)
    num_connects++;
  else if (MHD_CONNECTION_NOTIFY_CLOSED == toe)
    num_disconnects++;
  else
    abort ();
}


static int
test_tls_session_time_out (gnutls_session_t session, int port)
{
  int ret;
  MHD_socket sd;
  struct sockaddr_in sa;

  sd = socket (AF_INET, SOCK_STREAM, 0);
  if (sd == MHD_INVALID_SOCKET)
  {
    fprintf (stderr, "Failed to create socket: %s\n", strerror (errno));
    return 2;
  }

  memset (&sa, '\0', sizeof (struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  sa.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

  ret = connect (sd, (struct sockaddr *) &sa, sizeof (struct sockaddr_in));

  if (ret < 0)
  {
    fprintf (stderr, "Error: %s\n", MHD_E_FAILED_TO_CONNECT);
    MHD_socket_close_chk_ (sd);
    return 2;
  }

#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030109) && ! defined(_WIN64)
  gnutls_transport_set_int (session, (int) (sd));
#else  /* GnuTLS before 3.1.9 or Win64 */
  gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) (intptr_t) (sd));
#endif /* GnuTLS before 3.1.9 or Win64 */

  ret = gnutls_handshake (session);
  if (ret < 0)
  {
    fprintf (stderr, "Handshake failed\n");
    MHD_socket_close_chk_ (sd);
    return 2;
  }

  _MHD_sleep (TIME_OUT * 1000 + 1200);

  /* check that server has closed the connection */
  if (1 == num_disconnects)
  {
    fprintf (stderr, "Connection failed to time-out\n");
    MHD_socket_close_chk_ (sd);
    return 1;
  }
  else if (0 != num_disconnects)
    abort ();

  MHD_socket_close_chk_ (sd);
  return 0;
}


int
main (int argc, char *const *argv)
{
  int errorCount = 0;
  struct MHD_Daemon *d;
  gnutls_session_t session;
  gnutls_certificate_credentials_t xcred;
  int port;
  (void) argc;   /* Unused. Silent compiler warning. */

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 3070;

#ifdef MHD_SEND_SPIPE_SUPPRESS_NEEDED
#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
  if (SIG_ERR == signal (SIGPIPE, SIG_IGN))
  {
    fprintf (stderr, "Error suppressing SIGPIPE signal.\n");
    exit (99);
  }
#else /* ! HAVE_SIGNAL_H || ! SIGPIPE */
  fprintf (stderr, "Cannot suppress SIGPIPE signal.\n");
  /* exit (77); */
#endif
#endif /* MHD_SEND_SPIPE_SUPPRESS_NEEDED */

#ifdef MHD_HTTPS_REQUIRE_GCRYPT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
  if (GNUTLS_E_SUCCESS != gnutls_global_init ())
  {
    fprintf (stderr, "Cannot initialize GnuTLS.\n");
    exit (99);
  }
  gnutls_global_set_log_level (11);

  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS
                        | MHD_USE_ERROR_LOG, port,
                        NULL, NULL, &http_dummy_ahc, NULL,
                        MHD_OPTION_CONNECTION_TIMEOUT, TIME_OUT,
                        MHD_OPTION_HTTPS_MEM_KEY, srv_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_self_signed_cert_pem,
                        MHD_OPTION_END);

  if (NULL == d)
  {
    fprintf (stderr, MHD_E_SERVER_INIT);
    return -1;
  }
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return -1;
    }
    port = (int) dinfo->port;
  }

  if (0 != setup_session (&session, &xcred))
  {
    fprintf (stderr, "failed to setup session\n");
    return 1;
  }
  errorCount += test_tls_session_time_out (session, port);
  teardown_session (session, xcred);

  print_test_result (errorCount, argv[0]);

  MHD_stop_daemon (d);
  gnutls_global_deinit ();

  return errorCount != 0 ? 1 : 0;
}
