/*
 **************************************************************************
 * Copyright (c) 2014-2018, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * API's
 */
#ifndef ECM_DB_H_
#define ECM_DB_H_

#include "ecm_db_connection.h"
#include "ecm_db_mapping.h"
#include "ecm_db_host.h"
#include "ecm_db_node.h"
#include "ecm_db_iface.h"
#include "ecm_db_listener.h"
#include "ecm_db_multicast.h"
#include "ecm_db_timer.h"

extern spinlock_t ecm_db_lock;

/*
 * Management thread control
 */
extern bool ecm_db_terminate_pending;	/* When true the user has requested termination */

/*
 * Random seed used during hash calculations
 */
extern uint32_t ecm_db_jhash_rnd __read_mostly;

int ecm_db_adv_stats_state_write(struct ecm_state_file_instance *sfi,uint64_t from_data_total, uint64_t to_data_total,
				uint64_t from_packet_total, uint64_t to_packet_total, uint64_t from_data_total_dropped,
				uint64_t to_data_total_dropped, uint64_t from_packet_total_dropped, uint64_t to_packet_total_dropped);
#endif /* ECM_DB_H_ */
