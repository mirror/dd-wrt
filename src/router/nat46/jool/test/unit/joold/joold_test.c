#include <linux/kernel.h>
#include <linux/module.h>

#include "framework/bib.h"
#include "framework/unit_test.h"
#include "mod/common/joold.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("joold test.");

/* Test dummies */
static struct session_entry ss[9];

/********************** Mocks **********************/

struct sk_buff *sent;

void sendpkt_multicast(struct xlator *jool, struct sk_buff *skb)
{
	sent = skb;
}

static struct genl_family family_mock = {
	.id = 1234,
	.hdrsize = sizeof(struct joolnlhdr),
	.version = 2,
	.module = THIS_MODULE,
};

struct genl_family *jnl_family(void)
{
	return &family_mock;
}

unsigned int foreach_start;
unsigned int foreach_end;

int bib_foreach_session(struct xlator *jool, l4_protocol proto,
		session_foreach_entry_cb cb, void *cb_arg,
		struct session_foreach_offset *offset)
{
	unsigned int s;
	int error;

	if (proto != L4PROTO_TCP)
		return 0;

	for (s = foreach_start; s < foreach_end; s++) {
		error = cb(&ss[s], cb_arg);
		if (error)
			return error;
	}

	return 0;
}

int bib_add_session(struct xlator *jool,
		struct session_entry *session,
		struct collision_cb *cb)
{
	return -EINVAL;
}

/********************** Init **********************/

static void init_session(unsigned int index, struct session_entry *result)
{
	result->src6.l3.s6_addr32[0] = cpu_to_be32(0x20010db8);
	result->src6.l3.s6_addr32[1] = 0;
	result->src6.l3.s6_addr32[2] = 0;
	result->src6.l3.s6_addr32[3] = cpu_to_be32(index);
	result->src6.l4 = 3000;

	result->dst6.l3.s6_addr32[0] = cpu_to_be32(0x0064ff9b);
	result->dst6.l3.s6_addr32[1] = 0;
	result->dst6.l3.s6_addr32[2] = 0;
	result->dst6.l3.s6_addr32[3] = cpu_to_be32(0xc0000200 | index);
	result->dst6.l4 = 80;

	result->src4.l3.s_addr = cpu_to_be32(0xcb007100 | index);
	result->src4.l4 = 4000;

	result->dst4.l3.s_addr = result->dst6.l3.s6_addr32[3];
	result->dst4.l4 = result->dst6.l4;

	result->proto = L4PROTO_TCP;
	result->state = 0;
	result->timer_type = SESSION_TIMER_TRANS;
	result->update_time = jiffies;
	result->timeout = 5000;
	result->has_stored = false;
}

static int init_sessions(void)
{
	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(ss); i++)
		init_session(i, &ss[i]);
	return 0;
}

static struct joold_queue *init_xlator(struct xlator *jool)
{
	jool->globals.nat64.joold.enabled = true;
	jool->globals.nat64.joold.flush_asap = false;
	jool->globals.nat64.joold.flush_deadline = 2000;
	jool->globals.nat64.joold.capacity = 4;
	jool->globals.nat64.joold.max_sessions_per_pkt = 3;
	jool->nat64.joold = joold_alloc();
	return jool->nat64.joold;
}

/********************** Asserts **********************/

static bool assert_deferred(struct joold_queue *joold, ...)
{
	struct session_entry *expected;
	struct deferred_session *actual;
	unsigned int count;
	va_list args;
	bool success = true;

	va_start(args, joold);

	count = 0;
	list_for_each_entry(actual, &joold->deferred.list, lh) {
		expected = va_arg(args, struct session_entry *);
		if (!expected) {
			log_err("Unexpected deferred session: " SEPP,
					SEPA(&actual->session));
			success = false;
			goto end;
		}

		success &= ASSERT_SESSION(expected, &actual->session, "listed");
		count++;
	}

	expected = va_arg(args, struct session_entry *);
	if (expected != NULL) {
		log_err("Session missing from deferred: " SEPP, SEPA(expected));
		success = false;
		goto end;
	}

	success &= ASSERT_UINT(count, joold->deferred.count, "count");

end:	va_end(args);
	return success;
}

static bool assert_skb(int garbage, ...)
{
	struct session_entry *expected, actual;
	struct nlattr *root, *attr;
	struct bib_config bibcfg;
	int rem;
	va_list args;
	bool success;
	int error;

	va_start(args, garbage);
	expected = va_arg(args, struct session_entry *);
	va_end(args);

	if (expected != NULL) {
		if (!ASSERT_NOTNULL(sent, "skb was sent"))
			return false;
	} else {
		return ASSERT_NULL(sent, "skb was not sent");
	}

	root = nlmsg_attrdata(nlmsg_hdr(sent), GENL_HDRLEN + JOOLNL_HDRLEN);
	success = ASSERT_UINT(JNLAR_SESSION_ENTRIES, nla_type(root), "root");

	memset(&bibcfg, 0, sizeof(bibcfg));
	bibcfg.ttl.tcp_est = 1000 * TCP_EST;
	bibcfg.ttl.tcp_trans = 1000 * TCP_TRANS;
	bibcfg.ttl.udp = 1000 * UDP_DEFAULT;
	bibcfg.ttl.icmp = 1000 * ICMP_DEFAULT;

	va_start(args, garbage);

	nla_for_each_nested(attr, root, rem) {
		error = jnla_get_session(attr, "session", &bibcfg, &actual);
		if (error) {
			log_err("jnla_get_session: errcode %d", error);
			success = false;
			goto end;
		}

		expected = va_arg(args, struct session_entry *);
		if (!expected) {
			log_err("Unexpected pkt session: " SEPP, SEPA(&actual));
			success = false;
			goto end;
		}

		success &= ASSERT_SESSION(expected, &actual, "packet'd");
	}

	expected = va_arg(args, struct session_entry *);
	if (expected != NULL) {
		log_err("Session missing from packet: " SEPP, SEPA(expected));
		success = false;
	}

end:	va_end(args);
	kfree_skb(sent);
	sent = NULL;
	return success;
}

/********************** Unit tests **********************/

/* No assertions, simply prints packet content sizes for future reference. */
static bool print_sizes(void)
{
	struct sk_buff *skb;
	struct joolnlhdr *jhdr;
	struct nlattr *root;
	struct session_entry dummy_session;
	size_t basic_size; /* NL header + GNL header + Jool header */
	size_t root_size;
	size_t session_size;

	skb = genlmsg_new(1000, GFP_ATOMIC);
	if (!skb)
		return true;

	jhdr = genlmsg_put(skb, 0, 0, jnl_family(), 0, 0);
	if (WARN(!jhdr, "genlmsg_put() returned NULL"))
		goto end;

	basic_size = skb->len;

	root = nla_nest_start(skb, JNLAR_SESSION_ENTRIES);
	if (WARN(!root, "nla_nest_start() returned NULL"))
		goto end;

	root_size = skb->len - basic_size;

	memset(&dummy_session, 0, sizeof(dummy_session));
	if (jnla_put_session(skb, JNLAL_ENTRY, &dummy_session) != 0)
		goto end;

	session_size = skb->len - basic_size - root_size;

	log_info("Kernel headers size: %zu", basic_size);
	log_info("Netlink attribute header size: %zu", root_size);
	log_info("Serialized session size: %zu", session_size);

end:	kfree_skb(skb);
	return true;
}

static bool test_no_flush_asap(void)
{
	struct xlator jool;
	struct joold_queue *joold;
	bool success = true;

	joold = init_xlator(&jool);
	if (!joold)
		return false;

	log_info("1");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags1");
	success &= assert_deferred(joold, &ss[0], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("2");
	joold_add(&jool, &ss[1]);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags2");
	success &= assert_deferred(joold, &ss[0], &ss[1], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("3");
	joold_add(&jool, &ss[2]);
	success &= ASSERT_UINT(0, joold->flags, "flags3");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], &ss[1], &ss[2], NULL);
	if (!success)
		goto end;

	/* Note: ACK not received yet */

	log_info("4");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(0, joold->flags, "flags1");
	success &= assert_deferred(joold, &ss[0], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("5");
	joold_add(&jool, &ss[1]);
	success &= ASSERT_UINT(0, joold->flags, "flags2");
	success &= assert_deferred(joold, &ss[0], &ss[1], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("6");
	joold_add(&jool, &ss[2]);
	success &= ASSERT_UINT(0, joold->flags, "flags3");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("7");
	joold_add(&jool, &ss[3]);
	success &= ASSERT_UINT(0, joold->flags, "flags4");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Capacity exceeded; drop new session */
	log_info("8");
	joold_add(&jool, &ss[4]);
	success &= ASSERT_UINT(0, joold->flags, "flags5");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* ACK */
	log_info("9");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags6");
	success &= assert_deferred(joold, &ss[3], NULL);
	success &= assert_skb(0, &ss[0], &ss[1], &ss[2], NULL);
	if (!success)
		goto end;

	/* ACK again */
	log_info("10");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags7");
	success &= assert_deferred(joold, &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Refill; make sure we're still stable after the ACK */
	log_info("11");
	joold_add(&jool, &ss[4]);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags8");
	success &= assert_deferred(joold, &ss[3], &ss[4], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("12");
	joold_add(&jool, &ss[5]);
	success &= ASSERT_UINT(0, joold->flags, "flags9");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[3], &ss[4], &ss[5], NULL);
	if (!success)
		goto end;

	/* Try an ACK on an empty joold */
	log_info("13");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags10");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);

end:	joold_put(joold);
	return success;
}

static bool test_flush_asap(void)
{
	struct xlator jool;
	struct joold_queue *joold;
	bool success = true;

	joold = init_xlator(&jool);
	if (!joold)
		return false;
	jool.globals.nat64.joold.flush_asap = true;

	/* Flush immediately */
	log_info("1");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(0, joold->flags, "flags1");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], NULL);
	if (!success)
		goto end;

	/* No ACK; postpone flush despite ss-flush-asap */
	log_info("2");
	joold_add(&jool, &ss[1]);
	success &= ASSERT_UINT(0, joold->flags, "flags2");
	success &= assert_deferred(joold, &ss[1], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* ACK; flush */
	log_info("3");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags3");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[1], NULL);
	if (!success)
		goto end;

	/* Reach capacity */
	log_info("4");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(0, joold->flags, "flags4");
	success &= assert_deferred(joold, &ss[0], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("5");
	joold_add(&jool, &ss[1]);
	success &= ASSERT_UINT(0, joold->flags, "flags5");
	success &= assert_deferred(joold, &ss[0], &ss[1], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("6");
	joold_add(&jool, &ss[2]);
	success &= ASSERT_UINT(0, joold->flags, "flags6");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("7");
	joold_add(&jool, &ss[3]);
	success &= ASSERT_UINT(0, joold->flags, "flags7");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Capacity reached; drop session */
	log_info("8");
	joold_add(&jool, &ss[4]);
	success &= ASSERT_UINT(0, joold->flags, "flags8");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Again */
	log_info("9");
	joold_add(&jool, &ss[5]);
	success &= ASSERT_UINT(0, joold->flags, "flags9");
	success &= assert_deferred(joold, &ss[0], &ss[1], &ss[2], &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* ACK, finally */
	log_info("10");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags10");
	success &= assert_deferred(joold, &ss[3], NULL);
	success &= assert_skb(0, &ss[0], &ss[1], &ss[2], NULL);
	if (!success)
		goto end;

	/* Again */
	log_info("11");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags11");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[3], NULL);
	if (!success)
		goto end;

	/* Again */
	log_info("12");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags12");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Flush, ACK, flush */
	log_info("13");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(0, joold->flags, "flags13");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], NULL);
	if (!success)
		goto end;

	log_info("14");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags14");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("15");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(0, joold->flags, "flags15");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], NULL);

end:	joold_put(joold);
	return success;
}

static bool test_advertise(void)
{
	struct xlator jool;
	struct joold_queue *joold;
	bool success = true;

	joold = init_xlator(&jool);
	if (!joold)
		goto end;

	/* Empty advertise on startup */
	log_info("1");
	foreach_end = 0;
	joold_advertise(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags1");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Single session advertise */
	log_info("2");
	foreach_end = 1;
	joold_advertise(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags2");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], NULL);
	if (!success)
		goto end;

	/* Single session advertise, postponed because no ACK */
	log_info("3");
	joold_advertise(&jool);
	success &= ASSERT_UINT(JQF_AD_ONGOING, joold->flags, "flags3");
	success &= assert_deferred(joold, &ss[0], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* ACK */
	log_info("4");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags4");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], NULL);
	if (!success)
		goto end;

	/* Enable JQF_ACK_RECEIVED */
	log_info("5");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags5");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Full packet advertise */
	log_info("6");
	foreach_end = 3;
	joold_advertise(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags6");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[0], &ss[1], &ss[2], NULL);
	if (!success)
		goto end;

	/* Enable JQF_ACK_RECEIVED */
	log_info("7");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags7");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Advertise enough sessions to need 2 packets */
	log_info("8");
	foreach_end = 4;
	joold_advertise(&jool);
	success &= ASSERT_UINT(JQF_AD_ONGOING, joold->flags, "flags8");
	success &= assert_deferred(joold, &ss[3], NULL);
	success &= assert_skb(0, &ss[0], &ss[1], &ss[2], NULL);
	if (!success)
		goto end;

	/* Make sure advertises don't stack */
	log_info("9");
	joold_advertise(&jool);
	success &= ASSERT_UINT(JQF_AD_ONGOING, joold->flags, "flags9");
	success &= assert_deferred(joold, &ss[3], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	/* Send 2nd packet */
	log_info("10");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags10");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[3], NULL);
	if (!success)
		goto end;

	/* Large advertise, and joold isn't empty */
	log_info("11");
	joold_add(&jool, &ss[0]);
	success &= ASSERT_UINT(0, joold->flags, "flags11");
	success &= assert_deferred(joold, &ss[0], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("12");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags12");
	success &= assert_deferred(joold, &ss[0], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("13");
	joold_add(&jool, &ss[1]);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags13");
	success &= assert_deferred(joold, &ss[0], &ss[1], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("14");
	foreach_start = 2;
	foreach_end = 8;
	joold_advertise(&jool);
	success &= ASSERT_UINT(JQF_AD_ONGOING, joold->flags, "flags14");
	success &= assert_deferred(joold, &ss[3], &ss[4], &ss[5], &ss[6],
			&ss[7], NULL);
	success &= assert_skb(0, &ss[0], &ss[1], &ss[2], NULL);
	if (!success)
		goto end;

	log_info("15");
	joold_add(&jool, &ss[8]);
	success &= ASSERT_UINT(JQF_AD_ONGOING, joold->flags, "flags15");
	success &= assert_deferred(joold, &ss[3], &ss[4], &ss[5], &ss[6],
			&ss[7], &ss[8], NULL);
	success &= assert_skb(0, NULL);
	if (!success)
		goto end;

	log_info("16");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_AD_ONGOING, joold->flags, "flags16");
	success &= assert_deferred(joold, &ss[6], &ss[7], &ss[8], NULL);
	success &= assert_skb(0, &ss[3], &ss[4], &ss[5], NULL);
	if (!success)
		goto end;

	log_info("17");
	joold_ack(&jool);
	success &= ASSERT_UINT(0, joold->flags, "flags17");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, &ss[6], &ss[7], &ss[8], NULL);
	if (!success)
		goto end;

	log_info("18");
	joold_ack(&jool);
	success &= ASSERT_UINT(JQF_ACK_RECEIVED, joold->flags, "flags18");
	success &= assert_deferred(joold, NULL);
	success &= assert_skb(0, NULL);

end:	joold_put(joold);
	return success;
}

/********************** Hooks **********************/

int init_module(void)
{
	struct test_group test = {
		.name = "joold",
		.setup_fn = init_sessions,
	};

	if (test_group_begin(&test))
		return -EINVAL;
	test_group_test(&test, print_sizes, "print sizes");
	test_group_test(&test, test_no_flush_asap, "ss-flush-asap disabled");
	test_group_test(&test, test_flush_asap, "ss-flush-asap enabled");
	test_group_test(&test, test_advertise, "advertise");
	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
