#ifndef __IDT_PCI_H__
#define __IDT_PCI_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * PCI register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/pci.h#1 $
 *
 * Author : ryan.holmQVist@idt.com
 * Date   : 20020117
 * Update :
 *	    $Log: pci.h,v $
 *	    Revision 1.8  2002/06/19 19:53:10  rholmqvi
 *	    missing PCIPBAC_*
 *	
 *	    Revision 1.7  2002/06/19 14:38:54  astichte
 *	    Extraneous comma
 *	
 *	    Revision 1.6  2002/06/06 18:34:04  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.5  2002/06/06 16:00:57  mchessin
 *	    added vendor id, device id enums, cleaned up CFGA section
 *
 *	    Revision 1.4  2002/06/06 13:08:44  mchessin
 *	    revised comments regarding DMA chan 8&9, and messaging unit
 *
 *	    Revision 1.3  2002/06/05 20:43:28  mchessin
 *	    changes to DMA descriptor fields in accordance with
 *	    Valenv conventions, changes to PCI target control reg fields,
 *	    PCILBA prefix used for local base address
 *
 *	    Revision 1.2  2002/06/05 19:52:18  astichte
 *	    Removed IDTField
 *
 *	    Revision 1.1  2002/05/29 17:33:23  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *
 ******************************************************************************/


#include  <asm/rc32438/types.h>
enum
{
	PCI0_PhysicalAddress	= 0x18080000,
	PCI_PhysicalAddress	= PCI0_PhysicalAddress,

	PCI0_VirtualAddress	= 0xb8080000,
	PCI_VirtualAddress	= PCI0_VirtualAddress,
} ;

enum
{
	PCI_LbaCount	= 4,		// Local base addresses.
} ;

typedef struct
{
	U32	a ;		// Address.
	U32	c ;		// Control.
	U32	m ;		// mapping.
} PCI_Map_s ;

typedef struct
{
	U32		pcic ;
	U32		pcis ;
	U32		pcism ;
	U32		pcicfga ;
	U32		pcicfgd ;
	PCI_Map_s	pcilba [PCI_LbaCount] ;
	U32		pcidac ;
	U32		pcidas ;
	U32		pcidasm ;
	U32		pcidad ;
	U32		pcidma8c ;
	U32		pcidma9c ;
	U32		pcitc ;
} volatile *PCI_t ;

// PCI messaging unit.
enum
{
	PCIM_Count	= 2,
} ;
typedef struct
{
	U32		pciim [PCIM_Count] ;
	U32		pciom [PCIM_Count] ;
	U32		pciid ;
	U32		pciiic ;
	U32		pciiim ;
	U32		pciiod ;
	U32		pciioic ;
	U32		pciioim ;
} volatile *PCIM_t ;

/*******************************************************************************
 *
 * PCI Control Register
 *
 ******************************************************************************/
enum
{
	PCIC_en_b	= 0,
	PCIC_en_m	= 0x00000001,
	PCIC_tnr_b	= 1,
	PCIC_tnr_m	= 0x00000002,
	PCIC_sce_b	= 2,
	PCIC_sce_m	= 0x00000004,
	PCIC_ien_b	= 3,
	PCIC_ien_m	= 0x00000008,
	PCIC_aaa_b	= 4,
	PCIC_aaa_m	= 0x00000010,
	PCIC_eap_b	= 5,
	PCIC_eap_m	= 0x00000020,
	PCIC_pcim_b	= 6,
	PCIC_pcim_m	= 0x000001c0,
		PCIC_pcim_disabled_v	= 0,
		PCIC_pcim_tnr_v 	= 1,	// Satellite - target not ready
		PCIC_pcim_suspend_v	= 2,	// Satellite - suspended CPU.
		PCIC_pcim_extern_v	= 3,	// Host - external arbiter.
		PCIC_pcim_fixed_v	= 4,	// Host - fixed priority arb.
		PCIC_pcim_roundrobin_v	= 5,	// Host - round robin priority.
		PCIC_pcim_reserved6_v	= 6,
		PCIC_pcim_reserved7_v	= 7,
	PCIC_igm_b	= 9,
	PCIC_igm_m	= 0x00000200,
} ;

/*******************************************************************************
 *
 * PCI Status Register
 *
 ******************************************************************************/
enum {
	PCIS_eed_b	= 0,
	PCIS_eed_m	= 0x00000001,
	PCIS_wr_b	= 1,
	PCIS_wr_m	= 0x00000002,
	PCIS_nmi_b	= 2,
	PCIS_nmi_m	= 0x00000004,
	PCIS_ii_b	= 3,
	PCIS_ii_m	= 0x00000008,
	PCIS_cwe_b	= 4,
	PCIS_cwe_m	= 0x00000010,
	PCIS_cre_b	= 5,
	PCIS_cre_m	= 0x00000020,
	PCIS_mdpe_b	= 6,
	PCIS_mdpe_m	= 0x00000040,
	PCIS_sta_b	= 7,
	PCIS_sta_m	= 0x00000080,
	PCIS_rta_b	= 8,
	PCIS_rta_m	= 0x00000100,
	PCIS_rma_b	= 9,
	PCIS_rma_m	= 0x00000200,
	PCIS_sse_b	= 10,
	PCIS_sse_m	= 0x00000400,
	PCIS_ose_b	= 11,
	PCIS_ose_m	= 0x00000800,
	PCIS_pe_b	= 12,
	PCIS_pe_m	= 0x00001000,
	PCIS_tae_b	= 13,
	PCIS_tae_m	= 0x00002000,
	PCIS_rle_b	= 14,
	PCIS_rle_m	= 0x00004000,
	PCIS_bme_b	= 15,
	PCIS_bme_m	= 0x00008000,
	PCIS_prd_b	= 16,
	PCIS_prd_m	= 0x00010000,
	PCIS_rip_b	= 17,
	PCIS_rip_m	= 0x00020000,
} ;

/*******************************************************************************
 *
 * PCI Status Mask Register
 *
 ******************************************************************************/
enum {
	PCISM_eed_b		= 0,
	PCISM_eed_m		= 0x00000001,
	PCISM_wr_b		= 1,
	PCISM_wr_m		= 0x00000002,
	PCISM_nmi_b		= 2,
	PCISM_nmi_m		= 0x00000004,
	PCISM_ii_b		= 3,
	PCISM_ii_m		= 0x00000008,
	PCISM_cwe_b		= 4,
	PCISM_cwe_m		= 0x00000010,
	PCISM_cre_b		= 5,
	PCISM_cre_m		= 0x00000020,
	PCISM_mdpe_b		= 6,
	PCISM_mdpe_m		= 0x00000040,
	PCISM_sta_b		= 7,
	PCISM_sta_m		= 0x00000080,
	PCISM_rta_b		= 8,
	PCISM_rta_m		= 0x00000100,
	PCISM_rma_b		= 9,
	PCISM_rma_m		= 0x00000200,
	PCISM_sse_b		= 10,
	PCISM_sse_m		= 0x00000400,
	PCISM_ose_b		= 11,
	PCISM_ose_m		= 0x00000800,
	PCISM_pe_b		= 12,
	PCISM_pe_m		= 0x00001000,
	PCISM_tae_b		= 13,
	PCISM_tae_m		= 0x00002000,
	PCISM_rle_b		= 14,
	PCISM_rle_m		= 0x00004000,
	PCISM_bme_b		= 15,
	PCISM_bme_m		= 0x00008000,
	PCISM_prd_b		= 16,
	PCISM_prd_m		= 0x00010000,
	PCISM_rip_b		= 17,
	PCISM_rip_m		= 0x00020000,
} ;

/*******************************************************************************
 *
 * PCI Configuration Address Register
 *
 ******************************************************************************/
enum {
	PCICFGA_reg_b		= 2,
	PCICFGA_reg_m		= 0x000000fc,
		PCICFGA_reg_id_v	= 0x00>>2, //use PCFGID_
		PCICFGA_reg_04_v	= 0x04>>2, //use PCFG04_
		PCICFGA_reg_08_v	= 0x08>>2, //use PCFG08_
		PCICFGA_reg_0C_v	= 0x0C>>2, //use PCFG0C_
		PCICFGA_reg_pba0_v	= 0x10>>2, //use PCIPBA_
		PCICFGA_reg_pba1_v	= 0x14>>2, //use PCIPBA_
		PCICFGA_reg_pba2_v	= 0x18>>2, //use PCIPBA_
		PCICFGA_reg_pba3_v	= 0x1c>>2, //use PCIPBA_
		PCICFGA_reg_subsystem_v = 0x2c>>2, //use PCFGSS_
		PCICFGA_reg_3C_v	= 0x3C>>2, //use PCFG3C_
		PCICFGA_reg_pba0c_v	= 0x44>>2, //use PCIPBAC_
		PCICFGA_reg_pba0m_v	= 0x48>>2,
		PCICFGA_reg_pba1c_v	= 0x4c>>2, //use PCIPBAC_
		PCICFGA_reg_pba1m_v	= 0x50>>2,
		PCICFGA_reg_pba2c_v	= 0x54>>2, //use PCIPBAC_
		PCICFGA_reg_pba2m_v	= 0x58>>2,
		PCICFGA_reg_pba3c_v	= 0x5c>>2, //use PCIPBAC_
		PCICFGA_reg_pba3m_v	= 0x60>>2,
		PCICFGA_reg_pmgt_v	= 0x64>>2,
	PCICFGA_func_b		= 8,
	PCICFGA_func_m		= 0x00000700,
	PCICFGA_dev_b		= 11,
	PCICFGA_dev_m		= 0x0000f800,
		PCICFGA_dev_internal_v	= 0,
	PCICFGA_bus_b		= 16,
	PCICFGA_bus_m		= 0x00ff0000,
		PCICFGA_bus_type0_v	= 0,	//local bus
	PCICFGA_en_b		= 31,		// read only
	PCICFGA_en_m		= 0x80000000,
} ;

enum {
	PCFGID_vendor_b 	= 0,
	PCFGID_vendor_m 	= 0x0000ffff,
		PCFGID_vendor_IDT_v		= 0x111d,
	PCFGID_device_b 	= 16,
	PCFGID_device_m 	= 0xffff0000,
		PCFGID_device_Acaciade_v	= 0x0207,

	PCFG04_command_ioena_b		= 1,
	PCFG04_command_ioena_m		= 0x00000001,
	PCFG04_command_memena_b 	= 2,
	PCFG04_command_memena_m 	= 0x00000002,
	PCFG04_command_bmena_b		= 3,
	PCFG04_command_bmena_m		= 0x00000004,
	PCFG04_command_mwinv_b		= 5,
	PCFG04_command_mwinv_m		= 0x00000010,
	PCFG04_command_parena_b 	= 7,
	PCFG04_command_parena_m 	= 0x00000040,
	PCFG04_command_serrena_b	= 9,
	PCFG04_command_serrena_m	= 0x00000100,
	PCFG04_command_fastbbena_b	= 10,
	PCFG04_command_fastbbena_m	= 0x00000200,
	PCFG04_status_b 		= 16,
	PCFG04_status_m 		= 0xffff0000,
	PCFG04_status_66MHz_b		= 21,	// 66 MHz enable
	PCFG04_status_66MHz_m		= 0x00200000,
	PCFG04_status_fbb_b		= 23,
	PCFG04_status_fbb_m		= 0x00800000,
	PCFG04_status_mdpe_b		= 24,
	PCFG04_status_mdpe_m		= 0x01000000,
	PCFG04_status_dst_b		= 25,
	PCFG04_status_dst_m		= 0x06000000,
	PCFG04_status_sta_b		= 27,
	PCFG04_status_sta_m		= 0x08000000,
	PCFG04_status_rta_b		= 28,
	PCFG04_status_rta_m		= 0x10000000,
	PCFG04_status_rma_b		= 29,
	PCFG04_status_rma_m		= 0x20000000,
	PCFG04_status_sse_b		= 30,
	PCFG04_status_sse_m		= 0x40000000,
	PCFG04_status_pe_b		= 31,
	PCFG04_status_pe_m		= 0x40000000,

	PCFG08_revId_b			= 0,
	PCFG08_revId_m			= 0x000000ff,
	PCFG08_classCode_b		= 0,
	PCFG08_classCode_m		= 0xffffff00,
		PCFG08_classCode_bridge_v	= 06,
		PCFG08_classCode_proc_v 	= 0x0b3000, // processor-MIPS
	PCFG0C_cacheline_b		= 0,
	PCFG0C_cacheline_m		= 0x000000ff,
	PCFG0C_masterLatency_b		= 8,
	PCFG0C_masterLatency_m		= 0x0000ff00,
	PCFG0C_headerType_b		= 16,
	PCFG0C_headerType_m		= 0x00ff0000,
	PCFG0C_bist_b			= 24,
	PCFG0C_bist_m			= 0xff000000,

	PCIPBA_msi_b			= 0,
	PCIPBA_msi_m			= 0x00000001,
	PCIPBA_p_b			= 3,
	PCIPBA_p_m			= 0x00000004,
	PCIPBA_baddr_b			= 8,
	PCIPBA_baddr_m			= 0xffffff00,

	PCFGSS_vendorId_b		= 0,
	PCFGSS_vendorId_m		= 0x0000ffff,
	PCFGSS_id_b			= 16,
	PCFGSS_id_m			= 0xffff0000,

	PCFG3C_interruptLine_b		= 0,
	PCFG3C_interruptLine_m		= 0x000000ff,
	PCFG3C_interruptPin_b		= 8,
	PCFG3C_interruptPin_m		= 0x0000ff00,
	PCFG3C_minGrant_b		= 16,
	PCFG3C_minGrant_m		= 0x00ff0000,
	PCFG3C_maxLat_b 		= 24,
	PCFG3C_maxLat_m 		= 0xff000000,

	PCIPBAC_msi_b			= 0,
	PCIPBAC_msi_m			= 0x00000001,
	PCIPBAC_p_b			= 1,
	PCIPBAC_p_m			= 0x00000002,
	PCIPBAC_size_b			= 2,
	PCIPBAC_size_m			= 0x0000007c,
	PCIPBAC_sb_b			= 7,
	PCIPBAC_sb_m			= 0x00000080,
	PCIPBAC_pp_b			= 8,
	PCIPBAC_pp_m			= 0x00000100,
	PCIPBAC_mr_b			= 9,
	PCIPBAC_mr_m			= 0x00000600,
		PCIPBAC_mr_read_v	=0,	//no prefetching
		PCIPBAC_mr_readLine_v	=1,
		PCIPBAC_mr_readMult_v	=2,
	PCIPBAC_mrl_b			= 11,
	PCIPBAC_mrl_m			= 0x00000800,
	PCIPBAC_mrm_b			= 12,
	PCIPBAC_mrm_m			= 0x00001000,
	PCIPBAC_trp_b			= 13,
	PCIPBAC_trp_m			= 0x00002000,

	PCFG40_trdyTimeout_b		= 0,
	PCFG40_trdyTimeout_m		= 0x000000ff,
	PCFG40_retryLim_b		= 8,
	PCFG40_retryLim_m		= 0x0000ff00,
};

/*******************************************************************************
 *
 * PCI Local Base Address [0|1|2|3] Register
 *
 ******************************************************************************/
enum {
	PCILBA_baddr_b		= 0,		// In PCI_t -> pcilba [] .a
	PCILBA_baddr_m		= 0xffffff00,
} ;
/*******************************************************************************
 *
 * PCI Local Base Address Control Register
 *
 ******************************************************************************/
enum {
	PCILBAC_msi_b		= 0,		// In pPci->pcilba[i].c
	PCILBAC_msi_m		= 0x00000001,
		PCILBAC_msi_mem_v	= 0,
		PCILBAC_msi_io_v	= 1,
	PCILBAC_size_b		= 2,	// In pPci->pcilba[i].c
	PCILBAC_size_m		= 0x0000007c,
	PCILBAC_sb_b		= 7,	// In pPci->pcilba[i].c
	PCILBAC_sb_m		= 0x00000080,
	PCILBAC_rt_b		= 8,	// In pPci->pcilba[i].c
	PCILBAC_rt_m		= 0x00000100,
		PCILBAC_rt_noprefetch_v = 0, // mem read
		PCILBAC_rt_prefetch_v	= 1, // mem readline
} ;

/*******************************************************************************
 *
 * PCI Local Base Address [0|1|2|3] Mapping Register
 *
 ******************************************************************************/
enum {
	PCILBAM_maddr_b 	= 8,
	PCILBAM_maddr_m 	= 0xffffff00,
} ;

/*******************************************************************************
 *
 * PCI Decoupled Access Control Register
 *
 ******************************************************************************/
enum {
	PCIDAC_den_b		= 0,
	PCIDAC_den_m		= 0x00000001,
} ;

/*******************************************************************************
 *
 * PCI Decoupled Access Status Register
 *
 ******************************************************************************/
enum {
	PCIDAS_d_b	= 0,
	PCIDAS_d_m	= 0x00000001,
	PCIDAS_b_b	= 1,
	PCIDAS_b_m	= 0x00000002,
	PCIDAS_e_b	= 2,
	PCIDAS_e_m	= 0x00000004,
	PCIDAS_ofe_b	= 3,
	PCIDAS_ofe_m	= 0x00000008,
	PCIDAS_off_b	= 4,
	PCIDAS_off_m	= 0x00000010,
	PCIDAS_ife_b	= 5,
	PCIDAS_ife_m	= 0x00000020,
	PCIDAS_iff_b	= 6,
	PCIDAS_iff_m	= 0x00000040,
} ;

/*******************************************************************************
 *
 * PCI DMA Channel 8 Configuration Register
 *
 ******************************************************************************/
enum
{
	PCIDMA8C_mbs_b	= 0,		// Maximum Burst Size.
	PCIDMA8C_mbs_m	= 0x00000fff,	// { pcidma8c }
	PCIDMA8C_our_b	= 12,		// Optimize Unaligned Burst Reads.
	PCIDMA8C_our_m	= 0x00001000,	// { pcidma8c }
} ;

/*******************************************************************************
 *
 * PCI DMA Channel 9 Configuration Register
 *
 ******************************************************************************/
enum
{
	PCIDMA9C_mbs_b	= 0,		// Maximum Burst Size.
	PCIDMA9C_mbs_m	= 0x00000fff, // { pcidma9c }
} ;

/*******************************************************************************
 *
 * PCI to Memory(DMA Channel 8) AND Memory to PCI DMA(DMA Channel 9)Descriptors
 *
 ******************************************************************************/
enum {
	PCIDMAD_pt_b		= 22,		// in DEVCMD field (descriptor)
	PCIDMAD_pt_m		= 0x00c00000,	// preferred transaction field
		// These are for reads (DMA channel 8)
		PCIDMAD_devcmd_mr_v	= 0,	//memory read
		PCIDMAD_devcmd_mrl_v	= 1,	//memory read line
		PCIDMAD_devcmd_mrm_v	= 2,	//memory read multiple
		PCIDMAD_devcmd_ior_v	= 3,	//I/O read
		// These are for writes (DMA channel 9)
		PCIDMAD_devcmd_mw_v	= 0,	//memory write
		PCIDMAD_devcmd_mwi_v	= 1,	//memory write invalidate
		PCIDMAD_devcmd_iow_v	= 3,	//I/O write

	// Swap byte field applies to both DMA channel 8 and 9
	PCIDMAD_sb_b		= 24,		// in DEVCMD field (descriptor)
	PCIDMAD_sb_m		= 0x01000000,	// swap byte field
} ;


/*******************************************************************************
 *
 * PCI Target Control Register
 *
 ******************************************************************************/
enum
{
	PCITC_rtimer_b		= 0,		// In PCITC_t -> pcitc
	PCITC_rtimer_m		= 0x000000ff,
	PCITC_dtimer_b		= 8,		// In PCITC_t -> pcitc
	PCITC_dtimer_m		= 0x0000ff00,
	PCITC_rdr_b		= 18,		// In PCITC_t -> pcitc
	PCITC_rdr_m		= 0x00040000,
	PCITC_ddt_b		= 19,		// In PCITC_t -> pcitc
	PCITC_ddt_m		= 0x00080000,
} ;
/*******************************************************************************
 *
 * PCI messaging unit [applies to both inbound and outbound registers ]
 *
 ******************************************************************************/
enum
{
	PCIM_m0_b	= 0,		// In PCIM_t -> {pci{iic,iim,ioic,ioim}}
	PCIM_m0_m	= 0x00000001,	// inbound or outbound message 0
	PCIM_m1_b	= 1,		// In PCIM_t -> {pci{iic,iim,ioic,ioim}}
	PCIM_m1_m	= 0x00000002,	// inbound or outbound message 1
	PCIM_db_b	= 2,		// In PCIM_t -> {pci{iic,iim,ioic,ioim}}
	PCIM_db_m	= 0x00000004,	// inbound or outbound doorbell
};


#endif	// __IDT_PCI_H__



