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

struct logfile
{
  struct logfile *next;
  FILE *fp;		/* a hopefully uniq filepointer to the log file */
  char *name;		/* the name. used to reopen, when stat fails. */
  int opencount;	/* synchronize logfopen() and logfclose() */
  int writecount;	/* increments at logfwrite(), counts write() and fflush() */
  int flushcount;	/* increments at logfflush(), zeroed at logfwrite() */
  struct stat *st;	/* how the file looks like */
};

/*
 * open a logfile, The second argument must be NULL, when the named file
 * is already a logfile or must be a appropriatly opened file pointer
 * otherwise.
 * example: l = logfopen(name, islogfile(name) : NULL ? fopen(name, "a"));
 */
struct logfile *logfopen __P((char *name, FILE *fp));

/*
 * lookup a logfile by name. This is useful, so that we can provide
 * logfopen with a nonzero second argument, exactly when needed. 
 * islogfile(NULL); returns nonzero if there are any open logfiles at all.
 */
int islogfile __P((char *name));

/* 
 * logfclose does free()
 */
int logfclose __P((struct logfile *));
int logfwrite __P((struct logfile *, char *, int));

/* 
 * logfflush should be called periodically. If no argument is passed,
 * all logfiles are flushed, else the specified file
 * the number of flushed filepointers is returned
 */
int logfflush __P((struct logfile *ifany));

/* 
 * a reopen function may be registered here, in case you want to bring your 
 * own (more secure open), it may come along with a private data pointer.
 * this function is called, whenever logfwrite/logfflush detect that the
 * file has been (re)moved, truncated or changed by someone else.
 * if you provide NULL as parameter to logreopen_register, the builtin
 * reopen function will be reactivated.
 */
void logreopen_register __P((int (*fn) __P((char *, int, struct logfile *)) ));

/* 
 * Your custom reopen function is required to reuse the exact
 * filedescriptor. 
 * See logfile.c for further specs and an example.
 *
 * lf_move_fd may help you here, if you do not have dup2(2).
 * It closes fd and opens wantfd to access whatever fd accessed.
 */
int lf_move_fd __P((int fd, int wantfd));
