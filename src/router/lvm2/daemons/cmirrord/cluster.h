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
#ifndef _LVM_CLOG_CLUSTER_H
#define _LVM_CLOG_CLUSTER_H

#include "libdm/misc/dm-log-userspace.h"
#include "libdm/libdevmapper.h"

#define DM_ULOG_RESPONSE 0x1000U /* in last byte of 32-bit value */
#define DM_ULOG_CHECKPOINT_READY 21
#define DM_ULOG_MEMBER_JOIN      22

/*
 * There is other information in addition to what can
 * be found in the dm_ulog_request structure that we
 * need for processing.  'clog_request' is the wrapping
 * structure we use to make the additional fields
 * available.
 */
struct clog_request {
	/*
	 * If we don't use a union, the structure size will
	 * vary between 32-bit and 64-bit machines.  So, we
	 * pack two 64-bit version numbers in there to force
	 * the size of the structure to be the same.
	 *
	 * The two version numbers also help us with endian
	 * issues.  The first is always little endian, while
	 * the second is in native format of the sending
	 * machine.  If the two are equal, there is no need
	 * to do endian conversions.
	 */
	union {
		uint64_t version[2]; /* LE version and native version */
		struct dm_list list;
	} u;

	/*
	 * 'originator' is the machine from which the requests
	 * was made.
	 */
	uint32_t originator;

	/*
	 * 'pit_server' is the "point-in-time" server for the
	 * request.  (I.e.  The machine that was the server at
	 * the time the request was issued - only important during
	 * startup.
	 */
	uint32_t pit_server;

	/*
	 * The request from the kernel that is being processed
	 */
	struct dm_ulog_request u_rq;
};

int init_cluster(void);
void cleanup_cluster(void);
void cluster_debug(void);

int create_cluster_cpg(char *uuid, uint64_t luid);
int destroy_cluster_cpg(char *uuid);

int cluster_send(struct clog_request *rq);

#endif /* _LVM_CLOG_CLUSTER_H */
