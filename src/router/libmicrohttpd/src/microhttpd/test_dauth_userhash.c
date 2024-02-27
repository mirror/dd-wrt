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
 * @file microhttpd/test_dauth_userhash.c
 * @brief  Tests for Digest Auth calculations of userhash
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
  const uint8_t hash[MHD_MD5_DIGEST_SIZE];
};


static const struct data_md5 md5_tests[] = {
  {__LINE__,
   "u", "r",
   {0xba, 0x84, 0xbe, 0x20, 0x3f, 0xdf, 0xc7, 0xd3, 0x4e, 0x05, 0x4a, 0x76,
    0xd2, 0x85, 0xd0, 0xc9}},
  {__LINE__,
   "testuser", "testrealm",
   {0xab, 0xae, 0x15, 0x95, 0x24, 0xe5, 0x17, 0xbf, 0x48, 0xf4, 0x4a, 0xab,
    0xfe, 0xb9, 0x37, 0x40}},
  {__LINE__,
   "test_user", "TestRealm", /* Values from testcurl/test_digestauth2.c */
   {0xc5, 0x3c, 0x60, 0x15, 0x03, 0xff, 0x17, 0x6f, 0x18, 0xf6, 0x23, 0x72,
    0x5f, 0xba, 0x42, 0x81}},
  {__LINE__,
   "Mufasa", "myhost@testrealm.com",
   {0x26, 0x45, 0xae, 0x13, 0xd1, 0xa2, 0xa6, 0x9e, 0xd2, 0x6d, 0xd2, 0x1a,
    0xa5, 0x52, 0x86, 0xe3}},
  {__LINE__,
   "Mufasa", "myhost@example.com",
   {0x4f, 0xc8, 0x64, 0x02, 0xd7, 0x18, 0x5d, 0x7b, 0x38, 0xd4, 0x38, 0xad,
    0xd5, 0x5a, 0x35, 0x84}},
  {__LINE__,
   "Mufasa", "http-auth@example.org",
   {0x42, 0x38, 0xf3, 0xa1, 0x61, 0x67, 0x37, 0x3f, 0xeb, 0xb9, 0xbc, 0x4d,
    0x43, 0xdb, 0x9c, 0xc4}},
  {__LINE__,
   "J" "\xC3\xA4" "s" "\xC3\xB8" "n Doe" /* "Jäsøn Doe" */, "api@example.org",
   {0x2e, 0x06, 0x3f, 0xa2, 0xc5, 0x4d, 0xea, 0x1c, 0x36, 0x80, 0x8b, 0x7a,
    0x6e, 0x3b, 0x14, 0xc9}}
};

struct data_sha256
{
  unsigned int line_num;
  const char *const username;
  const char *const realm;
  const uint8_t hash[MHD_SHA256_DIGEST_SIZE];
};

static const struct data_sha256 sha256_tests[] = {
  {__LINE__,
   "u", "r",
   {0x1d, 0x8a, 0x03, 0xa6, 0xe2, 0x1a, 0x4c, 0xe7, 0x75, 0x06, 0x0e, 0xa5,
    0x73, 0x60, 0x32, 0x9a, 0xc7, 0x50, 0xde, 0xa5, 0xd8, 0x47, 0x29, 0x7b,
    0x42, 0xf0, 0xd4, 0x65, 0x39, 0xaf, 0x8a, 0xb2}},
  {__LINE__,
   "testuser", "testrealm",
   {0x75, 0xaf, 0x8a, 0x35, 0x00, 0xf7, 0x71, 0xe5, 0x8a, 0x52, 0x09, 0x3a,
    0x25, 0xe7, 0x90, 0x5d, 0x6e, 0x42, 0x8a, 0x51, 0x12, 0x85, 0xc1, 0x2e,
    0xa1, 0x42, 0x0c, 0x73, 0x07, 0x8d, 0xfd, 0x61}},
  {__LINE__,
   "test_user", "TestRealm", /* Values from testcurl/test_digestauth2.c */
   {0x09, 0x0c, 0x7e, 0x06, 0xb7, 0x7d, 0x66, 0x14, 0xcf, 0x5f, 0xe6, 0xca,
    0xfa, 0x00, 0x4d, 0x2e, 0x5f, 0x8f, 0xb3, 0x6b, 0xa4, 0x5a, 0x0e, 0x35,
    0xea, 0xcb, 0x2e, 0xb7, 0x72, 0x8f, 0x34, 0xde}},
  {__LINE__,
   "Mufasa", "myhost@testrealm.com",
   {0x92, 0x9f, 0xac, 0x9e, 0x6c, 0x8b, 0x76, 0xcc, 0xab, 0xa9, 0xe0, 0x6f,
    0xf6, 0x4d, 0xf2, 0x6f, 0xcb, 0x40, 0x56, 0x4c, 0x19, 0x9c, 0x32, 0xd9,
    0xea, 0xd9, 0x12, 0x4b, 0x25, 0x34, 0xe1, 0xf9}},
  {__LINE__,
   "Mufasa", "myhost@example.com",
   {0x97, 0x06, 0xf0, 0x07, 0x1c, 0xec, 0x05, 0x3f, 0x88, 0x22, 0xb6, 0x63,
    0x69, 0xc4, 0xa4, 0x00, 0x39, 0x79, 0xb7, 0xe7, 0x42, 0xb7, 0x4e, 0x42,
    0x59, 0x63, 0x57, 0xf4, 0xd3, 0x02, 0xae, 0x16}},
  {__LINE__,
   "Mufasa", "http-auth@example.org",
   {0xa9, 0x47, 0xaa, 0xd2, 0x05, 0xe8, 0x0e, 0x42, 0x99, 0x58, 0xa3, 0x87,
    0x39, 0x49, 0x44, 0xc6, 0xb4, 0x96, 0x30, 0x1e, 0x79, 0xf8, 0x9d, 0x35,
    0xa4, 0xcc, 0x23, 0xb6, 0xee, 0x12, 0xb5, 0xb6}},
  {__LINE__,
   "J" "\xC3\xA4" "s" "\xC3\xB8" "n Doe" /* "Jäsøn Doe" */, "api@example.org",
   {0x5a, 0x1a, 0x8a, 0x47, 0xdf, 0x5c, 0x29, 0x85, 0x51, 0xb9, 0xb4, 0x2b,
    0xa9, 0xb0, 0x58, 0x35, 0x17, 0x4a, 0x5b, 0xd7, 0xd5, 0x11, 0xff, 0x7f,
    0xe9, 0x19, 0x1d, 0x8e, 0x94, 0x6f, 0xc4, 0xe7}}
};

struct data_sha512_256
{
  unsigned int line_num;
  const char *const username;
  const char *const realm;
  const uint8_t hash[MHD_SHA512_256_DIGEST_SIZE];
};


static const struct data_sha512_256 sha512_256_tests[] = {
  {__LINE__,
   "u", "r",
   {0xc7, 0x38, 0xf2, 0xad, 0x40, 0x1b, 0xc8, 0x7a, 0x71, 0xfe, 0x78, 0x09,
    0x60, 0x15, 0xc9, 0x7b, 0x9a, 0x26, 0xd5, 0x5f, 0x15, 0xe9, 0xf5, 0x0a,
    0xc3, 0xa6, 0xde, 0x73, 0xdd, 0xcd, 0x3d, 0x08}},
  {__LINE__,
   "testuser", "testrealm",
   {0x4f, 0x69, 0x1e, 0xe9, 0x50, 0x8a, 0xe4, 0x55, 0x21, 0x32, 0x9e, 0xcf,
    0xd4, 0x91, 0xf7, 0xe2, 0x77, 0x4b, 0x6f, 0xb8, 0x60, 0x2c, 0x14, 0x86,
    0xad, 0x94, 0x9d, 0x1c, 0x23, 0xd8, 0xa1, 0xf5}},
  {__LINE__,
   "test_user", "TestRealm", /* Values from testcurl/test_digestauth2.c */
   {0x62, 0xe1, 0xac, 0x9f, 0x6c, 0xb1, 0xeb, 0x26, 0xaa, 0x75, 0xeb, 0x5d,
    0x46, 0xef, 0xcd, 0xc8, 0x9c, 0xcb, 0xa7, 0x81, 0xf0, 0xf9, 0xf7, 0x2f,
    0x6a, 0xfd, 0xb9, 0x42, 0x65, 0xd9, 0xa7, 0x9a}},
  {__LINE__,
   "Mufasa", "myhost@testrealm.com",
   {0xbd, 0x3e, 0xbc, 0x30, 0x10, 0x0b, 0x7c, 0xf1, 0x61, 0x45, 0x6c, 0xfe,
    0x64, 0x1c, 0x4c, 0xd2, 0x82, 0xe0, 0x62, 0x6e, 0x2c, 0x5e, 0x09, 0xc2,
    0x4c, 0x90, 0xb1, 0x60, 0x8a, 0xec, 0x28, 0x64}},
  {__LINE__,
   "Mufasa", "myhost@example.com",
   {0xea, 0x4b, 0x59, 0x37, 0xde, 0x2c, 0x4e, 0x9f, 0x16, 0xf9, 0x9c, 0x31,
    0x01, 0xb6, 0xdd, 0xf8, 0x8c, 0x85, 0xd7, 0xe8, 0xf1, 0x75, 0x90, 0xd0,
    0x63, 0x2a, 0x75, 0x75, 0xe4, 0x80, 0x13, 0x69}},
  {__LINE__,
   "Mufasa", "http-auth@example.org",
   {0xe2, 0xdf, 0xab, 0xd1, 0xa9, 0x6d, 0xdf, 0x86, 0x77, 0x10, 0xb6, 0x53,
    0xb6, 0xe6, 0x85, 0x7d, 0x1f, 0x14, 0x70, 0x86, 0xde, 0x7d, 0x7e, 0xf7,
    0x9d, 0xcd, 0x24, 0x98, 0x59, 0x87, 0x25, 0x70}},
  {__LINE__,
   "J" "\xC3\xA4" "s" "\xC3\xB8" "n Doe" /* "Jäsøn Doe" */, "api@example.org",
   {0x79, 0x32, 0x63, 0xca, 0xab, 0xb7, 0x07, 0xa5, 0x62, 0x11, 0x94, 0x0d,
    0x90, 0x41, 0x1e, 0xa4, 0xa5, 0x75, 0xad, 0xec, 0xcb, 0x7e, 0x36, 0x0a,
    0xeb, 0x62, 0x4e, 0xd0, 0x6e, 0xce, 0x9b, 0x0b}}
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

  func_name = "MHD_digest_auth_calc_userhash";
  if (MHD_YES != MHD_digest_auth_calc_userhash (algo3,
                                                data->username, data->realm,
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

  func_name = "MHD_digest_auth_calc_userhash_hex";
  if (MHD_YES !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         data->username, data->realm,
                                         hash_hex, sizeof(hash_hex)))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_YES.\n",
             func_name);
  }
  else if (sizeof(hash_hex) - 1 != strlen (hash_hex))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s produced hash with wrong length. "
             "Calculated length %u, expected digest %u.\n",
             func_name,
             (unsigned) strlen (hash_hex),
             (unsigned) (sizeof(hash_hex) - 1));
  }
  else
  {
    bin2hex (data->hash, sizeof(data->hash), expected_hex);
    if (0 != memcmp (hash_hex, expected_hex, sizeof(hash_hex)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() produced wrong hash. "
               "Calculated digest %s, expected digest %s.\n",
               func_name,
               hash_hex, expected_hex);
    }
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
  char hash_hex[MHD_MD5_DIGEST_SIZE * 2 + 1];
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
  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_MD5))
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
  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_DIGEST_AUTH_MD5))
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
check_sha256 (const struct data_sha256 *const data)
{
  static const enum MHD_DigestAuthAlgo3 algo3 = MHD_DIGEST_AUTH_ALGO3_SHA256;
  uint8_t hash_bin[MHD_SHA256_DIGEST_SIZE];
  char hash_hex[MHD_SHA256_DIGEST_SIZE * 2 + 1];
  char expected_hex[MHD_SHA256_DIGEST_SIZE * 2 + 1];
  const char *func_name;
  unsigned int failed = 0;

  func_name = "MHD_digest_auth_calc_userhash";
  if (MHD_YES != MHD_digest_auth_calc_userhash (algo3,
                                                data->username, data->realm,
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

  func_name = "MHD_digest_auth_calc_userhash_hex";
  if (MHD_YES !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         data->username, data->realm,
                                         hash_hex, sizeof(hash_hex)))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_YES.\n",
             func_name);
  }
  else if (sizeof(hash_hex) - 1 != strlen (hash_hex))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s produced hash with wrong length. "
             "Calculated length %u, expected digest %u.\n",
             func_name,
             (unsigned) strlen (hash_hex),
             (unsigned) (sizeof(hash_hex) - 1));
  }
  else
  {
    bin2hex (data->hash, sizeof(data->hash), expected_hex);
    if (0 != memcmp (hash_hex, expected_hex, sizeof(hash_hex)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() produced wrong hash. "
               "Calculated digest %s, expected digest %s.\n",
               func_name,
               hash_hex, expected_hex);
    }
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

  func_name = "MHD_digest_auth_calc_userhash";
  if (MHD_YES != MHD_digest_auth_calc_userhash (algo3,
                                                data->username, data->realm,
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

  func_name = "MHD_digest_auth_calc_userhash_hex";
  if (MHD_YES !=
      MHD_digest_auth_calc_userhash_hex (algo3,
                                         data->username, data->realm,
                                         hash_hex, sizeof(hash_hex)))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s() has not returned MHD_YES.\n",
             func_name);
  }
  else if (sizeof(hash_hex) - 1 != strlen (hash_hex))
  {
    failed++;
    fprintf (stderr,
             "FAILED: %s produced hash with wrong length. "
             "Calculated length %u, expected digest %u.\n",
             func_name,
             (unsigned) strlen (hash_hex),
             (unsigned) (sizeof(hash_hex) - 1));
  }
  else
  {
    bin2hex (data->hash, sizeof(data->hash), expected_hex);
    if (0 != memcmp (hash_hex, expected_hex, sizeof(hash_hex)))
    {
      failed++;
      fprintf (stderr,
               "FAILED: %s() produced wrong hash. "
               "Calculated digest %s, expected digest %s.\n",
               func_name,
               hash_hex, expected_hex);
    }
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
