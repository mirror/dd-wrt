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
 * $Id: athkey.c 1426 2006-02-01 20:07:11Z mrenzmann $
 */

/*
 * athkey [-i interface] keyix cipher keyval
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

static int s = -1;
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

static int
digittoint(int c)
{
	return isdigit(c) ? c - '0' : isupper(c) ? c - 'A' + 10 : c - 'a' + 10;
}

static int
getdata(const char *arg, u_int8_t *data, int maxlen)
{
	const char *cp = arg;
	int len;

	if (cp[0] == '0' && (cp[1] == 'x' || cp[1] == 'X'))
		cp += 2;
	len = 0;
	while (*cp) {
		int b0, b1;
		if (cp[0] == ':' || cp[0] == '-' || cp[0] == '.') {
			cp++;
			continue;
		}
		if (!isxdigit(cp[0])) {
			fprintf(stderr, "%s: invalid data value %c (not hex)\n",
				progname, cp[0]);
			exit(-1);
		}
		b0 = digittoint(cp[0]);
		if (cp[1] != '\0') {
			if (!isxdigit(cp[1])) {
				fprintf(stderr, "%s: invalid data value %c "
					"(not hex)\n", progname, cp[1]);
				exit(-1);
			}
			b1 = digittoint(cp[1]);
			cp += 2;
		} else {			/* fake up 0<n> */
			b1 = b0, b0 = 0;
			cp += 1;
		}
		if (len > maxlen) {
			fprintf(stderr,
				"%s: too much data in %s, max %u bytes\n",
				progname, arg, maxlen);
		}
		data[len++] = (b0 << 4) | b1;
	}
	return len;
}

static int
getcipher(const char *name)
{
#define	streq(a,b)	(strcasecmp(a,b) == 0)

	if (streq(name, "wep"))
		return IEEE80211_CIPHER_WEP;
	if (streq(name, "tkip"))
		return IEEE80211_CIPHER_TKIP;
	if (streq(name, "aes-ocb") || streq(name, "ocb"))
		return IEEE80211_CIPHER_AES_OCB;
	if (streq(name, "aes-ccm") || streq(name, "ccm") ||
	    streq(name, "aes"))
		return IEEE80211_CIPHER_AES_CCM;
	if (streq(name, "ckip"))
		return IEEE80211_CIPHER_CKIP;
	if (streq(name, "none") || streq(name, "clr"))
		return IEEE80211_CIPHER_NONE;

	fprintf(stderr, "%s: unknown cipher %s\n", progname, name);
	exit(-1);
#undef streq
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-i device] keyix cipher keyval [mac]\n",
		progname);
	exit(-1);
}

#ifdef DOMULTI

int
athkey_init(int argc, char *argv[])
{

#else

int
main(int argc, char *argv[])
{

#endif
	const char *ifname = "wifi0";
	struct ieee80211req_key setkey;
	struct ieee80211req_del_key delkey;
	int c, keyix;
	int op = IEEE80211_IOCTL_SETKEY;

	progname = argv[0];
	while ((c = getopt(argc, argv, "di:")) != -1)
		switch (c) {
		case 'd':
			op = IEEE80211_IOCTL_DELKEY;
			break;
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

	keyix = atoi(argv[0]);
	if (!(1 <= keyix && keyix <= 4))
		errx(-1, "%s: invalid key index %s, must be [1..4]",
			progname, argv[0]);
	switch (op) {
	case IEEE80211_IOCTL_DELKEY:
		memset(&delkey, 0, sizeof(delkey));
		delkey.idk_keyix = keyix - 1;
		return set80211priv(ifname, op, &delkey, sizeof(delkey), 1);
	case IEEE80211_IOCTL_SETKEY:
		if (argc != 3 && argc != 4)
			usage();
		memset(&setkey, 0, sizeof(setkey));
		setkey.ik_flags = IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV;
		setkey.ik_keyix = keyix - 1;
		setkey.ik_type = getcipher(argv[1]);
		setkey.ik_keylen = getdata(argv[2], setkey.ik_keydata,
			sizeof(setkey.ik_keydata));
		if (argc == 4)
			(void) getdata(argv[3], setkey.ik_macaddr,
				IEEE80211_ADDR_LEN);
		return set80211priv(ifname, op, &setkey, sizeof(setkey), 1);
	}
	return -1;
}
