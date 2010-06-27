/*
 * ProFTPD - mod_sftp MACs
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
 * $Id: mac.c,v 1.5 2009/09/16 20:51:15 castaglia Exp $
 */

#include "mod_sftp.h"
#include "ssh2.h"
#include "msg.h"
#include "packet.h"
#include "crypto.h"
#include "mac.h"
#include "session.h"
#include "disconnect.h"
#include "interop.h"

struct sftp_mac {
  const char *algo;
  const EVP_MD *digest;

  unsigned char *key;

  /* The keysz and key_len are usually the same; they can differ if, for
   * example, the client always truncates the MAC key len to 16 bits.
   */
  size_t keysz;
  uint32_t key_len;

  uint32_t mac_len;
};

/* We need to keep the old MACs around, so that we can handle N arbitrary
 * packets to/from the client using the old keys, as during rekeying.
 * Thus we have two read MAC contexts, two write MAC contexts.
 * The cipher idx variable indicates which of the MACs is currently in use.
 */

static struct sftp_mac read_macs[] = {
  { NULL, NULL, NULL, 0 },
  { NULL, NULL, NULL, 0 }
};
static HMAC_CTX read_ctxs[2];

static struct sftp_mac write_macs[] = {
  { NULL, NULL, NULL, 0 },
  { NULL, NULL, NULL, 0 }
};
static HMAC_CTX write_ctxs[2];

static size_t mac_blockszs[2] = { 0, 0 };

static unsigned int read_mac_idx = 0;
static unsigned int write_mac_idx = 0;

static void clear_mac(struct sftp_mac *);

static unsigned int get_next_read_index(void) {
  if (read_mac_idx == 1)
    return 0;

  return 1;
}

static unsigned int get_next_write_index(void) {
  if (write_mac_idx == 1)
    return 0;

  return 1;
}

static void switch_read_mac(void) {
  /* First we can clear the read MAC, kept from rekeying. */
  if (read_macs[read_mac_idx].key) {
    clear_mac(&(read_macs[read_mac_idx]));
#if OPENSSL_VERSION_NUMBER > 0x000907000L
    HMAC_CTX_cleanup(&(read_ctxs[read_mac_idx]));
#else
    HMAC_cleanup(&(read_ctxs[read_mac_idx]));
#endif
    mac_blockszs[read_mac_idx] = 0; 

    /* Now we can switch the index. */
    if (read_mac_idx == 1) {
      read_mac_idx = 0;
      return;
    }

    read_mac_idx = 1;
  }
}

static void switch_write_mac(void) {
  /* First we can clear the write MAC, kept from rekeying. */
  if (write_macs[write_mac_idx].key) {
    clear_mac(&(write_macs[write_mac_idx]));
#if OPENSSL_VERSION_NUMBER > 0x000907000L
    HMAC_CTX_cleanup(&(write_ctxs[write_mac_idx]));
#else
    HMAC_cleanup(&(write_ctxs[write_mac_idx]));
#endif

    /* Now we can switch the index. */
    if (write_mac_idx == 1) {
      write_mac_idx = 0;
      return;
    }

    write_mac_idx = 1;
  }
}

static void clear_mac(struct sftp_mac *mac) {
  if (mac->key) {
    pr_memscrub(mac->key, mac->keysz);
    free(mac->key);
    mac->key = NULL;
    mac->keysz = 0;
    mac->key_len = 0;
  }

  mac->digest = NULL;
  mac->algo = NULL;
}

static int set_mac_key(struct sftp_mac *mac, const EVP_MD *hash,
    const char *k, uint32_t klen, const char *h, uint32_t hlen, char *letter,
    const unsigned char *id, uint32_t id_len) {
 
  EVP_MD_CTX ctx;
  unsigned char *key = NULL;
  size_t key_sz;
  uint32_t key_len = 0;
 
  key_sz = sftp_crypto_get_size(EVP_MD_block_size(mac->digest),
    EVP_MD_size(hash)); 

  key = malloc(key_sz);
  if (key == NULL) {
    pr_log_pri(PR_LOG_CRIT, MOD_SFTP_VERSION ": Out of memory!");
    _exit(1);
  }

  /* In OpenSSL 0.9.6, many of the EVP_Digest* functions returned void, not
   * int.  Without these ugly OpenSSL version preprocessor checks, the
   * compiler will error out with "void value not ignored as it ought to be".
   */

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestInit(&ctx, hash) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing message digest: %s", sftp_crypto_get_errors());
    return -1;
  }
#else
  EVP_DigestInit(&ctx, hash);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, k, klen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest with K: %s", sftp_crypto_get_errors());
    return -1;
  }
#else
  EVP_DigestUpdate(&ctx, k, klen);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, h, hlen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest with H: %s", sftp_crypto_get_errors());
    return -1;
  }
#else
  EVP_DigestUpdate(&ctx, h, hlen);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, letter, sizeof(char)) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest with '%c': %s", *letter,
      sftp_crypto_get_errors());
    return -1;
  }
#else
  EVP_DigestUpdate(&ctx, letter, sizeof(char));
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(&ctx, (char *) id, id_len) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating message digest with ID: %s", sftp_crypto_get_errors());
    return -1;
  }
#else
  EVP_DigestUpdate(&ctx, (char *) id, id_len);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestFinal(&ctx, key, &key_len) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error finalizing message digest: %s", sftp_crypto_get_errors());
    return -1;
  }
#else
  EVP_DigestFinal(&ctx, key, &key_len);
#endif

  /* If we need more, keep hashing, as per RFC, until we have enough
   * material.
   */
  while (key_sz > key_len) {
    uint32_t len = key_len;

    pr_signals_handle();

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
    if (EVP_DigestInit(&ctx, hash) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error initializing message digest: %s", sftp_crypto_get_errors());
      return -1;
    }
#else
    EVP_DigestInit(&ctx, hash);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
    if (EVP_DigestUpdate(&ctx, k, klen) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error updating message digest with K: %s", sftp_crypto_get_errors());
      return -1;
    }
#else
    EVP_DigestUpdate(&ctx, k, klen);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
    if (EVP_DigestUpdate(&ctx, h, hlen) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error updating message digest with H: %s", sftp_crypto_get_errors());
      return -1;
    }
#else
    EVP_DigestUpdate(&ctx, h, hlen);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
    if (EVP_DigestUpdate(&ctx, key, len) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error updating message digest with data: %s",
        sftp_crypto_get_errors());
      return -1;
    }
#else
    EVP_DigestUpdate(&ctx, key, len);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
    if (EVP_DigestFinal(&ctx, key + len, &len) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error finalizing message digest: %s", sftp_crypto_get_errors());
      return -1;
    }
#else
    EVP_DigestFinal(&ctx, key + len, &len);
#endif

    key_len += len;
  }

  mac->key = key;
  mac->keysz = key_sz;

  mac->key_len = EVP_MD_size(mac->digest);
  if (!sftp_interop_supports_feature(SFTP_SSH2_FEAT_MAC_LEN)) {
    mac->key_len = 16;
  }

  return 0;
}

size_t sftp_mac_get_block_size(void) {
  return mac_blockszs[read_mac_idx];
}

void sftp_mac_set_block_size(size_t blocksz) {
  if (blocksz > mac_blockszs[read_mac_idx]) {
    mac_blockszs[read_mac_idx] = blocksz;
  }
}

const char *sftp_mac_get_read_algo(void) {
  if (read_macs[read_mac_idx].key) {
    return read_macs[read_mac_idx].algo;
  }

  return NULL;
}

int sftp_mac_set_read_algo(const char *algo) {
  uint32_t mac_len;
  unsigned int idx = write_mac_idx;

  if (read_macs[idx].key) {
    /* If we have an existing key, it means that we are currently rekeying. */
    idx = get_next_read_index();
  }

  read_macs[idx].digest = sftp_crypto_get_digest(algo, &mac_len);
  if (read_macs[idx].digest == NULL)
    return -1;

  read_macs[idx].algo = algo;
  read_macs[idx].mac_len = mac_len;
  return 0;
}

int sftp_mac_set_read_key(pool *p, const EVP_MD *hash, const BIGNUM *k,
    const char *h, uint32_t hlen) {
  const unsigned char *id = NULL;
  char *buf, *ptr;
  uint32_t buflen, bufsz, id_len;
  char letter;
  size_t blocksz;
  struct sftp_mac *mac;
  HMAC_CTX *mac_ctx;

  switch_read_mac();

  mac = &(read_macs[read_mac_idx]);
  mac_ctx = &(read_ctxs[read_mac_idx]);

  bufsz = buflen = 1024;
  ptr = buf = sftp_msg_getbuf(p, bufsz);

  /* Need to use SSH2-style format of K for the key. */
  sftp_msg_write_mpint(&buf, &buflen, k);

  id_len = sftp_session_get_id(&id);

  /* HASH(K || H || "E" || session_id) */
  letter = 'E';
  set_mac_key(mac, hash, ptr, (bufsz - buflen), h, hlen, &letter, id, id_len);

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  HMAC_CTX_init(mac_ctx);
#else
  /* Reset the HMAC context. */
  HMAC_Init(mac_ctx, NULL, 0, NULL);
#endif
  HMAC_Init(mac_ctx, mac->key, mac->key_len, mac->digest);

  if (mac->mac_len == 0) {
    blocksz = EVP_MD_size(mac->digest);

  } else {
    blocksz = mac->mac_len;
  }

  pr_memscrub(ptr, bufsz);
  sftp_mac_set_block_size(blocksz);
  return 0;
}

int sftp_mac_read_data(struct ssh2_packet *pkt) {
  struct sftp_mac *mac;
  HMAC_CTX *mac_ctx;

  mac = &(read_macs[read_mac_idx]);
  mac_ctx = &(read_ctxs[read_mac_idx]);

  if (mac->key) {
    unsigned char *mac_data;
    char *buf, *ptr;
    uint32_t buflen, bufsz = (sizeof(uint32_t) * 2) + pkt->packet_len,
      mac_len = 0;

    mac_data = pcalloc(pkt->pool, EVP_MAX_MD_SIZE);

    buflen = bufsz;
    ptr = buf = sftp_msg_getbuf(pkt->pool, bufsz);

    sftp_msg_write_int(&buf, &buflen, pkt->seqno);
    sftp_msg_write_int(&buf, &buflen, pkt->packet_len);
    sftp_msg_write_byte(&buf, &buflen, pkt->padding_len);
    sftp_msg_write_data(&buf, &buflen, pkt->payload, pkt->payload_len, FALSE);
    sftp_msg_write_data(&buf, &buflen, pkt->padding, pkt->padding_len, FALSE);

    HMAC_Init(mac_ctx, NULL, 0, NULL);
    HMAC_Update(mac_ctx, (unsigned char *) ptr, (bufsz - buflen));
    HMAC_Final(mac_ctx, mac_data, &mac_len);

    if (mac_len == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error computing MAC using %s: %s", mac->algo,
        sftp_crypto_get_errors());
      SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_MAC_ERROR, NULL);
    }

    if (mac->mac_len != 0) {
      mac_len = mac->mac_len;
    }

    if (mac_len != pkt->mac_len) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "expected %u MAC len from client, got %lu", mac_len,
        (unsigned long) pkt->mac_len);
      SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_MAC_ERROR, NULL);
    }

    if (memcmp(mac_data, pkt->mac, mac_len) != 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "MAC from client differs from expected MAC using %s", mac->algo);

#ifdef SFTP_DEBUG_PACKET
{
      unsigned int i;
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "client MAC (len %lu):", (unsigned long) pkt->mac_len);
      for (i = 0; i < mac_len;) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "  %02x%02x %02x%02x %02x%02x %02x%02x",
          ((unsigned char *) pkt->mac)[i], ((unsigned char *) pkt->mac)[i+1],
          ((unsigned char *) pkt->mac)[i+2], ((unsigned char *) pkt->mac)[i+3],
          ((unsigned char *) pkt->mac)[i+4], ((unsigned char *) pkt->mac)[i+5],
          ((unsigned char *) pkt->mac)[i+6], ((unsigned char *) pkt->mac)[i+7]);
        i += 8;
      }

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "server MAC (len %lu):", (unsigned long) mac_len);
      for (i = 0; i < mac_len;) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "  %02x%02x %02x%02x %02x%02x %02x%02x",
          ((unsigned char *) mac)[i], ((unsigned char *) mac)[i+1],
          ((unsigned char *) mac)[i+2], ((unsigned char *) mac)[i+3],
          ((unsigned char *) mac)[i+4], ((unsigned char *) mac)[i+5],
          ((unsigned char *) mac)[i+6], ((unsigned char *) mac)[i+7]);
        i += 8;
      }
}
#endif

      SFTP_DISCONNECT_CONN(SFTP_SSH2_DISCONNECT_MAC_ERROR, NULL);
    }

    return 0;
  }

  return 0;
}

const char *sftp_mac_get_write_algo(void) {
  if (write_macs[write_mac_idx].key) {
    return write_macs[write_mac_idx].algo;
  }

  return NULL;
}

int sftp_mac_set_write_algo(const char *algo) {
  uint32_t mac_len;
  unsigned int idx = write_mac_idx;

  if (write_macs[idx].key) {
    /* If we have an existing key, it means that we are currently rekeying. */
    idx = get_next_write_index();
  }

  write_macs[idx].digest = sftp_crypto_get_digest(algo, &mac_len);
  if (write_macs[idx].digest == NULL)
    return -1;

  write_macs[idx].algo = algo;
  write_macs[idx].mac_len = mac_len;
  return 0;
}

int sftp_mac_set_write_key(pool *p, const EVP_MD *hash, const BIGNUM *k,
    const char *h, uint32_t hlen) {
  const unsigned char *id = NULL;
  char *buf, *ptr;
  uint32_t buflen, bufsz, id_len;
  char letter;
  struct sftp_mac *mac;
  HMAC_CTX *mac_ctx;

  switch_write_mac();

  mac = &(write_macs[write_mac_idx]);
  mac_ctx = &(write_ctxs[write_mac_idx]);

  bufsz = buflen = 1024;
  ptr = buf = sftp_msg_getbuf(p, bufsz);

  /* Need to use SSH2-style format of K for the key. */
  sftp_msg_write_mpint(&buf, &buflen, k);

  id_len = sftp_session_get_id(&id);

  /* HASH(K || H || "F" || session_id) */
  letter = 'F';
  set_mac_key(mac, hash, ptr, (bufsz - buflen), h, hlen, &letter, id, id_len);

#if OPENSSL_VERSION_NUMBER > 0x000907000L
  HMAC_CTX_init(mac_ctx);
#else
  /* Reset the HMAC context. */
  HMAC_Init(mac_ctx, NULL, 0, NULL);
#endif
  HMAC_Init(mac_ctx, mac->key, mac->key_len, mac->digest);

  pr_memscrub(ptr, bufsz);
  return 0;
}

int sftp_mac_write_data(struct ssh2_packet *pkt) {
  struct sftp_mac *mac;
  HMAC_CTX *mac_ctx;

  mac = &(write_macs[write_mac_idx]);
  mac_ctx = &(write_ctxs[write_mac_idx]);

  if (mac->key) {
    unsigned char *mac_data;
    char *buf, *ptr;
    uint32_t buflen, bufsz = (sizeof(uint32_t) * 2) + pkt->packet_len,
      mac_len = 0;

    mac_data = pcalloc(pkt->pool, EVP_MAX_MD_SIZE);

    buflen = bufsz;
    ptr = buf = sftp_msg_getbuf(pkt->pool, bufsz);

    sftp_msg_write_int(&buf, &buflen, pkt->seqno);
    sftp_msg_write_int(&buf, &buflen, pkt->packet_len);
    sftp_msg_write_byte(&buf, &buflen, pkt->padding_len);
    sftp_msg_write_data(&buf, &buflen, pkt->payload, pkt->payload_len, FALSE);
    sftp_msg_write_data(&buf, &buflen, pkt->padding, pkt->padding_len, FALSE);

    HMAC_Init(mac_ctx, NULL, 0, NULL);
    HMAC_Update(mac_ctx, (unsigned char *) ptr, (bufsz - buflen));
    HMAC_Final(mac_ctx, mac_data, &mac_len);

    if (mac_len == 0) {
      pkt->mac = NULL;
      pkt->mac_len = 0;

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error computing MAC using %s: %s", mac->algo,
        sftp_crypto_get_errors());
      return -1;
    }

    if (mac->mac_len != 0) {
      mac_len = mac->mac_len;
    }

    pkt->mac_len = mac_len;
    pkt->mac = pcalloc(pkt->pool, pkt->mac_len);
    memcpy(pkt->mac, mac_data, mac_len);

#ifdef SFTP_DEBUG_PACKET
{
  unsigned int i;

  (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
    "server MAC (len %lu, seqno %lu):",
    (unsigned long) pkt->mac_len, (unsigned long) pkt->seqno);
  for (i = 0; i < mac_len;) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "  %02x%02x %02x%02x %02x%02x %02x%02x",
      ((unsigned char *) pkt->mac)[i], ((unsigned char *) pkt->mac)[i+1],
      ((unsigned char *) pkt->mac)[i+2], ((unsigned char *) pkt->mac)[i+3],
      ((unsigned char *) pkt->mac)[i+4], ((unsigned char *) pkt->mac)[i+5],
      ((unsigned char *) pkt->mac)[i+6], ((unsigned char *) pkt->mac)[i+7]);
    i += 8;
  }
}
#endif

    return 0;
  }

  pkt->mac = NULL;
  pkt->mac_len = 0;

  return 0;
}

