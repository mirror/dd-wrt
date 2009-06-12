/*
 * irq.h:
 *         dsc21-specific IRQ setup.
 * copyright:
 *         (C) 2001 RidgeRun, Inc. (http://www.ridgerun.com)
 * author: Gordon McNutt <gmcnutt@ridgerun.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 * WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 * USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You should have received a copy of the  GNU General Public License along
 * with this program; if not, write  to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>

#define fixup_irq(x) (x)

extern void dsc21_mask_irq(unsigned int irq);
extern void dsc21_unmask_irq(unsigned int irq);
extern void dsc21_mask_ack_irq(unsigned int irq);

static __inline__ void irq_init_irq(void)
{
	int irq;

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= dsc21_mask_ack_irq;
		irq_desc[irq].mask	= dsc21_mask_irq;
		irq_desc[irq].unmask	= dsc21_unmask_irq;
	}

	/* clear all status */
	outw(0xffff, DSC21_IRQ0_STATUS);
	outw(0xffff, DSC21_IRQ1_STATUS);
    
	/* disable all */
	outw(0, DSC21_FIQ_ENABLE);
	outw(0, DSC21_IRQ0_ENABLE);
	outw(0, DSC21_IRQ1_ENABLE);
}

#endif /* __ASM_ARCH_IRQ_H__ */
