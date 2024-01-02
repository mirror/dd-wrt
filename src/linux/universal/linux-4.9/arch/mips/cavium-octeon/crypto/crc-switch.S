/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995, 1996, 1998, 1999, 2002, 2003 Ralf Baechle
 * Copyright (C) 1996 David S. Miller (davem@davemloft.net)
 * Copyright (C) 1994, 1995, 1996, by Andreas Busse
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Copyright (C) 2000 MIPS Technologies, Inc.
 *    written by Carsten Langgaard, carstenl@mips.com
 */

#define USE_ALTERNATE_RESUME_IMPL 1
	.set push
	.set arch=mips64r2
	#include <asm/asm.h>
	#include <asm/cachectl.h>
	#include <asm/fpregdef.h>
	#include <asm/mipsregs.h>
	#include <asm/asm-offsets.h>
	#include <asm/regdef.h>
	#include <asm/stackframe.h>
	#include <asm/thread_info.h>

	#include <asm/asmmacro.h>

	.set pop

/*
 * void octeon_cop2_save(struct octeon_cop2_state *a0)
 */
	.align	7
	.set push
	.set noreorder
	LEAF(octeon_cop2_crc_save)
	dmfc0	t9, $9,7	/* CvmCtl register. */

	/* Save the COP2 CRC state */
	dmfc2	t0, 0x0201
	dmfc2	t1, 0x0202
	dmfc2	t2, 0x0200
	sd	t0, OCTEON_CP2_CRC_IV(a0)
	sd	t1, OCTEON_CP2_CRC_LENGTH(a0)
	nop
	sd	t2, OCTEON_CP2_CRC_POLY(a0)
	nop
	jr	ra
	 nop
	END(octeon_cop2_crc_save)
	.set pop

	.align	7
	.set push
	.set noreorder
	LEAF(octeon_cop2_crc_restore)
	/* First cache line was prefetched before the call */
	pref	4,  128(a0)
	dmfc0	t9, $9,7	/* CvmCtl register. */
	pref	4,  256(a0)
	ld	t0, OCTEON_CP2_CRC_IV(a0)
	pref	4,  384(a0)
	ld	t1, OCTEON_CP2_CRC_LENGTH(a0)
	ld	t2, OCTEON_CP2_CRC_POLY(a0)
	nop
	jr	ra
	nop
	END(octeon_cop2_crc_restore)
	.set pop

