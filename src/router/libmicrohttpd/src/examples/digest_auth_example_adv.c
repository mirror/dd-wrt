/*
     This file is part of libmicrohttpd
     Copyright (C) 2010 Christian Grothoff (and other contributing authors)
     Copyright (C) 2016-2024 Evgeny Grin (Karlson2k)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
 * @file digest_auth_example_adv.c
 * @brief Advanced example for digest auth with libmicrohttpd
 * @author Karlson2k (Evgeny Grin)
 */

#include <microhttpd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#if ! defined(_WIN32) || defined(__CYGWIN__)
#  include <errno.h>
#  include <fcntl.h>
#  include <unistd.h>
#else  /* Native W32 */
#  include <wincrypt.h>
#endif /* Native W32 */

#define SEC_AREA1_URL "/secret_page/"
#define SEC_AREA2_URL "/super_secret_page/"

#define MAIN_PAGE \
  "<html><head><title>Welcome to the site</title></head>" \
  "<body><p><a href=\"" SEC_AREA1_URL "\">Restricted Page</a></p>" \
  "<p><a href=\"" SEC_AREA2_URL "\">Very Restricted Page</a></p></body></html>"

#define OPAQUE_DATA "ServerOpaqueData"

#define REALM "authenticated_users@thishost"

/**
 * Force select "MD5" algorithm instead of MHD default (currently the same) if non-zero.
 */
static int force_md5 = 0;
/**
 * Force select "SHA-256" algorithm instead of MHD default (MD5) if non-zero.
 */
static int force_sha256 = 0;
/**
 * Force select "SHA-512/256" algorithm instead of MHD default (MD5) if non-zero.
 */
static int force_sha512_256 = 0;
/**
 * Disable fallback to (less secure) RFC2069 if non-zero.
 */
static int allow_rfc2069 = 0;

/**
 * The daemon's port
 */
static uint16_t daemon_port = 0;


/* *** "Database" of users and "database" functions *** */

/**
 * User record.
 * This kind of data (or something similar) should be stored in some database
 * or file.
 */
struct UserEntry
{
  /**
   * The username.
   * Static data is used in this example.
   * In real application dynamic buffer or fixed size array could be used.
   */
  const char *username;
#if 0 /* Disabled code */
  /* The cleartext password is not stored in the database.
     The more secure "userdigest" is used instead. */
  /**
   * The password.
   * Static data is used in this example.
   * In real application dynamic buffer or fixed size array could be used.
   */
  const char *password;
#endif /* Disabled code */
  /**
   * The realm for this entry.
   * Static data is used in this example.
   * In real application dynamic buffer or fixed size array could be used.
   */
  const char *realm;

  /**
   * The MD5 hash of the username together with the realm.
   * This hash can be used by the client to send the username in encrypted
   * form.
   * The purpose of userhash is to hide user identity when transmitting
   * requests over insecure link.
   */
  uint8_t userhash_md5[MHD_MD5_DIGEST_SIZE];
  /**
   * The MD5 hash of the username with the password and the realm.
   * It is used to verify that password used by the client matches password
   * required by the server.
   * The purpose of userhash is to avoid keeping the password in cleartext
   * on the server side.
   */
  uint8_t userdigest_md5[MHD_MD5_DIGEST_SIZE];

  /**
   * The SHA-256 hash of the username together with the realm.
   * This hash can be used by the client to send the username in encrypted
   * form.
   * The purpose of userhash is to hide user identity when transmitting
   * requests over insecure link.
   */
  uint8_t userhash_sha256[MHD_SHA256_DIGEST_SIZE];
  /**
   * The SHA-256 hash of the username with the password and the realm.
   * It is used to verify that password used by the client matches password
   * required by the server.
   * The purpose of userhash is to avoid keeping the password in cleartext
   * on the server side.
   */
  uint8_t userdigest_sha256[MHD_SHA256_DIGEST_SIZE];

  /**
   * The SHA-512/256 hash of the username together with the realm.
   * This hash can be used by the client to send the username in encrypted
   * form.
   * The purpose of userhash is to hide user identity when transmitting
   * requests over insecure link.
   */
  uint8_t userhash_sha512_256[MHD_SHA512_256_DIGEST_SIZE];
  /**
   * The SHA-512/256 hash of the username with the password and the realm.
   * It is used to verify that password used by the client matches password
   * required by the server.
   * The purpose of userhash is to avoid keeping the password in cleartext
   * on the server side.
   */
  uint8_t userdigest_sha512_256[MHD_SHA512_256_DIGEST_SIZE];

  /**
   * User has access to "area 1" if non-zero
   */
  int allow_area_1;

  /**
   * User has access to "area 2" if non-zero
   */
  int allow_area_2;
};

/**
 * The array of user entries.
 * In real application it should be loaded from external sources
 * at the application startup.
 */
static struct UserEntry user_ids[2];

/**
 * The number of entries used in @a user_ids.
 */
static size_t user_ids_used = 0;

/**
 * Add new user to the users database/array.
 *
 * This kind of function must be used only when the new user is introduced.
 * It must not be used at the every start of the application. The database
 * of users should be stored somewhere and reloaded when application is
 * started.
 *
 * @param username the username of the new user
 * @param password the password of the new user
 * @param realm the realm (the protection space) for which the new user
 *              is added
 * @param allow_area_1 if non-zero than user has access to the "area 1"
 * @param allow_area_2 if non-zero than user has access to the "area 2"
 * @return non-zero on success,
 *         zero on failure (like no more space in the database).
 */
static int
add_new_user_entry (const char *const username,
                    const char *const password,
                    const char *const realm,
                    int allow_area_1,
                    int allow_area_2)
{
  struct UserEntry *entry;
  enum MHD_Result res;

  if ((sizeof(user_ids) / sizeof(user_ids[0])) <= user_ids_used)
    return 0; /* No more space to add new entry */

  entry = user_ids + user_ids_used;

  entry->username = username;
  entry->realm = realm;

  res = MHD_YES;

  if (MHD_NO != res)
    res = MHD_digest_auth_calc_userhash (MHD_DIGEST_AUTH_ALGO3_MD5,
                                         username,
                                         realm,
                                         entry->userhash_md5,
                                         sizeof(entry->userhash_md5));
  if (MHD_NO != res)
    res = MHD_digest_auth_calc_userdigest (MHD_DIGEST_AUTH_ALGO3_MD5,
                                           username,
                                           realm,
                                           password,
                                           entry->userdigest_md5,
                                           sizeof(entry->userdigest_md5));

  if (MHD_NO != res)
    res = MHD_digest_auth_calc_userhash (MHD_DIGEST_AUTH_ALGO3_SHA256,
                                         username,
                                         realm,
                                         entry->userhash_sha256,
                                         sizeof(entry->userhash_sha256));
  if (MHD_NO != res)
    res = MHD_digest_auth_calc_userdigest (MHD_DIGEST_AUTH_ALGO3_SHA256,
                                           username,
                                           realm,
                                           password,
                                           entry->userdigest_sha256,
                                           sizeof(entry->userdigest_sha256));

  if (MHD_NO != res)
    res = MHD_digest_auth_calc_userhash (MHD_DIGEST_AUTH_ALGO3_SHA512_256,
                                         username,
                                         realm,
                                         entry->userhash_sha512_256,
                                         sizeof(entry->userhash_sha512_256));
  if (MHD_NO != res)
    res =
      MHD_digest_auth_calc_userdigest (MHD_DIGEST_AUTH_ALGO3_SHA512_256,
                                       username,
                                       realm,
                                       password,
                                       entry->userdigest_sha512_256,
                                       sizeof(entry->userdigest_sha512_256));

  if (MHD_NO == res)
    return 0; /* Failure exit point */

  entry->allow_area_1 = allow_area_1;
  entry->allow_area_2 = allow_area_2;

  user_ids_used++;

  return ! 0;
}


/**
 * Find the user entry for specified username
 * @param username the username to find
 * @return NULL if no entry for specified username is found,
 *         pointer to user entry if found
 */
static struct UserEntry *
find_entry_by_username (const char *const username)
{
  size_t i;

  for (i = 0; i < (sizeof(user_ids) / sizeof(user_ids[0])); ++i)
  {
    struct UserEntry *entry;

    entry = user_ids + i;
    if (0 == strcmp (username, entry->username))
      return entry;
  }
  return NULL;
}


/**
 * Find the user entry for specified userhash
 * @param algo3 the algorithm used for userhash calculation
 * @param userhash the userhash identifier to find
 * @param userhash_size the size @a userhash in bytes
 * @return NULL if no entry for specified userhash is found,
 *         pointer to user entry if found
 */
static struct UserEntry *
find_entry_by_userhash (enum MHD_DigestAuthAlgo3 algo3,
                        const void *userhash,
                        size_t userhash_size)
{
  size_t i;

  if (MHD_digest_get_hash_size (algo3) != userhash_size)
    return NULL; /* Wrong length of the userhash */

  switch (algo3)
  {
  case MHD_DIGEST_AUTH_ALGO3_MD5:
  case MHD_DIGEST_AUTH_ALGO3_MD5_SESSION: /* An extra case not used currently */
    if (sizeof(user_ids[0].userhash_md5) != userhash_size) /* Extra check. The size was checked before */
      return NULL;
    for (i = 0; i < (sizeof(user_ids) / sizeof(user_ids[0])); ++i)
    {
      struct UserEntry *entry;

      entry = user_ids + i;
      if (0 == memcmp (userhash, entry->userhash_md5,
                       sizeof(entry->userhash_md5)))
        return entry;
    }
    break;
  case MHD_DIGEST_AUTH_ALGO3_SHA256:
  case MHD_DIGEST_AUTH_ALGO3_SHA256_SESSION: /* An extra case not used currently */
    if (sizeof(user_ids[0].userhash_sha256) != userhash_size) /* Extra check. The size was checked before */
      return NULL;
    for (i = 0; i < (sizeof(user_ids) / sizeof(user_ids[0])); ++i)
    {
      struct UserEntry *entry;

      entry = user_ids + i;
      if (0 == memcmp (userhash, entry->userhash_sha256,
                       sizeof(entry->userhash_sha256)))
        return entry;
    }
    break;
  case MHD_DIGEST_AUTH_ALGO3_SHA512_256:
  case MHD_DIGEST_AUTH_ALGO3_SHA512_256_SESSION: /* An extra case not used currently */
    if (sizeof(user_ids[0].userhash_sha512_256) != userhash_size) /* Extra check. The size was checked before */
      return NULL;
    for (i = 0; i < (sizeof(user_ids) / sizeof(user_ids[0])); ++i)
    {
      struct UserEntry *entry;

      entry = user_ids + i;
      if (0 == memcmp (userhash, entry->userhash_sha512_256,
                       sizeof(entry->userhash_sha512_256)))
        return entry;
    }
    break;
  case MHD_DIGEST_AUTH_ALGO3_INVALID: /* Mute compiler warning. Impossible value in this context. */
  default:
    break;
  }
  return NULL;
}


/**
 * Find the user entry for the user specified by provided username info
 * @param user_info the pointer to the structure username info returned by MHD
 * @return NULL if no entry for specified username info is found,
 *         pointer to user entry if found
 */
static struct UserEntry *
find_entry_by_userinfo (const struct MHD_DigestAuthUsernameInfo *username_info)
{
  if (MHD_DIGEST_AUTH_UNAME_TYPE_STANDARD <= username_info->uname_type)
    return find_entry_by_username (username_info->username);

  if (MHD_DIGEST_AUTH_UNAME_TYPE_USERHASH == username_info->uname_type)
    return find_entry_by_userhash (username_info->algo3,
                                   username_info->userhash_bin,
                                   username_info->userhash_hex_len / 2);

  return NULL; /* Should be unreachable as all cases are covered before */
}


/* *** End of "database" of users and "database" functions *** */

/* *** Requests handling *** */

/**
 * Send "Requested HTTP method is not supported" page
 * @param c the connection structure
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_page_not_found (struct MHD_Connection *c)
{
  static const char page_content[] =
    "<html><head><title>Page Not Found</title></head>" \
    "<body>The requested page not found.</body></html>";
  static const size_t page_content_len =
    (sizeof(page_content) / sizeof(char)) - 1;
  struct MHD_Response *resp;
  enum MHD_Result ret;

  resp = MHD_create_response_from_buffer_static (page_content_len,
                                                 page_content);
  if (NULL == resp)
    return MHD_NO;

  /* Ignore possible error when adding the header as the reply will work even
     without this header. */
  (void) MHD_add_response_header (resp,
                                  MHD_HTTP_HEADER_CONTENT_TYPE,
                                  "text/html");

  ret = MHD_queue_response (c, MHD_HTTP_NOT_FOUND, resp);
  MHD_destroy_response (resp);
  return ret;
}


/**
 * Get enum MHD_DigestAuthMultiAlgo3 value to be used for authentication.
 * @return the algorithm number/value
 */
static enum MHD_DigestAuthMultiAlgo3
get_m_algo (void)
{
  if (force_md5)
    return MHD_DIGEST_AUTH_MULT_ALGO3_MD5;
  else if (force_sha256)
    return MHD_DIGEST_AUTH_MULT_ALGO3_SHA256;
  else if (force_sha512_256)
    return MHD_DIGEST_AUTH_MULT_ALGO3_SHA512_256;

  /* No forced algorithm selection, let MHD to use default */
  return MHD_DIGEST_AUTH_MULT_ALGO3_ANY_NON_SESSION;
}


/**
 * Get enum MHD_DigestAuthMultiQOP value to be used for authentication.
 * @return the "Quality Of Protection" number/value
 */
static enum MHD_DigestAuthMultiQOP
get_m_QOP (void)
{
  if (allow_rfc2069)
    return MHD_DIGEST_AUTH_MULT_QOP_ANY_NON_INT;

  return MHD_DIGEST_AUTH_MULT_QOP_AUTH;
}


/**
 * Send "Authentication required" page
 * @param c the connection structure
 * @param stale if non-zero then "nonce stale" is indicated in the reply
 * @param wrong_cred if non-zero then client is informed the previously
 *                   it used wrong credentials
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_auth_required (struct MHD_Connection *c,
                          int stale,
                          int wrong_cred)
{
  static const char auth_required_content[] =
    "<html><head><title>Authentication required</title></head>" \
    "<body>The requested page needs authentication.</body></html>";
  static const size_t auth_required_content_len =
    (sizeof(auth_required_content) / sizeof(char)) - 1;
  static const char wrong_creds_content[] =
    "<html><head><title>Wrong credentials</title></head>" \
    "<body>The provided credentials are incorrect.</body></html>";
  static const size_t wrong_creds_content_len =
    (sizeof(wrong_creds_content) / sizeof(char)) - 1;
  struct MHD_Response *resp;
  enum MHD_Result ret;

  if (wrong_cred)
    stale = 0; /* Force client to ask user for username and password */

  if (! wrong_cred)
    resp = MHD_create_response_from_buffer_static (auth_required_content_len,
                                                   auth_required_content);
  else
    resp = MHD_create_response_from_buffer_static (wrong_creds_content_len,
                                                   wrong_creds_content);
  if (NULL == resp)
    return MHD_NO;

  /* Ignore possible error when adding the header as the reply will work even
     without this header. */
  (void) MHD_add_response_header (resp,
                                  MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");


  ret = MHD_queue_auth_required_response3 (
    c,
    REALM,
    OPAQUE_DATA, /* The "opaque data", not really useful */
    SEC_AREA1_URL " " SEC_AREA2_URL, /* Space-separated list of URLs' initial parts */
    resp,
    stale,
    get_m_QOP (),
    get_m_algo (),
    ! 0, /* Userhash support enabled */
    ! 0 /* UTF-8 is preferred */);
  MHD_destroy_response (resp);
  return ret;
}


/**
 * Send "Forbidden" page
 * @param c the connection structure
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_forbidden (struct MHD_Connection *c)
{
  static const char page_content[] =
    "<html><head><title>Forbidden</title></head>" \
    "<body>You do not have access to this page.</body></html>";
  static const size_t page_content_len =
    (sizeof(page_content) / sizeof(char)) - 1;
  struct MHD_Response *resp;
  enum MHD_Result ret;

  resp = MHD_create_response_from_buffer_static (page_content_len, page_content)
  ;
  if (NULL == resp)
    return MHD_NO;

  /* Ignore possible error when adding the header as the reply will work even
     without this header. */
  (void) MHD_add_response_header (resp,
                                  MHD_HTTP_HEADER_CONTENT_TYPE,
                                  "text/html");

  ret = MHD_queue_response (c, MHD_HTTP_FORBIDDEN, resp);
  MHD_destroy_response (resp);
  return ret;
}


/**
 * Send "Area 1" pages
 * @param c the connection structure
 * @param url the requested URL
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_area1_pages (struct MHD_Connection *c,
                        const char *url)
{

  if (0 == strcmp (url, SEC_AREA1_URL ""))
  {
    static const char page_content[] =
      "<html><head><title>Restricted secret page</title></head>" \
      "<body>Welcome to the restricted area</body></html>";
    static const size_t page_content_len =
      (sizeof(page_content) / sizeof(char)) - 1;
    struct MHD_Response *resp;
    enum MHD_Result ret;

    resp = MHD_create_response_from_buffer_static (page_content_len,
                                                   page_content);
    if (NULL == resp)
      return MHD_NO;

    /* Ignore possible error when adding the header as the reply will work even
       without this header. */
    (void) MHD_add_response_header (resp, MHD_HTTP_HEADER_CONTENT_TYPE,
                                    "text/html");

    ret = MHD_queue_response (c, MHD_HTTP_OK, resp);
    MHD_destroy_response (resp);
    return ret;
  }
  /* If needed: add handlers for other URLs in this area */
#if 0 /* Disabled code */
  if (0 == strcmp (url, SEC_AREA1_URL "some_path/some_page"))
  {
    /* Add page creation/processing code */
  }
#endif /* Disabled code */

  /* The requested URL is unknown */
  return reply_with_page_not_found (c);
}


/**
 * Send "Area 2" pages
 * @param c the connection structure
 * @param url the requested URL
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_area2_pages (struct MHD_Connection *c,
                        const char *url)
{

  if (0 == strcmp (url, SEC_AREA2_URL ""))
  {
    static const char page_content[] =
      "<html><head><title>Very restricted secret page</title></head>" \
      "<body>Welcome to the super restricted area</body></html>";
    static const size_t page_content_len =
      (sizeof(page_content) / sizeof(char)) - 1;
    struct MHD_Response *resp;
    enum MHD_Result ret;

    resp = MHD_create_response_from_buffer_static (page_content_len,
                                                   page_content);
    if (NULL == resp)
      return MHD_NO;

    /* Ignore possible error when adding the header as the reply will work even
       without this header. */
    (void) MHD_add_response_header (resp, MHD_HTTP_HEADER_CONTENT_TYPE,
                                    "text/html");

    ret = MHD_queue_response (c, MHD_HTTP_OK, resp);
    MHD_destroy_response (resp);
    return ret;
  }
  /* If needed: add handlers for other URLs in this area */
#if 0 /* Disabled code */
  if (0 == strcmp (url, SEC_AREA2_URL "other_path/other_page"))
  {
    /* Add page creation/processing code */
  }
#endif /* Disabled code */

  /* The requested URL is unknown */
  return reply_with_page_not_found (c);
}


/**
 * Handle client's request for secured areas
 * @param c the connection structure
 * @param url the URL requested by the client
 * @param sec_area_num the number of secured area
 * @return MHD_YES if request was handled (either with "denied" or with
 *                 "allowed" result),
 *         MHD_NO if it was an error handling the request.
 */
static enum MHD_Result
handle_sec_areas_req (struct MHD_Connection *c, const char *url, unsigned int
                      sec_area_num)
{
  struct MHD_DigestAuthUsernameInfo *username_info;
  struct UserEntry *user_entry;
  void *userdigest;
  size_t userdigest_size;
  enum MHD_DigestAuthResult auth_res;

  username_info = MHD_digest_auth_get_username3 (c);

  if (NULL == username_info)
    return reply_with_auth_required (c, 0, 0);

  user_entry = find_entry_by_userinfo (username_info);

  if (NULL == user_entry)
    return reply_with_auth_required (c, 0, 1);

  switch (username_info->algo3)
  {
  case MHD_DIGEST_AUTH_ALGO3_MD5:
    userdigest = user_entry->userdigest_md5;
    userdigest_size = sizeof(user_entry->userdigest_md5);
    break;
  case MHD_DIGEST_AUTH_ALGO3_SHA256:
    userdigest = user_entry->userdigest_sha256;
    userdigest_size = sizeof(user_entry->userdigest_sha256);
    break;
  case MHD_DIGEST_AUTH_ALGO3_SHA512_256:
    userdigest = user_entry->userdigest_sha512_256;
    userdigest_size = sizeof(user_entry->userdigest_sha512_256);
    break;
  case MHD_DIGEST_AUTH_ALGO3_MD5_SESSION:
  case MHD_DIGEST_AUTH_ALGO3_SHA256_SESSION:
  case MHD_DIGEST_AUTH_ALGO3_SHA512_256_SESSION:
    /* Not supported currently and not used by MHD.
       The client incorrectly used algorithm not advertised by the server. */
    return reply_with_auth_required (c, 0, 1);
  case MHD_DIGEST_AUTH_ALGO3_INVALID: /* Mute compiler warning */
  default:
    return MHD_NO; /* Should be unreachable */
  }

  auth_res = MHD_digest_auth_check_digest3 (
    c,
    REALM, /* Make sure to use the proper realm, not the realm provided by the client and returned by "user_entry" */
    user_entry->username,
    userdigest,
    userdigest_size,
    0, /* Use daemon's default value for nonce_timeout*/
    0, /* Use daemon's default value for max_nc */
    get_m_QOP (),
    (enum MHD_DigestAuthMultiAlgo3) username_info->algo3 /* Direct cast from "single algorithm" to "multi-algorithm" is allowed */
    );

  if (MHD_DAUTH_OK != auth_res)
  {
    int need_just_refresh_nonce;
    /* Actually MHD_DAUTH_NONCE_OTHER_COND should not be returned as
       MHD_OPTION_DIGEST_AUTH_NONCE_BIND_TYPE is not used for the daemon.
       To keep the code universal the MHD_DAUTH_NONCE_OTHER_COND is
       still checked here. */
    need_just_refresh_nonce =
      (MHD_DAUTH_NONCE_STALE == auth_res)
      || (MHD_DAUTH_NONCE_OTHER_COND == auth_res);
    return reply_with_auth_required (c,
                                     need_just_refresh_nonce,
                                     ! need_just_refresh_nonce);
  }

  /* The user successfully authenticated */

  /* Check whether access to the request area is allowed for the user */
  if (1 == sec_area_num)
  {
    if (user_entry->allow_area_1)
      return reply_with_area1_pages (c, url);
    else
      return reply_with_forbidden (c);
  }
  else if (2 == sec_area_num)
  {
    if (user_entry->allow_area_2)
      return reply_with_area2_pages (c, url);
    else
      return reply_with_forbidden (c);
  }

  return MHD_NO; /* Should be unreachable */
}


/**
 * Send the main page
 * @param c the connection structure
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_main_page (struct MHD_Connection *c)
{
  static const char page_content[] = MAIN_PAGE;
  static const size_t page_content_len =
    (sizeof(page_content) / sizeof(char)) - 1;
  struct MHD_Response *resp;
  enum MHD_Result ret;

  resp = MHD_create_response_from_buffer_static (page_content_len, page_content)
  ;
  if (NULL == resp)
    return MHD_NO;

  /* Ignore possible error when adding the header as the reply will work even
     without this header. */
  (void) MHD_add_response_header (resp,
                                  MHD_HTTP_HEADER_CONTENT_TYPE,
                                  "text/html");

  ret = MHD_queue_response (c, MHD_HTTP_OK, resp);
  MHD_destroy_response (resp);
  return ret;
}


/**
 * Send "Requested HTTP method is not supported" page
 * @param c the connection structure
 * @return MHD_YES if response was successfully queued,
 *         MHD_NO otherwise
 */
static enum MHD_Result
reply_with_method_not_supported (struct MHD_Connection *c)
{
  static const char page_content[] =
    "<html><head><title>Requested HTTP Method Is Not Supported</title></head>" \
    "<body>The requested HTTP method is not supported.</body></html>";
  static const size_t page_content_len =
    (sizeof(page_content) / sizeof(char)) - 1;
  struct MHD_Response *resp;
  enum MHD_Result ret;

  resp = MHD_create_response_from_buffer_static (page_content_len, page_content)
  ;
  if (NULL == resp)
    return MHD_NO;

  /* Ignore possible error when adding the header as the reply will work even
     without this header. */
  (void) MHD_add_response_header (resp,
                                  MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");

  ret = MHD_queue_response (c, MHD_HTTP_NOT_IMPLEMENTED, resp);
  MHD_destroy_response (resp);
  return ret;
}


static enum MHD_Result
ahc_main (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **req_cls)
{
  static int already_called_marker;
  size_t url_len;
  (void) cls;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */

  if ((0 != strcmp (method, MHD_HTTP_METHOD_GET))
      && (0 != strcmp (method, MHD_HTTP_METHOD_HEAD)))
    return reply_with_method_not_supported (connection);

  if (0 != *upload_data_size)
    return MHD_NO; /* No upload expected for GET or HEAD */

  if (&already_called_marker != *req_cls)
  { /* Called for the first time, request not fully read yet */
    *req_cls = &already_called_marker;
    /* Wait for complete request */
    return MHD_YES;
  }

  if (0 == strcmp (url, "/"))
    return reply_with_main_page (connection);

  url_len = strlen (url);

  if ((strlen (SEC_AREA1_URL) <= url_len)
      && (0 == memcmp (url, SEC_AREA1_URL, strlen (SEC_AREA1_URL))))
    return handle_sec_areas_req (connection, url, 1); /* The requested URL is within SEC_AREA1_URL */

  if ((strlen (SEC_AREA2_URL) <= url_len)
      && (0 == memcmp (url, SEC_AREA2_URL, strlen (SEC_AREA2_URL))))
    return handle_sec_areas_req (connection, url, 2); /* The requested URL is within SEC_AREA2_URL */

  return reply_with_page_not_found (connection);
}


/* *** End of requests handling *** */


/**
 * Add new users to the users "database".
 *
 * In real application this kind of function must NOT be called at
 * the application startup. Instead similar function should be
 * called only when new user is introduced. The users "database"
 * should be stored somewhere and reloaded at the application
 * startup.
 *
 * @return non-zero on success,
 *         zero in case of error.
 */
static int
add_new_users (void)
{
  if (! add_new_user_entry ("joepublic",
                            "password",
                            REALM,
                            ! 0,
                            0))
    return 0;

  if (! add_new_user_entry ("superadmin",
                            "pA$$w0Rd",
                            REALM,
                            ! 0,
                            ! 0))
    return 0;

  return ! 0;
}


/**
 * Check and apply application parameters
 * @param argc the argc of the @a main function
 * @param argv the argv of the @a main function
 * @return non-zero on success,
 *         zero in case of any error (like wrong parameters).
 */
static int
check_params (int argc, char *const *const argv)
{
  size_t i;
  unsigned int port_value;

  if (2 > argc)
    return 0;

  for (i = 1; i < (unsigned int) argc; ++i)
  {
    if (0 == strcmp (argv[i], "--md5"))
    { /* Force use MD5 */
      force_md5 = ! 0;
      force_sha256 = 0;
      force_sha512_256 = 0;
    }
    else if (0 == strcmp (argv[i], "--sha256"))
    { /* Force use SHA-256 instead of default MD5 */
      force_md5 = 0;
      force_sha256 = ! 0;
      force_sha512_256 = 0;
    }
    else if (0 == strcmp (argv[i], "--sha512-256"))
    { /* Force use SHA-512/256 instead of default MD5 */
      force_md5 = 0;
      force_sha256 = 0;
      force_sha512_256 = ! 0;
    }
    else if (0 == strcmp (argv[i], "--allow-rfc2069"))
      allow_rfc2069 = ! 0; /* Allow fallback to RFC2069. Not recommended! */
    else if ((1 == sscanf (argv[i], "%u", &port_value))
             && (0 < port_value) && (65535 >= port_value))
      daemon_port = (uint16_t) port_value;
    else
    {
      fprintf (stderr, "Unrecognized parameter: %s\n",
               argv[i]);
      return 0;
    }
  }

  if (force_sha512_256)
    printf (
      "Note: when testing with curl/libcurl do not be surprised with failures as "
      "libcurl incorrectly implements SHA-512/256 algorithm.\n");
  return ! 0;
}


/**
 * The cryptographically secure random data
 */
static uint8_t rand_data[8];

/**
 * Initialise the random data
 * @return non-zero if succeed,
 *         zero if failed
 */
static int
init_rand_data (void)
{
#if ! defined(_WIN32) || defined(__CYGWIN__)
  int fd;
  ssize_t len;
  size_t off;

  fd = open ("/dev/urandom", O_RDONLY);
  if (-1 == fd)
  {
    fprintf (stderr, "Failed to open '%s': %s\n",
             "/dev/urandom",
             strerror (errno));
    return 0;
  }
  for (off = 0; off < sizeof(rand_data); off += (size_t) len)
  {
    len = read (fd, rand_data, 8);
    if (0 > len)
    {
      fprintf (stderr, "Failed to read '%s': %s\n",
               "/dev/urandom",
               strerror (errno));
      (void) close (fd);
      return 0;
    }
  }
  (void) close (fd);
#else  /* Native W32 */
  HCRYPTPROV cc;
  BOOL b;

  b = CryptAcquireContext (&cc,
                           NULL,
                           NULL,
                           PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT);
  if (FALSE == b)
  {
    fprintf (stderr,
             "Failed to acquire crypto provider context: %lu\n",
             (unsigned long) GetLastError ());
    return 0;
  }
  b = CryptGenRandom (cc, sizeof(rand_data), (BYTE *) rand_data);
  if (FALSE == b)
  {
    fprintf (stderr,
             "Failed to generate 8 random bytes: %lu\n",
             GetLastError ());
  }
  CryptReleaseContext (cc, 0);
  if (FALSE == b)
    return 0;
#endif /* Native W32 */

  return ! 0;
}


int
main (int argc, char *const *argv)
{
  struct MHD_Daemon *d;

  if (! check_params (argc, argv))
  {
    fprintf (stderr, "Usage: %s [--md5|--sha256|--sha512-256] "
             "[--allow-rfc2069] PORT\n", argv[0]);
    return 1;
  }
  if (! add_new_users ())
  {
    fprintf (stderr, "Failed to add new users to the users database.\n");
    return 2;
  }
  if (! init_rand_data ())
  {
    fprintf (stderr, "Failed to initialise random data.\n");
    return 2;
  }

  d = MHD_start_daemon (
    MHD_USE_INTERNAL_POLLING_THREAD
    | MHD_USE_THREAD_PER_CONNECTION
    | MHD_USE_ERROR_LOG,
    daemon_port,
    NULL, NULL, &ahc_main, NULL,
    MHD_OPTION_DIGEST_AUTH_RANDOM, sizeof(rand_data), rand_data,
    MHD_OPTION_NONCE_NC_SIZE, 500,
    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 180,
    MHD_OPTION_END);
  if (d == NULL)
  {
    fprintf (stderr, "Failed to start the server on port %lu.\n",
             (unsigned long) daemon_port);
    return 1;
  }
  printf ("Running server on port %lu.\nPress ENTER to stop.\n",
          (unsigned long) daemon_port);
  (void) getc (stdin);
  MHD_stop_daemon (d);
  return 0;
}


/* End of digest_auth_example_adv.c */
