/*
 * Chip-specific hardware definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#ifndef	_D11_H
#define	_D11_H

#include <bcmdevs.h>
#include <sbconfig.h>

/* enable structure packing */
#if defined(__GNUC__)
#define	PACKED	__attribute__((packed))
#else
#pragma pack(1)
#define	PACKED
#endif

/* cpp contortions to concatenate w/arg prescan */
#ifndef	PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif

#define	BCN_TMPL_LEN		512	/* length of the BCN template area */

/* RX FIFO numbers */
#define	RX_FIFO			0	/* data and ctl frames */
#define	RX_TXSTATUS_FIFO	3	/* RX fifo for tx status packages */

/* TX FIFO numbers using WME Access Classes */
#define	TX_AC_BK_FIFO		0	/* Access Category Background TX FIFO */
#define	TX_AC_BE_FIFO		1	/* Access Category Best-Effort TX FIFO */
#define	TX_AC_VI_FIFO		2	/* Access Class Video TX FIFO */
#define	TX_AC_VO_FIFO		3	/* Access Class Voice TX FIFO */
#define	TX_BCMC_FIFO		4	/* Broadcast/Multicast TX FIFO */
#define	TX_ATIM_FIFO		5	/* TX fifo for ATIM window info */

/* Addr is byte address used by SW; offset is word offset used by uCode */

/* Per AC TX limit settings */
#define M_AC_TXLMT_BASE_ADDR         (0x180 * 2)
#define M_AC_TXLMT_ADDR(_ac)         (M_AC_TXLMT_BASE_ADDR + (2 * (_ac)))

/* Legacy TX FIFO numbers */
#define	TX_DATA_FIFO		TX_AC_BE_FIFO
#define	TX_CTL_FIFO		TX_AC_VO_FIFO


/* delay from end of PLCP reception to RxTSFTime */
#define	M_APHY_PLCPRX_DLY	3
#define	M_BPHY_PLCPRX_DLY	4

typedef volatile struct {
	uint32	intstatus;
	uint32	intmask;
} intctrlregs_t;

/* read: 32-bit register that can be read as 32-bit or as 2 16-bit
 * write: only low 16b-it half can be written
 */
typedef volatile union {
	uint32 pmqhostdata;		/* read only! */
	struct {
		uint16 pmqctrlstatus;	/* read/write */
		uint16 PAD;
	} w;
} pmqreg_t;

/* pio register set 2/4 bytes union for d11 fifo */
typedef volatile union {
	pio2regp_t	b2;		/* < corerev 8 */
	pio4regp_t	b4;		/* >= corerev 8 */
} u_pioreg_t;

/* dma/pio corerev < 11 */
typedef volatile struct {
	dma32regp_t	dmaregs[8];	/* 0x200 - 0x2fc */
	u_pioreg_t	pioregs[8];	/* 0x300 */
} fifo32_t;

/* dma/pio corerev >= 11 */
typedef volatile struct {
	dma64regs_t	dmaxmt;		/* dma tx */
	pio4regs_t	piotx;		/* pio tx */
	dma64regs_t	dmarcv;		/* dma rx */
	pio4regs_t	piorx;		/* pio rx */
} fifo64_t;

/*
 * Host Interface Registers
 * - primed from hnd_cores/dot11mac/systemC/registers/ihr.h
 * - but definitely not complete
 */
typedef volatile struct _d11regs {
	/* Device Control ("semi-standard host registers") */
	uint32	PAD[3];			/* 0x0 - 0x8 */
	uint32	biststatus;		/* 0xC */
	uint32	biststatus2;		/* 0x10 */
	uint32	PAD;			/* 0x14 */
	uint32	gptimer;		/* 0x18 */	/* for corerev >= 3 */
	uint32	PAD;			/* 0x1c */

	/* Interrupt Control */		/* 0x20 */
	intctrlregs_t	intctrlregs[8];

	uint32	PAD[40];		/* 0x60 - 0xFC */

	/* tx fifos 6-7 and rx fifos 1-3 removed in corerev 5 */
	uint32	intrcvlazy[4];		/* 0x100 - 0x10C */

	uint32	PAD[4];			/* 0x110 - 0x11c */

	uint32	maccontrol;		/* 0x120 */
	uint32	maccommand;		/* 0x124 */
	uint32	macintstatus;		/* 0x128 */
	uint32	macintmask;		/* 0x12C */

	/* Transmit Template Access */
	uint32	tplatewrptr;		/* 0x130 */
	uint32	tplatewrdata;		/* 0x134 */
	uint32	PAD[2];			/* 0x138 - 0x13C */

	/* PMQ registers */
	pmqreg_t pmqreg;		/* 0x140 */
	uint32	pmqpatl;		/* 0x144 */
	uint32	pmqpath;		/* 0x148 */
	uint32	PAD;			/* 0x14C */

	uint32	chnstatus;		/* 0x150 */
	uint32	psmdebug;		/* 0x154 */	/* for corerev >= 3 */
	uint32	phydebug;		/* 0x158 */	/* for corerev >= 3 */
	uint32	machwcap;		/* 0x15C */	/* Corerev >= 13 */

	/* Extended Internal Objects */
	uint32	objaddr;		/* 0x160 */
	uint32	objdata;		/* 0x164 */
	uint32	PAD[2];			/* 0x168 - 0x16c */

	/* New txstatus registers on corerev >= 5 */
	uint32	frmtxstatus;		/* 0x170 */
	uint32	frmtxstatus2;		/* 0x174 */
	uint32	PAD[2];			/* 0x178 - 0x17c */

	/* New TSF host access on corerev >= 3 */

	uint32	tsf_timerlow;		/* 0x180 */
	uint32	tsf_timerhigh;		/* 0x184 */
	uint32	tsf_cfprep;		/* 0x188 */
	uint32	tsf_cfpstart;		/* 0x18c */
	uint32	tsf_cfpmaxdur32;	/* 0x190 */
	uint32	PAD[3];			/* 0x194 - 0x19c */

	uint32	maccontrol1;		/* 0x1a0 */
	uint32	machwcap1;		/* 0x1a4 */
	uint32	PAD[14];		/* 0x1a8 - 0x1dc */

	/* Clock control and hardware workarounds (corerev >= 13) */
	uint32	clk_ctl_st;		/* 0x1e0 */
	uint32	hw_war;
	uint32	PAD[6];			/* 0x1e8 - 0x1fc */

	/* 0x200-0x37F dma/pio registers */
	volatile union {
		fifo32_t	f32regs;	/* tx fifos 6-7 and rx fifos 1-3 (corerev < 5) */
		fifo64_t	f64regs[6];	/* on corerev >= 11 */
	} fifo;

	/* FIFO diagnostic port access */
	dma32diag_t dmafifo;		/* 0x380 - 0x38C */

	uint32	PAD[19];		/* 0x390 - 0x3D8 */

	/* time delay between the change on rf disable input and radio shutdown corerev 10 */
	uint32	rfdisabledly;		/* 0x3DC */

	/* PHY register access */
	uint16	phyversion;		/* 0x3e0 - 0x0 */
	uint16	phybbconfig;		/* 0x3e2 - 0x1 */
	uint16	phyadcbias;		/* 0x3e4 - 0x2	Bphy only */
	uint16	phyanacore;		/* 0x3e6 - 0x3	pwwrdwn on aphy */
	uint16	phyrxstatus0;		/* 0x3e8 - 0x4 */
	uint16	phyrxstatus1;		/* 0x3ea - 0x5 */
	uint16	phycrsth;		/* 0x3ec - 0x6 */
	uint16	phytxerror;		/* 0x3ee - 0x7 */
	uint16	phychannel;		/* 0x3f0 - 0x8 */
	uint16	PAD[1];			/* 0x3f2 - 0x9 */
	uint16	phytest;		/* 0x3f4 - 0xa */
	uint16	phy4waddr;		/* 0x3f6 - 0xb */
	uint16	phy4wdatahi;		/* 0x3f8 - 0xc */
	uint16	phy4wdatalo;		/* 0x3fa - 0xd */
	uint16	phyregaddr;		/* 0x3fc - 0xe */
	uint16	phyregdata;		/* 0x3fe - 0xf */

	/* IHR */			/* 0x400 - 0x7FE */

	/* RXE Block */
	uint16	PAD[3];			/* 0x400 - 0x406 */
	uint16	rcv_fifo_ctl;		/* 0x406 */
	uint16	PAD;			/* 0x408 - 0x40a */
	uint16	rcv_frm_cnt;		/* 0x40a */
	uint16	PAD[0xa];		/* 0x40a - 0x420 */
	uint16	rcm_ctl;		/* 0x420 */
	uint16	rcm_mat_data;		/* 0x422 */
	uint16	rcm_mat_mask;		/* 0x424 */
	uint16	rcm_mat_dly;		/* 0x426 */
	uint16	rcm_cond_mask_l;	/* 0x428 */
	uint16	rcm_cond_mask_h;	/* 0x42A */
	uint16	rcm_cond_dly;		/* 0x42C */
	uint16	PAD[1];			/* 0x42E */
	uint16	ext_ihr_addr;		/* 0x430 */
	uint16	ext_ihr_data;		/* 0x432 */
	uint16	rxe_phyrs_2;		/* 0x434 */
	uint16	rxe_phyrs_3;		/* 0x436 */
	uint16	phy_mode;		/* 0x438 */
	uint16	rcmta_ctl;		/* 0x43a */
	uint16	rcmta_size;		/* 0x43c */
	uint16	rcmta_addr0;		/* 0x43e */
	uint16	rcmta_addr1;		/* 0x440 */
	uint16	rcmta_addr2;		/* 0x442 */
	uint16	PAD[30];		/* 0x444 - 0x480 */

	/* PSM Block */			/* 0x480 - 0x500 */

	uint16 PAD;			/* 0x480 */
	uint16 psm_maccontrol_h;	/* 0x482 */
	uint16 psm_macintstatus_l;	/* 0x484 */
	uint16 psm_macintstatus_h;	/* 0x486 */
	uint16 psm_macintmask_l;	/* 0x488 */
	uint16 psm_macintmask_h;	/* 0x48A */
	uint16 PAD;			/* 0x48C */
	uint16 psm_maccommand;		/* 0x48E */
	uint16 psm_brc;			/* 0x490 */
	uint16 psm_phy_hdr_param;	/* 0x492 */
	uint16 psm_postcard;		/* 0x494 */
	uint16 psm_pcard_loc_l;		/* 0x496 */
	uint16 psm_pcard_loc_h;		/* 0x498 */
	uint16 psm_gpio_in;		/* 0x49A */
	uint16 psm_gpio_out;		/* 0x49C */
	uint16 psm_gpio_oe;		/* 0x49E */

	uint16 psm_bred_0;		/* 0x4A0 */
	uint16 psm_bred_1;		/* 0x4A2 */
	uint16 psm_bred_2;		/* 0x4A4 */
	uint16 psm_bred_3;		/* 0x4A6 */
	uint16 psm_brcl_0;		/* 0x4A8 */
	uint16 psm_brcl_1;		/* 0x4AA */
	uint16 psm_brcl_2;		/* 0x4AC */
	uint16 psm_brcl_3;		/* 0x4AE */
	uint16 psm_brpo_0;		/* 0x4B0 */
	uint16 psm_brpo_1;		/* 0x4B2 */
	uint16 psm_brpo_2;		/* 0x4B4 */
	uint16 psm_brpo_3;		/* 0x4B6 */
	uint16 psm_brwk_0;		/* 0x4B8 */
	uint16 psm_brwk_1;		/* 0x4BA */
	uint16 psm_brwk_2;		/* 0x4BC */
	uint16 psm_brwk_3;		/* 0x4BE */

	uint16 psm_base_0;		/* 0x4C0 */
	uint16 psm_base_1;		/* 0x4C2 */
	uint16 psm_base_2;		/* 0x4C4 */
	uint16 psm_base_3;		/* 0x4C6 */
	uint16 psm_base_4;		/* 0x4C8 */
	uint16 psm_base_5;		/* 0x4CA */
	uint16 psm_base_6;		/* 0x4CC */
	uint16 psm_pc_reg_0;		/* 0x4CE */
	uint16 psm_pc_reg_1;		/* 0x4D0 */
	uint16 psm_pc_reg_2;		/* 0x4D2 */
	uint16 psm_pc_reg_3;		/* 0x4D4 */
	uint16 PAD[0x15];		/* 0x4D6 - 0x4FE */

	/* TXE0 Block */		/* 0x500 - 0x580 */
	uint16	txe_ctl;		/* 0x500 */
	uint16	txe_aux;		/* 0x502 */
	uint16	txe_ts_loc;		/* 0x504 */
	uint16	txe_time_out;		/* 0x506 */
	uint16	txe_wm_0;		/* 0x508 */
	uint16	txe_wm_1;		/* 0x50A */
	uint16	txe_phyctl;		/* 0x50C */
	uint16	txe_status;		/* 0x50E */
	uint16	txe_mmplcp0;		/* 0x510 */
	uint16	txe_mmplcp1;		/* 0x512 */
	uint16	txe_phyctl1;		/* 0x514 */

	uint16	PAD[0x05];		/* 0x510 - 0x51E */

	/* Transmit control */
	uint16	xmtfifodef;		/* 0x520 */

	uint16	PAD[0x0F];		/* 0x522 - 0x53E */

	uint16	xmtfifocmd;		/* 0x540 */
	uint16	xmtfifoflush;		/* 0x542 */
	uint16	xmtfifothresh;		/* 0x544 */
	uint16	xmtfifordy;		/* 0x546 */
	uint16	xmtfifoprirdy;		/* 0x548 */
	uint16	xmtfiforqpri;		/* 0x54A */
	uint16	xmttplatetxptr;		/* 0x54C */
	uint16	PAD;			/* 0x54E */
	uint16	xmttplateptr;		/* 0x550 */

	uint16	PAD[0x07];		/* 0x552 - 0x55E */

	uint16	xmttplatedatalo;	/* 0x560 */
	uint16	xmttplatedatahi;	/* 0x562 */

	uint16	PAD[2];			/* 0x564 - 0x566 */

	uint16	xmtsel;			/* 0x568 */
	uint16	xmttxcnt;		/* 0x56A */
	uint16	xmttxshmaddr;		/* 0x56C */

	uint16	PAD[0x09];		/* 0x56E - 0x57E */

	/* TXE1 Block */
	uint16	PAD[0x40];		/* 0x580 - 0x5FE */

	/* TSF Block */
	uint16	PAD[0X02];		/* 0x600 - 0x602 */
	uint16	tsf_cfpstrt_l;		/* 0x604 */
	uint16	tsf_cfpstrt_h;		/* 0x606 */
	uint16	PAD[0X05];		/* 0x608 - 0x610 */
	uint16	tsf_cfppretbtt;		/* 0x612 */
	uint16	PAD[0X23];		/* 0x614 - 0x658 */
	uint16	tsf_random;		/* 0x65A */
	uint16	PAD[0x05];		/* 0x65C - 0x664 */
	/* GPTimer 2 registers are corerev >= 3 */
	uint16	tsf_gpt2_stat;		/* 0x666 */
	uint16	tsf_gpt2_ctr_l;		/* 0x668 */
	uint16	tsf_gpt2_ctr_h;		/* 0x66A */
	uint16	tsf_gpt2_val_l;		/* 0x66C */
	uint16	tsf_gpt2_val_h;		/* 0x66E */
	uint16	tsf_gptall_stat;	/* 0x670 */
	uint16	PAD[0x07];		/* 0x672 - 0x67E */

	/* IFS Block */
	uint16	ifs_sifs_rx_tx_tx;	/* 0x680 */
	uint16	ifs_sifs_nav_tx;	/* 0x682 */
	uint16	ifs_slot;		/* 0x684 */
	uint16	PAD;			/* 0x686 */
	uint16	ifs_ctl;		/* 0x688 */
	uint16	PAD[0x3];		/* 0x68a - 0x68F */
	uint16	ifsstat;		/* 0x690 */
	uint16	ifsmedbusyctl;		/* 0x692 */
	uint16	iftxdur;		/* 0x694 */
	uint16	PAD[5];
		/* New slow clock registers on corerev >= 5 */
	uint16	scc_ctl;		/* 0x6a0 */
	uint16	scc_timer_l;		/* 0x6a2 */
	uint16	scc_timer_h;		/* 0x6a4 */
	uint16	scc_frac;		/* 0x6a6 */
	uint16	scc_fastpwrup_dly;	/* 0x6a8 */
	uint16	scc_per;		/* 0x6aa */
	uint16	scc_per_frac;		/* 0x6ac */
	uint16	scc_cal_timer_l;	/* 0x6ae */
	uint16	scc_cal_timer_h;	/* 0x6b0 */
	uint16	PAD;				/* 0x6b2 */

	/* BTCX block on corerev >=13 */
	uint16	btcx_ctrl;				/* 0x6b4 */
	uint16	btcx_stat;			/* 0x6b6 */
	uint16	btcx_trans_ctrl;		/* 0x6b8 */
	uint16	btcx_pri_win;			/* 0x6ba */
	uint16	btcx_tx_conf_timer;		/* 0x6bc */
	uint16	btcx_ant_sw_timer;		/* 0x6be */

	uint16	PAD[32];		/* 0x6C0 - 0x6FF */

	/* NAV Block */
	uint16	nav_ctl;		/* 0x700 */
	uint16	navstat;		/* 0x702 */
	uint16	PAD[0x3e];		/* 0x702 - 0x77E */

	/* WEP/PMQ Block */		/* 0x780 - 0x7FE */
	uint16 PAD[0x20];		/* 0x780 - 0x7BE */

	uint16 wepctl;			/* 0x7C0 */
	uint16 wepivloc;		/* 0x7C2 */
	uint16 wepivkey;		/* 0x7C4 */
	uint16 wepwkey;			/* 0x7C6 */

	uint16 PAD[4];			/* 0x7C8 - 0x7CE */
	uint16 pcmctl;			/* 0X7D0 */
	uint16 pcmstat;			/* 0X7D2 */
	uint16 PAD[6];			/* 0x7D4 - 0x7DE */

	uint16 pmqctl;			/* 0x7E0 */
	uint16 pmqstatus;		/* 0x7E2 */
	uint16 pmqpat0;			/* 0x7E4 */
	uint16 pmqpat1;			/* 0x7E6 */
	uint16 pmqpat2;			/* 0x7E8 */

	uint16 pmqdat;			/* 0x7EA */
	uint16 pmqdator;		/* 0x7EC */
	uint16 pmqhst;			/* 0x7EE */
	uint16 pmqpath0;		/* 0x7F0 */
	uint16 pmqpath1;		/* 0x7F2 */
	uint16 pmqpath2;		/* 0x7F4 */
	uint16 pmqdath;			/* 0x7F6 */

	uint16 PAD[0x04];		/* 0x7F8 - 0x7FE */

	/* SHM */			/* 0x800 - 0xEFE */
	uint16	PAD[0x380];		/* 0x800 - 0xEFE */

	/* SB configuration registers: 0xF00 */
	sbconfig_t	sbconfig;	/* sb config regs occupy top 256 bytes */
} d11regs_t;

#define	PIHR_BASE	0x0400			/* byte address of packed IHR region */

/* biststatus */
#define	BT_DONE		((uint32)1 << 31)	/* bist done */
#define	BT_B2S		((uint32)1 << 30)	/* bist2 ram summary bit */

/* intstatus and intmask */
#define	I_PC		((uint32)1 << 10)	/* pci descriptor error */
#define	I_PD		((uint32)1 << 11)	/* pci data error */
#define	I_DE		((uint32)1 << 12)	/* descriptor protocol error */
#define	I_RU		((uint32)1 << 13)	/* receive descriptor underflow */
#define	I_RO		((uint32)1 << 14)	/* receive fifo overflow */
#define	I_XU		((uint32)1 << 15)	/* transmit fifo underflow */
#define	I_RI		((uint32)1 << 16)	/* receive interrupt */
#define	I_XI		((uint32)1 << 24)	/* transmit interrupt */

/* interrupt receive lazy */
#define	IRL_TO_MASK		0x00ffffff	/* timeout */
#define	IRL_FC_MASK		0xff000000	/* frame count */
#define	IRL_FC_SHIFT		24		/* frame count */

/* maccontrol register */
#define	MCTL_GMODE		(1 << 31)
#define	MCTL_DISCARD_PMQ	(1 << 30)
#define	MCTL_DISCARD_TXSTATUS	(1 << 29)
#define	MCTL_TBTT_HOLD		(1 << 28)
#define	MCTL_CLOSED_NETWORK	(1 << 27)
#define	MCTL_WAKE		(1 << 26)
#define	MCTL_HPS		(1 << 25)
#define	MCTL_PROMISC		(1 << 24)
#define	MCTL_PHYLOCK		(1 << 23)  
#define	MCTL_KEEPCONTROL	(1 << 22)
#define	MCTL_KEEPBADPLCP	(1 << 21)
#define	MCTL_BCNS_PROMISC	(1 << 20)
#define	MCTL_LOCK_RADIO		(1 << 19)
#define	MCTL_AP			(1 << 18)
#define	MCTL_INFRA		(1 << 17)
#define	MCTL_BIGEND		(1 << 16)
#define	MCTL_GPOUT_SEL_MASK	(3 << 14)
#define	MCTL_GPOUT_SEL_SHIFT	14
#define	MCTL_EN_PSMDBG		(1 << 13)
#define	MCTL_IHR_EN		(1 << 10)
#define	MCTL_SHM_UPPER		(1 <<  9)
#define	MCTL_SHM_EN		(1 <<  8)
#define	MCTL_PSM_JMP_0		(1 <<  2)
#define	MCTL_PSM_RUN		(1 <<  1)
#define	MCTL_EN_MAC		(1 <<  0)

/* maccontrol1 register */
#define	MCTL1_GCPS		0x00000001
#define MCTL1_EGS_MASK		0x0000c000
#define MCTL1_EGS_SHIFT		14

/* maccommand register */
#define	MCMD_BCN0VLD		(1 <<  0)
#define	MCMD_BCN1VLD		(1 <<  1)
#define	MCMD_DIRFRMQVAL		(1 <<  2)
#define	MCMD_CCA		(1 <<  3)
#define	MCMD_BG_NOISE		(1 <<  4)
#define	MCMD_SKIP_SHMINIT	(1 <<  5) /* only used for simulation */

/* macintstatus/macintmask */
#define	MI_MACSSPNDD		(1 <<  0)	/* MAC has gracefully suspended */
#define	MI_BCNTPL		(1 <<  1)	/* beacon template available */
#define	MI_TBTT			(1 <<  2)	/* TBTT indication */
#define	MI_BCNSUCCESS		(1 <<  3)	/* beacon succesfully tx */
#define	MI_BCNCANCLD		(1 <<  4)	/* beacon cancelled (IBSS) */
#define	MI_ATIMWINEND		(1 <<  5)	/* end of ATIM-window (IBSS) */
#define	MI_PMQ			(1 <<  6)	/* PMQ entries available */
#define	MI_NSPECGEN_0		(1 <<  7)	/* non-specific gen-stat bits that are set by PSM */
#define	MI_NSPECGEN_1		(1 <<  8)	/* non-specific gen-stat bits that are set by PSM */
#define	MI_MACTXERR		(1 <<  9)	/* MAC level Tx error */
#define	MI_NSPECGEN_3		(1 << 10)	/* non-specific gen-stat bits that are set by PSM */
#define	MI_PHYTXERR		(1 << 11)	/* PHY Tx error */
#define	MI_PME			(1 << 12)	/* Power Management Event */
#define	MI_GP0			(1 << 13)	/* General-purpose timer0 */
#define	MI_GP1			(1 << 14)	/* General-purpose timer1 */
#define	MI_DMAINT		(1 << 15)	/* (ORed) DMA-interrupts */
#define	MI_TXSTOP		(1 << 16)	/* MAC has completed a TX FIFO Suspend/Flush */
#define	MI_CCA			(1 << 17)	/* MAC has completed a CCA measurement */
#define	MI_BG_NOISE		(1 << 18)	/* MAC has collected background noise samples */
#define	MI_DTIM_TBTT		(1 << 19)	/* MBSS DTIM TBTT indication */
#define 	MI_PRQ			(1 << 20)	/* Probe response queue needs attention */
#define	MI_PWRUP		(1 << 21)	/* Radio/PHY has been powered back up. */
#define MI_RFDISABLE		(1 << 28)	/* MAC detected a change on RF Disable input
						 * (corerev >= 10)
						 */
#define	MI_TFS			(1 << 29)	/* MAC has completed a TX (corerev >= 5) */
#define	MI_PHYCHANGED		(1 << 30)	/* A phy status change wrt G mode */
#define	MI_TO			(1 << 31)	/* general purpose timeout (corerev >= 3) */

/* Mac capabilbities registers */
/* machwcap */
#define	MCAP_TKIPMIC		0x80000000	/* TKIP MIC hardware present */
#define	MCAP_TKIPPH2KEY		0x40000000	/* TKIP phase 2 key hardware present */
#define	MCAP_BTCX		0x20000000	/* BT coexistance hardware and pins present */
#define	MCAP_MBSS		0x10000000	/* Multi-BSS hardware present */
#define	MCAP_RXFSZ_MASK		0x03f80000	/* Rx fifo size (* 512 bytes) */
#define	MCAP_RXFSZ_SHIFT	19
#define	MCAP_NRXQ_MASK		0x00070000	/* Max Rx queues supported - 1 */
#define	MCAP_NRXQ_SHIFT		16
#define	MCAP_UCMSZ_MASK		0x0000e000	/* Ucode memory size */
#define	MCAP_UCMSZ_3K3		0x00000000	/* 3328 Words Ucode memory */
#define	MCAP_UCMSZ_4K		0x00002000	/* 4096 Words Ucode memory */
#define	MCAP_UCMSZ_SHIFT	13
#define	MCAP_TXFSZ_MASK		0x000003f8	/* Tx fifo size (* 512 bytes) */
#define	MCAP_TXFSZ_SHIFT	3
#define	MCAP_NTXQ_MASK		0x00000007	/* Max Tx queues supported - 1 */
#define	MCAP_NTXQ_SHIFT		0

/* machwcap1 */
#define	MCAP1_ERC_MASK		0x00000001	/* external radio coexistance */
#define	MCAP1_ERC_SHIFT		0

/* pmqhost data */
#define	PMQH_DATA_MASK		0xffff0000	/* data entry of head pmq entry */
#define	PMQH_BSSCFG		0x00100000	/* PM entry for BSS config */
#define	PMQH_PMOFF		0x00010000	/* PM Mode OFF: power save off */
#define	PMQH_PMON		0x00020000	/* PM Mode ON: power save on */
#define	PMQH_DASAT		0x00040000	/* Dis-associated or De-authenticated */
#define	PMQH_ATIMFAIL		0x00080000	/* ATIM not acknowledged */
#define	PMQH_DEL_ENTRY		0x00000001	/* delete head entry */
#define	PMQH_DEL_MULT		0x00000002	/* delete head entry to cur read pointer -1 */
#define	PMQH_OFLO		0x00000004	/* pmq overflow indication */
#define	PMQH_NOT_EMPTY		0x00000008	/* entries are present in pmq */

/* phydebug (corerev >= 3) */
#define	PDBG_CRS		(1 << 0)	/* phy is asserting carrier sense */
#define	PDBG_TXA		(1 << 1)	/* phy is taking xmit byte from mac this cycle */
#define	PDBG_TXF		(1 << 2)	/* mac is instructing the phy to transmit a frame */
#define	PDBG_TXE		(1 << 3)	/* phy is signalling a transmit Error to the mac */
#define	PDBG_RXF		(1 << 4)	/* phy detected the end of a valid frame preamble */
#define	PDBG_RXS		(1 << 5)	/* phy detected the end of a valid PLCP header */
#define	PDBG_RXFRG		(1 << 6)	/* rx start not asserted */
#define	PDBG_RXV		(1 << 7)	/* mac is taking receive byte from phy this cycle */
#define	PDBG_RFD		(1 << 16)	/* RF portion of the radio is disabled */

/* objaddr register */
#define	OBJADDR_SEL_MASK	0x000F0000
#define	OBJADDR_UCM_SEL		0x00000000
#define	OBJADDR_SHM_SEL		0x00010000
#define	OBJADDR_SCR_SEL		0x00020000
#define	OBJADDR_IHR_SEL		0x00030000
#define	OBJADDR_RCMTA_SEL	0x00040000
#define	OBJADDR_SRCHM_SEL	0x00060000
#define	OBJADDR_WINC		0x01000000
#define	OBJADDR_RINC		0x02000000
#define	OBJADDR_AUTO_INC	0x03000000

/* pcmaddr bits */
#define	PCMADDR_INC		0x4000
#define	PCMADDR_UCM_SEL		0x0000

#define	WEP_PCMADDR		0x07d4
#define	WEP_PCMDATA		0x07d6

/* frmtxstatus */
#define	TXS_V			(1 << 0)	/* valid bit */
#define	TXS_STATUS_MASK		0xffff
/* sw mask to map txstatus for corerevs <= 4 to be the same as for corerev > 4 */
#define	TXS_COMPAT_MASK		0x3
#define	TXS_COMPAT_SHIFT	1
#define	TXS_FID_MASK		0xffff0000
#define	TXS_FID_SHIFT		16

/* frmtxstatus2 */
#define	TXS_SEQ_MASK		0xffff
#define	TXS_PTX_MASK		0xff0000
#define	TXS_PTX_SHIFT		16
#define	TXS_MU_MASK		0x01000000
#define	TXS_MU_SHIFT		24


/* tsf_cfprep register */
#define	CFPREP_CBI_MASK		0xffffffc0
#define	CFPREP_CBI_SHIFT	6
#define	CFPREP_CFPP		0x00000001

/* transmit fifo control for 2-byte pio */
#define	XFC_BV_MASK		0x3		/* bytes valid */
#define	XFC_LO			(1 << 0)	/* low byte valid */
#define	XFC_HI			(1 << 1)	/* high byte valid */
#define	XFC_BOTH		(XFC_HI | XFC_LO) /* both bytes valid */
#define	XFC_EF			(1 << 2)	/* end of frame */
#define	XFC_FR			(1 << 3)	/* frame ready */
#define	XFC_FL			(1 << 5)	/* flush request */
#define	XFC_FP			(1 << 6)	/* flush pending */
#define	XFC_SE			(1 << 7)	/* suspend request */
#define	XFC_SP			(1 << 8)	
#define	XFC_CC_MASK		0xfc00		/* committed count */
#define	XFC_CC_SHIFT		10

/* transmit fifo control for 4-byte pio */
#define	XFC4_BV_MASK		0xf		/* bytes valid */
#define	XFC4_EF			(1 << 4)	/* end of frame */
#define	XFC4_FR			(1 << 7)	/* frame ready */
#define	XFC4_SE			(1 << 8)	/* suspend request */
#define	XFC4_SP			(1 << 9)	
#define	XFC4_FL			(1 << 10)	/* flush request */
#define	XFC4_FP			(1 << 11)	/* flush pending */

/* receive fifo control */
#define	RFC_FR			(1 << 0)	/* frame ready */
#define	RFC_DR			(1 << 1)	/* data ready */

/* tx fifo sizes for corerev >= 9 */
/* tx fifo sizes values are in terms of 256 byte blocks */
#define TXFIFOCMD_RESET_MASK	(1 << 15)	/* reset */
#define TXFIFOCMD_FIFOSEL_SHIFT	8		/* fifo */
#define TXFIFO_FIFOTOP_SHIFT	8		/* fifo start */

#define TXFIFO_START_BLK	 6		/* Base address + 6 * 256 B */
#define TXFIFO_SIZE_UNIT	256		/* one unit corresponds to 256 bytes */

/* PhyVersion regiser */

/* phy versions, PhyVersion:Revision field */
#define	PV_AV_MASK		0xf000		/* analog block version */
#define	PV_AV_SHIFT		12		/* analog block version bitfield offset */
#define	PV_PT_MASK		0x0f00		/* phy type */
#define	PV_PT_SHIFT		8		/* phy type bitfield offset */
#define	PV_PV_MASK		0x000f		/* phy version */
#define	PHY_TYPE(v)		((v & PV_PT_MASK) >> PV_PT_SHIFT)

/* phy types, PhyVersion:PhyType field */
#define	PHY_TYPE_A		0	/* A-Phy value */
#define	PHY_TYPE_B		1	/* B-Phy value */
#define	PHY_TYPE_G		2	/* G-Phy value */
#define	PHY_TYPE_N		4	/* N-Phy value */
#define	PHY_TYPE_LP		5	/* LP-Phy value */
#define	PHY_TYPE_NULL		0xf	/* Invalid Phy value */

/* analog types, PhyVersion:AnalogType field */
#define	ANA_11G_018		1
#define	ANA_11G_018_ALL		2
#define	ANA_11G_018_ALLI	3
#define	ANA_11G_013		4
#define	ANA_11N_013		5
#define	ANA_11LP_013		6

/* 802.11a PLCP header def */
typedef struct ofdm_phy_hdr ofdm_phy_hdr_t;
struct ofdm_phy_hdr {
	uint8	rlpt[3];	/* rate, length, parity, tail */
	uint16	service;
	uint8	pad;
} PACKED;

#define	D11A_PHY_HDR_GRATE(phdr)	((phdr)->rlpt[0] & 0x0f)
#define	D11A_PHY_HDR_GRES(phdr)		(((phdr)->rlpt[0] >> 4) & 0x01)
#define	D11A_PHY_HDR_GLENGTH(phdr)	(((uint32 *)((phdr)->rlpt) >> 5) & 0x0fff)
#define	D11A_PHY_HDR_GPARITY(phdr)	(((phdr)->rlpt[3] >> 1) & 0x01)
#define	D11A_PHY_HDR_GTAIL(phdr)	(((phdr)->rlpt[3] >> 2) & 0x3f)

/* rate encoded per 802.11a-1999 sec 17.3.4.1 */
#define	D11A_PHY_HDR_SRATE(phdr, rate)		\
	((phdr)->rlpt[0] = ((phdr)->rlpt[0] & 0xf0) | ((rate) & 0xf))
/* set reserved field to zero */
#define	D11A_PHY_HDR_SRES(phdr)		((phdr)->rlpt[0] &= 0xef)
/* length is number of octets in PSDU */
#define	D11A_PHY_HDR_SLENGTH(phdr, length)	\
	(*(uint32 *)((phdr)->rlpt) = *(uint32 *)((phdr)->rlpt) | \
	(((length) & 0x0fff) << 5))
/* set the tail to all zeros */
#define	D11A_PHY_HDR_STAIL(phdr)	((phdr)->rlpt[3] &= 0x03)

#define	D11A_PHY_HDR_LEN_L	3	/* low-rate part of PLCP header */
#define	D11A_PHY_HDR_LEN_R	2	/* high-rate part of PLCP header */

#define	D11A_PHY_TX_DELAY	(2) /* 2.1 usec */

#define	D11A_PHY_HDR_TIME	(4)	/* low-rate part of PLCP header */
#define	D11A_PHY_PRE_TIME	(16)
#define	D11A_PHY_PREHDR_TIME	(D11A_PHY_PRE_TIME + D11A_PHY_HDR_TIME)

/* 802.11b PLCP header def */
typedef struct cck_phy_hdr cck_phy_hdr_t;
struct cck_phy_hdr {
	uint8	signal;
	uint8	service;
	uint16	length;
	uint16	crc;
} PACKED;

#define	D11B_PHY_HDR_LEN	6

#define	D11B_PHY_TX_DELAY	(3) /* 3.4 usec */

#define	D11B_PHY_LHDR_TIME	(D11B_PHY_HDR_LEN << 3)
#define	D11B_PHY_LPRE_TIME	(144)
#define	D11B_PHY_LPREHDR_TIME	(D11B_PHY_LPRE_TIME + D11B_PHY_LHDR_TIME)

#define	D11B_PHY_SHDR_TIME	(D11B_PHY_LHDR_TIME >> 1)
#define	D11B_PHY_SPRE_TIME	(D11B_PHY_LPRE_TIME >> 1)
#define	D11B_PHY_SPREHDR_TIME	(D11B_PHY_SPRE_TIME + D11B_PHY_SHDR_TIME)

#define	D11B_PLCP_SIGNAL_LOCKED	(1 << 2)
#define	D11B_PLCP_SIGNAL_LE	(1 << 7)

/* AMPDUXXX: move to ewc header file once it is ready: Mimo PLCP */
#define MIMO_PLCP_MCS_MASK	0x7f	/* mcs index */
#define MIMO_PLCP_40MHZ		0x80	/* 40 Hz frame */
#define MIMO_PLCP_AMPDU		0x08	/* ampdu */

#define WLC_GET_MIMO_PLCP_LEN(plcp) (plcp[1] + (plcp[2] << 8))
#define WLC_SET_MIMO_PLCP_LEN(plcp, len) \
	plcp[1] = len & 0xff; plcp[2] = ((len >> 8) & 0xff);

#define WLC_SET_MIMO_PLCP_AMPDU(plcp) (plcp[3] |= MIMO_PLCP_AMPDU)
#define WLC_CLR_MIMO_PLCP_AMPDU(plcp) (plcp[3] &= ~MIMO_PLCP_AMPDU)
#define WLC_IS_MIMO_PLCP_AMPDU(plcp) (plcp[3] & MIMO_PLCP_AMPDU)


/* The dot11a PLCP header is 5 bytes.  To simplify the software (so that we
 * don't need e.g. different tx DMA headers for 11a and 11b), the PLCP header has
 * padding added in the ucode.
 */
#define	D11_PHY_HDR_LEN	6

/* TX DMA buffer header */
typedef struct d11txh d11txh_t;
struct d11txh {
	uint16	MacTxControlLow;
	uint16	MacTxControlHigh;
	uint16	MacFrameControl;
	uint16	TxFesTimeNormal;
	uint16	PhyTxControlWord;
	uint16	PhyTxControlWord_1;
	uint16	PhyTxControlWord_1_Fbr;
	uint16	PhyTxControlWord_1_Rts;
	uint16	PhyTxControlWord_1_FbrRts;
	uint16	MainRates;
	uint16	XtraFrameTypes;
	uint8	IV[16];
	uint8	TxFrameRA[6];
	uint16	TxFesTimeFallback;
	uint8	RTSPLCPFallback[6];
	uint16	RTSDurFallback;
	uint8	FragPLCPFallback[6];
	uint16	FragDurFallback;
	uint16	MModeLen;
	uint16	MModeFbrLen;
	uint16	TstampLow;
	uint16	TstampHigh;
	uint16	MimoAntSel;
	uint16	PreloadSize;
	uint16	PAD;
	uint16	TxFrameID;
	uint16	TxStatus;
	uint8	RTSPhyHeader[D11_PHY_HDR_LEN];
	struct dot11_rts_frame rts_frame;
	uint16	PAD;
} PACKED;

#define	D11_TXH_LEN		0x68

/* Frame Types */
#define FT_CCK	0
#define FT_OFDM	1
#define FT_EWC	2
#define FT_N	3

/* Position of MPDU inside A-MPDU; indicated with bits 10:9 of MacTxControlLow */
#define TXC_AMPDU_SHIFT		9	/* shift for ampdu settings */
#define TXC_AMPDU_NONE		0	/* Regular MPDU, not an A-MPDU */
#define TXC_AMPDU_FIRST		1	/* first MPDU of an A-MPDU */
#define TXC_AMPDU_MIDDLE	2	/* intermediate MPDU of an A-MPDU */
#define TXC_AMPDU_LAST		3	/* last (or single) MPDU of an A-MPDU */

/* MacTxControlLow */
#define TXC_AMIC		0x8000
#define TXC_USERIFS		0x4000
#define TXC_LIFETIME		0x2000
#define	TXC_FRAMEBURST		0x1000
#define	TXC_SENDCTS		0x0800
#define TXC_AMPDU_MASK		0x0600
#define TXC_BW_40		0x0100
#define TXC_FREQBAND_5G		0x0080
#define	TXC_DFCS		0x0040
#define	TXC_IGNOREPMQ		0x0020
#define	TXC_HWSEQ		0x0010
#define	TXC_STARTMSDU		0x0008
#define	TXC_SENDRTS		0x0004
#define	TXC_LONGFRAME		0x0002
#define	TXC_IMMEDACK		0x0001

/* MacTxControlHigh */
#define	TXC_AMPDU_FBR		0x1000	/* use fallback rate for this AMPDU */
#define	TXC_SECKEY_MASK		0x0FF0
#define	TXC_SECKEY_SHIFT	4
#define	TXC_SECTYPE_MASK	0x0007
#define	TXC_SECTYPE_SHIFT	0

/* Null delimiter for Fallback rate */
#define AMPDU_FBR_NULL_DELIM  5 /* Location of Null delimiter count for AMPDU */

/* PhyTxControl */
#define	PHY_TXC_PWR_MASK	0xFC00
#define	PHY_TXC_PWR_SHIFT	10
#define	PHY_TXC_ANT_MASK	0x03C0
#define	PHY_TXC_ANT_SHIFT	6
#define	PHY_TXC_ANT_0_1		0x00C0
#define	PHY_TXC_LPPHY_ANT_LAST	0x0000
#define	PHY_TXC_ANT_3		0x0200
#define	PHY_TXC_ANT_2		0x0100
#define	PHY_TXC_ANT_1		0x0080
#define	PHY_TXC_ANT_0		0x0040
#define	PHY_TXC_SHORT_HDR	0x0010
#define PHY_TXC_FT_MASK		0x0003
#define	PHY_TXC_FT_CCK		0x0000
#define	PHY_TXC_FT_OFDM		0x0001
#define	PHY_TXC_FT_EWC		0x0002
#define	PHY_TXC_FT_11N		0x0003

#define	PHY_TXC_OLD_ANT_0	0x0000
#define	PHY_TXC_OLD_ANT_1	0x0100
#define	PHY_TXC_OLD_ANT_LAST	0x0300

/* PhyTxControl_1 */
#define PHY_TXC1_BW_MASK	0x0007
#define PHY_TXC1_BW_10MHZ	0
#define PHY_TXC1_BW_10MHZ_UP	1
#define PHY_TXC1_BW_20MHZ	2
#define PHY_TXC1_BW_20MHZ_UP	3
#define PHY_TXC1_BW_40MHZ	4
#define PHY_TXC1_BW_40MHZ_DUP	5
#define PHY_TXC1_MODE_SHIFT	3
#define PHY_TXC1_MODE_MASK	0x0038
#define PHY_TXC1_MODE_SISO	0
#define PHY_TXC1_MODE_CDD	1
#define PHY_TXC1_MODE_STBC	2
#define PHY_TXC1_MODE_SDM	3
#define	PHY_TXC1_CODE_RATE_SHIFT	8
#define	PHY_TXC1_CODE_RATE_MASK		0x0700
#define	PHY_TXC1_CODE_RATE_1_2		0
#define	PHY_TXC1_CODE_RATE_2_3		1
#define	PHY_TXC1_CODE_RATE_3_4		2
#define	PHY_TXC1_CODE_RATE_4_5		3
#define	PHY_TXC1_CODE_RATE_5_6		4
#define	PHY_TXC1_CODE_RATE_7_8		6
#define	PHY_TXC1_MOD_SCHEME_SHIFT	11
#define	PHY_TXC1_MOD_SCHEME_MASK	0x3800
#define	PHY_TXC1_MOD_SCHEME_BPSK	0
#define	PHY_TXC1_MOD_SCHEME_QPSK	1
#define	PHY_TXC1_MOD_SCHEME_QAM16	2
#define	PHY_TXC1_MOD_SCHEME_QAM64	3
#define	PHY_TXC1_MOD_SCHEME_QAM256	4


/* XtraFrameTypes */
#define XFTS_RTS_FT_SHIFT	2
#define XFTS_FBRRTS_FT_SHIFT	4
#define XFTS_CHANNEL_SHIFT	8

/* Antenna diversity bit in ant_wr_settle */
#define	PHY_AWS_ANTDIV		0x2000

/* PHY CRS states */
#define	APHY_CRS_RESET			0
#define	APHY_CRS_SEARCH			1
#define	APHY_CRS_CLIP			3
#define	APHY_CRS_G_CLIP_POW1		4
#define	APHY_CRS_G_CLIP_POW2		5
#define	APHY_CRS_G_CLIP_NRSSI1		6
#define	APHY_CRS_G_CLIP_NRSSI1_POW1	7
#define	APHY_CRS_G_CLIP_NRSSI2		8

/* IFS ctl */
#define IFS_USEEDCF	(1 << 2)

/* tx status packet */
typedef struct tx_status tx_status_t;
struct tx_status {
	uint16 framelen;
	uint16 PAD;
	uint16 frameid;
	uint16 status;
	uint16 lasttxtime;
	uint16 sequence;
	uint16 phyerr;
	uint16 ackphyrxsh;
} PACKED;

#define	TXSTATUS_LEN	16

/* status field bit definitions */
#define	TX_STATUS_FRM_RTX_MASK	0xF000
#define	TX_STATUS_FRM_RTX_SHIFT	12
#define	TX_STATUS_RTS_RTX_MASK	0x0F00
#define	TX_STATUS_RTS_RTX_SHIFT	8
#define TX_STATUS_MASK		0x00FE
#define	TX_STATUS_PMINDCTD	(1 << 7)	/* PM mode indicated to AP */
#define	TX_STATUS_INTERMEDIATE	(1 << 6)	/* intermediate or 1st ampdu pkg */
#define	TX_STATUS_AMPDU		(1 << 5)	/* AMPDU status */
#define TX_STATUS_SUPR_MASK	0x1C		/* suppress status bits (4:2) */
#define TX_STATUS_SUPR_SHIFT	2
#define	TX_STATUS_ACK_RCV	(1 << 1)	/* ACK received */
#define	TX_STATUS_VALID		(1 << 0)	/* Tx status valid (corerev >= 5) */
#define	TX_STATUS_NO_ACK	0

/* suppress status reason codes */
#define	TX_STATUS_SUPR_PMQ	(1 << 2)	/* PMQ entry */
#define	TX_STATUS_SUPR_FLUSH	(2 << 2)	/* flush request */
#define	TX_STATUS_SUPR_FRAG	(3 << 2)	/* previous frag failure */
#define	TX_STATUS_SUPR_TBTT	(3 << 2)	/* SHARED: Probe response supr for TBTT */
#define	TX_STATUS_SUPR_BADCH	(4 << 2)	/* channel mismatch */
#define	TX_STATUS_SUPR_EXPTIME	(5 << 2)	/* lifetime expiry */
#define	TX_STATUS_SUPR_UF	(6 << 2)	/* underflow */
#define	TX_STATUS_SUPR_NACK	(7 << 2)	/* afterburner NACK */

/* Unexpected tx status for rate update */
#define TX_STATUS_UNEXP(status) \
	(((status & (TX_STATUS_SUPR_MASK | TX_STATUS_INTERMEDIATE)) != 0) && \
	 ((status & (TX_STATUS_SUPR_MASK | TX_STATUS_INTERMEDIATE)) != TX_STATUS_SUPR_EXPTIME) && \
	  ((status & (TX_STATUS_SUPR_MASK | TX_STATUS_INTERMEDIATE)) != TX_STATUS_SUPR_NACK))

/* Unexpected tx status for A-MPDU rate update */
#define TX_STATUS_UNEXP_AMPDU(status) \
	(((status & (TX_STATUS_SUPR_MASK)) != 0) && \
	 ((status & (TX_STATUS_SUPR_MASK)) != TX_STATUS_SUPR_EXPTIME) && \
	  ((status & (TX_STATUS_SUPR_MASK)) != TX_STATUS_SUPR_NACK))

#define TX_STATUS_BA_BMAP03_MASK	0xF000	/* ba bitmap 0:3 in 1st pkg */
#define TX_STATUS_BA_BMAP03_SHIFT	12	/* ba bitmap 0:3 in 1st pkg */
#define TX_STATUS_BA_BMAP47_MASK	0x001E	/* ba bitmap 4:7 in 2nd pkg */
#define TX_STATUS_BA_BMAP47_SHIFT	3	/* ba bitmap 4:7 in 2nd pkg */


/* RXE (Receive Engine) */

/* RCM_CTL */
#define	RCM_INC_MASK_H		0x0080
#define	RCM_INC_MASK_L		0x0040
#define	RCM_INC_DATA		0x0020
#define	RCM_INDEX_MASK		0x001F
#define	RCM_SIZE		15

#define	RCM_MAC_OFFSET		0	/* current MAC address */
#define	RCM_BSSID_OFFSET	3	/* current BSSID address */
#define	RCM_F_BSSID_0_OFFSET	6	/* foreign BSS CFP tracking */
#define	RCM_F_BSSID_1_OFFSET	9	/* foreign BSS CFP tracking */
#define	RCM_F_BSSID_2_OFFSET	12	/* foreign BSS CFP tracking */

/* PSM Block */

/* psm_phy_hdr_param bits */
#define MAC_PHY_RESET		1
#define MAC_PHY_CLOCK_EN	2
#define MAC_PHY_FORCE_CLK	4

/* WEP Block */

/* WEP_WKEY */
#define	WKEY_START		(1 << 8)
#define	WKEY_SEL_MASK		0x1F

/* WEP data formats */

/* max keys in rcmta block */
#define	WSEC_MAX_RCMTA_KEYS	54

/* max keys in M_TKMICKEYS_BLK */
#define	WSEC_MAX_TKMIC_ENGINE_KEYS		12 /* 8 + 4 default */

/* SECKINDXALGO (Security Key Index & Algorithm Block) word format */
/* SKL (Security Key Lookup) */
#define	SKL_INDEX_MASK		0xF0
#define	SKL_INDEX_SHIFT		4
#define	SKL_ALGO_MASK		0x07
#define	SKL_ALGO_SHIFT		0

/* additional bits defined for IBSS group key support */
#define SKL_IBSS_KEYID1_MASK	0x600
#define SKL_IBSS_KEYID1_SHIFT	9
#define SKL_IBSS_KEYID2_MASK	0x1800
#define SKL_IBSS_KEYID2_SHIFT	11
#define SKL_IBSS_KEYALGO_MASK	0xE000
#define SKL_IBSS_KEYALGO_SHIFT	13

#define	WSEC_MODE_OFF		0
#define	WSEC_MODE_HW		1
#define	WSEC_MODE_SW		2

#define	WSEC_ALGO_OFF		0
#define	WSEC_ALGO_WEP1		1
#define	WSEC_ALGO_TKIP		2
#define	WSEC_ALGO_AES		3
#define	WSEC_ALGO_WEP128	4
#define	WSEC_ALGO_AES_LEGACY	5
#define	WSEC_ALGO_NALG		6

#define	AES_MODE_NONE		0
#define	AES_MODE_CCM		1
#define	AES_MODE_OCB_MSDU	2
#define	AES_MODE_OCB_MPDU	3

/* WEP_CTL (Rev 0) */
#define	WECR0_KEYREG_SHIFT	0
#define	WECR0_KEYREG_MASK	0x7
#define	WECR0_DECRYPT		(1 << 3)
#define	WECR0_IVINLINE		(1 << 4)
#define	WECR0_WEPALG_SHIFT	5
#define	WECR0_WEPALG_MASK	(0x7 << 5)
#define	WECR0_WKEYSEL_SHIFT	8
#define	WECR0_WKEYSEL_MASK	(0x7 << 8)
#define	WECR0_WKEYSTART		(1 << 11)
#define	WECR0_WEPINIT		(1 << 14)
#define	WECR0_ICVERR		(1 << 15)

/* Frame template map byte offsets */
#define	T_ACTS_TPL_BASE		(0)
#define	T_NULL_TPL_BASE		(0xc * 2)
#define	T_QNULL_TPL_BASE	(0x1c * 2)
#define	T_RR_TPL_BASE		(0x2c * 2)
#define	T_BCN0_TPL_BASE		(0x34 * 2)
#define	T_PRS_TPL_BASE		(0x134 * 2)
#define	T_BCN1_TPL_BASE		(0x234 * 2)


#define T_BA_TPL_BASE		T_QNULL_TPL_BASE	/* template area for BA */

#define T_RAM_ACCESS_SZ		4	/* template ram is 4 byte access only */

/* Shared Mem byte offsets */

/* Location where the ucode expects the corerev */
#define	M_MACHW_VER		(0x00b * 2)

/* Location where the ucode expects the MAC capabilities */
#define	M_MACHW_CAP_L		(0x060 * 2)
#define	M_MACHW_CAP_H	(0x061 * 2)

/* WME shared memory */
#define M_EDCF_STATUS_OFF	(0x007 * 2)
#define M_TXF_CUR_INDEX		(0x018 * 2)
#define M_EDCF_QINFO		(0x120 * 2)

/* PS-mode related parameters */
#define	M_DOT11_SLOT		(0x008 * 2)
#define	M_DOT11_DTIMPERIOD	(0x009 * 2)
#define	M_NOSLPZNATDTIM		(0x026 * 2)

/* Beacon-related parameters */
#define	M_BCN0_FRM_BYTESZ	(0x00c * 2)	/* Bcn 0 template length */
#define	M_BCN1_FRM_BYTESZ	(0x00d * 2)	/* Bcn 1 template length */
#define	M_BCN_TXTSF_OFFSET	(0x00e * 2)
#define	M_TIMBPOS_INBEACON	(0x00f * 2)
#define	M_SFRMTXCNTFBRTHSD	(0x022 * 2)
#define	M_LFRMTXCNTFBRTHSD	(0x023 * 2)
#define	M_BCN_PCTLWD		(0x02a * 2)

/* MAX Rx Frame len */
#define M_MAXRXFRM_LEN		(0x010 * 2)

/* ACK/CTS related params */
#define	M_RSP_PCTLWD		(0x011 * 2)

/* Hardware Power Control */
#define M_TXPWR_N		(0x012 * 2)
#define M_TXPWR_TARGET		(0x013 * 2)
#define M_TXPWR_MAX		(0x014 * 2)
#define M_TXPWR_CUR		(0x019 * 2)

/* Rx-related paramenters */
#define	M_RX_PAD_DATA_OFFSET	(0x01a * 2)

/* WEP Shared mem data */
#define	M_SEC_DEFIVLOC		(0x01e * 2)
#define	M_SEC_VALNUMSOFTMCHTA	(0x01f * 2)
#define	M_PHYVER		(0x028 * 2)
#define	M_PHYTYPE		(0x029 * 2)
#define	M_SECRXKEYS_PTR		(0x02b * 2)
#define	M_TKMICKEYS_PTR		(0x059 * 2)
#define	M_SECKINDXALGO_BLK	(0x2ea * 2)
#define	M_SECPSMRXTAMCH_BLK	(0x2fa * 2)
#define	M_TKIP_TSC_TTAK		(0x18c * 2)
#define	D11_MAX_KEY_SIZE	16

#define	M_MAX_ANTCNT		(0x02e * 2)	/* antenna swap threshold */

/* Probe response related parameters */
#define	M_SSIDLEN		(0x024 * 2)
#define	M_PRB_RESP_FRM_LEN	(0x025 * 2)
#define	M_PRS_MAXTIME		(0x03a * 2)
#define	M_SSID			(0xb0 * 2)
#define	M_CTXPRS_BLK		(0xc0 * 2)
#define	C_CTX_PCTLWD_POS	(4 * 2)


/* Delta between OFDM and CCK power in CCK power boost mode */
#define M_OFDM_OFFSET		(0x027 * 2)

/* TSSI for last 4 11b/g CCK packets transmited */
#define	M_B_TSSI_0		(0x02c * 2)
#define	M_B_TSSI_1		(0x02d * 2)

/* Host flags to turn on ucode options */
#define	M_HOST_FLAGS1		(0x02f * 2)
#define	M_HOST_FLAGS2		(0x030 * 2)
#define	M_HOST_FLAGS3		(0x031 * 2)
#define	M_HOST_FLAGS_SZ		16

#define M_RADAR_REG		(0x033 * 2)

/* TSSI for last 4 11a OFDM packets transmited */
#define	M_A_TSSI_0		(0x034 * 2)
#define	M_A_TSSI_1		(0x035 * 2)

#define	M_RF_RX_SP_REG1		(0x036 * 2)

/* TSSI for last 4 11g OFDM packets transmited */
#define	M_G_TSSI_0		(0x038 * 2)
#define	M_G_TSSI_1		(0x039 * 2)

/* Background noise measure */
#define	M_JSSI_0		(0x44 * 2)
#define	M_JSSI_1		(0x45 * 2)
#define	M_JSSI_AUX		(0x46 * 2)

/* TX fifo sizes */
#define M_FIFOSIZE0		(0x4c * 2)
#define M_FIFOSIZE1		(0x4d * 2)
#define M_FIFOSIZE2		(0x4e * 2)
#define M_FIFOSIZE3		(0x4f * 2)
#define D11_MAX_TX_FRMS		32		/* max frames allowed in tx fifo */

/* Synthpu WAR */
#define M_CURCHANNEL		(0x50 * 2)
#define D11_CURCHANNEL_5G	0x0100;
#define D11_CURCHANNEL_40	0x0200;
#define D11_CURCHANNEL_MAX	0x00FF;

/* last posted frameid on the bcmc fifo */
#define M_BCMC_FID		(0x54 * 2)
#define INVALIDFID		0xffff

/* extended beacon phyctl bytes for 11N */
#define	M_BCN_PCTL1WD		(0x058 * 2)

/* Rate table offsets */
#define	M_RT_DIRMAP_A		(0xe0 * 2)
#define	M_RT_BBRSMAP_A		(0xf0 * 2)
#define	M_RT_DIRMAP_B		(0x100 * 2)
#define	M_RT_BBRSMAP_B		(0x110 * 2)

/* Rate table entry offsets */
#define	M_RT_PRS_PLCP_POS	10
#define	M_RT_PRS_DUR_POS	16
#define	M_RT_OFDM_PCTL1_POS	18

#define M_20IN40_IQ			(0x380 * 2)

/* SHM locations where ucode stores the current power index */
#define M_CURR_IDX1		(0x384 *2)
#define M_CURR_IDX2		(0x387 *2)

#define M_BSCALE_ANT0	(0x5e * 2)
#define M_BSCALE_ANT1	(0x5f * 2)

/* Antenna Diversity Testing */
#define M_MIMO_ANTSEL_RXDFLT	(0x63 * 2)
#define M_ANTSEL_CLKDIV	(0x61 * 2)
#define M_MIMO_ANTSEL_TXDFLT	(0x64 * 2)
#define M_MIMO_ANTSEL_RXUNID	(0x65 * 2)

#define ANTSEL_CLKDIV_4MHZ	6
#define MIMO_ANTSEL_BUSY	0x4000 /* bit 14 (busy) */
#define MIMO_ANTSEL_SEL		0x8000 /* bit 15 write the value */
#define MIMO_ANTSEL_WAIT	50	/* 50us wait */
#define MIMO_ANTSEL_OVERRIDE  0x8000 /* flag */

typedef struct shm_acparams shm_acparams_t;
struct shm_acparams {
	uint16	txop;
	uint16	cwmin;
	uint16	cwmax;
	uint16	cwcur;
	uint16	aifs;
	uint16	bslots;
	uint16	reggap;
	uint16	status;
	uint16	rsvd[8];
} PACKED;
#define M_EDCF_QLEN	(16 * 2)

#define WME_STATUS_NEWAC	(1 << 8)

/* M_HOST_FLAGS */
#define MHFMAX		3 /* Number of valid hostflag half-word (uint16) */
#define MHF1		0 /* Hostflag 1 index */
#define MHF2		1 /* Hostflag 2 index */
#define MHF3		2 /* Hostflag 3 index */

/* Flags in M_HOST_FLAGS */
#define	MHF1_ANTDIV		0x0001		/* Enable ucode antenna diversity help */
#define	MHF1_SYMWAR		0x0002		
#define	MHF1_RXPUWAR		0x0004		
#define	MHF1_MBSS_EN		0x0004		/* Enable MBSS: RXPUWAR deprecated for rev >= 9 */
#define	MHF1_CCKPWR		0x0008		/* Enable 4 Db CCK power boost */
#define	MHF1_BTCOEXIST		0x0010		/* Enable Bluetooth / WLAN coexistence */
#define	MHF1_DCFILTWAR		0x0020		/* Enable g-mode DC canceller filter bw WAR */
#define	MHF1_OFDMPWR		0x0040		/* Enable PA gain boost for OFDM frames */
#define	MHF1_ACPRWAR		0x0080		/* Enable ACPR.  Disable for Japan, channel 14 */
#define	MHF1_EDCF		0x0100		/* Enable EDCF access control */
#define MHF1_20IN40_IQ_WAR	0x0200		
#define	MHF1_FORCEFASTCLK	0x0400		/* Disable Slow clock request */
#define	MHF1_ACI		0x0800		/* Enable ACI war: shiftbits by 2 on PHY_CRS */
#define	MHF1_AWAR		0x1000		/* Toggle bit 11 of the 2060's rx-gm_updn register
						 * on rx/tx/rx transitions
						 */
#define MHF1_RADARWAR		0x2000		
#define MHF1_DEFKEYVALID	0x4000		/* Enable use of the default keys */
#define	MHF1_AFTERBURNER	0x8000		/* Enable afterburner */

/* Flags in M_HOST_FLAGS2 */
#define MHF2_BT4PCOEX		0x0001		/* Bluetooth 4-priority coexistence */
#define MHF2_FASTWAKE		0x0002		
#define MHF2_SYNTHPUWAR		0x0004		/* force VCO recal when powerup synthpu */
#define MHF2_PCISLOWCLKWAR	0x0008		
#define MHF2_PIO_RECVING	0x0020		
#define MHF2_TXBCMC_NOW		0x0040		/* Flush BCMC FIFO immediately */
#define MHF2_HWPWRCTL		0x0080		/* Enable hw power control */
#define MHF2_BTCMOD		0x0100		/* BTC in alternate pins */
#define MHF2_BTCPREMPT		0x0200		/* BTC enable bluetooth check during tx */
#define MHF2_SKIP_CFP_UPDATE	0x0400		/* Skip CFP update */
#define MHF2_NPHY40MHZ_WAR	0x0800		
#define MHF2_BTHWCOEX		0x2000		/* Bluetooth 3-wire hardware coexistence */
#define MHF2_BTCANTMODE		0x4000		/* Bluetooth coexistence  antenna mode */

/* Flags in M_HOST_FLAGS3 */
#define MHF3_ANTSEL_EN		0x0001		/* enabled antenna selection */
#define MHF3_ANTSEL_MODE	0x0002		/* antenna selection mode: */
#define MHF3_BTCX_DEF_BT	0x0004		/* corerev >= 13 BT Coex. */
#define MHF3_BTCX_DIS_C2S	0x0008		/* corerev >= 13 BT Coex. */
#define MHF3_NPHY_MLADV_WAR	0x0010		
#define MHF3_BTCX_FIXED_PRD	0x0100		/* corerev >= 13 BT Coex. */
#define MHF3_BTCX_PS_PROTECT	0x0200		/* corerev >= 13 BT Coex. */
#define MHF3_WAKE_MODE		0x0400		
#define MHF3_PR45960_WAR	0x0800		


/* Radio power setting for ucode */
#define	M_RADIO_PWR		(0x32 * 2)

/* phy noise recorded by ucode right after tx */
#define	M_PHY_NOISE		(0x037 * 2)
#define	PHY_NOISE_MASK		0x00ff

/* Receive Frame Data Header for 802.11b DCF-only frames */
typedef struct d11rxhdr d11rxhdr_t;
struct d11rxhdr {
	uint16	RxFrameSize;		/* Actual byte length of the frame data received */
	uint16	PAD;			/* Reserved */
	uint16	PhyRxStatus_0;		/* PhyRxStatus 15:0 */
	uint16	PhyRxStatus_1;		/* PhyRxStatus 31:16 */
	uint16	PhyRxStatus_2;		/* PhyRxStatus 47:32 */
	uint16	PhyRxStatus_3;		/* PhyRxStatus 63:48 */
	uint16	RxStatus1;		/* MAC Rx Status */
	uint16	RxStatus2;		/* extended MAC Rx status */
	uint16	RxTSFTime;		/* RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY */
	uint16	RxChan;			/* gain code, channel radio code, and phy type */
} PACKED;

#define	RXHDR_LEN		20	/* sizeof d11rxhdr_t */

#define	FRAMELEN(h)		((h)->RxFrameSize)

/* PhyRxStatus_0: */
#define	PRXS0_FT_MASK		0x0003	/* NPHY only: CCK, OFDM, preN, N */
#define	PRXS0_CLIP_MASK		0x000C	/* NPHY only: clip count adjustment steps by AGC */
#define	PRXS0_CLIP_SHIFT	2
#define	PRXS0_UNSRATE		0x0010	/* PHY received a frame with unsupported rate */
#define	PRXS0_RXANT_UPSUBBAND	0x0020	/* GPHY: rx ant, NPHY: upper sideband */
#define	PRXS0_LCRS		0x0040	/* CCK frame only: lost crs during cck frame reception */
#define	PRXS0_SHORTH		0x0080	/* Short Preamble */
#define	PRXS0_PLCPFV		0x0100	/* PLCP violation */
#define	PRXS0_PLCPHCF		0x0200	/* PLCP header integrity check failed */
#define	PRXS0_GAIN_CTL		0x4000	/* legacy PHY gain control */
#define PRXS0_ANTSEL_MASK	0xF000	/* NPHY: Antennas used for received frame, bitmask */
#define PRXS0_ANTSEL_SHIFT	0x12

	/* subfield PRXS0_FT_MASK */
#define	PRXS0_CCK		0x0000
#define	PRXS0_OFDM		0x0001	/* valid only for G phy, use rxh->RxChan for A phy */
#define	PRXS0_PREN		0x0002
#define	PRXS0_STDN		0x0003

	/* subfield PRXS0_ANTSEL_MASK */
#define PRXS0_ANTSEL_0		0x0	/* antenna 0 is used */
#define PRXS0_ANTSEL_1		0x2	/* antenna 1 is used */
#define PRXS0_ANTSEL_2		0x4	/* antenna 2 is used */
#define PRXS0_ANTSEL_3		0x8	/* antenna 3 is used */

/* PhyRxStatus_1: */
#define	PRXS1_JSSI_MASK		0x00FF
#define	PRXS1_JSSI_SHIFT	0
#define	PRXS1_SQ_MASK		0xFF00
#define	PRXS1_SQ_SHIFT		8

/* nphy PhyRxStatus_1: */
#define PRXS1_nphy_PWR0_MASK	0x00FF
#define PRXS1_nphy_PWR1_MASK	0xFF00

/* PhyRxStatus_2: */
#define	PRXS2_LNAGN_MASK	0xC000
#define	PRXS2_LNAGN_SHIFT	14
#define	PRXS2_PGAGN_MASK	0x3C00
#define	PRXS2_PGAGN_SHIFT	10
#define	PRXS2_FOFF_MASK		0x03FF

/* nphy PhyRxStatus_2: */
#define PRXS2_nphy_SQ_ANT0	0x000F	/* nphy overall signal quality for antenna 0 */
#define PRXS2_nphy_SQ_ANT1	0x00F0	/* nphy overall signal quality for antenna 0 */
#define PRXS2_nphy_cck_SQ	0x00FF	/* bphy signal quality(when FT field is 0) */
#define PRXS3_nphy_SSQ_MASK	0xFF00	/* spatial conditioning of the two receive channels */
#define PRXS3_nphy_SSQ_SHIFT	8

/* PhyRxStatus_3: */
#define	PRXS3_DIGGN_MASK	0x1800
#define	PRXS3_DIGGN_SHIFT	11
#define	PRXS3_TRSTATE		0x0400

/* nphy PhyRxStatus_3: */
#define PRXS3_nphy_MMPLCPLen_MASK	0x0FFF	/* Mixed-mode preamble PLCP length */
#define PRXS3_nphy_MMPLCP_RATE_MASK	0xF000	/* Mixed-mode preamble rate field */
#define PRXS3_nphy_MMPLCP_RATE_SHIFT	12


/* ucode RxStatus1: */
#define	RXS_BCNSENT		0x8000
#define	RXS_SECKINDX_MASK	0x07e0
#define	RXS_SECKINDX_SHIFT	5
#define	RXS_DECERR		(1 << 4)
#define	RXS_DECATMPT		(1 << 3)
#define	RXS_PBPRES		(1 << 2)	/* PAD bytes to make IP data 4 bytes aligned */
#define	RXS_RESPFRAMETX		(1 << 1)
#define	RXS_FCSERR		(1 << 0)

/* ucode RxStatus2: */
#define RXS_AMSDU_MASK		1
#define	RXS_AGGTYPE_MASK	0x6
#define	RXS_AGGTYPE_SHIFT	1
#define	RXS_AMSDU_FIRST		1
#define	RXS_AMSDU_INTERMEDIATE	0
#define	RXS_AMSDU_LAST		2
#define	RXS_AMSDU_N_ONE		3
#define	RXS_TKMICATMPT		(1 << 3)
#define	RXS_TKMICERR		(1 << 4)
#define	RXS_PHYRXST_VALID	(1 << 8)




/* RxChan */
#define RXS_CHAN_40		0x1000
#define RXS_CHAN_5G		0x0800
#define	RXS_CHAN_ID_MASK	0x07f8
#define	RXS_CHAN_ID_SHIFT	3
#define	RXS_CHAN_PHYTYPE_MASK	0x0007
#define	RXS_CHAN_PHYTYPE_SHIFT	0

/* Index of attenuations used during ucode power control. */
#define M_PWRIND_BLKS	(0x188 * 2)
#define M_PWRIND_MAP0	(M_PWRIND_BLKS + 0x0)
#define M_PWRIND_MAP1	(M_PWRIND_BLKS + 0x2)
#define M_PWRIND_MAP2	(M_PWRIND_BLKS + 0x4)
#define M_PWRIND_MAP3	(M_PWRIND_BLKS + 0x6)

/* PSM SHM variable offsets */
#define	M_PSM_SOFT_REGS	0x0
#define	M_BOM_REV_MAJOR	(M_PSM_SOFT_REGS + 0x0)
#define	M_BOM_REV_MINOR	(M_PSM_SOFT_REGS + 0x2)
#define	M_UCODE_DATE	(M_PSM_SOFT_REGS + 0x4)		/* 4:4:8 year:month:day format */
#define	M_UCODE_TIME	(M_PSM_SOFT_REGS + 0x6)		/* 5:6:5 hour:min:sec format */
#define	M_UCODE_DBGST	(M_PSM_SOFT_REGS + 0x40)	/* ucode debug status code */
#define	M_UCODE_MACSTAT	(M_PSM_SOFT_REGS + 0xE0)	/* macstat counters */

#define	M_MBURST_SIZE	(0x40 * 2)			/* max frames in a frameburst */
#define	M_MBURST_TXOP	(0x41 * 2)			/* max frameburst TXOP in unit of us */
#define M_SYNTHPU_DLY	(0x4a * 2)			/* pre-wakeup for synthpu, default: 500 */
#define	M_PRETBTT	(0x4b * 2)


/* ucode debug status codes */
#define	DBGST_INACTIVE		0		/* not valid really */
#define	DBGST_INIT		1		/* after zeroing SHM, before suspending at init */
#define	DBGST_ACTIVE		2		/* "normal" state */
#define	DBGST_SUSPENDED		3		/* suspended */
#define	DBGST_ASLEEP		4		/* asleep (PS mode) */

/* Scratch Reg defs */
#define	S_DOT11_CWMIN		3		/* Contention window min */
#define	S_DOT11_CWMAX		4		/* Contention window max */
#define	S_DOT11_CWCUR		5		/* Contention window current */
#define	S_DOT11_SRC_LMT		6		/* short retry count limit */
#define	S_DOT11_LRC_LMT		7		/* long retry count limit */
#define	S_DOT11_DTIMCOUNT	8		/* current DTIM count */
#define	S_BCN0_FRM_BYTESZ	21		/* Beacon 0 template length */
#define	S_BCN1_FRM_BYTESZ	22		/* Beacon 1 template length */
#define	S_SFRMTXCNTFBRTHSD	23		/* short frame tx count threshold for rate
						 * fallback
						 */
#define	S_LFRMTXCNTFBRTHSD	24		/* long frame tx count threshold */

/* IHR offsets */
#define TSF_TMR_TSF_L		0x119
#define TSF_TMR_TSF_ML		0x11A
#define TSF_TMR_TSF_MU		0x11B
#define TSF_TMR_TSF_H		0x11C

#define TSF_GPT_0_STAT		0x123
#define TSF_GPT_1_STAT		0x124
#define TSF_GPT_0_CTR_L		0x125
#define TSF_GPT_1_CTR_L		0x126
#define TSF_GPT_0_CTR_H		0x127
#define TSF_GPT_1_CTR_H		0x128
#define TSF_GPT_0_VAL_L		0x129
#define TSF_GPT_1_VAL_L		0x12A
#define TSF_GPT_0_VAL_H		0x12B
#define TSF_GPT_1_VAL_H		0x12C

/* GPT_2 is corerev >= 3 */
#define TSF_GPT_2_STAT		0x133
#define TSF_GPT_2_CTR_L		0x134
#define TSF_GPT_2_CTR_H		0x135
#define TSF_GPT_2_VAL_L		0x136
#define TSF_GPT_2_VAL_H		0x137

/* Slow timer registers */
#define SLOW_CTRL				0x150
#define SLOW_TIMER_L			0x151
#define SLOW_TIMER_H		0x152
#define SLOW_FRAC				0x153
#define FAST_PWRUP_DLY		0x154

/* IHR TSF_GPT STAT values */
#define TSF_GPT_PERIODIC	(1 << 12)
#define TSF_GPT_ADJTSF		(1 << 13)
#define TSF_GPT_USETSF		(1 << 14)
#define TSF_GPT_ENABLE		(1 << 15)

/* IHR SLOW_CTRL values */
#define SLOW_CTRL_PDE		(1 << 0)
#define SLOW_CTRL_FD		(1 << 8)


/* ucode mac statistic counters in shared memory */
typedef struct macstat {
	uint16	txallfrm;		/* 0x80 */
	uint16	txrtsfrm;		/* 0x82 */
	uint16	txctsfrm;		/* 0x84 */
	uint16	txackfrm;		/* 0x86 */
	uint16	txdnlfrm;		/* 0x88 */
	uint16	txbcnfrm;		/* 0x8a */
	uint16	txfunfl[8];		/* 0x8c - 0x9b */
	uint16	txtplunfl;		/* 0x9c */
	uint16	txphyerr;		/* 0x9e */
	uint16	PAD[2];
	uint16	rxfrmtoolong;		/* 0xa4 */
	uint16	rxfrmtooshrt;		/* 0xa6 */
	uint16	rxinvmachdr;		/* 0xa8 */
	uint16	rxbadfcs;		/* 0xaa */
	uint16	rxbadplcp;		/* 0xac */
	uint16	rxcrsglitch;		/* 0xae */
	uint16	rxstrt;			/* 0xb0 */
	uint16	rxdfrmucastmbss;	/* 0xb2 */
	uint16	rxmfrmucastmbss;	/* 0xb4 */
	uint16	rxcfrmucast;		/* 0xb6 */
	uint16	rxrtsucast;		/* 0xb8 */
	uint16	rxctsucast;		/* 0xba */
	uint16	rxackucast;		/* 0xbc */
	uint16	rxdfrmocast;		/* 0xbe */
	uint16	rxmfrmocast;		/* 0xc0 */
	uint16	rxcfrmocast;		/* 0xc2 */
	uint16	rxrtsocast;		/* 0xc4 */
	uint16	rxctsocast;		/* 0xc6 */
	uint16	rxdfrmmcast;		/* 0xc8 */
	uint16	rxmfrmmcast;		/* 0xca */
	uint16	rxcfrmmcast;		/* 0xcc */
	uint16	rxbeaconmbss;		/* 0xce */
	uint16	rxdfrmucastobss;	/* 0xd0 */
	uint16	rxbeaconobss;		/* 0xd2 */
	uint16	rxrsptmout;		/* 0xd4 */
	uint16	bcntxcancl;		/* 0xd6 */
	uint16	PAD;
	uint16	rxf0ovfl;		/* 0xda */
	uint16	rxf1ovfl;		/* 0xdc */
	uint16	rxf2ovfl;		/* 0xde */
	uint16	txsfovfl;		/* 0xe0 */
	uint16	pmqovfl;		/* 0xe2 */
	uint16	rxcgprqfrm;		/* 0xe4 */
	uint16	rxcgprsqovfl;		/* 0xe6 */
	uint16	txcgprsfail;		/* 0xe8 */
	uint16	txcgprssuc;		/* 0xea */
	uint16	prs_timeout;		/* 0xec */
	uint16	rxnack;			/* 0xee afterburner: received nacks */
	uint16	frmscons;		/* 0xf0 afterburner: frames consumed waiting requeue */
	uint16	txnack;			/* 0xf2 afterburner: transmitted nacks */
	uint16	txglitch_nack;		/* 0xf4 afterburner: subset of above */
	uint16	txburst;		/* 0xf6 # tx bursts */
	uint16	rxburst;		/* 0xf8 # rx bursts */
	uint16	phywatchdog;		/* 0xfa # of phy watchdog events */
} macstat_t;

/* dot11 core-specific sbtmstatelow flags */
#define	SBTML_CE		((uint32)1 << 18)	/* PHY clock enable */
#define	SBTML_PR		((uint32)1 << 19)	/* PHY reset */
#define	SBTML_MP		((uint32)1 << 20)	/* MAC PHY clockcontrol enable */
#define	SBTML_FS		((uint32)1 << 21)	/* PLL FreqRefSelect (corerev >= 5) */
/* NOTE: the following PHYCLK bits only apply when the core is attached
 * to a NPHY (and corerev >= 11 which it will always be for NPHYs).
 */
#define SBTML_PHYCLK_SHIFT	22			/* phy clock shift */
#define SBTML_PHYCLK_MASK	0x00C00000		/* phy clock mask (b22 & b23) */
#define SBTML_40MHZ_PHYCLK	0			/* 40MHZ phyclk */
#define SBTML_80MHZ_PHYCLK	1			/* 80MHZ phyclk */
#define SBTML_160MHZ_PHYCLK	2			/* 160MHZ phyclk */
#define	SBTML_GM		((uint32)1 << 29)	/* gmode enable */

/* dot11 core-specific sbtmstatehigh flags */
#define	SBTMH_2G_PHY		((uint32)1 << 16)	/* 2.4G capable phy (corerev >= 5) */
#define	SBTMH_5G_PHY		((uint32)1 << 17)	/* 5G capable phy (corerev >= 5) */
#define	SBTMH_FCA		((uint32)1 << 18)	/* FastClkAvailable (corerev >= 5) */
#define	SBTMH_DB_PHY		((uint32)1 << 19)	/* Dualband phy (corerev >= 11) */

#define	BPHY_REG_OFT_BASE	0x0
/* offsets for indirect access to bphy registers */
#define	BPHY_BB_CONFIG		0x01
#define	BPHY_ADCBIAS		0x02
#define	BPHY_ANACORE		0x03
#define	BPHY_PHYCRSTH		0x06
#define	BPHY_TEST		0x0a
#define	BPHY_PA_TX_TO		0x10
#define	BPHY_SYNTH_DC_TO	0x11
#define	BPHY_PA_TX_TIME_UP	0x12
#define	BPHY_RX_FLTR_TIME_UP	0x13
#define	BPHY_TX_POWER_OVERRIDE	0x14
#define	BPHY_RF_OVERRIDE	0x15
#define	BPHY_RF_TR_LOOKUP1	0x16
#define	BPHY_RF_TR_LOOKUP2	0x17
#define	BPHY_COEFFS		0x18
#define	BPHY_PLL_OUT		0x19
#define	BPHY_REFRESH_MAIN	0x1a
#define	BPHY_REFRESH_TO0	0x1b
#define	BPHY_REFRESH_TO1	0x1c
#define	BPHY_RSSI_TRESH		0x20
#define	BPHY_IQ_TRESH_HH	0x21
#define	BPHY_IQ_TRESH_H		0x22
#define	BPHY_IQ_TRESH_L		0x23
#define	BPHY_IQ_TRESH_LL	0x24
#define	BPHY_GAIN		0x25
#define	BPHY_LNA_GAIN_RANGE	0x26
#define	BPHY_JSSI		0x27
#define	BPHY_TSSI_CTL		0x28
#define	BPHY_TSSI		0x29
#define	BPHY_TR_LOSS_CTL	0x2a
#define	BPHY_LO_LEAKAGE		0x2b
#define	BPHY_LO_RSSI_ACC	0x2c
#define	BPHY_LO_IQMAG_ACC	0x2d
#define	BPHY_TX_DC_OFF1		0x2e
#define	BPHY_TX_DC_OFF2		0x2f
#define	BPHY_PEAK_CNT_THRESH	0x30
#define	BPHY_FREQ_OFFSET	0x31
#define	BPHY_DIVERSITY_CTL	0x32
#define	BPHY_PEAK_ENERGY_LO	0x33
#define	BPHY_PEAK_ENERGY_HI	0x34
#define	BPHY_SYNC_CTL		0x35
#define	BPHY_TX_PWR_CTRL	0x36
#define BPHY_TX_EST_PWR 	0x37
#define	BPHY_STEP		0x38
#define	BPHY_WARMUP		0x39
#define	BPHY_LMS_CFF_READ	0x3a
#define	BPHY_LMS_COEFF_I	0x3b
#define	BPHY_LMS_COEFF_Q	0x3c
#define	BPHY_SIG_POW		0x3d
#define	BPHY_RFDC_CANCEL_CTL	0x3e
#define	BPHY_HDR_TYPE		0x40
#define	BPHY_SFD_TO		0x41
#define	BPHY_SFD_CTL		0x42
#define	BPHY_DEBUG		0x43
#define	BPHY_RX_DELAY_COMP	0x44
#define	BPHY_CRS_DROP_TO	0x45
#define	BPHY_SHORT_SFD_NZEROS	0x46
#define	BPHY_DSSS_COEFF1	0x48
#define	BPHY_DSSS_COEFF2	0x49
#define	BPHY_CCK_COEFF1		0x4a
#define	BPHY_CCK_COEFF2		0x4b
#define	BPHY_TR_CORR		0x4c
#define	BPHY_ANGLE_SCALE	0x4d
#define	BPHY_TX_PWR_BASE_IDX	0x4e
#define	BPHY_OPTIONAL_MODES2	0x4f
#define	BPHY_CCK_LMS_STEP	0x50
#define	BPHY_BYPASS		0x51
#define	BPHY_CCK_DELAY_LONG	0x52
#define	BPHY_CCK_DELAY_SHORT	0x53
#define	BPHY_PPROC_CHAN_DELAY	0x54
#define	BPHY_DDFS_ENABLE	0x58
#define	BPHY_PHASE_SCALE	0x59
#define	BPHY_FREQ_CONTROL	0x5a
#define	BPHY_LNA_GAIN_RANGE_10	0x5b
#define	BPHY_LNA_GAIN_RANGE_32	0x5c
#define	BPHY_OPTIONAL_MODES	0x5d
#define	BPHY_RX_STATUS2		0x5e
#define	BPHY_RX_STATUS3		0x5f
#define	BPHY_DAC_CONTROL	0x60
#define	BPHY_ANA11G_FILT_CTRL	0x62
#define	BPHY_REFRESH_CTRL	0x64
#define	BPHY_RF_OVERRIDE2	0x65
#define	BPHY_SPUR_CANCEL_CTRL	0x66
#define	BPHY_FINE_DIGIGAIN_CTRL	0x67
#define	BPHY_SPUR_CANCEL_CTRL	0x66
#define	BPHY_RSSI_LUT		0x88
#define	BPHY_RSSI_LUT_END	0xa7
#define	BPHY_TSSI_LUT		0xa8
#define	BPHY_TSSI_LUT_END	0xc7
#define	BPHY_TSSI2PWR_LUT	0x380
#define	BPHY_TSSI2PWR_LUT_END	0x39f
#define	BPHY_LOCOMP_LUT		0x3a0
#define	BPHY_LOCOMP_LUT_END	0x3bf
#define	BPHY_TXGAIN_LUT		0x3c0
#define	BPHY_TXGAIN_LUT_END	0x3ff

/* Bits in BB_CONFIG: */
#define	PHY_BBC_ANT_MASK	0x0180
#define	PHY_BBC_ANT_SHIFT	7
#define	BB_DARWIN		0x1000
#define BBCFG_RESETCCA		0x4000
#define BBCFG_RESETRX		0x8000

/* Bits in phytest(0x0a): */
#define	TST_DDFS		0x2000
#define	TST_TXFILT1		0x0800
#define	TST_UNSCRAM		0x0400
#define	TST_CARR_SUPP		0x0200
#define	TST_DC_COMP_LOOP	0x0100
#define	TST_LOOPBACK		0x0080
#define	TST_TXFILT0		0x0040
#define	TST_TXTEST_ENABLE	0x0020
#define	TST_TXTEST_RATE		0x0018
#define	TST_TXTEST_PHASE	0x0007

/* phytest txTestRate values */
#define	TST_TXTEST_RATE_1MBPS	0
#define	TST_TXTEST_RATE_2MBPS	1
#define	TST_TXTEST_RATE_5_5MBPS	2
#define	TST_TXTEST_RATE_11MBPS	3
#define	TST_TXTEST_RATE_SHIFT	3

/* Bits in BPHY_RF_OVERRIDE(0x15): */
#define	RFO_FLTR_RX_CTRL_OVR	0x0080
#define	RFO_FLTR_RX_CTRL_VAL	0x0040

/* Bits in BPHY_RF_TR_LOOKUP1(0x16): */

/* Bits in BPHY_RF_TR_LOOKUP2(0x17): */

/* Bits in BPHY_REFRESH_MAIN(0x1a): */
#define	REF_RXPU_TRIG		0x8000
#define	REF_IDLE_TRIG		0x4000
#define	REF_TO0_TRIG		0x2000
#define	REF_TO1_TRIG		0x1000

/* Bits in BPHY_LNA_GAIN_RANGE(0x26): */
#define	LNA_DIGI_GAIN_ENABLE	0x8000
#define	LNA_ON_CTRL		0x4000
#define	LNA_PTR_THRESH		0x0f00
#define	LNA_GAIN_RANGE		0x00ff

/* Bits in BPHY_SYNC_CTL(0x35): */
#define	SYN_ANGLE_START		0x0f00
#define	SYN_TOGGLE_CUTOFF	0x0080
#define	SYN_WARMUP_DUR		0x007f

/* Bits in BPHY_OPTIONAL_MODES	(0x5d): */
#define	OPT_MODE_G		0x4000

/* Aphy regs offset in the gphy */
#define	GPHY_TO_APHY_OFF	0x400

#define	APHY_REG_OFT_BASE	0x0

/* offsets for indirect access to aphy registers */
#define	APHY_PHYVERSION		0x00
#define	APHY_BBCONFIG		0x01
#define	APHY_PWRDWN		0x03
#define	APHY_PHYCRSTH		0x06
#define	APHY_RF_OVERRIDE	0x10
#define	APHY_RF_OVERRIDE_VAL	0x11
#define	APHY_GPIO_OUTEN		0x12
#define	APHY_TR_LUT1		0x13
#define	APHY_TR_LUT2		0x14
#define	APHY_DIGI_GAIN1		0x15
#define	APHY_DIGI_GAIN2		0x16
#define	APHY_DIGI_GAIN3		0x17
#define	APHY_DESIRED_PWR	0x18
#define	APHY_PAR_GAIN_SEL	0x19
#define	APHY_MIN_MAX_GAIN	0x1a
#define	APHY_GAIN_INFO		0x1b
#define	APHY_INIT_GAIN_INDX	0x1c
#define	APHY_CLIP_GAIN_INDX	0x1d
#define	APHY_TRN_INFO		0x1e
#define	APHY_CLIP_BO_THRESH	0x1f
#define	APHY_LPF_GAIN_BO_THRESH	0x20
#define	APHY_ADC_VSQR		0x21
#define	APHY_CCK_CLIP_BO_THRESH	0x22
#define	APHY_CLIP_PWR_THRESH	0x24
#define	APHY_JSSI_OFFSET	0x25
#define	APHY_DC_B0		0x26
#define	APHY_DC_B1		0x27
#define	APHY_DC_A1		0x28
#define	APHY_CTHR_STHR_SHDIN	0x29
#define	APHY_MIN_PWR_GSETTL	0x2a
#define	APHY_ANT_DWELL		0x2b
#define	APHY_RESET_LEN		0x2c
#define	APHY_CLIP_CTR_INIT	0x2d
#define	APHY_ED_TO		0x2e
#define	APHY_CRS_HOLD		0x2f
#define	APHY_PLCP_TMT_STR0_MIN	0x30
#define	APHY_STRN_COLL_MAX_SAMP	0x31
#define	APHY_STRN_MIN_REAL	0x33
#define	APHY_COARSE_UPD_CTL	0x34
#define APHY_IqestEnWaitTime	0x34
#define	APHY_SCALE_FACT_I	0x35
#define APHY_IqestNumSamps	0x35
#define	APHY_SCALE_FACT_Q	0x36
#define APHY_IqestIqAccHi	0x36
#define	APHY_DC_OFFSET_I	0x37
#define APHY_IqestIqAccLo	0x37
#define	APHY_DC_OFFSET_Q	0x38
#define APHY_IqestIpwrAccHi	0x38
#define	APHY_FIX_VAL_OUT_I	0x39
#define APHY_IqestIpwrAccLo	0x39
#define	APHY_FIX_VAL_OUT_Q	0x3a
#define APHY_IqestQpwrAccHi	0x3a
#define	APHY_MAX_SAMP_FINE	0x3b
#define APHY_IqestQpwrAccLo	0x3b
#define	APHY_LTRN_MIN_OFFSET	0x3d
#define	APHY_COMP_CTL		0x3e
#define	APHY_HSQ_MIN_BPSK	0x41
#define	APHY_HSQ_MIN_QPSK	0x42
#define	APHY_HSQ_MIN_16QAM	0x43
#define	APHY_HSQ_MIN_64QAM	0x44
#define	APHY_QUANT_ST_BPSK	0x45
#define	APHY_QUANT_ST_QPSK	0x46
#define	APHY_QUANT_ST_16QAM	0x47
#define	APHY_QUANT_ST_64QAM	0x48
#define	APHY_VITERBI_OFFSET	0x49
#define	APHY_MAX_STEPS		0x4a
#define	APHY_ALPHA1		0x50
#define	APHY_ALPHA2		0x51
#define	APHY_BETA1		0x52
#define	APHY_BETA2		0x53
#define	APHY_NUM_LOOP		0x54
#define	APHY_MU			0x55
#define	APHY_THETA_I		0x56
#define	APHY_THETA_Q		0x57
#define	APHY_SCRAM_CTL_INIT_ST	0x58
#define	APHY_PKT_GAIN		0x59
#define	APHY_COARSE_ES		0x5a
#define	APHY_FINE_ES		0x5b
#define	APHY_TRN_OFFSET		0x5c
#define	APHY_NUM_PKT_CNT	0x5f
#define	APHY_STOP_PKT_CNT	0x60
#define	APHY_CTL		0x61
#define	APHY_PASS_TH_SAMPS	0x62
#define	APHY_RX_COMP_COEFF	0x63
#define	APHY_TC_PLCP_DELAY	0x68
#define	APHY_TX_COMP_COEFF	0x69
#define	APHY_TX_COMP_OFFSET	0x6a
#define	APHY_DC_BIAS		0x6b
#define	APHY_ROTATE_FACT	0x6e
#define	APHY_TABLE_ADDR		0x72
#define	APHY_TABLE_DATA_I	0x73
#define	APHY_TABLE_DATA_Q	0x74
#define	APHY_RSSI_FILT_B0	0x75
#define	APHY_RSSI_FILT_B1	0x76
#define	APHY_RSSI_FILT_B2	0x77
#define	APHY_RSSI_FILT_A1	0x78
#define	APHY_RSSI_FILT_A2	0x79
#define	APHY_RSSI_ADC_CTL	0x7a
#define	APHY_TSSI_STAT		0x7b
#define	APHY_TSSI_TEMP_CTL	0x7c
#define	APHY_TEMP_STAT		0x7d
#define	APHY_CRS_DELAY		0x7e
#define	APHY_WRSSI_NRSSI	0x7f
#define	APHY_P1_P2_GAIN_SETTLE	0x80
#define	APHY_N1_N2_GAIN_SETTLE	0x81
#define	APHY_N1_P1_GAIN_SETTLE	0x82
#define	APHY_P1_CLIP_CTR	0x83
#define	APHY_P2_CLIP_CTR	0x84
#define	APHY_N1_CLIP_CTR	0x85
#define	APHY_N2_CLIP_CTR	0x86
#define	APHY_P1_COMP_TIME	0x88
#define	APHY_N1_COMP_TIME	0x89
#define	APHY_N1_N2_THRESH	0x8a
#define	APHY_ANT2_DWELL		0x8b
#define	APHY_ANT_WR_SETTLE	0x8c
#define	APHY_ANT_COMP_TIME	0x8d
#define	APHY_AUX_CLIP_THRESH	0x8e
#define	APHY_DS_AUX_CLIP_THRESH	0x8f
#define	APHY_CLIP2RST_N1_P1	0x90
#define	APHY_P1_P2_EDDR_THRESH	0x91
#define	APHY_N1_N2_EDDR_THRESH	0x92
#define	APHY_CLIP_PWDN_THRESH	0x93
#define	APHY_SRCH_COMP_SETTLE	0x94
#define	APHY_ED_DROP_ENAB	0x95
#define	APHY_N1_P1_P2_COMP	0x96
#define	APHY_CCK_NUS_THRESH	0x9b
#define	APHY_CLIP_N1_P1_IDX	0xa0
#define	APHY_CLIP_P1_P2_IDX	0xa1
#define	APHY_CLIP_N1_N2_IDX	0xa2
#define	APHY_CLIP_THRESH	0xa3
#define	APHY_CCK_DESIRED_POW	0xa4
#define	APHY_CCK_GAIN_INFO	0xa5
#define	APHY_CCK_SHBITS_REF	0xa6
#define	APHY_CCK_SHBITS_GNREF	0xa7
#define	APHY_DIV_SEARCH_IDX	0xa8
#define	APHY_CLIP2_THRESH	0xa9
#define	APHY_CLIP3_THRESH	0xaa
#define	APHY_DIV_SEARCH_P1_P2	0xab
#define	APHY_CLIP_P1_P2_THRESH	0xac
#define	APHY_DIV_SEARCH_GN_BACK	0xad
#define	APHY_DIV_SEARCH_GN_CHANGE 0xae
#define	APHY_WB_PWR_THRESH	0xb0
#define	APHY_WW_CLIP0_THRESH	0xb1
#define	APHY_WW_CLIP1_THRESH	0xb2
#define	APHY_WW_CLIP2_THRESH	0xb3
#define	APHY_WW_CLIP3_THRESH	0xb4
#define	APHY_WW_CLIP0_IDX	0xb5
#define	APHY_WW_CLIP1_IDX	0xb6
#define	APHY_WW_CLIP2_IDX	0xb7
#define	APHY_WW_CLIP3_IDX	0xb8
#define	APHY_WW_CLIPWRSSI_IDX	0xb9
#define	APHY_WW_CLIPVAR_THRESH	0xba
#define	APHY_NB_WRRSI_WAIT	0xbb
#define	APHY_CRSON_THRESH	0xc0
#define	APHY_CRSOFF_THRESH	0xc1
#define	APHY_CRSMF_THRESH0	0xc2
#define	APHY_CRSMF_THRESH1	0xc3
#define	APHY_RADAR_BLANK_CTL	0xc4
#define	APHY_RADAR_FIFO_CTL	0xc5
#define	APHY_RADAR_FIFO		0xc6
#define	APHY_RADAR_THRESH0	0xc7
#define	APHY_RADAR_THRESH1	0xc8
#define	APHY_EDON_P1		0xc9
#define	APHY_FACT_RHOSQ		0xcc

#define APHY_RSSISELL1_TBL	0xdc /* RSSISelLookup1Table corerev >= 6 */

/* APHY_ANT_DWELL bits (FirstAntSecondAntDwellTime) */
#define	APHY_ANT_DWELL_FIRST_ANT	0x100

/* Bits in APHY_IqestEnWaitTime */
#define APHY_IqEnWaitTime_waitTime_SHIFT 0
#define APHY_IqEnWaitTime_waitTime_MASK (0xff << APHY_IqEnWaitTime_waitTime_SHIFT)
#define APHY_IqMode_SHIFT 8
#define APHY_IqMode (1 << APHY_IqMode_SHIFT)
#define APHY_IqStart_SHIFT 9
#define APHY_IqStart (1 << APHY_IqStart_SHIFT)

/* Bits in APHY_CTHR_STHR_SHDIN(0x29): */
#define APHY_CTHR_CRS1_ENABLE	0x4000

/* Gphy registers in the aphy :-( */

#define	GPHY_REG_OFT_BASE	0x800
/* Gphy registers */
#define	GPHY_PHY_VER		0x0800
#define	GPHY_CTRL		0x0801
#define	GPHY_CLASSIFY_CTRL	0x0802
#define	GPHY_TABLE_ADDR		0x0803
#define	GPHY_TRLUT1		0x0803		/* phyrev > 1 */
#define	GPHY_TABLE_DATA		0x0804
#define	GPHY_TRLUT2		0x0804		/* phyrev > 1 */
#define	GPHY_RSSI_B0		0x0805
#define	GPHY_RSSI_B1		0x0806
#define	GPHY_RSSI_B2		0x0807
#define	GPHY_RSSI_A0		0x0808
#define	GPHY_RSSI_A1		0x0809
#define	GPHY_TSSI_B0		0x080a
#define	GPHY_TSSI_B1		0x080b
#define	GPHY_TSSI_B2		0x080c
#define	GPHY_TSSI_A0		0x080d
#define	GPHY_TSSI_A1		0x080e
#define	GPHY_DC_OFFSET1		0x080f
#define	GPHY_DC_OFFSET2		0x0810
#define	GPHY_RF_OVERRIDE	0x0811
#define	GPHY_RF_OVERRIDE_VAL	0x0812
#define	GPHY_DBG_STATE		0x0813
#define	GPHY_ANA_OVERRIDE	0x0814
#define	GPHY_ANA_OVERRIDE_VAL	0x0815

/* Bphy regs offset in the nphy */
#define	NPHY_TO_BPHY_OFF	0xc00

/* Nphy registers, this can be auto-generated by regdb.pl:
 * regdb.pl -name mimophy
 *          -def /projects/BCM4321/a0/design/mimophy/doc/mimophy_regs.txt
 *          -chdr mimophy_regs.h
 *          -pfx NPHY
 */
#define NPHY_Version			0x00
#define NPHY_BBConfig			0x01
#define NPHY_Channel			0x05
#define NPHY_TxError			0x07
#define NPHY_BandControl		0x09
#define NPHY_FourwireAddress		0x0b
#define NPHY_FourwireDataHi		0x0c
#define NPHY_FourwireDataLo		0x0d
#define NPHY_BistStatus0		0x0e
#define NPHY_BistStatus1		0x0f
#define NPHY_Core1DesiredPower		0x18
#define NPHY_Core1cckDesiredPower	0x19
#define NPHY_Core1barelyClipBackoff	0x1a
#define NPHY_Core1cckbarelyClipBackoff	0x1b
#define NPHY_Core1computeGainInfo	0x1c
#define NPHY_Core1cckcomputeGainInfo	0x1d
#define NPHY_Core1MinMaxGain		0x1e
#define NPHY_Core1cckMinMaxGain		0x1f
#define NPHY_Core1InitGainCode		0x20
#define NPHY_Core1Clip1HiGainCode	0x21
#define NPHY_Core1Clip1MdGainCode	0x22
#define NPHY_Core1Clip1LoGainCode	0x23
#define NPHY_Core1Clip2GainCode		0x24
#define NPHY_Core1FilterGain		0x25
#define NPHY_Core1lpfQHpFBw		0x26
#define NPHY_Core1clipwbThreshold	0x27
#define NPHY_Core1w1Threshold		0x28
#define NPHY_Core1edThreshold		0x29
#define NPHY_Core1smallsigThreshold	0x2a
#define NPHY_Core1nbClipThreshold	0x2b
#define NPHY_Core1Clip1Threshold	0x2c
#define NPHY_Core1Clip2Threshold	0x2d
#define NPHY_Core2DesiredPower		0x2e
#define NPHY_Core2cckDesiredPower	0x2f
#define NPHY_Core2barelyClipBackoff	0x30
#define NPHY_Core2cckbarelyClipBackoff	0x31
#define NPHY_Core2computeGainInfo	0x32
#define NPHY_Core2cckcomputeGainInfo	0x33
#define NPHY_Core2MinMaxGain		0x34
#define NPHY_Core2cckMinMaxGain		0x35
#define NPHY_Core2InitGainCode		0x36
#define NPHY_Core2Clip1HiGainCode	0x37
#define NPHY_Core2Clip1MdGainCode	0x38
#define NPHY_Core2Clip1LoGainCode	0x39
#define NPHY_Core2Clip2GainCode		0x3a
#define NPHY_Core2FilterGain		0x3b
#define NPHY_Core2lpfQHpFBw		0x3c
#define NPHY_Core2clipwbThreshold	0x3d
#define NPHY_Core2w1Threshold		0x3e
#define NPHY_Core2edThreshold		0x3f
#define NPHY_Core2smallsigThreshold	0x40
#define NPHY_Core2nbClipThreshold	0x41
#define NPHY_Core2Clip1Threshold	0x42
#define NPHY_Core2Clip2Threshold	0x43
#define NPHY_crsThreshold1		0x44
#define NPHY_crsThreshold2		0x45
#define NPHY_crsThreshold3		0x46
#define NPHY_crsControl			0x47
#define NPHY_DcFiltAddress		0x48
#define NPHY_RxFilt20Num00		0x49
#define NPHY_RxFilt20Num01		0x4a
#define NPHY_RxFilt20Num02		0x4b
#define NPHY_RxFilt20Den00		0x4c
#define NPHY_RxFilt20Den01		0x4d
#define NPHY_RxFilt20Num10		0x4e
#define NPHY_RxFilt20Num11		0x4f
#define NPHY_RxFilt20Num12		0x50
#define NPHY_RxFilt20Den10		0x51
#define NPHY_RxFilt20Den11		0x52
#define NPHY_RxFilt40Num00		0x53
#define NPHY_RxFilt40Num01		0x54
#define NPHY_RxFilt40Num02		0x55
#define NPHY_RxFilt40Den00		0x56
#define NPHY_RxFilt40Den01		0x57
#define NPHY_RxFilt40Num10		0x58
#define NPHY_RxFilt40Num11		0x59
#define NPHY_RxFilt40Num12		0x5a
#define NPHY_RxFilt40Den10		0x5b
#define NPHY_RxFilt40Den11		0x5c
#define NPHY_pktprocResetLen		0x60
#define NPHY_initcarrierDetLen		0x61
#define NPHY_clip1carrierDetLen		0x62
#define NPHY_clip2carrierDetLen		0x63
#define NPHY_initgainSettleLen		0x64
#define NPHY_clip1gainSettleLen		0x65
#define NPHY_clip2gainSettleLen		0x66
#define NPHY_pktgainSettleLen		0x67
#define NPHY_carriersearchtimeoutLen	0x68
#define NPHY_timingsearchtimeoutLen	0x69
#define NPHY_energydroptimeoutLen	0x6a
#define NPHY_clip1nbdwellLen		0x6b
#define NPHY_clip2nbdwellLen		0x6c
#define NPHY_w1clip1dwellLen		0x6d
#define NPHY_w1clip2dwellLen		0x6e
#define NPHY_w2clip1dwellLen		0x6f
#define NPHY_payloadcrsExtensionLen	0x70
#define NPHY_energyDropcrsExtensionLen	0x71
#define NPHY_TableAddress		0x72
#define NPHY_TableDataLo		0x73
#define NPHY_TableDataHi		0x74
#define NPHY_WwiseLengthIndex		0x75
#define NPHY_NsyncLengthIndex		0x76
#define NPHY_TxMacIfHoldOff		0x77
#define NPHY_RfctrlCmd			0x78
#define NPHY_RfctrlRSSIOTHERS1		0x7a
#define NPHY_RfctrlRXGAIN1		0x7b
#define NPHY_RfctrlTXGAIN1		0x7c
#define NPHY_RfctrlRSSIOTHERS2		0x7d
#define NPHY_RfctrlRXGAIN2		0x7e
#define NPHY_RfctrlTXGAIN2		0x7f
#define NPHY_RfctrlRSSIOTHERS3		0x80
#define NPHY_RfctrlRXGAIN3		0x81
#define NPHY_RfctrlTXGAIN3		0x82
#define NPHY_RfctrlRSSIOTHERS4		0x83
#define NPHY_RfctrlRXGAIN4		0x84
#define NPHY_RfctrlTXGAIN4		0x85
#define NPHY_Core1TxIQCompCoeff		0x87
#define NPHY_Core2TxIQCompCoeff		0x88
#define NPHY_Core1TxControl		0x8b
#define NPHY_Core2TxControl		0x8c
#define NPHY_ScramSigCtrl		0x90
#define NPHY_RfctrlIntc1		0x91
#define NPHY_RfctrlIntc2		0x92
#define NPHY_RfctrlIntc3		0x93
#define NPHY_RfctrlIntc4		0x94
#define NPHY_NumDatatoneswwise		0x95
#define NPHY_NumDatatonesnsync		0x96
#define NPHY_sigfieldmodwwise		0x97
#define NPHY_legsigfieldmod11n		0x98
#define NPHY_htsigfieldmod11n		0x99
#define NPHY_Core1RxIQCompA0		0x9a
#define NPHY_Core1RxIQCompB0		0x9b
#define NPHY_Core2RxIQCompA1		0x9c
#define NPHY_Core2RxIQCompB1		0x9d
#define NPHY_RxControl			0xa0
#define NPHY_RfseqMode			0xa1
#define NPHY_RfseqCoreActv		0xa2
#define NPHY_RfseqTrigger		0xa3
#define NPHY_RfseqStatus		0xa4
#define NPHY_AfectrlOverride		0xa5
#define NPHY_AfectrlCore1		0xa6
#define NPHY_AfectrlCore2		0xa7
#define NPHY_AfectrlCore3		0xa8
#define NPHY_AfectrlCore4		0xa9
#define NPHY_AfectrlDacGain1		0xaa
#define NPHY_AfectrlDacGain2		0xab
#define NPHY_AfectrlDacGain3		0xac
#define NPHY_AfectrlDacGain4		0xad
#define NPHY_STRAddress1		0xae
#define NPHY_StrAddress2		0xaf
#define NPHY_ClassifierCtrl		0xb0
#define NPHY_IQFlip			0xb1
#define NPHY_SisoSnrThresh		0xb2
#define NPHY_SigmaNmult			0xb3
#define NPHY_TxMacDelay			0xb4
#define NPHY_TxFrameDelay		0xb5
#define NPHY_MLparams			0xb6
#define NPHY_MLcontrol			0xb7
#define NPHY_wwise20Ncycdata		0xb8
#define NPHY_wwise40Ncycdata		0xb9
#define NPHY_nsync20Ncycdata		0xba
#define NPHY_nsync40Ncycdata		0xbb
#define NPHY_initswizzlepattern		0xbc
#define NPHY_txTailCountValue		0xbd
#define NPHY_BphyControl1		0xbe
#define NPHY_BphyControl2		0xbf
#define NPHY_iqloCalCmd			0xc0
#define NPHY_iqloCalCmdNnum		0xc1
#define NPHY_iqloCalCmdGctl		0xc2
#define NPHY_sampleCmd			0xc3
#define NPHY_sampleLoopCount		0xc4
#define NPHY_sampleWaitCount		0xc5
#define NPHY_sampleDepthCount		0xc6
#define NPHY_sampleStatus		0xc7
#define NPHY_gpioLoOutEn		0xc8
#define NPHY_gpioHiOutEn		0xc9
#define NPHY_gpioSel			0xca
#define NPHY_gpioClkControl		0xcb
#define NPHY_txfilt20CoeffAStg0		0xcc
#define NPHY_txfilt20CoeffAStg1		0xcd
#define NPHY_txfilt20CoeffAStg2		0xce
#define NPHY_txfilt20CoeffB32Stg0	0xcf
#define NPHY_txfilt20CoeffB1Stg0	0xd0
#define NPHY_txfilt20CoeffB32Stg1	0xd1
#define NPHY_txfilt20CoeffB1Stg1	0xd2
#define NPHY_txfilt20CoeffB32Stg2	0xd3
#define NPHY_txfilt20CoeffB1Stg2	0xd4
#define NPHY_sigFldTolerance		0xd5
#define NPHY_TxServiceField		0xd6
#define NPHY_AfeseqRx2TxPwrUpDownDly	0xd7
#define NPHY_AfeseqTx2RxPwrUpDownDly	0xd8
#define NPHY_NsyncscramInit0		0xd9
#define NPHY_NsyncscramInit1		0xda
#define NPHY_initswizzlepatternleg	0xdb
#define NPHY_BphyControl3		0xdc
#define NPHY_BphyControl4		0xdd
#define NPHY_Core1TxBBMult		0xde
#define NPHY_Core2TxBBMult		0xdf
#define NPHY_txfilt40CoeffAStg0		0xe1
#define NPHY_txfilt40CoeffAStg1		0xe2
#define NPHY_txfilt40CoeffAStg2		0xe3
#define NPHY_txfilt40CoeffB32Stg0	0xe4
#define NPHY_txfilt40CoeffB1Stg0	0xe5
#define NPHY_txfilt40CoeffB32Stg1	0xe6
#define NPHY_txfilt40CoeffB1Stg1	0xe7
#define NPHY_txfilt40CoeffB32Stg2	0xe8
#define NPHY_txfilt40CoeffB1Stg2	0xe9
#define NPHY_BistStatus2		0xea
#define NPHY_BistStatus3		0xeb
#define NPHY_RfctrlOverride		0xec
#define NPHY_MimoConfig			0xed
#define NPHY_RadarBlankCtrl	0x0ee
#define NPHY_Antenna0_radarFifoCtrl	0x0ef
#define NPHY_Antenna1_radarFifoCtrl	0x0f0
#define NPHY_Antenna0_radarFifoData	0x0f1
#define NPHY_Antenna1_radarFifoData	0x0f2
#define NPHY_RadarThresh0	0x0f3
#define NPHY_RadarThresh1	0x0f4
#define NPHY_RadarThresh0R	0x0f5
#define NPHY_RadarThresh1R	0x0f6
#define NPHY_Crs20In40DwellLength	0x0f7
#define NPHY_RfctrlLUTtrswLower1	0xf8
#define NPHY_RfctrlLUTtrswUpper1	0xf9
#define NPHY_RfctrlLUTtrswLower2	0xfa
#define NPHY_RfctrlLUTtrswUpper2	0xfb
#define NPHY_RfctrlLUTtrswLower3	0x0fc
#define NPHY_RfctrlLUTtrswUpper3	0x0fd
#define NPHY_RfctrlLUTtrswLower4	0x0fe
#define NPHY_RfctrlLUTtrswUpper4	0x0ff
#define NPHY_RfctrlLUTLnaPa1	0x100
#define NPHY_RfctrlLUTLnaPa2	0x101
#define NPHY_RfctrlLUTLnaPa3	0x102
#define NPHY_RfctrlLUTLnaPa4	0x103
#define NPHY_nsynccrcmask0	0x104
#define NPHY_nsynccrcmask1	0x105
#define NPHY_nsynccrcmask2	0x106
#define NPHY_nsynccrcmask3	0x107
#define NPHY_nsynccrcmask4	0x108
#define NPHY_crcpolynomial	0x109
#define NPHY_NumSigCnt	0x10a
#define NPHY_sigstartbitCtrl	0x10b
#define NPHY_crcpolyorder	0x10c
#define NPHY_RFCtrlCoreSwapTbl0	0x10d
#define NPHY_RFCtrlCoreSwapTbl1	0x10e
#define NPHY_RFCtrlCoreSwapTbl2OTHERS	0x10f
#define NPHY_BphyControl5	0x111
#define NPHY_RFSeqLPFBW	0x112
#define NPHY_TSSIBiasVal1	0x114
#define NPHY_TSSIBiasVal2	0x115
#define NPHY_EstPower1	0x118
#define NPHY_EstPower2	0x119
#define NPHY_TSSIMaxTxFrmDlyTime	0x11c
#define NPHY_TSSIMaxTssiDlyTime	0x11d
#define NPHY_TSSIIdle1	0x11e
#define NPHY_TSSIIdle2	0x11f
#define NPHY_TSSIMode	0x122
#define NPHY_RxMacifMode	0x123
#define NPHY_CRSidleTimeCrsOnCountLo	0x124
#define NPHY_CRSidleTimeCrsOnCountHi	0x125
#define NPHY_CRSidleTimeMeasTimeCountLo	0x126
#define NPHY_CRSidleTimeMeasTimeCountHi	0x127
#define NPHY_sampleTailWaitCount	0x128
#define NPHY_IqestCmd			0x129
#define NPHY_IqestWaitTime		0x12a
#define NPHY_IqestSampleCount		0x12b
#define NPHY_IqestIqAccLo0		0x12c
#define NPHY_IqestIqAccHi0		0x12d
#define NPHY_IqestipwrAccLo0		0x12e
#define NPHY_IqestipwrAccHi0		0x12f
#define NPHY_IqestqpwrAccLo0		0x130
#define NPHY_IqestqpwrAccHi0		0x131
#define NPHY_IqestIqAccLo1		0x134
#define NPHY_IqestIqAccHi1		0x135
#define NPHY_IqestipwrAccLo1		0x136
#define NPHY_IqestipwrAccHi1		0x137
#define NPHY_IqestqpwrAccLo1		0x138
#define NPHY_IqestqpwrAccHi1		0x139
#define NPHY_mimophycrsTxExtension	0x13a
#define NPHY_PowerDet1	0x13b
#define NPHY_PowerDet2	0x13c
#define NPHY_RSSIMaxrssiDlyTime	0x13f
#define NPHY_PilotDataWeight0	0x141
#define NPHY_PilotDataWeight1	0x142
#define NPHY_PilotDataWeight2	0x143
#define NPHY_FMDemodConfig	0x144
#define NPHY_PhaseTrackAlpha0	0x145
#define NPHY_PhaseTrackAlpha1	0x146
#define NPHY_PhaseTrackAlpha2	0x147
#define NPHY_PhaseTrackBeta0	0x148
#define NPHY_PhaseTrackBeta1	0x149
#define NPHY_PhaseTrackBeta2	0x14a
#define NPHY_PhaseTrackChange0	0x14b
#define NPHY_PhaseTrackChange1	0x14c
#define NPHY_PhaseTrackOffset	0x14d
#define NPHY_RfctrlDebug	0x14e
#define NPHY_cckshiftbitsRefVar	0x150
#define NPHY_overideDigiGain0	0x152
#define NPHY_overideDigiGain1	0x153
#define NPHY_BistStatus4		0x156
#define NPHY_RadarMaLength	0x157
#define NPHY_RadarSearchCtrl	0x158
#define NPHY_VldDataTonesSIG	0x159
#define NPHY_VldDataTonesDATA	0x15a
#define NPHY_Core1BPhyRxIQCompA0	0x15b
#define NPHY_Core1BPhyRxIQCompB0	0x15c
#define NPHY_Core2BPhyRxIQCompA1	0x15d
#define NPHY_Core2BPhyRxIQCompB1	0x15e
#define NPHY_FreqGain0	0x160
#define NPHY_FreqGain1	0x161
#define NPHY_FreqGain2	0x162
#define NPHY_FreqGain3	0x163
#define NPHY_FreqGain4	0x164
#define NPHY_FreqGain5	0x165
#define NPHY_FreqGain6	0x166
#define NPHY_FreqGain7	0x167
#define NPHY_FreqGainBypass	0x168
#define NPHY_TRLossValue	0x169
#define NPHY_Core1Adcclip	0x16a
#define NPHY_Core2Adcclip	0x16b
#define NPHY_LtrnOffsetGain	0x16f
#define NPHY_LtrnOffset	0x170
#define NPHY_NumDatatonesWise20sig	0x171
#define NPHY_NumDatatonesWise40sig	0x172
#define NPHY_NumDatatonesnsync20sig	0x173
#define NPHY_NumDatatonesnsync40sig	0x174
#define NPHY_wwisecrcmask0	0x175
#define NPHY_wwisecrcmask1	0x176
#define NPHY_wwisecrcmask2	0x177
#define NPHY_wwisecrcmask3	0x178
#define NPHY_wwisecrcmask4	0x179
#define NPHY_ChanestCDDshift	0x17a
#define NPHY_HTAGCWaitCounters	0x17b
#define NPHY_Sqparams	0x17c
#define NPHY_mcsDup6M	0x17d
#define NPHY_NumDatatonesdup40	0x17e
#define NPHY_dup40Ncycdata	0x17f
#define NPHY_dup40GFfrmtbladdr		0x180
#define NPHY_dup40frmtbladdr		0x181
#define NPHY_legdupfrmtbladdr		0x182
#define NPHY_pktprocdebug		0x183
#define NPHY_pilotcyclecnt1	0x184
#define NPHY_pilotcyclecnt2	0x185
#define NPHY_txfilt20CoeffStg0A1	0x186
#define NPHY_txfilt20CoeffStg0A2	0x187
#define NPHY_txfilt20CoeffStg1A1	0x188
#define NPHY_txfilt20CoeffStg1A2	0x189
#define NPHY_txfilt20CoeffStg2A1	0x18a
#define NPHY_txfilt20CoeffStg2A2	0x18b
#define NPHY_txfilt20CoeffStg0B1	0x18c
#define NPHY_txfilt20CoeffStg0B2	0x18d
#define NPHY_txfilt20CoeffStg0B3	0x18e
#define NPHY_txfilt20CoeffStg1B1	0x18f
#define NPHY_txfilt20CoeffStg1B2	0x190
#define NPHY_txfilt20CoeffStg1B3	0x191
#define NPHY_txfilt20CoeffStg2B1	0x192
#define NPHY_txfilt20CoeffStg2B2	0x193
#define NPHY_txfilt20CoeffStg2B3	0x194
#define NPHY_txfilt40CoeffStg0A1	0x195
#define NPHY_txfilt40CoeffStg0A2	0x196
#define NPHY_txfilt40CoeffStg1A1	0x197
#define NPHY_txfilt40CoeffStg1A2	0x198
#define NPHY_txfilt40CoeffStg2A1	0x199
#define NPHY_txfilt40CoeffStg2A2	0x19a
#define NPHY_txfilt40CoeffStg0B1	0x19b
#define NPHY_txfilt40CoeffStg0B2	0x19c
#define NPHY_txfilt40CoeffStg0B3	0x19d
#define NPHY_txfilt40CoeffStg1B1	0x19e
#define NPHY_txfilt40CoeffStg1B2	0x19f
#define NPHY_txfilt40CoeffStg1B3	0x1a0
#define NPHY_txfilt40CoeffStg2B1	0x1a1
#define NPHY_txfilt40CoeffStg2B2	0x1a2
#define NPHY_txfilt40CoeffStg2B3	0x1a3
#define NPHY_RSSIMultCoef0IRSSIX	0x1a4
#define NPHY_RSSIMultCoef0IRSSIY	0x1a5
#define NPHY_RSSIMultCoef0IRSSIZ	0x1a6
#define NPHY_RSSIMultCoef0ITBD		0x1a7
#define NPHY_RSSIMultCoef0IPowerDet	0x1a8
#define NPHY_RSSIMultCoef0ITSSI		0x1a9
#define NPHY_RSSIMultCoef0QRSSIX	0x1aa
#define NPHY_RSSIMultCoef0QRSSIY	0x1ab
#define NPHY_RSSIMultCoef0QRSSIZ	0x1ac
#define NPHY_RSSIMultCoef0QTBD		0x1ad
#define NPHY_RSSIMultCoef0QPowerDet	0x1ae
#define NPHY_RSSIMultCoef0QTSSI		0x1af
#define NPHY_RSSIMultCoef1IRSSIX	0x1b0
#define NPHY_RSSIMultCoef1IRSSIY	0x1b1
#define NPHY_RSSIMultCoef1IRSSIZ	0x1b2
#define NPHY_RSSIMultCoef1ITBD		0x1b3
#define NPHY_RSSIMultCoef1IPowerDet	0x1b4
#define NPHY_RSSIMultCoef1ITSSI		0x1b5
#define NPHY_RSSIMultCoef1QRSSIX	0x1b6
#define NPHY_RSSIMultCoef1QRSSIY	0x1b7
#define NPHY_RSSIMultCoef1QRSSIZ	0x1b8
#define NPHY_RSSIMultCoef1QTBD		0x1b9
#define NPHY_RSSIMultCoef1QPowerDet	0x1ba
#define NPHY_RSSIMultCoef1QTSSI		0x1bb
#define NPHY_SampCollectWaitCounter	0x1bc
#define NPHY_PassThroughCounter	0x1bd
#define NPHY_LtrnOffsetGain_20L	0x1c4
#define NPHY_LtrnOffset_20L	0x1c5
#define NPHY_LtrnOffsetGain_20U	0x1c6
#define NPHY_LtrnOffset_20U	0x1c7
#define NPHY_dssscckgainSettleLen	0x1c8
#define NPHY_gpioLoOut			0x1c9
#define NPHY_gpioHiOut			0x1ca
#define NPHY_crsCheck	0x1cb
#define NPHY_Mllogssratio	0x1cc
#define NPHY_dupscale	0x1cd
#define NPHY_BW1a 			0x1ce /* sfo_chan_centerTs20__3 */
#define NPHY_BW2			0x1cf /* sfo_chan_centerTs20__2 */
#define NPHY_BW3			0x1d0 /* sfo_chan_centerTs20__1 */
#define NPHY_BW4			0x1d1 /* sfo_chan_center_factor__3 */
#define NPHY_BW5			0x1d2 /* sfo_chan_center_factor__2 */
#define NPHY_BW6			0x1d3 /* sfo_chan_center_factor__1 */
#define NPHY_CoarseLength0	0x1d4
#define NPHY_CoarseLength1	0x1d5
#define NPHY_crsThreshold1u	0x1d6
#define NPHY_crsThreshold2u	0x1d7
#define NPHY_crsThreshold3u	0x1d8
#define NPHY_crsControlu	0x1d9
#define NPHY_crsThreshold1l	0x1da
#define NPHY_crsThreshold2l	0x1db
#define NPHY_crsThreshold3l	0x1dc
#define NPHY_crsControll	0x1dd
#define NPHY_STRAddress1u	0x1de
#define NPHY_StrAddress2u	0x1df
#define NPHY_STRAddress1l	0x1e0
#define NPHY_StrAddress2l	0x1e1
#define NPHY_CrsCheck1	0x1e2
#define NPHY_CrsCheck2	0x1e3
#define NPHY_CrsCheck3	0x1e4
#define NPHY_JumpStep0	0x1e5
#define NPHY_JumpStep1	0x1e6
#define NPHY_TxPwrCtrlCmd	0x1e7
#define NPHY_TxPwrCtrlNnum	0x1e8
#define NPHY_TxPwrCtrlIdleTssi	0x1e9
#define NPHY_TxPwrCtrlTargetPwr	0x1ea
#define NPHY_TxPwrCtrlBaseIndex	0x1eb
#define NPHY_TxPwrCtrlPwrIndex	0x1ec
#define NPHY_Core1TxPwrCtrlStatus	0x1ed
#define NPHY_Core2TxPwrCtrlStatus	0x1ee
#define NPHY_smallsigGainSettleLen	0x1ef
#define NPHY_PhyStatsGainInfo0	0x1f0
#define NPHY_PhyStatsGainInfo1	0x1f1
#define NPHY_PhyStatsFreqEst	0x1f2
#define NPHY_PhyStatsAdvRetard	0x1f3
#define NPHY_PhyLoopbackMode	0x1f4
#define NPHY_ToneMapIndex201	0x1f5
#define NPHY_ToneMapIndex202	0x1f6
#define NPHY_ToneMapIndex203	0x1f7
#define NPHY_ToneMapIndex401	0x1f8
#define NPHY_ToneMapIndex402	0x1f9
#define NPHY_ToneMapIndex403	0x1fa
#define NPHY_ToneMapIndex404	0x1fb
#define NPHY_PilotToneMapIndex1	0x1fc
#define NPHY_PilotToneMapIndex2	0x1fd
#define NPHY_PilotToneMapIndex3	0x1fe
#define NPHY_TxRifsFrameDelay		0x1ff
#define NPHY_AfeseqRx2TxPwrUpDownDly40M	0x200
#define NPHY_AfeseqTx2RxPwrUpDownDly40M	0x201
#define NPHY_AfeseqRx2TxPwrUpDownDly20M	0x202
#define NPHY_AfeseqTx2RxPwrUpDownDly20M	0x203
#define NPHY_RxSigControl	0x204
#define NPHY_rxpilotcyclecnt0	0x205
#define NPHY_rxpilotcyclecnt1	0x206
#define NPHY_rxpilotcyclecnt2	0x207
#define NPHY_AfeseqRx2TxPwrUpDownDly10M	0x208
#define NPHY_AfeseqTx2RxPwrUpDownDly10M	0x209
#define NPHY_dssscckCrsExtensionLen	0x20a
#define NPHY_Mllogssratioslope	0x20b
#define NPHY_rifsSearchTimeoutLength	0x20c
#define NPHY_TxRealFrameDelay		0x20d
#define NPHY_highpowAntswitchThresh	0x20e
#define NPHY_ed_crsAssertThresh0	0x210
#define NPHY_ed_crsAssertThresh1	0x211
#define NPHY_ed_crsDeassertThresh0	0x212
#define NPHY_ed_crsDeassertThresh1	0x213
#define NPHY_StrWaitTime20U		0x214
#define NPHY_StrWaitTime20L		0x215
#define NPHY_ToneMapIndex675M		0x216
#define NPHY_HTSigTones			0x217
#define NPHY_RSSIVal1                   0x219
#define NPHY_RSSIVal2                   0x21a
#define NPHY_ChanestHang		0x21d
#define NPHY_fineRx2clockgatecontrol	0x221
#define NPHY_TxPwrCtrlInit		0x222

/* Bits in NPHY_BandControl */
#define NPHY_BandControl_currentBand	0x0001

/* Bits in NPHY_Core[12]computeGainInfo */
#define NPHY_CorecomputeGainInfo_gainBackoffValue_SHIFT 0
#define NPHY_CorecomputeGainInfo_gainBackoffValue_MASK \
		(0x1f << NPHY_CorecomputeGainInfo_gainBackoffValue_SHIFT)
#define NPHY_CorecomputeGainInfo_barelyClipGainBackoffValue_SHIFT 5
#define NPHY_CorecomputeGainInfo_barelyClipGainBackoffValue_MASK \
		(0x1f << NPHY_CorecomputeGainInfo_barelyClipGainBackoffValue_SHIFT)
#define NPHY_CorecomputeGainInfo_gainStepValue_SHIFT 10
#define NPHY_CorecomputeGainInfo_gainStepValue_MASK \
		(0x7 << NPHY_CorecomputeGainInfo_gainStepValue_SHIFT)
#define NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT 13
#define NPHY_CorecomputeGainInfo_disableClip2detect_MASK \
		(0x1 << NPHY_CorecomputeGainInfo_disableClip2detect_SHIFT)

/* Bits in NPHY_Core[12]MinMaxGain */
#define NPHY_CoreMinMaxGain_maxGainValue_SHIFT	8
#define NPHY_CoreMinMaxGain_maxGainValue_MASK	\
		(0xff << NPHY_CoreMinMaxGain_maxGainValue_SHIFT)
#define NPHY_CoreMinMaxGain_minGainValue_SHIFT	0
#define NPHY_CoreMinMaxGain_minGainValue_MASK	\
		(0xff << NPHY_CoreMinMaxGain_minGainValue_SHIFT)

/* Bits in NPHY_Core[12]cckcomputeGainInfo */
#define NPHY_CorecckcomputeGainInfo_gainBackoffValue_SHIFT 0
#define NPHY_CorecckcomputeGainInfo_gainBackoffValue_MASK \
		(0x1f << NPHY_CorecckcomputeGainInfo_gainBackoffValue_SHIFT)
#define NPHY_CorecckcomputeGainInfo_cckbarelyClipGainBackoffValue_SHIFT 5
#define NPHY_CorecckcomputeGainInfo_cckbarelyClipGainBackoffValue_MASK \
		(0xf << NPHY_CorecckcomputeGainInfo_cckbarelyClipGainBackoffValue_SHIFT)

/* Bits in NPHY_Core[12]cckMinMaxGain */
#define NPHY_CorecckMinMaxGain_cckmaxGainValue_SHIFT	8
#define NPHY_CorecckMinMaxGain_cckmaxGainValue_MASK	\
		(0xff << NPHY_CorecckMinMaxGain_cckmaxGainValue_SHIFT)
#define NPHY_CorecckMinMaxGain_cckminGainValue_SHIFT	0
#define NPHY_CorecckMinMaxGain_cckminGainValue_MASK	\
		(0xff << NPHY_CorecckMinMaxGain_cckminGainValue_SHIFT)

/* Bits in NPHY_Core[12]InitGainCode */
#define NPHY_CoreInitGainCode_initTrTxIndex_SHIFT	13
#define NPHY_CoreInitGainCode_initTrTxIndex_MASK	\
		(0x1 << NPHY_CoreInitGainCode_initTrTxIndex_SHIFT)
#define NPHY_CoreInitGainCode_initTrRxIndex_SHIFT	12
#define NPHY_CoreInitGainCode_initTrRxIndex_MASK	\
		(0x1 << NPHY_CoreInitGainCode_initTrRxIndex_SHIFT)
#define NPHY_CoreInitGainCode_initHpvga2Index_SHIFT	7
#define NPHY_CoreInitGainCode_initHpvga2Index_MASK	\
		(0x1f << NPHY_CoreInitGainCode_initHpvga2Index_SHIFT)
#define NPHY_CoreInitGainCode_initHpvga1Index_SHIFT	3
#define NPHY_CoreInitGainCode_initHpvga1Index_MASK	\
		(0xf << NPHY_CoreInitGainCode_initHpvga1Index_SHIFT)
#define NPHY_CoreInitGainCode_initLnaIndex_SHIFT	1
#define NPHY_CoreInitGainCode_initLnaIndex_MASK	\
		(0x3 << NPHY_CoreInitGainCode_initLnaIndex_SHIFT)
#define NPHY_CoreInitGainCode_initExtLnaIndex_SHIFT	0
#define NPHY_CoreInitGainCode_initExtLnaIndex_MASK	\
		(0x1 << NPHY_CoreInitGainCode_initExtLnaIndex_SHIFT)

/* Bits in NPHY_Core[12]clipwbThreshold */
#define NPHY_CoreclipwbThreshold_clip1wbThreshold_SHIFT 0
#define NPHY_CoreclipwbThreshold_clip1wbThreshold_MASK \
		(0x3f << NPHY_CoreclipwbThreshold_clip1wbThreshold_SHIFT)
#define NPHY_CoreclipwbThreshold_clip2wbThreshold_SHIFT 6
#define NPHY_CoreclipwbThreshold_clip2wbThreshold_MASK \
		(0x3f << NPHY_CoreclipwbThreshold_clip2wbThreshold_SHIFT)

/* Bits in NPHY_RfctrlCmd */
#define NPHY_RfctrlCmd_startseq_SHIFT	0
#define NPHY_RfctrlCmd_startseq_MASK	(0x1 << NPHY_RfctrlCmd_startseq_SHIFT)
#define NPHY_RfctrlCmd_RxOrTxn_SHIFT	2
#define NPHY_RfctrlCmd_RxOrTxn_MASK	(0x1 << NPHY_RfctrlCmd_RxOrTxn_SHIFT)
#define NPHY_RfctrlCmd_core_sel_SHIFT	3
#define NPHY_RfctrlCmd_core_sel_MASK	(0x7 << NPHY_RfctrlCmd_core_sel_SHIFT)
#define RFCC_POR_FORCE			0x0040
#define RFCC_OE_POR_FORCE		0x0080
#define NPHY_RfctrlCmd_rxen_SHIFT	8
#define NPHY_RfctrlCmd_rxen_MASK	(0x1 << NPHY_RfctrlCmd_rxen_SHIFT)
#define NPHY_RfctrlCmd_txen_SHIFT	9
#define NPHY_RfctrlCmd_txen_MASK	(0x1 << NPHY_RfctrlCmd_txen_SHIFT)
#define RFCC_CHIP0_PU			0x0400
#define NPHY_RfctrlCmd_seqen_core_SHIFT	12
#define NPHY_RfctrlCmd_seqen_core_MASK	(0xf << NPHY_RfctrlCmd_seqen_core_SHIFT)

/* Bits in NPHY_RfctrlRSSIOTHERS[12] */
#define NPHY_RfctrlRSSIOTHERS_rx_pd_SHIFT 0
#define NPHY_RfctrlRSSIOTHERS_rx_pd_MASK (0x1 << NPHY_RfctrlRSSIOTHERS_rx_pd_SHIFT)
#define NPHY_RfctrlRSSIOTHERS_tx_pd_SHIFT 1
#define NPHY_RfctrlRSSIOTHERS_tx_pd_MASK (0x1 << NPHY_RfctrlRSSIOTHERS_tx_pd_SHIFT)
#define NPHY_RfctrlRSSIOTHERS_pa_pd_SHIFT 2
#define NPHY_RfctrlRSSIOTHERS_pa_pd_MASK (0x1 << NPHY_RfctrlRSSIOTHERS_pa_pd_SHIFT)
#define NPHY_RfctrlRSSIOTHERS_rssi_ctrl_SHIFT 4
#define NPHY_RfctrlRSSIOTHERS_rssi_ctrl_MASK (0x3 << NPHY_RfctrlRSSIOTHERS_rssi_ctrl_SHIFT)
#define NPHY_RfctrlRSSIOTHERS_lpf_bw_SHIFT 6
#define NPHY_RfctrlRSSIOTHERS_lpf_bw_MASK (0x3 << NPHY_RfctrlRSSIOTHERS_lpf_bw_SHIFT)
#define NPHY_RfctrlRSSIOTHERS_hpf_bw_hi_SHIFT 8
#define NPHY_RfctrlRSSIOTHERS_hpf_bw_hi_MASK (0x1 << NPHY_RfctrlRSSIOTHERS_hpf_bw_hi_SHIFT)
#define NPHY_RfctrlRSSIOTHERS_hiq_dis_core_SHIFT 9
#define NPHY_RfctrlRSSIOTHERS_hiq_dis_core_MASK (0x1 << NPHY_RfctrlRSSIOTHERS_hiq_dis_core_SHIFT)

/* Bits in NPHY_ScramSigCtrl */
#define NPHY_ScramSigCtrl_initStateValue_SHIFT 0
#define NPHY_ScramSigCtrl_initStateValue_MASK (0x7f << NPHY_ScramSigCtrl_initStateValue_SHIFT)
#define NPHY_ScramSigCtrl_scramCtrlMode_SHIFT 7
#define NPHY_ScramSigCtrl_scramCtrlMode_MASK (0x1 << NPHY_ScramSigCtrl_scramCtrlMode_SHIFT)
#define NPHY_ScramSigCtrl_scramindexctlEn_SHIFT 8
#define NPHY_ScramSigCtrl_scramindexctlEn_MASK (0x1 << NPHY_ScramSigCtrl_scramindexctlEn_SHIFT)
#define NPHY_ScramSigCtrl_scramstartbit_SHIFT 9
#define NPHY_ScramSigCtrl_scramstartbit_MASK (0x7f << NPHY_ScramSigCtrl_scramstartbit_SHIFT)

/* Bits in NPHY_RxControl */
#define RIFS_ENABLE			0x80
#define BPHY_BAND_SEL_UP20		0x10
#define NPHY_MLenable			0x02

/* Bits in NPHY_RfseqMode */
#define NPHY_RfseqMode_CoreActv_override 0x0001
#define NPHY_RfseqMode_Trigger_override	0x0002

/* Bits in NPHY_RfseqCoreActv */
#define NPHY_RfseqCoreActv_EnTx_SHIFT	0
#define NPHY_RfseqCoreActv_EnTx_MASK	(0xf << NPHY_RfseqCoreActv_EnTx_SHIFT)
#define NPHY_RfseqCoreActv_EnRx_SHIFT	4
#define NPHY_RfseqCoreActv_EnRx_MASK	(0xf << NPHY_RfseqCoreActv_EnRx_SHIFT)
#define NPHY_RfseqCoreActv_DisTx_SHIFT	8
#define NPHY_RfseqCoreActv_DisTx_MASK	(0xf << NPHY_RfseqCoreActv_DisTx_SHIFT)
#define NPHY_RfseqCoreActv_DisRx_SHIFT	12
#define NPHY_RfseqCoreActv_DisRx_MASK	(0xf << NPHY_RfseqCoreActv_DisRx_SHIFT)
#define NPHY_RfseqCoreActv_TxRxChain0	(0x11)
#define NPHY_RfseqCoreActv_TxRxChain1	(0x22)


/* Bits in NPHY_RfseqTrigger */
#define NPHY_RfseqTrigger_rx2tx		0x0001
#define NPHY_RfseqTrigger_tx2rx		0x0002
#define NPHY_RfseqTrigger_updategainh	0x0004
#define NPHY_RfseqTrigger_updategainl	0x0008
#define NPHY_RfseqTrigger_updategainu	0x0010
#define NPHY_RfseqTrigger_reset2rx	0x0020

/* Bits in NPHY_RfseqStatus */
#define NPHY_RfseqStatus_rx2tx		0x0001
#define NPHY_RfseqStatus_tx2rx		0x0002
#define NPHY_RfseqStatus_updategainh	0x0004
#define NPHY_RfseqStatus_updategainl	0x0008
#define NPHY_RfseqStatus_updategainu	0x0010
#define NPHY_RfseqStatus_reset2rx	0x0020

/* Bits in NPHY_AfectrlOverride */
#define NPHY_AfectrlOverride_slowpwup_adc_SHIFT 0
#define NPHY_AfectrlOverride_slowpwup_adc_MASK (0x1 << NPHY_AfectrlOverride_slowpwup_adc_SHIFT)
#define NPHY_AfectrlOverride_coarsepwup_adc_SHIFT 1
#define NPHY_AfectrlOverride_coarsepwup_adc_MASK (0x1 << NPHY_AfectrlOverride_coarsepwup_adc_SHIFT)
#define NPHY_AfectrlOverride_finepwup_adc_SHIFT 2
#define NPHY_AfectrlOverride_finepwup_adc_MASK (0x1 << NPHY_AfectrlOverride_finepwup_adc_SHIFT)
#define NPHY_AfectrlOverride_slowpwup_dac_SHIFT 3
#define NPHY_AfectrlOverride_slowpwup_dac_MASK (0x1 << NPHY_AfectrlOverride_slowpwup_dac_SHIFT)
#define NPHY_AfectrlOverride_reset_dac_SHIFT 4
#define NPHY_AfectrlOverride_reset_dac_MASK (0x1 << NPHY_AfectrlOverride_reset_dac_SHIFT)
#define NPHY_AfectrlOverride_dac_lpf_en_SHIFT 5
#define NPHY_AfectrlOverride_dac_lpf_en_MASK (0x1 << NPHY_AfectrlOverride_dac_lpf_en_SHIFT)
#define NPHY_AfectrlOverride_slowpwup_rssi_i_SHIFT 6
#define NPHY_AfectrlOverride_slowpwup_rssi_i_MASK \
		(0x1 << NPHY_AfectrlOverride_slowpwup_rssi_i_SHIFT)
#define NPHY_AfectrlOverride_fastpwup_rssi_i_SHIFT 7
#define NPHY_AfectrlOverride_fastpwup_rssi_i_MASK \
		(0x1 << NPHY_AfectrlOverride_fastpwup_rssi_i_SHIFT)
#define NPHY_AfectrlOverride_slowpwup_rssi_q_SHIFT 8
#define NPHY_AfectrlOverride_slowpwup_rssi_q_MASK \
		(0x1 << NPHY_AfectrlOverride_slowpwup_rssi_q_SHIFT)
#define NPHY_AfectrlOverride_fastpwup_rssi_q_SHIFT 9
#define NPHY_AfectrlOverride_fastpwup_rssi_q_MASK \
		(0x1 << NPHY_AfectrlOverride_fastpwup_rssi_q_SHIFT)
#define NPHY_AfectrlOverride_pwup_bg_SHIFT 10
#define NPHY_AfectrlOverride_pwup_bg_MASK (0x1 << NPHY_AfectrlOverride_pwup_bg_SHIFT)
#define NPHY_AfectrlOverride_pwup_cmout_SHIFT 11
#define NPHY_AfectrlOverride_pwup_cmout_MASK (0x1 << NPHY_AfectrlOverride_pwup_cmout_SHIFT)
#define NPHY_AfectrlOverride_rssi_select_i_SHIFT 12
#define NPHY_AfectrlOverride_rssi_select_i_MASK (0x1 << NPHY_AfectrlOverride_rssi_select_i_SHIFT)
#define NPHY_AfectrlOverride_rssi_select_q_SHIFT 13
#define NPHY_AfectrlOverride_rssi_select_q_MASK (0x1 << NPHY_AfectrlOverride_rssi_select_q_SHIFT)
#define NPHY_AfectrlOverride_dac_gain_SHIFT 14
#define NPHY_AfectrlOverride_dac_gain_MASK (0x1 << NPHY_AfectrlOverride_dac_gain_SHIFT)

/* Bits in NPHY_AfectrlCore[12] */
#define NPHY_AfectrlCore_slowpwup_adc_SHIFT 0
#define NPHY_AfectrlCore_slowpwup_adc_MASK (0x1 << NPHY_AfectrlCore_slowpwup_adc_SHIFT)
#define NPHY_AfectrlCore_coarsepwup_adc_SHIFT 1
#define NPHY_AfectrlCore_coarsepwup_adc_MASK (0x1 << NPHY_AfectrlCore_coarsepwup_adc_SHIFT)
#define NPHY_AfectrlCore_finepwup_adc_SHIFT 2
#define NPHY_AfectrlCore_finepwup_adc_MASK (0x1 << NPHY_AfectrlCore_finepwup_adc_SHIFT)
#define NPHY_AfectrlCore_slowpwup_dac_SHIFT 3
#define NPHY_AfectrlCore_slowpwup_dac_MASK (0x1 << NPHY_AfectrlCore_slowpwup_dac_SHIFT)
#define NPHY_AfectrlCore_reset_dac_SHIFT 4
#define NPHY_AfectrlCore_reset_dac_MASK (0x1 << NPHY_AfectrlCore_reset_dac_SHIFT)
#define NPHY_AfectrlCore_dac_lpf_en_SHIFT 5
#define NPHY_AfectrlCore_dac_lpf_en_MASK (0x1 << NPHY_AfectrlCore_dac_lpf_en_SHIFT)
#define NPHY_AfectrlCore_slowpwup_rssi_i_SHIFT 6
#define NPHY_AfectrlCore_slowpwup_rssi_i_MASK (0x1 << NPHY_AfectrlCore_slowpwup_rssi_i_SHIFT)
#define NPHY_AfectrlCore_fastpwup_rssi_i_SHIFT 7
#define NPHY_AfectrlCore_fastpwup_rssi_i_MASK (0x1 << NPHY_AfectrlCore_fastpwup_rssi_i_SHIFT)
#define NPHY_AfectrlCore_slowpwup_rssi_q_SHIFT 8
#define NPHY_AfectrlCore_slowpwup_rssi_q_MASK (0x1 << NPHY_AfectrlCore_slowpwup_rssi_q_SHIFT)
#define NPHY_AfectrlCore_fastpwup_rssi_q_SHIFT 9
#define NPHY_AfectrlCore_fastpwup_rssi_q_MASK (0x1 << NPHY_AfectrlCore_fastpwup_rssi_q_SHIFT)
#define NPHY_AfectrlCore_pwup_bg_SHIFT 10
#define NPHY_AfectrlCore_pwup_bg_MASK (0x1 << NPHY_AfectrlCore_pwup_bg_SHIFT)
#define NPHY_AfectrlCore_pwup_cmout_SHIFT 11
#define NPHY_AfectrlCore_pwup_cmout_MASK (0x1 << NPHY_AfectrlCore_pwup_cmout_SHIFT)
#define NPHY_AfectrlCore_rssi_select_i_SHIFT 12
#define NPHY_AfectrlCore_rssi_select_i_MASK (0x3 << NPHY_AfectrlCore_rssi_select_i_SHIFT)
#define NPHY_AfectrlCore_rssi_select_q_SHIFT 14
#define NPHY_AfectrlCore_rssi_select_q_MASK (0x3 << NPHY_AfectrlCore_rssi_select_q_SHIFT)

/* Bits in NPHY_ClassifierCtrl */
#define NPHY_ClassifierCtrl_classifierSel_SHIFT 0
#define NPHY_ClassifierCtrl_classifierSel_MASK (0x7 << NPHY_ClassifierCtrl_classifierSel_SHIFT)
#define NPHY_ClassifierCtrl_cck_en	0x1
#define NPHY_ClassifierCtrl_ofdm_en	0x2
#define NPHY_ClassifierCtrl_waited_en	0x4

/* Bits in NPHY_IQFlip */
#define NPHY_IQFlip_ADC1		0x0001
#define NPHY_IQFlip_ADC2		0x0010

/* Bits in NPHY_BphyControl3 */
#define NPHY_BphyControl3_bphyScale_SHIFT	0
#define NPHY_BphyControl3_bphyScale_MASK	(0xff << NPHY_BphyControl3_bphyScale_SHIFT)
#define NPHY_BphyControl3_bphyFrmStartCntValue_SHIFT	8
#define NPHY_BphyControl3_bphyFrmStartCntValue_MASK	\
	(0xff << NPHY_BphyControl3_bphyFrmStartCntValue_SHIFT)

/* Bits in NPHY_RfctrlOverride */
#define NPHY_RfctrlOverride_trigger_SHIFT 0
#define NPHY_RfctrlOverride_trigger_MASK (0x1 << NPHY_RfctrlOverride_trigger_SHIFT)
#define NPHY_RfctrlOverride_core_sel_SHIFT 1
#define NPHY_RfctrlOverride_core_sel_MASK (0x1 << NPHY_RfctrlOverride_core_sel_SHIFT)
#define NPHY_RfctrlOverride_rx_pd_SHIFT 2
#define NPHY_RfctrlOverride_rx_pd_MASK (0x1 << NPHY_RfctrlOverride_rx_pd_SHIFT)
#define NPHY_RfctrlOverride_tx_pd_SHIFT 3
#define NPHY_RfctrlOverride_tx_pd_MASK (0x1 << NPHY_RfctrlOverride_tx_pd_SHIFT)
#define NPHY_RfctrlOverride_pa_pd_SHIFT 4
#define NPHY_RfctrlOverride_pa_pd_MASK (0x1 << NPHY_RfctrlOverride_pa_pd_SHIFT)
#define NPHY_RfctrlOverride_rssi_ctrl_SHIFT 5
#define NPHY_RfctrlOverride_rssi_ctrl_MASK (0x1 << NPHY_RfctrlOverride_rssi_ctrl_SHIFT)
#define NPHY_RfctrlOverride_lpf_bw_SHIFT 6
#define NPHY_RfctrlOverride_lpf_bw_MASK (0x1 << NPHY_RfctrlOverride_lpf_bw_SHIFT)
#define NPHY_RfctrlOverride_hpf_bw_hi_SHIFT 7
#define NPHY_RfctrlOverride_hpf_bw_hi_MASK (0x1 << NPHY_RfctrlOverride_hpf_bw_hi_SHIFT)
#define NPHY_RfctrlOverride_hiq_dis_core_SHIFT 8
#define NPHY_RfctrlOverride_hiq_dis_core_MASK (0x1 << NPHY_RfctrlOverride_hiq_dis_core_SHIFT)
#define NPHY_RfctrlOverride_RxOrTxn_SHIFT 9
#define NPHY_RfctrlOverride_RxOrTxn_MASK (0x1 << NPHY_RfctrlOverride_RxOrTxn_SHIFT)
#define NPHY_RfctrlOverride_rxgain_SHIFT 10
#define NPHY_RfctrlOverride_rxgain_MASK (0x1 << NPHY_RfctrlOverride_rxgain_SHIFT)
#define NPHY_RfctrlOverride_txgain_SHIFT 11
#define NPHY_RfctrlOverride_txgain_MASK (0x1 << NPHY_RfctrlOverride_txgain_SHIFT)
#define NPHY_RfctrlOverride_rxen_SHIFT 12
#define NPHY_RfctrlOverride_rxen_MASK (0x1 << NPHY_RfctrlOverride_rxen_SHIFT)
#define NPHY_RfctrlOverride_txen_SHIFT 13
#define NPHY_RfctrlOverride_txen_MASK (0x1 << NPHY_RfctrlOverride_txen_SHIFT)
#define NPHY_RfctrlOverride_seqen_core_SHIFT 14
#define NPHY_RfctrlOverride_seqen_core_MASK (0x1 << NPHY_RfctrlOverride_seqen_core_SHIFT)

/* Bits in NPHY_MimoConfig */
#define RX_GF_OR_MM			0x0004
#define RX_GF_MM_AUTO			0x0100

/* Bits in NPHY_sampleCmd */
#define NPHY_sampleCmd_STOP		0x0002

/* Bits in NPHY_BphyControl2 */
#define NPHY_BphyControl2_lutIndex_SHIFT	0
#define NPHY_BphyControl2_lutIndex_MASK	(0x1f << NPHY_BphyControl2_lutIndex_SHIFT)
#define NPHY_BphyControl2_macdelay_SHIFT	5
#define NPHY_BphyControl2_macdelay_MASK	(0x3ff << NPHY_BphyControl2_macdelay_SHIFT)

/* Bits in NPHY_iqloCalCmdGctl */
#define NPHY_iqloCalCmdGctl_IQLO_CAL_EN	0x8000

/* Bits in NPHY_TSSIBiasVal1 */
#define NPHY_TSSIBiasVal1_TSSIBias_SHIFT	8
#define NPHY_TSSIBiasVal1_TSSIBias_MASK	(0xff << NPHY_TSSIBiasVal1_TSSIBias_SHIFT)
#define NPHY_TSSIBiasVal1_TSSIVal_SHIFT	0
#define NPHY_TSSIBiasVal1_TSSIVal_MASK	(0xff << NPHY_TSSIBiasVal1_TSSIVal_SHIFT)

/* Bits in NPHY_TSSIBiasVal2 */
#define NPHY_TSSIBiasVal2_TSSIBias_SHIFT	8
#define NPHY_TSSIBiasVal2_TSSIBias_MASK	(0xff << NPHY_TSSIBiasVal2_TSSIBias_SHIFT)
#define NPHY_TSSIBiasVal2_TSSIVal_SHIFT	0
#define NPHY_TSSIBiasVal2_TSSIVal_MASK	(0xff << NPHY_TSSIBiasVal2_TSSIVal_SHIFT)

/* Bits in NPHY_EstPower1 */
#define NPHY_EstPower1_estPowerValid_SHIFT	8
#define NPHY_EstPower1_estPowerValid_MASK	(0x1 << NPHY_EstPower1_estPowerValid_SHIFT)
#define NPHY_EstPower1_estPower_SHIFT	0
#define NPHY_EstPower1_estPower_MASK	(0xff << NPHY_EstPower1_estPower_SHIFT)

/* Bits in NPHY_EstPower2 */
#define NPHY_EstPower2_estPowerValid_SHIFT	8
#define NPHY_EstPower2_estPowerValid_MASK	(0x1 << NPHY_EstPower2_estPowerValid_SHIFT)
#define NPHY_EstPower2_estPower_SHIFT	0
#define NPHY_EstPower2_estPower_MASK	(0xff << NPHY_EstPower2_estPower_SHIFT)

/* Bits in NPHY_TSSIMaxTxFrmDlyTime */
#define NPHY_TSSIMaxTxFrmDlyTime_maxtxfrmdlytime_SHIFT	0
#define NPHY_TSSIMaxTxFrmDlyTime_maxtxfrmdlytime_MASK \
	(0xff << NPHY_TSSIMaxTxFrmDlyTime_maxtxfrmdlytime_SHIFT)

/* Bits in NPHY_TSSIMaxTssiDlyTime */
#define NPHY_TSSIMaxTssiDlyTime_maxtssidlytime_SHIFT	0
#define NPHY_TSSIMaxTssiDlyTime_maxtssidlytime_MASK \
	(0xff << NPHY_TSSIMaxTssiDlyTime_maxtssidlytime_SHIFT)

/* Bits in NPHY_TSSIIdle1 */
#define NPHY_TSSIIdle1_idletssi_SHIFT	0
#define NPHY_TSSIIdle1_idletssi_MASK	(0xff << NPHY_TSSIIdle1_idletssi_SHIFT)

/* Bits in NPHY_TSSIIdle2 */
#define NPHY_TSSIIdle2_idletssi_SHIFT	0
#define NPHY_TSSIIdle2_idletssi_MASK	(0xff << NPHY_TSSIIdle2_idletssi_SHIFT)

/* Bits in NPHY_TSSIMode */
#define NPHY_TSSIMode_tssiEn_SHIFT	0
#define NPHY_TSSIMode_tssiEn_MASK	(0x1 << NPHY_TSSIMode_tssiEn_SHIFT)
#define NPHY_TSSIMode_PowerDetEn_SHIFT	1
#define NPHY_TSSIMode_PowerDetEn_MASK	(0x1 << NPHY_TSSIMode_PowerDetEn_SHIFT)

/* Bits in NPHY_IqestCmd */
#define NPHY_IqestCmd_iqstart		0x1
#define NPHY_IqestCmd_iqMode		0x2

/* Bits in NPHY_IqestWaitTime */
#define NPHY_IqestWaitTime_waitTime_SHIFT 0
#define NPHY_IqestWaitTime_waitTime_MASK (0xff << NPHY_IqestWaitTime_waitTime_SHIFT)

/* Bits in NPHY_PilotDataWeight1 */
#define NPHY_PilotDataWeight1_64qam_SHIFT 12
#define NPHY_PilotDataWeight1_64qam_MASK (0xf << NPHY_PilotDataWeight1_64qam_SHIFT)
#define NPHY_PilotDataWeight1_16qam_SHIFT 8
#define NPHY_PilotDataWeight1_16qam_MASK (0xf << NPHY_PilotDataWeight1_16qam_SHIFT)
#define NPHY_PilotDataWeight1_qpsk_SHIFT 4
#define NPHY_PilotDataWeight1_qpsk_MASK (0xf << NPHY_PilotDataWeight1_qpsk_SHIFT)
#define NPHY_PilotDataWeight1_bpsk_SHIFT 0
#define NPHY_PilotDataWeight1_bpsk_MASK (0xf << NPHY_PilotDataWeight1_bpsk_SHIFT)

/* Bits in NPHY_TxPwrCtrlCmd */
#define NPHY_TxPwrCtrlCmd_txPwrCtrl_en_SHIFT	15
#define NPHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK	(0x1 << NPHY_TxPwrCtrlCmd_txPwrCtrl_en_SHIFT)
#define NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_SHIFT	14
#define NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK	(0x1 << NPHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_SHIFT)
#define NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_SHIFT	13
#define NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK \
	(0x1 << NPHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_SHIFT)
#define NPHY_TxPwrCtrlCmd_pwrIndex_init_SHIFT	0
#define NPHY_TxPwrCtrlCmd_pwrIndex_init_MASK	(0x7f << NPHY_TxPwrCtrlCmd_pwrIndex_init_SHIFT)
#define NPHY_TxPwrCtrlCmd_pwrIndex_init		0x40

/* Bits in NPHY_TxPwrCtrlNnum */
#define NPHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT	8
#define NPHY_TxPwrCtrlNnum_Npt_intg_log2_MASK	(0x7 << NPHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)
#define NPHY_TxPwrCtrlNnum_Ntssi_delay_SHIFT	0
#define NPHY_TxPwrCtrlNnum_Ntssi_delay_MASK	(0xff << NPHY_TxPwrCtrlNnum_Ntssi_delay_SHIFT)

/* Bits in NPHY_TxPwrCtrlIdleTssi */
#define NPHY_TxPwrCtrlIdleTssi_rawTssiOffsetBinFormat_SHIFT	15
#define NPHY_TxPwrCtrlIdleTssi_rawTssiOffsetBinFormat_MASK \
	(0x1 << NPHY_TxPwrCtrlIdleTssi_rawTssiOffsetBinFormat_SHIFT)
#define NPHY_TxPwrCtrlIdleTssi_idleTssi1_SHIFT	8
#define NPHY_TxPwrCtrlIdleTssi_idleTssi1_MASK	(0x3f << NPHY_TxPwrCtrlIdleTssi_idleTssi1_SHIFT)
#define NPHY_TxPwrCtrlIdleTssi_idleTssi0_SHIFT	0
#define NPHY_TxPwrCtrlIdleTssi_idleTssi0_MASK	(0x3f << NPHY_TxPwrCtrlIdleTssi_idleTssi0_SHIFT)

/* Bits in NPHY_TxPwrCtrlTargetPwr */
#define NPHY_TxPwrCtrlTargetPwr_targetPwr1_SHIFT	8
#define NPHY_TxPwrCtrlTargetPwr_targetPwr1_MASK	(0xff << NPHY_TxPwrCtrlTargetPwr_targetPwr1_SHIFT)
#define NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT	0
#define NPHY_TxPwrCtrlTargetPwr_targetPwr0_MASK	(0xff << NPHY_TxPwrCtrlTargetPwr_targetPwr0_SHIFT)

/* Bits in NPHY_TxPwrCtrlBaseIndex */
#define NPHY_TxPwrCtrlBaseIndex_loadBaseIndex_SHIFT	15
#define NPHY_TxPwrCtrlBaseIndex_loadBaseIndex_MASK \
	(0x1 << NPHY_TxPwrCtrlBaseIndex_loadBaseIndex_SHIFT)
#define NPHY_TxPwrCtrlBaseIndex_uC_baseIndex1_SHIFT	8
#define NPHY_TxPwrCtrlBaseIndex_uC_baseIndex1_MASK \
	(0x7f << NPHY_TxPwrCtrlBaseIndex_uC_baseIndex1_SHIFT)
#define NPHY_TxPwrCtrlBaseIndex_uC_baseIndex0_SHIFT	0
#define NPHY_TxPwrCtrlBaseIndex_uC_baseIndex0_MASK \
	(0x7f << NPHY_TxPwrCtrlBaseIndex_uC_baseIndex0_SHIFT)

/* Bits in NPHY_TxPwrCtrlPwrIndex */
#define NPHY_TxPwrCtrlPwrIndex_loadPwrIndex_SHIFT	15
#define NPHY_TxPwrCtrlPwrIndex_loadPwrIndex_MASK \
	(0x1 << NPHY_TxPwrCtrlPwrIndex_loadPwrIndex_SHIFT)
#define NPHY_TxPwrCtrlPwrIndex_uC_pwrIndex1_SHIFT	8
#define NPHY_TxPwrCtrlPwrIndex_uC_pwrIndex1_MASK \
	(0x7f << NPHY_TxPwrCtrlPwrIndex_uC_pwrIndex1_SHIFT)
#define NPHY_TxPwrCtrlPwrIndex_uC_pwrIndex0_SHIFT	0
#define NPHY_TxPwrCtrlPwrIndex_uC_pwrIndex0_MASK \
	(0x7f << NPHY_TxPwrCtrlPwrIndex_uC_pwrIndex0_SHIFT)

/* Bits in NPHY_Core1TxPwrCtrlStatus */
#define NPHY_Core1TxPwrCtrlStatus_estPwrValid_SHIFT	15
#define NPHY_Core1TxPwrCtrlStatus_estPwrValid_MASK \
	(0x1 << NPHY_Core1TxPwrCtrlStatus_estPwrValid_SHIFT)
#define NPHY_Core1TxPwrCtrlStatus_baseIndex_SHIFT	8
#define NPHY_Core1TxPwrCtrlStatus_baseIndex_MASK \
	(0x7f << NPHY_Core1TxPwrCtrlStatus_baseIndex_SHIFT)
#define NPHY_Core1TxPwrCtrlStatus_estPwr_SHIFT		0
#define NPHY_Core1TxPwrCtrlStatus_estPwr_MASK \
	(0xff << NPHY_Core1TxPwrCtrlStatus_estPwr_SHIFT)

/* Bits in NPHY_Core2TxPwrCtrlStatus */
#define NPHY_Core2TxPwrCtrlStatus_estPwrValid_SHIFT	15
#define NPHY_Core2TxPwrCtrlStatus_estPwrValid_MASK \
	(0x1 << NPHY_Core2TxPwrCtrlStatus_estPwrValid_SHIFT)
#define NPHY_Core2TxPwrCtrlStatus_baseIndex_SHIFT	8
#define NPHY_Core2TxPwrCtrlStatus_baseIndex_MASK \
	(0x7f << NPHY_Core2TxPwrCtrlStatus_baseIndex_SHIFT)
#define NPHY_Core2TxPwrCtrlStatus_estPwr_SHIFT		0
#define NPHY_Core2TxPwrCtrlStatus_estPwr_MASK \
	(0xff << NPHY_Core2TxPwrCtrlStatus_estPwr_SHIFT)

/* Bits in NPHY_TxPwrCtrlInit */
#define NPHY_TxPwrCtrlInit_pwrIndex_init1_SHIFT		0
#define NPHY_TxPwrCtrlInit_pwrIndex_init1_MASK \
	(0xff << NPHY_TxPwrCtrlInit_pwrIndex_init1_SHIFT)

/* Bits in NPHY_overideDigiGain1 */
#define NPHY_overideDigiGain1_cckdigigainEnCntValue_SHIFT 8
#define NPHY_overideDigiGain1_cckdigigainEnCntValue_MASK \
	(0xff << NPHY_overideDigiGain1_cckdigigainEnCntValue_SHIFT)
#define NPHY_overideDigiGain1_forcedigigainEnable_SHIFT 3
#define NPHY_overideDigiGain1_forcedigigainEnable_MASK \
	(0x1 << NPHY_overideDigiGain1_forcedigigainEnable_SHIFT)
#define NPHY_overideDigiGain1_forcedigiGainValue_SHIFT 0
#define NPHY_overideDigiGain1_forcedigiGainValue_MASK \
	(0x7 << NPHY_overideDigiGain1_forcedigiGainValue_SHIFT)

/* NPHY RFSeq Commands */
#define NPHY_RFSEQ_RX2TX		0x0
#define NPHY_RFSEQ_TX2RX		0x1
#define NPHY_RFSEQ_RESET2RX		0x2
#define NPHY_RFSEQ_UPDATEGAINH		0x3
#define NPHY_RFSEQ_UPDATEGAINL		0x4
#define NPHY_RFSEQ_UPDATEGAINU		0x5

/* NPHY RFSeq Events */
#define NPHY_RFSEQ_CMD_NOP		0x0
#define NPHY_RFSEQ_CMD_RXG_FBW		0x1
#define NPHY_RFSEQ_CMD_TR_SWITCH	0x2
#define NPHY_RFSEQ_CMD_EXT_PA		0x3
#define NPHY_RFSEQ_CMD_RXPD_TXPD	0x4
#define NPHY_RFSEQ_CMD_TX_GAIN		0x5
#define NPHY_RFSEQ_CMD_RX_GAIN		0x6
#define NPHY_RFSEQ_CMD_SET_HPF_BW	0x7
#define NPHY_RFSEQ_CMD_CLR_HIQ_DIS	0x8
#define NPHY_RFSEQ_CMD_END		0xf

/* NPHY RSSI_SEL Codes */
#define NPHY_RSSI_SEL_W1 		0x0
#define NPHY_RSSI_SEL_W2 		0x1
#define NPHY_RSSI_SEL_NB 		0x2
#define NPHY_RSSI_SEL_IQ 		0x3
#define NPHY_RSSI_SEL_TSSI_2G 		0x4
#define NPHY_RSSI_SEL_TSSI_5G 		0x5
#define NPHY_RSSI_SEL_TBD 		0x6

/* NPHY Rail Codes */
#define NPHY_RAIL_I			0x0
#define NPHY_RAIL_Q			0x1

/* NPHY fineRx2clockgatecontrol */
#define NPHY_FORCESIG_DECODEGATEDCLKS	0x8

/*
 * MBSS shared memory address definitions; see MultiBSSUcode Twiki page
 *    Local terminology:
 *        addr is byte offset used by SW.
 *        offset is word offset used by uCode.
 */

#define SHM_MBSS_BASE_ADDR  (0x320 * 2)
#define SHM_MBSS_WORD_OFFSET_TO_ADDR(n)  (SHM_MBSS_BASE_ADDR + ((n) * 2))

#define SHM_MBSS_BSSID0     SHM_MBSS_WORD_OFFSET_TO_ADDR(0)
#define SHM_MBSS_BSSID1     SHM_MBSS_WORD_OFFSET_TO_ADDR(1)
#define SHM_MBSS_BSSID2     SHM_MBSS_WORD_OFFSET_TO_ADDR(2)
#define SHM_MBSS_BCN_COUNT  SHM_MBSS_WORD_OFFSET_TO_ADDR(3)
#define SHM_MBSS_PRQ_BASE   SHM_MBSS_WORD_OFFSET_TO_ADDR(4)
#define SHM_MBSS_BC_FID0    SHM_MBSS_WORD_OFFSET_TO_ADDR(5)
#define SHM_MBSS_BC_FID1    SHM_MBSS_WORD_OFFSET_TO_ADDR(6)
#define SHM_MBSS_BC_FID2    SHM_MBSS_WORD_OFFSET_TO_ADDR(7)
#define SHM_MBSS_BC_FID3    SHM_MBSS_WORD_OFFSET_TO_ADDR(8)
#define SHM_MBSS_PRE_TBTT   SHM_MBSS_WORD_OFFSET_TO_ADDR(9)

/* SSID lengths are encoded, two at a time in 16-bits */
#define SHM_MBSS_SSID_LEN0  SHM_MBSS_WORD_OFFSET_TO_ADDR(10)
#define SHM_MBSS_SSID_LEN1  SHM_MBSS_WORD_OFFSET_TO_ADDR(11)

/* Uses uCode (HW) BSS config IDX */
#define SHM_MBSS_SSID_ADDR(idx)  \
	(((idx) == 0) ? M_SSID : SHM_MBSS_WORD_OFFSET_TO_ADDR(0x10 * (idx)))

/* Uses uCode (HW) BSS config IDX */
#define SHM_MBSS_BC_FID_ADDR(ucidx) SHM_MBSS_WORD_OFFSET_TO_ADDR(5 + (ucidx))

/*
 * Definitions for PRQ fifo data as per MultiBSSUcode Twiki page
 */

#define SHM_MBSS_PRQ_ENTRY_BYTES 10  /* Size of each PRQ entry */
#define SHM_MBSS_PRQ_ENTRY_COUNT 12  /* Number of PRQ entries */
#define SHM_MBSS_PRQ_TOT_BYTES   (SHM_MBSS_PRQ_ENTRY_BYTES * SHM_MBSS_PRQ_ENTRY_COUNT)

#define SHM_MBSS_PRQ_READ_PTR (0x5E * 2)
#define SHM_MBSS_PRQ_WRITE_PTR (0x5F * 2)

typedef struct shm_mbss_prq_entry_s shm_mbss_prq_entry_t;
struct shm_mbss_prq_entry_s {
	struct ether_addr ta;
	uint8 prq_info[2];
	uint16 time_stamp;
} PACKED;

typedef enum shm_mbss_prq_ft_e {
	SHM_MBSS_PRQ_FT_CCK,
	SHM_MBSS_PRQ_FT_OFDM,
	SHM_MBSS_PRQ_FT_MIMO,
	SHM_MBSS_PRQ_FT_RESERVED
} shm_mbss_prq_ft_t;

#define SHM_MBSS_PRQ_FT_COUNT SHM_MBSS_PRQ_FT_RESERVED
#define SHM_MBSS_PRQ_ENT_FRAMETYPE(entry)      ((entry)->prq_info[0] & 0x3)
#define SHM_MBSS_PRQ_ENT_UPBAND(entry)         ((((entry)->prq_info[0] >> 2) & 0x1) != 0)

/* What was the index matched? */
#define SHM_MBSS_PRQ_ENT_UC_BSS_IDX(entry)     (((entry)->prq_info[0] >> 4) & 0x3)
#define SHM_MBSS_PRQ_ENT_PLCP0(entry)          ((entry)->prq_info[1])

/* Was this directed to a specific SSID or BSSID? If bit clear, quantity known */
#define SHM_MBSS_PRQ_ENT_DIR_SSID(entry) \
	((((entry)->prq_info[0] >> 6) == 0) || ((entry)->prq_info[0] >> 6) == 1)
#define SHM_MBSS_PRQ_ENT_DIR_BSSID(entry) \
	((((entry)->prq_info[0] >> 6) == 0) || ((entry)->prq_info[0] >> 6) == 2)

#undef PACKED
#if !defined(__GNUC__)
#pragma pack()
#endif

#define SHM_BYT_CNT	0x2 /* IHR location */
#define MAX_BYT_CNT	0x600 /* Maximum frame len */

#define M_HOST_WOWLBM	(0x61 * 2) /* Events to be set by driver */
#define M_WOWL_NOBCN	(0x62 * 2) /* loss of bcn value */
#define M_WAKEEVENT_IND	(0x102 * 2) /* Event indication by ucode */

/* Event definitions */
#define WOWL_MAGIC	(1 << 0)	/* Wakeup on Magic packet */
#define WOWL_NET	(1 << 1)	/* Wakeup on Netpattern */
#define WOWL_DIS	(1 << 2)	/* Wakeup on loss-of-link due to Disassoc/Deauth */
#define WOWL_RETR	(1 << 3)	/* Wakeup on retrograde TSF */
#define WOWL_BCN	(1 << 4)	/* Wakeup on loss of beacon */

#define MAXBCNLOSS (1 << 13) - 1	/* max 12-bit value for bcn loss */

/* Shared memory for magic pattern */
#define M_RXFRM_SRA0 	(0x19c * 2) 	/* word 0 of the station's shifted MAC address */
#define M_RXFRM_SRA1 	(0x19d * 2) 	/* word 1 of the station's shifted MAC address */
#define M_RXFRM_SRA2 	(0x19e * 2) 	/* word 2 of the station's shifted MAC address */
#define M_RXFRM_RA0 	(0x19f * 2) 	/* word 0 of the station's MAC address */
#define M_RXFRM_RA1 	(0x1a0 * 2) 	/* word 1 of the station's MAC address */
#define M_RXFRM_RA2 	(0x1a1 * 2) 	/* word 2 of the station's MAC address */

/* Shared memory for net-pattern */
#define M_NETPAT_NUM	(0x109 * 2)	/* #of netpatterns */
#define M_NETPAT_BLK0	(0x1a2 * 2)	/* pattern 1 */
/* UCODE shm view:
typedef struct {
         uint16 offset; // byte offset
         uint16 patternsize; // the length of value[.] in bytes
         uchar bitmask[MAXPATTERNSIZE/8]; // 16 bytes, the effect length is (patternsize+7)/8
         uchar value[MAXPATTERNSIZE]; // 128 bytes, the effect length is patternsize.
     } netpattern_t;
*/
#define NETPATTERNSIZE	(148) /* 128 value + 16 mask + 4 offset + 4 patternsize */
#define MAXPATTERNSIZE 128
#define MAXPATTERNS	4

/* Power-save related */
#define M_AID_NBIT 	(0x103 * 2) 	/* The station's AID bit position in AP's TIM bitmap */
#define M_PSP_PCTLWD 	(0x2a * 2)	/* PHYCTL word for the PS-Poll frame */
#define M_PSP_PCT1LWD 	(0x58 * 2)	/* PHYCTL_1 word for the PS-Poll frame */
#endif	/* _D11_H */
