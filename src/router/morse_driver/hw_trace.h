#ifndef _MORSE_HW_TRACE_H_
#define _MORSE_HW_TRACE_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "morse.h"

struct hw_trace {
	int pin;
	bool used;
};

extern struct hw_trace *hwt_tx_in;
extern struct hw_trace *hwt_tx_out;
extern struct hw_trace *hwt_pages;
extern struct hw_trace *hwt_page_return;

struct hw_trace *morse_hw_trace_register(void);

void morse_hw_trace_unregister(struct hw_trace *hwt);

void morse_hw_trace_set(struct hw_trace *hwt);

void morse_hw_trace_clear(struct hw_trace *hwt);

int morse_hw_trace_init(void);
void morse_hw_trace_deinit(void);

#endif /* !_MORSE_HW_TRACE_H_ */
