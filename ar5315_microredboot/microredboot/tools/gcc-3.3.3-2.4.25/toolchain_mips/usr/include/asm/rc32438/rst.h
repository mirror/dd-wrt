#ifndef __IDT_RST_H__
#define __IDT_RST_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Reset register definitions.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/rst.h#1 $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020118
 * Update :
 *	    $Log: rst.h,v $
 *	    Revision 1.2  2002/06/06 18:34:05  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.1  2002/05/29 17:33:24  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h>

enum
{
	RST0_PhysicalAddress	= 0x18000000,
	RST_PhysicalAddress	= RST0_PhysicalAddress,		// Default

	RST0_VirtualAddress	= 0xb8000000,
	RST_VirtualAddress	= RST0_VirtualAddress,		// Default
} ;

typedef struct RST_s
{
	U32	filler [0x0006] ;
	U32	sysid ;
	U32	filler2 [0x2000-8] ;		// Pad out to offset 0x8000
	U32	reset ;
	U32	bcv ;
	U32	cea ;
} volatile * RST_t ;

enum
{
	SYSID_rev_b		= 0,
	SYSID_rev_m		= 0x000000ff,
	SYSID_imp_b		= 8,
	SYSID_imp_m		= 0x000fff00,
	SYSID_vendor_b		= 8,
	SYSID_vendor_m		= 0xfff00000,

	BCV_pll_b		= 0,
	BCV_pll_m		= 0x0000000f,
		BCV_pll_PLLBypass_v	= 0x0,	// PCLK=1*CLK.
		BCV_pll_Mul3_v		= 0x1,	// PCLK=3*CLK.
		BCV_pll_Mul4_v		= 0x2,	// PCLK=4*CLK.
		BCV_pll_Mul6_v		= 0x3,	// PCLK=6*CLK.
		BCV_pll_Mul8_v		= 0x4,	// PCLK=8*CLK.
		BCV_pll_Res5_v		= 0x5,
		BCV_pll_Res6_v		= 0x6,
		BCV_pll_Res7_v		= 0x7,
		BCV_pll_Res8_v		= 0x8,
		BCV_pll_SlowMul3_v	= 0x9,	// Slow PCLK=3*CLK.
		BCV_pll_SlowMul4_v	= 0xa,	// Slow PCLK=4*CLK.
		BCV_pll_SlowMul6_v	= 0xb,	// Slow PCLK=6*CLK.
		BCV_pll_SlowMul8_v	= 0xc,	// Slow PCLK=8*CLK.
		BCV_pll_Res13_v		= 0xd,
		BCV_pll_Res14_v		= 0xe,
		BCV_pll_Res15_v		= 0xf,
	BCV_clkDiv_b		= 4,
	BCV_clkDiv_m		= 0x00000030,
		BCV_clkDiv_Div1_v	= 0x0,
		BCV_clkDiv_Div2_v	= 0x1,
		BCV_clkDiv_Div4_v	= 0x2,
		BCV_clkDiv_Res3_v	= 0x3,
	BCV_bigEndian_b		= 6,
	BCV_bigEndian_m		= 0x00000040,
	BCV_bootWidth_b		= 7,
	BCV_bootWidth_m		= 0x00000080,
		BCV_bootWidth_8_v	= 0,
		BCV_bootWidth_16_v	= 1,
	BCV_resetFast_b		= 8,
	BCV_resetFast_m		= 0x00000100,
	BCV_pciMode_b		= 9,
	BCV_pciMode_m		= 0x00000e00,
		BCV_pciMode_disabled_v	= 0,	// PCI is disabled.
		BCV_pciMode_tnr_v	= 1,	// satellite Target Not Ready.
		BCV_pciMode_suspended_v	= 2,	// satellite with suspended CPU.
		BCV_pciMode_external_v	= 3,	// host, external arbiter.
		BCV_pciMode_fixed_v	= 4,	// host, fixed priority arbiter.
		BCV_pciMode_roundRobin_v= 5,	// host, round robin arbiter.
		BCV_pciMode_res6_v	= 6,
		BCV_pciMode_res7_v	= 7,
	BCV_watchDisable_b	= 12,
	BCV_watchDisable_m	= 0x00001000,
	BCV_pllTest_b		= 13,
	BCV_pllTest_m		= 0x00002000,
	BCV_res14_b		= 14,
	BCV_res14_m		= 0x00004000,
	BCV_res15_b		= 15,
	BCV_res15_m		= 0x00008000,
} ;
#endif	// __IDT_RST_H__
