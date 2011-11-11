/*
 * ProFTPD - mod_sftp message format
 * Copyright (c) 2008-2011 TJ Saunders
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: msg.c,v 1.7 2011/05/23 21:03:12 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "msg.h"
#include "crypto.h"
#include "disconnect.h"

#ifdef HAVE_EXECINFO_H
# include <execinfo.h>
#endif

/* The scratch buffer used by getbuf() is a constant 8KB.  If the caller
 * requests a larger size than that, the request is fulfilled using the
 * caller-provided pool.
 */
static char msg_buf[8 * 1024];

static void log_stacktrace(void) {
#if defined(HAVE_EXECINFO_H) && \
    defined(HAVE_BACKTRACE) && \
    defined(HAVE_BACKTRACE_SYMBOLS)
  void *trace[PR_TUNABLE_CALLER_DEPTH];
  char **strings;
  size_t tracesz;

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "-----BEGIN STACK TRACE-----");

  tracesz = backtrace(trace, PR_TUNABLE_CALLER_DEPTH);
  strings = backtrace_symbols(trace, tracesz);
  if (strings != NULL) {
    register unsigned int i;

    for (i = 1; i < tracesz; i++) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "[%u] %s", i-1, strings[i]);
    }

    /* Prevent memory leaks. */
    free(strings);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining stacktrace symbols: %s", strerror(errno));
  }
 
  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "-----END STACK TRACE-----");
#endif
}

char *sftp_msg_getbuf(pool *p, size_t sz) {
  if (sz <= sizeof(msg_buf)) {
    return msg_buf;
  }

  return palloc(p, sz);
}

char sftp_msg_read_byte(pool *p, char **buf, uint32_t *buflen) {
  char byte = 0;

  (void) p;

  if (*buflen < sizeof(char)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read byte (buflen = %lu)",
      (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  memcpy(&byte, *buf, sizeof(char));
  (*buf) += sizeof(char);
  (*buflen) -= sizeof(char);

  return byte;
}

int sftp_msg_read_bool(pool *p, char **buf, uint32_t *buflen) {
  char bool = 0;

  (void) p;

  bool = sftp_msg_read_byte(p, buf, buflen);
  if (bool == 0)
    return 0;

  return 1;
}

char *sftp_msg_read_data(pool *p, char **buf, uint32_t *buflen,
    size_t datalen) {
  char *data = NULL;

  if (*buflen < datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read %lu bytes of raw data "
      "(buflen = %lu)", (unsigned long) datalen, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data = palloc(p, datalen);

  memcpy(data, *buf, datalen);
  (*buf) += datalen;
  (*buflen) -= datalen;

  return data;
}

uint32_t sftp_msg_read_int(pool *p, char **buf, uint32_t *buflen) {
  uint32_t val = 0;

  (void) p;

  if (*buflen < sizeof(uint32_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read int (buflen = %lu)",
      (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  memcpy(&val, *buf, sizeof(uint32_t));
  (*buf) += sizeof(uint32_t);
  (*buflen) -= sizeof(uint32_t);

  val = ntohl(val);
  return val;
}

BIGNUM *sftp_msg_read_mpint(pool *p, char **buf, uint32_t *buflen) {
  BIGNUM *mpint = NULL;
  const unsigned char *data = NULL;
  uint32_t datalen = 0;

  datalen = sftp_msg_read_int(p, buf, buflen);

  if (*buflen < datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read %lu bytes of mpint (buflen = %lu)",
      (unsigned long) datalen, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (datalen > (1024 * 16)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to handle mpint of %lu bytes",
      (unsigned long) datalen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data = (const unsigned char *) sftp_msg_read_data(p, buf, buflen, datalen);
  if (data == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read %lu bytes of mpint data",
      (unsigned long) datalen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (data[0] & 0x80) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: negative mpint numbers not supported");
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  mpint = BN_bin2bn(data, (int) datalen, NULL);
  if (mpint == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to convert binary mpint: %s",
      sftp_crypto_get_errors());
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  return mpint;
}

char *sftp_msg_read_string(pool *p, char **buf, uint32_t *buflen) {
  uint32_t len = 0;
  char *str = NULL;

  len = sftp_msg_read_int(p, buf, buflen);

  /* We can't use sftp_msg_read_data() here, since we need to allocate and
   * populate a buffer that is one byte longer than the len just read in,
   * for the terminating NUL.
   */

  if (*buflen < len) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read %lu bytes of string data "
      "(buflen = %lu)", (unsigned long) len, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  str = palloc(p, len + 1);

  memcpy(str, *buf, len);
  (*buf) += len;
  (*buflen) -= len;
  str[len] = '\0';

  return str;
}

void sftp_msg_write_byte(char **buf, uint32_t *buflen, char byte) {
  if (*buflen < sizeof(char)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write byte (buflen = %lu)",
      (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  memcpy(*buf, &byte, sizeof(char));
  (*buf) += sizeof(char);
  (*buflen) -= sizeof(char);
}

void sftp_msg_write_bool(char **buf, uint32_t *buflen, char bool) {
  sftp_msg_write_byte(buf, buflen, bool == 0 ? 0 : 1);
}

void sftp_msg_write_data(char **buf, uint32_t *buflen, const char *data,
   size_t datalen, int write_len) {

  if (write_len)
    sftp_msg_write_int(buf, buflen, datalen);

  if (*buflen < datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write %lu bytes of raw data "
      "(buflen = %lu)", (unsigned long) datalen, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (datalen > 0) {
    memcpy(*buf, data, datalen);
    (*buf) += datalen;
    (*buflen) -= datalen;
  }
}

void sftp_msg_write_int(char **buf, uint32_t *buflen, uint32_t val) {
  if (*buflen < sizeof(uint32_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write int (buflen = %lu)",
      (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  val = htonl(val);
  memcpy(*buf, &val, sizeof(uint32_t));
  (*buf) += sizeof(uint32_t);
  (*buflen) -= sizeof(uint32_t);
}

void sftp_msg_write_mpint(char **buf, uint32_t *buflen,
    const BIGNUM *mpint) {
  unsigned char *data = NULL;
  size_t datalen = 0;
  int res = 0;

  if (BN_is_zero(mpint)) {
    sftp_msg_write_int(buf, buflen, 0);
    return;
  }

  if (mpint->neg) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write mpint (negative numbers not "
      "supported)");
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  datalen = BN_num_bytes(mpint) + 1;

  if (*buflen < datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write %lu bytes of mpint (buflen = %lu)",
      (unsigned long) datalen, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data = malloc(datalen);
  if (data == NULL) {
    pr_log_pri(PR_LOG_CRIT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  data[0] = 0;

  res = BN_bn2bin(mpint, data + 1);
  if (res < 0 ||
      res != (datalen - 1)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: BN_bn2bin() failed: expected %lu bytes, got %d",
      (unsigned long) (datalen - 1), res);
    pr_memscrub(data, datalen);
    free(data);

    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (data[1] & 0x80) {
    sftp_msg_write_data(buf, buflen, (char *) data, datalen, TRUE);

  } else {
    sftp_msg_write_data(buf, buflen, (char *) data + 1, datalen - 1, TRUE);
  }

  pr_memscrub(data, datalen);
  free(data);
}

void sftp_msg_write_string(char **buf, uint32_t *buflen, const char *str) {
  uint32_t len = 0;

  len = strlen(str);
  sftp_msg_write_data(buf, buflen, str, len, TRUE);
}
