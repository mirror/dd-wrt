/*
 * Copyright (c) 2000-2003 Silicon Graphics, Inc.
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

#include <xfs/libxfs.h>
#include <signal.h>
#include "command.h"
#include "input.h"
#include "output.h"
#include "sig.h"
#include "malloc.h"
#include "init.h"

#if defined(ENABLE_READLINE)
# include <readline/history.h>
# include <readline/readline.h>
#elif defined(ENABLE_EDITLINE)
# include <histedit.h>
#endif

int	inputstacksize;
FILE	**inputstack;
FILE	*curinput;

static void	popfile(void);
static int	source_f(int argc, char **argv);

static const cmdinfo_t	source_cmd =
	{ "source", NULL, source_f, 1, 1, 0, N_("source-file"),
	  N_("get commands from source-file"), NULL };

/* our homegrown strtok that understands strings */

static char *
tokenize(
	char        *inp)
{
	static char *last_place = NULL;
	char        *start;
	char        *walk;
	int         in_string = 0;
	int         in_escape = 0;

	if (inp) {
		start = inp;
	} else {
		if (last_place == NULL)
			return NULL;

		/* we're done */
		if (*last_place != '\0')
			return NULL;

		start = last_place + 1;
	}
	last_place = NULL;

	/* eat whitespace */
	while (*start == ' ' || *start == '\t')
		start++;

	walk = start;
	for (;*walk != '\0'; walk++) {
		if (in_escape) {
			in_escape = 0;
			continue;
		}
		if (*walk == '\\')
			in_escape = 1;
		else if (*walk == '\"')
			in_string ^= 1;

		if (!in_string && !in_escape &&
		    (*walk == ' ' || *walk == '\t')) {
			last_place = walk;
			*last_place = '\0';
			break;
		}
	}
	if (walk == start)
		return NULL;

	return start;
}

char **
breakline(
	char	*input,
	int	*count)
{
	int	c;
	char	*inp;
	char	*p;
	char	**rval;

	c = 0;
	inp = input;
	rval = xcalloc(sizeof(char *), 1);
	for (;;) {

		p = tokenize(inp);

		if (p == NULL)
			break;
		inp = NULL;
		c++;
		rval = xrealloc(rval, sizeof(*rval) * (c + 1));
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
	xfree(input);
	xfree(vec);
}

static char *
get_prompt(void)
{
	static char	prompt[FILENAME_MAX + 1];

	if (!prompt[0])
		snprintf(prompt, sizeof(prompt), "%s> ", progname);
	return prompt;
}

static char *
fetchline_internal(void)
{
	char	buf[1024];
	int	iscont;
	size_t	len;
	size_t	rlen;
	char	*rval;

	rval = NULL;
	for (rlen = iscont = 0; ; ) {
		if (inputstacksize == 1) {
			if (iscont)
				dbprintf("... ");
			else
				dbprintf(get_prompt(), progname);
			fflush(stdin);
		}
		if (seenint() ||
		    (!fgets(buf, sizeof(buf), curinput) &&
		     ferror(curinput) && seenint())) {
			clearint();
			dbprintf("^C\n");
			clearerr(curinput);
			if (iscont) {
				iscont = 0;
				rlen = 0;
				if (rval) {
					xfree(rval);
					rval = NULL;
				}
			}
			continue;
		}
		if (ferror(curinput) || feof(curinput) ||
		    (len = strlen(buf)) == 0) {
			popfile();
			if (curinput == NULL) {
				dbprintf("\n");
				return NULL;
			}
			iscont = 0;
			rlen = 0;
			if (rval) {
				xfree(rval);
				rval = NULL;
			}
			continue;
		}
		if (inputstacksize == 1)
			logprintf("%s", buf);
		rval = xrealloc(rval, rlen + len + 1);
		if (rlen == 0)
			rval[0] = '\0';
		rlen += len;
		strcat(rval, buf);
		if (buf[len - 1] == '\n') {
			if (len > 1 && buf[len - 2] == '\\') {
				rval[rlen - 2] = ' ';
				rval[rlen - 1] = '\0';
				rlen--;
				iscont = 1;
			} else {
				rval[rlen - 1] = '\0';
				rlen--;
				break;
			}
		}
	}
	return rval;
}

#ifdef ENABLE_READLINE
char *
fetchline(void)
{
	char	*line;

	if (inputstacksize == 1) {
		line = readline(get_prompt());
		if (line && *line) {
			add_history(line);
			logprintf("%s", line);
		}
	} else {
		line = fetchline_internal();
	}
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

	if (inputstacksize == 1) {
		line = xstrdup(el_gets(el, &count));
		if (line) {
			if (count > 0)
				line[count-1] = '\0';
			if (*line) {
				history(hist, &hevent, H_ENTER, line);
				logprintf("%s", line);
			}
		}
	} else {
		line = fetchline_internal();
	}
	return line;
}
#else
char * fetchline(void) { return fetchline_internal(); }
#endif

static void
popfile(void)
{
	if (inputstacksize == 0) {
		curinput = NULL;
		return;
	}
	if (curinput != stdin)
		fclose(curinput);

	inputstacksize--;
	if (inputstacksize) {
	    inputstack =
		    xrealloc(inputstack, inputstacksize * sizeof(*inputstack));
	    curinput = inputstack[inputstacksize - 1];
	} else {
	    free(inputstack);
	    curinput = NULL;
	    inputstack = NULL;
	}
}

void
pushfile(
	FILE	*file)
{
	inputstack =
		xrealloc(inputstack,
			(inputstacksize + 1) * sizeof(*inputstack));
	inputstacksize++;
	curinput = inputstack[inputstacksize - 1] = file;
}

/* ARGSUSED */
static int
source_f(
	int	argc,
	char	**argv)
{
	FILE	*f;

	f = fopen(argv[1], "r");
	if (f == NULL)
		dbprintf(_("can't open %s\n"), argv[0]);
	else
		pushfile(f);
	return 0;
}

void
input_init(void)
{
	add_command(&source_cmd);
}
