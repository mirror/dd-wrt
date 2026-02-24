/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>

#include "morse.h"
#include "debug.h"
#include "skbq.h"
#include "mac.h"
#include "bus.h"
#include "hw.h"
#include "ps.h"

#define MORSE_PS_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_POWERSAVE, _m, _f, ##_a)

static bool morse_ps_is_busy_pin_asserted(struct morse *mors)
{
	bool active_high = !(mors->firmware_flags & MORSE_FW_FLAGS_BUSY_ACTIVE_LOW);

	if (!mors->cfg->mm_ps_gpios_supported)
		return false;

	return (!!gpio_get_value(mors->cfg->mm_ps_async_gpio) == active_high);
}

static u8 morse_ps_get_wakeup_delay_ms(struct morse *mors)
{
	return mors->cfg->get_ps_wakeup_delay_ms(mors->chip_id);
}

static void morse_ps_set_wake_gpio(struct morse *mors, bool raise)
{
	int ret;

	if (!mors->cfg->mm_ps_gpios_supported)
		return;

	ret = gpio_direction_output(mors->cfg->mm_wake_gpio, (raise) ? 1 : 0);

	if (ret) {
		MORSE_ERR(mors, "%s: Failed to %s wake pin (ret:%d)", __func__,
			  (raise) ? "raise" : "lower", ret);
		return;
	}

	MORSE_PS_DBG(mors, "%s: %s wake up pin", __func__, (raise) ? "set" : "cleared");
}

static void morse_ps_wait_after_wake_pin_raise(struct morse *mors)
{
	unsigned long rem;
	unsigned long timeout;
	unsigned int max_boot_time_ms = morse_ps_get_wakeup_delay_ms(mors);
	bool hw_signals_wake = !!(mors->firmware_flags &
				  MORSE_FW_FLAGS_TOGGLES_BUSY_PIN_ON_WAKE_PIN);

	if (!mors->cfg->mm_ps_gpios_supported)
		return;

	if (morse_ps_is_busy_pin_asserted(mors))
		return; /* device already indicating busy - don't wait the delay */

	/* increase grace period - HW should signal wake well within expected boot time */
	if (hw_signals_wake)
		max_boot_time_ms *= 3;

	timeout = msecs_to_jiffies(max_boot_time_ms);
	rem = wait_for_completion_timeout(mors->ps.awake, timeout);
	MORSE_PS_DBG(mors, "%s: took %dms to wake\n", __func__, jiffies_to_msecs(timeout - rem));

	if (hw_signals_wake && rem == 0) {
		MORSE_WARN(mors, "%s: HW did not signal in the expected boot period, continuing\n",
			   __func__);
	}
}

static int morse_ps_wakeup(struct morse_ps *mps)
{
	DECLARE_COMPLETION_ONSTACK(awake);
	struct morse *mors = container_of(mps, struct morse, ps);

	if (!mps->enable)
		return 0;

	if (!mps->suspended)
		return 0;

	WRITE_ONCE(mors->ps.awake, &awake);
	morse_ps_set_wake_gpio(mors, true);
	morse_ps_wait_after_wake_pin_raise(mors);
	WRITE_ONCE(mors->ps.awake, NULL);

	morse_set_bus_enable(mors, true);
	mps->suspended = false;
	return 0;
}

static int morse_ps_sleep(struct morse_ps *mps)
{
	struct morse *mors = container_of(mps, struct morse, ps);

	if (!mps->enable)
		return 0;

	if (mps->suspended)
		return 0;

	mps->suspended = true;
	morse_set_bus_enable(mors, false);
	morse_ps_set_wake_gpio(mors, false);
	return 0;
}

static irqreturn_t morse_ps_irq_handle(int irq, void *arg)
{
	struct morse_ps *mps = (struct morse_ps *)arg;
	struct morse *mors = container_of(mps, struct morse, ps);
	struct completion *awake = READ_ONCE(mps->awake);

	if (awake)
		complete(awake);
	else
		queue_work(mors->chip_wq, &mps->async_wake_work);

	return IRQ_HANDLED;
}

static void morse_ps_async_wake_work(struct work_struct *work)
{
	struct morse_ps *mps = container_of(work, struct morse_ps, async_wake_work);
	struct morse *mors = container_of(mps, struct morse, ps);

	if (!morse_ps_is_busy_pin_asserted(mors))
		return;

	mutex_lock(&mps->lock);
	morse_ps_wakeup(mps);
	mutex_unlock(&mps->lock);
}

void morse_ps_bus_activity(struct morse *mors, int timeout_ms)
{
	mutex_lock(&mors->ps.lock);
	mors->ps.bus_ps_timeout = jiffies + msecs_to_jiffies(timeout_ms);
	mutex_unlock(&mors->ps.lock);
}

static int morse_ps_evaluate(struct morse_ps *mps)
{
	struct morse *mors = container_of(mps, struct morse, ps);
	bool needs_wake = false;
	bool eval_later = false;
	unsigned long flags_on_entry =
		(mors->chip_if->event_flags & ~BIT(MORSE_DATA_TRAFFIC_PAUSE_PEND));

	if (!mps->enable)
		return 0;

	needs_wake = (mps->wakers > 0);
	needs_wake |= (flags_on_entry > 0);
	needs_wake |= (mors->cfg->ops->skbq_get_tx_buffered_count(mors) > 0);

	if (!needs_wake &&
	    mps->dynamic_ps_en &&
	    morse_is_data_tx_allowed(mors) && time_before(jiffies, mps->bus_ps_timeout)) {
		/*
		 * Eval later if there is nothing explicitly holding the bus awake,
		 * but the bus ps timeout has been set to some time in the future
		 * (i.e. network traffic has recently occurred).
		 *
		 * In TWT, the device may go into TWT sleep immediately without
		 * caring about recent network traffic.
		 */
		needs_wake = true;
		eval_later = true;
	}

	if (needs_wake) {
		morse_ps_wakeup(mps);
	} else if (morse_ps_is_busy_pin_asserted(mors)) {
		/* Chip has something to send across the bus, re-evaluate later */
		eval_later = true;
	} else {
		MORSE_WARN_ON(FEATURE_ID_POWERSAVE, eval_later);
		morse_ps_sleep(mps);
	}

	if (eval_later) {
		unsigned long expire = jiffies + msecs_to_jiffies(DEFAULT_BUS_TIMEOUT_MS);

		if (mps->dynamic_ps_en && time_after(mps->bus_ps_timeout, expire))
			expire = mps->bus_ps_timeout;

		expire = expire - jiffies;
		MORSE_PS_DBG(mors, "%s: Delaying eval work by %d ms\n",
			     __func__, jiffies_to_msecs(expire));

		cancel_delayed_work(&mps->delayed_eval_work);
		queue_delayed_work(mors->chip_wq, &mps->delayed_eval_work, expire);
	}

	return 0;
}

static void morse_ps_evaluate_work(struct work_struct *work)
{
	struct morse_ps *mps = container_of(work, struct morse_ps, delayed_eval_work.work);

	if (!mps->enable)
		return;

	mutex_lock(&mps->lock);
	morse_ps_evaluate(mps);
	mutex_unlock(&mps->lock);
}

int morse_ps_enable(struct morse *mors)
{
	int ret;
	struct morse_ps *mps = &mors->ps;

	if (!mps->enable)
		return 0;

	mutex_lock(&mps->lock);

	if (mps->wakers == 0) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_POWERSAVE, 1);
		ret = -EFAULT;
	} else {
		mps->wakers--;
		ret = morse_ps_evaluate(mps);
	}

	mutex_unlock(&mps->lock);

	return ret;
}

int morse_ps_disable(struct morse *mors)
{
	int ret;
	struct morse_ps *mps = &mors->ps;

	if (!mps->enable)
		return 0;

	mutex_lock(&mps->lock);
	mps->wakers++;
	ret = morse_ps_evaluate(mps);
	mutex_unlock(&mps->lock);

	return ret;
}

int morse_ps_init(struct morse *mors, bool enable, bool enable_dynamic_ps)
{
	int ret;
	int irq = gpio_to_irq(mors->cfg->mm_ps_async_gpio);
	struct morse_ps *mps = &mors->ps;

	mps->enable = enable;
	mps->bus_ps_timeout = jiffies;
	mps->dynamic_ps_en = enable_dynamic_ps;
	mps->suspended = false;
	mps->wakers = 1;	/* we default to being on */
	mutex_init(&mps->lock);

	if (mps->enable) {
		INIT_WORK(&mps->async_wake_work, morse_ps_async_wake_work);
		INIT_DELAYED_WORK(&mps->delayed_eval_work, morse_ps_evaluate_work);

		if (!mors->cfg->mm_ps_gpios_supported) {
			/* The rest of the code is GPIO related, we need to bail */
			return 0;
		}

		/**
		 * SW-1674: Should be the following, but issues observed.
		 * gpio_request_one(mors->cfg->mm_wake_gpio, GPIOF_OPEN_DRAIN, NULL);
		 */
		/* Default to allow chip to wakeup */
		ret = gpio_request(mors->cfg->mm_wake_gpio, "morse-wakeup-ctrl");
		if (ret < 0) {
			MORSE_PR_ERR(FEATURE_ID_POWERSAVE, "Failed to acquire wakeup gpio.\n");
			return ret;
		}
		gpio_direction_output(mors->cfg->mm_wake_gpio, 1);

		gpio_request(mors->cfg->mm_ps_async_gpio, "morse-async-wakeup-ctrl");

		/* The following input gpio must be configured with pull-down */
		gpio_direction_input(mors->cfg->mm_ps_async_gpio);

		ret = request_irq(irq, (irq_handler_t)morse_ps_irq_handle,
				  (mors->firmware_flags & MORSE_FW_FLAGS_BUSY_ACTIVE_LOW) ?
					IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING,
					"async_wakeup_from_chip", mps);

		MORSE_WARN_ON(FEATURE_ID_POWERSAVE, ret);
	}

	return 0;
}

void morse_ps_finish(struct morse *mors)
{
	struct morse_ps *mps = &mors->ps;

	if (mps->enable) {
		mps->enable = false;
		mps->dynamic_ps_en = false;

		if (mors->cfg->mm_ps_gpios_supported) {
			free_irq(gpio_to_irq(mors->cfg->mm_ps_async_gpio), mps);
			gpio_free(mors->cfg->mm_ps_async_gpio);
			gpio_free(mors->cfg->mm_wake_gpio);
		}

		cancel_work_sync(&mps->async_wake_work);
		cancel_delayed_work_sync(&mps->delayed_eval_work);
	}
}
