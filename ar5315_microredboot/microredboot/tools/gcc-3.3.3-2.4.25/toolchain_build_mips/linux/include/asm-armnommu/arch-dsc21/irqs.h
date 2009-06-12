/*
 * irqs.h:
 *         IRQ numbers for the dsc21.
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
#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

/* irq #s */
#define DSC21_IRQ_EXT0    0x00
#define DSC21_IRQ_EXT1    0x01
#define DSC21_IRQ_EXT2    0x02
#define DSC21_IRQ_EXT3    0x03
#define DSC21_IRQ_EXT4    0x04
#define DSC21_IRQ_EXT5    0x05
#define DSC21_IRQ_EXT6    0x06
#define DSC21_IRQ_EXT7    0x07
#define DSC21_IRQ_DMA     0x08
#define DSC21_IRQ_FLASH   0x09
#define DSC21_IRQ_IRDA_RX 0x0a
#define DSC21_IRQ_IRDA_TX 0x0b
#define DSC21_IRQ_TIMER0  0x10
#define DSC21_IRQ_TIMER1  0x11
#define DSC21_IRQ_TIMER2  0x12
#define DSC21_IRQ_TIMER3  0x13
#define DSC21_IRQ_CCDVD0  0x14
#define DSC21_IRQ_CCDVD1  0x15
#define DSC21_IRQ_CCDVD2  0x16
#define DSC21_IRQ_ENCVD   0x17
#define DSC21_IRQ_SERIAL0 0x18
#define DSC21_IRQ_SERIAL1 0x19
#define DSC21_IRQ_DSP     0x1a
#define DSC21_IRQ_UART0   0x1b
#define DSC21_IRQ_UART1   0x1c
#define DSC21_IRQ_USB     0x1d
#define DSC21_IRQ_BURST   0x1e
#define DSC21_IRQ_CARD    0x1f
#define DSC21_IRQ_EXB     0x1f /* conflict? */

#define NR_IRQS           (DSC21_IRQ_EXB + 1)

/* alias */
#define IRQ_TIMER         DSC21_IRQ_TIMER0

/* flags for request_irq() */
#define IRQ_FLG_LOCK      (0x0001)  /* handler is not replaceable   */
#define IRQ_FLG_REPLACE   (0x0002)  /* replace existing handler     */
#define IRQ_FLG_FAST      (0x0004)
#define IRQ_FLG_SLOW      (0x0008)
#define IRQ_FLG_STD       (0x8000)  /* internally used              */

#endif /* __ASM_ARCH_IRQS_H__ */
