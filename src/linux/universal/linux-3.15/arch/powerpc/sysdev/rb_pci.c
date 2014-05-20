#include <linux/module.h>
#include <linux/pci.h>
#include <sysdev/fsl_pci.h>

#ifdef CONFIG_PCI
static int rb_exclude_device(struct pci_controller *hose,
				 u_char bus, u_char devfn)
{
	return (bus == 0 && PCI_SLOT(devfn) == 0)
	    ? PCIBIOS_SUCCESSFUL // PCIBIOS_DEVICE_NOT_FOUND
	    : PCIBIOS_SUCCESSFUL;
}
#endif /* CONFIG_PCI */

void __init rb_init_pci(void)
{
	struct device_node *np;

	for_each_compatible_node(np, "pci", "fsl,mpc8540-pci")
	    fsl_add_bridge(np, 1);

	for_each_compatible_node(np, "pci", "fsl,mpc8540-pcie")
	    fsl_add_bridge(np, 0);

	ppc_md.pci_exclude_device = rb_exclude_device;
}

static void __init rb_secondary_bridge_fixup(struct pci_dev *dev) {
	/* enable i/o space & memory space and bus master control */
	pci_write_config_word(dev, PCI_COMMAND, 7);

	/* disable prefetched memory range */
	pci_write_config_word(dev, PCI_PREF_MEMORY_LIMIT, 0);
	pci_write_config_word(dev, PCI_PREF_MEMORY_BASE, 0x10);

	pci_write_config_word(dev, PCI_BASE_ADDRESS_0, 0);
	pci_write_config_word(dev, PCI_BASE_ADDRESS_1, 0);

	pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 8);

	pci_write_config_byte(dev, 0xc0, 1);
}

DECLARE_PCI_FIXUP_HEADER(0x3388, 0x0021, rb_secondary_bridge_fixup);

static void fixup_pci(struct pci_dev *dev)
{
	if ((dev->class >> 8) == PCI_CLASS_BRIDGE_PCI) {
		/* let the kernel itself set right memory windows */
		pci_write_config_word(dev, PCI_MEMORY_BASE, 0);
		pci_write_config_word(dev, PCI_MEMORY_LIMIT, 0);
		pci_write_config_word(dev, PCI_PREF_MEMORY_BASE, 0);
		pci_write_config_word(dev, PCI_PREF_MEMORY_LIMIT, 0);
		pci_write_config_byte(dev, PCI_IO_BASE, 0);
		pci_write_config_byte(dev, PCI_IO_LIMIT, 4 << 4);

		pci_write_config_byte(
		    dev, PCI_COMMAND,
		    PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 8);
	} else if (dev->vendor == 0x1957 &&
		   (dev->device == 0x32 || dev->device == 0x33)) {
		unsigned short val;
		pci_read_config_word(dev, 0x44, &val);
		pci_write_config_word(dev, 0x44, val | (1 << 10));
		pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0x00);
	} else {
		pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0x40);
	}
}

static void fixup_rb604(struct pci_dev *dev)
{
	pci_write_config_byte(dev, 0xC0, 0x01);
}

DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID, PCI_ANY_ID, fixup_pci)
DECLARE_PCI_FIXUP_HEADER(0x3388, 0x0021, fixup_rb604)
