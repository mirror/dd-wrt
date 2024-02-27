/*
     This file is part of GNU libmicrohttpd
     Copyright (C) 2007 Christian Grothoff
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

     GNU libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
     option) any later version.

     GNU libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @file test_parse_cookies.c
 * @brief  Testcase for HTTP cookie parsing
 * @author Karlson2k (Evgeny Grin)
 * @author Christian Grothoff
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
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


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 500000

#define EXPECTED_URI_BASE_PATH  "/"

#define URL_SCHEME "http:/" "/"

#define URL_HOST "127.0.0.1"

#define URL_SCHEME_HOST URL_SCHEME URL_HOST

#define PAGE \
  "<html><head><title>libmicrohttpd test page</title></head>" \
  "<body>Success!</body></html>"

#define PAGE_ERROR \
  "<html><body>Cookies parsing error</body></html>"


#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */


struct strct_str_len
{
  const char *str;
  const size_t len;
};

#define STR_LEN_(str)   {str, MHD_STATICSTR_LEN_ (str)}
#define STR_NULL_       {NULL, 0}

struct strct_cookie
{
  struct strct_str_len name;
  struct strct_str_len value;
};

#define COOKIE_(name,value)     {STR_LEN_ (name), STR_LEN_ (value)}
#define COOKIE_NULL             {STR_NULL_, STR_NULL_}

struct strct_test_data
{
  unsigned int line_num;
  const char *header_str;
  unsigned int num_cookies_strict_p2;
  unsigned int num_cookies_strict_p1;
  unsigned int num_cookies_strict_zero;
  unsigned int num_cookies_strict_n2;
  unsigned int num_cookies_strict_n3;
  struct strct_cookie cookies[5];
};

static const struct strct_test_data test_data[] = {
  {
    __LINE__,
    "name1=value1",
    1,
    1,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1;",
    0,
    1,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1; ",
    0,
    1,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "; name1=value1",
    0,
    0,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    ";name1=value1",
    0,
    0,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1 ",
    1,
    1,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1 ;",
    0,
    0,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1 ; ",
    0,
    0,
    1,
    1,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name2=\"value 2\"",
    0,
    0,
    0,
    1,
    1,
    {
      COOKIE_ ("name2", "value 2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1;\tname2=value2",
    0,
    1,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1; name1=value1",
    2,
    2,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name1", "value1"), /* The second value is not checked actually */
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1; name2=value2",
    2,
    2,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1; name2=value2    ",
    2,
    2,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1;  name2=value2",
    0,
    1,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1;name2=value2",
    0,
    1,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1;\tname2=value2",
    0,
    1,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1 ; name2=value2",
    0,
    0,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "     name1=value1; name2=value2",
    2,
    2,
    2,
    2,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=var1; name2=var2; name3=; " \
    "name4=\"var4 with spaces\"; " \
    "name5=var_with_=_char",
    0,
    3,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name1=var1;name2=var2;name3=;" \
    "name4=\"var4 with spaces\";" \
    "name5=var_with_=_char",
    0,
    1,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name1=var1;  name2=var2;  name3=;  " \
    "name4=\"var4 with spaces\";  " \
    "name5=var_with_=_char\t \t",
    0,
    1,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name1=var1;;name2=var2;;name3=;;" \
    "name4=\"var4 with spaces\";;" \
    "name5=var_with_=_char;\t \t",
    0,
    1,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name3=; name1=var1; name2=var2; " \
    "name5=var_with_=_char;" \
    "name4=\"var4 with spaces\"",
    0,
    4,
    4,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name4", "var4 with spaces")
    }
  },
  {
    __LINE__,
    "name2=var2; name1=var1; " \
    "name5=var_with_=_char; name3=; " \
    "name4=\"var4 with spaces\";",
    0,
    4,
    4,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name4", "var4 with spaces")
    }
  },
  {
    __LINE__,
    "name2=var2; name1=var1; " \
    "name5=var_with_=_char; " \
    "name4=\"var4 with spaces\"; name3=",
    0,
    3,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces")
    }
  },
  {
    __LINE__,
    "name2=var2; name1=var1; " \
    "name4=\"var4 with spaces\"; " \
    "name5=var_with_=_char; name3=;",
    0,
    2,
    2,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    ";;;;;;;;name1=var1; name2=var2; name3=; " \
    "name4=\"var4 with spaces\"; " \
    "name5=var_with_=_char",
    0,
    0,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name1=var1; name2=var2; name3=; " \
    "name4=\"var4 with spaces\"; ; ; ; ; " \
    "name5=var_with_=_char",
    0,
    3,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name1=var1; name2=var2; name3=; " \
    "name4=\"var4 with spaces\"; " \
    "name5=var_with_=_char;;;;;;;;",
    0,
    3,
    3,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces"),
      COOKIE_ ("name5", "var_with_=_char")
    }
  },
  {
    __LINE__,
    "name1=var1; name2=var2; " \
    "name4=\"var4 with spaces\";" \
    "name5=var_with_=_char; ; ; ; ; name3=",
    0,
    2,
    2,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces")
    }
  },
  {
    __LINE__,
    "name5=var_with_=_char ;" \
    "name1=var1; name2=var2; name3=; " \
    "name4=\"var4 with spaces\" ",
    0,
    0,
    4,
    5,
    5,
    {
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name4", "var4 with spaces")
    }
  },
  {
    __LINE__,
    "name5=var_with_=_char; name4=\"var4 with spaces\";" \
    "name1=var1; name2=var2; name3=",
    0,
    1,
    1,
    5,
    5,
    {
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4 with spaces")
    }
  },
  {
    __LINE__,
    "name5=var_with_=_char; name4=\"var4_without_spaces\"; " \
    "name1=var1; name2=var2; name3=",
    5,
    5,
    5,
    5,
    5,
    {
      COOKIE_ ("name5", "var_with_=_char"),
      COOKIE_ ("name1", "var1"),
      COOKIE_ ("name2", "var2"),
      COOKIE_ ("name3", ""),
      COOKIE_ ("name4", "var4_without_spaces")
    }
  },
  {
    __LINE__,
    "name1 = value1",
    0,
    0,
    0,
    0,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1\t=\tvalue1",
    0,
    0,
    0,
    0,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1\t = \tvalue1",
    0,
    0,
    0,
    0,
    1,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1 = value1; name2 =\tvalue2",
    0,
    0,
    0,
    0,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1=value1; name2 =\tvalue2",
    0,
    1,
    1,
    1,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "name1 = value1; name2=value2",
    0,
    0,
    0,
    0,
    2,
    {
      COOKIE_ ("name1", "value1"),
      COOKIE_ ("name2", "value2"),
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "      ",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "\t",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "var=,",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "var=\"\\ \"",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "var=value  space",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "var=value\ttab",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "=",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "====",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    ";=",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "var",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "=;",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    "= ;",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  },
  {
    __LINE__,
    ";= ;",
    0,
    0,
    0,
    0,
    0,
    {
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL,
      COOKIE_NULL
    }
  }
};

/* Global parameters */
static int verbose;
static int oneone;                  /**< If false use HTTP/1.0 for requests*/
static int use_discp_n3;
static int use_discp_n2;
static int use_discp_zero;
static int use_discp_p1;
static int use_discp_p2;
static int discp_level;

static void
test_global_init (void)
{
  libcurl_errbuf[0] = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    externalErrorExit ();
}


static void
test_global_cleanup (void)
{
  curl_global_cleanup ();
}


struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};


static size_t
copyBuffer (void *ptr,
            size_t size,
            size_t nmemb,
            void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    return 0;                   /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


struct ahc_cls_type
{
  const char *rq_method;
  const char *rq_url;
  const struct strct_test_data *check;
};


static enum MHD_Result
ahcCheck (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **req_cls)
{
  static int marker;
  struct MHD_Response *response;
  enum MHD_Result ret;
  struct ahc_cls_type *const param = (struct ahc_cls_type *) cls;
  unsigned int expected_num_cookies;
  unsigned int i;
  int cookie_failed;

  if (NULL == param)
    mhdErrorExitDesc ("cls parameter is NULL");
  if (use_discp_p2)
    expected_num_cookies = param->check->num_cookies_strict_p2;
  else if (use_discp_p1)
    expected_num_cookies = param->check->num_cookies_strict_p1;
  else if (use_discp_zero)
    expected_num_cookies = param->check->num_cookies_strict_zero;
  else if (use_discp_n2)
    expected_num_cookies = param->check->num_cookies_strict_n2;
  else if (use_discp_n3)
    expected_num_cookies = param->check->num_cookies_strict_n3;
  else
    externalErrorExit ();


  if (oneone)
  {
    if (0 != strcmp (version, MHD_HTTP_VERSION_1_1))
      mhdErrorExitDesc ("Unexpected HTTP version");
  }
  else
  {
    if (0 != strcmp (version, MHD_HTTP_VERSION_1_0))
      mhdErrorExitDesc ("Unexpected HTTP version");
  }

  if (0 != strcmp (url, param->rq_url))
    mhdErrorExitDesc ("Unexpected URI");

  if (NULL != upload_data)
    mhdErrorExitDesc ("'upload_data' is not NULL");

  if (NULL == upload_data_size)
    mhdErrorExitDesc ("'upload_data_size' pointer is NULL");

  if (0 != *upload_data_size)
    mhdErrorExitDesc ("'*upload_data_size' value is not zero");

  if (0 != strcmp (param->rq_method, method))
    mhdErrorExitDesc ("Unexpected request method");

  cookie_failed = 0;
  for (i = 0; i < expected_num_cookies; ++i)
  {
    const char *cookie_val;
    size_t cookie_val_len;
    const struct strct_cookie *const cookie_data = param->check->cookies + i;
    if (NULL == cookie_data->name.str)
      externalErrorExitDesc ("Broken test data");
    if (NULL == cookie_data->value.str)
      externalErrorExitDesc ("Broken test data");

    cookie_val =
      MHD_lookup_connection_value (connection,
                                   MHD_COOKIE_KIND,
                                   cookie_data->name.str);
    if (cookie_val == NULL)
    {
      fprintf (stderr, "'%s' cookie not found.\n",
               cookie_data->name.str);
      cookie_failed = 1;
    }
    else if (0 != strcmp (cookie_val,
                          cookie_data->value.str))
    {
      fprintf (stderr, "'%s' cookie decoded incorrectly.\n"
               "Expected: %s\nGot: %s\n",
               cookie_data->name.str,
               cookie_data->value.str,
               cookie_val);
      cookie_failed = 1;
    }
    else if (MHD_YES !=
             MHD_lookup_connection_value_n (connection,
                                            MHD_COOKIE_KIND,
                                            cookie_data->name.str,
                                            cookie_data->name.len,
                                            &cookie_val, &cookie_val_len))
    {
      fprintf (stderr, "'%s' (length %lu) cookie not found.\n",
               cookie_data->name.str,
               (unsigned long) cookie_data->name.len);
      cookie_failed = 1;
    }
    else
    {
      if (cookie_data->value.len != cookie_val_len)
      {
        fprintf (stderr, "'%s' (length %lu) cookie has wrong value length.\n"
                 "Expected: %lu\nGot: %lu\n",
                 cookie_data->name.str,
                 (unsigned long) cookie_data->name.len,
                 (unsigned long) cookie_data->value.len,
                 (unsigned long) cookie_val_len);
        cookie_failed = 1;
      }
      else if (0 != memcmp (cookie_val, cookie_data->value.str, cookie_val_len))
      {
        fprintf (stderr, "'%s' (length %lu) cookie has wrong value.\n"
                 "Expected: %.*s\nGot: %.*s\n",
                 cookie_data->name.str,
                 (unsigned long) cookie_data->name.len,
                 (int) cookie_data->value.len, cookie_data->value.str,
                 (int) cookie_val_len, cookie_val);
        cookie_failed = 1;
      }
    }
  }
  if (((int) expected_num_cookies) !=
      MHD_get_connection_values_n (connection, MHD_COOKIE_KIND, NULL, NULL))
  {
    fprintf (stderr, "Wrong total number of cookies.\n"
             "Expected: %u\nGot: %d\n",
             expected_num_cookies,
             MHD_get_connection_values_n (connection, MHD_COOKIE_KIND, NULL,
                                          NULL));
    cookie_failed = 1;
  }
  if (cookie_failed)
  {
    response =
      MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE_ERROR),
                                              PAGE_ERROR);
    ret = MHD_queue_response (connection,
                              MHD_HTTP_BAD_REQUEST,
                              response);
    MHD_destroy_response (response);

    return ret;
  }

  if (&marker != *req_cls)
  {
    *req_cls = &marker;
    return MHD_YES;
  }
  *req_cls = NULL;

  response =
    MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE),
                                            PAGE);
  if (NULL == response)
    mhdErrorExitDesc ("Failed to create response");

  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (MHD_YES != ret)
    mhdErrorExitDesc ("Failed to queue response");

  return ret;
}


static int
libcurl_debug_cb (CURL *handle,
                  curl_infotype type,
                  char *data,
                  size_t size,
                  void *userptr)
{
  static const char excess_mark[] = "Excess found";
  static const size_t excess_mark_len = MHD_STATICSTR_LEN_ (excess_mark);

  (void) handle;
  (void) userptr;

#ifdef _DEBUG
  switch (type)
  {
  case CURLINFO_TEXT:
    fprintf (stderr, "* %.*s", (int) size, data);
    break;
  case CURLINFO_HEADER_IN:
    fprintf (stderr, "< %.*s", (int) size, data);
    break;
  case CURLINFO_HEADER_OUT:
    fprintf (stderr, "> %.*s", (int) size, data);
    break;
  case CURLINFO_DATA_IN:
#if 0
    fprintf (stderr, "<| %.*s\n", (int) size, data);
#endif
    break;
  case CURLINFO_DATA_OUT:
  case CURLINFO_SSL_DATA_IN:
  case CURLINFO_SSL_DATA_OUT:
  case CURLINFO_END:
  default:
    break;
  }
#endif /* _DEBUG */
  if (CURLINFO_TEXT == type)
  {
    if ((size >= excess_mark_len) &&
        (0 == memcmp (data, excess_mark, excess_mark_len)))
      mhdErrorExitDesc ("Extra data has been detected in MHD reply");
  }
  return 0;
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
                                     (oneone) ?
                                     CURL_HTTP_VERSION_1_1 :
                                     CURL_HTTP_VERSION_1_0)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER,
                                     libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 0L)) ||
#ifdef _DEBUG
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_VERBOSE, 1L)) ||
#endif /* _DEBUG */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEBUGFUNCTION,
                                     &libcurl_debug_cb)) ||
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

  if (CURLE_OK !=
      curl_easy_setopt (c, CURLOPT_URL,
                        URL_SCHEME_HOST EXPECTED_URI_BASE_PATH))
    libcurlErrorExitDesc ("Cannot set request URL");

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
    if (0 == MHD_get_timeout64s (d))
      tv.tv_usec = 0;
    else
    {
      long curl_to = -1;
      curl_multi_timeout (multi, &curl_to);
      if (0 == curl_to)
        tv.tv_usec = 0;
    }
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
      Sleep ((unsigned long) tv.tv_usec / 1000);
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
check_result (CURLcode curl_code, CURL *c, long expected_code,
              struct CBC *pcbc)
{
  long code;

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

  if (CURLE_OK != curl_easy_getinfo (c, CURLINFO_RESPONSE_CODE, &code))
    libcurlErrorExit ();

  if (expected_code != code)
  {
    fprintf (stderr, "### The response has wrong HTTP code: %ld\t"
             "Expected: %ld.\n",
             code, expected_code);
    return 0;
  }
  else if (verbose)
    printf ("### The response has expected HTTP code: %ld\n", expected_code);

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
  fflush (stderr);
  fflush (stdout);

  return 1;
}


static unsigned int
testExternalPolling (void)
{
  struct MHD_Daemon *d;
  uint16_t port;
  struct CBC cbc;
  struct ahc_cls_type ahc_param;
  char buf[2048];
  CURL *c;
  CURLM *multi_reuse;
  size_t i;
  int failed = 0;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 1340 + oneone ? 0 : 6 + (uint16_t) (1 + discp_level);

  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY,
                        port, NULL, NULL,
                        &ahcCheck, &ahc_param,
                        MHD_OPTION_CLIENT_DISCIPLINE_LVL,
                        (int) (discp_level),
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
    port = dinfo->port;
  }

  ahc_param.rq_method = MHD_HTTP_METHOD_GET;
  ahc_param.rq_url = EXPECTED_URI_BASE_PATH;
  cbc.buf = buf;
  cbc.size = sizeof (buf);
  memset (cbc.buf, 0, cbc.size);
  c = setupCURL (&cbc, port);
  multi_reuse = NULL;
  for (i = 0; i < sizeof(test_data) / sizeof(test_data[0]); ++i)
  {
    cbc.pos = 0;
    ahc_param.check = test_data + i;
    if (CURLE_OK !=
        curl_easy_setopt (c, CURLOPT_COOKIE,
                          ahc_param.check->header_str))
      libcurlErrorExitDesc ("Cannot set request cookies");

    if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                      MHD_HTTP_OK, &cbc))
    {
      if (verbose)
        printf ("### Got expected response for the check at line %u.\n",
                test_data[i].line_num);
      fflush (stdout);
    }
    else
    {
      fprintf (stderr, "### FAILED request for the check at line %u.\n",
               test_data[i].line_num);
      fflush (stderr);
      failed = 1;
    }
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
  unsigned int errorCount = 0;

  /* Test type and test parameters */
  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));
  oneone = ! has_in_name (argv[0], "10");
  use_discp_n3 = has_in_name (argv[0], "_discp_n3");
  use_discp_n2 = has_in_name (argv[0], "_discp_n2");
  use_discp_zero = has_in_name (argv[0], "_discp_zero");
  use_discp_p1 = has_in_name (argv[0], "_discp_p1");
  use_discp_p2 = has_in_name (argv[0], "_discp_p2");
  if (1 != ((use_discp_n3 ? 1 : 0) + (use_discp_n2 ? 1 : 0)
            + (use_discp_zero ? 1 : 0)
            + (use_discp_p1 ? 1 : 0) + (use_discp_p2 ? 1 : 0)))
    return 99;

  if (use_discp_n3)
    discp_level = -3;
  else if (use_discp_n2)
    discp_level = -2;
  else if (use_discp_zero)
    discp_level = 0;
  else if (use_discp_p1)
    discp_level = 1;
  else if (use_discp_p2)
    discp_level = 2;

  test_global_init ();

  errorCount += testExternalPolling ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);

  test_global_cleanup ();

  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
