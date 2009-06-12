#ifndef __IDT_DEV_H__
#define __IDT_DEV_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Device Controller register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/dev.h#1 $
 *
 * Author : John.Ahrens@idt.com
 * Date   : 200112013
 * Update :
 *	    $Log: dev.h,v $
 *	    Revision 1.2  2002/06/06 18:34:03  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.1  2002/05/29 17:33:21  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h> 

enum
{
	DEV0_PhysicalAddress	= 0x18010000,
	DEV_PhysicalAddress	= DEV0_PhysicalAddress,		// Default

	DEV0_VirtualAddress	= 0xb8010000,
	DEV_VirtualAddress	= DEV0_VirtualAddress,		// Default
} ;

typedef struct DEVICE_s
{
	U32	devbase ;			// Device Base
	U32	devmask ;			// Device Mask
	U32	devc ;				// Device Control
	U32	devtc ;				// Device Timing Control
} volatile *DEVICE_t ;

enum
{
	DEV_Count = 5,
} ;

typedef struct DEV_s
{
	struct DEVICE_s	dev [DEV_Count] ;
	U32		btcs ;			// Bus timeout control / status 
	U32		btcompare ;		// Compare
	U32		btaddr ;		// Timeout address.
	U32		devdacs ;		// Decoupled access control.
	U32		devdaa ;		// Decoupled access address.
	U32		devdad ;		// Decoupled access address.
	U32		devspare ;		// spare.
} volatile *DEV_t ;

enum
{
	DEVBASE_baseaddr_b	= 16,
	DEVBASE_baseaddr_m	= 0xffff0000,
	DEVMASK_mask_b		= 16,
	DEVMASK_mask_m		= 0xffff0000,

	DEVC_ds_b		= 0,
	DEVC_ds_m		= 0x00000003,
		DEVC_ds_8_v	= 0,		// 8-bit device.
		DEVC_ds_16_v	= 1,		// 16-bit device.
		DEVC_ds_res_v	= 2,		// reserved.
		DEVC_ds_res2_v	= 3,		// reserved.
	DEVC_be_b		= 2,
	DEVC_be_m		= 0x00000004,
	DEVC_wp_b		= 3,
	DEVC_wp_m		= 0x00000008,
	DEVC_csd_b		= 4,
	DEVC_csd_m		= 0x000000f0,
	DEVC_oed_b		= 8,
	DEVC_oed_m		= 0x00000f00,
	DEVC_bwd_b		= 12,
	DEVC_bwd_m		= 0x0000f000,
	DEVC_rws_b		= 16,
	DEVC_rws_m		= 0x003f0000,
	DEVC_wws_b		= 22,
	DEVC_wws_m		= 0x0fc00000,
	DEVC_bre_b		= 28,
	DEVC_bre_m		= 0x10000000,
	DEVC_bwe_b		= 29,
	DEVC_bwe_m		= 0x20000000,
	DEVC_wam_b		= 30,
	DEVC_wam_m		= 0x40000000,

	DEVTC_prd_b		= 0,
	DEVTC_prd_m		= 0x0000000f,
	DEVTC_pwd_b		= 4,
	DEVTC_pwd_m		= 0x000000f0,
	DEVTC_wdh_b		= 8,
	DEVTC_wdh_m		= 0x00000700,
	DEVTC_csh_b		= 11,
	DEVTC_csh_m		= 0x00001800,

	BTCS_tt_b		= 0,
	BTCS_tt_m		= 0x00000001,
		BTCS_tt_write		= 0,	
		BTCS_tt_read		= 1,	
	BTCS_bto_b		= 1,		// In btcs
	BTCS_bto_m		= 0x00000002,	// In btcs
	BTCS_bte_b		= 2,		// In btcs
	BTCS_bte_m		= 0x00000004,	// In btcs

	BTCOMPARE_compare_b	= 0,		// In btcompare
	BTCOMPARE_compare_m	= 0x0000ffff,	// In btcompare

	DEVDACS_op_b		= 0,		// In devdacs
	DEVDACS_op_m		= 0x00000001,	// In devdacs
		DEVDACS_op_write_v		= 0,
		DEVDACS_op_read_v		= 1,
	DEVDACS_size_b		= 1,		// In devdacs
	DEVDACS_size_m		= 0x00000006,	// In devdacs
		DEVDACS_size_byte_v		= 0,
		DEVDACS_size_halfword	= 1,
		DEVDACS_size_triplebyte	= 2,
		DEVDACS_size_word		= 3,
	DEVDACS_err_b		= 3,		// In devdacs
	DEVDACS_err_m		= 0x00000008,	// In devdacs
	DEVDACS_f_b		= 4,		// In devdacs
	DEVDACS_f_m		= 0x00000010,	// In devdacs
} ;

#endif	//__IDT_DEV_H__

