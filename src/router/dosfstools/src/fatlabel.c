/* fatlabel.c - User interface

   Copyright (C) 1993 Werner Almesberger <werner.almesberger@lrc.di.epfl.ch>
   Copyright (C) 1998 Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
   Copyright (C) 2007 Red Hat, Inc.
   Copyright (C) 2008-2014 Daniel Baumann <mail@daniel-baumann.ch>
   Copyright (C) 2015-2017 Andreas Bombe <aeb@debian.org>
   Copyright (C) 2017-2018 Pali Rohár <pali.rohar@gmail.com>

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

#include "version.h"

#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>

#include "common.h"
#include "fsck.fat.h"
#include "io.h"
#include "boot.h"
#include "fat.h"
#include "file.h"
#include "check.h"
#include "charconv.h"

int rw = 0, list = 0, test = 0, verbose = 0, no_spaces_in_sfns = 0;
long fat_table = 0;
unsigned n_files = 0;
void *mem_queue = NULL;


static void handle_label(bool change, bool reset, const char *device, char *newlabel)
{
    DOS_FS fs = { 0 };
    off_t offset;
    DIR_ENT de;

    char label[12] = { 0 };
    size_t len;
    int ret;
    int i;

    if (change) {
	len = mbstowcs(NULL, newlabel, 0);
	if (len != (size_t)-1 && len > 11) {
	    fprintf(stderr,
		    "fatlabel: labels can be no longer than 11 characters\n");
	    exit(1);
	}

	if (!local_string_to_dos_string(label, newlabel, 12)) {
	    fprintf(stderr,
		    "fatlabel: error when processing label\n");
	    exit(1);
	}

	for (i = strlen(label); i < 11; ++i)
	    label[i] = ' ';
	label[11] = 0;

	ret = validate_volume_label(label);
	if (ret & 0x1) {
	    fprintf(stderr,
		    "fatlabel: warning - lowercase labels might not work properly on some systems\n");
	}
	if (ret & 0x2) {
	    fprintf(stderr,
		    "fatlabel: labels with characters below 0x20 are not allowed\n");
	    exit(1);
	}
	if (ret & 0x4) {
	    fprintf(stderr,
		    "fatlabel: labels with characters *?.,;:/\\|+=<>[]\" are not allowed\n");
	    exit(1);
	}
	if (ret & 0x08) {
	    fprintf(stderr,
		    "fatlabel: labels can't be empty or white space only\n");
	    exit(1);
	}
	if (ret & 0x10) {
	    fprintf(stderr,
		    "fatlabel: labels can't start with a space character\n");
	    exit(1);
	}
    }

    fs_open(device, rw);
    read_boot(&fs);

    if (!change && !reset) {
	if (fs.fat_bits == 32)
	    read_fat(&fs, 0);

	offset = find_volume_de(&fs, &de);
	if (offset != 0) {
	    if (de.name[0] == 0x05)
		de.name[0] = 0xe5;
	    printf("%s\n", pretty_label((char *)de.name));
	}

	if (fs.fat_bits == 32)
	    release_fat(&fs);

	exit(0);
    }

    if (fs.fat_bits == 32)
	read_fat(&fs, 1);

    if (!reset)
	write_label(&fs, label);
    else
	remove_label(&fs);

    if (fs.fat_bits == 32)
	release_fat(&fs);
}


static void handle_volid(bool change, bool reset, const char *device, const char *newserial)
{
    DOS_FS fs = { 0 };
    char *tmp;
    long long conversion;
    uint32_t serial = 0;

    if (change) {
	errno = 0;
	conversion = strtoll(newserial, &tmp, 16);

	if (!*newserial || isspace(*newserial) || *tmp || conversion < 0) {
	    fprintf(stderr, "fatlabel: volume ID must be a hexadecimal number\n");
	    exit(1);
	}
	if (conversion > UINT32_MAX) {
	    fprintf(stderr, "fatlabel: given volume ID does not fit in 32 bit\n");
	    exit(1);
	}
	if (errno) {
	    fprintf(stderr, "fatlabel: parsing volume ID failed (%s)\n", strerror(errno));
	    exit(1);
	}

	serial = conversion;
    }

    if (reset)
	serial = generate_volume_id();

    fs_open(device, rw);
    read_boot(&fs);
    if (!change && !reset) {
	printf("%08x\n", fs.serial);
	exit(0);
    }

    write_serial(&fs, serial);
}


static void usage(int error, int usage_only)
{
    FILE *f = error ? stderr : stdout;
    int status = error ? 1 : 0;

    fprintf(f, "Usage: fatlabel [OPTIONS] DEVICE [NEW]\n");
    if (usage_only)
	exit(status);

    fprintf(f, "Change the FAT filesystem label or serial on DEVICE to NEW or display the\n");
    fprintf(f, "existing label or serial if NEW is not given.\n");
    fprintf(f, "\n");
    fprintf(f, "Options:\n");
    fprintf(f, "  -i, --volume-id     Work on serial number instead of label\n");
    fprintf(f, "  -r, --reset         Remove label or generate new serial number\n");
    fprintf(f, "  -c N, --codepage=N  use DOS codepage N to encode/decode label (default: %d)\n", DEFAULT_DOS_CODEPAGE);
    fprintf(f, "  -V, --version       Show version number and terminate\n");
    fprintf(f, "  -h, --help          Print this message and terminate\n");
    exit(status);
}


int main(int argc, char *argv[])
{
    const struct option long_options[] = {
	{"volume-id", no_argument, NULL, 'i'},
	{"reset",     no_argument, NULL, 'r'},
	{"codepage",  required_argument, NULL, 'c'},
	{"version",   no_argument, NULL, 'V'},
	{"help",      no_argument, NULL, 'h'},
	{0,}
    };
    bool change;
    bool reset = false;
    bool volid_mode = false;
    char *device = NULL;
    char *new = NULL;
    char *tmp;
    long codepage;
    int c;

    check_atari();

    while ((c = getopt_long(argc, argv, "irc:Vh", long_options, NULL)) != -1) {
	switch (c) {
	    case 'i':
		volid_mode = 1;
		break;

	    case 'r':
		reset = true;
		break;

	    case 'c':
		errno = 0;
		codepage = strtol(optarg, &tmp, 10);
		if (!*optarg || isspace(*optarg) || *tmp || errno || codepage < 0 || codepage > INT_MAX) {
		    fprintf(stderr, "Invalid codepage : %s\n", optarg);
		    usage(1, 0);
		}
		if (!set_dos_codepage(codepage))
		    usage(1, 0);
		break;

	    case 'V':
		printf("fatlabel " VERSION " (" VERSION_DATE ")\n");
		exit(0);
		break;

	    case 'h':
		usage(0, 0);
		break;

	    case '?':
		usage(1, 0);
		exit(1);

	    default:
		fprintf(stderr,
			"Internal error: getopt_long() returned unexpected value %d\n", c);
		exit(2);
	}
    }

    if (!set_dos_codepage(-1))	/* set default codepage if none was given in command line */
        exit(1);

    if (optind == argc - 2) {
	change = true;
    } else if (optind == argc - 1) {
	change = false;
    } else {
	usage(1, 1);
    }

    if (change || reset)
	rw = 1;

    if (change && reset) {
	fprintf(stderr, "fatlabel: giving new value with --reset not allowed\n");
	exit(1);
    }

    device = argv[optind++];
    if (change)
	new = argv[optind];

    if (!volid_mode)
	handle_label(change, reset, device, new);
    else
	handle_volid(change, reset, device, new);

    fs_close(rw);
    return 0;
}
