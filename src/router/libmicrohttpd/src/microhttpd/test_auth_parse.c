/*
  This file is part of libmicrohttpd
  Copyright (C) 2022-2023 Karlson2k (Evgeny Grin)

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
 * @file microhttpd/test_auth_parse.c
 * @brief  Unit tests for request's 'Authorization" headers parsing
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "gen_auth.h"
#ifdef BAUTH_SUPPORT
#include "basicauth.h"
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
#include "digestauth.h"
#endif /* DAUTH_SUPPORT */
#include "mhd_assert.h"
#include "internal.h"
#include "connection.h"


#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */


#if defined(HAVE___FUNC__)
#define externalErrorExit(ignore) \
    _externalErrorExit_func(NULL, __func__, __LINE__)
#define externalErrorExitDesc(errDesc) \
    _externalErrorExit_func(errDesc, __func__, __LINE__)
#define mhdErrorExit(ignore) \
    _mhdErrorExit_func(NULL, __func__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
    _mhdErrorExit_func(errDesc, __func__, __LINE__)
#elif defined(HAVE___FUNCTION__)
#define externalErrorExit(ignore) \
    _externalErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define externalErrorExitDesc(errDesc) \
    _externalErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#define mhdErrorExit(ignore) \
    _mhdErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
    _mhdErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func(NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func(errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func(NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func(errDesc, NULL, __LINE__)
#endif


_MHD_NORETURN static void
_externalErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
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


_MHD_NORETURN static void
_mhdErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
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

  fflush (stderr);
  exit (8);
}


/* Declarations for local replacements of MHD functions */
/* None, headers are included */

/* Local replacements implementations */

void *
MHD_connection_alloc_memory_ (struct MHD_Connection *connection,
                              size_t size)
{
  void *ret;
  if (NULL == connection)
    mhdErrorExitDesc ("'connection' parameter is NULL");
  /* Use 'read_buffer' just as a flag */
  if (NULL != connection->read_buffer)
  {
    /* Use 'write_buffer' just as a flag */
    if (NULL != connection->write_buffer)
      mhdErrorExitDesc ("Unexpected third memory allocation, " \
                        "while previous allocations was not freed");
  }
  /* Just use simple "malloc()" here */
  ret = malloc (size);
  if (NULL == ret)
    externalErrorExit ();

  /* Track up to two allocations */
  if (NULL == connection->read_buffer)
    connection->read_buffer = (char *) ret;
  else
    connection->write_buffer = (char *) ret;
  return ret;
}


/**
 * Static variable to avoid additional malloc()/free() pairs
 */
static struct MHD_Connection conn;

void
MHD_DLOG (const struct MHD_Daemon *daemon,
          const char *format,
          ...)
{
  (void) daemon;
  if (! conn.rq.client_aware)
  {
    fprintf (stderr, "Unexpected call of 'MHD_LOG(), format is '%s'.\n",
             format);
    fprintf (stderr, "'Authorization' header value: '%s'.\n",
             (NULL == conn.rq.headers_received) ?
             "NULL" : (conn.rq.headers_received->value));
    mhdErrorExit ();
  }
  conn.rq.client_aware = false; /* Clear the flag */
  return;
}


/**
 * Static variable to avoid additional malloc()/free() pairs
 */
static struct MHD_HTTP_Req_Header req_header;

static void
test_global_init (void)
{
  memset (&conn, 0, sizeof(conn));
  memset (&req_header, 0, sizeof(req_header));
}


/**
 * Add "Authorization" client test header.
 *
 * @param hdr the pointer to the headr value, must be valid until end of
 *            checking of this header
 * @param hdr_len the length of the @a hdr
 * @note The function is NOT thread-safe
 */
static void
add_AuthHeader (const char *hdr, size_t hdr_len)
{
  if ((NULL != conn.rq.headers_received) ||
      (NULL != conn.rq.headers_received_tail))
    externalErrorExitDesc ("Connection's test headers are not empty already");
  if (NULL != hdr)
  {
    /* Skip initial whitespaces, emulate MHD's headers processing */
    while (' ' == hdr[0] || '\t' == hdr[0])
    {
      hdr++;
      hdr_len--;
    }
    req_header.header = MHD_HTTP_HEADER_AUTHORIZATION; /* Static string */
    req_header.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_AUTHORIZATION);
    req_header.value = hdr;
    req_header.value_size = hdr_len;
    req_header.kind = MHD_HEADER_KIND;
    req_header.prev = NULL;
    req_header.next = NULL;
    conn.rq.headers_received = &req_header;
    conn.rq.headers_received_tail = &req_header;
  }
  else
  {
    conn.rq.headers_received = NULL;
    conn.rq.headers_received_tail = NULL;
  }
  conn.state = MHD_CONNECTION_FULL_REQ_RECEIVED; /* Should be a typical value */
}


#ifdef BAUTH_SUPPORT
/**
 * Parse previously added Basic Authorization client header and return
 * result of the parsing.
 *
 * Function performs basic checking of the parsing result
 * @return result of header parsing
 * @note The function is NOT thread-safe
 */
static const struct MHD_RqBAuth *
get_BAuthRqParams (void)
{
  const struct MHD_RqBAuth *res1;
  const struct MHD_RqBAuth *res2;
  /* Store pointer in some member unused in this test */
  res1 = MHD_get_rq_bauth_params_ (&conn);
  if (! conn.rq.bauth_tried)
    mhdErrorExitDesc ("'rq.bauth_tried' is not set");
  res2 = MHD_get_rq_bauth_params_ (&conn);
  if (res1 != res2)
    mhdErrorExitDesc ("MHD_get_rq_bauth_params_() returned another pointer " \
                      "when called for the second time");
  return res2;
}


#endif /* BAUTH_SUPPORT */

#ifdef DAUTH_SUPPORT
/**
 * Parse previously added Digest Authorization client header and return
 * result of the parsing.
 *
 * Function performs basic checking of the parsing result
 * @return result of header parsing
 * @note The function is NOT thread-safe
 */
static const struct MHD_RqDAuth *
get_DAuthRqParams (void)
{
  const struct MHD_RqDAuth *res1;
  const struct MHD_RqDAuth *res2;
  /* Store pointer in some member unused in this test */
  res1 = MHD_get_rq_dauth_params_ (&conn);
  if (! conn.rq.dauth_tried)
    mhdErrorExitDesc ("'rq.dauth_tried' is not set");
  res2 = MHD_get_rq_dauth_params_ (&conn);
  if (res1 != res2)
    mhdErrorExitDesc ("MHD_get_rq_bauth_params_() returned another pointer " \
                      "when called for the second time");
  return res2;
}


#endif /* DAUTH_SUPPORT */


static void
clean_AuthHeaders (void)
{
  conn.state = MHD_CONNECTION_INIT;
  free (conn.read_buffer);
  free (conn.write_buffer);

#ifdef BAUTH_SUPPORT
  conn.rq.bauth_tried = false;
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  conn.rq.dauth_tried = false;
#endif /* BAUTH_SUPPORT */

#ifdef BAUTH_SUPPORT
  if ((NULL != conn.rq.bauth) &&
      (conn.read_buffer != (const char *) conn.rq.bauth) &&
      (conn.write_buffer != (const char *) conn.rq.bauth))
    externalErrorExitDesc ("Memory allocation is not tracked as it should be");
  conn.rq.bauth = NULL;
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  if ((NULL != conn.rq.dauth) &&
      (conn.read_buffer != (const char *) conn.rq.dauth) &&
      (conn.write_buffer != (const char *) conn.rq.dauth))
    externalErrorExitDesc ("Memory allocation is not tracked as it should be");
  conn.rq.dauth = NULL;
#endif /* BAUTH_SUPPORT */

  conn.rq.headers_received = NULL;
  conn.rq.headers_received_tail = NULL;

  conn.read_buffer = NULL;
  conn.write_buffer = NULL;
  conn.rq.client_aware = false;
}


enum MHD_TestAuthType
{
  MHD_TEST_AUTHTYPE_NONE,
  MHD_TEST_AUTHTYPE_BASIC,
  MHD_TEST_AUTHTYPE_DIGEST,
};


/* return zero if succeed, non-zero otherwise */
static unsigned int
expect_result_type_n (const char *hdr, size_t hdr_len,
                      const enum MHD_TestAuthType expected_type,
                      int expect_log,
                      unsigned int line_num)
{
  unsigned int ret;

  ret = 0;
  add_AuthHeader (hdr, hdr_len);
  if (expect_log)
    conn.rq.client_aware = true; /* Use like a flag */
  else
    conn.rq.client_aware = false;
#ifdef BAUTH_SUPPORT
  if (MHD_TEST_AUTHTYPE_BASIC == expected_type)
  {
    if (NULL == get_BAuthRqParams ())
    {
      fprintf (stderr,
               "'Authorization' header parsing FAILED:\n"
               "Basic Authorization was not found, while it should be.\n");
      ret++;
    }
  }
  else
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  if (MHD_TEST_AUTHTYPE_DIGEST == expected_type)
  {
    if (NULL == get_DAuthRqParams ())
    {
      fprintf (stderr,
               "'Authorization' header parsing FAILED:\n"
               "Digest Authorization was not found, while it should be.\n");
      ret++;
    }
  }
  else
#endif /* BAUTH_SUPPORT */
  {
#ifdef BAUTH_SUPPORT
    if (NULL != get_BAuthRqParams ())
    {
      fprintf (stderr,
               "'Authorization' header parsing FAILED:\n"
               "Found Basic Authorization, while it should not be.\n");
      ret++;
    }
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
    if (NULL != get_DAuthRqParams ())
    {
      fprintf (stderr,
               "'Authorization' header parsing FAILED:\n"
               "Found Digest Authorization, while it should not be.\n");
      ret++;
    }
#endif /* DAUTH_SUPPORT */
  }
#if defined(BAUTH_SUPPORT) && defined(DAUTH_SUPPORT)
  if (conn.rq.client_aware)
  {
    fprintf (stderr,
             "'Authorization' header parsing ERROR:\n"
             "Log function must be called, but it was not.\n");
    ret++;
  }
#endif /* BAUTH_SUPPORT && DAUTH_SUPPORT */

  if (ret)
  {
    if (NULL == hdr)
      fprintf (stderr,
               "Input: Absence of 'Authorization' header.\n");
    else if (0 == hdr_len)
      fprintf (stderr,
               "Input: empty 'Authorization' header.\n");
    else
      fprintf (stderr,
               "Input Header: '%.*s'\n", (int) hdr_len, hdr);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret = 1;
  }
  clean_AuthHeaders ();

  return ret;
}


#define expect_result_type(h,t,l) \
    expect_result_type_n(h,MHD_STATICSTR_LEN_(h),t,l,__LINE__)


static unsigned int
check_type (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_result_type_n (NULL, 0, MHD_TEST_AUTHTYPE_NONE, 0, __LINE__);

  r += expect_result_type ("", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("    ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" \t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t \t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" \t ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" \t \t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t \t ", MHD_TEST_AUTHTYPE_NONE, 0);

  r += expect_result_type ("Basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" Basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\tBasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t Basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" \tBasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("    Basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t\tBasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \tBasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \t Basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("Basic ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("Basic \t", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("Basic \t ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("Basic 123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("Basic \t123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("Basic  abc ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" bAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\tbAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t bAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" \tbAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("    bAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t\tbAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \tbAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \t bAsIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC \t", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC \t ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC 123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC \t123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("bAsIC  abc ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\tbasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" \tbasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("    basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t\tbasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \tbasic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \t basic", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic \t", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic \t ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic 123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic \t123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("basic  abc ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" BASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\tBASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t BASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type (" \tBASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("    BASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t\tBASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \tBASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("\t\t  \t BASIC", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC \t", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC \t ", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC 123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC \t123", MHD_TEST_AUTHTYPE_BASIC, 0);
  r += expect_result_type ("BASIC  abc ", MHD_TEST_AUTHTYPE_BASIC, 0);
  /* Only single token is allowed for 'Basic' Authorization */
  r += expect_result_type ("Basic a b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic a\tb", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic a\tb", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic abc1 b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic c abc1", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic c abc1 ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic c abc1\t", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic c\tabc1\t", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic c abc1 b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic zyx, b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic zyx,b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic zyx ,b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic zyx;b", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Basic zyx; b", MHD_TEST_AUTHTYPE_NONE, 1);

  r += expect_result_type ("Basic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" Basic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" Basic2 ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\tBasic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t Basic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" \tBasic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("    Basic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t\t\tBasic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t\t  \tBasic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t\t  \t Basic2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic2 ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic2 \t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic2 \t ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic2 123", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic2 \t123", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic2  abc ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("BasicBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" BasicBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\tBasicBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\t BasicBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" \tBasicBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("BasicBasic ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("BasicBasic \t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("BasicBasic \t\t", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("BasicDigest", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" BasicDigest", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("BasicDigest ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Basic\0", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\0" "Basic", MHD_TEST_AUTHTYPE_NONE, 0);

  r += expect_result_type ("Digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" Digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tDigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t Digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" \tDigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("    Digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t\tDigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \tDigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \t Digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tDigest ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("  Digest \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t \tDigest \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);

  r += expect_result_type ("digEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" digEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tdigEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t digEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" \tdigEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("    digEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t\tdigEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \tdigEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \t digEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digEST ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digEST \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digEST \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tdigEST ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("  digEST \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t \tdigEST \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tdigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" \tdigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("    digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t\tdigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \tdigest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \t digest", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digest ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digest \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("digest \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tdigest ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("  digest \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t \tdigest \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("DIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" DIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tDIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t DIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type (" \tDIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("    DIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t\tDIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \tDIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t\t  \t DIGEST", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("DIGEST ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("DIGEST \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("DIGEST \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\tDIGEST ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("  DIGEST \t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("\t \tDIGEST \t ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,\t", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,  ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest   ,  ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,\t, ,\t, ,\t, ,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,\t,\t,\t,\t,\t,\t,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a=b", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a=\"b\"", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc=1", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc=\"1\"", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a=b ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a=\"b\" ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc=1 ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc=\"1\" ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a = b", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a\t=\t\"b\"", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc =1", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc= \"1\"", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a=\tb ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest a = \"b\" ", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc\t\t\t= 1 ", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc   =\t\t\t\"1\" ", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc =1,,,,", MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest nc =1  ,,,,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,,nc= \"1 \"", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,,  nc= \" 1\"", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,, nc= \"1\",,,,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,, nc= \"1\"  ,,,,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,, nc= \"1\"  ,,,,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,, nc= \"1\"  ,,,,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);
  r += expect_result_type ("Digest ,,,, nc= \"1\"  ,,,,,", \
                           MHD_TEST_AUTHTYPE_DIGEST, 0);

  r += expect_result_type ("Digest nc", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest   nc", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc  ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc  ,", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc  , ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest \tnc\t  ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest \tnc\t  ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc,", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc,uri", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1,uri", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1,uri   ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1,uri,", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1, uri,", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1,uri   ,", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1,uri   , ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  /* Binary zero */
  r += expect_result_type ("Digest nc=1\0", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1\0" " ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1\t\0", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\0" "1", MHD_TEST_AUTHTYPE_NONE, 1);
  /* Semicolon */
  r += expect_result_type ("Digest nc=1;", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1; ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=;1", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc;=1", MHD_TEST_AUTHTYPE_NONE, 1);
  /* The equal sign alone */
  r += expect_result_type ("Digest =", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest   =", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest   =  ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest ,=", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest , =", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest ,= ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest , = ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1,=", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=1, =", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=bar,=", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=bar, =", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  /* Unclosed quotation */
  r += expect_result_type ("Digest nc=\"", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"abc", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"   ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"abc   ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"   abc", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"   abc", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"\\", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"\\\"", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"  \\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"\\\"  ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"  \\\"  ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc=\"\\\"\\\"\\\"\\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"abc", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"   ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"abc   ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"   abc", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"   abc", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"\\", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"\\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"  \\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"\\\"  ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"  \\\"  ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest nc= \"\\\"\\\"\\\"\\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"bar", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"   ", MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"bar   ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"   bar", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"   bar", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo= \"   bar", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\",   bar", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"   bar,", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"\\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"  \\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"\\\"  ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"  \\\"  ", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest foo=\"\\\"\\\"\\\"\\\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  /* Full set of parameters with semicolon inside */
  r += expect_result_type ("Digest username=\"test@example.com\", " \
                           "realm=\"users@example.com\", nonce=\"32141232413abcde\", " \
                           "uri=\"/example\", qop=auth, nc=00000001; cnonce=\"0a4f113b\", " \
                           "response=\"6629fae49393a05397450978507c4ef1\", " \
                           "opaque=\"sadfljk32sdaf\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest username=\"test@example.com\", " \
                           "realm=\"users@example.com\", nonce=\"32141232413abcde\", " \
                           "uri=\"/example\", qop=auth, nc=00000001;cnonce=\"0a4f113b\", " \
                           "response=\"6629fae49393a05397450978507c4ef1\", " \
                           "opaque=\"sadfljk32sdaf\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);
  r += expect_result_type ("Digest username;=\"test@example.com\", " \
                           "realm=\"users@example.com\", nonce=\"32141232413abcde\", " \
                           "uri=\"/example\", qop=auth, nc=00000001, cnonce=\"0a4f113b\", " \
                           "response=\"6629fae49393a05397450978507c4ef1\", " \
                           "opaque=\"sadfljk32sdaf\"", \
                           MHD_TEST_AUTHTYPE_NONE, 1);

  r += expect_result_type ("Digest2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("2Digest", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Digest" "a", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("a" "Digest", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" Digest2", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" 2Digest", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" Digest" "a", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" a" "Digest", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Digest2 ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("2Digest ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Digest" "a", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("a" "Digest ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("DigestBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("DigestBasic ", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type (" DigestBasic", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("DigestBasic" "a", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("Digest" "\0", MHD_TEST_AUTHTYPE_NONE, 0);
  r += expect_result_type ("\0" "Digest", MHD_TEST_AUTHTYPE_NONE, 0);
  return r;
}


#ifdef BAUTH_SUPPORT

/* return zero if succeed, 1 otherwise */
static unsigned int
expect_basic_n (const char *hdr, size_t hdr_len,
                const char *tkn, size_t tkn_len,
                unsigned int line_num)
{
  const struct MHD_RqBAuth *h;
  unsigned int ret;

  mhd_assert (NULL != hdr);
  mhd_assert (0 != hdr_len);

  add_AuthHeader (hdr, hdr_len);
  h = get_BAuthRqParams ();
  if (NULL == h)
    mhdErrorExitDesc ("'MHD_get_rq_bauth_params_()' returned NULL");
  ret = 1;
  if (tkn_len != h->token68.len)
    fprintf (stderr,
             "'Authorization' header parsing FAILED:\n"
             "Wrong token length:\tRESULT[%u]: %.*s\tEXPECTED[%u]: %.*s\n",
             (unsigned) h->token68.len,
             (int) h->token68.len,
             h->token68.str ?
             h->token68.str : "(NULL)",
             (unsigned) tkn_len, (int) tkn_len, tkn ? tkn : "(NULL)");
  else if ( ((NULL == tkn) != (NULL == h->token68.str)) ||
            ((NULL != tkn) &&
             (0 != memcmp (tkn, h->token68.str, tkn_len))) )
    fprintf (stderr,
             "'Authorization' header parsing FAILED:\n"
             "Wrong token string:\tRESULT[%u]: %.*s\tEXPECTED[%u]: %.*s\n",
             (unsigned) h->token68.len,
             (int) h->token68.len,
             h->token68.str ?
             h->token68.str : "(NULL)",
             (unsigned) tkn_len, (int) tkn_len, tkn ? tkn : "(NULL)");
  else
    ret = 0;
  if (0 != ret)
  {
    fprintf (stderr,
             "Input Header: '%.*s'\n", (int) hdr_len, hdr);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }
  clean_AuthHeaders ();

  return ret;
}


#define expect_basic(h,t) \
    expect_basic_n(h,MHD_STATICSTR_LEN_(h),t,MHD_STATICSTR_LEN_(t),__LINE__)

static unsigned int
check_basic (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_basic ("Basic a", "a");
  r += expect_basic ("Basic    a", "a");
  r += expect_basic ("Basic \ta", "a");
  r += expect_basic ("Basic \ta\t", "a");
  r += expect_basic ("Basic \ta ", "a");
  r += expect_basic ("Basic  a ", "a");
  r += expect_basic ("Basic \t a\t ", "a");
  r += expect_basic ("Basic \t abc\t ", "abc");
  r += expect_basic ("Basic 2143sdfa4325sdfgfdab354354314SDSDFc", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("Basic 2143sdfa4325sdfgfdab354354314SDSDFc  ", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("Basic   2143sdfa4325sdfgfdab354354314SDSDFc", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("Basic   2143sdfa4325sdfgfdab354354314SDSDFc  ", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("  Basic 2143sdfa4325sdfgfdab354354314SDSDFc", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("  Basic  2143sdfa4325sdfgfdab354354314SDSDFc", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("  Basic 2143sdfa4325sdfgfdab354354314SDSDFc ", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("  Basic  2143sdfa4325sdfgfdab354354314SDSDFc ", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("  Basic  2143sdfa4325sdfgfdab354354314SDSDFc  ", \
                     "2143sdfa4325sdfgfdab354354314SDSDFc");
  r += expect_basic ("Basic -A.1-z~9+/=====", "-A.1-z~9+/=====");
  r += expect_basic ("  Basic   -A.1-z~9+/===== ", "-A.1-z~9+/=====");

  r += expect_basic_n ("Basic", MHD_STATICSTR_LEN_ ("Basic"), NULL, 0,__LINE__);
  r += expect_basic_n ("   Basic", MHD_STATICSTR_LEN_ ("   Basic"), NULL, 0,
                       __LINE__);
  r += expect_basic_n ("Basic   ", MHD_STATICSTR_LEN_ ("Basic   "), NULL, 0,
                       __LINE__);
  r += expect_basic_n ("Basic \t\t", MHD_STATICSTR_LEN_ ("Basic \t\t"), NULL, 0,
                       __LINE__);

  return r;
}


#endif /* BAUTH_SUPPORT */


#ifdef DAUTH_SUPPORT

/* return zero if succeed, 1 otherwise */
static unsigned int
cmp_dauth_param (const char *pname, const struct MHD_RqDAuthParam *param,
                 const char *expected_value)
{
  unsigned int ret;
  size_t expected_len;
  bool expected_quoted;
  mhd_assert (NULL != param);
  mhd_assert (NULL != pname);
  ret = 0;

  if (NULL == expected_value)
  {
    expected_len = 0;
    expected_quoted = false;
    if (NULL != param->value.str)
      ret = 1;
    else if (param->value.len != expected_len)
      ret = 1;
    else if (param->quoted != expected_quoted)
      ret = 1;
  }
  else
  {
    expected_len = strlen (expected_value);
    expected_quoted = (NULL != memchr (expected_value, '\\', expected_len));
    if (NULL == param->value.str)
      ret = 1;
    else if (param->value.len != expected_len)
      ret = 1;
    else if (param->quoted != expected_quoted)
      ret = 1;
    else if (0 != memcmp (param->value.str, expected_value, expected_len))
      ret = 1;
  }
  if (0 != ret)
  {
    fprintf (stderr, "Parameter '%s' parsed incorrectly:\n", pname);
    fprintf (stderr, "\tRESULT  :\tvalue.str: %.*s",
             (int) (param->value.str ? param->value.len : 6),
             param->value.str ? param->value.str : "(NULL)");
    fprintf (stderr, "\tvalue.len: %u",
             (unsigned) param->value.len);
    fprintf (stderr, "\tquoted: %s\n",
             (unsigned) param->quoted ? "true" : "false");
    fprintf (stderr, "\tEXPECTED:\tvalue.str: %.*s",
             (int) (expected_value ? expected_len : 6),
             expected_value ? expected_value : "(NULL)");
    fprintf (stderr, "\tvalue.len: %u",
             (unsigned) expected_len);
    fprintf (stderr, "\tquoted: %s\n",
             (unsigned) expected_quoted ? "true" : "false");
  }
  return ret;
}


/* return zero if succeed, 1 otherwise */
static unsigned int
expect_digest_n (const char *hdr, size_t hdr_len,
                 const char *nonce,
                 enum MHD_DigestAuthAlgo3 algo3,
                 const char *response,
                 const char *username,
                 const char *username_ext,
                 const char *realm,
                 const char *uri,
                 const char *qop_raw,
                 enum MHD_DigestAuthQOP qop,
                 const char *cnonce,
                 const char *nc,
                 int userhash,
                 unsigned int line_num)
{
  const struct MHD_RqDAuth *h;
  unsigned int ret;

  mhd_assert (NULL != hdr);
  mhd_assert (0 != hdr_len);

  add_AuthHeader (hdr, hdr_len);

  h = get_DAuthRqParams ();
  if (NULL == h)
    mhdErrorExitDesc ("'MHD_get_rq_dauth_params_()' returned NULL");
  ret = 0;

  ret += cmp_dauth_param ("nonce", &h->nonce, nonce);
  if (h->algo3 != algo3)
  {
    ret += 1;
    fprintf (stderr, "Parameter 'algorithm' detected incorrectly:\n");
    fprintf (stderr, "\tRESULT  :\t%u\n",
             (unsigned) h->algo3);
    fprintf (stderr, "\tEXPECTED:\t%u\n",
             (unsigned) algo3);
  }
  ret += cmp_dauth_param ("response", &h->response, response);
  ret += cmp_dauth_param ("username", &h->username, username);
  ret += cmp_dauth_param ("username_ext", &h->username_ext,
                          username_ext);
  ret += cmp_dauth_param ("realm", &h->realm, realm);
  ret += cmp_dauth_param ("uri", &h->uri, uri);
  ret += cmp_dauth_param ("qop", &h->qop_raw, qop_raw);
  if (h->qop != qop)
  {
    ret += 1;
    fprintf (stderr, "Parameter 'qop' detected incorrectly:\n");
    fprintf (stderr, "\tRESULT  :\t%u\n",
             (unsigned) h->qop);
    fprintf (stderr, "\tEXPECTED:\t%u\n",
             (unsigned) qop);
  }
  ret += cmp_dauth_param ("cnonce", &h->cnonce, cnonce);
  ret += cmp_dauth_param ("nc", &h->nc, nc);
  if (h->userhash != ! (! userhash))
  {
    ret += 1;
    fprintf (stderr, "Parameter 'userhash' parsed incorrectly:\n");
    fprintf (stderr, "\tRESULT  :\t%s\n",
             h->userhash ? "true" : "false");
    fprintf (stderr, "\tEXPECTED:\t%s\n",
             userhash ? "true" : "false");
  }
  if (0 != ret)
  {
    fprintf (stderr,
             "Input Header: '%.*s'\n", (int) hdr_len, hdr);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }
  clean_AuthHeaders ();

  return ret;
}


#define expect_digest(h,no,a,rs,un,ux,rm,ur,qr,qe,c,nc,uh) \
    expect_digest_n(h,MHD_STATICSTR_LEN_(h),\
                    no,a,rs,un,ux,rm,ur,qr,qe,c,nc,uh,__LINE__)

static unsigned int
check_digest (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_digest ("Digest", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest nc=1", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, NULL, \
                      MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest nc=\"1\"", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest nc=\"1\"   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest ,nc=\"1\"   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest nc=\"1\",   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest nc=\"1\" ,   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest nc=1,   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest nc=1 ,   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest ,,,nc=1,   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest ,,,nc=1 ,   ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1", 0);
  r += expect_digest ("Digest ,,,nc=\"1 \",   ", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1 ", 0);
  r += expect_digest ("Digest nc=\"1 \"", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1 ", 0);
  r += expect_digest ("Digest nc=\"1 \" ,", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1 ", 0);
  r += expect_digest ("Digest nc=\"1 \", ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1 ", 0);
  r += expect_digest ("Digest nc=\"1;\", ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1;", 0);
  r += expect_digest ("Digest nc=\"1\\;\", ", NULL, MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, "1\\;", 0);

  r += expect_digest ("Digest userhash=false", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest userhash=\"false\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest userhash=foo", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest userhash=true", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 1);
  r += expect_digest ("Digest userhash=\"true\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 1);
  r += expect_digest ("Digest userhash=\"\\t\\r\\u\\e\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 1);
  r += expect_digest ("Digest userhash=TRUE", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 1);
  r += expect_digest ("Digest userhash=True", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 1);
  r += expect_digest ("Digest userhash = true", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL,  NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 1);
  r += expect_digest ("Digest userhash=True2", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest userhash=\" true\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL,  NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);

  r += expect_digest ("Digest algorithm=MD5", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=md5", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=Md5", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=mD5", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=\"MD5\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=\"\\M\\D\\5\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=\"\\m\\d\\5\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=SHA-256", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=sha-256", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=Sha-256", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=\"SHA-256\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=\"SHA\\-25\\6\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=\"shA-256\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=MD5-sess", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5_SESSION, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=MD5-SESS", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5_SESSION, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=md5-Sess", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5_SESSION, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=SHA-256-seSS", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA256_SESSION, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=SHA-512-256", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA512_256, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=SHA-512-256-sess", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_SHA512_256_SESSION, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=MD5-2", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_INVALID, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=MD5-sess2", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_INVALID, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=SHA-256-512", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_INVALID, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);
  r += expect_digest ("Digest algorithm=", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_INVALID, \
                      NULL, NULL, NULL, NULL, NULL, \
                      NULL, MHD_DIGEST_AUTH_QOP_NONE, NULL, NULL, 0);

  r += expect_digest ("Digest qop=auth", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, NULL, NULL, 0);
  r += expect_digest ("Digest qop=\"auth\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, NULL, NULL, 0);
  r += expect_digest ("Digest qop=Auth", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "Auth", MHD_DIGEST_AUTH_QOP_AUTH, NULL, NULL, 0);
  r += expect_digest ("Digest qop=AUTH", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "AUTH", MHD_DIGEST_AUTH_QOP_AUTH, NULL, NULL, 0);
  r += expect_digest ("Digest qop=\"\\A\\ut\\H\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "\\A\\ut\\H", MHD_DIGEST_AUTH_QOP_AUTH, NULL, NULL, 0);
  r += expect_digest ("Digest qop=\"auth \"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auth ", MHD_DIGEST_AUTH_QOP_INVALID, NULL, NULL, 0);
  r += expect_digest ("Digest qop=auth-int", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auth-int", MHD_DIGEST_AUTH_QOP_AUTH_INT, NULL, NULL, 0);
  r += expect_digest ("Digest qop=\"auth-int\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auth-int", MHD_DIGEST_AUTH_QOP_AUTH_INT, NULL, NULL, 0);
  r += expect_digest ("Digest qop=\"auTh-iNt\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auTh-iNt", MHD_DIGEST_AUTH_QOP_AUTH_INT, NULL, NULL, 0);
  r += expect_digest ("Digest qop=\"auTh-iNt2\"", NULL, \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      NULL, NULL, NULL, NULL, NULL, \
                      "auTh-iNt2", MHD_DIGEST_AUTH_QOP_INVALID, NULL, NULL, 0);

  r += expect_digest ("Digest username=\"test@example.com\", " \
                      "realm=\"users@example.com\", " \
                      "nonce=\"32141232413abcde\", " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\"", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_MD5, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      NULL, "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, \
                      "0a4f113b", "00000001", 0);
  r += expect_digest ("Digest username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\"", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest username=test@example.com, " \
                      "realm=users@example.com, algorithm=\"SHA-256-sess\", " \
                      "nonce=32141232413abcde, " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=/example, qop=\"auth\", nc=\"00000001\", " \
                      "cnonce=0a4f113b, " \
                      "response=6629fae49393a05397450978507c4ef1, " \
                      "opaque=sadfljk32sdaf", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256_SESSION, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest username = \"test@example.com\", " \
                      "realm\t=\t\"users@example.com\", algorithm\t= SHA-256, " \
                      "nonce\t= \"32141232413abcde\", " \
                      "username*\t=\tUTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri = \"/example\", qop = auth, nc\t=\t00000001, " \
                      "cnonce\t\t\t=   \"0a4f113b\", " \
                      "response  =\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\t\t\"sadfljk32sdaf\"", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest username=\"test@example.com\"," \
                      "realm=\"users@example.com\",algorithm=SHA-512-256," \
                      "nonce=\"32141232413abcde\"," \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates," \
                      "uri=\"/example\",qop=auth,nc=00000001," \
                      "cnonce=\"0a4f113b\"," \
                      "response=\"6629fae49393a05397450978507c4ef1\"," \
                      "opaque=\"sadfljk32sdaf\"", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA512_256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest username=\"test@example.com\"," \
                      "realm=\"users@example.com\",algorithm=SHA-256," \
                      "nonce=\"32141232413abcde\",asdf=asdffdsaf," \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates," \
                      "uri=\"/example\",qop=auth,nc=00000001,cnonce=\"0a4f113b\"," \
                      "response=\"6629fae49393a05397450978507c4ef1\"," \
                      "opaque=\"sadfljk32sdaf\"", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=zyx, username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\"", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=zyx,,,,,,,username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\"", "32141232413abcde",
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=zyx,,,,,,,username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, "
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\",,,,,", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=zyx,,,,,,,username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\",foo=bar", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=\"zyx\", username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, "
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\",foo=bar", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=\"zyx, abc\", " \
                      "username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, "
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\",foo=bar", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=\"zyx, abc=cde\", " \
                      "username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\",foo=bar", "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=\"zyx, abc=cde\", " \
                      "username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, " \
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\", foo=\"bar1, bar2\"", \
                      "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=\"zyx, \\\\\"abc=cde\\\\\"\", " \
                      "username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\", foo=\"bar1, bar2\"", \
                      "32141232413abcde",
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);
  r += expect_digest ("Digest abc=\"zyx, \\\\\"abc=cde\\\\\"\", " \
                      "username=\"test@example.com\", " \
                      "realm=\"users@example.com\", algorithm=SHA-256, " \
                      "nonce=\"32141232413abcde\", " \
                      "username*=UTF-8''%c2%a3%20and%20%e2%82%ac%20rates, " \
                      "uri=\"/example\", qop=auth, nc=00000001, "
                      "cnonce=\"0a4f113b\", " \
                      "response=\"6629fae49393a05397450978507c4ef1\", " \
                      "opaque=\"sadfljk32sdaf\", foo=\",nc=02\"",
                      "32141232413abcde", \
                      MHD_DIGEST_AUTH_ALGO3_SHA256, \
                      "6629fae49393a05397450978507c4ef1", "test@example.com", \
                      "UTF-8''%c2%a3%20and%20%e2%82%ac%20rates", \
                      "users@example.com", "/example", \
                      "auth", MHD_DIGEST_AUTH_QOP_AUTH, "0a4f113b", \
                      "00000001", 0);

  return r;
}


#endif /* DAUTH_SUPPORT */

#define TEST_AUTH_STR "dXNlcjpwYXNz"

static unsigned int
check_two_auths (void)
{
  unsigned int ret;
  static struct MHD_HTTP_Req_Header h1;
  static struct MHD_HTTP_Req_Header h2;
  static struct MHD_HTTP_Req_Header h3;
#ifdef BAUTH_SUPPORT
  const struct MHD_RqBAuth *bauth;
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  const struct MHD_RqDAuth *dauth;
#endif /* DAUTH_SUPPORT */

  if ((NULL != conn.rq.headers_received) ||
      (NULL != conn.rq.headers_received_tail))
    externalErrorExitDesc ("Connection's test headers are not empty already");

  /* Init and use both Basic and Digest Auth headers */
  memset (&h1, 0, sizeof(h1));
  memset (&h2, 0, sizeof(h2));
  memset (&h3, 0, sizeof(h3));

  h1.kind = MHD_HEADER_KIND;
  h1.header = MHD_HTTP_HEADER_HOST; /* Just some random header */
  h1.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_HOST);
  h1.value = "localhost";
  h1.value_size = strlen (h1.value);

  h2.kind = MHD_HEADER_KIND;
  h2.header = MHD_HTTP_HEADER_AUTHORIZATION;
  h2.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_AUTHORIZATION);
  h2.value = "Basic " TEST_AUTH_STR;
  h2.value_size = strlen (h2.value);

  h3.kind = MHD_HEADER_KIND;
  h3.header = MHD_HTTP_HEADER_AUTHORIZATION;
  h3.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_AUTHORIZATION);
  h3.value = "Digest cnonce=" TEST_AUTH_STR;
  h3.value_size = strlen (h3.value);

  conn.rq.headers_received = &h1;
  h1.next = &h2;
  h2.prev = &h1;
  h2.next = &h3;
  h3.prev = &h2;
  conn.rq.headers_received_tail = &h3;

  conn.state = MHD_CONNECTION_FULL_REQ_RECEIVED; /* Should be a typical value */

  ret = 0;
#ifdef BAUTH_SUPPORT
  bauth = get_BAuthRqParams ();
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  dauth = get_DAuthRqParams ();
#endif /* DAUTH_SUPPORT */
#ifdef BAUTH_SUPPORT
  if (NULL == bauth)
  {
    fprintf (stderr, "No Basic Authorization header detected. Line: %u\n",
             (unsigned int) __LINE__);
    ret++;
  }
  else if ((MHD_STATICSTR_LEN_ (TEST_AUTH_STR) != bauth->token68.len) ||
           (0 != memcmp (bauth->token68.str, TEST_AUTH_STR,
                         bauth->token68.len)))
  {
    fprintf (stderr, "Basic Authorization token does not match. Line: %u\n",
             (unsigned int) __LINE__);
    ret++;
  }
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  if (NULL == dauth)
  {
    fprintf (stderr, "No Digest Authorization header detected. Line: %u\n",
             (unsigned int) __LINE__);
    ret++;
  }
  else if ((MHD_STATICSTR_LEN_ (TEST_AUTH_STR) != dauth->cnonce.value.len) ||
           (0 != memcmp (dauth->cnonce.value.str, TEST_AUTH_STR,
                         dauth->cnonce.value.len)))
  {
    fprintf (stderr, "Digest Authorization 'cnonce' does not match. Line: %u\n",
             (unsigned int) __LINE__);
    ret++;
  }
#endif /* DAUTH_SUPPORT */

  /* Cleanup */
  conn.rq.headers_received = NULL;
  conn.rq.headers_received_tail = NULL;
  conn.state = MHD_CONNECTION_INIT;

  return ret;
}


int
main (int argc, char *argv[])
{
  unsigned int errcount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
  test_global_init ();

  errcount += check_type ();
#ifdef BAUTH_SUPPORT
  errcount += check_basic ();
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
  errcount += check_digest ();
#endif /* DAUTH_SUPPORT */
  errcount += check_two_auths ();
  if (0 == errcount)
    printf ("All tests were passed without errors.\n");
  return errcount == 0 ? 0 : 1;
}
