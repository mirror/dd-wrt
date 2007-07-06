/*
 * bindings.c	     Key Bindings
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/bindings.h>
#include <bmon/utils.h>

static binding_t *bindings;

void add_binding(binding_t *b)
{
	b->next = bindings;
	bindings = b;
}

static void run_binding(binding_t *b, const char *args_)
{
	pid_t pid;
	int i, n;
	char *s, *args = strdup(args_);

	for (i = 1; b->args[i]; i++);
	n = i;
	
	if (i < 255)
	b->args[i++] = args;
	
	for (s = args; i < 255; i++) {
		s = strchr (s, ' ');
		if (s) {
			*s = '\0';
			s++;
			b->args[i] = s;
		} else
			break;
	}

	b->args[i] = NULL;

	pid = fork();
	if (pid < 0)
		quit("fork error: %s\n", strerror(errno));
	else if (pid) {
		/* parent */
		b->args[n] = NULL;
		return;
	}
	
	/* child */

#ifdef HAVE_FCLOSEALL
	fcloseall();
#else
	for (i = 0; i < 256; i++)
		close(i);
#endif
	
	if (execve(b->cmd, b->args, NULL) < 0)
		quit("execve failed: %s\n", strerror(errno));
	
	/* this is only needed in case execve fails */
	_exit(0);
}

int handle_bindings(int ch, const char *args)
{
	binding_t *b;

	for (b = bindings; b; b = b->next) {
		if (ch == b->ch) {
			run_binding(b, args);
			return 1;
		}
	}

	return 0;
}
