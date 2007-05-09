/*
 * Copyright 2003 PMC-Sierra
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
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <asm/pci.h>
#include <asm/io.h>

#include <linux/init.h>
#include <asm/titan_dep.h>

#ifdef CONFIG_HYPERTRANSPORT

/*
 * This function check if the Hypertransport Link Initialization completed. If
 * it did, then proceed further with scanning bus #2
 */
static __inline__ int check_titan_htlink(void)
{
        u32 val;

        val = *(volatile u_int32_t *)(RM9000x2_HTLINK_REG);
        if (val & 0x00000020)
                return 1;
        else
                return 0;
}

static int titan_ht_config_read_dword(struct pci_dev *device,
                                             int offset, u32* val)
{
        int dev, bus, func;
        volatile uint32_t address;
	unsigned long reg_data;

        bus = device->bus->number;
        dev = PCI_SLOT(device->devfn);
        func = PCI_FUNC(device->devfn);

	if (bus != 0)
		address = (bus << 16) | (dev << 11) | (func << 8) | 
				(offset & 0xfc) | 0x80000000 | 0x1;
	else
		address = (bus << 16) | (dev << 11) | (func << 8) |
				(offset & 0xfc) | 0x80000000;
	
	/*
	 * RM9000 HT Errata: Issue back to back HT config
	 * transcations. Issue a BIU sync before and 
	 * after the HT cycle
	 */

	reg_data = *(volatile u_int32_t *)(0xfb0000f0);
	reg_data |= 0x2;
	*(volatile u_int32_t *)(0xfb0000f0) = reg_data;

	udelay(30);

	*(volatile u_int32_t *)(0xfb0006f8) = address;
	*(val) = *(volatile u_int32_t *)(0xfb0006fc);

	udelay(30);

	reg_data = *(volatile u_int32_t *)(0xfb0000f0);
        reg_data |= 0x2;
        *(volatile u_int32_t *)(0xfb0000f0) = reg_data;


        return PCIBIOS_SUCCESSFUL;
}


static int titan_ht_config_read_word(struct pci_dev *device,
                                             int offset, u16* val)
{
        int dev, bus, func;
        volatile uint32_t address_reg, data_reg;
        uint32_t address, *val1 = kmalloc(sizeof(uint32_t), GFP_KERNEL);

        bus = device->bus->number;
        dev = PCI_SLOT(device->devfn);
        func = PCI_FUNC(device->devfn);

	address =  (bus << 16) | (dev << 11) | (func << 8) | 
			(offset & 0xfc) | 0x80000000;

        address_reg = RM9000x2_OCD_HTCFGA;
        data_reg =  RM9000x2_OCD_HTCFGD;

	titan_ht_config_read_dword(device, offset, val1);

        if ((offset & 0x3) == 0)
		*val = (*val1 & 0x0000ffff);
        else
		*val = (*val1 & 0xffff0000) >> 16;

        return PCIBIOS_SUCCESSFUL;
}


u32 longswap(unsigned long l)
{
        unsigned char b1,b2,b3,b4;

        b1 = l&255;
        b2 = (l>>8)&255;
        b3 = (l>>16)&255;
        b4 = (l>>24)&255;

        return ((b1<<24) + (b2<<16) + (b3<<8) + b4);
}


static int titan_ht_config_read_byte(struct pci_dev *device,
                                             int offset, u8* val)
{
        int dev, bus, func;
        volatile uint32_t address_reg, data_reg;
        uint32_t address, *val1 = kmalloc(sizeof(uint32_t), GFP_KERNEL);

        bus = device->bus->number;
        dev = PCI_SLOT(device->devfn);
        func = PCI_FUNC(device->devfn);

        address = (bus << 16) | (dev << 11) | (func << 8) | 
			(offset & 0xfc) | 0x80000000;

        address_reg = RM9000x2_OCD_HTCFGA;
        data_reg =  RM9000x2_OCD_HTCFGD;

	titan_ht_config_read_dword(device, offset, val1);

        if ((offset & 0x3) == 0) {
		*val = (*val1 & 0x000000ff);
        }
        if ((offset & 0x3) == 1) {
		*val = ((*val1 & 0x0000ff00) >> 8);
        }
        if ((offset & 0x3) == 2) {
		*val = ((*val1 & 0x00ff0000) >> 16);
        }
        if ((offset & 0x3) == 3) {
		*val = ((*val1 & 0xff000000) >> 24);
        } 

        return PCIBIOS_SUCCESSFUL;
}


static int titan_ht_config_write_dword(struct pci_dev *device,
                                             int offset, u32 val)
{
        int dev, bus, func;
        volatile uint32_t address;
	unsigned long reg_data;

        bus = device->bus->number;
        dev = PCI_SLOT(device->devfn);
        func = PCI_FUNC(device->devfn);

	if (bus != 0)        
		address = (bus << 16) | (dev << 11) | (func << 8) | 
				(offset & 0xfc) | 0x80000000 | 0x1;
	else
		address = (bus << 16) | (dev << 11) | (func << 8) |
				(offset & 0xfc) | 0x80000000;

	reg_data = *(volatile u_int32_t *)(0xfb0000f0);
        reg_data |= 0x2;
        *(volatile u_int32_t *)(0xfb0000f0) = reg_data;
	
	udelay(30);

	*(volatile u_int32_t *)(0xfb0006f8) = address;
        *(volatile u_int32_t *)(0xfb0006fc) = val;

	udelay(30);

	reg_data = *(volatile u_int32_t *)(0xfb0000f0);
        reg_data |= 0x2;
        *(volatile u_int32_t *)(0xfb0000f0) = reg_data;

        return PCIBIOS_SUCCESSFUL;
}

static int titan_ht_config_write_word(struct pci_dev *device,
                                             int offset, u16 val)
{
        int dev, bus, func;
        volatile uint32_t address_reg, data_reg;
        uint32_t address, val1, val2;

        bus = device->bus->number;
        dev = PCI_SLOT(device->devfn);
        func = PCI_FUNC(device->devfn);

        address = (bus << 16) | (dev << 11) | (func << 8) | 
			(offset & 0xfc) | 0x80000000;

        address_reg = RM9000x2_OCD_HTCFGA;
        data_reg =  RM9000x2_OCD_HTCFGD;

	titan_ht_config_read_dword(device, offset, &val2);

        if ((offset & 0x3) == 0) {
		val2 = ( (val2 & ~(0x0000ffff)) | val);
	}
	else {
		val1 = val;
		val1 = (val1 << 16);
		val2 = ( (val2 & ~(0xffff0000)) | val1);
	}

	titan_ht_config_write_dword(device, offset, val2);

        return PCIBIOS_SUCCESSFUL;
}

static int titan_ht_config_write_byte(struct pci_dev *device,
                                             int offset, u8 val)
{
        int dev, bus, func;
        volatile uint32_t address_reg, data_reg;
        uint32_t address, val1, val2;
        int offset1;

        bus = device->bus->number;
        dev = PCI_SLOT(device->devfn);
        func = PCI_FUNC(device->devfn);

        address = (bus << 16) | (dev << 11) | (func << 8) | 
			(offset & 0xfc) | 0x80000000;

        address_reg = RM9000x2_OCD_HTCFGA;
        data_reg =  RM9000x2_OCD_HTCFGD;

	titan_ht_config_read_dword(device, offset, &val2);

        if ((offset & 0x3) == 0) {
		val1 = val;
		val2 = ( (val2 & ~(0x000000ff)) | val1);
        }
        if ((offset & 0x3) == 1) {
		val1 = val;
		val1 = val1 << 8;
		val2 = ( (val2 & ~(0x0000ff00)) | val1);
        }
        if ((offset & 0x3) == 2) {
		val1 = val;
		val1 = val1 << 16;
		val2 = ( (val2 & ~(0x00ff0000)) | val1);
        }
        if ((offset & 0x3) == 3) {
		val1 = val;
		val1 = val1 << 24;
		val2 = ( (val2 & ~(0xff000000)) | val1);
        }

	titan_ht_config_write_dword(device, offset, val2);
        return PCIBIOS_SUCCESSFUL;
}

/*
 * Bus mastering capabilities for the HT device
 */
static void titan_pcibios_set_master(struct pci_dev *dev)
{
        u16 cmd;
        int bus = dev->bus->number;

	if (check_titan_htlink())
            titan_ht_config_read_word(dev, PCI_COMMAND, &cmd);

	cmd |= PCI_COMMAND_MASTER;

	if (check_titan_htlink())
            titan_ht_config_write_word(dev, PCI_COMMAND, cmd);
}


int pcibios_enable_resources(struct pci_dev *dev)
{
        u16 cmd, old_cmd;
        u8 tmp1;
	u32 reg_data;
        int idx;
        struct resource *r;
        int bus = dev->bus->number;

	if (check_titan_htlink())
            titan_ht_config_read_word(dev, PCI_COMMAND, &cmd);

	old_cmd = cmd;
        for (idx = 0; idx < 6; idx++) {
                r = &dev->resource[idx];
                if (!r->start && r->end) {
			printk("%x    %x \n", r->start,  r->end);
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
        if (cmd != old_cmd) {
		if (check_titan_htlink())
                   titan_ht_config_write_word(dev, PCI_COMMAND, cmd);
	}

	if (check_titan_htlink())
		titan_ht_config_read_byte(dev, PCI_CACHE_LINE_SIZE, &tmp1);

	if (tmp1 != 8) {
                printk(KERN_WARNING "PCI setting cache line size to 8 from "
                       "%d\n", tmp1);
	}

	if (check_titan_htlink())
		titan_ht_config_write_byte(dev, PCI_CACHE_LINE_SIZE, 8);

	if (check_titan_htlink())
		titan_ht_config_read_byte(dev, PCI_LATENCY_TIMER, &tmp1);

	if (tmp1 < 32 || tmp1 == 0xff) {
                printk(KERN_WARNING "PCI setting latency timer to 32 from %d\n",
                       tmp1);
	}

	if (check_titan_htlink())
		titan_ht_config_write_byte(dev, PCI_LATENCY_TIMER, 32);

	return 0;
}


int pcibios_enable_device(struct pci_dev *dev, int mask)
{
        return pcibios_enable_resources(dev);
}

void pcibios_update_resource(struct pci_dev *dev, struct resource *root,
                             struct resource *res, int resource)
{
        u32 new, check;
        int reg;

        return;

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


void pcibios_align_resource(void *data, struct resource *res,
                            unsigned long size, unsigned long align)
{
        struct pci_dev *dev = data;

        if (res->flags & IORESOURCE_IO) {
                unsigned long start = res->start;

                /* We need to avoid collisions with `mirrored' VGA ports
                   and other strange ISA hardware, so we always want the
                   addresses kilobyte aligned.  */
                if (size > 0x100) {
                        printk(KERN_ERR "PCI: I/O Region %s/%d too large"
                               " (%ld bytes)\n", dev->slot_name,
                                dev->resource - res, size);
                }

                start = (start + 1024 - 1) & ~(1024 - 1);
                res->start = start;
        }
}

/*
 * PCI structure
 */
struct pci_ops titan_pci_ops = {
        titan_ht_config_read_byte,
        titan_ht_config_read_word,
        titan_ht_config_read_dword,
        titan_ht_config_write_byte,
        titan_ht_config_write_word,
        titan_ht_config_write_dword
};


struct pci_fixup pcibios_fixups[] = {
        {0}
};

void __init pcibios_fixup_bus(struct pci_bus *c)
{
        titan_ht_pcibios_fixup_bus(c);
}

void __init pcibios_init(void)
{
	/*
	 * IO space accesses for the device
	 */
	set_io_port_base(KSEG1);

	/* 
	 * IO and MEM space values
	 */
	ioport_resource.start = 0xe0000000;
        ioport_resource.end   = 0xe0000000 + 0x01000000 - 1;
        iomem_resource.start  = 0xd0000000;
        iomem_resource.end    = 0xd0000000 + 0x0f000000 - 1;

        pci_scan_bus(0, &titan_pci_ops, NULL);
}

/*
 * for parsing "pci=" kernel boot arguments.
 */
char *pcibios_setup(char *str)
{
        printk(KERN_INFO "rr: pcibios_setup\n");

        return str;
}

unsigned __init int pcibios_assign_all_busses(void)
{
        return 1;
}

#endif /* CONFIG_HYPERTRANSPORT */
