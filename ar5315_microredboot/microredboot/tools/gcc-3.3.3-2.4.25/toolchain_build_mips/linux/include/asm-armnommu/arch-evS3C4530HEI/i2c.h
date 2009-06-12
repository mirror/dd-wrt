/*
 * iic.h
 *
 * Copyright (C) 2002 Arcturus Networks Inc.
 * by Oleksandr Zhadan <oleks@arcturusnetworks.com>
 *
 * This file includes the I2C bus controller definitions
 * of the S3C4530A RISC microcontroller
 * based on the Samsung's "S3C4530A 32-bit RISC
 * Microprocessor Revision 1  User's Manual"
 */
 
#ifndef __ASM_ARCH_IIC_H
#define __ASM_ARCH_IIC_H

#define IICCON		0x3FFF000	/* Control status R	*/
#define IICCON_BF	0x01		/* Buffer flag		*/
#define IICCON_IEN	0x02		/* Interrupt enable	*/
#define IICCON_LRB	0x04		/* Last received bit 	*/
#define IICCON_ACK	0x08		/* Acknowlege enable	*/
#define IICCON_START	0x10		/* Start condition	*/
#define IICCON_STOP	0x20		/* Stop condition	*/
#define IICCON_RESTART	0x30		/* Repeat Start(Restart)*/
#define IICCON_BUSY	0x40		/* Bus busy		*/
#define IICCON_RESET	0x80		/* Reset		*/

#define IICBUF		0x3FFF004	/* Shift buffer R	*/
#define IICBUF_WRITE	0x00		/* Slave Addr Write bit	*/
#define IICBUF_READ	0x01		/* Slave Addr Read bit	*/

#define IICPS		0x3FFF008	/* Prescaler R		*/
#define IICCNT		0x3FFF00C	/* Prescaler counter R	*/

#endif
