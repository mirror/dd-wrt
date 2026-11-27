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

/** Tests for the VALUE_PAIR manipulation and search API
 *
 * @file src/tests/api/pair_tests.c
 *
 * Covers the public functions of src/lib/pair.c.
 *
 * Note that in v3 a list of pairs is a bare VALUE_PAIR * singly linked
 * through vp->next, not the fr_pair_list_t of later versions, so every
 * list is passed around as a VALUE_PAIR ** head pointer.
 *
 * @copyright 2026 The FreeRADIUS server project
 */
#include <freeradius-devel/libradius.h>
#include <freeradius-devel/conf.h>	/* RADIUS_DICTIONARY */

#include "acutest_common_init.h"
#include "acutest_helpers.h"

static TALLOC_CTX	*autofree;

/*
 *	The Tmp-* attributes in the internal dictionary exist for exactly
 *	this sort of thing: one of every interesting type, with no protocol
 *	meaning attached.
 */
static DICT_ATTR const	*da_string;	/* Tmp-String-0    1800 string    */
static DICT_ATTR const	*da_string1;	/* Tmp-String-1    1801 string    */
static DICT_ATTR const	*da_integer;	/* Tmp-Integer-0   1810 integer   */
static DICT_ATTR const	*da_integer1;	/* Tmp-Integer-1   1811 integer   */
static DICT_ATTR const	*da_ipaddr;	/* Tmp-IP-Address-0 1820 ipaddr   */
static DICT_ATTR const	*da_octets;	/* Tmp-Octets-0    1830 octets    */
static DICT_ATTR const	*da_integer64;	/* Tmp-Integer64-0 1871 integer64 */
static DICT_ATTR const	*da_tagged;	/* Tunnel-Type       64 integer, has_tag */
static DICT_ATTR const	*da_ipv4prefix;	/* Tmp-Cast-IPv4Prefix 1870 ipv4prefix */
static DICT_ATTR const	*da_ipv6;	/* Tmp-Cast-IPv6Addr   1858 ipv6addr   */
static DICT_ATTR const	*da_ipv6prefix;	/* Tmp-Cast-IPv6Prefix 1859 ipv6prefix */

static char const	*test_string = "We love testing!";
static uint8_t const	test_octets[] = { 0xde, 0xad, 0xbe, 0xef };

static void test_init(void) __attribute__((constructor));
static void test_init(void)
{
	char const *dict_dir = getenv("DICT_DIR");

DIAG_OFF(deprecated-declarations)
	autofree = talloc_autofree_context();
DIAG_ON(deprecated-declarations)
	if (!autofree) {
		fr_perror("pair_tests");
		exit(EXIT_FAILURE);
	}

	/*
	 *	Mismatch between the binary and the libraries it depends on.
	 */
	if (fr_check_lib_magic(RADIUSD_MAGIC_NUMBER) < 0) {
		fr_perror("pair_tests");
		exit(EXIT_FAILURE);
	}

	if (!dict_dir) dict_dir = "share";

	if (dict_init(dict_dir, RADIUS_DICTIONARY) < 0) {
		fr_perror("pair_tests: failed loading dictionaries from \"%s\"", dict_dir);
		fprintf(stderr, "Set DICT_DIR to the directory holding \"dictionary\"\n");
		exit(EXIT_FAILURE);
	}

	da_string	= dict_attrbyname("Tmp-String-0");
	da_string1	= dict_attrbyname("Tmp-String-1");
	da_integer	= dict_attrbyname("Tmp-Integer-0");
	da_integer1	= dict_attrbyname("Tmp-Integer-1");
	da_ipaddr	= dict_attrbyname("Tmp-IP-Address-0");
	da_octets	= dict_attrbyname("Tmp-Octets-0");
	da_integer64	= dict_attrbyname("Tmp-Integer64-0");
	da_tagged	= dict_attrbyname("Tunnel-Type");
	da_ipv4prefix	= dict_attrbyname("Tmp-Cast-IPv4Prefix");
	da_ipv6		= dict_attrbyname("Tmp-Cast-IPv6Addr");
	da_ipv6prefix	= dict_attrbyname("Tmp-Cast-IPv6Prefix");

	if (!da_string || !da_string1 || !da_integer || !da_integer1 ||
	    !da_ipaddr || !da_octets || !da_integer64 || !da_tagged ||
	    !da_ipv4prefix || !da_ipv6 || !da_ipv6prefix) {
		fprintf(stderr, "pair_tests: dictionary is missing one of the test attributes\n");
		exit(EXIT_FAILURE);
	}
}

/** Build a three element list: Tmp-String-0, Tmp-Integer-0, Tmp-Octets-0
 */
static VALUE_PAIR *test_list_alloc(TALLOC_CTX *ctx)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;

	vp = fr_pair_afrom_da(ctx, da_string);
	if (!vp) return NULL;
	fr_pair_value_strcpy(vp, test_string);
	fr_pair_add(&head, vp);

	vp = fr_pair_afrom_da(ctx, da_integer);
	if (!vp) return NULL;
	vp->vp_integer = 12345;
	fr_pair_add(&head, vp);

	vp = fr_pair_afrom_da(ctx, da_octets);
	if (!vp) return NULL;
	fr_pair_value_memcpy(vp, test_octets, sizeof(test_octets));
	fr_pair_add(&head, vp);

	return head;
}

static size_t list_len(VALUE_PAIR *head)
{
	size_t		count = 0;
	VALUE_PAIR	*vp;

	for (vp = head; vp; vp = vp->next) count++;

	return count;
}

/*
 *	Allocation
 *
 *	All checks for functions which allocate pairs must use TEST_ASSERT and not TEST_CHECK.
 *	If TEST_CHECK fails, the code continues, and dereferences a NULL pointer.
 */
static void test_fr_pair_alloc(void)
{
	VALUE_PAIR *vp;

	TEST_CASE("Allocation using fr_pair_alloc");
	vp = fr_pair_alloc(autofree);
	TEST_ASSERT(vp != NULL);

	/* A bare pair has no dictionary attribute yet */
	TEST_CHECK(vp->da == NULL);
	TEST_CHECK(vp->next == NULL);

	talloc_free(vp);
}

static void test_fr_pair_afrom_da(void)
{
	VALUE_PAIR *vp;

	TEST_CASE("Allocation using fr_pair_afrom_da");
	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK(vp->da == da_string);
	TEST_CHECK(vp->op == T_OP_EQ);

	fr_pair_value_strcpy(vp, test_string);
	TEST_CHECK_STRCMP(vp->vp_strvalue, test_string);
	TEST_CHECK_LEN(vp->vp_length, strlen(test_string));

	VERIFY_VP(vp);

	talloc_free(vp);
}

static void test_fr_pair_afrom_num(void)
{
	VALUE_PAIR *vp;

	TEST_CASE("Allocation using fr_pair_afrom_num");
	vp = fr_pair_afrom_num(autofree, 1800, 0);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK(vp->da == da_string);
	TEST_CHECK(vp->da->type == PW_TYPE_STRING);

	talloc_free(vp);
}

/** An attribute which is not in the dictionary cannot be allocated by number
 *
 * fr_pair_afrom_num() is a dictionary lookup followed by fr_pair_afrom_da(),
 * and it does not synthesise an unknown attribute for a number it cannot
 * find.  Callers which want an unknown have to build the DICT_ATTR
 * themselves, or allocate a known pair and call fr_pair_to_unknown().
 */
static void test_fr_pair_afrom_num_unknown(void)
{
	TEST_CASE("An attribute absent from the dictionary returns NULL");
	TEST_CHECK(fr_pair_afrom_num(autofree, 9999, 999999) == NULL);

	TEST_CASE("A known attribute with the wrong vendor also returns NULL");
	TEST_CHECK(fr_pair_afrom_num(autofree, 1800, 999999) == NULL);
}

static void test_fr_pair_to_unknown(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);

	vp->vp_integer = 12345;

	TEST_CASE("Converting a known attribute to an unknown one");
	TEST_CHECK_RET(fr_pair_to_unknown(vp), 0);

	TEST_CHECK(vp->da->flags.is_unknown);
	TEST_CHECK(vp->da->attr == 1810);

	talloc_free(vp);
}

static void test_fr_pair_list_free(void)
{
	VALUE_PAIR *head;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	TEST_CHECK_LEN(list_len(head), 3);

	fr_pair_list_free(&head);

	/* Freeing the list must also clear the caller's head pointer */
	TEST_CHECK(head == NULL);
}

/*
 *	Setting values
 */
static void test_fr_pair_value_strcpy(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	fr_pair_value_strcpy(vp, "hello");
	TEST_CHECK_STRCMP(vp->vp_strvalue, "hello");
	TEST_CHECK_LEN(vp->vp_length, 5);
	TEST_CHECK(vp->type == VT_DATA);

	/* Overwriting must replace, not append */
	fr_pair_value_strcpy(vp, "goodbye");
	TEST_CHECK_STRCMP(vp->vp_strvalue, "goodbye");
	TEST_CHECK_LEN(vp->vp_length, 7);

	talloc_free(vp);
}

static void test_fr_pair_value_bstrncpy(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	/* Copy a fixed length, ignoring any NUL inside it */
	fr_pair_value_bstrncpy(vp, "hello world", 5);
	TEST_CHECK_STRCMP(vp->vp_strvalue, "hello");
	TEST_CHECK_LEN(vp->vp_length, 5);

	talloc_free(vp);
}

static void test_fr_pair_value_strsteal(void)
{
	VALUE_PAIR	*vp;
	char		*str;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	str = talloc_strdup(autofree, "stolen");
	fr_pair_value_strsteal(vp, str);

	/* Stealing takes the buffer itself, it does not copy it */
	TEST_CHECK(vp->vp_strvalue == str);
	TEST_CHECK_STRCMP(vp->vp_strvalue, "stolen");
	TEST_CHECK_LEN(vp->vp_length, 6);

	talloc_free(vp);
}

static void test_fr_pair_value_sprintf(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	fr_pair_value_sprintf(vp, "%s-%i", "test", 42);
	TEST_CHECK_STRCMP(vp->vp_strvalue, "test-42");
	TEST_CHECK_LEN(vp->vp_length, 7);

	talloc_free(vp);
}

static void test_fr_pair_value_memcpy(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_octets);
	TEST_ASSERT(vp != NULL);

	fr_pair_value_memcpy(vp, test_octets, sizeof(test_octets));
	TEST_CHECK_LEN(vp->vp_length, sizeof(test_octets));
	TEST_CHECK(memcmp(vp->vp_octets, test_octets, sizeof(test_octets)) == 0);

	/* A copy, so the pair must not alias the source */
	TEST_CHECK(vp->vp_octets != test_octets);

	talloc_free(vp);
}

static void test_fr_pair_value_memsteal(void)
{
	VALUE_PAIR	*vp;
	uint8_t		*buff;

	vp = fr_pair_afrom_da(autofree, da_octets);
	TEST_ASSERT(vp != NULL);

	buff = talloc_array(autofree, uint8_t, 4);
	TEST_ASSERT(buff != NULL);
	memcpy(buff, test_octets, 4);

	fr_pair_value_memsteal(vp, buff);
	TEST_CHECK(vp->vp_octets == buff);
	TEST_CHECK_LEN(vp->vp_length, 4);

	talloc_free(vp);
}

static void test_fr_pair_value_from_str(void)
{
	VALUE_PAIR *vp;

	TEST_CASE("Parsing an integer from a string");
	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK_RET(fr_pair_value_from_str(vp, "54321", -1), 0);
	TEST_CHECK(vp->vp_integer == 54321);
	talloc_free(vp);

	TEST_CASE("Parsing an IP address from a string");
	vp = fr_pair_afrom_da(autofree, da_ipaddr);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK_RET(fr_pair_value_from_str(vp, "192.0.2.1", -1), 0);
	TEST_CHECK(vp->vp_ipaddr == htonl(0xc0000201));
	talloc_free(vp);

	TEST_CASE("Parsing octets from a hex string");
	vp = fr_pair_afrom_da(autofree, da_octets);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK_RET(fr_pair_value_from_str(vp, "0xdeadbeef", -1), 0);
	TEST_CHECK_LEN(vp->vp_length, 4);
	TEST_CHECK(memcmp(vp->vp_octets, test_octets, 4) == 0);
	talloc_free(vp);
}

static void test_fr_pair_mark_xlat(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	TEST_CASE("Marking a pair for later expansion");
	TEST_CHECK_RET(fr_pair_mark_xlat(vp, "%{User-Name}"), 0);

	/* The value is held as an unexpanded string, not as data */
	TEST_CHECK(vp->type == VT_XLAT);
	TEST_CHECK_STRCMP(vp->value.xlat, "%{User-Name}");

	talloc_free(vp);
}

/*
 *	Adding to and searching lists
 */
static void test_fr_pair_add(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	TEST_CHECK(head == vp);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	/* Added to the tail, so the head does not move */
	TEST_CHECK_LEN(list_len(head), 2);
	TEST_CHECK(head->next == vp);

	fr_pair_list_free(&head);
}

static void test_fr_pair_prepend(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*first, *second;

	first = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(first != NULL);
	fr_pair_add(&head, first);

	second = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(second != NULL);
	fr_pair_prepend(&head, second);

	/* Prepending moves the head */
	TEST_CHECK(head == second);
	if (!head) return;
	TEST_ASSERT(head->next == first);
	TEST_CHECK_LEN(list_len(head), 2);

	fr_pair_list_free(&head);
}

static void test_fr_pair_find_by_num(void)
{
	VALUE_PAIR	*head;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	vp = fr_pair_find_by_num(head, 1810, 0, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) {
		TEST_CHECK(vp->da == da_integer);
		TEST_CHECK(vp->vp_integer == 12345);
	}

	TEST_CASE("An attribute which is not in the list");
	TEST_CHECK(fr_pair_find_by_num(head, da_integer1->attr, da_integer1->vendor, TAG_ANY) == NULL);

	fr_pair_list_free(&head);
}

static void test_fr_pair_find_by_da(void)
{
	VALUE_PAIR	*head;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	if (!TEST_CHECK(head != NULL)) return;

	vp = fr_pair_find_by_da(head, da_octets, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK_LEN(vp->vp_length, sizeof(test_octets));

	TEST_CHECK(fr_pair_find_by_da(head, da_integer64, TAG_ANY) == NULL);

	fr_pair_list_free(&head);
}

static void test_find_by_num_tagged(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;

	vp = fr_pair_afrom_da(autofree, da_tagged);
	TEST_ASSERT(vp != NULL);
	vp->tag = 1;
	vp->vp_integer = 1;
	fr_pair_add(&head, vp);

	vp = fr_pair_afrom_da(autofree, da_tagged);
	TEST_ASSERT(vp != NULL);
	vp->tag = 2;
	vp->vp_integer = 2;
	fr_pair_add(&head, vp);

	TEST_CASE("A specific tag selects a specific pair");
	vp = fr_pair_find_by_num(head, da_tagged->attr, da_tagged->vendor, 2);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK(vp->vp_integer == 2);

	TEST_CASE("TAG_ANY matches the first one");
	vp = fr_pair_find_by_num(head, da_tagged->attr, da_tagged->vendor, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK(vp->vp_integer == 1);

	TEST_CASE("A tag which is not present matches nothing");
	TEST_CHECK(fr_pair_find_by_num(head, da_tagged->attr, da_tagged->vendor, 3) == NULL);

	fr_pair_list_free(&head);
}

/*
 *	Deleting from lists
 */
static void test_fr_pair_delete_by_num(void)
{
	VALUE_PAIR *head;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	fr_pair_delete_by_num(&head, 1810, 0, TAG_ANY);

	TEST_CHECK_LEN(list_len(head), 2);
	TEST_CHECK(fr_pair_find_by_num(head, 1810, 0, TAG_ANY) == NULL);

	/* The others must survive */
	TEST_CHECK(fr_pair_find_by_da(head, da_string, TAG_ANY) != NULL);
	TEST_CHECK(fr_pair_find_by_da(head, da_octets, TAG_ANY) != NULL);

	fr_pair_list_free(&head);
}

static void test_fr_pair_delete_by_da(void)
{
	VALUE_PAIR *head;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	/* Deleting the head must move the caller's head pointer */
	fr_pair_delete_by_da(&head, da_string);

	TEST_CHECK_LEN(list_len(head), 2);
	TEST_CHECK(fr_pair_find_by_da(head, da_string, TAG_ANY) == NULL);
	TEST_CHECK(head->da == da_integer);

	fr_pair_list_free(&head);
}

static void test_fr_pair_delete(void)
{
	VALUE_PAIR	*head;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	vp = fr_pair_find_by_da(head, da_integer, TAG_ANY);
	TEST_ASSERT(vp != NULL);

	fr_pair_delete(&head, vp);

	TEST_CHECK_LEN(list_len(head), 2);
	TEST_CHECK(fr_pair_find_by_da(head, da_integer, TAG_ANY) == NULL);

	fr_pair_list_free(&head);
}

static void test_fr_pair_replace(void)
{
	VALUE_PAIR	*head;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 999;

	fr_pair_replace(&head, vp);

	/* Replacing swaps the value in, so the count does not change */
	TEST_CHECK_LEN(list_len(head), 3);

	vp = fr_pair_find_by_da(head, da_integer, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK(vp->vp_integer == 999);

	fr_pair_list_free(&head);
}

/*
 *	Cursors
 */
static void test_cursor_iteration(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	TEST_CASE("fr_cursor_init returns the first pair");
	vp = fr_cursor_init(&cursor, &head);
	TEST_CHECK(vp == head);

	TEST_CASE("fr_cursor_current agrees");
	TEST_CHECK(fr_cursor_current(&cursor) == head);

	TEST_CASE("fr_cursor_next_peek does not advance");
	TEST_CHECK(fr_cursor_next_peek(&cursor) == head->next);
	TEST_CHECK(fr_cursor_current(&cursor) == head);

	TEST_CASE("fr_cursor_next walks the list");
	TEST_CHECK(fr_cursor_next(&cursor) == head->next);
	TEST_CHECK(fr_cursor_next(&cursor) == head->next->next);

	TEST_CASE("The end of the list is NULL");
	TEST_CHECK(fr_cursor_next(&cursor) == NULL);

	TEST_CASE("fr_cursor_first restarts");
	TEST_CHECK(fr_cursor_first(&cursor) == head);

	TEST_CASE("fr_cursor_last finds the tail");
	TEST_CHECK(fr_cursor_last(&cursor) == head->next->next);

	fr_pair_list_free(&head);
}

static void test_cursor_next_by_num(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	fr_cursor_init(&cursor, &head);

	vp = fr_cursor_next_by_num(&cursor, 1830, 0, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK(vp->da == da_octets);

	/* Only one such attribute, so a second search finds nothing */
	TEST_CHECK(fr_cursor_next_by_num(&cursor, 1830, 0, TAG_ANY) == NULL);

	fr_pair_list_free(&head);
}

static void test_cursor_next_by_da(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	fr_cursor_init(&cursor, &head);

	vp = fr_cursor_next_by_da(&cursor, da_integer, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK(vp->vp_integer == 12345);

	fr_pair_list_free(&head);
}

/** Several pairs of the same attribute
 *
 * fr_pair_find_by_da() only ever returns the first match, so walking a
 * cursor is the only way to reach the rest.
 */
static void test_cursor_next_by_da_repeated(void)
{
	VALUE_PAIR	*head = NULL;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;
	int		i;

	for (i = 0; i < 3; i++) {
		vp = fr_pair_afrom_da(autofree, da_string1);
		TEST_ASSERT(vp != NULL);
		fr_pair_value_sprintf(vp, "value-%i", i);
		fr_pair_add(&head, vp);
	}

	/* One pair of a different attribute, to prove the filter works */
	vp = fr_pair_afrom_da(autofree, da_integer1);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	TEST_CHECK_LEN(list_len(head), 4);

	TEST_CASE("The cursor reaches every matching pair in order");
	fr_cursor_init(&cursor, &head);
	for (i = 0; i < 3; i++) {
		char expected[32];

		vp = fr_cursor_next_by_da(&cursor, da_string1, TAG_ANY);
		TEST_CHECK(vp != NULL);
		if (!vp) break;

		snprintf(expected, sizeof(expected), "value-%i", i);
		TEST_CHECK_STRCMP(vp->vp_strvalue, expected);
	}

	TEST_CASE("And stops once they are exhausted");
	TEST_CHECK(fr_cursor_next_by_da(&cursor, da_string1, TAG_ANY) == NULL);

	TEST_CASE("fr_pair_find_by_da only ever returns the first");
	vp = fr_pair_find_by_da(head, da_string1, TAG_ANY);
	TEST_CHECK(vp != NULL);
	if (vp) TEST_CHECK_STRCMP(vp->vp_strvalue, "value-0");

	fr_pair_list_free(&head);
}

/** Deleting by attribute removes every instance, not just the first
 */
static void test_delete_by_num_repeated(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;
	int		i;

	for (i = 0; i < 3; i++) {
		vp = fr_pair_afrom_da(autofree, da_string1);
		TEST_ASSERT(vp != NULL);
		fr_pair_add(&head, vp);
	}

	vp = fr_pair_afrom_da(autofree, da_integer1);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	fr_pair_delete_by_num(&head, da_string1->attr, da_string1->vendor, TAG_ANY);

	TEST_CHECK_LEN(list_len(head), 1);
	TEST_CHECK(head->da == da_integer1);

	fr_pair_list_free(&head);
}

static void test_cursor_insert(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	fr_cursor_init(&cursor, &head);

	vp = fr_pair_afrom_da(autofree, da_integer64);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer64 = 1;

	fr_cursor_insert(&cursor, vp);

	TEST_CHECK_LEN(list_len(head), 4);
	TEST_CHECK(fr_pair_find_by_da(head, da_integer64, TAG_ANY) == vp);

	fr_pair_list_free(&head);
}

static void test_cursor_merge(void)
{
	VALUE_PAIR	*head, *other;
	vp_cursor_t	cursor;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	other = test_list_alloc(autofree);
	if (!TEST_CHECK(other != NULL)) return;

	fr_cursor_init(&cursor, &head);
	fr_cursor_merge(&cursor, other);

	/* Both lists are now one list */
	TEST_CHECK_LEN(list_len(head), 6);

	fr_pair_list_free(&head);
}

static void test_cursor_remove(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	fr_cursor_init(&cursor, &head);
	vp = fr_cursor_remove(&cursor);

	TEST_CHECK(vp != NULL);
	TEST_CHECK_LEN(list_len(head), 2);

	/* Removing hands ownership back to the caller rather than freeing */
	if (vp) {
		TEST_CHECK(vp->next == NULL);
		talloc_free(vp);
	}

	fr_pair_list_free(&head);
}

static void test_cursor_replace(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp, *old;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	vp = fr_pair_afrom_da(autofree, da_integer64);
	TEST_ASSERT(vp != NULL);

	fr_cursor_init(&cursor, &head);
	old = fr_cursor_replace(&cursor, vp);

	TEST_CHECK(old != NULL);
	TEST_CHECK_LEN(list_len(head), 3);
	TEST_CHECK(head == vp);

	if (old) talloc_free(old);

	fr_pair_list_free(&head);
}

static void test_cursor_copy(void)
{
	VALUE_PAIR	*head;
	vp_cursor_t	a, b;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	fr_cursor_init(&a, &head);
	fr_cursor_next(&a);

	fr_cursor_copy(&b, &a);

	/* The copy must be at the same position as the original */
	TEST_CHECK(fr_cursor_current(&b) == fr_cursor_current(&a));

	fr_pair_list_free(&head);
}

/*
 *	Copying
 */
static void test_fr_pair_copy(void)
{
	VALUE_PAIR	*vp, *copy;

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);
	fr_pair_value_strcpy(vp, test_string);

	copy = fr_pair_copy(autofree, vp);
	TEST_CHECK(copy != NULL);
	if (!copy) return;

	TEST_CHECK(copy != vp);
	TEST_CHECK(copy->da == vp->da);
	TEST_CHECK_STRCMP(copy->vp_strvalue, test_string);

	/* The string buffer must be copied too, not shared */
	TEST_CHECK(copy->vp_strvalue != vp->vp_strvalue);

	/* A copy is a single pair, detached from any list */
	TEST_CHECK(copy->next == NULL);

	talloc_free(vp);
	talloc_free(copy);
}

static void test_fr_pair_list_copy(void)
{
	VALUE_PAIR	*head, *copy;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	copy = fr_pair_list_copy(autofree, head);
	TEST_CHECK(copy != NULL);
	if (!copy) return;

	TEST_CHECK_LEN(list_len(copy), 3);
	TEST_CHECK(copy != head);

	/* Same contents, different pairs */
	TEST_CHECK(fr_pair_find_by_da(copy, da_string, TAG_ANY) != NULL);
	TEST_CHECK(fr_pair_find_by_da(copy, da_string, TAG_ANY) !=
		   fr_pair_find_by_da(head, da_string, TAG_ANY));

	fr_pair_list_free(&head);
	fr_pair_list_free(&copy);
}

static void test_fr_pair_list_copy_by_num(void)
{
	VALUE_PAIR	*head, *copy;

	head = test_list_alloc(autofree);
	TEST_ASSERT(head != NULL);

	copy = fr_pair_list_copy_by_num(autofree, head, 1810, 0, TAG_ANY);
	TEST_CHECK(copy != NULL);
	if (!copy) return;

	/* Only the matching attribute is copied */
	TEST_CHECK_LEN(list_len(copy), 1);
	TEST_CHECK(copy->da == da_integer);

	/* The source is left alone */
	TEST_CHECK_LEN(list_len(head), 3);

	fr_pair_list_free(&head);
	fr_pair_list_free(&copy);
}

static void test_fr_pair_steal(void)
{
	VALUE_PAIR	*vp;
	TALLOC_CTX	*ctx;

	ctx = talloc_init("pair_steal");
	TEST_ASSERT(ctx != NULL);

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK(talloc_parent(vp) == autofree);

	fr_pair_steal(ctx, vp);
	TEST_CHECK(talloc_parent(vp) == ctx);

	/* Freeing the new parent takes the pair with it */
	talloc_free(ctx);
}

/*
 *	Comparison
 */
static void test_fr_pair_cmp(void)
{
	VALUE_PAIR *a, *b;

	a = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(a != NULL);
	b = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(b != NULL);

	a->vp_integer = 1;
	b->vp_integer = 1;

	/*
	 *	The operator comes from "a", and the comparison reads
	 *	(b->data) (a->op) (a->data).
	 */
	a->op = T_OP_CMP_EQ;
	TEST_CHECK_RET(fr_pair_cmp(a, b), 1);

	b->vp_integer = 2;
	TEST_CHECK_RET(fr_pair_cmp(a, b), 0);

	a->op = T_OP_LT;
	/* Is b (2) < a (1)?  No. */
	TEST_CHECK_RET(fr_pair_cmp(a, b), 0);

	a->op = T_OP_GT;
	/* Is b (2) > a (1)?  Yes. */
	TEST_CHECK_RET(fr_pair_cmp(a, b), 1);

	talloc_free(a);
	talloc_free(b);
}

static void test_fr_pair_list_cmp(void)
{
	VALUE_PAIR *a, *b;

	a = test_list_alloc(autofree);
	b = test_list_alloc(autofree);
	if (!TEST_CHECK((a != NULL) && (b != NULL))) return;

	TEST_CASE("Identical lists compare equal");
	TEST_CHECK_RET(fr_pair_list_cmp(a, b), 0);

	TEST_CASE("Changing one value makes them differ");
	fr_pair_find_by_da(b, da_integer, TAG_ANY)->vp_integer = 999;
	TEST_CHECK(fr_pair_list_cmp(a, b) != 0);

	fr_pair_list_free(&a);
	fr_pair_list_free(&b);
}

static void test_fr_pair_cmp_by_da_tag(void)
{
	VALUE_PAIR *a, *b;

	a = fr_pair_afrom_da(autofree, da_string);	/* 1800 */
	b = fr_pair_afrom_da(autofree, da_integer);	/* 1810 */
	if (!TEST_CHECK((a != NULL) && (b != NULL))) return;

	/* Ordering is by attribute number, so 1800 sorts before 1810 */
	TEST_CHECK(fr_pair_cmp_by_da_tag(a, b) < 0);
	TEST_CHECK(fr_pair_cmp_by_da_tag(b, a) > 0);
	TEST_CHECK(fr_pair_cmp_by_da_tag(a, a) == 0);

	talloc_free(a);
	talloc_free(b);
}

static void test_fr_pair_list_sort(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;

	/* Build the list out of order: 1830, 1810, 1800 */
	vp = fr_pair_afrom_da(autofree, da_octets);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	vp = fr_pair_afrom_da(autofree, da_string);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	fr_pair_list_sort(&head, fr_pair_cmp_by_da_tag);

	TEST_CHECK_LEN(list_len(head), 3);
	TEST_CHECK(head->da == da_string);			/* 1800 */
	TEST_CHECK(head->next->da == da_integer);		/* 1810 */
	TEST_CHECK(head->next->next->da == da_octets);		/* 1830 */

	fr_pair_list_free(&head);
}

/*
 *	Validation
 */
static void test_fr_pair_validate(void)
{
	VALUE_PAIR	*filter = NULL, *list = NULL;
	VALUE_PAIR	*vp;
	VALUE_PAIR const *failed[2];

	TEST_CASE("Two empty lists validate");
	TEST_CHECK(fr_pair_validate(NULL, NULL, NULL) == true);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 12345;
	vp->op = T_OP_CMP_EQ;
	fr_pair_add(&filter, vp);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 12345;
	fr_pair_add(&list, vp);

	TEST_CASE("A list which matches the filter");
	TEST_CHECK(fr_pair_validate(failed, filter, list) == true);

	TEST_CASE("A list which does not match the filter");
	fr_pair_find_by_da(list, da_integer, TAG_ANY)->vp_integer = 999;
	TEST_CHECK(fr_pair_validate(failed, filter, list) == false);

	fr_pair_list_free(&filter);
	fr_pair_list_free(&list);
}

static void test_fr_pair_validate_relaxed(void)
{
	VALUE_PAIR	*filter = NULL, *list = NULL;
	VALUE_PAIR	*vp;
	VALUE_PAIR const *failed[2];

	/* The filter asks for one attribute */
	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 12345;
	vp->op = T_OP_CMP_EQ;
	fr_pair_add(&filter, vp);

	/* The list holds that attribute, plus others the filter ignores */
	list = test_list_alloc(autofree);
	if (!TEST_CHECK(list != NULL)) return;
	fr_pair_find_by_da(list, da_integer, TAG_ANY)->vp_integer = 12345;

	TEST_CASE("Relaxed validation ignores attributes not in the filter");
	TEST_CHECK(fr_pair_validate_relaxed(failed, filter, list) == true);

	fr_pair_list_free(&filter);
	fr_pair_list_free(&list);
}

/*
 *	Moving between lists
 */
static void test_fr_pair_list_move(void)
{
	VALUE_PAIR	*to = NULL, *from;

	from = test_list_alloc(autofree);
	if (!TEST_CHECK(from != NULL)) return;

	fr_pair_list_move(autofree, &to, &from, T_OP_ADD);

	TEST_CHECK_LEN(list_len(to), 3);

	/*
	 *	Anything which was moved is unlinked from the source, so
	 *	the caller frees whatever is left behind.
	 */
	fr_pair_list_free(&from);
	fr_pair_list_free(&to);
}

static void test_fr_pair_list_move_by_num(void)
{
	VALUE_PAIR	*to = NULL, *from;

	from = test_list_alloc(autofree);
	if (!TEST_CHECK(from != NULL)) return;

	fr_pair_list_move_by_num(autofree, &to, &from, 1810, 0, TAG_ANY);

	TEST_CASE("Only the named attribute moves");
	TEST_CHECK_LEN(list_len(to), 1);
	TEST_CHECK(to->da == da_integer);

	TEST_CASE("The rest stays behind");
	TEST_CHECK_LEN(list_len(from), 2);
	TEST_CHECK(fr_pair_find_by_da(from, da_integer, TAG_ANY) == NULL);

	fr_pair_list_free(&from);
	fr_pair_list_free(&to);
}

static void test_fr_pair_list_mcopy_by_num(void)
{
	VALUE_PAIR	*to = NULL, *from;

	from = test_list_alloc(autofree);
	if (!TEST_CHECK(from != NULL)) return;

	fr_pair_list_mcopy_by_num(autofree, &to, &from, 1810, 0, TAG_ANY);

	TEST_CHECK_LEN(list_len(to), 1);
	TEST_CHECK(to->da == da_integer);

	fr_pair_list_free(&from);
	fr_pair_list_free(&to);
}

/*
 *	Parsing from text
 */
static void test_fr_pair_make(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;

	vp = fr_pair_make(autofree, &head, "Tmp-Integer-0", "12345", T_OP_EQ);
	TEST_CHECK(vp != NULL);
	if (!vp) return;

	TEST_CHECK(vp->da == da_integer);
	TEST_CHECK(vp->vp_integer == 12345);

	/* fr_pair_make also adds to the list it was given */
	TEST_CHECK(head == vp);

	TEST_CASE("An attribute which is not in the dictionary");
	TEST_CHECK(fr_pair_make(autofree, &head, "Not-An-Attribute", "x", T_OP_EQ) == NULL);

	fr_pair_list_free(&head);
}

static void test_fr_pair_list_afrom_str(void)
{
	VALUE_PAIR	*head = NULL;
	FR_TOKEN	token;

	token = fr_pair_list_afrom_str(autofree, "Tmp-Integer-0 = 42, Tmp-String-0 = \"hello\"", &head);

	TEST_CHECK(token != T_INVALID);
	TEST_CHECK(head != NULL);
	if (!head) return;

	TEST_CHECK_LEN(list_len(head), 2);
	TEST_CHECK(fr_pair_find_by_da(head, da_integer, TAG_ANY) != NULL);
	if (fr_pair_find_by_da(head, da_integer, TAG_ANY)) {
		TEST_CHECK(fr_pair_find_by_da(head, da_integer, TAG_ANY)->vp_integer == 42);
	}
	if (fr_pair_find_by_da(head, da_string, TAG_ANY)) {
		TEST_CHECK_STRCMP(fr_pair_find_by_da(head, da_string, TAG_ANY)->vp_strvalue, "hello");
	}

	fr_pair_list_free(&head);
}

static void test_fr_pair_raw_from_str(void)
{
	VALUE_PAIR_RAW	raw;
	char const	*ptr = "Tmp-String-0 = \"hello\"";
	FR_TOKEN	token;

	token = fr_pair_raw_from_str(&ptr, &raw);

	TEST_CHECK(token != T_INVALID);
	TEST_CHECK_STRCMP(raw.l_opand, "Tmp-String-0");
	TEST_CHECK_STRCMP(raw.r_opand, "hello");
	TEST_CHECK(raw.op == T_OP_EQ);
}

/*
 *	fr_pair_afrom_ip_str()
 *
 *	Picks a dictionary attribute based on the shape of the string: a
 *	colon means IPv6, a slash means a prefix.
 *
 *	Note the API takes non-const DICT_ATTR *, while dict_attrbyname()
 *	returns const.  The casts below work around that; the function does
 *	not modify the attributes, so the parameters ought to be const.
 */
static void test_fr_pair_afrom_ip_str(void)
{
	VALUE_PAIR *vp;

#define IP_STR(_val) fr_pair_afrom_ip_str(autofree, _val, \
					  da_ipaddr, da_ipv6, \
					  da_ipv4prefix, da_ipv6prefix)

	TEST_CASE("A bare IPv4 address");
	vp = IP_STR("192.0.2.1");
	TEST_CHECK(vp != NULL);
	if (vp) {
		TEST_CHECK(vp->da == da_ipaddr);
		TEST_CHECK(vp->vp_ipaddr == htonl(0xc0000201));
		talloc_free(vp);
	}

	TEST_CASE("A slash selects the IPv4 prefix attribute");
	vp = IP_STR("10.0.0.0/8");
	TEST_CHECK(vp != NULL);
	if (vp) {
		TEST_CHECK(vp->da == da_ipv4prefix);
		talloc_free(vp);
	}

	TEST_CASE("A colon selects IPv6");
	vp = IP_STR("2001:db8::1");
	TEST_CHECK(vp != NULL);
	if (vp) {
		TEST_CHECK(vp->da == da_ipv6);
		talloc_free(vp);
	}

	TEST_CASE("A colon and a slash select the IPv6 prefix");
	vp = IP_STR("2001:db8::/32");
	TEST_CHECK(vp != NULL);
	if (vp) {
		TEST_CHECK(vp->da == da_ipv6prefix);
		talloc_free(vp);
	}

	TEST_CASE("A string which is not an address at all");
	TEST_CHECK(IP_STR("not-an-address") == NULL);
#undef IP_STR
}

/** Only offering some of the attributes still works for those shapes
 *
 * Passing none of them trips an assertion, so that case is deliberately not
 * exercised here.
 */
static void test_fr_pair_afrom_ip_str_partial(void)
{
	VALUE_PAIR *vp;

	TEST_CASE("An IPv4 address with only the IPv4 attribute given");
	vp = fr_pair_afrom_ip_str(autofree, "192.0.2.1", da_ipaddr, NULL, NULL, NULL);
	TEST_CHECK(vp != NULL);
	if (vp) {
		TEST_CHECK(vp->da == da_ipaddr);
		talloc_free(vp);
	}

	TEST_CASE("An IPv6 address when no IPv6 attribute was given");
	TEST_CHECK(fr_pair_afrom_ip_str(autofree, "2001:db8::1",
					da_ipaddr, NULL, NULL, NULL) == NULL);
}

/*
 *	fr_pair_list_afrom_file()
 */
static void test_fr_pair_list_afrom_file(void)
{
	VALUE_PAIR	*head = NULL;
	FILE		*fp;
	bool		done = true;
	char const	*path = "build/tests/api/pair_tests_input.txt";

	fp = fopen(path, "w");
	if (!fp) {
		TEST_MSG("Could not create %s - %s", path, strerror(errno));
		return;
	}
	fprintf(fp, "Tmp-Integer-0 = 1\n");
	fprintf(fp, "Tmp-String-0 = \"first\"\n");
	fprintf(fp, "\n");
	fprintf(fp, "Tmp-Integer-1 = 2\n");
	fclose(fp);

	fp = fopen(path, "r");
	if (!fp) {
		TEST_MSG("Could not open %s - %s", path, strerror(errno));
		return;
	}

	TEST_CASE("A blank line ends the entry, and the file is not yet finished");
	TEST_CHECK_RET(fr_pair_list_afrom_file(autofree, &head, fp, &done), 0);
	TEST_CHECK(done == false);
	TEST_CHECK_LEN(list_len(head), 2);

	if (head) {
		TEST_CHECK(fr_pair_find_by_da(head, da_integer, TAG_ANY) != NULL);
		if (fr_pair_find_by_da(head, da_string, TAG_ANY)) {
			TEST_CHECK_STRCMP(fr_pair_find_by_da(head, da_string, TAG_ANY)->vp_strvalue,
					  "first");
		}
	}
	fr_pair_list_free(&head);

	TEST_CASE("Reading again picks up the next entry, and reaches EOF");
	TEST_CHECK_RET(fr_pair_list_afrom_file(autofree, &head, fp, &done), 0);
	TEST_CHECK(done == true);
	TEST_CHECK_LEN(list_len(head), 1);
	if (head) TEST_CHECK(head->da == da_integer1);

	fr_pair_list_free(&head);
	fclose(fp);
	unlink(path);
}

/*
 *	fr_pair_validate_debug()
 */
static void test_fr_pair_validate_debug(void)
{
	VALUE_PAIR	*filter, *list;
	VALUE_PAIR const *failed[2];

	filter = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(filter != NULL);
	list   = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(list != NULL);

	filter->vp_integer = 1;
	filter->op = T_OP_CMP_EQ;
	list->vp_integer = 2;

	TEST_CASE("A value mismatch names both sides");
	failed[0] = filter;
	failed[1] = list;
	fr_pair_validate_debug(autofree, failed);
	TEST_CHECK_STRCMP(fr_strerror(), "Attribute value '2' didn't match filter: Tmp-Integer-0 == 1");

	TEST_CASE("An attribute missing from the list");
	failed[0] = filter;
	failed[1] = NULL;
	fr_pair_validate_debug(autofree, failed);
	TEST_CHECK_STRCMP(fr_strerror(), "Attribute 'Tmp-Integer-0' not found in list");

	TEST_CASE("An attribute missing from the filter");
	failed[0] = NULL;
	failed[1] = list;
	fr_pair_validate_debug(autofree, failed);
	TEST_CHECK_STRCMP(fr_strerror(), "Attribute 'Tmp-Integer-0' not found in filter");

	talloc_free(filter);
	talloc_free(list);
}

/** fr_pair_validate() fills in the failed[] array
 */
static void test_fr_pair_validate_failed_pairs(void)
{
	VALUE_PAIR	*filter = NULL, *list = NULL;
	VALUE_PAIR	*vp;
	VALUE_PAIR const *failed[2] = { NULL, NULL };

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 1;
	vp->op = T_OP_CMP_EQ;
	fr_pair_add(&filter, vp);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 999;
	fr_pair_add(&list, vp);

	TEST_CHECK(fr_pair_validate(failed, filter, list) == false);

	TEST_CASE("Both offending pairs are reported");
	TEST_CHECK(failed[0] != NULL);
	TEST_CHECK(failed[1] != NULL);
	if (failed[0]) TEST_CHECK(failed[0]->da == da_integer);
	if (failed[1]) TEST_CHECK(failed[1]->vp_integer == 999);

	fr_pair_list_free(&filter);
	fr_pair_list_free(&list);
}

/*
 *	fr_pair_cmp_op()
 */
static void test_fr_pair_cmp_op(void)
{
	VALUE_PAIR *a, *b;

	a = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(a != NULL);
	b = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(b != NULL);

	a->vp_integer = 1;
	b->vp_integer = 2;

	/*
	 *	Note the argument order.  fr_pair_cmp_op(op, a, b) evaluates
	 *	"a op b", which is the opposite way round from fr_pair_cmp(),
	 *	where the operator comes from "a" and the comparison is
	 *	"b op a".
	 */
	TEST_CASE("a < b");
	TEST_CHECK_RET(fr_pair_cmp_op(T_OP_LT, a, b), 1);
	TEST_CHECK_RET(fr_pair_cmp_op(T_OP_GT, a, b), 0);

	TEST_CASE("and the reverse");
	TEST_CHECK_RET(fr_pair_cmp_op(T_OP_GT, b, a), 1);

	TEST_CASE("equality");
	TEST_CHECK_RET(fr_pair_cmp_op(T_OP_CMP_EQ, a, b), 0);
	b->vp_integer = 1;
	TEST_CHECK_RET(fr_pair_cmp_op(T_OP_CMP_EQ, a, b), 1);

	talloc_free(a);
	talloc_free(b);
}

/*
 *	Gaps in the existing coverage.
 */
static void test_fr_pair_to_unknown_idempotent(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK_RET(fr_pair_to_unknown(vp), 0);
	TEST_CHECK(vp->da->flags.is_unknown);

	TEST_CASE("Converting an already-unknown pair is a no-op, not an error");
	TEST_CHECK_RET(fr_pair_to_unknown(vp), 0);
	TEST_CHECK(vp->da->flags.is_unknown);

	talloc_free(vp);
}

static void test_fr_pair_list_move_replace(void)
{
	VALUE_PAIR	*to = NULL, *from = NULL;
	VALUE_PAIR	*vp;

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 1;
	fr_pair_add(&to, vp);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);
	vp->vp_integer = 2;
	vp->op = T_OP_SET;
	fr_pair_add(&from, vp);

	TEST_CASE("T_OP_SET replaces rather than appends");
	fr_pair_list_move(autofree, &to, &from, T_OP_SET);

	TEST_CHECK_LEN(list_len(to), 1);
	if (to) TEST_CHECK(to->vp_integer == 2);

	fr_pair_list_free(&from);
	fr_pair_list_free(&to);
}

static void test_delete_by_da_repeated(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;
	int		i;

	for (i = 0; i < 3; i++) {
		vp = fr_pair_afrom_da(autofree, da_string1);
		TEST_ASSERT(vp != NULL);
		fr_pair_add(&head, vp);
	}

	vp = fr_pair_afrom_da(autofree, da_integer1);
	TEST_ASSERT(vp != NULL);
	fr_pair_add(&head, vp);

	fr_pair_delete_by_da(&head, da_string1);

	TEST_CHECK_LEN(list_len(head), 1);
	TEST_CHECK(head->da == da_integer1);

	fr_pair_list_free(&head);
}

static void test_cursor_on_empty_list(void)
{
	VALUE_PAIR	*head = NULL;
	vp_cursor_t	cursor;

	TEST_CASE("A cursor over an empty list yields nothing");
	TEST_CHECK(fr_cursor_init(&cursor, &head) == NULL);
	TEST_CHECK(fr_cursor_current(&cursor) == NULL);
	TEST_CHECK(fr_cursor_next(&cursor) == NULL);
	TEST_CHECK(fr_cursor_first(&cursor) == NULL);
	TEST_CHECK(fr_cursor_last(&cursor) == NULL);
	TEST_CHECK(fr_cursor_next_peek(&cursor) == NULL);
}

static void test_cursor_insert_into_empty(void)
{
	VALUE_PAIR	*head = NULL;
	vp_cursor_t	cursor;
	VALUE_PAIR	*vp;

	fr_cursor_init(&cursor, &head);

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);

	TEST_CASE("Inserting into an empty list sets the head");
	fr_cursor_insert(&cursor, vp);
	TEST_CHECK(head == vp);
	TEST_CHECK_LEN(list_len(head), 1);

	fr_pair_list_free(&head);
}

static void test_fr_pair_value_from_str_bad(void)
{
	VALUE_PAIR *vp;

	vp = fr_pair_afrom_da(autofree, da_integer);
	TEST_ASSERT(vp != NULL);

	TEST_CASE("A value which does not parse is an error");
	TEST_CHECK(fr_pair_value_from_str(vp, "not-a-number", -1) < 0);

	talloc_free(vp);
}

static void test_fr_pair_make_tagged(void)
{
	VALUE_PAIR	*head = NULL;
	VALUE_PAIR	*vp;

	TEST_CASE("A tag in the attribute name is parsed out");
	vp = fr_pair_make(autofree, &head, "Tunnel-Type:2", "VLAN", T_OP_EQ);
	TEST_ASSERT(vp != NULL);

	TEST_CHECK(vp->da == da_tagged);
	TEST_CHECK(vp->tag == 2);
	TEST_MSG("Expected tag 2, got %i", vp->tag);

	fr_pair_list_free(&head);
}

TEST_LIST = {
	/*
	 *	Allocation
	 */
	{ "fr_pair_alloc",			test_fr_pair_alloc },
	{ "fr_pair_afrom_da",			test_fr_pair_afrom_da },
	{ "fr_pair_afrom_num",			test_fr_pair_afrom_num },
	{ "fr_pair_afrom_num_unknown",		test_fr_pair_afrom_num_unknown },
	{ "fr_pair_to_unknown",			test_fr_pair_to_unknown },
	{ "fr_pair_list_free",			test_fr_pair_list_free },

	/*
	 *	Setting values
	 */
	{ "fr_pair_value_strcpy",		test_fr_pair_value_strcpy },
	{ "fr_pair_value_bstrncpy",		test_fr_pair_value_bstrncpy },
	{ "fr_pair_value_strsteal",		test_fr_pair_value_strsteal },
	{ "fr_pair_value_sprintf",		test_fr_pair_value_sprintf },
	{ "fr_pair_value_memcpy",		test_fr_pair_value_memcpy },
	{ "fr_pair_value_memsteal",		test_fr_pair_value_memsteal },
	{ "fr_pair_value_from_str",		test_fr_pair_value_from_str },
	{ "fr_pair_mark_xlat",			test_fr_pair_mark_xlat },

	/*
	 *	Adding to and searching lists
	 */
	{ "fr_pair_add",			test_fr_pair_add },
	{ "fr_pair_prepend",			test_fr_pair_prepend },
	{ "fr_pair_find_by_num",		test_fr_pair_find_by_num },
	{ "fr_pair_find_by_da",			test_fr_pair_find_by_da },
	{ "find_by_num_tagged",			test_find_by_num_tagged },

	/*
	 *	Deleting from lists
	 */
	{ "fr_pair_delete_by_num",		test_fr_pair_delete_by_num },
	{ "fr_pair_delete_by_da",		test_fr_pair_delete_by_da },
	{ "fr_pair_delete",			test_fr_pair_delete },
	{ "fr_pair_replace",			test_fr_pair_replace },

	/*
	 *	Cursors
	 */
	{ "cursor_iteration",			test_cursor_iteration },
	{ "cursor_next_by_num",			test_cursor_next_by_num },
	{ "cursor_next_by_da",			test_cursor_next_by_da },
	{ "cursor_next_by_da_repeated",		test_cursor_next_by_da_repeated },
	{ "delete_by_num_repeated",		test_delete_by_num_repeated },
	{ "cursor_insert",			test_cursor_insert },
	{ "cursor_merge",			test_cursor_merge },
	{ "cursor_remove",			test_cursor_remove },
	{ "cursor_replace",			test_cursor_replace },
	{ "cursor_copy",			test_cursor_copy },

	/*
	 *	Copying
	 */
	{ "fr_pair_copy",			test_fr_pair_copy },
	{ "fr_pair_list_copy",			test_fr_pair_list_copy },
	{ "fr_pair_list_copy_by_num",		test_fr_pair_list_copy_by_num },
	{ "fr_pair_steal",			test_fr_pair_steal },

	/*
	 *	Comparison and sorting
	 */
	{ "fr_pair_cmp",			test_fr_pair_cmp },
	{ "fr_pair_list_cmp",			test_fr_pair_list_cmp },
	{ "fr_pair_cmp_by_da_tag",		test_fr_pair_cmp_by_da_tag },
	{ "fr_pair_list_sort",			test_fr_pair_list_sort },

	/*
	 *	Validation
	 */
	{ "fr_pair_validate",			test_fr_pair_validate },
	{ "fr_pair_validate_relaxed",		test_fr_pair_validate_relaxed },

	/*
	 *	Moving between lists
	 */
	{ "fr_pair_list_move",			test_fr_pair_list_move },
	{ "fr_pair_list_move_by_num",		test_fr_pair_list_move_by_num },
	{ "fr_pair_list_mcopy_by_num",		test_fr_pair_list_mcopy_by_num },

	/*
	 *	Parsing from text
	 */
	{ "fr_pair_make",			test_fr_pair_make },
	{ "fr_pair_list_afrom_str",		test_fr_pair_list_afrom_str },
	{ "fr_pair_raw_from_str",		test_fr_pair_raw_from_str },

	/*
	 *	Address parsing
	 */
	{ "fr_pair_afrom_ip_str",		test_fr_pair_afrom_ip_str },
	{ "fr_pair_afrom_ip_str_partial",	test_fr_pair_afrom_ip_str_partial },

	/*
	 *	Reading from a file
	 */
	{ "fr_pair_list_afrom_file",		test_fr_pair_list_afrom_file },

	/*
	 *	Validation reporting
	 */
	{ "fr_pair_validate_debug",		test_fr_pair_validate_debug },
	{ "fr_pair_validate_failed_pairs",	test_fr_pair_validate_failed_pairs },

	/*
	 *	Comparison macro
	 */
	{ "fr_pair_cmp_op",			test_fr_pair_cmp_op },

	/*
	 *	Edge cases
	 */
	{ "fr_pair_to_unknown_idempotent",	test_fr_pair_to_unknown_idempotent },
	{ "fr_pair_list_move_replace",		test_fr_pair_list_move_replace },
	{ "delete_by_da_repeated",		test_delete_by_da_repeated },
	{ "cursor_on_empty_list",		test_cursor_on_empty_list },
	{ "cursor_insert_into_empty",		test_cursor_insert_into_empty },
	{ "fr_pair_value_from_str_bad",		test_fr_pair_value_from_str_bad },
	{ "fr_pair_make_tagged",		test_fr_pair_make_tagged },

	TEST_TERMINATOR
};
