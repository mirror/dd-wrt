/* C code produced by gperf version 3.0.3 */
/* Command-line: /usr/bin/gperf -m 10 -C -G -D -t  */
/* Computed positions: -k'2-3,6,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif


/*
 * Copyright (C) 2005 Andreas Steffen
 * Hochschule fuer Technik Rapperswil, Switzerland
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <string.h>

#include "keywords.h"

struct kw_entry {
    char *name;
    kw_token_t token;
};

#define TOTAL_KEYWORDS 136
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 17
#define MIN_HASH_VALUE 10
#define MAX_HASH_VALUE 259
/* maximum key range = 250, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned short asso_values[] =
    {
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260,   8,
       99, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260,   4, 260,  11,   4,  80,
       55,   6,   3,   2, 114,   2, 260, 114,  70,  33,
       22,  81,  51,   7,  14,   2,   7, 122,   8, 260,
      260,  43,   4, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260, 260, 260, 260, 260,
      260, 260, 260, 260, 260, 260
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
      case 4:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const struct kw_entry wordlist[] =
  {
    {"pfs",               KW_PFS_DEPRECATED},
    {"right",             KW_RIGHT},
    {"rightgroups",       KW_RIGHTGROUPS},
    {"left",              KW_LEFT},
    {"lifetime",          KW_KEYLIFE},
    {"aggressive",        KW_AGGRESSIVE},
    {"rightsubnet",       KW_RIGHTSUBNET},
    {"rightikeport",      KW_RIGHTIKEPORT},
    {"rightsendcert",     KW_RIGHTSENDCERT},
    {"lifepackets",       KW_LIFEPACKETS},
    {"leftcert",          KW_LEFTCERT},
    {"leftsendcert",      KW_LEFTSENDCERT},
    {"leftgroups",        KW_LEFTGROUPS},
    {"leftca",            KW_LEFTCA},
    {"keep_alive",        KW_SETUP_DEPRECATED},
    {"leftdns",           KW_LEFTDNS},
    {"uniqueids",         KW_UNIQUEIDS},
    {"leftprotoport",     KW_LEFTPROTOPORT},
    {"interfaces",        KW_SETUP_DEPRECATED},
    {"rightsubnetwithin", KW_RIGHTSUBNET},
    {"virtual_private",   KW_SETUP_DEPRECATED},
    {"certuribase",       KW_CERTURIBASE},
    {"mark_in",           KW_MARK_IN},
    {"lifebytes",         KW_LIFEBYTES},
    {"marginbytes",       KW_MARGINBYTES},
    {"marginpackets",     KW_MARGINPACKETS},
    {"margintime",        KW_REKEYMARGIN},
    {"keyingtries",       KW_KEYINGTRIES},
    {"keylife",           KW_KEYLIFE},
    {"fragmentation",     KW_FRAGMENTATION},
    {"leftrsasigkey",     KW_LEFTRSASIGKEY},
    {"rightid",           KW_RIGHTID},
    {"rightdns",          KW_RIGHTDNS},
    {"rightsourceip",     KW_RIGHTSOURCEIP},
    {"rightallowany",     KW_RIGHTALLOWANY},
    {"leftcertpolicy",    KW_LEFTCERTPOLICY},
    {"reqid",             KW_REQID},
    {"rightrsasigkey",    KW_RIGHTRSASIGKEY},
    {"rightprotoport",    KW_RIGHTPROTOPORT},
    {"leftnexthop",       KW_LEFT_DEPRECATED},
    {"me_peerid",         KW_ME_PEERID},
    {"strictcrlpolicy",   KW_STRICTCRLPOLICY},
    {"inactivity",        KW_INACTIVITY},
    {"rightnexthop",      KW_RIGHT_DEPRECATED},
    {"rightfirewall",     KW_RIGHTFIREWALL},
    {"ldapbase",          KW_CA_DEPRECATED},
    {"leftupdown",        KW_LEFTUPDOWN},
    {"leftfirewall",      KW_LEFTFIREWALL},
    {"crluri",            KW_CRLURI},
    {"mediation",         KW_MEDIATION},
    {"rightcert",         KW_RIGHTCERT},
    {"crluri1",           KW_CRLURI},
    {"rightca",           KW_RIGHTCA},
    {"mobike",	           KW_MOBIKE},
    {"type",              KW_TYPE},
    {"ocspuri",           KW_OCSPURI},
    {"lefthostaccess",    KW_LEFTHOSTACCESS},
    {"esp",               KW_ESP},
    {"cacert",            KW_CACERT},
    {"ocspuri1",          KW_OCSPURI},
    {"rightid2",          KW_RIGHTID2},
    {"forceencaps",       KW_FORCEENCAPS},
    {"nat_traversal",     KW_SETUP_DEPRECATED},
    {"eap",               KW_CONN_DEPRECATED},
    {"rightgroups2",      KW_RIGHTGROUPS2},
    {"packetdefault",     KW_SETUP_DEPRECATED},
    {"force_keepalive",   KW_SETUP_DEPRECATED},
    {"mark_out",          KW_MARK_OUT},
    {"mediated_by",       KW_MEDIATED_BY},
    {"leftcert2",         KW_LEFTCERT2},
    {"rightauth2",        KW_RIGHTAUTH2},
    {"leftid",            KW_LEFTID},
    {"leftca2",           KW_LEFTCA2},
    {"ike",               KW_IKE},
    {"compress",          KW_COMPRESS},
    {"aaa_identity",      KW_AAA_IDENTITY},
    {"leftgroups2",       KW_LEFTGROUPS2},
    {"leftallowany",      KW_LEFTALLOWANY},
    {"righthostaccess",   KW_RIGHTHOSTACCESS},
    {"rekeyfuzz",         KW_REKEYFUZZ},
    {"rightauth",         KW_RIGHTAUTH},
    {"klipsdebug",        KW_SETUP_DEPRECATED},
    {"ikelifetime",       KW_IKELIFETIME},
    {"leftikeport",       KW_LEFTIKEPORT},
    {"rightcertpolicy",   KW_RIGHTCERTPOLICY},
    {"mark",              KW_MARK},
    {"dpdaction",         KW_DPDACTION},
    {"pfsgroup",          KW_PFS_DEPRECATED},
    {"keyexchange",       KW_KEYEXCHANGE},
    {"hidetos",           KW_SETUP_DEPRECATED},
    {"leftsubnet",        KW_LEFTSUBNET},
    {"overridemtu",       KW_SETUP_DEPRECATED},
    {"installpolicy",     KW_INSTALLPOLICY},
    {"leftsourceip",      KW_LEFTSOURCEIP},
    {"dpdtimeout",        KW_DPDTIMEOUT},
    {"also",              KW_ALSO},
    {"rightupdown",       KW_RIGHTUPDOWN},
    {"charondebug",       KW_CHARONDEBUG},
    {"ldaphost",          KW_CA_DEPRECATED},
    {"fragicmp",          KW_SETUP_DEPRECATED},
    {"charonstart",       KW_SETUP_DEPRECATED},
    {"tfc",               KW_TFC},
    {"rekey",             KW_REKEY},
    {"leftsubnetwithin",  KW_LEFTSUBNET},
    {"leftid2",           KW_LEFTID2},
    {"eap_identity",      KW_EAP_IDENTITY},
    {"crlcheckinterval",  KW_SETUP_DEPRECATED},
    {"dumpdir",           KW_SETUP_DEPRECATED},
    {"cachecrls",         KW_CACHECRLS},
    {"rekeymargin",       KW_REKEYMARGIN},
    {"rightca2",          KW_RIGHTCA2},
    {"crluri2",           KW_CRLURI2},
    {"rightcert2",        KW_RIGHTCERT2},
    {"xauth_identity",    KW_XAUTH_IDENTITY},
    {"closeaction",       KW_CLOSEACTION},
    {"ocspuri2",          KW_OCSPURI2},
    {"plutostderrlog",    KW_SETUP_DEPRECATED},
    {"plutostart",        KW_SETUP_DEPRECATED},
    {"auto",              KW_AUTO},
    {"pkcs11initargs",    KW_PKCS11_DEPRECATED},
    {"pkcs11module",      KW_PKCS11_DEPRECATED},
    {"authby",            KW_AUTHBY},
    {"pkcs11keepstate",   KW_PKCS11_DEPRECATED},
    {"dpddelay",          KW_DPDDELAY},
    {"modeconfig",        KW_MODECONFIG},
    {"nocrsend",          KW_SETUP_DEPRECATED},
    {"prepluto",          KW_SETUP_DEPRECATED},
    {"leftauth2",         KW_LEFTAUTH2},
    {"postpluto",         KW_SETUP_DEPRECATED},
    {"auth",              KW_AUTH},
    {"reauth",            KW_REAUTH},
    {"xauth",             KW_XAUTH},
    {"leftauth",          KW_LEFTAUTH},
    {"pkcs11proxy",       KW_PKCS11_DEPRECATED},
    {"ikedscp",           KW_IKEDSCP,},
    {"plutodebug",        KW_SETUP_DEPRECATED}
  };

static const short lookup[] =
  {
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      0,  -1,  -1,  -1,  -1,  -1,   1,  -1,  -1,   2,
      3,   4,   5,  -1,   6,   7,   8,  -1,  -1,   9,
     10,  -1,  -1,  -1,  11,  12,  -1,  13,  -1,  14,
     15,  16,  -1,  17,  18,  19,  -1,  -1,  20,  -1,
     -1,  21,  -1,  -1,  -1,  -1,  22,  -1,  -1,  23,
     24,  -1,  25,  26,  27,  28,  29,  30,  31,  32,
     33,  34,  35,  36,  -1,  37,  38,  39,  -1,  -1,
     40,  -1,  -1,  -1,  -1,  -1,  41,  -1,  42,  43,
     44,  45,  46,  47,  48,  -1,  -1,  -1,  -1,  49,
     50,  51,  52,  53,  54,  55,  56,  57,  -1,  -1,
     -1,  58,  59,  60,  61,  62,  63,  64,  65,  -1,
     66,  67,  68,  69,  70,  71,  72,  -1,  -1,  73,
     74,  -1,  75,  76,  77,  78,  79,  -1,  80,  81,
     82,  83,  84,  85,  86,  87,  88,  89,  90,  91,
     92,  -1,  -1,  93,  -1,  -1,  94,  95,  -1,  96,
     97,  -1,  98,  -1,  99, 100, 101,  -1, 102, 103,
    104,  -1, 105,  -1,  -1,  -1, 106,  -1, 107,  -1,
     -1,  -1, 108,  -1,  -1,  -1, 109,  -1,  -1,  -1,
     -1, 110, 111, 112, 113, 114,  -1,  -1,  -1,  -1,
     -1,  -1,  -1, 115,  -1,  -1,  -1,  -1,  -1,  -1,
    116, 117,  -1,  -1, 118,  -1,  -1,  -1, 119,  -1,
    120, 121,  -1, 122,  -1,  -1,  -1, 123,  -1, 124,
    125, 126,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 127,
     -1,  -1,  -1, 128,  -1,  -1,  -1, 129,  -1,  -1,
     -1, 130, 131, 132,  -1,  -1, 133,  -1, 134, 135
  };

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct kw_entry *
in_word_set (str, len)
     register const char *str;
     register unsigned int len;
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strcmp (str + 1, s + 1))
                return &wordlist[index];
            }
        }
    }
  return 0;
}
