// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2013 SGI
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include <sys/types.h>
#include <unistd.h>
#include "init.h"
#include "io.h"

static cmdinfo_t seek_cmd;

static void
seek_help(void)
{
	printf(_(
"\n"
" returns the next hole and/or data offset at or after the requested offset\n"
"\n"
" Example:\n"
" 'seek -d 512'		- offset of data at or following offset 512\n"
" 'seek -a -r 0'	- offsets of all data and hole in entire file\n"
"\n"
" Returns the offset of the next data and/or hole. There is an implied hole\n"
" at the end of file. If the specified offset is past end of file, or there\n"
" is no data past the specified offset, EOF is returned.\n"
" -a	-- return the next data and hole starting at the specified offset.\n"
" -d	-- return the next data starting at the specified offset.\n"
" -h	-- return the next hole starting at the specified offset.\n"
" -r	-- return all remaining type(s) starting at the specified offset.\n"
" -s	-- also print the starting offset.\n"
"\n"));
}

#ifndef HAVE_SEEK_DATA
#define	SEEK_DATA	3	/* seek to the next data */
#define	SEEK_HOLE	4	/* seek to the next hole */
#endif

/* values for flag variable */
#define	SEEK_DFLAG	(1 << 0)
#define	SEEK_HFLAG	(1 << 1)
#define	SEEK_RFLAG	(1 << 2)

/* indexes into the seekinfo array */
#define	DATA		0
#define	HOLE		1

static struct seekinfo {
	char		*name;		/* display item name */
	int		seektype;	/* data or hole */
	int		mask;		/* compared for print and looping */
} seekinfo[] = {
		{"DATA", SEEK_DATA, SEEK_DFLAG},
		{"HOLE", SEEK_HOLE, SEEK_HFLAG}
};

/* print item type and offset. catch special cases of eof and error */
static void
seek_output(
	int	startflag,
	char	*type,
	off64_t	start,
	off64_t	offset)
{
	if (offset == -1) {
		if (errno == ENXIO) {
			if (startflag)
				printf("%s	%lld	EOF\n", type,
					 (long long)start);
			else
				printf("%s	EOF\n", type);
		} else {
			printf("ERR	%lld	", (long long)start);
			fflush(stdout);	/* so the printf preceded the perror */
			perror("");
		}
	} else {
		if (startflag)
			printf("%s	%lld	%lld\n", type,
				(long long)start, (long long)offset);
		else
			printf("%s	%lld\n", type, (long long)offset);
	}
}

static int
seek_f(
	int	argc,
	char	**argv)
{
	off64_t		offset, start;
	size_t		fsblocksize, fssectsize;
	int		c;
	int		current;	/* specify data or hole */
	int		flag;
	int		startflag;

	flag = startflag = 0;
	init_cvtnum(&fsblocksize, &fssectsize);

	while ((c = getopt(argc, argv, "adhrs")) != EOF) {
		switch (c) {
		case 'a':
			flag |= (SEEK_HFLAG | SEEK_DFLAG);
			break;
		case 'd':
			flag |= SEEK_DFLAG;
			break;
		case 'h':
			flag |= SEEK_HFLAG;
			break;
		case 'r':
			flag |= SEEK_RFLAG;
			break;
		case 's':
			startflag = 1;
			break;
		default:
			exitcode = 1;
			return command_usage(&seek_cmd);
		}
	}
	if (!(flag & (SEEK_DFLAG | SEEK_HFLAG)) || optind != argc - 1) {
		exitcode = 1;
		return command_usage(&seek_cmd);
	}

	start = offset = cvtnum(fsblocksize, fssectsize, argv[optind]);
	if (offset < 0) {
		exitcode = 1;
		return command_usage(&seek_cmd);
	}

	/*
	 * check to see if the offset is a data or hole entry and
	 * decide if we want to display that type of entry.
	 */
	if (flag & SEEK_HFLAG) {
		current = HOLE;
		offset = lseek(file->fd, start, SEEK_HOLE);
		if (offset != -1 && offset < start)
			goto bad_result;
		if ((start == offset) || !(flag & SEEK_DFLAG)) {
			/*
			 * this offset is a hole or are only displaying holes.
			 * if this offset is for data and we are displaying
			 * data, then we will fall through below to
			 * initialize the data search.
			 */
			goto found_hole;
		}
	}

	/* The offset is not a hole, or we are looking just for data */
	current = DATA;
	offset = lseek(file->fd, start, SEEK_DATA);
	if (offset != -1 && offset < start)
		goto bad_result;

found_hole:
	/*
	 * At this point we know which type and the offset of the starting
	 * item. "current" alternates between data / hole entries in
	 * assending order - this alternation is needed even if only one
	 * type is to be displayed.
	 *
	 * An error or EOF will terminate the display, otherwise "flag"
	 * determines if there are more items to be displayed.
	 */
	if (startflag)
		printf("Whence	Start	Result\n");
	else
		printf("Whence	Result\n");

	for (c = 0; flag; c++) {
		if (offset == -1) {
			/* print error or eof if the only entry */
			if (errno != ENXIO || c == 0 ) {
				seek_output(startflag, seekinfo[current].name,
					    start, offset);
			}
			return 0;	/* stop on error or EOF */
		}

		if (flag & seekinfo[current].mask)
			seek_output(startflag, seekinfo[current].name, start,
				    offset);

		/*
		 * When displaying only a single data and/or hole item, mask
		 * off the item as it is displayed. The loop will end when all
		 * requested items have been displayed.
		 */
		if (!(flag & SEEK_RFLAG))
			flag &= ~seekinfo[current].mask;

		current ^= 1;		/* alternate between data and hole */
		start = offset;
		offset = lseek(file->fd, start, seekinfo[current].seektype);
		if (offset != -1 && offset <= start)
			goto bad_result;
	}
	return 0;

bad_result:
	fprintf(stderr, "Invalid seek result: lseek(<fd>, %lld, SEEK_%s) = %lld\n",
		(long long)start, seekinfo[current].name, (long long)offset);
	return 0;
}

void
seek_init(void)
{
	seek_cmd.name = "seek";
	seek_cmd.cfunc = seek_f;
	seek_cmd.argmin = 2;
	seek_cmd.argmax = 5;
	seek_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	seek_cmd.args = _("-a | -d | -h [-r] off");
	seek_cmd.oneline = _("locate the next data and/or hole");
	seek_cmd.help = seek_help;

	add_command(&seek_cmd);
}
