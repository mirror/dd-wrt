#ifndef __IDT_DDR_H__
#define __IDT_DDR_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * DDR register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/ddr.h#1 $
 *
 * Author : ryan.holmQVist@idt.com
 * Date   : 20011005
 * Update :
 *          $Log: ddr.h,v $
 *          Revision 1.2  2002/06/06 18:34:03  astichte
 *          Added XXX_PhysicalAddress and XXX_VirtualAddress
 *
 *          Revision 1.1  2002/05/29 17:33:21  sysarch
 *          jba File moved from vcode/include/idt/acacia
 *
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h>

enum
{
	DDR0_PhysicalAddress	= 0x18018000,
	DDR_PhysicalAddress	= DDR0_PhysicalAddress,		// Default

	DDR0_VirtualAddress	= 0xb8018000,
	DDR_VirtualAddress	= DDR0_VirtualAddress,		// Default
} ;

typedef struct DDR_s
{
	U32	ddr0base ;
	U32	ddr0mask ;
	U32	ddr1base ;
	U32	ddr1mask ;
	U32	ddrc ;
	U32	ddr0abase ;
	U32	ddr0amask ;
	U32	ddr0amap ;
	U32	ddrspare;
} volatile *DDR_t ;

enum
{
	DDR0BASE_baseaddr_b	= 16,
	DDR0BASE_baseaddr_m	= 0xffff0000,

	DDR0MASK_mask_b		= 16,
	DDR0MASK_mask_m		= 0xffff0000,

	DDR1BASE_baseaddr_b	= 16,
	DDR1BASE_baseaddr_m	= 0xffff0000,

	DDR1MASK_mask_b		= 16,
	DDR1MASK_mask_m		= 0xffff0000,

	DDRC_cs_b		= 0,
	DDRC_cs_m		= 0x00000003,
	DDRC_we_b		= 2,
	DDRC_we_m		= 0x00000004,
	DDRC_ras_b		= 3,
	DDRC_ras_m		= 0x00000008,
	DDRC_cas_b		= 4,
	DDRC_cas_m		= 0x00000010,
	DDRC_cke_b		= 5,
	DDRC_cke_m		= 0x00000020,
	DDRC_ba_b		= 6,
	DDRC_ba_m		= 0x000000c0,
	DDRC_dbw_b		= 8,
	DDRC_dbw_m		= 0x00000100,
	DDRC_wr_b		= 9,
	DDRC_wr_m		= 0x00000600,
	DDRC_ps_b		= 11,
	DDRC_ps_m		= 0x00001800,
	DDRC_dtype_b		= 13,
	DDRC_dtype_m		= 0x0000e000,
	DDRC_rfc_b		= 16,
	DDRC_rfc_m		= 0x000f0000,
	DDRC_rp_b		= 20,
	DDRC_rp_m		= 0x00300000,
	DDRC_ap_b		= 22,
	DDRC_ap_m		= 0x00400000,
	DDRC_rcd_b		= 23,
	DDRC_rcd_m		= 0x01800000,
	DDRC_cl_b		= 25,
	DDRC_cl_m		= 0x06000000,
	DDRC_dbm_b		= 27,
	DDRC_dbm_m		= 0x08000000,
	DDRC_sds_b		= 28,
	DDRC_sds_m		= 0x10000000,
	DDRC_atp_b		= 29,
	DDRC_atp_m		= 0x60000000,
	DDRC_re_b		= 31,
	DDRC_re_m		= 0x80000000,

	DDR0ABASE_baseaddr_b	= 16,
	DDR0ABASE_baseaddr_m	= 0xffff0000,

	DDR0AMASK_mask_b	= 16,
	DDR0AMASK_mask_m	= 0xffff0000,

	DDR0AMAP_map_b		= 16,
	DDR0AMAP_map_m		= 0xffff0000,
} ;

#endif	// __IDT_DDR_H__
