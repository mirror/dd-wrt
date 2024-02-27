/* Feel free to use this example code in any way
   you see fit (Public Domain) */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <microhttpd_ws.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define PORT 80

#define PAGE \
  "<!DOCTYPE html>\n" \
  "<html>\n" \
  "<head>\n" \
  "<meta charset=\"UTF-8\">\n" \
  "<title>Websocket Demo</title>\n" \
  "<script>\n" \
  "\n" \
  "let url = 'ws' + (window.location.protocol === 'https:' ? 's' : '')" \
    "  + ':/" "/' +\n" \
  "          window.location.host + '/chat';\n" \
  "let socket = null;\n" \
  "\n" \
  "window.onload = function(event) {\n" \
  "  socket = new WebSocket(url);\n" \
  "  socket.onopen = function(event) {\n" \
  "    document.write('The websocket connection has been " \
    "established.<br>');\n" \
  "\n" \
  "    /" "/ Send some text\n" \
  "    socket.send('Hello from JavaScript!');\n" \
  "  }\n" \
  "\n" \
  "  socket.onclose = function(event) {\n" \
  "    document.write('The websocket connection has been closed.<br>');\n" \
  "  }\n" \
  "\n" \
  "  socket.onerror = function(event) {\n" \
  "    document.write('An error occurred during the websocket " \
    "communication.<br>');\n" \
  "  }\n" \
  "\n" \
  "  socket.onmessage = function(event) {\n" \
  "    document.write('Websocket message received: ' + " \
    "event.data + '<br>');\n" \
  "  }\n" \
  "}\n" \
  "\n" \
  "</script>\n" \
  "</head>\n" \
  "<body>\n" \
  "</body>\n" \
  "</html>"

#define PAGE_NOT_FOUND \
  "404 Not Found"

#define PAGE_INVALID_WEBSOCKET_REQUEST \
  "Invalid WebSocket request!"

static void
send_all (MHD_socket fd,
          const char *buf,
          size_t len);

static void
make_blocking (MHD_socket fd);

static void
upgrade_handler (void *cls,
                 struct MHD_Connection *connection,
                 void *req_cls,
                 const char *extra_in,
                 size_t extra_in_size,
                 MHD_socket fd,
                 struct MHD_UpgradeResponseHandle *urh)
{
  /* make the socket blocking (operating-system-dependent code) */
  make_blocking (fd);

  /* create a websocket stream for this connection */
  struct MHD_WebSocketStream *ws;
  int result = MHD_websocket_stream_init (&ws,
                                          0,
                                          0);
  if (0 != result)
  {
    /* Couldn't create the websocket stream.
     * So we close the socket and leave
     */
    MHD_upgrade_action (urh,
                        MHD_UPGRADE_ACTION_CLOSE);
    return;
  }

  /* Let's wait for incoming data */
  const size_t buf_len = 256;
  char buf[buf_len];
  ssize_t got;
  while (MHD_WEBSOCKET_VALIDITY_VALID == MHD_websocket_stream_is_valid (ws))
  {
    got = recv (fd,
                buf,
                buf_len,
                0);
    if (0 >= got)
    {
      /* the TCP/IP socket has been closed */
      break;
    }

    /* parse the entire received data */
    size_t buf_offset = 0;
    while (buf_offset < (size_t) got)
    {
      size_t new_offset = 0;
      char *frame_data = NULL;
      size_t frame_len  = 0;
      int status = MHD_websocket_decode (ws,
                                         buf + buf_offset,
                                         ((size_t) got) - buf_offset,
                                         &new_offset,
                                         &frame_data,
                                         &frame_len);
      if (0 > status)
      {
        /* an error occurred and the connection must be closed */
        if (NULL != frame_data)
        {
          MHD_websocket_free (ws, frame_data);
        }
        break;
      }
      else
      {
        buf_offset += new_offset;
        if (0 < status)
        {
          /* the frame is complete */
          switch (status)
          {
          case MHD_WEBSOCKET_STATUS_TEXT_FRAME:
            /* The client has sent some text.
             * We will display it and answer with a text frame.
             */
            if (NULL != frame_data)
            {
              printf ("Received message: %s\n", frame_data);
              MHD_websocket_free (ws, frame_data);
              frame_data = NULL;
            }
            result = MHD_websocket_encode_text (ws,
                                                "Hello",
                                                5,  /* length of "Hello" */
                                                0,
                                                &frame_data,
                                                &frame_len,
                                                NULL);
            if (0 == result)
            {
              send_all (fd,
                        frame_data,
                        frame_len);
            }
            break;

          case MHD_WEBSOCKET_STATUS_CLOSE_FRAME:
            /* if we receive a close frame, we will respond with one */
            MHD_websocket_free (ws,
                                frame_data);
            frame_data = NULL;

            result = MHD_websocket_encode_close (ws,
                                                 0,
                                                 NULL,
                                                 0,
                                                 &frame_data,
                                                 &frame_len);
            if (0 == result)
            {
              send_all (fd,
                        frame_data,
                        frame_len);
            }
            break;

          case MHD_WEBSOCKET_STATUS_PING_FRAME:
            /* if we receive a ping frame, we will respond */
            /* with the corresponding pong frame */
            {
              char *pong = NULL;
              size_t pong_len = 0;
              result = MHD_websocket_encode_pong (ws,
                                                  frame_data,
                                                  frame_len,
                                                  &pong,
                                                  &pong_len);
              if (0 == result)
              {
                send_all (fd,
                          pong,
                          pong_len);
              }
              MHD_websocket_free (ws,
                                  pong);
            }
            break;

          default:
            /* Other frame types are ignored
             * in this minimal example.
             * This is valid, because they become
             * automatically skipped if we receive them unexpectedly
             */
            break;
          }
        }
        if (NULL != frame_data)
        {
          MHD_websocket_free (ws, frame_data);
        }
      }
    }
  }

  /* free the websocket stream */
  MHD_websocket_stream_free (ws);

  /* close the socket when it is not needed anymore */
  MHD_upgrade_action (urh,
                      MHD_UPGRADE_ACTION_CLOSE);
}


/* This helper function is used for the case that
 * we need to resend some data
 */
static void
send_all (MHD_socket fd,
          const char *buf,
          size_t len)
{
  ssize_t ret;
  size_t off;

  for (off = 0; off < len; off += ret)
  {
    ret = send (fd,
                &buf[off],
                (int) (len - off),
                0);
    if (0 > ret)
    {
      if (EAGAIN == errno)
      {
        ret = 0;
        continue;
      }
      break;
    }
    if (0 == ret)
      break;
  }
}


/* This helper function contains operating-system-dependent code and
 * is used to make a socket blocking.
 */
static void
make_blocking (MHD_socket fd)
{
#ifndef _WIN32
  int flags;

  flags = fcntl (fd, F_GETFL);
  if (-1 == flags)
    abort ();
  if ((flags & ~O_NONBLOCK) != flags)
    if (-1 == fcntl (fd, F_SETFL, flags & ~O_NONBLOCK))
      abort ();
#else  /* _WIN32 */
  unsigned long flags = 0;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    abort ();
#endif /* _WIN32 */
}


static enum MHD_Result
access_handler (void *cls,
                struct MHD_Connection *connection,
                const char *url,
                const char *method,
                const char *version,
                const char *upload_data,
                size_t *upload_data_size,
                void **req_cls)
{
  static int aptr;
  struct MHD_Response *response;
  int ret;

  (void) cls;               /* Unused. Silent compiler warning. */
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

  if (0 == strcmp (url, "/"))
  {
    /* Default page for visiting the server */
    struct MHD_Response *response;
    response = MHD_create_response_from_buffer_static (strlen (PAGE),
                                                       PAGE);
    ret = MHD_queue_response (connection,
                              MHD_HTTP_OK,
                              response);
    MHD_destroy_response (response);
  }
  else if (0 == strcmp (url, "/chat"))
  {
    char is_valid = 1;
    const char *value = NULL;
    char sec_websocket_accept[29];

    if (0 != MHD_websocket_check_http_version (version))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_CONNECTION);
    if (0 != MHD_websocket_check_connection_header (value))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_UPGRADE);
    if (0 != MHD_websocket_check_upgrade_header (value))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_SEC_WEBSOCKET_VERSION);
    if (0 != MHD_websocket_check_version_header (value))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_SEC_WEBSOCKET_KEY);
    if (0 != MHD_websocket_create_accept_header (value, sec_websocket_accept))
    {
      is_valid = 0;
    }

    if (1 == is_valid)
    {
      /* upgrade the connection */
      response = MHD_create_response_for_upgrade (&upgrade_handler,
                                                  NULL);
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_UPGRADE,
                               "websocket");
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_SEC_WEBSOCKET_ACCEPT,
                               sec_websocket_accept);
      ret = MHD_queue_response (connection,
                                MHD_HTTP_SWITCHING_PROTOCOLS,
                                response);
      MHD_destroy_response (response);
    }
    else
    {
      /* return error page */
      struct MHD_Response *response;
      response =
        MHD_create_response_from_buffer_static (strlen (
                                                  PAGE_INVALID_WEBSOCKET_REQUEST),
                                                PAGE_INVALID_WEBSOCKET_REQUEST);
      ret = MHD_queue_response (connection,
                                MHD_HTTP_BAD_REQUEST,
                                response);
      MHD_destroy_response (response);
    }
  }
  else
  {
    struct MHD_Response *response;
    response =
      MHD_create_response_from_buffer_static (strlen (PAGE_NOT_FOUND),
                                              PAGE_NOT_FOUND);
    ret = MHD_queue_response (connection,
                              MHD_HTTP_NOT_FOUND,
                              response);
    MHD_destroy_response (response);
  }

  return ret;
}


int
main (int argc,
      char *const *argv)
{
  (void) argc;               /* Unused. Silent compiler warning. */
  (void) argv;               /* Unused. Silent compiler warning. */
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD
                             | MHD_USE_THREAD_PER_CONNECTION
                             | MHD_ALLOW_UPGRADE
                             | MHD_USE_ERROR_LOG,
                             PORT, NULL, NULL,
                             &access_handler, NULL,
                             MHD_OPTION_END);

  if (NULL == daemon)
    return 1;
  (void) getc (stdin);

  MHD_stop_daemon (daemon);

  return 0;
}
