/*
 * Embedded Linux library
 * Copyright (C) 2011-2014  Intel Corporation
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef __ELL_MAIN_H
#define __ELL_MAIN_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool l_main_init(void);
int l_main_prepare(void);
void l_main_iterate(int timeout);
int l_main_run(void);
bool l_main_exit(void);

bool l_main_quit(void);

typedef void (*l_main_signal_cb_t) (uint32_t signo, void *user_data);

int l_main_run_with_signal(l_main_signal_cb_t callback, void *user_data);

int l_main_get_epoll_fd(void);

#ifdef __cplusplus
}
#endif

#endif /* __ELL_MAIN_H */
