/*
 * Initialization and support routines for self-booting
 * compressed image.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte.c,v 1.175.2.2 2008/07/16 00:41:46 Exp $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <hndcpu.h>
#ifdef SBPCI
#include <pci_core.h>
#include <hndpci.h>
#include <pcicfg.h>
#endif /* SBPCI */
#include <hndchipc.h>
#include <hndrte.h>

/* debug */
#ifdef BCMDBG
#define RTE_MSG(x) printf x
#else
#define RTE_MSG(x)
#endif

si_t *hndrte_sih = NULL;		/* Backplane handle */
osl_t *hndrte_osh = NULL;		/* Backplane osl handle */
chipcregs_t *hndrte_ccr = NULL;		/* Chipc core regs */
sbconfig_t *hndrte_ccsbr = NULL;	/* Chipc core SB config regs */

/* Allow to program the h/w timer */
static bool timer_allowed = TRUE;
/* Don't program the h/w timer again if it has been */
static bool timer_armed;

static void hndrte_chipc_init(si_t *sih);
#ifdef WLGPIOHLR
static void hndrte_gpio_init(si_t *sih);
#endif

#ifdef BCMECICOEX
static void hndrte_eci_init(si_t *sih);
#endif /* BCMECICOEX */

/*
 * hndrte.
 *
 * Memory allocation:
 *	hndrte_free(w): Free previously allocated memory at w
 *	hndrte_malloc_align(size, abits): Allocate memory at a 2^abits boundary
 *	hndrte_memavail(): Return a (slightly optimistic) estimate of free memory
 *	hndrte_hwm(): Return a high watermark of allocated memory
 *	hndrte_print_memuse(): Dump memory usage stats.
 *	hndrte_print_memwaste(): Malloc memory to simulate low memory environment
 *	hndrte_print_malloc(): (BCMDBG) Dump free & inuse lists
 *	hndrte_arena_add(base, size): Add a block of memory to the arena
 */

#define	MIN_MEM_SIZE	8	/* Min. memory size is 8 bytes */
#define	MIN_ALIGN	4	/* Alignment at 4 bytes */
#define	MAX_ALIGN	4096	/* Max alignment at 4k */
#define	ALIGN(ad, al)	(((ad) + ((al) - 1)) & ~((al) - 1))
#define	ALIGN_DOWN(ad, al)	((ad) & ~((al) - 1))

typedef struct _mem {
#ifdef BCMDBG_MEM
#define BCM_MEM_FILENAME_LEN	64	/* Filename length */
	uint32		magic;
	uchar		*malloc_function;
	uchar		*free_function;
	char		file[BCM_MEM_FILENAME_LEN];
	int		line;
#endif /* BCMDBG_MEM */
	uint32		size;
	struct _mem	*next;
} mem_t;


static mem_t	free_mem;		/* Free list head */
static uint	arena_size;		/* Total heap size */
static uint	inuse_size;		/* Current in use */
static uint	inuse_overhead;		/* tally of allocated mem_t blocks */
static uint	inuse_hwm;		/* High watermark  of memory - reclaimed memory */
static uint	mf_count;		/* Malloc failure count */

#define	STACK_MAGIC	0x5354414b	/* Magic # for stack protection: 'STAK' */

static	uint32	*bos = NULL;		/* Bottom of the stack */
static	uint32	*tos = NULL;		/* Top of the stack */

#ifdef BCMDBG_MEM
#define	MEM_MAGIC	0x4d4e4743	/* Magic # for mem overwrite check: 'MNGC' */

static mem_t	inuse_mem;		/* In-use list head */
#endif /* BCMDBG_MEM */

static void hndrte_rte_timer(void *tid);
static void update_timeout_list(void);

void
BCMATTACHFN(hndrte_arena_init)(uintptr base, uintptr lim, uintptr stackbottom)
{
	mem_t *first;

	ASSERT(base);
	ASSERT(lim > base);

	/*
	 * Mark the stack with STACK_MAGIC here before using any other
	 * automatic variables! The positions of these variables are
	 * compiler + target + optimization dependant, and they can be
	 * overwritten by the code below if they are allocated before
	 * 'p' towards the stack bottom.
	 */
	{
		uint32 *p = (uint32 *)stackbottom;
		uint32 cbos;

		*p = STACK_MAGIC;

		/* Mark the stack */
		while (++p < &cbos - 32)
			*p = STACK_MAGIC;
	}

	/* Align */
	first = (mem_t *)ALIGN(base, MIN_ALIGN);

	arena_size = lim - (uint32)first;
	inuse_size = 0;
	inuse_overhead = 0;
	inuse_hwm = 0;

	/* Mark the bottom of the stack */
	bos = (uint32 *)stackbottom;
	tos = (uint32 *)((uintptr)bos + HNDRTE_STACK_SIZE);
	mf_count = 0;
#ifdef BCMDBG_MEM
	free_mem.magic = inuse_mem.magic = first->magic = MEM_MAGIC;
	inuse_mem.next = NULL;
#endif /* BCMDBG_MEM */
	first->size = arena_size - sizeof(mem_t);
	first->next = NULL;
	free_mem.next = first;
}

uint
hndrte_arena_add(uint32 base, uint size)
{
	uint32 addr;
	mem_t *this;

	addr = ALIGN(base, MIN_ALIGN);
	if ((addr - base) > size) {
		/* Ignore this miniscule thing,
		 * otherwise size below will go negative!
		 */
		return 0;
	}
	size -= (addr - base);
	size = ALIGN_DOWN(size, MIN_ALIGN);

	if (size < (sizeof(mem_t) + MIN_MEM_SIZE)) {
		return 0;
	}
	this = (mem_t *)addr;
	arena_size += size;
	size -= sizeof(mem_t);
	addr += sizeof(mem_t);
	this->size = size;

	/* This chunk was not in use before, make believe it was */
	inuse_size += size;
	inuse_overhead += sizeof(mem_t);

#ifdef BCMDBG_MEM
	this->magic = MEM_MAGIC;
	this->file[0] = '\0';
	this->line = 0;
	this->next = inuse_mem.next;
	inuse_mem.next = this;
	printf("%s: Adding 0x%x: 0x%x(%d) @ 0x%x\n", __FUNCTION__, (uint32)this,
	       size, size, addr);
#else
	this->next = NULL;
#endif /* BCMDBG_MEM */

	hndrte_free((void *)addr);
	return (size);
}

void *
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
hndrte_malloc_align(uint size, uint alignbits, char *file, int line)
#else
hndrte_malloc_align(uint size, uint alignbits)
#endif /* BCMDBG_MEM */
{
	mem_t	*curr, *last, *this = NULL, *prev = NULL;
	uint	align, rem, waste;
	uintptr	addr = 0, top = 0;
#ifdef BCMDBG_MEM
	char *basename;
#endif /* BCMDBG_MEM */

	ASSERT(size);

	size = ALIGN(size, MIN_ALIGN);

	align = 1 << alignbits;
	if (align <= MIN_ALIGN)
		align = MIN_ALIGN;
	else if (align > MAX_ALIGN)
		align = MAX_ALIGN;

	/* Search for available memory */
	last = &free_mem;
	waste = arena_size;
	while ((curr = last->next) != NULL) {
		if (curr->size >= size) {
			/* Calculate alignment */
			uintptr lowest = (uintptr)curr + sizeof(mem_t);
			uintptr end = lowest + curr->size;
			uintptr highest = end - size;
			uintptr a = ALIGN_DOWN(highest, align);

			/* Find closest sized buffer to avoid fragmentation */
			if ((a >= lowest) && ((curr->size - size) < waste)) {

				waste = curr->size - size;
				this = curr;
				prev = last;
				top = end;
				addr = a;
			}
		}

		last = curr;
	}

	if (this == NULL) {
		mf_count++; /* Increment malloc failure count */
#if defined(BCMDBG_MEM) || defined(BCMDBG_MEMFAIL)
		printf("No memory to satisfy request for %d bytes, inuse %d, file %s, line %d\n",
		       size, inuse_size, file ? file : "unknown", line);
#ifdef BCMDBG_MEM
		hndrte_print_malloc();
#endif
#else
		RTE_MSG(("No memory to satisfy request for %d bytes, inuse %d\n",
		       size, inuse_size));
#endif /* BCMDBG_MEM */
		return NULL;
	}

#ifdef BCMDBG_MEM
	ASSERT(this->magic == MEM_MAGIC);
#endif /* BCMDBG_MEM */
	if (*bos != STACK_MAGIC)
		RTE_MSG(("Stack bottom has been overwritten\n"));
	ASSERT(*bos == STACK_MAGIC);

	/* Anything above? */
	rem = (top - addr) - size;
	if (rem <= (MIN_MEM_SIZE + sizeof(mem_t))) {
		/* take it all */
		size += rem;
	} else {
		/* Split off the top */
		mem_t *new = (mem_t *)(addr + size);

		this->size -= rem;
		new->size = rem - sizeof(mem_t);
#ifdef BCMDBG_MEM
		new->magic = MEM_MAGIC;
#endif /* BCMDBG_MEM */
		new->next = this->next;
		this->next = new;
	}

	/* Anything below? */
	rem = this->size - size;
	if ((rem == 0) ||
	    ((align == MIN_ALIGN) && (rem <= (MIN_MEM_SIZE + sizeof(mem_t))))) {
		/* take it all */
		prev->next = this->next;
	} else {
		/* Split this */
		mem_t *new = (mem_t *)((uint32)this + rem);

		if (rem < sizeof(mem_t)) {
			/* Should NOT happen */
			mf_count++;
			RTE_MSG(("Internal malloc error sz:%d, al:%d, tsz:%d, rem:%5d,"
			       " add:0x%x, top:0x%x\n",
			       size, align, this->size, rem, addr, top));
			return NULL;
		}

		new->size = size;
		this->size = rem - sizeof(mem_t);
#ifdef BCMDBG_MEM
		new->magic = MEM_MAGIC;
#endif /* BCMDBG_MEM */

		this = new;
	}

#ifdef BCMDBG_MEM
	this->next = inuse_mem.next;
	inuse_mem.next = this;
	this->line = line;
	basename = strrchr(file, '/');
	/* skip the '/' */
	if (basename)
		basename++;
	if (!basename)
		basename = file;
	strncpy(this->file, basename, BCM_MEM_FILENAME_LEN);
	this->file[BCM_MEM_FILENAME_LEN - 1] = '\0';
	this->malloc_function = __builtin_return_address(0);
#else
	this->next = NULL;
#endif /* BCMDBG_MEM */
	inuse_size += this->size;
	inuse_overhead += sizeof(mem_t);

	/* find the instance where the free memory was the least to calculate
	 * inuse memory hwm
	 */
	if (inuse_size > inuse_hwm)
		inuse_hwm = inuse_size;

#ifdef BCMDBG_MEM
	RTE_MSG(("malloc: 0x%x\n", (uint32) ((void *)((uintptr)this + sizeof(mem_t)))));
#endif

	return ((void *)((uint32)this + sizeof(mem_t)));
}

void *
hndrte_realloc(void *ptr, uint size)
{
	void *new = hndrte_malloc(size);
	if (new == NULL)
		return NULL;
	memcpy(new, ptr, size);
	hndrte_free(ptr);
	return new;
}

int
hndrte_free(void *where)
{
	uint32 w = (uint32)where;
	mem_t *prev, *next, *this;

#ifdef BCMDBG_MEM
	/* Get it off of the inuse list */
	prev = &inuse_mem;
	while ((this = prev->next) != NULL) {
		if (((uint32)this + sizeof(mem_t)) == w)
			break;
		prev = this;
	}

	if (this == NULL) {
		RTE_MSG(("%s: 0x%x is not in the inuse list\n", __FUNCTION__, w));
		ASSERT(this);
		return -1;
	}

	if (this->magic != MEM_MAGIC) {
		RTE_MSG(("\n%s: Corrupt magic (0x%x) in 0x%x; size %d; file %s, line %d\n\n",
		       __FUNCTION__, this->magic, w, this->size, this->file, this->line));
		ASSERT(this->magic == MEM_MAGIC);
		return -1;
	}

	this->free_function = __builtin_return_address(0);
	prev->next = this->next;
#else
	this = (mem_t *)(w - sizeof(mem_t));
#endif /* BCMDBG_MEM */

	inuse_size -= this->size;
	inuse_overhead -= sizeof(mem_t);

	/* Find the right place in the free list for it */
	prev = &free_mem;
	while ((next = prev->next) != NULL) {
		if (next >= this)
			break;
		prev = next;
	}

	/* Coalesce with next if appropriate */
	if ((w + this->size) == (uint32)next) {
		this->size += next->size + sizeof(mem_t);
		this->next = next->next;
#ifdef BCMDBG_MEM
		next->magic = 0;
#endif /* BCMDBG_MEM */
	} else
		this->next = next;

	/* Coalesce with prev if appropriate */
	if (((uint32)prev + sizeof(mem_t) + prev->size) == (uint32)this) {
		prev->size += this->size + sizeof(mem_t);
		prev->next = this->next;
#ifdef BCMDBG_MEM
		this->magic = 0;
#endif /* BCMDBG_MEM */
	} else
		prev->next = this;

	return 0;
}

uint
hndrte_memavail(void)
{
	return (arena_size - inuse_size);
}

uint
hndrte_hwm(void)
{
	return (inuse_hwm);
}

void
hndrte_print_memuse(void)
{
	uint32 *p;
	uint32 tot;
	uint32 cbos;

	tot = (text_end - text_start) + (data_end - data_start) + (bss_end - bss_start);
	tot += inuse_hwm + HNDRTE_STACK_SIZE;

	printf("Memory usage:\n");
	printf("Text: %ld(%ldK), Data: %ld(%ldK), Bss: %ld(%ldK), Stack: %dK\n",
	       (text_end - text_start), (text_end - text_start)/1024,
	       (data_end - data_start), (data_end - data_start)/1024,
	       (bss_end - bss_start), (bss_end - bss_start)/1024,
	       HNDRTE_STACK_SIZE/1024);
	printf("Arena total: %d(%dK), Free: %d(%dK), In use: %d(%dK), HWM: %d(%dK)\n",
	       arena_size, arena_size/1024,
	       (arena_size - inuse_size), (arena_size - inuse_size)/1024,
	       inuse_size, inuse_size/1024, inuse_hwm, inuse_hwm/1024);
	printf("In use + overhead: %d(%dK), Max memory in use: %d(%dK)\n",
	       inuse_size + inuse_overhead, (inuse_size+ inuse_overhead)/1024,
	       tot, tot/1024);
	printf("Malloc failure count: %d\n", mf_count);

	if (*bos != STACK_MAGIC)
		printf("Stack bottom has been overwritten\n");
	else {
		for (p = bos; p < (uint32 *)(&cbos); p++)
			if (*p != STACK_MAGIC)
				break;

		printf("Stack bottom: 0x%p, lwm: 0x%p, curr: 0x%p, top: 0x%p\n"
		       "Free stack: 0x%x(%d) lwm: 0x%x(%d)\n"
		       "Inuse stack: 0x%x(%d) hwm: 0x%x(%d)\n",
		       bos, p, &p, tos,
		       ((uintptr)(&p) - (uintptr)bos),
		       ((uintptr)(&p) - (uintptr)bos),
		       ((uintptr)p - (uintptr)bos),
		       ((uintptr)p - (uintptr)bos),
		       ((uintptr)tos - (uintptr)(&p)),
		       ((uintptr)tos - (uintptr)(&p)),
		       ((uintptr)tos - (uintptr)p),
		       ((uintptr)tos - (uintptr)p));
	}
}

#ifdef BCMDBG_MEMTEST
/*
* Malloc memory to simulate low memory environment
* Memory waste [kk] in KB usage: mw [kk]
*/
void hndrte_print_memwaste(uint32 arg, uint argc, char *argv[])
{
	/* Only process if input argv specifies the mw size(in KB) */
	if (argc > 1)
		hndrte_malloc(atoi(argv[1])*1024);
}
#endif /* BCMDBG_MEMTEST */

#ifdef BCMDBG_MEM
int
hndrte_memcheck(char *file, int line) {

	mem_t *this = inuse_mem.next;

	while (this) {
		if (this->magic != MEM_MAGIC) {
			printf("CORRUPTION: %s %d\n", file, line);
			printf("\n%s: Corrupt magic (0x%x); size %d; file %s, line %d\n\n",
			       __FUNCTION__, this->magic, this->size, this->file, this->line);
			return (1);
		}
		this = this->next;
	}
	return (0);
}


void
hndrte_print_malloc(void)
{
	uint32 inuse = 0, free = 0, total;
	mem_t *this = inuse_mem.next;

	printf("Heap inuse list:\n");
	while (this) {
		printf("0x%08lx\t%5d\t%s:%d\t0x%p\t0x%p\n", (uintptr)this + sizeof(mem_t),
		       this->size,
		       this->file, this->line, this->malloc_function, this->free_function);
		inuse += this->size + sizeof(mem_t);
		this = this->next;
	}
	printf("Heap free list:\n");
	this = free_mem.next;
	while (this) {
		printf("0x%x: 0x%x(%d) @ 0x%lx\n", (uintptr)this, this->size, this->size,
		       (uintptr)this + sizeof(mem_t));
		free += this->size + sizeof(mem_t);
		this = this->next;
	}
	total = inuse + free;
	printf("Heap Dyn inuse: 0x%x(%d), inuse: 0x%x(%d)\nDyn free: 0x%x(%d), free: 0x%x(%d)\n",
	       inuse, inuse, inuse_size, inuse_size, free, free,
	       (arena_size - inuse_size), (arena_size - inuse_size));
	if (total != arena_size)
		printf("Total (%d) does NOT agree with original %d!\n",
		       total, arena_size);
}
#endif /* BCMDBG_MEM */

/*
 * Timing support:
 *
 * All these routines need some interrupt protection if they are shared
 * by ISR and other functions. However since they are all called from
 * timer or other ISRs except at initialization time so it is safe to
 * not have the protection.
 *
 * hndrte_init_timer(void *context, void *data,
 *	void (*mainfn)(hndrte_timer_t*), void (*auxfn)(void*));
 * hndrte_free_timer(hndrte_timer_t *t);
 * hndrte_add_timer(hndrte_timer_t *t, uint ms, int periodic);
 * hndrte_del_timer(hndrte_timer_t *t);
 *
 * hndrte_init_timeout(ctimeout_t *new);
 * hndrte_del_timeout(ctimeout_t *new);
 * hndrte_timeout(ctimeout_t *new, uint32 ms, to_fun_t fun, uint32 arg);
 */


static ctimeout_t timers = {NULL, 0, NULL, 0};

/* Update expiration times in timeout list */
static void
update_timeout_list(void)
{
	uint32 delta, current;
	volatile ctimeout_t *head = (volatile ctimeout_t *)&timers;
	ctimeout_t *this = head->next;
	static uint32 last = 0;

	current = hndrte_time();
	if ((delta = (current - last)) != 0) {
		last = current;
		while (this != NULL) {
			if (this->ms <= delta) {
				/* timer has expired */
				delta -= this->ms;
				this->ms = 0;
				this->expired = TRUE;
				this = this->next;
			} else {
				this->ms -= delta;
				break;
			}
		}
	}
}

/* Remove expired from timeout list and invoke their callbacks */
static void
run_timeouts(void)
{
	ctimeout_t *this;
	volatile ctimeout_t *head = (volatile ctimeout_t *)&timers;
	to_fun_t fun;

	if ((this = head->next) == NULL)
		return;

	update_timeout_list();

	/* Always start from the head */
	while (((this = head->next) != NULL) && (this->expired == TRUE)) {
		head->next = this->next;
		fun = this->fun;
		this->fun = NULL;
		this->expired = FALSE;
		fun(this->arg);
	}
}

/* Remove specified timeout from the list */
void
hndrte_del_timeout(ctimeout_t *new)
{
	ctimeout_t *this, *prev = &timers;

	while (((this = prev->next) != NULL)) {
		if (this == new) {
			prev->next = this->next;
			this->fun = NULL;
			break;
		}
		prev = this;
	}
}

void
hndrte_init_timeout(ctimeout_t *new)
{
	bzero(new, sizeof(ctimeout_t));
}

/* Add timeout to the list */
bool
hndrte_timeout(ctimeout_t *new, uint32 _ms, to_fun_t fun, void *arg)
{
	ctimeout_t *prev, *this;
	uint32 deltams = _ms;

	if (!new)
		return FALSE;

	if (new->fun != NULL) {
		RTE_MSG(("fun not null in 0x%p, timer already in list?\n", new));
#ifdef	BCMDBG
		hndrte_print_timers(0, 0, NULL);
#endif
		return FALSE;
	}

	update_timeout_list();

	/* find a proper location for the new timeout and update the timer value */
	prev = &timers;
	while (((this = prev->next) != NULL) && (this->ms <= deltams)) {
		deltams -= this->ms;
		prev = this;
	}
	if (this != NULL)
		this->ms -= deltams;

	new->fun = fun;
	new->arg = arg;
	new->ms = deltams;

	/* insert the new timeout */
	new->next = this;
	prev->next = new;

#ifndef HNDRTE_POLLING
	if (new == timers.next && timer_allowed) {
		hndrte_set_irq_timer(new->ms);
		timer_armed = TRUE;
	}
#endif

	return TRUE;
}

/* Primary timeout callback */
static void
hndrte_rte_timer(void *tid)
{
	hndrte_timer_t *t = (hndrte_timer_t *)tid;

	if (t && t->set) {
		if (!t->periodic) {
			t->set = FALSE;
		} else {
			hndrte_timeout(&t->t, t->interval, hndrte_rte_timer, (void *)t);
		}

		if (t->mainfn)
			(*t->mainfn)(t);

		if (t->_freedone == TRUE) {
			hndrte_free_timer(t);
		}
	}
}

#ifdef BCMDBG
void
hndrte_print_timers(uint32 arg, uint argc, char *argv[])
{
	ctimeout_t *this;

	if ((this = timers.next) == NULL) {
		printf("No timers\n");
		return;
	}
	while (this != NULL) {
		printf("timer %p, fun %p, arg %p, %d ms\n", this, this->fun, this->arg,
		       this->ms);
		this = this->next;
	}
}
#endif /* BCMDBG */

hndrte_timer_t*
hndrte_init_timer(void *context, void *data, void (*mainfn)(hndrte_timer_t*), void (*auxfn)(void*))
{
	hndrte_timer_t *t = (hndrte_timer_t *)hndrte_malloc(sizeof(hndrte_timer_t));

	if (t) {
		bzero(t, sizeof(hndrte_timer_t));
		t->context = context;
		t->mainfn = mainfn;
		t->auxfn = auxfn;
		t->data = data;
		t->set = FALSE;
		t->periodic = FALSE;
	} else {
		RTE_MSG(("hndrte_init_timer: hndrte_malloc failed\n"));
	}

	return t;
}

void
hndrte_free_timer(hndrte_timer_t *t)
{
	if (t) {
		hndrte_free(t);
	}
}

bool
hndrte_add_timer(hndrte_timer_t *t, uint ms, int periodic)
{
	if (t) {
		t->set = TRUE;
		t->periodic = periodic;
		t->interval = ms;

		return hndrte_timeout(&t->t, ms, hndrte_rte_timer, (void *)t);
	}
	return FALSE;
}

bool
hndrte_del_timer(hndrte_timer_t *t)
{
	if (t->set) {
		t->set = FALSE;

		hndrte_del_timeout((ctimeout_t*)&t->t);
	}

	return (TRUE);
}

void
hndrte_timer_isr(void)
{
	timer_armed = FALSE;
	run_timeouts();
	if (timer_armed)
		return;
	if (timers.next != NULL)
		hndrte_set_irq_timer(timers.next->ms);
	else
		hndrte_ack_irq_timer();
}

/* Schedule a completion handler to run at safe time */
int
hndrte_schedule_work(void *context, void *data, void (*taskfn)(hndrte_timer_t *), int delay)
{
	hndrte_timer_t *task;

	if (!(task = hndrte_init_timer(context, data, taskfn, NULL))) {
		return BCME_NORESOURCE;
	}
	task->_freedone = TRUE;

	if (!hndrte_add_timer(task, delay, FALSE)) {
		hndrte_free_timer(task);
		return BCME_NORESOURCE;
	}

	return 0;
}

static uint32 now = 0;

/* Return the up time in miliseconds */
uint32
hndrte_time(void)
{
	now += hndrte_update_now();

	return now;
}

/* Cancel the h/w timer if it is already armed and ignore any further h/w timer requests */
void
hndrte_suspend_timer(void)
{
	hndrte_ack_irq_timer();
	timer_allowed = FALSE;
}

/* Resume the timer activities */
void
hndrte_resume_timer(void)
{
	hndrte_set_irq_timer(0);
	timer_allowed = TRUE;
}

/*
 * hndrte.
 *
 * Device support:
 *	PCI support if included.
 *	hndrte_add_device(dev, coreid, device): Add a device.
 *	hndrte_get_dev(char *name): Get named device struct.
 *	hndrte_isr(): Invoke device ISRs.
 */

#ifdef	SBPCI

static pdev_t *pcidevs = NULL;


static bool
hndrte_read_pci_config(si_t *sih, uint slot, pci_config_regs *pcr)
{
	uint32 *p;
	uint i;

	extpci_read_config(sih, 1, slot, 0, PCI_CFG_VID, pcr, 4);
	if (pcr->vendor == PCI_INVALID_VENDORID)
		return FALSE;

	for (p = (uint32 *)((uint)pcr + 4), i = 4; i < SZPCR; p++, i += 4)
		extpci_read_config(sih, 1, slot, 0, i, p, 4);

	return TRUE;
}

static uint32
BCMINITFN(size_bar)(si_t *sih, uint slot, uint bar)
{
	uint32 w, v, s;

	w = 0xffffffff;
	extpci_write_config(sih, 1, slot, 0, (PCI_CFG_BAR0 + (bar * 4)), &w, 4);
	extpci_read_config(sih, 1, slot, 0,  (PCI_CFG_BAR0 + (bar * 4)), &v, 4);

	/* We don't do I/O */
	if (v & 1)
		return 0;

	/* Figure size */
	v &= 0xfffffff0;
	s = 0 - (int32)v;

	return s;
}

#define	MYPCISLOT	0
#define	HNDRTE_PCIADDR	0x20000000
static uint32 pciaddr = HNDRTE_PCIADDR;

static uint32
BCMINITFN(set_bar0)(si_t *sih, uint slot)
{
	uint32 w, mask, s, b0;

	/* Get size, give up if zero */
	s = size_bar(sih, slot, 0);
	if (s == 0)
		return 0;

	mask = s - 1;
	if ((pciaddr & mask) != 0)
		pciaddr = (pciaddr + s) & ~mask;
	b0 = pciaddr;
	extpci_write_config(sih, 1, slot, 0, PCI_CFG_BAR0, &b0, 4);
	extpci_read_config(sih, 1, slot, 0, PCI_CFG_BAR0, &w, 4);

	RTE_MSG(("set_bar0: 0x%x\n", w));
	/* Something went wrong */
	if ((w & 0xfffffff0) != b0)
		return 0;

	pciaddr += s;
	return b0;
}

static void
BCMINITFN(hndrte_init_pci)(si_t *sih)
{
	pci_config_regs pcr;
	sbpciregs_t *pci;
	pdev_t *pdev;
	uint slot;
	int rc;
	uint32 w, s, b0;
	uint16 hw, hr;
	extern uint32 _memsize;

	rc = hndpci_init_pci(sih);
	if (rc < 0) {
		RTE_MSG(("Cannot init PCI.\n"));
		return;
	} else if (rc > 0) {
		RTE_MSG(("PCI strapped for client mode.\n"));
		return;
	}

	if (!(pci = (sbpciregs_t *)si_setcore(sih, PCI_CORE_ID, 0))) {
		RTE_MSG(("Cannot setcore to PCI after init!!!\n"));
		return;
	}

	if (!hndrte_read_pci_config(sih, MYPCISLOT, &pcr) ||
	    (pcr.vendor != VENDOR_BROADCOM) ||
	    (pcr.base_class != PCI_CLASS_BRIDGE) ||
	    ((pcr.base[0] & 1) == 1)) {
		RTE_MSG(("Slot %d is not us!!!\n", MYPCISLOT));
		return;
	}

	/* Change the 64 MB I/O window to memory with base at HNDRTE_PCIADDR */
	W_REG(sb_osh(sih), &pci->sbtopci0, SBTOPCI_MEM | HNDRTE_PCIADDR);

	/* Give ourselves a bar0 for the fun of it */
	if ((b0 = set_bar0(sih, MYPCISLOT)) == 0) {
		RTE_MSG(("Cannot set my bar0!!!\n"));
		return;
	}
	/* And point it to our chipc */
	w = SI_ENUM_BASE;
	extpci_write_config(sih, 1, MYPCISLOT, 0, PCI_BAR0_WIN, &w, 4);
	extpci_read_config(sih, 1, MYPCISLOT, 0, PCI_BAR0_WIN, &w, 4);
	if (w != SI_ENUM_BASE) {
		RTE_MSG(("Cannot set my bar0window: 0x%08x should be 0x%08x\n", w, SI_ENUM_BASE));
	}

	/* Now setup our bar1 */
	if ((s = size_bar(sih, 0, 1)) < _memsize) {
		RTE_MSG(("My bar1 is disabled or too small: %d(0x%x)\n", s, s));
		return;
	}

	/* Make sure bar1 maps PCI address 0, and maps to memory */
	w = 0;
	extpci_write_config(sih, 1, MYPCISLOT, 0, PCI_CFG_BAR1, &w, 4);
	extpci_write_config(sih, 1, MYPCISLOT, 0, PCI_BAR1_WIN, &w, 4);

	/* Do we want to record ourselves in the pdev list? */
	RTE_MSG(("My slot %d, device 0x%04x:0x%04x subsys 0x%04x:0x%04x\n",
	       MYPCISLOT, pcr.vendor, pcr.device, pcr.subsys_vendor, pcr.subsys_id));

	/* OK, finally find the pci devices */
	for (slot = 0; slot < PCI_MAX_DEVICES; slot++) {

		if (slot == MYPCISLOT)
			continue;

		if (!hndrte_read_pci_config(sih, slot, &pcr) ||
		    (pcr.vendor == PCI_INVALID_VENDORID))
			continue;

		RTE_MSG(("Slot %d, device 0x%04x:0x%04x subsys 0x%04x:0x%04x ",
		       slot, pcr.vendor, pcr.device, pcr.subsys_vendor, pcr.subsys_id));

		/* Enable memory & master capabilities */
		hw = pcr.command | 6;
		extpci_write_config(sih, 1, slot, 0, PCI_CFG_CMD, &hw, 2);
		extpci_read_config(sih, 1, slot, 0, PCI_CFG_CMD, &hr, 2);
		if ((hr & 6) != 6) {
			RTE_MSG(("does not support master/memory operation (cmd: 0x%x)\n", hr));
			continue;
		}

		/* Give it a bar 0 */
		b0 = set_bar0(sih, slot);
		if ((b0 = set_bar0(sih, slot)) == 0) {
			RTE_MSG(("cannot set its bar0\n"));
			continue;
		}
		RTE_MSG(("\n"));

		pdev = hndrte_malloc(sizeof(pdev_t));
		pdev->sih = sih;
		pdev->vendor = pcr.vendor;
		pdev->device = pcr.device;
		pdev->bus = 1;
		pdev->slot = slot;
		pdev->func = 0;
		pdev->address = (void *)REG_MAP(SI_PCI_MEM | (b0 - HNDRTE_PCIADDR), SI_PCI_MEM_SZ);
		pdev->inuse = FALSE;
		pdev->next = pcidevs;
		pcidevs = pdev;
	}
}

#endif	/* SBPCI */

hndrte_dev_t *dev_list = NULL;

int
BCMINITFN(hndrte_add_device)(hndrte_dev_t *dev, uint16 coreid, uint16 device)
{
	int err = -1;
	void *softc = NULL;
	void *regs = NULL;
	uint unit = 0;

	if (coreid == NODEV_CORE_ID) {
		/* "Soft" device driver, just call its probe */
		if ((softc = (dev->dev_funcs->probe)(dev, NULL, SI_BUS, device,
		                                     coreid, unit)) != NULL)
			err = 0;
	} else if ((regs = si_setcore(hndrte_sih, coreid, unit)) != NULL) {
		/* Its a core in the SB */
		if ((softc = (dev->dev_funcs->probe)(dev, regs, SI_BUS, device,
		                                     coreid, unit)) != NULL)
			err = 0;
	}
#ifdef	SBPCI
	else {
		/* Try PCI devices */
		pdev_t *pdev = pcidevs;
		while (pdev != NULL) {
			if (!pdev->inuse) {
				dev->pdev = pdev;
				softc = (dev->dev_funcs->probe)(dev, pdev->address, PCI_BUS,
				                                pdev->device, coreid, unit);
				if (softc != NULL) {
					pdev->inuse = TRUE;
					regs = si_setcore(hndrte_sih, PCI_CORE_ID, 0);
					err = 0;
					break;
				}
			}
			pdev = pdev->next;
		}
		if (softc == NULL)
			dev->pdev = NULL;
	}
#endif	/* SBPCI */

	if (err != 0)
		return err;

	dev->devid = device;
	dev->dev_softc = softc;

	/* Add device to the head of devices list */
	dev->dev_next = dev_list;
	dev_list = dev;

	return 0;
}

hndrte_dev_t *
hndrte_get_dev(char *name)
{
	hndrte_dev_t *dev;

	/* Loop through each dev, looking for a match */
	for (dev = dev_list; dev != NULL; dev = dev->dev_next)
		if (strcmp(dev->dev_fullname, name) == 0)
			break;

	return dev;
}

#ifdef HNDRTE_POLLING
/* hndrte_dev_poll(): run the poll routines for all devices for interfaces, it means isrs */
static void
hndrte_dev_poll(void)
{
	hndrte_dev_t *dev = dev_list;

	/* Loop through each dev's isr routine until one of them claims the interrupt */
	while (dev) {
		/* call isr routine if one is registered */
		if (dev->dev_funcs->poll)
			dev->dev_funcs->poll(dev);

		dev = dev->dev_next;
	}
}

#else	/* !HNDRTE_POLLING */
/* hndrte ISR support:
 * hndrte_add_isr(action, irq, coreid, isr, cbdata): Add a ISR
 * hndrte_isr(): run the interrupt service routines for all devices
 */
/* ISR instance */
typedef struct hndrte_irq_action hndrte_irq_action_t;

struct hndrte_irq_action {
	hndrte_irq_action_t *next;
	isr_fun_t isr;
	void *cbdata;
	uint32 sbtpsflag;
};

static hndrte_irq_action_t *action_list = NULL;

/* irq is for future use */
int
BCMINITFN(hndrte_add_isr)(uint irq, uint coreid, uint unit,
                          isr_fun_t isr, void *cbdata)
{
	void	*regs;
	uint	origidx;
	hndrte_irq_action_t *action;

	if ((action = hndrte_malloc(sizeof(hndrte_irq_action_t))) == NULL) {
		RTE_MSG(("hndrte_add_isr: hndrte_malloc failed\n"));
		return BCME_NOMEM;
	}

	origidx = si_coreidx(hndrte_sih);
	regs = si_setcore(hndrte_sih, coreid, unit);
	ASSERT(regs);

	action->sbtpsflag = 1 << si_flag(hndrte_sih);
	action->isr = isr;
	action->cbdata = cbdata;
	action->next = action_list;
	action_list = action;

	/* restore core original idx */
	si_setcoreidx(hndrte_sih, origidx);

	return BCME_OK;
}

void
hndrte_isr(void)
{
	hndrte_irq_action_t *action;
	uint32 sbflagst;

	/* Get the flag bit corresponding to core interrupts */
	sbflagst = R_REG(hndrte_osh, &hndrte_ccsbr->sbflagst);

	/* Loop through each registered isr routine until one of
	 * them clain the interrupt
	 */
	action = action_list;
	while (action) {
		if ((sbflagst & action->sbtpsflag) && action->isr)
			(action->isr)(action->cbdata);

		action = action->next;
	}
}
#endif	/* !HNDRTE_POLLING */

/*
 * hndrte.
 *
 * Initialize and background:
 *	hndrte_init: Initialize the world.
 *	hndrte_poll: Run background work once.
 *	hndrte_idle: Run background work forever.
 */

#ifdef _HNDRTE_SIM_
extern uchar *sdrambuf;
#endif

void *
BCMATTACHFN(hndrte_init)(void)
{
	uint32 stackBottom = 0xdeaddead;
	uchar *ramStart, *ramLimit;

#if defined(EXT_CBALL)
	ramStart = (uchar *)RAMADDRESS;
	ramLimit = ramStart + RAMSZ;
#elif defined(_HNDRTE_SIM_)
	ramStart = sdrambuf;
	ramLimit = ramStart + RAMSZ;
#else
	ramStart = (uchar *)_end;
	ramLimit = (uchar *)&stackBottom - HNDRTE_STACK_SIZE;
#endif

	/* Init malloc */
	hndrte_arena_init((uintptr)ramStart,
	                  (uintptr)ramLimit,
	                  ((uintptr)&stackBottom) - HNDRTE_STACK_SIZE);

	hndrte_disable_interrupts();

#ifdef HNDRTE_CONSOLE
	/* Create a logbuf separately from a console. This logbuf will be
	 * dumpped and reused when the console is created later on.
	 */
	hndrte_log_init();
#endif

	/* Now that we have initialized memory management let's allocate the osh */
	hndrte_osh = osl_attach(NULL);
	ASSERT(hndrte_osh);

	/* Scan backplane */
	hndrte_sih = si_kattach(hndrte_osh);
	ASSERT(hndrte_sih);

	/* Initialize chipcommon related stuff */
	hndrte_chipc_init(hndrte_sih);

	/* Initialize CPU related stuff */
	hndrte_cpu_init(hndrte_sih);

	/* Initialize timer */
	hndrte_update_now();

#ifdef HNDRTE_CONSOLE
	/* No printf's come out earlier than this */
	hndrte_cons_init(hndrte_sih);
	/* Add a few commands */
	hndrte_cons_addcmd("mu", (cons_fun_t)hndrte_print_memuse, 0);
#ifdef BCMDBG_MEMTEST
	/* Max runtime memory requirement test */
	hndrte_cons_addcmd("mw", (cons_fun_t)hndrte_print_memwaste, 0);
#endif /* BCMDBG_MEMTEST */
#ifdef BCMDBG_MEM
	hndrte_cons_addcmd("ar", (cons_fun_t)hndrte_print_malloc, 0);
#endif /* BCMDBG_MEM */
#ifdef	BCMDBG
	hndrte_cons_addcmd("tim", hndrte_print_timers, 0);
#endif /* BCMDBG */
#endif /* HNDRTE_CONSOLE */

#ifdef	SBPCI
	/* Init pci core if there is one */
	hndrte_init_pci((void *)hndrte_sih);
#endif /* SBPCI */

#ifdef BCMECICOEX
	/* Initialize ECI registers */
	hndrte_eci_init(hndrte_sih);
#endif /* BCMECICOEX */

#ifdef WLGPIOHLR
	/* Initialize GPIO */
	hndrte_gpio_init(hndrte_sih);
#endif /* WLGPIOHLR */

	return ((void *)hndrte_sih);
}

void
hndrte_poll(si_t *sih)
{
#ifdef HNDRTE_POLLING
	run_timeouts();
	hndrte_dev_poll();
#else
	hndrte_wait_irq(sih);
#endif
}

void
hndrte_idle(si_t *sih)
{
#ifndef HNDRTE_POLLING
	hndrte_enable_interrupts();
#endif
	hndrte_idle_init(sih);
	while (TRUE)
		hndrte_poll(sih);
}

#ifdef BCMDBG_ASSERT
#include <hndrte_trap.h>

#ifdef BCMSPACE
void
hndrte_assert(char *exp, char *file, int line)
{
	printf("ASSERT \"%s\" in file %s line %d\n", exp, file, line);
	hndrte_die(line);
}
#else
void
hndrte_assert(char *exp, int line)
{
	printf("ASSERT \"%s\" in line %d\n", exp, line);
	hndrte_die(line);
}
#endif /* BCMSPACE */
#endif /* BCMDBG_ASSERT */

#ifdef HNDRTE_POLLING
static hndrte_devfuncs_t cfuns;
static hndrte_dev_t cdev;

static void *
BCMINITFN(hndrte_chipc_probe)(hndrte_dev_t *dev, void *regs, uint bus,
                              uint16 device, uint coreid, uint unit)
{
	return regs;
}
#endif	/* HNDRTE_POLLING */

static void
hndrte_chipc_isr(hndrte_dev_t *dev)
{
	si_cc_isr(hndrte_sih, hndrte_ccr);
}

static void
BCMATTACHFN(hndrte_chipc_init)(si_t *sih)
{
	/* get chipcommon and its sbconfig addr */
	hndrte_ccr = si_setcoreidx(sih, SI_CC_IDX);

	/* only support chips that have chipcommon */
	ASSERT(hndrte_ccr);
	hndrte_ccsbr = (sbconfig_t *)((ulong)hndrte_ccr + SBCONFIGOFF);

	/* register dev or isr */
#ifdef HNDRTE_POLLING
	cfuns.probe = hndrte_chipc_probe;
	cfuns.open = NULL;
	cfuns.close = NULL;
	cfuns.xmit = NULL;
	cfuns.ioctl = NULL;
	cfuns.poll = hndrte_chipc_isr;

	strcpy(cdev.dev_fullname, "cc");
	cdev.dev_funcs = &cfuns;
	cdev.devid = 0;
	cdev.dev_softc = NULL;
	cdev.flags = 0;
	cdev.dev_next = NULL;

	hndrte_add_device(&cdev, CC_CORE_ID, BCM4710_DEVICE_ID);
#else
	hndrte_add_isr(0, CC_CORE_ID, 0, (isr_fun_t)hndrte_chipc_isr, NULL);
#endif	/* !HNDRTE_POLLING */
}

#ifdef WLGPIOHLR
static void
hndrte_gpio_isr(void* cbdata, uint32 ccintst)
{
	si_t *sih = (si_t *)cbdata;
	sb_gpio_handler_process(sih);
}

static void
BCMINITFN(hndrte_gpio_init)(si_t *sih)
{
	if (sih->ccrev < 11)
		return;
	si_cc_register_isr(sih, hndrte_gpio_isr, CI_GPIO, (void *)sih);
	si_gpio_int_enable(sih, TRUE);
}
#endif	/* WLGPIOHLR */

#ifdef BCMECICOEX
static void
BCMINITFN(hndrte_eci_init)(si_t *sih)
{
	if (sih->ccrev < 21)
		return;
	si_eci_init(sih);
}
#endif	/* BCMECICOEX */
