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

#ifndef LVM_MEMLOCK_H
#define LVM_MEMLOCK_H

struct cmd_context;

/*
 * Inside a critical section, memory is always locked.
 *
 * After leaving the critical section, memory stays locked until 
 * memlock_unlock() is called.  This happens with
 * sync_local_dev_names() and sync_dev_names().
 *
 * This allows critical sections to be entered and exited repeatedly without
 * incurring the expense of locking memory every time.
 *
 * memlock_reset() is necessary to clear the state after forking (polldaemon).
 */

void critical_section_inc(struct cmd_context *cmd, const char *reason);
void critical_section_dec(struct cmd_context *cmd, const char *reason);
int critical_section(void);
int prioritized_section(void);
void memlock_inc_daemon(struct cmd_context *cmd);
void memlock_dec_daemon(struct cmd_context *cmd);
int memlock_count_daemon(void);
void memlock_init(struct cmd_context *cmd);
void memlock_reset(void);
void memlock_unlock(struct cmd_context *cmd);

#endif
