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
 * $Id: storage.c,v 1.14 2004/05/06 15:17:45 vlm Exp $
 */

#include "rflow.h"
#include "cfgvar.h"
#include "storage.h"

/*
 * Functions prototypes.
 */
void flow_aggregate(flow_t *flow, agg_e aggregate);
static void _storage_add(flow_storage_t *storage, flow_el_t *el);
static int _flow_hash_value(flow_t *flow);
static flow_t *_get_flow_byexample(flow_storage_t *storage, flow_t *key);

static double get_now() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec + (double)tv.tv_usec / 1000000;
}

/*
 * Active, checkpoint and NetFlow storages (flow caches).
 */
#if AFLOW
flow_storage_t active_storage;
flow_storage_t checkpoint_storage;
#endif
flow_storage_t netflow_storage;

/*
 * Add a flow into provided storage.
 */
int
flow_update(flow_storage_t *storage, flow_t *flow, agg_e aggregate) {
	flow_t *existing_flow;
	flow_el_t *el;
	int ret = 0;

	if(aggregate)
		flow_aggregate(flow, aggregate);

	/*
	 * Lock storage.
	 */
	lock_storage(storage);

	/*
	 * Get the existing flow from the flows hash.
 	 */
	existing_flow = _get_flow_byexample(storage, flow);
	if(existing_flow) {
		existing_flow->packets += flow->packets;
		existing_flow->bytes += flow->bytes;
		existing_flow->tcp_flags |= flow->tcp_flags;
		if(storage == &netflow_storage)
			existing_flow->seen_last = get_now();
		unlock_storage(storage);
		return 0;
	}

	/*
	 * If memory constraint are OK, add a new flow.
	 */
	if(conf->memsize
	  && (storage->entries * sizeof(flow_el_t)) > conf->memsize) {

		/*
		 * Update the "lost stuff" counters.
		 */
		if(storage->first_miss)
			time(&storage->first_miss);
		storage->missed_packets += flow->packets;
		storage->missed_bytes += flow->bytes;

		unlock_storage(storage);

		errno = ENOMEM;
		return -1;
	}

	el = malloc(sizeof *el);
	if(el) {
		/*
		 * New flow is created.
		 */
		memcpy(&el->flow, flow, sizeof(flow_t));
		if(storage == &netflow_storage) {
			el->flow.seen_first = get_now();
			el->flow.seen_last = el->flow.seen_first;
		}
		_storage_add(storage, el);
	} else {
		/*
		 * Update error counters.
		 */
		if(storage->first_miss)
			time(&storage->first_miss);
		storage->missed_packets += flow->packets;
		storage->missed_bytes += flow->bytes;
		if(storage == &netflow_storage) {
			/* Also indicate missing flow */
			netflow_storage.flows_count++;
		}
		ret = -1;
	}

	/*
	 * Unlock storage.
	 */
	unlock_storage(storage);

	return ret;
}

void
clear_storage(flow_storage_t *storage, int do_not_lock) {
	flow_el_t *storage_head, *el, *next;

	if(!do_not_lock) {
		lock_storage(storage);
	}

	/*
	 * Clear storage, remembering the flow elements chain.
	 */
		storage_head = storage->head;
		memset(storage->buckets, 0,
			storage->numbuckets * sizeof(storage->buckets[0]));
		time(&storage->create_time);
		storage->head = 0;
		storage->entries = 0;
		storage->first_miss = 0;
		storage->missed_packets = 0;
		storage->missed_bytes = 0;

	if(!do_not_lock) {
		unlock_storage(storage);
	}

	/*
	 * Slowly and sadly delete all elements (lock-free).
	 */
	for(el = storage_head; el; el = next) {
		next = el->hash_next;
		free(el);
	}
}

#if AFLOW
void
save_checkpoint(flow_storage_t *main, flow_storage_t *checkpoint) {
	flow_el_t **	cp_buckets;
	int		cp_numbuckets;

	lock_storage(main);
	lock_storage(checkpoint);

	clear_storage(checkpoint, 1);
	assert(checkpoint->entries == 0);

	/*
	 * Save certain elements of the checkpoint storage.
	 */
	cp_buckets = checkpoint->buckets;
	cp_numbuckets = checkpoint->numbuckets;

	/*
	 * Move all the data from main to checkpoint.
	 */
#define	_move(foo) do { checkpoint->foo = main->foo; main->foo = 0; } while(0)
	_move(buckets);
	_move(head);
	_move(numbuckets);
	_move(entries);
	_move(create_time);
	_move(first_miss);
	_move(missed_packets);
	_move(missed_bytes);

	/*
	 * Restore certain elements from the checkpoint storage.
	 */
	main->buckets = cp_buckets;
	main->numbuckets = cp_numbuckets;

	unlock_storage(checkpoint);
	unlock_storage(main);
}
#endif


/*
 * Storage must be locked at this point!
 */
int
get_flow_table(flow_storage_t *storage, flow_t **r_flows, int *r_size) {
	int entries = storage->entries;
	flow_t *flows;
	flow_el_t *el;

	if(entries == 0) {
		*r_size = 0;
		*r_flows = NULL;
		return 0;
	}

	flows = malloc(entries * sizeof(flow_t));
	if(flows == NULL) {
		*r_size = -1;
		*r_flows = NULL;
		return -1;
	}

	*r_flows = flows;
	*r_size = entries;

	for(el = storage->head; el; el = el->hash_next) {
		*flows = el->flow;
		entries--;
		flows++;
	}

	assert(entries == 0);

	return 0;
}


/*
 * Internal functions.
 */

/*
 * Perform a flow aggregation based on configured entries.
 */
void
flow_aggregate(flow_t *flow, agg_e aggregate) {
	u_int32_t src = flow->src.s_addr;
	u_int32_t dst = flow->dst.s_addr;
	int src_port = flow->src_port;
	int dst_port = flow->dst_port;
	struct atable *at;
	enum {
		NOT_DONE = 0,	/* Nothing is aggregated */
		SRC_DONE = 1,	/* Source aggregated */
		DST_DONE = 2,	/* Destination aggregated */
		ALL_DONE = 3	/* Everything aggregated */
	} did;

	/*
	 * IP aggregation.
	 */
	if(aggregate & AGG_IPS) {
		flow->src_mask = 32;
		flow->dst_mask = 32;

		/*
		 * IP ranges aggregation
		 */
		did = NOT_DONE;
		for(at = conf->atable; at; at = at->next) {
			if(!(did & SRC_DONE)
			&& ((src & at->mask.s_addr) == at->ip.s_addr)) {
				flow->src.s_addr = src & at->strip.s_addr;
				flow->src_mask = at->strip_bits;
				if((did |= SRC_DONE) == ALL_DONE)
					break;
			}
	
			if(!(did & DST_DONE)
			&& ((dst & at->mask.s_addr) == at->ip.s_addr)) {
				flow->dst.s_addr = dst & at->strip.s_addr;
				flow->dst_mask = at->strip_bits;
				if((did |= DST_DONE) == ALL_DONE)
					break;
			}
		}
	}

	/*
	 * Port ranges aggregation.
	 */
	if((aggregate & AGG_PORTS)
	&& src_port != -1	/* && dst_port != -1, guaranteed */
	&& flow->ip_p != IPPROTO_ICMP) {
		assert(!(src_port & ~0xffff));	/* 0..65535 */
		assert(!(dst_port & ~0xffff));	/* 0..65535 */
		flow->src_port = agr_portmap[src_port];
		flow->dst_port = agr_portmap[dst_port];
	}

}

static void
_storage_add(flow_storage_t *storage, flow_el_t *el) {
	flow_el_t **bucket;

	if(storage->buckets == NULL) {
		int buckets_number = 65537;	/* Large enough prime number */

		storage->buckets = calloc(buckets_number, sizeof(flow_el_t *));
		if(storage->buckets == NULL) {
			if(storage->first_miss)
				time(&storage->first_miss);
			storage->missed_packets += el->flow.packets;
			storage->missed_bytes += el->flow.bytes;
			free(el);
			return;
		}
		storage->numbuckets = buckets_number;
	}

	el->hash_value = _flow_hash_value(&el->flow);

	bucket = &storage->buckets[ el->hash_value % storage->numbuckets ];

	/*
	 * Add to the bucket (prepend).
	 */
	el->bucket_prev = NULL;
	if((el->bucket_next = *bucket))
		el->bucket_next->bucket_prev = el;
	*bucket = el;

	/*
	 * Add to the list.
	 */
	el->hash_next = storage->head;
	storage->head = el;

	storage->entries++;
}

static int
_flow_hash_value(flow_t *flow) {
	int h;

	h =
		  flow->src.s_addr
		^ flow->dst.s_addr
		^ (int)flow->ifSource
		^ flow->src_port
		^ flow->dst_port
		^ flow->src_mask
		^ flow->dst_mask
	;

	return (h & 0x7fffffff);
}

static flow_t *
_get_flow_byexample(flow_storage_t *storage, flow_t *key) {
	flow_el_t **bucket, *el;
	int hash_value;
	/*
	 * Cache flow fields locally to prevent pointers traversal.
	 */
	struct in_addr src = key->src;
	struct in_addr dst = key->dst;
	int src_port = key->src_port;
	int dst_port = key->dst_port;

	if(storage->buckets == NULL)
		return NULL;

	hash_value = _flow_hash_value(key);

	bucket = &storage->buckets[ hash_value % storage->numbuckets ];
	for(el = *bucket; el; el = el->bucket_next) {
		if(
			   el->flow.src.s_addr == src.s_addr
			&& el->flow.dst.s_addr == dst.s_addr
			&& el->flow.src_port == src_port
			&& el->flow.dst_port == dst_port
			&& el->flow.ifSource == key->ifSource
			&& el->flow.ip_p == key->ip_p
			&& el->flow.ip_tos == key->ip_tos
			&& el->flow.src_mask == key->src_mask
			&& el->flow.dst_mask == key->dst_mask
		) {

			if(el->bucket_prev) {
				/*
				 * Move this element to the top of bucket.
				 */
				if((el->bucket_prev->bucket_next
						= el->bucket_next))
					el->bucket_next->bucket_prev
						= el->bucket_prev;
				el->bucket_prev = NULL;
				el->bucket_next = *bucket;
				(*bucket)->bucket_prev = el;
				*bucket = el;
			}

			return &el->flow;
		}
	}

	return NULL;
}


