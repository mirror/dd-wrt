/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Author: Susant Sahani <susant@redhat.com>
 * Copyright (c) 2018 Red Hat, Inc.
 */

#include "nl-default.h"

#include <linux/netlink.h>

#include <netlink/route/link.h>
#include <netlink/route/link/sit.h>
#include <netlink/route/link/bonding.h>
#include <netlink/route/link/bridge.h>
#include <netlink/route/link/ip6tnl.h>
#include <netlink/route/link/ipgre.h>
#include <netlink/route/link/ipip.h>
#include <netlink/route/link/ipvlan.h>
#include <netlink/route/link/ipvti.h>
#include <netlink/route/link/macsec.h>
#include <netlink/route/link/macvlan.h>
#include <netlink/route/link/macvtap.h>
#include <netlink/route/link/veth.h>
#include <netlink/route/link/vlan.h>
#include <netlink/route/link/vrf.h>
#include <netlink/route/link/vxlan.h>

#include "cksuite-all.h"

/*****************************************************************************/

static void _nltst_delete_link2(const char *ifname)
{
	_nltst_delete_link(NULL, ifname);
}
#define _nltst_auto_delete_link _nl_auto(_nltst_auto_delete_link_fcn)
_NL_AUTO_DEFINE_FCN_TYPED0(const char *, _nltst_auto_delete_link_fcn,
			   _nltst_delete_link2);

/*****************************************************************************/

START_TEST(cache_and_clone)
{
	_nl_auto_nl_socket struct nl_sock *sk = NULL;
	_nl_auto_nl_cache struct nl_cache *link_cache = NULL;
	_nl_auto_free struct nl_object **links_all = NULL;
	static const struct {
		const char *ifname;
		const char *kind;
		bool add;
	} links[] = {
		{
			.ifname = "xbr0",
			.kind = "bridge",
			.add = true,
		},
		{
			.ifname = "xdummy0",
			.kind = "dummy",
			.add = true,
		},
		{
			.ifname = "xbond0",
			.kind = "bond",
			.add = true,
		},
		{
			.ifname = "lo",
			.kind = NULL,
			.add = false,
		},
	};
	int i;
	int r;

	for (i = 0; i < _NL_N_ELEMENTS(links); i++) {
		if (links[i].add)
			_nltst_add_link(NULL, links[i].ifname, links[i].kind,
					NULL);
	}

	sk = _nltst_socket(NETLINK_ROUTE);

	r = rtnl_link_alloc_cache(sk, AF_UNSPEC, &link_cache);
	ck_assert_int_eq(r, 0);

	r = nl_cache_refill(sk, link_cache);
	ck_assert_int_eq(r, 0);

	for (i = 0; i < _NL_N_ELEMENTS(links); i++) {
		_nl_auto_rtnl_link struct rtnl_link *link_clone = NULL;
		struct rtnl_link *link;

		link = _nltst_cache_get_link(link_cache, links[i].ifname);
		ck_assert_ptr_nonnull(link);

		ck_assert_str_eq(rtnl_link_get_name(link), links[i].ifname);

		if (_nl_streq(links[i].ifname, "lo"))
			ck_assert_int_eq(rtnl_link_get_ifindex(link), 1);
		else
			ck_assert_int_gt(rtnl_link_get_ifindex(link), 1);

		link_clone = (struct rtnl_link *)nl_object_clone(
			(struct nl_object *)link);
		ck_assert(link_clone);

		_nltst_object_identical(link, link_clone);
	}

	links_all = _nltst_cache_get_all(link_cache, NULL);
	for (i = 0; links_all[i]; i++) {
		struct rtnl_link *link = (struct rtnl_link *)links_all[i];
		_nl_auto_rtnl_link struct rtnl_link *link_clone = NULL;

		link_clone = (struct rtnl_link *)nl_object_clone(
			(struct nl_object *)link);
		ck_assert(link_clone);

		_nltst_object_identical(link, link_clone);
	}
}
END_TEST

/*****************************************************************************/

START_TEST(test_create_iface)
{
	const int TEST_IDX = _i;
	_nl_auto_nl_socket struct nl_sock *sk = _nltst_socket(NETLINK_ROUTE);
	_nl_auto_rtnl_link struct rtnl_link *link = NULL;
	_nl_auto_rtnl_link struct rtnl_link *link2 = NULL;
	_nl_auto_rtnl_link struct rtnl_link *peer = NULL;
	_nltst_auto_delete_link const char *IFNAME_DUMMY = NULL;
	_nltst_auto_delete_link const char *IFNAME = "ifname";
	int ifindex_dummy;
	uint32_t u32;
	int r;

	switch (TEST_IDX) {
	case 0:
		link = _nltst_assert(rtnl_link_bridge_alloc());
		rtnl_link_set_name(link, IFNAME);
		break;
	case 1:
		link = _nltst_assert(rtnl_link_vxlan_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_vxlan_set_id(link, 128));
		break;
	case 2:
		link = _nltst_assert(rtnl_link_alloc());
		rtnl_link_set_type(link, "ifb");
		rtnl_link_set_name(link, IFNAME);
		break;
	case 3:
		link = _nltst_assert(rtnl_link_ipgre_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_ipgre_set_local(
			link, _nltst_inet4("192.168.254.12")));
		_nltst_assert_retcode(rtnl_link_ipgre_set_remote(
			link, _nltst_inet4("192.168.254.13")));
		_nltst_assert_retcode(rtnl_link_ipgre_set_ttl(link, 64));
		break;
	case 4:
		link = _nltst_assert(rtnl_link_ip6_tnl_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_ip6_tnl_set_local(
			link, _nltst_inet6p("2607:f0d0:1002:51::4")));
		_nltst_assert_retcode(rtnl_link_ip6_tnl_set_remote(
			link, _nltst_inet6p("2607:f0d0:1002:52::5")));
		break;
	case 5:
		link = _nltst_assert(rtnl_link_ipgretap_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_ipgre_set_local(
			link, _nltst_inet4("10.211.55.10")));
		_nltst_assert_retcode(rtnl_link_ipgre_set_remote(
			link, _nltst_inet4("10.133.6.33")));
		_nltst_assert_retcode(rtnl_link_ipgre_set_ttl(link, 64));
		break;
	case 6:
		link = _nltst_assert(rtnl_link_ipvti_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_ipvti_set_local(
			link, _nltst_inet4("192.168.254.12")));
		_nltst_assert_retcode(rtnl_link_ipvti_set_remote(
			link, _nltst_inet4("192.168.254.13")));
		break;
	case 7:
		link = _nltst_assert(rtnl_link_sit_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_sit_set_local(
			link, _nltst_inet4("192.168.254.12")));
		_nltst_assert_retcode(rtnl_link_sit_set_remote(
			link, _nltst_inet4("192.168.254.13")));
		_nltst_assert_retcode(rtnl_link_sit_set_ttl(link, 64));
		break;
	case 8:
		link = _nltst_assert(rtnl_link_ipip_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_ipip_set_local(
			link, _nltst_inet4("192.168.254.12")));
		_nltst_assert_retcode(rtnl_link_ipip_set_remote(
			link, _nltst_inet4("192.168.254.13")));
		_nltst_assert_retcode(rtnl_link_ipip_set_ttl(link, 64));
		break;
	case 9:
		link = _nltst_assert(rtnl_link_bond_alloc());
		rtnl_link_set_name(link, IFNAME);
		break;
	case 10:
		IFNAME_DUMMY = "ci-dummy";
		_nltst_add_link(sk, IFNAME_DUMMY, "dummy", &ifindex_dummy);

		link = _nltst_assert(rtnl_link_macvtap_alloc());
		rtnl_link_set_link(link, ifindex_dummy);
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_macvtap_set_mode(
			link, rtnl_link_macvtap_str2mode("bridge")));
		break;
	case 11:
		IFNAME_DUMMY = "ci-dummy";
		_nltst_add_link(sk, IFNAME_DUMMY, "dummy", &ifindex_dummy);

		link = _nltst_assert(rtnl_link_macvlan_alloc());
		rtnl_link_set_link(link, ifindex_dummy);
		rtnl_link_set_name(link, IFNAME);
		break;
	case 12:
		IFNAME_DUMMY = "ci-dummy";
		_nltst_add_link(sk, IFNAME_DUMMY, "dummy", &ifindex_dummy);

		link = _nltst_assert(rtnl_link_vlan_alloc());
		rtnl_link_set_link(link, ifindex_dummy);
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_vlan_set_id(link, 10));
		break;
	case 13:
		IFNAME_DUMMY = "ci-dummy";
		_nltst_add_link(sk, IFNAME_DUMMY, "dummy", &ifindex_dummy);

		link = _nltst_assert(rtnl_link_macsec_alloc());
		rtnl_link_set_link(link, ifindex_dummy);
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_macsec_set_port(link, 10));
		_nltst_assert_retcode(rtnl_link_macsec_set_encrypt(link, 1));
		_nltst_assert_retcode(
			rtnl_link_macsec_set_replay_protect(link, 1));
		_nltst_assert_retcode(rtnl_link_macsec_set_window(link, 200));
		break;
	case 14:
		IFNAME_DUMMY = "ci-dummy";
		_nltst_add_link(sk, IFNAME_DUMMY, "dummy", &ifindex_dummy);

		link = _nltst_assert(rtnl_link_ipvlan_alloc());
		rtnl_link_set_link(link, ifindex_dummy);
		_nltst_assert_retcode(rtnl_link_ipvlan_set_mode(
			link, rtnl_link_ipvlan_str2mode("l2")));
		rtnl_link_set_name(link, IFNAME);
		break;
	case 15:
		link = _nltst_assert(rtnl_link_vrf_alloc());
		rtnl_link_set_name(link, IFNAME);
		_nltst_assert_retcode(rtnl_link_vrf_set_tableid(link, 10));
		break;
	case 16: {
		link = _nltst_assert(rtnl_link_veth_alloc());
		rtnl_link_set_name(link, IFNAME);
		peer = _nltst_assert(rtnl_link_veth_get_peer(link));
		rtnl_link_set_name(peer, "ci-veth-peer");
	} break;
	default:
		ck_assert_msg(0, "unexpected TEST_IDX=%d", _i);
		break;
	}

	r = rtnl_link_add(sk, link, NLM_F_CREATE);
	if (r == -NLE_OPNOTSUPP) {
		/* Hm, no kernel module? Skip the test. */
		_nltst_assert_link_not_exists(IFNAME);
		IFNAME = NULL;
		return;
	}
	_nltst_assert_retcode(r);

	_nltst_assert_link_exists(IFNAME);

	switch (TEST_IDX) {
	case 15:
		_nltst_get_link(sk, IFNAME, NULL, &link2);
		_nltst_assert_retcode(rtnl_link_vrf_get_tableid(link2, &u32));
		ck_assert_int_eq(u32, 10);
		break;
	case 16:
		_nltst_assert_link_exists("ci-veth-peer");
		if (_nltst_rand_bool())
			IFNAME = "ci-veth-peer";
		break;
	}
}
END_TEST

/*****************************************************************************/

Suite *make_nl_netns_suite(void)
{
	Suite *suite = suite_create("netns");
	TCase *tc = tcase_create("Core");

	tcase_add_checked_fixture(tc, nltst_netns_fixture_setup,
				  nltst_netns_fixture_teardown);
	tcase_add_test(tc, cache_and_clone);
	tcase_add_loop_test(tc, test_create_iface, 0, 17);

	suite_add_tcase(suite, tc);

	return suite;
}
