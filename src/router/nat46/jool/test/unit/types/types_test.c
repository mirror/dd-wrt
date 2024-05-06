#include <linux/module.h>

#include "framework/unit_test.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva Popper");
MODULE_DESCRIPTION("Types test.");

#define assert_prt(expected, r1min, r1max, r2min, r2max, name) \
	r1.min = r1min; \
	r1.max = r1max; \
	r2.min = r2min; \
	r2.max = r2max; \
	success &= ASSERT_BOOL(expected, port_range_touches(&r1, &r2), name);

static bool test_port_range_touches(void)
{
	struct port_range r1;
	struct port_range r2;
	bool success = true;

	assert_prt(true, 1, 3, 2, 6, "1326");
	assert_prt(true, 1, 3, 3, 6, "1336");
	assert_prt(true, 1, 3, 4, 6, "1346");
	assert_prt(false, 1, 3, 5, 6, "1356");

	/* The point of these is to test overflow on those +1/-1 on r2. */
	assert_prt(false, 2, 3, 0, 0, "2300");
	assert_prt(true, 1, 3, 0, 0, "1300");
	assert_prt(false, 65531, 65532, 65534, 65535, "2300");
	assert_prt(true, 65531, 65533, 65534, 65535, "2300");

	return success;
}

static void init_list(struct list_head *list, ...)
{
	struct list_head *cursor;
	va_list args;

	INIT_LIST_HEAD(list);

	va_start(args, list);
	while ((cursor = va_arg(args, struct list_head *)) != NULL)
		list_add_tail(cursor, list);
	va_end(args);
}

static bool assert_list(struct list_head *list, ...)
{
	struct list_head *expected, *actual;
	va_list args;

	va_start(args, list);
	list_for_each(actual, list) {
		expected = va_arg(args, struct list_head *);
		if (expected != actual)
			goto fail;
	}

	expected = va_arg(args, struct list_head *);
	if (expected != NULL)
		goto fail;

	va_end(args);
	return true;

fail:
	va_end(args);

	log_err("Bad list.");

	pr_err("  Expected: ");
	va_start(args, list);
	while ((expected = va_arg(args, struct list_head *)) != NULL)
		pr_cont("%p ", expected);
	va_end(args);
	pr_cont("\n");

	pr_err("  Actual  : ");
	list_for_each(actual, list)
		pr_cont("%p ", actual);

	return false;
}

static bool test_list_move_all(void)
{
	struct list_head node1, node2, node3, node4;
	struct list_head src, dst;
	bool success = true;

	/* Empty */
	log_info("1");
	init_list(&src, NULL);
	init_list(&dst, NULL);
	list_move_all(&src, &dst);
	success &= assert_list(&src, NULL);
	success &= assert_list(&dst, NULL);
	if (!success)
		return false;

	/* Move 1 to empty */
	log_info("2");
	list_add(&node1, &src);
	list_move_all(&src, &dst);
	success &= assert_list(&src, NULL);
	success &= assert_list(&dst, &node1, NULL);
	if (!success)
		return false;

	/* Move all to empty */
	log_info("3");
	init_list(&src, &node1, &node2, &node3, &node4, NULL);
	init_list(&dst, NULL);
	list_move_all(&src, &dst);
	success &= assert_list(&src, NULL);
	success &= assert_list(&dst, &node1, &node2, &node3, &node4, NULL);
	if (!success)
		return false;

	/* Move 2 to 1 */
	log_info("4");
	init_list(&src, &node1, &node2, NULL);
	init_list(&dst, &node3, NULL);
	list_move_all(&src, &dst);
	success &= assert_list(&src, NULL);
	success &= assert_list(&dst, &node3, &node1, &node2, NULL);
	if (!success)
		return false;

	/* Move 2 to 2, weird order */
	log_info("5");
	init_list(&src, &node3, &node1, NULL);
	init_list(&dst, &node2, &node4, NULL);
	list_move_all(&src, &dst);
	success &= assert_list(&src, NULL);
	success &= assert_list(&dst, &node2, &node4, &node3, &node1, NULL);
	if (!success)
		return false;

	return success;
}

int init_module(void)
{
	struct test_group test = {
		.name = "Types",
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, test_port_range_touches, "port range touches function");
	test_group_test(&test, test_list_move_all, "list move all function");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
