/*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <mach/pcie.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <mach/pm.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#endif


#define CONFIG_CMD(bus, device_fn, where) (0x80000000 | ((bus) << 16) | ((device_fn) << 8) | ((where) & ~3))
#define PCIE_ID(bus, device_fn, where) (((bus&0xf)<<20)| (device_fn<<12) | (where))

#define CNS3XXX_PCIE_DEBUG
// #undef CNS3XXX_PCIE_DEBUG
#define CNS3XXX_PCIE_RESET


static struct pci_dev *pcie0_bridge = NULL;
static struct pci_dev *pcie1_bridge = NULL;
static volatile u32 port0_cfg;
static int pcie_linked[2];

u32 cns3xxx_pcie0_irqs[2] = { IRQ_CNS3XXX_PCIE0_RC, IRQ_CNS3XXX_PCIE0_DEVICE,  };
u32 cns3xxx_pcie1_irqs[2] = { IRQ_CNS3XXX_PCIE1_RC, IRQ_CNS3XXX_PCIE1_DEVICE,  };
extern struct proc_dir_entry *cns3xxx_proc_dir;

#ifdef CNS3XXX_PCIE_DEBUG
struct proc_dir_entry *pcie_proc_entry;
extern struct pci_dev *pdev_tmp;
extern void e1000_shutdown(struct pci_dev *);
extern int e1000_resume(struct pci_dev *);

DEFINE_SPINLOCK(pci_config_lock);

/*
Read String into Buffer, Max String buffer is 100
*/
static ssize_t readstring(char *buff, const char *buf, size_t count)
{
	int i = 0;
	if (count) {
		char c;

		for (i = 0; i < count && i < 100; i++) {
			if (get_user(c, buf + i))
				return -EFAULT;
			buff[i] = c;
		}
		buff[i] = 0;
	}
	return count;
}

static int cns3xxx_pcie_read_proc(char *page, char **start, off_t off,
				  int count, int *eof, void *data)
{
	int num = 0;
//      int i;

//      num += sprintf(page+num, "=== Config Type 0 ===");
//      for (i=0; i<0x3f; i+=4) {
//              if (i%16 == 0) num += sprintf(page+num, "\n%x: ", i);
//              num += sprintf(page+num, "%4x ", __raw_readl(CNS3XXX_PCIE0_HOST_BASE_VIRT + i));
//              if (i%4 == 0) num += sprintf(page+num, " ");
//      }

	return num;

}

//#define PCIE_E1000_DEVICE_ID	0x107D		/* Intel(R) PRO/1000 PT server adapter */
#define PCIE_E1000_DEVICE_ID	0x10B9          
//#define PCIE_E1000_DEVICE_ID	0x10D3		/* Intel(R) Gigabit CT desktop adapter */
static int cns3xxx_pcie_write_proc(struct file *file, const char *buffer,
				   unsigned long count, void *data)
{
	char read_buff[100], buf_cmd[100], buf_param1[100];
	u16 pos, aspm_sp, aspm_ctl, link_ctl;
	u16 reg16;
	u32 reg32, i = 0, lc = 1000;
	struct pci_dev *prc = NULL, *pdev = NULL;
	readstring((char *)read_buff, (const char *)buffer, count);
	sscanf(read_buff, "%s %s\n", (char *)&buf_cmd, (char *)&buf_param1);

	prc = pci_get_device(PCIB_VENDOR_ID, PCIB_DEVICE_ID, NULL);
	pdev = pci_get_device(PCI_VENDOR_ID_INTEL, PCIE_E1000_DEVICE_ID, NULL);
	printk("prc:%x pdev:%x \n", (u32) prc, (u32) pdev);

	if (pdev == NULL) {
		printk("PCIe Device is not e1000 NIC\n");
		return -EINVAL;
	}

	if (strcmp(buf_cmd, "pm") == 0) {
		if (strcmp(buf_param1, "suspend") == 0) {
			pdev->driver->shutdown(pdev);
		} else if (strcmp(buf_param1, "resume") == 0) {
			//printk("resume:%x \n", (u32)pdev->driver->resume);
			pdev->driver->resume(pdev);
		} else
			printk("syntax: pm suspend/resume\n");
		return count;
	}

	if (strcmp(buf_cmd, "aspm") == 0) {
		if (strcmp(buf_param1, "L0") == 0) {
			/* RC */
			pos = pci_find_capability(prc, PCI_CAP_ID_EXP);
			pci_read_config_word(prc, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			aspm_ctl &= ~0x3;
			pci_write_config_word(prc, pos + PCI_EXP_LNKCTL,
					      aspm_ctl);
			/* Device */
			pos = pci_find_capability(pdev, PCI_CAP_ID_EXP);
			pci_read_config_word(pdev, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			aspm_ctl &= ~0x3;
			pci_write_config_word(pdev, pos + PCI_EXP_LNKCTL,
					      aspm_ctl);
			/* Verify Reg only */
			pci_read_config_word(prc, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			printk("RC ASPM Control: 0x%x\n", (aspm_ctl & 0x3));
			pci_read_config_word(pdev, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			printk("Device ASPM Control: 0x%x\n", (aspm_ctl & 0x3));
		} else if (strcmp(buf_param1, "L0sL1") == 0) {
			/* Device */
			pos = pci_find_capability(pdev, PCI_CAP_ID_EXP);
			pci_read_config_word(pdev, pos + PCI_EXP_LNKCAP,
					     &aspm_sp);
			printk("aspm_sp of device: 0x%x\n", aspm_sp);
			aspm_sp = (aspm_sp >> 10) & 0x3;
			pci_read_config_word(pdev, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			aspm_ctl &= ~0x3;
			aspm_ctl |= aspm_sp;
			pci_write_config_word(pdev, pos + PCI_EXP_LNKCTL,
					      aspm_ctl);
			/* RC */
			pos = pci_find_capability(prc, PCI_CAP_ID_EXP);
			pci_read_config_word(prc, pos + PCI_EXP_LNKCAP,
					     &aspm_sp);
			aspm_sp = (aspm_sp >> 10) & 0x3;
			pci_read_config_word(prc, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			aspm_ctl &= ~0x3;
			aspm_ctl |= aspm_sp;
			pci_write_config_word(prc, pos + PCI_EXP_LNKCTL,
					      aspm_ctl);
			/* Verify Reg only */
			pci_read_config_word(prc, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			printk("RC ASPM Control: 0x%x\n", (aspm_ctl & 0x3));
			pci_read_config_word(pdev, pos + PCI_EXP_LNKCTL,
					     &aspm_ctl);
			printk("Device ASPM Control: 0x%x\n", (aspm_ctl & 0x3));
		} else
			printk("syntax: aspm L0/L0sL1\n");
		return count;
	}
#define  PCI_EXP_LNKCTL_LBMIE   0x0400	/* Link Bandwidth Management Interrupt Enable */
#define  PCI_EXP_LNKCTL_LABIE   0x0800	/* Lnk Autonomous Bandwidth Interrupt Enable */

	if (strcmp(buf_cmd, "relinkintc") == 0) {
		u32 *ptr;
		printk("Relink Interrupt Test \n");

		ptr = (u32 *) 0xFFF07978;
		*ptr = 0;

		pos = pci_find_capability(prc, PCI_CAP_ID_EXP);
		pci_read_config_word(prc, pos + PCI_EXP_LNKCTL, &reg16);
		reg16 |= PCI_EXP_LNKCTL_LBMIE;	/* Link Bandwidth Management Interrupt Enable */
		reg16 |= PCI_EXP_LNKCTL_LABIE;	/* Lnk Autonomous Bandwidth Interrupt Enable */
		reg16 |= PCI_EXP_LNKCTL_RL;	/* Retrain Link */
		printk("Addr:%X(%d) Value:%X \n", pos + PCI_EXP_LNKCTL,
		       pos + PCI_EXP_LNKCTL, reg16);
		pci_write_config_word(prc, pos + PCI_EXP_LNKCTL, reg16);
		pci_read_config_word(prc, pos + PCI_EXP_LNKCTL, &reg16);
		printk("Value:%X \n", reg16);

		ptr = (u32 *) 0xFFF07974;
		printk(" 0x974:%08X , LBM Intc: %s \n", *ptr,
		       ((*ptr >> 13) & 0x1) == 0 ? "Disable" : "Enable");
	}
	if (strcmp(buf_cmd, "relink") == 0) {
/* Link Down */
		printk("[%d] Link down \n", i++);
		reg32 = __raw_readl(CNS3XXX_MISC_BASE_VIRT + 0x95C);
		__raw_writel((reg32 & ~0x1), CNS3XXX_MISC_BASE_VIRT + 0x95C);
		mdelay(100);
		/* Link Up */
		printk("[%d] Link Up \n", i++);
		__raw_writel((reg32 | 0x1), CNS3XXX_MISC_BASE_VIRT + 0x95C);
		printk("Wait for PCIe0 link up...");
		do {
			reg32 = __raw_readl(CNS3XXX_MISC_BASE_VIRT + 0x960);
		} while (!(reg32 & 0x1));
		printk("OK.\n");

	}
	if (strcmp(buf_cmd, "hotreset") == 0) {
		int i = 0;
		int rc_buf[1024 / 4], dev_buf[1024 / 4];
#if 0
		if (pdev->driver) {
			pdev->driver->remove(pdev);
			printk("Device driver has been removed.\n");
		}
#endif
		printk("PCIe hot reset.\n");

		for (i = 0; i < 1024; i += 4) {
			pci_read_config_dword(prc, i,
					      &rc_buf[i == 0 ? 0 : i / 4]);
			pci_read_config_dword(pdev, i,
					      &dev_buf[i == 0 ? 0 : i / 4]);
			printk("[%03X(%04d)] RC:%08X		DEV:%08X\n", i,
			       i, rc_buf[i == 0 ? 0 : i / 4],
			       dev_buf[i == 0 ? 0 : i / 4]);
		}
#if 0

		printk("[%d] Hot Reset\n", i++);
		reg32 = __raw_readl(CNS3XXX_MISC_BASE_VIRT + 0x95C);
		reg32 |= (0x1 << 20);	/* Assert hot reset */
		__raw_writel(reg32, CNS3XXX_MISC_BASE_VIRT + 0x95C);
		mdelay(100);
		reg32 &= ~(0x1 << 20);	/* De-assert hot reset */
		__raw_writel(reg32, CNS3XXX_MISC_BASE_VIRT + 0x95C);
#endif

/* Follow spec to do hotreset  */
#if 1
		{
			u16 tmp;

			pci_read_config_word(prc, PCI_CB_BRIDGE_CONTROL, &tmp);
			printk("[%d]BRIDGE REG: %04X \n", i, tmp);
			tmp |= PCI_CB_BRIDGE_CTL_CB_RESET;
			pci_write_config_word(prc, PCI_CB_BRIDGE_CONTROL, tmp);
#if 1
			mdelay(3);
			tmp &= ~PCI_CB_BRIDGE_CTL_CB_RESET;
			pci_write_config_word(prc, PCI_CB_BRIDGE_CONTROL, tmp);
			pci_read_config_word(prc, PCI_CB_BRIDGE_CONTROL, &tmp);
			printk("[%d]BRIDGE REG: %04X \n", i, tmp);
#endif
		}

#endif
		for (i = 0; i < 1024; i += 4) {
			pci_write_config_dword(prc, i,
					       rc_buf[i == 0 ? 0 : i / 4]);
			pci_write_config_dword(pdev, i,
					       dev_buf[i == 0 ? 0 : i / 4]);
		}
		return count;
	}

	if (strcmp(buf_cmd, "retrain") == 0) {
		for (i = 0; i < lc; i++) {
			pos = pci_find_capability(prc, PCI_CAP_ID_EXP);
			pci_read_config_word(prc, pos + PCI_EXP_LNKCTL,
					     &link_ctl);
			link_ctl |= 0x1 << 5;
			pci_write_config_word(prc, pos + PCI_EXP_LNKCTL,
					      link_ctl);

//                      printk("lc: %d: Wait for link training done...",i+1);
			do {
				pci_read_config_word(prc, pos + PCI_EXP_LNKSTA,
						     &reg16);
			} while (reg16 & (0x1 << 11));
//                      printk("OK.\n");

			printk("lc: %d: Wait for DL active...", i + 1);
			do {
				pci_read_config_word(prc, pos + PCI_EXP_LNKSTA,
						     &reg16);
			} while (!(reg16 & (0x1 << 13)));
			printk("OK.\n");
		}
		return count;
	}

	return count;
}

static int __init cns3xxx_proc_init(void)
{
	if (cns3xxx_proc_dir == NULL) {
		printk("Please Create Proc First \n");
		BUG();
	}
	pcie_proc_entry =
	    create_proc_entry("pcie", S_IFREG | S_IRUGO, cns3xxx_proc_dir);

	if (pcie_proc_entry) {
		pcie_proc_entry->read_proc = cns3xxx_pcie_read_proc;
		pcie_proc_entry->write_proc = cns3xxx_pcie_write_proc;
	}

	return 1;
}
#endif

static u32 access_base[2][3] = {
	{
		CNS3XXX_PCIE0_HOST_BASE_VIRT,
		CNS3XXX_PCIE0_CFG0_BASE_VIRT,
		CNS3XXX_PCIE0_CFG1_BASE_VIRT,
	},
	{
		CNS3XXX_PCIE1_HOST_BASE_VIRT,
		CNS3XXX_PCIE1_CFG0_BASE_VIRT,
		CNS3XXX_PCIE1_CFG1_BASE_VIRT,
	},
};

static u32 cns3xxx_pci_cfg_base(struct pci_bus *bus, unsigned int devfn,
				int where)
{
	int domain = pci_domain_nr(bus);
	int busno = bus->number;
	int slot = PCI_SLOT(devfn);
	int type;
	u32 base;

	/* If there is no link, just show the CNS PCI bridge. */
	if (!pcie_linked[domain] && (busno > 0 || slot > 0))
		return 0;

	/*
	 * The CNS PCI bridge doesn't fit into the PCI hierarchy, though
	 * we still want to access it. For this to work, we must place
	 * the first device on the same bus as the CNS PCI bridge.
	 */
	if (busno == 0) {
		if (slot > 1)
			return 0;
		type = slot;
	} else {
		type = 2;
	}

	where &= 0xffc;
	base = access_base[domain][type];

	return base + PCIE_ID(busno, devfn, where);
}

static int _cns3xxx_pci_read_config(struct pci_bus *bus,
				   unsigned int devfn, int where, int size,
				   u32 * val)
{
	u32 v = 0;
	u32 base;
	u8 modify_class = 0;
	int domain;

	domain = pci_domain_nr(bus);

	switch (size) {
	case 1:
		_cns3xxx_pci_read_config(bus, devfn, where, 4, &v);

		switch (where % 4) {
		case 0:
			v &= 0x000000FF;
			break;
		case 1:
			v &= 0x0000FF00;
			v = v >> 8;
			break;
		case 2:
			v &= 0x00FF0000;
			v = v >> 16;
			break;
		case 3:
			v &= 0xFF000000;
			v = v >> 24;
			break;
		}
		break;

	case 2:

		_cns3xxx_pci_read_config(bus, devfn, where, 4, &v);

		switch (where % 4) {
		case 0:
		case 1:
			v &= 0x0000FFFF;
			break;
		case 2:
		case 3:
			v &= 0xFFFF0000;
			v = v >> 16;
			break;
		}
		break;

	case 4:
		/* Read Class Code */
		if (bus->number == 0 && devfn == 0 && where == 0x8) {
			modify_class = 1;
			break;
		}

		base = cns3xxx_pci_cfg_base(bus, devfn, where);
		if (!base) {
			v = 0xffffffff;
			break;
		}

		v = __raw_readl(base);
		break;
	}

	/* RC's class is 0xb, but Linux PCI driver needs 0x604 for a PCIe bridge. */
	/* So we must dedicate the class code to 0x604 here */
	if (modify_class) {
		v &= 0xff;
		v |= (0x604 << 16);
	}
//	printk("[RLDBG] domain:%d bus:%d devfn:%d where:%d(0x%03x) v:0x%08x\n", domain, bus->number, devfn, where, where , v);
	*val = v;

	return PCIBIOS_SUCCESSFUL;
}
static int cns3xxx_pci_read_config(struct pci_bus *bus,
				   unsigned int devfn, int where, int size,
				   u32 * val)
{
	unsigned long flags;
	int ret;
#if 0
	local_irq_save(flags);
#else
	spin_lock_irqsave(&pci_config_lock, flags);
#endif

	ret = _cns3xxx_pci_read_config(bus, devfn, where, size, val);

#if 0
	local_irq_restore(flags);
#else
	spin_unlock_irqrestore(&pci_config_lock, flags);
#endif

	return ret;
}

static int _cns3xxx_pci_write_config(struct pci_bus *bus,
				    unsigned int devfn, int where, int size,
				    u32 val)
{
	u32 v;
	u32 base;
	int domain;

	domain = pci_domain_nr(bus);

	switch (size) {
	case 1:

		val &= 0x000000FF;
		_cns3xxx_pci_read_config(bus, devfn, where, 4, &v);

		switch (where % 4) {
		case 0:
			v &= 0xFFFFFF00;
			v |= val;
			break;
		case 1:
			v &= 0xFFFF00FF;
			v |= (val << 8);
			break;
		case 2:
			v &= 0xFF00FFFF;
			v |= (val << 16);
			break;
		case 3:
			v &= 0x00FFFFFF;
			v |= (val << 24);
			break;
		}
		_cns3xxx_pci_write_config(bus, devfn, where, 4, v);

		break;

	case 2:

		val &= 0x0000FFFF;
		_cns3xxx_pci_read_config(bus, devfn, where, 4, &v);

		switch (where % 4) {
		case 0:
		case 1:
			v &= 0xFFFF0000;
			v |= val;
			break;
		case 2:
		case 3:
			v &= 0x0000FFFF;
			v |= (val << 16);
			break;
		}
		_cns3xxx_pci_write_config(bus, devfn, where, 4, v);
		break;

	case 4:
		base = cns3xxx_pci_cfg_base(bus, devfn, where);
		if (!base) {
			v = 0xffffffff;
			break;
		}

		__raw_writel(val, base);
		break;
	}


	return PCIBIOS_SUCCESSFUL;
}


static int cns3xxx_pci_write_config(struct pci_bus *bus,
				    unsigned int devfn, int where, int size,
				    u32 val)
{
	unsigned long flags;
	int ret;
#if 0
	local_irq_save(flags);
#else
	spin_lock_irqsave(&pci_config_lock, flags);
#endif

	ret = _cns3xxx_pci_write_config(bus, devfn, where, size, val);

#if 0
	local_irq_restore(flags);
#else
	spin_unlock_irqrestore(&pci_config_lock, flags);
#endif
	return ret;

}


static struct pci_ops cns3xxx_pcie_ops = {
	.read = cns3xxx_pci_read_config,
	.write = cns3xxx_pci_write_config,
};

static struct resource cns3xxx_pcie0_io = {
	.name = "PCIe0 I/O space",
	.start = PCIE0_IO_SPACE_START,
	.end = PCIE0_IO_SPACE_END,
	.flags = IORESOURCE_IO,
};

static struct resource cns3xxx_pcie1_io = {
	.name = "PCIe1 I/O space",
	.start = PCIE1_IO_SPACE_START,
	.end = PCIE1_IO_SPACE_END,
	.flags = IORESOURCE_IO,
};

static struct resource cns3xxx_pcie0_mem = {
	.name = "PCIe0 non-prefetchable",
	.start = PCIE0_MEM_SPACE_START,
	.end = PCIE0_MEM_SPACE_END,
	.flags = IORESOURCE_MEM,
};

static struct resource cns3xxx_pcie1_mem = {
	.name = "PCIe1 non-prefetchable",
	.start = PCIE1_MEM_SPACE_START,
	.end = PCIE1_MEM_SPACE_END,
	.flags = IORESOURCE_MEM,
};
#if 0
static struct resource cns3xxx_pci_prefetch_mem = {
	.name = "PCI prefetchable",
	.start = PCI_PREFETCH_MEMORY_SPACE_START,
	.end = PCI_PREFETCH_MEMORY_SPACE_END,
	.flags = IORESOURCE_MEM | IORESOURCE_PREFETCH,
};
#endif

static int __init cns3xxx_pci_setup_resources(int nr, struct resource **resource)
{
	int ret = 1;

	if(nr==0){
		ret = request_resource(&iomem_resource, &cns3xxx_pcie0_io);
      		if (ret) 
              		panic("PCIe0: unable to allocate I/O region\n");
       		ret = request_resource(&iomem_resource, &cns3xxx_pcie0_mem);
      		if (ret) 
              		panic("PCIe0: unable to allocate memory region\n");
		resource[0] = &cns3xxx_pcie0_io;
		resource[1] = &cns3xxx_pcie0_mem;
		ret = 0;
	}else{
		ret = request_resource(&iomem_resource, &cns3xxx_pcie1_io);
      		if (ret) 
              		panic("PCIe1: unable to allocate I/O region\n");
       		ret = request_resource(&iomem_resource, &cns3xxx_pcie1_mem);
      		if (ret) 
              		panic("PCIe1: unable to allocate memory region\n");
		resource[0] = &cns3xxx_pcie1_io;
		resource[1] = &cns3xxx_pcie1_mem;
		ret = 0;
	}

	return ret;
}

static irqreturn_t cns3xxx_pcie0_dev_intc(int irq, void *dev_id)
{

#if 0
	u32 status;
	/* Read PCIe0 interrupt */
	status = __raw_readl( CNS3XXX_PCIE0_INT_STATUS );
#ifdef CNS3XXX_PCIE_DEBUG
	printk(KERN_DEBUG "PCIe0 Device interrupt status: 0x%x\n", status);
#endif

	/* Device interrupt is self-clear */
//      __raw_writel(status, CNS3XXX_PCIE0_INT_STATUS );
#endif

	return IRQ_HANDLED;
}

static irqreturn_t cns3xxx_pcie0_rc_intc(int irq, void *dev_id)
{
	u32 status;

	/* Read PCIe0 interrupt */
	status = __raw_readl( CNS3XXX_PCIE0_INT_STATUS );

//	if ((status & 0xFF) == 1)
//		printk(KERN_DEBUG "PCIe0 interrupt status: 0x%x\n",status);
//	else
//		printk(KERN_DEBUG "PCIe0 RC interrupt status: 0x%x\n",status);

	/* Write 1 to clear PCIe0 RC interrupt */
	status &= 0xFFF0;
	if (status)
		__raw_writel(status, CNS3XXX_PCIE0_INT_STATUS );

	return IRQ_HANDLED;
}

static irqreturn_t cns3xxx_pcie1_dev_intc(int irq, void *dev_id)
{

#if 0
	u32 status;
	/* Read PCIe0 interrupt */
	status = __raw_readl( CNS3XXX_PCIE1_INT_STATUS );
#ifdef CNS3XXX_PCIE_DEBUG
	printk(KERN_DEBUG "PCIe0 Device interrupt status: 0x%x\n", status);
#endif

	/* Device interrupt is self-clear */
//      __raw_writel( CNS3XXX_PCIE1_INT_STATUS );
#endif

	return IRQ_HANDLED;
}

static irqreturn_t cns3xxx_pcie1_rc_intc(int irq, void *dev_id)
{
	u32 status;

	/* Read PCIe0 interrupt */
	status = __raw_readl( CNS3XXX_PCIE1_INT_STATUS );

#if 0
#ifdef CNS3XXX_PCIE_DEBUG
      if ((status & 0xFF) == 1)
              printk(KERN_DEBUG "PCIe1 interrupt status: 0x%x\n",status);
      else
	      printk(KERN_DEBUG "PCIe1 RC interrupt status: 0x%x\n",status);
#endif
#endif

	/* Write 1 to clear PCIe0 RC interrupt */
	status &= 0xFFF0;
	if (status)
		__raw_writel(status, CNS3XXX_PCIE1_INT_STATUS );

	return IRQ_HANDLED;
}


int __init cns3xxx_pci_setup(int nr, struct pci_sys_data *sys)
{
	if (nr > 0) {
		printk("[RLDBG] NR > 1 \n");
		return 0;
	}

	if (cns3xxx_pci_setup_resources(sys->domain,sys->resource)) {
		BUG();
	}

	return 1;
}

struct pci_bus *cns3xxx_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
	return pci_scan_bus(sys->busnr, &cns3xxx_pcie_ops, sys);
}

/* 
 *   CNS3XXX PCIe device don't support hotplugin, and we will check the link at start up. 
 *
 */
static void cns3xxx_pcie_check_link(int port)
{

	u32 reg;
	u32 time;
	reg = __raw_readl(port == 0 ? CNS3XXX_PCIE0_CTRL : CNS3XXX_PCIE1_CTRL);
	//      reg |= 0x1;
	/* Enable Appluication Request to 1, it will exit L1 automatically , 
	   but when chip back, it will use another clock, still can use 0x1 */
	reg |= 0x3;
	__raw_writel(reg, port == 0 ? CNS3XXX_PCIE0_CTRL : CNS3XXX_PCIE1_CTRL);
	printk("PCIe: Port[%d] Enable PCIe LTSSM\n", port);
	printk("PCIe: Port[%d] Check data link layer...", port);

	time = jiffies;		/* set the start time for the receive */
	while (1) {
		reg = __raw_readl( port == 0 ? CNS3XXX_PCIE0_PM_DEBUG : CNS3XXX_PCIE1_PM_DEBUG);	/* check link up */
		reg =
		    __raw_readl(port ==
				0 ? CNS3XXX_PCIE0_PM_DEBUG :
				CNS3XXX_PCIE1_PM_DEBUG);
		if (reg & 0x1) {
			printk("Link up.\n");
			pcie_linked[port] = 1;
			break;
		} else if (time_after(jiffies, (unsigned long)(time + 50))) {
			printk("Device not found.\n");
			break;
		}
	}

}

static void cns3xxx_pcie_hw_init(int port){
	struct pci_bus bus;
	struct pci_sys_data sd;
	u32 devfn = 0;
	u8 pri_bus, sec_bus, sub_bus;
	u16 pos, dc;
	u32 mem_base, host_base, io_base, cfg0_base;
	//int i;

	bus.number = 0; 
	bus.ops    = &cns3xxx_pcie_ops;
	sd.domain = port;
	bus.sysdata = &sd;
	

	mem_base = ( port == 0 ? CNS3XXX_PCIE0_MEM_BASE : CNS3XXX_PCIE1_MEM_BASE );
	mem_base = mem_base >> 16;

	io_base = ( port == 0 ? CNS3XXX_PCIE0_IO_BASE : CNS3XXX_PCIE1_IO_BASE );
	io_base = io_base >> 16;

	host_base = ( port == 0 ? CNS3XXX_PCIE0_HOST_BASE_VIRT : CNS3XXX_PCIE1_HOST_BASE_VIRT );
	host_base = ( host_base -1 ) >> 16;

	cfg0_base = ( port == 0 ? CNS3XXX_PCIE0_CFG0_BASE_VIRT : CNS3XXX_PCIE1_CFG0_BASE_VIRT );
	cfg0_base = ( cfg0_base -1 ) >> 16;

	pci_bus_write_config_byte(&bus, devfn, PCI_PRIMARY_BUS, 0);
	pci_bus_write_config_byte(&bus, devfn, PCI_SECONDARY_BUS, 1);
	pci_bus_write_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, 1);

	pci_bus_read_config_byte(&bus, devfn, PCI_PRIMARY_BUS, &pri_bus);
	pci_bus_read_config_byte(&bus, devfn, PCI_SECONDARY_BUS, &sec_bus);
	pci_bus_read_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, &sub_bus);

	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_BASE, mem_base);
	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_LIMIT, host_base);
	pci_bus_write_config_word(&bus, devfn, PCI_IO_BASE_UPPER16, io_base);
	pci_bus_write_config_word(&bus, devfn, PCI_IO_LIMIT_UPPER16, cfg0_base);

	/* Modify device's Max_Read_Request size */
	devfn = PCI_DEVFN(1,0);
	if (!pcie_linked[port])
		return;

	/* Set Device Max_Read_Request_Size to 128 byte */
	pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, &dc);
	dc &= ~(0x3 << 12);	/* Clear Device Control Register [14:12] */
	pci_bus_write_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, dc);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, &dc);
	if (!(dc & (0x3 << 12)))
		printk ("PCIe: Set Device Max_Read_Request_Size to 128 byte\n");

}

#include <mach/board.h>

#define GPIOA_MEM_MAP_VALUE(reg_offset)	(*((uint32_t volatile *)(CNS3XXX_GPIOA_BASE_VIRT + reg_offset)))
#define GPIOA_INPUT				GPIOA_MEM_MAP_VALUE(0x004)
#define GPIOA_DIR					GPIOA_MEM_MAP_VALUE(0x008)

void __init cns3xxx_pcie0_preinit(void)
{
	int iInternalSource = 0;
  	u32 u32tmp;
	
	GPIOA_DIR &= ~(0xc00);
//	printk("\nGPIOA_INPUT:0x%.8x\n\n", GPIOA_INPUT);

	if((GPIOA_INPUT & 0x400) && (GPIOA_INPUT & 0x800)) {
//		printk("Board version 1.x.\n");
#ifdef CONFIG_CNS3XXX_AUTO_CLOCK_SOURCE
		iInternalSource = 0;
#endif
	}
	else {
//		printk("Board version 2.x.\n");
#ifdef CONFIG_CNS3XXX_AUTO_CLOCK_SOURCE
		iInternalSource = 1;
#endif
	}

#ifdef CONFIG_CNS3XXX_INTERNAL_CLOCK_SOURCE
	iInternalSource = 1;
#endif

	if(iInternalSource) {
		printk("PCI-E0 uses internal clock source.\n");
		/* Turn on PCIe clock source */
		cns3xxx_pwr_power_up(1<<PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB);

		/* Enable PMU CLK */
//		printk("Enable PCIe0 PMU CLK \n");
		u32tmp = __raw_readl(CNS3XXX_PM_BASE_VIRT + 0x14);
		u32tmp |= 0x1 << 28;
		u32tmp |= 0x1 << 29;
		__raw_writel(u32tmp, CNS3XXX_PM_BASE_VIRT + 0x14);

		/* Enable PCIe0 Internal Clk */
//		printk("Enable PCIe0 Internal CLK \n");
		u32tmp = __raw_readl(CNS3XXX_MISC_BASE_VIRT + 0x900);
		u32tmp |= 0x1 << 11;
		__raw_writel(u32tmp, CNS3XXX_MISC_BASE_VIRT + 0x900);
		
		/*equalizer setting*/
		u32tmp = 0xe2c;
		__raw_writel(u32tmp, CNS3XXX_MISC_BASE_VIRT + 0x940);
	}
	else {
		printk("PCI-E0 uses external clock source.\n");
	}

//	printk("Active PCIe0 Clock, PHY and reset PCIe \n");
	/* Enable PCIe Clock */
	cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_PCIE0);

	/* Software Reset PCIe */
	cns3xxx_pwr_soft_rst( 0x1 << PM_SOFT_RST_REG_OFFST_PCIE0 );

#ifdef CNS3XXX_PCIE_RESET
		/* Reset device */
        cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
        cns3xxx_pwr_power_up(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
        cns3xxx_pwr_soft_rst(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);

        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x8);
        u32tmp |= (0x1 << 21);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x8);

        udelay(500);

        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x0);
        u32tmp |= (0x1 << 21);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x0);

#if 0
        mdelay(10);
        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x0);
        u32tmp &= ~(0x1 << 21);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x0);

        mdelay(10);
        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x0);
        u32tmp |= (0x1 << 21);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x0);
#endif

#endif

	cns3xxx_pcie_check_link(0);
	cns3xxx_pcie_hw_init(0);
}
void __init cns3xxx_pcie1_preinit(void)
{
	int iInternalSource = 0;
  	u32 u32tmp;
	
	GPIOA_DIR &= ~(0xc00);
//	printk("\nGPIOA_INPUT:0x%.8x\n\n", GPIOA_INPUT);

	if((GPIOA_INPUT & 0x400) && (GPIOA_INPUT & 0x800)) {
//		printk("Board version 1.x.\n");
#ifdef CONFIG_CNS3XXX_AUTO_CLOCK_SOURCE
		iInternalSource = 0;
#endif
	}
	else {
//		printk("Board version 2.x.\n");
#ifdef CONFIG_CNS3XXX_AUTO_CLOCK_SOURCE
		iInternalSource = 1;
#endif
	}

#ifdef CONFIG_CNS3XXX_INTERNAL_CLOCK_SOURCE
	iInternalSource = 1;
#endif

	if(iInternalSource) {
		printk("PCI-E1 uses internal clock source.\n");
		/* Turn on PCIe clock source */
		cns3xxx_pwr_power_up(1<<PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB);

		/* Enable PMU CLK */
//		printk("Enable PCIe PMU CLK \n");
		u32tmp = __raw_readl(CNS3XXX_PM_BASE_VIRT + 0x14);
		/* Bit-28 enable internal reference clock for port0 and port1 
		 * Bit-29 enable reference clock out for port1 */
		u32tmp |= 0x3 << 28; 
		__raw_writel(u32tmp, CNS3XXX_PM_BASE_VIRT + 0x14);

		/* Enable PCIe1 Internal Clk */
//		printk("Enable PCIe1 Internal CLK \n");
		u32tmp = __raw_readl(CNS3XXX_MISC_BASE_VIRT + 0xA00);
		u32tmp |= 0x1 << 11;
		__raw_writel(u32tmp, CNS3XXX_MISC_BASE_VIRT + 0xA00);

		/*equalizer setting*/
		u32tmp = 0xe2c;
		__raw_writel(u32tmp, CNS3XXX_MISC_BASE_VIRT + 0xA40);
	}
	else {
		printk("PCI-E1 uses external clock source.\n");
	}

//	printk("Active PCIe1 Clock, PHY and reset PCIe \n");
        /* Enable PCIe Clock */
        cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_PCIE1);

	/* Software Reset PCIe */
	cns3xxx_pwr_soft_rst( 0x1 << PM_SOFT_RST_REG_OFFST_PCIE1 );

#ifdef CNS3XXX_PCIE_RESET

	/* If onlt enable PCIe1, please unmark for enable GPIO setting */
#if 0
        cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
        cns3xxx_pwr_power_up(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
        cns3xxx_pwr_soft_rst(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
#endif

        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x8);
        u32tmp |= (0x1 << 22);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x8);

        udelay(500);

        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x0);
        u32tmp |= (0x1 << 22);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x0);

#if 0
        mdelay(10);
        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x0);
        u32tmp &= ~(0x1 << 22);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x0);

        mdelay(10);
        u32tmp = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + 0x0);
        u32tmp |= (0x1 << 22);
        __raw_writel(u32tmp, CNS3XXX_GPIOA_BASE_VIRT + 0x0);
#endif

#endif

	cns3xxx_pcie_check_link(1);
	cns3xxx_pcie_hw_init(1);
}

void __init cns3xxx_pcie0_postinit(void)
{
	u32 ret;
	u32 err;

	pcie0_bridge = pci_get_device(PCIB_VENDOR_ID, PCIB_DEVICE_ID, NULL);
	if (pcie0_bridge == NULL) {
		printk("PCIe0: Bridge not found.\n");
		return;
	} else {
		printk("PCIe0: Bridge found.\n");
	}

	if ((ret =
	     request_irq(IRQ_CNS3XXX_PCIE0_DEVICE, cns3xxx_pcie0_dev_intc,
			 IRQF_SHARED, "PCIe Port 0 Device", pcie0_bridge))) {
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",
		       __FUNCTION__, IRQ_CNS3XXX_PCIE0_DEVICE, ret, -EBUSY);
		return;
	}

	if ((ret =
	     request_irq(IRQ_CNS3XXX_PCIE0_RC, cns3xxx_pcie0_rc_intc,
			 IRQF_SHARED, "PCIe Port 0 RC", pcie0_bridge))) {
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",
		       __FUNCTION__, IRQ_CNS3XXX_PCIE0_RC, ret, -EBUSY);
		return;
	}

	//MISC_PCI_ARBITER_INTERRUPT_MASK_REG &= ~0x1f;
	pci_write_config_dword(pcie0_bridge, PCI_BASE_ADDRESS_0, 0x0);	// = 0x0, can NOT use 0x20000000
	pci_write_config_dword(pcie0_bridge, PCI_BASE_ADDRESS_1, 0x0);	// = 0x0, can NOT use 0x20000000

	// if we enable pci on u-boot
	// the pci_enable_device will complain with resource collisions
	// use this to fixup
	{
		int i;
		struct resource *r;

		for (i = 0; i < 6; i++) {
			r = pcie0_bridge->resource + i;
			r->start = 0;
			r->end = 0;
		}
	}

	err = pci_enable_device(pcie0_bridge);
	pci_set_master(pcie0_bridge);

	/* Disable PCIe0 Interrupt Mask INTA to INTD */
	__raw_writel(~0x3FFF, CNS3XXX_MISC_BASE_VIRT + 0x978);
}

void __init cns3xxx_pcie1_postinit(void)
{
	u32 ret;
	u32 err;

	pcie1_bridge = pci_get_device(PCIB_VENDOR_ID, PCIB_DEVICE_ID, NULL);
	if (pcie1_bridge == NULL) {
		printk("PCIe1: Bridge not found.\n");
		return;
	} else {
		printk("PCIe1: Bridge found.\n");
	}

	if ((ret =
	     request_irq(IRQ_CNS3XXX_PCIE1_DEVICE, cns3xxx_pcie1_dev_intc,
			 IRQF_SHARED, "PCIe Port 1 Device", pcie1_bridge))) {
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",
		       __FUNCTION__, IRQ_CNS3XXX_PCIE1_DEVICE, ret, -EBUSY);
		return;
	}

	if ((ret =
	     request_irq(IRQ_CNS3XXX_PCIE1_RC, cns3xxx_pcie1_rc_intc,
			 IRQF_SHARED, "PCIe Port 1 RC", pcie1_bridge))) {
		printk("%s: request_irq %d failed(ret=0x%x)(-EBUSY=0x%x)\n",
		       __FUNCTION__, IRQ_CNS3XXX_PCIE1_RC, ret, -EBUSY);
		return;
	}

	//MISC_PCI_ARBITER_INTERRUPT_MASK_REG &= ~0x1f;
	pci_write_config_dword(pcie1_bridge, PCI_BASE_ADDRESS_0, 0x0);	// = 0x0, can NOT use 0x20000000
	pci_write_config_dword(pcie1_bridge, PCI_BASE_ADDRESS_1, 0x0);	// = 0x0, can NOT use 0x20000000

	// if we enable pci on u-boot
	// the pci_enable_device will complain with resource collisions
	// use this to fixup
	{
		int i;
		struct resource *r;

		for (i = 0; i < 6; i++) {
			r = pcie1_bridge->resource + i;
			r->start = 0;
			r->end = 0;
		}
	}

	err = pci_enable_device(pcie1_bridge);
	pci_set_master(pcie1_bridge);

	/* Disable PCIe1 Interrupt Mask INTA to INTD */
	__raw_writel(~0x3FFF, CNS3XXX_MISC_BASE_VIRT + 0xA78);
}
/*
 * map the specified device/slot/pin to an IRQ.   Different backplanes may need to modify this.
 */

static int __init cns3xxx_pcie0_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	/* slot,  pin,  irq
	 * 0      1     PCIe 0 Device, 61
	 * 1      1     PCIe 0 RC,     88
	 * 2      1     PCIe 1 Device, 62
	 * 3      1     PCIe 1 RC,     89
	 */
	irq = cns3xxx_pcie0_irqs[slot];

	printk("PCIe0 map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
	       pci_domain_nr(dev->bus),dev->bus->number, PCI_SLOT(dev->devfn),
	       PCI_FUNC(dev->devfn), slot, pin, irq);

	return irq;
}

static int __init cns3xxx_pcie1_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	/* slot,  pin,  irq
	 * 0      1     PCIe 0 Device, 61
	 * 1      1     PCIe 0 RC,     88
	 * 2      1     PCIe 1 Device, 62
	 * 3      1     PCIe 1 RC,     89
	 */
	irq = cns3xxx_pcie1_irqs[slot];

	printk("PCIe1 map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
	       pci_domain_nr(dev->bus),dev->bus->number, PCI_SLOT(dev->devfn),
	       PCI_FUNC(dev->devfn), slot, pin, irq);

	return irq;
}

static struct hw_pci cns3xxx_pcie0 __initdata = {
	.swizzle = pci_std_swizzle,
	.map_irq = cns3xxx_pcie0_map_irq,
	.nr_controllers = 1,
	.nr_domains = 0,
	.setup = cns3xxx_pci_setup,
	.scan = cns3xxx_pci_scan_bus,
	.preinit = cns3xxx_pcie0_preinit,
	.postinit = cns3xxx_pcie0_postinit,
};
static struct hw_pci cns3xxx_pcie1 __initdata = {
	.swizzle = pci_std_swizzle,
	.map_irq = cns3xxx_pcie1_map_irq,
	.nr_controllers = 1,
	.nr_domains = 1,
	.setup = cns3xxx_pci_setup,
	.scan = cns3xxx_pci_scan_bus,
	.preinit = cns3xxx_pcie1_preinit,
	.postinit = cns3xxx_pcie1_postinit,
};

#ifdef CONFIG_DEBUG_FS
struct pcie_dbgfs_reg dbgfs_reg[] = {
	{"PCIEPHY0_CMCTL0", 	(u32 *)CNS3XXX_PCIEPHY0_CMCTL0},
	{"PCIEPHY0_CMCTL1",	(u32 *)CNS3XXX_PCIEPHY0_CMCTL1},
	{"PCIEPHY0_CTL1", 	(u32 *)CNS3XXX_PCIEPHY0_CTL1},
	{"PCIE0_AXIS_AWMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_AWMISC},
	{"PCIE0_AXIS_ARMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_ARMISC},
	{"PCIE0_AXIS_RMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_RMISC},
	{"PCIE0_AXIS_BMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_BMISC},
	{"PCIE0_AXIM_RMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_RMISC},
	{"PCIE0_AXIM_BMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_BMISC},
	{"PCIE0_CTRL", 		(u32 *)CNS3XXX_PCIE0_CTRL},
	{"PCIE0_PM_DEBUG", 	(u32 *)CNS3XXX_PCIE0_PM_DEBUG},
	{"PCIE0_RFC_DEBUG", 	(u32 *)CNS3XXX_PCIE0_RFC_DEBUG},
	{"PCIE0_CXPL_DEBUGL", 	(u32 *)CNS3XXX_PCIE0_CXPL_DEBUGL},
	{"PCIE0_CXPL_DEBUGH", 	(u32 *)CNS3XXX_PCIE0_CXPL_DEBUGH},
	{"PCIE0_DIAG", 		(u32 *)CNS3XXX_PCIE0_DIAG},
	{"PCIE0_INT_STATUS", 	(u32 *)CNS3XXX_PCIE0_INT_STATUS},
	{"PCIE0_INT_MASK", 	(u32 *)CNS3XXX_PCIE0_INT_MASK},
	{"PCIEPHY1_CMCTL0", 	(u32 *)CNS3XXX_PCIEPHY0_CMCTL0},
	{"PCIEPHY1_CMCTL1",	(u32 *)CNS3XXX_PCIEPHY0_CMCTL1},
	{"PCIEPHY1_CTL1", 	(u32 *)CNS3XXX_PCIEPHY0_CTL1},
	{"PCIE1_AXIS_AWMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_AWMISC},
	{"PCIE1_AXIS_ARMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_ARMISC},
	{"PCIE1_AXIS_RMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_RMISC},
	{"PCIE1_AXIS_BMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_BMISC},
	{"PCIE1_AXIM_RMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_RMISC},
	{"PCIE1_AXIM_BMISC", 	(u32 *)CNS3XXX_PCIE0_AXIS_BMISC},
	{"PCIE1_CTRL", 		(u32 *)CNS3XXX_PCIE0_CTRL},
	{"PCIE1_PM_DEBUG", 	(u32 *)CNS3XXX_PCIE0_PM_DEBUG},
	{"PCIE1_RFC_DEBUG", 	(u32 *)CNS3XXX_PCIE0_RFC_DEBUG},
	{"PCIE1_CXPL_DEBUGL", 	(u32 *)CNS3XXX_PCIE0_CXPL_DEBUGL},
	{"PCIE1_CXPL_DEBUGH", 	(u32 *)CNS3XXX_PCIE0_CXPL_DEBUGH},
	{"PCIE1_DIAG", 		(u32 *)CNS3XXX_PCIE0_DIAG},
	{"PCIE1_INT_STATUS", 	(u32 *)CNS3XXX_PCIE0_INT_STATUS},
	{"PCIE1_INT_MASK", 	(u32 *)CNS3XXX_PCIE0_INT_MASK},
	{0,0}
	};
static void pcie_debugfs_init(void){

	struct dentry *pcie_dir;
	//char *data;
	int i=0;

	if( cns3xxx_debugfs_dir == NULL ){
		printk("Create Debug fs failed \n");
		BUG();
	}

	pcie_dir = debugfs_create_dir("pcie", cns3xxx_debugfs_dir);

	if( pcie_dir != NULL ) {
		while( dbgfs_reg[i].name != 0 ){
		//	printk("name: %s addr : %08x \n", dbgfs_reg[i].name, dbgfs_reg[i].addr);
			debugfs_create_x32(dbgfs_reg[i].name, 0644, pcie_dir, dbgfs_reg[i].addr);
			i++;
		}
	}

}
#endif

static int cns3xxx_pcie_abort_handler(unsigned long addr, unsigned int fsr,
				      struct pt_regs *regs)
{
	/*
	 * It seems we can't identify if that was really PCIe that caused an
	 * imprecise external abort (PCIe status register doesn't report that).
	 * So ignore all aborts, for now.
	 *
	 * Later, if there is really no way to ask hardware about the cause
	 * of the abort, we'll have to use some global variable under irqsave
	 * spinlock during the time we access the PCIe config space.
	 */
	return 0;
}

extern void pci_common_init(struct hw_pci *);

int cns3xxx_pcie_init(u8 ports)
{

	printk("CNS3XXX PCIe Host Control Driver\n");

	cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
	cns3xxx_pwr_power_up(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);
	cns3xxx_pwr_soft_rst(0x1 << PM_CLK_GATE_REG_OFFSET_GPIO);

	/* hook in our fault handler for PCI errors */
	hook_fault_code(16 + 6, cns3xxx_pcie_abort_handler, SIGBUS,
			"imprecise external abort");

	if (ports & 1)
	    pci_common_init(&cns3xxx_pcie0);
	if (ports & 2)
	    pci_common_init(&cns3xxx_pcie1);

	pci_assign_unassigned_resources();

#ifdef CNS3XXX_PCIE_DEBUG
	cns3xxx_proc_init();
#endif
#ifdef CONFIG_DEBUG_FS
	pcie_debugfs_init();
#endif
	return 0;
}

