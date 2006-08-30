#include <linux/byteorder/swab.h>
#include <asm/interrupt.h>
#include <asm/io.h>

#include <asm/rc32434/rc32434.h>
#include <asm/rc32434/pci.h>

#ifdef notyet
extern int idt_pci;
#else
#define idt_pci 0
#endif

static inline unsigned pci_read_word(unsigned long addr)
{
	unsigned long flags;
	unsigned i;

	if (!idt_pci)
		return readl((u32 *) addr);
	
	local_irq_save(flags);
	rc32434_pci->pcidac = 1;
	for (i = 0; i < 10; ++i) {
		unsigned status;
		unsigned val;

		rc32434_pci->pcidas = 0;
		val = *(volatile unsigned *) addr;

		while (1) {
			status = rc32434_pci->pcidas;
			if (status & (PCIDAS_d_m | PCIDAS_e_m)) break;
		}
		if (!(status & PCIDAS_e_m)) {
#ifdef __MIPSEB__
			val = __swab32(rc32434_pci->pcidad);
#else
			val = rc32434_pci->pcidad;
#endif
			rc32434_pci->pcidac = 0;
			local_irq_restore(flags);
			return val;
		}

	}
	rc32434_pci->pcidac = 0;
	local_irq_restore(flags);

	printk("via pci decoupled read failed, pcis=0x%x\n", rc32434_pci->pcis);

	return 0;
}


static inline void pci_writel(unsigned int b, unsigned long addr)
{   
	unsigned long flags;
	
	if (!idt_pci) {
		writel(b, (u32 *) addr);
	} else {
		local_irq_save(flags);
		while (!(rc32434_pci->pcidas & PCIDAS_ofe_m)) {}
		*(volatile unsigned int *) addr = b;
		local_irq_restore(flags);
	}
}

static inline void pci_writew(unsigned short b, unsigned long addr)
{   
	unsigned long flags;
	
	if (!idt_pci) {
		writew(b, (u32 *) addr);
	} else {
		local_irq_save(flags);
		while (!(rc32434_pci->pcidas & PCIDAS_ofe_m)) {}
		*(volatile unsigned short *) addr = b;
		local_irq_restore(flags);
	}
}

static inline void pci_writeb(unsigned char b, unsigned long addr)
{   
	unsigned long flags;
	
	if (!idt_pci) {
		writeb(b, (u32 *) addr);
	} else {
		local_irq_save(flags);
		while (!(rc32434_pci->pcidas & PCIDAS_ofe_m)) {}
		*(volatile unsigned char *) addr = b;
		local_irq_restore(flags);
	}
}

static inline unsigned pci_readl(unsigned long addr)
{
	return pci_read_word(addr);
}   

static inline unsigned short pci_readw(unsigned long addr)
{
	return pci_read_word(addr & ~3) >> ((addr & 2) * 8);
}   

static inline unsigned char pci_readb(unsigned long addr)
{
	return pci_read_word(addr & ~3) >> ((addr & 3) * 8);
}

