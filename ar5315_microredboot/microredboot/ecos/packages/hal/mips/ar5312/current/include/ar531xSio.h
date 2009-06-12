/* ar531xSio.h - Atheros AR531X (ns16550 core) DUART header file */

/* Copyright 1984-1993 Wind River Systems, Inc. */

/*
 *  Copyright © 2001 Atheros Communications, Inc.,  All Rights Reserved.
 */

#ifndef __INCar531XSioh
#define __INCar531XSioh

/*********************************************************************
 *              Copyright (c) 1990,1991 Intel Corporation
 *
 * Intel hereby grants you permission to copy, modify, and
 * distribute this software and its documentation.  Intel grants
 * this permission provided that the above copyright notice
 * appears in all copies and that both the copyright notice and
 * this permission notice appear in supporting documentation.  In
 * addition, Intel grants this permission provided that you
 * prominently mark as not part of the original any modifications
 * made to this software or documentation, and that the name of
 * Intel Corporation not be used in advertising or publicity
 * pertaining to distribution of the software or the documentation
 * without specific, written prior permission.
 *
 * Intel Corporation does not warrant, guarantee or make any
 * representations regarding the use of, or the results of the use
 * of, the software and documentation in terms of correctness,
 * accuracy, reliability, currentness, or otherwise; and you rely
 * on the software, documentation and results solely at your own risk.
 *********************************************************************/

/*
 * Register offsets from base address (in 32b words).
 */
#define RBR             0x00    /* receiver buffer register */
#define THR             0x00    /* transmit holding register */
#define DLL             0x00    /* divisor latch */
#define IER             0x01    /* interrupt enable register */
#define DLM             0x01    /* divisor latch(MS) */
#define IIR             0x02    /* interrupt identification register */
#define FCR             0x02    /* FIFO control register */
#define LCR             0x03    /* line control register */
#define MCR             0x04    /* modem control register */
#define LSR             0x05    /* line status register */
#define MSR             0x06    /* modem status register */
#define SCR             0x07    /* scratch register */

/*
 * Line Control Register
 */
#define CHAR_LEN_5      0x00    /* 5bits data size */
#define CHAR_LEN_6      0x01    /* 6bits data size */
#define CHAR_LEN_7      0x02    /* 7bits data size */
#define CHAR_LEN_8      0x03    /* 8bits data size */
#define LCR_STB         0x04    /* 2 stop bits */
#define ONE_STOP        0x00    /* one stop bit */
#define LCR_PEN         0x08    /* parity enable */
#define PARITY_NONE     0x00    /* parity disable */
#define LCR_EPS         0x10    /* even parity select */
#define LCR_SP          0x20    /* stick parity select */
#define LCR_SBRK        0x40    /* break control bit */
#define LCR_DLAB        0x80    /* divisor latch access enable */

/*
 * Line Status Register
 */
#define LSR_DR          0x01    /* data ready */
#define LSR_OE          0x02    /* overrun error */
#define LSR_PE          0x04    /* parity error */
#define LSR_FE          0x08    /* framing error */
#define LSR_BI          0x10    /* break interrupt */
#define LSR_THRE        0x20    /* transmit holding register empty */
#define LSR_TEMT        0x40    /* transmitter empty */
#define LSR_FERR        0x80    /* in fifo mode, set when PE,FE or BI error */

/*
 * Interrupt Identification Register
 */
#define IIR_IP          0x01    /* no pending interrupts */
#define IIR_ID          0x0e
#define IIR_RLS         0x06    /* received line status */
#define IIR_RDA         0x04    /* received data available */
#define IIR_THRE        0x02    /* transmit holding register empty */
#define IIR_MSTAT       0x00    /* modem status */
#define IIR_TIMEOUT     0x0c    /* char receiv tiemout */

/*
 * Interrupt Enable Register
 */
#define IER_ERDAI       0x01    /* received data avail. & timeout int */
#define IER_ETHREI      0x02    /* transmitter holding register empty int */
#define IER_ELSI        0x04    /* receiver line status int enable */
#define IER_EMSI        0x08    /* modem status int enable */

/*
 * Modem Control Register
 */
#define MCR_DTR         0x01    /* dtr output */
#define MCR_RTS         0x02    /* rts output */
#define MCR_OUT1        0x04    /* output #1 */
#define MCR_OUT2        0x08    /* output #2 */
#define MCR_LOOP        0x10    /* loopback enable */

/*
 * Modem Status Register
 */
#define MSR_DCTS        0x01    /* cts change */
#define MSR_DDSR        0x02    /* dsr change */
#define MSR_TERI        0x04    /* ring indicator change */
#define MSR_DDCD        0x08    /* data carrier indicator change */
#define MSR_CTS         0x10    /* complement of cts */
#define MSR_DSR         0x20    /* complement of dsr */
#define MSR_RI          0x40    /* complement of ring signal */
#define MSR_DCD         0x80    /* complement of dcd */

/*
 * FIFO Control Register
 */
#define FCR_EN          0x01    /* enable xmit and rcvr */
#define FCR_RXCLR       0x02    /* clears rcvr fifo */
#define FCR_TXCLR       0x04    /* clears xmit fifo */
#define FCR_DMA         0x08    /* dma */
#define FCR_RXTRIG_1    0x00    /* rcvr fifo trigger at 1 byte */
#define FCR_RXTRIG_4    0x40    /* rcvr fifo trigger at 4 bytes */
#define FCR_RXTRIG_8    0x80    /* rcvr fifo trigger at 8 bytes */
#define FCR_RXTRIG_14   0xc0    /* rcvr fifo trigger at X4 bytes */

#define FIFO_DEPTH      16

#define UARTDMA_TXSIZE  (8 * _CACHE_ALIGN_SIZE)
#define UARTDMA_RXSIZE  (2 * _CACHE_ALIGN_SIZE)
#define UARTDMA_ALIGN   _CACHE_ALIGN_SIZE


#endif /* __INCar531XSioh */
