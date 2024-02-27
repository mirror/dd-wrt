/*
     This file is part of libmicrohttpd
     Copyright (C) 2007-2020 Daniel Pittman and Christian Grothoff
     Copyright (C) 2015-2024 Evgeny Grin (Karlson2k)

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
 * @file connection.c
 * @brief  Methods for managing connections
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */
#include "internal.h"
#include "mhd_limits.h"
#include "connection.h"
#include "memorypool.h"
#include "response.h"
#include "mhd_mono_clock.h"
#include "mhd_str.h"
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#include "mhd_locks.h"
#endif
#include "mhd_sockets.h"
#include "mhd_compat.h"
#include "mhd_itc.h"
#ifdef MHD_LINUX_SOLARIS_SENDFILE
#include <sys/sendfile.h>
#endif /* MHD_LINUX_SOLARIS_SENDFILE */
#if defined(HAVE_FREEBSD_SENDFILE) || defined(HAVE_DARWIN_SENDFILE)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif /* HAVE_FREEBSD_SENDFILE || HAVE_DARWIN_SENDFILE */
#ifdef HTTPS_SUPPORT
#include "connection_https.h"
#endif /* HTTPS_SUPPORT */
#ifdef HAVE_SYS_PARAM_H
/* For FreeBSD version identification */
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */
#include "mhd_send.h"
#include "mhd_assert.h"

/**
 * Get whether bare LF in HTTP header and other protocol elements
 * should be treated as the line termination depending on the configured
 * strictness level.
 * RFC 9112, section 2.2
 */
#define MHD_ALLOW_BARE_LF_AS_CRLF_(discp_lvl) (0 >= discp_lvl)

/**
 * The reasonable length of the upload chunk "header" (the size specifier
 * with optional chunk extension).
 * MHD tries to keep the space in the read buffer large enough to read
 * the chunk "header" in one step.
 * The real "header" could be much larger, it will be handled correctly
 * anyway, however it may require several rounds of buffer grow.
 */
#define MHD_CHUNK_HEADER_REASONABLE_LEN 24

/**
 * Message to transmit when http 1.1 request is received
 */
#define HTTP_100_CONTINUE "HTTP/1.1 100 Continue\r\n\r\n"

/**
 * Response text used when the request (http header) is too big to
 * be processed.
 */
#ifdef HAVE_MESSAGES
#define ERR_MSG_REQUEST_TOO_BIG \
  "<html>" \
  "<head><title>Request too big</title></head>" \
  "<body>Request HTTP header is too big for the memory constraints " \
  "of this webserver.</body>" \
  "</html>"
#else
#define ERR_MSG_REQUEST_TOO_BIG ""
#endif

/**
 * Response text used when the request header is too big to be processed.
 */
#ifdef HAVE_MESSAGES
#define ERR_MSG_REQUEST_HEADER_TOO_BIG \
  "<html>" \
  "<head><title>Request too big</title></head>" \
  "<body><p>The total size of the request headers, which includes the " \
  "request target and the request field lines, exceeds the memory " \
  "constraints of this web server.</p>" \
  "<p>The request could be re-tried with shorter field lines, a shorter " \
  "request target or a shorter request method token.</p></body>" \
  "</html>"
#else
#define ERR_MSG_REQUEST_HEADER_TOO_BIG ""
#endif

/**
 * Response text used when the request cookie header is too big to be processed.
 */
#ifdef HAVE_MESSAGES
#define ERR_MSG_REQUEST_HEADER_WITH_COOKIES_TOO_BIG \
  "<html>" \
  "<head><title>Request too big</title></head>" \
  "<body><p>The total size of the request headers, which includes the " \
  "request target and the request field lines, exceeds the memory " \
  "constraints of this web server.</p> " \
  "<p>The request could be re-tried with smaller " \
  "<b>&quot;Cookie:&quot;</b> field value, shorter other field lines, " \
  "a shorter request target or a shorter request method token.</p></body> " \
  "</html>"
#else
#define ERR_MSG_REQUEST_HEADER_WITH_COOKIES_TOO_BIG ""
#endif

/**
 * Response text used when the request chunk size line with chunk extension
 * cannot fit the buffer.
 */
#ifdef HAVE_MESSAGES
#define ERR_MSG_REQUEST_CHUNK_LINE_EXT_TOO_BIG \
  "<html>" \
  "<head><title>Request too big</title></head>" \
  "<body><p>The total size of the request target, the request field lines " \
  "and the chunk size line exceeds the memory constraints of this web " \
  "server.</p>" \
  "<p>The request could be re-tried without chunk extensions, with a smaller " \
  "chunk size, shorter field lines, a shorter request target or a shorter " \
  "request method token.</p></body>" \
  "</html>"
#else
#define ERR_MSG_REQUEST_CHUNK_LINE_EXT_TOO_BIG ""
#endif

/**
 * Response text used when the request chunk size line without chunk extension
 * cannot fit the buffer.
 */
#ifdef HAVE_MESSAGES
#define ERR_MSG_REQUEST_CHUNK_LINE_TOO_BIG \
  "<html>" \
  "<head><title>Request too big</title></head>" \
  "<body><p>The total size of the request target, the request field lines " \
  "and the chunk size line exceeds the memory constraints of this web " \
  "server.</p>" \
  "<p>The request could be re-tried with a smaller " \
  "chunk size, shorter field lines, a shorter request target or a shorter " \
  "request method token.</p></body>" \
  "</html>"
#else
#define ERR_MSG_REQUEST_CHUNK_LINE_TOO_BIG ""
#endif

/**
 * Response text used when the request header is too big to be processed.
 */
#ifdef HAVE_MESSAGES
#define ERR_MSG_REQUEST_FOOTER_TOO_BIG \
  "<html>" \
  "<head><title>Request too big</title></head>" \
  "<body><p>The total size of the request headers, which includes the " \
  "request target, the request field lines and the chunked trailer " \
  "section exceeds the memory constraints of this web server.</p>" \
  "<p>The request could be re-tried with a shorter chunked trailer " \
  "section, shorter field lines, a shorter request target or " \
  "a shorter request method token.</p></body>" \
  "</html>"
#else
#define ERR_MSG_REQUEST_FOOTER_TOO_BIG ""
#endif

/**
 * Response text used when the request line has more then two whitespaces.
 */
#ifdef HAVE_MESSAGES
#define RQ_LINE_TOO_MANY_WSP \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>The request line has more then two whitespaces.</body>" \
  "</html>"
#else
#define RQ_LINE_TOO_MANY_WSP ""
#endif

/**
 * Response text used when the request HTTP header has bare CR character
 * without LF character (and CR is not allowed to be treated as whitespace).
 */
#ifdef HAVE_MESSAGES
#define BARE_CR_IN_HEADER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>Request HTTP header has bare CR character without " \
  "following LF character.</body>" \
  "</html>"
#else
#define BARE_CR_IN_HEADER ""
#endif

/**
 * Response text used when the request HTTP footer has bare CR character
 * without LF character (and CR is not allowed to be treated as whitespace).
 */
#ifdef HAVE_MESSAGES
#define BARE_CR_IN_FOOTER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>Request HTTP footer has bare CR character without " \
  "following LF character.</body>" \
  "</html>"
#else
#define BARE_CR_IN_FOOTER ""
#endif

/**
 * Response text used when the request HTTP header has bare LF character
 * without CR character.
 */
#ifdef HAVE_MESSAGES
#define BARE_LF_IN_HEADER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>Request HTTP header has bare LF character without " \
  "preceding CR character.</body>" \
  "</html>"
#else
#define BARE_LF_IN_HEADER ""
#endif

/**
 * Response text used when the request HTTP footer has bare LF character
 * without CR character.
 */
#ifdef HAVE_MESSAGES
#define BARE_LF_IN_FOOTER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>Request HTTP footer has bare LF character without " \
  "preceding CR character.</body>" \
  "</html>"
#else
#define BARE_LF_IN_FOOTER ""
#endif

/**
 * Response text used when the request line has invalid characters in URI.
 */
#ifdef HAVE_MESSAGES
#define RQ_TARGET_INVALID_CHAR \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request has invalid characters in " \
  "the request-target.</body>" \
  "</html>"
#else
#define RQ_TARGET_INVALID_CHAR ""
#endif

/**
 * Response text used when line folding is used in request headers.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_OBS_FOLD \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>Obsolete line folding is used in HTTP request header.</body>" \
  "</html>"
#else
#define ERR_RSP_OBS_FOLD ""
#endif

/**
 * Response text used when line folding is used in request footers.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_OBS_FOLD_FOOTER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>Obsolete line folding is used in HTTP request footer.</body>" \
  "</html>"
#else
#define ERR_RSP_OBS_FOLD_FOOTER ""
#endif

/**
 * Response text used when the request has whitespace at the start
 * of the first header line.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_WSP_BEFORE_HEADER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request has whitespace between the request line and " \
  "the first header.</body>" \
  "</html>"
#else
#define ERR_RSP_WSP_BEFORE_HEADER ""
#endif

/**
 * Response text used when the request has whitespace at the start
 * of the first footer line.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_WSP_BEFORE_FOOTER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>First HTTP footer line has whitespace at the first " \
  "position.</body>" \
  "</html>"
#else
#define ERR_RSP_WSP_BEFORE_FOOTER ""
#endif

/**
 * Response text used when the whitespace found before colon (inside header
 * name or between header name and colon).
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_WSP_IN_HEADER_NAME \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request has whitespace before the first colon " \
  "in header line.</body>" \
  "</html>"
#else
#define ERR_RSP_WSP_IN_HEADER_NAME ""
#endif

/**
 * Response text used when the whitespace found before colon (inside header
 * name or between header name and colon).
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_WSP_IN_FOOTER_NAME \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request has whitespace before the first colon " \
  "in footer line.</body>" \
  "</html>"
#else
#define ERR_RSP_WSP_IN_FOOTER_NAME ""
#endif

/**
 * Response text used when request header has invalid character.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_INVALID_CHR_IN_HEADER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request has invalid character in header.</body>" \
  "</html>"
#else
#define ERR_RSP_INVALID_CHR_IN_HEADER ""
#endif

/**
 * Response text used when request header has invalid character.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_INVALID_CHR_IN_FOOTER \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request has invalid character in footer.</body>" \
  "</html>"
#else
#define ERR_RSP_INVALID_CHR_IN_FOOTER ""
#endif

/**
 * Response text used when request header has no colon character.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_HEADER_WITHOUT_COLON \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request header line has no colon character.</body>" \
  "</html>"
#else
#define ERR_RSP_HEADER_WITHOUT_COLON ""
#endif

/**
 * Response text used when request footer has no colon character.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_FOOTER_WITHOUT_COLON \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request footer line has no colon character.</body>" \
  "</html>"
#else
#define ERR_RSP_FOOTER_WITHOUT_COLON ""
#endif

/**
 * Response text used when request header has zero-length header (filed) name.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_EMPTY_HEADER_NAME \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request header has empty header name.</body>" \
  "</html>"
#else
#define ERR_RSP_EMPTY_HEADER_NAME ""
#endif

/**
 * Response text used when request header has zero-length header (filed) name.
 */
#ifdef HAVE_MESSAGES
#define ERR_RSP_EMPTY_FOOTER_NAME \
  "<html>" \
  "<head><title>Request broken</title></head>" \
  "<body>HTTP request footer has empty footer name.</body>" \
  "</html>"
#else
#define ERR_RSP_EMPTY_FOOTER_NAME ""
#endif

/**
 * Response text used when the request (http header) does not
 * contain a "Host:" header and still claims to be HTTP 1.1.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_LACKS_HOST \
  "<html>" \
  "<head><title>&quot;Host:&quot; header required</title></head>" \
  "<body>HTTP/1.1 request without <b>&quot;Host:&quot;</b>.</body>" \
  "</html>"

#else
#define REQUEST_LACKS_HOST ""
#endif

/**
 * Response text used when the request has unsupported "Transfer-Encoding:".
 */
#ifdef HAVE_MESSAGES
#define REQUEST_UNSUPPORTED_TR_ENCODING \
  "<html>" \
  "<head><title>Unsupported Transfer-Encoding</title></head>" \
  "<body>The Transfer-Encoding used in request is not supported.</body>" \
  "</html>"
#else
#define REQUEST_UNSUPPORTED_TR_ENCODING ""
#endif

/**
 * Response text used when the request has unsupported both headers:
 * "Transfer-Encoding:" and "Content-Length:"
 */
#ifdef HAVE_MESSAGES
#define REQUEST_LENGTH_WITH_TR_ENCODING \
  "<html>" \
  "<head><title>Malformed request</title></head>" \
  "<body>Wrong combination of the request headers: both Transfer-Encoding " \
  "and Content-Length headers are used at the same time.</body>" \
  "</html>"
#else
#define REQUEST_LENGTH_WITH_TR_ENCODING ""
#endif

/**
 * Response text used when the request (http header) is
 * malformed.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_MALFORMED \
  "<html><head><title>Request malformed</title></head>" \
  "<body>HTTP request is syntactically incorrect.</body></html>"
#else
#define REQUEST_MALFORMED ""
#endif

/**
 * Response text used when the request HTTP chunked encoding is
 * malformed.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CHUNKED_MALFORMED \
  "<html><head><title>Request malformed</title></head>" \
  "<body>HTTP chunked encoding is syntactically incorrect.</body></html>"
#else
#define REQUEST_CHUNKED_MALFORMED ""
#endif

/**
 * Response text used when the request HTTP chunk is too large.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CHUNK_TOO_LARGE \
  "<html><head><title>Request content too large</title></head>" \
  "<body>The chunk size used in HTTP chunked encoded " \
  "request is too large.</body></html>"
#else
#define REQUEST_CHUNK_TOO_LARGE ""
#endif

/**
 * Response text used when the request HTTP content is too large.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CONTENTLENGTH_TOOLARGE \
  "<html><head><title>Request content too large</title></head>" \
  "<body>HTTP request has too large value for " \
  "<b>Content-Length</b> header.</body></html>"
#else
#define REQUEST_CONTENTLENGTH_TOOLARGE ""
#endif

/**
 * Response text used when the request HTTP chunked encoding is
 * malformed.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CONTENTLENGTH_MALFORMED \
  "<html><head><title>Request malformed</title></head>" \
  "<body>HTTP request has wrong value for " \
  "<b>Content-Length</b> header.</body></html>"
#else
#define REQUEST_CONTENTLENGTH_MALFORMED ""
#endif

/**
 * Response text used when there is an internal server error.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define ERROR_MSG_DATA_NOT_HANDLED_BY_APP \
  "<html><head><title>Internal server error</title></head>" \
  "<body>Please ask the developer of this Web server to carefully " \
  "read the GNU libmicrohttpd documentation about connection " \
  "management and blocking.</body></html>"
#else
#define ERROR_MSG_DATA_NOT_HANDLED_BY_APP ""
#endif

/**
 * Response text used when the request HTTP version is too old.
 */
#ifdef HAVE_MESSAGES
#define REQ_HTTP_VER_IS_TOO_OLD \
  "<html><head><title>Requested HTTP version is not supported</title></head>" \
  "<body>Requested HTTP version is too old and not " \
  "supported.</body></html>"
#else
#define REQ_HTTP_VER_IS_TOO_OLD ""
#endif

/**
 * Response text used when the request HTTP version is not supported.
 */
#ifdef HAVE_MESSAGES
#define REQ_HTTP_VER_IS_NOT_SUPPORTED \
  "<html><head><title>Requested HTTP version is not supported</title></head>" \
  "<body>Requested HTTP version is not supported.</body></html>"
#else
#define REQ_HTTP_VER_IS_NOT_SUPPORTED ""
#endif


/**
 * sendfile() chuck size
 */
#define MHD_SENFILE_CHUNK_         (0x20000)

/**
 * sendfile() chuck size for thread-per-connection
 */
#define MHD_SENFILE_CHUNK_THR_P_C_ (0x200000)

#ifdef HAVE_MESSAGES
/**
 * Return text description for MHD_ERR_*_ codes
 * @param mhd_err_code the error code
 * @return pointer to static string with error description
 */
static const char *
str_conn_error_ (ssize_t mhd_err_code)
{
  switch (mhd_err_code)
  {
  case MHD_ERR_AGAIN_:
    return _ ("The operation would block, retry later");
  case MHD_ERR_CONNRESET_:
    return _ ("The connection was forcibly closed by remote peer");
  case MHD_ERR_NOTCONN_:
    return _ ("The socket is not connected");
  case MHD_ERR_NOMEM_:
    return _ ("Not enough system resources to serve the request");
  case MHD_ERR_BADF_:
    return _ ("Bad FD value");
  case MHD_ERR_INVAL_:
    return _ ("Argument value is invalid");
  case MHD_ERR_OPNOTSUPP_:
    return _ ("Argument value is not supported");
  case MHD_ERR_PIPE_:
    return _ ("The socket is no longer available for sending");
  case MHD_ERR_TLS_:
    return _ ("TLS encryption or decryption error");
  default:
    break;   /* Mute compiler warning */
  }
  if (0 <= mhd_err_code)
    return _ ("Not an error code");

  mhd_assert (0); /* Should never be reachable */
  return _ ("Wrong error code value");
}


#endif /* HAVE_MESSAGES */

/**
 * Allocate memory from connection's memory pool.
 * If memory pool doesn't have enough free memory but read or write buffer
 * have some unused memory, the size of the buffer will be reduced as needed.
 * @param connection the connection to use
 * @param size the size of allocated memory area
 * @return pointer to allocated memory region in the pool or
 *         NULL if no memory is available
 */
void *
MHD_connection_alloc_memory_ (struct MHD_Connection *connection,
                              size_t size)
{
  struct MHD_Connection *const c = connection; /* a short alias */
  struct MemoryPool *const pool = c->pool;     /* a short alias */
  size_t need_to_be_freed = 0; /**< The required amount of additional free memory */
  void *res;

  res = MHD_pool_try_alloc (pool,
                            size,
                            &need_to_be_freed);
  if (NULL != res)
    return res;

  if (MHD_pool_is_resizable_inplace (pool,
                                     c->write_buffer,
                                     c->write_buffer_size))
  {
    if (c->write_buffer_size - c->write_buffer_append_offset >=
        need_to_be_freed)
    {
      char *buf;
      const size_t new_buf_size = c->write_buffer_size - need_to_be_freed;
      buf = MHD_pool_reallocate (pool,
                                 c->write_buffer,
                                 c->write_buffer_size,
                                 new_buf_size);
      mhd_assert (c->write_buffer == buf);
      mhd_assert (c->write_buffer_append_offset <= new_buf_size);
      mhd_assert (c->write_buffer_send_offset <= new_buf_size);
      c->write_buffer_size = new_buf_size;
      c->write_buffer = buf;
    }
    else
      return NULL;
  }
  else if (MHD_pool_is_resizable_inplace (pool,
                                          c->read_buffer,
                                          c->read_buffer_size))
  {
    if (c->read_buffer_size - c->read_buffer_offset >= need_to_be_freed)
    {
      char *buf;
      const size_t new_buf_size = c->read_buffer_size - need_to_be_freed;
      buf = MHD_pool_reallocate (pool,
                                 c->read_buffer,
                                 c->read_buffer_size,
                                 new_buf_size);
      mhd_assert (c->read_buffer == buf);
      mhd_assert (c->read_buffer_offset <= new_buf_size);
      c->read_buffer_size = new_buf_size;
      c->read_buffer = buf;
    }
    else
      return NULL;
  }
  else
    return NULL;
  res = MHD_pool_allocate (pool, size, true);
  mhd_assert (NULL != res); /* It has been checked that pool has enough space */
  return res;
}


/**
 * Callback for receiving data from the socket.
 *
 * @param connection the MHD connection structure
 * @param other where to write received data to
 * @param i maximum size of other (in bytes)
 * @return positive value for number of bytes actually received or
 *         negative value for error number MHD_ERR_xxx_
 */
static ssize_t
recv_param_adapter (struct MHD_Connection *connection,
                    void *other,
                    size_t i)
{
  ssize_t ret;

  if ( (MHD_INVALID_SOCKET == connection->socket_fd) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
  {
    return MHD_ERR_NOTCONN_;
  }
  if (i > MHD_SCKT_SEND_MAX_SIZE_)
    i = MHD_SCKT_SEND_MAX_SIZE_; /* return value limit */

  ret = MHD_recv_ (connection->socket_fd,
                   other,
                   i);
  if (0 > ret)
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
    {
#ifdef EPOLL_SUPPORT
      /* Got EAGAIN --- no longer read-ready */
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
#endif /* EPOLL_SUPPORT */
      return MHD_ERR_AGAIN_;
    }
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return MHD_ERR_AGAIN_;
    if (MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
      return MHD_ERR_CONNRESET_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EOPNOTSUPP_))
      return MHD_ERR_OPNOTSUPP_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_ENOTCONN_))
      return MHD_ERR_NOTCONN_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EINVAL_))
      return MHD_ERR_INVAL_;
    if (MHD_SCKT_ERR_IS_LOW_RESOURCES_ (err))
      return MHD_ERR_NOMEM_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EBADF_))
      return MHD_ERR_BADF_;
    /* Treat any other error as a hard error. */
    return MHD_ERR_NOTCONN_;
  }
#ifdef EPOLL_SUPPORT
  else if (i > (size_t) ret)
    connection->epoll_state &=
      ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
#endif /* EPOLL_SUPPORT */
  return ret;
}


/**
 * Get all of the headers from the request.
 *
 * @param connection connection to get values from
 * @param kind types of values to iterate over, can be a bitmask
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over
 *         -1 if connection is NULL.
 * @ingroup request
 */
_MHD_EXTERN int
MHD_get_connection_values (struct MHD_Connection *connection,
                           enum MHD_ValueKind kind,
                           MHD_KeyValueIterator iterator,
                           void *iterator_cls)
{
  int ret;
  struct MHD_HTTP_Req_Header *pos;

  if (NULL == connection)
    return -1;
  ret = 0;
  for (pos = connection->rq.headers_received; NULL != pos; pos = pos->next)
    if (0 != (pos->kind & kind))
    {
      ret++;
      if ( (NULL != iterator) &&
           (MHD_NO == iterator (iterator_cls,
                                pos->kind,
                                pos->header,
                                pos->value)) )
        return ret;
    }
  return ret;
}


/**
 * Get all of the headers from the request.
 *
 * @param connection connection to get values from
 * @param kind types of values to iterate over, can be a bitmask
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over,
 *         -1 if connection is NULL.
 * @ingroup request
 */
_MHD_EXTERN int
MHD_get_connection_values_n (struct MHD_Connection *connection,
                             enum MHD_ValueKind kind,
                             MHD_KeyValueIteratorN iterator,
                             void *iterator_cls)
{
  int ret;
  struct MHD_HTTP_Req_Header *pos;

  if (NULL == connection)
    return -1;
  ret = 0;

  if (NULL == iterator)
    for (pos = connection->rq.headers_received; NULL != pos; pos = pos->next)
    {
      if (0 != (kind & pos->kind))
        ret++;
    }
  else
    for (pos = connection->rq.headers_received; NULL != pos; pos = pos->next)
      if (0 != (kind & pos->kind))
      {
        ret++;
        if (MHD_NO == iterator (iterator_cls,
                                pos->kind,
                                pos->header,
                                pos->header_size,
                                pos->value,
                                pos->value_size))
          return ret;
      }
  return ret;
}


/**
 * This function can be used to add an arbitrary entry to connection.
 * Internal version of #MHD_set_connection_value_n() without checking
 * of arguments values.
 *
 * @param connection the connection for which a
 *                   value should be set
 * @param kind kind of the value
 * @param key key for the value, must be zero-terminated
 * @param key_size number of bytes in @a key (excluding 0-terminator)
 * @param value the value itself, must be zero-terminated
 * @param value_size number of bytes in @a value (excluding 0-terminator)
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
static enum MHD_Result
MHD_set_connection_value_n_nocheck_ (struct MHD_Connection *connection,
                                     enum MHD_ValueKind kind,
                                     const char *key,
                                     size_t key_size,
                                     const char *value,
                                     size_t value_size)
{
  struct MHD_HTTP_Req_Header *pos;

  pos = MHD_connection_alloc_memory_ (connection,
                                      sizeof (struct MHD_HTTP_Res_Header));
  if (NULL == pos)
    return MHD_NO;
  pos->header = key;
  pos->header_size = key_size;
  pos->value = value;
  pos->value_size = value_size;
  pos->kind = kind;
  pos->next = NULL;
  /* append 'pos' to the linked list of headers */
  if (NULL == connection->rq.headers_received_tail)
  {
    connection->rq.headers_received = pos;
    connection->rq.headers_received_tail = pos;
  }
  else
  {
    connection->rq.headers_received_tail->next = pos;
    connection->rq.headers_received_tail = pos;
  }
  return MHD_YES;
}


/**
 * This function can be used to add an arbitrary entry to connection.
 * This function could add entry with binary zero, which is allowed
 * for #MHD_GET_ARGUMENT_KIND. For other kind on entries it is
 * recommended to use #MHD_set_connection_value.
 *
 * This function MUST only be called from within the
 * #MHD_AccessHandlerCallback (otherwise, access maybe improperly
 * synchronized).  Furthermore, the client must guarantee that the key
 * and value arguments are 0-terminated strings that are NOT freed
 * until the connection is closed.  (The easiest way to do this is by
 * passing only arguments to permanently allocated strings.).
 *
 * @param connection the connection for which a
 *  value should be set
 * @param kind kind of the value
 * @param key key for the value, must be zero-terminated
 * @param key_size number of bytes in @a key (excluding 0-terminator)
 * @param value the value itself, must be zero-terminated
 * @param value_size number of bytes in @a value (excluding 0-terminator)
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
_MHD_EXTERN enum MHD_Result
MHD_set_connection_value_n (struct MHD_Connection *connection,
                            enum MHD_ValueKind kind,
                            const char *key,
                            size_t key_size,
                            const char *value,
                            size_t value_size)
{
  if ( (MHD_GET_ARGUMENT_KIND != kind) &&
       ( ((key ? strlen (key) : 0) != key_size) ||
         ((value ? strlen (value) : 0) != value_size) ) )
    return MHD_NO; /* binary zero is allowed only in GET arguments */

  return MHD_set_connection_value_n_nocheck_ (connection,
                                              kind,
                                              key,
                                              key_size,
                                              value,
                                              value_size);
}


/**
 * This function can be used to add an entry to the HTTP headers of a
 * connection (so that the #MHD_get_connection_values function will
 * return them -- and the `struct MHD_PostProcessor` will also see
 * them).  This maybe required in certain situations (see Mantis
 * #1399) where (broken) HTTP implementations fail to supply values
 * needed by the post processor (or other parts of the application).
 *
 * This function MUST only be called from within the
 * #MHD_AccessHandlerCallback (otherwise, access maybe improperly
 * synchronized).  Furthermore, the client must guarantee that the key
 * and value arguments are 0-terminated strings that are NOT freed
 * until the connection is closed.  (The easiest way to do this is by
 * passing only arguments to permanently allocated strings.).
 *
 * @param connection the connection for which a
 *  value should be set
 * @param kind kind of the value
 * @param key key for the value
 * @param value the value itself
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
_MHD_EXTERN enum MHD_Result
MHD_set_connection_value (struct MHD_Connection *connection,
                          enum MHD_ValueKind kind,
                          const char *key,
                          const char *value)
{
  return MHD_set_connection_value_n_nocheck_ (connection,
                                              kind,
                                              key,
                                              NULL != key
                                              ? strlen (key)
                                              : 0,
                                              value,
                                              NULL != value
                                              ? strlen (value)
                                              : 0);
}


/**
 * Get a particular header value.  If multiple
 * values match the kind, return any one of them.
 *
 * @param connection connection to get values from
 * @param kind what kind of value are we looking for
 * @param key the header to look for, NULL to lookup 'trailing' value without a key
 * @return NULL if no such item was found
 * @ingroup request
 */
_MHD_EXTERN const char *
MHD_lookup_connection_value (struct MHD_Connection *connection,
                             enum MHD_ValueKind kind,
                             const char *key)
{
  const char *value;

  value = NULL;
  (void) MHD_lookup_connection_value_n (connection,
                                        kind,
                                        key,
                                        (NULL == key) ? 0 : strlen (key),
                                        &value,
                                        NULL);
  return value;
}


/**
 * Get a particular header value.  If multiple
 * values match the kind, return any one of them.
 * @note Since MHD_VERSION 0x00096304
 *
 * @param connection connection to get values from
 * @param kind what kind of value are we looking for
 * @param key the header to look for, NULL to lookup 'trailing' value without a key
 * @param key_size the length of @a key in bytes
 * @param[out] value_ptr the pointer to variable, which will be set to found value,
 *                       will not be updated if key not found,
 *                       could be NULL to just check for presence of @a key
 * @param[out] value_size_ptr the pointer variable, which will set to found value,
 *                            will not be updated if key not found,
 *                            could be NULL
 * @return #MHD_YES if key is found,
 *         #MHD_NO otherwise.
 * @ingroup request
 */
_MHD_EXTERN enum MHD_Result
MHD_lookup_connection_value_n (struct MHD_Connection *connection,
                               enum MHD_ValueKind kind,
                               const char *key,
                               size_t key_size,
                               const char **value_ptr,
                               size_t *value_size_ptr)
{
  struct MHD_HTTP_Req_Header *pos;

  if (NULL == connection)
    return MHD_NO;

  if (NULL == key)
  {
    for (pos = connection->rq.headers_received; NULL != pos; pos = pos->next)
    {
      if ( (0 != (kind & pos->kind)) &&
           (NULL == pos->header) )
        break;
    }
  }
  else
  {
    for (pos = connection->rq.headers_received; NULL != pos; pos = pos->next)
    {
      if ( (0 != (kind & pos->kind)) &&
           (key_size == pos->header_size) &&
           ( (key == pos->header) ||
             (MHD_str_equal_caseless_bin_n_ (key,
                                             pos->header,
                                             key_size) ) ) )
        break;
    }
  }

  if (NULL == pos)
    return MHD_NO;

  if (NULL != value_ptr)
    *value_ptr = pos->value;

  if (NULL != value_size_ptr)
    *value_size_ptr = pos->value_size;

  return MHD_YES;
}


/**
 * Check whether request header contains particular token.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 * @param connection the connection to get values from
 * @param header     the header name
 * @param header_len the length of header, not including optional
 *                   terminating null-character
 * @param token      the token to find
 * @param token_len  the length of token, not including optional
 *                   terminating null-character.
 * @return true if token is found in specified header,
 *         false otherwise
 */
static bool
MHD_lookup_header_token_ci (const struct MHD_Connection *connection,
                            const char *header,
                            size_t header_len,
                            const char *token,
                            size_t token_len)
{
  struct MHD_HTTP_Req_Header *pos;

  if ((NULL == connection) || (NULL == header) || (0 == header[0]) ||
      (NULL == token) || (0 == token[0]))
    return false;

  for (pos = connection->rq.headers_received; NULL != pos; pos = pos->next)
  {
    if ((0 != (pos->kind & MHD_HEADER_KIND)) &&
        (header_len == pos->header_size) &&
        ( (header == pos->header) ||
          (MHD_str_equal_caseless_bin_n_ (header,
                                          pos->header,
                                          header_len)) ) &&
        (MHD_str_has_token_caseless_ (pos->value, token, token_len)))
      return true;
  }
  return false;
}


/**
 * Check whether request header contains particular static @a tkn.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 * @param c   the connection to get values from
 * @param h   the static string of header name
 * @param tkn the static string of token to find
 * @return true if token is found in specified header,
 *         false otherwise
 */
#define MHD_lookup_header_s_token_ci(c,h,tkn) \
  MHD_lookup_header_token_ci ((c),(h),MHD_STATICSTR_LEN_ (h), \
                              (tkn),MHD_STATICSTR_LEN_ (tkn))


/**
 * Do we (still) need to send a 100 continue
 * message for this connection?
 *
 * @param connection connection to test
 * @return false if we don't need 100 CONTINUE, true if we do
 */
static bool
need_100_continue (struct MHD_Connection *connection)
{
  const char *expect;

  if (! MHD_IS_HTTP_VER_1_1_COMPAT (connection->rq.http_ver))
    return false;

  if (0 == connection->rq.remaining_upload_size)
    return false;

  if (MHD_NO ==
      MHD_lookup_connection_value_n (connection,
                                     MHD_HEADER_KIND,
                                     MHD_HTTP_HEADER_EXPECT,
                                     MHD_STATICSTR_LEN_ ( \
                                       MHD_HTTP_HEADER_EXPECT),
                                     &expect,
                                     NULL))
    return false;

  if (MHD_str_equal_caseless_ (expect,
                               "100-continue"))
    return true;

  return false;
}


/**
 * Mark connection as "closed".
 * @remark To be called from any thread.
 *
 * @param connection connection to close
 */
void
MHD_connection_mark_closed_ (struct MHD_Connection *connection)
{
  const struct MHD_Daemon *daemon = connection->daemon;

  if (0 == (daemon->options & MHD_USE_TURBO))
  {
#ifdef HTTPS_SUPPORT
    /* For TLS connection use shutdown of TLS layer
     * and do not shutdown TCP socket. This give more
     * chances to send TLS closure data to remote side.
     * Closure of TLS layer will be interpreted by
     * remote side as end of transmission. */
    if (0 != (daemon->options & MHD_USE_TLS))
    {
      if (! MHD_tls_connection_shutdown (connection))
        shutdown (connection->socket_fd,
                  SHUT_WR);
    }
    else   /* Combined with next 'shutdown()'. */
#endif /* HTTPS_SUPPORT */
    shutdown (connection->socket_fd,
              SHUT_WR);
  }
  connection->state = MHD_CONNECTION_CLOSED;
  connection->event_loop_info = MHD_EVENT_LOOP_INFO_CLEANUP;
}


/**
 * Close the given connection and give the
 * specified termination code to the user.
 * @remark To be called only from thread that
 * process connection's recv(), send() and response.
 *
 * @param connection connection to close
 * @param termination_code termination reason to give
 */
void
MHD_connection_close_ (struct MHD_Connection *connection,
                       enum MHD_RequestTerminationCode termination_code)
{
  struct MHD_Daemon *daemon = connection->daemon;
  struct MHD_Response *resp = connection->rp.response;

  mhd_assert (! connection->suspended);
#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (connection->tid) );
#endif /* MHD_USE_THREADS */
  if ( (NULL != daemon->notify_completed) &&
       (connection->rq.client_aware) )
    daemon->notify_completed (daemon->notify_completed_cls,
                              connection,
                              &connection->rq.client_context,
                              termination_code);
  connection->rq.client_aware = false;
  if (NULL != resp)
  {
    connection->rp.response = NULL;
    MHD_destroy_response (resp);
  }
  if (NULL != connection->pool)
  {
    MHD_pool_destroy (connection->pool);
    connection->pool = NULL;
  }

  MHD_connection_mark_closed_ (connection);
}


#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
/**
 * Stop TLS forwarding on upgraded connection and
 * reflect remote disconnect state to socketpair.
 * @remark In thread-per-connection mode this function
 * can be called from any thread, in other modes this
 * function must be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param connection the upgraded connection
 */
void
MHD_connection_finish_forward_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  struct MHD_UpgradeResponseHandle *urh = connection->urh;

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
#endif /* MHD_USE_THREADS */

  if (0 == (daemon->options & MHD_USE_TLS))
    return; /* Nothing to do with non-TLS connection. */

  if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    DLL_remove (daemon->urh_head,
                daemon->urh_tail,
                urh);
#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon) &&
      (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                       EPOLL_CTL_DEL,
                       connection->socket_fd,
                       NULL)) )
  {
    MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
  }
  if (urh->in_eready_list)
  {
    EDLL_remove (daemon->eready_urh_head,
                 daemon->eready_urh_tail,
                 urh);
    urh->in_eready_list = false;
  }
#endif /* EPOLL_SUPPORT */
  if (MHD_INVALID_SOCKET != urh->mhd.socket)
  {
#ifdef EPOLL_SUPPORT
    if (MHD_D_IS_USING_EPOLL_ (daemon) &&
        (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                         EPOLL_CTL_DEL,
                         urh->mhd.socket,
                         NULL)) )
    {
      MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
    }
#endif /* EPOLL_SUPPORT */
    /* Reflect remote disconnect to application by breaking
     * socketpair connection. */
    shutdown (urh->mhd.socket, SHUT_RDWR);
  }
  /* Socketpair sockets will remain open as they will be
   * used with MHD_UPGRADE_ACTION_CLOSE. They will be
   * closed by cleanup_upgraded_connection() during
   * connection's final cleanup.
   */
}


#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT*/


/**
 * A serious error occurred, close the
 * connection (and notify the application).
 *
 * @param connection connection to close with error
 * @param emsg error message (can be NULL)
 */
static void
connection_close_error (struct MHD_Connection *connection,
                        const char *emsg)
{
  connection->stop_with_error = true;
  connection->discard_request = true;
#ifdef HAVE_MESSAGES
  if (NULL != emsg)
    MHD_DLOG (connection->daemon,
              "%s\n",
              emsg);
#else  /* ! HAVE_MESSAGES */
  (void) emsg; /* Mute compiler warning. */
#endif /* ! HAVE_MESSAGES */
  MHD_connection_close_ (connection,
                         MHD_REQUEST_TERMINATED_WITH_ERROR);
}


/**
 * Macro to only include error message in call to
 * #connection_close_error() if we have HAVE_MESSAGES.
 */
#ifdef HAVE_MESSAGES
#define CONNECTION_CLOSE_ERROR(c, emsg) connection_close_error (c, emsg)
#else
#define CONNECTION_CLOSE_ERROR(c, emsg) connection_close_error (c, NULL)
#endif


/**
 * Prepare the response buffer of this connection for
 * sending.  Assumes that the response mutex is
 * already held.  If the transmission is complete,
 * this function may close the socket (and return
 * #MHD_NO).
 *
 * @param connection the connection
 * @return #MHD_NO if readying the response failed (the
 *  lock on the response will have been released already
 *  in this case).
 */
static enum MHD_Result
try_ready_normal_body (struct MHD_Connection *connection)
{
  ssize_t ret;
  struct MHD_Response *response;

  response = connection->rp.response;
  mhd_assert (connection->rp.props.send_reply_body);

  if ( (0 == response->total_size) ||
                     /* TODO: replace the next check with assert */
       (connection->rp.rsp_write_position == response->total_size) )
    return MHD_YES;  /* 0-byte response is always ready */
  if (NULL != response->data_iov)
  {
    size_t copy_size;

    if (NULL != connection->rp.resp_iov.iov)
      return MHD_YES;
    copy_size = response->data_iovcnt * sizeof(MHD_iovec_);
    connection->rp.resp_iov.iov = MHD_connection_alloc_memory_ (connection,
                                                                copy_size);
    if (NULL == connection->rp.resp_iov.iov)
    {
      MHD_mutex_unlock_chk_ (&response->mutex);
      /* not enough memory */
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection (out of memory)."));
      return MHD_NO;
    }
    memcpy (connection->rp.resp_iov.iov,
            response->data_iov,
            copy_size);
    connection->rp.resp_iov.cnt = response->data_iovcnt;
    connection->rp.resp_iov.sent = 0;
    return MHD_YES;
  }
  if (NULL == response->crc)
    return MHD_YES;
  if ( (response->data_start <=
        connection->rp.rsp_write_position) &&
       (response->data_size + response->data_start >
        connection->rp.rsp_write_position) )
    return MHD_YES; /* response already ready */
#if defined(_MHD_HAVE_SENDFILE)
  if (MHD_resp_sender_sendfile == connection->rp.resp_sender)
  {
    /* will use sendfile, no need to bother response crc */
    return MHD_YES;
  }
#endif /* _MHD_HAVE_SENDFILE */

  ret = response->crc (response->crc_cls,
                       connection->rp.rsp_write_position,
                       (char *) response->data,
                       (size_t) MHD_MIN ((uint64_t) response->data_buffer_size,
                                         response->total_size
                                         - connection->rp.rsp_write_position));
  if (0 > ret)
  {
    /* either error or http 1.0 transfer, close socket! */
    /* TODO: do not update total size, check whether response
     * was really with unknown size */
    response->total_size = connection->rp.rsp_write_position;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    if (MHD_CONTENT_READER_END_OF_STREAM == ret)
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_COMPLETED_OK);
    else
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection (application reported " \
                                 "error generating data)."));
    return MHD_NO;
  }
  response->data_start = connection->rp.rsp_write_position;
  response->data_size = (size_t) ret;
  if (0 == ret)
  {
    connection->state = MHD_CONNECTION_NORMAL_BODY_UNREADY;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    return MHD_NO;
  }
  return MHD_YES;
}


/**
 * Prepare the response buffer of this connection for sending.
 * Assumes that the response mutex is already held.  If the
 * transmission is complete, this function may close the socket (and
 * return #MHD_NO).
 *
 * @param connection the connection
 * @param[out] p_finished the pointer to variable that will be set to "true"
 *                        when application returned indication of the end
 *                        of the stream
 * @return #MHD_NO if readying the response failed
 */
static enum MHD_Result
try_ready_chunked_body (struct MHD_Connection *connection,
                        bool *p_finished)
{
  ssize_t ret;
  struct MHD_Response *response;
  static const size_t max_chunk = 0xFFFFFF;
  char chunk_hdr[6];            /* 6: max strlen of "FFFFFF" */
  /* "FFFFFF" + "\r\n" */
  static const size_t max_chunk_hdr_len = sizeof(chunk_hdr) + 2;
  /* "FFFFFF" + "\r\n" + "\r\n" (chunk termination) */
  static const size_t max_chunk_overhead = sizeof(chunk_hdr) + 2 + 2;
  size_t chunk_hdr_len;
  uint64_t left_to_send;
  size_t size_to_fill;

  response = connection->rp.response;
  mhd_assert (NULL != response->crc || NULL != response->data);

  mhd_assert (0 == connection->write_buffer_append_offset);

  /* The buffer must be reasonably large enough */
  if (128 > connection->write_buffer_size)
  {
    size_t size;

    size = connection->write_buffer_size + MHD_pool_get_free (connection->pool);
    if (128 > size)
    {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      MHD_mutex_unlock_chk_ (&response->mutex);
#endif
      /* not enough memory */
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection (out of memory)."));
      return MHD_NO;
    }
    /* Limit the buffer size to the largest usable size for chunks */
    if ( (max_chunk + max_chunk_overhead) < size)
      size = max_chunk + max_chunk_overhead;
    mhd_assert ((NULL == connection->write_buffer) || \
                MHD_pool_is_resizable_inplace (connection->pool, \
                                               connection->write_buffer, \
                                               connection->write_buffer_size));
    connection->write_buffer =
      MHD_pool_reallocate (connection->pool,
                           connection->write_buffer,
                           connection->write_buffer_size,
                           size);
    mhd_assert (NULL != connection->write_buffer);
    connection->write_buffer_size = size;
  }
  mhd_assert (max_chunk_overhead < connection->write_buffer_size);

  if (MHD_SIZE_UNKNOWN == response->total_size)
    left_to_send = MHD_SIZE_UNKNOWN;
  else
    left_to_send = response->total_size
                   - connection->rp.rsp_write_position;

  size_to_fill = connection->write_buffer_size - max_chunk_overhead;
  /* Limit size for the callback to the max usable size */
  if (max_chunk < size_to_fill)
    size_to_fill = max_chunk;
  if (left_to_send < size_to_fill)
    size_to_fill = (size_t) left_to_send;

  if (0 == left_to_send)
    /* nothing to send, don't bother calling crc */
    ret = MHD_CONTENT_READER_END_OF_STREAM;
  else if ( (response->data_start <=
             connection->rp.rsp_write_position) &&
            (response->data_start + response->data_size >
             connection->rp.rsp_write_position) )
  {
    /* difference between rsp_write_position and data_start is less
       than data_size which is size_t type, no need to check for overflow */
    const size_t data_write_offset
      = (size_t) (connection->rp.rsp_write_position
                  - response->data_start);
    /* buffer already ready, use what is there for the chunk */
    mhd_assert (SSIZE_MAX >= (response->data_size - data_write_offset));
    mhd_assert (response->data_size >= data_write_offset);
    ret = (ssize_t) (response->data_size - data_write_offset);
    if ( ((size_t) ret) > size_to_fill)
      ret = (ssize_t) size_to_fill;
    memcpy (&connection->write_buffer[max_chunk_hdr_len],
            &response->data[data_write_offset],
            (size_t) ret);
  }
  else
  {
    if (NULL == response->crc)
    { /* There is no way to reach this code */
#if defined(MHD_USE_THREADS)
      MHD_mutex_unlock_chk_ (&response->mutex);
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("No callback for the chunked data."));
      return MHD_NO;
    }
    ret = response->crc (response->crc_cls,
                         connection->rp.rsp_write_position,
                         &connection->write_buffer[max_chunk_hdr_len],
                         size_to_fill);
  }
  if (MHD_CONTENT_READER_END_WITH_ERROR == ret)
  {
    /* error, close socket! */
    /* TODO: remove update of the response size */
    response->total_size = connection->rp.rsp_write_position;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Closing connection (application error " \
                               "generating response)."));
    return MHD_NO;
  }
  if (MHD_CONTENT_READER_END_OF_STREAM == ret)
  {
    *p_finished = true;
    /* TODO: remove update of the response size */
    response->total_size = connection->rp.rsp_write_position;
    return MHD_YES;
  }
  if (0 == ret)
  {
    connection->state = MHD_CONNECTION_CHUNKED_BODY_UNREADY;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    return MHD_NO;
  }
  if (size_to_fill < (size_t) ret)
  {
#if defined(MHD_USE_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Closing connection (application returned " \
                               "more data than requested)."));
    return MHD_NO;
  }
  chunk_hdr_len = MHD_uint32_to_strx ((uint32_t) ret, chunk_hdr,
                                      sizeof(chunk_hdr));
  mhd_assert (chunk_hdr_len != 0);
  mhd_assert (chunk_hdr_len < sizeof(chunk_hdr));
  *p_finished = false;
  connection->write_buffer_send_offset =
    (max_chunk_hdr_len - (chunk_hdr_len + 2));
  memcpy (connection->write_buffer + connection->write_buffer_send_offset,
          chunk_hdr,
          chunk_hdr_len);
  connection->write_buffer[max_chunk_hdr_len - 2] = '\r';
  connection->write_buffer[max_chunk_hdr_len - 1] = '\n';
  connection->write_buffer[max_chunk_hdr_len + (size_t) ret] = '\r';
  connection->write_buffer[max_chunk_hdr_len + (size_t) ret + 1] = '\n';
  connection->rp.rsp_write_position += (size_t) ret;
  connection->write_buffer_append_offset = max_chunk_hdr_len + (size_t) ret + 2;
  return MHD_YES;
}


/**
 * Are we allowed to keep the given connection alive?
 * We can use the TCP stream for a second request if the connection
 * is HTTP 1.1 and the "Connection" header either does not exist or
 * is not set to "close", or if the connection is HTTP 1.0 and the
 * "Connection" header is explicitly set to "keep-alive".
 * If no HTTP version is specified (or if it is not 1.0 or 1.1), we
 * definitively close the connection.  If the "Connection" header is
 * not exactly "close" or "keep-alive", we proceed to use the default
 * for the respective HTTP version.
 * If response has HTTP/1.0 flag or has "Connection: close" header
 * then connection must be closed.
 * If full request has not been read then connection must be closed
 * as well.
 *
 * @param connection the connection to check for keepalive
 * @return MHD_CONN_USE_KEEPALIVE if (based on the request and the response),
 *         a keepalive is legal,
 *         MHD_CONN_MUST_CLOSE if connection must be closed after sending
 *         complete reply,
 *         MHD_CONN_MUST_UPGRADE if connection must be upgraded.
 */
static enum MHD_ConnKeepAlive
keepalive_possible (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->rp.response;  /**< a short alias */

  mhd_assert (NULL != r);
  if (MHD_CONN_MUST_CLOSE == c->keepalive)
    return MHD_CONN_MUST_CLOSE;

#ifdef UPGRADE_SUPPORT
  /* TODO: Move below the next check when MHD stops closing connections
   * when response is queued in first callback */
  if (NULL != r->upgrade_handler)
  {
    /* No "close" token is enforced by 'add_response_header_connection()' */
    mhd_assert (0 == (r->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE));
    /* Valid HTTP version is enforced by 'MHD_queue_response()' */
    mhd_assert (MHD_IS_HTTP_VER_SUPPORTED (c->rq.http_ver));
    mhd_assert (! c->stop_with_error);
    return MHD_CONN_MUST_UPGRADE;
  }
#endif /* UPGRADE_SUPPORT */

  mhd_assert ( (! c->stop_with_error) || (c->discard_request));
  if ((c->read_closed) || (c->discard_request))
    return MHD_CONN_MUST_CLOSE;

  if (0 != (r->flags & MHD_RF_HTTP_1_0_COMPATIBLE_STRICT))
    return MHD_CONN_MUST_CLOSE;
  if (0 != (r->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE))
    return MHD_CONN_MUST_CLOSE;

  if (! MHD_IS_HTTP_VER_SUPPORTED (c->rq.http_ver))
    return MHD_CONN_MUST_CLOSE;

  if (MHD_lookup_header_s_token_ci (c,
                                    MHD_HTTP_HEADER_CONNECTION,
                                    "close"))
    return MHD_CONN_MUST_CLOSE;

  if ((MHD_HTTP_VER_1_0 == connection->rq.http_ver) ||
      (0 != (connection->rp.response->flags & MHD_RF_HTTP_1_0_SERVER)))
  {
    if (MHD_lookup_header_s_token_ci (connection,
                                      MHD_HTTP_HEADER_CONNECTION,
                                      "Keep-Alive"))
      return MHD_CONN_USE_KEEPALIVE;

    return MHD_CONN_MUST_CLOSE;
  }

  if (MHD_IS_HTTP_VER_1_1_COMPAT (c->rq.http_ver))
    return MHD_CONN_USE_KEEPALIVE;

  return MHD_CONN_MUST_CLOSE;
}


/**
 * Produce time stamp.
 *
 * Result is NOT null-terminated.
 * Result is always 29 bytes long.
 *
 * @param[out] date where to write the time stamp, with
 *             at least 29 bytes available space.
 */
static bool
get_date_str (char *date)
{
  static const char *const days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char *const mons[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static const size_t buf_len = 29;
  struct tm now;
  time_t t;
  const char *src;
#if ! defined(HAVE_C11_GMTIME_S) && ! defined(HAVE_W32_GMTIME_S) && \
  ! defined(HAVE_GMTIME_R)
  struct tm *pNow;
#endif

  if ((time_t) -1 == time (&t))
    return false;
#if defined(HAVE_C11_GMTIME_S)
  if (NULL == gmtime_s (&t,
                        &now))
    return false;
#elif defined(HAVE_W32_GMTIME_S)
  if (0 != gmtime_s (&now,
                     &t))
    return false;
#elif defined(HAVE_GMTIME_R)
  if (NULL == gmtime_r (&t,
                        &now))
    return false;
#else
  pNow = gmtime (&t);
  if (NULL == pNow)
    return false;
  now = *pNow;
#endif

  /* Day of the week */
  src = days[now.tm_wday % 7];
  date[0] = src[0];
  date[1] = src[1];
  date[2] = src[2];
  date[3] = ',';
  date[4] = ' ';
  /* Day of the month */
  if (2 != MHD_uint8_to_str_pad ((uint8_t) now.tm_mday, 2,
                                 date + 5, buf_len - 5))
    return false;
  date[7] = ' ';
  /* Month */
  src = mons[now.tm_mon % 12];
  date[8] = src[0];
  date[9] = src[1];
  date[10] = src[2];
  date[11] = ' ';
  /* Year */
  if (4 != MHD_uint16_to_str ((uint16_t) (1900 + now.tm_year), date + 12,
                              buf_len - 12))
    return false;
  date[16] = ' ';
  /* Time */
  MHD_uint8_to_str_pad ((uint8_t) now.tm_hour, 2, date + 17, buf_len - 17);
  date[19] = ':';
  MHD_uint8_to_str_pad ((uint8_t) now.tm_min, 2, date + 20, buf_len - 20);
  date[22] = ':';
  MHD_uint8_to_str_pad ((uint8_t) now.tm_sec, 2, date + 23, buf_len - 23);
  date[25] = ' ';
  date[26] = 'G';
  date[27] = 'M';
  date[28] = 'T';

  return true;
}


/**
 * Produce HTTP DATE header.
 * Result is always 37 bytes long (plus one terminating null).
 *
 * @param[out] header where to write the header, with
 *             at least 38 bytes available space.
 */
static bool
get_date_header (char *header)
{
  if (! get_date_str (header + 6))
  {
    header[0] = 0;
    return false;
  }
  header[0] = 'D';
  header[1] = 'a';
  header[2] = 't';
  header[3] = 'e';
  header[4] = ':';
  header[5] = ' ';
  header[35] = '\r';
  header[36] = '\n';
  header[37] = 0;
  return true;
}


/**
 * Try growing the read buffer.  We initially claim half the available
 * buffer space for the read buffer (the other half being left for
 * management data structures; the write buffer can in the end take
 * virtually everything as the read buffer can be reduced to the
 * minimum necessary at that point.
 *
 * @param connection the connection
 * @param required set to 'true' if grow is required, i.e. connection
 *                 will fail if no additional space is granted
 * @return 'true' on success, 'false' on failure
 */
static bool
try_grow_read_buffer (struct MHD_Connection *connection,
                      bool required)
{
  size_t new_size;
  size_t avail_size;
  const size_t def_grow_size = connection->daemon->pool_increment;
  void *rb;

  avail_size = MHD_pool_get_free (connection->pool);
  if (0 == avail_size)
    return false;               /* No more space available */
  if (0 == connection->read_buffer_size)
    new_size = avail_size / 2;  /* Use half of available buffer for reading */
  else
  {
    size_t grow_size;

    grow_size = avail_size / 8;
    if (def_grow_size > grow_size)
    {                  /* Shortage of space */
      const size_t left_free =
        connection->read_buffer_size - connection->read_buffer_offset;
      mhd_assert (connection->read_buffer_size >= \
                  connection->read_buffer_offset);
      if ((def_grow_size <= grow_size + left_free)
          && (left_free < def_grow_size))
        grow_size = def_grow_size - left_free;  /* Use precise 'def_grow_size' for new free space */
      else if (! required)
        return false;                           /* Grow is not mandatory, leave some space in pool */
      else
      {
        /* Shortage of space, but grow is mandatory */
        const size_t small_inc =
          ((MHD_BUF_INC_SIZE > def_grow_size) ?
           def_grow_size : MHD_BUF_INC_SIZE) / 8;
        if (small_inc < avail_size)
          grow_size = small_inc;
        else
          grow_size = avail_size;
      }
    }
    new_size = connection->read_buffer_size + grow_size;
  }
  /* Make sure that read buffer will not be moved */
  if ((NULL != connection->read_buffer) &&
      ! MHD_pool_is_resizable_inplace (connection->pool,
                                       connection->read_buffer,
                                       connection->read_buffer_size))
  {
    mhd_assert (0);
    return false;
  }
  /* we can actually grow the buffer, do it! */
  rb = MHD_pool_reallocate (connection->pool,
                            connection->read_buffer,
                            connection->read_buffer_size,
                            new_size);
  if (NULL == rb)
  {
    /* This should NOT be possible: we just computed 'new_size' so that
       it should fit. If it happens, somehow our read buffer is not in
       the right position in the pool, say because someone called
       MHD_pool_allocate() without 'from_end' set to 'true'? Anyway,
       should be investigated! (Ideally provide all data from
       *pool and connection->read_buffer and new_size for debugging). */
    mhd_assert (0);
    return false;
  }
  mhd_assert (connection->read_buffer == rb);
  connection->read_buffer = rb;
  mhd_assert (NULL != connection->read_buffer);
  connection->read_buffer_size = new_size;
  return true;
}


/**
 * Shrink connection read buffer to the zero size of free space in the buffer
 * @param connection the connection whose read buffer is being manipulated
 */
static void
connection_shrink_read_buffer (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  void *new_buf;

  if ((NULL == c->read_buffer) || (0 == c->read_buffer_size))
  {
    mhd_assert (0 == c->read_buffer_size);
    mhd_assert (0 == c->read_buffer_offset);
    return;
  }

  mhd_assert (c->read_buffer_offset <= c->read_buffer_size);
  if (0 == c->read_buffer_offset)
  {
    MHD_pool_deallocate (c->pool, c->read_buffer, c->read_buffer_size);
    c->read_buffer = NULL;
    c->read_buffer_size = 0;
  }
  else
  {
    mhd_assert (MHD_pool_is_resizable_inplace (c->pool, c->read_buffer, \
                                               c->read_buffer_size));
    new_buf = MHD_pool_reallocate (c->pool, c->read_buffer, c->read_buffer_size,
                                   c->read_buffer_offset);
    mhd_assert (c->read_buffer == new_buf);
    c->read_buffer = new_buf;
    c->read_buffer_size = c->read_buffer_offset;
  }
}


/**
 * Allocate the maximum available amount of memory from MemoryPool
 * for write buffer.
 * @param connection the connection whose write buffer is being manipulated
 * @return the size of the free space in the write buffer
 */
static size_t
connection_maximize_write_buffer (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MemoryPool *const pool = connection->pool;
  void *new_buf;
  size_t new_size;
  size_t free_size;

  mhd_assert ((NULL != c->write_buffer) || (0 == c->write_buffer_size));
  mhd_assert (c->write_buffer_append_offset >= c->write_buffer_send_offset);
  mhd_assert (c->write_buffer_size >= c->write_buffer_append_offset);

  free_size = MHD_pool_get_free (pool);
  if (0 != free_size)
  {
    new_size = c->write_buffer_size + free_size;
    /* This function must not move the buffer position.
     * MHD_pool_reallocate () may return the new position only if buffer was
     * allocated 'from_end' or is not the last allocation,
     * which should not happen. */
    mhd_assert ((NULL == c->write_buffer) || \
                MHD_pool_is_resizable_inplace (pool, c->write_buffer, \
                                               c->write_buffer_size));
    new_buf = MHD_pool_reallocate (pool,
                                   c->write_buffer,
                                   c->write_buffer_size,
                                   new_size);
    mhd_assert ((c->write_buffer == new_buf) || (NULL == c->write_buffer));
    c->write_buffer = new_buf;
    c->write_buffer_size = new_size;
    if (c->write_buffer_send_offset == c->write_buffer_append_offset)
    {
      /* All data have been sent, reset offsets to zero. */
      c->write_buffer_send_offset = 0;
      c->write_buffer_append_offset = 0;
    }
  }

  return c->write_buffer_size - c->write_buffer_append_offset;
}


#if 0 /* disable unused function */
/**
 * Shrink connection write buffer to the size of unsent data.
 *
 * @note: The number of calls of this function should be limited to avoid extra
 * zeroing of the memory.
 * @param connection the connection whose write buffer is being manipulated
 * @param connection the connection to manipulate write buffer
 */
static void
connection_shrink_write_buffer (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MemoryPool *const pool = connection->pool;
  void *new_buf;

  mhd_assert ((NULL != c->write_buffer) || (0 == c->write_buffer_size));
  mhd_assert (c->write_buffer_append_offset >= c->write_buffer_send_offset);
  mhd_assert (c->write_buffer_size >= c->write_buffer_append_offset);

  if ( (NULL == c->write_buffer) || (0 == c->write_buffer_size))
  {
    mhd_assert (0 == c->write_buffer_append_offset);
    mhd_assert (0 == c->write_buffer_send_offset);
    c->write_buffer = NULL;
    return;
  }
  if (c->write_buffer_append_offset == c->write_buffer_size)
    return;

  new_buf = MHD_pool_reallocate (pool, c->write_buffer, c->write_buffer_size,
                                 c->write_buffer_append_offset);
  mhd_assert ((c->write_buffer == new_buf) || \
              (0 == c->write_buffer_append_offset));
  c->write_buffer_size = c->write_buffer_append_offset;
  if (0 == c->write_buffer_size)
    c->write_buffer = NULL;
  else
    c->write_buffer = new_buf;
}


#endif /* unused function */


/**
 * Switch connection from recv mode to send mode.
 *
 * Current request header or body will not be read anymore,
 * response must be assigned to connection.
 * @param connection the connection to prepare for sending.
 */
static void
connection_switch_from_recv_to_send (struct MHD_Connection *connection)
{
  /* Read buffer is not needed for this request, shrink it.*/
  connection_shrink_read_buffer (connection);
}


/**
 * This enum type describes requirements for reply body and reply bode-specific
 * headers (namely Content-Length, Transfer-Encoding).
 */
enum replyBodyUse
{
  /**
   * No reply body allowed.
   * Reply body headers 'Content-Length:' or 'Transfer-Encoding: chunked' are
   * not allowed as well.
   */
  RP_BODY_NONE = 0,

  /**
   * Do not send reply body.
   * Reply body headers 'Content-Length:' or 'Transfer-Encoding: chunked' are
   * allowed, but optional.
   */
  RP_BODY_HEADERS_ONLY = 1,

  /**
   * Send reply body and
   * reply body headers 'Content-Length:' or 'Transfer-Encoding: chunked'.
   * Reply body headers are required.
   */
  RP_BODY_SEND = 2
};


/**
 * Check whether reply body must be used.
 *
 * If reply body is needed, it could be zero-sized.
 *
 * @param connection the connection to check
 * @param rcode the response code
 * @return enum value indicating whether response body can be used and
 *         whether response body length headers are allowed or required.
 * @sa is_reply_body_header_needed()
 */
static enum replyBodyUse
is_reply_body_needed (struct MHD_Connection *connection,
                      unsigned int rcode)
{
  struct MHD_Connection *const c = connection; /**< a short alias */

  mhd_assert (100 <= rcode);
  mhd_assert (999 >= rcode);

  if (199 >= rcode)
    return RP_BODY_NONE;

  if (MHD_HTTP_NO_CONTENT == rcode)
    return RP_BODY_NONE;

#if 0
  /* This check is not needed as upgrade handler is used only with code 101 */
#ifdef UPGRADE_SUPPORT
  if (NULL != rp.response->upgrade_handler)
    return RP_BODY_NONE;
#endif /* UPGRADE_SUPPORT */
#endif

#if 0
  /* CONNECT is not supported by MHD */
  /* Successful responses for connect requests are filtered by
   * MHD_queue_response() */
  if ( (MHD_HTTP_MTHD_CONNECT == c->rq.http_mthd) &&
       (2 == rcode / 100) )
    return false; /* Actually pass-through CONNECT is not supported by MHD */
#endif

  /* Reply body headers could be used.
   * Check whether reply body itself must be used. */

  if (MHD_HTTP_MTHD_HEAD == c->rq.http_mthd)
    return RP_BODY_HEADERS_ONLY;

  if (MHD_HTTP_NOT_MODIFIED == rcode)
    return RP_BODY_HEADERS_ONLY;

  /* Reply body must be sent. The body may have zero length, but body size
   * must be indicated by headers ('Content-Length:' or
   * 'Transfer-Encoding: chunked'). */
  return RP_BODY_SEND;
}


/**
 * Setup connection reply properties.
 *
 * Reply properties include presence of reply body, transfer-encoding
 * type and other.
 *
 * @param connection to connection to process
 */
static void
setup_reply_properties (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->rp.response;  /**< a short alias */
  enum replyBodyUse use_rp_body;
  bool use_chunked;

  mhd_assert (NULL != r);

  /* ** Adjust reply properties ** */

  c->keepalive = keepalive_possible (c);
  use_rp_body = is_reply_body_needed (c, c->rp.responseCode);
  c->rp.props.send_reply_body = (use_rp_body > RP_BODY_HEADERS_ONLY);
  c->rp.props.use_reply_body_headers = (use_rp_body >= RP_BODY_HEADERS_ONLY);

#ifdef UPGRADE_SUPPORT
  mhd_assert ( (NULL == r->upgrade_handler) ||
               (RP_BODY_NONE == use_rp_body) );
#endif /* UPGRADE_SUPPORT */

  if (c->rp.props.use_reply_body_headers)
  {
    if ((MHD_SIZE_UNKNOWN == r->total_size) ||
        (0 != (r->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED)))
    { /* Use chunked reply encoding if possible */

      /* Check whether chunked encoding is supported by the client */
      if (! MHD_IS_HTTP_VER_1_1_COMPAT (c->rq.http_ver))
        use_chunked = false;
      /* Check whether chunked encoding is allowed for the reply */
      else if (0 != (r->flags & (MHD_RF_HTTP_1_0_COMPATIBLE_STRICT
                                 | MHD_RF_HTTP_1_0_SERVER)))
        use_chunked = false;
      else
        /* If chunked encoding is supported and allowed, and response size
         * is unknown, use chunked even for non-Keep-Alive connections.
         * See https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.3
         * Also use chunked if it is enforced by application and supported by
         * the client. */
        use_chunked = true;
    }
    else
      use_chunked = false;

    if ( (MHD_SIZE_UNKNOWN == r->total_size) &&
         (! use_chunked) )
    {
      /* End of the stream is indicated by closure */
      c->keepalive = MHD_CONN_MUST_CLOSE;
    }
  }
  else
    use_chunked = false; /* chunked encoding cannot be used without body */

  c->rp.props.chunked = use_chunked;
#ifdef _DEBUG
  c->rp.props.set = true;
#endif /* _DEBUG */
}


/**
 * Check whether queued response is suitable for @a connection.
 * @param connection to connection to check
 */
static void
check_connection_reply (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->rp.response;  /**< a short alias */

  mhd_assert (c->rp.props.set);
#ifdef HAVE_MESSAGES
  if ( (! c->rp.props.use_reply_body_headers) &&
       (0 != r->total_size) )
  {
    MHD_DLOG (c->daemon,
              _ ("This reply with response code %u cannot use reply body. "
                 "Non-empty response body is ignored and not used.\n"),
              (unsigned) (c->rp.responseCode));
  }
  if ( (! c->rp.props.use_reply_body_headers) &&
       (0 != (r->flags_auto & MHD_RAF_HAS_CONTENT_LENGTH)) )
  {
    MHD_DLOG (c->daemon,
              _ ("This reply with response code %u cannot use reply body. "
                 "Application defined \"Content-Length\" header violates"
                 "HTTP specification.\n"),
              (unsigned) (c->rp.responseCode));
  }
#else
  (void) c; /* Mute compiler warning */
  (void) r; /* Mute compiler warning */
#endif
}


/**
 * Append data to the buffer if enough space is available,
 * update position.
 * @param[out] buf the buffer to append data to
 * @param[in,out] ppos the pointer to position in the @a buffer
 * @param buf_size the size of the @a buffer
 * @param append the data to append
 * @param append_size the size of the @a append
 * @return true if data has been added and position has been updated,
 *         false if not enough space is available
 */
static bool
buffer_append (char *buf,
               size_t *ppos,
               size_t buf_size,
               const char *append,
               size_t append_size)
{
  mhd_assert (NULL != buf); /* Mute static analyzer */
  if (buf_size < *ppos + append_size)
    return false;
  memcpy (buf + *ppos, append, append_size);
  *ppos += append_size;
  return true;
}


/**
 * Append static string to the buffer if enough space is available,
 * update position.
 * @param[out] buf the buffer to append data to
 * @param[in,out] ppos the pointer to position in the @a buffer
 * @param buf_size the size of the @a buffer
 * @param str the static string to append
 * @return true if data has been added and position has been updated,
 *         false if not enough space is available
 */
#define buffer_append_s(buf,ppos,buf_size,str) \
  buffer_append (buf,ppos,buf_size,str, MHD_STATICSTR_LEN_ (str))


/**
 * Add user-defined headers from response object to
 * the text buffer.
 *
 * @param buf the buffer to add headers to
 * @param ppos the pointer to the position in the @a buf
 * @param buf_size the size of the @a buf
 * @param response the response
 * @param filter_transf_enc skip "Transfer-Encoding" header if any
 * @param filter_content_len skip "Content-Length" header if any
 * @param add_close add "close" token to the
 *                  "Connection:" header (if any), ignored if no "Connection:"
 *                  header was added by user or if "close" token is already
 *                  present in "Connection:" header
 * @param add_keep_alive add "Keep-Alive" token to the
 *                       "Connection:" header (if any)
 * @return true if succeed,
 *         false if buffer is too small
 */
static bool
add_user_headers (char *buf,
                  size_t *ppos,
                  size_t buf_size,
                  struct MHD_Response *response,
                  bool filter_transf_enc,
                  bool filter_content_len,
                  bool add_close,
                  bool add_keep_alive)
{
  struct MHD_Response *const r = response; /**< a short alias */
  struct MHD_HTTP_Res_Header *hdr; /**< Iterates through User-specified headers */
  size_t el_size; /**< the size of current element to be added to the @a buf */

  mhd_assert (! add_close || ! add_keep_alive);

  if (0 == (r->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED))
    filter_transf_enc = false;   /* No such header */
  if (0 == (r->flags_auto & MHD_RAF_HAS_CONTENT_LENGTH))
    filter_content_len = false;  /* No such header */
  if (0 == (r->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
  {
    add_close = false;          /* No such header */
    add_keep_alive = false;     /* No such header */
  }
  else if (0 != (r->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE))
    add_close = false;          /* "close" token was already set */

  for (hdr = r->first_header; NULL != hdr; hdr = hdr->next)
  {
    size_t initial_pos = *ppos;
    if (MHD_HEADER_KIND != hdr->kind)
      continue;
    if (filter_transf_enc)
    { /* Need to filter-out "Transfer-Encoding" */
      if ((MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_TRANSFER_ENCODING) ==
           hdr->header_size) &&
          (MHD_str_equal_caseless_bin_n_ (MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                          hdr->header, hdr->header_size)) )
      {
        filter_transf_enc = false; /* There is the only one such header */
        continue; /* Skip "Transfer-Encoding" header */
      }
    }
    if (filter_content_len)
    { /* Need to filter-out "Content-Length" */
      if ((MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH) ==
           hdr->header_size) &&
          (MHD_str_equal_caseless_bin_n_ (MHD_HTTP_HEADER_CONTENT_LENGTH,
                                          hdr->header, hdr->header_size)) )
      {
        /* Reset filter flag if only one header is allowed */
        filter_transf_enc =
          (0 == (r->flags & MHD_RF_INSANITY_HEADER_CONTENT_LENGTH));
        continue; /* Skip "Content-Length" header */
      }
    }

    /* Add user header */
    el_size = hdr->header_size + 2 + hdr->value_size + 2;
    if (buf_size < *ppos + el_size)
      return false;
    memcpy (buf + *ppos, hdr->header, hdr->header_size);
    (*ppos) += hdr->header_size;
    buf[(*ppos)++] = ':';
    buf[(*ppos)++] = ' ';
    if (add_close || add_keep_alive)
    {
      /* "Connection:" header must be always the first one */
      mhd_assert (MHD_str_equal_caseless_n_ (hdr->header, \
                                             MHD_HTTP_HEADER_CONNECTION, \
                                             hdr->header_size));

      if (add_close)
      {
        el_size += MHD_STATICSTR_LEN_ ("close, ");
        if (buf_size < initial_pos + el_size)
          return false;
        memcpy (buf + *ppos, "close, ",
                MHD_STATICSTR_LEN_ ("close, "));
        *ppos += MHD_STATICSTR_LEN_ ("close, ");
      }
      else
      {
        el_size += MHD_STATICSTR_LEN_ ("Keep-Alive, ");
        if (buf_size < initial_pos + el_size)
          return false;
        memcpy (buf + *ppos, "Keep-Alive, ",
                MHD_STATICSTR_LEN_ ("Keep-Alive, "));
        *ppos += MHD_STATICSTR_LEN_ ("Keep-Alive, ");
      }
      add_close = false;
      add_keep_alive = false;
    }
    if (0 != hdr->value_size)
      memcpy (buf + *ppos, hdr->value, hdr->value_size);
    *ppos += hdr->value_size;
    buf[(*ppos)++] = '\r';
    buf[(*ppos)++] = '\n';
    mhd_assert (initial_pos + el_size == (*ppos));
  }
  return true;
}


/**
 * Allocate the connection's write buffer and fill it with all of the
 * headers from the response.
 * Required headers are added here.
 *
 * @param connection the connection
 * @return #MHD_YES on success, #MHD_NO on failure (out of memory)
 */
static enum MHD_Result
build_header_response (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->rp.response; /**< a short alias */
  char *buf;                                     /**< the output buffer */
  size_t pos;                                    /**< append offset in the @a buf */
  size_t buf_size;                               /**< the size of the @a buf */
  size_t el_size;                                /**< the size of current element to be added to the @a buf */
  unsigned rcode;                                /**< the response code */
  bool use_conn_close;                           /**< Use "Connection: close" header */
  bool use_conn_k_alive;                         /**< Use "Connection: Keep-Alive" header */

  mhd_assert (NULL != r);

  /* ** Adjust response properties ** */
  setup_reply_properties (c);

  mhd_assert (c->rp.props.set);
  mhd_assert ((MHD_CONN_MUST_CLOSE == c->keepalive) || \
              (MHD_CONN_USE_KEEPALIVE == c->keepalive) || \
              (MHD_CONN_MUST_UPGRADE == c->keepalive));
#ifdef UPGRADE_SUPPORT
  mhd_assert ((NULL == r->upgrade_handler) || \
              (MHD_CONN_MUST_UPGRADE == c->keepalive));
#else  /* ! UPGRADE_SUPPORT */
  mhd_assert (MHD_CONN_MUST_UPGRADE != c->keepalive);
#endif /* ! UPGRADE_SUPPORT */
  mhd_assert ((! c->rp.props.chunked) || c->rp.props.use_reply_body_headers);
  mhd_assert ((! c->rp.props.send_reply_body) || \
              c->rp.props.use_reply_body_headers);
#ifdef UPGRADE_SUPPORT
  mhd_assert (NULL == r->upgrade_handler || \
              ! c->rp.props.use_reply_body_headers);
#endif /* UPGRADE_SUPPORT */

  check_connection_reply (c);

  rcode = (unsigned) c->rp.responseCode;
  if (MHD_CONN_MUST_CLOSE == c->keepalive)
  {
    /* The closure of connection must be always indicated by header
     * to avoid hung connections */
    use_conn_close = true;
    use_conn_k_alive = false;
  }
  else if (MHD_CONN_USE_KEEPALIVE == c->keepalive)
  {
    use_conn_close = false;
    /* Add "Connection: keep-alive" if request is HTTP/1.0 or
     * if reply is HTTP/1.0
     * For HTTP/1.1 add header only if explicitly requested by app
     * (by response flag), as "Keep-Alive" is default for HTTP/1.1. */
    if ((0 != (r->flags & MHD_RF_SEND_KEEP_ALIVE_HEADER)) ||
        (MHD_HTTP_VER_1_0 == c->rq.http_ver) ||
        (0 != (r->flags & MHD_RF_HTTP_1_0_SERVER)))
      use_conn_k_alive = true;
    else
      use_conn_k_alive = false;
  }
  else
  {
    use_conn_close = false;
    use_conn_k_alive = false;
  }

  /* ** Actually build the response header ** */

  /* Get all space available */
  connection_maximize_write_buffer (c);
  buf = c->write_buffer;
  pos = c->write_buffer_append_offset;
  buf_size = c->write_buffer_size;
  if (0 == buf_size)
    return MHD_NO;
  mhd_assert (NULL != buf);

  /* * The status line * */

  /* The HTTP version */
  if (! c->rp.responseIcy)
  { /* HTTP reply */
    if (0 == (r->flags & MHD_RF_HTTP_1_0_SERVER))
    { /* HTTP/1.1 reply */
      /* Use HTTP/1.1 responses for HTTP/1.0 clients.
       * See https://datatracker.ietf.org/doc/html/rfc7230#section-2.6 */
      if (! buffer_append_s (buf, &pos, buf_size, MHD_HTTP_VERSION_1_1))
        return MHD_NO;
    }
    else
    { /* HTTP/1.0 reply */
      if (! buffer_append_s (buf, &pos, buf_size, MHD_HTTP_VERSION_1_0))
        return MHD_NO;
    }
  }
  else
  { /* ICY reply */
    if (! buffer_append_s (buf, &pos, buf_size, "ICY"))
      return MHD_NO;
  }

  /* The response code */
  if (buf_size < pos + 5) /* space + code + space */
    return MHD_NO;
  buf[pos++] = ' ';
  pos += MHD_uint16_to_str ((uint16_t) rcode, buf + pos,
                            buf_size - pos);
  buf[pos++] = ' ';

  /* The reason phrase */
  el_size = MHD_get_reason_phrase_len_for (rcode);
  if (0 == el_size)
  {
    if (! buffer_append_s (buf, &pos, buf_size, "Non-Standard Status"))
      return MHD_NO;
  }
  else if (! buffer_append (buf, &pos, buf_size,
                            MHD_get_reason_phrase_for (rcode),
                            el_size))
    return MHD_NO;

  /* The linefeed */
  if (buf_size < pos + 2)
    return MHD_NO;
  buf[pos++] = '\r';
  buf[pos++] = '\n';

  /* * The headers * */

  /* Main automatic headers */

  /* The "Date:" header */
  if ( (0 == (r->flags_auto & MHD_RAF_HAS_DATE_HDR)) &&
       (0 == (c->daemon->options & MHD_USE_SUPPRESS_DATE_NO_CLOCK)) )
  {
    /* Additional byte for unused zero-termination */
    if (buf_size < pos + 38)
      return MHD_NO;
    if (get_date_header (buf + pos))
      pos += 37;
  }
  /* The "Connection:" header */
  mhd_assert (! use_conn_close || ! use_conn_k_alive);
  mhd_assert (! use_conn_k_alive || ! use_conn_close);
  if (0 == (r->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
  {
    if (use_conn_close)
    {
      if (! buffer_append_s (buf, &pos, buf_size,
                             MHD_HTTP_HEADER_CONNECTION ": close\r\n"))
        return MHD_NO;
    }
    else if (use_conn_k_alive)
    {
      if (! buffer_append_s (buf, &pos, buf_size,
                             MHD_HTTP_HEADER_CONNECTION ": Keep-Alive\r\n"))
        return MHD_NO;
    }
  }

  /* User-defined headers */

  if (! add_user_headers (buf, &pos, buf_size, r,
                          ! c->rp.props.chunked,
                          (! c->rp.props.use_reply_body_headers) &&
                          (0 ==
                           (r->flags & MHD_RF_INSANITY_HEADER_CONTENT_LENGTH)),
                          use_conn_close,
                          use_conn_k_alive))
    return MHD_NO;

  /* Other automatic headers */

  if ( (c->rp.props.use_reply_body_headers) &&
       (0 == (r->flags & MHD_RF_HEAD_ONLY_RESPONSE)) )
  {
    /* Body-specific headers */

    if (c->rp.props.chunked)
    { /* Chunked encoding is used */
      if (0 == (r->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED))
      { /* No chunked encoding header set by user */
        if (! buffer_append_s (buf, &pos, buf_size,
                               MHD_HTTP_HEADER_TRANSFER_ENCODING ": " \
                               "chunked\r\n"))
          return MHD_NO;
      }
    }
    else /* Chunked encoding is not used */
    {
      if (MHD_SIZE_UNKNOWN != r->total_size)
      { /* The size is known */
        if (0 == (r->flags_auto & MHD_RAF_HAS_CONTENT_LENGTH))
        { /* The response does not have "Content-Length" header */
          if (! buffer_append_s (buf, &pos, buf_size,
                                 MHD_HTTP_HEADER_CONTENT_LENGTH ": "))
            return MHD_NO;
          el_size = MHD_uint64_to_str (r->total_size, buf + pos,
                                       buf_size - pos);
          if (0 == el_size)
            return MHD_NO;
          pos += el_size;

          if (buf_size < pos + 2)
            return MHD_NO;
          buf[pos++] = '\r';
          buf[pos++] = '\n';
        }
      }
    }
  }

  /* * Header termination * */
  if (buf_size < pos + 2)
    return MHD_NO;
  buf[pos++] = '\r';
  buf[pos++] = '\n';

  c->write_buffer_append_offset = pos;
  return MHD_YES;
}


/**
 * Allocate the connection's write buffer (if necessary) and fill it
 * with response footers.
 * Works only for chunked responses as other responses do not need
 * and do not support any kind of footers.
 *
 * @param connection the connection
 * @return #MHD_YES on success, #MHD_NO on failure (out of memory)
 */
static enum MHD_Result
build_connection_chunked_response_footer (struct MHD_Connection *connection)
{
  char *buf;           /**< the buffer to write footers to */
  size_t buf_size;     /**< the size of the @a buf */
  size_t used_size;    /**< the used size of the @a buf */
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_HTTP_Res_Header *pos;

  mhd_assert (connection->rp.props.chunked);
  /* TODO: allow combining of the final footer with the last chunk,
   * modify the next assert. */
  mhd_assert (MHD_CONNECTION_CHUNKED_BODY_SENT == connection->state);
  mhd_assert (NULL != c->rp.response);

  buf_size = connection_maximize_write_buffer (c);
  /* '5' is the minimal size of chunked footer ("0\r\n\r\n") */
  if (buf_size < 5)
    return MHD_NO;
  mhd_assert (NULL != c->write_buffer);
  buf = c->write_buffer + c->write_buffer_append_offset;
  mhd_assert (NULL != buf);
  used_size = 0;
  buf[used_size++] = '0';
  buf[used_size++] = '\r';
  buf[used_size++] = '\n';

  for (pos = c->rp.response->first_header; NULL != pos; pos = pos->next)
  {
    if (MHD_FOOTER_KIND == pos->kind)
    {
      size_t new_used_size; /* resulting size with this header */
      /* '4' is colon, space, linefeeds */
      new_used_size = used_size + pos->header_size + pos->value_size + 4;
      if (new_used_size > buf_size)
        return MHD_NO;
      memcpy (buf + used_size, pos->header, pos->header_size);
      used_size += pos->header_size;
      buf[used_size++] = ':';
      buf[used_size++] = ' ';
      memcpy (buf + used_size, pos->value, pos->value_size);
      used_size += pos->value_size;
      buf[used_size++] = '\r';
      buf[used_size++] = '\n';
      mhd_assert (used_size == new_used_size);
    }
  }
  if (used_size + 2 > buf_size)
    return MHD_NO;
  buf[used_size++] = '\r';
  buf[used_size++] = '\n';

  c->write_buffer_append_offset += used_size;
  mhd_assert (c->write_buffer_append_offset <= c->write_buffer_size);

  return MHD_YES;
}


/**
 * We encountered an error processing the request.
 * Handle it properly by stopping to read data
 * and sending the indicated response code and message.
 *
 * @param connection the connection
 * @param status_code the response code to send (400, 413 or 414)
 * @param message the error message to send
 * @param message_len the length of the @a message
 * @param header_name the name of the header, malloc()ed by the caller,
 *                    free() by this function, optional, can be NULL
 * @param header_name_len the length of the @a header_name
 * @param header_value the value of the header, malloc()ed by the caller,
 *                     free() by this function, optional, can be NULL
 * @param header_value_len the length of the @a header_value
 */
static void
transmit_error_response_len (struct MHD_Connection *connection,
                             unsigned int status_code,
                             const char *message,
                             size_t message_len,
                             char *header_name,
                             size_t header_name_len,
                             char *header_value,
                             size_t header_value_len)
{
  struct MHD_Response *response;
  enum MHD_Result iret;

  mhd_assert (! connection->stop_with_error); /* Do not send error twice */
  if (connection->stop_with_error)
  { /* Should not happen */
    if (MHD_CONNECTION_CLOSED > connection->state)
      connection->state = MHD_CONNECTION_CLOSED;
    free (header_name);
    free (header_value);
    return;
  }
  connection->stop_with_error = true;
  connection->discard_request = true;
#ifdef HAVE_MESSAGES
  MHD_DLOG (connection->daemon,
            _ ("Error processing request (HTTP response code is %u ('%s')). " \
               "Closing connection.\n"),
            status_code,
            message);
#endif
  if (MHD_CONNECTION_START_REPLY < connection->state)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Too late to send an error response, " \
                 "response is being sent already.\n"),
              status_code,
              message);
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Too late for error response."));
    free (header_name);
    free (header_value);
    return;
  }
  /* TODO: remove when special error queue function is implemented */
  connection->state = MHD_CONNECTION_FULL_REQ_RECEIVED;
  if (0 != connection->read_buffer_size)
  {
    /* Read buffer is not needed anymore, discard it
     * to free some space for error response. */
    MHD_pool_deallocate (connection->pool,
                         connection->read_buffer,
                         connection->read_buffer_size);
    connection->read_buffer = NULL;
    connection->read_buffer_size = 0;
    connection->read_buffer_offset = 0;
  }
  if (NULL != connection->rp.response)
  {
    MHD_destroy_response (connection->rp.response);
    connection->rp.response = NULL;
  }
  response = MHD_create_response_from_buffer_static (message_len,
                                                     message);
  if (NULL == response)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Failed to create error response.\n"),
              status_code,
              message);
#endif
    /* can't even send a reply, at least close the connection */
    connection->state = MHD_CONNECTION_CLOSED;
    free (header_name);
    free (header_value);
    return;
  }
  mhd_assert ((0 == header_name_len) || (NULL != header_name));
  mhd_assert ((NULL == header_name) || (0 != header_name_len));
  mhd_assert ((0 == header_value_len) || (NULL != header_value));
  mhd_assert ((NULL == header_value) || (0 != header_value_len));
  mhd_assert ((NULL == header_name) || (NULL != header_value));
  mhd_assert ((NULL != header_value) || (NULL == header_name));
  if (NULL != header_name)
  {
    iret = MHD_add_response_entry_no_alloc_ (response,
                                             MHD_HEADER_KIND,
                                             header_name, header_name_len,
                                             header_value, header_value_len);
    if (MHD_NO == iret)
    {
      free (header_name);
      free (header_value);
    }
  }
  else
    iret = MHD_YES;

  if (MHD_NO != iret)
  {
    bool before = connection->in_access_handler;

    /* Fake the flag for the internal call */
    connection->in_access_handler = true;
    iret = MHD_queue_response (connection,
                               status_code,
                               response);
    connection->in_access_handler = before;
  }
  MHD_destroy_response (response);
  if (MHD_NO == iret)
  {
    /* can't even send a reply, at least close the connection */
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Closing connection " \
                               "(failed to queue error response)."));
    return;
  }
  mhd_assert (NULL != connection->rp.response);
  /* Do not reuse this connection. */
  connection->keepalive = MHD_CONN_MUST_CLOSE;
  if (MHD_NO == build_header_response (connection))
  {
    /* No memory. Release everything. */
    connection->rq.version = NULL;
    connection->rq.method = NULL;
    connection->rq.url = NULL;
    connection->rq.url_len = 0;
    connection->rq.headers_received = NULL;
    connection->rq.headers_received_tail = NULL;
    connection->write_buffer = NULL;
    connection->write_buffer_size = 0;
    connection->write_buffer_send_offset = 0;
    connection->write_buffer_append_offset = 0;
    connection->read_buffer
      = MHD_pool_reset (connection->pool,
                        NULL,
                        0,
                        0);
    connection->read_buffer_size = 0;

    /* Retry with empty buffer */
    if (MHD_NO == build_header_response (connection))
    {
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection " \
                                 "(failed to create error response header)."));
      return;
    }
  }
  connection->state = MHD_CONNECTION_HEADERS_SENDING;
}


/**
 * Transmit static string as error response
 */
#define transmit_error_response_static(c, code, msg) \
  transmit_error_response_len (c, code, \
                               msg, MHD_STATICSTR_LEN_ (msg), \
                               NULL, 0, NULL, 0)

/**
 * Transmit static string as error response and add specified header
 */
#define transmit_error_response_header(c, code, m, hd_n, hd_n_l, hd_v, hd_v_l) \
  transmit_error_response_len (c, code, \
                               m, MHD_STATICSTR_LEN_ (m), \
                               hd_n, hd_n_l, \
                               hd_v, hd_v_l)


/**
 * Check whether the read buffer has any upload body data ready to
 * be processed.
 * Must be called only when connection is in MHD_CONNECTION_BODY_RECEIVING
 * state.
 *
 * @param c the connection to check
 * @return 'true' if upload body data is already in the read buffer,
 *         'false' if no upload data is received and not processed.
 */
static bool
has_unprocessed_upload_body_data_in_buffer (struct MHD_Connection *c)
{
  mhd_assert (MHD_CONNECTION_BODY_RECEIVING == c->state);
  if (! c->rq.have_chunked_upload)
    return 0 != c->read_buffer_offset;

  /* Chunked upload */
  mhd_assert (0 != c->rq.remaining_upload_size); /* Must not be possible in MHD_CONNECTION_BODY_RECEIVING state */
  if (c->rq.current_chunk_offset == c->rq.current_chunk_size)
  {
    /* 0 == c->rq.current_chunk_size: Waiting the chunk size (chunk header).
       0 != c->rq.current_chunk_size: Waiting for chunk-closing CRLF. */
    return false;
  }
  return 0 != c->read_buffer_offset; /* Chunk payload data in the read buffer */
}


/**
 * The stage of input data processing.
 * Used for out-of-memory (in the pool) handling.
 */
enum MHD_ProcRecvDataStage
{
  MHD_PROC_RECV_INIT,        /**< No data HTTP request data have been processed yet */
  MHD_PROC_RECV_METHOD,      /**< Processing/receiving the request HTTP method */
  MHD_PROC_RECV_URI,         /**< Processing/receiving the request URI */
  MHD_PROC_RECV_HTTPVER,     /**< Processing/receiving the request HTTP version string */
  MHD_PROC_RECV_HEADERS,     /**< Processing/receiving the request HTTP headers */
  MHD_PROC_RECV_COOKIE,      /**< Processing the received request cookie header */
  MHD_PROC_RECV_BODY_NORMAL, /**< Processing/receiving the request non-chunked body */
  MHD_PROC_RECV_BODY_CHUNKED,/**< Processing/receiving the request chunked body */
  MHD_PROC_RECV_FOOTERS      /**< Processing/receiving the request footers */
};


#ifndef MHD_MAX_REASONABLE_HEADERS_SIZE_
/**
 * A reasonable headers size (excluding request line) that should be sufficient
 * for most requests.
 * If incoming data buffer free space is not enough to process the complete
 * header (the request line and all headers) and the headers size is larger than
 * this size then the status code 431 "Request Header Fields Too Large" is
 * returned to the client.
 * The larger headers are processed by MHD if enough space is available.
 */
#  define MHD_MAX_REASONABLE_HEADERS_SIZE_ (6 * 1024)
#endif /* ! MHD_MAX_REASONABLE_HEADERS_SIZE_ */

#ifndef MHD_MAX_REASONABLE_REQ_TARGET_SIZE_
/**
 * A reasonable request target (the request URI) size that should be sufficient
 * for most requests.
 * If incoming data buffer free space is not enough to process the complete
 * header (the request line and all headers) and the request target size is
 * larger than this size then the status code 414 "URI Too Long" is
 * returned to the client.
 * The larger request targets are processed by MHD if enough space is available.
 * The value chosen according to RFC 9112 Section 3, paragraph 5
 */
#  define MHD_MAX_REASONABLE_REQ_TARGET_SIZE_ 8000
#endif /* ! MHD_MAX_REASONABLE_REQ_TARGET_SIZE_ */

#ifndef MHD_MIN_REASONABLE_HEADERS_SIZE_
/**
 * A reasonable headers size (excluding request line) that should be sufficient
 * for basic simple requests.
 * When no space left in the receiving buffer try to avoid replying with
 * the status code 431 "Request Header Fields Too Large" if headers size
 * is smaller then this value.
 */
#  define MHD_MIN_REASONABLE_HEADERS_SIZE_ 26
#endif /* ! MHD_MIN_REASONABLE_HEADERS_SIZE_ */

#ifndef MHD_MIN_REASONABLE_REQ_TARGET_SIZE_
/**
 * A reasonable request target (the request URI) size that should be sufficient
 * for basic simple requests.
 * When no space left in the receiving buffer try to avoid replying with
 * the status code 414 "URI Too Long" if the request target size is smaller then
 * this value.
 */
#  define MHD_MIN_REASONABLE_REQ_TARGET_SIZE_ 40
#endif /* ! MHD_MIN_REASONABLE_REQ_TARGET_SIZE_ */

#ifndef MHD_MIN_REASONABLE_REQ_METHOD_SIZE_
/**
 * A reasonable request method string size that should be sufficient
 * for basic simple requests.
 * When no space left in the receiving buffer try to avoid replying with
 * the status code 501 "Not Implemented" if the request method size is
 * smaller then this value.
 */
#  define MHD_MIN_REASONABLE_REQ_METHOD_SIZE_ 16
#endif /* ! MHD_MIN_REASONABLE_REQ_METHOD_SIZE_ */

#ifndef MHD_MIN_REASONABLE_REQ_CHUNK_LINE_LENGTH_
/**
 * A reasonable minimal chunk line length.
 * When no space left in the receiving buffer reply with 413 "Content Too Large"
 * if the chunk line length is larger than this value.
 */
#  define MHD_MIN_REASONABLE_REQ_CHUNK_LINE_LENGTH_ 4
#endif /* ! MHD_MIN_REASONABLE_REQ_CHUNK_LINE_LENGTH_ */


/**
 * Select the HTTP error status code for "out of receive buffer space" error.
 * @param c the connection to process
 * @param stage the current stage of request receiving
 * @param add_element the optional pointer to the element failed to be processed
 *                    or added, the meaning of the element depends on
 *                    the @a stage. Could be not zero-terminated and can
 *                    contain binary zeros. Can be NULL.
 * @param add_element_size the size of the @a add_element
 * @return the HTTP error code to use in the error reply
 */
static unsigned int
get_no_space_err_status_code (struct MHD_Connection *c,
                              enum MHD_ProcRecvDataStage stage,
                              const char *add_element,
                              size_t add_element_size)
{
  size_t method_size;
  size_t uri_size;
  size_t opt_headers_size;
  size_t host_field_line_size;

  mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVED < c->state);
  mhd_assert (MHD_PROC_RECV_HEADERS <= stage);
  mhd_assert ((0 == add_element_size) || (NULL != add_element));

  if (MHD_CONNECTION_HEADERS_RECEIVED > c->state)
  {
    mhd_assert (NULL != c->rq.field_lines.start);
    opt_headers_size =
      (size_t) ((c->read_buffer + c->read_buffer_offset)
                - c->rq.field_lines.start);
  }
  else
    opt_headers_size = c->rq.field_lines.size;

  /* The read buffer is fully used by the request line, the field lines
     (headers) and internal information.
     The return status code works as a suggestion for the client to reduce
     one of the request elements. */

  if ((MHD_PROC_RECV_BODY_CHUNKED == stage) &&
      (MHD_MIN_REASONABLE_REQ_CHUNK_LINE_LENGTH_ < add_element_size))
  {
    /* Request could be re-tried easily with smaller chunk sizes */
    return MHD_HTTP_CONTENT_TOO_LARGE;
  }

  host_field_line_size = 0;
  /* The "Host:" field line is mandatory.
     The total size of the field lines (headers) cannot be smaller than
     the size of the "Host:" field line. */
  if ((MHD_PROC_RECV_HEADERS == stage)
      && (0 != add_element_size))
  {
    static const size_t header_host_key_len =
      MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_HOST);
    const bool is_host_header =
      (header_host_key_len + 1 <= add_element_size)
      && ( (0 == add_element[header_host_key_len])
           || (':' == add_element[header_host_key_len]) )
      && MHD_str_equal_caseless_bin_n_ (MHD_HTTP_HEADER_HOST,
                                        add_element,
                                        header_host_key_len);
    if (is_host_header)
    {
      const bool is_parsed = ! (
        (MHD_CONNECTION_HEADERS_RECEIVED > c->state) &&
        (add_element_size == c->read_buffer_offset) &&
        (c->read_buffer == add_element) );
      size_t actual_element_size;

      mhd_assert (! is_parsed || (0 == add_element[header_host_key_len]));
      /* The actual size should be larger due to CRLF or LF chars,
         however the exact termination sequence is not known here and
         as perfect precision is not required, to simplify the code
         assume the minimal length. */
      if (is_parsed)
        actual_element_size = add_element_size + 1;  /* "1" for LF */
      else
        actual_element_size = add_element_size;

      host_field_line_size = actual_element_size;
      mhd_assert (opt_headers_size >= actual_element_size);
      opt_headers_size -= actual_element_size;
    }
  }
  if (0 == host_field_line_size)
  {
    static const size_t host_field_name_len =
      MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_HOST);
    size_t host_field_name_value_len;
    if (MHD_NO != MHD_lookup_connection_value_n (c,
                                                 MHD_HEADER_KIND,
                                                 MHD_HTTP_HEADER_HOST,
                                                 host_field_name_len,
                                                 NULL,
                                                 &host_field_name_value_len))
    {
      /* Calculate the minimal size of the field line: no space between
         colon and the field value, line terminated by LR */
      host_field_line_size =
        host_field_name_len + host_field_name_value_len + 2; /* "2" for ':' and LF */

      /* The "Host:" field could be added by application */
      if (opt_headers_size >= host_field_line_size)
      {
        opt_headers_size -= host_field_line_size;
        /* Take into account typical space after colon and CR at the end of the line */
        if (opt_headers_size >= 2)
          opt_headers_size -= 2;
      }
      else
        host_field_line_size = 0; /* No "Host:" field line set by the client */
    }
  }

  uri_size = c->rq.req_target_len;
  if (MHD_HTTP_MTHD_OTHER != c->rq.http_mthd)
    method_size = 0; /* Do not recommend shorter request method */
  else
  {
    mhd_assert (NULL != c->rq.method);
    method_size = strlen (c->rq.method);
  }

  if ((size_t) MHD_MAX_REASONABLE_HEADERS_SIZE_ < opt_headers_size)
  {
    /* Typically the easiest way to reduce request header size is
       a removal of some optional headers. */
    if (opt_headers_size > (uri_size / 8))
    {
      if ((opt_headers_size / 2) > method_size)
        return MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
      else
        return MHD_HTTP_NOT_IMPLEMENTED; /* The length of the HTTP request method is unreasonably large */
    }
    else
    { /* Request target is MUCH larger than headers */
      if ((uri_size / 16) > method_size)
        return MHD_HTTP_URI_TOO_LONG;
      else
        return MHD_HTTP_NOT_IMPLEMENTED; /* The length of the HTTP request method is unreasonably large */
    }
  }
  if ((size_t) MHD_MAX_REASONABLE_REQ_TARGET_SIZE_ < uri_size)
  {
    /* If request target size if larger than maximum reasonable size
       recommend client to reduce the request target size (length). */
    if ((uri_size / 16) > method_size)
      return MHD_HTTP_URI_TOO_LONG;     /* Request target is MUCH larger than headers */
    else
      return MHD_HTTP_NOT_IMPLEMENTED;  /* The length of the HTTP request method is unreasonably large */
  }

  /* The read buffer is too small to handle reasonably large requests */

  if ((size_t) MHD_MIN_REASONABLE_HEADERS_SIZE_ < opt_headers_size)
  {
    /* Recommend application to retry with minimal headers */
    if ((opt_headers_size * 4) > uri_size)
    {
      if (opt_headers_size > method_size)
        return MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
      else
        return MHD_HTTP_NOT_IMPLEMENTED; /* The length of the HTTP request method is unreasonably large */
    }
    else
    { /* Request target is significantly larger than headers */
      if (uri_size > method_size * 4)
        return MHD_HTTP_URI_TOO_LONG;
      else
        return MHD_HTTP_NOT_IMPLEMENTED; /* The length of the HTTP request method is unreasonably large */
    }
  }
  if ((size_t) MHD_MIN_REASONABLE_REQ_TARGET_SIZE_ < uri_size)
  {
    /* Recommend application to retry with a shorter request target */
    if (uri_size > method_size * 4)
      return MHD_HTTP_URI_TOO_LONG;
    else
      return MHD_HTTP_NOT_IMPLEMENTED; /* The length of the HTTP request method is unreasonably large */
  }

  if ((size_t) MHD_MIN_REASONABLE_REQ_METHOD_SIZE_ < method_size)
  {
    /* The request target (URI) and headers are (reasonably) very small.
       Some non-standard long request method is used. */
    /* The last resort response as it means "the method is not supported
       by the server for any URI". */
    return MHD_HTTP_NOT_IMPLEMENTED;
  }

  /* The almost impossible situation: all elements are small, but cannot
     fit the buffer. The application set the buffer size to
     critically low value? */

  if ((1 < opt_headers_size) || (1 < uri_size))
  {
    if (opt_headers_size >= uri_size)
      return MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
    else
      return MHD_HTTP_URI_TOO_LONG;
  }

  /* Nothing to reduce in the request.
     Reply with some status. */
  if (0 != host_field_line_size)
    return MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;

  return MHD_HTTP_URI_TOO_LONG;
}


/**
 * Send error reply when receive buffer space exhausted while receiving or
 * storing the request headers
 * @param c the connection to handle
 * @param add_header the optional pointer to the current header string being
 *                   processed or the header failed to be added.
 *                   Could be not zero-terminated and can contain binary zeros.
 *                   Can be NULL.
 * @param add_header_size the size of the @a add_header
 */
static void
handle_req_headers_no_space (struct MHD_Connection *c,
                             const char *add_header,
                             size_t add_header_size)
{
  unsigned int err_code;

  err_code = get_no_space_err_status_code (c,
                                           MHD_PROC_RECV_HEADERS,
                                           add_header,
                                           add_header_size);
  transmit_error_response_static (c,
                                  err_code,
                                  ERR_MSG_REQUEST_HEADER_TOO_BIG);
}


#ifdef COOKIE_SUPPORT
/**
 * Send error reply when the pool has no space to store 'cookie' header
 * parsing results.
 * @param c the connection to handle
 */
static void
handle_req_cookie_no_space (struct MHD_Connection *c)
{
  unsigned int err_code;

  err_code = get_no_space_err_status_code (c,
                                           MHD_PROC_RECV_COOKIE,
                                           NULL,
                                           0);
  transmit_error_response_static (c,
                                  err_code,
                                  ERR_MSG_REQUEST_HEADER_WITH_COOKIES_TOO_BIG);
}


#endif /* COOKIE_SUPPORT */


/**
 * Send error reply when receive buffer space exhausted while receiving
 * the chunk size line.
 * @param c the connection to handle
 * @param add_header the optional pointer to the partially received
 *                   the current chunk size line.
 *                   Could be not zero-terminated and can contain binary zeros.
 *                   Can be NULL.
 * @param add_header_size the size of the @a add_header
 */
static void
handle_req_chunk_size_line_no_space (struct MHD_Connection *c,
                                     const char *chunk_size_line,
                                     size_t chunk_size_line_size)
{
  unsigned int err_code;

  if (NULL != chunk_size_line)
  {
    const char *semicol;
    /* Check for chunk extension */
    semicol = memchr (chunk_size_line, ';', chunk_size_line_size);
    if (NULL != semicol)
    { /* Chunk extension present. It could be removed without any loss of the
         details of the request. */
      transmit_error_response_static (c,
                                      MHD_HTTP_CONTENT_TOO_LARGE,
                                      ERR_MSG_REQUEST_CHUNK_LINE_EXT_TOO_BIG);
    }
  }
  err_code = get_no_space_err_status_code (c,
                                           MHD_PROC_RECV_BODY_CHUNKED,
                                           chunk_size_line,
                                           chunk_size_line_size);
  transmit_error_response_static (c,
                                  err_code,
                                  ERR_MSG_REQUEST_CHUNK_LINE_TOO_BIG);
}


/**
 * Send error reply when receive buffer space exhausted while receiving or
 * storing the request footers (for chunked requests).
 * @param c the connection to handle
 * @param add_footer the optional pointer to the current footer string being
 *                   processed or the footer failed to be added.
 *                   Could be not zero-terminated and can contain binary zeros.
 *                   Can be NULL.
 * @param add_footer_size the size of the @a add_footer
 */
static void
handle_req_footers_no_space (struct MHD_Connection *c,
                             const char *add_footer,
                             size_t add_footer_size)
{
  (void) add_footer; (void) add_footer_size; /* Unused */
  mhd_assert (c->rq.have_chunked_upload);

  /* Footers should be optional */
  transmit_error_response_static (c,
                                  MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
                                  ERR_MSG_REQUEST_FOOTER_TOO_BIG);
}


/**
 * Handle situation with read buffer exhaustion.
 * Must be called when no more space left in the read buffer, no more
 * space left in the memory pool to grow the read buffer, but more data
 * need to be received from the client.
 * Could be called when the result of received data processing cannot be
 * stored in the memory pool (like some header).
 * @param c the connection to process
 * @param stage the receive stage where the exhaustion happens.
 */
static void
handle_recv_no_space (struct MHD_Connection *c,
                      enum MHD_ProcRecvDataStage stage)
{
  mhd_assert (MHD_PROC_RECV_INIT <= stage);
  mhd_assert (MHD_PROC_RECV_FOOTERS >= stage);
  mhd_assert (MHD_CONNECTION_FULL_REQ_RECEIVED > c->state);
  mhd_assert ((MHD_PROC_RECV_INIT != stage) || \
              (MHD_CONNECTION_INIT == c->state));
  mhd_assert ((MHD_PROC_RECV_METHOD != stage) || \
              (MHD_CONNECTION_REQ_LINE_RECEIVING == c->state));
  mhd_assert ((MHD_PROC_RECV_URI != stage) || \
              (MHD_CONNECTION_REQ_LINE_RECEIVING == c->state));
  mhd_assert ((MHD_PROC_RECV_HTTPVER != stage) || \
              (MHD_CONNECTION_REQ_LINE_RECEIVING == c->state));
  mhd_assert ((MHD_PROC_RECV_HEADERS != stage) || \
              (MHD_CONNECTION_REQ_HEADERS_RECEIVING == c->state));
  mhd_assert (MHD_PROC_RECV_COOKIE != stage); /* handle_req_cookie_no_space() must be called directly */
  mhd_assert ((MHD_PROC_RECV_BODY_NORMAL != stage) || \
              (MHD_CONNECTION_BODY_RECEIVING == c->state));
  mhd_assert ((MHD_PROC_RECV_BODY_CHUNKED != stage) || \
              (MHD_CONNECTION_BODY_RECEIVING == c->state));
  mhd_assert ((MHD_PROC_RECV_FOOTERS != stage) || \
              (MHD_CONNECTION_FOOTERS_RECEIVING == c->state));
  mhd_assert ((MHD_PROC_RECV_BODY_NORMAL != stage) || \
              (! c->rq.have_chunked_upload));
  mhd_assert ((MHD_PROC_RECV_BODY_CHUNKED != stage) || \
              (c->rq.have_chunked_upload));
  switch (stage)
  {
  case MHD_PROC_RECV_INIT:
  case MHD_PROC_RECV_METHOD:
    /* Some data has been received, but it is not clear yet whether
     * the received data is an valid HTTP request */
    connection_close_error (c,
                            _ ("No space left in the read buffer when " \
                               "receiving the initial part of " \
                               "the request line."));
    return;
  case MHD_PROC_RECV_URI:
  case MHD_PROC_RECV_HTTPVER:
    /* Some data has been received, but the request line is incomplete */
    mhd_assert (MHD_HTTP_MTHD_NO_METHOD != c->rq.http_mthd);
    mhd_assert (MHD_HTTP_VER_UNKNOWN == c->rq.http_ver);
    /* A quick simple check whether the incomplete line looks
     * like an HTTP request */
    if ((MHD_HTTP_MTHD_GET <= c->rq.http_mthd) &&
        (MHD_HTTP_MTHD_DELETE >= c->rq.http_mthd))
    {
      transmit_error_response_static (c,
                                      MHD_HTTP_URI_TOO_LONG,
                                      ERR_MSG_REQUEST_TOO_BIG);
      return;
    }
    connection_close_error (c,
                            _ ("No space left in the read buffer when " \
                               "receiving the URI in " \
                               "the request line. " \
                               "The request uses non-standard HTTP request " \
                               "method token."));
    return;
  case MHD_PROC_RECV_HEADERS:
    handle_req_headers_no_space (c, c->read_buffer, c->read_buffer_offset);
    return;
  case MHD_PROC_RECV_BODY_NORMAL:
  case MHD_PROC_RECV_BODY_CHUNKED:
    mhd_assert ((MHD_PROC_RECV_BODY_CHUNKED != stage) || \
                ! c->rq.some_payload_processed);
    if (has_unprocessed_upload_body_data_in_buffer (c))
    {
      /* The connection must not be in MHD_EVENT_LOOP_INFO_READ state
         when external polling is used and some data left unprocessed. */
      mhd_assert (MHD_D_IS_USING_THREADS_ (c->daemon));
      /* failed to grow the read buffer, and the
         client which is supposed to handle the
         received data in a *blocking* fashion
         (in this mode) did not handle the data as
         it was supposed to!
         => we would either have to do busy-waiting
         (on the client, which would likely fail),
         or if we do nothing, we would just timeout
         on the connection (if a timeout is even
         set!).
         Solution: we kill the connection with an error */
      transmit_error_response_static (c,
                                      MHD_HTTP_INTERNAL_SERVER_ERROR,
                                      ERROR_MSG_DATA_NOT_HANDLED_BY_APP);
    }
    else
    {
      if (MHD_PROC_RECV_BODY_NORMAL == stage)
      {
        /* A header probably has been added to a suspended connection and
           it took precisely all the space in the buffer.
           Very low probability. */
        mhd_assert (! c->rq.have_chunked_upload);
        handle_req_headers_no_space (c, NULL, 0);
      }
      else
      {
        mhd_assert (c->rq.have_chunked_upload);
        if (c->rq.current_chunk_offset != c->rq.current_chunk_size)
        { /* Receiving content of the chunk */
          /* A header probably has been added to a suspended connection and
             it took precisely all the space in the buffer.
             Very low probability. */
          handle_req_headers_no_space (c, NULL, 0);
        }
        else
        {
          if (0 != c->rq.current_chunk_size)
          { /* Waiting for chunk-closing CRLF */
            /* Not really possible as some payload should be
               processed and the space used by payload should be available. */
            handle_req_headers_no_space (c, NULL, 0);
          }
          else
          { /* Reading the line with the chunk size */
            handle_req_chunk_size_line_no_space (c,
                                                 c->read_buffer,
                                                 c->read_buffer_offset);
          }
        }
      }
    }
    return;
  case MHD_PROC_RECV_FOOTERS:
    handle_req_footers_no_space (c, c->read_buffer, c->read_buffer_offset);
    return;
  /* The next cases should not be possible */
  case MHD_PROC_RECV_COOKIE:
  default:
    break;
  }
  mhd_assert (0);
}


/**
 * Check whether enough space is available in the read buffer for the next
 * operation.
 * Handles grow of the buffer if required and error conditions (when buffer
 * grow is required but not possible).
 * Must be called only when processing the event loop states and when
 * reading is required for the next phase.
 * @param c the connection to check
 * @return true if connection handled successfully and enough buffer
 *         is available,
 *         false if not enough buffer is available and the loop's states
 *         must be processed again as connection is in the error state.
 */
static bool
check_and_grow_read_buffer_space (struct MHD_Connection *c)
{
  /**
   * The increase of read buffer size is desirable.
   */
  bool rbuff_grow_desired;
  /**
   * The increase of read buffer size is a hard requirement.
   */
  bool rbuff_grow_required;

  mhd_assert (0 != (MHD_EVENT_LOOP_INFO_READ & c->event_loop_info));
  mhd_assert (! c->discard_request);

  rbuff_grow_required = (c->read_buffer_offset == c->read_buffer_size);
  if (rbuff_grow_required)
    rbuff_grow_desired = true;
  else
  {
    rbuff_grow_desired = (c->read_buffer_offset + c->daemon->pool_increment >
                          c->read_buffer_size);

    if ((rbuff_grow_desired) &&
        (MHD_CONNECTION_BODY_RECEIVING == c->state))
    {
      if (! c->rq.have_chunked_upload)
      {
        mhd_assert (MHD_SIZE_UNKNOWN != c->rq.remaining_upload_size);
        /* Do not grow read buffer more than necessary to process the current
           request. */
        rbuff_grow_desired =
          (c->rq.remaining_upload_size > c->read_buffer_size);
      }
      else
      {
        mhd_assert (MHD_SIZE_UNKNOWN == c->rq.remaining_upload_size);
        if (0 == c->rq.current_chunk_size)
          rbuff_grow_desired =  /* Reading value of the next chunk size */
                               (MHD_CHUNK_HEADER_REASONABLE_LEN >
                                c->read_buffer_size);
        else
        {
          const uint64_t cur_chunk_left =
            c->rq.current_chunk_size - c->rq.current_chunk_offset;
          /* Do not grow read buffer more than necessary to process the current
             chunk with terminating CRLF. */
          mhd_assert (c->rq.current_chunk_offset <= c->rq.current_chunk_size);
          rbuff_grow_desired =
            ((cur_chunk_left + 2) > (uint64_t) (c->read_buffer_size));
        }
      }
    }
  }

  if (! rbuff_grow_desired)
    return true; /* No need to increase the buffer */

  if (try_grow_read_buffer (c, rbuff_grow_required))
    return true; /* Buffer increase succeed */

  if (! rbuff_grow_required)
    return true; /* Can continue without buffer increase */

  /* Failed to increase the read buffer size, but need to read the data
     from the network.
     No more space left in the buffer, no more space to increase the buffer. */

  /* 'PROCESS_READ' event state flag must be set only if the last application
     callback has processed some data. If any data is processed then some
     space in the read buffer must be available. */
  mhd_assert (0 == (MHD_EVENT_LOOP_INFO_PROCESS & c->event_loop_info));

  if ((! MHD_D_IS_USING_THREADS_ (c->daemon))
      && (MHD_CONNECTION_BODY_RECEIVING == c->state)
      && has_unprocessed_upload_body_data_in_buffer (c))
  {
    /* The application is handling processing cycles.
       The data could be processed later. */
    c->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS;
    return true;
  }
  else
  {
    enum MHD_ProcRecvDataStage stage;

    switch (c->state)
    {
    case MHD_CONNECTION_INIT:
      stage = MHD_PROC_RECV_INIT;
      break;
    case MHD_CONNECTION_REQ_LINE_RECEIVING:
      if (MHD_HTTP_MTHD_NO_METHOD == c->rq.http_mthd)
        stage = MHD_PROC_RECV_METHOD;
      else if (0 == c->rq.req_target_len)
        stage = MHD_PROC_RECV_URI;
      else
        stage = MHD_PROC_RECV_HTTPVER;
      break;
    case MHD_CONNECTION_REQ_HEADERS_RECEIVING:
      stage = MHD_PROC_RECV_HEADERS;
      break;
    case MHD_CONNECTION_BODY_RECEIVING:
      stage = c->rq.have_chunked_upload ?
              MHD_PROC_RECV_BODY_CHUNKED : MHD_PROC_RECV_BODY_NORMAL;
      break;
    case MHD_CONNECTION_FOOTERS_RECEIVING:
      stage = MHD_PROC_RECV_FOOTERS;
      break;
    case MHD_CONNECTION_REQ_LINE_RECEIVED:
    case MHD_CONNECTION_HEADERS_RECEIVED:
    case MHD_CONNECTION_HEADERS_PROCESSED:
    case MHD_CONNECTION_CONTINUE_SENDING:
    case MHD_CONNECTION_BODY_RECEIVED:
    case MHD_CONNECTION_FOOTERS_RECEIVED:
    case MHD_CONNECTION_FULL_REQ_RECEIVED:
    case MHD_CONNECTION_START_REPLY:
    case MHD_CONNECTION_HEADERS_SENDING:
    case MHD_CONNECTION_HEADERS_SENT:
    case MHD_CONNECTION_NORMAL_BODY_UNREADY:
    case MHD_CONNECTION_NORMAL_BODY_READY:
    case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
    case MHD_CONNECTION_CHUNKED_BODY_READY:
    case MHD_CONNECTION_CHUNKED_BODY_SENT:
    case MHD_CONNECTION_FOOTERS_SENDING:
    case MHD_CONNECTION_FULL_REPLY_SENT:
    case MHD_CONNECTION_CLOSED:
#ifdef UPGRADE_SUPPORT
    case MHD_CONNECTION_UPGRADE:
#endif
    default:
      stage = MHD_PROC_RECV_BODY_NORMAL;
      mhd_assert (0);
    }

    handle_recv_no_space (c, stage);
  }
  return false;
}


/**
 * Update the 'event_loop_info' field of this connection based on the state
 * that the connection is now in.  May also close the connection or
 * perform other updates to the connection if needed to prepare for
 * the next round of the event loop.
 *
 * @param connection connection to get poll set for
 */
static void
MHD_connection_update_event_loop_info (struct MHD_Connection *connection)
{
  /* Do not update states of suspended connection */
  if (connection->suspended)
    return; /* States will be updated after resume. */
#ifdef HTTPS_SUPPORT
  if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
  {   /* HTTPS connection. */
    switch (connection->tls_state)
    {
    case MHD_TLS_CONN_INIT:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      return;
    case MHD_TLS_CONN_HANDSHAKING:
    case MHD_TLS_CONN_WR_CLOSING:
      if (0 == gnutls_record_get_direction (connection->tls_session))
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      else
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      return;
    case MHD_TLS_CONN_CONNECTED:
      break; /* Do normal processing */
    case MHD_TLS_CONN_WR_CLOSED:
    case MHD_TLS_CONN_TLS_FAILED:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_CLEANUP;
      return;
    case MHD_TLS_CONN_TLS_CLOSING:  /* Not implemented yet */
    case MHD_TLS_CONN_TLS_CLOSED:   /* Not implemented yet */
    case MHD_TLS_CONN_INVALID_STATE:
    case MHD_TLS_CONN_NO_TLS: /* Not possible */
    default:
      MHD_PANIC (_ ("Invalid TLS state value.\n"));
    }
  }
#endif /* HTTPS_SUPPORT */
  while (1)
  {
#if DEBUG_STATES
    MHD_DLOG (connection->daemon,
              _ ("In function %s handling connection at state: %s\n"),
              MHD_FUNC_,
              MHD_state_to_string (connection->state));
#endif
    switch (connection->state)
    {
    case MHD_CONNECTION_INIT:
    case MHD_CONNECTION_REQ_LINE_RECEIVING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      break;
    case MHD_CONNECTION_REQ_LINE_RECEIVED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_REQ_HEADERS_RECEIVING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      break;
    case MHD_CONNECTION_HEADERS_RECEIVED:
    case MHD_CONNECTION_HEADERS_PROCESSED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_CONTINUE_SENDING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_BODY_RECEIVING:
      if ((connection->rq.some_payload_processed) &&
          has_unprocessed_upload_body_data_in_buffer (connection))
      {
        /* Some data was processed, the buffer must have some free space */
        mhd_assert (connection->read_buffer_offset < \
                    connection->read_buffer_size);
        if (! connection->rq.have_chunked_upload)
        {
          /* Not a chunked upload. Do not read more than necessary to
             process the current request. */
          if (connection->rq.remaining_upload_size >=
              connection->read_buffer_offset)
            connection->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS;
          else
            connection->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS_READ;
        }
        else
        {
          /* Chunked upload. The size of the current request is unknown.
             Continue reading as the space in the read buffer is available. */
          connection->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS_READ;
        }
      }
      else
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      break;
    case MHD_CONNECTION_BODY_RECEIVED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_FOOTERS_RECEIVING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      break;
    case MHD_CONNECTION_FOOTERS_RECEIVED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_FULL_REQ_RECEIVED:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS;
      break;
    case MHD_CONNECTION_START_REPLY:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_HEADERS_SENDING:
      /* headers in buffer, keep writing */
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_HEADERS_SENT:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_NORMAL_BODY_UNREADY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS;
      break;
    case MHD_CONNECTION_NORMAL_BODY_READY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_PROCESS;
      break;
    case MHD_CONNECTION_CHUNKED_BODY_READY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_CHUNKED_BODY_SENT:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_FOOTERS_SENDING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_FULL_REPLY_SENT:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_CLOSED:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_CLEANUP;
      return;           /* do nothing, not even reading */
#ifdef UPGRADE_SUPPORT
    case MHD_CONNECTION_UPGRADE:
      mhd_assert (0);
      break;
#endif /* UPGRADE_SUPPORT */
    default:
      mhd_assert (0);
    }

    if (0 != (MHD_EVENT_LOOP_INFO_READ & connection->event_loop_info))
    {
      /* Check whether the space is available to receive data */
      if (! check_and_grow_read_buffer_space (connection))
      {
        mhd_assert (connection->discard_request);
        continue;
      }
    }
    break; /* Everything was processed. */
  }
}


/**
 * Add an entry to the HTTP headers of a connection.  If this fails,
 * transmit an error response (request too big).
 *
 * @param cls the context (connection)
 * @param kind kind of the value
 * @param key key for the value
 * @param key_size number of bytes in @a key
 * @param value the value itself
 * @param value_size number of bytes in @a value
 * @return #MHD_NO on failure (out of memory), #MHD_YES for success
 */
static enum MHD_Result
connection_add_header (void *cls,
                       const char *key,
                       size_t key_size,
                       const char *value,
                       size_t value_size,
                       enum MHD_ValueKind kind)
{
  struct MHD_Connection *connection = (struct MHD_Connection *) cls;
  if (MHD_NO ==
      MHD_set_connection_value_n (connection,
                                  kind,
                                  key,
                                  key_size,
                                  value,
                                  value_size))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Not enough memory in pool to allocate header record!\n"));
#endif
    transmit_error_response_static (connection,
                                    MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
                                    ERR_MSG_REQUEST_TOO_BIG);
    return MHD_NO;
  }
  return MHD_YES;
}


#ifdef COOKIE_SUPPORT

/**
 * Cookie parsing result
 */
enum _MHD_ParseCookie
{
  MHD_PARSE_COOKIE_OK = MHD_YES,      /**< Success or no cookies in headers */
  MHD_PARSE_COOKIE_OK_LAX = 2,        /**< Cookies parsed, but workarounds used */
  MHD_PARSE_COOKIE_MALFORMED = -1,    /**< Invalid cookie header */
  MHD_PARSE_COOKIE_NO_MEMORY = MHD_NO /**< Not enough memory in the pool */
};


/**
 * Parse the cookies string (see RFC 6265).
 *
 * Try to parse the cookies string even if it is not strictly formed
 * as specified by RFC 6265.
 *
 * @param str the string to parse, without leading whitespaces
 * @param str_len the size of the @a str, not including mandatory
 *                zero-termination
 * @param connection the connection to add parsed cookies
 * @return #MHD_PARSE_COOKIE_OK for success, error code otherwise
 */
static enum _MHD_ParseCookie
parse_cookies_string (char *str,
                      const size_t str_len,
                      struct MHD_Connection *connection)
{
  size_t i;
  bool non_strict;
  /* Skip extra whitespaces and empty cookies */
  const bool allow_wsp_empty = (0 >= connection->daemon->client_discipline);
  /* Allow whitespaces around '=' character */
  const bool wsp_around_eq = (-3 >= connection->daemon->client_discipline);
  /* Allow whitespaces in quoted cookie value */
  const bool wsp_in_quoted = (-2 >= connection->daemon->client_discipline);
  /* Allow tab as space after semicolon between cookies */
  const bool tab_as_sp = (0 >= connection->daemon->client_discipline);
  /* Allow no space after semicolon between cookies */
  const bool allow_no_space = (0 >= connection->daemon->client_discipline);

  non_strict = false;
  i = 0;
  while (i < str_len)
  {
    size_t name_start;
    size_t name_len;
    size_t value_start;
    size_t value_len;
    bool val_quoted;
    /* Skip any whitespaces and empty cookies */
    while (' ' == str[i] || '\t' == str[i] || ';' == str[i])
    {
      if (! allow_wsp_empty)
        return MHD_PARSE_COOKIE_MALFORMED;
      non_strict = true;
      i++;
      if (i == str_len)
        return non_strict? MHD_PARSE_COOKIE_OK_LAX : MHD_PARSE_COOKIE_OK;
    }
    /* 'i' must point to the first char of cookie-name */
    name_start = i;
    /* Find the end of the cookie-name */
    do
    {
      const char l = str[i];
      if (('=' == l) || (' ' == l) || ('\t' == l) || ('"' == l) || (',' == l) ||
          (';' == l) || (0 == l))
        break;
    } while (str_len > ++i);
    name_len = i - name_start;
    /* Skip any whitespaces */
    while (str_len > i && (' ' == str[i] || '\t' == str[i]))
    {
      if (! wsp_around_eq)
        return MHD_PARSE_COOKIE_MALFORMED;
      non_strict = true;
      i++;
    }
    if ((str_len == i) || ('=' != str[i]) || (0 == name_len))
      return MHD_PARSE_COOKIE_MALFORMED; /* Incomplete cookie name */
    /* 'i' must point to the '=' char */
    mhd_assert ('=' == str[i]);
    i++;
    /* Skip any whitespaces */
    while (str_len > i && (' ' == str[i] || '\t' == str[i]))
    {
      if (! wsp_around_eq)
        return MHD_PARSE_COOKIE_MALFORMED;
      non_strict = true;
      i++;
    }
    /* 'i' must point to the first char of cookie-value */
    if (str_len == i)
    {
      value_start = 0;
      value_len = 0;
#ifdef _DEBUG
      val_quoted = false; /* This assignment used in assert */
#endif
    }
    else
    {
      bool valid_cookie;
      val_quoted = ('"' == str[i]);
      if (val_quoted)
        i++;
      value_start = i;
      /* Find the end of the cookie-value */
      while (str_len > i)
      {
        const char l = str[i];
        if ((';' == l) || ('"' == l) || (',' == l) || (';' == l) ||
            ('\\' == l) || (0 == l))
          break;
        if ((' ' == l) || ('\t' == l))
        {
          if (! val_quoted)
            break;
          if (! wsp_in_quoted)
            return MHD_PARSE_COOKIE_MALFORMED;
          non_strict = true;
        }
        i++;
      }
      value_len = i - value_start;
      if (val_quoted)
      {
        if ((str_len == i) || ('"' != str[i]))
          return MHD_PARSE_COOKIE_MALFORMED; /* Incomplete cookie value, no closing quote */
        i++;
      }
      /* Skip any whitespaces */
      if ((str_len > i) && ((' ' == str[i]) || ('\t' == str[i])))
      {
        do
        {
          i++;
        } while (str_len > i && (' ' == str[i] || '\t' == str[i]));
        /* Whitespace at the end? */
        if (str_len > i)
        {
          if (! allow_wsp_empty)
            return MHD_PARSE_COOKIE_MALFORMED;
          non_strict = true;
        }
      }
      if (str_len == i)
        valid_cookie = true;
      else if (';' == str[i])
        valid_cookie = true;
      else
        valid_cookie = false;

      if (! valid_cookie)
        return MHD_PARSE_COOKIE_MALFORMED; /* Garbage at the end of the cookie value */
    }
    mhd_assert (0 != name_len);
    str[name_start + name_len] = 0; /* Zero-terminate the name */
    if (0 != value_len)
    {
      mhd_assert (value_start + value_len <= str_len);
      str[value_start + value_len] = 0; /* Zero-terminate the value */
      if (MHD_NO ==
          MHD_set_connection_value_n_nocheck_ (connection,
                                               MHD_COOKIE_KIND,
                                               str + name_start,
                                               name_len,
                                               str + value_start,
                                               value_len))
        return MHD_PARSE_COOKIE_NO_MEMORY;
    }
    else
    {
      if (MHD_NO ==
          MHD_set_connection_value_n_nocheck_ (connection,
                                               MHD_COOKIE_KIND,
                                               str + name_start,
                                               name_len,
                                               "",
                                               0))
        return MHD_PARSE_COOKIE_NO_MEMORY;
    }
    if (str_len > i)
    {
      mhd_assert (0 == str[i] || ';' == str[i]);
      mhd_assert (! val_quoted || ';' == str[i]);
      mhd_assert (';' != str[i] || val_quoted || non_strict || 0 == value_len);
      i++;
      if (str_len == i)
      { /* No next cookie after semicolon */
        if (! allow_wsp_empty)
          return MHD_PARSE_COOKIE_MALFORMED;
        non_strict = true;
      }
      else if (' ' != str[i])
      {/* No space after semicolon */
        if (('\t' == str[i]) && tab_as_sp)
          i++;
        else if (! allow_no_space)
          return MHD_PARSE_COOKIE_MALFORMED;
        non_strict = true;
      }
      else
      {
        i++;
        if (str_len == i)
        {
          if (! allow_wsp_empty)
            return MHD_PARSE_COOKIE_MALFORMED;
          non_strict = true;
        }
      }
    }
  }
  return non_strict? MHD_PARSE_COOKIE_OK_LAX : MHD_PARSE_COOKIE_OK;
}


/**
 * Parse the cookie header (see RFC 6265).
 *
 * @param connection connection to parse header of
 * @return #MHD_PARSE_COOKIE_OK for success, error code otherwise
 */
static enum _MHD_ParseCookie
parse_cookie_header (struct MHD_Connection *connection)
{
  const char *hdr;
  size_t hdr_len;
  char *cpy;
  size_t i;
  enum _MHD_ParseCookie parse_res;
  struct MHD_HTTP_Req_Header *const saved_tail =
    connection->rq.headers_received_tail;
  const bool allow_partially_correct_cookie =
    (1 >= connection->daemon->client_discipline);

  if (MHD_NO ==
      MHD_lookup_connection_value_n (connection,
                                     MHD_HEADER_KIND,
                                     MHD_HTTP_HEADER_COOKIE,
                                     MHD_STATICSTR_LEN_ (
                                       MHD_HTTP_HEADER_COOKIE),
                                     &hdr,
                                     &hdr_len))
    return MHD_PARSE_COOKIE_OK;
  if (0 == hdr_len)
    return MHD_PARSE_COOKIE_OK;

  cpy = MHD_connection_alloc_memory_ (connection,
                                      hdr_len + 1);
  if (NULL == cpy)
    parse_res = MHD_PARSE_COOKIE_NO_MEMORY;
  else
  {
    memcpy (cpy,
            hdr,
            hdr_len);
    cpy[hdr_len] = '\0';

    i = 0;
    /* Skip all initial whitespaces */
    while (i < hdr_len && (' ' == cpy[i] || '\t' == cpy[i]))
      i++;

    parse_res = parse_cookies_string (cpy + i, hdr_len - i, connection);
  }

  switch (parse_res)
  {
  case MHD_PARSE_COOKIE_OK:
    break;
  case MHD_PARSE_COOKIE_OK_LAX:
#ifdef HAVE_MESSAGES
    if (saved_tail != connection->rq.headers_received_tail)
      MHD_DLOG (connection->daemon,
                _ ("The Cookie header has been parsed, but it is not fully "
                   "compliant with the standard.\n"));
#endif /* HAVE_MESSAGES */
    break;
  case MHD_PARSE_COOKIE_MALFORMED:
    if (saved_tail != connection->rq.headers_received_tail)
    {
      if (! allow_partially_correct_cookie)
      {
        /* Remove extracted values from partially broken cookie */
        /* Memory remains allocated until the end of the request processing */
        connection->rq.headers_received_tail = saved_tail;
        saved_tail->next = NULL;
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("The Cookie header has been ignored as it contains "
                     "malformed data.\n"));
#endif /* HAVE_MESSAGES */
      }
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (connection->daemon,
                  _ ("The Cookie header has been only partially parsed as it "
                     "contains malformed data.\n"));
#endif /* HAVE_MESSAGES */
    }
#ifdef HAVE_MESSAGES
    else
      MHD_DLOG (connection->daemon,
                _ ("The Cookie header has malformed data.\n"));
#endif /* HAVE_MESSAGES */
    break;
  case MHD_PARSE_COOKIE_NO_MEMORY:
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Not enough memory in the connection pool to "
                 "parse client cookies!\n"));
#endif /* HAVE_MESSAGES */
    break;
  default:
    mhd_assert (0);
    break;
  }
#ifndef HAVE_MESSAGES
  (void) saved_tail; /* Mute compiler warning */
#endif /* ! HAVE_MESSAGES */

  return parse_res;
}


#endif /* COOKIE_SUPPORT */


/**
 * The valid length of any HTTP version string
 */
#define HTTP_VER_LEN (MHD_STATICSTR_LEN_ (MHD_HTTP_VERSION_1_1))

/**
 * Detect HTTP version, send error response if version is not supported
 *
 * @param connection the connection
 * @param http_string the pointer to HTTP version string
 * @param len the length of @a http_string in bytes
 * @return true if HTTP version is correct and supported,
 *         false if HTTP version is not correct or unsupported.
 */
static bool
parse_http_version (struct MHD_Connection *connection,
                    const char *http_string,
                    size_t len)
{
  const char *const h = http_string; /**< short alias */
  mhd_assert (NULL != http_string);

  /* String must start with 'HTTP/d.d', case-sensetive match.
   * See https://www.rfc-editor.org/rfc/rfc9112#name-http-version */
  if ((HTTP_VER_LEN != len) ||
      ('H' != h[0]) || ('T' != h[1]) || ('T' != h[2]) || ('P' != h[3]) ||
      ('/' != h[4])
      || ('.' != h[6]) ||
      (('0' > h[5]) || ('9' < h[5])) ||
      (('0' > h[7]) || ('9' < h[7])))
  {
    connection->rq.http_ver = MHD_HTTP_VER_INVALID;
    transmit_error_response_static (connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    REQUEST_MALFORMED);
    return false;
  }
  if (1 == h[5] - '0')
  {
    /* HTTP/1.x */
    if (1 == h[7] - '0')
      connection->rq.http_ver = MHD_HTTP_VER_1_1;
    else if (0 == h[7] - '0')
      connection->rq.http_ver = MHD_HTTP_VER_1_0;
    else
      connection->rq.http_ver = MHD_HTTP_VER_1_2__1_9;

    return true;
  }

  if (0 == h[5] - '0')
  {
    /* Too old major version */
    connection->rq.http_ver = MHD_HTTP_VER_TOO_OLD;
    transmit_error_response_static (connection,
                                    MHD_HTTP_HTTP_VERSION_NOT_SUPPORTED,
                                    REQ_HTTP_VER_IS_TOO_OLD);
    return false;
  }

  connection->rq.http_ver = MHD_HTTP_VER_FUTURE;
  transmit_error_response_static (connection,
                                  MHD_HTTP_HTTP_VERSION_NOT_SUPPORTED,
                                  REQ_HTTP_VER_IS_NOT_SUPPORTED);
  return false;
}


/**
 * Detect standard HTTP request method
 *
 * @param connection the connection
 * @param method the pointer to HTTP request method string
 * @param len the length of @a method in bytes
 */
static void
parse_http_std_method (struct MHD_Connection *connection,
                       const char *method,
                       size_t len)
{
  const char *const m = method; /**< short alias */
  mhd_assert (NULL != m);
  mhd_assert (0 != len);

  if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_GET) == len) &&
      (0 == memcmp (m, MHD_HTTP_METHOD_GET, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_GET;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_HEAD) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_HEAD, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_HEAD;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_POST) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_POST, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_POST;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_PUT) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_PUT, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_PUT;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_DELETE) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_DELETE, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_DELETE;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_CONNECT) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_CONNECT, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_CONNECT;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_OPTIONS) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_OPTIONS, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_OPTIONS;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_TRACE) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_TRACE, len)))
    connection->rq.http_mthd = MHD_HTTP_MTHD_TRACE;
  else
    connection->rq.http_mthd = MHD_HTTP_MTHD_OTHER;
}


/**
 * Call the handler of the application for this
 * connection.  Handles chunking of the upload
 * as well as normal uploads.
 *
 * @param connection connection we're processing
 */
static void
call_connection_handler (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  size_t processed;

  if (NULL != connection->rp.response)
    return;                     /* already queued a response */
  processed = 0;
  connection->rq.client_aware = true;
  connection->in_access_handler = true;
  if (MHD_NO ==
      daemon->default_handler (daemon->default_handler_cls,
                               connection,
                               connection->rq.url,
                               connection->rq.method,
                               connection->rq.version,
                               NULL,
                               &processed,
                               &connection->rq.client_context))
  {
    connection->in_access_handler = false;
    /* serious internal error, close connection */
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Application reported internal error, " \
                               "closing connection."));
    return;
  }
  connection->in_access_handler = false;
}


/**
 * Call the handler of the application for this
 * connection.  Handles chunking of the upload
 * as well as normal uploads.
 *
 * @param connection connection we're processing
 */
static void
process_request_body (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  size_t available;
  bool instant_retry;
  char *buffer_head;
  const int discp_lvl = daemon->client_discipline;
  /* Treat bare LF as the end of the line.
     RFC 9112, section 2.2-3
     Note: MHD never replaces bare LF with space (RFC 9110, section 5.5-5).
     Bare LF is processed as end of the line or rejected as broken request. */
  const bool bare_lf_as_crlf = MHD_ALLOW_BARE_LF_AS_CRLF_ (discp_lvl);
  /* Allow "Bad WhiteSpace" in chunk extension.
     RFC 9112, Section 7.1.1, Paragraph 2 */
  const bool allow_bws = (2 < discp_lvl);

  mhd_assert (NULL == connection->rp.response);

  buffer_head = connection->read_buffer;
  available = connection->read_buffer_offset;
  do
  {
    size_t to_be_processed;
    size_t left_unprocessed;
    size_t processed_size;

    instant_retry = false;
    if (connection->rq.have_chunked_upload)
    {
      mhd_assert (MHD_SIZE_UNKNOWN == connection->rq.remaining_upload_size);
      if ( (connection->rq.current_chunk_offset ==
            connection->rq.current_chunk_size) &&
           (0 != connection->rq.current_chunk_size) )
      {
        size_t i;
        mhd_assert (0 != available);
        /* skip new line at the *end* of a chunk */
        i = 0;
        if ( (2 <= available) &&
             ('\r' == buffer_head[0]) &&
             ('\n' == buffer_head[1]) )
          i += 2;                        /* skip CRLF */
        else if (bare_lf_as_crlf && ('\n' == buffer_head[0]))
          i++;                           /* skip bare LF */
        else if (2 > available)
          break;                         /* need more upload data */
        if (0 == i)
        {
          /* malformed encoding */
          transmit_error_response_static (connection,
                                          MHD_HTTP_BAD_REQUEST,
                                          REQUEST_CHUNKED_MALFORMED);
          return;
        }
        available -= i;
        buffer_head += i;
        connection->rq.current_chunk_offset = 0;
        connection->rq.current_chunk_size = 0;
        if (0 == available)
          break;
      }
      if (0 != connection->rq.current_chunk_size)
      {
        uint64_t cur_chunk_left;
        mhd_assert (connection->rq.current_chunk_offset < \
                    connection->rq.current_chunk_size);
        /* we are in the middle of a chunk, give
           as much as possible to the client (without
           crossing chunk boundaries) */
        cur_chunk_left
          = connection->rq.current_chunk_size
            - connection->rq.current_chunk_offset;
        if (cur_chunk_left > available)
          to_be_processed = available;
        else
        {         /* cur_chunk_left <= (size_t)available */
          to_be_processed = (size_t) cur_chunk_left;
          if (available > to_be_processed)
            instant_retry = true;
        }
      }
      else
      { /* Need the parse the chunk size line */
        /** The number of found digits in the chunk size number */
        size_t num_dig;
        uint64_t chunk_size;
        bool broken;
        bool overflow;

        mhd_assert (0 != available);

        overflow = false;
        chunk_size = 0; /* Mute possible compiler warning.
                           The real value will be set later. */

        num_dig = MHD_strx_to_uint64_n_ (buffer_head,
                                         available,
                                         &chunk_size);
        mhd_assert (num_dig <= available);
        if (num_dig == available)
          continue; /* Need line delimiter */

        broken = (0 == num_dig);
        if (broken)
          /* Check whether result is invalid due to uint64_t overflow */
          overflow = ((('0' <= buffer_head[0]) && ('9' >= buffer_head[0])) ||
                      (('A' <= buffer_head[0]) && ('F' >= buffer_head[0])) ||
                      (('a' <= buffer_head[0]) && ('f' >= buffer_head[0])));
        else
        {
          /**
           * The length of the string with the number of the chunk size,
           * including chunk extension
           */
          size_t chunk_size_line_len;

          chunk_size_line_len = 0;
          if ((';' == buffer_head[num_dig]) ||
              (allow_bws &&
               ((' ' == buffer_head[num_dig]) ||
                ('\t' == buffer_head[num_dig]))))
          { /* Chunk extension */
            size_t i;

            /* Skip bad whitespaces (if any) */
            for (i = num_dig; i < available; ++i)
            {
              if ((' ' != buffer_head[i]) && ('\t' != buffer_head[i]))
                break;
            }
            if (i == available)
              break; /* need more data */
            if (';' == buffer_head[i])
            {
              for (++i; i < available; ++i)
              {
                if ('\n' == buffer_head[i])
                  break;
              }
              if (i == available)
                break; /* need more data */
              mhd_assert (i > num_dig);
              mhd_assert (1 <= i);
              /* Found LF position */
              if (bare_lf_as_crlf)
                chunk_size_line_len = i; /* Don't care about CR before LF */
              else if ('\r' == buffer_head[i - 1])
                chunk_size_line_len = i;
            }
            else
            { /* No ';' after "bad whitespace" */
              mhd_assert (allow_bws);
              mhd_assert (0 == chunk_size_line_len);
            }
          }
          else
          {
            mhd_assert (available >= num_dig);
            if ((2 <= (available - num_dig)) &&
                ('\r' == buffer_head[num_dig]) &&
                ('\n' == buffer_head[num_dig + 1]))
              chunk_size_line_len = num_dig + 2;
            else if (bare_lf_as_crlf &&
                     ('\n' == buffer_head[num_dig]))
              chunk_size_line_len = num_dig + 1;
            else if (2 > (available - num_dig))
              break; /* need more data */
          }

          if (0 != chunk_size_line_len)
          { /* Valid termination of the chunk size line */
            mhd_assert (chunk_size_line_len <= available);
            /* Start reading payload data of the chunk */
            connection->rq.current_chunk_offset = 0;
            connection->rq.current_chunk_size = chunk_size;

            available -= chunk_size_line_len;
            buffer_head += chunk_size_line_len;

            if (0 == chunk_size)
            { /* The final (termination) chunk */
              connection->rq.remaining_upload_size = 0;
              break;
            }
            if (available > 0)
              instant_retry = true;
            continue;
          }
          /* Invalid chunk size line */
        }

        if (! overflow)
          transmit_error_response_static (connection,
                                          MHD_HTTP_BAD_REQUEST,
                                          REQUEST_CHUNKED_MALFORMED);
        else
          transmit_error_response_static (connection,
                                          MHD_HTTP_CONTENT_TOO_LARGE,
                                          REQUEST_CHUNK_TOO_LARGE);
        return;
      }
    }
    else
    {
      /* no chunked encoding, give all to the client */
      mhd_assert (MHD_SIZE_UNKNOWN != connection->rq.remaining_upload_size);
      mhd_assert (0 != connection->rq.remaining_upload_size);
      if (connection->rq.remaining_upload_size < available)
        to_be_processed = (size_t) connection->rq.remaining_upload_size;
      else
        to_be_processed = available;
    }
    left_unprocessed = to_be_processed;
    connection->rq.client_aware = true;
    connection->in_access_handler = true;
    if (MHD_NO ==
        daemon->default_handler (daemon->default_handler_cls,
                                 connection,
                                 connection->rq.url,
                                 connection->rq.method,
                                 connection->rq.version,
                                 buffer_head,
                                 &left_unprocessed,
                                 &connection->rq.client_context))
    {
      connection->in_access_handler = false;
      /* serious internal error, close connection */
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Application reported internal error, " \
                                 "closing connection."));
      return;
    }
    connection->in_access_handler = false;

    if (left_unprocessed > to_be_processed)
      MHD_PANIC (_ ("libmicrohttpd API violation.\n"));

    connection->rq.some_payload_processed =
      (left_unprocessed != to_be_processed);

    if (0 != left_unprocessed)
    {
      instant_retry = false; /* client did not process everything */
#ifdef HAVE_MESSAGES
      if ((! connection->rq.some_payload_processed) &&
          (! connection->suspended))
      {
        /* client did not process any upload data, complain if
           the setup was incorrect, which may prevent us from
           handling the rest of the request */
        if (MHD_D_IS_USING_THREADS_ (daemon))
          MHD_DLOG (daemon,
                    _ ("WARNING: Access Handler Callback has not processed " \
                       "any upload data and connection is not suspended. " \
                       "This may result in hung connection.\n"));
      }
#endif /* HAVE_MESSAGES */
    }
    processed_size = to_be_processed - left_unprocessed;
    /* dh left "processed" bytes in buffer for next time... */
    buffer_head += processed_size;
    available -= processed_size;
    if (! connection->rq.have_chunked_upload)
    {
      mhd_assert (MHD_SIZE_UNKNOWN != connection->rq.remaining_upload_size);
      connection->rq.remaining_upload_size -= processed_size;
    }
    else
    {
      mhd_assert (MHD_SIZE_UNKNOWN == connection->rq.remaining_upload_size);
      connection->rq.current_chunk_offset += processed_size;
    }
  } while (instant_retry);
  /* TODO: zero out reused memory region */
  if ( (available > 0) &&
       (buffer_head != connection->read_buffer) )
    memmove (connection->read_buffer,
             buffer_head,
             available);
  else
    mhd_assert ((0 == available) || \
                (connection->read_buffer_offset == available));
  connection->read_buffer_offset = available;
}


/**
 * Check if we are done sending the write-buffer.
 * If so, transition into "next_state".
 *
 * @param connection connection to check write status for
 * @param next_state the next state to transition to
 * @return #MHD_NO if we are not done, #MHD_YES if we are
 */
static enum MHD_Result
check_write_done (struct MHD_Connection *connection,
                  enum MHD_CONNECTION_STATE next_state)
{
  if ( (connection->write_buffer_append_offset !=
        connection->write_buffer_send_offset)
       /* || data_in_tls_buffers == true  */
       )
    return MHD_NO;
  connection->write_buffer_append_offset = 0;
  connection->write_buffer_send_offset = 0;
  connection->state = next_state;
  return MHD_YES;
}


/**
 * Parse the various headers; figure out the size
 * of the upload and make sure the headers follow
 * the protocol.  Advance to the appropriate state.
 *
 * @param connection connection we're processing
 */
static void
parse_connection_headers (struct MHD_Connection *connection)
{
  const char *clen;
  const char *enc;
  size_t val_len;

#ifdef COOKIE_SUPPORT
  if (MHD_PARSE_COOKIE_NO_MEMORY == parse_cookie_header (connection))
  {
    handle_req_cookie_no_space (connection);
    return;
  }
#endif /* COOKIE_SUPPORT */
  if ( (-3 < connection->daemon->client_discipline) &&
       (MHD_IS_HTTP_VER_1_1_COMPAT (connection->rq.http_ver)) &&
       (MHD_NO ==
        MHD_lookup_connection_value_n (connection,
                                       MHD_HEADER_KIND,
                                       MHD_HTTP_HEADER_HOST,
                                       MHD_STATICSTR_LEN_ (
                                         MHD_HTTP_HEADER_HOST),
                                       NULL,
                                       NULL)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Received HTTP/1.1 request without `Host' header.\n"));
#endif
    transmit_error_response_static (connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    REQUEST_LACKS_HOST);
    return;
  }

  /* The presence of the request body is indicated by "Content-Length:" or
     "Transfer-Encoding:" request headers.
     Unless one of these two headers is used, the request has no request body.
     See RFC9112, Section 6, paragraph 4. */
  connection->rq.remaining_upload_size = 0;
  if (MHD_NO !=
      MHD_lookup_connection_value_n (connection,
                                     MHD_HEADER_KIND,
                                     MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                     MHD_STATICSTR_LEN_ (
                                       MHD_HTTP_HEADER_TRANSFER_ENCODING),
                                     &enc,
                                     NULL))
  {
    if (! MHD_str_equal_caseless_ (enc,
                                   "chunked"))
    {
      transmit_error_response_static (connection,
                                      MHD_HTTP_BAD_REQUEST,
                                      REQUEST_UNSUPPORTED_TR_ENCODING);
      return;
    }
    else if (MHD_NO !=
             MHD_lookup_connection_value_n (connection,
                                            MHD_HEADER_KIND,
                                            MHD_HTTP_HEADER_CONTENT_LENGTH,
                                            MHD_STATICSTR_LEN_ ( \
                                              MHD_HTTP_HEADER_CONTENT_LENGTH),
                                            NULL,
                                            NULL))
    {
      /* TODO: add individual settings */
      if (1 <= connection->daemon->client_discipline)
      {
        transmit_error_response_static (connection,
                                        MHD_HTTP_BAD_REQUEST,
                                        REQUEST_LENGTH_WITH_TR_ENCODING);
        return;
      }
      else
      {
        /* Must close connection after reply to prevent potential attack */
        connection->keepalive = MHD_CONN_MUST_CLOSE;
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("The 'Content-Length' request header is ignored "
                     "as chunked Transfer-Encoding is used "
                     "for this request.\n"));
#endif /* HAVE_MESSAGES */
      }
    }
    connection->rq.have_chunked_upload = true;
    connection->rq.remaining_upload_size = MHD_SIZE_UNKNOWN;
  }
  else if (MHD_NO !=
           MHD_lookup_connection_value_n (connection,
                                          MHD_HEADER_KIND,
                                          MHD_HTTP_HEADER_CONTENT_LENGTH,
                                          MHD_STATICSTR_LEN_ (
                                            MHD_HTTP_HEADER_CONTENT_LENGTH),
                                          &clen,
                                          &val_len))
  {
    size_t num_digits;

    num_digits = MHD_str_to_uint64_n_ (clen,
                                       val_len,
                                       &connection->rq.remaining_upload_size);

    if (((0 == num_digits) &&
         (0 != val_len) &&
         ('0' <= clen[0]) && ('9' >= clen[0]))
        || (MHD_SIZE_UNKNOWN == connection->rq.remaining_upload_size))
    {
      connection->rq.remaining_upload_size = 0;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Too large value of 'Content-Length' header. " \
                   "Closing connection.\n"));
#endif
      transmit_error_response_static (connection,
                                      MHD_HTTP_CONTENT_TOO_LARGE,
                                      REQUEST_CONTENTLENGTH_TOOLARGE);
    }
    else if ((val_len != num_digits) ||
             (0 == num_digits))
    {
      connection->rq.remaining_upload_size = 0;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to parse 'Content-Length' header. " \
                   "Closing connection.\n"));
#endif
      transmit_error_response_static (connection,
                                      MHD_HTTP_BAD_REQUEST,
                                      REQUEST_CONTENTLENGTH_MALFORMED);
    }
  }
}


/**
 * Reset request header processing state.
 *
 * This function resets the processing state before processing the next header
 * (or footer) line.
 * @param c the connection to process
 */
_MHD_static_inline void
reset_rq_header_processing_state (struct MHD_Connection *c)
{
  memset (&c->rq.hdrs.hdr, 0, sizeof(c->rq.hdrs.hdr));
}


/**
 * Switch to request headers (field lines) processing state.
 * @param c the connection to process
 */
_MHD_static_inline void
switch_to_rq_headers_processing (struct MHD_Connection *c)
{
  c->rq.field_lines.start = c->read_buffer;
  memset (&c->rq.hdrs.hdr, 0, sizeof(c->rq.hdrs.hdr));
  c->state = MHD_CONNECTION_REQ_HEADERS_RECEIVING;
}


#ifndef MHD_MAX_EMPTY_LINES_SKIP
/**
 * The maximum number of ignored empty line before the request line
 * at default "strictness" level.
 */
#define MHD_MAX_EMPTY_LINES_SKIP 1024
#endif /* ! MHD_MAX_EMPTY_LINES_SKIP */

/**
 * Find and parse the request line.
 * @param c the connection to process
 * @return true if request line completely processed (or unrecoverable error
 *         found) and state is changed,
 *         false if not enough data yet in the receive buffer
 */
static bool
get_request_line_inner (struct MHD_Connection *c)
{
  size_t p; /**< The current processing position */
  const int discp_lvl = c->daemon->client_discipline;
  /* Allow to skip one or more empty lines before the request line.
     RFC 9112, section 2.2 */
  const bool skip_empty_lines = (1 >= discp_lvl);
  /* Allow to skip more then one empty line before the request line.
     RFC 9112, section 2.2 */
  const bool skip_several_empty_lines = (skip_empty_lines && (0 >= discp_lvl));
  /* Allow to skip number of unlimited empty lines before the request line.
     RFC 9112, section 2.2 */
  const bool skip_unlimited_empty_lines =
    (skip_empty_lines && (-3 >= discp_lvl));
  /* Treat bare LF as the end of the line.
     RFC 9112, section 2.2 */
  const bool bare_lf_as_crlf = MHD_ALLOW_BARE_LF_AS_CRLF_ (discp_lvl);
  /* Treat tab as whitespace delimiter.
     RFC 9112, section 3 */
  const bool tab_as_wsp = (0 >= discp_lvl);
  /* Treat VT (vertical tab) and FF (form feed) as whitespace delimiters.
     RFC 9112, section 3 */
  const bool other_wsp_as_wsp = (-1 >= discp_lvl);
  /* Treat continuous whitespace block as a single space.
     RFC 9112, section 3 */
  const bool wsp_blocks = (-1 >= discp_lvl);
  /* Parse whitespace in URI, special parsing of the request line.
     RFC 9112, section 3.2 */
  const bool wsp_in_uri = (0 >= discp_lvl);
  /* Keep whitespace in URI, give app URI with whitespace instead of
     automatic redirect to fixed URI.
     Violates RFC 9112, section 3.2 */
  const bool wsp_in_uri_keep = (-2 >= discp_lvl);
  /* Keep bare CR character as is.
     Violates RFC 9112, section 2.2 */
  const bool bare_cr_keep = (wsp_in_uri_keep && (-3 >= discp_lvl));
  /* Treat bare CR as space; replace it with space before processing.
     RFC 9112, section 2.2 */
  const bool bare_cr_as_sp = ((! bare_cr_keep) && (-1 >= discp_lvl));

  mhd_assert (MHD_CONNECTION_INIT == c->state || \
              MHD_CONNECTION_REQ_LINE_RECEIVING == c->state);
  mhd_assert (NULL == c->rq.method || \
              MHD_CONNECTION_REQ_LINE_RECEIVING == c->state);
  mhd_assert (MHD_HTTP_MTHD_NO_METHOD == c->rq.http_mthd || \
              MHD_CONNECTION_REQ_LINE_RECEIVING == c->state);
  mhd_assert (MHD_HTTP_MTHD_NO_METHOD == c->rq.http_mthd || \
              0 != c->rq.hdrs.rq_line.proc_pos);

  if (0 == c->read_buffer_offset)
  {
    mhd_assert (MHD_CONNECTION_INIT == c->state);
    return false; /* No data to process */
  }
  p = c->rq.hdrs.rq_line.proc_pos;
  mhd_assert (p <= c->read_buffer_offset);

  /* Skip empty lines, if any (and if allowed) */
  /* See RFC 9112, section 2.2 */
  if ((0 == p)
      && (skip_empty_lines))
  {
    /* Skip empty lines before the request line.
       See RFC 9112, section 2.2 */
    bool is_empty_line;
    mhd_assert (MHD_CONNECTION_INIT == c->state);
    mhd_assert (NULL == c->rq.method);
    mhd_assert (NULL == c->rq.url);
    mhd_assert (0 == c->rq.url_len);
    mhd_assert (NULL == c->rq.hdrs.rq_line.rq_tgt);
    mhd_assert (0 == c->rq.req_target_len);
    mhd_assert (NULL == c->rq.version);
    do
    {
      is_empty_line = false;
      if ('\r' == c->read_buffer[0])
      {
        if (1 == c->read_buffer_offset)
          return false; /* Not enough data yet */
        if ('\n' == c->read_buffer[1])
        {
          is_empty_line = true;
          c->read_buffer += 2;
          c->read_buffer_size -= 2;
          c->read_buffer_offset -= 2;
          c->rq.hdrs.rq_line.skipped_empty_lines++;
        }
      }
      else if (('\n' == c->read_buffer[0]) &&
               (bare_lf_as_crlf))
      {
        is_empty_line = true;
        c->read_buffer += 1;
        c->read_buffer_size -= 1;
        c->read_buffer_offset -= 1;
        c->rq.hdrs.rq_line.skipped_empty_lines++;
      }
      if (is_empty_line)
      {
        if ((! skip_unlimited_empty_lines) &&
            (((unsigned int) ((skip_several_empty_lines) ?
                              MHD_MAX_EMPTY_LINES_SKIP : 1)) <
             c->rq.hdrs.rq_line.skipped_empty_lines))
        {
          connection_close_error (c,
                                  _ ("Too many meaningless extra empty lines " \
                                     "received before the request"));
          return true; /* Process connection closure */
        }
        if (0 == c->read_buffer_offset)
          return false;  /* No more data to process */
      }
    } while (is_empty_line);
  }
  /* All empty lines are skipped */

  c->state = MHD_CONNECTION_REQ_LINE_RECEIVING;
  /* Read and parse the request line */
  mhd_assert (1 <= c->read_buffer_offset);

  while (p < c->read_buffer_offset)
  {
    const char chr = c->read_buffer[p];
    bool end_of_line;
    /*
       The processing logic is different depending on the configured strictness:

       When whitespace BLOCKS are NOT ALLOWED, the end of the whitespace is
       processed BEFORE processing of the current character.
       When whitespace BLOCKS are ALLOWED, the end of the whitespace is
       processed AFTER processing of the current character.

       When space char in the URI is ALLOWED, the delimiter between the URI and
       the HTTP version string is processed only at the END of the line.
       When space in the URI is NOT ALLOWED, the delimiter between the URI and
       the HTTP version string is processed as soon as the FIRST whitespace is
       found after URI start.
     */

    end_of_line = false;

    mhd_assert ((0 == c->rq.hdrs.rq_line.last_ws_end) || \
                (c->rq.hdrs.rq_line.last_ws_end > \
                 c->rq.hdrs.rq_line.last_ws_start));
    mhd_assert ((0 == c->rq.hdrs.rq_line.last_ws_start) || \
                (0 != c->rq.hdrs.rq_line.last_ws_end));

    /* Check for the end of the line */
    if ('\r' == chr)
    {
      if (p + 1 == c->read_buffer_offset)
      {
        c->rq.hdrs.rq_line.proc_pos = p;
        return false; /* Not enough data yet */
      }
      else if ('\n' == c->read_buffer[p + 1])
        end_of_line = true;
      else
      {
        /* Bare CR alone */
        /* Must be rejected or replaced with space char.
           See RFC 9112, section 2.2 */
        if (bare_cr_as_sp)
        {
          c->read_buffer[p] = ' ';
          c->rq.num_cr_sp_replaced++;
          continue; /* Re-start processing of the current character */
        }
        else if (! bare_cr_keep)
        {
          /* A quick simple check whether this line looks like an HTTP request */
          if ((MHD_HTTP_MTHD_GET <= c->rq.http_mthd) &&
              (MHD_HTTP_MTHD_DELETE >= c->rq.http_mthd))
          {
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            BARE_CR_IN_HEADER);
          }
          else
            connection_close_error (c,
                                    _ ("Bare CR characters are not allowed " \
                                       "in the request line.\n"));
          return true; /* Error in the request */
        }
      }
    }
    else if ('\n' == chr)
    {
      /* Bare LF may be recognised as a line delimiter.
         See RFC 9112, section 2.2 */
      if (bare_lf_as_crlf)
        end_of_line = true;
      else
      {
        /* While RFC does not enforce error for bare LF character,
           if this char is not treated as a line delimiter, it should be
           rejected to avoid any security weakness due to request smuggling. */
        /* A quick simple check whether this line looks like an HTTP request */
        if ((MHD_HTTP_MTHD_GET <= c->rq.http_mthd) &&
            (MHD_HTTP_MTHD_DELETE >= c->rq.http_mthd))
        {
          transmit_error_response_static (c,
                                          MHD_HTTP_BAD_REQUEST,
                                          BARE_LF_IN_HEADER);
        }
        else
          connection_close_error (c,
                                  _ ("Bare LF characters are not allowed " \
                                     "in the request line.\n"));
        return true; /* Error in the request */
      }
    }

    if (end_of_line)
    {
      /* Handle the end of the request line */

      if (NULL != c->rq.method)
      {
        if (wsp_in_uri)
        {
          /* The end of the URI and the start of the HTTP version string
             should be determined now. */
          mhd_assert (NULL == c->rq.version);
          mhd_assert (0 == c->rq.req_target_len);
          if (0 != c->rq.hdrs.rq_line.last_ws_end)
          {
            /* Determine the end and the length of the URI */
            if (NULL != c->rq.hdrs.rq_line.rq_tgt)
            {
              c->read_buffer [c->rq.hdrs.rq_line.last_ws_start] = 0; /* Zero terminate the URI */
              c->rq.req_target_len =
                c->rq.hdrs.rq_line.last_ws_start
                - (size_t) (c->rq.hdrs.rq_line.rq_tgt - c->read_buffer);
            }
            else if ((c->rq.hdrs.rq_line.last_ws_start + 1 <
                      c->rq.hdrs.rq_line.last_ws_end) &&
                     (HTTP_VER_LEN == (p - c->rq.hdrs.rq_line.last_ws_end)))
            {
              /* Found only HTTP method and HTTP version and more than one
                 whitespace between them. Assume zero-length URI. */
              mhd_assert (wsp_blocks);
              c->rq.hdrs.rq_line.last_ws_start++;
              c->read_buffer[c->rq.hdrs.rq_line.last_ws_start] = 0; /* Zero terminate the URI */
              c->rq.hdrs.rq_line.rq_tgt =
                c->read_buffer + c->rq.hdrs.rq_line.last_ws_start;
              c->rq.req_target_len = 0;
              c->rq.hdrs.rq_line.num_ws_in_uri = 0;
              c->rq.hdrs.rq_line.rq_tgt_qmark = NULL;
            }
            /* Determine the start of the HTTP version string */
            if (NULL != c->rq.hdrs.rq_line.rq_tgt)
            {
              c->rq.version = c->read_buffer + c->rq.hdrs.rq_line.last_ws_end;
            }
          }
        }
        else
        {
          /* The end of the URI and the start of the HTTP version string
             should be already known. */
          if ((NULL == c->rq.version)
              && (NULL != c->rq.hdrs.rq_line.rq_tgt)
              && (HTTP_VER_LEN == p - (size_t) (c->rq.hdrs.rq_line.rq_tgt
                                                - c->read_buffer))
              && (0 != c->read_buffer[(size_t)
                                      (c->rq.hdrs.rq_line.rq_tgt
                                       - c->read_buffer) - 1]))
          {
            /* Found only HTTP method and HTTP version and more than one
               whitespace between them. Assume zero-length URI. */
            size_t uri_pos;
            mhd_assert (wsp_blocks);
            mhd_assert (0 == c->rq.req_target_len);
            uri_pos = (size_t) (c->rq.hdrs.rq_line.rq_tgt - c->read_buffer) - 1;
            mhd_assert (uri_pos < p);
            c->rq.version = c->rq.hdrs.rq_line.rq_tgt;
            c->read_buffer[uri_pos] = 0;  /* Zero terminate the URI */
            c->rq.hdrs.rq_line.rq_tgt = c->read_buffer + uri_pos;
            c->rq.req_target_len = 0;
            c->rq.hdrs.rq_line.num_ws_in_uri = 0;
            c->rq.hdrs.rq_line.rq_tgt_qmark = NULL;
          }
        }

        if (NULL != c->rq.version)
        {
          mhd_assert (NULL != c->rq.hdrs.rq_line.rq_tgt);
          if (! parse_http_version (c, c->rq.version,
                                    p
                                    - (size_t) (c->rq.version
                                                - c->read_buffer)))
          {
            mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING < c->state);
            return true; /* Unsupported / broken HTTP version */
          }
          c->read_buffer[p] = 0; /* Zero terminate the HTTP version strings */
          if ('\r' == chr)
          {
            p++; /* Consume CR */
            mhd_assert (p < c->read_buffer_offset); /* The next character has been already checked */
          }
          p++; /* Consume LF */
          c->read_buffer += p;
          c->read_buffer_size -= p;
          c->read_buffer_offset -= p;
          mhd_assert (c->rq.hdrs.rq_line.num_ws_in_uri <= \
                      c->rq.req_target_len);
          mhd_assert ((NULL == c->rq.hdrs.rq_line.rq_tgt_qmark) || \
                      (0 != c->rq.req_target_len));
          mhd_assert ((NULL == c->rq.hdrs.rq_line.rq_tgt_qmark) || \
                      ((size_t) (c->rq.hdrs.rq_line.rq_tgt_qmark \
                                 - c->rq.hdrs.rq_line.rq_tgt) < \
                       c->rq.req_target_len));
          mhd_assert ((NULL == c->rq.hdrs.rq_line.rq_tgt_qmark) || \
                      (c->rq.hdrs.rq_line.rq_tgt_qmark >= \
                       c->rq.hdrs.rq_line.rq_tgt));
          return true; /* The request line is successfully parsed */
        }
      }
      /* Error in the request line */

      /* A quick simple check whether this line looks like an HTTP request */
      if ((MHD_HTTP_MTHD_GET <= c->rq.http_mthd) &&
          (MHD_HTTP_MTHD_DELETE >= c->rq.http_mthd))
      {
        transmit_error_response_static (c,
                                        MHD_HTTP_BAD_REQUEST,
                                        REQUEST_MALFORMED);
      }
      else
        connection_close_error (c,
                                _ ("The request line is malformed.\n"));

      return true;
    }

    /* Process possible end of the previously found whitespace delimiter */
    if ((! wsp_blocks) &&
        (p == c->rq.hdrs.rq_line.last_ws_end) &&
        (0 != c->rq.hdrs.rq_line.last_ws_end))
    {
      /* Previous character was a whitespace char and whitespace blocks
         are not allowed. */
      /* The current position is the next character after
         a whitespace delimiter */
      if (NULL == c->rq.hdrs.rq_line.rq_tgt)
      {
        /* The current position is the start of the URI */
        mhd_assert (0 == c->rq.req_target_len);
        mhd_assert (NULL == c->rq.version);
        c->rq.hdrs.rq_line.rq_tgt = c->read_buffer + p;
        /* Reset the whitespace marker */
        c->rq.hdrs.rq_line.last_ws_start = 0;
        c->rq.hdrs.rq_line.last_ws_end = 0;
      }
      else
      {
        /* It was a whitespace after the start of the URI */
        if (! wsp_in_uri)
        {
          mhd_assert ((0 != c->rq.req_target_len) || \
                      (c->rq.hdrs.rq_line.rq_tgt + 1 == c->read_buffer + p));
          mhd_assert (NULL == c->rq.version); /* Too many whitespaces? This error is handled at whitespace start */
          c->rq.version = c->read_buffer + p;
          /* Reset the whitespace marker */
          c->rq.hdrs.rq_line.last_ws_start = 0;
          c->rq.hdrs.rq_line.last_ws_end = 0;
        }
      }
    }

    /* Process the current character.
       Is it not the end of the line.  */
    if ((' ' == chr)
        || (('\t' == chr) && (tab_as_wsp))
        || ((other_wsp_as_wsp) && ((0xb == chr) || (0xc == chr))))
    {
      /* A whitespace character */
      if ((0 == c->rq.hdrs.rq_line.last_ws_end) ||
          (p != c->rq.hdrs.rq_line.last_ws_end) ||
          (! wsp_blocks))
      {
        /* Found first whitespace char of the new whitespace block */
        if (NULL == c->rq.method)
        {
          /* Found the end of the HTTP method string */
          mhd_assert (0 == c->rq.hdrs.rq_line.last_ws_start);
          mhd_assert (0 == c->rq.hdrs.rq_line.last_ws_end);
          mhd_assert (NULL == c->rq.hdrs.rq_line.rq_tgt);
          mhd_assert (0 == c->rq.req_target_len);
          mhd_assert (NULL == c->rq.version);
          if (0 == p)
          {
            connection_close_error (c,
                                    _ ("The request line starts with "
                                       "a whitespace.\n"));
            return true; /* Error in the request */
          }
          c->read_buffer[p] = 0; /* Zero-terminate the request method string */
          c->rq.method = c->read_buffer;
          parse_http_std_method (c, c->rq.method, p);
        }
        else
        {
          /* A whitespace after the start of the URI */
          if (! wsp_in_uri)
          {
            /* Whitespace in URI is not allowed to be parsed */
            if (NULL == c->rq.version)
            {
              mhd_assert (NULL != c->rq.hdrs.rq_line.rq_tgt);
              /* This is a delimiter between URI and HTTP version string */
              c->read_buffer[p] = 0; /* Zero-terminate request URI string */
              mhd_assert (((size_t) (c->rq.hdrs.rq_line.rq_tgt   \
                                     - c->read_buffer)) <= p);
              c->rq.req_target_len =
                p - (size_t) (c->rq.hdrs.rq_line.rq_tgt - c->read_buffer);
            }
            else
            {
              /* This is a delimiter AFTER version string */

              /* A quick simple check whether this line looks like an HTTP request */
              if ((MHD_HTTP_MTHD_GET <= c->rq.http_mthd) &&
                  (MHD_HTTP_MTHD_DELETE >= c->rq.http_mthd))
              {
                transmit_error_response_static (c,
                                                MHD_HTTP_BAD_REQUEST,
                                                RQ_LINE_TOO_MANY_WSP);
              }
              else
                connection_close_error (c,
                                        _ ("The request line has more than "
                                           "two whitespaces.\n"));
              return true; /* Error in the request */
            }
          }
          else
          {
            /* Whitespace in URI is allowed to be parsed */
            if (0 != c->rq.hdrs.rq_line.last_ws_end)
            {
              /* The whitespace after the start of the URI has been found already */
              c->rq.hdrs.rq_line.num_ws_in_uri +=
                c->rq.hdrs.rq_line.last_ws_end
                - c->rq.hdrs.rq_line.last_ws_start;
            }
          }
        }
        c->rq.hdrs.rq_line.last_ws_start = p;
        c->rq.hdrs.rq_line.last_ws_end = p + 1; /* Will be updated on the next char parsing */
      }
      else
      {
        /* Continuation of the whitespace block */
        mhd_assert (0 != c->rq.hdrs.rq_line.last_ws_end);
        mhd_assert (0 != p);
        c->rq.hdrs.rq_line.last_ws_end = p + 1;
      }
    }
    else
    {
      /* Non-whitespace char, not the end of the line */
      mhd_assert ((0 == c->rq.hdrs.rq_line.last_ws_end) || \
                  (c->rq.hdrs.rq_line.last_ws_end == p) || \
                  wsp_in_uri);

      if ((p == c->rq.hdrs.rq_line.last_ws_end) &&
          (0 != c->rq.hdrs.rq_line.last_ws_end) &&
          (wsp_blocks))
      {
        /* The end of the whitespace block */
        if (NULL == c->rq.hdrs.rq_line.rq_tgt)
        {
          /* This is the first character of the URI */
          mhd_assert (0 == c->rq.req_target_len);
          mhd_assert (NULL == c->rq.version);
          c->rq.hdrs.rq_line.rq_tgt = c->read_buffer + p;
          /* Reset the whitespace marker */
          c->rq.hdrs.rq_line.last_ws_start = 0;
          c->rq.hdrs.rq_line.last_ws_end = 0;
        }
        else
        {
          if (! wsp_in_uri)
          {
            /* This is the first character of the HTTP version */
            mhd_assert (NULL != c->rq.hdrs.rq_line.rq_tgt);
            mhd_assert ((0 != c->rq.req_target_len) || \
                        (c->rq.hdrs.rq_line.rq_tgt + 1 == c->read_buffer + p));
            mhd_assert (NULL == c->rq.version); /* Handled at whitespace start */
            c->rq.version = c->read_buffer + p;
            /* Reset the whitespace marker */
            c->rq.hdrs.rq_line.last_ws_start = 0;
            c->rq.hdrs.rq_line.last_ws_end = 0;
          }
        }
      }

      /* Handle other special characters */
      if ('?' == chr)
      {
        if ((NULL == c->rq.hdrs.rq_line.rq_tgt_qmark) &&
            (NULL != c->rq.hdrs.rq_line.rq_tgt))
        {
          c->rq.hdrs.rq_line.rq_tgt_qmark = c->read_buffer + p;
        }
      }
      else if ((0xb == chr) || (0xc == chr))
      {
        /* VT or LF characters */
        mhd_assert (! other_wsp_as_wsp);
        if ((NULL != c->rq.hdrs.rq_line.rq_tgt) &&
            (NULL == c->rq.version) &&
            (wsp_in_uri))
        {
          c->rq.hdrs.rq_line.num_ws_in_uri++;
        }
        else
        {
          connection_close_error (c,
                                  _ ("Invalid character is in the "
                                     "request line.\n"));
          return true; /* Error in the request */
        }
      }
      else if (0 == chr)
      {
        /* NUL character */
        connection_close_error (c,
                                _ ("The NUL character is in the "
                                   "request line.\n"));
        return true; /* Error in the request */
      }
    }

    p++;
  }

  c->rq.hdrs.rq_line.proc_pos = p;
  return false; /* Not enough data yet */
}


#ifndef MHD_MAX_FIXED_URI_LEN
/**
 * The maximum size of the fixed URI for automatic redirection
 */
#define MHD_MAX_FIXED_URI_LEN (64 * 1024)
#endif /* ! MHD_MAX_FIXED_URI_LEN */

/**
 * Send the automatic redirection to fixed URI when received URI with
 * whitespaces.
 * If URI is too large, close connection with error.
 *
 * @param c the connection to process
 */
static void
send_redirect_fixed_rq_target (struct MHD_Connection *c)
{
  char *b;
  size_t fixed_uri_len;
  size_t i;
  size_t o;
  char *hdr_name;
  size_t hdr_name_len;

  mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING == c->state);
  mhd_assert (0 != c->rq.hdrs.rq_line.num_ws_in_uri);
  mhd_assert (c->rq.hdrs.rq_line.num_ws_in_uri <= \
              c->rq.req_target_len);
  fixed_uri_len = c->rq.req_target_len
                  + 2 * c->rq.hdrs.rq_line.num_ws_in_uri;
  if ( (fixed_uri_len + 200 > c->daemon->pool_size) ||
       (fixed_uri_len > MHD_MAX_FIXED_URI_LEN) ||
       (NULL == (b = malloc (fixed_uri_len + 1))) )
  {
    connection_close_error (c,
                            _ ("The request has whitespace character is " \
                               "in the URI and the URI is too large to " \
                               "send automatic redirect to fixed URI.\n"));
    return;
  }
  i = 0;
  o = 0;

  do
  {
    const char chr = c->rq.hdrs.rq_line.rq_tgt[i++];

    mhd_assert ('\r' != chr); /* Replaced during request line parsing */
    mhd_assert ('\n' != chr); /* Rejected during request line parsing */
    mhd_assert (0 != chr); /* Rejected during request line parsing */
    switch (chr)
    {
    case ' ':
      b[o++] = '%';
      b[o++] = '2';
      b[o++] = '0';
      break;
    case '\t':
      b[o++] = '%';
      b[o++] = '0';
      b[o++] = '9';
      break;
    case 0x0B:   /* VT (vertical tab) */
      b[o++] = '%';
      b[o++] = '0';
      b[o++] = 'B';
      break;
    case 0x0C:   /* FF (form feed) */
      b[o++] = '%';
      b[o++] = '0';
      b[o++] = 'C';
      break;
    default:
      b[o++] = chr;
      break;
    }
  } while (i < c->rq.req_target_len);
  mhd_assert (fixed_uri_len == o);
  b[o] = 0; /* Zero-terminate the result */

  hdr_name_len = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_LOCATION);
  hdr_name = malloc (hdr_name_len + 1);
  if (NULL != hdr_name)
  {
    memcpy (hdr_name,
            MHD_HTTP_HEADER_LOCATION,
            hdr_name_len + 1);
    /* hdr_name and b are free()d within this call */
    transmit_error_response_header (c,
                                    MHD_HTTP_MOVED_PERMANENTLY,
                                    RQ_TARGET_INVALID_CHAR,
                                    hdr_name,
                                    hdr_name_len,
                                    b,
                                    o);
    return;
  }
  free (b);
  connection_close_error (c,
                          _ ("The request has whitespace character is in the " \
                             "URI.\n"));
  return;
}


/**
 * Process request-target string, form URI and URI parameters
 * @param c the connection to process
 * @return true if request-target successfully processed,
 *         false if error encountered
 */
static bool
process_request_target (struct MHD_Connection *c)
{
#ifdef _DEBUG
  size_t params_len;
#endif /* _DEBUG */
  mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING == c->state);
  mhd_assert (NULL == c->rq.url);
  mhd_assert (0 == c->rq.url_len);
  mhd_assert (NULL != c->rq.hdrs.rq_line.rq_tgt);
  mhd_assert ((NULL == c->rq.hdrs.rq_line.rq_tgt_qmark) || \
              (c->rq.hdrs.rq_line.rq_tgt <= c->rq.hdrs.rq_line.rq_tgt_qmark));
  mhd_assert ((NULL == c->rq.hdrs.rq_line.rq_tgt_qmark) || \
              (c->rq.req_target_len > \
               (size_t) (c->rq.hdrs.rq_line.rq_tgt_qmark \
                         - c->rq.hdrs.rq_line.rq_tgt)));

  /* Log callback before the request-target is modified/decoded */
  if (NULL != c->daemon->uri_log_callback)
  {
    c->rq.client_aware = true;
    c->rq.client_context =
      c->daemon->uri_log_callback (c->daemon->uri_log_callback_cls,
                                   c->rq.hdrs.rq_line.rq_tgt,
                                   c);
  }

  if (NULL != c->rq.hdrs.rq_line.rq_tgt_qmark)
  {
#ifdef _DEBUG
    params_len =
      c->rq.req_target_len
      - (size_t) (c->rq.hdrs.rq_line.rq_tgt_qmark - c->rq.hdrs.rq_line.rq_tgt);
#endif /* _DEBUG */
    c->rq.hdrs.rq_line.rq_tgt_qmark[0] = 0; /* Replace '?' with zero termination */
    if (MHD_NO == MHD_parse_arguments_ (c,
                                        MHD_GET_ARGUMENT_KIND,
                                        c->rq.hdrs.rq_line.rq_tgt_qmark + 1,
                                        &connection_add_header,
                                        c))
    {
      mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING != c->state);
      return false;
    }
  }
#ifdef _DEBUG
  else
    params_len = 0;
#endif /* _DEBUG */

  mhd_assert (strlen (c->rq.hdrs.rq_line.rq_tgt) == \
              c->rq.req_target_len - params_len);

  /* Finally unescape URI itself */
  c->rq.url_len =
    c->daemon->unescape_callback (c->daemon->unescape_callback_cls,
                                  c,
                                  c->rq.hdrs.rq_line.rq_tgt);
  c->rq.url = c->rq.hdrs.rq_line.rq_tgt;

  return true;
}


/**
 * Find and parse the request line.
 * Advance to the next state when done, handle errors.
 * @param c the connection to process
 * @return true if request line completely processed and state is changed,
 *         false if not enough data yet in the receive buffer
 */
static bool
get_request_line (struct MHD_Connection *c)
{
  const int discp_lvl = c->daemon->client_discipline;
  /* Parse whitespace in URI, special parsing of the request line */
  const bool wsp_in_uri = (0 >= discp_lvl);
  /* Keep whitespace in URI, give app URI with whitespace instead of
     automatic redirect to fixed URI */
  const bool wsp_in_uri_keep = (-2 >= discp_lvl);

  if (! get_request_line_inner (c))
  {
    /* End of the request line has not been found yet */
    mhd_assert ((! wsp_in_uri) || NULL == c->rq.version);
    if ((NULL != c->rq.version) &&
        (HTTP_VER_LEN <
         (c->rq.hdrs.rq_line.proc_pos
          - (size_t) (c->rq.version - c->read_buffer))))
    {
      c->rq.http_ver = MHD_HTTP_VER_INVALID;
      transmit_error_response_static (c,
                                      MHD_HTTP_BAD_REQUEST,
                                      REQUEST_MALFORMED);
      return true; /* Error in the request */
    }
    return false;
  }
  if (MHD_CONNECTION_REQ_LINE_RECEIVING < c->state)
    return true; /* Error in the request */

  mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING == c->state);
  mhd_assert (NULL == c->rq.url);
  mhd_assert (0 == c->rq.url_len);
  mhd_assert (NULL != c->rq.hdrs.rq_line.rq_tgt);
  if (0 != c->rq.hdrs.rq_line.num_ws_in_uri)
  {
    if (! wsp_in_uri)
    {
      transmit_error_response_static (c,
                                      MHD_HTTP_BAD_REQUEST,
                                      RQ_TARGET_INVALID_CHAR);
      return true; /* Error in the request */
    }
    if (! wsp_in_uri_keep)
    {
      send_redirect_fixed_rq_target (c);
      return true; /* Error in the request */
    }
  }
  if (! process_request_target (c))
    return true; /* Error in processing */

  c->state = MHD_CONNECTION_REQ_LINE_RECEIVED;
  return true;
}


/**
 * Results of header line reading
 */
enum MHD_HdrLineReadRes_
{
  /**
   * Not enough data yet
   */
  MHD_HDR_LINE_READING_NEED_MORE_DATA = 0,
  /**
   * New header line has been read
   */
  MHD_HDR_LINE_READING_GOT_HEADER,
  /**
   * Error in header data, error response has been queued
   */
  MHD_HDR_LINE_READING_DATA_ERROR,
  /**
   * Found the end of the request header (end of field lines)
   */
  MHD_HDR_LINE_READING_GOT_END_OF_HEADER
} _MHD_FIXED_ENUM;


/**
 * Find the end of the request header line and make basic header parsing.
 * Handle errors and header folding.
 * @param c the connection to process
 * @param process_footers if true then footers are processed,
 *                        if false then headers are processed
 * @param[out] hdr_name the name of the parsed header (field)
 * @param[out] hdr_name the value of the parsed header (field)
 * @return true if request header line completely processed,
 *         false if not enough data yet in the receive buffer
 */
static enum MHD_HdrLineReadRes_
get_req_header (struct MHD_Connection *c,
                bool process_footers,
                struct _MHD_str_w_len *hdr_name,
                struct _MHD_str_w_len *hdr_value)
{
  const int discp_lvl = c->daemon->client_discipline;
  /* Treat bare LF as the end of the line.
     RFC 9112, section 2.2-3
     Note: MHD never replaces bare LF with space (RFC 9110, section 5.5-5).
     Bare LF is processed as end of the line or rejected as broken request. */
  const bool bare_lf_as_crlf = MHD_ALLOW_BARE_LF_AS_CRLF_ (discp_lvl);
  /* Keep bare CR character as is.
     Violates RFC 9112, section 2.2-4 */
  const bool bare_cr_keep = (-3 >= discp_lvl);
  /* Treat bare CR as space; replace it with space before processing.
     RFC 9112, section 2.2-4 */
  const bool bare_cr_as_sp = ((! bare_cr_keep) && (-1 >= discp_lvl));
  /* Treat NUL as space; replace it with space before processing.
     RFC 9110, section 5.5-5 */
  const bool nul_as_sp = (-1 >= discp_lvl);
  /* Allow folded header lines.
     RFC 9112, section 5.2-4 */
  const bool allow_folded = (0 >= discp_lvl);
  /* Do not reject headers with the whitespace at the start of the first line.
     When allowed, the first line with whitespace character at the first
     position is ignored (as well as all possible line foldings of the first
     line).
     RFC 9112, section 2.2-8 */
  const bool allow_wsp_at_start = allow_folded && (-1 >= discp_lvl);
  /* Allow whitespace in header (field) name.
     Violates RFC 9110, section 5.1-2 */
  const bool allow_wsp_in_name = (-2 >= discp_lvl);
  /* Allow zero-length header (field) name.
     Violates RFC 9110, section 5.1-2 */
  const bool allow_empty_name = (-2 >= discp_lvl);
  /* Allow whitespace before colon.
     Violates RFC 9112, section 5.1-2 */
  const bool allow_wsp_before_colon = (-3 >= discp_lvl);
  /* Do not abort the request when header line has no colon, just skip such
     bad lines.
     RFC 9112, section 5-1 */
  const bool allow_line_without_colon = (-2 >= discp_lvl);

  size_t p; /**< The position of the currently processed character */

#if ! defined (HAVE_MESSAGES) && ! defined(_DEBUG)
  (void) process_footers; /* Unused parameter */
#endif /* !HAVE_MESSAGES && !_DEBUG */

  mhd_assert ((process_footers ? MHD_CONNECTION_FOOTERS_RECEIVING : \
               MHD_CONNECTION_REQ_HEADERS_RECEIVING) == \
              c->state);

  p = c->rq.hdrs.hdr.proc_pos;

  mhd_assert (p <= c->read_buffer_offset);
  while (p < c->read_buffer_offset)
  {
    const char chr = c->read_buffer[p];
    bool end_of_line;

    mhd_assert ((0 == c->rq.hdrs.hdr.name_len) || \
                (c->rq.hdrs.hdr.name_len < p));
    mhd_assert ((0 == c->rq.hdrs.hdr.name_len) || (0 != p));
    mhd_assert ((0 == c->rq.hdrs.hdr.name_len) || \
                (c->rq.hdrs.hdr.name_end_found));
    mhd_assert ((0 == c->rq.hdrs.hdr.value_start) || \
                (c->rq.hdrs.hdr.name_len < c->rq.hdrs.hdr.value_start));
    mhd_assert ((0 == c->rq.hdrs.hdr.value_start) || \
                (0 != c->rq.hdrs.hdr.name_len));
    mhd_assert ((0 == c->rq.hdrs.hdr.ws_start) || \
                (0 == c->rq.hdrs.hdr.name_len) || \
                (c->rq.hdrs.hdr.ws_start > c->rq.hdrs.hdr.name_len));
    mhd_assert ((0 == c->rq.hdrs.hdr.ws_start) || \
                (0 == c->rq.hdrs.hdr.value_start) || \
                (c->rq.hdrs.hdr.ws_start > c->rq.hdrs.hdr.value_start));

    /* Check for the end of the line */
    if ('\r' == chr)
    {
      if (0 != p)
      {
        /* Line is not empty, need to check for possible line folding */
        if (p + 2 >= c->read_buffer_offset)
          break; /* Not enough data yet to check for folded line */
      }
      else
      {
        /* Line is empty, no need to check for possible line folding */
        if (p + 2 > c->read_buffer_offset)
          break; /* Not enough data yet to check for the end of the line */
      }
      if ('\n' == c->read_buffer[p + 1])
        end_of_line = true;
      else
      {
        /* Bare CR alone */
        /* Must be rejected or replaced with space char.
           See RFC 9112, section 2.2-4 */
        if (bare_cr_as_sp)
        {
          c->read_buffer[p] = ' ';
          c->rq.num_cr_sp_replaced++;
          continue; /* Re-start processing of the current character */
        }
        else if (! bare_cr_keep)
        {
          if (! process_footers)
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            BARE_CR_IN_HEADER);
          else
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            BARE_CR_IN_FOOTER);
          return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
        }
        end_of_line = false;
      }
    }
    else if ('\n' == chr)
    {
      /* Bare LF may be recognised as a line delimiter.
         See RFC 9112, section 2.2-3 */
      if (bare_lf_as_crlf)
      {
        if (0 != p)
        {
          /* Line is not empty, need to check for possible line folding */
          if (p + 1 >= c->read_buffer_offset)
            break; /* Not enough data yet to check for folded line */
        }
        end_of_line = true;
      }
      else
      {
        if (! process_footers)
          transmit_error_response_static (c,
                                          MHD_HTTP_BAD_REQUEST,
                                          BARE_LF_IN_HEADER);
        else
          transmit_error_response_static (c,
                                          MHD_HTTP_BAD_REQUEST,
                                          BARE_LF_IN_FOOTER);
        return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
      }
    }
    else
      end_of_line = false;

    if (end_of_line)
    {
      /* Handle the end of the line */
      /**
       *  The full length of the line, including CRLF (or bare LF).
       */
      const size_t line_len = p + (('\r' == chr) ? 2 : 1);
      char next_line_char;
      mhd_assert (line_len <= c->read_buffer_offset);

      if (0 == p)
      {
        /* Zero-length header line. This is the end of the request header
           section.
           RFC 9112, Section 2.1-1 */
        mhd_assert (! c->rq.hdrs.hdr.starts_with_ws);
        mhd_assert (! c->rq.hdrs.hdr.name_end_found);
        mhd_assert (0 == c->rq.hdrs.hdr.name_len);
        mhd_assert (0 == c->rq.hdrs.hdr.ws_start);
        mhd_assert (0 == c->rq.hdrs.hdr.value_start);
        /* Consume the line with CRLF (or bare LF) */
        c->read_buffer += line_len;
        c->read_buffer_offset -= line_len;
        c->read_buffer_size -= line_len;
        return MHD_HDR_LINE_READING_GOT_END_OF_HEADER;
      }

      mhd_assert (line_len < c->read_buffer_offset);
      mhd_assert (0 != line_len);
      mhd_assert ('\n' == c->read_buffer[line_len - 1]);
      next_line_char = c->read_buffer[line_len];
      if ((' ' == next_line_char) ||
          ('\t' == next_line_char))
      {
        /* Folded line */
        if (! allow_folded)
        {
          if (! process_footers)
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            ERR_RSP_OBS_FOLD);
          else
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            ERR_RSP_OBS_FOLD_FOOTER);

          return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
        }
        /* Replace CRLF (or bare LF) character(s) with space characters.
           See RFC 9112, Section 5.2-4 */
        c->read_buffer[p] = ' ';
        if ('\r' == chr)
          c->read_buffer[p + 1] = ' ';
        continue; /* Re-start processing of the current character */
      }
      else
      {
        /* It is not a folded line, it's the real end of the non-empty line */
        bool skip_line = false;
        mhd_assert (0 != p);
        if (c->rq.hdrs.hdr.starts_with_ws)
        {
          /* This is the first line and it starts with whitespace. This line
             must be discarded completely.
             See RFC 9112, Section 2.2-8 */
          mhd_assert (allow_wsp_at_start);
#ifdef HAVE_MESSAGES
          MHD_DLOG (c->daemon,
                    _ ("Whitespace-prefixed first header line " \
                       "has been skipped.\n"));
#endif /* HAVE_MESSAGES */
          skip_line = true;
        }
        else if (! c->rq.hdrs.hdr.name_end_found)
        {
          if (! allow_line_without_colon)
          {
            if (! process_footers)
              transmit_error_response_static (c,
                                              MHD_HTTP_BAD_REQUEST,
                                              ERR_RSP_HEADER_WITHOUT_COLON);
            else
              transmit_error_response_static (c,
                                              MHD_HTTP_BAD_REQUEST,
                                              ERR_RSP_FOOTER_WITHOUT_COLON);

            return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
          }
          /* Skip broken line completely */
          c->rq.skipped_broken_lines++;
          skip_line = true;
        }
        if (skip_line)
        {
          /* Skip the entire line */
          c->read_buffer += line_len;
          c->read_buffer_offset -= line_len;
          c->read_buffer_size -= line_len;
          p = 0;
          /* Reset processing state */
          memset (&c->rq.hdrs.hdr, 0, sizeof(c->rq.hdrs.hdr));
          /* Start processing of the next line */
          continue;
        }
        else
        {
          /* This line should be valid header line */
          size_t value_len;
          mhd_assert ((0 != c->rq.hdrs.hdr.name_len) || allow_empty_name);

          hdr_name->str = c->read_buffer + 0; /* The name always starts at the first character */
          hdr_name->len = c->rq.hdrs.hdr.name_len;
          mhd_assert (0 == hdr_name->str[hdr_name->len]);

          if (0 == c->rq.hdrs.hdr.value_start)
          {
            c->rq.hdrs.hdr.value_start = p;
            c->read_buffer[p] = 0;
            value_len = 0;
          }
          else if (0 != c->rq.hdrs.hdr.ws_start)
          {
            mhd_assert (p > c->rq.hdrs.hdr.ws_start);
            mhd_assert (c->rq.hdrs.hdr.ws_start > c->rq.hdrs.hdr.value_start);
            c->read_buffer[c->rq.hdrs.hdr.ws_start] = 0;
            value_len = c->rq.hdrs.hdr.ws_start - c->rq.hdrs.hdr.value_start;
          }
          else
          {
            mhd_assert (p > c->rq.hdrs.hdr.ws_start);
            c->read_buffer[p] = 0;
            value_len = p - c->rq.hdrs.hdr.value_start;
          }
          hdr_value->str = c->read_buffer + c->rq.hdrs.hdr.value_start;
          hdr_value->len = value_len;
          mhd_assert (0 == hdr_value->str[hdr_value->len]);
          /* Consume the entire line */
          c->read_buffer += line_len;
          c->read_buffer_offset -= line_len;
          c->read_buffer_size -= line_len;
          return MHD_HDR_LINE_READING_GOT_HEADER;
        }
      }
    }
    else if ((' ' == chr) || ('\t' == chr))
    {
      if (0 == p)
      {
        if (! allow_wsp_at_start)
        {
          if (! process_footers)
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            ERR_RSP_WSP_BEFORE_HEADER);
          else
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            ERR_RSP_WSP_BEFORE_FOOTER);
          return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
        }
        c->rq.hdrs.hdr.starts_with_ws = true;
      }
      else if ((! c->rq.hdrs.hdr.name_end_found) &&
               (! c->rq.hdrs.hdr.starts_with_ws))
      {
        /* Whitespace in header name / between header name and colon */
        if (allow_wsp_in_name || allow_wsp_before_colon)
        {
          if (0 == c->rq.hdrs.hdr.ws_start)
            c->rq.hdrs.hdr.ws_start = p;
        }
        else
        {
          if (! process_footers)
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            ERR_RSP_WSP_IN_HEADER_NAME);
          else
            transmit_error_response_static (c,
                                            MHD_HTTP_BAD_REQUEST,
                                            ERR_RSP_WSP_IN_FOOTER_NAME);

          return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
        }
      }
      else
      {
        /* Whitespace before/inside/after header (field) value */
        if (0 == c->rq.hdrs.hdr.ws_start)
          c->rq.hdrs.hdr.ws_start = p;
      }
    }
    else if (0 == chr)
    {
      if (! nul_as_sp)
      {
        if (! process_footers)
          transmit_error_response_static (c,
                                          MHD_HTTP_BAD_REQUEST,
                                          ERR_RSP_INVALID_CHR_IN_HEADER);
        else
          transmit_error_response_static (c,
                                          MHD_HTTP_BAD_REQUEST,
                                          ERR_RSP_INVALID_CHR_IN_FOOTER);

        return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
      }
      c->read_buffer[p] = ' ';
      continue; /* Re-start processing of the current character */
    }
    else
    {
      /* Not a whitespace, not the end of the header line */
      mhd_assert ('\r' != chr);
      mhd_assert ('\n' != chr);
      mhd_assert ('\0' != chr);
      if ((! c->rq.hdrs.hdr.name_end_found) &&
          (! c->rq.hdrs.hdr.starts_with_ws))
      {
        /* Processing the header (field) name */
        if (':' == chr)
        {
          if (0 == c->rq.hdrs.hdr.ws_start)
            c->rq.hdrs.hdr.name_len = p;
          else
          {
            mhd_assert (allow_wsp_in_name || allow_wsp_before_colon);
            if (! allow_wsp_before_colon)
            {
              if (! process_footers)
                transmit_error_response_static (c,
                                                MHD_HTTP_BAD_REQUEST,
                                                ERR_RSP_WSP_IN_HEADER_NAME);
              else
                transmit_error_response_static (c,
                                                MHD_HTTP_BAD_REQUEST,
                                                ERR_RSP_WSP_IN_FOOTER_NAME);
              return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
            }
            c->rq.hdrs.hdr.name_len = c->rq.hdrs.hdr.ws_start;
#ifndef MHD_FAVOR_SMALL_CODE
            c->rq.hdrs.hdr.ws_start = 0; /* Not on whitespace anymore */
#endif /* ! MHD_FAVOR_SMALL_CODE */
          }
          if ((0 == c->rq.hdrs.hdr.name_len) && ! allow_empty_name)
          {
            if (! process_footers)
              transmit_error_response_static (c,
                                              MHD_HTTP_BAD_REQUEST,
                                              ERR_RSP_EMPTY_HEADER_NAME);
            else
              transmit_error_response_static (c,
                                              MHD_HTTP_BAD_REQUEST,
                                              ERR_RSP_EMPTY_FOOTER_NAME);
            return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
          }
          c->rq.hdrs.hdr.name_end_found = true;
          c->read_buffer[c->rq.hdrs.hdr.name_len] = 0; /* Zero-terminate the name */
        }
        else
        {
          if (0 != c->rq.hdrs.hdr.ws_start)
          {
            /* End of the whitespace in header (field) name */
            mhd_assert (allow_wsp_in_name || allow_wsp_before_colon);
            if (! allow_wsp_in_name)
            {
              if (! process_footers)
                transmit_error_response_static (c,
                                                MHD_HTTP_BAD_REQUEST,
                                                ERR_RSP_WSP_IN_HEADER_NAME);
              else
                transmit_error_response_static (c,
                                                MHD_HTTP_BAD_REQUEST,
                                                ERR_RSP_WSP_IN_FOOTER_NAME);

              return MHD_HDR_LINE_READING_DATA_ERROR; /* Error in the request */
            }
#ifndef MHD_FAVOR_SMALL_CODE
            c->rq.hdrs.hdr.ws_start = 0; /* Not on whitespace anymore */
#endif /* ! MHD_FAVOR_SMALL_CODE */
          }
        }
      }
      else
      {
        /* Processing the header (field) value */
        if (0 == c->rq.hdrs.hdr.value_start)
          c->rq.hdrs.hdr.value_start = p;
#ifndef MHD_FAVOR_SMALL_CODE
        c->rq.hdrs.hdr.ws_start = 0; /* Not on whitespace anymore */
#endif /* ! MHD_FAVOR_SMALL_CODE */
      }
#ifdef MHD_FAVOR_SMALL_CODE
      c->rq.hdrs.hdr.ws_start = 0; /* Not on whitespace anymore */
#endif /* MHD_FAVOR_SMALL_CODE */
    }
    p++;
  }
  c->rq.hdrs.hdr.proc_pos = p;
  return MHD_HDR_LINE_READING_NEED_MORE_DATA; /* Not enough data yet */
}


/**
 * Find the end of the request headers and make basic header parsing.
 * Advance to the next state when done, handle errors.
 * @param c the connection to process
 * @param process_footers if true then footers are processed,
 *                        if false then headers are processed
 * @return true if request headers reading finished (either successfully
 *         or with error),
 *         false if not enough data yet in the receive buffer
 */
static bool
get_req_headers (struct MHD_Connection *c, bool process_footers)
{
  do
  {
    struct _MHD_str_w_len hdr_name;
    struct _MHD_str_w_len hdr_value;
    enum MHD_HdrLineReadRes_ res;

    mhd_assert ((process_footers ? MHD_CONNECTION_FOOTERS_RECEIVING : \
                 MHD_CONNECTION_REQ_HEADERS_RECEIVING) == \
                c->state);

    #ifdef _DEBUG
    hdr_name.str = NULL;
    hdr_value.str = NULL;
#endif /* _DEBUG */
    res = get_req_header (c, process_footers, &hdr_name, &hdr_value);
    if (MHD_HDR_LINE_READING_GOT_HEADER == res)
    {
      mhd_assert ((process_footers ? MHD_CONNECTION_FOOTERS_RECEIVING : \
                   MHD_CONNECTION_REQ_HEADERS_RECEIVING) == \
                  c->state);
      mhd_assert (NULL != hdr_name.str);
      mhd_assert (NULL != hdr_value.str);
      /* Values must be zero-terminated and must not have binary zeros */
      mhd_assert (strlen (hdr_name.str) == hdr_name.len);
      mhd_assert (strlen (hdr_value.str) == hdr_value.len);
      /* Values must not have whitespaces at the start or at the end */
      mhd_assert ((hdr_name.len == 0) || (hdr_name.str[0] != ' '));
      mhd_assert ((hdr_name.len == 0) || (hdr_name.str[0] != '\t'));
      mhd_assert ((hdr_name.len == 0) || \
                  (hdr_name.str[hdr_name.len - 1] != ' '));
      mhd_assert ((hdr_name.len == 0) || \
                  (hdr_name.str[hdr_name.len - 1] != '\t'));
      mhd_assert ((hdr_value.len == 0) || (hdr_value.str[0] != ' '));
      mhd_assert ((hdr_value.len == 0) || (hdr_value.str[0] != '\t'));
      mhd_assert ((hdr_value.len == 0) || \
                  (hdr_value.str[hdr_value.len - 1] != ' '));
      mhd_assert ((hdr_value.len == 0) || \
                  (hdr_value.str[hdr_value.len - 1] != '\t'));

      if (MHD_NO ==
          MHD_set_connection_value_n_nocheck_ (c,
                                               (! process_footers) ?
                                               MHD_HEADER_KIND :
                                               MHD_FOOTER_KIND,
                                               hdr_name.str, hdr_name.len,
                                               hdr_value.str, hdr_value.len))
      {
        size_t add_element_size;

        mhd_assert (hdr_name.str < hdr_value.str);

#ifdef HAVE_MESSAGES
        MHD_DLOG (c->daemon,
                  _ ("Failed to allocate memory in the connection memory " \
                     "pool to store %s.\n"),
                  (! process_footers) ? _ ("header") : _ ("footer"));
#endif /* HAVE_MESSAGES */

        add_element_size = hdr_value.len
                           + (size_t) (hdr_value.str - hdr_name.str);

        if (! process_footers)
          handle_req_headers_no_space (c, hdr_name.str, add_element_size);
        else
          handle_req_footers_no_space (c, hdr_name.str, add_element_size);

        mhd_assert (MHD_CONNECTION_FULL_REQ_RECEIVED < c->state);
        return true;
      }
      /* Reset processing state */
      reset_rq_header_processing_state (c);
      mhd_assert ((process_footers ? MHD_CONNECTION_FOOTERS_RECEIVING : \
                   MHD_CONNECTION_REQ_HEADERS_RECEIVING) == \
                  c->state);
      /* Read the next header (field) line */
      continue;
    }
    else if (MHD_HDR_LINE_READING_NEED_MORE_DATA == res)
    {
      mhd_assert ((process_footers ? MHD_CONNECTION_FOOTERS_RECEIVING : \
                   MHD_CONNECTION_REQ_HEADERS_RECEIVING) == \
                  c->state);
      return false;
    }
    else if (MHD_HDR_LINE_READING_DATA_ERROR == res)
    {
      mhd_assert ((process_footers ? \
                   MHD_CONNECTION_FOOTERS_RECEIVING : \
                   MHD_CONNECTION_REQ_HEADERS_RECEIVING) < c->state);
      mhd_assert (c->stop_with_error);
      mhd_assert (c->discard_request);
      return true;
    }
    mhd_assert (MHD_HDR_LINE_READING_GOT_END_OF_HEADER == res);
    break;
  } while (1);

#ifdef HAVE_MESSAGES
  if (1 == c->rq.num_cr_sp_replaced)
  {
    MHD_DLOG (c->daemon,
              _ ("One bare CR character has been replaced with space " \
                 "in %s.\n"),
              (! process_footers) ?
              _ ("the request line or in the request headers") :
              _ ("the request footers"));
  }
  else if (0 != c->rq.num_cr_sp_replaced)
  {
    MHD_DLOG (c->daemon,
              _ ("%" PRIu64 " bare CR characters have been replaced with " \
                 "spaces in the request line and/or in the request %s.\n"),
              (uint64_t) c->rq.num_cr_sp_replaced,
              (! process_footers) ? _ ("headers") : _ ("footers"));
  }
  if (1 == c->rq.skipped_broken_lines)
  {
    MHD_DLOG (c->daemon,
              _ ("One %s line without colon has been skipped.\n"),
              (! process_footers) ? _ ("header") : _ ("footer"));
  }
  else if (0 != c->rq.skipped_broken_lines)
  {
    MHD_DLOG (c->daemon,
              _ ("%" PRIu64 " %s lines without colons has been skipped.\n"),
              (uint64_t) c->rq.skipped_broken_lines,
              (! process_footers) ? _ ("header") : _ ("footer"));
  }
#endif /* HAVE_MESSAGES */

  mhd_assert (c->rq.method < c->read_buffer);
  if (! process_footers)
  {
    c->rq.header_size = (size_t) (c->read_buffer - c->rq.method);
    mhd_assert (NULL != c->rq.field_lines.start);
    c->rq.field_lines.size =
      (size_t) ((c->read_buffer - c->rq.field_lines.start) - 1);
    if ('\r' == *(c->read_buffer - 2))
      c->rq.field_lines.size--;
    c->state = MHD_CONNECTION_HEADERS_RECEIVED;

    if (MHD_BUF_INC_SIZE > c->read_buffer_size)
    {
      /* Try to re-use some of the last bytes of the request header */
      /* Do this only if space in the read buffer is limited AND
         amount of read ahead data is small. */
      /**
       *  The position of the terminating NUL after the last character of
       *  the last header element.
       */
      const char *last_elmnt_end;
      size_t shift_back_size;
      if (NULL != c->rq.headers_received_tail)
        last_elmnt_end =
          c->rq.headers_received_tail->value
          + c->rq.headers_received_tail->value_size;
      else
        last_elmnt_end = c->rq.version + HTTP_VER_LEN;
      mhd_assert ((last_elmnt_end + 1) < c->read_buffer);
      shift_back_size = (size_t) (c->read_buffer - (last_elmnt_end + 1));
      if (0 != c->read_buffer_offset)
        memmove (c->read_buffer - shift_back_size,
                 c->read_buffer,
                 c->read_buffer_offset);
      c->read_buffer -= shift_back_size;
      c->read_buffer_size += shift_back_size;
    }
  }
  else
    c->state = MHD_CONNECTION_FOOTERS_RECEIVED;

  return true;
}


/**
 * Update the 'last_activity' field of the connection to the current time
 * and move the connection to the head of the 'normal_timeout' list if
 * the timeout for the connection uses the default value.
 *
 * @param connection the connection that saw some activity
 */
void
MHD_update_last_activity_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
#if defined(MHD_USE_THREADS)
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */

  if (0 == connection->connection_timeout_ms)
    return;  /* Skip update of activity for connections
               without timeout timer. */
  if (connection->suspended)
    return;  /* no activity on suspended connections */

  connection->last_activity = MHD_monotonic_msec_counter ();
  if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    return; /* each connection has personal timeout */

  if (connection->connection_timeout_ms != daemon->connection_timeout_ms)
    return; /* custom timeout, no need to move it in "normal" DLL */
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  /* move connection to head of timeout list (by remove + add operation) */
  XDLL_remove (daemon->normal_timeout_head,
               daemon->normal_timeout_tail,
               connection);
  XDLL_insert (daemon->normal_timeout_head,
               daemon->normal_timeout_tail,
               connection);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
}


/**
 * This function handles a particular connection when it has been
 * determined that there is data to be read off a socket. All
 * implementations (multithreaded, external polling, internal polling)
 * call this function to handle reads.
 *
 * @param connection connection to handle
 * @param socket_error set to true if socket error was detected
 */
void
MHD_connection_handle_read (struct MHD_Connection *connection,
                            bool socket_error)
{
  ssize_t bytes_read;

  if ( (MHD_CONNECTION_CLOSED == connection->state) ||
       (connection->suspended) )
    return;
#ifdef HTTPS_SUPPORT
  if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
  {   /* HTTPS connection. */
    if (MHD_TLS_CONN_CONNECTED > connection->tls_state)
    {
      if (! MHD_run_tls_handshake_ (connection))
        return;
    }
  }
#endif /* HTTPS_SUPPORT */

  mhd_assert (NULL != connection->read_buffer);
  if (connection->read_buffer_size == connection->read_buffer_offset)
    return; /* No space for receiving data. */

  bytes_read = connection->recv_cls (connection,
                                     &connection->read_buffer
                                     [connection->read_buffer_offset],
                                     connection->read_buffer_size
                                     - connection->read_buffer_offset);
  if ((bytes_read < 0) || socket_error)
  {
    if ((MHD_ERR_AGAIN_ == bytes_read) && ! socket_error)
      return;     /* No new data to process. */
    if ((bytes_read > 0) && connection->sk_nonblck)
    { /* Try to detect the socket error */
      int dummy;
      bytes_read = connection->recv_cls (connection, &dummy, sizeof (dummy));
    }
    if (MHD_ERR_CONNRESET_ == bytes_read)
    {
      if ( (MHD_CONNECTION_INIT < connection->state) &&
           (MHD_CONNECTION_FULL_REQ_RECEIVED > connection->state) )
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("Socket has been disconnected when reading request.\n"));
#endif
        connection->discard_request = true;
      }
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_READ_ERROR);
      return;
    }

#ifdef HAVE_MESSAGES
    if (MHD_CONNECTION_INIT != connection->state)
      MHD_DLOG (connection->daemon,
                _ ("Connection socket is closed when reading " \
                   "request due to the error: %s\n"),
                (bytes_read < 0) ? str_conn_error_ (bytes_read) :
                "detected connection closure");
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            NULL);
    return;
  }

  if (0 == bytes_read)
  {   /* Remote side closed connection. */
    connection->read_closed = true;
    if ( (MHD_CONNECTION_INIT < connection->state) &&
         (MHD_CONNECTION_FULL_REQ_RECEIVED > connection->state) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Connection was closed by remote side with incomplete "
                   "request.\n"));
#endif
      connection->discard_request = true;
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_CLIENT_ABORT);
    }
    else if (MHD_CONNECTION_INIT == connection->state)
      /* This termination code cannot be reported to the application
       * because application has not been informed yet about this request */
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_COMPLETED_OK);
    else
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_WITH_ERROR);
    return;
  }
  connection->read_buffer_offset += (size_t) bytes_read;
  MHD_update_last_activity_ (connection);
#if DEBUG_STATES
  MHD_DLOG (connection->daemon,
            _ ("In function %s handling connection at state: %s\n"),
            MHD_FUNC_,
            MHD_state_to_string (connection->state));
#endif
  /* TODO: check whether the next 'switch()' really needed */
  switch (connection->state)
  {
  case MHD_CONNECTION_INIT:
  case MHD_CONNECTION_REQ_LINE_RECEIVING:
  case MHD_CONNECTION_REQ_HEADERS_RECEIVING:
  case MHD_CONNECTION_BODY_RECEIVING:
  case MHD_CONNECTION_FOOTERS_RECEIVING:
  case MHD_CONNECTION_FULL_REQ_RECEIVED:
    /* nothing to do but default action */
    if (connection->read_closed)
    {
      /* TODO: check whether this really needed */
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_READ_ERROR);
    }
    return;
  case MHD_CONNECTION_CLOSED:
    return;
#ifdef UPGRADE_SUPPORT
  case MHD_CONNECTION_UPGRADE:
    mhd_assert (0);
    return;
#endif /* UPGRADE_SUPPORT */
  case MHD_CONNECTION_START_REPLY:
    /* shrink read buffer to how much is actually used */
    /* TODO: remove shrink as it handled in special function */
    if ((0 != connection->read_buffer_size) &&
        (connection->read_buffer_size != connection->read_buffer_offset))
    {
      mhd_assert (NULL != connection->read_buffer);
      connection->read_buffer =
        MHD_pool_reallocate (connection->pool,
                             connection->read_buffer,
                             connection->read_buffer_size,
                             connection->read_buffer_offset);
      connection->read_buffer_size = connection->read_buffer_offset;
    }
    break;
  case MHD_CONNECTION_REQ_LINE_RECEIVED:
  case MHD_CONNECTION_HEADERS_RECEIVED:
  case MHD_CONNECTION_HEADERS_PROCESSED:
  case MHD_CONNECTION_BODY_RECEIVED:
  case MHD_CONNECTION_FOOTERS_RECEIVED:
    /* Milestone state, no data should be read */
    mhd_assert (0); /* Should not be possible */
    break;
  case MHD_CONNECTION_CONTINUE_SENDING:
  case MHD_CONNECTION_HEADERS_SENDING:
  case MHD_CONNECTION_HEADERS_SENT:
  case MHD_CONNECTION_NORMAL_BODY_UNREADY:
  case MHD_CONNECTION_NORMAL_BODY_READY:
  case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
  case MHD_CONNECTION_CHUNKED_BODY_READY:
  case MHD_CONNECTION_CHUNKED_BODY_SENT:
  case MHD_CONNECTION_FOOTERS_SENDING:
  case MHD_CONNECTION_FULL_REPLY_SENT:
  default:
    mhd_assert (0); /* Should not be possible */
    break;
  }
  return;
}


/**
 * This function was created to handle writes to sockets when it has
 * been determined that the socket can be written to. All
 * implementations (multithreaded, external select, internal select)
 * call this function
 *
 * @param connection connection to handle
 */
void
MHD_connection_handle_write (struct MHD_Connection *connection)
{
  struct MHD_Response *response;
  ssize_t ret;
  if (connection->suspended)
    return;

#ifdef HTTPS_SUPPORT
  if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
  {   /* HTTPS connection. */
    if (MHD_TLS_CONN_CONNECTED > connection->tls_state)
    {
      if (! MHD_run_tls_handshake_ (connection))
        return;
    }
  }
#endif /* HTTPS_SUPPORT */

#if DEBUG_STATES
  MHD_DLOG (connection->daemon,
            _ ("In function %s handling connection at state: %s\n"),
            MHD_FUNC_,
            MHD_state_to_string (connection->state));
#endif
  switch (connection->state)
  {
  case MHD_CONNECTION_INIT:
  case MHD_CONNECTION_REQ_LINE_RECEIVING:
  case MHD_CONNECTION_REQ_LINE_RECEIVED:
  case MHD_CONNECTION_REQ_HEADERS_RECEIVING:
  case MHD_CONNECTION_HEADERS_RECEIVED:
  case MHD_CONNECTION_HEADERS_PROCESSED:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_CONTINUE_SENDING:
    ret = MHD_send_data_ (connection,
                          &HTTP_100_CONTINUE
                          [connection->continue_message_write_offset],
                          MHD_STATICSTR_LEN_ (HTTP_100_CONTINUE)
                          - connection->continue_message_write_offset,
                          true);
    if (ret < 0)
    {
      if (MHD_ERR_AGAIN_ == ret)
        return;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to send data in request for %s.\n"),
                connection->rq.url);
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              NULL);
      return;
    }
#if _MHD_DEBUG_SEND_DATA
    fprintf (stderr,
             _ ("Sent 100 continue response: `%.*s'\n"),
             (int) ret,
             &HTTP_100_CONTINUE[connection->continue_message_write_offset]);
#endif
    connection->continue_message_write_offset += (size_t) ret;
    MHD_update_last_activity_ (connection);
    return;
  case MHD_CONNECTION_BODY_RECEIVING:
  case MHD_CONNECTION_BODY_RECEIVED:
  case MHD_CONNECTION_FOOTERS_RECEIVING:
  case MHD_CONNECTION_FOOTERS_RECEIVED:
  case MHD_CONNECTION_FULL_REQ_RECEIVED:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_START_REPLY:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_HEADERS_SENDING:
    {
      struct MHD_Response *const resp = connection->rp.response;
      const size_t wb_ready = connection->write_buffer_append_offset
                              - connection->write_buffer_send_offset;
      mhd_assert (connection->write_buffer_append_offset >= \
                  connection->write_buffer_send_offset);
      mhd_assert (NULL != resp);
      mhd_assert ( (0 == resp->data_size) || \
                   (0 == resp->data_start) || \
                   (NULL != resp->crc) );
      mhd_assert ( (0 == connection->rp.rsp_write_position) || \
                   (resp->total_size ==
                    connection->rp.rsp_write_position) );
      mhd_assert ((MHD_CONN_MUST_UPGRADE != connection->keepalive) || \
                  (! connection->rp.props.send_reply_body));

      if ( (connection->rp.props.send_reply_body) &&
           (NULL == resp->crc) &&
           (NULL == resp->data_iov) &&
           /* TODO: remove the next check as 'send_reply_body' is used */
           (0 == connection->rp.rsp_write_position) &&
           (! connection->rp.props.chunked) )
      {
        mhd_assert (resp->total_size >= resp->data_size);
        mhd_assert (0 == resp->data_start);
        /* Send response headers alongside the response body, if the body
         * data is available. */
        ret = MHD_send_hdr_and_body_ (connection,
                                      &connection->write_buffer
                                      [connection->write_buffer_send_offset],
                                      wb_ready,
                                      false,
                                      resp->data,
                                      resp->data_size,
                                      (resp->total_size == resp->data_size));
      }
      else
      {
        /* This is response for HEAD request or reply body is not allowed
         * for any other reason or reply body is dynamically generated. */
        /* Do not send the body data even if it's available. */
        ret = MHD_send_hdr_and_body_ (connection,
                                      &connection->write_buffer
                                      [connection->write_buffer_send_offset],
                                      wb_ready,
                                      false,
                                      NULL,
                                      0,
                                      ((0 == resp->total_size) ||
                                       (! connection->rp.props.send_reply_body)
                                      ));
      }

      if (ret < 0)
      {
        if (MHD_ERR_AGAIN_ == ret)
          return;
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("Failed to send the response headers for the " \
                     "request for `%s'. Error: %s\n"),
                  connection->rq.url,
                  str_conn_error_ (ret));
#endif
        CONNECTION_CLOSE_ERROR (connection,
                                NULL);
        return;
      }
      /* 'ret' is not negative, it's safe to cast it to 'size_t'. */
      if (((size_t) ret) > wb_ready)
      {
        /* The complete header and some response data have been sent,
         * update both offsets. */
        mhd_assert (0 == connection->rp.rsp_write_position);
        mhd_assert (! connection->rp.props.chunked);
        mhd_assert (connection->rp.props.send_reply_body);
        connection->write_buffer_send_offset += wb_ready;
        connection->rp.rsp_write_position = ((size_t) ret) - wb_ready;
      }
      else
        connection->write_buffer_send_offset += (size_t) ret;
      MHD_update_last_activity_ (connection);
      if (MHD_CONNECTION_HEADERS_SENDING != connection->state)
        return;
      check_write_done (connection,
                        MHD_CONNECTION_HEADERS_SENT);
      return;
    }
  case MHD_CONNECTION_HEADERS_SENT:
    return;
  case MHD_CONNECTION_NORMAL_BODY_READY:
    response = connection->rp.response;
    if (connection->rp.rsp_write_position <
        connection->rp.response->total_size)
    {
      uint64_t data_write_offset;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != response->crc)
        MHD_mutex_lock_chk_ (&response->mutex);
#endif
      if (MHD_NO == try_ready_normal_body (connection))
      {
        /* mutex was already unlocked by try_ready_normal_body */
        return;
      }
#if defined(_MHD_HAVE_SENDFILE)
      if (MHD_resp_sender_sendfile == connection->rp.resp_sender)
      {
        mhd_assert (NULL == response->data_iov);
        ret = MHD_send_sendfile_ (connection);
      }
      else /* combined with the next 'if' */
#endif /* _MHD_HAVE_SENDFILE */
      if (NULL != response->data_iov)
      {
        ret = MHD_send_iovec_ (connection,
                               &connection->rp.resp_iov,
                               true);
      }
      else
      {
        data_write_offset = connection->rp.rsp_write_position
                            - response->data_start;
        if (data_write_offset > (uint64_t) SIZE_MAX)
          MHD_PANIC (_ ("Data offset exceeds limit.\n"));
        ret = MHD_send_data_ (connection,
                              &response->data
                              [(size_t) data_write_offset],
                              response->data_size
                              - (size_t) data_write_offset,
                              true);
#if _MHD_DEBUG_SEND_DATA
        if (ret > 0)
          fprintf (stderr,
                   _ ("Sent %d-byte DATA response: `%.*s'\n"),
                   (int) ret,
                   (int) ret,
                   &rp.response->data[connection->rp.rsp_write_position
                                      - rp.response->data_start]);
#endif
      }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != response->crc)
        MHD_mutex_unlock_chk_ (&response->mutex);
#endif
      if (ret < 0)
      {
        if (MHD_ERR_AGAIN_ == ret)
          return;
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("Failed to send the response body for the " \
                     "request for `%s'. Error: %s\n"),
                  connection->rq.url,
                  str_conn_error_ (ret));
#endif
        CONNECTION_CLOSE_ERROR (connection,
                                NULL);
        return;
      }
      connection->rp.rsp_write_position += (size_t) ret;
      MHD_update_last_activity_ (connection);
    }
    if (connection->rp.rsp_write_position ==
        connection->rp.response->total_size)
      connection->state = MHD_CONNECTION_FULL_REPLY_SENT;
    return;
  case MHD_CONNECTION_NORMAL_BODY_UNREADY:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_CHUNKED_BODY_READY:
    ret = MHD_send_data_ (connection,
                          &connection->write_buffer
                          [connection->write_buffer_send_offset],
                          connection->write_buffer_append_offset
                          - connection->write_buffer_send_offset,
                          true);
    if (ret < 0)
    {
      if (MHD_ERR_AGAIN_ == ret)
        return;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to send the chunked response body for the " \
                   "request for `%s'. Error: %s\n"),
                connection->rq.url,
                str_conn_error_ (ret));
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              NULL);
      return;
    }
    connection->write_buffer_send_offset += (size_t) ret;
    MHD_update_last_activity_ (connection);
    if (MHD_CONNECTION_CHUNKED_BODY_READY != connection->state)
      return;
    check_write_done (connection,
                      (connection->rp.response->total_size ==
                       connection->rp.rsp_write_position) ?
                      MHD_CONNECTION_CHUNKED_BODY_SENT :
                      MHD_CONNECTION_CHUNKED_BODY_UNREADY);
    return;
  case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
  case MHD_CONNECTION_CHUNKED_BODY_SENT:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_FOOTERS_SENDING:
    ret = MHD_send_data_ (connection,
                          &connection->write_buffer
                          [connection->write_buffer_send_offset],
                          connection->write_buffer_append_offset
                          - connection->write_buffer_send_offset,
                          true);
    if (ret < 0)
    {
      if (MHD_ERR_AGAIN_ == ret)
        return;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to send the footers for the " \
                   "request for `%s'. Error: %s\n"),
                connection->rq.url,
                str_conn_error_ (ret));
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              NULL);
      return;
    }
    connection->write_buffer_send_offset += (size_t) ret;
    MHD_update_last_activity_ (connection);
    if (MHD_CONNECTION_FOOTERS_SENDING != connection->state)
      return;
    check_write_done (connection,
                      MHD_CONNECTION_FULL_REPLY_SENT);
    return;
  case MHD_CONNECTION_FULL_REPLY_SENT:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_CLOSED:
    return;
#ifdef UPGRADE_SUPPORT
  case MHD_CONNECTION_UPGRADE:
    mhd_assert (0);
    return;
#endif /* UPGRADE_SUPPORT */
  default:
    mhd_assert (0);
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Internal error.\n"));
    break;
  }
  return;
}


/**
 * Check whether connection has timed out.
 * @param c the connection to check
 * @return true if connection has timeout and needs to be closed,
 *         false otherwise.
 */
static bool
connection_check_timedout (struct MHD_Connection *c)
{
  const uint64_t timeout = c->connection_timeout_ms;
  uint64_t now;
  uint64_t since_actv;

  if (c->suspended)
    return false;
  if (0 == timeout)
    return false;
  now = MHD_monotonic_msec_counter ();
  since_actv = now - c->last_activity;
  /* Keep the next lines in sync with #connection_get_wait() to avoid
   * undesired side-effects like busy-waiting. */
  if (timeout < since_actv)
  {
    if (UINT64_MAX / 2 < since_actv)
    {
      const uint64_t jump_back = c->last_activity - now;
      /* Very unlikely that it is more than quarter-million years pause.
       * More likely that system clock jumps back. */
      if (5000 >= jump_back)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (c->daemon,
                  _ ("Detected system clock %u milliseconds jump back.\n"),
                  (unsigned int) jump_back);
#endif
        return false;
      }
#ifdef HAVE_MESSAGES
      MHD_DLOG (c->daemon,
                _ ("Detected too large system clock %" PRIu64 " milliseconds "
                   "jump back.\n"),
                jump_back);
#endif
    }
    return true;
  }
  return false;
}


/**
 * Clean up the state of the given connection and move it into the
 * clean up queue for final disposal.
 * @remark To be called only from thread that process connection's
 * recv(), send() and response.
 *
 * @param connection handle for the connection to clean up
 */
static void
cleanup_connection (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (connection->tid) );
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */

  if (connection->in_cleanup)
    return; /* Prevent double cleanup. */
  connection->in_cleanup = true;
  if (NULL != connection->rp.response)
  {
    MHD_destroy_response (connection->rp.response);
    connection->rp.response = NULL;
  }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if (connection->suspended)
  {
    DLL_remove (daemon->suspended_connections_head,
                daemon->suspended_connections_tail,
                connection);
    connection->suspended = false;
  }
  else
  {
    if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    {
      if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
        XDLL_remove (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     connection);
      else
        XDLL_remove (daemon->manual_timeout_head,
                     daemon->manual_timeout_tail,
                     connection);
    }
    DLL_remove (daemon->connections_head,
                daemon->connections_tail,
                connection);
  }
  DLL_insert (daemon->cleanup_head,
              daemon->cleanup_tail,
              connection);
  connection->resuming = false;
  connection->in_idle = false;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
  {
    /* if we were at the connection limit before and are in
       thread-per-connection mode, signal the main thread
       to resume accepting connections */
    if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
         (! MHD_itc_activate_ (daemon->itc, "c")) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to signal end of connection via inter-thread " \
                   "communication channel.\n"));
#endif
    }
  }
}


/**
 * Set initial internal states for the connection to start reading and
 * processing incoming data.
 * @param c the connection to process
 */
void
MHD_connection_set_initial_state_ (struct MHD_Connection *c)
{
  size_t read_buf_size;

#ifdef HTTPS_SUPPORT
  mhd_assert ( (0 == (c->daemon->options & MHD_USE_TLS)) || \
               (MHD_TLS_CONN_INIT == c->tls_state) );
  mhd_assert ( (0 != (c->daemon->options & MHD_USE_TLS)) || \
               (MHD_TLS_CONN_NO_TLS == c->tls_state) );
#endif /* HTTPS_SUPPORT */
  mhd_assert (MHD_CONNECTION_INIT == c->state);

  c->keepalive = MHD_CONN_KEEPALIVE_UNKOWN;
  c->event_loop_info = MHD_EVENT_LOOP_INFO_READ;

  memset (&c->rq, 0, sizeof(c->rq));
  memset (&c->rp, 0, sizeof(c->rp));

  c->write_buffer = NULL;
  c->write_buffer_size = 0;
  c->write_buffer_send_offset = 0;
  c->write_buffer_append_offset = 0;

  c->continue_message_write_offset = 0;

  c->read_buffer_offset = 0;
  read_buf_size = c->daemon->pool_size / 2;
  c->read_buffer
    = MHD_pool_allocate (c->pool,
                         read_buf_size,
                         false);
  c->read_buffer_size = read_buf_size;
}


/**
 * Reset connection after request-reply cycle.
 * @param connection the connection to process
 * @param reuse the flag to choose whether to close connection or
 *              prepare connection for the next request processing
 */
static void
connection_reset (struct MHD_Connection *connection,
                  bool reuse)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Daemon *const d = connection->daemon;

  if (! reuse)
  {
    /* Next function will destroy response, notify client,
     * destroy memory pool, and set connection state to "CLOSED" */
    MHD_connection_close_ (c,
                           c->stop_with_error ?
                           MHD_REQUEST_TERMINATED_WITH_ERROR :
                           MHD_REQUEST_TERMINATED_COMPLETED_OK);
    c->read_buffer = NULL;
    c->read_buffer_size = 0;
    c->read_buffer_offset = 0;
    c->write_buffer = NULL;
    c->write_buffer_size = 0;
    c->write_buffer_send_offset = 0;
    c->write_buffer_append_offset = 0;
  }
  else
  {
    /* Reset connection to process the next request */
    size_t new_read_buf_size;
    mhd_assert (! c->stop_with_error);
    mhd_assert (! c->discard_request);

    if ( (NULL != d->notify_completed) &&
         (c->rq.client_aware) )
      d->notify_completed (d->notify_completed_cls,
                           c,
                           &c->rq.client_context,
                           MHD_REQUEST_TERMINATED_COMPLETED_OK);
    c->rq.client_aware = false;

    if (NULL != c->rp.response)
      MHD_destroy_response (c->rp.response);
    c->rp.response = NULL;

    c->keepalive = MHD_CONN_KEEPALIVE_UNKOWN;
    c->state = MHD_CONNECTION_INIT;
    c->event_loop_info =
      (0 == c->read_buffer_offset) ?
      MHD_EVENT_LOOP_INFO_READ : MHD_EVENT_LOOP_INFO_PROCESS;

    memset (&c->rq, 0, sizeof(c->rq));

    /* iov (if any) will be deallocated by MHD_pool_reset */
    memset (&c->rp, 0, sizeof(c->rp));

    c->write_buffer = NULL;
    c->write_buffer_size = 0;
    c->write_buffer_send_offset = 0;
    c->write_buffer_append_offset = 0;
    c->continue_message_write_offset = 0;

    /* Reset the read buffer to the starting size,
       preserving the bytes we have already read. */
    new_read_buf_size = c->daemon->pool_size / 2;
    if (c->read_buffer_offset > new_read_buf_size)
      new_read_buf_size = c->read_buffer_offset;

    c->read_buffer
      = MHD_pool_reset (c->pool,
                        c->read_buffer,
                        c->read_buffer_offset,
                        new_read_buf_size);
    c->read_buffer_size = new_read_buf_size;
  }
  c->rq.client_context = NULL;
}


/**
 * This function was created to handle per-connection processing that
 * has to happen even if the socket cannot be read or written to.
 * All implementations (multithreaded, external select, internal select)
 * call this function.
 * @remark To be called only from thread that process connection's
 * recv(), send() and response.
 *
 * @param connection connection to handle
 * @return #MHD_YES if we should continue to process the
 *         connection (not dead yet), #MHD_NO if it died
 */
enum MHD_Result
MHD_connection_handle_idle (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  enum MHD_Result ret;
#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (connection->tid) );
#endif /* MHD_USE_THREADS */
  /* 'daemon' is not used if epoll is not available and asserts are disabled */
  (void) daemon; /* Mute compiler warning */

  connection->in_idle = true;
  while (! connection->suspended)
  {
#ifdef HTTPS_SUPPORT
    if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
    {     /* HTTPS connection. */
      if ((MHD_TLS_CONN_INIT <= connection->tls_state) &&
          (MHD_TLS_CONN_CONNECTED > connection->tls_state))
        break;
    }
#endif /* HTTPS_SUPPORT */
#if DEBUG_STATES
    MHD_DLOG (daemon,
              _ ("In function %s handling connection at state: %s\n"),
              MHD_FUNC_,
              MHD_state_to_string (connection->state));
#endif
    switch (connection->state)
    {
    case MHD_CONNECTION_INIT:
    case MHD_CONNECTION_REQ_LINE_RECEIVING:
      if (get_request_line (connection))
      {
        mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING < connection->state);
        mhd_assert ((MHD_IS_HTTP_VER_SUPPORTED (connection->rq.http_ver)) \
                    || (connection->discard_request));
        continue;
      }
      mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVING >= connection->state);
      break;
    case MHD_CONNECTION_REQ_LINE_RECEIVED:
      switch_to_rq_headers_processing (connection);
      mhd_assert (MHD_CONNECTION_REQ_LINE_RECEIVED != connection->state);
      continue;
    case MHD_CONNECTION_REQ_HEADERS_RECEIVING:
      if (get_req_headers (connection, false))
      {
        mhd_assert (MHD_CONNECTION_REQ_HEADERS_RECEIVING < connection->state);
        mhd_assert ((MHD_CONNECTION_HEADERS_RECEIVED == connection->state) || \
                    (connection->discard_request));
        continue;
      }
      mhd_assert (MHD_CONNECTION_REQ_HEADERS_RECEIVING == connection->state);
      break;
    case MHD_CONNECTION_HEADERS_RECEIVED:
      parse_connection_headers (connection);
      if (MHD_CONNECTION_HEADERS_RECEIVED != connection->state)
        continue;
      connection->state = MHD_CONNECTION_HEADERS_PROCESSED;
      if (connection->suspended)
        break;
      continue;
    case MHD_CONNECTION_HEADERS_PROCESSED:
      call_connection_handler (connection);     /* first call */
      if (MHD_CONNECTION_HEADERS_PROCESSED != connection->state)
        continue;
      if (connection->suspended)
        continue;

      if ( (NULL == connection->rp.response) &&
           (need_100_continue (connection)) &&
           /* If the client is already sending the payload (body)
              there is no need to send "100 Continue" */
           (0 == connection->read_buffer_offset) )
      {
        connection->state = MHD_CONNECTION_CONTINUE_SENDING;
        break;
      }
      if ( (NULL != connection->rp.response) &&
           (0 != connection->rq.remaining_upload_size) )
      {
        /* we refused (no upload allowed!) */
        connection->rq.remaining_upload_size = 0;
        /* force close, in case client still tries to upload... */
        connection->discard_request = true;
      }
      connection->state = (0 == connection->rq.remaining_upload_size)
                          ? MHD_CONNECTION_FULL_REQ_RECEIVED
                          : MHD_CONNECTION_BODY_RECEIVING;
      if (connection->suspended)
        break;
      continue;
    case MHD_CONNECTION_CONTINUE_SENDING:
      if (connection->continue_message_write_offset ==
          MHD_STATICSTR_LEN_ (HTTP_100_CONTINUE))
      {
        connection->state = MHD_CONNECTION_BODY_RECEIVING;
        continue;
      }
      break;
    case MHD_CONNECTION_BODY_RECEIVING:
      mhd_assert (0 != connection->rq.remaining_upload_size);
      mhd_assert (! connection->discard_request);
      mhd_assert (NULL == connection->rp.response);
      if (0 != connection->read_buffer_offset)
      {
        process_request_body (connection);           /* loop call */
        if (MHD_CONNECTION_BODY_RECEIVING != connection->state)
          continue;
      }
      /* Modify here when queueing of the response during data processing
         will be supported */
      mhd_assert (! connection->discard_request);
      mhd_assert (NULL == connection->rp.response);
      if (0 == connection->rq.remaining_upload_size)
      {
        connection->state = MHD_CONNECTION_BODY_RECEIVED;
        continue;
      }
      break;
    case MHD_CONNECTION_BODY_RECEIVED:
      mhd_assert (! connection->discard_request);
      mhd_assert (NULL == connection->rp.response);
      if (0 == connection->rq.remaining_upload_size)
      {
        if (connection->rq.have_chunked_upload)
        {
          /* Reset counter variables reused for footers */
          connection->rq.num_cr_sp_replaced = 0;
          connection->rq.skipped_broken_lines = 0;
          reset_rq_header_processing_state (connection);
          connection->state = MHD_CONNECTION_FOOTERS_RECEIVING;
        }
        else
          connection->state = MHD_CONNECTION_FULL_REQ_RECEIVED;
        continue;
      }
      break;
    case MHD_CONNECTION_FOOTERS_RECEIVING:
      if (get_req_headers (connection, true))
      {
        mhd_assert (MHD_CONNECTION_FOOTERS_RECEIVING < connection->state);
        mhd_assert ((MHD_CONNECTION_FOOTERS_RECEIVED == connection->state) || \
                    (connection->discard_request));
        continue;
      }
      mhd_assert (MHD_CONNECTION_FOOTERS_RECEIVING == connection->state);
      break;
    case MHD_CONNECTION_FOOTERS_RECEIVED:
      /* The header, the body, and the footers of the request has been received,
       * switch to the final processing of the request. */
      connection->state = MHD_CONNECTION_FULL_REQ_RECEIVED;
      continue;
    case MHD_CONNECTION_FULL_REQ_RECEIVED:
      call_connection_handler (connection);     /* "final" call */
      if (connection->state != MHD_CONNECTION_FULL_REQ_RECEIVED)
        continue;
      if (NULL == connection->rp.response)
        break;                  /* try again next time */
      /* Response is ready, start reply */
      connection->state = MHD_CONNECTION_START_REPLY;
      continue;
    case MHD_CONNECTION_START_REPLY:
      mhd_assert (NULL != connection->rp.response);
      connection_switch_from_recv_to_send (connection);
      if (MHD_NO == build_header_response (connection))
      {
        /* oops - close! */
        CONNECTION_CLOSE_ERROR (connection,
                                _ ("Closing connection (failed to create "
                                   "response header).\n"));
        continue;
      }
      connection->state = MHD_CONNECTION_HEADERS_SENDING;
      break;

    case MHD_CONNECTION_HEADERS_SENDING:
      /* no default action */
      break;
    case MHD_CONNECTION_HEADERS_SENT:
#ifdef UPGRADE_SUPPORT
      if (NULL != connection->rp.response->upgrade_handler)
      {
        connection->state = MHD_CONNECTION_UPGRADE;
        /* This connection is "upgraded".  Pass socket to application. */
        if (MHD_NO ==
            MHD_response_execute_upgrade_ (connection->rp.response,
                                           connection))
        {
          /* upgrade failed, fail hard */
          CONNECTION_CLOSE_ERROR (connection,
                                  NULL);
          continue;
        }
        /* Response is not required anymore for this connection. */
        if (1)
        {
          struct MHD_Response *const resp = connection->rp.response;

          connection->rp.response = NULL;
          MHD_destroy_response (resp);
        }
        continue;
      }
#endif /* UPGRADE_SUPPORT */

      if (connection->rp.props.send_reply_body)
      {
        if (connection->rp.props.chunked)
          connection->state = MHD_CONNECTION_CHUNKED_BODY_UNREADY;
        else
          connection->state = MHD_CONNECTION_NORMAL_BODY_UNREADY;
      }
      else
        connection->state = MHD_CONNECTION_FULL_REPLY_SENT;
      continue;
    case MHD_CONNECTION_NORMAL_BODY_READY:
      mhd_assert (connection->rp.props.send_reply_body);
      mhd_assert (! connection->rp.props.chunked);
      /* nothing to do here */
      break;
    case MHD_CONNECTION_NORMAL_BODY_UNREADY:
      mhd_assert (connection->rp.props.send_reply_body);
      mhd_assert (! connection->rp.props.chunked);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != connection->rp.response->crc)
        MHD_mutex_lock_chk_ (&connection->rp.response->mutex);
#endif
      if (0 == connection->rp.response->total_size)
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        if (NULL != connection->rp.response->crc)
          MHD_mutex_unlock_chk_ (&connection->rp.response->mutex);
#endif
        if (connection->rp.props.chunked)
          connection->state = MHD_CONNECTION_CHUNKED_BODY_SENT;
        else
          connection->state = MHD_CONNECTION_FULL_REPLY_SENT;
        continue;
      }
      if (MHD_NO != try_ready_normal_body (connection))
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        if (NULL != connection->rp.response->crc)
          MHD_mutex_unlock_chk_ (&connection->rp.response->mutex);
#endif
        connection->state = MHD_CONNECTION_NORMAL_BODY_READY;
        /* Buffering for flushable socket was already enabled*/

        break;
      }
      /* mutex was already unlocked by "try_ready_normal_body */
      /* not ready, no socket action */
      break;
    case MHD_CONNECTION_CHUNKED_BODY_READY:
      mhd_assert (connection->rp.props.send_reply_body);
      mhd_assert (connection->rp.props.chunked);
      /* nothing to do here */
      break;
    case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
      mhd_assert (connection->rp.props.send_reply_body);
      mhd_assert (connection->rp.props.chunked);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != connection->rp.response->crc)
        MHD_mutex_lock_chk_ (&connection->rp.response->mutex);
#endif
      if ( (0 == connection->rp.response->total_size) ||
           (connection->rp.rsp_write_position ==
            connection->rp.response->total_size) )
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        if (NULL != connection->rp.response->crc)
          MHD_mutex_unlock_chk_ (&connection->rp.response->mutex);
#endif
        connection->state = MHD_CONNECTION_CHUNKED_BODY_SENT;
        continue;
      }
      if (1)
      { /* pseudo-branch for local variables scope */
        bool finished;
        if (MHD_NO != try_ready_chunked_body (connection, &finished))
        {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
          if (NULL != connection->rp.response->crc)
            MHD_mutex_unlock_chk_ (&connection->rp.response->mutex);
#endif
          connection->state = finished ? MHD_CONNECTION_CHUNKED_BODY_SENT :
                              MHD_CONNECTION_CHUNKED_BODY_READY;
          continue;
        }
        /* mutex was already unlocked by try_ready_chunked_body */
      }
      break;
    case MHD_CONNECTION_CHUNKED_BODY_SENT:
      mhd_assert (connection->rp.props.send_reply_body);
      mhd_assert (connection->rp.props.chunked);
      mhd_assert (connection->write_buffer_send_offset <= \
                  connection->write_buffer_append_offset);

      if (MHD_NO == build_connection_chunked_response_footer (connection))
      {
        /* oops - close! */
        CONNECTION_CLOSE_ERROR (connection,
                                _ ("Closing connection (failed to create " \
                                   "response footer)."));
        continue;
      }
      mhd_assert (connection->write_buffer_send_offset < \
                  connection->write_buffer_append_offset);
      connection->state = MHD_CONNECTION_FOOTERS_SENDING;
      continue;
    case MHD_CONNECTION_FOOTERS_SENDING:
      mhd_assert (connection->rp.props.send_reply_body);
      mhd_assert (connection->rp.props.chunked);
      /* no default action */
      break;
    case MHD_CONNECTION_FULL_REPLY_SENT:
      if (MHD_HTTP_PROCESSING == connection->rp.responseCode)
      {
        /* After this type of response, we allow sending another! */
        connection->state = MHD_CONNECTION_HEADERS_PROCESSED;
        MHD_destroy_response (connection->rp.response);
        connection->rp.response = NULL;
        /* FIXME: maybe partially reset memory pool? */
        continue;
      }
      /* Reset connection after complete reply */
      connection_reset (connection,
                        MHD_CONN_USE_KEEPALIVE == connection->keepalive &&
                        ! connection->read_closed &&
                        ! connection->discard_request);
      continue;
    case MHD_CONNECTION_CLOSED:
      cleanup_connection (connection);
      connection->in_idle = false;
      return MHD_NO;
#ifdef UPGRADE_SUPPORT
    case MHD_CONNECTION_UPGRADE:
      connection->in_idle = false;
      return MHD_YES;     /* keep open */
#endif /* UPGRADE_SUPPORT */
    default:
      mhd_assert (0);
      break;
    }
    break;
  }
  if (connection_check_timedout (connection))
  {
    MHD_connection_close_ (connection,
                           MHD_REQUEST_TERMINATED_TIMEOUT_REACHED);
    connection->in_idle = false;
    return MHD_YES;
  }
  MHD_connection_update_event_loop_info (connection);
  ret = MHD_YES;
#ifdef EPOLL_SUPPORT
  if ( (! connection->suspended) &&
       MHD_D_IS_USING_EPOLL_ (daemon) )
  {
    ret = MHD_connection_epoll_update_ (connection);
  }
#endif /* EPOLL_SUPPORT */
  connection->in_idle = false;
  return ret;
}


#ifdef EPOLL_SUPPORT
/**
 * Perform epoll() processing, possibly moving the connection back into
 * the epoll() set if needed.
 *
 * @param connection connection to process
 * @return #MHD_YES if we should continue to process the
 *         connection (not dead yet), #MHD_NO if it died
 */
enum MHD_Result
MHD_connection_epoll_update_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *const daemon = connection->daemon;

  mhd_assert (MHD_D_IS_USING_EPOLL_ (daemon));

  if ((0 != (MHD_EVENT_LOOP_INFO_PROCESS & connection->event_loop_info)) &&
      (0 == (connection->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL)))
  {
    /* Make sure that connection waiting for processing will be processed */
    EDLL_insert (daemon->eready_head,
                 daemon->eready_tail,
                 connection);
    connection->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
  }

  if ( (0 == (connection->epoll_state & MHD_EPOLL_STATE_IN_EPOLL_SET)) &&
       (0 == (connection->epoll_state & MHD_EPOLL_STATE_SUSPENDED)) &&
       ( ( (MHD_EVENT_LOOP_INFO_WRITE == connection->event_loop_info) &&
           (0 == (connection->epoll_state & MHD_EPOLL_STATE_WRITE_READY))) ||
         ( (0 != (MHD_EVENT_LOOP_INFO_READ & connection->event_loop_info)) &&
           (0 == (connection->epoll_state & MHD_EPOLL_STATE_READ_READY)) ) ) )
  {
    /* add to epoll set */
    struct epoll_event event;

    event.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET;
    event.data.ptr = connection;
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_ADD,
                        connection->socket_fd,
                        &event))
    {
#ifdef HAVE_MESSAGES
      if (0 != (daemon->options & MHD_USE_ERROR_LOG))
        MHD_DLOG (daemon,
                  _ ("Call to epoll_ctl failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
      connection->state = MHD_CONNECTION_CLOSED;
      cleanup_connection (connection);
      return MHD_NO;
    }
    connection->epoll_state |= MHD_EPOLL_STATE_IN_EPOLL_SET;
  }
  return MHD_YES;
}


#endif


/**
 * Set callbacks for this connection to those for HTTP.
 *
 * @param connection connection to initialize
 */
void
MHD_set_http_callbacks_ (struct MHD_Connection *connection)
{
  connection->recv_cls = &recv_param_adapter;
}


/**
 * Obtain information about the given connection.
 * The returned pointer is invalidated with the next call of this function or
 * when the connection is closed.
 *
 * @param connection what connection to get information about
 * @param info_type what information is desired?
 * @param ... depends on @a info_type
 * @return NULL if this information is not available
 *         (or if the @a info_type is unknown)
 * @ingroup specialized
 */
_MHD_EXTERN const union MHD_ConnectionInfo *
MHD_get_connection_info (struct MHD_Connection *connection,
                         enum MHD_ConnectionInfoType info_type,
                         ...)
{
  switch (info_type)
  {
#ifdef HTTPS_SUPPORT
  case MHD_CONNECTION_INFO_CIPHER_ALGO:
    if (NULL == connection->tls_session)
      return NULL;
    if (1)
    { /* Workaround to mute compiler warning */
      gnutls_cipher_algorithm_t res;
      res = gnutls_cipher_get (connection->tls_session);
      connection->connection_info_dummy.cipher_algorithm = (int) res;
    }
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_PROTOCOL:
    if (NULL == connection->tls_session)
      return NULL;
    if (1)
    { /* Workaround to mute compiler warning */
      gnutls_protocol_t res;
      res = gnutls_protocol_get_version (connection->tls_session);
      connection->connection_info_dummy.protocol = (int) res;
    }
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_GNUTLS_SESSION:
    if (NULL == connection->tls_session)
      return NULL;
    connection->connection_info_dummy.tls_session = connection->tls_session;
    return &connection->connection_info_dummy;
#else  /* ! HTTPS_SUPPORT */
  case MHD_CONNECTION_INFO_CIPHER_ALGO:
  case MHD_CONNECTION_INFO_PROTOCOL:
  case MHD_CONNECTION_INFO_GNUTLS_SESSION:
#endif /* ! HTTPS_SUPPORT */
  case MHD_CONNECTION_INFO_GNUTLS_CLIENT_CERT:
    return NULL; /* Not implemented */
  case MHD_CONNECTION_INFO_CLIENT_ADDRESS:
    if (0 < connection->addr_len)
    {
      mhd_assert (sizeof (connection->addr) == \
                  sizeof (connection->connection_info_dummy.client_addr));
      memcpy (&connection->connection_info_dummy.client_addr,
              &connection->addr,
              sizeof(connection->addr));
      return &connection->connection_info_dummy;
    }
    return NULL;
  case MHD_CONNECTION_INFO_DAEMON:
    connection->connection_info_dummy.daemon =
      MHD_get_master (connection->daemon);
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_CONNECTION_FD:
    connection->connection_info_dummy.connect_fd = connection->socket_fd;
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_SOCKET_CONTEXT:
    connection->connection_info_dummy.socket_context =
      connection->socket_context;
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_CONNECTION_SUSPENDED:
    connection->connection_info_dummy.suspended =
      connection->suspended ? MHD_YES : MHD_NO;
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_CONNECTION_TIMEOUT:
#if SIZEOF_UNSIGNED_INT <= (SIZEOF_UINT64_T - 2)
    if (UINT_MAX < connection->connection_timeout_ms / 1000)
      connection->connection_info_dummy.connection_timeout = UINT_MAX;
    else
#endif /* SIZEOF_UNSIGNED_INT <=(SIZEOF_UINT64_T - 2) */
    connection->connection_info_dummy.connection_timeout =
      (unsigned int) (connection->connection_timeout_ms / 1000);
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_REQUEST_HEADER_SIZE:
    if ( (MHD_CONNECTION_HEADERS_RECEIVED > connection->state) ||
         (MHD_CONNECTION_CLOSED == connection->state) )
      return NULL;   /* invalid, too early! */
    connection->connection_info_dummy.header_size = connection->rq.header_size;
    return &connection->connection_info_dummy;
  case MHD_CONNECTION_INFO_HTTP_STATUS:
    if (NULL == connection->rp.response)
      return NULL;
    connection->connection_info_dummy.http_status = connection->rp.responseCode;
    return &connection->connection_info_dummy;
  default:
    return NULL;
  }
}


/**
 * Set a custom option for the given connection, overriding defaults.
 *
 * @param connection connection to modify
 * @param option option to set
 * @param ... arguments to the option, depending on the option type
 * @return #MHD_YES on success, #MHD_NO if setting the option failed
 * @ingroup specialized
 */
_MHD_EXTERN enum MHD_Result
MHD_set_connection_option (struct MHD_Connection *connection,
                           enum MHD_CONNECTION_OPTION option,
                           ...)
{
  va_list ap;
  struct MHD_Daemon *daemon;
  unsigned int ui_val;

  daemon = connection->daemon;
  switch (option)
  {
  case MHD_CONNECTION_OPTION_TIMEOUT:
    if (0 == connection->connection_timeout_ms)
      connection->last_activity = MHD_monotonic_msec_counter ();
    va_start (ap, option);
    ui_val = va_arg (ap, unsigned int);
    va_end (ap);
#if (SIZEOF_UINT64_T - 2) <= SIZEOF_UNSIGNED_INT
    if ((UINT64_MAX / 4000 - 1) < ui_val)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("The specified connection timeout (%u) is too " \
                   "large. Maximum allowed value (%" PRIu64 ") will be used " \
                   "instead.\n"),
                ui_val,
                (UINT64_MAX / 4000 - 1));
#endif
      ui_val = UINT64_MAX / 4000 - 1;
    }
#endif /* (SIZEOF_UINT64_T - 2) <= SIZEOF_UNSIGNED_INT */
    if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    {
#if defined(MHD_USE_THREADS)
      MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
      if (! connection->suspended)
      {
        if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
          XDLL_remove (daemon->normal_timeout_head,
                       daemon->normal_timeout_tail,
                       connection);
        else
          XDLL_remove (daemon->manual_timeout_head,
                       daemon->manual_timeout_tail,
                       connection);
        connection->connection_timeout_ms = ((uint64_t) ui_val) * 1000;
        if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
          XDLL_insert (daemon->normal_timeout_head,
                       daemon->normal_timeout_tail,
                       connection);
        else
          XDLL_insert (daemon->manual_timeout_head,
                       daemon->manual_timeout_tail,
                       connection);
      }
#if defined(MHD_USE_THREADS)
      MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
    }
    return MHD_YES;
  default:
    return MHD_NO;
  }
}


/**
 * Queue a response to be transmitted to the client (as soon as
 * possible but after #MHD_AccessHandlerCallback returns).
 *
 * For any active connection this function must be called
 * only by #MHD_AccessHandlerCallback callback.
 *
 * For suspended connection this function can be called at any moment (this
 * behaviour is deprecated and will be removed!). Response  will be sent
 * as soon as connection is resumed.
 *
 * For single thread environment, when MHD is used in "external polling" mode
 * (without MHD_USE_SELECT_INTERNALLY) this function can be called any
 * time (this behaviour is deprecated and will be removed!).
 *
 * If HTTP specifications require use no body in reply, like @a status_code with
 * value 1xx, the response body is automatically not sent even if it is present
 * in the response. No "Content-Length" or "Transfer-Encoding" headers are
 * generated and added.
 *
 * When the response is used to respond HEAD request or used with @a status_code
 * #MHD_HTTP_NOT_MODIFIED, then response body is not sent, but "Content-Length"
 * header is added automatically based the size of the body in the response.
 * If body size it set to #MHD_SIZE_UNKNOWN or chunked encoding is enforced
 * then "Transfer-Encoding: chunked" header (for HTTP/1.1 only) is added instead
 * of "Content-Length" header. For example, if response with zero-size body is
 * used for HEAD request, then "Content-Length: 0" is added automatically to
 * reply headers.
 * @sa #MHD_RF_HEAD_ONLY_RESPONSE
 *
 * In situations, where reply body is required, like answer for the GET request
 * with @a status_code #MHD_HTTP_OK, headers "Content-Length" (for known body
 * size) or "Transfer-Encoding: chunked" (for #MHD_SIZE_UNKNOWN with HTTP/1.1)
 * are added automatically.
 * In practice, the same response object can be used to respond to both HEAD and
 * GET requests.
 *
 * @param connection the connection identifying the client
 * @param status_code HTTP status code (i.e. #MHD_HTTP_OK)
 * @param response response to transmit, the NULL is tolerated
 * @return #MHD_NO on error (reply already sent, response is NULL),
 *         #MHD_YES on success or if message has been queued
 * @ingroup response
 * @sa #MHD_AccessHandlerCallback
 */
_MHD_EXTERN enum MHD_Result
MHD_queue_response (struct MHD_Connection *connection,
                    unsigned int status_code,
                    struct MHD_Response *response)
{
  struct MHD_Daemon *daemon;
  bool reply_icy;

  if ((NULL == connection) || (NULL == response))
    return MHD_NO;

  daemon = connection->daemon;
  if ((! connection->in_access_handler) && (! connection->suspended) &&
      MHD_D_IS_USING_THREADS_ (daemon))
    return MHD_NO;

  reply_icy = (0 != (status_code & MHD_ICY_FLAG));
  status_code &= ~MHD_ICY_FLAG;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if ( (! connection->suspended) &&
       MHD_D_IS_USING_THREADS_ (daemon) &&
       (! MHD_thread_handle_ID_is_current_thread_ (connection->tid)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Attempted to queue response on wrong thread!\n"));
#endif
    return MHD_NO;
  }
#endif

  if (NULL != connection->rp.response)
    return MHD_NO; /* The response was already set */

  if ( (MHD_CONNECTION_HEADERS_PROCESSED != connection->state) &&
       (MHD_CONNECTION_FULL_REQ_RECEIVED != connection->state) )
    return MHD_NO; /* Wrong connection state */

  if (daemon->shutdown)
    return MHD_NO;

#ifdef UPGRADE_SUPPORT
  if (NULL != response->upgrade_handler)
  {
    struct MHD_HTTP_Res_Header *conn_header;
    if (0 == (daemon->options & MHD_ALLOW_UPGRADE))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Attempted 'upgrade' connection on daemon without" \
                   " MHD_ALLOW_UPGRADE option!\n"));
#endif
      return MHD_NO;
    }
    if (MHD_HTTP_SWITCHING_PROTOCOLS != status_code)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used invalid status code for" \
                   " 'upgrade' response!\n"));
#endif
      return MHD_NO;
    }
    if (0 == (response->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used invalid response" \
                   " without \"Connection\" header!\n"));
#endif
      return MHD_NO;
    }
    conn_header = response->first_header;
    mhd_assert (NULL != conn_header);
    mhd_assert (MHD_str_equal_caseless_ (conn_header->header,
                                         MHD_HTTP_HEADER_CONNECTION));
    if (! MHD_str_has_s_token_caseless_ (conn_header->value,
                                         "upgrade"))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used invalid response" \
                   " without \"upgrade\" token in" \
                   " \"Connection\" header!\n"));
#endif
      return MHD_NO;
    }
    if (! MHD_IS_HTTP_VER_1_1_COMPAT (connection->rq.http_ver))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Connection \"Upgrade\" can be used only " \
                   "with HTTP/1.1 connections!\n"));
#endif
      return MHD_NO;
    }
  }
#endif /* UPGRADE_SUPPORT */
  if (MHD_HTTP_SWITCHING_PROTOCOLS == status_code)
  {
#ifdef UPGRADE_SUPPORT
    if (NULL == response->upgrade_handler)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used status code 101 \"Switching Protocols\" " \
                   "with non-'upgrade' response!\n"));
#endif /* HAVE_MESSAGES */
      return MHD_NO;
    }
#else  /* ! UPGRADE_SUPPORT */
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Application used status code 101 \"Switching Protocols\", " \
                 "but this MHD was built without \"Upgrade\" support!\n"));
#endif /* HAVE_MESSAGES */
    return MHD_NO;
#endif /* ! UPGRADE_SUPPORT */
  }
  if ( (100 > status_code) ||
       (999 < status_code) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Refused wrong status code (%u). " \
                 "HTTP requires three digits status code!\n"),
              status_code);
#endif
    return MHD_NO;
  }
  if (200 > status_code)
  {
    if (MHD_HTTP_VER_1_0 == connection->rq.http_ver)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Wrong status code (%u) refused. " \
                   "HTTP/1.0 clients do not support 1xx status codes!\n"),
                (status_code));
#endif
      return MHD_NO;
    }
    if (0 != (response->flags & (MHD_RF_HTTP_1_0_COMPATIBLE_STRICT
                                 | MHD_RF_HTTP_1_0_SERVER)))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Wrong status code (%u) refused. " \
                   "HTTP/1.0 reply mode does not support 1xx status codes!\n"),
                (status_code));
#endif
      return MHD_NO;
    }
  }
  if ( (MHD_HTTP_MTHD_CONNECT == connection->rq.http_mthd) &&
       (2 == status_code / 100) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Successful (%u) response code cannot be used to answer " \
                 "\"CONNECT\" request!\n"),
              (status_code));
#endif
    return MHD_NO;
  }

  if ( (0 != (MHD_RF_HEAD_ONLY_RESPONSE & response->flags)) &&
       (RP_BODY_HEADERS_ONLY < is_reply_body_needed (connection, status_code)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("HEAD-only response cannot be used when the request requires "
                 "reply body to be sent!\n"));
#endif
    return MHD_NO;
  }

#ifdef HAVE_MESSAGES
  if ( (0 != (MHD_RF_INSANITY_HEADER_CONTENT_LENGTH & response->flags)) &&
       (0 != (MHD_RAF_HAS_CONTENT_LENGTH & response->flags_auto)) )
  {
    MHD_DLOG (daemon,
              _ ("The response has application-defined \"Content-Length\" " \
                 "header. The reply to the request will be not " \
                 "HTTP-compliant and may result in hung connection or " \
                 "other problems!\n"));
  }
#endif

  MHD_increment_response_rc (response);
  connection->rp.response = response;
  connection->rp.responseCode = status_code;
  connection->rp.responseIcy = reply_icy;
#if defined(_MHD_HAVE_SENDFILE)
  if ( (response->fd == -1) ||
       (response->is_pipe) ||
       (0 != (connection->daemon->options & MHD_USE_TLS))
#if defined(MHD_SEND_SPIPE_SUPPRESS_NEEDED) && \
       defined(MHD_SEND_SPIPE_SUPPRESS_POSSIBLE)
       || (! daemon->sigpipe_blocked && ! connection->sk_spipe_suppress)
#endif /* MHD_SEND_SPIPE_SUPPRESS_NEEDED &&
          MHD_SEND_SPIPE_SUPPRESS_POSSIBLE */
       )
    connection->rp.resp_sender = MHD_resp_sender_std;
  else
    connection->rp.resp_sender = MHD_resp_sender_sendfile;
#endif /* _MHD_HAVE_SENDFILE */
  /* FIXME: if 'is_pipe' is set, TLS is off, and we have *splice*, we could use splice()
     to avoid two user-space copies... */

  if ( (MHD_HTTP_MTHD_HEAD == connection->rq.http_mthd) ||
       (MHD_HTTP_OK > status_code) ||
       (MHD_HTTP_NO_CONTENT == status_code) ||
       (MHD_HTTP_NOT_MODIFIED == status_code) )
  {
    /* if this is a "HEAD" request, or a status code for
       which a body is not allowed, pretend that we
       have already sent the full message body. */
    /* TODO: remove the next assignment, use 'rp_props.send_reply_body' in
     * checks */
    connection->rp.rsp_write_position = response->total_size;
  }
  if (MHD_CONNECTION_HEADERS_PROCESSED == connection->state)
  {
    /* response was queued "early", refuse to read body / footers or
       further requests! */
    connection->discard_request = true;
    connection->state = MHD_CONNECTION_START_REPLY;
    connection->rq.remaining_upload_size = 0;
  }
  if (! connection->in_idle)
    (void) MHD_connection_handle_idle (connection);
  MHD_update_last_activity_ (connection);
  return MHD_YES;
}


/* end of connection.c */
