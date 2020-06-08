/*
 * support/export/xtab.c
 *
 * Interface to the etab/exports file.
 *
 * Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>

#include "nfslib.h"
#include "exportfs.h"
#include "xio.h"
#include "xlog.h"
#include "v4root.h"
#include "misc.h"

static char state_base_dirname[PATH_MAX] = NFS_STATEDIR;
extern struct state_paths etab;

int v4root_needed;
static void cond_rename(char *newfile, char *oldfile);

static int
xtab_read(char *xtab, char *lockfn, int is_export)
{
    /* is_export == 0  => reading /proc/fs/nfs/exports - we know these things are exported to kernel
     * is_export == 1  => reading /var/lib/nfs/etab - these things are allowed to be exported
     */
	struct exportent	*xp;
	nfs_export		*exp;
	int			lockid;

	if ((lockid = xflock(lockfn, "r")) < 0)
		return 0;
	setexportent(xtab, "r");
	if (is_export == 1)
		v4root_needed = 1;
	while ((xp = getexportent(is_export==0, 0)) != NULL) {
		if (!(exp = export_lookup(xp->e_hostname, xp->e_path, is_export != 1)) &&
		    !(exp = export_create(xp, is_export!=1))) {
                        if(xp->e_hostname) {
                            free(xp->e_hostname);
                            xp->e_hostname=NULL;
                        }
                        if(xp->e_uuid) {
                            free(xp->e_uuid);
                            xp->e_uuid=NULL;
                        }
			continue;
		}
		switch (is_export) {
		case 0:
			exp->m_exported = 1;
			break;
		case 1:
			exp->m_xtabent = 1;
			exp->m_mayexport = 1;
			if ((xp->e_flags & NFSEXP_FSID) && xp->e_fsid == 0)
				v4root_needed = 0;
			break;
		}  
                if(xp->e_hostname) {
                    free(xp->e_hostname);
                    xp->e_hostname=NULL;
                }
                if(xp->e_uuid) {
                    free(xp->e_uuid);
                    xp->e_uuid=NULL;
                }

	}
	endexportent();
	xfunlock(lockid);

	return 0;
}

int
xtab_export_read(void)
{
	return xtab_read(etab.statefn, etab.lockfn, 1);
}

/*
 * mountd now keeps an open fd for the etab at all times to make sure that the
 * inode number changes when the xtab_export_write is done. If you change the
 * routine below such that the files are edited in place, then you'll need to
 * fix the auth_reload logic as well...
 */
static int
xtab_write(char *xtab, char *xtabtmp, char *lockfn, int is_export)
{
	struct exportent	xe;
	nfs_export		*exp;
	int			lockid, i;

	if ((lockid = xflock(lockfn, "w")) < 0) {
		xlog(L_ERROR, "can't lock %s for writing", xtab);
		return 0;
	}
	setexportent(xtabtmp, "w");

	for (i = 0; i < MCL_MAXTYPES; i++) {
		for (exp = exportlist[i].p_head; exp; exp = exp->m_next) {
			if (is_export && !exp->m_xtabent)
				continue;
			if (!is_export && ! exp->m_exported)
				continue;

			/* write out the export entry using the FQDN */
			xe = exp->m_export;
			xe.e_hostname = exp->m_client->m_hostname;
			putexportent(&xe);
		}
	}
	endexportent();

	cond_rename(xtabtmp, xtab);

	xfunlock(lockid);

	return 1;
}

int
xtab_export_write()
{
	return xtab_write(etab.statefn, etab.tmpfn, etab.lockfn, 1);
}

/*
 * rename newfile onto oldfile unless
 * they are identical
 */
static void cond_rename(char *newfile, char *oldfile)
{
	int nfd, ofd;
	char nbuf[4096], obuf[4096];
	int ncnt, ocnt;

	nfd = open(newfile, 0);
	if (nfd < 0)
		return;
	ofd = open(oldfile, 0);
	if (ofd < 0) {
		close(nfd);
		rename(newfile, oldfile);
		return;
	}

	do {
		ncnt = read(nfd, nbuf, sizeof(nbuf));
		if (ncnt < 0)
			break;
		ocnt = read(ofd, obuf, sizeof(obuf));
		if (ocnt < 0)
			break;
		if (ncnt != ocnt)
			break;
		if (ncnt == 0) {
			close(nfd);
			close(ofd);
			unlink(newfile);
			return;
		}
	} while (memcmp(obuf, nbuf, ncnt) == 0);

	/* some mis-match */
	close(nfd);
	close(ofd);
	rename(newfile, oldfile);
	return;
}

/*
 * Returns a dynamically allocated, '\0'-terminated buffer
 * containing an appropriate pathname, or NULL if an error
 * occurs.  Caller must free the returned result with free(3).
 */
static char *
state_make_pathname(const char *tabname)
{
	return generic_make_pathname(state_base_dirname, tabname);
}

/**
 * state_setup_basedir - set up basedir
 * @progname: C string containing name of program, for error messages
 * @parentdir: C string containing pathname to on-disk state, or NULL
 *
 * This runs before logging is set up, so error messages are directed
 * to stderr.
 *
 * Returns true and sets up our basedir, if @parentdir was valid
 * and usable; otherwise false is returned.
 */
_Bool
state_setup_basedir(const char *progname, const char *parentdir)
{
	return generic_setup_basedir(progname, parentdir, state_base_dirname,
				     PATH_MAX);
}

int
setup_state_path_names(const char *progname, const char *statefn,
		      const char *tmpfn, const char *lockfn,
		      struct state_paths *paths)
{
	paths->statefn = state_make_pathname(statefn);
	if (!paths->statefn) {
		fprintf(stderr, "%s: state_make_pathname(%s) failed\n",
			progname, statefn);
		goto out_err;
	}
	paths->tmpfn = state_make_pathname(tmpfn);
	if (!paths->tmpfn) {
		fprintf(stderr, "%s: state_make_pathname(%s) failed\n",
			progname, tmpfn);
		goto out_free_statefn;
	}
	paths->lockfn = state_make_pathname(lockfn);
	if (!paths->lockfn) {
		fprintf(stderr, "%s: state_make_pathname(%s) failed\n",
			progname, lockfn);
		goto out_free_tmpfn;
	}
	return 1;

out_free_tmpfn:
	free(paths->tmpfn);
out_free_statefn:
	free(paths->statefn);
out_err:
	return 0;

}

void
free_state_path_names(struct state_paths *paths)
{
	free(paths->statefn);
	free(paths->tmpfn);
	free(paths->lockfn);
}
