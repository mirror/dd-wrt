#ifndef __IDT_DMA_H__
#define __IDT_DMA_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * DMA register definition.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/devs/eth/mips/acacia/current/src/dma.h#1 $
 *
 * Author : ryan.holmQVist@idt.com
 * Date   : 20011005
 * Update :
 *	    $Log: dma.h,v $
 *	    Revision 1.3  2002/06/06 18:34:03  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.2  2002/06/05 18:30:46  astichte
 *	    Removed IDTField
 *	
 *	    Revision 1.1  2002/05/29 17:33:21  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  "types.h"
enum
{
	DMA0_PhysicalAddress	= 0x18040000,
	DMA_PhysicalAddress	= DMA0_PhysicalAddress,		// Default

	DMA0_VirtualAddress	= 0xb8040000,
	DMA_VirtualAddress	= DMA0_VirtualAddress,		// Default
} ;

/*
 * DMA descriptor (in physical memory).
 */

typedef struct DMAD_s
{
	U32			control ;	// Control. use DMAD_*
	U32			ca ;		// Current Address.
	U32			devcs ; 	// Device control and status.
	U32			link ;		// Next descriptor in chain.
} volatile *DMAD_t ;

enum
{
	DMAD_size		= sizeof (struct DMAD_s),
	DMAD_count_b		= 0,		// in DMAD_t -> control
	DMAD_count_m		= 0x0003ffff,	// in DMAD_t -> control
	DMAD_ds_b		= 20,		// in DMAD_t -> control
	DMAD_ds_m		= 0x00300000,	// in DMAD_t -> control
		DMAD_ds_extToMem0_v	= 0,
		DMAD_ds_memToExt0_v	= 1,
		DMAD_ds_extToMem1_v	= 0,
		DMAD_ds_memToExt1_v	= 1,
		DMAD_ds_ethRcv0_v	= 0,
		DMAD_ds_ethXmt0_v	= 0,
		DMAD_ds_ethRcv1_v	= 0,
		DMAD_ds_ethXmt2_v	= 0,
		DMAD_ds_memToFifo_v	= 0,
		DMAD_ds_fifoToMem_v	= 0,
		DMAD_ds_rng_de_v	   = 1,//randomNumberGenerator on LC/DE
		DMAD_ds_pciToMem_v	= 0,
		DMAD_ds_memToPci_v	= 0,
		DMAD_ds_securityInput_v = 0,
		DMAD_ds_securityOutput_v = 0,
		DMAD_ds_rng_se_v	= 0,//randomNumberGenerator on SE
	
	DMAD_devcmd_b		= 22,		// in DMAD_t -> control
	DMAD_devcmd_m		= 0x01c00000,	// in DMAD_t -> control
		DMAD_devcmd_byte_v	= 0,	//memory-to-memory
		DMAD_devcmd_halfword_v	= 1,	//memory-to-memory
		DMAD_devcmd_word_v	= 2,	//memory-to-memory
		DMAD_devcmd_2words_v	= 3,	//memory-to-memory
		DMAD_devcmd_4words_v	= 4,	//memory-to-memory
		DMAD_devcmd_6words_v	= 5,	//memory-to-memory
		DMAD_devcmd_8words_v	= 6,	//memory-to-memory
		DMAD_devcmd_16words_v	= 7,	//memory-to-memory
	DMAD_cof_b		= 25,		// chain on finished
	DMAD_cof_m		= 0x02000000,	// 
	DMAD_cod_b		= 26,		// chain on done
	DMAD_cod_m		= 0x04000000,	// 
	DMAD_iof_b		= 27,		// interrupt on finished
	DMAD_iof_m		= 0x08000000,	// 
	DMAD_iod_b		= 28,		// interrupt on done
	DMAD_iod_m		= 0x10000000,	// 
	DMAD_t_b		= 29,		// terminated
	DMAD_t_m		= 0x20000000,	// 
	DMAD_d_b		= 30,		// done
	DMAD_d_m		= 0x40000000,	// 
	DMAD_f_b		= 31,		// finished
	DMAD_f_m		= 0x80000000,	// 
} ;

/*
 * DMA register (within Internal Register Map).
 */

struct DMA_Chan_s
{
	U32		dmac ;		// Control.
	U32		dmas ;		// Status.	
	U32		dmasm ; 	// Mask.
	U32		dmadptr ;	// Descriptor pointer.
	U32		dmandptr ;	// Next descriptor pointer.
};

typedef struct DMA_Chan_s volatile *DMA_Chan_t ;

//DMA_Channels	  use DMACH_count instead

enum
{
	DMAC_run_b	= 0,		// 
	DMAC_run_m	= 0x00000001,	// 
	DMAC_dm_b	= 1,		// done mask
	DMAC_dm_m	= 0x00000002,	// 
	DMAC_mode_b	= 2,		// 
	DMAC_mode_m	= 0x0000000c,	// 
		DMAC_mode_auto_v	= 0,
		DMAC_mode_burst_v	= 1,
		DMAC_mode_transfer_v	= 2, //usually used
		DMAC_mode_reserved_v	= 3,
	DMAC_a_b	= 4,		// 
	DMAC_a_m	= 0x00000010,	// 

	DMAS_f_b	= 0,		// finished (sticky) 
	DMAS_f_m	= 0x00000001,	//		     
	DMAS_d_b	= 1,		// done (sticky)     
	DMAS_d_m	= 0x00000002,	//		     
	DMAS_c_b	= 2,		// chain (sticky)    
	DMAS_c_m	= 0x00000004,	//		     
	DMAS_e_b	= 3,		// error (sticky)    
	DMAS_e_m	= 0x00000008,	//		     
	DMAS_h_b	= 4,		// halt (sticky)     
	DMAS_h_m	= 0x00000010,	//		     

	DMASM_f_b	= 0,		// finished (1=mask)
	DMASM_f_m	= 0x00000001,	// 
	DMASM_d_b	= 1,		// done (1=mask)
	DMASM_d_m	= 0x00000002,	// 
	DMASM_c_b	= 2,		// chain (1=mask)
	DMASM_c_m	= 0x00000004,	// 
	DMASM_e_b	= 3,		// error (1=mask)
	DMASM_e_m	= 0x00000008,	// 
	DMASM_h_b	= 4,		// halt (1=mask)
	DMASM_h_m	= 0x00000010,	// 
} ;

#define DMA_SMASK_VAL (DMASM_e_m | DMASM_f_m | DMASM_d_m | DMASM_h_m)

/*
 * DMA channel definitions
 */

enum
{
	DMACH_extToMem0 = 0,
	DMACH_memToExt0 = 0,
	DMACH_extToMem1 = 1,
	DMACH_memToExt1 = 1,
	DMACH_ethRcv0 = 2,
	DMACH_ethXmt0 = 3,
	DMACH_ethRcv1 = 4,
	DMACH_ethXmt2 = 5,
	DMACH_memToFifo = 6,
	DMACH_fifoToMem = 7,
	DMACH_rng_de = 7,//randomNumberGenerator on LC/DE
	DMACH_pciToMem = 8,
	DMACH_memToPci = 9,
	DMACH_securityInput = 10,
	DMACH_securityOutput = 11,
	DMACH_rng_se = 12, //randomNumberGenerator on SE
	
	DMACH_count //must be last
};


typedef struct DMAC_s
{
	struct DMA_Chan_s ch [DMACH_count] ; //use ch[DMACH_]
} volatile *DMA_t ;


/*
 * External DMA parameters
*/

enum
{
	DMADEVCMD_ts_b	= 0,		// ts field in devcmd
	DMADEVCMD_ts_m	= 0x00000007,	// ts field in devcmd
		DMADEVCMD_ts_byte_v	= 0,
		DMADEVCMD_ts_halfword_v	= 1,
		DMADEVCMD_ts_word_v	= 2,
		DMADEVCMD_ts_2word_v	= 3,
		DMADEVCMD_ts_4word_v	= 4,
		DMADEVCMD_ts_6word_v	= 5,
		DMADEVCMD_ts_8word_v	= 6,
		DMADEVCMD_ts_16word_v	= 7
};


#if 1	// aws - Compatibility.
#	define	EXTDMA_ts_b		DMADEVCMD_ts_b
#	define	EXTDMA_ts_m		DMADEVCMD_ts_m
#	define	EXTDMA_ts_byte_v	DMADEVCMD_ts_byte_v
#	define	EXTDMA_ts_halfword_v	DMADEVCMD_ts_halfword_v
#	define	EXTDMA_ts_word_v	DMADEVCMD_ts_word_v
#	define	EXTDMA_ts_2word_v	DMADEVCMD_ts_2word_v
#	define	EXTDMA_ts_4word_v	DMADEVCMD_ts_4word_v
#	define	EXTDMA_ts_6word_v	DMADEVCMD_ts_6word_v
#	define	EXTDMA_ts_8word_v	DMADEVCMD_ts_8word_v
#	define	EXTDMA_ts_16word_v	DMADEVCMD_ts_16word_v
#endif	// aws - Compatibility.

/*
 * Ethernet Receive Descriptor Device Control and Status field bit definitions.  */

enum
{
	DMAD_RX_DEVCS_fd_m    = 0x00000001,  /* first descriptor */
	DMAD_RX_DEVCS_ld_m    = 0x00000002,  /* last descriptor */
	DMAD_RX_DEVCS_rok_m   = 0x00000004,  /* receive OK */
	DMAD_RX_DEVCS_fm_m    = 0x00000008,  /* filter match */
	DMAD_RX_DEVCS_mp_m    = 0x00000010,  /* multicast packet */
	DMAD_RX_DEVCS_bp_m    = 0x00000020,  /* broadcast packet */
	DMAD_RX_DEVCS_vlt_m   = 0x00000040,  /* VLAN tag detected */
	DMAD_RX_DEVCS_cf_m    = 0x00000080,  /* control frame */
	DMAD_RX_DEVCS_ovr_m   = 0x00000100,  /* receive FIFO overflow */
	DMAD_RX_DEVCS_crc_m   = 0x00000200,  /* CRC error */
	DMAD_RX_DEVCS_cv_m    = 0x00000400,  /* code violation */
	DMAD_RX_DEVCS_db_m    = 0x00000800,  /* dribble bits detected */
	DMAD_RX_DEVCS_le_m    = 0x00001000,  /* length error */
	DMAD_RX_DEVCS_lor_m   = 0x00002000,  /* length out of range */
	DMAD_RX_DEVCS_ces_m   = 0x00004000  /* carrier event seen */
};

/*
 * Ethernet Transmit Descriptor Device Control and Status field bit definitions.
 */
enum
{
        DMAD_TX_DEVCS_fd_m    = 0x00000001,  /* first descriptor */
        DMAD_TX_DEVCS_ld_m    = 0x00000002,  /* last descriptor */
        DMAD_TX_DEVCS_oen_m   = 0x00000004,  /* override enable */
        DMAD_TX_DEVCS_pen_m   = 0x00000008,  /* packet padding enable */
        DMAD_TX_DEVCS_cen_m   = 0x00000010,  /* packet CRC enable */
        DMAD_TX_DEVCS_hen_m   = 0x00000020,  /* huge packet enable */
        DMAD_TX_DEVCS_tok_m   = 0x00000040,  /* transmit OK */
        DMAD_TX_DEVCS_mp_m    = 0x00000080,  /* multicast packet */
        DMAD_TX_DEVCS_bp_m    = 0x00000100,  /* broadcast packet */
        DMAD_TX_DEVCS_und_m   = 0x00000200,  /* transmit FIFO underflow */
        DMAD_TX_DEVCS_of_m    = 0x00000400,  /* oversized frame */
        DMAD_TX_DEVCS_ed_m    = 0x00000800,  /* excessive deferral */
        DMAD_TX_DEVCS_ec_m    = 0x00001000,  /* excessive collisions */
        DMAD_TX_DEVCS_lc_m    = 0x00002000,  /* late collision */
        DMAD_TX_DEVCS_td_m    = 0x00004000,  /* transmit deferred */
        DMAD_TX_DEVCS_crc_m   = 0x00008000,  /* CRC error */
        DMAD_TX_DEVCS_le_m    = 0x00010000   /* length error */
};


#endif	// __IDT_DMA_H__
