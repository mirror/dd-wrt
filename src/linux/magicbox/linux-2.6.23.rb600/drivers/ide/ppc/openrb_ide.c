#include <linux/version.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/delay.h>
#include <linux/platform_device.h>


#define UIC0_PR 0xc4
#define UIC0_TR 0xc5
#define IRQ 25

static int ide_offsets[IDE_NR_PORTS] = {0, 2, 4, 6, 8, 10, 12, 14, -1, -1};

static u8 openrb_ide_inb (unsigned long port)
{
	return (u8) (readw((void __iomem *) port) >> 8) & 255;
}

static u16 openrb_ide_inw (unsigned long port)
{
	return (u16) readw((void __iomem *) port);
}

static void openrb_ide_insw (unsigned long port, void *addr, u32 count)
{
	u16 *ptr;

	for (ptr = addr; count--; ptr++)
		*ptr = readw((void __iomem *) port);
}

static u32 openrb_ide_inl (unsigned long port)
{
	return (u32) readl((void __iomem *) port);
}

static void openrb_ide_insl (unsigned long port, void *addr, u32 count)
{
	u32 *ptr;

	for (ptr = addr; count--; ptr++)
		*ptr = readl((void __iomem *) port);
}

static void openrb_ide_outb (u8 value, unsigned long port)
{
	writew(value << 8, (void __iomem *) port);
}

static void openrb_ide_outbsync (ide_drive_t *drive, u8 value, unsigned long port)
{
	writew(value << 8, (void __iomem *) port);
}

static void openrb_ide_outw (u16 value, unsigned long port)
{
	writew(value, (void __iomem *) port);
}

static void openrb_ide_outsw (unsigned long port, void *addr, u32 count)
{
	u16 *ptr;

	for (ptr = addr; count--; ptr++)
		writew(*ptr, (void __iomem *) port);
}

static void openrb_ide_outl (u32 value, unsigned long port)
{
	writel(value, (void __iomem *) port);
}

static void openrb_ide_outsl (unsigned long port, void *addr, u32 count)
{
	u32 *ptr;

	for (ptr = addr; count--; ptr++)
		writel(*ptr, (void __iomem *) port);
}


static void __init ide_openrb_register(unsigned long addr,
					 unsigned long caddr, int irq)
{
	hw_regs_t hw;
	ide_hwif_t *hwif;

  	memset(&hw, 0, sizeof(hw));
	ide_setup_ports(&hw, addr, ide_offsets, caddr + 12, 0, NULL,irq);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	if (ide_register_hw(&hw, &hwif) != -1)
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	if (ide_register_hw(&hw, 1, &hwif) != -1)
#else
	if (ide_register_hw(&hw, NULL, 1, &hwif) != -1)
#endif
	{
		printk(KERN_NOTICE "OpenRB-IDE: Registered IDE-CF driver\n");
		hwif->mmio = 2;
		hwif->drives[0].unmask = 1;
		hwif->OUTB = openrb_ide_outb;
		hwif->OUTBSYNC = openrb_ide_outbsync;
		hwif->OUTW = openrb_ide_outw;
		hwif->OUTSW = openrb_ide_outsw;
		hwif->OUTSL = openrb_ide_outsl;
		hwif->INB = openrb_ide_inb;
		hwif->INW = openrb_ide_inw;
		hwif->INSW = openrb_ide_insw;
		hwif->INSL = openrb_ide_insl;
	}
}

void __init ide_openrb_init(void)
{
	volatile u16 *addr;
	volatile u16 *caddr;

	/* Turn on PerWE instead of PCIsomething */
	mtdcr(DCRN_CPC0_PCI_BASE, mfdcr(DCRN_CPC0_PCI_BASE) | (0x80000000L >> 27));

	/* PerCS1 (CF's CS0): base 0xff100000, 16-bit, rw */
	mtdcr(DCRN_EBC_BASE, 1);
	mtdcr(DCRN_EBC_BASE + 1, 0xff11a000);
	mtdcr(DCRN_EBC_BASE, 0x11);
	mtdcr(DCRN_EBC_BASE + 1, 0x080bd800);

	/* PerCS2 (CF's CS1): base 0xff200000, 16-bit, rw */
	mtdcr(DCRN_EBC_BASE, 2);
	mtdcr(DCRN_EBC_BASE + 1, 0xff21a000);
	mtdcr(DCRN_EBC_BASE, 0x12);
	mtdcr(DCRN_EBC_BASE + 1, 0x080bd800);

	/* Remap physical address space */
	addr = ioremap_nocache(0xff100000, 4096);
	caddr = ioremap_nocache(0xff200000, 4096);

	/* Set interrupt to low-to-high-edge-triggered */
	mtdcr(UIC0_TR, mfdcr(UIC0_TR) & ~(0x80000000L >> IRQ));
	mtdcr(UIC0_PR, mfdcr(UIC0_PR) | (0x80000000L >> IRQ));

	ide_openrb_register((unsigned long)addr, (unsigned long)caddr, IRQ);
}

