#include <linux/init.h>
#include <linux/pci.h>
#include "ar7240.h"

/*
 * PCI IRQ map
 */
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
#if 1
    printk("Returning IRQ %d\n", AR7240_PCI_IRQ_DEV0);
    return AR7240_PCI_IRQ_DEV0;
#else
    switch(slot)
    {
        case 0:
            return AR7240_PCI_IRQ_DEV0;
        case 1:
            return AR7240_PCI_IRQ_DEV1;
        case 2:
            return AR7240_PCI_IRQ_DEV2;
        default:
            printk("unknown slot!\n");
            return -1;
    }
#endif
}

int 
pcibios_plat_dev_init(struct pci_dev *dev)
{
        return 0;
}

