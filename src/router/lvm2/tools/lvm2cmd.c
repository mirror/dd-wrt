/*
 * Copyright (C) 2006-2007 Red Hat, Inc. All rights reserved.
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

#include "lvm2cmdline.h"
#include "tools/lvm2cmd.h"

void *lvm2_init(void)
{
	return cmdlib_lvm2_init(0);
}

int lvm_shell(struct cmd_context *cmd __attribute__((unused)),
	      struct cmdline_context *cmdline __attribute__((unused)))
{
	return 0;
}
