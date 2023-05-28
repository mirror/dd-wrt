/*
  This file is part of libmicrohttpd
  Copyright (C) 2021 David Gausmann

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
 * @file microhttpd_ws/mhd_websocket.c
 * @brief  Support for the websocket protocol
 * @author David Gausmann
 */
#include "platform.h"
#include "microhttpd.h"
#include "microhttpd_ws.h"
#include "sha1.h"

struct MHD_WebSocketStream
{
  /* The function pointer to malloc for payload (can be used to use different memory management) */
  MHD_WebSocketMallocCallback malloc;
  /* The function pointer to realloc for payload (can be used to use different memory management) */
  MHD_WebSocketReallocCallback realloc;
  /* The function pointer to free for payload (can be used to use different memory management) */
  MHD_WebSocketFreeCallback free;
  /* A closure for the random number generator (only used for client mode; usually not required) */
  void *cls_rng;
  /* The random number generator (only used for client mode; usually not required) */
  MHD_WebSocketRandomNumberGenerator rng;
  /* The flags specified upon initialization. It may alter the behavior of decoding/encoding */
  int flags;
  /* The current step for the decoder. 0 means start of a frame. */
  char decode_step;
  /* Specifies whether the stream is valid (1) or not (0),
     if a close frame has been received this is (-1) to indicate that no data frames are allowed anymore  */
  char validity;
  /* The current step of the UTF-8 encoding check in the data payload */
  char data_utf8_step;
  /* The current step of the UTF-8 encoding check in the control payload */
  char control_utf8_step;
  /* if != 0 means that we expect a CONTINUATION frame */
  char data_type;
  /* The start of the current frame (may differ from data_payload for CONTINUATION frames) */
  char *data_payload_start;
  /* The buffer for the data frame */
  char *data_payload;
  /* The buffer for the control frame */
  char *control_payload;
  /* Configuration for the maximum allowed buffer size for payload data */
  size_t max_payload_size;
  /* The current frame header size */
  size_t frame_header_size;
  /* The current data payload size (can be greater than payload_size for fragmented frames) */
  size_t data_payload_size;
  /* The size of the payload of the current frame (control or data) */
  size_t payload_size;
  /* The processing offset to the start of the payload of the current frame (control or data) */
  size_t payload_index;
  /* The frame header of the current frame (control or data) */
  char frame_header[32];
  /* The mask key of the current frame (control or data); this is 0 if no masking used */
  char mask_key[4];
};

#define MHD_WEBSOCKET_FLAG_MASK_SERVERCLIENT          MHD_WEBSOCKET_FLAG_CLIENT
#define MHD_WEBSOCKET_FLAG_MASK_FRAGMENTATION         \
  MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS
#define MHD_WEBSOCKET_FLAG_MASK_GENERATE_CLOSE_FRAMES \
  MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR
#define MHD_WEBSOCKET_FLAG_MASK_ALL                   \
  (MHD_WEBSOCKET_FLAG_MASK_SERVERCLIENT                                                       \
   | MHD_WEBSOCKET_FLAG_MASK_FRAGMENTATION   \
   | MHD_WEBSOCKET_FLAG_MASK_GENERATE_CLOSE_FRAMES)

enum MHD_WebSocket_Opcode
{
  MHD_WebSocket_Opcode_Continuation = 0x0,
  MHD_WebSocket_Opcode_Text         = 0x1,
  MHD_WebSocket_Opcode_Binary       = 0x2,
  MHD_WebSocket_Opcode_Close        = 0x8,
  MHD_WebSocket_Opcode_Ping         = 0x9,
  MHD_WebSocket_Opcode_Pong         = 0xA
};

enum MHD_WebSocket_DecodeStep
{
  MHD_WebSocket_DecodeStep_Start                 =  0,
  MHD_WebSocket_DecodeStep_Length1ofX            =  1,
  MHD_WebSocket_DecodeStep_Length1of2            =  2,
  MHD_WebSocket_DecodeStep_Length2of2            =  3,
  MHD_WebSocket_DecodeStep_Length1of8            =  4,
  MHD_WebSocket_DecodeStep_Length2of8            =  5,
  MHD_WebSocket_DecodeStep_Length3of8            =  6,
  MHD_WebSocket_DecodeStep_Length4of8            =  7,
  MHD_WebSocket_DecodeStep_Length5of8            =  8,
  MHD_WebSocket_DecodeStep_Length6of8            =  9,
  MHD_WebSocket_DecodeStep_Length7of8            = 10,
  MHD_WebSocket_DecodeStep_Length8of8            = 11,
  MHD_WebSocket_DecodeStep_Mask1Of4              = 12,
  MHD_WebSocket_DecodeStep_Mask2Of4              = 13,
  MHD_WebSocket_DecodeStep_Mask3Of4              = 14,
  MHD_WebSocket_DecodeStep_Mask4Of4              = 15,
  MHD_WebSocket_DecodeStep_HeaderCompleted       = 16,
  MHD_WebSocket_DecodeStep_PayloadOfDataFrame    = 17,
  MHD_WebSocket_DecodeStep_PayloadOfControlFrame = 18,
  MHD_WebSocket_DecodeStep_BrokenStream          = 99
};

enum MHD_WebSocket_UTF8Result
{
  MHD_WebSocket_UTF8Result_Invalid    = 0,
  MHD_WebSocket_UTF8Result_Valid      = 1,
  MHD_WebSocket_UTF8Result_Incomplete = 2
};

static void
MHD_websocket_copy_payload (char *dst,
                            const char *src,
                            size_t len,
                            uint32_t mask,
                            unsigned long mask_offset);

static int
MHD_websocket_check_utf8 (const char *buf,
                          size_t buf_len,
                          int *utf8_step,
                          size_t *buf_offset);

static enum MHD_WEBSOCKET_STATUS
MHD_websocket_decode_header_complete (struct MHD_WebSocketStream *ws,
                                      char **payload,
                                      size_t *payload_len);

static enum MHD_WEBSOCKET_STATUS
MHD_websocket_decode_payload_complete (struct MHD_WebSocketStream *ws,
                                       char **payload,
                                       size_t *payload_len);

static char
MHD_websocket_encode_is_masked (struct MHD_WebSocketStream *ws);
static char
MHD_websocket_encode_overhead_size (struct MHD_WebSocketStream *ws,
                                    size_t payload_len);

static enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_data (struct MHD_WebSocketStream *ws,
                           const char *payload,
                           size_t payload_len,
                           int fragmentation,
                           char **frame,
                           size_t *frame_len,
                           char opcode);

static enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_ping_pong (struct MHD_WebSocketStream *ws,
                                const char *payload,
                                size_t payload_len,
                                char **frame,
                                size_t *frame_len,
                                char opcode);

static uint32_t
MHD_websocket_generate_mask (struct MHD_WebSocketStream *ws);

static uint16_t
MHD_htons (uint16_t value);

static uint64_t
MHD_htonll (uint64_t value);


/**
 * Checks whether the HTTP version is 1.1 or above.
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_check_http_version (const char *http_version)
{
  /* validate parameters */
  if (NULL == http_version)
  {
    /* Like with the other check routines, */
    /* NULL is threated as "value not given" and not as parameter error */
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }

  /* Check whether the version has a valid format */
  /* RFC 1945 3.1: The format must be "HTTP/x.x" where x is */
  /* any digit and must appear at least once */
  if (('H' != http_version[0]) ||
      ('T' != http_version[1]) ||
      ('T' != http_version[2]) ||
      ('P' != http_version[3]) ||
      ('/' != http_version[4]))
  {
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }

  /* Find the major and minor part of the version */
  /* RFC 1945 3.1: Both numbers must be threated as separate integers. */
  /* Leading zeros must be ignored and both integers may have multiple digits */
  const char *major = NULL;
  const char *dot   = NULL;
  size_t i = 5;
  for (;;)
  {
    char c = http_version[i];
    if (('0' <= c) && ('9' >= c))
    {
      if ((NULL == major) ||
          ((http_version + i == major + 1) && ('0' == *major)) )
      {
        major = http_version + i;
      }
      ++i;
    }
    else if ('.' == http_version[i])
    {
      dot = http_version + i;
      ++i;
      break;
    }
    else
    {
      return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
    }
  }
  const char *minor = NULL;
  const char *end   = NULL;
  for (;;)
  {
    char c = http_version[i];
    if (('0' <= c) && ('9' >= c))
    {
      if ((NULL == minor) ||
          ((http_version + i == minor + 1) && ('0' == *minor)) )
      {
        minor = http_version + i;
      }
      ++i;
    }
    else if (0 == c)
    {
      end = http_version + i;
      break;
    }
    else
    {
      return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
    }
  }
  if ((NULL == major) || (NULL == dot) || (NULL == minor) || (NULL == end))
  {
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }
  if ((2 <= dot - major) || ('2' <= *major) ||
      (('1' == *major) && ((2 <= end - minor) || ('1' <= *minor))) )
  {
    return MHD_WEBSOCKET_STATUS_OK;
  }

  return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
}


/**
 * Checks whether the "Connection" request header has the 'Upgrade' token.
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_check_connection_header (const char *connection_header)
{
  /* validate parameters */
  if (NULL == connection_header)
  {
    /* To be compatible with the return value */
    /* of MHD_lookup_connection_value, */
    /* NULL is threated as "value not given" and not as parameter error */
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }

  /* Check whether the Connection includes an Upgrade token */
  /* RFC 7230 6.1: Multiple tokens may appear. */
  /* RFC 7230 3.2.6: Tokens are comma separated */
  const char *token_start = NULL;
  const char *token_end   = NULL;
  for (size_t i = 0; ; ++i)
  {
    char c = connection_header[i];

    /* RFC 7230 3.2.6: The list of allowed characters is a token is: */
    /* "!" / "#" / "$" / "%" / "&" / "'" / "*" / */
    /* "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" */
    /* DIGIT / ALPHA */
    if (('!' == c) || ('#' == c) || ('$' == c) || ('%' == c) ||
        ('&' == c) || ('\'' == c) || ('*' == c) ||
        ('+' == c) || ('-' == c) || ('.' == c) || ('^' == c) ||
        ('_' == c) || ('`' == c) || ('|' == c) || ('~' == c) ||
        (('0' <= c) && ('9' >= c)) ||
        (('A' <= c) && ('Z' >= c)) || (('a' <= c) && ('z' >= c)) )
    {
      /* This is a valid token character */
      if (NULL == token_start)
      {
        token_start = connection_header + i;
      }
      token_end = connection_header + i + 1;
    }
    else if ((' ' == c) || ('\t' == c))
    {
      /* White-spaces around tokens will be ignored */
    }
    else if ((',' == c) || (0 == c))
    {
      /* Check the token (case-insensitive) */
      if (NULL != token_start)
      {
        if (7 == (token_end - token_start) )
        {
          if ( (('U' == token_start[0]) || ('u' == token_start[0])) &&
               (('P' == token_start[1]) || ('p' == token_start[1])) &&
               (('G' == token_start[2]) || ('g' == token_start[2])) &&
               (('R' == token_start[3]) || ('r' == token_start[3])) &&
               (('A' == token_start[4]) || ('a' == token_start[4])) &&
               (('D' == token_start[5]) || ('d' == token_start[5])) &&
               (('E' == token_start[6]) || ('e' == token_start[6])) )
          {
            /* The token equals to "Upgrade" */
            return MHD_WEBSOCKET_STATUS_OK;
          }
        }
      }
      if (0 == c)
      {
        break;
      }
      token_start = NULL;
      token_end   = NULL;
    }
    else
    {
      /* RFC 7230 3.2.6: Other characters are not allowed */
      return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
    }
  }
  return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
}


/**
 * Checks whether the "Upgrade" request header has the "websocket" keyword.
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_check_upgrade_header (const char *upgrade_header)
{
  /* validate parameters */
  if (NULL == upgrade_header)
  {
    /* To be compatible with the return value */
    /* of MHD_lookup_connection_value, */
    /* NULL is threated as "value not given" and not as parameter error */
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }

  /* Check whether the Connection includes an Upgrade token */
  /* RFC 7230 6.1: Multiple tokens may appear. */
  /* RFC 7230 3.2.6: Tokens are comma separated */
  const char *keyword_start = NULL;
  const char *keyword_end   = NULL;
  for (size_t i = 0; ; ++i)
  {
    char c = upgrade_header[i];

    /* RFC 7230 3.2.6: The list of allowed characters is a token is: */
    /* "!" / "#" / "$" / "%" / "&" / "'" / "*" / */
    /* "+" / "-" / "." / "^" / "_" / "`" / "|" / "~" */
    /* DIGIT / ALPHA */
    /* We also allow "/" here as the sub-delimiter for the protocol version */
    if (('!' == c) || ('#' == c) || ('$' == c) || ('%' == c) ||
        ('&' == c) || ('\'' == c) || ('*' == c) ||
        ('+' == c) || ('-' == c) || ('.' == c) || ('^' == c) ||
        ('_' == c) || ('`' == c) || ('|' == c) || ('~' == c) ||
        ('/' == c) ||
        (('0' <= c) && ('9' >= c)) ||
        (('A' <= c) && ('Z' >= c)) || (('a' <= c) && ('z' >= c)) )
    {
      /* This is a valid token character */
      if (NULL == keyword_start)
      {
        keyword_start = upgrade_header + i;
      }
      keyword_end = upgrade_header + i + 1;
    }
    else if ((' ' == c) || ('\t' == c))
    {
      /* White-spaces around tokens will be ignored */
    }
    else if ((',' == c) || (0 == c))
    {
      /* Check the token (case-insensitive) */
      if (NULL != keyword_start)
      {
        if (9 == (keyword_end - keyword_start) )
        {
          if ( (('W' == keyword_start[0]) || ('w' == keyword_start[0])) &&
               (('E' == keyword_start[1]) || ('e' == keyword_start[1])) &&
               (('B' == keyword_start[2]) || ('b' == keyword_start[2])) &&
               (('S' == keyword_start[3]) || ('s' == keyword_start[3])) &&
               (('O' == keyword_start[4]) || ('o' == keyword_start[4])) &&
               (('C' == keyword_start[5]) || ('c' == keyword_start[5])) &&
               (('K' == keyword_start[6]) || ('k' == keyword_start[6])) &&
               (('E' == keyword_start[7]) || ('e' == keyword_start[7])) &&
               (('T' == keyword_start[8]) || ('t' == keyword_start[8])) )
          {
            /* The keyword equals to "websocket" */
            return MHD_WEBSOCKET_STATUS_OK;
          }
        }
      }
      if (0 == c)
      {
        break;
      }
      keyword_start = NULL;
      keyword_end   = NULL;
    }
    else
    {
      /* RFC 7230 3.2.6: Other characters are not allowed */
      return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
    }
  }
  return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
}


/**
 * Checks whether the "Sec-WebSocket-Version" request header
 * equals to "13"
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_check_version_header (const char *version_header)
{
  /* validate parameters */
  if (NULL == version_header)
  {
    /* To be compatible with the return value */
    /* of MHD_lookup_connection_value, */
    /* NULL is threated as "value not given" and not as parameter error */
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }

  if (('1' == version_header[0]) &&
      ('3' == version_header[1]) &&
      (0   == version_header[2]))
  {
    /* The version equals to "13" */
    return MHD_WEBSOCKET_STATUS_OK;
  }
  return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
}


/**
 * Creates the response for the Sec-WebSocket-Accept header
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_create_accept_header (const char *sec_websocket_key,
                                    char *sec_websocket_accept)
{
  /* initialize output variables for errors cases */
  if (NULL != sec_websocket_accept)
    *sec_websocket_accept = 0;

  /* validate parameters */
  if (NULL == sec_websocket_accept)
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }
  if (NULL == sec_websocket_key)
  {
    /* NULL is not a parameter error, */
    /* because MHD_lookup_connection_value returns NULL */
    /* if the header wasn't found */
    return MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER;
  }

  /* build SHA1 hash of the given key and the UUID appended */
  char sha1[20];
  const char *suffix = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  int length = (int) strlen (sec_websocket_key);
  struct sha1_ctx ctx;
  MHD_SHA1_init (&ctx);
  MHD_SHA1_update (&ctx, (const uint8_t *) sec_websocket_key, length);
  MHD_SHA1_update (&ctx, (const uint8_t *) suffix, 36);
  MHD_SHA1_finish (&ctx, (uint8_t *) sha1);

  /* base64 encode that SHA1 hash */
  /* (simple algorithm here; SHA1 has always 20 bytes, */
  /* which will always result in a 28 bytes base64 hash) */
  const char *base64_encoding_table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  for (int i = 0, j = 0; i < 20;)
  {
    uint32_t octet_a = i < 20 ? (unsigned char) sha1[i++] : 0;
    uint32_t octet_b = i < 20 ? (unsigned char) sha1[i++] : 0;
    uint32_t octet_c = i < 20 ? (unsigned char) sha1[i++] : 0;
    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    sec_websocket_accept[j++] = base64_encoding_table[(triple >> 3 * 6) & 0x3F];
    sec_websocket_accept[j++] = base64_encoding_table[(triple >> 2 * 6) & 0x3F];
    sec_websocket_accept[j++] = base64_encoding_table[(triple >> 1 * 6) & 0x3F];
    sec_websocket_accept[j++] = base64_encoding_table[(triple >> 0 * 6) & 0x3F];

  }
  sec_websocket_accept[27] = '=';
  sec_websocket_accept[28] = 0;

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Initializes a new websocket stream
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_stream_init (struct MHD_WebSocketStream **ws,
                           int flags,
                           size_t max_payload_size)
{
  return MHD_websocket_stream_init2 (ws,
                                     flags,
                                     max_payload_size,
                                     malloc,
                                     realloc,
                                     free,
                                     NULL,
                                     NULL);
}


/**
 * Initializes a new websocket stream with
 * additional parameters for allocation functions
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_stream_init2 (struct MHD_WebSocketStream **ws,
                            int flags,
                            size_t max_payload_size,
                            MHD_WebSocketMallocCallback callback_malloc,
                            MHD_WebSocketReallocCallback callback_realloc,
                            MHD_WebSocketFreeCallback callback_free,
                            void *cls_rng,
                            MHD_WebSocketRandomNumberGenerator callback_rng)
{
  /* initialize output variables for errors cases */
  if (NULL != ws)
    *ws = NULL;

  /* validate parameters */
  if ((NULL == ws) ||
      (0 != (flags & ~MHD_WEBSOCKET_FLAG_MASK_ALL)) ||
      ((uint64_t) 0x7FFFFFFFFFFFFFFF < max_payload_size) ||
      (NULL == callback_malloc) ||
      (NULL == callback_realloc) ||
      (NULL == callback_free) ||
      ((0 != (flags & MHD_WEBSOCKET_FLAG_CLIENT)) &&
       (NULL == callback_rng)))
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  /* allocate stream */
  struct MHD_WebSocketStream *ws_ = (struct MHD_WebSocketStream *) malloc (
    sizeof (struct MHD_WebSocketStream));
  if (NULL == ws_)
    return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;

  /* initialize stream */
  memset (ws_, 0, sizeof (struct MHD_WebSocketStream));
  ws_->flags   = flags;
  ws_->max_payload_size = max_payload_size;
  ws_->malloc   = callback_malloc;
  ws_->realloc  = callback_realloc;
  ws_->free     = callback_free;
  ws_->cls_rng  = cls_rng;
  ws_->rng      = callback_rng;
  ws_->validity = MHD_WEBSOCKET_VALIDITY_VALID;

  /* return stream */
  *ws = ws_;

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Frees a previously allocated websocket stream
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_stream_free (struct MHD_WebSocketStream *ws)
{
  /* validate parameters */
  if (NULL == ws)
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;

  /* free allocated payload data */
  if (ws->data_payload)
    ws->free (ws->data_payload);
  if (ws->control_payload)
    ws->free (ws->control_payload);

  /* free the stream */
  free (ws);

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Invalidates a websocket stream (no more decoding possible)
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_stream_invalidate (struct MHD_WebSocketStream *ws)
{
  /* validate parameters */
  if (NULL == ws)
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;

  /* invalidate stream */
  ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Returns whether a websocket stream is valid
 */
_MHD_EXTERN enum MHD_WEBSOCKET_VALIDITY
MHD_websocket_stream_is_valid (struct MHD_WebSocketStream *ws)
{
  /* validate parameters */
  if (NULL == ws)
    return MHD_WEBSOCKET_VALIDITY_INVALID;

  return ws->validity;
}


/**
 * Decodes incoming data to a websocket frame
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_decode (struct MHD_WebSocketStream *ws,
                      const char *streambuf,
                      size_t streambuf_len,
                      size_t *streambuf_read_len,
                      char **payload,
                      size_t *payload_len)
{
  /* initialize output variables for errors cases */
  if (NULL != streambuf_read_len)
    *streambuf_read_len = 0;
  if (NULL != payload)
    *payload = NULL;
  if (NULL != payload_len)
    *payload_len = 0;

  /* validate parameters */
  if ((NULL == ws) ||
      ((NULL == streambuf) && (0 != streambuf_len)) ||
      (NULL == streambuf_read_len) ||
      (NULL == payload) ||
      (NULL == payload_len) )
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  /* validate stream validity */
  if (MHD_WEBSOCKET_VALIDITY_INVALID == ws->validity)
    return MHD_WEBSOCKET_STATUS_STREAM_BROKEN;

  /* decode loop */
  size_t current = 0;
  while (current < streambuf_len)
  {
    switch (ws->decode_step)
    {
    /* start of frame */
    case MHD_WebSocket_DecodeStep_Start:
      {
        /* The first byte contains the opcode, the fin flag and three reserved bits */
        if (MHD_WEBSOCKET_VALIDITY_INVALID != ws->validity)
        {
          char opcode = streambuf [current];
          if (0 != (opcode & 0x70))
          {
            /* RFC 6455 5.2 RSV1-3: If a reserved flag is set */
            /* (while it isn't specified by an extension) the communication must fail. */
            ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
            if (0 != (ws->flags
                      & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
            {
              MHD_websocket_encode_close (ws,
                                          MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                          0,
                                          0,
                                          payload,
                                          payload_len);
            }
            *streambuf_read_len = current;
            return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
          }
          switch (opcode & 0x0F)
          {
          case MHD_WebSocket_Opcode_Continuation:
            if (0 == ws->data_type)
            {
              /* RFC 6455 5.4: Continuation frame without previous data frame */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
            if (MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES ==
                ws->validity)
            {
              /* RFC 6455 5.5.1: After a close frame has been sent, */
              /* no data frames may be sent (so we don't accept data frames */
              /* for decoding anymore) */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
            break;

          case MHD_WebSocket_Opcode_Text:
          case MHD_WebSocket_Opcode_Binary:
            if (0 != ws->data_type)
            {
              /* RFC 6455 5.4: Continuation expected, but new data frame */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
            if (MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES ==
                ws->validity)
            {
              /* RFC 6455 5.5.1: After a close frame has been sent, */
              /* no data frames may be sent (so we don't accept data frames */
              /* for decoding anymore) */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
            break;

          case MHD_WebSocket_Opcode_Close:
          case MHD_WebSocket_Opcode_Ping:
          case MHD_WebSocket_Opcode_Pong:
            if ((opcode & 0x80) == 0)
            {
              /* RFC 6455 5.4: Control frames may not be fragmented */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
            if (MHD_WebSocket_Opcode_Close == (opcode & 0x0F))
            {
              /* RFC 6455 5.5.1: After a close frame has been sent, */
              /* no data frames may be sent (so we don't accept data frames */
              /* for decoding anymore) */
              ws->validity =
                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES;
            }
            break;

          default:
            /* RFC 6455 5.2 OPCODE: Only six opcodes are specified. */
            /* All other are invalid in version 13 of the protocol. */
            ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
            if (0 != (ws->flags
                      & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
            {
              MHD_websocket_encode_close (ws,
                                          MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                          0,
                                          0,
                                          payload,
                                          payload_len);
            }
            *streambuf_read_len = current;
            return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
          }
        }
        ws->frame_header [ws->frame_header_size++] = streambuf [current++];
        ws->decode_step = MHD_WebSocket_DecodeStep_Length1ofX;
      }
      break;

    case MHD_WebSocket_DecodeStep_Length1ofX:
      {
        /* The second byte specifies whether the data is masked and the size */
        /* (the client MUST mask the payload, the server MUST NOT mask the payload) */
        char frame_len = streambuf [current];
        char is_masked = (frame_len & 0x80);
        frame_len &= 0x7f;
        if (MHD_WEBSOCKET_VALIDITY_INVALID != ws->validity)
        {
          if (0 != is_masked)
          {
            if (MHD_WEBSOCKET_FLAG_CLIENT == (ws->flags
                                              & MHD_WEBSOCKET_FLAG_CLIENT))
            {
              /* RFC 6455 5.1: All frames from the server must be unmasked */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
          }
          else
          {
            if (MHD_WEBSOCKET_FLAG_SERVER == (ws->flags
                                              & MHD_WEBSOCKET_FLAG_CLIENT))
            {
              /* RFC 6455 5.1: All frames from the client must be masked */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
          }
          if (126 <= frame_len)
          {
            if (0 != (ws->frame_header [0] & 0x08))
            {
              /* RFC 6455 5.5: Control frames may not have more payload than 125 bytes */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
          }
          if (1 == frame_len)
          {
            if (MHD_WebSocket_Opcode_Close == (ws->frame_header [0] & 0x0F))
            {
              /* RFC 6455 5.5.1: The close frame must have at least */
              /* two bytes of payload if payload is used */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current;
              return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
            }
          }
        }
        ws->frame_header [ws->frame_header_size++] = streambuf [current++];

        if (126 == frame_len)
        {
          ws->decode_step = MHD_WebSocket_DecodeStep_Length1of2;
        }
        else if (127 == frame_len)
        {
          ws->decode_step = MHD_WebSocket_DecodeStep_Length1of8;
        }
        else
        {
          size_t size = (size_t) frame_len;
          if ((SIZE_MAX < size) ||
              (ws->max_payload_size && (ws->max_payload_size < size)) )
          {
            /* RFC 6455 7.4.1 1009: If the message is too big to process, we may close the connection */
            ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
            if (0 != (ws->flags
                      & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
            {
              MHD_websocket_encode_close (ws,
                                          MHD_WEBSOCKET_CLOSEREASON_MAXIMUM_ALLOWED_PAYLOAD_SIZE_EXCEEDED,
                                          0,
                                          0,
                                          payload,
                                          payload_len);
            }
            *streambuf_read_len = current;
            return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;
          }
          ws->payload_size = size;
          if (0 != is_masked)
          {
            /* with mask */
            ws->decode_step = MHD_WebSocket_DecodeStep_Mask1Of4;
          }
          else
          {
            /* without mask */
            *((uint32_t *) ws->mask_key) = 0;
            ws->decode_step = MHD_WebSocket_DecodeStep_HeaderCompleted;
          }
        }
      }
      break;

    /* Payload size first byte of 2 bytes */
    case MHD_WebSocket_DecodeStep_Length1of2:
    /* Payload size first 7 bytes of 8 bytes */
    case MHD_WebSocket_DecodeStep_Length1of8:
    case MHD_WebSocket_DecodeStep_Length2of8:
    case MHD_WebSocket_DecodeStep_Length3of8:
    case MHD_WebSocket_DecodeStep_Length4of8:
    case MHD_WebSocket_DecodeStep_Length5of8:
    case MHD_WebSocket_DecodeStep_Length6of8:
    case MHD_WebSocket_DecodeStep_Length7of8:
    /* Mask first 3 bytes of 4 bytes */
    case MHD_WebSocket_DecodeStep_Mask1Of4:
    case MHD_WebSocket_DecodeStep_Mask2Of4:
    case MHD_WebSocket_DecodeStep_Mask3Of4:
      ws->frame_header [ws->frame_header_size++] = streambuf [current++];
      ++ws->decode_step;
      break;

    /* 2 byte length finished */
    case MHD_WebSocket_DecodeStep_Length2of2:
      {
        ws->frame_header [ws->frame_header_size++] = streambuf [current++];
        size_t size = (size_t) MHD_htons (
          *((uint16_t *) &ws->frame_header [2]));
        if (125 >= size)
        {
          /* RFC 6455 5.2 Payload length: The minimal number of bytes */
          /* must be used for the length */
          ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
          if (0 != (ws->flags
                    & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
          {
            MHD_websocket_encode_close (ws,
                                        MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                        0,
                                        0,
                                        payload,
                                        payload_len);
          }
          *streambuf_read_len = current;
          return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
        }
        if ((SIZE_MAX < size) ||
            (ws->max_payload_size && (ws->max_payload_size < size)) )
        {
          /* RFC 6455 7.4.1 1009: If the message is too big to process, */
          /* we may close the connection */
          ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
          if (0 != (ws->flags
                    & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
          {
            MHD_websocket_encode_close (ws,
                                        MHD_WEBSOCKET_CLOSEREASON_MAXIMUM_ALLOWED_PAYLOAD_SIZE_EXCEEDED,
                                        0,
                                        0,
                                        payload,
                                        payload_len);
          }
          *streambuf_read_len = current;
          return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;
        }
        ws->payload_size = size;
        if (0 != (ws->frame_header [1] & 0x80))
        {
          /* with mask */
          ws->decode_step = MHD_WebSocket_DecodeStep_Mask1Of4;
        }
        else
        {
          /* without mask */
          *((uint32_t *) ws->mask_key) = 0;
          ws->decode_step = MHD_WebSocket_DecodeStep_HeaderCompleted;
        }
      }
      break;

    /* 8 byte length finished */
    case MHD_WebSocket_DecodeStep_Length8of8:
      {
        ws->frame_header [ws->frame_header_size++] = streambuf [current++];
        uint64_t size = MHD_htonll (*((uint64_t *) &ws->frame_header [2]));
        if (0x7fffffffffffffff < size)
        {
          /* RFC 6455 5.2 frame-payload-length-63: The length may */
          /* not exceed 0x7fffffffffffffff */
          ws->decode_step = MHD_WebSocket_DecodeStep_BrokenStream;
          ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
          if (0 != (ws->flags
                    & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
          {
            MHD_websocket_encode_close (ws,
                                        MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                        0,
                                        0,
                                        payload,
                                        payload_len);
          }
          *streambuf_read_len = current;
          return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
        }
        if (65535 >= size)
        {
          /* RFC 6455 5.2 Payload length: The minimal number of bytes */
          /* must be used for the length */
          ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
          if (0 != (ws->flags
                    & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
          {
            MHD_websocket_encode_close (ws,
                                        MHD_WEBSOCKET_CLOSEREASON_PROTOCOL_ERROR,
                                        0,
                                        0,
                                        payload,
                                        payload_len);
          }
          *streambuf_read_len = current;
          return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
        }
        if ((SIZE_MAX < size) ||
            (ws->max_payload_size && (ws->max_payload_size < size)) )
        {
          /* RFC 6455 7.4.1 1009: If the message is too big to process, */
          /* we may close the connection */
          ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
          if (0 != (ws->flags
                    & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
          {
            MHD_websocket_encode_close (ws,
                                        MHD_WEBSOCKET_CLOSEREASON_MAXIMUM_ALLOWED_PAYLOAD_SIZE_EXCEEDED,
                                        0,
                                        0,
                                        payload,
                                        payload_len);
          }
          *streambuf_read_len = current;
          return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;
        }
        ws->payload_size = (size_t) size;

        if (0 != (ws->frame_header [1] & 0x80))
        {
          /* with mask */
          ws->decode_step = MHD_WebSocket_DecodeStep_Mask1Of4;
        }
        else
        {
          /* without mask */
          *((uint32_t *) ws->mask_key) = 0;
          ws->decode_step = MHD_WebSocket_DecodeStep_HeaderCompleted;
        }
      }
      break;

    /* mask finished */
    case MHD_WebSocket_DecodeStep_Mask4Of4:
      ws->frame_header [ws->frame_header_size++] = streambuf [current++];
      *((uint32_t *) ws->mask_key) = *((uint32_t *) &ws->frame_header [ws->
                                                                       frame_header_size
                                                                       - 4]);
      ws->decode_step = MHD_WebSocket_DecodeStep_HeaderCompleted;
      break;

    /* header finished */
    case MHD_WebSocket_DecodeStep_HeaderCompleted:
      /* return or assign either to data or control */
      {
        int ret = MHD_websocket_decode_header_complete (ws,
                                                        payload,
                                                        payload_len);
        if (MHD_WEBSOCKET_STATUS_OK != ret)
        {
          *streambuf_read_len = current;
          return ret;
        }
      }
      break;

    /* payload data */
    case MHD_WebSocket_DecodeStep_PayloadOfDataFrame:
    case MHD_WebSocket_DecodeStep_PayloadOfControlFrame:
      {
        size_t bytes_needed    = ws->payload_size - ws->payload_index;
        size_t bytes_remaining = streambuf_len - current;
        size_t bytes_to_take   = bytes_needed < bytes_remaining ? bytes_needed :
                                 bytes_remaining;
        if (0 != bytes_to_take)
        {
          size_t utf8_start     = ws->payload_index;
          char *decode_payload = ws->decode_step ==
                                 MHD_WebSocket_DecodeStep_PayloadOfDataFrame ?
                                 ws->data_payload_start :
                                 ws->control_payload;

          /* copy the new payload data (with unmasking if necessary */
          MHD_websocket_copy_payload (decode_payload + ws->payload_index,
                                      &streambuf [current],
                                      bytes_to_take,
                                      *((uint32_t *) ws->mask_key),
                                      (unsigned long) (ws->payload_index
                                                       & 0x03));
          current += bytes_to_take;
          ws->payload_index += bytes_to_take;
          if (((MHD_WebSocket_DecodeStep_PayloadOfDataFrame ==
                ws->decode_step) &&
               (MHD_WebSocket_Opcode_Text == ws->data_type)) ||
              ((MHD_WebSocket_DecodeStep_PayloadOfControlFrame ==
                ws->decode_step) &&
               (MHD_WebSocket_Opcode_Close == (ws->frame_header [0] & 0x0f)) &&
               (2 < ws->payload_index)) )
          {
            /* RFC 6455 8.1: We need to check the UTF-8 validity */
            int utf8_step;
            char *decode_payload_utf8;
            size_t bytes_to_check;
            size_t utf8_error_offset = 0;
            if (MHD_WebSocket_DecodeStep_PayloadOfDataFrame == ws->decode_step)
            {
              utf8_step           = ws->data_utf8_step;
              decode_payload_utf8 = decode_payload + utf8_start;
              bytes_to_check      = bytes_to_take;
            }
            else
            {
              utf8_step = ws->control_utf8_step;
              if ((MHD_WebSocket_Opcode_Close == (ws->frame_header [0]
                                                  & 0x0f)) &&
                  (2 > utf8_start) )
              {
                /* The first two bytes of the close frame are binary content and */
                /* must be skipped in the UTF-8 check */
                utf8_start = 2;
                utf8_error_offset = 2;
              }
              decode_payload_utf8 = decode_payload + utf8_start;
              bytes_to_check      = bytes_to_take - utf8_start;
            }
            size_t utf8_check_offset = 0;
            int utf8_result = MHD_websocket_check_utf8 (decode_payload_utf8,
                                                        bytes_to_check,
                                                        &utf8_step,
                                                        &utf8_check_offset);
            if (MHD_WebSocket_UTF8Result_Invalid != utf8_result)
            {
              /* memorize current validity check step to continue later */
              ws->data_utf8_step = utf8_step;
            }
            else
            {
              /* RFC 6455 8.1: We must fail on broken UTF-8 sequence */
              ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
              if (0 != (ws->flags
                        & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
              {
                MHD_websocket_encode_close (ws,
                                            MHD_WEBSOCKET_CLOSEREASON_MALFORMED_UTF8,
                                            0,
                                            0,
                                            payload,
                                            payload_len);
              }
              *streambuf_read_len = current - bytes_to_take
                                    + utf8_check_offset + utf8_error_offset;
              return MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR;
            }
          }
        }
      }

      if (ws->payload_size == ws->payload_index)
      {
        /* all payload data of the current frame has been received */
        int ret = MHD_websocket_decode_payload_complete (ws,
                                                         payload,
                                                         payload_len);
        if (MHD_WEBSOCKET_STATUS_OK != ret)
        {
          *streambuf_read_len = current;
          return ret;
        }
      }
      break;

    case MHD_WebSocket_DecodeStep_BrokenStream:
      *streambuf_read_len = current;
      return MHD_WEBSOCKET_STATUS_STREAM_BROKEN;
    }
  }

  /* Special treatment for zero payload length messages */
  if (MHD_WebSocket_DecodeStep_HeaderCompleted == ws->decode_step)
  {
    int ret = MHD_websocket_decode_header_complete (ws,
                                                    payload,
                                                    payload_len);
    if (MHD_WEBSOCKET_STATUS_OK != ret)
    {
      *streambuf_read_len = current;
      return ret;
    }
  }
  switch (ws->decode_step)
  {
  case MHD_WebSocket_DecodeStep_PayloadOfDataFrame:
  case MHD_WebSocket_DecodeStep_PayloadOfControlFrame:
    if (ws->payload_size == ws->payload_index)
    {
      /* all payload data of the current frame has been received */
      int ret = MHD_websocket_decode_payload_complete (ws,
                                                       payload,
                                                       payload_len);
      if (MHD_WEBSOCKET_STATUS_OK != ret)
      {
        *streambuf_read_len = current;
        return ret;
      }
    }
    break;
  }
  *streambuf_read_len = current;

  /* more data needed */
  return MHD_WEBSOCKET_STATUS_OK;
}


static enum MHD_WEBSOCKET_STATUS
MHD_websocket_decode_header_complete (struct MHD_WebSocketStream *ws,
                                      char **payload,
                                      size_t *payload_len)
{
  /* assign either to data or control */
  char opcode = ws->frame_header [0] & 0x0f;
  switch (opcode)
  {
  case MHD_WebSocket_Opcode_Continuation:
    {
      /* validate payload size */
      size_t new_size_total = ws->payload_size + ws->data_payload_size;
      if ((0 != ws->max_payload_size) && (ws->max_payload_size <
                                          new_size_total) )
      {
        /* RFC 6455 7.4.1 1009: If the message is too big to process, */
        /* we may close the connection */
        ws->decode_step = MHD_WebSocket_DecodeStep_BrokenStream;
        ws->validity = MHD_WEBSOCKET_VALIDITY_INVALID;
        if (0 != (ws->flags
                  & MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR))
        {
          MHD_websocket_encode_close (ws,
                                      MHD_WEBSOCKET_CLOSEREASON_MAXIMUM_ALLOWED_PAYLOAD_SIZE_EXCEEDED,
                                      0,
                                      0,
                                      payload,
                                      payload_len);
        }
        return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;
      }
      /* allocate buffer for continued data frame */
      char *new_buf = NULL;
      if (0 != new_size_total)
      {
        new_buf = ws->realloc (ws->data_payload, new_size_total + 1);
        if (NULL == new_buf)
        {
          return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
        }
        new_buf [new_size_total] = 0;
        ws->data_payload_start = &new_buf[ws->data_payload_size];
      }
      else
      {
        ws->data_payload_start = new_buf;
      }
      ws->data_payload       = new_buf;
      ws->data_payload_size  = new_size_total;
    }
    ws->decode_step = MHD_WebSocket_DecodeStep_PayloadOfDataFrame;
    break;

  case MHD_WebSocket_Opcode_Text:
  case MHD_WebSocket_Opcode_Binary:
    /* allocate buffer for data frame */
    {
      size_t new_size_total = ws->payload_size;
      char *new_buf = NULL;
      if (0 != new_size_total)
      {
        new_buf = ws->malloc (new_size_total + 1);
        if (NULL == new_buf)
        {
          return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
        }
        new_buf [new_size_total] = 0;
      }
      ws->data_payload        = new_buf;
      ws->data_payload_start  = new_buf;
      ws->data_payload_size   = new_size_total;
      ws->data_type           = opcode;
    }
    ws->decode_step = MHD_WebSocket_DecodeStep_PayloadOfDataFrame;
    break;

  case MHD_WebSocket_Opcode_Close:
  case MHD_WebSocket_Opcode_Ping:
  case MHD_WebSocket_Opcode_Pong:
    /* allocate buffer for control frame */
    {
      size_t new_size_total = ws->payload_size;
      char *new_buf = NULL;
      if (0 != new_size_total)
      {
        new_buf = ws->malloc (new_size_total + 1);
        if (NULL == new_buf)
        {
          return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
        }
        new_buf[new_size_total] = 0;
      }
      ws->control_payload = new_buf;
    }
    ws->decode_step = MHD_WebSocket_DecodeStep_PayloadOfControlFrame;
    break;
  }

  return MHD_WEBSOCKET_STATUS_OK;
}


static enum MHD_WEBSOCKET_STATUS
MHD_websocket_decode_payload_complete (struct MHD_WebSocketStream *ws,
                                       char **payload,
                                       size_t *payload_len)
{
  /* all payload data of the current frame has been received */
  char is_continue = MHD_WebSocket_Opcode_Continuation ==
                     (ws->frame_header [0] & 0x0F);
  char is_fin      = ws->frame_header [0] & 0x80;
  if (0 != is_fin)
  {
    /* the frame is complete */
    if (MHD_WebSocket_DecodeStep_PayloadOfDataFrame == ws->decode_step)
    {
      /* data frame */
      char data_type = ws->data_type;
      if ((0 != (ws->flags & MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS)) &&
          (0 != is_continue))
      {
        data_type |= 0x40;   /* mark as last fragment */
      }
      *payload     = ws->data_payload;
      *payload_len = ws->data_payload_size;
      ws->data_payload       = 0;
      ws->data_payload_start = 0;
      ws->data_payload_size  = 0;
      ws->decode_step        = MHD_WebSocket_DecodeStep_Start;
      ws->payload_index      = 0;
      ws->data_type          = 0;
      ws->frame_header_size  = 0;
      return data_type;
    }
    else
    {
      /* control frame */
      *payload     = ws->control_payload;
      *payload_len = ws->payload_size;
      ws->control_payload   = 0;
      ws->decode_step       = MHD_WebSocket_DecodeStep_Start;
      ws->payload_index     = 0;
      ws->frame_header_size = 0;
      return (ws->frame_header [0] & 0x0f);
    }
  }
  else if (0 != (ws->flags & MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS))
  {
    /* RFC 6455 5.4: To allow streaming, the user can choose */
    /* to return fragments */
    if ((MHD_WebSocket_Opcode_Text == ws->data_type) &&
        (MHD_WEBSOCKET_UTF8STEP_NORMAL != ws->data_utf8_step) )
    {
      /* the last UTF-8 sequence is incomplete, so we keep the start of
      that and only return the part before */
      size_t given_utf8 = 0;
      switch (ws->data_utf8_step)
      {
      /* one byte given */
      case MHD_WEBSOCKET_UTF8STEP_UTF2TAIL_1OF1:
      case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL1_1OF2:
      case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL2_1OF2:
      case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_1OF2:
      case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL1_1OF3:
      case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL2_1OF3:
      case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_1OF3:
        given_utf8 = 1;
        break;
      /* two bytes given */
      case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_2OF2:
      case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_2OF3:
        given_utf8 = 2;
        break;
      /* three bytes given */
      case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_3OF3:
        given_utf8 = 3;
        break;
      }
      size_t new_len = ws->data_payload_size - given_utf8;
      if (0 != new_len)
      {
        char *next_payload = ws->malloc (given_utf8 + 1);
        if (NULL == next_payload)
        {
          return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
        }
        memcpy (next_payload,
                ws->data_payload_start + ws->payload_index - given_utf8,
                given_utf8);
        next_payload[given_utf8] = 0;

        ws->data_payload[new_len] = 0;
        *payload     = ws->data_payload;
        *payload_len = new_len;
        ws->data_payload      = next_payload;
        ws->data_payload_size = given_utf8;
      }
      else
      {
        *payload     = NULL;
        *payload_len = 0;
      }
      ws->decode_step       = MHD_WebSocket_DecodeStep_Start;
      ws->payload_index     = 0;
      ws->frame_header_size = 0;
      if (0 != is_continue)
        return ws->data_type | 0x20;    /* mark as middle fragment */
      else
        return ws->data_type | 0x10;    /* mark as first fragment */
    }
    else
    {
      /* we simply pass the entire data frame */
      *payload     = ws->data_payload;
      *payload_len = ws->data_payload_size;
      ws->data_payload       = 0;
      ws->data_payload_start = 0;
      ws->data_payload_size  = 0;
      ws->decode_step        = MHD_WebSocket_DecodeStep_Start;
      ws->payload_index      = 0;
      ws->frame_header_size  = 0;
      if (0 != is_continue)
        return ws->data_type | 0x20;    /* mark as middle fragment */
      else
        return ws->data_type | 0x10;    /* mark as first fragment */
    }
  }
  else
  {
    /* RFC 6455 5.4: We must await a continuation frame to get */
    /* the remainder of this data frame */
    ws->decode_step       = MHD_WebSocket_DecodeStep_Start;
    ws->frame_header_size = 0;
    ws->payload_index     = 0;
    return MHD_WEBSOCKET_STATUS_OK;
  }
}


/**
 * Splits the received close reason
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_split_close_reason (const char *payload,
                                  size_t payload_len,
                                  unsigned short *reason_code,
                                  const char **reason_utf8,
                                  size_t *reason_utf8_len)
{
  /* initialize output variables for errors cases */
  if (NULL != reason_code)
    *reason_code = MHD_WEBSOCKET_CLOSEREASON_NO_REASON;
  if (NULL != reason_utf8)
    *reason_utf8 = NULL;
  if (NULL != reason_utf8_len)
    *reason_utf8_len = 0;

  /* validate parameters */
  if ((NULL == payload) && (0 != payload_len))
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  if (1 == payload_len)
    return MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR;
  if (125 < payload_len)
    return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;

  /* decode reason code */
  if (2 > payload_len)
  {
    if (NULL != reason_code)
      *reason_code = MHD_WEBSOCKET_CLOSEREASON_NO_REASON;
  }
  else
  {
    if (NULL != reason_code)
      *reason_code = MHD_htons (*((uint16_t *) payload));
  }

  /* decode reason text */
  if (2 >= payload_len)
  {
    if (NULL != reason_utf8)
      *reason_utf8 = NULL;
    if (NULL != reason_utf8_len)
      *reason_utf8_len = 0;
  }
  else
  {
    if (NULL != reason_utf8)
      *reason_utf8 = payload + 2;
    if (NULL != reason_utf8_len)
      *reason_utf8_len = payload_len - 2;
  }

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Encodes a text into a websocket text frame
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_text (struct MHD_WebSocketStream *ws,
                           const char *payload_utf8,
                           size_t payload_utf8_len,
                           int fragmentation,
                           char **frame,
                           size_t *frame_len,
                           int *utf8_step)
{
  /* initialize output variables for errors cases */
  if (NULL != frame)
    *frame = NULL;
  if (NULL != frame_len)
    *frame_len = 0;
  if ((NULL != utf8_step) &&
      ((MHD_WEBSOCKET_FRAGMENTATION_FIRST == fragmentation) ||
       (MHD_WEBSOCKET_FRAGMENTATION_NONE == fragmentation) ))
  {
    /* the old UTF-8 step will be ignored for new fragments */
    *utf8_step = MHD_WEBSOCKET_UTF8STEP_NORMAL;
  }

  /* validate parameters */
  if ((NULL == ws) ||
      ((0 != payload_utf8_len) && (NULL == payload_utf8)) ||
      (NULL == frame) ||
      (NULL == frame_len) ||
      (MHD_WEBSOCKET_FRAGMENTATION_NONE > fragmentation) ||
      (MHD_WEBSOCKET_FRAGMENTATION_LAST < fragmentation) ||
      ((MHD_WEBSOCKET_FRAGMENTATION_NONE != fragmentation) &&
       (NULL == utf8_step)) )
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  /* check max length */
  if ((uint64_t) 0x7FFFFFFFFFFFFFFF < (uint64_t) payload_utf8_len)
  {
    return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;
  }

  /* check UTF-8 */
  int utf8_result = MHD_websocket_check_utf8 (payload_utf8,
                                              payload_utf8_len,
                                              utf8_step,
                                              NULL);
  if ((MHD_WebSocket_UTF8Result_Invalid == utf8_result) ||
      ((MHD_WebSocket_UTF8Result_Incomplete == utf8_result) &&
       (MHD_WEBSOCKET_FRAGMENTATION_NONE == fragmentation)) )
  {
    return MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR;
  }

  /* encode data */
  return MHD_websocket_encode_data (ws,
                                    payload_utf8,
                                    payload_utf8_len,
                                    fragmentation,
                                    frame,
                                    frame_len,
                                    MHD_WebSocket_Opcode_Text);
}


/**
 * Encodes binary data into a websocket binary frame
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_binary (struct MHD_WebSocketStream *ws,
                             const char *payload,
                             size_t payload_len,
                             int fragmentation,
                             char **frame,
                             size_t *frame_len)
{
  /* initialize output variables for errors cases */
  if (NULL != frame)
    *frame = NULL;
  if (NULL != frame_len)
    *frame_len = 0;

  /* validate parameters */
  if ((NULL == ws) ||
      ((0 != payload_len) && (NULL == payload)) ||
      (NULL == frame) ||
      (NULL == frame_len) ||
      (MHD_WEBSOCKET_FRAGMENTATION_NONE > fragmentation) ||
      (MHD_WEBSOCKET_FRAGMENTATION_LAST < fragmentation) )
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  /* check max length */
  if ((uint64_t) 0x7FFFFFFFFFFFFFFF < (uint64_t) payload_len)
  {
    return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;
  }

  return MHD_websocket_encode_data (ws,
                                    payload,
                                    payload_len,
                                    fragmentation,
                                    frame,
                                    frame_len,
                                    MHD_WebSocket_Opcode_Binary);
}


/**
 * Internal function for encoding text/binary data into a websocket frame
 */
static enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_data (struct MHD_WebSocketStream *ws,
                           const char *payload,
                           size_t payload_len,
                           int fragmentation,
                           char **frame,
                           size_t *frame_len,
                           char opcode)
{
  /* calculate length and masking */
  char is_masked      = MHD_websocket_encode_is_masked (ws);
  size_t overhead_len = MHD_websocket_encode_overhead_size (ws, payload_len);
  size_t total_len    = overhead_len + payload_len;
  uint32_t mask       = 0 != is_masked ? MHD_websocket_generate_mask (ws) : 0;

  /* allocate memory */
  char *result = ws->malloc (total_len + 1);
  if (NULL == result)
    return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
  result [total_len] = 0;
  *frame     = result;
  *frame_len = total_len;

  /* add the opcode */
  switch (fragmentation)
  {
  case MHD_WEBSOCKET_FRAGMENTATION_NONE:
    *(result++) = 0x80 | opcode;
    break;
  case MHD_WEBSOCKET_FRAGMENTATION_FIRST:
    *(result++) = opcode;
    break;
  case MHD_WEBSOCKET_FRAGMENTATION_FOLLOWING:
    *(result++) = MHD_WebSocket_Opcode_Continuation;
    break;
  case MHD_WEBSOCKET_FRAGMENTATION_LAST:
    *(result++) = 0x80 | MHD_WebSocket_Opcode_Continuation;
    break;
  }

  /* add the length */
  if (126 > payload_len)
  {
    *(result++) = is_masked | (char) payload_len;
  }
  else if (65536 > payload_len)
  {
    *(result++) = is_masked | 126;
    *((uint16_t *) result) = MHD_htons ((uint16_t) payload_len);
    result += 2;
  }
  else
  {
    *(result++) = is_masked | 127;
    *((uint64_t *) result) = MHD_htonll ((uint64_t) payload_len);
    result += 8;

  }

  /* add the mask */
  if (0 != is_masked)
  {
    *(result++) = ((char *) &mask)[0];
    *(result++) = ((char *) &mask)[1];
    *(result++) = ((char *) &mask)[2];
    *(result++) = ((char *) &mask)[3];
  }

  /* add the payload */
  if (0 != payload_len)
  {
    MHD_websocket_copy_payload (result,
                                payload,
                                payload_len,
                                mask,
                                0);
  }

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Encodes a websocket ping frame
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_ping (struct MHD_WebSocketStream *ws,
                           const char *payload,
                           size_t payload_len,
                           char **frame,
                           size_t *frame_len)
{
  /* encode the ping frame */
  return MHD_websocket_encode_ping_pong (ws,
                                         payload,
                                         payload_len,
                                         frame,
                                         frame_len,
                                         MHD_WebSocket_Opcode_Ping);
}


/**
 * Encodes a websocket pong frame
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_pong (struct MHD_WebSocketStream *ws,
                           const char *payload,
                           size_t payload_len,
                           char **frame,
                           size_t *frame_len)
{
  /* encode the pong frame */
  return MHD_websocket_encode_ping_pong (ws,
                                         payload,
                                         payload_len,
                                         frame,
                                         frame_len,
                                         MHD_WebSocket_Opcode_Pong);
}


/**
 * Internal function for encoding ping/pong frames
 */
static enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_ping_pong (struct MHD_WebSocketStream *ws,
                                const char *payload,
                                size_t payload_len,
                                char **frame,
                                size_t *frame_len,
                                char opcode)
{
  /* initialize output variables for errors cases */
  if (NULL != frame)
    *frame = NULL;
  if (NULL != frame_len)
    *frame_len = 0;

  /* validate the parameters */
  if ((NULL == ws) ||
      ((0 != payload_len) && (NULL == payload)) ||
      (NULL == frame) ||
      (NULL == frame_len) )
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  /* RFC 6455 5.5: Control frames may only have up to 125 bytes of payload data */
  if (125 < payload_len)
    return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;

  /* calculate length and masking */
  char is_masked      = MHD_websocket_encode_is_masked (ws);
  size_t overhead_len = MHD_websocket_encode_overhead_size (ws, payload_len);
  size_t total_len    = overhead_len + payload_len;
  uint32_t mask       = is_masked != 0 ? MHD_websocket_generate_mask (ws) : 0;

  /* allocate memory */
  char *result = ws->malloc (total_len + 1);
  if (NULL == result)
    return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
  result [total_len] = 0;
  *frame     = result;
  *frame_len = total_len;

  /* add the opcode */
  *(result++) = 0x80 | opcode;

  /* add the length */
  *(result++) = is_masked | (char) payload_len;

  /* add the mask */
  if (0 != is_masked)
  {
    *(result++) = ((char *) &mask)[0];
    *(result++) = ((char *) &mask)[1];
    *(result++) = ((char *) &mask)[2];
    *(result++) = ((char *) &mask)[3];
  }

  /* add the payload */
  if (0 != payload_len)
  {
    MHD_websocket_copy_payload (result,
                                payload,
                                payload_len,
                                mask,
                                0);
  }

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Encodes a websocket close frame
 */
_MHD_EXTERN enum MHD_WEBSOCKET_STATUS
MHD_websocket_encode_close (struct MHD_WebSocketStream *ws,
                            unsigned short reason_code,
                            const char *reason_utf8,
                            size_t reason_utf8_len,
                            char **frame,
                            size_t *frame_len)
{
  /* initialize output variables for errors cases */
  if (NULL != frame)
    *frame = NULL;
  if (NULL != frame_len)
    *frame_len = 0;

  /* validate the parameters */
  if ((NULL == ws) ||
      ((0 != reason_utf8_len) && (NULL == reason_utf8)) ||
      (NULL == frame) ||
      (NULL == frame_len) ||
      ((MHD_WEBSOCKET_CLOSEREASON_NO_REASON != reason_code) &&
       (1000 > reason_code)) ||
      ((0 != reason_utf8_len) &&
       (MHD_WEBSOCKET_CLOSEREASON_NO_REASON == reason_code)) )
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  /* RFC 6455 5.5: Control frames may only have up to 125 bytes of payload data, */
  /* but in this case only 123 bytes, because 2 bytes are reserved */
  /* for the close reason code. */
  if (123 < reason_utf8_len)
    return MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED;

  /* RFC 6455 5.5.1: If close payload data is given, it must be valid UTF-8 */
  if (0 != reason_utf8_len)
  {
    int utf8_result = MHD_websocket_check_utf8 (reason_utf8,
                                                reason_utf8_len,
                                                NULL,
                                                NULL);
    if (MHD_WebSocket_UTF8Result_Valid != utf8_result)
      return MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR;
  }

  /* calculate length and masking */
  char is_masked      = MHD_websocket_encode_is_masked (ws);
  size_t payload_len  = (MHD_WEBSOCKET_CLOSEREASON_NO_REASON != reason_code ?
                         2 + reason_utf8_len : 0);
  size_t overhead_len = MHD_websocket_encode_overhead_size (ws, payload_len);
  size_t total_len    = overhead_len + payload_len;
  uint32_t mask       = is_masked != 0 ? MHD_websocket_generate_mask (ws) : 0;

  /* allocate memory */
  char *result = ws->malloc (total_len + 1);
  if (NULL == result)
    return MHD_WEBSOCKET_STATUS_MEMORY_ERROR;
  result [total_len] = 0;
  *frame     = result;
  *frame_len = total_len;

  /* add the opcode */
  *(result++) = 0x88;

  /* add the length */
  *(result++) = is_masked | (char) payload_len;

  /* add the mask */
  if (0 != is_masked)
  {
    *(result++) = ((char *) &mask)[0];
    *(result++) = ((char *) &mask)[1];
    *(result++) = ((char *) &mask)[2];
    *(result++) = ((char *) &mask)[3];
  }

  /* add the payload */
  if (0 != reason_code)
  {
    /* close reason code */
    uint16_t reason_code_nb = MHD_htons (reason_code);
    MHD_websocket_copy_payload (result,
                                (const char *) &reason_code_nb,
                                2,
                                mask,
                                0);
    result += 2;

    /* custom reason payload */
    if (0 != reason_utf8_len)
    {
      MHD_websocket_copy_payload (result,
                                  reason_utf8,
                                  reason_utf8_len,
                                  mask,
                                  2);
    }
  }

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Returns the 0x80 prefix for masked data, 0x00 otherwise
 */
static char
MHD_websocket_encode_is_masked (struct MHD_WebSocketStream *ws)
{
  return (ws->flags & MHD_WEBSOCKET_FLAG_MASK_SERVERCLIENT) ==
         MHD_WEBSOCKET_FLAG_CLIENT ? 0x80 : 0x00;
}


/**
 * Calculates the size of the overhead in bytes
 */
static char
MHD_websocket_encode_overhead_size (struct MHD_WebSocketStream *ws,
                                    size_t payload_len)
{
  return 2 + (MHD_websocket_encode_is_masked (ws) != 0 ? 4 : 0) + (125 <
                                                                   payload_len ?
                                                                   (65535 <
                                                                    payload_len
  ? 8 : 2) : 0);
}


/**
 * Copies the payload to the destination (using mask)
 */
static void
MHD_websocket_copy_payload (char *dst,
                            const char *src,
                            size_t len,
                            uint32_t mask,
                            unsigned long mask_offset)
{
  if (0 != len)
  {
    if (0 == mask)
    {
      /* when the mask is zero, we can just copy the data */
      memcpy (dst, src, len);
    }
    else
    {
      /* mask is used */
      char mask_[4];
      *((uint32_t *) mask_) = mask;
      for (size_t i = 0; i < len; ++i)
      {
        dst[i] = src[i] ^ mask_[(i + mask_offset) & 3];
      }
    }
  }
}


/**
 * Checks a UTF-8 sequence
 */
static int
MHD_websocket_check_utf8 (const char *buf,
                          size_t buf_len,
                          int *utf8_step,
                          size_t *buf_offset)
{
  int utf8_step_ = (NULL != utf8_step) ? *utf8_step :
                   MHD_WEBSOCKET_UTF8STEP_NORMAL;

  for (size_t i = 0; i < buf_len; ++i)
  {
    unsigned char character = (unsigned char) buf[i];
    switch (utf8_step_)
    {
    case MHD_WEBSOCKET_UTF8STEP_NORMAL:
      if ((0x00 <= character) && (0x7F >= character))
      {
        /* RFC 3629 4: single byte UTF-8 sequence */
        /* (nothing to do here) */
      }
      else if ((0xC2 <= character) && (0xDF >= character))
      {
        /* RFC 3629 4: two byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF2TAIL_1OF1;
      }
      else if (0xE0 == character)
      {
        /* RFC 3629 4: three byte UTF-8 sequence, but the second byte must be 0xA0-0xBF */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL1_1OF2;
      }
      else if (0xED == character)
      {
        /* RFC 3629 4: three byte UTF-8 sequence, but the second byte must be 0x80-0x9F */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL2_1OF2;
      }
      else if (((0xE1 <= character) && (0xEC >= character)) ||
               ((0xEE <= character) && (0xEF >= character)) )
      {
        /* RFC 3629 4: three byte UTF-8 sequence, both tail bytes must be 0x80-0xBF */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_1OF2;
      }
      else if (0xF0 == character)
      {
        /* RFC 3629 4: four byte UTF-8 sequence, but the second byte must be 0x90-0xBF */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL1_1OF3;
      }
      else if (0xF4 == character)
      {
        /* RFC 3629 4: four byte UTF-8 sequence, but the second byte must be 0x80-0x8F */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL2_1OF3;
      }
      else if ((0xF1 <= character) && (0xF3 >= character))
      {
        /* RFC 3629 4: four byte UTF-8 sequence, all three tail bytes must be 0x80-0xBF */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_1OF3;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL1_1OF2:
      if ((0xA0 <= character) && (0xBF >= character))
      {
        /* RFC 3629 4: Second byte of three byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_2OF2;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL2_1OF2:
      if ((0x80 <= character) && (0x9F >= character))
      {
        /* RFC 3629 4: Second byte of three byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_2OF2;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_1OF2:
      if ((0x80 <= character) && (0xBF >= character))
      {
        /* RFC 3629 4: Second byte of three byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_2OF2;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL1_1OF3:
      if ((0x90 <= character) && (0xBF >= character))
      {
        /* RFC 3629 4: Second byte of four byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_2OF3;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL2_1OF3:
      if ((0x80 <= character) && (0x8F >= character))
      {
        /* RFC 3629 4: Second byte of four byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_2OF3;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_1OF3:
      if ((0x80 <= character) && (0xBF >= character))
      {
        /* RFC 3629 4: Second byte of four byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_2OF3;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_2OF3:
      if ((0x80 <= character) && (0xBF >= character))
      {
        /* RFC 3629 4: Third byte of four byte UTF-8 sequence */
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_3OF3;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    /* RFC 3629 4: Second byte of two byte UTF-8 sequence */
    case MHD_WEBSOCKET_UTF8STEP_UTF2TAIL_1OF1:
    /* RFC 3629 4: Third byte of three byte UTF-8 sequence */
    case MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_2OF2:
    /* RFC 3629 4: Fourth byte of four byte UTF-8 sequence */
    case MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_3OF3:
      if ((0x80 <= character) && (0xBF >= character))
      {
        utf8_step_ = MHD_WEBSOCKET_UTF8STEP_NORMAL;
      }
      else
      {
        /* RFC 3629 4: Invalid UTF-8 byte */
        if (NULL != buf_offset)
          *buf_offset = i;
        return MHD_WebSocket_UTF8Result_Invalid;
      }
      break;

    default:
      /* Invalid last step...? */
      if (NULL != buf_offset)
        *buf_offset = i;
      return MHD_WebSocket_UTF8Result_Invalid;
    }
  }

  /* return values */
  if (NULL != utf8_step)
    *utf8_step = utf8_step_;
  if (NULL != buf_offset)
    *buf_offset = buf_len;
  if (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step_)
  {
    return MHD_WebSocket_UTF8Result_Incomplete;
  }
  return MHD_WebSocket_UTF8Result_Valid;
}


/**
 * Generates a mask for masking by calling
 * a random number generator.
 */
static uint32_t
MHD_websocket_generate_mask (struct MHD_WebSocketStream *ws)
{
  unsigned char mask_[4];
  if (NULL != ws->rng)
  {
    size_t offset = 0;
    while (offset < 4)
    {
      size_t encoded = ws->rng (ws->cls_rng,
                                mask_ + offset,
                                4 - offset);
      offset += encoded;
    }
  }
  else
  {
    /* this case should never happen */
    mask_ [0] = 0;
    mask_ [1] = 0;
    mask_ [2] = 0;
    mask_ [3] = 0;
  }

  return *((uint32_t *) mask_);
}


/**
 * Calls the malloc function associated with the websocket steam
 */
_MHD_EXTERN void *
MHD_websocket_malloc (struct MHD_WebSocketStream *ws,
                      size_t buf_len)
{
  if (NULL == ws)
  {
    return NULL;
  }

  return ws->malloc (buf_len);
}


/**
 * Calls the realloc function associated with the websocket steam
 */
_MHD_EXTERN void *
MHD_websocket_realloc (struct MHD_WebSocketStream *ws,
                       void *buf,
                       size_t new_buf_len)
{
  if (NULL == ws)
  {
    return NULL;
  }

  return ws->realloc (buf, new_buf_len);
}


/**
 * Calls the free function associated with the websocket steam
 */
_MHD_EXTERN int
MHD_websocket_free (struct MHD_WebSocketStream *ws,
                    void *buf)
{
  if (NULL == ws)
  {
    return MHD_WEBSOCKET_STATUS_PARAMETER_ERROR;
  }

  ws->free (buf);

  return MHD_WEBSOCKET_STATUS_OK;
}


/**
 * Converts a 16 bit value into network byte order (MSB first)
 * in dependence of the host system
 */
static uint16_t
MHD_htons (uint16_t value)
{
  uint16_t endian = 0x0001;

  if (((char *) &endian)[0] == 0x01)
  {
    /* least significant byte first */
    ((char *) &endian)[0] = ((char *) &value)[1];
    ((char *) &endian)[1] = ((char *) &value)[0];
    return endian;
  }
  else
  {
    /* most significant byte first */
    return value;
  }
}


/**
 * Converts a 64 bit value into network byte order (MSB first)
 * in dependence of the host system
 */
static uint64_t
MHD_htonll (uint64_t value)
{
  uint64_t endian = 0x0000000000000001;

  if (((char *) &endian)[0] == 0x01)
  {
    /* least significant byte first */
    ((char *) &endian)[0] = ((char *) &value)[7];
    ((char *) &endian)[1] = ((char *) &value)[6];
    ((char *) &endian)[2] = ((char *) &value)[5];
    ((char *) &endian)[3] = ((char *) &value)[4];
    ((char *) &endian)[4] = ((char *) &value)[3];
    ((char *) &endian)[5] = ((char *) &value)[2];
    ((char *) &endian)[6] = ((char *) &value)[1];
    ((char *) &endian)[7] = ((char *) &value)[0];
    return endian;
  }
  else
  {
    /* most significant byte first */
    return value;
  }
}
