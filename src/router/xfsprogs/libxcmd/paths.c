/*
 * Copyright (c) 2005-2006 Silicon Graphics, Inc.
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

#include <paths.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <xfs/path.h>
#include <xfs/input.h>
#include <xfs/project.h>

extern char *progname;

int fs_count;
struct fs_path *fs_table;
struct fs_path *fs_path;

char *mtab_file;
#define PROC_MOUNTS	"/proc/self/mounts"

static int
fs_device_number(
	const char	*name,
	dev_t		*devnum)
{
	struct stat64	sbuf;

	if (stat64(name, &sbuf) < 0)
		return errno;
	/*
	 * We want to match st_rdev if the path provided is a device
	 * special file.  Otherwise we are looking for the the
	 * device id for the containing filesystem, in st_dev.
	 */
	if (S_ISBLK(sbuf.st_mode) || S_ISCHR(sbuf.st_mode))
		*devnum = sbuf.st_rdev;
	else
		*devnum = sbuf.st_dev;

	return 0;
}

/*
 * Find the FS table entry for the given path.  The "flags" argument
 * is a mask containing FS_MOUNT_POINT or FS_PROJECT_PATH (or both)
 * to indicate the type of table entry sought.
 */
struct fs_path *
fs_table_lookup(
	const char	*dir,
	uint		flags)
{
	uint		i;
	dev_t		dev = 0;

	if (fs_device_number(dir, &dev))
		return NULL;

	for (i = 0; i < fs_count; i++) {
		if (flags && !(flags & fs_table[i].fs_flags))
			continue;
		if (fs_table[i].fs_datadev == dev)
			return &fs_table[i];
	}
	return NULL;
}

static int
fs_table_insert(
	char		*dir,
	uint		prid,
	uint		flags,
	char		*fsname,
	char		*fslog,
	char		*fsrt)
{
	dev_t		datadev, logdev, rtdev;
	struct fs_path	*tmp_fs_table;
	int		error;

	datadev = logdev = rtdev = 0;
	error = fs_device_number(dir, &datadev);
	if (error)
		goto out_nodev;
	if (fslog) {
		error = fs_device_number(fslog, &logdev);
		if (error)
			goto out_nodev;
	}
	if (fsrt) {
		error = fs_device_number(fsrt, &rtdev);
		if (error)
			goto out_nodev;
	}

	/*
	 * Make copies of the directory and data device path.
	 * The log device and real-time device, if non-null,
	 * are already the result of strdup() calls so we
	 * don't need to duplicate those.  In fact, this
	 * function is assumed to "consume" both of those
	 * pointers, meaning if an error occurs they will
	 * both get freed.
	 */
	error = ENOMEM;
	dir = strdup(dir);
	if (!dir)
		goto out_nodev;
	fsname = strdup(fsname);
	if (!fsname)
		goto out_noname;

	tmp_fs_table = realloc(fs_table, sizeof(fs_path_t) * (fs_count + 1));
	if (!tmp_fs_table)
		goto out_norealloc;
	fs_table = tmp_fs_table;

	fs_path = &fs_table[fs_count];
	fs_path->fs_dir = dir;
	fs_path->fs_prid = prid;
	fs_path->fs_flags = flags;
	fs_path->fs_name = fsname;
	fs_path->fs_log = fslog;
	fs_path->fs_rt = fsrt;
	fs_path->fs_datadev = datadev;
	fs_path->fs_logdev = logdev;
	fs_path->fs_rtdev = rtdev;
	fs_count++;

	return 0;

out_norealloc:
	free(fsname);
out_noname:
	free(dir);
out_nodev:
	/* "Consume" fslog and fsrt even if there's an error */
	free(fslog);
	free(fsrt);

	return error;
}

/*
 * Table iteration (cursor-based) interfaces
 */

/*
 * Initialize an fs_table cursor.  If a directory path is supplied,
 * the cursor is set up to appear as though the table contains only
 * a single entry which represents the directory specified.
 * Otherwise it is set up to prepare for visiting all entries in the
 * global table, starting with the first.  "flags" can be either
 * FS_MOUNT_POINT or FS_PROJECT_PATH to limit what type of entries
 * will be selected by fs_cursor_next_entry().  0 can be used as a
 * wild card (selecting either type).
 */
void
fs_cursor_initialise(
	char		*dir,
	uint		flags,
	fs_cursor_t	*cur)
{
	fs_path_t	*path;

	memset(cur, 0, sizeof(*cur));
	if (dir) {
		if ((path = fs_table_lookup(dir, flags)) == NULL)
			return;
		cur->local = *path;
		cur->count = 1;
		cur->table = &cur->local;
	} else {
		cur->count = fs_count;
		cur->table = fs_table;
	}
	cur->flags = flags;
}

/*
 * Use the cursor to find the next entry in the table having the
 * type specified by the cursor's "flags" field.
 */
struct fs_path *
fs_cursor_next_entry(
	fs_cursor_t	*cur)
{
	while (cur->index < cur->count) {
		fs_path_t	*next = &cur->table[cur->index++];

		if (!cur->flags || (cur->flags & next->fs_flags))
			return next;
	}
	return NULL;
}


#if defined(HAVE_GETMNTENT)
#include <mntent.h>

/*
 * Determines whether the "logdev" or "rtdev" mount options are
 * present for the given mount point.  If so, the value for each (a
 * device path) is returned in the pointers whose addresses are
 * provided.  The pointers are assigned NULL for an option not
 * present.  Note that the path buffers returned are allocated
 * dynamically and it is the caller's responsibility to free them.
 */
static int
fs_extract_mount_options(
	struct mntent	*mnt,
	char		**logp,
	char		**rtp)
{
	char		*fslog, *fsrt;

	/* Extract log device and realtime device from mount options */
	if ((fslog = hasmntopt(mnt, "logdev=")))
		fslog += 7;
	if ((fsrt = hasmntopt(mnt, "rtdev=")))
		fsrt += 6;

	/* Do this only after we've finished processing mount options */
	if (fslog) {
		fslog = strndup(fslog, strcspn(fslog, " ,"));
		if (!fslog)
			goto out_nomem;
	}
	if (fsrt) {
		fsrt = strndup(fsrt, strcspn(fsrt, " ,"));
		if (!fsrt) {
			free(fslog);
			goto out_nomem;
		}
	}
	*logp = fslog;
	*rtp = fsrt;

	return 0;

out_nomem:
	*logp = NULL;
	*rtp = NULL;
	fprintf(stderr, _("%s: unable to extract mount options for \"%s\"\n"),
		progname, mnt->mnt_dir);
	return ENOMEM;
}

static int
fs_table_initialise_mounts(
	char		*path)
{
	struct mntent	*mnt;
	FILE		*mtp;
	char		*fslog, *fsrt;
	int		error, found;

	error = found = 0;
	fslog = fsrt = NULL;

	if (!mtab_file) {
		mtab_file = PROC_MOUNTS;
		if (access(mtab_file, R_OK) != 0)
			mtab_file = MOUNTED;
	}

	if ((mtp = setmntent(mtab_file, "r")) == NULL)
		return ENOENT;

	while ((mnt = getmntent(mtp)) != NULL) {
		if (strcmp(mnt->mnt_type, "xfs") != 0)
			continue;
		if (path &&
		    ((strcmp(path, mnt->mnt_dir) != 0) &&
		     (strcmp(path, mnt->mnt_fsname) != 0)))
			continue;
		if (fs_extract_mount_options(mnt, &fslog, &fsrt))
			continue;
		(void) fs_table_insert(mnt->mnt_dir, 0, FS_MOUNT_POINT,
					mnt->mnt_fsname, fslog, fsrt);
		if (path) {
			found = 1;
			break;
		}
	}
	endmntent(mtp);
	if (path && !found)
		error = ENXIO;

	return error;
}

#elif defined(HAVE_GETMNTINFO)
#include <sys/mount.h>

static int
fs_table_initialise_mounts(
	char		*path)
{
	struct statfs	*stats;
	int		i, count, error, found;

	error = found = 0;
	if ((count = getmntinfo(&stats, 0)) < 0) {
		fprintf(stderr, _("%s: getmntinfo() failed: %s\n"),
				progname, strerror(errno));
		return 0;
	}

	for (i = 0; i < count; i++) {
		if (strcmp(stats[i].f_fstypename, "xfs") != 0)
			continue;
		if (path &&
		    ((strcmp(path, stats[i].f_mntonname) != 0) &&
		     (strcmp(path, stats[i].f_mntfromname) != 0)))
			continue;
		/* TODO: external log and realtime device? */
		(void) fs_table_insert(stats[i].f_mntonname, 0,
					FS_MOUNT_POINT, stats[i].f_mntfromname,
					NULL, NULL);
		if (path) {
			found = 1;
			break;
		}
	}
	if (path && !found)
		error = ENXIO;

	return error;
}

#else
# error "How do I extract info about mounted filesystems on this platform?"
#endif

/*
 * Given a directory, match it up to a filesystem mount point.
 */
static struct fs_path *
fs_mount_point_from_path(
	const char	*dir)
{
	fs_cursor_t	cursor;
	fs_path_t	*fs;
	dev_t		dev = 0;

	if (fs_device_number(dir, &dev))
		return NULL;

	fs_cursor_initialise(NULL, FS_MOUNT_POINT, &cursor);
	while ((fs = fs_cursor_next_entry(&cursor))) {
		if (fs->fs_datadev == dev)
			break;
	}
	return fs;
}

static void
fs_table_insert_mount(
	char		*mount)
{
	int		error;

	error = fs_table_initialise_mounts(mount);
	if (error)
		fprintf(stderr, _("%s: cannot setup path for mount %s: %s\n"),
			progname, mount, strerror(error));
}

static int
fs_table_initialise_projects(
	char		*project)
{
	fs_project_path_t *path;
	fs_path_t	*fs;
	prid_t		prid = 0;
	int		error = 0, found = 0;

	if (project)
		prid = prid_from_string(project);

	setprpathent();
	while ((path = getprpathent()) != NULL) {
		if (project && prid != path->pp_prid)
			continue;
		fs = fs_mount_point_from_path(path->pp_pathname);
		if (!fs) {
			fprintf(stderr, _("%s: cannot find mount point for path `%s': %s\n"),
					progname, path->pp_pathname, strerror(errno));
			continue;
		}
		(void) fs_table_insert(path->pp_pathname, path->pp_prid,
					FS_PROJECT_PATH, fs->fs_name,
					NULL, NULL);
		if (project) {
			found = 1;
			break;
		}
	}
	endprpathent();

	if (project && !found)
		error = ENOENT;

	return error;
}

static void
fs_table_insert_project(
	char		*project)
{
	int		error;

	error = fs_table_initialise_projects(project);
	if (error)
		fprintf(stderr, _("%s: cannot setup path for project %s: %s\n"),
			progname, project, strerror(error));
}

/*
 * Initialize fs_table to contain the given set of mount points and
 * projects.  If mount_count is zero, mounts is ignored and the
 * table is populated with mounted filesystems.  If project_count is
 * zero, projects is ignored and the table is populated with all
 * projects defined in the projects file.
 */
void
fs_table_initialise(
	int	mount_count,
	char	*mounts[],
	int	project_count,
	char	*projects[])
{
	int	error;
	int	i;

	if (mount_count) {
		for (i = 0; i < mount_count; i++)
			fs_table_insert_mount(mounts[i]);
	} else {
		error = fs_table_initialise_mounts(NULL);
		if (error)
			goto out_error;
	}
	if (project_count) {
		for (i = 0; i < project_count; i++)
			fs_table_insert_project(projects[i]);
	} else {
		error = fs_table_initialise_projects(NULL);
		if (error)
			goto out_error;
	}

	return;

out_error:
	fprintf(stderr, _("%s: cannot initialise path table: %s\n"),
		progname, strerror(error));
}

void 
fs_table_insert_project_path(
	char		*dir,
	prid_t		prid)
{
	fs_path_t	*fs;
	int		error = 0;

	fs = fs_mount_point_from_path(dir);
	if (fs)
		error = fs_table_insert(dir, prid, FS_PROJECT_PATH,
					fs->fs_name, NULL, NULL);
	else
		error = ENOENT;

	if (error) {
		fprintf(stderr, _("%s: cannot setup path for project dir %s: %s\n"),
				progname, dir, strerror(error));
		exit(1);
	}
}

