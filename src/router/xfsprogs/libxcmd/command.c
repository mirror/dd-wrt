// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "platform_defs.h"
#include "command.h"
#include "input.h"

cmdinfo_t	*cmdtab;
int		ncmds;

static iterfunc_t	iter_func;
static checkfunc_t	check_func;

struct cmdline {
	char	*cmdline;
	bool	iterate;
};

static int		ncmdline;
static struct cmdline	*cmdline;

static int
compare(const void *a, const void *b)
{
	return strcmp(((const cmdinfo_t *)a)->name,
		      ((const cmdinfo_t *)b)->name);
}

void
add_command(
	const cmdinfo_t	*ci)
{
	cmdtab = realloc((void *)cmdtab, ++ncmds * sizeof(*cmdtab));
	cmdtab[ncmds - 1] = *ci;
	qsort(cmdtab, ncmds, sizeof(*cmdtab), compare);
}

static int
check_command(
	const cmdinfo_t	*ci)
{
	/* always run internal library supplied commands */
	if (ci->flags & CMD_FLAG_LIBRARY)
		return 1;

	if (check_func)
		return check_func(ci);
	return 1;
}

void
add_check_command(
	checkfunc_t	cf)
{
	check_func = cf;
}

int
command_usage(
	const cmdinfo_t *ci)
{
	printf("%s %s -- %s\n", ci->name, ci->args, ci->oneline);
	return 0;
}

int
command(
	const cmdinfo_t	*ct,
	int		argc,
	char		**argv)
{
	char		*cmd = argv[0];

	if (!check_command(ct))
		return 0;

	if (argc-1 < ct->argmin || (ct->argmax != -1 && argc-1 > ct->argmax)) {
		if (ct->argmax == -1)
			fprintf(stderr,
	_("bad argument count %d to %s, expected at least %d arguments\n"),
				argc-1, cmd, ct->argmin);
		else if (ct->argmin == ct->argmax)
			fprintf(stderr,
	_("bad argument count %d to %s, expected %d arguments\n"),
				argc-1, cmd, ct->argmin);
		else
			fprintf(stderr,
	_("bad argument count %d to %s, expected between %d and %d arguments\n"),
			argc-1, cmd, ct->argmin, ct->argmax);
		return 0;
	}
	platform_getoptreset();
	return ct->cfunc(argc, argv);
}

const cmdinfo_t *
find_command(
	const char	*cmd)
{
	cmdinfo_t	*ct;

	for (ct = cmdtab; ct < &cmdtab[ncmds]; ct++) {
		if (strcmp(ct->name, cmd) == 0 ||
		    (ct->altname && strcmp(ct->altname, cmd) == 0))
			return (const cmdinfo_t *)ct;
	}
	return NULL;
}

void
add_user_command(char *optarg)
{
	ncmdline++;
	cmdline = realloc(cmdline, sizeof(struct cmdline) * (ncmdline));
	if (!cmdline) {
		perror("realloc");
		exit(1);
	}
	cmdline[ncmdline-1].cmdline = optarg;
	cmdline[ncmdline-1].iterate = true;

}

void
add_oneshot_user_command(char *optarg)
{
	ncmdline++;
	cmdline = realloc(cmdline, sizeof(struct cmdline) * (ncmdline));
	if (!cmdline) {
		perror("realloc");
		exit(1);
	}
	cmdline[ncmdline-1].cmdline = optarg;
	cmdline[ncmdline-1].iterate = false;
}

/*
 * Run a command, iterating as necessary. Return 0 for success, non-zero
 * if an error occurred. Errors terminate loop iteration immediately.
 */
static int
iterate_command(
	const cmdinfo_t	*ct,
	int		argc,
	char		**argv)
{
	int		error = 0;
	int		j;

	/* if there's nothing to iterate, we're done! */
	if (!iter_func)
		return 0;

	for (j = iter_func(0); j; j = iter_func(j)) {
		error = command(ct, argc, argv);
		if (error)
			break;

	}

	return error;
}

void
add_command_iterator(
	iterfunc_t	func)
{
	iter_func = func;
}

static int
process_input(
	char		*input,
	bool		iterate)
{
	char		**v;
	const cmdinfo_t	*ct;
	int		c = 0;
	int		error = 0;

	v = breakline(input, &c);
	if (!c)
		goto out;

	ct = find_command(v[0]);
	if (!ct) {
		fprintf(stderr, _("command \"%s\" not found\n"), v[0]);
		goto out;
	}

	/* oneshot commands don't iterate */
	if (!iterate || (ct->flags & CMD_FLAG_ONESHOT))
		error = command(ct, c, v);
	else
		error = iterate_command(ct, c, v);
out:
	doneline(input, v);
	return error;
}

void
command_loop(void)
{
	char	*input;
	int	done = 0;
	int	i;

	if (!cmdline) {
		/* interactive mode */
		while (!done) {
			input = fetchline();
			if (!input)
				break;
			done = process_input(input, false);
		}
		return;
	}

	/* command line mode */
	for (i = 0; !done && i < ncmdline; i++) {
		input = strdup(cmdline[i].cmdline);
		if (!input) {
			fprintf(stderr,
				_("cannot strdup command '%s': %s\n"),
				cmdline[i].cmdline, strerror(errno));
			exit(1);
		}
		done = process_input(input, cmdline[i].iterate);
	}
	free(cmdline);
	return;
}

void
report_io_times(
	const char		*verb,
	struct timeval		*t2,
	long long		offset,
	long long		count,
	long long		total,
	int			ops,
	int			compact)
{
	char			s1[64], s2[64], ts[64];

	timestr(t2, ts, sizeof(ts), compact ? VERBOSE_FIXED_TIME : 0);
	if (!compact) {
		cvtstr((double)total, s1, sizeof(s1));
		cvtstr(tdiv((double)total, *t2), s2, sizeof(s2));
		printf(_("%s %lld/%lld bytes at offset %lld\n"),
			verb, total, count, (long long)offset);
		printf(_("%s, %d ops; %s (%s/sec and %.4f ops/sec)\n"),
			s1, ops, ts, s2, tdiv((double)ops, *t2));
	} else {/* bytes,ops,time,bytes/sec,ops/sec */
		printf("%lld,%d,%s,%.3f,%.3f\n",
			total, ops, ts,
			tdiv((double)total, *t2), tdiv((double)ops, *t2));
	}
}
