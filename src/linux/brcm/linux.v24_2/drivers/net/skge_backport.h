/*
 * Backport hacks. Redefine world to look as close to current 2.6
 * as posssible.
 */

#include <linux/version.h>

#ifndef __iomem
#define __iomem
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define pci_dma_sync_single_for_device(pdev, addr, len, dir)
#define pci_dma_sync_single_for_cpu(pdev, addr, len, dir) \
	pci_dma_sync_single(pdev, addr, len, dir)

#else
#include <linux/dma-mapping.h>
#endif

#ifndef DMA_32BIT_MASK
#define DMA_64BIT_MASK	0xffffffffffffffffULL
#define DMA_32BIT_MASK	0x00000000ffffffffULL
#endif

#ifndef module_param
#define module_param(var, type, perm) \
	MODULE_PARM(var, "i");
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static unsigned long msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs) + 1;

	__set_current_state(TASK_INTERRUPTIBLE);
	while (timeout && !signal_pending(current))
		timeout = schedule_timeout(timeout);
	return jiffies_to_msecs(timeout);
}
#endif

#ifndef PCI_DEVICE
#define PCI_DEVICE(vend,dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif

#ifndef HAVE_NETDEV_PRIV
#define netdev_priv(dev)	((dev)->priv)
#endif

#ifndef ALIGN
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#endif

#ifndef NET_IP_ALIGN
#define NET_IP_ALIGN 2
#endif

#ifndef NETDEV_TX_OK
#define NETDEV_TX_OK 0		/* driver took care of packet */
#define NETDEV_TX_BUSY 1	/* driver tx path was busy*/
#endif

#ifndef IRQ_NONE
#define irqreturn_t	void
#define IRQ_NONE	
#define IRQ_HANDLED
#endif

#ifndef PCI_D0
#define PCI_D0		0
#define PCI_D1		1
#define PCI_D2		2
#define PCI_D3hot	3
#define PCI_D3cold	4
typedef int pci_power_t;
#endif

