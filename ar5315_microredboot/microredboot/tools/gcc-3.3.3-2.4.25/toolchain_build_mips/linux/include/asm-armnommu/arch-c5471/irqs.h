/***********************************************************************
 * include/asm-armnommu/arch-c5471/irqs.h
 *
 *   Copyright (C) 2003 Cadenux, LLC. All rights reserved.
 *   todd.fischer@cadenux.com  <www.cadenux.com>
 *
 *   Copyright (C) 2001 RidgeRun, Inc. All rights reserved.
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
 ***********************************************************************/


#ifndef __ASM_ARCH_IRQS_H__
#define __ASM_ARCH_IRQS_H__

#include <linux/autoconf.h>

#define UCLINUX_IRQ_TIMER  C5471_IRQ_TIMER2 /* interrupt number */

#define C5471_IRQ_TIMER0         0
#define C5471_IRQ_WATCHDOG       0
#define C5471_IRQ_TIMER1         1
#define C5471_IRQ_TIMER2         2
#define C5471_IRQ_ETHER          3
#define C5471_IRQ_GPIO0          4
#define C5471_IRQ_KBGPIO_0_7     5
#define C5471_IRQ_UART           6
#define C5471_IRQ_UART_IRDA      7
#define C5471_IRQ_KBGPIO_8_15    8
#define C5471_IRQ_GPIO3          9
#define C5471_IRQ_GPIO2         10
#define C5471_IRQ_I2C           11
#define C5471_IRQ_GPIO1         12
#define C5471_IRQ_SPI           13
#define C5471_IRQ_GPIO_4_19     14
#define C5471_IRQ_API           15

#define IRQ_TIMER UCLINUX_IRQ_TIMER

/* flags for request_irq().  IRQ_FLG_STD is apparently a uClinux-ism,
 * so I'll keep it around even though it is just a mapping for the
 * real SA_INTERRUPT. -- skj 
 */

#define IRQ_FLG_STD SA_INTERRUPT

/*  If you want to use the watchdog timer, send in this
 *  as its interrupt flag
 */

#define FIQ_FORCE 0x2

#define INT_WD_TIMER		 0
#define INT_TIMER1		 1
#define INT_TIMER2		 2
#define INT_GPIO0		 3
#define INT_ETH			 4
#define INT_KBGPIO_0_7		 5
#define INT_UART		 6
#define INT_UART_IRDA		 7
#define INT_KGPIO_8_15		 8
#define INT_GPIO3		 9
#define INT_GPIO2		10
#define INT_I2C			11
#define INT_GPIO1		12
#define INT_SPI			13
#define INT_GPIO_4_19		14
#define INT_API			15
#define NR_IRQS			INT_API+1

#endif /* __ASM_ARCH_IRQS_H__ */
