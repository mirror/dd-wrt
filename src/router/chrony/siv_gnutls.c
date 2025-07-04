/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020, 2023
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 **********************************************************************

  =======================================================================

  SIV ciphers using the GnuTLS library
  */

#include "config.h"

#include "sysincl.h"

#include <gnutls/crypto.h>

#include "logging.h"
#include "memory.h"
#include "siv.h"

struct SIV_Instance_Record {
  gnutls_cipher_algorithm_t algorithm;
  gnutls_aead_cipher_hd_t cipher;
  int min_nonce_length;
  int max_nonce_length;
};

/* ================================================== */

static int instance_counter = 0;
static int gnutls_initialised = 0;

/* ================================================== */

static void
init_gnutls(void)
{
  int r;

  if (gnutls_initialised)
    return;

  r = gnutls_global_init();
  if (r < 0)
    LOG_FATAL("Could not initialise %s : %s", "gnutls", gnutls_strerror(r));

  DEBUG_LOG("Initialised");
  gnutls_initialised = 1;
}

/* ================================================== */

static void
deinit_gnutls(void)
{
  assert(gnutls_initialised);
  gnutls_global_deinit();
  gnutls_initialised = 0;
  DEBUG_LOG("Deinitialised");
}

/* ================================================== */

static gnutls_cipher_algorithm_t
get_cipher_algorithm(SIV_Algorithm algorithm)
{
  switch (algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      return GNUTLS_CIPHER_AES_128_SIV;
#if HAVE_GNUTLS_SIV_GCM
    case AEAD_AES_128_GCM_SIV:
      return GNUTLS_CIPHER_AES_128_SIV_GCM;
#endif
    default:
      return 0;
  }
}

/* ================================================== */

SIV_Instance
SIV_CreateInstance(SIV_Algorithm algorithm)
{
  gnutls_cipher_algorithm_t calgo;
  SIV_Instance instance;

  calgo = get_cipher_algorithm(algorithm);
  if (calgo == 0)
    return NULL;

  if (instance_counter == 0)
    init_gnutls();

  /* Check if the cipher is actually supported */
  if (gnutls_cipher_get_tag_size(calgo) == 0) {
    if (instance_counter == 0)
      deinit_gnutls();
    return NULL;
  }

  instance = MallocNew(struct SIV_Instance_Record);
  instance->algorithm = calgo;
  instance->cipher = NULL;

  switch (algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      instance->min_nonce_length = 1;
      instance->max_nonce_length = INT_MAX;
      break;
    case AEAD_AES_128_GCM_SIV:
      instance->min_nonce_length = 12;
      instance->max_nonce_length = 12;
      break;
    default:
      assert(0);
  }

  instance_counter++;

  return instance;
}

/* ================================================== */

void
SIV_DestroyInstance(SIV_Instance instance)
{
  if (instance->cipher)
    gnutls_aead_cipher_deinit(instance->cipher);
  Free(instance);

  instance_counter--;
  if (instance_counter == 0)
    deinit_gnutls();
}

/* ================================================== */

int
SIV_GetKeyLength(SIV_Algorithm algorithm)
{
  gnutls_cipher_algorithm_t calgo = get_cipher_algorithm(algorithm);
  int len;

  if (calgo == 0)
    return 0;

  len = gnutls_cipher_get_key_size(calgo);
  if (len == 0)
    return 0;

  if (len < 1 || len > SIV_MAX_KEY_LENGTH)
    LOG_FATAL("Invalid key length");

  return len;
}

/* ================================================== */

int
SIV_SetKey(SIV_Instance instance, const unsigned char *key, int length)
{
  gnutls_aead_cipher_hd_t cipher;
  gnutls_datum_t datum;
  int r;

  if (length <= 0 || length != gnutls_cipher_get_key_size(instance->algorithm))
    return 0;

  datum.data = (unsigned char *)key;
  datum.size = length;

#ifdef HAVE_GNUTLS_AEAD_CIPHER_SET_KEY
  if (instance->cipher) {
    r = gnutls_aead_cipher_set_key(instance->cipher, &datum);
    if (r < 0) {
      DEBUG_LOG("Could not set cipher key : %s", gnutls_strerror(r));
      return 0;
    }

    return 1;
  }
#endif

  /* Initialise a new cipher with the provided key */
  r = gnutls_aead_cipher_init(&cipher, instance->algorithm, &datum);
  if (r < 0) {
    DEBUG_LOG("Could not initialise %s : %s", "cipher", gnutls_strerror(r));
    return 0;
  }

  /* Destroy the previous cipher (if its key could not be changed directly) */
  if (instance->cipher)
    gnutls_aead_cipher_deinit(instance->cipher);

  instance->cipher = cipher;

  return 1;
}

/* ================================================== */

int
SIV_GetMinNonceLength(SIV_Instance instance)
{
  return instance->min_nonce_length;
}

/* ================================================== */

int
SIV_GetMaxNonceLength(SIV_Instance instance)
{
  return instance->max_nonce_length;
}

/* ================================================== */

int
SIV_GetTagLength(SIV_Instance instance)
{
  int len;

  len = gnutls_cipher_get_tag_size(instance->algorithm);

  if (len < 1 || len > SIV_MAX_TAG_LENGTH)
    LOG_FATAL("Invalid tag length");

  return len;
}

/* ================================================== */

int
SIV_Encrypt(SIV_Instance instance,
            const unsigned char *nonce, int nonce_length,
            const void *assoc, int assoc_length,
            const void *plaintext, int plaintext_length,
            unsigned char *ciphertext, int ciphertext_length)
{
  size_t clen = ciphertext_length;

  if (!instance->cipher)
    return 0;

  if (nonce_length < instance->min_nonce_length ||
      nonce_length > instance->max_nonce_length || assoc_length < 0 ||
      plaintext_length < 0 || ciphertext_length < 0)
    return 0;

  assert(assoc && plaintext);

  if (gnutls_aead_cipher_encrypt(instance->cipher,
                                 nonce, nonce_length, assoc, assoc_length, 0,
                                 plaintext, plaintext_length, ciphertext, &clen) < 0)
    return 0;

  if (clen != ciphertext_length)
    return 0;

  return 1;
}

/* ================================================== */

int
SIV_Decrypt(SIV_Instance instance,
            const unsigned char *nonce, int nonce_length,
            const void *assoc, int assoc_length,
            const unsigned char *ciphertext, int ciphertext_length,
            void *plaintext, int plaintext_length)
{
  size_t plen = plaintext_length;

  if (!instance->cipher)
    return 0;

  if (nonce_length < instance->min_nonce_length ||
      nonce_length > instance->max_nonce_length || assoc_length < 0 ||
      plaintext_length < 0 || ciphertext_length < 0)
    return 0;

  assert(assoc && plaintext);

  if (gnutls_aead_cipher_decrypt(instance->cipher,
                                 nonce, nonce_length, assoc, assoc_length, 0,
                                 ciphertext, ciphertext_length, plaintext, &plen) < 0)
    return 0;

  if (plen != plaintext_length)
    return 0;

  return 1;
}
