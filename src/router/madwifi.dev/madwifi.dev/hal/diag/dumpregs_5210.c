/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/dumpregs_5210.c#2 $
 */
#include "diag.h"

#include "ah.h"
#include "ah_devid.h"
#include "ah_internal.h"

#include "dumpregs.h"

#include "ar5210/ar5210reg.h"

#include <stdlib.h>
#include <string.h>

static const HAL_REGRANGE ar5210PublicRegs[] = {
	/* NB: keep these sorted by address */
	{ AR_TXDP0, AR_CFG },
	{ AR_BCR, AR_MISC },
	{ AR_RC, AR_SREV },
	{ AR_STA_ID0, AR_BEACON_CNT },
};
static const HAL_REGRANGE ar5210InterruptRegs[] = {
#if 0
	{ AR_ISR },		/* clears pending interrupts */
#endif
	{ AR_IMR, AR_IER },
};
static const HAL_REGRANGE ar5210KeyCacheRegs[] = {
	/* NB: keep these sorted by address */
	{ AR_KEYTABLE_KEY0(0), AR_KEYTABLE_MAC1(63) },
};
static const HAL_REGRANGE ar5210BasebandRegs[] = {
	{ 0x9800, 0x987c },
};

void
ar5210SetupRegs(struct ath_diag *atd, const HAL_REVS *revs, int what)
{
	size_t space = 0;
	u_int8_t *cp;

	if (what & DUMP_PUBLIC)
		space += sizeof(ar5210PublicRegs);
	if (what & DUMP_KEYCACHE)
		space += sizeof(ar5210KeyCacheRegs);
	if (what & DUMP_BASEBAND)
		space += sizeof(ar5210BasebandRegs);
	if (what & DUMP_INTERRUPT)
		space += sizeof(ar5210InterruptRegs);
	atd->ad_in_data = (caddr_t) malloc(space);
	if (atd->ad_in_data == NULL) {
		fprintf(stderr, "Cannot malloc memory for registers!\n");
		exit(-1);
	}
	atd->ad_in_size = space;
	cp = (u_int8_t *) atd->ad_in_data;
	if (what & DUMP_PUBLIC) {
		memcpy(cp, ar5210PublicRegs, sizeof(ar5210PublicRegs));
		cp += sizeof(ar5210PublicRegs);
	}
	if (what & DUMP_INTERRUPT) {
		memcpy(cp, ar5210InterruptRegs, sizeof(ar5210InterruptRegs));
		cp += sizeof(ar5210InterruptRegs);
	}
	if (what & DUMP_KEYCACHE) {
		memcpy(cp, ar5210KeyCacheRegs, sizeof(ar5210KeyCacheRegs));
		cp += sizeof(ar5210KeyCacheRegs);
	}
	if (what & DUMP_BASEBAND) {
		memcpy(cp, ar5210BasebandRegs, sizeof(ar5210BasebandRegs));
		cp += sizeof(ar5210BasebandRegs);
	}
}

void
ar5210DumpRegs(FILE *fd, const HAL_REVS *revs, int what)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	static const HAL_REG regs[] = {
		/* NB: keep these sorted by address */
		{ "TXDP0",	AR_TXDP0 },
		{ "TXDP1",	AR_TXDP1 },
		{ "CR",		AR_CR },
		{ "RXDP",	AR_RXDP },
		{ "CFG",	AR_CFG },
		{ "ISR",	AR_ISR },
		{ "IMR",	AR_IMR },
		{ "IER",	AR_IER },
		{ "BCR",	AR_BCR },
		{ "TXCFG",	AR_TXCFG },
		{ "RXCFG",	AR_RXCFG },
		{ "MIBC",	AR_MIBC },
		{ "TOPS",	AR_TOPS },
		{ "RXNOFR",	AR_RXNOFRM },
		{ "TXNOFR",	AR_TXNOFRM },
		{ "RPGTO",	AR_RPGTO },
		{ "RFCNT",	AR_RFCNT },
		{ "MISC",	AR_MISC },
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
		{ "SLOT_TIM",	AR_SLOT_TIME },
		{ "TIME_OUT",	AR_TIME_OUT },
		{ "RSSI_THR",	AR_RSSI_THR },
		{ "RETRY_LM",	AR_RETRY_LMT },
		{ "USEC",	AR_USEC },
		{ "BEACON",	AR_BEACON },
		{ "CFP_PER",	AR_CFP_PERIOD },
		{ "TIMER0",	AR_TIMER0 },
		{ "TIMER1",	AR_TIMER1 },
		{ "TIMER2",	AR_TIMER2 },
		{ "TIMER3",	AR_TIMER3 },
		{ "IFS0",	AR_IFS0 },
		{ "IFS1",	AR_IFS1 },
		{ "CFP_DUR",	AR_CFP_DUR },
		{ "RX_FILTR",	AR_RX_FILTER },
		{ "MCAST_0",	AR_MCAST_FIL0 },
		{ "MCAST_1",	AR_MCAST_FIL1 },
		{ "TXMASK0",	AR_TX_MASK0 },
		{ "TXMASK1",	AR_TX_MASK1 },
		{ "TRIGLEV",	AR_TRIG_LEV },
		{ "DIAG_SW",	AR_DIAG_SW },
		{ "TSF_L32",	AR_TSF_L32 },
		{ "TSF_U32",	AR_TSF_U32 },
		{ "LAST_TST",	AR_LAST_TSTP },
		{ "RETRYCNT",	AR_RETRY_CNT },
		{ "BACKOFF",	AR_BACKOFF },
		{ "NAV",  	AR_NAV },
		{ "RTS_OK",  	AR_RTS_OK },
		{ "RTS_FAIL",  	AR_RTS_FAIL },
		{ "ACK_FAIL",  	AR_ACK_FAIL },
		{ "FCS_FAIL",  	AR_FCS_FAIL },
		{ "BEAC_CNT",	AR_BEACON_CNT },
	};

	if (what & DUMP_BASIC) {
		ath_hal_dumpregs(fd, regs, N(regs));
	}
	if (what & DUMP_KEYCACHE)
		ath_hal_dumpkeycache(fd, 64, 0);
	if (what & DUMP_BASEBAND) {
		if (what &~ DUMP_BASEBAND)
			fprintf(fd, "\n");
		ath_hal_dumprange(fd, 0x9800, 0x9878);
	}
#undef N
}
