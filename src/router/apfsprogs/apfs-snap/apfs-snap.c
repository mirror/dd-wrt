/*
 * Copyright (C) 2022 Ernesto A. Fernández <ernesto@corellium.com>
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "version.h"

static char *progname;

/**
 * usage - Print usage information and exit
 */
static void usage(void)
{
	fprintf(stderr, "usage: %s mountpoint name\n", progname);
	exit(1);
}

/**
 * version - Print version information and exit
 */
static void version(void)
{
	if (*GIT_COMMIT)
		printf("apfs-snap %s\n", GIT_COMMIT);
	else
		printf("apfs-snap - unknown git commit id\n");
	exit(1);
}

/**
 * system_error - Print a system error message and exit
 */
static __attribute__((noreturn)) void system_error(void)
{
	perror(progname);
	exit(1);
}

/**
 * fatal - Print a message and exit with an error code
 * @message: text to print
 */
static __attribute__((noreturn)) void fatal(const char *message)
{
	fprintf(stderr, "%s: %s\n", progname, message);
	exit(1);
}

#define APFS_SNAP_MAX_NAMELEN	255
struct apfs_ioctl_snap_name {
	char name[APFS_SNAP_MAX_NAMELEN + 1];
};
#define APFS_IOC_TAKE_SNAPSHOT	_IOW('@', 0x85, struct apfs_ioctl_snap_name)

/**
 * create_snapshot - Submit the snapshot creation request to the driver
 * @fd:		open file descriptor for the mountpoint
 * @name:	label for the snapshot
 */
static void create_snapshot(int fd, const char *name)
{
	static struct apfs_ioctl_snap_name arg = {0};

	if (strlen(name) > APFS_SNAP_MAX_NAMELEN)
		fatal("snapshot label is too long");
	strcpy(arg.name, name);

	if (ioctl(fd, APFS_IOC_TAKE_SNAPSHOT, &arg) < 0)
		system_error();
}

int main(int argc, char *argv[])
{
	const char *mountpoint = NULL;
	const char *snapname = NULL;
	int fd;

	if (argc == 0)
		exit(1);
	progname = argv[0];

	while (1) {
		int opt = getopt(argc, argv, "v");

		if (opt == -1)
			break;

		switch (opt) {
		case 'v':
			version();
		default:
			usage();
		}
	}

	if (optind != argc - 2)
		usage();
	mountpoint = argv[optind];
	snapname = argv[optind + 1];

	fd = open(mountpoint, O_RDONLY);
	if (fd == -1)
		system_error();

	create_snapshot(fd, snapname);
	return 0;
}
