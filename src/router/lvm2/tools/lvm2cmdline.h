/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_CMDLINE_H
#define _LVM_CMDLINE_H

struct cmd_context;

struct cmdline_context {
	struct opt_name *opt_names;
	struct command *commands;
	int num_commands;
	struct command_name *command_names;
	int num_command_names;
};

int lvm2_main(int argc, char **argv);

void *cmdlib_lvm2_init(unsigned static_compile);
void lvm_fin(struct cmd_context *cmd);

struct cmd_context *init_lvm(unsigned set_connections, unsigned set_filters);
int lvm_register_commands(struct cmd_context *cmdtool, const char *name);
int lvm_split(char *str, int *argc, char **argv, int max);
int lvm_run_command(struct cmd_context *cmd, int argc, char **argv);
int lvm_return_code(int ret);
int lvm_shell(struct cmd_context *cmd, struct cmdline_context *cmdline);

#endif
