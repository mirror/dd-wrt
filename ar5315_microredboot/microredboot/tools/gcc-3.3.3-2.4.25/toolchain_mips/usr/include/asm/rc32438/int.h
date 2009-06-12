#ifndef __IDT_INT_H__
#define __IDT_INT_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Interrupt Controller register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/int.h#1 $
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

#include  <asm/rc32438/types.h> 

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
	INT2_ipBusMonEvntFinal_b	= 12,
	INT2_ipBusMonEvntFinal_m	= 0x00001000,
	INT2_ipBusMonRecDone_b		= 13,
	INT2_ipBusMonRecDone_m		= 0x00002000,
	INT2_ipBusMonEvnt0Trig_b	= 14,
	INT2_ipBusMonEvnt0Trig_m	= 0x00004000,

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
	INT3_dmaChannel6_b		= 6,
	INT3_dmaChannel6_m		= 0x00000040,
	INT3_dmaChannel7_b		= 7,
	INT3_dmaChannel7_m		= 0x00000080,
	INT3_dmaChannel8_b		= 8,
	INT3_dmaChannel8_m		= 0x00000100,
	INT3_dmaChannel9_b		= 9,
	INT3_dmaChannel9_m		= 0x00000200,
	INT3_dmaChannel10_b		= 10,
	INT3_dmaChannel10_m		= 0x00000400,
	INT3_dmaChannel11_b		= 11,
	INT3_dmaChannel11_m		= 0x00000800,
	INT3_dmaChannel12_b		= 12,
	INT3_dmaChannel12_m		= 0x00001000,

	INT4_rng_b			= 0,
	INT4_rng_m			= 0x00000001,
	INT4_pka_b			= 1,
	INT4_pka_m			= 0x00000002,
	INT4_se_b			= 2,
	INT4_se_m			= 0x00000004,

	INT5_uartGeneral0_b		= 0,
	INT5_uartGeneral0_m		= 0x00000001,
	INT5_uartTxrdy0_b		= 1,
	INT5_uartTxrdy0_m		= 0x00000002,
	INT5_uartRxrdy0_b		= 2,
	INT5_uartRxrdy0_m		= 0x00000004,
	INT5_uartGeneral1_b		= 3,
	INT5_uartGeneral1_m		= 0x00000008,
	INT5_uartTxrdy1_b		= 4,
	INT5_uartTxrdy1_m		= 0x00000010,
	INT5_uartRxrdy1_b		= 5,
	INT5_uartRxrdy1_m		= 0x00000020,
	INT5_pci_b			= 6,
	INT5_pci_m			= 0x00000040,
	INT5_pciDecoupled_b		= 7,
	INT5_pciDecoupled_m		= 0x00000080,
	INT5_spi_b			= 8,
	INT5_spi_m			= 0x00000100,
	INT5_deviceDecoupled_b		= 9,
	INT5_deviceDecoupled_m		= 0x00000200,
	INT5_i2cMaster_b		= 10,
	INT5_i2cMaster_m		= 0x00000400,
	INT5_i2cSlave_b			= 11,
	INT5_i2cSlave_m			= 0x00000800,
	INT5_eth0Ovr_b			= 12,
	INT5_eth0Ovr_m			= 0x00001000,
	INT5_eth0Und_b			= 13,
	INT5_eth0Und_m			= 0x00002000,
	INT5_eth0Pfd_b			= 14,
	INT5_eth0Pfd_m			= 0x00004000,
	INT5_eth1Ovr_b			= 15,
	INT5_eth1Ovr_m			= 0x00008000,
	INT5_eth1Und_b			= 16,
	INT5_eth1Und_m			= 0x00010000,
	INT5_eth1Pfd_b			= 17,
	INT5_eth1Pfd_m			= 0x00020000,

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
	INT6_gpio14_b			= 14,
	INT6_gpio14_m			= 0x00004000,
	INT6_gpio15_b			= 15,
	INT6_gpio15_m			= 0x00008000,
	INT6_gpio16_b			= 16,
	INT6_gpio16_m			= 0x00010000,
	INT6_gpio17_b			= 17,
	INT6_gpio17_m			= 0x00020000,
	INT6_gpio18_b			= 18,
	INT6_gpio18_m			= 0x00040000,
	INT6_gpio19_b			= 19,
	INT6_gpio19_m			= 0x00080000,
	INT6_gpio20_b			= 20,
	INT6_gpio20_m			= 0x00100000,
	INT6_gpio21_b			= 21,
	INT6_gpio21_m			= 0x00200000,
	INT6_gpio22_b			= 22,
	INT6_gpio22_m			= 0x00400000,
	INT6_gpio23_b			= 23,
	INT6_gpio23_m			= 0x00800000,
	INT6_gpio24_b			= 24,
	INT6_gpio24_m			= 0x01000000,
	INT6_gpio25_b			= 25,
	INT6_gpio25_m			= 0x02000000,
	INT6_gpio26_b			= 26,
	INT6_gpio26_m			= 0x04000000,
	INT6_gpio27_b			= 27,
	INT6_gpio27_m			= 0x08000000,
	INT6_gpio28_b			= 28,
	INT6_gpio28_m			= 0x10000000,
	INT6_gpio29_b			= 29,
	INT6_gpio29_m			= 0x20000000,
	INT6_gpio30_b			= 30,
	INT6_gpio30_m			= 0x40000000,
	INT6_gpio31_b			= 31,
	INT6_gpio31_m			= 0x80000000,

	NMIPS_gpio_b			= 0,
	NMIPS_gpio_m			= 0x00000001,
} ;

#endif	// __IDT_INT_H__


