/*
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
 */

#include <config.h>
#include <sysincl.h>
#include <hash.h>
#include <logging.h>
#include "test.h"

struct hash_test {
  const char *name;
  const unsigned char out[MAX_HASH_LENGTH];
  unsigned int length;
};

void
test_unit(void)
{
  unsigned char data1[] = "abcdefghijklmnopqrstuvwxyz";
  unsigned char data2[] = "12345678910";
  unsigned char out[MAX_HASH_LENGTH];
  struct hash_test tests[] = {
    { "MD5",       "\xfc\x24\x97\x1b\x52\x66\xdc\x46\xef\xe0\xe8\x08\x46\x89\xb6\x88", 16 },
    { "SHA1",      "\xd8\x85\xb3\x86\xce\xea\x93\xeb\x92\xcd\x7b\x94\xb9\x8d\xc2\x8e"
                   "\x3e\x31\x13\xdd", 20},
    { "SHA256",    "\x0e\x35\x14\xe7\x15\x7a\x1d\xdd\xea\x11\x78\xd3\x41\xf6\xb9\x3e"
                   "\xa0\x42\x96\x73\x3c\x54\x74\x0b\xfa\x6b\x9e\x29\x59\xad\x69\xd3", 32 },
    { "SHA384",    "\x2c\xeb\xbd\x4d\x95\xed\xad\x03\xed\x80\xc4\xf3\xa6\x10\x21\xde"
                   "\x40\x69\x54\xed\x42\x70\xb8\x95\xb0\x6f\x01\x1d\x04\xdf\x57\xbc"
                   "\x1d\xb5\x85\xbf\x4f\x03\x88\xd5\x83\x93\xbc\x81\x90\xb0\xa9\x9b", 48 },
    { "SHA512",    "\x20\xba\xec\xcb\x68\x98\x33\x5b\x70\x26\x63\x13\xe2\xf7\x0e\x67"
                   "\x08\xf3\x77\x4f\xbd\xeb\xc4\xa8\xc5\x94\xe2\x39\x40\x7e\xed\x0b"
                   "\x69\x0e\x18\xa5\xa2\x03\x73\xe7\x1d\x20\x7d\x3f\xc8\x70\x2d\x64"
                   "\x9e\x89\x6d\x20\x6a\x4a\x5a\x46\xe7\x4f\x2c\xf9\x0f\x0a\x54\xdc", 64 },
    { "SHA3-224",  "\x3b\xa2\x22\x28\xdd\x26\x18\xec\x3b\xb9\x25\x39\x5e\xbd\x94\x25"
                   "\xd4\x20\x8a\x76\x76\xc0\x3c\x5d\x9e\x0a\x06\x46", 28},
    { "SHA3-256",  "\x26\xd1\x19\xb2\xc1\x64\xc8\xb8\x10\xd8\xa8\x1c\xb6\xa4\x0d\x29"
                   "\x09\xc9\x8e\x2e\x2d\xde\x7a\x74\x8c\x43\x70\xb8\xaa\x0f\x09\x17", 32 },
    { "SHA3-384",  "\x6a\x64\xb9\x89\x08\x29\xd0\xa7\x4b\x84\xba\xa6\x65\xf5\xe7\x54"
                   "\xe2\x18\x12\xc3\x63\x34\xc6\xba\x26\xf5\x6e\x99\xe2\x54\xcc\x9d"
                   "\x01\x10\x9d\xee\x35\x38\x04\x83\xe5\x71\x70\xd8\xc8\x99\x96\xd8", 48 },
    { "SHA3-512",  "\xa8\xe3\x2b\x65\x1f\x87\x90\x73\x19\xc8\xa0\x3f\xe3\x85\x60\x3c"
                   "\x39\xfc\xcb\xc1\x29\xe1\x23\x7d\x8b\x56\x54\xe3\x08\x9d\xf9\x74"
                   "\x78\x69\x2e\x3c\x7e\x51\x1e\x9d\xab\x09\xbe\xe7\x6b\x1a\xa1\x22"
                   "\x93\xb1\x2b\x82\x9d\x1e\xcf\xa8\x99\xc5\xec\x7b\x1d\x89\x07\x2b", 64 },
    { "RMD128",    "\x6f\xd7\x1f\x37\x47\x0f\xbd\x42\x57\xc8\xbb\xee\xba\x65\xf9\x35", 16 },
    { "RMD160",    "\x7a\x88\xec\xc7\x09\xc5\x65\x34\x11\x24\xe3\xf9\xf7\xa5\xbf\xc6"
                   "\x01\xe2\xc9\x32", 20},
    { "RMD256",    "\x59\xdf\xd4\xcb\xc9\xbe\x7c\x27\x08\xa7\x23\xf7\xb3\x0c\xf0\x0d"
                   "\xa0\xcf\x5b\x18\x16\x51\x56\x6d\xda\x7b\x87\x24\x9d\x83\x35\xe1", 32 },
    { "RMD320",    "\x68\x98\x10\xf4\xb6\x79\xb6\x15\xf1\x48\x2d\x73\xd0\x23\x84\x01"
                   "\xbf\xaa\x67\xcf\x1e\x35\x5c\xbf\xe9\xb8\xaf\xe1\xee\x0d\xf0\x6b"
                   "\xe2\x3a\x9a\x3a\xa7\x56\xad\x70", 40},
    { "TIGER",     "\x1c\xcd\x68\x74\xca\xd6\xd5\x17\xba\x3e\x82\xaf\xbd\x70\xdc\x66"
                   "\x99\xaa\xae\x16\x72\x59\xd1\x64", 24},
    { "WHIRLPOOL", "\xe3\xcd\xe6\xbf\xe1\x8c\xe4\x4d\xc8\xb4\xa5\x7c\x36\x8d\xc8\x8a"
                   "\x8b\xad\x52\x24\xc0\x4e\x99\x5b\x7e\x86\x94\x2d\x10\x56\x12\xa3"
                   "\x29\x2a\x65\x0f\x9e\x07\xbc\x15\x21\x14\xe6\x07\xfc\xe6\xb9\x2f"
                   "\x13\xe2\x57\xe9\x0a\xb0\xd2\xf4\xa3\x20\x36\x9c\x88\x92\x8e\xc9", 64 },
    { "", "", 0 }
  };

  unsigned int length;
  int i, j, hash_id;

  for (i = 0; tests[i].name[0] != '\0'; i++) {
    hash_id = HSH_GetHashId(tests[i].name);
    if (hash_id < 0) {
      TEST_CHECK(strcmp(tests[i].name, "MD5"));
#ifdef FEAT_SECHASH
      TEST_CHECK(strcmp(tests[i].name, "SHA1"));
      TEST_CHECK(strcmp(tests[i].name, "SHA256"));
      TEST_CHECK(strcmp(tests[i].name, "SHA384"));
      TEST_CHECK(strcmp(tests[i].name, "SHA512"));
#endif
      continue;
    }

    DEBUG_LOG("testing %s", tests[i].name);

    for (j = 0; j <= sizeof (out); j++) {
      TEST_CHECK(HSH_GetHashId(tests[i].name) == hash_id);
      TEST_CHECK(HSH_GetHashId("nosuchhash") < 0);

      memset(out, 0, sizeof (out));
      length = HSH_Hash(hash_id, data1, sizeof (data1) - 1, data2, sizeof (data2) - 1,
                        out, j);

      if (j >= tests[i].length)
        TEST_CHECK(length == tests[i].length);
      else
        TEST_CHECK(length == j);

      TEST_CHECK(!memcmp(out, tests[i].out, length));
    }

    for (j = 0; j < 10000; j++) {
      length = HSH_Hash(hash_id, data1, random() % sizeof (data1),
                        random() % 2 ? data2 : NULL, random() % sizeof (data2),
                        out, sizeof (out));
      TEST_CHECK(length == tests[i].length);
    }
  }

  HSH_Finalise();
}
