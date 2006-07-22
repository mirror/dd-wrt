#ifndef __IDT_IPARB_H__
#define __IDT_IPARB_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * IP Arbiter register definitions.
 *
 * File   : $Id: iparb.h,v 1.3 2002/06/06 18:34:04 astichte Exp $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020120
 * Update :
 *	    $Log: iparb.h,v $
 *	    Revision 1.3  2002/06/06 18:34:04  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.2  2002/06/05 19:01:42  astichte
 *	    Removed IDTField
 *	
 *	    Revision 1.1  2002/05/29 17:33:23  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *
 ******************************************************************************/

#include  <asm/rc32434/types.h>

enum
{
	IPARB0_PhysicalAddress	= 0x18048000,
	IPARB_PhysicalAddress	= IPARB0_PhysicalAddress,	// Default

	IPARB0_VirtualAddress	= 0xb8048000,
	IPARB_VirtualAddress	= IPARB0_VirtualAddress,	// Default
} ;

enum
{
	IPABMXC_ethernetReceive		= 0,
	IPABMXC_ethernetTransmit	= 1,
	IPABMXC_memoryToHoldFifo	= 2,
	IPABMXC_holdFifoToMemory	= 3,
	IPABMXC_pciToMemory		= 4,
	IPABMXC_memoryToPci		= 5,
	IPABMXC_pciTarget		= 6,
	IPABMXC_pciTargetStart		= 7,
	IPABMXC_cpuToIpBus		= 8,

	IPABMXC_Count,				// Must be last in list !
	IPABMXC_Min			= IPABMXC_ethernetReceive,

	IPAPXC_PriorityCount	= 4,		// 3-highest, 0-lowest.
} ;

typedef struct
{
	U32	ipapc [IPAPXC_PriorityCount] ;	// ipapc[IPAPXC_] = IPAPC_
	U32	ipabmc [IPABMXC_Count] ;	// ipabmc[IPABMXC_] = IPABMC_
	U32	ipac ;				// use IPAC_
	U32	ipaitcc;			// use IPAITCC_
	U32	ipaspare ;
} volatile * IPARB_t ;

enum
{
	IPAC_dwm_b			= 2,
	IPAC_dwm_m			= 0x00000004,
	IPAC_drm_b			= 3,
	IPAC_drm_m			= 0x00000008,
	IPAC_msk_b			= 4,
	IPAC_msk_m			= 0x00000010,

	IPAPC_ptc_b			= 0,
	IPAPC_ptc_m			= 0x00003fff,
	IPAPC_mf_b			= 14,
	IPAPC_mf_m			= 0x00004000,
	IPAPC_cptc_b			= 16,
	IPAPC_cptc_m			= 0x3fff0000,

	IPAITCC_itcc			= 0,
	IPAITCC_itcc,			= 0x000001ff,

	IPABMC_mtc_b			= 0,
	IPABMC_mtc_m			= 0x00000fff,
	IPABMC_p_b			= 12,
	IPABMC_p_m			= 0x00003000,
	IPABMC_msk_b			= 14,
	IPABMC_msk_m			= 0x00004000,
	IPABMC_cmtc_b			= 16,
	IPABMC_cmtc_m			= 0x0fff0000,
};

#endif	// __IDT_IPARB_H__
