/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2017 Christian Grothoff
     Copyright (C) 2014--2023  Evgeny Grin (Karlson2k)

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
 * @file test_daemon.c
 * @brief  Testcase for libmicrohttpd starts and stops
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include "microhttpd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef WINDOWS
#include <unistd.h>
#endif


static unsigned int
testStartError (void)
{
  struct MHD_Daemon *d;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG, 0, NULL, NULL, NULL, NULL);
  if (NULL != d)
  {
    MHD_stop_daemon (d);
    fprintf (stderr,
             "Succeeded to start without MHD_AccessHandlerCallback?\n");
    return 1;
  }
  return 0;
}


static enum MHD_Result
apc_nothing (void *cls,
             const struct sockaddr *addr,
             socklen_t addrlen)
{
  (void) cls; (void) addr; (void) addrlen; /* Unused. Silent compiler warning. */

  return MHD_NO;
}


static enum MHD_Result
apc_all (void *cls,
         const struct sockaddr *addr,
         socklen_t addrlen)
{
  (void) cls; (void) addr; (void) addrlen; /* Unused. Silent compiler warning. */

  return MHD_YES;
}


static enum MHD_Result
ahc_nothing (void *cls,
             struct MHD_Connection *connection,
             const char *url,
             const char *method,
             const char *version,
             const char *upload_data, size_t *upload_data_size,
             void **req_cls)
{
  (void) cls; (void) connection; (void) url;         /* Unused. Silent compiler warning. */
  (void) method; (void) version; (void) upload_data; /* Unused. Silent compiler warning. */
  (void) upload_data_size; (void) req_cls;           /* Unused. Silent compiler warning. */

  return MHD_NO;
}


static unsigned int
testStartStop (void)
{
  struct MHD_Daemon *d;

  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        0,
                        &apc_nothing, NULL,
                        &ahc_nothing, NULL,
                        MHD_OPTION_END);
  if (NULL == d)
  {
    fprintf (stderr,
             "Failed to start daemon on port %u\n",
             (unsigned int) 0);
    exit (3);
  }
  MHD_stop_daemon (d);
  return 0;
}


static unsigned int
testExternalRun (int use_no_thread_safe)
{
  struct MHD_Daemon *d;
  fd_set rs;
  MHD_socket maxfd;
  int i;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG
                        | (use_no_thread_safe ? MHD_USE_NO_THREAD_SAFETY : 0),
                        0,
                        &apc_all, NULL,
                        &ahc_nothing, NULL,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);

  if (NULL == d)
  {
    fprintf (stderr,
             "Failed to start daemon on port %u\n",
             (unsigned int) 0);
    exit (3);
  }
  for (i = 0; i < 15; ++i)
  {
    maxfd = 0;
    FD_ZERO (&rs);
    if (MHD_YES != MHD_get_fdset (d, &rs, &rs, &rs, &maxfd))
    {
      MHD_stop_daemon (d);
      fprintf (stderr,
               "Failed in MHD_get_fdset().\n");
      return 256;
    }
    if (MHD_NO == MHD_run (d))
    {
      MHD_stop_daemon (d);
      fprintf (stderr,
               "Failed in MHD_run().\n");
      return 8;
    }
  }
  MHD_stop_daemon (d);
  return 0;
}


static unsigned int
testThread (void)
{
  struct MHD_Daemon *d;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_INTERNAL_POLLING_THREAD,
                        0,
                        &apc_all, NULL,
                        &ahc_nothing, NULL,
                        MHD_OPTION_END);

  if (NULL == d)
  {
    fprintf (stderr,
             "Failed to start daemon on port %u.\n",
             (unsigned int) 0);
    exit (3);
  }
  if (MHD_run (d) != MHD_NO)
  {
    fprintf (stderr,
             "Failed in MHD_run().\n");
    return 32;
  }
  MHD_stop_daemon (d);
  return 0;
}


static unsigned int
testMultithread (void)
{
  struct MHD_Daemon *d;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_INTERNAL_POLLING_THREAD
                        | MHD_USE_THREAD_PER_CONNECTION,
                        0,
                        &apc_all, NULL,
                        &ahc_nothing, NULL,
                        MHD_OPTION_END);

  if (NULL == d)
  {
    fprintf (stderr,
             "Failed to start daemon on port %u\n",
             (unsigned int) 0);
    exit (3);
  }
  if (MHD_run (d) != MHD_NO)
  {
    fprintf (stderr,
             "Failed in MHD_run().\n");
    return 128;
  }
  MHD_stop_daemon (d);
  return 0;
}


int
main (int argc,
      char *const *argv)
{
  unsigned int errorCount = 0;
  int has_threads_support;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */

  has_threads_support =
    (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_THREADS));
  errorCount += testStartError ();
  if (has_threads_support)
    errorCount += testStartStop ();
  if (has_threads_support)
    errorCount += testExternalRun (0);
  errorCount += testExternalRun (! 0);
  if (has_threads_support)
  {
    errorCount += testThread ();
    errorCount += testMultithread ();
  }
  if (0 != errorCount)
    fprintf (stderr,
             "Error (code: %u)\n",
             errorCount);
  return 0 != errorCount;       /* 0 == pass */
}
