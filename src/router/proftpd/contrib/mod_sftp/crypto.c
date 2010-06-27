/*
 * ProFTPD - mod_sftp OpenSSL interface
 * Copyright (c) 2008-2010 TJ Saunders
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
 * $Id: crypto.c,v 1.14 2010/01/31 20:34:26 castaglia Exp $
 */

#include "mod_sftp.h"
#include "crypto.h"

#if OPENSSL_VERSION_NUMBER > 0x000907000L
static const char *crypto_engine = NULL;
#endif

struct sftp_cipher {
  const char *name;
  const char *openssl_name;

  /* Used mostly for the RC4/ArcFour algorithms, for mitigating attacks
   * based on the first N bytes of the keystream.
   */
  size_t discard_len;

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  const EVP_CIPHER *(*get_type)(void);
#else
  EVP_CIPHER *(*get_type)(void);
#endif

  /* Is this cipher enabled by default?  If FALSE, then this cipher must
   * be explicitly requested via SFTPCiphers.
   */
  int enabled;
};

/* Currently, OpenSSL does NOT support AES CTR modes (not sure why).
 * Until then, we have to provide our own CTR code, for some of the ciphers
 * recommended by RFC4344.
 *
 * And according to:
 *
 *   http://www.cpni.gov.uk/Docs/Vulnerability_Advisory_SSH.txt
 *
 * it is highly recommended to use CTR mode ciphers, rather than CBC mode,
 * in order to avoid leaking plaintext.
 */

static struct sftp_cipher ciphers[] = {
  /* The handling of NULL openssl_name and get_type fields is done in
   * sftp_crypto_get_cipher(), as special cases.
   */

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  { "aes256-ctr",	NULL,		0,	NULL,			TRUE },
  { "aes192-ctr",	NULL,		0,	NULL,			TRUE },
  { "aes128-ctr",	NULL,		0,	NULL,			TRUE },

# ifndef HAVE_AES_CRIPPLED_OPENSSL
  { "aes256-cbc",	"aes-256-cbc",	0,	EVP_aes_256_cbc,	TRUE },
  { "aes192-cbc",	"aes-192-cbc",	0,	EVP_aes_192_cbc,	TRUE },
# endif /* !HAVE_AES_CRIPPLED_OPENSSL */

  { "aes128-cbc",	"aes-128-cbc",	0,	EVP_aes_128_cbc,	TRUE },
#endif

  { "blowfish-ctr",	NULL,		0,	NULL,			TRUE },
  { "blowfish-cbc",	"bf-cbc",	0,	EVP_bf_cbc,		TRUE },
  { "cast128-cbc",	"cast5-cbc",	0,	EVP_cast5_cbc,		TRUE },
  { "arcfour256",	"rc4",		1536,	EVP_rc4,		TRUE },
  { "arcfour128",	"rc4",		1536,	EVP_rc4,		TRUE },

#if 0
  /* This cipher is explicitly NOT supported because it does not discard
   * the first N bytes of the keystream, unlike the other RC4 ciphers.
   *
   * If there is a hue and cry, I might add this to the code BUT it would
   * require explicit configuration via SFTPCiphers, and would generate
   * warnings about its unsafe use.
   */
  { "arcfour",		"rc4",		0,	EVP_rc4,		FALSE },
#endif

  { "3des-ctr",		NULL,		0,	NULL,			TRUE },
  { "3des-cbc",		"des-ede3-cbc",	0,	EVP_des_ede3_cbc,	TRUE },
  { "none",		"null",		0,	EVP_enc_null,		FALSE },
  { NULL, NULL, 0, NULL, FALSE }
};

struct sftp_digest {
  const char *name;
  const char *openssl_name;

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  const EVP_MD *(*get_type)(void);
#else
  EVP_MD *(*get_type)(void);
#endif

  uint32_t mac_len;

  /* Is this MAC enabled by default?  If FALSE, then this MAC must be
   * explicitly requested via SFTPDigests.
   */
  int enabled;
};

static struct sftp_digest digests[] = {
  { "hmac-sha1",	"sha1",		EVP_sha1,	0,	TRUE },
  { "hmac-sha1-96",	"sha1",		EVP_sha1,	12,	TRUE },
  { "hmac-md5",		"md5",		EVP_md5,	0,	TRUE },
  { "hmac-md5-96",	"md5",		EVP_md5,	12,	TRUE },
  { "hmac-ripemd160",	"rmd160",	EVP_ripemd160,	0,	TRUE },
  { "none",		"null",		EVP_md_null,	0,	FALSE },
  { NULL, NULL, NULL, 0, FALSE }
};

static const char *trace_channel = "ssh2";

static void ctr_incr(unsigned char *ctr, size_t len) {
  register unsigned int i;

  for (i = len - 1; i >= 0; i--) {
    /* If we haven't overflowed, we're done. */
    if (++ctr[i]) {
      return;
    }
  }
}

/* Blowfish CTR mode implementation */

struct bf_ctr_ex {
  BF_KEY key;
  unsigned char counter[BF_BLOCK];
};

static int init_bf_ctr(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc) {
  struct bf_ctr_ex *bce;

  bce = EVP_CIPHER_CTX_get_app_data(ctx);
  if (bce == NULL) {

    /* Allocate our data structure. */
    bce = calloc(1, sizeof(struct bf_ctr_ex));
    if (bce == NULL) {
      pr_log_pri(PR_LOG_ERR, MOD_SFTP_VERSION ": Out of memory!");
      _exit(1);
    }

    EVP_CIPHER_CTX_set_app_data(ctx, bce);
  }

  if (key != NULL) {
    int key_len;

# if OPENSSL_VERSION_NUMBER == 0x0090805fL
    /* OpenSSL 0.9.8e had a bug where EVP_CIPHER_CTX_key_length() returned
     * the cipher key length rather than the context key length.
     */
    key_len = ctx->key_len;
# else
    key_len = EVP_CIPHER_CTX_key_length(ctx);
# endif

    BF_set_key(&(bce->key), key_len, key);
  }

  if (iv != NULL) {
    memcpy(bce->counter, iv, BF_BLOCK);
  }

  return 1;
}

static int cleanup_bf_ctr(EVP_CIPHER_CTX *ctx) {
  struct bf_ctr_ex *bce;

  bce = EVP_CIPHER_CTX_get_app_data(ctx);
  if (bce != NULL) {
    pr_memscrub(bce, sizeof(struct bf_ctr_ex));
    free(bce);
    EVP_CIPHER_CTX_set_app_data(ctx, NULL);
  }

  return 1;
}

static int do_bf_ctr(EVP_CIPHER_CTX *ctx, unsigned char *dst,
    const unsigned char *src, unsigned int len) {
  struct bf_ctr_ex *bce;
  unsigned int n;
  unsigned char buf[BF_BLOCK];

  if (len == 0)
    return 1;

  bce = EVP_CIPHER_CTX_get_app_data(ctx);
  if (bce == NULL)
    return 0;

  n = 0;

  while ((len--) > 0) {
    pr_signals_handle();

    if (n == 0) {
      BF_LONG ctr[2];

      /* Ideally, we would not be using htonl/ntohl here, and the following
       * code would be as simple as:
       *
       *  memcpy(buf, bce->counter, BF_BLOCK);
       *  BF_encrypt((BF_LONG *) buf, &(bce->key));
       *
       * However, the above is susceptible to endianness issues.  The only
       * client that I could find which implements the blowfish-ctr cipher,
       * PuTTy, uses its own big-endian Blowfish implementation.  So the
       * above code will work with PuTTy, but only on big-endian machines.
       * For little-endian machines, we need to handle the endianness
       * ourselves.  Whee.
       */

      memcpy(&(ctr[0]), bce->counter, sizeof(BF_LONG));
      memcpy(&(ctr[1]), bce->counter + sizeof(BF_LONG), sizeof(BF_LONG));

      /* Convert to big-endian values before encrypting the counter... */
      ctr[0] = htonl(ctr[0]);
      ctr[1] = htonl(ctr[1]);

      BF_encrypt(ctr, &(bce->key));

      /* ...and convert back to little-endian before XOR'ing the counter in. */
      ctr[0] = ntohl(ctr[0]);
      ctr[1] = ntohl(ctr[1]);

      memcpy(buf, ctr, BF_BLOCK);

      ctr_incr(bce->counter, BF_BLOCK);
    }

    *(dst++) = *(src++) ^ buf[n];
    n = (n + 1) % BF_BLOCK;
  }

  return 1;
}

static const EVP_CIPHER *get_bf_ctr_cipher(void) {
  static EVP_CIPHER bf_ctr_cipher;

  memset(&bf_ctr_cipher, 0, sizeof(EVP_CIPHER));

  bf_ctr_cipher.nid = NID_undef;
  bf_ctr_cipher.block_size = BF_BLOCK;
  bf_ctr_cipher.iv_len = BF_BLOCK;
  bf_ctr_cipher.key_len = 32;
  bf_ctr_cipher.init = init_bf_ctr;
  bf_ctr_cipher.cleanup = cleanup_bf_ctr;
  bf_ctr_cipher.do_cipher = do_bf_ctr;

  bf_ctr_cipher.flags = EVP_CIPH_CBC_MODE|EVP_CIPH_VARIABLE_LENGTH|EVP_CIPH_ALWAYS_CALL_INIT|EVP_CIPH_CUSTOM_IV;

  return &bf_ctr_cipher;
}

/* 3DES CTR mode implementation */

struct des3_ctr_ex {
  DES_key_schedule sched[3];
  unsigned char counter[8];
  int big_endian;
};

static uint32_t byteswap32(uint32_t in) {
  uint32_t out;

  out = (((in & 0x000000ff) << 24) |
         ((in & 0x0000ff00) << 8) |
         ((in & 0x00ff0000) >> 8) |
         ((in & 0xff000000) >> 24));

  return out;
}

static int init_des3_ctr(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc) {
  struct des3_ctr_ex *dce;

  dce = EVP_CIPHER_CTX_get_app_data(ctx);
  if (dce == NULL) {

    /* Allocate our data structure. */
    dce = calloc(1, sizeof(struct des3_ctr_ex));
    if (dce == NULL) {
      pr_log_pri(PR_LOG_ERR, MOD_SFTP_VERSION ": Out of memory!");
      _exit(1);
    }

    /* Simple test to see if we're on a big- or little-endian machine:
     * on big-endian machines, the ntohl() et al will be no-ops.
     */
    dce->big_endian = (ntohl(1234) == 1234);

    EVP_CIPHER_CTX_set_app_data(ctx, dce);
  }

  if (key != NULL) {
    register unsigned int i;
    unsigned char *ptr;

    ptr = (unsigned char *) key;

    for (i = 0; i < 3; i++) {
      DES_cblock material[8];
      memcpy(material, ptr, 8);
      ptr += 8;

      DES_set_key_unchecked(material, &(dce->sched[i]));
    }
  }

  if (iv != NULL) {
    memcpy(dce->counter, iv, 8);
  }

  return 1;
}

static int cleanup_des3_ctr(EVP_CIPHER_CTX *ctx) {
  struct des3_ctr_ex *dce;

  dce = EVP_CIPHER_CTX_get_app_data(ctx);
  if (dce != NULL) {
    pr_memscrub(dce, sizeof(struct des3_ctr_ex));
    free(dce);
    EVP_CIPHER_CTX_set_app_data(ctx, NULL);
  }

  return 1;
}

static int do_des3_ctr(EVP_CIPHER_CTX *ctx, unsigned char *dst,
    const unsigned char *src, unsigned int len) {
  struct des3_ctr_ex *dce;
  unsigned int n;
  unsigned char buf[8];

  if (len == 0)
    return 1;

  dce = EVP_CIPHER_CTX_get_app_data(ctx);
  if (dce == NULL)
    return 0;

  n = 0;

  while ((len--) > 0) {
    pr_signals_handle();

    if (n == 0) {
      DES_LONG ctr[2];

      memcpy(&(ctr[0]), dce->counter, sizeof(DES_LONG));
      memcpy(&(ctr[1]), dce->counter + sizeof(DES_LONG), sizeof(DES_LONG));

      if (dce->big_endian) {
        /* If we are on a big-endian machine, we need to initialize the counter
         * using little-endian values, since that is what OpenSSL's
         * DES_encryptX() functions expect.
         */

        ctr[0] = byteswap32(ctr[0]);
        ctr[1] = byteswap32(ctr[1]);
      }

      DES_encrypt3(ctr, &(dce->sched[0]), &(dce->sched[1]), &(dce->sched[2]));

      if (dce->big_endian) {
        ctr[0] = byteswap32(ctr[0]);
        ctr[1] = byteswap32(ctr[1]);
      }

      memcpy(buf, ctr, 8);

      ctr_incr(dce->counter, 8);
    }

    *(dst++) = *(src++) ^ buf[n];
    n = (n + 1) % 8;
  }

  return 1;
}

static const EVP_CIPHER *get_des3_ctr_cipher(void) {
  static EVP_CIPHER des3_ctr_cipher;

  memset(&des3_ctr_cipher, 0, sizeof(EVP_CIPHER));

  des3_ctr_cipher.nid = NID_undef;
  des3_ctr_cipher.block_size = 8;
  des3_ctr_cipher.iv_len = 8;
  des3_ctr_cipher.key_len = 24;
  des3_ctr_cipher.init = init_des3_ctr;
  des3_ctr_cipher.cleanup = cleanup_des3_ctr;
  des3_ctr_cipher.do_cipher = do_des3_ctr;

  des3_ctr_cipher.flags = EVP_CIPH_CBC_MODE|EVP_CIPH_VARIABLE_LENGTH|EVP_CIPH_ALWAYS_CALL_INIT|EVP_CIPH_CUSTOM_IV;

  return &des3_ctr_cipher;
}

#if OPENSSL_VERSION_NUMBER > 0x000907000L

/* AES CTR mode implementation */
struct aes_ctr_ex {
  AES_KEY key;
  unsigned char counter[AES_BLOCK_SIZE];
  unsigned char enc_counter[AES_BLOCK_SIZE];
  unsigned int num;
};

static int init_aes_ctr(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc) {
  struct aes_ctr_ex *ace;

  ace = EVP_CIPHER_CTX_get_app_data(ctx);
  if (ace == NULL) {

    /* Allocate our data structure. */
    ace = calloc(1, sizeof(struct aes_ctr_ex));
    if (ace == NULL) {
      pr_log_pri(PR_LOG_ERR, MOD_SFTP_VERSION ": Out of memory!");
      _exit(1);
    }

    EVP_CIPHER_CTX_set_app_data(ctx, ace);
  }

  if (key != NULL) {
    int nbits;

# if OPENSSL_VERSION_NUMBER == 0x0090805fL
    /* OpenSSL 0.9.8e had a bug where EVP_CIPHER_CTX_key_length() returned
     * the cipher key length rather than the context key length.
     */
    nbits = ctx->key_len * 8;
# else
    nbits = EVP_CIPHER_CTX_key_length(ctx) * 8;
# endif

    AES_set_encrypt_key(key, nbits, &(ace->key));
  }

  if (iv != NULL) {
    memcpy(ace->counter, iv, AES_BLOCK_SIZE);
  }

  return 1;
}

static int cleanup_aes_ctr(EVP_CIPHER_CTX *ctx) {
  struct aes_ctr_ex *ace;

  ace = EVP_CIPHER_CTX_get_app_data(ctx);
  if (ace != NULL) {
    pr_memscrub(ace, sizeof(struct aes_ctr_ex));
    free(ace);
    EVP_CIPHER_CTX_set_app_data(ctx, NULL);
  }

  return 1;
}

static int do_aes_ctr(EVP_CIPHER_CTX *ctx, unsigned char *dst,
    const unsigned char *src, unsigned int len) {
  struct aes_ctr_ex *ace;
# if OPENSSL_VERSION_NUMBER <= 0x0090704fL
  unsigned int n;
  unsigned char buf[AES_BLOCK_SIZE];
# endif

  if (len == 0)
    return 1;

  ace = EVP_CIPHER_CTX_get_app_data(ctx);
  if (ace == NULL)
    return 0;

# if OPENSSL_VERSION_NUMBER <= 0x0090704fL
  /* In OpenSSL-0.9.7d and earlier, the AES CTR code did not properly handle
   * the IV as big-endian; this would cause the dreaded "Incorrect MAC
   * received on packet" error when using clients e.g. PuTTy.  To see
   * the difference in OpenSSL, you have do manually do:
   *
   *  diff -u openssl-0.9.7d/crypto/aes/aes_ctr.c \
   *    openssl-0.9.7e/crypto/aes/aes_ctr.c
   *
   * This change is not documented in OpenSSL's CHANGES file.  Sigh.
   *
   * Thus for these versions, we have to use our own AES CTR code.
   */

  n = 0;

  while ((len--) > 0) {
    pr_signals_handle();

    if (n == 0) {
      AES_encrypt(ace->counter, buf, &(ace->key));
      ctr_incr(ace->counter, AES_BLOCK_SIZE);
    }

    *(dst++) = *(src++) ^ buf[n];
    n = (n + 1) % AES_BLOCK_SIZE;
  }

  return 1;
# else
  /* Thin wrapper around AES_ctr128_encrypt(). */
  AES_ctr128_encrypt(src, dst, len, &(ace->key), ace->counter, ace->enc_counter,
    &(ace->num));
# endif

  return 1;
}

static const EVP_CIPHER *get_aes_ctr_cipher(int key_len) {
  static EVP_CIPHER aes_ctr_cipher;

  memset(&aes_ctr_cipher, 0, sizeof(EVP_CIPHER));

  aes_ctr_cipher.nid = NID_undef;
  aes_ctr_cipher.block_size = AES_BLOCK_SIZE;
  aes_ctr_cipher.iv_len = AES_BLOCK_SIZE;
  aes_ctr_cipher.key_len = key_len;
  aes_ctr_cipher.init = init_aes_ctr;
  aes_ctr_cipher.cleanup = cleanup_aes_ctr;
  aes_ctr_cipher.do_cipher = do_aes_ctr;

  aes_ctr_cipher.flags = EVP_CIPH_CBC_MODE|EVP_CIPH_VARIABLE_LENGTH|EVP_CIPH_ALWAYS_CALL_INIT|EVP_CIPH_CUSTOM_IV;

  return &aes_ctr_cipher;
}
#endif /* OpenSSL older than 0.9.7 */

const EVP_CIPHER *sftp_crypto_get_cipher(const char *name, size_t *key_len,
    size_t *discard_len) {
  register unsigned int i;

  for (i = 0; ciphers[i].name; i++) {
    if (strcmp(ciphers[i].name, name) == 0) {
      const EVP_CIPHER *cipher;

      if (strcmp(name, "blowfish-ctr") == 0) {
        cipher = get_bf_ctr_cipher();

      } else if (strcmp(name, "3des-ctr") == 0) {
        cipher = get_des3_ctr_cipher();

#if OPENSSL_VERSION_NUMBER > 0x000907000L
      } else if (strcmp(name, "aes256-ctr") == 0) {
        cipher = get_aes_ctr_cipher(32);

      } else if (strcmp(name, "aes192-ctr") == 0) {
        cipher = get_aes_ctr_cipher(24);

      } else if (strcmp(name, "aes128-ctr") == 0) {
        cipher = get_aes_ctr_cipher(16);
#endif /* OpenSSL older than 0.9.7 */

      } else {
        cipher = ciphers[i].get_type();
      }

      if (key_len) {
        if (strcmp(name, "arcfour256") != 0) {
          *key_len = 0;

        } else {
          /* The arcfour256 cipher is special-cased here in order to use
           * a longer key (32 bytes), rather than the normal 16 bytes for the
           * RC4 cipher.
           */
          *key_len = 32;
        }
      }

      if (discard_len)
        *discard_len = ciphers[i].discard_len;

      return cipher;
    }
  }

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "no cipher matching '%s' found", name);
  return NULL;
}

const EVP_MD *sftp_crypto_get_digest(const char *name, uint32_t *mac_len) {
  register unsigned int i;

  for (i = 0; digests[i].name; i++) {
    if (strcmp(digests[i].name, name) == 0) {
      const EVP_MD *digest = digests[i].get_type();
      if (mac_len) {
        *mac_len = digests[i].mac_len;
      }

      return digest;
    }
  }

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "no digest matching '%s' found", name);
  return NULL;
}

const char *sftp_crypto_get_kexinit_cipher_list(pool *p) {
  char *res = "";
  config_rec *c;

  /* Make sure that OpenSSL can use these ciphers.  For example, in FIPS mode,
   * some ciphers cannot be used.  So we should not advertise ciphers that we
   * know we cannot use.
   */

  c = find_config(main_server->conf, CONF_PARAM, "SFTPCiphers", FALSE);
  if (c) {
    register unsigned int i;

    for (i = 0; i < c->argc; i++) {
      register unsigned int j;

      for (j = 0; ciphers[j].name; j++) {
        if (strcmp(c->argv[i], ciphers[j].name) == 0) {
          if (strcmp(c->argv[i], "none") != 0) {
            if (EVP_get_cipherbyname(ciphers[j].openssl_name) != NULL) {
              res = pstrcat(p, res, *res ? "," : "",
                pstrdup(p, ciphers[j].name), NULL);

            } else {
              /* The CTR modes are special cases. */

              if (strcmp(ciphers[j].name, "blowfish-ctr") == 0 ||
                  strcmp(ciphers[j].name, "3des-ctr") == 0
#if OPENSSL_VERSION_NUMBER > 0x000907000L
                  || strcmp(ciphers[j].name, "aes256-ctr") == 0 ||
                  strcmp(ciphers[j].name, "aes192-ctr") == 0 ||
                  strcmp(ciphers[j].name, "aes128-ctr") == 0
#endif
                  ) {
                res = pstrcat(p, res, *res ? "," : "",
                  pstrdup(p, ciphers[j].name), NULL);
       
              } else {
                pr_trace_msg(trace_channel, 3,
                  "unable to use '%s' cipher: Unsupported by OpenSSL",
                  ciphers[j].name);
              }
            }

          } else {
            res = pstrcat(p, res, *res ? "," : "",
              pstrdup(p, ciphers[j].name), NULL);
          }
        }
      }
    }

  } else {
    register unsigned int i;

    for (i = 0; ciphers[i].name; i++) {
      if (ciphers[i].enabled) {
        if (strcmp(ciphers[i].name, "none") != 0) {
          if (EVP_get_cipherbyname(ciphers[i].openssl_name) != NULL) {
            res = pstrcat(p, res, *res ? "," : "",
              pstrdup(p, ciphers[i].name), NULL);

          } else {
            /* The CTR modes are special cases. */

            if (strcmp(ciphers[i].name, "blowfish-ctr") == 0 ||
                strcmp(ciphers[i].name, "3des-ctr") == 0
#if OPENSSL_VERSION_NUMBER > 0x000907000L
                || strcmp(ciphers[i].name, "aes256-ctr") == 0 ||
                strcmp(ciphers[i].name, "aes192-ctr") == 0 ||
                strcmp(ciphers[i].name, "aes128-ctr") == 0
#endif
                ) {
              res = pstrcat(p, res, *res ? "," : "",
                pstrdup(p, ciphers[i].name), NULL);

            } else {       
              pr_trace_msg(trace_channel, 3,
                "unable to use '%s' cipher: Unsupported by OpenSSL",
                ciphers[i].name);
            }
          }

        } else {
          res = pstrcat(p, res, *res ? "," : "",
            pstrdup(p, ciphers[i].name), NULL);
        }

      } else {
        pr_trace_msg(trace_channel, 3, "unable to use '%s' cipher: "
          "Must be explicitly requested via SFTPCiphers", ciphers[i].name);
      }
    }
  }

  return res;
}

const char *sftp_crypto_get_kexinit_digest_list(pool *p) {
  char *res = "";
  config_rec *c;

  /* Make sure that OpenSSL can use these digests.  For example, in FIPS
   * mode, some digests cannot be used.  So we should not advertise digests
   * that we know we cannot use.
   */

  c = find_config(main_server->conf, CONF_PARAM, "SFTPDigests", FALSE);
  if (c) {
    register unsigned int i;

    for (i = 0; i < c->argc; i++) {
      register unsigned int j;

      for (j = 0; digests[j].name; j++) {
        if (strcmp(c->argv[i], digests[j].name) == 0) {
          if (strcmp(c->argv[i], "none") != 0) {
            if (EVP_get_digestbyname(digests[j].openssl_name) != NULL) {
              res = pstrcat(p, res, *res ? "," : "",
                pstrdup(p, digests[j].name), NULL);

            } else {
              pr_trace_msg(trace_channel, 3,
                "unable to use '%s' digest: Unsupported by OpenSSL",
                digests[j].name);
            }

          } else {
            res = pstrcat(p, res, *res ? "," : "",
              pstrdup(p, digests[j].name), NULL);
          }
        }
      }
    }

  } else {
    register unsigned int i;

    for (i = 0; digests[i].name; i++) {
      if (digests[i].enabled) {
        if (strcmp(digests[i].name, "none") != 0) {
          if (EVP_get_digestbyname(digests[i].openssl_name) != NULL) {
            res = pstrcat(p, res, *res ? "," : "",
              pstrdup(p, digests[i].name), NULL);

          } else {
            pr_trace_msg(trace_channel, 3,
              "unable to use '%s' digest: Unsupported by OpenSSL",
              digests[i].name);
          }

        } else {
          res = pstrcat(p, res, *res ? "," : "",
            pstrdup(p, digests[i].name), NULL);
        }

      } else {
        pr_trace_msg(trace_channel, 3, "unable to use '%s' digest: "
          "Must be explicitly requested via SFTPDigests", digests[i].name);
      }
    }
  }

  return res;
}

const char *sftp_crypto_get_errors(void) {
  unsigned int count = 0;
  unsigned long e = ERR_get_error();
  BIO *bio = NULL;
  char *data = NULL;
  long datalen;
  const char *str = "(unknown)";

  /* Use ERR_print_errors() and a memory BIO to build up a string with
   * all of the error messages from the error queue.
   */

  if (e)
    bio = BIO_new(BIO_s_mem());

  while (e) {
    pr_signals_handle();
    BIO_printf(bio, "\n  (%u) %s", ++count, ERR_error_string(e, NULL));
    e = ERR_get_error();
  }

  datalen = BIO_get_mem_data(bio, &data);
  if (data) {
    data[datalen] = '\0';
    str = pstrdup(sftp_pool, data);
  }

  if (bio)
    BIO_free(bio);

  return str;
}

/* Try to find the best multiple/block size which accommodates the two given
 * sizes by rounding up.
 */
size_t sftp_crypto_get_size(size_t first, size_t second) {
#ifdef roundup
  return roundup(first, second);
#else
  return (((first + (second - 1)) / second) * second);
#endif /* !roundup */
}

void sftp_crypto_free(int flags) {
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  if (crypto_engine) {
    ENGINE_cleanup();
    crypto_engine = NULL;
  }
#endif

  ERR_free_strings();
  ERR_remove_state(0);
  EVP_cleanup();
  RAND_cleanup();
}

int sftp_crypto_set_driver(const char *driver) {
#if OPENSSL_VERSION_NUMBER > 0x000907000L
  if (driver == NULL) {
    errno = EINVAL;
    return -1;
  }

  crypto_engine = driver;

  if (strcasecmp(driver, "ALL") == 0) {
    /* Load all ENGINE implementations bundled with OpenSSL. */
    ENGINE_load_builtin_engines();
    ENGINE_register_all_complete();

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "enabled all builtin crypto devices");

  } else {
    ENGINE *e;

    /* Load all ENGINE implementations bundled with OpenSSL. */
    ENGINE_load_builtin_engines();

    e = ENGINE_by_id(driver);
    if (e) {
      if (ENGINE_init(e)) {
        if (ENGINE_set_default(e, ENGINE_METHOD_ALL)) {
          ENGINE_finish(e);
          ENGINE_free(e);

          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "using SFTPCryptoDevice '%s'", driver);

        } else {
          /* The requested driver could not be used as the default for
           * some odd reason.
           */
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "unable to register SFTPCryptoDevice '%s' as the default: %s",
            driver, sftp_crypto_get_errors());

          ENGINE_finish(e);
          ENGINE_free(e);
          e = NULL;
          crypto_engine = NULL;

          errno = EPERM;
          return -1;
        }

      } else {
        /* The requested driver could not be initialized. */
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unable to initialize SFTPCryptoDevice '%s': %s", driver,
          sftp_crypto_get_errors());

        ENGINE_free(e);
        e = NULL;
        crypto_engine = NULL;

        errno = EPERM;
        return -1;
      }

    } else {
      /* The requested driver is not available. */
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "SFTPCryptoDevice '%s' is not available", driver);

      crypto_engine = NULL;

      errno = EPERM;
      return -1;
    }
  }

  return 0;
#else
  errno = ENOSYS;
  return -1;
#endif
}

