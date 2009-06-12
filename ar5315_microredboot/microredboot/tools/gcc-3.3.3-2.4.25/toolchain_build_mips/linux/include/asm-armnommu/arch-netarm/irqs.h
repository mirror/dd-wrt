/*
 * irqs.h:
 *         IRQ numbers for the netarm.
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
 * author(s) : Joe deBlaquiere, rp
 */

#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

#define IRQ_PORTC0	0
#define IRQ_PORTC1	1
#define IRQ_PORTC2	2
#define IRQ_PORTC3	3
#define IRQ_TIMER2	4
#define IRQ_TIMER1	5
#define IRQ_WDOG	6
#define IRQ_SER2_TX	12
#define IRQ_SER2_RX	13
#define IRQ_SER1_TX	14
#define IRQ_SER1_RX	15
#define IRQ_ENET_TX	16
#define IRQ_ENET_RX	17
#define IRQ_ENIP4	18
#define IRQ_ENIP3	19
#define IRQ_ENIP2	20
#define IRQ_ENIP1	21
#define IRQ_DMA10	22	
#define IRQ_DMA09	23	
#define IRQ_DMA08	24	
#define IRQ_DMA07	25	
#define IRQ_DMA06	26	
#define IRQ_DMA05	27	
#define IRQ_DMA04	28	
#define IRQ_DMA03	29	
#define IRQ_DMA02	30	
#define IRQ_DMA01	31	

#define NR_IRQS           (IRQ_DMA01 + 1)

/* alias */
/* #define IRQ_TIMER         DSC21_IRQ_TIMER0 */

/* flags for request_irq() */
#define IRQ_FLG_LOCK      (0x0001)  /* handler is not replaceable   */
#define IRQ_FLG_REPLACE   (0x0002)  /* replace existing handler     */
#define IRQ_FLG_FAST      (0x0004)
#define IRQ_FLG_SLOW      (0x0008)
#define IRQ_FLG_STD       (0x8000)  /* internally used              */

#endif /* __ASM_ARCH_IRQS_H__ */

