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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include <xfs/path.h>
#include <xfs/parent.h>
#include <xfs/handle.h>
#include <xfs/jdm.h>
#include "init.h"
#include "io.h"

#define PARENTBUF_SZ		16384
#define BSTATBUF_SZ		16384

static cmdinfo_t parent_cmd;
static int verbose_flag;
static int err_status;
static __u64 inodes_checked;
static char *mntpt;

/*
 * check out a parent entry to see if the values seem valid
 */
static void
check_parent_entry(xfs_bstat_t *bstatp, parent_t *parent)
{
	int sts;
	char fullpath[PATH_MAX];
	struct stat statbuf;
	char *str;

	sprintf(fullpath, _("%s%s"), mntpt, parent->p_name);

	sts = lstat(fullpath, &statbuf);
	if (sts != 0) {
		fprintf(stderr,
			_("inode-path for inode: %llu is incorrect - path \"%s\" non-existent\n"),
			(unsigned long long) bstatp->bs_ino, fullpath);
		if (verbose_flag) {
			fprintf(stderr,
				_("path \"%s\" does not stat for inode: %llu; err = %s\n"),
				fullpath,
			       (unsigned long long) bstatp->bs_ino,
				strerror(errno));
		}
		err_status++;
		return;
	} else {
		if (verbose_flag > 1) {
			printf(_("path \"%s\" found\n"), fullpath);
		}
	}

	if (statbuf.st_ino != bstatp->bs_ino) {
		fprintf(stderr,
			_("inode-path for inode: %llu is incorrect - wrong inode#\n"),
		       (unsigned long long) bstatp->bs_ino);
		if (verbose_flag) {
			fprintf(stderr,
				_("ino mismatch for path \"%s\" %llu vs %llu\n"),
				fullpath,
				(unsigned long long)statbuf.st_ino,
				(unsigned long long)bstatp->bs_ino);
		}
		err_status++;
		return;
	} else if (verbose_flag > 1) {
		printf(_("inode number match: %llu\n"),
			(unsigned long long)statbuf.st_ino);
	}

	/* get parent path */
	str = strrchr(fullpath, '/');
	*str = '\0';
	sts = stat(fullpath, &statbuf);
	if (sts != 0) {
		fprintf(stderr,
			_("parent path \"%s\" does not stat: %s\n"),
			fullpath,
			strerror(errno));
		err_status++;
		return;
	} else {
		if (parent->p_ino != statbuf.st_ino) {
			fprintf(stderr,
				_("inode-path for inode: %llu is incorrect - wrong parent inode#\n"),
			       (unsigned long long) bstatp->bs_ino);
			if (verbose_flag) {
				fprintf(stderr,
					_("ino mismatch for path \"%s\" %llu vs %llu\n"),
					fullpath,
					(unsigned long long)parent->p_ino,
					(unsigned long long)statbuf.st_ino);
			}
			err_status++;
			return;
		} else {
			if (verbose_flag > 1) {
			       printf(_("parent ino match for %llu\n"),
				       (unsigned long long) parent->p_ino);
			}
		}
	}
}

static void
check_parents(parent_t *parentbuf, size_t *parentbuf_size,
	     jdm_fshandle_t *fshandlep, xfs_bstat_t *statp)
{
	int error, i;
	__u32 count;
	parent_t *entryp;

	do {
		error = jdm_parentpaths(fshandlep, statp, parentbuf, *parentbuf_size, &count);

		if (error == ERANGE) {
			*parentbuf_size *= 2;
			parentbuf = (parent_t *)realloc(parentbuf, *parentbuf_size);
		} else if (error) {
			fprintf(stderr, _("parentpaths failed for ino %llu: %s\n"),
			       (unsigned long long) statp->bs_ino,
				strerror(errno));
			err_status++;
			break;
		}
	} while (error == ERANGE);
	

	if (count == 0) {
		/* no links for inode - something wrong here */
	       fprintf(stderr, _("inode-path for inode: %llu is missing\n"),
			       (unsigned long long) statp->bs_ino);
		err_status++;
	}

	entryp = parentbuf;
	for (i = 0; i < count; i++) {
		check_parent_entry(statp, entryp);
		entryp = (parent_t*) (((char*)entryp) + entryp->p_reclen);
	}
}

static int
do_bulkstat(parent_t *parentbuf, size_t *parentbuf_size, xfs_bstat_t *bstatbuf,
	    int fsfd, jdm_fshandle_t *fshandlep)
{
	__s32 buflenout;
	__u64 lastino = 0;
	xfs_bstat_t *p;
	xfs_bstat_t *endp;
	xfs_fsop_bulkreq_t bulkreq;
	struct stat mntstat;

	if (stat(mntpt, &mntstat)) {
		fprintf(stderr, _("can't stat mount point \"%s\": %s\n"),
			mntpt, strerror(errno));
		return 1;
	}

	bulkreq.lastip  = &lastino;
	bulkreq.icount  = BSTATBUF_SZ;
	bulkreq.ubuffer = (void *)bstatbuf;
	bulkreq.ocount  = &buflenout;

	while (xfsctl(mntpt, fsfd, XFS_IOC_FSBULKSTAT, &bulkreq) == 0) {
		if (*(bulkreq.ocount) == 0) {
			return 0;
		}
		for (p = bstatbuf, endp = bstatbuf + *bulkreq.ocount; p < endp; p++) {

			/* inode being modified, get synced data with iget */
			if ( (!p->bs_nlink || !p->bs_mode) && p->bs_ino != 0 ) {

				if (xfsctl(mntpt, fsfd, XFS_IOC_FSBULKSTAT_SINGLE, &bulkreq) < 0) {
				    fprintf(stderr,
					  _("failed to get bulkstat information for inode %llu\n"),
					 (unsigned long long) p->bs_ino);
				    continue;
				}
				if (!p->bs_nlink || !p->bs_mode || !p->bs_ino) {
				    fprintf(stderr,
					  _("failed to get valid bulkstat information for inode %llu\n"),
					 (unsigned long long) p->bs_ino);
				    continue;
				}
			}

			/* skip root */
			if (p->bs_ino == mntstat.st_ino) {
				continue;
			}

			if (verbose_flag > 1) {
			       printf(_("checking inode %llu\n"),
				       (unsigned long long) p->bs_ino);
			}

			/* print dotted progress */
			if ((inodes_checked % 100) == 0 && verbose_flag == 1) {
				printf("."); fflush(stdout);
			}
			inodes_checked++;

			check_parents(parentbuf, parentbuf_size, fshandlep, p);
		}

	}/*while*/

	fprintf(stderr, _("syssgi bulkstat failed: %s\n"), strerror(errno));
	return 1;
}

static int
parent_check(void)
{
	int fsfd;
	jdm_fshandle_t *fshandlep;
	parent_t *parentbuf;
	size_t parentbuf_size = PARENTBUF_SZ;
	xfs_bstat_t *bstatbuf;

	err_status = 0;
	inodes_checked = 0;

	sync();

        fsfd = file->fd;

	fshandlep = jdm_getfshandle(mntpt);
	if (fshandlep == NULL) {
		fprintf(stderr, _("unable to open \"%s\" for jdm: %s\n"),
		      mntpt,
		      strerror(errno));
		return 1;
	}

	/* allocate buffers */
        bstatbuf = (xfs_bstat_t *)calloc(BSTATBUF_SZ, sizeof(xfs_bstat_t));
	parentbuf = (parent_t *)malloc(parentbuf_size);
	if (!bstatbuf || !parentbuf) {
		fprintf(stderr, _("unable to allocate buffers: %s\n"),
			strerror(errno));
		return 1;
	}

	if (do_bulkstat(parentbuf, &parentbuf_size, bstatbuf, fsfd, fshandlep) != 0)
		err_status++;

	if (err_status > 0)
		fprintf(stderr, _("num errors: %d\n"), err_status);
	else
		printf(_("succeeded checking %llu inodes\n"),
			(unsigned long long) inodes_checked);

	free(bstatbuf);
	free(parentbuf);
	return err_status;
}

static void
print_parent_entry(parent_t *parent, int fullpath)
{
       printf(_("p_ino    = %llu\n"),  (unsigned long long) parent->p_ino);
	printf(_("p_gen    = %u\n"),	parent->p_gen);
	printf(_("p_reclen = %u\n"),	parent->p_reclen);
	if (fullpath)
		printf(_("p_name   = \"%s%s\"\n"), mntpt, parent->p_name);
	else
		printf(_("p_name   = \"%s\"\n"), parent->p_name);
}

static int
parent_list(int fullpath)
{
	void *handlep;
	size_t handlen;
	int error, i;
	int retval = 1;
	__u32 count;
	parent_t *entryp;
	parent_t *parentbuf = NULL;
	char *path = file->name;
	int pb_size = PARENTBUF_SZ;

	/* XXXX for linux libhandle version - to set libhandle fsfd cache */
	{
		void *fshandle;
		size_t fshlen;

		if (path_to_fshandle(mntpt, &fshandle, &fshlen) != 0) {
			fprintf(stderr, _("%s: failed path_to_fshandle \"%s\": %s\n"),
				progname, path, strerror(errno));
			goto error;
		}
	}

	if (path_to_handle(path, &handlep, &handlen) != 0) {
		fprintf(stderr, _("%s: path_to_handle failed for \"%s\"\n"), progname, path);
		goto error;
	}

	do {
		parentbuf = (parent_t *)realloc(parentbuf, pb_size);
		if (!parentbuf) {
			fprintf(stderr, _("%s: unable to allocate parent buffer: %s\n"),
				progname, strerror(errno));
			return 1;
		}

		if (fullpath) {
			error = parentpaths_by_handle(handlep,
						       handlen,
						       parentbuf,
						       pb_size,
						       &count);
		} else {
			error = parents_by_handle(handlep,
						   handlen,
						   parentbuf,
						   pb_size,
						   &count);
		}
		if (error == ERANGE) {
			pb_size *= 2;
		} else if (error) {
			fprintf(stderr, _("%s: %s call failed for \"%s\": %s\n"),
				progname, fullpath ? "parentpaths" : "parents",
				path, strerror(errno));
			goto error;
		}
	} while (error == ERANGE);

	if (count == 0) {
		/* no links for inode - something wrong here */
		fprintf(stderr, _("%s: inode-path is missing\n"), progname);
		goto error;
	}

	entryp = parentbuf;
	for (i = 0; i < count; i++) {
		print_parent_entry(entryp, fullpath);
		entryp = (parent_t*) (((char*)entryp) + entryp->p_reclen);
	}

	retval = 0;
error:
	free(parentbuf);
	return retval;
}

int
parent_f(int argc, char **argv)
{
	int c;
	int listpath_flag = 0;
	int check_flag = 0;
	fs_path_t *fs;
	static int tab_init;

	if (!tab_init) {
		tab_init = 1;
		fs_table_initialise(0, NULL, 0, NULL);
	}
	fs = fs_table_lookup(file->name, FS_MOUNT_POINT);
	if (!fs) {
		fprintf(stderr, _("file argument, \"%s\", is not in a mounted XFS filesystem\n"),
			file->name);
		return 1;
	}
	mntpt = fs->fs_dir;

	verbose_flag = 0;

	while ((c = getopt(argc, argv, "cpv")) != EOF) {
		switch (c) {
		case 'c':
			check_flag = 1;
			break;
		case 'p':
			listpath_flag = 1;
			break;
		case 'v':
			verbose_flag++;
			break;
		default:
			return command_usage(&parent_cmd);
		}
	}

	if (!check_flag && !listpath_flag) /* default case */
		exitcode = parent_list(listpath_flag);
	else {
		if (listpath_flag)
			exitcode = parent_list(listpath_flag);
		if (check_flag)
			exitcode = parent_check();
	}

	return 0;
}

static void
parent_help(void)
{
	printf(_(
"\n"
" list the current file's parents and their filenames\n"
"\n"
" -c -- check the current file's file system for parent consistency\n"
" -p -- list the current file's parents and their full paths\n"
" -v -- verbose mode\n"
"\n"));
}

void
parent_init(void)
{
	parent_cmd.name = "parent";
	parent_cmd.cfunc = parent_f;
	parent_cmd.argmin = 0;
	parent_cmd.argmax = -1;
	parent_cmd.args = _("[-cpv]");
	parent_cmd.flags = CMD_NOMAP_OK;
	parent_cmd.oneline = _("print or check parent inodes");
	parent_cmd.help = parent_help;

	if (expert)
		add_command(&parent_cmd);
}
