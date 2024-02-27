/*
 This file is part of libmicrohttpd
 Copyright (C) 2007 Christian Grothoff

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
 * @file test_options.c
 * @brief  Testcase for libmicrohttpd HTTPS GET operations
 * @author Sagie Amir
 */

#include "platform.h"
#include "microhttpd.h"
#include "mhd_sockets.h"

static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **req_cls)
{
  (void) cls;
  (void) connection;
  (void) url;
  (void) method;
  (void) version;
  (void) upload_data;
  (void) upload_data_size;
  (void) req_cls;

  return 0;
}


static unsigned int
test_wrap_loc (const char *test_name, unsigned int (*test)(void))
{
  unsigned int ret;

  fprintf (stdout, "running test: %s ", test_name);
  ret = test ();
  if (ret == 0)
  {
    fprintf (stdout, "[pass]\n");
  }
  else
  {
    fprintf (stdout, "[fail]\n");
  }
  return ret;
}


/**
 * Test daemon initialization with the MHD_OPTION_SOCK_ADDR or
 * the MHD_OPTION_SOCK_ADDR_LEN options
 */
static unsigned int
test_ip_addr_option (void)
{
  struct MHD_Daemon *d;
  const union MHD_DaemonInfo *dinfo;
  struct sockaddr_in daemon_ip_addr;
  uint16_t port4;
#if defined(HAVE_INET6) && defined(USE_IPV6_TESTING)
  struct sockaddr_in6 daemon_ip_addr6;
  uint16_t port6;
#endif
  unsigned int ret;

  memset (&daemon_ip_addr, 0, sizeof (struct sockaddr_in));
  daemon_ip_addr.sin_family = AF_INET;
  daemon_ip_addr.sin_port = 0;
  daemon_ip_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  port4 = 0;

#if defined(HAVE_INET6) && defined(USE_IPV6_TESTING)
  memset (&daemon_ip_addr6, 0, sizeof (struct sockaddr_in6));
  daemon_ip_addr6.sin6_family = AF_INET6;
  daemon_ip_addr6.sin6_port = 0;
  daemon_ip_addr6.sin6_addr = in6addr_loopback;
  port6 = 0;
#endif

  ret = 0;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR, &daemon_ip_addr,
                        MHD_OPTION_END);

  if (d == 0)
    return 1;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
  if (NULL == dinfo)
    ret |= 1 << 1;
  else
    port4 = dinfo->port;

  MHD_stop_daemon (d);


  daemon_ip_addr.sin_port = htons (port4);
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) sizeof(daemon_ip_addr), &daemon_ip_addr,
                        MHD_OPTION_END);

  if (d == 0)
    return 1;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
  if (NULL == dinfo)
    ret |= 1 << 1;
  else if (port4 != dinfo->port)
    ret |= 1 << 2;

  MHD_stop_daemon (d);


  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) (sizeof(daemon_ip_addr) / 2),
                        &daemon_ip_addr,
                        MHD_OPTION_END);

  if (NULL != d)
  {
    MHD_stop_daemon (d);
    return 1 << 3;
  }

#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN

  daemon_ip_addr.sin_len = (socklen_t) sizeof(daemon_ip_addr);
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) sizeof(daemon_ip_addr), &daemon_ip_addr,
                        MHD_OPTION_END);

  if (d == 0)
    return 1;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
  if (NULL == dinfo)
    ret |= 1 << 1;
  else if (port4 != dinfo->port)
    ret |= 1 << 2;

  MHD_stop_daemon (d);


  daemon_ip_addr.sin_len = (socklen_t) (sizeof(daemon_ip_addr) / 2);
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) sizeof(daemon_ip_addr), &daemon_ip_addr,
                        MHD_OPTION_END);

  if (NULL != d)
  {
    MHD_stop_daemon (d);
    return 1 << 3;
  }

#endif /* HAVE_STRUCT_SOCKADDR_IN_SIN_LEN */


#if defined(HAVE_INET6) && defined(USE_IPV6_TESTING)
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_IPv6
                        | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR, &daemon_ip_addr6,
                        MHD_OPTION_END);

  if (d == 0)
    return 1;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
  if (NULL == dinfo)
    ret |= 1 << 1;
  else
    port6 = dinfo->port;

  MHD_stop_daemon (d);


  daemon_ip_addr6.sin6_port = htons (port6);
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) sizeof(daemon_ip_addr6), &daemon_ip_addr6,
                        MHD_OPTION_END);

  if (d == 0)
    return 1;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
  if (NULL == dinfo)
    ret |= 1 << 1;
  else if (port6 != dinfo->port)
    ret |= 1 << 2;

  MHD_stop_daemon (d);


  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) (sizeof(daemon_ip_addr6) / 2),
                        &daemon_ip_addr6,
                        MHD_OPTION_END);

  if (NULL != d)
  {
    MHD_stop_daemon (d);
    return 1 << 3;
  }

#if defined(HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN)

  daemon_ip_addr6.sin6_len = (socklen_t) sizeof(daemon_ip_addr6);
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) sizeof(daemon_ip_addr6), &daemon_ip_addr6,
                        MHD_OPTION_END);

  if (d == 0)
    return 1;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
  if (NULL == dinfo)
    ret |= 1 << 1;
  else if (port6 != dinfo->port)
    ret |= 1 << 2;

  MHD_stop_daemon (d);


  daemon_ip_addr6.sin6_len = (socklen_t) (sizeof(daemon_ip_addr6) / 2);
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY, 0,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_SOCK_ADDR_LEN,
                        (socklen_t) sizeof(daemon_ip_addr6), &daemon_ip_addr6,
                        MHD_OPTION_END);

  if (NULL != d)
  {
    MHD_stop_daemon (d);
    return 1 << 3;
  }

#endif /* HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN */
#endif /* HAVE_INET6 && USE_IPV6_TESTING */

  return ret;
}


/* setup a temporary transfer test file */
int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */

  errorCount += test_wrap_loc ("ip addr option", &test_ip_addr_option);

  return errorCount != 0;
}
