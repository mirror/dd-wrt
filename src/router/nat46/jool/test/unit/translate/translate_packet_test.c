#include <linux/module.h>
#include <linux/printk.h>

#include "framework/unit_test.h"
#include "framework/skb_generator.h"
#include "framework/types.h"
#include "mod/common/db/global.h"
#include "mod/common/rfc7915/core.c"
#include "mod/common/rfc7915/6to4.c"
#include "mod/common/rfc7915/4to6.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva Popper");
MODULE_DESCRIPTION("Translating the Packet module test.");

xlator_type xlator_get_type(struct xlator const *instance)
{
	return XT_SIIT;
}

static bool test_function_has_unexpired_src_route(void)
{
	struct iphdr *hdr = kmalloc(60, GFP_ATOMIC); /* 60 is the max value allowed by hdr.ihl. */
	unsigned char *options;
	bool success = true;

	if (!hdr) {
		log_err("Can't allocate a test header.");
		return false;
	}
	options = (unsigned char *) (hdr + 1);

	hdr->ihl = 5; /* min legal value. */
	success &= ASSERT_BOOL(false, has_unexpired_src_route(hdr), "No options");

	hdr->ihl = 6;
	options[0] = IPOPT_SID;
	options[1] = 4;
	options[2] = 0xAB;
	options[3] = 0xCD;
	success = ASSERT_BOOL(false, has_unexpired_src_route(hdr), "No source route option, simple");

	hdr->ihl = 9;
	options[0] = IPOPT_RR; /* Record route option */
	options[1] = 11;
	options[2] = 8;
	options[3] = 0x12;
	options[4] = 0x34;
	options[5] = 0x56;
	options[6] = 0x78;
	options[7] = 0x00;
	options[8] = 0x00;
	options[9] = 0x00;
	options[10] = 0x00;
	options[11] = IPOPT_NOOP; /* No operation option. */
	options[12] = IPOPT_NOOP; /* No operation option. */
	options[13] = IPOPT_END; /* End of options list option. */
	/* Leave the rest as garbage. */
	success &= ASSERT_BOOL(false, has_unexpired_src_route(hdr), "No source option, multiple options");

	hdr->ihl = 9;
	options[0] = IPOPT_LSRR;
	options[1] = 15;
	options[2] = 16;
	options[3] = 0x11; /* First address */
	options[4] = 0x11;
	options[5] = 0x11;
	options[6] = 0x11;
	options[7] = 0x22; /* Second address */
	options[8] = 0x22;
	options[9] = 0x22;
	options[10] = 0x22;
	options[11] = 0x33; /* Third address */
	options[12] = 0x33;
	options[13] = 0x33;
	options[14] = 0x33;
	options[15] = IPOPT_END;
	success &= ASSERT_BOOL(false, has_unexpired_src_route(hdr), "Expired source route");

	options[2] = 4;
	success &= ASSERT_BOOL(true, has_unexpired_src_route(hdr), "Unexpired source route, first address");
	options[2] = 8;
	success &= ASSERT_BOOL(true, has_unexpired_src_route(hdr), "Unexpired source route, second address");
	options[2] = 12;
	success &= ASSERT_BOOL(true, has_unexpired_src_route(hdr), "Unexpired source route, third address");

	hdr->ihl = 11;
	options[0] = IPOPT_NOOP;
	options[1] = IPOPT_SID;
	options[2] = 4;
	options[3] = 0xAB;
	options[4] = 0xCD;
	options[5] = IPOPT_LSRR;
	options[6] = 15;
	options[7] = 16;
	options[8] = 0x11; /* First address */
	options[9] = 0x11;
	options[10] = 0x11;
	options[11] = 0x11;
	options[12] = 0x22; /* Second address */
	options[13] = 0x22;
	options[14] = 0x22;
	options[15] = 0x22;
	options[16] = 0x33; /* Third address */
	options[17] = 0x33;
	options[18] = 0x33;
	options[19] = 0x33;
	options[20] = IPOPT_SID;
	options[21] = 4;
	options[22] = 0xAB;
	options[23] = 0xCD;
	success &= ASSERT_BOOL(false, has_unexpired_src_route(hdr), "Expired source route, multiple opts");

	options[7] = 4;
	success &= ASSERT_BOOL(true, has_unexpired_src_route(hdr), "Unexpired src route, multiple opts (1)");
	options[7] = 8;
	success &= ASSERT_BOOL(true, has_unexpired_src_route(hdr), "Unexpired src route, multiple opts (2)");
	options[7] = 12;
	success &= ASSERT_BOOL(true, has_unexpired_src_route(hdr), "Unexpired src route, multiple opts (3)");

	kfree(hdr);
	return success;
}

static bool test_function_build_id_field(void)
{
	struct iphdr hdr;
	bool success = true;

	hdr.id = cpu_to_be16(1234);
	success &= ASSERT_BE32(1234, build_id_field(&hdr), "Simple");

	return success;
}

#define min_mtu(packet, in, out, len) be32_to_cpu(icmp6_minimum_mtu(&state, packet, out, in, len))
static bool test_function_icmp6_minimum_mtu(void)
{
	struct xlation state;
	int i;
	bool success = true;

	if (globals_init(&state.jool.globals, XT_SIIT, NULL))
		return false;

	/*
	 * I'm assuming the default plateaus list has 3 elements or more.
	 * (so I don't have to reallocate mtu_plateaus)
	 */
	state.jool.globals.plateaus.values[0] = 5000;
	state.jool.globals.plateaus.values[1] = 4000;
	state.jool.globals.plateaus.values[2] = 500;
	state.jool.globals.plateaus.count = 2;

	/* Simple tests */
	success &= ASSERT_UINT(1320, min_mtu(1300, 3000, 3000, 2000), "min(1300, 3000, 3000)");
	success &= ASSERT_UINT(1321, min_mtu(3001, 1301, 3001, 2001), "min(3001, 1301, 3001)");
	success &= ASSERT_UINT(1302, min_mtu(3002, 3002, 1302, 2002), "min(3002, 3002, 1302)");
	if (!success)
		return false;

	/* Lowest MTU is illegal on IPv6. */
	success &= ASSERT_UINT(1280, min_mtu(100, 200, 200, 150), "min(100, 200, 200)");
	success &= ASSERT_UINT(1280, min_mtu(200, 100, 200, 150), "min(200, 100, 200)");
	success &= ASSERT_UINT(1280, min_mtu(200, 200, 100, 150), "min(200, 200, 100)");

	/* Test plateaus (pkt is min). */
	for (i = 5500; i > 5000 && success; --i)
		success &= ASSERT_UINT(5020, min_mtu(0, 6000, 6000, i), "min(%d, 6000, 6000)", i);
	for (i = 5000; i > 4000 && success; --i)
		success &= ASSERT_UINT(4020, min_mtu(0, 6000, 6000, i), "min(%d, 6000, 6000)", i);
	for (i = 4000; i >= 0 && success; --i)
		success &= ASSERT_UINT(1280, min_mtu(0, 6000, 6000, i), "min(%d, 6000, 6000)", i);

	/* Test plateaus (in/out is min). */
	success &= ASSERT_UINT(1420, min_mtu(0, 1400, 5500, 4500), "min(4000,1400,5500)");
	success &= ASSERT_UINT(1400, min_mtu(0, 5500, 1400, 4500), "min(4000,5500,1400)");

	/* Plateaus and illegal MTU at the same time. */
	success &= ASSERT_UINT(1280, min_mtu(0, 700, 700, 1000), "min(500, 700, 700)");
	success &= ASSERT_UINT(1280, min_mtu(0, 1, 700, 1000), "min(500, 1, 700)");
	success &= ASSERT_UINT(1280, min_mtu(0, 700, 1, 1000), "min(500, 700, 1)");

	return success;
}
#undef min_mtu

static bool test_function_icmp4_to_icmp6_param_prob(void)
{
	static struct xlator jool;
	static struct xlation state;

	struct icmphdr *hdr4;
	struct icmp6hdr *hdr6;
	bool success = true;

	/* Annoying preparations */

	memset(&jool, 0, sizeof(jool));
	xlation_init(&state, &jool);
	if (create_skb4_icmp_error("1.1.1.1", "2.2.2.2", 10, 10, &state.in.skb))
		return false;
	if (create_skb6_icmp_error("1::1", "2::2", 10, 10, &state.out.skb))
		return false;
	hdr4 = pkt_icmp4_hdr(&state.in);
	hdr6 = pkt_icmp6_hdr(&state.out);

	/* Actual test */

	hdr4->type = ICMP_PARAMETERPROB;
	hdr4->code = ICMP_PTR_INDICATES_ERROR;
	hdr4->icmp4_unused = cpu_to_be32(0x08000000U);
	success &= ASSERT_VERDICT(CONTINUE, icmp4_to_icmp6_param_prob(&state), "func result 1");
	success &= ASSERT_UINT(ICMPV6_HDR_FIELD, hdr6->icmp6_code, "code");
	success &= ASSERT_UINT(7, be32_to_cpu(hdr6->icmp6_pointer), "pointer");

	hdr4->icmp4_unused = cpu_to_be32(0x05000000U);
	success &= ASSERT_VERDICT(DROP, icmp4_to_icmp6_param_prob(&state), "func result 2");

	return success;
}

/**
 * By the way. This test kind of looks like it should test more combinations of headers.
 * But that'd be testing the header iterator, not the build_protocol_field() function.
 * Please look elsewhere for that.
 */
static bool test_function_build_protocol_field(void)
{
	struct ipv6hdr *ip6_hdr;
	struct ipv6_opt_hdr *hop_by_hop_hdr;
	struct ipv6_opt_hdr *routing_hdr;
	struct ipv6_opt_hdr *dest_options_hdr;
	struct icmp6hdr *icmp6_hdr;

	ip6_hdr = kmalloc(sizeof(*ip6_hdr) + 8 + 16 + 24 + sizeof(struct tcphdr), GFP_ATOMIC);
	if (!ip6_hdr) {
		log_err("Could not allocate a test packet.");
		goto failure;
	}

	/* Just ICMP. */
	ip6_hdr->nexthdr = NEXTHDR_ICMP;
	ip6_hdr->payload_len = cpu_to_be16(sizeof(*icmp6_hdr));
	if (!ASSERT_UINT(IPPROTO_ICMP, xlat_proto(ip6_hdr), "Just ICMP"))
		goto failure;

	/* Skippable headers then ICMP. */
	ip6_hdr->nexthdr = NEXTHDR_HOP;
	ip6_hdr->payload_len = cpu_to_be16(8 + 16 + 24 + sizeof(*icmp6_hdr));

	hop_by_hop_hdr = (struct ipv6_opt_hdr *) (ip6_hdr + 1);
	hop_by_hop_hdr->nexthdr = NEXTHDR_ROUTING;
	hop_by_hop_hdr->hdrlen = 0; /* the hdrlen field does not include the first 8 octets. */

	routing_hdr = (struct ipv6_opt_hdr *) (((unsigned char *) hop_by_hop_hdr) + 8);
	routing_hdr->nexthdr = NEXTHDR_DEST;
	routing_hdr->hdrlen = 1;

	dest_options_hdr = (struct ipv6_opt_hdr *) (((unsigned char *) routing_hdr) + 16);
	dest_options_hdr->nexthdr = NEXTHDR_ICMP;
	dest_options_hdr->hdrlen = 2;

	if (!ASSERT_UINT(IPPROTO_ICMP, xlat_proto(ip6_hdr), "Skippable then ICMP"))
		goto failure;

	/* Skippable headers then something else */
	dest_options_hdr->nexthdr = NEXTHDR_TCP;
	ip6_hdr->payload_len = cpu_to_be16(8 + 16 + 24 + sizeof(struct tcphdr));
	if (!ASSERT_UINT(IPPROTO_TCP, xlat_proto(ip6_hdr), "Skippable then TCP"))
		goto failure;

	kfree(ip6_hdr);
	return true;

failure:
	kfree(ip6_hdr);
	return false;
}

static bool test_function_has_nonzero_segments_left(void)
{
	struct ipv6hdr *ip6_hdr;
	struct ipv6_rt_hdr *routing_hdr;
	struct frag_hdr *fragment_hdr;
	__u32 offset;

	bool success = true;

	ip6_hdr = kmalloc(sizeof(*ip6_hdr) + sizeof(*fragment_hdr) + sizeof(*routing_hdr), GFP_ATOMIC);
	if (!ip6_hdr) {
		log_err("Could not allocate a test packet.");
		return false;
	}
	ip6_hdr->payload_len = cpu_to_be16(sizeof(*fragment_hdr) + sizeof(*routing_hdr));

	/* No extension headers. */
	ip6_hdr->nexthdr = NEXTHDR_TCP;
	success &= ASSERT_BOOL(false, has_nonzero_segments_left(ip6_hdr, &offset), "No extension headers");

	if (!success)
		goto end;

	/* Routing header with nonzero segments left. */
	ip6_hdr->nexthdr = NEXTHDR_ROUTING;
	routing_hdr = (struct ipv6_rt_hdr *) (ip6_hdr + 1);
	routing_hdr->segments_left = 12;
	success &= ASSERT_BOOL(true, has_nonzero_segments_left(ip6_hdr, &offset), "Nonzero left - result");
	success &= ASSERT_UINT(40 + 3, offset, "Nonzero left - offset");

	if (!success)
		goto end;

	/* Routing header with zero segments left. */
	routing_hdr->segments_left = 0;
	success &= ASSERT_BOOL(false, has_nonzero_segments_left(ip6_hdr, &offset), "Zero left");

	if (!success)
		goto end;

	/*
	 * Fragment header, then routing header with nonzero segments left
	 * (further test the out parameter).
	 */
	ip6_hdr->nexthdr = NEXTHDR_FRAGMENT;
	fragment_hdr = (struct frag_hdr *) (ip6_hdr + 1);
	fragment_hdr->nexthdr = NEXTHDR_ROUTING;
	routing_hdr = (struct ipv6_rt_hdr *) (fragment_hdr + 1);
	routing_hdr->segments_left = 24;
	success &= ASSERT_BOOL(true, has_nonzero_segments_left(ip6_hdr, &offset), "Two headers - result");
	success &= ASSERT_UINT(40 + 8 + 3, offset, "Two headers - offset");

	/* Fall through. */
end:
	kfree(ip6_hdr);
	return success;
}

static bool test_function_icmp4_minimum_mtu(void)
{
	bool success = true;

	success &= ASSERT_UINT(2, be16_to_cpu(minimum(2, 4, 6)), "First is min");
	success &= ASSERT_UINT(8, be16_to_cpu(minimum(10, 8, 12)), "Second is min");
	success &= ASSERT_UINT(14, be16_to_cpu(minimum(16, 18, 14)), "Third is min");

	return success;
}

int init_module(void)
{
	struct test_group test = {
		.name = "Translating the Packet",
	};

	if (test_group_begin(&test))
		return -EINVAL;

	/* Misc single function tests */
	test_group_test(&test, test_function_has_unexpired_src_route, "Unexpired source route querier");
	test_group_test(&test, test_function_build_id_field, "Identification builder");
	test_group_test(&test, test_function_icmp6_minimum_mtu, "ICMP6 Minimum MTU function");
	test_group_test(&test, test_function_icmp4_to_icmp6_param_prob, "Param problem function");

	test_group_test(&test, test_function_build_protocol_field, "Build protocol function");
	test_group_test(&test, test_function_has_nonzero_segments_left, "Segments left indicator function");
	test_group_test(&test, test_function_icmp4_minimum_mtu, "ICMP4 Minimum MTU function");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
