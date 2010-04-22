/*
 * Support for IBM PPC 405EP-based MagicBox board 
 * Copyright (C) 2006  Karol Lewandowski
 *
 * Heavily based on bubinga.c
 *
 * Author: SAW (IBM), derived from walnut.c.
 *         Maintained by MontaVista Software <source@mvista.com>
 *
 * 2003 (c) MontaVista Softare Inc.  This file is licensed under the
 * terms of the GNU General Public License version 2. This program is
 * licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#include <linux/init.h>
#include <linux/smp.h>
#include <linux/threads.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/blkdev.h>
#include <linux/pci.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/delay.h>

#include <asm/system.h>
#include <asm/pci-bridge.h>
#include <asm/processor.h>
#include <asm/machdep.h>
#include <asm/page.h>
#include <asm/time.h>
#include <asm/io.h>
#include <asm/kgdb.h>
#include <asm/ocp.h>
#include <asm/ibm_ocp_pci.h>

#include <platforms/4xx/ibm405ep.h>

#undef DEBUG

#ifdef DEBUG
#define DBG(x...) printk(x)
#else
#define DBG(x...)
#endif

extern bd_t __res;

/* Some IRQs unique to the board
 * Used by the generic 405 PCI setup functions in ppc4xx_pci.c
 */
int __init
ppc405_map_irq(struct pci_dev *dev, unsigned char idsel, unsigned char pin)
{
	static char pci_irq_table[][4] =
	    /*
	     *      PCI IDSEL/INTPIN->INTLINE
	     *      A       B       C       D
	     */
	{
		{28, 28, 28, 28},	/* IDSEL 1 - PCI slot 1 */
		{29, 29, 29, 29},	/* IDSEL 2 - PCI slot 2 */
		{30, 30, 30, 30},	/* IDSEL 3 - PCI slot 3 */
		{31, 31, 31, 31},	/* IDSEL 4 - PCI slot 4 */
	};

	const long min_idsel = 1, max_idsel = 4, irqs_per_slot = 4;
	return PCI_IRQ_TABLE_LOOKUP;
};

extern int early_serial_setup(struct uart_port *port);

/* The serial clock for the chip is an internal clock determined by
 * different clock speeds/dividers.
 * Calculate the proper input baud rate and setup the serial driver.
 */
static void __init
magicbox_early_serial_map(void)
{
	u32 uart_div;
	int uart_clock;
	struct uart_port port;

         /* Calculate the serial clock input frequency
          *
          * The base baud is the PLL OUTA (provided in the board info
          * structure) divided by the external UART Divisor, divided
          * by 16.
          */
	uart_div = (mfdcr(DCRN_CPC0_UCR_BASE) & DCRN_CPC0_UCR_U0DIV);
	uart_clock = __res.bi_procfreq / uart_div;

	/* Setup serial port access */
	memset(&port, 0, sizeof(port));
	port.membase = (void*)ACTING_UART0_IO_BASE;
	port.irq = ACTING_UART0_INT;
	port.uartclk = uart_clock;
	port.regshift = 0;
	port.iotype = UPIO_MEM;
	port.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST;
	port.line = 0;

	if (early_serial_setup(&port) != 0) {
		printk("Early serial init of port 0 failed\n");
	}

	port.membase = (void*)ACTING_UART1_IO_BASE;
	port.irq = ACTING_UART1_INT;
	port.line = 1;

	if (early_serial_setup(&port) != 0) {
		printk("Early serial init of port 1 failed\n");
	}
}

void __init
bios_fixup(struct pci_controller *hose, struct pcil0_regs *pcip)
{
#ifdef CONFIG_PCI

	unsigned int bar_response, bar;
	/*
	 * Expected PCI mapping:
	 *
	 *  PLB addr             PCI memory addr
	 *  ---------------------       ---------------------
	 *  0000'0000 - 7fff'ffff <---  0000'0000 - 7fff'ffff
	 *  8000'0000 - Bfff'ffff --->  8000'0000 - Bfff'ffff
	 *
	 *  PLB addr             PCI io addr
	 *  ---------------------       ---------------------
	 *  e800'0000 - e800'ffff --->  0000'0000 - 0001'0000
	 *
	 * The following code is simplified by assuming that the bootrom
	 * has been well behaved in following this mapping.
	 */

#ifdef DEBUG
	int i;

	printk("ioremap PCLIO_BASE = 0x%x\n", pcip);
	printk("PCI bridge regs before fixup \n");
	for (i = 0; i <= 3; i++) {
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].ma)));
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].la)));
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].pcila)));
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].pciha)));
	}
	printk(" ptm1ms\t0x%x\n", in_le32(&(pcip->ptm1ms)));
	printk(" ptm1la\t0x%x\n", in_le32(&(pcip->ptm1la)));
	printk(" ptm2ms\t0x%x\n", in_le32(&(pcip->ptm2ms)));
	printk(" ptm2la\t0x%x\n", in_le32(&(pcip->ptm2la)));

#endif

	/* added for IBM boot rom version 1.15 bios bar changes  -AK */

	/* Disable region first */
	out_le32((void *) &(pcip->pmm[0].ma), 0x00000000);
	/* PLB starting addr, PCI: 0x80000000 */
	out_le32((void *) &(pcip->pmm[0].la), 0x80000000);
	/* PCI start addr, 0x80000000 */
	out_le32((void *) &(pcip->pmm[0].pcila), PPC405_PCI_MEM_BASE);
	/* 512MB range of PLB to PCI */
	out_le32((void *) &(pcip->pmm[0].pciha), 0x00000000);
	/* Enable no pre-fetch, enable region */
	out_le32((void *) &(pcip->pmm[0].ma), ((0xffffffff -
						(PPC405_PCI_UPPER_MEM -
						 PPC405_PCI_MEM_BASE)) | 0x01));

	/* Disable region one */
	out_le32((void *) &(pcip->pmm[1].ma), 0x00000000);
	out_le32((void *) &(pcip->pmm[1].la), 0x00000000);
	out_le32((void *) &(pcip->pmm[1].pcila), 0x00000000);
	out_le32((void *) &(pcip->pmm[1].pciha), 0x00000000);
	out_le32((void *) &(pcip->pmm[1].ma), 0x00000000);
	out_le32((void *) &(pcip->ptm1ms), 0x00000001);

	/* Disable region two */
	out_le32((void *) &(pcip->pmm[2].ma), 0x00000000);
	out_le32((void *) &(pcip->pmm[2].la), 0x00000000);
	out_le32((void *) &(pcip->pmm[2].pcila), 0x00000000);
	out_le32((void *) &(pcip->pmm[2].pciha), 0x00000000);
	out_le32((void *) &(pcip->pmm[2].ma), 0x00000000);
	out_le32((void *) &(pcip->ptm2ms), 0x00000000);
	out_le32((void *) &(pcip->ptm2la), 0x00000000);

	/* Zero config bars */
	for (bar = PCI_BASE_ADDRESS_1; bar <= PCI_BASE_ADDRESS_2; bar += 4) {
		early_write_config_dword(hose, hose->first_busno,
					 PCI_FUNC(hose->first_busno), bar,
					 0x00000000);
		early_read_config_dword(hose, hose->first_busno,
					PCI_FUNC(hose->first_busno), bar,
					&bar_response);
		DBG("BUS %d, device %d, Function %d bar 0x%8.8x is 0x%8.8x\n",
		    hose->first_busno, PCI_SLOT(hose->first_busno),
		    PCI_FUNC(hose->first_busno), bar, bar_response);
	}
	/* end workaround */

#ifdef DEBUG
	printk("PCI bridge regs after fixup \n");
	for (i = 0; i <= 3; i++) {
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].ma)));
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].la)));
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].pcila)));
		printk(" pmm%dma\t0x%x\n", i, in_le32(&(pcip->pmm[i].pciha)));
	}
	printk(" ptm1ms\t0x%x\n", in_le32(&(pcip->ptm1ms)));
	printk(" ptm1la\t0x%x\n", in_le32(&(pcip->ptm1la)));
	printk(" ptm2ms\t0x%x\n", in_le32(&(pcip->ptm2ms)));
	printk(" ptm2la\t0x%x\n", in_le32(&(pcip->ptm2la)));

#endif
#endif
}

static void isp116x_delay(struct device *dev, int delay)
{
	udelay(delay);
}
#include <linux/usb/isp116x.h>


static struct isp116x_platform_data isp116x_platform_data = {
	.remote_wakeup_enable	= 1,
	.sel15Kres	= 1,
	.delay			= isp116x_delay,
};

static struct resource isp116x_pfm_resources[] = {
	[0] =	{
		.start	= 0xff300000,
		.end	= 0xff300000 + 1,
		.flags	= IORESOURCE_MEM,
		},
	[1] =	{
		.start  = 0xff300000 + 2,
		.end	= 0xff300000 + 3,
		.flags  = IORESOURCE_MEM,
		},
	[2] =	{
		.start	= 27,
		.end	= 27,
		.flags	= IORESOURCE_IRQ,
		},
};


static struct platform_device usb_device = {
	.name		= "isp116x-hcd",
	.num_resources	= ARRAY_SIZE(isp116x_pfm_resources),
	.resource	= isp116x_pfm_resources,
	.dev.platform_data = &isp116x_platform_data,
};

/*#ifdef ISP116X_HCD_SEL15kRES
	isp116x_board.sel15Kres = 1;
#endif
#ifdef ISP116X_HCD_OC_ENABLE
	isp116x_board.oc_enable = 1;
#endif
#ifdef ISP116X_HCD_REMOTE_WAKEUP_ENABLE
	isp116x_board.remote_wakeup_enable = 1;
#endif
*/

/**
 * hcWriteWord - write a 16 bit value into the USB controller
 * @base: base address to access the chip registers
 * @value: 16 bit value to write into register @offset
 * @offset: register to write the @value into
 *
 */
static void inline hcWriteWord (unsigned long base, unsigned int value,
				unsigned int offset)
{
	out_le16 ((volatile u16*)(base + 2), offset | 0x80);
	out_le16 ((volatile u16*)base, value);
}

/**
 * hcWriteDWord - write a 32 bit value into the USB controller
 * @base: base address to access the chip registers
 * @value: 32 bit value to write into register @offset
 * @offset: register to write the @value into
 *
 */

static void inline hcWriteDWord (unsigned long base, unsigned long value,
				unsigned int offset)
{
	out_le16 ((volatile u16*)(base + 2), offset | 0x80);
	out_le16 ((volatile u16*)base, value);
	out_le16 ((volatile u16*)base, value >> 16);
}

/**
 * hcReadWord - read a 16 bit value from the USB controller
 * @base: base address to access the chip registers
 * @offset: register to read from
 *
 * Returns the readed register value
 */

static unsigned int inline hcReadWord (unsigned long base, unsigned int offset)
{
	out_le16 ((volatile u16*)(base + 2), offset);
	return (in_le16 ((volatile u16*)base));
}

/**
 * hcReadDWord - read a 32 bit value from the USB controller
 * @base: base address to access the chip registers
 * @offset: register to read from
 *
 * Returns the readed register value
 */

static unsigned long inline hcReadDWord (unsigned long base, unsigned int offset)
{
	unsigned long val, val16;

	out_le16 ((volatile u16*)(base + 2), offset);
	val = in_le16((volatile u16*)base);
	val16 = in_le16((volatile u16*)base);
	return (val | (val16 << 16));
}

/* control and status registers isp1161 */
#define HcRevision		0x00
#define HcControl		0x01
#define HcCommandStatus		0x02
#define HcInterruptStatus	0x03
#define HcInterruptEnable	0x04
#define HcInterruptDisable	0x05
#define HcFmInterval		0x0D
#define HcFmRemaining		0x0E
#define HcFmNumber		0x0F
#define HcLSThreshold		0x11
#define HcRhDescriptorA		0x12
#define HcRhDescriptorB		0x13
#define HcRhStatus		0x14
#define HcRhPortStatus1		0x15
#define HcRhPortStatus2		0x16

#define HcHardwareConfiguration 0x20
#define HcDMAConfiguration	0x21
#define HcTransferCounter	0x22
#define HcuPInterrupt		0x24
#define HcuPInterruptEnable	0x25
#define HcChipID		0x27
#define HcScratch		0x28
#define HcSoftwareReset		0x29
#define HcITLBufferLength	0x2A
#define HcATLBufferLength	0x2B
#define HcBufferStatus		0x2C
#define HcReadBackITL0Length	0x2D
#define HcReadBackITL1Length	0x2E
#define HcITLBufferPort		0x40
#define HcATLBufferPort		0x41

static inline void out_le16(volatile unsigned short __iomem *addr, int val)
{
	__asm__ __volatile__("sync; sthbrx %1,0,%2" : "=m" (*addr) :
			      "r" (val), "r" (addr));
}

static inline int in_le16(const volatile unsigned short __iomem *addr)
{
	int ret;

	__asm__ __volatile__("sync; lhbrx %0,0,%1;\n"
			     "twi 0,%0,0;\n"
			     "isync" : "=r" (ret) :
			      "r" (addr), "m" (*addr));
	return ret;
}


static int magicbox_usb(void)
{
int i;
	/* PerCS3 (CF's CS1): base 0xff300000, 16-bit, rw */
	mtdcr(DCRN_EBC_BASE, 3);
	mtdcr(DCRN_EBC_BASE + 1, 0xff31a000);
	mtdcr(DCRN_EBC_BASE, 0x13);
	mtdcr(DCRN_EBC_BASE + 1, 0x080bd800);
	unsigned long usbBase= ioremap_nocache(0xff300000, 4096);

#define USB_CHIP_ENABLE 0x04
#define IDE_BOOSTING 0x40
printk(KERN_INFO "reset usb controler\n");
			hcWriteDWord (usbBase, 0x00000001, HcCommandStatus);
			for (i = 1000; i > 0; i--) {	/* loop up to 10 seconds */
				udelay (10);
				if (!(hcReadDWord (usbBase, HcCommandStatus) & 0x01))
					break;
			}

			if (!i)
			{
			printk(KERN_INFO "reset failed, do not register\n");
				return 0;  /* the controller doesn't responding. Broken? */
			}
		/*
		 * OK. USB controller is ready. Initialize it in such way the later driver
		 * can us it (without any knowing about specific implementation)
		 */
			hcWriteDWord (usbBase, 0x00000000, HcControl);
		/*
		 * disable all interrupt sources. Because we
		 * don't know where we come from (hard reset, cold start, soft reset...)
		 */
			hcWriteDWord (usbBase, 0x8000007D, HcInterruptDisable);
		/*
		 * our current setup hardware configuration
		 * - every port power supply can switched indepently
		 * - every port can signal overcurrent
		 * - every port is "outside" and the devices are removeable
		 */
			hcWriteDWord (usbBase, 0x32000902, HcRhDescriptorA);
			hcWriteDWord (usbBase, 0x00060000, HcRhDescriptorB);
		/*
		 * don't forget to switch off power supply of each port
		 * The later running driver can reenable them to find and use
		 * the (maybe) connected devices.
		 *
		 */
			hcWriteDWord (usbBase, 0x00000200, HcRhPortStatus1);
			hcWriteDWord (usbBase, 0x00000200, HcRhPortStatus2);
			hcWriteWord (usbBase, 0x0428, HcHardwareConfiguration);
			hcWriteWord (usbBase, 0x0040, HcDMAConfiguration);
			hcWriteWord (usbBase, 0x0000, HcuPInterruptEnable);
			hcWriteWord (usbBase, 0xA000 | (0x03 << 8) | 27, HcScratch);
printk(KERN_INFO "register usb host\n");



	platform_device_register(&usb_device);

	return 0;
};

arch_initcall (magicbox_usb);

void __init
magicbox_setup_arch(void)
{
	ppc4xx_setup_arch();

	ibm_ocp_set_emac(0, 1);

	magicbox_early_serial_map();

	/* Identify the system */
	printk("MagicBox port (C) 2005 Karol Lewandowski <kl@jasmine.eu.org>\n");
}

void __init
magicbox_map_io(void)
{
	ppc4xx_map_io();
}

void __init
platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
	      unsigned long r6, unsigned long r7)
{
	ppc4xx_init(r3, r4, r5, r6, r7);

	ppc_md.setup_arch = magicbox_setup_arch;
	ppc_md.setup_io_mappings = magicbox_map_io;

#ifdef CONFIG_KGDB
	ppc_md.early_serial_map = bubinga_early_serial_map;
#endif
}

