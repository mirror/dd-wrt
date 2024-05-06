#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include "framework/types.h"
#include "framework/unit_test.h"
#include "mod/common/db/eam.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("dhernandez");
MODULE_AUTHOR("aleiva");
MODULE_DESCRIPTION("Unit tests for the EAMT module");

static struct eam_table *eamt;

static int init(void)
{
	eamt = eamt_alloc();
	return eamt ? 0 : -ENOMEM;
}

static void clean(void)
{
	eamt_put(eamt);
}

static int __add_entry(char *addr4, __u8 len4, char *addr6, __u8 len6)
{
	struct eamt_entry new;
	int error;

	if (str_to_addr4(addr4, &new.prefix4.addr))
		return false;
	new.prefix4.len = len4;

	if (str_to_addr6(addr6, &new.prefix6.addr))
		return false;
	new.prefix6.len = len6;

	error = eamt_add(eamt, &new, true, true);
	/*
	if (error) {
		log_err("Errcode %d; I'm not going to print the tree.", error);
	} else {
		rtrie_print(eamt.tree6);
	}
	*/

	return error;
}

static bool add_test(void)
{
	bool success = true;

	/* Collision tests */
	success &= ASSERT_INT(0, __add_entry("1.0.0.4", 30, "1::c", 126), "hackless add");
	success &= ASSERT_INT(-EEXIST, __add_entry("1.0.0.4", 30, "1::c", 126), "full collision");
	success &= ASSERT_INT(-EEXIST, __add_entry("1.0.0.4", 30, "1::", 126), "4 collides");
	success &= ASSERT_INT(-EEXIST, __add_entry("1.0.0.0", 30, "1::c", 126), "6 collides");

	success &= ASSERT_INT(0, __add_entry("1.0.0.6", 31, "2::a", 127), "4 inside - other address");
	success &= ASSERT_INT(0, __add_entry("1.0.0.4", 31, "3::c", 127), "4 inside - same address");
	success &= ASSERT_INT(0, __add_entry("2.0.0.10", 31, "1::e", 127), "6 is inside - other address");
	success &= ASSERT_INT(0, __add_entry("3.0.0.12", 31, "1::c", 127), "6 is inside - same address");
	success &= ASSERT_INT(0, __add_entry("1.0.0.0", 24, "4::", 120), "4 is outside");
	success &= ASSERT_INT(0, __add_entry("4.0.0.0", 24, "1::", 120), "6 is outside");

	success &= ASSERT_INT(0, __add_entry("100.0.0.0", 30, "100::", 126), "no collision");

	/* Prefix length tests */
	success &= ASSERT_INT(-EINVAL, __add_entry("5.0.0.0", 24, "5::", 124), "bigger suffix4");
	success &= ASSERT_INT(0, __add_entry("5.0.0.0", 28, "5::", 120), "bigger suffix6");
	success &= ASSERT_INT(-EINVAL, __add_entry("6.0.0.0", 33, "6::", 128), "prefix4 too big");
	success &= ASSERT_INT(-EINVAL, __add_entry("6.0.0.0", 32, "6::", 129), "prefix6 too big");
	success &= ASSERT_INT(-EINVAL, __add_entry("7.0.0.1", 24, "7::", 120), "nonzero suffix4");
	success &= ASSERT_INT(-EINVAL, __add_entry("7.0.0.0", 24, "7::1", 120), "nonzero suffix6");

	return success;
}

static bool add_entry(char *addr4, __u8 len4, char *addr6, __u8 len6)
{
	if (__add_entry(addr4, len4, addr6, len6)) {
		log_err("The call to eamt_add() failed.");
		return false;
	}

	return true;
}

static bool test_6to4(char *addr6_str, char *addr4_str)
{
	struct in6_addr addr6;
	struct result_addrxlat64 result;
	bool success = true;

	if (str_to_addr6(addr6_str, &addr6))
		return false;

	if (addr4_str) {
		success &= ASSERT_INT(0, eamt_xlat_6to4(eamt, &addr6, &result),
				"errcode");
		success &= ASSERT_ADDR4(addr4_str, &result.addr,
				"resulting address");
		success &= ASSERT_UINT(AXM_EAMT, result.entry.method,
			"translation method");
	} else {
		success &= ASSERT_INT(-ESRCH,
				eamt_xlat_6to4(eamt, &addr6, &result),
				"errcode");
	}

	return success;
}

static bool test_4to6(char *addr4_str, char *addr6_str)
{
	struct in_addr addr4;
	struct result_addrxlat46 result;
	bool success = true;

	if (str_to_addr4(addr4_str, &addr4))
		return false;

	if (addr6_str) {
		success &= ASSERT_INT(0, eamt_xlat_4to6(eamt, &addr4, &result),
				"errcode");
		success &= ASSERT_ADDR6(addr6_str, &result.addr,
				"resulting address");
		success &= ASSERT_UINT(AXM_EAMT, result.entry.method,
				"translation method");
	} else {
		success &= ASSERT_INT(-ESRCH,
				eamt_xlat_4to6(eamt, &addr4, &result),
				"errcode");
	}

	return success;
}

static bool test(char *addr4, char *addr6)
{
	return test_6to4(addr6, addr4) && test_4to6(addr4, addr6);
}

static bool daniel_test(void)
{
	bool success = true;

	success &= test_6to4("::", NULL);
	success &= test_6to4("ffff::ffff", NULL);
	success &= test_4to6("0.0.0.0", NULL);
	success &= test_4to6("255.255.255.255", NULL);

	success &= add_entry("10.0.0.0", 30, "2001:db8::0", 126);
	success &= add_entry("10.0.0.12", 30, "2001:db8::4", 126);
	success &= add_entry("10.0.0.16", 28, "2001:db8::20", 124);
	success &= add_entry("10.0.0.254", 32, "2001:db8::111", 128);
	success &= add_entry("10.0.1.0", 24, "2001:db8::200", 120);
	if (!success)
		return false;

	success &= test("10.0.0.2", "2001:db8::2");
	success &= test("10.0.0.14", "2001:db8::6");
	success &= test("10.0.0.27", "2001:db8::2b");
	success &= test("10.0.0.254", "2001:db8::111");
	success &= test("10.0.1.15", "2001:db8::20f");

	success &= test_6to4("2001:db8::8", NULL);
	success &= test_6to4("8000::", NULL);

	/* "test first bit doesn't match root". */
	success &= test_6to4("8000::", NULL);
	success &= test_4to6("128.0.0.0", NULL);

	return success;
}

static bool rfc7757_examples_test(void)
{
	bool success = true;

	success &= add_entry("192.0.2.1", 32, "2001:db8:aaaa::", 128);
	success &= add_entry("192.0.2.2", 32, "2001:db8:bbbb::b", 128);
	success &= add_entry("192.0.2.16", 28, "2001:db8:cccc::", 124);
	success &= add_entry("192.0.2.128", 26, "2001:db8:dddd::", 64);
	success &= add_entry("192.0.2.192", 29, "2001:db8:eeee:8::", 62);
	success &= add_entry("192.0.2.224", 31, "64:ff9b::", 127);
	if (!success)
		return false;

	success &= test("192.0.2.1", "2001:db8:aaaa::");
	success &= test("192.0.2.2", "2001:db8:bbbb::b");
	success &= test("192.0.2.16", "2001:db8:cccc::");
	success &= test("192.0.2.24", "2001:db8:cccc::8");
	success &= test("192.0.2.31", "2001:db8:cccc::f");
	success &= test("192.0.2.128", "2001:db8:dddd::");
	success &= test("192.0.2.152", "2001:db8:dddd:0:6000::");
	success &= test("192.0.2.183", "2001:db8:dddd:0:dc00::");
	success &= test("192.0.2.191", "2001:db8:dddd:0:fc00::");
	success &= test("192.0.2.195", "2001:db8:eeee:9:8000::");
	success &= test("192.0.2.225", "64:ff9b::1");

	return success;
}

static bool rfc7757_overlapping_test(void)
{
	bool success = true;

	success &= add_entry("0.0.0.0", 0, "2001:db8:ff00::", 40);
	success &= add_entry("198.51.100.64", 32, "2001:db8::abcd", 128);
	if (!success)
		return false;

	success &= test_6to4("2001:db8:ffc6:3364:4000::", "198.51.100.64");
	success &= test_4to6("198.51.100.64", "2001:db8::abcd");

	return success;
}

static bool rfc7757_identical_test(void)
{
	bool success = true;

	success &= add_entry("198.51.100.8", 32, "2001:db8::1", 128);
	success &= ASSERT_INT(-EEXIST, __add_entry("198.51.100.9", 32, "2001:db8::1", 128), "full collision");

	return success;
}

static bool remove_entry(char *addr4, __u8 len4, char *addr6, __u8 len6,
		int expected_error)
{
	struct ipv4_prefix prefix4;
	struct ipv6_prefix prefix6;
	bool success;
	int error;

	if (!addr4 && !addr6) {
		log_err("Both addr4 and addr6 are NULL.");
		return false;
	}

	if (addr4) {
		if (str_to_addr4(addr4, &prefix4.addr) != 0)
			return false;
		prefix4.len = len4;
	}

	if (addr6) {
		if (str_to_addr6(addr6, &prefix6.addr) != 0)
			return false;
		prefix6.len = len6;
	}

	error = eamt_rm(eamt, addr6 ? &prefix6 : NULL, addr4 ? &prefix4 : NULL);
	success = ASSERT_INT(expected_error, error, "removing EAM entry");

	/* rtrie_print(eamt.tree6); */

	return success;
}

static bool create_two_story_trie(void)
{
	bool success = true;

	success &= add_entry("1.0.0.0", 32, "1::00", 120);
	success &= add_entry("2.0.0.0", 32, "1::10", 124);
	success &= add_entry("3.0.0.0", 32, "1::20", 124);

	success &= remove_entry(NULL, 0, "1::", 128, -ESRCH);
	success &= remove_entry(NULL, 0, "1::", 121, -ESRCH);
	success &= remove_entry(NULL, 0, "1::", 119, -ESRCH);
	success &= remove_entry(NULL, 0, "0::", 0, -ESRCH);

	success &= test("1.0.0.0", "1::00");
	success &= test("2.0.0.0", "1::10");
	success &= test("3.0.0.0", "1::20");

	return success;
}

static bool create_four_story_trie(void)
{
	bool success = true;

	success &= add_entry("1.0.0.0", 32, "1::", 16);
	success &= add_entry("2.0.0.0", 32, "1:1::", 32);
	success &= add_entry("3.0.0.0", 32, "1:2::", 32);
	success &= add_entry("4.0.0.0", 32, "1:1:1::", 48);
	success &= add_entry("5.0.0.0", 32, "1:1:2::", 48);
	success &= add_entry("6.0.0.0", 32, "1:2:1::", 48);
	success &= add_entry("7.0.0.0", 32, "1:2:1:1::", 64);

	success &= remove_entry(NULL, 0, "1::", 0, -ESRCH);
	success &= remove_entry(NULL, 0, "1::", 15, -ESRCH);
	success &= remove_entry(NULL, 0, "1::", 17, -ESRCH);
	success &= remove_entry(NULL, 0, "1::", 128, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2::", 0, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2::", 31, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2::", 33, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2::", 128, -ESRCH);
	success &= remove_entry(NULL, 0, "1:1:2::", 0, -ESRCH);
	success &= remove_entry(NULL, 0, "1:1:2::", 47, -ESRCH);
	success &= remove_entry(NULL, 0, "1:1:2::", 49, -ESRCH);
	success &= remove_entry(NULL, 0, "1:1:2::", 128, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2:1:1::", 0, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2:1:1::", 63, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2:1:1::", 65, -ESRCH);
	success &= remove_entry(NULL, 0, "1:2:1:1::", 128, -ESRCH);

	success &= test("1.0.0.0", "1::");
	success &= test("2.0.0.0", "1:1::");
	success &= test("3.0.0.0", "1:2::");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test("7.0.0.0", "1:2:1:1::");

	return success;
}

static bool remove_test(void)
{
	bool success = true;

	/* trie is empty */
	success &= remove_entry("10.0.0.0", 24, "1::", 120, -ESRCH);

	/* trie is one node high */
	success &= add_entry("20.0.0.0", 25, "2::", 121);
	success &= remove_entry("30.0.0.1", 25, NULL, 0, -ESRCH);
	success &= remove_entry("20.0.0.130", 25, NULL, 0, -ESRCH);
	success &= remove_entry("20.0.0.120", 25, NULL, 0, 0);

	success &= add_entry("30.0.0.0", 24, "3::", 120);
	success &= test("30.0.0.0", "3::");
	success &= remove_entry(NULL, 0, "3::1:0", 120, -ESRCH);
	success &= test("30.0.0.0", "3::");
	success &= remove_entry(NULL, 0, "3::0", 120, 0);
	success &= test_6to4("3::", NULL);
	success &= test_4to6("30.0.0.0", NULL);

	success &= ASSERT_U64(0ULL, eamt->count, "Table count");
	if (!success)
		return false;

	/* trie is two nodes high */
	success &= create_two_story_trie();
	success &= remove_entry(NULL, 0, "1::10", 124, 0);
	success &= test("1.0.0.0", "1::00");
	success &= test_6to4("1::10", "1.0.0.0");
	success &= test_4to6("2.0.0.0", NULL);
	success &= test("3.0.0.0", "1::20");
	eamt_flush(eamt);

	success &= create_two_story_trie();
	success &= remove_entry(NULL, 0, "1::20", 124, 0);
	success &= test("1.0.0.0", "1::00");
	success &= test("2.0.0.0", "1::10");
	success &= test_6to4("1::20", "1.0.0.0");
	success &= test_4to6("3.0.0.0", NULL);
	eamt_flush(eamt);

	success &= create_two_story_trie();
	success &= remove_entry(NULL, 0, "1::00", 120, 0);
	success &= test_6to4("1::00", NULL);
	success &= test_4to6("1.0.0.0", NULL);
	success &= test("2.0.0.0", "1::10");
	success &= test("3.0.0.0", "1::20");
	eamt_flush(eamt);

	/* trie is three or more nodes high */
	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1::", 16, 0);
	success &= test_6to4("1::", NULL);
	success &= test_4to6("1.0.0.0", NULL);
	success &= test("2.0.0.0", "1:1::");
	success &= test("3.0.0.0", "1:2::");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test("7.0.0.0", "1:2:1:1::");
	eamt_flush(eamt);

	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1:1::", 32, 0);
	success &= test("1.0.0.0", "1::");
	success &= test_6to4("1:1::", "1.0.0.0");
	success &= test("3.0.0.0", "1:2::");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test("7.0.0.0", "1:2:1:1::");
	eamt_flush(eamt);

	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1:2::", 32, 0);
	success &= test("1.0.0.0", "1::");
	success &= test("2.0.0.0", "1:1::");
	success &= test_6to4("1:2::", "1.0.0.0");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test("7.0.0.0", "1:2:1:1::");
	eamt_flush(eamt);

	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1:1:1::", 48, 0);
	success &= test("1.0.0.0", "1::");
	success &= test("2.0.0.0", "1:1::");
	success &= test("3.0.0.0", "1:2::");
	success &= test_6to4("1:1:1::", "2.0.0.0");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test("7.0.0.0", "1:2:1:1::");
	eamt_flush(eamt);

	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1:1:2::", 48, 0);
	success &= test("1.0.0.0", "1::");
	success &= test("2.0.0.0", "1:1::");
	success &= test("3.0.0.0", "1:2::");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test_6to4("1:1:2::", "2.0.0.0");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test("7.0.0.0", "1:2:1:1::");
	eamt_flush(eamt);

	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1:2:1::", 48, 0);
	success &= test("1.0.0.0", "1::");
	success &= test("2.0.0.0", "1:1::");
	success &= test("3.0.0.0", "1:2::");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test_6to4("1:2:1::", "3.0.0.0");
	success &= test("7.0.0.0", "1:2:1:1::");
	eamt_flush(eamt);

	success &= create_four_story_trie();
	success &= remove_entry(NULL, 0, "1:2:1:1::", 64, 0);
	success &= test("1.0.0.0", "1::");
	success &= test("2.0.0.0", "1:1::");
	success &= test("3.0.0.0", "1:2::");
	success &= test("4.0.0.0", "1:1:1::");
	success &= test("5.0.0.0", "1:1:2::");
	success &= test("6.0.0.0", "1:2:1::");
	success &= test_6to4("1:2:1:1::", "6.0.0.0");
	eamt_flush(eamt);

	return success;
}

static int address_mapping_test_init(void)
{
	struct test_group test = {
		.name = "Address Mapping test",
		.init_fn = init,
		.clean_fn = clean,
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, add_test, "add function");
	test_group_test(&test, daniel_test, "Daniel's xlat tests");
	test_group_test(&test, rfc7757_examples_test, "RFC 7757 Appendix B");
	test_group_test(&test, rfc7757_overlapping_test, "RFC 7757 Section 5, 1st half");
	test_group_test(&test, rfc7757_identical_test, "RFC 7757 Section 5, 2nd half");
	test_group_test(&test, remove_test, "remove function");

	return test_group_end(&test);
}

static void address_mapping_test_exit(void)
{
	/* No code. */
}

module_init(address_mapping_test_init);
module_exit(address_mapping_test_exit);
