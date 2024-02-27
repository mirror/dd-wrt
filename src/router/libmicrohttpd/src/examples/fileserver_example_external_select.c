/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2008 Christian Grothoff (and other contributing authors)
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
 * @file fileserver_example_external_select.c
 * @brief minimal example for how to use libmicrohttpd to server files
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include <microhttpd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define PAGE \
  "<html><head><title>File not found</title></head><body>File not found</body></html>"

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = (FILE *) cls;
  size_t bytes_read;

  /* 'fseek' may not support files larger 2GiB, depending on platform.
   * For production code, make sure that 'pos' has valid values, supported by
   * 'fseek', or use 'fseeko' or similar function. */
  if (0 != fseek (file, (long) pos, SEEK_SET))
    return MHD_CONTENT_READER_END_WITH_ERROR;
  bytes_read = fread (buf, 1, max, file);
  if (0 == bytes_read)
    return (0 != ferror (file)) ? MHD_CONTENT_READER_END_WITH_ERROR :
           MHD_CONTENT_READER_END_OF_STREAM;
  return (ssize_t) bytes_read;
}


static void
free_callback (void *cls)
{
  FILE *file = cls;
  fclose (file);
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size, void **req_cls)
{
  static int aptr;
  struct MHD_Response *response;
  enum MHD_Result ret;
  FILE *file;
  int fd;
  struct stat buf;
  (void) cls;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *req_cls)
  {
    /* do never respond on first call */
    *req_cls = &aptr;
    return MHD_YES;
  }
  *req_cls = NULL;                  /* reset when done */

  file = fopen (&url[1], "rb");
  if (NULL != file)
  {
    fd = fileno (file);
    if (-1 == fd)
    {
      (void) fclose (file);
      return MHD_NO;     /* internal error */
    }
    if ( (0 != fstat (fd, &buf)) ||
         (! S_ISREG (buf.st_mode)) )
    {
      /* not a regular file, refuse to serve */
      fclose (file);
      file = NULL;
    }
  }

  if (NULL == file)
  {
    response = MHD_create_response_from_buffer_static (strlen (PAGE),
                                                       PAGE);
    ret = MHD_queue_response (connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response (response);
  }
  else
  {
    response = MHD_create_response_from_callback ((size_t) buf.st_size,
                                                  32 * 1024, /* 32k page size */
                                                  &file_reader,
                                                  file,
                                                  &free_callback);
    if (NULL == response)
    {
      fclose (file);
      return MHD_NO;
    }
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
  }
  return ret;
}


int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *d;
  time_t end;
  time_t t;
  struct timeval tv;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket max;
  uint64_t mhd_timeout;
  int port;

  if (argc != 3)
  {
    printf ("%s PORT SECONDS-TO-RUN\n", argv[0]);
    return 1;
  }
  port = atoi (argv[1]);
  if ( (1 > port) || (port > 65535) )
  {
    fprintf (stderr,
             "Port must be a number between 1 and 65535.\n");
    return 1;
  }

  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        (uint16_t) port,
                        NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);
  if (d == NULL)
    return 1;
  end = time (NULL) + atoi (argv[2]);
  while ((t = time (NULL)) < end)
  {
#if ! defined(_WIN32) || defined(__CYGWIN__)
    tv.tv_sec = end - t;
#else  /* Native W32 */
    tv.tv_sec = (long) (end - t);
#endif /* Native W32 */
    tv.tv_usec = 0;
    max = 0;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
      break; /* fatal internal error */
    if (MHD_get_timeout64 (d, &mhd_timeout) == MHD_YES)
    {
      if (((uint64_t) tv.tv_sec) < mhd_timeout / 1000LL)
      {
#if ! defined(_WIN32) || defined(__CYGWIN__)
        tv.tv_sec = (time_t) (mhd_timeout / 1000LL);
#else  /* Native W32 */
        tv.tv_sec = (long) (mhd_timeout / 1000LL);
#endif /* Native W32 */
        tv.tv_usec = ((long) (mhd_timeout % 1000)) * 1000;
      }
    }
    if (-1 == select ((int) max + 1, &rs, &ws, &es, &tv))
    {
      if (EINTR != errno)
        abort ();
    }
    MHD_run (d);
  }
  MHD_stop_daemon (d);
  return 0;
}
