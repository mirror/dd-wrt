/*
     This file is part of libmicrohttpd
     Copyright (C) 2018 Christian Grothoff (and other contributing authors)
     Copyright (C) 2022 Evgeny Grin (Karlson2k)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
 * @file suspend_resume_epoll.c
 * @brief example for how to use libmicrohttpd with epoll() and
 *        resume a suspended connection
 * @author Robert D Kocisko
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */
#include "platform.h"
#include <microhttpd.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <limits.h>
#include <errno.h>

#define TIMEOUT_INFINITE -1

struct Request
{
  struct MHD_Connection *connection;
  int timerfd;
};


static int epfd;

static struct epoll_event evt;


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **req_cls)
{
  struct MHD_Response *response;
  enum MHD_Result ret;
  struct Request *req;
  struct itimerspec ts;

  (void) cls;
  (void) method;
  (void) version;           /* Unused. Silence compiler warning. */
  (void) upload_data;       /* Unused. Silence compiler warning. */
  (void) upload_data_size;  /* Unused. Silence compiler warning. */
  req = *req_cls;
  if (NULL == req)
  {

    req = malloc (sizeof(struct Request));
    if (NULL == req)
      return MHD_NO;
    req->connection = connection;
    req->timerfd = -1;
    *req_cls = req;
    return MHD_YES;
  }

  if (-1 != req->timerfd)
  {
    /* send response (echo request url) */
    response = MHD_create_response_from_buffer_copy (strlen (url),
                                                     (const void *) url);
    if (NULL == response)
      return MHD_NO;
    ret = MHD_queue_response (connection,
                              MHD_HTTP_OK,
                              response);
    MHD_destroy_response (response);
    return ret;
  }
  /* create timer and suspend connection */
  req->timerfd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
  if (-1 == req->timerfd)
  {
    printf ("timerfd_create: %s", strerror (errno));
    return MHD_NO;
  }
  evt.events = EPOLLIN;
  evt.data.ptr = req;
  if (-1 == epoll_ctl (epfd, EPOLL_CTL_ADD, req->timerfd, &evt))
  {
    printf ("epoll_ctl: %s", strerror (errno));
    return MHD_NO;
  }
  ts.it_value.tv_sec = 1;
  ts.it_value.tv_nsec = 0;
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 0;
  if (-1 == timerfd_settime (req->timerfd, 0, &ts, NULL))
  {
    printf ("timerfd_settime: %s", strerror (errno));
    return MHD_NO;
  }
  MHD_suspend_connection (connection);
  return MHD_YES;
}


static void
connection_done (void *cls,
                 struct MHD_Connection *connection,
                 void **req_cls,
                 enum MHD_RequestTerminationCode toe)
{
  struct Request *req = *req_cls;

  (void) cls;
  (void) connection;
  (void) toe;
  if (-1 != req->timerfd)
    if (0 != close (req->timerfd))
      abort ();
  free (req);
}


int
main (int argc,
      char *const *argv)
{
  struct MHD_Daemon *d;
  const union MHD_DaemonInfo *info;
  int current_event_count;
  struct epoll_event events_list[1];
  struct Request *req;
  uint64_t timer_expirations;
  int port;

  if (argc != 2)
  {
    printf ("%s PORT\n", argv[0]);
    return 1;
  }
  port = atoi (argv[1]);
  if ( (1 > port) || (port > 65535) )
  {
    fprintf (stderr,
             "Port must be a number between 1 and 65535.\n");
    return 1;
  }
  d = MHD_start_daemon (MHD_USE_EPOLL | MHD_ALLOW_SUSPEND_RESUME,
                        (uint16_t) port,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_NOTIFY_COMPLETED, &connection_done, NULL,
                        MHD_OPTION_END);
  if (d == NULL)
    return 1;

  info = MHD_get_daemon_info (d, MHD_DAEMON_INFO_EPOLL_FD);
  if (info == NULL)
    return 1;

  epfd = epoll_create1 (EPOLL_CLOEXEC);
  if (-1 == epfd)
    return 1;

  evt.events = EPOLLIN;
  evt.data.ptr = NULL;
  if (-1 == epoll_ctl (epfd, EPOLL_CTL_ADD, info->epoll_fd, &evt))
    return 1;

  while (1)
  {
    current_event_count = epoll_wait (epfd, events_list, 1,
                                      MHD_get_timeout_i (d));

    if (1 == current_event_count)
    {
      if (events_list[0].data.ptr)
      {
        /*  A timer has timed out */
        req = events_list[0].data.ptr;
        /* read from the fd so the system knows we heard the notice */
        if (-1 == read (req->timerfd, &timer_expirations,
                        sizeof(timer_expirations)))
        {
          return 1;
        }
        /*  Now resume the connection */
        MHD_resume_connection (req->connection);
      }
    }
    else if (0 == current_event_count)
    {
      /* no events: continue */
    }
    else
    {
      /* error */
      return 1;
    }
    if (! MHD_run (d))
      return 1;
  }

  return 0;
}
