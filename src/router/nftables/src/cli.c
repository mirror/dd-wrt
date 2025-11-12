/*
 * Asynchronous readline-based interactive interface
 *
 * Actually not asynchronous so far, but intended to be.
 *
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#ifdef HAVE_LIBREADLINE
#include <readline/readline.h>
#include <readline/history.h>
#elif defined(HAVE_LIBEDIT)
#include <editline/readline.h>
#else
#include <linenoise.h>
#endif

#include <cli.h>
#include <list.h>

#define CMDLINE_HISTFILE	".nft.history"
#define CMDLINE_PROMPT		"nft> "
#define CMDLINE_QUIT		"quit"

static bool cli_quit;
static int cli_rc;

static void __cli_exit(int rc)
{
	cli_quit = true;
	cli_rc = rc;
}

static char histfile[PATH_MAX];

static void
init_histfile(void)
{
	const char *home;

	home = getenv("HOME");
	if (home == NULL)
		home = "";
	snprintf(histfile, sizeof(histfile), "%s/%s", home, CMDLINE_HISTFILE);
}

#if defined(HAVE_LIBREADLINE)
static void nft_rl_prompt_save(void)
{
	rl_save_prompt();
	rl_clear_message();
	rl_set_prompt(".... ");
}
#define nft_rl_prompt_restore rl_restore_prompt
#elif defined(HAVE_LIBEDIT)
static void nft_rl_prompt_save(void)
{
	rl_set_prompt(".... ");
}
static void nft_rl_prompt_restore(void)
{
	rl_set_prompt("nft> ");
}
#endif

#if defined(HAVE_LIBREADLINE) || defined(HAVE_LIBEDIT)

static struct nft_ctx *cli_nft;
static char *multiline;

static char *cli_append_multiline(char *line)
{
	size_t len = strlen(line);
	bool complete = false;
	char *s;

	if (len == 0)
		return NULL;

	if (line[len - 1] == '\\') {
		line[len - 1] = '\0';
		len--;
	} else if (multiline == NULL)
		return line;
	else
		complete = 1;

	if (multiline == NULL) {
		multiline = line;
		nft_rl_prompt_save();
	} else {
		len += strlen(multiline);
		s = malloc(len + 1);
		if (!s) {
			fprintf(stderr, "%s:%u: Memory allocation failure\n",
				__FILE__, __LINE__);
			cli_exit(EXIT_FAILURE);
			return NULL;
		}
		snprintf(s, len + 1, "%s%s", multiline, line);
		free(multiline);
		multiline = s;
	}
	line = NULL;

	if (complete) {
		line = multiline;
		multiline = NULL;
		nft_rl_prompt_restore();
	}
	return line;
}

static void cli_complete(char *line)
{
	const HIST_ENTRY *hist;
	const char *c;
	LIST_HEAD(msgs);

	if (line == NULL) {
		printf("\n");
		return cli_exit(0);
	}

	line = cli_append_multiline(line);
	if (line == NULL)
		return;

	for (c = line; *c != '\0'; c++)
		if (!isspace(*c))
			break;
	if (*c == '\0')
		return;

	if (!strcmp(line, CMDLINE_QUIT))
		return cli_exit(0);

	/* avoid duplicate history entries */
	hist = history_get(history_length);
	if (hist == NULL || strcmp(hist->line, line))
		add_history(line);

	nft_run_cmd_from_buffer(cli_nft, line);
	free(line);
}
#endif

#if defined(HAVE_LIBREADLINE)

static char **cli_completion(const char *text, int start, int end)
{
	return NULL;
}

int cli_init(struct nft_ctx *nft)
{
	cli_nft = nft;
	rl_readline_name = (char *)"nft";
	rl_instream  = stdin;
	rl_outstream = stdout;

	rl_callback_handler_install(CMDLINE_PROMPT, cli_complete);
	rl_attempted_completion_function = cli_completion;

	init_histfile();

	read_history(histfile);
	history_set_pos(history_length);

	while (!cli_quit)
		rl_callback_read_char();

	return cli_rc;
}

void cli_exit(int rc)
{
	rl_callback_handler_remove();
	rl_deprep_terminal();
	write_history(histfile);

	__cli_exit(rc);
}

#elif defined(HAVE_LIBEDIT)

int cli_init(struct nft_ctx *nft)
{
	char *line;

	cli_nft = nft;
	rl_readline_name = (char *)"nft";
	rl_instream  = stdin;
	rl_outstream = stdout;

	init_histfile();

	read_history(histfile);
	history_set_pos(history_length);

	rl_set_prompt(CMDLINE_PROMPT);
	while (!cli_quit) {
		line = readline(rl_prompt);
		if (!line) {
			cli_exit(0);
			break;
		}
		line = cli_append_multiline(line);
		if (!line)
			continue;

		cli_complete(line);
	}

	return cli_rc;
}

void cli_exit(int rc)
{
	rl_deprep_terminal();
	write_history(histfile);

	__cli_exit(rc);
}

#else /* HAVE_LINENOISE */

int cli_init(struct nft_ctx *nft)
{
	char *line;

	init_histfile();
	linenoiseHistoryLoad(histfile);
	linenoiseSetMultiLine(1);

	while (!cli_quit) {
		line = linenoise(CMDLINE_PROMPT);
		if (!line) {
			cli_exit(0);
			break;
		}
		if (strcmp(line, CMDLINE_QUIT) == 0) {
			cli_exit(0);
		} else if (line[0] != '\0') {
			linenoiseHistoryAdd(line);
			nft_run_cmd_from_buffer(nft, line);
		}
		linenoiseFree(line);
	}

	return cli_rc;
}

void cli_exit(int rc)
{
	linenoiseHistorySave(histfile);

	__cli_exit(rc);
}

#endif /* HAVE_LINENOISE */
