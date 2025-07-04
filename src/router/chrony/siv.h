/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019
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

  Header file for Synthetic Initialization Vector (SIV) ciphers.

  */

#ifndef GOT_SIV_H
#define GOT_SIV_H

/* Maximum key length of all supported SIVs */
#define SIV_MAX_KEY_LENGTH 32

/* Maximum difference between lengths of ciphertext and plaintext */
#define SIV_MAX_TAG_LENGTH 16

/* Identifiers of SIV algorithms following the IANA AEAD registry */
typedef enum {
  AEAD_SIV_INVALID = 0,
  AEAD_AES_SIV_CMAC_256 = 15,
  AEAD_AES_SIV_CMAC_384 = 16,
  AEAD_AES_SIV_CMAC_512 = 17,
  AEAD_AES_128_GCM_SIV = 30,
  AEAD_AES_256_GCM_SIV = 31,
} SIV_Algorithm;

typedef struct SIV_Instance_Record *SIV_Instance;

extern SIV_Instance SIV_CreateInstance(SIV_Algorithm algorithm);

extern void SIV_DestroyInstance(SIV_Instance instance);

extern int SIV_GetKeyLength(SIV_Algorithm algorithm);

extern int SIV_SetKey(SIV_Instance instance, const unsigned char *key, int length);

extern int SIV_GetMinNonceLength(SIV_Instance instance);

extern int SIV_GetMaxNonceLength(SIV_Instance instance);

extern int SIV_GetTagLength(SIV_Instance instance);

extern int SIV_Encrypt(SIV_Instance instance,
                       const unsigned char *nonce, int nonce_length,
                       const void *assoc, int assoc_length,
                       const void *plaintext, int plaintext_length,
                       unsigned char *ciphertext, int ciphertext_length);

extern int SIV_Decrypt(SIV_Instance instance,
                       const unsigned char *nonce, int nonce_length,
                       const void *assoc, int assoc_length,
                       const unsigned char *ciphertext, int ciphertext_length,
                       void *plaintext, int plaintext_length);

#endif
