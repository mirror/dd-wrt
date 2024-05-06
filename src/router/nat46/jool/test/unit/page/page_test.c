#include <linux/module.h>

#include "framework/unit_test.h"
#include "framework/send_packet.h"
#include "framework/skb_generator.h"
#include "framework/types.h"

#include "mod/common/core.h"
#include "mod/common/xlator.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("Pages test");

static struct xlator jool;

static int init(void)
{
	struct ipv6_prefix pool6;
	int error;

	pool6.len = 96;
	error = str_to_addr6("2001:db8::", &pool6.addr);
	if (error)
		return error;

	return xlator_add(XF_NETFILTER | XT_SIIT, INAME_DEFAULT, &pool6, &jool);
}

static void clean(void)
{
	xlator_put(&jool);
	xlator_rm(XT_SIIT, INAME_DEFAULT);
}

static bool validate_skb(struct sk_buff *skb, int payload_offset,
		int payload_len)
{
	u8 expected[256];
	u8 actual[256];
	int copied;
	unsigned int i;
	bool success = true;

	for (i = 0; i < 256; i++)
		expected[i] = i;

	success &= ASSERT_INT(payload_offset + payload_len, skb->len, "length");

	while (payload_len > 0) {
		copied = min(256, payload_len);

		if (skb_copy_bits(skb, payload_offset, actual, copied)) {
			log_err("Buffer extraction failed.");
			return false;
		}

		success &= ASSERT_INT(0, memcmp(expected, actual, copied),
				"byte comparison (payload offset %d)",
				payload_offset);

		payload_offset += 256;
		payload_len -= 256;
	}

	return success;
}

static int add_v6_hdr(struct sk_buff *skb, int *offset, u8 proto, u16 plen)
{
	return init_ipv6_hdr(skb, offset, "2001:db8::192.0.2.1",
			"2001:db8::203.0.113.2", plen, proto, 64);
}

static int add_v4_hdr(struct sk_buff *skb, int *offset, u8 proto, u16 plen)
{
	return init_ipv4_hdr(skb, offset, "192.0.2.1", "203.0.113.8", plen,
			proto, 32);
}

static int add_tcp_hdr(struct sk_buff *skb, int *offset, u16 dlen)
{
	return init_tcp_hdr(skb, offset, 5000, 6000, dlen);
}

static int add_icmp6_hdr(struct sk_buff *skb, unsigned int *offset)
{
	return init_icmp6_hdr_error(skb, offset, 0, 0, 0);
}

static int add_icmp4_hdr(struct sk_buff *skb, unsigned int *offset)
{
	return init_icmp4_hdr_error(skb, offset, 0, 0, 0);
}

static struct sk_buff *create_paged_skb(unsigned int head_len,
		unsigned int data_len)
{
	struct sk_buff *skb;
	unsigned int reserved_len = LL_MAX_HEADER;
	int error = 0;

	/*
	 * It's weird; @error pretty much always returns -ENOBUFS.
	 * This seems to be by design.
	 *
	 * Also, I can't for the life of me understand wtf the third argument
	 * is supposed to be used for. All I know is that it seems to create
	 * pages larger than PAGE_SIZE, which I'd expect to be illegal.
	 */
	skb = alloc_skb_with_frags(reserved_len + head_len, data_len, 0, &error,
			GFP_KERNEL);
	if (!skb)
		return NULL;

	/* One wonders why this is not part of alloc_skb_with_frags(), FFS */
	skb_reserve(skb, reserved_len);
	skb_put(skb, head_len);
	skb->data_len = data_len;
	skb->len += data_len;

	return skb;
}

static struct sk_buff *create_paged_tcp_skb(unsigned int head_len,
		unsigned int data_len)
{
	struct sk_buff *skb;
	unsigned int payload_len;
	int offset = 0;

	skb = create_paged_skb(head_len, data_len);
	if (!skb)
		return NULL;

	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));

	payload_len = head_len + data_len - sizeof(struct ipv6hdr);
	if (add_v6_hdr(skb, &offset, NEXTHDR_TCP, payload_len))
		goto abort;
	if (add_tcp_hdr(skb, &offset, payload_len))
		goto abort;
	if (init_payload_normal(skb, &offset))
		goto abort;

	return skb;

abort:
	kfree_skb(skb);
	return NULL;
}

static bool basic_single_test(unsigned int head_len, unsigned int data_len)
{
	static struct xlation state; /* Too large for the stack */
	const int IN_HDRS = sizeof(struct ipv6hdr) + sizeof(struct tcphdr);
	const int OUT_HDRS = sizeof(struct iphdr) + sizeof(struct tcphdr);
	struct sk_buff *skb_in;
	verdict result;
	bool success = true;

	if (head_len + data_len < IN_HDRS)
		return true; /* Invalid test, but we don't care */

	xlation_init(&state, &jool);
	log_debug(&state, "Test: %u %u", head_len, data_len);

	skb_in = create_paged_tcp_skb(head_len, data_len);
	if (!skb_in)
		return false;

	result = core_6to4(skb_in, &state);
	if (result != VERDICT_STOLEN)
		kfree_skb(skb_in);

	success &= ASSERT_VERDICT(STOLEN, result, "full xlat");

	if (skb_out == NULL) {
		log_err("skb_out is null.");
		return false;
	}

	success &= validate_skb(skb_out, OUT_HDRS,
			head_len + data_len - IN_HDRS);

	kfree_skb(skb_out);
	skb_out = NULL;

	return success;
}

static bool basic_test(void)
{
	unsigned int data_lens[] = { 0, 1, 2, 3, 4, 6, 7, 8, 9 };

	unsigned int h, d; /* head counter, data[_len] counter */

	for (h = 40; h < 110; h += 20)
		for (d = 0; d < ARRAY_SIZE(data_lens); d++)
			if (!basic_single_test(h, data_lens[d]))
				return false;

	return true;
}

static struct sk_buff *create_paged_icmp6err_skb(unsigned int head_len,
		unsigned int data_len)
{
	struct sk_buff *skb;
	unsigned int payload_len;
	int offset = 0;

	skb = create_paged_skb(head_len, data_len);
	if (!skb)
		return NULL;

	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));

	payload_len = head_len + data_len - sizeof(struct ipv6hdr);
	if (add_v6_hdr(skb, &offset, NEXTHDR_ICMP, payload_len))
		goto abort;
	if (add_icmp6_hdr(skb, &offset))
		goto abort;
	if (add_v6_hdr(skb, &offset, NEXTHDR_TCP, 1300))
		goto abort;
	if (add_tcp_hdr(skb, &offset, 1300))
		goto abort;
	if (init_payload_normal(skb, &offset))
		goto abort;

	return skb;

abort:
	kfree_skb(skb);
	return NULL;
}

static bool trim64_test(void)
{
	static struct xlation state; /* Too large for the stack */
	struct sk_buff *skb_in;
	verdict result;
	unsigned int hdrs_len;
	bool success = true;

	const unsigned int head_len = 500;
	const unsigned int data_len = 500;

	skb_in = create_paged_icmp6err_skb(head_len, data_len);
	if (!skb_in)
		return false;

	xlation_init(&state, &jool);
	result = core_6to4(skb_in, &state);
	if (result != VERDICT_STOLEN)
		kfree_skb(skb_in);

	success &= ASSERT_VERDICT(STOLEN, result, "full xlat");

	if (skb_out == NULL) {
		log_err("skb_out is null.");
		return false;
	}

	hdrs_len = sizeof(struct iphdr) + sizeof(struct icmphdr)
			+ sizeof(struct iphdr) + sizeof(struct tcphdr);
	success &= validate_skb(skb_out, hdrs_len, 576 - hdrs_len);

	kfree_skb(skb_out);
	skb_out = NULL;

	return success;
}

static struct sk_buff *create_paged_icmp4err_skb(unsigned int head_len,
		unsigned int data_len)
{
	struct sk_buff *skb;
	unsigned int payload_len;
	int offset = 0;

	skb = create_paged_skb(head_len, data_len);
	if (!skb)
		return NULL;

	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb_reset_network_header(skb);
	skb_set_transport_header(skb, sizeof(struct iphdr));

	payload_len = head_len + data_len - sizeof(struct iphdr);
	if (add_v4_hdr(skb, &offset, IPPROTO_ICMP, payload_len))
		goto abort;
	if (add_icmp4_hdr(skb, &offset))
		goto abort;
	if (add_v4_hdr(skb, &offset, IPPROTO_TCP, 1400))
		goto abort;
	if (add_tcp_hdr(skb, &offset, 1400))
		goto abort;
	if (init_payload_normal(skb, &offset))
		goto abort;

	return skb;

abort:
	kfree_skb(skb);
	return NULL;
}

static bool trim46_test(void)
{
	static struct xlation state; /* Too large for the stack */
	struct sk_buff *skb_in;
	verdict result;
	unsigned int hdrs_len;
	bool success = true;

	const unsigned int head_len = 500;
	const unsigned int data_len = 1000;

	skb_in = create_paged_icmp4err_skb(head_len, data_len);
	if (!skb_in)
		return false;

	xlation_init(&state, &jool);
	result = core_4to6(skb_in, &state);
	if (result != VERDICT_STOLEN)
		kfree_skb(skb_in);

	success &= ASSERT_VERDICT(STOLEN, result, "full xlat");

	if (skb_out == NULL) {
		log_err("skb_out is null.");
		return false;
	}

	hdrs_len = sizeof(struct ipv6hdr) + sizeof(struct icmp6hdr)
			+ sizeof(struct ipv6hdr) + sizeof(struct tcphdr);
	success &= validate_skb(skb_out, hdrs_len, 1280 - hdrs_len);

	kfree_skb(skb_out);
	skb_out = NULL;

	return success;
}

int init_module(void)
{
	struct test_group test = {
		.name = "Pages",
		.setup_fn = xlator_setup,
		.teardown_fn = xlator_teardown,
		.init_fn = init,
		.clean_fn = clean,
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, basic_test, "Basic test");

	/*
	 * These are mostly intended for testing the pskb_trim()s that can be
	 * found in ttp64_alloc_skb() and ttp46_alloc_skb().
	 */
	test_group_test(&test, trim64_test, "Trim test IPv6->IPv4");
	test_group_test(&test, trim46_test, "Trim test IPv4->IPv6");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
