/* Feel free to use this example code in any way
   you see fit (Public Domain) */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PORT 8888


static enum MHD_Result
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **req_cls)
{
  struct MHD_BasicAuthInfo *auth_info;
  enum MHD_Result ret;
  struct MHD_Response *response;
  (void) cls;               /* Unused. Silent compiler warning. */
  (void) url;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, "GET"))
    return MHD_NO;
  if (NULL == *req_cls)
  {
    *req_cls = connection;
    return MHD_YES;
  }
  auth_info = MHD_basic_auth_get_username_password3 (connection);
  if (NULL == auth_info)
  {
    static const char *page =
      "<html><body>Authorization required</body></html>";
    response = MHD_create_response_from_buffer_static (strlen (page), page);
    ret = MHD_queue_basic_auth_required_response3 (connection,
                                                   "admins",
                                                   MHD_YES,
                                                   response);
  }
  else if ((strlen ("root") != auth_info->username_len) ||
           (0 != memcmp (auth_info->username, "root",
                         auth_info->username_len)) ||
           /* The next check against NULL is optional,
            * if 'password' is NULL then 'password_len' is always zero. */
           (NULL == auth_info->password) ||
           (strlen ("pa$$w0rd") != auth_info->password_len) ||
           (0 != memcmp (auth_info->password, "pa$$w0rd",
                         auth_info->password_len)))
  {
    static const char *page =
      "<html><body>Wrong username or password</body></html>";
    response = MHD_create_response_from_buffer_static (strlen (page), page);
    ret = MHD_queue_basic_auth_required_response3 (connection,
                                                   "admins",
                                                   MHD_YES,
                                                   response);
  }
  else
  {
    static const char *page = "<html><body>A secret.</body></html>";
    response = MHD_create_response_from_buffer_static (strlen (page), page);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  }
  if (NULL != auth_info)
    MHD_free (auth_info);
  MHD_destroy_response (response);
  return ret;
}


int
main (void)
{
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
  if (NULL == daemon)
    return 1;

  (void) getchar ();

  MHD_stop_daemon (daemon);
  return 0;
}
