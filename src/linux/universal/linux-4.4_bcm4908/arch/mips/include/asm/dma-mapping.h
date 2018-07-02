#ifndef _ASM_DMA_MAPPING_H
#define _ASM_DMA_MAPPING_H

#include <linux/kmemcheck.h>
#include <linux/bug.h>
#include <linux/scatterlist.h>
#include <linux/dma-debug.h>
#include <linux/dma-attrs.h>

#include <asm/dma-coherence.h>
#include <asm/cache.h>
#include <asm/cpu-type.h>
#include <asm-generic/dma-coherent.h>

#ifndef CONFIG_SGI_IP27 /* Kludge to fix 2.6.39 build for IP27 */
#include <dma-coherence.h>
#endif

extern struct dma_map_ops *mips_dma_map_ops;

void __dma_sync(struct page *page, unsigned long offset, size_t size,
		enum dma_data_direction direction);
void *mips_dma_alloc_coherent(struct device *dev, size_t size,
			      dma_addr_t *dma_handle, gfp_t gfp,
			      struct dma_attrs *attrs);
void mips_dma_free_coherent(struct device *dev, size_t size, void *vaddr,
			    dma_addr_t dma_handle, struct dma_attrs *attrs);

static inline struct dma_map_ops *get_dma_ops(struct device *dev)
{
#ifdef CONFIG_SYS_HAS_DMA_OPS
	if (dev && dev->archdata.dma_ops)
		return dev->archdata.dma_ops;
	else
		return mips_dma_map_ops;
#else
	return NULL;
#endif
}

/*
 * The affected CPUs below in 'cpu_needs_post_dma_flush()' can
 * speculatively fill random cachelines with stale data at any time,
 * requiring an extra flush post-DMA.
 *
 * Warning on the terminology - Linux calls an uncached area coherent;
 * MIPS terminology calls memory areas with hardware maintained coherency
 * coherent.
 *
 * Note that the R14000 and R16000 should also be checked for in this
 * condition.  However this function is only called on non-I/O-coherent
 * systems and only the R10000 and R12000 are used in such systems, the
 * SGI IP28 Indigo² rsp. SGI IP32 aka O2.
 */
static inline int cpu_needs_post_dma_flush(struct device *dev)
{
	return !plat_device_is_coherent(dev) &&
	       (boot_cpu_type() == CPU_R10000 ||
		boot_cpu_type() == CPU_R12000 ||
		boot_cpu_type() == CPU_BMIPS5000);
}

static inline struct page *dma_addr_to_page(struct device *dev,
	dma_addr_t dma_addr)
{
	return pfn_to_page(
		plat_dma_addr_to_phys(dev, dma_addr) >> PAGE_SHIFT);
}

static inline bool dma_capable(struct device *dev, dma_addr_t addr, size_t size)
{
	if (!dev->dma_mask)
		return false;

	return addr + size <= *dev->dma_mask;
}

static inline void dma_mark_clean(void *addr, size_t size) {}

static inline dma_addr_t dma_map_single_attrs(struct device *dev, void *ptr,
					      size_t size,
					      enum dma_data_direction dir,
					      struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	unsigned long offset = (unsigned long)ptr & ~PAGE_MASK;
	struct page *page = virt_to_page(ptr);
	dma_addr_t addr;

	kmemcheck_mark_initialized(ptr, size);
	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		addr = ops->map_page(dev, page, offset, size, dir, attrs);
	} else {
		if (!plat_device_is_coherent(dev))
			__dma_sync(page, offset, size, dir);

		addr = plat_map_dma_mem_page(dev, page) + offset;
	}
	debug_dma_map_page(dev, page, offset, size, dir, addr, true);
	return addr;
}

static inline void dma_unmap_single_attrs(struct device *dev, dma_addr_t addr,
					  size_t size,
					  enum dma_data_direction dir,
					  struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ops->unmap_page(dev, addr, size, dir, attrs);
	} else {
		if (cpu_needs_post_dma_flush(dev))
			__dma_sync(dma_addr_to_page(dev, addr),
				   addr & ~PAGE_MASK, size, dir);
		plat_post_dma_flush(dev);
		plat_unmap_dma_mem(dev, addr, size, dir);
	}
	debug_dma_unmap_page(dev, addr, size, dir, true);
}

/*
 * dma_maps_sg_attrs returns 0 on error and > 0 on success.
 * It should never return a value < 0.
 */
static inline int dma_map_sg_attrs(struct device *dev, struct scatterlist *sg,
				   int nents, enum dma_data_direction dir,
				   struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	int i, ents;
	struct scatterlist *s;

	for_each_sg(sg, s, nents, i)
		kmemcheck_mark_initialized(sg_virt(s), s->length);
	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ents = ops->map_sg(dev, sg, nents, dir, attrs);
	} else {
		for_each_sg(sg, s, nents, i) {
			struct page *page = sg_page(s);

			if (!plat_device_is_coherent(dev))
				__dma_sync(page, s->offset, s->length, dir);
#ifdef CONFIG_NEED_SG_DMA_LENGTH
			s->dma_length = s->length;
#endif
			s->dma_address =
				plat_map_dma_mem_page(dev, page) + s->offset;
		}
		ents = nents;
	}
	BUG_ON(ents < 0);
	debug_dma_map_sg(dev, sg, nents, ents, dir);

	return ents;
}

static inline void dma_unmap_sg_attrs(struct device *dev, struct scatterlist *sg,
				      int nents, enum dma_data_direction dir,
				      struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	struct scatterlist *s;
	int i;

	BUG_ON(!valid_dma_direction(dir));
	debug_dma_unmap_sg(dev, sg, nents, dir);
	if (ops) {
		ops->unmap_sg(dev, sg, nents, dir, attrs);
		return;
	}
	for_each_sg(sg, s, nents, i) {
		if (!plat_device_is_coherent(dev) && dir != DMA_TO_DEVICE)
			__dma_sync(sg_page(s), s->offset, s->length, dir);
		plat_unmap_dma_mem(dev, s->dma_address, s->length, dir);
	}
}

static inline dma_addr_t dma_map_page(struct device *dev, struct page *page,
				      size_t offset, size_t size,
				      enum dma_data_direction dir)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	dma_addr_t addr;

	kmemcheck_mark_initialized(page_address(page) + offset, size);
	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		addr = ops->map_page(dev, page, offset, size, dir, NULL);
	} else {
		if (!plat_device_is_coherent(dev))
			__dma_sync(page, offset, size, dir);

		addr = plat_map_dma_mem_page(dev, page) + offset;
	}
	debug_dma_map_page(dev, page, offset, size, dir, addr, false);

	return addr;
}

static inline void dma_unmap_page(struct device *dev, dma_addr_t addr,
				  size_t size, enum dma_data_direction dir)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ops->unmap_page(dev, addr, size, dir, NULL);
	} else {
		if (cpu_needs_post_dma_flush(dev))
			__dma_sync(dma_addr_to_page(dev, addr),
				   addr & ~PAGE_MASK, size, dir);
		plat_post_dma_flush(dev);
		plat_unmap_dma_mem(dev, addr, size, dir);
	}
	debug_dma_unmap_page(dev, addr, size, dir, false);
}

static inline void dma_sync_single_for_cpu(struct device *dev, dma_addr_t addr,
					   size_t size,
					   enum dma_data_direction dir)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ops->sync_single_for_cpu(dev, addr, size, dir);
	} else {
		if (cpu_needs_post_dma_flush(dev))
			__dma_sync(dma_addr_to_page(dev, addr),
				   addr & ~PAGE_MASK, size, dir);
		plat_post_dma_flush(dev);
	}
	debug_dma_sync_single_for_cpu(dev, addr, size, dir);
}

static inline void dma_sync_single_for_device(struct device *dev,
					      dma_addr_t addr, size_t size,
					      enum dma_data_direction dir)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	BUG_ON(!valid_dma_direction(dir));
	if (ops)
		ops->sync_single_for_device(dev, addr, size, dir);
	else if (!plat_device_is_coherent(dev))
		__dma_sync(dma_addr_to_page(dev, addr),
			   addr & ~PAGE_MASK, size, dir);
	debug_dma_sync_single_for_device(dev, addr, size, dir);
}

static inline void dma_sync_single_range_for_cpu(struct device *dev,
						 dma_addr_t addr,
						 unsigned long offset,
						 size_t size,
						 enum dma_data_direction dir)
{
	const struct dma_map_ops *ops = get_dma_ops(dev);

	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ops->sync_single_for_cpu(dev, addr + offset, size, dir);
	} else {
		if (cpu_needs_post_dma_flush(dev))
			__dma_sync(dma_addr_to_page(dev, addr + offset),
				   (addr + offset) & ~PAGE_MASK, size, dir);
		plat_post_dma_flush(dev);
	}

	debug_dma_sync_single_range_for_cpu(dev, addr, offset, size, dir);
}

static inline void dma_sync_single_range_for_device(struct device *dev,
						    dma_addr_t addr,
						    unsigned long offset,
						    size_t size,
						    enum dma_data_direction dir)
{
	const struct dma_map_ops *ops = get_dma_ops(dev);

	BUG_ON(!valid_dma_direction(dir));
	if (ops)
		ops->sync_single_for_device(dev, addr + offset, size, dir);
	else if (!plat_device_is_coherent(dev))
		__dma_sync(dma_addr_to_page(dev, addr + offset),
			   (addr + offset) & ~PAGE_MASK, size, dir);
	debug_dma_sync_single_range_for_device(dev, addr, offset, size, dir);
}

static inline void
dma_sync_sg_for_cpu(struct device *dev, struct scatterlist *sg,
		    int nelems, enum dma_data_direction dir)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	struct scatterlist *s;
	int i;

	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ops->sync_sg_for_cpu(dev, sg, nelems, dir);
	} else if (cpu_needs_post_dma_flush(dev)) {
		for_each_sg(sg, s, nelems, i)
			__dma_sync(sg_page(s), s->offset, s->length, dir);
	}
	plat_post_dma_flush(dev);
	debug_dma_sync_sg_for_cpu(dev, sg, nelems, dir);
}

static inline void
dma_sync_sg_for_device(struct device *dev, struct scatterlist *sg,
		       int nelems, enum dma_data_direction dir)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	struct scatterlist *s;
	int i;

	BUG_ON(!valid_dma_direction(dir));
	if (ops) {
		ops->sync_sg_for_device(dev, sg, nelems, dir);
	} else if (!plat_device_is_coherent(dev)) {
		for_each_sg(sg, s, nelems, i)
			__dma_sync(sg_page(s), s->offset, s->length, dir);
	}
	debug_dma_sync_sg_for_device(dev, sg, nelems, dir);

}

#define dma_map_single(d, a, s, r) dma_map_single_attrs(d, a, s, r, NULL)
#define dma_unmap_single(d, a, s, r) dma_unmap_single_attrs(d, a, s, r, NULL)
#define dma_map_sg(d, s, n, r) dma_map_sg_attrs(d, s, n, r, NULL)
#define dma_unmap_sg(d, s, n, r) dma_unmap_sg_attrs(d, s, n, r, NULL)

extern int dma_common_mmap(struct device *dev, struct vm_area_struct *vma,
			   void *cpu_addr, dma_addr_t dma_addr, size_t size);

/**
 * dma_mmap_attrs - map a coherent DMA allocation into user space
 * @dev: valid struct device pointer, or NULL for ISA and EISA-like devices
 * @vma: vm_area_struct describing requested user mapping
 * @cpu_addr: kernel CPU-view address returned from dma_alloc_attrs
 * @handle: device-view address returned from dma_alloc_attrs
 * @size: size of memory originally requested in dma_alloc_attrs
 * @attrs: attributes of mapping properties requested in dma_alloc_attrs
 *
 * Map a coherent DMA buffer previously allocated by dma_alloc_attrs
 * into user space.  The coherent DMA buffer must not be freed by the
 * driver until the user space mapping has been released.
 */
static inline int
dma_mmap_attrs(struct device *dev, struct vm_area_struct *vma, void *cpu_addr,
	       dma_addr_t dma_addr, size_t size, struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	BUG_ON(!ops);
	if (ops && ops->mmap)
		return ops->mmap(dev, vma, cpu_addr, dma_addr, size, attrs);
	return dma_common_mmap(dev, vma, cpu_addr, dma_addr, size);
}

#define dma_mmap_coherent(d, v, c, h, s) dma_mmap_attrs(d, v, c, h, s, NULL)

int
dma_common_get_sgtable(struct device *dev, struct sg_table *sgt,
		       void *cpu_addr, dma_addr_t dma_addr, size_t size);

static inline int
dma_get_sgtable_attrs(struct device *dev, struct sg_table *sgt, void *cpu_addr,
		      dma_addr_t dma_addr, size_t size, struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	BUG_ON(!ops);
	if (ops && ops->get_sgtable)
		return ops->get_sgtable(dev, sgt, cpu_addr, dma_addr, size,
					attrs);
	return dma_common_get_sgtable(dev, sgt, cpu_addr, dma_addr, size);
}

#define dma_get_sgtable(d, t, v, h, s) dma_get_sgtable_attrs(d, t, v, h, s, NULL)

static inline int dma_supported(struct device *dev, u64 mask)
{
	struct dma_map_ops *ops = get_dma_ops(dev);
	if (ops)
		return ops->dma_supported(dev, mask);
	return plat_dma_supported(dev, mask);
}

static inline int dma_mapping_error(struct device *dev, u64 mask)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	debug_dma_mapping_error(dev, mask);
	if (ops)
		return ops->mapping_error(dev, mask);
	return 0;
}

static inline int
dma_set_mask(struct device *dev, u64 mask)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	if(!dev->dma_mask || !dma_supported(dev, mask))
		return -EIO;

	if (ops && ops->set_dma_mask)
		return ops->set_dma_mask(dev, mask);

	*dev->dma_mask = mask;

	return 0;
}

extern void dma_cache_sync(struct device *dev, void *vaddr, size_t size,
	       enum dma_data_direction direction);

#define dma_alloc_coherent(d,s,h,f)	dma_alloc_attrs(d,s,h,f,NULL)

static inline void *dma_alloc_attrs(struct device *dev, size_t size,
				    dma_addr_t *dma_handle, gfp_t gfp,
				    struct dma_attrs *attrs)
{
	void *ret;
	struct dma_map_ops *ops = get_dma_ops(dev);

	if (ops)
		ret = ops->alloc(dev, size, dma_handle, gfp, attrs);
	else
		ret = mips_dma_alloc_coherent(dev, size, dma_handle, gfp,
					      attrs);

	debug_dma_alloc_coherent(dev, size, *dma_handle, ret);

	return ret;
}

#define dma_free_coherent(d,s,c,h) dma_free_attrs(d,s,c,h,NULL)

static inline void dma_free_attrs(struct device *dev, size_t size,
				  void *vaddr, dma_addr_t dma_handle,
				  struct dma_attrs *attrs)
{
	struct dma_map_ops *ops = get_dma_ops(dev);

	if (ops)
		ops->free(dev, size, vaddr, dma_handle, attrs);
	else
		mips_dma_free_coherent(dev, size, vaddr, dma_handle, attrs);

	debug_dma_free_coherent(dev, size, vaddr, dma_handle);
}

static inline void *dma_alloc_noncoherent(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp)
{
	DEFINE_DMA_ATTRS(attrs);

	dma_set_attr(DMA_ATTR_NON_CONSISTENT, &attrs);
	return dma_alloc_attrs(dev, size, dma_handle, gfp, &attrs);
}

static inline void dma_free_noncoherent(struct device *dev, size_t size,
		void *cpu_addr, dma_addr_t dma_handle)
{
	DEFINE_DMA_ATTRS(attrs);

	dma_set_attr(DMA_ATTR_NON_CONSISTENT, &attrs);
	dma_free_attrs(dev, size, cpu_addr, dma_handle, &attrs);
}


#endif /* _ASM_DMA_MAPPING_H */
