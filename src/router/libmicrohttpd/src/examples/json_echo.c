/*
     This file is part of libmicrohttpd
     Copyright (C) 2025 Christian Grothoff (and other contributing authors)

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
 * @file json_echo.c
 * @brief example for processing POST requests with JSON uploads, echos the JSON back to the client
 * @author Christian Grothoff
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <microhttpd.h>
#include <jansson.h>

/**
 * Bad request page.
 */
#define BAD_REQUEST_ERROR \
  "<html><head><title>Illegal request</title></head><body>Go away.</body></html>"

/**
 * Invalid JSON page.
 */
#define NOT_FOUND_ERROR \
  "<html><head><title>Not found</title></head><body>Go away.</body></html>"


/**
 * State we keep for each request.
 */
struct Request
{

  /**
   * Number of bytes received.
   */
  size_t off;

  /**
   * Size of @a buf.
   */
  size_t len;

  /**
   * Buffer for POST data.
   */
  void *buf;

};


/**
 * Handler used to generate a 404 reply.
 *
 * @param connection connection to use
 */
static enum MHD_Result
not_found_page (struct MHD_Connection *connection)
{
  struct MHD_Response *response;
  enum MHD_Result ret;

  response =
    MHD_create_response_from_buffer_static (strlen (NOT_FOUND_ERROR),
                                            (const void *) NOT_FOUND_ERROR);
  if (NULL == response)
    return MHD_NO;
  ret = MHD_queue_response (connection,
                            MHD_HTTP_NOT_FOUND,
                            response);
  if (MHD_YES !=
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_CONTENT_ENCODING,
                               "text/html"))
  {
    fprintf (stderr,
             "Failed to set content encoding header!\n");
  }
  MHD_destroy_response (response);
  return ret;
}


/**
 * Handler used to generate a 400 reply.
 *
 * @param connection connection to use
 */
static enum MHD_Result
invalid_request (struct MHD_Connection *connection)
{
  enum MHD_Result ret;
  struct MHD_Response *response;

  response =
    MHD_create_response_from_buffer_static (
      strlen (BAD_REQUEST_ERROR),
      (const void *) BAD_REQUEST_ERROR);
  if (NULL == response)
    return MHD_NO;
  ret = MHD_queue_response (connection,
                            MHD_HTTP_BAD_REQUEST,
                            response);
  if (MHD_YES !=
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_CONTENT_ENCODING,
                               "text/html"))
  {
    fprintf (stderr,
             "Failed to set content encoding header!\n");
  }
  MHD_destroy_response (response);
  return ret;
}


/**
 * Main MHD callback for handling requests.
 *
 * @param cls argument given together with the function
 *        pointer when the handler was registered with MHD
 * @param connection handle identifying the incoming connection
 * @param url the requested url
 * @param method the HTTP method used ("GET", "PUT", etc.)
 * @param version the HTTP version string (i.e. "HTTP/1.1")
 * @param upload_data the data being uploaded (excluding HEADERS,
 *        for a POST that fits into memory and that is encoded
 *        with a supported encoding, the POST data will NOT be
 *        given in upload_data and is instead available as
 *        part of MHD_get_connection_values; very large POST
 *        data *will* be made available incrementally in
 *        upload_data)
 * @param upload_data_size set initially to the size of the
 *        upload_data provided; the method must update this
 *        value to the number of bytes NOT processed;
 * @param req_cls pointer that the callback can set to some
 *        address and that will be preserved by MHD for future
 *        calls for this request; since the access handler may
 *        be called many times (i.e., for a PUT/POST operation
 *        with plenty of upload data) this allows the application
 *        to easily associate some request-specific state.
 *        If necessary, this state can be cleaned up in the
 *        global "MHD_RequestCompleted" callback (which
 *        can be set with the MHD_OPTION_NOTIFY_COMPLETED).
 *        Initially, <tt>*req_cls</tt> will be NULL.
 * @return MHS_YES if the connection was handled successfully,
 *         MHS_NO if the socket must be closed due to a serious
 *         error while handling the request
 */
static enum MHD_Result
create_response (void *cls,
                 struct MHD_Connection *connection,
                 const char *url,
                 const char *method,
                 const char *version,
                 const char *upload_data,
                 size_t *upload_data_size,
                 void **req_cls)
{
  struct Request *request = *req_cls;
  struct MHD_Response *response;
  enum MHD_Result ret;
  unsigned int i;

  (void) cls;               /* Unused. Silence compiler warning. */
  (void) version;           /* Unused. Silence compiler warning. */

  if (NULL == request)
  {
    const char *clen;
    char dummy;
    unsigned int len;

    request = calloc (1, sizeof (struct Request));
    if (NULL == request)
    {
      fprintf (stderr,
               "calloc error: %s\n",
               strerror (errno));
      return MHD_NO;
    }
    *req_cls = request;
    if (0 != strcmp (method,
                     MHD_HTTP_METHOD_POST))
    {
      return not_found_page (connection);
    }
    clen = MHD_lookup_connection_value (
      connection,
      MHD_HEADER_KIND,
      MHD_HTTP_HEADER_CONTENT_LENGTH);
    if (NULL == clen)
      return invalid_request (connection);
    if (1 != sscanf (clen,
                     "%u%c",
                     &len,
                     &dummy))
      return invalid_request (connection);
    request->len = len;
    request->buf = malloc (request->len);
    if (NULL == request->buf)
      return MHD_NO;
    return MHD_YES;
  }
  if (0 != *upload_data_size)
  {
    if (request->len < *upload_data_size + request->off)
    {
      fprintf (stderr,
               "Content-length header wrong, aborting\n");
      return MHD_NO;
    }
    memcpy (request->buf,
            upload_data,
            *upload_data_size);
    request->off += *upload_data_size;
    *upload_data_size = 0;
    return MHD_YES;
  }
  {
    json_t *j;
    json_error_t err;
    char *s;

    j = json_loadb (request->buf,
                    request->len,
                    0,
                    &err);
    if (NULL == j)
      return invalid_request (connection);
    s = json_dumps (j,
                    JSON_INDENT (2));
    json_decref (j);
    response =
      MHD_create_response_from_buffer (strlen (s),
                                       s,
                                       MHD_RESPMEM_MUST_FREE);
    ret = MHD_queue_response (connection,
                              MHD_HTTP_OK,
                              response);
    MHD_destroy_response (response);
    return ret;
  }
}


/**
 * Callback called upon completion of a request.
 * Decrements session reference counter.
 *
 * @param cls not used
 * @param connection connection that completed
 * @param req_cls session handle
 * @param toe status code
 */
static void
request_completed_callback (void *cls,
                            struct MHD_Connection *connection,
                            void **req_cls,
                            enum MHD_RequestTerminationCode toe)
{
  struct Request *request = *req_cls;
  (void) cls;         /* Unused. Silence compiler warning. */
  (void) connection;  /* Unused. Silence compiler warning. */
  (void) toe;         /* Unused. Silence compiler warning. */

  if (NULL == request)
    return;
  if (NULL != request->buf)
    free (request->buf);
  free (request);
}


/**
 * Call with the port number as the only argument.
 * Never terminates (other than by signals, such as CTRL-C).
 */
int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *d;
  struct timeval tv;
  struct timeval *tvp;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket max;
  uint64_t mhd_timeout;
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
  /* initialize PRNG */
  srand ((unsigned int) time (NULL));
  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        (uint16_t) port,
                        NULL, NULL,
                        &create_response, NULL,
                        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
                        MHD_OPTION_NOTIFY_COMPLETED,
                        &request_completed_callback, NULL,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);
  if (NULL == d)
    return 1;
  while (1)
  {
    max = 0;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
      break; /* fatal internal error */
    if (MHD_get_timeout64 (d, &mhd_timeout) == MHD_YES)
    {
#if ! defined(_WIN32) || defined(__CYGWIN__)
      tv.tv_sec = (time_t) (mhd_timeout / 1000LL);
#else  /* Native W32 */
      tv.tv_sec = (long) (mhd_timeout / 1000LL);
#endif /* Native W32 */
      tv.tv_usec = ((long) (mhd_timeout % 1000)) * 1000;
      tvp = &tv;
    }
    else
      tvp = NULL;
    if (-1 == select ((int) max + 1, &rs, &ws, &es, tvp))
    {
      if (EINTR != errno)
        abort ();
    }
    MHD_run (d);
  }
  MHD_stop_daemon (d);
  return 0;
}
