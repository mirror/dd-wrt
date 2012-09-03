/*
 * Northstar PCI-Express driver
 * Only supports Root-Complex (RC) mode
 *
 * Notes:
 * PCI Domains are being used to identify the PCIe port 1:1.
 *
 * Only MEM access is supported, PAX does not support IO.
 *
 * TODO:
 *	MSI interrupts,
 *	DRAM > 128 MBytes (e.g. DMA zones)
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <mach/memory.h>
#include <mach/io_map.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <siutils.h>
#include <hndcpu.h>
#include <hndpci.h>
#include <pcicfg.h>
#include <bcmdevs.h>
#include <bcmnvram.h>

/* Global SB handle */
extern si_t *bcm947xx_sih;
extern spinlock_t bcm947xx_sih_lock;

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock


/*
 * Register offset definitions
 */
#define	SOC_PCIE_CONTROL	0x000	/* a.k.a. CLK_CONTROL reg */
#define	SOC_PCIE_PM_STATUS	0x008
#define	SOC_PCIE_PM_CONTROL	0x00c	/* in EP mode only ! */

#define	SOC_PCIE_EXT_CFG_ADDR	0x120
#define	SOC_PCIE_EXT_CFG_DATA	0x124
#define	SOC_PCIE_CFG_ADDR	0x1f8
#define	SOC_PCIE_CFG_DATA	0x1fc

#define	SOC_PCIE_SYS_RC_INTX_EN		0x330
#define	SOC_PCIE_SYS_RC_INTX_CSR	0x334
#define	SOC_PCIE_SYS_HOST_INTR_EN	0x344
#define	SOC_PCIE_SYS_HOST_INTR_CSR	0x348

#define	SOC_PCIE_HDR_OFF	0x400	/* 256 bytes per function */

/* 32-bit 4KB in-bound mapping windows for Function 0..3, n=0..7 */
#define	SOC_PCIE_SYS_IMAP0(f,n)		(0xc00+((f)<<9)((n)<<2)) 
/* 64-bit in-bound mapping windows for func 0..3 */
#define	SOC_PCIE_SYS_IMAP1(f)		(0xc80+((f)<<3))
#define	SOC_PCIE_SYS_IMAP2(f)		(0xcc0+((f)<<3))
/* 64-bit in-bound address range n=0..2 */
#define	SOC_PCIE_SYS_IARR(n)		(0xd00+((n)<<3))
/* 64-bit out-bound address filter n=0..2 */
#define	SOC_PCIE_SYS_OARR(n)		(0xd20+((n)<<3))
/* 64-bit out-bound mapping windows n=0..2 */
#define	SOC_PCIE_SYS_OMAP(n)		(0xd40+((n)<<3))

#ifdef	__nonexistent_regs_
#define	SOC_PCIE_MDIO_CONTROL	0x128
#define	SOC_PCIE_MDIO_RD_DATA	0x12c
#define	SOC_PCIE_MDIO_WR_DATA	0x130
#define	SOC_PCIE_CLK_STAT	0x1e0 
#endif

extern int _memsize;

#ifdef	CONFIG_PCI

/*
 * Forward declarations
 */
static int soc_pci_setup(int nr, struct pci_sys_data *sys);
static struct pci_bus * soc_pci_scan_bus(int nr, struct pci_sys_data *sys);
static int soc_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin);
static int soc_pci_read_config(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, u32 *val);
static int soc_pci_write_config(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 val);

#ifndef	CONFIG_PCI_DOMAINS
#error	CONFIG_PCI_DOMAINS is required
#endif


static int
sbpci_read_config_reg(struct pci_bus *bus, unsigned int devfn, int where,
                      int size, u32 *value)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&sih_lock, flags);
	ret = hndpci_read_config(sih, bus->number, PCI_SLOT(devfn),
	                        PCI_FUNC(devfn), where, value, size);
	spin_unlock_irqrestore(&sih_lock, flags);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static int
sbpci_write_config_reg(struct pci_bus *bus, unsigned int devfn, int where,
                       int size, u32 value)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&sih_lock, flags);
	ret = hndpci_write_config(sih, bus->number, PCI_SLOT(devfn),
	                         PCI_FUNC(devfn), where, &value, size);
	spin_unlock_irqrestore(&sih_lock, flags);
	return ret ? PCIBIOS_DEVICE_NOT_FOUND : PCIBIOS_SUCCESSFUL;
}

static struct pci_ops pcibios_ops = {
	sbpci_read_config_reg,
	sbpci_write_config_reg
};

/*
 * PCIe host controller registers
 * one entry per port
 */
static struct resource soc_pcie_regs[3] = {
	{
	.name = "pcie0",
	.start = 0x18012000,
	.end   = 0x18012fff,
	.flags = IORESOURCE_MEM,
	},
	{
	.name = "pcie1",
	.start = 0x18013000,
	.end   = 0x18013fff,
	.flags = IORESOURCE_MEM,
	},
	{
	.name = "pcie2",
	.start = 0x18014000,
	.end   = 0x18014fff,
	.flags = IORESOURCE_MEM,
	},
};

static struct resource soc_pcie_owin[3] = {
	{
	.name = "PCIe Outbound Window, Port 0",
	.start = 0x08000000,
	.end =   0x08000000 + SZ_128M - 1,
	.flags = IORESOURCE_MEM,
	},
	{
	.name = "PCIe Outbound Window, Port 1",
	.start = 0x40000000,
	.end =   0x40000000 + SZ_128M - 1,
	.flags = IORESOURCE_MEM,
	},
	{
	.name = "PCIe Outbound Window, Port 2",
	.start = 0x48000000,
	.end =   0x48000000 + SZ_128M - 1,
	.flags = IORESOURCE_MEM,
	},
};
/*
static struct resource soc_pcie_owin[3] = {
	{
	.name = "PCIe Outbound Window, Port 0",
	.start = 0x10000000,
	.end =   0x10000000 + SZ_128M - 1,
	.flags = IORESOURCE_MEM,
	},
	{
	.name = "PCIe Outbound Window, Port 1",
	.start = 0x40000000,
	.end =   0x40000000 + SZ_128M - 1,
	.flags = IORESOURCE_MEM,
	},
	{
	.name = "PCIe Outbound Window, Port 2",
	.start = 0x48000000,
	.end =   0x48000000 + SZ_128M - 1,
	.flags = IORESOURCE_MEM,
	},
};
*/
struct pci_bus __init *root_scan_bus(int nr, struct pci_sys_data *sys)
{
	pci_scan_root_bus(NULL, 0, &pcibios_ops, sys, &sys->resources);
}
static int __init dummy_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	return 0;
}
    
/*
 * Per port control structure
 */
static struct soc_pcie_port {
	struct resource * regs_res ;
	struct resource * owin_res ;
	void * __iomem reg_base;
	unsigned short irqs[6];
	struct hw_pci hw_pci ;

	bool	enable;
	bool	link;
} soc_pcie_ports[4] = {
	{
	.irqs = {0, 0, 0, 0, 0, 0},
	.regs_res = & soc_pcie_regs[0],
	.owin_res = & soc_pcie_owin[0],
	.hw_pci = {
		.domain 	= 0,
		.swizzle	= NULL,
		.nr_controllers = 1,
		.map_irq	= NULL,
		},
	.enable = 1,
	},
	{
	.regs_res = & soc_pcie_regs[0],
	.owin_res = & soc_pcie_owin[0],
	.irqs = {158, 159, 160, 161, 162, 163},
	.hw_pci = {
		.domain 	= 1,
		.swizzle 	= NULL,
		.nr_controllers = 1,
		.setup 		= soc_pci_setup,
		.scan 		= soc_pci_scan_bus,
		.map_irq 	= soc_pcie_map_irq,
		},
	.enable = 1,
	},
	{
	.regs_res = & soc_pcie_regs[1],
	.owin_res = & soc_pcie_owin[1],
	.irqs = {164, 165, 166, 167, 168, 169},
	.hw_pci = {
		.domain 	= 2,
		.swizzle 	= NULL,
		.nr_controllers = 1,
		.setup 		= soc_pci_setup,
		.scan 		= soc_pci_scan_bus,
		.map_irq 	= soc_pcie_map_irq,
		},
	.enable = 1,
	},
	{
	.regs_res = & soc_pcie_regs[2],
	.owin_res = & soc_pcie_owin[2],
	.irqs = {170, 171, 172, 173, 174, 175},
	.hw_pci = {
		.domain 	= 3,
		.swizzle 	= NULL,
		.nr_controllers = 1,
		.setup 		= soc_pci_setup,
		.scan 		= soc_pci_scan_bus,
		.map_irq 	= soc_pcie_map_irq,
		},
	.enable = 1,
	}
	};

/*
 * Methods for accessing configuration registers
 */
static struct pci_ops soc_pcie_ops = {
        .read = soc_pci_read_config,
        .write = soc_pci_write_config,
};

/*
 * Per hnd si bus devices irq map
 */
typedef struct si_bus_irq_map {
	unsigned short device;
	unsigned short unit;
	unsigned short max_unit;
	unsigned short irq;
} si_bus_irq_map_t;

si_bus_irq_map_t si_bus_irq_map[] = {
	{BCM47XX_GMAC_ID, 0, 4, 179}	/* 179, 180, 181, 182 */,
	{BCM47XX_USB20H_ID, 0, 1, 111}	/* 111 */,
	{BCM47XX_USB30H_ID, 0, 5, 112}	/* 112, 113, 114, 115, 116 */
};
#define SI_BUS_IRQ_MAP_SIZE (sizeof(si_bus_irq_map) / sizeof(si_bus_irq_map_t))

static int si_bus_map_irq(struct pci_dev *pdev)
{
        int i, irq = 0;

	for (i = 0; i < SI_BUS_IRQ_MAP_SIZE; i++) {
		if (pdev->device == si_bus_irq_map[i].device &&
		    si_bus_irq_map[i].unit < si_bus_irq_map[i].max_unit) {
			irq = si_bus_irq_map[i].irq + si_bus_irq_map[i].unit;
			si_bus_irq_map[i].unit++;
			break;
		}
	}

        return irq;
}

static struct soc_pcie_port * 
	soc_pcie_sysdata2port( struct pci_sys_data * sysdata )
{
	unsigned port;

	port = sysdata->domain;
	BUG_ON( port >= ARRAY_SIZE( soc_pcie_ports ));
	return & soc_pcie_ports[ port ];
}

static struct soc_pcie_port * soc_pcie_pdev2port(const struct pci_dev *pdev )
{
	return soc_pcie_sysdata2port( pdev->sysdata );
}

static struct soc_pcie_port * soc_pcie_bus2port( struct pci_bus * bus )
{
	return soc_pcie_sysdata2port( bus->sysdata );
}

static struct pci_bus *soc_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
	return pci_scan_root_bus(NULL, sys->busnr, &soc_pcie_ops, sys, &sys->resources);
}

static int soc_pcie_map_irq(const struct pci_dev *pdev, u8 slot, u8 pin)
{
        struct soc_pcie_port *port = soc_pcie_pdev2port(pdev);
        int irq;

        irq = port->irqs[5];	/* All INTx share int src 5, last per port */

        pr_debug("PCIe map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
                pci_domain_nr(pdev->bus), 
		pdev->bus->number, 
		PCI_SLOT(pdev->devfn),
                PCI_FUNC(pdev->devfn), 
		slot, pin, irq);

        return irq;
}

static void __iomem * soc_pci_cfg_base(struct pci_bus *bus,
                                  unsigned int devfn, int where)
{
        struct soc_pcie_port *port = soc_pcie_bus2port(bus);
        int busno = bus->number;
        int slot = PCI_SLOT(devfn);
        int fn  = PCI_FUNC(devfn);
        void __iomem *base;
        int offset;
        int type;
	u32 addr_reg ;

	base = port->reg_base ;

        /* If there is no link, just show the PCI bridge. */
        if (!port->link && (busno > 0 || slot > 0))
                return NULL;
        /*
         */
	if (busno == 0) {
                if (slot >= 1)
                        return NULL;
                type = slot;
		__raw_writel( where & 0xffc, base + SOC_PCIE_EXT_CFG_ADDR );
		offset = SOC_PCIE_EXT_CFG_DATA;
	} else {
                type = 1;
		addr_reg = 	(busno & 0xff) << 20 |
				(slot << 15) |
				(fn << 12)   |
				(where & 0xffc) |
				(type & 0x3);
 
		__raw_writel( addr_reg, base + SOC_PCIE_CFG_ADDR );
		offset =  SOC_PCIE_CFG_DATA ;
        }

        return base + offset;
}

static int soc_pci_read_config(struct pci_bus *bus, unsigned int devfn,
                                   int where, int size, u32 *val)
{
        void __iomem *base;
	u32 data_reg ;

	base = soc_pci_cfg_base(bus, devfn, where);

        if (base == NULL )
		{
                *val = ~0UL;
                return PCIBIOS_SUCCESSFUL;
		}

	data_reg = __raw_readl( base );

	/* NS: CLASS field is R/O, and set to wrong 0x200 value */
	if( bus->number == 0 && devfn == 0 ) {
		if ( (where & 0xffc) == PCI_CLASS_REVISION) {
                /*
                 * RC's class is 0x0280, but Linux PCI driver needs 0x604
                 * for a PCIe bridge. So we must fixup the class code
                 * to 0x604 here.
                 */
			data_reg &= 0xff;
			data_reg |= 0x604 << 16;
		}
        }
	/* HEADER_TYPE=00 indicates the port in EP mode */

	if( size == 4 )
		{
		*val = data_reg;
		}
	else if( size < 4 )
		{
		u32 mask = (1 << (size * 8)) - 1;
		int shift = (where % 4) * 8;
		*val = (data_reg >> shift) & mask;
		}

        return PCIBIOS_SUCCESSFUL;
}

static int soc_pci_write_config(struct pci_bus *bus, unsigned int devfn,
                                    int where, int size, u32 val)
{
        void __iomem *base;
	u32  data_reg ;

	base = soc_pci_cfg_base(bus, devfn, where);
        if (base == NULL)
		{
                return PCIBIOS_SUCCESSFUL;
		}

	if( size < 4 )
		{
		u32 mask = (1 << (size * 8)) - 1;
		int shift = (where % 4) * 8;
		data_reg = __raw_readl( base );
		data_reg &= ~(mask & shift);
		data_reg |=  (val & mask) << shift;
		}
	else
		{
		data_reg = val;
		}

	__raw_writel( data_reg, base );

        return PCIBIOS_SUCCESSFUL;
}

static int soc_pci_setup(int nr, struct pci_sys_data *sys)
{
        struct soc_pcie_port *port = soc_pcie_sysdata2port(sys);
	request_resource( &iomem_resource, port->owin_res );
	pci_add_resource_offset(&sys->resources, port->owin_res, sys->mem_offset);
	
	sys->private_data = port;
        return 1;
}

/*
 * Check link status, return 0 if link is up in RC mode,
 * otherwise return non-zero
 */
static int __init noinline soc_pcie_check_link(struct soc_pcie_port * port)
{
        u32 devfn = 0;
	u16 pos, tmp16;
	u8 nlw, tmp8;

        struct pci_sys_data sd = {
                .domain = port->hw_pci.domain,
        };
        struct pci_bus bus = {
                .number = 0,
                .ops = &soc_pcie_ops,
                .sysdata = &sd,
        };

	if( ! port->enable )
		return -EINVAL;

	/* See if the port is in EP mode, indicated by header type 00 */
        pci_bus_read_config_byte(&bus, devfn, PCI_HEADER_TYPE, &tmp8);
	if( tmp8 != PCI_HEADER_TYPE_BRIDGE ) {
		pr_info("PCIe port %d in End-Point mode - ignored\n",
			port->hw_pci.domain );
		return -ENODEV;
	}

	/* NS PAX only changes NLW field when card is present */
        pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
        pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_LNKSTA, &tmp16);

#ifdef	DEBUG
	pr_debug("PCIE%d: LINKSTA reg %#x val %#x\n", port->hw_pci.domain,
		pos+PCI_EXP_LNKSTA, tmp16 );
#endif

	nlw = (tmp16 & PCI_EXP_LNKSTA_NLW) >> PCI_EXP_LNKSTA_NLW_SHIFT ;
	port->link = tmp16 & PCI_EXP_LNKSTA_DLLLA ;

	if( nlw != 0 ) port->link = 1;

#ifdef	DEBUG
	for( ; pos < 0x100; pos += 2 )
		{
        	pci_bus_read_config_word(&bus, devfn, pos , &tmp16);
		if( tmp16 ) pr_debug("reg[%#x]=%#x, ", pos , tmp16 );
		}
#endif
	printk(KERN_INFO "PCIE%d link=%d\n", port->hw_pci.domain,  port->link );

	return( (port->link)? 0: -ENOSYS );
}

/*
 * Initializte the PCIe controller
 */
static void __init soc_pcie_hw_init(struct soc_pcie_port * port)
{
	/* Turn-on Root-Complex (RC) mode, from reset defailt of EP */

	/* The mode is set by straps, can be overwritten via DMU
	   register <cru_straps_control> bit 5, "1" means RC
	 */

	/* Send a downstream reset */
	__raw_writel( 0x3, port->reg_base + SOC_PCIE_CONTROL);
	udelay(250);
	__raw_writel( 0x1, port->reg_base + SOC_PCIE_CONTROL);
	mdelay(250);

	/* TBD: take care of PM, check we're on */
}

/*
 * Setup the address translation
 */
static void __init soc_pcie_map_init(struct soc_pcie_port * port)
{
	unsigned size, i ;
	u32 addr;

	/*
	 * NOTE:
	 * All PCI-to-CPU address mapping are 1:1 for simplicity
	 */

	/* Outbound address translation setup */
	size = resource_size(port->owin_res);
	addr = port->owin_res->start;
	BUG_ON( !addr );
	BUG_ON( addr & ((1<<25)-1) );	/* 64MB alignment */

	for(i=0 ; i < 3; i++)
		{
		const unsigned win_size = SZ_64M;
		/* 64-bit LE regs, write low word, high is 0 at reset */
		__raw_writel( addr,	port->reg_base + SOC_PCIE_SYS_OMAP(i));
		__raw_writel( addr|0x1,	port->reg_base + SOC_PCIE_SYS_OARR(i));
		addr += win_size;
		if( size >= win_size )
			size -= win_size;
		if( size == 0 )
			break;
		}
	WARN_ON( size > 0 );

	/* 
	 * Inbound address translation setup
	 * Northstar only maps up to 128 MiB inbound, DRAM could be up to 1 GiB.
	 *
	 * For now allow access to entire DRAM, assuming it is less than 128MiB,
	 * otherwise DMA bouncing mechanism may be required.
	 * Also consider DMA mask to limit DMA physical address
	 */
	size = _memsize;
	addr = PHYS_OFFSET;
	BUG_ON( size > SZ_128M );

	size >>= 20;	/* In MB */
	size &= 0xff;	/* Size is an 8-bit field */

	WARN_ON( size == 0 );
	/* 64-bit LE regs, write low word, high is 0 at reset */
	__raw_writel(addr | size,
		port->reg_base + SOC_PCIE_SYS_IMAP1(0));
	__raw_writel(addr | 0x1,
		port->reg_base + SOC_PCIE_SYS_IARR(1));

}

/*
 * Setup PCIE Host bridge
 */
static void __init noinline soc_pcie_bridge_init(struct soc_pcie_port * port)
{
        u32 devfn = 0;
        u8 tmp8;
	u16 tmp16;
	/* Fake <bus> object */
        struct pci_sys_data sd = {
                .domain = port->hw_pci.domain,
        };
        struct pci_bus bus = {
                .number = 0,
                .ops = &soc_pcie_ops,
                .sysdata = &sd,
        };


        pci_bus_write_config_byte(&bus, devfn, PCI_PRIMARY_BUS, 0);
        pci_bus_write_config_byte(&bus, devfn, PCI_SECONDARY_BUS, 1);
        pci_bus_write_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, 1);

        pci_bus_read_config_byte(&bus, devfn, PCI_PRIMARY_BUS, &tmp8);
        pci_bus_read_config_byte(&bus, devfn, PCI_SECONDARY_BUS, &tmp8);
        pci_bus_read_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, &tmp8);

	/* MEM_BASE, MEM_LIM require 1MB alignment */
	BUG_ON( (port->owin_res->start   >> 16) & 0xf );
//#ifdef	DEBUG
	printk(KERN_DEBUG "%s: membase %#x memlimit %#x\n", __FUNCTION__,
		port->owin_res->start, port->owin_res->end+1);
//#endif
        pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_BASE, port->owin_res->start   >> 16 );
	BUG_ON(((port->owin_res->end+1) >> 16 ) & 0xf );
        pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_LIMIT, (port->owin_res->end+1) >> 16 );

	/* These registers are not supported on the NS */
        pci_bus_write_config_word(&bus, devfn, PCI_IO_BASE_UPPER16, 0);
        pci_bus_write_config_word(&bus, devfn, PCI_IO_LIMIT_UPPER16, 0);

	/* Force class to that of a Bridge */
        pci_bus_write_config_word(&bus, devfn, PCI_CLASS_DEVICE,
		PCI_CLASS_BRIDGE_PCI);

        pci_bus_read_config_word(&bus, devfn, PCI_CLASS_DEVICE, &tmp16);
        pci_bus_read_config_word(&bus, devfn, PCI_MEMORY_BASE, &tmp16);
        pci_bus_read_config_word(&bus, devfn, PCI_MEMORY_LIMIT, &tmp16);
	
}


int
pcibios_enable_resources(struct pci_dev *dev)
{
	u16 cmd, old_cmd;
	int idx;
	struct resource *r;

	/* External PCI only */
	if (dev->bus->number == 0)
		return 0;

	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	old_cmd = cmd;
	for (idx = 0; idx < 6; idx++) {
		r = &dev->resource[idx];
		if (r->flags & IORESOURCE_IO)
			cmd |= PCI_COMMAND_IO;
		if (r->flags & IORESOURCE_MEM)
			cmd |= PCI_COMMAND_MEMORY;
	}
	if (dev->resource[PCI_ROM_RESOURCE].start)
		cmd |= PCI_COMMAND_MEMORY;
	if (cmd != old_cmd) {
		printk("PCI: Enabling device %s (%04x -> %04x)\n", pci_name(dev), old_cmd, cmd);
		pci_write_config_word(dev, PCI_COMMAND, cmd);
	}
	return 0;
}

static void
bcm5301x_usb_power_on(int coreid)
{
	int enable_usb;

	if (coreid == NS_USB20_CORE_ID) {
		enable_usb = getgpiopin(NULL, "usbport1", GPIO_PIN_NOTDEFINED);
		if (enable_usb != GPIO_PIN_NOTDEFINED) {
			int enable_usb_mask = 1 << enable_usb;

			si_gpioout(sih, enable_usb_mask, enable_usb_mask, GPIO_DRV_PRIORITY);
			si_gpioouten(sih, enable_usb_mask, enable_usb_mask, GPIO_DRV_PRIORITY);
		}

		enable_usb = getgpiopin(NULL, "usbport2", GPIO_PIN_NOTDEFINED);
		if (enable_usb != GPIO_PIN_NOTDEFINED) {
			int enable_usb_mask = 1 << enable_usb;

			si_gpioout(sih, enable_usb_mask, enable_usb_mask, GPIO_DRV_PRIORITY);
			si_gpioouten(sih, enable_usb_mask, enable_usb_mask, GPIO_DRV_PRIORITY);
		}
	}
	else if (coreid == NS_USB30_CORE_ID) {
		enable_usb = getgpiopin(NULL, "usbport2", GPIO_PIN_NOTDEFINED);
		if (enable_usb != GPIO_PIN_NOTDEFINED) {
			int enable_usb_mask = 1 << enable_usb;

			si_gpioout(sih, enable_usb_mask, enable_usb_mask, GPIO_DRV_PRIORITY);
			si_gpioouten(sih, enable_usb_mask, enable_usb_mask, GPIO_DRV_PRIORITY);
		}
	}
}

static void
bcm5301x_usb20_phy_init(void)
{
	uint32 dmu_base;
	uint32 *cru_clkset_key;
	uint32 *cru_usb2_control;

	/* Check Chip ID */
	if (CHIPID(sih->chip) != BCM4707_CHIP_ID)
		return;

	/* Check Package ID */
	if (sih->chippkg == BCM4709_PKG_ID) {
		return;
	}
	else if (sih->chippkg == BCM4707_PKG_ID || sih->chippkg == BCM4708_PKG_ID) {
		dmu_base = (uint32)REG_MAP(0x1800c000, 4096);
		cru_clkset_key = (uint32 *)(dmu_base + 0x180);
		cru_usb2_control = (uint32 *)(dmu_base + 0x164);

		/* unlock */
		writel(0x0000ea68, cru_clkset_key);

		/* fill value */
		writel(0x00dd10c3, cru_usb2_control);

		/* lock */
		writel(0x00000000, cru_clkset_key);

		REG_UNMAP(dmu_base);
	}
}

static void
bcm5301x_usb30_phy_init(void)
{
	uint32 ccb_mii_base;
	uint32 dmu_base;
	uint32 *ccb_mii_mng_ctrl_addr;
	uint32 *ccb_mii_mng_cmd_data_addr;
	uint32 *cru_rst_addr;

	/* Check Chip ID */
	if (CHIPID(sih->chip) != BCM4707_CHIP_ID)
		return;

	/* Reg map */
	ccb_mii_base = (uint32)REG_MAP(0x18003000, 4096);
	ccb_mii_mng_ctrl_addr = (uint32 *)(ccb_mii_base + 0x0);
	ccb_mii_mng_cmd_data_addr = (uint32 *)(ccb_mii_base + 0x4);

	dmu_base = (uint32)REG_MAP(0x1800c000, 4096);
	cru_rst_addr = (uint32 *)(dmu_base +  0x184);

	/* MDIO setting. set MDC-> MDCDIV is 7'd8 */
	writel(0x00000088, ccb_mii_mng_ctrl_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	/* Combo PHY reset from DMU */
	writel(0x00000033, cru_rst_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	/* Combo PHY reset out from DMU */
	writel(0x0000003b, cru_rst_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	/* PLL30 block register (base 0x8000) */
	writel(0x587e8000, ccb_mii_mng_cmd_data_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	writel(0x582a6400, ccb_mii_mng_cmd_data_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	/* To check PLL30 lock status */
	writel(0x587e8000, ccb_mii_mng_cmd_data_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	writel(0x68020000, ccb_mii_mng_cmd_data_addr);
	SPINWAIT(((readl(ccb_mii_mng_ctrl_addr) >> 8 & 1) == 1), 1000);

	/* Reg unmap */
	REG_UNMAP(ccb_mii_base);
	REG_UNMAP(dmu_base);
}

static void 
bcm5301x_usb_phy_init(int coreid)
{
	if (coreid == NS_USB20_CORE_ID) {
		bcm5301x_usb20_phy_init();
	}
	else if (coreid == NS_USB30_CORE_ID) {
		bcm5301x_usb30_phy_init();
	}
}

int
pcibios_enable_device(struct pci_dev *dev, int mask)
{
	ulong flags;
	uint coreidx, coreid;
	void *regs;
	int rc = -1;
	/* External PCI device enable */
	if (dev->bus->number != 0)
		return pcibios_enable_resources(dev);

	/* These cores come out of reset enabled */
	if (dev->device == NS_IHOST_CORE_ID ||
	    dev->device == CC_CORE_ID)
		return 0;
	spin_lock_irqsave(&sih_lock, flags);
	
	regs = si_setcoreidx(sih, PCI_SLOT(dev->devfn));
	coreidx = si_coreidx(sih);
	coreid = si_coreid(sih);

	if (!regs) {
		printk(KERN_ERR "WARNING! PCIBIOS_DEVICE_NOT_FOUND\n");
		goto out;
	}

	si_core_reset(sih, 0, 0);

	if (coreid == NS_USB20_CORE_ID || coreid == NS_USB30_CORE_ID) {
		/* Set gpio HIGH to turn on USB VBUS power */
		bcm5301x_usb_power_on(coreid);

		/* USB PHY init */
		bcm5301x_usb_phy_init(coreid);
	}

	rc = 0;
out:
	si_setcoreidx(sih, coreidx);
	spin_unlock_irqrestore(&sih_lock, flags);

	return rc;
}


bool __devinit
plat_fixup_bus(struct pci_bus *b)
{
	struct list_head *ln;
	struct pci_dev *d;
	u8 irq;

	printk("PCI: Fixing up bus %d\n", b->number);
    
	/* Fix up SB */
	//if (b->number == 0) {
	if (((struct pci_sys_data *)b->sysdata)->domain == 0) {
		for (ln = b->devices.next; ln != &b->devices; ln = ln->next) {
			d = pci_dev_b(ln);
			/* Fix up interrupt lines */
			pci_read_config_byte(d, PCI_INTERRUPT_LINE, &irq);
			d->irq = si_bus_map_irq(d);
			pci_write_config_byte(d, PCI_INTERRUPT_LINE, d->irq);
		}
		return TRUE;
	} else {
	}
	return FALSE;
}



static int __init soc_pcie_init(void)
{
        unsigned i;


	pcibios_min_io = 32;
	pcibios_min_mem = 32;

	hndpci_init(sih);

	/* Scan the SB bus */
	printk(KERN_INFO "PCI: scanning bus %x\n", 0);
	struct hw_pci *hw = &soc_pcie_ports[0].hw_pci;
	static struct pci_sys_data sys;
#ifdef CONFIG_PCI_DOMAINS
		sys.domain  = hw->domain;
#endif
		sys.busnr   = 0;
		sys.swizzle = hw->swizzle;
		sys.map_irq = hw->map_irq;

//		.domain 	= 0,
//		.swizzle	= NULL,
//		.nr_controllers = 1,
//		.map_irq 	= NULL,

	struct pci_bus *root_bus;
	root_bus = pci_scan_bus_parented(NULL, 0, &pcibios_ops, &sys);
	if (root_bus)
		pci_bus_add_devices(root_bus);


//	pci_scan_bus(0, &pcibios_ops, &sys);
//        pci_common_init( & soc_pcie_ports[0].hw_pci );

        for (i = 1; i < ARRAY_SIZE(soc_pcie_ports); i++)
	{
		struct soc_pcie_port *port = &soc_pcie_ports[i];
		
		/* Check if this port needs to be enabled */
		if( ! port->enable )
			continue;
		/* Setup PCIe controller registers */
		BUG_ON( request_resource( &iomem_resource, port->regs_res ));
		port->reg_base = ioremap( port->regs_res->start, resource_size(port->regs_res) );
		BUG_ON( IS_ERR_OR_NULL(port->reg_base ));

                soc_pcie_hw_init( port );
		soc_pcie_map_init( port );

		/*
		* Skip inactive ports -
		* will need to change this for hot-plugging
		*/
                if( soc_pcie_check_link( port ) != 0 )
			continue;

                soc_pcie_bridge_init( port );
		/* Announce this port to ARM/PCI common code */
			
                pci_common_init( & port->hw_pci );

		/* Setup virtual-wire interrupts */
		__raw_writel(0xf, port->reg_base + SOC_PCIE_SYS_RC_INTX_EN );

		/* Enable memory and bus master */
		__raw_writel(0x6, port->reg_base + SOC_PCIE_HDR_OFF + 4 );
        }

        /*pci_assign_unassigned_resources();*/
        return 0;
}

device_initcall(soc_pcie_init);

#endif	/* CONFIG_PCI */
