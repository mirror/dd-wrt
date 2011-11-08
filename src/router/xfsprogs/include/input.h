/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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
#ifndef __INPUT_H__
#define __INPUT_H__

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <xfs/project.h>

extern char	**breakline(char *input, int *count);
extern void	doneline(char *input, char **vec);
extern char	*fetchline(void);

extern long long cvtnum(size_t blocksize, size_t sectorsize, char *s);
extern void	cvtstr(double value, char *str, size_t sz);
extern unsigned long cvttime(char *s);

extern struct timeval tadd(struct timeval t1, struct timeval t2);
extern struct timeval tsub(struct timeval t1, struct timeval t2);
extern double	tdiv(double value, struct timeval tv);

enum {
	DEFAULT_TIME		= 0x0,
	TERSE_FIXED_TIME	= 0x1,
	VERBOSE_FIXED_TIME	= 0x2
};

extern void	timestr(struct timeval *tv, char *str, size_t sz, int flags);

extern uid_t	uid_from_string(char *user);
extern gid_t	gid_from_string(char *group);
extern prid_t	prid_from_string(char *project);

#define HAVE_FTW_H 1	/* TODO: configure me */

#ifdef HAVE_FTW_H
#include <ftw.h>
#else
struct FTW;
struct stat;
extern int nftw(
	char	*dir,
	int	(*fn)(const char *, const struct stat *, int, struct FTW *),
	int	depth,
	int	flags);
#endif

#endif	/* __INPUT_H__ */
