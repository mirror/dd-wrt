/*
     This file is part of libmicrohttpd
     Copyright (C) 2010 Christian Grothoff
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

     libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
     option) any later version.

     libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @file test_digest2.c
 * @brief  Testcase for MHD Digest Authorisation
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if defined(MHD_HTTPS_REQUIRE_GCRYPT) && \
  (defined(MHD_SHA256_TLSLIB) || defined(MHD_MD5_TLSLIB))
#define NEED_GCRYP_INIT 1
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT && (MHD_SHA256_TLSLIB || MHD_MD5_TLSLIB) */

#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
#else
#include <wincrypt.h>
#endif

#include "mhd_has_param.h"
#include "mhd_has_in_name.h"

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */

#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x) << 16 | (y) << 8 | (z))
#endif /* ! CURL_VERSION_BITS */
#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS (x, y, z))
#endif /* ! CURL_AT_LEAST_VERSION */

#ifndef _MHD_INSTRMACRO
/* Quoted macro parameter */
#define _MHD_INSTRMACRO(a) #a
#endif /* ! _MHD_INSTRMACRO */
#ifndef _MHD_STRMACRO
/* Quoted expanded macro parameter */
#define _MHD_STRMACRO(a) _MHD_INSTRMACRO (a)
#endif /* ! _MHD_STRMACRO */

#if defined(HAVE___FUNC__)
#define externalErrorExit(ignore) \
  _externalErrorExit_func (NULL, __func__, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, __func__, __LINE__)
#define libcurlErrorExit(ignore) \
  _libcurlErrorExit_func (NULL, __func__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func (errDesc, __func__, __LINE__)
#define mhdErrorExit(ignore) \
  _mhdErrorExit_func (NULL, __func__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
  _mhdErrorExit_func (errDesc, __func__, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func ((libcurlcall), _MHD_STRMACRO (libcurlcall), \
                       __func__, __LINE__)
#elif defined(HAVE___FUNCTION__)
#define externalErrorExit(ignore) \
  _externalErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define libcurlErrorExit(ignore) \
  _libcurlErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define mhdErrorExit(ignore) \
  _mhdErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
  _mhdErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func ((libcurlcall), _MHD_STRMACRO (libcurlcall), \
                       __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func (NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, NULL, __LINE__)
#define libcurlErrorExit(ignore) _libcurlErrorExit_func (NULL, NULL, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func (errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func (NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func (errDesc, NULL, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func ((libcurlcall), _MHD_STRMACRO (libcurlcall), NULL, \
                       __LINE__)
#endif


_MHD_NORETURN static void
_externalErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "System or external library call failed");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  fflush (stderr);
  exit (99);
}


/* Not actually used in this test */
static char libcurl_errbuf[CURL_ERROR_SIZE] = "";

_MHD_NORETURN static void
_libcurlErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "CURL library call failed");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  if (0 != libcurl_errbuf[0])
    fprintf (stderr, "Last libcurl error description: %s\n", libcurl_errbuf);

  fflush (stderr);
  exit (99);
}


_MHD_NORETURN static void
_mhdErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "MHD unexpected error");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */

  fflush (stderr);
  exit (8);
}


#if 0
/* Function unused in this test */
static void
_checkCURLE_OK_func (CURLcode code, const char *curlFunc,
                     const char *funcName, int lineNum)
{
  if (CURLE_OK == code)
    return;

  fflush (stdout);
  if ((NULL != curlFunc) && (0 != curlFunc[0]))
    fprintf (stderr, "'%s' resulted in '%s'", curlFunc,
             curl_easy_strerror (code));
  else
    fprintf (stderr, "libcurl function call resulted in '%s'",
             curl_easy_strerror (code));
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
  if (0 != libcurl_errbuf[0])
    fprintf (stderr, "Last libcurl error description: %s\n", libcurl_errbuf);

  fflush (stderr);
  exit (9);
}


#endif


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 10

#define MHD_URI_BASE_PATH "/bar%20foo?key=value"
#define MHD_URI_BASE_PATH2 "/another_path"
/* Should not fit buffer in the stack */
#define MHD_URI_BASE_PATH3 \
  "/long/long/long/long/long/long/long/long/long/long/long/long/long/long" \
  "/long/long/long/long/long/long/long/long/long/long/long/long/long/long" \
  "/long/long/long/long/long/long/long/long/long/long/long/long/long/long" \
  "/long/long/long/long/long/long/long/long/long/long/long/long/long/long" \
  "/long/long/long/long/long/long/long/long/long/long/long/long/long/long" \
  "/path?with%20some=parameters"

#define REALM_VAL "TestRealm"
#define USERNAME1 "test_user"
/* The hex form of MD5("test_user:TestRealm") */
#define USERHASH1_MD5_HEX "c53c601503ff176f18f623725fba4281"
#define USERHASH1_MD5_BIN 0xc5, 0x3c, 0x60, 0x15, 0x03, 0xff, 0x17, 0x6f, \
  0x18, 0xf6, 0x23, 0x72, 0x5f, 0xba, 0x42, 0x81
/* The hex form of SHA-256("test_user:TestRealm") */
#define USERHASH1_SHA256_HEX \
  "090c7e06b77d6614cf5fe6cafa004d2e5f8fb36ba45a0e35eacb2eb7728f34de"
/* The binary form of SHA-256("test_user:TestRealm") */
#define USERHASH1_SHA256_BIN 0x09, 0x0c, 0x7e, 0x06, 0xb7, 0x7d, 0x66, 0x14, \
  0xcf, 0x5f, 0xe6, 0xca, 0xfa, 0x00, 0x4d, 0x2e, 0x5f, 0x8f, 0xb3, 0x6b, \
  0xa4, 0x5a, 0x0e, 0x35, 0xea, 0xcb, 0x2e, 0xb7, 0x72, 0x8f, 0x34, 0xde
/* The hex form of MD5("test_user:TestRealm:test pass") */
#define USERDIGEST1_MD5_BIN 0xd8, 0xb4, 0xa6, 0xd0, 0x01, 0x13, 0x07, 0xb7, \
  0x67, 0x94, 0xea, 0x66, 0x86, 0x03, 0x6b, 0x43
/* The binary form of SHA-256("test_user:TestRealm:test pass") */
#define USERDIGEST1_SHA256_BIN 0xc3, 0x4e, 0x16, 0x5a, 0x17, 0x0f, 0xe5, \
  0xac, 0x04, 0xf1, 0x6e, 0x46, 0x48, 0x2b, 0xa0, 0xc6, 0x56, 0xc1, 0xfb, \
  0x8f, 0x66, 0xa6, 0xd6, 0x3f, 0x91, 0x12, 0xf8, 0x56, 0xa5, 0xec, 0x6d, \
  0x6d
#define PASSWORD_VALUE "test pass"
#define OPAQUE_VALUE "opaque+content" /* Base64 character set */


#define PAGE \
  "<html><head><title>libmicrohttpd demo page</title>" \
  "</head><body>Access granted</body></html>"

#define DENIED \
  "<html><head><title>libmicrohttpd - Access denied</title>" \
  "</head><body>Access denied</body></html>"

/* Global parameters */
static int verbose;
static int test_oldapi;
static int test_userhash;
static int test_userdigest;
static int test_sha256;
static int test_rfc2069;
/* Bind DAuth nonces to everything except URI */
static int test_bind_all;
/* Bind DAuth nonces to URI */
static int test_bind_uri;
static int curl_uses_usehash;

/* Static helper variables */
static const char userhash1_md5_hex[] = USERHASH1_MD5_HEX;
static const uint8_t userhash1_md5_bin[] = { USERHASH1_MD5_BIN };
static const char userhash1_sha256_hex[] = USERHASH1_SHA256_HEX;
static const uint8_t userhash1_sha256_bin[] = { USERHASH1_SHA256_BIN };
static const char *userhash_hex;
static size_t userhash_hex_len;
static const uint8_t *userhash_bin;
static const uint8_t userdigest1_md5_bin[] = { USERDIGEST1_MD5_BIN };
static const uint8_t userdigest1_sha256_bin[] = { USERDIGEST1_SHA256_BIN };
static const uint8_t *userdigest_bin;
static size_t userdigest_bin_size;
static const char *username_ptr;

static void
test_global_init (void)
{
  libcurl_errbuf[0] = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    externalErrorExit ();

  username_ptr = USERNAME1;
  if (! test_sha256)
  {
    userhash_hex = userhash1_md5_hex;
    userhash_hex_len = MHD_STATICSTR_LEN_ (userhash1_md5_hex);
    userhash_bin = userhash1_md5_bin;
    if ((userhash_hex_len / 2) != \
        (sizeof(userhash1_md5_bin) / sizeof(userhash1_md5_bin[0])))
      externalErrorExitDesc ("Wrong size of the 'userhash1_md5_bin' array");
    userdigest_bin = userdigest1_md5_bin;
    userdigest_bin_size =
      (sizeof(userdigest1_md5_bin) / sizeof(userdigest1_md5_bin[0]));
  }
  else
  {
    userhash_hex = userhash1_sha256_hex;
    userhash_hex_len = MHD_STATICSTR_LEN_ (userhash1_sha256_hex);
    userhash_bin = userhash1_sha256_bin;
    if ((userhash_hex_len / 2) != \
        (sizeof(userhash1_sha256_bin)   \
         / sizeof(userhash1_sha256_bin[0])))
      externalErrorExitDesc ("Wrong size of the 'userhash1_sha256_bin' array");
    userdigest_bin = userdigest1_sha256_bin;
    userdigest_bin_size =
      (sizeof(userdigest1_sha256_bin) / sizeof(userdigest1_sha256_bin[0]));
  }
}


static void
test_global_cleanup (void)
{
  curl_global_cleanup ();
}


static int
gen_good_rnd (void *rnd_buf, size_t rnd_buf_size)
{
  if (1024 < rnd_buf_size)
    externalErrorExitDesc ("Too large amount of random data " \
                           "is requested");
#ifndef _WIN32
  if (1)
  {
    const int urand_fd = open ("/dev/urandom", O_RDONLY);
    if (0 <= urand_fd)
    {
      size_t pos = 0;
      do
      {
        ssize_t res = read (urand_fd,
                            ((uint8_t *) rnd_buf) + pos, rnd_buf_size - pos);
        if (0 > res)
          break;
        pos += (size_t) res;
      } while (rnd_buf_size > pos);
      (void) close (urand_fd);

      if (rnd_buf_size == pos)
        return ! 0; /* Success */
    }
  }
#else  /* _WIN32 */
  if (1)
  {
    HCRYPTPROV cpr_hndl;
    if (CryptAcquireContextW (&cpr_hndl, NULL, NULL, PROV_RSA_FULL,
                              CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
    {
      if (CryptGenRandom (cpr_hndl, (DWORD) rnd_buf_size, (BYTE *) rnd_buf))
      {
        (void) CryptReleaseContext (cpr_hndl, 0);
        return ! 0; /* Success */
      }
      (void) CryptReleaseContext (cpr_hndl, 0);
    }
  }
#endif /* _WIN32 */
  return 0; /* Failure */
}


struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

struct req_track
{
  /**
   * The number of used URI, zero-based
   */
  unsigned int uri_num;

  /**
   * The number of request for URI.
   * This includes number of unauthorised requests.
   */
  unsigned int req_num;
};


static size_t
copyBuffer (void *ptr,
            size_t size,
            size_t nmemb,
            void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    mhdErrorExitDesc ("Wrong too large data");       /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **req_cls)
{
  struct MHD_Response *response;
  enum MHD_Result res;
  static int already_called_marker;
  struct req_track *const tr_p = (struct req_track *) cls;
  (void) url;              /* Unused. Silent compiler warning. */
  (void) method; (void) version; (void) upload_data; /* Unused. Silent compiler warning. */
  (void) upload_data_size; /* Unused. Silent compiler warning. */

  if (&already_called_marker != *req_cls)
  { /* Called for the first time, request not fully read yet */
    *req_cls = &already_called_marker;
    /* Wait for complete request */
    return MHD_YES;
  }

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    mhdErrorExitDesc ("Unexpected HTTP method");

  tr_p->req_num++;
  if (2 < tr_p->req_num)
    mhdErrorExitDesc ("Received more than two requests for the same URI");

  response = NULL;
  if (! test_oldapi)
  {
    struct MHD_DigestAuthInfo *dinfo;
    const enum MHD_DigestAuthAlgo3 algo3 =
      test_sha256 ? MHD_DIGEST_AUTH_ALGO3_SHA256 : MHD_DIGEST_AUTH_ALGO3_MD5;
    const enum MHD_DigestAuthQOP qop =
      test_rfc2069 ? MHD_DIGEST_AUTH_QOP_NONE : MHD_DIGEST_AUTH_QOP_AUTH;

    dinfo = MHD_digest_auth_get_request_info3 (connection);
    if (NULL != dinfo)
    {
      /* Got any kind of Digest response. Check it, it must be valid */
      struct MHD_DigestAuthUsernameInfo *uname;
      enum MHD_DigestAuthResult check_res;
      enum MHD_DigestAuthResult expect_res;

      if (curl_uses_usehash)
      {
        if (MHD_DIGEST_AUTH_UNAME_TYPE_USERHASH != dinfo->uname_type)
        {
          fprintf (stderr, "Unexpected 'uname_type'.\n"
                   "Expected: %d\tRecieved: %d. ",
                   (int) MHD_DIGEST_AUTH_UNAME_TYPE_USERHASH,
                   (int) dinfo->uname_type);
          mhdErrorExitDesc ("Wrong 'uname_type'");
        }
        else if (dinfo->userhash_hex_len != userhash_hex_len)
        {
          fprintf (stderr, "'userhash_hex_len' does not match.\n"
                   "Expected: %u\tRecieved: %u. ",
                   (unsigned) userhash_hex_len,
                   (unsigned) dinfo->userhash_hex_len);
          mhdErrorExitDesc ("Wrong 'userhash_hex_len'");
        }
        else if (0 != memcmp (dinfo->userhash_hex, userhash_hex,
                              dinfo->userhash_hex_len))
        {
          fprintf (stderr, "'userhash_hex' does not match.\n"
                   "Expected: '%s'\tRecieved: '%.*s'. ",
                   userhash_hex,
                   (int) dinfo->userhash_hex_len,
                   dinfo->userhash_hex);
          mhdErrorExitDesc ("Wrong 'userhash_hex'");
        }
        else if (NULL == dinfo->userhash_bin)
          mhdErrorExitDesc ("'userhash_bin' is NULL");
        else if (0 != memcmp (dinfo->userhash_bin, userhash_bin,
                              dinfo->username_len / 2))
          mhdErrorExitDesc ("Wrong 'userhash_bin'");
        else if (NULL != dinfo->username)
          mhdErrorExitDesc ("'username' is NOT NULL");
        else if (0 != dinfo->username_len)
          mhdErrorExitDesc ("'username_len' is NOT zero");
      }
      else
      {
        if (MHD_DIGEST_AUTH_UNAME_TYPE_STANDARD != dinfo->uname_type)
        {
          fprintf (stderr, "Unexpected 'uname_type'.\n"
                   "Expected: %d\tRecieved: %d. ",
                   (int) MHD_DIGEST_AUTH_UNAME_TYPE_STANDARD,
                   (int) dinfo->uname_type);
          mhdErrorExitDesc ("Wrong 'uname_type'");
        }
        else if (NULL == dinfo->username)
          mhdErrorExitDesc ("'username' is NULL");
        else if (dinfo->username_len != strlen (username_ptr))
        {
          fprintf (stderr, "'username_len' does not match.\n"
                   "Expected: %u\tRecieved: %u. ",
                   (unsigned) strlen (username_ptr),
                   (unsigned) dinfo->username_len);
          mhdErrorExitDesc ("Wrong 'username_len'");
        }
        else if (0 != memcmp (dinfo->username, username_ptr,
                              dinfo->username_len))
        {
          fprintf (stderr, "'username' does not match.\n"
                   "Expected: '%s'\tRecieved: '%.*s'. ",
                   username_ptr,
                   (int) dinfo->username_len,
                   dinfo->username);
          mhdErrorExitDesc ("Wrong 'username'");
        }
        else if (NULL != dinfo->userhash_hex)
          mhdErrorExitDesc ("'userhash_hex' is NOT NULL");
        else if (0 != dinfo->userhash_hex_len)
          mhdErrorExitDesc ("'userhash_hex_len' is NOT zero");
        else if (NULL != dinfo->userhash_bin)
          mhdErrorExitDesc ("'userhash_bin' is NOT NULL");
      }
      if (algo3 != dinfo->algo3)
      {
        fprintf (stderr, "Unexpected 'algo3'.\n"
                 "Expected: %d\tRecieved: %d. ",
                 (int) algo3,
                 (int) dinfo->algo3);
        mhdErrorExitDesc ("Wrong 'algo3'");
      }
      if (! test_rfc2069)
      {
        if (
#if CURL_AT_LEAST_VERSION (7,37,1)
          10 >= dinfo->cnonce_len
#else  /* libcurl before 7.37.1 */
          8 > dinfo->cnonce_len
#endif /* libcurl before 7.37.1 */
          )
        {
          fprintf (stderr, "Unexpected small 'cnonce_len': %ld. ",
                   (long) dinfo->cnonce_len);
          mhdErrorExitDesc ("Wrong 'cnonce_len'");
        }
      }
      else
      {
        if (0 != dinfo->cnonce_len)
        {
          fprintf (stderr, "'cnonce_len' is not zero: %ld. ",
                   (long) dinfo->cnonce_len);
          mhdErrorExitDesc ("Wrong 'cnonce_len'");
        }
      }
      if (NULL == dinfo->opaque)
        mhdErrorExitDesc ("'opaque' is NULL");
      else if (dinfo->opaque_len != MHD_STATICSTR_LEN_ (OPAQUE_VALUE))
      {
        fprintf (stderr, "'opaque_len' does not match.\n"
                 "Expected: %u\tRecieved: %u. ",
                 (unsigned) MHD_STATICSTR_LEN_ (OPAQUE_VALUE),
                 (unsigned) dinfo->opaque_len);
        mhdErrorExitDesc ("Wrong 'opaque_len'");
      }
      else if (0 != memcmp (dinfo->opaque, OPAQUE_VALUE, dinfo->opaque_len))
      {
        fprintf (stderr, "'opaque' does not match.\n"
                 "Expected: '%s'\tRecieved: '%.*s'. ",
                 OPAQUE_VALUE,
                 (int) dinfo->opaque_len,
                 dinfo->opaque);
        mhdErrorExitDesc ("Wrong 'opaque'");
      }
      else if (qop != dinfo->qop)
      {
        fprintf (stderr, "Unexpected 'qop'.\n"
                 "Expected: %d\tRecieved: %d. ",
                 (int) qop,
                 (int) dinfo->qop);
        mhdErrorExitDesc ("Wrong 'qop'");
      }
      else if (NULL == dinfo->realm)
        mhdErrorExitDesc ("'realm' is NULL");
      else if (dinfo->realm_len != MHD_STATICSTR_LEN_ (REALM_VAL))
      {
        fprintf (stderr, "'realm_len' does not match.\n"
                 "Expected: %u\tRecieved: %u. ",
                 (unsigned) MHD_STATICSTR_LEN_ (REALM_VAL),
                 (unsigned) dinfo->realm_len);
        mhdErrorExitDesc ("Wrong 'realm_len'");
      }
      else if (0 != memcmp (dinfo->realm, REALM_VAL, dinfo->realm_len))
      {
        fprintf (stderr, "'realm' does not match.\n"
                 "Expected: '%s'\tRecieved: '%.*s'. ",
                 OPAQUE_VALUE,
                 (int) dinfo->realm_len,
                 dinfo->realm);
        mhdErrorExitDesc ("Wrong 'realm'");
      }
      MHD_free (dinfo);

      uname = MHD_digest_auth_get_username3 (connection);
      if (NULL == uname)
        mhdErrorExitDesc ("MHD_digest_auth_get_username3() returned NULL");
      if (curl_uses_usehash)
      {
        if (MHD_DIGEST_AUTH_UNAME_TYPE_USERHASH != uname->uname_type)
        {
          fprintf (stderr, "Unexpected 'uname_type'.\n"
                   "Expected: %d\tRecieved: %d. ",
                   (int) MHD_DIGEST_AUTH_UNAME_TYPE_USERHASH,
                   (int) uname->uname_type);
          mhdErrorExitDesc ("Wrong 'uname_type'");
        }
        else if (uname->userhash_hex_len != userhash_hex_len)
        {
          fprintf (stderr, "'userhash_hex_len' does not match.\n"
                   "Expected: %u\tRecieved: %u. ",
                   (unsigned) userhash_hex_len,
                   (unsigned) uname->userhash_hex_len);
          mhdErrorExitDesc ("Wrong 'userhash_hex_len'");
        }
        else if (0 != memcmp (uname->userhash_hex, userhash_hex,
                              uname->userhash_hex_len))
        {
          fprintf (stderr, "'username' does not match.\n"
                   "Expected: '%s'\tRecieved: '%.*s'. ",
                   userhash_hex,
                   (int) uname->userhash_hex_len,
                   uname->userhash_hex);
          mhdErrorExitDesc ("Wrong 'userhash_hex'");
        }
        else if (NULL == uname->userhash_bin)
          mhdErrorExitDesc ("'userhash_bin' is NULL");
        else if (0 != memcmp (uname->userhash_bin, userhash_bin,
                              uname->username_len / 2))
          mhdErrorExitDesc ("Wrong 'userhash_bin'");
        else if (NULL != uname->username)
          mhdErrorExitDesc ("'username' is NOT NULL");
        else if (0 != uname->username_len)
          mhdErrorExitDesc ("'username_len' is NOT zero");
      }
      else
      {
        if (MHD_DIGEST_AUTH_UNAME_TYPE_STANDARD != uname->uname_type)
        {
          fprintf (stderr, "Unexpected 'uname_type'.\n"
                   "Expected: %d\tRecieved: %d. ",
                   (int) MHD_DIGEST_AUTH_UNAME_TYPE_STANDARD,
                   (int) uname->uname_type);
          mhdErrorExitDesc ("Wrong 'uname_type'");
        }
        else if (NULL == uname->username)
          mhdErrorExitDesc ("'username' is NULL");
        else if (uname->username_len != strlen (username_ptr))
        {
          fprintf (stderr, "'username_len' does not match.\n"
                   "Expected: %u\tRecieved: %u. ",
                   (unsigned) strlen (username_ptr),
                   (unsigned) uname->username_len);
          mhdErrorExitDesc ("Wrong 'username_len'");
        }
        else if (0 != memcmp (uname->username, username_ptr,
                              uname->username_len))
        {
          fprintf (stderr, "'username' does not match.\n"
                   "Expected: '%s'\tRecieved: '%.*s'. ",
                   username_ptr,
                   (int) uname->username_len,
                   uname->username);
          mhdErrorExitDesc ("Wrong 'username'");
        }
        else if (NULL != uname->userhash_hex)
          mhdErrorExitDesc ("'userhash_hex' is NOT NULL");
        else if (0 != uname->userhash_hex_len)
          mhdErrorExitDesc ("'userhash_hex_len' is NOT zero");
        else if (NULL != uname->userhash_bin)
          mhdErrorExitDesc ("'userhash_bin' is NOT NULL");
      }
      if (algo3 != uname->algo3)
      {
        fprintf (stderr, "Unexpected 'algo3'.\n"
                 "Expected: %d\tRecieved: %d. ",
                 (int) algo3,
                 (int) uname->algo3);
        mhdErrorExitDesc ("Wrong 'algo3'");
      }
      MHD_free (uname);

      if (! test_userdigest)
        check_res =
          MHD_digest_auth_check3 (connection, REALM_VAL, username_ptr,
                                  PASSWORD_VALUE,
                                  50 * TIMEOUTS_VAL,
                                  0,
                                  (enum MHD_DigestAuthMultiQOP) qop,
                                  (enum MHD_DigestAuthMultiAlgo3) algo3);
      else
        check_res =
          MHD_digest_auth_check_digest3 (connection, REALM_VAL, username_ptr,
                                         userdigest_bin, userdigest_bin_size,
                                         50 * TIMEOUTS_VAL,
                                         0,
                                         (enum MHD_DigestAuthMultiQOP) qop,
                                         (enum MHD_DigestAuthMultiAlgo3) algo3);

      if (test_rfc2069)
      {
        if ((0 != tr_p->uri_num) && (1 == tr_p->req_num))
          expect_res = MHD_DAUTH_NONCE_STALE;
        else
          expect_res = MHD_DAUTH_OK;
      }
      else if (test_bind_uri)
      {
        if ((0 != tr_p->uri_num) && (1 == tr_p->req_num))
          expect_res = MHD_DAUTH_NONCE_OTHER_COND;
        else
          expect_res = MHD_DAUTH_OK;
      }
      else
        expect_res = MHD_DAUTH_OK;

      switch (check_res)
      {
      /* Conditionally valid results */
      case MHD_DAUTH_OK:
        if (expect_res == MHD_DAUTH_OK)
        {
          if (verbose)
            printf ("Got valid auth check result: MHD_DAUTH_OK.\n");
        }
        else
          mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                            "MHD_DAUTH_OK");
        break;
      case MHD_DAUTH_NONCE_STALE:
        if (expect_res == MHD_DAUTH_NONCE_STALE)
        {
          if (verbose)
            printf ("Got expected auth check result: MHD_DAUTH_NONCE_STALE.\n");
        }
        else
          mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                            "MHD_DAUTH_NONCE_STALE");
        break;
      case MHD_DAUTH_NONCE_OTHER_COND:
        if (expect_res == MHD_DAUTH_NONCE_OTHER_COND)
        {
          if (verbose)
            printf ("Got expected auth check result: "
                    "MHD_DAUTH_NONCE_OTHER_COND.\n");
        }
        else
          mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                            "MHD_DAUTH_NONCE_OTHER_COND");
        break;
      /* Invalid results */
      case MHD_DAUTH_NONCE_WRONG:
        mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                          "MHD_DAUTH_NONCE_WRONG");
        break;
      case MHD_DAUTH_ERROR:
        externalErrorExitDesc ("General error returned " \
                               "by 'MHD_digest_auth_check[_digest]3()'");
        break;
      case MHD_DAUTH_WRONG_USERNAME:
        mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                          "MHD_DAUTH_WRONG_USERNAME");
        break;
      case MHD_DAUTH_RESPONSE_WRONG:
        mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                          "MHD_DAUTH_RESPONSE_WRONG");
        break;
      case MHD_DAUTH_WRONG_HEADER:
        mhdErrorExitDesc ("MHD_digest_auth_check[_digest]3()' returned " \
                          "MHD_DAUTH_WRONG_HEADER");
        break;
      case MHD_DAUTH_WRONG_REALM:
      case MHD_DAUTH_WRONG_URI:
      case MHD_DAUTH_WRONG_QOP:
      case MHD_DAUTH_WRONG_ALGO:
      case MHD_DAUTH_TOO_LARGE:
        fprintf (stderr, "'MHD_digest_auth_check[_digest]3()' returned "
                 "unexpected result: %d. ",
                 check_res);
        mhdErrorExitDesc ("Wrong returned code");
        break;
      default:
        fprintf (stderr, "'MHD_digest_auth_check[_digest]3()' returned "
                 "impossible result code: %d. ",
                 check_res);
        mhdErrorExitDesc ("Impossible returned code");
      }
      fflush (stderr);
      fflush (stdout);

      if (MHD_DAUTH_OK == check_res)
      {
        response =
          MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE),
                                                  (const void *) PAGE);
        if (NULL == response)
          mhdErrorExitDesc ("Response creation failed");

        if (MHD_YES !=
            MHD_queue_response (connection, MHD_HTTP_OK, response))
          mhdErrorExitDesc ("'MHD_queue_response()' failed");
      }
      else if ((MHD_DAUTH_NONCE_STALE == check_res) ||
               (MHD_DAUTH_NONCE_OTHER_COND == check_res))
      {
        response =
          MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (DENIED),
                                                  (const void *) DENIED);
        if (NULL == response)
          mhdErrorExitDesc ("Response creation failed");
        res =
          MHD_queue_auth_required_response3 (connection, REALM_VAL,
                                             OPAQUE_VALUE,
                                             "/", response, 1,
                                             (enum MHD_DigestAuthMultiQOP) qop,
                                             (enum MHD_DigestAuthMultiAlgo3)
                                             algo3,
                                             test_userhash, 0);
        if (MHD_YES != res)
          mhdErrorExitDesc ("'MHD_queue_auth_required_response3()' failed");
      }
      else
        externalErrorExitDesc ("Wrong 'check_res' value");
    }
    else
    {
      /* No Digest auth header */
      if ((1 != tr_p->req_num) || (0 != tr_p->uri_num))
      {
        fprintf (stderr, "Received request number %u for URI number %u "
                 "without Digest Authorisation header. ",
                 tr_p->req_num, tr_p->uri_num + 1);
        mhdErrorExitDesc ("Wrong requests sequence");
      }

      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (DENIED),
                                                (const void *) DENIED);
      if (NULL == response)
        mhdErrorExitDesc ("Response creation failed");
      res =
        MHD_queue_auth_required_response3 (
          connection, REALM_VAL, OPAQUE_VALUE, "/", response, 0,
          (enum MHD_DigestAuthMultiQOP) qop,
          (enum MHD_DigestAuthMultiAlgo3) algo3, test_userhash, 0);
      if (MHD_YES != res)
        mhdErrorExitDesc ("'MHD_queue_auth_required_response3()' failed");
    }
  }
  else if (2 == test_oldapi)
  {
    /* Use old API v2 */
    char *username;
    int check_res;
    int expect_res;

    username = MHD_digest_auth_get_username (connection);
    if (NULL != username)
    { /* Has a valid username in header */
      if (0 != strcmp (username, username_ptr))
      {
        fprintf (stderr, "'username' does not match.\n"
                 "Expected: '%s'\tRecieved: '%s'. ",
                 username_ptr,
                 username);
        mhdErrorExitDesc ("Wrong 'username'");
      }
      MHD_free (username);

      if (! test_userdigest)
        check_res =
          MHD_digest_auth_check2 (connection, REALM_VAL, username_ptr,
                                  PASSWORD_VALUE,
                                  50 * TIMEOUTS_VAL,
                                  test_sha256 ?
                                  MHD_DIGEST_ALG_SHA256 : MHD_DIGEST_ALG_MD5);
      else
        check_res =
          MHD_digest_auth_check_digest2 (connection, REALM_VAL, username_ptr,
                                         userdigest_bin, userdigest_bin_size,
                                         50 * TIMEOUTS_VAL,
                                         test_sha256 ?
                                         MHD_DIGEST_ALG_SHA256 :
                                         MHD_DIGEST_ALG_MD5);

      if (test_bind_uri)
      {
        if ((0 != tr_p->uri_num) && (1 == tr_p->req_num))
          expect_res = MHD_INVALID_NONCE;
        else
          expect_res = MHD_YES;
      }
      else
        expect_res = MHD_YES;

      if (expect_res != check_res)
      {
        fprintf (stderr, "'MHD_digest_auth_check[_digest]2()' returned "
                 "unexpected result '%d', while expected is '%d. ",
                 check_res, expect_res);
        mhdErrorExitDesc ("Wrong 'MHD_digest_auth_check[_digest]2()' result");
      }
      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE),
                                                (const void *) PAGE);
      if (NULL == response)
        mhdErrorExitDesc ("Response creation failed");

      if (MHD_YES == expect_res)
      {
        if (MHD_YES !=
            MHD_queue_response (connection, MHD_HTTP_OK, response))
          mhdErrorExitDesc ("'MHD_queue_response()' failed");
      }
      else if (MHD_INVALID_NONCE == expect_res)
      {
        if (MHD_YES !=
            MHD_queue_auth_fail_response2 (connection, REALM_VAL, OPAQUE_VALUE,
                                           response, 1,
                                           test_sha256 ?
                                           MHD_DIGEST_ALG_SHA256 :
                                           MHD_DIGEST_ALG_MD5))
          mhdErrorExitDesc ("'MHD_queue_auth_fail_response2()' failed");
      }
      else
        externalErrorExitDesc ("Wrong 'check_res' value");
    }
    else
    {
      /* Has no valid username in header */
      if ((1 != tr_p->req_num) || (0 != tr_p->uri_num))
      {
        fprintf (stderr, "Received request number %u for URI number %u "
                 "without Digest Authorisation header. ",
                 tr_p->req_num, tr_p->uri_num + 1);
        mhdErrorExitDesc ("Wrong requests sequence");
      }
      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (DENIED),
                                                (const void *) DENIED);
      if (NULL == response)
        mhdErrorExitDesc ("Response creation failed");

      res = MHD_queue_auth_fail_response2 (connection, REALM_VAL, OPAQUE_VALUE,
                                           response, 0,
                                           test_sha256 ?
                                           MHD_DIGEST_ALG_SHA256 :
                                           MHD_DIGEST_ALG_MD5);
      if (MHD_YES != res)
        mhdErrorExitDesc ("'MHD_queue_auth_fail_response2()' failed");
    }
  }
  else if (1 == test_oldapi)
  {
    /* Use old API v1 */
    char *username;
    int check_res;
    int expect_res;

    username = MHD_digest_auth_get_username (connection);
    if (NULL != username)
    { /* Has a valid username in header */
      if (0 != strcmp (username, username_ptr))
      {
        fprintf (stderr, "'username' does not match.\n"
                 "Expected: '%s'\tRecieved: '%s'. ",
                 username_ptr,
                 username);
        mhdErrorExitDesc ("Wrong 'username'");
      }
      MHD_free (username);

      if (! test_userdigest)
        check_res =
          MHD_digest_auth_check (connection, REALM_VAL, username_ptr,
                                 PASSWORD_VALUE,
                                 50 * TIMEOUTS_VAL);
      else
        check_res =
          MHD_digest_auth_check_digest (connection, REALM_VAL, username_ptr,
                                        userdigest_bin,
                                        50 * TIMEOUTS_VAL);

      if (test_bind_uri)
      {
        if ((0 != tr_p->uri_num) && (1 == tr_p->req_num))
          expect_res = MHD_INVALID_NONCE;
        else
          expect_res = MHD_YES;
      }
      else
        expect_res = MHD_YES;

      if (expect_res != check_res)
      {
        fprintf (stderr, "'MHD_digest_auth_check[_digest]()' returned "
                 "unexpected result '%d', while expected is '%d. ",
                 check_res, expect_res);
        mhdErrorExitDesc ("Wrong 'MHD_digest_auth_check[_digest]()' result");
      }

      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE),
                                                (const void *) PAGE);
      if (NULL == response)
        mhdErrorExitDesc ("Response creation failed");

      if (MHD_YES == expect_res)
      {
        if (MHD_YES !=
            MHD_queue_response (connection, MHD_HTTP_OK, response))
          mhdErrorExitDesc ("'MHD_queue_response()' failed");
      }
      else if (MHD_INVALID_NONCE == expect_res)
      {
        if (MHD_YES !=
            MHD_queue_auth_fail_response (connection, REALM_VAL, OPAQUE_VALUE,
                                          response, 1))
          mhdErrorExitDesc ("'MHD_queue_auth_fail_response()' failed");
      }
      else
        externalErrorExitDesc ("Wrong 'check_res' value");
    }
    else
    {
      /* Has no valid username in header */
      if ((1 != tr_p->req_num) || (0 != tr_p->uri_num))
      {
        fprintf (stderr, "Received request number %u for URI number %u "
                 "without Digest Authorisation header. ",
                 tr_p->req_num, tr_p->uri_num + 1);
        mhdErrorExitDesc ("Wrong requests sequence");
      }
      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (DENIED),
                                                (const void *) DENIED);
      if (NULL == response)
        mhdErrorExitDesc ("Response creation failed");

      res = MHD_queue_auth_fail_response (connection, REALM_VAL, OPAQUE_VALUE,
                                          response, 0);
      if (MHD_YES != res)
        mhdErrorExitDesc ("'MHD_queue_auth_fail_response()' failed");
    }
  }
  else
    externalErrorExitDesc ("Wrong 'test_oldapi' value");

  MHD_destroy_response (response);
  return MHD_YES;
}


/**
 *
 * @param c the CURL handle to use
 * @param port the port to set
 * @param uri_num the number of URI, should be 0, 1 or 2
 */
static void
setCURL_rq_path (CURL *c, uint16_t port, unsigned int uri_num)
{
  const char *req_path;
  char uri[512];
  int res;

  if (0 == uri_num)
    req_path = MHD_URI_BASE_PATH;
  else if (1 == uri_num)
    req_path = MHD_URI_BASE_PATH2;
  else
    req_path = MHD_URI_BASE_PATH3;
  /* A workaround for some old libcurl versions, which ignore the specified
   * port by CURLOPT_PORT when authorisation is used. */
  res = snprintf (uri, (sizeof(uri) / sizeof(uri[0])),
                  "http://127.0.0.1:%u%s", (unsigned int) port,
                  req_path);
  if ((0 >= res) || ((sizeof(uri) / sizeof(uri[0])) <= (size_t) res))
    externalErrorExitDesc ("Cannot form request URL");

  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL, uri))
    libcurlErrorExitDesc ("Cannot set request URL");
}


static CURL *
setupCURL (void *cbc, uint16_t port)
{
  CURL *c;

  c = curl_easy_init ();
  if (NULL == c)
    libcurlErrorExitDesc ("curl_easy_init() failed");

  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                     CURL_HTTP_VERSION_1_1)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER,
                                     libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 0L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPAUTH,
                                     (long) CURLAUTH_DIGEST)) ||
#if CURL_AT_LEAST_VERSION (7,19,1)
      /* Need version 7.19.1 for separate username and password */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_USERNAME, username_ptr)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PASSWORD, PASSWORD_VALUE)) ||
#endif /* CURL_AT_LEAST_VERSION(7,19,1) */
#ifdef _DEBUG
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_VERBOSE, 1L)) ||
#endif /* _DEBUG */
#if CURL_AT_LEAST_VERSION (7, 85, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS_STR, "http")) ||
#elif CURL_AT_LEAST_VERSION (7, 19, 4)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS, CURLPROTO_HTTP)) ||
#endif /* CURL_AT_LEAST_VERSION (7, 19, 4) */
#if CURL_AT_LEAST_VERSION (7, 45, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEFAULT_PROTOCOL, "http")) ||
#endif /* CURL_AT_LEAST_VERSION (7, 45, 0) */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, ((long) port))))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  setCURL_rq_path (c, port, 0);

  return c;
}


static CURLcode
performQueryExternal (struct MHD_Daemon *d, CURL *c, CURLM **multi_reuse)
{
  CURLM *multi;
  time_t start;
  struct timeval tv;
  CURLcode ret;

  ret = CURLE_FAILED_INIT; /* will be replaced with real result */
  if (NULL != *multi_reuse)
    multi = *multi_reuse;
  else
  {
    multi = curl_multi_init ();
    if (multi == NULL)
      libcurlErrorExitDesc ("curl_multi_init() failed");
    *multi_reuse = multi;
  }
  if (CURLM_OK != curl_multi_add_handle (multi, c))
    libcurlErrorExitDesc ("curl_multi_add_handle() failed");

  start = time (NULL);
  while (time (NULL) - start <= TIMEOUTS_VAL)
  {
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket maxMhdSk;
    int maxCurlSk;
    int running;

    maxMhdSk = MHD_INVALID_SOCKET;
    maxCurlSk = -1;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (NULL != multi)
    {
      curl_multi_perform (multi, &running);
      if (0 == running)
      {
        struct CURLMsg *msg;
        int msgLeft;
        int totalMsgs = 0;
        do
        {
          msg = curl_multi_info_read (multi, &msgLeft);
          if (NULL == msg)
            libcurlErrorExitDesc ("curl_multi_info_read() failed");
          totalMsgs++;
          if (CURLMSG_DONE == msg->msg)
            ret = msg->data.result;
        } while (msgLeft > 0);
        if (1 != totalMsgs)
        {
          fprintf (stderr,
                   "curl_multi_info_read returned wrong "
                   "number of results (%d).\n",
                   totalMsgs);
          externalErrorExit ();
        }
        curl_multi_remove_handle (multi, c);
        multi = NULL;
      }
      else
      {
        if (CURLM_OK != curl_multi_fdset (multi, &rs, &ws, &es, &maxCurlSk))
          libcurlErrorExitDesc ("curl_multi_fdset() failed");
      }
    }
    if (NULL == multi)
    { /* libcurl has finished, check whether MHD still needs to perform cleanup */
      if (0 != MHD_get_timeout64s (d))
        break; /* MHD finished as well */
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxMhdSk))
      mhdErrorExitDesc ("MHD_get_fdset() failed");
    tv.tv_sec = 0;
    tv.tv_usec = 200000;
#ifdef MHD_POSIX_SOCKETS
    if (maxMhdSk > maxCurlSk)
      maxCurlSk = maxMhdSk;
#endif /* MHD_POSIX_SOCKETS */
    if (-1 == select (maxCurlSk + 1, &rs, &ws, &es, &tv))
    {
#ifdef MHD_POSIX_SOCKETS
      if (EINTR != errno)
        externalErrorExitDesc ("Unexpected select() error");
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
        externalErrorExitDesc ("Unexpected select() error");
      Sleep (200);
#endif
    }
    if (MHD_YES != MHD_run_from_select (d, &rs, &ws, &es))
      mhdErrorExitDesc ("MHD_run_from_select() failed");
  }

  return ret;
}


/**
 * Check request result
 * @param curl_code the CURL easy return code
 * @param pcbc the pointer struct CBC
 * @return non-zero if success, zero if failed
 */
static unsigned int
check_result (CURLcode curl_code, CURL *c, struct CBC *pcbc)
{
  long code;
  if (CURLE_OK != curl_easy_getinfo (c, CURLINFO_RESPONSE_CODE, &code))
    libcurlErrorExit ();

  if (MHD_HTTP_OK != code)
  {
    fprintf (stderr, "Request returned wrong code: %ld.\n",
             code);
    return 0;
  }

  if (CURLE_OK != curl_code)
  {
    fflush (stdout);
    if (0 != libcurl_errbuf[0])
      fprintf (stderr, "Request failed. "
               "libcurl error: '%s'.\n"
               "libcurl error description: '%s'.\n",
               curl_easy_strerror (curl_code),
               libcurl_errbuf);
    else
      fprintf (stderr, "Request failed. "
               "libcurl error: '%s'.\n",
               curl_easy_strerror (curl_code));
    fflush (stderr);
    return 0;
  }

  if (pcbc->pos != MHD_STATICSTR_LEN_ (PAGE))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) pcbc->pos, (int) pcbc->pos, pcbc->buf,
             (unsigned) MHD_STATICSTR_LEN_ (PAGE));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != memcmp (PAGE, pcbc->buf, pcbc->pos))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ",
             (int) pcbc->pos, pcbc->buf);
    mhdErrorExitDesc ("Wrong returned data");
  }
  return 1;
}


static unsigned int
testDigestAuth (void)
{
  unsigned int dauth_nonce_bind;
  struct MHD_Daemon *d;
  uint16_t port;
  struct CBC cbc;
  struct req_track rq_tr;
  char buf[2048];
  CURL *c;
  CURLM *multi_reuse;
  int failed = 0;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 4210;

  if (1)
  {
    uint8_t salt[8]; /* Use local variable to test MHD "copy" function */
    if (! gen_good_rnd (salt, sizeof(salt)))
    {
      fprintf (stderr, "WARNING: the random buffer (used as salt value) is not "
               "initialised completely, nonce generation may be "
               "predictable in this test.\n");
      fflush (stderr);
    }

    dauth_nonce_bind = MHD_DAUTH_BIND_NONCE_NONE;
    if (test_bind_all)
      dauth_nonce_bind |=
        (MHD_DAUTH_BIND_NONCE_CLIENT_IP | MHD_DAUTH_BIND_NONCE_REALM);
    if (test_bind_uri)
      dauth_nonce_bind |= MHD_DAUTH_BIND_NONCE_URI_PARAMS;

    d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY,
                          port, NULL, NULL,
                          &ahc_echo, &rq_tr,
                          MHD_OPTION_DIGEST_AUTH_RANDOM_COPY,
                          sizeof (salt), salt,
                          MHD_OPTION_NONCE_NC_SIZE, 300,
                          MHD_OPTION_DIGEST_AUTH_NONCE_BIND_TYPE,
                          dauth_nonce_bind,
                          MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                          MHD_OPTION_END);
  }
  if (d == NULL)
    return 1;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;

    dinfo = MHD_get_daemon_info (d,
                                 MHD_DAEMON_INFO_BIND_PORT);
    if ( (NULL == dinfo) ||
         (0 == dinfo->port) )
      mhdErrorExitDesc ("MHD_get_daemon_info() failed");
    port = dinfo->port;
  }

  /* First request */
  rq_tr.req_num = 0;
  rq_tr.uri_num = 0;
  cbc.buf = buf;
  cbc.size = sizeof (buf);
  cbc.pos = 0;
  memset (cbc.buf, 0, cbc.size);
  c = setupCURL (&cbc, port);
  multi_reuse = NULL;
  /* First request */
  if (check_result (performQueryExternal (d, c, &multi_reuse), c, &cbc))
  {
    fflush (stderr);
    if (verbose)
      printf ("Got first expected response.\n");
    fflush (stdout);
  }
  else
  {
    fprintf (stderr, "First request FAILED.\n");
    failed = 1;
  }
  cbc.pos = 0; /* Reset buffer position */
  rq_tr.req_num = 0;
  /* Second request */
  setCURL_rq_path (c, port, ++rq_tr.uri_num);
  if (check_result (performQueryExternal (d, c, &multi_reuse), c, &cbc))
  {
    fflush (stderr);
    if (verbose)
      printf ("Got second expected response.\n");
    fflush (stdout);
  }
  else
  {
    fprintf (stderr, "Second request FAILED.\n");
    failed = 1;
  }
  cbc.pos = 0; /* Reset buffer position */
  rq_tr.req_num = 0;
  /* Third request */
  if (NULL != multi_reuse)
    curl_multi_cleanup (multi_reuse);
  multi_reuse = NULL; /* Force new connection */
  setCURL_rq_path (c, port, ++rq_tr.uri_num);
  if (check_result (performQueryExternal (d, c, &multi_reuse), c, &cbc))
  {
    fflush (stderr);
    if (verbose)
      printf ("Got third expected response.\n");
    fflush (stdout);
  }
  else
  {
    fprintf (stderr, "Third request FAILED.\n");
    failed = 1;
  }

  curl_easy_cleanup (c);
  if (NULL != multi_reuse)
    curl_multi_cleanup (multi_reuse);

  MHD_stop_daemon (d);
  return failed ? 1 : 0;
}


int
main (int argc, char *const *argv)
{
#if ! CURL_AT_LEAST_VERSION (7,19,1)
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
  /* Need version 7.19.1 or newer for separate username and password */
  fprintf (stderr, "Required libcurl at least version 7.19.1"
           " to run this test.\n");
  return 77;
#else  /* CURL_AT_LEAST_VERSION(7,19,1) */
  unsigned int errorCount = 0;
  const curl_version_info_data *const curl_info =
    curl_version_info (CURLVERSION_NOW);
  int curl_sspi;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */

#ifdef NEED_GCRYP_INIT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif /* GCRYCTL_INITIALIZATION_FINISHED */
#endif /* NEED_GCRYP_INIT */
  /* Test type and test parameters */
  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));
  test_oldapi = 0;
  if (has_in_name (argv[0], "_oldapi1"))
    test_oldapi = 1;
  if (has_in_name (argv[0], "_oldapi2"))
    test_oldapi = 2;
  test_userhash = has_in_name (argv[0], "_userhash");
  test_userdigest = has_in_name (argv[0], "_userdigest");
  test_sha256 = has_in_name (argv[0], "_sha256");
  test_rfc2069 = has_in_name (argv[0], "_rfc2069");
  test_bind_all = has_in_name (argv[0], "_bind_all");
  test_bind_uri = has_in_name (argv[0], "_bind_uri");

  /* Wrong test types combinations */
  if (1 == test_oldapi)
  {
    if (test_sha256)
      return 99;
  }
  if (test_oldapi)
  {
    if (test_userhash || test_rfc2069)
      return 99;
  }
  if (test_rfc2069)
  {
    if (test_userhash)
      return 99;
  }

  /* Curl version and known bugs checks */
  curl_sspi = 0;
#ifdef CURL_VERSION_SSPI
  if (0 != (curl_info->features & CURL_VERSION_SSPI))
    curl_sspi = 1;
#endif /* CURL_VERSION_SSPI */

  if ((CURL_VERSION_BITS (7,63,0) > curl_info->version_num) &&
      (CURL_VERSION_BITS (7,62,0) <= curl_info->version_num) )
  {
    fprintf (stderr, "libcurl version 7.62.x has bug in processing "
             "URI with GET arguments for Digest Auth.\n");
    fprintf (stderr, "This test with libcurl %u.%u.%u cannot be performed.\n",
             0xFF & (curl_info->version_num >> 16),
             0xFF & (curl_info->version_num >> 8),
             0xFF & (curl_info->version_num >> 0));
    return 77;
  }

  if (test_sha256)
  {
    if (curl_sspi)
    {
      fprintf (stderr, "Windows SSPI API does not support SHA-256 digests.\n");
      return 77;
    }
    else if (CURL_VERSION_BITS (7,57,0) > curl_info->version_num)
    {
      fprintf (stderr, "Required libcurl at least version 7.57.0 "
               "to run this test with SHA-256.\n");
      fprintf (stderr, "This libcurl version %u.%u.%u "
               "does not support SHA-256.\n",
               0xFF & (curl_info->version_num >> 16),
               0xFF & (curl_info->version_num >> 8),
               0xFF & (curl_info->version_num >> 0));
      return 77;
    }
  }

  if (test_userhash)
  {
    if (curl_sspi)
    {
      printf ("WARNING: Windows SSPI API does not support 'userhash'.\n");
      printf ("This test just checks Digest Auth compatibility with "
              "the clients without 'userhash' support "
              "when 'userhash=true' is specified by MHD.\n");
      curl_uses_usehash = 0;
    }
    else if (CURL_VERSION_BITS (7,57,0) > curl_info->version_num)
    {
      printf ("WARNING: libcurl before version 7.57.0 does not "
              "support 'userhash'.\n");
      printf ("This test just checks Digest Auth compatibility with "
              "libcurl version %u.%u.%u without 'userhash' support "
              "when 'userhash=true' is specified by MHD.\n",
              0xFF & (curl_info->version_num >> 16),
              0xFF & (curl_info->version_num >> 8),
              0xFF & (curl_info->version_num >> 0));
      curl_uses_usehash = 0;
    }
    else if (CURL_VERSION_BITS (7,81,0) > curl_info->version_num)
    {
      fprintf (stderr, "Required libcurl at least version 7.81.0 "
               "to run this test with userhash.\n");
      fprintf (stderr, "This libcurl version %u.%u.%u has broken digest "
               "calculation when userhash is used.\n",
               0xFF & (curl_info->version_num >> 16),
               0xFF & (curl_info->version_num >> 8),
               0xFF & (curl_info->version_num >> 0));
      return 77;
    }
    else
      curl_uses_usehash = ! 0;
  }
  else
    curl_uses_usehash = 0;

  test_global_init ();

  errorCount += testDigestAuth ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  test_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
#endif /* CURL_AT_LEAST_VERSION(7,19,1) */
}
