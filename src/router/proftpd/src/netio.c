/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2001-2009 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, the ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* NetIO routines
 * $Id: netio.c,v 1.39 2009/03/05 06:01:51 castaglia Exp $
 */

#include "conf.h"
#include <signal.h>

/* See RFC 854 for the definition of these Telnet values */

/* Telnet "Interpret As Command" indicator */
#ifndef TELNET_IAC
# define TELNET_IAC	255
#endif

#ifndef TELNET_DONT
# define TELNET_DONT	254
#endif

#ifndef TELNET_DO
# define TELNET_DO	253
#endif

#ifndef TELNET_WONT
# define TELNET_WONT	252
#endif

#ifndef TELNET_WILL
# define TELNET_WILL	251
#endif

/* Telnet "Interrupt Process" code */
#ifndef TELNET_IP
# define TELNET_IP	244
#endif

/* Telnet "Data Mark" code */
#ifndef TELNET_DM
# define TELNET_DM	242
#endif

static const char *trace_channel = "netio";

static pr_netio_t *core_ctrl_netio = NULL, *ctrl_netio = NULL;
static pr_netio_t *core_data_netio = NULL, *data_netio = NULL;
static pr_netio_t *core_othr_netio = NULL, *othr_netio = NULL;

/* Used to track whether the previous text read from the client's control
 * connection was a properly-terminated command.  If so, then read in the
 * next/current text as per normal.  If NOT (e.g. the client sent a too-long
 * command), then read in the next/current text, but ignore it.  Only clear
 * this flag if the next/current command can be read as per normal.
 *
 * The pr_netio_telnet_gets() uses this variable, in conjunction with its
 * saw_newline flag, for handling too-long commands from clients.
 */
static int properly_terminated_prev_command = TRUE;

static pr_netio_stream_t *netio_stream_alloc(pool *parent_pool) {
  pool *netio_pool = NULL;
  pr_netio_stream_t *nstrm = NULL;

  if (!parent_pool) {
    errno = EINVAL;
    return NULL;
  }

  netio_pool = make_sub_pool(parent_pool);
  nstrm = pcalloc(netio_pool, sizeof(pr_netio_stream_t));

  nstrm->strm_pool = netio_pool;
  nstrm->strm_fd = -1;
  nstrm->strm_mode = 0;
  nstrm->strm_flags = 0;
  nstrm->strm_buf = NULL;
  nstrm->strm_data = NULL;
  nstrm->strm_errno = 0;

  return nstrm;
}

static pr_buffer_t *netio_buffer_alloc(pr_netio_stream_t *nstrm) {
  size_t bufsz;
  pr_buffer_t *pbuf = NULL;

  pbuf = pcalloc(nstrm->strm_pool, sizeof(pr_buffer_t));

  /* Allocate a buffer. */
  bufsz = pr_config_get_xfer_bufsz();
  pbuf->buf = pcalloc(nstrm->strm_pool, bufsz);
  pbuf->buflen = bufsz;

  /* Position the offset at the start of the buffer, and set the
   * remaining bytes value accordingly.
   */
  pbuf->current = pbuf->buf;
  pbuf->remaining = bufsz;

  /* Add this buffer to the given stream. */
  nstrm->strm_buf = pbuf;

  return pbuf;
}

/* Default core NetIO handlers
 */

static void core_netio_abort_cb(pr_netio_stream_t *nstrm) {
  nstrm->strm_flags |= PR_NETIO_SESS_ABORT;
}

static int core_netio_close_cb(pr_netio_stream_t *nstrm) {
  int res;

  res = close(nstrm->strm_fd);
  nstrm->strm_fd = -1;

  return res;
}

static pr_netio_stream_t *core_netio_open_cb(pr_netio_stream_t *nstrm, int fd,
    int mode) {

  nstrm->strm_fd = fd;

  /* The stream's strm_mode field does not need to be set, as it is set
   * by the NetIO layer's open() wrapper function.
   */

  return nstrm;
}

static int core_netio_poll_cb(pr_netio_stream_t *nstrm) {
  int res;
  fd_set rfds, *rfdsp, wfds, *wfdsp;
  struct timeval tval;

  FD_ZERO(&rfds);
  rfdsp = NULL;
  FD_ZERO(&wfds);
  wfdsp = NULL;

  if (nstrm->strm_mode == PR_NETIO_IO_RD) {
    if (nstrm->strm_fd >= 0) {
      FD_SET(nstrm->strm_fd, &rfds);
      rfdsp = &rfds;
    }

  } else {
    if (nstrm->strm_fd >= 0) {
      FD_SET(nstrm->strm_fd, &wfds);
      wfdsp = &wfds;
    }
  }

  tval.tv_sec = ((nstrm->strm_flags & PR_NETIO_SESS_INTR) ?
    nstrm->strm_interval: 60);
  tval.tv_usec = 0;

  res = select(nstrm->strm_fd + 1, rfdsp, wfdsp, NULL, &tval);
  return res;
}

static int core_netio_postopen_cb(pr_netio_stream_t *nstrm) {
  return 0;
}

static int core_netio_read_cb(pr_netio_stream_t *nstrm, char *buf,
    size_t buflen) {
  return read(nstrm->strm_fd, buf, buflen);
}

static pr_netio_stream_t *core_netio_reopen_cb(pr_netio_stream_t *nstrm, int fd,
    int mode) {

  if (nstrm->strm_fd != -1)
    close(nstrm->strm_fd);

  nstrm->strm_fd = fd;
  nstrm->strm_mode = mode;

  return nstrm;
}

static int core_netio_shutdown_cb(pr_netio_stream_t *nstrm, int how) {
  return shutdown(nstrm->strm_fd, how);
}

static int core_netio_write_cb(pr_netio_stream_t *nstrm, char *buf,
    size_t buflen) {
  return write(nstrm->strm_fd, buf, buflen);
}

/* NetIO API wrapper functions.
 */

void pr_netio_abort(pr_netio_stream_t *nstrm) {

  if (!nstrm) {
    errno = EINVAL;
    return;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_CTRL)
    ctrl_netio ? ctrl_netio->abort(nstrm) :
      core_ctrl_netio->abort(nstrm);

  if (nstrm->strm_type == PR_NETIO_STRM_DATA)
    data_netio ? data_netio->abort(nstrm) :
      core_data_netio->abort(nstrm);

  if (nstrm->strm_type == PR_NETIO_STRM_OTHR)
    othr_netio ? othr_netio->abort(nstrm) :
      core_othr_netio->abort(nstrm);

  return;
}

int pr_netio_close(pr_netio_stream_t *nstrm) {
  int res = -1;

  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_CTRL) {
    res = ctrl_netio ? ctrl_netio->close(nstrm) :
      core_ctrl_netio->close(nstrm);
    destroy_pool(nstrm->strm_pool);
    return res;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_DATA) {
    res = data_netio ? data_netio->close(nstrm) :
      core_data_netio->close(nstrm);
    destroy_pool(nstrm->strm_pool);
    return res;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_OTHR) {
    res = othr_netio ? othr_netio->close(nstrm) :
      core_othr_netio->close(nstrm);
    destroy_pool(nstrm->strm_pool);
    return res;
  }

  errno = EPERM;
  return res;
}

static int netio_lingering_close(pr_netio_stream_t *nstrm, long linger,
    int flags) {
  int res;

  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_fd < 0)
    /* Already closed. */
    return 0;

  if (!(flags & NETIO_LINGERING_CLOSE_FL_NO_SHUTDOWN))
    pr_netio_shutdown(nstrm, 1);

  if (nstrm->strm_fd >= 0) {
    struct timeval tv;
    fd_set rfds;
    time_t when = time(NULL) + linger;

    tv.tv_sec = linger;
    tv.tv_usec = 0L;

    /* Handle timers during reading, once selected for read this
     * should mean all buffers have been flushed and the receiving end
     * has closed.
     */
    while (TRUE) {
      run_schedule();

      FD_ZERO(&rfds);
      FD_SET(nstrm->strm_fd, &rfds);

      pr_trace_msg(trace_channel, 8,
        "lingering %lu secs before closing fd %d", (unsigned long) tv.tv_sec,
        nstrm->strm_fd);

      res = select(nstrm->strm_fd+1, &rfds, NULL, NULL, &tv);
      if (res == -1) {
        if (errno == EINTR) {
          time_t now = time(NULL);
          pr_signals_handle();

          /* Still here? If the requested lingering interval hasn't passed,
           * continue lingering.  Reset the timeval struct's fields to
           * linger for the interval remaining in the given period of time.
           */
          if (now < when) {
            tv.tv_sec = when - now;
            tv.tv_usec = 0L;
            continue;
          }

        } else {
          nstrm->strm_errno = errno;
          return -1;
        }

      } else {
        if (FD_ISSET(nstrm->strm_fd, &rfds)) {
          pr_trace_msg(trace_channel, 8,
            "received data for reading on fd %d, ignoring", nstrm->strm_fd);
        }
      }

      break;
    }
  }

  pr_trace_msg(trace_channel, 8, "done lingering, closing fd %d",
    nstrm->strm_fd);

  if (nstrm->strm_type == PR_NETIO_STRM_CTRL)
    return ctrl_netio ? ctrl_netio->close(nstrm) :
      core_ctrl_netio->close(nstrm);

  if (nstrm->strm_type == PR_NETIO_STRM_DATA)
    return data_netio ? data_netio->close(nstrm) :
      core_data_netio->close(nstrm);

  if (nstrm->strm_type == PR_NETIO_STRM_OTHR)
    return othr_netio ? othr_netio->close(nstrm) :
      core_othr_netio->close(nstrm);

  errno = EPERM;
  return -1;
}

int pr_netio_lingering_abort(pr_netio_stream_t *nstrm, long linger) {
  int res;

  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  /* Send an appropriate response code down the stream asychronously. */
  pr_response_send_async(R_426, _("Transfer aborted. Data connection closed."));

  pr_netio_shutdown(nstrm, 1);

  if (nstrm->strm_fd >= 0) {
    fd_set rs;
    struct timeval tv;

    /* Wait for just a little while for the shutdown to take effect. */
    tv.tv_sec = 0L;
    tv.tv_usec = 300000L;

    while (TRUE) {
      run_schedule();

      FD_ZERO(&rs);
      FD_SET(nstrm->strm_fd, &rs);

      res = select(nstrm->strm_fd+1, &rs, NULL, NULL, &tv);
      if (res == -1) {
        if (errno == EINTR) {
          pr_signals_handle();

          /* Linger some more. */
          tv.tv_sec = 0L;
          tv.tv_usec = 300000L;
          continue;

        } else {
          nstrm->strm_errno = errno;
          return -1;
        }
      }

      break;
    }
  }

  /* Now continue with a normal lingering close. */
  return netio_lingering_close(nstrm, linger,
    NETIO_LINGERING_CLOSE_FL_NO_SHUTDOWN);  
}

int pr_netio_lingering_close(pr_netio_stream_t *nstrm, long linger) {
  return netio_lingering_close(nstrm, linger, 0);
}

pr_netio_stream_t *pr_netio_open(pool *parent_pool, int strm_type, int fd,
    int mode) {
  pr_netio_stream_t *nstrm = NULL;

  if (!parent_pool) {
    errno = EINVAL;
    return NULL;
  }

  /* Create a new stream object, then pass that the NetIO open handler. */
  nstrm = netio_stream_alloc(parent_pool);

  if (strm_type == PR_NETIO_STRM_CTRL) {
    nstrm->strm_type = PR_NETIO_STRM_CTRL;
    nstrm->strm_mode = mode;
    return ctrl_netio ? (ctrl_netio->open)(nstrm, fd, mode) :
      (core_ctrl_netio->open)(nstrm, fd, mode);
  }

  if (strm_type == PR_NETIO_STRM_DATA) {
    nstrm->strm_type = PR_NETIO_STRM_DATA;
    nstrm->strm_mode = mode;
    return data_netio ? (data_netio->open)(nstrm, fd, mode) :
      (core_data_netio->open)(nstrm, fd, mode);
  }

  if (strm_type == PR_NETIO_STRM_OTHR) {
    nstrm->strm_type = PR_NETIO_STRM_OTHR;
    nstrm->strm_mode = mode;
    return othr_netio ? (othr_netio->open)(nstrm, fd, mode) :
      (core_othr_netio->open)(nstrm, fd, mode);
  }

  destroy_pool(nstrm->strm_pool);
  nstrm->strm_pool = NULL;

  errno = EPERM;
  return NULL;
}

pr_netio_stream_t *pr_netio_reopen(pr_netio_stream_t *nstrm, int fd, int mode) {

  if (!nstrm) {
    errno = EINVAL;
    return NULL;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_CTRL)
    return ctrl_netio ? (ctrl_netio->reopen)(nstrm, fd, mode) :
      (core_ctrl_netio->reopen)(nstrm, fd, mode);

  if (nstrm->strm_type == PR_NETIO_STRM_DATA)
    return data_netio ? (data_netio->reopen)(nstrm, fd, mode) :
      (core_data_netio->reopen)(nstrm, fd, mode);

  if (nstrm->strm_type == PR_NETIO_STRM_OTHR)
    return othr_netio ? (othr_netio->reopen)(nstrm, fd, mode) :
      (core_othr_netio->reopen)(nstrm, fd, mode);

  errno = EPERM;
  return NULL;
}

void pr_netio_reset_poll_interval(pr_netio_stream_t *nstrm) {
  if (!nstrm) {
    errno = EINVAL;
    return;
  }

  /* Simply clear the "interruptible" flag. */
  nstrm->strm_flags &= ~PR_NETIO_SESS_INTR;
}

void pr_netio_set_poll_interval(pr_netio_stream_t *nstrm, unsigned int secs) {

  if (!nstrm) {
    errno = EINVAL;
    return;
  }

  nstrm->strm_flags |= PR_NETIO_SESS_INTR;
  nstrm->strm_interval = secs;
}

int pr_netio_poll(pr_netio_stream_t *nstrm) {
  int res = 0;

  /* Sanity checks. */
  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_fd == -1) {
    errno = EBADF;
    return -1;
  }

  /* Has this stream been aborted? */
  if (nstrm->strm_flags & PR_NETIO_SESS_ABORT) {
    nstrm->strm_flags &= ~PR_NETIO_SESS_ABORT;
    return 1;
  }

  while (TRUE) {
    run_schedule();
    pr_signals_handle();

    switch (nstrm->strm_type) {
      case PR_NETIO_STRM_CTRL:
        res = ctrl_netio ? (ctrl_netio->poll)(nstrm) :
          (core_ctrl_netio->poll)(nstrm);
        break;

      case PR_NETIO_STRM_DATA:
        res = data_netio ? (data_netio->poll)(nstrm) :
          (core_data_netio->poll)(nstrm);
        break;

      case PR_NETIO_STRM_OTHR:
        res = othr_netio ? (othr_netio->poll)(nstrm) :
          (core_othr_netio->poll)(nstrm);
        break;
    }

    switch (res) {
      case -1:
        if (errno == EINTR) {
          if (nstrm->strm_flags & PR_NETIO_SESS_ABORT) {
            nstrm->strm_flags &= ~PR_NETIO_SESS_ABORT;
            return 1;
          }

	  /* Otherwise, restart the call */
          pr_signals_handle();
          continue;
        }

        /* Some other error occured */
        nstrm->strm_errno = errno;

        /* If this is the control stream, and the error indicates a
         * broken pipe (i.e. the client went away), AND there is a data
         * transfer is progress, abort the transfer.
         */
        if (errno == EPIPE &&
            nstrm->strm_type == PR_NETIO_STRM_CTRL &&
            (session.sf_flags & SF_XFER)) {
          pr_trace_msg(trace_channel, 5,
            "received EPIPE on control connection, setting 'aborted' "
            "session flag");
          session.sf_flags |= SF_ABORT;
        }

        return -1;

      case 0:
        /* In case the kernel doesn't support interrupted syscalls. */
        if (nstrm->strm_flags & PR_NETIO_SESS_ABORT) {
          nstrm->strm_flags &= ~PR_NETIO_SESS_ABORT;
          return 1;
        }

        /* If the stream has been marked as "interruptible", AND the
         * poll interval is zero seconds (meaning a true poll, not blocking),
         * then return here.
         */
        if ((nstrm->strm_flags & PR_NETIO_SESS_INTR) &&
            nstrm->strm_interval == 0) {
          errno = EOF;
          return -1;
        }

        continue;

      default:
        return 0;
    }
  }

  /* This will never be reached. */
  return -1;
}

int pr_netio_postopen(pr_netio_stream_t *nstrm) {
  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_CTRL)
    return ctrl_netio ? (ctrl_netio->postopen)(nstrm) :
      (core_ctrl_netio->postopen)(nstrm);

  if (nstrm->strm_type == PR_NETIO_STRM_DATA)
    return data_netio ? (data_netio->postopen)(nstrm) :
      (core_data_netio->postopen)(nstrm);

  if (nstrm->strm_type == PR_NETIO_STRM_OTHR)
    return othr_netio ? (othr_netio->postopen)(nstrm) :
      (core_othr_netio->postopen)(nstrm);

  errno = EPERM;
  return -1;
}

int pr_netio_printf(pr_netio_stream_t *nstrm, const char *fmt, ...) {
  va_list msg;
  char buf[PR_RESPONSE_BUFFER_SIZE] = {'\0'};

  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  va_start(msg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, msg);
  va_end(msg);
  buf[sizeof(buf)-1] = '\0';

  return pr_netio_write(nstrm, buf, strlen(buf));
}

int pr_netio_printf_async(pr_netio_stream_t *nstrm, char *fmt, ...) {
  va_list msg;
  char buf[PR_RESPONSE_BUFFER_SIZE] = {'\0'};

  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  va_start(msg, fmt);
  vsnprintf(buf, sizeof(buf), fmt, msg);
  va_end(msg);
  buf[sizeof(buf)-1] = '\0';

  return pr_netio_write_async(nstrm, buf, strlen(buf));
}

int pr_netio_write(pr_netio_stream_t *nstrm, char *buf, size_t buflen) {
  int bwritten = 0, total = 0;

  /* Sanity check */
  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_fd == -1) {
    errno = (nstrm->strm_errno ? nstrm->strm_errno : EBADF);
    return -1;
  }

  while (buflen) {

    switch (pr_netio_poll(nstrm)) {
      case 1:
        return -2;

      case -1:
        return -1;

      default:
        /* We have to potentially restart here as well, in case we get EINTR. */
        do {
          pr_signals_handle(); 
          run_schedule();

          switch (nstrm->strm_type) {
            case PR_NETIO_STRM_CTRL:
              bwritten = ctrl_netio ? (ctrl_netio->write)(nstrm, buf, buflen) :
                (core_ctrl_netio->write)(nstrm, buf, buflen);
                break;

            case PR_NETIO_STRM_DATA:
              if (XFER_ABORTED)
                break;

              bwritten = data_netio ? (data_netio->write)(nstrm, buf, buflen) :
                (core_data_netio->write)(nstrm, buf, buflen);
              break;

            case PR_NETIO_STRM_OTHR:
              bwritten = othr_netio ? (othr_netio->write)(nstrm, buf, buflen) :
                (core_othr_netio->write)(nstrm, buf, buflen);
              break;
          }

        } while (bwritten == -1 && errno == EINTR);
        break;
    }

    if (bwritten == -1) {
      nstrm->strm_errno = errno;
      return -1;
    }

    buf += bwritten;
    total += bwritten;
    buflen -= bwritten;
  }

  return total;
}

int pr_netio_write_async(pr_netio_stream_t *nstrm, char *buf, size_t buflen) {
  int flags = 0;
  int bwritten = 0, total = 0;

  /* Sanity check */
  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_fd == -1) {
    errno = (nstrm->strm_errno ? nstrm->strm_errno : EBADF);
    return -1;
  }

  /* Prepare the descriptor for nonblocking IO. */
  if ((flags = fcntl(nstrm->strm_fd, F_GETFL)) == -1)
    return -1;

  if (fcntl(nstrm->strm_fd, F_SETFL, flags|O_NONBLOCK) == -1)
    return -1;

  while (buflen) {
    do {

      /* Do NOT check for XFER_ABORTED here.  After a client aborts a
       * transfer, proftpd still needs to send the 426 response code back
       * to the client via the control connection; checking for XFER_ABORTED
       * here would block that response code sending, which in turn causes
       * problems for clients waiting for that response code.
       */

      pr_signals_handle();

      switch (nstrm->strm_type) {
        case PR_NETIO_STRM_CTRL:
          bwritten = ctrl_netio ? (ctrl_netio->write)(nstrm, buf, buflen) :
            (core_ctrl_netio->write)(nstrm, buf, buflen);
          break;

        case PR_NETIO_STRM_DATA:
          bwritten = data_netio ? (data_netio->write)(nstrm, buf, buflen) :
            (core_data_netio->write)(nstrm, buf, buflen);
          break;

        case PR_NETIO_STRM_OTHR:
          bwritten = othr_netio ? (othr_netio->write)(nstrm, buf, buflen) :
            (core_othr_netio->write)(nstrm, buf, buflen);
          break;
      }

    } while (bwritten == -1 && errno == EINTR);

    if (bwritten < 0) {
      nstrm->strm_errno = errno;
      fcntl(nstrm->strm_fd, F_SETFL, flags);

      if (nstrm->strm_errno == EWOULDBLOCK)
        /* Give up ... */
        return total;

      return -1;
    }

    buf += bwritten;
    total += bwritten;
    buflen -= bwritten;
  }

  fcntl(nstrm->strm_fd, F_SETFL, flags);
  return total;
}

int pr_netio_read(pr_netio_stream_t *nstrm, char *buf, size_t buflen,
    int bufmin) {
  int bread = 0, total = 0;

  /* Sanity check. */
  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_fd == -1) {
    errno = (nstrm->strm_errno ? nstrm->strm_errno : EBADF);
    return -1;
  }

  if (bufmin < 1)
    bufmin = 1;

  if (bufmin > buflen)
    bufmin = buflen;

  while (bufmin > 0) {
    polling:

    switch (pr_netio_poll(nstrm)) {
      case 1:
        return -2;

      case -1:
        return -1;

      default:
        do {
          pr_signals_handle();

          run_schedule();

          switch (nstrm->strm_type) {
            case PR_NETIO_STRM_CTRL:
              bread = ctrl_netio ? (ctrl_netio->read)(nstrm, buf, buflen) :
                (core_ctrl_netio->read)(nstrm, buf, buflen);
                break;

            case PR_NETIO_STRM_DATA:
              if (XFER_ABORTED)
                break;

              bread = data_netio ? (data_netio->read)(nstrm, buf, buflen) :
                (core_data_netio->read)(nstrm, buf, buflen);
              break;

            case PR_NETIO_STRM_OTHR:
              bread = othr_netio ? (othr_netio->read)(nstrm, buf, buflen) :
                (core_othr_netio->read)(nstrm, buf, buflen);
              break;
          }

#ifdef EAGAIN
	  if (bread == -1 && errno == EAGAIN)
            goto polling;
#endif

        } while (bread == -1 && errno == EINTR);
        break;
    }

    if (bread == -1) {
      nstrm->strm_errno = errno;
      return -1;
    }

    /* EOF? */
    if (bread == 0) {
      if (nstrm->strm_type == PR_NETIO_STRM_CTRL) {
        pr_trace_msg(trace_channel, 7,
          "read %d bytes from control stream fd %d, handling as EOF", bread,
          nstrm->strm_fd);
      }

      nstrm->strm_errno = 0;
      break;
    }

    buf += bread;
    total += bread;
    bufmin -= bread;
    buflen -= bread;
  }

  return total;
}

int pr_netio_shutdown(pr_netio_stream_t *nstrm, int how) {
  int res = -1;

  if (!nstrm) {
    errno = EINVAL;
    return -1;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_CTRL) {
    res = ctrl_netio ? (ctrl_netio->shutdown)(nstrm, how) :
      (core_ctrl_netio->shutdown)(nstrm, how);
    return res;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_DATA) {
    res = data_netio ? (data_netio->shutdown)(nstrm, how) :
      (core_data_netio->shutdown)(nstrm, how);
    return res;
  }

  if (nstrm->strm_type == PR_NETIO_STRM_OTHR) {
    res = othr_netio ? (othr_netio->shutdown)(nstrm, how) :
      (core_othr_netio->shutdown)(nstrm, how);
    return res;
  }

  errno = EPERM;
  return res;
}

char *pr_netio_gets(char *buf, size_t buflen, pr_netio_stream_t *nstrm) {
  char *bp = buf;
  int toread;
  pr_buffer_t *pbuf = NULL;

  if (buflen == 0) {
    errno = EINVAL;
    return NULL;
  }

  buflen--;

  if (nstrm->strm_buf)
    pbuf = nstrm->strm_buf;
  else
    pbuf = netio_buffer_alloc(nstrm);

  while (buflen) {

    /* Is the buffer empty? */
    if (!pbuf->current ||
        pbuf->remaining == pbuf->buflen) {

      toread = pr_netio_read(nstrm, pbuf->buf,
        (buflen < pbuf->buflen ?  buflen : pbuf->buflen), 1);

      if (toread <= 0) {
        if (bp != buf) {
          *bp = '\0';
          return buf;

        } else
          return NULL;
      }

      pbuf->remaining = pbuf->buflen - toread;
      pbuf->current = pbuf->buf;

    } else
      toread = pbuf->buflen - pbuf->remaining;

    while (buflen && *pbuf->current != '\n' && toread--) {
      if (*pbuf->current & 0x80)
        pbuf->current++;

      else {
        *bp++ = *pbuf->current++;
        buflen--;
      }
      pbuf->remaining++;
    }

    if (buflen && toread && *pbuf->current == '\n') {
      buflen--;
      toread--;
      *bp++ = *pbuf->current++;
      pbuf->remaining++;
      break;
    }

    if (!toread)
      pbuf->current = NULL;
  }

  *bp = '\0';
  return buf;
}

static int telnet_mode = 0;

char *pr_netio_telnet_gets(char *buf, size_t buflen,
    pr_netio_stream_t *in_nstrm, pr_netio_stream_t *out_nstrm) {
  char *bp = buf;
  unsigned char cp;
  int toread, handle_iac = TRUE, saw_newline = FALSE;
  pr_buffer_t *pbuf = NULL;

  if (buflen == 0) {
    errno = EINVAL;
    return NULL;
  }

#ifdef PR_USE_NLS
  handle_iac = pr_encode_supports_telnet_iac();
#endif /* PR_USE_NLS */

  buflen--;

  if (in_nstrm->strm_buf)
    pbuf = in_nstrm->strm_buf;
  else
    pbuf = netio_buffer_alloc(in_nstrm);

  while (buflen) {

    /* Is the buffer empty? */
    if (!pbuf->current ||
        pbuf->remaining == pbuf->buflen) {

      toread = pr_netio_read(in_nstrm, pbuf->buf,
        (buflen < pbuf->buflen ?  buflen : pbuf->buflen), 1);

      if (toread <= 0) {
        if (bp != buf) {
          *bp = '\0';
          return buf;

        } else {
          return NULL;
        }
      }

      pbuf->remaining = pbuf->buflen - toread;
      pbuf->current = pbuf->buf;

    } else
      toread = pbuf->buflen - pbuf->remaining;

    while (buflen && toread > 0 && *pbuf->current != '\n' && toread--) {
      cp = *pbuf->current++;
      pbuf->remaining++;

      if (handle_iac == TRUE) {
        switch (telnet_mode) {
          case TELNET_IAC:
            switch (cp) {
              case TELNET_WILL:
              case TELNET_WONT:
              case TELNET_DO:
              case TELNET_DONT:
              case TELNET_IP:
              case TELNET_DM:

                /* Why do we do this crazy thing where we set the "telnet mode"
                 * to be the action, and let the while loop, on the next pass,
                 * handle that action?  It's because we don't know, right now,
                 * whether there actually a "next byte" in the input buffer.
                 * There _should_ be -- but we can't be sure.  And that next
                 * byte is needed for properly responding with WONT/DONT
                 * responses.
                 */
                telnet_mode = cp;
                continue;

              default:
                /* In this case, we know that the previous byte was TELNET_IAC,
                 * but the current byte is not a value we care about.  So
                 * write the TELNET_IAC into the output buffer, break out of
                 * of the switch, and let that handle the writing of the
                 * current byte into the output buffer.
                 */
                *bp++ = TELNET_IAC;
                buflen--;

                telnet_mode = 0;
                break;
            }
            break;

          case TELNET_WILL:
          case TELNET_WONT:
            pr_netio_printf(out_nstrm, "%c%c%c", TELNET_IAC, TELNET_DONT, cp);
            telnet_mode = 0;
            continue;

          case TELNET_DO:
          case TELNET_DONT:
            pr_netio_printf(out_nstrm, "%c%c%c", TELNET_IAC, TELNET_WONT, cp);
            telnet_mode = 0;
            continue;

          case TELNET_IP:
          case TELNET_DM:
          default:
            if (cp == TELNET_IAC) {
              telnet_mode = cp;
              continue;
            }
            break;
        }
      }

      *bp++ = cp;
      buflen--;
    }

    if (buflen && toread && *pbuf->current == '\n') {
      buflen--;
      toread--;
      *bp++ = *pbuf->current++;
      pbuf->remaining++;

      saw_newline = TRUE;
      break;
    }

    if (!toread)
      pbuf->current = NULL;
  }

  if (!saw_newline) {
    /* If we haven't seen a newline, then assume the client is deliberately
     * sending a too-long command, trying to exploit buffer sizes and make
     * the server make some possibly bad assumptions.
     */

    properly_terminated_prev_command = FALSE;
    errno = E2BIG;
    return NULL;
  }

  if (!properly_terminated_prev_command) {
    properly_terminated_prev_command = TRUE;
    pr_log_pri(PR_LOG_NOTICE, "client sent too-long command, ignoring");
    errno = E2BIG;
    return NULL;
  }

  properly_terminated_prev_command = TRUE;
  *bp = '\0';
  return buf;
}

int pr_register_netio(pr_netio_t *netio, int strm_types) {

  if (!netio) {
    pr_netio_t *core_netio = NULL;

    /* Only instantiate the core NetIO objects once, reusing the same pointer.
     */
    if (!core_ctrl_netio)
      core_netio = core_ctrl_netio = pr_alloc_netio(permanent_pool);

    if (!core_data_netio)
      core_data_netio = core_netio ? core_netio :
        (core_netio = pr_alloc_netio(permanent_pool));

    if (!core_othr_netio)
      core_othr_netio = core_netio ? core_netio :
        (core_netio = pr_alloc_netio(permanent_pool));

    return 0;
  }

  if (!netio->abort || !netio->close || !netio->open || !netio->poll ||
      !netio->postopen || !netio->read || !netio->reopen ||
      !netio->shutdown || !netio->write) {
    errno = EINVAL;
    return -1;
  }

  if (strm_types & PR_NETIO_STRM_CTRL)
    ctrl_netio = netio;

  if (strm_types & PR_NETIO_STRM_DATA)
    data_netio = netio;

  if (strm_types & PR_NETIO_STRM_OTHR)
    othr_netio = netio;

  return 0;
}

int pr_unregister_netio(int strm_types) {

  if (!strm_types) {
    errno = EINVAL;
    return -1;
  }

  /* NOTE: consider using cleanups here in the future? */

  if (strm_types & PR_NETIO_STRM_CTRL)
    ctrl_netio = NULL;

  if (strm_types & PR_NETIO_STRM_DATA)
    data_netio = NULL;

  if (strm_types & PR_NETIO_STRM_OTHR)
    othr_netio = NULL;

  return 0;
}

extern pid_t mpid;

pr_netio_t *pr_alloc_netio(pool *parent_pool) {
  pr_netio_t *netio = NULL;
  pool *netio_pool = NULL;

  if (!parent_pool) {
    errno = EINVAL;
    return NULL;
  }

  netio_pool = make_sub_pool(parent_pool);

  /* If this is the daemon process, we are allocating a sub-pool from the
   * permanent_pool.  You might wonder why the daemon process needs netio
   * objects.  It doesn't, really -- but it's for use by all of the session
   * processes that will be forked.  They will be able to reuse the memory
   * already allocated for the main ctrl/data/other netios, as is.
   *
   * This being the case, we should label the sub-pool accordingly.
   */
  if (mpid == getpid()) {
    pr_pool_tag(netio_pool, "Shared Netio Pool");

  } else {
    pr_pool_tag(netio_pool, "netio pool");
  }

  netio = pcalloc(netio_pool, sizeof(pr_netio_t));
  netio->pool = netio_pool;

  /* Set the default NetIO handlers to the core handlers. */
  netio->abort = core_netio_abort_cb;
  netio->close = core_netio_close_cb;
  netio->open = core_netio_open_cb;
  netio->poll = core_netio_poll_cb;
  netio->postopen = core_netio_postopen_cb;
  netio->read = core_netio_read_cb;
  netio->reopen = core_netio_reopen_cb;
  netio->shutdown = core_netio_shutdown_cb;
  netio->write = core_netio_write_cb;

  return netio;
}

void init_netio(void) {
  signal(SIGPIPE, SIG_IGN);
  signal(SIGURG, SIG_IGN);

  pr_register_netio(NULL, 0);
}

