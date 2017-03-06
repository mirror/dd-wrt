#include <linux/init.h>
#include <linux/pci.h>
#include "ar7100.h"

/*
 * PCI IRQ map
 */
int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
    pr_debug("fixing irq for slot %d pin %d\n", slot, pin);

#ifdef CONFIG_AR7100_EMULATION
    printk("Returning IRQ %d\n", AR7100_PCI_IRQ_DEV0);
    return AR7100_PCI_IRQ_DEV0;
#else
    switch(slot)
    {
        case 0:
            return AR7100_PCI_IRQ_DEV0;
        case 1:
            return AR7100_PCI_IRQ_DEV1;
        case 2:
            return AR7100_PCI_IRQ_DEV2;
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

