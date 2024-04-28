/*
 **************************************************************************
 * Copyright (c) 2014-2015 The Linux Foundation.  All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#ifndef ECM_TYPES_H_
#define ECM_TYPES_H_

#include <linux/printk.h>

/*
 * The ECM IP address is an array of 4 32 bit numbers.
 * This is enough to record both an IPv6 address aswell as an IPv4 address.
 * IPv4 addresses are stored encoded in an IPv6 as the usual ::FFFF:x:y/96
 * We store IP addresses in host order format and NOT network order which is different to Linux network internals.
 */
typedef uint32_t ip_addr_t[4];

/*
 * 32/64 bits pointer type
 */
#ifdef __LP64__
typedef uint64_t ecm_ptr_t;
#else
typedef uint32_t ecm_ptr_t;
#endif

#define ECM_IP_ADDR_OCTAL_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#define ECM_IP_ADDR_DOT_FMT "%u.%u.%u.%u"

#define ECM_IP_ADDR_TO_OCTAL(ipaddrt) ((uint16_t *)ipaddrt)[7], ((uint16_t *)ipaddrt)[6], ((uint16_t *)ipaddrt)[5], ((uint16_t *)ipaddrt)[4], ((uint16_t *)ipaddrt)[3], ((uint16_t *)ipaddrt)[2], ((uint16_t *)ipaddrt)[1], ((uint16_t *)ipaddrt)[0]

#define ECM_IP_ADDR_TO_DOT(ipaddrt) ((uint8_t *)ipaddrt)[3], ((uint8_t *)ipaddrt)[2], ((uint8_t *)ipaddrt)[1], ((uint8_t *)ipaddrt)[0]

#define ECM_IP_ADDR_MATCH(a, b) \
	((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]))

#define ECM_IP_ADDR_IS_V4(a) \
	((a[1] == 0x0000ffff) && !a[2] && !a[3])

#define ECM_IP_ADDR_IS_NULL(a) \
	((a[0] | a[1] | a[2] | a[3]) == 0)

#define ECM_IP_ADDR_NULL {0, 0, 0, 0}

#define ECM_MAC_ADDR_STR_BUFF_SIZE		18	/* This is the size of a string in the format of aa:bb:cc:dd:ee:ff */
#define ECM_IP_ADDR_STR_BUFF_SIZE		40	/* This is the size of a string in the format of aaaa:bbbb:cccc:0000:1111:dddd:eeee:ffff */
#define ECM_IP_ADDR_DOT_FMT_STR_BUFF_SIZE	16	/* This is the size of a string in the format of 192.168.100.200 */

/*
 * Type checking functions for various forms of IP address
 * Placing these in a macro, enables the compiler to check
 * that the caller is doing the right thing.
 */
static inline void ecm_type_check_ecm_ip_addr(ip_addr_t ipaddr){}
static inline void ecm_type_check_linux_ipv4(__be32 ipaddr){}
static inline void ecm_type_check_ae_ipv4(uint32_t addr){}
#ifdef ECM_IPV6_ENABLE
static inline void ecm_type_check_linux_ipv6(struct in6_addr in6){}
static inline void ecm_type_check_ae_ipv6(uint32_t ip6[4]){}
#endif

/*
 * This macro copies ip_addr_t's
 * It's usually quicker than a memcpy().
 */
#define __ECM_IP_ADDR_COPY_NO_CHECK(d, s) \
	{ \
		d[0] = s[0]; \
		d[1] = s[1]; \
		d[2] = s[2]; \
		d[3] = s[3]; \
	}

#define ECM_IP_ADDR_COPY(d,s) \
	{ \
		ecm_type_check_ecm_ip_addr(d); \
		ecm_type_check_ecm_ip_addr(s); \
		__ECM_IP_ADDR_COPY_NO_CHECK(d,s); \
	}

#define ECM_IP_ADDR_HASH(h, a) \
	{ \
		ecm_type_check_ecm_ip_addr(a); \
		h = a[0] ^ a[1] ^ a[2] ^ a[3]; \
	}

/*
 * This macro converts from Linux IPv4 address (network order) to ECM ip_addr_t
 */
#define ECM_NIN4_ADDR_TO_IP_ADDR(ipaddrt, nin4) \
	{ \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ecm_type_check_linux_ipv4(nin4); \
		ipaddrt[0] = ntohl(nin4); \
		ipaddrt[1] = 0x0000ffff; \
		ipaddrt[2] = 0x00000000; \
		ipaddrt[3] = 0x00000000; \
	}

/*
 * This macro converts from ECM ip_addr_t to Linux network IPv4 address
 */
#define ECM_IP_ADDR_TO_NIN4_ADDR(nin4, ipaddrt) \
	{ \
		nin4 = 0; \
		ecm_type_check_linux_ipv4(nin4); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		DEBUG_ASSERT(!ipaddrt[3] && !ipaddrt[2] && (ipaddrt[1] == 0x0000ffff), "Not IPv4 address: " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(ipaddrt)); \
		nin4 = htonl(ipaddrt[0]); \
	}

/*
 * This macro converts from Linux IPv4 address (host order) to ECM ip_addr_t
 */
#define ECM_HIN4_ADDR_TO_IP_ADDR(ipaddrt, hin4) \
	{ \
		ecm_type_check_linux_ipv4(hin4); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ipaddrt[0] = hin4; \
		ipaddrt[1] = 0x0000ffff; \
		ipaddrt[2] = 0x00000000; \
		ipaddrt[3] = 0x00000000; \
	}

/*
 * This macro converts from ECM ip_addr_t to Linux Host IPv4 address
 */
#define ECM_IP_ADDR_TO_HIN4_ADDR(hin4, ipaddrt) \
	{ \
		ecm_type_check_linux_ipv4(hin4); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		DEBUG_ASSERT(!ipaddrt[3] && !ipaddrt[2] && (ipaddrt[1] == 0x0000ffff), "Not IPv4 address: " ECM_IP_ADDR_OCTAL_FMT "\n", ECM_IP_ADDR_TO_OCTAL(ipaddrt)); \
		hin4 = ipaddrt[0]; \
	}

#ifdef ECM_IPV6_ENABLE
#define ECM_LINUX6_TO_IP_ADDR(d,s) \
	{ \
		ecm_type_check_ecm_ip_addr(d); \
		ecm_type_check_ae_ipv6(&s); \
		__ECM_IP_ADDR_COPY_NO_CHECK(d,s); \
	}

#define ECM_IP_ADDR_TO_LINUX6(d,s) \
	{ \
		ecm_type_check_ae_ipv6(&d); \
		ecm_type_check_ecm_ip_addr(s); \
		__ECM_IP_ADDR_COPY_NO_CHECK(d,s); \
	}

/*
 * This macro converts from Linux IPv6 address (network order) to ECM ip_addr_t
 */
#define ECM_NIN6_ADDR_TO_IP_ADDR(ipaddrt, nin6) \
	{ \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ecm_type_check_linux_ipv6(nin6); \
		ipaddrt[0] = ntohl(nin6.in6_u.u6_addr32[3]); \
		ipaddrt[1] = ntohl(nin6.in6_u.u6_addr32[2]); \
		ipaddrt[2] = ntohl(nin6.in6_u.u6_addr32[1]); \
		ipaddrt[3] = ntohl(nin6.in6_u.u6_addr32[0]); \
	}

/*
 * This macro converts from ECM ip_addr_t to Linux IPv6 address (network order)
 */
#define ECM_IP_ADDR_TO_NIN6_ADDR(nin6, ipaddrt) \
	{ \
		ecm_type_check_linux_ipv6(nin6); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		nin6.in6_u.u6_addr32[3] = htonl(ipaddrt[0]); \
		nin6.in6_u.u6_addr32[2] = htonl(ipaddrt[1]); \
		nin6.in6_u.u6_addr32[1] = htonl(ipaddrt[2]); \
		nin6.in6_u.u6_addr32[0] = htonl(ipaddrt[3]); \
	}

/*
 * This macro converts from Linux IPv6 address (host order) to ECM ip_addr_t
 */
#define ECM_HIN6_ADDR_TO_IP_ADDR(ipaddrt, hin6) \
	{ \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		ecm_type_check_linux_ipv6(hin6); \
		ipaddrt[0] = in6.in6_u.u6_addr32[0]; \
		ipaddrt[1] = in6.in6_u.u6_addr32[1]; \
		ipaddrt[2] = in6.in6_u.u6_addr32[2]; \
		ipaddrt[3] = in6.in6_u.u6_addr32[3]; \
	}

/*
 * This macro converts from ECM ip_addr_t to Linux IPv6 address (host order)
 */
#define ECM_IP_ADDR_TO_HIN6_ADDR(hin6, ipaddrt) \
	{ \
		ecm_type_check_linux_ipv6(hin6); \
		ecm_type_check_ecm_ip_addr(ipaddrt); \
		in6.in6_u.u6_addr32[3] = ipaddrt[3]; \
		in6.in6_u.u6_addr32[2] = ipaddrt[2]; \
		in6.in6_u.u6_addr32[1] = ipaddrt[1]; \
		in6.in6_u.u6_addr32[0] = ipaddrt[0]; \
	}
#endif

/*
 * ecm_mac_addr_equal()
 *	Compares two MAC addresses.
 */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,6,0))
static inline unsigned ecm_mac_addr_equal(const u8 *addr1, const u8 *addr2)
{
	return compare_ether_addr(addr1, addr2);
}
#else
static inline bool ecm_mac_addr_equal(const u8 *addr1, const u8 *addr2)
{
	return !ether_addr_equal(addr1, addr2);
}
#endif

/*
 * ecm_ip_addr_is_non_unicast()
 *	Returns true if the IP address is not unicast
 */
static inline bool ecm_ip_addr_is_non_unicast(ip_addr_t addr)
{
	uint32_t v4_addr = addr[0];

#ifdef ECM_IPV6_ENABLE
	if (!ECM_IP_ADDR_IS_V4(addr)) {
		/*
		 * IPv6
		 * ff00::0/8 are multicast
		 */
		if (ECM_IP_ADDR_IS_NULL(addr) || ((addr[3] & 0xff000000) == 0xff000000)) {
			return true;
		}
		return false;
	}
#endif

	if (ECM_IP_ADDR_IS_NULL(addr) || ((v4_addr & 0xe0000000) == 0xe0000000)) {
		return true;
	}
	return false;
}

/*
 * ecm_ip_addr_is_multicast()
 *	Returns true if the IP address is multicast
 */
static inline bool ecm_ip_addr_is_multicast(ip_addr_t addr)
{
	uint32_t v4_addr = addr[0];

#ifdef ECM_IPV6_ENABLE
	if (!ECM_IP_ADDR_IS_V4(addr)) {
		/*
		 * IPv6
		 * ff00::0/8 are multicast
		 */
		if (((addr[3] & 0xff000000) == 0xff000000)) {
			return true;
		}
		return false;
	}
#endif

	if (((v4_addr & 0xf0000000) == 0xe0000000)) {
		return true;
	}
	return false;
}

#ifdef ECM_MULTICAST_ENABLE
/*
 * ecm_translate_multicast_mac()
 * 	Create the multicast MAC address given a multicast IP address
 */
static inline void ecm_translate_multicast_mac(ip_addr_t addr, unsigned char multicast_mac[])
{
	uint32_t ip_addr = addr[0];

	if (ECM_IP_ADDR_IS_V4(addr)) {
		multicast_mac[0] = 0x01;
		multicast_mac[1] = 0x00;
		multicast_mac[2] = 0x5e;
		multicast_mac[3] = (ip_addr & 0x7f0000) >> 16;
		multicast_mac[4] = (ip_addr & 0xff00) >> 8;
		multicast_mac[5] = (ip_addr & 0xff);
		return;
	}

	multicast_mac[0] = 0x33;
	multicast_mac[1] = 0x33;
	multicast_mac[2] = (ip_addr & 0xff000000) >> 24;
	multicast_mac[3] = (ip_addr & 0xff0000) >> 16;
	multicast_mac[4] = (ip_addr & 0xff00) >> 8;
	multicast_mac[5] = (ip_addr & 0xff);
}
#endif

/*
 * ecm_ip_addr_in_range()
 *	Tests if a is >= s && <= e
 */
static inline bool ecm_ip_addr_in_range(ip_addr_t a, ip_addr_t s, ip_addr_t e)
{
	if (a[3] > s[3]) goto test_end;
	if (a[3] < s[3]) return false;
	/* a[3] == s[3] */
	if (a[2] > s[2]) goto test_end;
	if (a[2] < s[2]) return false;
	/* a[2] == s[2] */
	if (a[1] > s[1]) goto test_end;
	if (a[1] < s[1]) return false;
	/* a[1] == s[1] */
	if (a[0] > s[0]) goto test_end;
	if (a[0] < s[0]) return false;
	/* a[0] == s[0] */

test_end:
	;
	if (a[3] > e[3]) return false;
	if (a[3] < e[3]) return true;
	/* a[3] == e[3] */
	if (a[2] > e[2]) return false;
	if (a[2] < e[2]) return true;
	/* a[2] == e[2] */
	if (a[1] > e[1]) return false;
	if (a[1] < e[1]) return true;
	/* a[1] == e[1] */
	if (a[0] > e[0]) return false;
	/* a[0] <= e[0] */
	return true;
}

/*
 * ecm_ip_addr_to_string()
 *	Converts the given address to a string either as octal or dotted format
 *
 * Supplied buffer should be at least 40 bytes long.
 */
static inline void ecm_ip_addr_to_string(char *str, ip_addr_t a)
{
#ifdef ECM_IPV6_ENABLE
	if (!ECM_IP_ADDR_IS_V4(a)) {
		snprintf(str, ECM_IP_ADDR_STR_BUFF_SIZE, ECM_IP_ADDR_OCTAL_FMT, ECM_IP_ADDR_TO_OCTAL(a));
		return;
	}
#endif

	snprintf(str, ECM_IP_ADDR_DOT_FMT_STR_BUFF_SIZE, ECM_IP_ADDR_DOT_FMT, ECM_IP_ADDR_TO_DOT(a));
}

/*
 * ecm_string_to_ip_addr()
 *	Convert a string IP address to its ECM ip_addr_t representation
 */
static inline bool ecm_string_to_ip_addr(ip_addr_t addr, char *ip_str)
{
	struct in6_addr dbuf;
	uint8_t *dptr = dbuf.s6_addr;
	if (in4_pton(ip_str, -1, dptr, '\0', NULL) > 0) {
		/*
		 * IPv4
		 */
		ECM_NIN4_ADDR_TO_IP_ADDR(addr, dbuf.s6_addr[0]);
		return true;
	}
#ifdef ECM_IPV6_ENABLE
	if (in6_pton(ip_str, -1, dptr, '\0', NULL) > 0) {
		/*
		 * IPv6
		 */
		ECM_NIN6_ADDR_TO_IP_ADDR(addr, dbuf);
		return true;
	}
#endif
	return false;
}

/*
 * The following are debug macros used throughout the ECM.
 * Each file that #includes this file MUST have a:
 *
 * #define DEBUG_LEVEL X
 *
 * before the inclusion of this file.
 * X is:
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 * NOTE: But X is usually provided by a -D preprocessor defined in the Makefile
 */
#if (DEBUG_LEVEL < 1)
#define DEBUG_ASSERT(s, ...)
#define DEBUG_ERROR(s, ...)
#define DEBUG_CHECK_MAGIC(i, m, s, ...)
#define DEBUG_SET_MAGIC(i, m)
#define DEBUG_CLEAR_MAGIC(i)
#define DEBUG_ECM_IP_ADDR_TO_STRING(addr_str, addr)
#else
#define DEBUG_ASSERT(c, s, ...) if (!(c)) { pr_emerg("ASSERT: %s:%d:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG(); }
#define DEBUG_ERROR(s, ...) pr_err("%s:%d:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG_CHECK_MAGIC(i, m, s, ...) if (i->magic != m) { DEBUG_ASSERT(false, s, ##__VA_ARGS__); }
#define DEBUG_SET_MAGIC(i, m) i->magic = m
#define DEBUG_CLEAR_MAGIC(i) i->magic = 0
#define DEBUG_ECM_IP_ADDR_TO_STRING(addr_str, addr) ecm_ip_addr_to_string(addr_str, addr);
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define DEBUG_WARN(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG_INFO(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG_TRACE(s, ...) pr_debug("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels
 */
#if (DEBUG_LEVEL < 2)
#define DEBUG_WARN(s, ...)
#else
#define DEBUG_WARN(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (DEBUG_LEVEL < 3)
#define DEBUG_INFO(s, ...)
#else
#define DEBUG_INFO(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (DEBUG_LEVEL < 4)
#define DEBUG_TRACE(s, ...)
#else
#define DEBUG_TRACE(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif
#endif
