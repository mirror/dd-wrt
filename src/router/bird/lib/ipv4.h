/*
 *	BIRD -- IP Addresses et Cetera for IPv4
 *
 *	(c) 1998--1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_IPV4_H_
#define _BIRD_IPV4_H_

#include "lib/endian.h"
#include "lib/bitops.h"
#include "lib/unaligned.h"

#ifdef DEBUGGING

/*
 *	Use the structural representation when you want to make sure
 *	nobody unauthorized attempts to handle ip_addr as number.
 */

typedef struct ipv4_addr {
  u32 addr;
} ip_addr;

#define _I(x) (x).addr
#define _MI(x) ((struct ipv4_addr) { x })

#else

typedef u32 ip_addr;

#define _I(x) (x)
#define _MI(x) (x)

#endif

#define MAX_PREFIX_LENGTH 32
#define BITS_PER_IP_ADDRESS 32
#define STD_ADDRESS_P_LENGTH 15
#define SIZE_OF_IP_HEADER 24

#define IPA_NONE (_MI(0))

#define ipa_equal(x,y) (_I(x) == _I(y))
#define ipa_nonzero(x) _I(x)
#define ipa_and(x,y) _MI(_I(x) & _I(y))
#define ipa_or(x,y) _MI(_I(x) | _I(y))
#define ipa_xor(x,y) _MI(_I(x) ^ _I(y))
#define ipa_not(x) _MI(~_I(x))
#define ipa_mkmask(x) _MI(u32_mkmask(x))
#define ipa_mklen(x) u32_masklen(_I(x))
#define ipa_hash(x) ipv4_hash(_I(x))
#define ipa_hton(x) x = _MI(htonl(_I(x)))
#define ipa_ntoh(x) x = _MI(ntohl(_I(x)))
#define ipa_classify(x) ipv4_classify(_I(x))
#define ipa_has_link_scope(x) ipv4_has_link_scope(_I(x))
#define ipa_opposite_m1(x) _MI(_I(x) ^ 1)
#define ipa_opposite_m2(x) _MI(_I(x) ^ 3)
#define ipa_class_mask(x) _MI(ipv4_class_mask(_I(x)))
#define ipa_from_u32(x) _MI(x)
#define ipa_to_u32(x) _I(x)
#define ipa_compare(x,y) ipv4_compare(_I(x),_I(y))
/* ipa_pxlen() requires that x != y */
#define ipa_pxlen(x, y) ipv4_pxlen(_I(x), _I(y))
#define ipa_getbit(x, y) (_I(x) & (0x80000000 >> (y)))
#define ipa_put_addr(x, y) ipv4_put_addr(x, y)

#define ip_skip_header(x, y) ipv4_skip_header(x, y)

int ipv4_classify(u32);
u32 ipv4_class_mask(u32);
byte *ipv4_skip_header(byte *, int *);

static inline int ipv4_has_link_scope(u32 a UNUSED)
{
  return 0;
}

static inline unsigned ipv4_hash(u32 a)
{
  /* Returns a 16-bit value */
  a ^= a >> 16;
  a ^= a << 10;
  return a & 0xffff;
}

static inline int ipv4_compare(u32 x, u32 y)
{
  return (x > y) - (x < y);
}

static inline u32 ipv4_pxlen(u32 a, u32 b)
{
  return 31 - u32_log2(a ^ b);
}

static inline byte * ipv4_put_addr(byte *buf, ip_addr a)
{
  put_u32(buf, _I(a));
  return buf+4;
}

#define IP_PREC_INTERNET_CONTROL 0xc0

#endif
