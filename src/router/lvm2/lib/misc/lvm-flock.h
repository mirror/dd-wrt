/*
 * Copyright (C) 2014 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_FLOCK_H
#define _LVM_FLOCK_H

void init_flock(struct cmd_context *cmd);
int lock_file(const char *file, uint32_t flags);
void release_flocks(int unlock);

#endif /* _LVM_FLOCK_H */
