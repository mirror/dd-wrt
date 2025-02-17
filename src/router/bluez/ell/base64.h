/*
 * Embedded Linux library
 * Copyright (C) 2015  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_BASE64_H
#define __ELL_BASE64_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *l_base64_decode(const char *in, size_t in_len, size_t *n_written);

char *l_base64_encode(const uint8_t *in, size_t in_len, int columns);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_BASE64_H */
