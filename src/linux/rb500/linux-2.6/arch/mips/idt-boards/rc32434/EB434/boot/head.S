/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *   Board initialisation code.
 *
 *  Copyright 2004 IDT Inc. (rischelp@idt.com)
 *         
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 **************************************************************************
 * May 2004 rkt
 *
 * Initial release based on Johannes Stezenbach's (js@convergence.de) code.
 *
 * 
 *
 **************************************************************************
 */
	
#include <linux/config.h>
#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/cacheops.h>
#include <asm/mipsregs.h>
#include <asm/offset.h>
#include <asm/cachectl.h>
#include <asm/regdef.h>

#define IndexInvalidate_I       0x00

	.set noreorder
	.cprestore
	LEAF(zstartup)
zstartup:

        la      sp, .stack
	move	s0, a0
	move	s1, a1
	move	s2, a2
	move	s3, a3

	/* Clear BSS */
	/* Note: when zImage is in ROM, _edata and _bss point to
	 * ROM space even when using -Tbss on the linker command line;
	 * maybe ld.script needs to be corrected.
	 */
	la	a0, .stack
	la	a2, _end
1:	sw	zero, 0(a0)
	bne	a2, a0, 1b
	addu	a0, 4

	/* flush the I-Cache */
	li	k0, 0x80000000  # start address
	li	k1, 0x80004000  # end address (16KB I-Cache)
	subu	k1, 128

2:
	.set mips3
	cache   IndexInvalidate_I, 0(k0)
	cache   IndexInvalidate_I, 16(k0)
	cache   IndexInvalidate_I, 32(k0)
	cache   IndexInvalidate_I, 48(k0)
	cache   IndexInvalidate_I, 64(k0)
	cache   IndexInvalidate_I, 80(k0)
	cache   IndexInvalidate_I, 96(k0)
	cache   IndexInvalidate_I, 112(k0)
	.set mips0

	bne	k0, k1, 2b
	addu	k0, k0, 128
	/* done */

	/* flush the D-Cache */
	li	k0, 0x80000000  # start address
	li	k1, 0x80004000  # end address (16KB I-Cache)
	subu	k1, 128

2:
	.set mips3
	cache   Index_Writeback_Inv_D, 0(k0)
	cache   Index_Writeback_Inv_D, 16(k0)
	cache   Index_Writeback_Inv_D, 32(k0)
	cache   Index_Writeback_Inv_D, 48(k0)
	cache   Index_Writeback_Inv_D, 64(k0)
	cache   Index_Writeback_Inv_D, 80(k0)
	cache   Index_Writeback_Inv_D, 96(k0)
	cache   Index_Writeback_Inv_D, 112(k0)
	.set mips0

	bne	k0, k1, 2b
	addu	k0, k0, 128
	/* done */

	la	ra, 3f
	la	k0, decompress_kernel
	jr	k0
	nop
3:

	move	a0, s0
	move	a1, s1
	move	a2, s2
	move	a3, s3
	li	k0, KERNEL_ENTRY
	jr	k0
	nop
4:
	b 4b
	END(zstartup)

	.bss
	.fill 0x2000
	EXPORT(.stack)
