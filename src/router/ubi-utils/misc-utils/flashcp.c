/*
 * Copyright (c) 2d3D, Inc.
 * Written by Abraham vd Merwe <abraham@2d3d.co.za>
 * All rights reserved.
 *
 * Renamed to flashcp.c to avoid conflicts with fcp from fsh package
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *	  may be used to endorse or promote products derived from this software
 *	  without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define PROGRAM_NAME "flashcp"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <getopt.h>

#include "common.h"

/* for debugging purposes only */
#ifdef DEBUG
#undef DEBUG
#define DEBUG(fmt,args...) { fprintf (stderr,"%d: ",__LINE__); fprintf (stderr,fmt,## args); }
#else
#undef DEBUG
#define DEBUG(fmt,args...)
#endif

#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))

/* cmd-line flags */
#define FLAG_NONE		0x00
#define FLAG_HELP		0x02
#define FLAG_FILENAME	0x04
#define FLAG_DEVICE		0x08
#define FLAG_ERASE_ALL	0x10
#define FLAG_PARTITION	0x20
#define FLAG_WR_LAST	0x40

/* error levels */
#define LOG_NORMAL	1
#define LOG_ERROR	2

static NORETURN void log_failure (const char *fmt, ...)
{
	va_list ap;
	va_start (ap,fmt);
	vfprintf (stderr,fmt,ap);
	va_end (ap);
	fflush (stderr);

	exit (EXIT_FAILURE);
}

static int verbose = 0;
static void log_verbose (const char *fmt, ...)
{
	va_list ap;

	if (!verbose)
		return;

	va_start (ap,fmt);
	vfprintf (stdout,fmt,ap);
	va_end (ap);
	fflush (stdout);
}

static NORETURN void showusage(bool error)
{
	fprintf (error ? stderr : stdout,
			"\n"
			"Flash Copy - Written by Abraham van der Merwe <abraham@2d3d.co.za>\n"
			"\n"
			"usage: %1$s [ -v | --verbose | -A | --erase-all ] <filename> <device>\n"
			"       %1$s -h | --help\n"
			"       %1$s -V | --version\n"
			"\n"
			"   -h | --help           Show this help message\n"
			"   -v | --verbose        Show progress reports\n"
			"   -p | --partition      Only copy different block from file to device\n"
			"   -A | --erase-all      Erases the whole device regardless of the image size\n"
			"   -l | --wr-last=bytes  Write the first [bytes] last\n"
			"   -V | --version        Show version information and exit\n"
			"   <filename>            File which you want to copy to flash\n"
			"   <device>              Flash device node or 'mtd:<name>' to write to (e.g. /dev/mtd0, /dev/mtd1, mtd:data, etc.)\n"
			"\n",
			PROGRAM_NAME);

	exit (error ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int safe_open (const char *pathname,int flags)
{
	const char *access = "unknown";
	int fd;

	fd = open (pathname,flags);
	if (fd < 0)
	{
		if (flags & O_RDWR)
			access = "read/write";
		else if (flags & O_RDONLY)
			access = "read";
		else if (flags & O_WRONLY)
			access = "write";

		log_failure ("While trying to open %s for %s access: %m\n",pathname,access);
	}

	return (fd);
}

static void safe_read (int fd,const char *filename,void *buf,size_t count)
{
	ssize_t result;

	result = read (fd,buf,count);
	if (count != result)
	{
		log_verbose ("\n");
		if (result < 0)
		{
			log_failure("While reading data from %s: %m\n",filename);
		}
		log_failure("Short read count returned while reading from %s\n",filename);
	}
}

static void safe_write (int fd,const void *buf,size_t count,size_t written,unsigned long long to_write,const char *device)
{
	ssize_t result;

	/* write to device */
	result = write (fd,buf,count);
	if (count != result)
	{
		log_verbose ("\n");
		if (result < 0)
		{
			log_failure("While writing data to 0x%.8lx-0x%.8lx on %s: %m\n",
					written,written + count,device);
		}
		log_failure("Short write count returned while writing to x%.8zx-0x%.8zx on %s: %zu/%llu bytes written to flash\n",
				written,written + count,device,written + result,to_write);
	}
}

static off_t safe_lseek (int fd,off_t offset,int whence,const char *filename)
{
	off_t off;

	off = lseek (fd,offset,whence);
	if (off < 0)
	{
		log_failure("While seeking on %s: %m\n",filename);
	}

	return off;
}

static void safe_rewind (int fd,const char *filename)
{
	safe_lseek(fd,0L,SEEK_SET,filename);
}

static void safe_memerase (int fd,const char *device,struct erase_info_user *erase)
{
	if (ioctl (fd,MEMERASE,erase) < 0)
	{
		log_verbose ("\n");
		log_failure("While erasing blocks 0x%.8x-0x%.8x on %s: %m\n",
				(unsigned int) erase->start,(unsigned int) (erase->start + erase->length),device);
	}
}

/******************************************************************************/

static int dev_fd = -1,fil_fd = -1;

static void cleanup (void)
{
	if (dev_fd > 0) close (dev_fd);
	if (fil_fd > 0) close (fil_fd);
}

int main (int argc,char *argv[])
{
	const char *filename = NULL,*device = NULL;
	int i,flags = FLAG_NONE;
	size_t size,written;
	struct mtd_info_user mtd;
	struct erase_info_user erase;
	struct stat filestat;
	unsigned char *src,*dest,*wrlast_buf;
	unsigned long long wrlast_len = 0;
	int error = 0;

	/*********************
	 * parse cmd-line
	 *****************/

	for (;;) {
		int option_index = 0;
		static const char *short_options = "hvpAl:V";
		static const struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"verbose", no_argument, 0, 'v'},
			{"partition", no_argument, 0, 'p'},
			{"erase-all", no_argument, 0, 'A'},
			{"wr-last", required_argument, 0, 'l'},
			{"version", no_argument, 0, 'V'},
			{0, 0, 0, 0},
		};

		int c = getopt_long(argc, argv, short_options,
				long_options, &option_index);
		if (c == EOF) {
			break;
		}

		switch (c) {
			case 'h':
				flags |= FLAG_HELP;
				DEBUG("Got FLAG_HELP\n");
				break;
			case 'v':
				verbose = 1;
				DEBUG("Got FLAG_VERBOSE\n");
				break;
			case 'p':
				flags |= FLAG_PARTITION;
				DEBUG("Got FLAG_PARTITION");
				break;
			case 'A':
				flags |= FLAG_ERASE_ALL;
				DEBUG("Got FLAG_ERASE_ALL\n");
				break;
			case 'l':
				flags |= FLAG_WR_LAST;
				wrlast_len = simple_strtoll(optarg, &error);
				break;
			case 'V':
				common_print_version();
				exit(EXIT_SUCCESS);
				break;
			default:
				DEBUG("Unknown parameter: %s\n",argv[option_index]);
				showusage(true);
		}
	}
	if (optind+2 == argc) {
		flags |= FLAG_FILENAME;
		filename = argv[optind];
		DEBUG("Got filename: %s\n",filename);

		flags |= FLAG_DEVICE;
		device = mtd_find_dev_node(argv[optind+1]);
		if (!device)
			log_failure("Failed to find device %s\n", argv[optind+1]);

		DEBUG("Got device: %s\n",device);
	}

	if (flags & FLAG_HELP || device == NULL)
		showusage(flags != FLAG_HELP);

	if (flags & FLAG_PARTITION && flags & FLAG_ERASE_ALL)
		log_failure("Option --partition does not support --erase-all\n");

	if (flags & FLAG_PARTITION && flags & FLAG_WR_LAST) {
		log_failure("Option --partition does not support --wr-last\n");
	}

	atexit (cleanup);

	/* get some info about the flash device */
	dev_fd = safe_open (device,O_SYNC | O_RDWR);
	if (ioctl (dev_fd,MEMGETINFO,&mtd) < 0)
	{
		DEBUG("ioctl(): %m\n");
		log_failure("This doesn't seem to be a valid MTD flash device!\n");
	}

	/* get some info about the file we want to copy */
	fil_fd = safe_open (filename,O_RDONLY);
	if (fstat (fil_fd,&filestat) < 0)
		log_failure("While trying to get the file status of %s: %m\n",filename);

	/* does it fit into the device/partition? */
	if (filestat.st_size > mtd.size)
		log_failure("%s won't fit into %s!\n",filename,device);

	src = malloc(mtd.erasesize);
	if (!src)
		log_failure("Malloc failed");

	dest = malloc(mtd.erasesize);
	if (!dest)
		log_failure("Malloc failed");

	/* diff block flashcp */
	if (flags & FLAG_PARTITION)
	{
		goto DIFF_BLOCKS;
	}

	/*****************************************************
	 * erase enough blocks so that we can write the file *
	 *****************************************************/

#warning "Check for smaller erase regions"

	erase.start = 0;

	if (flags & FLAG_ERASE_ALL)
	{
		erase.length = mtd.size;
	}
	else
	{
		erase.length = (filestat.st_size + mtd.erasesize - 1) / mtd.erasesize;
		erase.length *= mtd.erasesize;
	}

	if (verbose)
	{
		/* if the user wants verbose output, erase 1 block at a time and show him/her what's going on */
		int blocks = erase.length / mtd.erasesize;
		erase.length = mtd.erasesize;
		log_verbose ("Erasing blocks: 0/%d (0%%)",blocks);
		for (i = 1; i <= blocks; i++)
		{
			log_verbose ("\rErasing blocks: %d/%d (%d%%)",i,blocks,PERCENTAGE (i,blocks));
			safe_memerase(dev_fd,device,&erase);
			erase.start += mtd.erasesize;
		}
		log_verbose ("\rErasing blocks: %d/%d (100%%)\n",blocks,blocks);
	}
	else
	{
		/* if not, erase the whole chunk in one shot */
		safe_memerase(dev_fd,device,&erase);
	}
	DEBUG("Erased %u / %luk bytes\n",erase.length,filestat.st_size);

	/**********************************
	 * write the entire file to flash *
	 **********************************/

	size = filestat.st_size;
	i = mtd.erasesize;
	written = 0;

	if ((flags & FLAG_WR_LAST) && (filestat.st_size > wrlast_len)) {
		if (wrlast_len > mtd.erasesize)
			log_failure("The wrlast (%lluk) is larger than erasesize (%lluk)\n", KB (wrlast_len), KB ((unsigned long long)mtd.erasesize));

		if (size < mtd.erasesize) i = size;

		log_verbose ("Reading %lluk of data to write last.\n", KB ((unsigned long long)wrlast_len));
		wrlast_buf = malloc(wrlast_len);
		if (!wrlast_buf)
			log_failure("Malloc failed");
		safe_read (fil_fd, filename, wrlast_buf, wrlast_len);
		safe_lseek(dev_fd, wrlast_len, SEEK_SET, device);
		written += wrlast_len;
		size -= wrlast_len;
		i -= wrlast_len;

		log_verbose ("Writing remaining erase block data: %dk/%lluk (%llu%%)\n",
				KB (written + i),
				KB ((unsigned long long)filestat.st_size),
				PERCENTAGE ((unsigned long long)written + i, (unsigned long long)filestat.st_size));
		safe_read (fil_fd, filename, src, i);
		safe_write(dev_fd, src, i, written, (unsigned long long)filestat.st_size, device);

		written += i;
		size -= i;
		i = mtd.erasesize;
	} else {
		log_verbose ("Writing data: 0k/%lluk (0%%)",KB ((unsigned long long)filestat.st_size));
	}

	while (size)
	{
		if (size < mtd.erasesize) i = size;
		log_verbose ("\rWriting data: %dk/%lluk (%llu%%)",
				KB (written + i),
				KB ((unsigned long long)filestat.st_size),
				PERCENTAGE ((unsigned long long)written + i,(unsigned long long)filestat.st_size));

		/* read from filename */
		safe_read (fil_fd,filename,src,i);

		/* write to device */
		safe_write(dev_fd,src,i,written,(unsigned long long)filestat.st_size,device);

		written += i;
		size -= i;
	}
	log_verbose ("\rWriting data: %lluk/%lluk (100%%)\n",
			KB ((unsigned long long)filestat.st_size),
			KB ((unsigned long long)filestat.st_size));
	DEBUG("Wrote %d / %lluk bytes\n",written,(unsigned long long)filestat.st_size);

	if ((flags & FLAG_WR_LAST) && (filestat.st_size > wrlast_len)) {
		log_verbose ("Writing %lluk of the write last data.\n", KB ((unsigned long long)wrlast_len));
		safe_rewind (dev_fd, device);
		safe_write(dev_fd, wrlast_buf, wrlast_len, 0, wrlast_len, device);
	}

	/**********************************
	 * verify that flash == file data *
	 **********************************/

	safe_rewind (fil_fd,filename);
	safe_rewind (dev_fd,device);
	size = filestat.st_size;
	i = mtd.erasesize;
	written = 0;
	log_verbose ("Verifying data: 0k/%lluk (0%%)",KB ((unsigned long long)filestat.st_size));
	while (size)
	{
		if (size < mtd.erasesize) i = size;
		log_verbose ("\rVerifying data: %luk/%lluk (%llu%%)",
				KB (written + i),
				KB ((unsigned long long)filestat.st_size),
				PERCENTAGE ((unsigned long long)written + i,(unsigned long long)filestat.st_size));

		/* read from filename */
		safe_read (fil_fd,filename,src,i);

		/* read from device */
		safe_read (dev_fd,device,dest,i);

		/* compare buffers */
		if (memcmp (src,dest,i))
			log_failure("File does not seem to match flash data. First mismatch at 0x%.8zx-0x%.8zx\n",
					written,written + i);

		written += i;
		size -= i;
	}
	log_verbose ("\rVerifying data: %lluk/%lluk (100%%)\n",
			KB ((unsigned long long)filestat.st_size),
			KB ((unsigned long long)filestat.st_size));
	DEBUG("Verified %d / %lluk bytes\n",written,(unsigned long long)filestat.st_size);

	exit (EXIT_SUCCESS);

	/*********************************************
	 * Copy different blocks from file to device *
	 ********************************************/
DIFF_BLOCKS:
	safe_rewind (fil_fd,filename);
	safe_rewind (dev_fd,device);
	size = filestat.st_size;
	i = mtd.erasesize;
	erase.start = 0;
	erase.length = (filestat.st_size + mtd.erasesize - 1) / mtd.erasesize;
	erase.length *= mtd.erasesize;
	written = 0;
	unsigned long current_dev_block = 0;
	int diffBlock = 0;
	int blocks = erase.length / mtd.erasesize;
	erase.length = mtd.erasesize;

	log_verbose ("\rProcessing blocks: 0/%d (%d%%)", blocks, PERCENTAGE (0,blocks));
	for (int s = 1; s <= blocks; s++)
	{
		if (size < mtd.erasesize) i = size;
		log_verbose ("\rProcessing blocks: %d/%d (%d%%)", s, blocks, PERCENTAGE (s,blocks));

		/* read from filename */
		safe_read (fil_fd,filename,src,i);

		/* read from device */
		current_dev_block = safe_lseek(dev_fd, 0, SEEK_CUR, device);
		safe_read (dev_fd,device,dest,i);

		/* compare buffers, if not the same, erase and write the block */
		if (memcmp (src,dest,i))
		{
			diffBlock++;
			/* erase block */
			safe_lseek(dev_fd, current_dev_block, SEEK_SET, device);
			safe_memerase(dev_fd,device,&erase);

			/* write to device */
			safe_lseek(dev_fd, current_dev_block, SEEK_SET, device);
			safe_write(dev_fd,src,i,written,(unsigned long long)filestat.st_size,device);

			/* read from device */
			safe_lseek(dev_fd, current_dev_block, SEEK_SET, device);
			safe_read (dev_fd,device,dest,i);

			/* compare buffers for write success */
			if (memcmp (src,dest,i))
				log_failure("File does not seem to match flash data. First mismatch at 0x%.8zx-0x%.8zx\n",
						written,written + i);
		}

		erase.start += i;
		written += i;
		size -= i;
	}

	log_verbose ("\ndiff blocks: %d\n", diffBlock);

	exit (EXIT_SUCCESS);
}
