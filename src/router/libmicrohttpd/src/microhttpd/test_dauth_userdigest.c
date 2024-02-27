/*
  This file is part of libmicrohttpd
  Copyright (C) 2022 Evgeny Grin (Karlson2)

  This test tool is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2, or
  (at your option) any later version.

  This test tool is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/test_dauth_userdigest.c
 * @brief  Tests for Digest Auth calculations of userdigest
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microhttpd.h"
#include "test_helpers.h"

#if defined(MHD_HTTPS_REQUIRE_GCRYPT) && \
  (defined(MHD_SHA256_TLSLIB) || defined(MHD_MD5_TLSLIB))
#define NEED_GCRYP_INIT 1
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT && (MHD_SHA256_TLSLIB || MHD_MD5_TLSLIB) */

static int verbose = 1; /* verbose level (0-1)*/

/* Declarations and data */

struct data_md5
{
  unsigned int line_num;
  const char *const username;
  const char *const realm;
  const char *const password;
  const uint8_t hash[MHD_MD5_DIGEST_SIZE];
};


static const struct data_md5 md5_tests[] = {
  {__LINE__,
   "u", "r", "p",
   {0x44, 0xad, 0xd2, 0x2b, 0x6f, 0x31, 0x79, 0xb7, 0x51, 0xea, 0xfd, 0x68,
    0xee, 0x37, 0x0f, 0x7d}},
  {__LINE__,
   "testuser", "testrealm", "testpass",
   {0xeb, 0xff, 0x22, 0x5e, 0x1c, 0xeb, 0x73, 0xe0, 0x26, 0xfc, 0xc6, 0x45,
    0xaf, 0x3e, 0x84, 0xf6}},
  {__LINE__,  /* Values from testcurl/test_digestauth2.c */
   "test_user", "TestRealm", "test pass",
   {0xd8, 0xb4, 0xa6, 0xd0, 0x01, 0x13, 0x07, 0xb7, 0x67, 0x94, 0xea, 0x66,
    0x86, 0x03, 0x6b, 0x43}},
  {__LINE__,
   "Mufasa", "myhost@testrealm.com", "CircleOfLife",
   {0x7e, 0xbe, 0xcc, 0x07, 0x18, 0xa5, 0x4a, 0xb4, 0x7e, 0x21, 0x65, 0x69,
    0x07, 0x66, 0x41, 0x6a}},
  {__LINE__,
   "Mufasa", "myhost@example.com", "Circle Of Life",
   {0x6a, 0x6d, 0x4e, 0x7c, 0xd7, 0x15, 0x18, 0x68, 0xf9, 0xb8, 0xc7, 0xc8,
    0xd1, 0xcd, 0xd4, 0xe0}},
  {__LINE__,
   "Mufasa", "http-auth@example.org", "Circle of Life",
   {0x3d, 0x78, 0x80, 0x7d, 0xef, 0xe7, 0xde, 0x21, 0x57, 0xe2, 0xb0, 0xb6,
    0x57, 0x3a, 0x85, 0x5f}},
  {__LINE__,
   "J" "\xC3\xA4" "s" "\xC3\xB8" "n Doe" /* "Jäsøn Doe" */,
   "api@example.org", "Secret, or not?",
   {0x83, 0xa3, 0xf7, 0xf6, 0xb8, 0x3f, 0x71, 0xc5, 0xc2, 0xeb, 0x7c, 0x6d,
    0xd2, 0xdd, 0x4c, 0x4b}}
};

struct data_sha256
{
  unsigned int line_num;
  const char *const username;
  const char *const realm;
  const char *const password;
  const uint8_t hash[MHD_SHA256_DIGEST_SIZE];
};

static const struct data_sha256 sha256_tests[] = {
  {__LINE__,
   "u", "r", "p",
   {0xdc, 0xb0, 0x21, 0x10, 0x2e, 0x49, 0x1e, 0x70, 0x1a, 0x4a, 0x23, 0x6d,
    0xaa, 0x89, 0x23, 0xaf, 0x21, 0x61, 0x44, 0x7b, 0xce, 0x7b, 0xb7, 0x26,
    0x0a, 0x35, 0x1e, 0xe8, 0x3e, 0x9f, 0x81, 0x54}},
  {__LINE__,
   "testuser", "testrealm", "testpass",
   {0xa9, 0x2e, 0xf6, 0x3b, 0x3d, 0xec, 0x38, 0x95, 0xb0, 0x8f, 0x3d, 0x4d,
    0x67, 0x33, 0xf0, 0x70, 0x74, 0xcb, 0xe6, 0xd4, 0xa0, 0x01, 0x27, 0xf5,
    0x74, 0x1a, 0x77, 0x4f, 0x05, 0xf9, 0xd4, 0x99}},
  {__LINE__,  /* Values from testcurl/test_digestauth2.c */
   "test_user", "TestRealm", "test pass",
   {0xc3, 0x4e, 0x16, 0x5a, 0x17, 0x0f, 0xe5, 0xac, 0x04, 0xf1, 0x6e, 0x46,
    0x48, 0x2b, 0xa0, 0xc6, 0x56, 0xc1, 0xfb, 0x8f, 0x66, 0xa6, 0xd6, 0x3f,
    0x91, 0x12, 0xf8, 0x56, 0xa5, 0xec, 0x6d, 0x6d}},
  {__LINE__,
   "Mufasa", "myhost@testrealm.com", "CircleOfLife",
   {0x8e, 0x64, 0x1f, 0xaa, 0x71, 0x7d, 0x20, 0x70, 0x5a, 0xd7, 0x3c, 0x54,
    0xfb, 0x04, 0x9e, 0x32, 0x6a, 0xe1, 0x1c, 0x80, 0xd6, 0x05, 0x9f, 0xc3,
    0x7e, 0xbb, 0x2d, 0x7b, 0x60, 0x6c, 0x11, 0xb9}},
  {__LINE__,
   "Mufasa", "myhost@example.com", "Circle Of Life",
   {0x8b, 0xc5, 0xa8, 0xed, 0xe3, 0x02, 0x15, 0x6b, 0x9f, 0x51, 0xce, 0x97,
    0x81, 0xb5, 0x26, 0xff, 0x99, 0x29, 0x0b, 0xb2, 0xc3, 0xe4, 0x41, 0x71,
    0x8e, 0xa3, 0xa1, 0x7e, 0x5a, 0xd9, 0xd6, 0x49}},
  {__LINE__,
   "Mufasa", "http-auth@example.org", "Circle of Life",
   {0x79, 0x87, 0xc6, 0x4c, 0x30, 0xe2, 0x5f, 0x1b, 0x74, 0xbe, 0x53, 0xf9,
    0x66, 0xb4, 0x9b, 0x90, 0xf2, 0x80, 0x8a, 0xa9, 0x2f, 0xaf, 0x9a, 0x00,
    0x26, 0x23, 0x92, 0xd7, 0xb4, 0x79, 0x42, 0x32}},
  {__LINE__,
   "J" "\xC3\xA4" "s" "\xC3\xB8" "n Doe" /* "Jäsøn Doe" */,
   "api@example.org", "Secret, or not?",
   {0xfd, 0x0b, 0xe3, 0x93, 0x9d, 0xca, 0x4b, 0x5c, 0x2d, 0x46, 0xe8, 0xfa,
    0x6a, 0x3d, 0x16, 0xdb, 0xea, 0x82, 0x47, 0x4c, 0xb9, 0xa5, 0x88, 0xd4,
    0xcb, 0x14, 0x9c, 0x54, 0xf3, 0x7c, 0xff, 0x37}}
};

struct data_sha512_256
{
  unsigned int line_num;
  const char *const username;
  const char *const realm;
  const char *const password;
  const uint8_t hash[MHD_SHA512_256_DIGEST_SIZE];
};

static const struct data_sha512_256 sha512_256_tests[] = {
  {__LINE__,
   "u", "r", "p",
   {0xd5, 0xe8, 0xe7, 0x3b, 0xa3, 0x47, 0xb9, 0xad, 0xf0, 0xe4, 0x7a, 0x9a,
    0xce, 0x43, 0xb7, 0x08, 0x2a, 0xbc, 0x8d, 0x27, 0x27, 0x2e, 0x38, 0x7d,
    0x1d, 0x9c, 0xe2, 0x44, 0x25, 0x68, 0x74, 0x04}},
  {__LINE__,
   "testuser", "testrealm", "testpass",
   {0x41, 0x7d, 0xf9, 0x60, 0x7c, 0xc9, 0x60, 0x28, 0x44, 0x74, 0x75, 0xf7,
    0x7b, 0x78, 0xe7, 0x60, 0xec, 0x9a, 0xe1, 0x62, 0xd4, 0x95, 0x82, 0x61,
    0x68, 0xa7, 0x94, 0xe8, 0x3b, 0xdf, 0x8d, 0x59}},
  {__LINE__,  /* Values from testcurl/test_digestauth2.c */
   "test_user", "TestRealm", "test pass",
   {0xe7, 0xa1, 0x9e, 0x27, 0xf6, 0x73, 0x88, 0xb2, 0xde, 0xa4, 0xe2, 0x66,
    0xc5, 0x16, 0x37, 0x17, 0x4d, 0x29, 0xcc, 0xa3, 0xc1, 0xf5, 0xb2, 0x49,
    0x20, 0xc1, 0x05, 0xc9, 0x20, 0x13, 0x3c, 0x3d}},
  {__LINE__,
   "Mufasa", "myhost@testrealm.com", "CircleOfLife",
   {0x44, 0xbc, 0xd2, 0xb1, 0x1f, 0x6f, 0x7d, 0xd3, 0xae, 0xa6, 0x66, 0x8a,
    0x24, 0x84, 0x4b, 0x87, 0x7d, 0xe1, 0x80, 0x24, 0x9a, 0x26, 0x6b, 0xe6,
    0xdb, 0x7f, 0xe3, 0xc8, 0x7a, 0xf9, 0x75, 0x64}},
  {__LINE__,
   "Mufasa", "myhost@example.com", "Circle Of Life",
   {0xd5, 0xf6, 0x25, 0x7c, 0x64, 0xe4, 0x01, 0xd2, 0x87, 0xd5, 0xaa, 0x19,
    0xae, 0xf0, 0xa2, 0xa2, 0xce, 0x4e, 0x5d, 0xfc, 0x77, 0x70, 0x0b, 0x72,
    0x90, 0x43, 0x96, 0xd2, 0x95, 0x6e, 0x83, 0x0a}},
  {__LINE__,
   "Mufasa", "http-auth@example.org", "Circle of Life",
   {0xfb, 0x17, 0x4f, 0x5c, 0x3c, 0x78, 0x02, 0x72, 0x15, 0x17, 0xca, 0xe1,
    0x3b, 0x98, 0xe2, 0xb8, 0xda, 0xe2, 0xe0, 0x11, 0x8c, 0xb7, 0x05, 0xd9,
    0x4e, 0xe2, 0x99, 0x46, 0x31, 0x92, 0x04, 0xce}},
  {__LINE__,
   "J" "\xC3\xA4" "s" "\xC3\xB8" "n Doe" /* "Jäsøn Doe" */,
   "api@example.org", "Secret, or not?",
   {0x2d, 0x3d, 0x9f, 0x12, 0xc9, 0xf3, 0xd3, 0x00, 0x11, 0x25, 0x9d, 0xc5,
    0xfe, 0xce, 0xe0, 0x05, 0xae, 0x24, 0xde, 0x40, 0xe3, 0xe1, 0xf6, 0x18,
    0x06, 0xd0, 0x3e, 0x65, 0xf1, 0xe6, 0x02, 0x4f}}
};


/*
 *  Helper functions
 */

/**
 * Print bin as lower case hex
 *
 * @param bin binary data
 * @param len number of bytes in bin
 * @param hex pointer to len*2+1 bytes buffer
 */
static void
bin2hex (const uint8_t *bin,
         size_t len,
         char *hex)
{
  while (len-- > 0)
  {
    unsigned int b1, b2;
    b1 = (*bin >> 4) & 0xf;
    *hex++ = (char) ((b1 > 9) ? (b1 + 'a' - 10) : (b1 + '0'));
    b2 = *bin++ & 0xf;
    *hex++ = (char) ((b2 > 9) ? (b2 + 'a' - 10) : (b2 + '0'));
  }
  *hex = 0;
}


/* Tests */

static unsigned int
check_md5 (const struct data_md5 *const data)
{
  static const enum MHD_DigestAuthAlgo3 algo3 = MHD_DIGEST_AUTH_ALGO3_MD5;
  uint8_t hash_bin[MHD_MD5_DIGEST_SIZE];
  char hash_hex[MHD_MD5_DIGEST_SIZE * 2 + 1];
  char expected_hex[MHD_MD5_DIGEST_SIZE * 2 + 1];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userdigest";
  if (MHD_YES != MHD_digest_auth_calc_userdigest (algo3,
                                                  data->username,
                                                  data->realm,
                                                  data->password,
                                                  hash_bin, sizeof(hash_bin)))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_YES.\n",
             func_name);
  }
  else if (0 != memcmp (hash_bin, data->hash, sizeof(data->hash)))
  {
    failed++;
    bin2hex (hash_bin, sizeof(hash_bin), hash_hex);
    bin2hex (data->hash, sizeof(data->hash), expected_hex);
    fprintf (stderr,
             "FAILED: %s() produced wrong hash. "
             "Calculated digest %s, expected digest %s.\n",
             func_name,
             hash_hex, expected_hex);
  }

  if (failed)
  {
    fprintf (stderr,
             "The check failed for data located at line: %u.\n",
             data->line_num);
    fflush (stderr);
  }
  else if (verbose)
  {
    printf ("PASSED: check for data at line: %u.\n",
            data->line_num);
  }
  return failed ? 1 : 0;
}


static unsigned int
test_md5 (void)
{
  unsigned int num_failed = 0;
  size_t i;

  for (i = 0; i < sizeof(md5_tests) / sizeof(md5_tests[0]); i++)
    num_failed += check_md5 (md5_tests + i);
  return num_failed;
}


static unsigned int
test_md5_failure (void)
{
  static const enum MHD_DigestAuthAlgo3 algo3 = MHD_DIGEST_AUTH_ALGO3_MD5;
  uint8_t hash_bin[MHD_MD5_DIGEST_SIZE];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userdigest";
  if (MHD_NO != MHD_digest_auth_calc_userdigest (algo3,
                                                 "u", "r", "p",
                                                 hash_bin, sizeof(hash_bin)
                                                 - 1))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO != MHD_digest_auth_calc_userdigest (algo3,
                                                 "u", "r", "p",
                                                 hash_bin, 0))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_MD5))
  {
    if (MHD_NO != MHD_digest_auth_calc_userdigest (algo3,
                                                   "u", "r", "p",
                                                   hash_bin, sizeof(hash_bin)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() has not returned MHD_NO at line: %u.\n",
               func_name, (unsigned) __LINE__);
    }
  }

  if (! failed && verbose)
  {
    printf ("PASSED: all checks with expected MHD_NO result near line: %u.\n",
            (unsigned) __LINE__);
  }
  return failed ? 1 : 0;
}


static unsigned int
check_sha256 (const struct data_sha256 *const data)
{
  static const enum MHD_DigestAuthAlgo3 algo3 = MHD_DIGEST_AUTH_ALGO3_SHA256;
  uint8_t hash_bin[MHD_SHA256_DIGEST_SIZE];
  char hash_hex[MHD_SHA256_DIGEST_SIZE * 2 + 1];
  char expected_hex[MHD_SHA256_DIGEST_SIZE * 2 + 1];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userdigest";
  if (MHD_YES != MHD_digest_auth_calc_userdigest (algo3,
                                                  data->username,
                                                  data->realm,
                                                  data->password,
                                                  hash_bin, sizeof(hash_bin)))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_YES.\n",
             func_name);
  }
  else if (0 != memcmp (hash_bin, data->hash, sizeof(data->hash)))
  {
    failed++;
    bin2hex (hash_bin, sizeof(hash_bin), hash_hex);
    bin2hex (data->hash, sizeof(data->hash), expected_hex);
    fprintf (stderr,
             "FAILED: %s() produced wrong hash. "
             "Calculated digest %s, expected digest %s.\n",
             func_name,
             hash_hex, expected_hex);
  }

  if (failed)
  {
    fprintf (stderr,
             "The check failed for data located at line: %u.\n",
             data->line_num);
    fflush (stderr);
  }
  else if (verbose)
  {
    printf ("PASSED: check for data at line: %u.\n",
            data->line_num);
  }
  return failed ? 1 : 0;
}


static unsigned int
test_sha256 (void)
{
  unsigned int num_failed = 0;
  size_t i;

  for (i = 0; i < sizeof(sha256_tests) / sizeof(sha256_tests[0]); i++)
    num_failed += check_sha256 (sha256_tests + i);
  return num_failed;
}


static unsigned int
test_sha256_failure (void)
{
  static const enum MHD_DigestAuthAlgo3 algo3 = MHD_DIGEST_AUTH_ALGO3_SHA256;
  uint8_t hash_bin[MHD_SHA256_DIGEST_SIZE];
  char hash_hex[MHD_SHA256_DIGEST_SIZE * 2 + 1];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userhash";
  if (MHD_NO != MHD_digest_auth_calc_userhash (algo3,
                                               "u", "r",
                                               hash_bin, sizeof(hash_bin) - 1))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO != MHD_digest_auth_calc_userhash (algo3,
                                               "u", "r",
                                               hash_bin, 0))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_SHA256))
  {
    if (MHD_NO != MHD_digest_auth_calc_userhash (algo3,
                                                 "u", "r",
                                                 hash_bin, sizeof(hash_bin)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() has not returned MHD_NO at line: %u.\n",
               func_name, (unsigned) __LINE__);
    }
  }

  func_name = "MHD_digest_auth_calc_userhash_hex";
  if (MHD_NO !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         "u", "r",
                                         hash_hex, sizeof(hash_hex) - 1))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         "u", "r",
                                         hash_hex, 0))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_SHA256))
  {
    if (MHD_NO !=
        MHD_digest_auth_calc_userhash_hex (algo3,
                                           "u", "r",
                                           hash_hex, sizeof(hash_hex)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() has not returned MHD_NO at line: %u.\n",
               func_name, (unsigned) __LINE__);
    }
  }

  if (! failed && verbose)
  {
    printf ("PASSED: all checks with expected MHD_NO result near line: %u.\n",
            (unsigned) __LINE__);
  }
  return failed ? 1 : 0;
}


static unsigned int
check_sha512_256 (const struct data_sha512_256 *const data)
{
  static const enum MHD_DigestAuthAlgo3 algo3 =
    MHD_DIGEST_AUTH_ALGO3_SHA512_256;
  uint8_t hash_bin[MHD_SHA512_256_DIGEST_SIZE];
  char hash_hex[MHD_SHA512_256_DIGEST_SIZE * 2 + 1];
  char expected_hex[MHD_SHA512_256_DIGEST_SIZE * 2 + 1];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userdigest";
  if (MHD_YES != MHD_digest_auth_calc_userdigest (algo3,
                                                  data->username,
                                                  data->realm,
                                                  data->password,
                                                  hash_bin, sizeof(hash_bin)))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_YES.\n",
             func_name);
  }
  else if (0 != memcmp (hash_bin, data->hash, sizeof(data->hash)))
  {
    failed++;
    bin2hex (hash_bin, sizeof(hash_bin), hash_hex);
    bin2hex (data->hash, sizeof(data->hash), expected_hex);
    fprintf (stderr,
             "FAILED: %s() produced wrong hash. "
             "Calculated digest %s, expected digest %s.\n",
             func_name,
             hash_hex, expected_hex);
  }

  if (failed)
  {
    fprintf (stderr,
             "The check failed for data located at line: %u.\n",
             data->line_num);
    fflush (stderr);
  }
  else if (verbose)
  {
    printf ("PASSED: check for data at line: %u.\n",
            data->line_num);
  }
  return failed ? 1 : 0;
}


static unsigned int
test_sha512_256 (void)
{
  unsigned int num_failed = 0;
  size_t i;

  for (i = 0; i < sizeof(sha512_256_tests) / sizeof(sha512_256_tests[0]); i++)
    num_failed += check_sha512_256 (sha512_256_tests + i);
  return num_failed;
}


static unsigned int
test_sha512_256_failure (void)
{
  static const enum MHD_DigestAuthAlgo3 algo3 =
    MHD_DIGEST_AUTH_ALGO3_SHA512_256;
  static const enum MHD_FEATURE feature = MHD_FEATURE_DIGEST_AUTH_SHA512_256;
  uint8_t hash_bin[MHD_SHA512_256_DIGEST_SIZE];
  char hash_hex[MHD_SHA512_256_DIGEST_SIZE * 2 + 1];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userhash";
  if (MHD_NO != MHD_digest_auth_calc_userhash (algo3,
                                               "u", "r",
                                               hash_bin, sizeof(hash_bin) - 1))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO != MHD_digest_auth_calc_userhash (algo3,
                                               "u", "r",
                                               hash_bin, 0))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO == MHD_is_feature_supported (feature))
  {
    if (MHD_NO != MHD_digest_auth_calc_userhash (algo3,
                                                 "u", "r",
                                                 hash_bin, sizeof(hash_bin)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() has not returned MHD_NO at line: %u.\n",
               func_name, (unsigned) __LINE__);
    }
  }

  func_name = "MHD_digest_auth_calc_userhash_hex";
  if (MHD_NO !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         "u", "r",
                                         hash_hex, sizeof(hash_hex) - 1))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         "u", "r",
                                         hash_hex, 0))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_NO at line: %u.\n",
             func_name, (unsigned) __LINE__);
  }
  if (MHD_NO == MHD_is_feature_supported (feature))
  {
    if (MHD_NO !=
        MHD_digest_auth_calc_userhash_hex (algo3,
                                           "u", "r",
                                           hash_hex, sizeof(hash_hex)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() has not returned MHD_NO at line: %u.\n",
               func_name, (unsigned) __LINE__);
    }
  }

  if (! failed && verbose)
  {
    printf ("PASSED: all checks with expected MHD_NO result near line: %u.\n",
            (unsigned) __LINE__);
  }
  return failed ? 1 : 0;
}


int
main (int argc, char *argv[])
{
  unsigned int num_failed = 0;
  (void) has_in_name; /* Mute compiler warning. */
  if (has_param (argc, argv, "-s") || has_param (argc, argv, "--silent"))
    verbose = 0;

#ifdef NEED_GCRYP_INIT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif /* GCRYCTL_INITIALIZATION_FINISHED */
#endif /* NEED_GCRYP_INIT */

  if (MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_MD5))
    num_failed += test_md5 ();
  num_failed += test_md5_failure ();
  if (MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_SHA256))
    num_failed += test_sha256 ();
  num_failed += test_sha256_failure ();
  if (MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_SHA512_256))
    num_failed += test_sha512_256 ();
  num_failed += test_sha512_256_failure ();

  return num_failed ? 1 : 0;
}
