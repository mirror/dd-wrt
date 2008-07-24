
#ifndef __SL2312_PCI_H__
#define __SL2312_PCI_H__

#define SL2312_PCI_PMC				0x40
#define SL2312_PCI_PMCSR			0x44
#define SL2312_PCI_CTRL1			0x48
#define SL2312_PCI_CTRL2			0x4c
#define SL2312_PCI_MEM1_BASE_SIZE	0x50
#define SL2312_PCI_MEM2_BASE_SIZE	0x54
#define SL2312_PCI_MEM3_BASE_SIZE	0x58


void sl2312_pci_mask_irq(unsigned int irq);
void sl2312_pci_unmask_irq(unsigned int irq);
int sl2312_pci_get_int_src(void);

#endif
