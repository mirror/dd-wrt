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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 8888

#define REALM     "Maintenance"
#define USER      "a legitimate user"
#define PASSWORD  "and his password"

#define SERVERKEYFILE "server.key"
#define SERVERCERTFILE "server.pem"


static size_t
get_file_size (const char *filename)
{
  FILE *fp;

  fp = fopen (filename, "rb");
  if (fp)
  {
    long size;

    if ((0 != fseek (fp, 0, SEEK_END)) || (-1 == (size = ftell (fp))))
      size = 0;

    fclose (fp);

    return (size_t) size;
  }
  else
    return 0;
}


static char *
load_file (const char *filename)
{
  FILE *fp;
  char *buffer;
  size_t size;

  size = get_file_size (filename);
  if (0 == size)
    return NULL;

  fp = fopen (filename, "rb");
  if (! fp)
    return NULL;

  buffer = malloc (size + 1);
  if (! buffer)
  {
    fclose (fp);
    return NULL;
  }
  buffer[size] = '\0';

  if (size != fread (buffer, 1, size, fp))
  {
    free (buffer);
    buffer = NULL;
  }

  fclose (fp);
  return buffer;
}


static enum MHD_Result
ask_for_authentication (struct MHD_Connection *connection, const char *realm)
{
  enum MHD_Result ret;
  struct MHD_Response *response;

  response = MHD_create_response_empty (MHD_RF_NONE);
  if (! response)
    return MHD_NO;

  ret = MHD_queue_basic_auth_required_response3 (connection,
                                                 realm,
                                                 MHD_YES,
                                                 response);
  MHD_destroy_response (response);
  return ret;
}


static int
is_authenticated (struct MHD_Connection *connection,
                  const char *username,
                  const char *password)
{
  struct MHD_BasicAuthInfo *auth_info;
  int authenticated;

  auth_info = MHD_basic_auth_get_username_password3 (connection);
  if (NULL == auth_info)
    return 0;
  authenticated =
    ( (strlen (username) == auth_info->username_len) &&
      (0 == memcmp (auth_info->username, username, auth_info->username_len)) &&
      /* The next check against NULL is optional,
       * if 'password' is NULL then 'password_len' is always zero. */
      (NULL != auth_info->password) &&
      (strlen (password) == auth_info->password_len) &&
      (0 == memcmp (auth_info->password, password, auth_info->password_len)) );

  MHD_free (auth_info);

  return authenticated;
}


static enum MHD_Result
secret_page (struct MHD_Connection *connection)
{
  enum MHD_Result ret;
  struct MHD_Response *response;
  const char *page = "<html><body>A secret.</body></html>";

  response = MHD_create_response_from_buffer_static (strlen (page), page);
  if (! response)
    return MHD_NO;

  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);

  return ret;
}


static enum MHD_Result
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **req_cls)
{
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

  if (! is_authenticated (connection, USER, PASSWORD))
    return ask_for_authentication (connection, REALM);

  return secret_page (connection);
}


int
main (void)
{
  struct MHD_Daemon *daemon;
  char *key_pem;
  char *cert_pem;

  key_pem = load_file (SERVERKEYFILE);
  cert_pem = load_file (SERVERCERTFILE);

  if ((key_pem == NULL) || (cert_pem == NULL))
  {
    printf ("The key/certificate files could not be read.\n");
    if (NULL != key_pem)
      free (key_pem);
    if (NULL != cert_pem)
      free (cert_pem);
    return 1;
  }

  daemon =
    MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS, PORT, NULL,
                      NULL, &answer_to_connection, NULL,
                      MHD_OPTION_HTTPS_MEM_KEY, key_pem,
                      MHD_OPTION_HTTPS_MEM_CERT, cert_pem, MHD_OPTION_END);
  if (NULL == daemon)
  {
    printf ("%s\n", cert_pem);

    free (key_pem);
    free (cert_pem);

    return 1;
  }

  (void) getchar ();

  MHD_stop_daemon (daemon);
  free (key_pem);
  free (cert_pem);

  return 0;
}
