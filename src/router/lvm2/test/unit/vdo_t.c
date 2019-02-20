/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "units.h"
#include "device_mapper/vdo/target.h"

//----------------------------------------------------------------

static bool _status_eq(struct dm_vdo_status *lhs, struct dm_vdo_status *rhs)
{
	return !strcmp(lhs->device, rhs->device) &&
		(lhs->operating_mode == rhs->operating_mode) &&
		(lhs->recovering == rhs->recovering) &&
		(lhs->index_state == rhs->index_state) &&
		(lhs->compression_state == rhs->compression_state) &&
		(lhs->used_blocks == rhs->used_blocks) &&
		(lhs->total_blocks == rhs->total_blocks);
}

#if 0
static const char *_op_mode(enum dm_vdo_operating_mode m)
{
	switch (m) {
	case DM_VDO_MODE_RECOVERING:
		return "recovering";
	case DM_VDO_MODE_READ_ONLY:
		return "read-only";
	case DM_VDO_MODE_NORMAL:
		return "normal";
	}

	return "<unknown>";
}

static const char *_index_state(enum dm_vdo_index_state is)
{
	switch (is) {
	case DM_VDO_INDEX_ERROR:
		return "error";
	case DM_VDO_INDEX_CLOSED:
		return "closed";
	case DM_VDO_INDEX_OPENING:
		return "opening";
	case DM_VDO_INDEX_CLOSING:
		return "closing";
	case DM_VDO_INDEX_OFFLINE:
		return "offline";
	case DM_VDO_INDEX_ONLINE:
		return "online";
	case DM_VDO_INDEX_UNKNOWN:
		return "unknown";
	}

	return "<unknown>";
}

static void _print_status(FILE *stream, struct dm_vdo_status *s)
{
	fprintf(stream, "<status| %s ", s->device);
	fprintf(stream, "%s ", _op_mode(s->operating_mode));
	fprintf(stream, "%s ", s->recovering ? "recovering" : "-");
	fprintf(stream, "%s ", _index_state(s->index_state));
	fprintf(stream, "%s ", s->compression_state == DM_VDO_COMPRESSION_ONLINE ? "online" : "offline");
	fprintf(stream, "%llu ", (unsigned long long) s->used_blocks);
	fprintf(stream, "%llu", (unsigned long long) s->total_blocks);
	fprintf(stream, ">");
}
#endif

struct example_good {
	const char *input;
	struct dm_vdo_status status;
};

static void _check_good(struct example_good *es, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++) {
		struct example_good *e = es + i;
		struct dm_vdo_status_parse_result pr;

		T_ASSERT(dm_vdo_status_parse(NULL, e->input, &pr));
#if 0
		_print_status(stderr, pr.status);
		fprintf(stderr, "\n");
		_print_status(stderr, &e->status);
		fprintf(stderr, "\n");
#endif
		T_ASSERT(_status_eq(&e->status, pr.status));
		free(pr.status);
	}
}

struct example_bad {
	const char *input;
	const char *reason;
};

static void _check_bad(struct example_bad *es, unsigned count)
{
	unsigned i;

	for (i = 0; i < count; i++) {
		struct example_bad *e = es + i;
		struct dm_vdo_status_parse_result pr;

		T_ASSERT(!dm_vdo_status_parse(NULL, e->input, &pr));
		T_ASSERT(!strcmp(e->reason, pr.error));
	}
}

static void _test_device_names_good(void *fixture)
{
	static struct example_good _es[] = {
		{"foo1234 read-only - error online 0 1234",
		{(char *) "foo1234", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"f read-only - error online 0 1234",
		{(char *) "f", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_operating_mode_good(void *fixture)
{
	static struct example_good _es[] = {
		{"device-name recovering - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name read-only - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name normal - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_NORMAL, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_operating_mode_bad(void *fixture)
{
	static struct example_bad _es[] = {
		{"device-name investigating - error online 0 1234",
		"couldn't parse 'operating mode'"}};

	_check_bad(_es, DM_ARRAY_SIZE(_es));
}

static void _test_recovering_good(void *fixture)
{
	static struct example_good _es[] = {
		{"device-name recovering - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name read-only recovering error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_READ_ONLY, true, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_recovering_bad(void *fixture)
{
	static struct example_bad _es[] = {
		{"device-name normal fish error online 0 1234",
		"couldn't parse 'recovering'"}};

	_check_bad(_es, DM_ARRAY_SIZE(_es));
}

static void _test_index_state_good(void *fixture)
{
	static struct example_good _es[] = {
		{"device-name recovering - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - closed online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_CLOSED, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - opening online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_OPENING, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - closing online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_CLOSING, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - offline online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_OFFLINE, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - online online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ONLINE, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - unknown online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_UNKNOWN, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_index_state_bad(void *fixture)
{
	static struct example_bad _es[] = {
		{"device-name normal - fish online 0 1234",
		"couldn't parse 'index state'"}};

	_check_bad(_es, DM_ARRAY_SIZE(_es));
}

static void _test_compression_state_good(void *fixture)
{
	static struct example_good _es[] = {
		{"device-name recovering - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name read-only - error offline 0 1234",
		{(char *) "device-name", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_OFFLINE, 0, 1234}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_compression_state_bad(void *fixture)
{
	static struct example_bad _es[] = {
		{"device-name normal - error fish 0 1234",
		"couldn't parse 'compression state'"}};

	_check_bad(_es, DM_ARRAY_SIZE(_es));
}

static void _test_used_blocks_good(void *fixture)
{
	static struct example_good _es[] = {
		{"device-name recovering - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name read-only - error offline 1 1234",
		{(char *) "device-name", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_OFFLINE, 1, 1234}},
		{"device-name read-only - error offline 12 1234",
		{(char *) "device-name", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_OFFLINE, 12, 1234}},
		{"device-name read-only - error offline 3456 1234",
		{(char *) "device-name", DM_VDO_MODE_READ_ONLY, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_OFFLINE, 3456, 1234}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_used_blocks_bad(void *fixture)
{
	static struct example_bad _es[] = {
		{"device-name normal - error online fish 1234",
		"couldn't parse 'used blocks'"}};

	_check_bad(_es, DM_ARRAY_SIZE(_es));
}

static void _test_total_blocks_good(void *fixture)
{
	static struct example_good _es[] = {
		{"device-name recovering - error online 0 1234",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1234}},
		{"device-name recovering - error online 0 1",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 1}},
		{"device-name recovering - error online 0 0",
		{(char *) "device-name", DM_VDO_MODE_RECOVERING, false, DM_VDO_INDEX_ERROR, DM_VDO_COMPRESSION_ONLINE, 0, 0}},
	};

	_check_good(_es, DM_ARRAY_SIZE(_es));
}

static void _test_total_blocks_bad(void *fixture)
{
	static struct example_bad _es[] = {
		{"device-name normal - error online 0 fish",
		"couldn't parse 'total blocks'"}};

	_check_bad(_es, DM_ARRAY_SIZE(_es));
}

static void _test_status_bad(void *fixture)
{
	struct example_bad _bad[] = {
		{"", "couldn't get token for device"},
		{"device-name read-only - error online 0 1000 lksd", "too many tokens"}
	};

	_check_bad(_bad, DM_ARRAY_SIZE(_bad));
}

//----------------------------------------------------------------

#define T(path, desc, fn) register_test(ts, "/device-mapper/vdo/status/" path, desc, fn)

static struct test_suite *_tests(void)
{
	struct test_suite *ts = test_suite_create(NULL, NULL);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	};

	T("device-names", "parse various device names", _test_device_names_good);
	T("operating-mode-good", "operating mode, good examples", _test_operating_mode_good);
	T("operating-mode-bad", "operating mode, bad examples", _test_operating_mode_bad);
	T("recovering-good", "recovering, good examples", _test_recovering_good);
	T("recovering-bad", "recovering, bad examples", _test_recovering_bad);
	T("index-state-good", "index state, good examples", _test_index_state_good);
	T("index-state-bad", "index state, bad examples", _test_index_state_bad);
	T("compression-state-good", "compression state, good examples", _test_compression_state_good);
	T("compression-state-bad", "compression state, bad examples", _test_compression_state_bad);
	T("used-blocks-good", "used blocks, good examples", _test_used_blocks_good);
	T("used-blocks-bad", "used blocks, bad examples", _test_used_blocks_bad);
	T("total-blocks-good", "total blocks, good examples", _test_total_blocks_good);
	T("total-blocks-bad", "total blocks, bad examples", _test_total_blocks_bad);
	T("bad", "parse various badly formed vdo status lines", _test_status_bad);

	return ts;
}

void vdo_tests(struct dm_list *all_tests)
{
	dm_list_add(all_tests, &_tests()->list);
}

//----------------------------------------------------------------
