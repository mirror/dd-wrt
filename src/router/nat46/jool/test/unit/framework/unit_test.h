#ifndef _JOOL_UNIT_TEST_H
#define _JOOL_UNIT_TEST_H

#include "mod/common/log.h"
#include "mod/common/types.h"
#include "mod/common/db/bib/entry.h"

#define ASSERT_PRIMITIVE(expected, actual, specifier, name, ...) ({	\
		/* don't want these to be evaluated multiple times. */	\
		typeof(expected) __expected = expected;			\
		typeof(actual) __actual = actual;			\
		if (__expected != __actual) {				\
			log_err("Test '" name "' failed.", ##__VA_ARGS__); \
			pr_err("  Expected: " specifier "\n", __expected); \
			pr_err("  Actual  : " specifier "\n", __actual); \
		}							\
		__expected == __actual;					\
	})

/* https://www.kernel.org/doc/Documentation/printk-formats.txt */
#define ASSERT_UINT(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, actual, "%u", name, ##__VA_ARGS__)
#define ASSERT_INT(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, actual, "%d", name, ##__VA_ARGS__)
#define ASSERT_BOOL(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, actual, "%u", name, ##__VA_ARGS__)
#define ASSERT_ULONG(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, actual, "%lu", name, ##__VA_ARGS__)
#define ASSERT_U64(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, actual, "%llu", name, ##__VA_ARGS__)
#define ASSERT_PTR(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, actual, "%p", name, ##__VA_ARGS__)
#define ASSERT_BE16(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, be16_to_cpu(actual), "%u", name, \
				##__VA_ARGS__)
#define ASSERT_BE32(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(expected, be32_to_cpu(actual), "%u", name, \
				##__VA_ARGS__)
#define ASSERT_VERDICT(expected, actual, name, ...) \
		ASSERT_PRIMITIVE(VERDICT_ ## expected, actual, "%d", name, \
				##__VA_ARGS__)

#define ASSERT_TRUE(actual, name, ...) \
	ASSERT_BOOL(true, actual, name, ##__VA_ARGS__)
#define ASSERT_FALSE(actual, name, ...) \
	ASSERT_BOOL(false, actual, name, ##__VA_ARGS__)
#define ASSERT_NULL(actual, name, ...) \
	ASSERT_PTR(NULL, actual, name, ##__VA_ARGS__)

/*
 * Ehh... there aren't macros, but they're still all caps so they're even
 * easier to recognize.
 */

bool ASSERT_NOTNULL(void *ptr, char const *test_name);
bool ASSERT_ADDR4(char const *expected,
		struct in_addr const *actual,
		char const *test_name);
bool __ASSERT_ADDR4(struct in_addr const *expected,
		struct in_addr const *actual,
		char const *test_name);
bool ASSERT_PREFIX4(struct ipv4_prefix const *expected,
		struct ipv4_prefix const *actual,
		char const *test_name);
bool ASSERT_TADDR4(struct ipv4_transport_addr const *expected,
		struct ipv4_transport_addr const *actual,
		char const *test_name);
bool ASSERT_ADDR6(char const *expected,
		struct in6_addr const *actual,
		char const *test_name);
bool __ASSERT_ADDR6(struct in6_addr const *expected,
		struct in6_addr const *actual,
		char const *test_name);
bool ASSERT_PREFIX6(struct ipv6_prefix const *expected,
		struct ipv6_prefix const *actual,
		char const *test_name);
bool ASSERT_TADDR6(struct ipv6_transport_addr const *expected,
		struct ipv6_transport_addr const *actual,
		char const *test_name);
bool ASSERT_TUPLE(struct tuple const *expected,
		struct tuple const *actual,
		char const *test_name);
bool ASSERT_BIB(struct bib_entry const *expected,
		struct bib_entry const *actual,
		char const *test_name);
bool ASSERT_SESSION(struct session_entry const *expected,
		struct session_entry const *actual,
		char const *test_name);

void print_session(struct session_entry *session);

struct test_group {
	char *name;

	/** To be run once per test group. */
	int (*setup_fn)(void);
	/** Reverts @setup_fn. */
	void (*teardown_fn)(void);
	/** To be run once per test. */
	int (*init_fn)(void);
	/** Reverts @init_fn. */
	void (*clean_fn)(void);

	unsigned int test_counter;
	unsigned int failure_counter;
};

int test_group_begin(struct test_group *group);
void test_group_test(struct test_group *group, bool (*test)(void), char *name);
int test_group_end(struct test_group *group);

int broken_unit_call(const char *function);

#endif /* _JOOL_UNIT_TEST_H */
