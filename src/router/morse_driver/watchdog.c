/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include "morse.h"
#include "debug.h"
#include "mac.h"

static enum hrtimer_restart morse_watchdog_fire(struct hrtimer *timer)
{
	ktime_t interval;
	struct morse *mors = container_of(timer, struct morse, watchdog.timer);

	if (mors->watchdog.ping) {
		if (!mors->watchdog.paused)
			mors->watchdog.ping(mors);
	}

	/* Get the updated watchdog interval secs */
	mors->watchdog.interval_secs = morse_mac_get_watchdog_interval_secs();

	interval = ktime_set(mors->watchdog.interval_secs, 0);

#if defined(MAC80211_BACKPORT_VERSION_CODE) && (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
	hrtimer_forward_now(&mors->watchdog.timer, interval.tv64);
#else
	hrtimer_forward_now(&mors->watchdog.timer, interval);
#endif

	return HRTIMER_RESTART;
}

static void watchdog_timer_start(struct morse *mors)
{
	ktime_t interval = ktime_set(mors->watchdog.interval_secs, 0);

#if defined(MAC80211_BACKPORT_VERSION_CODE) && (KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE)
	hrtimer_start(&mors->watchdog.timer, interval.tv64, HRTIMER_MODE_REL);
#else
	hrtimer_start(&mors->watchdog.timer, interval, HRTIMER_MODE_REL);
#endif
}

int morse_watchdog_start(struct morse *mors)
{
	int ret = 0;

	mutex_lock(&mors->watchdog.lock);

	if (!hrtimer_active(&mors->watchdog.timer) && !mors->watchdog.paused) {
		MORSE_INFO(mors, "Starting ...\n");
		watchdog_timer_start(mors);
	}

	mors->watchdog.consumers++;

	mutex_unlock(&mors->watchdog.lock);

	MORSE_INFO(mors, "Started (interval=%ds, consumers=%d) ...\n",
		   mors->watchdog.interval_secs, mors->watchdog.consumers);

	return ret;
}

int morse_watchdog_refresh(struct morse *mors)
{
	int ret = -1;

	mutex_lock(&mors->watchdog.lock);

	if (mors->watchdog.consumers < 1)
		goto exit;

	if (hrtimer_active(&mors->watchdog.timer)) {
		hrtimer_cancel(&mors->watchdog.timer);
		watchdog_timer_start(mors);
		ret = 0;
	}

exit:
	mutex_unlock(&mors->watchdog.lock);
	return ret;
}

int morse_watchdog_stop(struct morse *mors)
{
	int ret = -1;

	mutex_lock(&mors->watchdog.lock);

	if (mors->watchdog.consumers < 1)
		goto exit;

	if (hrtimer_active(&mors->watchdog.timer)) {
		if (--mors->watchdog.consumers > 0) {
			MORSE_INFO(mors, "Ignored because %d consumers are using watchdog\n",
				   mors->watchdog.consumers);
			ret = 0;
		}

		hrtimer_cancel(&mors->watchdog.timer);
	} else {
		MORSE_INFO(mors, "Watchdog has been stopped\n");
	}

exit:
	mutex_unlock(&mors->watchdog.lock);
	return ret;
}

int morse_watchdog_pause(struct morse *mors)
{
	int ret = -1;

	mutex_lock(&mors->watchdog.lock);

	if (mors->watchdog.consumers < 1)
		goto exit;

	if (mors->watchdog.paused) {
		mors->watchdog.paused++;
		goto exit;
	}

	if (hrtimer_active(&mors->watchdog.timer)) {
		hrtimer_cancel(&mors->watchdog.timer);
		ret = 0;
	}
	mors->watchdog.paused++;
	MORSE_INFO(mors, "Watchdog has been paused\n");

exit:
	mutex_unlock(&mors->watchdog.lock);
	return ret;
}

int morse_watchdog_resume(struct morse *mors)
{
	int ret = -1;

	mutex_lock(&mors->watchdog.lock);

	if (mors->watchdog.consumers < 1)
		goto exit;

	if (mors->watchdog.paused > 0)
		mors->watchdog.paused--;
	else
		goto exit;	/* Nothing to do */

	if (mors->watchdog.paused)
		goto exit;	/* Still some callers that want to keep it paused */

	if (!hrtimer_active(&mors->watchdog.timer)) {
		watchdog_timer_start(mors);
		ret = 0;
	}

	MORSE_INFO(mors, "Watchdog has been resumed\n");

exit:
	mutex_unlock(&mors->watchdog.lock);
	return ret;
}

uint morse_watchdog_get_interval(struct morse *mors)
{
	return mors->watchdog.interval_secs;
}

int morse_watchdog_init(struct morse *mors, uint interval_s,
						watchdog_callback_t ping)
{
	int ret = 0;

	hrtimer_init(&mors->watchdog.timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	mors->watchdog.timer.function = &morse_watchdog_fire;

	mors->watchdog.interval_secs = interval_s;
	mors->watchdog.ping = ping;
	mors->watchdog.consumers = 0;
	mors->watchdog.paused = 0;

	mutex_init(&mors->watchdog.lock);

	return ret;
}

int morse_watchdog_cleanup(struct morse *mors)
{
	if (mors->watchdog.consumers > 0) {
		mors->watchdog.consumers = 1;
		morse_watchdog_stop(mors);
	}

	return 0;
}
