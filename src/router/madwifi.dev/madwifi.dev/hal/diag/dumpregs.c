/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/dumpregs.c#4 $
 */
#include "diag.h"

#include "ah.h"
#include "ah_devid.h"
#include "ah_internal.h"

#include "dumpregs.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct	ath_diag atd;
int	s;
u_int32_t regdata[0xffff / sizeof(u_int32_t)];
#undef OS_REG_READ
#define	OS_REG_READ(ah, off)	regdata[(off) >> 2]

extern	void ar5210SetupRegs(struct ath_diag *, const HAL_REVS *, int);
extern	void ar5210DumpRegs(FILE *fd, const HAL_REVS *, int what);
extern	void ar5211SetupRegs(struct ath_diag *, const HAL_REVS *, int);
extern	void ar5211DumpRegs(FILE *fd, const HAL_REVS *, int what);
extern	void ar5212SetupRegs(struct ath_diag *, const HAL_REVS *, int);
extern	void ar5212DumpRegs(FILE *fd, const HAL_REVS *, int what);
extern	void ar5416SetupRegs(struct ath_diag *, const HAL_REVS *, int);
extern	void ar5416DumpRegs(FILE *fd, const HAL_REVS *, int what);

static void
usage(void)
{
	fprintf(stderr, "usage: dumpregs [-I interface] [-abkipx]\n");
	exit(-1);
}

int
main(int argc, char *argv[])
{
	const char *ifname;
	HAL_REVS revs;
	u_int32_t *data;
	u_int32_t *dp, *ep;
	int what, c;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(1, "socket");
	ifname = getenv("ATH");
	if (!ifname)
		ifname = ATH_DEFAULT;

	what = 0;
	while ((c = getopt(argc, argv, "I:abdkipqx")) != -1)
		switch (c) {
		case 'a':
			what |= DUMP_ALL;
			break;
		case 'b':
			what |= DUMP_BASEBAND;
			break;
		case 'd':
			what |= DUMP_DCU;
			break;
		case 'k':
			what |= DUMP_KEYCACHE;
			break;
		case 'i':
			what |= DUMP_INTERRUPT;
			break;
		case 'I':
			ifname = optarg;
			break;
		case 'p':
			what |= DUMP_PUBLIC;
			break;
		case 'q':
			what |= DUMP_QCU;
			break;
		case 'x':
			what |= DUMP_XR;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	strncpy(atd.ad_name, ifname, sizeof (atd.ad_name));

	argc -= optind;
	argv += optind;
	if (what == 0)
		what = DUMP_BASIC;

	atd.ad_id = HAL_DIAG_REVS;
	atd.ad_out_data = (caddr_t) &revs;
	atd.ad_out_size = sizeof(revs);
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);

	switch (revs.ah_devid) {
	case AR5210_PROD:
	case AR5210_DEFAULT:
		ar5210SetupRegs(&atd, &revs, what);
		break;
	case AR5211_DEVID:
	case AR5311_DEVID:
	case AR5211_DEFAULT:
	case AR5211_FPGA11B:
		ar5211SetupRegs(&atd, &revs, what);
		break;
	case AR5212_FPGA:
	case AR5212_DEVID:
	case AR5212_DEVID_IBM:
	case AR5212_DEFAULT:
	case AR5212_AR2317_REV1:
	case AR5212_AR5312_REV7:
		ar5212SetupRegs(&atd, &revs, what);
		break;
	case AR5416_DEVID_EMU_PCI:
	case AR5416_DEVID_EMU_PCIE:
	case AR5416_DEVID:
	case AR5418_DEVID:
		ar5416SetupRegs(&atd, &revs, what);
		break;
	default:
		fprintf(stderr, "devid=0x%x not supported\n",revs.ah_devid);	   
		break;
	}
	atd.ad_out_size = ath_hal_setupdiagregs((HAL_REGRANGE *) atd.ad_in_data,
		atd.ad_in_size / sizeof(HAL_REGRANGE));
	atd.ad_out_data = (caddr_t) malloc(atd.ad_out_size);
	if (atd.ad_out_data == NULL) {
		fprintf(stderr, "Cannot malloc output buffer, size %u\n",
			atd.ad_out_size);
		exit(-1);
	}
	atd.ad_id = HAL_DIAG_REGS | ATH_DIAG_IN | ATH_DIAG_DYN;
	if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
		err(1, atd.ad_name);

	/*
	 * Expand register data into global space that can be
	 * indexed directly by register offset.
	 */
	dp = (u_int32_t *)atd.ad_out_data;
	ep = (u_int32_t *)(atd.ad_out_data + atd.ad_out_size);
	while (dp < ep) {
		u_int r = dp[0] >> 16;		/* start of range */
		u_int e = dp[0] & 0xffff;	/* end of range */
		dp++;
		/* convert offsets to indices */
		r >>= 2; e >>= 2;
		do {
			if (dp >= ep) {
				fprintf(stderr, "Warning, botched return data;"
					"register at offset 0x%x not present\n",
					r << 2);
				break;
			}
			regdata[r++] = *dp++;
		} while (r <= e);
	} 

	switch (revs.ah_devid) {
	case AR5210_PROD:
	case AR5210_DEFAULT:
		ar5210DumpRegs(stdout, &revs, what);
		break;
	case AR5211_DEVID:
	case AR5311_DEVID:
	case AR5211_DEFAULT:
	case AR5211_FPGA11B:
		ar5211DumpRegs(stdout, &revs, what);
		break;
	case AR5212_FPGA:
	case AR5212_DEVID:
	case AR5212_DEVID_IBM:
	case AR5212_DEFAULT:
	case AR5212_AR2317_REV1:
	case AR5212_AR5312_REV7:
		ar5212DumpRegs(stdout, &revs, what);
		break;
	case AR5416_DEVID_EMU_PCI:
	case AR5416_DEVID_EMU_PCIE:
	case AR5416_DEVID:
	case AR5418_DEVID:
		ar5416DumpRegs(stdout, &revs, what);
		break;
	default:
		fprintf(stderr, "devid=0x%x not supported\n",revs.ah_devid);	   
		break;
	}
	return 0;
}

void
ath_hal_dumprange(FILE *fd, u_int a, u_int b)
{
	u_int r;

	for (r = a; r+16 <= b; r += 5*4)
		fprintf(fd,
			"%04x %08x  %04x %08x  %04x %08x  %04x %08x  %04x %08x\n"
			, r, OS_REG_READ(ah, r)
			, r+4, OS_REG_READ(ah, r+4)
			, r+8, OS_REG_READ(ah, r+8)
			, r+12, OS_REG_READ(ah, r+12)
			, r+16, OS_REG_READ(ah, r+16)
		);
	switch (b-r) {
	case 16:
		fprintf(fd
			, "%04x %08x  %04x %08x  %04x %08x  %04x %08x\n"
			, r, OS_REG_READ(ah, r)
			, r+4, OS_REG_READ(ah, r+4)
			, r+8, OS_REG_READ(ah, r+8)
			, r+12, OS_REG_READ(ah, r+12)
		);
		break;
	case 12:
		fprintf(fd, "%04x %08x  %04x %08x  %04x %08x\n"
			, r, OS_REG_READ(ah, r)
			, r+4, OS_REG_READ(ah, r+4)
			, r+8, OS_REG_READ(ah, r+8)
		);
		break;
	case 8:
		fprintf(fd, "%04x %08x  %04x %08x\n"
			, r, OS_REG_READ(ah, r)
			, r+4, OS_REG_READ(ah, r+4)
		);
		break;
	case 4:
		fprintf(fd, "%04x %08x\n"
			, r, OS_REG_READ(ah, r)
		);
		break;
	}
}

void
ath_hal_dumpregs(FILE *fd, const HAL_REG regs[], u_int nregs)
{
	int i;

	for (i = 0; i+3 < nregs; i += 4)
		fprintf(fd,
			"%-8s %08x  %-8s %08x  %-8s %08x  %-8s %08x\n"
			, regs[i+0].label, OS_REG_READ(ah, regs[i+0].reg)
			, regs[i+1].label, OS_REG_READ(ah, regs[i+1].reg)
			, regs[i+2].label, OS_REG_READ(ah, regs[i+2].reg)
			, regs[i+3].label, OS_REG_READ(ah, regs[i+3].reg)
		);
	switch (nregs - i) {
	case 3:
		fprintf(fd, "%-8s %08x  %-8s %08x  %-8s %08x\n"
			, regs[i+0].label, OS_REG_READ(ah, regs[i+0].reg)
			, regs[i+1].label, OS_REG_READ(ah, regs[i+1].reg)
			, regs[i+2].label, OS_REG_READ(ah, regs[i+2].reg)
		);
		break;
	case 2:
		fprintf(fd, "%-8s %08x  %-8s %08x\n"
			, regs[i+0].label, OS_REG_READ(ah, regs[i+0].reg)
			, regs[i+1].label, OS_REG_READ(ah, regs[i+1].reg)
		);
		break;
	case 1:
		fprintf(fd, "%-8s %08x\n"
			, regs[i+0].label, OS_REG_READ(ah, regs[i+0].reg)
		);
		break;
	}
}

u_int
ath_hal_setupdiagregs(const HAL_REGRANGE regs[], u_int nr)
{
	u_int space;
	int i;

	space = 0;
	for (i = 0; i < nr; i++) {
		u_int n = 2 * sizeof(u_int32_t);	/* reg range + first */
		if (regs[i].end) {
			if (regs[i].end < regs[i].start) {
				fprintf(stderr, "%s: bad register range, "
					"end 0x%x < start 0x%x\n",
					__func__, regs[i].end, regs[i].end);
				exit(-1);
			}
			n += regs[i].end - regs[i].start;
		}
		space += n;
	}
	return space;
}

/*
 * Format an Ethernet MAC for printing.
 */
const char*
ether_sprintf(const u_int8_t *mac)
{
	static char etherbuf[18];
	snprintf(etherbuf, sizeof(etherbuf), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return etherbuf;
}

/* XXX cheat, 5212 has a superset of the key table defs */
#include "ar5212/ar5212reg.h"

#ifndef isclr
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
#endif

void
ath_hal_dumpkeycache(FILE *fd, int nkeys, int micEnabled)
{
	static const char *keytypenames[] = {
		"WEP-40", 	/* AR_KEYTABLE_TYPE_40 */
		"WEP-104",	/* AR_KEYTABLE_TYPE_104 */
		"#2",
		"WEP-128",	/* AR_KEYTABLE_TYPE_128 */
		"TKIP",		/* AR_KEYTABLE_TYPE_TKIP */
		"AES-OCB",	/* AR_KEYTABLE_TYPE_AES */
		"AES-CCM",	/* AR_KEYTABLE_TYPE_CCM */
		"CLR",		/* AR_KEYTABLE_TYPE_CLR */
	};
	u_int8_t mac[IEEE80211_ADDR_LEN];
	u_int8_t ismic[128/NBBY];
	int entry;
	int first = 1;

	memset(ismic, 0, sizeof(ismic));
	for (entry = 0; entry < nkeys; entry++) {
		u_int32_t macLo, macHi, type;
		u_int32_t key0, key1, key2, key3, key4;

		macHi = OS_REG_READ(ah, AR_KEYTABLE_MAC1(entry));
		if ((macHi & AR_KEYTABLE_VALID) == 0 && isclr(ismic, entry))
			continue;
		macLo = OS_REG_READ(ah, AR_KEYTABLE_MAC0(entry));
		macHi <<= 1;
		if (macLo & (1<<31))
			macHi |= 1;
		macLo <<= 1;
		mac[4] = macHi & 0xff;
		mac[5] = macHi >> 8;
		mac[0] = macLo & 0xff;
		mac[1] = macLo >> 8;
		mac[2] = macLo >> 16;
		mac[3] = macLo >> 24;
		type = OS_REG_READ(ah, AR_KEYTABLE_TYPE(entry));
		if ((type & 7) == AR_KEYTABLE_TYPE_TKIP && micEnabled)
			setbit(ismic, entry+64);
		key0 = OS_REG_READ(ah, AR_KEYTABLE_KEY0(entry));
		key1 = OS_REG_READ(ah, AR_KEYTABLE_KEY1(entry));
		key2 = OS_REG_READ(ah, AR_KEYTABLE_KEY2(entry));
		key3 = OS_REG_READ(ah, AR_KEYTABLE_KEY3(entry));
		key4 = OS_REG_READ(ah, AR_KEYTABLE_KEY4(entry));
		if (first) {
			fprintf(fd, "\n");
			first = 0;
		}
		fprintf(fd, "KEY[%03u] MAC %s %-7s %02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x\n"
			, entry
			, ether_sprintf(mac)
			, isset(ismic, entry) ? "MIC" : keytypenames[type & 7]
			, (key0 >>  0) & 0xff
			, (key0 >>  8) & 0xff
			, (key0 >> 16) & 0xff
			, (key0 >> 24) & 0xff
			, (key1 >>  0) & 0xff
			, (key1 >>  8) & 0xff
			, (key2 >>  0) & 0xff
			, (key2 >>  8) & 0xff
			, (key2 >> 16) & 0xff
			, (key2 >> 24) & 0xff
			, (key3 >>  0) & 0xff
			, (key3 >>  8) & 0xff
			, (key4 >>  0) & 0xff
			, (key4 >>  8) & 0xff
			, (key4 >> 16) & 0xff
			, (key4 >> 24) & 0xff
		);
	}
}
