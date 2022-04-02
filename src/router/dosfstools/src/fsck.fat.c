/* fsck.fat.c - User interface

   Copyright (C) 1993 Werner Almesberger <werner.almesberger@lrc.di.epfl.ch>
   Copyright (C) 1998 Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
   Copyright (C) 2008-2014 Daniel Baumann <mail@daniel-baumann.ch>
   Copyright (C) 2018-2021 Pali Rohár <pali.rohar@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.

   The complete text of the GNU General Public License
   can be found in /usr/share/common-licenses/GPL-3 file.
*/

/* FAT32, VFAT, Atari format support, and various fixes additions May 1998
 * by Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de> */

#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <termios.h>
#include <getopt.h>

#include "common.h"
#include "fsck.fat.h"
#include "io.h"
#include "boot.h"
#include "fat.h"
#include "file.h"
#include "check.h"
#include "charconv.h"

int rw = 0, list = 0, test = 0, verbose = 0;
long fat_table = 0;
int no_spaces_in_sfns = 0;
int only_uppercase_label = 0;
int boot_only = 0;
unsigned n_files = 0;
void *mem_queue = NULL;

static struct termios original_termios;


static void restore_termios(void)
{
    tcsetattr(0, TCSAFLUSH, &original_termios);
}


static void usage(char *name, int exitval)
{
    fprintf(stderr, "Usage: %s [OPTIONS] DEVICE\n", name);
    fprintf(stderr, "Check FAT filesystem on DEVICE for errors.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -a              automatically repair the filesystem\n");
    fprintf(stderr, "  -A              toggle Atari variant of the FAT filesystem\n");
    fprintf(stderr, "  -b              make read-only boot sector check\n");
    fprintf(stderr, "  -c N            use DOS codepage N to decode short file names (default: %d)\n",
	    DEFAULT_DOS_CODEPAGE);
    fprintf(stderr, "  -d PATH         drop file with name PATH (can be given multiple times)\n");
    fprintf(stderr, "  -f              salvage unused chains to files\n");
    fprintf(stderr, "  -F NUM          specify FAT table NUM used for filesystem access\n");
    fprintf(stderr, "  -l              list path names\n");
    fprintf(stderr, "  -n              no-op, check non-interactively without changing\n");
    fprintf(stderr, "  -p              same as -a, for compat with other *fsck\n");
    fprintf(stderr, "  -r              interactively repair the filesystem (default)\n");
    fprintf(stderr, "  -S              disallow spaces in the middle of short file names\n");
    fprintf(stderr, "  -t              test for bad clusters\n");
    fprintf(stderr, "  -u PATH         try to undelete (non-directory) file that was named PATH (can be\n");
    fprintf(stderr, "                    given multiple times)\n");
    fprintf(stderr, "  -U              allow only uppercase characters in volume and boot label\n");
    fprintf(stderr, "  -v              verbose mode\n");
    fprintf(stderr, "  -V              perform a verification pass\n");
    fprintf(stderr, "  --variant=TYPE  handle variant TYPE of the filesystem\n");
    fprintf(stderr, "  -w              write changes to disk immediately\n");
    fprintf(stderr, "  -y              same as -a, for compat with other *fsck\n");
    fprintf(stderr, "  --help          print this message\n");
    exit(exitval);
}

int main(int argc, char **argv)
{
    DOS_FS fs;
    int salvage_files, verify, c;
    uint32_t free_clusters = 0;
    struct termios tio;
    char *tmp;
    long codepage;

    enum {OPT_HELP=1000, OPT_VARIANT};
    const struct option long_options[] = {
	    {"variant", required_argument, NULL, OPT_VARIANT},
	    {"help",    no_argument,       NULL, OPT_HELP},
	    {0,}
    };

    if (!tcgetattr(0, &original_termios)) {
	tio = original_termios;
	tio.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSAFLUSH, &tio);
	atexit(restore_termios);
    }

    memset(&fs, 0, sizeof(fs));
    salvage_files = verify = 0;
    rw = interactive = 1;
    check_atari();

    while ((c = getopt_long(argc, argv, "Aac:d:bfF:lnprStu:UvVwy",
				    long_options, NULL)) != -1)
	switch (c) {
	case 'A':		/* toggle Atari format */
	    atari_format = !atari_format;
	    break;
	case 'a':
	case 'p':
	case 'y':
	    rw = 1;
	    interactive = 0;
	    salvage_files = 1;
	    break;
	case 'b':
	    rw = 0;
	    interactive = 0;
	    boot_only = 1;
	    break;
	case 'c':
	    errno = 0;
	    codepage = strtol(optarg, &tmp, 10);
	    if (!*optarg || isspace(*optarg) || *tmp || errno || codepage < 0 || codepage > INT_MAX) {
		fprintf(stderr, "Invalid codepage : %s\n", optarg);
		usage(argv[0], 2);
	    }
	    if (!set_dos_codepage(codepage))
		usage(argv[0], 2);
	    break;
	case 'd':
	    file_add(optarg, fdt_drop);
	    break;
	case 'f':
	    salvage_files = 1;
	    break;
	case 'F':
	    errno = 0;
	    fat_table = strtol(optarg, &tmp, 10);
	    if (!*optarg || isspace(*optarg) || *tmp || errno || fat_table < 0 || fat_table > 255) {
		fprintf(stderr, "Invalid FAT table : %s\n", optarg);
		usage(argv[0], 2);
	    }
	    break;
	case 'l':
	    list = 1;
	    break;
	case 'n':
	    rw = 0;
	    interactive = 0;
	    break;
	case 'r':
	    rw = 1;
	    interactive = 1;
	    break;
	case 'S':
	    no_spaces_in_sfns = 1;
	    break;
	case 't':
	    test = 1;
	    break;
	case 'u':
	    file_add(optarg, fdt_undelete);
	    break;
	case 'U':
	    only_uppercase_label = 1;
	    break;
	case 'v':
	    verbose = 1;
	    break;
	case 'V':
	    verify = 1;
	    break;
	case OPT_VARIANT:
	    if (!strcasecmp(optarg, "standard")) {
		    atari_format = 0;
	    } else if (!strcasecmp(optarg, "atari")) {
		    atari_format = 1;
	    } else {
		    fprintf(stderr, "Unknown variant: %s\n", optarg);
		    usage(argv[0], 2);
	    }
	    break;
	case 'w':
	    write_immed = 1;
	    break;
	case OPT_HELP:
	    usage(argv[0], 0);
	    break;
	case '?':
	    usage(argv[0], 2);
	    break;
	default:
	    fprintf(stderr,
		    "Internal error: getopt_long() returned unexpected value %d\n", c);
	    exit(3);
	}
    if (!set_dos_codepage(-1))	/* set default codepage if none was given in command line */
        exit(2);
    if ((test || write_immed) && !rw) {
	fprintf(stderr, "-t and -w can not be used in read only mode\n");
	exit(2);
    }
    if (optind != argc - 1)
	usage(argv[0], 2);

    printf("fsck.fat " VERSION " (" VERSION_DATE ")\n");
    fs_open(argv[optind], rw);

    read_boot(&fs);
    if (boot_only)
	goto exit;

    if (verify)
	printf("Starting check/repair pass.\n");
    while (read_fat(&fs, 2), scan_root(&fs))
	qfree(&mem_queue);
    check_label(&fs);
    if (test)
	fix_bad(&fs);
    if (salvage_files)
	reclaim_file(&fs);
    else
	reclaim_free(&fs);
    if (!atari_format)
	check_dirty_bits(&fs);
    free_clusters = update_free(&fs);
    file_unused();
    qfree(&mem_queue);
    if (verify) {
	n_files = 0;
	printf("Starting verification pass.\n");
	read_fat(&fs, 2);
	scan_root(&fs);
	check_label(&fs);
	reclaim_free(&fs);
	if (!atari_format)
	    check_dirty_bits(&fs);
	qfree(&mem_queue);
    }
    release_fat(&fs);

exit:
    if (!write_immed && fs_changed()) {
	if (rw) {
	    printf("\n*** Filesystem was changed ***\n");
	    if (interactive)
		printf("The changes have not yet been written, you can still choose to leave the\n"
		       "filesystem unmodified:\n");

	    rw = get_choice(1, "Writing changes.",
			    2,
			    1, "Write changes",
			    2, "Leave filesystem unchanged") == 1;
	} else
	    printf("\nLeaving filesystem unchanged.\n");
    }

    if (!boot_only)
	printf("%s: %u files, %lu/%lu clusters\n", argv[optind],
	       n_files, (unsigned long)fs.data_clusters - free_clusters,
	       (unsigned long)fs.data_clusters);

    return fs_close(rw) ? 1 : 0;
}
