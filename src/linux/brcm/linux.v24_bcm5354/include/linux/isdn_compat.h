/* $Id: isdn_compat.h,v 1.53 2001/09/24 13:23:13 kai Exp $
 *
 * Linux ISDN subsystem
 * Compatibility for various Linux kernel versions
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */

#ifndef _LINUX_ISDN_COMPAT_H
#define _LINUX_ISDN_COMPAT_H

#ifdef __KERNEL__

#ifndef ISDN_COMPAT_NOT_GENERIC
/* when using std2kern -u, this part is left out and instead provided
   by the .ctrl files */

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18)

#define set_current_state(sta) (current->state = sta)
#define module_init(x)	int init_module(void) { return x(); }
#define module_exit(x)	void cleanup_module(void) { x(); }
#define BUG() do { printk("kernel BUG at %s:%d!\n", __FILE__, __LINE__); *(int *)0 = 0; } while (0)
#define init_MUTEX(x)				*(x)=MUTEX
#define init_MUTEX_LOCKED(x)			*(x)=MUTEX_LOCKED
#define __devinit
#define __devinitdata

#else /* 2.2.18 and later */

#define COMPAT_HAS_NEW_SETUP
#define COMPAT_HAS_NEW_WAITQ

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)

#define dev_kfree_skb_irq(a) dev_kfree_skb(a)
#define dev_kfree_skb_any(a) dev_kfree_skb(a)
#define COMPAT_HAS_2_2_PCI
#define get_pcibase(ps, nr) ps->base_address[nr]
#define pci_resource_start_io(pdev, nr) ((pdev)->base_address[nr] & PCI_BASE_ADDRESS_IO_MASK)
#define pci_resource_start_mem(pdev, nr) ((pdev)->base_address[nr] & PCI_BASE_ADDRESS_MEM_MASK)
#define pci_get_sub_vendor(pdev, id)	pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &id)
#define pci_get_sub_system(pdev, id)	pci_read_config_word(pdev, PCI_SUBSYSTEM_ID, &id)

#define __exit
#define __devinit
#define __devinitdata

#define net_device device
#define COMPAT_NO_SOFTNET
#define netif_running(d) test_bit(LINK_STATE_START, &d->state)
#define COMPAT_NEED_MPPP_DEFS
#define spin_lock_bh(lock)
#define spin_unlock_bh(lock)
#define COMPAT_NEED_SPIN_LOCK_BH
#define i_count_read(ic) ic
#define i_count_inc(ic)  ic++
#define COMPAT_USE_MODCOUNT_LOCK
#define devfs_register_chrdev(m,n,f) register_chrdev(m,n,f)
#define devfs_unregister_chrdev(m,n) unregister_chrdev(m,n)
#define COMPAT_NEED_PCI_IDS
#define in_irq() (local_irq_count[smp_processor_id()] != 0)

#else /* 2.4.0 and later */

#define pci_resource_start_io(pdev, nr) pci_resource_start(pdev, nr)
#define pci_resource_start_mem(pdev, nr) pci_resource_start(pdev, nr)
#define get_pcibase(ps, nr) ps->resource[nr].start
#define pci_get_sub_system(pdev, id)	id = pdev->subsystem_device
#define pci_get_sub_vendor(pdev, id)	id = pdev->subsystem_vendor

#define BIG_PHONE_NUMBERS
#define COMPAT_HAS_ISA_IOREMAP
#define i_count_read(ic) atomic_read(&ic)
#define i_count_inc(ic)  atomic_inc(&ic)
#define COMPAT_HAS_FILEOP_OWNER
#define COMPAT_HAVE_NEW_FILLDIR
#define COMPAT_has_fileops_in_inode
#define COMPAT_HAS_init_special_inode
#define COMPAT_d_alloc_root_one_parameter
#define HAVE_DEVFS_FS
#define COMPAT_HAS_SCHEDULE_TASK
#define COMPAT_HAS_USB_IDTAB

#endif

#endif /* ISDN_COMPAT_GENERIC */

#ifdef COMPAT_HAS_2_2_PCI 
#include <linux/pci.h>
#ifdef __powerpc__
static inline int pci_enable_device(struct pci_dev *dev)
{
	u16 cmd;
	pci_read_config_word(dev, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MEMORY | PCI_COMMAND_IO | PCI_COMMAND_SERR;
	cmd &= ~PCI_COMMAND_FAST_BACK;
	pci_write_config_word(dev, PCI_COMMAND, cmd);
	return(0);
}
#else
static inline int pci_enable_device(struct pci_dev *dev)
{
	return 0;
}
#endif /* __powerpc__ */

#define PCI_ANY_ID (~0)

/* as this is included multiple times, we make it inline */

static inline struct pci_dev * pci_find_subsys(unsigned int vendor, unsigned int device,
					unsigned int ss_vendor, unsigned int ss_device,
					struct pci_dev *from)
{
	unsigned short subsystem_vendor, subsystem_device;

	while ((from = pci_find_device(vendor, device, from))) {
		pci_read_config_word(from, PCI_SUBSYSTEM_VENDOR_ID, &subsystem_vendor);
		pci_read_config_word(from, PCI_SUBSYSTEM_ID, &subsystem_device);
		if ((ss_vendor == PCI_ANY_ID || subsystem_vendor == ss_vendor) &&
		    (ss_device == PCI_ANY_ID || subsystem_device == ss_device))
			return from;
	}
	return NULL;
}
#endif

#ifdef COMPAT_NO_SOFTNET
#include <linux/netdevice.h>

/*
 * Tell upper layers that the network device is ready to xmit more frames.
 */
static void __inline__ netif_wake_queue(struct net_device * dev)
{
	dev->tbusy = 0;
	mark_bh(NET_BH);
}

/*
 * called during net_device open()
 */
static void __inline__ netif_start_queue(struct net_device * dev)
{
	dev->tbusy = 0;
	/* actually, we never use the interrupt flag at all */
	dev->interrupt = 0;
	dev->start = 1;
}

/*
 * Ask upper layers to temporarily cease passing us more xmit frames.
 */
static void __inline__ netif_stop_queue(struct net_device * dev)
{
	dev->tbusy = 1;
}

#endif /* COMPAT_NO_SOFTNET */

#ifndef COMPAT_HAS_NEW_WAITQ
typedef struct wait_queue wait_queue_t;
typedef struct wait_queue *wait_queue_head_t;

#define DECLARE_WAITQUEUE(wait, current)	struct wait_queue wait = { current, NULL }
#define DECLARE_WAIT_QUEUE_HEAD(wait)		wait_queue_head_t wait
#define init_waitqueue_head(x)			*(x)=NULL
#define init_waitqueue_entry(q,p)		((q)->task)=(p)
#endif /* COMPAT_HAS_NEW_WAITQ */

#ifdef COMPAT_NEED_PCI_IDS

#define PCI_ANY_ID (~0)

#define PCI_VENDOR_ID_DYNALINK          0x0675
#define PCI_DEVICE_ID_DYNALINK_IS64PH   0x1702

#define PCI_DEVICE_ID_WINBOND2_6692	0x6692

#define PCI_DEVICE_ID_PLX_R685		0x1030
#define PCI_DEVICE_ID_PLX_DJINN_ITOO    0x1151
#define PCI_DEVICE_ID_PLX_R753          0x1152

#define PCI_VENDOR_ID_ELSA		0x1048
#define PCI_DEVICE_ID_ELSA_MICROLINK	0x1000
#define PCI_DEVICE_ID_ELSA_QS3000	0x3000

#define PCI_VENDOR_ID_EICON		0x1133
#define PCI_DEVICE_ID_EICON_DIVA20PRO	0xe001
#define PCI_DEVICE_ID_EICON_DIVA20	0xe002
#define PCI_DEVICE_ID_EICON_DIVA20PRO_U	0xe003
#define PCI_DEVICE_ID_EICON_DIVA20_U	0xe004
#define PCI_DEVICE_ID_EICON_DIVA201	0xe005
#define PCI_DEVICE_ID_EICON_MAESTRA	0xe010
#define PCI_DEVICE_ID_EICON_MAESTRAQ	0xe012
#define PCI_DEVICE_ID_EICON_MAESTRAQ_U	0xe013
#define PCI_DEVICE_ID_EICON_MAESTRAP	0xe014
 
#define PCI_VENDOR_ID_CCD		0x1397
#define PCI_DEVICE_ID_CCD_2BD0	        0x2BD0
#define PCI_DEVICE_ID_CCD_B000	        0xB000
#define PCI_DEVICE_ID_CCD_B006	        0xB006
#define PCI_DEVICE_ID_CCD_B007	        0xB007
#define PCI_DEVICE_ID_CCD_B008	        0xB008
#define PCI_DEVICE_ID_CCD_B009	        0xB009
#define PCI_DEVICE_ID_CCD_B00A	        0xB00A
#define PCI_DEVICE_ID_CCD_B00B	        0xB00B
#define PCI_DEVICE_ID_CCD_B00C	        0xB00C
#define PCI_DEVICE_ID_CCD_B100	        0xB100

#define PCI_VENDOR_ID_ASUSTEK           0x1043   
#define PCI_DEVICE_ID_ASUSTEK_0675      0x0675

#define PCI_VENDOR_ID_BERKOM		        0x0871
#define PCI_DEVICE_ID_BERKOM_A1T	        0xFFA1
#define PCI_DEVICE_ID_BERKOM_T_CONCEPT          0xFFA2
#define PCI_DEVICE_ID_BERKOM_A4T	        0xFFA4
#define PCI_DEVICE_ID_BERKOM_SCITEL_QUADRO      0xFFA8

#define PCI_DEVICE_ID_SATSAGEM_NICCY	0x1016

#define PCI_DEVICE_ID_TIGERJET_100	0x0002

#define PCI_VENDOR_ID_ANIGMA		0x1051
#define PCI_DEVICE_ID_ANIGMA_MC145575	0x0100

#define PCI_VENDOR_ID_ZOLTRIX		0x15b0
#define PCI_DEVICE_ID_ZOLTRIX_2BD0	0x2BD0

#define PCI_DEVICE_ID_DIGI_DF_M_IOM2_E	0x0070
#define PCI_DEVICE_ID_DIGI_DF_M_E	0x0071
#define PCI_DEVICE_ID_DIGI_DF_M_IOM2_A	0x0072
#define PCI_DEVICE_ID_DIGI_DF_M_A	0x0073

#define PCI_DEVICE_ID_AVM_B1		0x0700
#define PCI_DEVICE_ID_AVM_C4		0x0800
#define PCI_DEVICE_ID_AVM_C2		0x1100
#define PCI_DEVICE_ID_AVM_T1		0x1200

#define PCI_VENDOR_ID_HYPERCOPE		0x1365
#define PCI_DEVICE_ID_HYPERCOPE_PLX	0x9050
#define PCI_SUBDEVICE_ID_HYPERCOPE_OLD_ERGO     0x0104
#define PCI_SUBDEVICE_ID_HYPERCOPE_ERGO         0x0106
#define PCI_SUBDEVICE_ID_HYPERCOPE_METRO        0x0107
#define PCI_SUBDEVICE_ID_HYPERCOPE_CHAMP2       0x0108
#define PCI_SUBDEVICE_ID_HYPERCOPE_PLEXUS       0x0109

#define PCI_VENDOR_ID_ABOCOM            0x13D1
#define PCI_DEVICE_ID_ABOCOM_2BD1       0x2BD1

#endif /* COMPAT_NEED_PCI_IDS */

#endif /* __KERNEL__ */
#endif /* _LINUX_ISDN_COMPAT_H */
