/*
 * Hibernation support specific for ARM
 *
 * Copyright (C) 2010 Nokia Corporation
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2006 Rafael J. Wysocki <rjw@xxxxxxx>
 *
 * Contact: Hiroshi DOYU <Hiroshi.DOYU@xxxxxxxxx>
 *
 * License terms: GNU General Public License (GPL) version 2
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <asm/memory.h>

/*
 * Image of the saved processor state
 *
 * coprocessor 15 registers(RW)
 */
struct saved_context {
	/* CR1 */
	u32 cr;		/* Control */
	u32 acr;	/* Auxiliary Control*/
	u32 cacr;	/* Coprocessor Access Control */
	/* CR2 */
	u32 ttb_0r;	/* Translation Table Base 0 */
	u32 ttb_1r;	/* Translation Table Base 1 */
	u32 ttbcr;	/* Translation Talbe Base Control */
	/* CR3 */
	u32 dacr;	/* Domain Access Control */
	/* CR5 */
	u32 d_fsr;	/* Data Fault Status */
	u32 i_fsr;	/* Instruction Fault Status */
	/* CR6 */
	u32 far;	/* Fault Address */
	u32 w_far;	/* Watchpoint Fault Address */
	/* CR7 */
	u32 par;	/* Physical Address */
	/* CR10 */
	u32 tlblr;	/* TLB Lockdown Register */
	u32 prrr;	/* Primary Region Remap Register */
	u32 nrrr;	/* Normal Memory Remap Register */
	/* CR13 */
	u32 pid;	/* Precess ID */
	u32 cid;	/* Context ID */
	u32 urwtid;	/* User read/write Thread ID */
	u32 urotid;	/* User read-only Thread ID */
	u32 potid;	/* Privileged only Thread ID */
	/* CR15 */
	u32 tlb_entry; /* Main TLB Entry */
	u32 tlb_va; /* Main TLB VA */
	u32 tlb_pa; /* Main TLB PA */
	u32 tlb_attr; /* Main TLB Attribute */
} __attribute__((packed));

/* Used in hibernate_asm.S */
#define USER_CONTEXT_SIZE (sizeof(u32) * 15)
unsigned long saved_context_r0[USER_CONTEXT_SIZE];
unsigned long saved_cpsr;
unsigned long saved_context_r13_svc;
unsigned long saved_context_r14_svc;
unsigned long saved_spsr_svc;

static struct saved_context saved_context;

/* References to section boundaries */
extern const void __nosave_begin, __nosave_end;
/*
 * pfn_is_nosave - check if given pfn is in the 'nosave' section
 */
int pfn_is_nosave(unsigned long pfn)
{
	unsigned long nosave_begin_pfn = __pa_symbol(&__nosave_begin) >> PAGE_SHIFT;
	unsigned long nosave_end_pfn = PAGE_ALIGN(__pa_symbol(&__nosave_end)) >> PAGE_SHIFT;

	return (pfn >= nosave_begin_pfn) && (pfn < nosave_end_pfn);
}

static inline void __save_processor_state(struct saved_context *ctxt)
{
	/* CR1 */
	asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(ctxt->cr));
	asm volatile ("mrc p15, 0, %0, c1, c0, 2" : "=r"(ctxt->cacr));
	/* CR2 */
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r"(ctxt->ttb_0r));
	asm volatile ("mrc p15, 0, %0, c2, c0, 1" : "=r"(ctxt->ttb_1r));
	asm volatile ("mrc p15, 0, %0, c2, c0, 2" : "=r"(ctxt->ttbcr));
	/* CR3 */
	asm volatile ("mrc p15, 0, %0, c3, c0, 0" : "=r"(ctxt->dacr));
	/* CR5 */
	asm volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r"(ctxt->d_fsr));
	asm volatile ("mrc p15, 0, %0, c5, c0, 1" : "=r"(ctxt->i_fsr));
	/* CR6 */
	asm volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r"(ctxt->far));
	asm volatile ("mrc p15, 0, %0, c6, c0, 1" : "=r"(ctxt->w_far));
#if 0
	/* CR7 */
	asm volatile ("mrc p15, 0, %0, c7, c4, 0" : "=r"(ctxt->par));
#endif
	/* CR10 */
	asm volatile ("mrc p15, 0, %0, c10, c0, 0" : "=r"(ctxt->tlblr));
	asm volatile ("mrc p15, 0, %0, c10, c2, 0" : "=r"(ctxt->prrr));
	asm volatile ("mrc p15, 0, %0, c10, c2, 1" : "=r"(ctxt->nrrr));
	/* CR13 */
	asm volatile ("mrc p15, 0, %0, c13, c0, 0" : "=r"(ctxt->pid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 1" : "=r"(ctxt->cid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 2" : "=r"(ctxt->urwtid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 3" : "=r"(ctxt->urotid));
	asm volatile ("mrc p15, 0, %0, c13, c0, 4" : "=r"(ctxt->potid));

	/* CR15 */	
	asm volatile ("mrc p15, 0, %0, c15, c4, 2" : "=r"(ctxt->tlb_entry));
	asm volatile ("mrc p15, 0, %0, c15, c5, 2" : "=r"(ctxt->tlb_va));
	asm volatile ("mrc p15, 0, %0, c15, c6, 2" : "=r"(ctxt->tlb_pa));
	asm volatile ("mrc p15, 0, %0, c15, c7, 2" : "=r"(ctxt->tlb_attr));
}

static inline void __restore_processor_state(struct saved_context *ctxt)
{
	/* CR1 */
	asm volatile ("mcr p15, 0, %0, c1, c0, 0" : : "r"(ctxt->cr));
	asm volatile ("mcr p15, 0, %0, c1, c0, 2" : : "r"(ctxt->cacr));
	/* CR2 */
	asm volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r"(ctxt->ttb_0r));
	asm volatile ("mcr p15, 0, %0, c2, c0, 1" : : "r"(ctxt->ttb_1r));
	asm volatile ("mcr p15, 0, %0, c2, c0, 2" : : "r"(ctxt->ttbcr));
	/* CR3 */
	asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(ctxt->dacr));
	/* CR5 */
	asm volatile ("mcr p15, 0, %0, c5, c0, 0" : : "r"(ctxt->d_fsr));
	asm volatile ("mcr p15, 0, %0, c5, c0, 1" : : "r"(ctxt->i_fsr));
	/* CR6 */
	asm volatile ("mcr p15, 0, %0, c6, c0, 0" : : "r"(ctxt->far));
	asm volatile ("mcr p15, 0, %0, c6, c0, 1" : : "r"(ctxt->w_far));
#if 0
	/* CR7 */
	asm volatile ("mcr p15, 0, %0, c7, c4, 0" : : "r"(ctxt->par));
#endif
	/* CR10 */
	asm volatile ("mcr p15, 0, %0, c10, c0, 0" : : "r"(ctxt->tlblr));
	asm volatile ("mcr p15, 0, %0, c10, c2, 0" : : "r"(ctxt->prrr));
	asm volatile ("mcr p15, 0, %0, c10, c2, 1" : : "r"(ctxt->nrrr));
	/* CR13 */
	asm volatile ("mcr p15, 0, %0, c13, c0, 0" : : "r"(ctxt->pid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 1" : : "r"(ctxt->cid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 2" : : "r"(ctxt->urwtid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 3" : : "r"(ctxt->urotid));
	asm volatile ("mcr p15, 0, %0, c13, c0, 4" : : "r"(ctxt->potid));
	/* CR15 */	
	asm volatile ("mcr p15, 0, %0, c15, c4, 2" : "=r"(ctxt->tlb_entry));
	asm volatile ("mcr p15, 0, %0, c15, c5, 2" : "=r"(ctxt->tlb_va));
	asm volatile ("mcr p15, 0, %0, c15, c6, 2" : "=r"(ctxt->tlb_pa));
	asm volatile ("mcr p15, 0, %0, c15, c7, 2" : "=r"(ctxt->tlb_attr));
}

void save_processor_state(void)
{
	preempt_disable();
	__save_processor_state(&saved_context);
}

void restore_processor_state(void)
{
	__restore_processor_state(&saved_context);
	preempt_enable();
}
