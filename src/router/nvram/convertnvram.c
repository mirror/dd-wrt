/*
 * nvram_linux_gen.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
 * NVRAM Emulation Library for platforms which cannot support nvram flash based settings in any way
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <bcmtypedefs.h>
#include <bcmnvram.h>
#include <nvram_convert.h>

/* Globals */

static char value[65536];

static int isni = 0;

struct nvrams {
	char *name;
	char *value;
};

struct nvramdb {
	int offsets[('z' + 1) - 'A'];
	int nov;		//number of values;
	struct nvrams *values;
};

static struct nvramdb values;


//simply sort algorithm
void sort(void)
{
	int i, a;
	for (i = 0; i < values.nov; i++)
		for (a = i; a < values.nov; a++) {
			if (values.values[a].name && values.values[i].name) {
				if (values.values[a].name[0] < values.values[i].name[0]) {
					struct nvrams b = values.values[a];
					values.values[a] = values.values[i];
					values.values[i] = b;
				}
			}
		}
}

void lock(void)
{
	FILE *in;
	while ((in = fopen("/tmp/nvram/.lock", "rb")) != NULL) {
		fclose(in);
		sleep(1);
	}
	in = fopen("/tmp/nvram/.lock", "wb");
	fprintf(in, "lock");
	fclose(in);
}

void unlock(void)
{
	unlink("/tmp/nvram/.lock");
}

void writedb(void)
{
	FILE *in;
	in = fopen("/tmp/nvram/nvram.db", "wb");
	if (in == NULL)
		return;
	int c = 0;
	int i;
	for (i = 0; i < ('z' + 1) - 'A'; i++)
		values.offsets[i] = -1;
	for (i = 0; i < values.nov; i++) {
		if (values.values[i].name)
			c++;
	}
	sort();
	putc(c >> 8, in);
	putc(c & 255, in);
	for (i = 0; i < values.nov; i++) {
		if (values.values[i].name) {
			//take a look in our offset table
			int a;
			if (values.offsets[values.values[i].name[0] - 'A'] == -1)
				values.offsets[values.values[i].name[0] - 'A'] = ftell(in);
			int len = strlen(values.values[i].name);
			int fulllen = len + strlen(values.values[i].value) + 3;
			putc(fulllen >> 8, in);
			putc(fulllen & 255, in);
			putc(len, in);
			fwrite(values.values[i].name, len, 1, in);
			len = strlen(values.values[i].value);
			putc(len >> 8, in);
			putc(len & 255, in);
			fwrite(values.values[i].value, len, 1, in);
		}
	}
	fclose(in);
	in = fopen("/tmp/nvram/offsets.db", "wb");
	fwrite(values.offsets, (('z' + 1) - 'A') * 4, 1, in);
	fclose(in);
}

void readdb(void)
{
	FILE *in;
	in = fopen("/tmp/nvram/nvram.db", "rb");
	if (in == NULL) {
		values.nov = 0;
		return;		//first time init;
	}
	values.nov = getc(in) << 8;
	values.nov += getc(in);
	values.values = (struct nvrams *)malloc(values.nov * sizeof(struct nvrams));
	int i;
	for (i = 0; i < values.nov; i++) {
		getc(in);
		getc(in);
		//fseek (in, 2, SEEK_CUR);
		int len = getc(in);
		values.values[i].name = (char *)malloc(len + 1);
		int a;
		fread(values.values[i].name, len, 1, in);
		values.values[i].name[len] = 0;
		len = getc(in) << 8;
		len += getc(in);
		values.values[i].value = (char *)malloc(len + 1);
		fread(values.values[i].value, len, 1, in);
		values.values[i].value[len] = 0;
	}
	fclose(in);
}

void closedb(void)
{
	int i;
	for (i = 0; i < values.nov; i++) {
		if (values.values[i].name)
			free(values.values[i].name);
		if (values.values[i].value)
			free(values.values[i].value);
	}
	if (values.values)
		free(values.values);
	values.nov = -1;
	isni = 0;
}

void main(int argc, char *argv[])
{
	fprintf(stderr, "converting old nvram database....\n");
	lock();
	readdb();
	int i;
	for (i = 0; i < values.nov; i++) {
		nvram_set(values.values[i].name, values.values[i].value);
	}
	closedb();
	unlock();
	nvram_commit();
}
