/*
 * ProFTPD - mod_sftp ciphers
 * Copyright (c) 2008-2024 TJ Saunders
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
 */

#include "mod_sftp.h"

#include "packet.h"
#include "msg.h"
#include "crypto.h"
#include "cipher.h"
#include "session.h"
#include "interop.h"
#include "poly1305.h"

struct sftp_cipher {
  pool *pool;
  const char *algo;
  unsigned int algo_type;
  const EVP_CIPHER *cipher;

  unsigned char *iv;
  uint32_t iv_len;

  unsigned char *key;
  uint32_t key_len;

  uint32_t auth_len;
  size_t discard_len;
};

#define SFTP_CIPHER_ALGO_NONE	1
#define SFTP_CIPHER_ALGO_GCM	2
#define SFTP_CIPHER_ALGO_CHACHA	3

/* We need to keep the old ciphers around, so that we can handle N
 * arbitrary packets to/from the client using the old keys, as during rekeying.
 * Thus we have two read cipher contexts, two write cipher contexts.
 * The cipher idx variable indicates which of the ciphers is currently in use.
 */

static struct sftp_cipher read_ciphers[2] = {
  { NULL, NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0 },
  { NULL, NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0 }
};
static EVP_CIPHER_CTX *read_ctxs[2];
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
static EVP_CIPHER_CTX *read_header_ctxs[2] = { NULL, NULL };
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */

static struct sftp_cipher write_ciphers[2] = {
  { NULL, NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0 },
  { NULL, NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0 }
};
static EVP_CIPHER_CTX *write_ctxs[2];
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
static EVP_CIPHER_CTX *write_header_ctxs[2] = { NULL, NULL };
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */

#define SFTP_CIPHER_DEFAULT_BLOCK_SZ		8

static size_t read_blockszs[2] = {
  SFTP_CIPHER_DEFAULT_BLOCK_SZ,
  SFTP_CIPHER_DEFAULT_BLOCK_SZ
};

static size_t write_blockszs[2] = {
  SFTP_CIPHER_DEFAULT_BLOCK_SZ,
  SFTP_CIPHER_DEFAULT_BLOCK_SZ
};

static unsigned int read_cipher_idx = 0;
static unsigned int write_cipher_idx = 0;

static const char *trace_channel = "ssh2";

static void clear_cipher(struct sftp_cipher *);

static unsigned int get_next_read_index(void) {
  if (read_cipher_idx == 1) {
    return 0;
  }

  return 1;
}

static unsigned int get_next_write_index(void) {
  if (write_cipher_idx == 1) {
    return 0;
  }

  return 1;
}

static void switch_read_cipher(void) {
  /* First, clear the context of the existing read cipher, if any. */
  if (read_ciphers[read_cipher_idx].key != NULL) {
    clear_cipher(&(read_ciphers[read_cipher_idx]));

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
    if (EVP_CIPHER_CTX_cleanup(read_ctxs[read_cipher_idx]) != 1) {
#else
    if (EVP_CIPHER_CTX_reset(read_ctxs[read_cipher_idx]) != 1) {
#endif /* OpenSSL-1.1.x and later */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error clearing cipher context: %s", sftp_crypto_get_errors());
    }

    read_blockszs[read_cipher_idx] = SFTP_CIPHER_DEFAULT_BLOCK_SZ;

    /* Now we can switch the index. */
    if (read_cipher_idx == 1) {
      read_cipher_idx = 0;
      return;
    }

    read_cipher_idx = 1;
  }
}

static void switch_write_cipher(void) {
  /* First, clear the context of the existing read cipher, if any. */
  if (write_ciphers[write_cipher_idx].key != NULL) {
    clear_cipher(&(write_ciphers[write_cipher_idx]));

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
    if (EVP_CIPHER_CTX_cleanup(write_ctxs[write_cipher_idx]) != 1) {
#else
    if (EVP_CIPHER_CTX_reset(write_ctxs[write_cipher_idx]) != 1) {
#endif /* OpenSSL-1.1.x and later */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error clearing cipher context: %s", sftp_crypto_get_errors());
    }

    write_blockszs[write_cipher_idx] = SFTP_CIPHER_DEFAULT_BLOCK_SZ;

    /* Now we can switch the index. */
    if (write_cipher_idx == 1) {
      write_cipher_idx = 0;
      return;
    }

    write_cipher_idx = 1;
  }
}

static void clear_cipher(struct sftp_cipher *cipher) {
  if (cipher->iv != NULL) {
    pr_memscrub(cipher->iv, cipher->iv_len);
    free(cipher->iv);
    cipher->iv = NULL;
    cipher->iv_len = 0;
  }

  if (cipher->key != NULL) {
    pr_memscrub(cipher->key, cipher->key_len);
    free(cipher->key);
    cipher->key = NULL;
    cipher->key_len = 0;
  }

  cipher->cipher = NULL;
  cipher->algo = NULL;
}

static unsigned int get_algo_type(const char *algo) {
  unsigned int algo_type = 0;
  const char *gcm_suffix = "-gcm@openssh.com";

  if (strcmp(algo, "none") == 0) {
    algo_type = SFTP_CIPHER_ALGO_NONE;

  } else if (pr_strnrstr(algo, strlen(algo), gcm_suffix,
      strlen(gcm_suffix), 0) == TRUE) {
    algo_type = SFTP_CIPHER_ALGO_GCM;

#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
  } else if (strcmp(algo, "chacha20-poly1305@openssh.com") == 0) {
    algo_type = SFTP_CIPHER_ALGO_CHACHA;
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
  }

  return algo_type;
}

static int set_cipher_iv(struct sftp_cipher *cipher, const EVP_MD *hash,
    const unsigned char *k, uint32_t klen, const char *h, uint32_t hlen,
    char *letter, const unsigned char *id, uint32_t id_len) {
  EVP_MD_CTX *ctx;
  unsigned char *iv = NULL;
  size_t cipher_iv_len = 0, iv_sz = 0;
  uint32_t iv_len = 0;

  if (cipher->algo_type == SFTP_CIPHER_ALGO_NONE) {
    cipher->iv = iv;
    cipher->iv_len = iv_len;

    return 0;
  }

   /* Some ciphers do not use IVs; handle this case. */
  cipher_iv_len = EVP_CIPHER_iv_length(cipher->cipher);
  if (cipher_iv_len != 0) {
    iv_sz = sftp_crypto_get_size(cipher_iv_len, EVP_MD_size(hash));

  } else {
    iv_sz = EVP_MD_size(hash);
  }

  if (iv_sz == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to determine IV length for cipher '%s'", cipher->algo);
     errno = EINVAL;
    return -1;
  }

  iv = malloc(iv_sz);
  if (iv == NULL) {
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  ctx = EVP_MD_CTX_create();
  if (EVP_DigestInit(ctx, hash) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to initialize MD context for '%s': %s", EVP_MD_name(hash),
      sftp_crypto_get_errors());
    free(iv);
    errno = EINVAL;
    return -1;
  }

  if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_CIPHER_USE_K)) {
    EVP_DigestUpdate(ctx, k, klen);
  }

  if (EVP_DigestUpdate(ctx, h, hlen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to update MD context for '%s': %s", EVP_MD_name(hash),
      sftp_crypto_get_errors());
    free(iv);
    errno = EINVAL;
    return -1;
  }

  EVP_DigestUpdate(ctx, letter, sizeof(char));
  EVP_DigestUpdate(ctx, (char *) id, id_len);

  if (EVP_DigestFinal(ctx, iv, &iv_len) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to finish MD context for '%s': %s", EVP_MD_name(hash),
      sftp_crypto_get_errors());
    free(iv);
    errno = EINVAL;
    return -1;
  }

  EVP_MD_CTX_destroy(ctx);

  /* If we need more, keep hashing, as per RFC, until we have enough
   * material.
   */
  while (iv_sz > iv_len) {
    uint32_t len = iv_len;

    pr_signals_handle();

    ctx = EVP_MD_CTX_create();
    EVP_DigestInit(ctx, hash);
    if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_CIPHER_USE_K)) {
      EVP_DigestUpdate(ctx, k, klen);
    }
    EVP_DigestUpdate(ctx, h, hlen);
    EVP_DigestUpdate(ctx, iv, len);
    EVP_DigestFinal(ctx, iv + len, &len);
    EVP_MD_CTX_destroy(ctx);

    iv_len += len;
  }

  cipher->iv = iv;
  cipher->iv_len = iv_len;

  return 0;
}

static int set_cipher_key(struct sftp_cipher *cipher, const EVP_MD *hash,
    const unsigned char *k, uint32_t klen, const char *h, uint32_t hlen,
    char letter, const unsigned char *id, uint32_t id_len) {
  EVP_MD_CTX *ctx;
  unsigned char *key = NULL;
  size_t key_sz = 0;
  uint32_t key_len = 0;

  if (cipher->algo_type == SFTP_CIPHER_ALGO_NONE) {
    cipher->key = key;
    cipher->key_len = key_len;

    return 0;
  }

  key_sz = sftp_crypto_get_size(cipher->key_len > 0 ?
      cipher->key_len : (size_t) EVP_CIPHER_key_length(cipher->cipher),
    EVP_MD_size(hash));
  if (key_sz == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to determine key length for cipher '%s'", cipher->algo);
    errno = EINVAL;
    return -1;
  }

  pr_trace_msg(trace_channel, 19, "setting key (%lu bytes) for cipher %s",
    (unsigned long) key_sz, cipher->algo);

  key = malloc(key_sz);
  if (key == NULL) {
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  ctx = EVP_MD_CTX_create();
  if (EVP_DigestInit(ctx, hash) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to initialize MD context for '%s': %s", EVP_MD_name(hash),
      sftp_crypto_get_errors());
    free(key);
    errno = EINVAL;
    return -1;
  }

  if (EVP_DigestUpdate(ctx, k, klen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to update MD context for '%s': %s", EVP_MD_name(hash),
      sftp_crypto_get_errors());
    free(key);
    errno = EINVAL;
    return -1;
  }

  EVP_DigestUpdate(ctx, h, hlen);
  EVP_DigestUpdate(ctx, &letter, sizeof(letter));
  EVP_DigestUpdate(ctx, (char *) id, id_len);

  if (EVP_DigestFinal(ctx, key, &key_len) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to finish MD context for '%s': %s", EVP_MD_name(hash),
      sftp_crypto_get_errors());
    free(key);
    errno = EINVAL;
    return -1;
  }

  EVP_MD_CTX_destroy(ctx);

  pr_trace_msg(trace_channel, 19,
    "hashed data to produce key (%lu of %lu bytes)", (unsigned long) key_len,
    (unsigned long) key_sz);

  /* If we need more, keep hashing, as per RFC, until we have enough
   * material.
   */
  while (key_sz > key_len) {
    uint32_t len = key_len;

    pr_signals_handle();

    ctx = EVP_MD_CTX_create();
    EVP_DigestInit(ctx, hash);
    EVP_DigestUpdate(ctx, k, klen);
    EVP_DigestUpdate(ctx, h, hlen);
    EVP_DigestUpdate(ctx, key, len);
    EVP_DigestFinal(ctx, key + len, &len);
    EVP_MD_CTX_destroy(ctx);

    key_len += len;
  }

  cipher->key = key;

  return 0;
}

/* If the chosen cipher requires that we discard some of the initial bytes of
 * the cipher stream, then do so.  (This is mostly for any RC4 ciphers.)
 */
static int set_cipher_discarded(struct sftp_cipher *cipher,
    EVP_CIPHER_CTX *pctx) {
  unsigned char *garbage_in, *garbage_out;

  if (cipher->discard_len == 0) {
    return 0;
  }

  garbage_in = malloc(cipher->discard_len);
  if (garbage_in == NULL) {
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  garbage_out = malloc(cipher->discard_len);
  if (garbage_out == NULL) {
    free(garbage_in);
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  if (EVP_Cipher(pctx, garbage_out, garbage_in,
      cipher->discard_len) != 1) {
    free(garbage_in);
    pr_memscrub(garbage_out, cipher->discard_len);
    free(garbage_out);
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error ciphering discard data: %s", sftp_crypto_get_errors());

    return -1;
  }

  pr_trace_msg(trace_channel, 19, "discarded %lu bytes of cipher data",
    (unsigned long) cipher->discard_len);
  free(garbage_in);
  pr_memscrub(garbage_out, cipher->discard_len);
  free(garbage_out);

  return 0;
}

#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
/* Note that the given poly_key buffer MUST be POLY1305_KEYLEN in size. */
static int compute_chachapoly_key(struct ssh2_packet *pkt,
    EVP_CIPHER_CTX *pctx, unsigned char *poly_key) {
  unsigned char seqnobuf[16], *ptr;
  uint32_t len;

  /* Initialize our IV for the ChaCha cipher. */
  memset(seqnobuf, 0, sizeof(seqnobuf));
  ptr = seqnobuf + 8;
  len = 8;
  sftp_msg_write_long(&ptr, &len, pkt->seqno);

  if (EVP_CipherInit(pctx, NULL, NULL, seqnobuf, 1) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing ChaChaPoly cipher for encryption: %s",
      sftp_crypto_get_errors());
    return -1;
  }

  memset(poly_key, 0, POLY1305_KEYLEN);
  if (EVP_Cipher(pctx, poly_key, poly_key, POLY1305_KEYLEN) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error computing ChaChaPoly packet key: %s", sftp_crypto_get_errors());
    return -1;
  }

  return 0;
}
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */

#if !defined(HAVE_TIMINGSAFE_BCMP) && \
    defined(HAVE_EVP_CHACHA20_OPENSSL) && \
    !defined(HAVE_BROKEN_CHACHA20)
static int timingsafe_bcmp(const void *b1, const void *b2, size_t n) {
  const unsigned char *p1 = b1, *p2 = b2;
  int ret = 0;

  for (; n > 0; n--) {
    ret |= *p1++ ^ *p2++;
  }

  return (ret != 0);
}
#endif /* HAVE_TIMINGSAFE_BCMP and HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */

/* These accessors to get the authenticated data length for the read, write
 * ciphers are used during packet IO, and thus do not return the AAD lengths
 * until those ciphers are keyed.
 *
 * However, during KEX, there are times when we want to know the ADD lengths
 * after the algorithms are selected, but before they are keyed.  Thus for
 * those cases, we have the accessor variants.
 */

size_t sftp_cipher_get_read_auth_size2(void) {
  return read_ciphers[read_cipher_idx].auth_len;
}

size_t sftp_cipher_get_read_auth_size(void) {
  /* Do not indicate the read cipher authentication tag size until the
   * cipher has been keyed.
   */
  if (read_ciphers[read_cipher_idx].key != NULL) {
    return sftp_cipher_get_read_auth_size2();
  }

  return 0;
}

size_t sftp_cipher_get_write_auth_size2(void) {
  return write_ciphers[write_cipher_idx].auth_len;
}

size_t sftp_cipher_get_write_auth_size(void) {
  /* Do not indicate the write cipher authentication tag size until the
   * cipher has been keyed.
   */
  if (write_ciphers[write_cipher_idx].key != NULL) {
    return sftp_cipher_get_write_auth_size2();
  }

  return 0;
}

size_t sftp_cipher_get_read_block_size(void) {
  return read_blockszs[read_cipher_idx];
}

size_t sftp_cipher_get_write_block_size(void) {
  return write_blockszs[write_cipher_idx];
}

void sftp_cipher_set_read_block_size(size_t blocksz) {
  if (blocksz > read_blockszs[read_cipher_idx]) {
    read_blockszs[read_cipher_idx] = blocksz;
  }
}

void sftp_cipher_set_write_block_size(size_t blocksz) {
  if (blocksz > write_blockszs[write_cipher_idx]) {
    write_blockszs[write_cipher_idx] = blocksz;
  }
}

int sftp_cipher_is_read_chachapoly(void) {
  if (read_ciphers[read_cipher_idx].key != NULL &&
      read_ciphers[read_cipher_idx].algo_type == SFTP_CIPHER_ALGO_CHACHA) {
    return TRUE;
  }

  return FALSE;
}

const char *sftp_cipher_get_read_algo(void) {
  if (read_ciphers[read_cipher_idx].key != NULL ||
      read_ciphers[read_cipher_idx].algo_type == SFTP_CIPHER_ALGO_NONE) {
    return read_ciphers[read_cipher_idx].algo;
  }

  return NULL;
}

int sftp_cipher_set_read_algo(const char *algo) {
  unsigned int idx = read_cipher_idx;
  size_t key_len = 0, auth_len = 0, discard_len = 0;

  if (read_ciphers[idx].key != NULL) {
    /* If we have an existing key, it means that we are currently rekeying. */
    idx = get_next_read_index();
  }

  read_ciphers[idx].cipher = sftp_crypto_get_cipher(algo, &key_len, &auth_len,
    &discard_len);
  if (read_ciphers[idx].cipher == NULL) {
    return -1;
  }

  if (key_len > 0) {
    pr_trace_msg(trace_channel, 19,
      "setting read key for cipher %s: key len = %lu", algo,
      (unsigned long) key_len);
  }

  if (auth_len > 0) {
    pr_trace_msg(trace_channel, 19,
      "setting read key for cipher %s: auth len = %lu", algo,
      (unsigned long) auth_len);
  }

  if (discard_len > 0) {
    pr_trace_msg(trace_channel, 19,
      "setting read key for cipher %s: discard len = %lu", algo,
      (unsigned long) discard_len);
  }

  /* Note that we use a new pool, each time the algorithm is set (which
   * happens during key exchange) to prevent undue memory growth for
   * long-lived sessions with many rekeys.
   */
  if (read_ciphers[idx].pool != NULL) {
    destroy_pool(read_ciphers[idx].pool);
  }

  read_ciphers[idx].pool = make_sub_pool(sftp_pool);
  pr_pool_tag(read_ciphers[idx].pool, "SFTP cipher read pool");
  read_ciphers[idx].algo = pstrdup(read_ciphers[idx].pool, algo);
  read_ciphers[idx].algo_type = get_algo_type(algo);

  read_ciphers[idx].key_len = (uint32_t) key_len;
  read_ciphers[idx].auth_len = (uint32_t) auth_len;
  read_ciphers[idx].discard_len = discard_len;

  return 0;
}

int sftp_cipher_set_read_key(pool *p, const EVP_MD *hash,
    const unsigned char *k, uint32_t klen, const char *h, uint32_t hlen,
    int role) {
  const unsigned char *id = NULL;
  char letter;
  uint32_t id_len;
  int key_len, auth_len;
  struct sftp_cipher *cipher;
  EVP_CIPHER_CTX *pctx, *hpctx = NULL;

  switch_read_cipher();

  cipher = &(read_ciphers[read_cipher_idx]);
  pctx = read_ctxs[read_cipher_idx];
  if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
    hpctx = read_header_ctxs[read_cipher_idx];
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
  }

  id_len = sftp_session_get_id(&id);

  /* The letters used depend on the role; see:
   *  https://tools.ietf.org/html/rfc4253#section-7.2
   *
   * If we are the SERVER, then we use the letters for the "client to server"
   * flows, since we are READING from the client.
   */

  /* client-to-server IV: HASH(K || H || "A" || session_id)
   * server-to-client IV: HASH(K || H || "B" || session_id)
   */
  letter = (role == SFTP_ROLE_SERVER ? 'A' : 'B');
  if (set_cipher_iv(cipher, hash, k, klen, h, hlen, &letter, id, id_len) < 0) {
    return -1;
  }

  /* client-to-server key: HASH(K || H || "C" || session_id)
   * server-to-client key: HASH(K || H || "D" || session_id)
   */
  letter = (role == SFTP_ROLE_SERVER ? 'C' : 'D');
  if (set_cipher_key(cipher, hash, k, klen, h, hlen, letter, id, id_len) < 0) {
    return -1;
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_CIPHER_CTX_init(pctx);
  if (hpctx != NULL) {
    EVP_CIPHER_CTX_init(hpctx);
  }
#else
  EVP_CIPHER_CTX_reset(pctx);
  if (hpctx != NULL) {
    EVP_CIPHER_CTX_reset(hpctx);
  }
#endif /* prior to OpenSSL-1.1.0 */

#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
  if (EVP_CipherInit_ex(pctx, cipher->cipher, NULL, NULL,
    cipher->iv, 0) != 1) {
#else
  if (EVP_CipherInit(pctx, cipher->cipher, NULL, cipher->iv, 0) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing %s cipher for decryption: %s", cipher->algo,
      sftp_crypto_get_errors());
    return -1;
  }

  if (hpctx != NULL) {
#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
    if (EVP_CipherInit_ex(hpctx, cipher->cipher, NULL, NULL, NULL, 0) != 1) {
#else
    if (EVP_CipherInit(hpctx, cipher->cipher, NULL, NULL, 0) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error initializing %s cipher for header decryption: %s", cipher->algo,
        sftp_crypto_get_errors());
      return -1;
    }
  }

  auth_len = (int) cipher->auth_len;
  if (auth_len > 0) {
    if (cipher->algo_type == SFTP_CIPHER_ALGO_GCM) {
#if defined(EVP_CTRL_GCM_SET_IV_FIXED)
      if (EVP_CIPHER_CTX_ctrl(pctx, EVP_CTRL_GCM_SET_IV_FIXED, -1,
          cipher->iv) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error configuring %s cipher for decryption: %s", cipher->algo,
          sftp_crypto_get_errors());
        return -1;
      }
#endif /* EVP_CTRL_GCM_SET_IV_FIXED */
      pr_trace_msg(trace_channel, 19,
        "set auth length (%d) for %s cipher for decryption", auth_len,
        cipher->algo);
    }
  }

  /* Next, set the key length. */
  key_len = (int) cipher->key_len;
  if (key_len > 0 &&
      cipher->algo_type != SFTP_CIPHER_ALGO_CHACHA) {

    /* Skip setting our custom key length for ChaCha20, since the custom
     * key length is used for two different ChaCha20 cipher instances.
     */
    if (EVP_CIPHER_CTX_set_key_length(pctx, key_len) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting key length (%d bytes) for %s cipher for decryption: %s",
        key_len, cipher->algo, sftp_crypto_get_errors());
      return -1;
    }

    pr_trace_msg(trace_channel, 19,
      "set key length (%d) for %s cipher for decryption", key_len,
      cipher->algo);
  }

#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
  if (EVP_CipherInit_ex(pctx, NULL, NULL, cipher->key, NULL, -1) != 1) {
#else
  if (EVP_CipherInit(pctx, NULL, cipher->key, NULL, -1) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error re-initializing %s cipher for decryption: %s", cipher->algo,
      sftp_crypto_get_errors());
    return -1;
  }

  if (hpctx != NULL) {
    /* The ChaChaPoly header instance uses the "second half" of the computed
     * session key, per OpenSSH spec.
     */
#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
    if (EVP_CipherInit_ex(hpctx, NULL, NULL, cipher->key + 32, NULL, -1) != 1) {
#else
    if (EVP_CipherInit(hpctx, NULL, cipher->key + 32, NULL, -1) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error re-initializing %s cipher for header decryption: %s",
        cipher->algo, sftp_crypto_get_errors());
      return -1;
    }
  }

  if (set_cipher_discarded(cipher, pctx) < 0) {
    return -1;
  }

  if (strcmp(cipher->algo, "aes128-ctr") == 0 ||
      strcmp(cipher->algo, "aes128-gcm@openssh.com") == 0 ||
      strcmp(cipher->algo, "aes192-ctr") == 0 ||
      strcmp(cipher->algo, "aes256-ctr") == 0 ||
      strcmp(cipher->algo, "aes256-gcm@openssh.com") == 0) {
    /* For some reason, OpenSSL returns 8 for the AES CTR/GCM block size (even
     * though the AES block size is 16, per RFC 5647), but OpenSSH wants 16.
     */
    sftp_cipher_set_read_block_size(16);

  } else {
    sftp_cipher_set_read_block_size(EVP_CIPHER_block_size(cipher->cipher));
  }

  pr_trace_msg(trace_channel, 19,
    "set block size (%d) for %s cipher for decryption",
    (int) sftp_cipher_get_read_block_size(), cipher->algo);

  return 0;
}

int sftp_cipher_read_data(struct ssh2_packet *pkt, unsigned char *data,
    uint32_t data_len, unsigned char **buf, uint32_t *buflen) {
  int res;
  struct sftp_cipher *cipher;
  EVP_CIPHER_CTX *pctx;
  size_t auth_len = 0, read_blocksz;
  uint32_t output_buflen;
  unsigned char *ptr = NULL, *buf2 = NULL;
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
  unsigned char chachapoly_key[POLY1305_KEYLEN];
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */

  cipher = &(read_ciphers[read_cipher_idx]);

  if (cipher->key == NULL) {
    /* We haven't finished NEWKEYS yet, so our cipher isn't keyed. */

    *buf = data;
    *buflen = data_len;
    return 0;
  }

  pctx = read_ctxs[read_cipher_idx];
  read_blocksz = read_blockszs[read_cipher_idx];
  auth_len = sftp_cipher_get_read_auth_size();
  output_buflen = *buflen;

  if (*buf == NULL) {
    size_t bufsz;

    /* Allocate a buffer that's large enough. */
    bufsz = (data_len + read_blocksz - 1);
    ptr = buf2 = palloc(pkt->pool, bufsz);

  } else {
    ptr = buf2 = *buf;
  }

  if (pkt->packet_len == 0) {
    if (auth_len > 0) {
      if (cipher->algo_type == SFTP_CIPHER_ALGO_GCM) {
#if defined(EVP_CTRL_GCM_IV_GEN)
        unsigned char prev_iv[1];

        /* Increment the IV. */
        if (EVP_CIPHER_CTX_ctrl(pctx, EVP_CTRL_GCM_IV_GEN, 1, prev_iv) != 1) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error incrementing %s IV data for client: %s", cipher->algo,
            sftp_crypto_get_errors());
          errno = EIO;
          return -1;
        }
#endif
      }
    }

    if (pkt->aad_len > 0 &&
        pkt->aad == NULL) {
      pkt->aad = palloc(pkt->pool, pkt->aad_len);
      memcpy(pkt->aad, data, pkt->aad_len);
      memcpy(ptr, data, pkt->aad_len);

      /* Save room at the start of the output buffer `ptr` for the AAD
       * bytes.
       */
      buf2 += pkt->aad_len;
      data += pkt->aad_len;
      data_len -= pkt->aad_len;
      output_buflen -= pkt->aad_len;

      if (auth_len > 0) {
        if (cipher->algo_type != SFTP_CIPHER_ALGO_CHACHA) {
          if (EVP_Cipher(pctx, NULL, pkt->aad, pkt->aad_len) < 0) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "error setting %s AAD data for client: %s", cipher->algo,
              sftp_crypto_get_errors());
            errno = EIO;
            return -1;
          }
        }
      }
    }
  }

  if (output_buflen % read_blocksz != 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "bad input length for decryption (%u bytes, %lu AAD bytes, "
      "%u block size)", output_buflen, (unsigned long) pkt->aad_len,
      (unsigned int) read_blocksz);
    return -1;
  }

  if (pkt->packet_len > 0 &&
      auth_len > 0) {
    unsigned char *tag_data = NULL;
    uint32_t tag_datalen = auth_len;

    /* The authentication tag appears after the unencrypted AAD bytes, and
     * the encrypted payload bytes.
     */
    tag_data = data + (data_len - auth_len);

    if (cipher->algo_type == SFTP_CIPHER_ALGO_GCM) {
#if defined(EVP_CTRL_GCM_GET_TAG)
      if (EVP_CIPHER_CTX_ctrl(pctx, EVP_CTRL_GCM_SET_TAG, tag_datalen,
          tag_data) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error setting %s authentication tag for client: %s", cipher->algo,
          sftp_crypto_get_errors());
        errno = EIO;
        return -1;
      }
#endif
      data_len -= auth_len;

    } else if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
      unsigned char chachapoly_tag[POLY1305_TAGLEN];
      pool *tag_pool;
      unsigned char *tag_buf;
      size_t tag_bufsz;

      if (compute_chachapoly_key(pkt, pctx, chachapoly_key) < 0) {
        return -1;
      }

      /* Here we want to compute our Poly1305 tag over the combination
       * of the encrypted packet length (pkt->aad) and the encrypted
       * payload (buf).
       *
       * Thus we need to assemble that here.
       */

      tag_pool = make_sub_pool(pkt->pool);
      tag_bufsz = pkt->aad_len + data_len;
      tag_buf = palloc(tag_pool, tag_bufsz);

      memcpy(tag_buf, pkt->aad, pkt->aad_len);
      memcpy(tag_buf + pkt->aad_len, data, data_len);

      poly1305_auth(chachapoly_tag, tag_buf, tag_bufsz, chachapoly_key);
      destroy_pool(tag_pool);

      /* Our ChaChaPoly tag is stored as the MAC, NOT in the given network
       * data (which is just the payload).
       */
      if (timingsafe_bcmp(chachapoly_tag, pkt->mac,
          sizeof(chachapoly_tag)) != 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error verifying %s authentication tag from client: "
          "Mismatched tags", cipher->algo);
        errno = EIO;
        return -1;
      }
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
    }
  }

  if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
    unsigned char seqnobuf[16], *ptr;
    uint32_t len;

    memset(seqnobuf, 0, sizeof(seqnobuf));
    seqnobuf[0] = 1;

    ptr = seqnobuf + 8;
    len = 8;
    sftp_msg_write_long(&ptr, &len, pkt->seqno);

    if (EVP_CipherInit(pctx, NULL, NULL, seqnobuf, 1) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error initializing %s cipher for encryption: %s", cipher->algo,
        sftp_crypto_get_errors());
      return -1;
    }
  }

  res = EVP_Cipher(pctx, buf2, data, data_len);
  if (res < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error decrypting %s data from client: %s", cipher->algo,
      sftp_crypto_get_errors());
    return -1;
  }

  if (pkt->packet_len > 0) {
    *buflen = data_len;

  } else {
    /* If we don't know the packet length yet, it means we need to allow for
     * the processing of the AAD bytes.
     */
    *buflen = pkt->aad_len + data_len;
  }

  *buf = ptr;

  if (pkt->packet_len > 0 &&
      auth_len > 0) {
    /* Verify the authentication tag, but only if we have the full packet. */
    if (cipher->algo_type != SFTP_CIPHER_ALGO_CHACHA) {
      if (EVP_Cipher(pctx, NULL, NULL, 0) < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error verifying %s authentication tag for client: %s",
          cipher->algo, sftp_crypto_get_errors());
        errno = EIO;
        return -1;
      }
    }
  }

  return 0;
}

int sftp_cipher_read_packet_len(struct ssh2_packet *pkt, unsigned char *data,
    uint32_t data_len, unsigned char **buf, uint32_t *buflen,
    uint32_t *packet_len) {
  int res;
  struct sftp_cipher *cipher;
  uint32_t pkt_len = 0;

  cipher = &(read_ciphers[read_cipher_idx]);

  if (cipher->key == NULL) {
    /* We haven't finished NEWKEYS setup yet, so packet length is in
     * plaintext.
     */

    *buf = data;
    *buflen = data_len;

    memmove(&pkt_len, *buf, sizeof(uint32_t));
    *packet_len = ntohl(pkt_len);

    *buf += sizeof(uint32_t);
    *buflen -= sizeof(uint32_t);

    return 0;
  }

  if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
    unsigned char seqnobuf[16], *ptr;
    uint32_t len;
    EVP_CIPHER_CTX *hpctx;

    hpctx = read_header_ctxs[read_cipher_idx];

    /* Initialize our IV for the ChaChaPoly header. Note that the packet
     * sequence number must be encoded according to the SSH spec.
     */
    memset(seqnobuf, 0, sizeof(seqnobuf));
    ptr = seqnobuf + 8;
    len = 8;
    sftp_msg_write_long(&ptr, &len, pkt->seqno);

    if (EVP_CipherInit(hpctx, NULL, NULL, seqnobuf, 0) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error initializing %s cipher for packet length decryption: %s",
        cipher->algo, sftp_crypto_get_errors());
      return -1;
    }

    if (EVP_Cipher(hpctx, (unsigned char *) &pkt_len, data,
        sizeof(pkt_len)) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error decrypting %s packet length from client: %s", cipher->algo,
        sftp_crypto_get_errors());
      return -1;
    }

    /* We need to save these encrypted header bytes for later. */
    pkt->aad = palloc(pkt->pool, pkt->aad_len);
    memcpy(pkt->aad, data, pkt->aad_len);

    *packet_len = ntohl(pkt_len);

    /* No leftover network bytes for later processing; we used them all. */
    *buf = NULL;
    *buflen = 0;

    return 0;
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
  }

  res = sftp_cipher_read_data(pkt, data, data_len, buf, buflen);
  if (res < 0) {
    return -1;
  }

  memmove(&pkt_len, *buf, sizeof(uint32_t));
  *packet_len = ntohl(pkt_len);

  *buf += sizeof(uint32_t);
  *buflen -= sizeof(uint32_t);
  return 0;
}

const char *sftp_cipher_get_write_algo(void) {
  if (write_ciphers[write_cipher_idx].key != NULL ||
      write_ciphers[write_cipher_idx].algo_type == SFTP_CIPHER_ALGO_NONE) {
    return write_ciphers[write_cipher_idx].algo;
  }

  return NULL;
}

int sftp_cipher_set_write_algo(const char *algo) {
  unsigned int idx = write_cipher_idx;
  size_t key_len = 0, auth_len = 0, discard_len = 0;

  if (write_ciphers[idx].key != NULL) {
    /* If we have an existing key, it means that we are currently rekeying. */
    idx = get_next_write_index();
  }

  write_ciphers[idx].cipher = sftp_crypto_get_cipher(algo, &key_len, &auth_len,
    &discard_len);
  if (write_ciphers[idx].cipher == NULL) {
    return -1;
  }

  if (key_len > 0) {
    pr_trace_msg(trace_channel, 19,
      "setting write key for cipher %s: key len = %lu", algo,
      (unsigned long) key_len);
  }

  if (auth_len > 0) {
    pr_trace_msg(trace_channel, 19,
      "setting write key for cipher %s: auth len = %lu", algo,
      (unsigned long) auth_len);
  }

  if (discard_len > 0) {
    pr_trace_msg(trace_channel, 19,
      "setting write key for cipher %s: discard len = %lu", algo,
      (unsigned long) discard_len);
  }

  /* Note that we use a new pool, each time the algorithm is set (which
   * happens during key exchange) to prevent undue memory growth for
   * long-lived sessions with many rekeys.
   */
  if (write_ciphers[idx].pool != NULL) {
    destroy_pool(write_ciphers[idx].pool);
  }

  write_ciphers[idx].pool = make_sub_pool(sftp_pool);
  pr_pool_tag(write_ciphers[idx].pool, "SFTP cipher write pool");
  write_ciphers[idx].algo = pstrdup(write_ciphers[idx].pool, algo);
  write_ciphers[idx].algo_type = get_algo_type(algo);

  write_ciphers[idx].key_len = (uint32_t) key_len;
  write_ciphers[idx].auth_len = (uint32_t) auth_len;
  write_ciphers[idx].discard_len = discard_len;

  return 0;
}

int sftp_cipher_set_write_key(pool *p, const EVP_MD *hash,
    const unsigned char *k, uint32_t klen, const char *h, uint32_t hlen,
    int role) {
  const unsigned char *id = NULL;
  char letter;
  uint32_t id_len;
  int key_len, auth_len;
  struct sftp_cipher *cipher;
  EVP_CIPHER_CTX *pctx, *hpctx = NULL;

  switch_write_cipher();

  cipher = &(write_ciphers[write_cipher_idx]);
  pctx = write_ctxs[write_cipher_idx];
  if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
    hpctx = write_header_ctxs[write_cipher_idx];
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
  }

  id_len = sftp_session_get_id(&id);

  /* The letters used depend on the role; see:
   *  https://tools.ietf.org/html/rfc4253#section-7.2
   *
   * If we are the SERVER, then we use the letters for the "server to client"
   * flows, since we are WRITING to the client.
   */

  /* client-to-server IV: HASH(K || H || "A" || session_id)
   * server-to-client IV: HASH(K || H || "B" || session_id)
   */
  letter = (role == SFTP_ROLE_SERVER ? 'B' : 'A');
  if (set_cipher_iv(cipher, hash, k, klen, h, hlen, &letter, id, id_len) < 0) {
    return -1;
  }

  /* client-to-server key: HASH(K || H || "C" || session_id)
   * server-to-client key: HASH(K || H || "D" || session_id)
   */
  letter = (role == SFTP_ROLE_SERVER ? 'D' : 'C');
  if (set_cipher_key(cipher, hash, k, klen, h, hlen, letter, id, id_len) < 0) {
    return -1;
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_CIPHER_CTX_init(pctx);
  if (hpctx != NULL) {
    EVP_CIPHER_CTX_init(hpctx);
  }
#else
  EVP_CIPHER_CTX_reset(pctx);
  if (hpctx != NULL) {
    EVP_CIPHER_CTX_reset(hpctx);
  }
#endif

#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
  if (EVP_CipherInit_ex(pctx, cipher->cipher, NULL, NULL,
    cipher->iv, 1) != 1) {
#else
  if (EVP_CipherInit(pctx, cipher->cipher, NULL, cipher->iv, 1) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing %s cipher for encryption: %s", cipher->algo,
      sftp_crypto_get_errors());
    return -1;
  }

  if (hpctx != NULL) {
#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
    if (EVP_CipherInit_ex(hpctx, cipher->cipher, NULL, NULL, NULL, 1) != 1) {
#else
    if (EVP_CipherInit(hpctx, cipher->cipher, NULL, NULL, 1) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error initializing %s cipher for header encryption: %s", cipher->algo,
        sftp_crypto_get_errors());
      return -1;
    }
  }

  auth_len = (int) cipher->auth_len;
  if (auth_len > 0) {
    if (cipher->algo_type == SFTP_CIPHER_ALGO_GCM) {
#if defined(EVP_CTRL_GCM_SET_IV_FIXED)
      if (EVP_CIPHER_CTX_ctrl(pctx, EVP_CTRL_GCM_SET_IV_FIXED, -1,
          cipher->iv) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error configuring %s cipher for encryption: %s", cipher->algo,
          sftp_crypto_get_errors());
        return -1;
      }
#endif /* EVP_CTRL_GCM_SET_IV_FIXED */

      pr_trace_msg(trace_channel, 19,
        "set auth length (%d) for %s cipher for encryption", auth_len,
        cipher->algo);
    }
  }

  /* Next, set the key length. */
  key_len = (int) cipher->key_len;
  if (key_len > 0 &&
      cipher->algo_type != SFTP_CIPHER_ALGO_CHACHA) {

    /* Skip setting our custom key length for ChaCha20, since the custom
     * key length is used for two different ChaCha20 cipher instances.
     */

    if (EVP_CIPHER_CTX_set_key_length(pctx, key_len) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting key length (%d bytes) for %s cipher for decryption: %s",
        key_len, cipher->algo, sftp_crypto_get_errors());
      return -1;
    }

    pr_trace_msg(trace_channel, 19,
      "set key length (%d) for %s cipher for encryption", key_len,
      cipher->algo);
  }

#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
  if (EVP_CipherInit_ex(pctx, NULL, NULL, cipher->key, NULL, -1) != 1) {
#else
  if (EVP_CipherInit(pctx, NULL, cipher->key, NULL, -1) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error re-initializing %s cipher for encryption: %s", cipher->algo,
      sftp_crypto_get_errors());
    return -1;
  }

  if (hpctx != NULL) {
#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
    if (EVP_CipherInit_ex(hpctx, NULL, NULL, cipher->key + 32, NULL, -1) != 1) {
#else
    if (EVP_CipherInit(hpctx, NULL, cipher->key + 32, NULL, -1) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error re-initializing %s cipher for header encryption: %s",
        cipher->algo, sftp_crypto_get_errors());
      return -1;
    }
  }

  if (set_cipher_discarded(cipher, pctx) < 0) {
    return -1;
  }

  if (strcmp(cipher->algo, "aes128-ctr") == 0 ||
      strcmp(cipher->algo, "aes128-gcm@openssh.com") == 0 ||
      strcmp(cipher->algo, "aes192-ctr") == 0 ||
      strcmp(cipher->algo, "aes256-ctr") == 0 ||
      strcmp(cipher->algo, "aes256-gcm@openssh.com") == 0) {
    /* For some reason, OpenSSL returns 8 for the AES CTR/GCM block size (even
     * though the AES block size is 16, per RFC 5647), but OpenSSH wants 16.
     */
    sftp_cipher_set_write_block_size(16);

  } else {
    sftp_cipher_set_write_block_size(EVP_CIPHER_block_size(cipher->cipher));
  }

  pr_trace_msg(trace_channel, 19,
    "set block size (%d) for %s cipher for encryption",
    (int) sftp_cipher_get_write_block_size(), cipher->algo);

  return 0;
}

int sftp_cipher_write_data(struct ssh2_packet *pkt, unsigned char *buf,
    size_t *buflen) {
  int res;
  struct sftp_cipher *cipher;
  EVP_CIPHER_CTX *pctx;
  size_t auth_len = 0;
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
  unsigned char chachapoly_key[POLY1305_KEYLEN];
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
  unsigned char *data, *ptr;
  uint32_t datalen, datasz;

  cipher = &(write_ciphers[write_cipher_idx]);
  pctx = write_ctxs[write_cipher_idx];
  auth_len = sftp_cipher_get_write_auth_size();

  if (cipher->key == NULL) {
    *buflen = 0;
    return 0;
  }

  /* Always leave a little extra room in the buffer. */
  datasz = sizeof(uint32_t) + pkt->packet_len + 64;

  if (pkt->aad_len > 0) {
    /* Packet length is not encrypted for authentication encryption, or
     * Encrypt-Then-MAC modes.  However, it IS encrypted for ChaChaPoly.
     */
    if (cipher->algo_type != SFTP_CIPHER_ALGO_CHACHA) {
      datasz -= pkt->aad_len;
    }

    /* And, for ETM modes, we may need a little more space. */
    datasz += sftp_cipher_get_write_block_size();
  }

  datalen = datasz;
  ptr = data = palloc(pkt->pool, datasz);

  if (auth_len > 0) {
    if (cipher->algo_type == SFTP_CIPHER_ALGO_GCM) {
#if defined(EVP_CTRL_GCM_IV_GEN)
      unsigned char prev_iv[1];

      /* Increment the IV. */
      if (EVP_CIPHER_CTX_ctrl(pctx, EVP_CTRL_GCM_IV_GEN, 1, prev_iv) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error incrementing %s IV data for client: %s", cipher->algo,
          sftp_crypto_get_errors());
        errno = EIO;
        return -1;
      }
#endif
    } else if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
      if (compute_chachapoly_key(pkt, pctx, chachapoly_key) < 0) {
        return -1;
      }
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
    }
  }

  if (pkt->aad_len > 0 &&
      pkt->aad == NULL) {
    uint32_t packet_len;

    packet_len = htonl(pkt->packet_len);
    pkt->aad = palloc(pkt->pool, pkt->aad_len);
    memcpy(pkt->aad, &packet_len, pkt->aad_len);

    if (auth_len > 0) {
      if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
        unsigned char seqnobuf[16], *ptr;
        uint32_t len;
        EVP_CIPHER_CTX *hpctx;

        hpctx = write_header_ctxs[write_cipher_idx];

        memset(seqnobuf, 0, sizeof(seqnobuf));
        ptr = seqnobuf + 8;
        len = 8;
        sftp_msg_write_long(&ptr, &len, pkt->seqno);

        if (EVP_CipherInit(hpctx, NULL, NULL, seqnobuf, 1) != 1) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error initializing %s cipher for packet length encryption: %s",
            cipher->algo, sftp_crypto_get_errors());
          return -1;
        }

        if (EVP_Cipher(hpctx, pkt->aad, (unsigned char *) &packet_len,
            pkt->aad_len) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error encrypting %s packet length for client: %s", cipher->algo,
            sftp_crypto_get_errors());
          return -1;
        }
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */

      } else {
        if (EVP_Cipher(pctx, NULL, pkt->aad, pkt->aad_len) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error setting %s AAD (%lu bytes) for client: %s", cipher->algo,
            (unsigned long) pkt->aad_len, sftp_crypto_get_errors());
          errno = EIO;
          return -1;
        }
      }
    }

  } else {
    sftp_msg_write_int(&data, &datalen, pkt->packet_len);
  }

  sftp_msg_write_byte(&data, &datalen, pkt->padding_len);
  sftp_msg_write_data(&data, &datalen, pkt->payload, pkt->payload_len, FALSE);
  sftp_msg_write_data(&data, &datalen, pkt->padding, pkt->padding_len, FALSE);

  if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
    unsigned char seqnobuf[16], *ptr;
    uint32_t len;

    memset(seqnobuf, 0, sizeof(seqnobuf));
    seqnobuf[0] = 1;

    ptr = seqnobuf + 8;
    len = 8;
    sftp_msg_write_long(&ptr, &len, pkt->seqno);

    if (EVP_CipherInit(pctx, NULL, NULL, seqnobuf, 1) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error initializing %s cipher for encryption: %s", cipher->algo,
        sftp_crypto_get_errors());
      return -1;
    }
  }

  res = EVP_Cipher(pctx, buf, ptr, (datasz - datalen));
  if (res < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error encrypting %s data for client: %s", cipher->algo,
      sftp_crypto_get_errors());
    errno = EIO;
    return -1;
  }

  *buflen = (datasz - datalen);

#ifdef SFTP_DEBUG_PACKET
{
  unsigned int i;

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "encrypted packet data (len %lu):", (unsigned long) *buflen);
  for (i = 0; i < *buflen;) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "  %02x%02x %02x%02x %02x%02x %02x%02x",
      ((unsigned char *) buf)[i], ((unsigned char *) buf)[i+1],
      ((unsigned char *) buf)[i+2], ((unsigned char *) buf)[i+3],
      ((unsigned char *) buf)[i+4], ((unsigned char *) buf)[i+5],
      ((unsigned char *) buf)[i+6], ((unsigned char *) buf)[i+7]);
    i += 8;
  }
}
#endif

  if (auth_len > 0) {
    unsigned char *tag_data = NULL;
    uint32_t tag_datalen = 0;

    if (cipher->algo_type != SFTP_CIPHER_ALGO_CHACHA) {
      if (EVP_Cipher(pctx, NULL, NULL, 0) < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error generating %s authentication tag for client: %s",
          cipher->algo, sftp_crypto_get_errors());
        errno = EIO;
        return -1;
      }
    }

    tag_datalen = auth_len;
    tag_data = palloc(pkt->pool, tag_datalen);

    if (cipher->algo_type == SFTP_CIPHER_ALGO_GCM) {
#if defined(EVP_CTRL_GCM_GET_TAG)
      if (EVP_CIPHER_CTX_ctrl(pctx, EVP_CTRL_GCM_GET_TAG, tag_datalen,
          tag_data) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error getting %s authentication tag for client: %s", cipher->algo,
          sftp_crypto_get_errors());
        errno = EIO;
        return -1;
      }
#endif
    } else if (cipher->algo_type == SFTP_CIPHER_ALGO_CHACHA) {
#if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
   !defined(HAVE_BROKEN_CHACHA20)
      pool *tag_pool;
      unsigned char *tag_buf;
      size_t tag_bufsz;

      /* Here we want to compute our Poly1305 tag over the combination
       * of the encrypted packet length (pkt->aad) and the encrypted
       * payload (buf).
       *
       * Thus we need to assemble that here.
       */

      tag_pool = make_sub_pool(pkt->pool);
      tag_bufsz = pkt->aad_len + *buflen;
      tag_buf = palloc(tag_pool, tag_bufsz);

      memcpy(tag_buf, pkt->aad, pkt->aad_len);
      memcpy(tag_buf + pkt->aad_len, buf, *buflen);

      poly1305_auth(tag_data, tag_buf, tag_bufsz, chachapoly_key);
      destroy_pool(tag_pool);
#endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
    }

    pkt->mac_len = tag_datalen;
    pkt->mac = tag_data;
  }

  return 0;
}

#if OPENSSL_VERSION_NUMBER < 0x1000000fL
/* In older versions of OpenSSL, there was not a way to dynamically allocate
 * an EVP_CIPHER_CTX object.  Thus we have these static objects for those
 * older versions.
 */
static EVP_CIPHER_CTX read_ctx1, read_ctx2;
static EVP_CIPHER_CTX write_ctx1, write_ctx2;
#endif /* prior to OpenSSL-1.0.0 */

int sftp_cipher_init(void) {
#if OPENSSL_VERSION_NUMBER < 0x1000000fL
  read_ctxs[0] = &read_ctx1;
  read_ctxs[1] = &read_ctx2;
  write_ctxs[0] = &write_ctx1;
  write_ctxs[1] = &write_ctx2;
#else
  read_ctxs[0] = EVP_CIPHER_CTX_new();
  read_ctxs[1] = EVP_CIPHER_CTX_new();
  write_ctxs[0] = EVP_CIPHER_CTX_new();
  write_ctxs[1] = EVP_CIPHER_CTX_new();
# if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
    !defined(HAVE_BROKEN_CHACHA20)
  read_header_ctxs[0] = EVP_CIPHER_CTX_new();
  read_header_ctxs[1] = EVP_CIPHER_CTX_new();
  write_header_ctxs[0] = EVP_CIPHER_CTX_new();
  write_header_ctxs[1] = EVP_CIPHER_CTX_new();
# endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
#endif /* OpenSSL-1.0.0 and later */
  return 0;
}

int sftp_cipher_free(void) {
#if OPENSSL_VERSION_NUMBER >= 0x1000000fL
  if (read_ctxs[0] != NULL) {
    EVP_CIPHER_CTX_free(read_ctxs[0]);
    read_ctxs[0] = NULL;
  }

  if (read_ctxs[1] != NULL) {
    EVP_CIPHER_CTX_free(read_ctxs[1]);
    read_ctxs[1] = NULL;
  }

  if (write_ctxs[0] != NULL) {
    EVP_CIPHER_CTX_free(write_ctxs[0]);
    write_ctxs[0] = NULL;
  }

  if (write_ctxs[1] != NULL) {
    EVP_CIPHER_CTX_free(write_ctxs[1]);
    write_ctxs[1] = NULL;
  }

# if defined(HAVE_EVP_CHACHA20_OPENSSL) && \
    !defined(HAVE_BROKEN_CHACHA20)
  if (read_header_ctxs[0] != NULL) {
    EVP_CIPHER_CTX_free(read_header_ctxs[0]);
    read_header_ctxs[0] = NULL;
  }

  if (read_header_ctxs[1] != NULL) {
    EVP_CIPHER_CTX_free(read_header_ctxs[1]);
    read_header_ctxs[1] = NULL;
  }

  if (write_header_ctxs[0] != NULL) {
    EVP_CIPHER_CTX_free(write_header_ctxs[0]);
    write_header_ctxs[0] = NULL;
  }

  if (write_header_ctxs[1] != NULL) {
    EVP_CIPHER_CTX_free(write_header_ctxs[1]);
    write_header_ctxs[1] = NULL;
  }
# endif /* HAVE_EVP_CHACHA20_OPENSSL and !HAVE_BROKEN_CHACHA20 */
#endif /* OpenSSL-1.0.0 and later */
  return 0;
}
