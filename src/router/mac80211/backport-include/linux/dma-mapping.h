#ifndef __BACKPORT_LINUX_DMA_MAPPING_H
#define __BACKPORT_LINUX_DMA_MAPPING_H
#include_next <linux/dma-mapping.h>

#ifndef DMA_MAPPING_ERROR
/*
 * A dma_addr_t can hold any valid DMA or bus address for the platform.  It can
 * be given to a device to use as a DMA source or target.  It is specific to a
 * given device and there may be a translation between the CPU physical address
 * space and the bus address space.
 *
 * DMA_MAPPING_ERROR is the magic error code if a mapping failed.  It should not
 * be used directly in drivers, but checked for using dma_mapping_error()
 * instead.
 */
#define DMA_MAPPING_ERROR		(~(dma_addr_t)0)
#endif /* DMA_MAPPING_ERROR */


#if LINUX_VERSION_IS_LESS(3,18,0)
/*
 * Set both the DMA mask and the coherent DMA mask to the same thing.
 * Note that we don't check the return value from dma_set_coherent_mask()
 * as the DMA API guarantees that the coherent DMA mask can be set to
 * the same or smaller than the streaming DMA mask.
 */
static inline int dma_set_mask_and_coherent(struct device *dev, u64 mask)
{
	int rc = dma_set_mask(dev, mask);
	if (rc == 0)
		dma_set_coherent_mask(dev, mask);
	return rc;
}
#endif
#endif /* __BACKPORT_LINUX_DMA_MAPPING_H */
