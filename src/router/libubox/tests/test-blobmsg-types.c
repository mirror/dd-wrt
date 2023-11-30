#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>

#include "blobmsg.h"
#include "blobmsg_json.h"

enum {
	FOO_STRING,
	FOO_INT64_MAX,
	FOO_INT64_MIN,
	FOO_INT32_MAX,
	FOO_INT32_MIN,
	FOO_INT16_MAX,
	FOO_INT16_MIN,
	FOO_INT8_MAX,
	FOO_INT8_MIN,
	FOO_DOUBLE_MAX,
	FOO_DOUBLE_MIN,
	__FOO_MAX
};

static const struct blobmsg_policy pol[] = {
	[FOO_STRING] = {
		.name = "string",
		.type = BLOBMSG_TYPE_STRING,
	},
	[FOO_INT64_MAX] = {
		.name = "int64_max",
		.type = BLOBMSG_TYPE_INT64,
	},
	[FOO_INT64_MIN] = {
		.name = "int64_min",
		.type = BLOBMSG_TYPE_INT64,
	},
	[FOO_INT32_MAX] = {
		.name = "int32_max",
		.type = BLOBMSG_TYPE_INT32,
	},
	[FOO_INT32_MIN] = {
		.name = "int32_min",
		.type = BLOBMSG_TYPE_INT32,
	},
	[FOO_INT16_MAX] = {
		.name = "int16_max",
		.type = BLOBMSG_TYPE_INT16,
	},
	[FOO_INT16_MIN] = {
		.name = "int16_min",
		.type = BLOBMSG_TYPE_INT16,
	},
	[FOO_INT8_MAX] = {
		.name = "int8_max",
		.type = BLOBMSG_TYPE_INT8,
	},
	[FOO_INT8_MIN] = {
		.name = "int8_min",
		.type = BLOBMSG_TYPE_INT8,
	},
	[FOO_DOUBLE_MAX] = {
		.name = "double_max",
		.type = BLOBMSG_TYPE_DOUBLE,
	},
	[FOO_DOUBLE_MIN] = {
		.name = "double_min",
		.type = BLOBMSG_TYPE_DOUBLE,
	},
};

static const struct blobmsg_policy pol_json[] = {
	[FOO_STRING] = {
		.name = "string",
		.type = BLOBMSG_TYPE_STRING,
	},
	[FOO_INT64_MAX] = {
		.name = "int64_max",
		.type = BLOBMSG_TYPE_INT64,
	},
	[FOO_INT64_MIN] = {
		.name = "int64_min",
		.type = BLOBMSG_TYPE_INT64,
	},
	[FOO_INT32_MAX] = {
		.name = "int32_max",
		.type = BLOBMSG_TYPE_INT32,
	},
	[FOO_INT32_MIN] = {
		.name = "int32_min",
		.type = BLOBMSG_TYPE_INT32,
	},
	[FOO_INT16_MAX] = {
		.name = "int16_max",
		.type = BLOBMSG_TYPE_INT32,
	},
	[FOO_INT16_MIN] = {
		.name = "int16_min",
		.type = BLOBMSG_TYPE_INT32,
	},
	[FOO_INT8_MAX] = {
		.name = "int8_max",
		.type = BLOBMSG_TYPE_INT8,
	},
	[FOO_INT8_MIN] = {
		.name = "int8_min",
		.type = BLOBMSG_TYPE_INT8,
	},
	[FOO_DOUBLE_MAX] = {
		.name = "double_max",
		.type = BLOBMSG_TYPE_DOUBLE,
	},
	[FOO_DOUBLE_MIN] = {
		.name = "double_min",
		.type = BLOBMSG_TYPE_DOUBLE,
	},
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static void dump_message(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol, ARRAY_SIZE(pol), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_STRING])
		fprintf(stderr, "string: %s\n", blobmsg_get_string(tb[FOO_STRING]));
	if (tb[FOO_INT64_MAX])
		fprintf(stderr, "int64_max: %" PRId64 "\n", (int64_t)blobmsg_get_u64(tb[FOO_INT64_MAX]));
	if (tb[FOO_INT64_MIN])
		fprintf(stderr, "int64_min: %" PRId64 "\n", (int64_t)blobmsg_get_u64(tb[FOO_INT64_MIN]));
	if (tb[FOO_INT32_MAX])
		fprintf(stderr, "int32_max: %" PRId32 "\n", (int32_t)blobmsg_get_u32(tb[FOO_INT32_MAX]));
	if (tb[FOO_INT32_MIN])
		fprintf(stderr, "int32_min: %" PRId32 "\n", (int32_t)blobmsg_get_u32(tb[FOO_INT32_MIN]));
	if (tb[FOO_INT16_MAX])
		fprintf(stderr, "int16_max: %" PRId16 "\n", (int16_t)blobmsg_get_u16(tb[FOO_INT16_MAX]));
	if (tb[FOO_INT16_MIN])
		fprintf(stderr, "int16_min: %" PRId16 "\n", (int16_t)blobmsg_get_u16(tb[FOO_INT16_MIN]));
	if (tb[FOO_INT8_MAX])
		fprintf(stderr, "int8_max: %" PRId8 "\n", (int8_t)blobmsg_get_u8(tb[FOO_INT8_MAX]));
	if (tb[FOO_INT8_MIN])
		fprintf(stderr, "int8_min: %" PRId8 "\n", (int8_t)blobmsg_get_u8(tb[FOO_INT8_MIN]));
	if (tb[FOO_DOUBLE_MAX])
		fprintf(stderr, "double_max: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MAX]));
	if (tb[FOO_DOUBLE_MIN])
		fprintf(stderr, "double_min: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MIN]));
}

static void dump_message_cast_u64(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol, ARRAY_SIZE(pol), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_STRING])
		fprintf(stderr, "string: %s\n", blobmsg_get_string(tb[FOO_STRING]));
	if (tb[FOO_INT64_MAX])
		fprintf(stderr, "int64_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT64_MAX]));
	if (tb[FOO_INT64_MIN])
		fprintf(stderr, "int64_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT64_MIN]));
	if (tb[FOO_INT32_MAX])
		fprintf(stderr, "int32_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT32_MAX]));
	if (tb[FOO_INT32_MIN])
		fprintf(stderr, "int32_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT32_MIN]));
	if (tb[FOO_INT16_MAX])
		fprintf(stderr, "int16_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT16_MAX]));
	if (tb[FOO_INT16_MIN])
		fprintf(stderr, "int16_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT16_MIN]));
	if (tb[FOO_INT8_MAX])
		fprintf(stderr, "int8_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT8_MAX]));
	if (tb[FOO_INT8_MIN])
		fprintf(stderr, "int8_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT8_MIN]));
	if (tb[FOO_DOUBLE_MAX])
		fprintf(stderr, "double_max: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MAX]));
	if (tb[FOO_DOUBLE_MIN])
		fprintf(stderr, "double_min: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MIN]));
}

static void dump_message_cast_s64(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol, ARRAY_SIZE(pol), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_STRING])
		fprintf(stderr, "string: %s\n", blobmsg_get_string(tb[FOO_STRING]));
	if (tb[FOO_INT64_MAX])
		fprintf(stderr, "int64_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT64_MAX]));
	if (tb[FOO_INT64_MIN])
		fprintf(stderr, "int64_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT64_MIN]));
	if (tb[FOO_INT32_MAX])
		fprintf(stderr, "int32_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT32_MAX]));
	if (tb[FOO_INT32_MIN])
		fprintf(stderr, "int32_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT32_MIN]));
	if (tb[FOO_INT16_MAX])
		fprintf(stderr, "int16_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT16_MAX]));
	if (tb[FOO_INT16_MIN])
		fprintf(stderr, "int16_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT16_MIN]));
	if (tb[FOO_INT8_MAX])
		fprintf(stderr, "int8_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT8_MAX]));
	if (tb[FOO_INT8_MIN])
		fprintf(stderr, "int8_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT8_MIN]));
	if (tb[FOO_DOUBLE_MAX])
		fprintf(stderr, "double_max: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MAX]));
	if (tb[FOO_DOUBLE_MIN])
		fprintf(stderr, "double_min: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MIN]));
}

static void dump_message_json(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol_json, ARRAY_SIZE(pol_json), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_STRING])
		fprintf(stderr, "string: %s\n", blobmsg_get_string(tb[FOO_STRING]));
	if (tb[FOO_INT64_MAX])
		fprintf(stderr, "int64_max: %" PRId64 "\n", blobmsg_get_u64(tb[FOO_INT64_MAX]));
	if (tb[FOO_INT64_MIN])
		fprintf(stderr, "int64_min: %" PRId64 "\n", blobmsg_get_u64(tb[FOO_INT64_MIN]));
	if (tb[FOO_INT32_MAX])
		fprintf(stderr, "int32_max: %" PRId32 "\n", blobmsg_get_u32(tb[FOO_INT32_MAX]));
	if (tb[FOO_INT32_MIN])
		fprintf(stderr, "int32_min: %" PRId32 "\n", blobmsg_get_u32(tb[FOO_INT32_MIN]));
	/* u16 is unknown to json, retrieve as u32 */
	if (tb[FOO_INT16_MAX])
		fprintf(stderr, "int16_max: %" PRId32 "\n", blobmsg_get_u32(tb[FOO_INT16_MAX]));
	/* u16 is unknown to json, retrieve as u32 */
	if (tb[FOO_INT16_MIN])
		fprintf(stderr, "int16_min: %" PRId32 "\n", blobmsg_get_u32(tb[FOO_INT16_MIN]));
	/* u8 is converted to boolean (true: all values != 0/false: value 0) in json */
	if (tb[FOO_INT8_MAX])
		fprintf(stderr, "int8_max: %" PRId8 "\n", blobmsg_get_u8(tb[FOO_INT8_MAX]));
	/* u8 is converted to boolean (true: all values != 0/false: value 0) in json */
	if (tb[FOO_INT8_MIN])
		fprintf(stderr, "int8_min: %" PRId8 "\n", blobmsg_get_u8(tb[FOO_INT8_MIN]));
	if (tb[FOO_DOUBLE_MAX])
		fprintf(stderr, "double_max: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MAX]));
	if (tb[FOO_DOUBLE_MIN])
		fprintf(stderr, "double_min: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MIN]));
}

static void dump_message_cast_u64_json(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol_json, ARRAY_SIZE(pol_json), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_STRING])
		fprintf(stderr, "string: %s\n", blobmsg_get_string(tb[FOO_STRING]));
	if (tb[FOO_INT64_MAX])
		fprintf(stderr, "int64_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT64_MAX]));
	if (tb[FOO_INT64_MIN])
		fprintf(stderr, "int64_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT64_MIN]));
	if (tb[FOO_INT32_MAX])
		fprintf(stderr, "int32_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT32_MAX]));
	if (tb[FOO_INT32_MIN])
		fprintf(stderr, "int32_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT32_MIN]));
	if (tb[FOO_INT16_MAX])
		fprintf(stderr, "int16_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT16_MAX]));
	if (tb[FOO_INT16_MIN])
		fprintf(stderr, "int16_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT16_MIN]));
	if (tb[FOO_INT8_MAX])
		fprintf(stderr, "int8_max: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT8_MAX]));
	if (tb[FOO_INT8_MIN])
		fprintf(stderr, "int8_min: %" PRIu64 "\n", blobmsg_cast_u64(tb[FOO_INT8_MIN]));
	if (tb[FOO_DOUBLE_MAX])
		fprintf(stderr, "double_max: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MAX]));
	if (tb[FOO_DOUBLE_MIN])
		fprintf(stderr, "double_min: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MIN]));
}

static void dump_message_cast_s64_json(struct blob_buf *buf)
{
	struct blob_attr *tb[ARRAY_SIZE(pol)];

	if (blobmsg_parse(pol_json, ARRAY_SIZE(pol_json), tb, blob_data(buf->head), blob_len(buf->head)) != 0) {
		fprintf(stderr, "Parse failed\n");
		return;
	}
	if (tb[FOO_STRING])
		fprintf(stderr, "string: %s\n", blobmsg_get_string(tb[FOO_STRING]));
	if (tb[FOO_INT64_MAX])
		fprintf(stderr, "int64_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT64_MAX]));
	if (tb[FOO_INT64_MIN])
		fprintf(stderr, "int64_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT64_MIN]));
	if (tb[FOO_INT32_MAX])
		fprintf(stderr, "int32_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT32_MAX]));
	if (tb[FOO_INT32_MIN])
		fprintf(stderr, "int32_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT32_MIN]));
	if (tb[FOO_INT16_MAX])
		fprintf(stderr, "int16_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT16_MAX]));
	if (tb[FOO_INT16_MIN])
		fprintf(stderr, "int16_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT16_MIN]));
	if (tb[FOO_INT8_MAX])
		fprintf(stderr, "int8_max: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT8_MAX]));
	if (tb[FOO_INT8_MIN])
		fprintf(stderr, "int8_min: %" PRId64 "\n", blobmsg_cast_s64(tb[FOO_INT8_MIN]));
	if (tb[FOO_DOUBLE_MAX])
		fprintf(stderr, "double_max: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MAX]));
	if (tb[FOO_DOUBLE_MIN])
		fprintf(stderr, "double_min: %e\n", blobmsg_get_double(tb[FOO_DOUBLE_MIN]));
}

static void
fill_message(struct blob_buf *buf)
{
	blobmsg_add_string(buf, "string", "Hello, world!");
	blobmsg_add_u64(buf, "int64_max", INT64_MAX);
	blobmsg_add_u64(buf, "int64_min", INT64_MIN);
	blobmsg_add_u32(buf, "int32_max", INT32_MAX);
	blobmsg_add_u32(buf, "int32_min", INT32_MIN);
	blobmsg_add_u16(buf, "int16_max", INT16_MAX);
	blobmsg_add_u16(buf, "int16_min", INT16_MIN);
	blobmsg_add_u8(buf, "int8_max", INT8_MAX);
	blobmsg_add_u8(buf, "int8_min", INT8_MIN);
	blobmsg_add_double(buf, "double_max", DBL_MAX);
	blobmsg_add_double(buf, "double_min", DBL_MIN);
}

int main(int argc, char **argv)
{
	char *json = NULL;
	static struct blob_buf buf;

	blobmsg_buf_init(&buf);
	fill_message(&buf);
	fprintf(stderr, "[*] blobmsg dump:\n");
	dump_message(&buf);
	fprintf(stderr, "[*] blobmsg dump cast_u64:\n");
	dump_message_cast_u64(&buf);
	fprintf(stderr, "[*] blobmsg dump cast_s64:\n");
	dump_message_cast_s64(&buf);

	json = blobmsg_format_json(buf.head, true);
	if (!json)
		exit(EXIT_FAILURE);

	fprintf(stderr, "\n[*] blobmsg to json: %s\n", json);

	blobmsg_buf_init(&buf);
	if (!blobmsg_add_json_from_string(&buf, json))
		exit(EXIT_FAILURE);

	fprintf(stderr, "\n[*] blobmsg from json:\n");
	dump_message_json(&buf);
	fprintf(stderr, "\n[*] blobmsg from json/cast_u64:\n");
	dump_message_cast_u64_json(&buf);
	fprintf(stderr, "\n[*] blobmsg from json/cast_s64:\n");
	dump_message_cast_s64_json(&buf);

	if (buf.buf)
		free(buf.buf);
	free(json);

	return 0;
}
