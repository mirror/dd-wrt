/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ardecode/ar5210.c#1 $
 */
#include <stdio.h>

#include <sys/types.h>

#include "ah_decode.h"

#include "ar5210/ar5210reg.h"
#include "ar5210/ar5210phy.h"

static struct {
	u_int32_t	off;
	const char*	name;
	const char*	bits;
} regnames[] = {
	{ AR_TXDP0,		"AR_TXDP0" },
	{ AR_TXDP1,		"AR_TXDP1" },
	{ AR_CR,		"AR_CR",	AR_CR_BITS },
	{ AR_RXDP,		"AR_RXDP" },
	{ AR_CFG,		"AR_CFG",	AR_CFG_BITS },
	{ AR_ISR,		"AR_ISR",	AR_ISR_BITS },
	{ AR_IMR,		"AR_IMR",	AR_IMR_BITS },
	{ AR_IER,		"AR_IER",	AR_IER_BITS },
	{ AR_BCR,		"AR_BCR",	AR_BCR_BITS },
	{ AR_BSR,		"AR_BSR",	AR_BSR_BITS },
	{ AR_TXCFG,		"AR_TXCFG",	AR_TXCFG_BITS },
	{ AR_RXCFG,		"AR_RXCFG" },
	{ AR_MIBC,		"AR_MIBC" },
	{ AR_TOPS,		"AR_TOPS" },
	{ AR_RXNOFRM,		"AR_RXNOFRM" },
	{ AR_TXNOFRM,		"AR_TXNOFRM" },
	{ AR_RPGTO,		"AR_RPGTO" },
	{ AR_RFCNT,		"AR_RFCNT" },
	{ AR_MISC,		"AR_MISC" },
	{ AR_RC,		"AR_RC",	AR_RC_BITS },
	{ AR_SCR,		"AR_SCR",	AR_SCR_BITS },
	{ AR_INTPEND,		"AR_INTPEND",	AR_INTPEND_BITS },
	{ AR_SFR,		"AR_SFR" },
	{ AR_PCICFG,		"AR_PCICFG",	AR_PCICFG_BITS },
	{ AR_GPIOCR,		"AR_GPIOCR" },
	{ AR_GPIODO,		"AR_GPIODO" },
	{ AR_GPIODI,		"AR_GPIODI" },
	{ AR_SREV,		"AR_SREV" },
	{ AR_EP_RDATA,		"AR_EP_RDATA" },
	{ AR_EP_STA,		"AR_EP_STA",	AR_EP_STA_BITS },
	{ AR_STA_ID0,		"AR_STA_ID0" },
	{ AR_STA_ID1,		"AR_STA_ID1",	AR_STA_ID1_BITS },
	{ AR_BSS_ID0,		"AR_BSS_ID0" },
	{ AR_BSS_ID1,		"AR_BSS_ID1" },
	{ AR_SLOT_TIME,		"AR_SLOT_TIME" },
	{ AR_TIME_OUT,		"AR_TIME_OUT" },
	{ AR_RSSI_THR,		"AR_RSSI_THR" },
	{ AR_RETRY_LMT,		"AR_RETRY_LMT" },
	{ AR_USEC,		"AR_USEC" },
	{ AR_BEACON,		"AR_PCU_BEACON", AR_BEACON_BITS },
	{ AR_CFP_PERIOD,	"AR_CFP_PERIOD" },
	{ AR_TIMER0,		"AR_TIMER0" },
	{ AR_TIMER1,		"AR_TIMER1" },
	{ AR_TIMER2,		"AR_TIMER2" },
	{ AR_TIMER3,		"AR_TIMER3" },
	{ AR_IFS0,		"AR_IFS0" },
	{ AR_IFS1,		"AR_IFS1" },
	{ AR_CFP_DUR,		"AR_CFP_DUR" },
	{ AR_RX_FILTER,		"AR_RX_FILTER",	AR_RX_FILTER_BITS },
	{ AR_MCAST_FIL0,	"AR_MCAST_FIL0" },
	{ AR_MCAST_FIL1,	"AR_MCAST_FIL1" },
	{ AR_TX_MASK0,		"AR_TX_MASK0" },
	{ AR_TX_MASK1,		"AR_TX_MASK1" },
	{ AR_CLR_TMASK,		"AR_CLR_TMASK" },
	{ AR_TRIG_LEV,		"AR_TRIG_LEV" },
	{ AR_DIAG_SW,		"AR_DIAG_SW",	AR_DIAG_SW_BITS },
	{ AR_TSF_L32,		"AR_TSF_L32" },
	{ AR_TSF_U32,		"AR_TSF_U32" },
	{ AR_LAST_TSTP,		"AR_LAST_TSTP" },
	{ AR_RETRY_CNT,		"AR_RETRY_CNT" },
	{ AR_BACKOFF,		"AR_BACKOFF" },
	{ AR_NAV,		"AR_NAV" },
	{ AR_RTS_OK,		"AR_RTS_OK" },
	{ AR_RTS_FAIL,		"AR_RTS_FAIL" },
	{ AR_ACK_FAIL,		"AR_ACK_FAIL" },
	{ AR_FCS_FAIL,		"AR_FCS_FAIL" },
	{ AR_BEACON_CNT,	"AR_BEACON_CNT" },
	{ AR_PHY_FRCTL,		"AR_PHY_FRCTL",	AR_PHY_FRCTL_BITS },
	{ AR_PHY_AGC,		"AR_PHY_AGC",	AR_PHY_AGC_BITS },
	{ AR_PHY_CHIPID,	"AR_PHY_CHIPID" },
	{ AR_PHY_ACTIVE,	"AR_PHY_ACTIVE",AR_PHY_ACTIVE_BITS },
	{ AR_PHY_AGCCTL,	"AR_PHY_AGCCTL",AR_PHY_AGCCTL_BITS },
};

void
ar5210print(FILE *fd, u_int recnum, struct athregrec *r)
{
#define	N(a)	(sizeof (a) / sizeof (a[0]))
	char buf[64];
	const char* bits;
	int i;

	for (i = 0; i < N(regnames); i++)
		if (regnames[i].off == r->reg)
			break;
	fprintf(fd, "\n%05u: ", recnum);
	if (i < N(regnames)) {
		snprintf(buf, sizeof (buf), "%s (0x%x)",
			regnames[i].name, r->reg);
		bits = regnames[i].bits;
	} else if (AR_EP_AIR_BASE <= r->reg && r->reg < 0x8000) {
		snprintf(buf, sizeof (buf), "AR_EP_AIR(%u) (0x%x)",
			(r->reg - AR_EP_AIR_BASE) >> 2, r->reg);
		bits = NULL;
	} else if (AR_KEYTABLE_0 <= r->reg && r->reg < AR_PHY_BASE) {
		snprintf(buf, sizeof (buf), "AR_KEYTABLE(%u) (0x%x)",
			(r->reg - AR_KEYTABLE_0) >> 5, r->reg);
		bits = NULL;
	} else if (AR_PHY_BASE <= r->reg) {
		snprintf(buf, sizeof (buf), "AR_PHY(%u) (0x%x)",
			(r->reg - AR_PHY_BASE) >> 2, r->reg);
		bits = NULL;
	} else {
		snprintf(buf, sizeof (buf), "0x%x", r->reg);
		bits = NULL;
	}
	fprintf(fd, "%-24s %s 0x%x", buf, r->op ? "<=" : "=>", r->val);
	if (bits) {
		const char *p = bits;
		int tmp, n;

		for (tmp = 0, p++; *p;) {
			n = *p++;
			if (r->val & (1 << (n - 1))) {
				putc(tmp ? ',' : '<', fd);
				for (; (n = *p) > ' '; ++p)
					putc(n, fd);
				tmp = 1;
			} else
				for (; *p > ' '; ++p)
					continue;
		}
		if (tmp)
			putc('>', fd);
	}
#undef N
}
