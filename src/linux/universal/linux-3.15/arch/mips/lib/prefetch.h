/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012  Felix Fietkau <nbd@openwrt.org>
 */

.macro	prefetch_store	dst, size, temp1, temp2, temp3
#ifdef CONFIG_CPU_MIPS32
	li		\temp1, 31
	nor		\temp1, \temp1, \temp1

	and		\temp2, \size, \temp1
	beqz		\temp2, 2f
	 nop

	move		\temp2, \dst
	PTR_ADDIU	\temp2, 31
	and		\temp2, \temp2, \temp1

	move		\temp3, \dst
	PTR_ADDU	\temp3, \size
	and		\temp3, \temp3, \temp1

1:	beq		\temp2, \temp3, 2f
	 nop

	pref		30, 0(\temp2)

	b		1b
	  PTR_ADDIU	\temp2, 32
2:
#endif
.endm
