/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/firmware.h>
#include <linux/delay.h>

#include "hw.h"
#include "morse.h"
#include "firmware.h"
#include "mac.h"
#include "bus.h"
#include "debug.h"
#include "yaps.h"

#define MM8108_REG_HOST_MAGIC_VALUE		0xDEADBEEF
#define MM8108_REG_RESET_VALUE			0xDEAD

/* This should be at a fixed location for a family of chipset */
#define MM8108_REG_CHIP_ID			0x00002d20 /* Chip ID */

/* These can change but need to add them to hw_regs structure and dynamically attach it */
#define MM8108_REG_SDIO_DEVICE_ADDR		0x0000207C /* apps_hal_r_system_sdio_device*/
/* offset of apps_hal_rw_system_sdio_device_tl_burst_sel_fn2 */
#define MM8108_REG_SDIO_DEVICE_BURST_OFFSET	9

/* Generates IRQ to the target */
#define MM8108_REG_TRGR_BASE			0x00003c00 /* HostSync addr */
#define MM8108_REG_INT_BASE			0x00003c50 /* Hostsync reg*/
#define MM8108_REG_MSI				0x00004100 /* Clint */

#define MM8108_REG_MANIFEST_PTR_ADDRESS		0x00002d40 /* SW Manifest Pointer */
#define MM8108_REG_APPS_BOOT_ADDR		0x00002084 /* APPS core boot address */
#define MM8108_REG_RESET			0x000020AC /* Digital Reset */
#define MM8108_REG_AON_ADDR			0x00002114 /* system_ao_mem_in */
#define MM8108_REG_AON_LATCH_ADDR		0x00405020 /* radio_rf_ao_cfg_ao_latch */
#define MM8108_REG_AON_LATCH_MASK		0x1
#define MM8108_REG_AON_RESET_USB_VALUE		0x8
#define MM8108_APPS_MAC_DMEM_ADDR_START		0x00100000 /* DTCM */
#define MM8108_REG_GPIO_OUTPUT_EN_SET_ADDR	0x1360
#define MM8108_REG_GPIO_OUTPUT_EN_CLR_ADDR	0x1364
#define MM8108_REG_GPIO_OUTPUT_VALUE_SET_ADDR	0x1368
#define MM8108_REG_GPIO_OUTPUT_VALUE_CLR_ADDR	0x136c

/* PMU CTRL */
#define MM8108_REG_AON_PMU_CTRL_MASK            0x00000080
#define MM8108_REG_AON_PMU_CTRL_ADDR            0x00405020

/* Slow RC clock */
#define MM8108_REG_RC_CLK_POWER_OFF_ADDR	0x00405020
#define MM8108_REG_RC_CLK_POWER_OFF_MASK	0x00000040
#define MM8108_SLOW_RC_POWER_ON_DELAY_MS	2

#define MM8108_SPI_INTER_BLOCK_DELAY_BURST16_NS	4800
#define MM8108_SPI_INTER_BLOCK_DELAY_BURST8_NS	8000
#define MM8108_SPI_INTER_BLOCK_DELAY_BURST4_NS	15000
#define MM8108_SPI_INTER_BLOCK_DELAY_BURST2_NS	30000
#define MM8108_SPI_INTER_BLOCK_DELAY_BURST0_NS	58000

/* 64bit hardware clock registers (microsecond tick) */
#define MM8108_REG_APPS_CLINT_MTIME_MTIME_0_ADDR 0x4140
#define MM8108_REG_APPS_CLINT_MTIME_MTIME_1_ADDR 0x4144

/* Read OTP value */
#define MM8108_REG_OTPCTRL_PLDO			0x00004014
#define MM8108_REG_OTPCTRL_PENVDD2		0x00004010
#define MM8108_REG_OTPCTRL_PDSTB		0x00004018
#define MM8108_REG_OTPCTRL_PTM			0x0000401c
#define MM8108_REG_OTPCTRL_PCE			0x00004020
#define MM8108_REG_OTPCTRL_PA			0x00004034
#define MM8108_REG_OTPCTRL_PECCRDB		0x00004048
#define MM8108_REG_OTPCTRL_ACTION_AUTO_RD_START	0x0000400c
#define MM8108_REG_OTPCTRL_PDOUT		0x00004040

/* MM810x OTP defines */
#define MM810x_OTP_BOARD_TYPE_BANK_NUM		26
#define MM810x_OTP_BOARD_TYPE_MASK		GENMASK(15, 0)
#define MM810x_OTP_COUNTRY_CODE_BANK_NUM	25
#define MM810x_OTP_COUNTRY_CODE_MASK		GENMASK(15, 0)

/* MM810X board type max value */
#define MM810x_BOARD_TYPE_MAX_VALUE		(MM810x_OTP_BOARD_TYPE_MASK - 1)

#define MM8108_FW_BASE				"mm8108"

static void mm810x_otp_power_up(struct morse *mors)
{
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PENVDD2, 1);
	udelay(2);

	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PLDO, 1);
	usleep_range(10, 20);

	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PDSTB, 1);
	udelay(3);
}

static void mm810x_otp_power_down(struct morse *mors)
{
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PDSTB, 0);
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PLDO, 0);
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PENVDD2, 0);
}

static void mm810x_otp_read_enable(struct morse *mors)
{
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PTM, 0);
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PCE, 1);
	usleep_range(10, 20);
}

static void mm810x_otp_read_disable(struct morse *mors)
{
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PCE, 0);
	udelay(1);
}

/**
 * mm810x_otp_read() - Read 32 bit value from selected OTP bank into the given buffer.
 * @mors: Morse object.
 * @bank_num: Selected OTP bank number.
 * @buf: Buffer to read the values into.
 * @ignore_ecc: Ignore error correcting bits when reading.
 *
 * @return: 0 if successful, -EIO if unsuccessful
 */
static int mm810x_otp_read(struct morse *mors, u8 bank_num, u32 *buf, u8 ignore_ecc)
{
	u32 auto_rd_start_tmp;
	u32 auto_rd_start = 1;
	int i;

	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PA, bank_num);
	morse_reg32_write(mors, MM8108_REG_OTPCTRL_PECCRDB, ignore_ecc);

	morse_reg32_read(mors, MM8108_REG_OTPCTRL_ACTION_AUTO_RD_START, &auto_rd_start_tmp);
	auto_rd_start_tmp &= 0xfffffffe;

	morse_reg32_write(mors, MM8108_REG_OTPCTRL_ACTION_AUTO_RD_START,
			  auto_rd_start | auto_rd_start_tmp);

	/* Attempt reading up to 5 times. */
	for (i = 0; i < 5 && auto_rd_start; i++) {
		usleep_range(15, 20);
		morse_reg32_read(mors, MM8108_REG_OTPCTRL_ACTION_AUTO_RD_START, &auto_rd_start_tmp);
		auto_rd_start = auto_rd_start_tmp & 0x1;
	}

	if (i == 5)
		return -EIO;

	morse_reg32_read(mors, MM8108_REG_OTPCTRL_PDOUT, buf);

	return 0;
}

static int mm810x_get_board_type(struct morse *mors)
{
	int board_type = 0;
	u32 otp_word = 0;
	int ret;

	morse_claim_bus(mors);
	mm810x_otp_power_up(mors);
	mm810x_otp_read_enable(mors);

	ret = mm810x_otp_read(mors, MM810x_OTP_BOARD_TYPE_BANK_NUM, &otp_word, 1);

	mm810x_otp_read_disable(mors);
	mm810x_otp_power_down(mors);
	morse_release_bus(mors);

	if (ret)
		return -EINVAL;

	board_type = otp_word & MM810x_OTP_BOARD_TYPE_MASK;

	return board_type;
}

static const char *mm810x_get_hw_version(u32 chip_id)
{
	switch (chip_id) {
	case MM8108B0_FPGA_ID:
		return "MM8108B0-FPGA";
	case MM8108B0_ID:
		return "MM8108B0";
	case MM8108B1_FPGA_ID:
		return "MM8108B1-FPGA";
	case MM8108B1_ID:
		return "MM8108B1";
	case MM8108B2_FPGA_ID:
		return "MM8108B2-FPGA";
	case MM8108B2_ID:
		return "MM8108B2";
	}
	return "unknown";
}

static char *mm810x_get_revision_string(u32 chip_id)
{
	u8 chip_rev = MORSE_DEVICE_GET_CHIP_REV(chip_id);

	switch (chip_rev) {
	case MM8108B0_REV:
		return MM8108B0_REV_STRING;
	case MM8108B1_REV:
		return MM8108B1_REV_STRING;
	case MM8108B2_REV:
		return MM8108B2_REV_STRING;
	default:
		return "??";
	}
}

static const char *mm810x_get_fw_variant_string(void)
{
	const char *fw_variant = "";

	if (is_fullmac_mode())
		fw_variant = MORSE_FW_FULLMAC_STRING;
	else if (is_thin_lmac_mode())
		fw_variant = MORSE_FW_THIN_LMAC_STRING;
	else if (is_virtual_sta_test_mode())
		fw_variant = MORSE_FW_VIRTUAL_STA_STRING;

	return fw_variant;
}

static char *mm810x_get_fw_path(u32 chip_id)
{
	/* Get all the strings required to construct the fw bin name */
	const char *revision_string = mm810x_get_revision_string(chip_id);
	const char *fw_variant_string = mm810x_get_fw_variant_string();

	return kasprintf(GFP_KERNEL,
			 MORSE_FW_DIR "/" MM8108_FW_BASE "%s%s" FW_ROM_LINKED_STRING MORSE_FW_EXT,
			 revision_string,
			 fw_variant_string);
}

static u8 mm810x_get_wakeup_delay_ms(u32 chip_id)
{
	/* MM8108 takes < 5ms to be active */
	return 10;
}

static u32 mm810x_get_burst_mode_inter_block_delay_ns(const u8 burst_mode)
{
	int ret;

	switch (burst_mode) {
	case SDIO_WORD_BURST_SIZE_16:
		ret = MM8108_SPI_INTER_BLOCK_DELAY_BURST16_NS;
		break;
	case SDIO_WORD_BURST_SIZE_8:
		ret = MM8108_SPI_INTER_BLOCK_DELAY_BURST8_NS;
		break;
	case SDIO_WORD_BURST_SIZE_4:
		ret = MM8108_SPI_INTER_BLOCK_DELAY_BURST4_NS;
		break;
	case SDIO_WORD_BURST_SIZE_2:
		ret = MM8108_SPI_INTER_BLOCK_DELAY_BURST2_NS;
		break;
	default:
		ret = MM8108_SPI_INTER_BLOCK_DELAY_BURST0_NS;
		break;
	}

	return ret;
}

static int mm810x_enable_burst_mode(struct morse *mors, const u8 burst_mode)
{
	u32 reg32_value;
	int ret = mm810x_get_burst_mode_inter_block_delay_ns(burst_mode);

	MORSE_WARN_ON(FEATURE_ID_DEFAULT, !mors);

	/* We should perform a read, modify & write here, since it is the safest option. */
	morse_claim_bus(mors);
	if (morse_reg32_read(mors, MM8108_REG_SDIO_DEVICE_ADDR, &reg32_value)) {
		ret = -EPERM;
		goto exit;
	}

	reg32_value &= ~(u32)(SDIO_WORD_BURST_MASK << MM8108_REG_SDIO_DEVICE_BURST_OFFSET);
	reg32_value |= (u32)(burst_mode << MM8108_REG_SDIO_DEVICE_BURST_OFFSET);

	MORSE_INFO(mors, "Setting Burst mode to %d Writing 0x%08X to the register\n",
		   burst_mode, reg32_value);

	if (morse_reg32_write(mors, MM8108_REG_SDIO_DEVICE_ADDR, reg32_value)) {
		ret = -EPERM;
		goto exit;
	}

exit:
	morse_release_bus(mors);

	if (ret < 0)
		MORSE_PR_ERR(FEATURE_ID_DEFAULT, "%s failed\n", __func__);

	return ret;
}

static int mm810x_enable_internal_slow_clock(struct morse *mors)
{
	u32 rc_clock_reg_value;
	int ret = 0;

	MORSE_INFO(mors, "Enabling internal slow clock\n");

	/* We should perform a read, clear the power off bit and write it back */
	ret = morse_reg32_read(mors, MM8108_REG_RC_CLK_POWER_OFF_ADDR, &rc_clock_reg_value);
	if (ret)
		goto exit;

	rc_clock_reg_value &= ~MM8108_REG_RC_CLK_POWER_OFF_MASK;
	ret = morse_reg32_write(mors, MM8108_REG_RC_CLK_POWER_OFF_ADDR, rc_clock_reg_value);
	if (ret)
		goto exit;

	ret = morse_hw_toggle_aon_latch(mors);
	if (ret)
		goto exit;

	/* Wait for the clock to turn on and settle */
	mdelay(MM8108_SLOW_RC_POWER_ON_DELAY_MS);
exit:
	if (ret)
		MORSE_ERR(mors, "%s failed\n", __func__);

	return ret;
}

static int mm810x_disable_pmu_ctrl_cpu(struct morse *mors)
{
	u32 aon_reg_value;
	int ret = 0;

	MORSE_INFO(mors, "Disabling PMU contol to CPU\n");

	ret = morse_reg32_read(mors, MM8108_REG_AON_PMU_CTRL_ADDR, &aon_reg_value);
	if (ret)
		goto exit;

	aon_reg_value &= ~MM8108_REG_AON_PMU_CTRL_MASK;
	ret = morse_reg32_write(mors, MM8108_REG_AON_PMU_CTRL_ADDR, aon_reg_value);
	if (ret)
		goto exit;

	ret = morse_hw_toggle_aon_latch(mors);
	if (ret)
		goto exit;

exit:
	if (ret)
		MORSE_ERR(mors, "%s failed\n", __func__);

	return ret;
}

static int mm810x_digital_reset(struct morse *mors)
{
	int ret = 0;

	morse_claim_bus(mors);

	/* This should be the first step in digital reset, do not reorder */
	mm810x_enable_internal_slow_clock(mors);

	/* Disable PMU control to CPU before digital reset */
	mm810x_disable_pmu_ctrl_cpu(mors);

	if (mors->bus_type == MORSE_HOST_BUS_TYPE_USB) {
#ifdef CONFIG_MORSE_USB
		ret = morse_usb_ndr_reset(mors);
#endif
		goto usb_reset;
	}

	if (MORSE_REG_RESET(mors) != 0)
		ret = morse_reg32_write(mors, MORSE_REG_RESET(mors), MORSE_REG_RESET_VALUE(mors));

usb_reset:
	/* SDIO needs some time after reset */
	if (sdio_reset_time > 0)
		msleep(sdio_reset_time);

	morse_release_bus(mors);

	if (!ret)
		mors->chip_was_reset = true;

	return ret;
}

static const char *get_slow_clock_mode_string(enum morse_cmd_slow_clock_mode mode)
{
	switch (mode) {
	case MORSE_CMD_SLOW_CLOCK_MODE_AUTO:
		return "Auto";
	case MORSE_CMD_SLOW_CLOCK_MODE_INTERNAL:
		return "Internal";
	default:
		break;
	}

	return "Unknown";
}

static int mm810x_set_slow_clock_mode(struct morse *mors, enum morse_cmd_slow_clock_mode mode)
{
	int ret = morse_cmd_set_slow_clock_mode(mors, mode);

	if (ret) {
		MORSE_ERR(mors, "Failed to set slow clock source");
		return ret;
	}

	MORSE_INFO(mors, "Slow clock source selection mode set to: %s",
		   get_slow_clock_mode_string(mode));

	return ret;
}

static int mm810x_read_encoded_country(struct morse *mors)
{
	u32 country_word = 0;
	u32 otp_word = 0;
	char country_0, country_1;
	int ret;

	morse_claim_bus(mors);
	mm810x_otp_power_up(mors);
	mm810x_otp_read_enable(mors);

	ret = mm810x_otp_read(mors, MM810x_OTP_COUNTRY_CODE_BANK_NUM, &otp_word, 1);

	mm810x_otp_read_disable(mors);
	mm810x_otp_power_down(mors);
	morse_release_bus(mors);

	if (ret)
		return -EINVAL;

	country_word = otp_word & MM810x_OTP_COUNTRY_CODE_MASK;
	/* If country code is not set, no need to proceed and farther */
	if (!country_word)
		return -ENOENT;

	country_0 = (char)(country_word & GENMASK(7, 0));
	country_1 = (char)(country_word >> 8);

	/* Only consider ASCII printable characters (character code 32-126/0x20-0x7E) as valid */
	if (country_0 < (char)0x20 || country_1 < (char)0x20 ||
	    country_0 > (char)0x7E || country_1 > (char)0x7E) {
		MORSE_INFO(mors, "Ignore OTP country code 0x%02X%02X (out of range)",
			country_0, country_1);
		return -ENOENT;
	}

	mors->country[0] = country_0;
	mors->country[1] = country_1;
	MORSE_INFO(mors, "Country code is retrieved from OTP: %s", mors->country);
	return 0;
}

static int mm810x_pre_coredump_hook(struct morse *mors, enum morse_coredump_method method)
{
	int ret = 0;

	if (method == COREDUMP_METHOD_USERSPACE_SCRIPT)
		return ret;
	/* We need disable SDIO tilelink bursting for register reads to work from the driver. */
	if (mors->bus_ops->config_burst_mode)
		mors->bus_ops->config_burst_mode(mors, false);

	return ret;
}

static int mm810x_post_coredump_hook(struct morse *mors, enum morse_coredump_method method)
{
	int ret = 0;

	if (method == COREDUMP_METHOD_USERSPACE_SCRIPT)
		return ret;

	if (mors->bus_ops->config_burst_mode)
		mors->bus_ops->config_burst_mode(mors, true);

	return ret;
}

static void mm810x_pre_firmware_ndr_hook(struct morse *mors)
{
	/* We need disable bursting for firmware download/init procedure */
	if (mors->bus_ops->config_burst_mode)
		mors->bus_ops->config_burst_mode(mors, false);
}

static void mm810x_post_firmware_ndr_hook(struct morse *mors)
{
	/* We are safe here to reenable bursting again, if supported */
	if (mors->bus_ops->config_burst_mode)
		mors->bus_ops->config_burst_mode(mors, true);
}

static int mm810x_gpio_enable_output(struct morse *mors, int pin_num, bool enable)
{
	int ret;
	u32 write_addr = enable ? MM8108_REG_GPIO_OUTPUT_EN_SET_ADDR
				: MM8108_REG_GPIO_OUTPUT_EN_CLR_ADDR;
	morse_claim_bus(mors);
	ret = morse_reg32_write(mors, write_addr, 0x1 << pin_num);
	morse_release_bus(mors);

	if (ret < 0)
		MORSE_WARN(mors, "Output enable on GPIO %d failed\n", pin_num);
	return ret;
}

static void mm810x_gpio_write_output(struct morse *mors, int pin_num, bool active)
{
	u32 write_addr = active ? MM8108_REG_GPIO_OUTPUT_VALUE_SET_ADDR
				: MM8108_REG_GPIO_OUTPUT_VALUE_CLR_ADDR;
	morse_claim_bus(mors);
	morse_reg32_write(mors, write_addr, 0x1 << pin_num);
	morse_release_bus(mors);
}

const struct morse_hw_regs mm8108_regs = {
	/* Register address maps */
	.irq_base_address = MM8108_REG_INT_BASE,
	.trgr_base_address = MM8108_REG_TRGR_BASE,

	/* Reset */
	.cpu_reset_address = MM8108_REG_RESET,
	.cpu_reset_value = MM8108_REG_RESET_VALUE,

	/* Pointer to manifest */
	.manifest_ptr_address = MM8108_REG_MANIFEST_PTR_ADDRESS,

	/* Trigger SWI */
	.msi_address = MM8108_REG_MSI,
	.msi_value = 0x1,
	/* Firmware */
	.magic_num_value = MM8108_REG_HOST_MAGIC_VALUE,

	/*
	 *  Don't set the clock enables to the cores before RAM is loaded,
	 *      otherwise you will have a bad time.
	 *      As MAC FW is being loaded, it will straight away attempt to read memory
	 *      hammering the Memory system, preventing the SDIO Controller from writing memory
	 */
	.early_clk_ctrl_value = 0,

	/* OTP data base address */
	/*
	 * TODO: MM-4868
	 * Not yet implemented for MM8108 so skip this for now
	 */
	.otp_data_base_address = 0,

	.pager_base_address = MM8108_APPS_MAC_DMEM_ADDR_START,

	/* AON registers */
	.aon_latch = MM8108_REG_AON_LATCH_ADDR,
	.aon_latch_mask = MM8108_REG_AON_LATCH_MASK,
	.aon_reset_usb_value = MM8108_REG_AON_RESET_USB_VALUE,
	.aon = MM8108_REG_AON_ADDR,
	.aon_count = 2,

	/* MTIME registers */
	.mtime_lower = MM8108_REG_APPS_CLINT_MTIME_MTIME_0_ADDR,
	.mtime_upper = MM8108_REG_APPS_CLINT_MTIME_MTIME_1_ADDR,

	/* hart0 boot address */
	.boot_address = MM8108_REG_APPS_BOOT_ADDR,
};

struct morse_hw_cfg mm8108_cfg = {
	.regs = &mm8108_regs,
	.chip_id_address = MM8108_REG_CHIP_ID,
	.ops = &morse_yaps_ops,
	.bus_double_read = false,
	.enable_short_bcn_as_dtim = true,
	.led_group.enable_led_support = true,
	.enable_sdio_burst_mode = mm810x_enable_burst_mode,
	.digital_reset = mm810x_digital_reset,
	.get_ps_wakeup_delay_ms = mm810x_get_wakeup_delay_ms,
	.get_hw_version = mm810x_get_hw_version,
	.get_fw_path = mm810x_get_fw_path,
	.set_slow_clock_mode = mm810x_set_slow_clock_mode,
	.pre_coredump_hook = mm810x_pre_coredump_hook,
	.post_coredump_hook = mm810x_post_coredump_hook,
	.pre_firmware_ndr = mm810x_pre_firmware_ndr_hook,
	.post_firmware_ndr = mm810x_post_firmware_ndr_hook,
	.gpio_enable_output = mm810x_gpio_enable_output,
	.gpio_write_output = mm810x_gpio_write_output,
	.get_board_type = mm810x_get_board_type,
	.board_type_max_value = MM810x_BOARD_TYPE_MAX_VALUE,
	.get_encoded_country = mm810x_read_encoded_country,
};

struct morse_chip_series mm81xx_chip_series = {
	.chip_id_address = MM8108_REG_CHIP_ID
};

/* B0 ROM_LINKED */
MODULE_FIRMWARE(MORSE_FW_DIR "/" MM8108_FW_BASE
				 MM8108B0_REV_STRING
				 FW_ROM_LINKED_STRING
				 MORSE_FW_EXT);

/* B1 ROM_LINKED */
MODULE_FIRMWARE(MORSE_FW_DIR "/" MM8108_FW_BASE
				 MM8108B1_REV_STRING
				 FW_ROM_LINKED_STRING
				 MORSE_FW_EXT);

/* B2 ROM_LINKED */
MODULE_FIRMWARE(MORSE_FW_DIR "/" MM8108_FW_BASE
				 MM8108B2_REV_STRING
				 FW_ROM_LINKED_STRING
				 MORSE_FW_EXT);

/* B0 Fullmac */
MODULE_FIRMWARE(MORSE_FW_DIR "/" MM8108_FW_BASE
				 MM8108B0_REV_STRING
				 MORSE_FW_FULLMAC_STRING
				 FW_ROM_LINKED_STRING
				 MORSE_FW_EXT);

/* B1 Fullmac */
MODULE_FIRMWARE(MORSE_FW_DIR "/" MM8108_FW_BASE
				 MM8108B1_REV_STRING
				 MORSE_FW_FULLMAC_STRING
				 FW_ROM_LINKED_STRING
				 MORSE_FW_EXT);

/* B2 Fullmac */
MODULE_FIRMWARE(MORSE_FW_DIR "/" MM8108_FW_BASE
				 MM8108B2_REV_STRING
				 MORSE_FW_FULLMAC_STRING
				 FW_ROM_LINKED_STRING
				 MORSE_FW_EXT);
