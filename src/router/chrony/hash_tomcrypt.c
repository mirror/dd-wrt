/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2012, 2018
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

  Routines implementing crypto hashing using tomcrypt library.

  */

#include <tomcrypt.h>

#include "config.h"
#include "hash.h"
#include "util.h"

struct hash {
  HSH_Algorithm algorithm;
  const char *int_name;
  const struct ltc_hash_descriptor *desc;
};

static const struct hash hashes[] = {
  { HSH_MD5, "md5", &md5_desc },
#ifdef LTC_SHA1
  { HSH_SHA1, "sha1", &sha1_desc },
#endif
#ifdef LTC_SHA256
  { HSH_SHA256, "sha256", &sha256_desc },
#endif
#ifdef LTC_SHA384
  { HSH_SHA384, "sha384", &sha384_desc },
#endif
#ifdef LTC_SHA512
  { HSH_SHA512, "sha512", &sha512_desc },
#endif
#ifdef LTC_SHA3
  { HSH_SHA3_224, "sha3-224", &sha3_224_desc },
  { HSH_SHA3_256, "sha3-256", &sha3_256_desc },
  { HSH_SHA3_384, "sha3-384", &sha3_384_desc },
  { HSH_SHA3_512, "sha3-512", &sha3_512_desc },
#endif
#ifdef LTC_TIGER
  { HSH_TIGER, "tiger", &tiger_desc },
#endif
#ifdef LTC_WHIRLPOOL
  { HSH_WHIRLPOOL, "whirlpool", &whirlpool_desc },
#endif
  { 0, NULL, NULL }
};

int
HSH_GetHashId(HSH_Algorithm algorithm)
{
  int i, h;

  if (algorithm == HSH_MD5_NONCRYPTO)
    algorithm = HSH_MD5;

  for (i = 0; hashes[i].algorithm != 0; i++) {
    if (hashes[i].algorithm == algorithm)
      break;
  }

  if (hashes[i].algorithm == 0)
    return -1; /* not found */

  h = find_hash(hashes[i].int_name);
  if (h >= 0)
    return h; /* already registered */
  
  /* register and try again */
  register_hash(hashes[i].desc);

  return find_hash(hashes[i].int_name);
}

int
HSH_Hash(int id, const void *in1, int in1_len, const void *in2, int in2_len,
         unsigned char *out, int out_len)
{
  unsigned char buf[MAX_HASH_LENGTH];
  unsigned long len;
  int r;

  if (in1_len < 0 || in2_len < 0 || out_len < 0)
    return 0;

  len = sizeof (buf);
  if (in2)
    r = hash_memory_multi(id, buf, &len,
                          in1, (unsigned long)in1_len,
                          in2, (unsigned long)in2_len, NULL, 0);
  else
    r = hash_memory(id, in1, in1_len, buf, &len);

  if (r != CRYPT_OK)
    return 0;

  len = MIN(len, out_len);
  memcpy(out, buf, len);

  return len;
}

void
HSH_Finalise(void)
{
}
