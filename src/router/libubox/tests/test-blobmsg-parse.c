#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <libgen.h>

#include "blobmsg.h"

enum {
	FOO_MESSAGE,
	FOO_LIST,
	FOO_TESTDATA,
	__FOO_MAX
};

static const struct blobmsg_policy foo_policy[] = {
	[FOO_MESSAGE] = {
		.name = "message",
		.type = BLOBMSG_TYPE_STRING,
	},
	[FOO_LIST] = {
		.name = "list",
		.type = BLOBMSG_TYPE_ARRAY,
	},
	[FOO_TESTDATA] = {
		.name = "testdata",
		.type = BLOBMSG_TYPE_TABLE,
	},
};

static void dump_result(const char *fn, int r, const char *filename, struct blob_attr **tb)
{
	fprintf(stdout, "%s: %s: %c%c%c (%d)\n", basename((char *) filename), fn,
		tb[FOO_MESSAGE] ? 'M' : '.',
		tb[FOO_LIST] ? 'L' : '.',
		tb[FOO_TESTDATA] ? 'T' : '.',
		r);
}

static void test_blobmsg(const char *filename)
{
#define BUF_LEN 256
	int r = 0;
	size_t len = 0;
	FILE *fd = NULL;
	char *buf = NULL;
	struct blob_attr *tb[__FOO_MAX];

	fd = fopen(filename, "r");
	if (!fd) {
		fprintf(stderr, "unable to open %s\n", filename);
		return;
	}

	buf = malloc(BUF_LEN+1);
	if (!buf)
		return;

	len = fread(buf, 1, BUF_LEN, fd);
	fclose(fd);

	r = blobmsg_parse(foo_policy, ARRAY_SIZE(foo_policy), tb, buf, len);
	dump_result("blobmsg_parse", r, filename, tb);

	r = blobmsg_parse_array(foo_policy, ARRAY_SIZE(foo_policy), tb, buf, len);
	dump_result("blobmsg_parse_array", r, filename, tb);

	free(buf);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <blobmsg.bin>\n", argv[0]);
		return 3;
	}

	test_blobmsg(argv[1]);

	return 0;
}
