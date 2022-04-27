/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/ani.c#4 $
 */

#include "diag.h"

#include "ah.h"
#include "ah_devid.h"
#include "ah_internal.h"
#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"

#include <string.h>
#include <stdlib.h>
#include <err.h>

struct anistats {
	struct ath_stats ath;
	struct ar5212AniState ani_state;
	struct ar5212Stats ani_stats;
};

struct anihandler {
	struct statshandler sh;		/* base class */

	int	s;
	struct ath_diag atd;
	struct ifreq ifr;
	struct anistats total;
	struct anistats cur;
};

static void
ani_getstats(struct statshandler *sh, void *arg)
{
	struct anihandler *ah = (struct anihandler *)sh;
	struct anistats *stats = arg;

	ah->ifr.ifr_data = (caddr_t) &stats->ath;
	if (ioctl(ah->s, SIOCGATHSTATS, &ah->ifr) < 0)
		err(1, ah->ifr.ifr_name);
	ah->atd.ad_id = HAL_DIAG_ANI_CURRENT;
	ah->atd.ad_out_data = (caddr_t) &stats->ani_state;
	ah->atd.ad_out_size = sizeof(stats->ani_state);
	if (ioctl(ah->s, SIOCGATHDIAG, &ah->atd) < 0)
		err(1, ah->atd.ad_name);
	ah->atd.ad_id = HAL_DIAG_ANI_STATS;
	ah->atd.ad_out_data = (caddr_t) &stats->ani_stats;
	ah->atd.ad_out_size = sizeof(stats->ani_stats);
	if (ioctl(ah->s, SIOCGATHDIAG, &ah->atd) < 0)
		err(1, ah->atd.ad_name);
}

static void
ani_update(struct statshandler *sh)
{
	memcpy(sh->total, sh->cur, sizeof(struct anistats));
}

#define	FMT1	"%3d %3d %3d %2d %2d %2d %c%c"
#define	FMT1_a	" %3u"
#define	FMT1_b	" %3u"
#define	FMT1_c	" %3u"
#define	FMT1_d	" %3u"
#define	FMT2	" %5u"
#define	FMT3	" %5u"
#define	FMT4	" %5u"
#define	FMT5	" %5u"
#define	FMT6	" %6u"
#define	FMT7	" %3u"
#define	FMT8	" %3u"

static void
ani_printbanner(struct statshandler *sh, FILE *fd)
{
	fprintf(fd, "BSI DSI TSI NI SI ST OC NI+ NI- SI+ SI- %11s %11s %6s\n"
		, "OFDM"
		, "CCK"
		, "LISTEN"
	);
}

#define HAL_EP_RND(x,mul) \
	((((x)%(mul)) >= ((mul)/2)) ? ((x) + ((mul) - 1)) / (mul) : (x)/(mul))
#define HAL_RSSI(x)	HAL_EP_RND(x, HAL_RSSI_EP_MULTIPLIER)

void
ani_reportcommon(struct anistats *t, FILE *fd)
{
	fprintf(fd, FMT1
		, HAL_RSSI(t->ani_stats.ast_nodestats.ns_avgbrssi)
		, HAL_RSSI(t->ani_stats.ast_nodestats.ns_avgrssi)
		, HAL_RSSI(t->ani_stats.ast_nodestats.ns_avgtxrssi)
		, t->ani_state.noiseImmunityLevel
		, t->ani_state.spurImmunityLevel
		, t->ani_state.firstepLevel
		, t->ani_state.ofdmWeakSigDetectOff ? '!' : ' '
		, t->ani_state.cckWeakSigThreshold ? 'H' : ' '
	);
}

void
ani_reportdelta(struct statshandler *sh, FILE *fd)
{
#define	DELTA_STATE(field)	c->ani_state.field - t->ani_state.field
#define	DELTA_STAT(field)	c->ani_stats.field - t->ani_stats.field
	struct anistats *t = sh->total;
	struct anistats *c = sh->cur;	

	ani_reportcommon(c, fd);
	fprintf(fd, FMT1_a FMT1_b FMT1_c FMT1_d FMT2 FMT3 FMT4 FMT5 FMT6 FMT7 FMT8
		, DELTA_STAT(ast_ani_niup)
		, DELTA_STAT(ast_ani_nidown)
		, DELTA_STAT(ast_ani_spurup)
		, DELTA_STAT(ast_ani_spurdown)
		, t->ani_state.ofdmPhyErrCount
		, DELTA_STAT(ast_ani_ofdmerrs)
		, t->ani_state.cckPhyErrCount
		, DELTA_STAT(ast_ani_cckerrs)
		, t->ani_state.listenTime
		, DELTA_STAT(ast_ani_lzero)
		, DELTA_STAT(ast_ani_lneg)
	);
#undef DELTA_STAT
#undef DELTA_STATE
}

static void
ani_reporttotal(struct statshandler *sh, FILE *fd)
{
	struct anistats *t = sh->total;

	ani_reportcommon(t, fd);
	reportcol(fd, t->ani_stats.ast_ani_niup,	FMT1_a, 999, " %2u%c");
	reportcol(fd, t->ani_stats.ast_ani_nidown,	FMT1_b, 999, " %2u%c");
	reportcol(fd, t->ani_stats.ast_ani_spurup,	FMT1_c, 999, " %2u%c");
	reportcol(fd, t->ani_stats.ast_ani_spurdown,	FMT1_d, 999, " %2u%c");
	fprintf(fd, FMT2, t->ani_state.ofdmPhyErrCount);
	reportcol(fd, t->ani_stats.ast_ani_ofdmerrs,	FMT3, 99999, " %4u%c");
	fprintf(fd, FMT4, t->ani_state.ofdmPhyErrCount);
	reportcol(fd, t->ani_stats.ast_ani_cckerrs,	FMT5, 99999, " %4u%c");
	fprintf(fd, FMT6 , t->ani_state.listenTime);
	reportcol(fd, t->ani_stats.ast_ani_lzero,	FMT7, 999, " %2u%c");
	reportcol(fd, t->ani_stats.ast_ani_lneg,	FMT8, 999, " %2u%c");
}

static void ar5212AniReport(struct statshandler *, FILE *fd);

static void
anicontrol(struct anihandler *ah, int op, int param)
{
	u_int32_t args[2];

	args[0] = op;
	args[1] = param;
	ah->atd.ad_id = HAL_DIAG_ANI_CMD | ATH_DIAG_IN;
	ah->atd.ad_out_data = NULL;
	ah->atd.ad_out_size = 0;
	ah->atd.ad_in_data = (caddr_t) args;
	ah->atd.ad_in_size = sizeof(args);
	if (ioctl(ah->s, SIOCGATHDIAG, &ah->atd) < 0)
		err(1, ah->atd.ad_name);
}

static void
usage(void)
{
	const char *msg = "\
Usage: ani [-i device] [cmd]\n\
on                  enable ani operation\n\
off                 disable ani operation\n\
noise X             set noise immunity level to X [0..4]\n\
weak_detect X       set OFDM week signal detection to X [0,1]\n\
weak_threshold X    set CCK week signal threshold to X [0,1]\n\
firstep X           set first step level to X [0..2]\n\
spur X              set spure immunity level to X [0..7]\n\
\n\
Or: ani [-i device] [N] to get a rolling display to be printed N seconds\n";

	fprintf(stderr, "%s", msg);
}

int
main(int argc, char *argv[])
{
#define	streq(a,b)	(strcasecmp(a,b) == 0)
	struct anihandler ani;
	const char *ifname;
	HAL_REVS revs;

	memset(&ani, 0, sizeof(ani));
	ani.s = socket(AF_INET, SOCK_DGRAM, 0);
	if (ani.s < 0)
		err(1, "socket");
	ifname = getenv("ATH");
	if (!ifname)
		ifname = ATH_DEFAULT;

	if (argc > 1 && strcmp(argv[1], "-i") == 0) {
		if (argc < 2) {
			fprintf(stderr, "%s: missing interface name for -i\n",
				argv[0]);
			exit(-1);
		}
		strncpy(ani.atd.ad_name, argv[2], sizeof (ani.atd.ad_name));
		argc -= 2, argv += 2;
	}
	strncpy(ani.atd.ad_name, ifname, sizeof (ani.atd.ad_name));
	strncpy(ani.ifr.ifr_name, ifname, sizeof (ani.ifr.ifr_name));

	ani.atd.ad_id = HAL_DIAG_REVS;
	ani.atd.ad_out_data = (caddr_t) &revs;
	ani.atd.ad_out_size = sizeof(revs);
	if (ioctl(ani.s, SIOCGATHDIAG, &ani.atd) < 0)
		err(1, ani.atd.ad_name);
	switch (revs.ah_devid) {
	case AR5210_PROD:
	case AR5210_DEFAULT:
		printf("No ANI settings for a 5210\n");
		exit(0);
	case AR5211_DEVID:
	case AR5311_DEVID:
	case AR5211_DEFAULT:
	case AR5211_FPGA11B:
		printf("No ANI settings for a 5211\n");
		exit(0);
	case AR5212_FPGA:
	case AR5212_DEVID:
	case AR5212_DEVID_IBM:
	case AR5212_DEFAULT:
	case AR5416_DEVID_EMU_PCI:
	case AR5416_DEVID_EMU_PCIE:
	case AR5416_DEVID:
	case AR5418_DEVID:
	case AR5212_AR5312_REV2:
	case AR5212_AR5312_REV7:
	case AR5212_AR2313_REV8:
	case AR5212_AR2315_REV6:
	case AR5212_AR2315_REV7:
	case AR5212_AR2317_REV1:
	case AR5212_AR2317_REV2:
		ani.sh.reportverbose = ar5212AniReport;
		break;
	default:
		printf("No ANI settings for device 0x%x\n", revs.ah_devid);
		exit(0);
	}
	ani.sh.total = &ani.total;
	ani.sh.cur = &ani.cur;
	ani.sh.getstats = ani_getstats;
	ani.sh.update = ani_update;
	ani.sh.printbanner = ani_printbanner;
	ani.sh.reportdelta = ani_reportdelta;
	ani.sh.reporttotal = ani_reporttotal;

	if (argc > 1) {
		if (streq(argv[1], "on") || streq(argv[1], "auto")) {
			anicontrol(&ani, HAL_ANI_MODE, 1);
		} else if (streq(argv[1], "off") || streq(argv[1], "manual")) {
			anicontrol(&ani, HAL_ANI_MODE, 0);
		} else if (streq(argv[1], "noise")) {
			anicontrol(&ani, HAL_ANI_NOISE_IMMUNITY_LEVEL,
				atoi(argv[2]));
		} else if (streq(argv[1], "weak_detect")) {
			anicontrol(&ani, HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION,
				atoi(argv[2]));
		} else if (streq(argv[1], "weak_threshold")) {
			anicontrol(&ani, HAL_ANI_CCK_WEAK_SIGNAL_THR,
				atoi(argv[2]));
		} else if (streq(argv[1], "firstep")) {
			anicontrol(&ani, HAL_ANI_FIRSTEP_LEVEL, atoi(argv[2]));
		} else if (streq(argv[1], "spur")) {
			anicontrol(&ani, HAL_ANI_SPUR_IMMUNITY_LEVEL,
				atoi(argv[2]));
		} else if (isdigit(argv[1][0])) {
			ani.sh.interval = strtoul(argv[1], NULL, 0);
			runstats(stdout, &ani.sh);
			/*NOTREACHED*/
		} else {
			usage();
		}
	} else {
		reportstats(stdout, &ani.sh);
	}
	return 0;
}

static void
ar5212AniReport(struct statshandler *sh, FILE *fd)
{
	struct anihandler *ah = (struct anihandler *)sh;
	struct anistats *c = sh->total;

	fprintf(fd, "%24s: %u%c\n"
		, "Channel"
		, c->ani_state.c.channel
		, IS_CHAN_T(&c->ani_state.c) ? 'T' :
		  IS_CHAN_A(&c->ani_state.c) ? 'A' :
		  IS_CHAN_G(&c->ani_state.c) ? 'G' :
		  IS_CHAN_B(&c->ani_state.c) ? 'B' :
		  IS_CHAN_X(&c->ani_state.c) ? 'X' : '?');
	fprintf(fd, "%24s: %3d    [%4d %-9s %4d %-9s]\n"
		, "Beacon RSSI Average"
		, HAL_RSSI(c->ani_stats.ast_nodestats.ns_avgbrssi)
		, c->ani_state.rssiThrLow, "ThreshLow"
		, c->ani_state.rssiThrHigh, "ThreshHigh"
	);
	fprintf(fd, "%24s: %3u    [%4u %-9s %4u %-9s]\n"
		, "OFDM PHY errors"
		, c->ani_state.ofdmPhyErrCount
		, c->ani_state.ofdmTrigHigh, "TrigHigh"
		, c->ani_state.ofdmTrigLow, "TrigLow"
	);
	fprintf(fd, "%24s: %3u    [%4u %-9s %4u %-9s]\n"
		, "CCK PHY errors"
		, c->ani_state.cckPhyErrCount
		, c->ani_state.cckTrigHigh, "TrigHigh"
		, c->ani_state.cckTrigLow, "TrigLow"
	);
	fprintf(fd, "%24s: %3d    [%4u times forced to zero %4u negative]\n"
		, "Listen Time"
		, c->ani_state.listenTime
		, c->ani_stats.ast_ani_lzero
		, c->ani_stats.ast_ani_lneg
	);
	if (c->ani_stats.ast_ani_reset != 0)
		fprintf(fd, "%24s: %u\n"
			, "ANI reset 'cuz non-STA"
			, c->ani_stats.ast_ani_reset
		);
	fprintf(fd, "%24s: %3d    [%4u %-9s %4u %-9s]\n"
		, "Noise Immunity Level", c->ani_state.noiseImmunityLevel
		, c->ani_stats.ast_ani_niup, "increases"
		, c->ani_stats.ast_ani_nidown, "decreases"
	);
	fprintf(fd, "%24s: %3d    [%4u %-9s %4u %-9s]\n"
		, "Spur Immunity Level", c->ani_state.spurImmunityLevel
		, c->ani_stats.ast_ani_spurup, "increases"
		, c->ani_stats.ast_ani_spurdown, "decreases"
	);
	fprintf(fd, "%24s: %3d    [%4u %-9s %4u %-9s]\n"
		, "Firstep Level", c->ani_state.firstepLevel
		, c->ani_stats.ast_ani_stepup, "increases"
		, c->ani_stats.ast_ani_stepdown, "decreases"
	);
	fprintf(fd, "%24s: %3s    [%4u %-9s %4u %-9s]\n"
		, "OFDM Weak Signal Detect"
		, c->ani_state.ofdmWeakSigDetectOff ? "OFF" : "ON"
		, c->ani_stats.ast_ani_ofdmon, "on"
		, c->ani_stats.ast_ani_ofdmoff, "off"
	);
	fprintf(fd, "%24s: %3s    [%4u %-9s %4u %-9s]\n"
		, "CCK Weak Signal Thresh"
		, c->ani_state.cckWeakSigThreshold ? "ON" : "OFF"
		, c->ani_stats.ast_ani_cckhigh, "high"
		, c->ani_stats.ast_ani_ccklow, "low"
	);
	if (c->ani_stats.ast_mibstats.ackrcv_bad != 0)
		fprintf(fd, "%24s: %u\n"
			, "MIB: ACK receive bad"
			, c->ani_stats.ast_mibstats.ackrcv_bad
		);
	if (c->ani_stats.ast_mibstats.rts_bad != 0)
		fprintf(fd, "%24s: %u\n"
			, "MIB: RTS bad"
			, c->ani_stats.ast_mibstats.rts_bad
		);
	if (c->ani_stats.ast_mibstats.rts_good != 0)
		fprintf(fd, "%24s: %u\n"
			, "MIB: RTS good"
			, c->ani_stats.ast_mibstats.rts_good
		);
	if (c->ani_stats.ast_mibstats.fcs_bad != 0)
		fprintf(fd, "%24s: %u\n"
			, "MIB: FCS good"
			, c->ani_stats.ast_mibstats.fcs_bad
		);
	if (c->ani_stats.ast_mibstats.beacons != 0)
		fprintf(fd, "%24s: %u\n"
			, "MIB: beacons"
			, c->ani_stats.ast_mibstats.beacons
		);
}
