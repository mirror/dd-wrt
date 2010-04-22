/*
 * Copyright(c) 2007 Atheros Corporation. All rights reserved.
 *
 * Derived from Intel e1000 driver
 * Copyright(c) 1999 - 2005 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * There are a lot of defines in here that are unused and/or have cryptic
 * names.  Please leave them alone, as they're the closest thing we have
 * to a spec from Atheros at present. *ahem* -- CHS
 */
 
#ifndef _ATH_OS_DEP_H_
#define _ATH_OS_DEP_H_

#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>

#include "kcompat.h"

#define usec_delay(x) udelay(x)
#ifndef msec_delay
#define msec_delay(x)   do { if(in_interrupt()) { \
                                /* Don't mdelay in interrupt context! */ \
                                BUG(); \
                        } else { \
                                msleep(x); \
                        } } while (0)

/* Some workarounds require millisecond delays and are run during interrupt
 * context.  Most notably, when establishing link, the phy may need tweaking
 * but cannot process phy register reads/writes faster than millisecond
 * intervals...and we establish link due to a "link status change" interrupt.
 */
#define msec_delay_irq(x) mdelay(x)
#endif


#define PCI_COMMAND_REGISTER    PCI_COMMAND
#define CMD_MEM_WRT_INVALIDATE  PCI_COMMAND_INVALIDATE
#define ETH_ADDR_LEN            ETH_ALEN

typedef enum {
#undef FALSE
    FALSE = 0,
#undef TRUE
    TRUE = 1
} boolean_t;

#define AT_ERR(args...) printk(KERN_ERR "atheros: " args)

#if DBG
#define AT_DBG(args...) printk(KERN_DEBUG "atheros: " args)
#define DEBUGOUT(S)     printk(KERN_DEBUG S "\n")
#define DEBUGOUT1(S, A...)  printk(KERN_DEBUG S "\n", A)
#else
#define AT_DBG(args...)
#define DEBUGOUT(S)
#define DEBUGOUT1(S, A...)
#endif

#define DEBUGFUNC(F) DEBUGOUT(F)
#define DEBUGOUT2 DEBUGOUT1
#define DEBUGOUT3 DEBUGOUT2
#define DEBUGOUT7 DEBUGOUT3


#endif//_ATH_OS_DEP_H_

