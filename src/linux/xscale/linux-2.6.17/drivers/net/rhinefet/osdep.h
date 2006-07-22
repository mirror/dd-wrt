/*
 * Copyright (c) 1996, 2003 VIA Networking Technologies, Inc.
 * All rights reserved.
 *
 * This software is copyrighted by and is the sole property of
 * VIA Networking Technologies, Inc. This software may only be used
 * in accordance with the corresponding license agreement. Any unauthorized
 * use, duplication, transmission, distribution, or disclosure of this
 * software is expressly forbidden.
 *
 * This software is provided by VIA Networking Technologies, Inc. "as is"
 * and any express or implied warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose
 * are disclaimed. In no event shall VIA Networking Technologies, Inc.
 * be liable for any direct, indirect, incidental, special, exemplary, or
 * consequential damages.
 *
 *
 * File: _os_dep.h
 *
 * Purpose: OS depended function and macro defined, including register accessing.
 *
 * Author: Guard Kuo
 *
 * Date: Jan 28, 2005
 *
 *
 */
#ifndef ___OS_DEP_H__
#define ___OS_DEP_H__
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/sched.h>

typedef __u8    UCHAR,  *PUCHAR;
typedef __u16   U16,    *PU16;
typedef __u8    UINT8,  *PUINT8;
typedef __u32   U32,    *PU32;
typedef __u32   UINT32, *PUINT32;
typedef __u32   UINT,   *PUINT;
typedef __u8    BYTE,   *PBYTE;
typedef __u8    U8,     *PU8;
typedef __u32   BOOL,   *PBOOL;
typedef __u16   WORD,   *PWORD;
typedef __u32   DWORD,  *PDWORD;
typedef unsigned long   ULONG,  *PULONG;

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (!(FALSE))
#endif

/*
   little/big endian converting
*/
#if 0
#define cpu_to_le32(x) x
#define cpu_to_le16(x) x
#endif


/*
   delay function
*/
#if 0
#ifndef mdelay(x)
#define mdelay(x) x
#endif

#ifndef udelay(x)
#define udelay(x) x
#endif
#endif

// Power status setting
#ifndef PCI_D0
#define PCI_D0	0
#endif
#ifndef PCI_D1
#define PCI_D1	1
#endif
#ifndef PCI_D2
#define PCI_D2	2
#endif
#ifndef PCI_D3hot
#define PCI_D3hot	3
#endif
#ifndef PCI_D3cold
#define PCI_D3cold	4
#endif


/*
 */

#define RHINE_HW_PRT(p)    printk(p)


/*
 * register space access macros
 */
#define CSR_WRITE_4(hw, val, reg) writel(val, (hw)->hw_addr+reg)
#define CSR_WRITE_2(hw, val, reg) writew(val, (hw)->hw_addr+reg)
#define CSR_WRITE_1(hw, val, reg) writeb(val, (hw)->hw_addr+reg)

#define CSR_READ_4(hw, reg)   readl((hw)->hw_addr+reg)
#define CSR_READ_2(hw, reg)   readw((hw)->hw_addr+reg)
#define CSR_READ_1(hw, reg)   readb((hw)->hw_addr+reg)

#define _INB(hw, reg)       inb((hw)->ioaddr+reg)
#define _OUTB(hw, val, reg) outb(val, (hw)->ioaddr+reg)

#define TX_QUEUE_NO         8


#define rhine_update_rx_stats(dst, src) \
    (dst).rx_errors         +=(src).rx_errors; \
    (dst).rx_dropped        +=(src).rx_dropped;    \
    (dst).rx_crc_errors     +=(src).rx_crc_errors;  \
    (dst).rx_frame_errors   +=(src).rx_frame_errors;  \
    (dst).rx_fifo_errors    +=(src).rx_fifo_errors
    

#define rhine_update_tx_stats(dst, src) \
    (dst).tx_packets            +=(src).tx_packets; \
    (dst).tx_bytes              +=(src).tx_bytes; \
    (dst).tx_errors             +=(src).tx_errors; \
    (dst).tx_dropped            +=(src).tx_dropped; \
    (dst).tx_fifo_errors        +=(src).tx_fifo_errors; \
    (dst).tx_aborted_errors     +=(src).tx_aborted_errors; \
    (dst).tx_carrier_errors     +=(src).tx_carrier_errors; \
    (dst).tx_window_errors      +=(src).tx_window_errors; \
    (dst).tx_heartbeat_errors   +=(src).tx_heartbeat_errors; \
    (dst).collisions            +=(src).collisions


#define rhine_reset_rx_stats(pStats) \
    (pStats)->rx_errors=0;        \
    (pStats)->rx_dropped=0;       \
    (pStats)->rx_crc_errors=0;    \
    (pStats)->rx_frame_errors=0;  \
    (pStats)->rx_fifo_errors=0


#define rhine_reset_tx_stats(pStats) \
    (pStats)->tx_packets=0;           \
    (pStats)->tx_bytes=0;             \
    (pStats)->tx_errors=0;            \
    (pStats)->tx_dropped=0;           \
    (pStats)->tx_fifo_errors=0;       \
    (pStats)->tx_aborted_errors=0;    \
    (pStats)->tx_carrier_errors=0;    \
    (pStats)->tx_window_errors=0;     \
    (pStats)->tx_heartbeat_errors=0;  \
    (pStats)->collisions=0


#endif

