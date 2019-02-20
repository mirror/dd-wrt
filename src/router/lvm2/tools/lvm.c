/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "tools.h"

#include "lvm2cmdline.h"

int main(int argc, char **argv)
{
	return lvm2_main(argc, argv);
}

#ifdef READLINE_SUPPORT

#  include <readline/readline.h>
#  include <readline/history.h>
#  ifndef HAVE_RL_COMPLETION_MATCHES
#    define rl_completion_matches(a, b) completion_matches((char *)a, b)
#    define rl_completion_func_t CPPFunction
#  endif

static struct cmdline_context *_cmdline;

/* List matching commands */
static char *_list_cmds(const char *text, int state)
{
	static int i = 0;
	static size_t len = 0;

	/* Initialise if this is a new completion attempt */
	if (!state) {
		i = 0;
		len = strlen(text);
	}

	while (i < _cmdline->num_command_names)
		if (!strncmp(text, _cmdline->command_names[i++].name, len))
			return strdup(_cmdline->command_names[i - 1].name);

	return NULL;
}

/* List matching arguments */
static char *_list_args(const char *text, int state)
{
	static int match_no = 0;
	static size_t len = 0;
	static struct command_name *cname;

	/* Initialise if this is a new completion attempt */
	if (!state) {
		char *s = rl_line_buffer;
		int j;

		match_no = 0;
		cname = NULL;
		len = strlen(text);

		/* Find start of first word in line buffer */
		while (isspace(*s))
			s++;

		/* Look for word in list of command names */
		for (j = 0; j < _cmdline->num_command_names; j++) {
			const char *p;
			char *q = s;

			p = _cmdline->command_names[j].name;
			while (*p == *q) {
				p++;
				q++;
			}
			if ((!*p) && *q == ' ') {
				cname = _cmdline->command_names + j;
				break;
			}
		}
	}

	if (!cname)
		return NULL;

	/* Short form arguments */
	if (len < 3) {
		while (match_no < cname->num_args) {
			char s[3];
			char c;
			if (!(c = (_cmdline->opt_names +
				   cname->valid_args[match_no++])->short_opt))
				continue;

			sprintf(s, "-%c", c);
			if (!strncmp(text, s, len))
				return strdup(s);
		}
	}

	/* Long form arguments */
	if (match_no < cname->num_args)
		match_no = cname->num_args;

	while (match_no - cname->num_args < cname->num_args) {
		const char *l;
		l = (_cmdline->opt_names +
		     cname->valid_args[match_no++ - cname->num_args])->long_opt;
		if (*(l + 2) && !strncmp(text, l, len))
			return strdup(l);
	}

	return NULL;
}

/* Custom completion function */
static char **_completion(const char *text, int start_pos,
			  int end_pos __attribute__((unused)))
{
	char **match_list = NULL;
	int p = 0;

	while (isspace((int) *(rl_line_buffer + p)))
		p++;

	/* First word should be one of our commands */
	if (start_pos == p)
		match_list = rl_completion_matches(text, _list_cmds);

	else if (*text == '-')
		match_list = rl_completion_matches(text, _list_args);
	/* else other args */

	/* No further completion */
	rl_attempted_completion_over = 1;
	return match_list;
}

static int _hist_file(char *buffer, size_t size)
{
	char *e = getenv("HOME");

	if (dm_snprintf(buffer, size, "%s/.lvm_history", e) < 0) {
		log_error("$HOME/.lvm_history: path too long");
		return 0;
	}

	return 1;
}

static void _read_history(struct cmd_context *cmd)
{
	char hist_file[PATH_MAX];

	if (!_hist_file(hist_file, sizeof(hist_file)))
		return;

	if (read_history(hist_file))
		log_very_verbose("Couldn't read history from %s.", hist_file);

	stifle_history(find_config_tree_int(cmd, shell_history_size_CFG, NULL));
}

static void _write_history(void)
{
	char hist_file[PATH_MAX];

	if (!_hist_file(hist_file, sizeof(hist_file)))
		return;

	if (write_history(hist_file))
		log_very_verbose("Couldn't write history to %s.", hist_file);
}

static int _log_shell_command_status(struct cmd_context *cmd, int ret_code)
{
	log_report_t log_state;

	if (!cmd->cmd_report.log_rh)
		return 1;

	log_state = log_get_report_state();

	return report_cmdlog(cmd->cmd_report.log_rh, REPORT_OBJECT_CMDLOG_NAME,
			     log_get_report_context_name(log_state.context),
			     log_get_report_object_type_name(log_state.object_type),
			     log_state.object_name, log_state.object_id,
			     log_state.object_group, log_state.object_group_id,
			     ret_code == ECMD_PROCESSED ? REPORT_OBJECT_CMDLOG_SUCCESS
							: REPORT_OBJECT_CMDLOG_FAILURE,
			     stored_errno(), ret_code);
}

static void _discard_log_report_content(struct cmd_context *cmd)
{
	if (cmd->cmd_report.log_rh)
		dm_report_destroy_rows(cmd->cmd_report.log_rh);
}

int lvm_shell(struct cmd_context *cmd, struct cmdline_context *cmdline)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char *orig_command_log_selection = NULL;
	int is_lastlog_cmd = 0, argc, ret, i;
	char *input = NULL, *args[MAX_ARGS], **argv;

	rl_readline_name = "lvm";
	rl_attempted_completion_function = (rl_completion_func_t *) _completion;

	_read_history(cmd);
	_cmdline = cmdline;

	cmd->is_interactive = 1;

	if (!report_format_init(cmd))
		return_ECMD_FAILED;

	orig_command_log_selection = dm_pool_strdup(cmd->libmem, find_config_tree_str(cmd, log_command_log_selection_CFG, NULL));
	log_set_report_context(LOG_REPORT_CONTEXT_SHELL);
	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_CMD);

	while (1) {
		report_reset_cmdlog_seqnum();
		if (cmd->cmd_report.log_rh) {
			/*
			 * If previous command was lastlog, reset log report selection to
			 * its original value as set by log/command_log_selection config setting.
			 */
			if (is_lastlog_cmd &&
			    !dm_report_set_selection(cmd->cmd_report.log_rh, orig_command_log_selection))
				log_error("Failed to reset log report selection.");
		}

		log_set_report(cmd->cmd_report.log_rh);
		log_set_report_object_name_and_id(NULL, NULL);

		free(input);
		input = readline("lvm> ");

		/* EOF */
		if (!input) {
			_discard_log_report_content(cmd);
			/* readline sends prompt to stdout */
			printf("\n");
			break;
		}

		/* empty line */
		if (!*input) {
			_discard_log_report_content(cmd);
			continue;
		}

		add_history(input);

		for (i = 0; i < MAX_ARGS; i++)
			args[i] = NULL;

		argv = args;

		if (lvm_split(input, &argc, argv, MAX_ARGS) == MAX_ARGS) {
			_discard_log_report_content(cmd);
			log_error("Too many arguments, sorry.");
			continue;
		}

		if (!argc) {
			_discard_log_report_content(cmd);
			continue;
		}

		if (!strcmp(argv[0], "lvm")) {
			argv++;
			argc--;
		}

		if (!argc) {
			_discard_log_report_content(cmd);
			continue;
		}

		log_set_report_object_name_and_id(argv[0], NULL);

		is_lastlog_cmd = !strcmp(argv[0], "lastlog");

		if (!is_lastlog_cmd)
			_discard_log_report_content(cmd);

		if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit")) {
			_discard_log_report_content(cmd);
			remove_history(history_length - 1);
			log_error("Exiting.");
			break;
		}

		ret = lvm_run_command(cmd, argc, argv);
		if (ret == ENO_SUCH_CMD)
			log_error("No such command '%s'.  Try 'help'.",
				  argv[0]);

                if ((ret != ECMD_PROCESSED) && !error_message_produced()) {
			log_debug(INTERNAL_ERROR "Failed command did not use log_error");
			log_error("Command failed with status code %d.", ret);
		}
		_write_history();

		if (!is_lastlog_cmd)
			_log_shell_command_status(cmd, ret);

		log_set_report(NULL);
		dm_report_group_output_and_pop_all(cmd->cmd_report.report_group);

		if (cmd->cmd_report.log_rh &&
		    !(dm_report_group_push(cmd->cmd_report.report_group,
					   cmd->cmd_report.log_rh,
					   (void *) cmd->cmd_report.log_name))) {
			log_set_report(NULL);
			log_error("Failed to add log report.");
			break;
		}
	}

	log_restore_report_state(saved_log_report_state);
	cmd->is_interactive = 0;

	free(input);

	if (cmd->cmd_report.report_group) {
		if (!dm_report_group_destroy(cmd->cmd_report.report_group))
			stack;
		cmd->cmd_report.report_group = NULL;
	}

	if (cmd->cmd_report.log_rh) {
		dm_report_free(cmd->cmd_report.log_rh);
		cmd->cmd_report.report_group = NULL;
	}

	return 0;
}

#endif	/* READLINE_SUPPORT */
