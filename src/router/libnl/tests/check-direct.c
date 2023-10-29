/* SPDX-License-Identifier: LGPL-2.1-only */

#include "nl-default.h"

#include <check.h>

#include <linux/snmp.h>

#include <netlink/route/link.h>

#include "nl-priv-static-route/nl-priv-static-route.h"

START_TEST(static_checks)
{
	int i, j;
	char strbuf[100];

	_NL_STATIC_ASSERT(RTNL_LINK_RX_PACKETS == 0);
	assert(_nltst_map_stat_id_from_IPSTATS_MIB_v2[0] ==
	       RTNL_LINK_RX_PACKETS);
	for (i = 1; i < __IPSTATS_MIB_MAX; i++) {
		assert(_nltst_map_stat_id_from_IPSTATS_MIB_v2[i] > 0);
		assert(_nltst_map_stat_id_from_IPSTATS_MIB_v2[i] <
		       __RTNL_LINK_STATS_MAX);
		for (j = 1; j < i; j++)
			assert(_nltst_map_stat_id_from_IPSTATS_MIB_v2[i] !=
			       _nltst_map_stat_id_from_IPSTATS_MIB_v2[j]);
	}

	for (i = 0; i <= RTNL_LINK_STATS_MAX + 1; i++) {
		const char *s;

		s = rtnl_link_stat2str(i, strbuf, sizeof(strbuf));
		assert(s);
		assert(s == strbuf);
		assert(strlen(s) < sizeof(strbuf));
		if (strncmp(s, "0x", 2) == 0) {
			assert(i == RTNL_LINK_STATS_MAX + 1);
			ck_assert_int_eq(strtoll(&s[2], NULL, 16), i);
		} else
			ck_assert_int_le(i, RTNL_LINK_STATS_MAX);
		ck_assert_int_eq(i, rtnl_link_str2stat(s));
	}

	ck_assert_int_eq(nl_str2ip_proto(""), -NLE_OBJ_NOTFOUND);
	ck_assert_int_eq(nl_str2ip_proto("5"), 5);
	ck_assert_int_eq(nl_str2ip_proto("  13 "), -NLE_OBJ_NOTFOUND);
	ck_assert_int_eq(nl_str2ip_proto("13"), 13);
	ck_assert_int_eq(nl_str2ip_proto("0x13"), 0x13);
	ck_assert_int_eq(nl_str2ip_proto("0342"), 0342);
	ck_assert_int_eq(nl_str2ip_proto("2147483647"), 2147483647);
	ck_assert_int_eq(nl_str2ip_proto("2147483648"), -NLE_OBJ_NOTFOUND);
}
END_TEST

static Suite *make_suite(void)
{
	Suite *suite = suite_create("Direct");
	TCase *tc = tcase_create("Core");

	tcase_add_test(tc, static_checks);
	suite_add_tcase(suite, tc);
	return suite;
}

int main(int argc, char *argv[])
{
	SRunner *runner;
	int nfailed;

	runner = srunner_create(suite_create("main"));

	srunner_add_suite(runner, make_suite());

	srunner_run_all(runner, CK_ENV);

	nfailed = srunner_ntests_failed(runner);
	srunner_free(runner);
	return nfailed != 0;
}
