/***********************************************************************
 * linux/include/asm-arm/arch-c5471/serial.h
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
 * THIS  SOFTWARE  IS  PROVIDED  ``AS  IS''  AND   ANY  EXPRESS  OR IMPLIED
 * WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 * NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT,  INDIRECT,
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

#ifndef __ASM_ARCH_SERIAL_H
#define __ASM_ARCH_SERIAL_H

#include <linux/config.h>
#include <linux/termios.h>
#include <linux/tqueue.h>
#include <linux/circ_buf.h>
#include <linux/wait.h>

#include <asm/arch/hardware.h>
#include <asm/arch/irqs.h>

#define BASE_BAUD     115200
#define STD_COM_FLAGS (ASYNC_BOOT_AUTOCONF | ASYNC_SKIP_TEST)

/* This defines the initializer for a struct serial_state describing the
 * C5471 serial IRDA port.
 */

#define C5471_IRDA_SERIAL_PORT_DEFN \
  {  \
    magic:          0, \
    port:           0, \
    flags:          STD_COM_FLAGS, \
    xmit_fifo_size: UART_IRDA_XMIT_FIFO_SIZE, \
    baud_base:      BASE_BAUD, \
    irq:            C5471_IRQ_UART_IRDA, \
    type:           PORT_UNKNOWN, \
    iomem_base:     UART_IRDA_BASE, \
    io_type:        SERIAL_IO_MEM \
  }

/* This defines the initializer for a struct serial_state describing the
 * C5471 serial MODEM port.
 */

#define C5471_MODEM_SERIAL_PORT_DEFN \
  {  \
    magic:          0, \
    port:           1, \
    flags:          STD_COM_FLAGS, \
    xmit_fifo_size: UART_XMIT_FIFO_SIZE, \
    baud_base:      BASE_BAUD, \
    irq:            C5471_IRQ_UART, \
    type:           PORT_UNKNOWN, \
    iomem_base:     UART_MODEM_BASE, \
    io_type:        SERIAL_IO_MEM \
  }

/* There will always be two serial ports:  The C5471 Modem UART and
 * the C5471 IrDA UART.
 */

#define RS_TABLE_SIZE 2

/* STD_SERIAL_PORT_DEFNS (plus EXTRA_SERIAL_PORT_DEFNS, see
 * include/asm/serial.h) define an array of struct serial_state
 * instance initializers.  These array initilizers are used
 * in drivers/char/serial_c5471.c to described the serial
 * ports that will be support.
 */

# define STD_SERIAL_PORT_DEFNS \
  C5471_MODEM_SERIAL_PORT_DEFN, /* ttyS0 */ \
  C5471_IRDA_SERIAL_PORT_DEFN   /* ttyS1 */ \

#define EXTRA_SERIAL_PORT_DEFNS

/* Events are used to schedule things to happen at timer-interrupt
 * time, instead of at rs interrupt time.
 */

#define RS_EVENT_WRITE_WAKEUP	0

#define CONFIGURED_SERIAL_PORT(info) ((info)->state)

#define SERIAL_MAGIC 0x5301
#define SSTATE_MAGIC 0x5302

/* This is the internal structure for each serial port's state.
 *
 * For definitions of the flags field, see tty.h
 */

struct c5471_serial_state_s
{
  int			magic;
  int                   port;
  int			baud_base;
  int			irq;
  int			flags;
  int			type;
  int			line;
  int			xmit_fifo_size;
  int			custom_divisor;
  int			count;
  int			io_type;
  unsigned long		iomem_base;
  unsigned short	close_delay;
  unsigned short	closing_wait; /* time to wait before closing */
  struct async_icount	icount;	
  struct termios	normal_termios;
  struct termios	callout_termios;
  struct c5471_async_s	*info;
};
typedef struct c5471_serial_state_s c5471_serial_state_t;

struct c5471_async_s
{
  int			magic;
  int                   port;
  int			flags;
  c5471_serial_state_t	*state;
  struct tty_struct 	*tty;
  unsigned short	closing_wait;
  int			line;
  int			timeout;
  int			quot;
  int			x_char;		/* xon/xoff character */
  int			close_delay;
  int			blocked_open;	/* # of blocked opens */
  unsigned long		event;
  unsigned long		last_active;
  long			session;	/* Session of opening process */
  long			pgrp;		/* pgrp of opening process */
  struct circ_buf	xmit;
  struct tq_struct	tqueue;
#ifdef DECLARE_WAITQUEUE
  wait_queue_head_t	open_wait;
  wait_queue_head_t	close_wait;
  wait_queue_head_t	delta_msr_wait;
#else	
  struct wait_queue	*open_wait;
  struct wait_queue	*close_wait;
  struct wait_queue	*delta_msr_wait;
#endif	
  struct c5471_async_s	*next_port; /* For the linked list */
  struct c5471_async_s	*prev_port;
};
typedef struct c5471_async_s c5471_async_t;

struct hw_reg_shadow {
  unsigned long		IER_cached;	/* Cached UART interrupt enable reg */
  unsigned long		LCR_cached;	/* Cached UART line control reg */
  unsigned long		FCR_cached;	/* Cached UART fifo control reg */
  unsigned long		EFR_cached;	/* Cached UART enhanced feature reg */
  unsigned long		TCR_cached;	/* Cached UART TX control reg */
};

#endif /* __ASM_ARCH_SERIAL_H */
