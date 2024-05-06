#include "mod/common/address.h"

#include <linux/inet.h>
#include "common/types.h"
#include "mod/common/log.h"

int str_to_addr4(const char *str, struct in_addr *result)
{
	return in4_pton(str, -1, (u8 *) result, '\0', NULL) ? 0 : -EINVAL;
}

int str_to_addr6(const char *str, struct in6_addr *result)
{
	return in6_pton(str, -1, (u8 *) result, '\0', NULL) ? 0 : -EINVAL;
}

int prefix6_parse(char *str, struct ipv6_prefix *result)
{
	const char *slash_pos;

	if (in6_pton(str, -1, (u8 *)&result->addr.s6_addr, '/', &slash_pos) != 1)
		goto fail;
	if (kstrtou8(slash_pos + 1, 0, &result->len) != 0)
		goto fail;

	return 0;

fail:
	log_err("IPv6 prefix is malformed: %s.", str);
	return -EINVAL;
}

int prefix4_parse(char *str, struct ipv4_prefix *result)
{
	const char *slash_pos;

	if (strchr(str, '/') != NULL) {
		if (in4_pton(str, -1, (u8 *)&result->addr, '/', &slash_pos) != 1)
			goto fail;
		if (kstrtou8(slash_pos + 1, 0, &result->len) != 0)
			goto fail;
	} else {
		if (in4_pton(str, -1, (u8 *)&result->addr, '\0', NULL) != 1)
			goto fail;
		result->len = 32;
	}

	return 0;

fail:
	log_err("IPv4 prefix or address is malformed: %s.", str);
	return -EINVAL;
}

bool taddr6_equals(const struct ipv6_transport_addr *a,
		const struct ipv6_transport_addr *b)
{
	return addr6_equals(&a->l3, &b->l3) && (a->l4 == b->l4);
}

bool taddr4_equals(const struct ipv4_transport_addr *a,
		const struct ipv4_transport_addr *b)
{
	return addr4_equals(&a->l3, &b->l3) && (a->l4 == b->l4);
}

bool prefix6_equals(const struct ipv6_prefix *a, const struct ipv6_prefix *b)
{
	return addr6_equals(&a->addr, &b->addr) && (a->len == b->len);
}

bool prefix4_equals(const struct ipv4_prefix *a, const struct ipv4_prefix *b)
{
	return addr4_equals(&a->addr, &b->addr) && (a->len == b->len);
}

__u32 get_prefix4_mask(const struct ipv4_prefix *prefix)
{
	return ((__u64) 0xffffffffU) << (32 - prefix->len);
}

bool prefix4_contains(const struct ipv4_prefix *prefix,
		const struct in_addr *addr)
{
	__u32 maskbits = get_prefix4_mask(prefix);
	__u32 prefixbits = be32_to_cpu(prefix->addr.s_addr) & maskbits;
	__u32 addrbits = be32_to_cpu(addr->s_addr) & maskbits;
	return prefixbits == addrbits;
}

bool prefix4_intersects(const struct ipv4_prefix *p1,
		const struct ipv4_prefix *p2)
{
	return prefix4_contains(p1, &p2->addr)
			|| prefix4_contains(p2, &p1->addr);
}

__u64 prefix4_get_addr_count(const struct ipv4_prefix *prefix)
{
	return ((__u64) 1U) << (32 - prefix->len);
}

bool prefix6_contains(const struct ipv6_prefix *prefix,
		const struct in6_addr *addr)
{
	return ipv6_prefix_equal(&prefix->addr, addr, prefix->len);
}

int prefix4_validate(const struct ipv4_prefix *prefix)
{
	__u32 suffix_mask;

	if (unlikely(!prefix)) {
		log_err("Prefix is NULL.");
		return -EINVAL;
	}

	if (prefix->len > 32) {
		log_err("Prefix length %u is too high.", prefix->len);
		return -EINVAL;
	}

	suffix_mask = ~get_prefix4_mask(prefix);
	if ((be32_to_cpu(prefix->addr.s_addr) & suffix_mask) != 0) {
		log_err("'%pI4/%u' seems to have a suffix; please fix.",
				&prefix->addr, prefix->len);
		return -EINVAL;
	}

	return 0;
}

int prefix6_validate(const struct ipv6_prefix *prefix)
{
	unsigned int i;

	if (unlikely(!prefix)) {
		log_err("Prefix is NULL.");
		return -EINVAL;
	}

	if (prefix->len > 128) {
		log_err("Prefix length %u is too high.", prefix->len);
		return -EINVAL;
	}

	for (i = prefix->len; i < 128; i++) {
		if (addr6_get_bit(&prefix->addr, i)) {
			log_err("'%pI6c/%u' seems to have a suffix; please fix.",
					&prefix->addr, prefix->len);
			return -EINVAL;
		}
	}

	return 0;
}

int prefix4_validate_scope(struct ipv4_prefix *prefix, bool force)
{
	struct ipv4_prefix subnet;

	if (!force && prefix4_has_subnet_scope(prefix, &subnet)) {
		log_err("Prefix %pI4/%u intersects with subnet scoped network %pI4/%u.",
				&prefix->addr, prefix->len,
				&subnet.addr, subnet.len);
		log_err("Will cancel the operation. Use --force to ignore this validation.");
		return -EINVAL;
	}

	return 0;
}

__u32 addr4_get_bit(const struct in_addr *addr, unsigned int pos)
{
	__u32 mask = 1U << (31 - pos);
	return be32_to_cpu(addr->s_addr) & mask;
}

void addr4_set_bit(struct in_addr *addr, unsigned int pos, bool value)
{
	__u32 mask = 1U << (31 - pos);

	if (value)
		addr->s_addr |= cpu_to_be32(mask);
	else
		addr->s_addr &= cpu_to_be32(~mask);
}

__u32 addr6_get_bit(const struct in6_addr *addr, unsigned int pos)
{
	__u32 quadrant; /* As in, @addr has 4 "quadrants" of 32 bits each. */
	__u32 mask;

	/* "pos >> 5" is a more efficient version of "pos / 32". */
	quadrant = be32_to_cpu(addr->s6_addr32[pos >> 5]);
	/* "pos & 0x1FU" is a more efficient version of "pos % 32". */
	mask = 1U << (31 - (pos & 0x1FU));

	return quadrant & mask;
}

void addr6_set_bit(struct in6_addr *addr, unsigned int pos, bool value)
{
	__be32 *quadrant;
	__u32 mask;

	quadrant = &addr->s6_addr32[pos >> 5];
	mask = 1U << (31 - (pos & 0x1FU));

	if (value)
		*quadrant |= cpu_to_be32(mask);
	else
		*quadrant &= cpu_to_be32(~mask);
}

__u64 prefix4_next(const struct ipv4_prefix *prefix)
{
	return prefix4_get_addr_count(prefix)
			+ (__u64) be32_to_cpu(prefix->addr.s_addr);
}

/**
 * addr4_has_scope_subnet - returns true if @addr has low scope ("this" subnet
 * or lower), and therefore should not be translated under any circumstances.
 */
bool addr4_is_scope_subnet(const __be32 addr)
{
	/*
	 * I'm assuming private and doc networks do not belong to this category,
	 * to facilitate testing.
	 * (particularly users following the tutorials verbatim.)
	 */
	return ipv4_is_zeronet(addr)
			|| ipv4_is_loopback(addr)
			|| ipv4_is_linklocal_169(addr)
			|| ipv4_is_multicast(addr)
			|| ipv4_is_lbcast(addr);
}

/**
 * prefix4_has_subnet_scope - returns true if @prefix intersects with one of the
 * low-scoped networks ("this" subnet or lower), false otherwise.
 * If @subnet is sent, the colliding subnet is copied to it.
 */
bool prefix4_has_subnet_scope(struct ipv4_prefix *prefix,
		struct ipv4_prefix *subnet)
{
	struct ipv4_prefix subnets[] = {
			{ .addr.s_addr = cpu_to_be32(0x00000000), .len = 8 },
			{ .addr.s_addr = cpu_to_be32(0x7f000000), .len = 8 },
			{ .addr.s_addr = cpu_to_be32(0xa9fe0000), .len = 16 },
			{ .addr.s_addr = cpu_to_be32(0xe0000000), .len = 4 },
			{ .addr.s_addr = cpu_to_be32(0xffffffff), .len = 32 },
	};
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(subnets); i++) {
		if (prefix4_intersects(prefix, &subnets[i])) {
			if (subnet)
				*subnet = subnets[i];
			return true;
		}
	}

	return false;
}

int taddr6_compare(const struct ipv6_transport_addr *a1,
		const struct ipv6_transport_addr *a2)
{
	int gap;

	gap = ipv6_addr_cmp(&a1->l3, &a2->l3);
	if (gap)
		return gap;

	return ((int)a1->l4) - ((int)a2->l4);
}

int taddr4_compare(const struct ipv4_transport_addr *a1,
		const struct ipv4_transport_addr *a2)
{
	int gap;

	gap = ipv4_addr_cmp(&a1->l3, &a2->l3);
	if (gap)
		return gap;

	return ((int)a1->l4) - ((int)a2->l4);
}
