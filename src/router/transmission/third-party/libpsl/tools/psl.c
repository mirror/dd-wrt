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
 * This file is part of libpsl.
 *
 * Using the libpsl functions via command line
 *
 * Changelog
 * 11.04.2014  Tim Ruehsen  created
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef _WIN32
# include <winsock2.h> // WSAStartup, WSACleanup
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include <libpsl.h>

static void usage(int err, FILE* f)
{
	fprintf(f, "Usage: psl [options] <domains...>\n");
	fprintf(f, "\n");
	fprintf(f, "Options:\n");
	fprintf(f, "  --version                    show library version information\n");
	fprintf(f, "  --use-latest-data            use the latest PSL data available [default]\n");
	fprintf(f, "  --use-builtin-data           use the builtin PSL data\n");
	fprintf(f, "  --no-star-rule               do not apply the prevailing star rule\n");
	fprintf(f, "                                 (only applies to --is-public-suffix)\n");
	fprintf(f, "  --load-psl-file <filename>   load PSL data from file\n");
	fprintf(f, "  --is-public-suffix           check if domains are public suffixes [default]\n");
	fprintf(f, "  --is-cookie-domain-acceptable <cookie-domain>\n");
	fprintf(f, "                               check if cookie-domain is acceptable for domains\n");
	fprintf(f, "  --print-unreg-domain         print the longest public suffix part\n");
	fprintf(f, "  --print-reg-domain           print the shortest private suffix part\n");
	fprintf(f, "  --print-info                 print info about library builtin data\n");
	fprintf(f, "  -b,  --batch                 don't print leading domain\n");
	fprintf(f, "\n");

	exit(err);
}

static void init_windows(void) {
#ifdef _WIN32
	WSADATA wsa_data;
	int err;

	if ((err = WSAStartup(MAKEWORD(2,2), &wsa_data))) {
		printf("WSAStartup failed with error: %d\n", err);
		exit(EXIT_FAILURE);
	}

	atexit((void (__cdecl*)(void)) WSACleanup);
#endif
}

/* RFC 2822-compliant date format */
static const char *time2str(time_t t)
{
	static char buf[64];
	struct tm *tp = localtime(&t);

	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", tp);
	return buf;
}

int main(int argc, const char *const *argv)
{
	int mode = 1, no_star_rule = 0, batch_mode = 0;
	const char *const *arg, *psl_file = NULL, *cookie_domain = NULL;
	psl_ctx_t *psl = (psl_ctx_t *) psl_latest(NULL);

	/* set current locale according to the environment variables */
	setlocale(LC_ALL, "");

	for (arg = argv + 1; arg < argv + argc; arg++) {
		if (**arg == '-') {
			if (!strcmp(*arg, "--is-public-suffix"))
				mode = 1;
			else if (!strcmp(*arg, "--print-unreg-domain"))
				mode = 2;
			else if (!strcmp(*arg, "--print-reg-domain"))
				mode = 3;
			else if (!strcmp(*arg, "--print-info"))
				mode = 99;
			else if (!strcmp(*arg, "--is-cookie-domain-acceptable") && arg < argv + argc - 1) {
				mode = 4;
				cookie_domain = *(++arg);
			}
			else if (!strcmp(*arg, "--use-latest-data")) {
				psl_free(psl);
				if (psl_file) {
					fprintf(stderr, "Dropped data from %s\n", psl_file);
					psl_file = NULL;
				}
				if (!(psl = (psl_ctx_t *) psl_latest(NULL)))
					printf("No PSL data available\n");
			}
			else if (!strcmp(*arg, "--use-builtin-data")) {
				psl_free(psl);
				if (psl_file) {
					fprintf(stderr, "Dropped data from %s\n", psl_file);
					psl_file = NULL;
				}
				if (!(psl = (psl_ctx_t *) psl_builtin()))
					printf("No builtin PSL data available\n");
			}
			else if (!strcmp(*arg, "--no-star-rule")) {
				no_star_rule = 1;
			}
			else if (!strcmp(*arg, "--load-psl-file") && arg < argv + argc - 1) {
				psl_free(psl);
				if (psl_file) {
					fprintf(stderr, "Dropped data from %s\n", psl_file);
					psl_file = NULL;
				}
				if (!(psl = psl_load_file(psl_file = *(++arg)))) {
					fprintf(stderr, "Failed to load PSL data from %s\n\n", psl_file);
					psl_file = NULL;
				}
			}
			else if (!strcmp(*arg, "--batch") || !strcmp(*arg, "-b")) {
				batch_mode = 1;
			}
			else if (!strcmp(*arg, "--help")) {
				fprintf(stdout, "`psl' explores the Public Suffix List\n\n");
				usage(0, stdout);
			}
			else if (!strcmp(*arg, "--version")) {
				printf("psl %s (0x%06x)\n", PACKAGE_VERSION, psl_check_version_number(0));
				printf("libpsl %s\n", psl_get_version());
				printf("\n");
				printf("Copyright (C) 2014-2018 Tim Ruehsen\n");
				printf("License: MIT\n");
				exit(0);
			}
			else if (!strcmp(*arg, "--")) {
				arg++;
				break;
			}
			else {
				fprintf(stderr, "Unknown option '%s'\n", *arg);
				usage(1, stderr);
			}
		} else
			break;
	}

	if (mode != 99) {
		if (mode != 1 && no_star_rule) {
			fprintf(stderr, "--no-star-rule only combines with --is-public-suffix\n");
			usage(1, stderr);
		}
		if (!psl) {
			fprintf(stderr, "No PSL data available - aborting\n");
			exit(2);
		}
		if (arg >= argv + argc) {
			char buf[256], *domain, *lower;
			size_t len;
			psl_error_t rc;

			/* read URLs from STDIN */
			while (fgets(buf, sizeof(buf), stdin)) {
				for (domain = buf; isspace(*domain); domain++); /* skip leading spaces */
				if (*domain == '#' || !*domain) continue; /* skip empty lines and comments */
				for (len = strlen(domain); len && isspace(domain[len - 1]); len--); /* skip trailing spaces */
				domain[len] = 0;

				if ((rc = psl_str_to_utf8lower(domain, NULL, NULL, &lower)) != PSL_SUCCESS) {
					fprintf(stderr, "%s: Failed to convert to lowercase UTF-8 (%d)\n", domain, rc);
					continue;
				}

				if (!batch_mode && mode != 4)
					printf("%s: ", domain);

				if (mode == 1) {
					if (no_star_rule)
						printf("%d", psl_is_public_suffix2(psl, lower, PSL_TYPE_ANY|PSL_TYPE_NO_STAR_RULE));
					else
						printf("%d", psl_is_public_suffix(psl, lower));

					if (!batch_mode)
						printf(" (%s)\n", lower);
					else
						putchar('\n');
				}
				else if (mode == 2) {
					const char *dom = psl_unregistrable_domain(psl, lower);
					printf("%s\n", dom ? dom : "(null)");
				}
				else if (mode == 3) {
					const char *dom = psl_registrable_domain(psl, lower);
					printf("%s\n", dom ? dom : "(null)");
				}
				else if (mode == 4) {
					char *cookie_domain_lower;

					init_windows();

					if ((rc = psl_str_to_utf8lower(domain, NULL, NULL, &cookie_domain_lower)) == PSL_SUCCESS) {
						if (!batch_mode)
							printf("%s: ", domain);
						printf("%d\n", psl_is_cookie_domain_acceptable(psl, lower, cookie_domain));
						free(cookie_domain_lower);
					} else
						fprintf(stderr, "%s: Failed to convert cookie domain '%s' to lowercase UTF-8 (%d)\n", domain, cookie_domain, rc);
				}

				psl_free_string(lower);
			}

			psl_free(psl);
			exit(0);
		}
	}

	if (mode == 1) {
		for (; arg < argv + argc; arg++) {
			if (!batch_mode)
				printf("%s: ", *arg);
			if (no_star_rule)
				printf("%d\n", psl_is_public_suffix2(psl, *arg, PSL_TYPE_ANY|PSL_TYPE_NO_STAR_RULE));
			else
				printf("%d\n", psl_is_public_suffix(psl, *arg));
		}
	}
	else if (mode == 2) {
		for (; arg < argv + argc; arg++) {
			const char *dom = psl_unregistrable_domain(psl, *arg);
			if (!batch_mode)
				printf("%s: ", *arg);
			printf("%s\n", dom ? dom : "(null)");
		}
	}
	else if (mode == 3) {
		for (; arg < argv + argc; arg++) {
			const char *dom = psl_registrable_domain(psl, *arg);
			if (!batch_mode)
				printf("%s: ", *arg);
			printf("%s\n", dom ? dom : "(null)");
		}
	}
	else if (mode == 4) {
		init_windows();

		for (; arg < argv + argc; arg++) {
			if (!batch_mode)
				printf("%s: ", *arg);
			printf("%d\n", psl_is_cookie_domain_acceptable(psl, *arg, cookie_domain));
		}
	}
	else if (mode == 99) {
		printf("dist filename: %s\n", psl_dist_filename());

		if (psl && psl != psl_builtin()) {
			static char not_avail[] = "- information not available -";
			int n;

			if ((n = psl_suffix_count(psl)) >= 0)
				printf("suffixes: %d\n", n);
			else
				printf("suffixes: %s\n", not_avail);

			if ((n = psl_suffix_exception_count(psl)) >= 0)
				printf("exceptions: %d\n", n);
			else
				printf("exceptions: %s\n", not_avail);

			if ((n = psl_suffix_wildcard_count(psl)) >= 0)
				printf("wildcards: %d\n", n);
			else
				printf("wildcards: %s\n", not_avail);
		}

		psl_free(psl);
		psl = (psl_ctx_t *) psl_builtin();

		if (psl) {
			printf("builtin suffixes: %d\n", psl_suffix_count(psl));
			printf("builtin exceptions: %d\n", psl_suffix_exception_count(psl));
			printf("builtin wildcards: %d\n", psl_suffix_wildcard_count(psl));
			printf("builtin filename: %s\n", psl_builtin_filename());
			printf("builtin file time: %ld (%s)\n", (long) psl_builtin_file_time(), time2str(psl_builtin_file_time()));
			printf("builtin SHA1 file hash: %s\n", psl_builtin_sha1sum());
			printf("builtin outdated: %d\n", psl_builtin_outdated());
		} else
			printf("No builtin PSL data available\n");
	}

	psl_free(psl);

	return 0;
}
