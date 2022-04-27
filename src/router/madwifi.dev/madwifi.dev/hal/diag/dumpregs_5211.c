/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/dumpregs_5211.c#2 $
 */
#include "diag.h"

#include "ah.h"
#include "ah_devid.h"
#include "ah_internal.h"

#include "dumpregs.h"

#include "ar5211/ar5211reg.h"

#include <stdlib.h>
#include <string.h>

static const HAL_REGRANGE ar5211PublicRegs[] = {
	/* NB: keep these sorted by address */
	{ AR_CR, AR_MACMISC },
	{ AR_Q0_TXDP, AR_Q9_TXDP },
	{ AR_Q_TXE },
	{ AR_Q_TXD },
	{ AR_Q0_CBRCFG, AR_Q9_CBRCFG },
	{ AR_Q0_RDYTIMECFG, AR_Q9_RDYTIMECFG },
	{ AR_Q_ONESHOTARM_SC },
	{ AR_Q_ONESHOTARM_CC },
	{ AR_Q0_MISC, AR_Q9_MISC },
	{ AR_Q0_STS, AR_Q9_STS },
	{ AR_Q_RDYTIMESHDN },
	{ AR_D0_QCUMASK, AR_D9_QCUMASK },
	{ AR_D0_LCL_IFS, AR_D9_LCL_IFS },
	{ AR_D0_RETRY_LIMIT, AR_D9_RETRY_LIMIT },
	{ AR_D0_CHNTIME, AR_D9_CHNTIME },
	{ AR_D0_MISC, AR_D9_MISC },
	{ AR_D0_SEQNUM, AR_D9_SEQNUM },
	{ AR_D_GBL_IFS_SIFS },
	{ AR_D_GBL_IFS_SLOT },
	{ AR_D_GBL_IFS_EIFS },
	{ AR_D_GBL_IFS_MISC },
	{ AR_D_FPCTL },
	{ AR_D_TXPSE },
#ifdef notdef
	{ AR_D_TXBLK_DATA(0),  /* XXX?? */ },
#endif
	{ AR_RC, AR_SREV },
	{ AR_STA_ID0, AR_DEF_ANTENNA },
	{ AR_LAST_TSTP, AR_BEACON_CNT },
};
static const HAL_REGRANGE ar5211InterruptRegs[] = {
	{ AR_ISR_RAC },		/* NB: read before secondary regs */
	{ AR_ISR_S0_S, AR_ISR_S4_S },
	{ AR_IMR, AR_IMR_S4 },
};
static const HAL_REGRANGE ar5211KeyCacheRegs[] = {
	/* NB: keep these sorted by address */
	{ AR_KEYTABLE_KEY0(0), AR_KEYTABLE_MAC1(127) },
};
/*
 * NB: don't dump the baseband state for production releases.
 */
static const HAL_REGRANGE ar5211BasebandRegs[] = {
	{ 0x9800, 0x987c },
	{ 0x9900, 0x995c },
	{ 0x9c00, 0x9c1c },
};

void
ar5211SetupRegs(struct ath_diag *atd, const HAL_REVS *revs, int what)
{
	size_t space = 0;
	u_int8_t *cp;

	if (what & DUMP_PUBLIC)
		space += sizeof(ar5211PublicRegs);
	if (what & DUMP_KEYCACHE)
		space += sizeof(ar5211KeyCacheRegs);
	if (what & DUMP_BASEBAND)
		space += sizeof(ar5211BasebandRegs);
	if (what & DUMP_INTERRUPT)
		space += sizeof(ar5211InterruptRegs);
	atd->ad_in_data = (caddr_t) malloc(space);
	if (atd->ad_in_data == NULL) {
		fprintf(stderr, "Cannot malloc memory for registers!\n");
		exit(-1);
	}
	atd->ad_in_size = space;
	cp = (u_int8_t *) atd->ad_in_data;
	if (what & DUMP_PUBLIC) {
		memcpy(cp, ar5211PublicRegs, sizeof(ar5211PublicRegs));
		cp += sizeof(ar5211PublicRegs);
	}
	if (what & DUMP_INTERRUPT) {
		memcpy(cp, ar5211InterruptRegs, sizeof(ar5211InterruptRegs));
		cp += sizeof(ar5211InterruptRegs);
	}
	if (what & DUMP_KEYCACHE) {
		memcpy(cp, ar5211KeyCacheRegs, sizeof(ar5211KeyCacheRegs));
		cp += sizeof(ar5211KeyCacheRegs);
	}
	if (what & DUMP_BASEBAND) {
		memcpy(cp, ar5211BasebandRegs, sizeof(ar5211BasebandRegs));
		cp += sizeof(ar5211BasebandRegs);
	}
}

void
ar5211DumpRegs(FILE *fd, const HAL_REVS *revs, int what)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	static const HAL_REG regs[] = {
		/* NB: keep these sorted by address */
		{ "CR",		AR_CR },
		{ "RXDP",	AR_RXDP },
		{ "CFG",	AR_CFG },
		{ "IER",	AR_IER },
		{ "RTSD0",	AR_RTSD0 },
		{ "RTSD1",	AR_RTSD1 },
		{ "TXCFG",	AR_TXCFG },
		{ "RXCFG",	AR_RXCFG },
		{ "JLAST",	AR5211_JUMBO_LAST },
		{ "MIBC",	AR_MIBC },
		{ "TOPS",	AR_TOPS },
		{ "RXNPTO",	AR_RXNPTO },
		{ "TXNPTO",	AR_TXNPTO },
		{ "RFGTO",	AR_RFGTO },
		{ "RFCNT",	AR_RFCNT },
		{ "MISC",	AR_MACMISC },
		{ "D_SIFS",	AR_D_GBL_IFS_SIFS },
		{ "D_SLOT",	AR_D_GBL_IFS_SLOT },
		{ "D_EIFS",	AR_D_GBL_IFS_EIFS },
		{ "D_MISC",	AR_D_GBL_IFS_MISC },
		{ "D_TXPSE",	AR_D_TXPSE },
		{ "RC",		AR_RC },
		{ "SCR",	AR_SCR },
		{ "INTPEND",	AR_INTPEND },
		{ "SFR",	AR_SFR },
		{ "PCICFG",	AR_PCICFG },
		{ "GPIOCR",	AR_GPIOCR },
		{ "SREV",	AR_SREV },
		{ "STA_ID0",	AR_STA_ID0 },
		{ "STA_ID1",	AR_STA_ID1 },
		{ "BSS_ID0",	AR_BSS_ID0 },
		{ "BSS_ID1",	AR_BSS_ID1 },
		{ "TIME_OUT",	AR_TIME_OUT },
		{ "RSSI_THR",	AR_RSSI_THR },
		{ "USEC",	AR_USEC },
		{ "BEACON",	AR_BEACON },
		{ "CFP_PER",	AR_CFP_PERIOD },
		{ "TIMER0",	AR_TIMER0 },
		{ "TIMER1",	AR_TIMER1 },
		{ "TIMER2",	AR_TIMER2 },
		{ "TIMER3",	AR_TIMER3 },
		{ "CFP_DUR",	AR_CFP_DUR },
		{ "RX_FILTR",	AR_RX_FILTER },
		{ "MCAST_0",	AR_MCAST_FIL0 },
		{ "MCAST_1",	AR_MCAST_FIL1 },
		{ "DIAG_SW",	AR_DIAG_SW },
		{ "TSF_L32",	AR_TSF_L32 },
		{ "TSF_U32",	AR_TSF_U32 },
		{ "TST_ADAC",	AR_TST_ADDAC },
		{ "DEF_ANT",	AR_DEF_ANTENNA },
		{ "LAST_TST",	AR_LAST_TSTP },
		{ "NAV",  	AR_NAV },
		{ "RTS_OK",  	AR_RTS_OK },
		{ "RTS_FAIL",  	AR_RTS_FAIL },
		{ "ACK_FAIL",  	AR_ACK_FAIL },
		{ "FCS_FAIL",  	AR_FCS_FAIL },
		{ "BEAC_CNT",	AR_BEACON_CNT },
	};
	int i;
	u_int32_t v;

	if (what & DUMP_BASIC) {
		ath_hal_dumpregs(fd, regs, N(regs));
	}
	if (what & DUMP_INTERRUPT) {
		/* Interrupt registers */
		if (what & DUMP_BASIC)
			fprintf(fd, "\n");
		fprintf(fd, "IMR: %08x S0 %08x S1 %08x S2 %08x S3 %08x S4 %08x\n"
			, OS_REG_READ(ah, AR_IMR)
			, OS_REG_READ(ah, AR_IMR_S0)
			, OS_REG_READ(ah, AR_IMR_S1)
			, OS_REG_READ(ah, AR_IMR_S2)
			, OS_REG_READ(ah, AR_IMR_S3)
			, OS_REG_READ(ah, AR_IMR_S4)
		);
		v = OS_REG_READ(ah, AR_ISR_RAC);
		fprintf(fd, "ISR: %08x S0 %08x S1 %08x S2 %08x S3 %08x S4 %08x\n"
			, v
			, OS_REG_READ(ah, AR_ISR_S0_S)
			, OS_REG_READ(ah, AR_ISR_S1_S)
			, OS_REG_READ(ah, AR_ISR_S2_S)
			, OS_REG_READ(ah, AR_ISR_S3_S)
			, OS_REG_READ(ah, AR_ISR_S4_S)
		);
	}
	if (what & DUMP_QCU) {
		/* QCU registers */
		if (what & (DUMP_BASIC|DUMP_INTERRUPT))
			fprintf(fd, "\n");
		fprintf(fd, "%-8s %08x  %-8s %08x  %-8s %08x\n"
			, "Q_TXE", OS_REG_READ(ah, AR_Q_TXE)
			, "Q_TXD", OS_REG_READ(ah, AR_Q_TXD)
			, "Q_RDYTIMSHD", OS_REG_READ(ah, AR_Q_RDYTIMESHDN)
		);
		fprintf(fd, "Q_ONESHOTARM_SC %08x  Q_ONESHOTARM_CC %08x\n"
			, OS_REG_READ(ah, AR_Q_ONESHOTARM_SC)
			, OS_REG_READ(ah, AR_Q_ONESHOTARM_CC)
		);
		for (i = 0; i < 10; i++)
			fprintf(fd, "Q[%u] TXDP %08x CBR %08x RDYT %08x MISC %08x STS %08x\n"
				, i
				, OS_REG_READ(ah, AR_QTXDP(i))
				, OS_REG_READ(ah, AR_QCBRCFG(i))
				, OS_REG_READ(ah, AR_QRDYTIMECFG(i))
				, OS_REG_READ(ah, AR_QMISC(i))
				, OS_REG_READ(ah, AR_QSTS(i))
			);
	}
	if (what & DUMP_DCU) {
		/* DCU registers */
		if (what & (DUMP_BASIC|DUMP_INTERRUPT|DUMP_QCU))
			fprintf(fd, "\n");
		for (i = 0; i < 10; i++)
			fprintf(fd, "D[%u] MASK %08x IFS %08x RTRY %08x CHNT %08x MISC %06x SEQ %03x\n"
				, i
				, OS_REG_READ(ah, AR_DQCUMASK(i))
				, OS_REG_READ(ah, AR_DLCL_IFS(i))
				, OS_REG_READ(ah, AR_DRETRY_LIMIT(i))
				, OS_REG_READ(ah, AR_DCHNTIME(i))
				, OS_REG_READ(ah, AR_DMISC(i))
				, OS_REG_READ(ah, AR_DSEQNUM(i))
			);
	}
	for (i = 0; i < 10; i++) {
		u_int32_t f0 = OS_REG_READ(ah, AR_D_TXBLK_DATA((i<<8)|0x00));
		u_int32_t f1 = OS_REG_READ(ah, AR_D_TXBLK_DATA((i<<8)|0x40));
		u_int32_t f2 = OS_REG_READ(ah, AR_D_TXBLK_DATA((i<<8)|0x80));
		u_int32_t f3 = OS_REG_READ(ah, AR_D_TXBLK_DATA((i<<8)|0xc0));
		if (f0 || f1 || f2 || f3)
			fprintf(fd,
				"D[%u] XMIT MASK %08x %08x %08x %08x\n",
				i, f0, f1, f2, f3);
	}
	if (what & DUMP_KEYCACHE)
		ath_hal_dumpkeycache(fd, 128, 0);
	if (what & DUMP_BASEBAND) {
		if (what &~ DUMP_BASEBAND)
			fprintf(fd, "\n");
		ath_hal_dumprange(fd, 0x9800, 0x9878);
		ath_hal_dumprange(fd, 0x9900, 0x995c);
		ath_hal_dumprange(fd, 0x9c00, 0x9c1c);
	}
#undef N
}
