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
 * @file test_digest_emu_ext.c
 * @brief  Testcase for MHD Digest Authorisation client's header parsing
 * @author Karlson2k (Evgeny Grin)
 *
 * libcurl does not support extended notation for username, so this test
 * "emulates" client request will all valid fields except nonce, cnonce and
 * response (however syntactically these fields valid as well).
 */

#include "MHD_config.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef WINDOWS
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

#define REALM "TestRealm"
/* "titkos szuperügynök" in UTF-8 */
#define USERNAME "titkos szuper" "\xC3\xBC" "gyn" "\xC3\xB6" "k"
/* percent-encoded username */
#define USERNAME_PCTENC "titkos%20szuper%C3%BCgyn%C3%B6k"
#define PASSWORD_VALUE "fake pass"
#define OPAQUE_VALUE "opaque-content"
#define NONCE_EMU "badbadbadbadbadbadbadbadbadbadbadbadbadbadba"
#define CNONCE_EMU "utututututututututututututututututututututs="
#define RESPONSE_EMU "badbadbadbadbadbadbadbadbadbadba"


#define PAGE \
  "<html><head><title>libmicrohttpd demo page</title>" \
  "</head><body>Access granted</body></html>"

#define DENIED \
  "<html><head><title>libmicrohttpd - Access denied</title>" \
  "</head><body>Access denied</body></html>"

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

/* Global parameters */
static int verbose;
static int oldapi;

/* Static helper variables */
static struct curl_slist *curl_headers;

static void
test_global_init (void)
{
  libcurl_errbuf[0] = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    externalErrorExit ();

  curl_headers = NULL;
  curl_headers =
    curl_slist_append (curl_headers,
                       "Authorization: "
                       "Digest username*=UTF-8''" USERNAME_PCTENC ", "
                       "realm=\"" REALM "\", "
                       "nonce=\"" NONCE_EMU "\", "
                       "uri=\"" MHD_URI_BASE_PATH "\", "
                       "cnonce=\"" CNONCE_EMU "\", "
                       "nc=00000001, "
                       "qop=auth, "
                       "response=\"" RESPONSE_EMU "\", "
                       "opaque=\"" OPAQUE_VALUE "\", "
                       "algorithm=MD5");
  if (NULL == curl_headers)
    externalErrorExit ();
}


static void
test_global_cleanup (void)
{
  curl_slist_free_all (curl_headers);
  curl_headers = NULL;
  curl_global_cleanup ();
}


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
  enum MHD_Result ret;
  static int already_called_marker;
  (void) cls; (void) url;         /* Unused. Silent compiler warning. */
  (void) method; (void) version; (void) upload_data; /* Unused. Silent compiler warning. */
  (void) upload_data_size;        /* Unused. Silent compiler warning. */

  if (&already_called_marker != *req_cls)
  { /* Called for the first time, request not fully read yet */
    *req_cls = &already_called_marker;
    /* Wait for complete request */
    return MHD_YES;
  }

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    mhdErrorExitDesc ("Unexpected HTTP method");

  if (! oldapi)
  {
    struct MHD_DigestAuthUsernameInfo *creds;
    struct MHD_DigestAuthInfo *dinfo;
    enum MHD_DigestAuthResult check_res;

    creds = MHD_digest_auth_get_username3 (connection);
    if (NULL == creds)
      mhdErrorExitDesc ("MHD_digest_auth_get_username3() returned NULL");
    else if (MHD_DIGEST_AUTH_UNAME_TYPE_EXTENDED != creds->uname_type)
    {
      fprintf (stderr, "Unexpected 'uname_type'.\n"
               "Expected: %d\tRecieved: %d. ",
               (int) MHD_DIGEST_AUTH_UNAME_TYPE_EXTENDED,
               (int) creds->uname_type);
      mhdErrorExitDesc ("Wrong 'uname_type'");
    }
    else if (NULL == creds->username)
      mhdErrorExitDesc ("'username' is NULL");
    else if (creds->username_len != MHD_STATICSTR_LEN_ (USERNAME))
    {
      fprintf (stderr, "'username_len' does not match.\n"
               "Expected: %u\tRecieved: %u. ",
               (unsigned) MHD_STATICSTR_LEN_ (USERNAME),
               (unsigned) creds->username_len);
      mhdErrorExitDesc ("Wrong 'username_len'");
    }
    else if (0 != memcmp (creds->username, USERNAME, creds->username_len))
    {
      fprintf (stderr, "'username' does not match.\n"
               "Expected: '%s'\tRecieved: '%.*s'. ",
               USERNAME,
               (int) creds->username_len,
               creds->username);
      mhdErrorExitDesc ("Wrong 'username'");
    }
    else if (NULL != creds->userhash_hex)
      mhdErrorExitDesc ("'userhash_hex' is NOT NULL");
    else if (0 != creds->userhash_hex_len)
      mhdErrorExitDesc ("'userhash_hex' is NOT zero");
    else if (NULL != creds->userhash_bin)
      mhdErrorExitDesc ("'userhash_bin' is NOT NULL");

    dinfo = MHD_digest_auth_get_request_info3 (connection);
    if (NULL == dinfo)
      mhdErrorExitDesc ("MHD_digest_auth_get_username3() returned NULL");
    else if (MHD_DIGEST_AUTH_UNAME_TYPE_EXTENDED != dinfo->uname_type)
    {
      fprintf (stderr, "Unexpected 'uname_type'.\n"
               "Expected: %d\tRecieved: %d. ",
               (int) MHD_DIGEST_AUTH_UNAME_TYPE_EXTENDED,
               (int) creds->uname_type);
      mhdErrorExitDesc ("Wrong 'uname_type'");
    }
    else if (NULL == dinfo->username)
      mhdErrorExitDesc ("'username' is NULL");
    else if (dinfo->username_len != MHD_STATICSTR_LEN_ (USERNAME))
    {
      fprintf (stderr, "'username_len' does not match.\n"
               "Expected: %u\tRecieved: %u. ",
               (unsigned) MHD_STATICSTR_LEN_ (USERNAME),
               (unsigned) dinfo->username_len);
      mhdErrorExitDesc ("Wrong 'username_len'");
    }
    else if (0 != memcmp (dinfo->username, USERNAME, dinfo->username_len))
    {
      fprintf (stderr, "'username' does not match.\n"
               "Expected: '%s'\tRecieved: '%.*s'. ",
               USERNAME,
               (int) dinfo->username_len,
               dinfo->username);
      mhdErrorExitDesc ("Wrong 'username'");
    }
    else if (NULL != dinfo->userhash_hex)
      mhdErrorExitDesc ("'userhash_hex' is NOT NULL");
    else if (0 != dinfo->userhash_hex_len)
      mhdErrorExitDesc ("'userhash_hex' is NOT zero");
    else if (NULL != dinfo->userhash_bin)
      mhdErrorExitDesc ("'userhash_bin' is NOT NULL");
    else if (MHD_DIGEST_AUTH_ALGO3_MD5 != dinfo->algo3)
    {
      fprintf (stderr, "Unexpected 'algo'.\n"
               "Expected: %d\tRecieved: %d. ",
               (int) MHD_DIGEST_AUTH_ALGO3_MD5,
               (int) dinfo->algo3);
      mhdErrorExitDesc ("Wrong 'algo'");
    }
    else if (MHD_STATICSTR_LEN_ (CNONCE_EMU) != dinfo->cnonce_len)
    {
      fprintf (stderr, "Unexpected 'cnonce_len'.\n"
               "Expected: %d\tRecieved: %ld. ",
               (int) MHD_STATICSTR_LEN_ (CNONCE_EMU),
               (long) dinfo->cnonce_len);
      mhdErrorExitDesc ("Wrong 'cnonce_len'");
    }
    else if (NULL == dinfo->opaque)
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
    else if (MHD_DIGEST_AUTH_QOP_AUTH != dinfo->qop)
    {
      fprintf (stderr, "Unexpected 'qop'.\n"
               "Expected: %d\tRecieved: %d. ",
               (int) MHD_DIGEST_AUTH_QOP_AUTH,
               (int) dinfo->qop);
      mhdErrorExitDesc ("Wrong 'qop'");
    }
    else if (NULL == dinfo->realm)
      mhdErrorExitDesc ("'realm' is NULL");
    else if (dinfo->realm_len != MHD_STATICSTR_LEN_ (REALM))
    {
      fprintf (stderr, "'realm_len' does not match.\n"
               "Expected: %u\tRecieved: %u. ",
               (unsigned) MHD_STATICSTR_LEN_ (REALM),
               (unsigned) dinfo->realm_len);
      mhdErrorExitDesc ("Wrong 'realm_len'");
    }
    else if (0 != memcmp (dinfo->realm, REALM, dinfo->realm_len))
    {
      fprintf (stderr, "'realm' does not match.\n"
               "Expected: '%s'\tRecieved: '%.*s'. ",
               OPAQUE_VALUE,
               (int) dinfo->realm_len,
               dinfo->realm);
      mhdErrorExitDesc ("Wrong 'realm'");
    }
    MHD_free (creds);
    MHD_free (dinfo);

    check_res = MHD_digest_auth_check3 (connection, REALM, USERNAME,
                                        PASSWORD_VALUE,
                                        50 * TIMEOUTS_VAL,
                                        0, MHD_DIGEST_AUTH_MULT_QOP_AUTH,
                                        MHD_DIGEST_AUTH_MULT_ALGO3_MD5);

    switch (check_res)
    {
    /* Valid results */
    case MHD_DAUTH_NONCE_STALE:
      if (verbose)
        printf ("Got valid auth check result: MHD_DAUTH_NONCE_STALE.\n");
      break;
    case MHD_DAUTH_NONCE_WRONG:
      if (verbose)
        printf ("Got valid auth check result: MHD_DAUTH_NONCE_WRONG.\n");
      break;

    /* Invalid results */
    case MHD_DAUTH_OK:
      mhdErrorExitDesc ("'MHD_digest_auth_check3()' succeed, " \
                        "but it should not");
      break;
    case MHD_DAUTH_ERROR:
      externalErrorExitDesc ("General error returned " \
                             "by 'MHD_digest_auth_check3()'");
      break;
    case MHD_DAUTH_WRONG_USERNAME:
      mhdErrorExitDesc ("MHD_digest_auth_check3()' returned " \
                        "MHD_DAUTH_WRONG_USERNAME");
      break;
    case MHD_DAUTH_RESPONSE_WRONG:
      mhdErrorExitDesc ("MHD_digest_auth_check3()' returned " \
                        "MHD_DAUTH_RESPONSE_WRONG");
      break;
    case MHD_DAUTH_WRONG_HEADER:
    case MHD_DAUTH_WRONG_REALM:
    case MHD_DAUTH_WRONG_URI:
    case MHD_DAUTH_WRONG_QOP:
    case MHD_DAUTH_WRONG_ALGO:
    case MHD_DAUTH_TOO_LARGE:
    case MHD_DAUTH_NONCE_OTHER_COND:
      fprintf (stderr, "'MHD_digest_auth_check3()' returned "
               "unexpected result: %d. ",
               check_res);
      mhdErrorExitDesc ("Wrong returned code");
      break;
    default:
      fprintf (stderr, "'MHD_digest_auth_check3()' returned "
               "impossible result code: %d. ",
               check_res);
      mhdErrorExitDesc ("Impossible returned code");
    }

    response =
      MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (DENIED),
                                              (const void *) DENIED);
    if (NULL == response)
      mhdErrorExitDesc ("Response creation failed");
    ret = MHD_queue_auth_fail_response2 (connection, REALM, OPAQUE_VALUE,
                                         response, 0, MHD_DIGEST_ALG_MD5);
    if (MHD_YES != ret)
      mhdErrorExitDesc ("'MHD_queue_auth_fail_response2()' failed");
  }
  else
  {
    char *username;
    int check_res;

    username = MHD_digest_auth_get_username (connection);
    if (NULL == username)
      mhdErrorExitDesc ("'MHD_digest_auth_get_username()' returned NULL");
    else if (0 != strcmp (username, USERNAME))
    {
      fprintf (stderr, "'username' does not match.\n"
               "Expected: '%s'\tRecieved: '%s'. ",
               USERNAME,
               username);
      mhdErrorExitDesc ("Wrong 'username'");
    }
    MHD_free (username);

    check_res = MHD_digest_auth_check (connection, REALM, USERNAME,
                                       PASSWORD_VALUE,
                                       300);

    if (MHD_INVALID_NONCE != check_res)
    {
      fprintf (stderr, "'MHD_digest_auth_check()' returned unexpected"
               " result: %d. ", check_res);
      mhdErrorExitDesc ("Wrong 'MHD_digest_auth_check()' result");
    }
    response =
      MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (DENIED),
                                              (const void *) DENIED);
    if (NULL == response)
      mhdErrorExitDesc ("Response creation failed");

    ret = MHD_queue_auth_fail_response (connection, REALM, OPAQUE_VALUE,
                                        response, 0);
    if (MHD_YES != ret)
      mhdErrorExitDesc ("'MHD_queue_auth_fail_response()' failed");
  }

  MHD_destroy_response (response);
  return ret;
}


static CURL *
setupCURL (void *cbc, uint16_t port)
{
  CURL *c;
  char url[512];

  if (1)
  {
    int res;
    /* A workaround for some old libcurl versions, which ignore the specified
     * port by CURLOPT_PORT when authorisation is used. */
    res = snprintf (url, (sizeof(url) / sizeof(url[0])),
                    "http://127.0.0.1:%u%s",
                    (unsigned int) port, MHD_URI_BASE_PATH);
    if ((0 >= res) || ((sizeof(url) / sizeof(url[0])) <= (size_t) res))
      externalErrorExitDesc ("Cannot form request URL");
  }

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
      /* (CURLE_OK != curl_easy_setopt (c, CURLOPT_VERBOSE, 1L)) || */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 0L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, curl_headers)) ||
#if CURL_AT_LEAST_VERSION (7, 85, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS_STR, "http")) ||
#elif CURL_AT_LEAST_VERSION (7, 19, 4)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS, CURLPROTO_HTTP)) ||
#endif /* CURL_AT_LEAST_VERSION (7, 19, 4) */
#if CURL_AT_LEAST_VERSION (7, 45, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEFAULT_PROTOCOL, "http")) ||
#endif /* CURL_AT_LEAST_VERSION (7, 45, 0) */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, ((long) port))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL, url)))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");
  return c;
}


static CURLcode
performQueryExternal (struct MHD_Daemon *d, CURL *c)
{
  CURLM *multi;
  time_t start;
  struct timeval tv;
  CURLcode ret;

  ret = CURLE_FAILED_INIT; /* will be replaced with real result */
  multi = NULL;
  multi = curl_multi_init ();
  if (multi == NULL)
    libcurlErrorExitDesc ("curl_multi_init() failed");
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
        curl_multi_cleanup (multi);
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

  if (401 != code)
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

  if (pcbc->pos != strlen (DENIED))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) pcbc->pos, (int) pcbc->pos, pcbc->buf,
             (unsigned) strlen (DENIED));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != memcmp (DENIED, pcbc->buf, pcbc->pos))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ",
             (int) pcbc->pos, pcbc->buf);
    mhdErrorExitDesc ("Wrong returned data");
  }
  return 1;
}


static unsigned int
testDigestAuthEmu (void)
{
  struct MHD_Daemon *d;
  uint16_t port;
  struct CBC cbc;
  char buf[2048];
  CURL *c;
  int failed = 0;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 4210;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY,
                        port, NULL, NULL,
                        &ahc_echo, NULL,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);
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
    port = (uint16_t) dinfo->port;
  }

  /* First request */
  cbc.buf = buf;
  cbc.size = sizeof (buf);
  cbc.pos = 0;
  memset (cbc.buf, 0, cbc.size);
  c = setupCURL (&cbc, port);
  if (check_result (performQueryExternal (d, c), c, &cbc))
  {
    if (verbose)
      printf ("Got expected response.\n");
  }
  else
  {
    fprintf (stderr, "Request FAILED.\n");
    failed = 1;
  }
  curl_easy_cleanup (c);

  MHD_stop_daemon (d);
  return failed ? 1 : 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */

  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));
  oldapi = has_in_name (argv[0], "_oldapi");
  test_global_init ();

  errorCount += testDigestAuthEmu ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  test_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
