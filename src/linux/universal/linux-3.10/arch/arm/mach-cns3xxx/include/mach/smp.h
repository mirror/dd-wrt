#ifndef __MACH_SMP_H
#define __MACH_SMP_H

#include <linux/irqchip/arm-gic.h>

extern void smp_dma_map_area(const void *, size_t, int);
extern void smp_dma_unmap_area(const void *, size_t, int);
extern void smp_dma_flush_range(const void *, const void *);

#endif
