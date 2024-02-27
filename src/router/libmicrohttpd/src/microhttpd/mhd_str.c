/*
  This file is part of libmicrohttpd
  Copyright (C) 2015-2024 Karlson2k (Evgeny Grin)

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
 * @file microhttpd/mhd_str.c
 * @brief  Functions implementations for string manipulating
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_str.h"

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>

#include "mhd_assert.h"
#include "mhd_limits.h"
#include "mhd_assert.h"

#ifdef MHD_FAVOR_SMALL_CODE
#ifdef _MHD_static_inline
#undef _MHD_static_inline
#endif /* _MHD_static_inline */
/* Do not force inlining and do not use macro functions, use normal static
   functions instead.
   This may give more flexibility for size optimizations. */
#define _MHD_static_inline static
#ifndef INLINE_FUNC
#define INLINE_FUNC 1
#endif /* !INLINE_FUNC */
#endif /* MHD_FAVOR_SMALL_CODE */

/*
 * Block of functions/macros that use US-ASCII charset as required by HTTP
 * standards. Not affected by current locale settings.
 */

#ifdef INLINE_FUNC

#if 0 /* Disable unused functions. */
/**
 * Check whether character is lower case letter in US-ASCII
 *
 * @param c character to check
 * @return non-zero if character is lower case letter, zero otherwise
 */
_MHD_static_inline bool
isasciilower (char c)
{
  return (c >= 'a') && (c <= 'z');
}


#endif /* Disable unused functions. */


/**
 * Check whether character is upper case letter in US-ASCII
 *
 * @param c character to check
 * @return non-zero if character is upper case letter, zero otherwise
 */
_MHD_static_inline bool
isasciiupper (char c)
{
  return (c >= 'A') && (c <= 'Z');
}


#if 0 /* Disable unused functions. */
/**
 * Check whether character is letter in US-ASCII
 *
 * @param c character to check
 * @return non-zero if character is letter in US-ASCII, zero otherwise
 */
_MHD_static_inline bool
isasciialpha (char c)
{
  return isasciilower (c) || isasciiupper (c);
}


#endif /* Disable unused functions. */


/**
 * Check whether character is decimal digit in US-ASCII
 *
 * @param c character to check
 * @return non-zero if character is decimal digit, zero otherwise
 */
_MHD_static_inline bool
isasciidigit (char c)
{
  return (c >= '0') && (c <= '9');
}


#if 0 /* Disable unused functions. */
/**
 * Check whether character is hexadecimal digit in US-ASCII
 *
 * @param c character to check
 * @return non-zero if character is decimal digit, zero otherwise
 */
_MHD_static_inline bool
isasciixdigit (char c)
{
  return isasciidigit (c) ||
         ( (c >= 'A') && (c <= 'F') ) ||
         ( (c >= 'a') && (c <= 'f') );
}


/**
 * Check whether character is decimal digit or letter in US-ASCII
 *
 * @param c character to check
 * @return non-zero if character is decimal digit or letter, zero otherwise
 */
_MHD_static_inline bool
isasciialnum (char c)
{
  return isasciialpha (c) || isasciidigit (c);
}


#endif /* Disable unused functions. */


#if 0 /* Disable unused functions. */
/**
 * Convert US-ASCII character to lower case.
 * If character is upper case letter in US-ASCII than it's converted to lower
 * case analog. If character is NOT upper case letter than it's returned
 * unmodified.
 *
 * @param c character to convert
 * @return converted to lower case character
 */
_MHD_static_inline char
toasciilower (char c)
{
  return isasciiupper (c) ? (c - 'A' + 'a') : c;
}


/**
 * Convert US-ASCII character to upper case.
 * If character is lower case letter in US-ASCII than it's converted to upper
 * case analog. If character is NOT lower case letter than it's returned
 * unmodified.
 *
 * @param c character to convert
 * @return converted to upper case character
 */
_MHD_static_inline char
toasciiupper (char c)
{
  return isasciilower (c) ? (c - 'a' + 'A') : c;
}


#endif /* Disable unused functions. */


#if defined(MHD_FAVOR_SMALL_CODE) /* Used only in MHD_str_to_uvalue_n_() */
/**
 * Convert US-ASCII decimal digit to its value.
 *
 * @param c character to convert
 * @return value of decimal digit or -1 if @ c is not decimal digit
 */
_MHD_static_inline int
todigitvalue (char c)
{
  if (isasciidigit (c))
    return (unsigned char) (c - '0');

  return -1;
}


#endif /* MHD_FAVOR_SMALL_CODE */


/**
 * Convert US-ASCII hexadecimal digit to its value.
 *
 * @param c character to convert
 * @return value of hexadecimal digit or -1 if @ c is not hexadecimal digit
 */
_MHD_static_inline int
toxdigitvalue (char c)
{
#if ! defined(MHD_FAVOR_SMALL_CODE)
  switch ((unsigned char) c)
  {
#if 0 /* Disabled to give the compiler a hint about low probability */
  case 0x00U:    /* NUL */
  case 0x01U:    /* SOH */
  case 0x02U:    /* STX */
  case 0x03U:    /* ETX */
  case 0x04U:    /* EOT */
  case 0x05U:    /* ENQ */
  case 0x06U:    /* ACK */
  case 0x07U:    /* BEL */
  case 0x08U:    /* BS */
  case 0x09U:    /* HT */
  case 0x0AU:    /* LF */
  case 0x0BU:    /* VT */
  case 0x0CU:    /* FF */
  case 0x0DU:    /* CR */
  case 0x0EU:    /* SO */
  case 0x0FU:    /* SI */
  case 0x10U:    /* DLE */
  case 0x11U:    /* DC1 */
  case 0x12U:    /* DC2 */
  case 0x13U:    /* DC3 */
  case 0x14U:    /* DC4 */
  case 0x15U:    /* NAK */
  case 0x16U:    /* SYN */
  case 0x17U:    /* ETB */
  case 0x18U:    /* CAN */
  case 0x19U:    /* EM */
  case 0x1AU:    /* SUB */
  case 0x1BU:    /* ESC */
  case 0x1CU:    /* FS */
  case 0x1DU:    /* GS */
  case 0x1EU:    /* RS */
  case 0x1FU:    /* US */
  case 0x20U:    /* ' ' */
  case 0x21U:    /* '!' */
  case 0x22U:    /* '"' */
  case 0x23U:    /* '#' */
  case 0x24U:    /* '$' */
  case 0x25U:    /* '%' */
  case 0x26U:    /* '&' */
  case 0x27U:    /* '\'' */
  case 0x28U:    /* '(' */
  case 0x29U:    /* ')' */
  case 0x2AU:    /* '*' */
  case 0x2BU:    /* '+' */
  case 0x2CU:    /* ',' */
  case 0x2DU:    /* '-' */
  case 0x2EU:    /* '.' */
  case 0x2FU:    /* '/' */
    return -1;
#endif
  case 0x30U: /* '0' */
    return 0;
  case 0x31U: /* '1' */
    return 1;
  case 0x32U: /* '2' */
    return 2;
  case 0x33U: /* '3' */
    return 3;
  case 0x34U: /* '4' */
    return 4;
  case 0x35U: /* '5' */
    return 5;
  case 0x36U: /* '6' */
    return 6;
  case 0x37U: /* '7' */
    return 7;
  case 0x38U: /* '8' */
    return 8;
  case 0x39U: /* '9' */
    return 9;
#if 0         /* Disabled to give the compiler a hint about low probability */
  case 0x3AU: /* ':' */
  case 0x3BU: /* ';' */
  case 0x3CU: /* '<' */
  case 0x3DU: /* '=' */
  case 0x3EU: /* '>' */
  case 0x3FU: /* '?' */
  case 0x40U: /* '@' */
    return -1;
#endif
  case 0x41U: /* 'A' */
    return 0xAU;
  case 0x42U: /* 'B' */
    return 0xBU;
  case 0x43U: /* 'C' */
    return 0xCU;
  case 0x44U: /* 'D' */
    return 0xDU;
  case 0x45U: /* 'E' */
    return 0xEU;
  case 0x46U: /* 'F' */
    return 0xFU;
#if 0         /* Disabled to give the compiler a hint about low probability */
  case 0x47U: /* 'G' */
  case 0x48U: /* 'H' */
  case 0x49U: /* 'I' */
  case 0x4AU: /* 'J' */
  case 0x4BU: /* 'K' */
  case 0x4CU: /* 'L' */
  case 0x4DU: /* 'M' */
  case 0x4EU: /* 'N' */
  case 0x4FU: /* 'O' */
  case 0x50U: /* 'P' */
  case 0x51U: /* 'Q' */
  case 0x52U: /* 'R' */
  case 0x53U: /* 'S' */
  case 0x54U: /* 'T' */
  case 0x55U: /* 'U' */
  case 0x56U: /* 'V' */
  case 0x57U: /* 'W' */
  case 0x58U: /* 'X' */
  case 0x59U: /* 'Y' */
  case 0x5AU: /* 'Z' */
  case 0x5BU: /* '[' */
  case 0x5CU: /* '\' */
  case 0x5DU: /* ']' */
  case 0x5EU: /* '^' */
  case 0x5FU: /* '_' */
  case 0x60U: /* '`' */
    return -1;
#endif
  case 0x61U: /* 'a' */
    return 0xAU;
  case 0x62U: /* 'b' */
    return 0xBU;
  case 0x63U: /* 'c' */
    return 0xCU;
  case 0x64U: /* 'd' */
    return 0xDU;
  case 0x65U: /* 'e' */
    return 0xEU;
  case 0x66U: /* 'f' */
    return 0xFU;
#if 0         /* Disabled to give the compiler a hint about low probability */
  case 0x67U: /* 'g' */
  case 0x68U: /* 'h' */
  case 0x69U: /* 'i' */
  case 0x6AU: /* 'j' */
  case 0x6BU: /* 'k' */
  case 0x6CU: /* 'l' */
  case 0x6DU: /* 'm' */
  case 0x6EU: /* 'n' */
  case 0x6FU: /* 'o' */
  case 0x70U: /* 'p' */
  case 0x71U: /* 'q' */
  case 0x72U: /* 'r' */
  case 0x73U: /* 's' */
  case 0x74U: /* 't' */
  case 0x75U: /* 'u' */
  case 0x76U: /* 'v' */
  case 0x77U: /* 'w' */
  case 0x78U: /* 'x' */
  case 0x79U: /* 'y' */
  case 0x7AU: /* 'z' */
  case 0x7BU: /* '{' */
  case 0x7CU: /* '|' */
  case 0x7DU: /* '}' */
  case 0x7EU: /* '~' */
  case 0x7FU: /* DEL */
  case 0x80U: /* EXT */
  case 0x81U: /* EXT */
  case 0x82U: /* EXT */
  case 0x83U: /* EXT */
  case 0x84U: /* EXT */
  case 0x85U: /* EXT */
  case 0x86U: /* EXT */
  case 0x87U: /* EXT */
  case 0x88U: /* EXT */
  case 0x89U: /* EXT */
  case 0x8AU: /* EXT */
  case 0x8BU: /* EXT */
  case 0x8CU: /* EXT */
  case 0x8DU: /* EXT */
  case 0x8EU: /* EXT */
  case 0x8FU: /* EXT */
  case 0x90U: /* EXT */
  case 0x91U: /* EXT */
  case 0x92U: /* EXT */
  case 0x93U: /* EXT */
  case 0x94U: /* EXT */
  case 0x95U: /* EXT */
  case 0x96U: /* EXT */
  case 0x97U: /* EXT */
  case 0x98U: /* EXT */
  case 0x99U: /* EXT */
  case 0x9AU: /* EXT */
  case 0x9BU: /* EXT */
  case 0x9CU: /* EXT */
  case 0x9DU: /* EXT */
  case 0x9EU: /* EXT */
  case 0x9FU: /* EXT */
  case 0xA0U: /* EXT */
  case 0xA1U: /* EXT */
  case 0xA2U: /* EXT */
  case 0xA3U: /* EXT */
  case 0xA4U: /* EXT */
  case 0xA5U: /* EXT */
  case 0xA6U: /* EXT */
  case 0xA7U: /* EXT */
  case 0xA8U: /* EXT */
  case 0xA9U: /* EXT */
  case 0xAAU: /* EXT */
  case 0xABU: /* EXT */
  case 0xACU: /* EXT */
  case 0xADU: /* EXT */
  case 0xAEU: /* EXT */
  case 0xAFU: /* EXT */
  case 0xB0U: /* EXT */
  case 0xB1U: /* EXT */
  case 0xB2U: /* EXT */
  case 0xB3U: /* EXT */
  case 0xB4U: /* EXT */
  case 0xB5U: /* EXT */
  case 0xB6U: /* EXT */
  case 0xB7U: /* EXT */
  case 0xB8U: /* EXT */
  case 0xB9U: /* EXT */
  case 0xBAU: /* EXT */
  case 0xBBU: /* EXT */
  case 0xBCU: /* EXT */
  case 0xBDU: /* EXT */
  case 0xBEU: /* EXT */
  case 0xBFU: /* EXT */
  case 0xC0U: /* EXT */
  case 0xC1U: /* EXT */
  case 0xC2U: /* EXT */
  case 0xC3U: /* EXT */
  case 0xC4U: /* EXT */
  case 0xC5U: /* EXT */
  case 0xC6U: /* EXT */
  case 0xC7U: /* EXT */
  case 0xC8U: /* EXT */
  case 0xC9U: /* EXT */
  case 0xCAU: /* EXT */
  case 0xCBU: /* EXT */
  case 0xCCU: /* EXT */
  case 0xCDU: /* EXT */
  case 0xCEU: /* EXT */
  case 0xCFU: /* EXT */
  case 0xD0U: /* EXT */
  case 0xD1U: /* EXT */
  case 0xD2U: /* EXT */
  case 0xD3U: /* EXT */
  case 0xD4U: /* EXT */
  case 0xD5U: /* EXT */
  case 0xD6U: /* EXT */
  case 0xD7U: /* EXT */
  case 0xD8U: /* EXT */
  case 0xD9U: /* EXT */
  case 0xDAU: /* EXT */
  case 0xDBU: /* EXT */
  case 0xDCU: /* EXT */
  case 0xDDU: /* EXT */
  case 0xDEU: /* EXT */
  case 0xDFU: /* EXT */
  case 0xE0U: /* EXT */
  case 0xE1U: /* EXT */
  case 0xE2U: /* EXT */
  case 0xE3U: /* EXT */
  case 0xE4U: /* EXT */
  case 0xE5U: /* EXT */
  case 0xE6U: /* EXT */
  case 0xE7U: /* EXT */
  case 0xE8U: /* EXT */
  case 0xE9U: /* EXT */
  case 0xEAU: /* EXT */
  case 0xEBU: /* EXT */
  case 0xECU: /* EXT */
  case 0xEDU: /* EXT */
  case 0xEEU: /* EXT */
  case 0xEFU: /* EXT */
  case 0xF0U: /* EXT */
  case 0xF1U: /* EXT */
  case 0xF2U: /* EXT */
  case 0xF3U: /* EXT */
  case 0xF4U: /* EXT */
  case 0xF5U: /* EXT */
  case 0xF6U: /* EXT */
  case 0xF7U: /* EXT */
  case 0xF8U: /* EXT */
  case 0xF9U: /* EXT */
  case 0xFAU: /* EXT */
  case 0xFBU: /* EXT */
  case 0xFCU: /* EXT */
  case 0xFDU: /* EXT */
  case 0xFEU: /* EXT */
  case 0xFFU: /* EXT */
    return -1;
  default:
    mhd_assert (0);
    break;  /* Should be unreachable */
#else
  default:
    break;
#endif
  }
  return -1;
#else  /* MHD_FAVOR_SMALL_CODE */
  if (isasciidigit (c))
    return (unsigned char) (c - '0');
  if ( (c >= 'A') && (c <= 'F') )
    return (unsigned char) (c - 'A' + 10);
  if ( (c >= 'a') && (c <= 'f') )
    return (unsigned char) (c - 'a' + 10);

  return -1;
#endif /* MHD_FAVOR_SMALL_CODE */
}


/**
 * Caseless compare two characters.
 *
 * @param c1 the first char to compare
 * @param c2 the second char to compare
 * @return boolean 'true' if chars are caseless equal, false otherwise
 */
_MHD_static_inline bool
charsequalcaseless (const char c1, const char c2)
{
  return ( (c1 == c2) ||
           (isasciiupper (c1) ?
            ((c1 - 'A' + 'a') == c2) :
            ((c1 == (c2 - 'A' + 'a')) && isasciiupper (c2))) );
}


#else  /* !INLINE_FUNC */


/**
 * Checks whether character is lower case letter in US-ASCII
 *
 * @param c character to check
 * @return boolean true if character is lower case letter,
 *         boolean false otherwise
 */
#define isasciilower(c) (((char) (c)) >= 'a' && ((char) (c)) <= 'z')


/**
 * Checks whether character is upper case letter in US-ASCII
 *
 * @param c character to check
 * @return boolean true if character is upper case letter,
 *         boolean false otherwise
 */
#define isasciiupper(c) (((char) (c)) >= 'A' && ((char) (c)) <= 'Z')


/**
 * Checks whether character is letter in US-ASCII
 *
 * @param c character to check
 * @return boolean true if character is letter, boolean false
 *         otherwise
 */
#define isasciialpha(c) (isasciilower (c) || isasciiupper (c))


/**
 * Check whether character is decimal digit in US-ASCII
 *
 * @param c character to check
 * @return boolean true if character is decimal digit, boolean false
 *         otherwise
 */
#define isasciidigit(c) (((char) (c)) >= '0' && ((char) (c)) <= '9')


/**
 * Check whether character is hexadecimal digit in US-ASCII
 *
 * @param c character to check
 * @return boolean true if character is hexadecimal digit,
 *         boolean false otherwise
 */
#define isasciixdigit(c) (isasciidigit ((c)) || \
                          (((char) (c)) >= 'A' && ((char) (c)) <= 'F') || \
                          (((char) (c)) >= 'a' && ((char) (c)) <= 'f') )


/**
 * Check whether character is decimal digit or letter in US-ASCII
 *
 * @param c character to check
 * @return boolean true if character is decimal digit or letter,
 *         boolean false otherwise
 */
#define isasciialnum(c) (isasciialpha (c) || isasciidigit (c))


/**
 * Convert US-ASCII character to lower case.
 * If character is upper case letter in US-ASCII than it's converted to lower
 * case analog. If character is NOT upper case letter than it's returned
 * unmodified.
 *
 * @param c character to convert
 * @return converted to lower case character
 */
#define toasciilower(c) ((isasciiupper (c)) ? (((char) (c)) - 'A' + 'a') : \
                         ((char) (c)))


/**
 * Convert US-ASCII character to upper case.
 * If character is lower case letter in US-ASCII than it's converted to upper
 * case analog. If character is NOT lower case letter than it's returned
 * unmodified.
 *
 * @param c character to convert
 * @return converted to upper case character
 */
#define toasciiupper(c) ((isasciilower (c)) ? (((char) (c)) - 'a' + 'A') : \
                         ((char) (c)))


/**
 * Convert US-ASCII decimal digit to its value.
 *
 * @param c character to convert
 * @return value of hexadecimal digit or -1 if @ c is not hexadecimal digit
 */
#define todigitvalue(c) (isasciidigit (c) ? (int) (((char) (c)) - '0') : \
                         (int) (-1))


/**
 * Convert US-ASCII hexadecimal digit to its value.
 * @param c character to convert
 * @return value of hexadecimal digit or -1 if @ c is not hexadecimal digit
 */
#define toxdigitvalue(c) (isasciidigit (c) ? (int) (((char) (c)) - '0') : \
                          ( (((char) (c)) >= 'A' && ((char) (c)) <= 'F') ? \
                            (int) (((unsigned char) (c)) - 'A' + 10) : \
                            ( (((char) (c)) >= 'a' && ((char) (c)) <= 'f') ? \
                              (int) (((unsigned char) (c)) - 'a' + 10) : \
                              (int) (-1) )))

/**
 * Caseless compare two characters.
 *
 * @param c1 the first char to compare
 * @param c2 the second char to compare
 * @return boolean 'true' if chars are caseless equal, false otherwise
 */
#define charsequalcaseless(c1, c2) \
  ( ((c1) == (c2)) || \
           (isasciiupper (c1) ? \
             (((c1) - 'A' + 'a') == (c2)) : \
             (((c1) == ((c2) - 'A' + 'a')) && isasciiupper (c2))) )

#endif /* !INLINE_FUNC */


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
                         const char *str2)
{
  while (0 != (*str1))
  {
    const char c1 = *str1;
    const char c2 = *str2;
    if (charsequalcaseless (c1, c2))
    {
      str1++;
      str2++;
    }
    else
      return 0;
  }
  return 0 == (*str2);
}


#endif /* ! MHD_FAVOR_SMALL_CODE */


/**
 * Check two string for equality, ignoring case of US-ASCII letters and
 * checking not more than @a maxlen characters.
 * Compares up to first terminating null character, but not more than
 * first @a maxlen characters.
 *
 * @param str1 first string to compare
 * @param str2 second string to compare
 * @param maxlen maximum number of characters to compare
 * @return non-zero if two strings are equal, zero otherwise.
 */
int
MHD_str_equal_caseless_n_ (const char *const str1,
                           const char *const str2,
                           size_t maxlen)
{
  size_t i;

  for (i = 0; i < maxlen; ++i)
  {
    const char c1 = str1[i];
    const char c2 = str2[i];
    if (0 == c2)
      return 0 == c1;
    if (charsequalcaseless (c1, c2))
      continue;
    else
      return 0;
  }
  return ! 0;
}


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
                               size_t len)
{
  size_t i;

  for (i = 0; i < len; ++i)
  {
    const char c1 = str1[i];
    const char c2 = str2[i];
    if (charsequalcaseless (c1, c2))
      continue;
    else
      return 0;
  }
  return ! 0;
}


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
                             size_t token_len)
{
  if (0 == token_len)
    return false;

  while (0 != *str)
  {
    size_t i;
    /* Skip all whitespaces and empty tokens. */
    while (' ' == *str || '\t' == *str || ',' == *str)
      str++;

    /* Check for token match. */
    i = 0;
    while (1)
    {
      const char sc = *(str++);
      const char tc = token[i++];

      if (0 == sc)
        return false;
      if (! charsequalcaseless (sc, tc))
        break;
      if (i >= token_len)
      {
        /* Check whether substring match token fully or
         * has additional unmatched chars at tail. */
        while (' ' == *str || '\t' == *str)
          str++;
        /* End of (sub)string? */
        if ((0 == *str) || (',' == *str) )
          return true;
        /* Unmatched chars at end of substring. */
        break;
      }
    }
    /* Find next substring. */
    while (0 != *str && ',' != *str)
      str++;
  }
  return false;
}


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
                                ssize_t *buf_size)
{
  const char *s1; /**< the "input" string / character */
  char *s2;       /**< the "output" string / character */
  size_t t_pos;   /**< position of matched character in the token */
  bool token_removed;

  mhd_assert (NULL == memchr (token, 0, token_len));
  mhd_assert (NULL == memchr (token, ' ', token_len));
  mhd_assert (NULL == memchr (token, '\t', token_len));
  mhd_assert (NULL == memchr (token, ',', token_len));
  mhd_assert (0 <= *buf_size);

  if (SSIZE_MAX <= ((str_len / 2) * 3 + 3))
  {
    /* The return value may overflow, refuse */
    *buf_size = (ssize_t) -1;
    return false;
  }
  s1 = str;
  s2 = buf;
  token_removed = false;

  while ((size_t) (s1 - str) < str_len)
  {
    const char *cur_token; /**< the first char of current token */
    size_t copy_size;

    /* Skip any initial whitespaces and empty tokens */
    while ( ((size_t) (s1 - str) < str_len) &&
            ((' ' == *s1) || ('\t' == *s1) || (',' == *s1)) )
      s1++;

    /* 's1' points to the first char of token in the input string or
     * points just beyond the end of the input string */

    if ((size_t) (s1 - str) >= str_len)
      break; /* Nothing to copy, end of the input string */

    /* 's1' points to the first char of token in the input string */

    cur_token = s1; /* the first char of input token */

    /* Check the token with case-insensetive match */
    t_pos = 0;
    while ( ((size_t) (s1 - str) < str_len) && (token_len > t_pos) &&
            (charsequalcaseless (*s1, token[t_pos])) )
    {
      s1++;
      t_pos++;
    }
    /* s1 may point just beyond the end of the input string */
    if ( (token_len == t_pos) && (0 != token_len) )
    {
      /* 'token' matched, check that current input token does not have
       * any suffixes */
      while ( ((size_t) (s1 - str) < str_len) &&
              ((' ' == *s1) || ('\t' == *s1)) )
        s1++;
      /* 's1' points to the first non-whitespace char after the token matched
       * requested token or points just beyond the end of the input string after
       * the requested token */
      if (((size_t) (s1 - str) == str_len) || (',' == *s1))
      {/* full token match, do not copy current token to the output */
        token_removed = true;
        continue;
      }
    }

    /* 's1' points to first non-whitespace char, to some char after
     * first non-whitespace char in the token in the input string, to
     * the ',', or just beyond the end of the input string */
    /* The current token in the input string does not match the token
     * to exclude, it must be copied to the output string */
    /* the current token size excluding leading whitespaces and current char */
    copy_size = (size_t) (s1 - cur_token);
    if (buf == s2)
    { /* The first token to copy to the output */
      if ((size_t) *buf_size < copy_size)
      { /* Not enough space in the output buffer */
        *buf_size = (ssize_t) -1;
        return false;
      }
    }
    else
    { /* Some token was already copied to the output buffer */
      mhd_assert (s2 > buf);
      if ((size_t) *buf_size < ((size_t) (s2 - buf)) + copy_size + 2)
      { /* Not enough space in the output buffer */
        *buf_size = (ssize_t) -1;
        return false;
      }
      *(s2++) = ',';
      *(s2++) = ' ';
    }
    /* Copy non-matched token to the output */
    if (0 != copy_size)
    {
      memcpy (s2, cur_token, copy_size);
      s2 += copy_size;
    }

    while ( ((size_t) (s1 - str) < str_len) && (',' != *s1))
    {
      /* 's1' points to first non-whitespace char, to some char after
       * first non-whitespace char in the token in the input string */
      /* Copy all non-whitespace chars from the current token in
       * the input string */
      while ( ((size_t) (s1 - str) < str_len) &&
              (',' != *s1) && (' ' != *s1) && ('\t' != *s1) )
      {
        mhd_assert (s2 >= buf);
        if ((size_t) *buf_size <= (size_t) (s2 - buf)) /* '<= s2' equals '< s2 + 1' */
        { /* Not enough space in the output buffer */
          *buf_size = (ssize_t) -1;
          return false;
        }
        *(s2++) = *(s1++);
      }
      /* 's1' points to some whitespace char in the token in the input
       * string, to the ',', or just beyond the end of the input string */
      /* Skip all whitespaces */
      while ( ((size_t) (s1 - str) < str_len) &&
              ((' ' == *s1) || ('\t' == *s1)) )
        s1++;

      /* 's1' points to the first non-whitespace char in the input string
       * after whitespace chars, to the ',', or just beyond the end of
       * the input string */
      if (((size_t) (s1 - str) < str_len) && (',' != *s1))
      { /* Not the end of the current token */
        mhd_assert (s2 >= buf);
        if ((size_t) *buf_size <= (size_t) (s2 - buf)) /* '<= s2' equals '< s2 + 1' */
        { /* Not enough space in the output buffer */
          *buf_size = (ssize_t) -1;
          return false;
        }
        *(s2++) = ' ';
      }
    }
  }
  mhd_assert (((ssize_t) (s2 - buf)) <= *buf_size);
  *buf_size = (ssize_t) (s2 - buf);
  return token_removed;
}


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
                                 const size_t tokens_len)
{
  const char *const t = tokens;   /**< a short alias for @a tokens */
  size_t pt;                      /**< position in @a tokens */
  bool token_removed;

  mhd_assert (NULL == memchr (tokens, 0, tokens_len));

  token_removed = false;
  pt = 0;

  while (pt < tokens_len && *str_len != 0)
  {
    const char *tkn; /**< the current token */
    size_t tkn_len;

    /* Skip any initial whitespaces and empty tokens in 'tokens' */
    while ( (pt < tokens_len) &&
            ((' ' == t[pt]) || ('\t' == t[pt]) || (',' == t[pt])) )
      pt++;

    if (pt >= tokens_len)
      break; /* No more tokens, nothing to remove */

    /* Found non-whitespace char which is not a comma */
    tkn = t + pt;
    do
    {
      do
      {
        pt++;
      } while (pt < tokens_len &&
               (' ' != t[pt] && '\t' != t[pt] && ',' != t[pt]));
      /* Found end of the token string, space, tab, or comma */
      tkn_len = pt - (size_t) (tkn - t);

      /* Skip all spaces and tabs */
      while (pt < tokens_len && (' ' == t[pt] || '\t' == t[pt]))
        pt++;
      /* Found end of the token string or non-whitespace char */
    } while (pt < tokens_len && ',' != t[pt]);

    /* 'tkn' is the input token with 'tkn_len' chars */
    mhd_assert (0 != tkn_len);

    if (*str_len == tkn_len)
    {
      if (MHD_str_equal_caseless_bin_n_ (str, tkn, tkn_len))
      {
        *str_len = 0;
        token_removed = true;
      }
      continue;
    }
    /* 'tkn' cannot match part of 'str' if length of 'tkn' is larger
     * than length of 'str'.
     * It's know that 'tkn' is not equal to the 'str' (was checked previously).
     * As 'str' is normalized when 'tkn' is not equal to the 'str'
     * it is required that 'str' to be at least 3 chars larger then 'tkn'
     * (the comma, the space and at least one additional character for the next
     * token) to remove 'tkn' from the 'str'. */
    if (*str_len > tkn_len + 2)
    { /* Remove 'tkn' from the input string */
      size_t pr;    /**< the 'read' position in the @a str */
      size_t pw;    /**< the 'write' position in the @a str */

      pr = 0;
      pw = 0;

      do
      {
        mhd_assert (pr >= pw);
        mhd_assert ((*str_len) >= (pr + tkn_len));
        if ( ( ((*str_len) == (pr + tkn_len)) || (',' == str[pr + tkn_len]) ) &&
             MHD_str_equal_caseless_bin_n_ (str + pr, tkn, tkn_len) )
        {
          /* current token in the input string matches the 'tkn', skip it */
          mhd_assert ((*str_len == pr + tkn_len) || \
                      (' ' == str[pr + tkn_len + 1])); /* 'str' must be normalized */
          token_removed = true;
          /* Advance to the next token in the input string or beyond
           * the end of the input string. */
          pr += tkn_len + 2;
        }
        else
        {
          /* current token in the input string does not match the 'tkn',
           * copy to the output */
          if (0 != pw)
          { /* not the first output token, add ", " to separate */
            if (pr != pw + 2)
            {
              str[pw++] = ',';
              str[pw++] = ' ';
            }
            else
              pw += 2; /* 'str' is not yet modified in this round */
          }
          do
          {
            if (pr != pw)
              str[pw] = str[pr];
            pr++;
            pw++;
          } while (pr < *str_len && ',' != str[pr]);
          /* Advance to the next token in the input string or beyond
           * the end of the input string. */
          pr += 2;
        }
        /* 'pr' should point to the next token in the input string or beyond
         * the end of the input string */
        if ((*str_len) < (pr + tkn_len))
        { /* The rest of the 'str + pr' is too small to match 'tkn' */
          if ((*str_len) > pr)
          { /* Copy the rest of the string */
            size_t copy_size;
            copy_size = *str_len - pr;
            if (0 != pw)
            { /* not the first output token, add ", " to separate */
              if (pr != pw + 2)
              {
                str[pw++] = ',';
                str[pw++] = ' ';
              }
              else
                pw += 2; /* 'str' is not yet modified in this round */
            }
            if (pr != pw)
              memmove (str + pw, str + pr, copy_size);
            pw += copy_size;
          }
          *str_len = pw;
          break;
        }
        mhd_assert ((' ' != str[0]) && ('\t' != str[0]));
        mhd_assert ((0 == pr) || (3 <= pr));
        mhd_assert ((0 == pr) || (' ' == str[pr - 1]));
        mhd_assert ((0 == pr) || (',' == str[pr - 2]));
      } while (1);
    }
  }

  return token_removed;
}


#ifndef MHD_FAVOR_SMALL_CODE
/* Use individual function for each case */

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
                    uint64_t *out_val)
{
  const char *const start = str;
  uint64_t res;

  if (! str || ! out_val || ! isasciidigit (str[0]))
    return 0;

  res = 0;
  do
  {
    const int digit = (unsigned char) (*str) - '0';
    if ( (res > (UINT64_MAX / 10)) ||
         ( (res == (UINT64_MAX / 10)) &&
           ((uint64_t) digit > (UINT64_MAX % 10)) ) )
      return 0;

    res *= 10;
    res += (unsigned int) digit;
    str++;
  } while (isasciidigit (*str));

  *out_val = res;
  return (size_t) (str - start);
}


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
                      uint64_t *out_val)
{
  uint64_t res;
  size_t i;

  if (! str || ! maxlen || ! out_val || ! isasciidigit (str[0]))
    return 0;

  res = 0;
  i = 0;
  do
  {
    const int digit = (unsigned char) str[i] - '0';

    if ( (res > (UINT64_MAX / 10)) ||
         ( (res == (UINT64_MAX / 10)) &&
           ((uint64_t) digit > (UINT64_MAX % 10)) ) )
      return 0;

    res *= 10;
    res += (unsigned int) digit;
    i++;
  } while ( (i < maxlen) &&
            isasciidigit (str[i]) );

  *out_val = res;
  return i;
}


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
                     uint32_t *out_val)
{
  const char *const start = str;
  uint32_t res;
  int digit;

  if (! str || ! out_val)
    return 0;

  res = 0;
  digit = toxdigitvalue (*str);
  while (digit >= 0)
  {
    if ( (res < (UINT32_MAX / 16)) ||
         ((res == (UINT32_MAX / 16)) &&
          ( (uint32_t) digit <= (UINT32_MAX % 16)) ) )
    {
      res *= 16;
      res += (unsigned int) digit;
    }
    else
      return 0;
    str++;
    digit = toxdigitvalue (*str);
  }

  if (str - start > 0)
    *out_val = res;
  return (size_t) (str - start);
}


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
                       uint32_t *out_val)
{
  size_t i;
  uint32_t res;
  int digit;
  if (! str || ! out_val)
    return 0;

  res = 0;
  i = 0;
  while (i < maxlen && (digit = toxdigitvalue (str[i])) >= 0)
  {
    if ( (res > (UINT32_MAX / 16)) ||
         ((res == (UINT32_MAX / 16)) &&
          ( (uint32_t) digit > (UINT32_MAX % 16)) ) )
      return 0;

    res *= 16;
    res += (unsigned int) digit;
    i++;
  }

  if (i)
    *out_val = res;
  return i;
}


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
                     uint64_t *out_val)
{
  const char *const start = str;
  uint64_t res;
  int digit;
  if (! str || ! out_val)
    return 0;

  res = 0;
  digit = toxdigitvalue (*str);
  while (digit >= 0)
  {
    if ( (res < (UINT64_MAX / 16)) ||
         ((res == (UINT64_MAX / 16)) &&
          ( (uint64_t) digit <= (UINT64_MAX % 16)) ) )
    {
      res *= 16;
      res += (unsigned int) digit;
    }
    else
      return 0;
    str++;
    digit = toxdigitvalue (*str);
  }

  if (str - start > 0)
    *out_val = res;
  return (size_t) (str - start);
}


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
                       uint64_t *out_val)
{
  size_t i;
  uint64_t res;
  int digit;
  if (! str || ! out_val)
    return 0;

  res = 0;
  i = 0;
  while (i < maxlen && (digit = toxdigitvalue (str[i])) >= 0)
  {
    if ( (res > (UINT64_MAX / 16)) ||
         ((res == (UINT64_MAX / 16)) &&
          ( (uint64_t) digit > (UINT64_MAX % 16)) ) )
      return 0;

    res *= 16;
    res += (unsigned int) digit;
    i++;
  }

  if (i)
    *out_val = res;
  return i;
}


#else  /* MHD_FAVOR_SMALL_CODE */

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
                      unsigned int base)
{
  size_t i;
  uint64_t res;
  const uint64_t max_v_div_b = max_val / base;
  const uint64_t max_v_mod_b = max_val % base;

  if (! str || ! out_val ||
      ((base != 16) && (base != 10)) )
    return 0;

  res = 0;
  i = 0;
  while (maxlen > i)
  {
    const int digit = (base == 16) ?
                      toxdigitvalue (str[i]) : todigitvalue (str[i]);

    if (0 > digit)
      break;
    if ( ((max_v_div_b) < res) ||
         (( (max_v_div_b) == res) && ( (max_v_mod_b) < (uint64_t) digit) ) )
      return 0;

    res *= base;
    res += (unsigned int) digit;
    i++;
  }

  if (i)
  {
    if (8 == val_size)
      *(uint64_t *) out_val = res;
    else if (4 == val_size)
      *(uint32_t *) out_val = (uint32_t) res;
    else
      return 0;
  }
  return i;
}


#endif /* MHD_FAVOR_SMALL_CODE */


size_t
MHD_uint32_to_strx (uint32_t val,
                    char *buf,
                    size_t buf_size)
{
  size_t o_pos = 0; /**< position of the output character */
  int digit_pos = 8; /** zero-based, digit position in @a 'val' */
  int digit;

  /* Skip leading zeros */
  do
  {
    digit_pos--;
    digit = (int) (val >> 28);
    val <<= 4;
  } while ((0 == digit) && (0 != digit_pos));

  while (o_pos < buf_size)
  {
    buf[o_pos++] =
      (char) ((digit <= 9) ?
              ('0' + (char) digit) :
              ('A' + (char) digit - 10));
    if (0 == digit_pos)
      return o_pos;
    digit_pos--;
    digit = (int) (val >> 28);
    val <<= 4;
  }
  return 0; /* The buffer is too small */
}


#ifndef MHD_FAVOR_SMALL_CODE
size_t
MHD_uint16_to_str (uint16_t val,
                   char *buf,
                   size_t buf_size)
{
  char *chr;  /**< pointer to the current printed digit */
  /* The biggest printable number is 65535 */
  uint16_t divisor = UINT16_C (10000);
  int digit;

  chr = buf;
  digit = (int) (val / divisor);
  mhd_assert (digit < 10);

  /* Do not print leading zeros */
  while ((0 == digit) && (1 < divisor))
  {
    divisor /= 10;
    digit = (int) (val / divisor);
    mhd_assert (digit < 10);
  }

  while (0 != buf_size)
  {
    *chr = (char) ((char) digit + '0');
    chr++;
    buf_size--;
    if (1 == divisor)
      return (size_t) (chr - buf);
    val = (uint16_t) (val % divisor);
    divisor /= 10;
    digit = (int) (val / divisor);
    mhd_assert (digit < 10);
  }
  return 0; /* The buffer is too small */
}


#endif /* !MHD_FAVOR_SMALL_CODE */


size_t
MHD_uint64_to_str (uint64_t val,
                   char *buf,
                   size_t buf_size)
{
  char *chr;  /**< pointer to the current printed digit */
  /* The biggest printable number is 18446744073709551615 */
  uint64_t divisor = UINT64_C (10000000000000000000);
  int digit;

  chr = buf;
  digit = (int) (val / divisor);
  mhd_assert (digit < 10);

  /* Do not print leading zeros */
  while ((0 == digit) && (1 < divisor))
  {
    divisor /= 10;
    digit = (int) (val / divisor);
    mhd_assert (digit < 10);
  }

  while (0 != buf_size)
  {
    *chr = (char) ((char) digit + '0');
    chr++;
    buf_size--;
    if (1 == divisor)
      return (size_t) (chr - buf);
    val %= divisor;
    divisor /= 10;
    digit = (int) (val / divisor);
    mhd_assert (digit < 10);
  }
  return 0; /* The buffer is too small */
}


size_t
MHD_uint8_to_str_pad (uint8_t val,
                      uint8_t min_digits,
                      char *buf,
                      size_t buf_size)
{
  size_t pos; /**< the position of the current printed digit */
  int digit;
  mhd_assert (3 >= min_digits);
  if (0 == buf_size)
    return 0;

  pos = 0;
  digit = val / 100;
  if (0 == digit)
  {
    if (3 <= min_digits)
      buf[pos++] = '0';
  }
  else
  {
    buf[pos++] = (char) ('0' + (char) digit);
    val %= 100;
    min_digits = 2;
  }

  if (buf_size <= pos)
    return 0;
  digit = val / 10;
  if (0 == digit)
  {
    if (2 <= min_digits)
      buf[pos++] = '0';
  }
  else
  {
    buf[pos++] = (char) ('0' + (char) digit);
    val %= 10;
  }

  if (buf_size <= pos)
    return 0;
  buf[pos++] = (char) ('0' + (char) val);
  return pos;
}


size_t
MHD_bin_to_hex (const void *bin,
                size_t size,
                char *hex)
{
  size_t i;

  for (i = 0; i < size; ++i)
  {
    uint8_t j;
    const uint8_t b = ((const uint8_t *) bin)[i];
    j = b >> 4;
    hex[i * 2] = (char) ((j < 10) ? (j + '0') : (j - 10 + 'a'));
    j = b & 0x0f;
    hex[i * 2 + 1] = (char) ((j < 10) ? (j + '0') : (j - 10 + 'a'));
  }
  return i * 2;
}


size_t
MHD_bin_to_hex_z (const void *bin,
                  size_t size,
                  char *hex)
{
  size_t res;

  res = MHD_bin_to_hex (bin, size, hex);
  hex[res] = 0;

  return res;
}


size_t
MHD_hex_to_bin (const char *hex,
                size_t len,
                void *bin)
{
  uint8_t *const out = (uint8_t *) bin;
  size_t r;
  size_t w;

  if (0 == len)
    return 0;
  r = 0;
  w = 0;
  if (0 != len % 2)
  {
    /* Assume the first byte is encoded with single digit */
    const int l = toxdigitvalue (hex[r++]);
    if (0 > l)
      return 0;
    out[w++] = (uint8_t) ((unsigned int) l);
  }
  while (r < len)
  {
    const int h = toxdigitvalue (hex[r++]);
    const int l = toxdigitvalue (hex[r++]);
    if ((0 > h) || (0 > l))
      return 0;
    out[w++] = (uint8_t) ( ((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                           | ((uint8_t) ((unsigned int) l)) );
  }
  mhd_assert (len == r);
  mhd_assert ((len + 1) / 2 == w);
  return w;
}


size_t
MHD_str_pct_decode_strict_n_ (const char *pct_encoded,
                              size_t pct_encoded_len,
                              char *decoded,
                              size_t buf_size)
{
#ifdef MHD_FAVOR_SMALL_CODE
  bool broken;
  size_t res;

  res = MHD_str_pct_decode_lenient_n_ (pct_encoded, pct_encoded_len, decoded,
                                       buf_size, &broken);
  if (broken)
    return 0;
  return res;
#else  /* ! MHD_FAVOR_SMALL_CODE */
  size_t r;
  size_t w;
  r = 0;
  w = 0;

  if (buf_size >= pct_encoded_len)
  {
    while (r < pct_encoded_len)
    {
      const char chr = pct_encoded[r];
      if ('%' == chr)
      {
        if (2 > pct_encoded_len - r)
          return 0;
        else
        {
          const int h = toxdigitvalue (pct_encoded[++r]);
          const int l = toxdigitvalue (pct_encoded[++r]);
          unsigned char out;
          if ((0 > h) || (0 > l))
            return 0;
          out =
            (unsigned char) (((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                             | ((uint8_t) ((unsigned int) l)));
          decoded[w] = (char) out;
        }
      }
      else
        decoded[w] = chr;
      ++r;
      ++w;
    }
    return w;
  }

  while (r < pct_encoded_len)
  {
    const char chr = pct_encoded[r];
    if (w >= buf_size)
      return 0;
    if ('%' == chr)
    {
      if (2 > pct_encoded_len - r)
        return 0;
      else
      {
        const int h = toxdigitvalue (pct_encoded[++r]);
        const int l = toxdigitvalue (pct_encoded[++r]);
        unsigned char out;
        if ((0 > h) || (0 > l))
          return 0;
        out =
          (unsigned char) (((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                           | ((uint8_t) ((unsigned int) l)));
        decoded[w] = (char) out;
      }
    }
    else
      decoded[w] = chr;
    ++r;
    ++w;
  }
  return w;
#endif /* ! MHD_FAVOR_SMALL_CODE */
}


size_t
MHD_str_pct_decode_lenient_n_ (const char *pct_encoded,
                               size_t pct_encoded_len,
                               char *decoded,
                               size_t buf_size,
                               bool *broken_encoding)
{
  size_t r;
  size_t w;
  r = 0;
  w = 0;
  if (NULL != broken_encoding)
    *broken_encoding = false;
#ifndef MHD_FAVOR_SMALL_CODE
  if (buf_size >= pct_encoded_len)
  {
    while (r < pct_encoded_len)
    {
      const char chr = pct_encoded[r];
      if ('%' == chr)
      {
        if (2 > pct_encoded_len - r)
        {
          if (NULL != broken_encoding)
            *broken_encoding = true;
          decoded[w] = chr; /* Copy "as is" */
        }
        else
        {
          const int h = toxdigitvalue (pct_encoded[++r]);
          const int l = toxdigitvalue (pct_encoded[++r]);
          unsigned char out;
          if ((0 > h) || (0 > l))
          {
            r -= 2;
            if (NULL != broken_encoding)
              *broken_encoding = true;
            decoded[w] = chr; /* Copy "as is" */
          }
          else
          {
            out =
              (unsigned char) (((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                               | ((uint8_t) ((unsigned int) l)));
            decoded[w] = (char) out;
          }
        }
      }
      else
        decoded[w] = chr;
      ++r;
      ++w;
    }
    return w;
  }
#endif /* ! MHD_FAVOR_SMALL_CODE */
  while (r < pct_encoded_len)
  {
    const char chr = pct_encoded[r];
    if (w >= buf_size)
      return 0;
    if ('%' == chr)
    {
      if (2 > pct_encoded_len - r)
      {
        if (NULL != broken_encoding)
          *broken_encoding = true;
        decoded[w] = chr; /* Copy "as is" */
      }
      else
      {
        const int h = toxdigitvalue (pct_encoded[++r]);
        const int l = toxdigitvalue (pct_encoded[++r]);
        if ((0 > h) || (0 > l))
        {
          r -= 2;
          if (NULL != broken_encoding)
            *broken_encoding = true;
          decoded[w] = chr; /* Copy "as is" */
        }
        else
        {
          unsigned char out;
          out =
            (unsigned char) (((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                             | ((uint8_t) ((unsigned int) l)));
          decoded[w] = (char) out;
        }
      }
    }
    else
      decoded[w] = chr;
    ++r;
    ++w;
  }
  return w;
}


size_t
MHD_str_pct_decode_in_place_strict_ (char *str)
{
#ifdef MHD_FAVOR_SMALL_CODE
  size_t res;
  bool broken;

  res = MHD_str_pct_decode_in_place_lenient_ (str, &broken);
  if (broken)
  {
    res = 0;
    str[0] = 0;
  }
  return res;
#else  /* ! MHD_FAVOR_SMALL_CODE */
  size_t r;
  size_t w;
  r = 0;
  w = 0;

  while (0 != str[r])
  {
    const char chr = str[r++];
    if ('%' == chr)
    {
      const char d1 = str[r++];
      if (0 == d1)
        return 0;
      else
      {
        const char d2 = str[r++];
        if (0 == d2)
          return 0;
        else
        {
          const int h = toxdigitvalue (d1);
          const int l = toxdigitvalue (d2);
          unsigned char out;
          if ((0 > h) || (0 > l))
            return 0;
          out =
            (unsigned char) (((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                             | ((uint8_t) ((unsigned int) l)));
          str[w++] = (char) out;
        }
      }
    }
    else
      str[w++] = chr;
  }
  str[w] = 0;
  return w;
#endif /* ! MHD_FAVOR_SMALL_CODE */
}


size_t
MHD_str_pct_decode_in_place_lenient_ (char *str,
                                      bool *broken_encoding)
{
#ifdef MHD_FAVOR_SMALL_CODE
  size_t len;
  size_t res;

  len = strlen (str);
  res = MHD_str_pct_decode_lenient_n_ (str, len, str, len, broken_encoding);
  str[res] = 0;

  return res;
#else  /* ! MHD_FAVOR_SMALL_CODE */
  size_t r;
  size_t w;
  if (NULL != broken_encoding)
    *broken_encoding = false;
  r = 0;
  w = 0;
  while (0 != str[r])
  {
    const char chr = str[r++];
    if ('%' == chr)
    {
      const char d1 = str[r++];
      if (0 == d1)
      {
        if (NULL != broken_encoding)
          *broken_encoding = true;
        str[w++] = chr; /* Copy "as is" */
        str[w] = 0;
        return w;
      }
      else
      {
        const char d2 = str[r++];
        if (0 == d2)
        {
          if (NULL != broken_encoding)
            *broken_encoding = true;
          str[w++] = chr; /* Copy "as is" */
          str[w++] = d1; /* Copy "as is" */
          str[w] = 0;
          return w;
        }
        else
        {
          const int h = toxdigitvalue (d1);
          const int l = toxdigitvalue (d2);
          unsigned char out;
          if ((0 > h) || (0 > l))
          {
            if (NULL != broken_encoding)
              *broken_encoding = true;
            str[w++] = chr; /* Copy "as is" */
            str[w++] = d1;
            str[w++] = d2;
            continue;
          }
          out =
            (unsigned char) (((uint8_t) (((uint8_t) ((unsigned int) h)) << 4))
                             | ((uint8_t) ((unsigned int) l)));
          str[w++] = (char) out;
          continue;
        }
      }
    }
    str[w++] = chr;
  }
  str[w] = 0;
  return w;
#endif /* ! MHD_FAVOR_SMALL_CODE */
}


#ifdef DAUTH_SUPPORT
bool
MHD_str_equal_quoted_bin_n (const char *quoted,
                            size_t quoted_len,
                            const char *unquoted,
                            size_t unquoted_len)
{
  size_t i;
  size_t j;
  if (unquoted_len < quoted_len / 2)
    return false;

  j = 0;
  for (i = 0; quoted_len > i && unquoted_len > j; ++i, ++j)
  {
    if ('\\' == quoted[i])
    {
      i++; /* Advance to the next character */
      if (quoted_len == i)
        return false; /* No character after escaping backslash */
    }
    if (quoted[i] != unquoted[j])
      return false; /* Different characters */
  }
  if ((quoted_len != i) || (unquoted_len != j))
    return false; /* The strings have different length */

  return true;
}


bool
MHD_str_equal_caseless_quoted_bin_n (const char *quoted,
                                     size_t quoted_len,
                                     const char *unquoted,
                                     size_t unquoted_len)
{
  size_t i;
  size_t j;
  if (unquoted_len < quoted_len / 2)
    return false;

  j = 0;
  for (i = 0; quoted_len > i && unquoted_len > j; ++i, ++j)
  {
    if ('\\' == quoted[i])
    {
      i++; /* Advance to the next character */
      if (quoted_len == i)
        return false; /* No character after escaping backslash */
    }
    if (! charsequalcaseless (quoted[i], unquoted[j]))
      return false; /* Different characters */
  }
  if ((quoted_len != i) || (unquoted_len != j))
    return false; /* The strings have different length */

  return true;
}


size_t
MHD_str_unquote (const char *quoted,
                 size_t quoted_len,
                 char *result)
{
  size_t r;
  size_t w;

  r = 0;
  w = 0;

  while (quoted_len > r)
  {
    if ('\\' == quoted[r])
    {
      ++r;
      if (quoted_len == r)
        return 0; /* Last backslash is not followed by char to unescape */
    }
    result[w++] = quoted[r++];
  }
  return w;
}


#endif /* DAUTH_SUPPORT */

#if defined(DAUTH_SUPPORT) || defined(BAUTH_SUPPORT)

size_t
MHD_str_quote (const char *unquoted,
               size_t unquoted_len,
               char *result,
               size_t buf_size)
{
  size_t r;
  size_t w;

  r = 0;
  w = 0;

#ifndef MHD_FAVOR_SMALL_CODE
  if (unquoted_len * 2 <= buf_size)
  {
    /* Fast loop: the output will fit the buffer with any input string content */
    while (unquoted_len > r)
    {
      const char chr = unquoted[r++];
      if (('\\' == chr) || ('\"' == chr))
        result[w++] = '\\'; /* Escape current char */
      result[w++] = chr;
    }
  }
  else
  {
    if (unquoted_len > buf_size)
      return 0; /* Quick fail: the output buffer is too small */
#else  /* MHD_FAVOR_SMALL_CODE */
  if (1)
  {
#endif /* MHD_FAVOR_SMALL_CODE */

    while (unquoted_len > r)
    {
      if (buf_size <= w)
        return 0; /* The output buffer is too small */
      else
      {
        const char chr = unquoted[r++];
        if (('\\' == chr) || ('\"' == chr))
        {
          result[w++] = '\\'; /* Escape current char */
          if (buf_size <= w)
            return 0; /* The output buffer is too small */
        }
        result[w++] = chr;
      }
    }
  }

  mhd_assert (w >= r);
  mhd_assert (w <= r * 2);
  return w;
}


#endif /* DAUTH_SUPPORT || BAUTH_SUPPORT */

#ifdef BAUTH_SUPPORT

/*
 * MHD_BASE64_FUNC_VERSION
 * 1 = smallest,
 * 2 = medium,
 * 3 = fastest
 */
#ifndef MHD_BASE64_FUNC_VERSION
#ifdef MHD_FAVOR_SMALL_CODE
#define MHD_BASE64_FUNC_VERSION 1
#else  /* ! MHD_FAVOR_SMALL_CODE */
#define MHD_BASE64_FUNC_VERSION 3
#endif /* ! MHD_FAVOR_SMALL_CODE */
#endif /* ! MHD_BASE64_FUNC_VERSION */

#if MHD_BASE64_FUNC_VERSION < 1 || MHD_BASE64_FUNC_VERSION > 3
#error Wrong MHD_BASE64_FUNC_VERSION value
#endif /* MHD_BASE64_FUNC_VERSION < 1 || MHD_BASE64_FUNC_VERSION > 3 */

#if MHD_BASE64_FUNC_VERSION == 3
#define MHD_base64_map_type_ int
#else  /* MHD_BASE64_FUNC_VERSION < 3 */
#define MHD_base64_map_type_ int8_t
#endif /* MHD_BASE64_FUNC_VERSION < 3 */

#if MHD_BASE64_FUNC_VERSION == 1
static MHD_base64_map_type_
base64_char_to_value_ (uint8_t c)
{
  if ('Z' >= c)
  {
    if ('A' <= c)
      return (MHD_base64_map_type_) ((c - 'A') + 0);
    if ('0' <= c)
    {
      if ('9' >= c)
        return (MHD_base64_map_type_) ((c - '0') + 52);
      if ('=' == c)
        return -2;
      return -1;
    }
    if ('+' == c)
      return 62;
    if ('/' == c)
      return 63;
    return -1;
  }
  if (('z' >= c) && ('a' <= c))
    return (MHD_base64_map_type_) ((c - 'a') + 26);
  return -1;
}


#endif /* MHD_BASE64_FUNC_VERSION == 1 */


MHD_DATA_TRUNCATION_RUNTIME_CHECK_DISABLE_


size_t
MHD_base64_to_bin_n (const char *base64,
                     size_t base64_len,
                     void *bin,
                     size_t bin_size)
{
#if MHD_BASE64_FUNC_VERSION >= 2
  static const MHD_base64_map_type_ map[] = {
    /* -1 = invalid char, -2 = padding
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    NUL,  SOH,  STX,  ETX,  EOT,  ENQ,  ACK,  BEL,  */
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /*
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    BS,   HT,   LF,   VT,   FF,   CR,   SO,   SI,   */
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /*
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    DLE,  DC1,  DC2,  DC3,  DC4,  NAK,  SYN,  ETB,  */
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /*
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    CAN,  EM,   SUB,  ESC,  FS,   GS,   RS,   US,   */
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /*
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    ' ',  '!',  '"',  '#',  '$',  '%',  '&',  '\'', */
    -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
    /*
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  */
    -1,   -1,   -1,   62,   -1,   -1,   -1,   63,
    /*
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  */
    52,   53,   54,   55,   56,   57,   58,   59,
    /*
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',  */
    60,   61,   -1,   -1,   -1,   -2,   -1,   -1,
    /*
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    '@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  */
    -1,    0,    1,    2,    3,    4,    5,    6,
    /*
    0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',  */
    7,     8,    9,   10,   11,   12,   13,   14,
    /*
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  */
    15,   16,   17,   18,   19,   20,   21,   22,
    /*
     0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    'X',  'Y',  'Z',  '[',  '\',  ']',  '^',  '_',  */
    23,   24,   25,   -1,   -1,   -1,   -1,   -1,
    /*
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  */
    -1,   26,   27,   28,   29,   30,   31,   32,
    /*
    0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',  */
    33,   34,   35,   36,   37,   38,   39,   40,
    /*
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  */
    41,   42,   43,   44,   45,   46,   47,   48,
    /*
    0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    'x',  'y',  'z',  '{',  '|',  '}',  '~',  DEL,  */
    49,   50,   51,   -1,   -1,   -1,   -1,   -1

#if MHD_BASE64_FUNC_VERSION == 3
    ,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 80..8F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 90..9F */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* A0..AF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* B0..BF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* C0..CF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* D0..DF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* E0..EF */
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* F0..FF */
#endif /* ! MHD_BASE64_FUNC_VERSION == 3 */
  };
#define base64_char_to_value_(c) map[(c)]
#endif /* MHD_BASE64_FUNC_VERSION >= 2 */
  const uint8_t *const in = (const uint8_t *) base64;
  uint8_t *const out = (uint8_t *) bin;
  size_t i;
  size_t j;
  if (0 == base64_len)
    return 0;  /* Nothing to decode */
  if (0 != base64_len % 4)
    return 0;  /* Wrong input length */
  if (base64_len / 4 * 3 - 2 > bin_size)
    return 0;

  j = 0;
  for (i = 0; i < (base64_len - 4); i += 4)
  {
#if MHD_BASE64_FUNC_VERSION == 2
    if (0 != (0x80 & (in[i] | in[i + 1] | in[i + 2] | in[i + 3])))
      return 0;
#endif /* MHD_BASE64_FUNC_VERSION == 2 */
    if (1)
    {
      const MHD_base64_map_type_ v1 = base64_char_to_value_ (in[i + 0]);
      const MHD_base64_map_type_ v2 = base64_char_to_value_ (in[i + 1]);
      const MHD_base64_map_type_ v3 = base64_char_to_value_ (in[i + 2]);
      const MHD_base64_map_type_ v4 = base64_char_to_value_ (in[i + 3]);
      if ((0 > v1) || (0 > v2) || (0 > v3) || (0 > v4))
        return 0;
      out[j + 0] = (uint8_t) (((uint8_t) (((uint8_t) v1) << 2))
                              | ((uint8_t) (((uint8_t) v2) >> 4)));
      out[j + 1] = (uint8_t) (((uint8_t) (((uint8_t) v2) << 4))
                              | ((uint8_t) (((uint8_t) v3) >> 2)));
      out[j + 2] = (uint8_t) (((uint8_t) (((uint8_t) v3) << 6))
                              | ((uint8_t) v4));
    }
    j += 3;
  }
#if MHD_BASE64_FUNC_VERSION == 2
  if (0 != (0x80 & (in[i] | in[i + 1] | in[i + 2] | in[i + 3])))
    return 0;
#endif /* MHD_BASE64_FUNC_VERSION == 2 */
  if (1)
  { /* The last four chars block */
    const MHD_base64_map_type_ v1 = base64_char_to_value_ (in[i + 0]);
    const MHD_base64_map_type_ v2 = base64_char_to_value_ (in[i + 1]);
    const MHD_base64_map_type_ v3 = base64_char_to_value_ (in[i + 2]);
    const MHD_base64_map_type_ v4 = base64_char_to_value_ (in[i + 3]);
    if ((0 > v1) || (0 > v2))
      return 0; /* Invalid char or padding at first two positions */
    mhd_assert (j < bin_size);
    out[j++] = (uint8_t) (((uint8_t) (((uint8_t) v1) << 2))
                          | ((uint8_t) (((uint8_t) v2) >> 4)));
    if (0 > v3)
    { /* Third char is either padding or invalid */
      if ((-2 != v3) || (-2 != v4))
        return 0;  /* Both two last chars must be padding */
      if (0 != (uint8_t) (((uint8_t) v2) << 4))
        return 0;  /* Wrong last char */
      return j;
    }
    if (j >= bin_size)
      return 0; /* Not enough space */
    out[j++] = (uint8_t) (((uint8_t) (((uint8_t) v2) << 4))
                          | ((uint8_t) (((uint8_t) v3) >> 2)));
    if (0 > v4)
    { /* Fourth char is either padding or invalid */
      if (-2 != v4)
        return 0;  /* The char must be padding */
      if (0 != (uint8_t) (((uint8_t) v3) << 6))
        return 0;  /* Wrong last char */
      return j;
    }
    if (j >= bin_size)
      return 0; /* Not enough space */
    out[j++] = (uint8_t) (((uint8_t) (((uint8_t) v3) << 6))
                          | ((uint8_t) v4));
  }
  return j;
#if MHD_BASE64_FUNC_VERSION >= 2
#undef base64_char_to_value_
#endif /* MHD_BASE64_FUNC_VERSION >= 2 */
}


MHD_DATA_TRUNCATION_RUNTIME_CHECK_RESTORE_


#undef MHD_base64_map_type_

#endif /* BAUTH_SUPPORT */
