/*
 * des.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc. 
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the DES/3DES engine definitions 
 * of the S3C2500 RISC microcontroller
 * based on the Samsung's "S3C2500 32-bit RISC
 * microcontroller pre. User's Manual"
 */

#ifndef __ASM_ARCH_DES_H
#define __ASM_ARCH_DES_H

#define DES_BASE	0xf0090000

#define DESCON		0xf0090000	/* DES/3DES control (R/W)	      */
#define DESSTA		0xf0090004	/* DES/3DES status (R)		      */
#define DESINT		0xf0090008	/* DES/3DES interrupt enable (R/W)    */
#define DESRUN		0xf009000c	/* DES/3DES run enable (W)	      */
#define DESKEY1L	0xf0090010	/* DES/3DES Key 1 left half (R/W)     */
#define DESKEY1R	0xf0090014	/* DES/3DES Key 1 right half (R/W)    */
#define DESKEY2L	0xf0090018	/* DES/3DES Key 2 left half (R/W)     */
#define DESKEY2R	0xf009001c	/* DES/3DES Key 2 right half (R/W)    */
#define DESKEY3L	0xf0090020	/* DES/3DES Key 3 left half (R/W)     */
#define DESKEY3R	0xf0090024	/* DES/3DES Key 3 right half (R/W)    */
#define DESIVL		0xf0090028	/* DES/3DES IV left half (R/W)	      */
#define DESIVR		0xf009002c	/* DES/3DES IV right half (R/W)	      */
#define DESINFIFO	0xf0090030	/* DES/3DES input FIFO (W)	      */
#define DESOUTFIFO	0xf0090034	/* DES/3DES output FIFO (R)	      */

/*---------------------------------------< DES/3DES control (R/W) >-----------*/
#define	DES_RUN		0x00000001
#define	DES_INDMA	0x00000002
#define	DES_OUTDMA	0x00000004
#define	DES_R2LDATA	0x00000008
#define	DES_DECRYPT	0x00000010
#define	DES_3DES	0x00000020
#define	DES_CBC		0x00000040
#define	DES_2W		0x00000080
#define	DES_TEST	0x00000100
#define	DES_RESET	0x00000200

/*---------------------------------------< DES/3DES status (R) >--------------*/
#define	DES_IDLE	0x00000001
#define	DES_INAVAILABLE	0x00000010
#define	DES_INEMPTY	0x00000020
#define	DES_INFULL	0x00000040
#define	DES_OUTVALID	0x00000100
#define	DES_OUTEMPTY	0x00000200
#define	DES_OUTFULL	0x00000400

/*---------------------------------------< DES/3DES interrupt enable (R/W) >--*/
#define	DES_IDLE_IE		0x00000001
#define	DES_INAVAILABLE_IE	0x00000010
#define	DES_OUTVALID_IE		0x00000100


#endif /* _ASM_ARCH_DES_H */
