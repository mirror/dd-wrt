/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019, 2022
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

  SIV ciphers using the Nettle library
  */

#include "config.h"

#include "sysincl.h"

#include <nettle/siv-cmac.h>
#ifdef HAVE_NETTLE_SIV_GCM
#include <nettle/siv-gcm.h>
#endif

#include "memory.h"
#include "siv.h"
#include "util.h"

struct SIV_Instance_Record {
  SIV_Algorithm algorithm;
  int key_set;
  int min_nonce_length;
  int max_nonce_length;
  int tag_length;
  union {
    struct siv_cmac_aes128_ctx cmac_aes128;
#ifdef HAVE_NETTLE_SIV_GCM
    struct aes128_ctx aes128;
#endif
  } ctx;
};

/* ================================================== */

SIV_Instance
SIV_CreateInstance(SIV_Algorithm algorithm)
{
  SIV_Instance instance;

  if (SIV_GetKeyLength(algorithm) <= 0)
    return NULL;

  instance = MallocNew(struct SIV_Instance_Record);
  instance->algorithm = algorithm;
  instance->key_set = 0;

  switch (algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      instance->min_nonce_length = SIV_MIN_NONCE_SIZE;
      instance->max_nonce_length = INT_MAX;
      instance->tag_length = SIV_DIGEST_SIZE;
      break;
#ifdef HAVE_NETTLE_SIV_GCM
    case AEAD_AES_128_GCM_SIV:
      instance->min_nonce_length = SIV_GCM_NONCE_SIZE;
      instance->max_nonce_length = SIV_GCM_NONCE_SIZE;
      instance->tag_length = SIV_GCM_DIGEST_SIZE;
      break;
#endif
    default:
      assert(0);
  }

  return instance;
}

/* ================================================== */

void
SIV_DestroyInstance(SIV_Instance instance)
{
  Free(instance);
}

/* ================================================== */

int
SIV_GetKeyLength(SIV_Algorithm algorithm)
{
  assert(2 * AES128_KEY_SIZE <= SIV_MAX_KEY_LENGTH);

  switch (algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      return 2 * AES128_KEY_SIZE;
#ifdef HAVE_NETTLE_SIV_GCM
    case AEAD_AES_128_GCM_SIV:
      return AES128_KEY_SIZE;
#endif
    default:
      return 0;
  }
}

/* ================================================== */

int
SIV_SetKey(SIV_Instance instance, const unsigned char *key, int length)
{
  if (length != SIV_GetKeyLength(instance->algorithm))
    return 0;

  switch (instance->algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      siv_cmac_aes128_set_key(&instance->ctx.cmac_aes128, key);
      break;
#ifdef HAVE_NETTLE_SIV_GCM
    case AEAD_AES_128_GCM_SIV:
      aes128_set_encrypt_key(&instance->ctx.aes128, key);
      break;
#endif
    default:
      assert(0);
  }

  instance->key_set = 1;

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
  BRIEF_ASSERT(instance->tag_length >= 1 && instance->tag_length <= SIV_MAX_TAG_LENGTH);
  return instance->tag_length;
}

/* ================================================== */

int
SIV_Encrypt(SIV_Instance instance,
            const unsigned char *nonce, int nonce_length,
            const void *assoc, int assoc_length,
            const void *plaintext, int plaintext_length,
            unsigned char *ciphertext, int ciphertext_length)
{
  if (!instance->key_set)
    return 0;

  if (nonce_length < instance->min_nonce_length ||
      nonce_length > instance->max_nonce_length || assoc_length < 0 ||
      plaintext_length < 0 || plaintext_length > ciphertext_length ||
      plaintext_length + SIV_GetTagLength(instance) != ciphertext_length)
    return 0;

  assert(assoc && plaintext);

  switch (instance->algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      siv_cmac_aes128_encrypt_message(&instance->ctx.cmac_aes128,
                                      nonce_length, nonce, assoc_length, assoc,
                                      ciphertext_length, ciphertext, plaintext);
      break;
#ifdef HAVE_NETTLE_SIV_GCM
    case AEAD_AES_128_GCM_SIV:
      siv_gcm_aes128_encrypt_message(&instance->ctx.aes128,
                                     nonce_length, nonce, assoc_length, assoc,
                                     ciphertext_length, ciphertext, plaintext);
      break;
#endif
    default:
      assert(0);
  }

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
  if (!instance->key_set)
    return 0;

  if (nonce_length < instance->min_nonce_length ||
      nonce_length > instance->max_nonce_length || assoc_length < 0 ||
      plaintext_length < 0 || plaintext_length > ciphertext_length ||
      plaintext_length + SIV_GetTagLength(instance) != ciphertext_length)
    return 0;

  assert(assoc && plaintext);

  switch (instance->algorithm) {
    case AEAD_AES_SIV_CMAC_256:
      if (!siv_cmac_aes128_decrypt_message(&instance->ctx.cmac_aes128,
                                           nonce_length, nonce, assoc_length, assoc,
                                           plaintext_length, plaintext, ciphertext))
        return 0;
      break;
#ifdef HAVE_NETTLE_SIV_GCM
    case AEAD_AES_128_GCM_SIV:
      if (!siv_gcm_aes128_decrypt_message(&instance->ctx.aes128,
                                          nonce_length, nonce, assoc_length, assoc,
                                          plaintext_length, plaintext, ciphertext))
        return 0;
      break;
#endif
    default:
      assert(0);
  }

  return 1;
}
