#ifndef __IDT_NVRAM_H
#define __IDT_NVRAM_H

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *              All rights reserved.
 *
 * IP Arbiter register definitions.
 *
 * File   : $Id: nvram.h,v 1.3 2003/07/24 18:34:04 astichte Exp $
 *
 * Author : kiran.rao@idt.com
 * Date   : 20030724
 * Update :
 *          $Log: nvram.h,v $
 *      
 *
 ******************************************************************************/
#include <asm/rc32434/tpes.h>


enum
{
        NVRAM0_PhysicalAddress    = 0xba000000,
        NVRAM_PhysicalAddress     = NVRAM0_PhysicalAddress,         // Default

        NVRAM0_VirtualAddress     = 0xba000000,
        NVRAM_VirtualAddress      = NVRAM0_VirtualAddress,          // Default
} ;

enum
{
	NVRCMD_cmd_b		= 0,
	NVRCMD_cmd_m		= 0x0000007f,
	
	NVRS_r_b		= 0,
	NVRS_r_m		= 0x00000001,
	NVRS_e_b		= 1,
	NVRS_e_m		= 0x00000002,
	NVRS_k_b		= 2,
	NVRS_k_m		= 0x00000004, 
	
	NVRSM_r_b		= 0,
	NVRSM_r_m		= 0x00000001,
	NVRSM_e_b		= 1,
	NVRSM_e_m		= 0x00000002,
	NVRSM_k_b		= 2,
	NVRSM_k_m		= 0x00000004, 
	
	NVRCFG0_pwidth_b	= 0,
	NVRCFG0_pwidth_m	= 0x00000003,
	NVRCFG0_nmax_b		= 2,
	NVRCFG0_nmax_m		= 0x0000000C,
	NVRCFG0_vppl_b		= 4,
	NVRCFG0_vppl_m		= 0x000000f0,
	NVRCFG0_vppm_b		= 8,
	NVRCFG0_vppm_m		= 0x00000300,
	NVRCFG0_dvpp_b		= 10,
	NVRCFG0_dvpp_m		= 0x00000c00,
	NVRCFG0_x_b		= 12,
	NVRCFG0_x_m		= 0x00007000,
	
	NVRCFG1_t1tecc_b	= 0,
	NVRCFG1_t1tecc_m	= 0x00000003,
	NVRCFG1_t1mrcl_b	= 2,
	NVRCFG1_t1mrcl_m	= 0x0000000c,
	NVRCFG1_t1bias_b	= 4,
	NVRCFG1_t1bias_m	= 0x00000030,
	NVRCFG1_t2tecc_b	= 6,
	NVRCFG1_t2tecc_m	= 0x000000c0,
	NVRCFG1_t2mrcl_b	= 8,
	NVRCFG1_t2mrcl_m	= 0x00000300,
	NVRCFG1_t2bias_b	= 10,
	NVRCFG1_t2bias_m	= 0x00000c00,
	NVRCFG1_t3tecc_b	= 12,
	NVRCFG1_t3tecc_m	= 0x00003000,
	NVRCFG1_t3mrcl_b	= 14,
	NVRCFG1_t3mrcl_m	= 0x0000c000,
	NVRCFG1_t3bias_b	= 16,
	NVRCFG1_t3bias_m	= 0x00030000,
	NVRCFG1_t4tecc_b	= 18,
	NVRCFG1_t4tecc_m	= 0x000c0000,
	NVRCFG1_t4mrcl_b	= 20,
	NVRCFG1_t4mrcl_m	= 0x00300000,
	NVRCFG1_t4bias_b	= 22,
	NVRCFG1_t4bias_m	= 0x00c00000,
	NVRCFG1_t5tecc_b	= 24,
	NVRCFG1_t5tecc_m	= 0x03000000,
	NVRCFG1_t5mrcl_b	= 26,
	NVRCFG1_t5mrcl_m	= 0x0c000000,
	NVRCFG1_t5bias_b	= 28,
	NVRCFG1_t5bias_m	= 0x30000000,
}

#endif  // __IDT_NVRAM_H__

