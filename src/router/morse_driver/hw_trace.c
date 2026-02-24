/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/module.h>
#include <linux/gpio.h>
#include "hw_trace.h"
#include "debug.h"

struct hw_trace morse_traces[] = {
	{ 2, 0 },		/* GPIO 2 */
	{ 16, 0 },		/* GPIO 16 */
	{ 21, 0 },		/* GPIO 21 */
	{ 6, 0 },		/* GPIO 6 */
};

struct hw_trace *hwt_tx_in;
struct hw_trace *hwt_tx_out;
struct hw_trace *hwt_pages;
struct hw_trace *hwt_page_return;

void morse_hw_trace_set(struct hw_trace *hwt)
{
	if (!hwt)
		return;

	gpio_set_value(hwt->pin, 1);
}

void morse_hw_trace_clear(struct hw_trace *hwt)
{
	if (!hwt)
		return;

	gpio_set_value(hwt->pin, 0);
}

struct hw_trace *morse_hw_trace_register(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(morse_traces); i++) {
		struct hw_trace *hwt = &morse_traces[i];

		if (!hwt->used) {
			int ret;

			ret = gpio_request(hwt->pin, NULL);
			if (ret) {
				MORSE_PR_ERR(FEATURE_ID_DEFAULT, "Failed to acquire trace gpio.\n");
				continue;
			}
			gpio_direction_output(hwt->pin, 0);
			hwt->used = 1;
			return hwt;
		}
	}

	return NULL;
}

void morse_hw_trace_unregister(struct hw_trace *hwt)
{
	if (!hwt)
		return;

	hwt->used = 0;
	gpio_free(hwt->pin);
}

int morse_hw_trace_init(void)
{
	hwt_tx_in = morse_hw_trace_register();
	if (hwt_tx_in)
		pr_info("hwt_tx_in set to gpio %d\n", hwt_tx_in->pin);
	else
		pr_info("hwt_tx_in was not set\n");

	hwt_pages = morse_hw_trace_register();
	if (hwt_pages)
		pr_info("hwt_pages set to gpio %d\n", hwt_pages->pin);
	else
		pr_info("hwt_pages was not set\n");

	hwt_tx_out = morse_hw_trace_register();
	if (hwt_tx_out)
		pr_info("hwt_tx_out set to gpio %d\n", hwt_tx_out->pin);
	else
		pr_info("hwt_tx_out was not set\n");

	hwt_page_return = morse_hw_trace_register();
	if (hwt_page_return)
		pr_info("hwt_page_return set to gpio %d\n", hwt_page_return->pin);
	else
		pr_info("hwt_page_return was not set\n");

	return 0;
}

void morse_hw_trace_deinit(void)
{
	morse_hw_trace_unregister(hwt_tx_in);
	morse_hw_trace_unregister(hwt_tx_out);
	morse_hw_trace_unregister(hwt_pages);
	morse_hw_trace_unregister(hwt_page_return);
}
