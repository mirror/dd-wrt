/*
 * iic.h
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the I2C bus controller definitions
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */
 
#ifndef __ASM_ARCH_IIC_H
#define __ASM_ARCH_IIC_H

#define IICCON		0xF00F0000	/* Control status R	*/
#define IICCON_BF	0x01		/* Buffer flag		*/
#define IICCON_IEN	0x02		/* Interrupt enable	*/
#define IICCON_LRB	0x04		/* Last received bit 	*/
#define IICCON_ACK	0x08		/* Acknowlege enable	*/
#define IICCON_START	0x10		/* Start condition	*/
#define IICCON_STOP	0x20		/* Stop condition	*/
#define IICCON_RESTART	0x30		/* Repeat Start(Restart)*/
#define IICCON_BUSY	0x40		/* Bus busy		*/
#define IICCON_RESET	0x80		/* Reset		*/

#define IICBUF		0xF00F0004	/* Shift buffer R	*/
#define IICBUF_WRITE	0x00		/* Slave Addr Write bit	*/
#define IICBUF_READ	0x01		/* Slave Addr Read bit	*/

#define IICPS		0xF00F0008	/* Prescaler R		*/
#define IICCNT		0xF00F000C	/* Prescaler counter R	*/
#define IICPND		0xF00F0010	/* Interrupt pending R	*/

#endif
