/** 
 * @file IxUART_p.h
 * @author Intel Corporation
 * @date 12-OCT-01
 *
 * @brief Private header for the Intel ixp425 internal UART, generic driver.
 * 
 * Design Notes:
 *    
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
*/

/**
 * @defgroup IxUART_p IxUART_p
 *
 * @brief IXP400 UARTAcc Driver Private API
 * 
 * @{
 */

#define IX_UART_REG_DELTA 4	/* registers are 32bit wide = x4 spread */

/* Register offsets from base address */

#define IX_RBR	0x00	/* receiver buffer register (read only) */
#define IX_THR	0x00	/* transmit holding register (write only) */
#define IX_IER	0x01 	/* interrupt enable register (r/w) */
#define IX_IIR	0x02	/* interrupt identification register (read only) */
#define IX_FCR	0x02	/* FIFO control register (write only) */
#define IX_LCR	0x03	/* line control register (r/w) */
#define IX_MCR	0x04 	/* modem control register (r/w) */
#define IX_LSR	0x05	/* line status register (read only) */
#define IX_MSR	0x06	/* modem status register (read only) */
#define IX_SCR	0x07	/* scratch pad register (r/w) */
#define IX_DLL	0x00	/* divisor latch lower byte (r/w) */
#define IX_DLM	0x01	/* divisor latch upper bytes (r/w) */


/* Line Control Register */

#define IX_CHAR_LEN_5	0x00	/* 5bit data length (default) */
#define IX_CHAR_LEN_6	0x01	/* 6bit data length */
#define IX_CHAR_LEN_7	0x02	/* 7bit data length */
#define IX_CHAR_LEN_8	0x03	/* 8bit data length */
#define IX_LCR_STB_1	0x00	/* 1 stop bit */
#define IX_LCR_STB_2	0x04	/* 2 stop bits */
#define IX_LCR_PEN	0x08	/* parity enable */
#define IX_LCR_PDIS	0x00	/* parity disable */
#define IX_LCR_EPS	0x10	/* even parity select */
#define IX_LCR_STCKP	0x20	/* sticky parity select */
#define IX_LCR_SBRK	0x40	/* set break control */
#define IX_LCR_DLAB	0x80	/* divisor latch access enable */


/* Line Status Register */

#define IX_LSR_DR	0x01	/* data ready */
#define IX_LSR_OE	0x02	/* overrun error */
#define IX_LSR_PE	0x04	/* parity error */
#define IX_LSR_FE	0x08	/* framing error */
#define IX_LSR_BI	0x10	/* break interrupt */
#define IX_LSR_TDRQ	0x20	/* transmit data request */
#define IX_LSR_TEMT	0x40	/* transmitter empty */
#define IX_LSR_FIFOE	0x80	/* in FIFO mode, set when PE, FE or BI error */


/* Interrupt Enable Register */

#define IX_IER_RAVIE	0x01	/* received data available int enable */
#define IX_IER_TIE	0x02	/* transmitter data request int enable */
#define IX_IER_RLSE	0x04	/* receiver line status int enable */
#define IX_IER_MIE	0x08	/* modem status int enable */
#define IX_IER_RTOIE	0x10	/* receiver timeout int enable */
#define IX_IER_NRZIE	0x20	/* NZR coding enable */
#define IX_IER_UUE	0x40	/* UART Unit enable */
#define IX_IER_DMAE	0x80	/* DMA Requests enable */


/* Interrupt Identification Register */

#define IX_IIR_IP	0x00	/* interrupt pending */
#define IX_IIR_RLS	0x06	/* received line status int */
#define IX_IIR_RDA	0x04	/* received data available int */
#define IX_IIR_TIMEOUT	0x0C	/* receive data timeout (FIFO mode only) */
#define IX_IIR_THRE	0x02	/* transmit data request int */
#define IX_IIR_MSTAT	0x00	/* modem status int */
#define IX_IIR_FIFOES	0xC0	/* FIFO mode enable status */


/* Modem Control Register */

#define IX_MCR_RTS	0x02	/* request to send output */
#define IX_MCR_OUT1	0x04	/* output #1 */
#define IX_MCR_OUT2	0x08	/* output #2 */
#define IX_MCR_LOOP	0x10	/* loopback mode enable */


/* Modem Status Register */

#define IX_MSR_DCTS	0x01	/* clear to send change */
#define IX_MSR_CTS	0x10	/* complement of cts input */


/* FIFO Control Register */

#define IX_FCR_TRFIFOE	0x01	/* enable xmit and rcvr FIFOs */
#define IX_FCR_RESETRF	0x02	/* reset receiver FIFO */
#define IX_FCR_RESETTF	0x04	/* reset trasmitter FIFO */
#define IX_FCR_ITL_1	0x00	/* 1byte receiver trigger level */
#define IX_FCR_ITL_8	0x40	/* 8byte receiver trigger level */
#define IX_FCR_ITL_16	0x80	/* 16byte receiver trigger level */
#define IX_FCR_ITL_32	0xC0	/* 32byte receiver trigger level */

/**
 * @} defgroup IxUART_p
 */
