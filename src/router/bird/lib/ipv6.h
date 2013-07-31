
/*
 *	BIRD -- IP Addresses et Cetera for IPv6
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_IPV6_H_
#define _BIRD_IPV6_H_

#include <sys/types.h>
#include <netinet/in.h>
#include "lib/string.h"
#include "lib/bitops.h"
#include "lib/unaligned.h"

typedef struct ipv6_addr {
  u32 addr[4];
} ip_addr;

#define _MI(a,b,c,d) ((struct ipv6_addr) {{ a, b, c, d }})
#define _I0(a) ((a).addr[0])
#define _I1(a) ((a).addr[1])
#define _I2(a) ((a).addr[2])
#define _I3(a) ((a).addr[3])

#define MAX_PREFIX_LENGTH 128
#define BITS_PER_IP_ADDRESS 128
#define STD_ADDRESS_P_LENGTH 39
#define SIZE_OF_IP_HEADER 40

#define IPA_NONE _MI(0,0,0,0)

#define ipa_equal(x,y)  ({ ip_addr _a=(x), _b=(y); \
			   _I0(_a) == _I0(_b) && \
			   _I1(_a) == _I1(_b) && \
			   _I2(_a) == _I2(_b) && \
			   _I3(_a) == _I3(_b); })
#define ipa_nonzero(x) ({ ip_addr _a=(x); (_I0(_a) || _I1(_a) || _I2(_a) || _I3(_a)); })
#define ipa_and(x,y) ({ ip_addr _a=(x), _b=(y); \
		     _MI(_I0(_a) & _I0(_b), \
			 _I1(_a) & _I1(_b), \
			 _I2(_a) & _I2(_b), \
			 _I3(_a) & _I3(_b)); })
#define ipa_or(x,y)  ({ ip_addr _a=(x), _b=(y); \
		     _MI(_I0(_a) | _I0(_b), \
			 _I1(_a) | _I1(_b), \
			 _I2(_a) | _I2(_b), \
			 _I3(_a) | _I3(_b)); })
#define ipa_xor(x,y) ({ ip_addr _a=(x), _b=(y); \
		     _MI(_I0(_a) ^ _I0(_b), \
			 _I1(_a) ^ _I1(_b), \
			 _I2(_a) ^ _I2(_b), \
			 _I3(_a) ^ _I3(_b)); })
#define ipa_not(x) ({ ip_addr _a=(x); _MI(~_I0(_a),~_I1(_a),~_I2(_a),~_I3(_a)); })
#define ipa_mkmask(x) ipv6_mkmask(x)
#define ipa_mklen(x) ipv6_mklen(&(x))
#define ipa_hash(x) ipv6_hash(&(x))
#define ipa_hton(x) ipv6_hton(&(x))
#define ipa_ntoh(x) ipv6_ntoh(&(x))
#define ipa_classify(x) ipv6_classify(&(x))
#define ipa_has_link_scope(x) ipv6_has_link_scope(&(x))
#define ipa_opposite_m1(x) ({ ip_addr _a=(x); _MI(_I0(_a),_I1(_a),_I2(_a),_I3(_a) ^ 1); })
#define ipa_opposite_m2(x) ({ ip_addr _a=(x); _MI(_I0(_a),_I1(_a),_I2(_a),_I3(_a) ^ 3); })
/* ipa_class_mask don't make sense with IPv6 */
/* ipa_from_u32 and ipa_to_u32 replaced by ipa_build */
#define ipa_build(a,b,c,d) _MI(a,b,c,d)
#define ipa_compare(x,y) ipv6_compare(x,y)
/* ipa_pxlen() requires that x != y */
#define ipa_pxlen(x, y) ipv6_pxlen(x, y)
#define ipa_getbit(x, y) ipv6_getbit(x, y)
#define ipa_put_addr(x, y) ipv6_put_addr(x, y)
#define ipa_absolutize(x,y) ipv6_absolutize(x,y)

/* In IPv6, SOCK_RAW does not return packet header */
#define ip_skip_header(x, y) x

ip_addr ipv6_mkmask(unsigned);
unsigned ipv6_mklen(ip_addr *);
int ipv6_classify(ip_addr *);
void ipv6_hton(ip_addr *);
void ipv6_ntoh(ip_addr *);
int ipv6_compare(ip_addr, ip_addr);
int ipv4_pton_u32(char *, u32 *);
void ipv6_absolutize(ip_addr *, ip_addr *);

static inline int ipv6_has_link_scope(ip_addr *a)
{
  return ((a->addr[0] & 0xffc00000) == 0xfe800000);
}

/*
 *  This hash function looks well, but once IPv6 enters
 *  mainstream use, we need to check that it has good
 *  distribution properties on real routing tables.
 */

static inline unsigned ipv6_hash(ip_addr *a)
{
  /* Returns a 16-bit hash key */
  u32 x = _I0(*a) ^ _I1(*a) ^ _I2(*a) ^ _I3(*a);
  return (x ^ (x >> 16) ^ (x >> 8)) & 0xffff;
}

static inline u32 ipv6_getbit(ip_addr a, u32 y)
{
  return a.addr[y / 32] & (0x80000000 >> (y % 32));
}

static inline u32 ipv6_pxlen(ip_addr a, ip_addr b)
{
  int i = 0;
  i+= (a.addr[i] == b.addr[i]);
  i+= (a.addr[i] == b.addr[i]);
  i+= (a.addr[i] == b.addr[i]);
  i+= (a.addr[i] == b.addr[i]);
  return 32 * i + 31 - u32_log2(a.addr[i] ^ b.addr[i]);
}

static inline byte * ipv6_put_addr(byte *buf, ip_addr a)
{
  put_u32(buf+0,  _I0(a));
  put_u32(buf+4,  _I1(a));
  put_u32(buf+8,  _I2(a));
  put_u32(buf+12, _I3(a));
  return buf+16;
}

#define IP_PREC_INTERNET_CONTROL 0xc0

#endif
