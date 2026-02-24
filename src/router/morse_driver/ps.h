#ifndef _MORSE_PS_H_
#define _MORSE_PS_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "morse.h"

/** This should be nominally <= the dynamic ps timeout */
#define NETWORK_BUS_TIMEOUT_MS (90)
#define UAPSD_NETWORK_BUS_TIMEOUT_MS (5)
/** The default period of time to wait to re-evaluate powersave */
#define DEFAULT_BUS_TIMEOUT_MS (5)

static inline int morse_network_bus_timeout(struct morse *mors)
{
	return mors->uapsd_per_ac ? UAPSD_NETWORK_BUS_TIMEOUT_MS : NETWORK_BUS_TIMEOUT_MS;
}

/**
 * morse_ps_disable() - Raise the wake line, forcing the chip to wake up from powersave.
 * @mors: Morse chip instance
 *
 * Uses a reference counting mechanism for reentrancy.
 * Each call to morse_ps_disable() should be paired with a call to morse_ps_enable()
 * allowing the chip to go back to sleep when the operation is finished.
 */
int morse_ps_disable(struct morse *mors);

/**
 * morse_ps_enable() - Release the wake line, allowing the chip to go to sleep.
 * @mors: Morse chip instance
 */
int morse_ps_enable(struct morse *mors);

/**
 * Call this function when there is activity on the bus that should
 * delay the driver in disabling the bus.
 *
 * @mors: Morse chip instance
 * @timeout_ms: The timeout from now to add (ms)
 */
void morse_ps_bus_activity(struct morse *mors, int timeout_ms);

int morse_ps_init(struct morse *mors, bool enable, bool enable_dynamic_ps);

void morse_ps_finish(struct morse *mors);

#endif /* !_MORSE_PS_H_ */
