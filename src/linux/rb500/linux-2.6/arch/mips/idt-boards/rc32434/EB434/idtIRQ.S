/**************************************************************************
 *
 *  BRIEF MODULE DESCRIPTION
 *     Intterrupt dispatcher code for IDT boards
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
 * May 2004 rkt, neb
 *
 * Initial Release
 *
 * 
 *
 **************************************************************************
 */
		
	
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

	.text
	.set	noreorder
	.set	noat
	.align	5
	NESTED(idtIRQ, PT_SIZE, sp)
	.set noat
	SAVE_ALL
	CLI

	.set	at
	.set	noreorder

	/* Get the pending interrupts */
	mfc0    t0, CP0_CAUSE
	nop
			 
	/* Isolate the allowed ones by anding the irq mask */
	mfc0    t2, CP0_STATUS
	move	a1, sp		/* need a nop here, hence we anticipate */
	andi    t0, CAUSEF_IP
	and     t0, t2
								  
	/* check for r4k counter/timer IRQ. */
	
	andi    t1, t0, CAUSEF_IP7
	beqz    t1, 1f
	nop

	jal     idt_timer_interrupt	

	li	a0, 7

	j	ret_from_irq
	nop
1:
	jal	rc32434_irqdispatch
	move	a0, t0
	j	ret_from_irq
	nop

	END(idtIRQ)


