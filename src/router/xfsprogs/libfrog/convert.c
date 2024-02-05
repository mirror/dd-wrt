// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include "platform_defs.h"
#include "input.h"
#include <ctype.h>
#include <stdbool.h>

size_t
numlen(
	uint64_t	val,
	size_t		base)
{
	uint64_t	tmp;
	size_t		len;

	for (len = 0, tmp = val; tmp > 0; tmp = tmp / base)
		len++;
	return len == 0 ? 1 : len;
}

/*
 * Convert string to int64_t, set errno if the conversion fails or
 * doesn't fit.  Does not allow unit specifiers.  Sets errno to zero
 * prior to conversion so you can check for bad inputs by examining
 * errno immediately after the call.
 */
int64_t
cvt_s64(
	char		*s,
	int		base)
{
	long long	i;
	char		*sp;

	errno = 0;
	i = strtoll(s, &sp, base);
	/*
	 * If the input would over or underflow, return the clamped
	 * value and let the user check errno.  If we went all the
	 * way to the end of the input, return the converted value;
	 * errno will be zero.
	 */
	if (errno || (*sp == '\0' && sp != s))
		return i;

	/* Not all the input was consumed, return error. */
	errno = ERANGE;
	return INT64_MIN;
}

/*
 * Convert string to int32_t, set errno if the conversion fails or
 * doesn't fit.  Does not allow unit specifiers.  Sets errno to zero
 * prior to conversion so you can check for bad inputs by examining
 * errno immediately after the call.
 */
int32_t
cvt_s32(
	char		*s,
	int		base)
{
	int64_t		i;

	i = cvt_s64(s, base);
	if (errno)
		return i;
	if (i > INT32_MAX || i < INT32_MIN) {
		errno = ERANGE;
		return INT32_MIN;
	}
	return i;
}

/*
 * Convert string to int16_t, set errno if the conversion fails or
 * doesn't fit.  Does not allow unit specifiers.  Sets errno to zero
 * prior to conversion so you can check for bad inputs by examining
 * errno immediately after the call.
 */
int16_t
cvt_s16(
	char		*s,
	int		base)
{
	int64_t		i;

	i = cvt_s64(s, base);
	if (errno)
		return i;
	if (i > INT16_MAX || i < INT16_MIN) {
		errno = ERANGE;
		return INT16_MIN;
	}
	return i;
}

/*
 * Convert string to uint64_t, set errno if the conversion fails or
 * doesn't fit.  Does not allow unit specifiers.  Sets errno to zero
 * prior to conversion so you can check for bad inputs by examining
 * errno immediately after the call.
 */
uint64_t
cvt_u64(
	char			*s,
	int			base)
{
	unsigned long long	i;
	char			*sp;

	errno = 0;
	i = strtoull(s, &sp, base);
	/*
	 * If the input would over or underflow, return the clamped
	 * value and let the user check errno.  If we went all the
	 * way to the end of the input, return the converted value;
	 * errno will be zero.
	 */
	if (errno || (*sp == '\0' && sp != s))
		return i;

	/* Not all the input was consumed, return error. */
	errno = ERANGE;
	return UINT64_MAX;
}

/*
 * Convert string to uint32_t, set errno if the conversion fails or
 * doesn't fit.  Does not allow unit specifiers.  Sets errno to zero
 * prior to conversion so you can check for bad inputs by examining
 * errno immediately after the call.
 */
uint32_t
cvt_u32(
	char		*s,
	int		base)
{
	uint64_t	i;

	i = cvt_u64(s, base);
	if (errno)
		return i;
	if (i > UINT32_MAX) {
		errno = ERANGE;
		return UINT32_MAX;
	}
	return i;
}

/*
 * Convert string to uint16_t, set errno if the conversion fails or
 * doesn't fit.  Does not allow unit specifiers.  Sets errno to zero
 * prior to conversion so you can check for bad inputs by examining
 * errno immediately after the call.
 */
uint16_t
cvt_u16(
	char		*s,
	int		base)
{
	uint64_t	i;

	i = cvt_u64(s, base);
	if (errno)
		return i;
	if (i > UINT16_MAX) {
		errno = ERANGE;
		return UINT16_MAX;
	}
	return i;
}

#define EXABYTES(x)	((long long)(x) << 60)
#define PETABYTES(x)	((long long)(x) << 50)
#define TERABYTES(x)	((long long)(x) << 40)
#define GIGABYTES(x)	((long long)(x) << 30)
#define MEGABYTES(x)	((long long)(x) << 20)
#define KILOBYTES(x)	((long long)(x) << 10)

long long
cvtnum(
	size_t		blksize,
	size_t		sectsize,
	const char	*s)
{
	long long	i;
	char		*sp;
	int		c;

	i = strtoll(s, &sp, 0);
	if (i == 0 && sp == s)
		return -1LL;
	if (*sp == '\0')
		return i;

	if (sp[1] != '\0')
		return -1LL;

	c = tolower(*sp);
	switch (c) {
	case 'b':
		if (!blksize)
			return -1LL;
		return i * blksize;
	case 's':
		if (!sectsize)
			return -1LL;
		return i * sectsize;
	case 'k':
		return KILOBYTES(i);
	case 'm':
		return MEGABYTES(i);
	case 'g':
		return GIGABYTES(i);
	case 't':
		return TERABYTES(i);
	case 'p':
		return PETABYTES(i);
	case 'e':
		return  EXABYTES(i);
	}
	return -1LL;
}

#define TO_EXABYTES(x)	((x) / EXABYTES(1))
#define TO_PETABYTES(x)	((x) / PETABYTES(1))
#define TO_TERABYTES(x)	((x) / TERABYTES(1))
#define TO_GIGABYTES(x)	((x) / GIGABYTES(1))
#define TO_MEGABYTES(x)	((x) / MEGABYTES(1))
#define TO_KILOBYTES(x)	((x) / KILOBYTES(1))

void
cvtstr(
	double		value,
	char		*str,
	size_t		size)
{
	char		*fmt;
	int		precise;

	precise = ((double)value * 1000 == (double)(int)value * 1000);

	if (value >= EXABYTES(1)) {
		fmt = precise ? "%.f EiB" : "%.3f EiB";
		snprintf(str, size, fmt, TO_EXABYTES(value));
	} else if (value >= PETABYTES(1)) {
		fmt = precise ? "%.f PiB" : "%.3f PiB";
		snprintf(str, size, fmt, TO_PETABYTES(value));
	} else if (value >= TERABYTES(1)) {
		fmt = precise ? "%.f TiB" : "%.3f TiB";
		snprintf(str, size, fmt, TO_TERABYTES(value));
	} else if (value >= GIGABYTES(1)) {
		fmt = precise ? "%.f GiB" : "%.3f GiB";
		snprintf(str, size, fmt, TO_GIGABYTES(value));
	} else if (value >= MEGABYTES(1)) {
		fmt = precise ? "%.f MiB" : "%.3f MiB";
		snprintf(str, size, fmt, TO_MEGABYTES(value));
	} else if (value >= KILOBYTES(1)) {
		fmt = precise ? "%.f KiB" : "%.3f KiB";
		snprintf(str, size, fmt, TO_KILOBYTES(value));
	} else {
		snprintf(str, size, "%f bytes", value);
	}
}

#define MINUTES_TO_SECONDS(m)	((m) * 60)
#define HOURS_TO_SECONDS(h)	((h) * MINUTES_TO_SECONDS(60))
#define DAYS_TO_SECONDS(d)	((d) * HOURS_TO_SECONDS(24))
#define WEEKS_TO_SECONDS(w)	((w) * DAYS_TO_SECONDS(7))

time64_t
cvttime(
	char		*s)
{
	time64_t	i;
	char		*sp;

	i = strtoll(s, &sp, 0);
	if (i == 0 && sp == s)
		return 0;
	if (*sp == '\0')
		return i;
	if ((*sp == 'm' && sp[1] == '\0') ||
	    (strcmp(sp, "minutes") == 0) ||
	    (strcmp(sp, "minute") == 0))
		return MINUTES_TO_SECONDS(i);
	if ((*sp == 'h' && sp[1] == '\0') ||
	    (strcmp(sp, "hours") == 0) ||
	    (strcmp(sp, "hour") == 0))
		return HOURS_TO_SECONDS(i);
	if ((*sp == 'd' && sp[1] == '\0') ||
	    (strcmp(sp, "days") == 0) ||
	    (strcmp(sp, "day") == 0))
		return DAYS_TO_SECONDS(i);
	if ((*sp == 'w' && sp[1] == '\0') ||
	    (strcmp(sp, "weeks") == 0) ||
	    (strcmp(sp, "week") == 0))
		return WEEKS_TO_SECONDS(i);
	return 0;
}

/*
 * Convert from arbitrary user strings into a numeric ID.
 * If it's all numeric, we convert that inplace, else we do
 * the name lookup, and return the found identifier.
 */

prid_t
prid_from_string(
	char		*project)
{
	fs_project_t	*prj;
	unsigned long	prid_long;
	char		*sp;

	/*
	 * Allow either a full numeric or a valid projectname, even
	 * if it starts with a digit.
	 */
	prid_long = strtoul(project, &sp, 10);
	if (*project != '\0' && *sp == '\0') {
		if ((prid_long == ULONG_MAX && errno == ERANGE)
				|| (prid_long > (prid_t)-1))
			return -1;
		return (prid_t)prid_long;
	}
	prj = getprnam(project);
	if (prj)
		return prj->pr_prid;
	return -1;
}

uid_t
uid_from_string(
	char		*user)
{
	struct passwd	*pwd;
	unsigned long	uid_long;
	char		*sp;

	uid_long = strtoul(user, &sp, 10);
	if (sp != user && *sp == '\0') {
		if ((uid_long == ULONG_MAX && errno == ERANGE)
				|| (uid_long > (uid_t)-1))
			return -1;
		return (uid_t)uid_long;
	}
	pwd = getpwnam(user);
	if (pwd)
		return pwd->pw_uid;
	return -1;
}

gid_t
gid_from_string(
	char		*group)
{
	struct group	*grp;
	unsigned long	gid_long;
	char		*sp;

	gid_long = strtoul(group, &sp, 10);
	if (sp != group && *sp == '\0') {
		if ((gid_long == ULONG_MAX && errno == ERANGE)
				|| (gid_long > (gid_t)-1))
			return -1;
		return (gid_t)gid_long;
	}
	grp = getgrnam(group);
	if (grp)
		return grp->gr_gid;
	return -1;
}
