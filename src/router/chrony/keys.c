/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2012-2016, 2019-2020
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

  Module for managing keys used for authenticating NTP packets and commands

  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "keys.h"
#include "cmac.h"
#include "cmdparse.h"
#include "conf.h"
#include "memory.h"
#include "util.h"
#include "local.h"
#include "logging.h"

/* Consider 80 bits as the absolute minimum for a secure key */
#define MIN_SECURE_KEY_LENGTH 10

typedef enum {
  NTP_MAC,
  CMAC,
} KeyClass;

typedef struct {
  uint32_t id;
  int type;
  int length;
  KeyClass class;
  union {
    struct {
      unsigned char *value;
      int hash_id;
    } ntp_mac;
    CMC_Instance cmac;
  } data;
} Key;

static ARR_Instance keys;

static int cache_valid;
static uint32_t cache_key_id;
static int cache_key_pos;

/* ================================================== */

static void
free_keys(void)
{
  unsigned int i;
  Key *key;

  for (i = 0; i < ARR_GetSize(keys); i++) {
    key = ARR_GetElement(keys, i);
    switch (key->class) {
      case NTP_MAC:
        Free(key->data.ntp_mac.value);
        break;
      case CMAC:
        CMC_DestroyInstance(key->data.cmac);
        break;
      default:
        assert(0);
    }
  }

  ARR_SetSize(keys, 0);
  cache_valid = 0;
}

/* ================================================== */

void
KEY_Initialise(void)
{
  keys = ARR_CreateInstance(sizeof (Key));
  cache_valid = 0;
  KEY_Reload();
}

/* ================================================== */

void
KEY_Finalise(void)
{
  free_keys();
  ARR_DestroyInstance(keys);
}

/* ================================================== */

static Key *
get_key(unsigned int index)
{
  return ((Key *)ARR_GetElements(keys)) + index;
}

/* ================================================== */
/* Decode key encoded in ASCII or HEX */

static int
decode_key(char *key)
{
  int len = strlen(key);

  if (!strncmp(key, "ASCII:", 6)) {
    memmove(key, key + 6, len - 6);
    return len - 6;
  } else if (!strncmp(key, "HEX:", 4)) {
    return UTI_HexToBytes(key + 4, key, len);
  } else {
    /* assume ASCII */
    return len;
  }
}

/* ================================================== */

/* Compare two keys */

static int
compare_keys_by_id(const void *a, const void *b)
{
  const Key *c = (const Key *) a;
  const Key *d = (const Key *) b;

  if (c->id < d->id) {
    return -1;
  } else if (c->id > d->id) {
    return +1;
  } else {
    return 0;
  }

}

/* ================================================== */

void
KEY_Reload(void)
{
  unsigned int i, line_number, key_length, cmac_key_length;
  FILE *in;
  char line[2048], *key_file, *key_value;
  const char *key_type;
  HSH_Algorithm hash_algorithm;
  CMC_Algorithm cmac_algorithm;
  int hash_id;
  Key key;

  free_keys();

  key_file = CNF_GetKeysFile();
  line_number = 0;

  if (!key_file)
    return;

  if (!UTI_CheckFilePermissions(key_file, 0771))
    ;

  in = UTI_OpenFile(NULL, key_file, NULL, 'r', 0);
  if (!in) {
    LOG(LOGS_WARN, "Could not open keyfile %s", key_file);
    return;
  }

  while (fgets(line, sizeof (line), in)) {
    line_number++;

    CPS_NormalizeLine(line);
    if (!*line)
      continue;

    memset(&key, 0, sizeof (key));

    if (!CPS_ParseKey(line, &key.id, &key_type, &key_value)) {
      LOG(LOGS_WARN, "Could not parse key at line %u in file %s", line_number, key_file);
      continue;
    }

    key_length = decode_key(key_value);
    if (key_length == 0) {
      LOG(LOGS_WARN, "Could not decode key %"PRIu32, key.id);
      continue;
    }

    hash_algorithm = UTI_HashNameToAlgorithm(key_type);
    cmac_algorithm = UTI_CmacNameToAlgorithm(key_type);

    if (hash_algorithm != 0) {
      hash_id = HSH_GetHashId(hash_algorithm);
      if (hash_id < 0) {
        LOG(LOGS_WARN, "Unsupported %s in key %"PRIu32, "hash function", key.id);
        continue;
      }
      key.class = NTP_MAC;
      key.type = hash_algorithm;
      key.length = key_length;
      key.data.ntp_mac.value = MallocArray(unsigned char, key_length);
      memcpy(key.data.ntp_mac.value, key_value, key_length);
      key.data.ntp_mac.hash_id = hash_id;
    } else if (cmac_algorithm != 0) {
      cmac_key_length = CMC_GetKeyLength(cmac_algorithm);
      if (cmac_key_length == 0) {
        LOG(LOGS_WARN, "Unsupported %s in key %"PRIu32, "cipher", key.id);
        continue;
      } else if (cmac_key_length != key_length) {
        LOG(LOGS_WARN, "Invalid length of %s key %"PRIu32" (expected %u bits)",
            key_type, key.id, 8 * cmac_key_length);
        continue;
      }

      key.class = CMAC;
      key.type = cmac_algorithm;
      key.length = key_length;
      key.data.cmac = CMC_CreateInstance(cmac_algorithm, (unsigned char *)key_value,
                                         key_length);
      assert(key.data.cmac);
    } else {
      LOG(LOGS_WARN, "Invalid type in key %"PRIu32, key.id);
      continue;
    }

    ARR_AppendElement(keys, &key);
  }

  fclose(in);

  /* Sort keys into order.  Note, if there's a duplicate, it is
     arbitrary which one we use later - the user should have been
     more careful! */
  qsort(ARR_GetElements(keys), ARR_GetSize(keys), sizeof (Key), compare_keys_by_id);

  LOG(LOGS_INFO, "Loaded %u symmetric keys", ARR_GetSize(keys));

  /* Check for duplicates */
  for (i = 1; i < ARR_GetSize(keys); i++) {
    if (get_key(i - 1)->id == get_key(i)->id)
      LOG(LOGS_WARN, "Detected duplicate key %"PRIu32, get_key(i - 1)->id);
  }

  /* Erase any passwords from stack */
  memset(line, 0, sizeof (line));
}

/* ================================================== */

static int
lookup_key(uint32_t id)
{
  Key specimen, *where, *keys_ptr;
  int pos;

  keys_ptr = ARR_GetElements(keys);
  specimen.id = id;
  where = (Key *)bsearch((void *)&specimen, keys_ptr, ARR_GetSize(keys),
                         sizeof (Key), compare_keys_by_id);
  if (!where) {
    return -1;
  } else {
    pos = where - keys_ptr;
    return pos;
  }
}

/* ================================================== */

static Key *
get_key_by_id(uint32_t key_id)
{
  int position;

  if (cache_valid && key_id == cache_key_id)
    return get_key(cache_key_pos);

  position = lookup_key(key_id);

  if (position >= 0) {
    cache_valid = 1;
    cache_key_pos = position;
    cache_key_id = key_id;

    return get_key(position);
  }

  return NULL;
}

/* ================================================== */

int
KEY_KeyKnown(uint32_t key_id)
{
  return get_key_by_id(key_id) != NULL;
}

/* ================================================== */

int
KEY_GetAuthLength(uint32_t key_id)
{
  unsigned char buf[MAX_HASH_LENGTH];
  Key *key;

  key = get_key_by_id(key_id);

  if (!key)
    return 0;

  switch (key->class) {
    case NTP_MAC:
      return HSH_Hash(key->data.ntp_mac.hash_id, buf, 0, buf, 0, buf, sizeof (buf));
    case CMAC:
      return CMC_Hash(key->data.cmac, buf, 0, buf, sizeof (buf));
    default:
      assert(0);
      return 0;
  }
}

/* ================================================== */

int
KEY_CheckKeyLength(uint32_t key_id)
{
  Key *key;

  key = get_key_by_id(key_id);

  if (!key)
    return 0;

  return key->length >= MIN_SECURE_KEY_LENGTH;
}

/* ================================================== */

int
KEY_GetKeyInfo(uint32_t key_id, int *type, int *bits)
{
  Key *key;

  key = get_key_by_id(key_id);

  if (!key)
    return 0;

  *type = key->type;
  *bits = 8 * key->length;

  return 1;
}

/* ================================================== */

static int
generate_auth(Key *key, const void *data, int data_len, unsigned char *auth, int auth_len)
{
  switch (key->class) {
    case NTP_MAC:
      return HSH_Hash(key->data.ntp_mac.hash_id, key->data.ntp_mac.value,
                      key->length, data, data_len, auth, auth_len);
    case CMAC:
      return CMC_Hash(key->data.cmac, data, data_len, auth, auth_len);
    default:
      return 0;
  }
}

/* ================================================== */

static int
check_auth(Key *key, const void *data, int data_len,
           const unsigned char *auth, int auth_len, int trunc_len)
{
  unsigned char buf[MAX_HASH_LENGTH];
  int hash_len;

  hash_len = generate_auth(key, data, data_len, buf, sizeof (buf));

  return MIN(hash_len, trunc_len) == auth_len && UTI_IsMemoryEqual(buf, auth, auth_len);
}

/* ================================================== */

int
KEY_GenerateAuth(uint32_t key_id, const void *data, int data_len,
                 unsigned char *auth, int auth_len)
{
  Key *key;

  key = get_key_by_id(key_id);

  if (!key)
    return 0;

  return generate_auth(key, data, data_len, auth, auth_len);
}

/* ================================================== */

int
KEY_CheckAuth(uint32_t key_id, const void *data, int data_len,
              const unsigned char *auth, int auth_len, int trunc_len)
{
  Key *key;

  key = get_key_by_id(key_id);

  if (!key)
    return 0;

  return check_auth(key, data, data_len, auth, auth_len, trunc_len);
}
