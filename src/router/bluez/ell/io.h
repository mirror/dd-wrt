/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_IO_H
#define __ELL_IO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct l_io;

typedef void (*l_io_debug_cb_t) (const char *str, void *user_data);

typedef bool (*l_io_read_cb_t) (struct l_io *io, void *user_data);
typedef bool (*l_io_write_cb_t) (struct l_io *io, void *user_data);
typedef void (*l_io_disconnect_cb_t) (struct l_io *io, void *user_data);
typedef void (*l_io_destroy_cb_t) (void *user_data);

struct l_io *l_io_new(int fd);
void l_io_destroy(struct l_io *io);

int l_io_get_fd(struct l_io *io);
bool l_io_set_close_on_destroy(struct l_io *io, bool do_close);

bool l_io_set_read_handler(struct l_io *io, l_io_read_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);
bool l_io_set_write_handler(struct l_io *io, l_io_write_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);
bool l_io_set_disconnect_handler(struct l_io *io,
				l_io_disconnect_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);

bool l_io_set_debug(struct l_io *io, l_io_debug_cb_t callback,
				void *user_data, l_io_destroy_cb_t destroy);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_IO_H */
