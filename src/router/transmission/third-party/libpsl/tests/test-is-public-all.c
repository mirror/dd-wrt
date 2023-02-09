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
 * Test psl_is_public_suffix() for all entries in public_suffix_list.dat
 *
 * Changelog
 * 19.03.2014  Tim Ruehsen  created
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_ALLOCA_H
#	include <alloca.h>
#endif

#include <libpsl.h>

static int
	ok,
	failed;
#ifdef HAVE_CLOCK_GETTIME
	static struct timespec ts1, ts2;
#endif

static int _isspace_ascii(const char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static const char *_type_string(int type)
{
	switch (type) {
	case PSL_TYPE_ANY: return "PSL_TYPE_ANY";
	case PSL_TYPE_PRIVATE: return "PSL_TYPE_PRIVATE";
	case PSL_TYPE_ICANN: return "PSL_TYPE_ICANN";
	case PSL_TYPE_ANY|PSL_TYPE_NO_STAR_RULE: return "PSL_TYPE_ANY|PSL_TYPE_NO_STAR_RULE";
	case PSL_TYPE_PRIVATE|PSL_TYPE_NO_STAR_RULE: return "PSL_TYPE_PRIVATE|PSL_TYPE_NO_STAR_RULE";
	case PSL_TYPE_ICANN|PSL_TYPE_NO_STAR_RULE: return "PSL_TYPE_ICANN|PSL_TYPE_NO_STAR_RULE";
	default: return "Unsupported type";
	}
}

static void test_ps(const psl_ctx_t *psl, const char *domain, int type, int expected)
{
	int result;

	if ((result = psl_is_public_suffix2(psl, domain, type)) != expected) {
		failed++;
		printf("psl_is_public_suffix2(%s, %s)=%d (expected %d)\n", domain, _type_string(type), result, expected);
	} else ok++;
}

/* section: either PSL_TYPE_PRIVATE or PSL_TYPE_ICANN */
static void test_type_any(const psl_ctx_t *psl, const char *domain, int type, int expected)
{
	int wildcard = (*domain == '.');
	int tld = !(strchr(domain + wildcard, '.'));

	test_ps(psl, domain, type, expected);
	test_ps(psl, domain, type|PSL_TYPE_NO_STAR_RULE, expected);
	test_ps(psl, domain, PSL_TYPE_ANY, expected);
	test_ps(psl, domain, PSL_TYPE_ANY|PSL_TYPE_NO_STAR_RULE, expected);

	if (type == PSL_TYPE_PRIVATE) {
		if (tld) {
			test_ps(psl, domain, PSL_TYPE_ICANN, 1);
			test_ps(psl, domain, PSL_TYPE_ICANN|PSL_TYPE_NO_STAR_RULE, 0);
		} else {
			test_ps(psl, domain, PSL_TYPE_ICANN, 0);
			test_ps(psl, domain, PSL_TYPE_ICANN|PSL_TYPE_NO_STAR_RULE, 0);
		}
	} else if (type == PSL_TYPE_ICANN) {
		if (tld) {
			test_ps(psl, domain, PSL_TYPE_PRIVATE, 1);
			test_ps(psl, domain, PSL_TYPE_PRIVATE|PSL_TYPE_NO_STAR_RULE, 0);
		} else {
			test_ps(psl, domain, PSL_TYPE_PRIVATE, 0);
			test_ps(psl, domain, PSL_TYPE_PRIVATE|PSL_TYPE_NO_STAR_RULE, 0);
		}
	}
}

static void test_psl_entry(const psl_ctx_t *psl, const char *domain, int type)
{
	if (*domain == '!') { /* an exception to a wildcard, e.g. !www.ck (wildcard is *.ck) */
		test_type_any(psl, domain + 1, type, 0); /* the exception itself is not a PS */

		if ((domain = strchr(domain, '.')))
			test_type_any(psl, domain, type, 1); /* the related wildcard domain is a PS */

	} else if (*domain == '*') { /* a wildcard, e.g. *.ck or *.platform.sh */
		/* '*.platform.sh' -> 'y.x.platform.sh' */
		size_t len = strlen(domain);
		char *xdomain = alloca(len + 3);

		memcpy(xdomain, "y.x", 3);
		memcpy(xdomain + 3, domain + 1, len);

		test_type_any(psl, domain + 1, type, 1); /* the domain without wildcard is a PS */
		test_type_any(psl, xdomain + 2, type, 1); /* random wildcard-matching domain is a PS... */
		test_type_any(psl, xdomain, type, 0); /* ... but sub domain is not */

	} else {
		test_type_any(psl, domain, type, 1); /* Any normal PSL entry */
	}
}

static void test_psl(void)
{
	FILE *fp;
	psl_ctx_t *psl, *psl3, *psl4, *psl5;
	const psl_ctx_t *psl2;
	int type = 0;
	char buf[256], *linep, *p;

	psl = psl_load_file(PSL_FILE); /* PSL_FILE can be set by ./configure --with-psl-file=[PATH] */
	printf("loaded %d suffixes and %d exceptions\n", psl_suffix_count(psl), psl_suffix_exception_count(psl));

	psl2 = psl_builtin();
	printf("builtin PSL has %d suffixes and %d exceptions\n", psl_suffix_count(psl2), psl_suffix_exception_count(psl2));

	if (!(psl3 = psl_load_file(PSL_DAFSA))) {
		fprintf(stderr, "Failed to load 'psl.dafsa'\n");
		failed++;
	}

	if (!(psl4 = psl_load_file(PSL_ASCII_DAFSA))) {
		fprintf(stderr, "Failed to load 'psl_ascii.dafsa'\n");
		failed++;
	}

	psl5 = psl_latest("psl.dafsa");

	if ((fp = fopen(PSL_FILE, "r"))) {
#ifdef HAVE_CLOCK_GETTIME
		clock_gettime(CLOCK_REALTIME, &ts1);
#endif

		while ((linep = fgets(buf, sizeof(buf), fp))) {
			while (_isspace_ascii(*linep)) linep++; /* ignore leading whitespace */
			if (!*linep) continue; /* skip empty lines */

			if (*linep == '/' && linep[1] == '/') {
				if (!type) {
					if (strstr(linep + 2, "===BEGIN ICANN DOMAINS==="))
						type = PSL_TYPE_ICANN;
					else if (!type && strstr(linep + 2, "===BEGIN PRIVATE DOMAINS==="))
						type = PSL_TYPE_PRIVATE;
				}
				else if (type == PSL_TYPE_ICANN && strstr(linep + 2, "===END ICANN DOMAINS==="))
					type = 0;
				else if (type == PSL_TYPE_PRIVATE && strstr(linep + 2, "===END PRIVATE DOMAINS==="))
					type = 0;

				continue; /* skip comments */
			}

			/* parse suffix rule */
			for (p = linep; *linep && !_isspace_ascii(*linep);) linep++;
			*linep = 0;

			test_psl_entry(psl, p, type);

			if (psl2)
				test_psl_entry(psl2, p, type);

			if (psl3)
				test_psl_entry(psl3, p, type);

			if (psl4)
				test_psl_entry(psl4, p, type);

			if (psl5)
				test_psl_entry(psl5, p, type);
		}

#ifdef HAVE_CLOCK_GETTIME
		clock_gettime(CLOCK_REALTIME, &ts2);
#endif
		fclose(fp);
	} else {
		printf("Failed to open %s\n", PSL_FILE);
		failed++;
	}

	psl_free(psl5);
	psl_free(psl4);
	psl_free(psl3);
	psl_free((psl_ctx_t *)psl2);
	psl_free(psl);
}

int main(int argc, const char * const *argv)
{
#ifdef HAVE_CLOCK_GETTIME
	long ns;
#endif

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

#ifdef HAVE_CLOCK_GETTIME
	if (ts1.tv_sec == ts2.tv_sec)
		ns = ts2.tv_nsec - ts1.tv_nsec;
	else if (ts1.tv_sec == ts2.tv_sec - 1)
		ns = 1000000000L - (ts2.tv_nsec - ts1.tv_nsec);
	else
		ns = 0; /* let's assume something is wrong and skip outputting measured time */

	if (ns)
		printf("Summary: All %d tests passed in %ld.%06ld ms\n", ok, ns / 1000000, ns % 1000000000);
	else
		printf("Summary: All %d tests passed\n", ok);
#else
	printf("Summary: All %d tests passed\n", ok);
#endif

	return 0;
}
