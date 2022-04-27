/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/rfgain.c#2 $
 */
#include "diag.h"

#include <getopt.h>

#include "ah.h"
#include "ah_devid.h"
#include "ah_internal.h"
#undef AH_PRIVATE		/* XXX twist defs in ar5212.h to suit */
#define	AH_PRIVATE(ah)	(ah)
#include "ar5212/ar5212.h"
#include "ar5212/ar5212reg.h"

#include <stdlib.h>
#include <string.h>

struct rfstats {
	GAIN_VALUES gv;
	GAIN_OPTIMIZATION_STEP step;
};

struct rfhandler {
	struct statshandler sh;		/* base class */

	int	s;
	struct ath_diag atd;
	struct rfstats cur;
	HAL_REVS revs;
};

static void
rf_getstats(struct statshandler *sh, void *arg)
{
	struct rfhandler *rh = (struct rfhandler *)sh;
	struct rfstats *stats = arg;

	rh->atd.ad_id = HAL_DIAG_RFGAIN;
	rh->atd.ad_out_data = (caddr_t) &stats->gv;
	rh->atd.ad_out_size = sizeof(stats->gv);
	if (ioctl(rh->s, SIOCGATHDIAG, &rh->atd) < 0)
		err(1, rh->atd.ad_name);

	rh->atd.ad_id = HAL_DIAG_RFGAIN_CURSTEP;
	rh->atd.ad_out_data = (caddr_t) &stats->step;
	rh->atd.ad_out_size = sizeof(stats->step);
	if (ioctl(rh->s, SIOCGATHDIAG, &rh->atd) < 0)
		err(1, rh->atd.ad_name);
	/* XXX override kernel address */
	stats->gv.currStep = &stats->step;
}

static void
rf_update(struct statshandler *sh)
{
}

static void
rfDumpRfgain(struct statshandler *sh, FILE *fd)
{
	struct rfhandler *rh = (struct rfhandler *)sh;
	GAIN_VALUES *gv = sh->cur;

	fprintf(fd, "currStepNum %d currGain %d targetGain %d\n",
		gv->currStepNum , gv->currGain , gv->targetGain);
	fprintf(fd, "gainFCorrection %d loTrig %d hiTrig %d\n",
		gv->gainFCorrection, gv->loTrig, gv->hiTrig);
	fprintf(fd, "[Cur] StepName: %s StepGain: %2d ",
		gv->currStep->stepName , gv->currStep->stepGain);
	if ((rh->revs.ah_analog5GhzRev&0xf0) >= AR_RAD5112_SREV_MAJOR) {
		fprintf(fd, "rf_mixgain_ovr %d rf_pwd138 %d "
			"rf_pwd137 %d rf_pwd136 %d\n\t"
			"rf_pwd132 %d rf_pwd131 %d rf_pwd130 %d\n"
			, gv->currStep->paramVal[0]
			, gv->currStep->paramVal[1]
			, gv->currStep->paramVal[2]
			, gv->currStep->paramVal[3]
			, gv->currStep->paramVal[4]
			, gv->currStep->paramVal[5]
			, gv->currStep->paramVal[6]
		);
	} else {
		fprintf(fd, "bb_tx_clip %d  rf_pwd_90 %d  "
			"rf_pwd_84 %d  rf_rfgainsel %d\n"
			, gv->currStep->paramVal[0]
			, gv->currStep->paramVal[1]
			, gv->currStep->paramVal[2]
			, gv->currStep->paramVal[3]
		);
	}
}

static void
rf_printbanner(struct statshandler *sh, FILE *fd)
{
	struct rfhandler *rh = (struct rfhandler *)sh;

	fprintf(fd, "S# NAME CGN TGN SGN FCOR LO HI");
	if ((rh->revs.ah_analog5GhzRev&0xf0) >= AR_RAD5112_SREV_MAJOR) {
		fprintf(fd, " MIX P138 P137 P136 P132 P131 P130\n");
	} else {
		fprintf(fd, " TXCLIP P90 P84 GAINSEL\n");
	}
}

static void
rf_reportcur(struct statshandler *sh, FILE *fd)
{
	struct rfhandler *rh = (struct rfhandler *)sh;
	GAIN_VALUES *gv = sh->cur;

	fprintf(fd, "%2d %4s %3d %3d %3d %4d %2d %2d"
		, gv->currStepNum
		, gv->currStep->stepName
		, gv->currGain
		, gv->currStep->stepGain
		, gv->targetGain
		, gv->gainFCorrection
		, gv->loTrig
		, gv->hiTrig
	);
	if ((rh->revs.ah_analog5GhzRev&0xf0) >= AR_RAD5112_SREV_MAJOR) {
		fprintf(fd, " %3d %4d %4d %4d %4d %4d %4d"
			, gv->currStep->paramVal[0]
			, gv->currStep->paramVal[1]
			, gv->currStep->paramVal[2]
			, gv->currStep->paramVal[3]
			, gv->currStep->paramVal[4]
			, gv->currStep->paramVal[5]
			, gv->currStep->paramVal[6]
		);
	} else {
		fprintf(fd, " %6d %3d %3d %7d"
			, gv->currStep->paramVal[0]
			, gv->currStep->paramVal[1]
			, gv->currStep->paramVal[2]
			, gv->currStep->paramVal[3]
		);
	}
}

static void
printRevs(FILE *fd, const HAL_REVS *revs)
{
	const char *rfbackend;

	fprintf(fd, "PCI device id 0x%x subvendor id 0x%x\n",
		revs->ah_devid, revs->ah_subvendorid);
	fprintf(fd, "mac %d.%d phy %d.%d"
		, revs->ah_macVersion, revs->ah_macRev
		, revs->ah_phyRev >> 4, revs->ah_phyRev & 0xf
	);
	rfbackend = IS_5413(revs) ? "5413" :
		    IS_2413(revs) ? "2413" :
		    IS_5112(revs) ? "5112" :
				    "5111";
	if (revs->ah_analog5GhzRev && revs->ah_analog2GhzRev)
		fprintf(fd, " 5ghz radio %d.%d 2ghz radio %d.%d (%s)\n"
			, revs->ah_analog5GhzRev >> 4
			, revs->ah_analog5GhzRev & 0xf
			, revs->ah_analog2GhzRev >> 4
			, revs->ah_analog2GhzRev & 0xf
			, rfbackend
		);
	else
		fprintf(fd, " radio %d.%d (%s)\n"
			, revs->ah_analog5GhzRev >> 4
			, revs->ah_analog5GhzRev & 0xf
			, rfbackend
		);
}

static void
usage(const char *progname)
{
	fprintf(stderr, "usage: %s [-v] [-i dev]\n", progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct rfhandler rf;
	const char *ifname;
	int verbose = 0, c;

	memset(&rf, 0, sizeof(rf));
	rf.s = socket(AF_INET, SOCK_DGRAM, 0);
	if (rf.s < 0)
		err(1, "socket");
	ifname = getenv("ATH");
	if (!ifname)
		ifname = ATH_DEFAULT;
	while ((c = getopt(argc, argv, "i:v")) != -1)
		switch (c) {
		case 'i':
			ifname = optarg;
			break;
		case 'v':
			verbose++;
			break;
		default:
			usage(argv[0]);
		}
	strncpy(rf.atd.ad_name, ifname, sizeof (rf.atd.ad_name));
	argc -= optind, argv += optind;

	rf.atd.ad_id = HAL_DIAG_REVS;
	rf.atd.ad_out_data = (caddr_t) &rf.revs;
	rf.atd.ad_out_size = sizeof(rf.revs);
	if (ioctl(rf.s, SIOCGATHDIAG, &rf.atd) < 0)
		err(1, rf.atd.ad_name);

	if (verbose)
		printRevs(stdout, &rf.revs);

	switch (rf.revs.ah_devid) {
	case AR5210_PROD:
	case AR5210_DEFAULT:
		printf("No RFGAIN settings for a 5210\n");
		exit(0);
	}
	rf.sh.total = &rf.cur.gv;
	rf.sh.cur = &rf.cur.gv;
	rf.sh.getstats = rf_getstats;
	rf.sh.update = rf_update;
	rf.sh.printbanner = rf_printbanner;
	rf.sh.reportdelta = rf_reportcur;
	rf.sh.reporttotal = rf_reportcur;
	rf.sh.reportverbose = rfDumpRfgain;

	if (argc > 1) {
		rf.sh.interval = strtoul(argv[1], NULL, 0);
		runstats(stdout, &rf.sh);
		/*NOTREACHED*/
	} else {
		reportstats(stdout, &rf.sh);
	}
	return 0;
}
