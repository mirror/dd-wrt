/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2014 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_SIGNAL_H
#define _LVM_SIGNAL_H

void remove_ctrl_c_handler(void);
void install_ctrl_c_handler(void);
int init_signals(int suppress_messages);

void sigint_allow(void);
int sigint_caught(void);
void sigint_restore(void);
void sigint_clear(void);

void block_signals(uint32_t flags);
void unblock_signals(void);

#endif
