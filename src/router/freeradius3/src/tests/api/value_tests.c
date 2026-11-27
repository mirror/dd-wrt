/*
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/** Tests for the value_data_t API
 *
 * @file src/tests/api/value_tests.c
 *
 * Covers the public functions of src/lib/value.c:
 *
 * - value_data_cmp()
 * - value_data_cmp_op()
 * - value_data_from_str()
 * - value_data_cast()
 * - value_data_copy()
 * - value_data_prints()
 * - value_data_aprints()
 *
 * Note that v3 keeps the type and the length outside the value, so every
 * call passes a PW_TYPE and a length alongside the value_data_t.  This is
 * unlike the self-describing fr_value_box_t of later versions.
 *
 * @copyright 2026 The FreeRADIUS server project
 */
#include <freeradius-devel/libradius.h>
#include <freeradius-devel/conf.h>	/* RADIUS_DICTIONARY */

#include "acutest_common_init.h"
#include "acutest_helpers.h"

static TALLOC_CTX *autofree;

/*
 *	Most of the API takes a DICT_ATTR only to look up the names of
 *	enumerated values, so a dictionary is needed to reach those paths.
 *	Service-Type is a convenient integer attribute with VALUEs.
 */
static DICT_ATTR const *da_enum;

static void test_init(void) __attribute__((constructor));
static void test_init(void)
{
	char const *dict_dir = getenv("DICT_DIR");

DIAG_OFF(deprecated-declarations)
	autofree = talloc_autofree_context();
DIAG_ON(deprecated-declarations)
	if (!autofree) {
		fr_perror("value_tests");
		exit(EXIT_FAILURE);
	}

	/*
	 *	Mismatch between the binary and the libraries it depends on.
	 */
	if (fr_check_lib_magic(RADIUSD_MAGIC_NUMBER) < 0) {
		fr_perror("value_tests");
		exit(EXIT_FAILURE);
	}

	if (!dict_dir) dict_dir = "share";

	if (dict_init(dict_dir, RADIUS_DICTIONARY) < 0) {
		fr_perror("value_tests: failed loading dictionaries from \"%s\"", dict_dir);
		fprintf(stderr, "Set DICT_DIR to the directory holding \"dictionary\"\n");
		exit(EXIT_FAILURE);
	}

	da_enum = dict_attrbyname("Service-Type");
	if (!da_enum) {
		fprintf(stderr, "value_tests: dictionary is missing Service-Type\n");
		exit(EXIT_FAILURE);
	}
}

/** Build an IPv4 prefix, so the tests read as addresses rather than bytes
 */
static void mk_ipv4_prefix(value_data_t *out, char const *str)
{
	PW_TYPE type = PW_TYPE_IPV4_PREFIX;

	if (value_data_from_str(autofree, out, &type, NULL, str, -1, '\0') < 0) {
		TEST_MSG("Failed parsing prefix \"%s\": %s", str, fr_strerror());
	}
}

static void mk_ipv4_addr(value_data_t *out, char const *str)
{
	PW_TYPE type = PW_TYPE_IPV4_ADDR;

	if (value_data_from_str(autofree, out, &type, NULL, str, -1, '\0') < 0) {
		TEST_MSG("Failed parsing address \"%s\": %s", str, fr_strerror());
	}
}

/*
 *	value_data_cmp()
 *
 *	Returns -1, 0 or 1, and -2 when the values cannot be compared.
 */
static void test_cmp_integer(void)
{
	value_data_t a, b;

	a.integer = 42;
	b.integer = 42;
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 0);

	b.integer = 43;
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER, &b, 4, PW_TYPE_INTEGER, &a, 4), 1);
}

static void test_cmp_byte(void)
{
	value_data_t a, b;

	a.byte = 0;
	b.byte = 255;

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BYTE, &a, 1, PW_TYPE_BYTE, &b, 1), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BYTE, &b, 1, PW_TYPE_BYTE, &a, 1), 1);

	b.byte = 0;
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BYTE, &a, 1, PW_TYPE_BYTE, &b, 1), 0);
}

static void test_cmp_short(void)
{
	value_data_t a, b;

	a.ushort = 1;
	b.ushort = 65535;

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_SHORT, &a, 2, PW_TYPE_SHORT, &b, 2), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_SHORT, &b, 2, PW_TYPE_SHORT, &a, 2), 1);
}

static void test_cmp_signed(void)
{
	value_data_t a, b;

	/*
	 *	A signed comparison, so the negative value must sort first.
	 *	Comparing these as unsigned would give the opposite answer.
	 */
	a.sinteger = -1;
	b.sinteger = 1;

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_SIGNED, &a, 4, PW_TYPE_SIGNED, &b, 4), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_SIGNED, &b, 4, PW_TYPE_SIGNED, &a, 4), 1);
}

static void test_cmp_integer64(void)
{
	value_data_t a, b;

	a.integer64 = 0;
	b.integer64 = UINT64_C(0xffffffffffffffff);

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER64, &a, 8, PW_TYPE_INTEGER64, &b, 8), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER64, &b, 8, PW_TYPE_INTEGER64, &a, 8), 1);
}

static void test_cmp_date(void)
{
	value_data_t a, b;

	a.date = 1000;
	b.date = 2000;

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_DATE, &a, 4, PW_TYPE_DATE, &b, 4), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_DATE, &b, 4, PW_TYPE_DATE, &a, 4), 1);
}

static void test_cmp_string(void)
{
	value_data_t a, b;

	a.strvalue = "apple";
	b.strvalue = "banana";

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_STRING, &a, 5, PW_TYPE_STRING, &b, 6), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_STRING, &b, 6, PW_TYPE_STRING, &a, 5), 1);

	b.strvalue = "apple";
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_STRING, &a, 5, PW_TYPE_STRING, &b, 5), 0);
}

static void test_cmp_string_embedded_nul(void)
{
	value_data_t a, b;

	/*
	 *	Comparison is done with memcmp() rather than strcmp(), so
	 *	the bytes after an embedded NUL still count.
	 */
	a.strvalue = "a\0b";
	b.strvalue = "a\0c";

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_STRING, &a, 3, PW_TYPE_STRING, &b, 3), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_STRING, &b, 3, PW_TYPE_STRING, &a, 3), 1);
}

static void test_cmp_octets(void)
{
	value_data_t a, b;

	a.octets = (uint8_t const *)"\x01\x02";
	b.octets = (uint8_t const *)"\x01\x03";

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_OCTETS, &a, 2, PW_TYPE_OCTETS, &b, 2), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_OCTETS, &b, 2, PW_TYPE_OCTETS, &a, 2), 1);
}

static void test_cmp_octets_length(void)
{
	value_data_t a, b;

	/*
	 *	Same leading bytes, different lengths.  The shorter value
	 *	sorts first, i.e. "0x00" is smaller than "0x0000".
	 */
	a.octets = (uint8_t const *)"\x01";
	b.octets = (uint8_t const *)"\x01\x02";

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_OCTETS, &a, 1, PW_TYPE_OCTETS, &b, 2), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_OCTETS, &b, 2, PW_TYPE_OCTETS, &a, 1), 1);
}

static void test_cmp_octets_zero_length(void)
{
	value_data_t a, b;

	a.octets = (uint8_t const *)"";
	b.octets = (uint8_t const *)"";

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_OCTETS, &a, 0, PW_TYPE_OCTETS, &b, 0), 0);
}

static void test_cmp_ipaddr(void)
{
	value_data_t a, b;

	a.ipaddr.s_addr = htonl(0x7f000001);	/* 127.0.0.1 */
	b.ipaddr.s_addr = htonl(0x7f000002);	/* 127.0.0.2 */

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV4_ADDR, &a, 4, PW_TYPE_IPV4_ADDR, &b, 4), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV4_ADDR, &b, 4, PW_TYPE_IPV4_ADDR, &a, 4), 1);

	b.ipaddr.s_addr = htonl(0x7f000001);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV4_ADDR, &a, 4, PW_TYPE_IPV4_ADDR, &b, 4), 0);
}

static void test_cmp_ether(void)
{
	value_data_t a, b;

	memcpy(a.ether, "\x00\x01\x02\x03\x04\x05", 6);
	memcpy(b.ether, "\x00\x01\x02\x03\x04\x06", 6);

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_ETHERNET, &a, 6, PW_TYPE_ETHERNET, &b, 6), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_ETHERNET, &b, 6, PW_TYPE_ETHERNET, &a, 6), 1);

	memcpy(b.ether, a.ether, 6);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_ETHERNET, &a, 6, PW_TYPE_ETHERNET, &b, 6), 0);
}

static void test_cmp_different_types(void)
{
	value_data_t a, b;

	a.integer = 42;
	b.integer64 = 42;

	/*
	 *	Values of differing types cannot be compared, in either
	 *	direction, even when they hold the same number.
	 */
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER64, &b, 8), -2);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_INTEGER64, &b, 8, PW_TYPE_INTEGER, &a, 4), -2);
}

/*
 *	value_data_cmp_op()
 *
 *	Returns 1 for true, 0 for false and -1 on error.
 */
static void test_cmp_op_eq(void)
{
	value_data_t a, b;

	a.integer = 42;
	b.integer = 42;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 1);

	b.integer = 43;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 0);
}

static void test_cmp_op_ne(void)
{
	value_data_t a, b;

	a.integer = 42;
	b.integer = 43;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_NE, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 1);

	b.integer = 42;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_NE, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 0);
}

static void test_cmp_op_lt(void)
{
	value_data_t a, b;

	a.integer = 1;
	b.integer = 2;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_INTEGER, &b, 4, PW_TYPE_INTEGER, &a, 4), 0);

	/* Not less than itself */
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &a, 4), 0);
}

static void test_cmp_op_gt(void)
{
	value_data_t a, b;

	a.integer = 2;
	b.integer = 1;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_INTEGER, &b, 4, PW_TYPE_INTEGER, &a, 4), 0);

	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &a, 4), 0);
}

static void test_cmp_op_le(void)
{
	value_data_t a, b;

	a.integer = 1;
	b.integer = 2;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &a, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_INTEGER, &b, 4, PW_TYPE_INTEGER, &a, 4), 0);
}

static void test_cmp_op_ge(void)
{
	value_data_t a, b;

	a.integer = 2;
	b.integer = 1;
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GE, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &b, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GE, PW_TYPE_INTEGER, &a, 4, PW_TYPE_INTEGER, &a, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GE, PW_TYPE_INTEGER, &b, 4, PW_TYPE_INTEGER, &a, 4), 0);
}

static void test_cmp_op_string(void)
{
	value_data_t a, b;

	a.strvalue = "apple";
	b.strvalue = "banana";

	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_STRING, &a, 5, PW_TYPE_STRING, &b, 6), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_STRING, &a, 5, PW_TYPE_STRING, &b, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_NE, PW_TYPE_STRING, &a, 5, PW_TYPE_STRING, &b, 6), 1);
}

/*
 *	value_data_from_str()
 *
 *	Returns the length of the parsed data, or -1 on error.  Note that
 *	src_type is in/out; a COMBO_IP may be rewritten to a concrete type.
 */
static void test_from_str_integer(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_INTEGER;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "12345", -1, '\0'), 4);
	TEST_CHECK(dst.integer == 12345);
	TEST_MSG("Expected 12345, got %u", dst.integer);
}

/** Non-numeric input is rejected for an integer
 *
 * The trailing-garbage check used to be gated on src_enumv being non-NULL,
 * so with no enumeration to consult the parser kept whatever fr_strtoul()
 * had produced and reported success.  "not-a-number" became 0, and "42abc"
 * became 42.  Both are now errors.
 */
static void test_from_str_integer_bad(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_INTEGER;

	TEST_CASE("Wholly non-numeric");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "not-a-number", -1, '\0'), -1);

	TEST_CASE("Trailing garbage after a number");
	type = PW_TYPE_INTEGER;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "42abc", -1, '\0'), -1);

	TEST_CASE("integer64 agrees");
	type = PW_TYPE_INTEGER64;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "not-a-number", -1, '\0'), -1);

	TEST_CASE("byte and short agree");
	type = PW_TYPE_BYTE;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "banana", -1, '\0'), -1);
	type = PW_TYPE_SHORT;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "banana", -1, '\0'), -1);
}

/** Forms which are still valid after tightening the parser
 *
 * Rejecting trailing garbage must not reject trailing whitespace, nor the
 * "0x" hex form which fr_strtoul() accepts.
 */
static void test_from_str_integer_accepted_forms(void)
{
	value_data_t	dst;
	PW_TYPE		type;

	TEST_CASE("Trailing whitespace is allowed");
	type = PW_TYPE_INTEGER;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "42 ", -1, '\0'), 4);
	TEST_CHECK(dst.integer == 42);

	TEST_CASE("A hex prefix is allowed");
	type = PW_TYPE_INTEGER;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "0x2a", -1, '\0'), 4);
	TEST_CHECK(dst.integer == 42);
}

static void test_from_str_byte(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_BYTE;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "255", -1, '\0'), 1);
	TEST_CHECK(dst.byte == 255);

	/* Out of range for a byte */
	type = PW_TYPE_BYTE;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "256", -1, '\0'), -1);
}

static void test_from_str_short(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_SHORT;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "65535", -1, '\0'), 2);
	TEST_CHECK(dst.ushort == 65535);

	type = PW_TYPE_SHORT;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "65536", -1, '\0'), -1);
}

static void test_from_str_signed(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_SIGNED;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "-12345", -1, '\0'), 4);
	TEST_CHECK(dst.sinteger == -12345);
	TEST_MSG("Expected -12345, got %d", dst.sinteger);
}

static void test_from_str_integer64(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_INTEGER64;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "18446744073709551615", -1, '\0'), 8);
	TEST_CHECK(dst.integer64 == UINT64_C(0xffffffffffffffff));
}

static void test_from_str_string(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_STRING;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "hello", -1, '\0'), 5);
	TEST_CHECK_STRCMP(dst.strvalue, "hello");
}

static void test_from_str_string_length(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_STRING;

	/*
	 *	An explicit length stops the copy short, rather than
	 *	running to the NUL.
	 */
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "hello world", 5, '\0'), 5);
	TEST_CHECK_STRCMP(dst.strvalue, "hello");
}

static void test_from_str_octets_hex(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_OCTETS;

	/* A 0x prefix means the rest is hex, so four characters give two bytes */
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "0x0102", -1, '\0'), 2);
	if (dst.octets) {
		TEST_CHECK(dst.octets[0] == 0x01);
		TEST_CHECK(dst.octets[1] == 0x02);
	}
}

static void test_from_str_octets_verbatim(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_OCTETS;

	/* Without the 0x prefix the string is copied as-is */
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "abc", -1, '\0'), 3);
	if (dst.octets) TEST_CHECK(memcmp(dst.octets, "abc", 3) == 0);
}

static void test_from_str_octets_odd_hex(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_OCTETS;

	/* An odd number of hex digits cannot be turned into whole bytes */
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "0x010", -1, '\0'), -1);
}

static void test_from_str_ipaddr(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IPV4_ADDR;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "192.0.2.1", -1, '\0'), 4);
	TEST_CHECK(dst.ipaddr.s_addr == htonl(0xc0000201));
}

static void test_from_str_ipaddr_bad(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IPV4_ADDR;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "999.0.2.1", -1, '\0'), -1);
}

static void test_from_str_ipv6addr(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IPV6_ADDR;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "2001:db8::1", -1, '\0'), 16);
}

static void test_from_str_ether(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_ETHERNET;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "00:01:02:03:04:05", -1, '\0'), 6);
	TEST_CHECK(dst.ether[0] == 0x00);
	TEST_CHECK(dst.ether[5] == 0x05);
}

/** A short Ethernet address is rejected
 *
 * The parse loop consumed however many colon-separated octets it was given
 * and never checked that it had filled all six.  The return length for a
 * fixed-size type comes from the type size table and is always 6, so the
 * caller was told six bytes were valid while the tail of dst->ether was
 * left untouched.  The poison byte below proves the parser writes nothing
 * on the error path.
 */
static void test_from_str_ether_short(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_ETHERNET;

	memset(&dst, 0xAA, sizeof(dst));

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "00:01:02:03:04", -1, '\0'), -1);
	TEST_CHECK(dst.ether[5] == 0xAA);
	TEST_MSG("Nothing should be relied on after an error, got 0x%02x", dst.ether[5]);
}

/** A non-hex Ethernet address is rejected
 */
static void test_from_str_ether_bad(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_ETHERNET;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "zz:01:02:03:04:05", -1, '\0'), -1);
}

/** An integer is converted to an Ethernet address
 *
 * is_integer() short-circuits the colon parsing.  The value is byte-swapped
 * into a 64 bit integer, of which the low-order 48 bits are the address.  In
 * network byte order those are the last six bytes, so the copy has to start
 * two bytes in; it used to start at the front and take the high-order bytes,
 * which are zero for any address that fits in 48 bits.
 */
static void test_from_str_ether_from_integer(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_ETHERNET;

	TEST_CASE("12345 is 0x3039, so it lands in the last two octets");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "12345", -1, '\0'), 6);
	TEST_CHECK(memcmp(dst.ether, "\x00\x00\x00\x00\x30\x39", 6) == 0);

	TEST_CASE("The largest value which fits in 48 bits");
	type = PW_TYPE_ETHERNET;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "281474976710655", -1, '\0'), 6);
	TEST_CHECK(memcmp(dst.ether, "\xff\xff\xff\xff\xff\xff", 6) == 0);

	TEST_CASE("Anything larger cannot be an Ethernet address");
	type = PW_TYPE_ETHERNET;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "281474976710656", -1, '\0'), -1);
}

static void test_from_str_null(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_STRING;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, NULL, -1, '\0'), -1);
}

/*
 *	value_data_cast()
 *
 *	Note that casting to the same type trips an assertion, so there is
 *	deliberately no same-type test here.
 */
static void test_cast_integer_to_integer64(void)
{
	value_data_t	src, dst;

	src.integer = 12345;

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER64, NULL,
					PW_TYPE_INTEGER, NULL, &src, 4), 8);
	TEST_CHECK(dst.integer64 == 12345);
}

static void test_cast_byte_to_integer(void)
{
	value_data_t	src, dst;

	src.byte = 200;

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER, NULL,
					PW_TYPE_BYTE, NULL, &src, 1), 4);
	TEST_CHECK(dst.integer == 200);
}

static void test_cast_short_to_integer(void)
{
	value_data_t	src, dst;

	src.ushort = 4096;

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER, NULL,
					PW_TYPE_SHORT, NULL, &src, 2), 4);
	TEST_CHECK(dst.integer == 4096);
}

static void test_cast_integer_to_string(void)
{
	value_data_t	src, dst;

	src.integer = 12345;

	/* Serialising to a string returns the string length */
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_STRING, NULL,
					PW_TYPE_INTEGER, NULL, &src, 4), 5);
	TEST_CHECK_STRCMP(dst.strvalue, "12345");
}

static void test_cast_string_to_integer(void)
{
	value_data_t	src, dst;

	src.strvalue = "12345";

	/* Casting from a string is a parse, so it goes via from_str */
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER, NULL,
					PW_TYPE_STRING, NULL, &src, 5), 4);
	TEST_CHECK(dst.integer == 12345);
}

/** Casting a non-numeric string to an integer fails
 *
 * Casting from a string is implemented by calling value_data_from_str(), so
 * this follows whatever that function does with malformed input.  It used to
 * succeed and yield 0.
 */
static void test_cast_string_to_integer_bad(void)
{
	value_data_t	src, dst;

	src.strvalue = "banana";

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER, NULL,
					PW_TYPE_STRING, NULL, &src, 6), -1);

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER64, NULL,
					PW_TYPE_STRING, NULL, &src, 6), -1);
}

static void test_cast_integer_to_octets(void)
{
	value_data_t	src, dst;

	src.integer = 0x01020304;

	/* Casting to octets is a straight network-order copy */
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_OCTETS, NULL,
					PW_TYPE_INTEGER, NULL, &src, 4), 4);
	if (dst.octets) {
		TEST_CHECK(dst.octets[0] == 0x01);
		TEST_CHECK(dst.octets[3] == 0x04);
	}
}

static void test_cast_ipaddr_to_string(void)
{
	value_data_t	src, dst;

	src.ipaddr.s_addr = htonl(0xc0000201);

	TEST_CHECK(value_data_cast(autofree, &dst, PW_TYPE_STRING, NULL,
				   PW_TYPE_IPV4_ADDR, NULL, &src, 4) > 0);
	TEST_CHECK_STRCMP(dst.strvalue, "192.0.2.1");
}

static void test_cast_string_to_ipaddr(void)
{
	value_data_t	src, dst;

	src.strvalue = "192.0.2.1";

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_IPV4_ADDR, NULL,
					PW_TYPE_STRING, NULL, &src, 9), 4);
	TEST_CHECK(dst.ipaddr.s_addr == htonl(0xc0000201));
}

/*
 *	value_data_copy()
 */
static void test_copy_integer(void)
{
	value_data_t	src, dst;

	src.integer = 12345;

	TEST_CHECK_SLEN(value_data_copy(autofree, &dst, PW_TYPE_INTEGER, &src, 4), 4);
	TEST_CHECK(dst.integer == 12345);
}

static void test_copy_string(void)
{
	value_data_t	src, dst;

	src.strvalue = "hello";

	TEST_CHECK_SLEN(value_data_copy(autofree, &dst, PW_TYPE_STRING, &src, 5), 5);
	TEST_CHECK_STRCMP(dst.strvalue, "hello");

	/* A copy, not an alias: the buffers must differ */
	TEST_CHECK(dst.strvalue != src.strvalue);
}

static void test_copy_octets(void)
{
	value_data_t	src, dst;

	src.octets = (uint8_t const *)"\x01\x02\x03";

	TEST_CHECK_SLEN(value_data_copy(autofree, &dst, PW_TYPE_OCTETS, &src, 3), 3);
	TEST_CHECK(dst.octets != src.octets);
	if (dst.octets) TEST_CHECK(memcmp(dst.octets, "\x01\x02\x03", 3) == 0);
}

static void test_copy_string_embedded_nul(void)
{
	value_data_t	src, dst;

	src.strvalue = "a\0b";

	/* The length is what matters, so the trailing byte survives */
	TEST_CHECK_SLEN(value_data_copy(autofree, &dst, PW_TYPE_STRING, &src, 3), 3);
	if (dst.strvalue) TEST_CHECK(memcmp(dst.strvalue, "a\0b", 3) == 0);
}

/*
 *	value_data_prints()
 *
 *	Returns the number of bytes it would have written, so that
 *	truncation can be detected.
 */
static void test_prints_integer(void)
{
	value_data_t	data;
	char		buff[64];

	data.integer = 12345;

	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, NULL, &data, 4, '\0'), 5);
	TEST_CHECK_STRCMP(buff, "12345");
}

static void test_prints_signed_negative(void)
{
	value_data_t	data;
	char		buff[64];

	data.sinteger = -12345;

	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_SIGNED, NULL, &data, 4, '\0'), 6);
	TEST_CHECK_STRCMP(buff, "-12345");
}

static void test_prints_string(void)
{
	value_data_t	data;
	char		buff[64];

	data.strvalue = "hello";

	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_STRING, NULL, &data, 5, '\0'), 5);
	TEST_CHECK_STRCMP(buff, "hello");
}

static void test_prints_string_quoted(void)
{
	value_data_t	data;
	char		buff[64];

	data.strvalue = "hello";

	/* A quote character means the printer adds the quotes itself */
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_STRING, NULL, &data, 5, '"') > 0);
	TEST_CHECK_STRCMP(buff, "\"hello\"");
}

static void test_prints_octets(void)
{
	value_data_t	data;
	char		buff[64];

	data.octets = (uint8_t const *)"\x01\x02";

	/* Octets print as hex with a 0x prefix */
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_OCTETS, NULL, &data, 2, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "0x0102");
}

static void test_prints_ipaddr(void)
{
	value_data_t	data;
	char		buff[64];

	data.ipaddr.s_addr = htonl(0xc0000201);

	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_IPV4_ADDR, NULL, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "192.0.2.1");
}

static void test_prints_ether(void)
{
	value_data_t	data;
	char		buff[64];

	memcpy(data.ether, "\x00\x01\x02\x03\x04\x05", 6);

	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_ETHERNET, NULL, &data, 6, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "00:01:02:03:04:05");
}

static void test_prints_truncation(void)
{
	value_data_t	data;
	char		buff[4];

	data.integer = 12345;

	/*
	 *	The return value reports what was needed, not what fit, so a
	 *	caller can tell that the output was truncated.
	 */
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, NULL, &data, 4, '\0'), 5);
}

/*
 *	value_data_aprints()
 */
static void test_aprints_integer(void)
{
	value_data_t	data;
	char		*out;

	data.integer = 12345;

	out = value_data_aprints(autofree, PW_TYPE_INTEGER, NULL, &data, 4, '\0');
	TEST_ASSERT(out != NULL);
	if (!out) return;

	TEST_CHECK_STRCMP(out, "12345");
	talloc_free(out);
}

static void test_aprints_string(void)
{
	value_data_t	data;
	char		*out;

	data.strvalue = "hello";

	out = value_data_aprints(autofree, PW_TYPE_STRING, NULL, &data, 5, '\0');
	TEST_ASSERT(out != NULL);

	TEST_CHECK_STRCMP(out, "hello");
	talloc_free(out);
}

static void test_aprints_octets(void)
{
	value_data_t	data;
	char		*out;

	data.octets = (uint8_t const *)"\xde\xad\xbe\xef";

	out = value_data_aprints(autofree, PW_TYPE_OCTETS, NULL, &data, 4, '\0');
	TEST_ASSERT(out != NULL);
	if (!out) return;

	TEST_CHECK_STRCMP(out, "0xdeadbeef");
	talloc_free(out);
}

/*
 *	Round trips.  Parsing and printing should be inverses of each other.
 */
static void test_round_trip_integer(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_INTEGER;
	char		buff[64];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "4294967295", -1, '\0'), 4);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, NULL, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "4294967295");
}

static void test_round_trip_signed(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_SIGNED;
	char		buff[64];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "-2147483648", -1, '\0'), 4);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_SIGNED, NULL, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "-2147483648");
}

static void test_round_trip_ipaddr(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_IPV4_ADDR;
	char		buff[64];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "10.11.12.13", -1, '\0'), 4);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_IPV4_ADDR, NULL, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "10.11.12.13");
}

static void test_round_trip_ether(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_ETHERNET;
	char		buff[64];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "aa:bb:cc:dd:ee:ff", -1, '\0'), 6);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_ETHERNET, NULL, &data, 6, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "aa:bb:cc:dd:ee:ff");
}

static void test_round_trip_octets(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_OCTETS;
	char		buff[64];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "0xdeadbeef", -1, '\0'), 4);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_OCTETS, NULL, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "0xdeadbeef");
}

static void test_round_trip_cast_integer_string(void)
{
	value_data_t	a, b, c;

	a.integer = 987654;

	/* integer -> string -> integer must land back on the same number */
	TEST_CHECK(value_data_cast(autofree, &b, PW_TYPE_STRING, NULL, PW_TYPE_INTEGER, NULL, &a, 4) > 0);
	TEST_CHECK_SLEN(value_data_cast(autofree, &c, PW_TYPE_INTEGER, NULL,
					PW_TYPE_STRING, NULL, &b, strlen(b.strvalue)), 4);
	TEST_CHECK(c.integer == a.integer);
	TEST_MSG("Expected %u, got %u", a.integer, c.integer);
}

static void test_round_trip_copy_cmp(void)
{
	value_data_t	src, dst;

	src.strvalue = "round trip";

	/* A copy must compare equal to its original */
	TEST_CHECK_SLEN(value_data_copy(autofree, &dst, PW_TYPE_STRING, &src, 10), 10);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_STRING, &src, 10, PW_TYPE_STRING, &dst, 10), 0);
}

/*
 *	Enumerated values.
 *
 *	The DICT_ATTR argument to these functions exists only so that named
 *	values can be looked up.  Every other test in this file passes NULL,
 *	so without these the whole enumeration path is unreached.
 */
static void test_from_str_enum_name(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_INTEGER;

	TEST_CASE("A name from the dictionary resolves to its number");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, da_enum, "Login-User", -1, '\0'), 4);
	TEST_CHECK(dst.integer == 1);
	TEST_MSG("Expected Service-Type Login-User == 1, got %u", dst.integer);
}

static void test_from_str_enum_unknown_name(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_INTEGER;

	TEST_CASE("A name which is not in the dictionary is an error");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, da_enum, "Nonsense-Value", -1, '\0'), -1);
}

static void test_from_str_enum_number(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_INTEGER;

	TEST_CASE("A plain number is still a number, even with an enumeration");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, da_enum, "2", -1, '\0'), 4);
	TEST_CHECK(dst.integer == 2);
}

static void test_prints_enum_name(void)
{
	value_data_t	data;
	char		buff[64];

	TEST_CASE("A value with a name prints as the name");
	data.integer = 1;
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, da_enum, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "Login-User");

	TEST_CASE("A value with no name falls back to the number");
	data.integer = 99999;
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, da_enum, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "99999");
}

static void test_aprints_enum_name(void)
{
	value_data_t	data;
	char		*out;

	data.integer = 1;

	out = value_data_aprints(autofree, PW_TYPE_INTEGER, da_enum, &data, 4, '\0');
	TEST_ASSERT(out != NULL);
	if (!out) return;

	TEST_CHECK_STRCMP(out, "Login-User");
	talloc_free(out);
}

static void test_round_trip_enum(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_INTEGER;
	char		buff[64];

	/* name -> number -> name */
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, da_enum, "Framed-User", -1, '\0'), 4);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, da_enum, &data, 4, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "Framed-User");
}

/*
 *	CIDR comparisons.
 *
 *	An address compared against a prefix, or a prefix against a prefix,
 *	is routed through a separate containment routine rather than through
 *	value_data_cmp().  The operators do not mean what they mean for
 *	scalars: "less than" is "is contained within".
 */
static void test_cmp_op_addr_in_prefix(void)
{
	value_data_t	addr, prefix;

	mk_ipv4_addr(&addr, "10.0.0.1");
	mk_ipv4_prefix(&prefix, "10.0.0.0/8");

	TEST_CASE("An address inside the prefix is \"less than\" it");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 1);

	TEST_CASE("But not equal to it, because the netmasks differ");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_NE, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 1);

	TEST_CASE("A /32 cannot contain a /8");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GE, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 0);
}

static void test_cmp_op_addr_outside_prefix(void)
{
	value_data_t	addr, prefix;

	mk_ipv4_addr(&addr, "192.0.2.1");
	mk_ipv4_prefix(&prefix, "10.0.0.0/8");

	TEST_CASE("An address outside the prefix is not contained by it");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 0);

	TEST_CASE("Inequality still holds");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_NE, PW_TYPE_IPV4_ADDR, &addr, 4,
					 PW_TYPE_IPV4_PREFIX, &prefix, 6), 1);
}

static void test_cmp_op_prefix_identical(void)
{
	value_data_t	a, b;

	mk_ipv4_prefix(&a, "10.0.0.0/8");
	mk_ipv4_prefix(&b, "10.0.0.0/8");

	TEST_CASE("Identical prefixes are equal, and each contains the other");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GE, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 1);

	TEST_CASE("But neither is strictly inside the other");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_NE, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 0);
}

static void test_cmp_op_prefix_nested(void)
{
	value_data_t	inner, outer;

	mk_ipv4_prefix(&inner, "10.1.0.0/16");
	mk_ipv4_prefix(&outer, "10.0.0.0/8");

	TEST_CASE("A longer prefix is contained by the shorter one");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_PREFIX, &inner, 6,
					 PW_TYPE_IPV4_PREFIX, &outer, 6), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LE, PW_TYPE_IPV4_PREFIX, &inner, 6,
					 PW_TYPE_IPV4_PREFIX, &outer, 6), 1);

	TEST_CASE("And not the other way round");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_PREFIX, &outer, 6,
					 PW_TYPE_IPV4_PREFIX, &inner, 6), 0);

	TEST_CASE("Different netmasks are never equal");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV4_PREFIX, &inner, 6,
					 PW_TYPE_IPV4_PREFIX, &outer, 6), 0);
}

static void test_cmp_op_prefix_disjoint(void)
{
	value_data_t	a, b;

	mk_ipv4_prefix(&a, "10.1.0.0/16");
	mk_ipv4_prefix(&b, "192.168.0.0/16");

	TEST_CASE("Prefixes which do not overlap contain neither each other");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 0);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV4_PREFIX, &a, 6,
					 PW_TYPE_IPV4_PREFIX, &b, 6), 0);
}

static void test_cmp_op_prefix_vs_addr(void)
{
	value_data_t	prefix, addr;

	mk_ipv4_prefix(&prefix, "10.0.0.0/8");
	mk_ipv4_addr(&addr, "10.0.0.1");

	TEST_CASE("With the prefix on the left, containment reverses");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_GT, PW_TYPE_IPV4_PREFIX, &prefix, 6,
					 PW_TYPE_IPV4_ADDR, &addr, 4), 1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV4_PREFIX, &prefix, 6,
					 PW_TYPE_IPV4_ADDR, &addr, 4), 0);
}

static void test_cmp_op_ipv6_prefix(void)
{
	value_data_t	addr, prefix;
	PW_TYPE		type;

	type = PW_TYPE_IPV6_ADDR;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &addr, &type, NULL,
						   "2001:db8::1", -1, '\0'), 16);

	type = PW_TYPE_IPV6_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &prefix, &type, NULL,
						   "2001:db8::/32", -1, '\0'), 18);

	TEST_CASE("An IPv6 address inside its prefix");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV6_ADDR, &addr, 16,
					 PW_TYPE_IPV6_PREFIX, &prefix, 18), 1);

	TEST_CASE("And one outside it");
	type = PW_TYPE_IPV6_ADDR;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &addr, &type, NULL,
						   "2001:dba::1", -1, '\0'), 16);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_LT, PW_TYPE_IPV6_ADDR, &addr, 16,
					 PW_TYPE_IPV6_PREFIX, &prefix, 18), 0);
}

static void test_cmp_op_cross_family(void)
{
	value_data_t	v4, v6;
	PW_TYPE		type;

	mk_ipv4_addr(&v4, "192.0.2.1");

	type = PW_TYPE_IPV6_ADDR;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &v6, &type, NULL,
						   "2001:db8::1", -1, '\0'), 16);

	TEST_CASE("Comparing across address families is an error, not false");
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV4_ADDR, &v4, 4,
					 PW_TYPE_IPV6_ADDR, &v6, 16), -1);
	TEST_CHECK_RET(value_data_cmp_op(T_OP_CMP_EQ, PW_TYPE_IPV6_ADDR, &v6, 16,
					 PW_TYPE_IPV4_ADDR, &v4, 4), -1);
}

/*
 *	Types which the earlier tests never touched.
 */
static void test_cmp_boolean(void)
{
	value_data_t a, b;

	a.boolean = true;
	b.boolean = false;

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BOOLEAN, &a, 1, PW_TYPE_BOOLEAN, &b, 1), 1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BOOLEAN, &b, 1, PW_TYPE_BOOLEAN, &a, 1), -1);

	b.boolean = true;
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BOOLEAN, &a, 1, PW_TYPE_BOOLEAN, &b, 1), 0);
}

static void test_cmp_ifid(void)
{
	value_data_t a, b;

	memcpy(a.ifid, "\x00\x00\x00\x00\x00\x00\x00\x01", 8);
	memcpy(b.ifid, "\x00\x00\x00\x00\x00\x00\x00\x02", 8);

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IFID, &a, 8, PW_TYPE_IFID, &b, 8), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IFID, &b, 8, PW_TYPE_IFID, &a, 8), 1);

	memcpy(b.ifid, a.ifid, 8);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IFID, &a, 8, PW_TYPE_IFID, &b, 8), 0);
}

static void test_cmp_ipv6_addr(void)
{
	value_data_t	a, b;
	PW_TYPE		type;

	type = PW_TYPE_IPV6_ADDR;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &a, &type, NULL, "2001:db8::1", -1, '\0'), 16);
	type = PW_TYPE_IPV6_ADDR;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, "2001:db8::2", -1, '\0'), 16);

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV6_ADDR, &a, 16, PW_TYPE_IPV6_ADDR, &b, 16), -1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV6_ADDR, &b, 16, PW_TYPE_IPV6_ADDR, &a, 16), 1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV6_ADDR, &a, 16, PW_TYPE_IPV6_ADDR, &a, 16), 0);
}

/** Two identically-parsed prefixes must compare equal
 *
 * value_data_cmp() memcmp()s the whole prefix array, including the reserved
 * byte at index 0.  value_data_from_str() used to set only the prefix length
 * and the address, so that byte held stack garbage and two identical
 * prefixes compared as different depending on the caller's buffer.
 */
static void test_cmp_ipv4_prefix(void)
{
	value_data_t	a, b;
	PW_TYPE		type;

	/* Poison differently, so an uninitialised byte would show up */
	memset(&a, 0xAA, sizeof(a));
	type = PW_TYPE_IPV4_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &a, &type, NULL, "10.1.0.0/16", -1, '\0'), 6);

	memset(&b, 0x55, sizeof(b));
	type = PW_TYPE_IPV4_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, "10.1.0.0/16", -1, '\0'), 6);

	TEST_CASE("The reserved byte is initialised");
	TEST_CHECK(a.ipv4prefix[0] == 0);
	TEST_CHECK(b.ipv4prefix[0] == 0);

	TEST_CASE("So identical prefixes compare equal");
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV4_PREFIX, &a, 6, PW_TYPE_IPV4_PREFIX, &b, 6), 0);

	TEST_CASE("And different ones do not");
	type = PW_TYPE_IPV4_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, "10.2.0.0/16", -1, '\0'), 6);
	TEST_CHECK(value_data_cmp(PW_TYPE_IPV4_PREFIX, &a, 6, PW_TYPE_IPV4_PREFIX, &b, 6) != 0);
}

static void test_cmp_ipv6_prefix(void)
{
	value_data_t	a, b;
	PW_TYPE		type;

	memset(&a, 0xAA, sizeof(a));
	type = PW_TYPE_IPV6_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &a, &type, NULL, "2001:db8::/32", -1, '\0'), 18);

	memset(&b, 0x55, sizeof(b));
	type = PW_TYPE_IPV6_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, "2001:db8::/32", -1, '\0'), 18);

	TEST_CHECK(a.ipv6prefix[0] == 0);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_IPV6_PREFIX, &a, 18, PW_TYPE_IPV6_PREFIX, &b, 18), 0);
}

/*
 *	Parsing and printing the remaining types.
 */
static void test_from_str_date(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_DATE;

	TEST_CASE("A unix timestamp goes through the forgiving fallback parser");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "1234567890", -1, '\0'), 4);
	TEST_CHECK(dst.date == 1234567890);
	TEST_MSG("Expected 1234567890, got %u", dst.date);
}

static void test_round_trip_date(void)
{
	value_data_t	a, b;
	PW_TYPE		type = PW_TYPE_DATE;
	char		buff[128];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &a, &type, NULL, "1234567890", -1, '\0'), 4);

	/*
	 *	prints() uses the same format string that from_str() hands to
	 *	strptime(), so the printed form has to parse back to the same
	 *	instant.  Both go through localtime, so this holds whatever the
	 *	timezone is.
	 */
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_DATE, NULL, &a, 4, '\0') > 0);

	type = PW_TYPE_DATE;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, buff, -1, '\0'), 4);
	TEST_CHECK(b.date == a.date);
	TEST_MSG("\"%s\" parsed back to %u, expected %u", buff, b.date, a.date);
}

static void test_from_str_ifid(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IFID;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "0000:0000:3938:3737", -1, '\0'), 8);
	TEST_CHECK(memcmp(dst.ifid, "\x00\x00\x00\x00\x39\x38\x37\x37", 8) == 0);
}

static void test_from_str_ifid_bad(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IFID;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "not-an-ifid", -1, '\0'), -1);
}

static void test_from_str_ipv4_prefix(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IPV4_PREFIX;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "10.1.0.0/16", -1, '\0'), 6);

	/* Layout is reserved, prefix length, then the address */
	TEST_CHECK(dst.ipv4prefix[0] == 0);
	TEST_CHECK(dst.ipv4prefix[1] == 16);
	TEST_CHECK(memcmp(&dst.ipv4prefix[2], "\x0a\x01\x00\x00", 4) == 0);
}

static void test_from_str_ipv6_prefix(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_IPV6_PREFIX;

	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "2001:db8::/32", -1, '\0'), 18);
	TEST_CHECK(dst.ipv6prefix[0] == 0);
	TEST_CHECK(dst.ipv6prefix[1] == 32);
}

/** A combo address rewrites the caller's type
 *
 * PW_TYPE_COMBO_IP_ADDR is the one case where src_type is genuinely in/out:
 * the parser decides which family the string is and tells the caller.
 */
static void test_from_str_combo_ip(void)
{
	value_data_t	dst;
	PW_TYPE		type;

	TEST_CASE("An IPv4 string becomes PW_TYPE_IPV4_ADDR");
	type = PW_TYPE_COMBO_IP_ADDR;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "192.0.2.1", -1, '\0'), 4);
	TEST_CHECK(type == PW_TYPE_IPV4_ADDR);
	TEST_CHECK(dst.ipaddr.s_addr == htonl(0xc0000201));

	TEST_CASE("An IPv6 string becomes PW_TYPE_IPV6_ADDR");
	type = PW_TYPE_COMBO_IP_ADDR;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "2001:db8::1", -1, '\0'), 16);
	TEST_CHECK(type == PW_TYPE_IPV6_ADDR);
}

static void test_from_str_abinary(void)
{
	value_data_t	dst;
	PW_TYPE		type = PW_TYPE_ABINARY;

	TEST_CASE("An Ascend filter is parsed into its packed form");
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "ip in drop", -1, '\0'), 32);
}

static void test_prints_ifid(void)
{
	value_data_t	data;
	char		buff[64];

	memcpy(data.ifid, "\x00\x00\x00\x00\x39\x38\x37\x37", 8);

	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_IFID, NULL, &data, 8, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "0:0:3938:3737");
}

static void test_prints_ipv6_addr(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_IPV6_ADDR;
	char		buff[64];

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "2001:db8::1", -1, '\0'), 16);

	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_IPV6_ADDR, NULL, &data, 16, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "2001:db8::1");
}

static void test_prints_prefixes(void)
{
	value_data_t	data;
	PW_TYPE		type;
	char		buff[64];

	type = PW_TYPE_IPV4_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "10.1.0.0/16", -1, '\0'), 6);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_IPV4_PREFIX, NULL, &data, 6, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "10.1.0.0/16");

	type = PW_TYPE_IPV6_PREFIX;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &data, &type, NULL, "2001:db8::/32", -1, '\0'), 18);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_IPV6_PREFIX, NULL, &data, 18, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "2001:db8::/32");
}

/*
 *	Casts which the earlier tests did not reach.
 */
static void test_cast_integer64_to_ethernet(void)
{
	value_data_t	src, dst;

	TEST_CASE("A 48 bit value becomes the address");
	src.integer64 = 0x3938373C;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_ETHERNET, NULL,
					PW_TYPE_INTEGER64, NULL, &src, 8), 6);
	TEST_CHECK(memcmp(dst.ether, "\x00\x00\x39\x38\x37\x3c", 6) == 0);

	TEST_CASE("Anything which needs more than 48 bits is rejected");
	src.integer64 = UINT64_C(0x0001000000000000);
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_ETHERNET, NULL,
					PW_TYPE_INTEGER64, NULL, &src, 8), -1);
}

/** The integer and string forms of an address agree
 *
 * from_str() takes the decimal form and cast() takes the integer form, and
 * they used to disagree about which end of the 64 bit value the address
 * lived at.
 */
static void test_ethernet_integer_paths_agree(void)
{
	value_data_t	from_int, from_string;
	PW_TYPE		type = PW_TYPE_ETHERNET;

	from_int.integer64 = 0x3938373C;
	TEST_CHECK_SLEN_RETURN(value_data_cast(autofree, &from_int, PW_TYPE_ETHERNET, NULL,
					       PW_TYPE_INTEGER64, NULL, &from_int, 8), 6);

	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &from_string, &type, NULL,
						   "959985468", -1, '\0'), 6);

	TEST_CHECK_RET(value_data_cmp(PW_TYPE_ETHERNET, &from_int, 6,
				      PW_TYPE_ETHERNET, &from_string, 6), 0);
}

static void test_cast_ifid_to_integer64(void)
{
	value_data_t	src, dst;

	memcpy(src.ifid, "\x00\x00\x00\x00\x00\x00\x30\x39", 8);

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER64, NULL,
					PW_TYPE_IFID, NULL, &src, 8), 8);
	TEST_CHECK(dst.integer64 == 12345);
}

static void test_cast_octets_to_integer(void)
{
	value_data_t	src, dst;

	/* Octets are taken as the network-order representation */
	src.octets = (uint8_t const *)"\x00\x00\x30\x39";

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER, NULL,
					PW_TYPE_OCTETS, NULL, &src, 4), 4);
	TEST_CHECK(dst.integer == 12345);
}

static void test_cast_octets_to_ipaddr(void)
{
	value_data_t	src, dst;

	src.octets = (uint8_t const *)"\xc0\x00\x02\x01";

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_IPV4_ADDR, NULL,
					PW_TYPE_OCTETS, NULL, &src, 4), 4);
	TEST_CHECK(dst.ipaddr.s_addr == htonl(0xc0000201));
}

static void test_cast_string_to_octets(void)
{
	value_data_t	src, dst;

	src.strvalue = "0xdeadbeef";

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_OCTETS, NULL,
					PW_TYPE_STRING, NULL, &src, 10), 4);
	if (dst.octets) TEST_CHECK(memcmp(dst.octets, "\xde\xad\xbe\xef", 4) == 0);
}

static void test_cast_enum_string_to_integer(void)
{
	value_data_t	src, dst;

	TEST_CASE("A cast from a string consults the destination's enumeration");
	src.strvalue = "Login-User";

	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_INTEGER, da_enum,
					PW_TYPE_STRING, NULL, &src, 10), 4);
	TEST_CHECK(dst.integer == 1);
}

/*
 *	value_data_prints() for the remaining types, and its edge cases.
 */
static void test_prints_boolean(void)
{
	value_data_t	data;
	char		buff[64];

	TEST_CASE("true");
	data.boolean = true;
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_BOOLEAN, NULL, &data, 1, '\0'), 3);
	TEST_CHECK_STRCMP(buff, "yes");

	TEST_CASE("false");
	data.boolean = false;
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_BOOLEAN, NULL, &data, 1, '\0'), 2);
	TEST_CHECK_STRCMP(buff, "no");
}

static void test_aprints_boolean(void)
{
	value_data_t	data;
	char		buff[64];
	char		*out;

	data.boolean = true;
	out = value_data_aprints(autofree, PW_TYPE_BOOLEAN, NULL, &data, 1, '\0');
	TEST_ASSERT(out != NULL);
	if (!out) return;
	TEST_CHECK_STRCMP(out, "yes");

	TEST_CASE("and prints() says the same");
	value_data_prints(buff, sizeof(buff), PW_TYPE_BOOLEAN, NULL, &data, 1, '\0');
	TEST_CHECK_STRCMP(buff, out);
	talloc_free(out);

	data.boolean = false;
	out = value_data_aprints(autofree, PW_TYPE_BOOLEAN, NULL, &data, 1, '\0');
	TEST_ASSERT(out != NULL);
	if (!out) return;
	TEST_CHECK_STRCMP(out, "no");

	value_data_prints(buff, sizeof(buff), PW_TYPE_BOOLEAN, NULL, &data, 1, '\0');
	TEST_CHECK_STRCMP(buff, out);
	talloc_free(out);
}

static void test_from_str_boolean(void)
{
	value_data_t	dst;
	PW_TYPE		type;
	size_t		i;
	char const	*true_forms[]  = { "yes", "true", "1" };
	char const	*false_forms[] = { "no", "false", "0" };

	TEST_CASE("Forms which mean true");
	for (i = 0; i < (sizeof(true_forms) / sizeof(true_forms[0])); i++) {
		type = PW_TYPE_BOOLEAN;
		TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL,
						    true_forms[i], -1, '\0'), 1);
		TEST_CHECK(dst.boolean == true);
		TEST_MSG("\"%s\" should be true", true_forms[i]);
	}

	TEST_CASE("Forms which mean false");
	for (i = 0; i < (sizeof(false_forms) / sizeof(false_forms[0])); i++) {
		type = PW_TYPE_BOOLEAN;
		TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL,
						    false_forms[i], -1, '\0'), 1);
		TEST_CHECK(dst.boolean == false);
		TEST_MSG("\"%s\" should be false", false_forms[i]);
	}
}

static void test_from_str_boolean_bad(void)
{
	value_data_t	dst;
	PW_TYPE		type;
	size_t		i;
	char const	*bad[] = { "YES", "True", "maybe", "", "2", "-1", "yesno" };

	TEST_CASE("Anything else is an error, and the match is case sensitive");
	for (i = 0; i < (sizeof(bad) / sizeof(bad[0])); i++) {
		type = PW_TYPE_BOOLEAN;
		TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, bad[i], -1, '\0'), -1);
		TEST_MSG("\"%s\" should not parse", bad[i]);
	}
}

/** An explicit length is honoured
 *
 * The boolean case has to sit in the fixed-size half of the parser, after
 * src has been copied into a NUL terminated buffer.  Parsed from the
 * variable-length half instead, the strcmp() calls would run past src_len
 * and compare against the caller's whole buffer.
 */
static void test_from_str_boolean_length(void)
{
	value_data_t	dst;
	PW_TYPE		type;

	TEST_CASE("Only the first src_len bytes are considered");
	type = PW_TYPE_BOOLEAN;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "yesX", 3, '\0'), 1);
	TEST_CHECK(dst.boolean == true);

	type = PW_TYPE_BOOLEAN;
	TEST_CHECK_SLEN(value_data_from_str(autofree, &dst, &type, NULL, "nonsense", 2, '\0'), 1);
	TEST_CHECK(dst.boolean == false);
}

/** A boolean survives being printed and parsed again
 *
 * This also pins the returned length at 1, which comes from
 * dict_attr_sizes[].  That entry was missing at one point, which made every
 * successful boolean parse report zero bytes of data.
 */
static void test_round_trip_boolean(void)
{
	value_data_t	a, b;
	PW_TYPE		type;
	char		buff[64];

	TEST_CASE("true survives a round trip");
	type = PW_TYPE_BOOLEAN;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &a, &type, NULL, "yes", -1, '\0'), 1);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_BOOLEAN, NULL, &a, 1, '\0') > 0);

	type = PW_TYPE_BOOLEAN;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, buff, -1, '\0'), 1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BOOLEAN, &a, 1, PW_TYPE_BOOLEAN, &b, 1), 0);

	TEST_CASE("and so does false");
	type = PW_TYPE_BOOLEAN;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &a, &type, NULL, "no", -1, '\0'), 1);
	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_BOOLEAN, NULL, &a, 1, '\0') > 0);

	type = PW_TYPE_BOOLEAN;
	TEST_CHECK_SLEN_RETURN(value_data_from_str(autofree, &b, &type, NULL, buff, -1, '\0'), 1);
	TEST_CHECK_RET(value_data_cmp(PW_TYPE_BOOLEAN, &a, 1, PW_TYPE_BOOLEAN, &b, 1), 0);

	TEST_CASE("The printed form is one the parser accepts");
	TEST_CHECK_STRCMP(buff, "no");
}

/** A boolean and a byte convert both ways
 *
 * Both are one byte and share the same byte order, so this pair falls
 * through to the plain copy at the end of value_data_cast().  That copy
 * used to take the addresses of the dst and src pointers rather than the
 * pointers themselves, so it copied between two local variables and never
 * wrote the destination at all.  The path only became reachable once
 * PW_TYPE_BOOLEAN gained an entry in dict_attr_sizes[]; before that the
 * size check rejected the cast.
 */
static void test_cast_boolean_byte(void)
{
	value_data_t	src, dst;

	TEST_CASE("byte to boolean");
	src.byte = 1;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BOOLEAN, NULL,
					PW_TYPE_BYTE, NULL, &src, 1), 1);
	TEST_CHECK(dst.boolean == true);

	src.byte = 0;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BOOLEAN, NULL,
					PW_TYPE_BYTE, NULL, &src, 1), 1);
	TEST_CHECK(dst.boolean == false);

	TEST_CASE("boolean to byte");
	src.boolean = true;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BYTE, NULL,
					PW_TYPE_BOOLEAN, NULL, &src, 1), 1);
	TEST_CHECK(dst.byte == 1);

	src.boolean = false;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BYTE, NULL,
					PW_TYPE_BOOLEAN, NULL, &src, 1), 1);
	TEST_CHECK(dst.byte == 0);
}

/** A boolean is one byte, so a wider type cannot be cast to it
 *
 * The bounds come from dict_attr_sizes[PW_TYPE_BOOLEAN], which is {1, 1}.
 */
static void test_cast_to_boolean_wrong_size(void)
{
	value_data_t	src, dst;

	src.integer = 1;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BOOLEAN, NULL,
					PW_TYPE_INTEGER, NULL, &src, 4), -1);
}

static void test_cast_boolean_to_string(void)
{
	value_data_t	src, dst;

	src.boolean = true;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_STRING, NULL,
					PW_TYPE_BOOLEAN, NULL, &src, 1), 3);
	TEST_CHECK_STRCMP(dst.strvalue, "yes");

	src.boolean = false;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_STRING, NULL,
					PW_TYPE_BOOLEAN, NULL, &src, 1), 2);
	TEST_CHECK_STRCMP(dst.strvalue, "no");
}

/** Casting a boolean to octets is not supported
 *
 * value_data_hton(), which the cast to octets goes through, has no case for
 * PW_TYPE_BOOLEAN.  Note that the error text reads "Invalid cast to bool"
 * even though the cast was *from* bool, because hton() is called with the
 * source type in its destination argument.
 */
static void test_cast_boolean_to_octets_unsupported(void)
{
	value_data_t	src, dst;

	src.boolean = true;
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_OCTETS, NULL,
					PW_TYPE_BOOLEAN, NULL, &src, 1), -1);
}

static void test_cast_string_to_boolean(void)
{
	value_data_t	src, dst;

	src.strvalue = "yes";
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BOOLEAN, NULL,
					PW_TYPE_STRING, NULL, &src, 3), 1);
	TEST_CHECK(dst.boolean == true);

	src.strvalue = "maybe";
	TEST_CHECK_SLEN(value_data_cast(autofree, &dst, PW_TYPE_BOOLEAN, NULL,
					PW_TYPE_STRING, NULL, &src, 5), -1);
}

static void test_prints_byte(void)
{
	value_data_t	data;
	char		buff[64];

	data.byte = 200;
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_BYTE, NULL, &data, 1, '\0'), 3);
	TEST_CHECK_STRCMP(buff, "200");
}

static void test_prints_short(void)
{
	value_data_t	data;
	char		buff[64];

	data.ushort = 40000;
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_SHORT, NULL, &data, 2, '\0'), 5);
	TEST_CHECK_STRCMP(buff, "40000");
}

static void test_prints_integer64(void)
{
	value_data_t	data;
	char		buff[64];

	data.integer64 = UINT64_C(18446744073709551615);
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER64, NULL, &data, 8, '\0'), 20);
	TEST_CHECK_STRCMP(buff, "18446744073709551615");
}

static void test_prints_abinary(void)
{
	value_data_t	data;
	PW_TYPE		type = PW_TYPE_ABINARY;
	ssize_t		slen;
	char		buff[128];

	slen = value_data_from_str(autofree, &data, &type, NULL, "ip in drop", -1, '\0');
	TEST_CHECK_SLEN_RETURN(slen, 32);

	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_ABINARY, NULL, &data, slen, '\0') > 0);
	TEST_CHECK_STRCMP(buff, "ip in drop 0");
}

static void test_prints_empty_octets(void)
{
	value_data_t	data;
	char		buff[64];

	data.octets = (uint8_t const *)"";

	TEST_CASE("Zero-length octets still get the 0x prefix");
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_OCTETS, NULL, &data, 0, '\0'), 2);
	TEST_CHECK_STRCMP(buff, "0x");
}

/** A zero-length output buffer returns the input length
 *
 * Note that this is inlen, not the length which would have been needed.  The
 * buffer is not touched, so the caller must not read it.
 */
static void test_prints_zero_outlen(void)
{
	value_data_t	data;
	char		buff[64];

	data.integer = 12345;

	TEST_CHECK_LEN(value_data_prints(buff, 0, PW_TYPE_INTEGER, NULL, &data, 4, '\0'), 4);
}

static void test_prints_null_data(void)
{
	char buff[64];

	TEST_CASE("A NULL value prints nothing");
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_INTEGER, NULL, NULL, 4, '\0'), 0);
}

/** prints() wraps a quoted string, aprints() only escapes it
 *
 * Given the same quote character, value_data_prints() adds the surrounding
 * quotes and escapes any inside, while value_data_aprints() escapes without
 * wrapping.  Callers which build a string out of both need to know this.
 */
static void test_prints_quoting_vs_aprints(void)
{
	value_data_t	data;
	char		buff[128];
	char		*out;

	data.strvalue = "he said \"hi\"";

	TEST_CHECK(value_data_prints(buff, sizeof(buff), PW_TYPE_STRING, NULL, &data, 12, '"') > 0);
	TEST_CHECK_STRCMP(buff, "\"he said \\\"hi\\\"\"");

	out = value_data_aprints(autofree, PW_TYPE_STRING, NULL, &data, 12, '"');
	TEST_ASSERT(out != NULL);
	if (!out) return;
	TEST_CHECK_STRCMP(out, "he said \\\"hi\\\"");
	talloc_free(out);
}

static void test_prints_truncation_string(void)
{
	value_data_t	data;
	char		buff[4];

	data.strvalue = "hello world";

	TEST_CASE("A truncated string still reports the length needed");
	TEST_CHECK_LEN(value_data_prints(buff, sizeof(buff), PW_TYPE_STRING, NULL, &data, 11, '\0'), 11);

	TEST_CASE("And what did fit is NUL terminated");
	TEST_CHECK(strlen(buff) < sizeof(buff));
}

TEST_LIST = {
	/*
	 *	value_data_cmp()
	 */
	{ "cmp_integer",			test_cmp_integer },
	{ "cmp_byte",				test_cmp_byte },
	{ "cmp_short",				test_cmp_short },
	{ "cmp_signed",				test_cmp_signed },
	{ "cmp_integer64",			test_cmp_integer64 },
	{ "cmp_date",				test_cmp_date },
	{ "cmp_string",				test_cmp_string },
	{ "cmp_string_embedded_nul",		test_cmp_string_embedded_nul },
	{ "cmp_octets",				test_cmp_octets },
	{ "cmp_octets_length",			test_cmp_octets_length },
	{ "cmp_octets_zero_length",		test_cmp_octets_zero_length },
	{ "cmp_ipaddr",				test_cmp_ipaddr },
	{ "cmp_ether",				test_cmp_ether },
	{ "cmp_different_types",		test_cmp_different_types },

	/*
	 *	value_data_cmp_op()
	 */
	{ "cmp_op_eq",				test_cmp_op_eq },
	{ "cmp_op_ne",				test_cmp_op_ne },
	{ "cmp_op_lt",				test_cmp_op_lt },
	{ "cmp_op_gt",				test_cmp_op_gt },
	{ "cmp_op_le",				test_cmp_op_le },
	{ "cmp_op_ge",				test_cmp_op_ge },
	{ "cmp_op_string",			test_cmp_op_string },

	/*
	 *	value_data_from_str()
	 */
	{ "from_str_integer",			test_from_str_integer },
	{ "from_str_integer_bad",		test_from_str_integer_bad },
	{ "from_str_integer_accepted_forms",	test_from_str_integer_accepted_forms },
	{ "from_str_byte",			test_from_str_byte },
	{ "from_str_short",			test_from_str_short },
	{ "from_str_signed",			test_from_str_signed },
	{ "from_str_integer64",			test_from_str_integer64 },
	{ "from_str_string",			test_from_str_string },
	{ "from_str_string_length",		test_from_str_string_length },
	{ "from_str_octets_hex",		test_from_str_octets_hex },
	{ "from_str_octets_verbatim",		test_from_str_octets_verbatim },
	{ "from_str_octets_odd_hex",		test_from_str_octets_odd_hex },
	{ "from_str_ipaddr",			test_from_str_ipaddr },
	{ "from_str_ipaddr_bad",		test_from_str_ipaddr_bad },
	{ "from_str_ipv6addr",			test_from_str_ipv6addr },
	{ "from_str_ether",			test_from_str_ether },
	{ "from_str_ether_short",		test_from_str_ether_short },
	{ "from_str_ether_bad",			test_from_str_ether_bad },
	{ "from_str_ether_from_integer",	test_from_str_ether_from_integer },
	{ "from_str_null",			test_from_str_null },

	/*
	 *	value_data_cast()
	 */
	{ "cast_integer_to_integer64",		test_cast_integer_to_integer64 },
	{ "cast_byte_to_integer",		test_cast_byte_to_integer },
	{ "cast_short_to_integer",		test_cast_short_to_integer },
	{ "cast_integer_to_string",		test_cast_integer_to_string },
	{ "cast_string_to_integer",		test_cast_string_to_integer },
	{ "cast_string_to_integer_bad",		test_cast_string_to_integer_bad },
	{ "cast_integer_to_octets",		test_cast_integer_to_octets },
	{ "cast_ipaddr_to_string",		test_cast_ipaddr_to_string },
	{ "cast_string_to_ipaddr",		test_cast_string_to_ipaddr },

	/*
	 *	value_data_copy()
	 */
	{ "copy_integer",			test_copy_integer },
	{ "copy_string",			test_copy_string },
	{ "copy_octets",			test_copy_octets },
	{ "copy_string_embedded_nul",		test_copy_string_embedded_nul },

	/*
	 *	value_data_prints()
	 */
	{ "prints_integer",			test_prints_integer },
	{ "prints_signed_negative",		test_prints_signed_negative },
	{ "prints_string",			test_prints_string },
	{ "prints_string_quoted",		test_prints_string_quoted },
	{ "prints_octets",			test_prints_octets },
	{ "prints_ipaddr",			test_prints_ipaddr },
	{ "prints_ether",			test_prints_ether },
	{ "prints_truncation",			test_prints_truncation },

	/*
	 *	value_data_aprints()
	 */
	{ "aprints_integer",			test_aprints_integer },
	{ "aprints_string",			test_aprints_string },
	{ "aprints_octets",			test_aprints_octets },

	/*
	 *	Round trips
	 */
	{ "round_trip_integer",			test_round_trip_integer },
	{ "round_trip_signed",			test_round_trip_signed },
	{ "round_trip_ipaddr",			test_round_trip_ipaddr },
	{ "round_trip_ether",			test_round_trip_ether },
	{ "round_trip_octets",			test_round_trip_octets },
	{ "round_trip_cast_integer_string",	test_round_trip_cast_integer_string },
	{ "round_trip_copy_cmp",		test_round_trip_copy_cmp },

	/*
	 *	Enumerated values
	 */
	{ "from_str_enum_name",			test_from_str_enum_name },
	{ "from_str_enum_unknown_name",		test_from_str_enum_unknown_name },
	{ "from_str_enum_number",		test_from_str_enum_number },
	{ "prints_enum_name",			test_prints_enum_name },
	{ "aprints_enum_name",			test_aprints_enum_name },
	{ "round_trip_enum",			test_round_trip_enum },
	{ "cast_enum_string_to_integer",	test_cast_enum_string_to_integer },

	/*
	 *	CIDR comparisons
	 */
	{ "cmp_op_addr_in_prefix",		test_cmp_op_addr_in_prefix },
	{ "cmp_op_addr_outside_prefix",		test_cmp_op_addr_outside_prefix },
	{ "cmp_op_prefix_identical",		test_cmp_op_prefix_identical },
	{ "cmp_op_prefix_nested",		test_cmp_op_prefix_nested },
	{ "cmp_op_prefix_disjoint",		test_cmp_op_prefix_disjoint },
	{ "cmp_op_prefix_vs_addr",		test_cmp_op_prefix_vs_addr },
	{ "cmp_op_ipv6_prefix",			test_cmp_op_ipv6_prefix },
	{ "cmp_op_cross_family",		test_cmp_op_cross_family },

	/*
	 *	Remaining types
	 */
	{ "cmp_boolean",			test_cmp_boolean },
	{ "cmp_ifid",				test_cmp_ifid },
	{ "cmp_ipv6_addr",			test_cmp_ipv6_addr },
	{ "cmp_ipv4_prefix",			test_cmp_ipv4_prefix },
	{ "cmp_ipv6_prefix",			test_cmp_ipv6_prefix },
	{ "from_str_date",			test_from_str_date },
	{ "from_str_ifid",			test_from_str_ifid },
	{ "from_str_ifid_bad",			test_from_str_ifid_bad },
	{ "from_str_ipv4_prefix",		test_from_str_ipv4_prefix },
	{ "from_str_ipv6_prefix",		test_from_str_ipv6_prefix },
	{ "from_str_combo_ip",			test_from_str_combo_ip },
	{ "from_str_abinary",			test_from_str_abinary },
	{ "prints_ifid",			test_prints_ifid },
	{ "prints_ipv6_addr",			test_prints_ipv6_addr },
	{ "prints_prefixes",			test_prints_prefixes },
	{ "round_trip_date",			test_round_trip_date },

	/*
	 *	Further casts
	 */
	{ "cast_integer64_to_ethernet",		test_cast_integer64_to_ethernet },
	{ "ethernet_integer_paths_agree",	test_ethernet_integer_paths_agree },
	{ "cast_ifid_to_integer64",		test_cast_ifid_to_integer64 },
	{ "cast_octets_to_integer",		test_cast_octets_to_integer },
	{ "cast_octets_to_ipaddr",		test_cast_octets_to_ipaddr },
	{ "cast_string_to_octets",		test_cast_string_to_octets },

	/*
	 *	value_data_prints() : remaining types and edge cases
	 */
	{ "prints_boolean",			test_prints_boolean },
	{ "aprints_boolean",			test_aprints_boolean },
	{ "from_str_boolean",			test_from_str_boolean },
	{ "from_str_boolean_bad",		test_from_str_boolean_bad },
	{ "from_str_boolean_length",		test_from_str_boolean_length },
	{ "round_trip_boolean",			test_round_trip_boolean },
	{ "cast_string_to_boolean",		test_cast_string_to_boolean },
	{ "cast_boolean_byte",			test_cast_boolean_byte },
	{ "cast_to_boolean_wrong_size",		test_cast_to_boolean_wrong_size },
	{ "cast_boolean_to_string",		test_cast_boolean_to_string },
	{ "cast_boolean_to_octets_unsupported",	test_cast_boolean_to_octets_unsupported },
	{ "prints_byte",			test_prints_byte },
	{ "prints_short",			test_prints_short },
	{ "prints_integer64",			test_prints_integer64 },
	{ "prints_abinary",			test_prints_abinary },
	{ "prints_empty_octets",		test_prints_empty_octets },
	{ "prints_zero_outlen",			test_prints_zero_outlen },
	{ "prints_null_data",			test_prints_null_data },
	{ "prints_quoting_vs_aprints",		test_prints_quoting_vs_aprints },
	{ "prints_truncation_string",		test_prints_truncation_string },

	TEST_TERMINATOR
};
