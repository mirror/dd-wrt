/*
 * Copyright (c) 2013 Red Hat, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

#include <sys/types.h>
#include <dirent.h>

static struct cmdinfo readdir_cmd;

const char *d_type_str(unsigned int type)
{
	const char *str;

	switch (type) {
	case DT_UNKNOWN:
		str = "DT_UNKNOWN";
		break;
	case DT_FIFO:
		str = "DT_FIFO";
		break;
	case DT_CHR:
		str = "DT_CHR";
		break;
	case DT_DIR:
		str = "DT_DIR";
		break;
	case DT_BLK:
		str = "DT_BLK";
		break;
	case DT_REG:
		str = "DT_REG";
		break;
	case DT_LNK:
		str = "DT_LNK";
		break;
	case DT_SOCK:
		str = "DT_SOCK";
		break;
	case DT_WHT:
		str = "DT_WHT";
		break;
	default:
		str = "ERROR!";
		break;
	}

	return str;
}

static void
dump_dirent(
	long long offset,
	struct dirent *dirent)
{
	printf("%08llx: d_ino: 0x%08lx", offset, dirent->d_ino);
#ifdef _DIRENT_HAVE_D_OFF
	printf(" d_off: 0x%08lx", dirent->d_off);
#endif
#ifdef _DIRENT_HAVE_D_RECLEN
	printf(" d_reclen: 0x%x", dirent->d_reclen);
#endif
#ifdef _DIRENT_HAVE_D_TYPE
	printf(" d_type: %s", d_type_str(dirent->d_type));
#endif
	printf(" d_name: %s\n", dirent->d_name);
}

static int
read_directory(
	DIR *dir,
	long long offset,
	unsigned long long length,
	int dump,
	unsigned long long *total)
{
	struct dirent *dirent;
	int count = 0;

	seekdir(dir, offset);

	*total = 0;
	while (*total < length) {
		dirent = readdir(dir);
		if (!dirent)
			break;

#ifdef _DIRENT_HAVE_D_RECLEN
		*total += dirent->d_reclen;
#else
		*total += dirent->d_namlen + sizeof(*dirent);
#endif
		count++;

		if (dump) {
			dump_dirent(offset, dirent);
#ifdef _DIRENT_HAVE_D_OFF
			offset = dirent->d_off;
#else
			/* Some platforms don't have dirent->d_off, but because
			 * it is used only for dumping the value, it should be
			 * safe to only set it to zero in such case.
			 */
			offset = 0;
#endif
		}
	}

	return count;
}

static int
readdir_f(
	int argc,
	char **argv)
{
	int cnt;
	unsigned long long total;
	int c;
	size_t fsblocksize, fssectsize;
	struct timeval t1, t2;
	char s1[64], s2[64], ts[64];
	long long offset = -1;
	unsigned long long length = -1;		/* max length limit */
	int verbose = 0;
	DIR *dir;
	int dfd;

	init_cvtnum(&fsblocksize, &fssectsize);

	while ((c = getopt(argc, argv, "l:o:v")) != EOF) {
		switch (c) {
		case 'l':
			length = cvtnum(fsblocksize, fssectsize, optarg);
			break;
		case 'o':
			offset = cvtnum(fsblocksize, fssectsize, optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			return command_usage(&readdir_cmd);
		}
	}

	dfd = dup(file->fd);
	if (dfd < 0)
		return -1;

	dir = fdopendir(dfd);
	if (!dir) {
		close(dfd);
		return -1;
	}

	if (offset == -1) {
		rewinddir(dir);
		offset = telldir(dir);
	}

	gettimeofday(&t1, NULL);
	cnt = read_directory(dir, offset, length, verbose, &total);
	gettimeofday(&t2, NULL);

	closedir(dir);
	close(dfd);

	t2 = tsub(t2, t1);
	timestr(&t2, ts, sizeof(ts), 0);

	cvtstr(total, s1, sizeof(s1));
	cvtstr(tdiv(total, t2), s2, sizeof(s2));

	printf(_("read %llu bytes from offset %lld\n"), total, offset);
	printf(_("%s, %d ops, %s (%s/sec and %.4f ops/sec)\n"),
		s1, cnt, ts, s2, tdiv(cnt, t2));

	return 0;
}

void
readdir_init(void)
{
	readdir_cmd.name = "readdir";
	readdir_cmd.cfunc = readdir_f;
	readdir_cmd.argmax = 5;
	readdir_cmd.flags = CMD_NOMAP_OK|CMD_FOREIGN_OK;
	readdir_cmd.args = _("[-v][-o offset][-l length]");
	readdir_cmd.oneline = _("read directory entries");

	add_command(&readdir_cmd);
}
