/*-
 * Copyright (c) 2004 Sam Leffler, Errno Consulting
 * Copyright (c) 2004 Video54 Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
	without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: if_athrate.h 1667 2006-07-04 10:23:35Z kelmo $
 */
#ifndef _ATH_RATECTRL_H_
#define _ATH_RATECTRL_H_

/*
 * Interface definitions for transmit rate control modules for the
 * Atheros driver.
 *
 * A rate control module is responsible for choosing the transmit rate
 * for each data frame.  Management+control frames are always sent at
 * a fixed rate.
 *
 * An instance of the rate control module is attached to each device
 * at attach time and detached when the device is destroyed.  The module
 * may associate data with each device and each node (station).  Both
 * sets of storage are opaque except for the size of the per-node storage
 * which must be provided when the module is attached.
 *
 * The rate control module is notified for each state transition and
 * station association/reassociation.  Otherwise it is queried for a
 * rate for each outgoing frame and provided status from each transmitted
 * frame.  Any ancillary processing is the responsibility of the module
 * (e.g. if periodic processing is required then the module should setup
 * its own timer).
 *
 * In addition to the transmit rate for each frame, the module must also
 * indicate the number of attempts to make at the specified rate.  If this
 * number is != ATH_TXMAXTRY, an additional callback is made to request
 * 3 additional rate/retry pairs.
 */

enum {
	IEEE80211_RATE_AMRR,
	IEEE80211_RATE_MINSTREL,
	IEEE80211_RATE_ONOE,
	IEEE80211_RATE_SAMPLE,
	IEEE80211_RATE_MAX
};

struct ath_softc;
struct ath_node;
struct ath_buf;
struct ieee80211vap;

/* Multi-rare retry: 3 additional rate/retry pairs */
struct ieee80211_mrr {
	int rate0;
	int retries0;
	int rate1;
	int retries1;
	int rate2;
	int retries2;
	int rate3;
	int retries3;
	int privflags;
};

struct ieee80211_rate_ops {
	int ratectl_id;

	/* Attach/detach a rate control module */
	struct ath_ratectrl *(*attach) (struct ath_softc * sc);
	void (*detach)(struct ath_ratectrl * arc);

	/* Register proc entries with a VAP */
	void (*dynamic_proc_register)(struct ieee80211vap * vap);

	/* *** State storage handling *** */

	/* Initialize per-node state already allocated for the specified
	 * node; this space can be assumed initialized to zero */
	void (*node_init)(struct ath_softc * sc, struct ath_node * an);

	/* Cleanup any per-node state prior to the node being reclaimed */
	void (*node_cleanup)(struct ath_softc * sc, struct ath_node * an);

	/* Update rate control state on station associate/reassociate 
	 * (when operating as an ap or for nodes discovered when operating
	 * in ibss mode) */
	void (*newassoc)(struct ath_softc * sc, struct ath_node * an, int isnew);

	/* Update/reset rate control state for 802.11 state transitions.
	 * Important mostly as the analog to newassoc when operating
	 * in station mode */
	void (*newstate)(struct ieee80211vap * vap, enum ieee80211_state state);

	/* *** Transmit handling *** */

	/* Return the transmit info for a data packet.  If multi-rate state
	 * is to be setup then try0 should contain a value other than ATH_TXMAXTRY
	 * and setupxtxdesc will be called after deciding if the frame
	 * can be transmitted with multi-rate retry. */
	void (*findrate)(struct ath_softc * sc, struct ath_node * an, int shortPreamble, size_t frameLen, u_int8_t *rix, unsigned int *try0, u_int8_t *txrate);

	/* Return 3 more rates to try and corresponding number of retries.
	 * The rate index returned by findrate is passed back in. */
	void (*get_mrr)(struct ath_softc * sc, struct ath_node * an, int shortPreamble, size_t frame_size, u_int8_t rix, struct ieee80211_mrr * mrr);

	/* Update rate control state for a packet associated with the
	 * supplied transmit descriptor.  The routine is invoked both
	 * for packets that were successfully sent and for those that
	 * failed (consult the descriptor for details). */
	void (*tx_complete)(struct ath_softc * sc, struct ath_node * an, const struct ath_buf * bf, const struct ieee80211_mrr * mrr);
};

struct ath_ratectrl {
	struct ieee80211_rate_ops *ops;
	size_t arc_space;	/* space required for per-node state */
	size_t arc_vap_space;	/* space required for per-vap state */
};

int ieee80211_rate_register(struct ieee80211_rate_ops *ops);
void ieee80211_rate_unregister(struct ieee80211_rate_ops *ops);

struct ath_ratectrl *ieee80211_rate_attach(struct ath_softc *sc, const char *name);
void ieee80211_rate_detach(struct ath_ratectrl *);
#endif				/* _ATH_RATECTRL_H_ */
