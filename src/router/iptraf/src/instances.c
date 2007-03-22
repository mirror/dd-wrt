/***

instances.c - handler routines for multiple IPTraf instances

Copyright (c) Gerard Paul Java 1998

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "error.h"
#include "dirs.h"
#include "instances.h"

extern int daemonized;

void gen_lockfile_name(char *tagfile, char *iface, char *result)
{
    if (iface == NULL)
        snprintf(result, 64, "%s.all", tagfile);
    else
        snprintf(result, 64, "%s.%s", tagfile, iface);
}

void mark_facility(char *tagfile, char *facility, char *iface)
{
    int fd;
    char errstring[80];
    char lockfile[64];

    gen_lockfile_name(tagfile, iface, lockfile);
    fd = open(lockfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        snprintf(errstring, 80, "Warning: unable to lock %s on %s",
                 facility, iface);
        write_error(errstring, daemonized);
    }
    close(fd);
    strncpy(active_facility_lockfile, lockfile, 64);
}

void unmark_facility(char *tagfile, char *iface)
{
    char lockfile[64];

    gen_lockfile_name(tagfile, iface, lockfile);
    unlink(lockfile);
    strcpy(active_facility_lockfile, "");
}

int facility_active(char *tagfile, char *iface)
{
    int fd;
    char lockfile[64];

    gen_lockfile_name(tagfile, iface, lockfile);
    fd = open(lockfile, O_RDONLY);

    if (fd < 0)
        return 0;
    else {
        close(fd);
        return 1;
    }
}

/*
 * Increments or decrements the process count
 */

int adjust_instance_count(char *countfile, int inc)
{
    int fd;
    int proccount = 0;
    int brw;

    fd = open(countfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    brw = read(fd, &proccount, sizeof(int));
    if ((brw == 0) || (brw == -1))
        proccount = 0;

    proccount += inc;

    if (proccount < 0)
        proccount = 0;

    lseek(fd, 0, SEEK_SET);
    brw = write(fd, &proccount, sizeof(int));
    close(fd);

    return proccount;
}

int get_instance_count(char *countfile)
{
    int fd;
    int proccount = 0;
    int br;

    fd = open(countfile, O_RDONLY);
    br = read(fd, &proccount, sizeof(int));
    if ((br == 0) || (br == -1))
        proccount = 0;

    close(fd);
    return proccount;
}

/*
 * Returns TRUE if this is the last instance, and is therefore responsible
 * for restoring the promicuous states saved by the first instance.
 *
 * Man, this is getting more complex by the minute :)
 */

int is_last_instance(void)
{
    int fd;
    int proccount = 0;
    int br;

    fd = open(PROCCOUNTFILE, O_RDONLY);
    br = read(fd, &proccount, sizeof(int));
    close(fd);
    return ((proccount == 1) || (br < 0) || fd < 0);
}

/*
 * Returns TRUE if no facilities are currently running in other instances of
 * IPTraf.  Call this before the first invocation of adjust_process_count(1)
 */

int first_active_facility(void)
{
    int fd;
    int proccount = 0;
    int br;

    fd = open(PROCCOUNTFILE, O_RDONLY);
    br = read(fd, &proccount, sizeof(int));
    close(fd);
    return ((proccount == 0) || (br < 0) || (fd < 0));
}
