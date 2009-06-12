#ifndef __IDT_CROM_H__
#define __IDT_CROM_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Configuration ROM register definitions.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/crom.h#1 $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020118
 * Update :
 *	    $Log: crom.h,v $
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
	CROM0_PhysicalAddress	= 0x100b8000,
	CROM_PhysicalAddress	= CROM0_PhysicalAddress,

	CROM0_VirtualAddress	= 0xb00b8000,
	CROM_VirtualAddress	= CROM0_VirtualAddress,
} ;

typedef struct CROM_s
{
	U32	cromw0 ;		// use CROMW0_
	U32	cromw1 ;		// use CROMW1_
	U32	cromw2 ;		// use CROMW2_
} volatile * CROM_t ;

enum
{
	CROMW0_xloc_b	= 0,
	CROMW0_xloc_m	= 0x0000003f,
	CROMW0_yloc_b	= 8,
	CROMW0_yloc_m	= 0x00003f00,
	CROMW0_speed_b	= 16,
	CROMW0_speed_m	= 0x01ff0000,
	CROMW1_wafer_b	= 0,
	CROMW1_wafer_m	= 0x0000001f,
	CROMW1_lot_b	= 8,
	CROMW1_lot_m	= 0x0fffff00,
	CROMW1_fab_b	= 28,
	CROMW1_fab_m	= 0xf0000000,
	CROMW2_pci_b	= 0,
	CROMW2_pci_m	= 0x00000001,
	CROMW2_eth0_b	= 1,
	CROMW2_eth0_m	= 0x00000002,
	CROMW2_eth1_b	= 2,
	CROMW2_eth1_m	= 0x00000004
	CROMW2_i2c_b	= 3,
	CROMW2_i2c_m	= 0x00000008,
	CROMW2_rng_b	= 4,
	CROMW2_rng_m	= 0x00000010,
	CROMW2_se_b	= 5,
	CROMW2_se_m	= 0x00000020,
	CROMW2_des_b	= 6,
	CROMW2_des_m	= 0x00000040,
	CROMW2_tdes_b	= 7,
	CROMW2_tdes_m	= 0x00000080,
	CROMW2_a128_b	= 8,
	CROMW2_a128_m	= 0x00000100,
	CROMW2_a192_b	= 9,
	CROMW2_a192_m	= 0x00000200,
	CROMW2_a256_b	= 10,
	CROMW2_a256_m	= 0x00000400,
	CROMW2_md5_b	= 11,
	CROMW2_md5_m	= 0x00000800,
	CROMW2_s1_b	= 12,
	CROMW2_s1_m	= 0x00001000,
	CROMW2_s256_b	= 13,
	CROMW2_s256_m	= 0x00002000,
	CROMW2_pka_b	= 14,
	CROMW2_pka_m	= 0x00004000,
	CROMW2_exp_b	= 15,
	CROMW2_exp_m	= 0x00018000,
		CROMW2_exp_8192_v	= 0,
		CROMW2_exp_1536_v	= 1,
		CROMW2_exp_1024_v	= 2,
		CROMW2_exp_512_v	= 3,
	CROMW2_rocfg_b	= 17,
	CROMW2_rocfg_m	= 0x000e0000,
} ;

#endif	// __IDT_CROM_H__
