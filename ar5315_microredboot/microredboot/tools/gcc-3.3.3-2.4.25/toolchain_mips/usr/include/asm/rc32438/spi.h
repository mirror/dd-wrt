#ifndef __IDT_SPI_H__
#define __IDT_SPI_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * Serial Peripheral Interface register definitions. 
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/spi.h#1 $
 *
 * Author : ryan.holmQVist@idt.com
 * Date   : 20011005
 * Update :
 *	    $Log: spi.h,v $
 *	    Revision 1.2  2002/06/06 18:34:05  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.1  2002/05/29 17:33:25  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h> 

enum
{
	SPI0_PhysicalAddress	= 0x18078000,
	SPI_PhysicalAddress	= SPI0_PhysicalAddress,

	SPI0_VirtualAddress	= 0xb8078000,
	SPI_VirtualAddress	= SPI0_VirtualAddress,
} ;

typedef struct
{
	U32 spcp ;	// prescalar. 0=off, * spiClk = sysClk/(2*(spcp+1)*SPR)
	U32 spc ;	// spi control reg use SPC_
	U32 sps ;	// spi status reg use SPS_
	U32 spd ;	// spi data reg use SPD_
	U32 siofunc ;	// serial IO function use SIOFUNC_
	U32 siocfg ;	// serial IO config use SIOCFG_
	U32 siod;	// serial IO data use SIOD_
} volatile *SPI_t ;

enum
{
	SPCP_div_b	 = 0,	       
	SPCP_div_m	 = 0x000000ff,
	SPC_spr_b	= 0,	       
	SPC_spr_m	= 0x00000003,
	     SPC_spr_div2_v  = 0,
	     SPC_spr_div4_v  = 1,
	     SPC_spr_div16_v = 2,
	     SPC_spr_div32_v = 3,
	SPC_cpha_b	= 2,	       
	SPC_cpha_m	= 0x00000004,
	SPC_cpol_b	= 3,	       
	SPC_cpol_m	= 0x00000008,
	SPC_mstr_b	= 4,	       
	SPC_mstr_m	= 0x00000010,
	SPC_spe_b	= 6,	       
	SPC_spe_m	= 0x00000040,
	SPC_spie_b	= 7,	       
	SPC_spie_m	= 0x00000080,

	SPS_modf_b	= 4,	       
	SPS_modf_m	= 0x00000010,
	SPS_wcol_b	= 6,	       
	SPS_wcol_m	= 0x00000040,
	SPS_spif_b	= 7,	       
	SPS_spif_m	= 0x00000070,

	SPD_data_b	= 0,	       
	SPD_data_m	= 0x000000ff,

	SIOFUNC_sdo_b	    = 0,	   
	SIOFUNC_sdo_m	    = 0x00000001,
	SIOFUNC_sdi_b	    = 1,	   
	SIOFUNC_sdi_m	    = 0x00000002,
	SIOFUNC_sck_b	    = 2,	   
	SIOFUNC_sck_m	    = 0x00000004,
	SIOFUNC_pci_b	    = 3,	   
	SIOFUNC_pci_m	    = 0x00000008,
	
	SIOCFG_sdo_b	   = 0, 	   
	SIOCFG_sdo_m	   = 0x00000001,
	SIOCFG_sdi_b	   = 1, 	   
	SIOCFG_sdi_m	   = 0x00000002,
	SIOCFG_sck_b	   = 2, 	   
	SIOCFG_sck_m	   = 0x00000004,
	SIOCFG_pci_b	   = 3, 	   
	SIOCFG_pci_m	   = 0x00000008,
	
	SIOD_sdo_b	 = 0,		 
	SIOD_sdo_m	 = 0x00000001,
	SIOD_sdi_b	 = 1,		 
	SIOD_sdi_m	 = 0x00000002,
	SIOD_sck_b	 = 2,		 
	SIOD_sck_m	 = 0x00000004,
	SIOD_pci_b	 = 3,		 
	SIOD_pci_m	 = 0x00000008,
} ;
#endif	// __IDT_SPI_H__
