/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"

#include "logfile.h"

#include <sys/types.h>		/* dev_t, ino_t, off_t, ... */
#include <sys/stat.h>		/* struct stat */
#include <fcntl.h>		/* O_WRONLY for logfile_reopen */
#include <stdint.h>
#include <stdbool.h>

#include "screen.h"

#include "misc.h"

static void changed_logfile(Log *);
static Log *lookup_logfile(char *);
static int stolen_logfile(Log *);

static Log *logroot = NULL;

static void changed_logfile(Log *l)
{
	struct stat o, *s = l->st;

	if (fstat(fileno(l->fp), &o) < 0)	/* get trouble later */
		return;
	if (o.st_size > s->st_size) {	/* aha, appended text */
		s->st_size = o.st_size;	/* this should have changed */
		s->st_mtime = o.st_mtime;	/* only size and mtime */
	}
}

/*
 * Requires fd to be open and need_fd to be closed.
 * If possible, need_fd will be open afterwards and refer to
 * the object originally reffered by fd. fd will be closed then.
 * Works just like ``fcntl(fd, DUPFD, need_fd); close(fd);''
 *
 * need_fd is returned on success, else -1 is returned.
 */
int lf_move_fd(int fd, int need_fd)
{
	int r = -1;

	if (fd == need_fd)
		return fd;
	if (fd >= 0 && fd < need_fd)
		r = lf_move_fd(dup(fd), need_fd);
	close(fd);
	return r;
}

static int logfile_reopen(char *name, int wantfd, Log *l)
{
	int got_fd;

	close(wantfd);
	if (((got_fd = open(name, O_WRONLY | O_CREAT | O_APPEND, 0666)) < 0) || lf_move_fd(got_fd, wantfd) < 0) {
		logfclose(l);
		return -1;
	}
	changed_logfile(l);
	l->st->st_ino = l->st->st_dev = 0;
	return 0;
}

/*
 * If the logfile has been removed, truncated, unlinked or the like,
 * return nonzero.
 * The l->st structure initialised by logfopen is updated
 * on every call.
 */
static int stolen_logfile(Log *l)
{
	struct stat o, *s = l->st;

	o = *s;
	if (fstat(fileno(l->fp), s) < 0)	/* remember that stat failed */
		s->st_ino = s->st_dev = 0;
	if (!o.st_dev && !o.st_ino)	/* nothing to compare with */
		return 0;

	if ((!s->st_dev && !s->st_ino) ||	/* stat failed, that's new! */
	    !s->st_nlink ||	/* red alert: file unlinked */
	    (s->st_size < o.st_size) ||	/*           file truncated */
	    (s->st_mtime != o.st_mtime) ||	/*            file modified */
	    ((s->st_ctime != o.st_ctime) &&	/*     file changed (moved) */
	     !(s->st_mtime == s->st_ctime &&	/*  and it was not a change */
	       o.st_ctime < s->st_ctime))) {	/* due to delayed nfs write */
		return -1;
	}

	return 0;
}

static Log *lookup_logfile(char *name)
{
	Log *l;

	for (l = logroot; l; l = l->next)
		if (!strcmp(name, l->name))
			return l;
	return NULL;
}

Log *logfopen(char *name, FILE * fp)
{
	Log *l;

	if (!fp) {
		if (!(l = lookup_logfile(name)))
			return NULL;
		l->opencount++;
		return l;
	}

	if (!(l = calloc(1, sizeof(Log))))
		return NULL;
	if (!(l->st = calloc(1, sizeof(struct stat)))) {
		free((char *)l);
		return NULL;
	}

	if (!(l->name = SaveStr(name))) {
		free((char *)l->st);
		free((char *)l);
		return NULL;
	}
	l->fp = fp;
	l->opencount = 1;
	l->writecount = 0;
	l->flushcount = 0;
	changed_logfile(l);

	l->next = logroot;
	logroot = l;
	return l;
}

int islogfile(char *name)
{
	if (!name)
		return logroot ? 1 : 0;
	return lookup_logfile(name) ? 1 : 0;
}

int logfclose(Log *l)
{
	Log **lp;

	for (lp = &logroot; *lp; lp = &(*lp)->next)
		if (*lp == l)
			break;

	if (!*lp)
		return -1;

	if ((--l->opencount) > 0)
		return 0;
	if (l->opencount < 0)
		abort();

	*lp = l->next;
	fclose(l->fp);
	free(l->name);
	free((char *)l);
	return 0;
}

/*
 * XXX
 * write and flush both *should* check the file's stat, if it disappeared
 * or changed, re-open it.
 */
int logfwrite(Log *l, char *buf, size_t n)
{
	int r;

	if (stolen_logfile(l) && logfile_reopen(l->name, fileno(l->fp), l))
		return -1;
	r = fwrite(buf, n, 1, l->fp);
	l->writecount += l->flushcount + 1;
	l->flushcount = 0;
	changed_logfile(l);
	return r;
}

int logfflush(Log *l)
{
	int r = 0;

	if (!l)
		for (l = logroot; l; l = l->next) {
			if (stolen_logfile(l) && logfile_reopen(l->name, fileno(l->fp), l))
				return -1;
			r |= fflush(l->fp);
			l->flushcount++;
			changed_logfile(l);
	} else {
		if (stolen_logfile(l) && logfile_reopen(l->name, fileno(l->fp), l))
			return -1;
		r = fflush(l->fp);
		l->flushcount++;
		changed_logfile(l);
	}
	return r;
}
