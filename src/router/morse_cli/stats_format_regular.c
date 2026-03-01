/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "portable_endian.h"
#include "command.h"
#include "offchip_statistics.h"
#include "stats_format.h"
#include "utilities.h"

/** Regular formatting functions for morsectrl statistics */

static void stats_print_section_header(const char *key, int indent_level)
{
    mctrl_print("%*s%s\n",
                indent_level * INDENT_LEN, "", key);
}

static void stats_print_label(const char *key, int indent_level)
{
    mctrl_print("%*s%-*s:",
                indent_level * INDENT_LEN, "",
                LABEL_LEN - (indent_level * INDENT_LEN), key);
}

void stats_print_signed(const char *key, int64_t value, int indent_level)
{
    mctrl_print("%*s%-*s: %" PRId64 "\n",
                indent_level * INDENT_LEN, "",
                LABEL_LEN - (indent_level * INDENT_LEN), key,
                value);
}

void stats_print_unsigned(const char *key, uint64_t value, int indent_level)
{
    mctrl_print("%*s%-*s: %" PRIu64 "\n",
                indent_level * INDENT_LEN, "",
                LABEL_LEN - (indent_level * INDENT_LEN), key,
                value);
}

void stats_print_hex(const char *key, int64_t value, int indent_level)
{
    mctrl_print("%*s%-*s: %" PRIx64 "\n",
                indent_level * INDENT_LEN, "",
                LABEL_LEN - (indent_level * INDENT_LEN), key,
                value);
}

void stats_print_0hex(const char *key, int64_t value, int indent_level, uint32_t len)
{
    mctrl_print("%*s%-*s: 0x%0*" PRIx64 "\n",
                indent_level * INDENT_LEN, "",
                LABEL_LEN - (indent_level * INDENT_LEN), key,
                len * 2, value);
}

void stats_print_float(const char *key, float value, int indent_level)
{
    mctrl_print("%*s%-*s: %f\n",
                indent_level * INDENT_LEN, "",
                LABEL_LEN - (indent_level * INDENT_LEN), key,
                value);
}

static void print_dec(const char *key, const uint8_t *buf, uint32_t len)
{
    stats_print_signed(key, get_signed_value_as_int64(buf, len), 0);
}

static void print_udec(const char *key, const uint8_t *buf, uint32_t len)
{
    stats_print_unsigned(key, get_unsigned_value_as_uint64(buf, len), 0);
}


static void print_hex(const char *key, const uint8_t *buf, uint32_t len)
{
    stats_print_hex(key, get_unsigned_value_as_uint64(buf, len), 0);
}


static void print_0hex(const char *key, const uint8_t *buf, uint32_t len)
{
    stats_print_0hex(key, get_unsigned_value_as_uint64(buf, len), 0, len);
}

static void print_ampdu_aggregates(const char *key, const uint8_t *buf, uint32_t len)
{
    ampdu_count_t *count = (ampdu_count_t *)buf;
    stats_print_label(key, 0);
    for (int i = 0; i < MORSE_ARRAY_SIZE(count->count); i++)
    {
        mctrl_print(" %u", le32toh(count->count[i]));
    }
    mctrl_print("\n");
}

static void print_ampdu_bitmap(const char *key, const uint8_t *buf, uint32_t len)
{
    ampdu_bitmap_t *bitmap = (ampdu_bitmap_t *)buf;
    stats_print_label(key, 0);
    for (int i = 0; i < MORSE_ARRAY_SIZE(bitmap->bitmap); i++)
    {
        mctrl_print(" %u", le32toh(bitmap->bitmap[i]));
    }
    mctrl_print("\n");
}

static void print_txop(const char *key, const uint8_t *buf, uint32_t len)
{
    struct txop_statistics *txop_stats = (struct txop_statistics *)buf;
    uint32_t duration_avg = 0, packets_avg = 0;

    uint64_t duration = le64toh(txop_stats->duration);
    uint32_t count = le32toh(txop_stats->count);
    uint32_t pkts = le32toh(txop_stats->pkts);

    if (le32toh(txop_stats->count))
    {
        packets_avg = (uint32_t)(pkts / count);
        duration_avg = (uint32_t)(duration / count);
    }

    stats_print_section_header(key, 0);
    stats_print_unsigned("TXOP count", count, 1);
    stats_print_unsigned("Total TXOP time", duration, 1);
    stats_print_unsigned("Average TXOP time", duration_avg, 1);
    stats_print_unsigned("Total TXOP TX packets", pkts, 1);
    stats_print_unsigned("Average TXOP TX packets", packets_avg, 1);
}

static void print_pageset(const char *key, const uint8_t *buf, uint32_t len)
{
    struct pageset_stats *pageset = (struct pageset_stats *)buf;

    stats_print_section_header(key, 0);
    for (int i = 0; i < NUM_PAGESETS; i++)
    {
        mctrl_print("%*sPageset %d\n", INDENT_LEN, "", i);
        stats_print_unsigned("Allocated", le32toh(pageset->pages_allocated[i]), 2);
        stats_print_unsigned("Total", le32toh(pageset->pages_to_allocate[i]), 2);
    }
}

static void print_retries(const char *key, const uint8_t *buf, uint32_t len)
{
    struct retry_stats *retries = (struct retry_stats *)buf;
    stats_print_section_header(key, 0);
    mctrl_print("    Retry    Count    Avg Time\n");
    mctrl_print("    =====    =====    ========\n");

    for (int i = 0; i < APP_STATS_COUNT; i++)
    {
        uint32_t count = le32toh(retries->count[i]);
        mctrl_print("    %-8d %-8u %u\n", i, count,
            count ? (uint32_t)(le64toh(retries->sum[i]) / count) : 0);
    }
}

static void print_raw(const char *key, const uint8_t *buf, uint32_t len)
{
    raw_stats_t *raw_stats = (raw_stats_t *)buf;

    stats_print_section_header(key, 0);
    stats_print_section_header("RAW Assignments", 1);

    stats_print_label("Valid", 2);
    for (uint8_t i = 0; i < MORSE_ARRAY_SIZE(raw_stats->assignments); i++)
    {
        mctrl_print(" %d", le32toh(raw_stats->assignments[i]));
    }
    mctrl_print("\n");

    stats_print_unsigned("Truncated by TBTT",
                            le32toh(raw_stats->assignments_truncated_from_tbtt), 2);
    stats_print_unsigned("Invalid", le32toh(raw_stats->invalid_assignments), 2);
    stats_print_unsigned("Already past", le32toh(raw_stats->already_past_assignment), 2);

    stats_print_section_header("Delayed due to RAW", 1);
    stats_print_unsigned("From ACI queue", le32toh(raw_stats->aci_frames_delayed), 2);
    stats_print_unsigned("From BC/MC queue", le32toh(raw_stats->bc_mc_frames_delayed), 2);
    stats_print_unsigned("From absolute time queue", le32toh(raw_stats->abs_frames_delayed), 2);
    stats_print_unsigned("Frame crosses slot", le32toh(raw_stats->frame_crosses_slot_delayed), 2);
}

static void print_calibration(const char *key, const uint8_t *buf, uint32_t len)
{
    managed_calibration_stats_t *calib_stats = (managed_calibration_stats_t *)buf;
    stats_print_section_header(key, 0);

    stats_print_signed("Quiet calibration granted",
                        le32toh(calib_stats->quiet_calibration_granted), 1);
    stats_print_signed("Quiet calibration rejected",
                        le32toh(calib_stats->quiet_calibration_rejected), 1);
    stats_print_signed("Quiet calibration cancelled",
                        le32toh(calib_stats->quiet_calibration_cancelled), 1);
    stats_print_signed("Non-quiet calibration granted",
                       le32toh(calib_stats->non_quiet_calibration_granted), 1);
    stats_print_signed("Calibration complete", le32toh(calib_stats->calibration_complete), 1);
}

static void print_duty_cycle(const char *key, const uint8_t *buf, uint32_t len)
{
    duty_cycle_stats_t *duty_cycle_stats = (duty_cycle_stats_t *)buf;
    stats_print_section_header(key, 0);

    stats_print_label("Duty Cycle Target (%)", 1);
    mctrl_print(" %d.%02d\n",
                le32toh(duty_cycle_stats->target_duty_cycle) / 100,
                le32toh(duty_cycle_stats->target_duty_cycle) % 100);
    stats_print_unsigned("Duty Cycle TX on (usec)", le64toh(duty_cycle_stats->total_t_air), 1);
    stats_print_unsigned("Duty Cycle TX off (blocked) (usec)",
                        le64toh(duty_cycle_stats->total_t_off), 1);
    stats_print_unsigned("Duty Cycle max time off (usec)", le64toh(duty_cycle_stats->max_t_off), 1);
    stats_print_unsigned("Duty Cycle early frames", le32toh(duty_cycle_stats->num_early), 1);
}

static void print_mac_state(const char *key, const uint8_t *buf, uint32_t len)
{
    uint64_t mac_state = le64toh(*(__force __le64*) buf);
    stats_print_section_header(key, 0);
    stats_print_signed("RX state",
                       BMGET(mac_state, ENCODE_MAC_STATE_RX_STATE), 1);
    stats_print_signed("TX state",
                       BMGET(mac_state, ENCODE_MAC_STATE_TX_STATE), 1);
    stats_print_signed("Channel config",
                       BMGET(mac_state, ENCODE_MAC_STATE_CHANNEL_CONFIG), 1);
    stats_print_signed("Managed calibration state",
                       BMGET(mac_state, ENCODE_MAC_STATE_MGD_CALIB_STATE), 1);
    stats_print_signed("Powersave enabled",
                       BMGET(mac_state, ENCODE_MAC_STATE_PS_EN), 1);
    stats_print_signed("Dynamic powersave offload enabled",
                       BMGET(mac_state, ENCODE_MAC_STATE_DYN_PS_OFFLOAD_EN), 1);
    stats_print_signed("STA PS state",
                       BMGET(mac_state, ENCODE_MAC_STATE_STA_PS_STATE), 1);
    stats_print_signed("Waiting on dynamic powersave timeout",
                       BMGET(mac_state, ENCODE_MAC_STATE_WAITING_ON_DYN_PS), 1);
    stats_print_signed("TX blocked by host cmd",
                       BMGET(mac_state, ENCODE_MAC_STATE_TX_BLOCKED), 1);
    stats_print_signed("Waiting for medium sync",
                       BMGET(mac_state, ENCODE_MAC_STATE_WAITING_MED_SYNC), 1);
    stats_print_signed("Packets in QoS queues",
                       BMGET(mac_state, ENCODE_MAC_STATE_N_PKTS_IN_QUEUES), 1);
}

static void print_umac_latency_histogram(const char *key, const uint8_t *buf, uint32_t len)
{
    umac_latency_histogram_t *histogram = (umac_latency_histogram_t *)buf;

    stats_print_label(key, 0);
    for (size_t i = 0; i < MORSE_ARRAY_SIZE(histogram->buckets); i++)
    {
        mctrl_print(" %u", le32toh(histogram->buckets[i]));
    }
    mctrl_print("\n");
}

static void print_array(const char *key, const uint8_t *buf, uint32_t len)
{
    array_t *array = (array_t *)buf;
    stats_print_label(key, 0);
    for (int i = 0; i < le16toh(array->count); i++)
    {
        mctrl_print("%d ", le16toh(array->array[i]));
    }
    mctrl_print("\n");
}

static void print_default(const char *key, const uint8_t *buf, uint32_t len)
{
    /* Not implemented prior, use default hexdump in previous switch statement */
    mctrl_print("%*s: ", LABEL_LEN, key);
    hexdump(buf, len);
    mctrl_print("\n");
}

void hexdump(const uint8_t *buf, uint32_t len)
{
    for (int i = 0; i < len; i++)
    {
        mctrl_print("%02X ", buf[i]);
    }
}

/**
 * Array of function pointers indexed by the TLV format key
 */
static const struct format_table table = {
    .format_func = {
        [MORSE_STATS_FMT_DEC] = print_dec,
        [MORSE_STATS_FMT_U_DEC] = print_udec,
        [MORSE_STATS_FMT_HEX] = print_hex,
        [MORSE_STATS_FMT_0_HEX] = print_0hex,
        [MORSE_STATS_FMT_AMPDU_AGGREGATES] = print_ampdu_aggregates,
        [MORSE_STATS_FMT_AMPDU_BITMAP] = print_ampdu_bitmap,
        [MORSE_STATS_FMT_TXOP] =  print_txop,
        [MORSE_STATS_FMT_PAGESET] = print_pageset,
        [MORSE_STATS_FMT_RETRIES] = print_retries,
        [MORSE_STATS_FMT_RAW] = print_raw,
        [MORSE_STATS_FMT_CALIBRATION] = print_calibration,
        [MORSE_STATS_FMT_DUTY_CYCLE] = print_duty_cycle,
        [MORSE_STATS_FMT_MAC_STATE] = print_mac_state,
        [MORSE_STATS_FMT_UMAC_LATENCY_HISTOGRAM] = print_umac_latency_histogram,
        /* Add new function pointers here */
        /* [MORSE_STATS_NEW_TLV_FORMAT] = print_new_format */
        [MORSE_STATS_FMT_ARRAY] = print_array,

        [MORSE_STATS_FMT_LAST] = print_default,
    }
};

const struct format_table* stats_format_regular_get_formatter_table(void)
{
    return &table;
}
