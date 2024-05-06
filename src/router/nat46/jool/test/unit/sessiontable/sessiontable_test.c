#include <linux/module.h>
#include "framework/unit_test.h"
#include "common/constants.h"
#include "mod/common/db/bib/db.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("Session table module test.");

static struct xlator jool;
#define TEST_SESSION_COUNT 9
static struct session_entry entries[TEST_SESSION_COUNT];

static void init_src6(struct in6_addr *addr, __u16 last_byte)
{
	addr->s6_addr32[0] = cpu_to_be32(0x20010db8u);
	addr->s6_addr32[1] = 0;
	addr->s6_addr32[2] = 0;
	addr->s6_addr32[3] = cpu_to_be32(last_byte);
}

static void init_dst6(struct in6_addr *addr, __u16 last_byte)
{
	addr->s6_addr32[0] = cpu_to_be32(0x0064ff9bu);
	addr->s6_addr32[1] = 0;
	addr->s6_addr32[2] = 0;
	addr->s6_addr32[3] = cpu_to_be32(0xc0000200u | last_byte);
}

static bool inject(unsigned int index, __u32 src_addr, __u16 src_id,
		__u32 dst_addr, __u16 dst_id)
{
	struct session_entry *entry = &entries[index];
	int error;

	init_src6(&entry->src6.l3, src_addr);
	entry->src6.l4 = src_id;
	init_dst6(&entry->dst6.l3, dst_addr);
	entry->dst6.l4 = dst_id;
	entry->src4.l3.s_addr = cpu_to_be32(0xcb007100u | src_addr);
	entry->src4.l4 = src_id;
	entry->dst4.l3.s_addr = cpu_to_be32(0xc0000200u | dst_addr);
	entry->dst4.l4 = dst_id;
	entry->proto = L4PROTO_UDP;
	entry->state = ESTABLISHED;
	entry->timer_type = SESSION_TIMER_EST;
	entry->update_time = jiffies;
	entry->timeout = UDP_DEFAULT;
	entry->has_stored = false;

	error = bib_add_session(&jool, entry, NULL);
	if (error) {
		log_err("Errcode %d on sessiontable_add.", error);
		return false;
	}

	return true;
}

static bool insert_test_sessions(void)
{
	/*
	 * Notice:
	 * This whole file currently only intends to test the foreach function.
	 * Other functions are better covered in the Session DB test.
	 *
	 * However, I'm also adding noise to the add function because it's free:
	 *
	 * This should be every combination needed to test sessiontable_add()
	 * sorts by src transport address, then by dst transport address.
	 * (Though the test only covers IPv4 order.)
	 * Also,
	 * the insertion order is random; it doesn't have any purpose other
	 * than tentatively messing with the add function.
	 */
	return inject(1, 2, 100, 3, 1300)
			&& inject(6, 2, 200, 3, 1100)
			&& inject(3, 2, 200, 2, 1100)
			&& inject(7, 2, 300, 1, 1100)
			&& inject(0, 1, 300, 3, 1300)
			&& inject(8, 3, 100, 1, 1100)
			&& inject(4, 2, 200, 2, 1200)
			&& inject(2, 2, 200, 1, 1300)
			&& inject(5, 2, 200, 2, 1300);
}

struct unit_iteration_args {
	unsigned int i;
	unsigned int offset;
};

static int cb(struct session_entry const *session, void *void_args)
{
	struct unit_iteration_args *args = void_args;
	unsigned int index;
	bool success = true;

	pr_debug("Iterating: " SEPP "\n", SEPA(session));

	index = args->offset + args->i;
	success &= ASSERT_BOOL(true, index < TEST_SESSION_COUNT, "overflow");
	if (success)
		success &= ASSERT_SESSION(&entries[index], session, "Session");

	args->i++;
	return success ? 0 : -EINVAL;
}

static bool test_foreach(void)
{
	struct unit_iteration_args args;
	struct session_foreach_offset offset;
	int error;
	bool success = true;

	offset.offset.src.l3.s_addr = cpu_to_be32(0xcb007102u);/* 203.0.113.2 */
	offset.offset.src.l4 = 200;
	offset.offset.dst.l3.s_addr = cpu_to_be32(0xc0000202u);/* 192.0.2.2 */
	offset.offset.dst.l4 = 1200;

	/* Empty table, no offset. */
	args.i = 0;
	args.offset = 0;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, NULL);
	success &= ASSERT_INT(0, error, "call 1 result");
	success &= ASSERT_UINT(0, args.i, "call 1 counter");

	/* Empty table, offset, include offset, offset not found. */
	args.i = 0;
	args.offset = 0;
	offset.include_offset = true;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 2 result");
	success &= ASSERT_UINT(0, args.i, "call 2 counter");

	/* Empty table, offset, do not include offset, offset not found. */
	args.i = 0;
	args.offset = 0;
	offset.include_offset = false;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 3 result");
	success &= ASSERT_UINT(0, args.i, "call 3 counter");

	/* ----------------------------------- */

	if (!insert_test_sessions())
		return false;

	/* Populated table, no offset. */
	args.i = 0;
	args.offset = 0;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, NULL);
	success &= ASSERT_INT(0, error, "call 4 result");
	success &= ASSERT_UINT(9, args.i, "call 4 counter");

	/* Populated table, offset, include offset, offset found. */
	args.i = 0;
	args.offset = 4;
	offset.include_offset = true;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 5 result");
	success &= ASSERT_UINT(5, args.i, "call 5 counter");

	/* Populated table, offset, include offset, offset not found. */
	args.i = 0;
	args.offset = 5;
	offset.include_offset = true;
	offset.offset.dst.l4 = 1250;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 6 result");
	success &= ASSERT_UINT(4, args.i, "call 6 counter");

	/* Populated table, offset, do not include offset, offset found. */
	args.i = 0;
	args.offset = 5;
	offset.include_offset = false;
	offset.offset.dst.l4 = 1200;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 7 result");
	success &= ASSERT_UINT(4, args.i, "call 7 counter");

	/* Populated table, offset, do not include offset, offset not found. */
	args.i = 0;
	args.offset = 5;
	offset.include_offset = false;
	offset.offset.dst.l4 = 1250;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 8 result");
	success &= ASSERT_UINT(4, args.i, "call 8 counter");

	/* ----------------------------------- */

	/* Offset is before first, include offset. */
	offset.offset.src.l3.s_addr = cpu_to_be32(0xcb007101u);/* 203.0.113.1 */
	offset.offset.src.l4 = 300;
	offset.offset.dst.l3.s_addr = cpu_to_be32(0xc0000203u);/* 192.0.2.3 */
	offset.offset.dst.l4 = 1200;

	args.i = 0;
	args.offset = 0;
	offset.include_offset = true;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 9 result");
	success &= ASSERT_UINT(9, args.i, "call 9 counter");

	/* Offset is before first, do not include offset. */
	args.i = 0;
	offset.include_offset = false;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 10 result");
	success &= ASSERT_UINT(9, args.i, "call 10 counter");

	/* Offset is first, include offset. */
	offset.offset.dst.l4 = 1300;

	args.i = 0;
	offset.include_offset = true;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 11 result");
	success &= ASSERT_UINT(9, args.i, "call 11 counter");

	/* Offset is first, do not include offset. */
	args.i = 0;
	args.offset = 1;
	offset.include_offset = false;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 12 result");
	success &= ASSERT_UINT(8, args.i, "call 12 counter");

	/* Offset is last, include offset. */
	offset.offset.src.l3.s_addr = cpu_to_be32(0xcb007103u); /* 203.0.113.3 */
	offset.offset.src.l4 = 100;
	offset.offset.dst.l3.s_addr = cpu_to_be32(0xc0000201u); /* 192.0.2.1 */
	offset.offset.dst.l4 = 1100;

	args.i = 0;
	args.offset = 8;
	offset.include_offset = true;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 13 result");
	success &= ASSERT_UINT(1, args.i, "call 13 counter");

	/* Offset is last, do not include offset. */
	args.i = 0;
	offset.include_offset = false;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 14 result");
	success &= ASSERT_UINT(0, args.i, "call 14 counter");

	/* Offset is after last, include offset. */
	offset.offset.src.l4 = 1200;

	args.i = 0;
	offset.include_offset = true;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 15 result");
	success &= ASSERT_UINT(0, args.i, "call 15 counter");

	/* Offset is after last, do not include offset. */
	args.i = 0;
	offset.include_offset = false;
	error = bib_foreach_session(&jool, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 16 result");
	success &= ASSERT_UINT(0, args.i, "call 16 counter");

	return success;
}

enum session_fate tcp_est_expire_cb(struct session_entry *session, void *arg)
{
	return FATE_RM;
}

static int init(void)
{
	return xlator_init(&jool, NULL, INAME_DEFAULT, XF_NETFILTER | XT_NAT64,
			NULL);
}

static void clean(void)
{
	xlator_put(&jool);
}

int init_module(void)
{
	struct test_group test = {
		.name = "Session table",
		.teardown_fn = bib_teardown,
		.init_fn = init,
		.clean_fn = clean,
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, test_foreach, "Foreach");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
