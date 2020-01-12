/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994 by Waldorf Electronics
 * Copyright (C) 1995 - 1998, 2001 by Ralf Baechle
 */
#ifndef _ASM_DELAY_H
#define _ASM_DELAY_H

#include <linux/config.h>
#include <linux/param.h>

#include <asm/compiler.h>


extern void __delay(unsigned long loops);
extern void __ndelay(unsigned long ns, unsigned int lpj);
extern void __udelay(unsigned long us, unsigned int lpj);


extern unsigned long loops_per_jiffy;

#ifdef CONFIG_SMP
#define __udelay_val cpu_data[smp_processor_id()].udelay_val
#else
#define __udelay_val loops_per_jiffy
#endif

#define udelay(usecs) __udelay((usecs),__udelay_val)
#define ndelay(nsecs) __ndelay((nsecs),__udelay_val)

#endif /* _ASM_DELAY_H */
