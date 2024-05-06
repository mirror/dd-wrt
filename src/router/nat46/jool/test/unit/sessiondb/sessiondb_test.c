#include <linux/module.h>
#include <linux/printk.h>

#include "framework/unit_test.h"
#include "common/constants.h"
#include "mod/common/db/bib/db.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva Popper");
MODULE_DESCRIPTION("Session DB module test.");

static struct xlator jool;
static const l4_protocol PROTO = L4PROTO_UDP;
static struct session_entry session_instances[16];
static struct session_entry *sessions[4][4][4][4];

static void init_src6(struct ipv6_transport_addr *addr, __u16 last_byte,
		__u16 port)
{
	addr->l3.s6_addr32[0] = cpu_to_be32(0x20010db8u);
	addr->l3.s6_addr32[1] = 0;
	addr->l3.s6_addr32[2] = 0;
	addr->l3.s6_addr32[3] = cpu_to_be32(last_byte);
	addr->l4 = port;
}

static void init_dst6(struct ipv6_transport_addr *addr, __u16 last_byte,
		__u16 port)
{
	addr->l3.s6_addr32[0] = cpu_to_be32(0x0064ff9bu);
	addr->l3.s6_addr32[1] = 0;
	addr->l3.s6_addr32[2] = 0;
	addr->l3.s6_addr32[3] = cpu_to_be32(0xc0000200u | last_byte);
	addr->l4 = port;
}

static void init_src4(struct ipv4_transport_addr *addr, __u16 last_byte,
		__u16 port)
{
	addr->l3.s_addr = cpu_to_be32(0xcb007100u | last_byte);
	addr->l4 = port;
}

static void init_dst4(struct ipv4_transport_addr *addr, __u16 last_byte,
		__u16 port)
{
	addr->l3.s_addr = cpu_to_be32(0xc0000200u | last_byte);
	addr->l4 = port;
}

static int compare_session_foreach_cb(struct session_entry const *session,
		void *arg)
{
	return session_equals(session, arg);
}

static bool session_exists(struct session_entry *session)
{
	/* This is the closest we currently have to a find_session function. */
	return bib_foreach_session(&jool, session->proto,
			compare_session_foreach_cb, session, NULL);
}

static bool assert_session(unsigned int la, unsigned int lp,
		unsigned int ra, unsigned int rp)
{
	struct session_entry session;
	int expected;

	init_src6(&session.src6, la, lp);
	init_dst6(&session.dst6, ra, rp);
	init_src4(&session.src4, la, lp);
	init_dst4(&session.dst4, ra, rp);
	session.proto = PROTO;

	expected = !!sessions[la][lp][ra][rp];
	return ASSERT_INT(expected, session_exists(&session),
			"session %u %u %u %u lookup", la, lp, ra, rp);
}

static bool test_db(void)
{
	unsigned int la; /* local addr */
	unsigned int lp; /* local port */
	unsigned int ra; /* remote addr */
	unsigned int rp; /* remote port */
	bool success = true;

	for (la = 0; la < 4; la++) {
		for (lp = 0; lp < 4; lp++) {
			for (ra = 0; ra < 4; ra++) {
				for (rp = 0; rp < 4; rp++) {
					success &= assert_session(la, lp, ra, rp);
				}
			}
		}
	}

	return success;
}

static bool inject(unsigned int index, __u32 src_addr, __u16 src_id,
		__u32 dst_addr, __u16 dst_id)
{
	struct session_entry *entry;
	int error;

	entry = &session_instances[index];
	sessions[src_addr][src_id][dst_addr][dst_id] = entry;

	init_src6(&entry->src6, src_addr, src_id);
	init_dst6(&entry->dst6, dst_addr, dst_id);
	init_src4(&entry->src4, src_addr, src_id);
	init_dst4(&entry->dst4, dst_addr, dst_id);
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
	bool success = true;

	memset(session_instances, 0, sizeof(session_instances));
	memset(sessions, 0, sizeof(sessions));

	success &= inject(0, 1, 2, 2, 2);
	success &= inject(1, 1, 1, 2, 1);
	success &= inject(2, 2, 1, 2, 1);
	success &= inject(3, 2, 2, 2, 2);
	success &= inject(4, 1, 1, 2, 2);
	success &= inject(5, 2, 2, 1, 1);
	success &= inject(6, 2, 1, 1, 1);
	success &= inject(7, 1, 1, 1, 1);
	success &= inject(8, 2, 2, 1, 2);
	success &= inject(9, 1, 2, 1, 1);
	success &= inject(10, 2, 1, 1, 2);
	success &= inject(11, 1, 2, 1, 2);
	success &= inject(12, 2, 1, 2, 2);
	success &= inject(13, 1, 1, 1, 2);
	success &= inject(14, 1, 2, 2, 1);
	success &= inject(15, 2, 2, 2, 1);

	return success ? test_db() : false;
}

static bool flush(void)
{
	log_debug(NULL, "Flushing.");
	bib_flush(&jool);

	memset(session_instances, 0, sizeof(session_instances));
	memset(sessions, 0, sizeof(sessions));
	return test_db();
}

static bool simple_session(void)
{
	struct ipv4_range range;
	bool success = true;

	if (!insert_test_sessions())
		return false;

	/* ---------------------------------------------------------- */

	log_debug(NULL, "Deleting sessions by BIB.");
	range.prefix.addr.s_addr = cpu_to_be32(0xcb007101u);
	range.prefix.len = 32;
	range.ports.min = 1;
	range.ports.max = 1;
	bib_rm_range(&jool, PROTO, &range);

	sessions[1][1][2][2] = NULL;
	sessions[1][1][2][1] = NULL;
	sessions[1][1][1][1] = NULL;
	sessions[1][1][1][2] = NULL;
	success &= test_db();

	/* ---------------------------------------------------------- */

	log_debug(NULL, "Deleting again.");
	bib_rm_range(&jool, PROTO, &range);
	success &= test_db();

	/* ---------------------------------------------------------- */

	success &= flush();
	if (!insert_test_sessions())
		return false;

	/* ---------------------------------------------------------- */

	log_debug(NULL, "Deleting by range (all addresses, lower ports).");
	range.prefix.addr.s_addr = cpu_to_be32(0xcb007100u);
	range.prefix.len = 30;
	range.ports.min = 0;
	range.ports.max = 1;
	bib_rm_range(&jool, PROTO, &range);

	sessions[2][1][2][1] = NULL;
	sessions[2][1][1][1] = NULL;
	sessions[1][1][2][2] = NULL;
	sessions[2][1][2][2] = NULL;
	sessions[2][1][1][2] = NULL;
	sessions[1][1][2][1] = NULL;
	sessions[1][1][1][1] = NULL;
	sessions[1][1][1][2] = NULL;
	success &= test_db();

	/* ---------------------------------------------------------- */

	success &= flush();
	if (!insert_test_sessions())
		return false;

	/* ---------------------------------------------------------- */

	log_debug(NULL, "Deleting by range (lower addresses, all ports).");
	range.prefix.addr.s_addr = cpu_to_be32(0xcb007100u);
	range.prefix.len = 31;
	range.ports.min = 0;
	range.ports.max = 65535;
	bib_rm_range(&jool, PROTO, &range);

	sessions[1][2][2][2] = NULL;
	sessions[1][1][2][2] = NULL;
	sessions[1][2][1][1] = NULL;
	sessions[1][1][2][1] = NULL;
	sessions[1][2][2][1] = NULL;
	sessions[1][2][1][2] = NULL;
	sessions[1][1][1][1] = NULL;
	sessions[1][1][1][2] = NULL;
	success &= test_db();

	/* ---------------------------------------------------------- */

	success &= flush();
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
		.name = "Session",
		.teardown_fn = bib_teardown,
		.init_fn = init,
		.clean_fn = clean,
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, simple_session, "Single Session");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
