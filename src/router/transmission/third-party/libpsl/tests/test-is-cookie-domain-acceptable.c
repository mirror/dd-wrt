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
 * Test case for psl_is_cookie_doamin_acceptable()
 *
 * Changelog
 * 15.04.2014  Tim Ruehsen  created from libmget/cookie.c
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
# include <winsock2.h> // WSAStartup, WSACleanup
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
	static const struct test_data {
		const char
			*request_domain,
			*cookie_domain;
		int
			result;
	} test_data[] = {
		{ "www.dkg.forgot.his.name", "www.dkg.forgot.his.name", 1 },
		{ "www.dkg.forgot.his.name", "dkg.forgot.his.name", 1 },
		{ "www.dkg.forgot.his.name", "forgot.his.name", 0 },
		{ "www.dkg.forgot.his.name", "his.name", 0 },
		{ "www.dkg.forgot.his.name", "name", 0 },
		{ "www.his.name", "www.his.name", 1 },
		{ "www.his.name", "his.name", 1 },
		{ "www.his.name", "name", 0 },
		{ "www.example.com", "www.example.com", 1 },
		{ "www.example.com", "wwww.example.com", 0 },
		{ "www.example.com", "example.com", 1 },
		{ "www.example.com", "com", 0 }, /* not accepted by normalization (PSL rule 'com') */
		{ "www.example.com", "example.org", 0 },
		{ "www.sa.gov.au", "sa.gov.au", 0 }, /* not accepted by normalization  (PSL rule '*.ar') */
		{ "www.educ.ar", "educ.ar", 1 }, /* PSL exception rule '!educ.ar' */
		/* RFC6265 5.1.3: Having IP addresses, request and domain IP must be identical */
		{ "192.1.123.2", ".1.123.2", 0 }, /* IPv4 address, partial match */
		{ "192.1.123.2", "192.1.123.2", 1 }, /* IPv4 address, full match */
		{ "::1", "::1", 1 }, /* IPv6 address, full match */
		{ "2a00:1450:4013:c01::8b", ":1450:4013:c01::8b", 0 }, /* IPv6 address, partial match */
		{ "::ffff:192.1.123.2", "::ffff:192.1.123.2", 1 }, /* IPv6 address dotted-quad, full match */
		{ "::ffff:192.1.123.2", ".1.123.2", 0 }, /* IPv6 address dotted-quad, partial match */
		{ NULL, ".1.123.2", 0 },
		{ "hiho", NULL, 0 },
	};
	unsigned it;
	psl_ctx_t *psl;

	psl = psl_load_file(PSL_FILE);

	printf("loaded %d suffixes and %d exceptions\n", psl_suffix_count(psl), psl_suffix_exception_count(psl));

	for (it = 0; it < countof(test_data); it++) {
		const struct test_data *t = &test_data[it];
		int result = psl_is_cookie_domain_acceptable(psl, t->request_domain, t->cookie_domain);

		if (result == t->result) {
			ok++;
		} else {
			failed++;
			printf("psl_is_cookie_domain_acceptable(%s, %s)=%d (expected %d)\n",
				t->request_domain, t->cookie_domain, result, t->result);
		}
	}

	/* do checks to cover more code paths in libpsl */
	psl_is_cookie_domain_acceptable(NULL, "example.com", "example.com");

	psl_free(psl);
}

int main(int argc, const char * const *argv)
{
#ifdef _WIN32
	WSADATA wsa_data;
	int err;

	if ((err = WSAStartup(MAKEWORD(2,2), &wsa_data))) {
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}

	atexit((void (__cdecl*)(void)) WSACleanup);
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

	printf("Summary: All %d tests passed\n", ok + failed);
	return 0;
}
