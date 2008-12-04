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

static int analyze_file(SOURCE *s, int level);
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
  off_t result;

  fs = (FILE_SOURCE *)malloc(sizeof(FILE_SOURCE));
  if (fs == NULL)
    bailout("Out of memory");
  memset(fs, 0, sizeof(FILE_SOURCE));

  if (filekind != 0)  /* special treatment hook for devices */
    fs->c.analyze = analyze_file;
  fs->c.read_bytes = read_file;
  fs->c.close = close_file;
  fs->fd = fd;

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
#define u64 __u64   /* workaround for broken header file */
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
      printf("Size: Linux 32-bit ioctl reports %llu (%lu blocks)\n",
	     fs->c.size, blockcount);
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
      fs->c.size = (u8) dl.d_ncylinders * dl.d_secpercyl * dl.d_secsize;
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
	printf("Size: Darwin ioctl reports %llu (%llu blocks of %lu bytes)\n",
	       fs->c.size, blockcount, blocksize);
#endif
      }
    }
  }
#endif

#if USE_BINARY_SEARCH
  /*
   * binary search:
   * Works on anything that can seek, but is quite expensive.
   */
  if (!fs->c.size_known) {
    u8 lower, upper, current;

    /* TODO: check that the target can seek at all */

#if DEBUG_SIZE
    printf("Size: Doing a binary search\n");
#endif

    /* first, find an upper bound starting from a reasonable guess */
    lower = 0;
    upper = 1024 * 1024;  /* start with 1MB */
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

  return (SOURCE *)fs;
}

/*
 * special handling hook: devices may have out-of-band structure
 */

static int analyze_file(SOURCE *s, int level)
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

  /* seek to the requested position */
  result_seek = lseek(fd, pos, SEEK_SET);
  if (result_seek != pos) {
    errore("Seek to %llu failed", pos);
    return 0;
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

  if (fd >= 0)
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

/* EOF */
