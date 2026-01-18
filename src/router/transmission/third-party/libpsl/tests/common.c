/*
 * Copyright(c) 2014-2024 Tim Ruehsen
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
 */

#include <stdio.h> // snprintf
#include <stdlib.h> // exit, system
#include <string.h> // strlen
#if defined _WIN32
#	include <malloc.h>
#endif

int run_valgrind(const char *valgrind, const char *executable)
{
	char cmd[BUFSIZ];
	int n, rc;

	n = snprintf(cmd, sizeof(cmd), "TESTS_VALGRIND="" %s %s", valgrind, executable);
	if ((unsigned)n >= sizeof(cmd)) {
		printf("Valgrind command line is too long (>= %u)\n", (unsigned) sizeof(cmd));
		return EXIT_FAILURE;
	}

	if ((rc = system(cmd))) {
		printf("Failed to execute with '%s' (system() returned %d)\n", valgrind, rc);
	}

	return rc ? EXIT_FAILURE : EXIT_SUCCESS;
}
