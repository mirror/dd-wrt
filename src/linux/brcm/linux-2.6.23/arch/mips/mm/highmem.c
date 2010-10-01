#include <linux/module.h>
#include <linux/highmem.h>
#include <asm/tlbflush.h>

void *__kmap(struct page *page)
{
	void *addr;

	might_sleep();
	if (!PageHighMem(page))
		return page_address(page);
	addr = kmap_high(page);
	flush_tlb_one((unsigned long)addr);

	return addr;
}

void __kunmap(struct page *page)
{
	if (in_interrupt())
		BUG();
	if (!PageHighMem(page))
		return;
	kunmap_high(page);
}

/*
 * kmap_atomic/kunmap_atomic is significantly faster than kmap/kunmap because
 * no global lock is needed and because the kmap code must perform a global TLB
 * invalidation when the kmap pool wraps.
 *
 * However when holding an atomic kmap is is not legal to sleep, so atomic
 * kmaps are appropriate for short, tight code paths only.
 */

/*
 * need an array per cpu, and each array has to be cache aligned
 */
struct kmap_map {
	struct page *page;
	void        *vaddr;
};

struct {
	struct kmap_map map[KM_TYPE_NR];
} ____cacheline_aligned_in_smp kmap_atomic_maps[NR_CPUS];



void *
kmap_atomic_page_address(struct page *page)
{
	int i;

	for (i = 0; i < KM_TYPE_NR; i++)
		if (kmap_atomic_maps[smp_processor_id()].map[i].page == page)
			return(kmap_atomic_maps[smp_processor_id()].map[i].vaddr);

	return((struct page *)0);
}


void *__kmap_atomic(struct page *page, enum km_type type)
{
	enum fixed_addresses idx;
	unsigned long vaddr;

	/* even !CONFIG_PREEMPT needs this, for in_atomic in do_page_fault */
	pagefault_disable();
	if (!PageHighMem(page))
		return page_address(page);

	idx = type + KM_TYPE_NR*smp_processor_id();
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
#ifdef CONFIG_DEBUG_HIGHMEM
	if (!pte_none(*(kmap_pte-idx)))
		BUG();
#endif
	set_pte(kmap_pte-idx, mk_pte(page, kmap_prot));
	local_flush_tlb_one((unsigned long)vaddr);

	kmap_atomic_maps[smp_processor_id()].map[type].page = page;
	kmap_atomic_maps[smp_processor_id()].map[type].vaddr = (void *)vaddr;

	return (void*) vaddr;
}

void __kunmap_atomic(void *kvaddr, enum km_type type)
{
	unsigned long vaddr = (unsigned long) kvaddr & PAGE_MASK;
	enum fixed_addresses idx = type + KM_TYPE_NR*smp_processor_id();

	if (vaddr < FIXADDR_START) { // FIXME
		pagefault_enable();
		return;
	}

	if (vaddr != __fix_to_virt(FIX_KMAP_BEGIN+idx))
		BUG();

	/* Protect against multiple unmaps
	 * Can't cache flush an unmapped page.
	 */
	if ( kmap_atomic_maps[smp_processor_id()].map[type].vaddr ) {
		kmap_atomic_maps[smp_processor_id()].map[type].page = (struct page *)0;
		kmap_atomic_maps[smp_processor_id()].map[type].vaddr = (void *) 0;

		flush_data_cache_page((unsigned long)vaddr);
	}

#ifdef CONFIG_DEBUG_HIGHMEM
	/*
	 * force other mappings to Oops if they'll try to access
	 * this pte without first remap it
	 */
	pte_clear(&init_mm, vaddr, kmap_pte-idx);
	local_flush_tlb_one(vaddr);
#endif

	pagefault_enable();
}

/*
 * This is the same as kmap_atomic() but can map memory that doesn't
 * have a struct page associated with it.
 */
void *kmap_atomic_pfn(unsigned long pfn, enum km_type type)
{
	enum fixed_addresses idx;
	unsigned long vaddr;

	pagefault_disable();

	idx = type + KM_TYPE_NR*smp_processor_id();
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
	set_pte(kmap_pte-idx, pfn_pte(pfn, kmap_prot));
	flush_tlb_one(vaddr);

	return (void*) vaddr;
}

struct page *__kmap_atomic_to_page(void *ptr)
{
	unsigned long idx, vaddr = (unsigned long)ptr;
	pte_t *pte;

	if (vaddr < FIXADDR_START)
		return virt_to_page(ptr);

	idx = virt_to_fix(vaddr);
	pte = kmap_pte - (idx - FIX_KMAP_BEGIN);
	return pte_page(*pte);
}

EXPORT_SYMBOL(__kmap);
EXPORT_SYMBOL(__kunmap);
EXPORT_SYMBOL(__kmap_atomic);
EXPORT_SYMBOL(__kunmap_atomic);
EXPORT_SYMBOL(__kmap_atomic_to_page);
