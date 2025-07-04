/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2021
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

  Crypto hashing using the GnuTLS library
  */

#include "config.h"

#include "sysincl.h"

#include <gnutls/crypto.h>

#include "hash.h"
#include "logging.h"

struct hash {
  const HSH_Algorithm algorithm;
  const gnutls_digest_algorithm_t type;
  gnutls_hash_hd_t handle;
};

static struct hash hashes[] = {
  { HSH_MD5_NONCRYPTO, GNUTLS_DIG_MD5, NULL },
  { HSH_MD5, GNUTLS_DIG_MD5, NULL },
  { HSH_SHA1, GNUTLS_DIG_SHA1, NULL },
  { HSH_SHA256, GNUTLS_DIG_SHA256, NULL },
  { HSH_SHA384, GNUTLS_DIG_SHA384, NULL },
  { HSH_SHA512, GNUTLS_DIG_SHA512, NULL },
  { HSH_SHA3_224, GNUTLS_DIG_SHA3_224, NULL },
  { HSH_SHA3_256, GNUTLS_DIG_SHA3_256, NULL },
  { HSH_SHA3_384, GNUTLS_DIG_SHA3_384, NULL },
  { HSH_SHA3_512, GNUTLS_DIG_SHA3_512, NULL },
  { 0, 0, NULL }
};

static int gnutls_initialised = 0;

int
HSH_GetHashId(HSH_Algorithm algorithm)
{
  int id, r;

  if (!gnutls_initialised) {
    r = gnutls_global_init();
    if (r < 0)
      LOG_FATAL("Could not initialise %s : %s", "gnutls", gnutls_strerror(r));
    gnutls_initialised = 1;
  }

  for (id = 0; hashes[id].algorithm != 0; id++) {
    if (hashes[id].algorithm == algorithm)
      break;
  }

  if (hashes[id].algorithm == 0)
    return -1;

  if (hashes[id].handle)
    return id;

  if (algorithm == HSH_MD5_NONCRYPTO)
    GNUTLS_FIPS140_SET_LAX_MODE();

  r = gnutls_hash_init(&hashes[id].handle, hashes[id].type);

  if (algorithm == HSH_MD5_NONCRYPTO)
    GNUTLS_FIPS140_SET_STRICT_MODE();

  if (r < 0) {
    DEBUG_LOG("Could not initialise %s : %s", "hash", gnutls_strerror(r));
    hashes[id].handle = NULL;
    return -1;
  }

  return id;
}

int
HSH_Hash(int id, const void *in1, int in1_len, const void *in2, int in2_len,
         unsigned char *out, int out_len)
{
  unsigned char buf[MAX_HASH_LENGTH];
  gnutls_hash_hd_t handle;
  int hash_len;

  if (in1_len < 0 || in2_len < 0 || out_len < 0)
    return 0;

  handle = hashes[id].handle;
  hash_len = gnutls_hash_get_len(hashes[id].type);

  if (out_len > hash_len)
    out_len = hash_len;

  if (hash_len > sizeof (buf))
    return 0;

  if (gnutls_hash(handle, in1, in1_len) < 0 ||
      (in2 && gnutls_hash(handle, in2, in2_len) < 0)) {
    /* Reset the state */
    gnutls_hash_output(handle, buf);
    return 0;
  }

  gnutls_hash_output(handle, buf);
  memcpy(out, buf, out_len);

  return out_len;
}

void
HSH_Finalise(void)
{
  int i;

  if (!gnutls_initialised)
    return;

  for (i = 0; hashes[i].algorithm != 0; i++) {
    if (hashes[i].handle)
      gnutls_hash_deinit(hashes[i].handle, NULL);
  }

  gnutls_global_deinit();
}
