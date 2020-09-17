// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include <sys/mman.h>
#include <signal.h>
#include "init.h"
#include "io.h"

static cmdinfo_t mmap_cmd;
static cmdinfo_t mread_cmd;
static cmdinfo_t msync_cmd;
static cmdinfo_t munmap_cmd;
static cmdinfo_t mwrite_cmd;
#ifdef HAVE_MREMAP
static cmdinfo_t mremap_cmd;
#endif /* HAVE_MREMAP */

mmap_region_t	*maptable;
int		mapcount;
mmap_region_t	*mapping;

static void
print_mapping(
	mmap_region_t	*map,
	int		index,
	int		braces)
{
	char		buffer[8] = { 0 };
	int		i;

	static struct {
		int	prot;
		int	mode;
	} *p, pflags[] = {
		{ PROT_READ,	'r' },
		{ PROT_WRITE,	'w' },
		{ PROT_EXEC,	'x' },
		{ PROT_NONE,	0 }
	};

	for (i = 0, p = pflags; p->prot != PROT_NONE; i++, p++)
		buffer[i] = (map->prot & p->prot) ? p->mode : '-';

	if (map->map_sync)
		sprintf(&buffer[i], " S");

	printf("%c%03d%c 0x%lx - 0x%lx %s  %14s (%lld : %ld)\n",
		braces? '[' : ' ', index, braces? ']' : ' ',
		(unsigned long)map->addr,
		(unsigned long)((char *)map->addr + map->length),
		buffer, map->name ? map->name : "???",
		(long long)map->offset, (long)map->length);
}

void *
check_mapping_range(
	mmap_region_t	*map,
	off64_t		offset,
	size_t		length,
	int		pagealign)
{
	off64_t		relative;

	if (offset < mapping->offset) {
		printf(_("offset (%lld) is before start of mapping (%lld)\n"),
			(long long)offset, (long long)mapping->offset);
		return NULL;
	}
	relative = offset - mapping->offset;
	if (relative > mapping->length) {
		printf(_("offset (%lld) is beyond end of mapping (%lld)\n"),
			(long long)relative, (long long)mapping->offset);
		return NULL;
	}
	if ((relative + length) > (mapping->offset + mapping->length)) {
		printf(_("range (%lld:%lld) is beyond mapping (%lld:%ld)\n"),
			(long long)offset, (long long)relative,
			(long long)mapping->offset, (long)mapping->length);
		return NULL;
	}
	if (pagealign && (long)((char *)mapping->addr + relative) % pagesize) {
		printf(_("offset address (%p) is not page aligned\n"),
			(char *)mapping->addr + relative);
		return NULL;
	}

	return (char *)mapping->addr + relative;
}

int
maplist_f(void)
{
	int		i;

	for (i = 0; i < mapcount; i++)
		print_mapping(&maptable[i], i, &maptable[i] == mapping);
	return 0;
}

static int
mapset_f(
	int		argc,
	char		**argv)
{
	int		i;

	ASSERT(argc == 2);
	i = atoi(argv[1]);
	if (i < 0 || i >= mapcount) {
		printf("value %d is out of range (0-%d)\n", i, mapcount);
		exitcode = 1;
	} else {
		mapping = &maptable[i];
		maplist_f();
	}
	return 0;
}

static void
mmap_help(void)
{
	printf(_(
"\n"
" maps a range within the current file into memory\n"
"\n"
" Example:\n"
" 'mmap -rw 0 1m' - maps one megabyte from the start of the current file\n"
"\n"
" Memory maps a range of a file for subsequent use by other xfs_io commands.\n"
" With no arguments, mmap shows the current mappings.  The current mapping\n"
" can be set by using the single argument form (mapping number or address).\n"
" If two arguments are specified (a range), a new mapping is created and the\n"
" following options are available:\n"
" -r -- map with PROT_READ protection\n"
" -w -- map with PROT_WRITE protection\n"
" -x -- map with PROT_EXEC protection\n"
" -S -- map with MAP_SYNC and MAP_SHARED_VALIDATE flags\n"
" -s <size> -- first do mmap(size)/munmap(size), try to reserve some free space\n"
" If no protection mode is specified, all are used by default.\n"
"\n"));
}

static int
mmap_f(
	int		argc,
	char		**argv)
{
	off64_t		offset;
	ssize_t		length = 0, length2 = 0;
	void		*address = NULL;
	char		*filename;
	size_t		blocksize, sectsize;
	int		c, prot = 0, flags = MAP_SHARED;

	if (argc == 1) {
		if (mapping)
			return maplist_f();
		fprintf(stderr, file ?
			_("no mapped regions, try 'help mmap'\n") :
			_("no files are open, try 'help open'\n"));
		exitcode = 1;
		return 0;
	} else if (argc == 2) {
		if (mapping)
			return mapset_f(argc, argv);
		fprintf(stderr, file ?
			_("no mapped regions, try 'help mmap'\n") :
			_("no files are open, try 'help open'\n"));
		exitcode = 1;
		return 0;
	} else if (!file) {
		fprintf(stderr, _("no files are open, try 'help open'\n"));
		exitcode = 1;
		return 0;
	}

	init_cvtnum(&blocksize, &sectsize);

	while ((c = getopt(argc, argv, "rwxSs:")) != EOF) {
		switch (c) {
		case 'r':
			prot |= PROT_READ;
			break;
		case 'w':
			prot |= PROT_WRITE;
			break;
		case 'x':
			prot |= PROT_EXEC;
			break;
		case 'S':
			flags = MAP_SYNC | MAP_SHARED_VALIDATE;

			/*
			 * If MAP_SYNC and MAP_SHARED_VALIDATE aren't defined
			 * in the system headers we will have defined them
			 * both as 0.
			 */
			if (!flags) {
				printf("MAP_SYNC not supported\n");
				return 0;
			}
			break;
		case 's':
			length2 = cvtnum(blocksize, sectsize, optarg);
			break;
		default:
			exitcode = 1;
			return command_usage(&mmap_cmd);
		}
	}
	if (!prot)
		prot = PROT_READ | PROT_WRITE | PROT_EXEC;

	if (optind != argc - 2) {
		exitcode = 1;
		return command_usage(&mmap_cmd);
	}

	offset = cvtnum(blocksize, sectsize, argv[optind]);
	if (offset < 0) {
		printf(_("non-numeric offset argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	length = cvtnum(blocksize, sectsize, argv[optind]);
	if (length < 0) {
		printf(_("non-numeric length argument -- %s\n"), argv[optind]);
		exitcode = 1;
		return 0;
	}

	filename = strdup(file->name);
	if (!filename) {
		perror("strdup");
		exitcode = 1;
		return 0;
	}

	/*
	 * mmap and munmap memory area of length2 region is helpful to
	 * make a region of extendible free memory. It's generally used
	 * for later mremap operation(no MREMAP_MAYMOVE flag). But there
	 * isn't guarantee that the memory after length (up to length2)
	 * will stay free.
	 */
	if (length2 > length) {
		address = mmap(NULL, length2, prot,
		               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		munmap(address, length2);
	}
	address = mmap(address, length, prot, flags, file->fd, offset);
	if (address == MAP_FAILED) {
		perror("mmap");
		free(filename);
		exitcode = 1;
		return 0;
	}

	/* Extend the control array of mmap'd regions */
	maptable = (mmap_region_t *)realloc(maptable,		/* growing */
					++mapcount * sizeof(mmap_region_t));
	if (!maptable) {
		perror("realloc");
		mapcount = 0;
		munmap(address, length);
		free(filename);
		exitcode = 1;
		return 0;
	}

	/* Finally, make this the new active mapping */
	mapping = &maptable[mapcount - 1];
	mapping->addr = address;
	mapping->length = length;
	mapping->offset = offset;
	mapping->name = filename;
	mapping->prot = prot;
	mapping->map_sync = (flags == (MAP_SYNC | MAP_SHARED_VALIDATE));
	return 0;
}

static void
msync_help(void)
{
	printf(_(
"\n"
" flushes a range of bytes in the current memory mapping\n"
"\n"
" Writes all modified copies of pages over the specified range (or entire\n"
" mapping if no range specified) to their backing storage locations.  Also,\n"
" optionally invalidates so that subsequent references to the pages will be\n"
" obtained from their backing storage locations (instead of cached copies).\n"
" -a -- perform asynchronous writes (MS_ASYNC)\n"
" -i -- invalidate mapped pages (MS_INVALIDATE)\n"
" -s -- perform synchronous writes (MS_SYNC)\n"
"\n"));
}

static int
msync_f(
	int		argc,
	char		**argv)
{
	off64_t		offset;
	ssize_t		length;
	void		*start;
	int		c, flags = 0;
	size_t		blocksize, sectsize;

	while ((c = getopt(argc, argv, "ais")) != EOF) {
		switch (c) {
		case 'a':
			flags |= MS_ASYNC;
			break;
		case 'i':
			flags |= MS_INVALIDATE;
			break;
		case 's':
			flags |= MS_SYNC;
			break;
		default:
			exitcode = 1;
			return command_usage(&msync_cmd);
		}
	}

	if (optind == argc) {
		offset = mapping->offset;
		length = mapping->length;
	} else if (optind == argc - 2) {
		init_cvtnum(&blocksize, &sectsize);
		offset = cvtnum(blocksize, sectsize, argv[optind]);
		if (offset < 0) {
			printf(_("non-numeric offset argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
		optind++;
		length = cvtnum(blocksize, sectsize, argv[optind]);
		if (length < 0) {
			printf(_("non-numeric length argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
	} else {
		exitcode = 1;
		return command_usage(&msync_cmd);
	}

	start = check_mapping_range(mapping, offset, length, 1);
	if (!start) {
		exitcode = 1;
		return 0;
	}

	if (msync(start, length, flags) < 0) {
		perror("msync");
		exitcode = 1;
		return 0;
	}

	return 0;
}

static void
mread_help(void)
{
	printf(_(
"\n"
" reads a range of bytes in the current memory mapping\n"
"\n"
" Example:\n"
" 'mread -v 512 20' - dumps 20 bytes read from 512 bytes into the mapping\n"
"\n"
" Accesses a range of the current memory mapping, optionally dumping it to\n"
" the standard output stream (with -v option) for subsequent inspection.\n"
" -f -- verbose mode, dump bytes with offsets relative to start of file.\n"
" -r -- reverse order; start accessing from the end of range, moving backward\n"
" -v -- verbose mode, dump bytes with offsets relative to start of mapping.\n"
" The accesses are performed sequentially from the start offset by default.\n"
" Notes:\n"
"   References to whole pages following the end of the backing file results\n"
"   in delivery of the SIGBUS signal.  SIGBUS signals may also be delivered\n"
"   on various filesystem conditions, including quota exceeded errors, and\n"
"   for physical device errors (such as unreadable disk blocks).  No attempt\n"
"   has been made to catch signals at this stage...\n"
"\n"));
}

static int
mread_f(
	int		argc,
	char		**argv)
{
	off64_t		offset, tmp, dumpoffset, printoffset;
	ssize_t		length;
	size_t		dumplen, cnt = 0;
	char		*bp;
	void		*start;
	int		dump = 0, rflag = 0, c;
	size_t		blocksize, sectsize;

	while ((c = getopt(argc, argv, "frv")) != EOF) {
		switch (c) {
		case 'f':
			dump = 2;	/* file offset dump */
			break;
		case 'r':
			rflag = 1;	/* read in reverse */
			break;
		case 'v':
			dump = 1;	/* mapping offset dump */
			break;
		default:
			exitcode = 1;
			return command_usage(&mread_cmd);
		}
	}

	if (optind == argc) {
		offset = mapping->offset;
		length = mapping->length;
	} else if (optind == argc - 2) {
		init_cvtnum(&blocksize, &sectsize);
		offset = cvtnum(blocksize, sectsize, argv[optind]);
		if (offset < 0) {
			printf(_("non-numeric offset argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
		optind++;
		length = cvtnum(blocksize, sectsize, argv[optind]);
		if (length < 0) {
			printf(_("non-numeric length argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
	} else {
		return command_usage(&mread_cmd);
	}

	start = check_mapping_range(mapping, offset, length, 0);
	if (!start) {
		exitcode = 1;
		return 0;
	}
	dumpoffset = offset - mapping->offset;
	if (dump == 2)
		printoffset = offset;
	else
		printoffset = dumpoffset;

	if (alloc_buffer(pagesize, 0, 0) < 0) {
		exitcode = 1;
		return 0;
	}
	bp = (char *)io_buffer;

	dumplen = length % pagesize;
	if (!dumplen)
		dumplen = pagesize;

	if (rflag) {
		for (tmp = length - 1, c = 0; tmp >= 0; tmp--, c = 1) {
			*bp = *(((char *)mapping->addr) + dumpoffset + tmp);
			cnt++;
			if (c && cnt == dumplen) {
				if (dump) {
					dump_buffer(printoffset, dumplen);
					printoffset += dumplen;
				}
				bp = (char *)io_buffer;
				dumplen = pagesize;
				cnt = 0;
			} else {
				bp++;
			}
		}
	} else {
		for (tmp = 0, c = 0; tmp < length; tmp++, c = 1) {
			*bp = *(((char *)mapping->addr) + dumpoffset + tmp);
			cnt++;
			if (c && cnt == dumplen) {
				if (dump)
					dump_buffer(printoffset + tmp -
						(dumplen - 1), dumplen);
				bp = (char *)io_buffer;
				dumplen = pagesize;
				cnt = 0;
			} else {
				bp++;
			}
		}
	}
	return 0;
}

static int
munmap_f(
	int		argc,
	char		**argv)
{
	ssize_t		length;
	unsigned int	offset;

	if (munmap(mapping->addr, mapping->length) < 0) {
		perror("munmap");
		exitcode = 1;
		return 0;
	}
	free(mapping->name);

	/* Shuffle the mapping table entries down over the removed entry */
	offset = mapping - &maptable[0];
	length = mapcount * sizeof(mmap_region_t);
	length -= (offset + 1) * sizeof(mmap_region_t);
	if (length)
		memmove(mapping, mapping + 1, length);

	/* Resize the memory allocated for the table, possibly freeing */
	if (--mapcount) {
		maptable = (mmap_region_t *)realloc(maptable,	/* shrinking */
					mapcount * sizeof(mmap_region_t));
		if (offset == mapcount)
			offset--;
		mapping = maptable + offset;
	} else {
		free(maptable);
		mapping = maptable = NULL;
	}
	maplist_f();
	return 0;
}

static void
mwrite_help(void)
{
	printf(_(
"\n"
" dirties a range of bytes in the current memory mapping\n"
"\n"
" Example:\n"
" 'mwrite 512 20 - writes 20 bytes at 512 bytes into the current mapping.\n"
"\n"
" Stores a byte into memory for a range within a mapping.\n"
" The default stored value is 'X', repeated to fill the range specified.\n"
" -S -- use an alternate seed character\n"
" -r -- reverse order; start storing from the end of range, moving backward\n"
" The stores are performed sequentially from the start offset by default.\n"
"\n"));
}

static int
mwrite_f(
	int		argc,
	char		**argv)
{
	off64_t		offset, tmp;
	ssize_t		length;
	void		*start;
	char		*sp;
	int		seed = 'X';
	int		rflag = 0;
	int		c;
	size_t		blocksize, sectsize;

	while ((c = getopt(argc, argv, "rS:")) != EOF) {
		switch (c) {
		case 'r':
			rflag = 1;
			break;
		case 'S':
			seed = (int)strtol(optarg, &sp, 0);
			if (!sp || sp == optarg) {
				printf(_("non-numeric seed -- %s\n"), optarg);
				return 0;
			}
			break;
		default:
			exitcode = 1;
			return command_usage(&mwrite_cmd);
		}
	}

	if (optind == argc) {
		offset = mapping->offset;
		length = mapping->length;
	} else if (optind == argc - 2) {
		init_cvtnum(&blocksize, &sectsize);
		offset = cvtnum(blocksize, sectsize, argv[optind]);
		if (offset < 0) {
			printf(_("non-numeric offset argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
		optind++;
		length = cvtnum(blocksize, sectsize, argv[optind]);
		if (length < 0) {
			printf(_("non-numeric length argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
	} else {
		exitcode = 1;
		return command_usage(&mwrite_cmd);
	}

	start = check_mapping_range(mapping, offset, length, 0);
	if (!start) {
		exitcode = 1;
		return 0;
	}

	offset -= mapping->offset;
	if (rflag) {
		for (tmp = offset + length -1; tmp >= offset; tmp--)
			((char *)mapping->addr)[tmp] = seed;
	} else {
		for (tmp = offset; tmp < offset + length; tmp++)
			((char *)mapping->addr)[tmp] = seed;
	}

	return 0;
}

#ifdef HAVE_MREMAP
static void
mremap_help(void)
{
	printf(_(
"\n"
" resizes the current memory mapping\n"
"\n"
" Examples:\n"
" 'mremap 8192' - resizes the current mapping to 8192 bytes.\n"
"\n"
" Resizes the mapping, growing or shrinking from the current size.\n"
" The default stored value is 'X', repeated to fill the range specified.\n"
" -f <new_address> -- use MREMAP_FIXED flag to mremap on new_address\n"
" -m -- use the MREMAP_MAYMOVE flag\n"
"\n"));
}

static int
mremap_f(
	int		argc,
	char		**argv)
{
	ssize_t		new_length;
	void		*new_addr = NULL;
	int		flags = 0;
	int		c;
	size_t		blocksize, sectsize;

	init_cvtnum(&blocksize, &sectsize);

	while ((c = getopt(argc, argv, "f:m")) != EOF) {
		switch (c) {
		case 'f':
			flags = MREMAP_FIXED|MREMAP_MAYMOVE;
			new_addr = (void *)(unsigned long)cvtnum(blocksize,
			                          sectsize, optarg);
			break;
		case 'm':
			flags = MREMAP_MAYMOVE;
			break;
		default:
			exitcode = 1;
			return command_usage(&mremap_cmd);
		}
	}

	if (optind != argc - 1) {
		exitcode = 1;
		return command_usage(&mremap_cmd);
	}

	new_length = cvtnum(blocksize, sectsize, argv[optind]);
	if (new_length < 0) {
		printf(_("non-numeric offset argument -- %s\n"),
			argv[optind]);
		exitcode = 1;
		return 0;
	}

	if (!new_addr)
		new_addr = mremap(mapping->addr, mapping->length,
		                  new_length, flags);
	else
		new_addr = mremap(mapping->addr, mapping->length,
		                  new_length, flags, new_addr);
	if (new_addr == MAP_FAILED) {
		perror("mremap");
		exitcode = 1;
		return 0;
	}

	mapping->addr = new_addr;
	mapping->length = new_length;
	return 0;
}
#endif /* HAVE_MREMAP */

void
mmap_init(void)
{
	mmap_cmd.name = "mmap";
	mmap_cmd.altname = "mm";
	mmap_cmd.cfunc = mmap_f;
	mmap_cmd.argmin = 0;
	mmap_cmd.argmax = -1;
	mmap_cmd.flags = CMD_NOMAP_OK | CMD_NOFILE_OK |
			 CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	mmap_cmd.args = _("[N] | [-rwxS] [-s size] [off len]");
	mmap_cmd.oneline =
		_("mmap a range in the current file, show mappings");
	mmap_cmd.help = mmap_help;

	mread_cmd.name = "mread";
	mread_cmd.altname = "mr";
	mread_cmd.cfunc = mread_f;
	mread_cmd.argmin = 0;
	mread_cmd.argmax = -1;
	mread_cmd.flags = CMD_NOFILE_OK | CMD_FOREIGN_OK;
	mread_cmd.args = _("[-r] [off len]");
	mread_cmd.oneline =
		_("reads data from a region in the current memory mapping");
	mread_cmd.help = mread_help;

	msync_cmd.name = "msync";
	msync_cmd.altname = "ms";
	msync_cmd.cfunc = msync_f;
	msync_cmd.argmin = 0;
	msync_cmd.argmax = -1;
	msync_cmd.flags = CMD_NOFILE_OK | CMD_FOREIGN_OK;
	msync_cmd.args = _("[-ais] [off len]");
	msync_cmd.oneline = _("flush a region in the current memory mapping");
	msync_cmd.help = msync_help;

	munmap_cmd.name = "munmap";
	munmap_cmd.altname = "mu";
	munmap_cmd.cfunc = munmap_f;
	munmap_cmd.argmin = 0;
	munmap_cmd.argmax = 0;
	munmap_cmd.flags = CMD_NOFILE_OK | CMD_FOREIGN_OK;
	munmap_cmd.oneline = _("unmaps the current memory mapping");

	mwrite_cmd.name = "mwrite";
	mwrite_cmd.altname = "mw";
	mwrite_cmd.cfunc = mwrite_f;
	mwrite_cmd.argmin = 0;
	mwrite_cmd.argmax = -1;
	mwrite_cmd.flags = CMD_NOFILE_OK | CMD_FOREIGN_OK;
	mwrite_cmd.args = _("[-r] [-S seed] [off len]");
	mwrite_cmd.oneline =
		_("writes data into a region in the current memory mapping");
	mwrite_cmd.help = mwrite_help;

#ifdef HAVE_MREMAP
	mremap_cmd.name = "mremap";
	mremap_cmd.altname = "mrm";
	mremap_cmd.cfunc = mremap_f;
	mremap_cmd.argmin = 1;
	mremap_cmd.argmax = 3;
	mremap_cmd.flags = CMD_NOFILE_OK | CMD_FOREIGN_OK;
	mremap_cmd.args = _("[-m|-f <new_address>] newsize");
	mremap_cmd.oneline =
		_("alters the size of the current memory mapping");
	mremap_cmd.help = mremap_help;
#endif /* HAVE_MREMAP */

	add_command(&mmap_cmd);
	add_command(&mread_cmd);
	add_command(&msync_cmd);
	add_command(&munmap_cmd);
	add_command(&mwrite_cmd);
#ifdef HAVE_MREMAP
	add_command(&mremap_cmd);
#endif /* HAVE_MREMAP */
}
