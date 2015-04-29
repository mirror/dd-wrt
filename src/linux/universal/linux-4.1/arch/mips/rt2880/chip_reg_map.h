
/******************************************************************************
*
* (c) Copyright 1996-2001, Palmchip Corporation
*
* This document is an unpublished work protected under the copyright laws
* of the United States containing the confidential, proprietary and trade
* secret information of Palmchip Corporation. This document may not be
* copied or reproduced in any form whatsoever without the express written
* permission of Palmchip Corporation.
*
*******************************************************************************
*
*  File Name: chip_reg_map.h
*     Author: Ian Thompson 
*
*    Purpose: Contains the chip register map of the PalmPak system.
*
*  Sp. Notes:
*
*
*******************************************************************************
*
* Revision History:
*
*      Date    Name  Comments
*    --------  ---   ------------------------------------
*    02/09/01  IST   Split from mem_map.h
*
*
*******************************************************************************/

#ifndef CHIP_REG_MAP_H
#define CHIP_REG_MAP_H

/*=====================*
 *  Include Files      *
 *=====================*/

/*=====================*
 *  Defines            *
 *=====================*/


/*
**-------------------------------------------------------------------------- 
** Palmpak-related register definitions
**-------------------------------------------------------------------------- 
*/
#define SYSC		0x0000			/* PALMPAK */
#define TMR		0x0001			/* Timers */
#define INTC		0x0002			/* Interrupt Controller */
#define MAC		0x0003			/* Memory Access Controller */
#define RSV1		0x0004			/* Reserved */
#define UART		0x0005			/* UARTs */
#define PIO		0x0006			/* Programmable IO */
#define DMA		0x0007			/* Direct Memory Access */
#ifdef KIWI_BURNER
#define LCD		AUX0			/* LCD */
#else
#define LCD		0x0008			/* LCD */
#endif
#define RSV2		0x0009			/* Unused */
#define AUX0		0X000A			/* Auxiliary Block 0 */
#define AUX1		0X000B			/* Auxiliary Block 1 */
#define AUX2		0X000C			/* Auxiliary Block 2 */
#define AUX3		0x000D			/* Auxiliary Block 3 */
#define	AUX4		0x000E			/* Auxiliary Block 4 */
#define RSV3		0x000F			/* Unused */

#define IRAM		0x000D			/* For Error Reporting Only */
#define	ERAM		0x000E			/* For Error Reporting Only */
#define EROM		0x000F			/* For Error Reporting Only */
#define	SRAM		ERAM			/* For Error Reporting Only */
#define	FROM		EROM			/* For Error Reporting Only */

#ifdef KIWI_BURNER
#define CODEC		AUX1			/* CODEC */
#define CODEC_BASE	(PALMPAK_BASE + (CODEC << 8))	/* CODEC */
#endif

#define SYSC_BASE	(PALMPAK_BASE + (SYSC << 8))	/* PALMPAK */
#define TMR_BASE	(PALMPAK_BASE + (TMR << 8))	/* Timers */
#define INTC_BASE	(PALMPAK_BASE + (INTC << 8))	/* Interrupt Controller */
#define MAC_BASE	(PALMPAK_BASE + (MAC << 8))	/* Memory Access Controller */
#define RSV1_BASE	(PALMPAK_BASE + (RSV1 << 8))	/* Reserved */
#define UART_BASE	(PALMPAK_BASE + (UART << 8))	/* UART/s */
#define PIO_BASE	(PALMPAK_BASE + (PIO << 8))	/* Programmable IO */
#define DMA_BASE	(PALMPAK_BASE + (DMA << 8))	/* Direct Memory Access */
#define LCD_BASE	(PALMPAK_BASE + (LCD << 8))	/* LCD */
#define AUX0_BASE	(PALMPAK_BASE + (AUX0 << 8)) 	/* Auxiliary Block 0 */
#define AUX1_BASE	(PALMPAK_BASE + (AUX1 << 8)) 	/* Auxiliary Block 1 */
#define AUX2_BASE	(PALMPAK_BASE + (AUX2 << 8)) 	/* Auxiliary Block 2 */
#define AUX3_BASE	(PALMPAK_BASE + (AUX3 << 8)) 	/* Auxiliary Block 3 */
#define AUX4_BASE	(PALMPAK_BASE + (AUX4 << 8)) 	/* Auxiliary Block 4 */

#define UART0_BASE	UART_BASE


/*=====================*
 *  External Variables *
 *=====================*/


/*=====================*
 *  External Functions *
 *=====================*/


#endif /* CHIP_REG_MAP_H */

