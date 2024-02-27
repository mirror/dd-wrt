/*
  This file is part of libmicrohttpd
  Copyright (C) 2015-2023 Karlson2k (Evgeny Grin)

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
 * @file microhttpd/mhd_str.h
 * @brief  Header for string manipulating helpers
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_STR_H
#define MHD_STR_H 1

#include "mhd_options.h"
#include <stdint.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */

#include "mhd_str_types.h"

#if defined(_MSC_FULL_VER) && ! defined(_SSIZE_T_DEFINED)
#define _SSIZE_T_DEFINED
typedef intptr_t ssize_t;
#endif /* !_SSIZE_T_DEFINED */

#ifdef MHD_FAVOR_SMALL_CODE
#include "mhd_limits.h"
#endif /* MHD_FAVOR_SMALL_CODE */

/*
 * Block of functions/macros that use US-ASCII charset as required by HTTP
 * standards. Not affected by current locale settings.
 */

#ifndef MHD_FAVOR_SMALL_CODE
/**
 * Check two strings for equality, ignoring case of US-ASCII letters.
 *
 * @param str1 first string to compare
 * @param str2 second string to compare
 * @return non-zero if two strings are equal, zero otherwise.
 */
int
MHD_str_equal_caseless_ (const char *str1,
                         const char *str2);

#else  /* MHD_FAVOR_SMALL_CODE */
/* Reuse MHD_str_equal_caseless_n_() to reduce size */
#define MHD_str_equal_caseless_(s1,s2) MHD_str_equal_caseless_n_ ((s1),(s2), \
                                                                  SIZE_MAX)
#endif /* MHD_FAVOR_SMALL_CODE */


/**
 * Check two string for equality, ignoring case of US-ASCII letters and
 * checking not more than @a maxlen characters.
 * Compares up to first terminating null character, but not more than
 * first @a maxlen characters.
 * @param str1 first string to compare
 * @param str2 second string to compare
 * @param maxlen maximum number of characters to compare
 * @return non-zero if two strings are equal, zero otherwise.
 */
int
MHD_str_equal_caseless_n_ (const char *const str1,
                           const char *const str2,
                           size_t maxlen);


/**
 * Check two string for equality, ignoring case of US-ASCII letters and
 * checking not more than @a len bytes.
 * Compares not more first than @a len bytes, including binary zero characters.
 * Comparison stops at first unmatched byte.
 * @param str1 first string to compare
 * @param str2 second string to compare
 * @param len number of characters to compare
 * @return non-zero if @a len bytes are equal, zero otherwise.
 */
bool
MHD_str_equal_caseless_bin_n_ (const char *const str1,
                               const char *const str2,
                               size_t len);


/**
 * Check whether string is equal statically allocated another string,
 * ignoring case of US-ASCII letters and checking not more than @a len bytes.
 *
 * If strings have different sizes (lengths) then macro returns boolean false
 * without checking the content.
 *
 * Compares not more first than @a len bytes, including binary zero characters.
 * Comparison stops at first unmatched byte.
 * @param a the statically allocated string to compare
 * @param s the string to compare
 * @param len number of characters to compare
 * @return non-zero if @a len bytes are equal, zero otherwise.
 */
#define MHD_str_equal_caseless_s_bin_n_(a,s,l) \
  ((MHD_STATICSTR_LEN_(a) == (l)) \
   && MHD_str_equal_caseless_bin_n_(a,s,l))

/**
 * Check whether @a str has case-insensitive @a token.
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Match succeed if substring between start, end (of string) or comma
 * contains only case-insensitive token and optional spaces and tabs.
 * @warning token must not contain null-characters except optional
 *          terminating null-character.
 * @param str the string to check
 * @param token the token to find
 * @param token_len length of token, not including optional terminating
 *                  null-character.
 * @return non-zero if two strings are equal, zero otherwise.
 */
bool
MHD_str_has_token_caseless_ (const char *str,
                             const char *const token,
                             size_t token_len);

/**
 * Check whether @a str has case-insensitive static @a tkn.
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Match succeed if substring between start, end of string or comma
 * contains only case-insensitive token and optional spaces and tabs.
 * @warning tkn must be static string
 * @param str the string to check
 * @param tkn the static string of token to find
 * @return non-zero if two strings are equal, zero otherwise.
 */
#define MHD_str_has_s_token_caseless_(str,tkn) \
  MHD_str_has_token_caseless_ ((str),(tkn),MHD_STATICSTR_LEN_ (tkn))


/**
 * Remove case-insensitive @a token from the @a str and put result
 * to the output @a buf.
 *
 * Tokens in @a str could be surrounded by spaces and tabs and delimited by
 * comma. The token match succeed if substring between start, end (of string)
 * or comma contains only case-insensitive token and optional spaces and tabs.
 * The quoted strings and comments are not supported by this function.
 *
 * The output string is normalised: empty tokens and repeated whitespaces
 * are removed, no whitespaces before commas, exactly one space is used after
 * each comma.
 *
 * @param str the string to process
 * @param str_len the length of the @a str, not including optional
 *                terminating null-character.
 * @param token the token to find
 * @param token_len the length of @a token, not including optional
 *                  terminating null-character.
 * @param[out] buf the output buffer, not null-terminated.
 * @param[in,out] buf_size pointer to the size variable, at input it
 *                         is the size of allocated buffer, at output
 *                         it is the size of the resulting string (can
 *                         be up to 50% larger than input) or negative value
 *                         if there is not enough space for the result
 * @return 'true' if token has been removed,
 *         'false' otherwise.
 */
bool
MHD_str_remove_token_caseless_ (const char *str,
                                size_t str_len,
                                const char *const token,
                                const size_t token_len,
                                char *buf,
                                ssize_t *buf_size);


/**
 * Perform in-place case-insensitive removal of @a tokens from the @a str.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * The token match succeed if substring between start, end (of the string), or
 * comma contains only case-insensitive token and optional spaces and tabs.
 * The quoted strings and comments are not supported by this function.
 *
 * The input string must be normalised: empty tokens and repeated whitespaces
 * are removed, no whitespaces before commas, exactly one space is used after
 * each comma. The string is updated in-place.
 *
 * Behavior is undefined is the input string in not normalised.
 *
 * @param[in,out] str the string to update
 * @param[in,out] str_len the length of the @a str, not including optional
 *                        terminating null-character, not null-terminated
 * @param tokens the token to find
 * @param tokens_len the length of @a tokens, not including optional
 *                   terminating null-character.
 * @return 'true' if any token has been removed,
 *         'false' otherwise.
 */
bool
MHD_str_remove_tokens_caseless_ (char *str,
                                 size_t *str_len,
                                 const char *const tokens,
                                 const size_t tokens_len);


#ifndef MHD_FAVOR_SMALL_CODE
/* Use individual function for each case to improve speed */

/**
 * Convert decimal US-ASCII digits in string to number in uint64_t.
 * Conversion stopped at first non-digit character.
 *
 * @param str string to convert
 * @param[out] out_val pointer to uint64_t to store result of conversion
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then possible to store in uint64_t or @a out_val is NULL
 */
size_t
MHD_str_to_uint64_ (const char *str,
                    uint64_t *out_val);

/**
 * Convert not more then @a maxlen decimal US-ASCII digits in string to
 * number in uint64_t.
 * Conversion stopped at first non-digit character or after @a maxlen
 * digits.
 *
 * @param str string to convert
 * @param maxlen maximum number of characters to process
 * @param[out] out_val pointer to uint64_t to store result of conversion
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then possible to store in uint64_t or @a out_val is NULL
 */
size_t
MHD_str_to_uint64_n_ (const char *str,
                      size_t maxlen,
                      uint64_t *out_val);


/**
 * Convert hexadecimal US-ASCII digits in string to number in uint32_t.
 * Conversion stopped at first non-digit character.
 *
 * @param str string to convert
 * @param[out] out_val pointer to uint32_t to store result of conversion
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then possible to store in uint32_t or @a out_val is NULL
 */
size_t
MHD_strx_to_uint32_ (const char *str,
                     uint32_t *out_val);


/**
 * Convert not more then @a maxlen hexadecimal US-ASCII digits in string
 * to number in uint32_t.
 * Conversion stopped at first non-digit character or after @a maxlen
 * digits.
 *
 * @param str string to convert
 * @param maxlen maximum number of characters to process
 * @param[out] out_val pointer to uint32_t to store result of conversion
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then possible to store in uint32_t or @a out_val is NULL
 */
size_t
MHD_strx_to_uint32_n_ (const char *str,
                       size_t maxlen,
                       uint32_t *out_val);


/**
 * Convert hexadecimal US-ASCII digits in string to number in uint64_t.
 * Conversion stopped at first non-digit character.
 *
 * @param str string to convert
 * @param[out] out_val pointer to uint64_t to store result of conversion
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then possible to store in uint64_t or @a out_val is NULL
 */
size_t
MHD_strx_to_uint64_ (const char *str,
                     uint64_t *out_val);


/**
 * Convert not more then @a maxlen hexadecimal US-ASCII digits in string
 * to number in uint64_t.
 * Conversion stopped at first non-digit character or after @a maxlen
 * digits.
 *
 * @param str string to convert
 * @param maxlen maximum number of characters to process
 * @param[out] out_val pointer to uint64_t to store result of conversion
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then possible to store in uint64_t or @a out_val is NULL
 */
size_t
MHD_strx_to_uint64_n_ (const char *str,
                       size_t maxlen,
                       uint64_t *out_val);

#else  /* MHD_FAVOR_SMALL_CODE */
/* Use one universal function and macros to reduce size */

/**
 * Generic function for converting not more then @a maxlen
 * hexadecimal or decimal US-ASCII digits in string to number.
 * Conversion stopped at first non-digit character or after @a maxlen
 * digits.
 * To be used only within macro.
 *
 * @param str the string to convert
 * @param maxlen the maximum number of characters to process
 * @param out_val the pointer to variable to store result of conversion
 * @param val_size the size of variable pointed by @a out_val, in bytes, 4 or 8
 * @param max_val the maximum decoded number
 * @param base the numeric base, 10 or 16
 * @return non-zero number of characters processed on succeed,
 *         zero if no digit is found, resulting value is larger
 *         then @a max_val, @a val_size is not 4/8 or @a out_val is NULL
 */
size_t
MHD_str_to_uvalue_n_ (const char *str,
                      size_t maxlen,
                      void *out_val,
                      size_t val_size,
                      uint64_t max_val,
                      unsigned int base);

#define MHD_str_to_uint64_(s,ov) MHD_str_to_uvalue_n_ ((s),SIZE_MAX,(ov), \
                                                       sizeof(uint64_t), \
                                                       UINT64_MAX,10)

#define MHD_str_to_uint64_n_(s,ml,ov) MHD_str_to_uvalue_n_ ((s),(ml),(ov), \
                                                            sizeof(uint64_t), \
                                                            UINT64_MAX,10)

#define MHD_strx_to_sizet_(s,ov) MHD_str_to_uvalue_n_ ((s),SIZE_MAX,(ov), \
                                                       sizeof(size_t),SIZE_MAX, \
                                                       16)

#define MHD_strx_to_sizet_n_(s,ml,ov) MHD_str_to_uvalue_n_ ((s),(ml),(ov), \
                                                            sizeof(size_t), \
                                                            SIZE_MAX,16)

#define MHD_strx_to_uint32_(s,ov) MHD_str_to_uvalue_n_ ((s),SIZE_MAX,(ov), \
                                                        sizeof(uint32_t), \
                                                        UINT32_MAX,16)

#define MHD_strx_to_uint32_n_(s,ml,ov) MHD_str_to_uvalue_n_ ((s),(ml),(ov), \
                                                             sizeof(uint32_t), \
                                                             UINT32_MAX,16)

#define MHD_strx_to_uint64_(s,ov) MHD_str_to_uvalue_n_ ((s),SIZE_MAX,(ov), \
                                                        sizeof(uint64_t), \
                                                        UINT64_MAX,16)

#define MHD_strx_to_uint64_n_(s,ml,ov) MHD_str_to_uvalue_n_ ((s),(ml),(ov), \
                                                             sizeof(uint64_t), \
                                                             UINT64_MAX,16)

#endif /* MHD_FAVOR_SMALL_CODE */


/**
 * Convert uint32_t value to hexdecimal US-ASCII string.
 * @note: result is NOT zero-terminated.
 * @param val the value to convert
 * @param buf the buffer to result to
 * @param buf_size size of the @a buffer
 * @return number of characters has been put to the @a buf,
 *         zero if buffer is too small (buffer may be modified).
 */
size_t
MHD_uint32_to_strx (uint32_t val,
                    char *buf,
                    size_t buf_size);


#ifndef MHD_FAVOR_SMALL_CODE
/**
 * Convert uint16_t value to decimal US-ASCII string.
 * @note: result is NOT zero-terminated.
 * @param val the value to convert
 * @param buf the buffer to result to
 * @param buf_size size of the @a buffer
 * @return number of characters has been put to the @a buf,
 *         zero if buffer is too small (buffer may be modified).
 */
size_t
MHD_uint16_to_str (uint16_t val,
                   char *buf,
                   size_t buf_size);

#else  /* MHD_FAVOR_SMALL_CODE */
#define MHD_uint16_to_str(v,b,s) MHD_uint64_to_str(v,b,s)
#endif /* MHD_FAVOR_SMALL_CODE */


/**
 * Convert uint64_t value to decimal US-ASCII string.
 * @note: result is NOT zero-terminated.
 * @param val the value to convert
 * @param buf the buffer to result to
 * @param buf_size size of the @a buffer
 * @return number of characters has been put to the @a buf,
 *         zero if buffer is too small (buffer may be modified).
 */
size_t
MHD_uint64_to_str (uint64_t val,
                   char *buf,
                   size_t buf_size);


/**
 * Convert uint16_t value to decimal US-ASCII string padded with
 * zeros on the left side.
 *
 * @note: result is NOT zero-terminated.
 * @param val the value to convert
 * @param min_digits the minimal number of digits to print,
 *                   output padded with zeros on the left side,
 *                   'zero' value is interpreted as 'one',
 *                   valid values are 3, 2, 1, 0
 * @param buf the buffer to result to
 * @param buf_size size of the @a buffer
 * @return number of characters has been put to the @a buf,
 *         zero if buffer is too small (buffer may be modified).
 */
size_t
MHD_uint8_to_str_pad (uint8_t val,
                      uint8_t min_digits,
                      char *buf,
                      size_t buf_size);


/**
 * Convert @a size bytes from input binary data to lower case
 * hexadecimal digits.
 * Result is NOT zero-terminated
 * @param bin the pointer to the binary data to convert
 * @param size the size in bytes of the binary data to convert
 * @param[out] hex the output buffer, should be at least 2 * @a size
 * @return The number of characters written to the output buffer.
 */
size_t
MHD_bin_to_hex (const void *bin,
                size_t size,
                char *hex);

/**
 * Convert @a size bytes from input binary data to lower case
 * hexadecimal digits, zero-terminate the result.
 * @param bin the pointer to the binary data to convert
 * @param size the size in bytes of the binary data to convert
 * @param[out] hex the output buffer, should be at least 2 * @a size + 1
 * @return The number of characters written to the output buffer,
 *         not including terminating zero.
 */
size_t
MHD_bin_to_hex_z (const void *bin,
                  size_t size,
                  char *hex);

/**
 * Convert hexadecimal digits to binary data.
 *
 * The input decoded byte-by-byte (each byte is two hexadecimal digits).
 * If length is an odd number, extra leading zero is assumed.
 *
 * @param hex the input string with hexadecimal digits
 * @param len the length of the input string
 * @param[out] bin the output buffer, must be at least len/2 bytes long (or
 *                 len/2 + 1 if @a len is not even number)
 * @return the number of bytes written to the output buffer,
 *         zero if found any character which is not hexadecimal digits
 */
size_t
MHD_hex_to_bin (const char *hex,
                size_t len,
                void *bin);

/**
 * Decode string with percent-encoded characters as defined by
 * RFC 3986 #section-2.1.
 *
 * This function decode string by converting percent-encoded characters to
 * their decoded versions and copying all other characters without extra
 * processing.
 *
 * @param pct_encoded the input string to be decoded
 * @param pct_encoded_len the length of the @a pct_encoded
 * @param[out] decoded the output buffer, NOT zero-terminated, can point
 *                     to the same buffer as @a pct_encoded
 * @param buf_size the size of the output buffer
 * @return the number of characters written to the output buffer or
 *         zero if any percent-encoded characters is broken ('%' followed
 *         by less than two hexadecimal digits) or output buffer is too
 *         small to hold the result
 */
size_t
MHD_str_pct_decode_strict_n_ (const char *pct_encoded,
                              size_t pct_encoded_len,
                              char *decoded,
                              size_t buf_size);

/**
 * Decode string with percent-encoded characters as defined by
 * RFC 3986 #section-2.1.
 *
 * This function decode string by converting percent-encoded characters to
 * their decoded versions and copying all other characters without extra
 * processing.
 *
 * Any invalid percent-encoding sequences ('%' symbol not followed by two
 * valid hexadecimal digits) are copied to the output string without decoding.
 *
 * @param pct_encoded the input string to be decoded
 * @param pct_encoded_len the length of the @a pct_encoded
 * @param[out] decoded the output buffer, NOT zero-terminated, can point
 *                     to the same buffer as @a pct_encoded
 * @param buf_size the size of the output buffer
 * @param[out] broken_encoding will be set to true if any '%' symbol is not
 *                             followed by two valid hexadecimal digits,
 *                             optional, can be NULL
 * @return the number of characters written to the output buffer or
 *         zero if output buffer is too small to hold the result
 */
size_t
MHD_str_pct_decode_lenient_n_ (const char *pct_encoded,
                               size_t pct_encoded_len,
                               char *decoded,
                               size_t buf_size,
                               bool *broken_encoding);


/**
 * Decode string in-place with percent-encoded characters as defined by
 * RFC 3986 #section-2.1.
 *
 * This function decode string by converting percent-encoded characters to
 * their decoded versions and copying back all other characters without extra
 * processing.
 *
 * @param[in,out] str the string to be updated in-place, must be zero-terminated
 *                    on input, the output is zero-terminated; the string is
 *                    truncated to zero length if broken encoding is found
 * @return the number of character in decoded string
 */
size_t
MHD_str_pct_decode_in_place_strict_ (char *str);


/**
 * Decode string in-place with percent-encoded characters as defined by
 * RFC 3986 #section-2.1.
 *
 * This function decode string by converting percent-encoded characters to
 * their decoded versions and copying back all other characters without extra
 * processing.
 *
 * Any invalid percent-encoding sequences ('%' symbol not followed by two
 * valid hexadecimal digits) are copied to the output string without decoding.
 *
 * @param[in,out] str the string to be updated in-place, must be zero-terminated
 *                    on input, the output is zero-terminated
 * @param[out] broken_encoding will be set to true if any '%' symbol is not
 *                             followed by two valid hexadecimal digits,
 *                             optional, can be NULL
 * @return the number of character in decoded string
 */
size_t
MHD_str_pct_decode_in_place_lenient_ (char *str,
                                      bool *broken_encoding);

#ifdef DAUTH_SUPPORT
/**
 * Check two strings for equality, "unquoting" the first string from quoted
 * form as specified by RFC7230#section-3.2.6 and RFC7694#quoted.strings.
 *
 * Null-termination for input strings is not required, binary zeros compared
 * like other characters.
 *
 * @param quoted the quoted string to compare, must NOT include leading and
 *               closing DQUOTE chars, does not need to be zero-terminated
 * @param quoted_len the length in chars of the @a quoted string
 * @param unquoted the unquoted string to compare, does not need to be
 *                 zero-terminated
 * @param unquoted_len the length in chars of the @a unquoted string
 * @return zero if quoted form is broken (no character after the last escaping
 *         backslash), zero if strings are not equal after unquoting of the
 *         first string,
 *         non-zero if two strings are equal after unquoting of the
 *         first string.
 */
bool
MHD_str_equal_quoted_bin_n (const char *quoted,
                            size_t quoted_len,
                            const char *unquoted,
                            size_t unquoted_len);

/**
 * Check whether the string after "unquoting" equals static string.
 *
 * Null-termination for input string is not required, binary zeros compared
 * like other characters.
 *
 * @param q the quoted string to compare, must NOT include leading and
 *          closing DQUOTE chars, does not need to be zero-terminated
 * @param l the length in chars of the @a q string
 * @param u the unquoted static string to compare
 * @return zero if quoted form is broken (no character after the last escaping
 *         backslash), zero if strings are not equal after unquoting of the
 *         first string,
 *         non-zero if two strings are equal after unquoting of the
 *         first string.
 */
#define MHD_str_equal_quoted_s_bin_n(q,l,u) \
    MHD_str_equal_quoted_bin_n(q,l,u,MHD_STATICSTR_LEN_(u))

/**
 * Check two strings for equality, "unquoting" the first string from quoted
 * form as specified by RFC7230#section-3.2.6 and RFC7694#quoted.strings and
 * ignoring case of US-ASCII letters.
 *
 * Null-termination for input strings is not required, binary zeros compared
 * like other characters.
 *
 * @param quoted the quoted string to compare, must NOT include leading and
 *               closing DQUOTE chars, does not need to be zero-terminated
 * @param quoted_len the length in chars of the @a quoted string
 * @param unquoted the unquoted string to compare, does not need to be
 *                 zero-terminated
 * @param unquoted_len the length in chars of the @a unquoted string
 * @return zero if quoted form is broken (no character after the last escaping
 *         backslash), zero if strings are not equal after unquoting of the
 *         first string,
 *         non-zero if two strings are caseless equal after unquoting of the
 *         first string.
 */
bool
MHD_str_equal_caseless_quoted_bin_n (const char *quoted,
                                     size_t quoted_len,
                                     const char *unquoted,
                                     size_t unquoted_len);

/**
 * Check whether the string after "unquoting" equals static string, ignoring
 * case of US-ASCII letters.
 *
 * Null-termination for input string is not required, binary zeros compared
 * like other characters.
 *
 * @param q the quoted string to compare, must NOT include leading and
 *          closing DQUOTE chars, does not need to be zero-terminated
 * @param l the length in chars of the @a q string
 * @param u the unquoted static string to compare
 * @return zero if quoted form is broken (no character after the last escaping
 *         backslash), zero if strings are not equal after unquoting of the
 *         first string,
 *         non-zero if two strings are caseless equal after unquoting of the
 *         first string.
 */
#define MHD_str_equal_caseless_quoted_s_bin_n(q,l,u) \
    MHD_str_equal_caseless_quoted_bin_n(q,l,u,MHD_STATICSTR_LEN_(u))

/**
 * Convert string from quoted to unquoted form as specified by
 * RFC7230#section-3.2.6 and RFC7694#quoted.strings.
 *
 * @param quoted the quoted string, must NOT include leading and closing
 *               DQUOTE chars, does not need to be zero-terminated
 * @param quoted_len the length in chars of the @a quoted string
 * @param[out] result the pointer to the buffer to put the result, must
 *                    be at least @a size character long. May be modified even
 *                    if @a quoted is invalid sequence. The result is NOT
 *                    zero-terminated.
 * @return The number of characters written to the output buffer,
 *         zero if last backslash is not followed by any character (or
 *         @a quoted_len is zero).
 */
size_t
MHD_str_unquote (const char *quoted,
                 size_t quoted_len,
                 char *result);

#endif /* DAUTH_SUPPORT */

#if defined(DAUTH_SUPPORT) || defined(BAUTH_SUPPORT)

/**
 * Convert string from unquoted to quoted form as specified by
 * RFC7230#section-3.2.6 and RFC7694#quoted.strings.
 *
 * @param unquoted the unquoted string, does not need to be zero-terminated
 * @param unquoted_len the length in chars of the @a unquoted string
 * @param[out] result the pointer to the buffer to put the result. May be
 *                    modified even if function failed due to insufficient
 *                    space. The result is NOT zero-terminated and does not
 *                    have opening and closing DQUOTE chars.
 * @param buf_size the size of the allocated memory for @a result
 * @return The number of copied characters, can be up to two times more than
 *         @a unquoted_len, zero if @a unquoted_len is zero or if quoted
 *         string is larger than @a buf_size.
 */
size_t
MHD_str_quote (const char *unquoted,
               size_t unquoted_len,
               char *result,
               size_t buf_size);

#endif /* DAUTH_SUPPORT || BAUTH_SUPPORT */

#ifdef BAUTH_SUPPORT

/**
 * Returns the maximum possible size of the Base64 decoded data.
 * The real recoded size could be up to two bytes smaller.
 * @param enc_size the size of encoded data, in characters
 * @return the maximum possible size of the decoded data, in bytes, if
 *         @a enc_size is valid (properly padded),
 *         undefined value smaller then @a enc_size if @a enc_size is not valid
 */
#define MHD_base64_max_dec_size_(enc_size) (((enc_size) / 4) * 3)

/**
 * Convert Base64 encoded string to binary data.
 * @param base64 the input string with Base64 encoded data, could be NOT zero
 *               terminated
 * @param base64_len the number of characters to decode in @a base64 string,
 *                   valid number must be a multiple of four
 * @param[out] bin the pointer to the output buffer, the buffer may be altered
 *                 even if decoding failed
 * @param bin_size the size of the @a bin buffer in bytes, if the size is
 *                 at least @a base64_len / 4 * 3 then result will always
 *                 fit, regardless of the amount of the padding characters
 * @return 0 if @a base64_len is zero, or input string has wrong data (not
 *         valid Base64 sequence), or @a bin_size is too small;
 *         non-zero number of bytes written to the @a bin, the number must be
 *         (base64_len / 4 * 3 - 2), (base64_len / 4 * 3 - 1) or
 *         (base64_len / 4 * 3), depending on the number of padding characters.
 */
size_t
MHD_base64_to_bin_n (const char *base64,
                     size_t base64_len,
                     void *bin,
                     size_t bin_size);

#endif /* BAUTH_SUPPORT */

#endif /* MHD_STR_H */
