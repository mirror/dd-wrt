/* $Id: atomic.S,v 1.4 2001/11/18 00:12:56 davem Exp $
 * atomic.S: These things are too big to do inline.
 *
 * Copyright (C) 1999 David S. Miller (davem@redhat.com)
 */

#include <linux/config.h>
#include <asm/asi.h>

	/* On SMP we need to use memory barriers to ensure
	 * correct memory operation ordering, nop these out
	 * for uniprocessor.
	 */
#ifdef CONFIG_SMP
#define ATOMIC_PRE_BARRIER	membar #StoreLoad | #LoadLoad
#define ATOMIC_POST_BARRIER	membar #StoreLoad | #StoreStore
#else
#define ATOMIC_PRE_BARRIER	nop
#define ATOMIC_POST_BARRIER	nop
#endif

	.text

	.globl	atomic_impl_begin, atomic_impl_end
atomic_impl_begin:
	/* Two versions of the atomic routines, one that
	 * does not return a value and does not perform
	 * memory barriers, and a second which returns
	 * a value and does the barriers.
	 */
	.globl	atomic_add
	.type	atomic_add,#function
atomic_add: /* %o0 = increment, %o1 = atomic_ptr */
1:	lduw	[%o1], %g5
	add	%g5, %o0, %g7
	cas	[%o1], %g5, %g7
	cmp	%g5, %g7
	bne,pn	%icc, 1b
	 nop
	retl
	 nop
	.size	atomic_add, .-atomic_add

	.globl	atomic_sub
	.type	atomic_sub,#function
atomic_sub: /* %o0 = decrement, %o1 = atomic_ptr */
1:	lduw	[%o1], %g5
	sub	%g5, %o0, %g7
	cas	[%o1], %g5, %g7
	cmp	%g5, %g7
	bne,pn	%icc, 1b
	 nop
	retl
	 nop
	.size	atomic_sub, .-atomic_sub

	.globl	atomic_add_ret
	.type	atomic_add_ret,#function
atomic_add_ret: /* %o0 = increment, %o1 = atomic_ptr */
	ATOMIC_PRE_BARRIER
1:	lduw	[%o1], %g5
	add	%g5, %o0, %g7
	cas	[%o1], %g5, %g7
	cmp	%g5, %g7
	bne,pn	%icc, 1b
	 add	%g7, %o0, %g7
	ATOMIC_POST_BARRIER
	retl
	 sra	%g7, 0, %o0
	.size	atomic_add_ret, .-atomic_add_ret

	.globl	atomic_sub_ret
	.type	atomic_sub_ret,#function
atomic_sub_ret: /* %o0 = decrement, %o1 = atomic_ptr */
	ATOMIC_PRE_BARRIER
1:	lduw	[%o1], %g5
	sub	%g5, %o0, %g7
	cas	[%o1], %g5, %g7
	cmp	%g5, %g7
	bne,pn	%icc, 1b
	 sub	%g7, %o0, %g7
	ATOMIC_POST_BARRIER
	retl
	 sra	%g7, 0, %o0
	.size	atomic_sub_ret, .-atomic_sub_ret
atomic_impl_end:
