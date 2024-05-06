#include <linux/kernel.h>
#include <linux/module.h>

#include "framework/unit_test.h"
#include "mod/common/db/pool4/db.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Ramiro Nava");
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("IPv4 pool DB module test");

/**
 * Instances the functions do their testing on.
 * Ideally, each test would have its own pointers, but that floods argument
 * lists.
 */
static struct pool4 *pool;
static struct net *ns;

/**
 * add - Boilerplate code to add an entry to the pool during the tests.
 */
static bool add(__u32 addr, __u8 prefix_len, __u16 min, __u16 max)
{
	struct pool4_entry entry;

	entry.mark = 1;
	entry.iterations = 0;
	entry.flags = ITERATIONS_SET | ITERATIONS_INFINITE;
	entry.proto = L4PROTO_TCP;
	entry.range.prefix.addr.s_addr = cpu_to_be32(addr);
	entry.range.prefix.len = prefix_len;
	entry.range.ports.min = min;
	entry.range.ports.max = max;

	return ASSERT_INT(0, pool4db_add(pool, &entry), "add %pI4/%u (%u-%u)",
			&entry.range.prefix.addr, prefix_len, min, max);
}

static bool rm(__u32 addr, __u8 prefix_len, __u16 min, __u16 max)
{
	struct ipv4_range range;

	range.prefix.addr.s_addr = cpu_to_be32(addr);
	range.prefix.len = prefix_len;
	range.ports.min = min;
	range.ports.max = max;

	return ASSERT_INT(0, pool4db_rm(pool, 1, L4PROTO_TCP, &range),
			"rm of %pI4/%u (%u-%u)",
			&range.prefix.addr, prefix_len, min, max);
}

static bool add_common_samples(void)
{
	if (!add(0xc0000200U, 31, 6, 7)) /* 192.0.2.0/31 (6-7) */
		return false;
	if (!add(0xc0000210U, 32, 15, 18)) /* 192.0.2.16 (15-18) */
		return false;
	if (!add(0xc0000220U, 30, 1, 1)) /* 192.0.2.32/30 (1-1) */
		return false;
	if (!add(0xc0000210U, 32, 22, 23)) /* 192.0.2.16 (22-23) */
		return false;
	if (!add(0xc0000210U, 31, 19, 19)) /* 192.0.2.16/31 (19-19) */
		return false;

	return true;
}

struct foreach_taddr4_args {
	struct ipv4_transport_addr *expected;
	unsigned int expected_len;
	unsigned int i;
};

static void init_sample(struct pool4_entry *sample, __u32 addr, __u16 min,
		__u16 max)
{
	sample->mark = 1;
	sample->proto = L4PROTO_TCP;
	sample->range.prefix.addr.s_addr = cpu_to_be32(addr);
	sample->range.prefix.len = 32;
	sample->range.ports.min = min;
	sample->range.ports.max = max;
}

struct foreach_sample_args {
	struct pool4_entry *expected;
	unsigned int expected_len;
	unsigned int samples;
	unsigned int taddrs;
};

static int validate_sample(struct pool4_entry const *sample, void *void_args)
{
	struct foreach_sample_args *args = void_args;
	bool success = true;

	/* log_debug("  foreaching %pI4 %u-%u", &sample->range.addr,
			sample->range.ports.min, sample->range.ports.max); */

	success &= ASSERT_BOOL(true, args->samples < args->expected_len,
			"overflow (%u %u)", args->samples, args->expected_len);
	if (!success)
		return -EINVAL;

	success &= ASSERT_PREFIX4(&args->expected[args->samples].range.prefix,
			&sample->range.prefix, "prefix");
	success &= ASSERT_UINT(args->expected[args->samples].range.ports.min,
			sample->range.ports.min, "min");
	success &= ASSERT_UINT(args->expected[args->samples].range.ports.max,
			sample->range.ports.max, "max");

	args->samples++;
	args->taddrs += port_range_count(&sample->range.ports);
	return success ? 0 : -EINVAL;
}

#define COUNT 9

static bool test_foreach_sample(void)
{
	struct pool4_entry expected[COUNT];
	unsigned int i = 0;
	struct foreach_sample_args args;
	int error;
	bool success = true;

	if (!add_common_samples())
		return false;

	init_sample(&expected[i++], 0xc0000200U, 6, 7);
	init_sample(&expected[i++], 0xc0000201U, 6, 7);
	init_sample(&expected[i++], 0xc0000210U, 15, 19);
	init_sample(&expected[i++], 0xc0000210U, 22, 23);
	init_sample(&expected[i++], 0xc0000211U, 19, 19);
	init_sample(&expected[i++], 0xc0000220U, 1, 1);
	init_sample(&expected[i++], 0xc0000221U, 1, 1);
	init_sample(&expected[i++], 0xc0000222U, 1, 1);
	init_sample(&expected[i++], 0xc0000223U, 1, 1);

	if (i != COUNT) {
		log_err("Input mismatch. Unit test is broken: %u %u", i, COUNT);
		return false;
	}

	args.expected = &expected[0];
	args.expected_len = COUNT;
	args.samples = 0;
	args.taddrs = 0;
	error = pool4db_foreach_sample(pool, L4PROTO_TCP, validate_sample,
			&args, NULL);
	success &= ASSERT_INT(0, error, "no-offset call");
	success &= ASSERT_UINT(9, args.samples, "no-offset samples");
	success &= ASSERT_UINT(16, args.taddrs, "no-offset taddrs");

	for (i = 0; i < COUNT; i++) {
		/* foreach sample skips offset. */
		args.expected = &expected[i + 1];
		args.expected_len = COUNT - i - 1;
		args.samples = 0;
		args.taddrs = 0;
		error = pool4db_foreach_sample(pool, L4PROTO_TCP,
				validate_sample, &args, &expected[i]);
		success &= ASSERT_INT(0, error, "call %u", i);
		/* log_debug("--------------"); */
	}

	return success;
}

#undef COUNT

/**
 * assert_contains_range - "assert 192.0.2.@addr_min - 192.0.2.@addr_max on
 * ports @port_min through @port_max belong to the pool (@expected true) or not
 * (@expected false)."
 */
static bool assert_contains_range(__u32 addr_min, __u32 addr_max,
		__u16 port_min, __u16 port_max, bool expected)
{
	struct ipv4_transport_addr taddr;
	__u32 i;
	bool result;
	bool success = true;

	for (i = addr_min; i <= addr_max; i++) {
		taddr.l3.s_addr = cpu_to_be32(0xc0000200U | i);
		for (taddr.l4 = port_min; taddr.l4 <= port_max; taddr.l4++) {
			result = pool4db_contains(pool, ns, L4PROTO_TCP, &taddr);
			success &= ASSERT_BOOL(expected, result,
					"contains " TA4PP, TA4PA(taddr));
			result = pool4db_contains(pool, ns, L4PROTO_TCP, &taddr);
			success &= ASSERT_BOOL(expected, result,
					"contains_all " TA4PP, TA4PA(taddr));
		}
	}

	return success;
}

static bool __foreach(struct pool4_entry *expected, unsigned int expected_len,
		unsigned int expected_taddrs)
{
	struct foreach_sample_args args;
	int error;
	bool success = true;

	args.expected = expected;
	args.expected_len = expected_len;
	args.samples = 0;
	args.taddrs = 0;

	error = pool4db_foreach_sample(pool, L4PROTO_TCP, validate_sample,
			&args, NULL);
	success &= ASSERT_INT(0, error, "foreach result");
	success &= ASSERT_UINT(expected_len, args.samples, "foreach count");
	success &= ASSERT_UINT(expected_taddrs, args.taddrs, "foreach taddrs");
	return success;
}

static bool test_add(void)
{
	struct pool4_entry samples[8];
	bool success = true;

	/* ---------------------------------------------------------- */

	/* Add a single small range. */
	if (!add(0xc0000211U, 32, 10, 20)) /* 192.0.2.17 (10-20) */
		return false;

	success &= assert_contains_range(16, 16, 0, 30, false);
	success &= assert_contains_range(17, 17, 0, 9, false);
	success &= assert_contains_range(17, 17, 10, 20, true);
	success &= assert_contains_range(17, 17, 21, 30, false);
	success &= assert_contains_range(18, 18, 0, 30, false);

	init_sample(&samples[0], 0xc0000211U, 10, 20);
	success &= __foreach(samples, 1, 11);

	/* ---------------------------------------------------------- */

	/* Append an adjacent range (left). They should join each other. */
	if (!add(0xc0000211U, 32, 5, 10)) /* 192.0.2.17 (5-10) */
		return false;

	success &= assert_contains_range(0, 16, 0, 30, false);
	success &= assert_contains_range(17, 17, 0, 4, false);
	success &= assert_contains_range(17, 17, 5, 20, true);
	success &= assert_contains_range(17, 17, 21, 30, false);
	success &= assert_contains_range(18, 32, 0, 30, false);

	init_sample(&samples[0], 0xc0000211U, 5, 20);
	success &= __foreach(samples, 1, 16);

	/* ---------------------------------------------------------- */

	/* Append an adjacent range (right). They should join each other. */
	if (!add(0xc0000211U, 32, 20, 25)) /* 192.0.2.17 (20-25) */
		return false;

	success &= assert_contains_range(0, 16, 0, 30, false);
	success &= assert_contains_range(17, 17, 0, 4, false);
	success &= assert_contains_range(17, 17, 5, 25, true);
	success &= assert_contains_range(17, 17, 26, 30, false);
	success &= assert_contains_range(18, 32, 0, 30, false);

	init_sample(&samples[0], 0xc0000211U, 5, 25);
	success &= __foreach(samples, 1, 21);

	/* ---------------------------------------------------------- */

	/* Add intersecting ranges. They should join each other. */
	if (!add(0xc0000210U, 32, 10, 20)) /* 192.0.2.16 (10-20) */
		return false;
	if (!add(0xc0000210U, 32, 5, 12)) /* 192.0.2.16 (5-12) */
		return false;
	if (!add(0xc0000210U, 32, 18, 25)) /* 192.0.2.16 (18-25) */
		return false;

	success &= assert_contains_range(15, 15, 0, 30, false);
	success &= assert_contains_range(16, 17, 0, 4, false);
	success &= assert_contains_range(16, 17, 5, 25, true);
	success &= assert_contains_range(16, 17, 26, 30, false);
	success &= assert_contains_range(18, 18, 0, 30, false);

	init_sample(&samples[0], 0xc0000210U, 5, 25);
	init_sample(&samples[1], 0xc0000211U, 5, 25);
	success &= __foreach(samples, 2, 42);

	/* ---------------------------------------------------------- */

	/* Add a bigger range. The bigger one should replace. */
	if (!add(0xc0000212U, 32, 10, 20)) /* 192.0.2.18 (10-20) */
		return false;
	if (!add(0xc0000212U, 32, 5, 25)) /* 192.0.2.18 (5-25) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 18, 0, 4, false);
	success &= assert_contains_range(16, 18, 5, 25, true);
	success &= assert_contains_range(16, 18, 26, 30, false);
	success &= assert_contains_range(19, 32, 0, 30, false);

	init_sample(&samples[2], 0xc0000212U, 5, 25);
	success &= __foreach(samples, 3, 63);

	/* ---------------------------------------------------------- */

	/* Add an already existing range. Nothing should change. */
	if (!add(0xc0000212U, 32, 5, 25)) /* 192.0.2.18 (5-25) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 18, 0, 4, false);
	success &= assert_contains_range(16, 18, 5, 25, true);
	success &= assert_contains_range(16, 18, 26, 30, false);
	success &= assert_contains_range(19, 32, 0, 30, false);

	success &= __foreach(samples, 3, 63);

	/* ---------------------------------------------------------- */

	/* Add a smaller range. Nothing should change. */
	if (!add(0xc0000212U, 32, 5, 25)) /* 192.0.2.18 (10-20) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 18, 0, 4, false);
	success &= assert_contains_range(16, 18, 5, 25, true);
	success &= assert_contains_range(16, 18, 26, 30, false);
	success &= assert_contains_range(19, 32, 0, 30, false);

	success &= __foreach(samples, 3, 63);

	/* ---------------------------------------------------------- */

	/* Fill a hole. The three ranges should become one. */
	if (!add(0xc0000213U, 32, 5, 10)) /* 192.0.2.19 (5-10) */
		return false;
	if (!add(0xc0000213U, 32, 20, 25)) /* 192.0.2.19 (20-25) */
		return false;
	if (!add(0xc0000213U, 32, 11, 19)) /* 192.0.2.19 (11-19) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 19, 0, 4, false);
	success &= assert_contains_range(16, 19, 5, 25, true);
	success &= assert_contains_range(16, 19, 26, 30, false);
	success &= assert_contains_range(20, 32, 0, 30, false);

	init_sample(&samples[3], 0xc0000213U, 5, 25);
	success &= __foreach(samples, 4, 84);

	/* ---------------------------------------------------------- */

	/* Cover several holes with one big range. */
	if (!add(0xc0000214U, 32, 8, 11)) /* 192.0.2.20 (8-11) */
		return false;
	if (!add(0xc0000214U, 32, 14, 17)) /* 192.0.2.20 (14-17) */
		return false;
	if (!add(0xc0000214U, 32, 20, 23)) /* 192.0.2.20 (20-23) */
		return false;
	if (!add(0xc0000214U, 32, 5, 25)) /* 192.0.2.20 (5-25) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 20, 0, 4, false);
	success &= assert_contains_range(16, 20, 5, 25, true);
	success &= assert_contains_range(16, 20, 26, 30, false);
	success &= assert_contains_range(21, 32, 0, 30, false);

	init_sample(&samples[4], 0xc0000214U, 5, 25);
	success &= __foreach(samples, 5, 105);

	/* ---------------------------------------------------------- */

	/*
	 * Now add four addresses in one call.
	 * First one intersects, so only 3 are committed.
	 */
	if (!add(0xc0000214U, 30, 5, 25)) /* 192.0.2.20-23 (5-25) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 23, 0, 4, false);
	success &= assert_contains_range(16, 23, 5, 25, true);
	success &= assert_contains_range(16, 23, 26, 30, false);
	success &= assert_contains_range(24, 32, 0, 30, false);

	init_sample(&samples[5], 0xc0000215U, 5, 25);
	init_sample(&samples[6], 0xc0000216U, 5, 25);
	init_sample(&samples[7], 0xc0000217U, 5, 25);
	success &= __foreach(samples, 8, 168);

	return success;
}

static bool test_rm(void)
{
	struct pool4_entry samples[8];
	unsigned int i;
	bool success = true;

	if (!add(0xc0000210U, 29, 5, 25)) /* 192.0.2.16-23 (5-25) */
		return false;

	/* ---------------------------------------------------------- */

	/* Remove some outermost ports from multiple addresses. */
	if (!rm(0xc0000210U, 30, 5, 9)) /* Lower of 192.0.2.16-19 (exact)*/
		return false;
	if (!rm(0xc0000214U, 30, 1, 9)) /* Lower of 192.0.2.20-23 (excess) */
		return false;
	if (!rm(0xc0000210U, 30, 21, 25)) /* Upper of 192.0.2.16-19 (exact) */
		return false;
	if (!rm(0xc0000214U, 30, 21, 30)) /* Upper of 192.0.2.20-23 (excess) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 23, 0, 9, false);
	success &= assert_contains_range(16, 23, 10, 20, true);
	success &= assert_contains_range(16, 23, 21, 30, false);
	success &= assert_contains_range(24, 32, 0, 30, false);

	for (i = 0; i < 8; i++)
		init_sample(&samples[i], 0xc0000210U + i, 10, 20);
	success &= __foreach(samples, 8, 88);

	/* ---------------------------------------------------------- */

	/* Remove a handful of addresses completely. */
	if (!rm(0xc0000214U, 31, 10, 20)) /* 192.0.2.20-21 (exact)*/
		return false;
	if (!rm(0xc0000216U, 31, 0, 30)) /* 192.0.2.22-23 (excess)*/
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 19, 0, 9, false);
	success &= assert_contains_range(16, 19, 10, 20, true);
	success &= assert_contains_range(16, 19, 21, 30, false);
	success &= assert_contains_range(20, 32, 0, 30, false);

	success &= __foreach(samples, 4, 44);

	/* ---------------------------------------------------------- */

	/* Punch a hole in ranges from multiple addresses. */
	if (!rm(0xc0000212U, 31, 13, 17)) /* 192.0.2.18-19 (13-17) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 17, 0, 9, false);
	success &= assert_contains_range(16, 17, 10, 20, true);
	success &= assert_contains_range(16, 17, 21, 30, false);
	success &= assert_contains_range(18, 19, 0, 9, false);
	success &= assert_contains_range(18, 19, 10, 12, true);
	success &= assert_contains_range(18, 19, 13, 17, false);
	success &= assert_contains_range(18, 19, 18, 20, true);
	success &= assert_contains_range(18, 19, 21, 30, false);
	success &= assert_contains_range(20, 32, 0, 30, false);

	init_sample(&samples[2], 0xc0000212U, 10, 12);
	init_sample(&samples[3], 0xc0000212U, 18, 20);
	init_sample(&samples[4], 0xc0000213U, 10, 12);
	init_sample(&samples[5], 0xc0000213U, 18, 20);
	success &= __foreach(samples, 6, 34);

	/* ---------------------------------------------------------- */

	/* Remove multiple ranges from a single address at once. */
	if (!rm(0xc0000213U, 32, 0, 30)) /* 192.0.2.19 (0-30) */
		return false;

	success &= assert_contains_range(0, 15, 0, 30, false);
	success &= assert_contains_range(16, 17, 0, 9, false);
	success &= assert_contains_range(16, 17, 10, 20, true);
	success &= assert_contains_range(16, 17, 21, 30, false);
	success &= assert_contains_range(18, 18, 0, 9, false);
	success &= assert_contains_range(18, 18, 10, 12, true);
	success &= assert_contains_range(18, 18, 13, 17, false);
	success &= assert_contains_range(18, 18, 18, 20, true);
	success &= assert_contains_range(18, 18, 21, 30, false);
	success &= assert_contains_range(19, 32, 0, 30, false);

	success &= __foreach(samples, 4, 28);

	/* ---------------------------------------------------------- */

	/* Finally, test an empty database. */
	if (!rm(0xc0000200U, 24, 0, 65535U)) /* 192.0.2.0-255 (0-65535) */
		return false;

	success &= assert_contains_range(0, 32, 0, 30, false);
	success &= __foreach(samples, 0, 0);

	return success;
}

static bool test_flush(void)
{
	bool success = true;

	/*
	 * It doesn't look like there are corner cases here.
	 * Not even empty pool4, since that's pretty much all RB tree logic.
	 * (Which is tested elsewhere.)
	 */

	if (!add_common_samples())
		return false;

	pool4db_flush(pool);
	success &= __foreach(NULL, 0, 0);
	return success;
}

static int init(void)
{
	pool = pool4db_alloc();
	if (!pool)
		return -ENOMEM;

	ns = get_net_ns_by_pid(task_pid_vnr(current));
	if (IS_ERR(ns)) {
		log_err("Could not retrieve the current namespace.");
		pool4db_put(pool);
		return PTR_ERR(ns);
	}

	return 0;
}

static void clean(void)
{
	put_net(ns);
	pool4db_put(pool);
}

int init_module(void)
{
	struct test_group test = {
		.name = "IPv4 Pool DB",
		.init_fn = init,
		.clean_fn = clean,
	};

	if (test_group_begin(&test))
		return -EINVAL;

	/*
	 * TODO (test) This is missing a multiple-tables test.
	 * (it always does mark = 1.)
	 */
	test_group_test(&test, test_foreach_sample, "Sample for");
	test_group_test(&test, test_add, "Add");
	test_group_test(&test, test_rm, "Rm");
	test_group_test(&test, test_flush, "Flush");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
