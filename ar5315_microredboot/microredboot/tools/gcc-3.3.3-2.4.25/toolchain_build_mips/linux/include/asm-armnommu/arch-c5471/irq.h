/*
 * linux/include/asm-armnommu/arch-c5471/irq.h
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

#ifndef _ASM_ARCH_IRQ_H_
#define _ASM_ARCH_IRQ_H_

#include <asm/arch/hardware.h>
#include <asm/mach/irq.h>

#define BUILD_IRQ(s,n,m)

struct pt_regs;

extern int IRQ_interrupt(int irq, struct pt_regs *regs);
extern int timer_IRQ_interrupt(int irq, struct pt_regs *regs);
extern int fast_IRQ_interrupt(int irq, struct pt_regs *regs);
extern int bad_IRQ_interrupt(int irq, struct pt_regs *regs);
extern int probe_IRQ_interrupt(int irq, struct pt_regs *regs);

extern void c5471_init_irq(void);
extern void c5471_mask_irq(unsigned int irq);
extern void c5471_unmask_irq(unsigned int irq);
extern void c5471_mask_ack_irq(unsigned int irq);

#define IRQ_interrupt0  timer_IRQ_interrupt
#define IRQ_interrupt1  IRQ_interrupt
#define IRQ_interrupt2  IRQ_interrupt
#define IRQ_interrupt3  IRQ_interrupt
#define IRQ_interrupt4  IRQ_interrupt
#define IRQ_interrupt5  IRQ_interrupt
#define IRQ_interrupt4  IRQ_interrupt
#define IRQ_interrupt6  IRQ_interrupt
#define IRQ_interrupt7  IRQ_interrupt
#define IRQ_interrupt8  IRQ_interrupt
#define IRQ_interrupt9  IRQ_interrupt
#define IRQ_interrupt10 IRQ_interrupt
#define IRQ_interrupt11 IRQ_interrupt
#define IRQ_interrupt12 IRQ_interrupt
#define IRQ_interrupt13 IRQ_interrupt
#define IRQ_interrupt14 IRQ_interrupt
#define IRQ_interrupt15 IRQ_interrupt
#define IRQ_interrupt16 IRQ_interrupt
#define IRQ_interrupt17 IRQ_interrupt
#define IRQ_interrupt18 IRQ_interrupt
#define IRQ_interrupt19 IRQ_interrupt
#define IRQ_interrupt20 IRQ_interrupt
#define IRQ_interrupt21 IRQ_interrupt
#define IRQ_interrupt22 IRQ_interrupt
#define IRQ_interrupt23 IRQ_interrupt
#define IRQ_interrupt24 IRQ_interrupt
#define IRQ_interrupt25 IRQ_interrupt
#define IRQ_interrupt26 IRQ_interrupt
#define IRQ_interrupt27 IRQ_interrupt
#define IRQ_interrupt28 IRQ_interrupt
#define IRQ_interrupt29 IRQ_interrupt
#define IRQ_interrupt30 IRQ_interrupt
#define IRQ_interrupt31 IRQ_interrupt

#define IRQ_INTERRUPT(n)   (void (*)(void))IRQ_interrupt##n
#define FAST_INTERRUPT(n)  (void (*)(void))fast_IRQ_interrupt
#define BAD_INTERRUPT(n)   (void (*)(void))bad_IRQ_interrupt
#define PROBE_INTERRUPT(n) (void (*)(void))probe_IRQ_interrupt

#define fixup_irq(x) (x)

static __inline__ void irq_init_irq(void)
{
  int irq;

  for (irq = 0; irq < NR_IRQS; irq++)
    {
      irq_desc[irq].valid	= 1;
      irq_desc[irq].probe_ok	= 1;
      irq_desc[irq].mask_ack	= c5471_mask_ack_irq;
      irq_desc[irq].mask	= c5471_mask_irq;
      irq_desc[irq].unmask	= c5471_unmask_irq;
    }

  c5471_init_irq();
}

#endif /* _ASM_ARCH_IRQ_H_ */
