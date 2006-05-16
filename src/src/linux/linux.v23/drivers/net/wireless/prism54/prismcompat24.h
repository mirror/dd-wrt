/*  
 *  (C) 2004 Margit Schubert-While <margitsw@t-online.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*  
 *	Compatibility header file to aid support of different kernel versions
 */

#ifndef _PRISM_COMPAT_H
#define _PRISM_COMPAT_H

#include <linux/firmware.h>
#include <linux/config.h>
#include <linux/tqueue.h>
#include <linux/version.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <asm/uaccess.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,25)
#define module_param(x, y, z)	MODULE_PARM(x, "i")
#else
#include <linux/moduleparam.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23)
#define free_netdev(x)		kfree(x)
#define pci_name(x)		x->slot_name
#define irqreturn_t		void
#define IRQ_HANDLED
#define IRQ_NONE
#else
#include <linux/interrupt.h>
#endif

#define work_struct		tq_struct
#define INIT_WORK		INIT_TQUEUE
#define schedule_work		schedule_task

#ifndef __iomem
#define __iomem
#endif

#if !defined(HAVE_NETDEV_PRIV)
#define netdev_priv(x)		(x)->priv
#endif

#if !defined(CONFIG_FW_LOADER) && !defined(CONFIG_FW_LOADER_MODULE)
#error Firmware Loading is not configured in the kernel !
#endif

#define prism54_synchronize_irq(irq) synchronize_irq()

#define DEFINE_WAIT(y)			DECLARE_WAITQUEUE(y, current)
#define prepare_to_wait(x, y, z)	set_current_state(z); \
					add_wait_queue(x, y)
#define finish_wait(x, y)		remove_wait_queue(x, y); \
					set_current_state(TASK_RUNNING)

#define PRISM_FW_PDEV		pci_name(priv->pdev)

#endif				/* _PRISM_COMPAT_H */
