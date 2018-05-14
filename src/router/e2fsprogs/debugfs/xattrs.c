/*
 * xattrs.c --- Modify extended attributes via debugfs.
 *
 * Copyright (C) 2014 Oracle.  This file may be redistributed
 * under the terms of the GNU Public License.
 */

#include "config.h"
#include <stdio.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern int optind;
extern char *optarg;
#endif
#include <ctype.h>
#include "support/cstring.h"

#include "debugfs.h"

#define PRINT_XATTR_HEX		0x01
#define PRINT_XATTR_RAW		0x02
#define PRINT_XATTR_C		0x04
#define PRINT_XATTR_STATFMT	0x08
#define PRINT_XATTR_NOQUOTES	0x10

/* Dump extended attributes */
static void print_xattr_hex(FILE *f, const char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
		fprintf(f, "%02x ", (unsigned char)str[i]);
}

/* Dump extended attributes */
static void print_xattr_string(FILE *f, const char *str, int len, int flags)
{
	int printable = 0;
	int i;

	if (flags & PRINT_XATTR_RAW) {
		fwrite(str, len, 1, f);
		return;
	}

	if ((flags & PRINT_XATTR_C) == 0) {
		/* check: is string "printable enough?" */
		for (i = 0; i < len; i++)
			if (isprint(str[i]))
				printable++;

		if (printable <= len*7/8)
			flags |= PRINT_XATTR_HEX;
	}

	if (flags & PRINT_XATTR_HEX) {
		print_xattr_hex(f, str, len);
	} else {
		if ((flags & PRINT_XATTR_NOQUOTES) == 0)
			fputc('\"', f);
		print_c_string(f, str, len);
		if ((flags & PRINT_XATTR_NOQUOTES) == 0)
			fputc('\"', f);
	}
}

static void print_xattr(FILE *f, char *name, char *value, size_t value_len,
			int print_flags)
{
	print_xattr_string(f, name, strlen(name), PRINT_XATTR_NOQUOTES);
	fprintf(f, " (%zu)", value_len);
	if ((print_flags & PRINT_XATTR_STATFMT) &&
	    (strcmp(name, "system.data") == 0))
		value_len = 0;
	if (value_len != 0 &&
	    (!(print_flags & PRINT_XATTR_STATFMT) || (value_len < 40))) {
		fprintf(f, " = ");
		print_xattr_string(f, value, value_len, print_flags);
	}
	fputc('\n', f);
}

static int dump_attr(char *name, char *value, size_t value_len, void *data)
{
	FILE *out = data;

	fprintf(out, "  ");
	print_xattr(out, name, value, value_len, PRINT_XATTR_STATFMT);
	return 0;
}

void dump_inode_attributes(FILE *out, ext2_ino_t ino)
{
	struct ext2_xattr_handle *h;
	size_t sz;
	errcode_t err;

	err = ext2fs_xattrs_open(current_fs, ino, &h);
	if (err)
		return;

	err = ext2fs_xattrs_read(h);
	if (err)
		goto out;

	err = ext2fs_xattrs_count(h, &sz);
	if (err || sz == 0)
		goto out;

	fprintf(out, "Extended attributes:\n");
	err = ext2fs_xattrs_iterate(h, dump_attr, out);
	if (err)
		goto out;

out:
	err = ext2fs_xattrs_close(&h);
}

void do_list_xattr(int argc, char **argv)
{
	ext2_ino_t ino;

	if (argc != 2) {
		printf("%s: Usage: %s <file>\n", argv[0],
		       argv[0]);
		return;
	}

	if (check_fs_open(argv[0]))
		return;

	ino = string_to_inode(argv[1]);
	if (!ino)
		return;

	dump_inode_attributes(stdout, ino);
}

void do_get_xattr(int argc, char **argv)
{
	ext2_ino_t ino;
	struct ext2_xattr_handle *h;
	FILE *fp = NULL;
	char *buf = NULL;
	size_t buflen;
	int i;
	int print_flags = 0;
	unsigned int handle_flags = 0;
	errcode_t err;

	reset_getopt();
	while ((i = getopt(argc, argv, "Cf:rxV")) != -1) {
		switch (i) {
		case 'f':
			if (fp)
				fclose(fp);
			fp = fopen(optarg, "w");
			if (fp == NULL) {
				perror(optarg);
				return;
			}
			break;
		case 'r':
			handle_flags |= XATTR_HANDLE_FLAG_RAW;
			break;
		case 'x':
			print_flags |= PRINT_XATTR_HEX;
			break;
		case 'V':
			print_flags |= PRINT_XATTR_RAW|
				PRINT_XATTR_NOQUOTES;
			break;
		case 'C':
			print_flags |= PRINT_XATTR_C;
			break;
		default:
			goto usage;
		}
	}

	if (optind != argc - 2) {
	usage:
		printf("%s: Usage: %s [-f outfile]|[-xVC] [-r] <file> <attr>\n",
			       argv[0], argv[0]);

		goto out2;
	}

	if (check_fs_open(argv[0]))
		goto out2;

	ino = string_to_inode(argv[optind]);
	if (!ino)
		goto out2;

	err = ext2fs_xattrs_open(current_fs, ino, &h);
	if (err)
		goto out2;

	err = ext2fs_xattrs_flags(h, &handle_flags, NULL);
	if (err)
		goto out;

	err = ext2fs_xattrs_read(h);
	if (err)
		goto out;

	err = ext2fs_xattr_get(h, argv[optind + 1], (void **)&buf, &buflen);
	if (err)
		goto out;

	if (fp) {
		fwrite(buf, buflen, 1, fp);
	} else {
		if (print_flags & PRINT_XATTR_RAW) {
			if (print_flags & (PRINT_XATTR_HEX|PRINT_XATTR_C))
				print_flags &= ~PRINT_XATTR_RAW;
			print_xattr_string(stdout, buf, buflen, print_flags);
		} else {
			print_xattr(stdout, argv[optind + 1],
				    buf, buflen, print_flags);
		}
		printf("\n");
	}

	ext2fs_free_mem(&buf);
out:
	ext2fs_xattrs_close(&h);
	if (err)
		com_err(argv[0], err, "while getting extended attribute");
out2:
	if (fp)
		fclose(fp);
}

void do_set_xattr(int argc, char **argv)
{
	ext2_ino_t ino;
	struct ext2_xattr_handle *h;
	FILE *fp = NULL;
	char *buf = NULL;
	size_t buflen;
	unsigned int handle_flags = 0;
	int i;
	errcode_t err;

	reset_getopt();
	while ((i = getopt(argc, argv, "f:r")) != -1) {
		switch (i) {
		case 'f':
			if (fp)
				fclose(fp);
			fp = fopen(optarg, "r");
			if (fp == NULL) {
				perror(optarg);
				return;
			}
			break;
		case 'r':
			handle_flags |= XATTR_HANDLE_FLAG_RAW;
			break;
		default:
			goto print_usage;
		}
	}

	if (!(fp && optind == argc - 2) && !(!fp && optind == argc - 3)) {
	print_usage:
		printf("Usage:\t%s [-r] <file> <attr> <value>\n", argv[0]);
		printf("\t%s -f <value_file> [-r] <file> <attr>\n", argv[0]);
		goto out2;
	}

	if (check_fs_open(argv[0]))
		goto out2;
	if (check_fs_read_write(argv[0]))
		goto out2;
	if (check_fs_bitmaps(argv[0]))
		goto out2;

	ino = string_to_inode(argv[optind]);
	if (!ino)
		goto out2;

	err = ext2fs_xattrs_open(current_fs, ino, &h);
	if (err)
		goto out2;

	err = ext2fs_xattrs_flags(h, &handle_flags, NULL);
	if (err)
		goto out;

	err = ext2fs_xattrs_read(h);
	if (err)
		goto out;

	if (fp) {
		err = ext2fs_get_mem(current_fs->blocksize, &buf);
		if (err)
			goto out;
		buflen = fread(buf, 1, current_fs->blocksize, fp);
	} else {
		buf = argv[optind + 2];
		buflen = parse_c_string(buf);
	}

	err = ext2fs_xattr_set(h, argv[optind + 1], buf, buflen);
out:
	ext2fs_xattrs_close(&h);
	if (err)
		com_err(argv[0], err, "while setting extended attribute");
out2:
	if (fp) {
		fclose(fp);
		ext2fs_free_mem(&buf);
	}
}

void do_rm_xattr(int argc, char **argv)
{
	ext2_ino_t ino;
	struct ext2_xattr_handle *h;
	int i;
	errcode_t err;

	if (argc < 3) {
		printf("%s: Usage: %s <file> <attrs>...\n", argv[0], argv[0]);
		return;
	}

	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	if (check_fs_bitmaps(argv[0]))
		return;

	ino = string_to_inode(argv[1]);
	if (!ino)
		return;

	err = ext2fs_xattrs_open(current_fs, ino, &h);
	if (err)
		return;

	err = ext2fs_xattrs_read(h);
	if (err)
		goto out;

	for (i = 2; i < argc; i++) {
		err = ext2fs_xattr_remove(h, argv[i]);
		if (err)
			goto out;
	}
out:
	ext2fs_xattrs_close(&h);
	if (err)
		com_err(argv[0], err, "while removing extended attribute");
}
