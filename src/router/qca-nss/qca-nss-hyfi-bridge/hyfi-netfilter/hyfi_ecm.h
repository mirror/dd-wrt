/*
 *  QCA Hy-Fi ECM
 *
 * Copyright (c) 2014-2016, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef HYFI_ECM_H_
#define HYFI_ECM_H_

#include <linux/skbuff.h>
#include <linux/types.h>

#define ECM_HYFI_IS_IPPROTO_UDP   (1 << 0)
#define ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_FWD (1 << 1)
#define ECM_HYFI_SHOULD_KEEP_ON_FDB_UPDATE_REV (1 << 2)

struct hyfi_ecm_flow_data_t {
	u_int32_t ecm_serial;
        /* Since NSS connections are bidirectional, but HyFi hash entries
           are unidirectional, need to store both the forward and reverse
           direction hashes with the flow data. */
	u_int32_t hash;
        u_int32_t reverse_hash;
	u_int32_t flag;
	u_int32_t priority;
	u_int16_t seq;
	u_int8_t da[6];
	u_int8_t sa[6];
	/* Time of the last ECM stats update (in jiffies) */
	u_int32_t last_update;

	/* Duration of the last period over which a rate was estimated
	   (in jiffies) */
	u_int32_t last_elapsed_time;

	/* Time last ECM accel / decel command sent to hw (in jiffies) */
	unsigned long cmd_time_begun;

	/* Time last ECM accel / decel command completed by hw (in jiffies) */
	unsigned long cmd_time_completed;
};

static inline void hyfi_ecm_set_flag(struct hyfi_ecm_flow_data_t *flow, u_int32_t flag)
{
	flow->flag |= flag;
}

static inline void hyfi_ecm_clear_flag(struct hyfi_ecm_flow_data_t *flow,
		u_int32_t flag)
{
	flow->flag &= ~flag;
}

/*
 * Periodic stats updates
 * returns:
 * -1: error
 * 0: okay
 * 1: not interested
 * 2: hy-fi not attached
 */
int hyfi_ecm_update_stats(const struct hyfi_ecm_flow_data_t *flow, u_int32_t hash,
	u_int8_t *da, u_int8_t *sa, u_int64_t num_bytes, u_int64_t num_packets,
	u_int32_t time_now, bool *should_keep_on_fdb_update, u_int32_t *new_elapsed_time,
	const char *br_name);

/**
 * @brief An ECM connection is removed from the database, mark
 *        as no longer accelerated in H-Active
 *
 * @param [in] hash  hash of entry to decelerate
 * @param [in] ecm_serial  ECM serial number of entry to
 *                         decelerate
 * @param [in] da  destination MAC address of the flow to
 *                 decelerate
 */
void hyfi_ecm_decelerate(u_int32_t hash, u_int32_t ecm_serial, u_int8_t *da,
	const char *br_name);

/**
 * @brief Check if an ECM connection should be kept.
 *
 * @param [in] flow  HyFi / ECM per flow data
 * @param [in] mac  destination MAC address to evaluate to
 *                  determine if the connection should be kept
 *
 * @return true if the connection should be kept, false
 *         otherwise
 */
bool hyfi_ecm_should_keep(const struct hyfi_ecm_flow_data_t *flow, uint8_t *mac,
	const char *br_name);

/**
 * @brief Determine if the port provided by ECM matches the HyFi
 *        expectation
 *
 * @param [in] flow  HyFi / ECM state for the flow
 * @param [in] to_system_index  system index in the 'to'
 *                              direction provided by ECM
 * @param [in] from_system_index  system index in the 'from'
 *                                direction provided by ECM
 *
 * @return true if ECM and HyFi agree on egress ports, false
 *         otherwise
 */
bool hyfi_ecm_port_matches(const struct hyfi_ecm_flow_data_t *flow,
	int32_t to_system_index, int32_t from_system_index, const char * br_name);

/**
 * @brief Determine if a device is attached to the HyFi bridge
 *
 * @param [in] system_index  system index of the device
 *
 * @return true if system_index is for a device attached to the
 *         HyFi bridge, false if not
 */
bool hyfi_ecm_is_port_on_hyfi_bridge(int32_t system_index);

/**
 * @brief Determine if the HyFi bridge is attached
 *
 * @return Instance of the HyFi bridge that is attached with br_name,
 *         NULL if not
 */
struct hyfi_net_bridge * hyfi_ecm_bridge_attached(const char *br_name);

#endif /* HYFI_ECM_H_ */
