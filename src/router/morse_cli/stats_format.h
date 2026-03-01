/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "offchip_statistics.h"

/**
 * Bit field definitions for mac_state statistic
 **/
#define ENCODE_MAC_STATE_RX_STATE           (0x000000000000000F)
#define ENCODE_MAC_STATE_TX_STATE           (0x00000000000000F0)
#define ENCODE_MAC_STATE_CHANNEL_CONFIG     (0x0000000000000F00)
#define ENCODE_MAC_STATE_MGD_CALIB_STATE    (0x0000000000007000)
#define ENCODE_MAC_STATE_STA_PS_STATE       (0x0000000000038000)
#define ENCODE_MAC_STATE_TX_BLOCKED         (0x0000000000080000)
#define ENCODE_MAC_STATE_WAITING_MED_SYNC   (0x0000000000100000)
#define ENCODE_MAC_STATE_PS_EN              (0x0000000000200000)
#define ENCODE_MAC_STATE_DYN_PS_OFFLOAD_EN  (0x0000000000400000)
#define ENCODE_MAC_STATE_WAITING_ON_DYN_PS  (0x0000000000800000)
#define ENCODE_MAC_STATE_N_PKTS_IN_QUEUES   (0x00000000FF000000)

/* The maximum number of bits in the NDP bitmap */
#define DOT11AH_NDP_MAX_BITMAP_BIT          (16)

#define LABEL_LEN   48
#define INDENT_LEN  4

/**
 * A counter of the number of times an MPDU is received successfully for each position in the AMPDU
 **/
typedef struct __attribute__((packed)) {
    __le32 bitmap[DOT11AH_NDP_MAX_BITMAP_BIT];
} ampdu_bitmap_t;

/**
 * A counter of the number of aggregates
 */
typedef struct __attribute__((packed)) {
    /* This count starts at zero and goes to 16, so it's 17 elements */
    __le32 count[17];
} ampdu_count_t;

#define MAC_MAX_RETRY_COUNT 10
 /* Account for 0 retry, for more than
  * MAC_MAX_RETRY_COUNT, and fail */
#define APP_STATS_COUNT (MAC_MAX_RETRY_COUNT + 3)

struct __attribute__((packed)) retry_stats {
    __le64 start, stop;
    __le64 sum[APP_STATS_COUNT];
    __le32 count[APP_STATS_COUNT];
};


#define NUM_PAGESETS 2
struct __attribute__((packed)) pageset_stats {
    __le32 pages_allocated[NUM_PAGESETS];
    __le32 pages_to_allocate[NUM_PAGESETS];
};



/** TX OP statistics, Note: Changes made to this struct must be reflected in statistics.py parser */
struct __attribute__((packed)) txop_statistics {
    __le64    duration;
    __le32    count;
    __le32    pkts;
    __le32    max_pkts_in_txop;
    __le32    lost_beacons;
    uint8_t     beacon_lost;
};


/**
 * RAW (Restricted Access Window) statistics
 */
typedef struct PACKED {
    /** Currently only supporting up to 8 assignments */
    __le32 assignments[8];
    /** A counter of assignments that get truncated due to the next tbtt */
    __le32 assignments_truncated_from_tbtt;
    /** Number of invalid assignments (/unsupported) observed */
    __le32 invalid_assignments;
    /** Number of assignments that are valid but system time has already passed */
    __le32 already_past_assignment;
    /** ACI delayed frames due to RAW */
    __le32 aci_frames_delayed;
    /** Broadcast / Multicast delayed frames due to RAW */
    __le32 bc_mc_frames_delayed;
    /** Absolute time frames delayed due to RAW */
    __le32 abs_frames_delayed;
    /** Frames that could've been sent in the RAW but were too long for slot */
    __le32 frame_crosses_slot_delayed;
} raw_stats_t;

typedef struct {
    /** A quiet calibration was granted */
    __le32 quiet_calibration_granted;
    /** A non-quiet calibration was granted */
    __le32 non_quiet_calibration_granted;
    /** A quiet calibration was in progress, but then cancelled */
    __le32 quiet_calibration_cancelled;
    /** A quiet calibration was rejected */
    __le32 quiet_calibration_rejected;
    /** A quiet/non-quiet calibration completed */
    __le32 calibration_complete;
} managed_calibration_stats_t;

typedef struct
{
    /** Total transmitter 'on air' (Tair) time in us */
    __le64 total_t_air;
    /** Total transmitter 'blocked' (Toff) time in us */
    __le64 total_t_off;
    /** Configured duty cycle restriction (100th of a percent). 10000 is no restriction. */
    __le32 target_duty_cycle;
    /** Number of packets ignoring duty cycle restrictions. */
    __le32 num_early;
    /** Maximum time the t_off timer is started for */
    __le64 max_t_off;
} duty_cycle_stats_t;

/** Histogram of how long each packet spent being handled inside the umac code. */
typedef struct PACKED {
    __le32 buckets[9];
} umac_latency_histogram_t;

struct PACKED stats_response
{
    /** The contents of the response */
    uint8_t stats[2048];
};

/**
 * A int32_t generic array
 **/
typedef struct __attribute__((packed)) {
    __le16 count;
    __le16 array[];
} array_t;

/** Enum type for printing format  */
enum format_type
{
    FORMAT_REGULAR,
    FORMAT_JSON,
    FORMAT_JSON_PPRINT,
    /* Add additional formats here  */
};


typedef void (*format_func_t)(const char *key, const uint8_t *buf, uint32_t len);

struct format_table {
    format_func_t format_func[MORSE_STATS_FMT_LAST + 1];
};

/** Regular format function  */
const struct format_table* stats_format_regular_get_formatter_table();
void hexdump(const uint8_t *buf, uint32_t len);

/** JSON format functions  */
const struct format_table* stats_format_json_get_formatter_table();
void stats_format_json_init();
void stats_format_json_set_pprint(bool pprint);

/** Generic formatted print funtions */
void stats_print_signed(const char *key, int64_t value, int indent_level);
void stats_print_unsigned(const char *key, uint64_t value, int indent_level);
void stats_print_float(const char *key, float value, int indent_level);
void stats_print_hex(const char *key, int64_t value, int indent_level);
