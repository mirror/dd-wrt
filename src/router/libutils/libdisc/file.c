/*
 * file.c
 * Data source for files and block devices.
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

#define USE_BINARY_SEARCH 0
#define DEBUG_SIZE 0

#ifdef USE_IOCTL_LINUX
#include <sys/ioctl.h>
#include <linux/fs.h>
#endif

#ifdef USE_IOCTL_FREEBSD
#include <sys/disklabel.h>
#endif

#ifdef USE_IOCTL_DARWIN
#include <sys/ioctl.h>
#include <sys/disk.h>
#endif

/*
 * types
 */

typedef struct file_source {
	SOURCE c;
	int fd;
} FILE_SOURCE;

/*
 * helper functions
 */

void analyze_fd(int fd, int filekind, const char *filename);
static void determine_file_size(FILE_SOURCE *fs, int filekind);
int analyze_stat(struct stat *sb, const char *filename);

static int analyse_file(SOURCE *s, int level);
void analyze_stdin(void);
static u8 read_file(SOURCE *s, u8 pos, u8 len, void *buf);
static void close_file(SOURCE *s);

#if USE_BINARY_SEARCH
static int check_position(int fd, u8 pos);
#endif

/*
 * initialize the file source
 */

SOURCE *init_file_source(int fd, int filekind)
{
	FILE_SOURCE *fs;

	fs = (FILE_SOURCE *)malloc(sizeof(FILE_SOURCE));
	if (fs == NULL)
		bailout("Out of memory");
	memset(fs, 0, sizeof(FILE_SOURCE));

	if (filekind >= 3) /* pipe or similar, non-seekable */
		fs->c.sequential = 1;
	else if (filekind != 0) /* special treatment hook for devices */
		fs->c.analyze = analyse_file;
	fs->c.read_bytes = read_file;
	fs->c.close = close_file;
	fs->fd = fd;

	if (!fs->c.sequential)
		determine_file_size(fs, filekind);

	return (SOURCE *)fs;
}

static void determine_file_size(FILE_SOURCE *fs, int filekind)
{
	off_t result;
	int fd = fs->fd;

	/*
	 * Determine the size using various methods. The first method that
	 * works is used.
	 */

	/*
	 * lseek() to the end:
	 * Works on files. On some systems (Linux), this also works on devices.
	 */
	if (!fs->c.size_known) {
		result = lseek(fd, 0, SEEK_END);
#if DEBUG_SIZE
		printf("Size: lseek returned %lld\n", result);
#endif
		if (result > 0) {
			fs->c.size_known = 1;
			fs->c.size = result;
		}
	}
#ifdef USE_IOCTL_LINUX
	/*
	 * ioctl, Linux style:
	 * Works on certain devices.
	 */
#ifdef BLKGETSIZE64
#define u64 __u64 /* workaround for broken header file */
	if (!fs->c.size_known && filekind != 0) {
		u8 devsize;
		if (ioctl(fd, BLKGETSIZE64, (void *)&devsize) >= 0) {
			fs->c.size_known = 1;
			fs->c.size = devsize;
#if DEBUG_SIZE
			printf("Size: Linux 64-bit ioctl reports %llu\n", fs->c.size);
#endif
		}
	}
#undef u64
#endif

	if (!fs->c.size_known && filekind != 0) {
		u4 blockcount;
		if (ioctl(fd, BLKGETSIZE, (void *)&blockcount) >= 0) {
			fs->c.size_known = 1;
			fs->c.size = (u8)blockcount * 512;
#if DEBUG_SIZE
			printf("Size: Linux 32-bit ioctl reports %llu (%lu blocks)\n", fs->c.size, blockcount);
#endif
		}
	}
#endif

#ifdef USE_IOCTL_FREEBSD
	/*
	 * ioctl, FreeBSD style:
	 * Works on partitioned hard disks or somthing like that.
	 */
	if (!fs->c.size_known && filekind != 0) {
		struct disklabel dl;
		if (ioctl(fd, DIOCGDINFO, &dl) >= 0) {
			fs->c.size_known = 1;
			fs->c.size = (u8)dl.d_ncylinders * dl.d_secpercyl * dl.d_secsize;
			/* TODO: check this, it's the whole disk size... */
#if DEBUG_SIZE
			printf("Size: FreeBSD ioctl reports %llu\n", fs->c.size);
#endif
		}
	}
#endif

#ifdef USE_IOCTL_DARWIN
	/*
	 * ioctl, Darwin style:
	 * Works on certain devices.
	 */
	if (!fs->c.size_known && filekind != 0) {
		u4 blocksize;
		u8 blockcount;
		if (ioctl(fd, DKIOCGETBLOCKSIZE, (void *)&blocksize) >= 0) {
			if (ioctl(fd, DKIOCGETBLOCKCOUNT, (void *)&blockcount) >= 0) {
				fs->c.size_known = 1;
				fs->c.size = blockcount * blocksize;
#if DEBUG_SIZE
				printf("Size: Darwin ioctl reports %llu (%llu blocks of %lu bytes)\n", fs->c.size, blockcount,
				       blocksize);
#endif
			}
		}
	}
#endif

	/*
	 * Check if we can seek at all, and turn on sequential reads if not. This
	 * effectively detects "real" character devices. For pipes and sockets we
	 * already set the flag and don't even run this function, because we
	 * expect them to be non-seekable and not have a size.
	 */
	if (!fs->c.size_known) {
		result = lseek(fd, 1, SEEK_SET);
		if (result != 1)
			fs->c.sequential = 1;
	}
#if USE_BINARY_SEARCH
	/*
	 * binary search:
	 * Works on anything that can seek, but is quite expensive.
	 */
	if (!fs->c.size_known && !fs->c.sequential) {
		u8 lower, upper, current;

		/* TODO: check that the target can seek at all */

#if DEBUG_SIZE
		printf("Size: Doing a binary search\n");
#endif

		/* first, find an upper bound starting from a reasonable guess */
		lower = 0;
		upper = 1024 * 1024; /* start with 1MB */
		while (check_position(fd, upper)) {
			lower = upper;
			upper <<= 2;
		}

		/* second, nail down the size between the lower and upper bounds */
		while (upper > lower + 1) {
			current = (lower + upper) >> 1;
			if (check_position(fd, current))
				lower = current;
			else
				upper = current;
		}
		fs->c.size_known = 1;
		fs->c.size = lower + 1;

#if DEBUG_SIZE
		printf("Size: Binary search reports %llu\n", fs->c.size);
#endif
	}
#endif
}

/*
 * special handling hook: devices may have out-of-band structure
 */

static int analyse_file(SOURCE *s, int level)
{
	if (analyze_cdaccess(((FILE_SOURCE *)s)->fd, s, level))
		return 1;

	return 0;
}

/*
 * raw read
 */

static u8 read_file(SOURCE *s, u8 pos, u8 len, void *buf)
{
	off_t result_seek;
	ssize_t result_read;
	char *p;
	u8 got;
	int fd = ((FILE_SOURCE *)s)->fd;

	/* seek to the requested position (unless we're a pipe) */
	if (!s->sequential) {
		result_seek = lseek(fd, pos, SEEK_SET);
		if (result_seek != pos) {
			errore("Seek to %llu failed", pos);
			return 0;
		}
	}

	/* read from there */
	p = (char *)buf;
	got = 0;
	while (len > 0) {
		result_read = read(fd, p, len);
		if (result_read < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			errore("Data read failed at position %llu", pos + got);
			break;
		} else if (result_read == 0) {
			/* simple EOF, no message */
			break;
		} else {
			len -= result_read;
			got += result_read;
			p += result_read;
		}
	}

	return got;
}

/*
 * dispose of everything
 */

static void close_file(SOURCE *s)
{
	int fd = ((FILE_SOURCE *)s)->fd;

	if (fd > 2) /* don't close stdin/out/err */
		close(fd);
}

/*
 * check if the given position is inside the file's size
 */

#if USE_BINARY_SEARCH
static int check_position(int fd, u8 pos)
{
	char buf[2];

#if DEBUG_SIZE
	printf("      Probing %llu\n", pos);
#endif

	if (lseek(fd, pos, SEEK_SET) != pos)
		return 0;
	if (read(fd, buf, 1) != 1)
		return 0;
	return 1;
}
#endif

void analyze_file(const char *filename)
{
	int fd, filekind;
	struct stat sb;

	/* accept '-' as an alias for stdin */
	if (strcmp(filename, "-") == 0) {
		analyze_stdin();
		return;
	}

	print_line(0, "--- %s", filename);

	/* stat check */
	if (stat(filename, &sb) < 0) {
		errore("Can't stat %.300s", filename);
		return;
	}
	filekind = analyze_stat(&sb, filename);
	if (filekind < 0)
		return;

		/* Mac OS type & creator code (if running on Mac OS X) */
#ifdef USE_MACOS_TYPE
	if (filekind == 0)
		show_macos_type(filename);
#endif

	/* open for reading */
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		errore("Can't open %.300s", filename);
		return;
	}

	/* go for it */
	analyze_fd(fd, filekind, filename);
}

void analyze_stdin(void)
{
	int fd = 0;
	int filekind;
	const char *filename = "stdin";
	struct stat sb;

	print_line(0, "--- Standard Input");

	/* stat check */
	if (fstat(fd, &sb) < 0) {
		errore("Can't stat %.300s", filename);
		return;
	}
	filekind = analyze_stat(&sb, filename);
	if (filekind < 0)
		return;

	/* go for it */
	analyze_fd(fd, filekind, filename);
}

/*
 * Analyze one file
 */

void print_kind(int filekind, u8 size, int size_known)
{
	char buf[256], *kindname;

	if (filekind == 0)
		kindname = "Regular file";
	else if (filekind == 1)
		kindname = "Block device";
	else if (filekind == 2)
		kindname = "Character device";
	else if (filekind == 3)
		kindname = "FIFO";
	else if (filekind == 4)
		kindname = "Socket";
	else
		kindname = "Unknown kind";

	if (size_known) {
		format_size_verbose(buf, size);
		print_line(0, "%s, size %s", kindname, buf);
	} else {
		print_line(0, "%s, unknown size", kindname);
	}
}

int analyze_stat(struct stat *sb, const char *filename)
{
	int filekind = 0;
	u8 filesize;
	char *reason;

	reason = NULL;
	if (S_ISREG(sb->st_mode)) {
		filesize = sb->st_size;
		print_kind(filekind, filesize, 1);
	} else if (S_ISBLK(sb->st_mode))
		filekind = 1;
	else if (S_ISCHR(sb->st_mode))
		filekind = 2;
	else if (S_ISDIR(sb->st_mode))
		reason = "Is a directory";
	else if (S_ISFIFO(sb->st_mode))
		filekind = 3;
#ifdef S_ISSOCK
	else if (S_ISSOCK(sb->st_mode))
		filekind = 4;
#endif
	else
		reason = "Is an unknown kind of special file";

	if (reason != NULL) {
		error("%.300s: %s", filename, reason);
		return -1;
	}

	return filekind;
}

void analyze_fd(int fd, int filekind, const char *filename)
{
	SOURCE *s;

	/* (try to) guard against TTY character devices */
	if (filekind == 2) {
		if (isatty(fd)) {
			error("%.300s: Is a TTY device", filename);
			return;
		}
	}

	/* create a source */
	s = init_file_source(fd, filekind);

	/* tell the user what it is */
	if (filekind != 0)
		print_kind(filekind, s->size, s->size_known);

	/* now analyze it */
	analyze_source(s, 0);

	/* finish it up */
	close_source(s);
}

/* EOF */
