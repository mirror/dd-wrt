/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2018
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

  Routines implementing crypto hashing using the nettle library.

  */

#include "config.h"

#include "sysincl.h"

#include <nettle/nettle-meta.h>

#include "hash.h"
#include "memory.h"

struct hash {
  const HSH_Algorithm algorithm;
  const char *int_name;
  const struct nettle_hash *nettle_hash;
  void *context;
};

static struct hash hashes[] = {
  { HSH_MD5, "md5", NULL, NULL },
  { HSH_SHA1, "sha1", NULL, NULL },
  { HSH_SHA256, "sha256", NULL, NULL },
  { HSH_SHA384, "sha384", NULL, NULL },
  { HSH_SHA512, "sha512", NULL, NULL },
  { HSH_SHA3_224, "sha3_224", NULL, NULL },
  { HSH_SHA3_256, "sha3_256", NULL, NULL },
  { HSH_SHA3_384, "sha3_384", NULL, NULL },
  { HSH_SHA3_512, "sha3_512", NULL, NULL },
  { 0, NULL, NULL, NULL }
};

int
HSH_GetHashId(HSH_Algorithm algorithm)
{
  int id, nid;

  if (algorithm == HSH_MD5_NONCRYPTO)
    algorithm = HSH_MD5;

  for (id = 0; hashes[id].algorithm != 0; id++) {
    if (hashes[id].algorithm == algorithm)
      break;
  }

  if (hashes[id].algorithm == 0)
    return -1;

  if (hashes[id].context)
    return id;

  for (nid = 0; nettle_hashes[nid]; nid++) {
    if (!strcmp(hashes[id].int_name, nettle_hashes[nid]->name))
      break;
  }

  if (!nettle_hashes[nid] || !nettle_hashes[nid]->context_size || !nettle_hashes[nid]->init)
    return -1;

  hashes[id].nettle_hash = nettle_hashes[nid];
  hashes[id].context = Malloc(hashes[id].nettle_hash->context_size);

  return id;
}

int
HSH_Hash(int id, const void *in1, int in1_len, const void *in2, int in2_len,
         unsigned char *out, int out_len)
{
  const struct nettle_hash *hash;
  void *context;

  if (in1_len < 0 || in2_len < 0 || out_len < 0)
    return 0;

  hash = hashes[id].nettle_hash;
  context = hashes[id].context;

  if (out_len > hash->digest_size)
    out_len = hash->digest_size;

  hash->init(context);
  hash->update(context, in1_len, in1);
  if (in2)
    hash->update(context, in2_len, in2);
  hash->digest(context, out_len, out);

  return out_len;
}

void
HSH_Finalise(void)
{
  int i;

  for (i = 0; hashes[i].algorithm != 0; i++) {
    if (hashes[i].context)
      Free(hashes[i].context);
  }
}
