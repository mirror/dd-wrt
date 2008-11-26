/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/
#ifndef _MV_OS_LNX_H_
#define _MV_OS_LNX_H_
                                                                                                                                               
                                                                                                                                               
#ifdef __KERNEL__
/* for kernel space */
#include <linux/autoconf.h>
#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/reboot.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/mm.h>
  
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/hardirq.h>
#include <asm/dma.h>
#include <asm/io.h>
 
#define INLINE inline
#define mvOsPrintf          printk
#define mvOsOutput          printk
#define mvOsSPrintf         sprintf
#define mvOsMalloc(_size_)  kmalloc(_size_,GFP_ATOMIC)
#define mvOsFree            kfree
#define mvOsSleep(_mils_)   mdelay(_mils_)
#define mvOsTaskLock()
#define mvOsTaskUnlock()
#define strtol              simple_strtoul
#define mvOsDelay(x)        mdelay(x)
#define mvOsUDelay(x)       udelay(x)
#define mvCopyFromOs        copy_from_user
#define mvCopyToOs          copy_to_user

 
#include "mvTypes.h"
#include "mvCommon.h"
  
#ifdef MV_NDEBUG
#define mvOsAssert(cond)
#else
#define mvOsAssert(cond) { do { if(!(cond)) { BUG(); } }while(0); }
#endif /* MV_NDEBUG */
 
#else /* __KERNEL__ */
 
/* for user space applications */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
 
#define INLINE inline
#define mvOsPrintf printf
#define mvOsOutput printf
#define mvOsMalloc(_size_) malloc(_size_)
#define mvOsFree free
#define mvOsAssert(cond) assert(cond)
 
#endif /* __KERNEL__ */                                                                                                                                               
#if defined (MV_ARM)
#   include "mvLinuxArm.h"
#else
#   error "CPU type not selected"
#endif


#endif /* _MV_OS_LNX_H_ */


