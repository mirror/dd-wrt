// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "platform_defs.h"
#include "input.h"
#include <ctype.h>
#include <stdbool.h>

#ifdef ENABLE_EDITLINE
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

#ifdef ENABLE_EDITLINE
static char *el_get_prompt(EditLine *e) { return get_prompt(); }
char *
fetchline(void)
{
	static EditLine	*el;
	static History	*hist;
	HistEvent	hevent;
	const char	*cmd;
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
		el_set(el, EL_EDITOR, "emacs");
	}
	cmd = el_gets(el, &count);
	if (!cmd)
		return NULL;

	line = strdup(cmd);
	if (!line)
		return NULL;

	if (count > 0)
		line[count-1] = '\0';
	if (*line)
		history(hist, &hevent, H_ENTER, line);
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
	printf("%s", get_prompt());
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

#define HOURS(sec)		((sec) / (60 * 60))
#define MINUTES(sec)		(((sec) % (60 * 60)) / 60)
#define SECONDS(sec)		((sec) % 60)
#define USEC_TO_100THS(usec)	((usec) / 1000 / 10)

void
timestr(
	struct timeval	*tv,
	char		*ts,
	size_t		size,
	int		format)
{
	if (format & TERSE_FIXED_TIME) {
		if (!HOURS(tv->tv_sec)) {
			snprintf(ts, size, "%u:%02u.%02u",
				(unsigned int) MINUTES(tv->tv_sec),
				(unsigned int) SECONDS(tv->tv_sec),
				(unsigned int) USEC_TO_100THS(tv->tv_usec));
			return;
		}
		format |= VERBOSE_FIXED_TIME;	/* fallback if hours needed */
	}

	if ((format & VERBOSE_FIXED_TIME) || tv->tv_sec) {
		snprintf(ts, size, "%u:%02u:%02u.%02u",
			(unsigned int) HOURS(tv->tv_sec),
			(unsigned int) MINUTES(tv->tv_sec),
			(unsigned int) SECONDS(tv->tv_sec),
			(unsigned int) USEC_TO_100THS(tv->tv_usec));
	} else {
		snprintf(ts, size, "0.%04u sec",
			(unsigned int) tv->tv_usec / 100);
	}
}

/*
 * Convert from a pair of arbitrary user strings into a timespec.
 */

int
timespec_from_string(
	const char		*secs,
	const char		*nsecs,
	struct timespec		*ts)
{
	char			*p;
	unsigned long long int	ll;

	if (!secs || !nsecs || !ts)
		return 1;

	ll = strtoull(secs, &p, 0);
	if (*p)
		return 1;
	ts->tv_sec = ll;
	if ((unsigned long long int)ts->tv_sec != ll)
		return 1;

	ll = strtoull(nsecs, &p, 0);
	if (*p)
		return 1;
	ts->tv_nsec = ll;
	if ((unsigned long long int)ts->tv_nsec != ll)
		return 1;

	return 0;
}

bool isdigits_only(
	const char *str)
{
	int i;

	for (i = 0; i < strlen(str); i++) {
		if (!isdigit(str[i]))
			return false;
	}
	return true;
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
