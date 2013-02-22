/*
 *  nanddump.c
 *
 *  Copyright (C) 2000 David Woodhouse (dwmw2@infradead.org)
 *                     Steven J. Hill (sjhill@realitydiluted.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This utility dumps the contents of raw NAND chips or NAND
 *   chips contained in DoC devices.
 */

#define PROGRAM_NAME "nanddump"
#define VERSION "1.31"

#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <asm/types.h>
#include <mtd/mtd-user.h>
#include "common.h"
#include <libmtd.h>

static struct nand_oobinfo none_oobinfo = {
	.useecc = MTD_NANDECC_OFF,
};

static void display_help(void)
{
	printf(
"Usage: %s [OPTIONS] MTD-device\n"
"Dumps the contents of a nand mtd partition.\n"
"\n"
"           --help               Display this help and exit\n"
"           --version            Output version information and exit\n"
"           --bb=METHOD          Choose bad block handling method (see below).\n"
"-a         --forcebinary        Force printing of binary data to tty\n"
"-c         --canonicalprint     Print canonical Hex+ASCII dump\n"
"-f file    --file=file          Dump to file\n"
"-l length  --length=length      Length\n"
"-n         --noecc              Read without error correction\n"
"-o         --omitoob            Omit OOB data (default in next release)\n"
"           --oob                Dump OOB data (current default)\n"
"-p         --prettyprint        Print nice (hexdump)\n"
"-q         --quiet              Don't display progress and status messages\n"
"-s addr    --startaddress=addr  Start address\n"
"\n"
"--bb=METHOD, where METHOD can be `padbad', `dumpbad', or `skipbad':\n"
"    padbad:  dump flash data, substituting 0xFF for any bad blocks (default)\n"
"    dumpbad: dump flash data, including any bad blocks\n"
"    skipbad: dump good data, completely skipping any bad blocks\n"
"\n"
"Deprecated options:\n"
"The following options are being replaced by --bb=METHOD flags or being\n"
"removed entirely. Do not continue to use these options.\n"
"-b         --omitbad            Omit bad blocks from the dump (DEPRECATED)\n"
"-N         --noskipbad          Read without bad block skipping\n"
"\n"
"Notes on --omitbad and --bb=skipbad:\n"
"* `omitbad' and `skipbad' are very similar; we are deprecating `--omitbad'\n"
"  in favor of `--bb=skipbad'.\n"
"* With either option, we stop dumping data when we encounter a bad block\n"
"  and resume dumping at the next good block. However, with `omitbad', we\n"
"  count the bad block as part of the total dump length, whereas with\n"
"  `skipbad', the bad block is skipped, that is, not counted toward the\n"
"  total dump length.\n"
"\n"
"Note on --oob, --omitoob:\n"
"  To make nanddump act more like an inverse to nandwrite, we are changing\n"
"  the default OOB behavior. In the next release, nanddump will not dump\n"
"  OOB data by default. We will leave both the `--omitoob' and `--oob'\n"
"  options, but to mirror nandwrite, the short option `-o' will then stand\n"
"  for `--oob', not `--omitoob'. Please adjust your usage accordingly.\n",
	PROGRAM_NAME);
	exit(EXIT_SUCCESS);
}

static void display_version(void)
{
	printf("%1$s " VERSION "\n"
			"\n"
			"%1$s comes with NO WARRANTY\n"
			"to the extent permitted by law.\n"
			"\n"
			"You may redistribute copies of %1$s\n"
			"under the terms of the GNU General Public Licence.\n"
			"See the file `COPYING' for more information.\n",
			PROGRAM_NAME);
	exit(EXIT_SUCCESS);
}

// Option variables

static bool			pretty_print = false;	// print nice
static bool			noecc = false;		// don't error correct
static bool			omitoob = false;	// omit oob data
static long long		start_addr;		// start address
static long long		length;			// dump length
static const char		*mtddev;		// mtd device name
static const char		*dumpfile;		// dump file name
static bool			quiet = false;		// suppress diagnostic output
static bool			canonical = false;	// print nice + ascii
static bool			forcebinary = false;	// force printing binary to tty

static enum {
	padbad,   // dump flash data, substituting 0xFF for any bad blocks
	dumpbad,  // dump flash data, including any bad blocks
	skipbad,  // dump good data, completely skipping any bad blocks
	omitbad   // dump flash data, substituting nothing for any bad blocks (DEPRECATED)
} bb_method = padbad;

static void process_options(int argc, char * const argv[])
{
	int error = 0;
	bool bb_default = true, oob_default = true;

	for (;;) {
		int option_index = 0;
		static const char *short_options = "bs:f:l:opqnNca";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 0},
			{"version", no_argument, 0, 0},
			{"bb", required_argument, 0, 0},
			{"oob", no_argument, 0, 0},
			{"forcebinary", no_argument, 0, 'a'},
			{"canonicalprint", no_argument, 0, 'c'},
			{"file", required_argument, 0, 'f'},
			{"prettyprint", no_argument, 0, 'p'},
			{"omitoob", no_argument, 0, 'o'},
			{"omitbad", no_argument, 0, 'b'}, //DEPRECATED
			{"startaddress", required_argument, 0, 's'},
			{"length", required_argument, 0, 'l'},
			{"noecc", no_argument, 0, 'n'},
			{"noskipbad", no_argument, 0, 'N'},
			{"quiet", no_argument, 0, 'q'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (c == EOF) {
			break;
		}

		switch (c) {
			case 0:
				switch (option_index) {
					case 0:
						display_help();
						break;
					case 1:
						display_version();
						break;
					case 2:
						/* Handle --bb=METHOD */
						if (!strcmp(optarg, "padbad"))
							bb_method = padbad;
						else if (!strcmp(optarg, "dumpbad"))
							bb_method = dumpbad;
						else if (!strcmp(optarg, "skipbad"))
							bb_method = skipbad;
						else
							error++;
						bb_default = false;
						break;
					case 3: /* --oob */
						if (oob_default) {
							oob_default = false;
							omitoob = false;
						} else {
							errmsg_die("--oob and --oomitoob are mutually exclusive");
						}
						break;
				}
				break;
			case 'b':
				/* Check if bb_method was already set explicitly */
				if (bb_default) {
					bb_default = false;
					bb_method = omitbad;
					warnmsg("--omitbad is being deprecated in favor of --bb=skipbad.\n"
						"  --omitbad will not be available in future releases.\n"
						"  Please update your usage accordingly.");
				} else {
					error++;
				}
				break;
			case 's':
				start_addr = simple_strtoll(optarg, &error);
				break;
			case 'f':
				if (!(dumpfile = strdup(optarg))) {
					perror("stddup");
					exit(EXIT_FAILURE);
				}
				break;
			case 'l':
				length = simple_strtoll(optarg, &error);
				break;
			case 'o':
				if (oob_default) {
					oob_default = false;
					omitoob = true;
				} else {
					errmsg_die("--oob and --oomitoob are mutually exclusive");
				}
				break;
			case 'a':
				forcebinary = true;
				break;
			case 'c':
				canonical = true;
			case 'p':
				pretty_print = true;
				break;
			case 'q':
				quiet = true;
				break;
			case 'n':
				noecc = true;
				break;
			case 'N':
				/* Check if bb_method was already set explicitly */
				if (bb_default) {
					bb_default = false;
					bb_method = dumpbad;
					warnmsg("--noskipbad is being deprecated in favor of --bb=dumpbad.\n"
						"  --noskipbad will not be available in future releases.\n"
						"  Please update your usage accordingly.");
				} else {
					error++;
				}
				break;
			case '?':
				error++;
				break;
		}
	}

	if (start_addr < 0)
		errmsg_die("Can't specify negative offset with option -s: %lld",
				start_addr);

	if (length < 0)
		errmsg_die("Can't specify negative length with option -l: %lld", length);

	if (quiet && pretty_print) {
		fprintf(stderr, "The quiet and pretty print options are mutually-\n"
				"exclusive. Choose one or the other.\n");
		exit(EXIT_FAILURE);
	}

	if (forcebinary && pretty_print) {
		fprintf(stderr, "The forcebinary and pretty print options are\n"
				"mutually-exclusive. Choose one or the "
				"other.\n");
		exit(EXIT_FAILURE);
	}

	if (bb_default)
		warnmsg("you did not specify a default bad-block handling\n"
			"  method. In future versions, the default will change to\n"
			"  --bb=skipbad. Use \"nanddump --help\" for more information.");

	if (oob_default)
		warnmsg("in next release, nanddump will not dump OOB\n"
			"  by default. Use `nanddump --oob' explicitly to ensure\n"
			"  it is dumped.");

	if ((argc - optind) != 1 || error)
		display_help();

	mtddev = argv[optind];
}

#define PRETTY_ROW_SIZE 16
#define PRETTY_BUF_LEN 80

/**
 * pretty_dump_to_buffer - formats a blob of data to "hex ASCII" in memory
 * @buf: data blob to dump
 * @len: number of bytes in the @buf
 * @linebuf: where to put the converted data
 * @linebuflen: total size of @linebuf, including space for terminating NULL
 * @pagedump: true - dumping as page format; false - dumping as OOB format
 * @ascii: dump ascii formatted data next to hexdump
 * @prefix: address to print before line in a page dump, ignored if !pagedump
 *
 * pretty_dump_to_buffer() works on one "line" of output at a time, i.e.,
 * PRETTY_ROW_SIZE bytes of input data converted to hex + ASCII output.
 *
 * Given a buffer of unsigned char data, pretty_dump_to_buffer() converts the
 * input data to a hex/ASCII dump at the supplied memory location. A prefix
 * is included based on whether we are dumping page or OOB data. The converted
 * output is always NULL-terminated.
 *
 * e.g.
 *   pretty_dump_to_buffer(data, data_len, prettybuf, linelen, true,
 *                         false, 256);
 * produces:
 *   0x00000100: 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f
 * NOTE: This function was adapted from linux kernel, "lib/hexdump.c"
 */
static void pretty_dump_to_buffer(const unsigned char *buf, size_t len,
		char *linebuf, size_t linebuflen, bool pagedump, bool ascii,
		unsigned long long prefix)
{
	static const char hex_asc[] = "0123456789abcdef";
	unsigned char ch;
	unsigned int j, lx = 0, ascii_column;

	if (pagedump)
		lx += sprintf(linebuf, "0x%.8llx: ", prefix);
	else
		lx += sprintf(linebuf, "  OOB Data: ");

	if (!len)
		goto nil;
	if (len > PRETTY_ROW_SIZE)	/* limit to one line at a time */
		len = PRETTY_ROW_SIZE;

	for (j = 0; (j < len) && (lx + 3) <= linebuflen; j++) {
		ch = buf[j];
		linebuf[lx++] = hex_asc[(ch & 0xf0) >> 4];
		linebuf[lx++] = hex_asc[ch & 0x0f];
		linebuf[lx++] = ' ';
	}
	if (j)
		lx--;

	ascii_column = 3 * PRETTY_ROW_SIZE + 14;

	if (!ascii)
		goto nil;

	/* Spacing between hex and ASCII - ensure at least one space */
	lx += sprintf(linebuf + lx, "%*s",
			MAX((int)MIN(linebuflen, ascii_column) - 1 - lx, 1),
			" ");

	linebuf[lx++] = '|';
	for (j = 0; (j < len) && (lx + 2) < linebuflen; j++)
		linebuf[lx++] = (isascii(buf[j]) && isprint(buf[j])) ? buf[j]
			: '.';
	linebuf[lx++] = '|';
nil:
	linebuf[lx++] = '\n';
	linebuf[lx++] = '\0';
}


/*
 * Main program
 */
int main(int argc, char * const argv[])
{
	long long ofs, end_addr = 0;
	long long blockstart = 1;
	int ret, i, fd, ofd = 0, bs, badblock = 0;
	struct mtd_dev_info mtd;
	char pretty_buf[PRETTY_BUF_LEN];
	int oobinfochanged = 0, firstblock = 1;
	struct nand_oobinfo old_oobinfo;
	struct mtd_ecc_stats stat1, stat2;
	bool eccstats = false;
	unsigned char *readbuf = NULL, *oobbuf = NULL;
	libmtd_t mtd_desc;

	process_options(argc, argv);

	/* Initialize libmtd */
	mtd_desc = libmtd_open();
	if (!mtd_desc)
		return errmsg("can't initialize libmtd");

	/* Open MTD device */
	if ((fd = open(mtddev, O_RDONLY)) == -1) {
		perror(mtddev);
		exit(EXIT_FAILURE);
	}

	/* Fill in MTD device capability structure */
	if (mtd_get_dev_info(mtd_desc, mtddev, &mtd) < 0)
		return errmsg("mtd_get_dev_info failed");

	/* Allocate buffers */
	oobbuf = xmalloc(sizeof(oobbuf) * mtd.oob_size);
	readbuf = xmalloc(sizeof(readbuf) * mtd.min_io_size);

	if (noecc)  {
		ret = ioctl(fd, MTDFILEMODE, MTD_MODE_RAW);
		if (ret == 0) {
			oobinfochanged = 2;
		} else {
			switch (errno) {
			case ENOTTY:
				if (ioctl(fd, MEMGETOOBSEL, &old_oobinfo) != 0) {
					perror("MEMGETOOBSEL");
					goto closeall;
				}
				if (ioctl(fd, MEMSETOOBSEL, &none_oobinfo) != 0) {
					perror("MEMSETOOBSEL");
					goto closeall;
				}
				oobinfochanged = 1;
				break;
			default:
				perror("MTDFILEMODE");
				goto closeall;
			}
		}
	} else {
		/* check if we can read ecc stats */
		if (!ioctl(fd, ECCGETSTATS, &stat1)) {
			eccstats = true;
			if (!quiet) {
				fprintf(stderr, "ECC failed: %d\n", stat1.failed);
				fprintf(stderr, "ECC corrected: %d\n", stat1.corrected);
				fprintf(stderr, "Number of bad blocks: %d\n", stat1.badblocks);
				fprintf(stderr, "Number of bbt blocks: %d\n", stat1.bbtblocks);
			}
		} else
			perror("No ECC status information available");
	}

	/* Open output file for writing. If file name is "-", write to standard
	 * output. */
	if (!dumpfile) {
		ofd = STDOUT_FILENO;
	} else if ((ofd = open(dumpfile, O_WRONLY | O_TRUNC | O_CREAT, 0644))== -1) {
		perror(dumpfile);
		goto closeall;
	}

	if (!pretty_print && !forcebinary && isatty(ofd)) {
		fprintf(stderr, "Not printing binary garbage to tty. Use '-a'\n"
				"or '--forcebinary' to override.\n");
		goto closeall;
	}

	/* Initialize start/end addresses and block size */
	if (start_addr & (mtd.min_io_size - 1)) {
		fprintf(stderr, "the start address (-s parameter) is not page-aligned!\n"
				"The pagesize of this NAND Flash is 0x%x.\n",
				mtd.min_io_size);
		goto closeall;
	}
	if (length)
		end_addr = start_addr + length;
	if (!length || end_addr > mtd.size)
		end_addr = mtd.size;

	bs = mtd.min_io_size;

	/* Print informative message */
	if (!quiet) {
		fprintf(stderr, "Block size %d, page size %d, OOB size %d\n",
				mtd.eb_size, mtd.min_io_size, mtd.oob_size);
		fprintf(stderr,
				"Dumping data starting at 0x%08llx and ending at 0x%08llx...\n",
				start_addr, end_addr);
	}

	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr; ofs += bs) {
		/* Check for bad block */
		if (bb_method == dumpbad) {
			badblock = 0;
		} else if (blockstart != (ofs & (~mtd.eb_size + 1)) ||
				firstblock) {
			blockstart = ofs & (~mtd.eb_size + 1);
			firstblock = 0;
			if ((badblock = mtd_is_bad(&mtd, fd, ofs / mtd.eb_size)) < 0) {
				errmsg("libmtd: mtd_is_bad");
				goto closeall;
			}
		}

		if (badblock) {
			/* skip bad block, increase end_addr */
			if (bb_method == skipbad) {
				end_addr += mtd.eb_size;
				ofs += mtd.eb_size - bs;
				if (end_addr > mtd.size)
					end_addr = mtd.size;
				continue;
			}
			if (bb_method == omitbad)
				continue;
			memset(readbuf, 0xff, bs);
		} else {
			/* Read page data and exit on failure */
			if (mtd_read(&mtd, fd, ofs / mtd.eb_size, ofs % mtd.eb_size, readbuf, bs)) {
				errmsg("mtd_read");
				goto closeall;
			}
		}

		/* ECC stats available ? */
		if (eccstats) {
			if (ioctl(fd, ECCGETSTATS, &stat2)) {
				perror("ioctl(ECCGETSTATS)");
				goto closeall;
			}
			if (stat1.failed != stat2.failed)
				fprintf(stderr, "ECC: %d uncorrectable bitflip(s)"
						" at offset 0x%08llx\n",
						stat2.failed - stat1.failed, ofs);
			if (stat1.corrected != stat2.corrected)
				fprintf(stderr, "ECC: %d corrected bitflip(s) at"
						" offset 0x%08llx\n",
						stat2.corrected - stat1.corrected, ofs);
			stat1 = stat2;
		}

		/* Write out page data */
		if (pretty_print) {
			for (i = 0; i < bs; i += PRETTY_ROW_SIZE) {
				pretty_dump_to_buffer(readbuf + i, PRETTY_ROW_SIZE,
						pretty_buf, PRETTY_BUF_LEN, true, canonical, ofs + i);
				write(ofd, pretty_buf, strlen(pretty_buf));
			}
		} else
			write(ofd, readbuf, bs);

		if (omitoob)
			continue;

		if (badblock) {
			memset(oobbuf, 0xff, mtd.oob_size);
		} else {
			/* Read OOB data and exit on failure */
			if (mtd_read_oob(mtd_desc, &mtd, fd, ofs, mtd.oob_size, oobbuf)) {
				errmsg("libmtd: mtd_read_oob");
				goto closeall;
			}
		}

		/* Write out OOB data */
		if (pretty_print) {
			for (i = 0; i < mtd.oob_size; i += PRETTY_ROW_SIZE) {
				pretty_dump_to_buffer(oobbuf + i, mtd.oob_size - i,
						pretty_buf, PRETTY_BUF_LEN, false, canonical, 0);
				write(ofd, pretty_buf, strlen(pretty_buf));
			}
		} else
			write(ofd, oobbuf, mtd.oob_size);
	}

	/* reset oobinfo */
	if (oobinfochanged == 1) {
		if (ioctl(fd, MEMSETOOBSEL, &old_oobinfo) != 0) {
			perror("MEMSETOOBSEL");
			close(fd);
			close(ofd);
			return EXIT_FAILURE;
		}
	}
	/* Close the output file and MTD device, free memory */
	close(fd);
	close(ofd);
	free(oobbuf);
	free(readbuf);

	/* Exit happy */
	return EXIT_SUCCESS;

closeall:
	/* The new mode change is per file descriptor ! */
	if (oobinfochanged == 1) {
		if (ioctl(fd, MEMSETOOBSEL, &old_oobinfo) != 0)  {
			perror("MEMSETOOBSEL");
		}
	}
	close(fd);
	close(ofd);
	free(oobbuf);
	free(readbuf);
	exit(EXIT_FAILURE);
}
