/*
 * defaults.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

typedef struct NV {
	char *value;
	int index;
	struct NV *next;
};

int hasstored(struct NV *head, char *value)
{
	struct NV *cur = head;
	while ((cur = cur->next) != NULL) {
		if (!strcmp(cur->value, value))
			return cur->index;
	}
	return -1;
}

void recover(void)
{
	FILE *in = fopen("defaults.bin", "rb");
	int len, counts, stores;
	fread(&len, 4, 1, in);	// total count of pairs
	stores = getc(in);	// count of unique values
	unsigned char *index;
	index = malloc(sizeof(char) * len);
	int i;
	fread(index, len, 1, in);

	unsigned char **values = malloc(sizeof(char *) * stores);
	for (i = 0; i < stores; i++) {
		char temp[4096];
		int c;
		int a = 0;
		while ((c = getc(in)) != 0) {
			temp[a++] = c;
		}
		temp[a] = 0;
		values[i] = strdup(temp);
	}
	for (i = 0; i < len; i++) {
		char temp[4096];
		int c;
		int a = 0;
		while ((c = getc(in)) != 0) {
			temp[a++] = c;
		}
		temp[a] = 0;
//              fprintf(stderr, "name %s=%s\n", temp, values[index[i]]);
		if (strcmp(srouter_defaults[i].value, values[index[i]])) {
			fprintf(stderr, "error while validating\n");
			exit(1);
		}
	}
	for (i = 0; i < stores; i++) {
		free(values[i]);
	}
	free(values);
	free(index);

}

int main(int argc, char *argv[])
{
	FILE *out;
	out = fopen("defaults.bin", "wb");
	int i;
	int len = sizeof(srouter_defaults) / sizeof(struct nvram_param);
	struct NV head;
	struct NV *cur = &head;
	memset(&head, 0, sizeof(head));
	int counts = 0;
	int stored = 0;
	for (i = 0; i < len - 1; i++) {
		int f;

		if ((f = hasstored(&head, srouter_defaults[i].value)) == -1) {
			struct NV *next = malloc(sizeof(struct NV));
			memset(next, 0, sizeof(*next));
			next->value = srouter_defaults[i].value;
			next->next = NULL;
//                      fprintf(stderr, "%s: store %s\n", srouter_defaults[i].name, srouter_defaults[i].value);
			next->index = counts++;
			stored++;
			cur->next = next;
			cur = next;
		} else {
                        fprintf(stderr, "%s: reuse %s (%d)\n", srouter_defaults[i].name, srouter_defaults[i].value, f);
		}

	}
//      fprintf(stderr, "stored %d\n", stored);
	len -= 1;
	fwrite(&len, 4, 1, out);	// total count of pairs
	putc(stored, out);	// amount of unique values
	cur = &head;

	for (i = 0; i < len; i++) {
		int v = hasstored(&head, srouter_defaults[i].value);
		if (v == -1) {
			fprintf(stderr, "this should never happen\n");
			exit(-1);
		}
		putc(v, out);
	}

	while ((cur = cur->next) != NULL) {
		fprintf(out, "%s", cur->value);
		putc(0, out);
	}

	for (i = 0; i < len; i++) {
		fprintf(out, "%s", srouter_defaults[i].name);
		putc(0, out);

	}

	fclose(out);
	recover();
	return 0;
}
