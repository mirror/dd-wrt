/*
 * Copyright 2004 PMC-Sierra
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  PCI interface for the PMC-Sierra Stretch Board
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <asm/pci.h>
#include <asm/io.h>

#include "setup.h"

#ifdef CONFIG_PCI

void pmc_stretch_pcibios_fixup_bus(struct pci_bus* c);

static int pmc_stretch_read_config_dword(struct pci_dev *device,
					int offset, u32* val)
{
	int dev, bus, func;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	address = (bus << 8) | (dev << 16) | (func << 21) |
		((offset & 0xfc) << 24)  | 0x1;

	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_CONFIG_ADDRESS, address);
	PMC_STRETCH_READ(PMC_STRETCH_PCI_0_DATA_ADDRESS, val);

	return PCIBIOS_SUCCESSFUL;
}

static int pmc_stretch_read_config_word(struct pci_dev *device,
					int offset, u16* val)
{
	int dev, bus, func;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	address = (bus << 8) | (dev << 16) | (func << 21) |
		((offset & 0xfc) << 24)  | 0x1;

	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_CONFIG_ADDRESS, address);
	PMC_STRETCH_READ_16((PMC_STRETCH_PCI_0_DATA_ADDRESS + (offset & 0x3)), val);

	return PCIBIOS_SUCCESSFUL;
}

static int pmc_stretch_read_config_byte(struct pci_dev *device,
					int offset, u8* val)
{
	int dev, bus, func;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	address = (bus << 8) | (dev << 16) | (func << 21) |
		((offset & 0xfc) << 24)  | 0x1;

	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_CONFIG_ADDRESS, address);
	PMC_STRETCH_READ_8((PMC_STRETCH_PCI_0_DATA_ADDRESS + (offset & 0x3)), val);

	return PCIBIOS_SUCCESSFUL;
}

static int pmc_stretch_write_config_dword(struct pci_dev *device,
					int offset, u32 val)
{
	int dev, bus, func;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	address = (bus << 8) | (dev << 16) | (func << 21) |
		((offset & 0xfc) << 24)  | 0x1;

	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_CONFIG_ADDRESS, address);
	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_DATA_ADDRESS, val);

	return PCIBIOS_SUCCESSFUL;
}

static int pmc_stretch_write_config_word(struct pci_dev *device,
					int offset, u16 val)
{
	int dev, bus, func;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	address = (bus << 8) | (dev << 16) | (func << 21) |
		((offset & 0xfc) << 24)  | 0x1;

	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_CONFIG_ADDRESS, address);
	PMC_STRETCH_WRITE_16((PMC_STRETCH_PCI_0_DATA_ADDRESS + (offset & 0x3)), val);

	return PCIBIOS_SUCCESSFUL;
}

static int pmc_stretch_write_config_byte(struct pci_dev *device,
					int offset, u8 val)
{
	int dev, bus, func;
	uint32_t address;

	bus = device->bus->number;
	dev = PCI_SLOT(device->devfn);
	func = PCI_FUNC(device->devfn);

	address = (bus << 8) | (dev << 16) | (func << 21) |
		((offset & 0xfc) << 24)  | 0x1;

	PMC_STRETCH_WRITE(PMC_STRETCH_PCI_0_CONFIG_ADDRESS, address);
	PMC_STRETCH_WRITE_8((PMC_STRETCH_PCI_0_DATA_ADDRESS + (offset & 0x3)), val);

	return PCIBIOS_SUCCESSFUL;
}

static void pmc_stretch_pcibios_set_master(struct pci_dev *dev)
{
	u16 cmd;

	pmc_stretch_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER;
	pmc_stretch_write_config_word(dev, PCI_COMMAND, cmd);
}

int pcibios_enable_resources(struct pci_dev *dev)
{
	u16 cmd, old_cmd;
	u8 tmp1;
	int idx;
	struct resource *r;

	pmc_stretch_read_config_word(dev, PCI_COMMAND, &cmd);
	
	old_cmd = cmd;
	for (idx = 0; idx < 6; idx++) {
		r = &dev->resource[idx];
		if (!r->start && r->end) {
			printk(KERN_ERR
				"PCI: Device %s not available because of "
				"resource collisions\n", dev->slot_name);

			return -EINVAL;
		}

		if (r->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;

		if (r->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}

	if (cmd != old_cmd) 
		pmc_stretch_write_config_word(dev, PCI_COMMAND, cmd);

	pmc_stretch_read_config_byte(dev, PCI_CACHE_LINE_SIZE, &tmp1);
	if (tmp1 != 8) {
		printk(KERN_WARNING "PCI setting cache line size to 8 from "
			"%d\n", tmp1);
	
		pmc_stretch_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 8);
	}

	pmc_stretch_read_config_byte(dev, PCI_LATENCY_TIMER, &tmp1);
	if (tmp1 < 32 || tmp1 == 0xff) {
		printk(KERN_WARNING "PCI setting latency timer to 32 from %d\n",
			tmp1);
		pmc_stretch_write_config_byte(dev, PCI_LATENCY_TIMER, 32);
	}

	return 0;
}

/*
 * Export for pcibios_enable_device
 */
int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	return pcibios_enable_resources(dev);
}

/*
 * Export for pcibios_update_resource
 */
void pcibios_update_resource(struct pci_dev *dev, struct resource *root,
				struct resource *res, int resource)
{
	u32 new, check;
	int reg;

	new = res->start | (res->flags & PCI_REGION_FLAG_MASK);
	if (resource < 6) {
		reg = PCI_BASE_ADDRESS_0 + 4 * resource;
	} else if (resource == PCI_ROM_RESOURCE) {
		res->flags |= PCI_ROM_ADDRESS_ENABLE;
		reg = dev->rom_base_reg;
	} else {
		/*
		 * Somebody might have asked allocation of a non-standard
		 * resource
		 */
		return;
	}

	pci_write_config_dword(dev, reg, new);
	pci_read_config_dword(dev, reg, &check);
	if ((new ^ check) &
		((new & PCI_BASE_ADDRESS_SPACE_IO) ? PCI_BASE_ADDRESS_IO_MASK :
			PCI_BASE_ADDRESS_MEM_MASK)) {
				printk(KERN_ERR "PCI: Error while updating region "
					"%s/%d (%08x != %08x)\n", dev->slot_name, resource,
					new, check);
	}
}

/*
 * Export to pcibios_align_resource
 */
void pcibios_align_resource(void *data, struct resource *res,
                            unsigned long size, unsigned long align)
{
	struct pci_dev *dev = data;

	if (res->flags & IORESOURCE_IO) {
		unsigned long start = res->start;

		/* 
		 * We need to avoid collisions with `mirrored' VGA ports
		 * and other strange ISA hardware, so we always want the
		 * addresses kilobyte aligned.  
		 */
		if (size > 0x100) {
			printk(KERN_ERR "PCI: I/O Region %s/%d too large"
					" (%ld bytes)\n", dev->slot_name,
					dev->resource - res, size);
		}

		start = (start + 1024 - 1) & ~(1024 - 1);
		res->start = start;
	}
}

struct pci_ops pmc_stretch_pci_ops = {
	pmc_stretch_read_config_byte,
	pmc_stretch_read_config_word,
	pmc_stretch_read_config_dword,
	pmc_stretch_write_config_byte,
	pmc_stretch_write_config_word,
	pmc_stretch_write_config_dword
};

struct pci_fixup pcibios_fixups[] = {
	{0}
};

void __init pcibios_fixup_bus(struct pci_bus *c)
{
	pmc_stretch_pcibios_fixup_bus(c);
}

void __init pcibios_init(void)
{
	/*
	 * FIXME: Values to change once PMON configures the 
	 * IO and MEM space for the PCI devices
	 */
	ioport_resource.start = 0xe0000000;
	ioport_resource.end   = 0xe0000000 + 0x20000000 - 1;
	iomem_resource.start  = 0xc0000000;
	iomem_resource.end    = 0xc0000000 + 0x20000000 - 1;

	/*
	 * PMC-Sierra Stretch has only two PCI busses. Scan them now
	 */
	pci_scan_bus(0, &pmc_stretch_pci_ops, NULL);
	pci_scan_bus(1, &pmc_stretch_pci_ops, NULL);
}

/*
 * for parsing "pci=" kernel boot arguments.
 */
char *pcibios_setup(char *str)
{
	printk(KERN_INFO "rr: pcibios_setup\n");
	/* Nothing to do for now.  */

	return str;
}

unsigned int pcibios_assign_all_busses(void)
{
	return 1;
}

#endif

