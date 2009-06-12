/*
 * irq.h:
 *         netarm-specific IRQ setup.
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
 * tried to merge some of Joe deBlaquiere's code in here --rp
 */

#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>
#include <asm/arch/netarm_registers.h>

#define fixup_irq(x) (x)

/* defined in arch/armnommu/mach-netarm/irq.c: */
extern void netarm_mask_irq(unsigned int irq);
extern void netarm_unmask_irq(unsigned int irq);
extern void netarm_mask_ack_irq(unsigned int irq);

/* Mask & Acknowledge function for Timer 1 IRQ: */
extern void netarm_mask_ack_t1_irq(unsigned int irq);

/* Mask & Acknowledge function for Timer 2 IRQ: */
extern void netarm_mask_ack_t2_irq(unsigned int irq);

/* Mask & Acknowledge function for Port C Bit 0..3 IRQ */
extern void netarm_mask_ack_portc_irq(unsigned int irq);

/* Mask & Acknowledge function for DMA channel 1 IRQ */
extern void netarm_mask_ack_dma1_irq(unsigned int irq);

/* Mask & Acknowledge function for DMA channels 2..10 IRQ */
extern void netarm_mask_ack_dmax_irq(unsigned int irq);

/* setup IRQ descriptor array */
extern void irq_init_irq(void);

#endif /* __ASM_ARCH_IRQ_H__ */

