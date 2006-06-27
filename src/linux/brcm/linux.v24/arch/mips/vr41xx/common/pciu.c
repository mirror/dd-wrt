/*
 * arch/mips/vr41xx/common/pciu.c
 *
 * PCI Control Unit routines for the NEC VR4100 series.
 *
 * Author: Yoichi Yuasa <yyuasa@mvista.com, or source@mvista.com>
 *
 * 2001-2003 (c) MontaVista, Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */
/*
 * Changes:
 *  MontaVista Software Inc. <yyuasa@mvista.com> or <source@mvista.com>
 *  - New creation, NEC VR4122 and VR4131 are supported.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/types.h>

#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/pci_channel.h>
#include <asm/vr41xx/vr41xx.h>

#include "pciu.h"

static inline int vr41xx_pci_config_access(struct pci_dev *dev, int where)
{
	unsigned char bus = dev->bus->number;
	unsigned int dev_fn = dev->devfn;

	if (bus == 0) {
		/*
		 * Type 0 configuration
		 */
		if (PCI_SLOT(dev_fn) < 11 || PCI_SLOT(dev_fn) > 31 || where > 0xff)
			return -EINVAL;

		writel((1U << PCI_SLOT(dev_fn))	|
		       (PCI_FUNC(dev_fn) << 8)	|
		       (where & 0xfc),
		       PCICONFAREG);
	} else {
		/*
		 * Type 1 configuration
		 */
		if (PCI_SLOT(dev_fn) > 31 || where > 0xff)
			return -EINVAL;

		writel((bus << 16)	|
		       (dev_fn << 8)	|
		       (where & 0xfc)	|
		       1U,
		       PCICONFAREG);
	}

	return 0;
}

static int vr41xx_pci_config_read_byte(struct pci_dev *dev, int where, uint8_t *val)
{
	uint32_t data;

	*val = 0xff;
	if (vr41xx_pci_config_access(dev, where) < 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	data = readl(PCICONFDREG);
	*val = (uint8_t)(data >> ((where & 3) << 3));

	return PCIBIOS_SUCCESSFUL;

}

static int vr41xx_pci_config_read_word(struct pci_dev *dev, int where, uint16_t *val)
{
	uint32_t data;

	*val = 0xffff;
	if (where & 1)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	if (vr41xx_pci_config_access(dev, where) < 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	data = readl(PCICONFDREG);
	*val = (uint16_t)(data >> ((where & 2) << 3));

	return PCIBIOS_SUCCESSFUL;
}

static int vr41xx_pci_config_read_dword(struct pci_dev *dev, int where, uint32_t *val)
{
	*val = 0xffffffff;
	if (where & 3)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	if (vr41xx_pci_config_access(dev, where) < 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	*val = readl(PCICONFDREG);

	return PCIBIOS_SUCCESSFUL;
}

static int vr41xx_pci_config_write_byte(struct pci_dev *dev, int where, uint8_t val)
{
	uint32_t data;
	int shift;

	if (vr41xx_pci_config_access(dev, where) < 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	data = readl(PCICONFDREG);
	shift = (where & 3) << 3;
	data &= ~(0xffU << shift);
	data |= (uint32_t)val << shift;
	writel(data, PCICONFDREG);

	return PCIBIOS_SUCCESSFUL;
}

static int vr41xx_pci_config_write_word(struct pci_dev *dev, int where, uint16_t val)
{
	uint32_t data;
	int shift;

	if (where & 1)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	if (vr41xx_pci_config_access(dev, where) < 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	data = readl(PCICONFDREG);
	shift = (where & 2) << 3;
	data &= ~(0xffffU << shift);
	data |= (uint32_t)val << shift;
	writel(data, PCICONFDREG);

	return PCIBIOS_SUCCESSFUL;
}

static int vr41xx_pci_config_write_dword(struct pci_dev *dev, int where, uint32_t val)
{
	if (where & 3)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	if (vr41xx_pci_config_access(dev, where) < 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	writel(val, PCICONFDREG);

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops vr41xx_pci_ops = {
	.read_byte	= vr41xx_pci_config_read_byte,
	.read_word	= vr41xx_pci_config_read_word,
	.read_dword	= vr41xx_pci_config_read_dword,
	.write_byte	= vr41xx_pci_config_write_byte,
	.write_word	= vr41xx_pci_config_write_word,
	.write_dword	= vr41xx_pci_config_write_dword,
};

void __init vr41xx_pciu_init(struct vr41xx_pci_address_map *map)
{
	struct vr41xx_pci_address_space *s;
	unsigned long vtclock;
	uint32_t config;

	if (map == NULL)
		return;

	/* Disable PCI interrupt */
	writew(0, MPCIINTREG);

	/* Supply VTClock to PCIU */
	vr41xx_supply_clock(PCIU_CLOCK);

	/* Dummy read/write, waiting for supply of VTClock. */
	readw(MPCIINTREG);
	writew(0, MPCIINTREG);

	/* Select PCI clock */
	vtclock = vr41xx_get_vtclock_frequency();
	if (vtclock < MAX_PCI_CLOCK)
		writel(EQUAL_VTCLOCK, PCICLKSELREG);
	else if ((vtclock / 2) < MAX_PCI_CLOCK)
		writel(HALF_VTCLOCK, PCICLKSELREG);
	else if ((vtclock / 4) < MAX_PCI_CLOCK)
		writel(QUARTER_VTCLOCK, PCICLKSELREG);
	else
		printk(KERN_INFO "Warning: PCI Clock is over 33MHz.\n");

	/* Supply PCI clock by PCI bus */
	vr41xx_supply_clock(PCI_CLOCK);

	/*
	 * Set PCI memory & I/O space address conversion registers
	 * for master transaction.
	 */
	if (map->mem1 != NULL) {
		s = map->mem1;
		config = (s->internal_base & INTERNAL_BUS_BASE_ADDRESS)	|
		         ((s->address_mask >> 11) & ADDRESS_MASK)	|
		         PCI_ACCESS_ENABLE				|
		         ((s->pci_base >> 24) & PCI_ADDRESS_SETTING);
		writel(config, PCIMMAW1REG);
	}
	if (map->mem2 != NULL) {
		s = map->mem2;
		config = (s->internal_base & INTERNAL_BUS_BASE_ADDRESS)	|
		         ((s->address_mask >> 11) & ADDRESS_MASK)	|
		         PCI_ACCESS_ENABLE				|
		         ((s->pci_base >> 24) & PCI_ADDRESS_SETTING);
		writel(config, PCIMMAW2REG);
	}
	if (map->io != NULL) {
		s = map->io;
		config = (s->internal_base & INTERNAL_BUS_BASE_ADDRESS) |
		         ((s->address_mask >> 11) & ADDRESS_MASK) |
		         PCI_ACCESS_ENABLE |
		         ((s->pci_base >> 24) & PCI_ADDRESS_SETTING);
		writel(config, PCIMIOAWREG);
	}

	/* Set target memory windows */
	writel(0x00081000U, PCITAW1REG);
	writel(0U, PCITAW2REG);

	pciu_write_config_dword(PCI_BASE_ADDRESS_0, 0U);
	pciu_write_config_dword(PCI_BASE_ADDRESS_1, 0U);

	/* Clear bus error */
	readl(BUSERRADREG);

	if (current_cpu_data.cputype == CPU_VR4122) {
		writel(0U, PCITRDYVREG);
		pciu_write_config_byte(PCI_LATENCY_TIMER, 0xf8);
	} else {
		writel(100U, PCITRDYVREG);
		pciu_write_config_byte(PCI_LATENCY_TIMER, 0x80);
	}

	writel(CONFIG_DONE, PCIENREG);
	pciu_write_config_word(PCI_COMMAND,
	                       PCI_COMMAND_IO |
	                       PCI_COMMAND_MEMORY |
	                       PCI_COMMAND_MASTER |
	                       PCI_COMMAND_PARITY |
	                       PCI_COMMAND_SERR);
}
