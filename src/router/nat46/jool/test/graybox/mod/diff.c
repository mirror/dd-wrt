#include "diff.h"

#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include "log.h"

struct byte_array {
	unsigned char *array;
	size_t len;
};

enum field_type {
	FIELD_DECIMAL,
	FIELD_HEXADECIMAL,
	FIELD_ADDR6,
	FIELD_ADDR4,
};

struct field_metadata {
	char const *name;
	size_t len;
	size_t offset;
	enum field_type flags;
	bool (*equals)(struct field_metadata const *, __u8 *, __u8 *);
};

struct header_metadata {
	char const *name;
	struct header_metadata const *(*next)(void *);
	struct field_metadata const *fields;
	size_t len;
};

static const struct header_metadata ipv6h;
static const struct header_metadata ipv4h;
static const struct header_metadata tcph;
static const struct header_metadata udph;
static const struct header_metadata icmp6h;
static const struct header_metadata icmp4h;
static const struct header_metadata fragh;

static struct header_metadata const *v6_next(__u8 nexthdr)
{
	switch (nexthdr) {
	case NEXTHDR_TCP:
		return &tcph;
	case NEXTHDR_UDP:
		return &udph;
	case NEXTHDR_ICMP:
		return &icmp6h;
	case NEXTHDR_FRAGMENT:
		return &fragh;
	}

	return NULL;
}

static struct header_metadata const *ipv6h_next(void *hdr)
{
	return v6_next(((struct ipv6hdr *)hdr)->nexthdr);
}

static struct header_metadata const *ipv4h_next(void *hdr)
{
	struct iphdr *hdr4 = hdr;

	if (hdr4->frag_off & htons(IP_OFFSET))
		return NULL;

	switch (hdr4->protocol) {
	case IPPROTO_TCP:
		return &tcph;
	case IPPROTO_UDP:
		return &udph;
	case IPPROTO_ICMP:
		return &icmp4h;
	}

	return NULL;
}

static struct header_metadata const *l4h_next(void *hdr)
{
	return NULL;
}

static struct header_metadata const *icmp6h_next(void *hdr)
{
	struct icmp6hdr *hdri = hdr;

	switch (hdri->icmp6_type) {
	case ICMPV6_DEST_UNREACH:
	case ICMPV6_PKT_TOOBIG:
	case ICMPV6_TIME_EXCEED:
	case ICMPV6_PARAMPROB:
		return &ipv6h;
	default:
		return NULL;
	}
}

static struct header_metadata const *icmp4h_next(void *hdr)
{
	struct icmphdr *hdri = hdr;

	switch (hdri->type) {
	case ICMP_DEST_UNREACH:
	case ICMP_SOURCE_QUENCH:
	case ICMP_REDIRECT:
	case ICMP_TIME_EXCEEDED:
	case ICMP_PARAMETERPROB:
		return &ipv4h;
	default:
		return NULL;
	}
}

static struct header_metadata const *fragh_next(void *hdr)
{
	struct frag_hdr *fhdr = hdr;
	return (fhdr->frag_off & htons(IP6_OFFSET))
			? NULL
			: v6_next(fhdr->nexthdr);
}

/*
 * When Jool uses icmp_send() to send an error message, Linux sets the
 * precedence bits of the TOS field as 6, in likely accordance with RFC 1812,
 * section 4.3.2.5. If my reading of the code is correct, this behavior is
 * hardcoded: https://elixir.bootlin.com/linux/v4.15/source/net/ipv4/icmp.c#L693
 *
 * Since it doesn't appear as though this quirk will ever change, it'd probably
 * be a good idea to update all affected tests to expect precedence 6.
 *
 * But I already have too much on my plate during this particular refactor, so
 * in the meantime, I've decided to push a special validation function for TOS.
 */
static bool v4tos_equals(struct field_metadata const *field,
		__u8 *hdr1, __u8 *hdr2)
{
	struct iphdr *expected = (struct iphdr *)hdr1;
	struct iphdr *actual = (struct iphdr *)hdr2;

	if (actual->protocol == IPPROTO_ICMP && actual->tos == 192)
		return true; /* Set by icmp_send(), Jool can't override. */

	return expected->protocol == actual->protocol;
}

static bool v4id_equals(struct field_metadata const *field,
		__u8 *hdr1, __u8 *hdr2)
{
	struct iphdr *expected = (struct iphdr *)hdr1;
	struct iphdr *actual = (struct iphdr *)hdr2;

	if (expected->id == 0)
		return true; /* ID is meant to be random, so no validation. */

	return expected->id == actual->id;
}

static bool v4csum_equals(struct field_metadata const *field,
		__u8 *hdr1, __u8 *hdr2)
{
	struct iphdr *expected = (struct iphdr *)hdr1;
	struct iphdr *actual = (struct iphdr *)hdr2;
	size_t i;
	unsigned int csum;

	if (expected->id == 0) {
		/* ID is meant to be random, so validate checksum manually */
		csum = 0;
		for (i = 0; i < sizeof(struct iphdr); i += 2) {
			if (i == 10)
				continue; /* skip checksum itself */
			csum += (hdr2[i] << 8) + hdr2[i + 1];
			if (csum > 0xFFFF)
				csum = (csum & 0xFFFF) + 1;
		}
		expected->check = cpu_to_be16((~csum) & 0xFFFF);
	}

	return expected->check == actual->check;
}

static const struct field_metadata ipv6h_fields[] = {
	{ "version", 4, 0 },
	{ "traffic class", 8, 4 },
	{ "flow label", 20, 12 },
	{ "payload length", 16, 32 },
	{ "next header", 8, 48 },
	{ "hop limit", 8, 56 },
	{ "source address", 128, 64, FIELD_ADDR6 },
	{ "destination address", 128, 192, FIELD_ADDR6 },
	{ 0 },
};

static const struct field_metadata ipv4h_fields[] = {
	{ "version", 4, 0 },
	{ "ihl", 4, 4 },
	{ "tos", 8, 8, 0, v4tos_equals },
	{ "total length", 16, 16 },
	{ "identification", 16, 32, 0, v4id_equals },
	{ "reserved", 1, 48 },
	{ "df", 1, 49 },
	{ "mf", 1, 50 },
	{ "fragment offset", 13, 51 },
	{ "ttl", 8, 64 },
	{ "protocol", 8, 72 },
	{ "header checksum", 16, 80, FIELD_HEXADECIMAL, v4csum_equals },
	{ "source address", 32, 96, FIELD_ADDR4 },
	{ "destination address", 32, 128, FIELD_ADDR4 },
	{ 0 },
};

static const struct field_metadata tcph_fields[] = {
	{ "source port", 16, 0 },
	{ "destination port", 16, 16 },
	{ "seqnum", 32, 32 },
	{ "acknum", 32, 64 },
	{ "data offset", 4, 96 },
	{ "reserved", 3, 100 },
	{ "ns", 1, 103 },
	{ "cwr", 1, 104 },
	{ "ece", 1, 105 },
	{ "urg", 1, 106 },
	{ "ack", 1, 107 },
	{ "psh", 1, 108 },
	{ "rst", 1, 109 },
	{ "syn", 1, 110 },
	{ "fin", 1, 111 },
	{ "window size", 16, 112 },
	{ "checksum", 16, 128, FIELD_HEXADECIMAL },
	{ "urgent pointer", 16, 144 },
	{ 0 },
};

static const struct field_metadata udph_fields[] = {
	{ "source port", 16, 0 },
	{ "destination port", 16, 16 },
	{ "length", 16, 32 },
	{ "checksum", 16, 48, FIELD_HEXADECIMAL },
	{ 0 },
};

static const struct field_metadata icmph_fields[] = {
	{ "type", 8, 0 },
	{ "code", 8, 8 },
	{ "checksum", 16, 16, FIELD_HEXADECIMAL },
	{ "icmpv6 length", 8, 32 },
	{ "icmpv4 length", 8, 40 },
	{ "nexthop mtu", 16, 48 },
	{ 0 },
};

static const struct field_metadata fragh_fields[] = {
	{ "next header", 8, 0 },
	{ "reserved", 8, 8 },
	{ "fragment offset", 13, 16 },
	{ "res", 2, 29 },
	{ "m", 1, 31 },
	{ "identification", 32, 32 },
	{ 0 },
};

static const struct header_metadata ipv6h = {
	.name = "IPv6",
	.fields = ipv6h_fields,
	.next = ipv6h_next,
	.len = sizeof(struct ipv6hdr),
};

static const struct header_metadata ipv4h = {
	.name = "IPv4",
	.fields = ipv4h_fields,
	.next = ipv4h_next,
	.len = sizeof(struct iphdr),
};

static const struct header_metadata tcph = {
	.name = "TCP",
	.fields = tcph_fields,
	.next = l4h_next,
	.len = sizeof(struct tcphdr),
};

static const struct header_metadata udph = {
	.name = "UDP",
	.fields = udph_fields,
	.next = l4h_next,
	.len = sizeof(struct udphdr),
};

static const struct header_metadata icmp6h = {
	.name = "ICMPv6",
	.fields = icmph_fields,
	.next = icmp6h_next,
	.len = sizeof(struct icmp6hdr),
};

static const struct header_metadata icmp4h = {
	.name = "ICMPv4",
	.fields = icmph_fields,
	.next = icmp4h_next,
	.len = sizeof(struct icmphdr),
};

static const struct header_metadata fragh = {
	.name = "Frag",
	.fields = fragh_fields,
	.next = fragh_next,
	.len = sizeof(struct frag_hdr),
};

static struct header_metadata const *first_meta(
		struct expected_packet const *expected,
		struct sk_buff const *actual)
{
	__u8 proto = expected->bytes[0] >> 4;

	switch (proto) {
	case 6:
		return &ipv6h;
	case 4:
		return &ipv4h;
	default:
		return NULL; /* Use payload (generic) compare function */
	}
}

static __u8 get_bit(struct field_metadata const *field, __u8 const *hdr,
		size_t bit_offset)
{
	size_t byte_offset;
	byte_offset = (field->offset + bit_offset) >> 3;
	bit_offset = (field->offset + bit_offset) & 7;
	return (hdr[byte_offset] >> (7 - bit_offset)) & 1;
}

static void print_field(char const *prefix, struct field_metadata const *field,
		__u8 *hdr)
{
	size_t i;
	unsigned int value;

	if (field->flags == FIELD_ADDR6) {
		log_info("\t\t\t%s: %pI6c", prefix, hdr + (field->offset >> 3));
		return;
	}
	if (field->flags == FIELD_ADDR4) {
		log_info("\t\t\t%s: %pI4", prefix, hdr + (field->offset >> 3));
		return;
	}

	value = 0;
	if ((field->offset & 7) == 0 && (field->len & 7) == 0) {
		for (i = 0; i < (field->len >> 3); i++)
			value = (value << 8) + hdr[(field->offset >> 3) + i];
	} else {
		for (i = 0; i < field->len; i++)
			value = (value << 1) + get_bit(field, hdr, i);
	}

	if (field->flags == FIELD_HEXADECIMAL)
		log_info("\t\t\t%s: %x", prefix, value);
	else
		log_info("\t\t\t%s: %u", prefix, value);
}

static bool __equals(struct field_metadata const *field, __u8 *hdr1, __u8 *hdr2)
{
	size_t i;
	size_t index;

	/* Byte comparison (fast path) */
	if ((field->offset & 7) == 0 && (field->len & 7) == 0) {
		for (i = 0; i < (field->len >> 3); i++) {
			index = (field->offset >> 3) + i;
			if (hdr1[index] != hdr2[index])
				return false;
		}
		return true;
	}

	/* Bit comparison (slow path) */
	for (i = 0; i < field->len; i++)
		if (get_bit(field, hdr1, i) != get_bit(field, hdr2, i))
			return false;
	return true;
}

static bool equals(struct field_metadata const *field, __u8 *hdr1, __u8 *hdr2)
{
	bool (*equals_cb)(struct field_metadata const *, __u8 *, __u8 *);

	equals_cb = field->equals ? field->equals : __equals;
	if (equals_cb(field, hdr1, hdr2))
		return true;

	log_info("\t\t%s:", field->name);
	print_field("Expected", field, hdr1);
	print_field("Actual  ", field, hdr2);
	return false;
}

static unsigned int compare_payload(struct expected_packet const *expected,
		struct sk_buff const *actual, size_t offset)
{
	unsigned char *buffer;
	unsigned int errors;
	size_t remaining;
	size_t extracted;
	size_t i;
	int error;

	log_info("\t-- Payload --");

	buffer = kmalloc(256, GFP_ATOMIC);
	if (!buffer)
		return 1;

	remaining = min(expected->bytes_len - offset, actual->len - offset);
	errors = 0;
	while (remaining) {
		extracted = min(remaining, (size_t)256);
		error = skb_copy_bits(actual, offset, buffer, extracted);
		if (error) {
			log_err("skb_copy_bits() = %d; can't compare packets.",
					error);
			kfree(buffer);
			return errors + 1;
		}

		for (i = 0; i < extracted; i++) {
			if (expected->bytes[offset + i] != buffer[i]) {
				if (!errors)
					log_info("\t\tValue\tExpected\tActual");
				log_info("\t\tbyte %zu\t0x%x\t    0x%x",
						offset + i,
						expected->bytes[offset + i],
						buffer[i]);
				errors++;
			}
		}

		offset += extracted;
		remaining -= extracted;
	}

	kfree(buffer);
	return errors;
}

unsigned int collect_errors(struct expected_packet const *expected,
		struct sk_buff const *actual)
{
	struct header_metadata const *hdr_meta;
	struct byte_array hdra;
	size_t hdr_offset;
	size_t tmp;
	struct field_metadata const *field;
	unsigned int errors;
	int error;

	hdr_meta = first_meta(expected, actual);
	hdr_offset = 0;
	errors = 0;

	while (hdr_meta) {
		log_info("\t-- %s --", hdr_meta->name);

		hdra.len = min(actual->len - hdr_offset, hdr_meta->len);
		if (hdra.len == 0)
			return errors;

		hdra.array = kmalloc(hdra.len, GFP_ATOMIC);
		if (!hdra.array)
			return errors + 1;
		error = skb_copy_bits(actual, hdr_offset, hdra.array, hdra.len);
		if (error) {
			log_err("skb_copy_bits() = %d; can't compare packets.",
					error);
			kfree(hdra.array);
			return errors + 1;
		}

		for (field = hdr_meta->fields; field->name; field++) {
			tmp = field->offset + field->len;
			tmp = (tmp >> 3) + (tmp & 7) ? 1 : 0;
			if (hdr_offset + tmp > expected->bytes_len) {
				log_err("Expected's %s is trimmed.", field->name);
				return errors + 1;
			}
			if (hdr_offset + tmp > actual->len) {
				log_err("Actual's %s is trimmed.", field->name);
				return errors + 1;
			}

			if (!equals(field, &expected->bytes[hdr_offset], hdra.array))
				errors++;
		}

		kfree(hdra.array);
		tmp = hdr_offset;
		hdr_offset += hdr_meta->len;
		hdr_meta = hdr_meta->next(&expected->bytes[tmp]);
	}

	return errors + compare_payload(expected, actual, hdr_offset);
}
