/*
 * Rev:$Id: bf533_serial.h,v 1.1 2003/11/04 14:47:42 bas Exp $
 *
 * Copyright (C) 2003 Bas Vermeulen/Buyways B.V. <bf533@buyways.nl>
 *
 * Created by Buyways B.V. for Astent <info@astent.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */
#ifndef _BF533_SERIAL_H_
#define _BF533_SERIAL_H_
/* 
 *  ADI uart macro definition :
 *     UART 0 CONTROLLER REGISTERS  (0XFFC01800 - 0XFFC01BFF)
 *     UART 1 CONTROLLER REGISTERS  (0XFFC01C00 - 0XFFC01FFF)
 */
#define UART_THR_ADDR		0xffc00400  /* UART 0 Transmit holding register
					       16 bit */
#define UART_THR(idx)		HALFWORD_REF(UART_THR_ADDR)


#define UART_RBR_ADDR		0xffc00400  /* UART 0 Receive buffer register  
					     16 bit */
#define UART_RBR(idx)		HALFWORD_REF(UART_RBR_ADDR)

#define UART_DLL_ADDR		0xffc00400  /* UART 0 Divisor latch (low byte) 
					       register  16 bit */
#define UART_DLL(idx)		HALFWORD_REF(UART_DLL_ADDR)

#define UART_IER_ADDR		0xffc00404  /* UART 0 Interrupt enable register  16 bit */
#define UART_IER(idx)		HALFWORD_REF(UART_IER_ADDR)
#define UART_IER_ERBFI		0x01    /* Enable Receive Buffer Full Interrupt
					   (DR bit) */
#define UART_IER_ETBEI		0x02    /* Enable Transmit Buffer Empty 
					   Interrupt(THRE bit) */
#define UART_IER_ELSI		0x04    /* Enable RX Status Interrupt
					   (gen if any of LSR[4:1] set) */
#define UART_IER_EDDSI		0x08    /* Enable Modem Status Interrupt(gen if any UARTx_MSR[3:0] set) */

#define UART_DLH_ADDR		0xffc00404  /* UART 0 Divisor latch (high byte) register  16 bit */
#define UART_DLH(idx)		HALFWORD_REF(UART_DLH_ADDR)
#define UART_IIR_ADDR		0xffc00408  /* UART 0 Interrupt identification register  16 bit */
#define UART_IIR(idx)		HALFWORD_REF(UART_IIR_ADDR)
#define UART_IIR_NOINT		0x01    /* Bit0: cleared when no interrupt */
#define UART_IIR_STATUS		0x06    /* mask bit for the status: bit2-1 */
#define UART_IIR_LSR		0x06    /* Receive line status */
#define UART_IIR_RBR		0x04    /* Receive data ready */
#define UART_IIR_THR		0x02    /* Ready to transmit  */
#define UART_IIR_MSR		0x00    /* Modem status       */

#define UART_LCR_ADDR          	0xffc0040C  /* UART 0 Line control register  16 bit */
#define UART_LCR(idx)           	HALFWORD_REF(UART_LCR_ADDR)
#define UART_LCR_WLS5           0       /* word length 5 bits */
#define UART_LCR_WLS6           0x01    /* word length 6 bits */
#define UART_LCR_WLS7           0x02    /* word length 7 bits */
#define UART_LCR_WLS8           0x03    /* word length 8 bits */
#define UART_LCR_STB            0x04    /* StopBit: 1: 2 stop bits for 
					   non-5-bit word length 1/2 stop bits 
					   for 5-bit word length 0: 
					   1 stop bit */
#define UART_LCR_PEN            0x08    /* Parity Enable 1: for enable */
#define UART_LCR_EPS            0x10    /* Parity Selection: 
					   1: for even pariety
                                           0: odd parity when PEN =1 & SP =0 */
#define UART_LCR_SP             0x20    /* Sticky Parity: */
#define UART_LCR_SB             0x40    /* Set Break: force TX pin to 0 */
#define UART_LCR_DLAB           0x80    /* Divisor Latch Access */


#define UART_MCR_ADDR          	0xffc00410  /* UART 0 Module Control register  
					       16 bit */
#define UART_MCR(idx)           	HALFWORD_REF(UART_MCR_ADDR)

#define UART_LSR_ADDR          	0xffc00414  /* UART 0 Line status register  
					       16 bit */
#define UART_LSR(idx)           	HALFWORD_REF(UART_LSR_ADDR)
#define UART_LSR_DR             0x01    /* Data Ready */
#define UART_LSR_OE             0x02    /* Overrun Error */
#define UART_LSR_PE             0x04    /* Parity Error  */
#define UART_LSR_FE             0x08    /* Frame Error   */
#define UART_LSR_BI             0x10    /* Break Interrupt */
#define UART_LSR_THRE           0x20    /* THR empty, REady to accept */
#define UART_LSR_TEMT           0x40    /* TSR and UARTx_thr both empty */

#define UART_MSR_ADDR          	0xffc00418  /* UART 0 Modem status register  16 bit */
#define UART_MSR(idx)           	HALFWORD_REF(UART_MSR_ADDR)
#define UART_SCR_ADDR          	0xffc00418  /* UART 0 Scratch register  16 bit */
#define UART_SCR(idx)           	HALFWORD_REF(UART_SCR_ADDR)
#define UART_GCTL_ADDR		0xffc00424
#define UART_GCTL(idx)		HALFWORD_REF(UART_GCTL_ADDR)
#define UART_GCTL_UCEN		0x01
#endif /* _BF533_SERIAL_H_ */

