#ifndef __IDT_IRM_H__
#define __IDT_IRM_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Internal Register Map
 *
 * File   : $Id: irm.h,v 1.2 2002/06/05 14:51:06 astichte Exp $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020605
 * Update :
 *          $Log: irm.h,v $
 *          Revision 1.2  2002/06/05 14:51:06  astichte
 *          *** empty log message ***
 *
 *          Revision 1.1  2002/05/29 17:33:23  sysarch
 *          jba File moved from vcode/include/idt/acacia
 *
 ******************************************************************************/

/*
 * NOTE --
 *	This file is here for backwards compatibility.
 *	DO NOT USE !!!!
 */

typedef enum
{
	IRM_Physical	= 0x18000000,	// Internal Reg. map physical.
	RST_Offset	= 0x00000000,	// Includes sysid and RST.
	DEV_Offset	= 0x00010000,	// Device Controller 0.
	DDR_Offset	= 0x00018000,	// Double-Data-Rate mem. controller.
	PMARB_Offset	= 0x00020000,	// PM bus arbiter.
	TIM_Offset	= 0x00028000,	// Counter / timer.
	INTEG_Offset	= 0x00030000,	// System Integrity.
	INT_Offset	= 0x00038000,	// Interrupt controller.
	DMA_Offset	= 0x00040000,	// DMA.
	IPARB_Offset	= 0x00044000,	// IP bus arbiter.
	GPIO_Offset	= 0x00050000,	// GPIO.
	UART_Offset	= 0x00058000,	// UART
	ETH_Offset	= 0x00060000,	// Ethernet 1. 
	I2C_Offset	= 0x00068000,	// I2C interface.
	SPI_Offset	= 0x00070000,	// Serial Peripheral Interface.
	NVRAM_Offset  	= 0x00078000,	// NVRAM interface
	AUTH_Offset	= 0x0007c000,	// Authorization unit 
	PCI_Offset	= 0x00080000,
	CROM_Offset	= 0x000b8000,	// Configuration ROM.
	IRM_Size	= 0x00200000,	// Internal Reg. map size.
} IRM_Offset_t ;
 
#endif	// __IDT_IRM_H__
