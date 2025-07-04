/*
 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2019
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
#include <sysincl.h>
#include <cmac.h>
#include <logging.h>
#include <util.h>
#include "test.h"

#define MAX_KEY_LENGTH 64
#define MAX_HASH_LENGTH 64

struct cmac_test {
  const char *name;
  const unsigned char key[MAX_KEY_LENGTH];
  int key_length;
  const unsigned char hash[MAX_HASH_LENGTH];
  int hash_length;
};

void
test_unit(void)
{
  unsigned char data[] = "abcdefghijklmnopqrstuvwxyz0123456789";
  unsigned char hash[MAX_HASH_LENGTH];
  struct cmac_test tests[] = {
    { "AES128", "\xfc\x24\x97\x1b\x52\x66\xdc\x46\xef\xe0\xe8\x08\x46\x89\xb6\x88", 16,
                "\xaf\x3c\xfe\xc2\x66\x71\x08\x04\xd4\xaf\x5b\x16\x2b\x11\xf4\x85", 16 },
    { "AES256", "\x14\x02\x8e\x7d\x17\x3c\x2f\x4e\x17\x0f\x37\x96\xc3\x2c\xc5\x99"
                "\x18\xdd\x55\x23\xb7\xd7\x9b\xc5\x76\x36\x88\x3f\xc5\x82\xb5\x83", 32,
                "\xfe\xf7\x94\x96\x14\x04\x11\x0b\x87\xe4\xd4\x3f\x81\xb3\xb2\x2d", 16 },
    { "", "", 0, "", 0 }
  };

  CMC_Algorithm algorithm;
  CMC_Instance inst;
  int i, j, length;

#ifndef HAVE_CMAC
  TEST_REQUIRE(0);
#endif

  TEST_CHECK(CMC_INVALID == 0);

  for (i = 0; tests[i].name[0] != '\0'; i++) {
    algorithm = UTI_CmacNameToAlgorithm(tests[i].name);
    TEST_CHECK(algorithm != 0);
    TEST_CHECK(CMC_GetKeyLength(algorithm) == tests[i].key_length);

    DEBUG_LOG("testing %s", tests[i].name);

    for (j = -1; j <= 128; j++) {
      if (j == tests[i].key_length)
        continue;
      TEST_CHECK(!CMC_CreateInstance(algorithm, tests[i].key, j));
    }

    inst = CMC_CreateInstance(algorithm, tests[i].key, tests[i].key_length);
    TEST_CHECK(inst);

    TEST_CHECK(!CMC_CreateInstance(0, tests[i].key, tests[i].key_length));

    TEST_CHECK(CMC_Hash(inst, data, -1, hash, sizeof (hash)) == 0);
    TEST_CHECK(CMC_Hash(inst, data, sizeof (data) - 1, hash, -1) == 0);

    for (j = 0; j <= sizeof (hash); j++) {
      memset(hash, 0, sizeof (hash));
      length = CMC_Hash(inst, data, sizeof (data) - 1, hash, j);

#if 0
      for (int k = 0; k < length; k++)
        printf("\\x%02x", hash[k]);
      printf("\n");
#endif

      if (j >= tests[i].hash_length)
        TEST_CHECK(length == tests[i].hash_length);
      else
        TEST_CHECK(length == j);

      TEST_CHECK(!memcmp(hash, tests[i].hash, length));
    }

    for (j = 0; j < sizeof (data); j++) {
      length = CMC_Hash(inst, data, j, hash, sizeof (hash));
      TEST_CHECK(length == tests[i].hash_length);
    }

    CMC_DestroyInstance(inst);
  }
}
