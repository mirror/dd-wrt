#ifndef __IDT_OCM_H__
#define __IDT_OCM_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * On-chip memory register definitions.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/ocm.h#1 $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20011005
 * Update :
 *	    $Log: ocm.h,v $
 *	    Revision 1.2  2002/06/06 18:34:04  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.1  2002/05/29 17:33:23  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h>

enum
{
	OCM0_PhysicalAddress	= 0x18098000,
	OCM_PhysicalAddress	= OCM0_PhysicalAddress,

	OCM0_VirtualAddress	= 0xb8098000,
	OCM_VirtualAddress	= OCM0_VirtualAddress,
} ;

typedef struct OCM_s
{
	U32	ocmbase ;
	U32	ocmmask ;
} volatile *OCM_t ;

enum
{
	OCMBASE_baseaddr_b	= 0,
	OCMBASE_baseaddr_m	= 0xffff0000,
	OCMMASK_mask_b		= 0,
	OCMMASK_mask_m		= 0xffff0000,
} ;

#endif	// __IDT_OCM_H__
