/*
 * Copyright (C) 2001 MontaVista Software Inc.
 * Author: Jun Sun, jsun@mvista.com or jsun@junsun.net
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include <linux/kernel.h>

#include <asm/uaccess.h>
#include <ramconfig.h>
#include <uncompress.h>

#include "printf.h"
#include "print.h"

static void myoutput(void *arg, char *s, int l)
{
	int i;

	// special termination call
	if ((l == 1) && (s[0] == '\0'))
		return;

	for (i = 0; i < l; i++) {
		if (s[i] == '\n')
			putc('\r');
		putc(s[i]);
	}
}

static void printf(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	lp_Print(myoutput, 0, fmt, ap);
	va_end(ap);
}
