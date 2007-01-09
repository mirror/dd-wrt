/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: athchans.c 1426 2006-02-01 20:07:11Z mrenzmann $
 */

/*
 * athchans [-i interface] chan ...
 * (default interface is wifi0).
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include "wireless_copy.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"

#ifdef DOMULTI
#include "do_multi.h"
#endif

static	int s = -1;
const char *progname;

static void
checksocket()
{
	if (s < 0 ? (s = socket(AF_INET, SOCK_DGRAM, 0)) == -1 : 0)
		perror("socket(SOCK_DGRAM)");
}

#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int
set80211priv(const char *dev, int op, void *data, int len, int show_err)
{
	struct iwreq iwr;

	checksocket();

	memset(&iwr, 0, sizeof(iwr));
	strncpy(iwr.ifr_name, dev, IFNAMSIZ);
	if (len < IFNAMSIZ) {
		/*
		 * Argument data fits inline; put it there.
		 */
		memcpy(iwr.u.name, data, len);
	} else {
		/*
		 * Argument data too big for inline transfer; setup a
		 * parameter block instead; the kernel will transfer
		 * the data for the driver.
		 */
		iwr.u.data.pointer = data;
		iwr.u.data.length = len;
	}

	if (ioctl(s, op, &iwr) < 0) {
		if (show_err) {
			static const char *opnames[] = {
				IOCTL_ERR(IEEE80211_IOCTL_SETPARAM),
				IOCTL_ERR(IEEE80211_IOCTL_GETPARAM),
				IOCTL_ERR(IEEE80211_IOCTL_SETMODE),
				IOCTL_ERR(IEEE80211_IOCTL_GETMODE),
				IOCTL_ERR(IEEE80211_IOCTL_SETWMMPARAMS),
				IOCTL_ERR(IEEE80211_IOCTL_GETWMMPARAMS),
				IOCTL_ERR(IEEE80211_IOCTL_SETCHANLIST),
				IOCTL_ERR(IEEE80211_IOCTL_GETCHANLIST),
				IOCTL_ERR(IEEE80211_IOCTL_CHANSWITCH),
				IOCTL_ERR(IEEE80211_IOCTL_GETCHANINFO),
				IOCTL_ERR(IEEE80211_IOCTL_SETOPTIE),
				IOCTL_ERR(IEEE80211_IOCTL_GETOPTIE),
				IOCTL_ERR(IEEE80211_IOCTL_SETMLME),
				IOCTL_ERR(IEEE80211_IOCTL_SETKEY),
				IOCTL_ERR(IEEE80211_IOCTL_DELKEY),
				IOCTL_ERR(IEEE80211_IOCTL_ADDMAC),
				IOCTL_ERR(IEEE80211_IOCTL_DELMAC),
				IOCTL_ERR(IEEE80211_IOCTL_WDSADDMAC),
				IOCTL_ERR(IEEE80211_IOCTL_WDSDELMAC),
			};
			if (IEEE80211_IOCTL_SETPARAM <= op &&
			    op <= IEEE80211_IOCTL_SETCHANLIST)
				perror(opnames[op - SIOCIWFIRSTPRIV]);
			else
				perror("ioctl[unknown???]");
		}
		return -1;
	}
	return 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-i device] channels...\n", progname);
	exit(-1);
}

#define	MAXCHAN	((int)(sizeof(struct ieee80211req_chanlist) * NBBY))

#ifdef DOMULTI

int
athchans_init(int argc, char *argv[])
{

#else

int
main(int argc, char *argv[])
{

#endif
	const char *ifname = "wifi0";
	struct ieee80211req_chanlist chanlist;
	int c;

	progname = argv[0];
	while ((c = getopt(argc, argv, "i:")) != -1)
		switch (c) {
		case 'i':
			ifname = optarg;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	argc -= optind;
	argv += optind;
	if (argc < 1)
		usage();

	memset(&chanlist, 0, sizeof(chanlist));
	for (; argc > 0; argc--, argv++) {
		int first, last, f;

		switch (sscanf(argv[0], "%u-%u", &first, &last)) {
		case 1:
			if (first > MAXCHAN)
				errx(-1, "%s: channel %u out of range, max %u",
					progname, first, MAXCHAN);
			setbit(chanlist.ic_channels, first);
			break;
		case 2:
			if (first > MAXCHAN)
				errx(-1, "%s: channel %u out of range, max %u",
					progname, first, MAXCHAN);
			if (last > MAXCHAN)
				errx(-1, "%s: channel %u out of range, max %u",
					progname, last, MAXCHAN);
			if (first > last)
				errx(-1, "%s: void channel range, %u > %u",
					progname, first, last);
			for (f = first; f <= last; f++)
				setbit(chanlist.ic_channels, f);
			break;
		}
	}
	return set80211priv(ifname, IEEE80211_IOCTL_SETCHANLIST,
		&chanlist, sizeof(chanlist), 1);
}
