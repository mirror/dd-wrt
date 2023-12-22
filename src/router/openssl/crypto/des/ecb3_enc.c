/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "des_local.h"

void DES_ecb3_encrypt(const_DES_cblock *input, DES_cblock *output,
                      DES_key_schedule *ks1, DES_key_schedule *ks2,
                      DES_key_schedule *ks3, int enc)
{
#ifdef OCTEON_OPENSSL
  uint64_t *rdkey1 = &ks1->ks->cblock[0];
  uint64_t *rdkey2 = &ks2->ks->cblock[0];
  uint64_t *rdkey3 = &ks3->ks->cblock[0];

  CVMX_MT_3DES_KEY (*rdkey1, 0);
  CVMX_MT_3DES_KEY (*rdkey2, 1);
  CVMX_MT_3DES_KEY (*rdkey3, 2);

  if (enc) {
      register uint64_t inp = *(uint64_t *)input;
      CVMX_MT_3DES_ENC (inp);
      CVMX_MF_3DES_RESULT (inp);
      *(uint64_t *)output = inp;
  } else {
      register uint64_t inp = *(uint64_t *)input;
      CVMX_MT_3DES_DEC (inp);
      CVMX_MF_3DES_RESULT (inp);
      *(uint64_t *)output = inp;
  }
#else
    register DES_LONG l0, l1;
    DES_LONG ll[2];
    const unsigned char *in = &(*input)[0];
    unsigned char *out = &(*output)[0];

    c2l(in, l0);
    c2l(in, l1);
    ll[0] = l0;
    ll[1] = l1;
    if (enc)
        DES_encrypt3(ll, ks1, ks2, ks3);
    else
        DES_decrypt3(ll, ks1, ks2, ks3);
    l0 = ll[0];
    l1 = ll[1];
    l2c(l0, out);
    l2c(l1, out);
#endif
}
