/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017
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
 */

#include <keys.c>
#include "test.h"

#define KEYS 100
#define KEYFILE "keys.test-keys"

static
uint32_t write_random_key(FILE *f)
{
  const char *hash_name;
  char key[128];
  uint32_t id;
  int i, length;

  length = random() % sizeof (key) + 1;
  length = MAX(length, 4);
  UTI_GetRandomBytes(&id, sizeof (id));
  UTI_GetRandomBytes(key, length);

  switch (random() % 6) {
#ifdef FEAT_SECHASH
    case 0:
      hash_name = "SHA1";
      break;
    case 1:
      hash_name = "SHA256";
      break;
    case 2:
      hash_name = "SHA384";
      break;
    case 3:
      hash_name = "SHA512";
      break;
#endif
    case 4:
      hash_name = "MD5";
      break;
    default:
      hash_name = "";
  }

  fprintf(f, "%u %s %s", id, hash_name, random() % 2 ? "HEX:" : "");
  for (i = 0; i < length; i++)
    fprintf(f, "%02hhX", key[i]);
  fprintf(f, "\n");

  return id;
}

static void
generate_key_file(const char *name, uint32_t *keys)
{
  FILE *f;
  int i;

  f = fopen(name, "w");
  TEST_CHECK(f);
  for (i = 0; i < KEYS; i++)
    keys[i] = write_random_key(f);
  fclose(f);
}

void
test_unit(void)
{
  int i, j, data_len, auth_len;
  uint32_t keys[KEYS], key;
  unsigned char data[100], auth[MAX_HASH_LENGTH];
  char conf[][100] = {
    "keyfile "KEYFILE
  };

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);

  generate_key_file(KEYFILE, keys);
  KEY_Initialise();

  for (i = 0; i < 100; i++) {
    DEBUG_LOG("iteration %d", i);

    if (i) {
      generate_key_file(KEYFILE, keys);
      KEY_Reload();
    }

    UTI_GetRandomBytes(data, sizeof (data));

    for (j = 0; j < KEYS; j++) {
      TEST_CHECK(KEY_KeyKnown(keys[j]));
      TEST_CHECK(KEY_GetAuthDelay(keys[j]) >= 0);
      TEST_CHECK(KEY_GetAuthLength(keys[j]) >= 16);

      data_len = random() % (sizeof (data) + 1);
      auth_len = KEY_GenerateAuth(keys[j], data, data_len, auth, sizeof (auth));
      TEST_CHECK(auth_len >= 16);

      TEST_CHECK(KEY_CheckAuth(keys[j], data, data_len, auth, auth_len, auth_len));

      if (j > 0 && keys[j - 1] != keys[j])
        TEST_CHECK(!KEY_CheckAuth(keys[j - 1], data, data_len, auth, auth_len, auth_len));

      auth_len = random() % auth_len + 1;
      if (auth_len < MAX_HASH_LENGTH)
        auth[auth_len]++;
      TEST_CHECK(KEY_CheckAuth(keys[j], data, data_len, auth, auth_len, auth_len));

      auth[auth_len - 1]++;
      TEST_CHECK(!KEY_CheckAuth(keys[j], data, data_len, auth, auth_len, auth_len));
    }

    for (j = 0; j < 1000; j++) {
      UTI_GetRandomBytes(&key, sizeof (key));
      if (KEY_KeyKnown(key))
        continue;
      TEST_CHECK(!KEY_GenerateAuth(key, data, data_len, auth, sizeof (auth)));
      TEST_CHECK(!KEY_CheckAuth(key, data, data_len, auth, auth_len, auth_len));
    }
  }

  unlink(KEYFILE);

  KEY_Finalise();
  CNF_Finalise();
  HSH_Finalise();
}
