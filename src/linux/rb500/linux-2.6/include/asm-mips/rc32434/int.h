#ifndef __IDT_INT_H__
#define __IDT_INT_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Interrupt Controller register definition.
 *
 * File   : $Id: int.h,v 1.3 2002/06/06 18:34:04 astichte Exp $
 *
 * Author : ryan.holmqvist@idt.com
 * Date   : 20011005
 * Update :
 *	    $Log: int.h,v $
 *	    Revision 1.3  2002/06/06 18:34:04  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.2  2002/06/05 18:47:33  astichte
 *	    Removed IDTField
 *	
 *	    Revision 1.1  2002/05/29 17:33:22  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *
 *
 ******************************************************************************/

#include  <asm/rc32434/types.h> 

enum
{
	INT0_PhysicalAddress	= 0x18038000,
	INT_PhysicalAddress	= INT0_PhysicalAddress,		// Default

	INT0_VirtualAddress	= 0xb8038000,
	INT_VirtualAddress	= INT0_VirtualAddress,		// Default
} ;

struct INT_s
{
	U32		ipend ;		//Pending interrupts. use INT?_
	U32		itest ;		//Test bits.		use INT?_
	U32		imask ;		//Interrupt disabled when set. use INT?_
} ;

enum
{
	IPEND2	= 0,			// HW 2 interrupt to core. use INT2_
	IPEND3	= 1,			// HW 3 interrupt to core. use INT3_
	IPEND4	= 2,			// HW 4 interrupt to core. use INT4_
	IPEND5	= 3,			// HW 5 interrupt to core. use INT5_
	IPEND6	= 4,			// HW 6 interrupt to core. use INT6_

	IPEND_count,			// must be last (used in loops)
	IPEND_min	= IPEND2	// min IPEND (used in loops)
};

typedef struct INTC_s
{
	struct INT_s	i [IPEND_count] ;// use i[IPEND?] = INT?_
	U32		nmips ;		// use NMIPS_
} volatile *INT_t ;

enum
{
	INT2_timer0_b			= 0,
	INT2_timer0_m			= 0x00000001,
	INT2_timer1_b			= 1,
	INT2_timer1_m			= 0x00000002,
	INT2_timer2_b			= 2,
	INT2_timer2_m			= 0x00000004,
	INT2_refresh_b			= 3,
	INT2_refresh_m			= 0x00000008,
	INT2_watchdogTimeout_b		= 4,
	INT2_watchdogTimeout_m		= 0x00000010,
	INT2_undecodedCpuWrite_b	= 5,
	INT2_undecodedCpuWrite_m	= 0x00000020,
	INT2_undecodedCpuRead_b		= 6,
	INT2_undecodedCpuRead_m		= 0x00000040,
	INT2_undecodedPciWrite_b	= 7,
	INT2_undecodedPciWrite_m	= 0x00000080,
	INT2_undecodedPciRead_b		= 8,
	INT2_undecodedPciRead_m		= 0x00000100,
	INT2_undecodedDmaWrite_b	= 9,
	INT2_undecodedDmaWrite_m	= 0x00000200,
	INT2_undecodedDmaRead_b		= 10,
	INT2_undecodedDmaRead_m		= 0x00000400,
	INT2_ipBusSlaveAckError_b	= 11,
	INT2_ipBusSlaveAckError_m	= 0x00000800,

	INT3_dmaChannel0_b		= 0,
	INT3_dmaChannel0_m		= 0x00000001,
	INT3_dmaChannel1_b		= 1,
	INT3_dmaChannel1_m		= 0x00000002,
	INT3_dmaChannel2_b		= 2,
	INT3_dmaChannel2_m		= 0x00000004,
	INT3_dmaChannel3_b		= 3,
	INT3_dmaChannel3_m		= 0x00000008,
	INT3_dmaChannel4_b		= 4,
	INT3_dmaChannel4_m		= 0x00000010,
	INT3_dmaChannel5_b		= 5,
	INT3_dmaChannel5_m		= 0x00000020,

	INT5_uartGeneral0_b		= 0,
	INT5_uartGeneral0_m		= 0x00000001,
	INT5_uartTxrdy0_b		= 1,
	INT5_uartTxrdy0_m		= 0x00000002,
	INT5_uartRxrdy0_b		= 2,
	INT5_uartRxrdy0_m		= 0x00000004,
	INT5_pci_b			= 3,
	INT5_pci_m			= 0x00000008,
	INT5_pciDecoupled_b		= 4,
	INT5_pciDecoupled_m		= 0x00000010,
	INT5_spi_b			= 5,
	INT5_spi_m			= 0x00000020,
	INT5_deviceDecoupled_b		= 6,
	INT5_deviceDecoupled_m		= 0x00000040,
	INT5_i2cMaster_b		= 7,
	INT5_i2cMaster_m		= 0x00000080,
	INT5_i2cSlave_b			= 8,
	INT5_i2cSlave_m			= 0x00000100,
	INT5_ethOvr_b			= 9,
	INT5_ethOvr_m			= 0x00000200,
	INT5_ethUnd_b			= 10,
	INT5_ethUnd_m			= 0x00000400,
	INT5_ethPfd_b			= 11,
	INT5_ethPfd_m			= 0x00000800,
	INT5_nvram_b			= 12,
	INT5_nvram_m			= 0x00001000,
	
	INT6_gpio0_b			= 0,
	INT6_gpio0_m			= 0x00000001,
	INT6_gpio1_b			= 1,
	INT6_gpio1_m			= 0x00000002,
	INT6_gpio2_b			= 2,
	INT6_gpio2_m			= 0x00000004,
	INT6_gpio3_b			= 3,
	INT6_gpio3_m			= 0x00000008,
	INT6_gpio4_b			= 4,
	INT6_gpio4_m			= 0x00000010,
	INT6_gpio5_b			= 5,
	INT6_gpio5_m			= 0x00000020,
	INT6_gpio6_b			= 6,
	INT6_gpio6_m			= 0x00000040,
	INT6_gpio7_b			= 7,
	INT6_gpio7_m			= 0x00000080,
	INT6_gpio8_b			= 8,
	INT6_gpio8_m			= 0x00000100,
	INT6_gpio9_b			= 9,
	INT6_gpio9_m			= 0x00000200,
	INT6_gpio10_b			= 10,
	INT6_gpio10_m			= 0x00000400,
	INT6_gpio11_b			= 11,
	INT6_gpio11_m			= 0x00000800,
	INT6_gpio12_b			= 12,
	INT6_gpio12_m			= 0x00001000,
	INT6_gpio13_b			= 13,
	INT6_gpio13_m			= 0x00002000,

	NMIPS_gpio_b			= 0,
	NMIPS_gpio_m			= 0x00000001,
} ;

#endif	// __IDT_INT_H__


