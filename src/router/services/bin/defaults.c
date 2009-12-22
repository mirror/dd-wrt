/*
 * defaults.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
 * small helper utility to generate a defaults database file in /etc
 * $Id:
 */


#define STORE_DEFAULTS

#include "../sysinit/defaults.c"

int main(int argc, char *argv[])
{
	FILE *out;
	out = fopen("defaults.bin", "wb");
	int i;
	int len = sizeof(srouter_defaults) / sizeof(struct nvram_tuple);
	fwrite(&len, 4, 1, out);
	for (i = 0; i < sizeof(srouter_defaults) / sizeof(struct nvram_tuple);
	     i++) {
		if (srouter_defaults[i].name) {
			putc(strlen(srouter_defaults[i].name), out);
			fwrite(srouter_defaults[i].name,
			       strlen(srouter_defaults[i].name), 1, out);
			len = strlen(srouter_defaults[i].value);
			if (len > 127) {
				len |= 128;
				putc(len & 0xff, out);
				putc(strlen(srouter_defaults[i].value) >> 7,
				     out);
			} else {
				putc(len, out);
			}
			fwrite(srouter_defaults[i].value,
			       strlen(srouter_defaults[i].value), 1, out);
		} else {
			putc(0, out);
			putc(0, out);
		}

	}
	fclose(out);
	return 0;
}
