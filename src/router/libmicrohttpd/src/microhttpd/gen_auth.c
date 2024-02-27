/*
  This file is part of libmicrohttpd
  Copyright (C) 2022-2023 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.
  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file microhttpd/gen_auth.c
 * @brief  HTTP authorisation general functions
 * @author Karlson2k (Evgeny Grin)
 */

#include "gen_auth.h"
#include "internal.h"
#include "connection.h"
#include "mhd_str.h"
#include "mhd_assert.h"

#ifdef BAUTH_SUPPORT
#include "basicauth.h"
#endif /* BAUTH_SUPPORT */
#ifdef DAUTH_SUPPORT
#include "digestauth.h"
#endif /* DAUTH_SUPPORT */

#if ! defined(BAUTH_SUPPORT) && ! defined(DAUTH_SUPPORT)
#error This file requires Basic or Digest authentication support
#endif

/**
 * Type of authorisation
 */
enum MHD_AuthType
{
  MHD_AUTHTYPE_NONE = 0,/**< No authorisation, unused */
  MHD_AUTHTYPE_BASIC,   /**< Basic Authorisation, RFC 7617  */
  MHD_AUTHTYPE_DIGEST,  /**< Digest Authorisation, RFC 7616 */
  MHD_AUTHTYPE_UNKNOWN, /**< Unknown/Unsupported authorisation type, unused */
  MHD_AUTHTYPE_INVALID  /**< Wrong/broken authorisation header, unused */
};

/**
 * Find required "Authorization" request header
 * @param c the connection with request
 * @param type the type of the authorisation: basic or digest
 * @param[out] auth_value will be set to the remaining of header value after
 *                        authorisation token (after "Basic " or "Digest ")
 * @return true if requested header is found,
 *         false otherwise
 */
static bool
find_auth_rq_header_ (const struct MHD_Connection *c, enum MHD_AuthType type,
                      struct _MHD_str_w_len *auth_value)
{
  const struct MHD_HTTP_Req_Header *h;
  const char *token;
  size_t token_len;

  mhd_assert (MHD_CONNECTION_HEADERS_PROCESSED <= c->state);
  if (MHD_CONNECTION_HEADERS_PROCESSED > c->state)
    return false;

#ifdef DAUTH_SUPPORT
  if (MHD_AUTHTYPE_DIGEST == type)
  {
    token = _MHD_AUTH_DIGEST_BASE;
    token_len = MHD_STATICSTR_LEN_ (_MHD_AUTH_DIGEST_BASE);
  }
  else /* combined with the next line */
#endif /* DAUTH_SUPPORT */
#ifdef BAUTH_SUPPORT
  if (MHD_AUTHTYPE_BASIC == type)
  {
    token = _MHD_AUTH_BASIC_BASE;
    token_len = MHD_STATICSTR_LEN_ (_MHD_AUTH_BASIC_BASE);
  }
  else /* combined with the next line */
#endif /* BAUTH_SUPPORT */
  {
    mhd_assert (0);
    return false;
  }

  for (h = c->rq.headers_received; NULL != h; h = h->next)
  {
    if (MHD_HEADER_KIND != h->kind)
      continue;
    if (MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_AUTHORIZATION) != h->header_size)
      continue;
    if (token_len > h->value_size)
      continue;
    if (! MHD_str_equal_caseless_bin_n_ (MHD_HTTP_HEADER_AUTHORIZATION,
                                         h->header,
                                         MHD_STATICSTR_LEN_ ( \
                                           MHD_HTTP_HEADER_AUTHORIZATION)))
      continue;
    if (! MHD_str_equal_caseless_bin_n_ (h->value, token, token_len))
      continue;
    /* Match only if token string is full header value or token is
     * followed by space or tab
     * Note: RFC 9110 (and RFC 7234) allows only space character, but
     * tab is supported here as well for additional flexibility and uniformity
     * as tabs are supported as separators between parameters.
     */
    if ((token_len == h->value_size) ||
        (' ' == h->value[token_len]) || ('\t'  == h->value[token_len]))
    {
      if (token_len != h->value_size)
      { /* Skip whitespace */
        auth_value->str = h->value + token_len + 1;
        auth_value->len = h->value_size - (token_len + 1);
      }
      else
      { /* No whitespace to skip */
        auth_value->str = h->value + token_len;
        auth_value->len = h->value_size - token_len;
      }
      return true; /* Found a match */
    }
  }
  return false; /* No matching header has been found */
}


#ifdef BAUTH_SUPPORT


/**
 * Parse request Authorization header parameters for Basic Authentication
 * @param str the header string, everything after "Basic " substring
 * @param str_len the length of @a str in characters
 * @param[out] pbauth the pointer to the structure with Basic Authentication
 *               parameters
 * @return true if parameters has been successfully parsed,
 *         false if format of the @a str is invalid
 */
static bool
parse_bauth_params (const char *str,
                    size_t str_len,
                    struct MHD_RqBAuth *pbauth)
{
  size_t i;

  i = 0;

  /* Skip all whitespaces at start */
  while (i < str_len && (' ' == str[i] || '\t' == str[i]))
    i++;

  if (str_len > i)
  {
    size_t token68_start;
    size_t token68_len;

    /* 'i' points to the first non-whitespace char after scheme token */
    token68_start = i;
    /* Find end of the token. Token cannot contain whitespace. */
    while (i < str_len && ' ' != str[i] && '\t' != str[i])
    {
      if (0 == str[i])
        return false;  /* Binary zero is not allowed */
      if ((',' == str[i]) || (';' == str[i]))
        return false;  /* Only single token68 is allowed */
      i++;
    }
    token68_len = i - token68_start;
    mhd_assert (0 != token68_len);

    /* Skip all whitespaces */
    while (i < str_len && (' ' == str[i] || '\t' == str[i]))
      i++;
    /* Check whether any garbage is present at the end of the string */
    if (str_len != i)
      return false;
    else
    {
      /* No more data in the string, only single token68. */
      pbauth->token68.str = str + token68_start;
      pbauth->token68.len = token68_len;
    }
  }
  return true;
}


/**
 * Return request's Basic Authorisation parameters.
 *
 * Function return result of parsing of the request's "Authorization" header or
 * returns cached parsing result if the header was already parsed for
 * the current request.
 * @param connection the connection to process
 * @return the pointer to structure with Authentication parameters,
 *         NULL if no memory in memory pool, if called too early (before
 *         header has been received) or if no valid Basic Authorisation header
 *         found.
 */
const struct MHD_RqBAuth *
MHD_get_rq_bauth_params_ (struct MHD_Connection *connection)
{
  struct _MHD_str_w_len h_auth_value;
  struct MHD_RqBAuth *bauth;

  mhd_assert (MHD_CONNECTION_HEADERS_PROCESSED <= connection->state);

  if (connection->rq.bauth_tried)
    return connection->rq.bauth;

  if (MHD_CONNECTION_HEADERS_PROCESSED > connection->state)
    return NULL;

  if (! find_auth_rq_header_ (connection, MHD_AUTHTYPE_BASIC, &h_auth_value))
  {
    connection->rq.bauth_tried = true;
    connection->rq.bauth = NULL;
    return NULL;
  }

  bauth =
    (struct MHD_RqBAuth *)
    MHD_connection_alloc_memory_ (connection, sizeof (struct MHD_RqBAuth));

  if (NULL == bauth)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Not enough memory in the connection's pool to allocate " \
                 "for Basic Authorization header parsing.\n"));
#endif /* HAVE_MESSAGES */
    return NULL;
  }

  memset (bauth, 0, sizeof(struct MHD_RqBAuth));
  if (parse_bauth_params (h_auth_value.str, h_auth_value.len, bauth))
    connection->rq.bauth = bauth;
  else
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("The Basic Authorization client's header has "
                 "incorrect format.\n"));
#endif /* HAVE_MESSAGES */
    connection->rq.bauth = NULL;
    /* Memory in the pool remains allocated until next request */
  }
  connection->rq.bauth_tried = true;
  return connection->rq.bauth;
}


#endif /* BAUTH_SUPPORT */

#ifdef DAUTH_SUPPORT


/**
 * Get client's Digest Authorization algorithm type.
 * If no algorithm is specified by client, MD5 is assumed.
 * @param params the Digest Authorization 'algorithm' parameter
 * @return the algorithm type
 */
static enum MHD_DigestAuthAlgo3
get_rq_dauth_algo (const struct MHD_RqDAuthParam *const algo_param)
{
  if (NULL == algo_param->value.str)
    return MHD_DIGEST_AUTH_ALGO3_MD5; /* Assume MD5 by default */

  if (algo_param->quoted)
  {
    if (MHD_str_equal_caseless_quoted_s_bin_n (algo_param->value.str, \
                                               algo_param->value.len, \
                                               _MHD_MD5_TOKEN))
      return MHD_DIGEST_AUTH_ALGO3_MD5;
    if (MHD_str_equal_caseless_quoted_s_bin_n (algo_param->value.str, \
                                               algo_param->value.len, \
                                               _MHD_SHA256_TOKEN))
      return MHD_DIGEST_AUTH_ALGO3_SHA256;
    if (MHD_str_equal_caseless_quoted_s_bin_n (algo_param->value.str, \
                                               algo_param->value.len, \
                                               _MHD_MD5_TOKEN _MHD_SESS_TOKEN))
      return MHD_DIGEST_AUTH_ALGO3_SHA512_256;
    if (MHD_str_equal_caseless_quoted_s_bin_n (algo_param->value.str, \
                                               algo_param->value.len, \
                                               _MHD_SHA512_256_TOKEN \
                                               _MHD_SESS_TOKEN))

      /* Algorithms below are not supported by MHD for authentication */

      return MHD_DIGEST_AUTH_ALGO3_MD5_SESSION;
    if (MHD_str_equal_caseless_quoted_s_bin_n (algo_param->value.str, \
                                               algo_param->value.len, \
                                               _MHD_SHA256_TOKEN \
                                               _MHD_SESS_TOKEN))
      return MHD_DIGEST_AUTH_ALGO3_SHA256_SESSION;
    if (MHD_str_equal_caseless_quoted_s_bin_n (algo_param->value.str, \
                                               algo_param->value.len, \
                                               _MHD_SHA512_256_TOKEN))
      return MHD_DIGEST_AUTH_ALGO3_SHA512_256_SESSION;

    /* No known algorithm has been detected */
    return MHD_DIGEST_AUTH_ALGO3_INVALID;
  }
  /* The algorithm value is not quoted */
  if (MHD_str_equal_caseless_s_bin_n_ (_MHD_MD5_TOKEN, \
                                       algo_param->value.str, \
                                       algo_param->value.len))
    return MHD_DIGEST_AUTH_ALGO3_MD5;
  if (MHD_str_equal_caseless_s_bin_n_ (_MHD_SHA256_TOKEN, \
                                       algo_param->value.str, \
                                       algo_param->value.len))
    return MHD_DIGEST_AUTH_ALGO3_SHA256;
  if (MHD_str_equal_caseless_s_bin_n_ (_MHD_SHA512_256_TOKEN, \
                                       algo_param->value.str, \
                                       algo_param->value.len))
    return MHD_DIGEST_AUTH_ALGO3_SHA512_256;

  /* Algorithms below are not supported by MHD for authentication */

  if (MHD_str_equal_caseless_s_bin_n_ (_MHD_MD5_TOKEN _MHD_SESS_TOKEN, \
                                       algo_param->value.str, \
                                       algo_param->value.len))
    return MHD_DIGEST_AUTH_ALGO3_MD5_SESSION;
  if (MHD_str_equal_caseless_s_bin_n_ (_MHD_SHA256_TOKEN _MHD_SESS_TOKEN, \
                                       algo_param->value.str, \
                                       algo_param->value.len))
    return MHD_DIGEST_AUTH_ALGO3_SHA256_SESSION;
  if (MHD_str_equal_caseless_s_bin_n_ (_MHD_SHA512_256_TOKEN _MHD_SESS_TOKEN, \
                                       algo_param->value.str, \
                                       algo_param->value.len))
    return MHD_DIGEST_AUTH_ALGO3_SHA512_256_SESSION;

  /* No known algorithm has been detected */
  return MHD_DIGEST_AUTH_ALGO3_INVALID;
}


/**
 * Get QOP ('quality of protection') type.
 * @param qop_param the Digest Authorization 'QOP' parameter
 * @return detected QOP ('quality of protection') type.
 */
static enum MHD_DigestAuthQOP
get_rq_dauth_qop (const struct MHD_RqDAuthParam *const qop_param)
{
  if (NULL == qop_param->value.str)
    return MHD_DIGEST_AUTH_QOP_NONE;
  if (qop_param->quoted)
  {
    if (MHD_str_equal_caseless_quoted_s_bin_n (qop_param->value.str, \
                                               qop_param->value.len, \
                                               MHD_TOKEN_AUTH_))
      return MHD_DIGEST_AUTH_QOP_AUTH;
    if (MHD_str_equal_caseless_quoted_s_bin_n (qop_param->value.str, \
                                               qop_param->value.len, \
                                               MHD_TOKEN_AUTH_INT_))
      return MHD_DIGEST_AUTH_QOP_AUTH_INT;
  }
  else
  {
    if (MHD_str_equal_caseless_s_bin_n_ (MHD_TOKEN_AUTH_, \
                                         qop_param->value.str, \
                                         qop_param->value.len))
      return MHD_DIGEST_AUTH_QOP_AUTH;
    if (MHD_str_equal_caseless_s_bin_n_ (MHD_TOKEN_AUTH_INT_, \
                                         qop_param->value.str, \
                                         qop_param->value.len))
      return MHD_DIGEST_AUTH_QOP_AUTH_INT;
  }
  /* No know QOP has been detected */
  return MHD_DIGEST_AUTH_QOP_INVALID;
}


/**
 * Parse request Authorization header parameters for Digest Authentication
 * @param str the header string, everything after "Digest " substring
 * @param str_len the length of @a str in characters
 * @param[out] pdauth the pointer to the structure with Digest Authentication
 *               parameters
 * @return true if parameters has been successfully parsed,
 *         false if format of the @a str is invalid
 */
static bool
parse_dauth_params (const char *str,
                    const size_t str_len,
                    struct MHD_RqDAuth *pdauth)
{
  /* The tokens */
  static const struct _MHD_cstr_w_len nonce_tk = _MHD_S_STR_W_LEN ("nonce");
  static const struct _MHD_cstr_w_len opaque_tk = _MHD_S_STR_W_LEN ("opaque");
  static const struct _MHD_cstr_w_len algorithm_tk =
    _MHD_S_STR_W_LEN ("algorithm");
  static const struct _MHD_cstr_w_len response_tk =
    _MHD_S_STR_W_LEN ("response");
  static const struct _MHD_cstr_w_len username_tk =
    _MHD_S_STR_W_LEN ("username");
  static const struct _MHD_cstr_w_len username_ext_tk =
    _MHD_S_STR_W_LEN ("username*");
  static const struct _MHD_cstr_w_len realm_tk = _MHD_S_STR_W_LEN ("realm");
  static const struct _MHD_cstr_w_len uri_tk = _MHD_S_STR_W_LEN ("uri");
  static const struct _MHD_cstr_w_len qop_tk = _MHD_S_STR_W_LEN ("qop");
  static const struct _MHD_cstr_w_len cnonce_tk = _MHD_S_STR_W_LEN ("cnonce");
  static const struct _MHD_cstr_w_len nc_tk = _MHD_S_STR_W_LEN ("nc");
  static const struct _MHD_cstr_w_len userhash_tk =
    _MHD_S_STR_W_LEN ("userhash");
  /* The locally processed parameters */
  struct MHD_RqDAuthParam userhash;
  struct MHD_RqDAuthParam algorithm;
  /* Indexes */
  size_t i;
  size_t p;
  /* The list of the tokens.
     The order of the elements matches the next array. */
  static const struct _MHD_cstr_w_len *const tk_names[] = {
    &nonce_tk,          /* 0 */
    &opaque_tk,         /* 1 */
    &algorithm_tk,      /* 2 */
    &response_tk,       /* 3 */
    &username_tk,       /* 4 */
    &username_ext_tk,   /* 5 */
    &realm_tk,          /* 6 */
    &uri_tk,            /* 7 */
    &qop_tk,            /* 8 */
    &cnonce_tk,         /* 9 */
    &nc_tk,             /* 10 */
    &userhash_tk        /* 11 */
  };
  /* The list of the parameters.
     The order of the elements matches the previous array. */
  struct MHD_RqDAuthParam *params[sizeof(tk_names) / sizeof(tk_names[0])];

  params[0 ] = &(pdauth->nonce);           /* 0 */
  params[1 ] = &(pdauth->opaque);          /* 1 */
  params[2 ] = &algorithm;                 /* 2 */
  params[3 ] = &(pdauth->response);        /* 3 */
  params[4 ] = &(pdauth->username);        /* 4 */
  params[5 ] = &(pdauth->username_ext);    /* 5 */
  params[6 ] = &(pdauth->realm);           /* 6 */
  params[7 ] = &(pdauth->uri);             /* 7 */
  params[8 ] = &(pdauth->qop_raw);         /* 8 */
  params[9 ] = &(pdauth->cnonce);          /* 9 */
  params[10] = &(pdauth->nc);              /* 10 */
  params[11] = &userhash;                  /* 11 */

  mhd_assert ((sizeof(tk_names) / sizeof(tk_names[0])) == \
              (sizeof(params) / sizeof(params[0])));
  memset (&userhash, 0, sizeof(userhash));
  memset (&algorithm, 0, sizeof(algorithm));
  i = 0;

  /* Skip all whitespaces at start */
  while (i < str_len && (' ' == str[i] || '\t' == str[i]))
    i++;

  while (str_len > i)
  {
    size_t left;
    mhd_assert (' ' != str[i]);
    mhd_assert ('\t' != str[i]);

    left = str_len - i;
    if ('=' == str[i])
      return false; /* The equal sign is not allowed as the first character */
    for (p = 0; p < (sizeof(tk_names) / sizeof(tk_names[0])); ++p)
    {
      const struct _MHD_cstr_w_len *const tk_name = tk_names[p];
      struct MHD_RqDAuthParam *const param = params[p];
      if ( (tk_name->len <= left) &&
           MHD_str_equal_caseless_bin_n_ (str + i, tk_name->str,
                                          tk_name->len) &&
           ((tk_name->len == left) ||
            ('=' == str[i + tk_name->len]) ||
            (' ' == str[i + tk_name->len]) ||
            ('\t' == str[i + tk_name->len]) ||
            (',' == str[i + tk_name->len]) ||
            (';' == str[i + tk_name->len])) )
      {
        size_t value_start;
        size_t value_len;
        bool quoted; /* Only mark as "quoted" if backslash-escape used */

        if (tk_name->len == left)
          return false; /* No equal sign after parameter name, broken data */

        quoted = false;
        i += tk_name->len;
        /* Skip all whitespaces before '=' */
        while (str_len > i && (' ' == str[i] || '\t' == str[i]))
          i++;
        if ((i == str_len) || ('=' != str[i]))
          return false; /* No equal sign, broken data */
        i++;
        /* Skip all whitespaces after '=' */
        while (str_len > i && (' ' == str[i] || '\t' == str[i]))
          i++;
        if ((str_len > i) && ('"' == str[i]))
        { /* Value is in quotation marks */
          i++; /* Advance after the opening quote */
          value_start = i;
          while (str_len > i && '"' != str[i])
          {
            if ('\\' == str[i])
            {
              i++;
              quoted = true; /* Have escaped chars */
            }
            if (0 == str[i])
              return false; /* Binary zero in parameter value */
            i++;
          }
          if (str_len <= i)
            return false; /* No closing quote */
          mhd_assert ('"' == str[i]);
          value_len = i - value_start;
          i++; /* Advance after the closing quote */
        }
        else
        {
          value_start = i;
          while (str_len > i && ',' != str[i] &&
                 ' ' != str[i] && '\t' != str[i] && ';' != str[i])
          {
            if (0 == str[i])
              return false;  /* Binary zero in parameter value */
            i++;
          }
          if (';' == str[i])
            return false;  /* Semicolon in parameter value */
          value_len = i - value_start;
        }
        /* Skip all whitespaces after parameter value */
        while (str_len > i && (' ' == str[i] || '\t' == str[i]))
          i++;
        if ((str_len > i) && (',' != str[i]))
          return false; /* Garbage after parameter value */

        /* Have valid parameter name and value */
        mhd_assert (! quoted || 0 != value_len);
        param->value.str = str + value_start;
        param->value.len = value_len;
        param->quoted = quoted;

        break; /* Found matching parameter name */
      }
    }
    if (p == (sizeof(tk_names) / sizeof(tk_names[0])))
    {
      /* No matching parameter name */
      while (str_len > i && ',' != str[i])
      {
        if ((0 == str[i]) || (';' == str[i]))
          return false; /* Not allowed characters */
        if ('"' == str[i])
        { /* Skip quoted part */
          i++; /* Advance after the opening quote */
          while (str_len > i && '"' != str[i])
          {
            if (0 == str[i])
              return false;  /* Binary zero is not allowed */
            if ('\\' == str[i])
              i++;           /* Skip escaped char */
            i++;
          }
          if (str_len <= i)
            return false; /* No closing quote */
          mhd_assert ('"' == str[i]);
        }
        i++;
      }
    }
    mhd_assert (str_len == i || ',' == str[i]);
    if (str_len > i)
      i++; /* Advance after ',' */
    /* Skip all whitespaces before next parameter name */
    while (i < str_len && (' ' == str[i] || '\t' == str[i]))
      i++;
  }

  /* Postprocess values */

  if (NULL != userhash.value.str)
  {
    if (userhash.quoted)
      pdauth->userhash =
        MHD_str_equal_caseless_quoted_s_bin_n (userhash.value.str, \
                                               userhash.value.len, \
                                               "true");
    else
      pdauth->userhash =
        MHD_str_equal_caseless_s_bin_n_ ("true", userhash.value.str, \
                                         userhash.value.len);

  }
  else
    pdauth->userhash = false;

  pdauth->algo3 = get_rq_dauth_algo (&algorithm);
  pdauth->qop = get_rq_dauth_qop (&pdauth->qop_raw);

  return true;
}


/**
 * Return request's Digest Authorisation parameters.
 *
 * Function return result of parsing of the request's "Authorization" header or
 * returns cached parsing result if the header was already parsed for
 * the current request.
 * @param connection the connection to process
 * @return the pointer to structure with Authentication parameters,
 *         NULL if no memory in memory pool, if called too early (before
 *         header has been received) or if no valid Basic Authorisation header
 *         found.
 */
const struct MHD_RqDAuth *
MHD_get_rq_dauth_params_ (struct MHD_Connection *connection)
{
  struct _MHD_str_w_len h_auth_value;
  struct MHD_RqDAuth *dauth;

  mhd_assert (MHD_CONNECTION_HEADERS_PROCESSED <= connection->state);

  if (connection->rq.dauth_tried)
    return connection->rq.dauth;

  if (MHD_CONNECTION_HEADERS_PROCESSED > connection->state)
    return NULL;

  if (! find_auth_rq_header_ (connection, MHD_AUTHTYPE_DIGEST, &h_auth_value))
  {
    connection->rq.dauth_tried = true;
    connection->rq.dauth = NULL;
    return NULL;
  }

  dauth =
    (struct MHD_RqDAuth *)
    MHD_connection_alloc_memory_ (connection, sizeof (struct MHD_RqDAuth));

  if (NULL == dauth)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Not enough memory in the connection's pool to allocate " \
                 "for Digest Authorization header parsing.\n"));
#endif /* HAVE_MESSAGES */
    return NULL;
  }

  memset (dauth, 0, sizeof(struct MHD_RqDAuth));
  if (parse_dauth_params (h_auth_value.str, h_auth_value.len, dauth))
    connection->rq.dauth = dauth;
  else
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("The Digest Authorization client's header has "
                 "incorrect format.\n"));
#endif /* HAVE_MESSAGES */
    connection->rq.dauth = NULL;
    /* Memory in the pool remains allocated until next request */
  }
  connection->rq.dauth_tried = true;
  return connection->rq.dauth;
}


#endif /* DAUTH_SUPPORT */
