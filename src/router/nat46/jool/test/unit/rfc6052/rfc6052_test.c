#include <linux/module.h>
#include <linux/printk.h>
#include <linux/inet.h>

#include "framework/unit_test.h"
#include "common/types.h"
#include "mod/common/rfc6052.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Ramiro Nava");
MODULE_DESCRIPTION("RFC 6052 module test.");

static bool test(const char *prefix6_str, const unsigned int prefix6_len,
		const char *addr4_str, const char *addr6_str)
{
	struct result_addrxlat46 v6;
	struct ipv6_prefix prefix;
	struct result_addrxlat64 v4;
	bool success = true;

	if (str_to_addr6(prefix6_str, &prefix.addr))
		return false;
	prefix.len = prefix6_len;

	/* 6 to 4 */
	if (str_to_addr6(addr6_str, &v6.addr))
		return false;

	success &= ASSERT_INT(0, rfc6052_6to4(&prefix, &v6.addr, &v4),
			"result code of %pI6c - %pI6c/%u = %s",
			&v6.addr, &prefix.addr, prefix.len, addr4_str);
	success &= ASSERT_ADDR4(addr4_str, &v4.addr, "6to4 address result");
	success &= ASSERT_UINT(AXM_RFC6052, v4.entry.method,
			"6to4 xlation method");
	success &= ASSERT_ADDR6(prefix6_str, &v4.entry.prefix6052.addr,
			"6to4 prefix");
	success &= ASSERT_UINT(prefix6_len, v4.entry.prefix6052.len,
			"6to4 prefix length");

	/* 4 to 6 */
	if (str_to_addr4(addr4_str, &v4.addr))
		return false;

	success &= ASSERT_INT(0, rfc6052_4to6(&prefix, &v4.addr, &v6),
			"result code of %pI4c + %pI6c/%u = %s",
			&v4.addr, &prefix.addr, prefix.len, addr6_str);
	success &= ASSERT_ADDR6(addr6_str, &v6.addr, "4to6 address result");
	success &= ASSERT_UINT(AXM_RFC6052, v6.entry.method,
			"4to6 xlation method");
	success &= ASSERT_ADDR6(prefix6_str, &v6.entry.prefix6052.addr,
			"4to6 prefix");
	success &= ASSERT_UINT(prefix6_len, v6.entry.prefix6052.len,
			"4to6 prefix length");

	return success;
}

/**
 * Taken from https://tools.ietf.org/html/rfc6052#section-2.4.
 */
static bool test_rfc6052_table(void)
{
	bool success = true;

	success &= test("2001:db8::", 32, "192.0.2.33",
			"2001:db8:c000:221::");
	success &= test("2001:db8:100::", 40, "192.0.2.33",
			"2001:db8:1c0:2:21::");
	success &= test("2001:db8:122::", 48, "192.0.2.33",
			"2001:db8:122:c000:2:2100::");
	success &= test("2001:db8:122:300::", 56, "192.0.2.33",
			"2001:db8:122:3c0:0:221::");
	success &= test("2001:db8:122:344::", 64, "192.0.2.33",
			"2001:db8:122:344:c0:2:2100::");
	success &= test("2001:db8:122:344::", 96, "192.0.2.33",
			"2001:db8:122:344::192.0.2.33");

	return success;
}

int init_module(void)
{
	struct test_group test = {
		.name = "rfc6052.c",
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, test_rfc6052_table, "Translation tests");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
