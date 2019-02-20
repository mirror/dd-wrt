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
#ifndef _LVM_CLOG_FUNCTIONS_H
#define _LVM_CLOG_FUNCTIONS_H

#include "device_mapper/misc/dm-log-userspace.h"
#include "cluster.h"

#define LOG_RESUMED   1
#define LOG_SUSPENDED 2

int local_resume(struct dm_ulog_request *rq);
int cluster_postsuspend(char *, uint64_t);

int do_request(struct clog_request *rq, int server);
int push_state(const char *uuid, uint64_t luid,
	       const char *which, char **buf, uint32_t debug_who);
int pull_state(const char *uuid, uint64_t luid,
	       const char *which, char *buf, int size);

int log_get_state(struct dm_ulog_request *rq);
int log_status(void);
void log_debug(void);

#endif /* _LVM_CLOG_FUNCTIONS_H */
