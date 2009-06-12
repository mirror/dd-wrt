/*
 * This is included by init/main.c to check for architecture-dependent bugs.
 *
 * Needs:
 *	void check_bugs(void);
 *
 * Copyright (C) 1998, 1999 Hewlett-Packard Co
 * Copyright (C) 1998, 1999 David Mosberger-Tang <davidm@hpl.hp.com>
 */

#include <asm/processor.h>

/*
 * I don't know of any ia-64 bugs yet..
 */
static void
check_bugs (void)
{
}
