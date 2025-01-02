/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Cavium Networks
 * Cache error handler
 */

#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/stackframe.h>

/*
 * Handle cache error. Indicate to the second level handler whether
 * the exception is recoverable.
 */
	LEAF(except_vec2_octeon)

	.set	push
	.set	mips64r2
	.set	noreorder
	.set	noat


	/* due to an errata we need to read the COP0 CacheErr (Dcache)
	 * before any cache/DRAM access	 */

	rdhwr	k0, $0	      /* get core_id */
	PTR_LA	k1, cache_err_dcache
	sll	k0, k0, 3
	PTR_ADDU k1, k0, k1    /* k1 = &cache_err_dcache[core_id] */

	dmfc0	k0, CP0_CACHEERR, 1
	sd	k0, (k1)
	dmtc0	$0, CP0_CACHEERR, 1

	/* check whether this is a nested exception */
	mfc0	k1, CP0_STATUS
	andi	k1, k1, ST0_EXL
	beqz	k1, 1f
	 nop
	dla	k0, cache_parity_error_octeon_non_recoverable
	jr	k0
	 nop

	/* exception is recoverable */
1:	dla	k0, handle_cache_err
	jr	k0
	 nop

	.set	pop
	END(except_vec2_octeon)

  /* Handle cache error for OCTEON3. */
	LEAF(except_vec2_octeon3)

	.set	push
	.set	mips64r2
	.set	noreorder
	.set	noat

	/* Invalidate the DCache */
	cache	1, 0($0)

	/* check whether this is a nested exception */
	mfc0	k1, CP0_STATUS
	andi	k1, k1, ST0_EXL
	beqz	k1, 2f
	 nop
	dla	k0, cache_parity_error_octeon_non_recoverable
	jr	k0
	 nop

	/* exception is recoverable */
2:	dla	k0, handle_cache_err
	jr	k0
	 nop

	.set	pop
	END(except_vec2_octeon3)

 /* We need to jump to handle_cache_err so that the previous handler
  * can fit within 0x80 bytes. We also move from 0xFFFFFFFFAXXXXXXX
  * space (uncached) to the 0xFFFFFFFF8XXXXXXX space (cached).	*/
	LEAF(handle_cache_err)
	.set	push
	.set	noreorder
	.set	noat

	SAVE_ALL
	CLI
	jal	cache_parity_error_octeon_recoverable
	 nop
	RESTORE_ALL_AND_RET
	.set pop
	END(handle_cache_err)
