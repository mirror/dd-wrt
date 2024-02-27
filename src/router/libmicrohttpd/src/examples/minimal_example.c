/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Christian Grothoff (and other contributing authors)
     Copyright (C) 2014-2022 Evgeny Grin (Karlson2k)

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
 * @file minimal_example.c
 * @brief minimal example for how to use libmicrohttpd
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include <microhttpd.h>

#define PAGE \
  "<html><head><title>libmicrohttpd demo</title></head>" \
  "<body>libmicrohttpd demo</body></html>"

struct handler_param
{
  const char *response_page;
};

static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **req_cls)
{
  static int aptr;
  struct handler_param *param = (struct handler_param *) cls;
  struct MHD_Response *response;
  enum MHD_Result ret;

  (void) url;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, "GET"))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *req_cls)
  {
    /* do never respond on first call */
    *req_cls = &aptr;
    return MHD_YES;
  }
  *req_cls = NULL;                  /* reset when done */
  response =
    MHD_create_response_from_buffer_static (strlen (param->response_page),
                                            param->response_page);
  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  return ret;
}


int
main (int argc,
      char *const *argv)
{
  struct MHD_Daemon *d;
  struct handler_param data_for_handler;
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
  data_for_handler.response_page = PAGE;
  d = MHD_start_daemon (/* MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG, */
    MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
    /* MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG | MHD_USE_POLL, */
    /* MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG | MHD_USE_POLL, */
    /* MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG, */
    (uint16_t) port,
    NULL, NULL, &ahc_echo, &data_for_handler,
    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
    MHD_OPTION_END);
  if (d == NULL)
    return 1;
  (void) getc (stdin);
  MHD_stop_daemon (d);
  return 0;
}
