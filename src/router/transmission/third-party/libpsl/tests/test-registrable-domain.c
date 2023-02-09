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
 * Test psl_registered_domain() for all entries in test_psl.dat
 *
 * Changelog
 * 26.03.2014  Tim Ruehsen  created
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

static int
	ok,
	failed;

static void testx(const psl_ctx_t *psl, const char *domain, const char *encoding, const char *lang, const char *expected_result)
{
	const char *result;
	char *lower = NULL;
	int rc;

	/* just to cover special code paths for valgrind checking */
	psl_str_to_utf8lower(domain, encoding, lang, NULL);

	if ((rc = psl_str_to_utf8lower(domain, encoding, lang, &lower)) == PSL_SUCCESS)
		domain = lower;
	/* non-ASCII domains fail here if no runtime IDN library is configured, so skip it */
#if defined(WITH_LIBIDN) || defined(WITH_LIBIDN2) || defined(WITH_LIBICU)
	else if (domain) {
		/* if we do not runtime support, test failure have to be skipped */
		failed++;
		printf("psl_str_to_utf8lower(%s)=%d\n", domain ? domain : "NULL", rc);
		return;
	}
#endif

	result = psl_registrable_domain(psl, domain);

	if ((result && expected_result && !strcmp(result, expected_result)) || (!result && !expected_result)) {
		ok++;
	} else {
		failed++;
		printf("psl_registrable_domain(%s)=%s (expected %s)\n",
			   domain ? domain : "NULL", result ? result : "NULL", expected_result ? expected_result : "NULL");
	}

	psl_free_string(lower);
}

static void test(const psl_ctx_t *psl, const char *domain, const char *expected_result)
{
	testx(psl, domain, "utf-8", "en", expected_result);
}

static void test_iso(const psl_ctx_t *psl, const char *domain, const char *expected_result)
{
	/* makes only sense with a runtime IDN library configured */
#if defined(WITH_LIBIDN) || defined(WITH_LIBIDN2) || defined(WITH_LIBICU)
	testx(psl, domain, "iso-8859-15", "de", expected_result);
#endif
}

static void test_psl(void)
{
	FILE *fp;
	const psl_ctx_t *psl;
	const char *p;
	char buf[256], domain[128], expected_regdom[128], semicolon[2];
	char lbuf[258];
	int er_is_null, d_is_null;
	unsigned it;

	psl = psl_builtin();

	printf("have %d suffixes and %d exceptions\n", psl_suffix_count(psl), psl_suffix_exception_count(psl));

	/* special check with NULL values */
	test(NULL, NULL, NULL);

	/* special check with NULL psl context */
	test(NULL, "www.example.com", NULL);

	/* special check with NULL psl context and TLD */
	test(NULL, "com", NULL);

	/* Norwegian with uppercase oe */
#ifdef WITH_LIBICU
	test(psl, "www.\303\230yer.no", "www.\303\270yer.no");
#endif

	/* Norwegian with lowercase oe */
	test(psl, "www.\303\270yer.no", "www.\303\270yer.no");

	/* Norwegian with lowercase oe, encoded as ISO-8859-15 */
	test_iso(psl, "www.\370yer.no", "www.\303\270yer.no");

	/* Testing special code paths of psl_str_to_utf8lower() */
	for (it = 254; it <= 257; it++) {
		memset(lbuf, 'a', it);
		lbuf[it] = 0;

		lbuf[0] = '\370';
		test_iso(psl, lbuf, NULL);

		lbuf[0] = '\303';
		lbuf[1] = '\270';
		test(psl, lbuf, NULL);
	}

	/* special check with NULL psl context and TLD */
	test(psl, "whoever.forgot.his.name", "whoever.forgot.his.name");

	/* special check with NULL psl context and TLD */
	test(psl, "forgot.his.name", NULL);

	/* special check with NULL psl context and TLD */
	test(psl, "his.name", "his.name");

	if ((fp = fopen(PSL_TESTFILE, "r"))) {
		while ((fgets(buf, sizeof(buf), fp))) {
			/* advance over ASCII white space */
			for (p = buf; *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'; p++)
				;

			if (!*p || (*p == '/' && p[1] == '/'))
				continue; /* ignore comments and blank lines */

			er_is_null = 0;
			d_is_null = 0;

			if (sscanf(p, "checkPublicSuffix ( '%127[^']' , '%127[^']' ) %1[;]", domain, expected_regdom, semicolon) != 3) {
				if (sscanf(p, "checkPublicSuffix ( '%127[^']' , null ) %1[;]", domain, semicolon) == 2) {
					er_is_null = 1;
				} else if (sscanf(p, "checkPublicSuffix ( null , '%127[^']' ) %1[;]", expected_regdom, semicolon) == 2) {
					d_is_null = 1;
				} else if (sscanf(p, "checkPublicSuffix ( null , null ) %1[;]", semicolon) == 1) {
					d_is_null = 1;
					er_is_null = 1;
				} else if (sscanf(p, "%127s %127s", domain, expected_regdom) == 2) {
					if (!strcmp(domain, "null"))
						d_is_null = 1;
					if (!strcmp(expected_regdom, "null"))
						er_is_null = 1;
				} else {
					failed++;
					printf("Malformed line from '" PSL_TESTFILE "': %s", buf);
					continue;
				}
			}

			test(psl, d_is_null ? NULL : domain, er_is_null ? NULL : expected_regdom);
		}

		fclose(fp);
	} else {
		printf("Failed to open %s\n", PSL_TESTFILE);
		failed++;
	}
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
