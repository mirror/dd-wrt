/*
 * irqs.h:
 *         IRQ numbers for the dsc21.
 * copyright:
 *         (C) 2001 RidgeRun, Inc. (http://www.ridgerun.com)
 *	   Gordon McNutt <gmcnutt@ridgerun.com>
 *
 * 09 Sep 2001 - C Hanish Menon [www.hanishkvc.com]
 *   - Copied and updated to suite armnommu/swarm
 * 
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
#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

/* irq #s */
#define SWARM_IRQ_TIMER0  26
#define SWARM_IRQ_TIMER1  27
#define SWARM_IRQ_TIMER2  28
#define SWARM_IRQ_TIMER3  29
#define SWARM_IRQ_UART0   24
#define SWARM_IRQ_MAX     29

#define NR_IRQS           (SWARM_IRQ_MAX + 1)

/* alias */
#define IRQ_TIMER         SWARM_IRQ_TIMER0

/* flags for request_irq() */
#define IRQ_FLG_LOCK      (0x0001)  /* handler is not replaceable   */
#define IRQ_FLG_REPLACE   (0x0002)  /* replace existing handler     */
#define IRQ_FLG_FAST      (0x0004)
#define IRQ_FLG_SLOW      (0x0008)
#define IRQ_FLG_STD       (0x8000)  /* internally used              */

#endif /* __ASM_ARCH_IRQS_H__ */
