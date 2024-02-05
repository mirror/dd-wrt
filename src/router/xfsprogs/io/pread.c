// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <sys/uio.h>
#include "command.h"
#include "input.h"
#include <ctype.h>
#include "init.h"
#include "io.h"

static cmdinfo_t pread_cmd;

static void
pread_help(void)
{
	printf(_(
"\n"
" reads a range of bytes in a specified block size from the given offset\n"
"\n"
" Example:\n"
" 'pread -v 512 20' - dumps 20 bytes read from 512 bytes into the file\n"
"\n"
" Reads a segment of the currently open file, optionally dumping it to the\n"
" standard output stream (with -v option) for subsequent inspection.\n"
" The reads are performed in sequential blocks starting at offset, with the\n"
" blocksize tunable using the -b option (default blocksize is 4096 bytes),\n"
" unless a different pattern is requested.\n"
" -q   -- quiet mode, do not write anything to standard output.\n"
" -B   -- read backwards through the range from offset (backwards N bytes)\n"
" -F   -- read forwards through the range of bytes from offset (default)\n"
" -v   -- be verbose, dump out buffers (used when reading forwards)\n"
" -R   -- read at random offsets in the range of bytes\n"
" -Z N -- zeed the random number generator (used when reading randomly)\n"
"         (heh, zorry, the -s/-S arguments were already in use in pwrite)\n"
#ifdef HAVE_PREADV
" -V N -- use vectored IO with N iovecs of blocksize each (preadv)\n"
#endif
"\n"
" When in \"random\" mode, the number of read operations will equal the\n"
" number required to do a complete forward/backward scan of the range.\n"
" Note that the offset within the range is chosen at random each time\n"
" (an offset may be read more than once when operating in this mode).\n"
"\n"));
}

void	*io_buffer;
size_t	highwater;
size_t	io_buffersize;
int	vectors;
struct iovec *iov;

static int
alloc_iovec(
	size_t		bsize,
	int		uflag,
	unsigned int	seed)
{
	int		i;

	iov = calloc(vectors, sizeof(struct iovec));
	if (!iov)
		return -1;

	io_buffersize = 0;
	for (i = 0; i < vectors; i++) {
		iov[i].iov_base = memalign(pagesize, bsize);
		if (!iov[i].iov_base) {
			perror("memalign");
			goto unwind;
		}
		iov[i].iov_len = bsize;
		if (!uflag)
			memset(iov[i].iov_base, seed, bsize);
	}
	io_buffersize = bsize * vectors;
	return 0;
unwind:
	for( ; i >= 0; i--)
		free(iov[i].iov_base);
	free(iov);
	iov = NULL;
	return -1;
}

int
alloc_buffer(
	size_t		bsize,
	int		uflag,
	unsigned int	seed)
{
	if (vectors)
		return alloc_iovec(bsize, uflag, seed);

	if (bsize > highwater) {
		if (io_buffer)
			free(io_buffer);
		io_buffer = memalign(pagesize, bsize);
		if (!io_buffer) {
			perror("memalign");
			highwater = io_buffersize = 0;
			return -1;
		}
		highwater = bsize;
	}
	io_buffersize = bsize;
	if (!uflag)
		memset(io_buffer, seed, io_buffersize);
	return 0;
}

static void
__dump_buffer(
	void		*buf,
	off64_t		offset,
	ssize_t		len)
{
	int		i, j;
	char		*p;

	for (i = 0, p = (char *)buf; i < len; i += 16) {
		char	*s = p;

		printf("%08llx:  ", (unsigned long long)offset + i);
		for (j = 0; j < 16 && i + j < len; j++, p++)
			printf("%02x ", *p);
		printf(" ");
		for (j = 0; j < 16 && i + j < len; j++, s++) {
			if (isalnum((int)*s))
				printf("%c", *s);
			else
				printf(".");
		}
		printf("\n");
	}
}

void
dump_buffer(
	off64_t		offset,
	ssize_t		len)
{
	int		i, l;

	if (!vectors) {
		__dump_buffer(io_buffer, offset, len);
		return;
	}

	for (i = 0; len > 0 && i < vectors; i++) {
		l = min(len, iov[i].iov_len);

		__dump_buffer(iov[i].iov_base, offset, l);
		len -= l;
		offset += l;
	}
}

#ifdef HAVE_PREADV
static ssize_t
do_preadv(
	int		fd,
	off64_t		offset,
	long long	count)
{
	int		vecs = 0;
	ssize_t		oldlen = 0;
	ssize_t		bytes = 0;

	/* trim the iovec if necessary */
	if (count < io_buffersize) {
		size_t	len = 0;
		while (len + iov[vecs].iov_len < count) {
			len += iov[vecs].iov_len;
			vecs++;
		}
		oldlen = iov[vecs].iov_len;
		iov[vecs].iov_len = count - len;
		vecs++;
	} else {
		vecs = vectors;
	}
	bytes = preadv(fd, iov, vectors, offset);

	/* restore trimmed iov */
	if (oldlen)
		iov[vecs - 1].iov_len = oldlen;

	return bytes;
}
#else
#define do_preadv(fd, offset, count) (0)
#endif

static ssize_t
do_pread(
	int		fd,
	off64_t		offset,
	long long	count,
	size_t		buffer_size)
{
	if (!vectors)
		return pread(fd, io_buffer, min(count, buffer_size), offset);

	return do_preadv(fd, offset, count);
}

static int
read_random(
	int		fd,
	off64_t		offset,
	long long	count,
	long long	*total,
	unsigned int	seed,
	int		eof)
{
	off64_t		end, off, range;
	ssize_t		bytes;
	int		ops = 0;

	srandom(seed);
	end = lseek(fd, 0, SEEK_END);
	offset = (eof || offset > end) ? end : offset;
	if ((bytes = (offset % io_buffersize)))
		offset -= bytes;
	offset = max(0, offset);
	if ((bytes = (count % io_buffersize)))
		count += bytes;
	count = max(io_buffersize, count);
	range = count - io_buffersize;

	*total = 0;
	while (count > 0) {
		if (range)
			off = ((offset + (random() % range)) / io_buffersize) *
				io_buffersize;
		else
			off = offset;
		bytes = do_pread(fd, off, io_buffersize, io_buffersize);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("pread");
			return -1;
		}
		ops++;
		*total += bytes;
		if (bytes < io_buffersize)
			break;
		count -= bytes;
	}
	return ops;
}

static int
read_backward(
	int		fd,
	off64_t		*offset,
	long long	*count,
	long long	*total,
	int		eof)
{
	off64_t		end, off = *offset;
	ssize_t		bytes = 0, bytes_requested;
	long long	cnt = *count;
	int		ops = 0;

	end = lseek(fd, 0, SEEK_END);
	off = eof ? end : min(end, lseek(fd, off, SEEK_SET));
	if ((end = off - cnt) < 0) {
		cnt += end;	/* subtraction, end is negative */
		end = 0;
	}
	*total = 0;
	*count = cnt;
	*offset = off;

	/* Do initial unaligned read if needed */
	if ((bytes_requested = (off % io_buffersize))) {
		off -= bytes_requested;
		bytes = do_pread(fd, off, bytes_requested, io_buffersize);
		if (bytes == 0)
			return ops;
		if (bytes < 0) {
			perror("pread");
			return -1;
		}
		ops++;
		*total += bytes;
		if (bytes < bytes_requested)
			return ops;
		cnt -= bytes;
	}

	/* Iterate backward through the rest of the range */
	while (cnt > end) {
		bytes_requested = min(cnt, io_buffersize);
		off -= bytes_requested;
		bytes = do_pread(fd, off, cnt, io_buffersize);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("pread");
			return -1;
		}
		ops++;
		*total += bytes;
		if (bytes < bytes_requested)
			break;
		cnt -= bytes;
	}
	return ops;
}

static int
read_forward(
	int		fd,
	off64_t		offset,
	long long	count,
	long long	*total,
	int		verbose,
	int		onlyone,
	int		eof)
{
	ssize_t		bytes;
	int		ops = 0;

	*total = 0;
	while (count > 0 || eof) {
		bytes = do_pread(fd, offset, count, io_buffersize);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("pread");
			return -1;
		}
		ops++;
		if (verbose)
			dump_buffer(offset, bytes);
		*total += bytes;
		if (onlyone || bytes < min(count, io_buffersize))
			break;
		offset += bytes;
		count -= bytes;
	}
	return ops;
}

int
read_buffer(
	int		fd,
	off64_t		offset,
	long long	count,
	long long	*total,
	int		verbose,
	int		onlyone)
{
	return read_forward(fd, offset, count, total, verbose, onlyone, 0);
}

static int
pread_f(
	int		argc,
	char		**argv)
{
	size_t		bsize;
	off64_t		offset;
	unsigned int	zeed = 0;
	long long	count, total, tmp;
	size_t		fsblocksize, fssectsize;
	struct timeval	t1, t2;
	char		*sp;
	int		Cflag, qflag, uflag, vflag;
	int		eof = 0, direction = IO_FORWARD;
	int		c;

	Cflag = qflag = uflag = vflag = 0;
	init_cvtnum(&fsblocksize, &fssectsize);
	bsize = fsblocksize;

	while ((c = getopt(argc, argv, "b:BCFRquvV:Z:")) != EOF) {
		switch (c) {
		case 'b':
			tmp = cvtnum(fsblocksize, fssectsize, optarg);
			if (tmp < 0) {
				printf(_("non-numeric bsize -- %s\n"), optarg);
				exitcode = 1;
				return 0;
			}
			bsize = tmp;
			break;
		case 'C':
			Cflag = 1;
			break;
		case 'F':
			direction = IO_FORWARD;
			break;
		case 'B':
			direction = IO_BACKWARD;
			break;
		case 'R':
			direction = IO_RANDOM;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'u':
			uflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
#ifdef HAVE_PREADV
		case 'V':
			vectors = strtoul(optarg, &sp, 0);
			if (!sp || sp == optarg) {
				printf(_("non-numeric vector count == %s\n"),
					optarg);
				exitcode = 1;
				return 0;
			}
			break;
#endif
		case 'Z':
			zeed = strtoul(optarg, &sp, 0);
			if (!sp || sp == optarg) {
				printf(_("non-numeric seed -- %s\n"), optarg);
				exitcode = 1;
				return 0;
			}
			break;
		default:
			exitcode = 1;
			return command_usage(&pread_cmd);
		}
	}
	if (optind != argc - 2) {
		exitcode = 1;
		return command_usage(&pread_cmd);
	}

	offset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (offset < 0 && (direction & (IO_RANDOM|IO_BACKWARD))) {
		eof = -1;	/* read from EOF */
	} else if (offset < 0) {
		printf(_("non-numeric length argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	count = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (count < 0 && (direction & (IO_RANDOM|IO_FORWARD))) {
		eof = -1;	/* read to EOF */
	} else if (count < 0) {
		printf(_("non-numeric length argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}

	if (alloc_buffer(bsize, uflag, 0xabababab) < 0) {
		exitcode = 1;
		return 0;
	}

	gettimeofday(&t1, NULL);
	switch (direction) {
	case IO_RANDOM:
		if (!zeed)	/* srandom seed */
			zeed = time(NULL);
		c = read_random(file->fd, offset, count, &total, zeed, eof);
		break;
	case IO_FORWARD:
		c = read_forward(file->fd, offset, count, &total, vflag, 0, eof);
		if (eof)
			count = total;
		break;
	case IO_BACKWARD:
		c = read_backward(file->fd, &offset, &count, &total, eof);
		break;
	default:
		ASSERT(0);
	}
	if (c < 0) {
		exitcode = 1;
		return 0;
	}

	if (qflag)
		return 0;
	gettimeofday(&t2, NULL);
	t2 = tsub(t2, t1);

	report_io_times("read", &t2, (long long)offset, count, total, c, Cflag);
	return 0;
}

void
pread_init(void)
{
	pread_cmd.name = "pread";
	pread_cmd.altname = "r";
	pread_cmd.cfunc = pread_f;
	pread_cmd.argmin = 2;
	pread_cmd.argmax = -1;
	pread_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	pread_cmd.args = _("[-b bs] [-qv] [-i N] [-FBR [-Z N]] off len");
	pread_cmd.oneline = _("reads a number of bytes at a specified offset");
	pread_cmd.help = pread_help;

	add_command(&pread_cmd);
}
