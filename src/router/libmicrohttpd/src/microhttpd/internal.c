/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Daniel Pittman and Christian Grothoff
     Copyright (C) 2015-2023 Evgeny Grin (Karlson2k)

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
 * @file microhttpd/internal.c
 * @brief  internal shared structures
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "internal.h"
#include "mhd_str.h"

#ifdef HAVE_MESSAGES
#if DEBUG_STATES
/**
 * State to string dictionary.
 */
const char *
MHD_state_to_string (enum MHD_CONNECTION_STATE state)
{
  switch (state)
  {
  case MHD_CONNECTION_INIT:
    return "connection init";
  case MHD_CONNECTION_REQ_LINE_RECEIVING:
    return "receiving request line";
  case MHD_CONNECTION_REQ_LINE_RECEIVED:
    return "request line received";
  case MHD_CONNECTION_REQ_HEADERS_RECEIVING:
    return "headers receiving";
  case MHD_CONNECTION_HEADERS_RECEIVED:
    return "headers received";
  case MHD_CONNECTION_HEADERS_PROCESSED:
    return "headers processed";
  case MHD_CONNECTION_CONTINUE_SENDING:
    return "continue sending";
  case MHD_CONNECTION_BODY_RECEIVING:
    return "body receiving";
  case MHD_CONNECTION_BODY_RECEIVED:
    return "body received";
  case MHD_CONNECTION_FOOTERS_RECEIVING:
    return "footers receiving";
  case MHD_CONNECTION_FOOTERS_RECEIVED:
    return "footers received";
  case MHD_CONNECTION_FULL_REQ_RECEIVED:
    return "full request received";
  case MHD_CONNECTION_START_REPLY:
    return "start sending reply";
  case MHD_CONNECTION_HEADERS_SENDING:
    return "headers sending";
  case MHD_CONNECTION_HEADERS_SENT:
    return "headers sent";
  case MHD_CONNECTION_NORMAL_BODY_UNREADY:
    return "normal body unready";
  case MHD_CONNECTION_NORMAL_BODY_READY:
    return "normal body ready";
  case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
    return "chunked body unready";
  case MHD_CONNECTION_CHUNKED_BODY_READY:
    return "chunked body ready";
  case MHD_CONNECTION_CHUNKED_BODY_SENT:
    return "chunked body sent";
  case MHD_CONNECTION_FOOTERS_SENDING:
    return "footers sending";
  case MHD_CONNECTION_FULL_REPLY_SENT:
    return "reply sent completely";
  case MHD_CONNECTION_CLOSED:
    return "closed";
  default:
    return "unrecognized connection state";
  }
}


#endif
#endif


#ifdef HAVE_MESSAGES
/**
 * fprintf-like helper function for logging debug
 * messages.
 */
void
MHD_DLOG (const struct MHD_Daemon *daemon,
          const char *format,
          ...)
{
  va_list va;

  if (0 == (daemon->options & MHD_USE_ERROR_LOG))
    return;
  va_start (va, format);
  daemon->custom_error_log (daemon->custom_error_log_cls,
                            format,
                            va);
  va_end (va);
}


#endif


/**
 * Convert all occurrences of '+' to ' '.
 *
 * @param arg string that is modified (in place), must be 0-terminated
 */
void
MHD_unescape_plus (char *arg)
{
  char *p;

  for (p = strchr (arg, '+'); NULL != p; p = strchr (p + 1, '+'))
    *p = ' ';
}


/**
 * Process escape sequences ('%HH') Updates val in place; the
 * result cannot be larger than the input.
 * The result is still be 0-terminated.
 *
 * @param val value to unescape (modified in the process)
 * @return length of the resulting val (`strlen(val)` may be
 *  shorter afterwards due to elimination of escape sequences)
 */
_MHD_EXTERN size_t
MHD_http_unescape (char *val)
{
  return MHD_str_pct_decode_in_place_lenient_ (val, NULL);
}


/**
 * Parse and unescape the arguments given by the client
 * as part of the HTTP request URI.
 *
 * @param kind header kind to pass to @a cb
 * @param connection connection to add headers to
 * @param[in,out] args argument URI string (after "?" in URI),
 *        clobbered in the process!
 * @param cb function to call on each key-value pair found
 * @param cls the iterator context
 * @return #MHD_NO on failure (@a cb returned #MHD_NO),
 *         #MHD_YES for success (parsing succeeded, @a cb always
 *                               returned #MHD_YES)
 */
enum MHD_Result
MHD_parse_arguments_ (struct MHD_Connection *connection,
                      enum MHD_ValueKind kind,
                      char *args,
                      MHD_ArgumentIterator_ cb,
                      void *cls)
{
  struct MHD_Daemon *daemon = connection->daemon;
  char *equals;
  char *amper;

  while ( (NULL != args) &&
          ('\0' != args[0]) )
  {
    size_t key_len;
    size_t value_len;
    equals = strchr (args, '=');
    amper = strchr (args, '&');
    if (NULL == amper)
    {
      /* last argument */
      if (NULL == equals)
      {
        /* last argument, without '=' */
        MHD_unescape_plus (args);
        key_len = daemon->unescape_callback (daemon->unescape_callback_cls,
                                             connection,
                                             args);
        if (MHD_NO == cb (cls,
                          args,
                          key_len,
                          NULL,
                          0,
                          kind))
          return MHD_NO;
        break;
      }
      /* got 'foo=bar' */
      equals[0] = '\0';
      equals++;
      MHD_unescape_plus (args);
      key_len = daemon->unescape_callback (daemon->unescape_callback_cls,
                                           connection,
                                           args);
      MHD_unescape_plus (equals);
      value_len = daemon->unescape_callback (daemon->unescape_callback_cls,
                                             connection,
                                             equals);
      if (MHD_NO == cb (cls,
                        args,
                        key_len,
                        equals,
                        value_len,
                        kind))
        return MHD_NO;
      break;
    }
    /* amper is non-NULL here */
    amper[0] = '\0';
    amper++;
    if ( (NULL == equals) ||
         (equals >= amper) )
    {
      /* got 'foo&bar' or 'foo&bar=val', add key 'foo' with NULL for value */
      MHD_unescape_plus (args);
      key_len = daemon->unescape_callback (daemon->unescape_callback_cls,
                                           connection,
                                           args);
      if (MHD_NO == cb (cls,
                        args,
                        key_len,
                        NULL,
                        0,
                        kind))
        return MHD_NO;
      /* continue with 'bar' */
      args = amper;
      continue;
    }
    /* equals and amper are non-NULL here, and equals < amper,
 so we got regular 'foo=value&bar...'-kind of argument */
    equals[0] = '\0';
    equals++;
    MHD_unescape_plus (args);
    key_len = daemon->unescape_callback (daemon->unescape_callback_cls,
                                         connection,
                                         args);
    MHD_unescape_plus (equals);
    value_len = daemon->unescape_callback (daemon->unescape_callback_cls,
                                           connection,
                                           equals);
    if (MHD_NO == cb (cls,
                      args,
                      key_len,
                      equals,
                      value_len,
                      kind))
      return MHD_NO;
    args = amper;
  }
  return MHD_YES;
}


/* end of internal.c */
