/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_SIGNAL_H
#define __ELL_SIGNAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_signal;

typedef void (*l_signal_notify_cb_t) (void *user_data);
typedef void (*l_signal_destroy_cb_t) (void *user_data);

struct l_signal *l_signal_create(uint32_t signo, l_signal_notify_cb_t callback,
				void *user_data, l_signal_destroy_cb_t destroy);
void l_signal_remove(struct l_signal *signal);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_SIGNAL_H */
