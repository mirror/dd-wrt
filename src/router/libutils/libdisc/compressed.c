/*
 * compressed.c
 * Layered data source for compressed files.
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

#include <signal.h>
#include <sys/wait.h>

#define DEBUG 0

#if !defined(FD_ZERO)
#define DECOMPRESS 0
#warning Transparent decompression disabled, select() macros not defined
#elif defined(__amigaos__) && !defined(__ixemul__)
#define DECOMPRESS 0
#warning Transparent decompression disabled, ixemul not available
#else
#define DECOMPRESS 1
#endif

/*
 * types
 */

#if DECOMPRESS
typedef struct compressed_source {
	SOURCE c;
	u8 offset, write_pos, write_max;
	int write_pipe, read_pipe, nfds;
	pid_t pid;
} COMPRESSED_SOURCE;
#endif

/*
 * helper functions
 */

static void handle_compressed(SECTION *section, int level, int off, const char *program);

#if DECOMPRESS
static SOURCE *init_compressed_source(SOURCE *foundation, u8 offset, u8 size, const char *program);
static u8 read_compressed(SOURCE *s, u8 pos, u8 len, void *buf);
static void close_compressed(SOURCE *s);
#endif

/*
 * compressed file detection
 */

int detect_compressed(SECTION *section, int level)
{
	int fill, off, sector;
	unsigned char *buf;
	int found = 0;
	fill = get_buffer(section, 0, 4096, (void **)&buf);

	/* look for signatures at sector beginnings */
	for (off = 0; off + 512 <= fill; off += 512) {
		sector = off >> 9;

		/* compress */
		if (buf[off] == 037 && buf[off + 1] == 0235) {
			if (sector > 0)
				print_line(level, "compress-compressed data at sector %d", sector);
			else
				print_line(level, "compress-compressed data");

			handle_compressed(section, level, off, "gzip");
			found = 1;

			break;
		}

		/* gzip */
		if (buf[off] == 037 && (buf[off + 1] == 0213 || buf[off + 1] == 0236)) {
			if (sector > 0)
				print_line(level, "gzip-compressed data at sector %d", sector);
			else
				print_line(level, "gzip-compressed data");

			handle_compressed(section, level, off, "gzip");
			found = 1;

			break;
		}

		/* bzip2 */
		if (memcmp(buf + off, "BZh", 3) == 0) {
			if (sector > 0)
				print_line(level, "bzip2-compressed data at sector %d", sector);
			else
				print_line(level, "bzip2-compressed data");

			handle_compressed(section, level, off, "bzip2");
			found = 1;

			break;
		}
	}
	return found;
}

static void handle_compressed(SECTION *section, int level, int off, const char *program)
{
#if DECOMPRESS
	SOURCE *s;
	u8 size;

	/* create decompression data source */
	size = section->size;
	if (size > 0)
		size -= off;
	s = init_compressed_source(section->source, section->pos + off, size, program);
	analyze_source(s, level + 1);
	close_source(s);
#else
	print_line(level + 1, "Decompression disabled on this system");
#endif
}

/*
 * initialize the decompression
 */

#if DECOMPRESS

static SOURCE *init_compressed_source(SOURCE *foundation, u8 offset, u8 size, const char *program)
{
	COMPRESSED_SOURCE *cs;
	int write_pipe[2], read_pipe[2], flags;

	cs = (COMPRESSED_SOURCE *)malloc(sizeof(COMPRESSED_SOURCE));
	if (cs == NULL)
		bailout("Out of memory");
	memset(cs, 0, sizeof(COMPRESSED_SOURCE));

	cs->c.sequential = 1;
	cs->c.seq_pos = 0;
	cs->c.foundation = foundation;
	cs->c.read_bytes = read_compressed;
	cs->c.close = close_compressed;
	/* size is not known in advance by definition */

	cs->offset = offset;
	cs->write_pos = 0;
	cs->write_max = size;

	/* open "gzip -dc" in a dual pipe */
	if (pipe(write_pipe) < 0)
		bailoute("pipe for decompression");
	if (pipe(read_pipe) < 0)
		bailoute("pipe for decompression");
	cs->write_pipe = write_pipe[1];
	cs->read_pipe = read_pipe[0];

	cs->pid = fork();
	if (cs->pid < 0) {
		bailoute("fork");
	}
	if (cs->pid == 0) { /* we're the child process */
		/* set up pipe */
		dup2(write_pipe[0], 0);
		if (write_pipe[0] > 2)
			close(write_pipe[0]);
		close(write_pipe[1]);
		close(read_pipe[0]);
		dup2(read_pipe[1], 1);
		if (read_pipe[1] > 2)
			close(read_pipe[1]);

		/* execute decompressor (gzip or bzip2) */
		execlp(program, program, "-dc", NULL);
		exit(0);
	}
	/* we're the parent process */
	close(write_pipe[0]);
	close(read_pipe[1]);

	/* set non-blocking I/O */
	if ((flags = fcntl(cs->write_pipe, F_GETFL, 0)) >= 0)
		fcntl(cs->write_pipe, F_SETFL, flags | O_NONBLOCK);
	else
		bailoute("set pipe flags");
	if ((flags = fcntl(cs->read_pipe, F_GETFL, 0)) >= 0)
		fcntl(cs->read_pipe, F_SETFL, flags | O_NONBLOCK);
	else
		bailoute("set pipe flags");
	cs->nfds = ((cs->read_pipe > cs->write_pipe) ? cs->read_pipe : cs->write_pipe) + 1;

	return (SOURCE *)cs;
}

/*
 * raw read
 */

static u8 read_compressed(SOURCE *s, u8 pos, u8 len, void *buf)
{
	COMPRESSED_SOURCE *cs = (COMPRESSED_SOURCE *)s;
	SOURCE *fs = s->foundation;
	char *p, *filebuf;
	u8 got, fill;
	int askfor, selresult;
	ssize_t result;
	fd_set read_set;
	fd_set write_set;

#if DEBUG
	printf("rc got asked for pos %llu len %llu\n", pos, len);
#endif

	p = (char *)buf;
	got = 0;

	if (cs->read_pipe < 0) /* closed for reading */
		return got;

	while (got < len) {
		result = read(cs->read_pipe, p, len - got);
#if DEBUG
		printf("rc read got %d\n", result);
#endif
		if (result == 0) { /* end of file */
			/* remember size for buffer layer */
			s->size_known = 1;
			s->size = s->seq_pos + got;
			/* close pipe (stops future read attempts in the track) */
			close(cs->read_pipe);
			cs->read_pipe = -1;
			/* we're done */
			break;
		} else if (result > 0) { /* got data */
			p += result;
			got += result;
			continue;
		} else { /* error return */
			if (errno == EINTR)
				continue;
			if (errno != EAGAIN) {
				errore("read from pipe");
				break;
			}
		}

		/* no data available for reading right now, so try to write
		   some uncompressed data down the other pipe for a change */

		/* calculate how much data to write */
		askfor = 4096;
		if (cs->write_max && cs->write_pos + askfor > cs->write_max)
			askfor = cs->write_max - cs->write_pos;
		if (askfor <= 0 && cs->write_pipe >= 0) {
			/* there's no more data to write, close the pipe */
			close(cs->write_pipe);
			cs->write_pipe = -1;
		}
		if (cs->write_pipe < 0) {
			/* no more data to write, just wait for input using select */
			FD_ZERO(&read_set);
			FD_SET(cs->read_pipe, &read_set);
#if DEBUG
			printf("rc starting select\n");
#endif
			selresult = select(cs->nfds, &read_set, NULL, NULL, NULL);
#if DEBUG
			printf("rc select got %d\n", selresult);
#endif
			if (selresult < 0 && errno != EINTR) {
				errore("select");
				break;
			}
			continue;
		}

		/* get data from lower layer */
		fill = get_buffer_real(fs, cs->offset + cs->write_pos, askfor, NULL, (void **)&filebuf);
#if DEBUG
		printf("rc get_buffer asked for pos %llu len %d got %llu\n", cs->offset + cs->write_pos, askfor, fill);
#endif
		if (fill < askfor) {
			/* we reached the end of compressed input, note that down */
			cs->write_max = cs->write_pos + fill;
		}
		if (fill <= 0) {
			/* didn't get any data to write, so no need trying */
			/* NOTE: in this case, the above if() also caught on and the next
			   time through the loop, the write pipe will be closed. */
			continue;
		}

		/* try a write right now */
		result = write(cs->write_pipe, filebuf, fill);
#if DEBUG
		printf("rc write got %d\n", result);
#endif
		if (result >= 0) {
			cs->write_pos += result;
			continue; /* see if that made more data available for reading */
		} else {
			if (errno == EINTR)
				continue;
			if (errno != EAGAIN) {
				errore("write to pipe");
				break;
			}
		}

		/* both pipes are blocked right now. wait using select(). */
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_SET(cs->read_pipe, &read_set);
		FD_SET(cs->write_pipe, &write_set);
#if DEBUG
		printf("rc starting select\n");
#endif
		selresult = select(cs->nfds, &read_set, &write_set, NULL, NULL);
#if DEBUG
		printf("rc select got %d\n", selresult);
#endif
		if (selresult < 0 && errno != EINTR) {
			errore("select");
			break;
		}
	}

	return got;
}

/*
 * close cleanup
 */

static void close_compressed(SOURCE *s)
{
	COMPRESSED_SOURCE *cs = (COMPRESSED_SOURCE *)s;
	int status;

	if (cs->write_pipe >= 0)
		close(cs->write_pipe);
	if (cs->read_pipe >= 0)
		close(cs->read_pipe);
	kill(cs->pid, SIGHUP);
	waitpid(cs->pid, &status, 0);
}

#endif /* DECOMPRESS */

/* EOF */
