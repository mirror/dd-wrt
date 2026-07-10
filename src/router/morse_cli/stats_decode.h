/*
 * Copyright 2026 Morse Micro
 *
 * This file contains prototypes for functions to load and use statistics definitions and data
 */

#pragma once

#include "stats_format.h"

/* Load statistics definitions from an external firmware file.
 *
 * The definitions are used to decode statistics data into human readable form.
 *
 * The firmware file must match the firmware that originates the statistics data, otherwise
 * undefined behaviour may result if the statistics are incorrectly parsed.
 *
 * The definitions occupy heap memory allocated to mors->stats. It is up to the caller to
 * free these using free_offchip_statisics_definitions()
 */
int load_offchip_statistics_definitions(struct morsectrl *mors, const char *filename);

/* Free statistics definitions allocated by load_offchip_statistics_definitions() */
void free_offchip_statistics_definitions(struct morsectrl *mors);

/* Load a file of hex strings into a memory buffer. Hopefully these hexstrings represent a
 * statistics dump from elsewhere but there is no enforcement of this.
 *
 * There is a basic tolerance of:
 * - whitespace, including line endings
 * - comments starting with '#' which will discard everything else until the end of the line
 * - other characters, but if some of them match [A-Za-z0-9] then things start to get messy
 *
 * The overall intent is to allow hex strings to be thrown together from various sources including
 * multiple cores, and to permit comments that record from where the data originated
 *
 * The firmware file loaded in load_offchip_statistics_definitions() must match the firmware that
 * originated this statistics data, otherwise undefined behaviour may result as the statistics may
 * be incorrectly parsed.
 */
int load_offchip_statistics_data(const char *statsdump_filename,
                                 uint8_t **statsbin_buffer, size_t *statsbin_length);


/* Decode a memory buffer containing a binary statistics dump
 *
 * There are no general sanity checks that the data is legitimate, only the detection of
 * malformed TLVs.
 */
int statistics_data_decode(struct morsectrl *mors, enum format_type format_val,
                           const uint8_t *stats, size_t stats_len);

/* Show the statistics types definitions */
void statistics_types_dump(struct morsectrl *mors);

/* Provide a help string for the statistics filter
 *
 * Returns a pointer to the help string, which may vary by platform
 */
const char *stats_filter_help(void);

/* Initialise the statistics filter with the given string
 *
 * Returns 0 if filter successfully initialised, non-zero otherwise
 */
int stats_filter_init(const char *filter_string);

/* Release resources allocated to the statistics filter */
void stats_filter_deinit(void);

/* Test for whether filter is initialised with a filter string
 *
 * Returns True if filter is initialised, False if not
 */
bool stats_filter_is_set(void);

/* Compare a filter key to the filter string
 *
 * Returns 0 for a match or non-zero otherwise
 */
int stats_filter_is_match(const char *key);
