/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2013 Thomas Graf <tgraf@suug.ch>
 */

#include "nl-default.h"

#include <linux/netlink.h>
#include <linux/if_ether.h>

#include <netlink/attr.h>
#include <netlink/msg.h>
#include <netlink/route/cls/u32.h>

#include "cksuite-all.h"
#include "nl-aux-route/nl-route.h"

START_TEST(attr_size)
{
	ck_assert_msg(nla_attr_size(0) == NLA_HDRLEN,
		      "Length of empty attribute should match header size");
	ck_assert_msg(nla_attr_size(1) == NLA_HDRLEN + 1,
		      "Length of 1 bytes payload should be NLA_HDRLEN + 1");
	ck_assert_msg(nla_attr_size(2) == NLA_HDRLEN + 2,
		      "Length of 2 bytes payload should be NLA_HDRLEN + 2");
	ck_assert_msg(nla_attr_size(3) == NLA_HDRLEN + 3,
		      "Length of 3 bytes payload should be NLA_HDRLEN + 3");
	ck_assert_msg(nla_attr_size(4) == NLA_HDRLEN + 4,
		      "Length of 4 bytes payload should be NLA_HDRLEN + 4");

	ck_assert_msg(nla_total_size(1) == NLA_HDRLEN + 4,
		      "Total size of 1 bytes payload should result in 8 bytes");
	ck_assert_msg(nla_total_size(2) == NLA_HDRLEN + 4,
		      "Total size of 2 bytes payload should result in 8 bytes");
	ck_assert_msg(nla_total_size(3) == NLA_HDRLEN + 4,
		      "Total size of 3 bytes payload should result in 8 bytes");
	ck_assert_msg(nla_total_size(4) == NLA_HDRLEN + 4,
		      "Total size of 4 bytes payload should result in 8 bytes");

	ck_assert_msg(nla_padlen(1) == 3,
		      "2 bytes of payload should result in 3 padding bytes");
	ck_assert_msg(nla_padlen(2) == 2,
		      "2 bytes of payload should result in 2 padding bytes");
	ck_assert_msg(nla_padlen(3) == 1,
		      "3 bytes of payload should result in 1 padding bytes");
	ck_assert_msg(nla_padlen(4) == 0,
		      "4 bytes of payload should result in 0 padding bytes");
	ck_assert_msg(nla_padlen(5) == 3,
		      "5 bytes of payload should result in 3 padding bytes");
}
END_TEST

START_TEST(msg_construct)
{
	struct nl_msg *msg;
	struct nlmsghdr *nlh;
	struct nlattr *a;
	int i, rem;

	msg = nlmsg_alloc();
	ck_assert_msg(msg, "Unable to allocate netlink message");

	for (i = 1; i < 256; i++) {
		ck_assert_msg(nla_put_u32(msg, i, i + 1) == 0,
			      "Unable to add attribute %d", i);
	}

	nlh = nlmsg_hdr(msg);
	i = 1;
	nlmsg_for_each_attr(a, nlh, 0, rem) {
		ck_assert_msg(nla_type(a) == i, "Expected attribute %d", i);
		i++;
		ck_assert_msg(nla_get_u32(a) == i,
			      "Expected attribute value %d", i);
	}

	nlmsg_free(msg);
}
END_TEST

START_TEST(clone_cls_u32)
{
	_nl_auto_rtnl_link struct rtnl_link *link = NULL;
	_nl_auto_rtnl_cls struct rtnl_cls *cls = NULL;
	_nl_auto_rtnl_cls struct rtnl_cls *cls2 = NULL;
	int r;
	const uint32_t direction = 16;

	link = rtnl_link_alloc();
	ck_assert(link);

	rtnl_link_set_ifindex(link, 5);

	cls = rtnl_cls_alloc();
	ck_assert(cls);

	rtnl_tc_set_link(TC_CAST(cls), link);

	r = rtnl_tc_set_kind(TC_CAST(cls), "u32");
	ck_assert(r == 0);

	rtnl_cls_set_prio(cls, 1);
	rtnl_cls_set_protocol(cls, ETH_P_IP);

	rtnl_tc_set_parent(TC_CAST(cls), TC_HANDLE(1, 0));

	rtnl_u32_set_hashtable(cls, 5);

	rtnl_u32_add_key_uint32(cls, 0x0a000914, 0xffffffff, direction, 0);

	rtnl_u32_set_hashmask(cls, 0xff000000, direction);

	rtnl_u32_add_mark(cls, 55, 66);

	rtnl_u32_set_link(cls, 44);

	cls2 = (struct rtnl_cls *)nl_object_clone((struct nl_object *)cls);
	ck_assert(cls2);
}
END_TEST

/*****************************************************************************/

START_TEST(test_nltst_strtok)
{
#define _assert_strtok(str, ...)                                               \
	do {                                                                   \
		const char *const _expected[] = { NULL, ##__VA_ARGS__, NULL }; \
		_nltst_auto_strfreev char **_tokens = NULL;                    \
                                                                               \
		_tokens = _nltst_strtokv(str);                                 \
		_nltst_assert_strv_equal(_tokens, &_expected[1]);              \
	} while (0)

	_assert_strtok("");
	_assert_strtok("    \n");
	_assert_strtok("a", "a");
	_assert_strtok(" a ", "a");
	_assert_strtok(" a\\  b", "a\\ ", "b");
	_assert_strtok(" a\\  b   cc\\d", "a\\ ", "b", "cc\\d");
	_assert_strtok(" a\\  b\\   cc\\d", "a\\ ", "b\\ ", "cc\\d");
}
END_TEST

/*****************************************************************************/

Suite *make_nl_attr_suite(void)
{
	Suite *suite = suite_create("Netlink attributes");
	TCase *tc = tcase_create("Core");

	tcase_add_test(tc, attr_size);
	tcase_add_test(tc, msg_construct);
	tcase_add_test(tc, clone_cls_u32);
	tcase_add_test(tc, test_nltst_strtok);
	suite_add_tcase(suite, tc);

	return suite;
}
