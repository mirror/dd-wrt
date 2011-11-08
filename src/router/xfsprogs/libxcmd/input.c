/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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
#include <xfs/input.h>
#include <ctype.h>

#if defined(ENABLE_READLINE)
# include <readline/history.h>
# include <readline/readline.h>
#elif defined(ENABLE_EDITLINE)
# include <histedit.h>
#endif

extern char *progname;

static char *
get_prompt(void)
{
	static char	prompt[FILENAME_MAX + 2 /*"> "*/ + 1 /*"\0"*/ ];

	if (!prompt[0])
		snprintf(prompt, sizeof(prompt), "%s> ", progname);
	return prompt;
}

#if defined(ENABLE_READLINE)
char *
fetchline(void)
{
	char	*line;

	line = readline(get_prompt());
	if (line && *line)
		add_history(line);
	return line;
}
#elif defined(ENABLE_EDITLINE)
static char *el_get_prompt(EditLine *e) { return get_prompt(); }
char *
fetchline(void)
{
	static EditLine	*el;
	static History	*hist;
	HistEvent	hevent;
	char		*line;
	int		count;

	if (!el) {
		hist = history_init();
		history(hist, &hevent, H_SETSIZE, 100);
		el = el_init(progname, stdin, stdout, stderr);
		el_source(el, NULL);
		el_set(el, EL_SIGNAL, 1);
		el_set(el, EL_PROMPT, el_get_prompt);
		el_set(el, EL_HIST, history, (const char *)hist);
	}
	line = strdup(el_gets(el, &count));
	if (line) {
		if (count > 0)
			line[count-1] = '\0';
		if (*line)
			history(hist, &hevent, H_ENTER, line);
	}
	return line;
}
#else
# define MAXREADLINESZ	1024
char *
fetchline(void)
{
	char	*p, *line = malloc(MAXREADLINESZ);

	if (!line)
		return NULL;
	printf(get_prompt());
	fflush(stdout);
	if (!fgets(line, MAXREADLINESZ, stdin)) {
		free(line);
		return NULL;
	}
	p = line + strlen(line);
	if (p != line && p[-1] == '\n')
		p[-1] = '\0';
	return line;
}
#endif

char **
breakline(
	char	*input,
	int	*count)
{
	int	c = 0;
	char	*p;
	char	**rval = calloc(sizeof(char *), 1);

	while (rval && (p = strsep(&input, " ")) != NULL) {
		if (!*p)
			continue;
		c++;
		rval = realloc(rval, sizeof(*rval) * (c + 1));
		if (!rval) {
			c = 0;
			break;
		}
		rval[c - 1] = p;
		rval[c] = NULL;
	}
	*count = c;
	return rval;
}

void
doneline(
	char	*input,
	char	**vec)
{
	free(input);
	free(vec);
}

#define EXABYTES(x)	((long long)(x) << 60)
#define PETABYTES(x)	((long long)(x) << 50)
#define TERABYTES(x)	((long long)(x) << 40)
#define GIGABYTES(x)	((long long)(x) << 30)
#define MEGABYTES(x)	((long long)(x) << 20)
#define KILOBYTES(x)	((long long)(x) << 10)

long long
cvtnum(
	size_t		blocksize,
	size_t		sectorsize,
	char		*s)
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
		return i * blocksize;
	case 's':
		return i * sectorsize;
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

unsigned long
cvttime(
	char		*s)
{
	unsigned long	i;
	char		*sp;

	i = strtoul(s, &sp, 0);
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

struct timeval
tadd(struct timeval t1, struct timeval t2)
{
	t1.tv_usec += t2.tv_usec;
	if (t1.tv_usec > 1000000) {
		t1.tv_usec -= 1000000;
		t1.tv_sec++;
	}
	t1.tv_sec += t2.tv_sec;
	return t1;
}

struct timeval
tsub(struct timeval t1, struct timeval t2)
{
	t1.tv_usec -= t2.tv_usec;
	if (t1.tv_usec < 0) {
		t1.tv_usec += 1000000;
		t1.tv_sec--;
	}
	t1.tv_sec -= t2.tv_sec;
	return t1;
}

double
tdiv(double value, struct timeval tv)
{
	return value / ((double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0));
}

#define HOURS(sec)	((sec) / (60 * 60))
#define MINUTES(sec)	(((sec) % (60 * 60)) / 60)
#define SECONDS(sec)	((sec) % 60)

void
timestr(
	struct timeval	*tv,
	char		*ts,
	size_t		size,
	int		format)
{
	double		usec = (double)tv->tv_usec / 1000000.0;

	if (format & TERSE_FIXED_TIME) {
		if (!HOURS(tv->tv_sec)) {
			snprintf(ts, size, "%u:%02u.%02u",
				(unsigned int) MINUTES(tv->tv_sec),
				(unsigned int) SECONDS(tv->tv_sec),
				(unsigned int) usec * 100);
			return;
		}
		format |= VERBOSE_FIXED_TIME;	/* fallback if hours needed */
	}

	if ((format & VERBOSE_FIXED_TIME) || tv->tv_sec) {
		snprintf(ts, size, "%u:%02u:%02u.%02u",
			(unsigned int) HOURS(tv->tv_sec),
			(unsigned int) MINUTES(tv->tv_sec),
			(unsigned int) SECONDS(tv->tv_sec),
			(unsigned int) usec * 100);
	} else {
		snprintf(ts, size, "0.%04u sec", (unsigned int) usec * 10000);
	}
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
	if (sp != user) {
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
	if (sp != group) {
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

#define HAVE_FTW_H 1	/* TODO: configure me */

#ifndef HAVE_FTW_H
int
nftw(
	char	*dir,
	int	(*fn)(const char *, const struct stat *, int, struct FTW *),
	int	depth,
	int	flags)
{
	fprintf(stderr, "%s: not implemented, no recursion available\n",
		__FUNCTION__);
	return 0;
}
#endif
