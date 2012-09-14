/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

instances.c - handler routines for multiple IPTraf instances

***/

#include "iptraf-ng-compat.h"

#include "error.h"
#include "dirs.h"
#include "instances.h"

static void gen_lockfile_name(char *tagfile, char *iface, char *result)
{
	if (iface == NULL)
		snprintf(result, 64, "%s.all", tagfile);
	else
		snprintf(result, 64, "%s.%s", tagfile, iface);
}

void mark_facility(char *tagfile, char *facility, char *iface)
{
	int fd;
	char lockfile[64];

	gen_lockfile_name(tagfile, iface, lockfile);
	fd = open(lockfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0)
		write_error("Warning: unable to lock %s on %s", facility, iface);
	close(fd);
}

void unmark_facility(char *tagfile, char *iface)
{
	char lockfile[64];

	gen_lockfile_name(tagfile, iface, lockfile);
	unlink(lockfile);
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
