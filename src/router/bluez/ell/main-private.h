/*
 * Embedded Linux library
 * Copyright (C) 2021  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

typedef void (*watch_event_cb_t) (int fd, uint32_t events, void *user_data);
typedef void (*watch_destroy_cb_t) (void *user_data);

typedef void (*idle_event_cb_t) (void *user_data);
typedef void (*idle_destroy_cb_t) (void *user_data);

int watch_add(int fd, uint32_t events, watch_event_cb_t callback,
				void *user_data, watch_destroy_cb_t destroy);
int watch_modify(int fd, uint32_t events, bool force);
int watch_remove(int fd, bool epoll_del);
int watch_clear(int fd);

#define IDLE_FLAG_NO_WARN_DANGLING 0x10000000
int idle_add(idle_event_cb_t callback, void *user_data, uint32_t flags,
		idle_destroy_cb_t destroy);
void idle_remove(int id);
