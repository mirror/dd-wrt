/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 1998-2013 Sourcefire, Inc.
** Adam Keeton
** Kevin Liu <kliu@sourcefire.com>
*
** $ID: $
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
 * Adam Keeton
 * sf_ip.h
 * 11/17/06
*/

#ifndef SF_IP_H
#define SF_IP_H

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef WIN32
#include <ws2tcpip.h>
#endif

#include "snort_debug.h" /* for inline definition */

/* define SFIP_ROBUST to check pointers passed into the sfip libs.
 * Robustification should not be enabled if the client code is trustworthy.
 * Namely, if pointers are checked once in the client, or are pointers to
 * data allocated on the stack, there's no need to check them again here.
 * The intention is to prevent the same stack-allocated variable from being
 * checked a dozen different times. */
#define SFIP_ROBUST

#ifdef SFIP_ROBUST

#define ARG_CHECK1(a, z) if(!a) return z;
#define ARG_CHECK2(a, b, z) if(!a || !b) return z;
#define ARG_CHECK3(a, b, c, z) if(!a || !b || !c) return z;

#elif defined(DEBUG)

#define ARG_CHECK1(a, z) assert(a);
#define ARG_CHECK2(a, b, z) assert(a); assert(b);
#define ARG_CHECK3(a, b, c, z) assert(a); assert(b); assert(c);

#else

#define ARG_CHECK1(a, z)
#define ARG_CHECK2(a, b, z)
#define ARG_CHECK3(a, b, c, z)

#endif

#ifndef WIN32
#if !defined(s6_addr8)
#define s6_addr8  __u6_addr.__u6_addr8
#endif
#if !defined(s6_addr16)
#define s6_addr16 __u6_addr.__u6_addr16
#endif
#if !defined(s6_addr32)
#define s6_addr32 __u6_addr.__u6_addr32
#endif

#ifdef _WIN32
#pragma pack(push,1)
#endif

struct _sfaddr
{
    struct in6_addr ip;
    uint16_t family;
#   define ia8  ip.s6_addr
#   define ia16 ip.s6_addr16
#   define ia32 ip.s6_addr32
#ifdef _WIN32
};
#pragma pack(pop)
#else
} __attribute__((__packed__));
#endif
typedef struct _sfaddr sfaddr_t;

#ifdef _WIN32
#pragma pack(push,1)
#endif

struct _ip {
    sfaddr_t addr;
    uint16_t bits;
#   define ip8  addr.ip.s6_addr
#   define ip16 addr.ip.s6_addr16
#   define ip32 addr.ip.s6_addr32
#   define ip_family addr.family
#ifdef _WIN32
};
#pragma pack(pop)
#else
} __attribute__((__packed__));
#endif

typedef struct _ip sfcidr_t;
#else // WIN32 Build
#if !defined(s6_addr8)
#define s6_addr8  u.u6_addr8
#endif
#if !defined(s6_addr16)
#define s6_addr16 u.u6_addr16
#endif
#if !defined(s6_addr32)
#define s6_addr32 u.u6_addr32
#endif

struct sf_in6_addr {
    union {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
    } in6_u;
};

#pragma pack(push,1)
struct _sfaddr {
    struct in6_addr ip;
    uint16_t family;
#   define ia8  ip.s6_addr
#   define ia16 ip.s6_addr16
#   define ia32 ip.s6_addr32
};
typedef struct _sfaddr sfaddr_t;

struct _ip {
    sfaddr_t addr;
    uint16_t bits;
#   define ip8  addr.ip.s6_addr
#   define ip16 addr.ip.s6_addr16
#   define ip32 addr.ip.s6_addr32
#   define ip_family addr.family
};
typedef struct _ip sfcidr_t;
#pragma pack(pop)

#endif // WIN32

typedef enum _return_values {
    SFIP_SUCCESS=0,
    SFIP_FAILURE,
    SFIP_LESSER,
    SFIP_GREATER,
    SFIP_EQUAL,
    SFIP_ARG_ERR,
    SFIP_CIDR_ERR,
    SFIP_INET_PARSE_ERR,
    SFIP_INVALID_MASK,
    SFIP_ALLOC_ERR,
    SFIP_CONTAINS,
    SFIP_NOT_CONTAINS,
    SFIP_DUPLICATE,         /* Tried to add a duplicate variable name to table */
    SFIP_LOOKUP_FAILURE,    /* Failed to lookup a variable from the table */
    SFIP_UNMATCHED_BRACKET, /* IP lists that are missing a closing bracket */
    SFIP_NOT_ANY,           /* For !any */
    SFIP_CONFLICT,          /* For IP conflicts in IP lists */
    SFIP_INVALID_VAR	    /* variable definition is invalid */
} SFIP_RET;


/* IP allocations and setting ******************************************/

/* Parses "src" and stores results in "dst" */
/* If the conversion is invalid, returns SFIP_FAILURE */
SFIP_RET sfaddr_pton(const char *src, sfaddr_t *dst);
SFIP_RET sfip_pton(const char *src, sfcidr_t *dst);

/* Allocate IP address from a character array describing the IP */
sfcidr_t *sfip_alloc(const char *ip, SFIP_RET *status);

/* Frees an sfcidr_t */
void sfip_free(sfcidr_t *ip);

/* Allocate IP address from a character array describing the IP */
sfaddr_t *sfaddr_alloc(const char *ip, SFIP_RET *status);

/* Frees an sfaddr_t */
void sfaddr_free(sfaddr_t *ip);

/* Allocate IP address from an array of integers.  The array better be
 * long enough for the given family! */
sfaddr_t *sfip_alloc_raw(void *ip, int family, SFIP_RET *status);

/* Sets existing IP, "dst", to a raw source IP (4 or 16 bytes,
 * according to family) */
SFIP_RET sfip_set_raw(sfaddr_t *dst, const void *src, int src_family);

/* Sets existing IP, "dst", to be source IP, "src" */
#define sfip_set_ip(dst, src)   *(dst) = *(src)

/* Obfuscates an IP */
void sfip_obfuscate(sfcidr_t *ob, sfaddr_t *ip);

/* Member-access *******************************************************/

#define sfip_get_ip4_value(x) ((x)->ip32[3])
#define sfaddr_get_ip4_value(x) ((x)->ia32[3])

#define sfip_get_ip4_ptr(x) (&(x)->ip32[3])
#define sfip_get_ip6_ptr(x) ((x)->ip32)
#define sfip_get_ptr(x) (((x)->ip_family == AF_INET) ? &(x)->ip32[3] : (x)->ip32)

#define sfaddr_get_ip4_ptr(x) (&(x)->ia32[3])
#define sfaddr_get_ip6_ptr(x) ((x)->ia32)
#define sfaddr_get_ptr(x) (((x)->family == AF_INET) ? &(x)->ia32[3] : (x)->ia32)

/* Returns the family of "ip", either AF_INET or AF_INET6 */
/* XXX This is a performance critical function,
*  need to determine if it's safe to not check these pointers */
/* ARG_CHECK1(ip, 0); */
#define sfaddr_family(x)  ((x)->family)
#define sfip_family(x)  ((x)->ip_family)

/* Returns the number of bits used for masking "ip" */
static inline unsigned char sfip_bits(const sfcidr_t *ip) {
    ARG_CHECK1(ip, 0);
    return (unsigned char)ip->bits;
}

static inline void sfip_set_bits(sfcidr_t *p, int bits) {

    if(!p)
        return;

    if(bits < 0 || bits > 128) return;

    p->bits = bits;
}

/* Returns the raw IP address as an in6_addr */
/*inline struct in6_addr sfip_to_raw(sfcidr_t *); */



/* IP Comparisons ******************************************************/

/* Check if ip is contained within the network specified by net */
/* Returns SFIP_EQUAL if so */
SFIP_RET sfip_contains(const sfcidr_t *net, const sfaddr_t *ip);

/* Returns 1 if the IP is non-zero. 0 otherwise */
/* XXX This is a performance critical function, \
 *  need to determine if it's safe to not check these pointers */\
static inline int sfraw_is_set(const struct in6_addr *addr) {
/*    ARG_CHECK1(ip, -1); */
    return (addr->s6_addr32[3] || addr->s6_addr32[0] || addr->s6_addr32[1] || addr->s6_addr16[4] ||
            (addr->s6_addr16[5] && addr->s6_addr16[5] != 0xFFFF)) ? 1 : 0;
}

static inline int sfaddr_is_set(const sfaddr_t *addr) {
/*    ARG_CHECK1(ip, -1); */
    return ((addr->family == AF_INET && addr->ia32[3]) ||
            (addr->family == AF_INET6 &&
             (addr->ia32[0] || addr->ia32[1] || addr->ia32[3] || addr->ia16[4] ||
              (addr->ia16[5] && addr->ia16[5] != 0xFFFF)))) ? 1 : 0;
}

static inline int sfip_is_set(const sfcidr_t *ip) {
/*    ARG_CHECK1(ip, -1); */
    return (sfaddr_is_set(&ip->addr) ||
            ((ip->ip_family == AF_INET || ip->ip_family == AF_INET6) &&
             ip->bits != 128)) ? 1 : 0;
}

/* Return 1 if the IP is a loopback IP */
int sfip_is_loopback(const sfaddr_t *ip);

/* Returns 1 if the IPv6 address appears mapped. 0 otherwise. */
static inline int sfip_ismapped(const sfaddr_t *ip) {
    ARG_CHECK1(ip, 0);

    return (ip->ia32[0] || ip->ia32[1] || ip->ia16[4] || (ip->ia16[5] != 0xffff && ip->ia16[5])) ? 0 : 1;
}

/* Support function for sfip_compare */
static inline SFIP_RET _ip4_cmp(u_int32_t ip1, u_int32_t ip2) {
    u_int32_t hip1 = htonl(ip1);
    u_int32_t hip2 = htonl(ip2);
    if(hip1 < hip2) return SFIP_LESSER;
    if(hip1 > hip2) return SFIP_GREATER;
    return SFIP_EQUAL;
}

/* Support function for sfip_compare */
static inline SFIP_RET _ip6_cmp(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    SFIP_RET ret;
    const struct in6_addr p1 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip1);
    const struct in6_addr p2 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip2);

    /* XXX
     * Argument are assumed trusted!
     * This function is presently only called by sfip_compare
     * on validated pointers.
     * XXX */

    if( (ret = _ip4_cmp(p1.s6_addr32[0], p2.s6_addr32[0])) != SFIP_EQUAL) return ret;
    if( (ret = _ip4_cmp(p1.s6_addr32[1], p2.s6_addr32[1])) != SFIP_EQUAL) return ret;
    if( (ret = _ip4_cmp(p1.s6_addr32[2], p2.s6_addr32[2])) != SFIP_EQUAL) return ret;
    if( (ret = _ip4_cmp(p1.s6_addr32[3], p2.s6_addr32[3])) != SFIP_EQUAL) return ret;

    return ret;
}

/* Compares two IPs
 * Returns SFIP_LESSER, SFIP_EQUAL, SFIP_GREATER, if ip1 is less than, equal to,
 * or greater than ip2 In the case of mismatched families, the IPv4 address
 * is converted to an IPv6 representation. */
/* XXX-IPv6 Should add version of sfip_compare that just tests equality */
static inline SFIP_RET sfip_compare(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    int f1,f2;

    ARG_CHECK2(ip1, ip2, SFIP_ARG_ERR);

    /* This is being done because at some points in the existing Snort code,
     * an unset IP is considered to match anything.  Thus, if either IP is not
     * set here, it's considered equal. */
    if(!sfaddr_is_set(ip1) || !sfaddr_is_set(ip2)) return SFIP_EQUAL;

    f1 = sfaddr_family(ip1);
    f2 = sfaddr_family(ip2);

    if(f1 == AF_INET && f2 == AF_INET) {
        return _ip4_cmp(sfaddr_get_ip4_value(ip1), sfaddr_get_ip4_value(ip2));
    }
    return _ip6_cmp(ip1, ip2);
}

/* Compares two CIDRs
 * Returns SFIP_LESSER, SFIP_EQUAL, SFIP_GREATER, if ip1 is less than, equal to,
 * or greater than ip2 In the case of mismatched families, the IPv4 address
 * is converted to an IPv6 representation. */
static inline SFIP_RET sfip_cidr_compare(const sfcidr_t* ip1, const sfcidr_t *ip2) {
    SFIP_RET ret = sfip_compare(&ip1->addr, &ip2->addr);
    if(SFIP_EQUAL == ret)
    {
        if(ip1->bits < ip2->bits) return SFIP_LESSER;
        if(ip1->bits > ip2->bits) return SFIP_GREATER;
    }
    return ret;
}

/* Compares two IPs
 * Returns SFIP_LESSER, SFIP_EQUAL, SFIP_GREATER, if ip1 is less than, equal to,
 * or greater than ip2 In the case of mismatched families, the IPv4 address
 * is converted to an IPv6 representation. */
/* XXX-IPv6 Should add version of sfip_compare that just tests equality */
static inline SFIP_RET sfip_compare_unset(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    int f1,f2;

    ARG_CHECK2(ip1, ip2, SFIP_ARG_ERR);

    /* This is to handle the special case when one of the values being
     * unset is considered to match nothing.  This is the opposite of
     * sfip_compare(), defined above.  Thus, if either IP is not
     * set here, it's considered not equal. */
    if(!sfaddr_is_set(ip1) || !sfaddr_is_set(ip2)) return SFIP_FAILURE;

    f1 = sfaddr_family(ip1);
    f2 = sfaddr_family(ip2);

    if(f1 == AF_INET && f2 == AF_INET) {
        return _ip4_cmp(sfaddr_get_ip4_value(ip1), sfaddr_get_ip4_value(ip2));
    }
    return _ip6_cmp(ip1, ip2);
}

static inline int sfip_fast_lt4(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    return sfaddr_get_ip4_value(ip1) < sfaddr_get_ip4_value(ip2);
}
static inline int sfip_fast_gt4(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    return sfaddr_get_ip4_value(ip1) > sfaddr_get_ip4_value(ip2);
}
static inline int sfip_fast_eq4(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    return sfaddr_get_ip4_value(ip1) == sfaddr_get_ip4_value(ip2);
}

static inline int sfip_fast_lt6(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    const struct in6_addr p1 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip1);
    const struct in6_addr p2 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip2);

    if(p1.s6_addr32[0] < p2.s6_addr32[0]) return 1;
    else if(p1.s6_addr32[0] > p2.s6_addr32[0]) return 0;

    if(p1.s6_addr32[1] < p2.s6_addr32[1]) return 1;
    else if(p1.s6_addr32[1] > p2.s6_addr32[1]) return 0;

    if(p1.s6_addr32[2] < p2.s6_addr32[2]) return 1;
    else if(p1.s6_addr32[2] > p2.s6_addr32[2]) return 0;

    if(p1.s6_addr32[3] < p2.s6_addr32[3]) return 1;
    else if(p1.s6_addr32[3] > p2.s6_addr32[3]) return 0;

    return 0;
}

static inline int sfip_fast_gt6(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    const struct in6_addr p1 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip1);
    const struct in6_addr p2 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip2);

    if(p1.s6_addr32[0] > p2.s6_addr32[0]) return 1;
    else if(p1.s6_addr32[0] < p2.s6_addr32[0]) return 0;

    if(p1.s6_addr32[1] > p2.s6_addr32[1]) return 1;
    else if(p1.s6_addr32[1] < p2.s6_addr32[1]) return 0;

    if(p1.s6_addr32[2] > p2.s6_addr32[2]) return 1;
    else if(p1.s6_addr32[2] < p2.s6_addr32[2]) return 0;

    if(p1.s6_addr32[3] > p2.s6_addr32[3]) return 1;
    else if(p1.s6_addr32[3] < p2.s6_addr32[3]) return 0;

    return 0;
}

static inline int sfip_fast_eq6(const sfaddr_t *ip1, const sfaddr_t *ip2) {
    const struct in6_addr p1 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip1);
    const struct in6_addr p2 = *(struct in6_addr *)sfaddr_get_ip6_ptr(ip2);

    if(p1.s6_addr32[0] != p2.s6_addr32[0]) return 0;
    if(p1.s6_addr32[1] != p2.s6_addr32[1]) return 0;
    if(p1.s6_addr32[2] != p2.s6_addr32[2]) return 0;
    if(p1.s6_addr32[3] != p2.s6_addr32[3]) return 0;

    return 1;
}

/* Checks if ip2 is equal to ip1 or contained within the CIDR ip1 */
static inline int sfip_fast_cont4(const sfcidr_t *ip1, const sfaddr_t *ip2) {
    uint32_t shift = 128 - sfip_bits(ip1);
    uint32_t ip = ntohl(sfaddr_get_ip4_value(ip2));
    uint32_t ip3 = ntohl(sfip_get_ip4_value(ip1));

    ip >>= shift;
    ip <<= shift;

    if(ip3 == 0)
        return 1;

    return (ip3 == ip);
}

/* Checks if ip2 is equal to ip1 or contained within the CIDR ip1 */
static inline int sfip_fast_cont6(const sfcidr_t *ip1, const sfaddr_t *ip2) {
    uint32_t ip;
    int i, bits = sfip_bits(ip1);
    int words = bits / 32;
    bits = 32 - (bits % 32);

    for ( i = 0; i < words; i++ ) {
        if ( ip1->ip32[i] != ip2->ia32[i] )
            return 0;
    }

    if ( bits == 32 ) return 1;

    ip = ntohl(ip2->ia32[i]);

    ip >>= bits;
    ip <<= bits;

    return ntohl(ip1->ip32[i]) == ip;
}

/* Compares two IPs
 * Returns 1 for equal and 0 for not equal
 */
static inline int sfip_fast_equals_raw(const sfaddr_t *ip1, const sfaddr_t *ip2)
{
    int f1,f2;

    ARG_CHECK2(ip1, ip2, 0);

    f1 = sfaddr_family(ip1);
    f2 = sfaddr_family(ip2);

    if(f1 == AF_INET)
    {
        if(f2 != AF_INET)
            return 0;
        if (sfip_fast_eq4(ip1, ip2))
            return 1;
    }
    else if(f1 == AF_INET6)
    {
        if(f2 != AF_INET6)
            return 0;
        if (sfip_fast_eq6(ip1, ip2))
            return 1;
    }
    return 0;
}

/********************************************************************
 * Function: sfip_is_private()
 *
 * Checks if the address is local
 *
 * Arguments:
 *  sfcidr_t * - IP address to check
 *
 * Returns:
 *  1  if the IP is in local network
 *  0  otherwise
 *
 ********************************************************************/
static inline int sfip_is_private(const sfaddr_t *ip)
{
    ARG_CHECK1(ip, 0);

    /* Check the first 80 bits in an IPv6 address, and */
    /* verify they're zero.  If not, it's not a loopback */
    if(ip->ia32[0] || ip->ia32[1] || ip->ia16[4]) return 0;

    if ( ip->ia16[5] == 0xffff ) {
        /* ::ffff: IPv4 mapped over IPv6 */
        /*
         * 10.0.0.0        -   10.255.255.255  (10/8 prefix)
         * 172.16.0.0      -   172.31.255.255  (172.16/12 prefix)
         * 192.168.0.0     -   192.168.255.255 (192.168/16 prefix)
         * */
        return ( (ip->ia8[12] == 10)
                ||((ip->ia8[12] == 172) && ((ip->ia8[13] & 0xf0 ) == 16))
                ||((ip->ia8[12] == 192) && (ip->ia8[13] == 168)) );
    }

    /* Check if the 3rd 32-bit int is zero */
    if ( !ip->ia16[5] ) {
        /* ::ipv4 compatible ipv6 */
        /* ::1 is the IPv6 loopback */
        return ( (ip->ia8[12] == 10)
                ||((ip->ia8[12] == 172) && ((ip->ia8[13] & 0xf0 ) == 16))
                ||((ip->ia8[12] == 192) && (ip->ia8[13] == 168))
                || (ip->ia32[3] == htonl(0x1)) );
    }

    return 0;
}

static inline void sfaddr_copy_to_raw(struct in6_addr *dst, const sfaddr_t *src)
{
    dst->s6_addr32[0] = src->ia32[0];
    dst->s6_addr32[1] = src->ia32[1];
    dst->s6_addr32[2] = src->ia32[2];
    dst->s6_addr32[3] = src->ia32[3];
}

#define sfip_equals(x,y) (sfip_compare(&x, &y) == SFIP_EQUAL)
#define sfip_not_equals !sfip_equals
#define sfip_clear(x) memset(x, 0, 16)

/* Printing ************************************************************/

/* Uses a static buffer to return a string representation of the IP */
char *sfip_to_str(const sfaddr_t *ip);
#define sfip_ntoa(x) sfip_to_str(x)
void sfip_raw_ntop(int family, const void *ip_raw, char *buf, int bufsize);
void sfip_ntop(const sfaddr_t *ip, char *buf, int bufsize);

/* Conversions *********************************************************/

SFIP_RET sfip_convert_ip_text_to_binary( const int, const char *src, void *dst );

#endif /* SF_IP_H */

