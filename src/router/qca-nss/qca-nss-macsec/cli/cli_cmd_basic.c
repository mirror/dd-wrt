/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//#include "osal_common.h"
#include "vty.h"
#include "cli_lib.h"
#include "cli.h"
#include "cli_cmd.h"

static int cli_mode_exit_enable(VTY_T *pVty)
{
	pVty->state = VTY_STATE_CLOSE;
	return CLI_OK;
}

static CLI_MODE_T enableMode = {
	CLI_MODE_ENABLE,
	cli_mode_exit_enable,
	NULL
};

DEFCMD(exit_func,
       exit_cmd, "exit", "Exit current mode and down to previous mode\n")
{
	CLI_MODE_FUN_T exitFunc;

	exitFunc = cli_mode_exit_func(pVty->cliMode);
	if (exitFunc != NULL) {
		exitFunc(pVty);
	}

	return CLI_OK;
}

DEFALIAS(exit_func,
	 quit_cmd, "quit", "Exit current mode and down to previous mode\n")

int cli_install_mode_basic_cmds(int mode)
{
	cli_install_cmd(mode, &exit_cmd);
	cli_install_cmd(mode, &quit_cmd);

	return CLI_OK;
}

int cli_cmd_basic_mode_init(void)
{
	/* install enable mode */
	cli_install_mode(&enableMode);

	return CLI_OK;
}

/* Initialize command interface. Install basic nodes and commands. */
int cli_cmd_basic_init(void)
{
	cli_install_mode_basic_cmds(CLI_MODE_ENABLE);

	return CLI_OK;
}
