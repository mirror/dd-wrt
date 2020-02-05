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
  const char *name;
  const char *int_name;
  const struct ltc_hash_descriptor *desc;
};

static const struct hash hashes[] = {
  { "MD5", "md5", &md5_desc },
#ifdef LTC_RIPEMD128
  { "RMD128", "rmd128", &rmd128_desc },
#endif
#ifdef LTC_RIPEMD160
  { "RMD160", "rmd160", &rmd160_desc },
#endif
#ifdef LTC_RIPEMD256
  { "RMD256", "rmd256", &rmd256_desc },
#endif
#ifdef LTC_RIPEMD320
  { "RMD320", "rmd320", &rmd320_desc },
#endif
#ifdef LTC_SHA1
  { "SHA1", "sha1", &sha1_desc },
#endif
#ifdef LTC_SHA256
  { "SHA256", "sha256", &sha256_desc },
#endif
#ifdef LTC_SHA384
  { "SHA384", "sha384", &sha384_desc },
#endif
#ifdef LTC_SHA512
  { "SHA512", "sha512", &sha512_desc },
#endif
#ifdef LTC_SHA3
  { "SHA3-224", "sha3-224", &sha3_224_desc },
  { "SHA3-256", "sha3-256", &sha3_256_desc },
  { "SHA3-384", "sha3-384", &sha3_384_desc },
  { "SHA3-512", "sha3-512", &sha3_512_desc },
#endif
#ifdef LTC_TIGER
  { "TIGER", "tiger", &tiger_desc },
#endif
#ifdef LTC_WHIRLPOOL
  { "WHIRLPOOL", "whirlpool", &whirlpool_desc },
#endif
  { NULL, NULL, NULL }
};

int
HSH_GetHashId(const char *name)
{
  int i, h;

  for (i = 0; hashes[i].name; i++) {
    if (!strcmp(name, hashes[i].name))
      break;
  }

  if (!hashes[i].name)
    return -1; /* not found */

  h = find_hash(hashes[i].int_name);
  if (h >= 0)
    return h; /* already registered */
  
  /* register and try again */
  register_hash(hashes[i].desc);

  return find_hash(hashes[i].int_name);
}

unsigned int
HSH_Hash(int id, const unsigned char *in1, unsigned int in1_len,
    const unsigned char *in2, unsigned int in2_len,
    unsigned char *out, unsigned int out_len)
{
  unsigned char buf[MAX_HASH_LENGTH];
  unsigned long len;
  int r;

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
