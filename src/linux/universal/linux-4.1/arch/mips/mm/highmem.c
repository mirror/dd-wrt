#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <asm/fixmap.h>
#include <asm/tlbflush.h>

static pte_t *kmap_pte;

unsigned long highstart_pfn, highend_pfn;
unsigned int  last_pkmap_nr_arr[FIX_N_COLOURS] = { 0, 1, 2, 3, 4, 5, 6, 7 };

void *kmap(struct page *page)
{
	void *addr;

	might_sleep();
	if (!PageHighMem(page))
		return page_address(page);
	addr = kmap_high(page);
	flush_tlb_one((unsigned long)addr);

	return addr;
}
EXPORT_SYMBOL(kmap);

void kunmap(struct page *page)
{
	BUG_ON(in_interrupt());
	if (!PageHighMem(page))
		return;
	kunmap_high(page);
}
EXPORT_SYMBOL(kunmap);

/*
 * kmap_atomic/kunmap_atomic is significantly faster than kmap/kunmap because
 * no global lock is needed and because the kmap code must perform a global TLB
 * invalidation when the kmap pool wraps.
 *
 * However when holding an atomic kmap is is not legal to sleep, so atomic
 * kmaps are appropriate for short, tight code paths only.
 */

void *kmap_atomic(struct page *page)
{
	unsigned long vaddr;
	int idx, type;

	/* even !CONFIG_PREEMPT needs this, for in_atomic in do_page_fault */
	pagefault_disable();
	if (!PageHighMem(page))
		return page_address(page);

	type = kmap_atomic_idx_push();

	idx = (((unsigned long)lowmem_page_address(page)) >> PAGE_SHIFT) & (FIX_N_COLOURS - 1);
	idx = (FIX_N_COLOURS - idx);
	idx = idx + FIX_N_COLOURS * (smp_processor_id() + NR_CPUS * type);
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN - 1 + idx);    /* actually - FIX_CMAP_END */

#ifdef CONFIG_DEBUG_HIGHMEM
	BUG_ON(!pte_none(*(kmap_pte - idx)));
#endif
	set_pte(kmap_pte-idx, mk_pte(page, PAGE_KERNEL));
	local_flush_tlb_one((unsigned long)vaddr);

	return (void*) vaddr;
}
EXPORT_SYMBOL(kmap_atomic);

void __kunmap_atomic(void *kvaddr)
{
	unsigned long vaddr = (unsigned long) kvaddr & PAGE_MASK;
	int type __maybe_unused;

	if (vaddr < FIXADDR_START) { // FIXME
		pagefault_enable();
		return;
	}

#ifdef CONFIG_DEBUG_HIGHMEM
	{
		int idx;
		type = kmap_atomic_idx();

		idx = ((unsigned long)kvaddr >> PAGE_SHIFT) & (FIX_N_COLOURS - 1);
		idx = (FIX_N_COLOURS - idx);
		idx = idx + FIX_N_COLOURS * (smp_processor_id() + NR_CPUS * type);

		BUG_ON(vaddr != __fix_to_virt(FIX_KMAP_BEGIN -1 + idx));

		/*
		 * force other mappings to Oops if they'll try to access
		 * this pte without first remap it
		 */
		pte_clear(&init_mm, vaddr, kmap_pte-idx);
		local_flush_tlb_one(vaddr);
	}
#endif
	kmap_atomic_idx_pop();
	pagefault_enable();
}
EXPORT_SYMBOL(__kunmap_atomic);

struct page *kmap_atomic_to_page(void *ptr)
{
	unsigned long idx, vaddr = (unsigned long)ptr;
	pte_t *pte;

	if (vaddr < FIXADDR_START)
		return virt_to_page(ptr);

	idx = virt_to_fix(vaddr);
	pte = kmap_pte - (idx - FIX_KMAP_BEGIN + 1);
	return pte_page(*pte);
}

void __init kmap_init(void)
{
	unsigned long kmap_vstart;

	/* cache the first kmap pte */
	kmap_vstart = __fix_to_virt(FIX_KMAP_BEGIN - 1); /* actually - FIX_CMAP_END */
	kmap_pte = kmap_get_fixmap_pte(kmap_vstart);
}
