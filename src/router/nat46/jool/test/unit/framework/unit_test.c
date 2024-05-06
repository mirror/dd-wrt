#include "framework/unit_test.h"

#include <linux/kernel.h>
#include <net/ipv6.h>

bool ASSERT_NOTNULL(void *ptr, char const *test_name)
{
	if (ptr == NULL) {
		log_err("Test '%s' failed.", test_name);
		pr_err("  Expected: !NULL\n");
		pr_err("  Actual  : NULL\n");
		return false;
	}

	return true;
}

bool __ASSERT_ADDR4(const struct in_addr *expected,
		const struct in_addr *actual,
		const char *test_name)
{
	if (expected == actual)
		return true;

	if (!expected || !actual || expected->s_addr != actual->s_addr) {
		log_err("Test '%s' failed.", test_name);
		pr_err("  Expected: %pI4\n", expected);
		pr_err("  Actual  : %pI4\n", actual);
		return false;
	}

	return true;
}

bool ASSERT_ADDR4(char const *expected_str, struct in_addr const *actual,
		char const *test_name)
{
	struct in_addr expected;

	if (!expected_str)
		return __ASSERT_ADDR4(NULL, actual, test_name);

	return str_to_addr4(expected_str, &expected)
			? false
			: __ASSERT_ADDR4(&expected, actual, test_name);
}

bool ASSERT_PREFIX4(struct ipv4_prefix const *expected,
		struct ipv4_prefix const *actual,
		char const *test_name)
{
	if (expected == actual)
		return true;

	if (!expected) {
		log_err("Test '%s' failed.", test_name);
		pr_err("  Expected: NULL\n");
		pr_err("  Actual  : %pI4/%u\n", &actual->addr, actual->len);
	}
	if (!actual) {
		log_err("Test '%s' failed.", test_name);
		pr_err("  Expected: %pI4/%u\n", &expected->addr, expected->len);
		pr_err("  Actual  : NULL\n");
	}

	return __ASSERT_ADDR4(&expected->addr, &actual->addr, test_name)
			&& ASSERT_UINT(expected->len, actual->len, "%s", test_name);
}

bool ASSERT_TADDR4(struct ipv4_transport_addr const *expected,
		struct ipv4_transport_addr const *actual,
		char const *test_name)
{
	if (taddr4_equals(expected, actual))
		return true;

	log_err("Test '%s' failed.", test_name);
	pr_err("  Expected: " TA4PP "\n", TA4PA(*expected));
	pr_err("  Actual  : " TA4PP "\n", TA4PA(*actual));
	return false;
}

bool __ASSERT_ADDR6(struct in6_addr const *expected,
		struct in6_addr const *actual,
		char const *test_name)
{
	if (expected == actual)
		return true;

	if (!expected || !actual || ipv6_addr_cmp(expected, actual)) {
		log_err("Test '%s' failed. Expected:%pI6c Actual:%pI6c",
				test_name, expected, actual);
		return false;
	}

	return true;
}

bool ASSERT_ADDR6(char const *expected_str,
		struct in6_addr const *actual,
		char const *test_name)
{
	struct in6_addr expected;

	if (!expected_str)
		return __ASSERT_ADDR6(NULL, actual, test_name);

	return str_to_addr6(expected_str, &expected)
			? false
			: __ASSERT_ADDR6(&expected, actual, test_name);
}

bool ASSERT_PREFIX6(struct ipv6_prefix const *expected,
		struct ipv6_prefix const *actual,
		char const *test_name)
{
	if (expected == actual)
		return true;

	if (!expected) {
		log_err("Test '%s' failed.", test_name);
		pr_err("  Expected: NULL\n");
		pr_err("  Actual  : %pI6c/%u\n", &actual->addr, actual->len);
	}
	if (!actual) {
		log_err("Test '%s' failed.", test_name);
		pr_err("  Expected: %pI6c/%u\n", &expected->addr, expected->len);
		pr_err("  Actual  : NULL\n");
	}

	return __ASSERT_ADDR6(&expected->addr, &actual->addr, test_name)
			&& ASSERT_UINT(expected->len, actual->len, "%s", test_name);
}

bool ASSERT_TADDR6(struct ipv6_transport_addr const *expected,
		struct ipv6_transport_addr const *actual,
		char const *test_name)
{
	if (taddr6_equals(expected, actual))
		return true;

	log_err("Test '%s' failed.", test_name);
	pr_err("  Expected: " TA6PP "\n", TA6PA(*expected));
	pr_err("  Actual  : " TA6PP "\n", TA6PA(*actual));
	return false;
}

static bool ASSERT_TUPLE4(struct tuple const *expected,
		struct tuple const *actual,
		char const *test_name)
{
	if (expected->l4_proto != actual->l4_proto)
		goto fail;
	if (ipv4_addr_cmp(&expected->src.addr4.l3, &actual->src.addr4.l3))
		goto fail;
	if (expected->src.addr4.l4 != actual->src.addr4.l4)
		goto fail;
	if (ipv4_addr_cmp(&expected->dst.addr4.l3, &actual->dst.addr4.l3))
		goto fail;
	if (expected->dst.addr4.l4 != actual->dst.addr4.l4)
		goto fail;

	return true;

fail:
	log_err("Test '%s' failed.", test_name);
	if (expected)
		pr_err("  Expected:" T4PP "\n", T4PA(expected));
	else
		pr_err("  Expected:NULL\n");
	if (actual)
		pr_err("  Actual:  " T4PP "\n", T4PA(actual));
	else
		pr_err("  Actual:  NULL\n");
	return false;
}

static bool ASSERT_TUPLE6(struct tuple const *expected,
		struct tuple const *actual,
		char const *test_name)
{
	if (expected->l4_proto != actual->l4_proto)
		goto fail;
	if (ipv6_addr_cmp(&expected->src.addr6.l3, &actual->src.addr6.l3))
		goto fail;
	if (expected->src.addr6.l4 != actual->src.addr6.l4)
		goto fail;
	if (ipv6_addr_cmp(&expected->dst.addr6.l3, &actual->dst.addr6.l3))
		goto fail;
	if (expected->dst.addr6.l4 != actual->dst.addr6.l4)
		goto fail;

	return true;

fail:
	log_err("Test '%s' failed.", test_name);
	if (expected)
		pr_err("  Expected:" T6PP "\n", T6PA(expected));
	else
		pr_err("  Expected:NULL\n");
	if (actual)
		pr_err("  Actual:  " T6PP "\n", T6PA(actual));
	else
		pr_err("  Actual:  NULL\n");
	return false;
}

bool ASSERT_TUPLE(struct tuple const *expected,
		struct tuple const *actual,
		char const *test_name)
{
	if (expected->l3_proto != actual->l3_proto) {
		log_err("Test '%s' failed; Expected:%u Actual:%u", test_name,
				expected->l3_proto, actual->l3_proto);
		return false;
	}

	switch (expected->l3_proto) {
	case L3PROTO_IPV4:
		return ASSERT_TUPLE4(expected, actual, test_name);
	case L3PROTO_IPV6:
		return ASSERT_TUPLE6(expected, actual, test_name);
	}

	log_err("?");
	return false;
}

bool ASSERT_BIB(struct bib_entry const* expected,
		struct bib_entry const* actual,
		char const *test_name)
{
	if (expected == actual)
		return true;

	if (!expected) {
		log_err("Test '%s' failed:", test_name);
		pr_err("  Expected: NULL\n");
		pr_err("  Actual  : BIB " BEPP "\n", BEPA(actual));
		return false;
	}
	if (!actual) {
		log_err("Test '%s' failed:", test_name);
		pr_err("  Expected: BIB " BEPP "\n", BEPA(expected));
		pr_err("  Actual  : NULL\n");
		return false;
	}

	if (!taddr4_equals(&expected->addr4, &actual->addr4)
			|| !taddr6_equals(&expected->addr6, &actual->addr6)) {
		log_err("Test '%s' failed:", test_name);
		pr_err("  Expected: BIB " BEPP "\n", BEPA(expected));
		pr_err("  Actual  : BIB " BEPP "\n", BEPA(actual));
		return false;
	}

	return true;
}

bool ASSERT_SESSION(struct session_entry const *expected,
		struct session_entry const *actual,
		char const *test_name)
{
	if (expected == actual)
		return true;
	if (!expected || !actual)
		goto fail;

	if (expected->proto != actual->proto)
		goto fail;
	if (!taddr6_equals(&expected->src6, &actual->src6))
		goto fail;
	if (!taddr6_equals(&expected->dst6, &actual->dst6))
		goto fail;
	if (!taddr4_equals(&expected->src4, &actual->src4))
		goto fail;
	if (!taddr4_equals(&expected->dst4, &actual->dst4))
		goto fail;

	return true;

fail:
	log_err("Test '%s' failed.", test_name);
	if (expected)
		pr_err("  Expected: Session " SEPP "\n", SEPA(expected));
	else
		pr_err("  Expected: NULL\n");
	if (actual)
		pr_err("  Actual:   Session " SEPP "\n", SEPA(actual));
	else
		pr_err("  Actual:   NULL\n");
	return false;
}

void print_session(struct session_entry *session)
{
	pr_cont(SEPP, SEPA(session));
}

int test_group_begin(struct test_group *group)
{
	log_info("Module '%s': Starting tests...", group->name);
	return group->setup_fn ? group->setup_fn() : 0;
}

void test_group_test(struct test_group *group, bool (*test)(void),
		char *name)
{
	bool success;

	log_info("Test '%s': Starting...", name);
	group->test_counter++;

	if (group->init_fn && group->init_fn()) {
		group->failure_counter++;
		return;
	}

	success = test();
	if (!success)
		group->failure_counter++;

	if (group->clean_fn)
		group->clean_fn();

	log_info("Test '%s': %s.\n", name, success ? "Success" : "Failure");
}

int test_group_end(struct test_group *group)
{
	if (group->teardown_fn)
		group->teardown_fn();

	log_info("Finished. Runs: %d; Errors: %d",
			group->test_counter,
			group->failure_counter);

	return (group->failure_counter > 0) ? -EINVAL : 0;
}

int broken_unit_call(const char *function)
{
	WARN(true, "%s() was called! The unit test is broken.", function);
	return -EINVAL;
}
