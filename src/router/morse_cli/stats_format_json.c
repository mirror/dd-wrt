/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdarg.h>

#include "portable_endian.h"
#include "command.h"
#include "offchip_statistics.h"
#include "stats_format.h"
#include "utilities.h"

#define SPACES_PER_INDENT 4
#define INDENT_FIRST_LEVEL 1

static int indent_level;
static bool pretty;

/** Print wrapper function to prepend additional indentation*/
static void printf_indent(char* format, ...)
{
    if (pretty)
    {
        mctrl_print("%*s", indent_level * SPACES_PER_INDENT, "");
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
};


/** JSON formatting functions for morsectrl statistics */
static void print_dec(const char *key, const uint8_t *buf, uint32_t len)
{
    int64_t n = get_signed_value_as_int64(buf, len);
    printf_indent("\"%s\": %lld", key, n);
}


static void print_udec(const char *key, const uint8_t *buf, uint32_t len)
{
    uint64_t n = get_unsigned_value_as_uint64(buf, len);
    printf_indent("\"%s\": %llu", key, n);
}


static void print_ampdu_aggregates(const char *key, const uint8_t *buf, uint32_t len)
{
    ampdu_count_t *count = (ampdu_count_t *)buf;
    printf_indent("\"%s\": ", key);
    mctrl_print("\"");
    for (int i = 0; i < MORSE_ARRAY_SIZE(count->count); i++)
    {
        mctrl_print("%d ", count->count[i]);
    }
    mctrl_print("\"");
}


static void print_ampdu_bitmap(const char *key, const uint8_t *buf, uint32_t len)
{
    ampdu_bitmap_t *bitmap = (ampdu_bitmap_t *)buf;
    printf_indent("\"%s\": ", key);
    mctrl_print("\"");
    for (int i = 0; i < MORSE_ARRAY_SIZE(bitmap->bitmap); i++)
    {
        mctrl_print("%u ", bitmap->bitmap[i]);
    }
    mctrl_print("\"");
}


static void print_txop(const char *key, const uint8_t *buf, uint32_t len)
{
    struct txop_statistics *txop_stats = (struct txop_statistics *)buf;
    uint32_t duration_avg = 0, packets_avg = 0;

    uint64_t duration = le64toh(txop_stats->duration);
    uint32_t count = le32toh(txop_stats->count);
    uint32_t pkts = le32toh(txop_stats->pkts);

    if (txop_stats->count)
    {
        packets_avg = (uint32_t)(pkts / count);
        duration_avg = (uint32_t)(duration / count);
    }
    char* terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    printf_indent("\"TXOP count\": %u,%s", count, terminator);
    printf_indent("\"Total TXOP time\": %llu,%s", duration, terminator);
    printf_indent("\"Average TXOP time\": %u,%s", duration_avg, terminator);
    printf_indent("\"Total TXOP Tx packets\": %u,%s", pkts, terminator);
    printf_indent("\"Average TXOP Tx packets\": %u%s", packets_avg, terminator);

    mctrl_print("%s", terminator);
    indent_level--;
    printf_indent("}");
}


static void print_pageset(const char *key, const uint8_t *buf, uint32_t len)
{
    struct pageset_stats *pageset = (struct pageset_stats *)buf;
    char* terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("[%s", terminator);
    indent_level++;

    for (int i = 0; i < NUM_PAGESETS; i++)
    {
        if (i) mctrl_print(",%s", terminator);

        printf_indent("{%s", terminator);
        indent_level++;

        printf_indent("\"Pageset\": %d,%s", i, terminator);
        printf_indent("\"Allocated\": %d,%s", le32toh(pageset->pages_allocated[i]), terminator);
        printf_indent("\"Total\": %d%s", le32toh(pageset->pages_to_allocate[i]), terminator);

        indent_level--;
        printf_indent("}");
    }

    indent_level--;
    mctrl_print("%s", terminator);
    printf_indent("]");
}


static void print_retries(const char *key, const uint8_t *buf, uint32_t len)
{
    struct retry_stats *retries = (struct retry_stats *)buf;
    char *terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("[%s", terminator);
    indent_level++;

    for (int i = 0; i < APP_STATS_COUNT; i++)
    {
        if (i) mctrl_print(",%s", terminator);
        printf_indent("{%s", terminator);
        indent_level++;
        uint32_t count = le32toh(retries->count[i]);
        uint32_t avg_time = count ?
            (uint32_t)(le64toh(retries->sum[i])/count) : 0;

        printf_indent("\"Retry\": %d,%s", i, terminator);
        printf_indent("\"Count\": %lu,%s", count, terminator);
        printf_indent("\"Avg Time\": %lu%s", avg_time, terminator);
        indent_level--;
        printf_indent("}");
    }
    indent_level--;
    mctrl_print("%s", terminator);
    printf_indent("]");
}


static void print_raw(const char *key, const uint8_t *buf, uint32_t len)
{
    raw_stats_t *raw_stats = (raw_stats_t *)buf;
    char* terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    printf_indent("\"RAW Assignments\": %s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    printf_indent("\"Valid\": \"");

    for (uint8_t i = 0; i < MORSE_ARRAY_SIZE(raw_stats->assignments); i++)
    {
        mctrl_print(" %d", le32toh(raw_stats->assignments[i]));
    }
    mctrl_print("\",%s", terminator);

    printf_indent("\"Truncated by TBTT\": %u,%s",
        le32toh(raw_stats->assignments_truncated_from_tbtt), terminator);
    printf_indent("\"Invalid\": %u,%s", le32toh(raw_stats->invalid_assignments), terminator);
    printf_indent("\"Already past\": %u%s", le32toh(raw_stats->already_past_assignment),
        terminator);

    indent_level--;
    printf_indent("},%s", terminator);
    printf_indent("\"Delayed due to RAW\": %s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    printf_indent("\"From ACI queue\": %u,%s", le32toh(raw_stats->aci_frames_delayed), terminator);
    printf_indent("\"From BC/MC queue\": %u,%s",
        le32toh(raw_stats->bc_mc_frames_delayed), terminator);
    printf_indent("\"From absolute time queue\": %u,%s",
        raw_stats->abs_frames_delayed, terminator);
    printf_indent("\"Frame crosses slot\": %u%s",
        raw_stats->frame_crosses_slot_delayed, terminator);

    indent_level--;
    printf_indent("}%s", terminator);

    indent_level--;
    printf_indent("}");
}


static void print_calibration(const char *key, const uint8_t *buf, uint32_t len)
{
    managed_calibration_stats_t *calib_stats = (managed_calibration_stats_t *)buf;
    char* terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    printf_indent("\"Managed calibration\": %s", terminator);
    printf_indent("{%s", terminator);

    printf_indent("\"Quiet calibration granted\": %u,%s",
            le32toh(calib_stats->quiet_calibration_granted), terminator);
    printf_indent("\"Quiet calibration rejected\": %u,%s",
            le32toh(calib_stats->quiet_calibration_rejected), terminator);
    printf_indent("\"Quiet calibration cancelled\": %u,%s",
            le32toh(calib_stats->quiet_calibration_cancelled), terminator);
    printf_indent("\"Non-quiet calibration granted\": %u,%s",
            le32toh(calib_stats->non_quiet_calibration_granted), terminator);
    printf_indent("\"Calibration complete\": %u%s", calib_stats->calibration_complete,
            terminator);

    indent_level--;
    printf_indent("}%s", terminator);
    indent_level--;
    printf_indent("}");
}


static void print_duty_cycle(const char *key, const uint8_t *buf, uint32_t len)
{
    duty_cycle_stats_t *duty_cycle_stats = (duty_cycle_stats_t*)buf;
    char* terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    /* Duty Cycle Body*/
    printf_indent("\"Duty Cycle Target (%c)\": %d.%02d,%s",
                  '%',
                  le32toh(duty_cycle_stats->target_duty_cycle) / 100,
                  le32toh(duty_cycle_stats->target_duty_cycle) % 100,
                  terminator);
    printf_indent("\"Duty Cycle TX on (usec)\": %llu,%s",
                  le64toh(duty_cycle_stats->total_t_air),
                  terminator);
    printf_indent("\"Duty Cycle TX off (blocked) (usec)\": %llu,%s",
                  le64toh(duty_cycle_stats->total_t_off),
                  terminator);
    printf_indent("\"Duty Cycle max time off (usec)\": %llu,%s",
                  le64toh(duty_cycle_stats->max_t_off),
                  terminator);
    printf_indent("\"Duty Cycle early frames\": %u%s",
                  le32toh(duty_cycle_stats->num_early),
                  terminator);

    indent_level--;
    printf_indent("}");
}


static void print_mac_state(const char *key, const uint8_t *buf, uint32_t len)
{
    uint64_t mac_state = le64toh(*(__force __le64*) buf);
    char* terminator = pretty ? "\n" : "";

    printf_indent("\"%s\": ", key);
    mctrl_print("%s", terminator);
    printf_indent("{%s", terminator);
    indent_level++;

    printf_indent("\"%s\": %lld,%s", "RX state",
        BMGET(mac_state, ENCODE_MAC_STATE_RX_STATE), terminator);
    printf_indent("\"%s\": %lld,%s", "TX state",
        BMGET(mac_state, ENCODE_MAC_STATE_TX_STATE), terminator);
    printf_indent("\"%s\": %lld,%s", "Channel config",
        BMGET(mac_state, ENCODE_MAC_STATE_CHANNEL_CONFIG), terminator);
    printf_indent("\"%s\": %lld,%s", "Managed calibration state",
        BMGET(mac_state, ENCODE_MAC_STATE_MGD_CALIB_STATE), terminator);
    printf_indent("\"%s\": %lld,%s", "Powersave enabled",
        BMGET(mac_state, ENCODE_MAC_STATE_PS_EN), terminator);
    printf_indent("\"%s\": %lld,%s", "Dynamic powersave offload enabled",
        BMGET(mac_state, ENCODE_MAC_STATE_DYN_PS_OFFLOAD_EN), terminator);
    printf_indent("\"%s\": %lld,%s", "STA PS state",
        BMGET(mac_state, ENCODE_MAC_STATE_STA_PS_STATE), terminator);
    printf_indent("\"%s\": %lld,%s", "Waiting on dynamic powersave timeout",
        BMGET(mac_state, ENCODE_MAC_STATE_WAITING_ON_DYN_PS), terminator);
    printf_indent("\"%s\": %lld,%s", "TX blocked by host cmd",
        BMGET(mac_state, ENCODE_MAC_STATE_TX_BLOCKED), terminator);
    printf_indent("\"%s\": %lld,%s", "Waiting for medium sync",
        BMGET(mac_state, ENCODE_MAC_STATE_WAITING_MED_SYNC), terminator);
    printf_indent("\"%s\": %lld%s", "Packets in QoS queues",
        BMGET(mac_state, ENCODE_MAC_STATE_N_PKTS_IN_QUEUES), terminator);

    indent_level--;
    printf_indent("}");
}

static void print_umac_latency_histogram(const char *key, const uint8_t *buf, uint32_t len)
{
    umac_latency_histogram_t *histogram = (umac_latency_histogram_t *)buf;

    printf_indent("\"%s\": ", key);
    mctrl_print("\"");
    for (size_t i = 0; i < MORSE_ARRAY_SIZE(histogram->buckets); i++)
    {
        mctrl_print("%d ", le32toh(histogram->buckets[i]));
    }
    mctrl_print("\"");
}

static void print_array(const char *key, const uint8_t *buf, uint32_t len)
{
    array_t *array = (array_t *)buf;
    printf_indent("\"%s\": ", key);
    mctrl_print("\"");
    for (int i = 0; i < le16toh(array->count); i++)
    {
        mctrl_print("%d ", le16toh(array->array[i]));
    }
    mctrl_print("\"");
}

static void print_default(const char *key, const uint8_t *buf, uint32_t len)
{
    printf_indent("\"%s\": ", key);
    mctrl_print("\"");
    for (int i = 0; i < len; i++)
    {
        mctrl_print("%02X ", buf[i]);
    }
    mctrl_print("\"");
}


/**
 * Array of function pointers indexed by the TLV format key.
 *
 * JSON doesn't support hex numbers, so
 * so use unsigned decimal instead.
 */
static const struct format_table table = {
    .format_func = {
        [MORSE_STATS_FMT_DEC] = print_dec,
        [MORSE_STATS_FMT_U_DEC] = print_udec,
        [MORSE_STATS_FMT_HEX] = print_udec,
        [MORSE_STATS_FMT_0_HEX] = print_udec,
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


const struct format_table* stats_format_json_get_formatter_table(void)
{
    return &table;
}


void stats_format_json_init(void)
{
    indent_level = INDENT_FIRST_LEVEL;

    static bool first = true;
    if (first)
    {
        first = false;
    }
    else
    {
        pretty ? mctrl_print(",\n") : mctrl_print(",");
    }
}


void stats_format_json_set_pprint(bool pprint)
{
    pretty = pprint;
}
