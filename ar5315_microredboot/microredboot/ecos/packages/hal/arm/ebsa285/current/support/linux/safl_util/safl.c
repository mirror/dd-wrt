// #========================================================================
// #
// #    safl.c
// #
// #    Linux driver for Intel(R) StrongARM(R) PCI-based coprocessor boards.
// #
// #========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
// #========================================================================
// ######DESCRIPTIONBEGIN####
// #
// # Author(s):     msalter
// # Contributors:  msalter
// # Date:          1999-04-02
// # Purpose:       Linux driver for Intel(R) StrongARM(R) PCI-based
// #                coprocessor boards
// # Description:   This module currently supports EBSA-285 and SA-IOP.
// #                Intel is a Registered Trademark of Intel Corporation.
// #                StrongARM is a Registered Trademark of Advanced RISC
// #                Machines Limited.
// #                Other Brands and Trademarks are the property of their
// #                respective owners.
// #
// #####DESCRIPTIONEND####
// #
// #========================================================================

static char *version =
"safl.c:v0.01H 04/02/99 Mark Salter, Red Hat.\n";

#include <linux/module.h>
#include <linux/config.h>
#include <linux/version.h>
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/malloc.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>
#ifdef CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif

#include <asm/io.h>
#include <asm/system.h>

#if LINUX_VERSION_CODE >= 0x020100
#include <linux/init.h>
#include <linux/vmalloc.h>
#else
#include <linux/bios32.h>
#define __initfunc(x) x
#endif

#if LINUX_VERSION_CODE < 0x20155
#define PCI_SUPPORT_VER1
#else
#define PCI_SUPPORT_VER2
#endif

#if defined(MODULE) && LINUX_VERSION_CODE > 0x20115
MODULE_AUTHOR("Mark Salter <msalter@redhat.com>");
MODULE_DESCRIPTION("Intel(R) StrongARM(R) FLASH driver for PCI EVBs");
#endif

/* This isn't in /usr/include/linux/pci.h */
#ifndef PCI_DEVICE_ID_DEC_IOP
#define PCI_DEVICE_ID_DEC_21554     0x46
#endif

#ifndef PCI_DEVICE_ID_DEC_21285
#define PCI_DEVICE_ID_DEC_21285	    0x1065
#endif


/* somewhat arbitrary minor number. Major number is 10 (misc). */
#define SAFL_MINOR 178
#define FLASH_SZ   (4 * 1024 * 1024)

static int safl_open( struct inode *inode, struct file *file );
static int safl_close( struct inode *inode, struct file *file );

#if LINUX_VERSION_CODE >= 0x020100
static int safl_mmap(struct file * file, struct vm_area_struct * vma);
#else
static int safl_mmap(struct inode * inode, struct file * file, struct vm_area_struct * vma);
#endif


static int safl_debug = 0;

static unsigned long csr_ioaddr;
static unsigned long flash_addr;

static struct file_operations safl_fops = {
    NULL,                       /* lseek */
    NULL,                       /* read  */
    NULL,                       /* write */
    NULL,			/* readdir */
    NULL,			/* poll */
    NULL,                       /* ioctl */
    safl_mmap,			/* mmap */
    safl_open,                  /* open */
    NULL,			/* flush */
    safl_close                  /* close */
};

static struct miscdevice safl_dev = {
	SAFL_MINOR,
	"SA-FLASH",
	&safl_fops
};

/* PCI configuration space information. */
static u8 safl_pci_bus, safl_pci_devfn;
static int safl_devid;

#ifdef PCI_SUPPORT_VER2
static struct pci_dev *safl_pdev;
#endif

static int safl_open_cnt = 0;	/* #times opened */

__initfunc(int safl_scan(void))
{
    if (pcibios_present()) {
	int index;

	/*
	 * Search for an EBSA-285 board or an IOP board. Stop at
	 * first one found.
	 */
	for (index = 0; index < 8; index++) {
	    if (pcibios_find_device (PCI_VENDOR_ID_DEC, PCI_DEVICE_ID_DEC_21554,
				     index, &safl_pci_bus, &safl_pci_devfn)
		== PCIBIOS_SUCCESSFUL) {
		safl_devid = PCI_DEVICE_ID_DEC_21554;
		break;
	    }
	    if (pcibios_find_device (PCI_VENDOR_ID_DEC, PCI_DEVICE_ID_DEC_21285,
				     index, &safl_pci_bus, &safl_pci_devfn)
		== PCIBIOS_SUCCESSFUL) {
		safl_devid = PCI_DEVICE_ID_DEC_21285;
		break;
	    }
	}

	if (index < 8) {
#ifdef PCI_SUPPORT_VER2
	    safl_pdev = pci_find_slot(safl_pci_bus, safl_pci_devfn);
#endif
	    misc_register(&safl_dev);
	    return 0;
	}
    }
    if (safl_debug)
	printk(KERN_INFO "Can't find device.\n");
    return -ENODEV;
}


int
init_module(void)
{
    if (safl_debug)
	printk(KERN_INFO "%s", version);
    return safl_scan();
}


/*
 * Dword read from configuration space of secondary PCI bus.
 */
static unsigned int
sconfig_read(int addr)
{
    unsigned int val;

    outw(2, csr_ioaddr + 0x12);  /* enable downstream config */
    outl(addr | (1<<24), csr_ioaddr + 0x00);
    val = inl(csr_ioaddr + 4);
    outw(0, csr_ioaddr + 0x12);  /* disable downstream config */

    return val;
}


/*
 * Dword write to configuration space of secondary PCI bus.
 */
static void
sconfig_write(int addr, int val)
{
    outw(2, csr_ioaddr + 0x12);  /* enable downstream config */
    outl(addr | (1<<24), csr_ioaddr + 0x00);
    outl(val, csr_ioaddr + 4);
    outw(0, csr_ioaddr + 0x12);  /* disable downstream config */
}


/*
 * Dword read from configuration space of primary PCI bus.
 */
static unsigned int
pconfig_read(int addr)
{
    unsigned int val;

#ifdef PCI_SUPPORT_VER2
    pci_read_config_dword(safl_pdev, addr, &val);
#else
    pcibios_read_config_dword(safl_pci_bus, safl_pci_devfn, addr, &val);
#endif

    return val;
}


/*
 * Dword write to configuration space of primary PCI bus.
 */
static void
pconfig_write(int addr, int val)
{
#ifdef PCI_SUPPORT_VER2
    pci_write_config_dword(safl_pdev, addr, val);
#else
    pcibios_write_config_dword(safl_pci_bus, safl_pci_devfn, addr, val);
#endif
}



static int
safl_open( struct inode *inode, struct file *file )
{
    if (safl_open_cnt) {
	if (safl_debug)
	    printk(KERN_INFO "SA-Flash already open.\n");
	return( -EBUSY );
    }

    if (safl_devid == PCI_DEVICE_ID_DEC_21554) {

	csr_ioaddr = pconfig_read(PCI_BASE_ADDRESS_1) & PCI_BASE_ADDRESS_IO_MASK;
	flash_addr = pconfig_read(PCI_BASE_ADDRESS_3) & PCI_BASE_ADDRESS_MEM_MASK;

	if (safl_debug) {
	    printk(KERN_INFO "IOP: csr_io[%lx].\n", csr_ioaddr);
	    printk(KERN_INFO "IOP: flash_add[%lx].\n", flash_addr);
	}

	/*
	 * Need to configure downstream side of 21554.
	 * These addresses are pretty arbitrary.
	 */
	pconfig_write(0x50, 0xf0000000);  /* secondary CSR memory */
	pconfig_write(0x54, 0xf201);	  /* secondary CSR I/O    */
	pconfig_write(0x58, 0xf401);	  /* Upstream I/O         */
	pconfig_write(0x5c, 0xf1000008);  /* Upstream memory 1    */
	pconfig_write(0x44, 0x29000017);  /* enable mem an I/O    */

	/* set downstream mem2 xlate base */
	pconfig_write(0x9c, 0xA0000000);

	/* set 21285 ROM address to same */
	sconfig_write(0x30, 0xA0000001);

	/* set 21285 ROM write byte address */
	sconfig_write(0x68, 0);

    } else if (safl_devid == PCI_DEVICE_ID_DEC_21285) {
	/*
	 * I'm not sure how best to handle this. Basically, you want
	 * to assign a physical address for the expansion ROM space
	 * of the 21285. There doesn't appear to be a good way to
	 * find an unused physical space for the PCI bus. This seems
	 * to work for the limited number of motherboards that this
	 * code has been tested on. YMMV.
	 */
	flash_addr = 0xb0000000;
	pconfig_write(PCI_ROM_ADDRESS, flash_addr | PCI_ROM_ADDRESS_ENABLE);
    }

    safl_open_cnt++;

    MOD_INC_USE_COUNT;
    return 0;
}


static int
safl_close( struct inode *inode, struct file *file )
{
    safl_open_cnt--;

    if (safl_devid == PCI_DEVICE_ID_DEC_21285)
	pconfig_write(PCI_ROM_ADDRESS, 0);

    if (safl_devid == PCI_DEVICE_ID_DEC_21554)
	sconfig_write(0x30, 0);

    MOD_DEC_USE_COUNT;
    return 0;
}


static inline unsigned long
pgprot_noncached(unsigned long prot)
{
#if LINUX_VERSION_CODE >= 0x020100
    if (boot_cpu_data.x86 > 3)
	prot |= _PAGE_PCD;
#endif
    return prot;
}


static int
#if LINUX_VERSION_CODE >= 0x020100
safl_mmap(struct file * file, struct vm_area_struct * vma)
#else
safl_mmap(struct inode * inode, struct file * file, struct vm_area_struct * vma)
#endif
{
    unsigned long size;

    if (vma->vm_offset != 0)
	return -EINVAL;

    size = vma->vm_end - vma->vm_start;
    if (size > FLASH_SZ)
	return -EINVAL;

    pgprot_val(vma->vm_page_prot) = pgprot_noncached(pgprot_val(vma->vm_page_prot));

#if LINUX_VERSION_CODE >= 0x020100
    vma->vm_flags |= VM_IO;
#endif

    if (remap_page_range(vma->vm_start, flash_addr, size, vma->vm_page_prot))
	return -EAGAIN;

#if LINUX_VERSION_CODE < 0x020100
    vma->vm_inode = inode;
    inode->i_count++;
#endif

    return 0;
}

void
cleanup_module(void)
{
    misc_deregister(&safl_dev);
}

/*
 * Local variables:
 *  compile-command: "cc -DMODULE -D__KERNEL__ -Wall -Wstrict-prototypes -O6 -c safl.c `[ -f /usr/include/linux/modversions.h ] && echo -DMODVERSIONS`"
 * End:
 */
