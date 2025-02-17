/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <string.h>
#include <stdint.h>

void _siphash24(uint8_t out[8], const uint8_t *in, size_t inlen,
						const uint8_t k[16]);
