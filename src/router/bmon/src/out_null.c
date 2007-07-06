/*
 * out_null.c            Null Output
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
#include <bmon/output.h>
#include <bmon/utils.h>

static void
null_draw(void)
{
}

static int
null_probe(void)
{
	return 1;
}

static void
print_help(void)
{
	printf(
		"null - No output\n" \
		"\n" \
		"  Disable primary output method\n" \
		"  Author: Thomas Graf <tgraf@suug.ch>\n" \
		"\n");
}

static void
null_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static struct output_module null_ops = {
	.om_name = "null",
	.om_draw = null_draw,
	.om_probe = null_probe,
	.om_set_opts = null_set_opts,
};

#if defined PRAGMA_FALLBACK
#pragma init (null_init)
#endif

static void __init
null_init(void)
{
	register_output_module(&null_ops);
}
