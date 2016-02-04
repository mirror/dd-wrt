/*
 * reset_defaults.c
 *
 * Copyright (C) 2008-2016 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

extern struct nvram_param *srouter_defaults;

extern void load_defaults(void);
extern void free_defaults(void);

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#elif HAVE_NVRAM_128
#define NVRAMSPACE 0x20000
#elif HAVE_MVEBU
#define NVRAMSPACE 0x10000
#elif HAVE_IPQ806X
#define NVRAMSPACE 0x10000
#else
#define NVRAMSPACE NVRAM_SPACE
#endif

extern int nvram_critical(char *name);

void start_defaults(void)
{
	fprintf(stderr, "restore nvram to defaults\n");
	char *buf = (char *)malloc(NVRAMSPACE);
	int i;
	struct nvram_param *t;

	nvram_getall(buf, NVRAMSPACE);
	char *p = buf;

	//clean old values
	while (strlen(p) != 0) {
		int len = strlen(p);

		for (i = 0; i < len; i++)
			if (p[i] == '=')
				p[i] = 0;
		char *name = p;

		if (!nvram_critical(name))
			nvram_unset(name);
		p += len + 1;
	}
	load_defaults();
	for (t = srouter_defaults; t->name; t++) {
		nvram_set(t->name, t->value);
	}
	free_defaults();
	free(buf);
	nvram_commit();
}
