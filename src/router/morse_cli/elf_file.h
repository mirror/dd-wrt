/*
 * Copyright 2022-2026 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 *
 */

#pragma once

#include "offchip_statistics.h"

/*
 * Load the offchip statistics from an ELF data structure.
 *
 * An array of n_rec struct statistics_offchip_data elements is allocated in the stats_handle.
 * It is the caller's responsibility to free the array.
 */
int morse_stats_load(struct statistics_offchip_data **stats_handle,
                     size_t *n_rec,
                     const uint8_t *data);

int load_elf(struct morsectrl *mors, int argc, char *argv[]);
