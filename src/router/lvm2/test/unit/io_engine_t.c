/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "units.h"
#include "lib/device/bcache.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//----------------------------------------------------------------

#define SECTOR_SIZE 512
#define BLOCK_SIZE_SECTORS 8
#define NR_BLOCKS 64

struct fixture {
	struct io_engine *e;
	uint8_t *data;

	char fname[64];
	int fd;
};

static void _fill_buffer(uint8_t *buffer, uint8_t seed, size_t count)
{
        unsigned i;
        uint8_t b = seed;

	for (i = 0; i < count; i++) {
        	buffer[i] = b;
        	b = ((b << 5) + b) + i;
	}
}

static void _check_buffer(uint8_t *buffer, uint8_t seed, size_t count)
{
	unsigned i;
	uint8_t b = seed;

	for (i = 0; i < count; i++) {
        	T_ASSERT_EQUAL(buffer[i], b);
        	b = ((b << 5) + b) + i;
	}
}

static void _print_buffer(const char *name, uint8_t *buffer, size_t count)
{
	unsigned col;

	fprintf(stderr, "%s:\n", name);
	while (count) {
		for (col = 0; count && col < 20; col++) {
        		fprintf(stderr, "%x, ", (unsigned) *buffer);
        		col++;
        		buffer++;
        		count--;
		}
		fprintf(stderr, "\n");
	}
}

static void *_fix_init(void)
{
        struct fixture *f = malloc(sizeof(*f));

        T_ASSERT(f);
        f->e = create_async_io_engine();
        T_ASSERT(f->e);
	if (posix_memalign((void **) &f->data, 4096, SECTOR_SIZE * BLOCK_SIZE_SECTORS))
        	test_fail("posix_memalign failed");

        snprintf(f->fname, sizeof(f->fname), "unit-test-XXXXXX");
	f->fd = mkstemp(f->fname);
	T_ASSERT(f->fd >= 0);

	_fill_buffer(f->data, 123, SECTOR_SIZE * BLOCK_SIZE_SECTORS);

	T_ASSERT(write(f->fd, f->data, SECTOR_SIZE * BLOCK_SIZE_SECTORS) > 0);
	T_ASSERT(lseek(f->fd, 0, SEEK_SET) != -1);

        return f;
}

static void _fix_exit(void *fixture)
{
        struct fixture *f = fixture;

	close(f->fd);
	unlink(f->fname);
        free(f->data);
        if (f->e)
                f->e->destroy(f->e);
        free(f);
}

static void _test_create(void *fixture)
{
	// empty
}

struct io {
	bool completed;
	int error;
};

static void _io_init(struct io *io)
{
	io->completed = false;
	io->error = 0;
}

static void _complete_io(void *context, int io_error)
{
	struct io *io = context;
	io->completed = true;
	io->error = io_error;
}

static void _test_read(void *fixture)
{
	struct fixture *f = fixture;

	struct io io;

	_io_init(&io);
	T_ASSERT(f->e->issue(f->e, DIR_READ, f->fd, 0, BLOCK_SIZE_SECTORS, f->data, &io));
	T_ASSERT(f->e->wait(f->e, _complete_io));
	T_ASSERT(io.completed);
	T_ASSERT(!io.error);

	_check_buffer(f->data, 123, sizeof(f->data));
}

static void _test_write(void *fixture)
{
	struct fixture *f = fixture;

	struct io io;

	_io_init(&io);
	T_ASSERT(f->e->issue(f->e, DIR_WRITE, f->fd, 0, BLOCK_SIZE_SECTORS, f->data, &io));
	T_ASSERT(f->e->wait(f->e, _complete_io));
	T_ASSERT(io.completed);
	T_ASSERT(!io.error);
}

static void _test_write_bytes(void *fixture)
{
	struct fixture *f = fixture;

	unsigned offset = 345;
	char buf_out[32];
	char buf_in[32];
	struct bcache *cache = bcache_create(8, BLOCK_SIZE_SECTORS, f->e);
	T_ASSERT(cache);

	// T_ASSERT(bcache_read_bytes(cache, f->fd, offset, sizeof(buf_in), buf_in));
	_fill_buffer((uint8_t *) buf_out, 234, sizeof(buf_out));
	T_ASSERT(bcache_write_bytes(cache, f->fd, offset, sizeof(buf_out), buf_out));
	T_ASSERT(bcache_read_bytes(cache, f->fd, offset, sizeof(buf_in), buf_in));

	_print_buffer("buf_out", (uint8_t *) buf_out, sizeof(buf_out));
	_print_buffer("buf_in", (uint8_t *) buf_in, sizeof(buf_in));
	T_ASSERT(!memcmp(buf_out, buf_in, sizeof(buf_out)));

	bcache_destroy(cache);
	f->e = NULL;   // already destroyed
}

//----------------------------------------------------------------

#define T(path, desc, fn) register_test(ts, "/base/device/bcache/io-engine/" path, desc, fn)

static struct test_suite *_tests(void)
{
        struct test_suite *ts = test_suite_create(_fix_init, _fix_exit);
        if (!ts) {
                fprintf(stderr, "out of memory\n");
                exit(1);
        }

        T("create-destroy", "simple create/destroy", _test_create);
        T("read", "read sanity check", _test_read);
        T("write", "write sanity check", _test_write);
        T("bcache-write-bytes", "test the utility fns", _test_write_bytes);

        return ts;
}

void io_engine_tests(struct dm_list *all_tests)
{
	dm_list_add(all_tests, &_tests()->list);
}

