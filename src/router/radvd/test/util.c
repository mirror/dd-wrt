
#include <check.h>

/*
 * http://check.sourceforge.net/doc/check_html/check_3.html
 *
 * http://entrenchant.blogspot.com/2010/08/unit-testing-in-c.html
 */

START_TEST (test_safe_buffer)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	ck_assert_ptr_eq(0, sb.buffer);
	ck_assert_int_eq(0, sb.allocated);
	ck_assert_int_eq(0, sb.used);
	ck_assert_int_eq(0, sb.should_free);
	safe_buffer_free(&sb);

	struct safe_buffer * sbptr = new_safe_buffer();
	ck_assert_ptr_eq(0, sbptr->buffer);
	ck_assert_int_eq(0, sbptr->allocated);
	ck_assert_int_eq(0, sbptr->used);
	ck_assert_int_ne(0, sbptr->should_free);
	safe_buffer_free(sbptr);
}
END_TEST

START_TEST (test_safe_buffer_append)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	char array[] = {"This is a test"};
	safe_buffer_append(&sb, array, sizeof(array));
	ck_assert_str_eq(sb.buffer, array);
	ck_assert_int_eq(sb.used, sizeof(array));
	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_safe_buffer_append2)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	char array[] = {"This is a test"};
	safe_buffer_append(&sb, array, sizeof(array));
	safe_buffer_append(&sb, array, sizeof(array));
	ck_assert_str_eq(sb.buffer, array);
	ck_assert_str_eq(sb.buffer + sizeof(array), array);
	ck_assert_int_eq(sb.used, 2*sizeof(array));
	
	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_safe_buffer_pad)
{
	struct safe_buffer sb = SAFE_BUFFER_INIT;
	char array[] = {"This is a test"};
	safe_buffer_append(&sb, array, sizeof(array));
	safe_buffer_pad(&sb, 10);
	ck_assert_str_eq(sb.buffer, array);
	ck_assert_int_eq(sb.used, 10+sizeof(array));
	
	safe_buffer_free(&sb);
}
END_TEST

START_TEST (test_addrtostr)
{
	char buffer[INET6_ADDRSTRLEN] = {""};
	struct in6_addr addr = {
		{
			0xfe, 0x80, 0xfe, 0x80,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0xff, 0x00, 0x12, 0x34,
		},
	};
	addrtostr(&addr, buffer, sizeof(buffer));
	ck_assert_str_eq(buffer, "fe80:fe80::ff00:1234");
}
END_TEST

START_TEST (test_addrtostr_overflow)
{
	char buffer[18] = {""};
	struct in6_addr addr = {
		{
			0xfe, 0x80, 0xfe, 0x80,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0xff, 0x00, 0x12, 0x34,
		},
	};
	addrtostr(&addr, buffer, sizeof(buffer));
	ck_assert_str_eq(buffer, "[invalid address]");
}
END_TEST

START_TEST (test_strdupf)
{
	char * str = strdupf("%d %s %zu %c %% char", 1234, "str", (size_t)10, 'c');

	ck_assert_str_eq(str, "1234 str 10 c % char");

	free(str);
}
END_TEST

START_TEST (test_readn)
{
	int fd = open("/dev/zero", O_RDONLY);

	ck_assert_int_ne(fd, -1);

	char buffer[10000];

	memset(buffer, 1, sizeof(buffer));

	int count = readn(fd, buffer, sizeof(buffer));

	ck_assert_int_eq(count, 10000);
	
	for (int i = 0; i < sizeof(buffer); ++i) {
		ck_assert_int_eq(buffer[i], 0);
	}
}
END_TEST

START_TEST (test_writen)
{
	int fd = open("/dev/null", O_WRONLY);

	ck_assert_int_ne(fd, -1);

	char buffer[10000];

	memset(buffer, 1, sizeof(buffer));

	int count = writen(fd, buffer, sizeof(buffer));

	ck_assert_int_eq(count, 10000);
}
END_TEST

static struct Interface * iface = 0;

static void iface_setup(void)
{
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test1.conf");
	ck_assert_ptr_ne(0, iface);
}

static void iface_teardown(void)
{
	ck_assert_ptr_ne(0, iface);
	free_ifaces(iface);
	iface = 0;
}

START_TEST (test_check_dnssl_presence)
{
	int rc = check_dnssl_presence(iface->AdvDNSSLList, "example.com");
	ck_assert_int_ne(0, rc);

	rc = check_dnssl_presence(iface->AdvDNSSLList, "office.branch.example.net");
	ck_assert_int_ne(0, rc);

	rc = check_dnssl_presence(iface->AdvDNSSLList, "example.au");
	ck_assert_int_eq(0, rc);
}
END_TEST

START_TEST (test_check_rdnss_presence)
{
	struct in6_addr addr;
	int rc;

	/* The next three should be found */
	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
	rc = check_rdnss_presence(iface->AdvRDNSSList, &addr);
	ck_assert_int_ne(0, rc);

	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 };
	rc = check_rdnss_presence(iface->AdvRDNSSList, &addr);
	ck_assert_int_ne(0, rc);

	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3 };
	rc = check_rdnss_presence(iface->AdvRDNSSList, &addr);
	ck_assert_int_ne(0, rc);

	/* The next one should *not* be found */
	addr = (struct in6_addr){ 0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 };
	rc = check_rdnss_presence(iface->AdvRDNSSList, &addr);
	ck_assert_int_eq(0, rc);
}
END_TEST

START_TEST (test_rand_between)
{
	int const RAND_TEST_MAX = 1000;
	for (int i = 0; i < RAND_TEST_MAX; ++i) {
		double const min = -1;
		double const max = 1;
		double d = rand_between(min, max);
		ck_assert(min <= d);
		ck_assert(max >= d);
	}
	for (int i = 0; i < RAND_TEST_MAX; ++i) {
		double const min = -1000;
		double const max = 1000;
		double d = rand_between(min, max);
		ck_assert(min <= d);
		ck_assert(max >= d);
	}
	for (int i = 0; i < RAND_TEST_MAX; ++i) {
		double const min = 100;
		double const max = 200;
		double d = rand_between(min, max);
		ck_assert(min <= d);
		ck_assert(max >= d);
	}
}
END_TEST

Suite * util_suite(void)
{
	TCase * tc_safe_buffer = tcase_create("safe_buffer");
	tcase_add_test(tc_safe_buffer, test_safe_buffer);
	tcase_add_test(tc_safe_buffer, test_safe_buffer_append);
	tcase_add_test(tc_safe_buffer, test_safe_buffer_append2);
	tcase_add_test(tc_safe_buffer, test_safe_buffer_pad);

	TCase * tc_str = tcase_create("str");
	tcase_add_test(tc_str, test_addrtostr);
	tcase_add_test(tc_str, test_addrtostr_overflow);
	tcase_add_test(tc_str, test_strdupf);

	TCase * tc_ion = tcase_create("ion");
	tcase_add_test(tc_ion, test_readn);
	tcase_add_test(tc_ion, test_writen);

	TCase * tc_presence = tcase_create("presence");
	tcase_add_unchecked_fixture(tc_presence, iface_setup, iface_teardown);
	tcase_add_test(tc_presence, test_check_dnssl_presence);
	tcase_add_test(tc_presence, test_check_rdnss_presence);

	TCase * tc_misc = tcase_create("misc");
	tcase_add_test(tc_misc, test_rand_between);

	Suite *s = suite_create("util");
	suite_add_tcase(s, tc_safe_buffer);
	suite_add_tcase(s, tc_str);
	suite_add_tcase(s, tc_ion);
	suite_add_tcase(s, tc_presence);
	suite_add_tcase(s, tc_misc);

	return s;	
}

