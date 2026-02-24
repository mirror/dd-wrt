/*
 * Copyright 2025 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _MORSE_LED_H_
#define _MORSE_LED_H_

#include <linux/leds.h>
#include <linux/workqueue.h>

/* Forward declaration */
struct morse;

/* Hardcoded value in initial implememtation */
#define NUM_LEDS 3
#define LED_G 4
#define LED_R 5
#define LED_PWR 6

/**
 * LED behaviour configuration
 *	- DISABLED does not initialise the LED
 *	- HOST is controlled directly by the driver and indicates that the chip has been
 *		correctly initialised
 * The remaining five options register the LED with the kernel subsystem LED callbacks
 *	- TX and RX indicate there is network traffic
 *	- TPT flashes based on traffic with faster flashing for higher throughput
 *	- ASSOC indicates that the station has associated (STA only)
 *	- RADIO indicates that the RF is up/down
 *	- USER_DEF registers the LED with mac80211 but does not set a default behaviour
 * All of the mac80211-registered configurations can be configured and controlled in
 * userspace in /sys/class/leds/<led_name>
 */
enum led_mode {
	MORSE_LED_DISABLED,
	MORSE_LED_HOST,
	MORSE_LED_TPT,
	MORSE_LED_TX,
	MORSE_LED_RX,
	MORSE_LED_ASSOC,
	MORSE_LED_RADIO,
	MORSE_LED_USER_DEF,
};

struct morse_led {
	int pin_num;
	int active;
	struct led_classdev led_cdev;
	enum led_mode mode;
	struct work_struct work;
};

struct morse_led_group {
	bool enable_led_support;
	int num_leds;
	struct morse_led leds[NUM_LEDS];
};

/**
 * Initialise LEDs and register them with kernel subsystem callbacks
 * @mors: Global Morse structure
 */
void morse_led_init(struct morse *mors);

/**
 * Deinitialise LEDs and deregister them from  kernel subsystem
 * @mors: Global Morse structure
 */
void morse_led_exit(struct morse *mors);

#endif
