/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_TIMEOUT_H
#define __ELL_TIMEOUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_timeout;

typedef void (*l_timeout_notify_cb_t) (struct l_timeout *timeout,
						void *user_data);
typedef void (*l_timeout_destroy_cb_t) (void *user_data);

struct l_timeout *l_timeout_create(unsigned int seconds,
			l_timeout_notify_cb_t callback,
			void *user_data, l_timeout_destroy_cb_t destroy);
struct l_timeout *l_timeout_create_ms(uint64_t milliseconds,
			l_timeout_notify_cb_t callback,
			void *user_data, l_timeout_destroy_cb_t destroy);
void l_timeout_modify(struct l_timeout *timeout,
				unsigned int seconds);
void l_timeout_modify_ms(struct l_timeout *timeout,
				uint64_t milliseconds);
void l_timeout_remove(struct l_timeout *timeout);
void l_timeout_set_callback(struct l_timeout *timeout,
				l_timeout_notify_cb_t callback, void *user_data,
				l_timeout_destroy_cb_t destroy);
bool l_timeout_remaining(struct l_timeout *timeout,
				uint64_t *remaining);
#ifdef __cplusplus
}
#endif

#endif /* __ELL_TIMEOUT_H */
