/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_EXEC_H
#define _LVM_EXEC_H

#include "lib/misc/lib.h"

struct cmd_context;

/**
 * Execute command with paramaters and return status
 *
 * \param rstatus
 * Returns command's exit status code.
 *
 * \param sync_needed
 * Bool specifying whether local devices needs to be synchronized
 * before executing command.
 * Note: You cannot synchronize devices within activation context.
 *
 * \return
 * 1 (success) or 0 (failure).
 */
int exec_cmd(struct cmd_context *cmd, const char *const argv[],
	     int *rstatus, int sync_needed);


struct FILE;
struct pipe_data {
	FILE *fp;
	pid_t pid;
};

/**
 * popen() like function to read-only output from executed command
 * without running shell.
 *
 * \param argv
 * Arguments for execvp.
 *
 * \param sync_needed
 * Bool specifying whether local devices needs to be synchronized
 * before executing command.
 * Note: You cannot synchronize devices within activation context.
 *
 * \param pdata
 * Arguments to store data needed for pclose_exec().
 *
 * \return
 * 1 (success) or 0 (failure).
 */
FILE *pipe_open(struct cmd_context *cmd, const char *const argv[],
		int sync_needed, struct pipe_data *pdata);

int pipe_close(struct pipe_data *pdata);

#endif
