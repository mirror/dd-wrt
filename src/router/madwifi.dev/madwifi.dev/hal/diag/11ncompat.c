/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2002-2006 Atheros Communications, Inc.
 * All rights reserved.
 *
 * $Id: //depot/sw/branches/sam_hal/diag/11ncompat.c#1 $
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

static void
usage(void)
{
	const char *msg = "\
Usage: 11ncompat [-i device] [cmd]\n\
on                  enable 11ncompat\n\
off                 disable 11ncompat\n\
services            set services\n\
stomp               set tx stomp\n\
";
	fprintf(stderr, "%s", msg);
}

int
main(int argc, char *argv[])
{
#define	streq(a,b)	(strcasecmp(a,b) == 0)
	struct ath_diag atd;
	const char *ifname;
	int s, setting;

	memset(&atd, 0, sizeof(atd));
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
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
		ifname = argv[2];
		argc -= 2, argv += 2;
	}
	strncpy(atd.ad_name, ifname, sizeof (atd.ad_name));

	if (argc > 1) {
		u_int32_t args[1];

		atd.ad_id = HAL_DIAG_ANI_CMD | ATH_DIAG_IN | HAL_DIAG_11NCOMPAT;
		atd.ad_out_data = NULL;
		atd.ad_out_size = 0;
		atd.ad_in_data = (caddr_t) args;
		atd.ad_in_size = sizeof(args);
		args[0] = 0;
		if (streq(argv[1], "on")) {
			args[0] = HAL_DIAG_11N_SERVICES|HAL_DIAG_11N_TXSTOMP;
		} else if (streq(argv[1], "off")) {
			args[0] = 0;
		} else if (streq(argv[1], "services")) {
			args[0] = SM(3, HAL_DIAG_11N_SERVICES);
		} else if (streq(argv[1], "-services")) {
			args[0] = SM(2, HAL_DIAG_11N_SERVICES);
		} else if (streq(argv[1], "stomp")) {
			args[0] = SM(3, HAL_DIAG_11N_TXSTOMP);
		} else if (streq(argv[1], "-stomp")) {
			args[0] = SM(2, HAL_DIAG_11N_TXSTOMP);
		} else {
			usage();
		}
		if (ioctl(s, SIOCGATHDIAG, &atd) < 0)
			err(1, atd.ad_name);
	}
	return 0;
}
