
#include "test/print_safe_buffer.h"
#include <check.h>

/*
 * https://libcheck.github.io/check/
 * https://libcheck.github.io/check/doc/check_html/check_3.html
 * http://entrenchant.blogspot.com/2010/08/unit-testing-in-c.html
 */

// Used for some hand-written testcases
#define REP4(c) c, c, c, c
#define REP8(c) REP4(c), REP4(c)
#define REP16(c) REP8(c), REP8(c)
// RFC1035 section 4.1.4 encoding; [len, (data)]+, trailing len=0
// 62 chars, plus leading length & trailing length = 64 bytes
#define RFC1035_DNS_HACK_64BYTE(c) 0x3e, REP16(c), REP16(c), REP16(c), REP8(c), REP4(c), c, c, 0x00

START_TEST(test_decrement_lifetime)
{
	uint32_t lifetime = 10;
	decrement_lifetime(7, &lifetime);
	ck_assert_int_eq(lifetime, 3);
	decrement_lifetime(7, &lifetime);
	ck_assert_int_eq(lifetime, 0);
}
END_TEST

static struct Interface *iface = 0;

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

START_TEST(test_add_ra_header_cease_adv0)
{

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_header(&sb, &iface->ra_header_info, 0);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n// ra_header_info->AdvDefaultLifetime = %x\n%s", iface->ra_header_info.AdvDefaultLifetime, (char*)&buf);
#else
	// Lifetime should be -1/ffff in this case, because we have not set it to anything else.
	// interface.c:40:iface->ra_header_info.AdvDefaultLifetime = -1;
	unsigned char expected[] = {
		// nd_ra_type
		// nd_ra_code
		0x86, 0x00,
		// nd_ra_cksum
		0x00, 0x00,
		// nd_ra_curhoplimit
		0x40,
		// nd_ra_flags_reserved
		0x00,
		// nd_ra_router_lifetime
		0xff, 0xff,
		// nd_ra_reachable
		0x00, 0x00, 0x00, 0x00,
		// nd_ra_retransmit
		0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_header_cease_adv1)
{

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_header(&sb, &iface->ra_header_info, 1);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		// nd_ra_type
		// nd_ra_code
		0x86, 0x00,
		// nd_ra_cksum
		0x00, 0x00,
		// nd_ra_curhoplimit
		0x40,
		// nd_ra_flags_reserved
		0x00,
		// nd_ra_router_lifetime
		0x00, 0x00,
		// nd_ra_reachable
		0x00, 0x00, 0x00, 0x00,
		// nd_ra_retransmit
		0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_prefix)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_prefix(sbl, iface, iface->props.name, iface->AdvPrefixList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	// https://www.rfc-editor.org/rfc/rfc4861.html#section-4.6.2
	unsigned char expected[] = {
		// prefix fe80:1::/64
		0x03, 0x04, 0x40, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0x80, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// prefix fe80:2::/48
		0x03, 0x04, 0x30, 0x80, 0x00, 0x00, 0x27, 0x10, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// prefix fe80:2::/64
		0x03, 0x04, 0x40, 0xc0, 0x00, 0x01, 0x51, 0x80, 0x00, 0x00, 0x38, 0x40, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_route)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_route(sbl, iface, iface->AdvRouteList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	// https://www.rfc-editor.org/rfc/rfc4191.html#section-2.3
	unsigned char expected[] = {
		// fe80:f:1::/48
		0x18, 0x02, 0x30, 0x18, 0x00, 0x00, 0x27, 0x10, 0xfe, 0x80, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x00,
		// fe80:f:2::/40
		0x18, 0x02, 0x28, 0x08, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x80, 0x00, 0x0f, 0x00, 0x02, 0x00, 0x00,
		// fe80:f:2::/32
		0x18, 0x02, 0x20, 0x00, 0x00, 0x00, 0x0b, 0xb8, 0xfe, 0x80, 0x00, 0x0f, 0x00, 0x02, 0x00, 0x00,
		// fe80:f:2::/128
		0x18, 0x03, 0x80, 0x00, 0x00, 0x00, 0x0b, 0xb8, 0xfe, 0x80, 0x00, 0x0f, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif
	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_rdnss)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_rdnss(sbl, iface, iface->AdvRDNSSList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
	    0x19, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
	    0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // address 1
	    0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // address 2
	    0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, // address 3
	    0x19, 0x07, 0x00, 0x00, 0x00, 0x00, 0x11, 0xd7, // header
	    0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, // address 1
	    0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, // address 2
	    0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, // address 2
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_rdnss2)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_rdnss.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_rdnss(sbl, iface, iface->AdvRDNSSList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		0x19, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d,
		// address 1
		0x12, 0x34, 0x04, 0x23, 0xfe, 0xfe, 0x04, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_rdnss3)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_rdnss_long.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_rdnss(sbl, iface, iface->AdvRDNSSList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		//0x19, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, // header
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // address 1
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // address 2
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, // address 3
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, // address 4
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, // address 5
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, // address 6
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, // address 7
		//0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, // address 8
		0x19, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, // header
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // address 1
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // address 2
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, // address 3
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		0x1f, 0x09, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, // header
			0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, // "office"
			0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, // "branch"
			0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, // "example"
			0x03, 0x63, 0x6f, 0x6d, // "com"
			0x00, // len=0, terminates entry

			0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, // "branch"
			0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, // "example"
			0x03, 0x63, 0x6f, 0x6d, // "com"
			0x00, // len=0, terminates entry

			0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, // "example"
			0x03, 0x63, 0x6f, 0x6d, // "com"
			0x00, // len=0, terminates entry

		0x00, 0x00, 0x00, 0x00, // padding

		0x1f, 0x09, 0x00, 0x00, 0x00, 0x00, 0x04, 0x4b, // header
			0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, // "office"
			0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, // "branch"
			0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, // "example"
			0x03, 0x6e, 0x65, 0x74, //" net"
			0x00, // len=0, terminates entry

			0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, // "branch"
			0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, // "example"
			0x03, 0x6e, 0x65, 0x74, // "net"
			0x00, // len=0, terminates entry

			0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, // "example"
			0x03, 0x6e, 0x65, 0x74, // "net"
			0x00, // len=0, terminates entry

		0x00, 0x00, 0x00, 0x00, // padding

		0x1f, 0x07, 0x00, 0x00, 0x00, 0x00, 0x04, 0x4c, // header
		// "office.branch.example."
		0x06, 0x6f, 0x66, 0x66, 0x69, 0x63, 0x65, 0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00,
		// "branch.example."
		0x06, 0x62, 0x72, 0x61, 0x6e, 0x63, 0x68, 0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00,
		// "example."
		0x07, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x00,
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl1)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_dnssl1.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		0x1f, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
		0x3f, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, // len=63, "x" x 63
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x03, 0x63, 0x6f, 0x6d, // len=3, "com"
		0x00, // len=0, terminates entry
		0x00, 0x00, 0x00, // padding
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl2)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_dnssl2.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		0x1f, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
		0x3f, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, // len=63, "x" x 63
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x03, 0x63, 0x6f, 0x6d, // len=3, "com"
		0x00, // len=0, terminates entry
		0x00, 0x00, 0x00, // padding
		0x1f, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
		0x3f, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, // len=63, "x" x 63
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x03, 0x6e, 0x65, 0x74, 0x00, 0x00, 0x00, 0x00, // len=3, "com"
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl3)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_dnssl3.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else

	unsigned char expected[] = {
		0x1f, 0x13, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
		0x3f, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, // len=63, "x" x 63
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
		0x03, 0x63, 0x6f, 0x6d, // len=3, "com"
		0x00, // len=0
		0x3f, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, // len=63 "a" x 63
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
		0x03, 0x6e, 0x65, 0x74, // len=3, "net"
		0x00, // len=0
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // padding
	};


	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl4)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_dnssl4.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	ck_assert_ptr_ne(0, iface);
	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

	//print_safe_buffer(&sb);
#ifdef PRINT_SAFE_BUFFER
	char buf[65536];
	snprint_safe_buffer(buf, 65536, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else

	unsigned char expected[] = {
		0x1f, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
		RFC1035_DNS_HACK_64BYTE('a'),
		RFC1035_DNS_HACK_64BYTE('b'),
		RFC1035_DNS_HACK_64BYTE('c'),
		RFC1035_DNS_HACK_64BYTE('d'),
		RFC1035_DNS_HACK_64BYTE('e'),
		RFC1035_DNS_HACK_64BYTE('f'),
		RFC1035_DNS_HACK_64BYTE('g'),
		RFC1035_DNS_HACK_64BYTE('h'),
		RFC1035_DNS_HACK_64BYTE('i'),
		RFC1035_DNS_HACK_64BYTE('j'),
		RFC1035_DNS_HACK_64BYTE('k'),
		RFC1035_DNS_HACK_64BYTE('l'),
		RFC1035_DNS_HACK_64BYTE('m'),
		RFC1035_DNS_HACK_64BYTE('n'),
		RFC1035_DNS_HACK_64BYTE('o'),
		RFC1035_DNS_HACK_64BYTE('p'),
		RFC1035_DNS_HACK_64BYTE('q'),
		RFC1035_DNS_HACK_64BYTE('r'),
		RFC1035_DNS_HACK_64BYTE('s'),
		RFC1035_DNS_HACK_64BYTE('t'),
		RFC1035_DNS_HACK_64BYTE('u'),
		RFC1035_DNS_HACK_64BYTE('v'),
		RFC1035_DNS_HACK_64BYTE('w'),
		RFC1035_DNS_HACK_64BYTE('x'),
		RFC1035_DNS_HACK_64BYTE('y'),
		RFC1035_DNS_HACK_64BYTE('z'),
		RFC1035_DNS_HACK_64BYTE('0'),
		RFC1035_DNS_HACK_64BYTE('1'),
		RFC1035_DNS_HACK_64BYTE('2'),
		RFC1035_DNS_HACK_64BYTE('3'),
		RFC1035_DNS_HACK_64BYTE('4'), // 1992 bytes at this point
		//RFC1035_DNS_HACK_64BYTE('5'), // 2056 bytes at this point - would be too large
	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl5)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	// contains a too-large DNSSL config for a single DNSSL option packet.
	iface = readin_config("test/test_dnssl5.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	ck_assert_ptr_ne(0, iface);
	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

	// TODO: how to verify the "DNSSL too long" message?

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else

	unsigned char expected[] = {}; // intentional, the DNSSL option in this config should be skipped.
	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_options_dnssl6)
{
	struct Interface *iface = 0;
	ck_assert_ptr_eq(0, iface);
	iface = readin_config("test/test_dnssl6.conf");
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer_list *sbl = new_safe_buffer_list();
	struct safe_buffer sb = SAFE_BUFFER_INIT;

	ck_assert_ptr_ne(0, iface);
	add_ra_options_dnssl(sbl, iface, iface->AdvDNSSLList, iface->state_info.cease_adv, NULL);

	safe_buffer_list_to_safe_buffer(sbl, &sb);
	safe_buffer_list_free(sbl);
	free_ifaces(iface);

	//print_safe_buffer(&sb);
#ifdef PRINT_SAFE_BUFFER
	char buf[65536];
	snprint_safe_buffer(buf, 65536, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else

	unsigned char expected[] = {
		0x1f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2, // header
		RFC1035_DNS_HACK_64BYTE('a'),
		RFC1035_DNS_HACK_64BYTE('b'),
		RFC1035_DNS_HACK_64BYTE('c'),
		RFC1035_DNS_HACK_64BYTE('d'),
		RFC1035_DNS_HACK_64BYTE('e'),
		RFC1035_DNS_HACK_64BYTE('f'),
		RFC1035_DNS_HACK_64BYTE('g'),
		RFC1035_DNS_HACK_64BYTE('h'),
		RFC1035_DNS_HACK_64BYTE('i'),
		RFC1035_DNS_HACK_64BYTE('j'),
		RFC1035_DNS_HACK_64BYTE('k'),
		RFC1035_DNS_HACK_64BYTE('l'),
		RFC1035_DNS_HACK_64BYTE('m'),
		RFC1035_DNS_HACK_64BYTE('n'),
		RFC1035_DNS_HACK_64BYTE('o'),
		RFC1035_DNS_HACK_64BYTE('p'),
		RFC1035_DNS_HACK_64BYTE('q'),
		RFC1035_DNS_HACK_64BYTE('r'),
		RFC1035_DNS_HACK_64BYTE('s'),
		RFC1035_DNS_HACK_64BYTE('t'),
		RFC1035_DNS_HACK_64BYTE('u'),
		RFC1035_DNS_HACK_64BYTE('v'),
		RFC1035_DNS_HACK_64BYTE('w'),
		RFC1035_DNS_HACK_64BYTE('x'),
		RFC1035_DNS_HACK_64BYTE('y'),
		RFC1035_DNS_HACK_64BYTE('z'),
		RFC1035_DNS_HACK_64BYTE('0'),
		RFC1035_DNS_HACK_64BYTE('1'),
		RFC1035_DNS_HACK_64BYTE('2'),
		RFC1035_DNS_HACK_64BYTE('3'),
		RFC1035_DNS_HACK_64BYTE('4'), // 1992 bytes at this point
		//RFC1035_DNS_HACK_64BYTE('5'), // 2056 bytes at this point - too large!
		0x2e, REP16('A'), REP16('A'), REP8('A'), REP4('A'), 'A', 'A', 0x00,

	};

	ck_assert_int_eq(sizeof(expected), sb.used);
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sb.used)); // Newer libcheck supports: ck_assert_mem_eq(expected, sb.buffer, sb.used);
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_mtu)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_mtu(&sb, iface->AdvLinkMTU);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	// https://www.rfc-editor.org/rfc/rfc4861.html#section-4.6.4
	unsigned char expected[] = {
		0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xd2,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_sllao48)
{
	struct sllao sllao48 = {
		{1, 2, 3, 4, 5, 6, 7, 8}, 48, 64, 1500,
	};

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_sllao(&sb, &sllao48);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected48[] = {
		0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	};

	ck_assert_int_eq(sizeof(expected48), sb.used);
	ck_assert_int_eq(0, memcmp(sb.buffer, expected48, sizeof(expected48)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_sllao64)
{

	struct sllao sllao64 = {
		{1, 2, 3, 4, 5, 6, 7, 8}, 64, 64, 1500,
	};

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_sllao(&sb, &sllao64);

#ifdef PRINT_SAFE_BUFFER
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected64[] = {
		0x01, 0x02, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	ck_assert_int_eq(sizeof(expected64), sb.used);
	ck_assert_int_eq(0, memcmp(sb.buffer, expected64, sizeof(expected64)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_lowpanco)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_lowpanco(&sb, iface->AdvLowpanCoList);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		0x22, 0x03, 0x32, 0x14, 0x00, 0x00, 0x03, 0xe8, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

START_TEST(test_add_ra_option_abro)
{
	ck_assert_ptr_ne(0, iface);

	struct safe_buffer sb = SAFE_BUFFER_INIT;
	add_ra_option_abro(&sb, iface->AdvAbroList);

#ifdef PRINT_SAFE_BUFFER
	char buf[4096];
	snprint_safe_buffer(buf, 4096, &sb);
	ck_assert_msg(0, "\n%s", (char*)&buf);
#else
	unsigned char expected[] = {
		0x23, 0x03, 0x00, 0x0a, 0x00, 0x02, 0x00, 0x02, 0xfe, 0x80, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xa2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	};

	ck_assert_int_eq(sb.used, sizeof(expected));
	ck_assert_int_eq(0, memcmp(expected, sb.buffer, sizeof(expected)));
#endif

	safe_buffer_free(&sb);
}
END_TEST

Suite *send_suite(void)
{
	TCase *tc_update = tcase_create("update");
	tcase_add_test(tc_update, test_decrement_lifetime);

	TCase *tc_build = tcase_create("build");
	tcase_add_unchecked_fixture(tc_build, iface_setup, iface_teardown);
	tcase_add_test(tc_build, test_add_ra_header_cease_adv0);
	tcase_add_test(tc_build, test_add_ra_header_cease_adv1);
	tcase_add_test(tc_build, test_add_ra_options_prefix);
	tcase_add_test(tc_build, test_add_ra_options_route);
	tcase_add_test(tc_build, test_add_ra_options_rdnss);
	tcase_add_test(tc_build, test_add_ra_options_rdnss2);
	tcase_add_test(tc_build, test_add_ra_options_rdnss3);
	tcase_add_test(tc_build, test_add_ra_options_dnssl);
	tcase_add_test(tc_build, test_add_ra_options_dnssl1);
	tcase_add_test(tc_build, test_add_ra_options_dnssl2);
	tcase_add_test(tc_build, test_add_ra_options_dnssl3);
	tcase_add_test(tc_build, test_add_ra_options_dnssl4);
	tcase_add_test(tc_build, test_add_ra_options_dnssl5);
	tcase_add_test(tc_build, test_add_ra_options_dnssl6);
	tcase_add_test(tc_build, test_add_ra_option_mtu);
	tcase_add_test(tc_build, test_add_ra_option_sllao48);
	tcase_add_test(tc_build, test_add_ra_option_sllao64);
	tcase_add_test(tc_build, test_add_ra_option_lowpanco);
	tcase_add_test(tc_build, test_add_ra_option_abro);

	Suite *s = suite_create("send");
	suite_add_tcase(s, tc_update);
	suite_add_tcase(s, tc_build);

	return s;
}
