	/*
 * Linux dynamic resolving code for MIPS. Fixes up the GOT entry as
 * indicated in register t8 and jumps to the resolved address. Shamelessly
 * ripped from 'sysdeps/mips/dl-machine.h' in glibc-2.2.5.
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 *
 * Copyright (C) 1996-2001 Kazumoto Kojima <kkojima@info.kanagawa-u.ac.jp>
 * Copyright (C) 2002 Steven J. Hill <sjhill@realitydiluted.com>
 *
 */
.text
.align	2
.globl	_dl_linux_resolve
.type	_dl_linux_resolve,@function
.ent	_dl_linux_resolve
_dl_linux_resolve:
	.frame	$29, 40, $31
	.set noreorder
	move	$3, $28		# Save GP
	addu	$25, 8		# t9 ($25) now points at .cpload instruction
	.cpload	$25		# Compute GP
	.set reorder
	subu	$29, 40
	.cprestore 32
	sw	$15, 36($29)
	sw	$4, 16($29)
	sw	$5, 20($29)
	sw	$6, 24($29)
	sw	$7, 28($29)
	move	$4, $24
	move	$5, $3
	jal     _dl_linux_resolver
	lw	$31, 36($29)
	lw	$4, 16($29)
	lw	$5, 20($29)
	lw	$6, 24($29)
	lw	$7, 28($29)
	addu	$29, 40
	move	$25, $2
	jr	$25
.size _dl_linux_resolve,.-_dl_linux_resolve
.end _dl_linux_resolve
