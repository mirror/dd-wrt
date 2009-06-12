#ifndef __IDT_IPBM_H__
#define __IDT_IPBM_H__

/*******************************************************************************
 *
 * Copyright 2002 Integrated Device Technology, Inc.
 *		All rights reserved.
 *
 * IP Bus monitor definitions.
 *
 * File   : $Id: //depot/sw/releases/linuxsrc/src/kernels/mips-linux-2.4.25/include/asm-mips/rc32438/ipbm.h#1 $
 *
 * Author : Allen.Stichter@idt.com
 * Date   : 20020118
 * Update :
 *	    $Log: ipbm.h,v $
 *	    Revision 1.3  2002/06/06 18:34:04  astichte
 *	    Added XXX_PhysicalAddress and XXX_VirtualAddress
 *	
 *	    Revision 1.2  2002/06/04 19:18:14  astichte
 *	    Updated.
 *	
 *	    Revision 1.1  2002/05/29 17:33:23  sysarch
 *	    jba File moved from vcode/include/idt/acacia
 *	
 *
 ******************************************************************************/

#include  <asm/idt-boards/idt438/types.h>

enum
{
	IPBM0_PhysicalAddress	= 0x18090000,
	IPBM_PhysicalAddress	= IPBM0_PhysicalAddress,

	IPBM0_VirtualAddress	= 0xb8090000,
	IPBM_VirtualAddress	= IPBM0_VirtualAddress,
} ;

typedef struct 
{
	U32	ipbmtcfg ;
	U32	ipbmts ;
	U32	ipbmmt ;
	U32	ipbmtc0 ;
	U32	ipbmtc1 ;
	U32	ipbmtc2 ;
	U32	ipbmtc3 ;
	U32	ipbmfs ;
	U32	ipbmfc0 ;
	U32	ipbmfc1 ;
	U32	ipbmfc2 ;
	U32	ipbmrc ;
	U32	ipbmtt ;
	U32	ipbmtp ;
} volatile * IPBM_t ;

/*
 * IPBus Monitor Record Format.
 */

typedef struct 
{
	union
	{
		U32 ipbmccr_addr ;	// Clock Cycle Record if 'rf' clear.
		U32 ipbmtsr_mr ;	// Transaction Summary Record if set.
	} ;
	union
	{
		U32 ipbmccr_data ;
		U32 ipbmtsr_ts ;
	} ;
} volatile * IPBMR_t ;			// IP Bus Monitor Record type.

typedef struct 
{
	U32	emc ;
	U32	em0compare ;
	U32	em0count ;
	U32	em1count ;
	U32	em2count ;
	U32	em3count ;
	U32	em4count ;
	U32	em5count ;
	U32	em6count ;
	U32	em7count ;
} volatile * EM_t ;

enum
{
	IPBMTCFG_en_b		= 0,
	IPBMTCFG_en_m		= 0x00000001,
	IPBMTCFG_rc_b		= 1,
	IPBMTCFG_rc_m		= 0x00000002,
	IPBMTCFG_ra_b		= 2,
	IPBMTCFG_ra_m		= 0x00000004,
	IPBMTCFG_ft_b		= 3,
	IPBMTCFG_ft_m		= 0x00000008,
	IPBMTCFG_tc_b		= 4,
	IPBMTCFG_tc_m		= 0x00000010,
	IPBMTCFG_tip_b		= 5,
	IPBMTCFG_tip_m		= 0x00000020,
	IPBMTCFG_tom_b		= 6,
	IPBMTCFG_tom_m		= 0x000000c0,
	IPBMTCFG_tcount_b	= 8,
	IPBMTCFG_tcount_m	= 0x0000ff00,
	IPBMTCFG_rtcount_b	= 16,
	IPBMTCFG_rtcount_m	= 0x00ff0000,
	IPBMTCFG_die_b		= 24,
	IPBMTCFG_die_m		= 0x01000000,

	IPBMTS_a_b		= 0,
	IPBMTS_a_m		= 0x00000001,
	IPBMTS_op_b		= 1,
	IPBMTS_op_m		= 0x00000002,
	IPBMTS_d_b		= 2,
	IPBMTS_d_m		= 0x00000004,
	IPBMTS_mg_b		= 3,
	IPBMTS_mg_m		= 0x00000008,
	IPBMTS_mr_b		= 4,
	IPBMTS_mr_m		= 0x00000010,
	IPBMTS_ir_b		= 5,
	IPBMTS_ir_m		= 0x00000020,
	IPBMTS_wto_b		= 6,
	IPBMTS_wto_m		= 0x00000040,
	IPBMTS_uae_b		= 7,
	IPBMTS_uae_m		= 0x00000080,
	IPBMTS_sae_b		= 8,
	IPBMTS_sae_m		= 0x00000100,
	IPBMTS_bto_b		= 9,
	IPBMTS_bto_m		= 0x00000200,
	IPBMTS_wr_b		= 10,
	IPBMTS_wr_m		= 0x00000400,
	IPBMTS_et_b		= 11,
	IPBMTS_et_m		= 0x00000800,
	IPBMTS_trw_b		= 12,
	IPBMTS_trw_m		= 0x00001000,
	IPBMTS_em0_b		= 13,
	IPBMTS_em0_m		= 0x00002000,
	IPBMTS_mt_b		= 14,
	IPBMTS_mt_m		= 0x00004000,

	IPBMTC3_mg_b		= 0,
	IPBMTC3_mg_m		= 0x0000001f,
	IPBMTC3_mr_b		= 5,
	IPBMTC3_mr_m		= 0x003fffe0,
	IPBMTC3_mrm_b		= 22,
	IPBMTC3_mrm_m		= 0x00400000,
	IPBMTC3_ir_b		= 23,
	IPBMTC3_ir_m		= 0x0f800000,
	IPBMTC3_rw_b		= 28,
	IPBMTC3_rw_m		= 0x10000000,

	IPBMFS_en_b		= 0,
	IPBMFS_en_m		= 0x00000001,
	IPBMFS_fc_b		= 1,
	IPBMFS_fc_m		= 0x00000006,
	IPBMFS_a_b		= 3,
	IPBMFS_a_m		= 0x00000008,
	IPBMFS_bms_b		= 4,
	IPBMFS_bms_m		= 0x00000010,
	IPBMFS_op_b		= 5,
	IPBMFS_op_m		= 0x00000020,

	IPBMFC2_bms_b		= 0,
	IPBMFC2_bms_m		= 0x0001ffff,
	IPBMFC2_bms_b		= 17,
	IPBMFC2_bms_m		= 0x00020000,

	IPBMRC_ipbmbase_b	= 0,
	IPBMRC_ipbmbase_m	= 0x000007ff,
	IPBMRC_ftrl_b		= 11,
	IPBMRC_ftrl_m		= 0x003ff800,
	IPBMRC_dw_b		= 22,
	IPBMRC_dw_m		= 0x00400000,

	IPBMTT_ts_b		= 0,
	IPBMTT_ts_m		= 0x007fffff,

	IPBMTP_addr_b		= 0,
	IPBMTP_addr_m		= 0x000007ff,
	IPBMTP_tae_b		= 11,
	IPBMTP_tae_m		= 0x00000800,
} ;

/*
 * Record Formats.
 */
enum
{
	IPBMCCR_addr_b		= 0,
	IPBMCCR_addr_m		= 0x3fffffff,
	IPBMCCR_w_b		= 30,
	IPBMCCR_w_m		= 0x40000000,
	IPBMCCR_rf_b		= 31,
	IPBMCCR_rf_m		= 0x80000000,

	IPBMTSR_mr_b		= 0,
	IPBMTSR_mr_m		= 0x0001ffff,
	IPBMTSR_mg_b		= 17,
	IPBMTSR_mg_m		= 0x003e0000,
	IPBMTSR_ebe_b		= 22,
	IPBMTSR_ebe_m		= 0x03c00000,
	IPBMTSR_sbe_b		= 26,
	IPBMTSR_sbe_m		= 0x3c000000,
	IPBMTSR_f_b		= 30,
	IPBMTSR_f_m		= 0x40000000,
	IPBMTSR_rf_b		= 31,
	IPBMTSR_rf_m		= 0x80000000,

	IPBMTSR_ts_b		= 0,
	IPBMTSR_ts_m		= 0x007fffff,
	IPBMTSR_ovr_b		= 23,
	IPBMTSR_ovr_m		= 0x00800000,
	IPBMTSR_ba_b		= 24,
	IPBMTSR_ba_m		= 0x03000000,
	IPBMTSR_r_b		= 26,
	IPBMTSR_r_m		= 0x04000000,
	IPBMTSR_ipend_b		= 27,
	IPBMTSR_ipend_m		= 0xf8000000,
} ;

enum
{
	EMC_frz_b		= 0,
	EMC_frz_m		= 0x00000001,
	EMC_clr_b		= 1,
	EMC_clr_m		= 0x00000002,
	EMC_zor_b		= 2,
	EMC_zor_m		= 0x00000004,

	EM0COMPARE_compare_b	= 0,
	EM0COMPARE_compare_m	= 0x00ffffff,
	EM0COMPARE_die_b	= 30,
	EM0COMPARE_die_m	= 0x40000000,
	EM0COMPARE_t_b		= 31,
	EM0COMPARE_t_m		= 0x80000000,

	EMCOUNT_count_b		= 0,
	EMCOUNT_count_m		= 0x00ffffff,
	EMCOUNT_ovr_b		= 24,
	EMCOUNT_ovr_m		= 0x01000000,
	EMCOUNT_sel_b		= 26,
	EMCOUNT_sel_m		= 0xfc000000,
		EMCOUNT_sel_CPUInstructionExecuted	= 0,
		EMCOUNT_sel_CPUInstructionCacheMiss	= 1,
		EMCOUNT_sel_CPUDataCacheHit		= 2,
		EMCOUNT_sel_CPUDataCacheMiss		= 3,
		EMCOUNT_sel_CPUJoint TLB miss		= 4,
		EMCOUNT_sel_CPUInstructionTLBMiss	= 5,
		EMCOUNT_sel_CPUDataTLBMiss		= 6,
		EMCOUNT_sel_MaximumWaitStates		= 7,
		EMCOUNT_sel_IPBusClock			= 8,
		EMCOUNT_sel_TriggerEvent		= 9,
		EMCOUNT_sel_FinalTriggerEvent		= 10,
		EMCOUNT_sel_PMBusTransaction		= 11,
		EMCOUNT_sel_PMBusCPUTransaction		= 12,
		EMCOUNT_sel_PMBusIPBusTransaction	= 13,
		EMCOUNT_sel_PMBusSneakTransaction	= 14,
		EMCOUNT_sel_PMBusDelay			= 15,
		EMCOUNT_sel_DDRReadTransaction		= 16,
		EMCOUNT_sel_DDRWriteTransaction		= 17,
		EMCOUNT_sel_IPBusGrantCMTCZero		= 18,
		EMCOUNT_sel_DoubleWordsWrittenToOCM	= 19,
		EMCOUNT_sel_IPBusTransaction		= 20,
		EMCOUNT_sel_IPBusIdleCycle		= 21,
		EMCOUNT_sel_IPBusIndex00BytesTransferred= 22,
		EMCOUNT_sel_IPBusIndex01BytesTransferred= 23,
		EMCOUNT_sel_IPBusIndex02BytesTransferred= 24,
		EMCOUNT_sel_IPBusIndex03BytesTransferred= 25,
		EMCOUNT_sel_IPBusIndex04BytesTransferred= 26,
		EMCOUNT_sel_IPBusIndex05BytesTransferred= 27,
		EMCOUNT_sel_IPBusIndex06BytesTransferred= 28,
		EMCOUNT_sel_IPBusIndex07BytesTransferred= 29,
		EMCOUNT_sel_IPBusIndex08BytesTransferred= 30,
		EMCOUNT_sel_IPBusIndex09BytesTransferred= 31,
		EMCOUNT_sel_IPBusIndex10BytesTransferred= 32,
		EMCOUNT_sel_IPBusIndex11BytesTransferred= 33,
		EMCOUNT_sel_IPBusIndex12BytesTransferred= 34,
		EMCOUNT_sel_IPBusIndex14BytesTransferred= 35,
		EMCOUNT_sel_IPBusIndex15BytesTransferred= 36,
		EMCOUNT_sel_IPBusIndex16BytesTransferred= 37,
		EMCOUNT_sel_MaximumIdleCycles		= 38,
		EMCOUNT_sel_IPBusReadTransaction	= 39,
		EMCOUNT_sel_IPBusWriteTransaction	= 40,
		EMCOUNT_sel_IPBusTransaction01_16Bytes	= 41,
		EMCOUNT_sel_IPBusTransaction17_32Bytes	= 42,
		EMCOUNT_sel_IPBusTransaction33_48Bytes	= 43,
		EMCOUNT_sel_IPBusTransaction49_64Bytes	= 44,
		EMCOUNT_sel_IPBusUnalignedTransfer	= 45,
		EMCOUNT_sel_IPBusTransactionMerges	= 46,
		EMCOUNT_sel_IPBusIndex00Transaction	= 47,
		EMCOUNT_sel_IPBusIndex01Transaction	= 48,
		EMCOUNT_sel_IPBusIndex02Transaction	= 49,
		EMCOUNT_sel_IPBusIndex03Transaction	= 50,
		EMCOUNT_sel_IPBusIndex04Transaction	= 51,
		EMCOUNT_sel_IPBusIndex05Transaction	= 52,
		EMCOUNT_sel_IPBusIndex06Transaction	= 53,
		EMCOUNT_sel_IPBusIndex07Transaction	= 54,
		EMCOUNT_sel_IPBusIndex08Transaction	= 55,
		EMCOUNT_sel_IPBusIndex09Transaction	= 56,
		EMCOUNT_sel_IPBusIndex10Transaction	= 57,
		EMCOUNT_sel_IPBusIndex11Transaction	= 58,
		EMCOUNT_sel_IPBusIndex12Transaction	= 59,
		EMCOUNT_sel_ExternalMPBusMasterGranted	= 60,
		EMCOUNT_sel_IPBusIndex14Transaction	= 61,
		EMCOUNT_sel_IPBusIndex15Transaction	= 62,
		EMCOUNT_sel_IPBusIndex16Transaction	= 63,
} ;
#endif	// __IDT_IPBM_H__
