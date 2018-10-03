/*
 * main.c
 * Main entry point and global utility functions.
 *
 * Copyright (c) 2003 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#include "global.h"

#ifdef USE_MACOS_TYPE
#include <CoreServices/CoreServices.h>
#endif

/*
 * local functions
 */

void analyze_file(const char *filename);
void analyze_stdin(void);
int analyze_stat(struct stat *sb, const char *filename);
void analyze_fd(int fd, int filekind, const char *filename);
void print_kind(int filekind, u8 size, int size_known);

#ifdef USE_MACOS_TYPE
static void show_macos_type(const char *filename);
#endif

/*
 * entry point
 */

int main(int argc, char *argv[])
{
	int i;

	/* argument check */
	if (argc < 2) {
		if (isatty(0)) {
			fprintf(stderr, "Usage: %s <device/file>...\n", PROGNAME);
			return 1;
		} else {
			print_line(0, "");
			analyze_stdin();
		}
	}

	/* loop over filenames */
	print_line(0, "");
	for (i = 1; i < argc; i++) {
		analyze_file(argv[i]);
		print_line(0, "");
	}

	return 0;
}
/*
 * Mac OS type & creator code
 */

#ifdef USE_MACOS_TYPE

static void show_macos_type(const char *filename)
{
	int err;
	FSRef ref;
	FSCatalogInfo info;
	FInfo *finfo;

	err = FSPathMakeRef(filename, &ref, NULL);
	if (err == 0) {
		err = FSGetCatalogInfo(&ref, kFSCatInfoFinderInfo, &info, NULL, NULL, NULL);
	}

	if (err == 0) {
		finfo = (FInfo *) (info.finderInfo);
		if (finfo->fdType != 0 || finfo->fdCreator != 0) {
			char typecode[5], creatorcode[5], s1[256], s2[256];

			memcpy(typecode, &finfo->fdType, 4);
			typecode[4] = 0;
			format_ascii(typecode, s1);

			memcpy(creatorcode, &finfo->fdCreator, 4);
			creatorcode[4] = 0;
			format_ascii(creatorcode, s2);

			print_line(0, "Type code \"%s\", creator code \"%s\"", s1, s2);
		} else {
			print_line(0, "No type and creator code");
		}
	}
	if (err) {
		print_line(0, "Type and creator code unknown (error %d)", err);
	}
}

#endif

/* EOF */
