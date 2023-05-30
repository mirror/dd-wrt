/*
     This file is part of libmicrohttpd
     Copyright (C) 2020 Christian Grothoff, Silvio Clecio (and other
     contributing authors)

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
 * @file websocket_threaded_example.c
 * @brief example for how to provide a tiny threaded websocket server
 * @author Silvio Clecio (silvioprog)
 */

/* TODO: allow to send large messages. */

#include "platform.h"
#include <pthread.h>
#include <microhttpd.h>

#define CHAT_PAGE                                                             \
  "<html>\n"                                                                  \
  "<head>\n"                                                                  \
  "<title>WebSocket chat</title>\n"                                           \
  "<script>\n"                                                                \
  "document.addEventListener('DOMContentLoaded', function() {\n"              \
  "  const ws = new WebSocket('ws:/" "/ ' + window.location.host);\n"         \
  "  const btn = document.getElementById('send');\n"                          \
  "  const msg = document.getElementById('msg');\n"                           \
  "  const log = document.getElementById('log');\n"                           \
  "  ws.onopen = function() {\n"                                              \
  "    log.value += 'Connected\\n';\n"                                        \
  "  };\n"                                                                    \
  "  ws.onclose = function() {\n"                                             \
  "    log.value += 'Disconnected\\n';\n"                                     \
  "  };\n"                                                                    \
  "  ws.onmessage = function(ev) {\n"                                         \
  "    log.value += ev.data + '\\n';\n"                                       \
  "  };\n"                                                                    \
  "  btn.onclick = function() {\n"                                            \
  "    log.value += '<You>: ' + msg.value + '\\n';\n"                         \
  "    ws.send(msg.value);\n"                                                 \
  "  };\n"                                                                    \
  "  msg.onkeyup = function(ev) {\n"                                          \
  "    if (ev.keyCode === 13) {\n"                                            \
  "      ev.preventDefault();\n"                                              \
  "      ev.stopPropagation();\n"                                             \
  "      btn.click();\n"                                                      \
  "      msg.value = '';\n"                                                   \
  "    }\n"                                                                   \
  "  };\n"                                                                    \
  "});\n"                                                                     \
  "</script>\n"                                                               \
  "</head>\n"                                                                 \
  "<body>\n"                                                                  \
  "<input type='text' id='msg' autofocus/>\n"                                 \
  "<input type='button' id='send' value='Send' /><br /><br />\n"              \
  "<textarea id='log' rows='20' cols='28'></textarea>\n"                      \
  "</body>\n"                                                                 \
  "</html>"
#define BAD_REQUEST_PAGE                                                      \
  "<html>\n"                                                                  \
  "<head>\n"                                                                  \
  "<title>WebSocket chat</title>\n"                                           \
  "</head>\n"                                                                 \
  "<body>\n"                                                                  \
  "Bad Request\n"                                                             \
  "</body>\n"                                                                 \
  "</html>\n"
#define UPGRADE_REQUIRED_PAGE                                                 \
  "<html>\n"                                                                  \
  "<head>\n"                                                                  \
  "<title>WebSocket chat</title>\n"                                           \
  "</head>\n"                                                                 \
  "<body>\n"                                                                  \
  "Upgrade required\n"                                                        \
  "</body>\n"                                                                 \
  "</html>\n"

#define WS_SEC_WEBSOCKET_VERSION "13"
#define WS_UPGRADE_VALUE "websocket"
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_GUID_LEN 36
#define WS_KEY_LEN 24
#define WS_KEY_GUID_LEN ((WS_KEY_LEN) + (WS_GUID_LEN))
#define WS_FIN 128
#define WS_OPCODE_TEXT_FRAME 1
#define WS_OPCODE_CON_CLOSE_FRAME 8

#define MAX_CLIENTS 10

static MHD_socket CLIENT_SOCKS[MAX_CLIENTS];

static pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;

struct WsData
{
  struct MHD_UpgradeResponseHandle *urh;
  MHD_socket sock;
};


/********** begin SHA-1 **********/


#define SHA1HashSize 20

#define SHA1CircularShift(bits, word)                                         \
  (((word) << (bits)) | ((word) >> (32 - (bits))))

enum SHA1_RESULT
{
  SHA1_RESULT_SUCCESS = 0,
  SHA1_RESULT_NULL = 1,
  SHA1_RESULT_STATE_ERROR = 2
};

struct SHA1Context
{
  uint32_t intermediate_hash[SHA1HashSize / 4];
  uint32_t length_low;
  uint32_t length_high;
  int_least16_t message_block_index;
  unsigned char message_block[64];
  int computed;
  int corrupted;
};

static void
SHA1ProcessMessageBlock (struct SHA1Context *context)
{
  const uint32_t K[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
  int i;
  uint32_t temp;
  uint32_t W[80];
  uint32_t A, B, C, D, E;

  for (i = 0; i < 16; i++)
  {
    W[i] = context->message_block[i * 4] << 24;
    W[i] |= context->message_block[i * 4 + 1] << 16;
    W[i] |= context->message_block[i * 4 + 2] << 8;
    W[i] |= context->message_block[i * 4 + 3];
  }
  for (i = 16; i < 80; i++)
  {
    W[i]
      = SHA1CircularShift (1, W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16]);
  }
  A = context->intermediate_hash[0];
  B = context->intermediate_hash[1];
  C = context->intermediate_hash[2];
  D = context->intermediate_hash[3];
  E = context->intermediate_hash[4];
  for (i = 0; i < 20; i++)
  {
    temp = SHA1CircularShift (5, A) + ((B & C) | ((~B) & D)) + E + W[i]
           + K[0];
    E = D;
    D = C;
    C = SHA1CircularShift (30, B);
    B = A;
    A = temp;
  }
  for (i = 20; i < 40; i++)
  {
    temp = SHA1CircularShift (5, A) + (B ^ C ^ D) + E + W[i] + K[1];
    E = D;
    D = C;
    C = SHA1CircularShift (30, B);
    B = A;
    A = temp;
  }
  for (i = 40; i < 60; i++)
  {
    temp = SHA1CircularShift (5, A) + ((B & C) | (B & D) | (C & D)) + E
           + W[i] + K[2];
    E = D;
    D = C;
    C = SHA1CircularShift (30, B);
    B = A;
    A = temp;
  }
  for (i = 60; i < 80; i++)
  {
    temp = SHA1CircularShift (5, A) + (B ^ C ^ D) + E + W[i] + K[3];
    E = D;
    D = C;
    C = SHA1CircularShift (30, B);
    B = A;
    A = temp;
  }
  context->intermediate_hash[0] += A;
  context->intermediate_hash[1] += B;
  context->intermediate_hash[2] += C;
  context->intermediate_hash[3] += D;
  context->intermediate_hash[4] += E;
  context->message_block_index = 0;
}


static void
SHA1PadMessage (struct SHA1Context *context)
{
  if (context->message_block_index > 55)
  {
    context->message_block[context->message_block_index++] = 0x80;
    while (context->message_block_index < 64)
    {
      context->message_block[context->message_block_index++] = 0;
    }
    SHA1ProcessMessageBlock (context);
    while (context->message_block_index < 56)
    {
      context->message_block[context->message_block_index++] = 0;
    }
  }
  else
  {
    context->message_block[context->message_block_index++] = 0x80;
    while (context->message_block_index < 56)
    {
      context->message_block[context->message_block_index++] = 0;
    }
  }
  context->message_block[56] = context->length_high >> 24;
  context->message_block[57] = context->length_high >> 16;
  context->message_block[58] = context->length_high >> 8;
  context->message_block[59] = context->length_high;
  context->message_block[60] = context->length_low >> 24;
  context->message_block[61] = context->length_low >> 16;
  context->message_block[62] = context->length_low >> 8;
  context->message_block[63] = context->length_low;
  SHA1ProcessMessageBlock (context);
}


static enum SHA1_RESULT
SHA1Reset (struct SHA1Context *context)
{
  if (! context)
  {
    return SHA1_RESULT_NULL;
  }
  context->length_low = 0;
  context->length_high = 0;
  context->message_block_index = 0;
  context->intermediate_hash[0] = 0x67452301;
  context->intermediate_hash[1] = 0xEFCDAB89;
  context->intermediate_hash[2] = 0x98BADCFE;
  context->intermediate_hash[3] = 0x10325476;
  context->intermediate_hash[4] = 0xC3D2E1F0;
  context->computed = 0;
  context->corrupted = 0;
  return SHA1_RESULT_SUCCESS;
}


static enum SHA1_RESULT
SHA1Result (struct SHA1Context *context, unsigned char
            Message_Digest[SHA1HashSize])
{
  int i;

  if (! context || ! Message_Digest)
  {
    return SHA1_RESULT_NULL;
  }
  if (context->corrupted)
  {
    return SHA1_RESULT_STATE_ERROR;
  }
  if (! context->computed)
  {
    SHA1PadMessage (context);
    for (i = 0; i < 64; ++i)
    {
      context->message_block[i] = 0;
    }
    context->length_low = 0;
    context->length_high = 0;
    context->computed = 1;
  }
  for (i = 0; i < SHA1HashSize; ++i)
  {
    Message_Digest[i]
      = context->intermediate_hash[i >> 2] >> 8 * (3 - (i & 0x03));
  }
  return SHA1_RESULT_SUCCESS;
}


static enum SHA1_RESULT
SHA1Input (struct SHA1Context *context, const unsigned char *message_array,
           unsigned length)
{
  if (! length)
  {
    return SHA1_RESULT_SUCCESS;
  }
  if (! context || ! message_array)
  {
    return SHA1_RESULT_NULL;
  }
  if (context->computed)
  {
    context->corrupted = 1;
    return SHA1_RESULT_STATE_ERROR;
  }
  if (context->corrupted)
  {
    return SHA1_RESULT_STATE_ERROR;
  }
  while (length-- && ! context->corrupted)
  {
    context->message_block[context->message_block_index++]
      = (*message_array & 0xFF);
    context->length_low += 8;
    if (context->length_low == 0)
    {
      context->length_high++;
      if (context->length_high == 0)
      {
        context->corrupted = 1;
      }
    }
    if (context->message_block_index == 64)
    {
      SHA1ProcessMessageBlock (context);
    }
    message_array++;
  }
  return SHA1_RESULT_SUCCESS;
}


/********** end SHA-1 **********/


/********** begin Base64 **********/


ssize_t
BASE64Encode (const void *in, size_t len, char **output)
{
#define FILLCHAR '='
  const char *cvt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz"
                    "0123456789+/";
  const char *data = in;
  char *opt;
  ssize_t ret;
  size_t i;
  char c;
  ret = 0;

  opt = malloc (2 + (len * 4 / 3) + 8);
  if (NULL == opt)
  {
    return -1;
  }
  for (i = 0; i < len; ++i)
  {
    c = (data[i] >> 2) & 0x3F;
    opt[ret++] = cvt[(int) c];
    c = (data[i] << 4) & 0x3F;
    if (++i < len)
    {
      c |= (data[i] >> 4) & 0x0F;
    }
    opt[ret++] = cvt[(int) c];
    if (i < len)
    {
      c = (data[i] << 2) & 0x3F;
      if (++i < len)
      {
        c |= (data[i] >> 6) & 0x03;
      }
      opt[ret++] = cvt[(int) c];
    }
    else
    {
      ++i;
      opt[ret++] = FILLCHAR;
    }
    if (i < len)
    {
      c = data[i] & 0x3F;
      opt[ret++] = cvt[(int) c];
    }
    else
    {
      opt[ret++] = FILLCHAR;
    }
  }
  *output = opt;
  return ret;
}


/********** end Base64 **********/


static enum MHD_Result
is_websocket_request (struct MHD_Connection *con, const char *upg_header,
                      const char *con_header)
{

  (void) con;  /* Unused. Silent compiler warning. */

  return ((upg_header != NULL) && (con_header != NULL)
          && (0 == strcmp (upg_header, WS_UPGRADE_VALUE))
          && (NULL != strstr (con_header, "Upgrade")))
         ? MHD_YES
         : MHD_NO;
}


static enum MHD_Result
send_chat_page (struct MHD_Connection *con)
{
  struct MHD_Response *res;
  enum MHD_Result ret;

  res = MHD_create_response_from_buffer (strlen (CHAT_PAGE), (void *) CHAT_PAGE,
                                         MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (con, MHD_HTTP_OK, res);
  MHD_destroy_response (res);
  return ret;
}


static enum MHD_Result
send_bad_request (struct MHD_Connection *con)
{
  struct MHD_Response *res;
  enum MHD_Result ret;

  res = MHD_create_response_from_buffer (strlen (BAD_REQUEST_PAGE),
                                         (void *) BAD_REQUEST_PAGE,
                                         MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (con, MHD_HTTP_BAD_REQUEST, res);
  MHD_destroy_response (res);
  return ret;
}


static enum MHD_Result
send_upgrade_required (struct MHD_Connection *con)
{
  struct MHD_Response *res;
  enum MHD_Result ret;

  res = MHD_create_response_from_buffer (strlen (UPGRADE_REQUIRED_PAGE),
                                         (void *) UPGRADE_REQUIRED_PAGE,
                                         MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header (res, MHD_HTTP_HEADER_SEC_WEBSOCKET_VERSION,
                           WS_SEC_WEBSOCKET_VERSION);
  ret = MHD_queue_response (con, MHD_HTTP_UPGRADE_REQUIRED, res);
  MHD_destroy_response (res);
  return ret;
}


static enum MHD_Result
ws_get_accept_value (const char *key, char **val)
{
  struct SHA1Context ctx;
  unsigned char hash[SHA1HashSize];
  char *str;
  ssize_t len;

  if ( (NULL == key) || (WS_KEY_LEN != strlen (key)))
  {
    return MHD_NO;
  }
  str = malloc (WS_KEY_LEN + WS_GUID_LEN + 1);
  if (NULL == str)
  {
    return MHD_NO;
  }
  strncpy (str, key, (WS_KEY_LEN + 1));
  strncpy (str + WS_KEY_LEN, WS_GUID, WS_GUID_LEN + 1);
  SHA1Reset (&ctx);
  SHA1Input (&ctx, (const unsigned char *) str, WS_KEY_GUID_LEN);
  if (SHA1_RESULT_SUCCESS != SHA1Result (&ctx, hash))
  {
    free (str);
    return MHD_NO;
  }
  free (str);
  len = BASE64Encode (hash, SHA1HashSize, val);
  if (-1 == len)
  {
    return MHD_NO;
  }
  (*val)[len] = '\0';
  return MHD_YES;
}


static void
make_blocking (MHD_socket fd)
{
#if defined(MHD_POSIX_SOCKETS)
  int flags;

  flags = fcntl (fd, F_GETFL);
  if (-1 == flags)
    abort ();
  if ((flags & ~O_NONBLOCK) != flags)
    if (-1 == fcntl (fd, F_SETFL, flags & ~O_NONBLOCK))
      abort ();
#elif defined(MHD_WINSOCK_SOCKETS)
  unsigned long flags = 0;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    abort ();
#endif /* MHD_WINSOCK_SOCKETS */
}


static size_t
send_all (MHD_socket sock, const unsigned char *buf, size_t len)
{
  ssize_t ret;
  size_t off;

  for (off = 0; off < len; off += ret)
  {
    ret = send (sock, (const void *) &buf[off], len - off, 0);
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
    {
      break;
    }
  }
  return off;
}


static int
ws_send_frame (MHD_socket sock, const char *msg, size_t length)
{
  unsigned char *response;
  unsigned char frame[10];
  unsigned char idx_first_rdata;
  int idx_response;
  int output;
  MHD_socket isock;
  size_t i;

  frame[0] = (WS_FIN | WS_OPCODE_TEXT_FRAME);
  if (length <= 125)
  {
    frame[1] = length & 0x7F;
    idx_first_rdata = 2;
  }
#if SIZEOF_SIZE_T > 4
  else if (0xFFFF < length)
  {
    frame[1] = 127;
    frame[2] = (unsigned char) ((length >> 56) & 0xFF);
    frame[3] = (unsigned char) ((length >> 48) & 0xFF);
    frame[4] = (unsigned char) ((length >> 40) & 0xFF);
    frame[5] = (unsigned char) ((length >> 32) & 0xFF);
    frame[6] = (unsigned char) ((length >> 24) & 0xFF);
    frame[7] = (unsigned char) ((length >> 16) & 0xFF);
    frame[8] = (unsigned char) ((length >> 8) & 0xFF);
    frame[9] = (unsigned char) (length & 0xFF);
    idx_first_rdata = 10;
  }
#endif /* SIZEOF_SIZE_T > 4 */
  else
  {
    frame[1] = 126;
    frame[2] = (length >> 8) & 0xFF;
    frame[3] = length & 0xFF;
    idx_first_rdata = 4;
  }
  idx_response = 0;
  response = malloc (idx_first_rdata + length + 1);
  if (NULL == response)
  {
    return -1;
  }
  for (i = 0; i < idx_first_rdata; i++)
  {
    response[i] = frame[i];
    idx_response++;
  }
  for (i = 0; i < length; i++)
  {
    response[idx_response] = msg[i];
    idx_response++;
  }
  response[idx_response] = '\0';
  output = 0;
  pthread_mutex_lock (&MUTEX);
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    isock = CLIENT_SOCKS[i];
    if ((isock != MHD_INVALID_SOCKET) && (isock != sock))
    {
      output += send_all (isock, response, idx_response);
    }
  }
  pthread_mutex_unlock (&MUTEX);
  free (response);
  return output;
}


static unsigned char *
ws_receive_frame (unsigned char *frame, ssize_t *length, int *type)
{
  unsigned char masks[4];
  unsigned char mask;
  unsigned char *msg;
  unsigned char flength;
  unsigned char idx_first_mask;
  unsigned char idx_first_data;
  ssize_t data_length;
  int i;
  int j;

  msg = NULL;
  if (frame[0] == (WS_FIN | WS_OPCODE_TEXT_FRAME))
  {
    *type = WS_OPCODE_TEXT_FRAME;
    idx_first_mask = 2;
    mask = frame[1];
    flength = mask & 0x7F;
    if (flength == 126)
    {
      idx_first_mask = 4;
    }
    else if (flength == 127)
    {
      idx_first_mask = 10;
    }
    idx_first_data = idx_first_mask + 4;
    data_length = *length - idx_first_data;
    masks[0] = frame[idx_first_mask + 0];
    masks[1] = frame[idx_first_mask + 1];
    masks[2] = frame[idx_first_mask + 2];
    masks[3] = frame[idx_first_mask + 3];
    msg = malloc (data_length + 1);
    if (NULL != msg)
    {
      for (i = idx_first_data, j = 0; i < *length; i++, j++)
      {
        msg[j] = frame[i] ^ masks[j % 4];
      }
      *length = data_length;
      msg[j] = '\0';
    }
  }
  else if (frame[0] == (WS_FIN | WS_OPCODE_CON_CLOSE_FRAME))
  {
    *type = WS_OPCODE_CON_CLOSE_FRAME;
  }
  else
  {
    *type = frame[0] & 0x0F;
  }
  return msg;
}


static void *
run_usock (void *cls)
{
  struct WsData *ws = cls;
  struct MHD_UpgradeResponseHandle *urh = ws->urh;
  unsigned char buf[2048];
  unsigned char *msg;
  char client[20];
  char *text;
  ssize_t got;
  size_t size;
  int type;
  int sent;
  int i;

  make_blocking (ws->sock);
  while (1)
  {
    got = recv (ws->sock, (void *) buf, sizeof (buf), 0);
    if (0 >= got)
    {
      break;
    }
    msg = ws_receive_frame (buf, &got, &type);
    if (NULL == msg)
    {
      break;
    }
    if (type == WS_OPCODE_TEXT_FRAME)
    {
      size = sprintf (client, "User#%d: ", (int) ws->sock);
      size += got;
      text = malloc (size);
      if (NULL != text)
      {
        sprintf (text, "%s%s", client, msg);
        sent = ws_send_frame (ws->sock, text, size);
        free (text);
      }
      else
      {
        sent = -1;
      }
      free (msg);
      if (-1 == sent)
      {
        break;
      }
    }
    else
    {
      if (type == WS_OPCODE_CON_CLOSE_FRAME)
      {
        free (msg);
        break;
      }
    }
  }
  pthread_mutex_lock (&MUTEX);
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    if (CLIENT_SOCKS[i] == ws->sock)
    {
      CLIENT_SOCKS[i] = MHD_INVALID_SOCKET;
      break;
    }
  }
  pthread_mutex_unlock (&MUTEX);
  free (ws);
  MHD_upgrade_action (urh, MHD_UPGRADE_ACTION_CLOSE);
  return NULL;
}


static void
uh_cb (void *cls, struct MHD_Connection *con, void *con_cls,
       const char *extra_in, size_t extra_in_size, MHD_socket sock,
       struct MHD_UpgradeResponseHandle *urh)
{
  struct WsData *ws;
  pthread_t pt;
  int sock_overflow;
  int i;

  (void) cls;            /* Unused. Silent compiler warning. */
  (void) con;            /* Unused. Silent compiler warning. */
  (void) con_cls;        /* Unused. Silent compiler warning. */
  (void) extra_in;       /* Unused. Silent compiler warning. */
  (void) extra_in_size;  /* Unused. Silent compiler warning. */

  ws = malloc (sizeof (struct WsData));
  if (NULL == ws)
    abort ();
  memset (ws, 0, sizeof (struct WsData));
  ws->sock = sock;
  ws->urh = urh;
  sock_overflow = MHD_YES;
  pthread_mutex_lock (&MUTEX);
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    if (MHD_INVALID_SOCKET == CLIENT_SOCKS[i])
    {
      CLIENT_SOCKS[i] = ws->sock;
      sock_overflow = MHD_NO;
      break;
    }
  }
  pthread_mutex_unlock (&MUTEX);
  if (sock_overflow)
  {
    free (ws);
    MHD_upgrade_action (urh, MHD_UPGRADE_ACTION_CLOSE);
    return;
  }
  if (0 != pthread_create (&pt, NULL, &run_usock, ws))
    abort ();
  /* Note that by detaching like this we make it impossible to ensure
     a clean shutdown, as the we stop the daemon even if a worker thread
     is still running. Alas, this is a simple example... */
  pthread_detach (pt);
}


static enum MHD_Result
ahc_cb (void *cls, struct MHD_Connection *con, const char *url,
        const char *method, const char *version, const char *upload_data,
        size_t *upload_data_size, void **ptr)
{
  struct MHD_Response *res;
  const char *upg_header;
  const char *con_header;
  const char *ws_version_header;
  const char *ws_key_header;
  char *ws_ac_value;
  enum MHD_Result ret;
  size_t key_size;

  (void) cls;               /* Unused. Silent compiler warning. */
  (void) url;               /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (NULL == *ptr)
  {
    *ptr = (void *) 1;
    return MHD_YES;
  }
  *ptr = NULL;
  upg_header = MHD_lookup_connection_value (con, MHD_HEADER_KIND,
                                            MHD_HTTP_HEADER_UPGRADE);
  con_header = MHD_lookup_connection_value (con, MHD_HEADER_KIND,
                                            MHD_HTTP_HEADER_CONNECTION);
  if (MHD_NO == is_websocket_request (con, upg_header, con_header))
  {
    return send_chat_page (con);
  }
  if ((0 != strcmp (method, MHD_HTTP_METHOD_GET))
      || (0 != strcmp (version, MHD_HTTP_VERSION_1_1)))
  {
    return send_bad_request (con);
  }
  ws_version_header = MHD_lookup_connection_value (
    con, MHD_HEADER_KIND, MHD_HTTP_HEADER_SEC_WEBSOCKET_VERSION);
  if ((NULL == ws_version_header)
      || (0 != strcmp (ws_version_header, WS_SEC_WEBSOCKET_VERSION)))
  {
    return send_upgrade_required (con);
  }
  ret = MHD_lookup_connection_value_n (
    con, MHD_HEADER_KIND,
    MHD_HTTP_HEADER_SEC_WEBSOCKET_KEY,
    strlen (MHD_HTTP_HEADER_SEC_WEBSOCKET_KEY),
    &ws_key_header, &key_size);
  if ((MHD_NO == ret) || (key_size != WS_KEY_LEN))
  {
    return send_bad_request (con);
  }
  ret = ws_get_accept_value (ws_key_header, &ws_ac_value);
  if (MHD_NO == ret)
  {
    return ret;
  }
  res = MHD_create_response_for_upgrade (&uh_cb, NULL);
  MHD_add_response_header (res, MHD_HTTP_HEADER_UPGRADE, WS_UPGRADE_VALUE);
  MHD_add_response_header (res, MHD_HTTP_HEADER_SEC_WEBSOCKET_ACCEPT,
                           ws_ac_value);
  free (ws_ac_value);
  ret = MHD_queue_response (con, MHD_HTTP_SWITCHING_PROTOCOLS, res);
  MHD_destroy_response (res);
  return ret;
}


int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *d;
  uint16_t port;
  size_t i;

  if (argc != 2)
  {
    printf ("%s PORT\n", argv[0]);
    return 1;
  }
  port = atoi (argv[1]);
  d = MHD_start_daemon (MHD_ALLOW_UPGRADE | MHD_USE_AUTO_INTERNAL_THREAD
                        | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_cb, &port, MHD_OPTION_END);
  if (NULL == d)
    return 1;
  for (i = 0; i < sizeof(CLIENT_SOCKS) / sizeof(CLIENT_SOCKS[0]); ++i)
    CLIENT_SOCKS[i] = MHD_INVALID_SOCKET;
  (void) getc (stdin);
  MHD_stop_daemon (d);
  return 0;
}
