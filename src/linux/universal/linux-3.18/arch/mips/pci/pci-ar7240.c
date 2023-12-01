#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/irq.h>
#include <linux/pci.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/ath9k_platform.h>

#include <asm/delay.h>

#define ag7100_delay1s()    mdelay(1000);

#include "ar7240.h"


static int ar724x_pci_fixup_enable;
static int arnew=0;


/*
 * Support for Ar7100 pci interrupt and core pci initialization
 */
/*
 * PCI interrupts.
 * roughly the interrupts flow is:
 *
 * - save flags
 * - CLI (disable all)
 * - IC->ack (mask out the source)
 * - EI (enable all, except the source that was masked of course)
 * - action (ISR)
 * - IC->enable (unmask the source)
 *
 * The reason we have a separate PCI IC is beacause of the following:
 * If we dont, then Throughout the "action" of a PCI slot, the
 * entire PCI "IP" on the cpu will remain disabled. Which means that we cant
 * prioritize between PCI interrupts. Normally this should be ok, if all PCI 
 * interrupts are considered equal. However, creating a PCI IC gives 
 * the flexibility to prioritize.
 */

static void
ar7240_pci_irq_enable(unsigned int irq)
{
    ar7240_reg_rmw_set(AR7240_PCI_INT_MASK, AR7240_PCI_INT_A_L);
}

static void
ar7240_pci_irq_disable(unsigned int irq)
{
	ar7240_reg_rmw_clear(AR7240_PCI_INT_MASK, AR7240_PCI_INT_A_L);
	ar7240_reg_rmw_clear(AR7240_PCI_INT_STATUS, AR7240_PCI_INT_A_L);
}

static unsigned int
ar7240_pci_irq_startup(unsigned int irq)
{
	ar7240_pci_irq_enable(irq);
	return 0;
}

static void
ar7240_pci_irq_shutdown(unsigned int irq)
{
	ar7240_pci_irq_disable(irq);
}

static void
ar7240_pci_irq_ack(unsigned int irq)
{
	ar7240_pci_irq_disable(irq);
}

static void
ar7240_pci_irq_end(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		ar7240_pci_irq_enable(irq);
}

static void
ar7240_pci_irq_set_affinity(unsigned int irq, cpumask_t mask)
{
	/* 
     * Only 1 CPU; ignore affinity request 
     */
}

struct irq_chip ar7240_pci_irq_chip = {
	.name		= "AR7240 PCI",
	.mask		= ar7240_pci_irq_disable,
	.unmask		= ar7240_pci_irq_enable,
	.mask_ack	= ar7240_pci_irq_disable,
};
#define AR724X_RESET_PCIE_PHY_SERIAL	BIT(10)
#define AR724X_RESET_PCIE_PHY		BIT(7)
#define AR724X_RESET_PCIE		BIT(6)

#define AR724X_PCI_REG_INT_STATUS	0x4c
#define AR724X_PCI_REG_INT_MASK		0x50
static void __iomem *ar724x_pci_ctrl_base;
#define AR724X_PCI_INT_DEV0		BIT(14)
#define AR71XX_PCI_IRQ_BASE     48
#define AR71XX_PCI_IRQ_DEV0	(AR71XX_PCI_IRQ_BASE + 0)

static void ar724x_pci_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	void __iomem *base = ar724x_pci_ctrl_base;
	u32 pending;

	pending = __raw_readl(base + AR724X_PCI_REG_INT_STATUS) &
		  __raw_readl(base + AR724X_PCI_REG_INT_MASK);

	if (pending & AR724X_PCI_INT_DEV0)
		generic_handle_irq(AR71XX_PCI_IRQ_DEV0);

	else
		spurious_interrupt();
}

void
ar7240_pci_irq_init(int irq_base)
{
	int i;
	u32 t;
	void __iomem *base = ar724x_pci_ctrl_base;

	t = ar7240_reg_rd(AR7240_RESET);
	if (t & (AR724X_RESET_PCIE | AR724X_RESET_PCIE_PHY |
		 AR724X_RESET_PCIE_PHY_SERIAL)) {
		 printk(KERN_INFO "can't init irq, device still not up\n");
		return;
	}

	__raw_writel(0, base + AR724X_PCI_REG_INT_MASK);
	__raw_writel(0, base + AR724X_PCI_REG_INT_STATUS);

	for (i = irq_base; i < irq_base + AR7240_PCI_IRQ_COUNT; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		set_irq_chip_and_handler(i, &ar7240_pci_irq_chip,
					 handle_level_irq);
	}
//	set_irq_chained_handler(AR71XX_CPU_IRQ_IP2, ar724x_pci_irq_handler);
	set_irq_chained_handler(AR7240_CPU_IRQ_PCI, ar724x_pci_irq_handler);
}

/*
 * init the pci controller
 */

static struct resource ar7240_io_resource = {
	.name		= "PCI IO space",
	.start		= 0x0000,
	.end		= 0,
	.flags		= IORESOURCE_IO,
};

static struct resource ar7240_mem_resource = {
	.name		= "PCI memory space",
	.start		= AR7240_PCI_MEM_BASE,
	.end		= AR7240_PCI_MEM_BASE + AR7240_PCI_WINDOW - 1,
	.flags		= IORESOURCE_MEM
};

extern struct pci_ops ar7240_pci_ops;

static struct pci_controller ar7240_pci_controller = {
	.pci_ops	    = &ar7240_pci_ops,
	.mem_resource	= &ar7240_mem_resource,
	.io_resource	= &ar7240_io_resource,
};


irqreturn_t 
ar7240_pci_core_intr(int cpl, void *dev_id, struct pt_regs *regs)
{
    printk("PCI error intr\n");
#if 0
    ar7240_check_error(1);
#endif

    return IRQ_HANDLED;
}

static void ar724x_pci_fixup(struct pci_dev *dev)
{
	u16 t;

	if (!ar724x_pci_fixup_enable)
		return;

	if (dev->bus->number != 0 || dev->devfn != 0)
		return;

	pci_read_config_word(dev, PCI_COMMAND, &t);

	/* setup COMMAND register */
	t = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_INVALIDATE
	  | PCI_COMMAND_PARITY | PCI_COMMAND_SERR | PCI_COMMAND_FAST_BACK;

	pci_write_config_word(dev, PCI_COMMAND, t);
}
DECLARE_PCI_FIXUP_EARLY(PCI_ANY_ID, PCI_ANY_ID, ar724x_pci_fixup);

static void *getCalData(int slot)
{
#ifdef CONFIG_WDR2543
u8 *base = KSEG1ADDR(0x1fff1000);
		printk(KERN_INFO "found calibration data for slot %d on 0x%08X\n",slot,base);
return base;
#else
u8 *base;
for (base=(u8 *) KSEG1ADDR(0x1f000000);base<KSEG1ADDR (0x1fff0000);base+=0x1000) {
	u32 *cal = (u32 *)base;
	if (*cal==0xa55a0000 || *cal==0x5aa50000) { //protection bit is always zero on inflash devices, so we can use for match it
		if (slot) {
			base+=0x4000;
		}
		printk(KERN_INFO "found calibration data for slot %d on 0x%08X\n",slot,base);
		return base;
	}
    }
return NULL;
#endif
}


static struct ath9k_platform_data wmac_data[1];
extern int is_ar9300;
static void ap91_pci_fixup(struct pci_dev *dev)
{
	void __iomem *mem;
	u16 *cal_data=NULL;
	u8 *calcopy;
	u16 cmd;
	u32 val;
	u32 bar0;

	if (!ar724x_pci_fixup_enable)
		return;


	printk(KERN_INFO "PCI: fixup device %s\n", pci_name(dev));

	cal_data = (u16 *)getCalData(0);
	calcopy = (u8 *)cal_data;
	if (cal_data) {
		memcpy(wmac_data[0].eeprom_data,cal_data,sizeof(wmac_data[0].eeprom_data));
		dev->dev.platform_data = &wmac_data[0];
	} else {
		printk(KERN_INFO "no in flash calibration fata found, no fix required\n");
		return;
	}


	mem = ioremap(AR7240_PCI_MEM_BASE, 0x10000);
	if (!mem) {
		printk(KERN_ERR "PCI: ioremap error for device %s\n",
		       pci_name(dev));
		return;
	}

	pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &bar0);

	/* Setup the PCI device to allow access to the internal registers */
        if (arnew)
        {
	    pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0x1000ffff);
        }else{
	    pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0xffff);
	}
	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_write_config_word(dev, PCI_COMMAND, cmd);

        /* set pointer to first reg address */
	cal_data += 3;
	while (*cal_data != 0xffff) {
		u32 reg;
		reg = *cal_data++;
		val = *cal_data++;
		val |= (*cal_data++) << 16;

//		printk(KERN_INFO "bootstraping %X->%X\n",reg,val);
		__raw_writel(val, mem + reg);
		udelay(100);
	}

	pci_read_config_dword(dev, PCI_VENDOR_ID, &val);
	dev->vendor = val & 0xffff;
	dev->device = (val >> 16) & 0xffff;
	printk(KERN_INFO "bootstrap returns device %X:%X\n",dev->vendor,dev->device);
	#ifndef CONFIG_WDR2543
	if (dev->device==0x0030) //AR9300 Hack
 	    {
 		calcopy+=0x1000;
		memcpy(wmac_data[0].eeprom_data,calcopy,sizeof(wmac_data[0].eeprom_data));
		wmac_data[0].led_pin = 15;
		is_ar9300=1;
		dev->dev.platform_data = &wmac_data[0];	    
	    }
	#else
		is_ar9300=1;	
	#endif
	pci_read_config_dword(dev, PCI_CLASS_REVISION, &val);
	dev->revision = val & 0xff;
	dev->class = val >> 8; /* upper 3 bytes */

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd &= ~(PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	pci_write_config_word(dev, PCI_COMMAND, cmd);

	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, bar0);

	iounmap(mem);
	return;
}
DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_ATHEROS, PCI_ANY_ID, ap91_pci_fixup);

/*
 * We want a 1:1 mapping between PCI and DDR for inbound and outbound.
 * The PCI<---AHB decoding works as follows:
 *
 * 8 registers in the DDR unit provide software configurable 32 bit offsets
 * for each of the eight 16MB PCI windows in the 128MB. The offsets will be 
 * added to any address in the 16MB segment before being sent to the PCI unit.
 *
 * Essentially  for any AHB address generated by the CPU,
 * 1. the MSB  four bits are stripped off, [31:28],
 * 2. Bit 27 is used to decide between the lower 128Mb (PCI) or the rest of 
 *    the AHB space
 * 3. Bits 26:24 are used to access one of the 8 window registers and are 
 *    masked off.
 * 4. If it is a PCI address, then the WINDOW offset in the WINDOW register 
 *    corresponding to the next 3 bits (bit 26:24) is ADDED to the address, 
 *    to generate the address to PCI unit.
 *
 *     eg. CPU address = 0x100000ff
 *         window 0 offset = 0x10000000
 *         This points to lowermost 16MB window in PCI space.
 *         So the resulting address would be 0x000000ff+0x10000000
 *         = 0x100000ff
 *
 *         eg2. CPU address = 0x120000ff
 *         WINDOW 2 offset = 0x12000000
 *         resulting address would be 0x000000ff+0x12000000
 *                         = 0x120000ff 
 *
 * There is no translation for inbound access (PCI device as a master)
 */ 

extern void ar724x_pci_write(void __iomem *base, int where, int size, u32 value);

#define AR71XX_RESET_BASE	(AR7240_APB_BASE + 0x00060000)
#define AR71XX_RESET_SIZE	0x10000
static void __iomem *ar71xx_reset_base;

static inline void ar71xx_reset_wr(unsigned reg, u32 val)
{
	__raw_writel(val, ar71xx_reset_base + reg);
}

static inline u32 ar71xx_reset_rr(unsigned reg)
{
	return __raw_readl(ar71xx_reset_base + reg);
}

#define RESET_MODULE_USB_OHCI_DLL_7240	1<<3
#define AR724X_RESET_REG_RESET_MODULE		0x1c

void ar71xx_device_stop(u32 mask)
{
	unsigned long flags;
	u32 mask_inv;
	u32 t;

		mask_inv = mask & RESET_MODULE_USB_OHCI_DLL_7240;
		local_irq_save(flags);
		t = ar71xx_reset_rr(AR724X_RESET_REG_RESET_MODULE);
		t |= mask;
		t &= ~mask_inv;
		ar71xx_reset_wr(AR724X_RESET_REG_RESET_MODULE, t);
		local_irq_restore(flags);
}

void ar71xx_device_start(u32 mask)
{
	unsigned long flags;
	u32 mask_inv;
	u32 t;

		mask_inv = mask & RESET_MODULE_USB_OHCI_DLL_7240;
		local_irq_save(flags);
		t = ar71xx_reset_rr(AR724X_RESET_REG_RESET_MODULE);
		t &= ~mask;
		t |= mask_inv;
		ar71xx_reset_wr(AR724X_RESET_REG_RESET_MODULE, t);
		local_irq_restore(flags);
}
#define AR7240_PCIE_PLL_CONFIG          AR7240_PLL_BASE+0x10

static void __init ar7242_pci_reset(void)
{
//	printk(KERN_INFO "reset register before %X PLL:%X\n", ar7240_reg_rd(AR71XX_RESET_BASE + AR724X_RESET_REG_RESET_MODULE),ar7240_reg_rd(AR7240_PCIE_PLL_CONFIG));
/*	ar71xx_device_stop(AR724X_RESET_PCIE);
	ar71xx_device_stop(AR724X_RESET_PCIE_PHY);
	ar71xx_device_stop(AR724X_RESET_PCIE_PHY_SERIAL);
	udelay(100);
	printk(KERN_INFO "reset register middle %X PLL:%X\n", ar7240_reg_rd(AR71XX_RESET_BASE + AR724X_RESET_REG_RESET_MODULE),ar7240_reg_rd(AR7240_PCIE_PLL_CONFIG));
*/
	ar71xx_device_start(AR724X_RESET_PCIE_PHY_SERIAL);
	udelay(100);
	ar71xx_device_start(AR724X_RESET_PCIE_PHY);
	ar71xx_device_start(AR724X_RESET_PCIE);
//	printk(KERN_INFO "reset register after %X PLL:%X\n", ar7240_reg_rd(AR71XX_RESET_BASE + AR724X_RESET_REG_RESET_MODULE),ar7240_reg_rd(AR7240_PCIE_PLL_CONFIG));

/*	ar7240_reg_wr_nf(AR7240_PCI_LCL_RESET, 0);
	udelay(100000);
	ar7240_reg_wr(AR7240_PCIE_PLL_CONFIG,0x02050800);

	ar7240_reg_wr(AR7240_PCIE_PLL_CONFIG,0x00050800);
	udelay(100);

	ar7240_reg_wr(AR7240_PCIE_PLL_CONFIG,0x00040800);
	udelay(100000);
	ar7240_reg_wr_nf(AR7240_PCI_LCL_RESET, 4);
	udelay(100000);*/

}
#define AR724X_PCI_CTRL_BASE	(AR7240_APB_BASE + 0x000F0000)
#define AR724X_PCI_CTRL_SIZE	0x100
#define AR724X_PCI_REG_RESET		0x18
#define AR724X_PCI_REG_APP		0x00
#define AR7240_PLL_BASE                 AR7240_APB_BASE+0x00050000
#define AR724X_PCI_CRP_BASE	(AR7240_APB_BASE + 0x000C0000)
#define AR724X_PCI_CRP_SIZE	0x100
static void __iomem *ar724x_pci_localcfg_base;

int ath_nopcie=0;

static int __init ar7242_pci_setup(void)
{
	void __iomem *base = ar724x_pci_ctrl_base;
	u32 t;
	/* setup COMMAND register */

	t = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_INVALIDATE |
	    PCI_COMMAND_PARITY|PCI_COMMAND_SERR|PCI_COMMAND_FAST_BACK;

	ar724x_pci_write(ar724x_pci_localcfg_base, PCI_COMMAND, 4, t);
	ar724x_pci_write(ar724x_pci_localcfg_base, 0x20, 4, 0x1ff01000);
	ar724x_pci_write(ar724x_pci_localcfg_base, 0x24, 4, 0x1ff01000);


	t = __raw_readl(base + AR724X_PCI_REG_RESET);
	printk(KERN_INFO "reset reg %X\n", t);
	if (t != 0x7) {
		printk(KERN_INFO "reset PCIE Bus and PLL\n");
		udelay(100000);
		__raw_writel(0, base + AR724X_PCI_REG_RESET);
		udelay(100);
		__raw_writel(4, base + AR724X_PCI_REG_RESET);
		udelay(100000);
	}

	t = __raw_readl(base + AR724X_PCI_REG_RESET);
	printk(KERN_INFO "reset reg after %X\n", t);

	if (arnew)
		t = 0x1ffc1;
	else
		t = 1;

	__raw_writel(t, base + AR724X_PCI_REG_APP);
	/* flush write */
	(void) __raw_readl(base + AR724X_PCI_REG_APP);
	udelay(1000);

	t = __raw_readl(base + AR724X_PCI_REG_RESET);
	if ((t & 0x1) == 0x0) {
		printk(KERN_WARNING "PCI: no PCIe module found\n");
		ath_nopcie=1;
		return -ENODEV;
	}

	if (arnew) {
		t = __raw_readl(base + AR724X_PCI_REG_APP);
		t |= BIT(16);
		__raw_writel(t, base + AR724X_PCI_REG_APP);
	}

	return 0;
}
 
 
int __init ar7240_pcibios_init(void)
{

#ifdef CONFIG_WASP_SUPPORT
	if (is_ar9341()) {
		return 0;
	}
#endif
	if (is_ar7241() || is_ar7242())
	    {
	    printk(KERN_INFO "ar7241/ar7242 detected\n");
	    arnew=1;
	    }else
	    arnew=0;
	    
	ar724x_pci_localcfg_base = ioremap_nocache(AR724X_PCI_CRP_BASE,
						   AR724X_PCI_CRP_SIZE);
	ar724x_pci_ctrl_base = ioremap_nocache(AR724X_PCI_CTRL_BASE,
					       AR724X_PCI_CTRL_SIZE);
	ar71xx_reset_base = ioremap_nocache(AR71XX_RESET_BASE,
						AR71XX_RESET_SIZE);

	/*
	 * Check if the WLAN PCI-E H/W is present, If the
	 * WLAN H/W is not present, skip the PCI
	 * initialization code and just return.
	 */
	ar7242_pci_reset();
	ar7242_pci_setup();
/*
	if (((ar7240_reg_rd(AR7240_PCI_LCL_RESET)) & 0x1) == 0x0) {
		printk("***** Warning *****: PCIe WLAN H/W not found !!!\n");
		return 0;
	}
        if ((is_ar7241() || is_ar7242()))
		ar7240_reg_wr(AR7240_PCI_LCL_APP, (ar7240_reg_rd(AR7240_PCI_LCL_APP) | (0x1 << 16)));


	printk("PCI init:%s\n", __func__);
    uint32_t cmd;

    cmd = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_INVALIDATE |
          PCI_COMMAND_PARITY|PCI_COMMAND_SERR|PCI_COMMAND_FAST_BACK;

    ar7240_local_write_config(PCI_COMMAND, 4, cmd);
*/
    ar724x_pci_fixup_enable = 1;
    ar7240_pci_irq_init(AR7240_PCI_IRQ_BASE);

    register_pci_controller(&ar7240_pci_controller);

    return 0;
}

//arch_initcall(ar7240_pcibios_init);
