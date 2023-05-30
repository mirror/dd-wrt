/*
  This file is part of libmicrohttpd
  Copyright (C) 2017,2020 Karlson2k (Evgeny Grin), Full re-write of buffering and
                     pushing, many bugs fixes, optimisations, sendfile() porting
  Copyright (C) 2019 ng0 <ng0@n0.is>, Initial version of send() wrappers

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
 * @file microhttpd/mhd_send.c
 * @brief Implementation of send() wrappers and helper functions.
 * @author Karlson2k (Evgeny Grin)
 * @author ng0 (N. Gillmann)
 * @author Christian Grothoff
 */

/* Worth considering for future improvements and additions:
 * NetBSD has no sendfile or sendfile64. The way to work
 * with this seems to be to mmap the file and write(2) as
 * large a chunk as possible to the socket. Alternatively,
 * use madvise(..., MADV_SEQUENTIAL). */

#include "mhd_send.h"
#ifdef MHD_LINUX_SOLARIS_SENDFILE
#include <sys/sendfile.h>
#endif /* MHD_LINUX_SOLARIS_SENDFILE */
#if defined(HAVE_FREEBSD_SENDFILE) || defined(HAVE_DARWIN_SENDFILE)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif /* HAVE_FREEBSD_SENDFILE || HAVE_DARWIN_SENDFILE */
#ifdef HAVE_SYS_PARAM_H
/* For FreeBSD version identification */
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */
#ifdef HAVE_SYSCONF
#include <unistd.h>
#endif /* HAVE_SYSCONF */
#include "mhd_assert.h"

#include "mhd_limits.h"

#ifdef MHD_VECT_SEND
#if (! defined(HAVE_SENDMSG) || ! defined(MSG_NOSIGNAL)) && \
  defined(MHD_SEND_SPIPE_SUPPRESS_POSSIBLE) && \
  defined(MHD_SEND_SPIPE_SUPPRESS_NEEDED)
#define _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED 1
#endif /* (!HAVE_SENDMSG || !MSG_NOSIGNAL) &&
          MHD_SEND_SPIPE_SUPPRESS_POSSIBLE && MHD_SEND_SPIPE_SUPPRESS_NEEDED */
#endif /* MHD_VECT_SEND */

/**
 * sendfile() chuck size
 */
#define MHD_SENFILE_CHUNK_         (0x20000)

/**
 * sendfile() chuck size for thread-per-connection
 */
#define MHD_SENFILE_CHUNK_THR_P_C_ (0x200000)

#ifdef HAVE_FREEBSD_SENDFILE
#ifdef SF_FLAGS
/**
 * FreeBSD sendfile() flags
 */
static int freebsd_sendfile_flags_;

/**
 * FreeBSD sendfile() flags for thread-per-connection
 */
static int freebsd_sendfile_flags_thd_p_c_;


/**
 * Initialises variables for FreeBSD's sendfile()
 */
static void
freebsd_sendfile_init_ (void)
{
  long sys_page_size = sysconf (_SC_PAGESIZE);
  if (0 >= sys_page_size)
  {   /* Failed to get page size. */
    freebsd_sendfile_flags_ = SF_NODISKIO;
    freebsd_sendfile_flags_thd_p_c_ = SF_NODISKIO;
  }
  else
  {
    freebsd_sendfile_flags_ =
      SF_FLAGS ((uint16_t) ((MHD_SENFILE_CHUNK_ + sys_page_size - 1)
                            / sys_page_size), SF_NODISKIO);
    freebsd_sendfile_flags_thd_p_c_ =
      SF_FLAGS ((uint16_t) ((MHD_SENFILE_CHUNK_THR_P_C_ + sys_page_size - 1)
                            / sys_page_size), SF_NODISKIO);
  }
}


#endif /* SF_FLAGS */
#endif /* HAVE_FREEBSD_SENDFILE */


#if defined(HAVE_SYSCONF) && defined(_SC_IOV_MAX)
/**
 * Current IOV_MAX system value
 */
static unsigned long mhd_iov_max_ = 0;

static void
iov_max_init_ (void)
{
  long res = sysconf (_SC_IOV_MAX);
  if (res >= 0)
    mhd_iov_max_ = res;
#if defined(IOV_MAX)
  else
    mhd_iov_max_ = IOV_MAX;
#endif /* IOV_MAX */
}


/**
 * IOV_MAX (run-time) value
 */
#define _MHD_IOV_MAX    mhd_iov_max_
#elif defined(IOV_MAX)

/**
 * IOV_MAX (static) value
 */
#define _MHD_IOV_MAX    IOV_MAX
#endif /* HAVE_SYSCONF && _SC_IOV_MAX */


/**
 * Initialises static variables
 */
void
MHD_send_init_static_vars_ (void)
{
#ifdef HAVE_FREEBSD_SENDFILE
  /* FreeBSD 11 and later allow to specify read-ahead size
   * and handles SF_NODISKIO differently.
   * SF_FLAGS defined only on FreeBSD 11 and later. */
#ifdef SF_FLAGS
  freebsd_sendfile_init_ ();
#endif /* SF_FLAGS */
#endif /* HAVE_FREEBSD_SENDFILE */
#if defined(HAVE_SYSCONF) && defined(_SC_IOV_MAX)
  iov_max_init_ ();
#endif /* HAVE_SYSCONF && _SC_IOV_MAX */
}


bool
MHD_connection_set_nodelay_state_ (struct MHD_Connection *connection,
                                   bool nodelay_state)
{
#ifdef TCP_NODELAY
  const MHD_SCKT_OPT_BOOL_ off_val = 0;
  const MHD_SCKT_OPT_BOOL_ on_val = 1;
  int err_code;

  if (_MHD_YES == connection->is_nonip)
    return false;

  if (0 == setsockopt (connection->socket_fd,
                       IPPROTO_TCP,
                       TCP_NODELAY,
                       (const void *) (nodelay_state ? &on_val : &off_val),
                       sizeof (off_val)))
  {
    connection->sk_nodelay = nodelay_state;
    return true;
  }

  err_code = MHD_socket_get_error_ ();
  if (MHD_SCKT_ERR_IS_ (err_code, MHD_SCKT_EINVAL_) ||
      MHD_SCKT_ERR_IS_ (err_code, MHD_SCKT_ENOPROTOOPT_) ||
      MHD_SCKT_ERR_IS_ (err_code, MHD_SCKT_ENOTSOCK_))
  {
    if (_MHD_UNKNOWN == connection->is_nonip)
      connection->is_nonip = _MHD_YES;
#ifdef HAVE_MESSAGES
    else
    {
      MHD_DLOG (connection->daemon,
                _ ("Setting %s option to %s state failed "
                   "for TCP/IP socket %d: %s\n"),
                "TCP_NODELAY",
                nodelay_state ? _ ("ON") : _ ("OFF"),
                (int) connection->socket_fd,
                MHD_socket_strerr_ (err_code));
    }
#endif /* HAVE_MESSAGES */
  }
#ifdef HAVE_MESSAGES
  else
  {
    MHD_DLOG (connection->daemon,
              _ ("Setting %s option to %s state failed: %s\n"),
              "TCP_NODELAY",
              nodelay_state ? _ ("ON") : _ ("OFF"),
              MHD_socket_strerr_ (err_code));
  }
#endif /* HAVE_MESSAGES */

#else  /* ! TCP_NODELAY */
  (void) connection; (void) nodelay_state; /* Mute compiler warnings */
#endif /* ! TCP_NODELAY */
  return false;
}


/**
 * Set required cork state for connection socket
 *
 * The function automatically updates sk_corked state.
 *
 * @param connection the connection to manipulate
 * @param cork_state the requested new state of socket
 * @return true if succeed, false if failed or not supported
 *         by the current platform / kernel.
 */
bool
MHD_connection_set_cork_state_ (struct MHD_Connection *connection,
                                bool cork_state)
{
#if defined(MHD_TCP_CORK_NOPUSH)
  const MHD_SCKT_OPT_BOOL_ off_val = 0;
  const MHD_SCKT_OPT_BOOL_ on_val = 1;
  int err_code;

  if (_MHD_YES == connection->is_nonip)
    return false;
  if (0 == setsockopt (connection->socket_fd,
                       IPPROTO_TCP,
                       MHD_TCP_CORK_NOPUSH,
                       (const void *) (cork_state ? &on_val : &off_val),
                       sizeof (off_val)))
  {
    connection->sk_corked = cork_state;
    return true;
  }

  err_code = MHD_socket_get_error_ ();
  if (MHD_SCKT_ERR_IS_ (err_code, MHD_SCKT_EINVAL_) ||
      MHD_SCKT_ERR_IS_ (err_code, MHD_SCKT_ENOPROTOOPT_) ||
      MHD_SCKT_ERR_IS_ (err_code, MHD_SCKT_ENOTSOCK_))
  {
    if (_MHD_UNKNOWN == connection->is_nonip)
      connection->is_nonip = _MHD_YES;
#ifdef HAVE_MESSAGES
    else
    {
      MHD_DLOG (connection->daemon,
                _ ("Setting %s option to %s state failed "
                   "for TCP/IP socket %d: %s\n"),
#ifdef TCP_CORK
                "TCP_CORK",
#else  /* ! TCP_CORK */
                "TCP_NOPUSH",
#endif /* ! TCP_CORK */
                cork_state ? _ ("ON") : _ ("OFF"),
                (int) connection->socket_fd,
                MHD_socket_strerr_ (err_code));
    }
#endif /* HAVE_MESSAGES */
  }
#ifdef HAVE_MESSAGES
  else
  {
    MHD_DLOG (connection->daemon,
              _ ("Setting %s option to %s state failed: %s\n"),
#ifdef TCP_CORK
              "TCP_CORK",
#else  /* ! TCP_CORK */
              "TCP_NOPUSH",
#endif /* ! TCP_CORK */
              cork_state ? _ ("ON") : _ ("OFF"),
              MHD_socket_strerr_ (err_code));
  }
#endif /* HAVE_MESSAGES */

#else  /* ! MHD_TCP_CORK_NOPUSH */
  (void) connection; (void) cork_state; /* Mute compiler warnings. */
#endif /* ! MHD_TCP_CORK_NOPUSH */
  return false;
}


/**
 * Handle pre-send setsockopt calls.
 *
 * @param connection the MHD_Connection structure
 * @param plain_send set to true if plain send() or sendmsg() will be called,
 *                   set to false if TLS socket send(), sendfile() or
 *                   writev() will be called.
 * @param push_data whether to push data to the network from buffers after
 *                  the next call of send function.
 */
static void
pre_send_setopt (struct MHD_Connection *connection,
                 bool plain_send,
                 bool push_data)
{
  /* Try to buffer data if not sending the final piece.
   * Final piece is indicated by push_data == true. */
  const bool buffer_data = (! push_data);

  if (_MHD_YES == connection->is_nonip)
    return;
  /* The goal is to minimise the total number of additional sys-calls
   * before and after send().
   * The following tricky (over-)complicated algorithm typically use zero,
   * one or two additional sys-calls (depending on OS) for each response. */

  if (buffer_data)
  {
    /* Need to buffer data if possible. */
#ifdef MHD_USE_MSG_MORE
    if (plain_send)
      return; /* Data is buffered by send() with MSG_MORE flag.
               * No need to check or change anything. */
#else  /* ! MHD_USE_MSG_MORE */
    (void) plain_send; /* Mute compiler warning. */
#endif /* ! MHD_USE_MSG_MORE */

#ifdef MHD_TCP_CORK_NOPUSH
    if (_MHD_ON == connection->sk_corked)
      return; /* The connection was already corked. */

    if (MHD_connection_set_cork_state_ (connection, true))
      return; /* The connection has been corked. */

    /* Failed to cork the connection.
     * Really unlikely to happen on TCP connections. */
#endif /* MHD_TCP_CORK_NOPUSH */
    if (_MHD_OFF == connection->sk_nodelay)
      return; /* TCP_NODELAY was not set for the socket.
               * Nagle's algorithm will buffer some data. */

    /* Try to reset TCP_NODELAY state for the socket.
     * Ignore possible error as no other options exist to
     * buffer data. */
    MHD_connection_set_nodelay_state_ (connection, false);
    /* TCP_NODELAY has been (hopefully) reset for the socket.
     * Nagle's algorithm will buffer some data. */
    return;
  }

  /* Need to push data after send() */
  /* If additional sys-call is required prefer to make it after the send()
   * as the next send() may consume only part of the prepared data and
   * more send() calls will be used. */
#ifdef MHD_TCP_CORK_NOPUSH
#ifdef _MHD_CORK_RESET_PUSH_DATA
#ifdef _MHD_CORK_RESET_PUSH_DATA_ALWAYS
  /* Data can be pushed immediately by uncorking socket regardless of
   * cork state before. */
  /* This is typical for Linux, no other kernel with
   * such behavior are known so far. */

  /* No need to check the current state of TCP_CORK / TCP_NOPUSH
   * as reset of cork will push the data anyway. */
  return; /* Data may be pushed by resetting of
           * TCP_CORK / TCP_NOPUSH after send() */
#else  /* ! _MHD_CORK_RESET_PUSH_DATA_ALWAYS */
  /* Reset of TCP_CORK / TCP_NOPUSH will push the data
   * only if socket is corked. */

#ifdef _MHD_NODELAY_SET_PUSH_DATA_ALWAYS
  /* Data can be pushed immediately by setting TCP_NODELAY regardless
   * of TCP_NODDELAY or corking state before. */

  /* Dead code currently, no known kernels with such behavior. */
  return; /* Data may be pushed by setting of TCP_NODELAY after send().
             No need to make extra sys-calls before send().*/
#else  /* ! _MHD_NODELAY_SET_PUSH_DATA_ALWAYS */

#ifdef _MHD_NODELAY_SET_PUSH_DATA
  /* Setting of TCP_NODELAY will push the data only if
   * both TCP_NODELAY and TCP_CORK / TCP_NOPUSH were not set. */

  /* Data can be pushed immediately by uncorking socket if
   * socket was corked before or by setting TCP_NODELAY if
   * socket was not corked and TCP_NODELAY was not set before. */

  /* Dead code currently as Linux is the only kernel that push
   * data by setting of TCP_NODELAY and Linux push data always. */
#else  /* ! _MHD_NODELAY_SET_PUSH_DATA */
  /* Data can be pushed immediately by uncorking socket or
   * can be pushed by send() on uncorked socket if
   * TCP_NODELAY was set *before*. */

  /* This is typical FreeBSD behavior. */
#endif /* ! _MHD_NODELAY_SET_PUSH_DATA */

  if (_MHD_ON == connection->sk_corked)
    return; /* Socket is corked. Data can be pushed by resetting of
             * TCP_CORK / TCP_NOPUSH after send() */
  else if (_MHD_OFF == connection->sk_corked)
  {
    /* The socket is not corked. */
    if (_MHD_ON == connection->sk_nodelay)
      return; /* TCP_NODELAY was already set,
               * data will be pushed automatically by the next send() */
#ifdef _MHD_NODELAY_SET_PUSH_DATA
    else if (_MHD_UNKNOWN == connection->sk_nodelay)
    {
      /* Setting TCP_NODELAY may push data.
       * Cork socket here and uncork after send(). */
      if (MHD_connection_set_cork_state_ (connection, true))
        return; /* The connection has been corked.
                 * Data can be pushed by resetting of
                 * TCP_CORK / TCP_NOPUSH after send() */
      else
      {
        /* The socket cannot be corked.
         * Really unlikely to happen on TCP connections */
        /* Have to set TCP_NODELAY.
         * If TCP_NODELAY real system state was OFF then
         * already buffered data may be pushed here, but this is unlikely
         * to happen as it is only a backup solution when corking has failed.
         * Ignore possible error here as no other options exist to
         * push data. */
        MHD_connection_set_nodelay_state_ (connection, true);
        /* TCP_NODELAY has been (hopefully) set for the socket.
         * The data will be pushed by the next send(). */
        return;
      }
    }
#endif /* _MHD_NODELAY_SET_PUSH_DATA */
    else
    {
#ifdef _MHD_NODELAY_SET_PUSH_DATA
      /* TCP_NODELAY was switched off and
       * the socket is not corked. */
#else  /* ! _MHD_NODELAY_SET_PUSH_DATA */
      /* Socket is not corked and TCP_NODELAY was not set or unknown. */
#endif /* ! _MHD_NODELAY_SET_PUSH_DATA */

      /* At least one additional sys-call is required. */
      /* Setting TCP_NODELAY is optimal here as data will be pushed
       * automatically by the next send() and no additional
       * sys-call are needed after the send(). */
      if (MHD_connection_set_nodelay_state_ (connection, true))
        return;
      else
      {
        /* Failed to set TCP_NODELAY for the socket.
         * Really unlikely to happen on TCP connections. */
        /* Cork the socket here and make additional sys-call
         * to uncork the socket after send(). */
        /* Ignore possible error here as no other options exist to
         * push data. */
        MHD_connection_set_cork_state_ (connection, true);
        /* The connection has been (hopefully) corked.
         * Data can be pushed by resetting of TCP_CORK / TCP_NOPUSH
         * after send() */
        return;
      }
    }
  }
  /* Corked state is unknown. Need to make sys-call here otherwise
   * data may not be pushed. */
  if (MHD_connection_set_cork_state_ (connection, true))
    return; /* The connection has been corked.
             * Data can be pushed by resetting of
             * TCP_CORK / TCP_NOPUSH after send() */
  /* The socket cannot be corked.
   * Really unlikely to happen on TCP connections */
  if (_MHD_ON == connection->sk_nodelay)
    return; /* TCP_NODELAY was already set,
             * data will be pushed by the next send() */
  /* Have to set TCP_NODELAY. */
#ifdef _MHD_NODELAY_SET_PUSH_DATA
  /* If TCP_NODELAY state was unknown (external connection) then
   * already buffered data may be pushed here, but this is unlikely
   * to happen as it is only a backup solution when corking has failed. */
#endif /* _MHD_NODELAY_SET_PUSH_DATA */
  /* Ignore possible error here as no other options exist to
   * push data. */
  MHD_connection_set_nodelay_state_ (connection, true);
  /* TCP_NODELAY has been (hopefully) set for the socket.
   * The data will be pushed by the next send(). */
  return;
#endif /* ! _MHD_NODELAY_SET_PUSH_DATA_ALWAYS */
#endif /* ! _MHD_CORK_RESET_PUSH_DATA_ALWAYS */
#else  /* ! _MHD_CORK_RESET_PUSH_DATA */
  /* Neither uncorking the socket or setting TCP_NODELAY
   * push the data immediately. */
  /* The only way to push the data is to use send() on uncorked
   * socket with TCP_NODELAY switched on . */

  /* This is a typical *BSD (except FreeBSD) and Darwin behavior. */

  /* Uncork socket if socket wasn't uncorked. */
  if (_MHD_OFF != connection->sk_corked)
    MHD_connection_set_cork_state_ (connection, false);

  /* Set TCP_NODELAY if it wasn't set. */
  if (_MHD_ON != connection->sk_nodelay)
    MHD_connection_set_nodelay_state_ (connection, true);

  return;
#endif /* ! _MHD_CORK_RESET_PUSH_DATA */
#else  /* ! MHD_TCP_CORK_NOPUSH */
  /* Buffering of data is controlled only by
   * Nagel's algorithm. */
  /* Set TCP_NODELAY if it wasn't set. */
  if (_MHD_ON != connection->sk_nodelay)
    MHD_connection_set_nodelay_state_ (connection, true);
#endif /* ! MHD_TCP_CORK_NOPUSH */
}


#ifndef _MHD_CORK_RESET_PUSH_DATA_ALWAYS
/**
 * Send zero-sized data
 *
 * This function use send of zero-sized data to kick data from the socket
 * buffers to the network. The socket must not be corked and must have
 * TCP_NODELAY switched on.
 * Used only as last resort option, when other options are failed due to
 * some errors.
 * Should not be called on typical data processing.
 * @return true if succeed, false if failed
 */
static bool
zero_send_ (struct MHD_Connection *connection)
{
  int dummy;

  if (_MHD_YES == connection->is_nonip)
    return false;
  mhd_assert (_MHD_OFF == connection->sk_corked);
  mhd_assert (_MHD_ON == connection->sk_nodelay);
  dummy = 0; /* Mute compiler and analyzer warnings */
  if (0 == MHD_send_ (connection->socket_fd, &dummy, 0))
    return true;
#ifdef HAVE_MESSAGES
  MHD_DLOG (connection->daemon,
            _ ("Zero-send failed: %s\n"),
            MHD_socket_last_strerr_ () );
#endif /* HAVE_MESSAGES */
  return false;
}


#endif /* ! _MHD_CORK_RESET_PUSH_DATA_ALWAYS */

/**
 * Handle post-send setsockopt calls.
 *
 * @param connection the MHD_Connection structure
 * @param plain_send_next set to true if plain send() or sendmsg() will be
 *                        called next,
 *                        set to false if TLS socket send(), sendfile() or
 *                        writev() will be called next.
 * @param push_data whether to push data to the network from buffers
 */
static void
post_send_setopt (struct MHD_Connection *connection,
                  bool plain_send_next,
                  bool push_data)
{
  /* Try to buffer data if not sending the final piece.
   * Final piece is indicated by push_data == true. */
  const bool buffer_data = (! push_data);

  if (_MHD_YES == connection->is_nonip)
    return;
  if (buffer_data)
    return; /* Nothing to do after send(). */

#ifndef MHD_USE_MSG_MORE
  (void) plain_send_next; /* Mute compiler warning */
#endif /* ! MHD_USE_MSG_MORE */

  /* Need to push data. */
#ifdef MHD_TCP_CORK_NOPUSH
#ifdef _MHD_CORK_RESET_PUSH_DATA_ALWAYS
#ifdef _MHD_NODELAY_SET_PUSH_DATA_ALWAYS
#ifdef MHD_USE_MSG_MORE
  if (_MHD_OFF == connection->sk_corked)
  {
    if (_MHD_ON == connection->sk_nodelay)
      return; /* Data was already pushed by send(). */
  }
  /* This is Linux kernel. There are options:
   * * Push the data by setting of TCP_NODELAY (without change
   *   of the cork on the socket),
   * * Push the data by resetting of TCP_CORK.
   * The optimal choice depends on the next final send functions
   * used on the same socket. If TCP_NODELAY wasn't set then push
   * data by setting TCP_NODELAY (TCP_NODELAY will not be removed
   * and is needed to push the data by send() without MSG_MORE).
   * If send()/sendmsg() will be used next than push data by
   * resetting of TCP_CORK so next send without MSG_MORE will push
   * data to the network (without additional sys-call to push data).
   * If next final send function will not support MSG_MORE (like
   * sendfile() or TLS-connection) than push data by setting
   * TCP_NODELAY so socket will remain corked (no additional
   * sys-call before next send()). */
  if ((_MHD_ON != connection->sk_nodelay) ||
      (! plain_send_next))
  {
    if (MHD_connection_set_nodelay_state_ (connection, true))
      return; /* Data has been pushed by TCP_NODELAY. */
    /* Failed to set TCP_NODELAY for the socket.
     * Really unlikely to happen on TCP connections. */
    if (MHD_connection_set_cork_state_ (connection, false))
      return; /* Data has been pushed by uncorking the socket. */
    /* Failed to uncork the socket.
     * Really unlikely to happen on TCP connections. */

    /* The socket cannot be uncorked, no way to push data */
  }
  else
  {
    if (MHD_connection_set_cork_state_ (connection, false))
      return; /* Data has been pushed by uncorking the socket. */
    /* Failed to uncork the socket.
     * Really unlikely to happen on TCP connections. */
    if (MHD_connection_set_nodelay_state_ (connection, true))
      return; /* Data has been pushed by TCP_NODELAY. */
    /* Failed to set TCP_NODELAY for the socket.
     * Really unlikely to happen on TCP connections. */

    /* The socket cannot be uncorked, no way to push data */
  }
#else  /* ! MHD_USE_MSG_MORE */
  /* Use setting of TCP_NODELAY here to avoid sys-call
   * for corking the socket during sending of the next response. */
  if (MHD_connection_set_nodelay_state_ (connection, true))
    return; /* Data was pushed by TCP_NODELAY. */
  /* Failed to set TCP_NODELAY for the socket.
   * Really unlikely to happen on TCP connections. */
  if (MHD_connection_set_cork_state_ (connection, false))
    return; /* Data was pushed by uncorking the socket. */
  /* Failed to uncork the socket.
   * Really unlikely to happen on TCP connections. */

  /* The socket remains corked, no way to push data */
#endif /* ! MHD_USE_MSG_MORE */
#else  /* ! _MHD_NODELAY_SET_PUSH_DATA_ALWAYS */
  if (MHD_connection_set_cork_state_ (connection, false))
    return; /* Data was pushed by uncorking the socket. */
  /* Failed to uncork the socket.
   * Really unlikely to happen on TCP connections. */
  return; /* Socket remains corked, no way to push data */
#endif /* ! _MHD_NODELAY_SET_PUSH_DATA_ALWAYS */
#else  /* ! _MHD_CORK_RESET_PUSH_DATA_ALWAYS */
  /* This is a typical *BSD or Darwin kernel. */

  if (_MHD_OFF == connection->sk_corked)
  {
    if (_MHD_ON == connection->sk_nodelay)
      return; /* Data was already pushed by send(). */

    /* Unlikely to reach this code.
     * TCP_NODELAY should be turned on before send(). */
    if (MHD_connection_set_nodelay_state_ (connection, true))
    {
      /* TCP_NODELAY has been set on uncorked socket.
       * Use zero-send to push the data. */
      if (zero_send_ (connection))
        return; /* The data has been pushed by zero-send. */
    }

    /* Failed to push the data by all means. */
    /* There is nothing left to try. */
  }
  else
  {
#ifdef _MHD_CORK_RESET_PUSH_DATA
    enum MHD_tristate old_cork_state = connection->sk_corked;
#endif /* _MHD_CORK_RESET_PUSH_DATA */
    /* The socket is corked or cork state is unknown. */

    if (MHD_connection_set_cork_state_ (connection, false))
    {
#ifdef _MHD_CORK_RESET_PUSH_DATA
      /* FreeBSD kernel */
      if (_MHD_OFF == old_cork_state)
        return; /* Data has been pushed by uncorking the socket. */
#endif /* _MHD_CORK_RESET_PUSH_DATA */

      /* Unlikely to reach this code.
       * The data should be pushed by uncorking (FreeBSD) or
       * the socket should be uncorked before send(). */
      if ((_MHD_ON == connection->sk_nodelay) ||
          (MHD_connection_set_nodelay_state_ (connection, true)))
      {
        /* TCP_NODELAY is turned ON on uncorked socket.
         * Use zero-send to push the data. */
        if (zero_send_ (connection))
          return; /* The data has been pushed by zero-send. */
      }
    }
    /* The socket remains corked. Data cannot be pushed. */
  }
#endif /* ! _MHD_CORK_RESET_PUSH_DATA_ALWAYS */
#else  /* ! MHD_TCP_CORK_NOPUSH */
  /* Corking is not supported. Buffering is controlled
   * by TCP_NODELAY only. */
  mhd_assert (_MHD_ON != connection->sk_corked);
  if (_MHD_ON == connection->sk_nodelay)
    return; /* Data was already pushed by send(). */

  /* Unlikely to reach this code.
   * TCP_NODELAY should be turned on before send(). */
  if (MHD_connection_set_nodelay_state_ (connection, true))
  {
    /* TCP_NODELAY has been set.
     * Use zero-send to push the data. */
    if (zero_send_ (connection))
      return; /* The data has been pushed by zero-send. */
  }

  /* Failed to push the data. */
#endif /* ! MHD_TCP_CORK_NOPUSH */
#ifdef HAVE_MESSAGES
  MHD_DLOG (connection->daemon,
            _ ("Failed to push the data from buffers to the network. "
               "Client may experience some delay "
               "(usually in range 200ms - 5 sec).\n"));
#endif /* HAVE_MESSAGES */
  return;
}


ssize_t
MHD_send_data_ (struct MHD_Connection *connection,
                const char *buffer,
                size_t buffer_size,
                bool push_data)
{
  MHD_socket s = connection->socket_fd;
  ssize_t ret;
#ifdef HTTPS_SUPPORT
  const bool tls_conn = (connection->daemon->options & MHD_USE_TLS);
#else  /* ! HTTPS_SUPPORT */
  const bool tls_conn = false;
#endif /* ! HTTPS_SUPPORT */

  if ( (MHD_INVALID_SOCKET == s) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
  {
    return MHD_ERR_NOTCONN_;
  }

  if (buffer_size > SSIZE_MAX)
  {
    buffer_size = SSIZE_MAX; /* Max return value */
    push_data = false; /* Incomplete send */
  }

  if (tls_conn)
  {
#ifdef HTTPS_SUPPORT
    pre_send_setopt (connection, (! tls_conn), push_data);
    ret = gnutls_record_send (connection->tls_session,
                              buffer,
                              buffer_size);
    if (GNUTLS_E_AGAIN == ret)
    {
#ifdef EPOLL_SUPPORT
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif
      return MHD_ERR_AGAIN_;
    }
    if (GNUTLS_E_INTERRUPTED == ret)
      return MHD_ERR_AGAIN_;
    if ( (GNUTLS_E_ENCRYPTION_FAILED == ret) ||
         (GNUTLS_E_INVALID_SESSION == ret) ||
         (GNUTLS_E_COMPRESSION_FAILED == ret) ||
         (GNUTLS_E_EXPIRED == ret) ||
         (GNUTLS_E_HASH_FAILED == ret) )
      return MHD_ERR_TLS_;
    if ( (GNUTLS_E_PUSH_ERROR == ret) ||
         (GNUTLS_E_INTERNAL_ERROR == ret) ||
         (GNUTLS_E_CRYPTODEV_IOCTL_ERROR == ret) ||
         (GNUTLS_E_CRYPTODEV_DEVICE_ERROR == ret) )
      return MHD_ERR_PIPE_;
#if defined(GNUTLS_E_PREMATURE_TERMINATION)
    if (GNUTLS_E_PREMATURE_TERMINATION == ret)
      return MHD_ERR_CONNRESET_;
#elif defined(GNUTLS_E_UNEXPECTED_PACKET_LENGTH)
    if (GNUTLS_E_UNEXPECTED_PACKET_LENGTH == ret)
      return MHD_ERR_CONNRESET_;
#endif /* GNUTLS_E_UNEXPECTED_PACKET_LENGTH */
    if (GNUTLS_E_MEMORY_ERROR == ret)
      return MHD_ERR_NOMEM_;
    if (ret < 0)
    {
      /* Treat any other error as hard error. */
      return MHD_ERR_NOTCONN_;
    }
#ifdef EPOLL_SUPPORT
    /* Unlike non-TLS connections, do not reset "write-ready" if
     * sent amount smaller than provided amount, as TLS
     * connections may break data into smaller parts for sending. */
#endif /* EPOLL_SUPPORT */
#else  /* ! HTTPS_SUPPORT  */
    ret = MHD_ERR_NOTCONN_;
#endif /* ! HTTPS_SUPPORT  */
  }
  else
  {
    /* plaintext transmission */
    if (buffer_size > MHD_SCKT_SEND_MAX_SIZE_)
    {
      buffer_size = MHD_SCKT_SEND_MAX_SIZE_; /* send() return value limit */
      push_data = false; /* Incomplete send */
    }

    pre_send_setopt (connection, (! tls_conn), push_data);
#ifdef MHD_USE_MSG_MORE
    ret = MHD_send4_ (s,
                      buffer,
                      buffer_size,
                      push_data ? 0 : MSG_MORE);
#else
    ret = MHD_send4_ (s,
                      buffer,
                      buffer_size,
                      0);
#endif

    if (0 > ret)
    {
      const int err = MHD_socket_get_error_ ();

      if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
      {
#if EPOLL_SUPPORT
        /* EAGAIN, no longer write-ready */
        connection->epoll_state &=
          ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
        return MHD_ERR_AGAIN_;
      }
      if (MHD_SCKT_ERR_IS_EINTR_ (err))
        return MHD_ERR_AGAIN_;
      if (MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
        return MHD_ERR_CONNRESET_;
      if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EPIPE_))
        return MHD_ERR_PIPE_;
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
#if EPOLL_SUPPORT
    else if (buffer_size > (size_t) ret)
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
  }

  /* If there is a need to push the data from network buffers
   * call post_send_setopt(). */
  /* If TLS connection is used then next final send() will be
   * without MSG_MORE support. If non-TLS connection is used
   * it's unknown whether sendfile() will be used or not so
   * assume that next call will be the same, like this call. */
  if ( (push_data) &&
       (buffer_size == (size_t) ret) )
    post_send_setopt (connection, (! tls_conn), push_data);

  return ret;
}


ssize_t
MHD_send_hdr_and_body_ (struct MHD_Connection *connection,
                        const char *header,
                        size_t header_size,
                        bool never_push_hdr,
                        const char *body,
                        size_t body_size,
                        bool complete_response)
{
  ssize_t ret;
  bool push_hdr;
  bool push_body;
  MHD_socket s = connection->socket_fd;
#ifndef _WIN32
#define _MHD_SEND_VEC_MAX   MHD_SCKT_SEND_MAX_SIZE_
#else  /* ! _WIN32 */
#define _MHD_SEND_VEC_MAX   UINT32_MAX
#endif /* ! _WIN32 */
#ifdef MHD_VECT_SEND
#if defined(HAVE_SENDMSG) || defined(HAVE_WRITEV)
  struct iovec vector[2];
#ifdef HAVE_SENDMSG
  struct msghdr msg;
#endif /* HAVE_SENDMSG */
#endif /* HAVE_SENDMSG || HAVE_WRITEV */
#ifdef _WIN32
  WSABUF vector[2];
  DWORD vec_sent;
#endif /* _WIN32 */
  bool no_vec; /* Is vector-send() disallowed? */

  no_vec = false;
#ifdef HTTPS_SUPPORT
  no_vec = no_vec || (connection->daemon->options & MHD_USE_TLS);
#endif /* HTTPS_SUPPORT */
#if (! defined(HAVE_SENDMSG) || ! defined(MSG_NOSIGNAL) ) && \
  defined(MHD_SEND_SPIPE_SEND_SUPPRESS_POSSIBLE) && \
  defined(MHD_SEND_SPIPE_SUPPRESS_NEEDED)
  no_vec = no_vec || (! connection->daemon->sigpipe_blocked &&
                      ! connection->sk_spipe_suppress);
#endif /* (!HAVE_SENDMSG || ! MSG_NOSIGNAL) &&
          MHD_SEND_SPIPE_SEND_SUPPRESS_POSSIBLE &&
          MHD_SEND_SPIPE_SUPPRESS_NEEDED */
#endif /* MHD_VECT_SEND */

  mhd_assert ( (NULL != body) || (0 == body_size) );

  if ( (MHD_INVALID_SOCKET == s) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
  {
    return MHD_ERR_NOTCONN_;
  }

  push_body = complete_response;

  if (! never_push_hdr)
  {
    if (! complete_response)
      push_hdr = true; /* Push the header as the client may react
                        * on header alone while the body data is
                        * being prepared. */
    else
    {
      if (1400 > (header_size + body_size))
        push_hdr = false;  /* Do not push the header as complete
                           * reply is already ready and the whole
                           * reply most probably will fit into
                           * the single IP packet. */
      else
        push_hdr = true;   /* Push header alone so client may react
                           * on it while reply body is being delivered. */
    }
  }
  else
    push_hdr = false;

  if (complete_response && (0 == body_size))
    push_hdr = true; /* The header alone is equal to the whole response. */

  if (
#ifdef MHD_VECT_SEND
    (no_vec) ||
    (0 == body_size) ||
    ((size_t) SSIZE_MAX <= header_size) ||
    ((size_t) _MHD_SEND_VEC_MAX < header_size)
#ifdef _WIN32
    || ((size_t) UINT_MAX < header_size)
#endif /* _WIN32 */
#else  /* ! MHD_VECT_SEND */
    true
#endif /* ! MHD_VECT_SEND */
    )
  {
    ret = MHD_send_data_ (connection,
                          header,
                          header_size,
                          push_hdr);

    if ( (header_size == (size_t) ret) &&
         ((size_t) SSIZE_MAX > header_size) &&
         (0 != body_size) &&
         (connection->sk_nonblck) )
    {
      ssize_t ret2;
      /* The header has been sent completely.
       * Try to send the reply body without waiting for
       * the next round. */
      /* Make sure that sum of ret + ret2 will not exceed SSIZE_MAX as
       * function needs to return positive value if succeed. */
      if ( (((size_t) SSIZE_MAX) - ((size_t) ret)) <  body_size)
      {
        body_size = (((size_t) SSIZE_MAX) - ((size_t) ret));
        complete_response = false;
        push_body = complete_response;
      }

      ret2 = MHD_send_data_ (connection,
                             body,
                             body_size,
                             push_body);
      if (0 < ret2)
        return ret + ret2; /* Total data sent */
      if (MHD_ERR_AGAIN_ == ret2)
        return ret;

      return ret2; /* Error code */
    }
    return ret;
  }
#ifdef MHD_VECT_SEND

  if ( ((size_t) SSIZE_MAX <= body_size) ||
       ((size_t) SSIZE_MAX < (header_size + body_size)) )
  {
    /* Return value limit */
    body_size = SSIZE_MAX - header_size;
    complete_response = false;
    push_body = complete_response;
  }
#if (SSIZE_MAX != _MHD_SEND_VEC_MAX) || (_MHD_SEND_VEC_MAX + 0 == 0)
  if (((size_t) _MHD_SEND_VEC_MAX <= body_size) ||
      ((size_t) _MHD_SEND_VEC_MAX < (header_size + body_size)))
  {
    /* Send total amount limit */
    body_size = _MHD_SEND_VEC_MAX - header_size;
    complete_response = false;
    push_body = complete_response;
  }
#endif /* SSIZE_MAX != _MHD_SEND_VEC_MAX */

  pre_send_setopt (connection,
#ifdef HAVE_SENDMSG
                   true,
#else  /* ! HAVE_SENDMSG */
                   false,
#endif /* ! HAVE_SENDMSG */
                   push_hdr || push_body);
#if defined(HAVE_SENDMSG) || defined(HAVE_WRITEV)
  vector[0].iov_base = (void *) header;
  vector[0].iov_len = header_size;
  vector[1].iov_base = (void *) body;
  vector[1].iov_len = body_size;

#if defined(HAVE_SENDMSG)
  memset (&msg, 0, sizeof(msg));
  msg.msg_iov = vector;
  msg.msg_iovlen = 2;

  ret = sendmsg (s, &msg, MSG_NOSIGNAL_OR_ZERO);
#elif defined(HAVE_WRITEV)
  ret = writev (s, vector, 2);
#endif /* HAVE_WRITEV */
#endif /* HAVE_SENDMSG || HAVE_WRITEV */
#ifdef _WIN32
  if ((size_t) UINT_MAX < body_size)
  {
    /* Send item size limit */
    body_size = UINT_MAX;
    complete_response = false;
    push_body = complete_response;
  }
  vector[0].buf = (char *) header;
  vector[0].len = (unsigned long) header_size;
  vector[1].buf = (char *) body;
  vector[1].len = (unsigned long) body_size;

  ret = WSASend (s, vector, 2, &vec_sent, 0, NULL, NULL);
  if (0 == ret)
    ret = (ssize_t) vec_sent;
  else
    ret = -1;
#endif /* _WIN32 */

  if (0 > ret)
  {
    const int err = MHD_socket_get_error_ ();

    if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
    {
#if EPOLL_SUPPORT
      /* EAGAIN, no longer write-ready */
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
      return MHD_ERR_AGAIN_;
    }
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return MHD_ERR_AGAIN_;
    if (MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
      return MHD_ERR_CONNRESET_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EPIPE_))
      return MHD_ERR_PIPE_;
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
#if EPOLL_SUPPORT
  else if ((header_size + body_size) > (size_t) ret)
    connection->epoll_state &=
      ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */

  /* If there is a need to push the data from network buffers
   * call post_send_setopt(). */
  if ( (push_body) &&
       ((header_size + body_size) == (size_t) ret) )
  {
    /* Complete reply has been sent. */
    /* If TLS connection is used then next final send() will be
     * without MSG_MORE support. If non-TLS connection is used
     * it's unknown whether next 'send' will be plain send() / sendmsg() or
     * sendfile() will be used so assume that next final send() will be
     * the same, like for this response. */
    post_send_setopt (connection,
#ifdef HAVE_SENDMSG
                      true,
#else  /* ! HAVE_SENDMSG */
                      false,
#endif /* ! HAVE_SENDMSG */
                      true);
  }
  else if ( (push_hdr) &&
            (header_size <= (size_t) ret))
  {
    /* The header has been sent completely and there is a
     * need to push the header data. */
    /* Luckily the type of send function will be used next is known. */
    post_send_setopt (connection,
#if defined(_MHD_HAVE_SENDFILE)
                      MHD_resp_sender_std == connection->resp_sender,
#else  /* ! _MHD_HAVE_SENDFILE */
                      true,
#endif /* ! _MHD_HAVE_SENDFILE */
                      true);
  }

  return ret;
#else  /* ! MHD_VECT_SEND */
  mhd_assert (false);
  return MHD_ERR_CONNRESET_; /* Unreachable. Mute warnings. */
#endif /* ! MHD_VECT_SEND */
}


#if defined(_MHD_HAVE_SENDFILE)
ssize_t
MHD_send_sendfile_ (struct MHD_Connection *connection)
{
  ssize_t ret;
  const int file_fd = connection->response->fd;
  uint64_t left;
  uint64_t offsetu64;
#ifndef HAVE_SENDFILE64
  const uint64_t max_off_t = (uint64_t) OFF_T_MAX;
#else  /* HAVE_SENDFILE64 */
  const uint64_t max_off_t = (uint64_t) OFF64_T_MAX;
#endif /* HAVE_SENDFILE64 */
#ifdef MHD_LINUX_SOLARIS_SENDFILE
#ifndef HAVE_SENDFILE64
  off_t offset;
#else  /* HAVE_SENDFILE64 */
  off64_t offset;
#endif /* HAVE_SENDFILE64 */
#endif /* MHD_LINUX_SOLARIS_SENDFILE */
#ifdef HAVE_FREEBSD_SENDFILE
  off_t sent_bytes;
  int flags = 0;
#endif
#ifdef HAVE_DARWIN_SENDFILE
  off_t len;
#endif /* HAVE_DARWIN_SENDFILE */
  const bool used_thr_p_c = (0 != (connection->daemon->options
                                   & MHD_USE_THREAD_PER_CONNECTION));
  const size_t chunk_size = used_thr_p_c ? MHD_SENFILE_CHUNK_THR_P_C_ :
                            MHD_SENFILE_CHUNK_;
  size_t send_size = 0;
  bool push_data;
  mhd_assert (MHD_resp_sender_sendfile == connection->resp_sender);
  mhd_assert (0 == (connection->daemon->options & MHD_USE_TLS));

  offsetu64 = connection->response_write_position
              + connection->response->fd_off;
  if (max_off_t < offsetu64)
  {   /* Retry to send with standard 'send()'. */
    connection->resp_sender = MHD_resp_sender_std;
    return MHD_ERR_AGAIN_;
  }

  left = connection->response->total_size - connection->response_write_position;

  if ( (uint64_t) SSIZE_MAX < left)
    left = SSIZE_MAX;

  /* Do not allow system to stick sending on single fast connection:
   * use 128KiB chunks (2MiB for thread-per-connection). */
  if (chunk_size < left)
  {
    send_size = chunk_size;
    push_data = false; /* No need to push data, there is more to send. */
  }
  else
  {
    send_size = (size_t) left;
    push_data = true; /* Final piece of data, need to push to the network. */
  }
  pre_send_setopt (connection, false, push_data);

#ifdef MHD_LINUX_SOLARIS_SENDFILE
#ifndef HAVE_SENDFILE64
  offset = (off_t) offsetu64;
  ret = sendfile (connection->socket_fd,
                  file_fd,
                  &offset,
                  send_size);
#else  /* HAVE_SENDFILE64 */
  offset = (off64_t) offsetu64;
  ret = sendfile64 (connection->socket_fd,
                    file_fd,
                    &offset,
                    send_size);
#endif /* HAVE_SENDFILE64 */
  if (0 > ret)
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
    {
#ifdef EPOLL_SUPPORT
      /* EAGAIN --- no longer write-ready */
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
      return MHD_ERR_AGAIN_;
    }
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return MHD_ERR_AGAIN_;
#ifdef HAVE_LINUX_SENDFILE
    if (MHD_SCKT_ERR_IS_ (err,
                          MHD_SCKT_EBADF_))
      return MHD_ERR_BADF_;
    /* sendfile() failed with EINVAL if mmap()-like operations are not
       supported for FD or other 'unusual' errors occurred, so we should try
       to fall back to 'SEND'; see also this thread for info on
       odd libc/Linux behavior with sendfile:
       http://lists.gnu.org/archive/html/libmicrohttpd/2011-02/msg00015.html */
    connection->resp_sender = MHD_resp_sender_std;
    return MHD_ERR_AGAIN_;
#else  /* HAVE_SOLARIS_SENDFILE */
    if ( (EAFNOSUPPORT == err) ||
         (EINVAL == err) ||
         (EOPNOTSUPP == err) )
    {     /* Retry with standard file reader. */
      connection->resp_sender = MHD_resp_sender_std;
      return MHD_ERR_AGAIN_;
    }
    if ( (ENOTCONN == err) ||
         (EPIPE == err) )
    {
      return MHD_ERR_CONNRESET_;
    }
    return MHD_ERR_BADF_;   /* Fail hard */
#endif /* HAVE_SOLARIS_SENDFILE */
  }
#ifdef EPOLL_SUPPORT
  else if (send_size > (size_t) ret)
    connection->epoll_state &=
      ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
#elif defined(HAVE_FREEBSD_SENDFILE)
#ifdef SF_FLAGS
  flags = used_thr_p_c ?
          freebsd_sendfile_flags_thd_p_c_ : freebsd_sendfile_flags_;
#endif /* SF_FLAGS */
  if (0 != sendfile (file_fd,
                     connection->socket_fd,
                     (off_t) offsetu64,
                     send_size,
                     NULL,
                     &sent_bytes,
                     flags))
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EAGAIN_ (err) ||
        MHD_SCKT_ERR_IS_EINTR_ (err) ||
        (EBUSY == err) )
    {
      mhd_assert (SSIZE_MAX >= sent_bytes);
      if (0 != sent_bytes)
        return (ssize_t) sent_bytes;

      return MHD_ERR_AGAIN_;
    }
    /* Some unrecoverable error. Possibly file FD is not suitable
     * for sendfile(). Retry with standard send(). */
    connection->resp_sender = MHD_resp_sender_std;
    return MHD_ERR_AGAIN_;
  }
  mhd_assert (0 < sent_bytes);
  mhd_assert (SSIZE_MAX >= sent_bytes);
  ret = (ssize_t) sent_bytes;
#elif defined(HAVE_DARWIN_SENDFILE)
  len = (off_t) send_size; /* chunk always fit */
  if (0 != sendfile (file_fd,
                     connection->socket_fd,
                     (off_t) offsetu64,
                     &len,
                     NULL,
                     0))
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EAGAIN_ (err) ||
        MHD_SCKT_ERR_IS_EINTR_ (err))
    {
      mhd_assert (0 <= len);
      mhd_assert (SSIZE_MAX >= len);
      mhd_assert (send_size >= (size_t) len);
      if (0 != len)
        return (ssize_t) len;

      return MHD_ERR_AGAIN_;
    }
    if ((ENOTCONN == err) ||
        (EPIPE == err) )
      return MHD_ERR_CONNRESET_;
    if ((ENOTSUP == err) ||
        (EOPNOTSUPP == err) )
    {     /* This file FD is not suitable for sendfile().
           * Retry with standard send(). */
      connection->resp_sender = MHD_resp_sender_std;
      return MHD_ERR_AGAIN_;
    }
    return MHD_ERR_BADF_;   /* Return hard error. */
  }
  mhd_assert (0 <= len);
  mhd_assert (SSIZE_MAX >= len);
  mhd_assert (send_size >= (size_t) len);
  ret = (ssize_t) len;
#endif /* HAVE_FREEBSD_SENDFILE */

  /* If there is a need to push the data from network buffers
   * call post_send_setopt(). */
  /* It's unknown whether sendfile() will be used in the next
   * response so assume that next response will be the same. */
  if ( (push_data) &&
       (send_size == (size_t) ret) )
    post_send_setopt (connection, false, push_data);

  return ret;
}


#endif /* _MHD_HAVE_SENDFILE */

#if defined(MHD_VECT_SEND)


/**
 * Function sends iov data by system sendmsg or writev function.
 *
 * Connection must be in non-TLS (non-HTTPS) mode.
 *
 * @param connection the MHD connection structure
 * @param r_iov the pointer to iov data structure with tracking
 * @param push_data set to true to force push the data to the network from
 *                  system buffers (usually set for the last piece of data),
 *                  set to false to prefer holding incomplete network packets
 *                  (more data will be send for the same reply).
 * @return actual number of bytes sent
 */
static ssize_t
send_iov_nontls (struct MHD_Connection *connection,
                 struct MHD_iovec_track_ *const r_iov,
                 bool push_data)
{
  ssize_t res;
  ssize_t total_sent;
  size_t items_to_send;
#ifdef HAVE_SENDMSG
  struct msghdr msg;
#elif defined(MHD_WINSOCK_SOCKETS)
  DWORD bytes_sent;
  DWORD cnt_w;
#endif /* MHD_WINSOCK_SOCKETS */

  mhd_assert (0 == (connection->daemon->options & MHD_USE_TLS));

  if ( (MHD_INVALID_SOCKET == connection->socket_fd) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
  {
    return MHD_ERR_NOTCONN_;
  }

  items_to_send = r_iov->cnt - r_iov->sent;
#ifdef _MHD_IOV_MAX
  if (_MHD_IOV_MAX < items_to_send)
  {
    mhd_assert (0 < _MHD_IOV_MAX);
    if (0 == _MHD_IOV_MAX)
      return MHD_ERR_NOTCONN_; /* Should never happen */
    items_to_send = _MHD_IOV_MAX;
    push_data = false; /* Incomplete response */
  }
#endif /* _MHD_IOV_MAX */
#ifdef HAVE_SENDMSG
  memset (&msg, 0, sizeof(struct msghdr));
  msg.msg_iov = r_iov->iov + r_iov->sent;
  msg.msg_iovlen = items_to_send;

  pre_send_setopt (connection, true, push_data);
#ifdef MHD_USE_MSG_MORE
  res = sendmsg (connection->socket_fd, &msg,
                 MSG_NOSIGNAL_OR_ZERO | (push_data ? 0 : MSG_MORE));
#else  /* ! MHD_USE_MSG_MORE */
  res = sendmsg (connection->socket_fd, &msg, MSG_NOSIGNAL_OR_ZERO);
#endif /* ! MHD_USE_MSG_MORE */
#elif defined(HAVE_WRITEV)
  pre_send_setopt (connection, true, push_data);
  res = writev (connection->socket_fd, r_iov->iov + r_iov->sent,
                items_to_send);
#elif defined(MHD_WINSOCK_SOCKETS)
#ifdef _WIN64
  if (items_to_send > UINT32_MAX)
  {
    cnt_w = UINT32_MAX;
    push_data = false; /* Incomplete response */
  }
  else
    cnt_w = (DWORD) items_to_send;
#else  /* ! _WIN64 */
  cnt_w = (DWORD) items_to_send;
#endif /* ! _WIN64 */
  pre_send_setopt (connection, true, push_data);
  if (0 == WSASend (connection->socket_fd,
                    (LPWSABUF) (r_iov->iov + r_iov->sent),
                    cnt_w,
                    &bytes_sent, 0, NULL, NULL))
    res = (ssize_t) bytes_sent;
  else
    res = -1;
#else /* !HAVE_SENDMSG && !HAVE_WRITEV && !MHD_WINSOCK_SOCKETS */
#error No vector-send function available
#endif

  if (0 > res)
  {
    const int err = MHD_socket_get_error_ ();

    if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
    {
#ifdef EPOLL_SUPPORT
      /* EAGAIN --- no longer write-ready */
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
      return MHD_ERR_AGAIN_;
    }
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return MHD_ERR_AGAIN_;
    if (MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
      return MHD_ERR_CONNRESET_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EPIPE_))
      return MHD_ERR_PIPE_;
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

  /* Some data has been sent */
  total_sent = res;
  /* Adjust the internal tracking information for the iovec to
   * take this last send into account. */
  while ((0 != res) && (r_iov->iov[r_iov->sent].iov_len <= (size_t) res))
  {
    res -= r_iov->iov[r_iov->sent].iov_len;
    r_iov->sent++; /* The iov element has been completely sent */
    mhd_assert ((r_iov->cnt > r_iov->sent) || (0 == res));
  }

  if (r_iov->cnt == r_iov->sent)
    post_send_setopt (connection, true, push_data);
  else
  {
#ifdef EPOLL_SUPPORT
    connection->epoll_state &=
      ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
#endif /* EPOLL_SUPPORT */
    if (0 != res)
    {
      mhd_assert (r_iov->cnt > r_iov->sent);
      /* The last iov element has been partially sent */
      r_iov->iov[r_iov->sent].iov_base =
        (void *) ((uint8_t *) r_iov->iov[r_iov->sent].iov_base + (size_t) res);
      r_iov->iov[r_iov->sent].iov_len -= (MHD_iov_size_) res;
    }
  }

  return total_sent;
}


#endif /* MHD_VECT_SEND */

#if ! defined(MHD_VECT_SEND) || defined(HTTPS_SUPPORT) || \
  defined(_MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED)


/**
 * Function sends iov data by sending buffers one-by-one by standard
 * data send function.
 *
 * Connection could be in HTTPS or non-HTTPS mode.
 *
 * @param connection the MHD connection structure
 * @param r_iov the pointer to iov data structure with tracking
 * @param push_data set to true to force push the data to the network from
 *                  system buffers (usually set for the last piece of data),
 *                  set to false to prefer holding incomplete network packets
 *                  (more data will be send for the same reply).
 * @return actual number of bytes sent
 */
static ssize_t
send_iov_emu (struct MHD_Connection *connection,
              struct MHD_iovec_track_ *const r_iov,
              bool push_data)
{
  const bool non_blk = connection->sk_nonblck;
  size_t total_sent;
  ssize_t res;

  mhd_assert (NULL != r_iov->iov);
  total_sent = 0;
  do
  {
    if ((size_t) SSIZE_MAX - total_sent < r_iov->iov[r_iov->sent].iov_len)
      return total_sent; /* return value would overflow */

    res = MHD_send_data_ (connection,
                          r_iov->iov[r_iov->sent].iov_base,
                          r_iov->iov[r_iov->sent].iov_len,
                          push_data && (r_iov->cnt == r_iov->sent + 1));
    if (0 > res)
    {
      /* Result is an error */
      if (0 == total_sent)
        return res; /* Nothing was sent, return result as is */

      if (MHD_ERR_AGAIN_ == res)
        return total_sent; /* Some data has been sent, return the amount */

      return res; /* Any kind of a hard error */
    }

    total_sent += (size_t) res;

    if (r_iov->iov[r_iov->sent].iov_len != (size_t) res)
    {
      /* Incomplete buffer has been sent.
       * Adjust buffer of the last element. */
      r_iov->iov[r_iov->sent].iov_base =
        (void *) ((uint8_t *) r_iov->iov[r_iov->sent].iov_base + (size_t) res);
      r_iov->iov[r_iov->sent].iov_len -= res;

      return total_sent;
    }
    /* The iov element has been completely sent */
    r_iov->sent++;
  } while ((r_iov->cnt > r_iov->sent) && (non_blk));

  return (ssize_t) total_sent;
}


#endif /* !MHD_VECT_SEND || HTTPS_SUPPORT
          || _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED */


ssize_t
MHD_send_iovec_ (struct MHD_Connection *connection,
                 struct MHD_iovec_track_ *const r_iov,
                 bool push_data)
{
#ifdef MHD_VECT_SEND
#if defined(HTTPS_SUPPORT) || \
  defined(_MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED)
  bool use_iov_send = true;
#endif /* HTTPS_SUPPORT || _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED */
#endif /* MHD_VECT_SEND */

  mhd_assert (NULL != connection->resp_iov.iov);
  mhd_assert (NULL != connection->response->data_iov);
  mhd_assert (connection->resp_iov.cnt > connection->resp_iov.sent);
#ifdef MHD_VECT_SEND
#if defined(HTTPS_SUPPORT) || \
  defined(_MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED)
#ifdef HTTPS_SUPPORT
  use_iov_send = use_iov_send &&
                 (0 == (connection->daemon->options & MHD_USE_TLS));
#endif /* HTTPS_SUPPORT */
#ifdef _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED
  use_iov_send = use_iov_send && (connection->daemon->sigpipe_blocked ||
                                  connection->sk_spipe_suppress);
#endif /* _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED */
  if (use_iov_send)
#endif /* HTTPS_SUPPORT || _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED */
  return send_iov_nontls (connection, r_iov, push_data);
#endif /* MHD_VECT_SEND */

#if ! defined(MHD_VECT_SEND) || defined(HTTPS_SUPPORT) || \
  defined(_MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED)
  return send_iov_emu (connection, r_iov, push_data);
#endif /* !MHD_VECT_SEND || HTTPS_SUPPORT
          || _MHD_VECT_SEND_NEEDS_SPIPE_SUPPRESSED */
}
