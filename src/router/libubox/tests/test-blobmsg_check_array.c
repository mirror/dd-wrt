#include <stdio.h>

#include "blobmsg.h"

/*
 * This test tests a blob of this form...
 *
	{
		"array_a" : [
			{
				"array_b": [
					"1"
				]
			}
		]
	}
 *
 */


enum {
	ARRAY_A = 0,
	ARRAY_B = 0,
};

static char const array_a[] = "array_a";
static char const array_b[] = "array_b";

static const struct blobmsg_policy pol_a[] = {
	[ARRAY_A] = {
		.name = array_a,
		.type = BLOBMSG_TYPE_ARRAY
	}
};

static const struct blobmsg_policy pol_b[] = {
	[ARRAY_B] = {
		.name = array_b,
		.type = BLOBMSG_TYPE_ARRAY
	}
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static int
check_table_a_entries(struct blob_attr *attr)
{
	struct blob_attr *cur;
	size_t rem;
	int entry_number = 0;

	blobmsg_for_each_attr(cur, attr, rem) {
		int failed = 0;

		fprintf(stderr, "Process %s: entry %d\n", array_a, entry_number);

		struct blob_attr *tb[ARRAY_SIZE(pol_b)];

		if (blobmsg_parse(pol_b, ARRAY_SIZE(pol_b), tb,
						  blobmsg_data(cur), blobmsg_data_len(cur)) != 0) {
			fprintf(stderr, "Policy %s parse failed\n", array_b);
			return -1;
		}

		if (tb[ARRAY_B] == NULL) {
			fprintf(stderr, "%s not found\n", array_b);
			return -1;
		}

		/*
		 * This is the test that fails when blobmsg_check_array() passes the
		 * length obtained by blob_len(attr).
		 * It succeeds when blobmsg_check_array() uses blob_len(attr), which is
		 * equivalent to the origianl code, pre the length check changes.
		 */
		if (blobmsg_check_array(tb[ARRAY_B], BLOBMSG_TYPE_STRING) < 0) {
			fprintf(stderr, "Failed blobmsg_check_array() (STRING) on %s\n",
						array_b);
			failed = 1;
		}

		/*
		 * Continue outputting the strings even though the test above might
		 * have failed.
		 * This will show that the array does actually contain the expected
		 * string.
		 */

		struct blob_attr *cur2;
		size_t rem2;

		blobmsg_for_each_attr(cur2, tb[ARRAY_B], rem2) {
			fprintf(stderr, "%s contains string: %s\n",
							array_b, blobmsg_get_string(cur2));
		}


		entry_number++;

		if (failed)
			return -1;
	}

	return 0;
}

static int
check_message(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol_a)];

	if (blobmsg_parse(pol_a, ARRAY_SIZE(pol_a), tb,
					  blob_data(buf->head), blobmsg_data_len(buf->head)) != 0) {
		fprintf(stderr, "Policy %s parse failed\n", array_a);
		return -1;
	}

	if (tb[ARRAY_A] == NULL) {
		fprintf(stderr, "%s not found\n", array_a);
		return -1;
	}

	int const result = blobmsg_check_array(tb[ARRAY_A], BLOBMSG_TYPE_TABLE);

	if (result < 0) {
		fprintf(stderr, "Failed blobmsg_check_array() (TABLE) on %s (%d)\n",
				array_a, result);
		return -1;
	}

	return check_table_a_entries(tb[ARRAY_A]);
}

static void
fill_message(struct blob_buf * const buf)
{
	void * const tbl_a = blobmsg_open_array(buf, "array_a");
	void * const obj = blobmsg_open_table(buf, NULL);

	void * const tbl_b = blobmsg_open_array(buf, "array_b");

	blobmsg_add_string(buf, NULL, "1");

	blobmsg_close_array(buf, tbl_b);

	blobmsg_close_table(buf, obj);

	blobmsg_close_array(buf, tbl_a);
}

int main(int argc, char **argv)
{
	int result;
	static struct blob_buf buf;

	blobmsg_buf_init(&buf);
	fill_message(&buf);

	result = check_message(&buf);
	if (result == 0)
		fprintf(stderr, "blobmsg_check_array() test passed\n");

	if (buf.buf != NULL)
		free(buf.buf);

	return result ? EXIT_FAILURE : EXIT_SUCCESS;
}
