/*
 **************************************************************************
 * Copyright (c) 2015-2016, 2020-2021 The Linux Foundation. All rights reserved.
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

#ifndef __ECM_SFE_IPV4_H
#define __ECM_SFE_IPV4_H
#include <sfe_api.h>

extern int ecm_sfe_ipv4_no_action_limit_default;		/* Default no-action limit. */
extern int ecm_sfe_ipv4_driver_fail_limit_default;		/* Default driver fail limit. */
extern int ecm_sfe_ipv4_nack_limit_default;			/* Default nack limit. */
extern int ecm_sfe_ipv4_accelerated_count;			/* Total offloads */
extern int ecm_sfe_ipv4_pending_accel_count;			/* Total pending offloads issued to the SFE / awaiting completion */
extern int ecm_sfe_ipv4_pending_decel_count;			/* Total pending deceleration requests issued to the SFE / awaiting completion */

/*
 * Limiting the acceleration of connections.
 *
 * By default there is no acceleration limiting.
 * This means that when ECM has more connections (that can be accelerated) than the acceleration
 * engine will allow the ECM will continue to try to accelerate.
 * In this scenario the acceleration engine will begin removal of existing rules to make way for new ones.
 * When the accel_limit_mode is set to FIXED ECM will not permit more rules to be issued than the engine will allow.
 */
extern uint32_t ecm_sfe_ipv4_accel_limit_mode;

/*
 * Locking of the classifier - concurrency control for file global parameters.
 * NOTE: It is safe to take this lock WHILE HOLDING a feci->lock.  The reverse is NOT SAFE.
 */
extern spinlock_t ecm_sfe_ipv4_lock;			/* Protect against SMP access between netfilter, events and private threaded function. */

/*
 * SFE linkage
 */
extern struct sfe_ctx_instance *ecm_sfe_ipv4_mgr;

/*
 * ecm_sfe_ipv4_accel_pending_set()
 *	Set pending acceleration for the connection object.
 *
 * Return false if the acceleration is not permitted or is already in progress.
 */
static inline bool ecm_sfe_ipv4_accel_pending_set(struct ecm_front_end_connection_instance *feci)
{
	DEBUG_INFO("%px: Accel conn: %px\n", feci, feci->ci);

	/*
	 * If re-generation is required then we cannot permit acceleration
	 */
	if (ecm_db_connection_regeneration_required_peek(feci->ci)) {
		DEBUG_TRACE("%px: accel %px failed - regen required\n", feci, feci->ci);
		return false;
	}

	/*
	 * Is connection acceleration permanently failed?
	 */
	spin_lock_bh(&feci->lock);
	if (ECM_FRONT_END_ACCELERATION_FAILED(feci->accel_mode)) {
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%px: accel %px failed\n", feci, feci->ci);
		return false;
	}

	/*
	 * If acceleration mode is anything other than "not accelerated" then ignore.
	 */
	if (feci->accel_mode != ECM_FRONT_END_ACCELERATION_MODE_DECEL) {
		spin_unlock_bh(&feci->lock);
		DEBUG_TRACE("%px: Ignoring wrong mode accel for conn: %px\n", feci, feci->ci);
		return false;
	}

	/*
	 * Do we have a fixed upper limit for acceleration?
	 */
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	if (ecm_sfe_ipv4_accel_limit_mode & ECM_FRONT_END_ACCEL_LIMIT_MODE_FIXED) {
		if ((ecm_sfe_ipv4_pending_accel_count + ecm_sfe_ipv4_accelerated_count) >= sfe_ipv4_max_conn_count()) {
			spin_unlock_bh(&ecm_sfe_ipv4_lock);
			spin_unlock_bh(&feci->lock);
			DEBUG_INFO("%px: Accel limit reached, accel denied: %px\n", feci, feci->ci);
			return false;
		}
	}

	/*
	 * Okay to accelerate
	 */
	ecm_sfe_ipv4_pending_accel_count++;
	spin_unlock_bh(&ecm_sfe_ipv4_lock);

	/*
	 * Okay connection can be set to pending acceleration
	 */
	feci->accel_mode = ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING;
	spin_unlock_bh(&feci->lock);
	return true;
}

/*
 * _ecm_sfe_ipv4_accel_pending_clear()
 *	Clear pending acceleration for the connection object, setting it to the desired state.
 *
 * Returns true if "decelerate was pending".
 *
 * The feci->lock AND ecm_sfe_ipv4_lock must be held on entry.
 */
static inline bool _ecm_sfe_ipv4_accel_pending_clear(struct ecm_front_end_connection_instance *feci, ecm_front_end_acceleration_mode_t mode)
{
	bool decel_pending;

	/*
	 * Set the mode away from its accel pending state.
	 */
	DEBUG_ASSERT(feci->accel_mode == ECM_FRONT_END_ACCELERATION_MODE_ACCEL_PENDING, "%px: Accel mode unexpected: %d\n", feci, feci->accel_mode);
	feci->accel_mode = mode;

	/*
	 * Clear decelerate pending flag.
	 * This flag is only set when we are ACCEL_PENDING -
	 * and we are moving from that to the given mode anyway.
	 */
	decel_pending = feci->stats.decelerate_pending;
	feci->stats.decelerate_pending = false;

	/*
	 * Decrement pending counter
	 */
	ecm_sfe_ipv4_pending_accel_count--;
	DEBUG_ASSERT(ecm_sfe_ipv4_pending_accel_count >= 0, "Accel pending underflow\n");
	return decel_pending;
}

/*
 * ecm_sfe_ipv4_accel_pending_clear()
 *	Clear pending acceleration for the connection object, setting it to the desired state.
 */
static inline bool ecm_sfe_ipv4_accel_pending_clear(struct ecm_front_end_connection_instance *feci, ecm_front_end_acceleration_mode_t mode)
{
	bool decel_pending;
	spin_lock_bh(&feci->lock);
	spin_lock_bh(&ecm_sfe_ipv4_lock);
	decel_pending = _ecm_sfe_ipv4_accel_pending_clear(feci, mode);
	spin_unlock_bh(&ecm_sfe_ipv4_lock);
	spin_unlock_bh(&feci->lock);
	return decel_pending;
}

extern void ecm_sfe_ipv4_accel_done_time_update(struct ecm_front_end_connection_instance *feci);
extern void ecm_sfe_ipv4_decel_done_time_update(struct ecm_front_end_connection_instance *feci);
extern int ecm_sfe_ipv4_init(struct dentry *dentry);
extern void ecm_sfe_ipv4_exit(void);
#endif //__ECM_SFE_IPV4_H
