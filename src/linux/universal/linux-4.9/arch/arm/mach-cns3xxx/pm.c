/*
 * Copyright 2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include <mach/cns3xxx.h>
#include <mach/pm.h>
#include <linux/interrupt.h>
#ifdef CONFIG_SUSPEND
#include <linux/suspend.h>
#endif
#include "core.h"

//#define CNS_PMU_DEBUG

#ifdef CNS_PMU_DEBUG 
#include <linux/proc_fs.h>
#endif

#define WFI() \
asm volatile("mov r0, #0\n" \
	"mcr p15, 0, r0, c7, c10, 5\n" \
	"mcr p15, 0, r0, c7, c10, 4\n" \
	"mcr p15, 0, r0, c7, c0, 4\n");

void cns3xxx_pwr_clk_en(unsigned int block)
{
	u32 reg = __raw_readl(PM_CLK_GATE_REG);

	reg |= (block & PM_CLK_GATE_REG_MASK);
	__raw_writel(reg, PM_CLK_GATE_REG);
}
EXPORT_SYMBOL(cns3xxx_pwr_clk_en);



void cns3xxx_pwr_clk_dis(unsigned int block)
{
	u32 reg = __raw_readl(PM_CLK_GATE_REG);

	reg &= ~(block & PM_CLK_GATE_REG_MASK);
	__raw_writel(reg, PM_CLK_GATE_REG);
}
EXPORT_SYMBOL(cns3xxx_pwr_clk_dis);

void cns3xxx_pwr_power_up(unsigned int block)
{
	u32 reg = __raw_readl(PM_PLL_HM_PD_CTRL_REG);

	reg &= ~(block & CNS3XXX_PWR_PLL_ALL);
	__raw_writel(reg, PM_PLL_HM_PD_CTRL_REG);

	/* Wait for 300us for the PLL output clock locked. */
	udelay(300);
};
EXPORT_SYMBOL(cns3xxx_pwr_power_up);

void cns3xxx_pwr_power_down(unsigned int block)
{
	u32 reg = __raw_readl(PM_PLL_HM_PD_CTRL_REG);

	/* write '1' to power down */
	reg |= (block & CNS3XXX_PWR_PLL_ALL);
	__raw_writel(reg, PM_PLL_HM_PD_CTRL_REG);
};
EXPORT_SYMBOL(cns3xxx_pwr_power_down);

#define SPI_MEM_MAP_VALUE(reg_offset)		(*((u32 volatile *)(CNS3XXX_SSP_BASE_VIRT + reg_offset)))

static void cns3xxx_pwr_soft_rst_force(unsigned int block)
{
	unsigned int spi_block = CNS3XXX_PWR_SOFTWARE_RST(SPI_PCM_I2C);
	u32 reg = __raw_readl(PM_SOFT_RST_REG);

	/*
	 * bit 0, 28, 29 => program low to reset,
	 * the other else program low and then high
	 */
	if (block & 0x30000001) {
		/* If bootloader is booting from SPI, SPI interface must be enabled */
		reg &= ~(spi_block & PM_SOFT_RST_REG_MASK);
		reg |= (spi_block & PM_SOFT_RST_REG_MASK);
		mdelay(100); // wait SPI stable
		reg &= ~(block & PM_SOFT_RST_REG_MASK);
	} else {
		reg &= ~(block & PM_SOFT_RST_REG_MASK);
		__raw_writel(reg, PM_SOFT_RST_REG);
		reg |= (block & PM_SOFT_RST_REG_MASK);
	}

	__raw_writel(reg, PM_SOFT_RST_REG);
}
EXPORT_SYMBOL(cns3xxx_pwr_soft_rst_force);

void cns3xxx_pwr_soft_rst(unsigned int block)
{
	static unsigned int soft_reset;

	if (soft_reset & block) {
		/* SPI/I2C/GPIO use the same block, reset once. */
		return;
	} else {
		soft_reset |= block;
	}
	cns3xxx_pwr_soft_rst_force(block);
}
EXPORT_SYMBOL(cns3xxx_pwr_soft_rst);

void cns3xxx_restart(enum reboot_mode mode, const char *cmd)
{
	/*
	 * To reset, we hit the on-board reset register
	 * in the system FPGA.
	 */
	while(1)
	    cns3xxx_pwr_soft_rst_force(CNS3XXX_PWR_SOFTWARE_RST(GLOBAL));
}

/*
 * cns3xxx_cpu_clock - return CPU/L2 clock
 *  aclk: cpu clock/2
 *  hclk: cpu clock/4
 *  pclk: cpu clock/8
 */
int cns3xxx_cpu_clock(void)
{
	u32 reg = __raw_readl(PM_CLK_CTRL_REG);
	int cpu;
	int cpu_sel;
	int div_sel;

	cpu_sel = (reg >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf;
	div_sel = (reg >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3;

	cpu = (300 + ((cpu_sel / 3) * 100) + ((cpu_sel % 3) * 33)) >> div_sel;

	return cpu;
}
EXPORT_SYMBOL(cns3xxx_cpu_clock);

int get_cns3xxx_cpu_clock(void)
{
#define CPU_BASE 300
    int cpu, pll_cpu, cpu_sel, div_sel, cpu_grade;
    u32 reg = __raw_readl(PM_CLK_CTRL_REG);
    u32 misc = __raw_readl(MISC_CHIP_CONFIG_REG);
    cpu_sel = (reg >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf;
    div_sel = (reg >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3;
    cpu_grade = (misc >> 11) & 0x1;

	pll_cpu = CPU_BASE + ((cpu_sel/3) * 100) + ((cpu_sel %3) *33);
    cpu = (pll_cpu >> div_sel) >> cpu_grade;
	return cpu;
}

/*
 * cns3xxx_pwr_mode - change CPU power mode
 * @pwr_mode: CPU power mode
 */
void cns3xxx_pwr_mode(unsigned int pwr_mode)
{
	u32 data, addr;

	if (CNS3XXX_PWR_CPU_MODE_HIBERNATE < pwr_mode) { 
		return; 
	}

	addr = CNS3XXX_PM_BASE_VIRT + 0x014; /* PM_CLK_CTRL_REG */

	data = readl(addr);
    	data &= ~(0x7 << PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE);
    	data |= ((pwr_mode & 0x7)<<PM_CLK_CTRL_REG_OFFSET_CPU_PWR_MODE);
	writel(data, addr);
};
EXPORT_SYMBOL(cns3xxx_pwr_mode);

/*
 * void cns3xxx_pwr_lp_hs - enable lower power handshake
 * @dev: bitmap for device 
 */
static void cns3xxx_lp_hs(unsigned int dev)
{
	u32 data, addr;

	addr = CNS3XXX_PM_BASE_VIRT + 0x008; /* PM_HS_CFG_REG */
	data = readl(addr);
	data |= (PM_HS_CFG_REG_MASK_SUPPORT & dev);
	writel(data, addr);
}

/*
 * cns3xxx_pwr_change_pll_cpu - change PLL CPU frequency
 * @cpu_sel: PLL CPU frequency
 * @div_sel: divider
 *
 * This feature requires that 2nd core is in WFI mode and L2 cache is disabled
 * Before calling this function, please make sure that L2 cache is not in use.
 *
 * Procedure:
 * 	1. Set PLL_CPU_SEL
 * 	2. Set in DFS mode
 * 	3. disable all interrupt except interrupt ID-32 (clkscale_intr)
 * 	4. Let CPU enter into WFI state
 * 	5. Wait PMU to change PLL_CPU and divider and wake up CPU
 */
void cns3xxx_pwr_change_cpu_clock(unsigned int cpu_sel, unsigned int div_sel)
{
	int old_cpu, old_div;
	u32 data, addr;

	/* sanity check */
	if ((CNS3XXX_PWR_PLL_CPU_700MHZ < cpu_sel) 
		|| (CNS3XXX_PWR_CPU_CLK_DIV_BY4 < div_sel)) {
		printk("%s: incorrect parameter, cpu_sel:%d, div_sel:%d \n", 
			__FUNCTION__, cpu_sel, div_sel);
		return;
	}    

	addr = CNS3XXX_PM_BASE_VIRT + 0x014; /* PM_CLK_CTRL_REG */

	data = readl(addr);
	
	old_cpu = ((data >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf);
	old_div = ((data >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3); 

	if ((cpu_sel == old_cpu) && (div_sel == old_div)) {
		return;
	}    
    
	/* 1. Set PLL_CPU_SEL */
	data &= ~((0xF) << PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL);
	data |= (((cpu_sel) & 0xF) << PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL);
	data &= ~((0x3) << PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV);
	data |= (((div_sel) & 0x3) << PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV);
	writel(data, addr);
	/* 2. Set in DFS mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);

    /* 3. disable all interrupt except interrupt ID-32 (clkscale_intr) */
	writel(0xFFFFFFFF, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x184);
	writel(0xFFFFFFFF, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x188);
	writel(0x00000001, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x104);
	writel(0x80000000, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x108);

	mb();

    /* 4. Let CPU enter into WFI state */
	WFI();

    /* enable interrupts (we disabled before WFI) */
	writel(0xFFFFFFFF, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x104);
	writel(0xFFFFFFFF, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x108);

#if 0
	/* FIXME: this section should be move to cpufreq notifier of timer */
	cns3xxx_timer1_change_clock();
#endif
}
EXPORT_SYMBOL(cns3xxx_pwr_change_cpu_clock);

void cns3xxx_wfi(void)
{
        mb();
        asm volatile(
                        "mov r0, #0\n"
                        "mcr p15, 0, r0, c7, c10, 4\n"
                        "mcr p15, 0, r0, c7, c0, 4\n"
                        );
}
EXPORT_SYMBOL(cns3xxx_wfi);

static void wakeup_clean(void)
{
	writel(0x0, CNS3XXX_PM_BASE_VIRT + 0x028); /* PM_WU_CTRL0_REG */
	writel(0x0, CNS3XXX_PM_BASE_VIRT + 0x02C); /* PM_WU_CTRL1_REG */
}

static void wakeup_set(unsigned int id)
{
	u32 offset, data, addr;

    /* sanity check */
    if ((IRQ_CNS3XXX_PMU < id) || (IRQ_CNS3XXX_EXTERNAL_PIN2 > id)) {
        return;
    }

    offset = (id - IRQ_TC11MP_GIC_START);

    if (offset & 0x20) {
		addr = CNS3XXX_PM_BASE_VIRT + 0x02C; /* PM_WU_CTRL1_REG */	
	} else {
		addr = CNS3XXX_PM_BASE_VIRT + 0x028; /* PM_WU_CTRL0_REG */	
	}

	data = readl(addr);
	data |= (0x1 << (offset & 0x1f));
	writel(data, addr);
}

static void wakeup_set_all(void)
{
	writel(0xFFFFFFFF, CNS3XXX_PM_BASE_VIRT + 0x028); /* PM_WU_CTRL0_REG */
	writel(0xFFFFFFFF, CNS3XXX_PM_BASE_VIRT + 0x02C); /* PM_WU_CTRL1_REG */
}

/*
 * cns3xxx_pwr_doze - 
 */
void cns_pwr_doze(void)
{
	/* Set in doze mode */
	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DOZE);

	/* set wake up interrupt source*/
	wakeup_set_all();
	
	u32 reg = __raw_readl(MISC_GPIOB_PIN_ENABLE_REG);
	reg |= (0x1 << 27); 
	__raw_writel(reg, MISC_GPIOB_PIN_ENABLE_REG);
	 
	
	writel(0x00000001, CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT + 0x104); /* enable clock scaling interrupt */

	/* Let CPU enter into WFI state */
	WFI();

	cns3xxx_pwr_mode(CNS3XXX_PWR_CPU_MODE_DFS);
}

#ifdef CONFIG_SUSPEND
static int cns_suspend_enter(suspend_state_t state)
{
	cns_pwr_doze();
    return 0;
}

static struct platform_suspend_ops cnw_suspend_ops = {
	.enter = cns_suspend_enter,
	.valid = suspend_valid_only_mem,
};
#endif


#ifdef CNS_PMU_DEBUG
struct proc_dir_entry *pmu_proc_entry;
struct proc_dir_entry *pm_clk_proc_entry;
struct proc_dir_entry *pm_pll_pd_proc_entry;
 
const int ddr_speed_str[]={200,266,333,400};
static int cns3xxx_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int num = 0;

	num += sprintf(page + num, "CLK_GATE_REG         0x%.8x\n", PM_CLK_GATE_REG);
	num += sprintf(page + num, "SOFT_RST_REG         0x%.8x\n", PM_SOFT_RST_REG);
	num += sprintf(page + num, "HS_CFG_REG           0x%.8x\n", PM_HS_CFG_REG);
	num += sprintf(page + num, "CACTIVE_STA_REG      0x%.8x\n", PM_CACTIVE_STA_REG);
	num += sprintf(page + num, "PWR_STA_REG          0x%.8x\n", PM_PWR_STA_REG);
	num += sprintf(page + num, "CLK_CTRL_REG         0x%.8x\n", PM_CLK_CTRL_REG);
	num += sprintf(page + num, "PLL_LCD_I2S_CTRL_REG 0x%.8x\n", PM_PLL_LCD_I2S_CTRL_REG);
	num += sprintf(page + num, "PLL_HM_PD_CTRL_REG   0x%.8x\n", PM_PLL_HM_PD_CTRL_REG);
	num += sprintf(page + num, "REGULAT_CTRL_REG     0x%.8x\n", PM_REGULAT_CTRL_REG);
	num += sprintf(page + num, "WDT_CTRL_REG         0x%.8x\n", PM_WDT_CTRL_REG);
	num += sprintf(page + num, "WU_CTRL0_REG         0x%.8x\n", PM_WU_CTRL0_REG);
	num += sprintf(page + num, "WU_CTRL1_REG         0x%.8x\n", PM_WU_CTRL1_REG);
	num += sprintf(page + num, "CSR_REG 0x%.8x\n", PM_CSR_REG);

	num += sprintf(page + num, "PLL CPU Frequency: ");
	switch (PM_CLK_CTRL_REG & 0xf) {
	case 0: num += sprintf(page + num, "300MHz\n"); break;
	case 1: num += sprintf(page + num, "333MHz\n"); break;
	case 2: num += sprintf(page + num, "366MHz\n"); break;
	case 3: num += sprintf(page + num, "400MHz\n"); break;
	case 4: num += sprintf(page + num, "433MHz\n"); break;
	case 5: num += sprintf(page + num, "466MHz\n"); break;
	case 6: num += sprintf(page + num, "500MHz\n"); break;
	case 7: num += sprintf(page + num, "533MHz\n"); break;
	case 8: num += sprintf(page + num, "566MHz\n"); break;
	case 9: num += sprintf(page + num, "600MHz\n"); break;
	default:
		num += sprintf(page + num, "!!!!!\n");
	}

	num += sprintf(page + num, "CPU clock divider: %d\n", 0x1 << ((PM_CLK_CTRL_REG>>4)&0x3));
	num += sprintf(page + num, "CPU clock: %d MHz\n", cns3xxx_cpu_clock());
	num += sprintf(page + num, "DDR2 clock %d MHz\n", ddr_speed_str[(PM_CLK_CTRL_REG>>7)&0x3]);

	return num;
}

static int pm_clk_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
    if (count) {
        unsigned int index = PM_CLK_GATE_REG_OFFSET_SDIO+1, val = 2;
        sscanf(buffer, "%u %u", &index, &val);
        if (1 < val) {
            goto clk_debug_exit;
        }

        if ((0x1 << index) & PM_CLK_GATE_REG_MASK) {
            if (val) {
                PM_CLK_GATE_REG |= (0x1 << index);
            } else {
                PM_CLK_GATE_REG &= ~(0x1 << index);
            }
        }
    }
clk_debug_exit:
    return count;
}

static int pm_clk_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
#define CLK_STRING(DEV) \
    "(%.2d): %s\n", PM_CLK_GATE_REG_OFFSET_##DEV, (0x1 & (reg>>PM_CLK_GATE_REG_OFFSET_##DEV))?"On":"Off"

    int num = 0, reg = PM_CLK_GATE_REG;

    num += sprintf(page + num, "PM_CLK_GATE_REG 0x%.8x\n", reg);
    num += sprintf(page + num, "SMC     "CLK_STRING(SMC_NFI));
    num += sprintf(page + num, "SPI/I2C "CLK_STRING(SPI_PCM_I2C));
    num += sprintf(page + num, "GDMA    "CLK_STRING(GDMA));
    num += sprintf(page + num, "RTC     "CLK_STRING(RTC));
    num += sprintf(page + num, "UART0   "CLK_STRING(UART0));
    num += sprintf(page + num, "UART1   "CLK_STRING(UART1));
    num += sprintf(page + num, "UART2   "CLK_STRING(UART2));
    num += sprintf(page + num, "GPIO    "CLK_STRING(GPIO));
    num += sprintf(page + num, "SWITCH  "CLK_STRING(SWITCH));
    num += sprintf(page + num, "HCIE    "CLK_STRING(HCIE));
    num += sprintf(page + num, "CRYPTO  "CLK_STRING(CRYPTO));
    num += sprintf(page + num, "TIMER   "CLK_STRING(TIMER));
    num += sprintf(page + num, "USB_OTG "CLK_STRING(USB_OTG));
    num += sprintf(page + num, "USB_HOST"CLK_STRING(USB_HOST));
    num += sprintf(page + num, "PCIE1   "CLK_STRING(PCIE(1)));
    num += sprintf(page + num, "PCIE0   "CLK_STRING(PCIE(0)));
    num += sprintf(page + num, "SATA    "CLK_STRING(SATA));
    num += sprintf(page + num, "RAID    "CLK_STRING(RAID));
    num += sprintf(page + num, "I2S     "CLK_STRING(I2S));
    num += sprintf(page + num, "LCDC    "CLK_STRING(LCDC));
    num += sprintf(page + num, "CIM     "CLK_STRING(CIM));
    num += sprintf(page + num, "GPU     "CLK_STRING(GPU));
    num += sprintf(page + num, "SDIO    "CLK_STRING(SDIO));

    return num;
}
static int pm_pll_pd_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
    if (count) {
        unsigned int index = PM_PLL_HM_PD_CTRL_REG_OFFSET_SATA_PHY1+1, val = 2;
        sscanf(buffer, "%u %u", &index, &val);
        if (1< val) {
            goto clk_debug_exit;
        }

        if ((0x1 << index) & PM_PLL_HM_PD_CTRL_REG_MASK) {
            if (val) {
                PM_PLL_HM_PD_CTRL_REG |= (0x1 << index);
            } else {
                PM_PLL_HM_PD_CTRL_REG &= ~(0x1 << index);
            }
        }
    }
clk_debug_exit:
    return count;
}
static int pm_pll_pd_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
#define PLL_PD_STRING(DEV) \
    "(%.2d): %s\n", \
    PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_##DEV, \
    (0x1 & (reg>>PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_##DEV))?"Power Down":"Powe Up"

#define PHY_PD_STRING(DEV) \
    "(%.2d): %s\n", \
    PM_PLL_HM_PD_CTRL_REG_OFFSET_##DEV, \
    (0x1 & (reg>>PM_PLL_HM_PD_CTRL_REG_OFFSET_##DEV))?"Power Down":"Powe Up"

    int num = 0, reg = PM_PLL_HM_PD_CTRL_REG;

    num += sprintf(page + num, "PM_PLL_HM_PD_CTRL_REG 0x%.8x\n", reg);
    num += sprintf(page + num, "RGMII     "PLL_PD_STRING(RGMII));
    num += sprintf(page + num, "USB       "PLL_PD_STRING(USB));
    num += sprintf(page + num, "LCD       "PLL_PD_STRING(LCD));
    num += sprintf(page + num, "I2S       "PLL_PD_STRING(I2S));
    num += sprintf(page + num, "I2SCD     "PLL_PD_STRING(I2SCD));
    num += sprintf(page + num, "SATA_PHY0 "PHY_PD_STRING(SATA_PHY0));
    num += sprintf(page + num, "SATA_PHY1 "PHY_PD_STRING(SATA_PHY1));

    return num;
}

static int __init cns3xxx_pmu_proc_init(void)
{
	pmu_proc_entry = create_proc_entry("pmu", S_IFREG | S_IRUGO, cns3xxx_proc_dir);
	if (pmu_proc_entry) {
		pmu_proc_entry->read_proc = cns3xxx_read_proc;
	}
	pm_clk_proc_entry = create_proc_entry("pm_clk", S_IFREG | S_IRUGO, cns3xxx_proc_dir);
	if (pm_clk_proc_entry) {
		pm_clk_proc_entry->read_proc = pm_clk_read_proc;
		pm_clk_proc_entry->write_proc = pm_clk_write_proc;
	}

	pm_pll_pd_proc_entry = create_proc_entry("pm_pll_pd", S_IFREG | S_IRUGO, cns3xxx_proc_dir);
	if (pm_pll_pd_proc_entry) {
		pm_pll_pd_proc_entry->read_proc = pm_pll_pd_read_proc;
		pm_pll_pd_proc_entry->write_proc = pm_pll_pd_write_proc;
	}
	return 1;
}

#endif

static int __init cns_pm_init(void)
{
	/* enable low power handshaking interface */
	cns3xxx_lp_hs(PM_HS_CFG_REG_MASK_SUPPORT);

#ifdef CONFIG_SUSPEND
	suspend_set_ops(&cnw_suspend_ops);
#endif

#ifdef CNS_PMU_DEBUG
	cns3xxx_pmu_proc_init();
#endif
    return 0;
}

late_initcall(cns_pm_init);

atomic_t usb_pwr_ref = ATOMIC_INIT(0);
EXPORT_SYMBOL(usb_pwr_ref);



