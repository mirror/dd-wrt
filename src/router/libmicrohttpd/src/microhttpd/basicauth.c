/*
     This file is part of libmicrohttpd
     Copyright (C) 2010, 2011, 2012 Daniel Pittman and Christian Grothoff

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
 * @file basicauth.c
 * @brief Implements HTTP basic authentication methods
 * @author Amr Ali
 * @author Matthieu Speder
 */
#include "platform.h"
#include "mhd_limits.h"
#include "internal.h"
#include "mhd_str.h"
#include "mhd_compat.h"

/**
 * Beginning string for any valid Basic authentication header.
 */
#define _BASIC_BASE   "Basic "


/**
 * Get the username and password from the basic authorization header sent by the client
 *
 * @param connection The MHD connection structure
 * @param password a pointer for the password
 * @return NULL if no username could be found, a pointer
 *      to the username if found
 * @ingroup authentication
 */
char *
MHD_basic_auth_get_username_password (struct MHD_Connection *connection,
                                      char **password)
{
  const char *header;
  size_t enc_size;
  size_t value_size;
  size_t dec_size;
  char *decode;
  char *separator;

  if (NULL != password)
    *password = NULL;

  if (MHD_NO ==
      MHD_lookup_connection_value_n (connection,
                                     MHD_HEADER_KIND,
                                     MHD_HTTP_HEADER_AUTHORIZATION,
                                     MHD_STATICSTR_LEN_ ( \
                                       MHD_HTTP_HEADER_AUTHORIZATION),
                                     &header,
                                     &value_size))
    return NULL;

  if (0 != strncmp (header,
                    _BASIC_BASE,
                    MHD_STATICSTR_LEN_ (_BASIC_BASE)))
    return NULL;

  header += MHD_STATICSTR_LEN_ (_BASIC_BASE);
  enc_size = value_size - MHD_STATICSTR_LEN_ (_BASIC_BASE);
  if (0 != (enc_size % 4))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Bad length of basic authentication value.\n"));
#endif
    return NULL;
  }
  dec_size = MHD_base64_max_dec_size_ (enc_size);
  decode = (char *) malloc (dec_size + 1);
  if (NULL == decode)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Failed to allocate memory.\n"));
#endif
    return NULL;
  }
  dec_size = MHD_base64_to_bin_n (header, enc_size, decode, dec_size);
  if (0 != dec_size)
  {
    decode[dec_size] = 0; /* Zero-terminate */
    /* Find user:password pattern */
    separator = memchr (decode, ':', dec_size);
    if (NULL != separator)
    {
      *separator = 0; /* Zero-terminate 'username' */
      if (NULL == password)
        return decode;  /* Success exit point */
      else
      {
        *password = strdup (separator + 1);
        if (NULL != *password)
          return decode; /* Success exit point */
#ifdef HAVE_MESSAGES
        else
        {
          MHD_DLOG (connection->daemon,
                    _ ("Failed to allocate memory for password.\n"));
        }
#endif /* HAVE_MESSAGES */
      }
    }
#ifdef HAVE_MESSAGES
    else
    {
      MHD_DLOG (connection->daemon,
                _ ("Basic authentication doesn't contain ':' separator.\n"));
    }
#endif /* HAVE_MESSAGES */
  }
#ifdef HAVE_MESSAGES
  else
  {
    MHD_DLOG (connection->daemon,
              _ ("Error decoding basic authentication.\n"));
  }
#endif /* HAVE_MESSAGES */
  free (decode);
  return NULL;  /* Failure exit point */
}


/**
 * Queues a response to request basic authentication from the client.
 * The given response object is expected to include the payload for
 * the response; the "WWW-Authenticate" header will be added and the
 * response queued with the 'UNAUTHORIZED' status code.
 *
 * @param connection The MHD connection structure
 * @param realm the realm presented to the client
 * @param response response object to modify and queue
 * @return #MHD_YES on success, #MHD_NO otherwise
 * @ingroup authentication
 */
enum MHD_Result
MHD_queue_basic_auth_fail_response (struct MHD_Connection *connection,
                                    const char *realm,
                                    struct MHD_Response *response)
{
  enum MHD_Result ret;
  int res;
  size_t hlen = strlen (realm) + strlen ("Basic realm=\"\"") + 1;
  char *header;

  header = (char *) malloc (hlen);
  if (NULL == header)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              "Failed to allocate memory for auth header.\n");
#endif /* HAVE_MESSAGES */
    return MHD_NO;
  }
  res = MHD_snprintf_ (header,
                       hlen,
                       "Basic realm=\"%s\"",
                       realm);
  if ((res > 0) && ((size_t) res < hlen))
    ret = MHD_add_response_header (response,
                                   MHD_HTTP_HEADER_WWW_AUTHENTICATE,
                                   header);
  else
    ret = MHD_NO;

  free (header);
  if (MHD_NO != ret)
  {
    ret = MHD_queue_response (connection,
                              MHD_HTTP_UNAUTHORIZED,
                              response);
  }
  else
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Failed to add Basic auth header.\n"));
#endif /* HAVE_MESSAGES */
  }
  return ret;
}


/* end of basicauth.c */
