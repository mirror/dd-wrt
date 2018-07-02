/*******************************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Security.L.1.0.3-98

  Contact Information:

  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226 

*******************************************************************************/


/* glue for the OS independent part of iegbe
 * includes register access macros
 */

#ifndef _IEGBE_OSDEP_H_
#define _IEGBE_OSDEP_H_

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include "kcompat.h"

#define usec_delay(x) udelay(x)
#ifndef msec_delay
#define msec_delay(x)	do { if(in_interrupt()) { \
				/* Don't mdelay in interrupt context! */ \
	                	BUG(); \
			} else { \
				msleep(x); \
			} } while(0)

/* Some workarounds require millisecond delays and are run during interrupt
 * context.  Most notably, when establishing link, the phy may need tweaking
 * but cannot process phy register reads/writes faster than millisecond
 * intervals...and we establish link due to a "link status change" interrupt.
 */
#define msec_delay_irq(x) mdelay(x)
#endif

#define PCI_COMMAND_REGISTER   PCI_COMMAND
#define CMD_MEM_WRT_INVALIDATE PCI_COMMAND_INVALIDATE
typedef enum {
#undef FALSE
    FALSE = 0,
#undef TRUE
    TRUE = 1
} boolean_t;

#define MSGOUT(S, A, B)	printk(KERN_DEBUG S "\n", A, B)
#ifdef DBG
#define DEBUGOUT(S)		printk(KERN_DEBUG S "\n")
#define DEBUGOUT1(S, A...)	printk(KERN_DEBUG S "\n", A)
#else
#define DEBUGOUT(S)
#define DEBUGOUT1(S, A...)
#endif

#define DEBUGFUNC(F) DEBUGOUT(F)
#define DEBUGFUNC1(F, B...) DEBUGOUT1(F, B)  

#define DEBUGOUT2 DEBUGOUT1
#define DEBUGOUT3 DEBUGOUT2
#define DEBUGOUT7 DEBUGOUT3


#define E1000_WRITE_REG(a, reg, value) ( \
    writel((value), ((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg))))

#define E1000_READ_REG(a, reg) ( \
    readl((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg)))

#define E1000_WRITE_REG_ARRAY(a, reg, offset, value) ( \
    writel((value), ((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg) + \
        ((offset) << 2))))

#define E1000_READ_REG_ARRAY(a, reg, offset) ( \
    readl((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg) + \
        ((offset) << 2)))

#define E1000_READ_REG_ARRAY_DWORD E1000_READ_REG_ARRAY
#define E1000_WRITE_REG_ARRAY_DWORD E1000_WRITE_REG_ARRAY

#define E1000_WRITE_REG_ARRAY_WORD(a, reg, offset, value) ( \
    writew((value), ((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg) + \
        ((offset) << 1))))

#define E1000_READ_REG_ARRAY_WORD(a, reg, offset) ( \
    readw((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg) + \
        ((offset) << 1)))

#define E1000_WRITE_REG_ARRAY_BYTE(a, reg, offset, value) ( \
    writeb((value), ((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg) + \
        (offset))))

#define E1000_READ_REG_ARRAY_BYTE(a, reg, offset) ( \
    readb((a)->hw_addr + \
        (((a)->mac_type >= iegbe_82543) ? E1000_##reg : E1000_82542_##reg) + \
        (offset)))

#define E1000_WRITE_FLUSH(a) E1000_READ_REG(a, STATUS)

#endif /* _IEGBE_OSDEP_H_ */
 
