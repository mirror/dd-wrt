/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
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
" -B   -- read backwards through the range from offset (backwards N bytes)\n"
" -F   -- read forwards through the range of bytes from offset (default)\n"
" -v   -- be verbose, dump out buffers (used when reading forwards)\n"
" -R   -- read at random offsets in the range of bytes\n"
" -Z N -- zeed the random number generator (used when reading randomly)\n"
"         (heh, zorry, the -s/-S arguments were already in use in pwrite)\n"
" When in \"random\" mode, the number of read operations will equal the\n"
" number required to do a complete forward/backward scan of the range.\n"
" Note that the offset within the range is chosen at random each time\n"
" (an offset may be read more than once when operating in this mode).\n"
"\n"));
}

void	*buffer;
size_t	highwater;
size_t	buffersize;

int
alloc_buffer(
	size_t		bsize,
	int		uflag,
	unsigned int	seed)
{
	if (bsize > highwater) {
		if (buffer)
			free(buffer);
		buffer = memalign(pagesize, bsize);
		if (!buffer) {
			perror("memalign");
			highwater = buffersize = 0;
			return -1;
		}
		highwater = bsize;
	}
	buffersize = bsize;
	if (!uflag)
		memset(buffer, seed, buffersize);
	return 0;
}

void
dump_buffer(
	off64_t		offset,
	ssize_t		len)
{
	int		i, j;
	char		*p;

	for (i = 0, p = (char *)buffer; i < len; i += 16) {
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
	end = lseek64(fd, 0, SEEK_END);
	offset = (eof || offset > end) ? end : offset;
	if ((bytes = (offset % buffersize)))
		offset -= bytes;
	offset = max(0, offset);
	if ((bytes = (count % buffersize)))
		count += bytes;
	count = max(buffersize, count);
	range = count - buffersize;

	*total = 0;
	while (count > 0) {
		off = ((random() % range) / buffersize) * buffersize;
		bytes = pread64(fd, buffer, buffersize, off);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("pread64");
			return -1;
		}
		ops++;
		*total += bytes;
		if (bytes < buffersize)
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

	end = lseek64(fd, 0, SEEK_END);
	off = eof ? end : min(end, lseek64(fd, off, SEEK_SET));
	if ((end = off - cnt) < 0) {
		cnt += end;	/* subtraction, end is negative */
		end = 0;
	}
	*total = 0;
	*count = cnt;
	*offset = off;

	/* Do initial unaligned read if needed */
	if ((bytes_requested = (off % buffersize))) {
		bytes_requested = min(cnt, bytes_requested);
		off -= bytes_requested;
		bytes = pread(fd, buffer, bytes_requested, off);
		if (bytes == 0)
			return ops;
		if (bytes < 0) {
			perror("pread64");
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
		bytes_requested = min(cnt, buffersize);
		off -= bytes_requested;
		bytes = pread64(fd, buffer, bytes_requested, off);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("pread64");
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
	size_t		bytes_requested;
	ssize_t		bytes;
	int		ops = 0;

	*total = 0;
	while (count > 0 || eof) {
		bytes_requested = min(count, buffersize);
		bytes = pread64(fd, buffer, bytes_requested, offset);
		if (bytes == 0)
			break;
		if (bytes < 0) {
			perror("pread64");
			return -1;
		}
		ops++;
		if (verbose)
			dump_buffer(offset, bytes);
		*total += bytes;
		if (onlyone || bytes < bytes_requested)
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
	char		s1[64], s2[64], ts[64];
	char		*sp;
	int		Cflag, qflag, uflag, vflag;
	int		eof = 0, direction = IO_FORWARD;
	int		c;

	Cflag = qflag = uflag = vflag = 0;
	init_cvtnum(&fsblocksize, &fssectsize);
	bsize = fsblocksize;

	while ((c = getopt(argc, argv, "b:BCFRquvZ:")) != EOF) {
		switch (c) {
		case 'b':
			tmp = cvtnum(fsblocksize, fssectsize, optarg);
			if (tmp < 0) {
				printf(_("non-numeric bsize -- %s\n"), optarg);
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
		case 'Z':
			zeed = strtoul(optarg, &sp, 0);
			if (!sp || sp == optarg) {
				printf(_("non-numeric seed -- %s\n"), optarg);
				return 0;
			}
			break;
		default:
			return command_usage(&pread_cmd);
		}
	}
	if (optind != argc - 2)
		return command_usage(&pread_cmd);

	offset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (offset < 0 && (direction & (IO_RANDOM|IO_BACKWARD))) {
		eof = -1;	/* read from EOF */
	} else if (offset < 0) {
		printf(_("non-numeric length argument -- %s\n"), argv[optind]);
		return 0;
	}
	optind++;
	count = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (count < 0 && (direction & (IO_RANDOM|IO_FORWARD))) {
		eof = -1;	/* read to EOF */
	} else if (count < 0) {
		printf(_("non-numeric length argument -- %s\n"), argv[optind]);
		return 0;
	}

	if (alloc_buffer(bsize, uflag, 0xabababab) < 0)
		return 0;

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
	if (c < 0)
		return 0;
	if (qflag)
		return 0;
	gettimeofday(&t2, NULL);
	t2 = tsub(t2, t1);

	/* Finally, report back -- -C gives a parsable format */
	timestr(&t2, ts, sizeof(ts), Cflag ? VERBOSE_FIXED_TIME : 0);
	if (!Cflag) {
		cvtstr((double)total, s1, sizeof(s1));
		cvtstr(tdiv((double)total, t2), s2, sizeof(s2));
		printf(_("read %lld/%lld bytes at offset %lld\n"),
			total, count, (long long)offset);
		printf(_("%s, %d ops; %s (%s/sec and %.4f ops/sec)\n"),
			s1, c, ts, s2, tdiv((double)c, t2));
	} else {/* bytes,ops,time,bytes/sec,ops/sec */
		printf("%lld,%d,%s,%.3f,%.3f\n",
			total, c, ts,
			tdiv((double)total, t2), tdiv((double)c, t2));
	}
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
	pread_cmd.args = _("[-b bs] [-v] off len");
	pread_cmd.oneline = _("reads a number of bytes at a specified offset");
	pread_cmd.help = pread_help;

	add_command(&pread_cmd);
}
