/*
 * Embedded Linux library
 * Copyright (C) 2015  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_RANDOM_H
#define __ELL_RANDOM_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool l_getrandom(void *buf, size_t len);
bool l_getrandom_is_supported(void);

uint32_t l_getrandom_uint32(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_RANDOM_H */
