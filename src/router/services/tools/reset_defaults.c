/*
 * reset_defaults.c
 *
 * Copyright (C) 2008-2016 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <shutils.h>
#include <wlutils.h>
#include <bcmnvram.h>

extern struct nvram_param *load_defaults(void);
extern void free_defaults(struct nvram_param *);

extern int nvram_critical(char *name);

int nvram_size(void);
void start_defaults(void)
{
	struct nvram_param *srouter_defaults;
	struct nvram_param *t;
	fprintf(stderr, "restore nvram to defaults\n");
	nvram_clear();
	srouter_defaults = load_defaults();
	for (t = srouter_defaults; t->name; t++) {
		nvram_set(t->name, t->value);
	}
	free_defaults(srouter_defaults);
	nvram_commit();
}
