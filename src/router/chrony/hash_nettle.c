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
  const char *name;
  const char *int_name;
  const struct nettle_hash *nettle_hash;
  void *context;
};

static struct hash hashes[] = {
  { "MD5", "md5", NULL, NULL },
  { "RMD160", "ripemd160", NULL, NULL },
  { "SHA1", "sha1", NULL, NULL },
  { "SHA256", "sha256", NULL, NULL },
  { "SHA384", "sha384", NULL, NULL },
  { "SHA512", "sha512", NULL, NULL },
  { "SHA3-224", "sha3_224", NULL, NULL },
  { "SHA3-256", "sha3_256", NULL, NULL },
  { "SHA3-384", "sha3_384", NULL, NULL },
  { "SHA3-512", "sha3_512", NULL, NULL },
  { NULL, NULL, NULL, NULL }
};

int
HSH_GetHashId(const char *name)
{
  int id, nid;

  for (id = 0; hashes[id].name; id++) {
    if (!strcmp(name, hashes[id].name))
      break;
  }

  if (!hashes[id].name)
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

unsigned int
HSH_Hash(int id, const unsigned char *in1, unsigned int in1_len,
         const unsigned char *in2, unsigned int in2_len,
         unsigned char *out, unsigned int out_len)
{
  const struct nettle_hash *hash;
  void *context;

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

  for (i = 0; hashes[i].name; i++) {
    if (hashes[i].context)
      Free(hashes[i].context);
  }
}
