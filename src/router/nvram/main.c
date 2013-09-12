/*
 * Frontend command-line utility for Linux NVRAM layer
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: main.c,v 1.1.1.5 2003/10/29 03:06:28 honor Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <bcmnvram.h>

static void usage(void)
{
	fprintf(stderr,
		"usage: nvram [get name] [set name=value] [unset name] [show]\n");
	exit(0);
}

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#else
#define NVRAMSPACE NVRAM_SPACE
#endif

/* NVRAM utility */
int main(int argc, char **argv)
{
	char *name, *value, buf[NVRAMSPACE];
	int size;

	/* Skip program name */
	--argc;
	++argv;

	if (!*argv)
		usage();

	/* Process the remaining arguments. */
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3)) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		} else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
				strncpy(value = buf, *argv, sizeof(buf));
				name = strsep(&value, "=");
				nvram_set(name, value);
			}
		} else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv)
				nvram_unset(*argv);
		} else if (!strncmp(*argv, "commit", 5)) {
			nvram_commit();
		} else if (!strncmp(*argv, "show", 4)
			   || !strncmp(*argv, "getall", 6)) {
			nvram_getall(buf, sizeof(buf));
			for (name = buf; *name; name += strlen(name) + 1)
				puts(name);
			size =
			    sizeof(struct nvram_header) + (int)name - (int)buf;
			fprintf(stderr, "size: %d bytes (%d left)\n", size,
				NVRAMSPACE - size);
		}
		if (!*argv)
			break;
	}

	return 0;
}
