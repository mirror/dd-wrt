/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2008 Christian Grothoff (and other contributing authors)
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

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
 * @file refuse_post_example.c
 * @brief example for how to refuse a POST request properly
 * @author Christian Grothoff and Sebastian Gerhardt
 * @author Karlson2k (Evgeny Grin)
 */
#include "platform.h"
#include <microhttpd.h>

struct handler_param
{
  const char *response_page;
};

static const char *askpage =
  "<html><body>\n\
 Upload a file, please!<br>\n\
 <form action=\"/filepost\" method=\"post\" enctype=\"multipart/form-data\">\n\
 <input name=\"file\" type=\"file\">\n\
 <input type=\"submit\" value=\" Send \"></form>\n\
 </body></html>";

#define BUSYPAGE \
  "<html><head><title>Webserver busy</title></head>" \
  "<body>We are too busy to process POSTs right now.</body></html>"

static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **req_cls)
{
  static int aptr;
  struct handler_param *param = (struct handler_param *) cls;
  struct MHD_Response *response;
  enum MHD_Result ret;
  (void) cls;               /* Unused. Silent compiler warning. */
  (void) url;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if ((0 != strcmp (method, "GET")) && (0 != strcmp (method, "POST")))
    return MHD_NO;              /* unexpected method */

  if (&aptr != *req_cls)
  {
    *req_cls = &aptr;

    /* always to busy for POST requests */
    if (0 == strcmp (method, "POST"))
    {
      response =
        MHD_create_response_from_buffer_static (strlen (BUSYPAGE),
                                                (const void *) BUSYPAGE);
      ret =
        MHD_queue_response (connection, MHD_HTTP_SERVICE_UNAVAILABLE,
                            response);
      MHD_destroy_response (response);
      return ret;
    }
  }

  *req_cls = NULL;                  /* reset when done */
  response =
    MHD_create_response_from_buffer_static (strlen (param->response_page),
                                            (const void *)
                                            param->response_page);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


int
main (int argc, char *const *argv)
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
  data_for_handler.response_page = askpage;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        (uint16_t) port,
                        NULL, NULL, &ahc_echo, &data_for_handler,
                        MHD_OPTION_END);
  if (d == NULL)
    return 1;
  (void) getc (stdin);
  MHD_stop_daemon (d);
  return 0;
}


/* end of refuse_post_example.c */
