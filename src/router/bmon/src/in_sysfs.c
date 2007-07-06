/*
 * in_sysfs.c           /sys input (Linux)
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * TODO: Give cached FD's some timer to they can be closed if unused
 */

#include <bmon/bmon.h>
#include <bmon/input.h>
#include <bmon/item.h>
#include <bmon/node.h>
#include <bmon/utils.h>
#include <inttypes.h>

#if defined SYS_LINUX

#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

static char * c_dir = "/sys";
static int    c_cache = 1;

struct sysfs_fd_cache
{
	char *filename;
	FILE *fd;
	struct sysfs_fd_cache *next;
};

#define FD_HTSIZE 2048

struct sysfs_fd_cache * fd_ht[FD_HTSIZE];

static uint32_t hash(register const char *s)
{
#define ROTATE_RIGHT(x, n) (x = (x >> (n & 0x1f)) | (x << (32 - (n & 0x1f))))

    uint32_t ret = 0xcafebabe;
    uint8_t  temp;

    do {
        temp = *s++;
        ret ^= temp ^ ((ret >> 8) & 0xff);
        ROTATE_RIGHT(ret, ret);
    } while(temp);

#undef ROTATE_RIGHT

    return ret % FD_HTSIZE;
}

static struct sysfs_fd_cache * cache_get(const char *filename)
{
	struct sysfs_fd_cache *f;
	uint32_t h = hash(filename);

	for (f = fd_ht[h]; f; f = f->next)
		if (!strcmp(f->filename, filename))
			return f;

	return NULL;
}

static struct sysfs_fd_cache * cache_put(struct sysfs_fd_cache *f)
{
	uint32_t h = hash(f->filename);

	f->next = fd_ht[h];
	fd_ht[h] = f;

	return f;
}

static FILE * get_cached_fd(const char *filename)
{
	struct sysfs_fd_cache *f = cache_get(filename);

	if (NULL == f) {
		f = xcalloc(1, sizeof(*f));
		f->filename = strdup(filename);
		f->fd = fopen(filename, "r");

		if (NULL == f->fd)
			quit("Cannot open file %s: %s\n", filename, strerror(errno));

		return cache_put(f)->fd;
	}

	fseek(f->fd, 0, SEEK_SET);
	return f->fd;
}

static b_cnt_t read_int(const char *prefix, const char *file)
{
	FILE *f;
	b_cnt_t r;
	char p[FILENAME_MAX];

	snprintf(p, sizeof(p), "%s/%s", prefix, file);

	if (c_cache)
		f = get_cached_fd(p);
	else {
		f = fopen(p, "r");
		if (NULL == f)
			quit("Cannot open file %s: %s\n", p, strerror(errno));
	}
			

	if (fscanf(f, "%" SCNu64 "\n", &r) != 1)
		quit("fscanf failed: format error\n");

	if (!c_cache)
		fclose(f);

	return r;
}

static void sysfs_read(void)
{
	item_t *n;
	DIR *d;
	struct dirent *de;
	char topdir[FILENAME_MAX];

	snprintf(topdir, sizeof(topdir), "%s/class/net", c_dir);

	d = opendir(topdir);

	if (!d)
		quit("Failed to open directory %s: %s\n", topdir, strerror(errno));

	while ((de = readdir(d))) {
		if (de->d_type == DT_DIR && de->d_name[0] != '.') {
			char p[FILENAME_MAX];

			snprintf(p, sizeof(p), "%s/%s/statistics",
				topdir, de->d_name);

			n = lookup_item(get_local_node(), de->d_name, 0, 0);

			if (NULL == n)
				continue;

			n->i_major_attr = BYTES;
			n->i_minor_attr = PACKETS;
			
			update_attr(n, BYTES, read_int(p, "rx_bytes"),
				    read_int(p, "tx_bytes"), RX_PROVIDED|TX_PROVIDED);
			update_attr(n, PACKETS, read_int(p, "rx_packets"),
				    read_int(p, "tx_packets"), RX_PROVIDED|TX_PROVIDED);
			update_attr(n, ERRORS, read_int(p, "rx_errors"),
				read_int(p, "tx_errors"), RX_PROVIDED|TX_PROVIDED);
			update_attr(n, DROP, read_int(p, "rx_dropped"),
				read_int(p, "tx_dropped"), RX_PROVIDED|TX_PROVIDED);
			update_attr(n, FIFO, read_int(p, "rx_fifo_errors"),
				read_int(p, "tx_fifo_errors"), RX_PROVIDED|TX_PROVIDED);
			update_attr(n, FRAME, read_int(p, "rx_frame_errors"),
				0, RX_PROVIDED);
			update_attr(n, COMPRESSED, read_int(p, "rx_compressed"),
				read_int(p, "tx_compressed"), RX_PROVIDED|TX_PROVIDED);
			update_attr(n, MULTICAST, read_int(p, "multicast"),
				0, RX_PROVIDED);
			update_attr(n, COLLISIONS, 0,
				read_int(p, "collisions"), TX_PROVIDED);
			update_attr(n, CRC_ERRORS, read_int(p, "rx_crc_errors"),
				0, RX_PROVIDED);
			update_attr(n, LENGTH_ERRORS, read_int(p, "rx_length_errors"),
				0, RX_PROVIDED);
			update_attr(n, MISSED_ERRORS, read_int(p, "rx_missed_errors"),
				0, RX_PROVIDED);
			update_attr(n, OVER_ERRORS, read_int(p, "rx_over_errors"),
				0, RX_PROVIDED);
			update_attr(n, ABORTED_ERRORS, 0,
				read_int(p, "tx_aborted_errors"), TX_PROVIDED);
			update_attr(n, CARRIER_ERRORS, 0,
				read_int(p, "tx_carrier_errors"), TX_PROVIDED);
			update_attr(n, HEARTBEAT_ERRORS, 0,
				read_int(p, "tx_heartbeat_errors"), TX_PROVIDED);
			update_attr(n, WINDOW_ERRORS, 0,
				read_int(p, "tx_window_errors"), TX_PROVIDED);

			notify_update(n, NULL);
			increase_lifetime(n, 1);
		}
	}

	closedir(d);
}

static void print_help(void)
{
	printf(
	"sysfs - sysfs statistic collector for Linux" \
	"\n" \
	"  Reads statistics from sysfs (/sys/class/net). File descriptors are\n" \
	"  cached and not closed to minimize I/O operations. This might cause\n" \
	"  problems due to the huge amount of open file descriptors.\n" \
	"  Author: Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    dir=DIR        Sysfs directory (default: /sys)\n" \
	"    nocache        Don't cache file descriptors.\n");
}

static void sysfs_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "dir") && attrs->value)
			c_dir = attrs->value;
		else if (!strcasecmp(attrs->type, "nocache"))
			c_cache = 0;
		else if (!strcasecmp(attrs->type, "help")) {
			print_help();
			exit(0);
		}
		attrs = attrs->next;
	}
}

static int sysfs_probe(void)
{
	DIR *d;
	char topdir[FILENAME_MAX];

	snprintf(topdir, sizeof(topdir), "%s/class/net", c_dir);
	d = opendir(topdir);

	if (d) {
		closedir(d);
		return 1;
	}
	return 0;
}

static struct input_module sysfs_ops = {
	.im_name = "sysfs",
	.im_read = sysfs_read,
	.im_set_opts = sysfs_set_opts,
	.im_probe = sysfs_probe,
};

static void __init proc_init(void)
{
	register_input_module(&sysfs_ops);
}

#endif
