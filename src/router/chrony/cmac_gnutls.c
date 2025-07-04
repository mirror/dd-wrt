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

  CMAC using the GnuTLS library
  */

#include "config.h"

#include "sysincl.h"

#include <gnutls/crypto.h>

#include "cmac.h"
#include "hash.h"
#include "logging.h"
#include "memory.h"

struct CMC_Instance_Record {
  gnutls_mac_algorithm_t algorithm;
  gnutls_hmac_hd_t mac;
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

static gnutls_mac_algorithm_t
get_mac_algorithm(CMC_Algorithm algorithm)
{
  switch (algorithm) {
    case CMC_AES128:
      return GNUTLS_MAC_AES_CMAC_128;
    case CMC_AES256:
      return GNUTLS_MAC_AES_CMAC_256;
    default:
      return GNUTLS_MAC_UNKNOWN;
  }
}

/* ================================================== */

int
CMC_GetKeyLength(CMC_Algorithm algorithm)
{
  gnutls_mac_algorithm_t malgo = get_mac_algorithm(algorithm);
  int len;

  if (malgo == GNUTLS_MAC_UNKNOWN)
    return 0;

  len = gnutls_hmac_get_key_size(malgo);

  if (len < 0)
    return 0;

  return len;
}

/* ================================================== */

CMC_Instance
CMC_CreateInstance(CMC_Algorithm algorithm, const unsigned char *key, int length)
{
  gnutls_hmac_hd_t handle;
  CMC_Instance inst;

  int r;

  if (instance_counter == 0)
    init_gnutls();

  if (length <= 0 || length != CMC_GetKeyLength(algorithm))
    goto error;

  r = gnutls_hmac_init(&handle, get_mac_algorithm(algorithm), key, length);
  if (r < 0) {
    DEBUG_LOG("Could not initialise %s : %s", "mac", gnutls_strerror(r));
    goto error;
  }

  inst = MallocNew(struct CMC_Instance_Record);
  inst->algorithm = get_mac_algorithm(algorithm);
  inst->mac = handle;

  instance_counter++;

  return inst;

error:
  if (instance_counter == 0)
    deinit_gnutls();
  return NULL;
}

/* ================================================== */

int
CMC_Hash(CMC_Instance inst, const void *in, int in_len, unsigned char *out, int out_len)
{
  unsigned char buf[MAX_HASH_LENGTH];
  int hash_len;

  if (in_len < 0 || out_len < 0)
    return 0;

  hash_len = gnutls_hmac_get_len(inst->algorithm);

  if (out_len > hash_len)
    out_len = hash_len;

  if (hash_len > sizeof (buf))
    return 0;

  if (gnutls_hmac(inst->mac, in, in_len) < 0) {
    /* Reset the state */
    gnutls_hmac_output(inst->mac, buf);
    return 0;
  }

  gnutls_hmac_output(inst->mac, buf);
  memcpy(out, buf, out_len);

  return out_len;
}

/* ================================================== */

void
CMC_DestroyInstance(CMC_Instance inst)
{
  gnutls_hmac_deinit(inst->mac, NULL);
  Free(inst);

  instance_counter--;
  if (instance_counter == 0)
    deinit_gnutls();
}
