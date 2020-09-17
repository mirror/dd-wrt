// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include <ctype.h>
#include "block.h"
#include "bmap.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "inode.h"
#include "io.h"
#include "output.h"
#include "init.h"
#include "text.h"

static void     print_rawtext(void *data, int len);

void
print_text(
	const field_t   *fields,
	int             argc,
	char            **argv)
{
	print_rawtext(iocur_top->data, iocur_top->len);
}

static void
print_rawtext(
	void    *data,
	int     len)
{
	int     i;
	int     j;
	int     lastaddr;
	int     offchars;
	unsigned char   *p;

	lastaddr = (len - 1) & ~(16 - 1);
	if (lastaddr < 0x10)
		offchars = 1;
	else if (lastaddr < 0x100)
		offchars = 2;
	else if (lastaddr < 0x1000)
		offchars = 3;
	else
		offchars = 4;

	for (i = 0, p = data; i < len; i += 16) {
		unsigned char *s = p;

		dbprintf("%-0*.*x:  ", offchars, offchars, i);

		for (j = 0; j < 16 && i + j < len; j++, p++) {
			dbprintf("%02x ", *p);
		}

		dbprintf(" ");

		for (j = 0; j < 16 && i + j < len; j++, s++) {
			if (isalnum(*s))
				dbprintf("%c", *s);
			else
				dbprintf(".", *s);
		}

		dbprintf("\n");
	}
}
