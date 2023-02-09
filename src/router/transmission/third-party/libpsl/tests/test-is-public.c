/*
 * Copyright(c) 2014-2018 Tim Ruehsen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the test suite of libpsl.
 *
 * Test case for psl_load_file(), psl_is_public_suffix(), psl_free()
 *
 * Changelog
 * 19.03.2014  Tim Ruehsen  created from libmget/cookie.c
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_ALLOCA_H
#	include <alloca.h>
#endif

#include <libpsl.h>

#define countof(a) (sizeof(a)/sizeof(*(a)))

static int
	ok,
	failed;

static void test_psl(void)
{
	/* punycode generation: idn ?? */
	/* octal code generation: echo -n "??" | od -b */
	static const struct test_data {
		const char
			*domain;
		int
			result;
		int
		    no_star_result;
	} test_data[] = {
		{ "www.example.com", 0, 0 },
		{ "com.ar", 1 , 1},
		{ "www.com.ar", 0, 0 },
		{ "cc.ar.us", 1, 1 },
		{ ".cc.ar.us", 1, 1 },
		{ "www.cc.ar.us", 0, 0 },
		{ "www.ck", 0, 0 }, /* exception from *.ck */
		{ "abc.www.ck", 0, 0 },
		{ "xxx.ck", 1, 1 },
		{ "www.xxx.ck", 0, 0 },
		{ "\345\225\206\346\240\207", 1, 1 }, /* xn--czr694b or ?? */
		{ "www.\345\225\206\346\240\207", 0, 0 },
		/* some special test follow ('name' and 'forgot.his.name' are public, but e.g. his.name is not) */
		{ "name", 1, 1 },
		{ ".name", 1, 1 },
		{ "his.name", 0, 0 },
		{ ".his.name", 0, 0 },
		{ "forgot.his.name", 1, 1 },
		{ ".forgot.his.name", 1, 1 },
		{ "whoever.his.name", 0, 0 },
		{ "whoever.forgot.his.name", 0, 0},
		{ ".", 1, 0 }, /* special case */
		{ "", 1, 0 },  /* special case */
		{ NULL, 1, 1 },  /* special case */
		{ "adfhoweirh", 1, 0 }, /* unknown TLD */
		{ "compute.amazonaws.com", 1, 1 }, /* special rule *.compute.amazonaws.com */
		{ "y.compute.amazonaws.com", 1, 1 },
		{ "x.y.compute.amazonaws.com", 0, 0 },
	};
	unsigned it;
	int result, ver;
	psl_ctx_t *psl;

	psl = psl_load_file(PSL_FILE);

	printf("loaded %d suffixes and %d exceptions\n", psl_suffix_count(psl), psl_suffix_exception_count(psl));

	for (it = 0; it < countof(test_data); it++) {
		const struct test_data *t = &test_data[it];
		result = psl_is_public_suffix(psl, t->domain);

		if (result == t->result) {
			ok++;
		} else {
			failed++;
			printf("psl_is_public_suffix(%s)=%d (expected %d)\n", t->domain, result, t->result);
		}
	}

	for (it = 0; it < countof(test_data); it++) {
		const struct test_data *t = &test_data[it];
		result = psl_is_public_suffix2(psl, t->domain, PSL_TYPE_ANY|PSL_TYPE_NO_STAR_RULE);

		if (result == t->no_star_result) {
			ok++;
		} else {
			failed++;
			printf("psl_is_public_suffix2(%s, NO_STAR_RULE)=%d (expected %d)\n", t->domain, result, t->no_star_result);
		}
	}

	/* do some checks to cover more code paths in libpsl */
	psl_is_public_suffix(NULL, "xxx");

	if ((ver = psl_check_version_number(0)) == 0) {
		printf("psl_check_version_number(0) is 0\n");
		failed++;
	} else {
		if (((result = psl_check_version_number(ver)) != ver)) {
			printf("psl_check_version_number(%06X) is %06X\n", ver, result);
			failed++;
		}

		if (((result = psl_check_version_number(ver - 1)) != 0)) {
			printf("psl_check_version_number(%06X) is %06X\n", ver - 1, result);
			failed++;
		}

		if (((result = psl_check_version_number(ver + 1)) != ver)) {
			printf("psl_check_version_number(%06X) is %06X\n", ver, result);
			failed++;
		}
	}

	psl_str_to_utf8lower("www.example.com", "utf-8", "en", NULL);
	psl_str_to_utf8lower(NULL, "utf-8", "en", NULL);

	{
		char *lower = NULL;

		psl_str_to_utf8lower("www.example.com", NULL, "de", &lower);
		psl_free_string(lower); lower = NULL;

		psl_str_to_utf8lower("\374bel.de", NULL, "de", &lower);
		psl_free_string(lower); lower = NULL;

		psl_str_to_utf8lower("\374bel.de", "iso-8859-1", NULL, &lower);
		psl_free_string(lower); lower = NULL;

		psl_str_to_utf8lower(NULL, "utf-8", "en", &lower);
		psl_free_string(lower); lower = NULL;
	}

	psl_get_version();
	psl_dist_filename();
	psl_builtin_filename();
	psl_builtin_outdated();
	psl_builtin_file_time();
	psl_builtin_sha1sum();
	psl_suffix_wildcard_count(NULL);
	psl_suffix_wildcard_count(psl);
	psl_suffix_wildcard_count(psl_builtin());
	psl_suffix_count(NULL);
	psl_suffix_exception_count(NULL);
	psl_load_file(NULL);
	psl_load_fp(NULL);
	psl_registrable_domain(NULL, "");
	psl_registrable_domain(psl, NULL);
	psl_registrable_domain(psl, "www.example.com");
	psl_unregistrable_domain(NULL, "");
	psl_unregistrable_domain(psl, NULL);
	psl_is_public_suffix2(NULL, "", PSL_TYPE_ANY);
	psl_is_public_suffix2(psl, NULL, PSL_TYPE_ANY);

	psl_free(psl);
}

int main(int argc, const char * const *argv)
{
	/* if VALGRIND testing is enabled, we have to call ourselves with valgrind checking */
	if (argc == 1) {
		const char *valgrind = getenv("TESTS_VALGRIND");

		if (valgrind && *valgrind) {
			size_t cmdsize = strlen(valgrind) + strlen(argv[0]) + 32;
			char *cmd = alloca(cmdsize);

			snprintf(cmd, cmdsize, "TESTS_VALGRIND="" %s %s", valgrind, argv[0]);
			return system(cmd) != 0;
		}
	}

	test_psl();

	if (failed) {
		printf("Summary: %d out of %d tests failed\n", failed, ok + failed);
		return 1;
	}

	printf("Summary: All %d tests passed\n", ok + failed);
	return 0;
}
