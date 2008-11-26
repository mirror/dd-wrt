/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/init.h>
#include <linux/mm.h>

#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>


/*
 * ARMv4 optimised copy_user_page
 *
 * We flush the destination cache lines just before we write the data into the
 * corresponding address.  Since the Dcache is read-allocate, this removes the
 * Dcache aliasing issue.  The writes will be forwarded to the write buffer,
 * and merged as appropriate.
 *
 * Note: We rely on all ARMv4 processors implementing the "invalidate D line"
 * instruction.  If your processor does not supply this, you have to write your
 * own copy_user_page that does the right thing.
 */
static void __attribute__((naked))
v4wb_wa_copy_user_page(void *to, const void *from, unsigned long vaddr)
{   
        asm volatile(
        "stmfd	sp!, {r4, lr}			@ 2\n\
	mov	r2, %2		@ 1\n\
	ldmia	r1!, {r3, r4, ip, lr}		@ 4\n\
1:	@mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line\n\
	stmia	r0!, {r3, r4, ip, lr}		@ 4\n\
	ldmia	r1!, {r3, r4, ip, lr}		@ 4+1\n\
	stmia	r0!, {r3, r4, ip, lr}		@ 4\n\
	ldmia	r1!, {r3, r4, ip, lr}		@ 4\n\
        sub     r0, r0, #32                     @ clean & invalidate D line (write allocate)\n\
	mcr	p15, 0, r0, c7, c14, 1		@ \n\
        add     r0, r0, #32                     @ \n\
	@mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line\n\
	stmia	r0!, {r3, r4, ip, lr}		@ 4\n\
	ldmia	r1!, {r3, r4, ip, lr}		@ 4\n\
	subs	r2, r2, #1			@ 1\n\
	stmia	r0!, {r3, r4, ip, lr}		@ 4\n\
	ldmneia	r1!, {r3, r4, ip, lr}		@ 4\n\
        sub     r0, r0, #32                     @ clean & invalidate D line (write allocate) \n\
	mcr	p15, 0, r0, c7, c14, 1		@\n\
        add     r0, r0, #32                     @ \n\
        bne	1b				@ 1\n\
	mcr	p15, 0, r1, c7, c10, 4		@ 1   drain WB\n\
	ldmfd	 sp!, {r4, pc}			@ 3"
	:
	: "r" (to), "r" (from), "I" (PAGE_SIZE / 64));
}

/*
 * Orion II with write allocate optimised clear_user_page
 *
 * Same story as above.
 */
void __attribute__((naked))
v4wb_wa_clear_user_page(void *kaddr, unsigned long vaddr)
{
        asm volatile(
        "str	lr, [sp, #-4]!\n\
	mov	r1, %0		@ 1\n\
	mov	r2, #0				@ 1\n\
	mov	r3, #0				@ 1\n\
	mov	ip, #0				@ 1\n\
	mov	lr, #0				@ 1\n\
1:	@mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line\n\
	stmia	r0!, {r2, r3, ip, lr}		@ 4\n\
	stmia	r0!, {r2, r3, ip, lr}		@ 4\n\
        sub     r0, r0, #32                     @ clean & invalidate D line-> write allocate\n\
	mcr	p15, 0, r0, c7, c14, 1		@ \n\
        add     r0, r0, #32                     @ \n\
	@mcr	p15, 0, r0, c7, c6, 1		@ 1   invalidate D line\n\
	stmia	r0!, {r2, r3, ip, lr}		@ 4\n\
	stmia	r0!, {r2, r3, ip, lr}		@ 4\n\
        sub     r0, r0, #32                     @ clean & invalidate D line-> write allocate\n\
	mcr	p15, 0, r0, c7, c14, 1		@\n\
        add     r0, r0, #32                     @\n\
	subs	r1, r1, #1			@ 1\n\
	bne	1b				@ 1\n\
	mcr	p15, 0, r1, c7, c10, 4		@ 1   drain WB\n\
	ldr	pc, [sp], #4"
        :
        : "I" (PAGE_SIZE / 64));
}

extern void v4wb_clear_user_page(void *p, unsigned long user);
extern void v4wb_copy_user_page(void *to, const void *from,
				 unsigned long user);
struct cpu_user_fns v4wb_wa_user_fns __initdata = {
	.cpu_clear_user_page	= v4wb_clear_user_page, 
	.cpu_copy_user_page	= v4wb_copy_user_page,
};

 
static int __init mrvl_userpage_init(void)
{
    unsigned int r14;

#if defined(CONFIG_ARCH_FEROCEON_KW) || defined(CONFIG_ARCH_FEROCEON_MV78XX0)
    asm("mrc p15, 1, %0, c15, c1, 0;" : "=r"(r14) :);
#else 
    asm("mrc p15, 0, %0, c14, cr0, 0;" : "=r"(r14) :);
#endif    
    /* if write allocate is enabled (bit 28), then use the above functions*/
    if (r14 & (0x1<<28)) {
        cpu_user.cpu_clear_user_page = v4wb_wa_clear_user_page;
        cpu_user.cpu_copy_user_page = v4wb_wa_copy_user_page;
    }
    
    return 0;
}

core_initcall(mrvl_userpage_init);
