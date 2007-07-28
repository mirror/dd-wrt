#ifndef AMD74XX_H
#define AMD74XX_H

#include <linux/config.h>
#include <linux/pci.h>
#include <linux/ide.h>

#define DISPLAY_AMD_TIMINGS

#if defined(DISPLAY_AMD_TIMINGS) && defined(CONFIG_PROC_FS)
#include <linux/stat.h>
#include <linux/proc_fs.h>

static u8 amd74xx_proc;

static int amd74xx_get_info(char *, char **, off_t, int);

static ide_pci_host_proc_t amd74xx_procs[] __initdata = {
	{
		.name		= "amd74xx",
		.set		= 1,
		.get_info	= amd74xx_get_info,
		.parent		= NULL,
	},
};
#endif  /* defined(DISPLAY_AMD_TIMINGS) && defined(CONFIG_PROC_FS) */

static unsigned int init_chipset_amd74xx(struct pci_dev *, const char *);
static void init_hwif_amd74xx(ide_hwif_t *);

static ide_pci_device_t amd74xx_chipsets[] __devinitdata = {
	{	/* 0 */
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_COBRA_7401,
		.name		= "AMD7401",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x40,0x02,0x02}, {0x40,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0
	},{	/* 1 */
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_VIPER_7409,
		.name		= "AMD7409",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x40,0x02,0x02}, {0x40,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0
	},{	/* 2 */
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_VIPER_7411,
		.name		= "AMD7411",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x40,0x02,0x02}, {0x40,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0
	},{	/* 3 */
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_OPUS_7441,
		.name		= "AMD7441",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x40,0x02,0x02}, {0x40,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0
	},{	/* 4 */
		.vendor		= PCI_VENDOR_ID_AMD,
		.device		= PCI_DEVICE_ID_AMD_8111_IDE,
		.name		= "AMD8111",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.autodma	= AUTODMA,
		.channels	= 2,
		.enablebits	= {{0x40,0x02,0x02}, {0x40,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0
	},
	{	/* 5 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE_IDE,
		.name		= "NFORCE",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0,
	},
	{	/* 6 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE2_IDE,
		.name		= "NFORCE2",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
		.extra		= 0,
	},
	{	/* 7 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE2S_IDE,
		.name		= "NFORCE2-U400R",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 8 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE2S_SATA,
		.name		= "NFORCE2-U400R-SATA",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 9 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE3_IDE,
		.name		= "NFORCE3-150",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 10 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE3S_IDE,
		.name		= "NFORCE3-250",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 11 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE3S_SATA,
		.name		= "NFORCE3-250-SATA",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 12 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE3S_SATA2,
		.name		= "NFORCE3-250-SATA2",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 13 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE_CK804_IDE,
		.name		= "NFORCE-CK804",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 14 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE_MCP04_IDE,
		.name		= "NFORCE-MCP04",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{	/* 15 */
		.vendor		= PCI_VENDOR_ID_NVIDIA,
		.device		= PCI_DEVICE_ID_NVIDIA_NFORCE_MCP55_IDE,
		.name		= "NFORCE-MCP55",
		.init_chipset	= init_chipset_amd74xx,
		.init_hwif	= init_hwif_amd74xx,
		.channels	= 2,
		.autodma	= AUTODMA,
		.enablebits	= {{0x50,0x02,0x02}, {0x50,0x01,0x01}},
		.bootable	= ON_BOARD,
	},
	{
		.vendor		= 0,
		.device		= 0,
		.channels	= 0,
		.bootable	= EOL,
	}
};

#endif /* AMD74XX_H */
