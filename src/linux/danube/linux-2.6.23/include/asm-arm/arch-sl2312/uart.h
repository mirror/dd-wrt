/* *
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __UART_H
#define __UART_H

/*
 * Register definitions for the UART
 */

#define UART_TX_FIFO_SIZE      (15)

#define UART_RBR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x00))  // read
#define UART_THR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x00))  // write
#define UART_IER(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x04))
#define UART_IER_MS                                 (0x08)
#define UART_IER_RLS                                (0x04)
#define UART_IER_TE                                 (0x02)
#define UART_IER_DR                                 (0x01)
#define UART_IIR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x08))   // read
#define UART_IIR_NONE                		    (0x01)	/* No interrupt pending */
#define UART_IIR_RLS                 		    (0x06)	/* Receive Line Status */
#define UART_IIR_DR                  		    (0x04)	/* Receive Data Ready */
#define UART_IIR_TIMEOUT             		    (0x0c)	/* Receive Time Out */
#define UART_IIR_TE                  		    (0x02)	/* THR Empty */
#define UART_IIR_MODEM               		    (0x00)	/* Modem Status */
#define UART_FCR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x08))  // write
#define UART_FCR_FE                  		    (0x01) 	/* FIFO Enable */
#define UART_FCR_RXFR                		    (0x02) 	/* Rx FIFO Reset */
#define UART_FCR_TXFR               		    (0x04) 	/* Tx FIFO Reset */
#define UART_FCR_FIFO_1C                            (0x00)
#define UART_FCR_FIFO_4C                            (0x40)
#define UART_FCR_FIFO_8C                            (0x80)
#define UART_FCR_FIFO_14C                           (0xC0)
#define UART_LCR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x0C))
#define UART_LCR_MSK                                (0x03)
#define UART_LCR_LEN5                		    (0x00)
#define UART_LCR_LEN6                		    (0x01)
#define UART_LCR_LEN7                		    (0x02)
#define UART_LCR_LEN8                		    (0x03)
#define UART_LCR_STOP                		    (0x04)
#define UART_LCR_EVEN                		    (0x18) 	/* Even Parity */
#define UART_LCR_ODD                 		    (0x08)     	/* Odd Parity */
#define UART_LCR_PE                  		    (0x08)	/* Parity Enable */
#define UART_LCR_SETBREAK            		    (0x40)	/* Set Break condition */
#define UART_LCR_STICKPARITY         		    (0x20)	/* Stick Parity Enable */
#define UART_LCR_DLAB                		    (0x80)     	/* Divisor Latch Access Bit */
#define UART_MCR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x10))
#define UART_MCR_DTR                 		    (0x1)	/* Data Terminal Ready */
#define UART_MCR_RTS                 		    (0x2)	/* Request to Send */
#define UART_MCR_OUT1                		    (0x4)	/* output	1 */
#define UART_MCR_OUT2                		    (0x8)	/* output2 or global interrupt enable */
#define UART_MCR_LPBK                		    (0x10)	/* loopback mode */
#define UART_MCR_MASK                               (0xE3)
#define UART_LSR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x14))
#define UART_LSR_DR                  		    (0x01)     	/* Data Ready */
#define UART_LSR_OE                  		    (0x02)     	/* Overrun Error */
#define UART_LSR_PE                  		    (0x04)     	/* Parity Error */
#define UART_LSR_FE                  		    (0x08)     	/* Framing Error */
#define UART_LSR_BI                                 (0x10)     	/* Break Interrupt */
#define UART_LSR_THRE                               (0x20)     	/* THR Empty */
#define UART_LSR_TE                                 (0x40)     	/* Transmitte Empty */
#define UART_LSR_DE                                 (0x80)     	/* FIFO Data Error */
#define UART_MSR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x18))
#define UART_MSR_DELTACTS            		    (0x01)	/* Delta CTS */
#define UART_MSR_DELTADSR            		    (0x02)	/* Delta DSR */
#define UART_MSR_TERI                		    (0x04)	/* Trailing Edge RI */
#define UART_MSR_DELTACD             		    (0x08)	/* Delta CD */
#define UART_MSR_CTS                 		    (0x10)	/* Clear To Send */
#define UART_MSR_DSR                 		    (0x20)	/* Data Set Ready */
#define UART_MSR_RI                  		    (0x40)	/* Ring Indicator */
#define UART_MSR_DCD                 		    (0x80)	/* Data Carrier Detect */
#define UART_SPR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x1C))
#define UART_DIV_LO(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x0))
#define UART_DIV_HI(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x4))
#define UART_PSR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x8))
#define UART_MDR(BASE_ADDR) (UART_TYPE (BASE_ADDR  + 0x20))
#define UART_MDR_SERIAL                             (0x0)

#define UART_MSR_DDCD	0x08	/* Delta DCD */
#define UART_MSR_DDSR	0x02	/* Delta DSR */
#define UART_MSR_DCTS	0x01	/* Delta CTS */
#define UART_MSR_ANY_DELTA 0x0F	/* Any of the delta bits! */


#endif /* __UART_H */
