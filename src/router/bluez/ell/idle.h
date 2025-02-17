/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_IDLE_H
#define __ELL_IDLE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_idle;

typedef void (*l_idle_notify_cb_t) (struct l_idle *idle, void *user_data);
typedef void (*l_idle_oneshot_cb_t) (void *user_data);
typedef void (*l_idle_destroy_cb_t) (void *user_data);

struct l_idle *l_idle_create(l_idle_notify_cb_t callback,
			void *user_data, l_idle_destroy_cb_t destroy);
void l_idle_remove(struct l_idle *idle);

bool l_idle_oneshot(l_idle_oneshot_cb_t callback, void *user_data,
			l_idle_destroy_cb_t destroy);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_IDLE_H */
