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
#include "scripts/generator-internals.c"

#include "device_mapper/all.h"

//----------------------------------------------------------------

static void _error(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

//----------------------------------------------------------------

struct bw_test {
	const char *input;
	const char *prefix;
	const char *val;
};

static void _test_begins_with(void *fixture)
{
	static struct bw_test _tests[] = {
		{"", "foo", NULL},
		{"lskdj", "foo", NULL},
		{"foo", "foobar", NULL},
		{"fish", "fish", ""},
		{"foo=bar ", "foo=", "bar "},
        };

	unsigned i;
	for (i = 0; i < DM_ARRAY_SIZE(_tests); i++) {
        	const char *val;
        	struct bw_test *t = _tests + i;
        	if (t->val) {
                	if (!_begins_with(t->input, t->prefix, &val))
                        	test_fail("_begins_with('%s', '%s') failed", t->input, t->prefix);
			if (strcmp(val, t->val))
        			test_fail("_begins_with('%s', '%s') -> '%s', expected '%s'",
                                          t->input, t->prefix, val, t->val);
		} else {
			if (_begins_with(t->input, t->prefix, &val))
        			test_fail("_begins_with('%s', '%s') unexpectedly succeeded",
                                          t->input, t->prefix);

		}
	}
}

struct pb_test {
	const char *input;
	bool parsed;
	bool result;
};

static const char *_bool(bool v)
{
        return v ? "true" : "false";
}

static void _test_parse_bool(void *fixture)
{
	static struct pb_test _tests[] = {
		{"", false, false},
		{"fish", false, false},
		{"true", false, false},
		{"false", false, false},
		{"1", true, true},
		{" \t 1\t\t", true, true},
		{"0", true, false},
		{"  \t0  ", true, false}
	};

	unsigned i;

	for (i = 0; i < DM_ARRAY_SIZE(_tests); i++) {
        	bool result;
        	struct pb_test *t = _tests + i;

        	if (t->parsed) {
			if (!_parse_bool(t->input, &result))
        			test_fail("_parse_bool('%s') unexpectedly failed", t->input);
        		if (result != t->result)
                		test_fail("_parse_bool('%s') -> %s", t->input, _bool(result));
        	} else {
                	if (_parse_bool(t->input, &result))
                        	test_fail("_parse_bool('%s') unexpectedly succeeded", t->input);
        	}
	}
}

struct pl_test {
	const char *input;
	bool success;
	bool use_lvmetad;
	bool sysinit_needed;
};

static void _test_parse_line(void *fixture)
{
	static struct pl_test _tests[] = {
        	{"", false, false, false},
        	{"sldkjfs", false, false, false},
        	{"use_lvmetad=1", true, true, true},
        	{"use_lvmetad=0", true, false, true},
        	{"use_lvmpolld=1", true, false, false},
        	{"use_lvmpolld=0", true, false, true}
	};

	unsigned i;

	for (i = 0; i< DM_ARRAY_SIZE(_tests); i++) {
        	bool r;
		struct config cfg = {
			.sysinit_needed = true
		};
		struct pl_test *t = _tests + i;

		r = _parse_line(t->input, &cfg);
		if (t->success) {
			if (!r)
        			test_fail("_parse_line('%s') failed", t->input);

			//if (cfg.use_lvmetad != t->use_lvmetad)
			//	test_fail("_parse_line('%s') -> use_lvmetad='%s'",
			//		t->input, _bool(cfg.use_lvmetad));

                	if (cfg.sysinit_needed != t->sysinit_needed)
                        	test_fail("_parse_line('%s') -> sysinit_needed='%s'",
                                          t->input, _bool(cfg.sysinit_needed));
		} else if (r)
                		test_fail("_parse_line('%s') succeeded", t->input);
	}
}

static void _test_get_config_bad_path(void *fixture)
{
        struct config cfg;

	if (_get_config(&cfg, "/usr/bin/no-such-file"))
        	test_fail("_get_config() succeeded despite a bad lvmconfig path");
}

static void _test_get_config_bad_exit(void *fixture)
{
        struct config cfg;

	if (_get_config(&cfg, "/usr/bin/false"))
        	test_fail("_get_config() succeeded despite a bad lvmconfig exit");
}

struct gc_test {
        const char *output;
        bool success;
        bool use_lvmetad;
        bool sysinit_needed;
};

static const char *_fake_lvmconfig(const char *output)
{
        const char *path = "./fake-lvmconfig";

	FILE *fp = fopen(path, "w");
	if (!fp)
        	return NULL;

	fprintf(fp, "#!/usr/bin/sh\n");
	fprintf(fp, "cat <<EOF\n");
	fprintf(fp, "%s", output);
	fprintf(fp, "EOF\n");

	fclose(fp);
	chmod(path, 0770);

	return path;
}

static void _test_get_config(void *fixture)
{
	static struct gc_test _tests[] = {
		{"", true, false, true},
		{"lsdjkf\n\n\n", false, false, false},

		{"use_lvmetad=0\nuse_lvmpolld=1\n", true, false, false},
		{"use_lvmetad=1\nuse_lvmpolld=1\n", true, true, false},
		{"use_lvmetad=1\nuse_lvmpolld=0\n", true, true, true},
	};

	bool r;
	unsigned i;
	const char *path;

	for (i = 0; i < DM_ARRAY_SIZE(_tests); i++) {
        	struct gc_test *t = _tests + i;
		struct config cfg = {
			.sysinit_needed = true
		};

		path = _fake_lvmconfig(t->output);
		if (!path)
        		test_fail("couldn't create fake lvmconfig");

        	r = _get_config(&cfg, path);
        	if (t->success) {
			if (!r)
        			test_fail("_get_config() <- '%s' failed", t->output);

			//if (t->use_lvmetad != cfg.use_lvmetad)
			//	test_fail("_get_config() <- '%s', use_lvmetad = %s",
			//		  t->output, _bool(cfg.use_lvmetad));

                	if (t->sysinit_needed != cfg.sysinit_needed)
                        	test_fail("_get_config() <- '%s', sysinit = %s",
                                          t->output, _bool(cfg.sysinit_needed));
        	} else {
			if (r)
        			test_fail("_get_config() <- '%s' unexpectedly succeeded", t->output);
        	}

        	unlink(path);
	}
}

//----------------------------------------------------------------

#define T(path, desc, fn) register_test(ts, "/activation-generator/" path, desc, fn)

static struct test_suite *_tests(void)
{
	struct test_suite *ts = test_suite_create(NULL, NULL);
	if (!ts) {
        	fprintf(stderr, "out of memory\n");
        	exit(1);
	};

	T("begins-with", "Test cases for _begins_with()", _test_begins_with);
	T("parse-bool", "Test cases for _parse_bool()", _test_parse_bool);
	T("parse-line", "Test cases for _parse_line()", _test_parse_line);
	T("get-config-bad-path", "_get_config() needs a valid lvmconfig path", _test_get_config_bad_path);
	T("get-config-bad-exit", "lvmconfig bad exit code gets propagated", _test_get_config_bad_exit);
	T("get-config", "Test cases for _get_config()", _test_get_config);

	return ts;
}

void activation_generator_tests(struct dm_list *all_tests)
{
	dm_list_add(all_tests, &_tests()->list);
}

//----------------------------------------------------------------
