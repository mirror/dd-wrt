/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Assembler-only file 
 */

#include <asm/arch/orion_ver.h>

mv_orion_ver_address:
  	.word   mv_orion_ver

.macro	mv_check_orion_ver, rd
        ldr     \rd, mv_orion_ver_address 
	ldr     \rd, [\rd]
	.endm

support_wait_for_interrupt_address:
        .word   support_wait_for_interrupt

/* rd, rs, rt, re - are temp registers that will b used (non are input/output) */
.macro mv_flush_all, rd, rs, rt, re
	mov     \re, #0

        mv_check_orion_ver r1
        tst     \rd, #MV_ORION1
        bne     1f /* if Orion I*/
	tst     \rd, #MV_ORION2
        bne     2f
	b	5f

1:
        mov     \rd, #(1 - 1) << 30      @ 1 way cache
#if (CONFIG_MV_DCACHE_SIZE == 0x4000)
	mov     \rs, #(512 * CACHE_DLINESIZE)
#else
        mov     \rs, #(1024 * CACHE_DLINESIZE)
#endif
        b       3f

2:
        mov     \rd, #(4 - 1) << 30      @ 4 way cache
        mov     \rs, #(256 * CACHE_DLINESIZE)
        b       3f

3:      orr     \rt, \re, \rd
4:      mcr     p15, 0, \rt, c7, c14, 2          @ clean & invalidate D index
        subs    \rt, \rt, #1 << 30
        bcs     4b                              @ entries 3 to 0
        add     \re, \re, #32
        cmp     \re, \rs
        bne     3b
	b 	6f	/*exit*/

/* default implementation */
5:   	mrc     p15, 0, r15, c7, c14, 3         @ test,clean,invalidate
    	bne     5b

/* exit */
6:	
	.endm
