/*
 * Copyright 2025 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "morse.h"
#include "debug.h"
#include "bus.h"
#include "led.h"
#include "mac.h"

#include <linux/leds.h>
#include <linux/workqueue.h>

/* This is based on the iwlwifi.c blink rates. */
static const struct ieee80211_tpt_blink morse_blink[] = {
	{ .throughput = 0, .blink_time = 334 },
	{ .throughput = 512 - 1, .blink_time = 260 },
	{ .throughput = 1024 - 1, .blink_time = 220 },
	{ .throughput = 2 * 1024 - 1, .blink_time = 190 },
	{ .throughput = 4 * 1024 - 1, .blink_time = 170 },
	{ .throughput = 7 * 1024 - 1, .blink_time = 150 },
	{ .throughput = 11 * 1024 - 1, .blink_time = 130 },
	{ .throughput = 16 * 1024 - 1, .blink_time = 110 },
	{ .throughput = 22 * 1024 - 1, .blink_time = 80 },
	{ .throughput = 29 * 1024 - 1, .blink_time = 50 },
};

static void morse_led_callback(struct led_classdev *led_cdev, enum led_brightness brightness)
{
	struct morse_led *led = container_of(led_cdev, struct morse_led, led_cdev);

	led->active = brightness > 0 ? 1 : 0;
	schedule_work(&led->work);
}

static void led_work_function(struct work_struct *work)
{
	struct morse *mors;
	struct device *dev;
	struct morse_led *led = container_of(work, struct morse_led, work);

	dev = led->led_cdev.dev->parent;
	if (!dev)
		return;

	mors = (struct morse *)dev_get_drvdata(dev);
	if (!mors || !mors->cfg || !mors->cfg->gpio_write_output)
		return;

	mors->cfg->gpio_write_output(mors, led->pin_num, led->active);
}

static int morse_led_feature_enabled(struct morse *mors)
{
	return  !morse_mac_ps_enabled(mors) && mors->cfg->led_group.enable_led_support;
}

static int morse_led_register(struct morse *mors, struct morse_led *led, int pin_num,
			const char *name, enum led_mode mode)
{
	int ret = 0;

	led->mode = mode;
	if (led->mode == MORSE_LED_DISABLED)
		return 0;

	ret = (!mors->cfg->gpio_enable_output || mors->cfg->gpio_enable_output(mors, pin_num, 1));
	if (ret) {
		MORSE_WARN(mors, "LED could not enable GPIO %d output\n", pin_num);
		led->mode = MORSE_LED_DISABLED;
		return ret;
	}

	led->pin_num = pin_num;
	led->mode = mode;

	switch (mode) {
	case MORSE_LED_HOST:
		/* Don't register with mac80211, just turn on */
		if (mors->cfg->gpio_write_output)
			mors->cfg->gpio_write_output(mors, led->pin_num, 1);
		return 0;
	case MORSE_LED_TX:
		led->led_cdev.default_trigger = ieee80211_get_tx_led_name(mors->hw);
		break;
	case MORSE_LED_RX:
		led->led_cdev.default_trigger = ieee80211_get_rx_led_name(mors->hw);
		break;
	case MORSE_LED_ASSOC:
		led->led_cdev.default_trigger = ieee80211_get_assoc_led_name(mors->hw);
		break;
	case MORSE_LED_RADIO:
		led->led_cdev.default_trigger = ieee80211_get_radio_led_name(mors->hw);
		break;
	case MORSE_LED_TPT:
		led->led_cdev.default_trigger =
			ieee80211_create_tpt_led_trigger(mors->hw,
					IEEE80211_TPT_LEDTRIG_FL_CONNECTED,
					morse_blink, ARRAY_SIZE(morse_blink));
		break;
	case MORSE_LED_USER_DEF:
		/* Still register led but defer control to user space */
		led->led_cdev.default_trigger = NULL;
		break;
	default:
		MORSE_WARN(mors, "Unknown LED behaviour\n");
		break;
	}

	led->led_cdev.name = name;
	led->led_cdev.max_brightness = 1;
	led->led_cdev.brightness_set = morse_led_callback;

	INIT_WORK(&led->work, led_work_function);

	ret = led_classdev_register(mors->dev, &led->led_cdev);
	if (ret) {
		MORSE_WARN(mors, "Failed to register LED device on pin %d\n", pin_num);
		if (mors->cfg->gpio_write_output)
			mors->cfg->gpio_write_output(mors, led->pin_num, 0);
		if (mors->cfg->gpio_enable_output)
			mors->cfg->gpio_enable_output(mors, led->pin_num, 0);
		led->mode = MORSE_LED_DISABLED;
	}

	return ret;
}

void morse_led_init(struct morse *mors)
{
	/* This initial LED implemetation is presently only available for mm810x softmac, and
	 * only when powersave is disabled. The default GPIO pins are hardcoded in, and are not
	 * configurable at run time. The led behaviour is also hard-coded, but can be configured in
	 * user space.
	 */
	struct morse_led_group *led_group;

	if (is_fullmac_mode() || !morse_led_feature_enabled(mors))
		return;

	led_group = &mors->cfg->led_group;
	led_group->num_leds = NUM_LEDS;

	morse_led_register(mors, &led_group->leds[0], LED_G, "morse_led0", MORSE_LED_ASSOC);
	morse_led_register(mors, &led_group->leds[1], LED_R, "morse_led1", MORSE_LED_TPT);
	morse_led_register(mors, &led_group->leds[2], LED_PWR, "morse_led2", MORSE_LED_HOST);
}

void morse_led_exit(struct morse *mors)
{
	int i;
	struct morse_led *led;
	struct morse_led_group *led_group;

	if (is_fullmac_mode() || !morse_led_feature_enabled(mors))
		return;

	led_group = &mors->cfg->led_group;
	for (i = 0; i < led_group->num_leds; i++) {
		led = &led_group->leds[i];

		switch (led->mode) {
		case MORSE_LED_DISABLED:
			continue;
		case MORSE_LED_TX:
		case MORSE_LED_RX:
		case MORSE_LED_ASSOC:
		case MORSE_LED_RADIO:
		case MORSE_LED_TPT:
		case MORSE_LED_USER_DEF:
			led_classdev_unregister(&led->led_cdev);
			cancel_work_sync(&led->work);
			fallthrough;
		case MORSE_LED_HOST:
			if (mors->cfg->gpio_write_output)
				mors->cfg->gpio_write_output(mors, led->pin_num, 0);
			if (mors->cfg->gpio_enable_output)
				mors->cfg->gpio_enable_output(mors, led->pin_num, 0);
			led->mode = MORSE_LED_DISABLED;
			break;
		}
	}
}
