#ifndef SRC_MOD_COMMON_RFC6052_H_
#define SRC_MOD_COMMON_RFC6052_H_

/**
 * @file
 * The algorithm defined in RFC 6052 (http://tools.ietf.org/html/rfc6052).
 */

#include <linux/types.h>
#include <linux/in.h>
#include <linux/in6.h>
#include "common/config.h"


/**
 * Translates @src into an IPv4 address and returns it as @dst.
 *
 * In other words, removes @prefix from @src. The result will be 32 bits of
 * address.
 */
int __rfc6052_6to4(struct ipv6_prefix const *prefix, struct in6_addr const *src,
		struct in_addr *dst);

/**
 * Translates @src into an IPv6 address and returns it as @dst.
 *
 * In other words, adds @prefix to @src. The result will be 128 bits of address.
 */
int __rfc6052_4to6(struct ipv6_prefix const *prefix, struct in_addr const *src,
		struct in6_addr *dst);

int rfc6052_6to4(struct ipv6_prefix const *prefix, struct in6_addr const *src,
		struct result_addrxlat64 *dst);
int rfc6052_4to6(struct ipv6_prefix const *prefix, struct in_addr const *src,
		struct result_addrxlat46 *dst);

#endif /* SRC_MOD_COMMON_RFC6052_H_ */
