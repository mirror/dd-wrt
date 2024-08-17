/* ====================================================================
 * Copyright (c) 1998-2008 The OpenSSL Project.  All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */



/* 
   This file has utility functions used internally by cvm api's.
   These are not part of any API's
 */

#ifdef OCTEON_OPENSSL
#include <string.h>
#include <assert.h>

#include "cvmx-address.h"
#include "cvmx-asm.h"
#include "cvmx-key.h"
#include "cvmx-swap.h"

#include "openssl/aes.h"
#include <openssl/crypto.h>


void
cvm_crypto_aes_initialize (uint64_t * key, uint32_t key_len)
{
  CVMX_MT_AES_KEY (key[0], 0);
  CVMX_MT_AES_KEY (key[1], 1);
  CVMX_MT_AES_KEY (key[2], 2);
  CVMX_MT_AES_KEY (key[3], 3);
  CVMX_MT_AES_KEYLENGTH (key_len / 64 - 1);
}

void
cvm_crypto_aes_encrypt (uint64_t * data, uint32_t data_len)
{

  while (data_len) {
    CVMX_MT_AES_ENC0 (*data);
    CVMX_MT_AES_ENC1 (*(data + 1));
    CVMX_MF_AES_RESULT (*data++, 0);
    CVMX_MF_AES_RESULT (*data++, 1);
    data_len -= 16;
  }
}

void
cvm_crypto_aes_decrypt (uint64_t * data, uint32_t data_len)
{

  while (data_len) {
    CVMX_MT_AES_DEC0 (*data);
    CVMX_MT_AES_DEC1 (*(data + 1));
    CVMX_MF_AES_RESULT (*data++, 0);
    CVMX_MF_AES_RESULT (*data++, 1);
    data_len -= 16;
  }
}

void
cvm_crypto_aes_encrypt_cbc (uint64_t * iv, uint64_t * data,
  uint32_t data_len)
{

  CVMX_MT_AES_IV (iv[0], 0);
  CVMX_MT_AES_IV (iv[1], 1);

  while (data_len) {
    CVMX_MT_AES_ENC_CBC0 (*data);
    CVMX_MT_AES_ENC_CBC1 (*(data + 1));
    CVMX_MF_AES_RESULT (*data++, 0);
    CVMX_MF_AES_RESULT (*data++, 1);
    data_len -= 16;
  }
}

void
cvm_crypto_aes_decrypt_cbc (uint64_t * iv, uint64_t * data,
  uint32_t data_len)
{

  CVMX_MT_AES_IV (iv[0], 0);
  CVMX_MT_AES_IV (iv[1], 1);

  while (data_len) {
    CVMX_MT_AES_DEC_CBC0 (*data);
    CVMX_MT_AES_DEC_CBC1 (*(data + 1));
    CVMX_MF_AES_RESULT (*data++, 0);
    CVMX_MF_AES_RESULT (*data++, 1);
    data_len -= 16;
  }
}

//References    
//RFC3566
//http://csrc.nist.gov/CryptoToolkit/modes/proposedmodes/xcbc-mac/xcbc-mac-spec.pdf
//compliant to NIST xcbc-mac-spec.pdf rather than RFC3566
int
cvm_crypto_aes_xcbc_mac_nist_generic (uint64_t * key,
  uint32_t orgkeylen,
  uint64_t * const1A,
  uint64_t * const1B,
  uint64_t * const2,
  uint64_t * const3, uint64_t * data, uint32_t dlen, uint64_t * mac)
{
  uint32_t keylen = orgkeylen * 8;
  aes_assert (((keylen & 0xf) == 0), -1);
  aes_assert ((key != NULL), -1);
  aes_assert ((mac != NULL), -1);
  aes_assert ((data != NULL), -1);
  aes_assert ((keylen <= 256), -1);

  CVMX_MT_AES_KEY (key[0], 0);
  CVMX_MT_AES_KEY (key[1], 1);
  CVMX_MT_AES_KEY (key[2], 2);
  CVMX_MT_AES_KEY (key[3], 3);

  CVMX_MT_AES_KEYLENGTH (keylen / 64 - 1);

  uint32_t nblocks = dlen / AES_BLOCK_SIZE;
  uint32_t last_block_len = dlen % AES_BLOCK_SIZE;
  if ((!last_block_len) && (nblocks))
    last_block_len = AES_BLOCK_SIZE;


  uint64_t iv[2] = { 0, 0 };

  //key sizes upto 256 are supported
  uint64_t k1[2 * AES_BLOCK_SIZE / sizeof (uint64_t)];
  uint64_t k1a[AES_BLOCK_SIZE / sizeof (uint64_t)];
  uint64_t k1b[AES_BLOCK_SIZE / sizeof (uint64_t)];
  uint64_t k2[AES_BLOCK_SIZE / sizeof (uint64_t)];
  uint64_t k3[AES_BLOCK_SIZE / sizeof (uint64_t)];

  memset (k1, 0 ,2 * AES_BLOCK_SIZE / sizeof (uint64_t));
  memcpy (k1a, const1A, 16);
  memcpy (k1b, const1B, 16);
  memcpy (k2, const2, 16);
  memcpy (k3, const3, 16);


  //Derive 3 128bit keys from given key K
  cvm_octeon_crypto_aes_encrypt_cbc (key, keylen, iv, k1a, sizeof (k1a));
  if (keylen > 16) {
    cvm_octeon_crypto_aes_encrypt_cbc (key, keylen, iv, k1b, sizeof (k1b));
    memcpy (k1, k1a, sizeof (k1a));
    memcpy ((uint8_t *) k1 + sizeof (k1a), k1b, sizeof (k1b));
  } else {
    memcpy (k1, k1a, sizeof (k1a));
  }

  cvm_octeon_crypto_aes_encrypt_cbc (key, keylen, iv, k2, sizeof (k2));
  cvm_octeon_crypto_aes_encrypt_cbc (key, keylen, iv, k3, sizeof (k3));

  uint64_t oldE[2];
  uint64_t newE[2];
  memset (oldE, 0, sizeof (oldE));
  memset (newE, 0, sizeof (newE));

  uint32_t block;
  uint64_t tmp[2];


  for (block = 1;
    ((dlen % AES_BLOCK_SIZE) ? (block <= nblocks) : (block < nblocks));
    block++) {
    //As per RFC E[i] = EK(M[i] xor E[i-1]);
    tmp[0] = data[2 * (block - 1)] ^ oldE[0];
    tmp[1] = data[2 * (block - 1) + 1] ^ oldE[1];
    cvm_octeon_crypto_aes_encrypt_cbc (k1, keylen, iv, tmp, sizeof (tmp));
    newE[0] = oldE[0] = tmp[0];
    newE[1] = oldE[1] = tmp[1];
  }

  //for block=n handle
  if (last_block_len == AES_BLOCK_SIZE) {
    tmp[0] = data[2 * (block - 1)] ^ newE[0];
    tmp[1] = data[2 * (block - 1) + 1] ^ newE[1];

    tmp[0] ^= k2[0];
    tmp[1] ^= k2[1];

    cvm_octeon_crypto_aes_encrypt_cbc (k1, keylen, iv, tmp, sizeof (tmp));
    newE[0] = tmp[0];
    newE[1] = tmp[1];

    mac[0] = tmp[0];
    mac[1] = tmp[1];
  } else {
    uint8_t tmp16[16];
    uint64_t tmp[2];
    memcpy (tmp16, &data[2 * (block - 1)], last_block_len);
    tmp16[last_block_len] = 0x80;
    memset (tmp16 + last_block_len + 1, 0,
      AES_BLOCK_SIZE - last_block_len - 1);


    tmp[0] = ((uint64_t *) tmp16)[0] ^ newE[0];
    tmp[1] = ((uint64_t *) tmp16)[1] ^ newE[1];

    tmp[0] ^= k3[0];
    tmp[1] ^= k3[1];

    cvm_octeon_crypto_aes_encrypt_cbc (k1, keylen, iv, tmp, sizeof (tmp));

    mac[0] = tmp[0];
    mac[1] = tmp[1];
  }
  return 0;
}


//encrypt a single block
void
cvm_AES_encrypt (const unsigned char *in, unsigned char *out,
  const AES_KEY * key)
{
  uint64_t *in64 = (uint64_t *) in;
  uint64_t *out64 = (uint64_t *) out;

  CVMX_MT_AES_KEY (key->cvmkey[0], 0);
  CVMX_MT_AES_KEY (key->cvmkey[1], 1);
  CVMX_MT_AES_KEY (key->cvmkey[2], 2);
  CVMX_MT_AES_KEY (key->cvmkey[3], 3);
  CVMX_MT_AES_KEYLENGTH (key->cvm_keylen / 64 - 1);

  CVMX_MT_AES_ENC0 (*in64);
  CVMX_MT_AES_ENC1 (*(in64 + 1));
  CVMX_MF_AES_RESULT (*out64, 0);
  CVMX_MF_AES_RESULT (*(out64 + 1), 1);
}

#endif
