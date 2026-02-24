/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 *
 */

#ifndef _MMRC_H_
#define _MMRC_H_

#include "mmrc_osal.h"

/**
 * Define used to specify 802.11 specification to build MMRC for
 */
#define MMRC_MODE_80211AH 1
#define MMRC_MODE_80211AC 2
/**
 * Define the specification to build MMRC. MMRC_MODE_80211AH is used
 * for the purpose built use case of the MMRC Module. In contrary
 * MMRC_MODE_80211AC is used for testing purposes in ns-3 as 802.11ah
 * is not supported in this environment.
 */
#define MMRC_MODE MMRC_MODE_80211AH

/**
 * The max length of a retry chain for a single packet transmission
 */
#define MMRC_MAX_CHAIN_LENGTH 4

/**
 * Rate minimum allowed attempts
 */
#define MMRC_MIN_CHAIN_ATTEMPTS 1

/**
 * Rate upper limit for attempts
 */
#define MMRC_MAX_CHAIN_ATTEMPTS 2

#ifndef MMRC_SUPP_NUM_MCS
#define MMRC_SUPP_NUM_MCS	(MMRC_MCS7 + 1)
#endif
#ifndef MMRC_SUPP_NUM_BW
#define MMRC_SUPP_NUM_BW	(MMRC_BW_8MHZ + 1)
#endif
#ifndef MMRC_SUPP_NUM_GUARD
#define MMRC_SUPP_NUM_GUARD	(MMRC_GUARD_SHORT + 1)
#endif
#ifndef MMRC_SUPP_NUM_NSS
#define MMRC_SUPP_NUM_NSS	(MMRC_SPATIAL_STREAM_1 + 1)
#endif

/**
 * The default rows of a probability table for a STA.
 * It is derived from hardware support of: 1/2/4/8 MHz, L/SGI,
 * 4 NSS and 10 MCS [0..9] and MCS10 twice for 1 MHz channels only.
 *
 * E.g. For MCS 0-7, BW 1,2,4,8 MHz, LGI and SGI, 1SS, MCS10
 * Total: (8 mcs) * (4 bandwidths) * (2 GI) * (1 NSS) + (2 MCS10) = 66
 */
#define MMRC_DEFAULT_TABLE_SIZE	\
	((MMRC_SUPP_NUM_MCS * MMRC_SUPP_NUM_BW * MMRC_SUPP_NUM_GUARD * MMRC_SUPP_NUM_NSS) + 2)

/**
 * The frequency of MMRC stat table updates
 */
#define MMRC_UPDATE_FREQUENCY_MS 100

/**
 * Used to specify supported features when initialising a STA
 */
#define MMRC_MASK(x) (1u << (x))

/**
 * Flags to be used with a mmrc_rate
 */
enum mmrc_flags {
	/** CTS/RTS flag */
	MMRC_FLAGS_CTS_RTS,
};

/**
 * Rates supported by the MMRC module
 */
enum mmrc_mcs_rate {
	MMRC_MCS0,
	MMRC_MCS1,
	MMRC_MCS2,
	MMRC_MCS3,
	MMRC_MCS4,
	MMRC_MCS5,
	MMRC_MCS6,
	MMRC_MCS7,
	MMRC_MCS8,
	MMRC_MCS9,
#if MMRC_MODE == MMRC_MODE_80211AH
	MMRC_MCS10,
#endif
	MMRC_MCS_UNUSED,
};

/**
 * Bandwidths supported by the MMRC module
 */
enum mmrc_bw {
	MMRC_BW_1MHZ	= 0,
	MMRC_BW_2MHZ	= 1,
	MMRC_BW_4MHZ	= 2,
	MMRC_BW_8MHZ	= 3,
	MMRC_BW_16MHZ	= 4,
	MMRC_BW_20MHZ	= MMRC_BW_1MHZ,
	MMRC_BW_40MHZ	= MMRC_BW_2MHZ,
	MMRC_BW_80MHZ	= MMRC_BW_4MHZ,
	MMRC_BW_160MHZ	= MMRC_BW_8MHZ,
	MMRC_BW_MAX	= 5,
};

/**
 * Spatial streams supported by the MMRC module
 */
enum mmrc_spatial_stream {
	MMRC_SPATIAL_STREAM_1	= 0,
	MMRC_SPATIAL_STREAM_2	= 1,
	MMRC_SPATIAL_STREAM_3	= 2,
	MMRC_SPATIAL_STREAM_4	= 3,
	MMRC_SPATIAL_STREAM_MAX,
};

/**
 * Guards supported by the MMRC module
 */
enum mmrc_guard {
	MMRC_GUARD_LONG		= 0,
	MMRC_GUARD_SHORT	= 1,
	MMRC_GUARD_MAX,
};

#define MMRC_RATE_TO_BITFIELD(x)     ((x) & 0xF)
#define MMRC_ATTEMPTS_TO_BITFIELD(x) ((x) & 0x7)
#define MMRC_GUARD_TO_BITFIELD(x)    ((x) & 0x1)
#define MMRC_SS_TO_BITFIELD(x)       ((x) & 0x3)
#define MMRC_BW_TO_BITFIELD(x)       ((x) & 0x7)
#define MMRC_FLAGS_TO_BITFIELD(x)    ((x) & 0x7)

/**
 * A single rate chain
 */
struct mmrc_rate {
	/** The MCS for this entry in the rate table */
	u8 rate		: 4;

	/** The number of attempts at this rate */
	u8 attempts	: 3;

	/** The guard to be used for this rate */
	u8 guard	: 1;

	/** The spatial streams to be used for this rate */
	u8 ss		: 2;

	/** The bandwidth for this rate */
	u8 bw		: 3;

	/** The flags for this rate */
	u8 flags	: 3;

	/** The index in the mmrc_table */
	u16 index;
};

/**
 * Rate table generated on a per packet basis
 */
struct mmrc_rate_table {
	/** The rates to attempt within this chain */
	struct mmrc_rate rates[MMRC_MAX_CHAIN_LENGTH];
};

#define SGI_PER_BW(bw)		(1 << (bw))

/**
 * Capabilities of an individual STA
 */
struct mmrc_sta_capabilities {
	/** The maximum number of output rates */
	u8 max_rates	: 3;

	/** The maximum retries */
	u8 max_retries	: 3;

	/** The supported bandwidths of the STA (bitfield) */
	u8 bandwidth	: 5;

	/** The supported spatial streams of the STA (bitfield) */
	u8 spatial_streams	: 4;

	/** The supported rates of the STA (bitfield) */
	u16 rates	: 11;

	/** The supported guards of the STA (bitfield) */
	u8 guard	: 2;

	/** Flags of relevant features supported by the STA, e.g. dynamic SMPS... */
	u8 sta_flags	: 4;

	/** Per BW supported guards of the STA (2 bits per BW) */
	u8 sgi_per_bw	: 5;
};

/**
 * Statistics table of a STA
 */
struct mmrc_stats_table {
	/** The average calculated running throughput counter of this rate */
	u32 avg_throughput_counter;

	/** The average running throughput sum of this rate */
	u32 sum_throughput;

	/** The maximum observed calculated throughput of this rate */
	u32 max_throughput;

	/** The number of attempts at sending a packet at this rate since the last update */
	u16 sent;

	/** The number of successfully sent packets at this rate since the last	update */
	u16 sent_success;

	/** The number of succesful MPDUs in acknowledged AMPDUs at this rate since the
	 * last update
	 */
	u16 back_mpdu_success;

	/** The number of failed MPDUs in acknowledged AMPDUs at this rate since the last update */
	u16 back_mpdu_failure;

	/** The total attempts of packets sent at this rate */
	u32 total_sent;

	/** The total successful attempts at sending at this rate */
	u32 total_success;
	/**
	 * A store of evidence as to how relevant a rate is based on how many
	 * times it has been attempted recently
	 */
	u16 evidence;

	/**
	 * The probability this rate will successfully transmit.
	 * This is updated based on the configured EWMA value and the
	 * period set in the configured timer
	 */
	u8 prob;

	/** Have we sent aggregates at this rate since the last update */
	bool have_sent_ampdus;
};

/**
 * Store of information the MMRC module requires for a STA
 */
struct mmrc_table {
	/** The capabilities of the STA */
	struct mmrc_sta_capabilities caps;

	/** The index of the rate with the best tp */
	struct mmrc_rate best_tp;

	/** The index of the rate with the second best tp */
	struct mmrc_rate second_tp;

	/** The index of the rate with the best tp */
	struct mmrc_rate baseline;

	/** The index of the rate with the best probability */
	struct mmrc_rate best_prob;

	/** The index of the fixed rate */
	struct mmrc_rate fixed_rate;

	/** Number of rate control cycles performed */
	u32 cycle_cnt;

	/** Used to do additional lookarounds when traffic is very low */
	u32 last_lookaround_cycle;

	/** Used to decide when to do a lookaround */
	u8 lookaround_cnt;

	/** The ratio of using normal rate and sampling */
	u8 lookaround_wrap;

	/**
	 * A counter that is used to determine when we should force a lookaround.
	 * Should be a portion of the above lookaround with less constraints
	 */
	u8 forced_lookaround;

	/** Counter of attempts at the current lookaround rate */
	u8 current_lookaround_rate_attempts;

	/** Index of the current lookaround rate */
	u16 current_lookaround_rate_index;

	/** Counter for analysis purposes */
	u32 total_lookaround;

	/**
	 * A counter to detect if the current best rate is optimal
	 * and may slow down sample frequency.
	 */
	u32 stability_cnt;

	/** Threshold for sample frequency switch. */
	u32 stability_cnt_threshold;

	/** Best rate variation EWMA */
	u8 probability_variation;

	/** The difference in MCS from each of the last 2 rate changes */
	s8 best_rate_diff[2];

	/** Indication of random versus consistently one-sided variation */
	s8 probability_variation_direction;

	/** Has rate control detected possible interference */
	bool interference_likely;

	/** Has rate control detected the best rate is no longer converged */
	bool unconverged;

	/** Is rate control just entering unconverged state */
	bool newly_unconverged;

	/** Number of rate control cycles the best rate has remained unchanged */
	s32 best_rate_cycle_count;

	/**
	 * The probability table for the STA. This MUST always be the last
	 * element in the struct.
	 *
	 * @note The table may not always be of length @ref MMRC_MAX_TABLE_SIZE as the
	 * @c mmrc_table may be allocated using @ref mmrc_memory_required_for_caps
	 */
	struct mmrc_stats_table table[MMRC_DEFAULT_TABLE_SIZE];
};

/**
 * Initialise the mmrc table, based on the capabilities provided
 *
 * @param tb A pointer to an empty mmrc_sta_capabilities struct.
 * @param caps The capabilities of this STA.
 * @param rssi The average RSSI value for this station.
 *
 * @note If the STA capabilities change this function will need to be called again
 * and the @c mmrc_table may need to be reallocated if allocated using
 * @ref mmrc_memory_required_for_caps
 */
void mmrc_sta_init(struct mmrc_table *tb, struct mmrc_sta_capabilities *caps, s8 rssi);

/**
 * Calculate the size of the mmrc_table required for these capabilities.
 *
 * @param caps The capabilities of this STA.
 *
 * @returns size_t the size of an mmrc_table for this STA
 */
size_t mmrc_memory_required_for_caps(struct mmrc_sta_capabilities *caps);

/**
 * Get a retry chain from MMRC for a specific mmrc_table.
 *
 * @param tb A pointer to a mmrc table to generate rates from.
 * @param out A points to an empty rate table to be populated.
 * @param size The size of the packet to be sent
 */
void mmrc_get_rates(struct mmrc_table *tb,
		    struct mmrc_rate_table *out,
		    size_t size);

/**
 * Feedback to MMRC so the appropriate stats table can be updated.
 *
 * @param tb A pointer to a mmrc table to update
 * @param rates The rate table used to send the last packet
 * @param retry_count The amount of retries attempted using the last
 *	rate table
 */
void mmrc_feedback(struct mmrc_table *tb,
		   struct mmrc_rate_table *rates,
		   s32 retry_count);

/**
 * Feedback to MMRC based on aggregated frames.
 *
 * @param tb Pointer to a mmrc table to update
 * @param rates The rate table used to send the last packet
 * @param retry_count The amount of retries attempted using the last
 *      rate table
 * @param success The amount of successfully sent frames in the A-MPDU
 * @param failure The amount of unsuccessfully sent frames in the A-MPDU
 */
void mmrc_feedback_agg(struct mmrc_table *tb,
		       struct mmrc_rate_table *rates,
		       s32 retry_count,
		       u32 success,
		       u32 failure);

/**
 * Update an MMRC table from the most recent stats.
 * delta_time_ms is currently ignored.
 *
 * @param tb A pointer to a mmrc table to update.
 */
void mmrc_update(struct mmrc_table *tb);

/**
 * Set a fixed rate.
 *
 * @param tb A pointer to a mmrc table to update.
 * @param fixed_rate The fixed rate to be set
 *
 * @returns bool true if setting rate succeeds, otherwise false
 */
bool mmrc_set_fixed_rate(struct mmrc_table *tb, struct mmrc_rate fixed_rate);

/**
 * Calculate the amount of rows occupied by a stations capabilities
 *
 * @param caps A pointer to the desired station capabilities
 * @returns u16 the total number of rows to accommodate all
 *                   capabilities options
 */
u16 rows_from_sta_caps(struct mmrc_sta_capabilities *caps);

/**
 * Calculate the transmit time of a given rate in the mmrc_table based on a
 * default packet size in microseconds
 *
 * @param rate  The rate to calculate the tx time for
 * @returns u32 The tx time of the given rate
 */
u32 get_tx_time(struct mmrc_rate *rate);

/**
 * Validates that the combinations if a given rate is valid
 *
 * @param rate The rate to validate
 * @returns bool If the rate is valid
 */
bool validate_rate(struct mmrc_table *tb, struct mmrc_rate *rate);

/**
 * Takes an index in an mmrc_table and calculate the capabilities
 * of that rate
 *
 * @param tb A pointer to a mmrc table to update
 * @param index A valid index in the mmrc_stats_table (to find the
 *              upper index limit use rows_from_sta_caps())
 * @returns struct mmrc_rate with the updated rate parameters
 */
struct mmrc_rate get_rate_row(struct mmrc_table *tb, u16 index);

/**
 * Set a fixed rate.
 *
 * @param tb A pointer to a mmrc table to update
 * @param fixed_rate The fixed rate to be set
 *
 * @param tb A pointer to a mmrc table to update
 * @param rate The rate to be updated with the current index
 */
void rate_update_index(struct mmrc_table *tb, struct mmrc_rate *rate);

/**
 * Calculate the theoretical thoughput of a given rate
 *
 * @param rate The rate with updated index, bandwidth, spatial-streams and guard
 * @returns u32 The standard data rate in bps
 */
u32 mmrc_calculate_theoretical_throughput(struct mmrc_rate rate);

/**
 * Gets best MRRC rate from given rate table.
 *
 * @param tb pointer to a mmrc table.
 *
 * @returns The MMRC rate with the best throughput.
 */

struct mmrc_rate mmrc_sta_get_best_rate(struct mmrc_table *tb);

#endif /* _MMRC_H_ */
