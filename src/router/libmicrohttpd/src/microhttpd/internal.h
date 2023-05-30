/*
  This file is part of libmicrohttpd
  Copyright (C) 2007-2018 Daniel Pittman and Christian Grothoff
  Copyright (C) 2014-2021 Evgeny Grin (Karlson2k)

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
 * @file microhttpd/internal.h
 * @brief  MHD internal shared structures
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef INTERNAL_H
#define INTERNAL_H

#include "mhd_options.h"
#include "platform.h"
#include "microhttpd.h"
#include "mhd_assert.h"

#ifdef HTTPS_SUPPORT
#include <gnutls/gnutls.h>
#if GNUTLS_VERSION_MAJOR >= 3
#include <gnutls/abstract.h>
#endif
#endif /* HTTPS_SUPPORT */

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif /* HAVE_INTTYPES_H */

#ifndef PRIu64
#define PRIu64  "llu"
#endif /* ! PRIu64 */

#ifdef MHD_PANIC
/* Override any defined MHD_PANIC macro with proper one */
#undef MHD_PANIC
#endif /* MHD_PANIC */

#ifdef HAVE_MESSAGES
/**
 * Trigger 'panic' action based on fatal errors.
 *
 * @param msg error message (const char *)
 */
#define MHD_PANIC(msg) do { mhd_panic (mhd_panic_cls, __FILE__, __LINE__, msg); \
                            BUILTIN_NOT_REACHED; } while (0)
#else
/**
 * Trigger 'panic' action based on fatal errors.
 *
 * @param msg error message (const char *)
 */
#define MHD_PANIC(msg) do { mhd_panic (mhd_panic_cls, __FILE__, __LINE__, NULL); \
                            BUILTIN_NOT_REACHED; } while (0)
#endif

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#include "mhd_threads.h"
#endif
#include "mhd_locks.h"
#include "mhd_sockets.h"
#include "mhd_itc_types.h"


/**
 * @def _MHD_MACRO_NO
 * "Negative answer"/"false" for use in macros, meaningful for precompiler
 */
#define _MHD_MACRO_NO   0

/**
 * @def _MHD_MACRO_YES
 * "Positive answer"/"true" for use in macros, meaningful for precompiler
 */
#define _MHD_MACRO_YES  1

/**
 * Close FD and abort execution if error is detected.
 * @param fd the FD to close
 */
#define MHD_fd_close_chk_(fd) do {                      \
    if ( (0 != close ((fd)) && (EBADF == errno)) ) {    \
      MHD_PANIC (_ ("Failed to close FD.\n"));          \
    }                                                   \
} while (0)

/*
#define EXTRA_CHECKS _MHD_MACRO_NO
 * Not used. Behaviour is controlled by _DEBUG/NDEBUG macros.
 */

#ifndef _MHD_DEBUG_CONNECT
/**
 * Print extra messages when establishing
 * connections? (only adds non-error messages).
 */
#define _MHD_DEBUG_CONNECT _MHD_MACRO_NO
#endif /* ! _MHD_DEBUG_CONNECT */

#ifndef _MHD_DEBUG_SEND_DATA
/**
 * Should all data send be printed to stderr?
 */
#define _MHD_DEBUG_SEND_DATA _MHD_MACRO_NO
#endif /* ! _MHD_DEBUG_SEND_DATA */

#ifndef _MHD_DEBUG_CLOSE
/**
 * Add extra debug messages with reasons for closing connections
 * (non-error reasons).
 */
#define _MHD_DEBUG_CLOSE _MHD_MACRO_NO
#endif /* ! _MHD_DEBUG_CLOSE */

#define MHD_MAX(a,b) (((a)<(b)) ? (b) : (a))
#define MHD_MIN(a,b) (((a)<(b)) ? (a) : (b))


/**
 * Minimum reasonable size by which MHD tries to increment read/write buffers.
 * We usually begin with half the available pool space for the
 * IO-buffer, but if absolutely needed we additively grow by the
 * number of bytes given here (up to -- theoretically -- the full pool
 * space).
 */
#define MHD_BUF_INC_SIZE 1024


/**
 * Handler for fatal errors.
 */
extern MHD_PanicCallback mhd_panic;

/**
 * Closure argument for "mhd_panic".
 */
extern void *mhd_panic_cls;

/* If we have Clang or gcc >= 4.5, use __builtin_unreachable() */
#if defined(__clang__) || (__GNUC__ > 4) || \
  (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define BUILTIN_NOT_REACHED __builtin_unreachable ()
#elif defined(_MSC_FULL_VER)
#define BUILTIN_NOT_REACHED __assume (0)
#else
#define BUILTIN_NOT_REACHED
#endif

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */


/**
 * Tri-state on/off/unknown
 */
enum MHD_tristate
{
  _MHD_UNKNOWN = -1,    /**< State is not yet checked nor set */
  _MHD_OFF     = false, /**< State is "off" / "disabled" */
  _MHD_NO      = false, /**< State is "off" / "disabled" */
  _MHD_ON      = true,  /**< State is "on"  / "enabled" */
  _MHD_YES     = true   /**< State is "on"  / "enabled" */
} _MHD_FIXED_ENUM;


/**
 * State of the socket with respect to epoll (bitmask).
 */
enum MHD_EpollState
{

  /**
   * The socket is not involved with a defined state in epoll() right
   * now.
   */
  MHD_EPOLL_STATE_UNREADY = 0,

  /**
   * epoll() told us that data was ready for reading, and we did
   * not consume all of it yet.
   */
  MHD_EPOLL_STATE_READ_READY = 1,

  /**
   * epoll() told us that space was available for writing, and we did
   * not consume all of it yet.
   */
  MHD_EPOLL_STATE_WRITE_READY = 2,

  /**
   * Is this connection currently in the 'eready' EDLL?
   */
  MHD_EPOLL_STATE_IN_EREADY_EDLL = 4,

  /**
   * Is this connection currently in the epoll() set?
   */
  MHD_EPOLL_STATE_IN_EPOLL_SET = 8,

  /**
   * Is this connection currently suspended?
   */
  MHD_EPOLL_STATE_SUSPENDED = 16,

  /**
   * Is this connection in some error state?
   */
  MHD_EPOLL_STATE_ERROR = 128
} _MHD_FIXED_FLAGS_ENUM;


/**
 * What is this connection waiting for?
 */
enum MHD_ConnectionEventLoopInfo
{
  /**
   * We are waiting to be able to read.
   */
  MHD_EVENT_LOOP_INFO_READ = 0,

  /**
   * We are waiting to be able to write.
   */
  MHD_EVENT_LOOP_INFO_WRITE = 1,

  /**
   * We are waiting for the application to provide data.
   */
  MHD_EVENT_LOOP_INFO_BLOCK = 2,

  /**
   * We are finished and are awaiting cleanup.
   */
  MHD_EVENT_LOOP_INFO_CLEANUP = 3
} _MHD_FIXED_ENUM;


/**
 * Additional test value for enum MHD_FLAG to check only for MHD_ALLOW_SUSPEND_RESUME and
 * NOT for MHD_USE_ITC.
 */
#define MHD_TEST_ALLOW_SUSPEND_RESUME 8192

/**
 * Maximum length of a nonce in digest authentication.  64(SHA-256 Hex) +
 * 8(Timestamp Hex) + 1(NULL); hence 73 should suffice, but Opera
 * (already) takes more (see Mantis #1633), so we've increased the
 * value to support something longer...
 */
#define MAX_NONCE_LENGTH 129


/**
 * A structure representing the internal holder of the
 * nonce-nc map.
 */
struct MHD_NonceNc
{

  /**
   * Nonce counter, a value that increases for each subsequent
   * request for the same nonce.
   */
  uint64_t nc;

  /**
   * Bitmask over the nc-64 previous nonce values.  Used to
   * allow out-of-order nonces.
   */
  uint64_t nmask;

  /**
   * Nonce value:
   */
  char nonce[MAX_NONCE_LENGTH];

};

#ifdef HAVE_MESSAGES
/**
 * fprintf()-like helper function for logging debug
 * messages.
 */
void
MHD_DLOG (const struct MHD_Daemon *daemon,
          const char *format,
          ...);

#endif


/**
 * Header or cookie in HTTP request or response.
 */
struct MHD_HTTP_Header
{
  /**
   * Headers are kept in a double-linked list.
   */
  struct MHD_HTTP_Header *next;

  /**
   * Headers are kept in a double-linked list.
   */
  struct MHD_HTTP_Header *prev;

  /**
   * The name of the header (key), without the colon.
   */
  char *header;

  /**
   * The length of the @a header, not including the final zero termination.
   */
  size_t header_size;

  /**
   * The value of the header.
   */
  char *value;

  /**
   * The length of the @a value, not including the final zero termination.
   */
  size_t value_size;

  /**
   * Type of the header (where in the HTTP protocol is this header
   * from).
   */
  enum MHD_ValueKind kind;

};


/**
 * Automatically assigned flags
 */
enum MHD_ResponseAutoFlags
{
  MHD_RAF_NO_FLAGS = 0,                   /**< No auto flags */
  MHD_RAF_HAS_CONNECTION_HDR = 1 << 0,    /**< Has "Connection" header */
  MHD_RAF_HAS_CONNECTION_CLOSE = 1 << 1,  /**< Has "Connection: close" */
  MHD_RAF_HAS_TRANS_ENC_CHUNKED = 1 << 2, /**< Has "Transfer-Encoding: chunked */
  MHD_RAF_HAS_DATE_HDR = 1 << 3           /**< Has "Date" header */
} _MHD_FIXED_FLAGS_ENUM;


#if defined(MHD_WINSOCK_SOCKETS)
/**
 * Internally used I/O vector type for use with winsock.
 * Binary matches system "WSABUF".
 */
typedef struct _MHD_W32_iovec
{
  unsigned long iov_len;
  char *iov_base;
} MHD_iovec_;
#define MHD_IOV_ELMN_MAX_SIZE    ULONG_MAX
typedef unsigned long MHD_iov_size_;
#elif defined(HAVE_SENDMSG) || defined(HAVE_WRITEV)
/**
 * Internally used I/O vector type for use when writev or sendmsg
 * is available. Matches system "struct iovec".
 */
typedef struct iovec MHD_iovec_;
#define MHD_IOV_ELMN_MAX_SIZE    SIZE_MAX
typedef size_t MHD_iov_size_;
#else
/**
 * Internally used I/O vector type for use when writev or sendmsg
 * is not available.
 */
typedef struct MHD_IoVec MHD_iovec_;
#define MHD_IOV_ELMN_MAX_SIZE    SIZE_MAX
typedef size_t MHD_iov_size_;
#endif


struct MHD_iovec_track_
{
  /**
   * The copy of array of iovec elements.
   * The copy of elements are updated during sending.
   * The number of elements is not changed during lifetime.
   */
  MHD_iovec_ *iov;

  /**
   * The number of elements in @iov.
   * This value is not changed during lifetime.
   */
  size_t cnt;

  /**
   * The number of sent elements.
   * At the same time, it is the index of the next (or current) element
   * to send.
   */
  size_t sent;
};

/**
 * Representation of a response.
 */
struct MHD_Response
{

  /**
   * Head of double-linked list of headers to send for the response.
   */
  struct MHD_HTTP_Header *first_header;

  /**
   * Tail of double-linked list of headers to send for the response.
   */
  struct MHD_HTTP_Header *last_header;

  /**
   * Buffer pointing to data that we are supposed
   * to send as a response.
   */
  char *data;

  /**
   * Closure to give to the content reader @e crc
   * and content reader free callback @e crfc.
   */
  void *crc_cls;

  /**
   * How do we get more data?  NULL if we are
   * given all of the data up front.
   */
  MHD_ContentReaderCallback crc;

  /**
   * NULL if data must not be freed, otherwise
   * either user-specified callback or "&free".
   */
  MHD_ContentReaderFreeCallback crfc;

#ifdef UPGRADE_SUPPORT
  /**
   * Application function to call once we are done sending the headers
   * of the response; NULL unless this is a response created with
   * #MHD_create_response_for_upgrade().
   */
  MHD_UpgradeHandler upgrade_handler;

  /**
   * Closure for @e uh.
   */
  void *upgrade_handler_cls;
#endif /* UPGRADE_SUPPORT */

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /**
   * Mutex to synchronize access to @e data, @e size and
   * @e reference_count.
   */
  MHD_mutex_ mutex;
#endif

  /**
   * The size of the response body.
   * Set to #MHD_SIZE_UNKNOWN if size is not known.
   */
  uint64_t total_size;

  /**
   * At what offset in the stream is the
   * beginning of @e data located?
   */
  uint64_t data_start;

  /**
   * Offset to start reading from when using @e fd.
   */
  uint64_t fd_off;

  /**
   * Number of bytes ready in @e data (buffer may be larger
   * than what is filled with payload).
   */
  size_t data_size;

  /**
   * Size of the writable data buffer @e data.
   */
  size_t data_buffer_size;

  /**
   * Reference count for this response.  Free once the counter hits
   * zero.
   */
  unsigned int reference_count;

  /**
   * File-descriptor if this response is FD-backed.
   */
  int fd;

  /**
   * Flags set for the MHD response.
   */
  enum MHD_ResponseFlags flags;

  /**
   * Automatic flags set for the MHD response.
   */
  enum MHD_ResponseAutoFlags flags_auto;

  /**
   * If the @e fd is a pipe (no sendfile()).
   */
  bool is_pipe;

  /**
   * I/O vector used with MHD_create_response_from_iovec.
   */
  MHD_iovec_ *data_iov;

  /**
   * Number of elements in data_iov.
   */
  unsigned int data_iovcnt;
};


/**
 * States in a state machine for a connection.
 *
 * The main transitions are any-state to #MHD_CONNECTION_CLOSED, any
 * state to state+1, #MHD_CONNECTION_FOOTERS_SENT to
 * #MHD_CONNECTION_INIT.  #MHD_CONNECTION_CLOSED is the terminal state
 * and #MHD_CONNECTION_INIT the initial state.
 *
 * Note that transitions for *reading* happen only after the input has
 * been processed; transitions for *writing* happen after the
 * respective data has been put into the write buffer (the write does
 * not have to be completed yet).  A transition to
 * #MHD_CONNECTION_CLOSED or #MHD_CONNECTION_INIT requires the write
 * to be complete.
 */
enum MHD_CONNECTION_STATE
{
  /**
   * Connection just started (no headers received).
   * Waiting for the line with the request type, URL and version.
   */
  MHD_CONNECTION_INIT = 0,

  /**
   * Part of the request line was received.
   * Wait for complete line.
   */
  MHD_CONNECTION_REQ_LINE_RECEIVING = MHD_CONNECTION_INIT + 1,

  /**
   * We got the URL (and request type and version).  Wait for a header line.
   */
  MHD_CONNECTION_URL_RECEIVED = MHD_CONNECTION_REQ_LINE_RECEIVING + 1,

  /**
   * We got part of a multi-line request header.  Wait for the rest.
   */
  MHD_CONNECTION_HEADER_PART_RECEIVED = MHD_CONNECTION_URL_RECEIVED + 1,

  /**
   * We got the request headers.  Process them.
   */
  MHD_CONNECTION_HEADERS_RECEIVED = MHD_CONNECTION_HEADER_PART_RECEIVED + 1,

  /**
   * We have processed the request headers.  Send 100 continue.
   */
  MHD_CONNECTION_HEADERS_PROCESSED = MHD_CONNECTION_HEADERS_RECEIVED + 1,

  /**
   * We have processed the headers and need to send 100 CONTINUE.
   */
  MHD_CONNECTION_CONTINUE_SENDING = MHD_CONNECTION_HEADERS_PROCESSED + 1,

  /**
   * We have sent 100 CONTINUE (or do not need to).  Read the message body.
   */
  MHD_CONNECTION_CONTINUE_SENT = MHD_CONNECTION_CONTINUE_SENDING + 1,

  /**
   * We got the request body.  Wait for a line of the footer.
   */
  MHD_CONNECTION_BODY_RECEIVED = MHD_CONNECTION_CONTINUE_SENT + 1,

  /**
   * We got part of a line of the footer.  Wait for the
   * rest.
   */
  MHD_CONNECTION_FOOTER_PART_RECEIVED = MHD_CONNECTION_BODY_RECEIVED + 1,

  /**
   * We received the entire footer.
   */
  MHD_CONNECTION_FOOTERS_RECEIVED = MHD_CONNECTION_FOOTER_PART_RECEIVED + 1,

  /**
   * We received the entire request.
   * Wait for a response to be queued.
   */
  MHD_CONNECTION_FULL_REQ_RECEIVED = MHD_CONNECTION_FOOTERS_RECEIVED + 1,

  /**
   * Finished reading of the request and the response is ready.
   * Switch internal logic from receiving to sending, prepare connection
   * sending the reply and build the reply header.
   */
  MHD_CONNECTION_START_REPLY = MHD_CONNECTION_FULL_REQ_RECEIVED + 1,

  /**
   * We have prepared the response headers in the write buffer.
   * Send the response headers.
   */
  MHD_CONNECTION_HEADERS_SENDING = MHD_CONNECTION_START_REPLY + 1,

  /**
   * We have sent the response headers.  Get ready to send the body.
   */
  MHD_CONNECTION_HEADERS_SENT = MHD_CONNECTION_HEADERS_SENDING + 1,

  /**
   * We are waiting for the client to provide more
   * data of a non-chunked body.
   */
  MHD_CONNECTION_NORMAL_BODY_UNREADY = MHD_CONNECTION_HEADERS_SENT + 1,

  /**
   * We are ready to send a part of a non-chunked body.  Send it.
   */
  MHD_CONNECTION_NORMAL_BODY_READY = MHD_CONNECTION_NORMAL_BODY_UNREADY + 1,

  /**
   * We are waiting for the client to provide a chunk of the body.
   */
  MHD_CONNECTION_CHUNKED_BODY_UNREADY = MHD_CONNECTION_NORMAL_BODY_READY + 1,

  /**
   * We are ready to send a chunk.
   */
  MHD_CONNECTION_CHUNKED_BODY_READY = MHD_CONNECTION_CHUNKED_BODY_UNREADY + 1,

  /**
   * We have sent the response body. Prepare the footers.
   */
  MHD_CONNECTION_BODY_SENT = MHD_CONNECTION_CHUNKED_BODY_READY + 1,

  /**
   * We have prepared the response footer.  Send it.
   */
  MHD_CONNECTION_FOOTERS_SENDING = MHD_CONNECTION_BODY_SENT + 1,

  /**
   * We have sent the response footer.  Shutdown or restart.
   */
  MHD_CONNECTION_FOOTERS_SENT = MHD_CONNECTION_FOOTERS_SENDING + 1,

  /**
   * This connection is to be closed.
   */
  MHD_CONNECTION_CLOSED = MHD_CONNECTION_FOOTERS_SENT + 1,

#ifdef UPGRADE_SUPPORT
  /**
   * Connection was "upgraded" and socket is now under the
   * control of the application.
   */
  MHD_CONNECTION_UPGRADE
#endif /* UPGRADE_SUPPORT */

} _MHD_FIXED_ENUM;


/**
 * States of TLS transport layer.
 */
enum MHD_TLS_CONN_STATE
{
  MHD_TLS_CONN_NO_TLS = 0,  /**< Not a TLS connection (plain socket).   */
  MHD_TLS_CONN_INIT,        /**< TLS connection is not established yet. */
  MHD_TLS_CONN_HANDSHAKING, /**< TLS is in handshake process.           */
  MHD_TLS_CONN_CONNECTED,   /**< TLS is established.                    */
  MHD_TLS_CONN_WR_CLOSING,  /**< Closing WR side of TLS layer.          */
  MHD_TLS_CONN_WR_CLOSED,   /**< WR side of TLS layer is closed.        */
  MHD_TLS_CONN_TLS_CLOSING, /**< TLS session is terminating.            */
  MHD_TLS_CONN_TLS_CLOSED,  /**< TLS session is terminated.             */
  MHD_TLS_CONN_TLS_FAILED,  /**< TLS session failed.                    */
  MHD_TLS_CONN_INVALID_STATE/**< Sentinel. Not a valid value.           */
} _MHD_FIXED_ENUM;

/**
 * Should all state transitions be printed to stderr?
 */
#define DEBUG_STATES _MHD_MACRO_NO


#ifdef HAVE_MESSAGES
#if DEBUG_STATES
const char *
MHD_state_to_string (enum MHD_CONNECTION_STATE state);

#endif
#endif

/**
 * Function to receive plaintext data.
 *
 * @param conn the connection struct
 * @param write_to where to write received data
 * @param max_bytes maximum number of bytes to receive
 * @return number of bytes written to @a write_to
 */
typedef ssize_t
(*ReceiveCallback) (struct MHD_Connection *conn,
                    void *write_to,
                    size_t max_bytes);


/**
 * Function to transmit plaintext data.
 *
 * @param conn the connection struct
 * @param read_from where to read data to transmit
 * @param max_bytes maximum number of bytes to transmit
 * @return number of bytes transmitted
 */
typedef ssize_t
(*TransmitCallback) (struct MHD_Connection *conn,
                     const void *read_from,
                     size_t max_bytes);


/**
 * Ability to use same connection for next request
 */
enum MHD_ConnKeepAlive
{
  /**
   * Connection must be closed after sending response.
   */
  MHD_CONN_MUST_CLOSE = -1,

  /**
   * KeelAlive state is not yet determined
   */
  MHD_CONN_KEEPALIVE_UNKOWN = 0,

  /**
   * Connection can be used for serving next request
   */
  MHD_CONN_USE_KEEPALIVE = 1,

  /**
   * Connection will be upgraded
   */
  MHD_CONN_MUST_UPGRADE = 2
} _MHD_FIXED_ENUM;

enum MHD_HTTP_Version
{
  /**
   * Not a HTTP protocol or HTTP version is invalid.
   */
  MHD_HTTP_VER_INVALID = -1,

  /**
   * HTTP version is not yet received from the client.
   */
  MHD_HTTP_VER_UNKNOWN = 0,

  /**
   * HTTP version before 1.0, unsupported.
   */
  MHD_HTTP_VER_TOO_OLD = 1,

  /**
   * HTTP version 1.0
   */
  MHD_HTTP_VER_1_0 = 2,

  /**
   * HTTP version 1.1
   */
  MHD_HTTP_VER_1_1 = 3,

  /**
   * HTTP version 1.2-1.9, must be used as 1.1
   */
  MHD_HTTP_VER_1_2__1_9 = 4,

  /**
   * HTTP future version. Unsupported.
   */
  MHD_HTTP_VER_FUTURE = 100
} _MHD_FIXED_ENUM;

/**
 * Returns boolean 'true' if HTTP version is supported by MHD
 */
#define MHD_IS_HTTP_VER_SUPPORTED(ver) (MHD_HTTP_VER_1_0 <= (ver) && \
    MHD_HTTP_VER_1_2__1_9 >= (ver))

/**
 * Protocol should be used as HTTP/1.1 protocol.
 *
 * See the last paragraph of
 * https://datatracker.ietf.org/doc/html/rfc7230#section-2.6
 */
#define MHD_IS_HTTP_VER_1_1_COMPAT(ver) (MHD_HTTP_VER_1_1 == (ver) || \
    MHD_HTTP_VER_1_2__1_9 == (ver))

/**
 * The HTTP method.
 *
 * Only primary methods (specified in RFC7231) are defined here.
 */
enum MHD_HTTP_Method
{
  /**
   * No request string has been received yet
   */
  MHD_HTTP_MTHD_NO_METHOD = 0,
  /**
   * HTTP method GET
   */
  MHD_HTTP_MTHD_GET = 1,
  /**
   * HTTP method HEAD
   */
  MHD_HTTP_MTHD_HEAD = 2,
  /**
   * HTTP method POST
   */
  MHD_HTTP_MTHD_POST = 3,
  /**
   * HTTP method PUT
   */
  MHD_HTTP_MTHD_PUT = 4,
  /**
   * HTTP method DELETE
   */
  MHD_HTTP_MTHD_DELETE = 5,
  /**
   * HTTP method CONNECT
   */
  MHD_HTTP_MTHD_CONNECT = 6,
  /**
   * HTTP method OPTIONS
   */
  MHD_HTTP_MTHD_OPTIONS = 7,
  /**
   * HTTP method TRACE
   */
  MHD_HTTP_MTHD_TRACE = 8,
  /**
   * Other HTTP method. Check the string value.
   */
  MHD_HTTP_MTHD_OTHER = 1000
} _MHD_FIXED_ENUM;


/**
 * Reply-specific properties.
 */
struct MHD_Reply_Properties
{
  bool set; /**< Indicates that other members are set and valid */
  bool use_reply_body_headers; /**< Use reply body-specific headers */
  bool send_reply_body; /**< Send reply body (can be zero-sized) */
  bool chunked; /**< Use chunked encoding for reply */
};

/**
 * State kept for each HTTP request.
 */
struct MHD_Connection
{

#ifdef EPOLL_SUPPORT
  /**
   * Next pointer for the EDLL listing connections that are epoll-ready.
   */
  struct MHD_Connection *nextE;

  /**
   * Previous pointer for the EDLL listing connections that are epoll-ready.
   */
  struct MHD_Connection *prevE;
#endif

  /**
   * Next pointer for the DLL describing our IO state.
   */
  struct MHD_Connection *next;

  /**
   * Previous pointer for the DLL describing our IO state.
   */
  struct MHD_Connection *prev;

  /**
   * Next pointer for the XDLL organizing connections by timeout.
   * This DLL can be either the
   * 'manual_timeout_head/manual_timeout_tail' or the
   * 'normal_timeout_head/normal_timeout_tail', depending on whether a
   * custom timeout is set for the connection.
   */
  struct MHD_Connection *nextX;

  /**
   * Previous pointer for the XDLL organizing connections by timeout.
   */
  struct MHD_Connection *prevX;

  /**
   * Reference to the MHD_Daemon struct.
   */
  struct MHD_Daemon *daemon;

  /**
   * Linked list of parsed headers.
   */
  struct MHD_HTTP_Header *headers_received;

  /**
   * Tail of linked list of parsed headers.
   */
  struct MHD_HTTP_Header *headers_received_tail;

  /**
   * Response to transmit (initially NULL).
   */
  struct MHD_Response *response;

  /**
   * The memory pool is created whenever we first read from the TCP
   * stream and destroyed at the end of each request (and re-created
   * for the next request).  In the meantime, this pointer is NULL.
   * The pool is used for all connection-related data except for the
   * response (which maybe shared between connections) and the IP
   * address (which persists across individual requests).
   */
  struct MemoryPool *pool;

  /**
   * We allow the main application to associate some pointer with the
   * HTTP request, which is passed to each #MHD_AccessHandlerCallback
   * and some other API calls.  Here is where we store it.  (MHD does
   * not know or care what it is).
   */
  void *client_context;

  /**
   * We allow the main application to associate some pointer with the
   * TCP connection (which may span multiple HTTP requests).  Here is
   * where we store it.  (MHD does not know or care what it is).
   * The location is given to the #MHD_NotifyConnectionCallback and
   * also accessible via #MHD_CONNECTION_INFO_SOCKET_CONTEXT.
   */
  void *socket_context;

  /**
   * Request method.  Should be GET/POST/etc.  Allocated in pool.
   */
  char *method;

  /**
   * The request method as enum.
   */
  enum MHD_HTTP_Method http_mthd;

  /**
   * Requested URL (everything after "GET" only).  Allocated
   * in pool.
   */
  const char *url;

  /**
   * HTTP version string (i.e. http/1.1).  Allocated
   * in pool.
   */
  char *version;

  /**
   * HTTP protocol version as enum.
   */
  enum MHD_HTTP_Version http_ver;

  /**
   * Close connection after sending response?
   * Functions may change value from "Unknown" or "KeepAlive" to "Must close",
   * but no functions reset value "Must Close" to any other value.
   */
  enum MHD_ConnKeepAlive keepalive;

  /**
   * Buffer for reading requests.  Allocated in pool.  Actually one
   * byte larger than @e read_buffer_size (if non-NULL) to allow for
   * 0-termination.
   */
  char *read_buffer;

  /**
   * Buffer for writing response (headers only).  Allocated
   * in pool.
   */
  char *write_buffer;

  /**
   * Last incomplete header line during parsing of headers.
   * Allocated in pool.  Only valid if state is
   * either #MHD_CONNECTION_HEADER_PART_RECEIVED or
   * #MHD_CONNECTION_FOOTER_PART_RECEIVED.
   */
  char *last;

  /**
   * Position after the colon on the last incomplete header
   * line during parsing of headers.
   * Allocated in pool.  Only valid if state is
   * either #MHD_CONNECTION_HEADER_PART_RECEIVED or
   * #MHD_CONNECTION_FOOTER_PART_RECEIVED.
   */
  char *colon;

  /**
   * Foreign address (of length @e addr_len).  MALLOCED (not
   * in pool!).
   */
  struct sockaddr *addr;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /**
   * Thread handle for this connection (if we are using
   * one thread per connection).
   */
  MHD_thread_handle_ID_ pid;
#endif

  /**
   * Size of @e read_buffer (in bytes).
   * This value indicates how many bytes we're willing to read
   * into the buffer.
   */
  size_t read_buffer_size;

  /**
   * Position where we currently append data in @e read_buffer (the
   * next char after the last valid position).
   */
  size_t read_buffer_offset;

  /**
   * Size of @e write_buffer (in bytes).
   */
  size_t write_buffer_size;

  /**
   * Offset where we are with sending from @e write_buffer.
   */
  size_t write_buffer_send_offset;

  /**
   * Last valid location in write_buffer (where do we
   * append and up to where is it safe to send?)
   */
  size_t write_buffer_append_offset;

  /**
   * Number of bytes we had in the HTTP header, set once we
   * pass #MHD_CONNECTION_HEADERS_RECEIVED.
   */
  size_t header_size;

  /**
   * How many more bytes of the body do we expect
   * to read? #MHD_SIZE_UNKNOWN for unknown.
   */
  uint64_t remaining_upload_size;

  /**
   * Current write position in the actual response
   * (excluding headers, content only; should be 0
   * while sending headers).
   */
  uint64_t response_write_position;

  /**
   * The copy of iov response.
   * Valid if iovec response is used.
   * Updated during send.
   * Members are allocated in the pool.
   */
  struct MHD_iovec_track_ resp_iov;


#if defined(_MHD_HAVE_SENDFILE)
  enum MHD_resp_sender_
  {
    MHD_resp_sender_std = 0,
    MHD_resp_sender_sendfile
  } resp_sender;
#endif /* _MHD_HAVE_SENDFILE */

  /**
   * Position in the 100 CONTINUE message that
   * we need to send when receiving http 1.1 requests.
   */
  size_t continue_message_write_offset;

  /**
   * Length of the foreign address.
   */
  socklen_t addr_len;

  /**
   * Last time this connection had any activity
   * (reading or writing).
   */
  uint64_t last_activity;

  /**
   * After how many milliseconds of inactivity should
   * this connection time out?
   * Zero for no timeout.
   */
  uint64_t connection_timeout_ms;

  /**
   * Special member to be returned by #MHD_get_connection_info()
   */
  unsigned int connection_timeout_dummy;

  /**
   * Did we ever call the "default_handler" on this connection?  (this
   * flag will determine if we call the #MHD_OPTION_NOTIFY_COMPLETED
   * handler when the connection closes down).
   */
  bool client_aware;

  /**
   * Socket for this connection.  Set to #MHD_INVALID_SOCKET if
   * this connection has died (daemon should clean
   * up in that case).
   */
  MHD_socket socket_fd;

  /**
   * true if @e socket_fd is not TCP/IP (a UNIX domain socket, a pipe),
   * false (TCP/IP) otherwise.
   */
  enum MHD_tristate is_nonip;

  /**
   * true if #socket_fd is non-blocking, false otherwise.
   */
  bool sk_nonblck;

  /**
   * true if connection socket has set SIGPIPE suppression
   */
  bool sk_spipe_suppress;

  /**
   * Tracks TCP_CORK / TCP_NOPUSH of the connection socket.
   */
  enum MHD_tristate sk_corked;

  /**
   * Tracks TCP_NODELAY state of the connection socket.
   */
  enum MHD_tristate sk_nodelay;

  /**
   * Has this socket been closed for reading (i.e.  other side closed
   * the connection)?  If so, we must completely close the connection
   * once we are done sending our response (and stop trying to read
   * from this socket).
   */
  bool read_closed;

  /**
   * Some error happens during processing the connection therefore this
   * connection must be closed.
   * The error may come from the client side (like wrong request format),
   * from the application side (like data callback returned error), or from
   * the OS side (like out-of-memory).
   */
  bool stop_with_error;

  /**
   * Response queued early, before the request is fully processed,
   * the client upload is rejected.
   * The connection cannot be reused for additional requests as the current
   * request is incompletely read and it is unclear where is the initial
   * byte of the next request.
   */
  bool discard_request;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /**
   * Set to `true` if the thread has been joined.
   */
  bool thread_joined;
#endif

  /**
   * Are we currently inside the "idle" handler (to avoid recursively
   * invoking it).
   */
  bool in_idle;

  /**
   * Connection is in the cleanup DL-linked list.
   */
  bool in_cleanup;

#ifdef EPOLL_SUPPORT
  /**
   * What is the state of this socket in relation to epoll?
   */
  enum MHD_EpollState epoll_state;
#endif

  /**
   * State in the FSM for this connection.
   */
  enum MHD_CONNECTION_STATE state;

  /**
   * What is this connection waiting for?
   */
  enum MHD_ConnectionEventLoopInfo event_loop_info;

  /**
   * HTTP response code.  Only valid if response object
   * is already set.
   */
  unsigned int responseCode;

  /**
   * Reply-specific properties
   */
  struct MHD_Reply_Properties rp_props;

  /**
   * Are we receiving with chunked encoding?
   * This will be set to #MHD_YES after we parse the headers and
   * are processing the body with chunks.
   * After we are done with the body and we are processing the footers;
   * once the footers are also done, this will be set to #MHD_NO again
   * (before the final call to the handler).
   * It is used only for requests, chunked encoding for response is
   * indicated by @a rp_props.
   */
  bool have_chunked_upload;

  /**
   * If we are receiving with chunked encoding, where are we right
   * now?
   * Set to 0 if we are waiting to receive the chunk size;
   * otherwise, this is the size of the current chunk.
   * A value of zero is also used when we're at the end of the chunks.
   */
  uint64_t current_chunk_size;

  /**
   * If we are receiving with chunked encoding, where are we currently
   * with respect to the current chunk (at what offset / position)?
   */
  uint64_t current_chunk_offset;

  /**
   * Function used for reading HTTP request stream.
   */
  ReceiveCallback recv_cls;

#ifdef UPGRADE_SUPPORT
  /**
   * If this connection was upgraded, this points to
   * the upgrade response details such that the
   * #thread_main_connection_upgrade()-logic can perform the
   * bi-directional forwarding.
   */
  struct MHD_UpgradeResponseHandle *urh;
#endif /* UPGRADE_SUPPORT */

#ifdef HTTPS_SUPPORT

  /**
   * State required for HTTPS/SSL/TLS support.
   */
  gnutls_session_t tls_session;

  /**
   * Memory location to return for protocol session info.
   */
  int protocol;

  /**
   * Memory location to return for protocol session info.
   */
  int cipher;

  /**
   * State of connection's TLS layer
   */
  enum MHD_TLS_CONN_STATE tls_state;

  /**
   * Could it be that we are ready to read due to TLS buffers
   * even though the socket is not?
   */
  bool tls_read_ready;
#endif /* HTTPS_SUPPORT */

  /**
   * Is the connection suspended?
   */
  bool suspended;

  /**
   * Special member to be returned by #MHD_get_connection_info()
   */
  int suspended_dummy;

  /**
   * Is the connection wanting to resume?
   */
  volatile bool resuming;
};


#ifdef UPGRADE_SUPPORT
/**
 * Buffer we use for upgrade response handling in the unlikely
 * case where the memory pool was so small it had no buffer
 * capacity left.  Note that we don't expect to _ever_ use this
 * buffer, so it's mostly wasted memory (except that it allows
 * us to handle a tricky error condition nicely). So no need to
 * make this one big.  Applications that want to perform well
 * should just pick an adequate size for the memory pools.
 */
#define RESERVE_EBUF_SIZE 8

/**
 * Context we pass to epoll() for each of the two sockets
 * of a `struct MHD_UpgradeResponseHandle`.  We need to do
 * this so we can distinguish the two sockets when epoll()
 * gives us event notifications.
 */
struct UpgradeEpollHandle
{
  /**
   * Reference to the overall response handle this struct is
   * included within.
   */
  struct MHD_UpgradeResponseHandle *urh;

  /**
   * The socket this event is kind-of about.  Note that this is NOT
   * necessarily the socket we are polling on, as for when we read
   * from TLS, we epoll() on the connection's socket
   * (`urh->connection->socket_fd`), while this then the application's
   * socket (where the application will read from).  Nevertheless, for
   * the application to read, we need to first read from TLS, hence
   * the two are related.
   *
   * Similarly, for writing to TLS, this epoll() will be on the
   * connection's `socket_fd`, and this will merely be the FD which
   * the application would write to.  Hence this struct must always be
   * interpreted based on which field in `struct
   * MHD_UpgradeResponseHandle` it is (`app` or `mhd`).
   */
  MHD_socket socket;

  /**
   * IO-state of the @e socket (or the connection's `socket_fd`).
   */
  enum MHD_EpollState celi;

};


/**
 * Handle given to the application to manage special
 * actions relating to MHD responses that "upgrade"
 * the HTTP protocol (i.e. to WebSockets).
 */
struct MHD_UpgradeResponseHandle
{
  /**
   * The connection for which this is an upgrade handle.  Note that
   * because a response may be shared over many connections, this may
   * not be the only upgrade handle for the response of this connection.
   */
  struct MHD_Connection *connection;

#ifdef HTTPS_SUPPORT
  /**
   * Kept in a DLL per daemon.
   */
  struct MHD_UpgradeResponseHandle *next;

  /**
   * Kept in a DLL per daemon.
   */
  struct MHD_UpgradeResponseHandle *prev;

#ifdef EPOLL_SUPPORT
  /**
   * Next pointer for the EDLL listing urhs that are epoll-ready.
   */
  struct MHD_UpgradeResponseHandle *nextE;

  /**
   * Previous pointer for the EDLL listing urhs that are epoll-ready.
   */
  struct MHD_UpgradeResponseHandle *prevE;

  /**
   * Specifies whether urh already in EDLL list of ready connections.
   */
  bool in_eready_list;
#endif

  /**
   * The buffer for receiving data from TLS to
   * be passed to the application.  Contains @e in_buffer_size
   * bytes (unless @e in_buffer_size is zero). Do not free!
   */
  char *in_buffer;

  /**
   * The buffer for receiving data from the application to
   * be passed to TLS.  Contains @e out_buffer_size
   * bytes (unless @e out_buffer_size is zero). Do not free!
   */
  char *out_buffer;

  /**
   * Size of the @e in_buffer.
   * Set to 0 if the TLS connection went down for reading or socketpair
   * went down for writing.
   */
  size_t in_buffer_size;

  /**
   * Size of the @e out_buffer.
   * Set to 0 if the TLS connection went down for writing or socketpair
   * went down for reading.
   */
  size_t out_buffer_size;

  /**
   * Number of bytes actually in use in the @e in_buffer.  Can be larger
   * than @e in_buffer_size if and only if @a in_buffer_size is zero and
   * we still have bytes that can be forwarded.
   * Reset to zero if all data was forwarded to socketpair or
   * if socketpair went down for writing.
   */
  size_t in_buffer_used;

  /**
   * Number of bytes actually in use in the @e out_buffer. Can be larger
   * than @e out_buffer_size if and only if @a out_buffer_size is zero and
   * we still have bytes that can be forwarded.
   * Reset to zero if all data was forwarded to TLS connection or
   * if TLS connection went down for writing.
   */
  size_t out_buffer_used;

  /**
   * The socket we gave to the application (r/w).
   */
  struct UpgradeEpollHandle app;

  /**
   * If @a app_sock was a socketpair, our end of it, otherwise
   * #MHD_INVALID_SOCKET; (r/w).
   */
  struct UpgradeEpollHandle mhd;

  /**
   * Emergency IO buffer we use in case the memory pool has literally
   * nothing left.
   */
  char e_buf[RESERVE_EBUF_SIZE];

#endif /* HTTPS_SUPPORT */

  /**
   * Set to true after the application finished with the socket
   * by #MHD_UPGRADE_ACTION_CLOSE.
   *
   * When BOTH @e was_closed (changed by command from application)
   * AND @e clean_ready (changed internally by MHD) are set to
   * #MHD_YES, function #MHD_resume_connection() will move this
   * connection to cleanup list.
   * @remark This flag could be changed from any thread.
   */
  volatile bool was_closed;

  /**
   * Set to true if connection is ready for cleanup.
   *
   * In TLS mode functions #MHD_connection_finish_forward_() must
   * be called before setting this flag to true.
   *
   * In thread-per-connection mode, true in this flag means
   * that connection's thread exited or about to exit and will
   * not use MHD_Connection::urh data anymore.
   *
   * In any mode true in this flag also means that
   * MHD_Connection::urh data will not be used for socketpair
   * forwarding and forwarding itself is finished.
   *
   * When BOTH @e was_closed (changed by command from application)
   * AND @e clean_ready (changed internally by MHD) are set to
   * true, function #MHD_resume_connection() will move this
   * connection to cleanup list.
   * @remark This flag could be changed from thread that process
   * connection's recv(), send() and response.
   */
  volatile bool clean_ready;
};
#endif /* UPGRADE_SUPPORT */


/**
 * Signature of function called to log URI accesses.
 *
 * @param cls closure
 * @param uri uri being accessed
 * @param con connection handle
 * @return new closure
 */
typedef void *
(*LogCallback)(void *cls,
               const char *uri,
               struct MHD_Connection *con);

/**
 * Signature of function called to unescape URIs.  See also
 * #MHD_http_unescape().
 *
 * @param cls closure
 * @param conn connection handle
 * @param uri 0-terminated string to unescape (should be updated)
 * @return length of the resulting string
 */
typedef size_t
(*UnescapeCallback)(void *cls,
                    struct MHD_Connection *conn,
                    char *uri);


/**
 * State kept for each MHD daemon.  All connections are kept in two
 * doubly-linked lists.  The first one reflects the state of the
 * connection in terms of what operations we are waiting for (read,
 * write, locally blocked, cleanup) whereas the second is about its
 * timeout state (default or custom).
 */
struct MHD_Daemon
{

  /**
   * Callback function for all requests.
   */
  MHD_AccessHandlerCallback default_handler;

  /**
   * Closure argument to default_handler.
   */
  void *default_handler_cls;

  /**
   * Daemon's flags (bitfield).
   *
   * @remark Keep this member after pointer value to keep it
   * properly aligned as it will be used as member of union MHD_DaemonInfo.
   */
  enum MHD_FLAG options;

  /**
   * Head of doubly-linked list of new, externally added connections.
   */
  struct MHD_Connection *new_connections_head;

  /**
   * Tail of doubly-linked list of new, externally added connections.
   */
  struct MHD_Connection *new_connections_tail;

  /**
   * Head of doubly-linked list of our current, active connections.
   */
  struct MHD_Connection *connections_head;

  /**
   * Tail of doubly-linked list of our current, active connections.
   */
  struct MHD_Connection *connections_tail;

  /**
   * Head of doubly-linked list of our current but suspended connections.
   */
  struct MHD_Connection *suspended_connections_head;

  /**
   * Tail of doubly-linked list of our current but suspended connections.
   */
  struct MHD_Connection *suspended_connections_tail;

  /**
   * Head of doubly-linked list of connections to clean up.
   */
  struct MHD_Connection *cleanup_head;

  /**
   * Tail of doubly-linked list of connections to clean up.
   */
  struct MHD_Connection *cleanup_tail;

  /**
   * _MHD_YES if the @e listen_fd socket is a UNIX domain socket.
   */
  enum MHD_tristate listen_is_unix;

#ifdef EPOLL_SUPPORT
  /**
   * Head of EDLL of connections ready for processing (in epoll mode).
   */
  struct MHD_Connection *eready_head;

  /**
   * Tail of EDLL of connections ready for processing (in epoll mode)
   */
  struct MHD_Connection *eready_tail;

  /**
   * File descriptor associated with our epoll loop.
   *
   * @remark Keep this member after pointer value to keep it
   * properly aligned as it will be used as member of union MHD_DaemonInfo.
   */
  int epoll_fd;

  /**
   * true if the @e listen_fd socket is in the 'epoll' set,
   * false if not.
   */
  bool listen_socket_in_epoll;

#ifdef UPGRADE_SUPPORT
#ifdef HTTPS_SUPPORT
  /**
   * File descriptor associated with the #run_epoll_for_upgrade() loop.
   * Only available if #MHD_USE_HTTPS_EPOLL_UPGRADE is set.
   */
  int epoll_upgrade_fd;

  /**
   * true if @e epoll_upgrade_fd is in the 'epoll' set,
   * false if not.
   */
  bool upgrade_fd_in_epoll;
#endif /* HTTPS_SUPPORT */

  /**
   * Head of EDLL of upgraded connections ready for processing (in epoll mode).
   */
  struct MHD_UpgradeResponseHandle *eready_urh_head;

  /**
   * Tail of EDLL of upgraded connections ready for processing (in epoll mode)
   */
  struct MHD_UpgradeResponseHandle *eready_urh_tail;
#endif /* UPGRADE_SUPPORT */
#endif /* EPOLL_SUPPORT */

  /**
   * Head of the XDLL of ALL connections with a default ('normal')
   * timeout, sorted by timeout (earliest at the tail, most recently
   * used connection at the head).  MHD can just look at the tail of
   * this list to determine the timeout for all of its elements;
   * whenever there is an event of a connection, the connection is
   * moved back to the tail of the list.
   *
   * All connections by default start in this list; if a custom
   * timeout that does not match @e connection_timeout_ms is set, they
   * are moved to the @e manual_timeout_head-XDLL.
   * Not used in MHD_USE_THREAD_PER_CONNECTION mode as each thread
   * needs only one connection-specific timeout.
   */
  struct MHD_Connection *normal_timeout_head;

  /**
   * Tail of the XDLL of ALL connections with a default timeout,
   * sorted by timeout (earliest timeout at the tail).
   * Not used in MHD_USE_THREAD_PER_CONNECTION mode.
   */
  struct MHD_Connection *normal_timeout_tail;

  /**
   * Head of the XDLL of ALL connections with a non-default/custom
   * timeout, unsorted.  MHD will do a O(n) scan over this list to
   * determine the current timeout.
   * Not used in MHD_USE_THREAD_PER_CONNECTION mode.
   */
  struct MHD_Connection *manual_timeout_head;

  /**
   * Tail of the XDLL of ALL connections with a non-default/custom
   * timeout, unsorted.
   * Not used in MHD_USE_THREAD_PER_CONNECTION mode.
   */
  struct MHD_Connection *manual_timeout_tail;

  /**
   * Function to call to check if we should accept or reject an
   * incoming request.  May be NULL.
   */
  MHD_AcceptPolicyCallback apc;

  /**
   * Closure argument to apc.
   */
  void *apc_cls;

  /**
   * Function to call when we are done processing
   * a particular request.  May be NULL.
   */
  MHD_RequestCompletedCallback notify_completed;

  /**
   * Closure argument to @e notify_completed.
   */
  void *notify_completed_cls;

  /**
   * Function to call when we are starting/stopping
   * a connection.  May be NULL.
   */
  MHD_NotifyConnectionCallback notify_connection;

  /**
   * Closure argument to @e notify_connection.
   */
  void *notify_connection_cls;

  /**
   * Function to call with the full URI at the
   * beginning of request processing.  May be NULL.
   * <p>
   * Returns the initial pointer to internal state
   * kept by the client for the request.
   */
  LogCallback uri_log_callback;

  /**
   * Closure argument to @e uri_log_callback.
   */
  void *uri_log_callback_cls;

  /**
   * Function to call when we unescape escape sequences.
   */
  UnescapeCallback unescape_callback;

  /**
   * Closure for @e unescape_callback.
   */
  void *unescape_callback_cls;

  /**
   * Listen port.
   *
   * @remark Keep this member after pointer value to keep it
   * properly aligned as it will be used as member of union MHD_DaemonInfo.
   */
  uint16_t port;

#ifdef HAVE_MESSAGES
  /**
   * Function for logging error messages (if we
   * support error reporting).
   */
  MHD_LogCallback custom_error_log;

  /**
   * Closure argument to @e custom_error_log.
   */
  void *custom_error_log_cls;
#endif

  /**
   * Pointer to master daemon (NULL if this is the master)
   */
  struct MHD_Daemon *master;

  /**
   * Listen socket.
   *
   * @remark Keep this member after pointer value to keep it
   * properly aligned as it will be used as member of union MHD_DaemonInfo.
   */
  MHD_socket listen_fd;

  /**
   * Listen socket is non-blocking.
   */
  bool listen_nonblk;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /**
   * Worker daemons (one per thread)
   */
  struct MHD_Daemon *worker_pool;
#endif

  /**
   * Table storing number of connections per IP
   */
  void *per_ip_connection_count;

  /**
   * Number of active parallel connections.
   *
   * @remark Keep this member after pointer value to keep it
   * properly aligned as it will be used as member of union MHD_DaemonInfo.
   */
  unsigned int connections;

  /**
   * Size of the per-connection memory pools.
   */
  size_t pool_size;

  /**
   * Increment for growth of the per-connection memory pools.
   */
  size_t pool_increment;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /**
   * Size of threads created by MHD.
   */
  size_t thread_stack_size;

  /**
   * Number of worker daemons
   */
  unsigned int worker_pool_size;

  /**
   * The select thread handle (if we have internal select)
   */
  MHD_thread_handle_ID_ pid;

  /**
   * Mutex for per-IP connection counts.
   */
  MHD_mutex_ per_ip_connection_mutex;

  /**
   * Mutex for (modifying) access to the "cleanup", "normal_timeout" and
   * "manual_timeout" DLLs.
   */
  MHD_mutex_ cleanup_connection_mutex;

  /**
   * Mutex for any access to the "new connections" DL-list.
   */
  MHD_mutex_ new_connections_mutex;
#endif

  /**
   * Our #MHD_OPTION_SERVER_INSANITY level, bits indicating
   * which sanity checks are off.
   */
  enum MHD_DisableSanityCheck insanity_level;

  /**
   * Whether to allow/disallow/ignore reuse of listening address.
   * The semantics is the following:
   * 0: ignore (user did not ask for neither allow/disallow, use SO_REUSEADDR
   *    except W32)
   * >0: allow (use SO_REUSEPORT on most platforms, SO_REUSEADDR on Windows)
   * <0: disallow (mostly no action, SO_EXCLUSIVEADDRUSE on Windows or SO_EXCLBIND
   *     on Solaris)
   */
  int listening_address_reuse;


  /**
   * Inter-thread communication channel (also used to unblock
   * select() in non-threaded code).
   */
  struct MHD_itc_ itc;

  /**
   * Are we shutting down?
   */
  volatile bool shutdown;

  /**
   * Has this daemon been quiesced via #MHD_quiesce_daemon()?
   * If so, we should no longer use the @e listen_fd (including
   * removing it from the @e epoll_fd when possible).
   */
  volatile bool was_quiesced;

  /**
   * Did we hit some system or process-wide resource limit while
   * trying to accept() the last time? If so, we don't accept new
   * connections until we close an existing one.  This effectively
   * temporarily lowers the "connection_limit" to the current
   * number of connections.
   */
  bool at_limit;

  /*
   * Do we need to process resuming connections?
   */
  volatile bool resuming;

  /**
   * Indicate that new connections in @e new_connections_head list
   * need to be processed.
   */
  volatile bool have_new;

  /**
   * 'True' if some data is already waiting to be processed.
   * If set to 'true' - zero timeout for select()/poll*()
   * is used.
   * Should be reset each time before processing connections
   * and raised by any connection which require additional
   * immediately processing (application does not provide
   * data for response, data waiting in TLS buffers etc.)
   */
  bool data_already_pending;

  /**
   * Limit on the number of parallel connections.
   */
  unsigned int connection_limit;

  /**
   * After how many milliseconds of inactivity should
   * this connection time out?
   * Zero for no timeout.
   */
  uint64_t connection_timeout_ms;

  /**
   * Maximum number of connections per IP, or 0 for
   * unlimited.
   */
  unsigned int per_ip_connection_limit;

  /**
   * Be neutral (zero), strict (1) or permissive (-1) to client.
   */
  int strict_for_client;

  /**
   * True if SIGPIPE is blocked
   */
  bool sigpipe_blocked;

#ifdef HTTPS_SUPPORT
#ifdef UPGRADE_SUPPORT
  /**
   * Head of DLL of upgrade response handles we are processing.
   * Used for upgraded TLS connections when thread-per-connection
   * is not used.
   */
  struct MHD_UpgradeResponseHandle *urh_head;

  /**
   * Tail of DLL of upgrade response handles we are processing.
   * Used for upgraded TLS connections when thread-per-connection
   * is not used.
   */
  struct MHD_UpgradeResponseHandle *urh_tail;
#endif /* UPGRADE_SUPPORT */

  /**
   * Desired cipher algorithms.
   */
  gnutls_priority_t priority_cache;

  /**
   * What kind of credentials are we offering
   * for SSL/TLS?
   */
  gnutls_credentials_type_t cred_type;

  /**
   * Server x509 credentials
   */
  gnutls_certificate_credentials_t x509_cred;

  /**
   * Diffie-Hellman parameters
   */
  gnutls_dh_params_t dh_params;

  /**
   * Server PSK credentials
   */
  gnutls_psk_server_credentials_t psk_cred;

#if GNUTLS_VERSION_MAJOR >= 3
  /**
   * Function that can be used to obtain the certificate.  Needed
   * for SNI support.  See #MHD_OPTION_HTTPS_CERT_CALLBACK.
   */
  gnutls_certificate_retrieve_function2 *cert_callback;

  /**
   * Function that can be used to obtain the shared key.
   */
  MHD_PskServerCredentialsCallback cred_callback;

  /**
   * Closure for @e cred_callback.
   */
  void *cred_callback_cls;
#endif

#if GNUTLS_VERSION_NUMBER >= 0x030603
  /**
   * Function that can be used to obtain the certificate.  Needed
   * for OCSP stapling support.  See #MHD_OPTION_HTTPS_CERT_CALLBACK2.
   */
  gnutls_certificate_retrieve_function3 *cert_callback2;
#endif

  /**
   * Pointer to our SSL/TLS key (in ASCII) in memory.
   */
  const char *https_mem_key;

  /**
   * Pointer to our SSL/TLS certificate (in ASCII) in memory.
   */
  const char *https_mem_cert;

  /**
   * Pointer to 0-terminated HTTPS passphrase in memory.
   */
  const char *https_key_password;

  /**
   * Pointer to our SSL/TLS certificate authority (in ASCII) in memory.
   */
  const char *https_mem_trust;

  /**
   * Our Diffie-Hellman parameters in memory.
   */
  gnutls_dh_params_t https_mem_dhparams;

  /**
   * true if we have initialized @e https_mem_dhparams.
   */
  bool have_dhparams;

  /**
   * true if ALPN is disabled.
   */
  bool disable_alpn;

  #endif /* HTTPS_SUPPORT */

#ifdef DAUTH_SUPPORT

  /**
   * Character array of random values.
   */
  const char *digest_auth_random;

  /**
   * An array that contains the map nonce-nc.
   */
  struct MHD_NonceNc *nnc;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /**
   * A rw-lock for synchronizing access to @e nnc.
   */
  MHD_mutex_ nnc_lock;
#endif

  /**
   * Size of `digest_auth_random.
   */
  size_t digest_auth_rand_size;

  /**
   * Size of the nonce-nc array.
   */
  unsigned int nonce_nc_size;

#endif

#ifdef TCP_FASTOPEN
  /**
   * The queue size for incoming SYN + DATA packets.
   */
  unsigned int fastopen_queue_size;
#endif

  /**
   * The size of queue for listen socket.
   */
  unsigned int listen_backlog_size;

  /**
   * The number of user options used.
   *
   * Contains number of only meaningful options, i.e. #MHD_OPTION_END
   * and #MHD_OPTION_ARRAY are not counted, while options inside
   * #MHD_OPTION_ARRAY are counted.
   */
  size_t num_opts;
};


/**
 * Insert an element at the head of a DLL. Assumes that head, tail and
 * element are structs with prev and next fields.
 *
 * @param head pointer to the head of the DLL
 * @param tail pointer to the tail of the DLL
 * @param element element to insert
 */
#define DLL_insert(head,tail,element) do { \
    mhd_assert (NULL == (element)->next); \
    mhd_assert (NULL == (element)->prev); \
    (element)->next = (head);       \
    (element)->prev = NULL;         \
    if ((tail) == NULL) {           \
      (tail) = element;             \
    } else {                        \
      (head)->prev = element;       \
    }                               \
    (head) = (element); } while (0)


/**
 * Remove an element from a DLL. Assumes
 * that head, tail and element are structs
 * with prev and next fields.
 *
 * @param head pointer to the head of the DLL
 * @param tail pointer to the tail of the DLL
 * @param element element to remove
 */
#define DLL_remove(head,tail,element) do { \
    mhd_assert ( (NULL != (element)->next) || ((element) == (tail)));  \
    mhd_assert ( (NULL != (element)->prev) || ((element) == (head)));  \
    if ((element)->prev == NULL) {                                     \
      (head) = (element)->next;                \
    } else {                                   \
      (element)->prev->next = (element)->next; \
    }                                          \
    if ((element)->next == NULL) {             \
      (tail) = (element)->prev;                \
    } else {                                   \
      (element)->next->prev = (element)->prev; \
    }                                          \
    (element)->next = NULL;                    \
    (element)->prev = NULL; } while (0)


/**
 * Insert an element at the head of a XDLL. Assumes that head, tail and
 * element are structs with prevX and nextX fields.
 *
 * @param head pointer to the head of the XDLL
 * @param tail pointer to the tail of the XDLL
 * @param element element to insert
 */
#define XDLL_insert(head,tail,element) do { \
    mhd_assert (NULL == (element)->nextX); \
    mhd_assert (NULL == (element)->prevX); \
    (element)->nextX = (head);     \
    (element)->prevX = NULL;       \
    if (NULL == (tail)) {          \
      (tail) = element;            \
    } else {                       \
      (head)->prevX = element;     \
    }                              \
    (head) = (element); } while (0)


/**
 * Remove an element from a XDLL. Assumes
 * that head, tail and element are structs
 * with prevX and nextX fields.
 *
 * @param head pointer to the head of the XDLL
 * @param tail pointer to the tail of the XDLL
 * @param element element to remove
 */
#define XDLL_remove(head,tail,element) do { \
    mhd_assert ( (NULL != (element)->nextX) || ((element) == (tail)));  \
    mhd_assert ( (NULL != (element)->prevX) || ((element) == (head)));  \
    if (NULL == (element)->prevX) {                                     \
      (head) = (element)->nextX;                  \
    } else {                                      \
      (element)->prevX->nextX = (element)->nextX; \
    }                                             \
    if (NULL == (element)->nextX) {               \
      (tail) = (element)->prevX;                  \
    } else {                                      \
      (element)->nextX->prevX = (element)->prevX; \
    }                                             \
    (element)->nextX = NULL;                      \
    (element)->prevX = NULL; } while (0)


/**
 * Insert an element at the head of a EDLL. Assumes that head, tail and
 * element are structs with prevE and nextE fields.
 *
 * @param head pointer to the head of the EDLL
 * @param tail pointer to the tail of the EDLL
 * @param element element to insert
 */
#define EDLL_insert(head,tail,element) do { \
    (element)->nextE = (head); \
    (element)->prevE = NULL;   \
    if ((tail) == NULL) {      \
      (tail) = element;        \
    } else {                   \
      (head)->prevE = element; \
    }                          \
    (head) = (element); } while (0)


/**
 * Remove an element from a EDLL. Assumes
 * that head, tail and element are structs
 * with prevE and nextE fields.
 *
 * @param head pointer to the head of the EDLL
 * @param tail pointer to the tail of the EDLL
 * @param element element to remove
 */
#define EDLL_remove(head,tail,element) do {       \
    if ((element)->prevE == NULL) {               \
      (head) = (element)->nextE;                  \
    } else {                                      \
      (element)->prevE->nextE = (element)->nextE; \
    }                                             \
    if ((element)->nextE == NULL) {               \
      (tail) = (element)->prevE;                  \
    } else {                                      \
      (element)->nextE->prevE = (element)->prevE; \
    }                                             \
    (element)->nextE = NULL;                      \
    (element)->prevE = NULL; } while (0)


/**
 * Convert all occurrences of '+' to ' '.
 *
 * @param arg string that is modified (in place), must be 0-terminated
 */
void
MHD_unescape_plus (char *arg);


/**
 * Callback invoked when iterating over @a key / @a value
 * argument pairs during parsing.
 *
 * @param connection context of the iteration
 * @param key 0-terminated key string, never NULL
 * @param key_size number of bytes in key
 * @param value 0-terminated binary data, may include binary zeros, may be NULL
 * @param value_size number of bytes in value
 * @param kind origin of the key-value pair
 * @return #MHD_YES on success (continue to iterate)
 *         #MHD_NO to signal failure (and abort iteration)
 */
typedef enum MHD_Result
(*MHD_ArgumentIterator_)(struct MHD_Connection *connection,
                         const char *key,
                         size_t key_size,
                         const char *value,
                         size_t value_size,
                         enum MHD_ValueKind kind);


/**
 * Parse and unescape the arguments given by the client
 * as part of the HTTP request URI.
 *
 * @param kind header kind to pass to @a cb
 * @param connection connection to add headers to
 * @param[in,out] args argument URI string (after "?" in URI),
 *        clobbered in the process!
 * @param cb function to call on each key-value pair found
 * @param[out] num_headers set to the number of headers found
 * @return #MHD_NO on failure (@a cb returned #MHD_NO),
 *         #MHD_YES for success (parsing succeeded, @a cb always
 *                               returned #MHD_YES)
 */
enum MHD_Result
MHD_parse_arguments_ (struct MHD_Connection *connection,
                      enum MHD_ValueKind kind,
                      char *args,
                      MHD_ArgumentIterator_ cb,
                      unsigned int *num_headers);


/**
 * Check whether response header contains particular token.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 *
 * @param response  the response to query
 * @param key       header name
 * @param key_len   the length of @a key, not including optional
 *                  terminating null-character.
 * @param token     the token to find
 * @param token_len the length of @a token, not including optional
 *                  terminating null-character.
 * @return true if token is found in specified header,
 *         false otherwise
 */
bool
MHD_check_response_header_token_ci (const struct MHD_Response *response,
                                    const char *key,
                                    size_t key_len,
                                    const char *token,
                                    size_t token_len);

/**
 * Check whether response header contains particular static @a tkn.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 * @param r   the response to query
 * @param k   header name
 * @param tkn the static string of token to find
 * @return true if token is found in specified header,
 *         false otherwise
 */
#define MHD_check_response_header_s_token_ci(r,k,tkn) \
  MHD_check_response_header_token_ci ((r),(k),MHD_STATICSTR_LEN_ (k), \
                                      (tkn),MHD_STATICSTR_LEN_ (tkn))

/**
 * Trace up to and return master daemon. If the supplied daemon
 * is a master, then return the daemon itself.
 *
 * @param daemon handle to a daemon
 * @return master daemon handle
 */
struct MHD_Daemon *
MHD_get_master (struct MHD_Daemon *daemon);

/**
 * Internal version of #MHD_suspend_connection().
 *
 * @remark In thread-per-connection mode: can be called from any thread,
 * in any other mode: to be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param connection the connection to suspend
 */
void
internal_suspend_connection_ (struct MHD_Connection *connection);


#ifdef UPGRADE_SUPPORT
/**
 * Mark upgraded connection as closed by application.
 *
 * The @a connection pointer must not be used after call of this function
 * as it may be freed in other thread immediately.
 * @param connection the upgraded connection to mark as closed by application
 */
void
MHD_upgraded_connection_mark_app_closed_ (struct MHD_Connection *connection);

#endif /* UPGRADE_SUPPORT */


#endif
