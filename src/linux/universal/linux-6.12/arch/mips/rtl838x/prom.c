// SPDX-License-Identifier: GPL-2.0-only
/*
 * prom.c
 * Early intialization code for the Realtek RTL838X SoC
 *
 * based on the original BSP by
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 * Copyright (C) 2020 B. Koblitz
 *
 */

#include <asm/fw/fw.h>
#include <asm/mips-cps.h>
#include <asm/prom.h>
#include <asm/smp-ops.h>
#include <linux/smp.h>

#include <mach-rtl83xx.h>

struct rtl83xx_soc_info soc_info;
const void *fdt;

#define RTL9310_MIPSIA_L2SIZE_OFFSET (4)
#define RTL9310_MIPSIA_L2SIZE_MASK (0x0f)
#define RTL9310_MIPSIA_L2_LINESIZE_0 (0x0)
#define RTL9310_MIPSIA_L2_LINESIZE_256 (0x7)

#ifdef CONFIG_MIPS_GIC
extern void init_l2_cache(void);
static unsigned int l2_linesize;
static void __init rtl9310_l2cache_init(void)
{
	unsigned long config2;
	config2 = read_c0_config2();
	l2_linesize = (config2 >> RTL9310_MIPSIA_L2SIZE_OFFSET) &
		      RTL9310_MIPSIA_L2SIZE_MASK;
	pr_info("L2 linesize is %d\n", 1 << l2_linesize);
	/*if enable l2_bypass mode, linesize will be 0       */
	/*if arch not implement L2cache, linesize will be 0  */
	if (RTL9310_MIPSIA_L2_LINESIZE_0 < l2_linesize &&
	    l2_linesize <=
		    RTL9310_MIPSIA_L2_LINESIZE_256) { //Scache linesize >0 and <=256 (B)
		init_l2_cache();
	}
	config2 = read_c0_config2();
	l2_linesize = (config2 >> RTL9310_MIPSIA_L2SIZE_OFFSET) &
		      RTL9310_MIPSIA_L2SIZE_MASK;
	pr_info("L2 linesize is %d\n", 1 << l2_linesize);
}
#endif

static char soc_name[16];
static char rtl83xx_system_type[32];

#ifdef CONFIG_MIPS_MT_SMP

extern const struct plat_smp_ops vsmp_smp_ops;
static struct plat_smp_ops rtlops;

static void rtlsmp_init_secondary(void)
{
	/*
	 * Enable all CPU interrupts, as everything is managed by the external controller.
	 * TODO: Standard vsmp_init_secondary() has special treatment for Malta if external
	 * GIC is available. Maybe we need this too.
	 */
	if (mips_gic_present())
		pr_warn("%s: GIC present. Maybe interrupt enabling required.\n", __func__);
	else
		set_c0_status(ST0_IM);
}

static void rtlsmp_finish(void)
{
	/* These devices are low on resources. There might be the chance that CEVT_R4K is
	 * not enabled in kernel build. Nevertheless the timer and interrupt 7 might be
	 * active by default after startup of secondary VPEs. With no registered handler
	 * that leads to continuous unhandeled interrupts. Disable it but keep the counter
	 * running so it can still be used as an entropy source.
	 */
	if (!IS_ENABLED(CONFIG_CEVT_R4K)) {
		write_c0_status(read_c0_status() & ~CAUSEF_IP7);
		write_c0_compare(read_c0_count() - 1);
	}

	local_irq_enable();
}

static int rtlsmp_register(void)
{
	if (!cpu_has_mipsmt)
		return 1;

	rtlops = vsmp_smp_ops;
	rtlops.init_secondary = rtlsmp_init_secondary;
	rtlops.smp_finish = rtlsmp_finish;
	register_smp_ops(&rtlops);

	return 0;
}

#else /* !CONFIG_MIPS_MT_SMP */

#define rtlsmp_register() (1)

#endif

void __init device_tree_init(void)
{
	if (!fdt_check_header(&__appended_dtb)) {
		fdt = &__appended_dtb;
		pr_info("Using appended Device Tree.\n");
	}
	initial_boot_params = (void *)fdt;
	unflatten_and_copy_device_tree();

	/* delay cpc & smp probing to allow devicetree access */
	mips_cpc_probe();

	if (!register_cps_smp_ops())
		return;

	if (!rtlsmp_register())
		return;

	register_up_smp_ops();
}

const char *get_system_type(void)
{
	return rtl83xx_system_type;
}

static uint32_t __init read_model_name(void)
{
	uint32_t model, id;

	model = sw_r32(RTL838X_MODEL_NAME_INFO);
	id = model >> 16 & 0xffff;
	if ((id >= 0x8380 && id <= 0x8382) || id == 0x8330 || id == 0x8332) {
		soc_info.id = id;
		soc_info.family = RTL8380_FAMILY_ID;
		return model;
	}

	model = sw_r32(RTL839X_MODEL_NAME_INFO);
	id = model >> 16 & 0xffff;
	if ((id >= 0x8391 && id <= 0x8396) || (id >= 0x8351 && id <= 0x8353)) {
		soc_info.id = id;
		soc_info.family = RTL8390_FAMILY_ID;
		return model;
	}

	model = sw_r32(RTL93XX_MODEL_NAME_INFO);
	id = model >> 16 & 0xffff;
	if (id >= 0x9301 && id <= 0x9303) {
		soc_info.id = id;
		soc_info.family = RTL9300_FAMILY_ID;
		soc_info.revision = model & 0xf;
		return model;
	} else if (id >= 0x9311 && id <= 0x9313) {
		soc_info.id = id;
		soc_info.family = RTL9310_FAMILY_ID;
		soc_info.revision = model & 0xf;
		return model;
	}

	return 0;
}

static void __init parse_model_name(uint32_t model)
{
	int val, offset, num_chars, pos;
	char suffix[5] = {};

	if (soc_info.family == RTL9300_FAMILY_ID ||
	    soc_info.family == RTL9310_FAMILY_ID) {
		/*
		 * RTL93xx seems to have a flag for engineering samples
		 * instead of a third character.
		 */
		num_chars = 2;
	} else {
		num_chars = 3;
	}

	for (pos = 0; pos < num_chars; pos++) {
		offset = 11 - pos * 5;
		val = (model & (0x1f << offset)) >> offset;

		if (val == 0 || val > 24)
			break;

		suffix[pos] = 'A' + (val - 1);
	}

	if (num_chars == 2 && (model & 0x30)) {
		suffix[pos] = 'E';
		suffix[pos+1] = 'S';
		pos += 2;
	}

	if (pos >= 2 && suffix[pos-2] == 'E' && suffix[pos-1] == 'S') {
		soc_info.testchip = true;
	}

	snprintf(soc_name, sizeof(soc_name), "RTL%04X%s",
		 soc_info.id, suffix);

	soc_info.name = soc_name;
}

static void __init read_chip_info(void)
{
	uint32_t val = 0;

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		sw_w32(0x3, RTL838X_INT_RW_CTRL);
		sw_w32(0xa << 28, RTL838X_CHIP_INFO);
		val = sw_r32(RTL838X_CHIP_INFO);
		soc_info.revision = (val >> 16) & 0x1f;
		break;

	case RTL8390_FAMILY_ID:
		sw_w32(0xa << 28, RTL839X_CHIP_INFO);
		val = sw_r32(RTL839X_CHIP_INFO);
		soc_info.revision = (val >> 16) & 0x1f;
		break;

	case RTL9300_FAMILY_ID:
	case RTL9310_FAMILY_ID:
		sw_w32(0xa << 16, RTL93XX_CHIP_INFO);
		val = sw_r32(RTL93XX_CHIP_INFO);
		break;
	}

	soc_info.cpu = val & 0xffff;
}

static void __init rtl83xx_set_system_type(void) {
	char revision = '?';

	if (soc_info.revision > 0 && soc_info.revision <= 24)
		revision = 'A' + (soc_info.revision - 1);

	snprintf(rtl83xx_system_type, sizeof(rtl83xx_system_type),
		 "Realtek %s rev %c (%04X)", soc_info.name, revision, soc_info.cpu);
}

#ifdef CONFIG_EARLY_PRINTK
#define rtl838x_r8(reg) __raw_readb(reg)
#define rtl838x_w8(val, reg) __raw_writeb(val, reg)

void prom_putchar(char c)
{
	unsigned int retry = 0;

	do {
		if (retry++ >= 30000) {
			/* Reset Tx FIFO */
			rtl838x_w8(TXRST | CHAR_TRIGGER_14, UART0_FCR);
			return;
		}
	} while ((rtl838x_r8(UART0_LSR) & LSR_THRE) == TxCHAR_AVAIL);

	/* Send Character */
	rtl838x_w8(c, UART0_THR);
}
#endif


void __init prom_init(void)
{
	uint32_t model;
#ifdef CONFIG_MIPS_GIC
	rtl9310_l2cache_init();
#endif
	model = read_model_name();
	parse_model_name(model);
	read_chip_info();
	rtl83xx_set_system_type();

	pr_info("SoC Type: %s\n", get_system_type());

	/*
	 * fw_arg2 is be the pointer to the environment. Some devices (e.g. HP JG924A) hand
	 * over other than expected kernel boot arguments. Something like 0xfffdffff looks
	 * suspicous. Do extra cleanup for fw_init_cmdline() to avoid a hang during boot.
	 */
	if (fw_arg2 >= CKSEG2)
		fw_arg2 = 0;

	fw_init_cmdline();
}
