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


support_wait_for_interrupt_address:
        .word   support_wait_for_interrupt

/* rd, rs, rt, re - are temp registers that will b used (non are input/output) */
.macro mv_flush_all, rd, rs, rt, re
	mov     \re, #0

        mov     \rd, #(4 - 1) << 30      @ 4 way cache
        mov     \rs, #(256 * CACHE_DLINESIZE)

1:      orr     \rt, \re, \rd
2:      mcr     p15, 0, \rt, c7, c14, 2          @ clean & invalidate D index
        subs    \rt, \rt, #1 << 30
        bcs     2b                              @ entries 3 to 0
        add     \re, \re, #32
        cmp     \re, \rs
        bne     1b

/* exit */	
	.endm
