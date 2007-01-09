/*
 *  GPLv2
 *  Copyright 2003, Glenn McGrath <bug1@iinet.net.au>
 *  Copyright 2006, Pavel Roskin <proski@gnu.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation; either version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Based on specification from
 *  http://www.opengroup.org/onlinepubs/007904975/utilities/uuencode.html
 *
 *  Bugs: the spec doesn't mention anything about "`\n`\n" prior to the "end" line
 */


#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

static void uudecode_usage(void)
{
	printf("Usage: uudecode [-o OUTFILE] [INFILE]\n");
}

static char *get_line_from_file(FILE *file)
{
	int ch;
	int idx = 0;
	static char linebuf[80];

	while ((ch = getc(file)) != EOF) {
		linebuf[idx++] = (char)ch;
		if (!ch)
			return linebuf;
		if (ch == '\n') {
			--idx;
			break;
		}
		/* Dumb overflow protection */
		if (idx >= (int)sizeof(linebuf))
			idx--;
	}
	if (ferror(file))
		return NULL;

	linebuf[idx] = 0;
	if (idx > 0 && linebuf[idx - 1] == '\r')
		linebuf[idx - 1] = 0;

	return linebuf;
}

#define char_val(n) ((line_ptr[n] - 0x20) & 0x3f)

static void read_stduu(FILE *src_stream, FILE *dst_stream)
{
	char *line;

	while ((line = get_line_from_file(src_stream)) != NULL) {
		int length;
		char *line_ptr = line;

		if (strcmp(line, "end") == 0)
			return;
		length = char_val(0) * 4 / 3;

		/* Ignore the "`\n" line, why is it even in the encode file ? */
		if (length <= 0)
			continue;

		if (length > 60) {
			fprintf(stderr, "uudecode: Line too long\n");
			exit(1);
		}

		line_ptr++;
		/* Tolerate an overly long line to accommodate an extra '`' */
		if ((int)strlen(line_ptr) < length) {
			fprintf(stderr, "uudecode: Short line detected\n");
			exit(1);
		}

 		while (length > 0) {
			/* Merge four 6 bit chars to three 8 bit chars */
			fputc(char_val(0) << 2 | char_val(1) >> 4, dst_stream);
			line_ptr++;
			if (--length == 0)
				break;

   			fputc(char_val(0) << 4 | char_val(1) >> 2, dst_stream);
			line_ptr++;
			if (--length == 0)
				break;

  			fputc(char_val(0) << 6 | char_val(1), dst_stream);
			line_ptr += 2;
			length -= 2;
		}
	}
	fprintf(stderr, "uudecode: no `end' found\n");
	exit(1);
}

int main(int argc, char **argv)
{
	FILE *src_stream;
	FILE *dst_stream = NULL;
	char *outname = NULL;
	char *line;
	int mode;
	char *line_ptr = NULL;
	int c;
	int forced_output = 0;

	while ((c = getopt (argc, argv, "o:")) != -1)
		switch (c) {
		case 'o':
			forced_output = 1;
			outname = optarg;
			break;
		default:
			uudecode_usage();
			exit(1);
		}

	if (optind == argc) {
		src_stream = stdin;
	} else if (optind + 1 == argc) {
		src_stream = fopen(argv[optind], "rt");
		if (!src_stream) {
			fprintf(stderr, "uudecode: Cannot open \"%s\": %s\n",
			        argv[optind], strerror(errno));
			exit(1);
		}
	} else {
		uudecode_usage();
		exit(1);
	}

	/* Search for the start of the encoding */
	while ((line = get_line_from_file(src_stream)) != NULL) {
		if (strncmp(line, "begin ", 6) == 0) {
			line_ptr = line + 6;
			break;
		}
	}

	if (!line_ptr) {
		fprintf(stderr, "uudecode: No `begin' line\n");
		exit(1);
	}

	mode = strtoul(line_ptr, NULL, 8);
	if (outname == NULL) {
		outname = strchr(line_ptr, ' ');
		if ((outname == NULL) || (*outname == '\0')) {
			fprintf(stderr, "uudecode: No file name specified\n");
			exit(1);
		}
		outname++;
	}

	if (forced_output && (strcmp(outname, "-") == 0)) {
		dst_stream = stdout;
	} else {
		int fd;
		int flags = O_WRONLY | O_CREAT | O_TRUNC;

		/* don't clobber files without an explicit "-o" */
		if (!forced_output)
			flags |= O_EXCL;

		fd = open(outname, flags,
			  mode & (S_IRWXU | S_IRWXG | S_IRWXO));
		if (fd != -1)
			dst_stream = fdopen(fd, "wb");
		if ((fd == -1) || !dst_stream) {
			fprintf(stderr, "uudecode: Cannot open \"%s\": %s\n",
			        outname, strerror(errno));
			exit(1);
		}
	}

	read_stduu(src_stream, dst_stream);
	if (src_stream != stdin)
		fclose(src_stream);
	if (dst_stream != stdout)
		fclose(dst_stream);
	return 0;
}
