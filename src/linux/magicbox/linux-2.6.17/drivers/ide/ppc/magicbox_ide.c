/* Driver for MagicBox 2.0 onboard CompactFlash adapter.
 *
 * Driver written with an invaluable help of Wojtek Kaniewski.
 * Modifications by Karol Lewandowski.
 *
 * GNU General Public License.
 */

#include <linux/types.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/ide.h>
#include <linux/delay.h>
#include <linux/byteorder/swab.h>

#define UIC0_PR 0xc4
#define UIC0_TR 0xc5
#define IRQ 25

static int ide_offsets[IDE_NR_PORTS] = {0, 3, 5, 7, 9, 11, 13, 15, -1, -1};

static void magicbox_outsw(void __iomem *port, void *addr, u32 count)
{
//	printk("magicbox_outsw: mark\n");
	while (count--) {
//		printk("magicbox_outsw: %c,%c\n", *(u8 *)addr, *(u8 *)(addr + 1));
		u16 tmp = swab16(*(u16 *)addr);
		writew(tmp, port);
		addr += 2;
	}
}

static void magicbox_insw(void __iomem *port, void *addr, u32 count)
{
	while (count--) {
		*(u16 *)addr = readw(port+1);
		addr += 2;
	}
}
 
static void __init ide_magicbox_register(unsigned long addr,
					 unsigned long caddr, int irq)
{
	hw_regs_t hw;
	ide_hwif_t *hwif;

  	memset(&hw, 0, sizeof(hw));
	ide_setup_ports(&hw, addr, ide_offsets, caddr + 0xd, 0, NULL,irq);

	if (ide_register_hw(&hw, &hwif) != -1)
	{
		printk(KERN_NOTICE "magicbox-cf: Found IDE-CF adapter\n");
		hwif->mmio = 2;
		hwif->drives[0].unmask = 1;
		default_hwif_mmiops(hwif);

//		hwif->OUTSW = magicbox_outsw;
//		hwif->INSW = magicbox_insw;
//		hwif->INW = magicbox_inw;
	}
}

void __init ide_magicbox_init(void)
{
	volatile u16 *addr;
	volatile u16 *caddr;

	/* Turn on PerWE instead of PCIsomething */
	mtdcr(DCRN_CPC0_PCI_BASE, mfdcr(DCRN_CPC0_PCI_BASE) | (0x80000000L >> 27));
	
	/* PerCS2 (CF's CS0): base 0xff100000, 16-bit, rw */
	mtdcr(DCRN_EBC_BASE, 0x02);
	mtdcr(DCRN_EBC_BASE + 1, 0xff11a000);
	mtdcr(DCRN_EBC_BASE, 0x12);
	//	mtdcr(DCRN_EBC_BASE + 1, 0x080bd800);
	mtdcr(DCRN_EBC_BASE + 1, 0x080bd400);

	/* PerCS1 (CF's CS1): base 0xff200000, 16-bit, rw */
	mtdcr(DCRN_EBC_BASE, 0x01);
	mtdcr(DCRN_EBC_BASE + 1, 0xff31a000);
	mtdcr(DCRN_EBC_BASE, 0x11);
	//	mtdcr(DCRN_EBC_BASE + 1, 0x080bd800);
	mtdcr(DCRN_EBC_BASE + 1, 0x080bd400);

	/* Remap physical address space */
	addr = ioremap_nocache(0xff100000, 4096);
	caddr = ioremap_nocache(0xff300000, 4096);

	/* Set interrupt to low-to-high-edge-triggered */
	mtdcr(UIC0_TR, mfdcr(UIC0_TR) & ~(0x80000000L >> IRQ));
	mtdcr(UIC0_PR, mfdcr(UIC0_PR) | (0x80000000L >> IRQ));

	ide_magicbox_register((unsigned long)addr, (unsigned long)caddr, IRQ);
}
