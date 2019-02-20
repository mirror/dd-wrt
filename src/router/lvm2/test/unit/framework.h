#ifndef TEST_UNIT_FRAMEWORK_H
#define TEST_UNIT_FRAMEWORK_H

#include "configure.h"
#include "device_mapper/all.h"

#include <stdbool.h>
#include <stdint.h>
#include <setjmp.h>

//-----------------------------------------------------------------

// A test suite gathers a set of tests with a common fixture together.
struct test_suite {
	struct dm_list list;

	void *(*fixture_init)(void);
	void (*fixture_exit)(void *);
	struct dm_list tests;
};

struct test_details {
	struct test_suite *parent;
	struct dm_list list;

	const char *path;
	const char *desc;
	void (*fn)(void *);
};

struct test_suite *test_suite_create(void *(*fixture_init)(void),
				     void (*fixture_exit)(void *));
void test_suite_destroy(struct test_suite *ts);

bool register_test(struct test_suite *ts,
		   const char *path, const char *desc, void (*fn)(void *));

void test_fail(const char *fmt, ...)
	__attribute__((noreturn, format (printf, 1, 2)));

#define T_ASSERT(e) do {if (!(e)) {test_fail("assertion failed: '%s'", # e);} } while(0)
#define T_ASSERT_EQUAL(x, y) T_ASSERT((x) == (y))
#define T_ASSERT_NOT_EQUAL(x, y) T_ASSERT((x) != (y))

extern jmp_buf test_k;
#define TEST_FAILED 1

//-----------------------------------------------------------------

#endif
