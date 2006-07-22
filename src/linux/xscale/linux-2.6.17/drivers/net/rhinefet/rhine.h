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
 * File: rhine.h
 *
 * Purpose: The extended driver header file.
 *
 * Author: Chuang Liang-Shing, AJ Jiang
 *
 * Date: Aug 15, 2003
 *
 */

#ifndef __RHINE_H__
#define __RHINE_H__

#ifdef MODULE
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif /* MODVERSIONS */
#include <linux/module.h>
#endif /* MODULE */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/wait.h>
#include <asm/io.h>

#include <linux/if.h>
#include <linux/config.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/inetdevice.h>
#include <linux/reboot.h>

#ifdef SIOCETHTOOL
#define RHINE_ETHTOOL_IOCTL_SUPPORT
#include <linux/ethtool.h>
#else
#undef RHINE_ETHTOOL_IOCTL_SUPPORT
#endif

#ifdef SIOCGMIIPHY
#define RHINE_MII_IOCTL_SUPPORT
#include <linux/mii.h>
#else
#undef RHINE_MII_IOCTL_SUPPORT
#endif

#if 0
//The skb fragments of this version isn't be tested.
#ifdef MAX_SKB_FRAGS
#define RHINE_ZERO_COPY_SUPPORT
#else
#undef  RHINE_ZERO_COPY_SUPPORT
#endif
#endif

#ifdef NETIF_F_IP_CSUM
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#define RHINE_TX_CSUM_SUPPORT
#else
#undef RHINE_TX_CSUM_SUPPORT
#endif

#include "kcompat.h"
#include "rhine_cfg.h"

#ifdef CONFIG_PROC_FS
#include "rhine_proc.h"
#endif
#include "rhine_hw.h"
#include "rhine_wol.h"

#ifdef RHINE_DEBUG
#define ASSERT(x) { \
    if (!(x)) { \
        printk(KERN_ERR "assertion %s failed: file %s line %d\n", #x,\
        __FUNCTION__, __LINE__);\
        *(int*) 0=0;\
    }\
}
#else
#define ASSERT(x)
#endif

typedef enum __rhine_msg_level {
    MSG_LEVEL_ERR=0,            //Errors that will cause abnormal operation.
    MSG_LEVEL_NOTICE=1,         //Some errors need users to be notified.
    MSG_LEVEL_INFO=2,           //Normal message.
    MSG_LEVEL_VERBOSE=3,        //Will report all trival errors.
    MSG_LEVEL_DEBUG=4           //Only for debug purpose.
} RHINE_MSG_LEVEL, *PRHINE_MSG_LEVEL;

typedef enum __rhine_init_type {
    RHINE_INIT_COLD=0,
    RHINE_INIT_RESET,
    RHINE_INIT_WOL
} RHINE_INIT_TYPE, *PRHINE_INIT_TYPE;

typedef enum __rhine_proc_rmon_type {
    RMON_DropEvents=0,
    RMON_Octets,
    RMON_Pkts,
    RMON_BroadcastPkts,
    RMON_MulticastPkts,
    RMON_CRCAlignErrors,
    RMON_UndersizePkts,
    RMON_OversizePkts,
    RMON_Fragments,
    RMON_Jabbers,
    RMON_Collisions,
    RMON_Pkts64Octets,
    RMON_Pkts65to127Octets,
    RMON_Pkts128to255Octets,
    RMON_Pkts256to511Octets,
    RMON_Pkts512to1023Octets,
    RMON_Pkts1024to1518Octets,
    RMON_TAB_SIZE
} PROC_RMON_TYPE, *PPROC_RMON_TYPE;

typedef enum _hw_mib_counter {
    HW_MIB_RxNoBuf=0,
    HW_MIB_RxErrPkts,
    HW_MIB_RxFCSErrs,
    HW_MIB_RxMSDPktErrs,
    HW_MIB_RxFAErrs,
    HW_MIB_RxFrameTooLong,
    HW_MIB_RxIRLErrs,
    HW_MIB_RxBadOpCode,
    HW_MIB_RxPauseFrame,
    HW_MIB_TxPauseFrame,
    HW_MIB_TxSQEErrs,
    HW_MIB_RxSymErrs,
    HW_MIB_SIZE
} HW_MIBS, *PHW_MIBS;

#define RHINE_PRT(l, p, args...) {if (l<=msglevel) printk( p ,##args);}
//
typedef struct __chip_info_tbl{
    CHIP_TYPE   chip_id;
    char*       name;
    U32         flags;
} CHIP_INFO, *PCHIP_INFO;

typedef struct {
    struct sk_buff*     skb;
    dma_addr_t          skb_dma;
    dma_addr_t          curr_desc;
} RHINE_RD_INFO,    *PRHINE_RD_INFO;

typedef struct {
    struct sk_buff*     skb;
    PU8                 buf;
    dma_addr_t          skb_dma;
    dma_addr_t          buf_dma;
    dma_addr_t          curr_desc;
} RHINE_TD_INFO,    *PRHINE_TD_INFO;

typedef struct __rhine_info {
    struct __rhine_info*        next;
    struct __rhine_info*        prev;

    struct pci_dev*             pcid;

#if CONFIG_PM
    U32                         pci_state[16];
#endif

    U8                          abyIPAddr[4];
    struct net_device*          dev;
    struct net_device_stats     stats;

    dma_addr_t                  pool_dma;
    dma_addr_t                  rd_pool_dma;
    dma_addr_t                  td_pool_dma[TX_QUEUE_NO];

    dma_addr_t                  tx_bufs_dma;
    PU8                         tx_bufs;

    CHIP_TYPE                   chip_id;

    
    /* define in rhine_hw.h */
    struct rhine_hw             hw;




    PRHINE_RD_INFO              aRDInfo;
    PRHINE_TD_INFO              apTDInfos[TX_QUEUE_NO];
    spinlock_t                  lock;
    spinlock_t                  xmit_lock;
    int                         NoRBuf;
    int                         wol_opts;
    U8                          wol_passwd[6];
    RHINE_CONTEXT               mac_context;
    U32                         pci_context;

#ifdef CONFIG_PROC_FS
    PRHINE_PROC_ENTRY           pProcDir;
#endif
    U32                         adwRMONStats[RMON_TAB_SIZE];
    U32                         adwHWMIBCounters[HW_MIB_SIZE];
    U32                         ticks;
    U32                         rx_bytes;

} RHINE_INFO, *PRHINE_INFO;



#define PCI_BYTE_REG_BITS_ON(x,i,p) do{\
    U8 byReg;\
    pci_read_config_byte((p), (i), &(byReg));\
    (byReg) |= (x);\
    pci_write_config_byte((p), (i), (byReg));\
} while (0)

#define PCI_BYTE_REG_BITS_OFF(x,i,p) do{\
    U8 byReg;\
    pci_read_config_byte((p), (i), &(byReg));\
    (byReg) &= (~(x));\
    pci_write_config_byte((p), (i), (byReg));\
} while (0)




#define WAIT_MAC_TX_OFF(pInfo)   do {} while (BYTE_REG_BITS_IS_ON(&(pInfo)->hw, CR0_TXON,MAC_REG_CR0))







inline static BOOL rhine_get_ip(PRHINE_INFO pInfo) {
    struct in_device* in_dev = (struct in_device*)pInfo->dev->ip_ptr;
    struct in_ifaddr* ifa;

    if (in_dev!=NULL) {
        ifa=(struct in_ifaddr*) in_dev->ifa_list;
        if (ifa!=NULL) {
            memcpy(pInfo->abyIPAddr, &ifa->ifa_address, 4);
            return TRUE;
        }
    }
    return FALSE;
}

inline static void rhine_UpdateMIBCounter(PRHINE_INFO pInfo) {
    int i;
    BYTE_REG_BITS_ON(&pInfo->hw, MIBCR_MIBRTN,MAC_REG_MIBCR);
    for (i=0;i<HW_MIB_SIZE;i++) {
        //U16 wData=readw(&pInfo->pMacRegs->wMIBPort);
        U16 wData=CSR_READ_2(&pInfo->hw, MAC_REG_MIBPORT);
        pInfo->adwHWMIBCounters[i]+=wData;

        if (i==HW_MIB_RxFCSErrs)
            pInfo->stats.rx_crc_errors+=wData;
        if (i==HW_MIB_RxMSDPktErrs)
            pInfo->stats.rx_missed_errors+=wData;
    }

}

void rhine_init_cam_filter(PRHINE_INFO pInfo);

#endif // __RHINE_H__
