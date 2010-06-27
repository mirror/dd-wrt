/*
 * ProFTPD - mod_sftp key exchange (kex)
 * Copyright (c) 2008-2009 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: kex.c,v 1.14 2009/11/22 21:46:02 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "msg.h"
#include "packet.h"
#include "session.h"
#include "cipher.h"
#include "mac.h"
#include "compress.h"
#include "kex.h"
#include "keys.h"
#include "crypto.h"
#include "disconnect.h"
#include "interop.h"
#include "tap.h"

#define SFTP_DH_PRIV_KEY_RANDOM_BITS	2048

extern module sftp_module;

/* For managing the kexinit process */
static pool *kex_pool = NULL;

static int kex_rekey_interval = 0;
static int kex_rekey_timeout = 0;
static int kex_rekey_timerno = -1;
static int kex_rekey_timeout_timerno = -1;

struct sftp_kex_names {
  const char *kex_algo;
  const char *server_hostkey_algo;
  const char *c2s_encrypt_algo;
  const char *s2c_encrypt_algo;
  const char *c2s_mac_algo;
  const char *s2c_mac_algo;
  const char *c2s_comp_algo;
  const char *s2c_comp_algo;
  const char *c2s_lang;
  const char *s2c_lang;
};

struct sftp_kex {
  /* Versions */
  const char *client_version;
  const char *server_version;

  /* KEXINIT lists from client */
  struct sftp_kex_names *client_names;

  /* KEXINIT lists from server. */
  struct sftp_kex_names *server_names;

  /* Session algorithms */
  struct sftp_kex_names *session_names;

  /* For constructing the session ID/hash */
  char *client_kexinit_payload;
  size_t client_kexinit_payload_len;

  char *server_kexinit_payload;
  size_t server_kexinit_payload_len;

  int first_kex_follows;

  /* Client-preferred hostkey type, based on algorithm:
   *
   *  "ssh-dss" --> EVP_PKEY_DSA
   *  "ssh-rsa" --> EVP_PKEY_RSA
   */
  int use_hostkey_type;

  /* Using group-exchange? */
  int use_gex;

  /* Using rsa key exchange? */
  int use_kexrsa;

  /* For generating the session ID */
  DH *dh;
  BIGNUM *e;
  const EVP_MD *hash;

  BIGNUM *k;
  const char *h;
  uint32_t hlen;

  RSA *rsa;
  unsigned char *rsa_encrypted;
  uint32_t rsa_encrypted_len;
};

static struct sftp_kex *kex_first_kex = NULL;
static struct sftp_kex *kex_rekey_kex = NULL;
static int kex_sent_kexinit = FALSE;

/* Diffie-Hellman group moduli */

static const char *dh_group1_str =
  "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74"
  "020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F1437"
  "4FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
  "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE65381FFFFFFFFFFFFFFFF";

static const char *dh_group14_str = 
  "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74"
  "020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F1437"
  "4FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
  "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF05"
  "98DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB"
  "9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
  "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
  "3995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF";

#define SFTP_DH_GROUP1_SHA1		1
#define SFTP_DH_GROUP14_SHA1		2
#define SFTP_DH_GEX_SHA1		3
#define SFTP_DH_GEX_SHA256		4
#define SFTP_KEXRSA_SHA1		5
#define SFTP_KEXRSA_SHA256		6

#define SFTP_KEXRSA_SHA1_SIZE		2048
#define SFTP_KEXRSA_SHA256_SIZE		3072

static const char *kex_client_version = NULL;
static const char *kex_server_version = NULL;
static unsigned char kex_digest_buf[EVP_MAX_MD_SIZE];

/* Used for access to a SFTPDHParamsFile during rekeys, even if the process
 * has chrooted itself.
 */
static FILE *kex_dhparams_fp = NULL;

static const char *trace_channel = "ssh2";

static int kex_rekey_timeout_cb(CALLBACK_FRAME) {
  pr_trace_msg(trace_channel, 5,
    "Failed to rekey before timeout, disconnecting client");
  SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  return 0;
}

static int kex_rekey_timer_cb(CALLBACK_FRAME) {
  pr_trace_msg(trace_channel, 17, "SFTPRekey timer expired, requesting rekey");
  sftp_kex_rekey();
  return 0;
}

static const unsigned char *calculate_h(struct sftp_kex *kex,
    const char *hostkey_data, size_t hostkey_datalen, const BIGNUM *k,
    uint32_t *hlen) {
  EVP_MD_CTX ctx;
  char *buf, *ptr;
  uint32_t buflen, bufsz;

  bufsz = buflen = 3072;

  /* XXX Is this buffer large enough? Too large? */
  ptr = buf = sftp_msg_getbuf(kex_pool, bufsz);

  /* Write all of the data into the buffer in the SSH2 format, and hash it. */

  /* First, the version strings */
  sftp_msg_write_string(&buf, &buflen, kex->client_version);
  sftp_msg_write_string(&buf, &buflen, kex->server_version);

  /* Client's KEXINIT */
  sftp_msg_write_int(&buf, &buflen, kex->client_kexinit_payload_len + 1);
  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);
  sftp_msg_write_data(&buf, &buflen, kex->client_kexinit_payload,
    kex->client_kexinit_payload_len, FALSE);

  /* Server's KEXINIT */
  sftp_msg_write_int(&buf, &buflen, kex->server_kexinit_payload_len + 1);
  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);
  sftp_msg_write_data(&buf, &buflen, kex->server_kexinit_payload,
    kex->server_kexinit_payload_len, FALSE);

  /* Hostkey data */
  sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);

  /* Client's key */
  sftp_msg_write_mpint(&buf, &buflen, kex->e);

  /* Server's key */
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->pub_key);

  /* Shared secret */
  sftp_msg_write_mpint(&buf, &buflen, k);

  /* In OpenSSL 0.9.6, many of the EVP_Digest* functions returned void, not
   * int.  Without these ugly OpenSSL version preprocessor checks, the
   * compiler will error out with "void value not ignored as it ought to be".
   */

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestInit(&ctx, kex->hash) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing message digest: %s", sftp_crypto_get_errors());
    BN_clear_free(kex->e);
    kex->e = NULL;
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestInit(&ctx, kex->hash);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, ptr, (bufsz - buflen)) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest: %s", sftp_crypto_get_errors());
    BN_clear_free(kex->e);
    kex->e = NULL;
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestUpdate(&ctx, ptr, (bufsz - buflen));
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestFinal(&ctx, kex_digest_buf, hlen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error finalizing message digest: %s", sftp_crypto_get_errors());
    BN_clear_free(kex->e);
    kex->e = NULL;
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestFinal(&ctx, kex_digest_buf, hlen);
#endif

  BN_clear_free(kex->e);
  kex->e = NULL;

  pr_memscrub(ptr, bufsz);
  return kex_digest_buf;
}

static const unsigned char *calculate_gex_h(struct sftp_kex *kex,
    const char *hostkey_data, size_t hostkey_datalen, const BIGNUM *k,
    uint32_t min, uint32_t pref, uint32_t max, uint32_t *hlen) {
  EVP_MD_CTX ctx;
  char *buf, *ptr;
  uint32_t buflen, bufsz;

  bufsz = buflen = 8192;

  /* XXX Is this buffer large enough? Too large? */
  ptr = buf = sftp_msg_getbuf(kex_pool, bufsz);

  /* Write all of the data into the buffer in the SSH2 format, and hash it.
   * The ordering of these fields is described in RFC4419.
   */

  /* First, the version strings */
  sftp_msg_write_string(&buf, &buflen, kex->client_version);
  sftp_msg_write_string(&buf, &buflen, kex->server_version);

  /* Client's KEXINIT */
  sftp_msg_write_int(&buf, &buflen, kex->client_kexinit_payload_len + 1);
  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);
  sftp_msg_write_data(&buf, &buflen, kex->client_kexinit_payload,
    kex->client_kexinit_payload_len, FALSE);

  /* Server's KEXINIT */
  sftp_msg_write_int(&buf, &buflen, kex->server_kexinit_payload_len + 1);
  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);
  sftp_msg_write_data(&buf, &buflen, kex->server_kexinit_payload,
    kex->server_kexinit_payload_len, FALSE);

  /* Hostkey data */
  sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);

  if (min == 0 ||
      max == 0) {
    sftp_msg_write_int(&buf, &buflen, pref);

  } else {
    sftp_msg_write_int(&buf, &buflen, min);
    sftp_msg_write_int(&buf, &buflen, pref);
    sftp_msg_write_int(&buf, &buflen, max);
  }

  sftp_msg_write_mpint(&buf, &buflen, kex->dh->p);
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->g);

  /* Client's key */
  sftp_msg_write_mpint(&buf, &buflen, kex->e);

  /* Server's key */
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->pub_key);

  /* Shared secret */
  sftp_msg_write_mpint(&buf, &buflen, k);

  /* In OpenSSL 0.9.6, many of the EVP_Digest* functions returned void, not
   * int.  Without these ugly OpenSSL version preprocessor checks, the
   * compiler will error out with "void value not ignored as it ought to be".
   */

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestInit(&ctx, kex->hash) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing message digest: %s", sftp_crypto_get_errors());
    BN_clear_free(kex->e);
    kex->e = NULL;
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestInit(&ctx, kex->hash);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, ptr, (bufsz - buflen)) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest: %s", sftp_crypto_get_errors());
    BN_clear_free(kex->e);
    kex->e = NULL;
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestUpdate(&ctx, ptr, (bufsz - buflen));
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestFinal(&ctx, kex_digest_buf, hlen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error finalizing message digest: %s", sftp_crypto_get_errors());
    BN_clear_free(kex->e);
    kex->e = NULL;
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestFinal(&ctx, kex_digest_buf, hlen);
#endif

  BN_clear_free(kex->e);
  kex->e = NULL;
  pr_memscrub(ptr, bufsz);

  return kex_digest_buf;
}

static const unsigned char *calculate_kexrsa_h(struct sftp_kex *kex,
    const char *hostkey_data, size_t hostkey_datalen, const BIGNUM *k,
    char *rsa_key, uint32_t rsa_keylen, uint32_t *hlen) {
  EVP_MD_CTX ctx;
  char *buf, *ptr;
  uint32_t buflen, bufsz;

  bufsz = buflen = 3072;

  /* XXX Is this buffer large enough? Too large? */
  ptr = buf = sftp_msg_getbuf(kex_pool, bufsz);

  /* Write all of the data into the buffer in the SSH2 format, and hash it. */

  /* First, the version strings */
  sftp_msg_write_string(&buf, &buflen, kex->client_version);
  sftp_msg_write_string(&buf, &buflen, kex->server_version);

  /* Client's KEXINIT */
  sftp_msg_write_int(&buf, &buflen, kex->client_kexinit_payload_len + 1);
  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);
  sftp_msg_write_data(&buf, &buflen, kex->client_kexinit_payload,
    kex->client_kexinit_payload_len, FALSE);

  /* Server's KEXINIT */
  sftp_msg_write_int(&buf, &buflen, kex->server_kexinit_payload_len + 1);
  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);
  sftp_msg_write_data(&buf, &buflen, kex->server_kexinit_payload,
    kex->server_kexinit_payload_len, FALSE);

  /* Hostkey data */
  sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);

  /* Transient RSA public key */
  sftp_msg_write_data(&buf, &buflen, rsa_key, rsa_keylen, TRUE);

  /* RSA-encrypted secret */
  sftp_msg_write_data(&buf, &buflen, (const char *) kex->rsa_encrypted,
    kex->rsa_encrypted_len, TRUE);

  /* Shared secret. */
  sftp_msg_write_mpint(&buf, &buflen, k);

  /* In OpenSSL 0.9.6, many of the EVP_Digest* functions returned void, not
   * int.  Without these ugly OpenSSL version preprocessor checks, the
   * compiler will error out with "void value not ignored as it ought to be".
   */

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestInit(&ctx, kex->hash) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing message digest: %s", sftp_crypto_get_errors());
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestInit(&ctx, kex->hash);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, ptr, (bufsz - buflen)) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest: %s", sftp_crypto_get_errors());
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestUpdate(&ctx, ptr, (bufsz - buflen));
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestFinal(&ctx, kex_digest_buf, hlen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error finalizing message digest: %s", sftp_crypto_get_errors());
    pr_memscrub(ptr, bufsz);
    return NULL;
  }
#else
  EVP_DigestFinal(&ctx, kex_digest_buf, hlen);
#endif

  pr_memscrub(ptr, bufsz);
  return kex_digest_buf;
}

/* Make sure that the DH key we're generating is good enough. */
static int have_good_dh(DH *dh, BIGNUM *pub_key) {
  register unsigned int i;
  unsigned int nbits = 0;
  BIGNUM *tmp;

  if (pub_key->neg) {
    pr_trace_msg(trace_channel, 10,
      "DH public keys cannot have negative numbers");
    errno = EINVAL;
    return -1;
  }

  if (BN_cmp(pub_key, BN_value_one()) != 1) {
    pr_trace_msg(trace_channel, 10, "bad DH public key exponent (<= 1)");
    errno = EINVAL;
    return -1;
  }

  tmp = BN_new();
  if (!BN_sub(tmp, dh->p, BN_value_one()) ||
      BN_cmp(pub_key, tmp) != -1) {
    BN_clear_free(tmp);
    pr_trace_msg(trace_channel, 10, "bad DH public key (>= p-1)");
    errno = EINVAL;
    return -1;
  }

  BN_clear_free(tmp);

  for (i = 0; i <= BN_num_bits(pub_key); i++) {
    if (BN_is_bit_set(pub_key, i)) {
      nbits++;
    }
  }

  /* The number of bits set in the public key must be greater than one.
   * Otherwise, the public key will not hold up under scrutiny, not for
   * our needs.  (The OpenSSH client is picky about the DH public keys it
   * will accept as well, so this is necessary to pass OpenSSH's requirements.
   */
  if (nbits <= 1) {
    errno = EINVAL;
    return -1;
  }

  pr_trace_msg(trace_channel, 10, "good DH public key: %u bits set", nbits);
  return 0;
}

static int create_dh(struct sftp_kex *kex, int type) {
  unsigned int attempts = 0;
  DH *dh;

  if (type != SFTP_DH_GROUP1_SHA1 &&
      type != SFTP_DH_GROUP14_SHA1) {
    errno = EINVAL;
    return -1;
  }

  if (kex->dh) {
    if (kex->dh->p) {
      BN_clear_free(kex->dh->p);
      kex->dh->p = NULL;
    }

    if (kex->dh->g) {
      BN_clear_free(kex->dh->g);
      kex->dh->g = NULL;
    }

    if (kex->dh->priv_key) {
      BN_clear_free(kex->dh->priv_key);
      kex->dh->priv_key = NULL;
    }

    if (kex->dh->pub_key) {
      BN_clear_free(kex->dh->pub_key);
      kex->dh->pub_key = NULL;
    }

    DH_free(kex->dh);
    kex->dh = NULL;
  }

  /* We have 10 attempts to make a DH key which passes muster. */
  while (attempts <= 10) {
    pr_signals_handle();

    attempts++;
    pr_trace_msg(trace_channel, 9, "attempt #%u to create a good DH key",
      attempts);

    dh = DH_new();
    if (!dh) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error creating DH: %s", sftp_crypto_get_errors());
      return -1;
    }

    dh->p = BN_new();
    dh->g = BN_new();
    dh->priv_key = BN_new();
  
    if (type == SFTP_DH_GROUP1_SHA1) {
      if (BN_hex2bn(&dh->p, dh_group1_str) == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error setting DH (group1) P: %s", sftp_crypto_get_errors());
        DH_free(dh);
        return -1;
      }

    } else if (type == SFTP_DH_GROUP14_SHA1) {
      if (BN_hex2bn(&dh->p, dh_group14_str) == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error setting DH (group14) P: %s", sftp_crypto_get_errors());
        DH_free(dh);
        return -1;
      }
    }

    if (BN_hex2bn(&dh->g, "2") == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting DH G: %s", sftp_crypto_get_errors());
      DH_free(dh);
      return -1;
    }

    if (!BN_rand(dh->priv_key, SFTP_DH_PRIV_KEY_RANDOM_BITS, 0, 0)) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error generating DH random key (%d bytes): %s",
        SFTP_DH_PRIV_KEY_RANDOM_BITS, sftp_crypto_get_errors());
      DH_free(dh);
      return -1;
    }

    pr_trace_msg(trace_channel, 12, "generating DH key");
    if (DH_generate_key(dh) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error generating DH key: %s", sftp_crypto_get_errors());
      DH_free(dh);
      return -1;
    }

    if (have_good_dh(dh, dh->pub_key) < 0) {
      DH_free(dh);
      continue;
    }

    kex->dh = dh;
    kex->hash = EVP_sha1();
    return 0;
  }

  errno = EPERM;
  return -1;
}

static int prepare_dh(struct sftp_kex *kex, int type) {
  DH *dh;

  if (type != SFTP_DH_GEX_SHA1 &&
      type != SFTP_DH_GEX_SHA256) {
    errno = EINVAL;
    return -1;
  }

  if (kex->dh) {
    if (kex->dh->p) {
      BN_clear_free(kex->dh->p);
      kex->dh->p = NULL;
    }

    if (kex->dh->g) {
      BN_clear_free(kex->dh->g);
      kex->dh->g = NULL;
    }

    if (kex->dh->priv_key) {
      BN_clear_free(kex->dh->priv_key);
      kex->dh->priv_key = NULL;
    }

    if (kex->dh->pub_key) {
      BN_clear_free(kex->dh->pub_key);
      kex->dh->pub_key = NULL;
    }

    DH_free(kex->dh);
    kex->dh = NULL;
  }

  dh = DH_new();
  if (!dh) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error creating DH: %s", sftp_crypto_get_errors());
    return -1;
  }

  kex->dh = dh;

  if (type == SFTP_DH_GEX_SHA1) {
    kex->hash = EVP_sha1();

#if (OPENSSL_VERSION_NUMBER > 0x000907000L && defined(OPENSSL_FIPS)) || \
    (OPENSSL_VERSION_NUMBER > 0x000908000L)
  } else if (type == SFTP_DH_GEX_SHA256) {
    kex->hash = EVP_sha256();
#endif
  }

  return 0;
}

static int finish_dh(struct sftp_kex *kex) {
  unsigned int attempts = 0;

  /* We have 10 attempts to make a DH key which passes muster. */
  while (attempts <= 10) {
    pr_signals_handle();

    attempts++;
    pr_trace_msg(trace_channel, 9, "attempt #%u to create a good DH key",
      attempts);

    kex->dh->priv_key = BN_new();
  
    if (!BN_rand(kex->dh->priv_key, SFTP_DH_PRIV_KEY_RANDOM_BITS, 0, 0)) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error generating DH random key (%d bytes): %s",
        SFTP_DH_PRIV_KEY_RANDOM_BITS, sftp_crypto_get_errors());
      return -1;
    }

    pr_trace_msg(trace_channel, 12, "generating DH key");
    if (DH_generate_key(kex->dh) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error generating DH key: %s", sftp_crypto_get_errors());
      return -1;
    }

    if (have_good_dh(kex->dh, kex->e) < 0) {
      if (kex->dh->priv_key) {
        BN_clear_free(kex->dh->priv_key);
        kex->dh->priv_key = NULL;
      } 

      if (kex->dh->pub_key) {
        BN_clear_free(kex->dh->pub_key);
        kex->dh->pub_key = NULL;
      } 

      continue;
    }

    return 0;
  }

  errno = EPERM;
  return -1;
}

static int create_kexrsa(struct sftp_kex *kex, int type) {
  RSA *rsa = NULL;

  if (type != SFTP_KEXRSA_SHA1 &&
      type != SFTP_KEXRSA_SHA256) {
    errno = EINVAL;
    return -1;
  }

  if (kex->rsa) {
    RSA_free(kex->rsa);
    kex->rsa = NULL;
  }

  if (kex->rsa_encrypted) {
    pr_memscrub(kex->rsa_encrypted, kex->rsa_encrypted_len);
    kex->rsa_encrypted = NULL;
    kex->rsa_encrypted_len = 0;
  }

  if (type == SFTP_KEXRSA_SHA1) {
    rsa = RSA_generate_key(SFTP_KEXRSA_SHA1_SIZE, 17, NULL, NULL);
    if (!rsa) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error generating %u-bit RSA key: %s", SFTP_KEXRSA_SHA1_SIZE,
        sftp_crypto_get_errors());
      return -1;
    }

    kex->hash = EVP_sha1();

#if (OPENSSL_VERSION_NUMBER > 0x000907000L && defined(OPENSSL_FIPS)) || \
    (OPENSSL_VERSION_NUMBER > 0x000908000L)
  } else if (type == SFTP_KEXRSA_SHA256) {
    rsa = RSA_generate_key(SFTP_KEXRSA_SHA256_SIZE, 65537, NULL, NULL);
    if (!rsa) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error generating %u-bit RSA key: %s", SFTP_KEXRSA_SHA256_SIZE,
        sftp_crypto_get_errors());
      return -1;
    }

    kex->hash = EVP_sha256();
#endif
  }

#if OPENSSL_VERSION_NUMBER < 0x0090702fL
  /* In OpenSSL-0.9.7a and later, RSA blinding is turned on by default.  Thus
   * if our OpenSSL is older than that, manually enable RSA blinding.
   */
  if (RSA_blinding_on(rsa, NULL) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error enabling RSA blinding for generated key: %s",
      sftp_crypto_get_errors());

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "RSA blinding enabled for generated key");
  }
#endif

  kex->rsa = rsa;
  return 0;
}

static array_header *parse_namelist(pool *p, const char *names) {
  char *ptr;
  array_header *list;

  list = make_array(p, 0, sizeof(const char *));

  ptr = strchr(names, ',');
  while (ptr) {
    char *elt;

    pr_signals_handle();

    elt = pcalloc(p, (ptr - names) + 1);
    memcpy(elt, names, (ptr - names));

    *((const char **) push_array(list)) = elt;
    names = ++ptr;

    ptr = strchr(names, ',');
  }
  *((const char **) push_array(list)) = pstrdup(p, names);

  return list;
}

/* Given a name-list, return the first (i.e. preferred) name in the list. */
static const char *get_preferred_name(pool *p, const char *names) {
  register unsigned int i;

  /* Advance to the first comma, or NUL. */
  for (i = 0; names[i] && names[i] != ','; i++);
  
  if (names[i] == ',' ||
      names[i] == '\0') {
    char *pref;

    pref = pcalloc(p, i + 1);
    memcpy(pref, names, i);

    return pref;
  }

  /* This should never happen. */
  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "unable to find preferred name in '%s'", names ? names : "(null)");
  return NULL;
}

/* Given name-lists from the client and server, find the first name from the
 * client list which appears on the server list.
 */
static const char *get_shared_name(pool *p, const char *c2s_names,
    const char *s2c_names) {
  register unsigned int i;
  const char *name = NULL, **client_names, **server_names;
  pool *tmp_pool;
  array_header *client_list, *server_list;

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "SSH2 session shared name pool");

  client_list = parse_namelist(tmp_pool, c2s_names);
  client_names = (const char **) client_list->elts;

  server_list = parse_namelist(tmp_pool, s2c_names);
  server_names = (const char **) server_list->elts;

  for (i = 0; i < client_list->nelts; i++) {
    register unsigned int j;

    if (name)
      break;

    for (j = 0; j < server_list->nelts; j++) {
      if (strcmp(client_names[i], server_names[j]) == 0) {
        name = client_names[i];
        break;
      }
    }
  }

  name = pstrdup(p, name);
  destroy_pool(tmp_pool);

  return name;
}

static const char *kex_exchanges[] = {
#if (OPENSSL_VERSION_NUMBER > 0x000907000L && defined(OPENSSL_FIPS)) || \
    (OPENSSL_VERSION_NUMBER > 0x000908000L)
  "diffie-hellman-group-exchange-sha256",
#endif
  "diffie-hellman-group-exchange-sha1",
  "diffie-hellman-group14-sha1",
  "diffie-hellman-group1-sha1",

#if 0
/* We cannot currently support rsa2048-sha256, since it requires support
 * for PKCS#1 v2.1 (RFC3447).  OpenSSL only supports PKCS#1 v2.0 (RFC2437)
 * at present, which only allows EME-OAEP using SHA1.  v2.1 allows for
 * using other message digests, e.g. SHA256, for EME-OAEP.
 */
#if (OPENSSL_VERSION_NUMBER > 0x000907000L && defined(OPENSSL_FIPS)) || \
    (OPENSSL_VERSION_NUMBER > 0x000908000L)
  "rsa2048-sha256",
#endif
#endif

  "rsa1024-sha1",
  NULL,
};

static const char *get_kexinit_exchange_list(pool *p) {
  char *res = "";
  config_rec *c;

  c = find_config(main_server->conf, CONF_PARAM, "SFTPKeyExchanges", FALSE);
  if (c) {
    res = pstrdup(p, c->argv[0]);

  } else {
    register unsigned int i;

    for (i = 0; kex_exchanges[i]; i++) {
      res = pstrcat(p, res, *res ? "," : "", pstrdup(p, kex_exchanges[i]),
        NULL);
    }
  }

  return res;
}

static struct sftp_kex *create_kex(pool *p) {
  struct sftp_kex *kex;
  const char *list;
  config_rec *c;

  kex = pcalloc(p, sizeof(struct sftp_kex));
  kex->client_version = kex_client_version;
  kex->server_version = kex_server_version;
  kex->client_names = pcalloc(p, sizeof(struct sftp_kex_names));
  kex->server_names = pcalloc(p, sizeof(struct sftp_kex_names));
  kex->session_names = pcalloc(p, sizeof(struct sftp_kex_names));
  kex->use_hostkey_type = 0;
  kex->dh = NULL;
  kex->e = NULL;
  kex->hash = NULL;
  kex->k = NULL;
  kex->h = NULL;
  kex->hlen = 0;
  kex->rsa = NULL;
  kex->rsa_encrypted = NULL;
  kex->rsa_encrypted_len = 0;

  list = get_kexinit_exchange_list(kex_pool);
  kex->server_names->kex_algo = list;

  /* Our list of supported hostkey algorithms depends on the hostkeys
   * that have been configured.  Show a preference for RSA over DSA.
   * XXX Should this be configurable later?
   */

  if (sftp_keys_have_dsa_hostkey() == 0 &&
      sftp_keys_have_rsa_hostkey() == 0) {
    kex->server_names->server_hostkey_algo = "ssh-rsa,ssh-dss";

  } else if (sftp_keys_have_rsa_hostkey() == 0) {
    kex->server_names->server_hostkey_algo = "ssh-rsa";

  } else if (sftp_keys_have_dsa_hostkey() == 0) {
    kex->server_names->server_hostkey_algo = "ssh-dss";
  }

  list = sftp_crypto_get_kexinit_cipher_list(kex_pool);
  kex->server_names->c2s_encrypt_algo = list;
  kex->server_names->s2c_encrypt_algo = list;

  list = sftp_crypto_get_kexinit_digest_list(kex_pool);
  kex->server_names->c2s_mac_algo = list;
  kex->server_names->s2c_mac_algo = list;

  c = find_config(main_server->conf, CONF_PARAM, "SFTPCompression", FALSE);
  if (c) {
    int comp_mode;

    comp_mode = *((int *) c->argv[0]);

    switch (comp_mode) {
      case 2:
        /* Advertise that we support OpenSSH's "delayed" compression mode. */
        kex->server_names->c2s_comp_algo = "zlib@openssh.com,zlib,none";
        kex->server_names->s2c_comp_algo = "zlib@openssh.com,zlib,none";
        break;

      case 1:
        kex->server_names->c2s_comp_algo = "zlib,none";
        kex->server_names->s2c_comp_algo = "zlib,none";
        break;

      default:
        kex->server_names->c2s_comp_algo = "none";
        kex->server_names->s2c_comp_algo = "none";
        break;
    }

  } else {
    kex->server_names->c2s_comp_algo = "none";
    kex->server_names->s2c_comp_algo = "none";
  }

#ifdef PR_USE_NLS
  c = find_config(main_server->conf, CONF_PARAM, "SFTPLanguages", FALSE);
  if (c) {
    /* XXX Need to implement functionality here. */

  } else {
    kex->server_names->c2s_lang = "en_US";
    kex->server_names->s2c_lang = "en_US";
  }
#else
  kex->server_names->c2s_lang = "";
  kex->server_names->s2c_lang = "";
#endif /* !PR_USE_NLS */

  return kex;
}

static void destroy_kex(struct sftp_kex *kex) {
  if (kex) {
    if (kex->dh) {
      if (kex->dh->p) {
        BN_clear_free(kex->dh->p);
        kex->dh->p = NULL;
      }

      if (kex->dh->g) {
        BN_clear_free(kex->dh->g);
        kex->dh->g = NULL;
      }

      DH_free(kex->dh);
      kex->dh = NULL;
    }

    if (kex->rsa) {
      RSA_free(kex->rsa);
      kex->rsa = NULL;
    }

    if (kex->rsa_encrypted) {
      pr_memscrub(kex->rsa_encrypted, kex->rsa_encrypted_len);
      kex->rsa_encrypted = NULL;
      kex->rsa_encrypted_len = 0;
    }

    if (kex->e) {
      BN_clear_free(kex->e);
      kex->e = NULL;
    }

    if (kex->k) {
      BN_clear_free(kex->k);
      kex->k = NULL;
    }

    if (kex->hlen > 0) {
      pr_memscrub((char *) kex->h, kex->hlen);
      kex->hlen = 0;
    }
  }

  kex_first_kex = kex_rekey_kex = NULL;
}

static int setup_kex_algo(struct sftp_kex *kex, const char *algo) {

  if (strcmp(algo, "diffie-hellman-group1-sha1") == 0) {
    if (create_dh(kex, SFTP_DH_GROUP1_SHA1) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using '%s' as the key exchange algorithm: %s", algo,
        strerror(errno));
      return -1;
    }

    kex->session_names->kex_algo = algo;
    return 0;

  } else if (strcmp(algo, "diffie-hellman-group14-sha1") == 0) {
    if (create_dh(kex, SFTP_DH_GROUP14_SHA1) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using '%s' as the key exchange algorithm: %s", algo,
        strerror(errno));
      return -1;
    }

    kex->session_names->kex_algo = algo;
    return 0;

  } else if (strcmp(algo, "diffie-hellman-group-exchange-sha1") == 0) {
    if (prepare_dh(kex, SFTP_DH_GEX_SHA1) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using '%s' as the key exchange algorithm: %s", algo,
        strerror(errno));
      return -1;
    }

    kex->session_names->kex_algo = algo;
    kex->use_gex = TRUE;
    return 0;

  } else if (strcmp(algo, "rsa1024-sha1") == 0) {
    if (create_kexrsa(kex, SFTP_KEXRSA_SHA1) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using '%s' as the key exchange algorithm: %s", algo,
        strerror(errno));
      return -1;
    }

    kex->session_names->kex_algo = algo;
    kex->use_kexrsa = TRUE;
    return 0;

#if (OPENSSL_VERSION_NUMBER > 0x000907000L && defined(OPENSSL_FIPS)) || \
    (OPENSSL_VERSION_NUMBER > 0x000908000L)
  } else if (strcmp(algo, "diffie-hellman-group-exchange-sha256") == 0) {
    if (prepare_dh(kex, SFTP_DH_GEX_SHA256) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using '%s' as the key exchange algorithm: %s", algo,
        strerror(errno));
      return -1;
    }

    kex->session_names->kex_algo = algo;
    kex->use_gex = TRUE;
    return 0;

  } else if (strcmp(algo, "rsa2048-sha256") == 0) {
    if (create_kexrsa(kex, SFTP_KEXRSA_SHA256) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error using '%s' as the key exchange algorithm: %s", algo,
        strerror(errno));
      return -1;
    }

    kex->session_names->kex_algo = algo;
    kex->use_kexrsa = TRUE;
    return 0;
#endif
  }

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "unsupported key exchange algorithm '%s'", algo);
  errno = EINVAL;
  return -1;
}

static int setup_hostkey_algo(struct sftp_kex *kex, const char *algo) {
  kex->session_names->server_hostkey_algo = (char *) algo;

  if (strcmp(algo, "ssh-dss") == 0) {
    kex->use_hostkey_type = EVP_PKEY_DSA;
    return 0;
  }

  if (strcmp(algo, "ssh-rsa") == 0) {
    kex->use_hostkey_type = EVP_PKEY_RSA;
    return 0;
  }

  errno = EINVAL;
  return -1;
}

static int setup_c2s_encrypt_algo(struct sftp_kex *kex, const char *algo) {
  (void) kex;

  if (sftp_cipher_set_read_algo(algo) < 0)
    return -1;

  return 0;
}

static int setup_s2c_encrypt_algo(struct sftp_kex *kex, const char *algo) {
  (void) kex;

  if (sftp_cipher_set_write_algo(algo) < 0)
    return -1;

  return 0;
}

static int setup_c2s_mac_algo(struct sftp_kex *kex, const char *algo) {
  (void) kex;

  if (sftp_mac_set_read_algo(algo) < 0)
    return -1;

  return 0;
}

static int setup_s2c_mac_algo(struct sftp_kex *kex, const char *algo) {
  (void) kex;

  if (sftp_mac_set_write_algo(algo) < 0)
    return -1;

  return 0;
}

static int setup_c2s_comp_algo(struct sftp_kex *kex, const char *algo) {
  (void) kex;

  if (sftp_compress_set_read_algo(algo) < 0)
    return -1;

  return 0;
}

static int setup_s2c_comp_algo(struct sftp_kex *kex, const char *algo) {
  (void) kex;

  if (sftp_compress_set_write_algo(algo) < 0)
    return -1;

  return 0;
}

static int setup_c2s_lang(struct sftp_kex *kex, const char *lang) {
  (void) kex;

  /* XXX Need to implement the functionality here. */

  return 0;
}

static int setup_s2c_lang(struct sftp_kex *kex, const char *lang) {
  (void) kex;

  /* XXX Need to implement the functionality here. */

  return 0;
}

static int get_session_names(struct sftp_kex *kex, int *correct_guess) {
  const char *client_pref, *server_pref, *shared, *client_list, *server_list;
  pool *tmp_pool;

  tmp_pool = make_sub_pool(kex_pool);
  pr_pool_tag(tmp_pool, "SSH2 session shared name pool");

  client_list = kex->client_names->kex_algo;
  server_list = kex->server_names->kex_algo;

  pr_trace_msg(trace_channel, 8, "client-sent key exchange algorithms: %s",
    client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_kex_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session key exchange: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared key exchange algorithm found (client sent '%s', server sent "
      "'%s')", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->server_hostkey_algo;
  server_list = kex->server_names->server_hostkey_algo;

  pr_trace_msg(trace_channel, 8,
    "client-sent host key algorithms: %s", client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_hostkey_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session server hostkey: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared server hostkey algorithm found (client sent '%s', server sent "
      "'%s'", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->c2s_encrypt_algo;
  server_list = kex->server_names->c2s_encrypt_algo;

  pr_trace_msg(trace_channel, 8, "client-sent client encryption algorithms: %s",
    client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_c2s_encrypt_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session client-to-server encryption: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared client-to-server encryption algorithm found (client sent '%s',"
      " server sent '%s')", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->s2c_encrypt_algo;
  server_list = kex->server_names->s2c_encrypt_algo;

  pr_trace_msg(trace_channel, 8, "client-sent server encryption algorithms: %s",
    client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_s2c_encrypt_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session server-to-client encryption: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared server-to-client encryption algorithm found (client sent '%s',"
      " server sent '%s')", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->c2s_mac_algo;
  server_list = kex->server_names->c2s_mac_algo;

  pr_trace_msg(trace_channel, 8, "client-sent client MAC algorithms: %s",
    client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_c2s_mac_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session client-to-server MAC: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared client-to-server MAC algorithm found (client sent '%s', "
      "server sent '%s')", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->s2c_mac_algo;
  server_list = kex->server_names->s2c_mac_algo;

  pr_trace_msg(trace_channel, 8, "client-sent server MAC algorithms: %s",
    client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_s2c_mac_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session server-to-client MAC: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared server-to-client MAC algorithm found (client sent '%s', "
      "server sent '%s')", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->c2s_comp_algo;
  server_list = kex->server_names->c2s_comp_algo;

  pr_trace_msg(trace_channel, 8,
    "client-sent client compression algorithms: %s", client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_c2s_comp_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session client-to-server compression: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared client-to-server compression algorithm found (client sent "
      "'%s', server sent '%s'", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->s2c_comp_algo;
  server_list = kex->server_names->s2c_comp_algo;

  pr_trace_msg(trace_channel, 8,
    "client-sent server compression algorithms: %s", client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_s2c_comp_algo(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session server-to-client compression: %s", shared);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared server-to-client compression algorithm found (client sent "
      "'%s', server sent '%s'", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
  }

  client_list = kex->client_names->c2s_lang;
  server_list = kex->server_names->c2s_lang;

  pr_trace_msg(trace_channel, 8,
    "client-sent client languages: %s", client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_c2s_lang(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session client-to-server language: %s", shared);

/* XXX Do not error out if there are no shared languages yet. */
#if 0
  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared client-to-server language found (client sent '%s', server "
      "sent '%s'", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
#endif
  }

  client_list = kex->client_names->s2c_lang;
  server_list = kex->server_names->s2c_lang;

  pr_trace_msg(trace_channel, 8,
    "client-sent server languages: %s", client_list);

  client_pref = get_preferred_name(tmp_pool, client_list);
  server_pref = get_preferred_name(tmp_pool, server_list);

  if (*correct_guess &&
      client_pref &&
      server_pref &&
      strcmp(client_pref, server_pref) != 0) {
    *correct_guess = FALSE;
  }

  shared = get_shared_name(kex_pool, client_list, server_list);
  if (shared) {
    if (setup_s2c_lang(kex, shared) < 0) {
      destroy_pool(tmp_pool);
      return -1;
    }

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      " + Session server-to-client language: %s", shared);

/* XXX Do not error out if there are no shared languages yet. */
#if 0
  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "no shared server-to-client language found (client sent '%s', server "
      "sent '%s'", client_list, server_list);
    destroy_pool(tmp_pool);
    return -1;
#endif
  }

  destroy_pool(tmp_pool);
  return 0;
}

static int read_kexinit(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  char *buf, *cookie, *list;
  uint32_t buflen;

  buf = pkt->payload;
  buflen = pkt->payload_len;

  /* Make a copy of the payload for later. */
  kex->client_kexinit_payload = palloc(kex_pool, pkt->payload_len);
  kex->client_kexinit_payload_len = pkt->payload_len;
  memcpy(kex->client_kexinit_payload, pkt->payload, pkt->payload_len);

  /* Read the cookie, which is a mandated length of 16 bytes. */
  cookie = sftp_msg_read_data(pkt->pool, &buf, &buflen, 16);

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->kex_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->server_hostkey_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->c2s_encrypt_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->s2c_encrypt_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->c2s_mac_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->s2c_mac_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->c2s_comp_algo = list;

  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->s2c_comp_algo = list;

  /* Client-to-server languages */
  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->c2s_lang = list;

  /* Server-to-client languages */
  list = sftp_msg_read_string(kex_pool, &buf, &buflen);
  kex->client_names->s2c_lang = list;

  /* Read the "first kex packet follows" byte */
  kex->first_kex_follows = sftp_msg_read_bool(pkt->pool, &buf, &buflen);

  pr_trace_msg(trace_channel, 3, "first kex packet follows = %s",
    kex->first_kex_follows ? "true" : "false");

  /* Reserved flags */
  (void) sftp_msg_read_int(pkt->pool, &buf, &buflen);

  return 0;
}

static int write_kexinit(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  unsigned char cookie[16];
  char *buf, *ptr;
  const char *list;
  uint32_t bufsz, buflen;

  /* XXX Always have empty language lists; we really don't care. */
  const char *langs = "";

  bufsz = buflen = sizeof(char) +
    sizeof(cookie) +
    sizeof(uint32_t) + strlen(kex->server_names->kex_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->server_hostkey_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->c2s_encrypt_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->s2c_encrypt_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->c2s_mac_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->s2c_mac_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->c2s_comp_algo) +
    sizeof(uint32_t) + strlen(kex->server_names->s2c_comp_algo) +
    sizeof(uint32_t) + strlen(langs) +
    sizeof(uint32_t) + strlen(langs) +
    sizeof(char) +
    sizeof(uint32_t);

  ptr = buf = pcalloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXINIT);

  /* Try first to use cryptographically secure bytes for the cookie.
   * If that fails (e.g. if the PRNG hasn't been seeded well), use
   * pseudo-cryptographically secure bytes.
   */
  memset(cookie, 0, sizeof(cookie));
  if (RAND_bytes(cookie, sizeof(cookie)) != 1) {
    RAND_pseudo_bytes(cookie, sizeof(cookie));
  }

  sftp_msg_write_data(&buf, &buflen, (char *) cookie, sizeof(cookie), FALSE);

  list = kex->server_names->kex_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->server_hostkey_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->c2s_encrypt_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->s2c_encrypt_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->c2s_mac_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->s2c_mac_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->c2s_comp_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  list = kex->server_names->s2c_comp_algo;
  sftp_msg_write_string(&buf, &buflen, list);

  /* XXX Need to support langs here. */
  sftp_msg_write_string(&buf, &buflen, langs);
  sftp_msg_write_string(&buf, &buflen, langs);

  /* We don't try to optimistically guess what algorithms the client would
   * use and send a preemptive kex packet.
   */
  sftp_msg_write_bool(&buf, &buflen, FALSE);
  sftp_msg_write_int(&buf, &buflen, 0);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  /* Make a copy of the payload for later. Skip past the first byte, which
   * is the KEXINIT identifier.
   */
  kex->server_kexinit_payload_len = pkt->payload_len - 1;
  kex->server_kexinit_payload = palloc(kex_pool, pkt->payload_len - 1);
  memcpy(kex->server_kexinit_payload, pkt->payload + 1, pkt->payload_len - 1);

  return 0;
}

static int read_dh_init(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  char *buf;
  uint32_t buflen;

  buf = pkt->payload;
  buflen = pkt->payload_len;

  /* Read in 'e' */
  kex->e = sftp_msg_read_mpint(pkt->pool, &buf, &buflen);
  if (kex->e == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading DH_INIT: %s", strerror(errno));
    return -1;
  }

  return 0;
}

static int set_session_keys(struct sftp_kex *kex) {
  const char *k, *v;

  if (sftp_cipher_set_read_key(kex_pool, kex->hash, kex->k, kex->h,
      kex->hlen) < 0)
    return -1;

  if (sftp_cipher_set_write_key(kex_pool, kex->hash, kex->k, kex->h,
      kex->hlen) < 0)
    return -1;

  if (sftp_mac_set_read_key(kex_pool, kex->hash, kex->k, kex->h,
      kex->hlen) < 0)
    return -1;

  if (sftp_mac_set_write_key(kex_pool, kex->hash, kex->k, kex->h,
      kex->hlen) < 0)
    return -1;

  if (sftp_compress_init_read(SFTP_COMPRESS_FL_NEW_KEY) < 0)
    return -1;

  if (sftp_compress_init_write(SFTP_COMPRESS_FL_NEW_KEY) < 0)
    return -1;

  k = pstrdup(session.pool, "SFTP_CLIENT_CIPHER_ALGO");
  v = pstrdup(session.pool, sftp_cipher_get_read_algo());
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  k = pstrdup(session.pool, "SFTP_SERVER_CIPHER_ALGO");
  v = pstrdup(session.pool, sftp_cipher_get_write_algo());
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  k = pstrdup(session.pool, "SFTP_CLIENT_MAC_ALGO");
  v = pstrdup(session.pool, sftp_mac_get_read_algo());
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  k = pstrdup(session.pool, "SFTP_SERVER_MAC_ALGO");
  v = pstrdup(session.pool, sftp_mac_get_write_algo());
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  k = pstrdup(session.pool, "SFTP_CLIENT_COMPRESSION_ALGO");
  v = pstrdup(session.pool, sftp_compress_get_read_algo());
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  k = pstrdup(session.pool, "SFTP_SERVER_COMPRESSION_ALGO");
  v = pstrdup(session.pool, sftp_compress_get_write_algo());
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  k = pstrdup(session.pool, "SFTP_KEX_ALGO");
  v = pstrdup(session.pool, kex->session_names->kex_algo);
  pr_env_unset(session.pool, k);
  pr_env_set(session.pool, k, v);

  if (kex_rekey_interval > 0 &&
      kex_rekey_timerno == -1) {
    /* Register the rekey timer. */
    kex_rekey_timerno = pr_timer_add(kex_rekey_interval, -1,
      &sftp_module, kex_rekey_timer_cb, "SFTP KEX Rekey timer");
  }

  if (kex_rekey_timeout > 0) {
    pr_timer_remove(kex_rekey_timeout_timerno, &sftp_module);
    kex_rekey_timeout_timerno = -1;
  }

  sftp_ssh2_packet_rekey_reset();
  kex_rekey_kex = NULL;

  /* If any CBC mode ciphers have been negotiated for the server-to-client
   * stream, then we need to use the 'rogaway' TAP policy.
   */
  k = sftp_cipher_get_write_algo();
  if (strncmp(k + strlen(k) - 4, "-cbc", 4) == 0) {
    const char *policy = "rogaway";

    pr_trace_msg("ssh2", 4, "CBC mode cipher chosen for server-to-client "
      "messages, automatically enabling '%s' TAP policy", policy);

    if (sftp_tap_set_policy(policy) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting TrafficPolicy '%s': %s", policy, strerror(errno));
    }
  }
 
  return 0;
}

static int write_dh_reply(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  const unsigned char *h;
  const char *hostkey_data, *hsig;
  char *buf, *ptr;
  uint32_t bufsz, buflen, hlen = 0;
  size_t dhlen, hostkey_datalen, hsiglen;
  BIGNUM *k = NULL;
  int res;

  /* Compute the shared secret */
  dhlen = DH_size(kex->dh);
  buf = palloc(kex_pool, dhlen);

  pr_trace_msg(trace_channel, 12, "computing DH key");
  res = DH_compute_key((unsigned char *) buf, kex->e, kex->dh);
  if (res < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error computing DH shared secret: %s", sftp_crypto_get_errors());
    return -1;
  }

  k = BN_new();
  if (BN_bin2bn((unsigned char *) buf, res, k) == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error converting DH shared secret to BN: %s", sftp_crypto_get_errors());

    pr_memscrub(buf, res);
    return -1;
  }

  pr_memscrub(buf, res);
  kex->k = k;

  /* Get the hostkey data; it will be part of the data we hash in order
   * to create the session key.
   */
  hostkey_data = sftp_keys_get_hostkey_data(pkt->pool, kex->use_hostkey_type,
    &hostkey_datalen);
  if (hostkey_data == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error converting hostkey for signing");

    BN_clear_free(kex->k);
    kex->k = NULL;
    return -1;
  }

  /* Calculate H */
  h = calculate_h(kex, hostkey_data, hostkey_datalen, k, &hlen);
  if (h == NULL) {
    pr_memscrub((char *) hostkey_data, hostkey_datalen);
    BN_clear_free(kex->k);
    kex->k = NULL;
    return -1;
  }

  kex->h = palloc(pkt->pool, hlen);
  kex->hlen = hlen;
  memcpy((char *) kex->h, h, kex->hlen);

  /* Save H as the session ID */
  sftp_session_set_id(h, hlen);

  /* Sign H with our hostkey */
  hsig = sftp_keys_sign_data(pkt->pool, kex->use_hostkey_type, h, hlen,
    &hsiglen);
  if (hsig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "error signing H");
    pr_memscrub((char *) hostkey_data, hostkey_datalen);
    BN_clear_free(kex->k);
    kex->k = NULL;
    return -1;
  }

  /* XXX Is this large enough?  Too large? */
  buflen = bufsz = 4096;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEX_DH_REPLY);
  sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->pub_key);
  sftp_msg_write_data(&buf, &buflen, hsig, hsiglen, TRUE);

  /* Scrub any sensitive data when done */
  pr_memscrub((char *) hostkey_data, hostkey_datalen);
  pr_memscrub((char *) hsig, hsiglen);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  return 0;
}

static int write_newkeys_reply(struct ssh2_packet *pkt) {
  char *buf, *ptr;
  uint32_t bufsz, buflen;

  /* Write out the NEWKEYS message. */
  bufsz = buflen = 1;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_NEWKEYS);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  return 0;
}

static int handle_kex_dh(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  int res;
  cmd_rec *cmd;

  cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "DH_INIT"));
  cmd->arg = "(data)";
  cmd->class = CL_AUTH;

  pr_trace_msg(trace_channel, 9, "reading DH_INIT message from client");

  res = read_dh_init(pkt, kex);
  if (res < 0) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  destroy_pool(pkt->pool);

  pr_trace_msg(trace_channel, 9, "writing DH_INIT message to client");

  /* Send our key exchange reply. */
  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_dh_reply(pkt, kex);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  destroy_pool(pkt->pool);
  return 0;
}

static int read_dh_gex(struct ssh2_packet *pkt, uint32_t *min, uint32_t *pref,
    uint32_t *max, int old_request) {
  char *buf;
  uint32_t buflen;

  buf = pkt->payload;
  buflen = pkt->payload_len;

  if (!old_request) {
    *min = sftp_msg_read_int(pkt->pool, &buf, &buflen);
    if (*min < SFTP_KEX_DH_GROUP_MIN) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "DH_GEX_REQUEST min value (%lu) too small (< %lu)",
        (unsigned long) *min, (unsigned long) SFTP_KEX_DH_GROUP_MIN);
      return -1;
    }

    *pref = sftp_msg_read_int(pkt->pool, &buf, &buflen);

    *max = sftp_msg_read_int(pkt->pool, &buf, &buflen);
    if (*max > SFTP_KEX_DH_GROUP_MAX) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "DH_GEX_REQUEST max value (%lu) too large (> %lu)",
        (unsigned long) *max, (unsigned long) SFTP_KEX_DH_GROUP_MAX);
      return -1;
    }

  } else {
    *min = SFTP_KEX_DH_GROUP_MIN;
    *pref = sftp_msg_read_int(pkt->pool, &buf, &buflen);
    *max = SFTP_KEX_DH_GROUP_MAX;
  }

  if (*max < *min ||
      *pref < *min ||
      *pref > *max) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "bad DH_GEX_REQUEST parameters: min = %lu, pref = %lu, max = %lu",
      (unsigned long) *min, (unsigned long) *pref, (unsigned long) *max);
    return -1;
  }

  return 0;
}

static int get_dh_gex_group(struct sftp_kex *kex, uint32_t min,
    uint32_t pref, uint32_t max) {
  const char *dhparam_path;
  config_rec *c;
  int use_fixed_modulus = FALSE;

  dhparam_path = PR_CONFIG_DIR "/dhparams.pem";
  c = find_config(main_server->conf, CONF_PARAM, "SFTPDHParamFile", FALSE);
  if (c) {
    dhparam_path = c->argv[0];
  }

  if (dhparam_path) {
    if (kex_dhparams_fp != NULL) {
      /* Rewind to the start of the file. */
      fseek(kex_dhparams_fp, 0, SEEK_SET);

    } else {
      kex_dhparams_fp = fopen(dhparam_path, "r");
    }

    if (kex_dhparams_fp) {
      register unsigned int i;
      pool *tmp_pool;
      array_header *smaller_dhs, *pref_dhs, *larger_dhs;
      DH *dh, **dhs;
      int smaller_dh_nbits = 0, larger_dh_nbits = 0;

      pr_trace_msg(trace_channel, 15,
        "using DH parameters from SFTPDHParamFile '%s' for group exchange",
        dhparam_path);

      tmp_pool = make_sub_pool(kex_pool);
      pr_pool_tag(tmp_pool, "Kex DHparams selection pool");

      smaller_dhs = make_array(tmp_pool, 1, sizeof(DH *)); 
      pref_dhs = make_array(tmp_pool, 1, sizeof(DH *)); 
      larger_dhs = make_array(tmp_pool, 1, sizeof(DH *)); 

      /* From Section 3 of RFC4419:
       *
       *  "The server should return the smallest group it knows that is larger
       *   than the size the client requested.  If the server does not know a
       *   group that is larger than the client request, then it SHOULD return
       *   the largest group it knows.  In all cases, the size of the returned
       *   group SHOULD be at least 1024 bits."
       *
       * Make lists of DHs in the param file whose size falls within the
       * bit lengths requested by the client.  Note that DH_size() returns
       * the sizes _in bytes_, not bits.  We have three lists: one for DHs
       * which match the client-requested preferred size, one for DHs which
       * are smaller than the preferred size, and one for DHs which are larger
       * than the preferred size.  DHs in these last two lists will be of the
       * same size.  Once the lists are populated, we will randomly choose one
       * from the preferred DH list (if available), else one from the larger
       * DH list (if available), else one from the smaller DH list.
       */

      while (TRUE) {
        int nbits;

        pr_signals_handle();

        dh = PEM_read_DHparams(kex_dhparams_fp, NULL, NULL, NULL);
        if (dh == NULL) {
          if (!feof(kex_dhparams_fp)) {
            pr_trace_msg(trace_channel, 5, "error reading DH params from "
              "SFTPDHParamFile '%s': %s", dhparam_path,
              sftp_crypto_get_errors());
          }

          break;
        }

        nbits = DH_size(dh) * 8;

        if (nbits < min ||
            nbits > max) {
          DH_free(dh);
          continue;
        }

        if (nbits == pref) {
          *((DH **) push_array(pref_dhs)) = dh;

        } else if (nbits < pref) {
          if (nbits > smaller_dh_nbits) {
            if (smaller_dhs->nelts > 0) {
              dhs = smaller_dhs->elts;
              for (i = 0; i < smaller_dhs->nelts; i++) {
                DH_free(dhs[i]);
              }

              clear_array(smaller_dhs);
            }

            smaller_dh_nbits = nbits;
            *((DH **) push_array(smaller_dhs)) = dh;

          } else if (nbits == smaller_dh_nbits) {
            *((DH **) push_array(smaller_dhs)) = dh;
          }

        } else {
          /* By process of elimination, nbits here MUST be > pref. */

          if (nbits < larger_dh_nbits) {
            if (larger_dhs->nelts > 0) {
              dhs = larger_dhs->elts;
              for (i = 0; i < larger_dhs->nelts; i++) {
                DH_free(dhs[i]);
              }

              clear_array(larger_dhs);
            }

            larger_dh_nbits = nbits;
            *((DH **) push_array(larger_dhs)) = dh;

          } else if (nbits == larger_dh_nbits) {
            *((DH **) push_array(larger_dhs)) = dh;
          }
        }
      }

      dh = NULL;

      /* The use of rand(3) below is NOT intended to be perfect, or even
       * uniformly distributed.  It simply needs to be good enough to pick
       * a single item from a small list, where all items are equally
       * usable and valid.
       */

      if (pref_dhs->nelts > 0) {
        int r = (int) (rand() / (RAND_MAX / pref_dhs->nelts + 1));

        dhs = pref_dhs->elts;
        dh = dhs[r];

      } else if (larger_dhs->nelts > 0) {
        int r = (int) (rand() / (RAND_MAX / larger_dhs->nelts + 1));

        dhs = larger_dhs->elts;
        dh = dhs[r];

      } else if (smaller_dhs->nelts > 0) {
        int r = (int) (rand() / (RAND_MAX / smaller_dhs->nelts + 1));

        dhs = smaller_dhs->elts;
        dh = dhs[r];

      } else {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unable to find suitable DH in SFTPDHParamFile '%s' for %lu-%lu "
          "bit sizes", dhparam_path, (unsigned long) min, (unsigned long) max);
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "WARNING: using fixed modulus for DH group exchange");
        use_fixed_modulus = TRUE;
      }

      if (dh) {
        pr_trace_msg(trace_channel, 20, "client requested min %lu, pref %lu, "
          "max %lu sizes for DH group exchange, selected DH of %lu bits",
          (unsigned long) min, (unsigned long) pref, (unsigned long) max,
          (unsigned long) DH_size(dh) * 8);

        kex->dh->p = BN_dup(dh->p);
        if (kex->dh->p == NULL) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error copying selected DH P: %s", sftp_crypto_get_errors());
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "WARNING: using fixed modulus for DH group exchange");
          use_fixed_modulus = TRUE;

        } else {
          kex->dh->g = BN_dup(dh->g);
          if (kex->dh->g == NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "error copying selected DH G: %s", sftp_crypto_get_errors());
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "WARNING: using fixed modulus for DH group exchange");

            BN_clear_free(kex->dh->p);
            kex->dh->p = NULL;
            use_fixed_modulus = TRUE;
          }
        }
      }

      /* Don't forget to clean up all of the allocated DHs. */
      dhs = (DH **) smaller_dhs->elts;
      for (i = 0; i < smaller_dhs->nelts; i++) {
        pr_signals_handle();
        DH_free(dhs[i]);
      }

      dhs = (DH **) pref_dhs->elts;
      for (i = 0; i < pref_dhs->nelts; i++) {
        pr_signals_handle();
        DH_free(dhs[i]);
      }

      dhs = (DH **) larger_dhs->elts;
      for (i = 0; i < larger_dhs->nelts; i++) {
        pr_signals_handle();
        DH_free(dhs[i]);
      }

      destroy_pool(tmp_pool);

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "WARNING: unable to read SFTPDHParamFile '%s': %s", dhparam_path,
        strerror(errno));
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "WARNING: using fixed modulus for DH group exchange");
      use_fixed_modulus = TRUE;
    }
  }

  if (use_fixed_modulus) {
    kex->dh->p = BN_new();
    kex->dh->g = BN_new();

    if (BN_hex2bn(&kex->dh->p, dh_group14_str) == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting DH P: %s", sftp_crypto_get_errors());
      BN_clear_free(kex->dh->p);
      kex->dh->p = NULL;

      errno = EACCES;
      return -1;
    }

    if (BN_hex2bn(&kex->dh->g, "2") == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting DH G: %s", sftp_crypto_get_errors());
      BN_clear_free(kex->dh->g);
      kex->dh->g = NULL;

      errno = EACCES;
      return -1;
    }
  }

  return 0;
}

static int write_dh_gex_group(struct ssh2_packet *pkt, struct sftp_kex *kex,
    uint32_t min, uint32_t pref, uint32_t max) {
  char *buf, *ptr;
  uint32_t buflen, bufsz;

  if (get_dh_gex_group(kex, min, pref, max) < 0) {
    return -1;
  }

  /* XXX Is this large enough?  Too large? */
  buflen = bufsz = 1024;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEX_DH_GEX_GROUP);
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->p);
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->g);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  return 0;
}

static int read_dh_gex_init(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  char *buf;
  uint32_t buflen;

  buf = pkt->payload;
  buflen = pkt->payload_len;

  /* Read in 'e' */
  kex->e = sftp_msg_read_mpint(pkt->pool, &buf, &buflen);
  if (kex->e == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading DH_GEX_INIT: %s", strerror(errno));
    return -1;
  }

  return 0;
}

static int write_dh_gex_reply(struct ssh2_packet *pkt, struct sftp_kex *kex,
    uint32_t min, uint32_t pref, uint32_t max, int old_request) {
  const unsigned char *h;
  const char *hostkey_data, *hsig;
  char *buf, *ptr;
  uint32_t bufsz, buflen, hlen = 0;
  size_t dhlen, hostkey_datalen, hsiglen;
  BIGNUM *k = NULL;
  int res;

  /* Compute the shared secret. */
  dhlen = DH_size(kex->dh);
  buf = palloc(kex_pool, dhlen);

  pr_trace_msg(trace_channel, 12, "computing DH key");
  res = DH_compute_key((unsigned char *) buf, kex->e, kex->dh);
  if (res < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error computing DH shared secret: %s", sftp_crypto_get_errors());
    return -1;
  }

  k = BN_new();
  if (BN_bin2bn((unsigned char *) buf, res, k) == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error converting DH shared secret to BN: %s", sftp_crypto_get_errors());

    pr_memscrub(buf, res);
    return -1;
  }

  pr_memscrub(buf, res);
  kex->k = k;

  /* Get the hostkey data; it will be part of the data we hash in order
   * to create the session key.
   */
  hostkey_data = sftp_keys_get_hostkey_data(pkt->pool, kex->use_hostkey_type,
    &hostkey_datalen);
  if (hostkey_data == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error converting hostkey for signing");

    BN_clear_free(kex->k);
    kex->k = NULL;
    return -1;
  }

  if (old_request) {
    max = min = 0;
  }

  /* Calculate H */
  h = calculate_gex_h(kex, hostkey_data, hostkey_datalen, k, min, pref, max,
    &hlen);
  if (h == NULL) {
    pr_memscrub((char *) hostkey_data, hostkey_datalen);
    BN_clear_free(kex->k);
    kex->k = NULL;
    return -1;
  } 

  kex->h = palloc(pkt->pool, hlen);
  kex->hlen = hlen;
  memcpy((char *) kex->h, h, kex->hlen);

  /* Save H as the session ID */
  sftp_session_set_id(h, hlen);

  /* Sign H with our hostkey */
  hsig = sftp_keys_sign_data(pkt->pool, kex->use_hostkey_type, h, hlen,
    &hsiglen);
  if (hsig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "error signing H");
    pr_memscrub((char *) hostkey_data, hostkey_datalen);
    BN_clear_free(kex->k);
    kex->k = NULL;
    return -1;
  }

  /* XXX Is this large enough?  Too large? */
  buflen = bufsz = 8192;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEX_DH_GEX_REPLY);
  sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
  sftp_msg_write_mpint(&buf, &buflen, kex->dh->pub_key);
  sftp_msg_write_data(&buf, &buflen, hsig, hsiglen, TRUE);

  /* Scrub any sensitive data when done */
  pr_memscrub((char *) hostkey_data, hostkey_datalen);
  pr_memscrub((char *) hsig, hsiglen);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  return 0;
}

static int handle_kex_dh_gex(struct ssh2_packet *pkt, struct sftp_kex *kex,
    int old_request) {
  char mesg_type;
  int res;
  uint32_t min = 0, pref = 0, max = 0;
  cmd_rec *cmd;

  if (!old_request) {
    pr_trace_msg(trace_channel, 9,
      "reading DH_GEX_REQUEST message from client");

    cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "DH_GEX_REQUEST"));
    cmd->arg = "(data)";
    cmd->class = CL_AUTH;

  } else {
    pr_trace_msg(trace_channel, 9,
      "reading DH_GEX_REQUEST_OLD message from client");

    cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "DH_GEX_REQUEST_OLD"));
    cmd->arg = "(data)";
    cmd->class = CL_AUTH;
  }

  res = read_dh_gex(pkt, &min, &pref, &max, old_request);
  if (res < 0) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  destroy_pool(pkt->pool);

  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_dh_gex_group(pkt, kex, min, pref, max);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_trace_msg(trace_channel, 9, "writing DH_GEX_GROUP message to client");

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  destroy_pool(pkt->pool);

  pkt = sftp_ssh2_packet_create(kex_pool);
  res = sftp_ssh2_packet_read(sftp_conn->rfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    return -1;
  }

  mesg_type = sftp_ssh2_packet_get_mesg_type(pkt);
  if (mesg_type != SFTP_SSH2_MSG_KEX_DH_GEX_INIT) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "expecting KEX_DH_GEX_INIT message, received %s (%d), disconnecting",
      sftp_ssh2_packet_get_mesg_type_desc(mesg_type), mesg_type);
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "DH_GEX_INIT"));
  cmd->arg = "(data)";

  pr_trace_msg(trace_channel, 9, "reading DH_GEX_INIT message from client");

  res = read_dh_gex_init(pkt, kex);
  if (res < 0) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    destroy_pool(pkt->pool);
    return -1;
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  destroy_pool(pkt->pool);

  if (finish_dh(kex) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error finishing DH key for group exchange: %s", strerror(errno));
    return -1;
  }

  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_dh_gex_reply(pkt, kex, min, pref, max, old_request);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_trace_msg(trace_channel, 9, "writing DH_GEX_REPLY message to client");

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  destroy_pool(pkt->pool);
  return 0;
}

static int read_kexrsa_secret(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  char *buf;
  unsigned char *encrypted, *decrypted;
  uint32_t buflen, encrypted_len;
  BIGNUM *k = NULL;
  int res;

  buf = pkt->payload;
  buflen = pkt->payload_len;

  encrypted_len = sftp_msg_read_int(pkt->pool, &buf, &buflen);
  encrypted = (unsigned char *) sftp_msg_read_data(pkt->pool, &buf, &buflen,
    encrypted_len);

  /* Save the encrypted secret for calculating H. */
  kex->rsa_encrypted_len = encrypted_len;
  kex->rsa_encrypted = palloc(kex_pool, encrypted_len);
  memcpy(kex->rsa_encrypted, encrypted, encrypted_len);

  pr_trace_msg(trace_channel, 12, "decrypting RSA shared secret");

  decrypted = palloc(pkt->pool, RSA_size(kex->rsa));

  res = RSA_private_decrypt((int) encrypted_len, encrypted, decrypted,
    kex->rsa, RSA_PKCS1_OAEP_PADDING);
  if (res == -1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error decrypting RSA shared secret: %s", sftp_crypto_get_errors());
    RSA_free(kex->rsa);
    kex->rsa = NULL;
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  /* When converting the decrypted secret into an mpint, watch out for any
   * leading padding (as required per SSH RFCs).
   */

  k = BN_new();
  if (BN_mpi2bn(decrypted, res, k) == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error converting RSA shared secret to BN: %s", sftp_crypto_get_errors());

    pr_memscrub(decrypted, res);
    RSA_free(kex->rsa);
    kex->rsa = NULL;
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_memscrub(decrypted, res);

  kex->k = k;
  return 0;
}

static int write_kexrsa_pubkey(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  char *buf, *ptr, *buf2, *ptr2;
  const char *hostkey_data;
  uint32_t buflen, bufsz, buflen2, bufsz2, hostkey_datalen;

  hostkey_data = sftp_keys_get_hostkey_data(pkt->pool, kex->use_hostkey_type,
    (size_t *) &hostkey_datalen);
  if (hostkey_data == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining hostkey for KEXRSA key exchange");
    RSA_free(kex->rsa);
    kex->rsa = NULL;
    return -1;
  }

  /* XXX Is this buffer large enough?  Too large? */
  bufsz = buflen = 512;
  ptr = buf = palloc(kex_pool, bufsz);

  /* Write the transient RSA public key into its own buffer, to then be
   * written in its entirety as an SSH2 string.
   */
  sftp_msg_write_string(&buf, &buflen, "ssh-rsa");
  sftp_msg_write_mpint(&buf, &buflen, kex->rsa->e);
  sftp_msg_write_mpint(&buf, &buflen, kex->rsa->n);

  /* XXX Is this buffer large enough?  Too large? */
  bufsz2 = buflen2 = 1024;
  ptr2 = buf2 = palloc(pkt->pool, bufsz2);

  sftp_msg_write_byte(&buf2, &buflen2, SFTP_SSH2_MSG_KEXRSA_PUBKEY);
  sftp_msg_write_data(&buf2, &buflen2, hostkey_data, hostkey_datalen, TRUE);
  sftp_msg_write_data(&buf2, &buflen2, ptr, (bufsz - buflen), TRUE);

  pr_memscrub((char *) hostkey_data, hostkey_datalen);

  pkt->payload = ptr2;
  pkt->payload_len = (bufsz2 - buflen2);

  return 0;
}

static int write_kexrsa_done(struct ssh2_packet *pkt, struct sftp_kex *kex) {
  char *buf, *ptr, *buf2, *ptr2;
  const char *hostkey_data, *hsig;
  const unsigned char *h;
  uint32_t buflen, bufsz, buflen2, bufsz2, hlen;
  size_t hostkey_datalen, hsiglen;

  hostkey_data = sftp_keys_get_hostkey_data(pkt->pool, kex->use_hostkey_type,
    &hostkey_datalen);
  if (hostkey_data == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining hostkey for KEXRSA key exchange");

    RSA_free(kex->rsa);
    kex->rsa = NULL;
    BN_clear_free(kex->k);
    kex->k = NULL;
    pr_memscrub(kex->rsa_encrypted, kex->rsa_encrypted_len);
    kex->rsa_encrypted = NULL;
    kex->rsa_encrypted_len = 0;

    return -1;
  }

  /* XXX Is this buffer large enough?  Too large? */
  bufsz2 = buflen2 = 512;
  ptr2 = buf2 = palloc(kex_pool, bufsz2);

  /* Write the transient RSA public key into its own buffer, to then be
   * written in its entirety as an SSH2 string.
   */
  sftp_msg_write_string(&buf2, &buflen2, "ssh-rsa");
  sftp_msg_write_mpint(&buf2, &buflen2, kex->rsa->e);
  sftp_msg_write_mpint(&buf2, &buflen2, kex->rsa->n);

  /* Calculate H */
  h = calculate_kexrsa_h(kex, hostkey_data, hostkey_datalen, kex->k,
    ptr2, (bufsz2 - buflen2), &hlen);
  if (h == NULL) {
    pr_memscrub((char *) hostkey_data, hostkey_datalen);
    RSA_free(kex->rsa);
    kex->rsa = NULL;
    BN_clear_free(kex->k);
    kex->k = NULL;
    pr_memscrub(kex->rsa_encrypted, kex->rsa_encrypted_len);
    kex->rsa_encrypted = NULL;
    kex->rsa_encrypted_len = 0;

    return -1;
  } 

  kex->h = palloc(pkt->pool, hlen);
  kex->hlen = hlen;
  memcpy((char *) kex->h, h, kex->hlen);

  /* Save H as the session ID */
  sftp_session_set_id(h, hlen);

  /* Sign H with our host key */
  hsig = sftp_keys_sign_data(pkt->pool, kex->use_hostkey_type, h, hlen,
    &hsiglen);
  if (hsig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "error signing H");

    pr_memscrub((char *) hostkey_data, hostkey_datalen);
    RSA_free(kex->rsa);
    kex->rsa = NULL;
    BN_clear_free(kex->k);
    kex->k = NULL;
    pr_memscrub(kex->rsa_encrypted, kex->rsa_encrypted_len);
    kex->rsa_encrypted = NULL;
    kex->rsa_encrypted_len = 0;

    return -1;
  }

  /* XXX Is this buffer large enough?  Too large? */
  bufsz = buflen = 2048;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_KEXRSA_DONE);
  sftp_msg_write_data(&buf, &buflen, hsig, hsiglen, TRUE);

  pr_memscrub((char *) hostkey_data, hostkey_datalen);
  pr_memscrub((char *) h, hlen);
  pr_memscrub((char *) hsig, hsiglen);

  pr_memscrub(kex->rsa_encrypted, kex->rsa_encrypted_len);
  kex->rsa_encrypted = NULL;
  kex->rsa_encrypted_len = 0;

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  return 0;
}

static int handle_kex_rsa(struct sftp_kex *kex) {
  char mesg_type;
  struct ssh2_packet *pkt;
  int res;
  cmd_rec *cmd;

  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_kexrsa_pubkey(pkt, kex);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_trace_msg(trace_channel, 9, "writing KEXRSA_PUBKEY message to client");

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  destroy_pool(pkt->pool);

  pkt = sftp_ssh2_packet_create(kex_pool);
  res = sftp_ssh2_packet_read(sftp_conn->rfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    return -1;
  }

  mesg_type = sftp_ssh2_packet_get_mesg_type(pkt);
  if (mesg_type != SFTP_SSH2_MSG_KEXRSA_SECRET) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "expecting KEXRSA_SECRET message, received %s (%d), disconnecting",
      sftp_ssh2_packet_get_mesg_type_desc(mesg_type), mesg_type);
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "KEXRSA_SECRET"));
  cmd->arg = "(data)";
  cmd->class = CL_AUTH;

  pr_trace_msg(trace_channel, 9, "reading KEXRSA_SECRET message from client");

  res = read_kexrsa_secret(pkt, kex);
  if (res < 0) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  destroy_pool(pkt->pool);

  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_kexrsa_done(pkt, kex);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  pr_trace_msg(trace_channel, 9, "writing KEXRSA_DONE message to client");

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  destroy_pool(pkt->pool);
  return 0;
}

int sftp_kex_handle(struct ssh2_packet *pkt) {
  int correct_guess = TRUE, res;
  char mesg_type;
  struct sftp_kex *kex;
  cmd_rec *cmd;

  /* We may already have a kex structure, either from the client
   * initial connect (kex_first_kex not null), or because we
   * are in a server-initiated rekeying (kex_rekey_kex not null).
   */
  if (kex_first_kex) {
    kex = kex_first_kex;

    /* We need to assign the client/server versions, which this struct
     * will not have.
     */
    kex->client_version = kex_client_version;
    kex->server_version = kex_server_version;

  } else if (kex_rekey_kex) {
    kex = kex_rekey_kex;

  } else {
    kex = create_kex(kex_pool);
  }

  /* The packet we are given is guaranteed to be a KEXINIT packet. */

  cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "KEXINIT"));
  cmd->arg = "(data)";
  cmd->class = CL_AUTH;

  pr_trace_msg(trace_channel, 9, "reading KEXINIT message from client");

  res = read_kexinit(pkt, kex);
  if (res < 0) {
    pr_cmd_dispatch_phase(cmd, LOG_CMD_ERR, 0);

    destroy_kex(kex);
    destroy_pool(pkt->pool);
    return -1;
  }

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);
  destroy_pool(pkt->pool);

  pr_trace_msg(trace_channel, 9,
    "determining shared algorithms for SSH session");

  if (get_session_names(kex, &correct_guess) < 0) {
    destroy_kex(kex);
    return -1;
  }

  /* Once we have received the client KEXINIT message, we can compare what we
   * want to send against what we already received from the client.
   *
   * If the client said that it was going to send a "guess" KEX packet,
   * and we determine that its guess matches what we would have
   * sent in our KEXINIT, then we don't need to send our KEXINIT.
   * We can simply proceed on with reading and handling that packet.
   * Otherwise, we will send our KEXINIT and proceed.
   */

  if (!kex->first_kex_follows) {
    /* No guess packet sent; send our KEXINIT as normal (as long as we are
     * not in a server-initiated rekeying).
     */

    if (!kex_sent_kexinit) {
      pkt = sftp_ssh2_packet_create(kex_pool);
      res = write_kexinit(pkt, kex);
      if (res < 0) {
        destroy_kex(kex);
        destroy_pool(pkt->pool);
        return -1;
      }

      pr_trace_msg(trace_channel, 9, "sending KEXINIT message to client");

      res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
      if (res < 0) {
        destroy_kex(kex);
        destroy_pool(pkt->pool);
        return res;
      }

      kex_sent_kexinit = TRUE;
      destroy_pool(pkt->pool);
    }

  } else {

    /* If the client sent a guess kex packet, but that guess was incorrect,
     * then we need to consume and silently ignore that packet, and proceed
     * to send our own KEXINIT (if needed).
     */
    if (!correct_guess) {
      pr_trace_msg(trace_channel, 3, "client sent incorrect KEXINIT guess, "
        "ignoring guess packet");

      pkt = sftp_ssh2_packet_create(kex_pool);
      if (sftp_ssh2_packet_read(sftp_conn->rfd, pkt) < 0) {
        destroy_kex(kex);
        destroy_pool(pkt->pool);
        return -1;
      }

      destroy_pool(pkt->pool);

      if (!kex_sent_kexinit) {
        pkt = sftp_ssh2_packet_create(kex_pool);
        res = write_kexinit(pkt, kex);
        if (res < 0) {
          destroy_kex(kex);
          destroy_pool(pkt->pool);
          return -1;
        }

        pr_trace_msg(trace_channel, 9, "sending KEXINIT message to client");

        res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
        if (res < 0) {
          destroy_kex(kex);
          destroy_pool(pkt->pool);
          return res;
        }

        kex_sent_kexinit = TRUE;
        destroy_pool(pkt->pool);
      }

    } else {
      pr_trace_msg(trace_channel, 3, "client sent correct KEXINIT guess, "
        "no need to send our KEXINIT");
    }
  }

  if (!kex->use_kexrsa) {
    /* Read the client key exchange mesg. */
    pkt = sftp_ssh2_packet_create(kex_pool);
    res = sftp_ssh2_packet_read(sftp_conn->rfd, pkt);
    if (res < 0) {
      destroy_kex(kex);
      destroy_pool(pkt->pool);
      return -1;
    }

    mesg_type = sftp_ssh2_packet_get_mesg_type(pkt);
    switch (mesg_type) {
      case SFTP_SSH2_MSG_KEX_DH_INIT:
        /* This handles the case of SFTP_SSH2_MSG_KEX_DH_GEX_REQUEST_OLD as
         * well; that ID has the same value as the KEX_DH_INIT ID.
         */
        if (kex->use_gex) {
          res = handle_kex_dh_gex(pkt, kex, TRUE);

        } else {
          res = handle_kex_dh(pkt, kex);
        }

        if (res < 0) {
          destroy_kex(kex);
          destroy_pool(pkt->pool);
          return -1;
        }
        break;

      case SFTP_SSH2_MSG_KEX_DH_GEX_REQUEST:
        res = handle_kex_dh_gex(pkt, kex, FALSE);
        if (res < 0) {
          destroy_kex(kex);
          destroy_pool(pkt->pool);
          return -1;
        }
        break;

      default:
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "expecting KEX_DH_INIT or KEX_DH_GEX_GROUP message, "
          "received %s (%d), disconnecting",
          sftp_ssh2_packet_get_mesg_type_desc(mesg_type), mesg_type);
        destroy_kex(kex);
        destroy_pool(pkt->pool);
        SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_PROTOCOL_ERROR, NULL);
    }

  } else {
    res = handle_kex_rsa(kex);
    if (res < 0) {
      destroy_kex(kex);
      return -1;
    }
  }

  pr_trace_msg(trace_channel, 9, "reading NEWKEYS message from client");

  /* Read the client NEWKEYS mesg. */
  pkt = sftp_ssh2_packet_create(kex_pool);

  res = sftp_ssh2_packet_read(sftp_conn->rfd, pkt);
  if (res < 0) {
    destroy_kex(kex);
    destroy_pool(pkt->pool);
    return -1;
  }

  mesg_type = sftp_ssh2_packet_get_mesg_type(pkt);
  if (mesg_type != SFTP_SSH2_MSG_NEWKEYS) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "expecting NEWKEYS message, received %s (%d), disconnecting",
      sftp_ssh2_packet_get_mesg_type_desc(mesg_type), mesg_type);
    destroy_kex(kex);
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_PROTOCOL_ERROR, NULL);
  }

  cmd = pr_cmd_alloc(pkt->pool, 1, pstrdup(pkt->pool, "NEWKEYS"));
  cmd->arg = "";
  cmd->class = CL_AUTH;

  pr_cmd_dispatch_phase(cmd, LOG_CMD, 0);

  pr_trace_msg(trace_channel, 9, "sending NEWKEYS message to client");

  /* Send our NEWKEYS reply. */
  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_newkeys_reply(pkt);
  if (res < 0) {
    destroy_kex(kex);
    destroy_pool(pkt->pool);
    return -1;
  }

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_kex(kex);
    destroy_pool(pkt->pool);
    return -1;
  }

  destroy_pool(pkt->pool);

  /* Last but certainly not least, set up the keys for encryption and
   * authentication, based on H and K.
   */
  pr_trace_msg(trace_channel, 9, "setting session keys");
  if (set_session_keys(kex) < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error setting session keys, disconnecting");
    destroy_kex(kex);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_BY_APPLICATION, NULL);
  }

  /* Reset this flag for the next time through. */
  kex_sent_kexinit = FALSE;

  destroy_kex(kex);
  return 0;
}

int sftp_kex_free(void) {
  if (kex_dhparams_fp != NULL) {
    (void) fclose(kex_dhparams_fp);
    kex_dhparams_fp = NULL;
  }

  if (kex_pool) {
    destroy_pool(kex_pool);
    kex_pool = NULL;
  }

  return 0;
}

int sftp_kex_init(const char *client_version, const char *server_version) {
  /* If we are called with client_version and server_version both NULL,
   * then we're setting up for a rekey.  We can destroy/create the Kex
   * pool in that case.  But not otherwise.
   */
  if (client_version == NULL &&
      server_version == NULL) {
    if (kex_pool) {
      destroy_pool(kex_pool);
      kex_pool = NULL;
    }
  }

  if (!kex_pool) {
    kex_pool = make_sub_pool(sftp_pool);
    pr_pool_tag(kex_pool, "Kex Pool");
  }

  /* Save the client and server versions, the first time through.  They
   * will be used for any future rekey KEXINIT exchanges.
   */

  if (client_version != NULL &&
      kex_client_version == NULL) {
    kex_client_version = pstrdup(sftp_pool, client_version);
  }

  if (server_version != NULL &&
      kex_server_version == NULL) {
    kex_server_version = pstrdup(sftp_pool, server_version);
  }

  return 0;
}

int sftp_kex_rekey(void) {
  int res;
  struct ssh2_packet *pkt;

  /* We cannot perform a rekey if we have not even finished the first kex. */ 
  if (!(sftp_sess_state & SFTP_SESS_STATE_HAVE_KEX)) {
    pr_trace_msg(trace_channel, 3,
      "unable to request rekey: Initial KEX not completed");

    /* If this was triggered by a rekey timer, register a new timer and
     * try the rekey request in another 5 seconds.
     */
    if (kex_rekey_interval > 0 &&
        kex_rekey_timerno == -1) {
      pr_trace_msg(trace_channel, 3,
        "trying rekey request in another 15 seconds");
      kex_rekey_timerno = pr_timer_add(15, -1, &sftp_module, kex_rekey_timer_cb,
        "SFTP KEX Rekey timer");
    }

    return 0;
  }

  if (!sftp_interop_supports_feature(SFTP_SSH2_FEAT_REKEYING)) {
    pr_trace_msg(trace_channel, 3,
      "unable to request rekeying: Not supported by client");
    sftp_ssh2_packet_rekey_reset();
    return 0;
  }
 
  /* If already rekeying, return now. */
  if (sftp_sess_state & SFTP_SESS_STATE_REKEYING) {
    pr_trace_msg(trace_channel, 17,
      "rekeying already in effect, ignoring rekey request");
    return 0;
  }

  /* Make sure that any rekey timer will try not to interfere while the
   * rekeying is happening.
   */
  if (kex_rekey_timerno != -1) {
    pr_timer_remove(kex_rekey_timerno, &sftp_module);
    kex_rekey_timerno = -1;
  }

  pr_trace_msg(trace_channel, 17, "sending rekey KEXINIT");

  sftp_sess_state |= SFTP_SESS_STATE_REKEYING;
  sftp_kex_init(NULL, NULL);

  kex_rekey_kex = create_kex(kex_pool);

  pr_trace_msg(trace_channel, 9, "writing KEXINIT message to client");

  /* Sent our KEXINIT mesg. */
  pkt = sftp_ssh2_packet_create(kex_pool);
  res = write_kexinit(pkt, kex_rekey_kex);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_pool(pkt->pool);
    SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_KEY_EXCHANGE_FAILED, NULL);
  }

  destroy_pool(pkt->pool);

  kex_sent_kexinit = TRUE;

  if (kex_rekey_timeout > 0) {
    pr_trace_msg(trace_channel, 17, "client has %d secs to rekey",
      kex_rekey_timeout);
    kex_rekey_timeout_timerno = pr_timer_add(kex_rekey_timeout, -1,
      &sftp_module, kex_rekey_timeout_cb, "SFTP KEX Rekey Timeout timer");
  }

  return 0;
}

int sftp_kex_rekey_set_interval(int rekey_interval) {
  if (rekey_interval < 0) {
    errno = EINVAL;
    return -1;
  }

  kex_rekey_interval = rekey_interval;
  return 0;
}

int sftp_kex_rekey_set_timeout(int timeout) {
  if (timeout < 0) {
    errno = EINVAL;
    return -1;
  }

  kex_rekey_timeout = timeout;
  return 0;
}

int sftp_kex_send_first_kexinit(void) {
  struct ssh2_packet *pkt;
  int res;

  if (!kex_pool) {
    kex_pool = make_sub_pool(sftp_pool);
    pr_pool_tag(kex_pool, "Kex Pool");
  }

  /* The client has just connected to us.  We want to send our version
   * ID string _and_ the KEXINIT in the same TCP packet, and save a 
   * TCP round trip (one TCP ACK for both messages, rather than one ACK
   * per message).  The packet API will automatically send the version
   * ID string along with the first packet we send; we just have to
   * send a packet, and the KEXINIT is the first one in the protocol.
   */
  kex_first_kex = create_kex(kex_pool);

  pkt = sftp_ssh2_packet_create(kex_pool); 
  res = write_kexinit(pkt, kex_first_kex);
  if (res < 0) {
    destroy_kex(kex_first_kex);
    destroy_pool(pkt->pool);
    return -1;
  }

  pr_trace_msg(trace_channel, 9, "sending KEXINIT message to client");

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  if (res < 0) {
    destroy_kex(kex_first_kex);
    destroy_pool(pkt->pool);
    return -1;
  }
  kex_sent_kexinit = TRUE;

  destroy_pool(pkt->pool);
  return 0;
}

