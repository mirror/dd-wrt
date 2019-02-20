/*
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef _LVM_CLOG_LOCAL_H
#define _LVM_CLOG_LOCAL_H

int init_local(void);
void cleanup_local(void);

int kernel_send(struct dm_ulog_request *rq);

#endif /* _LVM_CLOG_LOCAL_H */
