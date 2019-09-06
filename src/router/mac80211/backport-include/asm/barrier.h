#ifndef __BACKPORT_ASM_BARRIER_H
#define __BACKPORT_ASM_BARRIER_H

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0) || \
    defined(CONFIG_ALPHA) || defined(CONFIG_MIPS)
#include_next <asm/barrier.h>
#endif /* >= 3.4 */

#ifndef dma_rmb
#define dma_rmb()	rmb()
#endif

#ifndef smp_mb__after_atomic
#define smp_mb__after_atomic smp_mb__after_clear_bit
#endif

#endif /* __BACKPORT_ASM_BARRIER_H */
