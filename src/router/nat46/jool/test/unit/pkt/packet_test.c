#include <linux/module.h>
#include "mod/common/packet.h"

#include "mod/common/translation_state.h"
#include "framework/unit_test.h"
#include "framework/types.h"
#include "framework/skb_generator.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("Packet test");

static bool test_function_is_df_set(void)
{
	struct iphdr hdr;
	bool success = true;

	hdr.frag_off = cpu_to_be16(0x0000);
	success &= ASSERT_UINT(0, is_df_set(&hdr), "All zeroes");

	hdr.frag_off = cpu_to_be16(0x4000);
	success &= ASSERT_UINT(IP_DF, is_df_set(&hdr), "All zeroes except DF");

	hdr.frag_off = cpu_to_be16(0xFFFF);
	success &= ASSERT_UINT(IP_DF, is_df_set(&hdr), "All ones");

	hdr.frag_off = cpu_to_be16(0xBFFF);
	success &= ASSERT_UINT(0, is_df_set(&hdr), "All ones except DF");

	return success;
}

static bool test_function_is_mf_set(void)
{
	struct iphdr hdr;
	bool success = true;

	hdr.frag_off = cpu_to_be16(0x0000);
	success &= ASSERT_UINT(0, is_mf_set_ipv4(&hdr), "All zeroes");

	hdr.frag_off = cpu_to_be16(0x2000);
	success &= ASSERT_UINT(IP_MF, is_mf_set_ipv4(&hdr), "All zeroes except MF");

	hdr.frag_off = cpu_to_be16(0xFFFF);
	success &= ASSERT_UINT(IP_MF, is_mf_set_ipv4(&hdr), "All ones");

	hdr.frag_off = cpu_to_be16(0xDFFF);
	success &= ASSERT_UINT(0, is_mf_set_ipv4(&hdr), "All ones except MF");

	return success;
}

static bool test_function_build_ipv4_frag_off_field(void)
{
	bool success = true;

	success &= ASSERT_BE16(0x400F, build_ipv4_frag_off_field(1, 0, 120), "Simple 1");
	success &= ASSERT_BE16(0x202B, build_ipv4_frag_off_field(0, 1, 344), "Simple 2");

	return success;
}

static bool test_inner_validation4(void)
{
	static struct xlation state; /* Too large for the stack. */
	static struct xlator jool; /* Too large for the stack. */
	struct sk_buff *skb;
	bool result = true;

	/* Only state.result is expected, and default values are fine. */
	memset(&jool, 0, sizeof(jool));
	xlation_init(&state, &jool);

	if (create_skb4_icmp_error("1.1.1.1", "2.2.2.2", 100, 32, &skb))
		return false;
	result &= ASSERT_VERDICT(CONTINUE, pkt_init_ipv4(&state, skb), "complete inner pkt");
	kfree_skb(skb);

	if (create_skb4_icmp_error("1.1.1.1", "2.2.2.2", 30, 32, &skb))
		return false;
	result &= ASSERT_VERDICT(DROP, pkt_init_ipv4(&state, skb), "incomplete inner tcp");
	kfree_skb(skb);

	if (create_skb4_icmp_error("1.1.1.1", "2.2.2.2", 15, 32, &skb))
		return false;
	result &= ASSERT_VERDICT(DROP, pkt_init_ipv4(&state, skb), "incomplete inner ipv4");
	kfree_skb(skb);

	return result;
}

static bool test_inner_validation6(void)
{
	static struct xlation state; /* Too large for the stack. */
	static struct xlator jool; /* Too large for the stack. */
	struct sk_buff *skb;
	bool result = true;

	/* Only state.result is expected, and default values are fine. */
	memset(&jool, 0, sizeof(jool));
	xlation_init(&state, &jool);

	if (create_skb6_icmp_error("1::1", "2::2", 100, 32, &skb))
		return false;
	result &= ASSERT_VERDICT(CONTINUE, pkt_init_ipv6(&state, skb), "complete inner pkt 6");
	kfree_skb(skb);

	/* 50 = 40 + 8 + 40 + 20 (lolwut? This comment seems outdated.) */
	if (create_skb6_icmp_error("1::1", "2::2", 50, 32, &skb))
		return false;
	result &= ASSERT_VERDICT(DROP, pkt_init_ipv6(&state, skb), "incomplete inner tcp");
	kfree_skb(skb);

	if (create_skb6_icmp_error("1::1", "2::2", 30, 32, &skb))
		return false;
	result &= ASSERT_VERDICT(DROP, pkt_init_ipv6(&state, skb), "incomplete inner ipv6hdr");
	kfree_skb(skb);

	return result;
}

int init_module(void)
{
	struct test_group test = {
		.name = "Packet",
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, test_function_is_df_set, "DF getter");
	test_group_test(&test, test_function_is_mf_set, "MF getter");
	test_group_test(&test, test_function_build_ipv4_frag_off_field, "Generate frag offset + flags function");

	test_group_test(&test, test_inner_validation4, "Inner IPv4 pkt validation");
	test_group_test(&test, test_inner_validation6, "Inner IPv6 pkt validation");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
