/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software may be redistributed and/or modified under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 *
 * File: kcompat.h
 *
 * Purpose: Header file for Linux kernel version compatibility.
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#ifndef __KCOMPAT_H__
#define __KCOMPAT_H__

#include <linux/version.h>

#ifndef __init
#define __init
#endif

#ifndef __exit
#define __exit
#endif

#ifndef __devexit
#define __devexit
#endif

#ifndef __devinitdata
#define __devinitdata
#endif

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(license)
#endif

#ifndef MOD_INC_USE_COUNT
#define MOD_INC_USE_COUNT do {} while (0)
#endif
#ifndef MOD_DEC_USE_COUNT
#define MOD_DEC_USE_COUNT do {} while (0)
#endif

#ifndef HAVE_NETDEV_PRIV
#define netdev_priv(dev) (dev->priv)
#endif

#ifndef IRQ_RETVAL
typedef void irqreturn_t;
#define IRQ_RETVAL(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18)

typedef unsigned long dma_addr_t;
typedef struct wait_queue *wait_queue_head_t;
#define init_waitqueue_head(x)                  *(x)=NULL
#define set_current_state(status)       { current->state = (status); mb(); }

#ifdef MODULE

#define module_init(fn) int  init_module   (void) { return fn(); }
#define module_exit(fn) void cleanup_module(void) { return fn(); }

#else /* MODULE */

#define module_init(fn) int  rhinefet_probe    (void) { return fn(); }
#define module_exit(fn)  /* NOTHING */

#endif /* MODULE */

#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)

#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <asm/io.h>

#define pci_resource_start(dev, bar)                            \
    (((dev)->base_address[(bar)] & PCI_BASE_ADDRESS_SPACE_IO) ? \
    ((dev)->base_address[(bar)] & PCI_BASE_ADDRESS_IO_MASK) :   \
    ((dev)->base_address[(bar)] & PCI_BASE_ADDRESS_MEM_MASK))

/*
static inline unsigned long pci_resource_len(struct pci_dev *dev, int bar) {
    u32 old_addr, len;
    
    int bar_reg = PCI_BASE_ADDRESS_0 + (bar << 2);
    pci_read_config_dword(dev, bar_reg, &old_addr);
    pci_write_config_dword(dev, bar_reg, ~0);
    pci_read_config_dword(dev, bar_reg, &len);
    pci_write_config_dword(dev, bar_reg, old_addr);
    
    if ((len & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY)
        len = ~(len & PCI_BASE_ADDRESS_MEM_MASK);
    else
        len = ~(len & PCI_BASE_ADDRESS_IO_MASK);
    
    return (len+1);
}
*/

static inline int pci_enable_device(struct pci_dev *dev) { return 0; }
#define pci_disable_device(dev)  /* NOTHING */
#define __constant_cpu_to_le32 cpu_to_le32
#define __constant_cpu_to_le16 cpu_to_le16

#define PCI_DMA_TODEVICE   1
#define PCI_DMA_FROMDEVICE 2

extern inline void *pci_alloc_consistent (struct pci_dev *dev,
                                          size_t size,
                                          dma_addr_t *dma_handle) {
    void *vaddr = kmalloc(size, GFP_ATOMIC);
    if(vaddr != NULL) {
        *dma_handle = virt_to_bus(vaddr);
    }
    return vaddr;
}

#define pci_dma_sync_single(dev,dma_handle,size,direction)   do{} while(0)
#define pci_dma_supported(dev, addr_mask)                    (1)
#define pci_free_consistent(dev, size, cpu_addr, dma_handle) kfree(cpu_addr)
#define pci_map_single(dev, addr, size, direction)           virt_to_bus(addr)
#define pci_unmap_single(dev, dma_handle, size, direction)   do{} while(0)


#define spin_lock_bh            spin_lock_irq
#define spin_unlock_bh          spin_unlock_irq
#define del_timer_sync(timer)   del_timer(timer)
#define net_device              device

#define netif_start_queue(dev)   ( clear_bit(0, &(dev)->tbusy))
#define netif_stop_queue(dev)    (   set_bit(0, &(dev)->tbusy))
#define netif_wake_queue(dev)    { clear_bit(0, &(dev)->tbusy); \
                                   mark_bh(NET_BH); }
#define netif_running(dev)       (  test_bit(0, &(dev)->start))
#define netif_queue_stopped(dev) (  test_bit(0, &(dev)->tbusy))

#define dev_kfree_skb_irq(skb) dev_kfree_skb(skb)

#define netif_carrier_on(dev)  do {} while (0)
#define netif_carrier_off(dev) do {} while (0)


#define PCI_ANY_ID (~0U)

struct pci_device_id {
    unsigned int vendor, device;
    unsigned int subvendor, subdevice;
    unsigned int class, classmask;
    unsigned long driver_data;
};

#define MODULE_DEVICE_TABLE(bus, dev_table)
#define PCI_MAX_NUM_NICS 256

struct pci_driver {
    char *name;
    struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *dev);
    void (*suspend)(struct pci_dev *dev);
    void (*resume)(struct pci_dev *dev);
    struct pci_dev *pcimap[PCI_MAX_NUM_NICS];
};

static inline int pci_module_init(struct pci_driver *drv)
{
    struct pci_dev *pdev;
    struct pci_device_id *pcid;
    uint16_t subvendor, subdevice;
    int board_count = 0;

    /* walk the global pci device list looking for matches */
    for (pdev = pci_devices; pdev && (board_count < PCI_MAX_NUM_NICS); pdev = pdev->next) {

        pcid = &drv->id_table[0];
        pci_read_config_word(pdev, PCI_SUBSYSTEM_VENDOR_ID, &subvendor);
        pci_read_config_word(pdev, PCI_SUBSYSTEM_ID, &subdevice);

        while (pcid->vendor != 0) {
            if (((pcid->vendor == pdev->vendor) || (pcid->vendor == PCI_ANY_ID)) &&
                ((pcid->device == pdev->device) || (pcid->device == PCI_ANY_ID)) &&
                ((pcid->subvendor == subvendor) || (pcid->subvendor == PCI_ANY_ID)) &&
                ((pcid->subdevice == subdevice) || (pcid->subdevice == PCI_ANY_ID))) {

                if (drv->probe(pdev, pcid) == 0) {
                    drv->pcimap[board_count] = pdev;
                    board_count++;
                }
                break;
            }
            pcid++;
        }
    }

    if (board_count < PCI_MAX_NUM_NICS) {
        drv->pcimap[board_count] = NULL;
    }

    return (board_count > 0) ? 0 : -ENODEV;
}

static inline void pci_unregister_driver(struct pci_driver *drv)
{
    int i;

    for (i = 0; i < PCI_MAX_NUM_NICS; i++) {
        if (!drv->pcimap[i])
            break;

        drv->remove(drv->pcimap[i]);
    }
}


#define pci_set_drvdata(pcid, data)
//#define pci_get_drvdata(pcid) 0

#define pci_get_drvdata(pcid) ({                            \
    PRHINE_INFO pInfo;                                      \
    for (pInfo = pRhine3_Infos;                             \
        pInfo; pInfo = pInfo->next) {                       \
        if (pInfo->pcid == pcid)                            \
            break;                                          \
    }                                                       \
    pInfo; })

#define pci_set_dma_mask(pdev, mask) (0)


#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,5)
#if !defined(skb_linearize)
#define skb_linearize(skb, gfp_mask) ({     \
    struct sk_buff *tmp_skb;                \
    tmp_skb = skb;                          \
    skb = skb_copy(tmp_skb, gfp_mask);      \
    dev_kfree_skb_irq(tmp_skb); })
#endif
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,5) */

#ifndef MODULE_LICESEN
#define MODULE_LICESEN(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,2)
    static inline int pci_set_power_state(struct pci_dev *dev, int state) { return 0; }
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,6)

#include <linux/types.h>
#include <linux/pci.h>

#define PMCSR       0xe0
#define PM_ENABLE_BIT   0x0100
#define PM_CLEAR_BIT    0x8000
#define PM_STATE_MASK   0xFFFC
#define PM_STATE_D1 0x0001
static inline int
pci_enable_wake(struct pci_dev *dev, u32 state, int enable)
{
    u16 p_state;

    pci_read_config_word(dev, PMCSR, &p_state);
    pci_write_config_word(dev, PMCSR, p_state | PM_CLEAR_BIT);

    if (enable == 0) {
        p_state &= ~PM_ENABLE_BIT;
    } else {
        p_state |= PM_ENABLE_BIT;
    }
    p_state &= PM_STATE_MASK;
    p_state |= state;

    pci_write_config_word(dev, PMCSR, p_state);

    return 0;
}
#endif

#endif // __KCOMPAT_H__

