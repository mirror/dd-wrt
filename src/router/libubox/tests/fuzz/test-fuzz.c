#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#include "blob.h"
#include "blobmsg.h"

#define BLOBMSG_TYPE_TROUBLE INT_MAX

static void fuzz_blobmsg_parse(const uint8_t *data, size_t size)
{
	enum {
		FOO_MESSAGE,
		FOO_LIST,
		FOO_TESTDATA,
		__FOO_MAX
	};

	static const int blobmsg_type[] = {
		BLOBMSG_TYPE_UNSPEC,
		BLOBMSG_TYPE_ARRAY,
		BLOBMSG_TYPE_TABLE,
		BLOBMSG_TYPE_STRING,
		BLOBMSG_TYPE_INT64,
		BLOBMSG_TYPE_INT32,
		BLOBMSG_TYPE_INT16,
		BLOBMSG_TYPE_INT8,
		BLOBMSG_TYPE_DOUBLE,
		BLOBMSG_TYPE_TROUBLE,
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

	struct blob_attr *tb[__FOO_MAX];

	blobmsg_parse(foo_policy, __FOO_MAX, tb, (uint8_t *)data, size);
	blobmsg_parse_array(foo_policy, __FOO_MAX, tb, (uint8_t *)data, size);

	blobmsg_check_attr_len((struct blob_attr *)data, false, size);
	blobmsg_check_attr_len((struct blob_attr *)data, true, size);

	for (size_t i=0; i < ARRAY_SIZE(blobmsg_type); i++) {
		blobmsg_check_array_len((struct blob_attr *)data, blobmsg_type[i], size);
		blobmsg_check_attr_list_len((struct blob_attr *)data, blobmsg_type[i], size);
	}
}

static void fuzz_blob_parse(const uint8_t *data, size_t size)
{
	enum {
		FOO_ATTR_NESTED,
		FOO_ATTR_BINARY,
		FOO_ATTR_STRING,
		FOO_ATTR_INT8,
		FOO_ATTR_INT16,
		FOO_ATTR_INT32,
		FOO_ATTR_INT64,
		FOO_ATTR_DOUBLE,
		__FOO_ATTR_MAX
	};


	static const struct blob_attr_info foo_policy[__FOO_ATTR_MAX] = {
		[FOO_ATTR_NESTED] = { .type = BLOB_ATTR_NESTED },
		[FOO_ATTR_BINARY] = { .type = BLOB_ATTR_BINARY },
		[FOO_ATTR_STRING] = { .type = BLOB_ATTR_STRING },
		[FOO_ATTR_INT8] = { .type = BLOB_ATTR_INT8 },
		[FOO_ATTR_INT16] = { .type = BLOB_ATTR_INT16 },
		[FOO_ATTR_INT32] = { .type = BLOB_ATTR_INT32 },
		[FOO_ATTR_INT64] = { .type = BLOB_ATTR_INT64 },
		[FOO_ATTR_DOUBLE] = { .type = BLOB_ATTR_DOUBLE },
	};

	struct blob_attr *foo[__FOO_ATTR_MAX];
	struct blob_attr *buf = (struct blob_attr *)data;

	blob_parse_untrusted(buf, size, foo, foo_policy, __FOO_ATTR_MAX);
}

int LLVMFuzzerTestOneInput(const uint8_t *input, size_t size)
{
	uint8_t *data;

	data = malloc(size);
	if (!data)
		return -1;

	memcpy(data, input, size);
	fuzz_blob_parse(data, size);
	fuzz_blobmsg_parse(data, size);
	free(data);

	return 0;
}
