#include <linux/module.h>
#include "framework/unit_test.h"
#include "mod/common/db/bib/db.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("BIB table module test.");

static struct xlator jool;
#define TEST_BIB_COUNT 5
static struct bib_entry entries[TEST_BIB_COUNT];

static bool inject(unsigned int index, char *addr4, u16 port4,
		char *addr6, u16 port6)
{
	if (str_to_addr4(addr4, &entries[index].addr4.l3))
		return false;
	if (str_to_addr6(addr6, &entries[index].addr6.l3))
		return false;
	entries[index].addr4.l4 = port4;
	entries[index].addr6.l4 = port6;
	entries[index].l4_proto = L4PROTO_UDP;

	return !bib_add_static(&jool, &entries[index]);
}

static bool insert_test_bibs(void)
{
	return inject(0, "192.0.2.1", 100, "2001:db8::1", 100)
			&& inject(1, "192.0.2.2", 50, "2001:db8::2", 50)
			&& inject(2, "192.0.2.2", 100, "2001:db8::2", 100)
			&& inject(3, "192.0.2.2", 150, "2001:db8::2", 150)
			&& inject(4, "192.0.2.3", 100, "2001:db8::3", 100);
}

struct unit_iteration_args {
	unsigned int i;
	unsigned int offset;
};

static int cb(struct bib_entry const *bib, void *void_args)
{
	struct unit_iteration_args *args = void_args;
	unsigned int index;
	bool success = true;

	index = args->offset + args->i;
	success &= ASSERT_BOOL(true, index < TEST_BIB_COUNT, "overflow");
	if (success)
		success &= ASSERT_BIB(&entries[index], bib, "Bib");

	args->i++;
	return success ? 0 : -EINVAL;
}

static bool test_foreach(void)
{
	struct ipv4_transport_addr offset;
	struct unit_iteration_args args;
	int error;
	bool success = true;

	offset.l3.s_addr = cpu_to_be32(0xc0000202u); /* 192.0.2.2 */
	offset.l4 = 100;

	/* Empty table, no offset. */
	args.i = 0;
	args.offset = 0;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, NULL);
	success &= ASSERT_INT(0, error, "call 1 result");
	success &= ASSERT_UINT(0, args.i, "call 1 counter");

	/* Empty table, offset, offset not found. */
	args.i = 0;
	args.offset = 0;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 3 result");
	success &= ASSERT_UINT(0, args.i, "call 3 counter");

	/* ----------------------------------- */

	if (!insert_test_bibs())
		return false;

	/* Populated table, no offset. */
	args.i = 0;
	args.offset = 0;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, NULL);
	success &= ASSERT_INT(0, error, "call 4 result");
	success &= ASSERT_UINT(5, args.i, "call 4 counter");

	/* Populated table, offset, offset found. */
	args.i = 0;
	args.offset = 3;
	offset.l4 = 100;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 7 result");
	success &= ASSERT_UINT(2, args.i, "call 7 counter");

	/* Populated table, offset, offset not found. */
	args.i = 0;
	args.offset = 3;
	offset.l4 = 125;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 8 result");
	success &= ASSERT_UINT(2, args.i, "call 8 counter");

	/* ----------------------------------- */

	/* Offset is before first. */
	args.i = 0;
	args.offset = 0;
	offset.l3.s_addr = cpu_to_be32(0xc0000201u);
	offset.l4 = 50;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 10 result");
	success &= ASSERT_UINT(5, args.i, "call 10 counter");

	/* Offset is first. */
	args.i = 0;
	args.offset = 1;
	offset.l4 = 100;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 12 result");
	success &= ASSERT_UINT(4, args.i, "call 12 counter");

	/* Offset is last, do not include offset. */
	args.i = 0;
	offset.l3.s_addr = cpu_to_be32(0xc0000203u);
	offset.l4 = 100;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
	success &= ASSERT_INT(0, error, "call 14 result");
	success &= ASSERT_UINT(0, args.i, "call 14 counter");

	/* Offset is after last, do not include offset. */
	args.i = 0;
	offset.l4 = 150;
	error = bib_foreach(jool.nat64.bib, L4PROTO_UDP, cb, &args, &offset);
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
		.name = "BIB table",
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
