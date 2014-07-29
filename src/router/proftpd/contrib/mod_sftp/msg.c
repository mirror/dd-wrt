/*
 * ProFTPD - mod_sftp message format
 * Copyright (c) 2008-2013 TJ Saunders
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
 * $Id: msg.c,v 1.16 2013/10/07 01:29:05 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "msg.h"
#include "crypto.h"
#include "disconnect.h"

#ifdef HAVE_EXECINFO_H
# include <execinfo.h>
#endif


#ifdef PR_USE_OPENSSL_ECC
/* Max GFp field length = 528 bits.  SEC1 uncompressed encoding uses 2
 * bitstring points.  SEC1 specifies a 1 byte point type header.
 */
# define MAX_ECPOINT_LEN		((528*2 / 8) + 1)
#endif /* PR_USE_OPENSSL_ECC */

/* The scratch buffer used by getbuf() is a constant 8KB.  If the caller
 * requests a larger size than that, the request is fulfilled using the
 * caller-provided pool.
 */
static unsigned char msg_buf[8 * 1024];

static void log_stacktrace(void) {
#if defined(HAVE_EXECINFO_H) && \
    defined(HAVE_BACKTRACE) && \
    defined(HAVE_BACKTRACE_SYMBOLS)
  void *trace[PR_TUNABLE_CALLER_DEPTH];
  char **strings;
  int tracesz;

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "-----BEGIN STACK TRACE-----");

  tracesz = backtrace(trace, PR_TUNABLE_CALLER_DEPTH);
  if (tracesz < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "backtrace(3) error: %s", strerror(errno));
  }

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

unsigned char *sftp_msg_getbuf(pool *p, size_t sz) {
  if (sz <= sizeof(msg_buf)) {
    return msg_buf;
  }

  return palloc(p, sz);
}

char sftp_msg_read_byte(pool *p, unsigned char **buf, uint32_t *buflen) {
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

int sftp_msg_read_bool(pool *p, unsigned char **buf, uint32_t *buflen) {
  char bool = 0;

  (void) p;

  bool = sftp_msg_read_byte(p, buf, buflen);
  if (bool == 0)
    return 0;

  return 1;
}

unsigned char *sftp_msg_read_data(pool *p, unsigned char **buf,
    uint32_t *buflen, size_t datalen) {
  unsigned char *data = NULL;

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

uint32_t sftp_msg_read_int(pool *p, unsigned char **buf, uint32_t *buflen) {
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

uint64_t sftp_msg_read_long(pool *p, unsigned char **buf, uint32_t *buflen) {
  uint64_t val = 0;
  unsigned char data[8];

  if (*buflen < sizeof(data)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read long (buflen = %lu)",
      (unsigned long) *buflen);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  memcpy(data, *buf, sizeof(data));
  (*buf) += sizeof(data);
  (*buflen) -= sizeof(data);

  val = (uint64_t) data[0] << 56;
  val |= (uint64_t) data[1] << 48;
  val |= (uint64_t) data[2] << 40;
  val |= (uint64_t) data[3] << 32;
  val |= (uint64_t) data[4] << 24;
  val |= (uint64_t) data[5] << 16;
  val |= (uint64_t) data[6] << 8;
  val |= (uint64_t) data[7];

  return val;
}

BIGNUM *sftp_msg_read_mpint(pool *p, unsigned char **buf, uint32_t *buflen) {
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

char *sftp_msg_read_string(pool *p, unsigned char **buf, uint32_t *buflen) {
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

#ifdef PR_USE_OPENSSL_ECC
EC_POINT *sftp_msg_read_ecpoint(pool *p, unsigned char **buf, uint32_t *buflen,
    const EC_GROUP *curve, EC_POINT *point) {
  BN_CTX *bn_ctx;
  unsigned char *data = NULL;
  uint32_t datalen = 0;

  bn_ctx = BN_CTX_new();
  if (bn_ctx == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating new BN_CTX: %s", sftp_crypto_get_errors());
    return NULL;
  }

  datalen = sftp_msg_read_int(p, buf, buflen);

  if (*buflen < datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read %lu bytes of EC point"
      " (buflen = %lu)", (unsigned long) datalen, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (datalen > MAX_ECPOINT_LEN) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: EC point length too long (%lu > max %lu)",
      (unsigned long) datalen, (unsigned long) MAX_ECPOINT_LEN);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data = sftp_msg_read_data(p, buf, buflen, datalen); 
  if (data == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to read %lu bytes of EC point data",
      (unsigned long) datalen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (data[0] != POINT_CONVERSION_UNCOMPRESSED) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: EC point data formatted incorrectly "
      "(leading byte 0x%02x should be 0x%02x)", data[0],
      POINT_CONVERSION_UNCOMPRESSED);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (EC_POINT_oct2point(curve, point, data, datalen, bn_ctx) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to convert binary EC point data: %s",
      sftp_crypto_get_errors());
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  BN_CTX_free(bn_ctx);

  pr_memscrub(data, datalen);
  return point;
}
#endif /* PR_USE_OPENSSL_ECC */

uint32_t sftp_msg_write_byte(unsigned char **buf, uint32_t *buflen, char byte) {
  uint32_t len = 0;

  if (*buflen < sizeof(char)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write byte (buflen = %lu)",
      (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  len = sizeof(char);

  memcpy(*buf, &byte, len);
  (*buf) += len;
  (*buflen) -= len;

  return len;
}

uint32_t sftp_msg_write_bool(unsigned char **buf, uint32_t *buflen, char bool) {
  return sftp_msg_write_byte(buf, buflen, bool == 0 ? 0 : 1);
}

uint32_t sftp_msg_write_data(unsigned char **buf, uint32_t *buflen,
   const unsigned char *data, size_t datalen, int write_len) {
  uint32_t len = 0;

  if (write_len) {
    len += sftp_msg_write_int(buf, buflen, datalen);
  }

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

    len += datalen;
  }

  return len;
}

uint32_t sftp_msg_write_int(unsigned char **buf, uint32_t *buflen,
    uint32_t val) {
  uint32_t len;

  if (*buflen < sizeof(uint32_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write int (buflen = %lu)",
      (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  len = sizeof(uint32_t);

  val = htonl(val);
  memcpy(*buf, &val, len);
  (*buf) += len;
  (*buflen) -= len;

  return len;
}

uint32_t sftp_msg_write_long(unsigned char **buf, uint32_t *buflen,
    uint64_t val) {
  unsigned char data[8];

  if (*buflen < sizeof(uint64_t)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write long (buflen = %lu)",
      (unsigned long) *buflen);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data[0] = (unsigned char) (val >> 56) & 0xFF;
  data[1] = (unsigned char) (val >> 48) & 0xFF;
  data[2] = (unsigned char) (val >> 40) & 0xFF;
  data[3] = (unsigned char) (val >> 32) & 0xFF;
  data[4] = (unsigned char) (val >> 24) & 0xFF;
  data[5] = (unsigned char) (val >> 16) & 0xFF;
  data[6] = (unsigned char) (val >> 8) & 0xFF;
  data[7] = (unsigned char) val & 0xFF;

  return sftp_msg_write_data(buf, buflen, data, sizeof(data), FALSE);
}

uint32_t sftp_msg_write_mpint(unsigned char **buf, uint32_t *buflen,
    const BIGNUM *mpint) {
  unsigned char *data = NULL;
  size_t datalen = 0;
  int res = 0;
  uint32_t len = 0;

  if (BN_is_zero(mpint)) {
    return sftp_msg_write_int(buf, buflen, 0);
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
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
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

    /* Needed to avoid compiler (and static code analysis) complaints. */
    return 0;
  }

  if (data[1] & 0x80) {
    len += sftp_msg_write_data(buf, buflen, data, datalen, TRUE);

  } else {
    len += sftp_msg_write_data(buf, buflen, data + 1, datalen - 1,
      TRUE);
  }

  pr_memscrub(data, datalen);
  free(data);

  return len;
}

uint32_t sftp_msg_write_string(unsigned char **buf, uint32_t *buflen,
    const char *str) {
  uint32_t len = 0;

  len = strlen(str);
  return sftp_msg_write_data(buf, buflen, (const unsigned char *) str, len,
    TRUE);
}

#ifdef PR_USE_OPENSSL_ECC
uint32_t sftp_msg_write_ecpoint(unsigned char **buf, uint32_t *buflen,
    const EC_GROUP *curve, const EC_POINT *point) {
  unsigned char *data = NULL;
  size_t datalen = 0;
  uint32_t len = 0;
  BN_CTX *bn_ctx;

  bn_ctx = BN_CTX_new();
  if (bn_ctx == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating new BN_CTX: %s", sftp_crypto_get_errors());
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  datalen = EC_POINT_point2oct(curve, point, POINT_CONVERSION_UNCOMPRESSED,
    NULL, 0, bn_ctx);
  if (datalen > MAX_ECPOINT_LEN) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: EC point length too long (%lu > max %lu)",
      (unsigned long) datalen, (unsigned long) MAX_ECPOINT_LEN);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  if (*buflen < datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "message format error: unable to write %lu bytes of EC point "
      "(buflen = %lu)", (unsigned long) datalen, (unsigned long) *buflen);
    log_stacktrace();
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  data = malloc(datalen);
  if (data == NULL) {
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  if (EC_POINT_point2oct(curve, point, POINT_CONVERSION_UNCOMPRESSED, data,
      datalen, bn_ctx) != datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error writing EC point data: Length mismatch");
    pr_memscrub(data, datalen);
    free(data);
    BN_CTX_free(bn_ctx);

    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);

    /* Needed to avoid compiler (and static code analysis) complaints. */
    return 0;
  }

  len = sftp_msg_write_data(buf, buflen, (const unsigned char *) data, datalen,
    TRUE);

  pr_memscrub(data, datalen);
  free(data);
  BN_CTX_free(bn_ctx);

  return len;
}
#endif /* PR_USE_OPENSSL_ECC */

