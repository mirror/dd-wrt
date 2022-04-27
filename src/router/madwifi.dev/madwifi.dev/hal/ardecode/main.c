/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/ardecode/main.c#3 $
 */
#include <stdio.h>
#include <stdlib.h>

#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "ah.h"
#include "ah_internal.h"
#include "ah_decode.h"
#include "ah_devid.h"

extern	void ar5210print(FILE *, u_int, struct athregrec *);
extern	void ar5211print(FILE *, u_int, struct athregrec *);
extern	void ar5212print(FILE *, u_int, struct athregrec *);

int	chipnum = 5210;

const char*
markname(u_int i, u_int v)
{
	static char buf[256];

	switch (i) {
	case AH_MARK_RESET:
		sprintf(buf, "ar%uReset %s", chipnum,
			v ? "change channel" : "no channel change");
		break;
	case AH_MARK_RESET_LINE:
		sprintf(buf, "ar%u_reset.c; line %u", chipnum, v);
		break;
	case AH_MARK_RESET_DONE:
		if (v)
			sprintf(buf, "ar%uReset (done), FAIL, error %u",
				chipnum, v);
		else
			sprintf(buf, "ar%uReset (done), OK", chipnum);
		break;
	case AH_MARK_CHIPRESET:
		sprintf(buf, "ar%uChipReset, channel %u Mhz", chipnum, v);
		break;
	case AH_MARK_PERCAL:
		sprintf(buf, "ar%uPerCalibration, channel %u Mhz", chipnum, v);
		break;
	case AH_MARK_SETCHANNEL:
		sprintf(buf, "ar%uSetChannel, channel %u Mhz", chipnum, v);
		break;
	case AH_MARK_ANI_RESET:
		switch (v) {
		case HAL_M_STA:
			sprintf(buf, "ar%uAniReset, HAL_M_STA", chipnum);
			break;
		case HAL_M_IBSS:
			sprintf(buf, "ar%uAniReset, HAL_M_IBSS", chipnum);
			break;
		case HAL_M_HOSTAP:
			sprintf(buf, "ar%uAniReset, HAL_M_HOSTAP", chipnum);
			break;
		case HAL_M_MONITOR:
			sprintf(buf, "ar%uAniReset, HAL_M_MONITOR", chipnum);
			break;
		default:
			sprintf(buf, "ar%uAniReset, opmode %u", chipnum, v);
			break;
		}
		break;
	case AH_MARK_ANI_POLL:
		sprintf(buf, "ar%uAniPoll, listenTime %u", chipnum, v);
		break;
	case AH_MARK_ANI_CONTROL:
		switch (v) {
		case HAL_ANI_PRESENT:
			sprintf(buf, "ar%uAniControl, PRESENT", chipnum);
			break;
		case HAL_ANI_NOISE_IMMUNITY_LEVEL:
			sprintf(buf, "ar%uAniControl, NOISE_IMMUNITY", chipnum);
			break;
		case HAL_ANI_OFDM_WEAK_SIGNAL_DETECTION:
			sprintf(buf, "ar%uAniControl, OFDM_WEAK_SIGNAL", chipnum);
			break;
		case HAL_ANI_CCK_WEAK_SIGNAL_THR:
			sprintf(buf, "ar%uAniControl, CCK_WEAK_SIGNAL", chipnum);
			break;
		case HAL_ANI_FIRSTEP_LEVEL:
			sprintf(buf, "ar%uAniControl, FIRSTEP_LEVEL", chipnum);
			break;
		case HAL_ANI_SPUR_IMMUNITY_LEVEL:
			sprintf(buf, "ar%uAniControl, SPUR_IMMUNITY", chipnum);
			break;
		case HAL_ANI_MODE:
			sprintf(buf, "ar%uAniControl, MODE", chipnum);
			break;
		case HAL_ANI_PHYERR_RESET:
			sprintf(buf, "ar%uAniControl, PHYERR_RESET", chipnum);
			break;
		default:
			sprintf(buf, "ar%uAniControl, cmd %u", chipnum, v);
			break;
		}
		break;
	default:
		sprintf(buf, "mark #%u value %u/0x%x", i, v, v);
		break;
	}
	return buf;
}

#ifndef MAP_NOCORE
#define	MAP_NOCORE	0		/* not defined on Linux */
#endif
#ifndef MAP_NORESERVE
#define	MAP_NORESERVE	0		/* not defined on BSD */
#endif

main(int argc, char *argv[])
{
	int fd, i, nrecs, same;
	struct stat sb;
	void *addr;
	const char *filename = "/tmp/ath_hal.log";
	struct athregrec *rprev;
	void (*print)(FILE *fd, u_int recnum, struct athregrec *r);

	if (argc > 1)
		filename = argv[1];
	fd = open(filename, O_RDONLY);
	if (fd < 0)
		err(1, filename);
	if (fstat(fd, &sb) < 0)
		err(1, "fstat");
	addr = mmap(0, sb.st_size, PROT_READ,
		MAP_PRIVATE|MAP_NOCORE|MAP_NORESERVE, fd, 0);
	if (addr == MAP_FAILED)
		err(1, "mmap");
	nrecs = sb.st_size / sizeof (struct athregrec);
	printf("%u records", nrecs);
	rprev = NULL;
	same = 0;
	print = ar5210print;		/* for backwards compatibility */
	for (i = 0; i < nrecs; i++) {
		struct athregrec *r = &((struct athregrec *) addr)[i];
		if (rprev && bcmp(r, rprev, sizeof (*r)) == 0) {
			same++;
			continue;
		}
		if (same)
			printf("\t\t+%u time%s", same, same == 1 ? "" : "s");
		switch (r->op) {
		case OP_DEVICE:
			switch (r->val) {
			case AR5210_PROD:
			case AR5210_DEFAULT:
				print = ar5210print;
				chipnum = 5210;
				break;
			case AR5211_DEVID:
			case AR5311_DEVID:
			case AR5211_DEFAULT:
			case AR5211_FPGA11B:
				print = ar5211print;
				chipnum = 5211;
				break;
			case AR5212_FPGA:
			case AR5212_DEVID:
			case AR5212_DEFAULT:
				print = ar5212print;
				chipnum = 5212;
				break;
			default:
				printf("Unknown device id 0x%x\n", r->val);
				exit(-1);
			}
			break;
		case OP_READ:
		case OP_WRITE:
			(*print)(stdout, i, r);
			break;
		case OP_MARK:
			printf("\n%05u: %s", i, markname(r->reg, r->val));
			break;
		}
		rprev = r;
		same = 0;
	}
	putchar('\n');
	exit(0);
}
