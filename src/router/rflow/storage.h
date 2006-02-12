/*-
 * Copyright (c) 2002, 2003, 2004 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: storage.h,v 1.9 2004/04/12 10:20:30 vlm Exp $
 */

#ifndef	__STORAGE_H__
#define	__STORAGE_H__

#include "headers.h"

/*
 * Structure describing the single flow.
 */
typedef struct flow_s {
	/*
	 * Flow source and destination.
	 */
	struct in_addr src;	/* Source IP address */
	struct in_addr dst;	/* Destination IP address */

	/* Ports gathered when "capture-ports enable" is set */
	int	src_port;	/* Source IP port (-1 if not available) */
	int	dst_port;	/* Destination IP port (-1 if not available) */

	/*
	 * Flow counters.
	 */
	unsigned long long bytes;
	size_t	packets;

	/*
	 * Things meaningful if NetFlow is enabled.
	 */
	void	*ifSource;	/* Packet source */
	u_char	src_mask;	/* Source IP prefix mask bits */
	u_char	dst_mask;	/* Destination IP prefix mask bits */
	u_char	ip_p;		/* IP Protocol */
	u_char	ip_tos;		/* IP TypeOfService */
	u_char	tcp_flags;	/* TCP flags */

	double	seen_first;	/* First packet in the flow */
	double	seen_last;	/* Last packet in the flow */

	char ifName[IFNAMSIZ];	/* if(ifIndex==0), for import into RSH-based */
} flow_t;
#define	FLOW_EMPTY_VALUE { {0}, {0}, -1, -1 }	/* Used in static inits */

/*
 * Wrapper to include the flow into a hash.
 */
typedef struct flow_el_s {

	flow_t flow;	/* Flow itself */

	/*
	 * Hash value of the flow.
	 */
	int hash_value;

	/*
	 * Place in the bucket.
	 */
	struct flow_el_s *bucket_prev;
	struct flow_el_s *bucket_next;

	/*
	 * Place in the global list.
	 */
	struct flow_el_s *hash_next;
} flow_el_t;


/*
 * Hash of flows.
 */
typedef struct flow_storage_s {
	flow_el_t **buckets;	/* Hash buckets */
	flow_el_t *head;	/* Linked list of elements */
	int numbuckets;		/* Number of bytes allocated */
	int entries;		/* Number of flow elements linked in */

	time_t	create_time;	/* Hash creation time */

	time_t	first_miss;	/* Time when problems started to happen */
	long long missed_packets;	/* Number of lost packets */
	long long missed_bytes;		/* Number of lost bytes */
	u_int32_t missed_flows;	/* Missed NetFlow flows */
	u_int32_t flows_count;	/* Number of NetFlow flows seen so far */

	pthread_mutex_t storage_lock;
} flow_storage_t;

/*
 * Active, checkpoint and NetFlow storages (flow caches).
 */
extern flow_storage_t active_storage;
extern flow_storage_t checkpoint_storage;
extern flow_storage_t netflow_storage;

#define	lock_storage(foo)	pthread_mutex_lock(&(foo)->storage_lock)
#define	unlock_storage(foo)	pthread_mutex_unlock(&(foo)->storage_lock)

/*
 * Atomically add flow counters into the list of flows.
 * flow will NOT be consumed (yet may be modified): you may allocate it
 * on the stack. The function may allocate new flow_el_t object or update
 * existing one.
 * If allocation fails, it will increase .missed_* counters in the storage
 * and return -1.
 */
typedef enum {
	AGG_NONE	= 0x00,	/* Do not aggregate */
	AGG_IPS		= 0x01,	/* Aggregate IP addresses */
	AGG_PORTS	= 0x02,	/* Aggregate Port numbers */
	AGG_ALL		= 0x03,	/* Aggregate everything */
} agg_e;
int flow_update(flow_storage_t *storage, flow_t *flow, agg_e aggregate_flow);


/*
 * Remove flows from the storage.
 * Operation is atomic.
 */
void clear_storage(flow_storage_t *storage, int do_not_lock_storage);

/*
 * Move main storage in place of checkpoint.
 * Operation is atomic.
 */
void save_checkpoint(flow_storage_t *main, flow_storage_t *checkpoint);

/*
 * Get a table with flows (array of flow_t).
 * If return value is 0, r_flows and r_size will be initialized
 * to point to a newly allocated table of flows.
 * You must lock storage before calling this function!
 */
int get_flow_table(flow_storage_t *storage, flow_t **r_flows, int *r_size);

#endif	/* __STORAGE_H__ */
