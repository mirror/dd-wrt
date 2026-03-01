/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 *
 */

#pragma once

#include <stddef.h>
#include <stdio.h>
#include "offchip_statistics.h"
#include "transport/transport.h"

int morse_stats_load(struct statistics_offchip_data **stats_handle,
                     size_t *n_rec,
                     const uint8_t *data);

int load_elf(struct morsectrl *mors, int argc, char *argv[]);
