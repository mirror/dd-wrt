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
 * $Id$ GNU
 */

#ifndef SCREEN_LOGFILE_H
#define SCREEN_LOGFILE_H

#include <stdio.h>

typedef struct Log Log;
struct Log {
	Log *next;
	FILE *fp;	/* a hopefully uniq filepointer to the log file */
	char *name;	/* the name. used to reopen, when stat fails. */
	int opencount;	/* synchronize logfopen() and logfclose() */
	int writecount;	/* increments at logfwrite(), counts write() and fflush() */
	int flushcount;	/* increments at logfflush(), zeroed at logfwrite() */
	struct stat *st;/* how the file looks like */
};

/*
 * open a logfile, The second argument must be NULL, when the named file
 * is already a logfile or must be a appropriatly opened file pointer
 * otherwise.
 * example: l = logfopen(name, islogfile(name) : NULL ? fopen(name, "a"));
 */
Log *logfopen (char *name, FILE *fp);

/*
 * lookup a logfile by name. This is useful, so that we can provide
 * logfopen with a nonzero second argument, exactly when needed.
 * islogfile(NULL); returns nonzero if there are any open logfiles at all.
 */
int islogfile (char *name);

/*
 * logfclose does free()
 */
int logfclose (Log *);
int logfwrite (Log *, char *, size_t);

/*
 * logfflush should be called periodically. If no argument is passed,
 * all logfiles are flushed, else the specified file
 * the number of flushed filepointers is returned
 */
int logfflush (Log *ifany);

/*
 * Your custom reopen function is required to reuse the exact
 * filedescriptor.
 * See logfile.c for further specs and an example.
 *
 * lf_move_fd may help you here, if you do not have dup2(2).
 * It closes fd and opens wantfd to access whatever fd accessed.
 */
int lf_move_fd (int fd, int wantfd);

#endif /*  SCREEN_LOGFILE_H */
