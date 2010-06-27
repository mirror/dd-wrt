/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2010 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Non-specific support functions.
 * $Id: support.h,v 1.34 2010/02/07 18:36:53 castaglia Exp $
 */

#ifndef PR_SUPPORT_H
#define PR_SUPPORT_H

#include <time.h>

#if defined(NAME_MAX)
# define NAME_MAX_GUESS		(NAME_MAX)
#elif defined(MAXNAMELEN)
# define NAME_MAX_GUESS		(MAXNAMELEN - 1)
#else
# define NAME_MAX_GUESS		(255)
#endif

/* Functions [optionally] provided by libsupp.a */
#ifndef HAVE_GETOPT
int getopt(int, char * const [], const char *);
extern char *optarg;
extern int optind,opterr,optopt;

#ifndef HAVE_GETOPT_LONG
struct option {
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

int getopt_long(int, char * const [], const char *, const struct option *,
  int *);
# endif /* !HAVE_GETOPT_LONG */
#endif /* !HAVE_GETOPT */

void pr_signals_block(void);
void pr_signals_unblock(void);

char *dir_interpolate(pool *, const char *);
char *dir_abs_path(pool *, const char *, int);
char *dir_realpath(pool *, const char *);
char *dir_canonical_path(pool *, const char *);
char *dir_canonical_vpath(pool *, const char *);
char *dir_best_path(pool *, const char *);

void schedule(void (*f)(void *, void *, void *, void *), int, void *, void *,
  void *, void *);
void run_schedule(void);

int get_name_max(char *, int);

mode_t file_mode(char *);
int file_exists(char *);
int dir_exists(char *);
int exists(char *);

char *safe_token(char **);
int check_shutmsg(time_t *, time_t *, time_t *, char *, size_t);

void pr_memscrub(void *, size_t);

struct tm *pr_gmtime(pool *, const time_t *);
struct tm *pr_localtime(pool *, const time_t *);
const char *pr_strtime(time_t);
const char *pr_strtime2(time_t, int);

#endif /* PR_SUPPORT_H */
