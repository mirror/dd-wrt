/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/types.h>
#include <linux/gpio.h>
#include "morse.h"
#include "debug.h"
#include "hw.h"
#include "pager_if.h"
#include "bus.h"
#include "firmware.h"

#define MORSE_HWCLOCK_DBG(_m, _f, _a...)	morse_dbg(FEATURE_ID_HWCLOCK, _m, _f, ##_a)
#define MORSE_HWCLOCK_INFO(_m, _f, _a...)	morse_info(FEATURE_ID_HWCLOCK, _m, _f, ##_a)
#define MORSE_HWCLOCK_WARN(_m, _f, _a...)	morse_warn(FEATURE_ID_HWCLOCK, _m, _f, ##_a)
#define MORSE_HWCLOCK_ERR(_m, _f, _a...)	morse_err(FEATURE_ID_HWCLOCK, _m, _f, ##_a)

static int hw_reload_after_stop __read_mostly = 5;
module_param(hw_reload_after_stop, int, 0644);
MODULE_PARM_DESC(hw_reload_after_stop,
"Reload HW after a stop notification. Abort if stop events are less than this seconds apart (-1 to disable)");

/* Re-attach to a running hardware */
bool reattach_hw __read_mostly;
module_param(reattach_hw, bool, 0644);
MODULE_PARM_DESC(reattach_hw,
	"Do not reset hardware state during module exit, attempt to reattach during module init");

int morse_hw_irq_enable(struct morse *mors, u32 irq, bool enable)
{
	u32 irq_en, irq_en_addr = irq < 32 ? MORSE_REG_INT1_EN(mors) : MORSE_REG_INT2_EN(mors);
	u32 irq_clr_addr = irq < 32 ? MORSE_REG_INT1_CLR(mors) : MORSE_REG_INT2_CLR(mors);
	u32 mask = irq < 32 ? (1 << irq) : (1 << (irq - 32));

	morse_claim_bus(mors);
	morse_reg32_read(mors, irq_en_addr, &irq_en);
	if (enable)
		irq_en |= (mask);
	else
		irq_en &= ~(mask);
	morse_reg32_write(mors, irq_clr_addr, mask);
	morse_reg32_write(mors, irq_en_addr, irq_en);
	morse_release_bus(mors);

	return 0;
}

void morse_hw_stop_work(struct work_struct *work)
{
	struct morse *mors = container_of(work, struct morse, hw_stop);

	if (!mors->started) {
		dev_err(mors->dev, "HW already stopped\n");
		return;
	}

	if (hw_reload_after_stop > 0 &&
	    (ktime_get_seconds() - mors->last_hw_stop) < hw_reload_after_stop) {
		/* HW reload was attempted twice in rapid succession - abort to prevent thrashing */
		dev_err(mors->dev,
			"Automatic HW reload aborted due to retry in < %ds\n",
			hw_reload_after_stop);
		return;
	}

	mutex_lock(&mors->lock);
	if (!morse_coredump_new(mors, MORSE_COREDUMP_REASON_CHIP_INDICATED_STOP))
		set_bit(MORSE_STATE_FLAG_DO_COREDUMP, &mors->state_flags);

	set_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags);
	mors->last_hw_stop = ktime_get_seconds();
	mutex_unlock(&mors->lock);
	schedule_work(&mors->driver_restart);
}

static void to_host_hw_stop_irq_handle(struct morse *mors)
{
	dev_err(mors->dev, "HW has stopped%s\n",
		(hw_reload_after_stop < 0) ? " (ignoring)" : "");

	if (hw_reload_after_stop < 0)
		return;

	schedule_work(&mors->hw_stop);
}

static void morse_hw_attach_done_irq_handle(struct morse *mors)
{
	struct completion *attach = READ_ONCE(mors->attach_done);

	if (attach)
		complete(attach);
}

int morse_hw_irq_handle(struct morse *mors)
{
	u32 status1 = 0;
#if defined(CONFIG_MORSE_DEBUG_IRQ)
	int i;
#endif

	morse_reg32_read(mors, MORSE_REG_INT1_STS(mors), &status1);

	if (status1 & MORSE_CHIP_IF_IRQ_MASK_ALL)
		mors->cfg->ops->chip_if_handle_irq(mors, status1);
	if (status1 & MORSE_INT_BEACON_VIF_MASK_ALL)
		morse_beacon_irq_handle(mors, status1);
	if (status1 & MORSE_INT_NDP_PROBE_REQ_PV0_VIF_MASK_ALL)
		morse_ndp_probe_req_resp_irq_handle(mors, status1);
	if (status1 & MORSE_INT_HW_STOP_NOTIFICATION)
		to_host_hw_stop_irq_handle(mors);
	if (status1 & MORSE_HW_ATTACH_DONE)
		morse_hw_attach_done_irq_handle(mors);
#if defined(CONFIG_MORSE_ENABLE_TEST_MODES)
	if (status1 & MORSE_INT_BUS_IRQ_SELF_TEST)
		morse_bus_interrupt_profiler_irq(mors);
#endif

	morse_reg32_write(mors, MORSE_REG_INT1_CLR(mors), status1);

#if defined(CONFIG_MORSE_DEBUG_IRQ)
	mors->debug.hostsync_stats.irq++;
	for (i = 0; i < ARRAY_SIZE(mors->debug.hostsync_stats.irq_bits); i++) {
		if (status1 & BIT(i))
			mors->debug.hostsync_stats.irq_bits[i]++;
	}
#endif

	return status1 ? 1 : 0;
}

int morse_hw_irq_clear(struct morse *mors)
{
	morse_claim_bus(mors);
	morse_reg32_write(mors, MORSE_REG_INT1_CLR(mors), 0xFFFFFFFF);
	morse_reg32_write(mors, MORSE_REG_INT2_CLR(mors), 0xFFFFFFFF);
	morse_release_bus(mors);
	return 0;
}

int morse_hw_toggle_aon_latch(struct morse *mors)
{
	u32 address = MORSE_REG_AON_LATCH_ADDR(mors);
	u32 mask = MORSE_REG_AON_LATCH_MASK(mors);
	u32 latch;

	if (address) {
		/* invoke AON latch procedure */
		morse_reg32_read(mors, address, &latch);
		morse_reg32_write(mors, address, latch & ~(mask));
		mdelay(5);
		morse_reg32_write(mors, address, latch | mask);
		mdelay(5);
		morse_reg32_write(mors, address, latch & ~(mask));
		mdelay(5);
	}

	return 0;
}

int morse_hw_reset(int reset_pin)
{
	int ret = gpio_request(reset_pin, "morse-reset-ctrl");

	if (ret < 0) {
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, "Failed to acquire reset gpio. Skipping reset.\n");
		return ret;
	}

	pr_info("Resetting Morse Chip\n");
	gpio_direction_output(reset_pin, 0);
	mdelay(20);
	/* setting gpio as float to avoid forcing 3.3V High */
	gpio_direction_input(reset_pin);
	pr_info("Done\n");

	gpio_free(reset_pin);

	return ret;
}

int morse_hw_clock_now(const struct morse *mors, u64 *now)
{
	int ret;
	const struct morse_hw_clock *local;
	u64 clock_val;
	ktime_t reference;

	rcu_read_lock();
	local = rcu_dereference(mors->hw_clock.clock);
	if (local) {
		ret = 0;
		clock_val = local->val;
		reference = local->reference;
	} else {
		/* Clock reference yet to be taken */
		ret = -EFAULT;
	}
	rcu_read_unlock();

	if (ret)
		MORSE_HWCLOCK_ERR(mors, "%s: failed (ret:%d)", __func__, ret);
	else
		*now = clock_val + ktime_to_us(ktime_sub(ktime_get_boottime(), reference));

	return ret;
}

static bool hw_supports_clock_read(const struct morse *mors)
{
	return MORSE_REG_MTIME_LOWER(mors) || MORSE_REG_MTIME_UPPER(mors);
}

static int hw_read_clock(struct morse *mors, u64 *out)
{
	int ret;
	u32 lower1;
	u32 lower2;
	u32 upper;

	ret = morse_reg32_read(mors, MORSE_REG_MTIME_LOWER(mors), &lower1);
	if (ret)
		return ret;

	ret = morse_reg32_read(mors, MORSE_REG_MTIME_UPPER(mors), &upper);
	if (ret)
		return ret;

	ret = morse_reg32_read(mors, MORSE_REG_MTIME_LOWER(mors), &lower2);
	if (ret)
		return ret;

	/* If lower has wrapped, upper will have incremented */
	if (lower2 < lower1)
		upper++;

	*out = ((u64)upper << 32) | lower2;
	return 0;
}

int morse_hw_clock_update(struct morse *mors)
{
	int ret;
	u64 now = 0;
	unsigned long flags;
	struct morse_hw_clock *new;
	struct morse_hw_clock *old;
	ktime_t reference;

	if (!hw_supports_clock_read(mors)) {
		ret = -ENOTSUPP;
		MORSE_WARN_ON_ONCE(FEATURE_ID_HWCLOCK, 1);
		goto exit;
	}

	/* This code is time-critical - disable interrupts */
	local_irq_save(flags);
	ret = hw_read_clock(mors, &now);
	reference = ktime_get_boottime();
	local_irq_restore(flags);

	if (ret)
		goto exit;

	MORSE_HWCLOCK_DBG(mors, "%s: %lld us\n", __func__, now);
	new = kmalloc(sizeof(*new), GFP_KERNEL);
	if (!new) {
		ret = -ENOMEM;
		goto exit;
	}
	new->val = now;
	new->reference = reference;

	/* Old may be NULL if this is the first update operation */
	old = rcu_dereference(mors->hw_clock.clock);
	rcu_assign_pointer(mors->hw_clock.clock, new);
	kfree_rcu(old, rcu);

exit:
	mutex_lock(&mors->hw_clock.update_wait_lock);
	if (mors->hw_clock.update)
		complete(mors->hw_clock.update);
	mutex_unlock(&mors->hw_clock.update_wait_lock);

	if (ret)
		MORSE_HWCLOCK_ERR(mors, "%s: failed to update clock (ret:%d)\n", __func__, ret);

	return ret;
}

int morse_hw_clock_trigger_update(struct morse *mors, bool wait)
{
	int ret = 0;
	unsigned int timeout_ms;
	const int overhead_ms = 10;
	DECLARE_COMPLETION_ONSTACK(hw_clock_update);

	if (!hw_supports_clock_read(mors)) {
		ret = -EOPNOTSUPP;
		goto exit;
	}

	/* Derive timeout to wait for clock update */
	timeout_ms = mors->cfg->get_ps_wakeup_delay_ms(mors->chip_id) + overhead_ms;

	if (wait) {
		mutex_lock(&mors->hw_clock.update_wait_lock);
		/* WARN if HW clock completion already set/waiting */
		MORSE_WARN_ON(FEATURE_ID_HWCLOCK, mors->hw_clock.update);
		mors->hw_clock.update = &hw_clock_update;
		mutex_unlock(&mors->hw_clock.update_wait_lock);
	}

	set_bit(MORSE_UPDATE_HW_CLOCK_REFERENCE, &mors->chip_if->event_flags);
	queue_work(mors->chip_wq, &mors->chip_if_work);

	if (wait) {
		unsigned long rem = wait_for_completion_timeout(mors->hw_clock.update,
								msecs_to_jiffies(timeout_ms));

		/* Clear completion */
		mutex_lock(&mors->hw_clock.update_wait_lock);
		mors->hw_clock.update = NULL;
		mutex_unlock(&mors->hw_clock.update_wait_lock);
		ret = (rem) ? 0 : -ETIMEDOUT;
	}

exit:
	if (ret)
		MORSE_HWCLOCK_ERR(mors, "%s: failed to trigger update (ret:%d)\n", __func__, ret);

	return ret;
}

bool is_otp_xtal_wait_supported(struct morse *mors)
{
	int ret;
	u32 otp_word2;
	u32 otp_xtal_wait;

	if (MORSE_REG_OTP_DATA_WORD(mors, 0) == 0)
		/* Device doesn't support OTP (probably an FPGA) */
		return true;

	if (MORSE_REG_OTP_DATA_WORD(mors, 2) != 0) {
		morse_claim_bus(mors);
		ret = morse_reg32_read(mors, MORSE_REG_OTP_DATA_WORD(mors, 2), &otp_word2);
		morse_release_bus(mors);
		if (ret < 0) {
			MORSE_ERR(mors, "OTP data2 value read failed: %d\n", ret);
			return false;
		}
		otp_xtal_wait = (otp_word2 & MM610X_OTP_DATA2_XTAL_WAIT_POS);
		if (!otp_xtal_wait) {
			ret = -1;
			MORSE_ERR(mors, "OTP xtal wait bits not set\n");
			return false;
		}
		return true;
	}
	return false;
}

int morse_hw_enable_stop_notifications(struct morse *mors, bool enable)
{
	return morse_hw_irq_enable(mors, MORSE_INT_HW_STOP_NOTIFICATION_NUM, enable);
}

int morse_chip_cfg_detect_and_init(struct morse *mors, struct morse_chip_series *mors_chip_series)
{
	int ret = 0;
	u32 chip_id = 0;

	morse_claim_bus(mors);
	ret = morse_reg32_read(mors, mors_chip_series->chip_id_address, &chip_id);
	morse_release_bus(mors);
	if (ret < 0) {
		MORSE_ERR(mors, "%s: Failed to access HW (errno:%d)", __func__, ret);
		return ret;
	}

	ret = morse_chip_cfg_init(mors, chip_id);

	return ret;
}

int morse_chip_cfg_init(struct morse *mors, u32 chip_id)
{
	int ret = 0;

	mors->chip_id = chip_id;

	switch (chip_id) {
	case(MM8108B0_ID):
	case(MM8108B1_ID):
	case(MM8108B2_ID):
	case(MM8108B0_FPGA_ID):
	case(MM8108B1_FPGA_ID):
	case(MM8108B2_FPGA_ID):
		mors->cfg = &mm8108_cfg;
		break;
	case(MM6108A0_ID):
	case(MM6108A1_ID):
	case(MM6108A2_ID):
		mors->cfg = &mm6108_cfg;
		break;
	default:
		return -ENODEV;
	}

	return ret;
}

bool morse_hw_is_already_loaded(struct morse *mors)
{
	return (morse_firmware_get_host_table_ptr(mors) == 0) &&
			(morse_firmware_magic_verify(mors) == 0) &&
			(morse_firmware_check_compatibility(mors) == 0);
}

int morse_hw_attach_done_irq_enable(struct morse *mors, bool enable)
{
	return morse_hw_irq_enable(mors, MORSE_HW_ATTACH_DONE_NUM, enable);
}

int morse_hw_attach(struct morse *mors)
{
	int ret = 0;
	unsigned int rem = 0;
    /* Current FW takes about 1350 ms to boot in the worst case. Attach time is a fraction of that,
     * so use this value as the baseline for attach timeout.
     */
	const int timeout_ms = 1350;
	DECLARE_COMPLETION_ONSTACK(attach_done);

	if (!(mors->firmware_flags & MORSE_FW_FLAGS_SUPPORT_HW_REATTACH)) {
		ret = -ENOTSUPP;
		goto exit;
	}

	ret = morse_hw_attach_done_irq_enable(mors, true);
	if (ret)
		goto exit;

	morse_claim_bus(mors);
	ret = morse_reg32_write(mors, MORSE_HW_ATTACH_TRGR_SET(mors), MORSE_HW_ATTACH_IRQ_BIT);
	morse_release_bus(mors);

	if (ret)
		goto exit;

	WRITE_ONCE(mors->attach_done, &attach_done);
	rem = wait_for_completion_timeout(mors->attach_done, msecs_to_jiffies(timeout_ms));
	WRITE_ONCE(mors->attach_done, NULL);

	if (rem == 0)
		ret = -ETIMEDOUT;

exit:
	if (ret)
		MORSE_ERR(mors, "%s: failed to attach to hardware (ret:%d)\n", __func__, ret);
	else
		MORSE_INFO(mors, "%s: Attached to running hardware\n", __func__);

	morse_hw_attach_done_irq_enable(mors, false);

	return ret;
}

int morse_hw_detach(struct morse *mors)
{
	int ret;

	if (!(mors->firmware_flags & MORSE_FW_FLAGS_SUPPORT_HW_REATTACH)) {
		ret = -ENOTSUPP;
		goto exit;
	}

	morse_claim_bus(mors);
	ret = morse_reg32_write(mors, MORSE_HW_DETACH_TRGR_SET(mors), MORSE_HW_DETACH_IRQ_BIT);
	morse_release_bus(mors);

exit:
	if (ret)
		MORSE_ERR(mors, "%s: failed to detach from hardware (ret:%d)\n", __func__, ret);
	else
		MORSE_INFO(mors, "%s: Detached from hardware\n", __func__);

	return ret;
}

bool morse_hw_is_stopped(struct morse *mors)
{
	u32 status1 = 0;
	int ret = 0;

	morse_claim_bus(mors);
	ret = morse_reg32_read(mors, MORSE_REG_INT1_STS(mors), &status1);
	morse_release_bus(mors);

	return (ret == 0) && (status1 & MORSE_INT_HW_STOP_NOTIFICATION);
}

bool morse_hw_should_reattach(void)
{
	return reattach_hw;
}
