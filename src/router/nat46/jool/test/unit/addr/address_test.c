#include <linux/module.h>
#include <linux/printk.h>
#include "framework/unit_test.h"


MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("Address module test.");

static bool test_count(__u8 prefix_len, __u64 expected)
{
	struct ipv4_prefix prefix;

	get_random_bytes(&prefix.addr, sizeof(prefix.addr));
	prefix.len = prefix_len;

	return ASSERT_U64(expected, prefix4_get_addr_count(&prefix),
			"Address count of /%u", prefix_len);
}

static bool addr_count_test(void)
{
	bool success = true;

	success &= test_count(32, 1U);
	success &= test_count(31, 2U);
	success &= test_count(24, 0x100U);
	success &= test_count(16, 0x10000U);
	success &= test_count(8, 0x1000000U);
	success &= test_count(3, 0x20000000U);
	success &= test_count(2, 0x40000000U);
	success &= test_count(1, 0x80000000U);
	success &= test_count(0, 0x100000000UL);

	return success;
}

static bool test_contains(__u32 prefix_addr, __u8 prefix_len, __u32 addr,
		bool expected)
{
	struct ipv4_prefix prefix;
	struct in_addr inaddr;

	prefix.addr.s_addr = cpu_to_be32(prefix_addr);
	prefix.len = prefix_len;
	inaddr.s_addr = cpu_to_be32(addr);

	return ASSERT_BOOL(expected, prefix4_contains(&prefix, &inaddr),
			"%pI4/%u contains %pI4",
			&prefix.addr, prefix.len, &inaddr);
}

static bool contains_test(void)
{
	bool success = true;

	success &= test_contains(0x12345678U, 32, 0x12345677U, false);
	success &= test_contains(0x12345678U, 32, 0x12345678U, true);
	success &= test_contains(0x12345678U, 32, 0x12345679U, false);

	success &= test_contains(0x01020300U, 24, 0x010202FFU, false);
	success &= test_contains(0x01020300U, 24, 0x01020300U, true);
	success &= test_contains(0x01020300U, 24, 0x010203FFU, true);
	success &= test_contains(0x01020300U, 24, 0x01020400U, false);

	success &= test_contains(0x01020304U, 30, 0x01020303U, false);
	success &= test_contains(0x01020304U, 30, 0x01020304U, true);
	success &= test_contains(0x01020304U, 30, 0x01020305U, true);
	success &= test_contains(0x01020304U, 30, 0x01020306U, true);
	success &= test_contains(0x01020304U, 30, 0x01020307U, true);
	success &= test_contains(0x01020304U, 30, 0x01020308U, false);

	success &= test_contains(0x00000000U, 0, 0x00000000U, true);
	success &= test_contains(0x00000000U, 0, 0x12345678U, true);
	success &= test_contains(0x00000000U, 0, 0xFFFFFFFFU, true);

	return success;
}

int init_module(void)
{
	struct test_group test = {
		.name = "Addr",
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, addr_count_test, "Addr count");
	test_group_test(&test, contains_test, "Prefix contains");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
