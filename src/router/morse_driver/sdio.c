/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/pm_runtime.h>

#include "hw.h"
#include "morse.h"
#include "bus.h"
#include "mac.h"
#include "firmware.h"
#include "debug.h"
#include "of.h"
#include "bus_trace.h"

#ifdef CONFIG_MORSE_USER_ACCESS
#include "uaccess.h"
#endif

#define MORSE_SDIO_VENDOR_ID (0x325B)

#define SDIO_CLK_DEBUGFS_MAX  128
char sdio_clk_debugfs[SDIO_CLK_DEBUGFS_MAX] = "";
module_param_string(sdio_clk_debugfs, sdio_clk_debugfs, sizeof(sdio_clk_debugfs), 0644);
MODULE_PARM_DESC(sdio_clk_debugfs,
	"Path to the SDIO clock in the debugfs used to automatically lower SDIO clock during powersave");

/** Minimum string length for the path of SDIO-CLK switching */
#define MIN_STRLEN_SDIO_CLK_PATH 20

/** Power management runtime auto-suspend delay value in milliseconds */
#define PM_RUNTIME_AUTOSUSPEND_DELAY_MS 50

/**
 * Put the SDIO-clk back to where it was before. If SDIO-clk is set to 42MHz in
 * boot/config.txt then it won't exceed 42MHz. The SDIO-clk will be 42MHz.
 **/
#define FAST_SDIO_CLK_HZ 50000000

/* Slow down SDIO CLK to 150KHz. This is the lowest value we can set. */
#define SLOW_SDIO_CLK_HZ 150000

/* Value to indicate that the base address for bulk/register read/writes has yet to be set */
#define MORSE_SDIO_BASE_ADDR_UNSET 0xFFFFFFFF

/** Max pad size for an SKB. This must be kept in sync with the firmware.
 *  2 less than max alignment requirements, as mac80211 guarantees 2 byte alignment for skbs.
 *  Pager requires pages to have a word aligned length, so round up to nearest word.
 */
#define MAX_PAGER_HOST_SKB_ALIGNMENT_PAD        (ROUND_BYTES_TO_WORD(6))

#if defined(CONFIG_MORSE_SDIO_ALIGNMENT)
#define MORSE_SDIO_ALIGNMENT (CONFIG_MORSE_SDIO_ALIGNMENT)
#else
#define MORSE_SDIO_ALIGNMENT	(2)
#endif

#define MORSE_SDIO_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_SDIO, _m, _f, ##_a)
#define MORSE_SDIO_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_SDIO, _m, _f, ##_a)
#define MORSE_SDIO_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_SDIO, _m, _f, ##_a)
#define MORSE_SDIO_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_SDIO, _m, _f, ##_a)

struct morse_sdio {
	bool enabled;
	u32 bulk_addr_base;
	u32 register_addr_base;
	struct sdio_func *func;
	const struct sdio_device_id *id;
	struct bus_trace trace;
};

#ifdef CONFIG_MORSE_USER_ACCESS
struct uaccess *morse_uaccess;
#endif

static void morse_sdio_remove(struct sdio_func *func);

static void sdio_log_err(struct morse_sdio *sdio, const char *operation, unsigned int fn,
			   unsigned int address, unsigned int len, int ret)
{
	struct morse *mors = sdio->func ? sdio_get_drvdata(sdio->func) : NULL;

	if (!mors)
		return;

	MORSE_SDIO_ERR(mors, "sdio: %s fn=%d 0x%08x:%d r=0x%08x b=0x%08x (ret:%d)",
		       operation, fn, address, len, sdio->register_addr_base,
		       sdio->bulk_addr_base, ret);
}

static void irq_handler(struct sdio_func *func1)
{
	int handled;
	struct sdio_func *func = func1->card->sdio_func[1];
	struct morse *mors = sdio_get_drvdata(func);
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;

	MORSE_WARN_ON(FEATURE_ID_SDIO, !mors);

	(void)sdio;

	handled = morse_hw_irq_handle(mors);
	if (!handled)
		MORSE_SDIO_WARN(mors, "%s: nothing was handled\n", __func__);

	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_HANDLE_IRQ, func->num, 0, handled);
}

static int morse_sdio_enable_irq(struct morse_sdio *sdio)
{
	int ret;
	struct sdio_func *func = sdio->func;
	struct sdio_func *func1 = func->card->sdio_func[0];
	struct morse *mors = sdio_get_drvdata(func);

	sdio_claim_host(func);
	/* Register the isr */
	ret = sdio_claim_irq(func1, irq_handler);
	if (ret)
		MORSE_SDIO_ERR(mors, "Failed to enable sdio irq: %d\n", ret);
	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_EN_IRQ, func1->num, 0, (ret) ? 0 : 1);
	sdio_release_host(func);
	return ret;
}

static void morse_sdio_disable_irq(struct morse_sdio *sdio)
{
	struct sdio_func *func = sdio->func;
	struct sdio_func *func1 = func->card->sdio_func[0];

	sdio_claim_host(func);
	sdio_release_irq(func1);
	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_EN_IRQ, func1->num, 0, 0);
	sdio_release_host(func);
}

static void morse_sdio_set_irq(struct morse *mors, bool enable)
{
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;

	if (enable)
		morse_sdio_enable_irq(sdio);
	else
		morse_sdio_disable_irq(sdio);
}

static u32 morse_sdio_calculate_base_address(u32 address, u8 access)
{
	return (address & MORSE_SDIO_RW_ADDR_BOUNDARY_MASK) | (access & 0x3);
}

static void morse_sdio_reset_base_address(struct morse_sdio *sdio)
{
	sdio->bulk_addr_base = MORSE_SDIO_BASE_ADDR_UNSET;
	sdio->register_addr_base = MORSE_SDIO_BASE_ADDR_UNSET;
	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_RESET_BASE_ADDRESSES, 0, 0, 0);
}

static int morse_sdio_set_func_address_base(struct morse_sdio *sdio,
					    u32 address, u8 access, bool bulk)
{
	int ret = 0;
	u8 base[4];
	const char *operation = "set_address_base";
	u32 calculated_addr_base = morse_sdio_calculate_base_address(address, access);
	u32 *current_addr_base = bulk ? &sdio->bulk_addr_base : &sdio->register_addr_base;
	bool base_addr_is_unset = (*current_addr_base == MORSE_SDIO_BASE_ADDR_UNSET);
	struct sdio_func *func2 = sdio->func;
	struct sdio_func *func1 = sdio->func->card->sdio_func[0];
	struct sdio_func *func_to_use = bulk ? func2 : func1;
	struct morse *mors = sdio_get_drvdata(sdio->func);
	int retries = 0;
	static const int max_retries = 3;

	if ((*current_addr_base) == calculated_addr_base && !base_addr_is_unset)
		return ret;

	base[0] = (u8)((address & 0x00FF0000) >> 16);
	base[1] = (u8)((address & 0xFF000000) >> 24);
	base[2] = access & 0x3;	/* 1, 2 or 4 byte access */

retry:
	/* Write them as single bytes for now */
	if (base_addr_is_unset ||
	    (base[0] != (u8)(((*current_addr_base) & 0x00FF0000) >> 16))) {
		sdio_writeb(func_to_use, base[0], MORSE_REG_ADDRESS_WINDOW_0, &ret);
		if (mors->cfg->xtal_init_bus_trans_delay_ms) {
			msleep(mors->cfg->xtal_init_bus_trans_delay_ms);
			ret = 0;
		}
		if (ret) {
			sdio_log_err(sdio, operation, func_to_use->num,
				       MORSE_REG_ADDRESS_WINDOW_0, 1, ret);
			goto err;
		}
	}

	if (base_addr_is_unset ||
	    (base[1] != (u8)(((*current_addr_base) & 0xFF000000) >> 24))) {
		sdio_writeb(func_to_use, base[1], MORSE_REG_ADDRESS_WINDOW_1, &ret);
		if (mors->cfg->xtal_init_bus_trans_delay_ms) {
			msleep(mors->cfg->xtal_init_bus_trans_delay_ms);
			ret = 0;
		}
		if (ret) {
			sdio_log_err(sdio, operation, func_to_use->num,
				       MORSE_REG_ADDRESS_WINDOW_1, 1, ret);
			goto err;
		}
	}

	if (base_addr_is_unset ||
	    (base[2] != (u8)(((*current_addr_base) & 0x3)))) {
		sdio_writeb(func_to_use, base[2], MORSE_REG_ADDRESS_CONFIG, &ret);
		if (mors->cfg->xtal_init_bus_trans_delay_ms) {
			msleep(mors->cfg->xtal_init_bus_trans_delay_ms);
			ret = 0;
		}
		if (ret) {
			sdio_log_err(sdio, operation, func_to_use->num,
				       MORSE_REG_ADDRESS_CONFIG, 1, ret);
			goto err;
		}
	}

	/**
	 * TODO: The following can give us a speed boost, but it is risky.
	 *
	 * One of the bytes you are writing sets the size of our memory access,
	 * it is byte, half word or word. This is used for
	 * extended IO (i.e using CMD53). The memcpy_toio is internally
	 * using CMD53. So you are accessing the memory before
	 * setting the access size.
	 *
	 * ret = sdio_memcpy_toio(sdio->func, MORSE_REG_ADDRESS_WINDOW_0, base, 3);
	 */

	*current_addr_base = calculated_addr_base;
	if (retries)
		MORSE_SDIO_INFO(mors, "%s succeeded after %d retries\n", __func__, retries);

	bus_trace_log(&sdio->trace, (bulk) ? BUS_TRACE_EVENT_ID_SET_BULK_BASE_ADDRESS :
		      BUS_TRACE_EVENT_ID_SET_REG_BASE_ADDRESS,
		      func_to_use->num, *current_addr_base, 4);
	return ret;
err:
	retries++;
	if (ret == -ETIMEDOUT && retries <= max_retries) {
		MORSE_SDIO_INFO(mors, "%s failed (%d), retrying (%d/%d)\n", __func__, ret,
				retries, max_retries);
		goto retry;
	}

	*current_addr_base = MORSE_SDIO_BASE_ADDR_UNSET;
	bus_trace_log(&sdio->trace, (bulk) ? BUS_TRACE_EVENT_ID_SET_BULK_BASE_ADDRESS :
		      BUS_TRACE_EVENT_ID_SET_REG_BASE_ADDRESS,
		      func_to_use->num, *current_addr_base, 4);
	return ret;
}

static struct sdio_func *morse_sdio_get_func(struct morse_sdio *sdio,
					     u32 address, ssize_t size, u8 access)
{
	int ret = 0;
	u32 calculated_base_address = morse_sdio_calculate_base_address(address, access);
	struct sdio_func *func2 = sdio->func;
	struct sdio_func *func1 = sdio->func ? sdio->func->card->sdio_func[0] : NULL;
	struct morse *mors = sdio->func ? sdio_get_drvdata(sdio->func) : NULL;
	struct sdio_func *func_to_use;

	/* Uncoditionally generate the output if !mors. Should this be an assert? */
	WARN_ON(!mors);

	/* Order matters here, please don't re-order */
	if (size > sizeof(u32)) {
		ret = morse_sdio_set_func_address_base(sdio, address, access, true);
		MORSE_WARN_ON(FEATURE_ID_SDIO, sdio->bulk_addr_base == 0);
		func_to_use = func2;
	} else if (sdio->bulk_addr_base == calculated_base_address && func2) {
		func_to_use = func2;
	} else if (func1) {
		ret = morse_sdio_set_func_address_base(sdio, address, access, false);
		MORSE_WARN_ON(FEATURE_ID_SDIO, sdio->register_addr_base == 0);
		func_to_use = func1;
	} else {
		ret = morse_sdio_set_func_address_base(sdio, address, access, true);
		MORSE_WARN_ON(FEATURE_ID_SDIO, sdio->bulk_addr_base == 0);
		func_to_use = func2;
	}

	return ret ? NULL : func_to_use;
}

static int morse_sdio_regl_write(struct morse_sdio *sdio, u32 address, u32 value)
{
	ssize_t ret = 0;
	struct morse *mors = sdio->func ? sdio_get_drvdata(sdio->func) : NULL;
	u32 original_address = address;
	struct sdio_func *func_to_use;

	if (!mors) {
		ret = -EINVAL;
		goto exit;
	}

	func_to_use = morse_sdio_get_func(sdio, address, sizeof(u32), MORSE_CONFIG_ACCESS_4BYTE);
	if (!func_to_use) {
		ret = -EIO;
		goto exit;
	}

	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_REG_WRITE, func_to_use->num, address, 4);
	address &= 0x0000FFFF;	/* remove base and keep offset */
	sdio_writel(func_to_use, (__force u32)cpu_to_le32(value),
				(__force u32)cpu_to_le32(address), (int *)&ret);

	/* return written size */
	if (ret)
		sdio_log_err(sdio, "writel", func_to_use->num, address, sizeof(u32), ret);
	else
		ret = sizeof(value);

	if (original_address == MORSE_REG_RESET(mors) && value == MORSE_REG_RESET_VALUE(mors)) {
		MORSE_SDIO_DBG(mors, "SDIO reset detected, invalidating base addr\n");
		morse_sdio_reset_base_address(sdio);
	}
exit:
	return (int)ret;
}

static int morse_sdio_regl_read(struct morse_sdio *sdio, u32 address, u32 *value)
{
	ssize_t ret = 0;
	struct morse *mors = sdio->func ? sdio_get_drvdata(sdio->func) : NULL;
	struct sdio_func *func_to_use;

	if (!mors) {
		ret = -EINVAL;
		goto exit;
	}

	func_to_use = morse_sdio_get_func(sdio, address, sizeof(u32), MORSE_CONFIG_ACCESS_4BYTE);
	if (!func_to_use) {
		ret = -EIO;
		goto exit;
	}

	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_REG_READ, func_to_use->num, address, 4);
	address &= 0x0000FFFF;	/* remove base and keep offset */
	*value = sdio_readl(func_to_use,  (__force u32)cpu_to_le32(address), (int *)&ret);
	/* return read size */
	if (ret)
		sdio_log_err(sdio, "readl", func_to_use->num, address, sizeof(u32), ret);
	else
		ret = sizeof(*value);
exit:
	return (int)ret;
}

static int morse_sdio_mem_write(struct morse_sdio *sdio, u32 address, u8 *data, ssize_t size)
{
	ssize_t ret = 0;
	struct morse *mors = sdio->func ? sdio_get_drvdata(sdio->func) : NULL;
	int access = (size & 0x03) ? MORSE_CONFIG_ACCESS_1BYTE : MORSE_CONFIG_ACCESS_4BYTE;
	struct sdio_func *func_to_use;

	if (!mors) {
		ret = -EINVAL;
		goto exit;
	}

	func_to_use = morse_sdio_get_func(sdio, address, size, access);
	if (!func_to_use) {
		ret = -EIO;
		goto exit;
	}

	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_BULK_WRITE, func_to_use->num, address, size);
	address &= 0x0000FFFF;	/* remove base and keep offset */
	if (access == MORSE_CONFIG_ACCESS_4BYTE) {
		if (unlikely(!IS_ALIGNED((uintptr_t)data, mors->bus_ops->bulk_alignment))) {
			MORSE_ERR_RATELIMITED(mors, "Bulk write data is not aligned to %u bytes\n",
					mors->bus_ops->bulk_alignment);
			ret = -EBADE;
			goto exit;
		}

		/* Use ex write */
		ret = sdio_memcpy_toio(func_to_use, address, data, size);

		if (ret) {
			sdio_log_err(sdio, "memcpy_toio", func_to_use->num, address, size, ret);
			goto exit;
		}
	} else {
		int i;

		for (i = 0; i < size; i++) {
			sdio_writeb(func_to_use, data[i], address + i, (int *)&ret);
			if (ret) {
				sdio_log_err(sdio, "writeb", func_to_use->num,
					       address + i, 1, ret);
				goto exit;
			}
		}
	}
	ret = size;
exit:
	return ret;
}

static void morse_sdio_claim_host(struct morse *mors)
{
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;
	struct sdio_func *func = sdio->func;

	sdio_claim_host(func);
}

static void morse_sdio_release_host(struct morse *mors)
{
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;
	struct sdio_func *func = sdio->func;

	sdio_release_host(func);
}

static int morse_sdio_mem_read(struct morse_sdio *sdio, u32 address, u8 *data, ssize_t size)
{
	ssize_t ret = 0;
	struct morse *mors = sdio->func ? sdio_get_drvdata(sdio->func) : NULL;
	int access = (size & 0x03) ? MORSE_CONFIG_ACCESS_1BYTE : MORSE_CONFIG_ACCESS_4BYTE;
	struct sdio_func *func_to_use;

	if (!mors) {
		ret = -EINVAL;
		goto exit;
	}

	func_to_use = morse_sdio_get_func(sdio, address, size, access);
	if (!func_to_use) {
		ret = -EIO;
		goto exit;
	}

	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_BULK_READ, func_to_use->num, address, size);
	address &= 0x0000FFFF;	/* remove base and keep offset */
	if (access == MORSE_CONFIG_ACCESS_4BYTE) {
		if (unlikely(!IS_ALIGNED((uintptr_t)data, mors->bus_ops->bulk_alignment))) {
			MORSE_ERR_RATELIMITED(mors, "Bulk read buffer is not aligned to %u bytes\n",
					mors->bus_ops->bulk_alignment);
			ret = -EBADE;
			goto exit;
		}

		ret = sdio_memcpy_fromio(func_to_use, data, address, size);
		if (ret) {
			sdio_log_err(sdio, "memcpy_fromio", func_to_use->num, address, size, ret);
			goto exit;
		}

		/* Observed sometimes that SDIO read repeats the first 4-bytes word twice,
		 * overwriting second word (hence, tail will be overwritten with 'sync' byte). When
		 * this happens, reading will fetch the correct word.
		 * NB: if repeated again, pass it anyway and upper layers will handle it
		 */
		if (size >= 8 && mors->cfg->bus_double_read &&
			memcmp(data, data + 4, 4) == 0) {
			/* sdio memcpy repeats first word. Try one more time before passing up */
			sdio_memcpy_fromio(func_to_use, data, address, 8);
		}
	} else {
		int i;

		for (i = 0; i < size; i++) {
			data[i] = sdio_readb(func_to_use, address + i, (int *)&ret);
			if (ret) {
				sdio_log_err(sdio, "readb", func_to_use->num,
					       address + i, 1, ret);
				goto exit;
			}
		}
	}
	ret = size;
exit:
	return ret;
}

static int morse_sdio_dm_write(struct morse *mors, u32 address, const u8 *data, int len)
{
	int ret = 0;
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;
	int remaining = len;
	int offset = 0;

	if (WARN_ON(len < 0))
		return -EINVAL;

	while (remaining > 0) {
		/*
		 * We can only write up to the end of a single window in
		 * each write operation.
		 */
		u32 window_end = (address + offset) | ~MORSE_SDIO_RW_ADDR_BOUNDARY_MASK;

		len = min(remaining, (int)(window_end + 1 - address - offset));
		ret = morse_sdio_mem_write(sdio, address + offset, (u8 *)(data + offset), len);
		if (ret != len)
			goto err;

		offset += len;
		MORSE_WARN_ON(FEATURE_ID_SDIO, len > remaining);
		remaining -= len;
	}

	return 0;

err:
	return -EIO;
}

static int morse_sdio_dm_read(struct morse *mors, u32 address, u8 *data, int len)
{
	int ret = 0;
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;
	int remaining = len;
	int offset = 0;

	if (WARN_ON(len < 0))
		return -EINVAL;

	MORSE_WARN_ON(FEATURE_ID_SDIO, len % 4);

	while (remaining > 0) {
		/*
		 * We can only read up to the end of a single window in
		 * each read operation.
		 */
		u32 window_end = (address + offset) | ~MORSE_SDIO_RW_ADDR_BOUNDARY_MASK;

		len = min(remaining, (int)(window_end + 1 - address - offset));
		ret = morse_sdio_mem_read(sdio, address + offset, data + offset, len);
		if (ret != len)
			goto err;

		offset += len;
		MORSE_WARN_ON(FEATURE_ID_SDIO, len > remaining);
		remaining -= len;
	}

	return 0;

err:
	return -EIO;
}

static int morse_sdio_reg32_write(struct morse *mors, u32 address, u32 val)
{
	ssize_t ret = 0;
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;

	ret = morse_sdio_regl_write(sdio, address, val);
	if (ret == sizeof(val))
		return 0;

	return -EIO;
}

static int morse_sdio_reg32_read(struct morse *mors, u32 address, u32 *val)
{
	ssize_t ret = 0;
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;

	ret = morse_sdio_regl_read(sdio, address, val);
	if (ret == sizeof(*val)) {
		*val = le32_to_cpup((__le32 *)val);
		return 0;
	}
	return -EIO;
}

/**
 * MM-5188 : Set the sdio clk to lowest 150KHz when disabling the sdio. And resume the sdio clk
 * when enabling it.
 *
 * When the chip enters into sleep then MM input SDIO-CLK pad is not going into high-z and the
 * sdio-clk from driver is consistently running in high speed. As a result it leaks more IO
 * current. Reducing sdio-clk will reduce the IO leakage.
 **/
static void morse_sdio_clk_freq_switch(struct morse *mors, u64 sdio_clk_hz)
{
	int ret;
	static const char *const envp[] = { "HOME=/", NULL };
	char cmd[128];

	char *const argv[] = { "/bin/sh", "-c", cmd, NULL };

	if (strlen(sdio_clk_debugfs) <= MIN_STRLEN_SDIO_CLK_PATH) {
		MORSE_SDIO_DBG(mors, "SDIO clock switching not supported, Skip.\n");

		return;
	}
	snprintf(cmd, sizeof(cmd), "echo %lld > %s", sdio_clk_hz, sdio_clk_debugfs);

	ret = call_usermodehelper(argv[0], (char **)argv, (char **)envp, UMH_WAIT_PROC);

	if (ret)
		MORSE_SDIO_ERR(mors, "%s: Failed to switch SDIO-CLK to %lldHz (errno=%d)\n",
			       __func__, sdio_clk_hz, ret);
	else
		MORSE_SDIO_DBG(mors, "%s: SDIO-CLK switched to %lldHz\n", __func__, sdio_clk_hz);
}

static void morse_sdio_bus_enable(struct morse *mors, bool enable)
{
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;
	struct sdio_func *func = sdio->func;
	struct mmc_host *host = func->card->host;

	sdio_claim_host(func);
	bus_trace_log(&sdio->trace, BUS_TRACE_EVENT_ID_BUS_EN, func->num, 0, enable);

	if (enable) {
		/* No need to do anything special to re-enable the sdio bus. This will happen
		 * automatically when a read/write is attempted and sdio->bulk_addr_base == 0.
		 */
		sdio->enabled = true;
		host->ops->enable_sdio_irq(host, 1);
		MORSE_SDIO_DBG(mors, "%s: enabling bus\n", __func__);

		/* Make sure the card will not be powered off by runtime PM */
		pm_runtime_get_sync(&func->dev);
	} else {
		host->ops->enable_sdio_irq(host, 0);
		morse_sdio_reset_base_address(sdio);
		sdio->enabled = false;
		MORSE_SDIO_DBG(mors, "%s: disabling bus\n", __func__);

		/* Let runtime PM know the card is powered off */
		pm_runtime_put_sync(&func->dev);
	}

	sdio_release_host(func);
	morse_sdio_clk_freq_switch(mors, (enable) ? FAST_SDIO_CLK_HZ : SLOW_SDIO_CLK_HZ);
}

static int morse_sdio_reset(int reset_pin, struct sdio_func *func)
{
	int ret = 0;
	struct mmc_card *card = func->card;

	/* reset the adapter */
	sdio_claim_host(func);
	sdio_disable_func(func);
	sdio_release_host(func);

	/* Let runtime PM know the card is powered off */
	pm_runtime_put(&card->dev);

	morse_hw_reset(reset_pin);
	mdelay(20);

	sdio_claim_host(func);
	sdio_disable_func(func);
#if KERNEL_VERSION(5, 18, 0) > LINUX_VERSION_CODE
	mmc_hw_reset(func->card->host);
#else
	mmc_hw_reset(func->card);
#endif
	sdio_enable_func(func);
	sdio_release_host(func);

	return ret;
}

static int morse_sdio_bus_reset(struct morse *mors)
{
	int ret = 0;
	struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;
	struct sdio_func *func = sdio->func;

	morse_sdio_remove(func);

	return ret;
}

static void morse_sdio_config_burst_mode(struct morse *mors, bool enable_burst)
{
	u8 burst_mode = (enable_burst) ? SDIO_WORD_BURST_SIZE_16 : SDIO_WORD_BURST_DISABLE;

	if (mors->cfg->enable_sdio_burst_mode)
		mors->cfg->enable_sdio_burst_mode(mors, burst_mode);
}

static const struct morse_bus_ops morse_sdio_ops = {
	.dm_read = morse_sdio_dm_read,
	.dm_write = morse_sdio_dm_write,
	.reg32_read = morse_sdio_reg32_read,
	.reg32_write = morse_sdio_reg32_write,
	.set_bus_enable = morse_sdio_bus_enable,
	.claim = morse_sdio_claim_host,
	.release = morse_sdio_release_host,
	.reset = morse_sdio_bus_reset,
	.config_burst_mode = morse_sdio_config_burst_mode,
	.set_irq = morse_sdio_set_irq,
	.bulk_alignment = MORSE_SDIO_ALIGNMENT
};

static int morse_sdio_enable(struct morse_sdio *sdio)
{
	int ret;
	struct sdio_func *func = sdio->func;
	struct morse *mors = sdio_get_drvdata(func);

	sdio_claim_host(func);
	ret = sdio_enable_func(func);
	if (ret)
		MORSE_SDIO_ERR(mors, "sdio_enable_func failed: %d\n", ret);
	sdio_release_host(func);
	return ret;
}

static void morse_sdio_release(struct morse_sdio *sdio)
{
	struct sdio_func *func = sdio->func;

	sdio_claim_host(func);
	sdio_disable_func(func);
	sdio_release_host(func);
}

static const struct of_device_id morse_of_match_table[] = {
	{.compatible = "morse,mm6104", },	/* DEPRECATED */
	{.compatible = "morse,mm610x", },
	{ },
};

static int morse_sdio_probe(struct sdio_func *func, const struct sdio_device_id *id)
{
	int ret = 0;
	u32 chip_id;
	struct morse *mors = NULL;
	struct morse_sdio *sdio;
	struct device *dev = &func->dev;
	bool mac_registered = false;
	bool irq_enabled = false;
	bool sdio_enabled = false;
	bool if_initiated = false;
	const bool reset_hw = false;
	bool attach = false;

	BUILD_BUG_ON_MSG(!IS_POWER_OF_TWO(MORSE_SDIO_ALIGNMENT),
			"SDIO bulk alignment must be a multiple of two");
	BUILD_BUG_ON_MSG((MORSE_SDIO_ALIGNMENT - 2) > MAX_PAGER_HOST_SKB_ALIGNMENT_PAD,
			"SDIO bulk alignment is too large for the firmware");

	dev_dbg(dev,
		"sdio new func %d vendor 0x%x device 0x%x block 0x%x/0x%x\n",
		func->num, func->vendor, func->device, func->max_blksize, func->cur_blksize);

	/* Consume func num 1 but don't do anything with it. */
	if (func->num == 1)
		return 0;

	/* Ignore anything but func 2 */
	if (func->num != 2)
		return -ENODEV;

	mors = morse_mac_create(sizeof(*sdio), dev);
	if (!mors) {
		dev_err(dev, "morse_mac_create failed\n");
		ret = -ENOMEM;
		goto err_exit;
	}

	sdio = (struct morse_sdio *)mors->drv_priv;
	sdio->func = func;
	sdio->id = id;
	sdio->enabled = true;
	bus_trace_init(&sdio->trace);
	morse_sdio_reset_base_address(sdio);

	mors->bus_ops = &morse_sdio_ops;
	mors->bus_type = MORSE_HOST_BUS_TYPE_SDIO;

	sdio_set_drvdata(func, mors);

	ret = morse_sdio_enable(sdio);
	if (ret) {
		MORSE_SDIO_ERR(mors, "morse_sdio_enable failed: %d\n", ret);
		goto err_exit;
	}
	sdio_enabled = true;

	/* Unlike SPI and USB, SDIO already knows the chip_id from the SDIO Function.
	 * Use that to initialise mors->cfg
	 */
	ret = morse_chip_cfg_init(mors, func->device);
	if (ret) {
		MORSE_SDIO_ERR(mors, "morse_chip_cfg_init failed: %d\n", ret);
		goto err_exit;
	}

	/* Verify the above chip_id matches the one read directly from the chip */
	morse_claim_bus(mors);
	ret = morse_reg32_read(mors, MORSE_REG_CHIP_ID(mors), &chip_id);
	morse_release_bus(mors);
	if (ret || chip_id != mors->chip_id) {
		MORSE_SDIO_ERR(mors, "Chip ID read failed: %d\n", ret);
		goto err_exit;
	}
	MORSE_SDIO_INFO(mors, "Morse Micro SDIO device found, chip ID=0x%04x\n", mors->chip_id);

	/* setting gpio pin configs from device tree */
	if (morse_of_probe(dev, mors->cfg, morse_of_match_table) < 0)
		goto err_exit;

	/* Digital reset the chip now if external (host) xtal initialisation is required */
	if (enable_ext_xtal_init) {
		MORSE_DBG(mors, "Resetting chip early for external xtal init");
		mors->cfg->digital_reset(mors);
	}

	morse_sdio_config_burst_mode(mors, true);

	mors->board_serial = serial;
	MORSE_SDIO_INFO(mors, "Board serial: %s\n", mors->board_serial);

	/* OTP BXW check is done only for MM610x */
	if (enable_otp_check && !is_otp_xtal_wait_supported(mors)) {
		MORSE_SDIO_ERR(mors, "OTP check failed\n");
		ret = -EIO;
		goto err_exit;
	}

	ret = morse_firmware_prepare_and_init(mors, reset_hw, morse_hw_should_reattach());
	if (ret == -EALREADY)
		attach = true;
	else if (ret)
		goto err_exit;

	if (morse_test_mode_is_interactive(test_mode)) {
		mors->chip_wq = create_singlethread_workqueue("MorseChipIfWorkQ");
		if (!mors->chip_wq) {
			MORSE_SDIO_ERR(mors,
				       "create_singlethread_workqueue(MorseChipIfWorkQ) failed\n");
			ret = -ENOMEM;
			goto err_exit;
		}
		mors->net_wq = create_singlethread_workqueue("MorseNetWorkQ");
		if (!mors->net_wq) {
			MORSE_SDIO_ERR(mors,
				       "create_singlethread_workqueue(MorseNetWorkQ) failed\n");
			ret = -ENOMEM;
			goto err_exit;
		}

		ret = mors->cfg->ops->init(mors);
		if (ret) {
			MORSE_SDIO_ERR(mors, "chip_if_init failed: %d\n", ret);
			goto err_exit;
		}
		if_initiated = true;

		ret = morse_firmware_parse_extended_host_table(mors);
		if (ret) {
			MORSE_SDIO_ERR(mors, "failed to parse extended host table: %d\n", ret);
			goto err_exit;
		}
	}

	/* Enable SDIO interrupts before callng ieee80211_register_hw() or morse_wiphy_register */
	ret = morse_sdio_enable_irq(sdio);
	if (ret) {
		MORSE_SDIO_ERR(mors, "morse_sdio_enable_irq failed: %d\n", ret);
		goto err_exit;
	}
	irq_enabled = true;

	if (morse_test_mode_is_interactive(test_mode)) {
		ret = morse_mac_register(mors);
		if (ret) {
			MORSE_SDIO_ERR(mors, "morse_mac_register failed: %d\n", ret);
			goto err_exit;
		}
		mac_registered = true;
	}

	if (attach)
		ret = morse_hw_attach(mors);

	if (ret)
		goto err_exit;

#ifdef CONFIG_MORSE_ENABLE_TEST_MODES
	if (test_mode == MORSE_CONFIG_TEST_MODE_BUS)
		morse_bus_test(mors, "SDIO");

	if (test_mode == MORSE_CONFIG_TEST_MODE_BUS_PROFILE) {
		morse_bus_throughput_profiler(mors);
		morse_sdio_disable_irq(sdio);
	}
#endif

	/* Initialize PM runtime.
	 * Set the power auto-suspend delay. If 'delay' is negative then runtime suspends are
	 * prevented. If 'use auto-suspend' is set, pm_runtime_get_sync may be called which
	 * will put the device on pm_runtime_idle.
	 **/
	pm_runtime_set_autosuspend_delay(&func->dev, PM_RUNTIME_AUTOSUSPEND_DELAY_MS);
	pm_runtime_use_autosuspend(&func->dev);
	pm_runtime_enable(&func->dev);
	pm_runtime_get_sync(&func->dev);

#ifdef CONFIG_MORSE_USER_ACCESS
	morse_uaccess = uaccess_alloc();
	if (IS_ERR(morse_uaccess)) {
		MORSE_PR_ERR(FEATURE_ID_SDIO, "uaccess_alloc() failed\n");
		ret = PTR_ERR(morse_uaccess);
		goto err_exit;
	}

	ret = uaccess_init(morse_uaccess);
	if (ret) {
		MORSE_PR_ERR(FEATURE_ID_SDIO, "uaccess_init() failed: %d\n", ret);
		goto err_exit;
	}

	ret = uaccess_device_register(mors, morse_uaccess, &func->dev);
	if (ret) {
		MORSE_SDIO_ERR(mors, "uaccess_device_register() failed: %d\n", ret);
		goto err_exit;
	}
#endif

	/* morse_buffers_test(mors); */
	return ret;

err_exit:
#ifdef CONFIG_MORSE_USER_ACCESS
	if (morse_uaccess) {
		uaccess_cleanup(morse_uaccess);
		morse_uaccess = NULL;
	}
	if (mac_registered)
		morse_mac_unregister(mors);
#endif
	if (irq_enabled)
		morse_sdio_disable_irq(sdio);
	if (sdio_enabled)
		morse_sdio_release(sdio);
	if (if_initiated)
		mors->cfg->ops->finish(mors);
	if (mors && mors->net_wq) {
		flush_workqueue(mors->net_wq);
		destroy_workqueue(mors->net_wq);
	}
	if (mors && mors->chip_wq) {
		flush_workqueue(mors->chip_wq);
		destroy_workqueue(mors->chip_wq);
	}
	if (mors)
		morse_mac_destroy(mors);
	pr_err("%s failed. The driver has not been loaded!\n", __func__);
	return ret;
}

static void morse_sdio_remove(struct sdio_func *func)
{
	struct morse *mors = sdio_get_drvdata(func);
	int ret = 0;
	bool reattach_hw = morse_hw_should_reattach();
	bool is_hw_detached = reattach_hw;

	dev_info(&func->dev, "sdio removed func %d vendor 0x%x device 0x%x\n",
		 func->num, func->vendor, func->device);

	if (mors) {
		struct morse_sdio *sdio = (struct morse_sdio *)mors->drv_priv;

#ifdef CONFIG_MORSE_USER_ACCESS
		uaccess_device_unregister(mors);
		uaccess_cleanup(morse_uaccess);
#endif

		if (morse_test_mode_is_interactive(test_mode)) {
			morse_mac_unregister(mors);
			morse_sdio_disable_irq(sdio);
			mors->cfg->ops->finish(mors);
			flush_workqueue(mors->chip_wq);
			destroy_workqueue(mors->chip_wq);
			flush_workqueue(mors->net_wq);
			destroy_workqueue(mors->net_wq);
			if (reattach_hw) {
				ret = morse_hw_detach(mors);
				if (ret)
					is_hw_detached = false;
			}
		} else {
			morse_sdio_disable_irq(sdio);
		}

		morse_sdio_release(sdio);
		morse_mac_destroy(mors);

		/* Reset HW for a cleaner restart */
		sdio_set_drvdata(func, NULL);
		if (!is_hw_detached)
			morse_sdio_reset(mors->cfg->mm_reset_gpio, func);
	}
}

static const struct sdio_device_id morse_sdio_devices[] = {
	{SDIO_DEVICE(MORSE_SDIO_VENDOR_ID, SDIO_ANY_ID)},
	{ },
};

MODULE_DEVICE_TABLE(sdio, morse_sdio_devices);

static struct sdio_driver morse_sdio_driver = {
	.name = "morse_sdio",
	.id_table = morse_sdio_devices,
	.probe = morse_sdio_probe,
	.remove = morse_sdio_remove,
};

int __init morse_sdio_init(void)
{
	int ret;

	ret = sdio_register_driver(&morse_sdio_driver);
	if (ret)
		MORSE_PR_ERR(FEATURE_ID_SDIO, "sdio_register_driver() failed: %d\n", ret);
	return ret;
}

void morse_sdio_exit(void)
{
	sdio_unregister_driver(&morse_sdio_driver);
}
