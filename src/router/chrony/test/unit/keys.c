/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2017, 2025
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

#include <config.h>
#include <local.h>
#include "test.h"

#include <keys.c>

#define KEYS 100
#define KEYFILE "keys.test-keys"
#define MIN_TIMING_INTERVAL 1.0e-3

static
uint32_t write_random_key(FILE *f)
{
  const char *type, *prefix;
  char key[128];
  uint32_t id;
  int i, length;

  length = random() % sizeof (key) + 1;
  length = MAX(length, 4);
  prefix = random() % 2 ? "HEX:" : "";

  switch (random() % 8) {
#ifdef FEAT_SECHASH
    case 0:
      type = "SHA1";
      break;
    case 1:
      type = "SHA256";
      break;
    case 2:
      type = "SHA384";
      break;
    case 3:
      type = "SHA512";
      break;
#endif
#ifdef HAVE_CMAC
    case 4:
      type = "AES128";
      length = prefix[0] == '\0' ? 8 : 16;
      break;
    case 5:
      type = "AES256";
      length = prefix[0] == '\0' ? 16 : 32;
      break;
#endif
    case 6:
      type = "MD5";
      break;
    default:
      type = "";
  }

  UTI_GetRandomBytes(&id, sizeof (id));
  UTI_GetRandomBytes(key, length);

  fprintf(f, "%u %s %s", id, type, prefix);
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
  int i, j, data_len, auth_len, type, bits, s, timing_fails, timing_iters;
  uint32_t keys[KEYS], key;
  unsigned char data[100], auth[MAX_HASH_LENGTH], auth2[MAX_HASH_LENGTH];
  struct timespec ts1, ts2;
  double diff1, diff2;
  char conf[][100] = {
    "keyfile "KEYFILE
  };

  CNF_Initialise(0, 0);
  for (i = 0; i < sizeof conf / sizeof conf[0]; i++)
    CNF_ParseLine(NULL, i + 1, conf[i]);
  LCL_Initialise();

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

      TEST_CHECK(KEY_GetKeyInfo(keys[j], &type, &bits));
      TEST_CHECK(type > 0 && bits > 0);
    }

    for (j = 0; j < 1000; j++) {
      UTI_GetRandomBytes(&key, sizeof (key));
      if (KEY_KeyKnown(key))
        continue;
      TEST_CHECK(!KEY_GetKeyInfo(key, &type, &bits));
      TEST_CHECK(!KEY_GenerateAuth(key, data, data_len, auth, sizeof (auth)));
      TEST_CHECK(!KEY_CheckAuth(key, data, data_len, auth, auth_len, auth_len));
    }
  }

  if (!getenv("NO_TIMING_TESTS") &&
      LCL_GetSysPrecisionAsQuantum() < MIN_TIMING_INTERVAL / 100.0) {
    auth_len = sizeof (auth);
    UTI_GetRandomBytes(auth, auth_len);
    memcpy(auth2, auth, auth_len);

    timing_fails = 0;
    timing_iters = 1000;

    i = 0;
    for (i = 0; i < 100; i++) {
      int d = random() % 2;

      auth2[0] = auth[0] + d;

      for (j = s = 0; j < timing_iters; j++) {
        if (j == 100)
          LCL_ReadRawTime(&ts1);
        s += UTI_IsMemoryEqual(auth, auth2, auth_len);
      }
      LCL_ReadRawTime(&ts2);
      TEST_CHECK(s == (d + 1) % 2 * timing_iters);
      diff1 = UTI_DiffTimespecsToDouble(&ts2, &ts1);

      auth2[0] = auth[0] + (d + 1) % 2;

      for (j = s = 0; j < timing_iters; j++) {
        if (j == 100)
          LCL_ReadRawTime(&ts1);
        s += UTI_IsMemoryEqual(auth, auth2, auth_len);
      }
      LCL_ReadRawTime(&ts2);
      TEST_CHECK(s == d * timing_iters);
      diff2 = UTI_DiffTimespecsToDouble(&ts2, &ts1);

      DEBUG_LOG("d=%d diff1=%e diff2=%e iters=%d", d, diff1, diff2, timing_iters);

      if (diff1 < MIN_TIMING_INTERVAL && diff2 < MIN_TIMING_INTERVAL) {
        if (timing_iters >= INT_MAX / 2)
          break;
        timing_iters *= 2;
        i--;
        continue;
      }

      if ((d == 0 && 0.8 * diff1 > diff2) || (d == 1 && diff1 < 0.8 * diff2))
        timing_fails++;
    }

    DEBUG_LOG("timing fails %d/%d", timing_fails, i);
    TEST_CHECK(timing_fails < i / 2);
  }

  unlink(KEYFILE);

  KEY_Finalise();
  LCL_Finalise();
  CNF_Finalise();
  HSH_Finalise();
}
