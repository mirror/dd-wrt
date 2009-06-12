#ifndef __IDT_I2C_H__
#define __IDT_I2C_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * I2C register definitions.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/i2c.h#1 $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020120
 * Update :
 *	    $Log: i2c.h,v $
 *	    Revision 1.2  2002/06/06 18:34:04  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.1  2002/05/29 17:33:22  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h>

enum
{
	I2C0_PhysicalAddress	= 0x18070000,
	I2C_PhysicalAddress	= I2C0_PhysicalAddress,

	I2C0_VirtualAddress	= 0xb8070000,
	I2C_VirtualAddress	= I2C0_VirtualAddress,
} ;

typedef struct 
{
	U32	i2cc ;
	U32	i2cdi ;
	U32	i2cdo ;
	U32	i2ccp ;		// I2C clk = ICLK / div / 8
	U32	i2cmcmd ;
	U32	i2cms ;
	U32	i2cmsm ;
	U32	i2css ;
	U32	i2cssm ;
	U32	i2csaddr ;
	U32	i2csack ;
} volatile * I2C_t ;
enum
{
	I2CC_men_b	= 0,		// In I2C-> i2cc
	I2CC_men_m	= 0x00000001,
	I2CC_sen_b	= 1,		// In I2C-> i2cc
	I2CC_sen_m	= 0x00000002,
	I2CC_iom_b	= 2,		// In I2C-> i2cc
	I2CC_iom_m	= 0x00000004,

	I2CDI_data_b	= 0,		// In I2C-> i2cdi
	I2CDI_data_m	= 0x000000ff,

	I2CDO_data_b	= 0,		// In I2C-> i2cdo
	I2CDO_data_m	= 0x000000ff,

	I2CCP_div_b	= 0,		// In I2C-> i2ccp
	I2CCP_div_m	= 0x0000ffff,

	I2CMCMD_cmd_b	= 0,		// In I2C-> i2cmcmd
	I2CMCMD_cmd_m	= 0x0000000f,
		I2CMCMD_cmd_nop_v	= 0,
		I2CMCMD_cmd_start_v	= 1,
		I2CMCMD_cmd_stop_v	= 2,
		I2CMCMD_cmd_res3_v	= 3,
		I2CMCMD_cmd_rd_v	= 4,
		I2CMCMD_cmd_rdack_v	= 5,
		I2CMCMD_cmd_wd_v	= 6,
		I2CMCMD_cmd_wdack_v	= 7,
		I2CMCMD_cmd_res8_v	= 8,
		I2CMCMD_cmd_res9_v	= 9,
		I2CMCMD_cmd_res10_v	= 10,
		I2CMCMD_cmd_res11_v	= 11,
		I2CMCMD_cmd_res12_v	= 12,
		I2CMCMD_cmd_res13_v	= 13,
		I2CMCMD_cmd_res14_v	= 14,
		I2CMCMD_cmd_res15_v	= 15,

	I2CMS_d_b	= 0,		// In I2C-> i2cms
	I2CMS_d_m	= 0x00000001,
	I2CMS_na_b	= 1,		// In I2C-> i2cms
	I2CMS_na_m	= 0x00000002,
	I2CMS_la_b	= 2,		// In I2C-> i2cms
	I2CMS_la_m	= 0x00000004,
	I2CMS_err_b	= 3,		// In I2C-> i2cms
	I2CMS_err_m	= 0x00000008,

	I2CMSM_d_b	= 0,		// In I2C-> i2cmsm
	I2CMSM_d_m	= 0x00000001,
	I2CMSM_na_b	= 1,		// In I2C-> i2cmsm
	I2CMSM_na_m	= 0x00000002,
	I2CMSM_la_b	= 2,		// In I2C-> i2cmsm
	I2CMSM_la_m	= 0x00000004,
	I2CMSM_err_b	= 3,		// In I2C-> i2cmsm
	I2CMSM_err_m	= 0x00000008,

	I2CSS_rr_b	= 0,		// In I2C-> i2css
	I2CSS_rr_m	= 0x00000001,
	I2CSS_wr_b	= 1,		// In I2C-> i2css
	I2CSS_wr_m	= 0x00000002,
	I2CSS_sa_b	= 2,		// In I2C-> i2css
	I2CSS_sa_m	= 0x00000004,
	I2CSS_tf_b	= 3,		// In I2C-> i2css
	I2CSS_tf_m	= 0x00000008,
	I2CSS_gc_b	= 4,		// In I2C-> i2css
	I2CSS_gc_m	= 0x00000010,
	I2CSS_na_b	= 5,		// In I2C-> i2css
	I2CSS_na_m	= 0x00000020,
	I2CSS_err_b	= 6,		// In I2C-> i2css
	I2CSS_err_m	= 0x00000040,

	I2CSSM_rr_b	= 0,		// In I2C-> i2cssm
	I2CSSM_rr_m	= 0x00000001,
	I2CSSM_wr_b	= 1,		// In I2C-> i2cssm
	I2CSSM_wr_m	= 0x00000002,
	I2CSSM_sa_b	= 2,		// In I2C-> i2cssm
	I2CSSM_sa_m	= 0x00000004,
	I2CSSM_tf_b	= 3,		// In I2C-> i2cssm
	I2CSSM_tf_m	= 0x00000008,
	I2CSSM_gc_b	= 4,		// In I2C-> i2cssm
	I2CSSM_gc_m	= 0x00000010,
	I2CSSM_na_b	= 5,		// In I2C-> i2cssm
	I2CSSM_na_m	= 0x00000020,
	I2CSSM_err_b	= 6,		// In I2C-> i2cssm
	I2CSSM_err_m	= 0x00000040,

	I2CSADDR_addr_b	= 0,		// In I2C-> i2csaddr
	I2CSADDR_addr_m	= 0x000003ff,
	I2CSADDR_a_gc_b	= 10,		// In I2C-> i2csaddr
	I2CSADDR_a_gc_m	= 0x00000400,
	I2CSADDR_a10_b	= 11,		// In I2C-> i2csaddr
	I2CSADDR_a10_m	= 0x00000800,

	I2CSACK_ack_b	= 0,		// In I2C-> i2csack
	I2CSACK_ack_m	= 0x00000001,

} ;
#endif	// __IDT_I2C_H__
