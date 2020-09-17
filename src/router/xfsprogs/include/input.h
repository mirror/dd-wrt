// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __INPUT_H__
#define __INPUT_H__

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include "libfrog/projects.h"
#include "libfrog/convert.h"
#include <stdbool.h>

extern char	**breakline(char *input, int *count);
extern void	doneline(char *input, char **vec);
extern char	*fetchline(void);

extern size_t numlen(uint64_t val, size_t base);

extern struct timeval tsub(struct timeval t1, struct timeval t2);
extern double	tdiv(double value, struct timeval tv);

enum {
	DEFAULT_TIME		= 0x0,
	TERSE_FIXED_TIME	= 0x1,
	VERBOSE_FIXED_TIME	= 0x2
};

extern void	timestr(struct timeval *tv, char *str, size_t sz, int flags);

extern bool	isdigits_only(const char *str);
extern int	timespec_from_string(const char *sec, const char *nsec, struct timespec *ts);

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
